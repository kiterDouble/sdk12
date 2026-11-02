#include <array>
#include <pthread.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <iostream>
#include <random>
#include <sys/file.h>
#include <atomic>

#include "terminal_management_api.h"
#include "sr_log.h"
#include "srTspCallbackRegister.h"

#include "terminal_management_data_monitor.h"
#include "terminal_management_data_packag.h"
#include "terminal_management_data_report.h"

#include "queue_msg.h"
// #include "sr_ipc.h"
#include "modem_api.h"
#include "pm_server_api.h"
#include "hal_modem_ant.h"

#include "net/sirun/tboxs3/connect/report.pb.h"
#include "net/sirun/tboxs3/connect/common.pb.h"
#include "net/sirun/tboxs3/connect/event_report.pb.h"

struct SpeedHistory
{
    static const int HISTORY_SIZE = (3000 + INTERVAL_MS - 1) / INTERVAL_MS;
    float speeds[HISTORY_SIZE];
    int index;
    int count; // 已记录点数，用于判断数据是否攒够

    SpeedHistory() : index(0), count(0)
    {
        for (int i = 0; i < HISTORY_SIZE; ++i)
            speeds[i] = 0;
    }

    void add(float speed)
    {
        speeds[index] = speed;
        index = (index + 1) % HISTORY_SIZE;
        if (count < HISTORY_SIZE)
            ++count;
    }

    bool anyInLast3SecondsGreaterThan(float threshold) const
    {
        for (int i = 0; i < HISTORY_SIZE; ++i)
        {
            if (speeds[i] > threshold)
                return true;
        }
        return false;
    }

    /**
     * @brief 获取当前加速度（速度变化率）
     * @param intervalSec 采样间隔（秒）
     * @return 单位为 m/s²，加速为正，减速为负；若历史数据不足2点，返回 0.0f
     */
    float getSpeedDifference(float intervalSec) const
    {
        if (count < 2)
            return 0.0f; // 历史数据不足，返回0

        int lastIdx = (index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
        int prevIdx = (index - 2 + HISTORY_SIZE) % HISTORY_SIZE;

        float deltaSpeed = speeds[lastIdx] - speeds[prevIdx];   // km/h
        float deltaSpeed_ms = deltaSpeed * (1000.0f / 3600.0f); // m/s
        return deltaSpeed_ms / intervalSec;                     // m/s²，正为加速，负为减速
    }

    // 判断已有数据是否满3秒窗口（即已记录点数达到窗口大小）
    bool isHistoryFull() const
    {
        return count >= HISTORY_SIZE;
    }
};

struct BatteryVoltageHistory
{
    static const int HISTORY_SIZE = (30 * 60 * 1000 + INTERVAL_MS - 1) / INTERVAL_MS;
    float voltages[HISTORY_SIZE];
    int index;
    int count;

    BatteryVoltageHistory() : index(0), count(0)
    {
        for (int i = 0; i < HISTORY_SIZE; ++i)
            voltages[i] = 0;
    }

    void add(float voltage)
    {
        voltages[index] = voltage;
        index = (index + 1) % HISTORY_SIZE;
        if (count < HISTORY_SIZE)
            ++count;
    }

    // 检查最近指定分钟内所有电压是否都低于阈值
    bool isVoltageBelowThresholdForLastMinutes(float threshold, int minutes) const
    {
        int requiredPoints = (minutes * 60 * 1000 + INTERVAL_MS - 1) / INTERVAL_MS;
        if (count < requiredPoints)
            return false;
        int startIdx = (index - requiredPoints + HISTORY_SIZE) % HISTORY_SIZE;
        for (int i = 0; i < requiredPoints; ++i)
        {
            int idx = (startIdx + i) % HISTORY_SIZE;
            if (voltages[idx] >= threshold)
                return false;
        }
        return true;
    }

    // 检查最近指定分钟内所有电压是否都高于阈值
    bool isVoltageAboveThresholdForLastMinutes(float threshold, int minutes) const
    {
        int requiredPoints = (minutes * 60 * 1000 + INTERVAL_MS - 1) / INTERVAL_MS;
        if (count < requiredPoints)
            return false;
        int startIdx = (index - requiredPoints + HISTORY_SIZE) % HISTORY_SIZE;
        for (int i = 0; i < requiredPoints; ++i)
        {
            int idx = (startIdx + i) % HISTORY_SIZE;
            if (voltages[idx] <= threshold)
                return false;
        }
        return true;
    }

    // 获取最近30分钟内的电压上升量（当前电压 - 30分钟前的电压）
    float getVoltageRiseInLast30Minutes() const
    {
        if (count < HISTORY_SIZE)
            return 0.0f;
        int latestIdx = (index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
        int startIdx = index;
        return voltages[latestIdx] - voltages[startIdx];
    }

    bool isHistoryFull() const
    {
        return count >= HISTORY_SIZE;
    }
};

static EventGather g_triggerEventGather[13] = {
    {.eventStatus = EventStatus::EVENT_STATUS_RECOVERY,
     .monitorFuncCallback = NULL,
     .triggerHandleFunc = NULL,
     .recoveryHandleFunc = NULL},
};

typedef struct
{
    std::atomic<bool> VehicleCanCommFail;
    std::atomic<bool> GpsModuleFault;
    std::atomic<bool> BatteryChargeFail;
} FaultStatus_t;

static MonitoringData_t g_monitoringData = {0};

static pthread_t g_monitoringId = -1;
static pthread_t g_reissueId = -1;

static FaultStatus_t g_faultStatus; // 故障状态标记

static SpeedHistory g_speedHistory;                          // 整车CAN数据采集 HISTORY_SIZE(当前：3) 秒内的历史速度数据
static BatteryVoltageHistory g_backupBatteryVoltageHistory;  // TBOX备用电池在 HISTORY_SIZE(当前: 30*60) 秒内的历史电压数据
static BatteryVoltageHistory g_vehicleBatteryVoltageHistory; // 整车KL30蓄电池在 HISTORY_SIZE(当前: 30*60) 秒内的历史电压数据

static pthread_mutex_t g_statusFlagMutex = PTHREAD_MUTEX_INITIALIZER;        // 用于保护事件、命令状态标志文件操作的互斥锁
static pthread_mutex_t g_configMutex = PTHREAD_MUTEX_INITIALIZER;            // 配置地址的互斥锁
static pthread_rwlock_t g_monitoringDataRwlock = PTHREAD_RWLOCK_INITIALIZER; // 监控数据的的读写锁
static MonitorConfigPath g_configPath;                                       // g_configPath.downloadFolder 补发数据库存放地址

extern pthread_cond_t g_recvedMcuRebootAckCond;   // 用于通知接收到MCU重启ACK回执
extern pthread_mutex_t g_recvedMcuRebootAckMutex; // 接收到MCU重启ACK回执的互斥锁

static timer_t g_updateDatatimer = NULL; // 更新CPU使用率所使用的定时器句柄

/*事件状态监控函数声明-1期*/
static EventStatus MonitorBatteryLowVoltage(const MonitoringData_t rawData);
static EventStatus MonitorGpsSignalLost(const MonitoringData_t rawData);
static EventStatus MonitorCanBusCommInterrupt(const MonitoringData_t rawData);
static EventStatus MonitorVehicleCanCommFail(const MonitoringData_t rawData);
static EventStatus MonitorGpsModuleFault(const MonitoringData_t rawData);
static EventStatus MonitorVehicleStart(const MonitoringData_t rawData);
static EventStatus MonitorVehicleSpeedGreater0(const MonitoringData_t rawData);
static EventStatus MonitorVehicleSpeedGreater100(const MonitoringData_t rawData);
static EventStatus MonitorVehicleRemoteStart(const MonitoringData_t rawData);

/*事件状态监控函数声明-2期*/
static EventStatus MonitorVehicleUnexpectedHvPowerDown(const MonitoringData_t rawData);
static EventStatus MonitorDrivingDecelerationPowerLoss(const MonitoringData_t rawData);
static EventStatus MonitorVehicleHvPowerOnFail(const MonitoringData_t rawData);
static EventStatus MonitorBatteryChargeFail(const MonitoringData_t rawData);

// 初始化相关
// ******************************************************************************************************************************************************//

/**
 * @brief 获取配置地址
 */
void MonitorConfig_Get(MonitorConfigPath *out)
{
    if (!out)
        return;

    pthread_mutex_lock(&g_configMutex);
    memcpy(out, &g_configPath, sizeof(MonitorConfigPath));
    pthread_mutex_unlock(&g_configMutex);
}

/**
 * @brief 更新配置地址
 */
void MonitorConfig_Set(const MonitorConfigPath *newConfig)
{
    if (!newConfig)
        return;

    pthread_mutex_lock(&g_configMutex);
    memcpy(&g_configPath, newConfig, sizeof(MonitorConfigPath));
    pthread_mutex_unlock(&g_configMutex);
}

/**
 * @brief 地址配置初始化
 *
 * @return int
 */
int MonitorCfgPathInit(const char *dataFilePath, const char *taskConfigFilePath,
                       const char *downloadFolder, const char *unzipFolder)
{
    MonitorConfigPath configPath = {0};
    strncpy(configPath.dataFilePath, dataFilePath, sizeof(configPath.dataFilePath) - 1);
    configPath.dataFilePath[sizeof(configPath.dataFilePath) - 1] = '\0';
    strncpy(configPath.taskConfigFilePath, taskConfigFilePath, sizeof(configPath.taskConfigFilePath) - 1);
    configPath.taskConfigFilePath[sizeof(configPath.taskConfigFilePath) - 1] = '\0';
    strncpy(configPath.downloadFolder, downloadFolder, sizeof(configPath.downloadFolder) - 1);
    configPath.downloadFolder[sizeof(configPath.downloadFolder) - 1] = '\0';
    strncpy(configPath.unzipFolder, unzipFolder, sizeof(configPath.unzipFolder) - 1);
    configPath.unzipFolder[sizeof(configPath.unzipFolder) - 1] = '\0';

    MonitorConfig_Set(&configPath);
    return RET_OK;
}

/**
 * @brief 初始化触发事件回调
 *
 */
void InitTriggerEventCallback()
{
    // 1、TBOX备用电池低压监控
    g_triggerEventGather[0].monitorFuncCallback = MonitorBatteryLowVoltage;
    g_triggerEventGather[0].triggerHandleFunc = BatteryLowVoltageTriggerEventPack;
    g_triggerEventGather[0].recoveryHandleFunc = BatteryLowVoltageRecoveryEventPack;
    // 2、GPS信号丢失状态监控
    g_triggerEventGather[1].monitorFuncCallback = MonitorGpsSignalLost;
    g_triggerEventGather[1].triggerHandleFunc = GpsSignalLostTriggerEventPack;
    g_triggerEventGather[1].recoveryHandleFunc = GpsSignalLostRecoveryEventPack;
    // 3、CAN总线通讯中断监控
    g_triggerEventGather[2].monitorFuncCallback = MonitorCanBusCommInterrupt;
    g_triggerEventGather[2].triggerHandleFunc = CanBusCommInterruptTriggerEventPack;
    g_triggerEventGather[2].recoveryHandleFunc = CanBusCommInterruptRecoveryEventPack;
    // 4、车辆意外高压下电监控
    g_triggerEventGather[3].monitorFuncCallback = MonitorVehicleUnexpectedHvPowerDown;
    g_triggerEventGather[3].triggerHandleFunc = VehicleUnexpectedHvPowerDownTriggerEventPack;
    g_triggerEventGather[3].recoveryHandleFunc = VehicleUnexpectedHvPowerDownRecoveryEventPack;
    // 5、行驶中异常减速监控（不实现）
    g_triggerEventGather[4].monitorFuncCallback = NULL; // MonitorDrivingDecelerationPowerLoss
    g_triggerEventGather[4].triggerHandleFunc = NULL;
    g_triggerEventGather[4].recoveryHandleFunc = NULL;
    // 6、车辆高压上电失败监控（不实现）
    g_triggerEventGather[5].monitorFuncCallback = NULL; // MonitorVehicleHvPowerOnFail
    g_triggerEventGather[5].triggerHandleFunc = NULL;
    g_triggerEventGather[5].recoveryHandleFunc = NULL;
    // 7、车辆无法与CAN通信监控
    g_triggerEventGather[6].monitorFuncCallback = MonitorVehicleCanCommFail;
    g_triggerEventGather[6].triggerHandleFunc = VehicleCanCommFailTriggerEventPack;
    g_triggerEventGather[6].recoveryHandleFunc = VehicleCanCommFailRecoveryEventPack;
    // 8、GPS模块故障监控
    g_triggerEventGather[7].monitorFuncCallback = MonitorGpsModuleFault;
    g_triggerEventGather[7].triggerHandleFunc = GpsModuleFaultTriggerEventPack;
    g_triggerEventGather[7].recoveryHandleFunc = GpsModuleFaultRecoveryEventPack;
    // 9、小电池无法充电监控
    g_triggerEventGather[8].monitorFuncCallback = MonitorBatteryChargeFail;
    g_triggerEventGather[8].triggerHandleFunc = BatteryChargeFailTriggerEventPack;
    g_triggerEventGather[8].recoveryHandleFunc = BatteryChargeFailRecoveryEventPack;
    // 10、车辆启动状态监控
    g_triggerEventGather[9].monitorFuncCallback = MonitorVehicleStart;
    g_triggerEventGather[9].triggerHandleFunc = VehicleStartTriggerEventPack;
    g_triggerEventGather[9].recoveryHandleFunc = NULL;
    // 11、车辆速度大于0监控
    g_triggerEventGather[10].monitorFuncCallback = MonitorVehicleSpeedGreater0;
    g_triggerEventGather[10].triggerHandleFunc = VehicleSpeedGreater0TriggerEventPack;
    g_triggerEventGather[10].recoveryHandleFunc = NULL;
    // 12、车辆速度大于100km/h监控
    g_triggerEventGather[11].monitorFuncCallback = MonitorVehicleSpeedGreater100;
    g_triggerEventGather[11].triggerHandleFunc = VehicleSpeedGreater100TriggerEventPack;
    g_triggerEventGather[11].recoveryHandleFunc = NULL;
    // 13、车辆远程启动状态监控（不实现）
    g_triggerEventGather[12].monitorFuncCallback = NULL; // MonitorVehicleRemoteStart
    g_triggerEventGather[12].triggerHandleFunc = NULL;
    g_triggerEventGather[12].recoveryHandleFunc = NULL;
}

/**
 * @brief 从磁盘读取事件的触发状态（支持位图和独立文件）
 * @param flagIdx     事件索引（1~13 使用位图，20 使用独立文件）
 * @param cmdId       输出缓冲区，仅在 flagIdx=20 且文件存在时读取内容
 * @param cmdIdSize   缓冲区大小
 * @return EventStatus::EVENT_STATUS_TRIGGER 触发；EVENT_STATUS_NORMAL 未触发
 */
EventStatus InitStateFlagFromDisk(int flagIdx, char *cmdId, size_t cmdIdSize)
{
    char filePath[256] = "";

    // 1~13：位图方式管理
    if (flagIdx >= 1 && flagIdx <= 13)
    {
        snprintf(filePath, sizeof(filePath), "%s%s",
                 g_configPath.taskConfigFilePath, "StatusFlagGroup");
        printf("InitStateFlagFromDisk flagIdx=%d path=%s\n", flagIdx, filePath);

        pthread_mutex_lock(&g_statusFlagMutex);

        FILE *fp = fopen(filePath, "r");
        if (!fp)
        {
            pthread_mutex_unlock(&g_statusFlagMutex);
            return EventStatus::EVENT_STATUS_NORMAL;
        }

        unsigned int statusBits = 0;
        int ret = fscanf(fp, "%u", &statusBits);
        fclose(fp);
        pthread_mutex_unlock(&g_statusFlagMutex);

        if (ret != 1)
            return EventStatus::EVENT_STATUS_NORMAL;

        return ((statusBits >> flagIdx) & 1U) ? EventStatus::EVENT_STATUS_TRIGGER
                                              : EventStatus::EVENT_STATUS_NORMAL;
    }

    // 20~22：重启类标志文件
    const char *fileName = NULL;
    switch (flagIdx)
    {
    case 20:
        fileName = "rebootTboxAck";
        break;
    case 21:
        fileName = "rebootMcuAck";
        break;
    case 22:
        fileName = "rebootCpuAck";
        break;
    default:
        break;
    }
    if (fileName)
    {
        snprintf(filePath, sizeof(filePath), "%s%s",
                 g_configPath.taskConfigFilePath, fileName);
        printf("InitStateFlagFromDisk flagIdx=%d path=%s\n", flagIdx, filePath);

        pthread_mutex_lock(&g_statusFlagMutex);
        FILE *fp = fopen(filePath, "r");
        if (!fp)
        {
            LOG_Error("%s() fopen error for %s", __func__, fileName);
            pthread_mutex_unlock(&g_statusFlagMutex);
            return EventStatus::EVENT_STATUS_NORMAL;
        }

        // 读取 cmdId
        if (cmdId != NULL && cmdIdSize > 0)
        {
            if (fgets(cmdId, cmdIdSize, fp) == NULL)
                cmdId[0] = '\0';
            // else
            //     cmdId[strcspn(cmdId, "\r\n")] = '\0'; // 清除换行
        }
        fclose(fp);
        pthread_mutex_unlock(&g_statusFlagMutex);

        return EventStatus::EVENT_STATUS_TRIGGER;
    }

    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 设置事件触发状态到磁盘
 * @param flagIdx  事件索引（1~13 使用位图，20 使用独立文件）
 * @param cmdId    指令 ID（仅 flagIdx=20 时写入文件，其余忽略）
 * @return RET_OK 成功，RET_ERR 失败
 */
int SetStatusFlagToDisk(int flagIdx, const char *cmdId)
{
    char filePath[256];

    // 1~13：更新位图文件
    if (flagIdx >= 1 && flagIdx <= 13)
    {
        snprintf(filePath, sizeof(filePath), "%s%s",
                 g_configPath.taskConfigFilePath, "StatusFlagGroup");

        pthread_mutex_lock(&g_statusFlagMutex);

        // 尝试以 r+ 模式打开，不存在则创建新文件
        FILE *fp = fopen(filePath, "r+");
        if (fp == NULL)
        {
            fp = fopen(filePath, "w");
            if (fp == NULL)
            {
                pthread_mutex_unlock(&g_statusFlagMutex);
                LOG_Error("%s() fopen error", __func__);
                return RET_ERR;
            }
            fprintf(fp, "0");
            fclose(fp);

            fp = fopen(filePath, "r+");
            if (fp == NULL)
            {
                pthread_mutex_unlock(&g_statusFlagMutex);
                LOG_Error("%s() reopen error", __func__);
                return RET_ERR;
            }
        }

        // 读取当前位图
        unsigned int statusBits = 0;
        if (fscanf(fp, "%u", &statusBits) != 1)
            statusBits = 0;

        // 置位
        statusBits |= (1U << flagIdx);

        // 写回文件
        rewind(fp);
        fprintf(fp, "%u", statusBits);
        fflush(fp);
        ftruncate(fileno(fp), ftell(fp));
        fclose(fp);

        pthread_mutex_unlock(&g_statusFlagMutex);
        return RET_OK;
    }

    // 20~22：创建重启类标志文件，并写入 cmdId
    const char *fileName = NULL;
    switch (flagIdx)
    {
    case 20:
        fileName = "rebootTboxAck";
        break;
    case 21:
        fileName = "rebootMcuAck";
        break;
    case 22:
        fileName = "rebootCpuAck";
        break;
    default:
        break;
    }
    if (fileName)
    {
        snprintf(filePath, sizeof(filePath), "%s%s",
                 g_configPath.taskConfigFilePath, fileName);

        pthread_mutex_lock(&g_statusFlagMutex);

        FILE *fp = fopen(filePath, "w");
        if (fp == NULL)
        {
            pthread_mutex_unlock(&g_statusFlagMutex);
            LOG_Error("%s() fopen error for %s", __func__, fileName);
            return RET_ERR;
        }

        if (cmdId != NULL)
        {
            fprintf(fp, "%s", cmdId);
        }
        // 若 cmdId 为 NULL，仅创建空文件，表示触发
        fclose(fp);

        pthread_mutex_unlock(&g_statusFlagMutex);
        return RET_OK;
    }

    return RET_ERR;
}

/**
 * @brief 清除磁盘中记录的事件触发状态
 * @param flagIdx 事件索引（1~13 使用位图，20 使用独立文件）
 * @return RET_OK 成功，RET_ERR 失败
 */
int ResetStatusFlagToDisk(int flagIdx)
{
    char filePath[256];

    // 1~13：清除位图中的对应位
    if (flagIdx >= 1 && flagIdx <= 13)
    {
        snprintf(filePath, sizeof(filePath), "%s%s",
                 g_configPath.taskConfigFilePath, "StatusFlagGroup");

        pthread_mutex_lock(&g_statusFlagMutex);

        FILE *fp = fopen(filePath, "r+");
        if (fp == NULL)
        {
            // 文件不存在即认为状态已清除
            pthread_mutex_unlock(&g_statusFlagMutex);
            return RET_OK;
        }

        unsigned int statusBits = 0;
        if (fscanf(fp, "%u", &statusBits) != 1)
        {
            fclose(fp);
            pthread_mutex_unlock(&g_statusFlagMutex);
            return RET_OK; // 内容异常，视为无状态
        }

        // 清除对应位
        statusBits &= ~(1U << flagIdx);

        rewind(fp);
        fprintf(fp, "%u", statusBits);
        fflush(fp);
        ftruncate(fileno(fp), ftell(fp));
        fclose(fp);

        pthread_mutex_unlock(&g_statusFlagMutex);
        return RET_OK;
    }

    // 20~22：删除重启类标志文件
    const char *fileName = NULL;
    switch (flagIdx)
    {
    case 20:
        fileName = "rebootTboxAck";
        break;
    case 21:
        fileName = "rebootMcuAck";
        break;
    case 22:
        fileName = "rebootCpuAck";
        break;
    default:
        break;
    }
    if (fileName)
    {
        snprintf(filePath, sizeof(filePath), "%s%s",
                 g_configPath.taskConfigFilePath, fileName);

        pthread_mutex_lock(&g_statusFlagMutex);
        int ret = remove(filePath);
        pthread_mutex_unlock(&g_statusFlagMutex);

        if (ret != 0 && errno != ENOENT) // 文件不存在也视为成功
        {
            LOG_Error("%s() remove %s failed", __func__, fileName);
            return RET_ERR;
        }
        return RET_OK;
    }

    return RET_ERR;
}

// 辅助函数
// ******************************************************************************************************************************************************//

/**
 * @brief 创建周期定时器
 * @param callback 回调函数（必须是 void (*)(union sigval) 类型）
 * @param timeoutMs 超时时间（毫秒，必须 > 0）
 * @param userData 传递给回调的用户数据
 * @param timerId 输出参数，返回定时器句柄
 * @return 0 成功，-1 失败
 */
int CreateResourceTimer(void (*callback)(union sigval), int timeoutMs, void *userData, timer_t *timerId)
{
    // 1. 参数校验
    if (!callback || !timerId || timeoutMs <= 0)
    {
        errno = EINVAL;
        return -1;
    }

    // 2. 配置 sigevent
    struct sigevent sev;
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = callback;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_value.sival_ptr = userData;

    // 3. 创建定时器
    timer_t localTimerId;
    if (timer_create(CLOCK_MONOTONIC, &sev, &localTimerId) == -1)
    {
        return -1;
    }

    // 4. 配置 itimerspec
    struct itimerspec its;
    its.it_value.tv_sec = timeoutMs / 1000;
    its.it_value.tv_nsec = (timeoutMs % 1000) * 1000000;
    its.it_interval.tv_sec = timeoutMs / 1000;
    its.it_interval.tv_nsec = (timeoutMs % 1000) * 1000000;

    // 5. 启动定时器
    if (timer_settime(localTimerId, 0, &its, NULL) == -1)
    {
        timer_delete(localTimerId);
        return -1;
    }

    // 6. 输出定时器句柄
    *timerId = localTimerId;
    return 0;
}

/**
 * @brief 获取唤醒源到 WorkMode 的映射表
 *
 * @return const std::array<net::sirun::tboxs3::report::WorkMode, 0x28>&
 */
static const std::array<net::sirun::tboxs3::report::WorkMode, 0x28> &GetWakeupSrcMap()
{
    static const auto map = []()
    {
        std::array<net::sirun::tboxs3::report::WorkMode, 0x28> m{};
        // 默认值均为 unknowAwakenedMode（数组已零初始化）

        // CAN 唤醒 (0x01 ~ 0x08)
        for (int i = 0x01; i <= 0x08; ++i)
        {
            m[i] = net::sirun::tboxs3::report::WorkMode::can;
        }
        // 根据真值表逐项赋值
        m[PM_WAKEUP_SOURCE_MCU] = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode;             // MPU 唤醒（暂未定义）
        m[PM_WAKEUP_SOURCE_KL15] = net::sirun::tboxs3::report::WorkMode::kl15;                          // KL15 唤醒
        m[PM_WAKEUP_SOURCE_KL30] = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode;            // KL30 唤醒（暂未定义）
        m[PM_WAKEUP_SOURCE_BLE] = net::sirun::tboxs3::report::WorkMode::remote;                         // 蓝牙唤醒
        m[PM_WAKEUP_SOURCE_RTC] = net::sirun::tboxs3::report::WorkMode::rtc;                            // MCU RTC 唤醒
        m[PM_WAKEUP_SOURCE_G_SENSOR] = net::sirun::tboxs3::report::WorkMode::sensor;                    // G-Sensor 唤醒
        m[PM_WAKEUP_SOURCE_BCALL] = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode;           // B-Call 唤醒（暂未定义）
        m[PM_WAKEUP_SOURCE_ECALL] = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode;           // E-Call 唤醒（暂未定义）
        m[PM_WAKEUP_SOURCE_TIMER] = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode;           // MPU 定时器唤醒（暂未定义）
        m[PM_WAKEUP_SOURCE_SMS] = net::sirun::tboxs3::report::WorkMode::sms;                            // SMS 短信唤醒
        m[PM_WAKEUP_SOURCE_RING] = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode;            // 电话唤醒（暂未定义）
        m[PM_WAKEUP_SOURCE_DATA] = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode;            // 网络数据唤醒（暂未定义）
        m[PM_WAKEUP_SOURCE_ETH] = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode;             // 以太网唤醒（暂未定义）
        m[PM_WAKEUP_SOURCE_TIMER_HEARTBEAT] = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode; // 暂未定义
        m[PM_WAKEUP_SOURCE_TIMER_REBOOT] = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode;    // 暂未定义
        m[PM_WAKEUP_SOURCE_TIMER_WEEK] = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode;      // 暂未定义
        return m;
    }();
    return map;
}

/**
 * @brief 获取 sigType 到 NetworkType 的映射表
 *
 * @return const std::array<net::sirun::tboxs3::common::NetworkType, 2>&
 */
static const std::array<net::sirun::tboxs3::common::NetworkType, 2> &GetNetworkTypeMap()
{
    static const auto map = []()
    {
        std::array<net::sirun::tboxs3::common::NetworkType, 2> m{};
        m[0] = net::sirun::tboxs3::common::NetworkType::GSM_2G; // sigType = 0 → 2G
        m[1] = net::sirun::tboxs3::common::NetworkType::On_4G;  // sigType = 1 → 4G
        return m;
    }();
    return map;
}

// 原始数据相关
// ******************************************************************************************************************************************************//

/**
 * @brief 获取原始数据
 *
 * @param out
 * @return int
 */
int GetMonitoringData(MonitoringData_t *out)
{
    if (!out)
        return RET_ERR;
    pthread_rwlock_rdlock(&g_monitoringDataRwlock);
    memcpy(out, &g_monitoringData, sizeof(MonitoringData_t));
    pthread_rwlock_unlock(&g_monitoringDataRwlock);
    return RET_OK;
}

/**
 * @brief 获取 CPU 使用率（百分比）
 * @return CPU 使用率 (0.0 ~ 100.0)，失败返回 -1.0
 */
static double GetCpuUsage(void)
{
    static unsigned long last_total = 0, last_idle = 0;
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp)
        return -1.0;

    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
    if (fscanf(fp, "cpu  %lu %lu %lu %lu %lu %lu %lu %lu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) != 8)
    {
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    unsigned long total = user + nice + system + idle + iowait + irq + softirq + steal;
    unsigned long idle_total = idle + iowait;

    if (last_total == 0)
    {
        // 第一次调用，仅记录基准值
        last_total = total;
        last_idle = idle_total;
        return 0.0;
    }

    unsigned long total_diff = total - last_total;
    unsigned long idle_diff = idle_total - last_idle;

    last_total = total;
    last_idle = idle_total;

    if (total_diff == 0)
        return 0.0;
    return 100.0 * (1.0 - (double)idle_diff / total_diff);
}

/**
 * @brief 获取内存使用率（百分比）
 * @return 内存使用率 (0.0 ~ 100.0)，失败返回 -1.0
 */
static double GetMemoryUsage(void)
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp)
        return -1.0;

    unsigned long mem_total = 0, mem_available = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        if (sscanf(line, "MemTotal: %lu kB", &mem_total) == 1)
            continue;
        if (sscanf(line, "MemAvailable: %lu kB", &mem_available) == 1)
            break;
    }
    fclose(fp);

    if (mem_total == 0)
        return -1.0;
    return 100.0 * (1.0 - (double)mem_available / mem_total);
}

/**
 * @brief 获取EMMC分区挂载状态（/mnt/sdcard）
 * @return 1 已挂载，0 未挂载
 */
static int GetEmmcMountStatus(void)
{
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp)
        return 0;

