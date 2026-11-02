#ifndef __MCU_IPC_MSG_H__
#define __MCU_IPC_MSG_H__
#include <stdint.h>
#include "sr_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct
{
    SYSQueMsgType_e type;
    uint32_t dataLen;
    uint8_t data[0];
}McuSrvRpcReq_t;

typedef struct
{
    SYSQueMsgType_e type;
    int32_t result;
}McuSrvRpcRes_t;
#ifdef __cplusplus
}
#endif
#endif 