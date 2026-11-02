#include "gnss_ipc_msg.h"
#include "queue_msg.h"

#include "sr_ipc.h"
#include "sr_log.h"
#include "pm_server_api.h"

#include "time_status.h"
#include "time_server.h"


/**
 * @brief GNSS 时间状态结构体
 *
 * 用于 GNSS 回调函数和线程主循环之间共享 GNSS 定位数据。
 * 通过 GMutex 保护对计时器和状态的并发访问。
 */
typedef struct{
    time_t utcTime;                 /**< 最新 GNSS UTC 时间戳 */
    GTimer* reflushTimer;           /**< 数据刷新计时器 */
    GMutex reflushTimerMutex;       /**< 保护计时器和状态的互斥锁 */
    int status;                     /**< GPS 状态（gpsState & 0x3F） */
}GnssTimeState_t;

/**
 * @brief GNSS 定位信息 IPC 回调函数
 *
 * 订阅 SYS_QUE_TYPE_GNSS_LOCATIONINFO 后触发。
 * 解析 GnssLocation_t，提取时间戳和 GPS 状态，在互斥锁保护下更新 GnssTimeState_t。
 *
 * @param[in] recvData 接收到的 GNSS 数据指针（GnssLocation_t）
 * @param[in] dataLen  数据长度
 * @param[in] userdata 用户数据（指向 GnssTimeState_t）
 */
static void GnssDataCallBackFunc(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    GnssLocation_t *pGnssData = (GnssLocation_t *)recvData;
    GnssTimeState_t* gnssTimeState = (GnssTimeState_t*) userdata;

    do{
        if(pGnssData == NULL || gnssTimeState == NULL || dataLen != sizeof(GnssLocation_t))
        {
            LOG_Error("%s argv is invalid", __func__);
            break;
        }
        
        g_mutex_lock(&gnssTimeState->reflushTimerMutex);

        gnssTimeState->status = pGnssData->gpsState & 0x3F;
        gnssTimeState->utcTime = pGnssData->timestamp;

        /** 启动/重置定时器 **/
        g_timer_start(gnssTimeState->reflushTimer);

        g_mutex_unlock(&gnssTimeState->reflushTimerMutex);     

    }while(0);
}

/**
 * @brief 判断当前 GNSS 时间数据是否有效
 *
 * 在互斥锁保护下读取计时器 elapsed 时间和 GPS 状态。
 * 判定有效条件：数据在 2 秒内刷新过，且 gpsState == 1，且时间戳 > 0。
 *
 * @param[in] gnssTimeInfo GNSS 时间状态指针
 * @return time_t 有效的 UTC 时间戳；0 表示无效
 */
