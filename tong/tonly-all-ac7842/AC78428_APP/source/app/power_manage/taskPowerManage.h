#ifndef _TASK_POWER_MANAGE_H
#define _TASK_POWER_MANAGE_H

#include <stdint.h>

void TaskPowerManage(void *pvParameters);

void Kl30DtcCheckReset(void);
uint8_t Kl30VoltageState(void);

/*************************************************
  Function:       PowerManageGetLastWakeupSource
  Description:    获取系统最近一次的唤醒源
  Input:          无
  Output:         无
  Return:         唤醒源ID (定义于 powerManageHal.h)
  Others:
*************************************************/
uint8_t PowerManageGetLastWakeupSource(void); // 0xB245_cxl

#endif //_TASK_POWER_MANAGE_H