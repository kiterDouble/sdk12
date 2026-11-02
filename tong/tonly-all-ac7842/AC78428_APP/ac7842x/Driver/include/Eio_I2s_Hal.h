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

/*!
 * @file Eio_I2s_Hal.h
 *
 * @brief This file provides eio i2s integration functions interface.
 *
 */

#ifndef EIO_I2S_HAL_H
#define EIO_I2S_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "Eio_I2s_Hal_Types.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */
/*!
 * @brief Initialize the EIO I2S driver
 *
 * @details
 * This function initializes the EIO I2S driver for the specified channel.
 * It configures the hardware according to the user configuration structure.
 * The configuration includes baud rate, bit width, transfer mode, and callback settings.
 *
 * @param[in] Channel: EIO I2S channel number to initialize
 * @param[in] UserConfigPtr: Pointer to the EIO I2S user configuration structure. The function
 *                           reads configuration data from this structure and initializes the
 *                           driver accordingly. The application may free this structure after
 *                           the function returns.
 *
 * @return
 *     HAL_STATUS_SUCCESS: Initialization successful
 *     HAL_STATUS_ERROR: Initialization failed due to invalid parameters or hardware error
 *
 * @pre
 *     The EIO I2S hardware must be clocked and enabled.
 *     UserConfigPtr must not be NULL and must point to a valid configuration structure.
 *
 * @post
 *     The EIO I2S channel is initialized and ready for data transfer operations.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     This function should not be called from an ISR context.
 *     DES ID: DES_I2S_API_001
 */
Hal_StatusType Eio_I2s_Hal_Init(uint8 Channel, const Eio_I2sUserConfigType *UserConfigPtr);

/*!
 * @brief De-initialize the EIO I2S driver
 *
 * @details
 * This function de-initializes the EIO I2S driver for the specified channel.
 * It disables the hardware and resets the channel configuration.
 * The driver cannot be used again until reinitialized.
 *
 * @param[in] Channel: EIO I2S channel number to de-initialize
 *
 * @return
 *     HAL_STATUS_SUCCESS: De-initialization successful
 *     HAL_STATUS_ERROR: De-initialization failed
 *
 * @pre
 *     The EIO I2S channel must be initialized via Eio_I2s_Hal_Init().
 *     No ongoing transfer should be in progress.
 *
 * @post
 *     The EIO I2S channel is disabled and reset to default state.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     This function should not be called from an ISR context.
 *     DES ID: DES_I2S_API_002
 */
Hal_StatusType Eio_I2s_Hal_Deinit(uint8 Channel);

/*!
 * @brief Set the sample rate for I2S communication
 *
 * @details
 * This function sets the sample rate (SCK frequency) for the specified I2S channel.
 * Due to hardware limitations, not every sample rate can be achieved exactly. The driver
 * will set a sample rate as close as possible to the requested value. There may be
 * substantial differences, especially when requesting a high sample rate while using
 * a low-frequency clock source. Use Eio_I2s_Hal_GetSampleRate() to verify the actual
 * sample rate that was configured.
 *
 * @param[in] Channel: EIO I2S channel number
 * @param[in] SampleRate: Desired sample rate in hertz
 *
 * @return
 *     HAL_STATUS_SUCCESS: Sample rate configured successfully
 *     HAL_STATUS_ERROR: Configuration failed due to invalid parameters
 *
 * @pre
 *     The EIO I2S channel must be initialized via Eio_I2s_Hal_Init().
 *
 * @post
 *     The I2S sample rate is updated for subsequent communication.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     This function should not be called during an ongoing transfer.
 *     DES ID: DES_I2S_API_003
 */
Hal_StatusType Eio_I2s_Hal_SetSampleRate(uint8 Channel, uint32 SampleRate);

/*!
 * @brief Get the currently configured sample rate
 *
 * @details
 * This function retrieves the currently configured I2S sample rate for the specified channel.
 * The returned value reflects the actual sample rate that was set by the hardware,
 * which may differ from the requested value due to clock divider limitations.
 *
 * @param[in] Channel: EIO I2S channel number
 * @param[out] SampleRate: Pointer to store the current sample rate in hertz
 *
 * @return
 *     HAL_STATUS_SUCCESS: Sample rate retrieved successfully
 *     HAL_STATUS_ERROR: Operation failed due to invalid parameters
 *
 * @pre
 *     The EIO I2S channel must be initialized via Eio_I2s_Hal_Init().
 *     SampleRate must not be NULL.
 *
 * @post
 *     The current sample rate value is written to the SampleRate pointer.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     DES ID: DES_I2S_API_004
 */
