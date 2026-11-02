/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: mpuHal.h
 Author:
 Created Time:
 Description:
 Others:
*************************************************/
#ifndef _MPU_HAL_IF_H
#define _MPU_HAL_IF_H
/****************************** include ***************************************/
#include <stdint.h>
#include "Uart_Hal.h"
/****************************** Macro Definitions ******************************/
#define MPU_HAL_UART2_DEBUG 0 // 使用UART2调试需要禁用logHal初始化
#define SPI_TEST_ENABLE 0 // 设置为1会启用SPI测试功能，打印发送和接收数据

#define MPU_HAL_STATUS_OK                      0
#define MPU_HAL_STATUS_ERR                     0x1001
#define MPU_HAL_NULL_PTR_ERR                   0x1002
#define MPU_HAL_LEN_ERR                        0x1003
#define MPU_HAL_CRC_ERR                        0x1004
#define MPU_HAL_MODE_ERR                       0x1005
#define MPU_HAL_QUEUE_ERR                      0x1006

/* MPU SPI相关宏定义 */
#define MPU_HAL_SPI_REQ_PORT           PORTID_E
#define MPU_HAL_SPI_REQ_PORT_PIN       8U
#define MPU_HAL_SPI_REQ_PORT_MUX            PORT_MUX_AS_GPIO
#define MPU_HAL_SPI_REQ_IRQN           PORTE_IRQn

/* error code*/
#define MPU_HAL_OK 0x0
#define MPU_HAL_PROC_BUFFER_LEN_ERR 0x10000001
#define MPU_HAL_DMA_TIMEOUT_ERR 0x10000002
#define MPU_HAL_DMA_TXBUFF_BUSY_ERR 0x10000003

#define MPU_HAL_RX_ONE_FRAME_MAX_SIZE (4096)
/****************************** Type Definitions ******************************/
typedef struct
{
        uint8_t aid;
        uint8_t mid;
        uint8_t subcommand;
        uint16_t dataBufferSize;
        uint16_t dataLength;
        uint8_t *pDataBuffer;
} MpuHalDataPack_t;

typedef struct
{
        uint8_t aid;
        uint8_t midMin;
        uint8_t midMax;
} MpuHalFilter_t;

typedef enum
{
        MPU_HAL_UART_MODE = 0,
        MPU_HAL_SPI_MODE = 1,
        MPU_HAL_TX_MODE_MAX = 2,
} MpuHalTXMode_e;

/****************************** Function Declarations *************************/
/*************************************************
 Function: MpuHal_BootInit
 Description: Initialize the MPU HAL
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHal_BootInit(void);

/*************************************************
 Function: MpuHal_DiagnosticTask
 Description: Diagnostic task for MPU HAL errors
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHal_PrintDiagnosticReport(void);

/*************************************************
 Function: MpuHalCycleProcess
Description: Process the MPU HAL cycle
Input: uint32_t cycleTime: cycle time in milliseconds
Output: void
Return: void
Others:
*************************************************/
void MpuHalCycleProcess(uint32_t cycleTime);

/*************************************************
 Function: MpuHal_ProcessRxData
 Description: 主循环中处理接收到的UART数据
 Input: void
 Output: void
 Return: void
 Others: 
*************************************************/
void MpuHal_ProcessRxData(void);

/*************************************************
 Function: MpuHalOpen
Description: Open a MPU HAL handle
Input: void
Output: void
Return: int16_t: handle index, -1 if no available handle
Others:
*************************************************/
int16_t MpuHalOpen(void);

/*************************************************
 Function: MpuHalSetRxFilter
Description: Set the RX filter for a given handle
Input: int16_t handle: handle index
        const MpuHalFilter_t* pFilter: pointer to the filter configuration, NULL to disable filter
Output: void
Return: int16_t: MPU_HAL_STATUS_OK if successful, MPU_HAL_STATUS_ERR if error
Others:
*************************************************/
int16_t MpuHalSetRxFilter(int16_t handle, const MpuHalFilter_t *pFilter);

