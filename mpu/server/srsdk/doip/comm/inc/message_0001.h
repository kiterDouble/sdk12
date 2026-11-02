#include "message.h"
/*Vehicle identification request message*/

#ifndef __DOIP_MESSAG_0001_H__
#define __DOIP_MESSAG_0001_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name DoipParserMessage0001
 * @brief 解析Doip消息0001
 *        本函数用于解析车辆识别请求的消息。首先会校验输入的数据是否有效，然后解析消息头，
 *        并检查消息类型是否符合预期。只有当所有检查都通过时，才返回成功。
 * @param data 指向待解析数据的指针。
 * @param dataLen 数据长度。
 * @return 若解析成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipParserMessage0001(const uint8_t *data, uint32_t dataLen);

/**
 * @name DoipGenMessage0001
 * @brief 生成Doip消息0001
 *        本函数用于生成一条车辆识别请求的消息。首先会校验输入参数的合法性，然后构造消息的头部，
 *        并调用DoipGeneratePacket函数完成消息体的生成。 
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针。函数执行后，此处将更新为实际使用的缓冲区长度。
 * @return 返回生成消息的操作结果，成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t DoipGenMessage0001(uint8_t *data, uint32_t *dataLen);

/**
 * @name DoipCheckMessage0001
 * @brief 检查Doip消息0001的合法性。
 * @param data 指向Doip消息数据的指针。
 * @param dataLen Doip消息数据的长度。
 * @return 如果消息合法，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipCheckMessage0001(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif