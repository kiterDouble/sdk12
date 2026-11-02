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
    * @file Acmp_Hal_Types.h
    * @brief This file provides ACMP config
 */

#ifndef ACMP_HAL_TYPES_H
#define ACMP_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/
/**
 * @brief ACMP input channel number.
 * @note DES ID: DES_ACMP_TYP_011
 */
typedef enum
{
    ACMP_EXTERNAL_CH0 = 0U,  /*!< External channel 0 */
    ACMP_EXTERNAL_CH1,       /*!< External channel 1 */
    ACMP_EXTERNAL_CH2,       /*!< External channel 2 */
    ACMP_EXTERNAL_CH3,       /*!< External channel 3 */
    ACMP_EXTERNAL_CH4,       /*!< External channel 4 */
    ACMP_EXTERNAL_CH5,       /*!< External channel 5 */
    ACMP_EXTERNAL_CH6,       /*!< External channel 6 */
    ACMP_EXTERNAL_CH7,       /*!< External channel 7 */
    ACMP_DAC_OUTPUT,         /*!< DAC output channel 8 */
    ACMP_CHANNEL_MAX         /*!< Invalid channel */
} Acmp_ChNumberType;

/**
 * @brief ACMP output interrupt trigger type.
 * @note DES ID: DES_ACMP_TYP_005
 */
typedef enum
{
    ACMP_FALLING_EDGE = 0U,  /*!< Trigger interrupt on falling edge */
    ACMP_RISING_EDGE = 1U,   /*!< Trigger interrupt on rising edge */
    ACMP_BOTH_EDGES = 3U     /*!< Trigger interrupt on falling/rising edge */
} Acmp_OutputTriggerType;

/**
 * @brief ACMP result output signal select.
 * @note DES ID: DES_ACMP_TYP_006
 */
typedef enum
{
    ACMP_COUT = 0U,   /*!< Select filtered result COUT as comparator output signal. */
    ACMP_COUTA = 1U   /*!< Select unfiltered result COUTA as comparator output signal. */
} Acmp_OutputSelectType;

/**
 * @brief ACMP hysteresis mode type.
 * @note DES ID: DES_ACMP_TYP_009
 */
typedef enum
{
    ACMP_HYS_FALLING_EDGE = 0U,  /*!< hysteresis direction on falling edge */
    ACMP_HYS_BOTH_EDGE           /*!< hysteresis direction on both edge */
} Acmp_HysteresisModeType;

/**
 * @brief ACMP hysteresis level type.
 * @note DES ID: DES_ACMP_TYP_010
 */
typedef enum
{
    ACMP_LEVEL_HYS_0MV = 0U,  /*!< 0mv  LEVEL_HYS voltage */
    ACMP_LEVEL_HYS_10MV,      /*!< 10mv LEVEL_HYS voltage */
    ACMP_LEVEL_HYS_20MV,      /*!< 20mv LEVEL_HYS voltage */
    ACMP_LEVEL_HYS_40MV       /*!< 40mv LEVEL_HYS voltage */
} Acmp_HysteresisType;

/**
 * @brief ACMP digital filter clock divide type.
 * @note DES ID: DES_ACMP_TYP_007
 */
typedef enum
{
    ACMP_FLT_DIVIDE_1 = 0U,  /*!< Input clock divided by 1. */
    ACMP_FLT_DIVIDE_2,       /*!< Input clock divided by 2. */
    ACMP_FLT_DIVIDE_3,       /*!< Input clock divided by 3. */
    ACMP_FLT_DIVIDE_4,       /*!< Input clock divided by 4. */
    ACMP_FLT_DIVIDE_5,       /*!< Input clock divided by 5. */
    ACMP_FLT_DIVIDE_6,       /*!< Input clock divided by 6. */
    ACMP_FLT_DIVIDE_7,       /*!< Input clock divided by 7. */
    ACMP_FLT_DIVIDE_8        /*!< Input clock divided by 8. */
} Acmp_FilterDivideType;

/**
 * @brief ACMP low pass filter bandwidth type.
 * @note DES ID: DES_ACMP_TYP_008
 */
typedef enum
{
    ACMP_LPF_500KHZ = 0U,  /*!< 500KHz filter frequence */
    ACMP_LPF_1MHZ,         /*!< 1MHz filter frequence */
    ACMP_LPF_2MHZ,         /*!< 2MHz filter frequence */
    ACMP_LPF_BYPASS        /*!< Bypass filter frequence */
} Acmp_LowPassFilterType;

/**
 * @brief ACMP DAC voltage reference source type.
 * @note DES ID: DES_ACMP_TYP_012
 */
typedef enum
{
    ACMP_DAC_BANDGAP = 0U,  /*!< Bandgap as reference source */
    ACMP_DAC_VDD            /*!< Vdd as reference source */
} Acmp_VoltageReferenceType;

/**
 * @brief ACMP input channel polling mode type.
 * @note DES ID: DES_ACMP_TYP_014
 */
typedef enum
{
    ACMP_NONE_POLLING = 0U,  /*!< Not using polling mode, which is normal comparator mode */
    ACMP_POSITIVE_POLLING,   /*!< Positive polling */
    ACMP_NEGATIVE_POLLING    /*!< Negative polling */
} Acmp_InputPollingType;

/**
 * @brief ACMP polling clock divide type.
 * @note DES ID: DES_ACMP_TYP_013
 */
typedef enum
{
    ACMP_CLK_DIVIDE_256 = 0U,  /*!< Clock/256 */
    ACMP_CLK_DIVIDE_101,       /*!< Clock/101 */
    ACMP_CLK_DIVIDE_71,        /*!< Clock/71 */
    ACMP_CLK_DIVIDE_51,        /*!< Clock/51 */
    ACMP_CLK_DIVIDE_MAX        /*!< Invalid divid ratio */
} Acmp_PollingClkDivideType;
/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/**
 * @brief ACMP interrupt callback function type.
 * @note DES ID: DES_ACMP_TYP_015
 */
typedef void (*Acmp_CallbackType)(uint8 Instance, uint32 Status);

/**
 * @brief ACMP comparator configuration type.
 * @note DES ID: DES_ACMP_TYP_001
 */
typedef struct
{
    boolean InterruptEn;                    /*!< Interrupt enable flag */
    boolean FilterEnable;                   /*!< Digital filter enable flag */
    boolean InverterEnable;                 /*!< ACMP result invertion enable flag */
    boolean PinState;                       /*!< ACMP output pin enable flag */
#if !defined(AC7840E)
    boolean UsingLSIEnable;                 /*!< Using LSI as function clock enable flag */
#endif
    boolean WindowModeEnable;               /*!< Window mode enable flag */
    boolean PowerEnable;                    /*!< ACMP analog module power enable flag */
    uint8 FilterSampleCount;                /*!< Digital filter sample count */
    Acmp_OutputTriggerType OutputTrigger;   /*!< Trigger mode */
    Acmp_OutputSelectType OutputSelect;     /*!< Output signal selection */
    Acmp_FilterDivideType ClockDivide;      /*!< Digital filter clock divide */
    Acmp_LowPassFilterType LpfBandwidth;    /*!< Low pass filter bandwidth */
    Acmp_HysteresisModeType HysteresisMode; /*!< Hysteresis mode */
    Acmp_HysteresisType HysteresisLevel;    /*!< Hysteresis level selection */
    Acmp_CallbackType Callback;             /*!< ACMP interrupt callback function pointer */
} Acmp_ComparatorType;

/**
 * @brief ACMP analog mux input configuration type.
 * @note DES ID: DES_ACMP_TYP_002
 */
typedef struct
{
    Acmp_ChNumberType PositiveInputMux;  /*!< Positive input channel selection */
    Acmp_ChNumberType NegativeInputMux;  /*!< Negative input channel selection */
} Acmp_AnmuxType;

/**
 * @brief ACMP DAC configuration type.
 * @note DES ID: DES_ACMP_TYP_003
 */
typedef struct
{
    Acmp_VoltageReferenceType VoltageReferenceSource;   /*!< DAC voltage reference source selection */
    uint8 Voltage;                                      /*!< DAC output code */
    boolean State;                                      /*!< DAC power enable flag */
#if defined (AC7842X)  || defined (AC7843X)
    boolean OutToPin;                                   /*!< DAC output to pin */
    boolean BufferEnable;                               /*!< DAC buffer enable */
#endif
} Acmp_DacType;

/**
 * @brief ACMP polling mode configuration.
 * @note DES ID: DES_ACMP_TYP_004
 */
typedef struct
{
    Acmp_PollingClkDivideType PollingClockDivide;   /*!< Polling clock divide */
    Acmp_InputPollingType Mode;                     /*!< Polling mode selection */
    uint16 PollingSequence;                         /*!< Polling sequence, using bit0:8, each bit for each channel,
                                                         bit 0 represent channel 0, and other bits likewise */
    boolean HallOutputEnable;                       /*!< Hall output enable flag */
    Acmp_ChNumberType HallAOutputCh;                /*!< Hall A output channel selection */
    Acmp_ChNumberType HallBOutputCh;                /*!< Hall B output channel selection */
    Acmp_ChNumberType HallCOutputCh;                /*!< Hall C output channel selection */
} Acmp_PollingModeType;

/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/**
 * @brief ACMP module configuration type.
 * @note DES ID: DES_ACMP_TYP_000
 */
typedef struct
{
    Acmp_ComparatorType Comparator;    /*!< comparator basic configuration */
    Acmp_AnmuxType Mux;                /*!< Input mux configuration */
    Acmp_DacType Dac;                  /*!< DAC configuration */
    Acmp_PollingModeType PollingMode;  /*!< Polling mode configuration */
} Acmp_ModuleType;
/*PRQA S 3630 -- # AMCE_QAC_3630 */
/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

#ifdef __cplusplus
}
#endif

#endif
/*============================================EOF===================================================*/
