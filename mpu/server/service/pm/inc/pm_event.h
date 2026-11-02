#ifndef _PM_EVENT_H_
#define _PM_EVENT_H_

#include <stdint.h>
#include <glib.h>

/*
* @brief include files here
*
*/
#ifdef __cplusplus
extern "C" {
#endif

uint8_t PmSchedGetPmState(void);

void PmSchedSetPmState(uint8_t PmState);

int8_t PmSleepEventListIsEmpty(void);

int8_t PmSleepEventIsExist(char *Event);

int8_t SetPmSleepEvent(char *Event);

int8_t CleanPmSleepEvent(char *Event);

gboolean MonitorUpdateTime();

gboolean MonitorEcallTime();

gpointer KeepWakeup();

#ifdef __cplusplus
}
#endif

#endif //_PM_EVENT_H_



