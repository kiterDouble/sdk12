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
 * @file Acmp_Hal.c
 *
 * @brief This file provides analog comparator module integration functions.
 *
 */
/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Acmp_Reg.h"
#include "Acmp_Hal.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/
/**
 * @brief Table of ACMP base addresses.
 * @details Contains pointers to hardware register base addresses for all ACMP instances.
 * @note DES ID: DES_ACMP_VAR_001
 */
static ACMP_Type *const Acmp_HalBase[ACMP_INSTANCE_MAX] =
{
    ACMP0,
#if defined (AC7843X)
    ACMP1
#endif
};

/**
 * @brief Table of ACMP interrupt request numbers.
 * @details Contains IRQ numbers for all ACMP instances used for interrupt management.
 * @note DES ID: DES_ACMP_VAR_002
 */
static const IRQn_Type Acmp_IrqId[ACMP_INSTANCE_MAX] = ACMP_IRQS;

/**
 * @brief Table of ACMP clock generator interface clocks.
 * @details Contains clock IDs for enabling/disabling peripheral clocks for all ACMP instances.
 * @note DES ID: DES_ACMP_VAR_003
 */
static const Ckgen_BusClkIdType Acmp_HalBusClock[ACMP_INSTANCE_MAX] =
{
    CKGEN_ACMP0_BUS_CLK,
#if defined (AC7843X)
    CKGEN_ACMP1_BUS_CLK
#endif
};

/**
 * @brief Table of ACMP soft reset identifiers.
 * @details Contains reset IDs for performing soft reset operations on all ACMP instances.
 * @note DES ID: DES_ACMP_VAR_004
 */
static const Rcm_ResetIDType Acmp_HalClockReset[ACMP_INSTANCE_MAX] =
{
    RCM_RESET_ID_ACMP0,
#if defined (AC7843X)
    RCM_RESET_ID_ACMP1
#endif
};

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/**
 * @brief ACMP interrupt callback array.
 * @note DES ID: DES_ACMP_VAR_005
 */
static Acmp_CallbackType Acmp_HalCallback[ACMP_INSTANCE_MAX];

/*==========================================FUNCTION PROTOTYPES=====================================*/
ISR(ACMP0_IRQHandler);

#if defined (AC7843X)
ISR(ACMP1_IRQHandler);
#endif

/**
 * @brief Configures all ACMP comparator functions with the given configuration structure.
 *
 * @details
 * This static function applies the comparator configuration including interrupt enable,
 * output trigger mode, filter settings, hysteresis, and power control. It directly
 * writes to hardware registers based on the provided configuration.
 *
 * @param[in] Instance: ACMP instance number.
 * @param[in] Config: Pointer to ACMP comparator configuration structure.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     Config must point to a valid configuration structure.
 *
 * @post
 *     ACMP comparator hardware registers are configured.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     This is an internal static function.
 *     DES ID: DES_ACMP_API_050
 */
static void Acmp_Hal_ConfigComparator(uint8 Instance, const Acmp_ComparatorType *Config);

#ifndef ACMP_SDK_NON_EXTENDED_API
/**
 * @brief Gets the current ACMP comparator configuration.
 *
 * @details
 * This static function reads hardware registers and fills the provided configuration
 * structure with the current comparator settings including interrupt enable status,
 * output trigger mode, filter settings, hysteresis, and power control.
 *
 * @param[in] Instance: ACMP instance number.
 * @param[out] Config: Pointer to ACMP comparator configuration structure to be filled.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     Config must point to a valid configuration structure.
 *
 * @post
 *     Config structure contains the current hardware configuration.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This is an internal static function.
 *     DES ID: DES_ACMP_API_051
 */
static void Acmp_Hal_GetComparatorConfig(uint8 Instance, Acmp_ComparatorType *const Config);
#endif

/**
 * @brief Configures all ACMP internal DAC functions with the given configuration structure.
 *
 * @details
 * This static function applies the DAC configuration including voltage reference selection,
 * output level settings, and enable/disable control. The DAC can be used to provide a
 * reference voltage to the comparator.
 *
 * @param[in] Instance: ACMP instance number.
 * @param[in] Config: Pointer to ACMP DAC configuration structure.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     Config must point to a valid configuration structure.
 *
 * @post
 *     ACMP DAC hardware registers are configured.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     This is an internal static function.
 *     DES ID: DES_ACMP_API_052
 */
static void Acmp_Hal_ConfigDAC(uint8 Instance, const Acmp_DacType *Config);

#ifndef ACMP_SDK_NON_EXTENDED_API
/**
 * @brief Gets the current ACMP internal DAC configuration.
 *
 * @details
 * This static function reads hardware registers and fills the provided configuration
 * structure with the current DAC settings including voltage reference selection,
 * output level, and enable status.
 *
 * @param[in] Instance: ACMP instance number.
 * @param[out] Config: Pointer to ACMP DAC configuration structure to be filled.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     Config must point to a valid configuration structure.
 *
 * @post
 *     Config structure contains the current DAC hardware configuration.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This is an internal static function.
 *     DES ID: DES_ACMP_API_053
 */
static void Acmp_Hal_GetDACConfig(uint8 Instance, Acmp_DacType *const Config);
#endif

/**
 * @brief Configures ACMP analog input multiplexer.
 *
 * @details
 * This static function configures the positive and negative input pins for the
 * comparator through the analog multiplexer. This allows selection of different
 * analog input sources for comparison.
 *
 * @param[in] Instance: ACMP instance number.
 * @param[in] Config: Pointer to ACMP analog multiplexer configuration structure.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     Config must point to a valid configuration structure.
 *
 * @post
 *     ACMP analog multiplexer is configured.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     This is an internal static function.
 *     DES ID: DES_ACMP_API_054
 */
static void Acmp_Hal_ConfigMUX(uint8 Instance, const Acmp_AnmuxType *Config);

#ifndef ACMP_SDK_NON_EXTENDED_API
/**
 * @brief Gets the current ACMP analog input multiplexer configuration.
 *
 * @details
 * This static function reads hardware registers and retrieves the current selection
 * of positive and negative input pins for the comparator.
 *
 * @param[in] Instance: ACMP instance number.
 * @param[out] Config: Pointer to ACMP analog multiplexer configuration structure to be filled.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     Config must point to a valid configuration structure.
 *
 * @post
 *     Config structure contains the current multiplexer configuration.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This is an internal static function.
 *     DES ID: DES_ACMP_API_055
 */
static void Acmp_Hal_GetMUXConfig(uint8 Instance, Acmp_AnmuxType *const Config);
#endif

/**
 * @brief Sets the ACMP polling mode.
 *
 * @details
 * This static function enables or disables positive/negative input polling mode.
 * In polling mode, the comparator can automatically scan multiple input channels.
 *
 * @param[in] Base: Pointer to ACMP hardware base address.
 * @param[in] Polling: Polling mode selection (none, positive, or negative).
 *
 * @return void
 *
 * @pre
 *     Base must point to a valid ACMP hardware register address.
 *
 * @post
 *     ACMP polling mode is configured.
 *
 * @note
 *     Reentrancy: Not reentrant.
 *     This is an internal static function.
 *     DES ID: DES_ACMP_API_056
 */
static void Acmp_Hal_SetPollingMode(ACMP_Type *Base, const Acmp_InputPollingType Polling);

/**
 * @brief Configures ACMP polling mode with the given configuration structure.
 *
 * @details
 * This static function applies the polling mode configuration including polling mode
 * selection, clock divider, polling sequence, and Hall sensor output settings. Polling
 * mode allows automatic scanning of multiple input channels.
 *
 * @param[in] Instance: ACMP instance number.
 * @param[in] Config: Pointer to ACMP polling mode configuration structure.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     Config must point to a valid configuration structure.
 *
 * @post
 *     ACMP polling mode hardware registers are configured.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     This is an internal static function.
 *     DES ID: DES_ACMP_API_057
 */
static void Acmp_Hal_ConfigPollingMode(uint8 Instance, const Acmp_PollingModeType *Config);

#ifndef ACMP_SDK_NON_EXTENDED_API
/**
 * @brief Gets the current ACMP polling mode.
 *
 * @details
 * This static function reads the hardware registers to determine whether the ACMP is
 * using positive polling, negative polling, or no polling mode.
 *
 * @param[in] Base: Pointer to ACMP hardware base address.
 *
 * @return Acmp_InputPollingType: Current polling mode (none, positive, or negative).
 *
 * @pre
 *     Base must point to a valid ACMP hardware register address.
 *
 * @post
 *     None.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This is an internal static function.
 *     DES ID: DES_ACMP_API_058
 */
static Acmp_InputPollingType Acmp_Hal_GetPollingMode(const ACMP_Type *Base);

/**
 * @brief Gets the current ACMP polling mode configuration.
 *
 * @details
 * This static function reads hardware registers and fills the provided configuration
 * structure with the current polling mode settings including polling mode selection,
 * clock divider, polling sequence, and Hall sensor output configuration.
 *
 * @param[in] Instance: ACMP instance number.
 * @param[out] Config: Pointer to ACMP polling mode configuration structure to be filled.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     Config must point to a valid configuration structure.
 *
 * @post
 *     Config structure contains the current polling mode configuration.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This is an internal static function.
 *     DES ID: DES_ACMP_API_059
 */
static void Acmp_Hal_GetPollingModeConfig(uint8 Instance, Acmp_PollingModeType *const Config);
#endif

/**
 * @brief ACMP interrupt handler.
 *
 * @details
 * This static function is the common interrupt handler for all ACMP instances.
 * It reads the interrupt status, clears the interrupt flags, and calls the
 * user-registered callback function if configured.
 *
 * @param[in] Instance: ACMP instance number.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     ACMP interrupt must be enabled and triggered.
 *
 * @post
 *     Interrupt flags are cleared and callback function is invoked.
 *
 * @note
 *     Reentrancy: Not reentrant.
 *     This is an internal static function called from ISR context.
 *     DES ID: DES_ACMP_API_060
 */
static void Acmp_Hal_IRQHandler(uint8 Instance);

/*=================================GLOBAL FUNCTION IMPLEMENTATIONS=================================*/
#ifndef ACMP_SDK_NON_EXTENDED_API
/* Implementation of Acmp_Hal_GetDefaultConfig declared in Acmp_Hal.h */
void Acmp_Hal_GetDefaultConfig(Acmp_ModuleType *Config)
{
    DEVICE_ASSERT(Config != NULL_PTR);

    if (Config != NULL_PTR)
    {
        Config->Comparator.InterruptEn = FALSE;
        Config->Comparator.OutputTrigger = ACMP_BOTH_EDGES;
        Config->Comparator.OutputSelect = ACMP_COUT;
        Config->Comparator.FilterEnable = FALSE;
        Config->Comparator.FilterSampleCount = 0U;
        Config->Comparator.LpfBandwidth = ACMP_LPF_500KHZ;
        Config->Comparator.InverterEnable = FALSE;
        Config->Comparator.PinState = FALSE;
        Config->Comparator.HysteresisMode = ACMP_HYS_FALLING_EDGE;
        Config->Comparator.HysteresisLevel = ACMP_LEVEL_HYS_10MV;
#if !defined(AC7840E)
        Config->Comparator.UsingLSIEnable = FALSE;
#endif
        Config->Comparator.WindowModeEnable = FALSE;
        Config->Comparator.ClockDivide = ACMP_FLT_DIVIDE_1;
        Config->Comparator.Callback = NULL_PTR;
        Config->Comparator.PowerEnable = TRUE;

        Config->Mux.PositiveInputMux = ACMP_EXTERNAL_CH1;
        Config->Mux.NegativeInputMux = ACMP_DAC_OUTPUT;

        Config->Dac.VoltageReferenceSource = ACMP_DAC_VDD;
        Config->Dac.Voltage = (0xFFU >> 1U);
        Config->Dac.State = TRUE;
#if defined (AC7842X) || defined (AC7843X)
        Config->Dac.OutToPin = FALSE;
        Config->Dac.BufferEnable = FALSE;
#endif

        Config->PollingMode.Mode = ACMP_NONE_POLLING;
        Config->PollingMode.PollingClockDivide = ACMP_CLK_DIVIDE_256;
        Config->PollingMode.PollingSequence = 0U;
        Config->PollingMode.HallOutputEnable = 0U;
        Config->PollingMode.HallAOutputCh = ACMP_EXTERNAL_CH0;
        Config->PollingMode.HallBOutputCh = ACMP_EXTERNAL_CH0;
        Config->PollingMode.HallCOutputCh = ACMP_EXTERNAL_CH0;
    }
}

/* Implementation of Acmp_Hal_Reset declared in Acmp_Hal.h */
void Acmp_Hal_Reset(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    IRQn_Type Irq = Acmp_IrqId[Instance];

    /*!< Disable ACMP interrupt */
    Core_Hal_DisableIrq(Irq);
    Core_Hal_ClearPendingIrq(Irq);

    Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    /*PRQA S 2842 -- */
}
#endif

/* Implementation of Acmp_Hal_Init declared in Acmp_Hal.h */
void Acmp_Hal_Init(uint8 Instance, const Acmp_ModuleType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    if (Config != NULL_PTR)
    {
        /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
        (void)Ckgen_Hal_EnablePeriphClk(Acmp_HalBusClock[Instance], TRUE);
        Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
        /*PRQA S 2842 -- */

        Acmp_Hal_ConfigComparator(Instance, &Config->Comparator);
        Acmp_Hal_ConfigDAC(Instance, &Config->Dac);
        Acmp_Hal_ConfigMUX(Instance, &Config->Mux);
        Acmp_Hal_ConfigPollingMode(Instance, &Config->PollingMode);
    }
}

/* Implementation of Acmp_Hal_Enable declared in Acmp_Hal.h */
void Acmp_Hal_Enable(uint8 Instance, boolean Enable)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    Acmp_Reg_SetEnableFlag(Base, Enable);
}

/* Implementation of Acmp_Hal_Deinit declared in Acmp_Hal.h */
void Acmp_Hal_Deinit(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    IRQn_Type Irq = Acmp_IrqId[Instance];

    /*!< Disable ACMP interrupt */
    Core_Hal_DisableIrq(Irq);

    Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(Acmp_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    (void)Ckgen_Hal_EnablePeriphClk(Acmp_HalBusClock[Instance], FALSE);

    Core_Hal_ClearPendingIrq(Irq);
    /*PRQA S 2842 -- */
}

#ifndef ACMP_SDK_NON_EXTENDED_API
/* Implementation of Acmp_Hal_GetConfigAll declared in Acmp_Hal.h */
void Acmp_Hal_GetConfigAll(uint8 Instance, Acmp_ModuleType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    if (Config != NULL_PTR)
    {
        Acmp_Hal_GetComparatorConfig(Instance, &Config->Comparator);
        Acmp_Hal_GetDACConfig(Instance, &Config->Dac);
        Acmp_Hal_GetMUXConfig(Instance, &Config->Mux);
        Acmp_Hal_GetPollingModeConfig(Instance, &Config->PollingMode);
    }
}
#endif

/* Implementation of Acmp_Hal_GetOutputData declared in Acmp_Hal.h */
uint8 Acmp_Hal_GetOutputData(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    return Acmp_Reg_GetOutputData(Base);
}

#ifndef ACMP_SDK_NON_EXTENDED_API
/* Implementation of Acmp_Hal_GetOutputFlags declared in Acmp_Hal.h */
uint8 Acmp_Hal_GetOutputFlags(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    return Acmp_Reg_GetOutputStatus(Base);
}

/* Implementation of Acmp_Hal_ClearOutputFlags declared in Acmp_Hal.h */
void Acmp_Hal_ClearOutputFlags(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    Acmp_Reg_ClearOutputStatus(Base);
}
#endif

/* Implementation of Acmp_Hal_GetPollingData declared in Acmp_Hal.h */
uint16 Acmp_Hal_GetPollingData(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    return Acmp_Reg_GetPollingData(Base);
}

#ifndef ACMP_SDK_NON_EXTENDED_API
/* Implementation of Acmp_Hal_GetPollingFlags declared in Acmp_Hal.h */
uint16 Acmp_Hal_GetPollingFlags(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    return Acmp_Reg_GetPollingStatus(Base);
}

/* Implementation of Acmp_Hal_ClearInputFlags declared in Acmp_Hal.h */
void Acmp_Hal_ClearInputFlags(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    Acmp_Reg_ClearPollingStatus(Base);
}
#endif

/**
 * @brief ACMP0 interrupt service routine.
 *
 * @details
 * This ISR is triggered when ACMP0 generates an interrupt. It calls the common
 * interrupt handler Acmp_Hal_IRQHandler with instance 0.
 *
 * @param void
 *
 * @return void
 *
 * @pre
 *     ACMP0 interrupt must be enabled.
 *
 * @post
 *     ACMP0 interrupt is serviced.
 *
 * @note
 *     Reentrancy: Not reentrant.
 *     DES ID: DES_ACMP_API_010
 */
ISR(ACMP0_IRQHandler)
{
    Acmp_Hal_IRQHandler(0U);
}

#if defined (AC7843X)
/**
 * @brief ACMP1 interrupt service routine.
 *
 * @details
 * This ISR is triggered when ACMP1 generates an interrupt. It calls the common
 * interrupt handler Acmp_Hal_IRQHandler with instance 1. Only available on AC7843X.
 *
 * @param void
 *
 * @return void
 *
 * @pre
 *     ACMP1 interrupt must be enabled.
 *
 * @post
 *     ACMP1 interrupt is serviced.
 *
 * @note
 *     Reentrancy: Not reentrant.
 *     DES ID: DES_ACMP_API_011
 */
ISR(ACMP1_IRQHandler)
{
    Acmp_Hal_IRQHandler(1U);
}
#endif

/*==================================STATIC  FUNCTION  IMPLEMENTATIONS=================================*/
static void Acmp_Hal_ConfigComparator(uint8 Instance, const Acmp_ComparatorType *Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    IRQn_Type Irq = Acmp_IrqId[Instance];

    /*PRQA S 2812 ++ # AMCE_ACMP_2812.*/
    Acmp_Reg_SetInterruptEnableFlag(Base, Config->InterruptEn);
    Acmp_Reg_SetTriggerMode(Base, Config->OutputTrigger);
    Acmp_Reg_SetOutputSelection(Base, Config->OutputSelect);
    Acmp_Reg_SetFilterEnableFlag(Base, Config->FilterEnable);
    Acmp_Reg_SetFilterLength(Base, Config->FilterSampleCount);
    Acmp_Reg_SetFilterClockPrescaler(Base, Config->ClockDivide);
    Acmp_Reg_SetLowPassFilter(Base, Config->LpfBandwidth);
    Acmp_Reg_SetInvertMode(Base, Config->InverterEnable);
    Acmp_Reg_SetOutputPinEnableFlag(Base, Config->PinState);
    Acmp_Reg_SetHysteresisMode(Base, Config->HysteresisMode);
    Acmp_Reg_SetHysteresisVoltage(Base, Config->HysteresisLevel);
#if !defined(AC7840E)
    /* Before changing clock source, should disable the ACMP power first */
    Acmp_Reg_SetUsingLSIEnableFlag(Base, Config->UsingLSIEnable);
#if defined (AC7840X)
    Acmp_Reg_SetLPEnableFlag(Config->UsingLSIEnable);
#elif defined (AC7842X)
    Acmp_Reg_SetLPEnableFlag(Base, Config->UsingLSIEnable);
#endif
#endif
    Acmp_Reg_SetWindowMode(Base, Config->WindowModeEnable);
    Acmp_Reg_SetEnableFlag(Base, Config->PowerEnable);

    Acmp_HalCallback[Instance] = Config->Callback;
    /*PRQA S 2842 -- */

    /* Config ACMP interrupt */
    if (TRUE == Config->InterruptEn)
    {
        Core_Hal_EnableIrq(Irq);
    }
    else
    {
        Core_Hal_DisableIrq(Irq);
        Core_Hal_ClearPendingIrq(Irq);
    }
    /*PRQA S 2812 -- */
}

#ifndef ACMP_SDK_NON_EXTENDED_API
static void Acmp_Hal_GetComparatorConfig(uint8 Instance, Acmp_ComparatorType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];

    /*PRQA S 2812 ++ # AMCE_ACMP_2812.*/
    Config->InterruptEn = Acmp_Reg_GetInterruptEnableFlag(Base);
    Config->OutputTrigger = Acmp_Reg_GetTriggerMode(Base);
    Config->OutputSelect = Acmp_Reg_GetOutputSelection(Base);
    Config->FilterEnable = Acmp_Reg_GetFilterEnableFlag(Base);
    Config->FilterSampleCount = Acmp_Reg_GetFilterLength(Base);
    Config->ClockDivide = Acmp_Reg_GetFilterClockPrescaler(Base);
    Config->LpfBandwidth = Acmp_Reg_GetLowPassFilter(Base);
    Config->InverterEnable = Acmp_Reg_GetInvertMode(Base);
    Config->PinState = Acmp_Reg_GetOutputPinEnableFlag(Base);
    Config->HysteresisMode = Acmp_Reg_GetHysteresisMode(Base);
    Config->HysteresisLevel = Acmp_Reg_GetHysteresisVoltage(Base);
#if !defined(AC7840E)
    Config->UsingLSIEnable = Acmp_Reg_GetUsingLSIEnableFlag(Base);
#endif
    Config->WindowModeEnable = Acmp_Reg_GetWindowMode(Base);
    Config->Callback = Acmp_HalCallback[Instance];
    Config->PowerEnable = Acmp_Reg_GetEnableFlag(Base);
    /*PRQA S 2842 -- */
    /*PRQA S 2812 -- */
}
#endif

static void Acmp_Hal_ConfigDAC(uint8 Instance, const Acmp_DacType *Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    /*PRQA S 2812 ++ # AMCE_ACMP_2812.*/
    Acmp_Reg_SetDACReference(Base, Config->VoltageReferenceSource);
    Acmp_Reg_SetDACOutput(Base, Config->Voltage);
    Acmp_Reg_SetDACEnableFlag(Base, Config->State);
#if defined (AC7842X)  || defined (AC7843X)
    Acmp_Reg_SetDACOutputEnableFlag(Base, Config->OutToPin);
    Acmp_Reg_SetDACBufferEnableFlag(Base, Config->BufferEnable);
#endif
    /*PRQA S 2812 -- */
}

#ifndef ACMP_SDK_NON_EXTENDED_API
static void Acmp_Hal_GetDACConfig(uint8 Instance, Acmp_DacType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    /*PRQA S 2812 ++ # AMCE_ACMP_2812.*/
    Config->VoltageReferenceSource = Acmp_Reg_GetDACReference(Base);
    Config->Voltage = Acmp_Reg_GetDACOutput(Base);
    Config->State = Acmp_Reg_GetDACEnableFlag(Base);
#if defined (AC7842X)  || defined (AC7843X)
    Config->OutToPin = Acmp_Reg_GetDACOutputEnableFlag(Base);
    Config->BufferEnable = Acmp_Reg_GetDACBufferEnableFlag(Base);
#endif
    /*PRQA S 2812 -- */
}
#endif

static void Acmp_Hal_ConfigMUX(uint8 Instance, const Acmp_AnmuxType *Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    /*PRQA S 2812 ++ # AMCE_ACMP_2812.*/
    Acmp_Reg_SetPositiveInputPin(Base, Config->PositiveInputMux);
    Acmp_Reg_SetNegativeInputPin(Base, Config->NegativeInputMux);
    /*PRQA S 2812 -- */
}

#ifndef ACMP_SDK_NON_EXTENDED_API
static void Acmp_Hal_GetMUXConfig(uint8 Instance, Acmp_AnmuxType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    /*PRQA S 2812 ++ # AMCE_ACMP_2812.*/
    Config->PositiveInputMux = Acmp_Reg_GetPositiveInputPin(Base);
    Config->NegativeInputMux = Acmp_Reg_GetNegativeInputPin(Base);
    /*PRQA S 2812 -- */
}
#endif

static void Acmp_Hal_SetPollingMode(ACMP_Type *Base, const Acmp_InputPollingType Polling)
{
    boolean PositiveEnable;
    boolean NegativeEnable;

    /* Determine polling mode settings */
    if (ACMP_NONE_POLLING == Polling)
    {
        PositiveEnable = FALSE;
        NegativeEnable = FALSE;
    }
    else if (ACMP_POSITIVE_POLLING == Polling)
    {
        PositiveEnable = TRUE;
        NegativeEnable = FALSE;
    }
    else /* ACMP_NEGATIVE_POLLING */
    {
        PositiveEnable = FALSE;
        NegativeEnable = TRUE;
    }

    /* Configure polling mode with minimal register access */
    Acmp_Reg_SetPositivePollingEnableFlag(Base, PositiveEnable);
    Acmp_Reg_SetNegativePollingEnableFlag(Base, NegativeEnable);
}

static void Acmp_Hal_ConfigPollingMode(uint8 Instance, const Acmp_PollingModeType *Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    /*PRQA S 2812 ++ # AMCE_ACMP_2812.*/
    Acmp_Hal_SetPollingMode(Base, Config->Mode);

    if (ACMP_NONE_POLLING != Config->Mode)
    {
        Acmp_Reg_SetPollingFreqDiv(Base, Config->PollingClockDivide);
        Acmp_Reg_SetPollingSequence(Base, Config->PollingSequence);
        Acmp_Reg_SetHallOutputEnableFlag(Base, Config->HallOutputEnable);
        Acmp_Reg_SetHallOutputA(Base, Config->HallAOutputCh);
        Acmp_Reg_SetHallOutputB(Base, Config->HallBOutputCh);
        Acmp_Reg_SetHallOutputC(Base, Config->HallCOutputCh);
    }
    /*PRQA S 2812 -- */
}

#ifndef ACMP_SDK_NON_EXTENDED_API
static Acmp_InputPollingType Acmp_Hal_GetPollingMode(const ACMP_Type *Base)
{
    Acmp_InputPollingType Polling = ACMP_NONE_POLLING;
    boolean Positive = Acmp_Reg_GetPositivePollingEnableFlag(Base);
    boolean Negative = Acmp_Reg_GetNegativePollingEnableFlag(Base);

    /* Use positive polling */
    if ((TRUE == Positive) && (FALSE == Negative))
    {
        Polling = ACMP_POSITIVE_POLLING;
    }
    else if ((FALSE == Positive) && (TRUE == Negative)) /* Use Negative polling */
    {
        Polling = ACMP_NEGATIVE_POLLING;
    }
    else /* Not using polling mode */
    {
        Polling = ACMP_NONE_POLLING;
    }

    return Polling;
}

static void Acmp_Hal_GetPollingModeConfig(uint8 Instance, Acmp_PollingModeType *const Config)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2842 ++ # AMCE_ACMP_2842.*/
    const ACMP_Type *Base = Acmp_HalBase[Instance];
    /*PRQA S 2842 -- */

    /*PRQA S 2812 ++ # AMCE_ACMP_2812.*/
    Config->PollingClockDivide = Acmp_Reg_GetPollingFreqDiv(Base);
    Config->Mode = Acmp_Hal_GetPollingMode(Base);
    Config->PollingSequence = Acmp_Reg_GetPollingSequence(Base);
    Config->HallOutputEnable = Acmp_Reg_GetHallOutputEnableFlag(Base);
    Config->HallAOutputCh = Acmp_Reg_GetHallOutputA(Base);
    Config->HallBOutputCh = Acmp_Reg_GetHallOutputB(Base);
    Config->HallCOutputCh = Acmp_Reg_GetHallOutputC(Base);
    /*PRQA S 2812 -- */
}
#endif

static void Acmp_Hal_IRQHandler(uint8 Instance)
{
    ACMP_Type *Base = Acmp_HalBase[Instance];
    uint32 state;

    /* Get all interrupt status */
    state = Acmp_Reg_GetInterruptStatus(Base);

    /* Clear all interrupt status */
    Acmp_Reg_ClearInterruptStatus(Base, state);

    /* Call the customized interrupt handler function */
    if (NULL_PTR != Acmp_HalCallback[Instance])
    {
        Acmp_HalCallback[Instance](Instance, state);
    }
}
/* =============================================  EOF  ============================================== */
