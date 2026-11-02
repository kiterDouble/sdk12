/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: clockCheck.c
 Author: 
 Created Time: 
 Description: 系统时钟频率检查工具
 Others: 
 *************************************************/

/****************************** include ***************************************/
#include "Ckgen_Hal.h"
#include "logHal.h"

/****************************** Macro Definitions ******************************/
/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/
/****************************** Function Declarations *************************/
/****************************** Public Function Implementations ***************/

/*************************************************
   Function:        ClockCheck_PrintAllClocks
   Description:     打印所有关键时钟频率
   Input:           None
   Output:          None
   Return:          None
   Others:          用于确认系统运行主频
 *************************************************/
void ClockCheck_PrintAllClocks(void)
{
    uint32_t coreFreq, busFreq, slowFreq;
    uint32_t timerFreq;

    TBOX_PRINT("========== System Clock Information ==========\n");

    // 获取核心时钟频率
    if (Ckgen_Hal_GetFreq(CKGEN_CORE_CLK, &coreFreq) == STATUS_SUCCESS)
    {
        TBOX_PRINT("CORE Clock: %d Hz (%.2f MHz)\n", coreFreq, coreFreq / 1000000.0f);
    }

    // 获取总线时钟频率
    if (Ckgen_Hal_GetFreq(CKGEN_BUS_CLK, &busFreq) == STATUS_SUCCESS)
    {
        TBOX_PRINT("BUS Clock:  %d Hz (%.2f MHz)\n", busFreq, busFreq / 1000000.0f);
    }

    // 获取慢速时钟频率
    if (Ckgen_Hal_GetFreq(CKGEN_LSI_CLK, &slowFreq) == STATUS_SUCCESS)
    {
        TBOX_PRINT("SLOW Clock: %d Hz (%.2f MHz)\n", slowFreq, slowFreq / 1000000.0f);
    }

    // 获取定时器时钟频率
    if (Ckgen_Hal_GetFreq(CKGEN_TIMER_CLK, &timerFreq) == STATUS_SUCCESS)
    {
        TBOX_PRINT("TIMER Clock: %d Hz (%.2f MHz)\n", timerFreq, timerFreq / 1000000.0f);
    }

    TBOX_PRINT("==============================================\n");
}

/*************************************************
   Function:        ClockCheck_GetCoreFreq
   Description:     获取核心时钟频率
   Input:           None
   Output:          None
   Return:          核心时钟频率 (Hz)
   Others:          返回0表示错误
 *************************************************/
uint32_t ClockCheck_GetCoreFreq(void)
{
    uint32_t freq = 0;
    Ckgen_Hal_GetFreq(CKGEN_CORE_CLK, &freq);
    return freq;
}

/*************************************************
   Function:        ClockCheck_GetBusFreq
   Description:     获取总线时钟频率
   Input:           None
   Output:          None
   Return:          总线时钟频率 (Hz)
   Others:          返回0表示错误
 *************************************************/
uint32_t ClockCheck_GetBusFreq(void)
{
    uint32_t freq = 0;
    Ckgen_Hal_GetFreq(CKGEN_BUS_CLK, &freq);
    return freq;
}

/*************************************************
   Function:        ClockCheck_GetTimFreq
   Description:     获取定时器时钟频率
   Input:           timId - 定时器ID (未使用, AC7842x共用一个定时器时钟源)
   Output:          None
   Return:          定时器时钟频率 (Hz)
   Others:          返回0表示错误
 *************************************************/
uint32_t ClockCheck_GetTimFreq(uint8_t timId)
{
    uint32_t freq = 0;

    (void)timId;  /* AC7842x 只有一个定时器时钟源 CKGEN_TIMER_CLK */
    Ckgen_Hal_GetFreq(CKGEN_TIMER_CLK, &freq);
    return freq;
}
