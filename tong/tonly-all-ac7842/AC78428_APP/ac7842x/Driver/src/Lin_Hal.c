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
* @file Lin_Hal.c
* @brief This file provides Hal lin api.
*/
/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Uart_Reg.h"
#include "Lin_Hal.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/
/**
 * @brief Table of base addresses for LIN instances.
 * @note DES ID: DES_LIN_VAR_001
 */
static UART_Type *const Lin_HalBase[UART_INSTANCE_MAX] =
{
    UART0, UART1, UART2,
#if !defined(AC7840E)
    UART3,
#endif
#if defined (AC7843X)
    UART4, UART5, UART6, UART7
#endif
};

/*PRQA S 3218 ++ # AMCE_LIN_3218 */
/**
 * @brief Table of UART IRQ numbers for LIN instances.
 * @note DES ID: DES_LIN_VAR_002
 */
static const IRQn_Type Lin_IrqId[UART_INSTANCE_MAX] = UART_IRQS;

/**
 * @brief Table of clock IDs for LIN instances.
 * @note DES ID: DES_LIN_VAR_003
 */
static const Ckgen_ClkIdType Lin_HalClock[UART_INSTANCE_MAX] =
{
    CKGEN_UART0_CLK, CKGEN_UART1_CLK, CKGEN_UART2_CLK,
#if !defined(AC7840E)
    CKGEN_UART3_CLK,
#endif
#if defined (AC7843X)
    CKGEN_UART4_CLK, CKGEN_UART5_CLK, CKGEN_UART6_CLK, CKGEN_UART7_CLK
#endif
};

/**
 * @brief Table of bus clock IDs for LIN instances.
 * @note DES ID: DES_LIN_VAR_004
 */
static const Ckgen_BusClkIdType Lin_HalBusClock[UART_INSTANCE_MAX] =
{
    CKGEN_UART0_BUS_CLK, CKGEN_UART1_BUS_CLK, CKGEN_UART2_BUS_CLK,
#if !defined(AC7840E)
    CKGEN_UART3_BUS_CLK,
#endif
#if defined (AC7843X)
    CKGEN_UART4_BUS_CLK, CKGEN_UART5_BUS_CLK, CKGEN_UART6_BUS_CLK, CKGEN_UART7_BUS_CLK
#endif
};

/**
 * @brief Table of reset IDs for LIN instances.
 * @note DES ID: DES_LIN_VAR_005
 */
static const Rcm_ResetIDType Lin_HalClockReset[UART_INSTANCE_MAX] =
{
    RCM_RESET_ID_UART0, RCM_RESET_ID_UART1, RCM_RESET_ID_UART2,
#if !defined(AC7840E)
    RCM_RESET_ID_UART3,
#endif
#if defined (AC7843X)
    RCM_RESET_ID_UART4, RCM_RESET_ID_UART5, RCM_RESET_ID_UART6, RCM_RESET_ID_UART7
#endif
};
/*PRQA S 3218 -- */
/*============================================DEFINES AND MACROS====================================*/
/**
 * @def LIN_SMP_CNT16
 * @brief LIN sample count based on 16 times baud pulse.
 * @note DES ID: DES_LIN_MACRO_001
 */
#define LIN_SMP_CNT16                  (0U)

/**
 * @def LIN_SAMPLE_CNT_16_VALUE
 * @brief LIN sample count value of 16.
 * @note DES ID: DES_LIN_MACRO_002
 */
#define LIN_SAMPLE_CNT_16_VALUE         (16UL)

/**
 * @def LIN_MIN_CHANNEL_BAUDRATE
 * @brief Minimum allowed baud rate for LIN channel.
 * @note DES ID: DES_LIN_MACRO_003
 */
#define LIN_MIN_CHANNEL_BAUDRATE        (1000U)
/**
 * @def LIN_MAX_CHANNEL_BAUDRATE
 * @brief Maximum allowed baud rate for LIN channel.
 * @note DES ID: DES_LIN_MACRO_004
 */
#define LIN_MAX_CHANNEL_BAUDRATE        (20000U)

/**
 * @def LIN_SYNC_DATA
 * @brief LIN synchronization byte value.
 * @note DES ID: DES_LIN_MACRO_005
 */
#define LIN_SYNC_DATA                   (0x55UL)

/**
 * @def LIN_DATA_LENGTH_8
 * @brief Maximum LIN data field length in bytes.
 * @note DES ID: DES_LIN_MACRO_006
 */
#define LIN_DATA_LENGTH_8               ((uint8)8U)
/**
 * @def LIN_ID_MASK
 * @brief Mask for extracting LIN frame ID (6 bits).
 * @note DES ID: DES_LIN_MACRO_007
 */
#define LIN_ID_MASK                     (0x3FU)

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/**
 * @brief LIN channel current state structure.
 * @note DES ID: DES_LIN_TYP_001
 */
typedef struct
{
    const uint8 *TxBuff; /*!< The buffer of transmitted data */
    uint8 CntByte; /*!< The count size of bytes already transmitted or received */
    uint8 TxSize; /*!< Byte size need to be transmitted */
    uint8 RxSize; /*!< Byte size need to be received */
    uint8 CheckSum; /*!< Checksum byte */
    boolean IsBusBusy; /*!< Bus busy state */
    volatile uint32 TimeoutCounter; /*!< Timeout counter value */
    float32 Divisor; /*!< Baudrate divisor value for UART hardware configuration */
    Lin_NodeStateType CurrentNodeState; /*!< Current node state */
    Lin_NodeStateType PreviousNodeState; /*!< Store previous node state when set Lin channel to idle for further processing */
    Lin_CallbackType Callback; /*!< Callback function */
    Lin_ChannelInfoType ChannelInfo; /*!< LIN channeal information */
} Lin_ChannelStateType;

/*===========================================VARIABLE DECLARATIONS==================================*/
/**
 * @brief LIN channel 0 state structure instance.
 * @note DES ID: DES_LIN_VAR_006
 */
#if (CONFIG_LIN0_ENABLE)
static Lin_ChannelStateType Lin0_ChannelState;
#endif
/**
 * @brief LIN channel 1 state structure instance.
 * @note DES ID: DES_LIN_VAR_007
 */
#if (CONFIG_LIN1_ENABLE)
static Lin_ChannelStateType Lin1_ChannelState;
#endif
/**
 * @brief LIN channel 2 state structure instance.
 * @note DES ID: DES_LIN_VAR_008
 */
#if (CONFIG_LIN2_ENABLE)
static Lin_ChannelStateType Lin2_ChannelState;
#endif

#if !defined(AC7840E)
/**
 * @brief LIN channel 3 state structure instance.
 * @note DES ID: DES_LIN_VAR_009
 */
#if (CONFIG_LIN3_ENABLE)
static Lin_ChannelStateType Lin3_ChannelState;
#endif
#endif

#if defined (AC7843X)
/**
 * @brief LIN channel 4 state structure instance.
 * @note DES ID: DES_LIN_VAR_010
 */
#if (CONFIG_LIN4_ENABLE)
static Lin_ChannelStateType Lin4_ChannelState;
#endif
/**
 * @brief LIN channel 5 state structure instance.
 * @note DES ID: DES_LIN_VAR_011
 */
#if (CONFIG_LIN5_ENABLE)
static Lin_ChannelStateType Lin5_ChannelState;
#endif
/**
 * @brief LIN channel 6 state structure instance.
 * @note DES ID: DES_LIN_VAR_012
 */
#if (CONFIG_LIN6_ENABLE)
static Lin_ChannelStateType Lin6_ChannelState;
#endif
/**
 * @brief LIN channel 7 state structure instance.
 * @note DES ID: DES_LIN_VAR_013
 */
#if (CONFIG_LIN7_ENABLE)
static Lin_ChannelStateType Lin7_ChannelState;
#endif
#endif

/**
 * @brief Array of pointers to channel state structures for each LIN instance.
 * @note DES ID: DES_LIN_VAR_014
 */
static Lin_ChannelStateType *const Lin_ChannelStateArray[UART_INSTANCE_MAX] =
{
#if (CONFIG_LIN0_ENABLE)
    &Lin0_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_LIN1_ENABLE)
    &Lin1_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_LIN2_ENABLE)
    &Lin2_ChannelState,
#else
    NULL_PTR,
#endif
#if !defined(AC7840E)
#if (CONFIG_LIN3_ENABLE)
    &Lin3_ChannelState,
#else
    NULL_PTR,
#endif
#endif
#if defined (AC7843X)
#if (CONFIG_LIN4_ENABLE)
    &Lin4_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_LIN5_ENABLE)
    &Lin5_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_LIN6_ENABLE)
    &Lin6_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_LIN7_ENABLE)
    &Lin7_ChannelState,
#else
    NULL_PTR,
#endif
#endif
};

/**
 * @brief Array of pointers to channel configuration for each LIN instance.
 * @note DES ID: DES_LIN_VAR_015
 */
static const Lin_ChannelConfigType *Lin_ChannelConfigPtr[UART_INSTANCE_MAX];

/**
 * @brief Buffer for storing LIN frame data for each channel.
 * @note DES ID: DES_LIN_VAR_016
 */
static uint8 Lin_SduBuffer[UART_INSTANCE_MAX][LIN_DATA_LENGTH_8];

/**
 * @brief Array of wakeup signal values for each LIN instance.
 * @note DES ID: DES_LIN_VAR_017
 */
static uint8 Lin_WakeupSignal[UART_INSTANCE_MAX];

/*============================================FUNCTION PROTOTYPES===================================*/
#if (CONFIG_LIN0_ENABLE)
ISR(UART0_IRQHandler);
#endif

#if (CONFIG_LIN1_ENABLE)
ISR(UART1_IRQHandler);
#endif

#if (CONFIG_LIN2_ENABLE)
ISR(UART2_IRQHandler);
#endif

#if !defined(AC7840E)
#if (CONFIG_LIN3_ENABLE)
ISR(UART3_IRQHandler);
#endif
#endif

#if defined (AC7843X)
#if (CONFIG_LIN4_ENABLE)
ISR(UART4_IRQHandler);
#endif

#if (CONFIG_LIN5_ENABLE)
ISR(UART5_IRQHandler);
#endif

#if (CONFIG_LIN6_ENABLE)
ISR(UART6_IRQHandler);
#endif

#if (CONFIG_LIN7_ENABLE)
ISR(UART7_IRQHandler);
#endif
#endif

/**
 * @brief Get LIN channel status when no event has occurred.
 *
 * @details
 * This function determines the channel status based on the current node state
 * when no specific event is active. It handles states such as transmission busy,
 * sleep mode, idle, and operational states.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return Lin_StatusType: Current channel status
 *     - LIN_TX_BUSY: Channel is busy transmitting break/sync/PID
 *     - LIN_CH_SLEEP: Channel is in sleep mode
 *     - LIN_OPERATIONAL: Channel is operational and idle
 *     - LIN_NOT_OK: Status could not be determined
 *
 * @pre
 *     The LIN driver must be initialized.
 *     Instance must be a valid channel number.
 *
 * @post
 *     Channel status is returned without modifying state.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     DES ID: DES_LIN_API_051
 */
static Lin_StatusType Lin_Hal_GetStatusFromNoEvent
(
    uint8 Instance
);

/**
 * @brief Get LIN channel status after successful PID transmission.
 *
 * @details
 * This function determines the channel status when the PID has been successfully
 * transmitted. It checks whether the channel is sending or receiving data and
 * returns the appropriate status.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return Lin_StatusType: Current channel status
 *     - LIN_TX_BUSY: Channel is busy transmitting data
 *     - LIN_RX_BUSY: Channel is busy receiving data
 *     - LIN_RX_NO_RESPONSE: No response received yet
 *     - LIN_NOT_OK: Status could not be determined
 *
 * @pre
 *     The LIN driver must be initialized.
 *     PID transmission must have completed successfully.
 *
 * @post
 *     Channel status is returned without modifying state.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     DES ID: DES_LIN_API_052
 */
static Lin_StatusType Lin_Hal_GetStatusFromPidOk
(
    uint8 Instance
);

/**
 * @brief Get LIN channel status when a timeout error has occurred.
 *
 * @details
 * This function determines the channel status when a timeout error event is detected.
 * It checks the previous node state to determine if the timeout occurred during
 * transmission or reception and returns the appropriate error status.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return Lin_StatusType: Current channel status
 *     - LIN_RX_NO_RESPONSE: No response received (timeout with zero bytes)
 *     - LIN_RX_ERROR: Reception error (timeout during reception)
 *     - LIN_TX_ERROR: Transmission error (timeout during transmission)
 *     - LIN_NOT_OK: Status could not be determined
 *
 * @pre
 *     The LIN driver must be initialized.
 *     A timeout error event must have occurred.
 *
 * @post
 *     Channel status is returned without modifying state.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     DES ID: DES_LIN_API_067
 */
static Lin_StatusType Lin_Hal_GetStatusFromTimeoutError
(
    uint8 Instance
);

/**
 * @brief Check and process LIN wakeup signal flag.
 *
 * @details
 * This function checks the wakeup flag in the LSR1 register and processes
 * wakeup events. If a wakeup signal is detected, it sets the appropriate
 * event ID and invokes the callback function if registered.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] Lsr1: LSR1 register value containing wakeup flag.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     LSR1 register value must be valid.
 *
 * @post
 *     If wakeup signal is detected, channel event is updated and callback is invoked.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     DES ID: DES_LIN_API_053
 */
static void Lin_Hal_CheckWakeupFlag
(
    uint8 Instance,
    uint32 Lsr1
);

/**
 * @brief Check and process LIN error flags.
 *
 * @details
 * This function checks error flags in LSR0 and LSR1 registers and processes
 * various error conditions including framing errors, overrun errors, noise errors,
 * and synchronization errors. It sets the appropriate event ID for each error type.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] Lsr0: LSR0 register value containing error flags.
 * @param[in] Lsr1: LSR1 register value containing additional error flags.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     Register values must be valid.
 *
 * @post
 *     If error is detected, channel event is updated accordingly.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     DES ID: DES_LIN_API_054
 */
static void Lin_Hal_CheckErrorFlag
(
    uint8 Instance,
    uint32 Lsr0,
    uint32 Lsr1
);

/**
 * @brief Configure LIN channel baud rate.
 *
 * @details
 * This function calculates and sets the baud rate for the LIN channel by
 * configuring the UART divider registers. It uses the peripheral clock frequency
 * and sample count to determine the appropriate divider value.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] BaudRate: Desired baud rate in bps.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     Peripheral clock must be enabled and stable.
 *
 * @post
 *     UART baud rate is configured to the specified value.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     DES ID: DES_LIN_API_055
 */
static void Lin_Hal_SetBaudRate
(
    uint8 Instance,
    uint32 BaudRate
);

/**
 * @brief Copy received LIN frame data to user buffer.
 *
 * @details
 * This function copies the received LIN frame data from the internal SDU buffer
 * to the user-provided buffer. It copies the number of bytes specified by the
 * frame data length.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[out] LinSduPtr: Pointer to buffer for storing received data.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     LinSduPtr must point to a valid buffer with sufficient size.
 *     Data reception must be complete.
 *
 * @post
 *     Received data is copied to the user buffer.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     DES ID: DES_LIN_API_056
 */
static void Lin_Hal_CopyData
(
    uint8 Instance,
    uint8 *LinSduPtr
);

/**
 * @brief Process LIN break field detection.
 *
 * @details
 * This function handles the detection of a LIN break field. For slave nodes,
 * it prepares the channel for frame reception by setting the appropriate state
 * and resetting counters. For master nodes, it processes break detection
 * during transmission.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     Channel must be configured as master or slave.
 *
 * @post
 *     Channel state is updated to receive PID or process transmission.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     DES ID: DES_LIN_API_057
 */
static void Lin_Hal_ProcessBreakDetect
(
    uint8 Instance
);

/**
 * @brief Process received LIN PID (Protected Identifier).
 *
 * @details
 * This function processes the received PID byte, validates parity bits,
 * and determines the frame direction (transmit or receive). It updates
 * the channel state and prepares for data phase based on the PID.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] RxByte: Received PID byte.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     Break and sync fields must have been received.
 *
 * @post
 *     Channel state is updated for data transmission or reception.
 *     PID OK event is set if validation succeeds.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     DES ID: DES_LIN_API_058
 */
static void Lin_Hal_ProcessReceivePid
(
    uint8 Instance,
    uint8 RxByte
);

/**
 * @brief Process LIN frame header bytes.
 *
 * @details
 * This function processes the header phase of a LIN frame, including
 * break detection, sync byte validation, and PID reception. It manages
 * state transitions during the header phase and handles auto-baud detection
 * if configured.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] RxByte: Received byte during header phase.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     Channel must be in header reception state.
 *
 * @post
 *     Channel state is updated based on received header bytes.
 *     Auto-baud may be adjusted if enabled.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     DES ID: DES_LIN_API_059
 */
static void Lin_Hal_ProcessFrameHeader
(
    uint8 Instance,
    uint8 RxByte
);

/**
 * @brief Calculate LIN frame checksum byte.
 *
 * @details
 * This function calculates the checksum for a LIN frame using either
 * classic or enhanced checksum model. For enhanced checksum, the PID
 * is included in the calculation. The checksum is the inverted sum of
 * all data bytes (and PID for enhanced).
 *
 * @param[in] BuffPtr: Pointer to data buffer.
 * @param[in] Size: Number of data bytes.
 * @param[in] Pid: Protected identifier (used for enhanced checksum).
 *
 * @return uint8: Calculated checksum byte.
 *
 * @pre
 *     BuffPtr must point to a valid buffer.
 *     Size must not exceed maximum frame length.
 *
 * @post
 *     Checksum value is calculated and returned.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     DES ID: DES_LIN_API_060
 */
static uint8 Lin_Hal_MakeCheckSumByte
(
    const uint8 *BuffPtr,
    uint8 Size,
    uint8 Pid
);

/**
 * @brief Process received LIN frame data bytes.
 *
 * @details
 * This function handles the reception of data bytes during the data phase
 * of a LIN frame. It stores received bytes in the buffer, validates the
 * checksum, and updates the channel state upon completion.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] RxByte: Received data byte.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     Channel must be in data reception state.
 *
 * @post
 *     Received byte is stored in buffer.
 *     On completion, checksum is validated and reception event is set.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     DES ID: DES_LIN_API_061
 */
static void Lin_Hal_ProcessReceiveFrameData
(
    uint8 Instance,
    uint8 RxByte
);

/**
 * @brief Process transmitted LIN frame data bytes.
 *
 * @details
 * This function handles the transmission of data bytes during the data phase
 * of a LIN frame. It reads back transmitted bytes for verification, transmits
 * the next byte or checksum, and manages transmission completion.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] RxByte: Received echo byte for readback verification.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     Channel must be in data transmission state.
 *
 * @post
 *     Next data byte or checksum is transmitted.
 *     On completion, transmission event is set.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     DES ID: DES_LIN_API_062
 */
static void Lin_Hal_ProcessSendFrameData
(
    uint8 Instance,
    uint8 RxByte
);

/**
 * @brief Process LIN frame transmission and reception.
 *
 * @details
 * This function is the main frame processing handler that manages both
 * transmission and reception of LIN frames. It processes header and data
 * phases based on the current channel state and handles state transitions.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     Channel must be in an active transmission or reception state.
 *
 * @post
 *     Frame processing advances to the next state or completes.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     Called from interrupt context.
 *     DES ID: DES_LIN_API_063
 */
static void Lin_Hal_ProcessFrame
(
    uint8 Instance
);

/**
 * @brief Initiate LIN frame transmission.
 *
 * @details
 * This function starts the transmission of a LIN frame by preparing the
 * channel state, calculating the checksum, and initiating break field
 * transmission. It configures the frame parameters including PID, data
 * length, and checksum model.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] PduInfo: Pointer to PDU structure containing:
 *                      - Pid: Protected identifier
 *                      - Cs: Checksum model (classic or enhanced)
 *                      - Drc: Data response configuration
 *                      - Dl: Data length
 *                      - SduPtr: Pointer to data buffer
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     PduInfo must be valid with correct parameters.
 *     Channel must be in idle state.
 *
 * @post
 *     Frame transmission is initiated with break field.
 *     Channel state is updated to transmission mode.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     DES ID: DES_LIN_API_064
 */
static void Lin_Hal_StartSendFrame
(
    uint8 Instance,
    const Lin_PduType *PduInfo
);

/**
 * @brief Initialize LIN channel state structure.
 *
 * @details
 * This function initializes the channel state structure with default values
 * and registers the callback function. It prepares the channel for operation
 * by setting initial node state and clearing all counters and flags.
 *
 * @param[in] Instance: LIN module instance number.
 * @param[in] ChannelConfigPtr: Pointer to channel configuration.
 *
 * @return void
 *
 * @pre
 *     ChannelConfigPtr must be valid.
 *     Channel state structure must be allocated.
 *
 * @post
 *     Channel state is initialized and ready for operation.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     DES ID: DES_LIN_API_065
 */
static void Lin_Hal_StateInit
(
    uint8 Instance,
    const Lin_ChannelConfigType *ChannelConfigPtr
);

/**
 * @brief LIN module interrupt service routine.
 *
 * @details
 * This function is the main interrupt handler for LIN communication. It reads
 * the interrupt status registers, checks for wakeup signals and error flags,
 * processes received data bytes, and manages frame transmission and reception.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     Interrupts must be enabled.
 *
 * @post
 *     Interrupt flags are cleared.
 *     Frame processing is advanced or completed.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     Called from interrupt context.
 *     DES ID: DES_LIN_API_066
 */
static void Lin_Hal_IRQHandler
(
    uint8 Instance
);

/**
 * @brief Check if the calculated baudrate is within acceptable tolerance of the configured baudrate.
 *
 * @details
 * This function verifies if the actual baudrate divisor is within +-15% tolerance
 * of the configured baudrate divisor. If the error exceeds the tolerance,
 * it reconfigures the baudrate to match the desired value.
 *
 * @param[in] Instance: LIN module instance number.
 *
 * @return void
 *
 * @pre
 *     The LIN driver must be initialized.
 *     Instance must be valid and within range.
 *
 * @post
 *     If the calculated baudrate is outside the tolerance range, it will be corrected.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     Called from interrupt context.
 *     DES ID: DES_LIN_API_067
 */
static void Lin_Hal_CheckBaudrate(uint8 Instance);
/*===============================GLOBAL FUNCTION  IMPLEMENTATIONS=================================*/
/* Implementation of Lin_Hal_Init declared in Lin_Hal.h */
void Lin_Hal_Init
(
    uint8 Instance,
    const Lin_ChannelConfigType *ChannelConfigPtr
)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelConfigPtr != NULL_PTR);
    /*PRQA S 2842 ++ # AMCE_LIN_2842.*/
    DEVICE_ASSERT(Lin_ChannelStateArray[Instance] != NULL_PTR);

    /* LINCR reg data */
    uint32 Ctrl;
    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];

    /* Check if current instance is already initialized. */
    DEVICE_ASSERT(LIN_NODE_STATE_UNINIT == Lin_ChannelStateArray[Instance]->CurrentNodeState);

    /* init ChannelConfigPtr */
    Lin_ChannelConfigPtr[Instance] = ChannelConfigPtr;

    /* enable uart clock */
    (void)Ckgen_Hal_EnablePeriphClk(Lin_HalBusClock[Instance], TRUE);
    Rcm_Hal_SetResetState(Lin_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(Lin_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);

    Lin_Hal_StateInit(Instance, ChannelConfigPtr);

    /*PRQA S 2812 ++ # AMCE_LIN_2812.*/
    /* Configure UART */
    Lin_Hal_SetBaudRate(Instance, ChannelConfigPtr->BaudRate);
    /*PRQA S 2812 -- # AMCE_LIN_2812.*/
    Uart_Reg_SetBitCountPerChar(Base, UART_8_BITS_PER_CHAR);
    Uart_Reg_SetParityMode(Base, UART_PARITY_DISABLED);
    Uart_Reg_SetStopBitCount(Base, UART_ONE_STOP_BIT);
    Uart_Reg_SetFIFO(Base, TRUE);
    Uart_Reg_SetTransmitterCmd(Base, TRUE);
    Uart_Reg_SetReceiverCmd(Base, TRUE);

    /* baud rate set more than 10000U */
    if (ChannelConfigPtr->BaudRate > 10000U)
    {
        Lin_WakeupSignal[Instance] = 0x80U;
    }
    else
    {
        Lin_WakeupSignal[Instance] = 0xF8U;
    }

    /* Configure master node */
    if (LIN_MASTER == ChannelConfigPtr->ModeType)
    {
        Ctrl = UART_LINCR_LINEN_Msk | UART_LINCR_LBRKDL_Msk;
        Uart_Reg_SetLinBreakLength(Base, (uint32)ChannelConfigPtr->BreakLength);
    }
    else
    {
        Ctrl = (uint32)UART_LINCR_LINEN_Msk;
        /* 11 bit transmit */
        if (BREAK_THRESHOLD_11BIT == ChannelConfigPtr->BreakThreshold)
        {
            Ctrl |= UART_LINCR_LBRKDL_Msk;
        }
        /* support sync */
        if (TRUE == ChannelConfigPtr->AutoBaudEnable)
        {
            Ctrl |= UART_LINCR_LABAUDEN_Msk;
        }
    }
    Uart_Reg_SetLinCtrl(Base, Ctrl);

    /* Initialize interrupt */
    if (TRUE == ChannelConfigPtr->AutoBaudEnable)
    {
        Uart_Reg_SetIntMode(Base, UART_INT_LIN_SYNC_ERR, TRUE);
    }

    Uart_Reg_SetIntMode(Base, UART_INT_LIN_BREAK, TRUE);
    Uart_Reg_SetIntMode(Base, UART_INT_RX_NOT_EMPTY, TRUE);
    Uart_Reg_SetIntMode(Base, UART_INT_FRAME_ERR, TRUE);
    Uart_Reg_SetIntMode(Base, UART_INT_RX_OVERRUN, TRUE);
    Uart_Reg_SetIntMode(Base, UART_INT_NOISE_ERR, TRUE);

    Core_Hal_EnableIrq(Lin_IrqId[Instance]);
    /*PRQA S 2842 -- # AMCE_LIN_2842.*/
}

/* Implementation of Lin_Hal_DeInit declared in Lin_Hal.h */
void Lin_Hal_DeInit
(
    uint8 Instance
)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_LIN_2842.*/
    DEVICE_ASSERT(Lin_ChannelStateArray[Instance] != NULL_PTR);

    UART_Type *Base = Lin_HalBase[Instance];
    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];

    /* Check if current instance is already de-initialized.*/
    DEVICE_ASSERT(LIN_NODE_STATE_UNINIT != ChannelState->CurrentNodeState);

    /* Disable UART transmitter and receiver */
    Uart_Reg_SetTransmitterCmd(Base, FALSE);
    Uart_Reg_SetReceiverCmd(Base, FALSE);

    /* Disable UART NVIC interrupt. */
    Core_Hal_DisableIrq(Lin_IrqId[Instance]);

    /* disable uart clock */
    Rcm_Hal_SetResetState(Lin_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(Lin_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    (void)Ckgen_Hal_EnablePeriphClk(Lin_HalBusClock[Instance], FALSE);

    Core_Hal_ClearPendingIrq(Lin_IrqId[Instance]);
    /*PRQA S 2842 -- # AMCE_LIN_2842.*/

    ChannelState->IsBusBusy = FALSE;
    ChannelState->ChannelInfo.CurrentEventId = LIN_NO_EVENT;
    ChannelState->CurrentNodeState = LIN_NODE_STATE_UNINIT;
}

/* Implementation of Lin_Hal_GoToSleepMode declared in Lin_Hal.h */
Hal_StatusType Lin_Hal_GoToSleepMode
(
    uint8 Instance
)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_LIN_2842.*/
    DEVICE_ASSERT(Lin_ChannelStateArray[Instance] != NULL_PTR);

    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];
    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # AMCE_LIN_2842.*/
    Hal_StatusType RetVal = STATUS_SUCCESS;

    /* Check if current instance is already initialized.*/
    DEVICE_ASSERT(LIN_NODE_STATE_UNINIT != ChannelState->CurrentNodeState);

    if (TRUE == ChannelState->IsBusBusy)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        if (LIN_NODE_STATE_SLEEP_MODE != ChannelState->CurrentNodeState)
        {
            /* ENABLE LIN wakeup interrupt */
            Uart_Reg_SetIntMode(Base, UART_INT_LIN_WAKEUP, TRUE);
            /* Set LIN to be sleep mode */
            Uart_Reg_SetLinSleep(Base, TRUE);

            ChannelState->CurrentNodeState = LIN_NODE_STATE_SLEEP_MODE;
            ChannelState->ChannelInfo.CurrentEventId = LIN_NO_EVENT;
            ChannelState->IsBusBusy = FALSE;
        }
    }

    return RetVal;
}

/* Implementation of Lin_Hal_GoToIdleState declared in Lin_Hal.h */
void Lin_Hal_GoToIdleState
(
    uint8 Instance
)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_LIN_2842.*/
    DEVICE_ASSERT(Lin_ChannelStateArray[Instance] != NULL_PTR);

    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];
    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # AMCE_LIN_2842.*/

    /* Check if current instance is already initialized.*/
    DEVICE_ASSERT(LIN_NODE_STATE_UNINIT != ChannelState->CurrentNodeState);

    /* DISABLE LIN wakeup interrupt */
    Uart_Reg_SetIntMode(Base, UART_INT_LIN_WAKEUP, FALSE);
    /* Exit from sleep mode */
    Uart_Reg_SetLinSleep(Base, FALSE);

    /* Restored current state to get status */
    ChannelState->PreviousNodeState = ChannelState->CurrentNodeState;
    /* Init Lin_State config */
    ChannelState->CurrentNodeState = LIN_NODE_STATE_IDLE;
    ChannelState->IsBusBusy = FALSE;
}

/* Implementation of Lin_Hal_SendWakeupSignal declared in Lin_Hal.h */
Hal_StatusType Lin_Hal_SendWakeupSignal
(
    uint8 Instance
)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_LIN_2842.*/
    DEVICE_ASSERT(Lin_ChannelStateArray[Instance] != NULL_PTR);

    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];
    Lin_ChannelStateType const *ChannelState = Lin_ChannelStateArray[Instance];
    /* api return */
    Hal_StatusType RetVal = STATUS_SUCCESS;

    /* Check if current instance is already initialized.*/
    DEVICE_ASSERT(LIN_NODE_STATE_UNINIT != ChannelState->CurrentNodeState);

    /* state not bus busy */
    if (FALSE == ChannelState->IsBusBusy)
    {
        /* Send LIN wakeup signal */
        Uart_Reg_PutChar(Base, Lin_WakeupSignal[Instance]);
        /*PRQA S 2842 -- # AMCE_LIN_2842.*/
    }
    else
    {
        RetVal = STATUS_BUSY;
    }

    return RetVal;
}

/* Implementation of Lin_Hal_ProcessParity declared in Lin_Hal.h */
uint8 Lin_Hal_ProcessParity
(
    uint8 Pid,
    uint8 Type
)
{
    /* lin slave mode id */
    uint8 Id = (Pid & 0x3FU);
    /* id to pid */
    uint8 ByteNum;
    /* return value */
    uint8 RetValue;

    /* Calculate the two parity bits */
    ByteNum = Id | (((uint8)(~((Pid >> 1U) ^ (Pid >> 3U) ^ (Pid >> 4U) ^ (Pid >> 5U))) << 7U) & 0x80U) | \
              (((Pid ^ (Pid >> 1U) ^ (Pid >> 2U) ^ (Pid >> 4U)) << 6U) & 0x40U);
    /* type is check parity */
    if (LIN_CHECK_PARITY == Type)
    {
        /* pid not byteNum */
        if (Pid != ByteNum)
        {
            RetValue = 0xFFU;
        }
        else
        {
            RetValue = Id;
        }
    }
    else
    {
        RetValue = ByteNum;
    }

    return RetValue;
}

/* Implementation of Lin_Hal_SendFrameData declared in Lin_Hal.h */
Hal_StatusType Lin_Hal_SendFrameData
(
    uint8 Instance,
    const Lin_PduType *PduInfo
)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(PduInfo != NULL_PTR);
    /*PRQA S 2842 ++ # AMCE_LIN_2842.*/
    DEVICE_ASSERT(Lin_ChannelStateArray[Instance] != NULL_PTR);

    Lin_ChannelStateType const *ChannelState = Lin_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # AMCE_LIN_2842.*/
    Hal_StatusType RetVal = STATUS_ERROR;
    boolean CheckSleepMode;
    boolean CheckPid;
    boolean CheckDl;

    /* Check if current instance is already initialized.*/
    DEVICE_ASSERT(LIN_NODE_STATE_UNINIT != ChannelState->CurrentNodeState);

    if (PduInfo != NULL_PTR)
    {
        /* Check whether current mode is sleep mode */
        CheckSleepMode = (LIN_NODE_STATE_SLEEP_MODE == ChannelState->CurrentNodeState) ? TRUE : FALSE;
        /* Check whether Pid is valid */
        CheckPid = (0xFFu == Lin_Hal_ProcessParity(PduInfo->Pid, LIN_CHECK_PARITY)) ? TRUE : FALSE;
        /* Check whether data length is in range */
        CheckDl = (((LIN_FRAMERESPONSE_IGNORE != PduInfo->Drc) && (0u == PduInfo->Dl))\
                   || (LIN_DATA_LENGTH_8 < PduInfo->Dl)) ? TRUE : FALSE;

        if ((TRUE == CheckPid) || (TRUE == CheckDl) || (TRUE == CheckSleepMode))
        {
            RetVal = STATUS_ERROR;
        }
        else
        {
            /* Check if the LIN bus is busy */
            if (TRUE == ChannelState->IsBusBusy)
            {
                RetVal = STATUS_BUSY;
            }
            else
            {
                Lin_Hal_StartSendFrame(Instance, PduInfo);
                RetVal = STATUS_SUCCESS;
            }
        }
    }

    return RetVal;
}

/* Implementation of Lin_Hal_AbortTransferData declared in Lin_Hal.h */
void Lin_Hal_AbortTransferData
(
    uint8 Instance
)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    if (Instance < UART_INSTANCE_MAX)
    {
        DEVICE_ASSERT(Lin_ChannelStateArray[Instance] != NULL_PTR);

        Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];

        /* Check if current instance is already initialized.*/
        DEVICE_ASSERT(LIN_NODE_STATE_UNINIT != ChannelState->CurrentNodeState);

        Lin_Hal_GoToIdleState(Instance);

        ChannelState->IsBusBusy = FALSE;
    }
}

/* Implementation of Lin_Hal_GetStatus declared in Lin_Hal.h */
Lin_StatusType Lin_Hal_GetStatus
(
    uint8 Instance,
    uint8 *LinSduPtr
)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_LIN_2842.*/
    DEVICE_ASSERT(Lin_ChannelStateArray[Instance] != NULL_PTR);

    /* api return */
    Lin_StatusType TempReturn = LIN_NOT_OK;
    Lin_ChannelStateType const *ChannelState = Lin_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # AMCE_LIN_2842.*/

    /* Determine current event ID */
    switch (ChannelState->ChannelInfo.CurrentEventId)
    {
    case LIN_NO_EVENT:
        TempReturn = Lin_Hal_GetStatusFromNoEvent(Instance);
        break;
    case LIN_PID_OK:
        TempReturn = Lin_Hal_GetStatusFromPidOk(Instance);
        break;

    case LIN_RX_COMPLETED:
        TempReturn = LIN_RX_OK;
        if (NULL_PTR != LinSduPtr)
        {
            Lin_Hal_CopyData(Instance, LinSduPtr);
        }
        break;
    case LIN_TX_COMPLETED:
        TempReturn = LIN_TX_OK;
        break;

    case LIN_SYNC_ERROR:
    case LIN_PID_ERROR:
        TempReturn = LIN_TX_HEADER_ERROR;
        break;

    case LIN_READBACK_ERROR:
        TempReturn = LIN_TX_ERROR;
        break;

    case LIN_CHECKSUM_ERROR:
    case LIN_RX_OVERRUN:
        TempReturn = LIN_RX_ERROR;
        break;

    case LIN_FRAME_ERROR:
        if (LIN_NODE_STATE_RECV_DATA == ChannelState->CurrentNodeState)
        {
            TempReturn = LIN_RX_ERROR;
        }
        else if (LIN_NODE_STATE_SEND_DATA == ChannelState->CurrentNodeState)
        {
            TempReturn = LIN_TX_ERROR;
        }
        else
        {
            /* do nothing */
        }
        break;
    case LIN_WAKEUP_SIGNAL:
        TempReturn = LIN_OPERATIONAL;
        break;
    case LIN_TIMEOUT_ERROR:
        TempReturn = Lin_Hal_GetStatusFromTimeoutError(Instance);
        break;

    default:
        /* Do nothing */
        break;
    }

    return TempReturn;
}

#ifndef LIN_SDK_NON_EXTENDED_API
/* Implementation of Lin_Hal_SetTimeoutCounter declared in Lin_Hal.h */
void Lin_Hal_SetTimeoutCounter
(
    uint8 Instance,
    uint32 Timeout
)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_LIN_2842.*/
    DEVICE_ASSERT(Lin_ChannelStateArray[Instance] != NULL_PTR);

    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # AMCE_LIN_2842.*/

    ChannelState->TimeoutCounter = Timeout;
}

/* Implementation of Lin_Hal_TimeoutService declared in Lin_Hal.h */
void Lin_Hal_TimeoutService(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_LIN_2842.*/
    DEVICE_ASSERT(Lin_ChannelStateArray[Instance] != NULL_PTR);

    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];
    /*PRQA S 2842 -- # AMCE_LIN_2842.*/

    if ((LIN_NODE_STATE_SEND_DATA == ChannelState->CurrentNodeState) || \
            (LIN_NODE_STATE_RECV_DATA == ChannelState->CurrentNodeState))
    {
        if (0U == ChannelState->TimeoutCounter)
        {
            ChannelState->ChannelInfo.CurrentEventId = LIN_TIMEOUT_ERROR;

            Lin_Hal_GoToIdleState(Instance);
            /* call back funtion is no pointer */
            if (NULL_PTR != ChannelState->Callback)
            {
                ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
            }
        }
        else
        {
            /*PRQA S 3387 ++ # AMCE_LIN_3387.*/
            ChannelState->TimeoutCounter--;
            /*PRQA S 3387 ++ # AMCE_LIN_3387.*/
        }
    }
}
#endif

#if (CONFIG_LIN0_ENABLE)
/**
 * @brief UART0 interrupt handler for LIN channel 0.
 *
 * @details
 * This is the interrupt service routine for UART0 hardware interrupt.
 * It delegates the interrupt processing to the common LIN HAL IRQ handler
 * for channel 0.
 *
 * @param[in] void: None
 *
 * @return void
 *
 * @pre
 *     LIN channel 0 must be initialized.
 *     UART0 interrupt must be enabled.
 *
 * @post
 *     Interrupt is processed and flags are cleared.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from interrupt context.
 *     DES ID: DES_LIN_API_009
 */
ISR(UART0_IRQHandler)
{
    Lin_Hal_IRQHandler(0U);
}
#endif

#if (CONFIG_LIN1_ENABLE)
/**
 * @brief UART1 interrupt handler for LIN channel 1.
 *
 * @details
 * This is the interrupt service routine for UART1 hardware interrupt.
 * It delegates the interrupt processing to the common LIN HAL IRQ handler
 * for channel 1.
 *
 * @param[in] void: None
 *
 * @return void
 *
 * @pre
 *     LIN channel 1 must be initialized.
 *     UART1 interrupt must be enabled.
 *
 * @post
 *     Interrupt is processed and flags are cleared.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from interrupt context.
 *     DES ID: DES_LIN_API_010
 */
ISR(UART1_IRQHandler)
{
    Lin_Hal_IRQHandler(1U);
}
#endif

#if (CONFIG_LIN2_ENABLE)
/**
 * @brief UART2 interrupt handler for LIN channel 2.
 *
 * @details
 * This is the interrupt service routine for UART2 hardware interrupt.
 * It delegates the interrupt processing to the common LIN HAL IRQ handler
 * for channel 2.
 *
 * @param[in] void: None
 *
 * @return void
 *
 * @pre
 *     LIN channel 2 must be initialized.
 *     UART2 interrupt must be enabled.
 *
 * @post
 *     Interrupt is processed and flags are cleared.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from interrupt context.
 *     DES ID: DES_LIN_API_011
 */
ISR(UART2_IRQHandler)
{
    Lin_Hal_IRQHandler(2U);
}
#endif

#if !defined(AC7840E)
#if (CONFIG_LIN3_ENABLE)
/**
 * @brief UART3 interrupt handler for LIN channel 3.
 *
 * @details
 * This is the interrupt service routine for UART3 hardware interrupt.
 * It delegates the interrupt processing to the common LIN HAL IRQ handler
 * for channel 3.
 *
 * @param[in] void: None
 *
 * @return void
 *
 * @pre
 *     LIN channel 3 must be initialized.
 *     UART3 interrupt must be enabled.
 *
 * @post
 *     Interrupt is processed and flags are cleared.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from interrupt context.
 *     DES ID: DES_LIN_API_012
 */
ISR(UART3_IRQHandler)
{
    Lin_Hal_IRQHandler(3U);
}
#endif
#endif

#if defined (AC7843X)
#if (CONFIG_LIN4_ENABLE)
/**
 * @brief UART4 interrupt handler for LIN channel 4.
 *
 * @details
 * This is the interrupt service routine for UART4 hardware interrupt.
 * It delegates the interrupt processing to the common LIN HAL IRQ handler
 * for channel 4.
 *
 * @param[in] void: None
 *
 * @return void
 *
 * @pre
 *     LIN channel 4 must be initialized.
 *     UART4 interrupt must be enabled.
 *
 * @post
 *     Interrupt is processed and flags are cleared.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from interrupt context.
 *     DES ID: DES_LIN_API_013
 */
ISR(UART4_IRQHandler)
{
    Lin_Hal_IRQHandler(4U);
}
#endif

#if (CONFIG_LIN5_ENABLE)
/**
 * @brief UART5 interrupt handler for LIN channel 5.
 *
 * @details
 * This is the interrupt service routine for UART5 hardware interrupt.
 * It delegates the interrupt processing to the common LIN HAL IRQ handler
 * for channel 5.
 *
 * @param[in] void: None
 *
 * @return void
 *
 * @pre
 *     LIN channel 5 must be initialized.
 *     UART5 interrupt must be enabled.
 *
 * @post
 *     Interrupt is processed and flags are cleared.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from interrupt context.
 *     DES ID: DES_LIN_API_014
 */
ISR(UART5_IRQHandler)
{
    Lin_Hal_IRQHandler(5U);
}
#endif

#if (CONFIG_LIN6_ENABLE)
/**
 * @brief UART6 interrupt handler for LIN channel 6.
 *
 * @details
 * This is the interrupt service routine for UART6 hardware interrupt.
 * It delegates the interrupt processing to the common LIN HAL IRQ handler
 * for channel 6.
 *
 * @param[in] void: None
 *
 * @return void
 *
 * @pre
 *     LIN channel 6 must be initialized.
 *     UART6 interrupt must be enabled.
 *
 * @post
 *     Interrupt is processed and flags are cleared.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from interrupt context.
 *     DES ID: DES_LIN_API_015
 */
ISR(UART6_IRQHandler)
{
    Lin_Hal_IRQHandler(6U);
}
#endif

#if (CONFIG_LIN7_ENABLE)
/**
 * @brief UART7 interrupt handler for LIN channel 7.
 *
 * @details
 * This is the interrupt service routine for UART7 hardware interrupt.
 * It delegates the interrupt processing to the common LIN HAL IRQ handler
 * for channel 7.
 *
 * @param[in] void: None
 *
 * @return void
 *
 * @pre
 *     LIN channel 7 must be initialized.
 *     UART7 interrupt must be enabled.
 *
 * @post
 *     Interrupt is processed and flags are cleared.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from interrupt context.
 *     DES ID: DES_LIN_API_016
 */
ISR(UART7_IRQHandler)
{
    Lin_Hal_IRQHandler(7U);
}
#endif
#endif
/*================================STATIC  FUNCTION  IMPLEMENTATIONS=================================*/
static Lin_StatusType Lin_Hal_GetStatusFromNoEvent
(
    uint8 Instance
)
{
    Lin_StatusType RetVal = LIN_NOT_OK;
    /* lin hal channel state */
    Lin_ChannelStateType const *ChannelState = Lin_ChannelStateArray[Instance];

    /* check the current node status */
    switch (ChannelState->CurrentNodeState)
    {
    case LIN_NODE_STATE_SEND_BREAK_FIELD:
    case LIN_NODE_STATE_SEND_SYNC:
    case LIN_NODE_STATE_SEND_PID:
        RetVal = LIN_TX_BUSY;
        break;
    case LIN_NODE_STATE_SLEEP_MODE:
        RetVal = LIN_CH_SLEEP;
        break;
    case LIN_NODE_STATE_IDLE:
        RetVal = LIN_OPERATIONAL;
        break;
    default:
        /* Do nothing */
        break;
    }

    return RetVal;
}

static Lin_StatusType Lin_Hal_GetStatusFromPidOk
(
    uint8 Instance
)
{
    Lin_StatusType RetVal = LIN_NOT_OK;
    /* lin hal channel state */
    Lin_ChannelStateType const *ChannelState = Lin_ChannelStateArray[Instance];

    /* check the current node status */
    if (LIN_NODE_STATE_SEND_DATA == ChannelState->CurrentNodeState)
    {
        RetVal = LIN_TX_BUSY;
    }
    else if (LIN_NODE_STATE_RECV_DATA == ChannelState->CurrentNodeState)
    {
        if (0U != ChannelState->CntByte)
        {
            RetVal = LIN_RX_BUSY;
        }
        else
        {
            RetVal = LIN_RX_NO_RESPONSE;
        }
    }
    else
    {
        /* Do nothing */
    }

    return RetVal;
}

static Lin_StatusType Lin_Hal_GetStatusFromTimeoutError
(
    uint8 Instance
)
{
    Lin_StatusType RetVal = LIN_NOT_OK;
    /* lin hal channel state */
    Lin_ChannelStateType const *ChannelState = Lin_ChannelStateArray[Instance];

    /* check the previous node status */
    switch (ChannelState->PreviousNodeState)
    {
    /* if the node is receiving data */
    case LIN_NODE_STATE_RECV_DATA:
        if (0U == ChannelState->CntByte)
        {
            RetVal = LIN_RX_NO_RESPONSE;
        }
        else
        {
            RetVal = LIN_RX_ERROR;
        }
        break;
    case LIN_NODE_STATE_SEND_DATA:
        RetVal = LIN_TX_ERROR;
        break;
    default:
        /* Do nothing */
        break;
    }

    return RetVal;
}

static void Lin_Hal_CheckWakeupFlag
(
    uint8 Instance,
    uint32 Lsr1
)
{
    /* lin hal channel state */
    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];
    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];

    /* check lin wake up mask */
    if (0U != (Lsr1 & UART_LSR1_LINWAK_Msk))
    {
        Uart_Reg_ClearStatusFlag(Base, UART_LIN_WAKEUP);

        if (LIN_NODE_STATE_SLEEP_MODE == ChannelState->CurrentNodeState)
        {
            ChannelState->ChannelInfo.CurrentEventId = LIN_WAKEUP_SIGNAL;
            /* Check if callback is null pointer */
            if (NULL_PTR != ChannelState->Callback)
            {
                ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
            }

            Lin_Hal_GoToIdleState(Instance);
        }
    }
}

static void Lin_Hal_CheckBaudrate(uint8 Instance)
{
    const Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];
    UART_Type *Base = Lin_HalBase[Instance];
    float32 Divisor = 0.0F;
    float32 MaxDivisor, MinDivisor;

    Divisor = Uart_Reg_GetBaudRateDivisor(Base);

    MaxDivisor = ChannelState->Divisor * (100.0F + 15.0F) / 100.0F;
    MinDivisor = ChannelState->Divisor * (100.0F - 15.0F) / 100.0F;

    // Check if calculated baudrate is within +-15% of configured baudrate
    if ((Divisor > MaxDivisor) || (Divisor < MinDivisor))
    {
        // Reconfigure the baudrate to match the desired value if error exceeds +-15%
        Uart_Reg_SetBaudRateDivisor(Base, ChannelState->Divisor);
    }
}

static void Lin_Hal_CheckErrorFlag
(
    uint8 Instance,
    uint32 Lsr0,
    uint32 Lsr1
)
{
    /* lin hal channel config */
    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];
    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];

    /* Check UART frame error flag */
    if (0U != (Lsr0 & UART_LSR0_FE_Msk))
    {
        Uart_Reg_ClearStatusFlag(Base, UART_FRAME_ERR);

        ChannelState->ChannelInfo.CurrentEventId = LIN_FRAME_ERROR;
        /* if note statue is tx or rx data */
        if ((LIN_NODE_STATE_SEND_DATA == ChannelState->CurrentNodeState) || \
                (LIN_NODE_STATE_RECV_DATA == ChannelState->CurrentNodeState))
        {
            /* Check callback is null pointer */
            if (ChannelState->Callback != NULL_PTR)
            {
                ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
            }
        }

        Lin_Hal_GoToIdleState(Instance);
    }
    /* Check UART overrun flag */
    else if (0U != (Lsr0 & UART_LSR0_OE_Msk))
    {
        Uart_Reg_ClearStatusFlag(Base, UART_RX_OVERRUN);

        ChannelState->ChannelInfo.CurrentEventId = LIN_RX_OVERRUN;
        /* is callback is null pointer */
        if (ChannelState->Callback != NULL_PTR)
        {
            ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
        }
    }
    /* Check UART NOISE flag */
    else if (0U != (Lsr0 & UART_LSR0_NE_Msk))
    {
        Uart_Reg_ClearStatusFlag(Base, UART_NOISE_ERR);

        ChannelState->ChannelInfo.CurrentEventId = LIN_NOISE_ERROR;
        /* is callback is null pointer */
        if (ChannelState->Callback != NULL_PTR)
        {
            ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
        }
    }
    /* Check LIN sync error flag */
    else if (0U != (Lsr1 & UART_LSR1_SYNERR_Msk))
    {
        Uart_Reg_ClearStatusFlag(Base, UART_LIN_SYNC_ERR);

        ChannelState->ChannelInfo.CurrentEventId = LIN_SYNC_ERROR;
        /* is callback is null pointer */
        if (ChannelState->Callback != NULL_PTR)
        {
            ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
        }

        Lin_Hal_CheckBaudrate(Instance);
        Lin_Hal_GoToIdleState(Instance);
    }
    else
    {
        /* do nothing */
    }
}

static void Lin_Hal_SetBaudRate
(
    uint8 Instance,
    uint32 BaudRate
)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT((LIN_MIN_CHANNEL_BAUDRATE <= BaudRate) && (LIN_MAX_CHANNEL_BAUDRATE >= BaudRate));

    /* channel clock source */
    uint32 LinSourceClock;
    float32 Divisor;
    /*PRQA S 2842 ++ # AMCE_LIN_2842 */
    UART_Type *BaseAddress = Lin_HalBase[Instance];

    (void)Ckgen_Hal_GetFreq(Lin_HalClock[Instance], &LinSourceClock);

    /* Check if current instance is clock gated off. */
    DEVICE_ASSERT(LinSourceClock > 0U);

    /* Check if the desired baud rate can be configured with the current protocol clock. */
    DEVICE_ASSERT(LinSourceClock >= (BaudRate * LIN_SAMPLE_CNT_16_VALUE));

    /* calculate the baud rate */
    Divisor = ((float32)LinSourceClock / ((float32)BaudRate * (float32)LIN_SAMPLE_CNT_16_VALUE));
    Lin_ChannelStateArray[Instance]->Divisor = Divisor;
    /*PRQA S 2842 -- */
    /* program the sampleCntReg value */
    Uart_Reg_SetSampleCounter(BaseAddress, LIN_SMP_CNT16);
    /* write the divisor value */
    Uart_Reg_SetBaudRateDivisor(BaseAddress, Divisor);
}

static void Lin_Hal_CopyData
(
    uint8 Instance,
    uint8 *LinSduPtr
)
{
    /* array index */
    uint8 Index;
    /* lin hal channel config */
    Lin_ChannelStateType const *ChannelState = Lin_ChannelStateArray[Instance];
    /* used buffer length */
    uint8 BufferLength = (uint8)(ChannelState->RxSize - 1U);

    for (Index = (uint8)0U; Index < BufferLength; Index++)
    {
        LinSduPtr[Index] = Lin_SduBuffer[Instance][Index];
    }
}

static void Lin_Hal_ProcessBreakDetect
(
    uint8 Instance
)
{
    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];
    /* current LIN state */
    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];

    /* channel config from init */
    const Lin_ChannelConfigType *ChannelConfig = Lin_ChannelConfigPtr[Instance];
    /* For master mode */
    if (LIN_MASTER == ChannelConfig->ModeType)
    {
        /* send break field */
        if (LIN_NODE_STATE_SEND_BREAK_FIELD == ChannelState->CurrentNodeState)
        {
            /* If the master detect break correctly that send by itself, then send sync byte */
            ChannelState->IsBusBusy = TRUE;
            ChannelState->CurrentNodeState = LIN_NODE_STATE_SEND_SYNC;

            Uart_Reg_PutChar(Base, (uint8)LIN_SYNC_DATA);
        }
    }
    else /* For slave mode */
    {
        /* If the slave detect break, then receive sync byte */
        ChannelState->IsBusBusy = TRUE;
        ChannelState->ChannelInfo.CurrentEventId = LIN_NO_EVENT;
        /* support sync */
        if (TRUE == ChannelConfig->AutoBaudEnable)
        {
            /* If enable auto sync baudrate function, the sync byte 0x55 can't be received */
            ChannelState->CurrentNodeState = LIN_NODE_STATE_RECV_PID;
        }
        else
        {
            ChannelState->CurrentNodeState = LIN_NODE_STATE_RECV_SYNC;
        }
    }
}

static void Lin_Hal_ProcessReceivePid
(
    uint8 Instance,
    uint8 RxByte
)
{
    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];
    /* current LIN state */
    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];

    /* for master mode */
    if (LIN_MASTER == Lin_ChannelConfigPtr[Instance]->ModeType) /* For master mode */
    {
        /* pid is read byte */
        if (RxByte == ChannelState->ChannelInfo.CurrentPid)
        {
            /* If the master receive Pid byte correctly that send by itself, then receive or send data */
            ChannelState->ChannelInfo.CurrentEventId = LIN_PID_OK;

            if (ChannelState->TxSize > 0U)
            {
                ChannelState->CurrentNodeState = LIN_NODE_STATE_SEND_DATA;
                /* Start transmitting firs data */
                Uart_Reg_PutChar(Base, Lin_SduBuffer[Instance][0U]);
            }
            else if (ChannelState->RxSize > 0U)
            {
                ChannelState->CurrentNodeState = LIN_NODE_STATE_RECV_DATA;
            }
            else
            {
                ChannelState->ChannelInfo.CurrentEventId = LIN_TX_COMPLETED;
                /* Slave to slave frame, ignor */
                Lin_Hal_GoToIdleState(Instance);
            }
        }
        else
        {
            /* If receive Pid byte error */
            ChannelState->ChannelInfo.CurrentEventId = LIN_PID_ERROR;
            ChannelState->IsBusBusy = FALSE;

            Lin_Hal_GoToIdleState(Instance);

            /* callback is no pointer */
            if (NULL_PTR != ChannelState->Callback)
            {
                ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
            }
        }
    }
    else /* For slave mode */
    {
        /* Check parity bits of Pid */
        ChannelState->ChannelInfo.CurrentPid = Lin_Hal_ProcessParity(RxByte, (uint8)LIN_CHECK_PARITY);
        /* current id is 0xFF */
        if (ChannelState->ChannelInfo.CurrentPid != 0xFFU)
        {
            /* If the slave receive Pid byte correctly, run callback function, then receive or send data */
            ChannelState->ChannelInfo.CurrentPid = RxByte;
            ChannelState->ChannelInfo.CurrentEventId = LIN_PID_OK;
            ChannelState->IsBusBusy = FALSE;

            if (NULL_PTR != ChannelState->Callback)
            {
                ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
            }
        }
        else
        {
            /* If receive Pid byte error, then go to idle state */
            ChannelState->ChannelInfo.CurrentEventId = LIN_PID_ERROR;
            /* callback is no pointer */
            if (NULL_PTR != ChannelState->Callback)
            {
                ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
            }

            Lin_Hal_GoToIdleState(Instance);
        }
    }
}

static void Lin_Hal_ProcessFrameHeader
(
    uint8 Instance,
    uint8 RxByte
)
{
    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];
    /* current LIN state */
    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];

    /* current node state */
    switch (ChannelState->CurrentNodeState)
    {
    case LIN_NODE_STATE_RECV_SYNC:
        /* rxbyte is sync Data */
        if (LIN_SYNC_DATA == RxByte)
        {
            /* If receive sync byte correctly, then receive the Pid byte */
            ChannelState->ChannelInfo.CurrentEventId = LIN_SYNC_OK;
            ChannelState->CurrentNodeState = LIN_NODE_STATE_RECV_PID;
        }
        else
        {
            /* If receive sync byte error, then go to idle state */
            ChannelState->ChannelInfo.CurrentEventId = LIN_SYNC_ERROR;
            /* callback is no pointer */
            if (NULL_PTR != ChannelState->Callback)
            {
                ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
            }

            Lin_Hal_GoToIdleState(Instance);
        }
        break;

    case LIN_NODE_STATE_SEND_SYNC:
        if (LIN_SYNC_DATA == RxByte)
        {
            /* If the master receive sync byte correctly that send by itself, then send Pid byte */
            ChannelState->CurrentNodeState = LIN_NODE_STATE_SEND_PID;
            Uart_Reg_PutChar(Base, ChannelState->ChannelInfo.CurrentPid);
        }
        else
        {
            /* If receive sync byte error, then go to idle state */
            ChannelState->ChannelInfo.CurrentEventId = LIN_SYNC_ERROR;
            ChannelState->IsBusBusy = FALSE;

            Lin_Hal_GoToIdleState(Instance);

            /* callback is no pointer */
            if (NULL_PTR != ChannelState->Callback)
            {
                ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
            }
        }
        break;

    case LIN_NODE_STATE_SEND_PID:
    case LIN_NODE_STATE_RECV_PID:
        Lin_Hal_ProcessReceivePid(Instance, RxByte);
        break;

    default:
        /* Do nothing */
        break;
    }
}

static uint8 Lin_Hal_MakeCheckSumByte
(
    const uint8 *BuffPtr,
    uint8 Size,
    uint8 Pid
)
{
    /* arrau index */
    uint8 Index;
    /* check sum value negation */
    uint8 ElCheckSum;
    /* check sum value */
    uint16 CheckSum;

    /* is unused pid */
    if ((Pid != 0x3CU) && (Pid != 0x7DU) && (Pid != 0xFEU) && (Pid != 0xBFU))
    {
        CheckSum = Pid;
    }
    else
    {
        /* if the id is 0x3C, 0x3D, 0x3E, 0x3F, not add Pid to checksum */
        CheckSum = 0x0U;
    }

    /* rx data check sum */
    for (Index = 0x0U; Index < Size; Index++)
    {
        CheckSum += BuffPtr[Index];
        if (CheckSum > 0xFFU)
        {
            CheckSum -= 0xFFU;
        }
    }
    ElCheckSum = (uint8)((~ CheckSum) & 0xFFU);

    return ElCheckSum;
}

static void Lin_Hal_ProcessReceiveFrameData
(
    uint8 Instance,
    uint8 RxByte
)
{
    /* current LIN state */
    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];
    uint8 CurCheckSum = 0U;

    /* Save frame data or checksum */
    if (ChannelState->RxSize > (ChannelState->CntByte + 1U))
    {
        Lin_SduBuffer[Instance][ChannelState->CntByte] = RxByte;
    }
    else
    {
        CurCheckSum = RxByte;
    }

    ChannelState->CntByte++;
    /* Check recieve completely */
    if (ChannelState->CntByte == ChannelState->RxSize)
    {
        /* Check the checksum */
        if (Lin_Hal_MakeCheckSumByte(&Lin_SduBuffer[Instance][0U], ChannelState->RxSize - 1U, \
                                     ChannelState->CheckSum) == CurCheckSum)
        {
            ChannelState->ChannelInfo.RxBuff = &Lin_SduBuffer[Instance][0U];
            /* If the checksum byte is correct, run callback function, then go to idle state */
            ChannelState->ChannelInfo.CurrentEventId = LIN_RX_COMPLETED;
            ChannelState->CurrentNodeState = LIN_NODE_STATE_RECV_DATA_COMPLETED;
        }
        else
        {
            /* If the checksum byte is error, then go to idle state */
            ChannelState->ChannelInfo.CurrentEventId = LIN_CHECKSUM_ERROR;
        }

        ChannelState->IsBusBusy = FALSE;
        /* call back funtion is no pointer */
        if (NULL_PTR != ChannelState->Callback)
        {
            ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
        }

        if (ChannelState->CurrentNodeState != LIN_NODE_STATE_SLEEP_MODE)
        {
            Lin_Hal_GoToIdleState(Instance);
        }
    }
}

