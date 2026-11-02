#ifndef _PM_IPC_RPC_H_
#define _PM_IPC_RPC_H_

#include <stdint.h>

/*
* @brief include files here
*
*/
#ifdef __cplusplus
extern "C" {
#endif

uint8_t GetSmsSubWakeupType(void);

void PmIpcSendPmState(uint8_t WorkMode, uint8_t PmState, uint8_t WakeupSrc, uint8_t SleepAlarmEventId);

void PmIpcInit(void);

void PmIpcExit(void);

#ifdef __cplusplus
}
#endif

#endif //_PM_IPC_RPC_H_


