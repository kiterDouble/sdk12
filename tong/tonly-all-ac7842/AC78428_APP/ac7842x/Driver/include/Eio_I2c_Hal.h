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
 * @file Eio_I2c_Drv.h
 *
 * @brief This file provides eio i2c integration functions interface.
 *
 */

#ifndef EIO_I2C_HAL_H
#define EIO_I2C_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "Eio_Hal_Types.h"
#include "I2c_Hal_Types.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */
/*!
 * @brief Define the enum of the events which can trigger I2C master callback
 */
typedef enum
{
    EIO_I2C_MASTER_EVENT_END_TRANSFER = I2C_MASTER_EVENT_END_TRANSFER    /*!< i2c master end transfer event */
} eio_i2c_master_event_t;

/*!< Callback for all peripherals which supports I2C features for master mode */
typedef void (*eio_i2c_master_callback_t)(eio_i2c_master_event_t event);

/*!
 * @brief Master configuration structure
 */
typedef struct
{
    uint16 slaveAddress;                  /*!< Slave address, 7-bit */
    Eio_DriverType driverType;         /*!< Driver type: interrupts/polling/DMA */
    uint32 baudRate;                      /*!< Baud rate in hertz */
    uint8 resourceIndex;                   /*!< EIO shifter/timer start index (0 ~ EIO_MAX_SHIFTER_COUNT-2). */
    uint8 sdaPin;                         /*!< eio pin to use as I2C SDA pin */
    uint8 sclPin;                         /*!< eio pin to use as I2C SCL pin */
    eio_i2c_master_callback_t callback;   /*!< User callback function. Note that this function will be
                                            called from the interrupt service routine, so its
                                            execution time should be as small as possible. It can be
                                            NULL if it is not needed */
    uint8 rxDMAChannel;                   /*!< Rx DMA channel number. Only used in DMA mode */
    uint8 txDMAChannel;                   /*!< Tx DMA channel number. Only used in DMA mode */
} eio_i2c_master_user_config_t;

/*!
 * @brief Master internal context structure
 */
typedef struct
{
    Eio_CommonStateType eioCommon;        /*!< Common eio drivers structure */
    const uint8 *txData;                 /*!< Transmit buffer */
    uint8 *rxData;                       /*!< Receive buffer */
    uint32 txRemainingBytes;             /*!< Number of remaining bytes to be transmitted */
    uint32 rxRemainingBytes;             /*!< Number of remaining bytes to be received */
    uint32 dmaReceiveBytes;              /*!< Number of remaining bytes to be dma received */
    uint8 rxDMAChannel;                  /*!< Rx DMA channel number */
    uint8 txDMAChannel;                  /*!< Tx DMA channel number */
    eio_i2c_master_callback_t callback;  /*!< User callback function */
    void *callbackParam;                 /*!< param[in]eter for the callback function */
    uint16 slaveAddress;                 /*!< Slave address */
    uint16 eventCount;                   /*!< Number of timer events, needed for long transfers */
    Eio_DriverType driverType;           /*!< Driver type: interrupts/polling/DMA */
    Hal_StatusType status;               /*!< Current status of the driver */
    boolean receive;                     /*!< Transfer direction, true = receive, false = transmit */
    boolean addrReceived;                /*!< Indicated start of receive (after address transmission) */
    boolean driverIdle;                  /*!< Idle/busy state of the driver */
    boolean blocking;                    /*!< Specifies if the current transfer is blocking */
    boolean sendStop;                    /*!< Specifies if STOP condition must be generated after current transfer */
    uint8 sdaPin;                        /*!< eio pin to use as I2C SDA pin */
    uint8 sclPin;                        /*!< eio pin to use as I2C SCL pin */
    uint32 dummyDmaIdle;                 /*!< Dummy location for DMA transfers */
    uint32 dummyDmaStop;                 /*!< Dummy location for DMA transfers */
    uint32 dummyDmaReceive;              /*!< Dummy location for DMA transfers */
    uint8 lastReload;                    /*!< Counter value for the last timer reload. */
} eio_i2c_master_state_t;

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */
/**
* @brief Initializes the I2C module.
* @note Service ID: NA
* @param[in] HwChannel : I2C hardware channel ID
* @param[in] ChannelConfigPtr : Pointer to I2c_ChannelConfigType
* @param[inout] None
* @param[out] None
* @return Std_ReturnType: E_OK  Initializes Success
                          E_NOT_OK Initializes Failed
*/
Std_ReturnType Eio_I2c_Hal_InitChannel
(
    uint8 HwChannel,
    const I2c_Hal_ChannelConfigType* ChannelConfigPtr
);

/**
* @brief DeInitializes the I2C module.
* @note Service ID: NA
* @param[in] HwChannel : I2C hardware channel ID
* @param[inout] None
* @param[out] None
* @return void
*/
void Eio_I2c_Hal_DeInitChannel
(
    uint8 HwChannel
);

/**
* @brief Starts an asynchronous transmission on the I2C bus.
* @note Service ID: NA
* @param[in] HwChannel : I2C hardware channel ID
* @param[inout] DataTransmitPtr: transmit data used
* @param[out] None
* @return Std_ReturnType: E_OK:  Success to Send or receive
*                         E_NOT_OK: Otherwise
*/
Std_ReturnType Eio_I2c_Hal_AsyncTransmit
(
    uint8 HwChannel,
    const DataTransmitType* DataTransmitPtr
);

/**
* @brief Gets the status of an I2C channel.
* @note Service ID: NA
* @param[in] HwChannel : I2C hardware channel ID
* @param[inout] None
* @param[out] None
* @return I2c_ChannelStatusType: I2c channel currenr status
*/
I2c_Hal_ChannelStatusType Eio_I2c_Hal_GetStatus
(
    uint8 HwChannel
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _EIO_I2C_DRV_H */

/* =============================================  EOF  ============================================== */
