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
 * @file AC784xx_Pbr_Reg.h
 *
 * @brief pbr access register inline function definition.
 *
 */

#ifndef AC784XX_PBR_REG_H
#define AC784XX_PBR_REG_H

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
#define PBR_PACRA           (PBR_BASE + 0x10U)
/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/**
 * @brief Set value to peripheral access permission configuration register
 * @note Function ID: DES_PBR_API_010
 * @param[in] PeriphIdx: peripheral idx
 * @param[in] Value: the value being set
 * @return void
 */
LOCAL_INLINE void Pbr_Reg_SetPeriphAccess(uint32 PeriphIdx, uint8 Value)
{
    uint32 RegAddr = PBR_PACRA + (PeriphIdx / 8U * 0x04U);
    uint32 Pos = (PeriphIdx % 8U) * 0x04U;
    uint32 Msk = 7UL << Pos;

    MODIFY_MEM32(RegAddr, Msk, Pos, Value);/*cstat !MISRAC2012-Rule-11.4*/
}

/**
 * @brief Get peripheral access permission configuration register value
 * @note Function ID: DES_PBR_API_011
 * @param[in] PeriphIdx: peripheral idx
 * @param[in] Value: the value being set
 * @return uint8: access permission register value
 */
LOCAL_INLINE uint8 Pbr_Reg_GetPeriphAccess(uint32 PeriphIdx)
{
    uint32 RegAddr = PBR_PACRA + (PeriphIdx / 8U * 0x04U);
    uint32 Pos = (PeriphIdx % 8U) * 0x04U;
    uint32 Msk = 7UL << Pos;
    uint32 RegVal = READ_MEM32(RegAddr);/*cstat !MISRAC2012-Rule-11.4*/

    return ((uint8)((RegVal & Msk) >> Pos));
}

/**
 * @brief Set master privilege configuration register value
 * @note Function ID: DES_PBR_API_012
 * @param[in] MasterIdx: master idx
 * @param[in] Value: the value being set
 * @return void
 */
LOCAL_INLINE void Pbr_Reg_SetMasterPrivilege(uint32 MasterIdx, uint8 Value)
{
    uint32 RegAddr = PBR_BASE + (MasterIdx * 4U);

    MODIFY_MEM32(RegAddr, 7U, 0U, Value);/*cstat !MISRAC2012-Rule-11.4*/
}

/**
 * @brief Get master privilege configuration register value
 * @note Function ID: DES_PBR_API_013
 * @param[in] MasterIdx: master idx
 * @return uint8: privilege register value
 */
LOCAL_INLINE uint8 Pbr_Reg_GetMasterPrivilege(uint32 MasterIdx)
{
    uint32 RegAddr = PBR_BASE + (MasterIdx * 4U);
    uint32 RegVal = READ_MEM32(RegAddr);/*cstat !MISRAC2012-Rule-11.4*/

    return ((uint8)(RegVal & 7U));
}

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* AC784XX_PBR_REG_H */

/* =============================================  EOF  ============================================== */
