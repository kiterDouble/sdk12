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
 * @file Eio_I2s_Hal_Types.h
 *
 * @brief This file provides eio i2s integration functions interface.
 *
 */

#ifndef EIO_I2S_HAL_TYPES_H
#define EIO_I2S_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "Eio_Hal_Types.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */
/**
 * @brief I2S events which can trigger the I2S callback
 * @note DES_I2S_TYP_001
 */
typedef enum
{
    EIO_I2S_EVENT_RX_FULL      = 0U,    /*!< Rx buffer is full */
    EIO_I2S_EVENT_TX_EMPTY     = 1U,    /*!< Tx buffer is empty */
    EIO_I2S_EVENT_END_TRANSFER = 2U,    /*!< The current transfer is ending */
    EIO_I2S_EVENT_ERROR        = 3U     /*!< An error occurred during transfer */
} Eio_I2sEventType;

/**
 * @brief Callback type for I2S events
 * @note DES_I2S_TYP_004
 */
typedef void (*Eio_I2sCallbackType)(uint8 Channel, Eio_I2sEventType Event);

/**
 * @brief I2S driver mode selection
 * @note DES_I2S_TYP_002
 */
typedef enum
{
    EIO_I2S_MASTER = 0U,    /*!< Master mode */
    EIO_I2S_SLAVE  = 1U     /*!< Slave mode */
} Eio_I2sModeType;

/**
 * @brief I2S driver configuration structure
 * @note DES_I2S_TYP_003
 */
typedef struct
{
    uint8 EioInstance;                      /*!< EIO instance number */
    uint8 ResourceIndex;                    /*!< EIO shifter/timer start index (0 ~ EIO_MAX_SHIFTER_COUNT-1). ResourceCount resources are used. */
    Eio_I2sModeType Mode;                   /*!< I2S mode: Master or Slave */
    Eio_DriverType DriverType;              /*!< Driver type: interrupts/polling/DMA */
    uint32 SampleRate;                      /*!< Sample rate in hertz (only used in Master mode) */
    uint8 BitsWidth;                        /*!< Number of bits in a word - multiple of 8 */
    uint8 TxPin;                            /*!< eio pin to use for transmit */
    uint8 RxPin;                            /*!< eio pin to use for receive */
    uint8 SckPin;                           /*!< eio pin to use for serial clock */
    uint8 WsPin;                            /*!< eio pin to use for word select */
    Eio_I2sCallbackType Callback;           /*!< User callback function. Note that this function will be
                                                 called from the interrupt service routine, so its
                                                 execution time should be as small as possible. It can be
                                                 NULL if it is not needed */
    uint8 RxDmaChannel;                     /*!< Rx DMA channel number. Only used in DMA mode */
    uint8 TxDmaChannel;                     /*!< Tx DMA channel number. Only used in DMA mode */
    boolean ClockContinueOutput;            /*!< Continue clock output when transmitter is idle */
} Eio_I2sUserConfigType;

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EIO_I2S_HAL_TYPES_H */

/* =============================================  EOF  ============================================== */
