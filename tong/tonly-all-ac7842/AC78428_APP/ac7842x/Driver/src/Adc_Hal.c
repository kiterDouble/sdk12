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
 * @file Adc_Hal.c
 * @brief This file provides all adc hal api.
 */
/* ===========================================  INCLUDE FILES  =========================================== */
#include "AC784xx_Ctu_Reg.h"
#include "OsIf_Time.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Dma_Hal.h"
#include "Adc_Hal.h"
#include "Core_Hal.h"

/* ============================================  DEFINES AND MACROS  ============================================ */
/**
 * @def ROUND
 * @brief Macro for rounding float to signed integer.
 * @note DES ID: DES_ADC_MACRO_207
 */
#define ROUND(x) ((x) >= 0 ? ((sint16)((x) + 0.5)) : ((sint16)((x) - 0.5)))
/* ============================================= TYPEDEFS ================================================ */
/* =========================================== LOCAL VARIABLES ============================================== */
/**
 * @brief Table of ADC base addresses.
 * @note DES ID: DES_ADC_VAR_201
 */
static ADC_Type *const AdcBase[ADC_INSTANCE_MAX] = ADC_BASE_PTRS;

/**
 * @brief Table of ADC interrupt handler information.
 * @note DES ID: DES_ADC_VAR_202
 */
static Adc_InterruptType AdcISR[ADC_INSTANCE_MAX];

/**
 * @brief Table of ADC IRQ IDs.
 * @note DES ID: DES_ADC_VAR_203
 */
static const IRQn_Type AdcIrqs[ADC_INSTANCE_MAX] = ADC_IRQS;

/**
 * @brief Table of ADC CKGEN interface clocks.
 * @note DES ID: DES_ADC_VAR_204
 */
static const Ckgen_BusClkIdType AdcCkgenBusClock[ADC_INSTANCE_MAX] = ADC_CKGEN_BUS_CLKS;

/**
 * @brief Table of ADC soft resets.
 * @note DES ID: DES_ADC_VAR_205
 */
#if defined (AC7840E)
static const Rcm_ResetIDType AdcClockReset[ADC_INSTANCE_MAX] = {RCM_RESET_ID_ADC0};
#else
static const Rcm_ResetIDType AdcClockReset[ADC_INSTANCE_MAX] = {RCM_RESET_ID_ADC0, RCM_RESET_ID_ADC1};
#endif
/* ====================================  FUNCTION PROTOTYPES  ===================================== */

#ifndef ADC_SDK_NON_EXTENDED_API
/**
 * @brief Configures ADC0 and ADC1 interleave mode for simultaneous sampling.
 *
 * @details
 * This function enables or disables the interleave mode between ADC0 and ADC1, allowing
 * them to sample the same external channel simultaneously to achieve higher sampling rates.
 * The function configures the CTU (Cross Triggering Unit) registers and sets up the
 * appropriate ADC channel mappings based on the selected interleave mode.
 *
 * @param[in] Instance: ADC hardware instance number (0 for ADC0, 1 for ADC1).
 * @param[in] Interleave: Interleave mode selection (DISABLE, INTERLEAVE_0, or INTERLEAVE_1).
 *
 * @return void
 *
 * @pre
 *     Both ADC0 and ADC1 must be initialized.
 *     CTU module must be available and initialized.
 *
 * @post
 *     ADC interleave mode is configured according to the selected mode.
 *     CTU registers are updated with the appropriate channel mappings.
 *
 * @note DES ID: DES_ADC_API_228
 */
static void Adc_SetInterleave(uint8 Instance, Adc_InterleaveType Interleave);
#endif

/**
 * @brief Common interrupt service routine handler for ADC events.
 *
 * @details
 * This internal function processes ADC interrupt events including end-of-conversion (EOC)
 * for both regular and injected groups, and analog monitor (AMO) events. It checks the
 * interrupt status, clears the appropriate flags, and invokes the user-registered callback
 * function with event information. The function handles interrupts for both regular and
 * injected conversion sequences.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 *
 * @return void
 *
 * @pre
 *     ADC must be initialized with interrupt callback configured.
 *     NVIC must be enabled for the ADC interrupt.
 *
 * @post
 *     Interrupt flags are cleared.
 *     User callback is invoked with appropriate event information.
 *
 * @note
 *     This function is called from ADC0_IRQHandler and ADC1_IRQHandler.
 *     DES ID: DES_ADC_API_231
 */
static void Adc_CommonISR(uint8 Instance);

/**
 * @brief De-initializes and stops the DMA channel associated with an ADC instance.
 *
 * @details
 * This function stops the DMA channel that was previously configured for ADC data transfer
 * and disables all DMA interrupts. It retrieves the DMA channel ID based on the ADC instance
 * and performs cleanup operations. This function should be called during ADC de-initialization
 * to properly release DMA resources.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 *
 * @return void
 *
 * @pre
 *     DMA channel must have been previously configured for the ADC instance.
 *
 * @post
 *     DMA channel is stopped and all DMA interrupts are disabled.
 *     DMA resources are released for the ADC instance.
 *
 * @note DES ID: DES_ADC_API_232
 */
static void Adc_DmaDeinit(uint8 Instance);

#if defined (AC7843X)
/**
 * @brief Configures the analog multiplexer (AMUX) for ADC internal channel sources.
 *
 * @details
 * This function configures the AMUX settings to enable ADC internal channel measurements
 * such as bandgap reference voltage, on-chip temperature sensor, and other analog sources.
 * For AC7843X platform, it sets up both the ADC instance and the specific internal channel
 * routing through the analog multiplexer hardware. The function enables the appropriate
 * analog source and configures the signal path for accurate internal measurements.
 *
 * @param[in]: Instance: ADC hardware instance number (0 or 1).
 * @param[in]: Channel: Internal channel identifier specifying the analog source (e.g., bandgap, temperature sensor).
 *
 * @return void
 *
 * @pre
 *     ADC instance must be initialized.
 *     Internal channel type must be selected in ADC configuration.
 *     AMUX hardware must be available and not in use by other peripherals.
 *
 * @post
 *     AMUX is configured to route the specified internal analog source to the ADC.
 *     Selected internal channel source is enabled and ready for conversion.
 *
 * @note DES ID: DES_ADC_API_233
 */
static void Adc_SetInternalChannelAna(uint32 Instance, Adc_InputChannelType Channel);
#elif defined (AC7842X) || defined (AC7840E)
/**
 * @brief Configures the analog multiplexer (AMUX) buffer for ADC internal channel sources.
 *
 * @details
 * This function configures the AMUX buffer settings to enable ADC internal channel measurements
 * for AC7842X platform. It selects the appropriate internal analog source (such as bandgap voltage,
 * temperature sensor, or supply monitoring channels) and configures the AMUX buffer and output path.
 * The function handles platform-specific analog routing to ensure proper signal conditioning and
 * measurement accuracy for internal voltage and temperature references.
 *
 * @param[in]: InternalChannelSrc: Internal channel source type specifying which analog source to connect (bandgap, VDDA, temperature, etc.).
 *
 * @return void
 *
 * @pre
 *     ADC must be configured for internal channel measurements.
 *     AMUX buffer must be available and not conflicting with other analog functions.
 *
 * @post
 *     AMUX buffer is configured based on the selected internal channel source.
 *     AMUX output is routed to the ADC input for the specified internal measurement.
 *     Internal analog source is enabled and stable for conversion.
 *
 * @note DES ID: DES_ADC_API_234
 */
static void Adc_SetInternalChannelAna(Adc_InternalChannelSrcType InternalChannelSrc);
#endif

/*=================================GLOBAL FUNCTION IMPLEMENTATIONS=================================*/
/* Implementation of Adc_Hal_Init declared in Adc_Hal.h */
void Adc_Hal_Init(uint8 Instance, const Adc_InitConfigType *Config)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    /*PRQA S 2842 ++ # AMCE_ADC_2842.*/
    /*PRQA S 2812 ++ # AMCE_ADC_2812.*/
    AdcISR[Instance].Callback = Config->Interrupt.Callback;
    (void)Ckgen_Hal_EnablePeriphClk(AdcCkgenBusClock[Instance], TRUE);
    Rcm_Hal_SetResetState(AdcClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(AdcClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    /*Enable nvic*/
    Core_Hal_EnableIrq(AdcIrqs[Instance]);
    /*PRQA S 2812 -- # AMCE_ADC_2812.*/
    /*PRQA S 2842 -- # AMCE_ADC_2842.*/
}

/* Implementation of Adc_Hal_Deinit declared in Adc_Hal.h */
void Adc_Hal_Deinit(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);

    Adc_DmaDeinit(Instance);
    /* Disable nvic */
    /*PRQA S 2842 ++ # AMCE_ADC_2842.*/
    Core_Hal_DisableIrq(AdcIrqs[Instance]);
    Rcm_Hal_SetResetState(AdcClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(AdcClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    (void)Ckgen_Hal_EnablePeriphClk(AdcCkgenBusClock[Instance], FALSE);
    Core_Hal_ClearPendingIrq(AdcIrqs[Instance]);
    /*PRQA S 2842 -- # AMCE_ADC_2842.*/
}

/* Implementation of Adc_Hal_InitConverterStruct declared in Adc_Hal.h */
void Adc_Hal_InitConverterStruct(Adc_ConverterConfigType *const Config)
{
    DEVICE_ASSERT(Config != NULL_PTR);

    static const Adc_ConverterConfigType DefaultAdcConverterStruct =
    {
        .ClockDivide = ADC_CLK_DIVIDE_16,
        .Resolution = ADC_RESOLUTION_12BIT,
        .Alignment = ADC_ALIGN_RIGHT,
        .VoltageRef = ADC_VOLTAGEREF_VREF,
        .DmaEnable = FALSE,
        .PowerEn = TRUE,
        .HwAverage =
        {
            .Enable = FALSE,
            .Value = ADC_AVERAGE_4,
        },
        .Interleave = ADC_INTERLEAVE_DISABLE
    };

    if (Config != NULL_PTR)
    {
        *Config = DefaultAdcConverterStruct;
    }
}

/* Implementation of Adc_Hal_ConfigConverter declared in Adc_Hal.h */
void Adc_Hal_ConfigConverter(uint8 Instance, const Adc_ConverterConfigType *Config)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    const Ckgen_ClkIdType AdcCkgenClock[ADC_INSTANCE_MAX] = ADC_CKGEN_CLOCKS;
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842
    uint32 Freq = 0;
    Hal_StatusType ReturnValue;

    ReturnValue = Ckgen_Hal_GetFreq(AdcCkgenClock[Instance], &Freq);//PRQA S 2842 # AMCE_ADC_2842
    DEVICE_ASSERT(ReturnValue == STATUS_SUCCESS);
    DEVICE_ASSERT((Freq / ((uint32)Config->ClockDivide + 1U)) <= 30000000U);//PRQA S 2842,2812 # AMCE_ADC_2842,AMCE_ADC_2812
    (void)ReturnValue;
    Adc_Reg_SetClockPrescaler(Base, Config->ClockDivide); /* Set clock divide */
#ifndef AC7840E
    Adc_Reg_SetResolution(Base, Config->Resolution); /* Set Resolution */
#endif
    Adc_Reg_SetDataAlign(Base, Config->Alignment); /* Set Alignment */
    Adc_Reg_SetVoltageReference(Base, Config->VoltageRef); /* Set voltage reference */
    Adc_Reg_SetDMAEnableFlag(Base, Config->DmaEnable);
    Adc_Reg_SetAverageEnableFlag(Base, Config->HwAverage.Enable);
    Adc_Reg_SetAverageMode(Base, (uint8)Config->HwAverage.Value);
#ifndef ADC_SDK_NON_EXTENDED_API
    Adc_SetInterleave(Instance, Config->Interleave);
#endif
    Adc_Reg_SetCalibrationEnableFlag(Base, FALSE);
    Adc_Reg_SetPowerEnableFlag(Base, Config->PowerEn); /* Set power */
#ifndef AC7840E
    OsIf_UDelay(100);
#else
    /* wait ADC LDO ready*/
    while (TRUE != Adc_Reg_GetLdoFlag(Base))
    {
        /* Wait */
    }
#endif
}

/* Implementation of Adc_Hal_InitChanStruct declared in Adc_Hal.h */
void Adc_Hal_InitChanStruct(Adc_ChanConfigType *const Config)
{
    DEVICE_ASSERT(Config != NULL_PTR);

    static const Adc_ChanConfigType DefaultAdcChanStruct =
    {
        .Spt = ADC_SPT_CLK_MAX,
        .Channel = ADC_CH_DISABLE,
        .InterruptEn = FALSE,
        .SeqIndex = ADC_ISEQ_MAX,
        .InternalChannelSrc = ADC_INTL_CH_SRC_MAX
    };

    if (Config != NULL_PTR)
    {
        *Config = DefaultAdcChanStruct;
    }
}

/* Implementation of Adc_Hal_ConfigChannel declared in Adc_Hal.h */
void Adc_Hal_ConfigChannel(uint8 Instance, const Adc_ChanConfigType *Config)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    DEVICE_ASSERT(Config->SeqIndex < ADC_ISEQ_MAX);//PRQA S 2812 # AMCE_ADC_2812
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842
    Adc_InputChannelType Channel = Config->Channel;

    Adc_Reg_SetChannelSampleTime(Base, Channel, Config->Spt);
    if ((uint32)Config->SeqIndex < (uint32)ADC_ISEQ_0)
    {
        Adc_Reg_SetRegularConversionChannel(Base, Config->SeqIndex, Channel);
        Adc_Reg_SetRegularEOCInterruptEnableFlag(Base, Config->SeqIndex, Config->InterruptEn);
    }
    else
    {
        Adc_Reg_SetInjectConversionChannel(Base, Config->SeqIndex, Channel);
        Adc_Reg_SetInjectEOCInterruptEnableFlag(Base, Config->SeqIndex, Config->InterruptEn);
    }
#if defined (AC7840X) || defined (AC7843X)
    if (ADC_CH_SUPPLY == Channel)
    {
        Adc_Reg_SetSupplyChannel(Base, Config->InternalChannelSrc);
#if defined (AC7843X)
        if( ADC_INTL_CH_SRC_OSC_LDO == Config->InternalChannelSrc)
        {
            Adc_Reg_SetAMUXXoscOutputEnable(TRUE);
        }
#endif
    }
#if defined (AC7843X)
    else if ((ADC_CH_BANDGAP == Channel) || (ADC_CH_TSENSOR == Channel))
    {
        Adc_SetInternalChannelAna(Instance, Channel);
    }
    else
    {
    /* do nothing*/
    }
#endif
#elif defined (AC7842X)
    if (ADC_CH_INTERNAL == Channel)
    {
         Adc_SetInternalChannelAna(Config->InternalChannelSrc);
    }
#elif defined (AC7840E)
    if (ADC_CH_SUPPLY == Channel)
    {
        Adc_Reg_SetSupplyChannel(Base, Config->InternalChannelSrc);
    }

    if (ADC_CH_BG_TS == Channel)
    {
         Adc_SetInternalChannelAna(Config->InternalChannelSrc);
    }
#endif
}

/* Implementation of Adc_Hal_ConfigRegularGroup declared in Adc_Hal.h */
void Adc_Hal_ConfigRegularGroup(uint32 Instance, const Adc_GroupConfigType* Config)
{
    ADC_Type * const Base = AdcBase[Instance];

    Adc_Reg_SetScanEnableFlag(Base, Config->ScanModeEn);
    Adc_Reg_SetContinuousEnableFlag(Base, Config->ContinuousModeEn);
    Adc_Reg_SetRegularTriggerSource(Base, Config->RegularTrigger);
    Adc_Reg_SetRegularLength(Base, Config->RegularSequenceLength);
}

/* Implementation of Adc_Hal_ConfigInjectGroup declared in Adc_Hal.h */
void Adc_Hal_ConfigInjectGroup(uint32 Instance, const Adc_GroupConfigType* Config)
{
    ADC_Type * const Base = AdcBase[Instance];

    Adc_Reg_SetScanEnableFlag(Base, Config->ScanModeEn);
    Adc_Reg_SetIntervalEnableFlag(Base, Config->IntervalModeEn);
    Adc_Reg_SetInjectTriggerSource(Base, Config->InjectTrigger);
    Adc_Reg_SetInjectLength(Base, Config->InjectSequenceLength);
}

/* Implementation of Adc_Hal_DmaEnable declared in Adc_Hal.h */
void Adc_Hal_DmaEnable(uint32 Instance, boolean Enable)
{
    ADC_Type * const Base = AdcBase[Instance];
    /* Set DMA enable flag */
    Adc_Reg_SetDMAEnableFlag(Base, Enable);
}

/* Implementation of Adc_Hal_SwTriggerRegularConvert declared in Adc_Hal.h */
void Adc_Hal_SwTriggerRegularConvert(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842

    Adc_Reg_SoftwareStartRegularConvert(Base);
}

/* Implementation of Adc_Hal_SwTriggerInjectConvert declared in Adc_Hal.h */
void Adc_Hal_SwTriggerInjectConvert(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842

    Adc_Reg_SoftwareStartInjectConvert(Base);
}

/* Implementation of Adc_Hal_GetSeqResult declared in Adc_Hal.h */
uint16 Adc_Hal_GetSeqResult(uint8 Instance, Adc_SequenceType SeqIndex)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(SeqIndex < ADC_ISEQ_MAX);
    uint16 result = 0;
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842

    if ((uint32)SeqIndex < (uint32)ADC_ISEQ_0)
    {
        result = Adc_Reg_GetRegularData(Base, SeqIndex);
    }
    else
    {
        result = Adc_Reg_GetInjectData(Base, SeqIndex);
    }

    return result;
}

#ifndef ADC_SDK_NON_EXTENDED_API
/* Implementation of Adc_Hal_InitGroupStruct declared in Adc_Hal.h */
void Adc_Hal_InitGroupStruct(Adc_GroupConfigType *const Config)
{
    DEVICE_ASSERT(Config != NULL_PTR);

    static const Adc_GroupConfigType DefaultAdcGroupStruct =
    {
        .DmaDstAddr = 0U,
        .RegularSequenceLength = 0U,
        .RegularTrigger = ADC_TRIGG_SRC_SW,
        .InjectSequenceLength = 0U,
        .InjectTrigger = ADC_TRIGG_SRC_SW,
        /* Conversion mode */
        .ScanModeEn = FALSE,
        .ContinuousModeEn = FALSE,
        .RegularDiscontinuousModeEn = FALSE,
        .InjectDiscontinuousModeEn = FALSE,
        .InjectAutoModeEn = FALSE,
        .IntervalModeEn = FALSE,
        .RegularDiscontinuousNum = 0U,
        .DmaCallback = NULL_PTR,
        .DmaArgs = NULL_PTR
    };

    if (Config != NULL_PTR)
    {
        *Config = DefaultAdcGroupStruct;
    }
}

/* Implementation of Adc_Hal_ConfigGroup declared in Adc_Hal.h */
void Adc_Hal_ConfigGroup(uint8 Instance, const Adc_GroupConfigType *Config)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842
    /* Set adc mode */
    if (Config != NULL_PTR)
    {
        Adc_Reg_SetScanEnableFlag(Base, Config->ScanModeEn);
        Adc_Reg_SetContinuousEnableFlag(Base, Config->ContinuousModeEn);
        Adc_Reg_SetRegularDiscontinuousEnableFlag(Base, Config->RegularDiscontinuousModeEn);
        Adc_Reg_SetInjectDiscontinuousEnableFlag(Base, Config->InjectDiscontinuousModeEn);
        Adc_Reg_SetInjectAutoEnableFlag(Base, Config->InjectAutoModeEn);
        Adc_Reg_SetIntervalEnableFlag(Base, Config->IntervalModeEn);
        Adc_Reg_SetRegularDiscontinuousNum(Base, Config->RegularDiscontinuousNum);

        /* Set adc regular*/
        Adc_Reg_SetRegularTriggerSource(Base, Config->RegularTrigger);
        Adc_Reg_SetRegularLength(Base, Config->RegularSequenceLength);

        /* Set adc inject*/
        Adc_Reg_SetInjectTriggerSource(Base, Config->InjectTrigger);
        Adc_Reg_SetInjectLength(Base, Config->InjectSequenceLength);
    }
}

/* Implementation of Adc_Hal_InitAMOStruct declared in Adc_Hal.h */
void Adc_Hal_InitAMOStruct(Adc_AmoConfigType *const Config)
{
    DEVICE_ASSERT(Config != NULL_PTR);

    static const Adc_AmoConfigType DefaultAdcAMOStruct =
    {
        .AmoTriggerMode = ADC_AMO_TRIGGER_LEVEL,
        .AmoInterruptEn = FALSE,
        .AmoRegularEn = FALSE,
        .AmoInjectEn = FALSE,
        .AmoSingleModeEn = FALSE,
        .AmoSingleChannel = ADC_CH_0,
        .AmoUpThreshold = 1U,
        .AmoLowThreshold = 0U,
        .AmoUpOffset = 0U,
        .AmoLowOffset = 0U
    };

    if (Config != NULL_PTR)
    {
        *Config = DefaultAdcAMOStruct;
    }
}

/* Implementation of Adc_Hal_ConfigAmo declared in Adc_Hal.h */
void Adc_Hal_ConfigAmo(uint8 Instance, const Adc_AmoConfigType *Config)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842

    if (Config != NULL_PTR)
    {
        DEVICE_ASSERT(Config->AmoUpThreshold > Config->AmoLowThreshold);
        Adc_Reg_SetAMOTriggerMode(Base, Config->AmoTriggerMode);
        Adc_Reg_SetAMORegularMode(Base, Config->AmoRegularEn);
        Adc_Reg_SetAMOInjectMode(Base, Config->AmoInjectEn);
        Adc_Reg_SetAMOSingleChannelMode(Base, Config->AmoSingleModeEn);
        Adc_Reg_SetAMOSingleChannel(Base, Config->AmoSingleChannel);

        Adc_Reg_SetAMOThreshold(Base, Config->AmoUpThreshold, Config->AmoLowThreshold);
        Adc_Reg_SetAMOOffset(Base, Config->AmoUpOffset, Config->AmoLowOffset);
        Adc_Reg_SetAMOInterrupt(Base, Config->AmoInterruptEn);
    }
}

/* Implementation of Adc_Hal_GetIdleFlag declared in Adc_Hal.h */
boolean Adc_Hal_GetIdleFlag(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842
    return (0U != (Adc_Reg_GetSTRFlag(Base) & ADC_STR_IDLE_Msk)) ? TRUE : FALSE;
}

/* Implementation of Adc_Hal_GetConvertCompleteFlag declared in Adc_Hal.h */
boolean Adc_Hal_GetConvertCompleteFlag(uint8 Instance, Adc_SequenceType SeqIndex)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(SeqIndex < ADC_ISEQ_MAX);
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842
    boolean Status = FALSE;

    if ((uint32)SeqIndex < (uint32)ADC_ISEQ_0)
    {
        Status = Adc_Reg_GetRegularEOCFlag(Base, SeqIndex);
    }
    else
    {
        Status = Adc_Reg_GetInjectEOCFlag(Base, SeqIndex);
    }

    return Status;
}

/* Implementation of Adc_Hal_ClearConvertCompleteFlag declared in Adc_Hal.h */
void Adc_Hal_ClearConvertCompleteFlag(uint8 Instance, Adc_SequenceType SeqIndex)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(SeqIndex < ADC_ISEQ_MAX);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842

    if ((uint32)SeqIndex < (uint32)ADC_ISEQ_0)
    {
        Adc_Reg_ClearRegularEOCFlag(Base, SeqIndex);
    }
    else
    {
        Adc_Reg_ClearInjectEOCFlag(Base, SeqIndex);
    }
}

/* Implementation of Adc_Hal_GetTriggerConflictFlag declared in Adc_Hal.h */
boolean Adc_Hal_GetTriggerConflictFlag(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842

    return (0U != (Adc_Reg_GetSTRFlag(Base) & ADC_STR_COVCFT_Msk)) ? TRUE : FALSE;
}

/* Implementation of Adc_Hal_ClearTriggerConflictFlag declared in Adc_Hal.h */
void Adc_Hal_ClearTriggerConflictFlag(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842

    Adc_Reg_ClearSTRFlag(Base, ADC_STR_COVCFT_Msk);
}

/* Implementation of Adc_Hal_GetParityVal declared in Adc_Hal.h */
uint8 Adc_Hal_GetParityVal(uint8 Instance, Adc_SequenceType SeqIndex)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT(SeqIndex < ADC_ISEQ_MAX);
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842
    uint8 ret = 0U;

    if ((uint32)SeqIndex < (uint32)ADC_ISEQ_0)
    {
        ret = Adc_Reg_GetRegularParityVal(Base, SeqIndex);
    }
    else
    {
        ret = Adc_Reg_GetInjectParityVal(Base, SeqIndex);
    }
    return ret;
}

/* Implementation of Adc_Hal_ConvertToTemperature declared in Adc_Hal.h */
float32 Adc_Hal_ConvertToTemperature(const uint16 Value, const float32 RefVoltage)
{
    float32 Temperature = 0.0f;
#if defined (AC7840E)
    float32 Slop = 1.698f, VolTempBase = 711.5f, TempBase = 25.0f;
#elif defined (AC7843X)
    float32 Slop = 1.704f, VolTempBase = 710.0f, TempBase = 25.0f;
#else
    float32 Slop = 1.788f, VolTempBase = 673.0f, TempBase = 25.0f;
#endif
    Temperature = ((VolTempBase - ((float32)Value / 4096.0f * RefVoltage)) / Slop) + TempBase;

    return Temperature;
}

/* Implementation of Adc_Hal_SetInjectOffset declared in Adc_Hal.h */
void Adc_Hal_SetInjectOffset(uint8 Instance, Adc_SequenceType SeqIndex, uint16 Offset)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    DEVICE_ASSERT((SeqIndex > ADC_RSEQ_31) && (SeqIndex < ADC_ISEQ_MAX));
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842

    Adc_Reg_SetInjectOffset(Base, (uint8)SeqIndex - (uint8)ADC_ISEQ_0, Offset);
}

/* Implementation of Adc_Hal_GetBase declared in Adc_Hal.h */
ADC_Type *Adc_Hal_GetBase(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    return AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842
}

/* Implementation of Adc_Hal_SelfTest declared in Adc_Hal.h */
boolean Adc_Hal_SelfTest(uint8 Instance, uint16 VrefMv)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842

    /* Internal self-test parameters */
    const uint16 BandgapMv    = (uint16)1200U;   /* Nominal Bandgap reference voltage in mV */
    const uint16 AdcMaxVal    = (uint16)4095U;   /* 12-bit ADC full-scale value */
    const uint16 TolerancePct = (uint16)10U;     /* Bandgap tolerance: +/- 10% */
    const uint32 ConvTimeout  = (uint32)10000U;  /* Max polling iterations for EOC */
    uint32 Timeout = 0U;
    uint16 AdcRaw    = 0U;
    uint32 VoltageMv = 0U;
    uint16 LowerLimitMv = 0U;
    uint16 UpperLimitMv = 0U;
    boolean TestResult = FALSE;

    /* ------------------------------------------------------------------
     * Step 0: Enable ADC bus clock and perform peripheral reset,
     *         mirroring the clock-init sequence in Adc_Hal_Init.
     * ------------------------------------------------------------------ */
    /*PRQA S 2842 ++ # AMCE_ADC_2842.*/
    (void)Ckgen_Hal_EnablePeriphClk(AdcCkgenBusClock[Instance], TRUE);
    Rcm_Hal_SetResetState(AdcClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(AdcClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    /*PRQA S 2842 -- # AMCE_ADC_2842.*/

    /* ------------------------------------------------------------------
     * Step 1: Configure converter registers for Bandgap measurement.
     *   - VDDA as voltage reference, clock /8, right-aligned, no DMA, no HW average
     * ------------------------------------------------------------------ */
    Adc_Reg_SetClockPrescaler(Base, ADC_CLK_DIVIDE_8);
#ifndef AC7840E
    Adc_Reg_SetResolution(Base, ADC_RESOLUTION_12BIT);
#endif
    Adc_Reg_SetDataAlign(Base, ADC_ALIGN_RIGHT);
    Adc_Reg_SetVoltageReference(Base, ADC_VOLTAGEREF_VDDA);
    Adc_Reg_SetDMAEnableFlag(Base, FALSE);
    Adc_Reg_SetAverageEnableFlag(Base, FALSE);
    Adc_Reg_SetCalibrationEnableFlag(Base, FALSE);
    Adc_Reg_SetPowerEnableFlag(Base, TRUE);
#ifndef AC7840E
    OsIf_UDelay(100U); /* Wait for ADC power-up stabilization */
#else
    while (FALSE == Adc_Reg_GetLdoFlag(Base))
    {
        /* Wait for LDO ready (AC7840E only) */
    }
#endif

    /* ------------------------------------------------------------------
     * Step 2: Configure regular group registers: single SW-triggered conversion.
     * ------------------------------------------------------------------ */
    Adc_Reg_SetScanEnableFlag(Base, FALSE);
    Adc_Reg_SetContinuousEnableFlag(Base, FALSE);
    Adc_Reg_SetRegularTriggerSource(Base, ADC_TRIGG_SRC_SW);
    Adc_Reg_SetRegularLength(Base, 1U);

    /* ------------------------------------------------------------------
     * Step 3: Configure the platform-specific Bandgap input channel.
     *   Set sample time, regular sequence slot, and AMUX routing.
     * ------------------------------------------------------------------ */
#if defined (AC7840X)
    Adc_Reg_SetChannelSampleTime(Base, ADC_CH_SUPPLY, ADC_SPT_CLK_185);
    Adc_Reg_SetRegularConversionChannel(Base, ADC_RSEQ_0, ADC_CH_SUPPLY);
    Adc_Reg_SetRegularEOCInterruptEnableFlag(Base, ADC_RSEQ_0, FALSE);
    Adc_Reg_SetSupplyChannel(Base, ADC_INTL_CH_SRC_REFH);
#elif defined (AC7843X)
    Adc_Reg_SetChannelSampleTime(Base, ADC_CH_BANDGAP, ADC_SPT_CLK_185);
    Adc_Reg_SetRegularConversionChannel(Base, ADC_RSEQ_0, ADC_CH_BANDGAP);
    Adc_Reg_SetRegularEOCInterruptEnableFlag(Base, ADC_RSEQ_0, FALSE);
    Adc_SetInternalChannelAna(Instance, ADC_CH_BANDGAP);
#elif defined (AC7842X)
    Adc_Reg_SetChannelSampleTime(Base, ADC_CH_INTERNAL, ADC_SPT_CLK_185);
    Adc_Reg_SetRegularConversionChannel(Base, ADC_RSEQ_0, ADC_CH_INTERNAL);
    Adc_Reg_SetRegularEOCInterruptEnableFlag(Base, ADC_RSEQ_0, FALSE);
    Adc_SetInternalChannelAna(ADC_INTL_CH_SRC_BG0);
#elif defined (AC7840E)
    Adc_Reg_SetChannelSampleTime(Base, ADC_CH_BG_TS, ADC_SPT_CLK_185);
    Adc_Reg_SetRegularConversionChannel(Base, ADC_RSEQ_0, ADC_CH_BG_TS);
    Adc_Reg_SetRegularEOCInterruptEnableFlag(Base, ADC_RSEQ_0, FALSE);
    Adc_SetInternalChannelAna(ADC_INTL_CH_SRC_AMUX_BG);
#endif

    /* ------------------------------------------------------------------
     * Step 4: Trigger software conversion and wait for EOC.
     * ------------------------------------------------------------------ */
    Adc_Reg_ClearRegularEOCFlag(Base, ADC_RSEQ_0);
    Adc_Reg_SoftwareStartRegularConvert(Base);

    Timeout = ConvTimeout;
    while (FALSE == Adc_Reg_GetRegularEOCFlag(Base, ADC_RSEQ_0))
    {
        Timeout--;
        if (0U == Timeout)
        {
            break;
        }
    }

    if (Timeout > 0U)
    {
        /* Read raw conversion result */
        AdcRaw = Adc_Reg_GetRegularData(Base, ADC_RSEQ_0);
        Adc_Reg_ClearRegularEOCFlag(Base, ADC_RSEQ_0);

        /* ------------------------------------------------------------------
         * Step 5: Convert raw ADC value to mV and verify Bandgap tolerance.
         *   voltage_mV = (raw * VREF_mV) / ADC_MAX
         *   Expected:  1200 mV +/- 10%  => [1080 mV, 1320 mV]
         * ------------------------------------------------------------------ */
        VoltageMv = ((uint32)AdcRaw * (uint32)VrefMv) / (uint32)AdcMaxVal;

        LowerLimitMv = BandgapMv - (uint16)((uint32)BandgapMv * (uint32)TolerancePct / 100U);
        UpperLimitMv = BandgapMv + (uint16)((uint32)BandgapMv * (uint32)TolerancePct / 100U);

        if (((uint16)VoltageMv >= LowerLimitMv) && ((uint16)VoltageMv <= UpperLimitMv))
        {
            TestResult = TRUE;
        }
    }

    /* ------------------------------------------------------------------
     * Step 6: Disable ADC bus clock - this resets all ADC registers to
     *         their default values, no manual register restore needed.
     * ------------------------------------------------------------------ */
    Adc_Reg_SetPowerEnableFlag(Base, FALSE);
    /*PRQA S 2842 ++ # AMCE_ADC_2842.*/
    (void)Ckgen_Hal_EnablePeriphClk(AdcCkgenBusClock[Instance], FALSE);
    /*PRQA S 2842 -- # AMCE_ADC_2842.*/

    return TestResult;
}

static void Adc_SetInterleave(uint8 Instance, Adc_InterleaveType Interleave)
{
#if defined (AC7840X)
    if (0U == Instance)
    {
        if (ADC_INTERLEAVE_DISABLE == Interleave)
        {
            /* turn off RG_MODE */
            Adc_Reg_SetAdc1Interleave(FALSE);
        }
        else
        {
            Ctu_Reg_SetAdc0Inter((uint8)Interleave);
            /* enable ADC1 RG_MODE for ADC1 interleave */
            Adc_Reg_SetAdc1Interleave(TRUE);
            if (ADC_INTERLEAVE_0 == Interleave)
            {
                Adc_Reg_SetAdc1InterleaveChannel(8);
            }
            else
            {
                Adc_Reg_SetAdc1InterleaveChannel(9);
            }
        }
    }
    else
    {
        if (ADC_INTERLEAVE_DISABLE == Interleave)
        {
            /* turn off RG_MODE */
            Adc_Reg_SetAdc0Interleave(FALSE);
        }
        else
        {
            Ctu_Reg_SetAdc1Inter((uint8)Interleave);
            /* enable ADC1 RG_MODE for ADC1 interleave */
            Adc_Reg_SetAdc0Interleave(TRUE);
            if (ADC_INTERLEAVE_0 == Interleave)
            {
                Adc_Reg_SetAdc0InterleaveChannel(4);
            }
            else
            {
                Adc_Reg_SetAdc0InterleaveChannel(5);
            }
        }
    }
#elif defined (AC7842X)  || defined (AC7843X)
    if (ADC_INTERLEAVE_DISABLE != Interleave)
    {
        if (0U == Instance)
        {
           Ctu_Reg_SetAdc0Inter((uint32)Interleave);
        }
        else
        {
           Ctu_Reg_SetAdc1Inter((uint32)Interleave);
        }
    }
#else
    (void)Instance;
    (void)Interleave;
#endif
}
#endif

#if defined (AC7843X)
static void Adc_SetInternalChannelAna(uint32 Instance, Adc_InputChannelType Channel)
{
    Adc_Reg_SetAMUXBufEnable(TRUE);
    Adc_Reg_SetAMUXOutputEnable(TRUE);
    if (ADC_CH_BANDGAP == Channel)
    {
        if(1U == Instance)
        {
            Adc_Reg_SetBG1OutputValue((uint8)ANA_OUTPUT_SELECT_BG1);
            Adc_Reg_SetAmuxInternalChannel(ANA_CHANNEL_AMUX_BG1);
        }
        else
        {
            Adc_Reg_SetBG0OutputValue((uint8)ANA_OUTPUT_SELECT_BG0);
            Adc_Reg_SetAmuxInternalChannel(ANA_CHANNEL_AMUX_BG0);
        }
    }
    else if (ADC_CH_TSENSOR == Channel)
    {
        if(1U == Instance)
        {
            Adc_Reg_SetTsensor1OutputValue((uint8)ANA_OUTPUT_SELECT_TSENSOR_ON);
            Adc_Reg_SetAmuxInternalChannel(ANA_CHANNEL_AMUX_TSENSOR1);
        }
        else
        {
            Adc_Reg_SetTsensor0OutputValue((uint8)ANA_OUTPUT_SELECT_TSENSOR_ON);
        }
        Adc_Reg_SetAmuxInternalChannel(ANA_CHANNEL_AMUX_TSENSOR0);
    }
    else
    {
        /*do nothing*/
    }
}
#elif defined (AC7842X)
static void Adc_SetInternalChannelAna(Adc_InternalChannelSrcType InternalChannelSrc)
{
    if ((ADC_INTL_CH_SRC_VDDA == InternalChannelSrc) || (ADC_INTL_CH_SRC_VDD == InternalChannelSrc))
    {
        Adc_Reg_SetAMUXBufEnable(FALSE);
        Adc_Reg_SetAMUXBufBypassEnable(TRUE);
    }
    else
    {
        Adc_Reg_SetAMUXBufEnable(TRUE);
        Adc_Reg_SetAMUXBufBypassEnable(FALSE);
        if (ADC_INTL_CH_SRC_DIGLDO_SUPPLY == InternalChannelSrc)
        {
            Adc_Reg_SetDigitalLDOOutputEnable(TRUE);
            Adc_Reg_SetDigitalLDOEnable(TRUE);
        }
        else if (ADC_INTL_CH_SRC_FLHLDO_SUPPLY == InternalChannelSrc)
        {
            Adc_Reg_SetFlashLDOOutputEnable(TRUE);
        }
        else if (ADC_INTL_CH_SRC_BG0 == InternalChannelSrc)
        {
            Adc_Reg_SetBG0OutputValue((uint8)ANA_OUTPUT_SELECT_BG0);
        }
        else if (ADC_INTL_CH_SRC_BG1 == InternalChannelSrc)
        {
            Adc_Reg_SetBG1OutputValue((uint8)ANA_OUTPUT_SELECT_BG1);
        }
        else if (ADC_INTL_CH_SRC_TSENSOR0 == InternalChannelSrc)
        {
            Adc_Reg_SetTsensor0OutputValue((uint8)ANA_OUTPUT_SELECT_TSENSOR_ON);
        }
        else if (ADC_INTL_CH_SRC_TSENSOR1 == InternalChannelSrc)
        {
             Adc_Reg_SetTsensor1OutputValue((uint8)ANA_OUTPUT_SELECT_TSENSOR_ON);
        }
        else if (ADC_INTL_CH_SRC_XOSC == InternalChannelSrc)
        {
             Adc_Reg_SetAMUXXoscOutputEnable(TRUE);
        }
        else
        {
            /* Do nothing */
        }
    }
    Adc_Reg_SetAMUXOutputEnable(TRUE);
    Adc_Reg_SetAmuxInternalChannel(InternalChannelSrc);
}
#elif defined (AC7840E)
static void Adc_SetInternalChannelAna(Adc_InternalChannelSrcType InternalChannelSrc)
{
    Adc_Reg_SetAMUXBufEnable(TRUE);
    Adc_Reg_SetAMUXBufBypassEnable(FALSE);
    if (ADC_INTL_CH_SRC_AMUX_BG == InternalChannelSrc)
    {
        Adc_Reg_SetBGOutputValue((uint8)ANA_OUTPUT_SELECT_BG);
    }
    else if (ADC_INTL_CH_SRC_AMUX_TSENSOR == InternalChannelSrc)
    {
        Adc_Reg_SetTsensor0OutputValue((uint8)ANA_OUTPUT_SELECT_TSENSOR_ON);
    }
    else
    {
        /* Do nothing */
    }
    Adc_Reg_SetAMUXOutputEnable(TRUE);
    Adc_Reg_SetAmuxInternalChannel(InternalChannelSrc);
}
#endif

static void Adc_DmaDeinit(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    const ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842
    uint8 ChannelId = 0;

    /*PRQA S 4394 ++ # AMCE_ADC_4394.*/
    /*PRQA S 4342 ++ # AMCE_ADC_4342.*/
    ChannelId = Dma_Hal_GetChIdByReqSrc((Dma_RequestSourceType)((uint32)DMA_REQ_ADC0 + Instance));
    /*PRQA S 4342 -- # AMCE_ADC_4342.*/
    /*PRQA S 4394 -- # AMCE_ADC_4394.*/
    if (TRUE == Adc_Reg_GetDMAEnableFlag(Base))
    {
        (void)Dma_Hal_StopCh(ChannelId);
        (void)Dma_Hal_EnableChIrq(ChannelId, (uint8)DMA_IRQ_NONE);
    }
}

static void Adc_CommonISR(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ADC_INSTANCE_MAX);
    ADC_Type *const Base = AdcBase[Instance];//PRQA S 2842 # AMCE_ADC_2842
    Adc_InterruptInfoType Info = {0U};
    uint32 Event;
    const uint32 AmoMask = ADC_STR_AAMO_Msk | ADC_STR_NAMO_Msk | ADC_STR_AMO_Msk;
    /* take snapshot of current regular & injection group length */
    uint32 RegularLen = Adc_Reg_GetRegularLength(Base);
    uint32 InjectLen = Adc_Reg_GetInjectLength(Base);
    uint32 i;
    Adc_SequenceType Seq;

    /* Restrict RegularLen form above ADC_REGULAR_SEQ_NUM. No need for injection group for now */
    if (RegularLen > ADC_REGULAR_SEQ_NUM)
    {
        RegularLen = ADC_REGULAR_SEQ_NUM;
    }
    /* store device Status */
    Event = Adc_Reg_GetSTRFlag(Base);
    Event &= AmoMask;
    /* clear device Status, and clear AMO State only */
    Adc_Reg_ClearSTRFlag(Base, AmoMask);
    Info.Instance = Instance;
    /* call AMO interrupt handler */
    if ((Event > 0U) && (NULL_PTR != AdcISR[Instance].Callback))//PRQA S 2842 # AMCE_ADC_2842
    {
        Info.Event = Event;
        /* Callback */
        AdcISR[Instance].Callback(&Info);//PRQA S 2842 # AMCE_ADC_2842
    }
    /* Process EOC interrupt */
    Event = 0U;
    for (i = 0U; i < InjectLen; i++)
    {
        /*PRQA S 4394 ++ # AMCE_ADC_4394.*/
        /*PRQA S 4342 ++ # AMCE_ADC_4342.*/
        Seq = (Adc_SequenceType)((uint32)ADC_ISEQ_0 + i);
        /*PRQA S 4394 -- # AMCE_ADC_4342.*/
        /*PRQA S 4342 -- # AMCE_ADC_4394.*/
        boolean IEOCCtrlFlag, IEOCFlag;
        IEOCCtrlFlag = Adc_Reg_GetInjectEOCInterruptEnableFlag(Base, Seq);
        IEOCFlag = Adc_Reg_GetInjectEOCFlag(Base, Seq);
        if (((boolean)TRUE == IEOCCtrlFlag) && ((boolean)TRUE == IEOCFlag))
        {
            /* clear EOC Status */
            Adc_Reg_ClearInjectEOCFlag(Base, Seq);
            /* save Event information */
            Info.sequence = Seq;
            Event = (uint32)ADC_EVENT_IEOC;
            break;
        }
    }
    /* call EOC interrupt handler */
    if (((uint32)ADC_EVENT_IEOC == Event)\
            && (NULL_PTR != AdcISR[Instance].Callback))//PRQA S 2842,2843 # AMCE_ADC_2842,AMCE_ADC_2843
    {
        Info.Event = Event;
        AdcISR[Instance].Callback(&Info);//PRQA S 2842,2843 # AMCE_ADC_2842,AMCE_ADC_2843
    }
    Event = 0U;
    for (i = 0U; i < RegularLen; i++)
    {
        Seq = (Adc_SequenceType)i;//PRQA S 4342  # AMCE_ADC_4342
        boolean EOCCtrlFlag, EOCFlag;
        EOCCtrlFlag = Adc_Reg_GetRegularEOCInterruptEnableFlag(Base, Seq);
        EOCFlag = Adc_Reg_GetRegularEOCFlag(Base, Seq);
        if (((boolean)TRUE == EOCCtrlFlag) && ((boolean)TRUE == EOCFlag))
        {
            /* clear EOC Status */
            Adc_Reg_ClearRegularEOCFlag(Base, Seq);
            /* save Event information */
            Info.sequence = Seq;
            Event = (uint32)ADC_EVENT_EOC;
            break;
        }
    }
    /* call EOC interrupt handler */
    if (((uint32)ADC_EVENT_EOC == Event)\
            && (NULL_PTR != AdcISR[Instance].Callback))//PRQA S 2842,2843 # AMCE_ADC_2842,AMCE_ADC_2843
    {
        Info.Event = Event;
        AdcISR[Instance].Callback(&Info);//PRQA S 2842,2843 # AMCE_ADC_2842,AMCE_ADC_2843
    }
}

/**
 * @brief ADC0 Irq.
 * @return void
 */
ISR(ADC0_IRQHandler)//PRQA S 1503,3408 # AMCE_QAC_NoUsedApi,AMCE_QAC_3408
{
    Adc_CommonISR(0U);
}

/**
 * @brief ADC1 Irq.
 * @return void
 */
ISR(ADC1_IRQHandler)//PRQA S 1503,3408 # AMCE_QAC_NoUsedApi,AMCE_QAC_3408
{
    Adc_CommonISR(1U);
}

/* =============================================  EOF  ============================================== */