/*************************************************
 Function: MpuHalSetRxBuffer
Description: Set the RX buffer for a given handle
Input: int16_t handle: handle index
        uint8_t* pBuffer: pointer to the RX buffer
        uint32_t bufferSize: size of the RX buffer
Output: void
Return: int16_t: MPU_HAL_STATUS_OK if successful, MPU_HAL_STATUS_ERR if error
Others:
*************************************************/
int16_t MpuHalSetRxBuffer(int16_t handle, uint8_t *pBuffer, uint32_t bufferSize);

/*************************************************
 Function: MpuHal_Transmit
 Description: Transmit MPU HAL data packet using UART
 Input: handle - MPU HAL handle index
        pTxMsg - Pointer to the MpuHalDataPack_t structure containing the data packet to be transmitted
 Output: void
 Return: int16_t - MPU_HAL_STATUS_OK if successful, MPU_HAL_STATUS_ERR if failed
 Others:
*************************************************/
int16_t MpuHal_Transmit(int16_t handle, const MpuHalDataPack_t* pTxMsg);

/*************************************************
 Function: MpuHal_Receive
 Description: Receive a data packet via MPU HAL
 Input: int16_t handle: handle index
        MpuHalDataPack_t* pRxMsg: pointer to the data packet to be received
        uint32_t waitTime: maximum wait time in ticks
 Output: void
 Return: int16_t: MPU_HAL_STATUS_OK if successful, MPU_HAL_STATUS_ERR if error
 Others:
*************************************************/
int16_t MpuHal_Receive(int16_t handle, MpuHalDataPack_t *pRxMsg, uint32_t waitTime);

/*************************************************
 Function: MpuHalStartIsFinished
 Description: Check if the MPU HAL start process is finished
 Input: void
 Output: void
 Return: int16_t: MPU_HAL_STATUS_OK if finished, MPU_HAL_STATUS_ERR if not finished
 Others:
*************************************************/
void MpuHalStart(void);

/*************************************************
 Function: MpuHalStartIsFinished
 Description: Check if the MPU HAL start process is finished
 Input: void
 Output: void
 Return: int16_t: MPU_HAL_STATUS_OK if finished, MPU_HAL_STATUS_ERR if not finished
 Others:
*************************************************/
int16_t MpuHalStartIsFinished(void);

/*************************************************
 Function: MpuHalReset
 Description: Reset the MPU HAL
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHalReset(void);

/*************************************************
 Function: MpuHalResetIsFinished
 Description: Check if the MPU HAL reset process is finished
 Input: void
 Output: void
 Return: int16_t: MPU_HAL_STATUS_OK if finished, MPU_HAL_STATUS_ERR if not finished
 Others:
*************************************************/
int16_t MpuHalResetIsFinished(void);

/*************************************************
 Function: MpuHalPowerOff
 Description: Power off the MPU HAL
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHalPowerOff(void);

/*************************************************
 Function: MpuHalSetMode
 Description: Set the MPU HAL mode (wake or sleep)
 Input: uint8_t wakeMode: 0 for sleep mode, 1 for wake mode
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHalSetMode(uint8_t wakeMode);

/*************************************************
 Function: MpuHalGetWakeMode
 Description: Get the current MPU HAL mode (wake or sleep)
 Input: void
 Output: void
 Return: int16_t: 0 for sleep mode, 1 for wake mode
 Others:
*************************************************/
int16_t MpuHalGetWakeMode(void);

/*************************************************
 Function:       MpuHalUartTimerCallBack
 Description:    Timer callback function for MPU UART communication
 Input:          None
 Output:         None
 Return:         None
 Others:         Calls UartProtocalProcess to handle received data in timer interrupt context
*************************************************/
void MpuHalUartTimerCallBack(void);

void MpuHalSetPower(uint8_t flag);

/*************************************************
 Function: MpuHal_BootGpioInit
 Description: Initialize the GPIO used by MPU
 Input: void
 Output: void
Return:void
 Others:
*************************************************/
void MpuHal_BootGpioInit(void);


#endif
