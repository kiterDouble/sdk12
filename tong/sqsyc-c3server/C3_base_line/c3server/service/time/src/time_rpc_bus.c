
#include <string.h>

#include "time_status.h"
#include "time_server.h"
#include "sr_log.h"

/**
 * @brief RPC TSP 时间上报子命令枚举
 */
typedef enum{
    E_RPC_TIME_RSEVER_REPORT_TSP_TIME = 0x0,
    E_RPC_TIME_RSEVER_GET_TSP_PRIORITY,
}RpcDigitalKeySubcmd_e;

/**
 * @brief 处理 TSP 时间上报 RPC 请求
 *
 * 接收外部模块通过 RPC 推送的 TSP 时间戳，校验参数和优先级条件后，
 * 发送 E_TIME_SYNC_SRC_TSP 时间同步事件到 Manage 线程的事件队列。
 *
 * @param[in]  reqData  请求数据指针（期望为 time_t 类型的时间戳）
 * @param[in]  reqLen   请求数据长度（期望为 sizeof(time_t)）
 * @param[out] respData 响应数据缓冲区（写入 int32_t 返回码）
 * @param[out] respLen   响应数据长度（固定为 sizeof(int32_t)）
 * @param[in]  userdata  用户数据（指向 TimeSrvCtx_t）
 */
static void RpcReportTspTime(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*) userdata;
    time_t tspTimeStamp = 0;
    int32_t ret  = -1;
    int32_t* result = (int32_t*) respData;

    do {
        if (userdata == NULL || reqData == NULL || reqLen != sizeof(time_t) || respData == NULL) {
            LOG_Error("Rpc input param error");
            break;
        }

        memcpy(&tspTimeStamp, reqData, sizeof(time_t));
        LOG_Warn("[%s] tspTimeStamp=%ld", __func__, tspTimeStamp);

        if (tspTimeStamp == 0){
            ret = -2;
            break;
        }

        if(g_atomic_int_get(&ctx->priority) < ctx->config.tspCfg.priorityActivation.priority || 
            (g_atomic_int_get(&ctx->priority) == ctx->config.tspCfg.priorityActivation.priority && g_atomic_int_get(&ctx->timeSyncSourceAboutToExpire)))
        {
            MsgQueueSend(&ctx->timeSyncEventQueue, E_TIME_SYNC_SRC_TSP, &tspTimeStamp, sizeof(time_t));  
        }

        ret = 0;
    } while(0);

    *respLen = sizeof(int32_t);
    *result = ret;
}

static void RpcGetTspPriority(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*) userdata;
    int32_t ret  = -1;
    int32_t* result = (int32_t*) respData;

    do {
        if (userdata == NULL || respData == NULL) {
            LOG_Error("Rpc input param error");
            break;
        }

        LOG_Info("[%s] get tsp priority=%d", __func__, ctx->config.tspCfg.priorityActivation.priority);

        ret = ctx->config.tspCfg.priorityActivation.priority;
    } while(0);

    *respLen = sizeof(int32_t);
    *result = ret;
}

/**
 * @brief RPC Server 总回调函数
 *
 * 注册到 RPC Server 的入口回调，根据 subcmd 分发到对应的处理函数。
 * 当前仅支持 E_RPC_TIME_RSEVER_REPORT_TSP_TIME（0x0），其他 subcmd 忽略。
 *
 * @param[in]  subcmd  RPC 子命令
 * @param[in]  reqData 请求数据指针
 * @param[in]  reqLen  请求数据长度
 * @param[out] respData 响应数据缓冲区
 * @param[out] respLen  响应数据长度
 * @param[in]  userdata 用户数据（指向 TimeSrvCtx_t）
 */
void TimeRpcBusServerCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    switch (subcmd)
    {
        case E_RPC_TIME_RSEVER_REPORT_TSP_TIME:
            RpcReportTspTime(reqData, reqLen, respData, respLen, userdata);
            break;
        case E_RPC_TIME_RSEVER_GET_TSP_PRIORITY:
            RpcGetTspPriority(reqData, reqLen, respData, respLen, userdata);
            break;
        default:
            break;
    }
}
