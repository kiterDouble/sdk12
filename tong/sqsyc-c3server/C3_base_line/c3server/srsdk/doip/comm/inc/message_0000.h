/*Generic DoIP header negative acknowledge*/

#ifndef __DOIP_MESSAG_0000_H__
#define __DOIP_MESSAG_0000_H__
#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name DoipParserMessage0000
 * @brief 解析Doip消息0000，提取通用头部否定应答信息。
 * @param data 指向Doip数据包起始位置的指针。
 * @param dataLen 数据包长度。
 * @param ghNack 指向GeneralHeaderNack_t结构体的指针，用于存储解析结果。
 * @return 若成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipParserMessage0000(const uint8_t *data, uint32_t dataLen, GeneralHeaderNack_t *ghNack);

/**
 * @name DoipGenMessage0000
 * @brief 生成DOIP协议中的消息0000，主要用于发送否定应答。
 * @param ghNack 指向GeneralHeaderNack_t结构的指针，包含否定应答的详细信息。
 * @param data 指向用于存储生成消息的数据缓冲区的指针。
 * @param dataLen 指向存储数据缓冲区长度的变量的指针，入参和出参，用于输入时指定数据缓冲区的初始长度，
 *                输出时返回实际使用的数据长度。
 * @return 返回生成消息的操作结果，成功时返回RET_OK，错误时返回其他错误码。
 */
int32_t DoipGenMessage0000(const GeneralHeaderNack_t *ghNack, uint8_t *data, uint32_t *dataLen);

/**
 * @name DoipCheckMessage0000
 * @brief 检查Doip消息是否为特定的否定应答类型。
 * @param data 指向Doip消息数据的指针。
 * @param dataLen Doip消息数据的长度。
 * @return 如果消息是预期的否定应答类型则返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipCheckMessage0000(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif