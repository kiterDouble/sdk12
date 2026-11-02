/*Alive check response*/

#ifndef __DOIP_MESSAG_0008_H__
#define __DOIP_MESSAG_0008_H__
#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name DoipParserMessage0008
 * @brief 解析DOIP消息0008，用于处理alive检查响应。
 * @param data 指向DOIP数据包的指针。
 * @param dataLen DOIP数据包的长度。
 * @param resPL 指向AliveCheckRes_t结构体的指针，用于存储解析结果。
 * @return 返回解析结果的状态码。RET_OK表示成功，RET_ERR表示错误。
 */
int32_t DoipParserMessage0008(const uint8_t *data, uint32_t dataLen, AliveCheckRes_t *resPL);

/**
 * @name DoipGenMessage0008
 * @brief 生成DOIP消息0008：响应 alive 检查。
 * @param resPL 指向 AliveCheckRes_t 结构体的指针，包含 alive 检查的响应结果。
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针，入参时指定期望的最大长度，出参时返回实际使用的长度。
 * @return 返回生成消息的操作结果，成功时返回 RET_OK，错误时返回其他错误码。
 */
int32_t DoipGenMessage0008(const AliveCheckRes_t *resPL, uint8_t *data, uint32_t *dataLen);

/**
 * @name DoipCheckMessage0008
 * @brief 检查Doip消息0008的合法性。
 *        该函数用于校验接收到的数据是否为期望的Doip alive检查响应消息，
 *        根据消息的类型和长度进行验证。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据的长度。
 * @return 如果消息验证成功，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipCheckMessage0008(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif