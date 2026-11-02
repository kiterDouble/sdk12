#include <stdint.h>
#include "avntRpcApi.h"
#include "err_code.h"
#include "sr_ipc.h" 
#include "sr_log.h"
#include "queue_msg.h"

/* ==== 新接口：强类型版本 ==== */
/**
 * @brief 获取 TSP 会话状态（强类型版本）
 *
 * @param[out] outStatus  返回会话状态（仅当返回 RET_OK 时才写入）
 * @param[in]  timeoutMs  超时时间（毫秒）
 * @return RET_OK/RET_ERR
 */
int32_t GetAVNTSessionStatus(RpcAppAvntSessionStatus_e *outStatus,
                            uint32_t timeoutMs)
{
    if (outStatus == NULL)
    {
        LOG_Error("GetTspSessionStatus: outStatus is NULL\n");
        return RET_ERR;
    }

    /* 请求与响应缓冲 */
    const uint8_t req = 0U;
    uint8_t resp = 0U;
    uint16_t recvLen = 0U;

    /* 发送同步 RPC */
    const int rc = RpcGetMsgSync(RPC_APP_SERVER_TYPE_AVNT,
                                 RPC_APP_AVNT_SUBCMD_GET_CONNECT_STATUS_E,
                                 (int32_t)timeoutMs,
                                 &req,
                                 (uint16_t)sizeof(req),
                                 &resp,
                                 &recvLen);
    if (rc != RET_OK)
    {
        LOG_Error("GetTspSessionStatus: RpcGetMsgSync failed, rc=%d\n", rc);
        return RET_ERR;
    }

    /* 校验枚举范围，避免上游越界取值 */
    // if (resp > (uint8_t)RPC_APP_AVNT_CONN_STATE_DISCONNECTED_E)
    // {
    //     LOG_Error("GetTspSessionStatus: invalid status value=%u\n", resp);
    //     return RET_ERR;
    // }

    *outStatus = (RpcAppAvntSessionStatus_e)resp;
    return RET_OK;
}
