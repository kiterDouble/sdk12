#include "message.h"
/*Vehicle identification request message with EID*/

#ifndef __DOIP_MESSAG_0002_H__
#define __DOIP_MESSAG_0002_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name DoipParserMessage0002
 * @brief 解析车辆ID请求消息（消息类型0002）
 *        该函数用于解析接收到的数据，提取车辆电子标识（EID），并将其存储在提供的请求参数结构中。
 * @param data 指向接收到的数据包的指针。
 * @param dataLen 接收到的数据包的长度。
 * @param reqPL 指向存储车辆ID请求信息的结构体的指针。
 * @return 若解析成功，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipParserMessage0002(const uint8_t *data, uint32_t dataLen, VehicleIDReqWithEid_t *reqPL);

/**
 * @name DoipGenMessage0002
 * @brief 生成车辆ID请求消息（基于EID）。
 * @param reqPL 指向包含车辆EID（电子识别码）的请求结构体的指针。
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针，函数执行后会更新为实际使用的长度。
 * @return 返回生成消息的操作结果，成功返回RET_OK，错误返回RET_ERR。
 */
int32_t DoipGenMessage0002(const VehicleIDReqWithEid_t *reqPL, uint8_t *data, uint32_t *dataLen);

/**
 * @name DoipCheckMessage0002
 * @brief 检查Doip消息0002的合法性。
 * @param data 指向Doip消息数据的指针。
 * @param dataLen Doip消息数据的长度。
 * @return 如果消息合法，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipCheckMessage0002(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif