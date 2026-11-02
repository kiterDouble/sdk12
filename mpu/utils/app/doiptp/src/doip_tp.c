#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "sr_log.h"
#include "err_code.h"
#include "sdk/doip_tester.h"
#include "doip_tp.h"

#define DOIP_TP_FAIL                (0)
#define DOIP_TP_OK                  (1)


typedef struct {
    ethtpCallbacks cbs;
    DoipTesterHandle_t hdl;
}EthTpPriv_t;

static EthTpPriv_t *priv;
static void DoipConnNotify(uint32_t targetLa, int32_t status)
{
    if (!priv)
    {
        return;
    }

    if (priv->cbs.conFunc)
    {
        priv->cbs.conFunc(targetLa, status, DOIP_TP_OK);
    }
}

static void DoipTxAck(uint32_t targetLa, int32_t status)
{
    if (!priv)
    {
        return;
    }

    if (priv->cbs.txFunc)
    {
        uint8 errcode = (status == 0) ? DOIP_TP_OK : DOIP_TP_FAIL;
        priv->cbs.txFunc(targetLa, NULL, errcode);
    }
}

static void DoipRxNotify(uint32_t targetLa, const uint8_t* udsData, uint32_t udsDataLen)
{
    if (!priv)
    {
        return;
    }

    if (priv->cbs.rxFunc)
    {
        ethtpData tpData;
        memcpy(tpData.payloadBuf, udsData, udsDataLen);
        tpData.payloadLength = udsDataLen;
        tpData.reqType = 0;
        priv->cbs.rxFunc(targetLa, &tpData, DOIP_TP_OK);
    }
}
static void DoipOnlineNotify(uint32_t targetLa)
{
    if (!priv)
    {
        return;
    }

    if (priv->cbs.onlineFunc)
    {
        priv->cbs.onlineFunc(targetLa);
    }
}

uint8 ethtp_init(ethtpCallbacks* cbs)
{
    LOG_INIT("carota_doip");
    if (!cbs)
    {
        LOG_Error("%s Invalid parameter", __FUNCTION__);
        return DOIP_TP_FAIL;
    }

    if (priv)
    {
        if (priv->hdl)
        {
            DoipTesterDeinit(priv->hdl);
        }
        free(priv);
        priv = NULL;
    }

    priv = (EthTpPriv_t*)calloc(1, sizeof(EthTpPriv_t));

    if (!priv)
    {
        LOG_Error("%s calloc failed", __FUNCTION__);
        return DOIP_TP_FAIL;
    }
    memcpy(&priv->cbs, cbs, sizeof(ethtpCallbacks));
    DoipTesterCbs_t funs = {DoipConnNotify, DoipTxAck, DoipRxNotify, DoipOnlineNotify};
    if(RET_OK != DoipTesterInit(&priv->hdl, &funs))
    {
        LOG_Error("%s DoipTesterInit failed", __FUNCTION__);
        free(priv);
        priv = NULL;
        return DOIP_TP_FAIL;
    }
    LOG_Debug("%s called success.", __FUNCTION__);
    return DOIP_TP_OK;
}

uint8 ethtp_connect_node(ethtpNode* node)
{
    uint8 ret = DOIP_TP_FAIL;

    if (!node)
    {
        LOG_Error("%s Invalid parameter", __FUNCTION__);
        return ret;
    }

    if (!priv)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    DoipTesterNode_t testerNode;
    memset(&testerNode, 0, sizeof(DoipTesterNode_t));
    memcpy(testerNode.ipAddr, node->targetIpAddr, DOIP_TP_IPADDR_LEN);
    testerNode.activeType = 0x0;
    testerNode.functionalLa = node->functionalLa;
    testerNode.sourceLa = node->sourceLa;
    testerNode.targetLa = node->targetLa;
    testerNode.maxPL = (MAX_BUFFER_LENGTH - MAX_BUFFER_LENGTH % 1024 + 1024);
    if (RET_OK != DoipTesterConnect(priv->hdl, &testerNode))
    {
        LOG_Error("%s DoipTesterConnect failed", __FUNCTION__);
        return ret;
    }

    LOG_Debug("%s called success.", __FUNCTION__);
    return DOIP_TP_OK;
}

uint8 ethtp_transmit(ethtpNode* node, ethtpData* data)
{
    uint8 ret = DOIP_TP_FAIL;

    if (!node || !data)
    {
        LOG_Error("%s Invalid parameter", __FUNCTION__);
        return ret;
    }

    if (!priv)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    uint32_t targetAddr = (data->reqType) ? node->functionalLa : node->targetLa;
    if (RET_OK != DoipTesterUdsRequest(priv->hdl, targetAddr, data->payloadBuf, data->payloadLength))
    {
        LOG_Error("%s DoipTesterUdsRequest failed", __FUNCTION__);
        return ret;
    }

    LOG_Debug("%s called success.", __FUNCTION__);
    return DOIP_TP_OK;
}

uint8 ethtp_disconnect_node(ethtpNode* node)
{
    uint8 ret = DOIP_TP_FAIL;

    LOG_Debug("%s called.", __FUNCTION__);
    if (!priv)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    LOG_Debug("%s called success.", __FUNCTION__);
    DoipTesterDisconnect(priv->hdl);
    return DOIP_TP_OK;
}