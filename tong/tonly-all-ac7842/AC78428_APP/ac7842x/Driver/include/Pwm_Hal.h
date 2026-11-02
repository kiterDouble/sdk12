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

#ifndef PWM_HAL_H
#define PWM_HAL_H

#ifdef __cplusplus
extern "C" {
#endif
/* ===========================================  INCLUDE FILES  =========================================== */
#include "Pwm_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/**
 * @brief Initialize PWM output mode
 * @details This function initializes the PWM module in output mode according to the provided configuration.
 *          It configures clock source, prescaler, counter values, channel modes and other related parameters.
 *
 * @param[in] Instance PWM instance to initialize
 * @param[in] Config Pointer to the output configuration structure
 *
 * @return void
 *
 * @pre The PWM module must be de-initialized before calling this function
 * @post The PWM module is configured in output mode according to the provided configuration
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_201
 */
void Pwm_Hal_InitOutputMode
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_OutputCfg *Config
);

/**
 * @brief Initialize PWM input mode
 * @details This function initializes the PWM module in input mode according to the provided configuration.
 *          It configures clock source, prescaler, counter values, capture channels and other related parameters.
 *
 * @param[in] Instance PWM instance to initialize
 * @param[in] Config Pointer to the input configuration structure
 *
 * @return void
 *
 * @pre The PWM module must be de-initialized before calling this function
 * @post The PWM module is configured in input mode according to the provided configuration
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_202
 */
void Pwm_Hal_InitInputMode
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_InputCfg *Config
);

/**
 * @brief Initialize PWM quadrature decoder mode
 * @details This function initializes the PWM module in quadrature decoder mode according to the provided configuration.
 *          It configures clock source, prescaler, counter values, phase inputs and other related parameters.
 *
 * @param[in] Instance PWM instance to initialize
 * @param[in] Config Pointer to the quadrature decoder configuration structure
 *
 * @return void
 *
 * @pre The PWM module must be de-initialized before calling this function
 * @post The PWM module is configured in quadrature decoder mode according to the provided configuration
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_261
 */
void Pwm_Hal_InitQuadDecoderMode
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_QuadDecoderCfg *Config
);

/**
 * @brief Deinitialize PWM module
 * @details This function deinitializes the PWM module by resetting it and disabling its clock.
 *          It also disables all related interrupts and clears pending interrupt requests.
 *
 * @param[in] Instance PWM instance to deinitialize
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The PWM module is reset and its clock is disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_203
 */
void Pwm_Hal_DeInit
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Set PWM module clock source and prescaler
 * @details This function configures the clock source and prescaler for the specified PWM module.
 *
 * @param[in] Instance PWM instance to configure
 * @param[in] ClkSource Clock source type
 * @param[in] ClkPsc Clock prescaler value
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The PWM module clock source and prescaler are configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_204
 */
void Pwm_Hal_SetClockSource
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ClockSourceType ClkSource,
    uint16 ClkPsc
);

/**
 * @brief Reset module counter value
 * @details This function resets the counter value of the specified PWM module to zero.
 *
 * @param[in] Instance PWM instance to reset
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The PWM module counter value is reset to zero
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_212
 */
void Pwm_Hal_ResetCounter
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Get module counter value
 * @details This function returns the current counter value of the specified PWM module.
 *
 * @param[in] Instance PWM instance to read
 *
 * @return Current counter value
 *
 * @pre The PWM module should be initialized
 * @post The current counter value is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_211
 */
uint16 Pwm_Hal_GetCountValue
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Set PWM module counter maximum value
 * @details This function sets the maximum counter value for the specified PWM module.
 *
 * @param[in] Instance PWM instance to configure
 * @param[in] Value Maximum counter value
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The PWM module maximum counter value is configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_208
 */
void Pwm_Hal_SetMaxCountValue
(
    Pwm_Hal_InstanceType Instance,
    uint16 Value
);

/**
 * @brief Get PWM module counter maximum value
 * @details This function returns the maximum counter value of the specified PWM module.
 *
 * @param[in] Instance PWM instance to read
 *
 * @return Maximum counter value
 *
 * @pre The PWM module should be initialized
 * @post The maximum counter value is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_207
 */
