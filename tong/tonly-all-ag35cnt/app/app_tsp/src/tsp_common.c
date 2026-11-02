/**
 * @file tsp_common.c
 * @brief 公共接口实现
 */

#include "tsp_common.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "queue_msg.h"
#include "tsp_remote_ctrl.h"
#include "tsp_log.h"
#include "tsp_app.h"
#include <string.h>
#include <pthread.h>
#include <time.h>

/* ======================== 静态变量 ======================== */

static pthread_mutex_t g_gnssMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_hardLineMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_pmStateMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_modemSrvMutex = PTHREAD_MUTEX_INITIALIZER;

static GnssLocation_t g_gnssLocation = {0};
static HardLineSyncInfo_t g_hardLineInfo = {0};
static MPUPmStateChange_t g_pmState = {0};
static ModemSrvNotify_t g_modemSrvInfo = {0};

static gint g_tspPmStateHandleThreadQuitFlag = 0;
static GThread* g_tspPmStateHandleThread = NULL;

#define PM_LOCATION_HEART_TSP_INTERVAL 2*60*60  /* PM休眠时，唤醒上报位置的间隔(秒) */

/**
 * @brief 处理TCP长连接闹钟（设置tcp长连接唤醒闹钟）
 */
 static int tly_app_set_tcp_keepalive_alarm(tly_app_t *app)
 {
     int8_t ret;
 
     if (!app) {
         TLY_LOG_E("Keepalive: invalid params");
         return -1;
     }
 
     /* 创建RTC休眠闹钟 */
     if (app->tcp_keepalive_alarm_id != 0) {
         /* 销毁已存在的闹钟 */
         ret = PmDestroySleepAlarm(app->tcp_keepalive_alarm_id);
         TLY_LOG_D("Keepalive: destroy existing alarm, alarmId=%u, ret=%d",
                   app->tcp_keepalive_alarm_id, ret);
         app->tcp_keepalive_alarm_id = 0;
     }
 
     /* 创建RTC闹钟，使用APP层预留ID范围(10-30) */
     app->tcp_keepalive_alarm_id = CPU_APP_TIMER_TCP_KEEPALIVE;
     ret = PmCreateSleepAlarm(app->tcp_keepalive_alarm_id, PM_LOCATION_HEART_TSP_INTERVAL, 0);
     if (ret != 0) {
         TLY_LOG_E("Keepalive: failed to create RTC alarm, alarmId=%u, ret=%d",
                   app->tcp_keepalive_alarm_id, ret);
         app->tcp_keepalive_alarm_id = 0;
         return -1;
     }
 
     TLY_LOG_I("Keepalive scheduled: alarmId=%u", app->tcp_keepalive_alarm_id);
     return 0;
 }

 /**
 * @brief 处理TCP长连接闹钟（删除tcp长连接唤醒闹钟）
 */
static void tly_app_delete_tcp_keepalive_alarm(tly_app_t *app)
{
    int8_t ret;
 
    /* 销毁已存在的闹钟 */
    if (app->tcp_keepalive_alarm_id != 0) { 
        ret = PmDestroySleepAlarm(app->tcp_keepalive_alarm_id);
        TLY_LOG_D("Keepalive: delete tcp keepalive alarm, alarmId=%u, ret=%d",
                   app->tcp_keepalive_alarm_id, ret);
        app->tcp_keepalive_alarm_id = 0;
    }
}

/* ======================== 内部回调函数 ======================== */

/**
 * @brief GNSS定位信息订阅回调
 */
static void GnssMsgSubCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
    (void)userdata;
    
    if (data == NULL || len != sizeof(GnssLocation_t)) {
        TLY_LOG_D("GnssMsgSubCallBack: invalid params, len=%d, expected=%zu",
                  len, sizeof(GnssLocation_t));
        return;
    }

    pthread_mutex_lock(&g_gnssMutex);
    memcpy(&g_gnssLocation, data, sizeof(GnssLocation_t));
    pthread_mutex_unlock(&g_gnssMutex);
}

/**
 * @brief 硬线状态同步订阅回调
 */
static void HardLineSyncCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
    (void)userdata;
    
    if (data == NULL || len < 12) {
        TLY_LOG_D("HardLineSyncCallBack: invalid params, len=%d", len);
        return;
    }

    pthread_mutex_lock(&g_hardLineMutex);
    g_hardLineInfo.kl15status  = data[0];
    g_hardLineInfo.kl30voltage = ((uint32_t)data[1] << 24) |
                                 ((uint32_t)data[2] << 16) |
                                 ((uint32_t)data[3] << 8)  |
                                 (uint32_t)data[4];
    g_hardLineInfo.fastCharge  = ((uint16_t)data[5] << 8) | data[6];
    g_hardLineInfo.OBD         = data[7];
    g_hardLineInfo.kl30status   = data[8];
    pthread_mutex_unlock(&g_hardLineMutex);
    
    // TLY_LOG_D("HardLineSyncCallBack: kl30voltage=%u mV", g_hardLineInfo.kl30voltage);
}

/**
 * @brief 保存PM电源状态数据
 * @param[in] pmData PM电源状态数据
 * @description 供其他模块获取PM状态使用
 */
static void SavePmData(const MPUPmStateChange_t *pmData)
{
    if (pmData == NULL) {
        TLY_LOG_D("SavePmData: invalid params");
        return;
    }

    pthread_mutex_lock(&g_pmStateMutex);
    memcpy(&g_pmState, pmData, sizeof(MPUPmStateChange_t));
    pthread_mutex_unlock(&g_pmStateMutex);

    // TLY_LOG_D("SavePmData: WorkMode=%u, PmState=%u, WakeupSrc=%u, SleepAlarmEventId=%u",
    //           pmData->WorkMode, pmData->PmState, pmData->WakeupSrc, pmData->SleepAlarmEventId);
}

/**
 * @brief PM电源状态变化订阅回调
 * @description 处理SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY消息，
 *              调用remote_ctrl_handle_battery_heat_wakeup处理电池加热逻辑，
 *              调用SavePmData保存数据供其他模块使用
 */
static void PmStateChangeCallBack(const uint8_t *data, const uint16_t dataLen, void *userData)
{
    const MPUPmStateChange_t *pmData = NULL;
    tly_app_t *app = (tly_app_t *)userData;

    if (data == NULL) {
        TLY_LOG_E("PmStateChangeCallBack: input data is NULL");
        return;
    }

    if (dataLen != sizeof(MPUPmStateChange_t)) {
        TLY_LOG_E("PmStateChangeCallBack: data len is invalid, dataLen=%u, expected=%zu",
                  dataLen, sizeof(MPUPmStateChange_t));
        return;
    }

    pmData = (const MPUPmStateChange_t *)data;

    if (pmData->WakeupSrc == PM_WAKEUP_SOURCE_TIMER_MCU && pmData->SleepAlarmEventId == CPU_APP_TIMER_BATTERY_HEAT) {
        app->battery_heat_alarm_arrived = 1; /* 标记已接收到电池包加热MCU唤醒事件 */
    }

    if (pmData->WakeupSrc == PM_WAKEUP_SOURCE_TIMER_MCU && pmData->SleepAlarmEventId == CPU_APP_TIMER_TCP_KEEPALIVE) {
        app->tcp_keepalive_alarm_arrived = 1; /* 标记已接收到TCP长连接MCU唤醒事件 */
    }

    /* 处理电池加热RTC唤醒逻辑 */
    // (void)remote_ctrl_handle_battery_heat_wakeup(pmData, app);
    
    /* 保存PM状态数据供其他模块使用 */
    (void)SavePmData(pmData);
}

static void NetMsgCallback(const uint8_t *data, const uint16_t dataLen, void *userData)
{
    (void)userData;
    if (data == NULL || dataLen != sizeof(ModemSrvNotify_t)) {
        TLY_LOG_E("NetMsgCallback: invalid params, dataLen=%u", dataLen);
        return;
    }

    pthread_mutex_lock(&g_modemSrvMutex);
    memcpy(&g_modemSrvInfo, data, sizeof(ModemSrvNotify_t));
    pthread_mutex_unlock(&g_modemSrvMutex);
}

static void *TspPmStateHandleThreadFunc(void* data)
{
    tly_app_t *app = (tly_app_t *)data;
    static int tspWorkState = 1;
    TLY_LOG_I("TspPmStateHandleThread: start");

    while (g_atomic_int_get(&g_tspPmStateHandleThreadQuitFlag) == 0) {
        if (g_pmState.PmState == PM_STATE_SLEEP && tspWorkState == 1) {
            TLY_LOG_I("TspPmStateHandleThread: PM state is sleep");
            tspWorkState = 0;
            tly_app_set_tcp_keepalive_alarm(app);  /* 休眠时，设置tcp长连接唤醒闹钟 */
            if (tly_tcp_is_connected(app->tcp)) {
                tly_app_send_vehicle_logout(app);  /* 休眠前，发送登出平台消息 */
            }
        } else if (g_pmState.PmState == PM_STATE_NORMAL && tspWorkState == 0) {
            TLY_LOG_I("TspPmStateHandleThread: PM state is normal");
            tspWorkState = 1;
            tly_app_delete_tcp_keepalive_alarm(app);  /* 唤醒时，删除tcp长连接唤醒闹钟 */
        }

        if (app->battery_heat_alarm_arrived == 1) {
            app->battery_heat_alarm_arrived = 0;
            TLY_LOG_I("Battery heat RTC wakeup, alarmId=%u", g_pmState.SleepAlarmEventId);
            /* 闹钟唤醒时，重新设置下一个闹钟 */
            tly_app_set_battery_heat_alarm(app);
            /* 发送电池加热命令到MCU */
            tly_lock_control_t battery_heat_lock;
            memset(&battery_heat_lock, 0, sizeof(battery_heat_lock));
            battery_heat_lock.ctrl_cmd = TLY_CTRL_BATTERY_HEAT;
            battery_heat_lock.lock_type = 1; /* 电池加热类型 */
            send_remote_cmd(NULL, &battery_heat_lock);
        }

        /* 检查是否是定时器唤醒 */
        if (app->tcp_keepalive_alarm_arrived == 1) {
            app->tcp_keepalive_alarm_arrived = 0;
            /* 检查是否是TCP长连接保持的RTC闹钟事件 */
                TLY_LOG_I("TCP keepalive RTC wakeup, alarmId=%u",
                          g_pmState.SleepAlarmEventId);
                /* 闹钟唤醒时，重新设置下一个闹钟 */
                tly_app_set_tcp_keepalive_alarm(app);
                /* 尝试连接登入平台，并发送心跳包 */
                if (!tly_tcp_is_connected(app->tcp)) {
                    TLY_LOG_I("RTC wakeup: TCP not connected, trying to connect...");
                    tly_tcp_connect(app->tcp,
                                    app->config.tcp_host,
                                    app->config.tcp_port,
                                    app->config.tcp_reconnect_interval);
            } 
        }

        usleep(100000);    /* 100ms */
    }

    return NULL;
}

