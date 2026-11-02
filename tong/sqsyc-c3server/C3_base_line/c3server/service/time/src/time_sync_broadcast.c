#include <stdint.h>
#include <stdio.h>


#include "mcu_api.h"
#include "sr_log.h"
#include "pm_server_api.h"

#include "time_status.h"
#include "time_server.h"



/**
 * @brief 同步时间状态至 MCU
 *
 * 将当前系统时间和同步状态格式化为 MCU 消息协议（6 字节 payload）：
 * - serviceData[0..3]: 系统时间（大端，4 字节）
 * - serviceData[4]: 服务类型固定为 0x0D
 * - serviceData[5]: 状态字节（bit5: GNSS=0x20/NTP/TSP=0x10，bit0: 同步OK=0x01）
 *
 * @param[in] timeSync 时间同步状态结构体
 * @return int32_t McuSrvSendMesg 的返回值，0 表示成功
 */
static int32_t SyncTimeStatusToMcu(MpuTimeSync_t timeSync)
{
    uint8_t serviceData[8] = {0};
    time_t  systemTime = 0;
    McuMesg_t msg;

    
    time(&systemTime);

    serviceData[0] = (systemTime >> 24) & 0xFF;
    serviceData[1] = (systemTime >> 16) & 0xFF;
    serviceData[2] = (systemTime >> 8) & 0xFF;
    serviceData[3] = systemTime & 0xFF;

    serviceData[4] = 13;


    if(E_TIME_SYNC_SRC_GNSS == timeSync.timeSyncSrc)
    {
        serviceData[5] = 0x20;
    }
    else if(E_TIME_SYNC_SRC_NTP == timeSync.timeSyncSrc || E_TIME_SYNC_SRC_TSP == timeSync.timeSyncSrc)
    {
        serviceData[5] = 0x10;
    }
    else
    {
        serviceData[5] = 0x00;
    }

    if(TIME_SYNC_OK == timeSync.timeSyncState)
    {
        serviceData[5] |= 0x01;
    }
    else
    {
        serviceData[5] = 0x00;
    }

    msg.type = SYS_QUE_TYPE_MCU_SYNC_TIME;
    msg.payloadLen = 6;
    msg.payload = (uint8_t *)serviceData;

    return McuSrvSendMesg(&msg);
}

/**
 * @brief 广播时间同步状态
 *
 * 通过 SendMsgToAllQueue 将 MpuTimeSync_t（2 字节：state + src）广播到所有订阅者。
 *
 * @param[in] timeSync 时间同步状态结构体
 * @return int32_t 0 表示成功
 */
static int32_t BroadcastTimeSyncStatus(MpuTimeSync_t timeSync)
{
    uint8_t serviceData[3] = {0};
    
    serviceData[0] = timeSync.timeSyncState;
    serviceData[1] = timeSync.timeSyncSrc;
    serviceData[2] = timeSync.priority;
    SendMsgToAllQueue(SYS_QUE_TYPE_TIME_STATUS_NOTIFY, serviceData, 3);

    return 0;
}

/**
 * @brief 时间同步广播线程主函数
 *
 * 每秒执行一次：
 * 1. 读取当前授时源状态，构建 MpuTimeSync_t
 * 2. 调用 SyncTimeStatusToMcu 将状态同步至 MCU
 * 3. 调用 BroadcastTimeSyncStatus 广播到 IPC
 *
 * @param[in] user_data 指向 TimeSrvCtx_t 的指针
 * @return void* 始终返回 NULL
 */
static void* TimeSyncBroadcastTaskThreadFunc(void* user_data)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*)user_data;
    TimeSyncTaskThreadData_t* threadData = (TimeSyncTaskThreadData_t*)ctx->broadcastTaskThreadData;
    MpuTimeSync_t timeSync = {0};

    while(g_atomic_int_get(&threadData->taskQuitFlag) == 0)
    {
        /** 休眠 **/
        if(g_atomic_int_get(&ctx->pmState) != PM_STATE_NORMAL){
            sleep(1);
            continue;
        }

        timeSync.timeSyncSrc = g_atomic_int_get(&ctx->timeSyncSource);
        timeSync.timeSyncState = g_atomic_int_get(&ctx->timeSyncSource) > E_TIME_SYNC_SRC_NONE ? TIME_SYNC_OK : TIME_SYNC_NOT_OK;
        timeSync.priority = g_atomic_int_get(&ctx->priority);

        /* 同步时间同步状态及时间源至MCU */
        SyncTimeStatusToMcu(timeSync);

        /* 广播时间同步状态及时间源 */
        BroadcastTimeSyncStatus(timeSync);

        sleep(1);
    }

    return NULL;
}


/**
 * @brief 初始化时间同步广播线程
 *
 * 分配线程控制数据结构，创建 GLib 线程。
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功，-1 表示失败
 */
int InitTimeSyncBroadcastTask(void* arg)
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
        LOG_Fatal("broadcastTaskThreadData malloc failed");
        return -1;
    }
    threadData->taskQuitFlag = 0;
    threadData->taskThread = NULL;
    ctx->broadcastTaskThreadData = threadData;

    threadData->taskThread = g_thread_new("time_sync_broadcast", TimeSyncBroadcastTaskThreadFunc, (void*)arg);
    if (threadData->taskThread == NULL)
    {
        LOG_Fatal("Create TimeSyncBroadcastTaskThread failed");
        free(threadData);
        ctx->broadcastTaskThreadData = NULL;
        return -1;
    }

    return 0;
}

/**
 * @brief 释放时间同步广播线程
 *
 * 设置线程退出标志，等待线程结束，释放资源。
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功
 */
int ReleaseTimeSyncBroadcastTask(void* arg)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*)arg;
    TimeSyncTaskThreadData_t* threadData = NULL;

    if (ctx == NULL) {
        return 0;
    }

    threadData = (TimeSyncTaskThreadData_t*)ctx->broadcastTaskThreadData;
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
    ctx->broadcastTaskThreadData = NULL;

    return 0;
}
