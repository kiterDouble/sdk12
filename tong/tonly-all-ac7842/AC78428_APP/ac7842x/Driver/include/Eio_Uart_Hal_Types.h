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
 * AutoChips Inc. (C) 2021. All rights reserved.
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

/*!
 * @file Eio_Uart_Hal_Types.h
 *
 * @brief This file provides eio uart integration functions interface.
 *
 */

#ifndef EIO_UART_HAL_TYPES_H
#define EIO_UART_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "Eio_Hal_Types.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */
/**
 * @brief Define the enum of the events which can trigger UART callback
 * @note DES_UART_TYP_301
 */
typedef enum
{
    EIO_UART_EVENT_RX_FULL      = 0U,    /*!< Rx buffer is full */
    EIO_UART_EVENT_TX_EMPTY     = 1U,    /*!< Tx buffer is empty */
    EIO_UART_EVENT_END_TRANSFER = 2U,    /*!< The current transfer is ending */
    EIO_UART_EVENT_ERROR        = 3U     /*!< An error occurred during transfer */
} Eio_UartEventType;

/**
 * @brief Callback for all peripherals which support UART features
 * @note DES_UART_TYP_302
 */
typedef void (*Eio_UartCallbackType)(uint8 Instance, Eio_UartEventType Event);

/**
 * @brief EIO UART driver direction (tx or rx)
 * @note DES_UART_TYP_303
 */
typedef enum
{
    EIO_UART_DIRECTION_TX       = 1U,    /*!< Tx UART driver (bit 0) */
    EIO_UART_DIRECTION_RX       = 2U,    /*!< Rx UART driver (bit 1) */
    EIO_UART_DIRECTION_BOTH     = 3U     /*!< Tx and Rx UART driver (bit 0 | bit 1) */
} Eio_UartDriverDirectionType;

/*PRQA S 3630 ++ # AMCE_QAC_3630 */
/**
 * @brief Driver configuration structure
 * @note DES_UART_TYP_304
 */
typedef struct
{
    uint8 EioInstance;                              /*!< EIO instance number */
    Eio_DriverType DriverType;                      /*!< Driver type: interrupts/polling/DMA */
    uint32 BaudRate;                                /*!< Baud rate in hertz */
    uint8 BitCount;                                 /*!< Number of bits per word */
    Eio_UartDriverDirectionType Direction;          /*!< Driver direction: Tx or Rx */
    uint8 TxResourceIndex;                          /*!< EIO shifter/timer index to use for Tx (0 ~ EIO_MAX_SHIFTER_COUNT-1) */
    uint8 RxResourceIndex;                          /*!< EIO shifter/timer index to use for Rx (0 ~ EIO_MAX_SHIFTER_COUNT-1) */
    uint8 TxDataPin;                                /*!< EIO pin to use as Tx pin */
    uint8 RxDataPin;                                /*!< EIO pin to use as Rx pin */
    Eio_UartCallbackType TxCallback;                /*!< User callback function for Tx events */
    Eio_UartCallbackType RxCallback;                /*!< User callback function for Rx events */
    uint8 TxDmaChannel;                             /*!< DMA channel number for Tx. Only used in DMA mode */
    uint8 RxDmaChannel;                             /*!< DMA channel number for Rx. Only used in DMA mode */
} Eio_UartUserConfigType;
/*PRQA S 3630 -- # AMCE_QAC_3630 */
/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EIO_UART_HAL_TYPES_H */

/* =============================================  EOF  ============================================== */