    char line[512];
    int mounted = 0;
    while (fgets(line, sizeof(line), fp))
    {
        char dev[128], mnt[128];
        if (sscanf(line, "%127s %127s", dev, mnt) == 2)
        {
            if (strcmp(mnt, "/mnt/sdcard") == 0)
            {
                mounted = 1;
                break;
            }
        }
    }
    fclose(fp);
    return mounted;
}

/**
 * @brief 获取指定路径的磁盘使用率（百分比）
 * @param path 挂载点路径，如 "/mnt/sdcard/" 或 "/oemdata/"
 * @return 磁盘使用率 (0.0 ~ 100.0)，失败返回 -1.0
 */
static double GetDiskUsage(const char *path)
{
    struct statvfs stat;
    if (statvfs(path, &stat) != 0)
        return -1.0;

    if (stat.f_blocks == 0)
        return -1.0;

    unsigned long long used = stat.f_blocks - stat.f_bfree;
    return 100.0 * used / stat.f_blocks;
}

// 原始数据回调获取
// ******************************************************************************************************************************************************//

/**
 * @brief 初始化TBOX静态信息
 * @return RET_OK 成功，RET_ERR 失败
 */
int InitTboxStaticInfo()
{
    int ret = RET_OK;
    static MonitorTboxDbInfo_t tboxDbInfo = {0};
    static MonitorNetStatus_t netInfo = {0};

    StBaseInfoCallbacks_t cbs;
    if (TspBaseInfoGetCallbacks(&cbs) != 0)
    {
        LOG_Error("No base info callbacks registered");
        return RET_ERR;
    }

    if (cbs.getSn(tboxDbInfo.sn, sizeof(tboxDbInfo.sn)) != RET_OK)
        ret = RET_ERR;
    if (cbs.getIccid(tboxDbInfo.iccid, sizeof(tboxDbInfo.iccid)) != RET_OK)
        ret = RET_ERR;
    if (cbs.getSim(tboxDbInfo.imsi, sizeof(tboxDbInfo.imsi)) != RET_OK)
        ret = RET_ERR;
    if (cbs.getVin(tboxDbInfo.vin, sizeof(tboxDbInfo.vin)) != RET_OK)
        ret = RET_ERR;
    if (cbs.getImei(tboxDbInfo.imei, sizeof(tboxDbInfo.imei)) != RET_OK)
        ret = RET_ERR;
    if (cbs.getProjectName(tboxDbInfo.projectName, sizeof(tboxDbInfo.projectName)) != RET_OK)
        ret = RET_ERR;
    if (cbs.getSwVersionSecondary(tboxDbInfo.srSwVersion, sizeof(tboxDbInfo.srSwVersion)) != RET_OK)
        ret = RET_ERR;
    if (cbs.getHwVersion(tboxDbInfo.srHwVersion, sizeof(tboxDbInfo.srHwVersion)) != RET_OK)
        ret = RET_ERR;
    if (cbs.getSwVersionMain(tboxDbInfo.SwVersion, sizeof(tboxDbInfo.SwVersion)) != RET_OK)
        ret = RET_ERR;
    if (cbs.getHwVersion(tboxDbInfo.HwVersion, sizeof(tboxDbInfo.HwVersion)) != RET_OK)
        ret = RET_ERR;

    // 根据 ICCID 判断运营商类型
    if (ret == RET_OK && tboxDbInfo.iccid[0] != '\0')
    {
        if (strncmp(tboxDbInfo.iccid, "898600", 6) == 0 ||
            strncmp(tboxDbInfo.iccid, "898602", 6) == 0 ||
            strncmp(tboxDbInfo.iccid, "898604", 6) == 0 ||
            strncmp(tboxDbInfo.iccid, "898608", 6) == 0)
        {
            netInfo.carrierType = net::sirun::tboxs3::common::CarrierType::chinamobile; // 中国移动
        }
        else if (strncmp(tboxDbInfo.iccid, "898601", 6) == 0 ||
                 strncmp(tboxDbInfo.iccid, "898606", 6) == 0)
        {
            netInfo.carrierType = net::sirun::tboxs3::common::CarrierType::chinaunicom; // 中国联通
        }
        else if (strncmp(tboxDbInfo.iccid, "898603", 6) == 0 ||
                 strncmp(tboxDbInfo.iccid, "898611", 6) == 0)
        {
            netInfo.carrierType = net::sirun::tboxs3::common::CarrierType::chinatelecom; // 中国电信
        }
    }

    if (ret == RET_OK)
    {
        pthread_rwlock_wrlock(&g_monitoringDataRwlock);
        memcpy(&g_monitoringData.tboxDbInfo, &tboxDbInfo, sizeof(MonitorTboxDbInfo_t));
        g_monitoringData.netInfo.carrierType = netInfo.carrierType;
        pthread_rwlock_unlock(&g_monitoringDataRwlock);

        LOG_Debug("-- VIN:%s  ICCID:%s  SN:%s  IMEI:%s  SIM:%s  Project:%s  "
                  "srSW:%s  srHW:%s  SW:%s  HW:%s  carrierType:%d",
                  tboxDbInfo.vin, tboxDbInfo.iccid, tboxDbInfo.sn, tboxDbInfo.imei,
                  tboxDbInfo.imsi, tboxDbInfo.projectName,
                  tboxDbInfo.srSwVersion, tboxDbInfo.srHwVersion,
                  tboxDbInfo.SwVersion, tboxDbInfo.HwVersion,
                  netInfo.carrierType);
    }
    return ret;
}

/**
 * @brief IPC回调：获取电源管理数据
 * @param
 * @return
 */
static void PmStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    if (NULL == recvData)
    {
        LOG_Error("%s recvData is NULL", __FUNCTION__);
        return;
    }
    if (dataLen != sizeof(MPUPmStateChange_t))
    {
        LOG_Error("%s dataLen mismatch: %u != %zu", __FUNCTION__, dataLen, sizeof(MPUPmStateChange_t));
        return;
    }
    // printf("-%s(), recv DataLen:%d, type:%08x\n", __FUNCTION__, dataLen, SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY);

    const MPUPmStateChange_t *powerDate = (const MPUPmStateChange_t *)recvData;
    static MonitorMcuStatusData_t mcuStatusData = {0};
    static uint8_t lastPmState = 0U;
    uint8_t curPmState = powerDate->PmState;
    uint8_t WakeupSrc = powerDate->WakeupSrc;

    const auto &wakeupSrcMap = GetWakeupSrcMap();
    if (curPmState == PM_STATE_NORMAL)
    {
        if (WakeupSrc < wakeupSrcMap.size())
            mcuStatusData.workMode = wakeupSrcMap[WakeupSrc];
        else
            mcuStatusData.workMode = net::sirun::tboxs3::report::WorkMode::unknowAwakenedMode;
    }
    else
    {
        mcuStatusData.workMode = net::sirun::tboxs3::report::WorkMode::sleep;
    }

    if (curPmState != lastPmState)
    {
        // 更新电源状态变更时间
        lastPmState = curPmState;
        mcuStatusData.powerModeChangeTime = (int64_t)::time(nullptr);
    }

    pthread_rwlock_wrlock(&g_monitoringDataRwlock);
    memcpy(&g_monitoringData.powerDate, powerDate, sizeof(MPUPmStateChange_t));
    g_monitoringData.mcuStatusData.workMode = mcuStatusData.workMode;
    g_monitoringData.mcuStatusData.powerModeChangeTime = mcuStatusData.powerModeChangeTime;
    pthread_rwlock_unlock(&g_monitoringDataRwlock);

    LOG_Debug("---The current power mode is %s; WakeupSrc:%d",
              powerDate->PmState == PM_STATE_NORMAL ? "Normal" : "sleep", WakeupSrc);
    return;
}

/**
 * @brief IPC回调：获取硬线状态信息数据
 * @param
 * @return
 */
static void HardLineInfoCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    typedef struct
    {
        uint8_t kl15status;
        uint32_t kl30voltage;
        uint16_t fastCharge;
    } HardLineSyncInfo_t;

    if (NULL == recvData)
    {
        LOG_Error("%s recvData is NULL", __FUNCTION__);
        return;
    }
    // if (dataLen != sizeof(HardLineSyncInfo_t))
    // {
    //     LOG_Error("%s dataLen mismatch: %u != %zu", __FUNCTION__, dataLen, sizeof(HardLineSyncInfo_t));
    //     return;
    // }
    // printf("-%s(), recv DataLen:%d, type:%08x\n", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_SYNC_KLINE);

    HardLineSyncInfo_t hardLineInfo = {0};
    hardLineInfo.kl15status = recvData[0];
    hardLineInfo.kl30voltage = ((uint32_t)recvData[1] << 24) |
                               ((uint32_t)recvData[2] << 16) |
                               ((uint32_t)recvData[3] << 8) |
                               (uint32_t)recvData[4];
    hardLineInfo.fastCharge = ((uint32_t)recvData[5] << 8) | (uint32_t)recvData[6];
    MonitorMcuStatusData_t mcuStatusData = {0};
    mcuStatusData.vehicleBatteryVoltage = hardLineInfo.kl30voltage * 0.001;
    mcuStatusData.ignOn = hardLineInfo.kl15status == 2 ? 1 : 0;

    pthread_rwlock_wrlock(&g_monitoringDataRwlock);
    g_monitoringData.mcuStatusData.ignOn = mcuStatusData.ignOn;
    g_monitoringData.mcuStatusData.vehicleBatteryVoltage = mcuStatusData.vehicleBatteryVoltage;
    pthread_rwlock_unlock(&g_monitoringDataRwlock);
    g_vehicleBatteryVoltageHistory.add(mcuStatusData.vehicleBatteryVoltage);

    // LOG_Debug("---%s() ignOn:%d vehicleBatteryVoltage:%.3f", __func__,
    //           mcuStatusData.ignOn, mcuStatusData.vehicleBatteryVoltage);
}

/**
 * @brief IPC回调：获取备用电池信息数据
 * @param
 * @return
 */
static void BakBatteryInfoCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    typedef struct
    {
        uint8_t Status;
        uint16_t Voltage;
        uint16_t Temp;
        uint8_t Soc;
        uint8_t VoltageAlarm;
        uint8_t TempAlarm;
        uint8_t ConnectState;
    } BatteryInfoSync_t;

    if (NULL == recvData)
    {
        LOG_Error("%s recvData is NULL", __FUNCTION__);
        return;
    }
    // if (dataLen != sizeof(BatteryInfoSync_t))
    // {
    //     LOG_Error("%s dataLen mismatch: %u != %zu", __FUNCTION__, dataLen, sizeof(BatteryInfoSync_t));
    //     return;
    // }
    // printf("-%s(), recv DataLen:%d, type:%08x\n", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_SYNC_BATTERY);

    BatteryInfoSync_t bakBatteryInfo = {0};
    bakBatteryInfo.Status = recvData[0];
    bakBatteryInfo.Voltage = ((uint32_t)recvData[1] << 8) | (uint32_t)recvData[2];
    bakBatteryInfo.Temp = ((uint32_t)recvData[3] << 8) | (uint32_t)recvData[4];
    bakBatteryInfo.Soc = recvData[5];
    bakBatteryInfo.VoltageAlarm = recvData[6];
    bakBatteryInfo.TempAlarm = recvData[7];
    bakBatteryInfo.ConnectState = recvData[8];
    MonitorMcuStatusData_t mcuStatusData = {0};
    mcuStatusData.backupBatteryVoltage = bakBatteryInfo.Voltage * 0.001;

    pthread_rwlock_wrlock(&g_monitoringDataRwlock);
    g_monitoringData.mcuStatusData.backupBatteryVoltage = mcuStatusData.backupBatteryVoltage;
    pthread_rwlock_unlock(&g_monitoringDataRwlock);
    g_backupBatteryVoltageHistory.add(mcuStatusData.backupBatteryVoltage);

    LOG_Debug("---%s() backupBatteryVoltage:%.3f", __func__, mcuStatusData.backupBatteryVoltage);
}

/**
 * @brief IPC回调：获取CAN信息同步数据
 * @param
 * @return
 */
static void CanInfoSyncCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    typedef struct
    {
        uint8_t CANBusNum;
        uint8_t BUSOffStatus;
        uint8_t LimpHome;
    } CanInfoSync_t;

    if (NULL == recvData)
    {
        LOG_Error("%s recvData is NULL", __FUNCTION__);
        return;
    }
    // if (dataLen != sizeof(CanInfoSync_t))
    // {
    //     LOG_Error("%s dataLen mismatch: %u != %zu", __FUNCTION__, dataLen, sizeof(CanInfoSync_t));
    //     return;
    // }
    // printf("-%s(), recv DataLen:%d, type:%08x\n", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_SYNC_CAN);

    CanInfoSync_t canInfo = {0};
    memcpy(&canInfo, recvData, sizeof(CanInfoSync_t));
    MonitorMcuStatusData_t mcuStatusData = {0};
    mcuStatusData.canBusOff = 0;
    for (int i = 0; i < canInfo.CANBusNum; i++)
    {
        if ((canInfo.BUSOffStatus & (1 << i)) != 0)
        {
            mcuStatusData.canBusOff = 1;
            break;
        }
    }

    pthread_rwlock_wrlock(&g_monitoringDataRwlock);
    g_monitoringData.mcuStatusData.canBusOff = mcuStatusData.canBusOff;
    pthread_rwlock_unlock(&g_monitoringDataRwlock);

    LOG_Debug("---%s() canBusOff:%d", __func__, mcuStatusData.canBusOff);
}

#if 0
/**
 * @brief IPC回调：获取MCU的运行状态数据（暂废）
 * @param
 * @return
 */
static void McuStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    if (NULL == recvData)
    {
        LOG_Error("%s recvData is NULL", __FUNCTION__);
        return;
    }
    if (dataLen != sizeof(MonitorMcuStatusData_t))
    {
        LOG_Error("%s dataLen mismatch: %u != %zu", __FUNCTION__, dataLen, sizeof(MonitorMcuStatusData_t));
        return;
    }
    printf("-%s(), recv DataLen:%d, type:%08x\n", __FUNCTION__, dataLen, SYS_QUE_TYPE_MONITOR_POWER_NOTIFY);

    const MonitorMcuStatusData_t *mcuStatusData = (const MonitorMcuStatusData_t *)recvData;
    pthread_rwlock_wrlock(&g_monitoringDataRwlock);
    memcpy(&g_monitoringData.mcuStatusData, mcuStatusData, sizeof(MonitorMcuStatusData_t));
    pthread_rwlock_unlock(&g_monitoringDataRwlock);

    g_backupBatteryVoltageHistory.add(mcuStatusData->backupBatteryVoltage);
    g_vehicleBatteryVoltageHistory.add(mcuStatusData->vehicleBatteryVoltage);
}
#endif

/**
 * @brief IPC回调：获取GNSS数据
 * @param
 * @return
 */
static void GnssInfoCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    if (NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    if (dataLen != sizeof(GnssLocation_t))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    // printf("-%s(), recv DataLen:%d, type:%08x\n", __FUNCTION__, dataLen, SYS_QUE_TYPE_GNSS_LOCATIONINFO);

    MonitorPositioningData_t positioningData = {0};
    memcpy(&positioningData.locationInfo, recvData, sizeof(GnssLocation_t));
    positioningData.gpsModuleResetTimes = 0; // 陕汽代码中似乎还没有定位模组的重启操作
    // 模组没有原始数据，函数就不会回调，与监控函数一起实现<GPS模组有/无原始数据>的心跳控制
    positioningData.hasGpsRawData = 1;
    HalModemAntType_e ant = E_HAL_MODEM_ANT_INTERNAL;
    HalModemAntGetCurrent(&ant);
    if (ant == E_HAL_MODEM_ANT_EXTERNEL)
        positioningData.usesexternalgnss = net::sirun::tboxs3::common::BOOLEXT::isTrue;
    else if (ant == E_HAL_MODEM_ANT_INTERNAL)
        positioningData.usesexternalgnss = net::sirun::tboxs3::common::BOOLEXT::isFalse;
    else
        positioningData.usesexternalgnss = net::sirun::tboxs3::common::BOOLEXT::isUnknow;

    pthread_rwlock_wrlock(&g_monitoringDataRwlock);
    memcpy(&g_monitoringData.positioningData, &positioningData, sizeof(MonitorPositioningData_t));
    pthread_rwlock_unlock(&g_monitoringDataRwlock);

    // LOG_Debug("---GPS status:%d antStatus:%d gpsState:%d lola:%ld,%ld flola:%lf,%lf "
    //           "seaHeight:%d speed:%d direction:%d num_svs:%d timestamp:%u "
    //           "hasGpsRawData:%d GpsModuleResetTimes:%d",
    //           positioningData.locationInfo.status,
    //           positioningData.locationInfo.antStatus,
    //           positioningData.locationInfo.gpsState,
    //           positioningData.locationInfo.longtitudeValue,
    //           positioningData.locationInfo.latitudeValue,
    //           positioningData.locationInfo.flongitude,
    //           positioningData.locationInfo.flatitude,
    //           positioningData.locationInfo.seaHeight,
    //           positioningData.locationInfo.speed,
    //           positioningData.locationInfo.direction,
    //           positioningData.locationInfo.num_svs,
    //           positioningData.locationInfo.timestamp,
    //           positioningData.hasGpsRawData,
    //           positioningData.gpsModuleResetTimes);
    return;
}

/**
 * @brief  IPC回调：获取网络数据
 * @param
 * @return
 */
static void NetWorkDataCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    if (NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    if (dataLen != sizeof(ModemSrvNotify_t))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    // printf("-%s(), recv DataLen:%d, type:%08x\n", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_SYNC_NETWORK_STATUS);

    ModemSrvNotify_t cpuModemInfo = {0};
    memcpy(&cpuModemInfo, recvData, sizeof(ModemSrvNotify_t));
    MonitorNetStatus_t netInfo = {0};
    netInfo.csq = cpuModemInfo.csq;

    ModemSrvExtSigInfo_t modemSrvExtSigInfo = {0};
    int ret = ModemSrvGetExtSignalInfo(&modemSrvExtSigInfo);
    if (ret == RET_ERR)
    {
        LOG_Error("%s() ModemSrvGetExtSignalInfo fail!", __func__);
        netInfo.networkType = net::sirun::tboxs3::common::NetworkType::unknowType;
    }
    else
    {
        const auto &networkTypeMap = GetNetworkTypeMap();
        uint8_t sigType = modemSrvExtSigInfo.lteSiginfo.sigType;
        if (sigType < networkTypeMap.size())
            netInfo.networkType = networkTypeMap[sigType];
        else
            netInfo.networkType = net::sirun::tboxs3::common::NetworkType::unknowType;
    }

    pthread_rwlock_wrlock(&g_monitoringDataRwlock);
    g_monitoringData.netInfo.csq = netInfo.csq;
    g_monitoringData.netInfo.networkType = netInfo.networkType;
    pthread_rwlock_unlock(&g_monitoringDataRwlock);

    // LOG_Debug("---networkType:%d csq:%d", netInfo.networkType, netInfo.csq);
}

/**
 * @brief IPC回调：获取CAN数据解析后的信号数据
 * @param
 * @return
 */
