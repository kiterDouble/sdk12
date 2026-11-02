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

/*!
 * @file Core_Hal.h
 *
 * @brief This file provides extern Hal Core api.
 *
 */

#ifndef CORE_HAL_H
#define CORE_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Mcm_Reg.h"
#include "Core_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief Enable an interrupt in NVIC.
 *
 * @details
 * Enables the specified IRQ in the Nested Vectored Interrupt Controller (NVIC)
 * and clears any pending flag prior to enabling. This is a low-level HAL
 * operation that directly manipulates NVIC state.
 *
 * @param[in] IrqNumber: IRQ number to enable.
 * @return void
 * @pre NVIC and core clock infrastructure must be available.
 * @post The specified IRQ will be enabled and not pending.
 * @note Function ID: DES_CORE_API_200
 */
void Core_Hal_EnableIrq(IRQn_Type IrqNumber);

/**
 * @brief Disable an interrupt in NVIC.
 *
 * @details
 * Disables the specified IRQ in the NVIC to prevent the interrupt from
 * being taken. This function does not alter pending or active flags.
 *
 * @param[in] IrqNumber: IRQ number to disable.
 * @return void
 * @pre Interrupts may be disabled globally only if permitted by caller.
 * @post The specified IRQ will be disabled.
 * @note Function ID: DES_CORE_API_201
 */
void Core_Hal_DisableIrq(IRQn_Type IrqNumber);

/**
 * @brief Get IRQ enabled status.
 *
 * @details
 * Returns whether the specified IRQ is enabled in the NVIC.
 *
 * @param[in] IrqNumber: IRQ number to query.
 * @return boolean: IRQ enable status.
 *          - FALSE: Interrupt is disabled.
 *          - TRUE: Interrupt is enabled.
 * @pre NVIC implemented and accessible.
 * @post No side effects.
 * @note Function ID: DES_CORE_API_202
 */
boolean Core_Hal_IsIrqEnable(IRQn_Type IrqNumber);

/**
 * @brief Set IRQ priority.
 *
 * @details
 * Configure the priority of the given IRQ in the NVIC. Priority encoding
 * follows the platform interrupt controller convention.
 *
 * @param[in] IrqNumber: IRQ number to configure.
 * @param[in] Priority: Priority value to set (numeric, lower is higher priority
 *                      depending on platform encoding).
 * @return void
 * @pre Caller must ensure Priority is within valid range for the device.
 * @post IRQ priority is updated.
 * @note Function ID: DES_CORE_API_203
 */
void Core_Hal_SetIrqPriority(IRQn_Type IrqNumber, uint32 Priority);

/**
 * @brief Get IRQ priority.
 *
 * @details
 * Returns the current priority of the specified IRQ from the NVIC.
 *
 * @param[in] IrqNumber: IRQ number to query.
 * @return uint32: IRQ priority value.
 * @pre NVIC accessible.
 * @post No side effects.
 * @note Function ID: DES_CORE_API_204
 */
uint32 Core_Hal_GetIrqPriority(IRQn_Type IrqNumber);

/**
 * @brief Perform a system software reset.
 *
 * @details
 * Triggers a system reset via the NVIC system control block. This function
 * will not return on many platforms as the system resets immediately.
 *
 * @return void
 * @pre All critical data must be saved by caller prior to calling.
 * @post System will reset shortly after call.
 * @note Function ID: DES_CORE_API_205
 */
void Core_Hal_PerformReset(void);

/**
 * @brief Set an IRQ as pending.
 *
 * @details
 * Sets the pending flag for the specified IRQ in NVIC, causing the
 * interrupt to be serviced according to priority and masking.
 *
 * @param[in] IrqNumber: IRQ number to set pending.
 * @return void
 * @pre NVIC supports pending flags.
 * @post The specified IRQ will be marked pending.
 * @note Function ID: DES_CORE_API_206
 */
void Core_Hal_SetPendingIrq(IRQn_Type IrqNumber);

/**
 * @brief Clear pending IRQ flag.
 *
 * @details
 * Clears the pending status of the specified IRQ in the NVIC.
 *
 * @param[in] IrqNumber: IRQ number to clear pending flag.
 * @return void
 * @pre None.
 * @post Pending flag for the IRQ will be cleared.
 * @note Function ID: DES_CORE_API_207
 */
void Core_Hal_ClearPendingIrq(IRQn_Type IrqNumber);

/**
 * @brief Get IRQ pending status.
 *
 * @details
 * Returns whether the specified IRQ is currently pending in the NVIC.
 *
 * @param[in] IrqNumber: IRQ number to query.
 * @return boolean: Pending status. FALSE = not pending, TRUE = pending.
 * @pre NVIC accessible.
 * @post No side effects.
 * @note Function ID: DES_CORE_API_208
 */
boolean Core_Hal_IsIrqPending(IRQn_Type IrqNumber);

/**
 * @brief Get IRQ active status.
 *
 * @details
 * Returns whether the specified IRQ is currently active (being serviced)
 * in the NVIC.
 *
 * @param[in] IrqNumber: IRQ number to query.
 * @return boolean: Active status. FALSE = not active, TRUE = active.
 * @pre NVIC accessible.
 * @post No side effects.
 * @note Function ID: DES_CORE_API_209
 */
boolean Core_Hal_IsIrqActive(IRQn_Type IrqNumber);

/**
 * @brief Set IRQ priority grouping.
 *
 * @details
 * Configures the priority grouping in the NVIC, affecting how preemption
 * and subpriority are interpreted.
 *
 * @param[in] PriorityGroup: Priority grouping value.
 * @return void
 * @pre Caller must ensure PriorityGroup is valid for the platform.
 * @post NVIC priority grouping updated.
 * @note Function ID: DES_CORE_API_210
 */
void Core_Hal_SetIrqPriorityGrouping(uint32 PriorityGroup);

/**
 * @brief Get IRQ priority grouping.
 *
 * @details
 * Returns the current NVIC priority grouping configuration.
 *
 * @return uint32: Priority grouping value.
 * @pre NVIC accessible.
 * @post No side effects.
 * @note Function ID: DES_CORE_API_211
 */
uint32 Core_Hal_GetIrqPriorityGrouping(void);

/**
 * @brief Set processor priority mask.
 *
 * @details
 * Sets the processor priority mask (PRIMASK/BASEPRI depending on platform)
 * to control interrupt preemption at CPU level.
 *
 * @param[in] PriorityMAsk: Priority mask value.
 * @return void
 * @pre Caller must understand global interrupt masking implications.
 * @post Processor priority mask updated.
 * @note Function ID: DES_CORE_API_231
 */
void Core_Hal_SetPriorityMask(uint32 PriorityMAsk);

/**
 * @brief Get processor priority mask.
 *
 * @details
 * Returns the current processor priority mask value.
 *
 * @return uint32: Priority mask value.
 * @pre None.
 * @post No side effects.
 * @note Function ID: DES_CORE_API_232
 */
uint32 Core_Hal_GetPriorityMask(void);

/**
 * @brief Retrieve device UUID.
 *
 * @details
 * Reads the unique device identifier from device-specific registers and
 * stores four 32-bit words into the provided buffer.
 *
 * @param[out] uuidBuffer: Pointer to buffer sized at least 4 words to receive
 *                        the UUID.
 * @return void
 * @pre uuidBuffer must not be NULL.
 * @post uuidBuffer is populated with device UUID words.
 * @note Function ID: DES_CORE_API_212
 */
void Core_Hal_GetUUID(uint32 *uuidBuffer);

/**
 * @brief Get device chip ID.
 *
 * @details
 * Reads the chip identification register and returns a numeric chip ID.
 *
 * @return uint64: Chip ID value.
 * @pre None.
 * @post No side effects.
 * @note Function ID: DES_CORE_API_213
 */
uint64 Core_Hal_GetChipID(void);

/**
 * @brief Enable or disable Non-Maskable Interrupt (NMI).
 *
 * @details
 * Enables or disables NMI functionality after proper pinmux/configuration
 * has been applied. The behavior is hardware dependent.
 *
 * @param[in] En: TRUE to enable NMI, FALSE to disable.
 * @return void
 * @pre Pin multiplexing and required hardware configuration must be set.
 * @post NMI enable state changed according to En.
 * @note Function ID: DES_CORE_API_214
 */
