#ifndef _PM_MODEM_H_
#define _PM_MODEM_H_

#include <stdint.h>

/*
* @brief include files here
*
*/
#ifdef __cplusplus
extern "C" {
#endif

void PmModemInit(void);

void PmModemExit(void);

void PmModemSleep(void);

int8_t PmModemWakeup(uint8_t WakeupSrc, uint8_t WakeupOption);

uint8_t PmSchedGetWakeupSrc(void);

void PmSchedSetWakeupSrc(uint8_t WakeupSrc);

void PmModemSleepQuicklyFlagSet(void);

void PmModemSleepQuicklyFlagClear(void);

void PmModemCallCallBack(const uint8_t *data, const uint16_t len, void* userdata);

void PmModemSmsCallBack(const uint8_t *data, const uint16_t len, void* userdata);

void PmModemEcallIncomingCallBack(const uint8_t *data, const uint16_t len, void* userdata);

#ifdef __cplusplus
}
#endif

#endif //_PM_MODEM_H_



