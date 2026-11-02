#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "ql_type.h"
#include "ql_sms.h"
#include "sr_log.h"
#include "hal_modem_sms.h"

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

static void HalModemSmsRxMsgHandler(ql_sms_msg_t *msg,
                                ql_sms_timestamp_t *timestamp,
                                ql_sms_user_data_head_t *head)
{
    if (!msg || !timestamp || !head)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;        
    }

    HalModemSmsInfo_t smsInfo;

    strcpy((char *)smsInfo.sourceAddr, msg->addr);
    memcpy(smsInfo.msgContent, msg->content, msg->content_size);
    smsInfo.msgLen = msg->content_size;
    
    if (gSmsPriv.smsRecvCb)
    {
        gSmsPriv.smsRecvCb(&smsInfo);
    }    
}

int32_t HalModemSmsInit(HalModemSmsRecvCallback recvCb)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (!recvCb)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_SMS_INITIALED == gSmsPriv.initialed)
    {
        LOG_Error("%s have init.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    retVal = ql_sms_init();

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s init fail.", __FUNCTION__);
        return ret;       
    }

    retVal = ql_sms_set_msg_recv_cb(HalModemSmsRxMsgHandler);

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_sms_set_msg_recv_cb fail.", __FUNCTION__);
        ql_sms_deinit();
        return ret;
    }

    gSmsPriv.smsRecvCb = recvCb;
    gSmsPriv.initialed = HAL_MODEM_SMS_INITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemSmsDeinit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_SMS_INITIALED != gSmsPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }

    retVal = ql_sms_deinit();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_sms_deinit fail.", __FUNCTION__);
        return ret;
    }
    
    gSmsPriv.smsRecvCb = NULL;
    gSmsPriv.initialed = HAL_MODEM_SMS_UNINITIALED; 
    ret = RET_OK;
    return ret;
}

int32_t HalModemSmsSend(const uint8_t *phoneNum, const uint8_t *mesg, uint8_t mesgLen)
{
    ql_sms_msg_t smsInfo;
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (!phoneNum || !mesg || (mesgLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_SMS_INITIALED != gSmsPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }
    memset(&smsInfo, 0x0, sizeof(ql_sms_msg_t));
    smsInfo.format =QL_SMS_MSG_FORMAT_BINARY_DATA;
    strcpy(smsInfo.addr, (char *)phoneNum);
    memcpy(smsInfo.content,mesg, mesgLen);
    smsInfo.content_size = mesgLen;

    retVal = ql_sms_send_msg(&smsInfo);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s send fail.", __FUNCTION__);
        return ret;       
    }
    
    ret = RET_OK;
    return ret;    
}