void Core_Hal_EnableNMI(boolean En);

#if (__FPU_PRESENT == 1U) && (__FPU_USED == 1U)
/**
 * @brief Get FPU error information.
 *
 * @details
 * Reads FPU exception status flags and returns error information if any
 * floating point exceptions have occurred since last clear.
 *
 * @param[in,out] ErrorPtr: Pointer to structure to receive FPU error info.
 * @return Hal_StatusType: STATUS_SUCCESS if no error, STATUS_ERROR if error(s) found.
 * @pre FPU present and used on platform.
 * @post FPU error data may be returned in ErrorPtr when not NULL.
 * @note Function ID: DES_CORE_API_215
 */
Hal_StatusType Core_Hal_GetFpuErrorInfo(Core_FpuErrorInfoType *ErrorPtr);

/**
 * @brief Clear FPU error information and related flags.
 *
 * @details
 * Clears FPU status registers and any saved exception state to prevent
 * repeated FPU interrupts.
 *
 * @return void
 * @pre FPU present and used on platform.
 * @post FPU exception flags cleared.
 * @note Function ID: DES_CORE_API_216
 */
void Core_Hal_ClearFpuErrorInfo(void);

#endif

/**
 * @brief Get MBIST execution status.
 *
 * @details
 * Queries the memory built-in self-test (MBIST) status from the MCM
 * status registers and maps platform-specific codes to Core_BistStatusType.
 *
 * @return Core_BistStatusType: BIST execution status.
 *               - BIST_NORUN: BIST not run
 *               - BIST_OK: BIST success
 *               - BIST_ERROR: BIST fault occurred
 *               - BIST_BUSY: BIST running/unknown
 * @pre MBIST hardware present.
 * @post No side effects.
 * @note Function ID: DES_CORE_API_217
 */
Core_BistStatusType Core_Hal_GetMBistExecStatus(void);

#if defined(AC7840X) || defined(AC7842X)
/**
 * @brief Get flash cache ECC error details.
 *
 * @details
 * Reads flash cache ECC status and address information, returning details
 * if an ECC error occurred.
 *
 * @param[out] ErrorPtr: Pointer to structure to receive ECC error info.
 * @return Hal_StatusType: STATUS_SUCCESS if no error, STATUS_ERROR if error(s) found.
 * @pre Flash cache present on device.
 * @post ErrorPtr populated when not NULL.
 * @note Function ID: DES_CORE_API_218
 */
Hal_StatusType Core_Hal_GetFlashCacheErrorInfo(Core_EccErrorInfoType *ErrorPtr);

/**
 * @brief Clear flash cache ECC error indicators.
 *
 * @details
 * Clears ECC status bits for the flash cache to acknowledge and reset error
 * conditions.
 *
 * @return void
 * @pre Flash cache ECC error status may be set.
 * @post ECC status cleared.
 * @note Function ID: DES_CORE_API_219
 */
void Core_Hal_ClearFlashCacheErrorInfo(void);

#endif /* AC7840X AC7842X */

#if defined (AC7843X)
/**
 * @brief Enable or disable CPU instruction cache.
 *
 * @details
 * Enables or disables CPU instruction cache. Hardware initialization or
 * cache configuration may be required prior to use.
 *
 * @param[in] En: TRUE to enable instruction cache, FALSE to disable.
 * @return void
 * @pre Caller must ensure cache control is supported on the device.
 * @post Instruction cache enabled/disabled according to En.
 * @note Function ID: DES_CORE_API_220
 */
void Core_Hal_EnableICache(boolean En);

/**
 * @brief Invalidate CPU instruction cache.
 *
 * @details
 * Invalidates the entire instruction cache to ensure coherency with memory
 * after code/data updates.
 *
 * @return void
 * @pre Caller should ensure system state allows cache invalidation.
 * @post Instruction cache invalidated.
 * @note Function ID: DES_CORE_API_221
 */
void Core_Hal_InvalidICache(void);

