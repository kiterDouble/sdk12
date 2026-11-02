#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_type.h"
#include "ql_sms.h"
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

static void HalModemSmsRxMsgHandler(ql_sms_msg_t *p_msg,
                                ql_sms_timestamp_t *timestamp,
                                ql_sms_user_data_head_t *head)
{
    HalModemSmsInfo_t smsInfo;
    if (!p_msg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    strcpy((char *)smsInfo.sourceAddr, p_msg->addr);
    memcpy(smsInfo.msgContent,p_msg->content, p_msg->content_size);
    smsInfo.msgLen = p_msg->content_size;
    
    if (gSmsPriv.smsRecvCb)
    {
        gSmsPriv.smsRecvCb(&smsInfo);
    }
}
/**
 * @name HalModemSmsInit
 * @brief 初始化Modem的SMS功能。
 * @param recvCb 接收短信的回调函数指针。当有短信收到时，此回调函数会被调用。
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
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

/**
 * @name HalModemSmsDeinit
 * @brief 反初始化SMS功能
 *        该函数用于当需要停止使用短信功能时，对之前初始化的短信模块进行反初始化操作。
 *        它将释放与短信功能相关的资源，并将模块状态设置为未初始化。
 * @param null
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
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

/**
 * @name HalModemSmsSend
 * @brief 发送SMS短信
 * @param phoneNum 接收短信的手机号码，类型为uint8_t数组
 * @param mesg 要发送的短信内容，类型为uint8_t数组
 * @param mesgLen 短信内容的长度，单位为字节
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
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