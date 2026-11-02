/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: peripheralHal.h
 Author: 
 Created Time: 2026年7月15日
 Description: 
 Others: 
*************************************************/
#ifndef _PERIPHERALHAL_H
#define _PERIPHERALHAL_H

/****************************** include ***************************************/
#include <stdint.h>

/****************************** Macro Definitions *****************************/
#define TIMER_KPI_ENABLE              (0U)        // 定时器中断耗时KPI使能

/* ERROR CODE */
#define PERIPHERALHAL_OK                0u
#define PERIPHERALHAL_ERR_INVALID_PARAM 0x3001u
#define PERIPHERALHAL_ERR_NO_RESOURCE   0x3002u

/* AD convert */
#define AD_CHANNEL_KL30                 0
#define AD_CHANNEL_BATTERY              1
#define AD_CHANNEL_NTC                  2

#define AD_CHANNEL_BCALL                3
#define AD_CHANNEL_ECALL                4
#define AD_CHANNEL_BCALL_LIGHT          5
#define AD_CHANNEL_ECALL_LIGHT          6

// #define AD_CHANNEL_MICP                 4
#define AD_CHANNEL_4G                   7
#define AD_CHANNEL_CAN_POWER            8
#define AD_CHANNEL_GPS                  9
#define AD_CHANNEL_GPS1                 14
#define AD_CHANNEL_MIC                  10
#define AD_CHANNEL_MIC_POWER            15

#define AD_CHANNEL_MICP                 11
#define AD_CHANNEL_SYS_5V               12
#define AD_CHANNEL_SOS_KEY              13

#define AD_CHANNEL_HW_VER               16

/* Backward compatibility aliases (Z20K148MS legacy names) */
#define AD_CHANNEL_BACKUP_BATTERY_NTC   AD_CHANNEL_NTC      /* renamed to AD_CHANNEL_NTC */

#define AD_REF_VOLTAGE                  (3300)
#define AD_CHANNEL_KL30_FACTOR          (731 / 51)
// adc0
#define AD_CHANNEL_BCALL_FACTOR         1
#define AD_CHANNEL_ECALL_FACTOR         1
#define AD_CHANNEL_BCALL_LIGHT_FACTOR   1
#define AD_CHANNEL_ECALL_LIGHT_FACTOR   1
#define AD_CHANNEL_MIC_POWER_FACTOR      1
// adc1
#define AD_CHANNEL_KL_30_FACTOR         1
#define AD_CHANNEL_4G_FACTOR            1
#define AD_CHANNEL_CAN_POWER_FACTOR     1
#define AD_CHANNEL_GPS_FACTOR           1
#define AD_CHANNEL_GPS1_FACTOR          1
#define AD_CHANNEL_MIC_FACTOR           1
#define AD_CHANNEL_SOS_FACTOR           1
#define AD_CHANNEL_HW_VER_FACTOR        (1047 / 1000)
#define AD_CHANNEL_SYS_5V_FACTOR        (94)          /* R443=47K(上) R424=47K(下), 系数=94/47≈2*/
#define AD_CHANNEL_SYS_5V_DIVIDER       (47)
/****************************** Type Definitions ******************************/
typedef void(* typeSetWakeupSourceCallbackPtr)(uint8_t source);
typedef void(* typeSetBatteryModeCallbackPtr)(uint8_t mode);
typedef void(* SoftwareTimerCallback_t)(void);

/****************************** Function Declarations *************************/
/*************************************************
  Function:     PeripheralHalInit
  Description:  Peripheral Hal init
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void PeripheralHalInit(void);

/*************************************************
  Function:     PeripheralHalSetMode
  Description:  Set the sleep and wake mode
  Input:        mode :sleep or wake
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void PeripheralHalSetMode(uint8_t mode);

/*************************************************
  Function:     PeripheralHalGetKl15Status
  Description:  Get Kl15 status
  Input:        none
  Output:       KL15 status
  Return:       None
  Others:       None
*************************************************/
int16_t PeripheralHalGetKl15Status (void);
int16_t PeripheralHalGetKl30Status (void);
void PeripheralHalSetSysOn (uint8_t flag);
int16_t PeripheralHalAdGet(uint8_t adChannel, uint32_t *pValue);
void PeripheralHalAdcStart(uint8_t sarNo);
void PeripheralHal1msTimerProcess(void);
void PeripheralHalMcuHardReset(void);
void PeripheralHalFeedWatchDog(void);
void PeripheralHalSetTestMode(uint8_t testMode);
void PeripheralHalRegisterSetWakeupSourceCallback(typeSetWakeupSourceCallbackPtr pCallBackFun);
void PeripheralHalRegisterSetBatteryModeCallback(typeSetBatteryModeCallbackPtr pCallBackFun);
int16_t RtcWakeUpProcess(uint32_t maxvoltage,uint32_t minvoltage);
void PeripheralHalTestMain(void);

/*************************************************
  Function:     SoftwareTimerCreate
  Description:  Create a software timer
  Input:        callback: Timer callback function
                interval: Timer interval in milliseconds
  Output:       None
  Return:       Timer ID (0-4) on success, -1 on failure
  Others:       None
*************************************************/
uint32_t PeripheralHal_TimerCreate(SoftwareTimerCallback_t callback, uint32_t interval);

/*************************************************
  Function:     SoftwareTimerStart
  Description:  Start a software timer
  Input:        timerId: Timer ID
  Output:       None
  Return:       PERIPHERALHAL_OK on success, PERIPHERALHAL_ERR_INVALID_PARAM on failure
  Others:       None
*************************************************/
uint32_t PeripheralHal_TimerStart(uint8_t timerId);

/*************************************************
  Function:     SoftwareTimerStop
  Description:  Stop a software timer
  Input:        timerId: Timer ID
  Output:       None
  Return:       PERIPHERALHAL_OK on success, PERIPHERALHAL_ERR_INVALID_PARAM on failure
  Others:       None
*************************************************/
uint32_t PeripheralHal_TimerStop(uint8_t timerId);

/*************************************************
  Function:       PeripheralHal_SetTimerVal
  Description:    Set the interval of a software timer
  Input:          timerId - Timer ID (0-4)
                  interval - New interval in milliseconds
  Output:         none
  Return:         PERIPHERALHAL_OK on success, PERIPHERALHAL_ERR_INVALID_PARAM on failure
  Others:         none
*************************************************/
uint32_t PeripheralHal_SetTimerVal(uint8_t timerId, uint32_t interval);

#endif    //_PERIPHERALHAL_H
