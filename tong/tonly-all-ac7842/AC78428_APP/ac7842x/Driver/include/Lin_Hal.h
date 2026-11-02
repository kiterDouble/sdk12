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
* AutoChips Inc. (C) 2023. All rights reserved.
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

/**
* @file Lin_Hal.h
* @brief This file provides extern Hal lin api.
*/

#ifndef LIN_HAL_H
#define LIN_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Lin_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief Initialize a LIN channel.
 *
 * @details
 * This function initializes the specified LIN hardware channel with the provided
 * configuration parameters. It configures the baud rate, mode (master/slave),
 * break length/threshold, and enables the necessary interrupts for LIN communication.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] ChannelConfigPtr: Pointer to LIN channel configuration structure.
 *
 * @return void
 *
 * @pre
 *     The LIN module must not be initialized.
 *     ChannelConfigPtr must point to a valid configuration structure.
 *
 * @post
 *     The LIN channel is initialized and ready for communication.
 *     Hardware registers are configured according to the provided settings.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     This function must be called before any other LIN operations.
 *     DES ID: DES_LIN_API_000
 */
void Lin_Hal_Init
(
    uint8 Instance,
    const Lin_ChannelConfigType *ChannelConfigPtr
);

/**
 * @brief Deinitialize a LIN channel.
 *
 * @details
 * This function deinitializes the specified LIN hardware channel. It disables
 * the UART transmitter and receiver, disables interrupts, and resets the
 * channel to an uninitialized state.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return void
 *
 * @pre
 *     The LIN channel must be initialized.
 *
 * @post
 *     The LIN channel is deinitialized.
 *     Hardware is disabled and interrupts are cleared.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     DES ID: DES_LIN_API_001
 */
void Lin_Hal_DeInit
(
    uint8 Instance
);

/**
 * @brief Set LIN channel to sleep mode.
 *
 * @details
 * This function sets the specified LIN channel to sleep mode. In sleep mode,
 * the channel can detect wakeup signals. The function will return busy status
 * if the bus is currently busy.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return Hal_StatusType
 *     STATUS_SUCCESS: Sleep mode entered successfully.
 *     STATUS_BUSY: Channel is busy, cannot enter sleep mode.
 *
 * @pre
 *     The LIN channel must be initialized.
 *     The channel must not be in the middle of a transmission.
 *
 * @post
 *     If successful, the channel enters sleep mode with wakeup detection enabled.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     DES ID: DES_LIN_API_005
 */
Hal_StatusType Lin_Hal_GoToSleepMode
(
    uint8 Instance
);

/**
 * @brief Set LIN channel to idle state.
 *
 * @details
 * This function sets the specified LIN channel to idle state. It exits from
 * sleep mode, disables wakeup interrupts, and prepares the channel for normal
 * operation.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return void
 *
 * @pre
 *     The LIN channel must be initialized.
 *
 * @post
 *     The channel is in idle state and ready for communication.
 *     Wakeup interrupt is disabled.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     DES ID: DES_LIN_API_007
 */
void Lin_Hal_GoToIdleState
(
    uint8 Instance
);

/**
 * @brief Send LIN wakeup signal.
 *
 * @details
 * This function sends a wakeup signal on the LIN bus. The wakeup signal is
 * used to wake up nodes that are in sleep mode. The signal length depends
 * on the configured baud rate.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return Hal_StatusType
 *     STATUS_SUCCESS: Wakeup signal sent successfully.
 *     STATUS_BUSY: Channel is busy, cannot send wakeup signal.
 *
 * @pre
 *     The LIN channel must be initialized.
 *
 * @post
 *     A wakeup signal is transmitted on the bus.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     DES ID: DES_LIN_API_006
 */
Hal_StatusType Lin_Hal_SendWakeupSignal
(
    uint8 Instance
);

/**
 * @brief Process LIN identifier parity.
 *
 * @details
 * This function either computes the parity bits for a LIN identifier or
 * validates the parity bits of a received PID. The LIN PID consists of
 * a 6-bit identifier plus 2 parity bits.
 *
 * @param[in] Pid: Protected identifier or raw identifier.
 * @param[in] Type: Operation type:
 *                  - LIN_MAKE_PARITY: Compute parity bits and return complete PID.
 *                  - LIN_CHECK_PARITY: Validate parity bits.
 *
 * @return uint8
 *     For LIN_MAKE_PARITY: Complete PID with parity bits.
 *     For LIN_CHECK_PARITY: Raw identifier if parity is correct, 0xFF if parity error.
 *
 * @pre
 *     None.
 *
 * @post
 *     None.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This is a pure computation function with no side effects.
 *     DES ID: DES_LIN_API_003
 */
uint8 Lin_Hal_ProcessParity
(
    uint8 Pid,
    uint8 Type
);

/**
 * @brief Send LIN frame.
 *
 * @details
 * This function initiates the transmission of a LIN frame. It sends the header
 * (break, sync, PID) and manages the response based on the configuration. The
 * function validates the PID parity and data length before transmission.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] PduInfo: Pointer to PDU structure containing:
 *                     - Pid: Protected identifier.
 *                     - Cs: Checksum model (classic or enhanced).
 *                     - Drc: Data response configuration (TX/RX/IGNORE).
 *                     - Dl: Data length in bytes.
 *                     - SduPtr: Pointer to data buffer.
 *
 * @return Hal_StatusType
 *     STATUS_SUCCESS: Frame transmission initiated successfully.
 *     STATUS_ERROR: Invalid parameters (PID parity error, invalid data length, sleep mode).
 *     STATUS_BUSY: Channel is busy with another transmission.
 *
 * @pre
 *     The LIN channel must be initialized.
 *     The channel must not be in sleep mode.
 *     PduInfo must point to a valid structure.
 *
 * @post
 *     Frame transmission is initiated.
 *     Channel state is updated to reflect ongoing transmission/reception.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     DES ID: DES_LIN_API_002
 */
Hal_StatusType Lin_Hal_SendFrameData
(
    uint8 Instance,
    const Lin_PduType *PduInfo
);

/**
 * @brief Abort ongoing LIN transfer.
 *
 * @details
 * This function aborts the current LIN transmission or reception. It sets the
 * channel to idle state and clears the bus busy flag.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return void
 *
 * @pre
 *     The LIN channel must be initialized.
 *
 * @post
 *     The ongoing transfer is aborted.
 *     Channel is returned to idle state.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     DES ID: DES_LIN_API_004
 */
void Lin_Hal_AbortTransferData
(
    uint8 Instance
);

/**
 * @brief Get LIN channel status.
 *
 * @details
 * This function returns the current status of the specified LIN channel. It
 * provides information about ongoing operations, completion status, and errors.
 * If LinSduPtr is provided and reception is complete, the received data is
 * copied to the buffer.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[out] LinSduPtr: Pointer to buffer for storing received data (optional, can be NULL).
 *
 * @return Lin_StatusType
 *     LIN_NOT_OK: Error occurred.
 *     LIN_TX_OK: Transmission completed successfully.
 *     LIN_TX_BUSY: Transmission in progress.
 *     LIN_TX_HEADER_ERROR: Header transmission error.
 *     LIN_TX_ERROR: Data transmission error.
 *     LIN_RX_OK: Reception completed successfully.
 *     LIN_RX_BUSY: Reception in progress.
 *     LIN_RX_ERROR: Reception error.
 *     LIN_RX_NO_RESPONSE: No response received.
 *     LIN_OPERATIONAL: Channel is operational and idle.
 *     LIN_CH_SLEEP: Channel is in sleep mode.
 *
 * @pre
 *     The LIN channel must be initialized.
 *
 * @post
 *     If reception is complete and LinSduPtr is valid, received data is copied.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     DES ID: DES_LIN_API_008
 */
Lin_StatusType Lin_Hal_GetStatus
(
    uint8 Instance,
    uint8 *LinSduPtr
);

#ifndef LIN_SDK_NON_EXTENDED_API
/**
 * @brief Set timeout counter for LIN channel.
 *
 * @details
 * This function sets the timeout counter value for the specified LIN channel.
 * The counter is decremented by the timeout service function and used to detect
 * transmission/reception timeout conditions.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] Timeout: Timeout counter value.
 *
 * @return void
 *
 * @pre
 *     The LIN channel must be initialized.
 *
 * @post
 *     Timeout counter is set to the specified value.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     This function is typically called before starting a frame transmission.
 *     DES ID: DES_LIN_API_017
 */
void Lin_Hal_SetTimeoutCounter
(
    uint8 Instance,
    uint32 Timeout
);

/**
 * @brief Timeout service function for LIN channel.
 *
 * @details
 * This function should be called periodically (typically from a timer interrupt)
 * to manage timeout detection for LIN transmissions and receptions. It decrements
 * the timeout counter and triggers timeout events when the counter reaches zero.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return void
 *
 * @pre
 *     The LIN channel must be initialized.
 *     This function should be called periodically at a fixed rate.
 *
 * @post
 *     If timeout occurs, a timeout event is generated and the callback is invoked.
 *     Channel returns to idle state on timeout.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     This function is typically called from timer interrupt context.
 *     DES ID: DES_LIN_API_018
 */
void Lin_Hal_TimeoutService
(
    uint8 Instance
);
#endif

#ifdef __cplusplus
}
#endif
#endif
/*============================================EOF===================================================*/
