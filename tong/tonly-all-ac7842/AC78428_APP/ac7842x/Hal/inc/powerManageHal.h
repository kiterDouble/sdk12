/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name:       powerManageHal.h
 Author:          lei.wang
 Created Time:    2024
 Description:     电源管理休眠唤醒操作及唤醒源管理
 Others:
*************************************************/
#ifndef _POWER_MANAGE_HAL_H_
#define _POWER_MANAGE_HAL_H_

/****************************** include ***************************************/
#include "stdint.h"

/****************************** Macro Definitions ******************************/
#define PM_HAL_STATUS_OK    0           //成功
#define PM_HAL_STATUS_ERR   -1          //失败

#define PM_HAL_WAKEUP_SOURCE_NONE           0     // 无有效唤醒源    
#define PM_HAL_WAKEUP_SOURCE_CAN1           1     // can 1 唤醒
#define PM_HAL_WAKEUP_SOURCE_CAN2           2     // can 2 唤醒
#define PM_HAL_WAKEUP_SOURCE_CAN3           3     // can 3 唤醒
#define PM_HAL_WAKEUP_SOURCE_CAN4           4     // can 4 唤醒
#define PM_HAL_WAKEUP_SOURCE_CAN5           5     // can 5 唤醒
#define PM_HAL_WAKEUP_SOURCE_CAN6           6     // can 6 唤醒
#define PM_HAL_WAKEUP_SOURCE_CAN7           7     // can 7 唤醒
#define PM_HAL_WAKEUP_SOURCE_CAN8           8     // can 8 唤醒
#define PM_HAL_WAKEUP_SOURCE_MPU            9     // MPU 唤醒
#define PM_HAL_WAKEUP_SOURCE_KL15           10    // KL15 上电唤醒
#define PM_HAL_WAKEUP_SOURCE_KL30           11    // KL30 上电唤醒
#define PM_HAL_WAKEUP_SOURCE_BLE            12    // 蓝牙唤醒
#define PM_HAL_WAKEUP_SOURCE_RTC            13    // CPU定时唤醒
#define PM_HAL_WAKEUP_SOURCE_MCURTC         14    // MCU timed wake-up
#define PM_HAL_WAKEUP_SOURCE_GSENSOR        15    // MCU timed wake-up
#define PM_HAL_WAKEUP_SOURCE_CHARGE_FAST    16    // 快充唤醒
#define PM_HAL_WAKEUP_SOURCE_CHARGE_SLOW    17    // 慢充唤醒
#define PM_HAL_WAKEUP_SOURCE_SPI            20    // SPI 唤醒
#define PM_HAL_WAKEUP_SOURCE_MAX            21    // 唤醒源最大值

/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/
/****************************** Function Declarations *************************/
/****************************** Public Function Implementations ***************/
/*************************************************
  Function:       PowerManageHalInit
  Description:    电源管理模块硬件初始化接口
  Input:          无
  Output:         无
  Return:         
  Others:         
*************************************************/
void PowerManageHalInit(uint8_t mode,uint32_t time,uint32_t maxvoltage,uint32_t minvoltage);

/*************************************************
  Function:       PowerManageHalInit
  Description:    Set the maximum and minimum voltage of the RTC wake up
  Input:          maxvoltage : If the voltage is greater than this value, it wakes up
                  minvoltage : If the voltage is less than this value, it wakes up
  Output:         无
  Return:         
  Others:         
*************************************************/
void PowerManageHalSetWakeupVoltage(uint32_t maxvoltage,uint32_t minvoltage);

/*************************************************
  Function:       PowerManageHalSleep
  Description:    电源管理接口进入休眠函数
  Input:          无
  Output:         无
  Return:         
  Others:         
*************************************************/
void PowerManageHalSleep(void);

/*************************************************
  Function:       PowerManageHalGetWakeupSource
  Description:    MCU唤醒后获取唤醒源
  Input:          无
  Output:         执行结果
  Return:         成功：PM_HAL_STATUS_OK
                  失败：PM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t PowerManageHalGetWakeupSource(void);

/*************************************************
  Function:       PowerManageHalEnableCanWake
  Description:    调用此接口后允许CAN唤醒
  Input:          无
  Output:         执行结果
  Return:         成功：PM_HAL_STATUS_OK
                  失败：PM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t PowerManageHalEnableCanWake(uint8_t canChannel);

/*************************************************
  Function:       PowerManageHalIsrSetWakeUpSource
  Description:    中断唤醒时调用此函数更新唤醒源
  Input:          无
  Output:         执行结果
  Return:         成功：PM_HAL_STATUS_OK
                  失败：PM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t PowerManageHalIsrSetWakeUpSource(uint8_t wakeupSource);

/*************************************************
  Function:       PowerManageHalClearWakeUpSource
  Description:    清空唤醒源操作
  Input:          无
  Output:         执行结果
  Return:         成功：PM_HAL_STATUS_OK
                  失败：PM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t PowerManageHalClearWakeUpSource(void);

void PowerManageHalTestMain(void);

#endif
