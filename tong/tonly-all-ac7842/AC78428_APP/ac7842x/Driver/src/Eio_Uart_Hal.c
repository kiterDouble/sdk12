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
 * @file Eio_Uart_Hal.c
 *
 * @brief This file provides eio uart integration functions.
 *
 */

/* PRQA S 1503 EOF # AMCE_QAC_NoUsedApi */
/* ===========================================  Includes  =========================================== */
#include "Eio_Uart_Hal.h"
#include "AC784xx_Eio_Reg.h"
#include "Eio_Common_Hal.h"
#include "Dma_Hal.h"
#include "Ckgen_Hal.h"

/* ============================================  Define  ============================================ */
/**
 * @def UART_DIVIDER_MIN_VALUE
 * @brief Minimum allowed value for UART baud rate divider
 * @note DES_UART_MACRO_301
 */
#define UART_DIVIDER_MIN_VALUE  (0)

/**
 * @def UART_DIVIDER_MAX_VALUE
 * @brief Maximum allowed value for UART baud rate divider
 * @note DES_UART_MACRO_302
 */
#define UART_DIVIDER_MAX_VALUE  (0xFF)

/**
 * @def UART_TX_SHIFTER
 * @brief Maps channel to EIO transmit shifter index
 * @note DES_UART_MACRO_303
 */
#define UART_TX_SHIFTER(x)      (x)

/**
 * @def UART_RX_SHIFTER
 * @brief Maps channel to EIO receive shifter index
 * @note DES_UART_MACRO_304
 */
#define UART_RX_SHIFTER(x)      (x)

/**
 * @def UART_SHIFTER
 * @brief Maps channel to EIO shifter index for both TX and RX
 * @note DES_UART_MACRO_305
 */
#define UART_SHIFTER(x)         (x)

/**
 * @def UART_TX_TIMER
 * @brief Maps channel to EIO transmit timer index
 * @note DES_UART_MACRO_306
 */
#define UART_TX_TIMER(x)        (x)

/**
 * @def UART_RX_TIMER
 * @brief Maps channel to EIO receive timer index
 * @note DES_UART_MACRO_307
 */
#define UART_RX_TIMER(x)        (x)

/**
 * @def UART_TIMER
 * @brief Maps channel to EIO timer index for both TX and RX
 * @note DES_UART_MACRO_308
 */
#define UART_TIMER(x)           (x)

#define MAX_EUART_CHANNEL       2U

/* ===========================================  Typedef  ============================================ */
/**
 * @brief Driver internal context structure
 * @note DES_UART_TYP_300
 */
typedef struct
{
    boolean InitState;                              /*!< Initialization state flag */
    uint8 EioInstance;                              /*!< EIO instance number */
    Eio_CommonStateType TxEioCommon;                /*!< Common eio drivers structure for Tx */
    Eio_CommonStateType RxEioCommon;                /*!< Common eio drivers structure for Rx */
    uint8 *RxData;                                  /*!< Receive buffer pointer */
    const uint8 *TxData;                            /*!< Transmit buffer pointer */
    uint32 TxRemainingBytes;                        /*!< Number of remaining bytes to be transferred */
    uint32 RxRemainingBytes;                        /*!< Number of remaining bytes to be received */
    uint8 TxDmaChannel;                             /*!< DMA channel number for Tx */
    uint8 RxDmaChannel;                             /*!< DMA channel number for Rx */
    Eio_UartCallbackType TxCallback;                /*!< User callback function for Tx events */
    Eio_UartCallbackType RxCallback;                /*!< User callback function for Rx events */
    Eio_UartDriverDirectionType Direction;          /*!< Driver direction: Tx or Rx */
    Eio_DriverType DriverType;                      /*!< Driver type: interrupts/polling/DMA */
    Hal_StatusType TxStatus;                        /*!< Current status of the transmitter */
    Hal_StatusType RxStatus;                        /*!< Current status of the receiver */
    boolean IsTxBusy;                               /*!< Idle/busy state of the transmitter */
    boolean IsRxBusy;                               /*!< Idle/busy state of the receiver */
    uint8 BitCount;                                 /*!< Number of bits per word */
    uint8 TxFlush;                                  /*!< Used for flushing Tx buffer before ending a transmission */
} Eio_UartStateType;
/* ==========================================  Variables  =========================================== */
#if (CONFIG_EUART0_ENABLE)
/*!
 * @brief EIO UART channel 0 driver state structure
 * @note DES ID: DES_UART_VAR_302
 */
static Eio_UartStateType EUart0_ChannelState;
#endif

#if (CONFIG_EUART1_ENABLE)
/*!
 * @brief EIO UART channel 1 driver state structure
 * @note DES ID: DES_UART_VAR_303
 */
static Eio_UartStateType EUart1_ChannelState;
#endif

/**
 * @brief Array of EIO UART driver state structures
 * @note DES_UART_VAR_301
 */
static Eio_UartStateType *const EioUartState[MAX_EUART_CHANNEL] =
{
#if (CONFIG_EUART0_ENABLE)
    &EUart0_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_EUART1_ENABLE)
    &EUart1_ChannelState,
#else
    NULL_PTR,
#endif
};

/*PRQA S 3218 ++ # AMCE_EIO_3218 */
/**
 * @brief Array of EIO UART driver state structures
 * @note DES_UART_VAR_304
 */
static Eio_UartUserConfigType DefaultEioUartConfig =
{
    .EioInstance = 0U,
    .DriverType = EIO_DRIVER_TYPE_INTERRUPTS,
    .BaudRate = 115200U,
    .BitCount = 8U,
    .Direction = EIO_UART_DIRECTION_BOTH,
    .TxResourceIndex = 0U,
    .RxResourceIndex = 1U,
    .TxDataPin = 0U,
    .RxDataPin = 1U,
    .TxCallback = NULL_PTR,
    .RxCallback = NULL_PTR,
    .TxDmaChannel = 0xFFU,
    .RxDmaChannel = 0xFFU,
};
/*PRQA S 3218 -- */

/* ====================================  Functions declaration  ===================================== */

/* =====================================  Functions definition  ===================================== */
/**
 * @brief Computes the baud rate divider for a target baud rate.
 *
 * @details
 * This function calculates the timer divider value needed to achieve the desired baud rate
 * based on the input clock frequency. The divider is computed using the formula:
 * divider = ((input_clock / baud_rate) / 2) - 1, rounded to the nearest integer.
 * The result is clamped to the valid range [UART_DIVIDER_MIN_VALUE, UART_DIVIDER_MAX_VALUE].
 *
 * @param[in] BaudRate: The desired baud rate in hertz.
 * @param[out] Divider: Pointer to store the computed baud rate divider value.
 * @param[in] InputClock: The frequency of the input clock in hertz.
 *
 * @return void
 *
 * @pre
 *     BaudRate must be greater than 0.
 *     InputClock must be greater than 0.
 *     Divider must not be NULL.
 *
 * @post
 *     The computed divider value is stored in the location pointed to by Divider.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This is an internal helper function.
 *     DES_UART_API_311
 */
static void Eio_Uart_Hal_ComputeBaudRateDivider(uint32 BaudRate, uint16 *Divider, uint32 InputClock)
{
    sint32 TmpDiv;

    /* Compute divider: ((input_clock / baud_rate) / 2) - 1. Round to nearest integer */
    TmpDiv = (((sint32)InputClock + (sint32)BaudRate) / (2 * (sint32)BaudRate)) - 1;
    /* Enforce upper/lower limits */
    if (TmpDiv < UART_DIVIDER_MIN_VALUE)
    {
        TmpDiv = UART_DIVIDER_MIN_VALUE;
    }
    if (TmpDiv > UART_DIVIDER_MAX_VALUE)
    {
        TmpDiv = UART_DIVIDER_MAX_VALUE;
    }

    *Divider = (uint16)TmpDiv;
}

/**
 * @brief Configures the EIO module for UART Tx.
 *
 * @details
 * This function configures the EIO shifter and timer for UART transmission.
 * It sets up the shifter for transmit mode with appropriate start/stop bits,
 * and configures the timer for baud rate generation based on the input clock
 * and desired baud rate.
 *
 * @param[in] State: Pointer to the EIO_UART driver context structure.
 * @param[in] UserConfigPtr: Pointer to the EIO_UART user configuration structure.
 * @param[in] InputClock: The frequency of the input clock in hertz.
 *
 * @return void
 *
 * @pre
 *     State must not be NULL.
 *     UserConfigPtr must not be NULL.
 *     EIO module must be initialized.
 *
 * @post
 *     EIO shifter and timer are configured for UART transmission.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This is an internal configuration function.
 *     DES_UART_API_312
 */
static void Eio_Uart_Hal_ConfigureTx
(
    const Eio_UartStateType *State,
    const Eio_UartUserConfigType *UserConfigPtr,
    uint32 InputClock
)
{
    EIO_Type *BaseAddr;
    uint16 Divider;
    uint16 Bits;
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    BaseAddr = EioBase[State->TxEioCommon.Instance];
    ResourceIndex = State->TxEioCommon.ResourceIndex;

    /* Compute divider */
    Eio_Uart_Hal_ComputeBaudRateDivider(UserConfigPtr->BaudRate, &Divider, InputClock);
    Bits = UserConfigPtr->BitCount;

    /* Configure tx shifter */
    Eio_Reg_SetShifterConfig(BaseAddr,
                            UART_TX_SHIFTER(ResourceIndex),
                            EIO_SHIFTER_START_BIT_0,
                            EIO_SHIFTER_STOP_BIT_1,
                            EIO_SHIFTER_SOURCE_PIN);
    Eio_Reg_SetShifterControl(BaseAddr,
                             UART_TX_SHIFTER(ResourceIndex),
                             EIO_SHIFTER_MODE_TRANSMIT,
                             UserConfigPtr->TxDataPin,             /* Output on tx pin */
                             EIO_PIN_POLARITY_HIGH,
                             EIO_PIN_CONFIG_OUTPUT,
                             UART_TX_TIMER(ResourceIndex),
                             EIO_TIMER_POLARITY_POSEDGE);

    /* Configure tx timer */
    Eio_Reg_SetTimerCompare(BaseAddr, UART_TX_TIMER(ResourceIndex), \
                           (uint16)((((uint16)(Bits << 1U) - 1U) << 8U) + Divider));

    Eio_Reg_SetTimerConfig(BaseAddr,
                       UART_TX_TIMER(ResourceIndex),
                       EIO_TIMER_START_BIT_ENABLED,
                       EIO_TIMER_STOP_BIT_TIM_DIS,
                       EIO_TIMER_ENABLE_TRG_HIGH,         /* Enable when Tx data is available */
                       EIO_TIMER_DISABLE_TIM_CMP,
                       EIO_TIMER_RESET_NEVER,
                       EIO_TIMER_DECREMENT_CLK_SHIFT_TMR, /* Decrement on EIO clock */
                       EIO_TIMER_INITOUT_ONE);
    Eio_Reg_SetTimerControl(BaseAddr,
                        UART_TX_TIMER(ResourceIndex),
                        (uint8)((uint8)(UART_TX_SHIFTER(ResourceIndex) << 2U) + 1U), /* Trigger on tx shifter status flag */
                        EIO_TRIGGER_POLARITY_LOW,
                        EIO_TRIGGER_SOURCE_INTERNAL,
                        0U,                                     /* Pin unused */
                        EIO_PIN_POLARITY_HIGH,
                        EIO_PIN_CONFIG_DISABLED,
                        EIO_TIMER_MODE_DISABLED);
}

/**
 * @brief Configures the EIO module for UART Rx.
 *
 * @details
 * This function configures the EIO shifter and timer for UART reception.
 * It sets up the shifter for receive mode with appropriate start/stop bits,
 * and configures the timer for baud rate generation and data sampling based
 * on the input clock and desired baud rate.
 *
 * @param[in] State: Pointer to the EIO_UART driver context structure.
 * @param[in] UserConfigPtr: Pointer to the EIO_UART user configuration structure.
 * @param[in] InputClock: The frequency of the input clock in hertz.
 *
 * @return void
 *
 * @pre
 *     State must not be NULL.
 *     UserConfigPtr must not be NULL.
 *     EIO module must be initialized.
 *
 * @post
 *     EIO shifter and timer are configured for UART reception.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This is an internal configuration function.
 *     DES_UART_API_313
 */
static void Eio_Uart_Hal_ConfigureRx(const Eio_UartStateType *State,
                                      const Eio_UartUserConfigType *UserConfigPtr,
                                      uint32 InputClock)
{
    EIO_Type *BaseAddr;
    uint16 Divider;
    uint16 Bits;
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    BaseAddr = EioBase[State->RxEioCommon.Instance];
    ResourceIndex = State->RxEioCommon.ResourceIndex;

    /* Compute divider. */
    Eio_Uart_Hal_ComputeBaudRateDivider(UserConfigPtr->BaudRate, &Divider, InputClock);
    Bits = UserConfigPtr->BitCount;

    /* Configure rx shifter */
    Eio_Reg_SetShifterConfig(BaseAddr,
                         UART_RX_SHIFTER(ResourceIndex),
                         EIO_SHIFTER_START_BIT_0,
                         EIO_SHIFTER_STOP_BIT_1,
                         EIO_SHIFTER_SOURCE_PIN);
    Eio_Reg_SetShifterControl(BaseAddr,
                          UART_RX_SHIFTER(ResourceIndex),
                          EIO_SHIFTER_MODE_DISABLED,
                          UserConfigPtr->RxDataPin,             /* Input from rx pin */
                          EIO_PIN_POLARITY_HIGH,
                          EIO_PIN_CONFIG_DISABLED,
                          UART_RX_TIMER(ResourceIndex),
                          EIO_TIMER_POLARITY_NEGEDGE);

    /* Configure rx timer */
    Eio_Reg_SetTimerCompare(BaseAddr, UART_RX_TIMER(ResourceIndex), \
                        (uint16)((((uint16)(Bits << 1U) - 1U) << 8U) + Divider));
    Eio_Reg_SetTimerConfig(BaseAddr,
                       UART_RX_TIMER(ResourceIndex),
                       EIO_TIMER_START_BIT_ENABLED,
                       EIO_TIMER_STOP_BIT_TIM_DIS,
                       EIO_TIMER_ENABLE_PIN_POSEDGE,         /* Enable when data is available */
                       EIO_TIMER_DISABLE_TIM_CMP,
                       EIO_TIMER_RESET_PIN_RISING,
                       EIO_TIMER_DECREMENT_CLK_SHIFT_TMR,    /* Decrement on EIO clock */
                       EIO_TIMER_INITOUT_ONE_RESET);
    Eio_Reg_SetTimerControl(BaseAddr,
                        UART_RX_TIMER(ResourceIndex),
                        0U,                                      /* Trigger unused */
                        EIO_TRIGGER_POLARITY_HIGH,
                        EIO_TRIGGER_SOURCE_EXTERNAL,
                        UserConfigPtr->RxDataPin,                   /* Input from rx pin */
                        EIO_PIN_POLARITY_LOW,
                        EIO_PIN_CONFIG_DISABLED,
                        EIO_TIMER_MODE_DISABLED);
}

/**
 * @brief End the current transfer.
 *
 * @details
 * This function terminates the current UART transfer in the specified direction.
 * It disables the transfer engine (interrupts or DMA), clears remaining byte counts,
 * and marks the driver as idle. The specific actions depend on the driver type
 * (interrupts, polling, or DMA).
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[in] Direction: UART transfer direction (TX or RX).
 *
 * @return void
 *
 * @pre
 *     Channel must be valid.
 *     The driver must be initialized.
 *
 * @post
 *     Transfer engine is disabled and driver state is reset to idle.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This is an internal function used to clean up after a transfer.
 *     DES_UART_API_314
 */
static void Eio_Uart_Hal_EndTransfer(uint8 Channel, Eio_UartDriverDirectionType Direction)
{
    EIO_Type *BaseAddr;
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    uint8 DmaChannel;

    Eio_UartStateType *ChannelState = EioUartState[Channel];
    BaseAddr = EioBase[ChannelState->EioInstance];
    if (Direction == EIO_UART_DIRECTION_TX)
    {
        ResourceIndex = ChannelState->TxEioCommon.ResourceIndex;
        DmaChannel = ChannelState->TxDmaChannel;
    }
    else
    {
        ResourceIndex = ChannelState->RxEioCommon.ResourceIndex;
        DmaChannel = ChannelState->RxDmaChannel;
    }

    /* Disable transfer engine */
    switch (ChannelState->DriverType)
    {
        case EIO_DRIVER_TYPE_INTERRUPTS:
            /* Disable interrupts for Rx / Tx shifter */
            Eio_Reg_SetShifterInterrupt(BaseAddr, (uint8)(1U << UART_SHIFTER(ResourceIndex)), FALSE);
            Eio_Reg_SetShifterErrorInterrupt(BaseAddr, (uint8)(1U << UART_SHIFTER(ResourceIndex)), FALSE);
            /* Disable timer interrupt  */
            Eio_Reg_SetTimerInterrupt(BaseAddr, (uint8)(1U << UART_SHIFTER(ResourceIndex)), FALSE);
            break;

        case EIO_DRIVER_TYPE_POLLING:
            /* Nothing to do here */
            break;

        case EIO_DRIVER_TYPE_DMA:
            /* For Tx we need to disable timer interrupt */
            Eio_Reg_SetTimerInterrupt(BaseAddr, (uint8)(1U << UART_SHIFTER(ResourceIndex)), FALSE);
            /* Stop DMA channels */
            (void)Dma_Hal_StopCh(DmaChannel);
            break;

        default:
            /* Impossible type - do nothing */
            break;
    }

    if (Direction == EIO_UART_DIRECTION_TX)
    {
        ChannelState->TxRemainingBytes = 0U;
        ChannelState->IsTxBusy = FALSE;
    }
    else
    {
        ChannelState->RxRemainingBytes = 0U;
        ChannelState->IsRxBusy = FALSE;
    }
}

/**
 * @brief Enables timers and shifters to start a transfer.
 *
 * @details
 * This function enables the EIO timers and shifters to begin a UART transfer
 * in the specified direction. For reception mode, it also discards any leftover
 * received data to ensure a clean start.
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[in] Direction: UART transfer direction (TX or RX).
 *
 * @return void
 *
 * @pre
 *     Channel must be valid.
 *     The driver must be initialized and configured.
 *
 * @post
 *     EIO timers and shifters are enabled for the specified transfer direction.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This is an internal function used to activate the hardware for a transfer.
 *     DES_UART_API_315
 */
static void Eio_Uart_Hal_EnableTransfer(uint8 Channel, Eio_UartDriverDirectionType Direction)
{
    const Eio_UartStateType *ChannelState = EioUartState[Channel];
    EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    if (Direction == EIO_UART_DIRECTION_TX)
    {
        ResourceIndex = ChannelState->TxEioCommon.ResourceIndex;
    }
    else
    {
        ResourceIndex = ChannelState->RxEioCommon.ResourceIndex;
    }

    /* Enable timers and shifters */
    if (Direction == EIO_UART_DIRECTION_RX)
    {
        /* In rx mode, discard any leftover rx. data */
        if (Eio_Reg_GetShifterMode(BaseAddr, UART_TIMER(ResourceIndex)) == EIO_SHIFTER_MODE_MATCH_STORE)
        {
            Eio_Reg_SetShifterMode(BaseAddr, UART_TIMER(ResourceIndex), EIO_SHIFTER_MODE_MATCH_STORE);
        }
        else
        {
            Eio_Reg_SetShifterMode(BaseAddr, UART_TIMER(ResourceIndex), EIO_SHIFTER_MODE_RECEIVE);
        }
    }
    Eio_Reg_SetTimerMode(BaseAddr, UART_TIMER(ResourceIndex), EIO_TIMER_MODE_8BIT_BAUD);
}

/**
 * @brief Forcefully stops the current transfer.
 *
 * @details
 * This function immediately stops a UART transfer in the specified direction
 * by disabling and re-enabling timers and shifters to reset them. It clears
 * any error flags and calls Eio_Uart_Hal_EndTransfer to clean up the transfer state.
 * For Tx mode, the shifter is re-enabled to ensure correct idle state.
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[in] Direction: UART transfer direction (TX or RX).
 *
 * @return void
 *
 * @pre
 *     Channel must be valid.
 *     The driver must be initialized.
 *
 * @post
 *     Transfer is stopped, hardware is reset, and driver returns to idle state.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This function performs a hard stop of the transfer.
 *     DES_UART_API_316
 */
static void Eio_Uart_Hal_StopTransfer(uint8 Channel, Eio_UartDriverDirectionType Direction)
{
    uint8 ResourceIndex;
    const Eio_UartStateType *ChannelState = EioUartState[Channel];
    EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];

    if (Direction == EIO_UART_DIRECTION_TX)
    {
        ResourceIndex = ChannelState->TxEioCommon.ResourceIndex;
    }
    else
    {
        ResourceIndex = ChannelState->RxEioCommon.ResourceIndex;
    }

    /* Disable and re-enable timers and shifters to reset them */
    Eio_Reg_SetTimerMode(BaseAddr, UART_TIMER(ResourceIndex), EIO_TIMER_MODE_DISABLED);

    /* Clear any leftover error flags */
    Eio_Reg_ClearShifterErrorStatus(BaseAddr, UART_SHIFTER(ResourceIndex));

    /* End the transfer */
    Eio_Uart_Hal_EndTransfer(Channel, Direction);

    /* Re-enable shifter for Tx, to ensure correct idle state */
    if (Direction == EIO_UART_DIRECTION_TX)
    {
        /* In tx mode restore start bit in case it was changed for end of transmission detection */
        Eio_Reg_SetShifterStartBit(BaseAddr, UART_TX_SHIFTER(ResourceIndex), EIO_SHIFTER_START_BIT_0);
        Eio_Reg_SetShifterMode(BaseAddr, UART_TX_SHIFTER(ResourceIndex), EIO_SHIFTER_MODE_TRANSMIT);
    }
}

/**
 * @brief Reads data received by the module.
 *
 * @details
 * This function reads received data from the EIO shifter buffer and stores it
 * in the user-provided receive buffer. It handles different shifter modes and
 * adjusts for bit count (8-bit or 16-bit transfers). The receive buffer pointer
 * and remaining bytes count are updated accordingly.
 *
 * @param[in] Channel: EIO UART channel number.
 *
 * @return void
 *
 * @pre
 *     Channel must be valid.
 *     RxRemainingBytes must be greater than 0.
 *     RxData must not be NULL.
 *
 * @post
 *     Data is read from shifter buffer into user buffer.
 *     RxData pointer and RxRemainingBytes are updated.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This is an internal data handling function.
 *     DES_UART_API_317
 */
static void Eio_Uart_Hal_ReadData(uint8 Channel)
{
    DEVICE_ASSERT(EioUartState[Channel]->RxRemainingBytes > 0U);
    DEVICE_ASSERT(EioUartState[Channel]->RxData != NULL_PTR);

    uint32 Data;
    Eio_UartStateType *ChannelState = EioUartState[Channel];
    EIO_Type const *BaseAddr = EioBase[ChannelState->EioInstance];
    uint8 ResourceIndex = ChannelState->RxEioCommon.ResourceIndex;

    /* Read data from shifter buffer */
    if (Eio_Reg_GetShifterMode(BaseAddr, UART_RX_SHIFTER(ResourceIndex)) == EIO_SHIFTER_MODE_MATCH_STORE)
    {
        Data = Eio_Reg_ReadShifterBuffer(BaseAddr, UART_RX_SHIFTER(ResourceIndex), EIO_SHIFTER_RW_MODE_BIT_SWAP);
        Data >>= 32U - (uint32)(ChannelState->BitCount);
    }
    else
    {
        Data = Eio_Reg_ReadShifterBuffer(BaseAddr, UART_RX_SHIFTER(ResourceIndex), EIO_SHIFTER_RW_MODE_NORMAL);
        Data >>= 32U - (uint32)(ChannelState->BitCount);
    }
    if (ChannelState->BitCount <= 8U)
    {
        *(uint8 *)ChannelState->RxData = (uint8)Data;
        /* Update rx buffer pointer and remaining bytes count */
        ChannelState->RxData++;
        ChannelState->RxRemainingBytes -= 1U;
    }
    else
    {
        /* For more than 8 bits per word 2 bytes are needed */
        /* PRQA S 0310,3305 ++ # AMCE_EIO_0310, AMCE_EIO_3305*/
        *(uint16 *)ChannelState->RxData = (uint16)Data;
        /* PRQA S 0310,3305 -- */
        /* Update rx buffer pointer and remaining bytes count */
        ChannelState->RxData = &ChannelState->RxData[2U];
        ChannelState->RxRemainingBytes -= 2U;
    }
}

/**
 * @brief Writes data to be transmitted by the module.
 *
 * @details
 * This function writes data from the user transmit buffer to the EIO shifter
 * buffer for transmission. It handles different bit counts (8-bit or 16-bit transfers)
 * and updates the transmit buffer pointer and remaining bytes count. If no data
 * remains, the function returns without writing.
 *
 * @param[in] Channel: EIO UART channel number.
 *
 * @return void
 *
 * @pre
 *     Channel must be valid.
 *     TxData must not be NULL.
 *
 * @post
 *     Data is written from user buffer to shifter buffer.
 *     TxData pointer and TxRemainingBytes are updated.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This is an internal data handling function.
 *     DES_UART_API_318
 */
static void Eio_Uart_Hal_WriteData(uint8 Channel)
{
    DEVICE_ASSERT(EioUartState[Channel]->TxData != NULL_PTR);

    uint32 Data;
    Eio_UartStateType *ChannelState = EioUartState[Channel];
    EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];
    uint8 ResourceIndex = ChannelState->TxEioCommon.ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    if (ChannelState->TxRemainingBytes != 0U)
    {
        /* Read data from user buffer and update tx buffer pointer and remaining bytes count */
        if (ChannelState->BitCount <= 8U)
        {
            Data = (uint32)(*(const uint8 *)ChannelState->TxData);
            ChannelState->TxData++;
            ChannelState->TxRemainingBytes -= 1U;
        }
        else
        {
            /* For more than 8 bits per word 2 bytes are needed */
            /* PRQA S 0310,3305 ++ # AMCE_EIO_0310, AMCE_EIO_3305 */
            Data = (uint32)(*(const uint16 *)ChannelState->TxData);
            /* PRQA S 0310,3305 -- */
            ChannelState->TxData = &ChannelState->TxData[2U];
            ChannelState->TxRemainingBytes -= 2U;
        }

        Eio_Reg_WriteShifterBuffer(BaseAddr, UART_TX_SHIFTER(ResourceIndex), Data, EIO_SHIFTER_RW_MODE_NORMAL);
    }
}

/**
 * @brief Check status of the UART transmission.
 *
 * @details
 * This function monitors the UART transmission status, checks for transfer completion,
 * handles data requests from the shifter, and manages the end-of-transmission sequence.
 * It is called periodically in polling mode or from interrupt context. When transmission
 * is complete, it invokes the user callback.
 *
 * @param[in] StateStruct: Pointer to the EIO common state structure (cast to void*).
 *
 * @return void
 *
 * @pre
 *     StateStruct must not be NULL.
 *     The driver must be in transmit mode.
 *
 * @post
 *     Transmit status is updated, data is written if needed, callbacks are invoked.
 *
 * @note
 *     Reentrancy: Not reentrant.
 *     This function is used as an ISR callback in interrupt mode.
 *     DES_UART_API_319
 */
/*PRQA S 3673 ++ # AMCE_EIO_3673 */
static void Eio_Uart_Hal_CheckStatusTx(void *StateStruct)
/*PRQA S 3673 -- */
{
    DEVICE_ASSERT(StateStruct != NULL_PTR);

    EIO_Type *BaseAddr;
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    Eio_CommonStateType const *State;
    uint32 Channel;
    Eio_UartStateType *ChannelState;

    /*PRQA S 0316 ++ # AMCE_QAC_0316 */
    State = (Eio_CommonStateType *)StateStruct;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # AMCE_EIO_0326.*/
    Channel = (uint32)(State->UserArgs);
    /*PRQA S 0326 -- */
    BaseAddr = EioBase[State->Instance];
    ResourceIndex = State->ResourceIndex;
    ChannelState = EioUartState[Channel];

    /* No need to check for Tx underflow since timer is controlled by the shifter status flag */
    /* Check for transfer end */
    if (ChannelState->TxRemainingBytes == 0U)
    {
        if (TRUE == Eio_Reg_GetTimerStatus(BaseAddr, UART_TX_TIMER(ResourceIndex)))
        {
            /* Clear timer status */
            Eio_Reg_ClearTimerStatus(BaseAddr, UART_TX_TIMER(ResourceIndex));
            ChannelState->TxFlush--;
            if (ChannelState->TxFlush == 0U)
            {
                /* Done flushing the Tx buffer, end transfer */
                /* Record success if there was no error */
                if (ChannelState->TxStatus == STATUS_BUSY)
                {
                    ChannelState->TxStatus = STATUS_SUCCESS;
                }
                Eio_Uart_Hal_StopTransfer((uint8)Channel, EIO_UART_DIRECTION_TX);
                /* Call callback to announce the end transfer event to the user */
                if (ChannelState->TxCallback != NULL_PTR)
                {
                    ChannelState->TxCallback((uint8)Channel, EIO_UART_EVENT_END_TRANSFER);
                }
            }
            else if (TRUE == Eio_Reg_GetShifterStatus(BaseAddr, UART_TX_SHIFTER(ResourceIndex)))
            {
                /* txFlush == 1, but last byte was already transferred from buffer to shifter. There is a
                   danger that the transmission is over and we end up never reporting the end event.
                   To avoid this, send one extra dummy byte */
                /* Set start bit to 1 and send an 0xFF byte, this way the line will appear idle */
                Eio_Reg_SetShifterStartBit(BaseAddr, UART_TX_SHIFTER(ResourceIndex), EIO_SHIFTER_START_BIT_1);
                Eio_Reg_WriteShifterBuffer(BaseAddr, UART_TX_SHIFTER(ResourceIndex), 0xFFFFFFFFU,
                        EIO_SHIFTER_RW_MODE_NORMAL);
            }
            else
            {
                /* txFlush == 1, and last byte was not yet transferred from buffer to shifter.
                   No need to do anything, just wait for the next timer event. */
            }
        }
    }
    /* Check if transmitter needs more data */
    else if ((TRUE == Eio_Reg_GetShifterStatus(BaseAddr, UART_TX_SHIFTER(ResourceIndex))) &&
            (ChannelState->TxRemainingBytes > 0U))
    {
        Eio_Uart_Hal_WriteData((uint8)Channel);
        if (ChannelState->TxRemainingBytes == 0U)
        {
            /* Out of data, call callback to allow user to provide a new buffer */
            if (ChannelState->TxCallback != NULL_PTR)
            {
                ChannelState->TxCallback((uint8)Channel, EIO_UART_EVENT_TX_EMPTY);
            }
        }
        if (ChannelState->TxRemainingBytes == 0U)
        {
            /* No more data, transmission will stop after the last bytes are sent.
               The timer event will indicate when the send is complete */
            /* Clear any previous timer events */
            Eio_Reg_ClearTimerStatus(BaseAddr, UART_TX_TIMER(ResourceIndex));
            if (ChannelState->DriverType == EIO_DRIVER_TYPE_INTERRUPTS)
            {
                /* Transmission completed; disable interrupt */
                Eio_Reg_SetShifterInterrupt(BaseAddr, (uint8)(1U << UART_TX_SHIFTER(ResourceIndex)), FALSE);
                /* Enable timer interrupt to ensure that transfer is completed */
                Eio_Reg_SetTimerInterrupt(BaseAddr, (uint8)(1U << UART_TX_TIMER(ResourceIndex)), TRUE);
            }
        }
    }
    else
    {
        /* No relevant events - nothing to do */
    }
}

/**
 * @brief Check status of the UART reception.
 *
 * @details
 * This function monitors the UART reception status, checks for errors (overrun),
 * handles received data, and manages the end-of-reception sequence. It is called
 * periodically in polling mode or from interrupt context. When reception is complete
 * or an error occurs, it invokes the user callback.
 *
 * @param[in] StateStruct: Pointer to the EIO common state structure (cast to void*).
 *
 * @return void
 *
 * @pre
 *     StateStruct must not be NULL.
 *     The driver must be in receive mode.
 *
 * @post
 *     Receive status is updated, data is read if available, callbacks are invoked.
 *
 * @note
 *     Reentrancy: Not reentrant.
 *     This function is used as an ISR callback in interrupt mode.
 *     DES_UART_API_320
 */
/*PRQA S 3673 ++ # AMCE_EIO_3673 */
static void Eio_Uart_Hal_CheckStatusRx(void *StateStruct)
/*PRQA S 3673 -- */
{
    DEVICE_ASSERT(StateStruct != NULL_PTR);

    EIO_Type *BaseAddr;
    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    Eio_CommonStateType const *State;
    uint32 Channel;
    Eio_UartStateType *ChannelState;

    /*PRQA S 0316 ++ # AMCE_QAC_0316 */
    State = (Eio_CommonStateType *)StateStruct;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # AMCE_EIO_0326.*/
    Channel = (uint32)(State->UserArgs);
    /*PRQA S 0326 -- */
    BaseAddr = EioBase[State->Instance];
    ResourceIndex = State->ResourceIndex;
    ChannelState = EioUartState[Channel];

    /* Check for errors */
    if (TRUE == Eio_Reg_GetShifterErrorStatus(BaseAddr, UART_RX_SHIFTER(ResourceIndex)))
    {
        ChannelState->RxStatus = STATUS_UART_RX_OVERRUN;
        Eio_Reg_ClearShifterErrorStatus(BaseAddr, UART_RX_SHIFTER(ResourceIndex));
        ChannelState->RxRemainingBytes = 0U;
        /* Continue processing events */
    }
    /* Check if data was received */
    else if (TRUE == Eio_Reg_GetShifterStatus(BaseAddr, UART_RX_SHIFTER(ResourceIndex)))
    {
        Eio_Uart_Hal_ReadData((uint8)Channel);
        if (ChannelState->RxRemainingBytes == 0U)
        {
            /* Out of data, call callback to allow user to provide a new buffer */
            if (ChannelState->RxCallback != NULL_PTR)
            {
                ChannelState->RxCallback((uint8)Channel, EIO_UART_EVENT_RX_FULL);
            }
        }
    }
    else
    {
        /* No events - nothing to do */
    }
    /* Check if transfer is over */
    if (ChannelState->RxRemainingBytes == 0U)
    {
        /* Record success if there was no error */
        if (ChannelState->RxStatus == STATUS_BUSY)
        {
            ChannelState->RxStatus = STATUS_SUCCESS;
        }
        /* Discard any leftover rx. data */
        Eio_Reg_ClearShifterStatus(BaseAddr, UART_RX_SHIFTER(ResourceIndex));
        /* End transfer */
        Eio_Uart_Hal_StopTransfer((uint8)Channel, EIO_UART_DIRECTION_RX);
        /* Call callback to announce the event to the user */
        if (ChannelState->RxCallback != NULL_PTR)
        {
            ChannelState->RxCallback((uint8)Channel, EIO_UART_EVENT_END_TRANSFER);
        }
    }
}

/**
 * @brief Check status of the UART transfer.
 *
 * @details
 * This function dispatches status checking to the appropriate handler based on
 * the transfer direction (Tx or Rx). It serves as a unified entry point for
 * status monitoring in both polling and interrupt modes.
 *
 * @param[in] Channel: EIO UART channel number.
 * @param[in] Direction: UART transfer direction (TX or RX).
 *
 * @return void
 *
 * @pre
 *     Channel must be valid.
 *     The driver must be initialized.
 *
 * @post
 *     Appropriate status check function is called for the specified direction.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This is an internal dispatcher function.
 *     DES_UART_API_321
 */
static void Eio_Uart_Hal_CheckStatus(uint8 Channel, Eio_UartDriverDirectionType Direction)
{
     /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    Eio_UartStateType *ChannelState = EioUartState[Channel];
    /*PRQA S 2842 -- */
    if (Direction == EIO_UART_DIRECTION_TX)
    {
        Eio_Uart_Hal_CheckStatusTx(&ChannelState->TxEioCommon);
    }
    else
    {
        Eio_Uart_Hal_CheckStatusRx(&ChannelState->RxEioCommon);
    }
}

/**
 * @brief Computes the address of the register used for DMA Tx transfer.
 *
 * @details
 * This function calculates the physical address of the EIO shifter buffer register
 * that the DMA controller should use as the destination for UART transmission.
 * The address is based on the EIO instance and the Tx shifter index.
 *
 * @param[in] Channel: EIO UART channel number.
 *
 * @return
 *     The physical address of the transmit shifter buffer register.
 *
 * @pre
 *     Channel must be valid.
 *     The driver must be initialized.
 *
 * @post
 *     None.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This is a helper function for DMA configuration.
 *     DES_UART_API_324
 */
static uint32 Eio_Uart_Hal_ComputeTxRegAddr(uint8 Channel)
{
    uint32 Addr;

    const Eio_UartStateType *ChannelState = EioUartState[Channel];
    const EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];
    uint8 Shifter = UART_TX_SHIFTER(ChannelState->TxEioCommon.ResourceIndex);
    Addr = (uint32)(&(BaseAddr->SHIFTBUF[Shifter]));

    return Addr;
}

/**
 * @brief Computes the address of the register used for DMA Rx transfer.
 *
 * @details
 * This function calculates the physical address of the EIO shifter buffer register
 * that the DMA controller should use as the source for UART reception. The address
 * is adjusted based on shifter mode, EIO instance, Rx shifter index, and byte count
 * to ensure proper byte alignment.
 *
 * @param[in] Channel: EIO UART channel number.
 *
 * @return
 *     The physical address of the receive shifter buffer register.
 *
 * @pre
 *     Channel must be valid.
 *     The driver must be initialized.
 *
 * @post
 *     None.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This is a helper function for DMA configuration.
 *     Address adjustment accounts for endianness and byte alignment.
 *     DES_UART_API_325
 */
static uint32 Eio_Uart_Hal_ComputeRxRegAddr(uint8 Channel)
{
    uint32 Addr;
    uint32 ByteCount;

    const Eio_UartStateType *ChannelState = EioUartState[Channel];
    const EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];
    uint8 Shifter = UART_RX_SHIFTER(ChannelState->RxEioCommon.ResourceIndex);

    if (ChannelState->BitCount <= 8U)
    {
        ByteCount = 1U;
    }
    else
    {
        ByteCount = 2U;
    }

    if (Eio_Reg_GetShifterMode(BaseAddr, Shifter) == EIO_SHIFTER_MODE_MATCH_STORE)
    {
        Addr = (uint32)(&(BaseAddr->SHIFTBUFBIS[Shifter])) + (sizeof(uint32) - ByteCount);
    }
    else
    {
        Addr = (uint32)(&(BaseAddr->SHIFTBUF[Shifter])) + (sizeof(uint32) - ByteCount);
    }

    return Addr;
}

/**
 * @brief Function called at the end of a DMA Tx transfer.
 *
 * @details
 * This callback function is invoked by the DMA driver when a transmission transfer
 * completes or encounters an error. It handles error conditions by stopping the transfer
 * and notifying the user. On success, it invokes the user callback to allow providing
 * a new buffer and enables timer interrupt to ensure complete transmission.
 *
 * @param[in] Param: Pointer to DMA callback information structure (Dma_ChannelCBInfoType*).
 *
 * @return void
 *
 * @pre
 *     Param must not be NULL.
 *     DMA transfer must have been initiated.
 *
 * @post
 *     Transfer is stopped on error, or timer interrupt is enabled for completion check.
 *
 * @note
 *     Reentrancy: Not reentrant.
 *     This function is called from DMA interrupt context.
 *     DES_UART_API_322
 */
/*PRQA S 3673 ++ # AMCE_EIO_3673 */
static void Eio_Uart_Hal_EndDmaTxTransfer(void *Param)
/*PRQA S 3673 -- */
{
    /*PRQA S 0316 ++ # AMCE_QAC_0316.*/
    const Dma_ChannelCBInfoType *DmaInfo = (const Dma_ChannelCBInfoType *)Param;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # AMCE_EIO_0326.*/
    uint32 Channel = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */
    Dma_ChannelAddrType DmaAddr;
    Dma_TransferUnitType DmaTransferSize;

    Eio_UartStateType *ChannelState = EioUartState[Channel];
    EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];

    if (0U != (DmaInfo->DmaEvent & DMA_ERROR_EVENT))
    {
        /* DMA error, stop transfer */
        ChannelState->TxStatus = STATUS_ERROR;
        Eio_Uart_Hal_StopTransfer((uint8)Channel, EIO_UART_DIRECTION_TX);
        /* Call callback to announce the end transfer event to the user */
        if (ChannelState->TxCallback != NULL_PTR)
        {
            ChannelState->TxCallback((uint8)Channel, EIO_UART_EVENT_END_TRANSFER);
        }
    }
    else
    {
        ChannelState->TxRemainingBytes = 0U;

        /* Call callback to allow user to provide a new buffer */
        if (ChannelState->TxCallback != NULL_PTR)
        {
            ChannelState->TxCallback((uint8)Channel, EIO_UART_EVENT_TX_EMPTY);
        }

        if (ChannelState->TxRemainingBytes == 0U)
        {
            /* No more data to transmit, transmission will stop */
            /* Enable timer interrupt to let IRQ ensure that transfer is completed */
            Eio_Reg_ClearTimerStatus(BaseAddr, UART_TX_TIMER(ChannelState->TxEioCommon.ResourceIndex));
            Eio_Reg_SetTimerInterrupt(BaseAddr,
                    (uint8)(1U << UART_TX_TIMER(ChannelState->TxEioCommon.ResourceIndex)), TRUE);
        }
        else
        {
            if (ChannelState->BitCount <= 8U)
            {
                DmaTransferSize = DMA_TRANSFER_UNIT_1B;
            }
            else
            {
                DmaTransferSize = DMA_TRANSFER_UNIT_2B;
            }
            DmaAddr.Length = (uint16)ChannelState->TxRemainingBytes;
            DmaAddr.SrcOffset = (uint16)(1UL << ((uint16)DmaTransferSize));;
            DmaAddr.DestOffset = 0U;
            DmaAddr.SrcStartAddr = (uint32)ChannelState->TxData;
            DmaAddr.DestStartAddr = Eio_Uart_Hal_ComputeTxRegAddr((uint8)Channel);
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
    }
}

/**
 * @brief Function called at the end of a DMA Rx transfer.
 *
 * @details
 * This callback function is invoked by the DMA driver when a reception transfer
 * completes or encounters an error. It handles error conditions by stopping the
 * transfer and notifying the user. On successful completion, it stops the transfer,
 * updates the status, and invokes the user callback.
 *
 * @param[in] Param: Pointer to DMA callback information structure (Dma_ChannelCBInfoType*).
 *
 * @return void
 *
 * @pre
 *     Param must not be NULL.
 *     DMA transfer must have been initiated.
 *
 * @post
 *     Transfer is stopped and user is notified of completion or error.
 *
 * @note
 *     Reentrancy: Not reentrant.
 *     This function is called from DMA interrupt context.
 *     DES_UART_API_323
 */
/*PRQA S 3673 ++ # AMCE_EIO_3673 */
static void Eio_Uart_Hal_EndDmaRxTransfer(void *Param)
/*PRQA S 3673 -- */
{
    /*PRQA S 0316 ++ # AMCE_QAC_0316.*/
    const Dma_ChannelCBInfoType *DmaInfo = (const Dma_ChannelCBInfoType *)Param;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # AMCE_EIO_0326.*/
    uint32 Channel = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */
    Dma_ChannelAddrType DmaAddr;
    Dma_TransferUnitType DmaTransferSize;

    Eio_UartStateType *ChannelState = EioUartState[Channel];
    if (0U != (DmaInfo->DmaEvent & DMA_ERROR_EVENT))
    {
        /* DMA error, stop transfer */
        ChannelState->RxStatus = STATUS_ERROR;
        ChannelState->RxRemainingBytes = 0U;
        /* Notify the application that an error occurred */
        if (NULL_PTR != ChannelState->RxCallback)
        {
            ChannelState->RxCallback((uint8)Channel, EIO_UART_EVENT_ERROR);
        }
    }

    /* Return if an error occurred; error cases are treated by the interrupt handler */
    if ((0U != (DmaInfo->DmaEvent & DMA_FINISH_EVENT)) && (ChannelState->RxStatus == STATUS_BUSY))
    {
        ChannelState->RxRemainingBytes = 0U;
        if (NULL_PTR != ChannelState->RxCallback)
        {
            ChannelState->RxCallback((uint8)Channel, EIO_UART_EVENT_RX_FULL);
        }
    }
    else/* ReceiveStatus not STATUS_BUSY */
    {
        /* do nothing */
    }

    if (ChannelState->RxRemainingBytes == 0U)
    {
        if (ChannelState->RxStatus == STATUS_BUSY)
        {
            ChannelState->RxStatus = STATUS_SUCCESS;
        }

        /* Stop the reception */
        Eio_Uart_Hal_StopTransfer((uint8)Channel, EIO_UART_DIRECTION_RX);

        /* Call callback to announce the event to the user */
        if (NULL_PTR != ChannelState->RxCallback)
        {
            ChannelState->RxCallback((uint8)Channel, EIO_UART_EVENT_END_TRANSFER);
        }
    }
    else
    {
        if (ChannelState->BitCount <= 8U)
        {
            DmaTransferSize = DMA_TRANSFER_UNIT_1B;
        }
        else
        {
            DmaTransferSize = DMA_TRANSFER_UNIT_2B;
        }
        DmaAddr.Length = (uint16)ChannelState->RxRemainingBytes;
        DmaAddr.SrcOffset = 0U;
        DmaAddr.DestOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
        DmaAddr.SrcStartAddr = Eio_Uart_Hal_ComputeRxRegAddr((uint8)Channel);
        DmaAddr.DestStartAddr = (uint32)ChannelState->RxData;
        /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
        DmaAddr.SrcEndAddr = DmaAddr.SrcStartAddr + \
                            (DmaAddr.Length / (1UL << (uint8)DmaTransferSize) * \
                            DmaAddr.SrcOffset);
        DmaAddr.DestEndAddr = DmaAddr.DestStartAddr + \
                            (DmaAddr.Length / (1UL << (uint8)DmaTransferSize) * \
                            DmaAddr.DestOffset);
        /*PRQA S 2985 -- */
        Dma_Hal_UpdateChAddr(ChannelState->RxDmaChannel, &DmaAddr);
        /* Start the channel */
        (void)Dma_Hal_StartCh(ChannelState->RxDmaChannel);
    }
}

/**
 * @brief Starts a Tx DMA transfer.
 *
 * @details
 * This function configures and starts a DMA transfer for UART transmission.
 * It sets up the DMA channel with appropriate source/destination addresses,
 * transfer size, and callback. It also enables EIO DMA requests for the Tx shifter.
 *
 * @param[in] Channel: EIO UART channel number.
 *
 * @return
 *     STATUS_SUCCESS: DMA transfer started successfully.
 *     Error code: DMA configuration or start failed.
 *
 * @pre
 *     Channel must be valid.
 *     The driver must be initialized in DMA mode.
 *     DMA channel must be allocated.
 *
 * @post
 *     DMA transfer is configured and started.
 *     EIO DMA requests are enabled.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This is an internal function for DMA mode operation.
 *     DES_UART_API_326
 */
static Hal_StatusType Eio_Uart_Hal_StartTxDmaTransfer(uint8 Channel)
{
    Hal_StatusType RetCode;
    Dma_TransferUnitType DmaTransferSize;
    Dma_TransferConfigType TransferConfig;

    const Eio_UartStateType *ChannelState = EioUartState[Channel];
    EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];
    uint8 ResourceIndex = ChannelState->TxEioCommon.ResourceIndex;
    uint32 UartChannel;

    /* Configure the transfer control descriptor for the previously allocated channel */
    if (ChannelState->BitCount <= 8U)
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_1B;
    }
    else
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_2B;
    }

    TransferConfig.TriggerMode = FALSE;
    TransferConfig.CircularMode = FALSE;
#if defined (AC7840E)
    TransferConfig.DaisyMode = FALSE;
#endif
    TransferConfig.SrcUnit = DmaTransferSize;
    TransferConfig.DestUnit = DmaTransferSize;
    TransferConfig.Type = DMA_TRANSFER_MEM2PERIPH;
    TransferConfig.SrcStartAddr = (uint32)ChannelState->TxData;
    TransferConfig.DestStartAddr = Eio_Uart_Hal_ComputeTxRegAddr(Channel);
    TransferConfig.SrcOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
    TransferConfig.DestOffset = 0U;
    TransferConfig.Length = (uint16)ChannelState->TxRemainingBytes;
    TransferConfig.Callback = (Hal_CallbackType)Eio_Uart_Hal_EndDmaTxTransfer;
    TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
    /*PRQA S 0326 ++ # AMCE_EIO_0326.*/
    UartChannel = Channel;
    TransferConfig.UserArgs = (void *)(UartChannel);
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
    RetCode = Dma_Hal_ConfigCh(ChannelState->TxDmaChannel, &TransferConfig);

    if (STATUS_SUCCESS == RetCode)/* config dma success */
    {
        /* Enable EIO DMA requests */
        Eio_Reg_SetShifterDMARequest(BaseAddr, (uint8)(1U << UART_TX_SHIFTER(ResourceIndex)), TRUE);
        /* Start tx DMA channel */
        (void)Dma_Hal_StartCh(ChannelState->TxDmaChannel);
    }

    return RetCode;
}

/**
 * @brief Starts a Rx DMA transfer.
 *
 * @details
 * This function configures and starts a DMA transfer for UART reception.
 * It sets up the DMA channel with appropriate source/destination addresses,
 * transfer size, and callback. It also enables EIO DMA requests for the Rx shifter.
 *
 * @param[in] Channel: EIO UART channel number.
 *
 * @return
 *     STATUS_SUCCESS: DMA transfer started successfully.
 *     Error code: DMA configuration failed.
 *
 * @pre
 *     Channel must be valid.
 *     The driver must be initialized in DMA mode.
 *     DMA channel must be allocated.
 *
 * @post
 *     DMA transfer is configured and started.
 *     EIO DMA requests are enabled.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; non-reentrant for the same channel.
 *     This is an internal function for DMA mode operation.
 *     DES_UART_API_327
 */
static Hal_StatusType Eio_Uart_Hal_StartRxDmaTransfer(uint8 Channel)
{
    Hal_StatusType RetCode;
    Dma_TransferUnitType DmaTransferSize;
    Dma_TransferConfigType TransferConfig;
    uint32 UartChannel;

    const Eio_UartStateType *ChannelState = EioUartState[Channel];
    EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];
    uint8 ResourceIndex = ChannelState->RxEioCommon.ResourceIndex;

    /* Configure the transfer control descriptor for the previously allocated channel */
    if (ChannelState->BitCount <= 8U)
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_1B;
    }
    else
    {
        DmaTransferSize = DMA_TRANSFER_UNIT_2B;
    }

    TransferConfig.TriggerMode = FALSE;
    TransferConfig.CircularMode = FALSE;
#if defined (AC7840E)
    TransferConfig.DaisyMode = FALSE;
#endif
    TransferConfig.SrcUnit = DmaTransferSize;
    TransferConfig.DestUnit = DmaTransferSize;
    TransferConfig.Type = DMA_TRANSFER_PERIPH2MEM;
    TransferConfig.SrcStartAddr = Eio_Uart_Hal_ComputeRxRegAddr(Channel);
    TransferConfig.DestStartAddr = (uint32)ChannelState->RxData;
    TransferConfig.SrcOffset = 0U;
    TransferConfig.DestOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
    TransferConfig.Length = (uint16)ChannelState->RxRemainingBytes;
    TransferConfig.Callback = (Hal_CallbackType)Eio_Uart_Hal_EndDmaRxTransfer;
    TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
    UartChannel = Channel;
    /*PRQA S 0326 ++ # AMCE_EIO_0326.*/
    TransferConfig.UserArgs = (void *)(UartChannel);
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
    RetCode = Dma_Hal_ConfigCh(ChannelState->RxDmaChannel, &TransferConfig);

    if (STATUS_SUCCESS == RetCode)
    {
        /* Enable EIO DMA requests */
        Eio_Reg_SetShifterDMARequest(BaseAddr, (uint8)(1U << UART_RX_SHIFTER(ResourceIndex)), TRUE);
        /* Start rx DMA channel */
        (void)Dma_Hal_StartCh(ChannelState->RxDmaChannel);
    }

    return STATUS_SUCCESS;
}

/* Implementation of Eio_Uart_Hal_Init declared in Eio_Uart_Hal.h */
Hal_StatusType Eio_Uart_Hal_Init(uint8 Channel, const Eio_UartUserConfigType *UserConfigPtr)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    DEVICE_ASSERT(UserConfigPtr != NULL_PTR);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);

    uint32 InputClock;
    uint32 EioChannel = Channel;
    Hal_StatusType RetCode = STATUS_SUCCESS;
    EIO_Type *BaseAddr;

    /*PRQA S 2812 ++ # AMCE_EIO_2812.*/
    BaseAddr = EioBase[UserConfigPtr->EioInstance];
    /*PRQA S 2812 -- */
    Eio_UartStateType *ChannelState = EioUartState[Channel];
    /*PRQA S 2842 -- */

    Eio_Hal_InitDevice(UserConfigPtr->EioInstance);

    /* Get the protocol clock frequency */
    (void)Ckgen_Hal_GetFreq(Eio_HalClock[UserConfigPtr->EioInstance], &InputClock);
    DEVICE_ASSERT(InputClock > 0U);

    /* Instruct the resource allocator that we need one shifter/timer at a specific index */
    if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U)
    {
        ChannelState->TxEioCommon.ResourceCount = 1U;
        ChannelState->TxEioCommon.ResourceIndex = UserConfigPtr->TxResourceIndex;
        /*PRQA S 0326 ++ # AMCE_EIO_0326.*/
        ChannelState->TxEioCommon.UserArgs = (void *)EioChannel;
        /*PRQA S 0326 -- */
        RetCode = Eio_Hal_InitDriver(UserConfigPtr->EioInstance, &ChannelState->TxEioCommon);
    }

    if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_RX) != 0U)
    {
        ChannelState->RxEioCommon.ResourceCount = 1U;
        ChannelState->RxEioCommon.ResourceIndex = UserConfigPtr->RxResourceIndex;
        /*PRQA S 0326 ++ # AMCE_EIO_0326.*/
        ChannelState->RxEioCommon.UserArgs = (void *)EioChannel;
        /*PRQA S 0326 -- */
        RetCode = Eio_Hal_InitDriver(UserConfigPtr->EioInstance, &ChannelState->RxEioCommon);
    }

    if (RetCode == STATUS_SUCCESS)
    {
        /* Initialize driver-specific context structure */
        ChannelState->EioInstance = UserConfigPtr->EioInstance;
        ChannelState->RxData = NULL_PTR;
        ChannelState->TxData = NULL_PTR;
        ChannelState->TxRemainingBytes = 0U;
        ChannelState->RxRemainingBytes = 0U;
        ChannelState->TxCallback = UserConfigPtr->TxCallback;
        ChannelState->RxCallback = UserConfigPtr->RxCallback;
        ChannelState->DriverType = UserConfigPtr->DriverType;
        ChannelState->Direction = UserConfigPtr->Direction;
        ChannelState->TxStatus = STATUS_IDLE;
        ChannelState->RxStatus = STATUS_IDLE;
        ChannelState->IsTxBusy = FALSE;
        ChannelState->IsRxBusy = FALSE;
        ChannelState->BitCount = UserConfigPtr->BitCount;

        if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U)
        {
            if (ChannelState->DriverType == EIO_DRIVER_TYPE_DMA)
            {
                /* Enable EIO DMA requests */
                Eio_Reg_SetShifterDMARequest(BaseAddr,
                        (uint8)(1U << UART_TX_SHIFTER(ChannelState->TxEioCommon.ResourceIndex)), TRUE);
            }
            /* Configure device for UART Tx mode */
            Eio_Uart_Hal_ConfigureTx(ChannelState, UserConfigPtr, InputClock);
        }

        if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_RX) != 0U)
        {
            if (ChannelState->DriverType == EIO_DRIVER_TYPE_DMA)
            {
                /* Enable EIO DMA requests */
                Eio_Reg_SetShifterDMARequest(BaseAddr,
                        (uint8)(1U << UART_RX_SHIFTER(ChannelState->RxEioCommon.ResourceIndex)), TRUE);
            }
            /* Configure device for UART Rx mode */
            Eio_Uart_Hal_ConfigureRx(ChannelState, UserConfigPtr, InputClock);
        }

        /* Set up transfer engine */
        switch (ChannelState->DriverType)
        {
            case EIO_DRIVER_TYPE_INTERRUPTS:
                if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U)
                {
                    ChannelState->TxEioCommon.Isr = Eio_Uart_Hal_CheckStatusTx;
                }

                if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_RX) != 0U)
                {
                    ChannelState->RxEioCommon.Isr = Eio_Uart_Hal_CheckStatusRx;
                }
                break;

            case EIO_DRIVER_TYPE_POLLING:
                /* Nothing to do here, Eio_Uart_Hal_GetSendStatus()/Eio_Uart_Hal_GetReceiveStatus() will handle the transfer */
                break;

            case EIO_DRIVER_TYPE_DMA:
                /* Store DMA channel number */
                ChannelState->TxDmaChannel = UserConfigPtr->TxDmaChannel;
                ChannelState->RxDmaChannel = UserConfigPtr->RxDmaChannel;

                /* For Tx we will still need interrupt to signal end of transfer */
                if (((uint8)UserConfigPtr->Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U)
                {
                    ChannelState->TxEioCommon.Isr = Eio_Uart_Hal_CheckStatusTx;
                }
                break;

            default:
                /* Impossible type - do nothing */
                break;
        }

        ChannelState->InitState = TRUE;
    }

    return RetCode;
}

/* Implementation of Eio_Uart_Hal_Deinit declared in Eio_Uart_Hal.h */
Hal_StatusType Eio_Uart_Hal_Deinit(uint8 Channel)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);
    DEVICE_ASSERT(TRUE == EioUartState[Channel]->InitState);

    Hal_StatusType RetCode = STATUS_SUCCESS;
    Eio_UartStateType *ChannelState = EioUartState[Channel];

    /* Check if driver is busy */
    if ((ChannelState->IsTxBusy == TRUE) || (ChannelState->IsRxBusy == TRUE))
    {
        RetCode = STATUS_BUSY;
    }
    else
    {
        Eio_Hal_DeinitDriver(&ChannelState->TxEioCommon);
        Eio_Hal_DeinitDriver(&ChannelState->RxEioCommon);
        Eio_Hal_DeinitDevice(ChannelState->EioInstance);
    }
    /*PRQA S 2842 -- */

    ChannelState->InitState = FALSE;

    return RetCode;
}

/* Implementation of Eio_Uart_Hal_SetBaudRate declared in Eio_Uart_Hal.h */
Hal_StatusType Eio_Uart_Hal_SetBaudRate(uint8 Channel, uint32 BaudRate)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);
    DEVICE_ASSERT(TRUE == EioUartState[Channel]->InitState);
    DEVICE_ASSERT(BaudRate > 0U);

    Hal_StatusType RetCode = STATUS_SUCCESS;
    uint16 Divider;
    uint32 InputClock;
    uint8 TxResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    uint8 RxResourceIndex;
    const Eio_UartStateType *ChannelState = EioUartState[Channel];
    uint16 Bits = ChannelState->BitCount;
    EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];

    TxResourceIndex = ChannelState->TxEioCommon.ResourceIndex;
    RxResourceIndex = ChannelState->RxEioCommon.ResourceIndex;

    /* Check if driver is busy */
    if ((ChannelState->IsTxBusy == TRUE) || (ChannelState->IsRxBusy == TRUE))
    {
        RetCode = STATUS_BUSY;
    }
    else
    {
        /* Get the protocol clock frequency */
        (void)Ckgen_Hal_GetFreq(Eio_HalClock[ChannelState->EioInstance], &InputClock);
        DEVICE_ASSERT(InputClock > 0U);

        /* Compute divider */
        Eio_Uart_Hal_ComputeBaudRateDivider(BaudRate, &Divider, InputClock);

        /* Configure tx/rx timer */
        if (((uint8)ChannelState->Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U)
        {
            Eio_Reg_SetTimerCompare(BaseAddr, UART_TX_TIMER(TxResourceIndex),
                     (uint16)((((uint16)(Bits << 1U) - 1U) << 8U) + Divider));
        }
        if (((uint8)ChannelState->Direction & (uint8)EIO_UART_DIRECTION_RX) != 0U)
        {
            Eio_Reg_SetTimerCompare(BaseAddr, UART_RX_TIMER(RxResourceIndex),
                     (uint16)((((uint16)(Bits << 1U) - 1U) << 8U) + Divider));
        }
    }
    /*PRQA S 2842 -- */

    return RetCode;
}

/* Implementation of Eio_Uart_Hal_GetBaudRate declared in Eio_Uart_Hal.h */
void Eio_Uart_Hal_GetBaudRate(uint8 Channel, uint32 *BaudRate)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);
    DEVICE_ASSERT(TRUE == EioUartState[Channel]->InitState);
    DEVICE_ASSERT(BaudRate != NULL_PTR);

    uint32 InputClock;
    uint16 Divider;
    uint16 TimerCmp;
    uint8 TxResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    uint8 RxResourceIndex;

    const Eio_UartStateType *ChannelState = EioUartState[Channel];
    const EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];
    TxResourceIndex = ChannelState->TxEioCommon.ResourceIndex;
    RxResourceIndex = ChannelState->RxEioCommon.ResourceIndex;

    /* Get the protocol clock frequency */
    (void)Ckgen_Hal_GetFreq(Eio_HalClock[ChannelState->EioInstance], &InputClock);
    DEVICE_ASSERT(InputClock > 0U);

    /* Get the currently configured divider */
    if (((uint8)ChannelState->Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U)
    /*PRQA S 2842 -- */
    {
        TimerCmp = Eio_Reg_GetTimerCompare(BaseAddr, UART_TX_TIMER(TxResourceIndex));
    }
    else
    {
        TimerCmp = Eio_Reg_GetTimerCompare(BaseAddr, UART_RX_TIMER(RxResourceIndex));
    }
    Divider = (uint16)(TimerCmp & 0x00FFU);

    /*PRQA S 2812 ++ # AMCE_EIO_2812.*/
    /* Compute baud rate: input_clock / (2 * (divider + 1)). Round to nearest integer */
    *BaudRate = (InputClock + (uint32)Divider + 1U) / (2U * ((uint32)Divider + 1U));
    /*PRQA S 2812 -- */
}

/* Implementation of Eio_Uart_Hal_SendData declared in Eio_Uart_Hal.h */
Hal_StatusType Eio_Uart_Hal_SendData(uint8 Channel, const uint8 *TxBuff, uint32 TxSize)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);
    DEVICE_ASSERT(TRUE == EioUartState[Channel]->InitState);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);
    DEVICE_ASSERT(((uint8)EioUartState[Channel]->Direction & (uint8)EIO_UART_DIRECTION_TX) != 0U);
    /* If 2 bytes per word are needed, then the size must be even */
    DEVICE_ASSERT((EioUartState[Channel]->BitCount <= 8U) || ((TxSize & 1U) == 0U));

    uint8 ResourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    Hal_StatusType RetCode = STATUS_SUCCESS;

    Eio_UartStateType *ChannelState = EioUartState[Channel];
    EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];
    ResourceIndex = ChannelState->TxEioCommon.ResourceIndex;

    /* Check if driver is busy */
    if (TRUE == ChannelState->IsTxBusy)
    {
        RetCode =  STATUS_BUSY;
    }
    else
    {
        ChannelState->TxData = (const uint8 *)TxBuff;
        ChannelState->TxRemainingBytes = TxSize;
        ChannelState->TxStatus = STATUS_BUSY;
        ChannelState->IsTxBusy = TRUE;
        /* Number of bytes to flush after the last byte is copied in the tx shifter buffer */
        ChannelState->TxFlush = (uint8)((TxSize == 1U) ? 1U : 2U);

        /* Enable timers and shifters */
        Eio_Uart_Hal_EnableTransfer(Channel, EIO_UART_DIRECTION_TX);
        /* Enable transfer engine */
        switch (ChannelState->DriverType)
        /*PRQA S 2842 -- */
        {
            case EIO_DRIVER_TYPE_INTERRUPTS:
                /* Enable interrupts for Tx shifter */
                Eio_Reg_SetShifterInterrupt(BaseAddr, (uint8)(1U << UART_TX_SHIFTER(ResourceIndex)), TRUE);
                Eio_Reg_SetShifterErrorInterrupt(BaseAddr, (uint8)(1U << UART_TX_SHIFTER(ResourceIndex)), TRUE);
                break;

            case EIO_DRIVER_TYPE_POLLING:
                /* Call Eio_Uart_Hal_CheckStatus once to send the first byte */
                Eio_Uart_Hal_CheckStatus(Channel, EIO_UART_DIRECTION_TX);
                break;

            case EIO_DRIVER_TYPE_DMA:
                RetCode = Eio_Uart_Hal_StartTxDmaTransfer(Channel);
                break;

            default:
                /* Impossible type - do nothing */
                break;
        }
    }

    return RetCode;
}

/* Implementation of Eio_Uart_Hal_ReceiveData declared in Eio_Uart_Hal.h */
Hal_StatusType Eio_Uart_Hal_ReceiveData(uint8 Channel, uint8 *RxBuff, uint32 RxSize)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);
    DEVICE_ASSERT(TRUE == EioUartState[Channel]->InitState);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    DEVICE_ASSERT(((uint8)EioUartState[Channel]->Direction & (uint8)EIO_UART_DIRECTION_RX) != 0U);
    /* If 2 bytes per word are needed, then the size must be even */
    DEVICE_ASSERT((EioUartState[Channel]->BitCount <= 8U) || ((RxSize & 1U) == 0U));

    Hal_StatusType RetCode = STATUS_SUCCESS;
    Eio_UartStateType *ChannelState = EioUartState[Channel];
    EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];
    uint8 ResourceIndex = ChannelState->RxEioCommon.ResourceIndex;

    /* Check if driver is busy */
    if (TRUE == ChannelState->IsRxBusy)
    {
        RetCode = STATUS_BUSY;
    }
    else
    {
        ChannelState->RxData = RxBuff;
        ChannelState->RxRemainingBytes = RxSize;
        ChannelState->RxStatus = STATUS_BUSY;
        ChannelState->IsRxBusy = TRUE;

        /* Enable timers and shifters */
        Eio_Uart_Hal_EnableTransfer(Channel, EIO_UART_DIRECTION_RX);
        /* Enable transfer engine */
        switch (ChannelState->DriverType)
        /*PRQA S 2842 -- */
        {
            case EIO_DRIVER_TYPE_INTERRUPTS:
                /* Enable interrupts for Rx shifter */
                Eio_Reg_SetShifterInterrupt(BaseAddr, (uint8)(1U << UART_RX_SHIFTER(ResourceIndex)), TRUE);
                Eio_Reg_SetShifterErrorInterrupt(BaseAddr, (uint8)(1U << UART_RX_SHIFTER(ResourceIndex)), TRUE);
                break;

            case EIO_DRIVER_TYPE_POLLING:
                /* Call Eio_Uart_Hal_CheckStatus once to send the first byte */
                Eio_Uart_Hal_CheckStatus(Channel, EIO_UART_DIRECTION_RX);
                break;

            case EIO_DRIVER_TYPE_DMA:
                if (STATUS_SUCCESS != Eio_Uart_Hal_StartRxDmaTransfer(Channel))
                {
                    RetCode = STATUS_ERROR;
                }
                break;

            default:
                /* Impossible type - do nothing */
                break;
        }
    }

    return RetCode;
}

/* Implementation of Eio_Uart_Hal_AbortSendingData declared in Eio_Uart_Hal.h */
Hal_StatusType Eio_Uart_Hal_AbortSendingData(uint8 Channel)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);
    DEVICE_ASSERT(TRUE == EioUartState[Channel]->InitState);

    Eio_UartStateType *ChannelState = EioUartState[Channel];
    EIO_Type *BaseAddr = EioBase[ChannelState->EioInstance];

    /* Check if driver is busy */
    if (ChannelState->IsTxBusy == TRUE)
    {
        if (ChannelState->DriverType == EIO_DRIVER_TYPE_DMA)
        {
            (void)Dma_Hal_StopCh(ChannelState->TxDmaChannel);
        }

        ChannelState->TxStatus = STATUS_UART_ABORTED;
        ChannelState->TxRemainingBytes = 0U;
        Eio_Reg_ClearTimerStatus(BaseAddr, UART_TX_TIMER(ChannelState->TxEioCommon.ResourceIndex));
        Eio_Reg_SetTimerInterrupt(BaseAddr,
                (uint8)(1U << UART_TX_TIMER(ChannelState->TxEioCommon.ResourceIndex)), TRUE);
    }
    /*PRQA S 2842 -- */

    return STATUS_SUCCESS;
}

/* Implementation of Eio_Uart_Hal_AbortReceivingData declared in Eio_Uart_Hal.h */
Hal_StatusType Eio_Uart_Hal_AbortReceivingData(uint8 Channel)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);
    DEVICE_ASSERT(TRUE == EioUartState[Channel]->InitState);

    Eio_UartStateType *ChannelState = EioUartState[Channel];
    /* Check if driver is busy */
    if (ChannelState->IsRxBusy == TRUE)
    {
        ChannelState->RxStatus = STATUS_UART_ABORTED;
        Eio_Uart_Hal_StopTransfer(Channel, EIO_UART_DIRECTION_RX);
    }
    /*PRQA S 2842 -- */

    return STATUS_SUCCESS;
}

/* Implementation of Eio_Uart_Hal_GetSendStatus declared in Eio_Uart_Hal.h */
Hal_StatusType Eio_Uart_Hal_GetSendStatus(uint8 Channel, uint32 *BytesRemaining)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);

    uint32 RemainingBytes;

    const Eio_UartStateType *ChannelState = EioUartState[Channel];
    /*PRQA S 2842 -- */

    /* Initialize with the actual remaining bytes count */
    RemainingBytes = ChannelState->TxRemainingBytes;

    if (TRUE == ChannelState->IsTxBusy)
    {
        switch (ChannelState->DriverType)
        {
            case EIO_DRIVER_TYPE_POLLING:
                /* In polling mode advance the UART transfer here */
                Eio_Uart_Hal_CheckStatus(Channel, EIO_UART_DIRECTION_TX);
                break;

            case EIO_DRIVER_TYPE_DMA:
                /* In DMA mode just update the remaining count.
                DO NOT write State->RemainingBytes directly !!! */
                RemainingBytes = ChannelState->TxRemainingBytes -
                        Dma_Hal_GetTransBytes(ChannelState->TxDmaChannel);
                break;

            default:
                /* Nothing */
                break;
        }
    }

    if (BytesRemaining != NULL_PTR)
    {
        *BytesRemaining = RemainingBytes;
    }

    return ChannelState->TxStatus;
}

/* Implementation of Eio_Uart_Hal_GetReceiveStatus declared in Eio_Uart_Hal.h */
Hal_StatusType Eio_Uart_Hal_GetReceiveStatus(uint8 Channel, uint32 *BytesRemaining)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);

    uint32 RemainingBytes;

    const Eio_UartStateType *ChannelState = EioUartState[Channel];
    /*PRQA S 2842 -- */
    /* Initialize with the actual remaining bytes count */
    RemainingBytes = ChannelState->RxRemainingBytes;

    if (TRUE == ChannelState->IsRxBusy)
    {
        switch (ChannelState->DriverType)
        {
            case EIO_DRIVER_TYPE_POLLING:
                /* In polling mode advance the UART transfer here */
                Eio_Uart_Hal_CheckStatus(Channel, EIO_UART_DIRECTION_RX);
                break;

            case EIO_DRIVER_TYPE_DMA:
                /* In DMA mode just update the remaining count.
                DO NOT write State->RemainingBytes directly !!! */
                RemainingBytes = ChannelState->RxRemainingBytes -
                        Dma_Hal_GetTransBytes(ChannelState->RxDmaChannel);
                break;

            default:
                /* Nothing */
                break;
        }
    }

    if (BytesRemaining != NULL_PTR)
    {
        *BytesRemaining = RemainingBytes;
    }

    return ChannelState->RxStatus;
}

/* Implementation of Eio_Uart_Hal_SetRxBuffer declared in Eio_Uart_Hal.h */
void Eio_Uart_Hal_SetRxBuffer(uint8 Channel, uint8 *RxBuff, uint32 RxSize)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);

    Eio_UartStateType *ChannelState = EioUartState[Channel];
    /*PRQA S 2842 -- */
    ChannelState->RxData = RxBuff;
    ChannelState->RxRemainingBytes = RxSize;
}

/* Implementation of Eio_Uart_Hal_SetTxBuffer declared in Eio_Uart_Hal.h */
void Eio_Uart_Hal_SetTxBuffer(uint8 Channel, const uint8 *TxBuff, uint32 TxSize)
{
    DEVICE_ASSERT(Channel < EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioUartState[Channel] != NULL_PTR);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);

    Eio_UartStateType *ChannelState = EioUartState[Channel];
    /*PRQA S 2842 -- */
    ChannelState->TxData = TxBuff;
    ChannelState->TxRemainingBytes = TxSize;
    /* In case of continuous transmission there are always 2 bytes to flush */
    ChannelState->TxFlush = 2U;
}

/* Implementation of Eio_Uart_Hal_GetDefaultConfig declared in Eio_Uart_Hal.h */
void Eio_Uart_Hal_GetDefaultConfig(Eio_UartUserConfigType *UserConfigPtr)
{
    DEVICE_ASSERT(UserConfigPtr != NULL_PTR);

    if (UserConfigPtr != NULL_PTR)
    {
        *UserConfigPtr = DefaultEioUartConfig;
    }
}

/*============================================EOF===================================================*/

