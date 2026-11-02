#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "ql_oe.h"
#include "ql_mcm_sms.h"
#include "sr_log.h"
#include "hal_modem_sms.h"

#define HAL_MODEM_SMS_INITIALED (0)
#define HAL_MODEM_SMS_UNINITIALED (-1)

typedef struct
{
    sms_client_handle_type smsHdl;
    HalModemSmsRecvCallback smsRecvCb;
    int32_t initialed;
}HalModemSmsPriv_t;

HalModemSmsPriv_t gSmsPriv =
{
    .smsHdl = 0x0,
    .smsRecvCb = NULL,
    .initialed = HAL_MODEM_SMS_UNINITIALED
};

/**
 * @name HalModemSmsRxMsgHandler
 * @brief 处理接收到的SMS消息。
 *        该函数被调用以处理来自MODEM的接收到的SMS消息。它首先验证输入参数的有效性，
 *        然后将消息信息（发件人地址、消息内容、消息长度）复制到本地结构体中，
 *        最后如果已注册了消息接收回调函数，则调用该回调函数通知应用程序。
 * @param msgRef 指向接收到的SMS消息的引用。
 * @param contextPtr 上下文指针，用于传递给回调函数，以便于回调函数中使用。
 * @return 函数无返回值
 */
static void HalModemSmsRxMsgHandler(QL_SMS_MsgRef msgRef,void* contextPtr)
{
    HalModemSmsInfo_t smsInfo;
    if (!msgRef)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    strcpy((char *)smsInfo.sourceAddr, msgRef->src_addr);
    memcpy(smsInfo.msgContent, msgRef->sms_data, msgRef->sms_data_len);
    smsInfo.msgLen = msgRef->sms_data_len;
    
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

    retVal = QL_SMS_Client_Init(&gSmsPriv.smsHdl);

    if (E_QL_SUCCESS != retVal)
    {
        LOG_Error("%s init fail.", __FUNCTION__);
        return ret;       
    }

    retVal = QL_SMS_AddRxMsgHandler(HalModemSmsRxMsgHandler, NULL);

    if (E_QL_SUCCESS != retVal)
    {
        LOG_Error("%s AddRxMsgHandler fail.", __FUNCTION__);
        QL_SMS_Client_Deinit(gSmsPriv.smsHdl);
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

    if (HAL_MODEM_SMS_INITIALED != gSmsPriv.initialed)
    {
        LOG_Error("%s have init.", __FUNCTION__);
        return ret;
    }

    QL_SMS_Client_Deinit(gSmsPriv.smsHdl);
    gSmsPriv.smsHdl = 0x0;
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
    ql_sms_info_t smsInfo;
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
    memset(&smsInfo, 0x0, sizeof(ql_sms_info_t));
    smsInfo.storage_index = -1;
    smsInfo.format =E_QL_SMS_FORMAT_BINARY_DATA;
    strcpy(smsInfo.src_addr, (char *)phoneNum);
    memcpy(smsInfo.sms_data,mesg, mesgLen);
    smsInfo.sms_data_len = mesgLen;

    retVal = QL_SMS_Send_Sms(gSmsPriv.smsHdl, &smsInfo);
    if (E_QL_SUCCESS != retVal)
    {
        LOG_Error("%s send fail.", __FUNCTION__);
        return ret;       
    }
    
    ret = RET_OK;
    return ret;    
}