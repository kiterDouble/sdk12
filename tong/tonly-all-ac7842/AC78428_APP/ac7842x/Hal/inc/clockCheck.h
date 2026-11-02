/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: clockCheck.h
 Author: 
 Created Time: 
 Description: 系统时钟频率检查工具头文件
 Others: 
 *************************************************/

#ifndef _CLOCKCHECK_H
#define _CLOCKCHECK_H

/****************************** include ***************************************/
#include <stdint.h>

/****************************** Macro Definitions ******************************/
/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/
/****************************** Function Declarations *************************/

/*************************************************
   Function:     ClockCheck_PrintAllClocks
   Description:  打印所有关键时钟频率
   Input:        None
   Output:       None
   Return:       None
   Others:       用于确认系统运行主频
 *************************************************/
void ClockCheck_PrintAllClocks(void);

/*************************************************
   Function:     ClockCheck_GetCoreFreq
   Description:  获取核心时钟频率
   Input:        None
   Output:       None
   Return:       核心时钟频率 (Hz)
   Others:       返回0表示错误
 *************************************************/
uint32_t ClockCheck_GetCoreFreq(void);

/*************************************************
   Function:     ClockCheck_GetBusFreq
   Description:  获取总线时钟频率
   Input:        None
   Output:       None
   Return:       总线时钟频率 (Hz)
   Others:       返回0表示错误
 *************************************************/
uint32_t ClockCheck_GetBusFreq(void);

/*************************************************
   Function:     ClockCheck_GetTimFreq
   Description:  获取指定定时器时钟频率
   Input:        timId - 定时器ID (0-3)
   Output:       None
   Return:       定时器时钟频率 (Hz)
   Others:       返回0表示错误
 *************************************************/
uint32_t ClockCheck_GetTimFreq(uint8_t timId);

#endif // _CLOCKCHECK_H
