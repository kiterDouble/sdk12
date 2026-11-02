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
 * @file AC784xx_Ctu_Reg.h
 *
 * @brief ctu access register inline function definition.
 *
 */

#ifndef AC784XX_CTU_REG_H
#define AC784XX_CTU_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*PRQA S ALL --*/
/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
#define CTU_REG_BASE           (CTU_BASE)
/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/**
 * @brief enable/disable function
 * @note Function ID: DES_MCL_API_210
 * @param[in] FuncId: function id
 * @param[in] Enable: TRUE or FALSE, enable or disable function
 * @return void
 */
LOCAL_INLINE void Ctu_Reg_SetFunc(uint32 FuncId, boolean Enable)
{
    uint32 RegAddr = CTU_REG_BASE + 0x5cU + (FuncId / 32U * 0x04U);
    uint32 Pos = FuncId % 32U;
    uint32 Msk = 1UL << Pos;
    uint32 Value = (TRUE == Enable) ? 1U : 0U;

    MODIFY_MEM32(RegAddr, Msk, Pos, Value);/*cstat !MISRAC2012-Rule-11.4*/
}

/**
 * @brief Set module trigger source
 * @note Function ID: DES_MCL_API_211
 * @param[in] Module: target module
 * @param[in] Source: trigger source
 * @return void
 */
LOCAL_INLINE void Ctu_Reg_SetModuleTrig(uint32 Module, uint32 Source)
{
    uint32 RegAddr = CTU_REG_BASE + (Module / 4U * 0x04U);
    uint32 Pos = (Module % 4U) * 0x08U;
    uint32 Msk = 127UL << Pos;

    MODIFY_MEM32(RegAddr, Msk, Pos, Source);/*cstat !MISRAC2012-Rule-11.4*/
}

/**
 * @brief Get module trigger source
 * @note Function ID: DES_MCL_API_212
 * @param[in] Module: module id
 * @return uint32 : trigger source
 */
LOCAL_INLINE uint32 Ctu_Reg_GetModuleTrig(uint32 Module)
{
    uint32 RegAddr = CTU_REG_BASE + (Module / 4U * 0x04U);
    uint32 Pos = (Module % 4U) * 0x08U;
    uint32 Msk = 127UL << Pos;
    uint32 RegVal = READ_MEM32(RegAddr);

    return ((RegVal & Msk) >> Pos);
}

/**
 * @brief Enable module lock
 * @note Function ID: DES_MCL_API_213
 * @param[in] ModuleIdx: module idx
 * @param[in] Enable: TRUE or FALSE, enable or disable lock
 * @return void
 */
LOCAL_INLINE void Ctu_Reg_EnableModuleLock(uint32 ModuleIdx, boolean Enable)
{
    uint32 RegAddr = CTU_REG_BASE + (ModuleIdx * 0x04U);
    uint32 Pos = 31U;
    uint32 Msk = 1UL << Pos;
    uint32 Value = (TRUE == Enable) ? 1U : 0U;

    MODIFY_MEM32(RegAddr, Msk, Pos, Value);/*cstat !MISRAC2012-Rule-11.4*/
}

/**
 * @brief Get module lock
 * @note Function ID: DES_MCL_API_214
 * @param[in] ModuleIdx: module idx
 * @return uint32: register lock, value is 0 or 1
 */
LOCAL_INLINE uint32 Ctu_Reg_GetModuleLock(uint32 ModuleIdx)
{
    uint32 RegAddr = CTU_REG_BASE + (ModuleIdx / 4U * 0x04U);
    uint32 Pos = 31U;
    uint32 Msk = 1UL << Pos;
    uint32 RegVal = READ_MEM32(RegAddr);

    return ((RegVal & Msk) >> Pos);
}

#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
/**
 * @brief set adc0 inter
 * @note Function ID: DES_MCL_API_215
 * @param[in] Value: register value
 * @return void
 */
LOCAL_INLINE void Ctu_Reg_SetAdc0Inter(uint32 Value)
{
    MODIFY_REG32(CTU->ADC_SYNC, CTU_ADC_SYNC_ADC0_INTER_Msk, CTU_ADC_SYNC_ADC0_INTER_Pos, Value);
}

/**
 * @brief set adc1 inter
 * @note Function ID: DES_MCL_API_216
 * @param[in] Value: register value
 * @return void
 */
LOCAL_INLINE void Ctu_Reg_SetAdc1Inter(uint32 Value)
{
    MODIFY_REG32(CTU->ADC_SYNC, CTU_ADC_SYNC_ADC1_INTER_Msk, CTU_ADC_SYNC_ADC1_INTER_Pos, Value);
}
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* AC784XX_CTU_REG_H */

/* =============================================  EOF  ============================================== */
