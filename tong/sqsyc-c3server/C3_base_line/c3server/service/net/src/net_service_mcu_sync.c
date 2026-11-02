#include <stdint.h>
#include <unistd.h>

#include <glib.h>

#include "err_code.h"
#include "sr_log.h"
#include "mcu_api.h"
#include "queue_msg.h"
#include "modem_api.h"
#include "pm_server_api.h"
#include "net_service.h"


int32_t SendMcuNetStatusSync(NetSrvCtx_t *ctx)
{
    int32_t retValue = RET_OK;
    uint8_t payLoad[NET_STATUS_LENGTH] = {0};
    McuMesg_t McuMsg = {0};
    uint8_t apnStatus = 0;
    uint8_t antStatus = 0;
    ModemSrvExtSigInfo_t modemSrvExtSigInfo = {0};

    if (PM_STATE_SLEEP == ctx->pmState->PmState)
    {
        LOG_Debug("SendMcuNetStatusSync: PM_STATE_SLEEP");
        return RET_OK;
    }
    ModemSrvGetExtSignalInfo(&modemSrvExtSigInfo);
    for (int i = 0; i < ctx->userCfg->apnNum; i++)
    {
        if (ctx->strNetInfo[i].dialStatus == 1)
        {
            apnStatus |= (1 << i);
        }
    }
    antStatus = ((ctx->modemSrvNotify->antennaStatus & 0x0f) | (ctx->modemSrvNotify->secondaryAntennaStatus << 4));

    McuMsg.type = SYS_QUE_TYPE_MCU_SYNC_NETWORK_STATUS;
    McuMsg.payloadLen = NET_STATUS_LENGTH;
    payLoad[0] = ctx->modemSrvNotify->dataRegStatus;
    payLoad[1] = ctx->modemSrvNotify->csq;
    payLoad[2] = modemSrvExtSigInfo.lteSiginfo.sigType;
    payLoad[3] = ctx->userCfg->apnNum;
    payLoad[4] = apnStatus;
    payLoad[5] = antStatus;

    McuMsg.payload = payLoad;
    if (RET_ERR == McuSrvSendMesg(&McuMsg))
    {
        retValue = RET_ERR;
        LOG_Error(" %s:McuSrvSendMesg faild!", __func__);
    }
    LOG_Debug("%s - Creg:%d,Csq:%d,netType:%d,ApnCounter:%d,ApnStatus:%d,antStatus:%d",
    __func__,payLoad[0],payLoad[1],payLoad[2],payLoad[3],payLoad[4],payLoad[5]);
    return retValue;
}

gpointer NetStatusSyncMcu(gpointer pArg)
{
    NetSrvCtx_t *ctx = (NetSrvCtx_t *)pArg;
    while (1)
    {
        SendMcuNetStatusSync(ctx);
        usleep(1000000);
    }
}