static time_t GnssDataIsValid(GnssTimeState_t* gnssTimeInfo)
{
    double gnssDataElapsed = 0;
    uint8_t gnssDataState = 0;
    time_t gnssDataTime = 0;

    g_mutex_lock(&gnssTimeInfo->reflushTimerMutex);

    gnssDataElapsed = g_timer_elapsed(gnssTimeInfo->reflushTimer, NULL);
    gnssDataState = gnssTimeInfo->status;
    gnssDataTime = gnssTimeInfo->utcTime;

    g_mutex_unlock(&gnssTimeInfo->reflushTimerMutex);

    /** 最新的gnss定位信息有效，且最新回调时间已经过去少于2.0秒**/
    if (gnssDataElapsed < 2.0f && gnssDataState == 1 && gnssDataTime > 0)
    {
        return gnssDataTime;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief GNSS 时间同步线程主函数
 *
 * 线程生命周期：
 * 1. 初始化计时器和互斥锁
 * 2. 订阅 GNSS 定位信息 IPC 消息
 * 3. 循环检测：若当前授时源优先级低于 GNSS（或相同但即将过期），且 GNSS 数据有效，则发送时间事件到 Manage 线程
 * 4. 退出时取消订阅并释放资源
 *
 * @param[in] userData 指向 TimeSrvCtx_t 的指针
 * @return void* 始终返回 NULL
 */
static void* TimeSyncGnssTaskThreadFunc(void* userData)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*)userData;
    GnssTimeState_t gnssTimeState = {0};
    time_t gnssUtcRealTime = 0;

    /** 初始化 gnss 授时全局变量数据访问互斥锁 ，初始化 gnss 数据刷新计时器 **/
    gnssTimeState.reflushTimer = g_timer_new();
    g_mutex_init(&gnssTimeState.reflushTimerMutex);

    /** 注册Gnss定位信息回调 **/
    (void) SubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO, GnssDataCallBackFunc, &gnssTimeState);

    TimeSyncTaskThreadData_t* threadData = (TimeSyncTaskThreadData_t*)ctx->gnssTaskThreadData;

    while(g_atomic_int_get(&threadData->taskQuitFlag) == 0)
    {
        /** 休眠 **/
        if(g_atomic_int_get(&ctx->pmState) != PM_STATE_NORMAL){
            sleep(1);
            continue;
        }

        /* 当前 授时源 优先级 低于 Gnss授时 优先级
         * 或者 Gnss授时即将过期，重新开始授时 */
        if(g_atomic_int_get(&ctx->priority) < ctx->config.gnssCfg.priorityActivation.priority || 
            (g_atomic_int_get(&ctx->priority) == ctx->config.gnssCfg.priorityActivation.priority && g_atomic_int_get(&ctx->timeSyncSourceAboutToExpire)))
        {
            gnssUtcRealTime = GnssDataIsValid(&gnssTimeState);
            /** 当前获取的gnss定时时间有效 **/
            if (gnssUtcRealTime >0)
            {
                LOG_Info("gnssUtcRealTime=%ld", gnssUtcRealTime);
                MsgQueueSend(&ctx->timeSyncEventQueue, E_TIME_SYNC_SRC_GNSS, &gnssUtcRealTime, sizeof(time_t));
            }
        }

        sleep(1);
    }

    /** 释放 gnss 授时全局变量数据访问互斥锁 ，释放 gnss 数据刷新计时器 **/
    g_mutex_clear(&gnssTimeState.reflushTimerMutex);
    g_timer_destroy(gnssTimeState.reflushTimer);

    /** 释放Gnss定位信息回调 **/
    (void) UnSubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO);

    return NULL;
}

/**
 * @brief 初始化 GNSS 时间同步线程
 *
 * 分配线程控制数据结构，创建 GLib 线程。
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功，-1 表示失败
 */
int InitTimeSyncGnssTask(void* arg)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*)arg;
    TimeSyncTaskThreadData_t* threadData = NULL;

    if(arg == NULL)
    {
        LOG_Error("input empty is arg");
        return -1;
    }

    threadData = (TimeSyncTaskThreadData_t*)malloc(sizeof(TimeSyncTaskThreadData_t));
    if (threadData == NULL) {
        LOG_Fatal("gnssTaskThreadData malloc failed");
        return -1;
    }
    threadData->taskQuitFlag = 0;
    threadData->taskThread = NULL;
    ctx->gnssTaskThreadData = threadData;

    threadData->taskThread = g_thread_new("time_sync_by_gnss", TimeSyncGnssTaskThreadFunc, (void*) arg);
    if (threadData->taskThread == NULL)
    {
        LOG_Fatal("Create TimeSyncGnssTaskThread failed");
        free(threadData);
        ctx->gnssTaskThreadData = NULL;
        return -1;
    }

    return 0;
}

/**
 * @brief 释放 GNSS 时间同步线程
 *
 * 设置线程退出标志，等待线程结束，释放资源。
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功
 */
int ReleaseTimeSyncGnssTask(void* arg)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*)arg;
    TimeSyncTaskThreadData_t* threadData = NULL;

    if (ctx == NULL) {
        return 0;
    }

    threadData = (TimeSyncTaskThreadData_t*)ctx->gnssTaskThreadData;
    if (threadData == NULL) {
        return 0;
    }

    g_atomic_int_set(&threadData->taskQuitFlag, 1);

    if (threadData->taskThread != NULL){
        g_thread_join(threadData->taskThread);
        g_thread_unref(threadData->taskThread);
        threadData->taskThread = NULL;
    }

    free(threadData);
    ctx->gnssTaskThreadData = NULL;

    return 0;
}