static void CanSignalDataCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    // CAN信号解析获取
    if (NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    if (dataLen != sizeof(MonitorCanSignalData_canVal_t))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    // printf("-%s(), recv DataLen:%d, type:%08x\n", __FUNCTION__, dataLen, (SYSQueMsgType_e)MonitorCanSignalData_canVal_t_msgid);

    const MonitorCanSignalData_canVal_t *tmpRecvData = (const MonitorCanSignalData_canVal_t *)recvData;
    MonitorCanSignalData_t canSignalData = {0};
    canSignalData.keyStatus = tmpRecvData->vehicle_status;
    canSignalData.isReady = (tmpRecvData->HV_ready_status == 0x1);
    canSignalData.speed = tmpRecvData->vehicle_speed;
    canSignalData.motorSpeed = tmpRecvData->drive_motor_speed;
    canSignalData.hvSystemFltLv = tmpRecvData->BMS_fault_Lv;
    canSignalData.hvSystemStatus = (HighVoltageSystemState)tmpRecvData->BMS_status;
    canSignalData.driverOffSeat = (tmpRecvData->driver_departure_alarm == 0x1);
    canSignalData.vehiclePowerState = (VehiclePowerState)tmpRecvData->gear;
    canSignalData.engineStatus = (EngineStatus)tmpRecvData->motor_status;
    canSignalData.remoteStartActive = (tmpRecvData->remote_cmd_exec_status == 0x7);

    pthread_rwlock_wrlock(&g_monitoringDataRwlock);
    memcpy(&g_monitoringData.canSignalData, &canSignalData, sizeof(MonitorCanSignalData_t));
    pthread_rwlock_unlock(&g_monitoringDataRwlock);

    // printf("vehicle_status=%u, gear=%u, vehicle_speed=%.2f, drive_motor_speed=%d, "
    //        "HV_ready_status=%u, motor_status=%u, BMS_fault_Lv=%u, VCU_sys_DTC=%u, "
    //        "BMS_status=%u, driver_departure_alarm=%u, remote_cmd_exec_status=%u\n",
    //        tmpRecvData->vehicle_status, tmpRecvData->gear, tmpRecvData->vehicle_speed, tmpRecvData->drive_motor_speed,
    //        tmpRecvData->HV_ready_status, tmpRecvData->motor_status, tmpRecvData->BMS_fault_Lv, tmpRecvData->VCU_sys_DTC,
    //        tmpRecvData->BMS_status, tmpRecvData->driver_departure_alarm, tmpRecvData->remote_cmd_exec_status);

    return;
}

/**
 * @brief IPC回调：获取CAN总线状态
 * @param
 * @return
 */
static void CanBusStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    // CAN总线状态获取
    if (NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    if (dataLen  < 2)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    // printf("-%s(), recv DataLen:%d, type:%08x\n", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_VEHICLE_CAN_DATA);

    pthread_rwlock_wrlock(&g_monitoringDataRwlock);
    g_monitoringData.mcuStatusData.canReceiverStatus = CAN_STATUS_NORMAL;
    pthread_rwlock_unlock(&g_monitoringDataRwlock);

    return;
}

/**
 * @brief IPC回调：获取MCU重启ACK回执
 * @param
 * @return
 */
static void RebootMcuAckCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    typedef struct
    {
        uint8_t serviceData;
    } RebootMcuAck_t;

    // CAN信号解析获取
    if (NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    if (dataLen != sizeof(RebootMcuAck_t))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    // printf("-%s(), recv DataLen:%d, type:%08x\n", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_SYNC_REBOOT_MCU_REQ);

    const RebootMcuAck_t *tmpRecvData = (const RebootMcuAck_t *)recvData;
    // printf("recv serviceData:%d\n", tmpRecvData->serviceData);

    pthread_mutex_lock(&g_recvedMcuRebootAckMutex);
    pthread_cond_signal(&g_recvedMcuRebootAckCond);
    pthread_mutex_unlock(&g_recvedMcuRebootAckMutex);
    return;
}

/**
 * @brief 定时器回调：更新CPU运行状态中的全部数据
 *
 * @param sv
 */
static void UpdateResourceUsage(union sigval sv)
{
    MonitorCPUStatusData_t cpuStatusData = {0};

    // 1. CPU 使用率
    double cpu = GetCpuUsage();
    if (cpu >= 0)
        cpuStatusData.cpuUsage = (float)cpu;

    // 2. 内存使用率 & 内存使用字节数
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp)
    {
        unsigned long mem_total_kb = 0, mem_available_kb = 0;
        char line[256];
        while (fgets(line, sizeof(line), fp))
        {
            if (sscanf(line, "MemTotal: %lu kB", &mem_total_kb) == 1)
                continue;
            if (sscanf(line, "MemAvailable: %lu kB", &mem_available_kb) == 1)
                break;
        }
        fclose(fp);

        if (mem_total_kb > 0)
        {
            // 内存使用率 (%)
            double mem_usage_percent = 100.0 * (1.0 - (double)mem_available_kb / mem_total_kb);
            cpuStatusData.memoryUsage = (float)mem_usage_percent;

            // 内存已使用字节数
            unsigned long mem_used_kb = mem_total_kb - mem_available_kb;
            cpuStatusData.memoryUsedBytes = (uint64_t)mem_used_kb * 1024;
        }
    }

    // 3. EMMC 挂载状态
    int mounted = GetEmmcMountStatus();
    cpuStatusData.emmcMounted = (uint8_t)mounted;

    // 4. 磁盘使用率 & 磁盘剩余空间
    // const char *disk_path = mounted ? "/mnt/sdcard/" : "/oemdata/";
    const char *disk_path = "/oemdata/";
    struct statvfs stat;
    if (statvfs(disk_path, &stat) == 0 && stat.f_blocks > 0)
    {
        // 磁盘使用率 (%)
        unsigned long long used_blocks = stat.f_blocks - stat.f_bfree;
        double disk_usage_percent = 100.0 * used_blocks / stat.f_blocks;
        cpuStatusData.diskUsage = (float)disk_usage_percent;

        // 磁盘剩余空间 (KB)
        unsigned long long free_bytes = (unsigned long long)stat.f_bfree * stat.f_frsize;
        cpuStatusData.diskFreeSpaceKB = (uint32_t)(free_bytes / 1024);
    }

    // 5. 原始数据更新
    pthread_rwlock_wrlock(&g_monitoringDataRwlock);
    memcpy(&g_monitoringData.cpuStatusData, &cpuStatusData, sizeof(MonitorCPUStatusData_t));
    pthread_rwlock_unlock(&g_monitoringDataRwlock);

    // LOG_Debug("--cpuUsage:%6.2f%%  memoryUsage:%6.2f%%  memoryUsedBytes:%llu  diskUsage:%6.2f%%  diskFreeSpaceKB:%u  emmcMounted:%u",
    //           cpuStatusData.cpuUsage,
    //           cpuStatusData.memoryUsage,
    //           (unsigned long long)cpuStatusData.memoryUsedBytes,
    //           cpuStatusData.diskUsage,
    //           cpuStatusData.diskFreeSpaceKB,
    //           cpuStatusData.emmcMounted);
}

/**
 * @brief 原始数据接收回调初始化
 *
 * @return int32_t
 */
int32_t InitSignalIpcCallback()
{
    // TBOX静态信息初始化
    InitTboxStaticInfo();
    // 模组信息初始化
    HalModemAntInit();

    // 数据库信息回调
    // SubscriptionQueueMsg(SYS_QUE_TYPE_MONITOR_DBINFO_NOTIFY, DbInfoSyncCallback, NULL);
    // 电源状态数据回调
    SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, PmStatusCallback, NULL);
    // 硬线状态数据回调
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_KLINE, HardLineInfoCallback, NULL);
    // 备用电池数据回调
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_BATTERY, BakBatteryInfoCallback, NULL);
    // CAN信息数据回调
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_CAN, CanInfoSyncCallback, NULL);
#if 0
    // Mcu状态数据回调
    SubscriptionQueueMsg(SYS_QUE_TYPE_MONITOR_POWER_NOTIFY, McuStatusCallback, NULL); // 协议重拟，暂废
#endif
    // 定位数据回调
    SubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO, GnssInfoCallback, NULL);
    // 网络数据回调
    SubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY, NetWorkDataCallback, NULL);
    // 实车CAN数据回调
    SubscriptionQueueMsg((SYSQueMsgType_e)MonitorCanSignalData_canVal_t_msgid, CanSignalDataCallback, NULL);
    // CAN原始数据透传
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_VEHICLE_CAN_DATA, CanBusStatusCallback, NULL);

    // MCU重启ACK回调
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_REBOOT_MCU_REQ, RebootMcuAckCallback, NULL);

    // 更新资源使用率
    CreateResourceTimer(UpdateResourceUsage, 1000, NULL, &g_updateDatatimer);
    return RET_OK;
}

// 线程实现
// ******************************************************************************************************************************************************//

/**
 * @brief 用于判断是否满足监控-组包条件
 *
 * @return true
 * @return false
 */
static bool IsMonitoringAllowed(char *reason, int reasonLen)
{
    static MonitoringData_t rawData = {0};
    static struct timespec oldTime = {0, 0};
    static struct timespec newTime = {0, 0};

    clock_gettime(CLOCK_MONOTONIC, &newTime);
    if (abs(newTime.tv_sec - oldTime.tv_sec) >= 1)
    {
        // 控频，避免频繁调度
        oldTime = newTime;
        GetMonitoringData(&rawData);
        if (rawData.tboxDbInfo.sn[0] == '\0')
        {
            InitTboxStaticInfo();
            GetMonitoringData(&rawData);
        }
    }

    // 1. TBOX处于低功耗/整车熄火状态下不进行数据组包和监控
    if (rawData.powerDate.PmState == PM_STATE_SLEEP)
    {
        if (reason != NULL && reasonLen > 0)
            snprintf(reason, reasonLen, "Power mode is SLEEP");
        return false;
    }

    // 2. 未下产线，SN值为空的板子，不允许登录平台和组包数据上报
    if (strncmp(rawData.tboxDbInfo.sn, "0000", 4) == 0 ||
        rawData.tboxDbInfo.sn[0] == '\0')
    {
        if (reason != NULL && reasonLen > 0)
            snprintf(reason, reasonLen, "SN is NULL or starts with '0000'");
        return false;
    }

    if (reason != NULL && reasonLen > 0)
        snprintf(reason, reasonLen, "OK");
    return true;
}

/**
 * @brief 组包触发事件和恢复事件
 *
 */
static void PackageTriggeringEvent(const MonitoringData_t rawData)
{
    for (int idx = 0; idx < sizeof(g_triggerEventGather) / sizeof(g_triggerEventGather[0]); idx++)
    {
        // LOG_Debug("idx:%d status:%d", idx, g_triggerEventGather[idx].eventStatus);
        if (EventStatus::EVENT_STATUS_TRIGGER == g_triggerEventGather[idx].eventStatus)
        {
            if (NULL != g_triggerEventGather[idx].triggerHandleFunc)
                g_triggerEventGather[idx].triggerHandleFunc(rawData);
            else
                LOG_Warn("idx:%d triggerHandleFunc is NULL", idx);
        }
        else if (EventStatus::EVENT_STATUS_RECOVERY == g_triggerEventGather[idx].eventStatus)
        {
            if (NULL != g_triggerEventGather[idx].recoveryHandleFunc)
                g_triggerEventGather[idx].recoveryHandleFunc(rawData);
            else
                LOG_Warn("idx:%d recoveryHandleFunc is NULL", idx);
        }
    }
    return;
}

/**
 * @brief (触发/恢复)事件监控
 *
 */
static void EventMonitoring(const MonitoringData_t rawData)
{
    for (int idx = 0; idx < sizeof(g_triggerEventGather) / sizeof(g_triggerEventGather[0]); idx++)
    {
        if (NULL != g_triggerEventGather[idx].monitorFuncCallback)
            g_triggerEventGather[idx].eventStatus = g_triggerEventGather[idx].monitorFuncCallback(rawData);
        // else
        //     LOG_Warn("idx:%d monitorFuncCallback is NULL", idx);
    }
    return;
}

/**
 * @brief TBOX运行状态监控
 *
 */
static void StatusMonitoring()
{
    // 获取（更新）原始数据
    MonitoringData_t rawData;
    GetMonitoringData(&rawData);

    // 开启状态监控（数据组包）的条件
    if (rawData.powerDate.PmState == PM_STATE_SLEEP)
    {
        return;
    }

    // printf("--cpuUsage:%6.2f%%  memoryUsage:%6.2f%%  memoryUsedBytes:%llu  "
    //        "diskUsage:%6.2f%%  diskFreeSpaceKB:%u  "
    //        "emmcMounted:%u\n",
    //        rawData.cpuStatusData.cpuUsage,
    //        rawData.cpuStatusData.memoryUsage,
    //        (unsigned long long)rawData.cpuStatusData.memoryUsedBytes,
    //        rawData.cpuStatusData.diskUsage,
    //        rawData.cpuStatusData.diskFreeSpaceKB,
    //        rawData.cpuStatusData.emmcMounted);

    // 状态监控
    EventMonitoring(rawData);

    // 触发数据组包
    PackageTriggeringEvent(rawData);

    // 周期数据组包
    static long long nowMs = 0;
    static long long lastPackTimeMs = 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    nowMs = (long long)(tv.tv_sec) * 1000ll + tv.tv_usec / 1000; // ms
    if (nowMs - lastPackTimeMs > PACKAGE_INTERVAL_MS || lastPackTimeMs - nowMs > PACKAGE_INTERVAL_MS)
    {
        lastPackTimeMs += PACKAGE_INTERVAL_MS;
        if (nowMs - lastPackTimeMs > 3 * PACKAGE_INTERVAL_MS ||
            lastPackTimeMs - nowMs > 3 * PACKAGE_INTERVAL_MS)
        {
            // 严重滞后(>3周期)：放弃追赶历史欠账，避免连续调度导致CPU风暴。
            lastPackTimeMs = (nowMs / 1000) * 1000;
        }
        // 处理组包
        PeriodicPack(rawData);
    }

    pthread_rwlock_wrlock(&g_monitoringDataRwlock);
    g_monitoringData.mcuStatusData.canReceiverStatus = CAN_STATUS_NO_RESPONSE;
    g_monitoringData.positioningData.hasGpsRawData = 0;
    pthread_rwlock_unlock(&g_monitoringDataRwlock);

    return;
}

