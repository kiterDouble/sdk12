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
 * @file Acmp_Hal.h
 *
 * @brief This file provides analog comparator module integration functions interfaces.
 *
 */

#ifndef ACMP_HAL_H
#define ACMP_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Acmp_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
#ifndef ACMP_SDK_NON_EXTENDED_API
/**
 * @brief Gets the default configuration structure for ACMP module.
 *
 * @details
 * This function initializes the provided configuration structure with default values
 * suitable for basic ACMP operation. The default configuration includes disabled interrupt,
 * both edge triggering, normal output, disabled filter, default hysteresis settings, and
 * enabled power. This can be used as a starting point before customization.
 *
 * @param[out] Config: Pointer to ACMP configuration structure to be filled with defaults.
 *
 * @return void
 *
 * @pre
 *     Config must point to a valid Acmp_ModuleType structure.
 *
 * @post
 *     Config structure is populated with default ACMP settings.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This function does not modify hardware; it only fills the configuration structure.
 *     DES ID: DES_ACMP_API_008
 */
void Acmp_Hal_GetDefaultConfig(Acmp_ModuleType *Config);

/**
 * @brief Resets all ACMP registers to their default state.
 *
 * @details
 * This function performs a hardware reset on the specified ACMP instance by asserting
 * and then deasserting the reset signal through the Reset and Clock Manager (RCM).
 * This function must be called after the ACMP clock has been enabled and any previous
 * reset state has been cleared. All ACMP interrupts are disabled and cleared before reset.
 *
 * @param[in] Instance: ACMP instance number.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *
 * @post
 *     ACMP hardware registers are reset to default values.
 *     ACMP interrupts are disabled and pending flags are cleared.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     DES ID: DES_ACMP_API_009
 */
void Acmp_Hal_Reset(uint8 Instance);
#endif

/**
 * @brief Initializes the ACMP module with the specified configuration.
 *
 * @details
 * This function configures all ACMP comparator functions including the comparator itself,
 * internal DAC, analog input multiplexer, and polling mode settings. It enables the
 * peripheral clock, performs a reset, and applies the complete configuration provided.
 * The function configures interrupts, trigger modes, filters, hysteresis, power control,
 * and all other ACMP features according to the configuration structure.
 *
 * @param[in] Instance: ACMP instance number.
 * @param[in] Config: Pointer to the complete ACMP configuration structure.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     Config must point to a valid and properly initialized configuration structure.
 *     System clock and reset control must be properly initialized.
 *
 * @post
 *     ACMP peripheral clock is enabled.
 *     ACMP hardware is reset and configured according to the provided settings.
 *     Comparator, DAC, MUX, and polling mode are configured.
 *     Interrupts are enabled if configured in the structure.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     This function must be called before using any ACMP functionality.
 *     DES ID: DES_ACMP_API_000
 */
void Acmp_Hal_Init(uint8 Instance, const Acmp_ModuleType *const Config);

/**
 * @brief Deinitializes the ACMP module and disables its clock.
 *
 * @details
 * This function disables the specified ACMP instance by disabling interrupts, performing
 * a hardware reset, and disabling the peripheral clock. This effectively powers down the
 * ACMP module and returns it to its reset state. All pending interrupts are cleared.
 *
 * @param[in] Instance: ACMP instance number.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     ACMP module should have been previously initialized.
 *
 * @post
 *     ACMP interrupts are disabled and cleared.
 *     ACMP hardware is reset to default state.
 *     ACMP peripheral clock is disabled.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     After calling this function, the ACMP module must be reinitialized before use.
 *     DES ID: DES_ACMP_API_001
 */
void Acmp_Hal_Deinit(uint8 Instance);

/**
 * @brief Enables or disables the specified ACMP instance.
 *
 * @details
 * This function controls the power state of the ACMP comparator by setting or clearing
 * the enable bit in the hardware register. When enabled, the comparator becomes active
 * and can perform analog comparisons. When disabled, the comparator is powered down to
 * save energy.
 *
 * @param[in] Instance: ACMP instance number.
 * @param[in] Enable: TRUE to enable the comparator; FALSE to disable it.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     ACMP module must be initialized via Acmp_Hal_Init().
 *
 * @post
 *     ACMP comparator is enabled or disabled according to the Enable parameter.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     Configuration should be completed before enabling the comparator.
 *     DES ID: DES_ACMP_API_014
 */
void Acmp_Hal_Enable(uint8 Instance, boolean Enable);

#ifndef ACMP_SDK_NON_EXTENDED_API
/**
 * @brief Retrieves the current configuration of the ACMP module.
 *
 * @details
 * This function reads all hardware registers and fills the provided configuration structure
 * with the current settings of the comparator, internal DAC, analog input multiplexer,
 * and polling mode. This is useful for saving the current state or verifying configuration.
 *
 * @param[in] Instance: ACMP instance number.
 * @param[out] Config: Pointer to configuration structure to be filled with current settings.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     Config must point to a valid Acmp_ModuleType structure.
 *     ACMP module should be initialized.
 *
 * @post
 *     Config structure contains the complete current hardware configuration.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This function only reads registers and does not modify hardware state.
 *     DES ID: DES_ACMP_API_015
 */
void Acmp_Hal_GetConfigAll(uint8 Instance, Acmp_ModuleType *const Config);
#endif

/**
 * @brief Gets the ACMP comparator output data in normal mode.
 *
 * @details
 * This function reads and returns the current output state of the comparator in normal
 * (non-polling) mode. The output reflects the result of comparing the positive and
 * negative input voltages. The output is typically 1 when the positive input is higher
 * than the negative input, and 0 otherwise (or inverted if inversion is enabled).
 *
 * @param[in] Instance: ACMP instance number.
 *
 * @return uint8: Current comparator output value (0 or 1).
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     ACMP module must be initialized and enabled.
 *     ACMP must be in normal (non-polling) mode.
 *
 * @post
 *     None. This is a read-only operation.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This function performs a register read and does not modify hardware state.
 *     DES ID: DES_ACMP_API_002
 */
uint8 Acmp_Hal_GetOutputData(uint8 Instance);

#ifndef ACMP_SDK_NON_EXTENDED_API
/**
 * @brief Gets the ACMP output status flags.
 *
 * @details
 * This function reads and returns the output status flags from the ACMP hardware.
 * These flags indicate events such as rising edge detection, falling edge detection,
 * or other output-related status conditions configured in the comparator.
 *
 * @param[in] Instance: ACMP instance number.
 *
 * @return uint8: Current output status flags.
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     ACMP module must be initialized.
 *
 * @post
 *     None. This is a read-only operation.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     Flags must be explicitly cleared using Acmp_Hal_ClearOutputFlags().
 *     DES ID: DES_ACMP_API_004
 */
uint8 Acmp_Hal_GetOutputFlags(uint8 Instance);

/**
 * @brief Clears the ACMP output status flags.
 *
 * @details
 * This function clears all pending output status flags in the ACMP hardware.
 * This is typically done after processing detected events to prepare for
 * detecting new events. Clearing flags is necessary to acknowledge interrupt
 * conditions and to detect subsequent events.
 *
 * @param[in] Instance: ACMP instance number.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     ACMP module must be initialized.
 *
 * @post
 *     All ACMP output status flags are cleared.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     This function is typically called from an interrupt service routine.
 *     DES ID: DES_ACMP_API_005
 */
void Acmp_Hal_ClearOutputFlags(uint8 Instance);
#endif

/**
 * @brief Gets the ACMP polling mode comparison data.
 *
 * @details
 * This function reads and returns the comparison data from the ACMP when operating
 * in polling mode. In polling mode, the ACMP can automatically scan multiple input
 * channels. The returned value contains the comparison results for the polled channels,
 * with each bit representing the comparison result of one input channel.
 *
 * @param[in] Instance: ACMP instance number.
 *
 * @return uint16: Polling mode comparison data with each bit representing a channel result.
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     ACMP module must be initialized and enabled.
 *     ACMP must be configured in polling mode.
 *
 * @post
 *     None. This is a read-only operation.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This function is typically used in polling mode for multi-channel sensing.
 *     DES ID: DES_ACMP_API_003
 */
uint16 Acmp_Hal_GetPollingData(uint8 Instance);

#ifndef ACMP_SDK_NON_EXTENDED_API
/**
 * @brief Gets the ACMP polling mode status flags.
 *
 * @details
 * This function reads and returns the polling mode status flags from the ACMP hardware.
 * These flags indicate the status of polled input channels, such as whether a channel
 * has completed comparison or if specific events have occurred during polling operation.
 *
 * @param[in] Instance: ACMP instance number.
 *
 * @return uint16: Polling mode status flags for all polled channels.
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     ACMP module must be initialized.
 *     ACMP should be configured in polling mode.
 *
 * @post
 *     None. This is a read-only operation.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     Flags must be explicitly cleared using Acmp_Hal_ClearInputFlags().
 *     DES ID: DES_ACMP_API_006
 */
uint16 Acmp_Hal_GetPollingFlags(uint8 Instance);

/**
 * @brief Clears the ACMP polling mode status flags.
 *
 * @details
 * This function clears all pending polling mode status flags in the ACMP hardware.
 * This is typically done after processing polling mode events to prepare for detecting
 * new events on the polled input channels. Clearing flags acknowledges the processed
 * events and allows detection of subsequent polling events.
 *
 * @param[in] Instance: ACMP instance number.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than ACMP_INSTANCE_MAX.
 *     ACMP module must be initialized.
 *     ACMP should be configured in polling mode.
 *
 * @post
 *     All ACMP polling mode status flags are cleared.
 *
 * @note
 *     Reentrancy: Reentrant for different instances; not reentrant for the same instance.
 *     This function is typically called after reading polling data or from ISR.
 *     DES ID: DES_ACMP_API_007
 */
void Acmp_Hal_ClearInputFlags(uint8 Instance);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ACMP_HAL_H */

/* =============================================  EOF  ============================================== */