uint16 Pwm_Hal_GetMaxCountValue
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Set PWM module counter initial value
 * @details This function sets the initial counter value for the specified PWM module.
 *
 * @param[in] Instance PWM instance to configure
 * @param[in] Value Initial counter value
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The PWM module initial counter value is configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_210
 */
void Pwm_Hal_SetInitCountValue
(
    Pwm_Hal_InstanceType Instance,
    uint16 Value
);

/**
 * @brief Get PWM module counter initial value
 * @details This function returns the initial counter value of the specified PWM module.
 *
 * @param[in] Instance PWM instance to read
 *
 * @return Initial counter value
 *
 * @pre The PWM module should be initialized
 * @post The initial counter value is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_209
 */
uint16 Pwm_Hal_GetInitCountValue
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Set channel value
 * @details This function sets the compare value for the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Value Channel compare value
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The channel compare value is configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_206
 */
void Pwm_Hal_SetChannelValue
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    uint16 Value
);

/**
 * @brief Get channel match value
 * @details This function returns the compare value of the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to read
 *
 * @return Channel compare value
 *
 * @pre The PWM module should be initialized
 * @post The channel compare value is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_205
 */
uint16 Pwm_Hal_GetChannelValue
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel
);

/**
 * @brief Enable or disable PWM channel match interrupt
 * @details This function enables or disables the match interrupt for the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Enable TRUE to enable interrupt, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The channel match interrupt is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_214
 */
void Pwm_Hal_EnableChannelInterrupt
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief Enable or disable PWM module overflow interrupt
 * @details This function enables or disables the overflow interrupt for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Enable TRUE to enable interrupt, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The module overflow interrupt is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_213
 */
void Pwm_Hal_EnableOverflowInterrupt
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief Enable or disable PWM module overflow event
 * @details This function enables or disables the overflow event for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Enable TRUE to enable event, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The module overflow event is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_215
 */
void Pwm_Hal_EnableOverflowEvent
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief Enable or disable PWM module underflow event
 * @details This function enables or disables the underflow event for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Enable TRUE to enable event, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The module underflow event is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_216
 */
void Pwm_Hal_EnableUnderflowEvent
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief Get all PWM interrupt flags
 * @details This function returns the combined interrupt flags for all channels of the specified PWM module.
 *
 * @param[in] Instance PWM instance
 *
 * @return Interrupt flags for all channels
 *
 * @pre The PWM module should be initialized
 * @post The combined interrupt flags are returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_217
 */
uint32 Pwm_Hal_GetChannelInterruptFlag
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Clear all channel interrupt flags
 * @details This function clears the interrupt flags for the specified channels using the provided mask.
 *
 * @param[in] Instance PWM instance
 * @param[in] Mask Bit mask to clear specific channel interrupt flags
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The specified channel interrupt flags are cleared
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_218
 */
void Pwm_Hal_ClearChannelInterruptFlag
(
    Pwm_Hal_InstanceType Instance,
    uint32 Mask
);

/**
 * @brief Get PWM overflow flag
 * @details This function returns the overflow flag of the specified PWM module.
 *
 * @param[in] Instance PWM instance
 *
 * @return Overflow flag
 *
 * @pre The PWM module should be initialized
 * @post The overflow flag is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_219
 */
uint32 Pwm_Hal_GetOverflowFlag
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Clear PWM overflow flag
 * @details This function clears the overflow flag of the specified PWM module.
 *
 * @param[in] Instance PWM instance
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The overflow flag is cleared
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_220
 */
void Pwm_Hal_ClearOverflowFlag
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Get PWM overflow direction
 * @details This function returns the overflow direction of the specified PWM module.
 *
 * @param[in] Instance PWM instance
 *
 * @return Overflow direction flag
 *
 * @pre The PWM module should be initialized
 * @post The overflow direction flag is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_221
 */
uint32 Pwm_Hal_GetOverflowDir
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Get all PWM channel levels
 * @details This function returns the current output levels of all channels in the specified PWM module.
 *
 * @param[in] Instance PWM instance
 *
 * @return Current output levels of all channels
 *
 * @pre The PWM module should be initialized
 * @post The current output levels are returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_222
 */
