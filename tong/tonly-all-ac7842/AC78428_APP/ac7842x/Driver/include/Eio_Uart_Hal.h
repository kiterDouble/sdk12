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
 * @file Eio_Uart_Hal.h
 *
 * @brief This file provides eio uart integration functions interface.
 *
 */

#ifndef EIO_UART_HAL_H
#define EIO_UART_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "Eio_Uart_Hal_Types.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */
/**
 * @brief Initialize the EIO_UART driver.
 *
 * @details
 * This function initializes the EIO UART driver with the specified configuration.
 * It allocates and configures EIO resources (shifters and timers) for UART operation,
 * sets up the baud rate, and prepares the driver for transmission and/or reception
 * based on the specified direction. The function supports interrupt, polling, and DMA modes.
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[in] UserConfigPtr: Pointer to the EIO_UART user configuration structure.
 *
 * @return
 *     STATUS_SUCCESS: Driver initialized successfully.
 *     STATUS_ERROR: Initialization failed due to resource allocation error or invalid configuration.
 *
 * @pre
 *     EIO module must be clocked and enabled.
 *     Channel must be valid and not already initialized.
 *     UserConfigPtr must not be NULL and contain valid configuration.
 *
 * @post
 *     EIO UART driver is configured and ready for data transfer.
 *     Hardware shifters and timers are configured for UART operation.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This function must be called before any other UART operations.
 *     DES_UART_API_300
 */
Hal_StatusType Eio_Uart_Hal_Init(uint8 Channel, const Eio_UartUserConfigType *UserConfigPtr);

/**
 * @brief De-initialize the EIO_UART driver.
 *
 * @details
 * This function de-initializes the EIO UART driver and releases allocated resources.
 * It stops any ongoing transfers, disables interrupts and DMA requests, and resets
 * the hardware configuration. The function cannot be called if a transfer is in progress.
 *
 * @param[in] Channel: EIO UART channel number.
 *
 * @return
 *     STATUS_SUCCESS: Driver de-initialized successfully.
 *     STATUS_BUSY: De-initialization failed because a transfer is in progress.
 *
 * @pre
 *     Driver must be initialized via Eio_Uart_Hal_Init().
 *     No transfer must be in progress.
 *
 * @post
 *     EIO UART driver is de-initialized and resources are released.
 *     Hardware is reset to idle state.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     Ensure all transfers are completed or aborted before calling this function.
 *     DES_UART_API_301
 */
Hal_StatusType Eio_Uart_Hal_Deinit(uint8 Channel);

/**
 * @brief Set the baud rate.
 *
 * @details
 * This function dynamically changes the UART baud rate by recalculating and updating
 * the timer divider values. The function cannot change the baud rate while a transfer
 * is in progress. The actual baud rate achieved may differ slightly from the requested
 * value due to clock divider limitations.
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[in] BaudRate: The desired baud rate in hertz.
 *
 * @return
 *     STATUS_SUCCESS: Baud rate updated successfully.
 *     STATUS_BUSY: Baud rate change failed because a transfer is in progress.
 *
 * @pre
 *     Driver must be initialized via Eio_Uart_Hal_Init().
 *     No transfer must be in progress.
 *     BaudRate must be greater than 0 and achievable with the current clock frequency.
 *
 * @post
 *     Baud rate is updated for future transfers.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     The function does not validate if the requested baud rate is achievable.
 *     DES_UART_API_302
 */
Hal_StatusType Eio_Uart_Hal_SetBaudRate(uint8 Channel, uint32 BaudRate);

/**
 * @brief Get the currently configured baud rate.
 *
 * @details
 * This function retrieves the current baud rate by reading the timer compare register
 * and calculating the baud rate based on the divider value and input clock frequency.
 * The returned value represents the actual configured baud rate, which may differ
 * slightly from the originally requested value.
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[out] BaudRate: Pointer to store the current baud rate in hertz.
 *
 * @return void
 *
 * @pre
 *     Driver must be initialized via Eio_Uart_Hal_Init().
 *     BaudRate must not be NULL.
 *
 * @post
 *     BaudRate contains the current baud rate value.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     DES_UART_API_303
 */
void Eio_Uart_Hal_GetBaudRate(uint8 Channel, uint32 *BaudRate);

/**
 * @brief Perform a non-blocking UART transmission.
 *
 * @details
 * This function initiates a non-blocking UART transmission of the specified data.
 * The function returns immediately after starting the transfer. The actual transmission
 * is handled by interrupts, polling, or DMA depending on the driver configuration.
 * The user callback is invoked when the transmission completes or when the buffer
 * becomes empty (if more data needs to be provided).
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[in] TxBuff: Pointer to the data buffer to be transmitted.
 * @param[in] TxSize: Length in bytes of the data to be transferred.
 *
 * @return
 *     STATUS_SUCCESS: Transmission started successfully.
 *     STATUS_BUSY: Transmission failed because another transfer is in progress.
 *     STATUS_ERROR: Transmission failed due to DMA configuration error.
 *
 * @pre
 *     Driver must be initialized via Eio_Uart_Hal_Init() with TX direction enabled.
 *     TxBuff must not be NULL and must remain valid until transmission completes.
 *     TxSize must be greater than 0.
 *
 * @post
 *     Transmission is in progress.
 *     User callback will be invoked upon completion or buffer empty event.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     The buffer must remain valid until the transfer completes.
 *     DES_UART_API_304
 */
Hal_StatusType Eio_Uart_Hal_SendData(uint8 Channel, const uint8 *TxBuff, uint32 TxSize);

/**
 * @brief Perform a non-blocking UART reception.
 *
 * @details
 * This function initiates a non-blocking UART reception into the specified buffer.
 * The function returns immediately after starting the reception. The actual data
 * reception is handled by interrupts, polling, or DMA depending on the driver configuration.
 * The user callback is invoked when the reception completes, when the buffer becomes
 * full, or if an error occurs (such as overrun).
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[in] RxBuff: Pointer to the buffer where received data will be stored.
 * @param[in] RxSize: Length in bytes of the data to be received.
 *
 * @return
 *     STATUS_SUCCESS: Reception started successfully.
 *     STATUS_BUSY: Reception failed because another transfer is in progress.
 *     STATUS_ERROR: Reception failed due to DMA configuration error.
 *
 * @pre
 *     Driver must be initialized via Eio_Uart_Hal_Init() with RX direction enabled.
 *     RxBuff must not be NULL and must remain valid until reception completes.
 *     RxSize must be greater than 0.
 *
 * @post
 *     Reception is in progress.
 *     User callback will be invoked upon completion, buffer full, or error event.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     The buffer must remain valid until the transfer completes.
 *     DES_UART_API_305
 */
Hal_StatusType Eio_Uart_Hal_ReceiveData(uint8 Channel, uint8 *RxBuff, uint32 RxSize);

/**
 * @brief Aborts a non-blocking UART transmission.
 *
 * @details
 * This function immediately stops an ongoing UART transmission. It disables the
 * transfer engine, resets hardware state, and marks the transfer status as aborted.
 * Any data remaining in the transmit buffer will not be sent. If no transmission
 * is in progress, the function has no effect.
 *
 * @param[in] Channel: EIO UART channel number.
 *
 * @return
 *     STATUS_SUCCESS: Operation completed (transfer aborted or no transfer was in progress).
 *
 * @pre
 *     Driver must be initialized via Eio_Uart_Hal_Init().
 *
 * @post
 *     Transmission is stopped if it was in progress.
 *     Transfer status is set to STATUS_UART_ABORTED.
 *     Driver returns to idle state.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     No callback is invoked when aborting a transfer.
 *     DES_UART_API_306
 */
Hal_StatusType Eio_Uart_Hal_AbortSendingData(uint8 Channel);

/**
 * @brief Aborts a non-blocking UART reception.
 *
 * @details
 * This function immediately stops an ongoing UART reception. It disables the
 * transfer engine, resets hardware state, and marks the transfer status as aborted.
 * Any data already received is preserved in the buffer, but no additional data
 * will be received. If no reception is in progress, the function has no effect.
 *
 * @param[in] Channel: EIO UART channel number.
 *
 * @return
 *     STATUS_SUCCESS: Operation completed (transfer aborted or no transfer was in progress).
 *
 * @pre
 *     Driver must be initialized via Eio_Uart_Hal_Init().
 *
 * @post
 *     Reception is stopped if it was in progress.
 *     Transfer status is set to STATUS_UART_ABORTED.
 *     Driver returns to idle state.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     No callback is invoked when aborting a transfer.
 *     DES_UART_API_307
 */
Hal_StatusType Eio_Uart_Hal_AbortReceivingData(uint8 Channel);

/**
 * @brief Get the status of the current non-blocking UART transmission.
 *
 * @details
 * This function retrieves the current status of an ongoing or completed transmission
 * and optionally returns the number of bytes remaining to be transmitted. In polling
 * mode, this function also advances the transfer by checking status and writing data.
 * In DMA mode, it queries the DMA controller for the actual remaining byte count.
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[out] BytesRemaining: Pointer to store the remaining number of bytes to be transferred (can be NULL).
 *
 * @return
 *     STATUS_SUCCESS: Transmission completed successfully.
 *     STATUS_BUSY: Transmission is in progress.
 *     STATUS_UART_ABORTED: Transmission was aborted.
 *     STATUS_ERROR: Transmission failed due to an error.
 *     STATUS_IDLE: No transmission in progress.
 *
 * @pre
 *     Driver must be initialized via Eio_Uart_Hal_Init().
 *
 * @post
 *     BytesRemaining (if not NULL) contains the number of bytes remaining.
 *     In polling mode, the transfer is advanced.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     In polling mode, this function must be called periodically to advance the transfer.
 *     DES_UART_API_308
 */
Hal_StatusType Eio_Uart_Hal_GetSendStatus(uint8 Channel, uint32 *BytesRemaining);

/**
 * @brief Get the status of the current non-blocking UART reception.
 *
 * @details
 * This function retrieves the current status of an ongoing or completed reception
 * and optionally returns the number of bytes remaining to be received. In polling
 * mode, this function also advances the transfer by checking status and reading data.
 * In DMA mode, it queries the DMA controller for the actual remaining byte count.
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[out] BytesRemaining: Pointer to store the remaining number of bytes to be received (can be NULL).
 *
 * @return
 *     STATUS_SUCCESS: Reception completed successfully.
 *     STATUS_BUSY: Reception is in progress.
 *     STATUS_UART_ABORTED: Reception was aborted.
 *     STATUS_UART_RX_OVERRUN: Reception failed due to overrun error.
 *     STATUS_ERROR: Reception failed due to an error.
 *     STATUS_IDLE: No reception in progress.
 *
 * @pre
 *     Driver must be initialized via Eio_Uart_Hal_Init().
 *
 * @post
 *     BytesRemaining (if not NULL) contains the number of bytes remaining.
 *     In polling mode, the transfer is advanced.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     In polling mode, this function must be called periodically to advance the transfer.
 *     DES_UART_API_309
 */
Hal_StatusType Eio_Uart_Hal_GetReceiveStatus(uint8 Channel, uint32 *BytesRemaining);

/**
 * @brief Returns default configuration structure for EIO_UART.
 *
 * @details
 * This function fills the provided configuration structure with default values
 * suitable for basic UART operation. The default configuration includes:
 * - EIO instance: 0
 * - Driver type: Interrupt mode
 * - Baud rate: 115200 bps
 * - Bit count: 8 bits
 * - Direction: Both TX and RX
 * - TX pin: 0, RX pin: 1
 * - Callbacks: NULL
 * - DMA channels: 255 (disabled)
 *
 * @param[out] UserConfigPtr: Pointer to the EIO_UART user configuration structure.
 *
 * @return void
 *
 * @pre
 *     UserConfigPtr must not be NULL.
 *
 * @post
 *     UserConfigPtr is filled with default configuration values.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     The user should modify the configuration as needed before calling Eio_Uart_Hal_Init().
 *     DES_UART_API_310
 */
void Eio_Uart_Hal_GetDefaultConfig(Eio_UartUserConfigType *UserConfigPtr);

/**
 * @brief Sets the receive buffer for the specified UART channel.
 *
 * @details
 * This function configures the receive buffer for the specified UART channel.
 * It allows dynamic setting of the buffer where incoming data will be stored.
 * This is particularly useful when handling non-blocking UART reception with
 * large amounts of data that may need to be stored in different buffers.
 * The function updates the internal driver structure with the new buffer
 * information without starting a new reception.
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[in] RxBuff: Pointer to the buffer where received data will be stored.
 * @param[in] RxSize: Length in bytes of the receive buffer.
 *
 * @return void
 *
 * @pre
 *     Driver must be initialized via Eio_Uart_Hal_Init() with RX direction enabled.
 *     RxBuff must not be NULL and must remain valid until reception completes.
 *     RxSize must be greater than 0.
 *
 * @post
 *     Internal driver structure is updated with the new buffer information.
 *     Subsequent reception operations will use the newly configured buffer.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This function only sets the buffer, it does not start a reception.
 *     The buffer must remain valid until all data has been received.
 *     DES_UART_API_328
 */
void Eio_Uart_Hal_SetRxBuffer(uint8 Channel, uint8 *RxBuff, uint32 RxSize);

/**
 * @brief Sets the transmit buffer for the specified UART channel.
 *
 * @details
 * This function configures the transmit buffer for the specified UART channel.
 * It allows dynamic setting of the buffer containing data to be transmitted.
 * This is particularly useful when handling non-blocking UART transmission
 * with data that may be prepared in different buffers. The function updates
 * the internal driver structure with the new buffer information without
 * starting a new transmission.
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[in] TxBuff: Pointer to the data buffer to be transmitted.
 * @param[in] TxSize: Length in bytes of the data to be transferred.
 *
 * @return void
 *
 * @pre
 *     Driver must be initialized via Eio_Uart_Hal_Init() with TX direction enabled.
 *     TxBuff must not be NULL and must remain valid until transmission completes.
 *     TxSize must be greater than 0.
 *
 * @post
 *     Internal driver structure is updated with the new buffer information.
 *     Subsequent transmission operations will use the newly configured buffer.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This function only sets the buffer, it does not start a transmission.
 *     The buffer must remain valid until all data has been transmitted.
 *     DES_UART_API_329
 */
void Eio_Uart_Hal_SetTxBuffer(uint8 Channel, const uint8 *TxBuff, uint32 TxSize);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EIO_UART_HAL_H */
/* =============================================  EOF  ============================================== */
