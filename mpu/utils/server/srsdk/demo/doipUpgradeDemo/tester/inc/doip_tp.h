#ifndef __DOIP_TP_H__
#define __DOIP_TP_H__
#include "doip_tester.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
int32_t DoipTpInit(DoipTesterConnNotifyCb connNotify, DoipTesterOnlineNotifyCb onlineCb);
int32_t DoipTpConnect(const DoipTesterNode_t *node);
int32_t DoipTpTransmit(uint32_t targetLa, const uint8_t *data, uint32_t dataLen);
int32_t DoipTpGetResponse(uint8_t *data, uint32_t *dataLen);
int32_t DoipTpDisconnect(void);
int32_t DoipTpDeinit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif