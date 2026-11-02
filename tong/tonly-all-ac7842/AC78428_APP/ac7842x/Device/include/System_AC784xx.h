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
 * @file System_AC784xx.h
 *
 * @brief This file provides system clock config integration api.
 *
 */

#ifndef SYSTEM_AC784XX_H
#define SYSTEM_AC784XX_H

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Types.h"

/* ============================================  DEFINES AND MACROS  ============================================ */
#define STARTUP_HAL_SW_MAJOR_VERSION        28
#define STARTUP_HAL_SW_MINOR_VERSION        0
#define STARTUP_HAL_SW_PATCH_VERSION        0
/* ============================================= TYPEDEFS ================================================ */
typedef enum {
    NON_DEV = 0,
    CSE_DEV = 1,
    FLS_DEV = 2
} FlashDeviceType;

/* =========================================== LOCAL VARIABLES ============================================== */
/**
 * @brief System Clock Frequency (Core Clock)
 */
extern uint32 SystemCoreClock;

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/**
 * @brief Update system core clock.
 * @note  Function ID: DES_BOOT_API_001
 * @return void
 */
void SystemCoreClockUpdate(void);

/**
 * @brief Setup the microcontroller system. Initialize the System.
 * @note  Function ID: DES_BOOT_API_000
 * @return void
 */
void SystemInit(void);

/*!
 * @brief Initialize the SRAM.
 * @note  Function ID: DES_BOOT_API_002
 * @param[in] StackBase: Main stack start address.
 * @param[in] StackEnd:  Main stack end address.
 * @return void
 */
void SystemInitRam(uint32 StackBase, uint32 StackEnd);

/*!
 * @brief DFlash lock.
 * @note  Function ID: DES_BOOT_API_010
 * @param[in] Device_ID: Device ID for access flash memory
 * @return FlashDeviceType : NON_DEV / CSE_DEV / FLS_DEV
 */
FlashDeviceType System_FlsDeviceTryLock(FlashDeviceType Device_Type);

/*!
 * @brief DFlash unlock
 * @note  Function ID: DES_BOOT_API_011
 * @param[in] Device_ID: Device ID for access flash memory
 * @return FlashDeviceType
 */
FlashDeviceType System_FlsDeviceUnlock(FlashDeviceType Device_Type);

/*!
 * @brief DFlash free lock
 * @note  Function ID: DES_BOOT_API_012
 * @param[in] Device_ID: Device ID for access flash memory
 * @return FlashDeviceType
 */
FlashDeviceType System_FlsDeviceFreelock(FlashDeviceType Device_Type);

/**
 * @brief set a memory to value
 * @note  Function ID : DES_BOOT_API_014
 * @param[in] Addr: dest memory addr
 * @param[in] Val: set val
 * @param[in] Len: len
 * @return dest address
 */
void *System_Memset(void *Addr, uint8 Val, uint32 Len);

/**
 * @brief copy src addr to dest addr
 * @note  Function ID : DES_BOOT_API_013
 * @param[in] Dest: dest  addr
 * @param[in] Src: src addr
 * @param[in] Len: copy len
 * @return dest address
 */
void *System_Memcpy(void *Dest, const void *Src, uint32 Len);
/* =====================================  Functions definition  ===================================== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SYSTEM_AC784XX_H */

/* =============================================  EOF  ============================================== */
