/*************************************************
Copyright ? 2024 SiRun (Beijing) . All rights reserved.
  File Name:       parameterSyncSdk
  Author:          lei.wang
  Created Time:    2024
  Description:     Parameter synchronization module
   Others:      // 其它说明
*************************************************/

#ifndef _PARAMETER_SYNC_SDK_H_
#define _PARAMETER_SYNC_SDK_H_

#include <stdint.h>
#include "mpuHal.h"

/*************************************************
  Function:       pMcuParametFun_t
  Description:    参数读写回调函数定义
  Input:          parameterItem：参数ID,用来调用接口的时候识别对应的参数
                  parameterData：参数数据
                  pDataLength：参数数据长度
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
typedef  int16_t (*pMcuParametReadFun_t)(uint8_t parameterItem,uint8_t *parameterData,uint16_t *pDataLength);

/*************************************************
  Function:       pMcuParametFun_t
  Description:    参数读写回调函数定义
  Input:          parameterItem：参数ID,用来调用接口的时候识别对应的参数
                  parameterData：参数数据
                  pDataLength：参数数据长度
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
typedef  int16_t (*pMcuParametWriteFun_t)(uint8_t parameterItem,uint8_t *parameterData,uint16_t pDataLength);


/*************************************************
  Function:       ParameterSyncSdkInit
  Description:    参数同步模块初始化接口
  Input:          mpuHandle ：mpu通信句柄
                  mcuParamReadCb：读取参数接口回调函数
                  mcuParamWriteCb：写入参数接口回调函数
                  cycleTime：周期调用接口调用周期，单位ms，>0
                  syncParamList:参数ID列表
                  syncParamSize：参数列表大小
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t ParameterSyncSdkInit(int16_t mpuHandle,pMcuParametReadFun_t mcuParamReadCb,pMcuParametWriteFun_t mcuParamWriteCb,uint16_t cycleTime,uint8_t *syncParamList,uint8_t syncParamSize);

/*************************************************
  Function:       ParameterSyncSdkCycleProcess
  Description:    参数同步模块周期调用接口
  Input:          recvDataPack：接收到的mpu参数同步请求响应数据包，未收到传入NULL
  Output:         无
  Return:         无
  Others:         
*************************************************/
void ParameterSyncSdkCycleProcess(MpuHalDataPack_t  *recvDataPack);

/*************************************************
  Function:       ParameterSyncSdkSetToCpu
  Description:    同步参数至MPU
  Input:          parameterId：需要同步至MPU的参数ID
                  pData:参数数据
                  length：参数数据长度
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t ParameterSyncSdkSetToCpu(uint8_t parameterId,uint8_t *pData,uint16_t length);

/*************************************************
  Function:       SyncParameterSdkGetFromCpuIsFinished
  Description:    获取参数同步的结果
  Input:          无
  Output:         无
  Return:         0：同步成功
                  -1：同步未成功
  Others:         
*************************************************/
int16_t ParameterSyncSdkGetFromCpuIsFinished(void);







#endif
