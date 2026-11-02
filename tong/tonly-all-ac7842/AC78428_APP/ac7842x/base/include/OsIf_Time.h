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
#ifndef OSIF_TIME_H
#define OSIF_TIME_H

/**
*   @file OsIf_Time.h
*   @brief This file provides OsIf Time API.
*
*/

#ifdef __cplusplus
extern "C" {
#endif /* endif of __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Std_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/
#define OSIF_TIME_SW_MAJOR_VERSION             (1U)
#define OSIF_TIME_SW_MINOR_VERSION             (0U)
#define OSIF_TIME_SW_PATCH_VERSION             (0U)

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief Get the current value of the counter.
 * @note Function ID: DES_OSIF_API_207
 * @return The current value of the counter
 */
uint32 OsIf_GetCounter(void);

/**
 * @brief Get the delta time in ticks compared to a reference, and updates the reference.
 * @note Function ID: DES_OSIF_API_206
 * @param[inout] CurrentRef:  reference counter value, updated to current counter value
 * @return The elapsed time
 */
uint32 OsIf_GetElapsed(uint32 *const CurrentRef);

/**
 * @brief Converts a value from microsecond units to ticks units.
 * @note Function ID: DES_OSIF_API_204
 * @param[in] Micros:  microseconds value (multiple of 1000 can be convert to tick)
 * @return uint32: ticks value
 */
uint32 OsIf_MicrosToTicks(uint32 Micros);

/**
 * @brief Microseconds delay.
 * @note Function ID: DES_OSIF_API_205
 * @param[in] Micros:  microseconds value
 * @return void
 */
void OsIf_UDelay(uint32 Micros);

/*========================================GLOBAL FUNCTIONS==========================================*/

#ifdef __cplusplus
}
#endif /* endif of __cplusplus */

#endif /* OSIF_TIME_H */
