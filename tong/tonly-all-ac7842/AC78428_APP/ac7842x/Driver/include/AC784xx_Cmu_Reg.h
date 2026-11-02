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
 * @file AC784xx_Cmu_Reg.h
 *
 * @brief cmu access register inline function definition.
 *
 */

#ifndef AC784XX_CMU_REG_H
#define AC784XX_CMU_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
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

/* =========================================== LOCAL VARIABLES ============================================== */
/*!
 * @brief cmu base address array
 */
static CMU_Type *const CmuBase[CMU_INSTANCE_MAX] = CMU_BASE_PTRS;

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/**
 * @brief Set cmu window length
 * @note Function ID: DES_MCU_API_420
 * @param[in] CmuIdx: index in the CmuBase
 * @param[in] WinLen: window length
 * @return void
 */
LOCAL_INLINE void Cmu_Reg_SetWindowLen(uint8 CmuIdx, uint16 WinLen)
{
    if (CmuIdx < CMU_INSTANCE_MAX)
    {
        WRITE_REG32(CmuBase[CmuIdx]->RCCR, WinLen);
    }
}

/**
 * @brief Get cmu window length
 * @note Function ID: DES_MCU_API_421
 * @param[in] CmuIdx: index in the CmuBase
 * @return cmu window length
 */
LOCAL_INLINE uint16 Cmu_Reg_GetWindowLen(uint8 CmuIdx)
{
    uint16 WinLen = 0U;

    if (CmuIdx < CMU_INSTANCE_MAX)
    {
        WinLen = (uint16)(READ_REG32(CmuBase[CmuIdx]->RCCR));
    }

    return WinLen;
}

/**
 * @brief Set cmu high threshold
 * @note Function ID: DES_MCU_API_422
 * @param[in] CmuIdx: index in the CmuBase
 * @param[in] Threshold: Threshold value
 * @return void
 */
LOCAL_INLINE void Cmu_Reg_SetHighThreshold(uint8 CmuIdx, uint32 Threshold)
{
    if (CmuIdx < CMU_INSTANCE_MAX)
    {
        WRITE_REG32(CmuBase[CmuIdx]->HTCR, Threshold);
    }
}

/**
 * @brief Get cmu high threshold value
 * @note Function ID: DES_MCU_API_423
 * @param[in] CmuIdx: index in the CmuBase
 * @return cmu high threshold value
 */
LOCAL_INLINE uint32 Cmu_Reg_GetHighThreshold(uint8 CmuIdx)
{
    uint32 Threshold = 0U;

    if (CmuIdx < CMU_INSTANCE_MAX)
    {
        Threshold = READ_REG32(CmuBase[CmuIdx]->HTCR);
    }

    return Threshold;
}

/**
 * @brief Set cmu low threshold
 * @note Function ID: DES_MCU_API_424
 * @param[in] CmuIdx: index in the CmuBase
 * @param[in] Threshold: Threshold value
 * @return void
 */
LOCAL_INLINE void Cmu_Reg_SetLowThreshold(uint8 CmuIdx, uint32 Threshold)
{
    if (CmuIdx < CMU_INSTANCE_MAX)
    {
        WRITE_REG32(CmuBase[CmuIdx]->LTCR, Threshold);
    }
}

/**
 * @brief Get cmu low threshold value
 * @note Function ID: DES_MCU_API_425
 * @param[in] CmuIdx: index in the CmuBase
 * @return cmu low threshold value
 */
LOCAL_INLINE uint32 Cmu_Reg_GetLowThreshold(uint8 CmuIdx)
{
    uint32 Threshold = 0U;

    if (CmuIdx < CMU_INSTANCE_MAX)
    {
        Threshold = READ_REG32(CmuBase[CmuIdx]->LTCR);
    }

    return Threshold;
}

/**
 * @brief Enable cmu
 * @note Function ID: DES_MCU_API_426
 * @param[in] CmuIdx: index in the CmuBase
 * @param[in] IsEnable: true enable cmu; false disable cmu
 * @return void
 */
LOCAL_INLINE void Cmu_Reg_Enable(uint8 CmuIdx, boolean IsEnable)
{
    uint32 Value = (TRUE == IsEnable) ? 1U : 0U;

    if (CmuIdx < CMU_INSTANCE_MAX)
    {
        WRITE_REG32(CmuBase[CmuIdx]->CR, Value);
    }
}

/**
 * @brief Get cmu Enable
 * @note Function ID: DES_MCU_API_427
 * @param[in] CmuIdx: index in the CmuBase
 * @return cmu is enable
 */
LOCAL_INLINE boolean Cmu_Reg_GetEnable(uint8 CmuIdx)
{
    boolean IsEnable = FALSE;

    if (CmuIdx < CMU_INSTANCE_MAX)
    {
        IsEnable = (1U == (READ_REG32(CmuBase[CmuIdx]->CR))) ? TRUE : FALSE;
    }

    return IsEnable;
}

/**
 * @brief Get cmu status
 * @note Function ID: DES_MCU_API_428
 * @param[in] CmuIdx: index in the CmuBase
 * @return cmu status
 */
LOCAL_INLINE uint8 Cmu_Reg_GetStatus(uint8 CmuIdx)
{
    uint8 Status = 0U;

    if (CmuIdx < CMU_INSTANCE_MAX)
    {
        Status = (uint8)(READ_REG32(CmuBase[CmuIdx]->SR));
    }

    return Status;
}

/**
 * @brief Clear cmu status
 * @note Function ID: DES_MCU_API_429
 * @param[in] CmuIdx: index in the CmuBase
 * @return void
 */
LOCAL_INLINE void Cmu_Reg_ClearStatus(uint8 CmuIdx)
{
    if (CmuIdx < CMU_INSTANCE_MAX)
    {
        SET_BIT32(CmuBase[CmuIdx]->SR, (CMU_SR_FLL_Msk | CMU_SR_FHH_Msk));
    }
}

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* AC784XX_CMU_REG_H */

/* =============================================  EOF  ============================================== */
