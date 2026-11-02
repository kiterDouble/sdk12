#include "message.h"
/*Vehicle announcement/Vehicle identification response message*/

#ifndef __DOIP_MESSAG_0004_H__
#define __DOIP_MESSAG_0004_H__

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @name DoipParserMessage0004
 * @brief 解析Doip消息0004，用于处理车辆识别响应。
 * @param data 指向Doip消息数据的指针。
 * @param dataLen Doip消息数据的长度。
 * @param resPL 指向用于存储解析结果的VehicleAnnoRes_t结构体的指针。
 * @return 若解析成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipParserMessage0004(const uint8_t *data, uint32_t dataLen, VehicleAnnoRes_t *resPL);

/**
 * @name DoipGenMessage0004
 * @brief 生成车辆识别消息DoipGenMessage0004
 *        本函数用于根据给定的车辆信息资源生成一个特定格式的车辆识别消息。
 * @param resPL 指向包含车辆信息资源的指针。车辆信息包括VIN码、逻辑地址、电子标识符和组标识符。
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储消息数据长度的变量的指针。在函数调用前，该变量应初始化为预期的消息长度。
 *                函数执行后，如果成功，该变量将被更新为实际生成的消息长度。
 * @return 返回生成消息的操作结果。成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t DoipGenMessage0004(const VehicleAnnoRes_t *resPL, uint8_t *data, uint32_t *dataLen);

/**
 * @name DoipCheckMessage0004
 * @brief 检查Doip消息是否符合0004规范
 * @param data 指向Doip数据包的指针
 * @param dataLen 数据包长度
 * @return 若检查通过返回RET_OK，否则返回RET_ERR
 */
int32_t DoipCheckMessage0004(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif