/*************************************************
Copyright ? 2024 SiRun (Beijing) . All rights reserved.
  File Name:       timeSyncSdk
  Author:          lei.wang
  Created Time:    2024
  Description:     time sync with MPU
   Others:      // 其它说明
*************************************************/

#ifndef _TIME_SYNC_SDK_H_
#define _TIME_SYNC_SDK_H_

#include <stdint.h>
#include "mpuHal.h"
#include "canPeriodTask.h"
/*************************************************
  Function:       TimeSyncSdkInit
  Description:    sdk层时间同步模块初始化接口
  Input:          mpuHandle:mpu的通信句柄
                  cycleTime：周期接口调用周期设置
  Output:         无
  Return:         无
  Others:         
*************************************************/
void TimeSyncSdkInit(int16_t mpuHandle,uint16_t cycleTime);

/*************************************************
  Function:       TimeSyncSdkCycleProcess
  Description:    sdk层时间同步模块初始化接口
  Input:          pRxData：接收到的MPU的时间同步数据
                  rxLength：接收到的MPU时间同步数据长度
  Output:         无
  Return:         无
  Others:         
*************************************************/
void TimeSyncSdkCycleProcess(MpuHalDataPack_t *msgData);

/*************************************************
  Function:       TimeSyncSdkGetRealTime
  Description:    获取有格式的时间
  Input:          无
  Output:         timeZone：时间差
                  pYear：年
                  pMonth：月
                  pDay：日
                  pHour：时
                  pMin：分
                  pSecond：秒
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t TimeSyncSdkGetRealTime(uint8_t timeZone, TimeSevice_t *pTimeOut);

/*************************************************
  Function:       TimeSyncSdkGetUtcTime
  Description:    获取有格式的时间
  Input:          无
  Output:         pUtc:UTC时间戳
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t TimeSyncSdkGetUtcTime(uint32_t *pUtc);



#endif


