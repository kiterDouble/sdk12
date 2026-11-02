#include "time_status.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "queue_msg.h"
#include "err_code.h"


static MpuTimeSync_t g_timeSync;

typedef enum{
    E_RPC_TIME_RSEVER_REPORT_TSP_TIME = 0x0,
    E_RPC_TIME_RSEVER_GET_TSP_PRIORITY,
}RpcDigitalKeySubcmd_e;


/**
 * @brief 时间同步状态 IPC 回调函数
 *
 * 订阅 SYS_QUE_TYPE_TIME_STATUS_NOTIFY 后，当 timeServer 广播时间同步状态时触发此回调。
 * 将接收到的状态保存到全局变量 g_timeSync 中。
 *
 * @param[in] data    指向接收到的数据指针（3字节：state + src + priority）
 * @param[in] len     接收到的数据长度
 * @param[in] userdata 用户自定义数据（当前未使用）
 */
static void TimeSyncCallBack(const uint8_t *data, const uint16_t len, void* userdata)
{
    if(NULL == data)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(len != 3)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    // LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, len, SYS_QUE_TYPE_TIME_STATUS_NOTIFY);
    g_timeSync.timeSyncState = data[0];
    g_timeSync.timeSyncSrc = data[1];
    g_timeSync.priority = data[2];
}

/**
 * @brief 初始化时间同步状态订阅回调
 *
 * 注册 IPC 消息订阅，监听 SYS_QUE_TYPE_TIME_STATUS_NOTIFY 主题的时间同步状态广播。
 *
 * @return int32_t 返回订阅操作的结果状态码。成功返回正值，失败返回负值。
 */
int32_t InitGetTimeSyncStatusCallBack()
{
    return SubscriptionQueueMsg(SYS_QUE_TYPE_TIME_STATUS_NOTIFY, TimeSyncCallBack, NULL);
}

/**
 * @brief 获取当前时间同步状态
 *
 * 从内部缓存中读取最近一次收到的时间同步状态和时间源。
 *
 * @param[out] timeSync 指向 MpuTimeSync_t 结构体的指针，用于存放时间同步的状态和来源信息
 * @return int32_t RET_OK(0) 表示成功，RET_ERR 表示参数无效
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
    timeSync->priority = g_timeSync.priority;
    return RET_OK;
}

/**
 * @brief 向时间管理服务上报 TSP 时间戳
 *
 * 通过同步 RPC 调用将 TSP 模块获取的时间戳推送给 timeServer。
 * timeServer 校验优先级条件后决定是否采纳该时间进行系统授时。
 *
 * @param[in] timeStamp TSP UTC 时间戳（秒）
 * @return int32_t 返回 timeServer 处理结果：0 表示成功处理，负值表示 RPC 调用失败
 */
int32_t SetTspSyncTime(time_t timeStamp)
{
    int ret = -1;
    int32_t result = 0;
    uint16_t recvLen = (uint16_t)sizeof(result);

    RpcGetMsgSync(RPCSERVER_TYPE_TIMESERVER, E_RPC_TIME_RSEVER_REPORT_TSP_TIME, 0, (uint8_t *)&timeStamp, sizeof(time_t), &result, &recvLen);

    if (recvLen != sizeof(int32_t)){
        LOG_Error("[%s] Req Failed!!!", __func__);
        ret = RET_ERR;
    }else{
        ret = result;
    }

    return ret;
}

/**
 * @brief 销毁时间同步状态订阅回调
 *
 * 取消订阅 SYS_QUE_TYPE_TIME_STATUS_NOTIFY 消息，停止接收时间同步状态更新。
 *
 * @return int32_t 返回操作的结果状态码。成功返回 0，失败返回非 0 值。
 */
int32_t DestoryGetTimeSyncStatusCallBack()
{
    return UnSubscriptionQueueMsg(SYS_QUE_TYPE_TIME_STATUS_NOTIFY);
}

/**
 * @brief 获取TSP校时源优先级
 *
 * 通过同步 RPC 调用获取 TSP 模块优先级。
 *
 * @return int TSP校时源优先级
 */
int GetTSPPriority(void)
{
    int ret = -1;
    int32_t result = 0;
    uint16_t recvLen = (uint16_t)sizeof(result);
    
    RpcGetMsgSync(RPCSERVER_TYPE_TIMESERVER, E_RPC_TIME_RSEVER_GET_TSP_PRIORITY, 0, NULL, 0, &result, &recvLen);

    if (recvLen != sizeof(int32_t)){
        LOG_Error("[%s] Req Failed!!!", __func__);
        ret = RET_ERR;
    }else{
        ret = result;
    }

    return ret;
}