uint32 Pwm_Hal_GetAllChannelLevel
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Enable DMA requests for channel events
 * @details This function enables or disables DMA requests for events on the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Enable TRUE to enable DMA requests, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post DMA requests for the specified channel are enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_223
 */
void Pwm_Hal_EnableChannelDmaRequest
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief Enable DMA requests for counter overflow events
 * @details This function enables or disables DMA requests for counter overflow events.
 *
 * @param[in] Instance PWM instance
 * @param[in] Enable TRUE to enable DMA requests, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post DMA requests for overflow are enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_283
 */
void Pwm_Hal_EnableOverflowDmaRequest
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief Initialize PWM synchronization configuration
 * @details This function initializes the synchronization configuration for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] ConfigPtr Pointer to the synchronization configuration structure
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post The synchronization configuration is applied
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_224
 */
void Pwm_Hal_InitSyncConfigSet
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_SyncCfg *ConfigPtr
);

/**
 * @brief Enable/disable synchronization
 * @details This function enables or disables synchronization for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Enable TRUE to enable synchronization, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Synchronization is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_288
 */
void Pwm_Hal_EnableSync
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief Enable/disable sync bypass
 * @details This function enables or disables synchronization bypass for CHnV/CNTIN/MCVR registers
 *          in the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Enable TRUE to enable bypass, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Synchronization bypass is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_225
 */
void Pwm_Hal_EnableSyncBypass
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief Start software sync trigger
 * @details This function triggers or stops a software synchronization for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] IsTrigger TRUE to trigger synchronization, FALSE to stop
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Software synchronization is triggered or stopped
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_226
 */
void Pwm_Hal_TrigSoftwareSync
(
    Pwm_Hal_InstanceType Instance,
    boolean IsTrigger
);

/**
 * @brief Enable/Disable global timer base
 * @details This function enables or disables the global timer base for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Enable TRUE to enable global timer base, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Global timer base is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_227
 */
void Pwm_Hal_EnableGlobalTimeBase
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief Global timer base output enable/disable
 * @details This function enables or disables the global timer base output for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Enable TRUE to enable global timer base output, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Global timer base output is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_228
 */
void Pwm_Hal_EnableGlobalTimeBaseOutput
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief Enable initialization trigger source
 * @details This function enables or disables the initialization trigger source for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Enable TRUE to enable init trigger, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Initialization trigger source is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_229
 */
void Pwm_Hal_EnableInitTrigger
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief Enable max trigger source
 * @details This function enables or disables the maximum value trigger source for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Enable TRUE to enable max trigger, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Maximum value trigger source is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_230
 */
void Pwm_Hal_EnableMaxTrigger
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief Enable channel match trigger source
 * @details This function enables or disables the match trigger source for the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Enable TRUE to enable match trigger, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Channel match trigger source is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_231
 */
void Pwm_Hal_EnableChannelMatchTrigger
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief Set the max/match/init trigger ratio
 * @details This function sets the trigger ratio for maximum, match, and initialization triggers
 *          in the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] TriggerRatio Trigger ratio value (0 ~ 7)
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Trigger ratio is configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_232
 */
void Pwm_Hal_SetTriggerRatio
(
    Pwm_Hal_InstanceType Instance,
    uint8 TriggerRatio
);

/**
 * @brief Set input channel capture edge
 * @details This function configures the capture edge for the specified input channel in the PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Edge Capture edge type
 *
 * @return void
 *
 * @pre The PWM module should be initialized in input mode
 * @post Capture edge is configured for the specified channel
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_233
 */
void Pwm_Hal_SetChannelCaptureEdge
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_EdgeType Edge
);

/**
 * @brief Set channel input filter value
 * @details This function sets the input filter value for the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Value Filter value (0~15)
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Input filter value is configured for the specified channel
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_234
 */
void Pwm_Hal_SetChannelInputFilterVal
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    uint8 Value
);

/**
 * @brief Set PWM input filter prescaler value
 * @details This function sets the input filter prescaler value for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Prescaler Filter prescaler value (0 ~ 111)
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Input filter prescaler is configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_268
 */
void Pwm_Hal_SetInputFilterPsc
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_InputFilterPscType Prescaler
);

/**
 * @brief Enable/disable channel event reset
 * @details This function enables or disables counter reset by channel events for the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Enable TRUE to enable counter reset, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Channel event reset is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_235
 */
void Pwm_Hal_EnableChannelEventReset
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief Get PWM hall status
 * @details This function returns the hall status of the specified PWM module.
 *
 * @param[in] Instance PWM instance
 *
 * @return Hall status
 *
 * @pre The PWM module should be initialized in hall mode
 * @post Hall status is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_236
 */
uint32 Pwm_Hal_GetHallStatus
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Set matching action for output comparison mode
 * @details This function sets the action to be taken when a match occurs in output comparison mode
 *          for the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Action Compare action type
 *
 * @return void
 *
 * @pre The PWM module should be initialized in output compare mode
 * @post Compare action is configured for the specified channel
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_237
 */
void Pwm_Hal_SetChannelCompareAction
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_CompareActionType Action
);

/**
 * @brief Set software output enable status for each channel
 * @details This function sets the software output enable status for all channels in the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] EnableMask Status masks for each channel
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Software output enable status is configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_238
 */
void Pwm_Hal_SetSoftControlEnableStates
(
    Pwm_Hal_InstanceType Instance,
    uint8 EnableMask
);

/**
 * @brief Enable channel soft control
 * @details This function enables or disables software control for the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Enable TRUE to enable soft control, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Software control is enabled or disabled for the specified channel
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_239
 */
void Pwm_Hal_EnableChannelSoftControl
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief Enable soft control function for combine channels
 * @details This function enables or disables software control for the specified combine channel pair.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel pair to configure (must be even channel: 0, 2, 4, 6)
 * @param[in] Enable TRUE to enable soft control, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Software control is enabled or disabled for the specified channel pair
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_240
 */
void Pwm_Hal_EnableCombineChannelSoftControl
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief Set software output levels for all channels
 * @details This function sets the software output levels for all channels in the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] LevelMask Software output levels for all channels
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Software output levels are configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_241
 */
void Pwm_Hal_SetSoftControlLevels
(
    Pwm_Hal_InstanceType Instance,
    uint8 LevelMask
);

/**
 * @brief Set channel software control level
 * @details This function sets the software output level for the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Level Output level
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Software output level is configured for the specified channel
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_242
 */
void Pwm_Hal_SetChannelSoftControlLevel
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_OutputLevelType Level
);

/**
 * @brief Set channel dither value
 * @details This function sets the dither value for the match value of the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] DitherValue Channel match dither value
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Channel match dither value is configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_243
 */
void Pwm_Hal_SetChannelMatchDither
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    uint8 DitherValue
);

/**
 * @brief Set period dither value
 * @details This function sets the dither value for the period (maximum count) of the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] MaxCountDitherValue Period dither value (0 ~ 31)
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Period dither value is configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_244
 */
void Pwm_Hal_SetPeriodDither
(
    Pwm_Hal_InstanceType Instance,
    uint8 MaxCountDitherValue
);

/**
 * @brief Enable/disable channel output mask
 * @details This function enables or disables output masking for the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Enable TRUE to enable output mask, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Output mask is enabled or disabled for the specified channel
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_245
 */
void Pwm_Hal_EnableChannelOutputMask
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief Set the mask status for all channels
 * @details This function sets the output mask status for all channels in the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Mask The shielding status of all channels
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Output mask status is configured for all channels
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_247
 */
void Pwm_Hal_SetOutputMask
(
    Pwm_Hal_InstanceType Instance,
    uint8 Mask
);

/**
 * @brief Get PWM module channel output status
 * @details This function returns the current output mask status of all channels in the specified PWM module.
 *
 * @param[in] Instance PWM instance
 *
 * @return Channel output mask status
 *
 * @pre The PWM module should be initialized
 * @post Current output mask status is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_246
 */
uint32 Pwm_Hal_GetOutputMask
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Set channel polarity
 * @details This function sets the output polarity for the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Polarity Output polarity
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Output polarity is configured for the specified channel
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_248
 */
void Pwm_Hal_SetChannelPolarity
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_ActivePolarityType Polarity
);

/**
 * @brief Set deadtime prescaler & value
 * @details This function configures the deadtime insertion for the specified channel pair,
 *          including prescaler and value.
 *
 * @param[in] Instance PWM instance
 * @param[in] ChannelPair PWM channel pair to configure
 * @param[in] Prescaler Prescaler divider
 *            - PWM_DEADTIME_DIVID_1
 *            - PWM_DEADTIME_DIVID_4
 *            - PWM_DEADTIME_DIVID_16
 * @param[in] Value Inserted value (0 ~ 63)
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Deadtime configuration is applied to the specified channel pair
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_249
 */
void Pwm_Hal_SetDeadtime
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelPairType ChannelPair,
    Pwm_Hal_DeadTimePscType Prescaler,
    uint16 Value
);

/**
 * @brief Set pair channel symmetric
 * @details This function enables or disables symmetric mode for the specified channel pair.
 *
 * @param[in] Instance PWM instance
 * @param[in] ChannelPair PWM channel pair to configure
 * @param[in] Enable TRUE to enable symmetric mode, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Symmetric mode is enabled or disabled for the specified channel pair
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_250
 */
void Pwm_Hal_EnableChannelPairSymmetric
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelPairType ChannelPair,
    boolean Enable
);

/**
 * @brief Set channel inverting control
 * @details This function enables or disables inversion for the specified channel pair.
 *
 * @param[in] Instance PWM instance
 * @param[in] ChannelPair PWM channel pair to configure
 * @param[in] Enable TRUE to enable inversion, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Inversion is enabled or disabled for the specified channel pair
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_251
 */
void Pwm_Hal_EnableChannelPairInvert
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelPairType ChannelPair,
    boolean Enable
);

/**
 * @brief Initialize fault control
 * @details This function initializes the fault control configuration for the specified PWM module.
 *          Only applicable to PWM modulation output mode, called after PWM_Init().
 *
 * @param[in] Instance PWM instance
 * @param[in] ConfigPtr Pointer to configuration structure
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Fault control configuration is applied
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_252
 */
void Pwm_Hal_InitFaultControl
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_FaultCfg *ConfigPtr
);

/**
 * @brief Enable fault input pin
 * @details This function enables or disables the specified fault input pin.
 *
 * @param[in] Instance PWM instance
 * @param[in] FaultInputId PWM fault input ID
 * @param[in] Enable TRUE to enable fault input, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Fault input pin is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_253
 */
void Pwm_Hal_EnableFaultPinInput
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId,
    boolean Enable
);

/**
 * @brief Enable fault input filter
 * @details This function enables or disables the filtering function of the specified fault input pin.
 *
 * @param[in] Instance PWM instance
 * @param[in] FaultInputId PWM fault input ID
 * @param[in] Enable TRUE to enable filter, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Fault input filter is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_254
 */
void Pwm_Hal_EnableFaultInputFilter
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId,
    boolean Enable
);

/**
 * @brief Set fault input polarity
 * @details This function sets the polarity for the specified fault input pin.
 *
 * @param[in] Instance PWM instance
 * @param[in] FaultInputId PWM fault input ID
 * @param[in] ActivePolarity Active polarity (PWM_ACTIVE_POLARITY_HIGH / PWM_ACTIVE_POLARITY_LOW)
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Fault input polarity is configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_255
 */
void Pwm_Hal_SetFaultInputPolarity
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId,
    Pwm_Hal_ActivePolarityType ActivePolarity
);

/**
 * @brief Get fault pin detection flag
 * @details This function returns the detection flag for the specified fault input pin.
 *
 * @param[in] Instance PWM instance
 * @param[in] FaultInputId PWM fault input ID
 *
 * @return Fault pin detection flag
 *
 * @pre The PWM module should be initialized
 * @post Fault pin detection flag is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_256
 */
uint32 Pwm_Hal_GetFaultPinFlag
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId
);

/**
 * @brief Clear fault pin detection flag
 * @details This function clears the detection flag for the specified fault input pin.
 *
 * @param[in] Instance PWM instance
 * @param[in] FaultInputId PWM fault input ID
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Fault pin detection flag is cleared
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_257
 */
void Pwm_Hal_ClearFaultPinFlag
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId
);

