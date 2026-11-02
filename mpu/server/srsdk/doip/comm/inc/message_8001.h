#include "message.h"
/*Diagnostic Message*/

#ifndef __DOIP_MESSAG_8001_H__
#define __DOIP_MESSAG_8001_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name DoipParserMessage8001
 * @brief 解析DOIP消息（8001消息）
 *        此函数用于解析接收到的DOIP（车辆诊断over以太网协议）消息，检查消息是否为期望的诊断请求类型。
 * @param data 指向接收到的数据缓冲区的指针。
 * @param dataLen 数据缓冲区的长度。
 * @return 如果解析成功且消息类型匹配，则返回RET_OK（定义为成功返回值）；否则返回RET_ERR（定义为错误返回值）。
 */
int32_t DoipParserMessage8001(const uint8_t *data, uint32_t dataLen);

/**
 * @name DoipGenMessage8001
 * @brief 生成DOIP消息(8001)，用于诊断通信。
 * @param sourceLa 源逻辑地址，16位无符号整型。
 * @param targetLa 目标逻辑地址，16位无符号整型。
 * @param udsData UDS（统一诊断服务）数据，指向数据的指针。
 * @param udsLen UDS数据长度，无符号整型。
 * @param data 指向用于存放生成消息数据的缓冲区的指针。
 * @param dataLen 指向用于指示缓冲区长度的变量的指针。
 * @return 若成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipGenMessage8001(uint32_t sourceLa, 
                            uint32_t targetLa, 
                            const uint8_t *udsData, 
                            uint32_t udsLen, 
                            uint8_t *data, 
                            uint32_t *dataLen);


/**
 * @name DoipCheckMessage8001
 * @brief 检查Doip消息是否符合8001协议规范
 * @param data 指向Doip消息数据的指针
 * @param dataLen Doip消息数据的长度
 * @return 若消息符合规范则返回RET_OK，否则返回RET_ERR
 */          
int32_t DoipCheckMessage8001(const uint8_t *data, uint32_t dataLen);
#ifdef __cplusplus
}
#endif
#endif