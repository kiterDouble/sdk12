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
 * @file eio_i2s_drv.c
 *
 * @brief This file provides eio i2s module integration functions.
 *
 */

/* PRQA S 1503 EOF # AMCE_QAC_NoUsedApi */
/* ===========================================  Includes  =========================================== */
#include "Eio_I2s_Hal.h"
#include "AC784xx_Eio_Reg.h"
#include "Eio_Common_Hal.h"
#include "Dma_Hal.h"
#include "Ckgen_Hal.h"

/* ============================================  Define  ============================================ */
/*!
 * @def DIVIDER_MIN_VALUE
 * @brief Minimum allowed value for the baud rate divider
 * @note DES ID: DES_I2S_MACRO_001
 */
#define DIVIDER_MIN_VALUE (1)
/*!
 * @def DIVIDER_MAX_VALUE
 * @brief Maximum allowed value for the baud rate divider
 * @note DES ID: DES_I2S_MACRO_002
 */
#define DIVIDER_MAX_VALUE (0xFF)

/*!
 * @def I2S_TX_SHIFTER
 * @brief Compute the TX shifter index from resource index
 * @note DES ID: DES_I2S_MACRO_003
 */
#define I2S_TX_SHIFTER(x)           (x)
/*!
 * @def I2S_RX_SHIFTER
 * @brief Compute the RX shifter index from resource index
 * @note DES ID: DES_I2S_MACRO_004
 */
#define I2S_RX_SHIFTER(x)           (uint8)((x) + 1UL)
/*!
 * @def I2S_SCK_TIMER
 * @brief Compute the SCK timer index from resource index
 * @note DES ID: DES_I2S_MACRO_005
 */
#define I2S_SCK_TIMER(x)            (x)
/*!
 * @def I2S_WS_TIMER
 * @brief Compute the WS timer index from resource index
 * @note DES ID: DES_I2S_MACRO_006
 */
#define I2S_WS_TIMER(x)             (uint8)((x) + 1UL)
/*!
 * @def I2S_WS_TIMER_RISING_BLK
 * @brief Compute the WS rising block timer index from resource index
 * @note DES ID: DES_I2S_MACRO_007
 */
#define I2S_WS_TIMER_RISING_BLK(x)  (uint8)((x) + 2UL)

/*!
 * @def I2S_MAX_CHANNEL
 * @brief Maximum number of I2S channels supported
 * @note DES ID: DES_I2S_MACRO_008
 */
#define I2S_MAX_CHANNEL             (2U)

/* ===========================================  Typedef  ============================================ */
/*!
 * @brief EIO I2S driver internal state structure
 * @note DES ID: DES_I2S_TYP_005
 */
typedef struct
{
    boolean InitState;                     /*!< Initialization state flag */
    Eio_CommonStateType EioCommon;         /*!< Common eio drivers structure */
    const uint8 *TxData;                   /*!< Transmit buffer. */
    uint8 *RxData;                         /*!< Receive buffer. */
    uint32 TxRemainingBytes;               /*!< Number of remaining bytes to be transmitted. */
    uint32 RxRemainingBytes;               /*!< Number of remaining bytes to be received. */
    uint32 DummyDmaData;                   /*!< Dummy location for DMA transfers. */
    uint8 RxDmaChannel;                    /*!< Rx DMA channel number */
    uint8 TxDmaChannel;                    /*!< Tx DMA channel number */
    uint8 BitsWidth;                       /*!< Number of bits in a word */
    uint8 ByteWidth;                       /*!< Number of bytes in a word */
    boolean Master;                        /*!< Current instance is in master mode */
    Eio_DriverType DriverType;             /*!< Driver type: interrupts/polling/DMA */
    Hal_StatusType Status;                 /*!< Current status of the driver */
    boolean DriverIdle;                    /*!< Idle/busy state of the driver */
    Eio_I2sCallbackType Callback;          /*!< User callback function */
    boolean ClockContinueOutput;           /*!< clock Continue Output,use for AUDIO */
} Eio_I2sStateType;

/* ==========================================  Variables  =========================================== */
/*!
 * @brief First initialization flag for I2S in clock continue mode
 * @note DES ID: DES_I2S_VAR_001
 */
static boolean s_i2sInit = FALSE;

#if (CONFIG_EI2S0_ENABLE)
/*!
 * @brief EIO I2S channel 0 driver state structure
 * @note DES ID: DES_I2S_VAR_002
 */
static Eio_I2sStateType EI2S0_ChannelState;
#endif

#if (CONFIG_EI2S1_ENABLE)
/*!
 * @brief EIO I2S channel 1 driver state structure
 * @note DES ID: DES_I2S_VAR_003
 */
static Eio_I2sStateType EI2S1_ChannelState;
#endif

/*!
 * @brief Array of pointers to EIO I2S driver state structures for each channel
 * @note DES ID: DES_I2S_VAR_004
 */
static Eio_I2sStateType *const EioI2sState[I2S_MAX_CHANNEL] =
{
#if (CONFIG_EI2S0_ENABLE)
    &EI2S0_ChannelState,
#else
    NULL_PTR,
#endif
#if (CONFIG_EI2S1_ENABLE)
    &EI2S1_ChannelState,
#else
    NULL_PTR,
#endif
};

/*PRQA S 3218 ++ # AMCE_EIO_3218 */
/**
 * @brief I2S master default configuration
 * @note DES ID: DES_I2S_VAR_005
 */
static Eio_I2sUserConfigType DefaultMasterConfig =
{
    .EioInstance = 0U,
    .Mode = EIO_I2S_MASTER,
    .DriverType = EIO_DRIVER_TYPE_INTERRUPTS,
    .SampleRate = 8000U,   /* 8 kHz sampling rate */
    .BitsWidth = 16U,
    .TxPin = 0U,
    .RxPin = 1U,
    .SckPin = 2U,
    .WsPin = 3U,
    .Callback = NULL_PTR,
    .RxDmaChannel = 255U,
    .TxDmaChannel = 255U,
    .ClockContinueOutput = FALSE,
};

/**
 * @brief I2S slave default configuration
 * @note DES ID: DES_I2S_VAR_006
 */
static Eio_I2sUserConfigType DefaultSlaveConfig =
{
    .EioInstance = 0U,
    .Mode = EIO_I2S_SLAVE,
    .DriverType = EIO_DRIVER_TYPE_INTERRUPTS,
    .BitsWidth = 16U,
    .TxPin = 0U,
    .RxPin = 1U,
    .SckPin = 2U,
    .WsPin = 3U,
    .Callback = NULL_PTR,
    .RxDmaChannel = 255U,
    .TxDmaChannel = 255U,
    .ClockContinueOutput = FALSE,
};
/*PRQA S 3218 -- */

/**
 * @brief Table to map EIO_i2s transfer sizes to EDMA transfer sizes
 * @note DES ID: DES_I2S_VAR_007
 */
static const Dma_TransferUnitType DmaTransferSizeArray[4U] =
{
    DMA_TRANSFER_UNIT_1B,
    DMA_TRANSFER_UNIT_2B,
    DMA_TRANSFER_UNIT_4B,
    DMA_TRANSFER_UNIT_4B
};

/* ====================================  Functions declaration  ===================================== */

/* =====================================  Functions definition  ===================================== */
/*!
 * @brief Compute byte width from bits width
 *
 * @details
 * Converts the configured bits-per-word value into the number of bytes
 * that represent a single audio sample for register and DMA programming.
 *
 * @param[in] BitsWidth: Number of bits per audio word
 *
 * @return
 *     Number of bytes per audio word (1, 2 or 4)
 *
 * @pre
 *     The BitsWidth must be a positive integer multiple of 8.
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant.
 *     DES ID: DES_I2S_API_013
 */
static uint8 Eio_I2s_Hal_ComputeByteWidth(uint8 BitsWidth)
{
    uint8 ByteWidth;

    if (BitsWidth <= 8U)
    {
        ByteWidth = 1U;
    }
    else if (BitsWidth <= 16U)
    {
        ByteWidth = 2U;
    }
    else
    {
        ByteWidth = 4U;
    }

    return ByteWidth;
}

/*!
 * @brief Compute master baud-rate divider
 *
 * @details
 * Calculates the timer/divider value required to produce the requested
 * I2S SCK frequency from the provided input clock. The result is
 * clamped to hardware supported limits.
 *
 * @param[in] BaudRate: Desired baud rate in hertz
 * @param[out] Divider: Pointer where the computed divider is stored
 * @param[in] InputClock: Frequency of the input clock in hertz
 *
 * @return
 *     void
 *
 * @pre
 *     InputClock must be greater than zero.
 *
 * @post
 *     *Divider contains a value within hardware limits.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     DES ID: DES_I2S_API_014
 */
static void Eio_I2s_Hal_MasterComputeBaudRateDivider(uint32 BaudRate, uint16 *Divider, uint32 InputClock)
{
    sint32 TmpDiv;

    /* Compute Divider: ((input_clock / baud_rate) / 2) - 1. Round to nearest integer */
    TmpDiv = (((sint32)InputClock + (sint32)BaudRate) / (2 * (sint32)BaudRate)) - 1;
    /* Enforce upper/lower limits */
    if (TmpDiv < DIVIDER_MIN_VALUE)
    {
        TmpDiv = DIVIDER_MIN_VALUE;
    }

    if (TmpDiv > DIVIDER_MAX_VALUE)
    {
        TmpDiv = DIVIDER_MAX_VALUE;
    }

    *Divider = (uint16)TmpDiv;
}

/*!
 * @brief Configure EIO as I2S master
 *
 * @details
 * Program the EIO shifters and timers to operate in I2S master mode
 * according to the provided user configuration and input clock.
 *
 * @param[in] Channel: Logical I2S channel index
 * @param[in] UserConfigPtr: Pointer to user configuration structure
 * @param[in] InputClock: Frequency of the input clock in hertz
 *
 * @return
 *     void
 *
 * @pre
 *     UserConfigPtr must be a valid pointer and InputClock > 0.
 *
 * @post
 *     EIO registers for the channel are configured for master operation.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     DES ID: DES_I2S_API_015
 */
static void Eio_I2s_Hal_MasterConfigure(uint8 Channel, const Eio_I2sUserConfigType *UserConfigPtr, uint32 InputClock)
{
    EIO_Type *BaseAddr;
    uint16 Divider;
    uint16 Bits;
    uint8 ResourceIndex;    /* Index of first used internal resource Instance (shifter and timer) */

    const Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    BaseAddr = EioBase[ChannelState->EioCommon.Instance];
    ResourceIndex = ChannelState->EioCommon.ResourceIndex;
    Bits = UserConfigPtr->BitsWidth;

    /* Calculate baud rate from sample rate: BaudRate = SampleRate x BitsWidth x 2 (stereo) */
    uint32 BaudRate = UserConfigPtr->SampleRate * (uint32)Bits * 2U;

    /* Compute Divider. */
    Eio_I2s_Hal_MasterComputeBaudRateDivider(BaudRate, &Divider, InputClock);

    /* Configure tx shifter */
    Eio_Reg_SetShifterConfig(BaseAddr,
                         I2S_TX_SHIFTER(ResourceIndex),
                         EIO_SHIFTER_START_BIT_DISABLED_SH,
                         EIO_SHIFTER_STOP_BIT_DISABLED,
                         EIO_SHIFTER_SOURCE_PIN);
    Eio_Reg_SetShifterControl(BaseAddr,
                          I2S_TX_SHIFTER(ResourceIndex),
                          EIO_SHIFTER_MODE_DISABLED,
                          UserConfigPtr->TxPin,             /* output on tx pin */
                          EIO_PIN_POLARITY_HIGH,
                          EIO_PIN_CONFIG_DISABLED,       /* disable for now, will enable on transmit */
                          I2S_SCK_TIMER(ResourceIndex),         /* use clock timer to drive the shifter */
                          EIO_TIMER_POLARITY_POSEDGE);

    /* Configure rx shifter */
    Eio_Reg_SetShifterConfig(BaseAddr,
                         I2S_RX_SHIFTER(ResourceIndex),
                         EIO_SHIFTER_START_BIT_DISABLED,
                         EIO_SHIFTER_STOP_BIT_DISABLED,
                         EIO_SHIFTER_SOURCE_PIN);
    Eio_Reg_SetShifterControl(BaseAddr,
                          I2S_RX_SHIFTER(ResourceIndex),
                          EIO_SHIFTER_MODE_DISABLED,
                          UserConfigPtr->RxPin,                    /* output to rx pin */
                          EIO_PIN_POLARITY_HIGH,
                          EIO_PIN_CONFIG_DISABLED,
                          I2S_SCK_TIMER(ResourceIndex),     /* use control timer to drive the shifter */
                          EIO_TIMER_POLARITY_NEGEDGE);

    /* Configure SCK timer */
    Eio_Reg_SetTimerCompare(BaseAddr, I2S_SCK_TIMER(ResourceIndex),
                        (uint16)((uint16)(((uint16)(Bits << 1U) - 1U) << 8U) + Divider));
    Eio_Reg_SetTimerConfig(BaseAddr,
                       I2S_SCK_TIMER(ResourceIndex),
                       EIO_TIMER_START_BIT_ENABLED,
                       EIO_TIMER_STOP_BIT_DISABLED,
                       EIO_TIMER_ENABLE_TRG_HIGH,         /* enable when Tx data is available */
                       EIO_TIMER_DISABLE_NEVER,
                       EIO_TIMER_RESET_NEVER,
                       EIO_TIMER_DECREMENT_CLK_SHIFT_TMR, /* decrement on EIO clock */
                       EIO_TIMER_INITOUT_ONE);
    Eio_Reg_SetTimerControl(BaseAddr,
                        I2S_SCK_TIMER(ResourceIndex),
                        (uint8)((uint8)(I2S_TX_SHIFTER(ResourceIndex) << 2U) + 1U),  /* trigger on tx shifter Status flag */
                        EIO_TRIGGER_POLARITY_LOW,
                        EIO_TRIGGER_SOURCE_INTERNAL,
                        UserConfigPtr->SckPin,                 /* output on SCK pin */
                        EIO_PIN_POLARITY_LOW,
                        EIO_PIN_CONFIG_OUTPUT,              /* enable output */
                        EIO_TIMER_MODE_DISABLED);

    /* Configure WS timer */
    Eio_Reg_SetTimerCompare(BaseAddr, I2S_WS_TIMER(ResourceIndex), (uint16)((Bits * ((Divider + 1U) * 2U)) - 1U));
    Eio_Reg_SetTimerConfig(BaseAddr,
                       I2S_WS_TIMER(ResourceIndex),
                       EIO_TIMER_START_BIT_DISABLED,
                       EIO_TIMER_STOP_BIT_DISABLED,
                       EIO_TIMER_ENABLE_TIM_ENABLE,       /* enable when SCK timer is enabled */
                       EIO_TIMER_DISABLE_NEVER,
                       EIO_TIMER_RESET_NEVER,
                       EIO_TIMER_DECREMENT_CLK_SHIFT_TMR, /* decrement on EIO clock */
                       EIO_TIMER_INITOUT_ONE);
    Eio_Reg_SetTimerControl(BaseAddr,
                        I2S_WS_TIMER(ResourceIndex),
                        0U,                                  /* trigger not used */
                        EIO_TRIGGER_POLARITY_HIGH,
                        EIO_TRIGGER_SOURCE_EXTERNAL,
                        UserConfigPtr->WsPin,                /* output on WS pin */
                        EIO_PIN_POLARITY_LOW,
                        EIO_PIN_CONFIG_OUTPUT,            /* enable output */
                        EIO_TIMER_MODE_DISABLED);
}

/*!
 * @brief Configure EIO as I2S slave
 *
 * @details
 * Program the EIO shifters and timers to operate in I2S slave mode
 * based on the user configuration. In slave mode the peripheral uses
 * external SCK/WS inputs provided on the configured pins.
 *
 * @param[in] Channel: Logical I2S channel index
 * @param[in] UserConfigPtr: Pointer to user configuration structure
 *
 * @return
 *     void
 *
 * @pre
 *     UserConfigPtr must be a valid pointer.
 *
 * @post
 *     EIO registers for the channel are configured for slave operation.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     DES ID: DES_I2S_API_016
 */
static void Eio_I2s_Hal_SlaveConfigure(uint8 Channel, const Eio_I2sUserConfigType *UserConfigPtr)
{
    EIO_Type *BaseAddr;
    uint16 Bits;
    uint8 ResourceIndex;    /* Index of first used internal resource Instance (shifter and timer) */

    const Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    BaseAddr = EioBase[ChannelState->EioCommon.Instance];
    ResourceIndex = ChannelState->EioCommon.ResourceIndex;
    Bits = UserConfigPtr->BitsWidth;

    /* Configure tx shifter */
    Eio_Reg_SetShifterConfig(BaseAddr,
                         I2S_TX_SHIFTER(ResourceIndex),
                         EIO_SHIFTER_START_BIT_DISABLED,
                         EIO_SHIFTER_STOP_BIT_DISABLED,
                         EIO_SHIFTER_SOURCE_PIN);
    Eio_Reg_SetShifterControl(BaseAddr,
                          I2S_TX_SHIFTER(ResourceIndex),
                          EIO_SHIFTER_MODE_DISABLED,
                          UserConfigPtr->TxPin,             /* output on tx pin */
                          EIO_PIN_POLARITY_HIGH,
                          EIO_PIN_CONFIG_DISABLED,           /* disable for now, will enable on transmit *///EIO_PIN_CONFIG_DISABLED
                          I2S_WS_TIMER(ResourceIndex),     /* use clock timer to drive the shifter */
                          EIO_TIMER_POLARITY_POSEDGE);

    /* Configure rx shifter */
    Eio_Reg_SetShifterConfig(BaseAddr,
                         I2S_RX_SHIFTER(ResourceIndex),
                         EIO_SHIFTER_START_BIT_DISABLED,
                         EIO_SHIFTER_STOP_BIT_DISABLED,
                         EIO_SHIFTER_SOURCE_PIN);
    Eio_Reg_SetShifterControl(BaseAddr,
                          I2S_RX_SHIFTER(ResourceIndex),
                          EIO_SHIFTER_MODE_DISABLED,
                          UserConfigPtr->RxPin,                    /* output to rx pin */
                          EIO_PIN_POLARITY_HIGH,
                          EIO_PIN_CONFIG_DISABLED,
                          I2S_WS_TIMER(ResourceIndex),     /* use control timer to drive the shifter */
                          EIO_TIMER_POLARITY_NEGEDGE);

    /* Configure SCK timer */
    Eio_Reg_SetTimerCompare(BaseAddr, I2S_SCK_TIMER(ResourceIndex), (uint16)((uint16)(Bits << 2U) - 3U));
    Eio_Reg_SetTimerConfig(BaseAddr,
                       I2S_SCK_TIMER(ResourceIndex),
                       EIO_TIMER_START_BIT_DISABLED,
                       EIO_TIMER_STOP_BIT_DISABLED,
                       EIO_TIMER_ENABLE_PIN_POSEDGE,      /* enable on WS positive edge */
                       EIO_TIMER_DISABLE_TIM_CMP,
                       EIO_TIMER_RESET_NEVER,
                       EIO_TIMER_DECREMENT_TRIG_SHIFT_TRIG, /* decrement on SCK input  */
                       EIO_TIMER_INITOUT_ONE);
    Eio_Reg_SetTimerControl(BaseAddr,
                        I2S_SCK_TIMER(ResourceIndex),
                        (uint8)(UserConfigPtr->SckPin << 1U), /* trigger on SCK pin */
                        EIO_TRIGGER_POLARITY_LOW,
                        EIO_TRIGGER_SOURCE_INTERNAL,
                        UserConfigPtr->WsPin,                 /* use WS input pin */
                        EIO_PIN_POLARITY_LOW,
                        EIO_PIN_CONFIG_DISABLED,
                        EIO_TIMER_MODE_DISABLED);
    /* configure WS_TIMER_RISING_BLK timer */
    Eio_Reg_SetTimerCompare(BaseAddr, I2S_WS_TIMER_RISING_BLK(ResourceIndex), (uint16)((uint16)(Bits << 2U) - 4U));
    Eio_Reg_SetTimerConfig(BaseAddr,
                       I2S_WS_TIMER_RISING_BLK(ResourceIndex),
                       EIO_TIMER_START_BIT_DISABLED,
                       EIO_TIMER_STOP_BIT_DISABLED,
                       EIO_TIMER_ENABLE_PIN_POSEDGE_TRG_HIGH,
                       EIO_TIMER_DISABLE_TRG,
                       EIO_TIMER_RESET_NEVER,
                       EIO_TIMER_DECREMENT_PIN_SHIFT_PIN,
                       EIO_TIMER_INITOUT_ONE);
    Eio_Reg_SetTimerControl(BaseAddr,
                        I2S_WS_TIMER_RISING_BLK(ResourceIndex),
                        (uint8)((uint8)(I2S_SCK_TIMER(ResourceIndex) << 2U) + 3U), /* SCK timer trigger output */
                        EIO_TRIGGER_POLARITY_HIGH,
                        EIO_TRIGGER_SOURCE_INTERNAL,
                        UserConfigPtr->SckPin,                 /* use sck input pin */
                        EIO_PIN_POLARITY_HIGH,
                        EIO_PIN_CONFIG_DISABLED,
                        EIO_TIMER_MODE_DISABLED);

    /* Configure WS timer */
    Eio_Reg_SetTimerCompare(BaseAddr, I2S_WS_TIMER(ResourceIndex), (uint16)((uint16)(Bits << 1U) - 1U));
    Eio_Reg_SetTimerConfig(BaseAddr,
                       I2S_WS_TIMER(ResourceIndex),
                       EIO_TIMER_START_BIT_DISABLED,
                       EIO_TIMER_STOP_BIT_DISABLED,
                       EIO_TIMER_ENABLE_PIN_POSEDGE_TRG_HIGH, /* enable at first falling edge after first rising edge of bclk */
                       EIO_TIMER_DISABLE_TIM_CMP_TRG_LOW,
                       EIO_TIMER_RESET_NEVER,
                       EIO_TIMER_DECREMENT_PIN_SHIFT_PIN,
                       EIO_TIMER_INITOUT_ONE);
    Eio_Reg_SetTimerControl(BaseAddr,
                        I2S_WS_TIMER(ResourceIndex),
                        (uint8)((uint8)(I2S_WS_TIMER_RISING_BLK(ResourceIndex) << 2U) + 3U), /* I2S_WS_TIMER_RISING_BLK timer trigger output */
                        EIO_TRIGGER_POLARITY_HIGH,
                        EIO_TRIGGER_SOURCE_INTERNAL,
                        UserConfigPtr->SckPin,               /* SCK input pin */
                        EIO_PIN_POLARITY_LOW,
                        EIO_PIN_CONFIG_DISABLED,          /* enable output */
                        EIO_TIMER_MODE_DISABLED);
}

/*!
 * @brief End the current I2S transfer
 *
 * @details
 * Stop active shifters/timers and disable DMA/interrupts associated with
 * the ongoing transfer for the specified channel. Updates internal state
 * to reflect transfer completion.
 *
 * @param[in] Channel: Logical I2S channel index
 *
 * @return
 *     void
 *
 * @pre
 *     A transfer may be active on the specified channel.
 *
 * @post
 *     Transfer engine is stopped and driver internal state updated.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     DES ID: DES_I2S_API_017
 */
static void Eio_I2s_Hal_EndTransfer(uint8 Channel)
{
    EIO_Type *BaseAddr;
    uint8 ResourceIndex;    /* Index of first used internal resource Instance (shifter and timer) */

    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    BaseAddr = EioBase[ChannelState->EioCommon.Instance];
    ResourceIndex = ChannelState->EioCommon.ResourceIndex;

    /* Disable transfer engine */
    switch (ChannelState->DriverType)
    {
        case EIO_DRIVER_TYPE_INTERRUPTS:
            /* Disable interrupts for Rx and Tx shifters */
            Eio_Reg_SetShifterInterrupt(BaseAddr,
                                (uint8)((1U << I2S_TX_SHIFTER(ResourceIndex)) | (1U << I2S_RX_SHIFTER(ResourceIndex))),
                                FALSE);
            Eio_Reg_SetShifterErrorInterrupt(BaseAddr,
                                (uint8)((1U << I2S_TX_SHIFTER(ResourceIndex)) | (1U << I2S_RX_SHIFTER(ResourceIndex))),
                                FALSE);
            break;

        case EIO_DRIVER_TYPE_POLLING:
            /* Nothing to do here */
            break;

        case EIO_DRIVER_TYPE_DMA:
            /* Stop DMA channels */
            (void)Dma_Hal_StopCh(ChannelState->TxDmaChannel);
            (void)Dma_Hal_StopCh(ChannelState->RxDmaChannel);
            /* Disable EIO Tx and Rx DMA requests */
            Eio_Reg_SetShifterDMARequest(BaseAddr,
                    (uint8)((1U << I2S_TX_SHIFTER(ResourceIndex)) | (1U << I2S_RX_SHIFTER(ResourceIndex))), FALSE);
            break;

        default:
            /* Impossible type - do nothing */
            break;
    }

    ChannelState->DriverIdle = TRUE;
    ChannelState->TxData = NULL_PTR;
    ChannelState->RxData = NULL_PTR;
    ChannelState->TxRemainingBytes = 0U;
    ChannelState->RxRemainingBytes = 0U;
}

/*!
 * @brief Enable transfer engine for channel
 *
 * @details
 * Enable EIO timers/shifters and associated DMA or interrupts to start
 * a data transfer on the specified channel.
 *
 * @param[in] Channel: Logical I2S channel index
 *
 * @return
 *     void
 *
 * @pre
 *     Driver context for the channel must be initialized and idle.
 *
 * @post
 *     Transfer engine is enabled and ready to process data.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     DES ID: DES_I2S_API_018
 */
static void Eio_I2s_Hal_EnableTransfer(uint8 Channel)
{
    const Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    uint8 ResourceIndex = ChannelState->EioCommon.ResourceIndex;    /* Index of first used internal resource Instance (shifter and timer) */
    EIO_Type *BaseAddr = EioBase[ChannelState->EioCommon.Instance];

    /* enable timers and shifters */
    Eio_Reg_SetShifterMode(BaseAddr, I2S_TX_SHIFTER(ResourceIndex), EIO_SHIFTER_MODE_TRANSMIT);
    Eio_Reg_SetShifterMode(BaseAddr, I2S_RX_SHIFTER(ResourceIndex), EIO_SHIFTER_MODE_RECEIVE);
    Eio_Reg_SetTimerMode(BaseAddr, I2S_WS_TIMER(ResourceIndex), EIO_TIMER_MODE_16BIT);
    if (ChannelState->Master == TRUE)
    {
        Eio_Reg_SetTimerMode(BaseAddr, I2S_SCK_TIMER(ResourceIndex), EIO_TIMER_MODE_8BIT_BAUD);
    }
    else
    {
        Eio_Reg_SetTimerMode(BaseAddr, I2S_SCK_TIMER(ResourceIndex), EIO_TIMER_MODE_16BIT);
        Eio_Reg_SetTimerMode(BaseAddr, I2S_WS_TIMER_RISING_BLK(ResourceIndex), EIO_TIMER_MODE_16BIT);
    }
}

/*!
 * @brief Stop transfer engine for channel
 *
 * @details
 * Disable EIO timers/shifters and DMA requests for the specified channel
 * to halt any ongoing transfer operations.
 *
 * @param[in] Channel: Logical I2S channel index
 *
 * @return
 *     void
 *
 * @pre
 *     Transfer engine may be running.
 *
 * @post
 *     Transfer engine is stopped and DMA/interrupts are disabled.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     DES ID: DES_I2S_API_019
 */
static void Eio_I2s_Hal_StopTransfer(uint8 Channel)
{
    EIO_Type *BaseAddr;
    uint8 ResourceIndex;    /* Index of first used internal resource Instance (shifter and timer) */

    const Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    ResourceIndex = ChannelState->EioCommon.ResourceIndex;
    BaseAddr = EioBase[ChannelState->EioCommon.Instance];

    if (ChannelState->ClockContinueOutput == FALSE)
    {
        /* disable timers and shifters */
        Eio_Reg_SetTimerMode(BaseAddr, I2S_SCK_TIMER(ResourceIndex), EIO_TIMER_MODE_DISABLED);
        Eio_Reg_SetTimerMode(BaseAddr, I2S_WS_TIMER(ResourceIndex), EIO_TIMER_MODE_DISABLED);
    }

    Eio_Reg_SetShifterMode(BaseAddr, I2S_TX_SHIFTER(ResourceIndex), EIO_SHIFTER_MODE_DISABLED);
    Eio_Reg_SetShifterMode(BaseAddr, I2S_RX_SHIFTER(ResourceIndex), EIO_SHIFTER_MODE_DISABLED);
    if (ChannelState->Master == FALSE)
    {
        Eio_Reg_SetTimerMode(BaseAddr, I2S_WS_TIMER_RISING_BLK(ResourceIndex), EIO_TIMER_MODE_DISABLED);
    }

    if (ChannelState->ClockContinueOutput == FALSE)
    {
        /* Disable pin output */
        Eio_Reg_SetShifterPinConfig(BaseAddr, I2S_TX_SHIFTER(ResourceIndex), EIO_PIN_CONFIG_DISABLED);
    }

    /* clear any leftover error flags */
    Eio_Reg_ClearShifterErrorStatus(BaseAddr, I2S_TX_SHIFTER(ResourceIndex));
    Eio_Reg_ClearShifterErrorStatus(BaseAddr, I2S_RX_SHIFTER(ResourceIndex));
    /* discard any leftover rx. data */
    Eio_Reg_ClearShifterStatus(BaseAddr, I2S_RX_SHIFTER(ResourceIndex));

    /* end the transfer */
    Eio_I2s_Hal_EndTransfer(Channel);
}

/*!
 * @brief Read available data from the peripheral
 *
 * @details
 * Consume words available in the EIO shifter FIFO and copy them into
 * the driver's receive buffer, updating remaining byte counters and
 * invoking callbacks as needed.
 *
 * @param[in] Channel: Logical I2S channel index
 *
 * @return
 *     void
 *
 * @pre
 *     Channel driver must be configured and a receive buffer provided.
 *
 * @post
 *     Rx buffer and internal counters are updated.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     DES ID: DES_I2S_API_020
 */
static void Eio_I2s_Hal_ReadData(uint8 Channel)
{
    uint32 Data;
    const EIO_Type *BaseAddr = EioBase[EioI2sState[Channel]->EioCommon.Instance];
    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    uint8 ResourceIndex = ChannelState->EioCommon.ResourceIndex;

    /* Read data from shifter buffer */
    Data = Eio_Reg_ReadShifterBuffer(BaseAddr, I2S_RX_SHIFTER(ResourceIndex), EIO_SHIFTER_RW_MODE_BIT_SWAP);

    if (ChannelState->RxRemainingBytes > 0U)
    {
        if (ChannelState->RxData != NULL_PTR)
        {
            switch (ChannelState->ByteWidth)
            {
                case 1U:
                    *(uint8 *)ChannelState->RxData = (uint8)Data;
                    break;

                case 2U:
                    /* PRQA S 0310,3305 ++ # AMCE_EIO_0310, AMCE_EIO_3305*/
                    *(uint16 *)ChannelState->RxData  = (uint16)Data;
                    /* PRQA S 0310,3305 -- */
                    break;

                default:
                    /* PRQA S 0310,3305 ++ # AMCE_EIO_0310, AMCE_EIO_3305*/
                    *(uint32 *)ChannelState->RxData  = (uint32)Data;
                    /* PRQA S 0310,3305 -- */
                    break;
            }
            /* Update rx buffer pointer */
            ChannelState->RxData = &ChannelState->RxData[ChannelState->ByteWidth];
        }
        /* Update remaining bytes count even if buffer is NULL_PTR */
        ChannelState->RxRemainingBytes -= (uint32)(ChannelState->ByteWidth);
    }
    else
    {
        /* No data to receive, just ignore the read data */
    }
}

/*!
 * @brief Write data from transmit buffer to peripheral
 *
 * @details
 * Load pending transmit words from the driver's Tx buffer into the EIO
 * shifter registers and update internal counters accordingly.
 *
 * @param[in] Channel: Logical I2S channel index
 *
 * @return
 *     void
 *
 * @pre
 *     Channel driver must be configured and a transmit buffer provided.
 *
 * @post
 *     Tx buffer pointers and counters are advanced.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     DES ID: DES_I2S_API_021
 */
static void Eio_I2s_Hal_WriteData(uint8 Channel)
{
    uint32 Data;
    EIO_Type *BaseAddr = EioBase[EioI2sState[Channel]->EioCommon.Instance];
    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    uint8 ResourceIndex = ChannelState->EioCommon.ResourceIndex;

    if (ChannelState->TxRemainingBytes > 0U)
    {
        /* Read data from user buffer and update tx buffer pointer and remaining bytes count */
        switch (ChannelState->ByteWidth)
        {
            case 1U:
                Data = (uint32)(*(const uint8 *)ChannelState->TxData);
                break;

            case 2U:
                /* PRQA S 0310,3305 ++ # AMCE_EIO_0310, AMCE_EIO_3305*/
                Data = (uint32)(*(const uint16 *)ChannelState->TxData);
                /* PRQA S 0310,3305 -- */
                break;

            default:
                /* PRQA S 0310,3305 ++ # AMCE_EIO_0310, AMCE_EIO_3305*/
                Data = (uint32)(*(const uint32 *)ChannelState->TxData);
                /* PRQA S 0310,3305 -- */
                break;
        }
        ChannelState->TxData = &ChannelState->TxData[ChannelState->ByteWidth];
        ChannelState->TxRemainingBytes -= (uint32)(ChannelState->ByteWidth);

        /* Write data to shifter buffer */
        /* Shift data before bit-swapping it to get the relevant bits in the lower part of the shifter */
        Data <<= 32U - (uint32)(ChannelState->BitsWidth);
        Eio_Reg_WriteShifterBuffer(BaseAddr, I2S_TX_SHIFTER(ResourceIndex), Data, EIO_SHIFTER_RW_MODE_BIT_SWAP);
    }
}

/*!
 * @brief Check and handle transfer status (interrupt or polling callback)
 *
 * @details
 * Periodic callback used in polling mode to inspect shifter/timer
 * status, advance the transfer state machine and detect errors.
 *
 * @param[in] StateStruct: Pointer to driver state structure (cast to void)
 *
 * @return
 *     void
 *
 * @pre
 *     StateStruct must point to a valid Eio_I2sStateType instance.
 *
 * @post
 *     Transfer state may be advanced and interrupts/DMA triggered.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function can be called from an ISR context.
 *     DES ID: DES_I2S_API_022
 */
/*PRQA S 3673 ++ # AMCE_EIO_3673 */
static void Eio_I2s_Hal_CheckStatus(void *StateStruct)
/*PRQA S 3673 -- */
{
    DEVICE_ASSERT(StateStruct != NULL_PTR);

    EIO_Type *BaseAddr;
    uint8 ResourceIndex;    /* Index of first used internal resource Instance (shifter and timer) */
    const Eio_CommonStateType *State;
    uint32 Channel;
    Eio_I2sStateType *ChannelState;

    /*PRQA S 0316 ++ # AMCE_QAC_0316 */
    State = (Eio_CommonStateType *)StateStruct;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # AMCE_EIO_0326.*/
    Channel = (uint32)(State->UserArgs);
    /*PRQA S 0326 -- */
    ChannelState = EioI2sState[Channel];
    BaseAddr = EioBase[State->Instance];
    ResourceIndex = State->ResourceIndex;

    /* Check for errors */
    if (ChannelState->TxData != NULL_PTR)
    {
        if (Eio_Reg_GetShifterErrorStatus(BaseAddr, I2S_TX_SHIFTER(ResourceIndex)) == TRUE)
        {
            ChannelState->Status = STATUS_I2S_TX_UNDERRUN;
            Eio_Reg_ClearShifterErrorStatus(BaseAddr, I2S_TX_SHIFTER(ResourceIndex));
            /* Continue processing events */
        }
    }

    if (ChannelState->RxData != NULL_PTR)
    {
        if (Eio_Reg_GetShifterErrorStatus(BaseAddr, I2S_RX_SHIFTER(ResourceIndex)) == TRUE)
        {
            ChannelState->Status = STATUS_I2S_RX_OVERRUN;
            Eio_Reg_ClearShifterErrorStatus(BaseAddr, I2S_RX_SHIFTER(ResourceIndex));
            /* Continue processing events */
        }
    }

    /* Check if data was received */
    if (Eio_Reg_GetShifterStatus(BaseAddr, I2S_RX_SHIFTER(ResourceIndex)) == TRUE)
    {
        Eio_I2s_Hal_ReadData((uint8)Channel);
        if ((ChannelState->RxData != NULL_PTR) && (ChannelState->RxRemainingBytes == 0U))
        {
            /* Out of rx space, call callback to allow user to provide a new buffer */
            if (ChannelState->Callback != NULL_PTR)
            {
                ChannelState->Callback((uint8)Channel, EIO_I2S_EVENT_RX_FULL);
            }
        }
    }

    /* Check if transmitter needs more data */
    if (ChannelState->TxData != NULL_PTR)
    {
        if (Eio_Reg_GetShifterStatus(BaseAddr, I2S_TX_SHIFTER(ResourceIndex)) == TRUE)
        {
            Eio_I2s_Hal_WriteData((uint8)Channel);
            if (ChannelState->TxRemainingBytes == 0U)
            {
                ChannelState->TxData = NULL_PTR;
                /* Out of data, call callback to allow user to provide a new buffer */
                if (ChannelState->Callback != NULL_PTR)
                {
                    ChannelState->Callback((uint8)Channel, EIO_I2S_EVENT_TX_EMPTY);
                }

                if ((ChannelState->TxRemainingBytes == 0U) && (ChannelState->DriverType == EIO_DRIVER_TYPE_INTERRUPTS))
                {
                    /* Still no more data to transmit, transmission will stop */
                    /* disable tx interrupts */
                    Eio_Reg_SetShifterInterrupt(BaseAddr, (uint8)(1U << I2S_TX_SHIFTER(ResourceIndex)), FALSE);
                    Eio_Reg_SetShifterErrorInterrupt(BaseAddr, (uint8)(1U << I2S_TX_SHIFTER(ResourceIndex)), FALSE);
                    /* Enable rx interrupt to signal end of transfer */
                    Eio_Reg_SetShifterInterrupt(BaseAddr, (uint8)(1U << I2S_RX_SHIFTER(ResourceIndex)), TRUE);
                }
            }
        }
    }

    /* Check if transfer is over */
    if (ChannelState->RxRemainingBytes == 0U)
    {
        /* Record success if there was no error */
        if (ChannelState->Status == STATUS_BUSY)
        {
            ChannelState->Status = STATUS_SUCCESS;
        }

        /* End transfer */
        Eio_I2s_Hal_StopTransfer((uint8)Channel);
        /* Call callback to announce the event to the user */
        if (ChannelState->Callback != NULL_PTR)
        {
            ChannelState->Callback((uint8)Channel, EIO_I2S_EVENT_END_TRANSFER);
        }
    }
}

/*!
 * @brief Compute TX register address used for DMA transfers
 *
 * @details
 * Return the memory-mapped register address corresponding to the TX
 * shifter for the given logical channel. Used to program the DMA
 * peripheral transfer descriptor.
 *
 * @param[in] Channel: Logical I2S channel index
 *
 * @return
 *     32-bit register address for TX shifter
 *
 * @pre
 *     Channel must map to a valid EIO instance.
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant.
 *     DES ID: DES_I2S_API_023
 */
static uint32 Eio_I2s_Hal_ComputeTxRegAddr(uint8 Channel)
{
    uint32 Addr;
    const EIO_Type *BaseAddr;
    uint8 Shifter;

    const Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    BaseAddr = EioBase[ChannelState->EioCommon.Instance];
    Shifter = I2S_TX_SHIFTER(ChannelState->EioCommon.ResourceIndex);
    Addr = (uint32)(&(BaseAddr->SHIFTBUFBIS[Shifter])) + (sizeof(uint32) - ChannelState->ByteWidth);

    return Addr;
}

/*!
 * @brief Compute RX register address used for DMA transfers
 *
 * @details
 * Return the memory-mapped register address corresponding to the RX
 * shifter for the given logical channel. Used to program the DMA
 * peripheral transfer descriptor.
 *
 * @param[in] Channel: Logical I2S channel index
 *
 * @return
 *     32-bit register address for RX shifter
 *
 * @pre
 *     Channel must map to a valid EIO instance.
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant.
 *     DES ID: DES_I2S_API_024
 */
static uint32 Eio_I2s_Hal_ComputeRxRegAddr(const uint8 Channel)
{
    uint32 Addr;
    const EIO_Type *BaseAddr;
    uint8 Shifter;

    const Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    BaseAddr = EioBase[ChannelState->EioCommon.Instance];
    Shifter = I2S_RX_SHIFTER(ChannelState->EioCommon.ResourceIndex);
    Addr = (uint32)(&(BaseAddr->SHIFTBUFBIS[Shifter]));

    return Addr;
}

/*!
 * @brief DMA transmit completion callback
 *
 * @details
 * Called by the DMA HAL when a transmit DMA transfer completes. Updates
 * driver state and triggers upper-layer callback if required.
 *
 * @param[in] Param: User parameter passed by DMA driver (driver state)
 *
 * @return
 *     void
 *
 * @pre
 *     Param must be the pointer passed when configuring the DMA channel.
 *
 * @post
 *     Driver Tx state is updated and completion event may be propagated.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from DMA ISR context.
 *     DES ID: DES_I2S_API_025
 */
/*PRQA S 3673 ++ # AMCE_EIO_3673 */
static void Eio_I2s_Hal_EndDmaTxTransfer(void *Param)
/*PRQA S 3673 -- */
{
    /*PRQA S 0316 ++ # interrupt functions allow conversion of a pointer to void to a pointer to an object.*/
    const Dma_ChannelCBInfoType *DmaInfo = (const Dma_ChannelCBInfoType *)Param;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    uint32 Channel = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */
    Dma_ChannelAddrType DmaAddr;
    Dma_TransferUnitType DmaTransferSize;

    Eio_I2sStateType *ChannelState = EioI2sState[Channel];

    /* Call callback to allow user to provide a new buffer */
    if (0U != (DmaInfo->DmaEvent & DMA_ERROR_EVENT))
    {
        /* DMA error, stop transfer */
        ChannelState->Status = STATUS_ERROR;
        /* End transfer */
        Eio_I2s_Hal_StopTransfer((uint8)Channel);
        /* Call callback to announce the event to the user */
        if (ChannelState->Callback != NULL_PTR)
        {
            ChannelState->Callback((uint8)Channel, EIO_I2S_EVENT_END_TRANSFER);
        }
    }
    else
    {
        if (ChannelState->Callback != NULL_PTR)
        {
            ChannelState->Callback((uint8)Channel, EIO_I2S_EVENT_TX_EMPTY);
        }
    }

    if (ChannelState->TxRemainingBytes == 0U)
    {
        /* No more data to transmit, transmission will stop */
        ChannelState->TxData = NULL_PTR;
    }
    else
    {
        DmaTransferSize = DmaTransferSizeArray[(uint32)ChannelState->ByteWidth - 1U];
        DmaAddr.Length = (uint16)ChannelState->TxRemainingBytes;
        DmaAddr.SrcOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
        DmaAddr.DestOffset = 0U;
        DmaAddr.SrcStartAddr = (uint32)ChannelState->TxData;
        DmaAddr.DestStartAddr = Eio_I2s_Hal_ComputeTxRegAddr((uint8)Channel);
        DmaAddr.SrcEndAddr = DmaAddr.SrcStartAddr + \
                            (DmaAddr.Length / (1UL << (uint8)DmaTransferSize) * \
                            DmaAddr.SrcOffset);
        /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
        DmaAddr.DestEndAddr = DmaAddr.DestStartAddr + \
                            (DmaAddr.Length / (1UL << (uint8)DmaTransferSize) * \
                            DmaAddr.DestOffset);
        /*PRQA S 2985 -- */
        Dma_Hal_UpdateChAddr(ChannelState->TxDmaChannel, &DmaAddr);
        (void)Dma_Hal_StartCh(ChannelState->TxDmaChannel);
    }
}

/*!
 * @brief DMA receive completion callback
 *
 * @details
 * Called by the DMA HAL when a receive DMA transfer completes. Updates
 * driver state and invokes upper-layer callback if configured.
 *
 * @param[in] Param: User parameter passed by DMA driver (driver state)
 *
 * @return
 *     void
 *
 * @pre
 *     Param must be the pointer passed when configuring the DMA channel.
 *
 * @post
 *     Driver Rx state is updated and completion event may be propagated.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from DMA ISR context.
 *     DES ID: DES_I2S_API_026
 */
/*PRQA S 3673 ++ # AMCE_EIO_3673 */
static void Eio_I2s_Hal_EndDmaRxTransfer(void *Param)
/*PRQA S 3673 -- */
{
    /*PRQA S 0316 ++ # interrupt functions allow conversion of a pointer to void to a pointer to an object.*/
    const Dma_ChannelCBInfoType *DmaInfo = (const Dma_ChannelCBInfoType *)Param;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # user-defined callback parameters allow conversion between void and integer.*/
    uint32 Channel = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */
    uint32 Addr;
    Dma_ChannelAddrType DmaAddr;
    Dma_TransferUnitType DmaTransferSize;

    Eio_I2sStateType *ChannelState = EioI2sState[Channel];

    /* Check if this was a reception */
    if (ChannelState->RxData != NULL_PTR)
    {
        if (0U != (DmaInfo->DmaEvent & DMA_ERROR_EVENT))
        {
            /* DMA error, stop transfer */
            ChannelState->Status = STATUS_ERROR;
        }
        else
        {
            /* Call callback to allow user to provide a new buffer */
            if (ChannelState->Callback != NULL_PTR)
            {
                ChannelState->Callback((uint8)Channel, EIO_I2S_EVENT_RX_FULL);
            }
        }
    }

    if (ChannelState->RxRemainingBytes == 0U)
    {
        /* No more data, end transfer */
        if (ChannelState->Status == STATUS_BUSY)
        {
            ChannelState->Status = STATUS_SUCCESS;
        }
        /* End transfer */
        Eio_I2s_Hal_StopTransfer((uint8)Channel);
        /* Call callback to announce the event to the user */
        if (ChannelState->Callback != NULL_PTR)
        {
            ChannelState->Callback((uint8)Channel, EIO_I2S_EVENT_END_TRANSFER);
        }
    }
    else
    {
        /* There is more data to transfer, restart DMA channel */
        /* Update buffer address and size */
        if (ChannelState->RxData != NULL_PTR)
        {
            Addr = (uint32)(ChannelState->RxData);
        }
        else
        {
            /* if there is no data to receive, use dummy data as destination for DMA transfer */
            Addr = (uint32)(&(ChannelState->DummyDmaData));
        }

        DmaTransferSize = DmaTransferSizeArray[(uint32)ChannelState->ByteWidth - 1U];
        DmaAddr.Length = (uint16)ChannelState->RxRemainingBytes;
        DmaAddr.SrcOffset = 0U;
        if (ChannelState->RxData == NULL_PTR)
        {
            /* if there is no data to receive, don't increment destination offset */
            DmaAddr.DestOffset = 0U;
        }
        else
        {
            DmaAddr.DestOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
        }
        DmaAddr.SrcStartAddr = Eio_I2s_Hal_ComputeRxRegAddr((uint8)Channel);
        DmaAddr.DestStartAddr = Addr;
        /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
        DmaAddr.SrcEndAddr = DmaAddr.SrcStartAddr + \
                            (DmaAddr.Length / (1UL << (uint8)DmaTransferSize) * \
                            DmaAddr.SrcOffset);
        /*PRQA S 2985 -- */
        DmaAddr.DestEndAddr = DmaAddr.DestStartAddr + \
                            (DmaAddr.Length / (1UL << (uint8)DmaTransferSize) * \
                            DmaAddr.DestOffset);
        Dma_Hal_UpdateChAddr(ChannelState->RxDmaChannel, &DmaAddr);
        /* Now that this rx is set up, clear remaining bytes count */
        ChannelState->RxRemainingBytes = 0U;
        /* Start the channel */
        (void)Dma_Hal_StartCh(ChannelState->RxDmaChannel);
    }
}

/*!
 * @brief Start DMA transfers for the current transaction
 *
 * @details
 * Configure and enable DMA channels for TX/RX as required by the
 * current transfer. Sets up DMA descriptors and starts the channels.
 *
 * @param[in] Channel: Logical I2S channel index
 *
 * @return
 *     HAL_STATUS_SUCCESS on success or an error code from the DMA HAL
 *
 * @pre
 *     Channel driver must be configured and DMA channels available.
 *
 * @post
 *     DMA channels are started for the transaction.
 *
 * @note
 *     Reentrancy: Non-reentrant for the same Channel; reentrant for different Channels.
 *     DES ID: DES_I2S_API_027
 */
static Hal_StatusType Eio_I2s_Hal_StartDmaTransfer(uint8 Channel)
{
    uint8 ResourceIndex;    /* Index of first used internal resource Instance (shifter and timer) */
    EIO_Type *BaseAddr;
    uint32 Addr;
    uint8 RequestMask = 0U;
    Hal_StatusType RetCode = STATUS_SUCCESS;
    Dma_TransferConfigType TransferConfig;
    Dma_TransferUnitType DmaTransferSize;
    uint32 I2sChannel;

    BaseAddr = EioBase[EioI2sState[Channel]->EioCommon.Instance];
    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    ResourceIndex = ChannelState->EioCommon.ResourceIndex;
    DmaTransferSize = DmaTransferSizeArray[(uint32)ChannelState->ByteWidth - 1U];

    TransferConfig.TriggerMode = FALSE;
    TransferConfig.CircularMode = FALSE;
#if defined (AC7840E)
    TransferConfig.DaisyMode = FALSE;
#endif
    I2sChannel = Channel;
    /*PRQA S 0326 ++ # AMCE_EIO_0326.*/
    TransferConfig.UserArgs = (void *)(I2sChannel);
    /*PRQA S 0326 -- */
    TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
    /* Configure Tx channel if this is a transmission */
    if (ChannelState->TxData != NULL_PTR)
    {
        /* Configure the transfer control descriptor for the Tx channel */
        TransferConfig.SrcUnit = DmaTransferSize;
        TransferConfig.DestUnit = DmaTransferSize;
        TransferConfig.Type = DMA_TRANSFER_MEM2PERIPH;
        TransferConfig.SrcStartAddr = (uint32)ChannelState->TxData;
        TransferConfig.DestStartAddr = Eio_I2s_Hal_ComputeTxRegAddr(Channel);
        TransferConfig.SrcOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
        TransferConfig.DestOffset = 0U;
        TransferConfig.Length = (uint16)ChannelState->TxRemainingBytes;
        TransferConfig.Callback = (Hal_CallbackType)Eio_I2s_Hal_EndDmaTxTransfer;
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

        if (STATUS_SUCCESS == RetCode)
        {
            /* Now that this tx is set up, clear remaining bytes count */
            ChannelState->TxRemainingBytes = 0U;

            /* Start tx DMA channel */
            (void)Dma_Hal_StartCh(ChannelState->TxDmaChannel);
            RequestMask = (uint8)(1U << I2S_TX_SHIFTER(ResourceIndex));
        }
    }

    /* Configure Rx channel; if this is a transmission we still use Rx for the "end transfer" event */
    if (ChannelState->RxData != NULL_PTR)
    {
        Addr = (uint32)(ChannelState->RxData);
    }
    else
    {
        /* if there is no data to receive, use dummy data as destination for DMA transfer */
        Addr = (uint32)(&(ChannelState->DummyDmaData));
    }

    if (STATUS_SUCCESS == RetCode)
    {
        /* Configure the transfer control descriptor for the Rx channel */
        TransferConfig.SrcUnit = DmaTransferSize;
        TransferConfig.DestUnit = DmaTransferSize;
        TransferConfig.Type = DMA_TRANSFER_PERIPH2MEM;
        TransferConfig.SrcStartAddr = Eio_I2s_Hal_ComputeRxRegAddr(Channel);
        TransferConfig.DestStartAddr = Addr;
        TransferConfig.SrcOffset = 0U;
        if (ChannelState->RxData == NULL_PTR)
        {
            /* if there is no data to receive, don't increment destination offset */
            TransferConfig.DestOffset = 0U;
        }
        else
        {
            TransferConfig.DestOffset = (uint16)(1UL << ((uint16)DmaTransferSize));
        }
        TransferConfig.Length = (uint16)ChannelState->RxRemainingBytes;
        TransferConfig.Callback = (Hal_CallbackType)Eio_I2s_Hal_EndDmaRxTransfer;
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
            /* Now that this rx is set up, clear remaining bytes count */
            ChannelState->RxRemainingBytes = 0U;

            /* Start rx DMA channel */
            (void)Dma_Hal_StartCh(ChannelState->RxDmaChannel);
            RequestMask |= (uint8)(1U << I2S_RX_SHIFTER(ResourceIndex));

            /* Enable EIO DMA requests */
            Eio_Reg_SetShifterDMARequest(BaseAddr, RequestMask, TRUE);
        }
    }

    return RetCode;
}

/* Implementation of Eio_I2s_Hal_Init declared in Eio_I2s_Hal.h */
Hal_StatusType Eio_I2s_Hal_Init(uint8 Channel, const Eio_I2sUserConfigType *UserConfigPtr)
{
    DEVICE_ASSERT(Channel < I2S_MAX_CHANNEL);
    DEVICE_ASSERT(UserConfigPtr != NULL_PTR);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioI2sState[Channel] != NULL_PTR);
    /* Check that device was initialized */
    DEVICE_ASSERT(EioI2sState[Channel]->InitState == FALSE);

    uint32 InputClock;
    Hal_StatusType RetCode;
    /*PRQA S 2812 ++ # AMCE_EIO_2842.*/
    EIO_Type *BaseAddr = EioBase[UserConfigPtr->EioInstance];
    /*PRQA S 2812 -- */
    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    /*PRQA S 2842 -- */

    Eio_Hal_InitDevice(UserConfigPtr->EioInstance);

    if (UserConfigPtr->Mode == EIO_I2S_MASTER)
    {
        /* Get the protocol clock frequency */
        (void)Ckgen_Hal_GetFreq(Eio_HalClock[UserConfigPtr->EioInstance], &InputClock);
        DEVICE_ASSERT(InputClock > 0U);

        /* Instruct the resource allocator that we need two shifters/timers */
        ChannelState->EioCommon.ResourceIndex = UserConfigPtr->ResourceIndex;
        ChannelState->EioCommon.ResourceCount = 2U;
    }
    else
    {
        /* Instruct the resource allocator that we need three shifters/timers */
        ChannelState->EioCommon.ResourceIndex = UserConfigPtr->ResourceIndex;
        ChannelState->EioCommon.ResourceCount = 3U;
    }

    /* Common EIO driver initialization */
    RetCode = Eio_Hal_InitDriver(UserConfigPtr->EioInstance, &ChannelState->EioCommon);
    if (RetCode == STATUS_SUCCESS)
    {
        /* Initialize driver-specific context structure */
        ChannelState->DriverType = UserConfigPtr->DriverType;
        ChannelState->BitsWidth = UserConfigPtr->BitsWidth;
        ChannelState->ByteWidth = Eio_I2s_Hal_ComputeByteWidth(UserConfigPtr->BitsWidth);
        ChannelState->DriverIdle = TRUE;
        ChannelState->Callback = UserConfigPtr->Callback;
        ChannelState->TxData = NULL_PTR;
        ChannelState->TxRemainingBytes = 0U;
        ChannelState->RxData = NULL_PTR;
        ChannelState->RxRemainingBytes = 0U;
        ChannelState->Status = STATUS_SUCCESS;
        ChannelState->Master = (UserConfigPtr->Mode == EIO_I2S_MASTER) ? TRUE : FALSE;
        ChannelState->ClockContinueOutput = UserConfigPtr->ClockContinueOutput;

        if (UserConfigPtr->Mode == EIO_I2S_MASTER)
        {
            /* Configure device for I2S mode */
            Eio_I2s_Hal_MasterConfigure(Channel, UserConfigPtr, InputClock);
        }
        else
        {
            /* Configure device for I2S slave mode */
            Eio_I2s_Hal_SlaveConfigure(Channel, UserConfigPtr);
        }

        /* Set up transfer engine */
        switch (ChannelState->DriverType)
        {
            case EIO_DRIVER_TYPE_INTERRUPTS:
                ChannelState->EioCommon.Isr = Eio_I2s_Hal_CheckStatus;
                break;

            case EIO_DRIVER_TYPE_POLLING:
                /* Nothing to do here, Eio_I2s_Hal_GetStatus() will handle the transfer */
                break;

            case EIO_DRIVER_TYPE_DMA:
                /* Store DMA channel numbers */
                ChannelState->RxDmaChannel = UserConfigPtr->RxDmaChannel;
                ChannelState->TxDmaChannel = UserConfigPtr->TxDmaChannel;
                Eio_Reg_SetShifterDMARequest(BaseAddr,
                        (uint8)(((1U << I2S_TX_SHIFTER(ChannelState->EioCommon.ResourceIndex))) |
                        ((1U << I2S_RX_SHIFTER(ChannelState->EioCommon.ResourceIndex)))), TRUE);
                break;

            default:
                /* Impossible type - do nothing */
                break;
        }

        ChannelState->InitState = TRUE;
        s_i2sInit = TRUE;
    }

    return RetCode;
}

/* Implementation of Eio_I2s_Hal_Deinit declared in Eio_I2s_Hal.h */
Hal_StatusType Eio_I2s_Hal_Deinit(uint8 Channel)
{
    DEVICE_ASSERT(Channel < I2S_MAX_CHANNEL);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioI2sState[Channel] != NULL_PTR);

    Hal_StatusType RetCode = STATUS_SUCCESS;
    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    /*PRQA S 2842 -- */

    /* Check if driver is busy */
    if (FALSE ==ChannelState->DriverIdle)
    {
        RetCode = STATUS_BUSY;
    }
    else
    {
        Eio_Hal_DeinitDriver(&(ChannelState->EioCommon));
        Eio_Hal_DeinitDevice(ChannelState->EioCommon.Instance);
    }

    ChannelState->InitState = FALSE;

    return RetCode;
}

/* Implementation of Eio_I2s_Hal_SetSampleRate declared in Eio_I2s_Hal.h */
Hal_StatusType Eio_I2s_Hal_SetSampleRate(uint8 Channel, uint32 SampleRate)
{
    DEVICE_ASSERT(Channel < I2S_MAX_CHANNEL);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioI2sState[Channel] != NULL_PTR);

    uint16 Divider;
    uint32 InputClock;
    uint32 BaudRate;
    Hal_StatusType RetCode = STATUS_SUCCESS;
    uint8 ResourceIndex;    /* Index of first used internal resource Instance (shifter and timer) */

    EIO_Type *BaseAddr = EioBase[EioI2sState[Channel]->EioCommon.Instance];
    const Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    /*PRQA S 2842 -- */
    ResourceIndex = ChannelState->EioCommon.ResourceIndex;

    if (ChannelState->Master == FALSE)
    {
        RetCode = STATUS_UNSUPPORTED;
    }
    /* Check if driver is busy */
    else if (FALSE == ChannelState->DriverIdle)
    {
        RetCode = STATUS_BUSY;
    }
    else
    {
        /* Get the protocol clock frequency */
        (void)Ckgen_Hal_GetFreq(Eio_HalClock[ChannelState->EioCommon.Instance], &InputClock);
        DEVICE_ASSERT(InputClock > 0U);

        /* Calculate bit clock rate from sample rate: BaudRate = SampleRate * BitsWidth * 2 (stereo) */
        BaudRate = SampleRate * (uint32)ChannelState->BitsWidth * 2U;

        /* Compute Divider */
        Eio_I2s_Hal_MasterComputeBaudRateDivider(BaudRate, &Divider, InputClock);

        /* Configure SCK timer */
        Eio_Reg_SetTimerCompare(BaseAddr, I2S_SCK_TIMER(ResourceIndex),
                (uint16)((((uint16)((uint16)ChannelState->BitsWidth << 1U) - 1U) << 8U) + Divider));
        /* Configure WS timer */
        Eio_Reg_SetTimerCompare(BaseAddr, I2S_WS_TIMER(ResourceIndex),
                 (uint16)((ChannelState->BitsWidth * ((Divider + 1U) * 2U)) - 1U));
    }

    return RetCode;
}

/* Implementation of Eio_I2s_Hal_GetSampleRate declared in Eio_I2s_Hal.h */
Hal_StatusType Eio_I2s_Hal_GetSampleRate(uint8 Channel, uint32 *SampleRate)
{
    DEVICE_ASSERT(Channel < I2S_MAX_CHANNEL);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioI2sState[Channel] != NULL_PTR);
    DEVICE_ASSERT(SampleRate != NULL_PTR);

    Hal_StatusType RetCode = STATUS_SUCCESS;
    uint32 InputClock;
    uint16 Divider;
    uint16 TimerCmp;
    uint8 ResourceIndex;    /* Index of first used internal resource Instance (shifter and timer) */

    const EIO_Type *BaseAddr = EioBase[EioI2sState[Channel]->EioCommon.Instance];
    const Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    /*PRQA S 2842 -- */
    ResourceIndex = ChannelState->EioCommon.ResourceIndex;

    if (ChannelState->Master == FALSE)
    {
        RetCode = STATUS_UNSUPPORTED;
    }
    else
    {
        /* Get the protocol clock frequency */
        (void)Ckgen_Hal_GetFreq(Eio_HalClock[ChannelState->EioCommon.Instance], &InputClock);
        DEVICE_ASSERT(InputClock > 0U);

        /* Get the currently configured Divider */
        TimerCmp = Eio_Reg_GetTimerCompare(BaseAddr, I2S_SCK_TIMER(ResourceIndex));
        Divider = (uint16)(TimerCmp & 0x00FFU);

        /* Compute bit clock rate: input_clock / (2 * (Divider + 1)). Round to nearest integer */
        uint32 BaudRate = (InputClock + (uint32)Divider + 1U) / (2U * ((uint32)Divider + 1U));

        /*PRQA S 2812 ++ # AMCE_EIO_2812.*/
        /* Calculate sample rate from bit clock rate: SampleRate = BaudRate / (BitsWidth * 2) */
        *SampleRate = BaudRate / ((uint32)ChannelState->BitsWidth * 2U);
        /*PRQA S 2812 -- */
    }

    return RetCode;
}

/* Implementation of Eio_I2s_Hal_SendData declared in Eio_I2s_Hal.h */
Hal_StatusType Eio_I2s_Hal_SendData(uint8 Channel, const uint8 *TxBuff, uint32 TxSize)
{
    DEVICE_ASSERT(Channel < I2S_MAX_CHANNEL);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioI2sState[Channel] != NULL_PTR);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);
    DEVICE_ASSERT((TxSize % (uint32)(EioI2sState[Channel]->ByteWidth)) == 0U);
  
    Hal_StatusType RetCode = STATUS_SUCCESS;
    EIO_Type *BaseAddr = EioBase[EioI2sState[Channel]->EioCommon.Instance];
    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    /*PRQA S 2842 -- */
    uint8 ResourceIndex = ChannelState->EioCommon.ResourceIndex;    /* Index of first used internal resource Instance (shifter and timer) */

    /* Check if driver is busy */
    if (FALSE == ChannelState->DriverIdle)
    {
        RetCode = STATUS_BUSY;
    }
    else
    {
        ChannelState->TxData = TxBuff;
        ChannelState->TxRemainingBytes = TxSize;
        /* also count received data, it tells us when tx is actually completed */
        if (ChannelState->ClockContinueOutput == TRUE)
        {
            if (s_i2sInit == TRUE)
            {
                ChannelState->RxRemainingBytes = TxSize;
                s_i2sInit = FALSE;
            }
            else
            {
                ChannelState->RxRemainingBytes = TxSize + ChannelState->ByteWidth;
            }
        }
        else
        {
            ChannelState->RxRemainingBytes = TxSize;
        }

        ChannelState->RxData = NULL_PTR;
        ChannelState->Status = STATUS_BUSY;
        ChannelState->DriverIdle = FALSE;

        /* Enable pin output */
        Eio_Reg_SetShifterPinConfig(BaseAddr, I2S_TX_SHIFTER(ResourceIndex), EIO_PIN_CONFIG_OUTPUT);
        /* Enable transfer engine */
        switch (ChannelState->DriverType)
        {
            case EIO_DRIVER_TYPE_INTERRUPTS:
                /* Enable interrupts for Tx shifter */
                Eio_Reg_SetShifterInterrupt(BaseAddr,
                                        (uint8)(1U << I2S_TX_SHIFTER(ResourceIndex)),
                                        TRUE);
                Eio_Reg_SetShifterErrorInterrupt(BaseAddr,
                                            (uint8)(1U << I2S_TX_SHIFTER(ResourceIndex)),
                                            TRUE);
                /* Enable timers and shifters */
                Eio_I2s_Hal_EnableTransfer(Channel);
                break;

            case EIO_DRIVER_TYPE_POLLING:
                /* Enable timers and shifters */
                Eio_I2s_Hal_EnableTransfer(Channel);
                /* Call Eio_I2s_Hal_CheckStatus once to send the first byte */
                Eio_I2s_Hal_CheckStatus(&(ChannelState->EioCommon));
                break;

            case EIO_DRIVER_TYPE_DMA:
                RetCode = Eio_I2s_Hal_StartDmaTransfer(Channel);
                /* Enable timers and shifters */
                if (RetCode == STATUS_SUCCESS)
                {
                    Eio_I2s_Hal_EnableTransfer(Channel);
                }
                break;

            default:
                /* Impossible type - do nothing */
                break;
        }
    }

    return RetCode;
}

/* Implementation of Eio_I2s_Hal_ReceiveData declared in Eio_I2s_Hal.h */
Hal_StatusType Eio_I2s_Hal_ReceiveData(uint8 Channel, uint8 *RxBuff, uint32 RxSize)
{
    DEVICE_ASSERT(Channel < I2S_MAX_CHANNEL);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioI2sState[Channel] != NULL_PTR);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    DEVICE_ASSERT((RxSize % (uint32)(EioI2sState[Channel]->ByteWidth)) == 0U);

    Hal_StatusType RetCode = STATUS_SUCCESS;
    EIO_Type *BaseAddr = EioBase[EioI2sState[Channel]->EioCommon.Instance];
    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    /*PRQA S 2842 -- */
    uint8 ResourceIndex = ChannelState->EioCommon.ResourceIndex;

    /* Check if driver is busy */
    if (FALSE == ChannelState->DriverIdle)
    {
        RetCode = STATUS_BUSY;
    }
    else
    {
        ChannelState->RxData = RxBuff;
        ChannelState->RxRemainingBytes = RxSize;
        ChannelState->TxData = NULL_PTR;
        ChannelState->TxRemainingBytes = 0U;
        ChannelState->Status = STATUS_BUSY;
        ChannelState->DriverIdle = FALSE;
        /* Enable transfer engine */
        switch (ChannelState->DriverType)
        {
            case EIO_DRIVER_TYPE_INTERRUPTS:
                Eio_I2s_Hal_EnableTransfer(Channel);
                /* Enable interrupts for Rx shifter */
                Eio_Reg_SetShifterInterrupt(BaseAddr,
                                        (uint8)(1U << I2S_RX_SHIFTER(ResourceIndex)),
                                        TRUE);
                Eio_Reg_SetShifterErrorInterrupt(BaseAddr,
                                            (uint8)(1U << I2S_RX_SHIFTER(ResourceIndex)),
                                            TRUE);
                break;

            case EIO_DRIVER_TYPE_POLLING:
                Eio_I2s_Hal_EnableTransfer(Channel);
                /* Call Eio_I2s_Hal_CheckStatus once to send the first byte */
                Eio_I2s_Hal_CheckStatus(&(ChannelState->EioCommon));
                break;

            case EIO_DRIVER_TYPE_DMA:
                RetCode = Eio_I2s_Hal_StartDmaTransfer(Channel);
                if (RetCode == STATUS_SUCCESS)
                {
                    Eio_I2s_Hal_EnableTransfer(Channel);
                }
                break;

            default:
                /* Impossible type - do nothing */
                break;
        }

        /* For master we need to send a dummy char to start the clock.
        For slave just put a 0 in the buffer to keep the tx line clear while receiving. */
        Eio_Reg_WriteShifterBuffer(BaseAddr, I2S_TX_SHIFTER(ResourceIndex), 0x0U, EIO_SHIFTER_RW_MODE_BIT_SWAP);

    }

    return RetCode;
}

/* Implementation of Eio_I2s_Hal_TransferAbort declared in Eio_I2s_Hal.h */
Hal_StatusType Eio_I2s_Hal_TransferAbort(uint8 Channel)
{
    DEVICE_ASSERT(Channel < I2S_MAX_CHANNEL);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioI2sState[Channel] != NULL_PTR);

    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    /*PRQA S 2842 -- */

    /* Check if driver is busy */
    if (ChannelState->DriverIdle == FALSE)
    {
        ChannelState->Status = STATUS_I2S_ABORTED;
        Eio_I2s_Hal_StopTransfer(Channel);
    }

    return STATUS_SUCCESS;
}

/* Implementation of Eio_I2s_Hal_GetStatus declared in Eio_I2s_Hal.h */
Hal_StatusType Eio_I2s_Hal_GetStatus(uint8 Channel, uint32 *BytesRemaining)
{
    DEVICE_ASSERT(Channel < I2S_MAX_CHANNEL);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioI2sState[Channel] != NULL_PTR);

    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    /*PRQA S 2842 -- */
    uint32 RemainingBytes;

    /* Use rxRemainingBytes even for transmit; byte is not transmitted
    until rx shifter reports a receive event */
    RemainingBytes = ChannelState->RxRemainingBytes;

    if (ChannelState->DriverIdle == FALSE)
    {
        switch (ChannelState->DriverType)
        {
            case EIO_DRIVER_TYPE_POLLING:
                /* In polling mode advance the I2S transfer here */
                Eio_I2s_Hal_CheckStatus(ChannelState);
                break;

            case EIO_DRIVER_TYPE_DMA:
                /* In DMA mode just update the remaining count.
                DO NOT write RxRemainingBytes directly !!! */
                RemainingBytes = RemainingBytes - Dma_Hal_GetTransBytes(ChannelState->RxDmaChannel);
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

    return ChannelState->Status;
}

/* Implementation of Eio_I2s_Hal_MasterGetDefaultConfig declared in Eio_I2s_Hal.h */
void Eio_I2s_Hal_MasterGetDefaultConfig(Eio_I2sUserConfigType *UserConfigPtr)
{
    DEVICE_ASSERT(UserConfigPtr != NULL_PTR);

    if (UserConfigPtr != NULL_PTR)
    {
        *UserConfigPtr = DefaultMasterConfig;
    }
}

/* Implementation of Eio_I2s_Hal_SlaveGetDefaultConfig declared in Eio_I2s_Hal.h */
void Eio_I2s_Hal_SlaveGetDefaultConfig(Eio_I2sUserConfigType *UserConfigPtr)
{
    DEVICE_ASSERT(UserConfigPtr != NULL_PTR);

    if (UserConfigPtr != NULL_PTR)
    {
        *UserConfigPtr = DefaultSlaveConfig;
    }
}

/* Implementation of Eio_I2s_Hal_SlaveGetDefaultConfig declared in Eio_I2s_Hal.h */
void Eio_I2s_Hal_SetRxBuffer(uint8 Channel, uint8 *RxBuff, uint32 RxSize)
{
    DEVICE_ASSERT(Channel < I2S_MAX_CHANNEL);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioI2sState[Channel] != NULL_PTR);
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    DEVICE_ASSERT((RxSize % (uint32)(EioI2sState[Channel]->ByteWidth)) == 0U);

    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    /*PRQA S 2842 -- */
    ChannelState->RxData = RxBuff;
    ChannelState->RxRemainingBytes = RxSize;

}

/* Implementation of Eio_I2s_Hal_SlaveGetDefaultConfig declared in Eio_I2s_Hal.h */
void Eio_I2s_Hal_SetTxBuffer(uint8 Channel, const uint8 *TxBuff, uint32 TxSize)
{
    DEVICE_ASSERT(Channel < I2S_MAX_CHANNEL);
    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    DEVICE_ASSERT(EioI2sState[Channel] != NULL_PTR);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);
    DEVICE_ASSERT((TxSize % (uint32)(EioI2sState[Channel]->ByteWidth)) == 0U);

    Eio_I2sStateType *ChannelState = EioI2sState[Channel];
    /*PRQA S 2842 -- */
    ChannelState->TxData = TxBuff;
    ChannelState->TxRemainingBytes = TxSize;
    /* for transmit we also count received bytes for end condition */
    ChannelState->RxRemainingBytes += TxSize;
}

/* =============================================  EOF  ============================================== */
