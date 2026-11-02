#include "message.h"
/*Diagnostic power mode information request*/

#ifndef __DOIP_MESSAG_4003_H__
#define __DOIP_MESSAG_4003_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name DoipParserMessage4003
 * @brief 解析4003消息
 *        本函数用于解析接收到的数据，检查其是否为期望的电力模式请求消息。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据的长度。
 * @return 如果解析成功且消息类型匹配，则返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipParserMessage4003(const uint8_t *data, uint32_t dataLen);

/**
 * @name DoipGenMessage4003
 * @brief 生成4003消息（电源模式请求）
 *        本函数用于生成一条与电源模式请求相关的DOIP消息。首先会校验输入参数的有效性，
 *        然后构造消息的头部，并调用另一个函数完成整个消息包的生成。
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针。函数执行后，此处将更新为实际使用的长度。
 * @return 返回生成消息的操作结果，成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t DoipGenMessage4003(uint8_t *data, uint32_t *dataLen);

/**
 * @name DoipCheckMessage4003
 * @brief 检查Doip消息是否为4003类型的消息（电源模式请求） 
 * @param data 指向Doip消息数据的指针。
 * @param dataLen Doip消息数据的长度。
 * @return 如果消息是有效的4003类型消息，则返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipCheckMessage4003(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif