/*************************************************
Copyright ? 2024 SiRun (Beijing) . All rights reserved.
  File Name:       tfirmwareUpdateSdk
  Author:          lei.wang
  Created Time:    2024
  Description:     upgradation module
   Others:      // 其它说明
*************************************************/

#ifndef _FIRMWARE_UPDATE_SDK_H_
#define _FIRMWARE_UPDATE_SDK_H_

#include "stdint.h"

/*************************************************
  Function:       FirmwareUpdateSdkInit
  Description:    升级模块初始化接口
  Input:          cycleTime:周期调用接口调用周期
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t FirmwareUpdateSdkInit(uint16_t cycleTime);

/*************************************************
  Function:       FirmwareUpdateSdkCycleProcess
  Description:    升级模块周期调用接口
  Input:          无
  Output:         无
  Return:         无
  Others:         
*************************************************/
void FirmwareUpdateSdkCycleProcess(void);


#endif
