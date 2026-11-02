/*************************************************
 Copyright © 2025 SiRun (AnHui) . All rights reserved.
 File Name: canPeriodTask.h
 Author: 
 Created Time: 
 Description: 
 Others: 
*************************************************/
#ifndef _CAN_PERIODTASH_TASK_H_
#define _CAN_PERIODTASH_TASK_H_
/****************************** include ***************************************/
#include "stdint.h"
/****************************** Macro Definitions ******************************/


/****************************** Type Definitions ******************************/
typedef struct
{
      uint32_t year;
      uint8_t  month;
      uint8_t  day;
      uint8_t  hour;
      uint8_t  minute;
      uint8_t  second;

}TimeSevice_t;
/****************************** Function Declarations *************************/
/*************************************************
  Function:       CanPeriodCycleInit
  Description:    can报文周期发送初始化接口
  Input:          无
  Output:         无
  Return:         成功：0
                  失败：-1
  Others:         
*************************************************/
//int16_t CanPeriodCycleInit(uint32_t cycleTime);

/*************************************************
  Function:       CanPeriodCycleStart
  Description:    can报文周期发送开始接口
  Input:          无
  Output:         无
  Return:         成功：0
                  失败：-1
  Others:         
*************************************************/
int16_t CanPeriodCycleStart(void);

/*************************************************
  Function:       CanPeriodSendEnable
  Description:    周期发送使能接口
  Input:          canChannel：can通道
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t CanPeriodSendEnable(uint8_t canChannel);

/*************************************************
  Function:       CanPeriodSendDisable
  Description:    周期发送禁止接口
  Input:          canChannel：can通道
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t CanPeriodSendDisable(uint8_t canChannel);

/*************************************************
  Function:       CanPeriodSendEnableAll
  Description:    周期发送使能所有can接口
  Input:          void
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t CanPeriodSendEnableAll(void);

/*************************************************
  Function:       CanPeriodSendDisableAll
  Description:    周期发送禁止所有can接口
  Input:          void
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t CanPeriodSendDisableAll(void);

/*************************************************
  Function:       CanPeriodSendReset
  Description:    周期发送复位接口
  Input:          pcanChannel：can通道
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t CanPeriodSendReset(uint8_t canChannel);

/*************************************************
  Function:       CanPeriodCycleProcess
  Description:    Aes ecb 计算初始化
  Input:          pAesData ：Aes计算对象
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t CanPeriodCycleProcess(void);

/*************************************************
  Function:       canPeriodTaskMain
  Description:    can自发报文周期发送主任务接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
void canPeriodTaskMain(void *pvParameters);

uint8_t CanGetAppMsgEnable(uint8_t canChannel);
#endif


