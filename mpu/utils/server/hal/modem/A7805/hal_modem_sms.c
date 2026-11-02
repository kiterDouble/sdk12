#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_sms.h"
#include "Sunsea_SMS.h"

#define HAL_MODEM_SMS_INITIALED (0)
#define HAL_MODEM_SMS_UNINITIALED (-1)

typedef struct
{
    HalModemSmsRecvCallback smsRecvCb;
    int32_t initialed;
}HalModemSmsPriv_t;

HalModemSmsPriv_t gSmsPriv =
{
    .smsRecvCb = NULL,
    .initialed = HAL_MODEM_SMS_UNINITIALED
};
static void HalModemSmsEventInd(sms_event_e smsEv, sms_info *info)
{
    HalModemSmsInfo_t smsInfo;
       
    if (gSmsPriv.initialed != HAL_MODEM_SMS_INITIALED)
    {
        LOG_Error("%s sms have not initialed.", __FUNCTION__);
        return;
    }

    if (smsEv != SMS_RECEIVE_FLAG) 
    {
        LOG_Debug("%s not receive msg, return.", __FUNCTION__);
        return;
    }

    if (!info)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    strcpy((char *)smsInfo.sourceAddr, info->phoneNum);
    memcpy(smsInfo.msgContent, info->smsData, info->smsDataLen);
    smsInfo.msgLen = info->smsDataLen;

    if (gSmsPriv.smsRecvCb != NULL)
    {
        gSmsPriv.smsRecvCb(&smsInfo);
    } 
}

int32_t HalModemSmsInit(HalModemSmsRecvCallback recvCb)
{

    if (gSmsPriv.initialed == HAL_MODEM_SMS_INITIALED)
    {
        LOG_Error("%s sms have initialed.", __FUNCTION__);
        return RET_OK;
    }
    if(!recvCb)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }
    if(Sunsea_SMS_Init(HalModemSmsEventInd) < 0)
    {
        LOG_Error("%s Sunsea_SMS_Init fail.", __FUNCTION__);
        return RET_ERR;
    }
    gSmsPriv.smsRecvCb = recvCb;
    gSmsPriv.initialed = HAL_MODEM_SMS_INITIALED;
    return RET_OK;
}

int32_t HalModemSmsDeinit(void)
{
    if (gSmsPriv.initialed != HAL_MODEM_SMS_INITIALED)
    {
        LOG_Error("%s sms have not initialed.", __FUNCTION__);
        return RET_ERR;
    }

    Sunsea_SMS_Deinit();
    gSmsPriv.smsRecvCb = NULL;
    gSmsPriv.initialed = HAL_MODEM_SMS_UNINITIALED;
    return RET_OK;
}

int32_t HalModemSmsSend(const uint8_t *phoneNum, const uint8_t *mesg, uint8_t mesgLen)
{
    sms_info smsInfo;
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (!phoneNum || !mesg || (mesgLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_SMS_INITIALED != gSmsPriv.initialed)
    {
        LOG_Error("%s have init.", __FUNCTION__);
        return ret;
    }
    memset(&smsInfo, 0x0, sizeof(sms_info));
    smsInfo.format =SMS_FORMAT_BINARY_DATA;
    strcpy(smsInfo.phoneNum, (char *)phoneNum);
    memcpy(smsInfo.smsData,mesg, mesgLen);
    smsInfo.smsDataLen = mesgLen;

    retVal = Sunsea_SMS_SendMsg(&smsInfo);

    if (retVal < 0)
    {
        LOG_Error("%s Sunsea_SMS_SendMsg fail.", __FUNCTION__);
        return ret;
    }

    return RET_OK;
}