/* ======================== 对外接口实现 ======================== */

int CommonMsg_GetGnssLocation(GnssLocation_t *location)
{
    if (location == NULL) {
        return -1;
    }

    pthread_mutex_lock(&g_gnssMutex);
    memcpy(location, &g_gnssLocation, sizeof(GnssLocation_t));
    pthread_mutex_unlock(&g_gnssMutex);
    
    return 0;
}

int CommonMsg_GetHardLineInfo(HardLineSyncInfo_t *info)
{
    if (info == NULL) {
        return -1;
    }

    pthread_mutex_lock(&g_hardLineMutex);
    memcpy(info, &g_hardLineInfo, sizeof(HardLineSyncInfo_t));
    pthread_mutex_unlock(&g_hardLineMutex);
    
    return 0;
}

int CommonMsg_GetPmState(MPUPmStateChange_t *pmState)
{
    if (pmState == NULL) {
        return -1;
    }

    pthread_mutex_lock(&g_pmStateMutex);
    memcpy(pmState, &g_pmState, sizeof(MPUPmStateChange_t));
    pthread_mutex_unlock(&g_pmStateMutex);
    
    return 0;
}

int CommonMsg_GetModemSrvInfo(ModemSrvNotify_t *modemInfo)
{
    if (modemInfo == NULL) {
        return -1;
    }

    pthread_mutex_lock(&g_modemSrvMutex);
    memcpy(modemInfo, &g_modemSrvInfo, sizeof(ModemSrvNotify_t));
    pthread_mutex_unlock(&g_modemSrvMutex);
    
    return 0;
}

int CommonMsg_Subscribe(tly_app_t *app)
{
    int ret = 0;
    
    /* 初始化TSP长连接处理线程 */
    g_tspPmStateHandleThread = g_thread_new("TspPmStateHandleThread", TspPmStateHandleThreadFunc, app);
    if (g_tspPmStateHandleThread == NULL)
    {
        LOG_Fatal("Create TspPmStateHandleThread failed");
        return -1;
    }

    /* 订阅硬线状态同步 */
    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_KLINE, HardLineSyncCallBack, NULL);
    if (ret != 0) {
        TLY_LOG_E("Failed to subscribe SYS_QUE_TYPE_MCU_SYNC_KLINE, ret=%d", ret);
    }

    /* 订阅GNSS定位信息 */
    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO, GnssMsgSubCallBack, NULL);
    if (ret != 0) {
        TLY_LOG_E("Failed to subscribe SYS_QUE_TYPE_GNSS_LOCATIONINFO, ret=%d", ret);
    }
    
    /* 订阅PM电源状态通知 */
    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, PmStateChangeCallBack, app);
    if (ret != 0) {
        TLY_LOG_E("Failed to subscribe SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, ret=%d", ret);
    }

    /* 订阅通信模块信息 */
    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY, NetMsgCallback, NULL);
    if (ret != 0) {
        TLY_LOG_E("Failed to subscribe SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY, ret=%d", ret);
    }

    TLY_LOG_D("CommonMsg_Subscribe: success");
    return 0;
}

int CommonMsg_Unsubscribe(void)
{
    g_atomic_int_set(&g_tspPmStateHandleThreadQuitFlag, 1);

    /* 释放TSP长连接处理线程 */
    if (g_tspPmStateHandleThread != NULL){
        g_thread_join(g_tspPmStateHandleThread);
        g_thread_unref(g_tspPmStateHandleThread);
    }
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_KLINE);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY);
    TLY_LOG_D("CommonMsg_Unsubscribe: success");
    return 0;
}

/* ======================== 单调时间工具函数 ======================== */

time_t tly_monotonic_time_sec(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (time_t)ts.tv_sec;
    }
    return (time_t)0;
}

uint64_t tly_monotonic_time_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
    }
    return 0;
}

uint64_t tly_monotonic_time_us(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
    }
    return 0;
}
