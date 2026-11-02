#ifndef __MODEM_SMS_H__
#define __MODEM_SMS_H__
#include <stdint.h>
#include "modem_data.h"
#include "hal_modem_sms.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name ModemSrvSmsRecvHandle
 * @brief 接收到短信处理函数
 *        该函数负责处理来自Modem的短信接收事件，将短信内容和来源信息封装成消息，
 *        然后将该消息发送到所有相关队列中。
 * @param mesg 指向包含短信信息的结构体的指针。该结构体包括短信内容、长度和来源地址。
 * @return 函数无返回值
 */
void ModemSrvSmsRecvHandle(HalModemSmsInfo_t *mesg);

/**
 * @name ModemSrvSmsInit
 * @brief 初始化Modem的SMS服务。
 *        该函数负责初始化Modem的SMS处理机制，确保能够接收和处理SMS消息。
 * @param 无
 * @return int32_t 返回初始化操作的结果状态码。成功时返回0，失败时返回错误码。
 */
int32_t ModemSrvSmsInit(void);

/**
 * @name ModemSrvSendSmsMesgProc
 * @brief 发送SMS消息处理函数。
 * @param reqData 指向请求数据的指针，包含要发送的SMS消息的信息。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针，用于存储操作结果和响应数据的长度。
 * @param respLen 指向响应数据长度的指针，用于输出实际的响应数据长度。
 * @param userdata 用户数据，通常用于传递额外的上下文信息，本函数中用于访问Modem服务数据。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvSendSmsMesgProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

#ifdef __cplusplus
}
#endif
#endif