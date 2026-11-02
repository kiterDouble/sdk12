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
 * @file Ctu_Hal.h
 *
 * @brief ctu hal define.
 */

#ifndef CTU_HAL_H
#define CTU_HAL_H
#ifdef __cplusplus
extern "C" {
#endif /* endif of __cplusplus */
/* ===========================================  INCLUDE FILES  =========================================== */
#include "AC784xx_Ctu_Reg.h"
/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */
/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/*!
 * @brief Ctu mapping configure structure
 */
typedef struct
{
    Ctu_TargetModuleType Module;/* register value */
    Ctu_TriggerSourceType Source;
} Ctu_MappingCfgType;

/*!
 * @brief Ctu configure structure
 */
typedef struct
{
    boolean EnLock;
    uint8 CfgCnt;
    const Ctu_MappingCfgType *MappingCfgs;
} Ctu_CfgType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/
/* ========================================== LOCAL VARIABLES =========================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/**
 * @brief Initialize module trigger source
 * @note Function ID: DES_MCL_API_201
 * @param[in] CfgPtr: the pointer to the Ctu_CfgType structure
 * @return void
 */
void Ctu_Hal_Init(const Ctu_CfgType *CfgPtr);

/**
 * @brief Set module trigger source
 * @note Function ID: DES_MCL_API_202
 * @param[in] Module: target module
 * @param[in] Source: trigger source
 * @return Hal_StatusType: set success or not
 */
Hal_StatusType Ctu_Hal_SetModuleTriggerSource(Ctu_TargetModuleType Module, Ctu_TriggerSourceType Source);

/**
 * @brief Get module trigger source
 * @note Function ID: DES_MCL_API_203
 * @param[in] Module: module id
 * @return Ctu_TriggerSourceType: module trigger source
 */
Ctu_TriggerSourceType Ctu_Hal_GetModuleTriggerSource(Ctu_TargetModuleType Module);

/**
 * @brief enable/disbale software trigger source
 * @note Function ID: DES_MCL_API_204
 * @param[in] Id: software trigger source id
 * @param[in] Enable: enable/disable software trigger
 * @return void
 */
void Ctu_Hal_EnableSWTrigger(Ctu_SWTriggerIdType Id, boolean Enable);

/**
 * @brief enable module function
 * @note Function ID: DES_MCL_API_205
 * @param[in] FuncId: module function id
 * @param[in] Enable: enable or disable
 * @return void
 */
void Ctu_Hal_EnableFunc(Ctu_FuncIdType FuncId, boolean Enable);

/**
 * @brief Deinit ctu
 * @note Function ID: DES_MCL_API_206
 * @return void
 */
void Ctu_Hal_DeInit(void);

#ifdef __cplusplus
}
#endif /* endif of __cplusplus */
#endif /* CTU_HAL_H */
/* =============================================  EOF  ============================================== */
