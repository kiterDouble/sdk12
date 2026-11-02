/*************************************************
 Copyright © 2025 SiRun (AnHui) . All rights reserved.
 File Name: timerHal.c
 Author: HuShangrui
 Created Time: 2026年7月10日
 Description: 
 Others: 
*************************************************/
/****************************** include ***************************************/
#include "stdio.h"
#include "timerHal.h"
#include "peripheralHal.h"
#include "mpuHal.h"
#include "logHal.h"
#include "Timer_Hal.h"
#include "Rtc_Hal.h"
#include "Core_Hal.h"

/****************************** Macro Definitions ******************************/
#define TIMER0_KPI_ENABLE                      (0)
#define PERIPHERAL_TIMER_INSTANCE_NUMBER       40
#define SRS_SIGNAL_PROCESS_ENABLE              1
#define AD_CONVERT_CYCLE_TIME                  5

#define RTC_DEBUG_ENABLE                       0
#define RTC_START_YEAR                         (2000U)
#define RTC_OSC32K_READY_TIMEOUT               (10000U)
#define SECONDS_PER_MINUTE                     (60U)
#define SECONDS_PER_HOUR                       (3600U)
#define SECONDS_PER_DAY                        (86400U)
#define DAYS_PER_YEAR                          (365U)
#define DAYS_PER_LEAP_YEAR                     (366U)
#define MONTHS_PER_YEAR                        (12U)
#define IS_LEAP_YEAR(year)       (((year) % 4U == 0U) && ((year) % 100U != 0U)) || ((year) % 400U == 0U)

/* Hardware Timer Channel for 1ms Tick */
#define MPU_HAL_TIMER_CHANNEL                  (0U) 

/****************************** Type Definitions ******************************/
typedef struct
{
    uint8_t UseFlag;
    uint8_t timerOnFlag;
    uint32_t TimerCount;
    typeTimerCallbackPtr TimerCallback;
} PeripheralTimer_t;

/****************************** Global Variables ******************************/
#if (TIMER0_KPI_ENABLE == 1)
static TimerIrqKpi_t g_timer0IrqKpi;
#endif
static PeripheralTimer_t g_peripheralTimer[PERIPHERAL_TIMER_INSTANCE_NUMBER];
static uint8_t g_adChannelCount = 0;
static uint32_t g_rtcSCount = 0;
static uint32_t g_rtcMsCount = 0;
static TimeData_t g_currentTime;
static uint32_t g_rtcBaseSeconds = 0U;
static TimeData_t g_baseTime;
static const uint8_t g_monthDays[] = {
    0U,  31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U
};

/****************************** Function Declarations *************************/
static void DelayRLTUnderFlowCallback(void);
static void Timer0Handler(void *Args);
static void TimerHal_TimInit(void);

static uint8_t TimerHal_RTCInit(void);
static uint32_t TimerHalTimeToSeconds(const TimeData_t *time);
static void TimerHalSecondsToTime(uint32_t seconds, TimeData_t *time);
static void TimerHalRtcSecCallback(uint32_t Event);
static void TimerHal_RtcTimeUpdate(void);
static uint8_t TimerHal_SetRtcTime(const TimeData_t *time);
static void TimerHal_RtcIrqEnable(void);
static void TimerHal_RtcIrqDisable(void);
#if (RTC_DEBUG_ENABLE == 1)
static void TimerHal_PrintTimeInfo(void);
#endif

/****************************** Public Function Implementations ***************/
static void DelayRLTUnderFlowCallback(void)
{
    static uint32_t adTimeCount = 0;

    for (uint32_t i = 0; i < PERIPHERAL_TIMER_INSTANCE_NUMBER; i++)
    {
        if (g_peripheralTimer[i].TimerCount > 0)
        {
            g_peripheralTimer[i].TimerCount--;
            if (g_peripheralTimer[i].TimerCount == 0) {
                if (g_peripheralTimer[i].TimerCallback != NULL) {
                    g_peripheralTimer[i].TimerCallback();
                }
            }
        }    
    }
    
    g_rtcMsCount++;
    if (g_rtcMsCount >= 1000)
    {
        g_rtcMsCount = 0;
        g_rtcSCount++;
    }
    
    adTimeCount++;
    if (adTimeCount >= AD_CONVERT_CYCLE_TIME)
    {
        __disable_irq();
        PeripheralHalAdcStart(g_adChannelCount);
        __enable_irq();
        g_adChannelCount++;
        if (g_adChannelCount >= 2)
        {
            g_adChannelCount = 0;
        }
        adTimeCount = 0;
    }
}

