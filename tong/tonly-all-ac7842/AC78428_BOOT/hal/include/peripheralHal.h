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

void PeripheralHalSetSysOn (uint8_t flag);
void PeripheralHal1msTimerProcess(void);
void PeripheralHalMcuHardReset(void);
void PeripheralHalFeedWatchDog(void);

#endif    //_PERIPHERALHAL_H
