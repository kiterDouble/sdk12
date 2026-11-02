#ifndef _PM_SCHED_H_
#define _PM_SCHED_H_

#include <stdint.h>

/*
* @brief include files here
*
*/
#ifdef __cplusplus
extern "C" {
#endif

uint8_t PmSchedGetWorkMode(void);

void PmSchedSetWorkMode(uint8_t WorkMode);

void PmSchedSetIsSleep(uint8_t IsSleep);

void PmSchedInit(void);

void PmSchedExit(void);

#ifdef __cplusplus
}
#endif

#endif //_PM_SCHED_H_




