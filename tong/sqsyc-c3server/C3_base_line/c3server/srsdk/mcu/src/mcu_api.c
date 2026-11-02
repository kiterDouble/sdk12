#include <string.h>
#include <stdlib.h>
#include "err_code.h"
#include "sr_log.h"
#include "mcu_api.h"

int32_t McuSrvSendMesg(const McuMesg_t *msg)
{
    int32_t ret = RET_ERR;

    if (!msg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    McuSrvRpcRes_t res;
    uint16_t resLen = (uint16_t)sizeof(McuSrvRpcRes_t);

    res.type = msg->type;
    res.result = RET_ERR;

    McuSrvRpcReq_t *req;
    uint16_t len = (uint16_t)sizeof(McuSrvRpcReq_t) + (uint16_t)msg->payloadLen;

    req = (McuSrvRpcReq_t *)calloc(1, len);

    if (!req)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return ret;        
    }

    req->type =  msg->type;
    req->dataLen = msg->payloadLen;
    (void)memcpy(req->data, msg->payload, msg->payloadLen);
    (void)RpcGetMsgSync(RPCSERVER_TYPE_MCUSERVER, 0, 0, (void*)req, len, (void*)&res, &resLen);
    free(req);
    ret = res.result;
    return ret;
}