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
#define DEBUG_UART_REV_ENABLE   1
#define INFO_PRINT(var...)      LogHalRttPrint(var)
#define PURE_PRINT(var...)      LogHalRttPrint(var)

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
void LogHalSetMode(uint8_t mode);
void LogHalUpLoadLog(const char *format, ...);

#if (DEBUG_UART_REV_ENABLE == 1)
/*************************************************
  Function:     LogHal_MainFunc
  Description:  MPU uart or debug uart main function
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void LogHal_MainFunc(void);
#endif


#endif    //LOGHAL_H