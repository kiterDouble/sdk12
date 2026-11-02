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
 * @file Smu_Hal.h
 *
 * @brief This file provides extern Hal Smu api.
 *
 */

#ifndef SMU_HAL_H
#define SMU_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Smu_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Lock SMU registers write permission.
 * @note  Function ID: DES_SMU_API_001
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Smu_Hal_Lock(void);

/*!
 * @brief Unlock SMU registers write permission.
 * @note  Function ID: DES_SMU_API_002
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Smu_Hal_Unlock(void);

/*!
 * @brief Init SMU driver.
 * @note  Function ID: DES_SMU_API_003
 * @warning This function should be called at privileged mode.
 * @param[in] ConfigPtr: point of SMU driver configuration.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 */
Hal_StatusType Smu_Hal_Init(const Smu_ConfigType *ConfigPtr);

/*!
 * @brief De-Initialze SMU module.
 * @note  Function ID: DES_SMU_API_004
 * @warning This function should be called at privileged mode.
 * @param[in] void.
 * @return void
 */
void Smu_Hal_DeInit(void);

/*!
 * @brief Get current SMU count, single point fault, latent fault values.
 * @note  Function ID: DES_SMU_API_005
 * @warning This function should be called at privileged mode.
 * @param[out] FaultPtr: pointer of current SMU status.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 *            -STATUS_SMU_FAULT_OCCURRED: Fault has been detected in current.
 */
Hal_StatusType Smu_Hal_GetCurrentFaults(Smu_FaultInfoType *FaultPtr);

/*!
 * @brief  Get last SMU count, single point fault, latent fault values.
 * @note  Function ID: DES_SMU_API_006
 * @warning This function should be called at privileged mode.
 * @param[out] FaultPtr: pointer of last SMU status.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 *            -STATUS_SMU_FAULT_OCCURRED: Fault has been detected at last boot.
 */
Hal_StatusType Smu_Hal_GetLastFaults(Smu_FaultInfoType *FaultPtr);

/*!
 * @brief Clear current SMU count, single point fault, latent fault values.
 * @note  Function ID: DES_SMU_API_007
 * @warning This function should be called at privileged mode.
 * @param[in] Reset: Reset SMU count to zero.
 * @return void
 */
void Smu_Hal_ClearCurrentFaultInfo(boolean Reset);

/*!
 * @brief Assert a software single point or latent fault to SMU.
 * @note  Function ID: DES_SMU_API_008
 * @warning This function should be called at privileged mode.
 * @param[in] FaultType: Fault type, software single point or latent fault.
 * @param[in] FaultId: Fault ID.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 */
Hal_StatusType Smu_Hal_AssertSwFault(Smu_FaultType FaultType, uint8 FaultId);

/*!
 * @brief Deassert a software single point or latent fault to SMU.
 * @note  Function ID: DES_SMU_API_009
 * @warning This function should be called at privileged mode.
 * @param[in] FaultType: Fault type, software single point or latent fault.
 * @param[in] FaultId: Fault ID.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 */
Hal_StatusType Smu_Hal_DeassertSwFault(Smu_FaultType FaultType, uint8 FaultId);

/*!
 * @brief Inject a hardware fault to SMU.
 * @note  Function ID: DES_SMU_API_010
 * @warning This function should be called at privileged mode.
 * @param[in] FaultType: Fault type, Hardware single point or latent fault.
 * @param[in] FaultId: Fault ID.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 */
Hal_StatusType Smu_Hal_InjectFault(Smu_FaultType FaultType, uint8 FaultId);

/*!
 * @brief Clear a hardware fault to SMU.
 * @note  Function ID: DES_SMU_API_011
 * @warning This function should be called at privileged mode.
 * @param[in] FaultType: Fault type, Hardware single point or latent fault.
 * @param[in] FaultId: Fault ID.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 */
Hal_StatusType Smu_Hal_ClearFault(Smu_FaultType FaultType, uint8 FaultId);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* SMU_HAL_H */