/**
 * @brief Get instruction cache ECC error information.
 *
 * @details
 * Reads instruction cache ECC status and returns any error detail information
 * available from cache controller registers.
 *
 * @param[out] ErrorPtr: Pointer to structure to receive ECC error info.
 * @return Hal_StatusType: STATUS_SUCCESS if no error, STATUS_ERROR otherwise.
 * @pre Instruction cache present and ECC supported.
 * @post ErrorPtr populated when not NULL.
 * @note Function ID: DES_CORE_API_222
 */
Hal_StatusType Core_Hal_GetICacheErrorInfo(Core_EccErrorInfoType *ErrorPtr);

/**
 * @brief Clear instruction cache ECC error indicators.
 *
 * @details
 * Clears ECC error flags for the instruction cache controller.
 *
 * @return void
 * @pre ECC error flags may be set.
 * @post ECC flags cleared.
 * @note Function ID: DES_CORE_API_223
 */
void Core_Hal_ClearICacheErrorInfo(void);

/**
 * @brief Enable or disable CPU data cache.
 *
 * @details
 * Enables or disables CPU data cache. Proper cache maintenance (invalidate/
 * clean) may be required around enabling/disabling to maintain coherency.
 *
 * @param[in] En: TRUE to enable data cache, FALSE to disable.
 * @return void
 * @pre Caller must ensure cache control is supported.
 * @post Data cache enabled/disabled according to En.
 * @note Function ID: DES_CORE_API_224
 */
void Core_Hal_EnableDCache(boolean En);

/**
 * @brief Invalidate CPU data cache.
 *
 * @details
 * Invalidates the entire data cache to ensure data coherency with memory
 * after DMA or direct memory modifications.
 *
 * @return void
 * @pre Caller must ensure no critical data will be lost during invalidation.
 * @post Data cache invalidated.
 * @note Function ID: DES_CORE_API_225
 */
void Core_Hal_InvalidDCache(void);

/**
 * @brief Get data cache ECC error information.
 *
 * @details
 * Reads data cache ECC status and provides error details if any ECC errors
 * are reported by the data cache controller.
 *
 * @param[out] ErrorPtr: Pointer to structure to receive ECC error info.
 * @return Hal_StatusType: STATUS_SUCCESS if no error, STATUS_ERROR otherwise.
 * @pre Data cache present and ECC supported.
 * @post ErrorPtr populated when not NULL.
 * @note Function ID: DES_CORE_API_226
 */
Hal_StatusType Core_Hal_GetDCacheErrorInfo(Core_EccErrorInfoType *ErrorPtr);

/**
 * @brief Clear data cache ECC error indicators.
 *
 * @details
 * Clears ECC error flags reported by the data cache controller.
 *
 * @return void
 * @pre ECC error flags may be set.
 * @post ECC flags cleared.
 * @note Function ID: DES_CORE_API_227
 */
void Core_Hal_ClearDCacheErrorInfo(void);
#endif /* AC7843X */

/**
 * @brief Initialize the MCM/Core HAL module.
 *
 * @details
 * Configures core-level interrupt sources and internal state according to
 * provided configuration structure. Stores the configuration pointer for
 * runtime usage (e.g. callback invocation).
 *
 * @param[in] ConfigPtr: Pointer to core configuration structure.
 * @return void
 * @pre ConfigPtr must be a valid pointer.
 * @post Core HAL internal configuration applied and stored.
 * @note Function ID: DES_CORE_API_228
 */
void Core_Hal_Init(const Core_ConfigType *ConfigPtr);

/**
 * @brief De-initialize the MCM/Core HAL module.
 *
 * @details
 * Disables configured core interrupts and clears internal configuration
 * state stored during initialization.
 *
 * @return void
 * @pre Core_Hal_Init has been called previously.
 * @post Core HAL internal state cleared and interrupts disabled.
 * @note Function ID: DES_CORE_API_229
 */
void Core_Hal_DeInit(void);

/**
 * @brief Initialize DWT cycle counter.
 *
 * @details
 * Enables DWT trace and cycle counting for performance measurements and
 * profiling. This sets up the cycle counter and enables counting.
 *
 * @return void
 * @pre Core debug and DWT units must be present and accessible.
 * @post DWT cycle counter enabled and cleared.
 * @note Function ID: DES_CORE_API_233
 */
void Core_Hal_InitDWT(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* CORE_HAL_H */