/**
 * @brief Get the OR value of each fault input pin flag
 * @details This function returns the combined (OR) value of all fault input pin flags.
 *
 * @param[in] Instance PWM instance
 *
 * @return The OR value of each fault input pin flag
 *
 * @pre The PWM module should be initialized
 * @post Combined fault flags are returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_258
 */
uint32 Pwm_Hal_GetFaultFlag
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Clear the OR value of each fault input pin flag
 * @details This function clears all fault input pin flags.
 *
 * @param[in] Instance PWM instance
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post All fault input pin flags are cleared
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_259
 */
void Pwm_Hal_ClearFaultFlag
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Enable/disable channel high-Z output
 * @details This function enables or disables high-Z (high impedance) output for the specified PWM channel.
 *
 * @param[in] Instance PWM instance
 * @param[in] Channel PWM channel to configure
 * @param[in] Enable TRUE to enable high-Z output, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post High-Z output is enabled or disabled for the specified channel
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_260
 */
void Pwm_Hal_EnableChannelHizOutput
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
);

/**
 * @brief Get the current quadrature decoding count direction
 * @details This function returns the current count direction in quadrature decoder mode
 *          for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 *
 * @return Counting direction
 *
 * @pre The PWM module should be initialized in quadrature decoder mode
 * @post Current counting direction is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_262
 */
uint32 Pwm_Hal_GetQuadCountingDir
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Get the quadrature timer overflow direction
 * @details This function returns the overflow direction in quadrature decoder mode
 *          for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 *
 * @return Timer overflow direction
 *
 * @pre The PWM module should be initialized in quadrature decoder mode
 * @post Timer overflow direction is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_263
 */
uint32 Pwm_Hal_GetQuadOverflowDir
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Get the phaseZ status
 * @details This function returns the phaseZ status for the specified PWM module in quadrature decoder mode.
 *
 * @param[in] Instance PWM instance
 *
 * @return PhaseZ status
 *
 * @pre The PWM module should be initialized in quadrature decoder mode
 * @post PhaseZ status is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_264
 */
uint32 Pwm_Hal_GetQuadPhaseZFlag
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Clear the phaseZ status
 * @details This function clears the phaseZ status for the specified PWM module in quadrature decoder mode.
 *
 * @param[in] Instance PWM instance
 *
 * @return void
 *
 * @pre The PWM module should be initialized in quadrature decoder mode
 * @post PhaseZ status is cleared
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_265
 */
void Pwm_Hal_ClearQuadPhaseZFlag
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Check Z index detect event status
 * @details This function checks the Z index detect event status for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 *
 * @return Z index event status
 *
 * @pre The PWM module should be initialized in quadrature decoder mode
 * @post Z index event status is returned
 *
 * @note Reentrant: Yes
 *       DES ID: DES_PWM_API_269
 */
uint32 Pwm_Hal_GetDetectEventFlag
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Clear Z index detect event status
 * @details This function clears the Z index detect event status for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 *
 * @return void
 *
 * @pre The PWM module should be initialized in quadrature decoder mode
 * @post Z index event status is cleared
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_270
 */
void Pwm_Hal_ClearDetectEventFlag
(
    Pwm_Hal_InstanceType Instance
);

/**
 * @brief Enable write protection
 * @details This function enables or disables write protection for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Enable TRUE to enable write protection, FALSE to disable
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Write protection is enabled or disabled
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_266
 */
void Pwm_Hal_EnableWriteProtection
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
);

/**
 * @brief Set debug mode
 * @details This function configures the debug mode for the specified PWM module.
 *
 * @param[in] Instance PWM instance
 * @param[in] Mode PWM debug mode
 *            - PWM_DEBUG_MODE_NO_EFFECT
 *            - PWM_DEBUG_MODE_COUNTER_STOPPED_OUTPUT_PREVIOUS
 *            - PWM_DEBUG_MODE_COUNTER_STOPPED_OUTPUT_HIGH
 *
 * @return void
 *
 * @pre The PWM module should be initialized
 * @post Debug mode is configured
 *
 * @note Reentrant: No
 *       DES ID: DES_PWM_API_267
 */
void Pwm_Hal_SetDebugMode
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_DebugModeType Mode
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
/* =============================================  EOF  ============================================== */
#endif   /* PWM_HAL_H */
