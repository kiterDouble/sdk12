
#ifndef __DOIP_MESSAGE_PROC_H__
#define __DOIP_MESSAGE_PROC_H__
#include "message_0000.h"
#include "message_0001.h"
#include "message_0002.h"
#include "message_0003.h"
#include "message_0004.h"
#include "message_0005.h"
#include "message_0006.h"
#include "message_0007.h"
#include "message_0008.h"
#include "message_4001.h"
#include "message_4002.h"
#include "message_4003.h"
#include "message_4004.h"
#include "message_8001.h"
#include "message_8002.h"
#include "message_8003.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @name DoipCheckHeader
 * @brief 检查DoIP消息头是否合法。
 * @param data 指向DoIP消息数据的指针。
 * @param dataLen 消息数据的长度。
 * @param err 错误码的输出参数，用于返回错误代码。
 * @return RET_OK表示成功，RET_ERR表示失败。
 */
int32_t DoipCheckHeader(const uint8_t *data, uint32_t dataLen, uint32_t *err);
#ifdef __cplusplus
}
#endif
#endif