static void Timer0Handler(void *Args)
{
    (void)Args;

    MpuHalUartTimerCallBack();
    DelayRLTUnderFlowCallback();
}

/*************************************************
  Function:       TIM0_Init
  Description:    Initialize TIM0 module for timing interrupt function
  Input:          none
  Output:         none
  Return:         none
  Others:         Configures TIM0 with HSOSC clock source, up-counting mode
                  Sets up overflow interrupt with Timer0Handler callback
                  Enables TIM0 Overflow interrupt with priority level 7
*************************************************/
static void TimerHal_TimInit(void)
{
    Timer_Hal_Init(TIMER_CLOCK_SPLL);
    
    /* 2. 配置定时器通道参数 */
    Timer_Channel_ConfigType timerCfg;
    timerCfg.Mode = TIMER_MODE_0;           /* 设置为 32 bit 周期定时器模式 */
    timerCfg.Config = TIMER_IRQ_EN;
    timerCfg.TriggerSrc = TRIG_SOURCE_TIMER_CH0;                /* 内部触发源 */
    Timer_Hal_SetConfig(MPU_HAL_TIMER_CHANNEL, &timerCfg);
    
    /* 3. 注册中断回调函数 */
    Timer_Hal_InstallCallback(MPU_HAL_TIMER_CHANNEL, Timer0Handler, NULL);

    Timer_Hal_EnableInterrupt(MPU_HAL_TIMER_CHANNEL, TIMER_INT_EN);
    
    /* 5. 启动定时器 (1000us = 1ms) */
    uint32_t ticks_1ms = Timer_Hal_MicrosToTicks(1000U);
    Timer_Hal_Start(MPU_HAL_TIMER_CHANNEL, ticks_1ms);
}

/*************************************************
  Function:     TimerHalInit
  Description:  Timer init
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void TimerHalInit(void)
{
    TimerHal_TimInit();    
    TimerHal_RTCInit();    
}

/*************************************************
  Function:     TimerHalOpen
  Description:  Timer open handle
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
int16_t TimerHalOpen(void)
{
    int16_t timerHandle = -1, i;
    
    __disable_irq();
    for(i = 0; i < PERIPHERAL_TIMER_INSTANCE_NUMBER; i++)
    {
        if(g_peripheralTimer[i].UseFlag == 0)
        {
            g_peripheralTimer[i].UseFlag = 1;
            g_peripheralTimer[i].timerOnFlag = 0;
            g_peripheralTimer[i].TimerCount = 0;
            g_peripheralTimer[i].TimerCallback = NULL;
            timerHandle = i;
            break;
        }
    }
    __enable_irq();
    
    return timerHandle;  
}

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
int16_t TimerHalStartTime(int16_t timerHandle, uint32_t time_ms)
{
    if(timerHandle < 0 || timerHandle >= PERIPHERAL_TIMER_INSTANCE_NUMBER)
    {
        return -1;    
    }
    __disable_irq();
    g_peripheralTimer[timerHandle].TimerCount = time_ms;
    g_peripheralTimer[timerHandle].timerOnFlag = 1;
    __enable_irq();
    return 0;
}

/*************************************************
  Function:     TimerHalStopTime
  Description:  Timer stop the timer
  Input:        timerHandle :timer handle
  Output:       None
  Return:       0 :success
                1 :fail
  Others:       None
*************************************************/
int16_t TimerHalStopTime(int16_t timerHandle)
{
    if(timerHandle < 0 || timerHandle >= PERIPHERAL_TIMER_INSTANCE_NUMBER)
    {
        return -1;    
    }
    __disable_irq();
    g_peripheralTimer[timerHandle].timerOnFlag = 0;
    __enable_irq();
    return 0;
}

