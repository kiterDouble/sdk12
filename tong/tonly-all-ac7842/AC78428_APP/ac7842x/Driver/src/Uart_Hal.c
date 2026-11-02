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
* @brief  This file provides Uart hal function
*/
/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Uart_Reg.h"
#include "Uart_Hal.h"
#include "Dma_Hal.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"
#include "OsIf_Time.h"
#include "OsIf_Irq.h"
#include "OsIf_Critical.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/**
@def UART_SAMPLE_CNT_4_VALUE
@brief Sample count value for 4x oversampling.
@note DES ID: DES_UART_MACRO_001
*/
#define UART_SAMPLE_CNT_4_VALUE       (4U)
/**
@def UART_SAMPLE_CNT_8_VALUE
@brief Sample count value for 8x oversampling.
@note DES ID: DES_UART_MACRO_002
*/
#define UART_SAMPLE_CNT_8_VALUE       (8U)
/**
@def UART_SAMPLE_CNT_16_VALUE
@brief Sample count value for 16x oversampling.
@note DES ID: DES_UART_MACRO_003
*/
#define UART_SAMPLE_CNT_16_VALUE      (16U)

/**
@def UART_USE_DMA_TRANSMIT_LEN_MAX
@brief Maximum length for DMA transmit operations in bytes.
@note DES ID: DES_UART_MACRO_004
*/
#define UART_USE_DMA_TRANSMIT_LEN_MAX (32768U)

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/**
@brief UART runtime state structure.
@note DES ID: DES_UART_TYP_015
*/
typedef struct
{
    boolean InitState;                       /*!< Initialization state of the UART channel */
    const uint8 *TxBuff;                     /*!< The buffer of transmit data */
    uint8 *RxBuff;                           /*!< The buffer of received data */
    uint32 TxSize;                           /*!< The remaining number of bytes to be transmitted */
    uint32 RxSize;                           /*!< The remaining number of bytes to be received */
    boolean IsTxBusy;                        /*!< True if transmit is active */
    boolean IsRxBusy;                        /*!< True if receive is active */
    boolean IsTxBlocking;                    /*!< True if transmit is blocking */
    boolean IsRxBlocking;                    /*!< True if receive is blocking */
    Uart_PerCharConfigType BitCountPerChar;  /*!< number of bits(7/8/9 bits) in a char */
    Uart_CallbackType RxCallback;            /*!< Callback for data receive */
    Uart_CallbackType TxCallback;            /*!< Callback for data send */
    Uart_TransferType TransferType;          /*!< Type of UART transfer (interrupt or dma) */
    uint8 RxDmaChannel;                      /*!< DMA channel for UART receive. */
    uint8 TxDmaChannel;                      /*!< DMA channel for UART transmit. */
    volatile boolean RxComplete;             /*!< Flag to indicate the completion of a blocking receive operation */
    volatile boolean TxComplete;             /*!< Flag to indicate the completion of a blocking transmit operation */
    Hal_StatusType SendStatus;               /*!< Status of last transmit */
    Hal_StatusType ReceiveStatus;            /*!< Status of last receive */
} Uart_ChannelStateType;

/*===========================================VARIABLE DECLARATIONS==================================*/
/**
@brief Runtime state structure for UART channel 0.
@note DES ID: DES_UART_VAR_001
*/
#if (CONFIG_UART0_ENABLE)
static Uart_ChannelStateType Uart0_ChannelState;
#endif
/**
@brief Runtime state structure for UART channel 1.
@note DES ID: DES_UART_VAR_002
*/
#if (CONFIG_UART1_ENABLE)
static Uart_ChannelStateType Uart1_ChannelState;
#endif
/**
@brief Runtime state structure for UART channel 2.
@note DES ID: DES_UART_VAR_003
*/
#if (CONFIG_UART2_ENABLE)
static Uart_ChannelStateType Uart2_ChannelState;
#endif
#if !defined(AC7840E)
/**
@brief Runtime state structure for UART channel 3.
@note DES ID: DES_UART_VAR_004
*/
#if (CONFIG_UART3_ENABLE)
static Uart_ChannelStateType Uart3_ChannelState;
#endif
#endif

#if defined (AC7843X)
/**
@brief Runtime state structure for UART channel 4.
@note DES ID: DES_UART_VAR_005
*/
#if (CONFIG_UART4_ENABLE)
static Uart_ChannelStateType Uart4_ChannelState;
#endif
/**
@brief Runtime state structure for UART channel 5.
@note DES ID: DES_UART_VAR_006
*/
#if (CONFIG_UART5_ENABLE)
static Uart_ChannelStateType Uart5_ChannelState;
#endif
/**
@brief Runtime state structure for UART channel 6.
@note DES ID: DES_UART_VAR_007
*/
#if (CONFIG_UART6_ENABLE)
static Uart_ChannelStateType Uart6_ChannelState;
#endif
/**
@brief Runtime state structure for UART channel 7.
@note DES ID: DES_UART_VAR_008
*/
#if (CONFIG_UART7_ENABLE)
static Uart_ChannelStateType Uart7_ChannelState;
#endif
#endif

/**
@brief Array of pointers to runtime state structures for all UART channels.
@note DES ID: DES_UART_VAR_009
*/
static Uart_ChannelStateType *const Uart_ChannelStateArray[UART_INSTANCE_MAX] =
{
#if (CONFIG_UART0_ENABLE)
    &Uart0_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_UART1_ENABLE)
    &Uart1_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_UART2_ENABLE)
    &Uart2_ChannelState,
#else
    NULL_PTR,
#endif
#if !defined(AC7840E)
#if (CONFIG_UART3_ENABLE)
    &Uart3_ChannelState,
#else
    NULL_PTR,
#endif
#endif

#if defined (AC7843X)
#if (CONFIG_UART4_ENABLE)
    &Uart4_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_UART5_ENABLE)
    &Uart5_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_UART6_ENABLE)
    &Uart6_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_UART7_ENABLE)
    &Uart7_ChannelState,
#else
    NULL_PTR,
#endif
#endif
};

/**
@brief Table of base addresses for UART instances.
@note DES ID: DES_UART_VAR_010
*/
static UART_Type *const Uart_HalBase[UART_INSTANCE_MAX] =
{
    UART0, UART1, UART2,
#if !defined(AC7840E)
    UART3,
#endif
#if defined (AC7843X)
    UART4, UART5, UART6, UART7
#endif
};
/*PRQA S 0306 -- */

/**
@brief Table to save UART IRQ numbers for each instance.
@note DES ID: DES_UART_VAR_011
*/
static const IRQn_Type Uart_IrqId[UART_INSTANCE_MAX] = UART_IRQS;

/**
@brief UART peripheral clock identifiers for each instance.
@note DES ID: DES_UART_VAR_012
*/
/*PRQA S 3218 ++ # AMCE_UART_3218.*/
static const Ckgen_ClkIdType Uart_HalClock[UART_INSTANCE_MAX] =
{
    CKGEN_UART0_CLK, CKGEN_UART1_CLK, CKGEN_UART2_CLK,
#if !defined(AC7840E)
    CKGEN_UART3_CLK,
#endif
#if defined (AC7843X)
    CKGEN_UART4_CLK, CKGEN_UART5_CLK, CKGEN_UART6_CLK, CKGEN_UART7_CLK
#endif
};
/*PRQA S 3218 -- */

/**
@brief UART bus clock identifiers for each instance.
@note DES ID: DES_UART_VAR_013
*/
static const Ckgen_BusClkIdType Uart_HalBusClock[UART_INSTANCE_MAX] =
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
@brief UART reset control identifiers for each instance.
@note DES ID: DES_UART_VAR_014
*/
static const Rcm_ResetIDType Uart_HalClockReset[UART_INSTANCE_MAX] =
{
    RCM_RESET_ID_UART0, RCM_RESET_ID_UART1, RCM_RESET_ID_UART2,
#if !defined(AC7840E)
    RCM_RESET_ID_UART3,
#endif
#if defined (AC7843X)
    RCM_RESET_ID_UART4, RCM_RESET_ID_UART5, RCM_RESET_ID_UART6, RCM_RESET_ID_UART7
#endif
};

/*============================================FUNCTION PROTOTYPES===================================*/
#if (CONFIG_UART0_ENABLE)
ISR(UART0_IRQHandler);
#endif

#if (CONFIG_UART1_ENABLE)
ISR(UART1_IRQHandler);
#endif

#if (CONFIG_UART2_ENABLE)
ISR(UART2_IRQHandler);
#endif

#if !defined(AC7840E)
#if (CONFIG_UART3_ENABLE)
ISR(UART3_IRQHandler);
#endif
#endif

#if defined (AC7843X)
#if (CONFIG_UART4_ENABLE)
ISR(UART4_IRQHandler);
#endif

#if (CONFIG_UART5_ENABLE)
ISR(UART5_IRQHandler);
#endif

#if (CONFIG_UART6_ENABLE)
ISR(UART6_IRQHandler);
#endif

#if (CONFIG_UART7_ENABLE)
ISR(UART7_IRQHandler);
#endif
#endif

#ifndef UART_SDK_NON_EXTENDED_API
/**
 * @brief Prepares timeout checking by initializing counter and tick values.
 *
 * @details
 * This function initializes the timeout mechanism by converting the timeout
 * value from microseconds to timer ticks and capturing the current counter value.
 * It is used internally for blocking UART operations with timeout.
 *
 * @param[out] CounterOut: Pointer to store the current counter value.
 * @param[out] TicksOut: Pointer to store the number of ticks corresponding to the timeout.
 * @param[in] Timeout: Timeout in microseconds.
 *
 * @return void
 *
 * @pre None.
 *
 * @post Counter and tick values are initialized for timeout checking.
 *
 * @note
 *     This is an inline helper function for internal use.
 *     DES ID: DES_UART_API_051
 */
LOCAL_INLINE void Uart_Hal_StartTimeout(uint32 *CounterOut, uint32 *TicksOut, uint32 Timeout);

/**
 * @brief Checks if the timeout period has elapsed.
 *
 * @details
 * This function calculates the elapsed time since the last check and determines
 * if the timeout period has expired. It accumulates elapsed ticks and compares
 * against the timeout threshold.
 *
 * @param[inout] Counter: Pointer to counter value, which is updated with the current counter.
 * @param[inout] ElapsedTicks: Pointer to store the accumulated elapsed ticks.
 * @param[in] TimeoutTicks: The total number of ticks representing the timeout.
 *
 * @return TRUE if the timeout has elapsed, FALSE otherwise.
 *
 * @pre Uart_Hal_StartTimeout must have been called to initialize the timeout mechanism.
 *
 * @post Counter is updated with current value, elapsed ticks are accumulated.
 *
 * @note
 *     This is an inline helper function for internal use.
 *     DES ID: DES_UART_API_052
 */
LOCAL_INLINE boolean Uart_Hal_CheckTimeout(uint32 *Counter, uint32 *ElapsedTicks, uint32 TimeoutTicks);
#endif

/**
 * @brief Initiates UART interrupt-based data transmission.
 *
 * @details
 * This function configures the UART channel state for transmission, sets the
 * transmit buffer pointer and size, marks the transmission status as busy, and
 * enables the TX not full interrupt to begin the transmission process.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] TxBuffer: Pointer to the transmit data buffer.
 * @param[in] TxSize: Number of bytes to transmit.
 *
 * @return Hal_StatusType: STATUS_SUCCESS if successful.
 *
 * @pre UART channel must be initialized and not busy.
 *
 * @post TX interrupt is enabled and transmission begins.
 *
 * @note
 *     For internal use by the UART HAL driver.
 *     DES ID: DES_UART_API_053
 */
static Hal_StatusType Uart_Hal_StartSendDataUsingInt(uint8 Instance, const uint8 *TxBuffer, uint32 TxSize);

/**
 * @brief Stops UART DMA-based data transmission.
 *
 * @details
 * This function disables TX DMA requests, stops the DMA channel, disables the
 * TX complete interrupt, updates the transmission status, and marks the channel
 * as not busy. If blocking mode is active, it signals completion.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return void
 *
 * @pre DMA transmission must be in progress.
 *
 * @post DMA channel stopped, interrupts disabled, transmission marked complete.
 *
 * @note
 *     For internal use by the UART HAL driver.
 *     DES ID: DES_UART_API_054
 */
static void Uart_Hal_StopTxDma(uint8 Instance);

/**
 * @brief Completes UART interrupt-based data transmission.
 *
 * @details
 * This function finalizes the transmission by updating the status, disabling
 * TX interrupts, clearing the busy flag, and signaling completion if in
 * blocking mode.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return void
 *
 * @pre Interrupt-based transmission must be in progress.
 *
 * @post TX interrupts disabled, transmission marked complete.
 *
 * @note
 *     For internal use by the UART HAL driver.
 *     DES ID: DES_UART_API_055
 */
static void Uart_Hal_CompleteSendDataUsingInt(uint8 Instance);

/**
 * @brief Handles UART transmit complete interrupt.
 *
 * @details
 * This function is called when the TX complete interrupt fires. It checks if
 * all data has been transmitted, completes the transfer operation, and invokes
 * the user callback if registered.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return void
 *
 * @pre TX complete interrupt must be enabled.
 *
 * @post Transmission completed, callback invoked if registered.
 *
 * @note
 *     Called from ISR context.
 *     DES ID: DES_UART_API_056
 */
static void Uart_Hal_TxCompleteIrqHandler(uint8 Instance);

/**
 * @brief Callback function for UART DMA transmission completion.
 *
 * @details
 * This callback is invoked by the DMA driver when a transfer completes or
 * encounters an error. It handles error conditions, updates transfer status,
 * enables TX complete interrupt for final verification, and invokes user callbacks.
 *
 * @param[in] Param: Pointer to DMA channel callback information structure.
 *
 * @return void
 *
 * @pre DMA transmission must be in progress.
 *
 * @post DMA transfer finalized, status updated, callbacks invoked.
 *
 * @note
 *     Called from DMA ISR context.
 *     DES ID: DES_UART_API_057
 */
static void Uart_Hal_TxDmaCallback(void *Param);

/**
 * @brief Initiates UART DMA-based data transmission.
 *
 * @details
 * This function configures and starts DMA transfer for UART transmission.
 * It enables TX DMA requests, configures the DMA channel with source/destination
 * addresses, transfer size, and callback, then starts the DMA channel and enables
 * the UART transmitter.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] TxBuffer: Pointer to the transmit data buffer.
 * @param[in] TxSize: Number of bytes to transmit (must be less than 32768).
 *
 * @return Hal_StatusType: STATUS_SUCCESS if successful, STATUS_ERROR if DMA configuration fails.
 *
 * @pre UART channel must be initialized and not busy. DMA channel must be configured.
 *
 * @post DMA channel started, TX DMA enabled, transmission in progress.
 *
 * @note
 *     For internal use by the UART HAL driver.
 *     Maximum transfer size is 32768 bytes.
 *     DES ID: DES_UART_API_058
 */
static Hal_StatusType Uart_Hal_StartSendDataUsingDma(uint8 Instance, const uint8 *TxBuffer, uint32 TxSize);

/**
 * @brief Initiates UART interrupt-based data reception.
 *
 * @details
 * This function configures the UART channel state for reception, sets the
 * receive buffer pointer and size, enables the receiver and error interrupts,
 * and enables the RX not empty interrupt to begin the reception process.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[out] RxBuffer: Pointer to the receive data buffer.
 * @param[in] RxSize: Number of bytes to receive.
 *
 * @return Hal_StatusType: STATUS_SUCCESS if successful.
 *
 * @pre UART channel must be initialized and not busy.
 *
 * @post RX interrupt is enabled and reception begins.
 *
 * @note
 *     For internal use by the UART HAL driver.
 *     DES ID: DES_UART_API_059
 */
static Hal_StatusType Uart_Hal_StartReceiveDataUsingInt(uint8 Instance, uint8 *RxBuffer, uint32 RxSize);

/**
 * @brief Stops UART DMA-based data reception.
 *
 * @details
 * This function disables the receiver and error interrupts, disables RX DMA requests,
 * clears receive data ready flags, stops the DMA channel, updates reception status,
 * and marks the channel as not busy. If blocking mode is active, it signals completion.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return void
 *
 * @pre DMA reception must be in progress.
 *
 * @post DMA channel stopped, interrupts disabled, reception marked complete.
 *
 * @note
 *     For internal use by the UART HAL driver.
 *     DES ID: DES_UART_API_060
 */
static void Uart_Hal_StopRxDma(uint8 Instance);

/**
 * @brief Callback function for UART DMA reception completion.
 *
 * @details
 * This callback is invoked by the DMA driver when a receive transfer completes
 * or encounters an error. It handles error conditions, updates transfer status,
 * stops reception, clears error flags, and invokes user callbacks.
 *
 * @param[in] Param: Pointer to DMA channel callback information structure.
 *
 * @return void
 *
 * @pre DMA reception must be in progress.
 *
 * @post DMA transfer finalized, status updated, callbacks invoked.
 *
 * @note
 *     Called from DMA ISR context.
 *     DES ID: DES_UART_API_061
 */
static void Uart_Hal_RxDmaCallback(void *Param);

/**
 * @brief Initiates UART DMA-based data reception.
 *
 * @details
 * This function configures and starts DMA transfer for UART reception.
 * It enables RX DMA requests, clears receive flags and errors, configures the
 * DMA channel with source/destination addresses, transfer size, and callback,
 * then starts the DMA channel and enables the UART receiver.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[out] RxBuffer: Pointer to the receive data buffer.
 * @param[in] RxSize: Number of bytes to receive (must be less than 32768).
 *
 * @return Hal_StatusType: STATUS_SUCCESS if successful, STATUS_ERROR if DMA configuration fails.
 *
 * @pre UART channel must be initialized and not busy. DMA channel must be configured.
 *
 * @post DMA channel started, RX DMA enabled, reception in progress.
 *
 * @note
 *     For internal use by the UART HAL driver.
 *     Maximum transfer size is 32768 bytes.
 *     DES ID: DES_UART_API_062
 */
static Hal_StatusType Uart_Hal_StartReceiveDataUsingDma(uint8 Instance, uint8 *RxBuffer, uint32 RxSize);

/**
 * @brief Completes UART interrupt-based data reception.
 *
 * @details
 * This function finalizes the reception by disabling the receiver and error
 * interrupts, disabling RX interrupt, clearing the receive data ready flag,
 * updating the status, clearing the busy flag, and signaling completion if
 * in blocking mode.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return void
 *
 * @pre Interrupt-based reception must be in progress.
 *
 * @post RX interrupts disabled, reception marked complete.
 *
 * @note
 *     For internal use by the UART HAL driver.
 *     DES ID: DES_UART_API_063
 */
static void Uart_Hal_CompleteReceiveDataUsingInt(uint8 Instance);

/**
 * @brief Handles UART error interrupts.
 *
 * @details
 * This function processes UART error conditions including framing errors,
 * noise errors, parity errors, and RX overrun. It updates the receive status,
 * aborts the current reception, clears error flags, and invokes the user
 * callback if registered.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] ErrFlag: Error status flags from LSR register.
 *
 * @return void
 *
 * @pre UART must be in receive mode with error interrupts enabled.
 *
 * @post Error handled, reception aborted, callback invoked if registered.
 *
 * @note
 *     Called from ISR context.
 *     DES ID: DES_UART_API_064
 */
static void Uart_Hal_ErrIrqHandler(uint8 Instance, uint32 ErrFlag);
/**
 * @brief Reads data from UART receive buffer register.
 *
 * @details
 * This function reads one character (7/8/9 bits) from the UART RBR register
 * and stores it in the receive buffer. It handles both standard (7/8-bit) and
 * extended (9-bit) character formats.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return void
 *
 * @pre Receive buffer must have space available.
 *
 * @post Data read from RBR and stored in receive buffer.
 *
 * @note
 *     Called from RX interrupt handler.
 *     DES ID: DES_UART_API_065
 */
static void Uart_Hal_GetData(uint8 Instance);

/**
 * @brief Handles UART receive data not empty interrupt.
 *
 * @details
 * This function is called when the RX not empty interrupt fires. It reads
 * the received data, updates buffer pointers and size, invokes user callbacks
 * when buffer is full, and completes the reception when all expected data
 * has been received.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return void
 *
 * @pre RX interrupt must be enabled and data available.
 *
 * @post Data read, buffer updated, callbacks invoked if needed.
 *
 * @note
 *     Called from ISR context.
 *     DES ID: DES_UART_API_066
 */
static void Uart_Hal_RxIrqHandler(uint8 Instance);

/**
 * @brief Writes data to UART transmit buffer register.
 *
 * @details
 * This function writes one character (7/8/9 bits) from the transmit buffer
 * to the UART TBR register. It handles both standard (7/8-bit) and extended
 * (9-bit) character formats.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return void
 *
 * @pre Transmit buffer must have data available.
 *
 * @post Data written to TBR from transmit buffer.
 *
 * @note
 *     Called from TX interrupt handler.
 *     DES ID: DES_UART_API_067
 */
static void Uart_Hal_PutData(uint8 Instance);

/**
 * @brief Handles UART transmit not full interrupt.
 *
 * @details
 * This function is called when the TX not full interrupt fires. It writes
 * the next byte to the transmit register, updates buffer pointers and size,
 * enables TX complete interrupt for the last byte, invokes user callbacks
 * when buffer is empty, and disables TX interrupt when transmission is complete.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return void
 *
 * @pre TX interrupt must be enabled and transmit FIFO not full.
 *
 * @post Data written, buffer updated, callbacks invoked if needed.
 *
 * @note
 *     Called from ISR context.
 *     DES ID: DES_UART_API_068
 */
static void Uart_Hal_TxIrqHandler(uint8 Instance);

/**
 * @brief Initializes the UART channel runtime state structure.
 *
 * @details
 * This function initializes all runtime state variables for a UART channel
 * including busy flags, blocking flags, completion flags, buffer sizes,
 * transfer type, bit count, DMA channels, status values, and callback pointers.
 *
 * @param[in] Instance: UART hardware channel ID.
 * @param[in] ChannelConfigPtr: Pointer to the UART channel configuration structure.
 *
 * @return void
 *
 * @pre Configuration structure must be valid.
 *
 * @post Channel state structure fully initialized.
 *
 * @note
 *     Called during UART initialization.
 *     DES ID: DES_UART_API_069
 */
static void Uart_Hal_StateInit(uint8 Instance, const Uart_ChannelConfigType *ChannelConfigPtr);

/**
 * @brief Main UART interrupt handler dispatcher.
 *
 * @details
 * This function is the central interrupt handler for all UART interrupt sources.
 * It reads the interrupt status registers and dispatches to the appropriate
 * handler for error, receive, transmit, data match, and idle line interrupts.
 *
 * @param[in] Instance: UART hardware channel ID.
 *
 * @return void
 *
 * @pre UART must be initialized and interrupts enabled.
 *
 * @post Appropriate interrupt handlers called and flags cleared.
 *
 * @note
 *     Called from ISR context by UARTx_IRQHandler functions.
 *     DES ID: DES_UART_API_070
 */
static void Uart_Hal_IRQHandler(uint8 Instance);

/*===============================GLOBAL FUNCTION  IMPLEMENTATIONS=================================*/
/* Implementation of Uart_Hal_Init declared in Uart_Hal.h */
void Uart_Hal_Init(uint8 Instance, const Uart_ChannelConfigType *ChannelConfigPtr)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(ChannelConfigPtr != NULL_PTR);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    /* Check if current instance is already initialized. */
    DEVICE_ASSERT(FALSE == ChannelState->InitState);

    /* Init the state struct */
    Uart_Hal_StateInit(Instance, ChannelConfigPtr);
    /* enable uart clock */
    (void)Ckgen_Hal_EnablePeriphClk(Uart_HalBusClock[Instance], TRUE);
    Rcm_Hal_SetResetState(Uart_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(Uart_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);

    /*PRQA S 2812 ++ # AMCE_UART_2812.*/
    /* initialize the parameters of the UART config structure */
    (void)Uart_Hal_SetBaudRate(Instance, ChannelConfigPtr->BaudRate, ChannelConfigPtr->SampleCnt);
    /*PRQA S 2812 -- */

    Uart_Reg_SetBitCountPerChar(BaseAddress, ChannelConfigPtr->BitCountPerChar);

    Uart_Reg_SetParityMode(BaseAddress, ChannelConfigPtr->ParityMode);
    Uart_Reg_SetStopBitCount(BaseAddress, ChannelConfigPtr->StopBitCount);

    /* Enable UART FIFO */
    Uart_Reg_SetFIFO(BaseAddress, TRUE);

    /* Enable UART NVIC interrupt */
    Core_Hal_EnableIrq(Uart_IrqId[Instance]);
    /*PRQA S 2842 -- */

    /* Enable the UART transmitter */
    Uart_Reg_SetTransmitterCmd(BaseAddress, TRUE);

    /* Enable the UART receiver */
    Uart_Reg_SetReceiverCmd(BaseAddress, TRUE);

    ChannelState->InitState = TRUE;
}

/* Implementation of Uart_Hal_DeInit declared in Uart_Hal.h */
void Uart_Hal_DeInit(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    /* Disable UART NVIC interrupt. */
    Core_Hal_DisableIrq(Uart_IrqId[Instance]);

    /* disable uart clock */
    Rcm_Hal_SetResetState(Uart_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(Uart_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    (void)Ckgen_Hal_EnablePeriphClk(Uart_HalBusClock[Instance], FALSE);

    Core_Hal_ClearPendingIrq(Uart_IrqId[Instance]);
    /*PRQA S 2842 -- */

    ChannelState->TxComplete = FALSE;
    ChannelState->RxComplete = FALSE;
    ChannelState->InitState = FALSE;
}

/* Implementation of Uart_Hal_SendData declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_SendData(uint8 Instance, const uint8 *TxBuff, uint32 TxSize)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Hal_StatusType RetVal = STATUS_SUCCESS;
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check it's not busy transmitting data from a previous function call */
    if (TRUE == ChannelState->IsTxBusy)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        DEVICE_ASSERT((UART_USING_INTERRUPTS == ChannelState->TransferType) \
                      || (UART_USING_DMA == ChannelState->TransferType));

        /* Indicates this is a non-blocking transaction. */
        ChannelState->IsTxBlocking = FALSE;
        ChannelState->IsTxBusy = TRUE;

        if (UART_USING_INTERRUPTS == ChannelState->TransferType)
        {
            /* Start the transmission process using interrupts */
            RetVal = Uart_Hal_StartSendDataUsingInt(Instance, TxBuff, TxSize);
        }
        else
        {
            /* Start the transmission process using DMA */
            RetVal = Uart_Hal_StartSendDataUsingDma(Instance, TxBuff, TxSize);
        }
    }

    return RetVal;
}

#ifndef UART_SDK_NON_EXTENDED_API
/* Implementation of Uart_Hal_SendDataPolling declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_SendDataPolling(uint8 Instance, const uint8 *TxBuff, uint32 TxSize)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    const UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */
    Hal_StatusType RetVal = STATUS_SUCCESS;

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check the validity of the parameters */
    DEVICE_ASSERT((UART_7_BITS_PER_CHAR == ChannelState->BitCountPerChar) \
                  || (UART_8_BITS_PER_CHAR == ChannelState->BitCountPerChar) \
                  || (0U == (TxSize & 1U)));

    /* Check driver is not busy transmitting data from a previous asynchronous call */
    if (TRUE == ChannelState->IsTxBusy)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        ChannelState->TxSize = TxSize;
        ChannelState->TxBuff = TxBuff;
        while (ChannelState->TxSize > 0U)
        {
            while (FALSE == Uart_Reg_GetStatusFlag(BaseAddress, UART_TX_DATA_NOT_FULL))
            {
                /* Do nothing */
            }

            Uart_Hal_PutData(Instance);

            if ((UART_7_BITS_PER_CHAR == ChannelState->BitCountPerChar) \
                    || (UART_8_BITS_PER_CHAR == ChannelState->BitCountPerChar))
            {
                ++ChannelState->TxBuff;
                --ChannelState->TxSize;
            }
            else
            {
                /*PRQA S 0488 ++ # AMCE_UART_0488.*/
                ChannelState->TxBuff += 2U;
                /*PRQA S 0488 -- */
                ChannelState->TxSize -= 2U;
            }
        }

        ChannelState->SendStatus = STATUS_SUCCESS;
    }

    return RetVal;
}

/* Implementation of Uart_Hal_SendDataBlocking declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_SendDataBlocking(uint8 Instance, const uint8 *TxBuff, uint32 TxSize, uint32 TimeoutUs)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);

    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */
    uint32 Counter;
    uint32 Ticks;
    uint32 Elapse = 0U;
    boolean IsTimeOut = FALSE;
    Hal_StatusType RetVal = STATUS_SUCCESS;

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Indicates this is a blocking transaction. */
    ChannelState->IsTxBlocking = TRUE;
    ChannelState->TxComplete = FALSE;

    DEVICE_ASSERT((UART_USING_INTERRUPTS == ChannelState->TransferType) \
                  || (UART_USING_DMA == ChannelState->TransferType));

    if (UART_USING_INTERRUPTS == ChannelState->TransferType)
    {
        /* Start the transmission process using interrupts */
        RetVal = Uart_Hal_StartSendDataUsingInt(Instance, TxBuff, TxSize);
    }
    else
    {
        /* Start the transmission process using DMA */
        RetVal = Uart_Hal_StartSendDataUsingDma(Instance, TxBuff, TxSize);
    }

    if (STATUS_SUCCESS == RetVal)
    {
        /* Wait until the transmit is complete. */
        Uart_Hal_StartTimeout(&Counter, &Ticks, TimeoutUs);

        while ((FALSE == ChannelState->TxComplete) && (FALSE == IsTimeOut))
        {
            IsTimeOut = Uart_Hal_CheckTimeout(&Counter, &Elapse, Ticks);
        }

        /* Finish the transmission if timeout expired */
        if (TRUE == IsTimeOut)
        {
            ChannelState->IsTxBlocking = FALSE;
            ChannelState->SendStatus = STATUS_TIMEOUT;

            if (UART_USING_INTERRUPTS == ChannelState->TransferType)
            {
                Uart_Hal_CompleteSendDataUsingInt(Instance);
            }
            else
            {
                Uart_Hal_StopTxDma(Instance);
            }
        }
    }

    return ChannelState->SendStatus;
}
#endif

/* Implementation of Uart_Hal_GetSendStatus declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_GetSendStatus(uint8 Instance, uint32 *BytesRemaining)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    const Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    if (BytesRemaining != NULL_PTR)
    {
        if (TRUE == ChannelState->IsTxBusy)
        {
            /* Fill in the bytes not transferred yet. */
            if (UART_USING_INTERRUPTS == ChannelState->TransferType)
            {
                /* In interrupt communication, the remaining bytes are
                   retrieved from the state structure */
                *BytesRemaining = ChannelState->TxSize;
            }
            else
            {
                /* In DMA communication, the remaining bytes are retrieved
                   from the current DMA channel */
                *BytesRemaining = ChannelState->TxSize - Dma_Hal_GetTransBytes(ChannelState->TxDmaChannel);
            }
        }
        else
        {
            *BytesRemaining = 0U;
        }
    }

    return ChannelState->SendStatus;
}

/* Implementation of Uart_Hal_AbortSendingData declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_AbortSendingData(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check if a transfer is running. */
    if (TRUE == ChannelState->IsTxBusy)
    {
        /* Update the tx status */
        ChannelState->SendStatus = STATUS_UART_ABORTED;

        /* Stop the running transfer. */
        if (UART_USING_INTERRUPTS == ChannelState->TransferType)
        {
            Uart_Hal_CompleteSendDataUsingInt(Instance);
        }
        else
        {
            Uart_Hal_StopTxDma(Instance);
        }
    }

    return STATUS_SUCCESS;
}

/* Implementation of Uart_Hal_ReceiveData declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_ReceiveData(uint8 Instance, uint8 *RxBuff, uint32 RxSize)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Hal_StatusType RetVal = STATUS_SUCCESS;
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check it's not busy receiving data from a previous function call */
    if (TRUE == ChannelState->IsRxBusy)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        DEVICE_ASSERT((UART_USING_INTERRUPTS == ChannelState->TransferType) \
                      || (UART_USING_DMA == ChannelState->TransferType));

        /* Indicates this is a non-blocking transaction. */
        ChannelState->IsRxBlocking = FALSE;
        ChannelState->IsRxBusy = TRUE;

        if (UART_USING_INTERRUPTS == ChannelState->TransferType)
        {
            /* Start the reception process using interrupts */
            RetVal = Uart_Hal_StartReceiveDataUsingInt(Instance, RxBuff, RxSize);
        }
        else
        {
            /* Start the reception process using DMA */
            RetVal = Uart_Hal_StartReceiveDataUsingDma(Instance, RxBuff, RxSize);
        }
    }

    return RetVal;
}

#ifndef UART_SDK_NON_EXTENDED_API
/* Implementation of Uart_Hal_ReceiveDataPolling declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_ReceiveDataPolling(uint8 Instance, uint8 *RxBuff, uint32 RxSize)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    uint8 TmpByte = 0U;
    boolean IsError = FALSE;
    Hal_StatusType RetVal = STATUS_SUCCESS;
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check the validity of the parameters */
    DEVICE_ASSERT((ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR) \
                  || (0U == (RxSize & 1U)));

    /* Check driver is not busy receiving data from a previous asynchronous call */
    if (TRUE == ChannelState->IsRxBusy)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        /* Enable the UART receiver */
        Uart_Reg_SetReceiverCmd(BaseAddress, TRUE);
        ChannelState->RxSize = RxSize;
        ChannelState->RxBuff = RxBuff;

        while (ChannelState->RxSize > 0U)
        {
            while (FALSE == Uart_Reg_GetStatusFlag(BaseAddress, UART_RX_DATA_READY))
            {
                /* Do nothing */
            }

            Uart_Hal_GetData(Instance);

            if ((UART_7_BITS_PER_CHAR == ChannelState->BitCountPerChar) \
                    || (UART_8_BITS_PER_CHAR == ChannelState->BitCountPerChar))
            {
                ++ChannelState->RxBuff;
                --ChannelState->RxSize;
            }
            else
            {
                /*PRQA S 0488 ++ # AMCE_UART_0488.*/
                ChannelState->RxBuff += 2U;
                /*PRQA S 0488 -- */
                ChannelState->RxSize -= 2U;
            }

            /* Check for errors on received data */
            if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_FRAME_ERR))
            {
                RetVal = STATUS_UART_FRAMING_ERROR;
                /* Disable the UART receiver */
                Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
                /* Clear the flag */
                Uart_Reg_ClearStatusFlag(BaseAddress, UART_FRAME_ERR);
                IsError = TRUE;
            }
            else if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_NOISE_ERR))
            {
                RetVal = STATUS_UART_NOISE_ERROR;
                /* Disable the UART receiver */
                Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
                /* Clear the flag */
                Uart_Reg_ClearStatusFlag(BaseAddress, UART_NOISE_ERR);
                IsError = TRUE;
            }
            else if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_PARITY_ERR))
            {
                RetVal = STATUS_UART_PARITY_ERROR;
                /* Disable the UART receiver */
                Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
                /* Clear the flag */
                Uart_Reg_ClearStatusFlag(BaseAddress, UART_PARITY_ERR);
                IsError = TRUE;
            }
            else if (TRUE == Uart_Reg_GetStatusFlag(BaseAddress, UART_RX_OVERRUN))
            {
                RetVal = STATUS_UART_RX_OVERRUN;
                /* Disable the UART receiver */
                Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
                /* Clear the flag */
                Uart_Reg_ClearStatusFlag(BaseAddress, UART_RX_OVERRUN);
                IsError = TRUE;
            }
            else
            {
                /* No error condition - avoid MISRA violation */
            }

            if (TRUE == IsError)
            {
                break;
            }
        }

        ChannelState->ReceiveStatus = RetVal;

        if (STATUS_SUCCESS == RetVal)
        {
            /* Disable the UART receiver */
            Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
        }

        /* Read dummy to clear RX data ready flag */
        Uart_Reg_GetChar(BaseAddress, &TmpByte);
    }

    return RetVal;
}

/* Implementation of Uart_Hal_ReceiveDataBlocking declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_ReceiveDataBlocking(uint8 Instance, uint8 *RxBuff, uint32 RxSize, uint32 TimeoutUs)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */
    uint32 Counter;
    uint32 Ticks;
    uint32 Elapse = 0U;
    boolean IsTimeOut = FALSE;
    Hal_StatusType RetVal = STATUS_SUCCESS;

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Indicates this is a blocking transaction. */
    ChannelState->IsRxBlocking = TRUE;
    ChannelState->RxComplete = FALSE;

    DEVICE_ASSERT((UART_USING_INTERRUPTS == ChannelState->TransferType) \
                  || (UART_USING_DMA == ChannelState->TransferType));

    if (UART_USING_INTERRUPTS == ChannelState->TransferType)
    {
        /* Start the reception process using interrupts */
        RetVal = Uart_Hal_StartReceiveDataUsingInt(Instance, RxBuff, RxSize);
    }
    else
    {
        /* Start the reception process using DMA */
        RetVal = Uart_Hal_StartReceiveDataUsingDma(Instance, RxBuff, RxSize);
    }

    if (STATUS_SUCCESS == RetVal)
    {
        /* Wait until the receive is complete. */
        Uart_Hal_StartTimeout(&Counter, &Ticks, TimeoutUs);

        while ((FALSE == ChannelState->RxComplete) && (FALSE == IsTimeOut))
        {
            IsTimeOut = Uart_Hal_CheckTimeout(&Counter, &Elapse, Ticks);
        }

        /* Finish the reception if timeout expired */
        if (TRUE == IsTimeOut)
        {
            ChannelState->IsRxBlocking = FALSE;
            ChannelState->ReceiveStatus = STATUS_TIMEOUT;

            if (UART_USING_INTERRUPTS == ChannelState->TransferType)
            {
                Uart_Hal_CompleteReceiveDataUsingInt(Instance);
            }
            else
            {
                Uart_Hal_StopRxDma(Instance);
            }
        }
    }

    return ChannelState->ReceiveStatus;
}
#endif

/* Implementation of Uart_Hal_GetReceiveStatus declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_GetReceiveStatus(uint8 Instance, uint32 *BytesRemaining)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    const Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    if (BytesRemaining != NULL_PTR)
    {
        if (TRUE == ChannelState->IsRxBusy)
        {
            /* Fill in the bytes transferred. */
            if (UART_USING_INTERRUPTS == ChannelState->TransferType)
            {
                /* In interrupt-based communication, the remaining bytes are retrieved
                   from the state structure */
                *BytesRemaining = ChannelState->RxSize;
            }
            else
            {
                /* In DMA communication, the remaining bytes are retrieved
                   from the current DMA channel */
                *BytesRemaining = ChannelState->RxSize - Dma_Hal_GetTransBytes(ChannelState->RxDmaChannel);
            }
        }
        else
        {
            *BytesRemaining = 0;
        }
    }

    return ChannelState->ReceiveStatus;
}

/* Implementation of Uart_Hal_AbortReceivingData declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_AbortReceivingData(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */

    DEVICE_ASSERT(TRUE == ChannelState->InitState);

    /* Check if a transfer is running. */
    if (TRUE == ChannelState->IsRxBusy)
    {
        /* Update the rx status */
        ChannelState->ReceiveStatus = STATUS_UART_ABORTED;

        /* Stop the running transfer. */
        if (UART_USING_INTERRUPTS == ChannelState->TransferType)
        {
            Uart_Hal_CompleteReceiveDataUsingInt(Instance);
        }
        else
        {
            Uart_Hal_StopRxDma(Instance);
        }
    }

    return STATUS_SUCCESS;
}

/* Implementation of Uart_Hal_SetTxBuffer declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_SetTxBuffer(uint8 Instance, const uint8 *TxBuff, uint32 TxSize)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Hal_StatusType RetVal = STATUS_UNSUPPORTED;

    if ((TxBuff != NULL_PTR) && (TxSize > 0U))
    {
        Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
        /*PRQA S 2842 -- */
        ChannelState->TxBuff = TxBuff;
        ChannelState->TxSize = TxSize;
        RetVal = STATUS_SUCCESS;
    }

    return RetVal;
}

/* Implementation of Uart_Hal_SetRxBuffer declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_SetRxBuffer(uint8 Instance, uint8 *RxBuff, uint32 RxSize)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Hal_StatusType RetVal = STATUS_UNSUPPORTED;

    if ((RxBuff != NULL_PTR) && (RxSize > 0U))
    {
        Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
        /*PRQA S 2842 -- */
        ChannelState->RxBuff = RxBuff;
        ChannelState->RxSize = RxSize;
        RetVal = STATUS_SUCCESS;
    }

    return RetVal;
}

/*PRQA S 1505 ++ */ /* external function in sdk project */
/* Implementation of Uart_Hal_SetBaudRate declared in Uart_Hal.h */
Hal_StatusType Uart_Hal_SetBaudRate(uint8 Instance, uint32 DesiredBaudRate, Uart_SampleCntType SampleCnt)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(DesiredBaudRate > 0U);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    uint32 SampleCntValue = UART_SAMPLE_CNT_16_VALUE;
    uint32 UartSourceClock;
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    float32 Divisor;
    Uart_ChannelStateType const *ChannelState = Uart_ChannelStateArray[Instance];
    Hal_StatusType RetVal = STATUS_SUCCESS;/* function return value */

    /* Check if there is an ongoing transfer */
    if ((ChannelState->IsTxBusy == TRUE) || (ChannelState->IsRxBusy == TRUE))
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        (void)Ckgen_Hal_GetFreq(Uart_HalClock[Instance], &UartSourceClock);
        /*PRQA S 2842 -- */

        /* Check if current instance is clock gated off. */
        DEVICE_ASSERT(UartSourceClock > 0U);

        /* Determine sampleCntValue based on sampleCntReg */
        switch (SampleCnt)
        {
        case UART_SMP_CNT4:
            SampleCntValue = UART_SAMPLE_CNT_4_VALUE;
            break;
        case UART_SMP_CNT8:
            SampleCntValue = UART_SAMPLE_CNT_8_VALUE;
            break;
        case UART_SMP_CNT16:
            SampleCntValue = UART_SAMPLE_CNT_16_VALUE;
            break;
        default:
            SampleCntValue = UART_SAMPLE_CNT_16_VALUE;
            break;
        }

        /* Check if the desired baud rate can be configured with the current protocol clock. */
        DEVICE_ASSERT(UartSourceClock >= (DesiredBaudRate * SampleCntValue));

        /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        Divisor = ((float32)UartSourceClock / (float32)(DesiredBaudRate * SampleCntValue));
        /*PRQA S 4394 -- */

        /* program the sampleCntReg value */
        Uart_Reg_SetSampleCounter(BaseAddress, (uint32)SampleCnt);

        /* write the divisor value */
        Uart_Reg_SetBaudRateDivisor(BaseAddress, Divisor);
    }

    return RetVal;
}
/*PRQA S 1505 -- */ /* external function in sdk project */

#ifndef UART_SDK_NON_EXTENDED_API
/* Implementation of Uart_Hal_GetBaudRate declared in Uart_Hal.h */
void Uart_Hal_GetBaudRate(uint8 Instance, uint32 *ConfiguredBaudRate)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(ConfiguredBaudRate != NULL_PTR);
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    DEVICE_ASSERT(Uart_ChannelStateArray[Instance] != NULL_PTR);

    Uart_SampleCntType SampleCntReg = UART_SMP_CNT4;
    uint32 SampleCntValue = UART_SAMPLE_CNT_16_VALUE;
    float32 Divisor = 0.0F;
    uint32 UartSourceClock = 0U;
    const UART_Type *BaseAddress = Uart_HalBase[Instance];

    /* Get the UART clock */
    (void)Ckgen_Hal_GetFreq(Uart_HalClock[Instance], &UartSourceClock);
    /*PRQA S 2842 -- */

    /*PRQA S 4342 ++ # AMCE_UART_4342.*/
    SampleCntReg = (Uart_SampleCntType)Uart_Reg_GetSampleCounter(BaseAddress);
    /*PRQA S 4342 -- */
    /* Determine sampleCntValue based on sampleCntReg */
    switch (SampleCntReg)
    {
    case UART_SMP_CNT4:
        SampleCntValue = UART_SAMPLE_CNT_4_VALUE;
        break;
    case UART_SMP_CNT8:
        SampleCntValue = UART_SAMPLE_CNT_8_VALUE;
        break;
    case UART_SMP_CNT16:
        SampleCntValue = UART_SAMPLE_CNT_16_VALUE;
        break;
    default:
        SampleCntValue = UART_SAMPLE_CNT_4_VALUE;
        break;
    }
    Divisor = Uart_Reg_GetBaudRateDivisor(BaseAddress);

    if (ConfiguredBaudRate != NULL_PTR)
    {
        /*PRQA S 4395 ++ # AMCE_UART_4395.*/
        *ConfiguredBaudRate = (uint32)((float32)UartSourceClock / ((float32)SampleCntValue * Divisor));
        /*PRQA S 4395 -- */
    }
}
#endif

/* Implementation of Uart_Hal_SetIdleInterrupt declared in Uart_Hal.h */
void Uart_Hal_SetIdleInterrupt(uint8 Instance, boolean IsEnable)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- */

    Uart_Reg_SetIdleInterrupt(BaseAddress, IsEnable);
}

#ifndef UART_SDK_NON_EXTENDED_API
/* Implementation of Uart_Hal_SetDataMatch declared in Uart_Hal.h */
void Uart_Hal_SetDataMatch(uint8 Instance, uint16 Data, boolean IsEnable)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- */

    Uart_Reg_SetDataMatch(BaseAddress, Data, IsEnable);
}

/* Implementation of Uart_Hal_SetAddrFilter declared in Uart_Hal.h */
void Uart_Hal_SetAddrFilter(uint8 Instance, uint16 Addr, boolean IsEnable)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- */

    Uart_Reg_SetAddrFilter(BaseAddress, Addr, IsEnable);
}

/* Implementation of Uart_Hal_SetMatchInterrupt declared in Uart_Hal.h */
void Uart_Hal_SetMatchInterrupt(uint8 Instance, boolean IsEnable)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- */

    Uart_Reg_SetIntMode(BaseAddress, UART_INT_DATA_MATCH, IsEnable);
}

/* Implementation of Uart_Hal_SetCtsRts declared in Uart_Hal.h */
void Uart_Hal_SetCtsRts(uint8 Instance, Uart_RtsCtsType RtsCts)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- */

    Uart_Reg_SetCtsRts(BaseAddress, RtsCts);
}

/* Implementation of Uart_Hal_SetRS485 declared in Uart_Hal.h */
void Uart_Hal_SetRS485(uint8 Instance, const Uart_RS485ConfigType *Config)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- */

    /*PRQA S 2812 ++ # AMCE_UART_2812.*/
    Uart_Reg_SetRS485(BaseAddress, Config->Enable);
    /*PRQA S 2812 -- */
    Uart_Reg_SetRS485Invpol(BaseAddress, Config->InvpolEnable);
    Uart_Reg_SetRS485Delay(BaseAddress, Config->DelayEnable);
    Uart_Reg_SetRS485DelayCnt(BaseAddress, Config->DelayCnt);
    Uart_Reg_SetRS485GuardTime(BaseAddress, Config->GuardTime);
    Uart_Reg_SetRS485GuardEnable(BaseAddress, (Config->GuardTime > 0U) ? TRUE : FALSE);
}

/* Implementation of Uart_Hal_SetIrDA declared in Uart_Hal.h */
void Uart_Hal_SetIrDA(uint8 Instance, const Uart_IrDAConfigType *Config)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- */
    boolean TxEnable;
    boolean RxEnable;

    Uart_Reg_SetIrDA(BaseAddress, Config->Enable);
    Uart_Reg_SetIrDARxWidth(BaseAddress, Config->RxWidth);
    Uart_Reg_SetIrDATxWidth(BaseAddress, Config->TxWidth);

    /* Determine TX/RX enable states based on IrDA mode */
    if (TRUE == Config->Enable)
    {
        TxEnable = Config->TxMode;
        RxEnable = (FALSE == Config->TxMode) ? TRUE : FALSE;
    }
    else
    {
        TxEnable = TRUE;
        RxEnable = TRUE;
    }

    /* Set both transmitter and receiver in one operation sequence */
    Uart_Reg_SetTransmitterCmd(BaseAddress, TxEnable);
    Uart_Reg_SetReceiverCmd(BaseAddress, RxEnable);
}
#endif

#if (CONFIG_UART0_ENABLE)
/**
 * @brief UART0 interrupt service routine.
 *
 * @details
 * This ISR handles all UART0 interrupts including transmit, receive, error,
 * data match, and idle line events. It dispatches to the main UART interrupt
 * handler which processes the specific interrupt sources.
 *
 * @return void
 *
 * @pre UART0 must be initialized and interrupts enabled.
 *
 * @post Appropriate interrupt handlers called and flags cleared.
 *
 * @note
 *     This is an interrupt service routine.
 *     Must not be called directly by application code.
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_UART_API_021
 */
ISR(UART0_IRQHandler)
{
    Uart_Hal_IRQHandler(0U);
}
#endif

#if (CONFIG_UART1_ENABLE)
/**
 * @brief UART1 interrupt service routine.
 *
 * @details
 * This ISR handles all UART1 interrupts including transmit, receive, error,
 * data match, and idle line events. It dispatches to the main UART interrupt
 * handler which processes the specific interrupt sources.
 *
 * @return void
 *
 * @pre UART1 must be initialized and interrupts enabled.
 *
 * @post Appropriate interrupt handlers called and flags cleared.
 *
 * @note
 *     This is an interrupt service routine.
 *     Must not be called directly by application code.
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_UART_API_022
 */
ISR(UART1_IRQHandler)
{
    Uart_Hal_IRQHandler(1U);
}
#endif

#if (CONFIG_UART2_ENABLE)
/**
 * @brief UART2 interrupt service routine.
 *
 * @details
 * This ISR handles all UART2 interrupts including transmit, receive, error,
 * data match, and idle line events. It dispatches to the main UART interrupt
 * handler which processes the specific interrupt sources.
 *
 * @return void
 *
 * @pre UART2 must be initialized and interrupts enabled.
 *
 * @post Appropriate interrupt handlers called and flags cleared.
 *
 * @note
 *     This is an interrupt service routine.
 *     Must not be called directly by application code.
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_UART_API_023
 */
ISR(UART2_IRQHandler)
{
    Uart_Hal_IRQHandler(2U);
}
#endif

#if !defined(AC7840E)
#if (CONFIG_UART3_ENABLE)
/**
 * @brief UART3 interrupt service routine.
 *
 * @details
 * This ISR handles all UART3 interrupts including transmit, receive, error,
 * data match, and idle line events. It dispatches to the main UART interrupt
 * handler which processes the specific interrupt sources.
 *
 * @return void
 *
 * @pre UART3 must be initialized and interrupts enabled.
 *
 * @post Appropriate interrupt handlers called and flags cleared.
 *
 * @note
 *     This is an interrupt service routine.
 *     Must not be called directly by application code.
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_UART_API_024
 */
ISR(UART3_IRQHandler)
{
    Uart_Hal_IRQHandler(3U);
}
#endif
#endif

#if defined (AC7843X)
#if (CONFIG_UART4_ENABLE)
/**
 * @brief UART4 interrupt service routine.
 *
 * @details
 * This ISR handles all UART4 interrupts including transmit, receive, error,
 * data match, and idle line events. It dispatches to the main UART interrupt
 * handler which processes the specific interrupt sources.
 *
 * @return void
 *
 * @pre UART4 must be initialized and interrupts enabled.
 *
 * @post Appropriate interrupt handlers called and flags cleared.
 *
 * @note
 *     This is an interrupt service routine.
 *     Must not be called directly by application code.
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_UART_API_025
 */
ISR(UART4_IRQHandler)
{
    Uart_Hal_IRQHandler(4U);
}
#endif

#if (CONFIG_UART5_ENABLE)
/**
 * @brief UART5 interrupt service routine.
 *
 * @details
 * This ISR handles all UART5 interrupts including transmit, receive, error,
 * data match, and idle line events. It dispatches to the main UART interrupt
 * handler which processes the specific interrupt sources.
 *
 * @return void
 *
 * @pre UART5 must be initialized and interrupts enabled.
 *
 * @post Appropriate interrupt handlers called and flags cleared.
 *
 * @note
 *     This is an interrupt service routine.
 *     Must not be called directly by application code.
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_UART_API_026
 */
ISR(UART5_IRQHandler)
{
    Uart_Hal_IRQHandler(5U);
}
#endif

#if (CONFIG_UART6_ENABLE)
/**
 * @brief UART6 interrupt service routine.
 *
 * @details
 * This ISR handles all UART6 interrupts including transmit, receive, error,
 * data match, and idle line events. It dispatches to the main UART interrupt
 * handler which processes the specific interrupt sources.
 *
 * @return void
 *
 * @pre UART6 must be initialized and interrupts enabled.
 *
 * @post Appropriate interrupt handlers called and flags cleared.
 *
 * @note
 *     This is an interrupt service routine.
 *     Must not be called directly by application code.
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_UART_API_027
 */
ISR(UART6_IRQHandler)
{
    Uart_Hal_IRQHandler(6U);
}
#endif

#if (CONFIG_UART7_ENABLE)
/**
 * @brief UART7 interrupt service routine.
 *
 * @details
 * This ISR handles all UART7 interrupts including transmit, receive, error,
 * data match, and idle line events. It dispatches to the main UART interrupt
 * handler which processes the specific interrupt sources.
 *
 * @return void
 *
 * @pre UART7 must be initialized and interrupts enabled.
 *
 * @post Appropriate interrupt handlers called and flags cleared.
 *
 * @note
 *     This is an interrupt service routine.
 *     Must not be called directly by application code.
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_UART_API_028
 */
ISR(UART7_IRQHandler)
{
    Uart_Hal_IRQHandler(7U);
}
#endif
#endif
/*================================STATIC  FUNCTION  IMPLEMENTATIONS=================================*/
#ifndef UART_SDK_NON_EXTENDED_API
LOCAL_INLINE void Uart_Hal_StartTimeout(uint32 *CounterOut, uint32 *TicksOut, uint32 Timeout)
{
    *TicksOut = OsIf_MicrosToTicks(Timeout);
    *CounterOut = OsIf_GetCounter();
}

LOCAL_INLINE boolean Uart_Hal_CheckTimeout(uint32 *Counter, uint32 *ElapsedTicks, uint32 TimeoutTicks)
{
    uint32 CurrentElapsedTicks = OsIf_GetElapsed(Counter);
    *ElapsedTicks += CurrentElapsedTicks;
    return ((*ElapsedTicks >= TimeoutTicks) ? TRUE : FALSE);
}
#endif

static Hal_StatusType Uart_Hal_StartSendDataUsingInt(uint8 Instance, const uint8 *TxBuffer, uint32 TxSize)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    DEVICE_ASSERT((ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR) \
                  || (0U == (TxSize & 1U)));

    ChannelState->TxBuff = TxBuffer;
    ChannelState->TxSize = TxSize;
    ChannelState->SendStatus = STATUS_BUSY;

    /* Enable tx not full interrupt */
    Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_NOT_FULL, TRUE);

    return STATUS_SUCCESS;
}

static void Uart_Hal_StopTxDma(uint8 Instance)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    /* Disable tx DMA requests for the current instance */
    Uart_Reg_SetTxDmaCmd(BaseAddress, FALSE);
    /* Stop the dma channel */
    (void)Dma_Hal_StopCh(ChannelState->TxDmaChannel);
    /* Disable transmit complete interrupt */
    Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_COMPLETE, FALSE);
    /* current transmission status is STATUS_BUSY */
    if (STATUS_BUSY == ChannelState->SendStatus)
    {
        /* If the transfer is completed, update the transmit status */
        ChannelState->SendStatus = STATUS_SUCCESS;
    }
    /* Update the internal busy flag */
    ChannelState->IsTxBusy = FALSE;
    ChannelState->TxSize = 0U;

    if (TRUE == ChannelState->IsTxBlocking) /* Mark the transmission as complete */
    {
        ChannelState->TxComplete = TRUE;
    }
}

static void Uart_Hal_CompleteSendDataUsingInt(uint8 Instance)
{
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    UART_Type *BaseAddress = Uart_HalBase[Instance];

    if (STATUS_BUSY == ChannelState->SendStatus)
    {
        /* If the transfer is completed, update the transmit status */
        ChannelState->SendStatus = STATUS_SUCCESS;
    }
    else
    {
        /* If the transfer is aborted or timed out, disable tx not full interrupt */
        Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_NOT_FULL, FALSE);
    }
    /* Disable transmit complete interrupt */
    Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_COMPLETE, FALSE);

    /* Update the internal busy flag */
    ChannelState->IsTxBusy = FALSE;

    if (TRUE == ChannelState->IsTxBlocking) /* Mark the transmission as complete */
    {
        ChannelState->TxComplete = TRUE;
    }
}

static void Uart_Hal_TxCompleteIrqHandler(uint8 Instance)
{
    Uart_ChannelStateType const *ChannelState = Uart_ChannelStateArray[Instance];

    if (0U == ChannelState->TxSize)/* current transmission size is 0, means transaction complete */
    {
        if (UART_USING_INTERRUPTS == ChannelState->TransferType)/* Complete the interrupt based transfer */
        {
            Uart_Hal_CompleteSendDataUsingInt(Instance);
        }
        else/* Complete the DMA based transfer */
        {
            Uart_Hal_StopTxDma(Instance);
        }
        if (NULL_PTR != ChannelState->TxCallback)/* Invoke callback if there is one */
        {
            ChannelState->TxCallback(Instance, UART_EVENT_END_TRANSFER);
        }
    }
}

/*PRQA S 3673 ++ # AMCE_UART_3673.*/
static void Uart_Hal_TxDmaCallback(void *Param)
/*PRQA S 3673 -- .*/
{
    /*PRQA S 0316 ++ # AMCE_QAC_0316.*/
    const Dma_ChannelCBInfoType *DmaInfo = (const Dma_ChannelCBInfoType *)Param;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # AMCE_UART_0326.*/
    uint32 Instance = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */

    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    if (0U != (DmaInfo->DmaEvent & DMA_ERROR_EVENT))/* Check if the DMA transfer completed with errors */
    {
        /* Update the status */
        ChannelState->SendStatus = STATUS_ERROR;
        /* Stop the transfer */
        Uart_Hal_StopTxDma((uint8)Instance);
        /* Notify the application that an error occurred */
        if (ChannelState->TxCallback != NULL_PTR)
        {
            ChannelState->TxCallback((uint8)Instance, UART_EVENT_ERROR);
        }
    }
    else if (0U != (DmaInfo->DmaEvent & DMA_FINISH_EVENT))/* DMA transfer completed without errors */
    {
        ChannelState->TxSize = 0U;

        /* Invoke callback if there is one */
        if (ChannelState->TxCallback != NULL_PTR)
        {
            /* Allow the user to provide a new buffer, for continuous transmission */
            ChannelState->TxCallback((uint8)Instance, UART_EVENT_TX_EMPTY);
        }
        /* If the callback has updated the tx buffer, restart DMA for continuous transmission */
        if (ChannelState->TxSize > 0U)
        {
            Dma_TransferUnitType DmaTransferSize;
            Dma_ChannelAddrType DmaAddr;
            if (UART_9_BITS_PER_CHAR == ChannelState->BitCountPerChar)
            {
                DmaTransferSize = DMA_TRANSFER_UNIT_2B;
            }
            else
            {
                DmaTransferSize = DMA_TRANSFER_UNIT_1B;
            }
            DmaAddr.Length = (uint16)ChannelState->TxSize;
            DmaAddr.SrcOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
            DmaAddr.DestOffset = 0U;
            DmaAddr.SrcStartAddr = (uint32)ChannelState->TxBuff;
            /*PRQA S 2842 ++ # AMCE_UART_2842.*/
            DmaAddr.DestStartAddr = (uint32)(&(Uart_HalBase[Instance]->RBR));
            /*PRQA S 2842 -- */
            /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
            DmaAddr.SrcEndAddr = DmaAddr.SrcStartAddr + \
                                (DmaAddr.Length / (1UL << (uint8)DmaTransferSize) * \
                                 DmaAddr.SrcOffset);
            DmaAddr.DestEndAddr = DmaAddr.DestStartAddr + \
                                 (DmaAddr.Length / (1UL << (uint8)DmaTransferSize) * \
                                  DmaAddr.DestOffset);
            /*PRQA S 2985 -- */
            Dma_Hal_UpdateChAddr(ChannelState->TxDmaChannel, &DmaAddr);
            (void)Dma_Hal_StartCh(ChannelState->TxDmaChannel);
        }
        else
        {
#if defined (AC7840X)
            Uart_Hal_TxCompleteIrqHandler((uint8)Instance);
#else
            /*PRQA S 2842 ++ # AMCE_UART_2842.*/
            /* Enable the TX COMPLETE interrupt to check end of transfer */
            Uart_Reg_SetIntMode(Uart_HalBase[Instance], UART_INT_TX_COMPLETE, TRUE);
            /*PRQA S 2842 -- */
#endif
        }
    }
    else
    {
        /* do nothing */
    }
}

static Hal_StatusType Uart_Hal_StartSendDataUsingDma(uint8 Instance, const uint8 *TxBuffer, uint32 TxSize)
{
    DEVICE_ASSERT(TxSize < UART_USE_DMA_TRANSMIT_LEN_MAX);

    Hal_StatusType DmaRetValue;/* config dma status */
    Hal_StatusType RetValue;/* function return value */
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    Dma_TransferUnitType DmaTransferSize;
    Dma_TransferConfigType TransferConfig;
    uint32 UartInstance;

    /* Check it's not busy transmitting data from a previous function call */
    /* Enable tx DMA requests */
    Uart_Reg_SetTxDmaCmd(BaseAddress, TRUE);

    /* Update state structure */
    ChannelState->TxBuff = TxBuffer;
    ChannelState->TxSize = TxSize;
    ChannelState->IsTxBusy = TRUE;
    ChannelState->SendStatus = STATUS_BUSY;

    /* Set DMA channel transfer size is DMA_TRANSFER_UNIT_2B*/
    if (UART_9_BITS_PER_CHAR == ChannelState->BitCountPerChar)
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_2B;
    }
    else/* Set DMA channel transfer size is DMA_TRANSFER_UNIT_1B*/
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_1B;
    }

    TransferConfig.TriggerMode = FALSE;
    TransferConfig.CircularMode = FALSE;
#if defined (AC7840E)
    TransferConfig.DaisyMode = FALSE;
#endif
    TransferConfig.SrcUnit = DmaTransferSize;
    TransferConfig.DestUnit = DmaTransferSize;
    TransferConfig.Type = DMA_TRANSFER_MEM2PERIPH;
    TransferConfig.SrcStartAddr = (uint32)TxBuffer;
    TransferConfig.DestStartAddr = (uint32)(&(BaseAddress->RBR));
    TransferConfig.SrcOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
    TransferConfig.DestOffset = 0U;
    TransferConfig.Length = (uint16)TxSize;
    TransferConfig.Callback = (Hal_CallbackType)Uart_Hal_TxDmaCallback;
    TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
    UartInstance = Instance;
    /*PRQA S 0326 ++ # AMCE_UART_0326.*/
    TransferConfig.UserArgs = (void *)(UartInstance);
    /*PRQA S 0326 -- */
    /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
    TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + \
                                (TransferConfig.Length / (1UL << (uint8)TransferConfig.SrcUnit) * \
                                 TransferConfig.SrcOffset);
    TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + \
                                 (TransferConfig.Length / (1UL << (uint8)TransferConfig.DestUnit) * \
                                  TransferConfig.DestOffset);
    /*PRQA S 2985 -- */
    /* Configure the DMA transfer control */
    DmaRetValue = Dma_Hal_ConfigCh(ChannelState->TxDmaChannel, &TransferConfig);
    if (STATUS_SUCCESS == DmaRetValue)/* config dma success */
    {
        /* Start the DMA channel */
        (void)Dma_Hal_StartCh(ChannelState->TxDmaChannel);

        /* Enable the UART transmitter */
        Uart_Reg_SetTransmitterCmd(BaseAddress, TRUE);
        RetValue = STATUS_SUCCESS;
    }
    else/* config dma fail */
    {
        RetValue = STATUS_ERROR;
    }

    return RetValue;
}

static Hal_StatusType Uart_Hal_StartReceiveDataUsingInt
(
    uint8 Instance,
    uint8 *RxBuffer,
    uint32 RxSize
)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    DEVICE_ASSERT((ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR) \
                  || (0U == (RxSize & 1U)));

    /* Initialize the module driver state struct to indicate transfer in progress
    * and with the buffer and byte count data. */
    ChannelState->RxBuff = RxBuffer;
    ChannelState->RxSize = RxSize;
    ChannelState->ReceiveStatus = STATUS_BUSY;
    /* Enable the receiver */
    Uart_Reg_SetReceiverCmd(BaseAddress, TRUE);
    /* Enable error interrupts TRUE */
    Uart_Reg_SetErrorInterrupts(BaseAddress, TRUE);
    /* Enable receive data not empty interrupt */
    Uart_Reg_SetIntMode(BaseAddress, UART_INT_RX_NOT_EMPTY, TRUE);

    return STATUS_SUCCESS;
}

static void Uart_Hal_StopRxDma
(
    uint8 Instance
)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    uint8 TmpByte = 0U;

    /* Disable receiver */
    Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
    /* Disable error interrupts */
    Uart_Reg_SetErrorInterrupts(BaseAddress, FALSE);
    /* Disable rx DMA requests */
    Uart_Reg_SetRxDmaCmd(BaseAddress, FALSE);
    /* Read dummy to clear RX data ready flag */
    Uart_Reg_GetChar(BaseAddress, &TmpByte);
    /* Read dummy to clear RX data ready flag */
    Uart_Reg_GetChar(BaseAddress, &TmpByte);
    /* Stop the DMA channel */
    (void)Dma_Hal_StopCh(ChannelState->RxDmaChannel);

    /* Signal the synchronous completion object. */
    if (TRUE == ChannelState->IsRxBlocking)/* modify variable IsRxBlocking */
    {
        ChannelState->RxComplete = TRUE;
        ChannelState->IsRxBlocking = FALSE;
    }

    /* If status is STATUS_BUSY, update the internal driver status */
    if (STATUS_BUSY == ChannelState->ReceiveStatus)
    {
        ChannelState->ReceiveStatus = STATUS_SUCCESS;
    }

    /* Update the information of the module driver state */
    ChannelState->IsRxBusy = FALSE;
    ChannelState->RxSize = 0U;
}

/*PRQA S 3673 ++ # AMCE_UART_3673.*/
static void Uart_Hal_RxDmaCallback(void *Param)
/*PRQA S 3673 -- */
{
    /*PRQA S 0316 ++ # AMCE_QAC_0316.*/
    const Dma_ChannelCBInfoType *DmaInfo = (const Dma_ChannelCBInfoType *)Param;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # AMCE_UART_0326.*/
    uint32 Instance = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */

    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    /*PRQA S 2842 -- */

    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    if (0U != (DmaInfo->DmaEvent & DMA_ERROR_EVENT))/* current status is error */
    {
        /* Update the status */
        ChannelState->ReceiveStatus = STATUS_ERROR;
        /* Stop the transfer */
        Uart_Hal_StopRxDma((uint8)Instance);
        /* Notify the application that an error occurred */
        if (NULL_PTR != ChannelState->RxCallback)/* RxCallback not null pointer, callback to user */
        {
            ChannelState->RxCallback((uint8)Instance, UART_EVENT_ERROR);
        }
        /* Clear the flags */
        Uart_Reg_ClearErrorFlags(BaseAddress, UART_ALL_ERR_FLAG);
    }

    /* Return if an error occurred; error cases are treated by the interrupt handler */
    if ((0U != (DmaInfo->DmaEvent & DMA_FINISH_EVENT)) && (ChannelState->ReceiveStatus == STATUS_BUSY))
    {
        ChannelState->RxSize = 0U;
        /* Invoke callback to allow user to provide a new buffer for continuous reception */
        if (NULL_PTR != ChannelState->RxCallback)
        {
            ChannelState->RxCallback((uint8)Instance, UART_EVENT_RX_FULL);
        }

        if (0U == ChannelState->RxSize)
        {
            /* Stop the reception */
            Uart_Hal_StopRxDma((uint8)Instance);
            /* Invoke the callback to notify the end of the transfer */
            if (NULL_PTR != ChannelState->RxCallback)
            {
                ChannelState->RxCallback((uint8)Instance, UART_EVENT_END_TRANSFER);
            }
        }
        else
        {
            /* Restart DMA with new buffer for continuous reception */
            Dma_TransferUnitType DmaTransferSize;
            Dma_ChannelAddrType DmaAddr;
            if (UART_9_BITS_PER_CHAR == ChannelState->BitCountPerChar)
            {
                DmaTransferSize = DMA_TRANSFER_UNIT_2B;
            }
            else
            {
                DmaTransferSize = DMA_TRANSFER_UNIT_1B;
            }
            DmaAddr.Length = (uint16)ChannelState->RxSize;
            DmaAddr.SrcOffset = 0U;
            DmaAddr.DestOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
            /*PRQA S 2842 ++ # AMCE_UART_2842.*/
            DmaAddr.SrcStartAddr = (uint32)(&(BaseAddress->RBR));
            /*PRQA S 2842 -- */
            DmaAddr.DestStartAddr = (uint32)ChannelState->RxBuff;
            /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
            DmaAddr.SrcEndAddr = DmaAddr.SrcStartAddr + \
                                (DmaAddr.Length / (1UL << (uint8)DmaTransferSize) * \
                                 DmaAddr.SrcOffset);
            DmaAddr.DestEndAddr = DmaAddr.DestStartAddr + \
                                 (DmaAddr.Length / (1UL << (uint8)DmaTransferSize) * \
                                  DmaAddr.DestOffset);
            /*PRQA S 2985 -- */
            Dma_Hal_UpdateChAddr(ChannelState->RxDmaChannel, &DmaAddr);
            (void)Dma_Hal_StartCh(ChannelState->RxDmaChannel);
        }
        /* Clear the flags */
        Uart_Reg_ClearErrorFlags(BaseAddress, UART_ALL_ERR_FLAG);
    }
    else/* ReceiveStatus not STATUS_BUSY */
    {
        /* do nothing */
    }

    return;
}

static Hal_StatusType Uart_Hal_StartReceiveDataUsingDma(uint8 Instance, uint8 *RxBuffer, uint32 RxSize)
{
    DEVICE_ASSERT(RxSize < UART_USE_DMA_TRANSMIT_LEN_MAX);

    Hal_StatusType RetValue;
    Hal_StatusType DmaRetValue;
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    Dma_TransferUnitType DmaTransferSize;
    uint8 TmpByte = 0U;
    uint32 UartInstance;

    /* Enable rx DMA requests for the current instance */
    Uart_Reg_SetRxDmaCmd(BaseAddress, TRUE);
    /* Read dummy to clear RX data ready flag */
    Uart_Reg_GetChar(BaseAddress, &TmpByte);
    /* Read dummy to clear RX data ready flag */
    Uart_Reg_GetChar(BaseAddress, &TmpByte);
    /* Clear the flags */
    Uart_Reg_ClearErrorFlags(BaseAddress, UART_ALL_ERR_FLAG);
    /* Set DMA channel transfer size */
    if (UART_9_BITS_PER_CHAR == ChannelState->BitCountPerChar)
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_2B;
    }
    else
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_1B;
    }
    Dma_TransferConfigType TransferConfig;
    TransferConfig.TriggerMode = FALSE;
#if defined (AC7840E)
    TransferConfig.DaisyMode = FALSE;
#endif
    TransferConfig.CircularMode = FALSE;
    TransferConfig.SrcUnit = DmaTransferSize;
    TransferConfig.DestUnit = DmaTransferSize;
    TransferConfig.Type = DMA_TRANSFER_PERIPH2MEM;
    TransferConfig.SrcStartAddr = (uint32)(&(BaseAddress->RBR));
    TransferConfig.DestStartAddr = (uint32)RxBuffer;
    TransferConfig.SrcOffset = 0U;
    TransferConfig.DestOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
    TransferConfig.Length = (uint16)RxSize;
    TransferConfig.Callback = (Hal_CallbackType)Uart_Hal_RxDmaCallback;
    TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
    UartInstance = Instance;
    /*PRQA S 0326 ++ # AMCE_UART_0326.*/
    TransferConfig.UserArgs = (void *)(UartInstance);
    /*PRQA S 0326 -- */
    /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
    TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + \
                                (TransferConfig.Length / (1UL << (uint8)TransferConfig.SrcUnit) * \
                                 TransferConfig.SrcOffset);
    TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + \
                                 (TransferConfig.Length / (1UL << (uint8)TransferConfig.DestUnit) * \
                                  TransferConfig.DestOffset);
    /*PRQA S 2985 -- */
    /* Configure the DMA transfer control */
    DmaRetValue = Dma_Hal_ConfigCh(ChannelState->RxDmaChannel, &TransferConfig);
    if (STATUS_SUCCESS == DmaRetValue)/* config dma channel is STATUS_SUCCESS */
    {
        /* Start the DMA channel */
        (void)Dma_Hal_StartCh(ChannelState->RxDmaChannel);
        /* Update the state structure */
        ChannelState->RxBuff = RxBuffer;
        ChannelState->RxSize = RxSize;
        ChannelState->ReceiveStatus = STATUS_BUSY;

        /* Enable error interrupts */
        Uart_Reg_SetErrorInterrupts(BaseAddress, TRUE);
        /* Enable the receiver */
        Uart_Reg_SetReceiverCmd(BaseAddress, TRUE);
        RetValue = STATUS_SUCCESS;
    }
    else/* config dma channel fail */
    {
        RetValue = STATUS_ERROR;
    }

    return RetValue;
}

static void Uart_Hal_CompleteReceiveDataUsingInt(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    uint8 TmpByte = 0U;
    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */

    /* Disable receiver */
    Uart_Reg_SetReceiverCmd(BaseAddress, FALSE);
    /* Disable error interrupts */
    Uart_Reg_SetErrorInterrupts(BaseAddress, FALSE);
    /* Read dummy to clear rx data ready flag */
    Uart_Reg_GetChar(BaseAddress, &TmpByte);
    /* Disable receive data not full interrupt. */
    Uart_Reg_SetIntMode(BaseAddress, UART_INT_RX_NOT_EMPTY, FALSE);
    if (TRUE == ChannelState->IsRxBlocking)/* Signal the synchronous completion object. */
    {
        ChannelState->RxComplete = TRUE;
        ChannelState->IsRxBlocking = FALSE;
    }
    /* Update the information of the module driver state */
    ChannelState->IsRxBusy = FALSE;
    if (STATUS_BUSY == ChannelState->ReceiveStatus)
    {
        ChannelState->ReceiveStatus = STATUS_SUCCESS;
    }
}

static void Uart_Hal_ErrIrqHandler(uint8 Instance, uint32 ErrFlag)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    boolean IsError = FALSE;

    /* Handle receive overrun interrupt */
    if (0U != (ErrFlag & UART_LSR0_OE_Msk))
    {
        /* Update the internal status */
        ChannelState->ReceiveStatus = STATUS_UART_RX_OVERRUN;
        IsError = TRUE;
    }
    /* Handle parity error interrupt */
    else if (0U != (ErrFlag & UART_LSR0_PE_Msk))
    {
        /* Update the internal status */
        ChannelState->ReceiveStatus = STATUS_UART_PARITY_ERROR;
        IsError = TRUE;
    }
    /* Handle framing error interrupt */
    else if (0U != (ErrFlag & UART_LSR0_FE_Msk))
    {
        /* Update the internal status */
        ChannelState->ReceiveStatus = STATUS_UART_FRAMING_ERROR;
        IsError = TRUE;
    }
    /* Handle break error interrupt */
    else if (0U != (ErrFlag & UART_LSR0_BI_Msk))
    {
        /* Update the internal status */
        ChannelState->ReceiveStatus = STATUS_UART_BREAK_ERROR;
        IsError = TRUE;
    }
    /* Handle noise error interrupt */
    else if (0U != (ErrFlag & UART_LSR0_NE_Msk))
    {
        /* Update the internal status */
        ChannelState->ReceiveStatus = STATUS_UART_NOISE_ERROR;
        IsError = TRUE;
    }
    else
    {
        /* No error condition - avoid MISRA violation */
    }

    if (TRUE == IsError)
    {
        /* Clear the flags */
        Uart_Reg_ClearErrorFlags(BaseAddress, ErrFlag);

        if (UART_USING_INTERRUPTS == ChannelState->TransferType)
        {
            /* Complete transfer (disable rx logic) */
            Uart_Hal_CompleteReceiveDataUsingInt(Instance);
        }
        else
        {
            /* Complete the transfer (stop DMA channel) */
            Uart_Hal_StopRxDma(Instance);
        }

        /* Invoke callback if there is one */
        if (NULL_PTR != ChannelState->RxCallback)
        {
            ChannelState->RxCallback(Instance, UART_EVENT_ERROR);
        }
    }
}

static void Uart_Hal_GetData(uint8 Instance)
{
    UART_Type const *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType const *ChannelState = Uart_ChannelStateArray[Instance];
    uint16 Data = 0U;
    uint8 *RxBuff = ChannelState->RxBuff;

    if ((UART_7_BITS_PER_CHAR == ChannelState->BitCountPerChar)
            || (UART_8_BITS_PER_CHAR == ChannelState->BitCountPerChar)) /* 7-bits or 8-bits */
    {
        /* Receive the data */
        Uart_Reg_GetChar(BaseAddress, RxBuff);
    }
    else /* 9-bits */
    {
        /* Receive the data */
        Uart_Reg_GetChar9(BaseAddress, &Data);

        /* Write the least significant bits to the receive buffer */
        *RxBuff = (uint8)(Data & 0xFFU);
        ++RxBuff;
        /* Write the ninth bit to the subsequent byte in the rx buffer */
        *RxBuff = (uint8)(Data >> 8U);
    }
}

static void Uart_Hal_RxIrqHandler(uint8 Instance)
{
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];

    /* Get data and put in receive buffer  */
    Uart_Hal_GetData(Instance);

    /* Update the internal state */
    if (ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR)
    {
        ++ChannelState->RxBuff;
        --ChannelState->RxSize;
    }
    else
    {
        ChannelState->RxBuff = &ChannelState->RxBuff[2];
        ChannelState->RxSize -= 2U;
    }

    /* Check if this was the last byte in the current buffer */
    if (0U == ChannelState->RxSize)
    {
        /* Invoke callback if there is one (callback may reset the rx buffer
           for continuous reception) */
        if (NULL_PTR != ChannelState->RxCallback)
        {
            ChannelState->RxCallback(Instance, UART_EVENT_RX_FULL);
        }
    }

    /* Finish reception if this was the last byte received */
    if (0U == ChannelState->RxSize)
    {
        /* Complete transfer (disable rx logic) */
        Uart_Hal_CompleteReceiveDataUsingInt(Instance);

        /* Invoke callback if there is one */
        if (NULL_PTR != ChannelState->RxCallback)
        {
            ChannelState->RxCallback(Instance, UART_EVENT_END_TRANSFER);
        }
    }
}

static void Uart_Hal_PutData(uint8 Instance)
{
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType const *ChannelState = Uart_ChannelStateArray[Instance];
    const uint8 *TxBuff = ChannelState->TxBuff;
    uint16 Data;

    if ((UART_7_BITS_PER_CHAR == ChannelState->BitCountPerChar)
            || (UART_8_BITS_PER_CHAR == ChannelState->BitCountPerChar)) /* 7-bits or 8-bits */
    {
        /* Transmit the data */
        Uart_Reg_PutChar(BaseAddress, *TxBuff);
    }
    else /* 9-bit per char */
    {
        /* Create a 16-bits integer from two bytes */
        Data = (uint16)(*TxBuff);
        ++TxBuff;
        Data |= (uint16)(((uint16)(*TxBuff)) << 8U);

        /* Transmit the data */
        Uart_Reg_PutChar9(BaseAddress, Data);
    }
}

static void Uart_Hal_TxIrqHandler(uint8 Instance)
{
    Uart_ChannelStateType *ChannelState = Uart_ChannelStateArray[Instance];
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    boolean IsLastByte = FALSE;

    /* Check if there are any more bytes to send */
    if (ChannelState->TxSize > 0U)
    {
        /* Check if it is the last character */
        if (ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR)
        {
            if (1U == ChannelState->TxSize)
            {
                IsLastByte = TRUE;
            }
        }
        else
        {
            if (2U == ChannelState->TxSize)
            {
                IsLastByte = TRUE;
            }
        }

        /* When TC interrupt is enabled, suspend all interrupts */
        if (TRUE == IsLastByte)
        {
            OSIF_ENTER_CRITICAL(UART_HAL_ID1);
        }
        /* Transmit the data */
        Uart_Hal_PutData(Instance);

        /* enable tx complete interrupt */
        if (TRUE == IsLastByte)
        {
            Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_COMPLETE, TRUE);
            OSIF_EXIT_CRITICAL(UART_HAL_ID1);
        }

        /* Update the internal state */
        if (ChannelState->BitCountPerChar != UART_9_BITS_PER_CHAR)
        {
            ++ChannelState->TxBuff;
            --ChannelState->TxSize;
        }
        else
        {
            ChannelState->TxBuff = &ChannelState->TxBuff[2];
            ChannelState->TxSize -= 2U;
        }

        /* Check if this was the last byte in the current buffer */
        if (0U == ChannelState->TxSize)
        {
            /* Invoke callback if there is one (callback may reset the tx buffer
               for continuous transmission)*/
            if (NULL_PTR != ChannelState->TxCallback)
            {
                ChannelState->TxCallback(Instance, UART_EVENT_TX_EMPTY);
            }

            /* If there's no new data, disable tx not full interrupt */
            if (0U == ChannelState->TxSize)
            {
                Uart_Reg_SetIntMode(BaseAddress, UART_INT_TX_NOT_FULL, FALSE);
            }
        }
    }
}

static void Uart_Hal_StateInit(uint8 Instance, const Uart_ChannelConfigType *ChannelConfigPtr)
{
    /* lin hal channel config */
    Uart_ChannelStateType *StatePtr = Uart_ChannelStateArray[Instance];

    /* Init Uart State */
    StatePtr->IsTxBusy = FALSE;
    StatePtr->IsRxBusy = FALSE;
    StatePtr->IsRxBlocking = FALSE;
    StatePtr->IsTxBlocking = FALSE;
    StatePtr->TxComplete = FALSE;
    StatePtr->RxComplete = FALSE;
    StatePtr->TxSize = 0U;
    StatePtr->RxSize = 0U;
    /* Save the transfer information for runtime retrieval */
    StatePtr->TransferType = ChannelConfigPtr->TransferType;
    StatePtr->BitCountPerChar = ChannelConfigPtr->BitCountPerChar;

    StatePtr->RxDmaChannel = ChannelConfigPtr->RxDmaChannel;
    StatePtr->TxDmaChannel = ChannelConfigPtr->TxDmaChannel;

    /* initialize SendStatus and receiveStatus */
    StatePtr->SendStatus = STATUS_IDLE;
    StatePtr->ReceiveStatus = STATUS_IDLE;

    /* Set the Callback from config struct */
    StatePtr->RxCallback = ChannelConfigPtr->RxCallback;
    StatePtr->TxCallback = ChannelConfigPtr->TxCallback;
}

static void Uart_Hal_IRQHandler(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_UART_2842.*/
    UART_Type *BaseAddress = Uart_HalBase[Instance];
    Uart_ChannelStateType const *ChannelState = Uart_ChannelStateArray[Instance];
    /*PRQA S 2842 -- */

    uint32 Lsr0Reg = Uart_Reg_GetLSR0(BaseAddress);
    uint32 Lsr1Reg = Uart_Reg_GetLSR1(BaseAddress);
    uint32 IerReg = Uart_Reg_GetIER(BaseAddress);
    uint32 MtchcrReg = Uart_Reg_GetMTCHCR(BaseAddress);
    uint32 IdleReg = Uart_Reg_GetIDLE(BaseAddress);

    /* Handle error interrupt */
    if (TRUE == Uart_Reg_IsErrorInterruptEnable(BaseAddress))
    {
        Uart_Hal_ErrIrqHandler(Instance, Lsr0Reg);
    }

    /* Handle receive not empty interrupt */
    if ((0U != (IerReg & UART_IER_ERXNE_Msk)) && (0U != (Lsr0Reg & UART_LSR0_DR_Msk)))
    {
        Uart_Hal_RxIrqHandler(Instance);
    }

    /* Handle transmit not full interrupt */
    if ((0U != (IerReg & UART_IER_ETXNF_Msk)) && (0U != (Lsr0Reg & UART_LSR0_TXNF_Msk)))
    {
        Uart_Hal_TxIrqHandler(Instance);
    }

    /* Handle transmit complete interrupt */
    if ((0U != (IerReg & UART_IER_ETC_Msk)) && (0U != (Lsr0Reg & UART_LSR0_TC_Msk)))
    {
        Uart_Hal_TxCompleteIrqHandler(Instance);
    }

    /* Handle receive data match interrupt */
    if ((0U != (MtchcrReg & UART_MTCHCR_MTCHIE_Msk)) && (0U != (Lsr1Reg & UART_LSR1_DTMTCH_Msk)))
    {
        Uart_Reg_ClearStatusFlag(BaseAddress, UART_DATA_MATCH);
        /* Invoke callback if there is one */
        if (ChannelState->RxCallback != NULL_PTR)
        {
            ChannelState->RxCallback(Instance, UART_EVENT_DATA_MATCH);
        }
    }

    /* Handle idle line interrupt */
    if ((0U != (IdleReg & UART_IDLE_IDLEIE_Msk)) && (0U != (Lsr1Reg & UART_LSR1_IDLE_Msk)))
    {
        Uart_Reg_ClearStatusFlag(BaseAddress, UART_IDLE_LINE);
        /* Invoke callback if there is one */
        if (ChannelState->RxCallback != NULL_PTR)
        {
            ChannelState->RxCallback(Instance, UART_EVENT_IDLE_LINE);
        }
    }
}

/*============================================EOF===================================================*/
