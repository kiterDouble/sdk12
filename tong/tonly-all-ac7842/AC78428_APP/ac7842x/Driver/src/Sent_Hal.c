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
 * @file Sent_Hal.c
 *
 * @brief This file provides sent integration functions.
 *
 */

/* PRQA S 1503 EOF # AMCE_QAC_NoUsedApi */
/* ===========================================  INCLUDE FILES  =========================================== */
#include "AC784xx_Sent_Reg.h"
#include "Sent_Hal.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"

/* ============================================  DEFINES AND MACROS  ============================================ */
/**
 * @def IRQ_RSI
 * @brief Receive success interrupt flag.
 * @note DES ID: DES_SENT_MACRO_001
 */
#define IRQ_RSI                 (0x1UL)

/**
 * @def IRQ_RDI
 * @brief Receive data interrupt flag.
 * @note DES ID: DES_SENT_MACRO_002
 */
#define IRQ_RDI                 (0x2UL)

/**
 * @def IRQ_RFI
 * @brief Receive FIFO overflow interrupt flag.
 * @note DES ID: DES_SENT_MACRO_003
 */
#define IRQ_RFI                 (0x4UL)

/**
 * @def IRQ_TDI
 * @brief Transmit data interrupt flag.
 * @note DES ID: DES_SENT_MACRO_004
 */
#define IRQ_TDI                 (0x8UL)

/**
 * @def IRQ_TBI
 * @brief Transmit buffer underflow interrupt flag.
 * @note DES ID: DES_SENT_MACRO_005
 */
#define IRQ_TBI                 (0x10UL)

/**
 * @def IRQ_FRI
 * @brief Frequency range error interrupt flag.
 * @note DES ID: DES_SENT_MACRO_006
 */
#define IRQ_FRI                 (0x20UL)

/**
 * @def IRQ_FDI
 * @brief Frequency drift error interrupt flag.
 * @note DES ID: DES_SENT_MACRO_007
 */
#define IRQ_FDI                 (0x40UL)

/**
 * @def IRQ_NNI
 * @brief Nibble number error interrupt flag.
 * @note DES ID: DES_SENT_MACRO_008
 */
#define IRQ_NNI                 (0x80UL)

/**
 * @def IRQ_NVI
 * @brief Nibble value error interrupt flag.
 * @note DES ID: DES_SENT_MACRO_009
 */
#define IRQ_NVI                 (0x100UL)

/**
 * @def IRQ_CRCI
 * @brief CRC error interrupt flag.
 * @note DES ID: DES_SENT_MACRO_010
 */
#define IRQ_CRCI                (0x200UL)

/**
 * @def IRQ_WSI
 * @brief Short start bit error interrupt flag.
 * @note DES ID: DES_SENT_MACRO_011
 */
#define IRQ_WSI                 (0x400UL)

/**
 * @def IRQ_SDI
 * @brief Serial data interrupt flag.
 * @note DES ID: DES_SENT_MACRO_012
 */
#define IRQ_SDI                 (0x800UL)

/**
 * @def IRQ_SCRI
 * @brief Serial message CRC error interrupt flag.
 * @note DES ID: DES_SENT_MACRO_013
 */
#define IRQ_SCRI                (0x1000UL)

/**
 * @def IRQ_WDI
 * @brief Watchdog timeout interrupt flag.
 * @note DES ID: DES_SENT_MACRO_014
 */
#define IRQ_WDI                 (0x2000UL)

/**
 * @def IRQ_ESI
 * @brief Enhanced start bit error interrupt flag.
 * @note DES ID: DES_SENT_MACRO_015
 */
#define IRQ_ESI                 (0x4000UL)

/**
 * @def IRQ_HFLL
 * @brief FIFO half full interrupt flag.
 * @note DES ID: DES_SENT_MACRO_016
 */
#define IRQ_HFLL                (0x8000UL)

/**
 * @def IRQ_FDONE
 * @brief Frame done interrupt flag.
 * @note DES ID: DES_SENT_MACRO_017
 */
#define IRQ_FDONE               (0x10000UL)

/**
 * @def IRQ_RECEIVE
 * @brief Combined receive interrupt flags mask.
 * @note DES ID: DES_SENT_MACRO_018
 */
#define IRQ_RECEIVE             (IRQ_RSI | IRQ_RDI | IRQ_SDI | IRQ_HFLL | IRQ_FDONE)

/**
 * @def IRQ_TRANSMIT
 * @brief Combined transmit interrupt flags mask.
 * @note DES ID: DES_SENT_MACRO_019
 */
#define IRQ_TRANSMIT            (IRQ_TDI)

/**
 * @def IRQ_ERROR
 * @brief Combined error interrupt flags mask.
 * @note DES ID: DES_SENT_MACRO_020
 */
#define IRQ_ERROR (IRQ_RFI | IRQ_TBI | IRQ_FRI | IRQ_FDI | IRQ_NNI \
                   | IRQ_NVI | IRQ_CRCI | IRQ_WSI | IRQ_SCRI | IRQ_WDI | IRQ_ESI)

/* ============================================= TYPEDEFS ================================================ */
/**
 * @brief ATC SENT runtime status.
 * @note DES ID: DES_SENT_TYP_005
 */
typedef struct
{
    Sent_MsgType SentMsg;             /*!< sent message frame */
    uint32 EnabledInterrupts;         /*!< interrupts control */
    Sent_RxCallbackType RxCallback;   /*!< sent receive callback function */
    Sent_CallbackType TxCallback;     /*!< sent transmit callback user data */
    Sent_CallbackType ErrCallback;    /*!< sent channel error callback function */
} Sent_StateType;

/* =========================================== LOCAL VARIABLES ============================================== */
/**
 * @brief SENT channel 0 runtime state.
 * @note DES ID: DES_SENT_VAR_001
 */
#if (CONFIG_SENT_CHANNEL0_ENABLE)
static Sent_StateType Sent_Channel0State;
#endif
/**
 * @brief SENT channel 1 runtime state.
 * @note DES ID: DES_SENT_VAR_002
 */
#if (CONFIG_SENT_CHANNEL1_ENABLE)
static Sent_StateType Sent_Channel1State;
#endif

#if defined (AC7843X)
/**
 * @brief SENT channel 2 runtime state.
 * @note DES ID: DES_SENT_VAR_003
 */
#if (CONFIG_SENT_CHANNEL2_ENABLE)
static Sent_StateType Sent_Channel2State;
#endif
/**
 * @brief SENT channel 3 runtime state.
 * @note DES ID: DES_SENT_VAR_004
 */
#if (CONFIG_SENT_CHANNEL3_ENABLE)
static Sent_StateType Sent_Channel3State;
#endif
#endif

/**
 * @brief Array of pointers to SENT channel state structures.
 * @note DES ID: DES_SENT_VAR_005
 */
static Sent_StateType *const Sent_ChannelStateArray[SENT_INSTANCE_MAX] =
{
#if (CONFIG_SENT_CHANNEL0_ENABLE)
    &Sent_Channel0State,
#else
    NULL_PTR,
#endif
#if (CONFIG_SENT_CHANNEL1_ENABLE)
    &Sent_Channel1State,
#else
    NULL_PTR,
#endif

#if defined (AC7843X)
#if (CONFIG_SENT_CHANNEL2_ENABLE)
    &Sent_Channel2State,
#else
    NULL_PTR,
#endif
#if (CONFIG_SENT_CHANNEL3_ENABLE)
    &Sent_Channel3State,
#else
    NULL_PTR,
#endif
#endif
};

/**
 * @brief Array of base addresses for SENT channel instances.
 * @note DES ID: DES_SENT_VAR_006
 */
static SENT_CHANNEL_Type *const SentChannelBase[SENT_INSTANCE_MAX] = SENT_BASE_PTRS;

/**
 * @brief Base address for SENT module control registers.
 * @note DES ID: DES_SENT_VAR_007
 */
static SENT_CTRL_Type *const SentModuleBase = SENT_CTRL;

/**
 * @brief Array of IRQ numbers for SENT channel instances.
 * @note DES ID: DES_SENT_VAR_008
 */
static const IRQn_Type SentIrqId[SENT_INSTANCE_MAX] = SENT_IRQS;

#ifndef SENT_SDK_NON_EXTENDED_API
/*PRQA S 3218 ++ # AMCE_SENT_3218. */
/**
 * @brief Default SENT channel configuration structure.
 * @note DES ID: DES_SENT_VAR_009
 */
static const Sent_ChannelConfigType DefaultSentChannelConfig =
{
    .WatchdogTimerLimit = 0,
    .IoControl =
    {
        .InputInvertEnable = FALSE,
        .OutputInvertEnable = FALSE,
        .GlitchFilterDepth = 2U,
        .Input = SENT_INPUT_DATA_SELECT_0,
        .TriggerSource = SENT_EXTERNAL_TRIGGER_0,
    },
    .RxControl =
    {
        .PausePulseEnable = FALSE,
        .AlternateCRCEnable = FALSE,
        .StatusIncludedInCRC = FALSE,
        .SerialDataEnable = FALSE,
        .SerialCRCEnable = FALSE,
        .FastCRCEnable = FALSE,
        .FrameCheckMode = SENT_FRAME_CHECK_PAST_SYNC_PULSE,
        .FrameLength = 6U,
        .ZeroAugumentInCRCDisable = FALSE,
        .SerialMsgType = SENT_SHORT_SERIAL_MSG,
        .DriftErrorDisable = FALSE,
        .FDFL = FALSE,
    },
    .TickUnit = 3E-6F,
    .EnabledInterrupts = 0xffffffffU,
    .NibblePointer =
    {
        .Nibble7Position = SENT_NIBBLE_POSITION_7,
        .Nibble6Position = SENT_NIBBLE_POSITION_6,
        .Nibble5Position = SENT_NIBBLE_POSITION_5,
        .Nibble4Position = SENT_NIBBLE_POSITION_4,
        .Nibble3Position = SENT_NIBBLE_POSITION_3,
        .Nibble2Position = SENT_NIBBLE_POSITION_2,
        .Nibble1Position = SENT_NIBBLE_POSITION_1,
        .Nibble0Position = SENT_NIBBLE_POSITION_0,
    },
    .RxCallback = NULL_PTR,
    .TxCallback = NULL_PTR,
    .ErrCallback = NULL_PTR,
};

/**
 * @brief Default SENT module configuration structure.
 * @note DES ID: DES_SENT_VAR_010
 */
static const Sent_ModuleConfigType DefaultSentModuleConfig =
{
    .FractionDivider = 1U,
    .TimeStampPreDivider = 0U,
};
/*PRQA S 3218 -- */
#endif

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief Initializes the SENT channel IO configuration.
 *
 * @details
 * This function configures the input/output control settings for a SENT channel,
 * including input/output inversion, glitch filter depth, input data source,
 * and external trigger source selection.
 *
 * @param[in] ChannelId: SENT channel identifier.
 * @param[in] SentChannelIoConfig: Pointer to SENT channel IO configuration structure.
 *
 * @return void
 *
 * @pre
 *     SentChannelIoConfig must point to a valid configuration structure.
 *
 * @post
 *     The SENT channel IO control settings are configured.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; not reentrant for the same channel.
 *     DES ID: DES_SENT_API_001
 */
static void Sent_Hal_InitChannelIoCtrl(Sent_ChannelIdType ChannelId, const Sent_IoCtrlType *SentChannelIoConfig);

/**
 * @brief Initializes the SENT channel receive configuration.
 *
 * @details
 * This function configures the receive control settings for a SENT channel,
 * including pause pulse, CRC options, frame check mode, frame length,
 * serial message type, and error handling settings.
 *
 * @param[in] ChannelId: SENT channel identifier.
 * @param[in] SentChannelRxConfig: Pointer to SENT channel receive configuration structure.
 *
 * @return void
 *
 * @pre
 *     SentChannelRxConfig must point to a valid configuration structure.
 *
 * @post
 *     The SENT channel receive control settings are configured.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; not reentrant for the same channel.
 *     DES ID: DES_SENT_API_002
 */
static void Sent_Hal_InitChannelRxCtrl(Sent_ChannelIdType ChannelId, const Sent_ReceiveCtrlType *SentChannelRxConfig);

/**
 * @brief Initializes the SENT channel nibble order configuration.
 *
 * @details
 * This function configures the data nibble position mapping for a SENT channel.
 * It sets how the 8 data nibbles are organized and accessed in the received frame.
 *
 * @param[in] ChannelId: SENT channel identifier.
 * @param[in] SentNibbleOrder: Pointer to SENT channel nibble position configuration structure.
 *
 * @return void
 *
 * @pre
 *     SentNibbleOrder must point to a valid configuration structure.
 *
 * @post
 *     The SENT channel nibble position view is configured.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; not reentrant for the same channel.
 *     DES ID: DES_SENT_API_003
 */
static void Sent_Hal_InitChannelNibbleOrder(Sent_ChannelIdType ChannelId,
        const Sent_NibblePointerType *SentNibbleOrder);

/**
 * @brief Initializes the SENT channel tick time configuration.
 *
 * @details
 * This function calculates and configures the channel clock divider (predivider and
 * fractional divider) based on the desired tick unit time. The tick time is fundamental
 * to SENT protocol timing and must match the sensor's tick time specification.
 *
 * @param[in] ChannelId: SENT channel identifier.
 * @param[in] TickUnit: Desired tick time unit in seconds.
 *
 * @return void
 *
 * @pre
 *     The SENT module must be initialized and clocked.
 *     TickUnit must be a valid positive value.
 *
 * @post
 *     The SENT channel predivider and fractional divider are configured.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; not reentrant for the same channel.
 *     The function automatically calculates the optimal divider values.
 *     DES ID: DES_SENT_API_004
 */
static void Sent_Hal_InitChannelTickTime(Sent_ChannelIdType ChannelId, float32 TickUnit);

/**
 * @brief SENT channel interrupt handler.
 *
 * @details
 * This function is the main interrupt handler for a SENT channel. It reads the
 * interrupt status, determines the interrupt source (receive, transmit, or error),
 * and dispatches to the appropriate sub-handler. After processing, it clears
 * the interrupt flags.
 *
 * @param[in] ChannelId: SENT channel identifier.
 *
 * @return void
 *
 * @pre
 *     The SENT channel must be initialized and enabled.
 *     Called from ISR context.
 *
 * @post
 *     All pending interrupts for the channel are processed and cleared.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; not reentrant for the same channel.
 *     This function is called from ISR context.
 *     DES ID: DES_SENT_API_005
 */
static void Sent_Hal_IrqHandler(Sent_ChannelIdType ChannelId);

/**
 * @brief SENT receive interrupt handler function.
 *
 * @details
 * This function handles receive-related interrupts for a SENT channel. It processes
 * received frame data, serial message data, and FIFO status updates. If a receive
 * callback is registered, it invokes the callback with the received message data.
 *
 * @param[in] ChannelId: SENT channel identifier.
 * @param[in] InterruptStatus: Current interrupt status register value.
 *
 * @return void
 *
 * @pre
 *     Called from Sent_Hal_IrqHandler in ISR context.
 *     InterruptStatus contains valid receive interrupt flags.
 *
 * @post
 *     Received data is stored in channel state structure.
 *     Receive callback is invoked if registered.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; not reentrant for the same channel.
 *     This function is called from ISR context.
 *     DES ID: DES_SENT_API_006
 */
static void Sent_Hal_RxIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus);

/**
 * @brief SENT transmit interrupt handler function.
 *
 * @details
 * This function handles transmit-related interrupts for a SENT channel,
 * specifically the transmit data interrupt. If a transmit callback is registered,
 * it invokes the callback with the transmit event.
 *
 * @param[in] ChannelId: SENT channel identifier.
 * @param[in] InterruptStatus: Current interrupt status register value.
 *
 * @return void
 *
 * @pre
 *     Called from Sent_Hal_IrqHandler in ISR context.
 *     InterruptStatus contains valid transmit interrupt flags.
 *
 * @post
 *     Transmit callback is invoked if registered.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; not reentrant for the same channel.
 *     This function is called from ISR context.
 *     DES ID: DES_SENT_API_007
 */
static void Sent_Hal_TxIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus);

/**
 * @brief SENT error interrupt handler function.
 *
 * @details
 * This function handles error-related interrupts for a SENT channel, including
 * FIFO overflow, transmit buffer underflow, frequency errors, nibble errors,
 * CRC errors, and watchdog timeout. If an error callback is registered,
 * it invokes the callback with the error event flags.
 *
 * @param[in] ChannelId: SENT channel identifier.
 * @param[in] InterruptStatus: Current interrupt status register value.
 *
 * @return void
 *
 * @pre
 *     Called from Sent_Hal_IrqHandler in ISR context.
 *     InterruptStatus contains valid error interrupt flags.
 *
 * @post
 *     FIFO is flushed if overflow occurs.
 *     Error callback is invoked if registered.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; not reentrant for the same channel.
 *     This function is called from ISR context.
 *     DES ID: DES_SENT_API_008
 */
static void Sent_Hal_ErrIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus);

/*===============================GLOBAL FUNCTION  IMPLEMENTATIONS=================================*/
#ifndef SENT_SDK_NON_EXTENDED_API
/* Implementation of Sent_Hal_GetDefaultChannelConfig declared in Sent_Hal.h */
void Sent_Hal_GetDefaultChannelConfig(Sent_ChannelConfigType *SentChannelConfig)
{
    *SentChannelConfig = DefaultSentChannelConfig;
}

/* Implementation of Sent_Hal_GetDefaultModuleConfig declared in Sent_Hal.h */
void Sent_Hal_GetDefaultModuleConfig(Sent_ModuleConfigType *SentModuleConfig)
{
    *SentModuleConfig = DefaultSentModuleConfig;
}
#endif

/* Implementation of Sent_Hal_Init declared in Sent_Hal.h */
void Sent_Hal_Init(const Sent_ModuleConfigType *SentModuleConfig)
{
    DEVICE_ASSERT(SentModuleConfig != NULL_PTR);
    /*PRQA S 2812 ++ # AMCE_SENT_2812.*/
    DEVICE_ASSERT(SentModuleConfig->FractionDivider > 0U);
    /*PRQA S 2812 -- */
    uint8 Idx;

    (void)Ckgen_Hal_EnablePeriphClk(CKGEN_SENT_BUS_CLK, TRUE);
    Rcm_Hal_SetResetState(RCM_RESET_ID_SENT, RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(RCM_RESET_ID_SENT, RCM_RESET_STATE_DEASSERT);

    Sent_Reg_SetModuleClkDivider(SentModuleBase, SentModuleConfig->FractionDivider);
    Sent_Reg_SetModuleTimeStampDivider(SentModuleBase, SentModuleConfig->TimeStampPreDivider);

    /* Enable SENT NVIC interrupt */
    for (Idx = 0U; Idx < SENT_INSTANCE_MAX; Idx++)
    {
        Core_Hal_EnableIrq(SentIrqId[Idx]);
    }
}

/* Implementation of Sent_Hal_Deinit declared in Sent_Hal.h */
void Sent_Hal_Deinit(void)
{
    uint8 Idx;

    /* Disable SENT clock */
    Rcm_Hal_SetResetState(RCM_RESET_ID_SENT, RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(RCM_RESET_ID_SENT, RCM_RESET_STATE_DEASSERT);
    (void)Ckgen_Hal_EnablePeriphClk(CKGEN_SENT_BUS_CLK, FALSE);

    /* Disable SENT NVIC interrupt. */
    for (Idx = 0U; Idx < SENT_INSTANCE_MAX; Idx++)
    {
        Core_Hal_DisableIrq(SentIrqId[Idx]);
        Core_Hal_ClearPendingIrq(SentIrqId[Idx]);
    }
}

/* Implementation of Sent_Hal_ConfigChannel declared in Sent_Hal.h */
void Sent_Hal_ConfigChannel(Sent_ChannelIdType ChannelId,
                            const Sent_ChannelConfigType *SentChannelConfig)
{
    DEVICE_ASSERT(SentChannelConfig != NULL_PTR);
    DEVICE_ASSERT(Sent_ChannelStateArray[ChannelId] != NULL_PTR);

    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];
    Sent_StateType *SentChannelState = Sent_ChannelStateArray[ChannelId];

    /* disable channel */
    Sent_Hal_EnableChannel(ChannelId, FALSE);

    /* Set SENT channel config */
    /*PRQA S 2812 ++ # AMCE_SENT_2812.*/
    Sent_Reg_SetWatchdogLimit(ChannelBase, SentChannelConfig->WatchdogTimerLimit);
    /*PRQA S 2812 -- */
    Sent_Hal_InitChannelIoCtrl(ChannelId, &(SentChannelConfig->IoControl));
    Sent_Hal_InitChannelRxCtrl(ChannelId, &(SentChannelConfig->RxControl));

    Sent_Hal_InitChannelTickTime(ChannelId, SentChannelConfig->TickUnit);
    Sent_Reg_EnableIrq(ChannelBase, SentChannelConfig->EnabledInterrupts);
    Sent_Hal_InitChannelNibbleOrder(ChannelId, &(SentChannelConfig->NibblePointer));

    SentChannelState->EnabledInterrupts = SentChannelConfig->EnabledInterrupts;
    SentChannelState->RxCallback = SentChannelConfig->RxCallback;
    SentChannelState->TxCallback = SentChannelConfig->TxCallback;
    SentChannelState->ErrCallback = SentChannelConfig->ErrCallback;
}

/*PRQA S 1505 ++ # AMCE_SENT_1505. */
/* Implementation of Sent_Hal_EnableChannel declared in Sent_Hal.h */
void Sent_Hal_EnableChannel(Sent_ChannelIdType ChannelId, boolean Enable)
{
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    Sent_Reg_EnableChannel(ChannelBase, Enable);
}
/*PRQA S 1505 -- */

/* Implementation of Sent_Hal_InitChannelTxCtrl declared in Sent_Hal.h */
void Sent_Hal_InitChannelTxCtrl(Sent_ChannelIdType ChannelId, const Sent_TransmitCtrlType *SentChannelTxConfig)
{
    DEVICE_ASSERT(SentChannelTxConfig != NULL_PTR);
    uint32 params = 0;
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    /*PRQA S 2812 ++ # AMCE_SENT_2812.*/
    params = ((uint32)(SentChannelTxConfig->PulseLength) & SENT_CHANNEL_SCR_PLEN_Msk);
    /*PRQA S 2812 -- */
    params |= (((uint32)(SentChannelTxConfig->PulseDelayLength) << SENT_CHANNEL_SCR_DEL_Pos) \
               & SENT_CHANNEL_SCR_DEL_Msk);
    params |= (((uint32)(SentChannelTxConfig->Mode) << SENT_CHANNEL_SCR_TRIG_Pos) & SENT_CHANNEL_SCR_TRIG_Msk);
    params |= (((uint32)(SentChannelTxConfig->TimeBase) << SENT_CHANNEL_SCR_BASE_Pos) & SENT_CHANNEL_SCR_BASE_Msk);

    Sent_Reg_SetSPCParams(ChannelBase, params);
}

/* Implementation of Sent_Hal_GetChannelStatus declared in Sent_Hal.h */
Sent_ChannelStatusType Sent_Hal_GetChannelStatus(Sent_ChannelIdType ChannelId)
{
    const SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    return Sent_Reg_GetChannelStatus(ChannelBase);
}

/*PRQA S 3408 ++ # AMCE_QAC_3408.*/
/**
 * @brief SENT channel 0 interrupt handler.
 *
 * @details
 * This is the interrupt service routine for SENT channel 0.
 * It calls the common interrupt handler with the channel 0 identifier.
 *
 * @return void
 *
 * @pre
 *     SENT channel 0 must be initialized and interrupts enabled.
 *
 * @post
 *     All pending interrupts for channel 0 are processed.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from ISR context.
 *     DES ID: DES_SENT_API_010
 */
ISR(SENT_Channel0_IRQHandler)
{
    Sent_Hal_IrqHandler(SENT_CHANNEL_0);
}

/**
 * @brief SENT channel 1 interrupt handler.
 *
 * @details
 * This is the interrupt service routine for SENT channel 1.
 * It calls the common interrupt handler with the channel 1 identifier.
 *
 * @return void
 *
 * @pre
 *     SENT channel 1 must be initialized and interrupts enabled.
 *
 * @post
 *     All pending interrupts for channel 1 are processed.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from ISR context.
 *     DES ID: DES_SENT_API_011
 */
ISR(SENT_Channel1_IRQHandler)
{
    Sent_Hal_IrqHandler(SENT_CHANNEL_1);
}

#if defined (AC7843X)
/**
 * @brief SENT channel 2 interrupt handler.
 *
 * @details
 * This is the interrupt service routine for SENT channel 2.
 * It calls the common interrupt handler with the channel 2 identifier.
 *
 * @return void
 *
 * @pre
 *     SENT channel 2 must be initialized and interrupts enabled.
 *
 * @post
 *     All pending interrupts for channel 2 are processed.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from ISR context.
 *     DES ID: DES_SENT_API_012
 */
ISR(SENT_Channel2_IRQHandler)
{
    Sent_Hal_IrqHandler(SENT_CHANNEL_2);
}

/**
 * @brief SENT channel 3 interrupt handler.
 *
 * @details
 * This is the interrupt service routine for SENT channel 3.
 * It calls the common interrupt handler with the channel 3 identifier.
 *
 * @return void
 *
 * @pre
 *     SENT channel 3 must be initialized and interrupts enabled.
 *
 * @post
 *     All pending interrupts for channel 3 are processed.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from ISR context.
 *     DES ID: DES_SENT_API_013
 */
ISR(SENT_Channel3_IRQHandler)
{
    Sent_Hal_IrqHandler(SENT_CHANNEL_3);
}
#endif
/*PRQA S 3408 -- */
/*================================STATIC  FUNCTION  IMPLEMENTATIONS=================================*/
static void Sent_Hal_InitChannelIoCtrl(Sent_ChannelIdType ChannelId, const Sent_IoCtrlType *SentChannelIoConfig)
{
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    Sent_Reg_EnableInputInvert(ChannelBase, SentChannelIoConfig->InputInvertEnable);
    Sent_Reg_EnableOutputInvert(ChannelBase, SentChannelIoConfig->OutputInvertEnable);
    Sent_Reg_SetGlitchFilterDepth(ChannelBase, SentChannelIoConfig->GlitchFilterDepth);
    Sent_Reg_SelectInputDataSource(ChannelBase, SentChannelIoConfig->Input);
    Sent_Reg_SelectExternalTriggerSource(ChannelBase, SentChannelIoConfig->TriggerSource);
}

static void Sent_Hal_InitChannelRxCtrl(Sent_ChannelIdType ChannelId, const Sent_ReceiveCtrlType *SentChannelRxConfig)
{
    DEVICE_ASSERT(SentChannelRxConfig != NULL_PTR);

    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    /*PRQA S 2812 ++ # AMCE_SENT_2812.*/
    Sent_Reg_EnablePausePulse(ChannelBase, SentChannelRxConfig->PausePulseEnable);
    /*PRQA S 2812 -- */
    Sent_Reg_EnableAlternateCRC(ChannelBase, SentChannelRxConfig->AlternateCRCEnable);
    Sent_Reg_EnableStatusNibbleInCRC(ChannelBase, SentChannelRxConfig->StatusIncludedInCRC);
    Sent_Reg_EnableSerialMsgProcess(ChannelBase, SentChannelRxConfig->SerialDataEnable);
    Sent_Reg_EnableSerialMsgCRC(ChannelBase, SentChannelRxConfig->SerialCRCEnable);
    Sent_Reg_EnableFastMsgCRC(ChannelBase, SentChannelRxConfig->FastCRCEnable);
    Sent_Reg_SetFrameCheckMode(ChannelBase, SentChannelRxConfig->FrameCheckMode);
    Sent_Reg_SetFrameLength(ChannelBase, SentChannelRxConfig->FrameLength);
    Sent_Reg_EnableCRCAugumentZero(ChannelBase, SentChannelRxConfig->ZeroAugumentInCRCDisable);
    Sent_Reg_SetSerialMsgType(ChannelBase, SentChannelRxConfig->SerialMsgType);
    Sent_Reg_DisableDriftError(ChannelBase, SentChannelRxConfig->DriftErrorDisable);
    Sent_Reg_EnableFDFLMode(ChannelBase, SentChannelRxConfig->FDFL);
}

static void Sent_Hal_InitChannelTickTime(Sent_ChannelIdType ChannelId, float32 TickUnit)
{
    uint32 SentSourceClock = 0U;
    float32 SendModuleDivClock, Tmp;
    uint32 Pdiv, Fdiv;

    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    /* Get the SENT clock as configured in the clock manager */
    (void)Ckgen_Hal_GetFreq(CKGEN_BUS_CLK, &SentSourceClock);

    SendModuleDivClock = (float32)(SentSourceClock) / (float32)(Sent_Reg_GetModuleClkDivider(SentModuleBase));

    Pdiv = 1U;
    Tmp = (SendModuleDivClock * (float32)56U * TickUnit) / (float32)Pdiv;
    Fdiv = (uint32)Tmp;

    /* DIV max: 49100; min: todo */
    while (Fdiv >= 49100U)
    {
        Pdiv = Pdiv << 1U;
        Tmp = (SendModuleDivClock * (float32)56U * TickUnit) / (float32)Pdiv;
        Fdiv = (uint32)Tmp;
    }

    Sent_Reg_SetChannelPreDivider(ChannelBase, (uint16)(Pdiv - 1U));
    Sent_Reg_SetChannelFractionalDivider(ChannelBase, (uint16)Fdiv);
}

static void Sent_Hal_InitChannelNibbleOrder(Sent_ChannelIdType ChannelId,
        const Sent_NibblePointerType *SentNibbleOrder)
{
    DEVICE_ASSERT(SentNibbleOrder != NULL_PTR);

    uint32 View;
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    /*PRQA S 2812 ++ # AMCE_SENT_2812.*/
    View = (uint32)(SentNibbleOrder->Nibble0Position)
           | ((uint32)(SentNibbleOrder->Nibble1Position) << 4U)
           | ((uint32)(SentNibbleOrder->Nibble2Position) << 8U)
           | ((uint32)(SentNibbleOrder->Nibble3Position) << 12U)
           | ((uint32)(SentNibbleOrder->Nibble4Position) << 16U)
           | ((uint32)(SentNibbleOrder->Nibble5Position) << 20U)
           | ((uint32)(SentNibbleOrder->Nibble6Position) << 24U)
           | ((uint32)(SentNibbleOrder->Nibble7Position) << 28U);
    /*PRQA S 2812 -- */

    Sent_Reg_SetDataNibbleView(ChannelBase, View);
}

static void Sent_Hal_IrqHandler(Sent_ChannelIdType ChannelId)
{
    uint32 InterruptStatus, EnabledInterrupts;
    const Sent_StateType *SentChannelState = Sent_ChannelStateArray[ChannelId];
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];

    InterruptStatus = Sent_Reg_GetIrqStatus(ChannelBase);
    EnabledInterrupts = SentChannelState->EnabledInterrupts;

    if (((EnabledInterrupts & (uint32)IRQ_RECEIVE) != 0U) && ((InterruptStatus & (uint32)IRQ_RECEIVE) != 0U))
    {
        Sent_Hal_RxIrqHandler(ChannelId, InterruptStatus);
    }

    if (((EnabledInterrupts & (uint32)IRQ_TRANSMIT) != 0U) && ((InterruptStatus & (uint32)IRQ_TRANSMIT) != 0U))
    {
        Sent_Hal_TxIrqHandler(ChannelId, InterruptStatus);
    }

    if (((EnabledInterrupts & (uint32)IRQ_ERROR) != 0U) && ((InterruptStatus & (uint32)IRQ_ERROR) != 0U))
    {
        Sent_Hal_ErrIrqHandler(ChannelId, InterruptStatus);
    }

    Sent_Reg_ClearIrqStatus(ChannelBase, InterruptStatus);
}

static void Sent_Hal_RxIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus)
{
    const SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];
    const SENT_CTRL_Type *ModuleBase = SentModuleBase;
    Sent_StateType *SentChannelState = Sent_ChannelStateArray[ChannelId];
    uint32 Event;

    SentChannelState->SentMsg.FifoStatus.FrameDataFifoAvail = Sent_Reg_GetFifoAvailable(ChannelBase);
    SentChannelState->SentMsg.FifoStatus.FrameDataFifoEmpty = (boolean)(Sent_Reg_GetFifoEmptyStatus(ChannelBase));
    SentChannelState->SentMsg.FifoStatus.FrameDataFifoFull = (boolean)(Sent_Reg_GetFifoFullStatus(ChannelBase));
    SentChannelState->SentMsg.FifoStatus.FrameDataFifoHalfFull = (boolean)(Sent_Reg_GetFifoHalfFullStatus(ChannelBase));

    if ((InterruptStatus & IRQ_RDI) != 0U)
    {
        SentChannelState->SentMsg.DataFrame.Data = Sent_Reg_GetFrameDataNibbles(ChannelBase);
        SentChannelState->SentMsg.DataFrame.FrameDataCRC = Sent_Reg_GetFrameCRCNibble(ChannelBase);
        SentChannelState->SentMsg.DataFrame.StatusNibble = Sent_Reg_GetFrameStatusNibble(ChannelBase);
        SentChannelState->SentMsg.DataFrame.TimeStamp = Sent_Reg_GetCurrentTimeStamp(ModuleBase);
        SentChannelState->SentMsg.DataFrame.TotalFrameLength = Sent_Reg_GetFrameLengthCounter(ChannelBase);
    }

    if ((InterruptStatus & IRQ_SDI) != 0U)
    {
        SentChannelState->SentMsg.SerialFrame.ConfigBit = Sent_Reg_GetEnhancedConfigType(ChannelBase);
        SentChannelState->SentMsg.SerialFrame.SerialMsgCRC = Sent_Reg_GetSerialMsgCRC(ChannelBase);
        SentChannelState->SentMsg.SerialFrame.SerialMsgData = Sent_Reg_GetSerialMsgData(ChannelBase);
        SentChannelState->SentMsg.SerialFrame.SerialMsgID = Sent_Reg_GetSerialMsgID(ChannelBase);
    }

    if (SentChannelState->RxCallback != NULL_PTR)
    {
        Event = 0U;

        if ((InterruptStatus & IRQ_RSI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_RECEIVE_SUCCESS;
        }

        if ((InterruptStatus & IRQ_RDI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_RECEIVE_DATA;
        }

        if ((InterruptStatus & IRQ_SDI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_RECEIVE_SERIAL_MSG;
        }

        if ((InterruptStatus & IRQ_HFLL) != 0U)
        {
            Event |= (uint32)SENT_EVENT_RECEIVE_FIFO_HALF_FULL;
        }

        if ((InterruptStatus & IRQ_FDONE) != 0U)
        {
            Event |= (uint32)SENT_EVENT_RECEIVE_FRAME_DONE;
        }

        SentChannelState->RxCallback(ChannelId, Event, &SentChannelState->SentMsg);
    }
}

static void Sent_Hal_TxIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus)
{
    const Sent_StateType *SentChannelState = Sent_ChannelStateArray[ChannelId];

    if (SentChannelState->TxCallback != NULL_PTR)
    {
        if ((InterruptStatus & IRQ_TDI) != 0U)
        {
            SentChannelState->TxCallback(ChannelId, (uint32)SENT_EVENT_TRANSMIT_DATA);
        }
    }
}

static void Sent_Hal_ErrIrqHandler(Sent_ChannelIdType ChannelId, uint32 InterruptStatus)
{
    SENT_CHANNEL_Type *ChannelBase = SentChannelBase[ChannelId];
    const Sent_StateType *SentChannelState = Sent_ChannelStateArray[ChannelId];
    uint32 Event;

    if ((InterruptStatus & IRQ_RFI) != 0U)
    {
        Sent_Reg_EnableFlushFifo(ChannelBase, TRUE);
        Sent_Reg_EnableFlushFifo(ChannelBase, FALSE);
    }

    if (SentChannelState->ErrCallback != NULL_PTR)
    {
        Event = 0U;

        if ((InterruptStatus & IRQ_RFI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_RECEIVE_FIFO_OVERFLOW;
        }

        if ((InterruptStatus & IRQ_TBI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_TRANSMIT_BUFFER_UNDERFLOW;
        }

        if ((InterruptStatus & IRQ_FRI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_FREQUENCY_RANGE;
        }

        if ((InterruptStatus & IRQ_FDI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_FREQUENCY_DRIFT;
        }

        if ((InterruptStatus & IRQ_NNI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_NIBBLE_NUMBER;
        }

        if ((InterruptStatus & IRQ_NVI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_NIBBLE_DATA;
        }

        if ((InterruptStatus & IRQ_CRCI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_NIBBLE_CRC;
        }

        if ((InterruptStatus & IRQ_WSI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_SHORT_START_BIT;
        }

        if ((InterruptStatus & IRQ_SCRI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_SERIAL_CRC;
        }

        if ((InterruptStatus & IRQ_WDI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_WATCH_DOG;
        }

        if ((InterruptStatus & IRQ_ESI) != 0U)
        {
            Event |= (uint32)SENT_EVENT_ERROR_ENHANCED_START_BIT;
        }

        SentChannelState->ErrCallback(ChannelId, Event);
    }
}

/* =============================================  EOF  ============================================== */
