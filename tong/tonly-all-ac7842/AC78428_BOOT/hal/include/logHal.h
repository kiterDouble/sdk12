/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: logHal.h
 Author: HuShangrui
 Created Time: 2026年7月10日
 Description:
 Others:
*************************************************/
#ifndef LOGHAL_H
#define LOGHAL_H
/****************************** include ***************************************/
#include "stdint.h"

/****************************** Macro Definitions ******************************/
#define TBOX_PRINT(var...)	    LogHalPrint(var)
#define PRINT_MAX_LEN           64
#define TBOX_PRINT_RTT_ENABLE   0

/****************************** Type Definitions ******************************/
/****************************** Function Declarations *************************/
/*************************************************
  Function:     LogHalInit
  Description:  MPU uart or debug uart init
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void LogHalInit(uint8_t mode);
void LogHalPrint(const char *format, ...);
void LogHalRttPrint(const char *format, ...);

/*************************************************
  Function:     LogHalDeInit
  Description:  串口去初始化
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void LogHalDeInit(void);


/*****************************************************************************
 * Function Name: Delay_ms
 * Description  : Provides a blocking millisecond-level delay.
 * Parameters   : uint32_t ms - The number of milliseconds to delay.
 * Return Value : None
 * Note         : This is a blocking function. CPU will wait in a loop until 
                  the specified time has elapsed. Accuracy depends on the 
                  underlying implementation (SysTick timer or software loop) 
                  and system interrupt frequency.
****************************************************************************/
void Delay_ms(uint32_t ms);


#endif    //LOGHAL_H