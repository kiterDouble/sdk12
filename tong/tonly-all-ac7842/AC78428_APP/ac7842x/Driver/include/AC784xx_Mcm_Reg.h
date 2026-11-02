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
 * @file AC784xx_Mcm_Reg.h
 *
 * @brief This file provides extern Low level Mcm register api.
 *
 */

#ifndef AC784XX_MCM_REG_H
#define AC784XX_MCM_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Get MCM exceptions.
 * @note      Function ID: DES_STP_API_532
 * @param[in] none

 * @return MCM exceptions.
 */
LOCAL_INLINE uint32 Mcm_Reg_GetInterruptStatus(void)
{
    /*cstat !MISRAC2012-Rule-11.4 The cast is needed to get the register address.*/
    return READ_BIT32(MCM->MISCR, 0x0000FFFFU);
}

/*!
 * @brief Set which MCM exception trigger interrupt.
 * @note      Function ID: DES_STP_API_533
 * @param[in] IntSources: MCM exception sources which can trigger MCM interrupt.

 * @return none
 */
LOCAL_INLINE void Mcm_Reg_SetInterruptSources(uint32 IntSources)
{
    /*cstat !MISRAC2012-Rule-11.4 The cast is needed to get the register address.*/
    MODIFY_REG32(MCM->MISCR, 0xFFFF0000U, 16U, IntSources);
}

/*!
 * @brief Get MCM exception sources trigger interrupt.
 * @note      Function ID: DES_STP_API_534
 * @param[in] none

 * @return MCM exception sources which can trigger Mcm interrupt.
 */
LOCAL_INLINE uint32 Mcm_Reg_GetInterruptSources(void)
{
    /*cstat !MISRAC2012-Rule-11.4 The cast is needed to get the register address.*/
    return READ_BIT32(MCM->MISCR, 0xFFFF0000U) >> 16U;
}

/*!
 * @brief Get MBIST status.
 * @note      Function ID: DES_STP_API_538
 * @param[in] none

 * @return MBIST status.
 */
LOCAL_INLINE uint32 Mcm_Reg_GetMbistStatus(void)
{
    /*cstat !MISRAC2012-Rule-11.4 The cast is needed to get the register address.*/
    return READ_REG32(MCM->MBIST);
}

#if defined(AC7840X) || defined(AC7842X)
/*!
 * @brief Get cache error status.
 * @note      Function ID: DES_STP_API_539
 * @param[in] none

 * @return Cache error status.
 */
LOCAL_INLINE uint32 Mcm_Reg_GetFlashCacheEccErrorStatus(void)
{
    /*cstat !MISRAC2012-Rule-11.4 The cast is needed to get the register address.*/
    return READ_REG32(MCM->LMPEIR);
}

/*!
 * @brief Clear cache error status.
 * @note      Function ID: DES_STP_API_540
 * @param[in] none

 * @return none.
 */
LOCAL_INLINE void Mcm_Reg_ClearFlashCacheEccError(void)
{
    /*cstat !MISRAC2012-Rule-11.4 The cast is needed to get the register address.*/
    WRITE_REG32(MCM->LMPEIR, 0xFFFFFFFFU);
}

/*!
 * @brief Get cache error address.
 * @note      Function ID: DES_STP_API_541
 * @param[in] none

 * @return Cache error address.
 */
LOCAL_INLINE uint32 Mcm_Reg_GetFlashCacheEccErrorAddress(void)
{
    /*cstat !MISRAC2012-Rule-11.4 The cast is needed to get the register address.*/
    return READ_REG32(MCM->LMFAR);
}
#endif

/*!
 * @brief Enable or Disable NMI.
 * @note      Function ID: DES_STP_API_541
 * @param[in] Enable: NMI enable status

 * @return NMI enable status.
 */
LOCAL_INLINE void Mcm_Reg_EnableNmi(uint32 Enable)
{
    /*cstat !MISRAC2012-Rule-11.4 The cast is needed to get the register address.*/
    if (Enable != 0U)
    {
        SET_BIT32(MCM->MNCR, MCM_MNCR_NMI_EN_Msk);
    }
    else
    {
        CLEAR_BIT32(MCM->MNCR, MCM_MNCR_NMI_EN_Msk);
    }
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* AC784XX_MCM_REG_H */
