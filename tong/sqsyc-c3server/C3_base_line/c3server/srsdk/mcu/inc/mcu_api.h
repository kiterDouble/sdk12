#ifndef __MCU_API_H__
#define __MCU_API_H__

#include "mcu_ipc_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    SYSQueMsgType_e type;
    uint32_t payloadLen;
    uint8_t* payload;
}McuMesg_t;

/**
 * @name McuSrvSendMesg
 * @brief 向mcu发送消息函数
 * @param msg:输入参数，发送的消息结构体
 * @return 成功返回0，失败返回小于0
 */
int32_t McuSrvSendMesg(const McuMesg_t *msg);
#ifdef __cplusplus
}
#endif
#endif