// 线程
// ******************************************************************************************************************************************************//

/**
 * @brief TBOX状态监控+组包线程
 *
 * @param args
 * @return void*
 */
static void *StartRunStatusMonitoring(void *args)
{
    prctl(PR_SET_NAME, "MonitorPackage");

    static long long newTime = 0;
    static long long oldTime = 0;
    struct timeval tv;

    /*获取周期上报间隔INTERVAL_MS-功能待确认*/

    while (1)
    {
        char reason[128] = "";
        if (!IsMonitoringAllowed(reason, sizeof(reason)))
        {
            // 不满足监控-组包条件
            printf("%s() stop monitoring. reason:%s\n", __func__, reason);
            LOG_Warn("%s() stop monitoring. reason:%s", __func__, reason);
            usleep(500 * 1000);
            continue;
        }

        // 控频下的状态监控
        gettimeofday(&tv, NULL);
        newTime = (long long)(tv.tv_sec) * 1000ll + tv.tv_usec / 1000; // ms
        if (newTime - oldTime > INTERVAL_MS || oldTime - newTime > INTERVAL_MS)
        {
            oldTime += INTERVAL_MS;
            if (newTime - oldTime > 3 * INTERVAL_MS || oldTime - newTime > 3 * INTERVAL_MS)
            {
                // 严重滞后(>3周期)：放弃追赶历史欠账，避免连续调度导致CPU风暴。
                oldTime = (newTime / 1000) * 1000;
            }
            StatusMonitoring();
        }
        else
        {
            usleep(100 * 1000);
        }
    }
    return NULL;
}

/**
 * @brief 补发数据处理线程
 *
 * @param args 未使用
 * @return void*
 */
static void *ReissueDataProcessing(void *args)
{
    prctl(PR_SET_NAME, "GetReissue");
    const int CHECK_INTERVAL_MS = 100;

    while (1)
    {
        if (!IsMonitoringAllowed(NULL, 0))
        {
            // 不满足监控-组包条件
            usleep(5LL * CHECK_INTERVAL_MS * 1000);
            continue;
        }

        if (GetQueueDataCnt(4) == 0)
        {
            LoadData_t reissueData = {0};
            if (CacheGet(&reissueData) == RET_OK)
            {
                if (reissueData.len > 0)
                {
                    // 加入 P4 补发队列
                    reissueData.priority = 4;
                    AddQueueData(reissueData, 4);
                    printf("%s() loaded one data from DB to P4 queue, data_len=%zu\n", __func__, reissueData.len);
                }
            }
            else
            {
                LOG_Error("%s() Get Reissue Data Error", __func__);
            }
        }

        usleep(CHECK_INTERVAL_MS * 1000);
    }
    return NULL;
}

/**
 * @brief 开始数据组包
 *
 * @return int
 */
