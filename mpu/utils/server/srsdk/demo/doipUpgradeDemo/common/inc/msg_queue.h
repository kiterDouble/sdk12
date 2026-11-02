#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void* MsgQueueHandle_t;
int32_t CreateMsgQueue(MsgQueueHandle_t *qHdl);
int32_t SendMsgQueue(MsgQueueHandle_t qHdl, uint8_t *data, uint32_t len);
int32_t RecvMsgQueue(MsgQueueHandle_t qHdl, uint8_t *data, uint32_t *plen, long ms);
int32_t DestroyMsgQueue(MsgQueueHandle_t qHdl);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif