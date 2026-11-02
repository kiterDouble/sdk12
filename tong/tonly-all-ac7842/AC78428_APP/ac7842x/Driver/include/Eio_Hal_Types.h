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
 * @file Eio_Hal_Types.h
 *
 * @brief This file provides eio integration functions interface.
 *
 */

#ifndef EIO_HAL_TYPES_H
#define EIO_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "Device_Register.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */
/**
 * @brief Shift clock polarity options.
 * @note DES ID: DES_EIO_TYP_023
 */
typedef enum
{
    EIO_TIMER_POLARITY_POSEDGE            = 0x00U,  /*!< Shift on positive edge of Shift clock */
    EIO_TIMER_POLARITY_NEGEDGE            = 0x01U   /*!< Shift on negative edge of Shift clock */
} Eio_TimerPolarityType;

/**
 * @brief Pin polarity options.
 * @note DES ID: DES_EIO_TYP_024
 */
typedef enum
{
    EIO_PIN_POLARITY_HIGH                  = 0x00U,  /*!< Pin is active high */
    EIO_PIN_POLARITY_LOW                   = 0x01U   /*!< Pin is active low */
} Eio_PinPolarityType;

/**
 * @brief Pin configuration options.
 * @note DES ID: DES_EIO_TYP_025
 */
typedef enum
{
    EIO_PIN_CONFIG_DISABLED                = 0x00U,  /*!< Shifter pin output disabled */
    EIO_PIN_CONFIG_OPEN_DRAIN              = 0x01U,  /*!< Shifter pin open drain or bidirectional output enable */
    EIO_PIN_CONFIG_BIDIR_OUTPUT            = 0x02U,  /*!< Shifter pin bidirectional output data */
    EIO_PIN_CONFIG_OUTPUT                  = 0x03U   /*!< Shifter pin output */
} Eio_PinConfigType;

/**
 * @brief Shifter mode options.
 * @note DES ID: DES_EIO_TYP_026
 */
typedef enum
{
    EIO_SHIFTER_MODE_DISABLED               = 0x00U,  /*!< Shifter disabled */
    EIO_SHIFTER_MODE_RECEIVE                = 0x01U,  /*!< Receive mode */
    EIO_SHIFTER_MODE_TRANSMIT               = 0x02U,  /*!< Transmit mode */
    EIO_SHIFTER_MODE_MATCH_STORE            = 0x04U   /*!< Match Store mode */
} Eio_ShifterModeType;

/**
 * @brief Shifter input source options.
 * @note DES ID: DES_EIO_TYP_027
 */
typedef enum
{
    EIO_SHIFTER_SOURCE_PIN                  = 0x00U,  /*!< Input source is selected pin */
    EIO_SHIFTER_SOURCE_SHIFTER              = 0x01U   /*!< Input source is shifter N+1 output */
} Eio_ShifterSourceType;

/**
 * @brief Read/Write mode for shifter buffer.
 * @note DES ID: DES_EIO_TYP_028
 */
typedef enum
{
    EIO_SHIFTER_RW_MODE_NORMAL              = 0x00U,  /*!< Normal shifter buffer read/write */
    EIO_SHIFTER_RW_MODE_BIT_SWAP            = 0x01U,  /*!< Data is bit-swapped */
    EIO_SHIFTER_RW_MODE_BYTE_SWAP           = 0x02U,  /* Data is byte-swapped */
    EIO_SHIFTER_RW_MODE_BIT_BYTE_SWAP       = 0x03U   /* Data is bit byte-swapped */
} Eio_ShifterBufferModeType;

/**
 * @brief Trigger polarity options.
 * @note DES ID: DES_EIO_TYP_029
 */
typedef enum
{
    EIO_TRIGGER_POLARITY_HIGH               = 0x00U,  /*!< Trigger is active high */
    EIO_TRIGGER_POLARITY_LOW                = 0x01U   /*!< Trigger is active low */
} Eio_TriggerPolarityType;

/**
 * @brief Trigger source options.
 * @note DES ID: DES_EIO_TYP_030
 */
typedef enum
{
    EIO_TRIGGER_SOURCE_EXTERNAL             = 0x00U,  /*!< External trigger selected */
    EIO_TRIGGER_SOURCE_INTERNAL             = 0x01U   /*!< Internal trigger selected */
} Eio_TriggerSourceType;

/**
 * @brief Timer mode options.
 * @note DES ID: DES_EIO_TYP_031
 */
typedef enum
{
    EIO_TIMER_MODE_DISABLED                 = 0x00U,  /*!< Timer Disabled */
    EIO_TIMER_MODE_8BIT_BAUD                = 0x01U,  /*!< Dual 8-bit counters baud/bit mode */
    EIO_TIMER_MODE_8BIT_PWM                 = 0x02U,  /*!< Dual 8-bit counters PWM mode */
    EIO_TIMER_MODE_16BIT                    = 0x03U   /*!< Single 16-bit counter mode */
} Eio_TimerModeType;

/**
 * @brief Timer initial output options.
 * @note DES ID: DES_EIO_TYP_032
 */
typedef enum
{
    EIO_TIMER_INITOUT_ONE                    = 0x00U,  /*!< Timer output is logic one when enabled, unaffected by timer reset */
    EIO_TIMER_INITOUT_ZERO                   = 0x01U,  /*!< Timer output is logic zero when enabled, unaffected by timer reset */
    EIO_TIMER_INITOUT_ONE_RESET              = 0x02U,  /*!< Timer output is logic one when enabled and on timer reset */
    EIO_TIMER_INITOUT_ZERO_RESET             = 0x03U   /*!< Timer output is logic zero when enabled and on timer reset */
} Eio_TimerOutputType;

/**
 * @brief Timer decrement options.
 * @note DES ID: DES_EIO_TYP_033
 */
typedef enum
{
    EIO_TIMER_DECREMENT_CLK_SHIFT_TMR        = 0x00U,  /*!< Decrement counter on EIO clock, Shift clock equals Timer output */
    EIO_TIMER_DECREMENT_TRG_SHIFT_TMR        = 0x01U,  /*!< Decrement counter on Trigger input (both edges), Shift clock equals Timer output */
    EIO_TIMER_DECREMENT_PIN_SHIFT_PIN        = 0x02U,  /*!< Decrement counter on Pin input (both edges), Shift clock equals Pin input */
    EIO_TIMER_DECREMENT_TRIG_SHIFT_TRIG      = 0x03U   /*!< Decrement counter on Trigger input (both edges), Shift clock equals Trigger input */
} Eio_TimerDecrementType;

/**
 * @brief Timer reset options.
 * @note DES ID: DES_EIO_TYP_034
 */
typedef enum
{
    EIO_TIMER_RESET_NEVER                    = 0x00U,  /*!< Timer never reset */
    EIO_TIMER_RESET_PIN_OUT                  = 0x02U,  /*!< Timer reset on Timer Pin equal to Timer Output */
    EIO_TIMER_RESET_TRG_OUT                  = 0x03U,  /*!< Timer reset on Timer Trigger equal to Timer Output */
    EIO_TIMER_RESET_PIN_RISING               = 0x04U,  /*!< Timer reset on Timer Pin rising edge */
    EIO_TIMER_RESET_TRG_RISING               = 0x06U,  /*!< Timer reset on Trigger rising edge */
    EIO_TIMER_RESET_TRG_BOTH                 = 0x07U   /*!< Timer reset on Trigger rising or falling edge */
} Eio_TimerResetType;

/**
 * @brief Timer disable options.
 * @note DES ID: DES_EIO_TYP_035
 */
typedef enum
{
    EIO_TIMER_DISABLE_NEVER                  = 0x00U,  /*!< Timer never disabled */
    EIO_TIMER_DISABLE_TIM_DISABLE            = 0x01U,  /*!< Timer disabled on Timer N-1 disable */
    EIO_TIMER_DISABLE_TIM_CMP                = 0x02U,  /*!< Timer disabled on Timer compare */
    EIO_TIMER_DISABLE_TIM_CMP_TRG_LOW        = 0x03U,  /*!< Timer disabled on Timer compare and Trigger Low */
    EIO_TIMER_DISABLE_PIN                    = 0x04U,  /*!< Timer disabled on Pin rising or falling edge */
    EIO_TIMER_DISABLE_PIN_TRG_HIGH           = 0x05U,  /*!< Timer disabled on Pin rising or falling edge provided Trigger is high */
    EIO_TIMER_DISABLE_TRG                    = 0x06U   /*!< Timer disabled on Trigger falling edge */
} Eio_TimerDisableType;

/**
 * @brief Timer enable options.
 * @note DES ID: DES_EIO_TYP_036
 */
typedef enum
{
    EIO_TIMER_ENABLE_ALWAYS                  = 0x00U,  /*!< Timer always enabled */
    EIO_TIMER_ENABLE_TIM_ENABLE              = 0x01U,  /*!< Timer enabled on Timer N-1 enable */
    EIO_TIMER_ENABLE_TRG_HIGH                = 0x02U,  /*!< Timer enabled on Trigger high */
    EIO_TIMER_ENABLE_TRG_PIN_HIGH            = 0x03U,  /*!< Timer enabled on Trigger high and Pin high */
    EIO_TIMER_ENABLE_PIN_POSEDGE             = 0x04U,  /*!< Timer enabled on Pin rising edge */
    EIO_TIMER_ENABLE_PIN_POSEDGE_TRG_HIGH    = 0x05U,  /*!< Timer enabled on Pin rising edge and Trigger high */
    EIO_TIMER_ENABLE_TRG_POSEDGE             = 0x06U,  /*!< Timer enabled on Trigger rising edge */
    EIO_TIMER_ENABLE_TRG_EDGE                = 0x07U   /*!< Timer enabled on Trigger rising or falling edge */
} Eio_TimerEnableType;

/**
 * @brief Timer stop bit options.
 * @note DES ID: DES_EIO_TYP_037
 */
typedef enum
{
    EIO_TIMER_STOP_BIT_DISABLED              = 0x00U,  /*!< Stop bit disabled */
    EIO_TIMER_STOP_BIT_TIM_CMP               = 0x01U,  /*!< Stop bit is enabled on timer compare */
    EIO_TIMER_STOP_BIT_TIM_DIS               = 0x02U,  /*!< Stop bit is enabled on timer disable */
    EIO_TIMER_STOP_BIT_TIM_CMP_DIS           = 0x03U   /*!< Stop bit is enabled on timer compare and disable */
} Eio_TimerStopType;

/**
 * @brief Shifter stop bit options for Transmit, Receive or Match Store modes.
 * @note DES ID: DES_EIO_TYP_038
 */
typedef enum
{
    EIO_SHIFTER_STOP_BIT_DISABLED            = 0x00U,  /*!< Stop bit disabled */
    EIO_SHIFTER_STOP_BIT_0                   = 0x02U,  /*!< Transmit/expect stop bit value 0 */
    EIO_SHIFTER_STOP_BIT_1                   = 0x03U   /*!< Transmit/expect stop bit value 1 */
} Eio_ShifterStopType;

/**
 * @brief Shifter start bit options for Transmit, Receive or Match Store modes.
 * @note DES ID: DES_EIO_TYP_039
 */
typedef enum
{
    EIO_SHIFTER_START_BIT_DISABLED           = 0x00U,  /*!< Start bit disabled, transmitter loads data on enable */
    EIO_SHIFTER_START_BIT_DISABLED_SH        = 0x01U,  /*!< Start bit disabled, transmitter loads data on first shift */
    EIO_SHIFTER_START_BIT_0                  = 0x02U,  /*!< Transmit/expect start bit value 0 */
    EIO_SHIFTER_START_BIT_1                  = 0x03U   /*!< Transmit/expect start bit value 1 */
} Eio_ShifterStartType;

/**
 * @brief Timer start bit options.
 * @note DES ID: DES_EIO_TYP_040
 */
typedef enum
{
    EIO_TIMER_START_BIT_DISABLED             = 0x00U,  /*!< Start bit disabled */
    EIO_TIMER_START_BIT_ENABLED              = 0x01U   /*!< Start bit enabled */
} Eio_TimerStartType;

/**
 * @brief Driver type options for data transfer.
 * @note DES ID: DES_EIO_TYP_041
 */
typedef enum
{
    EIO_DRIVER_TYPE_INTERRUPTS    = 0U,  /*!< Driver uses interrupts for data transfers */
    EIO_DRIVER_TYPE_POLLING       = 1U,  /*!< Driver is based on polling */
    EIO_DRIVER_TYPE_DMA           = 2U   /*!< Driver uses DMA for data transfers */
} Eio_DriverType;

/**
 * @brief EIO interrupt service routine callback function type.
 * @note DES ID: DES_EIO_TYP_042
 */
typedef void (*Eio_IsrType)(void *IsrParam);

/**
 * @brief EIO common driver context structure.
 * @note DES ID: DES_EIO_TYP_043
 */
typedef struct
{
    Eio_IsrType Isr;          /*!< Interrupt handler for this driver instance */
    uint8 Instance;           /*!< EIO device instance number */
    uint8 ResourceCount;      /*!< Count of internal resources used (shifters and timers) */
    uint8 ResourceIndex;      /*!< Index of first used internal resource instance (shifter and timer) */
    void *UserArgs;           /*!< User defined arguments pointer */
} Eio_CommonStateType;

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EIO_HAL_TYPES_H */

/* =============================================  EOF  ============================================== */
