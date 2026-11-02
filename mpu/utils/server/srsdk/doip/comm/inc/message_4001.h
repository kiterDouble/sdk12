#include "message.h"
/*DoIP entity status request*/

#ifndef __DOIP_MESSAG_4001_H__
#define __DOIP_MESSAG_4001_H__

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @name DoipParserMessage4001
 * @brief 解析4001消息
 *        本函数用于解析接收到的数据，检查其是否为期望的实体状态请求消息。若是，则返回成功, 否则返回错误。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据的长度。
 * @return 若解析成功，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipParserMessage4001(const uint8_t *data, uint32_t dataLen);

/**
 * @name DoipGenMessage4001
 * @brief 生成用于4001实体状态请求的消息。
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针。函数执行后，此处将更新为实际生成消息的长度。
 * @return 若成功，则返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipGenMessage4001(uint8_t *data, uint32_t *dataLen);

/**
 * @name DoipCheckMessage4001
 * @brief 检查Doip消息是否为4001类型的消息（实体状态请求）
 * @param data 指向Doip数据包的指针
 * @param dataLen 数据包长度
 * @return 如果消息是有效的4001类型消息，则返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipCheckMessage4001(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif