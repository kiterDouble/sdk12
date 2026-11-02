/*************************************************
Copyright ? 2024 SiRun (Beijing) . All rights reserved.
  File Name:       udsTp.h.h
  Author:          lei.wang
  Created Time:    2024
  Description:     remote diagnostic SDK module 
   Others:      // 其它说明
*************************************************/
#ifndef _USD_TP_H_
#define _USD_TP_H_

#include "canHal.h"
#include "stdint.h"

//CanTp设置
typedef struct
{
    uint8_t blockSize;
    uint8_t STmin;
    uint8_t fillByte;
    uint32_t N_As_enforced;
    uint32_t N_Ar_enforced;
    uint32_t N_Bs_enforced;
    uint32_t N_Br_enforced;
    uint32_t N_Cs_enforced;
    uint32_t N_Cr_enforced;
    uint32_t N_As_obd;
    uint32_t N_Ar_obd;
    uint32_t N_Bs_obd;
    uint32_t N_Br_obd;
    uint32_t N_Cs_obd;
    uint32_t N_Cr_obd;
}UdsTpParameter_t;


/*************************************************
  Function:       UdsTpOpen
  Description:    转发诊断报文获取句柄
  Input:          canChanal:
                  pCanRxbuffer:
                  pBufferNum:
                  pTpParameter:
  Output:         无
  Return:         -1：失败
                  >0:有效句柄
  Others:         
*************************************************/
int16_t UdsTpOpen(uint8_t canChanal,CanHalMsg_t *pCanRxbuffer,uint16_t pBufferNum,UdsTpParameter_t *pTpParameter);

/*************************************************
  Function:       UdsTpSetFunctionId
  Description:    设置功能寻址id
  Input:          handle：udsTp句柄
                  requestId：功能寻址请求ID
  Output:         无
  Return:         函数执行结果 
                  0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t UdsTpSetFunctionId(int16_t handle,uint32_t requestId);

/*************************************************
  Function:       UdsTpTransmit
  Description:    远程诊断报文TP发送接口
  Input:          handle：诊断Tp据句柄
                  typeFlag：诊断报文类型标记 0：物理寻址 1：功能寻址
                  txData：Can数据
                  dataLen：数据长度
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t UdsTpTransmit(int16_t handle,uint8_t typeFlag,uint8_t *txData,uint16_t dataLen);

/*************************************************
  Function:       UdsTpReceive
  Description:    获取mcu设置单个参数到CPU的请求包
  Input:          udsTpHandle：接收usd响应句柄
                  pData：接收响应数据
                  dataLen：接收数据长度
                  waitTime：接收超时时间设置 单位：ms
  Output:         无
  Return:         0：接收数据成功 
                  -1：未接收到数据
  Others:         
*************************************************/
int16_t UdsTpReceive(int16_t udsTpHandle,uint8_t *pData,uint16_t *dataLen,uint32_t waitTime);

/*************************************************
  Function:       UdsTpPhyAddressSetCanId
  Description:    USD TP物理寻址设置canID
  Input:          handle：udsTp句柄
                  reqCanId：诊断请求ID
                  respCanId：对应的响应ID
  Output:         无
  Return:         
  Others:         
*************************************************/
void UdsTpPhyAddressSetCanId(int16_t handle,uint32_t reqCanId,uint32_t respCanId);

/*************************************************
  Function:       UdsTpSetFilter
  Description:    UDS诊断设置filter
  Input:          handle：UdsTp 通信句柄
                  respType：诊断请求类型0：物理寻址 1：功能寻址
  Output:         无
  Return:         
  Others:         
*************************************************/
void UdsTpSetFilter(int16_t handle,uint8_t respType);

/*************************************************
  Function:       UdsTpSetFunctionalResponseId
  Description:    设置功能寻址响应ID范围
  Input:          handle：udsTp句柄
                  responseIdMin：响应ID最小值
                  responseIdMax：响应ID最大值
  Output:         无
  Return:
  Others:
*************************************************/
void UdsTpSetFunctionalResponseId(int16_t handle, uint32_t responseIdMin, uint32_t responseIdMax);

/*************************************************
  Function:       UdsTpClearRecvBuffer
  Description:    UDS诊断请求请求清空接收buf
  Input:          handle：UdsTp 通信句柄
  Output:         无
  Return:         
  Others:         
*************************************************/
void UdsTpClearRecvBuffer(int16_t handle);

//透传函数
int16_t UdsTpTransmitRaw(int16_t udsTpHandle, uint32_t canId, uint8_t *txData, uint16_t dataLen);
int16_t UdsTpReceiveRaw(int16_t udsTpHandle, uint8_t *pData, uint16_t *dataLen);

/*************************************************
  Function:       UdsTpSetPendingCallback
  Description:    设置pending响应回调函数
  Input:          handle：udsTp句柄
                  callback：回调函数指针
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         当收到pending响应时调用此回调
*************************************************/
typedef void (*UdsTpPendingCallback_t)(uint32_t responseId, uint8_t sid);
int16_t UdsTpSetPendingCallback(int16_t handle, UdsTpPendingCallback_t callback);

#endif
