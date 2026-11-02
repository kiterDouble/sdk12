#include "message.h"
/*Diagnostic power mode information response*/

#ifndef __DOIP_MESSAG_4002_H__
#define __DOIP_MESSAG_4002_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name DoipParserMessage4002
 * @brief 解析车辆实体状态响应消息
 *        本函数用于解析接收到的数据，提取出车辆实体的状态信息，并存储到提供的结果结构体中。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据的长度。
 * @param resPL 指向用于存储解析结果的结构体的指针。
 * @return 若解析成功，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipParserMessage4002(const uint8_t *data, uint32_t dataLen, VehicleEntityStatusRes_t *resPL);

/**
 * @name DoipGenMessage4002
 * @brief 生成用于4002错误消息的DOIP包
 *        本函数用于根据给定的车辆实体状态响应信息，生成一个DOIP协议消息包。该消息包的类型为实体状态响应（VehicleEntityStatusRes）。
 * @param resPL 指向VehicleEntityStatusRes_t结构体的指针，包含车辆实体状态的响应数据。
 * @param data 指向用于存储生成消息包数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针，函数执行后，会更新为实际生成的消息包长度。
 * @return 返回生成消息包的操作结果，成功返回RET_OK，错误返回RET_ERR。
 */
int32_t DoipGenMessage4002(const VehicleEntityStatusRes_t *resPL, uint8_t *data, uint32_t *dataLen);

/**
 * @name DoipCheckMessage4002
 * @brief 检查Doip消息是否为特定的实体状态响应消息。
 * @param data 指向Doip消息数据的指针。
 * @param dataLen Doip消息数据的长度。
 * @return 如果消息是有效的实体状态响应消息则返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipCheckMessage4002(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif