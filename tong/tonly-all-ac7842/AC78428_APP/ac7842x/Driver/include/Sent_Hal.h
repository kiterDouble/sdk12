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

/**
 * @file Sent_Hal.h
 *
 * @brief This file provides SENT HAL layer integration functions interface.
 *
 */

#ifndef SENT_HAL_H
#define SENT_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Sent_Hal_Types.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
#ifndef SENT_SDK_NON_EXTENDED_API
/**
 * @brief Initializes the SENT channel configuration structure with default values.
 *
 * @details
 * This function fills the provided SENT channel configuration structure with default
 * settings suitable for typical SENT channel operation. It includes default values
 * for watchdog timer, I/O control, receive control, tick time, interrupt settings,
 * nibble ordering, and callback functions.
 *
 * @param[out] SentChannelConfig: Pointer to the SENT channel configuration structure
 *                                 to be initialized with default values.
 *
 * @return void
 *
 * @pre
 *     SentChannelConfig must point to a valid memory location.
 *
 * @post
 *     The configuration structure is filled with default values.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This function is typically called before customizing specific parameters.
 *     DES ID: DES_SENT_API_014
 */
void Sent_Hal_GetDefaultChannelConfig(Sent_ChannelConfigType *SentChannelConfig);

/**
 * @brief Initializes the SENT module configuration structure with default values.
 *
 * @details
 * This function fills the provided SENT module configuration structure with default
 * settings for the module-level parameters, including clock divider and timestamp
 * predivider values.
 *
 * @param[out] SentModuleConfig: Pointer to the SENT module configuration structure
 *                                to be initialized with default values.
 *
 * @return void
 *
 * @pre
 *     SentModuleConfig must point to a valid memory location.
 *
 * @post
 *     The configuration structure is filled with default values.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This function is typically called before customizing specific parameters.
 *     DES ID: DES_SENT_API_015
 */
void Sent_Hal_GetDefaultModuleConfig(Sent_ModuleConfigType *SentModuleConfig);
#endif

/**
 * @brief Initializes the SENT module.
 *
 * @details
 * This function initializes the SENT module by enabling the peripheral clock,
 * performing a module reset, configuring the module clock divider and timestamp
 * divider, and enabling NVIC interrupts for all SENT channels.
 *
 * @param[in] SentModuleConfig: Pointer to the SENT module configuration structure
 *                               containing initialization parameters.
 *
 * @return void
 *
 * @pre
 *     SentModuleConfig must point to a valid configuration structure.
 *     The module must not be already initialized.
 *
 * @post
 *     The SENT module is initialized and ready for channel configuration.
 *     Module clock and interrupts are enabled.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function must be called before any channel operations.
 *     DES ID: DES_SENT_API_016
 */
void Sent_Hal_Init(const Sent_ModuleConfigType *SentModuleConfig);

/**
 * @brief Deinitializes the SENT module.
 *
 * @details
 * This function deinitializes the SENT module by disabling all channel interrupts
 * at the NVIC level, performing a module reset, and disabling the peripheral clock.
 * All pending interrupts are cleared.
 *
 * @return void
 *
 * @pre
 *     The SENT module must have been previously initialized.
 *
 * @post
 *     The SENT module is deinitialized and all interrupts are disabled.
 *     Module clock is disabled.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     All channels should be disabled before calling this function.
 *     DES ID: DES_SENT_API_017
 */
void Sent_Hal_Deinit(void);

/**
 * @brief Configures a SENT channel.
 *
 * @details
 * This function configures a specific SENT channel with the provided settings.
 * It disables the channel first, then configures watchdog timer, I/O control,
 * receive control, tick time, interrupts, nibble ordering, and callback functions.
 *
 * @param[in] ChannelId: SENT channel identifier (0 to SENT_INSTANCE_MAX-1).
 * @param[in] SentChannelConfig: Pointer to the SENT channel configuration structure
 *                                containing all configuration parameters.
 *
 * @return void
 *
 * @pre
 *     The SENT module must be initialized via Sent_Hal_Init().
 *     SentChannelConfig must point to a valid configuration structure.
 *     ChannelId must be valid and the channel state must be allocated.
 *
 * @post
 *     The SENT channel is configured but remains disabled.
 *     Channel must be enabled separately via Sent_Hal_EnableChannel().
 *
 * @note
 *     Reentrancy: Reentrant for different channels; not reentrant for the same channel.
 *     The channel is automatically disabled during configuration.
 *     DES ID: DES_SENT_API_018
 */
void Sent_Hal_ConfigChannel(Sent_ChannelIdType ChannelId, const Sent_ChannelConfigType *SentChannelConfig);

/**
 * @brief Enables or disables a SENT channel.
 *
 * @details
 * This function enables or disables the specified SENT channel for operation.
 * When enabled, the channel can start receiving or transmitting SENT frames.
 *
 * @param[in] ChannelId: SENT channel identifier (0 to SENT_INSTANCE_MAX-1).
 * @param[in] Enable: TRUE to enable the channel, FALSE to disable.
 *
 * @return void
 *
 * @pre
 *     The SENT module must be initialized via Sent_Hal_Init().
 *     The channel must be configured via Sent_Hal_ConfigChannel().
 *
 * @post
 *     The SENT channel is enabled or disabled as requested.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; not reentrant for the same channel.
 *     Disabling the channel during active communication may result in incomplete frames.
 *     DES ID: DES_SENT_API_019
 */
void Sent_Hal_EnableChannel(Sent_ChannelIdType ChannelId, boolean Enable);

/**
 * @brief Retrieves the current status of a SENT channel.
 *
 * @details
 * This function reads and returns the current operational status of the specified
 * SENT channel, indicating whether the channel is idle, busy, or in an error state.
 *
 * @param[in] ChannelId: SENT channel identifier (0 to SENT_INSTANCE_MAX-1).
 *
 * @return Sent_ChannelStatusType: Current status of the SENT channel.
 *
 * @pre
 *     The SENT module must be initialized via Sent_Hal_Init().
 *     The channel must be configured via Sent_Hal_ConfigChannel().
 *
 * @post
 *     None. This is a read-only operation.
 *
 * @note
 *     Reentrancy: Reentrant.
 *     This function can be called at any time after channel configuration.
 *     DES ID: DES_SENT_API_020
 */
Sent_ChannelStatusType Sent_Hal_GetChannelStatus(Sent_ChannelIdType ChannelId);

/**
 * @brief Initializes the SENT channel transmit configuration.
 *
 * @details
 * This function configures the SPC (Short PWM Code) transmit control parameters
 * for the specified SENT channel, including pulse length, delay length, trigger mode,
 * and time base settings.
 *
 * @param[in] ChannelId: SENT channel identifier (0 to SENT_INSTANCE_MAX-1).
 * @param[in] SentChannelTxConfig: Pointer to the SENT channel transmit configuration
 *                                  structure containing SPC parameters.
 *
 * @return void
 *
 * @pre
 *     The SENT module must be initialized via Sent_Hal_Init().
 *     The channel must be configured via Sent_Hal_ConfigChannel().
 *     SentChannelTxConfig must point to a valid configuration structure.
 *
 * @post
 *     The SENT channel transmit control is configured.
 *
 * @note
 *     Reentrancy: Reentrant for different channels; not reentrant for the same channel.
 *     This configuration is specific to SPC (Short PWM Code) transmission.
 *     DES ID: DES_SENT_API_021
 */
void Sent_Hal_InitChannelTxCtrl(Sent_ChannelIdType ChannelId, const Sent_TransmitCtrlType *SentChannelTxConfig);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* SENT_HAL_H */

/* =============================================  EOF  ============================================== */