Hal_StatusType Eio_I2s_Hal_GetSampleRate(uint8 Channel, uint32 *SampleRate);

/*!
 * @brief Perform a non-blocking send transaction on the I2S bus
 *
 * @details
 * This function initiates the transmission of a block of data and returns immediately.
 * The rest of the transmission is handled by the interrupt service routine (if the driver
 * is initialized in interrupt mode) or by polling (if initialized in polling mode).
 * Use Eio_I2s_Hal_GetStatus() to check the progress of the transmission.
 *
 * @param[in] Channel: EIO I2S channel number
 * @param[in] TxBuff: Pointer to the data to be transmitted
 * @param[in] TxSize: Length in bytes of the data to be transmitted
 *
 * @return
 *     HAL_STATUS_SUCCESS: Transmit request accepted successfully
 *     HAL_STATUS_BUSY: Channel is busy with another transfer
 *     HAL_STATUS_ERROR: Operation failed due to invalid parameters
 *
 * @pre
 *     The EIO I2S channel must be initialized via Eio_I2s_Hal_Init().
 *     TxBuff must not be NULL and must point to valid data.
 *     No other transfer should be in progress on this channel.
 *
 * @post
 *     Data transmission is initiated and will complete asynchronously.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     The TxBuff memory must remain valid until the transfer completes.
 *     DES ID: DES_I2S_API_005
 */
Hal_StatusType Eio_I2s_Hal_SendData(uint8 Channel, const uint8 *TxBuff, uint32 TxSize);

/*!
 * @brief Perform a non-blocking receive transaction on the I2S bus
 *
 * @details
 * This function initiates the reception of a block of data and returns immediately.
 * The rest of the reception is handled by the interrupt service routine (if the driver
 * is initialized in interrupt mode) or by polling (if initialized in polling mode).
 * Use Eio_I2s_Hal_GetStatus() to check the progress of the reception.
 *
 * @param[in] Channel: EIO I2S channel number
 * @param[out] RxBuff: Pointer to the buffer where received data will be stored
 * @param[in] RxSize: Length in bytes of the data to be received
 *
 * @return
 *     HAL_STATUS_SUCCESS: Receive request accepted successfully
 *     HAL_STATUS_BUSY: Channel is busy with another transfer
 *     HAL_STATUS_ERROR: Operation failed due to invalid parameters
 *
 * @pre
 *     The EIO I2S channel must be initialized via Eio_I2s_Hal_Init().
 *     RxBuff must not be NULL and must have sufficient space for RxSize bytes.
 *     No other transfer should be in progress on this channel.
 *
 * @post
 *     Data reception is initiated and will complete asynchronously.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     The RxBuff memory must remain valid until the transfer completes.
 *     DES ID: DES_I2S_API_006
 */
Hal_StatusType Eio_I2s_Hal_ReceiveData(uint8 Channel, uint8 *RxBuff, uint32 RxSize);

/*!
 * @brief Abort a non-blocking I2S transaction
 *
 * @details
 * This function aborts an ongoing non-blocking I2S transfer on the specified channel.
 * Any data that has not yet been transmitted or received will be discarded.
 * The channel will return to idle state after the abort operation.
 *
 * @param[in] Channel: EIO I2S channel number
 *
 * @return
 *     HAL_STATUS_SUCCESS: Transfer aborted successfully
 *     HAL_STATUS_ERROR: Operation failed
 *
 * @pre
 *     The EIO I2S channel must be initialized via Eio_I2s_Hal_Init().
 *
 * @post
 *     Any ongoing transfer is terminated and the channel returns to idle state.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     This function can be called from an ISR context if necessary.
 *     DES ID: DES_I2S_API_007
 */
Hal_StatusType Eio_I2s_Hal_TransferAbort(uint8 Channel);

/*!
 * @brief Get the status of the current non-blocking I2S transaction
 *
 * @details
 * This function returns the current status of a non-blocking I2S transaction.
 * A return status of HAL_STATUS_BUSY means the transfer is still in progress.
 * Otherwise the function returns a status reflecting the outcome of the last transfer.
 * When the driver is initialized in polling mode, this function also advances the transfer
 * by checking and handling transmit and receive events, so it must be called frequently
 * to avoid overflows or underflows.
 *
 * @param[in] Channel: EIO I2S channel number
 * @param[out] BytesRemaining: Pointer to store the remaining number of bytes to be transferred
 *
 * @return
 *     HAL_STATUS_SUCCESS: Transfer completed successfully
 *     HAL_STATUS_BUSY: Transfer is still in progress
 *     HAL_STATUS_ERROR: Transfer failed or invalid parameters
 *
 * @pre
 *     The EIO I2S channel must be initialized via Eio_I2s_Hal_Init().
 *     A transfer must have been initiated via Eio_I2s_Hal_SendData() or Eio_I2s_Hal_ReceiveData().
 *     BytesRemaining must not be NULL.
 *
 * @post
 *     The number of remaining bytes is written to BytesRemaining.
 *     In polling mode, the transfer is advanced by processing pending events.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     In polling mode, this function must be called frequently to ensure timely data transfer.
 *     DES ID: DES_I2S_API_008
 */
Hal_StatusType Eio_I2s_Hal_GetStatus(uint8 Channel, uint32 *BytesRemaining);

/*!
 * @brief Get default configuration structure for EIO I2S master mode
 *
 * @details
 * This function fills the provided configuration structure with default values
 * suitable for I2S master mode operation. The application can use these default
 * values as a starting point and modify specific fields as needed before calling
 * Eio_I2s_Hal_Init().
 *
 * @param[out] userConfigPtr: Pointer to the EIO I2S user configuration structure to be filled
 *
 * @return
 *     void
 *
 * @pre
 *     UserConfigPtr must not be NULL and must point to a valid configuration structure.
 *
 * @post
 *     The configuration structure is filled with default values for master mode.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This is a utility function that can be called at any time.
 *     DES ID: DES_I2S_API_009
 */
void Eio_I2s_Hal_MasterGetDefaultConfig(Eio_I2sUserConfigType *UserConfigPtr);

/*!
 * @brief Get default configuration structure for EIO I2S slave mode
 *
 * @details
 * This function fills the provided configuration structure with default values
 * suitable for I2S slave mode operation. The application can use these default
 * values as a starting point and modify specific fields as needed before calling
 * Eio_I2s_Hal_Init().
 *
 * @param[out] userConfigPtr: Pointer to the EIO I2S user configuration structure to be filled
 *
 * @return
 *     void
 *
 * @pre
 *     UserConfigPtr must not be NULL and must point to a valid configuration structure.
 *
 * @post
 *     The configuration structure is filled with default values for slave mode.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This is a utility function that can be called at any time.
 *     DES ID: DES_I2S_API_010
 */
void Eio_I2s_Hal_SlaveGetDefaultConfig(Eio_I2sUserConfigType *UserConfigPtr);

/*!
 * @brief Provide a buffer for receiving data
 *
 * @details
 * This function can be used to provide a new buffer for receiving data to the driver.
 * It can be called from the user callback when event EIO_I2S_EVENT_RX_FULL is reported.
 * This way the reception will continue without interruption.
 *
 * @param[in] Channel: EIO I2S channel number
 * @param[in] RxBuff: pointer to the buffer where to store received data
 * @param[in] RxSize: length in bytes of the data to be transferred
 *
 * @return
 *     void
 *
 * @pre
 *     Channel must be initialized via Eio_I2s_Hal_Init().
 *     RxBuff must not be NULL and must point to a valid buffer of at least RxSize bytes.
 *     RxSize must be greater than 0 and divisible by the byte width of the channel.
 *
 * @post
 *     The driver will use the new buffer for subsequent receive operations.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     This function should not be called from an ISR context.
 *     DES ID: DES_I2S_API_XXX
 */
void Eio_I2s_Hal_SetRxBuffer(uint8 Channel, uint8 *RxBuff, uint32 RxSize);

/*!
 * @brief Provide a buffer for transmitting data
 *
 * @details
 * This function can be used to provide a new buffer for transmitting data to the driver.
 * It can be called from the user callback when event EIO_I2S_EVENT_TX_EMPTY is reported.
 * This way the transmission will continue without interruption.
 *
 * @param[in] Channel: EIO I2S channel number
 * @param[in] TxBuff: pointer to the buffer containing transmit data
 * @param[in] TxSize: length in bytes of the data to be transferred
 *
 * @return
 *     void
 *
 * @pre
 *     Channel must be initialized via Eio_I2s_Hal_Init().
 *     TxBuff must not be NULL and must point to a valid buffer of at least TxSize bytes.
 *     TxSize must be greater than 0 and divisible by the byte width of the channel.
 *
 * @post
 *     The driver will use the new buffer for subsequent transmit operations.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     This function should not be called from an ISR context.
 *     DES ID: DES_I2S_API_XXX
 */
void Eio_I2s_Hal_SetTxBuffer(uint8 Channel, const uint8 *TxBuff, uint32 TxSize);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EIO_I2S_HAL_H */

/* =============================================  EOF  ============================================== */
