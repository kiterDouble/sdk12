#ifndef _TERMINAL_MANAGEMENT_DATA_MONITOR_H
#define _TERMINAL_MANAGEMENT_DATA_MONITOR_H

#include <stdint.h>
#include <stdio.h>

#include <string.h>
#include <glib.h>

#include "pm_server_api.h"
#include "sr_hal_gnss.h"
#include "gnss_ipc_msg.h"
#include "modem_ipc_msg.h"
#include "sr_param.h"
#include "can_signal_header_tonly.h"

#define MS_1000 (1000LL)                   // 统一时间单位
#define INTERVAL_MS (MS_1000 / 1)          // 数据采集(更新)间隔
#define PACKAGE_INTERVAL_MS (MS_1000 * 10) // 周期数据组包间隔

typedef enum
{
    EVENT_STATUS_NORMAL = 0, // 事件静默
    EVENT_STATUS_TRIGGER,    // 事件触发
    EVENT_STATUS_RECOVERY,   // 事件恢复
} EventStatus;

// 设备状态枚举
typedef enum
{
    DEVICES_STATUS_NORMAL = 0, // 正常
    DEVICES_STATUS_ABNORMAL,   // 异常
    DEVICES_STATUS_FAULT       // 故障
} DeviceStatus;

// 高压系统状态
typedef enum
{
    HV_SYSTEM_STATE_STANDBY = 0x0,      // 高压系统待机（预充未完成）
    HV_SYSTEM_STATE_FAULT = 0x1,        // 高压系统故障（严重故障，需要维修）
    HV_SYSTEM_STATE_DISCONNECTED = 0x2, // 高压系统断开（故障或主动断开）
    HV_SYSTEM_STATE_ACTIVE = 0x3,       // 高压系统激活（主继电器闭合，高压已上电）
    HV_SYSTEM_STATE_OFF = 0x4           // 高压系统关闭（下电）
} HighVoltageSystemState;

// 整车电源管理系统状态
typedef enum
{
    PEPS_INITIAL = 0x0,
    PEPS_OFF = 0x1,
    PEPS_ACC = 0x2,
    PEPS_ON = 0x3,
    PEPS_START = 0x4,
    PEPS_INVALID = 0xF
} VehiclePowerState;

// 发动机运行状态
typedef enum
{
    MOTOR_STATUS_POWER_CONSUMING = 0x1, // 耗电（驱动状态）
    MOTOR_STATUS_GENERATING = 0x2,      // 发电（能量回收）
    MOTOR_STATUS_OFF = 0x3,             // 关闭状态
    MOTOR_STATUS_STANDBY = 0x4,         // 准备状态
    MOTOR_STATUS_FAULT = 0xFE,          // 异常
    MOTOR_STATUS_INVALID = 0xFF         // 无效
} EngineStatus;

// GPS 定位状态枚举
typedef enum
{
    INVALID = 0, // 无效定位
    VALID        // 有效定位
} GpsFixStatus;

// CAN接收器状态
typedef enum
{
    CAN_STATUS_NORMAL = 0,  // 正常,接收到CAN报文
    CAN_STATUS_NO_RESPONSE, // 无应答,未收到CAN报文
    CAN_STATUS_ACK_ERROR    // ACK错误
} CanStatus;

/******************************************************************************************************************************************************* */

typedef struct
{
    uint8_t keyStatus;     // 0x18F107D0-VCU_T_VhlSts 点火状态
    bool isReady;          // 0x18F101D0-VCU_VhclSts 车辆上高压状态
    float speed;           // 0x18F101D0-VCU_VhclSpd 车速
    int32_t motorSpeed;    // 0x0CFF3108-MCU_T_DrvMtrSpd 驱动电机转速
    uint8_t hvSystemFltLv; // 0x18F13DF3-BMS_BMSFltLv 高压-BMS系统故障等级
    // 0x18F101D0-VCU_SysDTC 高压-VCU系统故障（暂废）
    HighVoltageSystemState hvSystemStatus; // 0x18F13DF3-BMS_BMSSts 高压系统状态
    bool driverOffSeat;                    // 0x18FFE117-IC_DrvrDepAlrm 驾驶员离座状态
    VehiclePowerState vehiclePowerState;   // 0x18FFD81D-PEPS_PDUPosition 整车电源状态
    // uint8_t highVoltageFailureCount;       // 上高压失败重试次数（前期会议：终端根据触发状态自行计数）（后期会议：暂废）
    EngineStatus engineStatus; // 0x0CFF0308-MCU_MotWorkSts 发动机运行状态
    bool remoteStartActive;    // 0x18FFC1D0-VCU_RemoteCmdExecSts 远程启动标志位
} MonitorCanSignalData_t;

/******************************************************************************************************************************************************* */

typedef struct
{
    char dataFilePath[128];
    char taskConfigFilePath[128];
    char downloadFolder[128];
    char unzipFolder[128];
} MonitorConfigPath;

typedef struct
{
    float cpuUsage;           // CPU 使用率
    float memoryUsage;        // 内存使用率
    uint64_t memoryUsedBytes; // 内存使用字节数
    float diskUsage;          // 硬盘使用率
    uint32_t diskFreeSpaceKB; // 磁盘剩余空间数
    uint8_t emmcMounted;      // EMMC挂载状态
} MonitorCPUStatusData_t;

typedef struct
{
    GnssLocation_t locationInfo;
    uint8_t gpsModuleResetTimes; // GPS模组复位次数
    uint8_t hasGpsRawData;       // GPS模组有原始数据
    uint8_t usesexternalgnss;    // 是否使用外置天线
} MonitorPositioningData_t;      // IPC发送终端管理平台定位数据的数据结构

