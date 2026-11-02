/*Vehicle identification request message with VIN*/

#ifndef __DOIP_MESSAG_0003_H__
#define __DOIP_MESSAG_0003_H__
#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @name DoipParserMessage0003
 * @brief 解析DoIP消息0003，提取车辆识别码（VIN）。
 * @param data 指向DoIP数据包起始位置的指针。
 * @param dataLen DoIP数据包的长度。
 * @param reqPL 指向存储解析结果的结构体的指针。
 * @return 若成功提取VIN码则返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipParserMessage0003(const uint8_t *data, uint32_t dataLen, VehicleIDReqWithVin_t *reqPL);

/**
 * @name DoipGenMessage0003
 * @brief 生成车辆ID请求消息（基于VIN）。
 * @param reqPL 指向包含车辆VIN的请求结构体的指针。
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针，输入时应包含预期的最大长度。
 * @return 若成功，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipGenMessage0003(const VehicleIDReqWithVin_t *reqPL, uint8_t *data, uint32_t *dataLen);

/**
 * @name DoipCheckMessage0003
 * @brief 检查Doip消息是否符合指定格式和内容。
 * @param data 指向待检查数据的指针。
 * @param dataLen 数据长度。
 * @return 如果消息检查通过，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipCheckMessage0003(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif