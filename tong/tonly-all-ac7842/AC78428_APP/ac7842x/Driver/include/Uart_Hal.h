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
* @file Uart_Hal.c
* @brief This file provides Uart hal function extern
*/

#ifndef UART_HAL_H
#define UART_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Uart_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief Initializes the UART hardware module.
 *
 * @details
 * This function configures the specified UART instance with the provided
 * configuration parameters including baud rate, parity, stop bits, data bits,
 * transfer type, and callback functions. It prepares the UART hardware for
 * subsequent transmit and receive operations.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] ChannelConfigPtr: Pointer to the UART channel configuration structure.
 *
 * @return void
 *
 * @pre
 *     The UART channel must not be initialized.
 *     The ChannelConfigPtr parameter must point to a valid configuration structure.
 *
 * @post
 *     The UART instance is initialized and ready for data transmission and reception.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     This function must be called before any other UART HAL operations on the instance.
 *     DES ID: DES_UART_API_000
 */
void Uart_Hal_Init(uint8 Instance, const Uart_ChannelConfigType *ChannelConfigPtr);

/**
 * @brief Deinitializes the UART hardware module.
 *
 * @details
 * This function resets the specified UART instance to its default state,
 * disables all interrupts, clears pending flags, and releases any allocated
 * resources. After calling this function, the UART instance cannot be used
 * until reinitialized.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return void
 *
 * @pre
 *     The UART instance must have been previously initialized.
 *     No active transmission or reception should be in progress.
 *
 * @post
 *     The UART instance is reset to default state and all operations are stopped.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     After deinitialization, Uart_Hal_Init() must be called again to use the UART instance.
 *     DES ID: DES_UART_API_001
 */
void Uart_Hal_DeInit(uint8 Instance);

/**
 * @brief Transmits data using non-blocking method (interrupt or DMA).
 *
 * @details
 * This function initiates a non-blocking transmission of the specified data buffer
 * through the UART interface. The transfer method (interrupt or DMA) is determined
 * by the configuration set during initialization. The function returns immediately
 * after starting the transmission, and completion is signaled via callback.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] TxBuff: Pointer to the transmit data buffer.
 * @param[in] TxSize: Number of bytes to transmit.
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: Transmission started successfully.
 *     STATUS_ERROR: Invalid parameters or UART not initialized.
 *     STATUS_BUSY: Previous transmission still in progress.
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     TxBuff must point to a valid buffer with at least TxSize bytes.
 *     No other transmission should be in progress on this instance.
 *
 * @post
 *     Transmission is initiated and will complete asynchronously.
 *     Callback will be invoked upon completion or error.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     The TxBuff buffer must remain valid until the transmission completes.
 *     DES ID: DES_UART_API_002
 */
Hal_StatusType Uart_Hal_SendData(uint8 Instance, const uint8 *TxBuff, uint32 TxSize);

#ifndef UART_SDK_NON_EXTENDED_API
/**
 * @brief Transmits data using polling mode.
 *
 * @details
 * This function transmits the specified data buffer through the UART interface
 * using polling method. The function continuously polls the transmit buffer status
 * and sends data byte by byte. It returns when all bytes are loaded into the
 * transmit buffer, but may not wait for actual hardware transmission completion.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] TxBuff: Pointer to the transmit data buffer.
 * @param[in] TxSize: Number of bytes to transmit.
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: All data loaded into transmit buffer successfully.
 *     STATUS_BUSY: UART is busy or unable to transmit.
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     TxBuff must point to a valid buffer with at least TxSize bytes.
 *
 * @post
 *     All data bytes are loaded into the transmit buffer.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     This function blocks until all data is loaded into the transmit buffer.
 *     Do not use this function in time-critical contexts or ISRs.
 *     DES ID: DES_UART_API_003
 */
Hal_StatusType Uart_Hal_SendDataPolling(uint8 Instance, const uint8 *TxBuff, uint32 TxSize);

/**
 * @brief Transmits data using blocking method with timeout.
 *
 * @details
 * This function transmits the specified data buffer through the UART interface
 * using a blocking method. The function waits until the entire transmission is
 * complete or the specified timeout expires. It ensures all data is physically
 * transmitted from the hardware before returning.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] TxBuff: Pointer to the transmit data buffer.
 * @param[in] TxSize: Number of bytes to transmit.
 * @param[in] TimeoutUs: Maximum time in microseconds to wait for transmission completion.
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: All data transmitted successfully.
 *     STATUS_TIMEOUT: Transmission did not complete within the timeout period.
 *     STATUS_ERROR: Invalid parameters or transmission error occurred.
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     TxBuff must point to a valid buffer with at least TxSize bytes.
 *
 * @post
 *     All data bytes are transmitted through the UART hardware.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     This function blocks the calling thread until transmission completes or timeout.
 *     Do not call from ISR or time-critical contexts.
 *     The timeout is approximate and depends on system tick resolution.
 *     DES ID: DES_UART_API_004
 */
Hal_StatusType Uart_Hal_SendDataBlocking(uint8 Instance, const uint8 *TxBuff, uint32 TxSize, uint32 TimeoutUs);
#endif

/**
 * @brief Retrieves the current transmission status.
 *
 * @details
 * This function queries the status of an ongoing non-blocking transmission
 * and returns the number of bytes remaining to be transmitted. It can be
 * used to monitor transmission progress or verify completion.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[out] BytesRemaining: Pointer to store the number of bytes remaining to be transmitted.
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: Transmission completed, no bytes remaining.
 *     STATUS_BUSY: Transmission in progress, BytesRemaining contains count.
 *     STATUS_ERROR: Invalid parameters or no transmission was initiated.
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     BytesRemaining must point to a valid uint32 variable.
 *
 * @post
 *     The BytesRemaining variable is updated with current transmission status.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This function is intended for monitoring non-blocking transmissions.
 *     DES ID: DES_UART_API_008
 */
Hal_StatusType Uart_Hal_GetSendStatus(uint8 Instance, uint32 *BytesRemaining);

/**
 * @brief Terminates an ongoing non-blocking transmission.
 *
 * @details
 * This function aborts an active non-blocking transmission on the specified
 * UART instance. It stops any further data transmission, disables related
 * interrupts or DMA channels, and resets the transmission state. Any unsent
 * data in the buffer is discarded.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: Transmission aborted successfully.
 *     STATUS_ERROR: No active transmission to abort or invalid instance.
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *
 * @post
 *     The transmission is stopped and the UART is ready for new operations.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     Any data not yet transmitted will be lost.
 *     DES ID: DES_UART_API_010
 */
Hal_StatusType Uart_Hal_AbortSendingData(uint8 Instance);

/**
 * @brief Receives data using non-blocking method (interrupt or DMA).
 *
 * @details
 * This function initiates a non-blocking reception of data into the specified
 * buffer through the UART interface. The transfer method (interrupt or DMA) is
 * determined by the configuration set during initialization. The function returns
 * immediately after starting the reception, and completion is signaled via callback.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[out] RxBuff: Pointer to the receive data buffer.
 * @param[in] RxSize: Number of bytes to receive.
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: Reception started successfully.
 *     STATUS_ERROR: Invalid parameters or UART not initialized.
 *     STATUS_BUSY: Previous reception still in progress.
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     RxBuff must point to a valid buffer with at least RxSize bytes.
 *     No other reception should be in progress on this instance.
 *
 * @post
 *     Reception is initiated and will complete asynchronously.
 *     Callback will be invoked upon completion or error.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     The RxBuff buffer must remain valid until the reception completes.
 *     DES ID: DES_UART_API_005
 */
Hal_StatusType Uart_Hal_ReceiveData(uint8 Instance, uint8 *RxBuff, uint32 RxSize);

#ifndef UART_SDK_NON_EXTENDED_API
/**
 * @brief Receives data using polling mode.
 *
 * @details
 * This function receives data into the specified buffer through the UART interface
 * using polling method. The function continuously polls the receive buffer status
 * and reads data byte by byte until the requested number of bytes is received.
 * This is a blocking operation that returns only when all data is received.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[out] RxBuff: Pointer to the receive data buffer.
 * @param[in] RxSize: Number of bytes to receive.
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: All data received successfully.
 *     STATUS_ERROR: Reception error or timeout occurred.
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     RxBuff must point to a valid buffer with at least RxSize bytes.
 *
 * @post
 *     The receive buffer is filled with received data.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     This function blocks until all requested data is received.
 *     Do not use this function in time-critical contexts or ISRs.
 *     There is no timeout mechanism; ensure data will be available.
 *     DES ID: DES_UART_API_006
 */
Hal_StatusType Uart_Hal_ReceiveDataPolling(uint8 Instance, uint8 *RxBuff, uint32 RxSize);

/**
 * @brief Receives data using blocking method with timeout.
 *
 * @details
 * This function receives data into the specified buffer through the UART interface
 * using a blocking method. The function waits until the entire reception is
 * complete or the specified timeout expires. It ensures all requested data bytes
 * are received before returning, unless a timeout or error occurs.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[out] RxBuff: Pointer to the receive data buffer.
 * @param[in] RxSize: Number of bytes to receive.
 * @param[in] TimeoutUs: Maximum time in microseconds to wait for reception completion.
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: All data received successfully.
 *     STATUS_TIMEOUT: Reception did not complete within the timeout period.
 *     STATUS_ERROR: Invalid parameters or reception error occurred.
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     RxBuff must point to a valid buffer with at least RxSize bytes.
 *
 * @post
 *     The receive buffer contains the received data up to the point of return.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     This function blocks the calling thread until reception completes or timeout.
 *     Do not call from ISR or time-critical contexts.
 *     The timeout is approximate and depends on system tick resolution.
 *     DES ID: DES_UART_API_007
 */
Hal_StatusType Uart_Hal_ReceiveDataBlocking(uint8 Instance, uint8 *RxBuff, uint32 RxSize, uint32 TimeoutUs);
#endif

/**
 * @brief Retrieves the current reception status.
 *
 * @details
 * This function queries the status of an ongoing non-blocking reception
 * and returns the number of bytes remaining to be received. It can be
 * used to monitor reception progress or verify completion.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[out] BytesRemaining: Pointer to store the number of bytes remaining to be received.
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: Reception completed, no bytes remaining.
 *     STATUS_BUSY: Reception in progress, BytesRemaining contains count.
 *     STATUS_ERROR: Invalid parameters or no reception was initiated.
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     BytesRemaining must point to a valid uint32 variable.
 *
 * @post
 *     The BytesRemaining variable is updated with current reception status.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This function is intended for monitoring non-blocking receptions.
 *     DES ID: DES_UART_API_009
 */
Hal_StatusType Uart_Hal_GetReceiveStatus(uint8 Instance, uint32 *BytesRemaining);

/**
 * @brief Terminates an ongoing non-blocking reception.
 *
 * @details
 * This function aborts an active non-blocking reception on the specified
 * UART instance. It stops any further data reception, disables related
 * interrupts or DMA channels, and resets the reception state. Any data
 * already received remains in the buffer, but remaining data is discarded.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: Reception aborted successfully.
 *     STATUS_ERROR: No active reception to abort or invalid instance.
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *
 * @post
 *     The reception is stopped and the UART is ready for new operations.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     Partially received data remains in the buffer.
 *     DES ID: DES_UART_API_011
 */
Hal_StatusType Uart_Hal_AbortReceivingData(uint8 Instance);

/**
 * @brief Configures the UART baud rate.
 *
 * @details
 * This function dynamically changes the baud rate of the specified UART instance.
 * It calculates and configures the necessary hardware register values based on
 * the desired baud rate and sample count. The UART should not have active
 * transmissions or receptions when calling this function.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] DesiredBaudRate: The desired baud rate in bits per second.
 * @param[in] SampleCnt: Sample count mode (16x, 8x, or 4x oversampling).
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: Baud rate configured successfully.
 *     STATUS_BUSY: UART is busy with active transmission or reception.
 *     STATUS_ERROR: Invalid parameters or configuration failed.
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     No active transmission or reception should be in progress.
 *
 * @post
 *     The UART baud rate is updated to the specified value.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     Changing baud rate during active communication may cause data corruption.
 *     The actual baud rate may differ slightly from the desired value due to clock divisor limitations.
 *     DES ID: DES_UART_API_012
 */
Hal_StatusType Uart_Hal_SetBaudRate(uint8 Instance, uint32 DesiredBaudRate, Uart_SampleCntType SampleCnt);

/**
 * @brief Enables or disables the UART idle line interrupt.
 *
 * @details
 * This function enables or disables the idle line detection interrupt for the
 * specified UART instance. When enabled, an interrupt is generated when the
 * receive line remains idle for a complete frame time. This is useful for
 * detecting the end of a variable-length message.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] IsEnable: TRUE to enable idle interrupt, FALSE to disable.
 *
 * @return void
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *
 * @post
 *     Idle line interrupt is enabled or disabled as specified.
 *     If enabled, callback will be invoked on idle line detection.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     The idle callback must be configured during initialization to receive events.
 *     Idle detection occurs after receiving at least one character.
 *     DES ID: DES_UART_API_019
 */
void Uart_Hal_SetIdleInterrupt(uint8 Instance, boolean IsEnable);

#ifndef UART_SDK_NON_EXTENDED_API
/**
 * @brief Retrieves the currently configured UART baud rate.
 *
 * @details
 * This function reads the hardware configuration registers and calculates
 * the currently configured baud rate for the specified UART instance.
 * The returned value represents the actual baud rate based on hardware settings.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[out] ConfiguredBaudRate: Pointer to store the configured baud rate in bits per second.
 *
 * @return void
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     ConfiguredBaudRate must point to a valid uint32 variable.
 *
 * @post
 *     The ConfiguredBaudRate variable contains the current baud rate.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     The returned value may differ slightly from the originally requested baud rate due to clock divisor limitations.
 *     DES ID: DES_UART_API_013
 */
void Uart_Hal_GetBaudRate(uint8 Instance, uint32 *ConfiguredBaudRate);

/**
 * @brief Configures the data match feature for wake-up function.
 *
 * @details
 * This function configures the UART data match feature typically used for wake-up.
 * When enabled (DTMTCH_EN set), the UART RX checks if received data matches the
 * specified data value (UART_DATA[8:0]). When a match occurs and the match interrupt
 * is enabled via Uart_Hal_SetMatchInterrupt(), an interrupt is generated. The interrupt
 * can be cleared by writing to UART_LSR1[DTMATCH]. This is useful for detecting
 * special wake-up characters or synchronization patterns.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] Data: The data value (or pattern) to match against received data.
 * @param[in] IsEnable: TRUE to enable data match detection, FALSE to disable.
 *
 * @return void
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *
 * @post
 *     Data match detection is enabled or disabled with the specified pattern.
 *     If enabled, callback will be invoked when the pattern is detected (if interrupt enabled).
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     The match interrupt must be enabled separately via Uart_Hal_SetMatchInterrupt().
 *     The data width depends on the configured character size (7, 8, or 9 bits).
 *     DES ID: DES_UART_API_016
 */
void Uart_Hal_SetDataMatch(uint8 Instance, uint16 Data, boolean IsEnable);

/**
 * @brief Configures the address filtering feature for selective data reception.
 *
 * @details
 * This function enables or disables address filtering for multi-drop communication.
 * When enabled (ADDTCH_EN set), the UART checks if received data matches the
 * specified address (UART_ADDR[8:0]). When an address matches, subsequent data bytes
 * will be received until the next address mismatch occurs.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] Addr: The device address to match for receiving data.
 * @param[in] IsEnable: TRUE to enable address filtering, FALSE to disable.
 *
 * @return void
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *
 * @post
 *     Address filtering is enabled or disabled with the specified address.
 *     If enabled, only data following matching addresses will be received.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     DES ID: DES_UART_API_015
 */
void Uart_Hal_SetAddrFilter(uint8 Instance, uint16 Addr, boolean IsEnable);

/**
 * @brief Enables or disables the data match interrupt.
 *
 * @details
 * This function enables or disables the interrupt generation when a data match
 * occurs. The data match pattern must be configured separately using
 * Uart_Hal_SetDataMatch(). When enabled, an interrupt is generated whenever
 * the received data matches the configured pattern.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] IsEnable: TRUE to enable data match interrupt, FALSE to disable.
 *
 * @return void
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     Data match pattern should be configured via Uart_Hal_SetDataMatch().
 *
 * @post
 *     Data match interrupt is enabled or disabled as specified.
 *     If enabled, callback will be invoked when a data match is detected.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     The match callback must be configured during initialization to receive events.
 *     This function only controls interrupt generation, not match detection itself.
 *     DES ID: DES_UART_API_020
 */
void Uart_Hal_SetMatchInterrupt(uint8 Instance, boolean IsEnable);

/**
 * @brief Configures the RTS/CTS hardware flow control.
 *
 * @details
 * This function enables or disables hardware flow control using RTS (Request to Send)
 * and CTS (Clear to Send) signals. Flow control prevents data loss by signaling
 * when the receiver is ready to accept more data. RTS is output by the receiver,
 * CTS is input to the transmitter.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] RtsCts: Flow control configuration (none, RTS only, CTS only, or both).
 *
 * @return void
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     RTS/CTS pins must be properly configured for UART alternate function.
 *
 * @post
 *     Hardware flow control is configured according to the specified mode.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     Both devices must support and enable flow control for it to work properly.
 *     RTS/CTS is typically used in point-to-point connections.
 *     DES ID: DES_UART_API_014
 */
void Uart_Hal_SetCtsRts(uint8 Instance, Uart_RtsCtsType RtsCts);

/**
 * @brief Configures RS485 mode settings.
 *
 * @details
 * This function configures the UART to operate in RS485 mode, which includes
 * automatic driver enable control, polarity settings, and timing parameters.
 * RS485 is a multi-drop bus standard that requires explicit transmitter enable
 * control to prevent bus contention.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] Config: Pointer to the RS485 configuration structure containing enable flags, polarity, delay, and guard time.
 *
 * @return void
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     Config must point to a valid RS485 configuration structure.
 *     The driver enable pin must be properly configured.
 *
 * @post
 *     RS485 mode is configured with the specified parameters.
 *     Transmitter enable is automatically controlled during transmission.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     Delay and guard time ensure proper bus settling and prevent collisions.
 *     Polarity inversion may be needed depending on transceiver hardware.
 *     DES ID: DES_UART_API_018
 */
void Uart_Hal_SetRS485(uint8 Instance, const Uart_RS485ConfigType *Config);

/**
 * @brief Configures IrDA (Infrared Data Association) mode settings.
 *
 * @details
 * This function configures the UART to operate in IrDA mode for infrared
 * communication. IrDA mode modulates the UART data using short pulses suitable
 * for IR transceivers. The pulse width determines the communication range and
 * power consumption.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] Config: Pointer to the IrDA configuration structure containing enable flag, mode, and pulse width settings.
 *
 * @return void
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     Config must point to a valid IrDA configuration structure.
 *     An IrDA transceiver must be connected to the UART pins.
 *
 * @post
 *     IrDA mode is configured with the specified parameters.
 *     UART signals are modulated for infrared transmission.
 *
 * @note
 *     Reentrancy: Reentrant for different instance; not reentrant for the same instance.
 *     Pulse width affects communication range: narrower pulses save power but reduce range.
 *     TxMode determines whether the UART is in transmit or receive mode.
 *     DES ID: DES_UART_API_017
 */
void Uart_Hal_SetIrDA(uint8 Instance, const Uart_IrDAConfigType *Config);
#endif

/**
 * @brief Sets the internal driver TX buffer pointer and size.
 *
 * @details
 * This function updates the internal TX buffer pointer and remaining size for
 * the specified UART instance. It is used to redirect an in-progress interrupt-
 * or DMA-based transmission to a new data source without re-initializing the
 * channel.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] TxBuff: Pointer to the new transmit data buffer.
 * @param[in] TxSize: Number of bytes in the buffer (must be greater than 0).
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: Buffer updated successfully.
 *     STATUS_UNSUPPORTED: Invalid parameters (NULL pointer or zero size).
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     TxBuff must point to a valid data buffer of at least TxSize bytes.
 *
 * @post
 *     Internal TX buffer pointer and size are updated.
 *
 * @note
 *     Reentrancy: Not reentrant for the same instance.
 *     DES ID: DES_UART_API_029
 */
Hal_StatusType Uart_Hal_SetTxBuffer(uint8 Instance, const uint8 *TxBuff, uint32 TxSize);

/**
 * @brief Sets the internal driver RX buffer pointer and size.
 *
 * @details
 * This function updates the internal RX buffer pointer and remaining size for
 * the specified UART instance. It is used to redirect an in-progress interrupt-
 * or DMA-based reception to a new storage location without re-initializing the
 * channel.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[out] RxBuff: Pointer to the new receive data buffer.
 * @param[in] RxSize: Number of bytes the buffer can hold (must be greater than 0).
 *
 * @return Hal_StatusType:
 *     STATUS_SUCCESS: Buffer updated successfully.
 *     STATUS_UNSUPPORTED: Invalid parameters (NULL pointer or zero size).
 *
 * @pre
 *     The UART instance must be initialized via Uart_Hal_Init().
 *     RxBuff must point to a valid buffer of at least RxSize bytes.
 *
 * @post
 *     Internal RX buffer pointer and size are updated.
 *
 * @note
 *     Reentrancy: Not reentrant for the same instance.
 *     DES ID: DES_UART_API_030
 */
Hal_StatusType Uart_Hal_SetRxBuffer(uint8 Instance, uint8 *RxBuff, uint32 RxSize);

#ifdef __cplusplus
}
#endif

#endif
/*============================================EOF===================================================*/
