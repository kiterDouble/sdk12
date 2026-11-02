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
 * @file Eio_Common_Hal.h
 *
 * @brief This file provides eio comm integration functions interfaces.
 *
 */

#ifndef EIO_COMMON_HAL_H
#define EIO_COMMON_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================  Includes  =========================================== */
#include "Eio_Hal_Types.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */

/* ==========================================  Variables  =========================================== */
/**
 * @brief Table of base addresses for EIO instances.
 * @note DES ID: DES_EIO_VAR_008
 */
extern EIO_Type * const EioBase[EIO_INSTANCE_COUNT];

/**
 * @brief EIO clock identifiers.
 * @note DES ID: DES_EIO_VAR_009
 */
extern const Ckgen_ClkIdType Eio_HalClock[EIO_INSTANCE_COUNT];

/**
 * @brief EIO DMA request sources for each shifter.
 * @note DES ID: DES_EIO_VAR_010
 */
extern const Dma_RequestSourceType EioDMASrc[EIO_INSTANCE_COUNT][EIO_MAX_SHIFTER_COUNT];

/* ====================================  Functions declaration  ===================================== */
/**
 * @brief Initialize the EIO device.
 *
 * @details
 * This function initializes the EIO peripheral hardware. It enables the peripheral
 * clock, performs a hardware reset, enables the EIO module, and initializes the
 * device state structure. This function is idempotent and can be called multiple
 * times safely.
 *
 * @param[in] Instance: EIO peripheral instance number.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than EIO_INSTANCE_COUNT.
 *
 * @post
 *     EIO device is initialized and ready for driver allocation.
 *     EIO interrupt is enabled.
 *     Device state structure is initialized.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     DES ID: DES_EIO_API_004
 */
void Eio_Hal_InitDevice(uint8 Instance);

/**
 * @brief Deinitialize the EIO device.
 *
 * @details
 * This function deinitializes the EIO peripheral hardware. It performs a hardware
 * reset, disables the EIO interrupt, disables the peripheral clock, and marks the
 * device as uninitialized. This function only executes if no resources are currently
 * allocated on the device.
 *
 * @param[in] Instance: EIO peripheral instance number.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than EIO_INSTANCE_COUNT.
 *     No driver instances should be using the device (ResourceAllocation must be 0).
 *
 * @post
 *     EIO device is deinitialized.
 *     EIO interrupt is disabled.
 *     Peripheral clock is disabled.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     DES ID: DES_EIO_API_005
 */
void Eio_Hal_DeinitDevice(uint8 Instance);

/**
 * @brief Reset the EIO device to default state.
 *
 * @details
 * This function performs a software reset of the EIO peripheral, restoring all
 * registers to their default values. This resets all shifters, timers, and control
 * registers.
 *
 * @param[in] Instance: EIO peripheral instance number.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than EIO_INSTANCE_COUNT.
 *
 * @post
 *     All EIO registers are reset to default values.
 *     All shifters and timers are disabled.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     DES ID: DES_EIO_API_006
 */
void Eio_Hal_Reset(uint8 Instance);

/**
 * @brief Initialize an EIO driver instance.
 *
 * @details
 * This function allocates the EIO resources (shifters and timers) specified by
 * Driver->ResourceIndex and Driver->ResourceCount, and initializes them to a
 * clean disabled state. The caller must set ResourceIndex to the desired first
 * resource index before calling this function.
 *
 * @param[in] Instance: EIO peripheral instance number.
 * @param[in] Driver: Pointer to the EIO common driver context structure.
 *                    Driver->ResourceIndex must be set to the desired first resource index.
 *                    Driver->ResourceCount must be set to the required number of resources.
 *
 * @return Hal_StatusType
 *     STATUS_SUCCESS: Driver initialized successfully.
 *     STATUS_ERROR: Requested resources are already in use.
 *
 * @pre
 *     Instance must be valid and less than EIO_INSTANCE_COUNT.
 *     Driver pointer must be valid.
 *     Driver->ResourceIndex must be set to the desired shifter/timer index.
 *     Driver->ResourceCount must be set to the required number of resources.
 *     Driver->ResourceIndex + Driver->ResourceCount must not exceed EIO_MAX_SHIFTER_COUNT.
 *     EIO device must be initialized via Eio_Hal_InitDevice().
 *
 * @post
 *     Requested resources are allocated and marked as busy.
 *     Driver is registered in device state.
 *     All allocated resources are initialized and disabled.
 *     Driver->Instance is set to the device instance.
 *
 * @note
 *     Reentrancy: Reentrant for different instances, non-reentrant for the same instance.
 *     DES ID: DES_EIO_API_007
 */
Hal_StatusType Eio_Hal_InitDriver(uint8 Instance, Eio_CommonStateType *Driver);

/**
 * @brief Deinitialize an EIO driver instance.
 *
 * @details
 * This function deallocates EIO resources (shifters and timers) that were previously
 * allocated to a driver instance. It removes the driver from the device state and
 * frees the resources for reuse by other drivers.
 *
 * @param[in] Driver: Pointer to the EIO common driver context structure.
 *
 * @return void
 *
 * @pre
 *     Driver pointer must be valid.
 *     Driver must have been previously initialized via Eio_Hal_InitDriver().
 *
 * @post
 *     Driver is removed from device state.
 *     Resources are deallocated and marked as available.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_EIO_API_008
 */
void Eio_Hal_DeinitDriver(const Eio_CommonStateType *Driver);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EIO_COMMON_HAL_H */

/* =============================================  EOF  ============================================== */
