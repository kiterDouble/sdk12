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
 * @file Eim_Hal.h
 *
 * @brief This file provides extern Hal Eim api.
 *
 */

#ifndef EIM_HAL_H
#define EIM_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Eim_Hal_Types.h"
#include "Esm_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Lock EIM.
 * @note  Function ID: DES_EIM_API_001
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Eim_Hal_Lock(void);

/*!
 * @brief Unlock EIM.
 * @note  Function ID: DES_EIM_API_002
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Eim_Hal_Unlock(void);

/*!
 * @brief Setup EIM configuration.
 * @note  Function ID: DES_EIM_API_003
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: EIM channel.
 * @param[in] ConfigPtr: pointer of EIM channel configuration.
 * @return void
 */
void Eim_Hal_SetupConfig(Eim_ChannelType Channel, const Eim_ChannelConfigType *ConfigPtr);

/*!
 * @brief Inject Fault to test ECC.
 * @note  Function ID: DES_EIM_API_004
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: EIM channel.
 * @param[in] Offset: offset address to inject fault.
 * @return void
 */
uint32 Eim_Hal_InjectFault(Eim_ChannelType Channel, uint32 Offset);

/*!
 * @brief  Clear fault to test ECC.
 * @note  Function ID: DES_EIM_API_005
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: EIM channel.
 * @return void
 */
void Eim_Hal_ClearFault(Eim_ChannelType Channel);

#if defined (AC7843X)
/*!
 * @brief Setup E2E EIM configuration.
 * @note  Function ID: DES_EIM_API_006
 * @warning This function should be called at privileged mode.
 * @param[in] ConfigPtr: pointer of E2E EIM channel configuration.
 * @return void
 */
void Eim_Hal_SetupE2EConfig(const Eim_ChannelConfigType *ConfigPtr);

/*!
 * @brief Inject Fault to test E2E ECC.
 * @note  Function ID: DES_EIM_API_007
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: E2E EIM channel.
 * @return void
 */
void Eim_Hal_InjectE2EFault(Eim_E2EChannelType Channel);

/*!
 * @brief  Clear fault to test E2E ECC.
 * @note  Function ID: DES_EIM_API_008
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Eim_Hal_ClearE2EFault(void);
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* EIM_HAL_H */
