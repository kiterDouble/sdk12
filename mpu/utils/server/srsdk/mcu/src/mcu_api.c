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
    uint16_t resLen = sizeof(McuSrvRpcRes_t);

    res.type = msg->type;
    res.result = RET_ERR;

    McuSrvRpcReq_t *req;
    uint16_t len = sizeof(McuSrvRpcReq_t) + msg->payloadLen;

    req = (McuSrvRpcReq_t *)calloc(1, len);

    if (!req)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return ret;        
    }

    req->type =  msg->type;
    req->dataLen = msg->payloadLen;
    memcpy(req->data, msg->payload, msg->payloadLen);
    RpcGetMsgSync(RPCSERVER_TYPE_MCUSERVER, 0, 0, (void*)req, len, (void*)&res, &resLen);
    free(req);
    ret = res.result;
    return ret;
}

void McuSrvSetSendLock(uint8_t flag)
{
    McuSrvRpcReq_t *req;
    uint16_t len = (uint16_t)sizeof(McuSrvRpcReq_t) + sizeof(uint8_t);

    McuSrvRpcRes_t res;
    uint16_t resLen = (uint16_t)sizeof(McuSrvRpcRes_t);

    res.type = 0;
    res.result = RET_ERR;

    req = (McuSrvRpcReq_t *)calloc(1, len);
    if (!req)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return;        
    }

    req->type = 0;
    req->dataLen = 1;
    req->data[0] = flag;
    (void)RpcGetMsgSync(RPCSERVER_TYPE_MCUSERVER, 1, 0, (void*)req, len, (void*)&res, &resLen);
    free(req);
}

int32_t McuSrvGetSendLock(int32_t *flag)
{
    McuSrvRpcReq_t *req;
    uint16_t len = (uint16_t)sizeof(McuSrvRpcReq_t) + sizeof(uint8_t);

    McuSrvRpcRes_t res;
    uint16_t resLen = (uint16_t)sizeof(McuSrvRpcRes_t);

    res.type = 0;
    res.result = RET_ERR;

    req = (McuSrvRpcReq_t *)calloc(1, len);
    if (!req)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return RET_ERR;        
    }

    req->type = 0;
    req->dataLen = 1;
    req->data[0] = 0;
    (void)RpcGetMsgSync(RPCSERVER_TYPE_MCUSERVER, 2, 0, (void*)req, len, (void*)&res, &resLen);
    free(req);
    *flag = res.result;

    return RET_OK;
}
