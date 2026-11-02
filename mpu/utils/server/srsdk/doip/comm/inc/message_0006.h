#include "message.h"
/*Routing activation response*/

#ifndef __DOIP_MESSAG_0006_H__
#define __DOIP_MESSAG_0006_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name DoipParserMessage0006
 * @brief 解析0006消息的函数，用于处理路由激活响应。
 * @param data 指向DOIP数据包的指针。
 * @param dataLen 数据包长度。
 * @param resPL 指向要填充的结果结构体的指针。
 * @return 返回处理结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t DoipParserMessage0006(const uint8_t *data, uint32_t dataLen, RoutineActivationRes_t *resPL);

/**
 * @name DoipGenMessage0006
 * @brief 生成DOIP消息0006，用于响应路由激活请求。
 * @param resPL 指向RoutineActivationRes_t结构体的指针，包含测试设备和实体设备信息以及状态码。
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针，输入时应包含足够的空间，输出时将包含实际使用的长度。
 * @return 返回生成消息的操作结果，成功时返回RET_OK，错误时返回其他错误码。
 */
int32_t DoipGenMessage0006(const RoutineActivationRes_t *resPL, uint8_t *data, uint32_t *dataLen);

/**
 * @name DoipCheckMessage0006
 * @brief 检查DoIP消息是否为路由激活响应
 *        本函数用于校验接收到的DoIP消息是否是路由激活的响应消息。它首先验证消息的数据指针和长度是否合法，
 *        然后解析消息头，检查消息类型和负载长度是否符合预期。
 * 
 * @param data 指向接收到的DoIP数据的指针。
 * @param dataLen 接收到的数据长度。
 * @return 若消息合法且为路由激活响应，则返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipCheckMessage0006(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif