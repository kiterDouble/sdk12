#include <string.h>
#include <stdlib.h>
#include "err_code.h"
#include "sr_log.h"
#include "doip_tp.h"
#include "msg_queue.h"
#include "doip_tester.h"

typedef struct
{
    MsgQueueHandle_t resp;
    MsgQueueHandle_t ack;
    DoipTesterNode_t node;
    uint32_t isTpReady;
}DoipTesterInfo_t;

typedef struct
{
    uint32_t isInitialed;
    DoipTesterHandle_t tester;
    DoipTesterInfo_t *info;
}DoipTpInfo_t;

DoipTpInfo_t doipTp;

static void DoipTpTxAck(uint32_t targetLa, int32_t status)
{

    if (!doipTp.isInitialed || !doipTp.info)
    {
        return;
    }
    SendMsgQueue(doipTp.info->ack, (void *) &status, sizeof(status));
}

static void DoipTpRxNotify(uint32_t targetLa, const uint8_t* udsData, uint32_t udsDataLen)
{
    if (!doipTp.isInitialed || !doipTp.info)
    {
        return;
    }
    SendMsgQueue(doipTp.info->resp, (void *) udsData, udsDataLen);
}

int32_t DoipTpInit(DoipTesterConnNotifyCb connNotify, DoipTesterOnlineNotifyCb onlineCb)
{
    int32_t ret = RET_ERR;
    if (doipTp.isInitialed)
    {
        LOG_Error("%s have inited.", __FUNCTION__);
        return ret;
    }

    memset(&doipTp, 0x0, sizeof(DoipTpInfo_t));
    DoipTesterCbs_t funs =
    {
        .conFunc = connNotify,
        .txFunc = DoipTpTxAck,
        .rxFunc = DoipTpRxNotify,
        .onlineFunc = onlineCb
    };

    ret = DoipTesterInit(&doipTp.tester, &funs);
    if (RET_OK == ret)
    {
        doipTp.isInitialed = 1;
    }
    return ret;
}

int32_t DoipTpConnect(const DoipTesterNode_t *node)
{
    int32_t ret = RET_ERR;
    if (!node)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        goto error_exit;
    }

    DoipTesterInfo_t *info = calloc(1, sizeof(DoipTesterInfo_t));

    if (!info)
    {
        LOG_Error("%s info calloc fail.", __FUNCTION__);
        goto error_exit;        
    }

    if (RET_OK != CreateMsgQueue(&info->resp))
    {
        LOG_Error("%s CreateMsgQueue resp fail.", __FUNCTION__);
        goto error_exit0;
    }

    if (RET_OK != CreateMsgQueue(&info->ack))
    {
        LOG_Error("%s CreateMsgQueue resp fail.", __FUNCTION__);
        goto error_exit1;
    }

    if (RET_OK != DoipTesterConnect(doipTp.tester, node))
    {
        LOG_Error("%s DoipTesterConnect fail.", __FUNCTION__);
        goto error_exit2;
    }

    doipTp.info = info;
    ret = RET_OK;
    goto error_exit;
    
error_exit2:
    DestroyMsgQueue(info->ack);
error_exit1:
    DestroyMsgQueue(info->resp);
error_exit0:
    free(info);
error_exit:
    return ret;
}

int32_t DoipTpTransmit(uint32_t targetLa, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || !dataLen)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (!doipTp.isInitialed)
    {
        LOG_Error("%s tp should initialed.", __FUNCTION__);
        return ret;       
    }

    if (!doipTp.info)
    {
        LOG_Error("%s have not connected.", __FUNCTION__);
        return ret;   
    }

    if (DoipTesterUdsRequest(doipTp.tester, targetLa, data, dataLen))
    {
        LOG_Error("%s DoipTesterUdsRequest fail.", __FUNCTION__);
        return ret;
    }
    int32_t status;
    uint32_t len;

    if ((RET_OK != RecvMsgQueue(doipTp.info->ack, (void*)&status, &len, 200)) || (status != RET_OK))
    {
        return ret;
    }

    return RET_OK;
}

int32_t DoipTpGetResponse(uint8_t *data, uint32_t *dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || !dataLen)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (!doipTp.isInitialed)
    {
        LOG_Error("%s tp should initialed.", __FUNCTION__);
        return ret;       
    }

    if (!doipTp.info)
    {
        LOG_Error("%s have not connected.", __FUNCTION__);
        return ret;   
    }

    if (RET_OK != RecvMsgQueue(doipTp.info->resp, data, dataLen, 1))
    {
        *dataLen = 0;
    }

    return RET_OK;
}

int32_t DoipTpDisconnect(void)
{
    int32_t ret = RET_ERR;

    if (!doipTp.isInitialed)
    {
        LOG_Error("%s tp should initialed.", __FUNCTION__);
        return ret;       
    }

    if (!doipTp.info)
    {
        LOG_Error("%s have not connected.", __FUNCTION__);
        return ret;   
    }

    DoipTesterInfo_t *info = doipTp.info;
    DestroyMsgQueue(info->ack);
    DestroyMsgQueue(info->resp);
    free(info);
    doipTp.info = NULL;
    return DoipTesterDisconnect(doipTp.tester);
    //return RET_OK;
}

int32_t DoipTpDeinit(void)
{
    int32_t ret = RET_ERR;
    if (0 == doipTp.isInitialed)
    {
         LOG_Error("%s tp should initialed.", __FUNCTION__);
        return ret;       
    }
    if (doipTp.info)
    {
        DoipTpDisconnect();
    }
    DoipTesterDeinit(doipTp.tester);
    doipTp.isInitialed = 0;
    ret = RET_OK;
    return ret;
}