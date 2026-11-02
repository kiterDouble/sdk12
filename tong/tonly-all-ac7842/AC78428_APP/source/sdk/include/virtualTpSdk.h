/*************************************************
Copyright ? 2024 SiRun (Beijing) . All rights reserved.
  File Name:        virtualTpSdk.h
  Author:          lei.wang
  Created Time:    2024
  Description:     virtual Transmit pipe ,Used for remote diagnosis and diagnosis
   Others:      // 其它说明
*************************************************/
#ifndef _VIRTUAL_TP_SDK_H_
#define _VIRTUAL_TP_SDK_H_

#include "stdint.h"

/*************************************************
  Function:       VirtualTpSdkClientOpen
  Description:    Client端打开一个句柄
  Input:          rxBuffer：接收buffer首地址
                  rxBufLength：接收buffer长度
                  txBuffer：发送buffer首地址
                  txBufLength：发送buffer长度
  Output:         无
  Return:         >=0：返回一个正确的句柄
                  -1：获取句柄失败
  Others:         
*************************************************/
int16_t VirtualTpSdkClientOpen(uint8_t *rxBuffer,uint16_t rxBufLength,uint8_t *txBuffer,uint16_t txBufLength);

/*************************************************
  Function:       VirtualTpSdkClientReceive
  Description:    Client接收函数
  Input:          handle：句柄
                  pBuffer：接收缓存
                  bufLength：缓存长度
                  pLengthOut：接收数据长度
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t VirtualTpSdkClientReceive(int16_t handle,uint8_t *pBuffer,uint16_t bufLength,uint16_t *pLengthOut);

/*************************************************
  Function:       VirtualTpSdkClientTransmit
  Description:    client端发送数据接口
  Input:          handle：句柄
                  pData：发送数据首地址
                  bufLength：发送长度
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t VirtualTpSdkClientTransmit(int16_t handle,const uint8_t *pData,uint16_t bufLength);

/*************************************************
  Function:       VirtualTpSdkServerTransmit
  Description:    server端发送数据接口
  Input:          channelId：通道id，由接收函数（VirtualTpSdkServerReceive）获得
                  pData：发送数据首地址
                  bufLength：发送长度
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t VirtualTpSdkServerTransmit(uint8_t channelId,const uint8_t *pData,uint16_t bufLength);

/*************************************************
  Function:       VirtualTpSdkServerReceive
  Description:    server端接收数据接口
  Input:          pChannelId：通道id，发送时使用
                  pBuffer：接收缓存
                  bufLength：缓存长度
                  pLengthOut：接收数据长度
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t VirtualTpSdkServerReceive(uint8_t *pChannelId,uint8_t *pBuffer,uint16_t bufLength,uint16_t *pLengthOut);



#endif