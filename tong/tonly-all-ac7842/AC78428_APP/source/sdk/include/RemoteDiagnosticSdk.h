/*************************************************
Copyright ? 2024 SiRun (Beijing) . All rights reserved.
  File Name:       remoteDiagnosticSdk.h
  Author:          lei.wang
  Created Time:    2024
  Description:     remote diagnostic SDK module
   Others:      // 其它说明
*************************************************/
#ifndef _REMOTE_DIAGNOSTIC_SDK_H_
#define _REMOTE_DIAGNOSTIC_SDK_H_

#include "canHal.h"
#include "udsTp.h"
#include "stdint.h"


typedef int16_t (* CheckPreconditionFun_t)(void);
typedef int16_t (* SetTestPresentFun_t)(uint8_t stopFlag,uint8_t *pTxData,uint8_t txLength,uint32_t cycleTime);


//
typedef struct 
{
    uint8_t *canChanelList;   //Can通道列表
    uint16_t canChanelListSize;//Can通道列表大小
}CanChanel_t;

//诊断报文对应表
typedef struct
{
    uint32_t requestId;      //诊断请求报文ID
    uint32_t functionalId;   //功能寻址ID
    uint32_t responseId;     //诊断响应报文ID
    uint8_t channel;         //诊断报文对应can
}EcuId_t;

//诊断信息设置结构体
typedef struct 
{
    uint32_t functionalId;                        //功能寻址请求ID
    uint32_t selfDiagnosticReqId;                 //自诊断请求ID
    uint32_t selfDiagnosticRespId;                //自诊断响应ID
    const uint16_t ecuListSize;                  //诊断报文数量
    const EcuId_t *pEcuList;                     //诊断报文列表
    CheckPreconditionFun_t pCheckPreconditionFun; //
    SetTestPresentFun_t pSetTestPresentFun;       //
}CanIdConfig_t;

//
typedef struct 
{
    uint8_t mpuRecvBuffer[2200];    //MPU通信接收buffer设置
    uint16_t mpuRecvBufferSize;     //接收buf的大小
    uint8_t txData[2000];           //MPU通信发送缓存
    uint8_t rxData[3000];           //MPU通信接收缓存
    CanHalMsg_t canRxBuffer[10];    //Can数据接收buffer
    uint16_t canRxBufferSize;       //Can数据接收buffer数量 10z

    uint8_t virtualTpRxBuffer[100]; 
    uint8_t virtualTpTxBuffer[100]; 
    uint8_t canTxBuffer[120];
}MpuBuffer_t;

typedef struct 
{
    uint8_t txData[10];
    uint8_t txLength;
    uint8_t startFlag;
    uint32_t cycleTime;
    uint32_t timeCount;
}TestPreset_t;


/*************************************************
  Function:       RemoteDiagnosticSdkProcess
  Description:    远程诊断接口
  Input:          pEcuConfigure：
                  pMpuBuffer：
                  pTpParameter：
                  canChannelList：
  Output:         无
  Return:         无
  Others:         
*************************************************/
void RemoteDiagnosticSdkProcess(CanIdConfig_t *pEcuConfigure,MpuBuffer_t *pMpuBuffer,UdsTpParameter_t *pTpParameter,CanChanel_t *canChannelList);

/*************************************************
  Function:       RemoteDiagnosticSdkShortDisable
  Description:    打断远程诊断功能接口，持续10s,10s之后自动恢复
  Input:          无
  Output:         无
  Return:         无
  Others:         
*************************************************/
void RemoteDiagnosticSdkShortDisable(void);

/*************************************************
  Function:       RemoteDiagnosticSdkDisable
  Description:    打断远程诊断功能接口，必须调用RemoteDiagnosticSdkRecover接口进行恢复
  Input:          无
  Output:         无
  Return:         无
  Others:         
*************************************************/
void RemoteDiagnosticSdkDisable(void);

/*************************************************
  Function:       RemoteDiagnosticSdkRecover
  Description:    恢复远程诊断功能接口,立即恢复RemoteDiagnosticSdkShortDisable和RemoteDiagnosticSdkDisable接口的打断
  Input:          无
  Output:         无
  Return:         无
  Others:         
*************************************************/
void RemoteDiagnosticSdkRecover(void);

#endif
