#ifndef __CAROTA_SLEEP_HANDLE_H__
#define __CAROTA_SLEEP_HANDLE_H__ 
#include <glib.h>
#include <time.h>.


#ifdef __cplusplus
extern "C"
{
#endif

#define PM_SLEEP_EVENT_NOTIFY_CAROTA "sleep_notify_carota"
#define DOTA_NOTIFY_PM_SLEEP_EVENT "dota_notify_pm_sleep_event"
#define FOTA_NOTIFY_PM_SLEEP_EVENT "fota_notify_pm_wake_event"
void* carotaTimerThread(void* arg);
void CarotaSetPmState(uint8_t pmState);
uint8_t GetPmState();




#ifdef __cplusplus
}
#endif


#endif