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
 * @file Cmu_Hal.h
 *
 * @brief cmu hal define.
 */

#ifndef CMU_HAL_H
#define CMU_HAL_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Cmu_Reg.h"
#include "Cmu_Hal_Types.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief Initialize CMU module
 * @note Function ID: DES_MCU_API_301
 * @param[in] CmuId: CMU module id
 * @param[in] CtrlParam: the pointer to the Cmu_CtrlParamType structure
 * @return Hal_StatusType: Initialize success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Cmu_Hal_Init(Cmu_ModuleIdType CmuId, const Cmu_CtrlParamType *CtrlParam);

/**
 * @brief Deinitialize CMU module
 * @note Function ID: DES_MCU_API_302
 * @param[in] CmuId: CMU module id
 * @return void
 */
void Cmu_Hal_Deinit(Cmu_ModuleIdType CmuId);

/**
 * @brief Enable Cmu
 * @note Function ID: DES_MCU_API_303
 * @param[in] CmuId: CMU module id
 * @param[in] IsEnable: true enable cmu module, false disable cmu module
 * @return Hal_StatusType: Enable success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Cmu_Hal_Enable(Cmu_ModuleIdType CmuId, boolean IsEnable);

/**
 * @brief Clear Cmu error flag
 * @note Function ID: DES_MCU_API_304
 * @param[in] CmuId: CMU module id
 * @return void
 */
void Cmu_Hal_ClearErrStatus(Cmu_ModuleIdType CmuId);

/**
 * @brief Get the error information of CMU
 * @note Function ID: DES_MCU_API_305
 * @param[in] CmuId: CMU module id
 * @param[out] ErrInfo: the pointer to the Cmu_ErrInfoType structure, save error message
 * @return Hal_StatusType: Is there an error, STATUS_SUCCESS stands for no error; STATUS_ERROR is a error
 */
Hal_StatusType Cmu_Hal_GetErrInfo(Cmu_ModuleIdType CmuId, Cmu_ErrInfoType *ErrInfo);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CMU_HAL_H */

/* =============================================  EOF  ============================================== */
