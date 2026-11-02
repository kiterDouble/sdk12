/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2024. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef UART_HAL_TYPES_H
#define UART_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/
/**
 * @brief Define the enum of the events which can trigger UART callback.
 * @note DES ID: DES_UART_TYP_008
 */
typedef enum
{
    UART_EVENT_RX_FULL = 0x00U,         /*!< Rx buffer received completed */
    UART_EVENT_TX_EMPTY = 0x01U,        /*!< Tx buffer transfer completed */
    UART_EVENT_END_TRANSFER = 0x02U,    /*!< The current transfer is ending */
    UART_EVENT_ERROR = 0x03U,           /*!< An error occured during transfer */
    UART_EVENT_DATA_MATCH = 0x04U,      /*!< Rx data match */
    UART_EVENT_IDLE_LINE = 0x05U        /*!< Idle line */
} Uart_EventType;

/**
 * @brief Callback function pointer type for UART module events.
 * @note DES ID: DES_UART_TYP_001
 */
typedef void (*Uart_CallbackType)(uint8 Instance, Uart_EventType Event);

/**
 * @brief Non-blocking transfer type.
 * @note DES ID: DES_UART_TYP_005
 */
typedef enum
{
    UART_USING_DMA = 0U,    /*!< Use DMA to perform UART transfer */
    UART_USING_INTERRUPTS   /*!< Use interrupts to perform UART transfer */
} Uart_TransferType;

/**
 * @brief UART sample counter enumeration.
 * @note DES ID: DES_UART_TYP_006
 */
typedef enum
{
    UART_SMP_CNT16 = 0U,    /*!< SAM_CNT0: based on 16*baud_pulse */
    UART_SMP_CNT8,          /*!< SAM_CNT1: based on 8*baud_pulse */
    UART_SMP_CNT4,          /*!< SAM_CNT2: based on 4*baud_pulse */
} Uart_SampleCntType;

/**
 * @brief UART number of bits in a character.
 * @note DES ID: DES_UART_TYP_004
 */
typedef enum
{
    UART_7_BITS_PER_CHAR = 0x0U, /*!< 7-bit data characters */
    UART_8_BITS_PER_CHAR = 0x1U, /*!< 8-bit data characters */
    UART_9_BITS_PER_CHAR = 0x2U  /*!< 9-bit data characters */
} Uart_PerCharConfigType;

/**
 * @brief UART stop bit configuration enumeration.
 * @note DES ID: DES_UART_TYP_002
 */
typedef enum
{
    UART_ONE_STOP_BIT = 0x0U,   /*!< One stop bit */
    UART_TWO_STOP_BIT = 0x1U    /*!< Two stop bits */
} Uart_StopBitConfigType;

/**
 * @brief UART parity mode configuration enumeration.
 * @note DES ID: DES_UART_TYP_003
 */
typedef enum
{
    UART_PARITY_DISABLED = 0x0U,    /*!< Parity disabled */
    UART_PARITY_EVEN = 0x1U,        /*!< Parity enabled, type even */
    UART_PARITY_ODD = 0x2U          /*!< Parity enabled, type odd */
} Uart_ParityModeConfigType;

/**
 * @brief UART hardware flow enumeration.
 * @note DES ID: DES_UART_TYP_007
 */
typedef enum
{
    UART_RTSCTS_NONE = 0U,  /*!< RTS CTS: all disable */
    UART_RTS_ONLY,          /*!< RTS only */
    UART_CTS_ONLY,          /*!< CTS only */
    UART_RTSCTS_ALL         /*!< RTS CTS: all Enable */
} Uart_RtsCtsType;

/**
 * @brief UART TX PIN direction enumeration.
 * @note DES ID: DES_UART_TYP_012
 */
typedef enum
{
    UART_TX_PIN_DIR_INPUT = 0U, /*!< TX PIN as input in single-wire mode */
    UART_TX_PIN_DIR_OUTPUT,     /*!< TX PIN as output in single-wire mode */
} Uart_TxPinDirType;

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/**
 * @brief UART channel configuration structure.
 * @note DES ID: DES_UART_TYP_000
 */
typedef struct
{
    uint32 BaudRate;                        /*!< UART baud rate */
    Uart_ParityModeConfigType ParityMode;   /*!< Parity mode, disabled, even, odd */
    Uart_StopBitConfigType StopBitCount;    /*!< Number of stop bits, 1 or 2 stop bits */
    Uart_PerCharConfigType BitCountPerChar; /*!< Number of bits in a character (7, 8 or 9) */
    Uart_TransferType TransferType;         /*!< Type of UART transfer (interrupt or dma) */
    Uart_SampleCntType SampleCnt;           /*!< UART sample counter */
    uint8 RxDmaChannel;                     /*!< DMA RX channel for DMA transfer */
    uint8 TxDmaChannel;                     /*!< DMA TX channel for DMA transfer */
    Uart_CallbackType RxCallback;           /*!< Callback for data receive */
    Uart_CallbackType TxCallback;           /*!< Callback for data send */
} Uart_ChannelConfigType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/

#ifndef UART_SDK_NON_EXTENDED_API
/**
 * @brief Configuration structure for RS485 settings.
 * @note DES ID: DES_UART_TYP_009
 */
typedef struct
{
    boolean Enable;         /*!< Enable or disable RS485 mode */
    boolean InvpolEnable;   /*!< Enable or disable polarity inversion */
    boolean DelayEnable;    /*!< enable or disable RS485 delay function */
    uint8 DelayCnt;         /*!< RS485 delay time*/
    uint8 GuardTime;        /*!< RS484 guard Time*/
} Uart_RS485ConfigType;

/**
 * @brief Configuration structure for IrDA settings.
 * @note DES ID: DES_UART_TYP_010
 */
typedef struct
{
    boolean Enable;     /*!< Enable or disable IrDA mode */
    boolean TxMode;     /*!< Set IrDA mode to transmit(TRUE) or receive(FALSE) */
    uint16 RxWidth;     /*!< Receive pulse width */
    uint8 TxWidth;      /*!< Transmit pulse width*/
} Uart_IrDAConfigType;
#endif

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

#ifdef __cplusplus
}
#endif

#endif
/*============================================EOF===================================================*/
