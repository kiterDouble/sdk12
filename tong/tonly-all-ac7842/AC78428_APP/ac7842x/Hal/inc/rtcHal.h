#ifndef __RTC_HAL_H__
#define __RTC_HAL_H__

#include <stdint.h>
#include "Std_Types.h"
#include "Device_Types.h"

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} DateTime_t;


typedef struct {
    void (*Init)(void);
    void (*DeInit)(void);

    

    /* 休眠与唤醒 */
    void (*Sleep)(void);
    void (*Wakeup)(void);
} Hal_Rtc_t;

void RtcHal_Init(void);


/*************************************************
  Function:     TimerHal_RtcRemoteTimeSync
  Description:  Remote time sync
  Input:        None
  Output:       None
  Return:       Common_ResultStatusType
  Others:       None
*************************************************/
Hal_StatusType TimerHal_RtcRemoteTimeSync(const DateTime_t *time);


/*************************************************
  Function:     RtcHal_SetSeconds
  Description:  set rtc time
  Input:        utc :enter the time point
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void RtcHal_SetSeconds(uint32_t utc);

/*************************************************
  Function:     RtcHal_GetSeconds
  Description:  Get current time
  Input:        pUtc :the current time obtained
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void RtcHal_GetSeconds(uint32_t *pUtc);


void RtcHal_Sleep(void);

void RtcHal_Wakeup(void);


#endif  /* __RTC_HAL_H__ */