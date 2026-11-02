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
 * @file Esm_Hal.h
 *
 * @brief This file provides extern Hal Esm api.
 *
 */

#ifndef ESM_HAL_H
#define ESM_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
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
 * @brief Initialize ESM module.
 * @note  Function ID: DES_ESM_API_001
 * @warning This function should be called at privileged mode.
 * @param[in] ConfigPtr: pointer of ESM driver configuration.
 * @return void
 */
void Esm_Hal_Init(const Esm_ConfigType *ConfigPtr);

/*!
 * @brief De-Initialize ESM module.
 * @note  Function ID: DES_ESM_API_004
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Esm_Hal_DeInit(void);

/*!
 * @brief Get current ESM error Information.
 * @note  Function ID: DES_ESM_API_002
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: ESM channel.
 * @param[out] ErrorPtr:  Pointer to where to store the error information of ESM.
 * @return error status.
 *            -STATUS_SUCCESS: no error occurred.
 *            -STATUS_ERROR: error detected.
 */
Hal_StatusType Esm_Hal_GetErrorInfo(Esm_ChannelType Channel, Esm_ErrorInfoType *ErrorPtr);

/*!
 * @brief Clear ESM error information.
 * @note  Function ID: DES_ESM_API_003
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Esm_Hal_ClearErrorInfo(Esm_ChannelType Channel);

#if defined (AC7843X)
/*!
 * @brief Get current E2E ECC error Information.
 * @note  Function ID: DES_ESM_API_006
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: E2E EIM channel.
 * @param[out] ErrorPtr: Pointer to where to store the error information of E2E ECC.
 * @return error status.
 *            -STATUS_SUCCESS: no error occurred.
 *            -STATUS_ERROR: error detected.
 */
Hal_StatusType Esm_Hal_GetE2EErrorInfo(Eim_E2EChannelType Channel, Esm_ErrorInfoType *ErrorPtr);

/*!
 * @brief Clear E2E ECC error information.
 * @note  Function ID: DES_ESM_API_007
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: E2E EIM channel.
 * @return void
 */
void Esm_Hal_ClearE2EErrorInfo(Eim_E2EChannelType Channel);
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* ESM_HAL_H */
/*============================================EOF===================================================*/