static void Lin_Hal_ProcessSendFrameData
(
    uint8 Instance,
    uint8 RxByte
)
{
    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];
    /* current LIN state */
    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];

    uint8 LeftSize = (uint8)(ChannelState->TxSize - ChannelState->CntByte);

    /* Check recieve byte that send by itself */
    if (((1U == LeftSize) && (ChannelState->CheckSum != RxByte)) || \
            ((LeftSize != 1U) && (*ChannelState->TxBuff != RxByte)))
    {
        /* If the receive byte is error, then go to idle state */
        ChannelState->ChannelInfo.CurrentEventId = LIN_READBACK_ERROR;

        Lin_Hal_GoToIdleState(Instance);

        /* call back funtion is no pointer */
        if (NULL_PTR != ChannelState->Callback)
        {
            ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
        }
    }
    else
    {
        ChannelState->TxBuff++;
        ChannelState->CntByte++;
        /* current byte data less than txsize */
        if (ChannelState->CntByte < ChannelState->TxSize)
        {
            /* If sending data is not complete, continue send data or checksum */
            if (1U == (ChannelState->TxSize - ChannelState->CntByte))
            {
                Uart_Reg_PutChar(Base, ChannelState->CheckSum);
            }
            else
            {
                Uart_Reg_PutChar(Base, *ChannelState->TxBuff);
            }
        }
        else
        {
            /* If sending data is completed, run callback function, then go to idle state */
            ChannelState->ChannelInfo.CurrentEventId = LIN_TX_COMPLETED;
            ChannelState->CurrentNodeState = LIN_NODE_STATE_SEND_DATA_COMPLETED;

            Lin_Hal_GoToIdleState(Instance);

            /* call back funtion is no pointer */
            if (NULL_PTR != ChannelState->Callback)
            {
                ChannelState->Callback(Instance, &ChannelState->ChannelInfo);
            }
        }
    }

    return;
}

static void Lin_Hal_ProcessFrame
(
    uint8 Instance
)
{
    /* rx data */
    uint8 RxByte = 0U;
    /* used channel address */
    const UART_Type *Base = Lin_HalBase[Instance];
    /* current LIN state */
    Lin_ChannelStateType const *ChannelState = Lin_ChannelStateArray[Instance];

    Uart_Reg_GetChar(Base, &RxByte);

    /* current node state */
    switch (ChannelState->CurrentNodeState)
    {
    case LIN_NODE_STATE_SEND_SYNC:
    case LIN_NODE_STATE_RECV_SYNC:
    case LIN_NODE_STATE_SEND_PID:
    case LIN_NODE_STATE_RECV_PID:
        Lin_Hal_ProcessFrameHeader(Instance, RxByte);
        break;

    case LIN_NODE_STATE_RECV_DATA:
        Lin_Hal_ProcessReceiveFrameData(Instance, RxByte);
        break;

    case LIN_NODE_STATE_SEND_DATA:
        Lin_Hal_ProcessSendFrameData(Instance, RxByte);
        break;

    default:
        /* Do nothing */
        break;
    }
}

static void Lin_Hal_StartSendFrame(uint8 Instance, const Lin_PduType *PduInfo)
{
    uint8 Index;
    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];
    Lin_ChannelStateType *ChannelState = Lin_ChannelStateArray[Instance];
    const Lin_ChannelConfigType *ChannelConfig = Lin_ChannelConfigPtr[Instance];

    ChannelState->ChannelInfo.CurrentPid = PduInfo->Pid;
    /* Current Checksum is 0x00u if checksum type is classic or CurrentPid in otherwise */
    ChannelState->CheckSum = (LIN_CLASSIC_CS == PduInfo->Cs) ? 0x00u : PduInfo->Pid;

    if (LIN_FRAMERESPONSE_TX == PduInfo->Drc)
    {
        for (Index = 0u; Index < PduInfo->Dl; Index++)
        {
            Lin_SduBuffer[Instance][Index] = PduInfo->SduPtr[Index];
        }

        ChannelState->CheckSum = Lin_Hal_MakeCheckSumByte(PduInfo->SduPtr, PduInfo->Dl, ChannelState->CheckSum);

        ChannelState->TxBuff = &Lin_SduBuffer[Instance][0];
        ChannelState->TxSize = PduInfo->Dl + 1U;
        ChannelState->RxSize = 0U;
    }
    else if (LIN_FRAMERESPONSE_RX == PduInfo->Drc)
    {
        for (Index = 0u; Index < LIN_DATA_LENGTH_8; Index++)
        {
            Lin_SduBuffer[Instance][Index] = 0U;
        }
        ChannelState->TxSize = 0U;
        ChannelState->RxSize = PduInfo->Dl + 1U;
    }
    else
    {
        ChannelState->TxSize = 0U;
        ChannelState->RxSize = 0U;
    }

    ChannelState->CntByte = 0U;

    if (LIN_MASTER == ChannelConfig->ModeType)
    {
        ChannelState->CurrentNodeState = LIN_NODE_STATE_SEND_BREAK_FIELD;
        ChannelState->ChannelInfo.CurrentEventId = LIN_NO_EVENT;
        ChannelState->IsBusBusy = TRUE;

        /* Send break */
        Uart_Reg_SendLINBreak(Base);
        /* reset divisor for CDC */
        Uart_Reg_SetBaudRateDivisor(Base, ChannelState->Divisor);
    }
    else
    {
        if (LIN_FRAMERESPONSE_TX == PduInfo->Drc)
        {
            ChannelState->CurrentNodeState = LIN_NODE_STATE_SEND_DATA;
            ChannelState->IsBusBusy = TRUE;

            /* Start transmitting data */
            Uart_Reg_PutChar(Base, *ChannelState->TxBuff);
        }
        else if (LIN_FRAMERESPONSE_RX == PduInfo->Drc)
        {
            ChannelState->CurrentNodeState = LIN_NODE_STATE_RECV_DATA;
            ChannelState->IsBusBusy = TRUE;
        }
        else
        {
            Lin_Hal_GoToIdleState(Instance);
        }
    }
}

static void Lin_Hal_StateInit
(
    uint8 Instance,
    const Lin_ChannelConfigType *ChannelConfigPtr
)
{
    /* lin hal channel config */
    Lin_ChannelStateType *StatePtr = Lin_ChannelStateArray[Instance];

    /* Init Lin_State config */
    StatePtr->IsBusBusy = FALSE;
    StatePtr->ChannelInfo.CurrentEventId = LIN_NO_EVENT;
    StatePtr->CurrentNodeState = LIN_NODE_STATE_IDLE;
    StatePtr->PreviousNodeState = LIN_NODE_STATE_IDLE;
    StatePtr->Callback = ChannelConfigPtr->Callback;
    StatePtr->TimeoutCounter = 0U;
}

static void Lin_Hal_IRQHandler
(
    uint8 Instance
)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_LIN_2842.*/
    /* used channel address */
    UART_Type *Base = Lin_HalBase[Instance];
    /*PRQA S 2842 -- # AMCE_LIN_2842.*/

    /* Lsr0 read data */
    uint32 Lsr0 = Uart_Reg_GetLSR0(Base);
    /* Lsr1 read data */
    uint32 Lsr1 = Uart_Reg_GetLSR1(Base);

    /* Check LIN wakeup flag */
    Lin_Hal_CheckWakeupFlag(Instance, Lsr1);

    /* Check UART error flag */
    Lin_Hal_CheckErrorFlag(Instance, Lsr0, Lsr1);

    /* Check LIN received break flag */
    if (0U != (Lsr1 & UART_LSR1_FBRK_Msk))
    {
        Uart_Reg_ClearStatusFlag(Base, UART_LIN_BREAK);
        Lin_Hal_ProcessBreakDetect(Instance);
    }
    /* Check LIN received data flag */
    else if (0U != (Lsr0 & UART_LSR0_DR_Msk))
    {
        Lin_Hal_ProcessFrame(Instance);
    }
    else
    {
        /* do nothing */
    }
}
/*============================================EOF===================================================*/