/*************************************************
  Function:     TimerHalIsTimeout
  Description:  Timer timeout 
  Input:        timerHandle :timer handle
  Output:       None
  Return:       0 :success
                1 :fail
  Others:       None
*************************************************/
int16_t TimerHalIsTimeout(int16_t timerHandle)
{
    uint8_t timerOnFlag;
    uint32_t timerCount;
    if (timerHandle < 0 || timerHandle >= PERIPHERAL_TIMER_INSTANCE_NUMBER)
    {
        return -1;    
    }

    __disable_irq();
    timerOnFlag = g_peripheralTimer[timerHandle].timerOnFlag;
    timerCount = g_peripheralTimer[timerHandle].TimerCount;
    __enable_irq();

    if (timerOnFlag == 0) return -2;
    if (timerCount) return 1;   

    return 0;
}

/*************************************************
  Function:     TimerHalGetRemainder
  Description:  Get timer remainder
  Input:        timerHandle :timer handle
  Output:       None
  Return:       0 :success
                1 :fail
  Others:       None
*************************************************/
uint32_t TimerHalGetRemainder(int16_t timerHandle)
{
    uint8_t timerOnFlag;
    uint32_t timerCount;
    if (timerHandle < 0 || timerHandle >= PERIPHERAL_TIMER_INSTANCE_NUMBER)
    {
        return 0;    
    }

    __disable_irq();
    timerOnFlag = g_peripheralTimer[timerHandle].timerOnFlag;
    timerCount = g_peripheralTimer[timerHandle].TimerCount;
    __enable_irq();

    if (timerOnFlag == 0) return 0;
    if (timerCount) return timerCount;
    
    return 0;
}

/*************************************************
  Function:     TimerHalGetMs
  Description:  Get system time in milliseconds
  Input:        None
  Output:       None
  Return:       System time in milliseconds
  Others:       Returns total milliseconds since system start
*************************************************/
uint32_t TimerHalGetMs(void)
{
    uint32_t ms;
    __disable_irq();
    ms = g_rtcSCount * 1000 + g_rtcMsCount;
    __enable_irq();
    return ms;
}

/*************************************************
  Function:     TimerHalSetRtcTime
  Description:  set rtc time
  Input:        utc :enter the time point
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void TimerHalSetRtcTime(uint32_t utc)
{
    __disable_irq();
    g_rtcMsCount = 0;
    g_rtcSCount = utc;
    __enable_irq();
}

/*************************************************
  Function:     TimerHalGetRtcTime
  Description:  Get current time
  Input:        pUtc :the current time obtained
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void TimerHalGetRtcTime(uint32_t *pUtc)
{
    __disable_irq();
    *pUtc = g_rtcSCount;
    __enable_irq();
}

/*************************************************
  Function:     TimerHalSetMode
*************************************************/
void TimerHalSetMode(uint8_t mode)
{
    if (mode != 0)
    {
        TimerHal_RtcIrqEnable();
        TimerHal_TimInit();
    }
    else
    {
        TimerHal_RtcIrqDisable();
        Timer_Hal_Stop(MPU_HAL_TIMER_CHANNEL);
        Core_Hal_DisableIrq(TIMER_CHANNEL0_IRQn);
        Core_Hal_ClearPendingIrq(TIMER_CHANNEL0_IRQn);
    }        
}

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
int16_t TimerHalSetCallback(int16_t timerHandle, typeTimerCallbackPtr TimerCallback)
{
    if (timerHandle < 0 || timerHandle >= PERIPHERAL_TIMER_INSTANCE_NUMBER)    
    {
        return -1;    
    }
    __disable_irq(); 
    g_peripheralTimer[timerHandle].TimerCallback = TimerCallback;
    __enable_irq(); 
    return 0;
}

