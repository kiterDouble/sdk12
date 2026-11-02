#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>

#include "sr_log.h"
#include "pm_server_api.h"

#include "time_status.h"
#include "time_server.h"
#include "time_rtc_util.h"

 /**
 * @brief 设置系统时间
 *
 * 通过 settimeofday 设置系统时间。若配置了 RTC 设备节点，同时调用 RtcTimeSet 同步写入 RTC。
 *
 * @param[in] devRtc RTC 设备节点路径（可为 NULL 或空字符串，表示不同步 RTC）
 * @param[in] newtime 要设置的 UTC 时间戳（秒）
 * @return int32_t 0 表示成功，-1 表示 settimeofday 失败
 */
static int32_t SetSystemTime(const char* devRtc, time_t newtime)
{
    struct timeval tv;

    memset(&tv, 0, sizeof(struct timeval));

    tv.tv_sec = newtime;

    if (settimeofday(&tv, NULL) != 0)
    {
        LOG_Error("settimeofday error : %s", strerror(errno));
        return -1;
    }

    if (devRtc != NULL && strlen(devRtc) > 0){
        (void)RtcTimeSet(devRtc, newtime);
    }

    return 0;
}

/**
 * @brief 时间同步管理线程主函数
 *
 * 核心决策线程，循环执行以下操作：
 * 1. 从 timeSyncEventQueue 接收各授时源发送的时间事件（超时 1000ms）
 * 2. 按优先级决策是否采纳新时间（新事件优先级 >= 当前优先级时采纳）
 * 3. 调用 SetSystemTime 设置系统时间并同步 RTC
 * 4. 管理活性定时器：授时源过期前标记 aboutToExpire，过期后清除状态
 * 5. 检测系统时间跳变：若跳变超过阈值则清除授时状态
 *
 * @param[in] userData 指向 TimeSrvCtx_t 的指针
 * @return void* 始终返回 NULL
 */
static void* TimeSyncManageTaskThreadFunc(void* userData)
{
    int ret = 0;
    uint8_t syncTimeEvent = 0;
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*)userData;
    Msg_t timeSyncMsg = {0};
    time_t syncTimeStamp = 0;
    time_t lastTimeStamp = 0;
    time_t curTimeStamp = 0;
    double activationElapsed = 0;

    GTimer* activationTimer = g_timer_new();
    int activationDurationSec = 0;

    TimeSyncTaskThreadData_t* threadData = (TimeSyncTaskThreadData_t*)ctx->manageTaskThreadData;

    while(g_atomic_int_get(&threadData->taskQuitFlag) == 0)
    {
        /** 休眠 **/
        if(g_atomic_int_get(&ctx->pmState) != PM_STATE_NORMAL){
            sleep(1);
            continue;
        }
        
        ret = MsgQueueRecv(&ctx->timeSyncEventQueue, &timeSyncMsg, 1000);
        if(ret == 0)
        {
            if (timeSyncMsg.len == sizeof(time_t))
            {
                memcpy(&syncTimeStamp, timeSyncMsg.data, sizeof(time_t));

                LOG_Warn("time sync event; src:%d utcTime:%ld pri:%d act:%d(s)", 
                    timeSyncMsg.type,
                    syncTimeStamp,
                    TimeCfgGetSyncSourcePriority(&ctx->config, timeSyncMsg.type),
                    TimeCfgGetSyncSourceActivation(&ctx->config, timeSyncMsg.type));
                
                /** 刷新当前授时源 **/
                if (TimeCfgGetSyncSourcePriority(&ctx->config, timeSyncMsg.type) >= g_atomic_int_get(&ctx->priority))
                {
                    /** 启动/重置 活性定时器，更新全局授时源和授时优先级 **/
                    LOG_Warn("sync time by this sync event");
                    if (SetSystemTime(ctx->config.rtcDev, syncTimeStamp) == 0)
                    {
                        g_atomic_int_set(&ctx->timeSyncSource, timeSyncMsg.type);
                        g_atomic_int_set(&ctx->timeSyncSourceAboutToExpire, 0);
                        g_atomic_int_set(&ctx->priority, TimeCfgGetSyncSourcePriority(&ctx->config, timeSyncMsg.type));
                        activationDurationSec = TimeCfgGetSyncSourceActivation(&ctx->config, timeSyncMsg.type); 
                        g_timer_start(activationTimer);
                        /** 本次循环触发了时间同步事件  **/
                        syncTimeEvent = 1;
                        LOG_Warn("sync time by this sync event success !!!");
                    }
                    else
                    {
                        LOG_Error("sync time by this sync event failed !!!");
                    }
                }
                else
                {
                    LOG_Warn("Already sync time by higher priority source, ignore this sync event");
                }
            }
            MsgFree(&timeSyncMsg);
        }

        /** 判断当前授时是否过期 **/
        if (g_atomic_int_get(&ctx->timeSyncSource) > E_TIME_SYNC_SRC_NONE){
            activationElapsed = g_timer_elapsed(activationTimer, NULL);
            if (activationElapsed >= (activationDurationSec + 60)){
                LOG_Warn("current sync time source(%d) priority(%d) is Expired !!!", g_atomic_int_get(&ctx->timeSyncSource), g_atomic_int_get(&ctx->priority));
                g_atomic_int_set(&ctx->timeSyncSource, E_TIME_SYNC_SRC_NONE);
                g_atomic_int_set(&ctx->priority, 0);
                g_atomic_int_set(&ctx->timeSyncSourceAboutToExpire, 0); 
            }else if(activationElapsed >= (activationDurationSec)){
                LOG_Warn("current sync time source(%d) priority(%d) is About Expired !!!", g_atomic_int_get(&ctx->timeSyncSource), g_atomic_int_get(&ctx->priority));
                g_atomic_int_set(&ctx->timeSyncSourceAboutToExpire, 1); 
            }
        }

        /** 判断系统时间跳变 **/
        curTimeStamp = time(NULL);
        /** 本次循环未触发了时间同步事件, 系统时间跳变超过阀值，清除授时状态，触发重新授时**/
        if (g_atomic_int_get(&ctx->timeSyncSource) > E_TIME_SYNC_SRC_NONE && 
            !syncTimeEvent && llabs((long long)curTimeStamp - (long long)lastTimeStamp) >= ctx->config.timeJumpEventThreshold)
        {
            LOG_Warn("systemTime jump (%lld)s over (%d)s, set timeSyncSource to inValidity !!!",
                    llabs((long long)curTimeStamp - (long long)lastTimeStamp), ctx->config.timeJumpEventThreshold);
            g_atomic_int_set(&ctx->timeSyncSource, E_TIME_SYNC_SRC_NONE);
            g_atomic_int_set(&ctx->priority, 0);
            g_atomic_int_set(&ctx->timeSyncSourceAboutToExpire, 0); 
        }
    
        /** 更新上次循环记录的当前时间 **/
        if (lastTimeStamp != curTimeStamp){
            lastTimeStamp = curTimeStamp;
        }

        /** 清除本次循环同步事件  **/
        if (syncTimeEvent){
            syncTimeEvent = 0;
        }
    }

    g_timer_destroy(activationTimer);

    return NULL;
}


/**
 * @brief 初始化同步时间管理线程
 *
 * 初始化 timeSyncEventQueue（长度 2），分配线程控制数据结构，创建 GLib 线程。
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功，-1 表示失败
 */
int InitTimeSyncManageTask(void* arg)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*)arg;
    TimeSyncTaskThreadData_t* threadData = NULL;

    if(ctx == NULL)
    {
        LOG_Error("input empty is arg");
        return -1;
    }
    
    /* 初始化 timeSyncEventQueue 队列 */
    if(MsgQueueInit(&ctx->timeSyncEventQueue, 2) != 0)
    {
        LOG_Error("timeSyncEventQueue int failed");
        return -1;
    }

    threadData = (TimeSyncTaskThreadData_t*)malloc(sizeof(TimeSyncTaskThreadData_t));
    if (threadData == NULL) {
        LOG_Error("manageTaskThreadData malloc failed");
        return -1;
    }
    threadData->taskQuitFlag = 0;
    threadData->taskThread = NULL;
    ctx->manageTaskThreadData = threadData;

    /* 初始化 TimeSyncManage 处理线程 */
    threadData->taskThread = g_thread_new("time_sync_manage", TimeSyncManageTaskThreadFunc, (void*) arg);
    if (threadData->taskThread == NULL)
    {
        LOG_Error("Create TimeSyncManageTaskThread failed");
        free(threadData);
        ctx->manageTaskThreadData = NULL;
        return -1;
    }

    return 0;
}

/**
 * @brief 释放同步时间管理线程
 *
 * 设置线程退出标志，等待线程结束，释放线程数据结构和消息队列。
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功
 */
int ReleaseTimeSyncManageTask(void* arg)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*)arg;
    TimeSyncTaskThreadData_t* threadData = NULL;

    if(ctx == NULL)
    {
        LOG_Error("input empty is arg");
        return -1;
    }
    
    threadData = (TimeSyncTaskThreadData_t*)ctx->manageTaskThreadData;
    if (threadData != NULL) {
        g_atomic_int_set(&threadData->taskQuitFlag, 1);

        if (threadData->taskThread != NULL){
            g_thread_join(threadData->taskThread);
            g_thread_unref(threadData->taskThread);
            threadData->taskThread = NULL;
        }

        free(threadData);
        ctx->manageTaskThreadData = NULL;
    }

    /* 释放 timeSyncEventQueue 队列 */
    MsgQueueDeinit(&ctx->timeSyncEventQueue);
    
    return 0;
}
