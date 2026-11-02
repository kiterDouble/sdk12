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

static HalModemSmsPriv_t gSmsPriv =
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
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    HalModemSmsInfo_t smsInfo;

    (void)strcpy((char *)smsInfo.sourceAddr, msg->addr);
    char *tmpMsgContent = (char*)smsInfo.msgContent;
    (void)memcpy(tmpMsgContent, msg->content, (uint32_t)msg->content_size);
    smsInfo.msgLen = msg->content_size;
    
    if (NULL != gSmsPriv.smsRecvCb)
    {
        LOG_Debug("func:%s,line:%d, addr:%s, content:%s", __FUNCTION__, __LINE__, (char *)smsInfo.sourceAddr, (char *)smsInfo.msgContent);
        gSmsPriv.smsRecvCb(&smsInfo);
    }    
}

int32_t HalModemSmsInit(HalModemSmsRecvCallback recvCb)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

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
        LOG_Error("%s init fail,return: %d", __FUNCTION__, retVal);
        return ret;       
    }

    retVal = ql_sms_set_msg_recv_cb(HalModemSmsRxMsgHandler);

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_sms_set_msg_recv_cb fail,retVal:%d", __FUNCTION__, retVal);
        (void)ql_sms_deinit();
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
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (HAL_MODEM_SMS_INITIALED != gSmsPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }

    retVal = ql_sms_deinit();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_sms_deinit fail,return: %d", __FUNCTION__, retVal);
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
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (!phoneNum || !mesg || (mesgLen == 0U))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_SMS_INITIALED != gSmsPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }
    (void)memset(&smsInfo, 0x0, sizeof(ql_sms_msg_t));
    smsInfo.format =QL_SMS_MSG_FORMAT_BINARY_DATA;
    (void)strcpy(smsInfo.addr, (const char *)phoneNum);
    const char *tmpMesg = (const char*) mesg;
    (void)memcpy(smsInfo.content, tmpMesg, mesgLen);
    smsInfo.content_size = (int)mesgLen;

    retVal = ql_sms_send_msg(&smsInfo);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s send fail,return: %d", __FUNCTION__, retVal);
        return ret;       
    }
    
    ret = RET_OK;
    return ret;    
}