typedef struct
{
    // uint8_t value[E_ParamId_enumParamEnd][256];// 数据量(无效数据)太大，需拆分索引分帧传输
    char sn[32];
    char iccid[32];
    char imsi[32];
    char vin[32];
    char imei[32];
    char projectName[32];
    char srSwVersion[32]; // 内部软件版本
    char srHwVersion[32]; // 内部硬件版本
    char SwVersion[32];   // 客户软件版本
    char HwVersion[32];   // 客户硬件版本
} MonitorTboxDbInfo_t;

typedef struct
{
    uint8_t networkType;
    uint8_t csq;
    uint8_t carrierType;
} MonitorNetStatus_t;

typedef struct
{
    uint8_t workMode;                   // 工作模式（唤醒原+sleep）
    int64_t powerModeChangeTime;        // 电源状态变更时间
    float backupBatteryVoltage;         // 备用电池电压
    uint8_t ignOn;                      // KL15状态(ON/OFF)
    uint8_t canBusOff;                  // 是否处于busOff状态
    uint8_t canReceiverFaultIndication; // CAN接收器故障指示
    uint8_t canReceiverStatus;          // CAN接收器的状态
    float vehicleBatteryVoltage;        // 整车蓄电池电压 KL30电压
    uint8_t batterySource;              // 终端电源类型
} MonitorMcuStatusData_t;

typedef struct
{
    uint8_t lastCommStatus; // 最近通信状态（可自定义字符串，如 "OK", "Timeout"）
    // 设备状态；数据来源：监控程序
    DeviceStatus devicesStatus; // 设备状态（正常、异常、故障）
    // 车辆状态
    uint8_t vehicleStatus; // 车辆状态-集充电、档位、发动机状态、软件升级、为一体的状态

    // 版本信息
    MonitorTboxDbInfo_t tboxDbInfo;
    // 电源信息
    MPUPmStateChange_t powerDate;
    // CPU状态信息
    MonitorCPUStatusData_t cpuStatusData;
    // MCU状态信息
    MonitorMcuStatusData_t mcuStatusData;
    // GPS 信息
    MonitorPositioningData_t positioningData;
    // 网络状态
    MonitorNetStatus_t netInfo;
    // can信号-整车数据
    MonitorCanSignalData_t canSignalData;
} MonitoringData_t;

typedef struct
{
    EventStatus eventStatus;                                            // 事件状态
    EventStatus (*monitorFuncCallback)(const MonitoringData_t rawData); // 监控函数
    void (*triggerHandleFunc)(const MonitoringData_t rawData);          // 触发状态处理函数
    void (*recoveryHandleFunc)(const MonitoringData_t rawData);         // 恢复状态处理函数
} EventGather;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 获取配置地址
     */
    void MonitorConfig_Get(MonitorConfigPath *out);

    /**
     * @brief 更新配置地址
     */
    void MonitorConfig_Set(const MonitorConfigPath *newConfig);

    /**
     * @brief 地址配置初始化
     *
     * @return int
     */
    int MonitorCfgPathInit(const char *dataFilePath, const char *taskConfigFilePath,
                           const char *downloadFolder, const char *unzipFolder);

    /**
     * @brief 初始化触发事件回调
     *
     */
    void InitTriggerEventCallback(void);

    /**
     * @brief 从磁盘文件中初始化事件的触发状态
     *
     * @param idx
     * @param cmdId
     * @param cmdIdSize
     * @return EventStatus
     */
    EventStatus InitStateFlagFromDisk(int idx, char *cmdId, size_t cmdIdSize);

    /**
     * @brief 根据 idx 创建事件文件到磁盘空间
     * 旨在磁盘中记录无法恢复事件(需售后干预处理的事件)的触发状态
     * @param idx
     * @param cmdId
     * @return int
     */
    int SetStatusFlagToDisk(int idx, const char *cmdId);

    /**
     * @brief 清除磁盘中记录的事件触发状态
     * @param flagIdx 事件索引
     * @return RET_OK 成功，RET_ERR 失败
     */
    int ResetStatusFlagToDisk(int flagIdx);

    /**
     * @brief 原始数据接收回调初始化
     *
     * @return int32_t
     */
    int32_t InitSignalIpcCallback();

    /**
     * @brief 获取原始数据
     *
     * @param out
     * @return int
     */
    int GetMonitoringData(MonitoringData_t *out);

    /**
     * @brief 初始化TBOX静态信息
     * @param
     * @return
     */
    int InitTboxStaticInfo();

    /**
     * @brief 开始数据组包
     *
     * @return int
     */
    int DataPackagingStart(void);

    /**
     * @brief 结束数据组包
     *
     * @return int
     */
    int DataPackagingStop(void);

    /**
     * @brief 创建周期定时器
     * @param callback 回调函数（必须是 void (*)(union sigval) 类型）
     * @param timeoutMs 超时时间（毫秒，必须 > 0）
     * @param userData 传递给回调的用户数据
     * @param timerId 输出参数，返回定时器句柄
     * @return 0 成功，-1 失败
     */
    int CreateResourceTimer(void (*callback)(union sigval), int timeoutMs, void *userData, timer_t *timerId);

    /**
     * @brief 判断是否处于故障状态
     *
     */
    bool IsInFaultState(void);

#ifdef __cplusplus
}
#endif

#endif // _TERMINAL_MANAGEMENT_DATA_MONITOR_H
