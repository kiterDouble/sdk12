/*Routing activation request*/

#ifndef __DOIP_MESSAG_0005_H__
#define __DOIP_MESSAG_0005_H__
#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name DoipParserMessage0005
 * @brief 解析0005消息，提取路由激活请求信息
 * @param data 指向DOIP数据包起始位置的指针
 * @param dataLen DOIP数据包的长度
 * @param reqPL 指向要填充的路由激活请求结构体的指针
 * @return 成功返回RET_OK，否则返回RET_ERR
 */
int32_t DoipParserMessage0005(const uint8_t *data, uint32_t dataLen, RoutineActivationReq_t *reqPL);

/**
 * @name DoipGenMessage0005
 * @brief 生成0005消息：路由激活请求
 *        本函数用于根据输入的路由激活请求信息，生成相应的DOIP消息。
 * @param reqPL 指向RoutineActivationReq_t结构体的指针，包含路由激活的请求参数。
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针，输入时应包含足够的长度，输出时将更新为实际使用的长度。
 * @return 返回生成消息的操作结果，成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t DoipGenMessage0005(const RoutineActivationReq_t *reqPL, uint8_t *data, uint32_t *dataLen);

/**
 * @name DoipCheckMessage0005
 * @brief 检查DoIP消息是否为路由激活请求。
 * @param data 指向DoIP数据包的指针。
 * @param dataLen 数据包长度。
 * @return 如果消息是路由激活请求且格式正确，则返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipCheckMessage0005(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif