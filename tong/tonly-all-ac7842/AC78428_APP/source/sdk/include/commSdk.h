#ifndef __COMM_SDK_H__
#define __COMM_SDK_H__

#include <stdint.h>
#include "mpuHal.h"


#define COMM_SDK_OK         0
#define COMM_SDK_ERR_STATUS         0x1001
#define COMM_SDK_ERR_NULL_PTR       0x1002
#define COMM_SDK_ERR_LEN            0x1003
#define COMM_SDK_ERR_CRC            0x1004
#define COMM_SDK_ERR_MODE           0x1005
#define COMM_SDK_ERR_QUEUE          0x1006


typedef enum {
    E_MPU_HAL_START_STATE_IDLE = 0,
    E_MPU_HAL_START_STATE_POWER_OFF,
    E_MPU_HAL_START_STATE_OFF_DELAY,
    E_MPU_HAL_START_STATE_POWER_ON,
    E_MPU_HAL_START_STATE_POWER_DELAY,
    E_MPU_HAL_START_STATE_KEY_ON,
    E_MPU_HAL_START_STATE_KEY_ON_DELAY,
    E_MPU_HAL_START_STATE_KEY_OFF,
    E_MPU_HAL_START_STATE_FINISH,
} MpuHalStartState_e;


void CommSdk_Init(void);

void CommSdk_DeInit(void);

void CommSdk_Start(void);

void CommSdk_Stop(void);

/*************************************************
 Function: CommSdk_SetRxFilter
Description: Set the RX filter for a given handle
Input: int16_t handle: handle index
        const MpuHalFilter_t* pFilter: pointer to the filter configuration, NULL to disable filter
Output: void
Return: int16_t: COMM_SDK_OK if successful, COMM_SDK_ERR_STATUS if error
Others:
*************************************************/
int16_t CommSdk_SetRxFilter(int16_t handle, const MpuHalFilter_t *pFilter);

/*************************************************
 Function: CommSdk_SetRxBuffer
Description: Set the RX buffer for a given handle
Input: int16_t handle: handle index
        uint8_t* pBuffer: pointer to the RX buffer
        uint32_t bufferSize: size of the RX buffer
Output: void
Return: int16_t: COMM_SDK_OK if successful, COMM_SDK_ERR_STATUS if error
Others:
*************************************************/
int16_t CommSdk_SetRxBuffer(int16_t handle, uint8_t *pBuffer, uint32_t bufferSize);


void CommSdk_UpdateState(MpuHalStartState_e state);

/*************************************************
 Function: CommSdk_UartTxTask
 Description: Task to handle UART transmission for MPU HAL
 Input: void *pvParameters: task parameters (not used)
 Output: void
 Return: void
 Others:
*************************************************/
void CommSdk_UartTxTask(void *pvParameters);

/*************************************************
 Function: CommSdk_SpiTxTask
 Description: Task to handle SPI transmission for MPU HAL
 Input: void *pvParameters: task parameters (not used)
 Output: void
 Return: void
 Others:
*************************************************/
void CommSdk_SpiTxTask(void *pvParameters);

void CommSdk_Sleep(void);

void CommSdk_Wakeup(void);

#endif  /* __COMM_SDK_H__ */