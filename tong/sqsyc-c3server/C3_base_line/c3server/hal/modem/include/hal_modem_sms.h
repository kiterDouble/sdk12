#ifndef __HAL_MODEM_SMS_H__
#define __HAL_MODEM_SMS_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
//QL defined LENGTH
#define HAL_MODEM_SMS_MAX_ADDRESS_LENGTH  (252)
#define HAL_MODEM_SMS_MAX_CONTENT_LENGTH  (1440)
typedef struct 
{
    int32_t msgLen;
    uint8_t msgContent[HAL_MODEM_SMS_MAX_CONTENT_LENGTH];
    uint8_t sourceAddr[HAL_MODEM_SMS_MAX_ADDRESS_LENGTH];
}HalModemSmsInfo_t;//短信内容

/**
 * @name HalModemSmsRecvCallback
 * @brief 接收短信回调
 * @param mesg 输入参数, 短信内容
 * @return 函数无返回值
 */
typedef void (*HalModemSmsRecvCallback)(HalModemSmsInfo_t *mesg);

/**
 * @name HalModemSmsInit
 * @brief 初始化Modem的SMS功能。
 * @param recvCb 接收短信的回调函数指针。当有短信收到时，此回调函数会被调用。
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
int32_t HalModemSmsInit(HalModemSmsRecvCallback recvCb);
/**
 * @name HalModemSmsDeinit
 * @brief 反初始化SMS功能
 *        该函数用于当需要停止使用短信功能时，对之前初始化的短信模块进行反初始化操作。
 *        它将释放与短信功能相关的资源，并将模块状态设置为未初始化。
 * @param null
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
int32_t HalModemSmsDeinit(void);
/**
 * @name HalModemSmsSend
 * @brief 发送SMS短信
 * @param phoneNum 接收短信的手机号码，类型为uint8_t数组
 * @param mesg 要发送的短信内容，类型为uint8_t数组
 * @param mesgLen 短信内容的长度，单位为字节
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
int32_t HalModemSmsSend(const uint8_t *phoneNum, const uint8_t *mesg, uint8_t mesgLen);
#ifdef __cplusplus
}
#endif
#endif