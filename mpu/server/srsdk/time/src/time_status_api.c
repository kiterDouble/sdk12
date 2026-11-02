#include "time_status.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "queue_msg.h"
#include "err_code.h"


static MpuTimeSync_t g_timeSync;

/**
 * @name TimeSyncCallBack
 * @brief 时间同步回调函数
 * 
 * 此函数用于处理接收到的时间同步数据，更新时间同步状态和来源。
 * 
 * @param data 指向接收到的数据的指针。
 * @param len 接收到的数据长度。
 * @param userdata 用户自定义数据。
 */
static void TimeSyncCallBack(const uint8_t *data, const uint16_t len, void* userdata)
{
    if(NULL == data)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(len != 2)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, len, SYS_QUE_TYPE_TIME_STATUS_NOTIFY);
    g_timeSync.timeSyncState = data[0];
    g_timeSync.timeSyncSrc = data[1];
}

/**
 * @name InitGetTimeSyncStatusCallBack
 * 初始化获取时间同步状态回调函数
 * 
 * 该函数用于初始化并注册一个用于接收时间同步状态通知的消息订阅回调。
 * 
 * @return int32_t 返回订阅操作的结果状态码。成功返回正值，失败返回负值。
 */
int32_t InitGetTimeSyncStatusCallBack()
{
    return SubscriptionQueueMsg(SYS_QUE_TYPE_TIME_STATUS_NOTIFY, TimeSyncCallBack, NULL);
}

/**
 * @name GetTimeSyncStatus
 * 获取时间同步状态
 * 
 * 本函数用于获取当前时间同步的状态信息，包括时间同步的状态和来源。
 * 
 * @param timeSync 指向MpuTimeSync_t结构体的指针，用于存放时间同步的状态和来源信息。
 * @return 返回函数执行状态，如果执行成功返回RET_OK，如果timeSync参数为NULL返回RET_ERR。
 */
int32_t GetTimeSyncStatus(MpuTimeSync_t *timeSync)
{
    if(NULL == timeSync)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    timeSync->timeSyncState =  g_timeSync.timeSyncState;
    timeSync->timeSyncSrc = g_timeSync.timeSyncSrc;

    return RET_OK;
}
/**
 * @name DestoryGetTimeSyncStatusCallBack
 * 销毁获取时间同步状态回调函数
 * 
 * 该函数用于取消订阅系统时间状态通知的消息队列消息。执行该操作后，将不再接收时间同步状态的更新通知。
 * 
 * @return int32_t 返回操作的结果状态码。成功返回0，失败返回非0值。
 */
int32_t DestoryGetTimeSyncStatusCallBack()
{
    return UnSubscriptionQueueMsg(SYS_QUE_TYPE_TIME_STATUS_NOTIFY);
}