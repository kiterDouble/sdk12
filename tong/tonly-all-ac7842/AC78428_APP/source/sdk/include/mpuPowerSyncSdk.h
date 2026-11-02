/*************************************************
Copyright ? 2024 SiRun (Beijing) . All rights reserved.
  File Name:       mpuPowerSyncSdk
  Author:          lei.wang
  Created Time:    2024
  Description:     Synchronize with the power status of the CPU
   Others:      // 其它说明
*************************************************/

#ifndef _MPU_POWER_SYNC_SDK_H_
#define _MPU_POWER_SYNC_SDK_H_

#include <stdint.h>
#include "mpuHal.h"

/*************************************************
  Function:       DisableSleepEventIsEcall  DisableSleepEventIsOTA
  Description:    判定是否是某一MPU阻止休眠事件
  Input:          event：MpuPowerSyncSdkGetSleepDisableState接口的返回值，是一个事件集合
  Output:         无
  Return:         1：是
                  0：否
  Others:         
*************************************************/
#define DisableSleepEventIsEcall(event)   ((event&0x01) != 0?1:0)     //bit 0  E_CALL
#define DisableSleepEventIsOTA(event)     ((event&0x02) != 0?1:0)   //bit 1  升级

//
typedef int16_t (* MpuErrorCbFun_t)(void);

/*************************************************
  Function:       MpuPowerSyncSdkInit
  Description:    与MPU同步电源状态模块的初始化接口
  Input:          mpuHandle：mpu通信句柄
                  cycleTime：模块周期调用接口调用周期，>0
  Output:         无
  Return:         0:执行成功
                  -1：执行失败
  Others:         
*************************************************/
int16_t MpuPowerSyncSdkInit(int16_t mpuHandle,uint16_t cycleTime);

/*************************************************
  Function:       MpuPowerSyncSdkCycleProcess
  Description:    mpu同步电源状态模块周期调用接口
  Input:          msgData：mpu的消息，没有接收到mpu消息可传NULL
  Output:         无
  Return:         无
  Others:         
*************************************************/
void MpuPowerSyncSdkCycleProcess(MpuHalDataPack_t *msgData);

/*************************************************
  Function:       MpuPowerSyncSdkSetSleep
  Description:    设置mpu进入休眠状态 
  Input:          deepSleepFlag：休眠状态标记 0:正常休眠 1:深度休眠
  Output:         无
  Return:         0:成功
                  -1：失败
  Others:         
*************************************************/
int16_t MpuPowerSyncSdkSetSleep(uint8_t deepSleepFlag);

/*************************************************
  Function:       MpuPowerSyncSdkGetSleepStatus
  Description:    获取MPU休眠完成状态
  Input:          无
  Output:         无
  Return:         休眠状态：0：已休眠 1：未休眠
  Others:         
*************************************************/
int16_t MpuPowerSyncSdkGetSleepStatus(void);

/*************************************************
  Function:       MpuPowerSyncSdkSetWake
  Description:    设置MPU唤醒状态
  Input:          wakeupSource:唤醒源
  Output:         无
  Return:         无
  Others:         
*************************************************/
int16_t MpuPowerSyncSdkSetWake(uint8_t wakeupSource);

/*************************************************
  Function:       MpuPowerSyncSdkSetDeepSleepFlag
  Description:    设置电源状态为深度休眠
  Input:          deepSleepFlag：0:正常休眠 1:深度休眠
  Output:         无
  Return:         无
  Others:         
*************************************************/
void MpuPowerSyncSdkSetDeepSleepFlag(uint8_t deepSleepFlag);

/*************************************************
  Function:       MpuPowerSyncSdkGetWakeStatus
  Description:    获取唤醒状态
  Input:          无
  Output:         pWakeSource：0：mpu唤醒完成  1：MPU唤醒未完成
  Return:         0:成功
                  -1：失败
  Others:         
*************************************************/
int16_t MpuPowerSyncSdkGetWakeStatus(uint8_t *pWakeSource);

/*************************************************
  Function:       MpuPowerSyncSdkGetSleepDisableState
  Description:    获取MPU禁止休眠状态
  Input:          无
  Output:         无
  Return:         0：无阻值休眠时间
                  非零值：有阻止休眠事件
  Others:         
*************************************************/
int16_t MpuPowerSyncSdkGetSleepDisableState(void);

/*************************************************
  Function:       MpuPowerSyncSdkSetRkMode
  Description:    设置上报给MPU的rkmode
  Input:          mode ：0:INIT  1：正常模式（全功能）2：运输模式 3：生产模式
  Output:         无
  Return:         0：设置成功
                  -1：设置失败
  Others:         
*************************************************/
int16_t MpuPowerSyncSdkSetRkMode(uint8_t mode);

/*************************************************
  Function:       MpuPowerSyncSdkRegisteCb
  Description:    注册MPU通信异常回调接口
  Input:          mode ：0:INIT  1：正常模式（全功能）2：运输模式 3：生产模式
  Output:         无
  Return:         0：设置成功
                  -1：设置失败
  Others:         
*************************************************/
int16_t MpuPowerSyncSdkRegisteMpuErrorCb(MpuErrorCbFun_t mpuErrorFunc);

/*************************************************
  Function:       MpuPowerSyncSdkGetNadModuleStatus
  Description:    获取nad 模块的运行状态
  Input:          无
  Output:         无
  Return:         0：正常
                  1：异常
  Others:         
*************************************************/
uint8_t MpuPowerSyncSdkGetNadModuleStatus(void);

#endif