/* =========================== TIME CONVERSION LOGIC =========================== */
/*************************************************
  Function:     TimerHalTimeToSeconds
  Description:  Convert time to seconds
  Input:        time :time point
  Output:       None
  Return:       seconds
  Others:       None
*************************************************/
static uint32_t TimerHalTimeToSeconds(const TimeData_t *time)
{
    uint32_t totalSeconds = 0U;
    uint16_t year;
    uint8_t month;
    
    for (year = RTC_START_YEAR; year < time->year; year++)
    {
        if (IS_LEAP_YEAR(year)) totalSeconds += (uint32_t)DAYS_PER_LEAP_YEAR * SECONDS_PER_DAY;
        else totalSeconds += (uint32_t)DAYS_PER_YEAR * SECONDS_PER_DAY;
    }
    
    for (month = 1U; month < time->month; month++)
    {
        if (month == 2U && IS_LEAP_YEAR(time->year)) totalSeconds += 29U * SECONDS_PER_DAY;
        else totalSeconds += (uint32_t)g_monthDays[month] * SECONDS_PER_DAY;
    }
    
    totalSeconds += (uint32_t)(time->day - 1U) * SECONDS_PER_DAY;
    totalSeconds += (uint32_t)time->hour * SECONDS_PER_HOUR;
    totalSeconds += (uint32_t)time->minute * SECONDS_PER_MINUTE;
    totalSeconds += time->second;
    
    return totalSeconds;
}