int DataPackagingStart()
{
    int ret = -1;
    MonitorConfigPath cfgPath = {0};
    // 待发送队列初始化
    InitSendQueue();
    // 获取配置地址
    MonitorConfig_Get(&cfgPath);
    // 初始化当前数据库
    InitAllDatabases(cfgPath.dataFilePath);

    ret = pthread_create(&g_monitoringId, NULL, StartRunStatusMonitoring, NULL);
    if (ret < 0)
    {
        LOG_Error("pthread_create err!");
        return RET_ERR;
    }

    ret = pthread_create(&g_reissueId, NULL, ReissueDataProcessing, NULL);
    if (ret < 0)
    {
        LOG_Error("pthread_create err!");
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @brief 结束数据组包
 *
 * @return int
 */
int DataPackagingStop()
{
    int ret;

    // 1. 停止定时器，防止回调访问已销毁资源
    if (g_updateDatatimer != NULL)
    {
        timer_delete(g_updateDatatimer);
        g_updateDatatimer = NULL;
        LOG_Debug("Resource update timer deleted.");
    }

    // 2. 等待监控线程退出
    if (g_monitoringId != 0)
    {
        ret = pthread_join(g_monitoringId, NULL);
        if (ret != 0)
        {
            LOG_Error("Failed to join <StartRunStatusMonitoring> thread, ret = %d", ret);
            return RET_ERR;
        }
    }

    // 3. 等待补发线程退出
    if (g_reissueId != 0)
    {
        ret = pthread_join(g_reissueId, NULL);
        if (ret != 0)
        {
            LOG_Error("Failed to join <ReissueDataProcessing> thread, ret = %d", ret);
            return RET_ERR;
        }
    }

    // 4. 关闭数据库连接（修复资源泄漏）
    CloseAllDatabases();

    // 5. 销毁发送队列
    DeinitSendQueue();
    return RET_OK;
}

// 数据监控函数
// ******************************************************************************************************************************************************//

/**
 * @brief 1、TBOX备用电池低压监控
 * 触发条件：连续10次采样（间隔1秒）电压均低于阈值，且持续5秒
 * 恢复条件：连续10次采样电压均高于恢复阈值（如>3.3V），且持续10秒
 * @return EventStatus TRIGGER/RECOVERY/NORMAL
 */
static EventStatus MonitorBatteryLowVoltage(const MonitoringData_t rawData)
{
    static int lowCount = 0;  // 低压连续计数
    static int highCount = 0; // 高压连续计数
    static EventStatus currentStatus = InitStateFlagFromDisk(1, NULL, 0);

    const float LOW_THRESHOLD = 3.0f;   // 触发阈值 (V)
    const float RECOV_THRESHOLD = 3.3f; // 恢复阈值 (V)

    if (rawData.mcuStatusData.backupBatteryVoltage < LOW_THRESHOLD)
    {
        // 低压状态：累加低压计数，清零高压计数
        lowCount++;
        highCount = 0;

        // 连续5次低压，且当前未处于触发状态，则上报触发事件
        if (lowCount >= 5)
        {
            if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
            {
                currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
                lowCount = 0;
                highCount = 0;
                return EventStatus::EVENT_STATUS_TRIGGER;
            }
        }
    }
    else if (rawData.mcuStatusData.backupBatteryVoltage > RECOV_THRESHOLD)
    {
        // 高压状态：累加高压计数，清零低压计数
        highCount++;
        lowCount = 0;

        // 连续10次高压，且当前处于触发状态，则上报恢复事件
        if (highCount >= 10)
        {
            if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
            {
                currentStatus = EventStatus::EVENT_STATUS_NORMAL;
                lowCount = 0;
                highCount = 0;
                return EventStatus::EVENT_STATUS_RECOVERY;
            }
        }
    }
    else
    {
        // 电压介于阈值之间：重置两个计数
        lowCount = 0;
        highCount = 0;
    }

    // 无事件发生，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 2、GPS信号丢失状态监控
 * 触发条件：满足以下任一条件且持续30秒：
 * (1) 连续30秒定位状态为无效；
 * (2) 连续30秒可见卫星数 < 4颗
 * 恢复条件：满足以下任一条件且持续10秒：
 * (1) 连续10秒定位有效；
 * (2) 连续10秒可见卫星数 ≥ 4颗
 * @return EventStatus TRIGGER/RECOVERY/NORMAL
 */
static EventStatus MonitorGpsSignalLost(const MonitoringData_t rawData)
{
    static int lostCount = 0;  // 信号丢失状态连续计数
    static int validCount = 0; // 信号有效状态连续计数
    static EventStatus currentStatus = InitStateFlagFromDisk(2, NULL, 0);

    const int TRIGGER_COUNT = (30000 + INTERVAL_MS - 1) / INTERVAL_MS; // 触发阈值（30秒）
    const int RECOV_COUNT = (10000 + INTERVAL_MS - 1) / INTERVAL_MS;   // 恢复阈值（10秒）
    const uint8_t MIN_VISIBLE = 4;                                     // 最小可见卫星数

    // 判断当前是否为信号丢失状态：定位无效 或 可见卫星数不足
    bool isLost = (!rawData.positioningData.locationInfo.gpsState) ||
                  (rawData.positioningData.locationInfo.num_svs < MIN_VISIBLE);

    if (isLost)
    {
        // 信号丢失：累加丢失计数，清零有效计数
        lostCount++;
        validCount = 0;

        if (lostCount >= TRIGGER_COUNT)
        {
            // 触发条件满足，且当前未触发，则上报触发事件
            if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
            {
                currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
                lostCount = 0;
                validCount = 0;
                return EventStatus::EVENT_STATUS_TRIGGER;
            }
        }
    }
    else
    {
        // 信号有效：累加有效计数，清零丢失计数
        validCount++;
        lostCount = 0;

        if (validCount >= RECOV_COUNT)
        {
            // 恢复条件满足，且当前处于触发状态，则上报恢复事件
            if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
            {
                currentStatus = EventStatus::EVENT_STATUS_NORMAL;
                lostCount = 0;
                validCount = 0;
                return EventStatus::EVENT_STATUS_RECOVERY;
            }
        }
    }

    // 无事件发生，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 3、CAN总线通讯中断监控
 * 触发条件：满足以下任一条件：
 * (1) CAN控制器进入Bus Off状态；
 * (2) IGN ON且连续100ms未收到任何CAN报文
 * 恢复条件：满足以下任一条件且持续10ms：
 * (1) CAN控制器退出Bus Off状态；
 * (2) 连续收到3帧有效CAN报文
 * @return EventStatus TRIGGER/RECOVERY/NORMAL
 */
static EventStatus MonitorCanBusCommInterrupt(const MonitoringData_t rawData)
{
    static int noFrameCount = 0;        // 无CAN报文计数
    static int busOffRecoverCount = 0;  // 退出Bus Off恢复计数
    static int validFrameCount = 0;     // 连续有效帧计数
    static int validFrameTimeCount = 0; // 连续有效帧持续时间计数
    static EventStatus currentStatus = InitStateFlagFromDisk(3, NULL, 0);

    const int NO_FRAME_COUNT = (100 + INTERVAL_MS - 1) / INTERVAL_MS;
    const int RECOVER_TIME_COUNT = (10 + INTERVAL_MS - 1) / INTERVAL_MS;

    // ========== 1. 触发条件判断 ==========
    if (rawData.mcuStatusData.canBusOff)
    {
        // Bus Off 触发
        if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
        {
            // 未触发时，上报触发事件
            currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
            noFrameCount = 0;
            busOffRecoverCount = 0;
            validFrameCount = 0;
            validFrameTimeCount = 0;
            return EventStatus::EVENT_STATUS_TRIGGER;
        }
        // 已触发，无需重复上报
    }
    else if (rawData.mcuStatusData.ignOn &&
             (uint8_t)CanStatus::CAN_STATUS_NO_RESPONSE == rawData.mcuStatusData.canReceiverStatus)
    {
        // IGN ON 且无报文触发
        noFrameCount++;
        busOffRecoverCount = 0;
        validFrameCount = 0;
        validFrameTimeCount = 0;

        if (noFrameCount >= NO_FRAME_COUNT)
        {
            if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
            {
                // 未触发时，上报触发事件
                currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
                noFrameCount = 0;
                busOffRecoverCount = 0;
                validFrameCount = 0;
                validFrameTimeCount = 0;
                return EventStatus::EVENT_STATUS_TRIGGER;
            }
        }
        else
        {
            // 尚未达到触发阈值，继续监控
            return EventStatus::EVENT_STATUS_NORMAL;
        }
    }
    else
    {
        // ========== 2. 无触发条件，尝试恢复 ==========
        // 有报文或IGN OFF，清零无报文计数
        noFrameCount = 0;

        bool recover = false;

        // 恢复条件1：退出Bus Off持续10ms
        if (!rawData.mcuStatusData.canBusOff)
        {
            busOffRecoverCount++;
            if (busOffRecoverCount >= RECOVER_TIME_COUNT)
            {
                busOffRecoverCount = RECOVER_TIME_COUNT;
                recover = true;
            }
        }
        else
        {
            busOffRecoverCount = 0;
        }

        // 恢复条件2：连续收到3帧有效CAN报文且持续10ms
        if ((uint8_t)CanStatus::CAN_STATUS_NORMAL == rawData.mcuStatusData.canReceiverStatus)
        {
            validFrameCount++;
            if (validFrameCount >= 3)
            {
                validFrameTimeCount++;
                if (validFrameTimeCount >= RECOVER_TIME_COUNT)
                {
                    recover = true;
                }
            }
            else
            {
                validFrameTimeCount = 0;
            }
        }
        else
        {
            validFrameCount = 0;
            validFrameTimeCount = 0;
        }

        // 如果恢复条件满足且当前处于触发状态，则上报恢复事件
        if (recover && currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_NORMAL;
            noFrameCount = 0;
            busOffRecoverCount = 0;
            validFrameCount = 0;
            validFrameTimeCount = 0;
            return EventStatus::EVENT_STATUS_RECOVERY;
        }
    }

    // 无事件发生，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 4、车辆意外高压下电监控
 * 触发条件：同时满足以下所有条件：
 * (1) 车辆状态为Ready；
 * (2) 前3秒内车速 > 5 km/h；
 * (3) 电机转速 < 50 rpm持续超过2秒；
 * (4) 高压系统状态变为“断开”或“故障”；
 * (5) 无驾驶员主动下电信号（如无熄火请求）// 需求不明确，歧义，暂弃
 * 恢复条件：满足以下任一条件：
 * (1) 电机转速 > 500 rpm持续3秒；
 * (2) 车速 > 0 km/h持续3秒；
 * (3) 车辆状态变为OFF
 * @return EventStatus TRIGGER/RECOVERY/NORMAL
 */
static EventStatus MonitorVehicleUnexpectedHvPowerDown(const MonitoringData_t rawData)
{
    static int motorLowCount = 0; // 电机转速<50rpm的连续计数
    static int recoveryCount = 0; // 恢复条件（转速>500或车速>0）的连续计数
    static EventStatus currentStatus = InitStateFlagFromDisk(4, NULL, 0);

    const int MOTOR_LOW_TIME_MS = 2000;
    const int MOTOR_LOW_COUNT = (MOTOR_LOW_TIME_MS + INTERVAL_MS - 1) / INTERVAL_MS;

    const int RECOV_TIME_MS = 3000;
    const int RECOV_COUNT = (RECOV_TIME_MS + INTERVAL_MS - 1) / INTERVAL_MS;

    // 判断前3秒内是否有车速>5km/h
    bool speedHighPast = g_speedHistory.isHistoryFull() && g_speedHistory.anyInLast3SecondsGreaterThan(5.0f);

    // 高压断开或故障标志
    bool hvDisconnected = (rawData.canSignalData.hvSystemStatus == HighVoltageSystemState::HV_SYSTEM_STATE_DISCONNECTED ||
                           rawData.canSignalData.hvSystemFltLv != 0);

    // 车辆OFF标志
    bool vehicleOff = (rawData.canSignalData.vehiclePowerState == VehiclePowerState::PEPS_OFF);

    // // 无驾驶员主动下电信号（如无熄火请求）
    // bool noDriverActivePowerOff = (rawData.canSignalData.driverOffSeat && vehicleOff);

    // ==================== 触发条件判断 ====================
    bool triggerConditionsMet = (rawData.canSignalData.isReady &&
                                 speedHighPast &&
                                 rawData.canSignalData.motorSpeed < 50 &&
                                 hvDisconnected); // && noDriverActivePowerOff

    if (triggerConditionsMet)
    {
        motorLowCount++;
        recoveryCount = 0; // 触发条件满足时清零恢复计数

        if (motorLowCount >= MOTOR_LOW_COUNT)
        {
            // 触发条件持续满足2秒，上报触发事件
            motorLowCount = 0; // 清零，避免重复触发
            if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
            {
                currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
                return EventStatus::EVENT_STATUS_TRIGGER;
            }
        }
    }
    else
    {
        motorLowCount = 0; // 任一条件不满足，中断触发计数
    }

    // ==================== 恢复条件判断 ====================
    // 只有在当前已触发的情况下才检查恢复条件
    if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
    {
        // 恢复条件1：电机转速>500rpm持续3秒
        // 恢复条件2：车速>0km/h持续3秒
        bool recoveryConditionsMet = (rawData.canSignalData.motorSpeed > 500) ||
                                     (rawData.canSignalData.speed > 0.0f);

        if (recoveryConditionsMet)
        {
            recoveryCount++;
            if (recoveryCount >= RECOV_COUNT)
            {
                // 持续满足恢复条件3秒，上报恢复事件
                recoveryCount = 0;
                motorLowCount = 0;
                currentStatus = EventStatus::EVENT_STATUS_NORMAL;
                return EventStatus::EVENT_STATUS_RECOVERY;
            }
        }
        else
        {
            recoveryCount = 0; // 条件不满足，中断恢复计数
        }

        // 恢复条件3：车辆状态变为OFF（立即恢复）
        if (vehicleOff)
        {
            recoveryCount = 0;
            motorLowCount = 0;
            currentStatus = EventStatus::EVENT_STATUS_NORMAL;
            return EventStatus::EVENT_STATUS_RECOVERY;
        }
    }
    else
    {
        // 当前未触发，任何恢复条件都无意义，清零恢复计数
        recoveryCount = 0;
    }

    // 无状态变化，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 5、行驶中异常减速监控
 * 触发条件：同时满足：
 * (1) 当前车速 > 30 km/h；
 * (2) 减速度 > 0.5g（约4.9 m/s²）持续超过0.5秒
 * 恢复条件：减速度 < 0.4g（约3.9 m/s²）持续超过0.5秒 或 车速 ≤ 30 km/h
 * @return EventStatus TRIGGER/RECOVERY/NORMAL
 */
static EventStatus MonitorDrivingDecelerationPowerLoss(const MonitoringData_t rawData)
{
    static int decelHighCount = 0;   // 高减速度持续时间计数（对应负加速度绝对值大）
    static int decelNormalCount = 0; // 低减速度持续时间计数
    static EventStatus currentStatus = InitStateFlagFromDisk(5, NULL, 0);

    const float SPEED_THRESHOLD = 30.0f; // 车速阈值 km/h
    const float DECEL_HIGH = 4.9f;       // 触发减速度绝对值阈值 m/s² (0.5g)
    const float DECEL_NORMAL = 3.9f;     // 恢复减速度绝对值阈值 m/s² (0.4g)
    const int DECEL_TIME_MS = 500;       // 持续时间要求 ms
    const int DECEL_COUNT = (DECEL_TIME_MS + INTERVAL_MS - 1) / INTERVAL_MS;

    // 获取当前加速度（正值加速，负值减速）
    float acceleration = g_speedHistory.getSpeedDifference(INTERVAL_MS / 1000.0f);

    // 判断是否为高减速度（减速，且绝对值大于阈值）
    bool isHighDeceleration = g_speedHistory.isHistoryFull() && (acceleration < -DECEL_HIGH);

    // ==================== 触发条件判断 ====================
    if (rawData.canSignalData.speed > SPEED_THRESHOLD && isHighDeceleration)
    {
        decelHighCount++;
        decelNormalCount = 0;
        if (decelHighCount >= DECEL_COUNT)
        {
            if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
            {
                currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
                decelHighCount = 0;
                decelNormalCount = 0;
                return EventStatus::EVENT_STATUS_TRIGGER;
            }
        }
    }
    else
    {
        decelHighCount = 0;
    }

    // ==================== 恢复条件判断 ====================
    if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
    {
        // 恢复条件1：减速度绝对值低于阈值持续0.5秒（即加速度 > -DECEL_NORMAL）
        bool isLowDeceleration = (acceleration > -DECEL_NORMAL);
        if (isLowDeceleration)
        {
            decelNormalCount++;
            if (decelNormalCount >= DECEL_COUNT)
            {
                currentStatus = EventStatus::EVENT_STATUS_NORMAL;
                decelHighCount = 0;
                decelNormalCount = 0;
                return EventStatus::EVENT_STATUS_RECOVERY;
            }
        }
        else
        {
            decelNormalCount = 0;
        }

        // 恢复条件2：车速 ≤ 30 km/h（立即恢复）
        if (rawData.canSignalData.speed <= SPEED_THRESHOLD)
        {
            currentStatus = EventStatus::EVENT_STATUS_NORMAL;
            decelHighCount = 0;
            decelNormalCount = 0;
            return EventStatus::EVENT_STATUS_RECOVERY;
        }
    }
    else
    {
        decelNormalCount = 0;
    }

    // 无事件发生，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 6、车辆高压上电失败监控
 * 触发条件：同时满足：
 * (1) IGN从OFF变为ON；
 * (2) 低压蓄电池电压正常（>9V）；
 * (3) 尝试上电后10秒内车辆状态未进入Ready；
 * (4) 重复尝试超过2次
 * 恢复条件：车辆状态变为Ready，或IGN状态变为OFF
 * @return EventStatus TRIGGER/RECOVERY/NORMAL
 */
static EventStatus MonitorVehicleHvPowerOnFail(const MonitoringData_t rawData)
{
    static bool ignPrev = false;            // 用于上升沿检测（触发条件）
    static bool ignPrevForRecovery = false; // 用于下降沿检测（恢复条件）
    static bool firstCall = true;
    static int timeCount = 0;
    static EventStatus currentStatus = InitStateFlagFromDisk(6, NULL, 0);

    const float LOW_BATTERY_OK = 9.0f;
    const int TIMEOUT_MS = 10000;
    const int TIMEOUT_COUNT = (TIMEOUT_MS + INTERVAL_MS - 1) / INTERVAL_MS;
    // const int RETRY_LIMIT = 2;

    // ---------- 首次调用：初始化边沿状态 ----------
    if (firstCall)
    {
        ignPrev = rawData.mcuStatusData.ignOn;
        ignPrevForRecovery = rawData.mcuStatusData.ignOn;
        firstCall = false;
        return EventStatus::EVENT_STATUS_NORMAL;
    }

    // ---------- 边沿检测 ----------
    bool ignRising = (rawData.mcuStatusData.ignOn && !ignPrev);             // OFF → ON
    bool ignFalling = (!rawData.mcuStatusData.ignOn && ignPrevForRecovery); // ON → OFF

    // 更新边沿记忆
    ignPrev = rawData.mcuStatusData.ignOn;
    ignPrevForRecovery = rawData.mcuStatusData.ignOn;

    bool lowBatteryOk = (rawData.mcuStatusData.vehicleBatteryVoltage > LOW_BATTERY_OK);
    bool notReady = !rawData.canSignalData.isReady;

    // ========== 1. 触发逻辑：上升沿时重置计时器 ==========
    if (ignRising && lowBatteryOk)
    {
        timeCount = 0; // 新的一次上电尝试开始
    }

    // ========== 2. 超时触发判断 ==========
    if (rawData.mcuStatusData.ignOn && lowBatteryOk && notReady)
    {
        timeCount++;
        // if (timeCount >= TIMEOUT_COUNT && rawData.canSignalData.highVoltageFailureCount >= RETRY_LIMIT)
        if (timeCount >= TIMEOUT_COUNT)
        {
            if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
            {
                currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
                timeCount = 0;
                return EventStatus::EVENT_STATUS_TRIGGER;
            }
        }
    }
    else
    {
        timeCount = 0;
    }

    // ========== 3. 恢复逻辑：Ready 为真 或 下降沿 ==========
    if (rawData.canSignalData.isReady || ignFalling)
    {
        if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_NORMAL;
            return EventStatus::EVENT_STATUS_RECOVERY;
        }
    }

    // 无事件发生，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 7、车辆无法与CAN通信监控
 * 触发条件：满足以下任一条件且持续60秒：
 * （1）CAN控制器反复进入Bus Off且无法恢复
 * （2）发送CAN报文无应答或ACK错误
 * （3）CAN收发器故障指示(暂无)
 * 恢复条件：条件消失后自动恢复
 * @return EventStatus TRIGGER/NORMAL/RECOVERY
 */
static EventStatus MonitorVehicleCanCommFail(const MonitoringData_t rawData)
{
    static int noFrameCount = 0;    // 故障帧计数
    static int validFrameCount = 0; // 恢复帧计数
    static EventStatus currentStatus = InitStateFlagFromDisk(7, NULL, 0);
    if (currentStatus == EVENT_STATUS_TRIGGER)
        g_faultStatus.VehicleCanCommFail.store(true);

    const int NO_FRAME_TIME_MS = 60000; // 改为 60 秒
    const int NO_FRAME_COUNT = (NO_FRAME_TIME_MS + INTERVAL_MS - 1) / INTERVAL_MS;

    // 触发条件判断：（1）CAN接收器状态异常(含CAN控制器Bus Off、发送CAN报文无应答或ACK错误)（2）CAN收发器故障指示(陕汽暂无,默认为0)
    if ((rawData.mcuStatusData.ignOn && (uint8_t)CanStatus::CAN_STATUS_NORMAL != rawData.mcuStatusData.canReceiverStatus) ||
        rawData.mcuStatusData.canReceiverFaultIndication)
    {
        noFrameCount++;
        validFrameCount = 0;

        if (noFrameCount >= NO_FRAME_COUNT)
        {
            // 触发条件满足，且当前未触发，则上报触发事件
            if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
            {
                currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
                noFrameCount = 0;
                validFrameCount = 0;
                g_faultStatus.VehicleCanCommFail.store(true);
                return EventStatus::EVENT_STATUS_TRIGGER;
            }
        }
    }
    else
    {
        // 条件不满足，清零触发计数
        noFrameCount = 0;

        // 如果之前处于触发状态，现在故障消失，则上报恢复事件
        if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
        {
            validFrameCount++;
            if (validFrameCount >= NO_FRAME_COUNT) // 连续正常达到阈值才恢复
            {
                currentStatus = EventStatus::EVENT_STATUS_NORMAL;
                validFrameCount = 0;
                g_faultStatus.VehicleCanCommFail.store(false);
                return EventStatus::EVENT_STATUS_RECOVERY;
            }
        }
    }

    // 无事件发生，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 8、GPS模块故障监控
 * 触发条件：满足以下任一条件：
 * （1）GPS模块连续3次复位后仍无法正常响应
 * （2）连续30秒无法读取到GPS模块的任何输出
 * （3）GPS模块返回明确的硬件错误状态码
 * 恢复条件：现改为：条件消失后自动恢复
 * @return EventStatus TRIGGER/NORMAL/RECOVERY
 */
static EventStatus MonitorGpsModuleFault(const MonitoringData_t rawData)
{
    static int noRawDataCount = 0; // 故障帧计数
    static int validCount = 0;     // 恢复帧计数
    // 从磁盘加载上次保存的事件状态（如为 TRIGGER 则继续维持触发状态）
    static EventStatus currentStatus = InitStateFlagFromDisk(8, NULL, 0);
    if (currentStatus == EVENT_STATUS_TRIGGER)
        g_faultStatus.GpsModuleFault.store(true);

    const int NO_RAW_DATA_TIME_MS = 30000; // 连续无数据超时时间
    const int NO_RAW_DATA_COUNT = (NO_RAW_DATA_TIME_MS + INTERVAL_MS - 1) / INTERVAL_MS;

    // 更新无原始数据计数
    if (1 != rawData.positioningData.hasGpsRawData)
    {
        noRawDataCount++;
        // 避免计数器溢出（达到阈值后不再累加）
        if (noRawDataCount > NO_RAW_DATA_COUNT)
            noRawDataCount = NO_RAW_DATA_COUNT;
    }
    else
    {
        noRawDataCount = 0;
    }
    // printf("@@@@@@@@@@@@@@@@@@@@@@@@@ GPS noRawDataCount:%d\n", noRawDataCount);

    // 触发条件判断
    bool trigger = false;
    // 满足以下任一条件：（1）GPS模块连续3次复位后仍无法正常响应（2）连续30秒无法读取到GPS模块的任何输出（3）GPS模块返回明确的硬件错误状态码
    if ((rawData.positioningData.gpsModuleResetTimes >= 3 && 1 != rawData.positioningData.hasGpsRawData) ||
        noRawDataCount >= NO_RAW_DATA_COUNT ||
        HalGnssAntStatus_e::E_GNSS_ANT_STATUS_CIRCUIT_NORMAL != rawData.positioningData.locationInfo.antStatus)
    {
        trigger = true;
    }

    if (trigger)
    {
        validCount = 0; // 故障存在，清零恢复计数器
        if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
            noRawDataCount = 0;
            g_faultStatus.GpsModuleFault.store(true);
            return EventStatus::EVENT_STATUS_TRIGGER;
        }
    }
    else
    {
        // 故障消失，但需要连续确认
        if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
        {
            validCount++;
            if (validCount >= NO_RAW_DATA_COUNT) // 保证条件（1）和（2）采集的数据充足
            {
                currentStatus = EventStatus::EVENT_STATUS_NORMAL;
                g_faultStatus.GpsModuleFault.store(false);
                return EventStatus::EVENT_STATUS_RECOVERY;
            }
        }
    }

    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 9、小电池无法充电监控
 * 触发条件：同时满足以下所有条件，且持续30分钟：
 * （1） KL30电压正常（> 11V，确认车辆主电源供电正常）
 * （2） 小电池电压 < 3.2V（低于正常充电区间）
 * （3） 充电管理芯片状态指示“充电中”但电压未上升（如有状态反馈）
 * （4） 小电池电压在30分钟内未上升超过 0.1V（确认充电无效）
 * 恢复条件：满足以下任一条件：
 * （1） 小电池电压 > 3.3V 且持续10分钟
 * （2） 小电池电压在30分钟内上升超过 0.2V
 * （3） KL30断电后重新上电（阈值0.5V），且充电恢复正常（废弃！！！）
 * @return EventStatus TRIGGER/RECOVERY/NORMAL
 */
static EventStatus MonitorBatteryChargeFail(const MonitoringData_t rawData)
{
    static EventStatus currentStatus = InitStateFlagFromDisk(9, NULL, 0);
    if (currentStatus == EVENT_STATUS_TRIGGER)
        g_faultStatus.BatteryChargeFail.store(true);

    static bool kl30PowerPrev = false; // 上一次KL30是否有电（>0.5V）

    const float KL30_OK_THRESHOLD = 11.0f;      // KL30正常阈值（V）
    const float KL30_POWER_ON_THRESHOLD = 0.5f; // KL30重新上电检测阈值（V）
    const float BATTERY_LOW_THRESHOLD = 3.2f;
    const float BATTERY_RECOV_THRESHOLD = 3.3f;
    const float VOLTAGE_RISE_MIN_TRIGGER = 0.1f;
    const float VOLTAGE_RISE_MIN_RECOV = 0.2f;

    // 1. 获取小电池历史信息
    float voltageRise = g_backupBatteryVoltageHistory.getVoltageRiseInLast30Minutes();
    bool backupHistoryFull = g_backupBatteryVoltageHistory.isHistoryFull();

    // 2. 获取KL30历史信息（持续30分钟正常）
    bool kl30SustainedNormal = g_vehicleBatteryVoltageHistory.isVoltageAboveThresholdForLastMinutes(KL30_OK_THRESHOLD, 30);
    bool kl30HistoryFull = g_vehicleBatteryVoltageHistory.isHistoryFull();

    // 3. 小电池持续低压判断（持续30分钟异常）
    bool voltageSustainedLow = g_backupBatteryVoltageHistory.isVoltageBelowThresholdForLastMinutes(BATTERY_LOW_THRESHOLD, 30);

    // 4. 触发条件判断
    if (kl30HistoryFull && backupHistoryFull &&
        kl30SustainedNormal &&
        voltageSustainedLow &&
        (voltageRise < VOLTAGE_RISE_MIN_TRIGGER))
    {
        if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
            g_faultStatus.BatteryChargeFail.store(true);
            return EventStatus::EVENT_STATUS_TRIGGER;
        }
    }

    // 5. 恢复条件判断
    if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
    {
        bool recover = false;

        // 恢复条件(1)：小电池电压 > 3.3V 持续10分钟
        bool voltageSustainedHigh = g_backupBatteryVoltageHistory.isVoltageAboveThresholdForLastMinutes(BATTERY_RECOV_THRESHOLD, 10);
        if (voltageSustainedHigh)
            recover = true;

        // 恢复条件(2)：30分钟内电压上升 > 0.2V
        if (backupHistoryFull && voltageRise > VOLTAGE_RISE_MIN_RECOV)
            recover = true;

        // // 恢复条件(3)：KL30断电后重新上电（从0V->>0V的上升沿）
        // bool kl30PowerNow = (rawData.mcuStatusData.vehicleBatteryVoltage > KL30_POWER_ON_THRESHOLD);
        // bool kl30PowerOnRising = (kl30PowerNow && !kl30PowerPrev);
        // kl30PowerPrev = kl30PowerNow;
        // if (kl30PowerOnRising)
        //     recover = true;

        if (recover)
        {
            currentStatus = EventStatus::EVENT_STATUS_NORMAL;
            g_faultStatus.BatteryChargeFail.store(false);
            return EventStatus::EVENT_STATUS_RECOVERY;
        }
    }
    else
    {
        // 非触发状态，更新KL30电源状态静态变量，以便下次上升沿检测
        bool kl30PowerNow = (rawData.mcuStatusData.vehicleBatteryVoltage > KL30_POWER_ON_THRESHOLD);
        kl30PowerPrev = kl30PowerNow;
    }

    // 无事件发生，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 10、车辆启动状态监控
 * 触发条件：车辆进入Ready状态
 * 恢复条件：车辆退出Ready状态
 * @return EventStatus TRIGGER/RECOVERY/NORMAL
 */
static EventStatus MonitorVehicleStart(const MonitoringData_t rawData)
{
    static EventStatus currentStatus = EventStatus::EVENT_STATUS_NORMAL; // 当前状态，初始无事件

    if (rawData.canSignalData.keyStatus == 0x01)
    {
        if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
            return EventStatus::EVENT_STATUS_TRIGGER;
        }
    }
    else
    {
        if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_NORMAL;
            return EventStatus::EVENT_STATUS_RECOVERY;
        }
    }

    // 无事件发生，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 11、车辆速度大于0监控
 * 触发条件：车速 > 0 km/h
 * 恢复条件：车速 = 0 km/h
 * @return EventStatus TRIGGER/RECOVERY/NORMAL
 */
static EventStatus MonitorVehicleSpeedGreater0(const MonitoringData_t rawData)
{
    static EventStatus currentStatus = EventStatus::EVENT_STATUS_NORMAL; // 当前状态，初始无事件

    if (rawData.canSignalData.speed > 0.0f)
    {
        if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
            return EventStatus::EVENT_STATUS_TRIGGER;
        }
    }
    else
    {
        if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_NORMAL;
            return EventStatus::EVENT_STATUS_RECOVERY;
        }
    }

    // 无事件发生，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 12、车辆速度大于100km/h监控
 * 触发条件：车速 > 100 km/h
 * 恢复条件：车速 ≤ 100 km/h
 * @return EventStatus TRIGGER/RECOVERY/NORMAL
 */
static EventStatus MonitorVehicleSpeedGreater100(const MonitoringData_t rawData)
{
    static EventStatus currentStatus = EventStatus::EVENT_STATUS_NORMAL; // 当前状态，初始无事件

    if (rawData.canSignalData.speed > 100.0f)
    {
        if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
            return EventStatus::EVENT_STATUS_TRIGGER;
        }
    }
    else
    {
        if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_NORMAL;
            return EventStatus::EVENT_STATUS_RECOVERY;
        }
    }

    // 无事件发生，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 13、车辆远程启动状态监控
 * 触发条件：远程启动激活
 * 恢复条件：远程启动关闭
 * @return EventStatus TRIGGER/RECOVERY/NORMAL
 */
static EventStatus MonitorVehicleRemoteStart(const MonitoringData_t rawData)
{
    // LOG_Debug("%s() is running", __func__);
    static EventStatus currentStatus = EventStatus::EVENT_STATUS_NORMAL; // 当前状态，初始无事件

    if (rawData.canSignalData.remoteStartActive)
    {
        if (currentStatus != EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_TRIGGER;
            return EventStatus::EVENT_STATUS_TRIGGER;
        }
    }
    else
    {
        if (currentStatus == EventStatus::EVENT_STATUS_TRIGGER)
        {
            currentStatus = EventStatus::EVENT_STATUS_NORMAL;
            return EventStatus::EVENT_STATUS_RECOVERY;
        }
    }

    // 无事件发生，返回静默状态
    return EventStatus::EVENT_STATUS_NORMAL;
}

/**
 * @brief 判断是否处于故障状态
 *
 */
bool IsInFaultState()
{
    return (g_faultStatus.VehicleCanCommFail.load() ||
            g_faultStatus.GpsModuleFault.load() ||
            g_faultStatus.BatteryChargeFail.load());
}
