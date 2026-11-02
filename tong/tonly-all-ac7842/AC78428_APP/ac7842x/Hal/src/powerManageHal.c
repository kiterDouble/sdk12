/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: powerManageHal.c
 Author: HuShangrui
 Created Time: 2026年7月10日
 Description: 
 Others: 
*************************************************/
/****************************** include ***************************************/
#include "PeripheralHal.h"
#include "powerManageHal.h"
#include "logHal.h"
#include <string.h>
#include "Gpio_Hal.h"
#include "Wdg_Hal.h"
#include "Spm_Hal.h"
#include "Ckgen_Hal.h"
#include "Core_Hal.h"
#include "clock_cfg.h"

/****************************** Macro Definitions ******************************/
#define CAN_CHANEL_NUMBER_MAX    8

/****************************** Global Variables ******************************/
static uint16_t g_mcuWakeUpSoureFlag = PM_HAL_WAKEUP_SOURCE_NONE;
static uint8_t g_canWakeUpFlag[CAN_CHANEL_NUMBER_MAX];
static uint8_t g_sleepmode = 0;
static uint32_t g_maxvoltage = 0;
static uint32_t g_minvoltage = 0;

/****************************** Private Functions *****************************/
static void PmKernelIntoSleepSet(void)
{
    Wdg_Hal_Feed();
    /* 清除所有中断 */
    for (uint8_t i = 0; i < (uint8_t)MAX_IRQn; i++)
    {
        Core_Hal_ClearPendingIrq((IRQn_Type)i);
    }
    // SPM_DRV_SetStandbyWakeup(STANDBY_WAKEUP_PB0,STANDBY_WAKEUP_FALLING_EDGE); // 深度休眠需要设置唤醒源
    Spm_Hal_SetPowerMode(SPM_MODE_VLPS); // 待确认各个休眠模式 POWER_MODE_VLPS SPM_MODE_STOP1 POWER_MODE_STANDBY
}

static void PmKernelIntoWakeupSet(void)
{
    Wdg_Hal_Feed();
    /* 设置时钟 */
    Ckgen_Hal_InitClk(&g_clockGenCfg);
    Ckgen_Hal_DistributeClk(&g_clkDistributeCfg);
}

static void PowerManageHalWakeupSourceChangeFun(uint8_t source)
{
    if(source < PM_HAL_WAKEUP_SOURCE_MAX)
        g_mcuWakeUpSoureFlag = source;
}

/*************************************************
  Function:       PowerManageHalInit
  Description:    Power management module hardware initialization interface
  Input:          mode : 1 - RTC wakeup enabled
                         0 - RTC wakeup disabled
                  time : RTC wake up time in seconds
                  maxvoltage : Wakeup threshold when voltage exceeds this value
                  minvoltage : Wakeup threshold when voltage drops below this value
  Output:         none
  Return:         none
  Others:         Registers wakeup source callback function
                  Initializes CAN wakeup flags to zero
*************************************************/
void PowerManageHalInit(uint8_t mode, uint32_t time, uint32_t maxvoltage, uint32_t minvoltage)
{
    memset(g_canWakeUpFlag, 0, CAN_CHANEL_NUMBER_MAX);
    PeripheralHalRegisterSetWakeupSourceCallback(PowerManageHalWakeupSourceChangeFun);
    g_sleepmode = (mode == 1) ? 1 : 0;
    g_maxvoltage = maxvoltage;
    g_minvoltage = minvoltage;
}

/*************************************************
  Function:       PowerManageHalInit
  Description:    Set the maximum and minimum voltage of the RTC wake up
  Input:          maxvoltage : If the voltage is greater than this value, it wakes up
                  minvoltage : If the voltage is less than this value, it wakes up
  Output:         none
  Return:         
  Others:         
*************************************************/
void PowerManageHalSetWakeupVoltage(uint32_t maxvoltage, uint32_t minvoltage)
{
    g_maxvoltage = maxvoltage;
    g_minvoltage = minvoltage;
}

/*************************************************
  Function:       PowerManageHalSleep
  Description:    Power management interface sleep entry function
  Input:          none
  Output:         none
  Return:         none
  Others:         Clears wakeup source flag before entering sleep
                  Handles both RTC scheduled wakeup and external wakeup
                  Restores system configuration after wakeup
*************************************************/
void PowerManageHalSleep(void)
{
    g_mcuWakeUpSoureFlag = PM_HAL_WAKEUP_SOURCE_NONE;
    if (g_sleepmode == 1)
    {
        while(1)
        {
            PmKernelIntoSleepSet();
            PmKernelIntoWakeupSet();
            
            if (PowerManageHalGetWakeupSource() == PM_HAL_WAKEUP_SOURCE_MCURTC)
            {
                if (RtcWakeUpProcess(g_maxvoltage, g_minvoltage)) break;
            }
            else break;
        }
    }
    else
    {
        PmKernelIntoSleepSet();
        PmKernelIntoWakeupSet();
    }
}

/*************************************************
  Function:       PowerManageHalGetWakeupSource
  Description:    Get wakeup source after MCU wakes up
  Input:          none
  Output:         none
  Return:         Wakeup source flag
  Others:         Returns the global wakeup source flag value
*************************************************/
int16_t PowerManageHalGetWakeupSource(void)
{
    return g_mcuWakeUpSoureFlag;
}

/*************************************************
  Function:       PowerManageHalEnableCanWake
  Description:    Enable CAN wakeup functionality for specified channel
  Input:          canChannel - CAN channel number to enable wakeup on
  Output:         none
  Return:         0 (PM_HAL_STATUS_OK) - Success
                  -1 (PM_HAL_STATUS_ERR) - Error, invalid channel
  Others:         Sets the corresponding CAN wakeup flag
                  Valid channels range from 0 to CAN_CHANEL_NUMBER_MAX-1
*************************************************/
int16_t PowerManageHalEnableCanWake(uint8_t canChannel)
{
    if(canChannel >= CAN_CHANEL_NUMBER_MAX) return -1;
    g_canWakeUpFlag[canChannel] = 1;
    return 0;
}

/*************************************************
  Function:       PowerManageHalIsrSetWakeUpSource
  Description:    Update wakeup source when waking up from interrupt
  Input:          wakeupSource - Wakeup source identifier
  Output:         none
  Return:         0 (PM_HAL_STATUS_OK) - Success
                  -1 (PM_HAL_STATUS_ERR) - Error, invalid source
  Others:         Valid wakeup sources range from 0 to PM_HAL_WAKEUP_SOURCE_MAX-1
                  Typically called from interrupt service routines
*************************************************/
int16_t PowerManageHalIsrSetWakeUpSource(uint8_t wakeupSource)
{
    if(wakeupSource >= PM_HAL_WAKEUP_SOURCE_MAX) return -1;
    g_mcuWakeUpSoureFlag = wakeupSource;
    return 0;
}

/*************************************************
  Function:       PowerManageHalClearWakeUpSource
  Description:    Clear wakeup source operation
  Input:          none
  Output:         none
  Return:         0 (PM_HAL_STATUS_OK) - Success
  Others:         Resets the global wakeup source flag to PM_HAL_WAKEUP_SOURCE_NONE
                  Always returns success
*************************************************/
int16_t PowerManageHalClearWakeUpSource(void)
{
    g_mcuWakeUpSoureFlag = PM_HAL_WAKEUP_SOURCE_NONE;
    return 0;
}

void PowerManageHalTestMain(void)
{
    PowerManageHalSleep();
}