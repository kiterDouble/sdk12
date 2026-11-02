/*************************************************
 Copyright © 2025 SiRun (WuXi) . All rights reserved.
 File Name: canPeriodTask.h
 Author:
 Created Time:
 Description:
 Others:
*************************************************/
#ifndef __DIGITAL_KEY_H__
#define __DIGITAL_KEY_H__

/****************************** include ***************************************/
#include <stdint.h>
#include "canHal.h"
/****************************** Macro Definitions ******************************/

// #define DIGITAL_KEY_CANID_MAX_NUM         10U

#define DIGITAL_KEY_CAN_RX_BUFFER_SIZE      10U
#define DIGITAL_KEY_CAN_CHANNEL             TBOX_CAN_CHANNEL_3

#define BLE_UATY_FRAME_FIX_LEN              6U
#define BLE_UART_FRAME_HEADER               0x3CU
#define BLE_UART_FRAME_TAILER               0xC3U

/****************************** Type Definitions ******************************/

void DigitalKey_Task(void *pvParameters);

#endif /* __DIGITAL_KEY_H__ */
