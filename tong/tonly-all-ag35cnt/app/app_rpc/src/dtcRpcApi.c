#include <stdint.h>
#include "dtcRpcApi.h"
#include "err_code.h"
#include "sr_ipc.h" 
#include "sr_log.h"
#include "queue_msg.h"

int32_t RpcAppDtcReadDid   (const RpcAppDtcDidReadReq_t  *req,
                             RpcAppDtcDidReadResp_t      *resp,
                             uint32_t timeoutMs){
    if (req == NULL || resp == NULL)
    {
        LOG_Error("RpcAppDtcReadDid: req or resp is NULL");
        return RET_ERR;
    }
    if (req->did == 0U)
    {
        LOG_Error("RpcAppDtcReadDid: invalid did=%u", req->did);
        return RET_ERR;
    }

    RpcAppDtcDidReadResp_t recvData = {0};
    uint16_t recvLen = 0U;

    const int rc = RpcGetMsgSync(RPC_APP_SERVER_TYPE_DTC,
                                 RPC_APP_DTC_SUBCMD_READ_DID_E,
                                 (int32_t)timeoutMs,
                                 (const uint8_t *)req,
                                 (uint16_t)sizeof(RpcAppDtcDidReadReq_t),
                                 (uint8_t *)&recvData,
                                 (uint16_t *)&recvLen);
    
    if (rc != RET_OK)
    {
        LOG_Error("RpcAppDtcReadDid: RpcGetMsgSync failed, did=%u, rc=%d",req->did, rc);
        return RET_ERR;
    }

    /* 校验枚举范围，避免上游越界取值 */
    if (recvLen != sizeof(RpcAppDtcDidReadResp_t))
    {
        LOG_Error("RpcAppDtcReadDid: recvLen != sizeof(RpcAppDtcDidReadResp_t) did=%u, recvLen=%u", req->did, recvLen);
        return RET_ERR;
    }

    (void)memcpy(resp, &recvData, sizeof(RpcAppDtcDidReadResp_t));
    return RET_OK;
}

int32_t RpcAppDtcWriteDid  (const RpcAppDtcDidWriteReq_t *req,
                             RpcAppDtcDidWriteResp_t     *resp,
                             uint32_t timeoutMs){
    if (req == NULL || resp == NULL)
    {
        LOG_Error("RpcAppDtcWriteDid: req or resp is NULL");
        return RET_ERR;
    }

    if (req->did == 0U)
    {
        LOG_Error("RpcAppDtcWriteDid: invalid did=%u", req->did);
        return RET_ERR;
    }
    
    RpcAppDtcDidWriteResp_t recvData = {0};
    uint16_t recvLen = 0U;

    const int rc = RpcGetMsgSync(RPC_APP_SERVER_TYPE_DTC,
                                 RPC_APP_DTC_SUBCMD_WRITE_DID_E,
                                 (int32_t)timeoutMs,
                                 (const uint8_t *)req,
                                 (uint16_t)sizeof(RpcAppDtcDidWriteReq_t),
                                 (uint8_t *)&recvData,
                                 (uint16_t *)&recvLen);
    if (rc != RET_OK)
    {
        LOG_Error("RpcAppDtcWriteDid: RpcGetMsgSync failed, did=%u, rc=%d",req->did, rc);
        return RET_ERR;
    }

    if (recvLen != sizeof(RpcAppDtcDidWriteResp_t))
    {
        LOG_Error("RpcAppDtcWriteDid: recvLen != sizeof(RpcAppDtcDidWriteResp_t) did=%u, recvLen=%u", req->did, recvLen);
        return RET_ERR;
    }

    (void)memcpy(resp, &recvData, sizeof(RpcAppDtcDidWriteResp_t));
    return RET_OK;
}