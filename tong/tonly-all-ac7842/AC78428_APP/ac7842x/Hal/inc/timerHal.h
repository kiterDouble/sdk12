/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name:      timerHal.h
 Author:         JiaYingNan
 Created Time:   2023.12.25
 Description:    timer Indicates the function related to the interface
 Others:         none
*************************************************/
#ifndef _TIMERHAL_H
#define _TIMERHAL_H

/****************************** include ***************************************/
#include <stdint.h>

/****************************** Macro Definitions ******************************/
/****************************** Type Definitions ******************************/
typedef void(* typeTimerCallbackPtr)(void);

/* ---- 0xB203 当前时间结构体 ---- */
typedef struct
{
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} TimeData_t;

/****************************** Global Variables ******************************/
/****************************** Function Declarations *************************/
/****************************** Public Function Implementations ***************/
/*************************************************
  Function:     TimerHalInit
  Description:  Timer init
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void TimerHalInit(void);

/*************************************************
  Function:     TimerHalOpen
  Description:  Timer open handle
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
int16_t TimerHalOpen(void);

/*************************************************
  Function:     TimerHalStartTime
  Description:  Timer starts the timer
  Input:        timerHandle :timer handle
                time_ms :unit ms
  Output:       None
  Return:       0 :success
                1 :fail
  Others:       None
*************************************************/
int16_t TimerHalStartTime(int16_t timerHandle, uint32_t time_ms);

/*************************************************
  Function:     TimerHalStopTime
  Description:  Timer stop the timer
  Input:        timerHandle :timer handle
  Output:       None
  Return:       0 :success
                1 :fail
  Others:       None
*************************************************/
int16_t TimerHalStopTime(int16_t timerHandle);

/*************************************************
  Function:     TimerHalIsTimeout
  Description:  Timer timeout 
  Input:        timerHandle :timer handle
  Output:       None
  Return:       0 :success
                1 :fail
  Others:       None
*************************************************/
int16_t TimerHalIsTimeout(int16_t timerHandle);

/*************************************************
  Function:     TimerHalGetRemainder
  Description:  Get timer remainder
  Input:        timerHandle :timer handle
  Output:       None
  Return:       0 :success
                1 :fail
  Others:       None
*************************************************/
uint32_t TimerHalGetRemainder(int16_t timerHandle);

uint32_t TimerHalGetMs(void);

/*************************************************
  Function:     TimerHalSetRtcTime
  Description:  set rtc time
  Input:        utc :enter the time point
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void TimerHalSetRtcTime(uint32_t utc);

/*************************************************
  Function:     TimerHalGetRtcTime
  Description:  Get current time
  Input:        pUtc :the current time obtained
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void TimerHalGetRtcTime(uint32_t *pUtc);

/*************************************************
  Function:     TimerHalSetMode
  Description:  Set timer mode
  Input:        mode :timer mode
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void TimerHalSetMode(uint8_t mode);

/*************************************************
  Function:     TimerHalSetCallback
  Description:  Set timer callback function
  Input:        timerHandle :timer handle
                TimerCallback :timer callback function
  Output:       None
  Return:       0 :success
                1 :fail
  Others:       None
*************************************************/
int16_t TimerHalSetCallback(int16_t timerHandle, typeTimerCallbackPtr TimerCallback);

/*************************************************
  Function:     TimerHal_GetRtcTime
  Description:  Get rtc time
  Input:        time :time point
  Output:       None
  Return:       uint8_t
  Others:       None
*************************************************/
uint8_t TimerHal_GetRtcTime(TimeData_t *time);

/*************************************************
  Function:     TimerHal_RtcRemoteTimeSync
  Description:  Remote time sync
  Input:        None
  Output:       None
  Return:       uint8_t
  Others:       None
*************************************************/
uint8_t TimerHal_RtcRemoteTimeSync(const TimeData_t *time);

#endif // _TIMERHAL_H