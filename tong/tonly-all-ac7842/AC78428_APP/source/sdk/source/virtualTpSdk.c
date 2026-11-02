
#include "FreeRTOS.h"
#include "semphr.h"
#include "string.h"
#include "logHal.h"
#include "virtualTpSdk.h"

#define VIRTUAL_TP_HANDLE_MAX  10

static uint8_t g_handleList[VIRTUAL_TP_HANDLE_MAX]  = {0};

//static QueueHandle_t g_queueHandleList[VIRTUAL_TP_HANDLE_MAX];
typedef struct
{
    SemaphoreHandle_t mutexHandle;
    uint8_t *rxBuffer;
    uint16_t rxBufferSize;
    uint16_t rxBufferLenth;
    uint8_t *txBuffer;
    uint16_t txBufferSize;
    uint16_t txBufferLenth;
}VirtualTpBuffer_t;

static VirtualTpBuffer_t g_bufferList[VIRTUAL_TP_HANDLE_MAX];

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
int16_t VirtualTpSdkClientOpen(uint8_t *rxBuffer,uint16_t rxBufLength,uint8_t *txBuffer,uint16_t txBufLength)
{
    int16_t i = 0;
    
    if(rxBuffer == NULL || txBuffer == NULL || rxBufLength == 0 || txBufLength == 0)
    {
        return -1;
    }
    
    for(i = 0;i < VIRTUAL_TP_HANDLE_MAX;i++)
    {
        if(g_handleList[i] != 0x5A)
        {
            g_handleList[i] = 0x5A;
            break;
        } 
    }
    if(i == VIRTUAL_TP_HANDLE_MAX)
    {
        return -1;
    }
    //g_queueHandleList = xQueueCreate(5,64);
    g_bufferList[i].mutexHandle = xSemaphoreCreateMutex();
    g_bufferList[i].rxBuffer = rxBuffer;
    g_bufferList[i].rxBufferSize = rxBufLength;
    g_bufferList[i].rxBufferLenth = 0;
    g_bufferList[i].txBuffer = txBuffer;
    g_bufferList[i].txBufferSize = txBufLength;
    g_bufferList[i].txBufferLenth = 0;
    return i;
}

/*************************************************
  Function:       VirtualTpSdkClientReceive
  Description:    Client接收函数
  Input:          handle：句柄
                  pBuffer：接收缓存
                  bufLength：缓存长度
                  pLengthOut：接收数据长度
  Output:         无
  Return:         结果
                  0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t VirtualTpSdkClientReceive(int16_t handle,uint8_t *pBuffer,uint16_t bufLength,uint16_t *pLengthOut)
{
    if(handle >= VIRTUAL_TP_HANDLE_MAX)
    {
        return -1;
    }
    if(g_handleList[handle] != 0x5A)
    {
        return -1;
    }
    
    xSemaphoreTake(g_bufferList[handle].mutexHandle, portMAX_DELAY );
    
    if((g_bufferList[handle].rxBufferLenth > 0) && (g_bufferList[handle].rxBufferLenth <= bufLength))
    {
        memcpy(pBuffer,g_bufferList[handle].rxBuffer,g_bufferList[handle].rxBufferLenth);
        *pLengthOut = g_bufferList[handle].rxBufferLenth;
        g_bufferList[handle].rxBufferLenth = 0;
    }
    else
    {
        *pLengthOut = 0;
    }
    
    xSemaphoreGive(g_bufferList[handle].mutexHandle);
    
    if(*pLengthOut > 0)
    {
        TBOX_PRINT("client recv :");
        for(uint8_t i = 0;i<*pLengthOut;i++)
        {
            TBOX_PRINT("%02x ",pBuffer[i]);
        }
        TBOX_PRINT("\r\n");
    }
    return 0;
}

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
int16_t VirtualTpSdkClientTransmit(int16_t handle,const uint8_t *pData,uint16_t bufLength)
{
    if(handle >= VIRTUAL_TP_HANDLE_MAX)
    {
        return -1;
    }
    if(g_handleList[handle] != 0x5A)
    {
        return -1;
    }
    if((pData == NULL) || (bufLength == 0))
    {
        return -1;
    }
    TBOX_PRINT("client transmit :");
    for(uint8_t i = 0;i<bufLength;i++)
    {
        TBOX_PRINT("%02x ",pData[i]);
    }
    TBOX_PRINT("\r\n");
    xSemaphoreTake(g_bufferList[handle].mutexHandle, portMAX_DELAY );
    
    if(g_bufferList[handle].txBufferSize >= bufLength)
    {
        memcpy(g_bufferList[handle].txBuffer,pData,bufLength);
        g_bufferList[handle].txBufferLenth = bufLength;
    }
    
    xSemaphoreGive(g_bufferList[handle].mutexHandle);
    return 0;
}

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
int16_t VirtualTpSdkServerTransmit(uint8_t channelId,const uint8_t *pData,uint16_t bufLength)
{
    if(channelId >= VIRTUAL_TP_HANDLE_MAX)
    {
        return -1;
    }
    if(g_handleList[channelId] != 0x5A)
    {
        return -1;
    }
    if((pData == NULL) || (bufLength == 0))
    {
        return -1;
    }
    
    TBOX_PRINT("server transmit :");
    for(uint8_t i = 0;i<bufLength;i++)
    {
        TBOX_PRINT("%02x ",pData[i]);
    }
    TBOX_PRINT("\r\n");
    
    xSemaphoreTake(g_bufferList[channelId].mutexHandle, portMAX_DELAY );
    
    if(g_bufferList[channelId].rxBufferSize >= bufLength)
    {
        memcpy(g_bufferList[channelId].rxBuffer,pData,bufLength);
        g_bufferList[channelId].rxBufferLenth = bufLength;
    }
    
    xSemaphoreGive(g_bufferList[channelId].mutexHandle);
    return 0;
}

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
int16_t VirtualTpSdkServerReceive(uint8_t *pChannelId,uint8_t *pBuffer,uint16_t bufLength,uint16_t *pLengthOut)
{
    uint8_t i = 0;
    if((pBuffer == NULL) || (bufLength == 0))
    {
        return -1;
    }
    for(i = 0;i < VIRTUAL_TP_HANDLE_MAX;i++)
    {
        if(g_handleList[i] == 0x5A)
        {
            xSemaphoreTake(g_bufferList[i].mutexHandle, portMAX_DELAY );
            
            if((g_bufferList[i].txBufferLenth > 0) && (g_bufferList[i].txBufferLenth <= bufLength))
            {
                memcpy(pBuffer,g_bufferList[i].txBuffer,g_bufferList[i].txBufferLenth);
                *pLengthOut = g_bufferList[i].txBufferLenth;
                g_bufferList[i].txBufferLenth = 0;
                *pChannelId = i;
                xSemaphoreGive(g_bufferList[i].mutexHandle);
                
                TBOX_PRINT("server recv :");
                for(uint8_t j = 0;j<*pLengthOut;j++)
                {
                    TBOX_PRINT("%02x ",pBuffer[j]);
                }
                TBOX_PRINT("\r\n");
                
                return 0;
            }
            
            xSemaphoreGive(g_bufferList[i].mutexHandle);
        }
        else
        {
            *pLengthOut = 0;
        }
    }
    
    return -1;
}




