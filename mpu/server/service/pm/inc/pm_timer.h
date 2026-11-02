#ifndef _PM_TIMER_H_
#define _PM_TIMER_H_

#include <stdint.h>

/*
* @brief include files here
*
*/
#ifdef __cplusplus
extern "C" {
#endif

uint8_t GetTimerWakeupSrc(void);

int8_t PmSleepAlarmEventIdIsExist(uint8_t SleepAlarmEventId);

int8_t CreatePmSleepAlarm(uint8_t SleepAlarmEventId, uint32_t SleepAlarmSecond, uint16_t WakeupOption);

int8_t CreatePmSleepAlarmEveryDay(uint8_t SleepAlarmEventId, uint8_t SleepAlarmHour, uint8_t SleepAlarmMinute, uint16_t WakeupOption);

int8_t DestroyPmSleepAlarm(uint8_t SleepAlarmEventId);

int32_t GetTimerMinTimesToFreePmLock();

#ifdef __cplusplus
}
#endif

#endif //_PM_TIMER_H_




