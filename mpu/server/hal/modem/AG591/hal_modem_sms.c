#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_util_common.h"
#include "ql-sdk/ql_type.h"
#include "ql-sdk/ql_ms_sms.h"
#include "err_code.h"
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


static void sms_msg_recv_cb(int sim_id,
                        ql_sms_msg_t *msg,
                        ql_sms_timestamp_t *timestamp,
                        ql_sms_user_data_head_t *head)
{
    if ((HalModemSimGetActiveSlot_2(false) != sim_id) ||!msg || !timestamp || !head)
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

    retVal = ql_ms_sms_init();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_sms_init fail.", __FUNCTION__);
        return ret;       
    }

    // add
    int32_t activesimslot = 0; // add
    activesimslot = HalModemSimGetActiveSlot_2(false);
    if(activesimslot < 0 )
    {
        LOG_Error("sim_slot_2: %s HalModemSimGetActiveSolt", __FUNCTION__);
        return ret;
    }


    retVal = ql_ms_sms_set_msg_recv_cb(activesimslot, sms_msg_recv_cb); // ?

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_sms_set_msg_recv_cb fail.", __FUNCTION__);
        ql_ms_sms_deinit();
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

    retVal = ql_ms_sms_deinit();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_sms_deinit fail.", __FUNCTION__);
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

    // add
    int32_t activesimslot = 0; // add
    activesimslot = HalModemSimGetActiveSlot_2(false);
    if(activesimslot < 0 )
    {
        LOG_Error("sim_slot_2: %s HalModemSimGetActiveSolt", __FUNCTION__);
        return ret;
    }


    retVal = ql_ms_sms_send_msg(activesimslot, &smsInfo);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s send fail.", __FUNCTION__);
        return ret;       
    }
    
    ret = RET_OK;
    return ret;    
}