/*************************************************
  Function:     TimerHalSecondsToTime
  Description:  Convert seconds to time
  Input:        seconds :seconds
  Output:       time :time point
  Return:       None
  Others:       None
*************************************************/
static void TimerHalSecondsToTime(uint32_t seconds, TimeData_t *time)
{
    uint32_t remainingSeconds = seconds;
    uint16_t year = RTC_START_YEAR;
    uint8_t month, day;
    uint32_t daySeconds;
    
    while (1)
    {
        uint32_t yearDays = IS_LEAP_YEAR(year) ? DAYS_PER_LEAP_YEAR : DAYS_PER_YEAR;
        uint32_t yearSeconds = (uint32_t)yearDays * SECONDS_PER_DAY;
        
        if (remainingSeconds < yearSeconds) break;
        
        remainingSeconds -= yearSeconds;
        year++;
    }
    
    month = 1U;
    while (month <= MONTHS_PER_YEAR)
    {
        uint32_t monthDays;
        
        if (month == 2U && IS_LEAP_YEAR(year)) monthDays = 29U;
        else monthDays = g_monthDays[month];
        
        uint32_t monthSeconds = (uint32_t)monthDays * SECONDS_PER_DAY;
        
        if (remainingSeconds < monthSeconds) break;
        
        remainingSeconds -= monthSeconds;
        month++;
    }
    
    daySeconds = remainingSeconds % SECONDS_PER_DAY;
    day = (uint8_t)(remainingSeconds / SECONDS_PER_DAY + 1U);
    
    time->year = year;
    time->month = month;
    time->day = day;
    time->hour = (uint8_t)(daySeconds / SECONDS_PER_HOUR);
    time->minute = (uint8_t)((daySeconds % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE);
    time->second = (uint8_t)(daySeconds % SECONDS_PER_MINUTE);
}


/* =========================== RTC SDK ADAPTATION =========================== */
/*************************************************
  Function:     TimerHalRtcSecCallback
  Description:  rtc second callback function
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
static void TimerHalRtcSecCallback(uint32_t Event)
{
    if (Event & RTC_EVENT_PRESCALER) 
    {
        TimerHal_RtcTimeUpdate();
#if (RTC_DEBUG_ENABLE == 1)
        TimerHal_PrintTimeInfo();
#endif
    }
}

/*************************************************
  Function:     TimerHal_RTCInit
  Description:  Initialize RTC
  Input:        None
  Output:       None
  Return:       ResultStatus_t
  Others:       None
*************************************************/
static uint8_t TimerHal_RTCInit(void)
{
    uint8_t ret = 0;

    Rtc_Hal_Stop();
    Rtc_Hal_DeInit();
    /* Initialize RTC using Low Speed Internal 32KHz Clock */
    Rtc_Hal_Init(RTC_CLOCK_LSI32K);
    Rtc_Hal_InstallCallback(TimerHalRtcSecCallback);
    Rtc_Hal_EnableInterrupt(RTC_INT_PRESCALER_EN);
    TimerHal_RtcIrqEnable();

    uint32_t workFreq = Rtc_Hal_GetWorkFreq();
    Rtc_Hal_Start(workFreq);

    /* Initialize base time references */
    g_currentTime.year = RTC_START_YEAR;
    g_currentTime.month = 0U;
    g_currentTime.day = 0U;
    g_currentTime.hour = 0U;
    g_currentTime.minute = 0U;
    g_currentTime.second = 0U;
    g_baseTime = g_currentTime;
    g_rtcBaseSeconds = 0U;
    
#if (RTC_DEBUG_ENABLE == 1)
    TimeData_t time = {
        .year = 2026U,
        .month = 7U,
        .day = 9U,
        .hour = 19U,
        .minute = 36U,
        .second = 0U,
    };
    TimerHal_RtcRemoteTimeSync(&time);
#endif

    return ret;
}

/*************************************************
  Function:     TimerHal_GetRtcTime
  Description:  Get rtc time
  Input:        time :time point
  Output:       None
  Return:       ResultStatus_t
  Others:       None
*************************************************/
uint8_t TimerHal_GetRtcTime(TimeData_t *time)
{
    if (time == NULL) return 1;
    
    TimerHal_RtcTimeUpdate();
    *time = g_currentTime;
    return 0;
}

/*************************************************
  Function:     TimerHal_SetRtcTime
  Description:  Set system time
  Input:        time :time point
  Output:       None
  Return:       ResultStatus_t
  Others:       None
*************************************************/
static uint8_t TimerHal_SetRtcTime(const TimeData_t *time)
{
    if ((time->month < 1U) || (time->month > 12U) || 
        (time->day < 1U) || (time->day > 31U) ||
        (time->hour > 23U) || (time->minute > 59U) || (time->second > 59U))
    {
        return 1;
    }
    
    uint32_t totalSeconds = TimerHalTimeToSeconds(time);
    
    Rtc_Hal_Pause();
    Rtc_Hal_SetCurrentValue(totalSeconds);
    
    /* Restart RTC main counter */
    uint32_t workFreq = Rtc_Hal_GetWorkFreq();
    Rtc_Hal_Start(workFreq);
    
    g_baseTime = *time;
    g_rtcBaseSeconds = totalSeconds;
    g_currentTime = *time;
    
    return 0;
}

/*************************************************
  Function:     TimerHal_RtcTimeUpdate
  Description:  Update rtc time
  Input:        None
  Output:       None
  Return:       ResultStatus_t
  Others:       None
*************************************************/
static void TimerHal_RtcTimeUpdate(void)
{
    uint32_t currentRtcSeconds;
    uint32_t elapsedSeconds;
    uint32_t totalSeconds;

    /* Get current counter driven by hardware */
    currentRtcSeconds = Rtc_Hal_GetCurrentValue();
    elapsedSeconds = currentRtcSeconds - g_rtcBaseSeconds;
    totalSeconds = TimerHalTimeToSeconds(&g_baseTime) + elapsedSeconds;
    
    TimerHalSecondsToTime(totalSeconds, &g_currentTime);
}

/*************************************************
  Function:     TimerHal_RtcRemoteTimeSync
  Description:  Remote time sync
  Input:        time :time point
  Output:       None
  Return:       ResultStatus_t
  Others:       None
*************************************************/
uint8_t TimerHal_RtcRemoteTimeSync(const TimeData_t *time)
{
    uint8_t ret = 0;
    __disable_irq();
    ret = TimerHal_SetRtcTime(time);
    __enable_irq();
    return ret;
}

/*************************************************
  Function:     TimerHal_RtcIrqEnable
  Description:  Enable rtc second interrupt
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
static void TimerHal_RtcIrqEnable(void)
{
    Core_Hal_ClearPendingIrq(RTC_IRQn);
    Core_Hal_EnableIrq(RTC_IRQn);
}

/*************************************************
  Function:     TimerHal_RtcIrqDisable
  Description:  Disable rtc second interrupt
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
static void TimerHal_RtcIrqDisable(void)
{
    Core_Hal_DisableIrq(RTC_IRQn);
    Core_Hal_ClearPendingIrq(RTC_IRQn);
}

/*************************************************
  Function:     TimerHal_PrintTimeInfo
  Description:  Print rtc time
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
#if (RTC_DEBUG_ENABLE == 1)
static void TimerHal_PrintTimeInfo(void)
{
    TBOX_PRINT("Current time: %04u-%02u-%02u %02u:%02u:%02u\n", 
           g_currentTime.year, g_currentTime.month, g_currentTime.day,
           g_currentTime.hour, g_currentTime.minute, g_currentTime.second);
}
#endif