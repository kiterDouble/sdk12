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
 * @file AC784xx_Rcm_Reg.h
 *
 * @brief This file provides extern Low level Rcm register api.
 *
 */

#ifndef AC784XX_RCM_REG_H
#define AC784XX_RCM_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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
 * @brief Enable or disable global reset interrupt.
 * @note Function ID: DES_MCU_API_306
 * @param[in] En: enable state
 *            - true: enable global reset interrupt
 *            - false: disable global reset interrupt
 * @return void
 */
LOCAL_INLINE void Rcm_Reg_EnableGlobalResetInterrupt(boolean En)
{
    MODIFY_REG32(CKGEN->RCM_CTRL, CKGEN_RCM_CTRL_RST_GLB_INT_EN_Msk, CKGEN_RCM_CTRL_RST_GLB_INT_EN_Pos, (uint32)En);
}

/*!
 * @brief Set reset sources trigger interrupt.
 * @note Function ID: DES_MCU_API_307
 * @param[in] RstSources: reset sources which can trigger RCM interrupt
 * @return void
 */
LOCAL_INLINE void Rcm_Reg_EnableResetInterrupts(uint32 RstSources)
{
    MODIFY_REG32(CKGEN->RCM_CTRL, (0xFFFFU << CKGEN_RCM_CTRL_SW_RST_INT_EN_Pos), CKGEN_RCM_CTRL_SW_RST_INT_EN_Pos, \
                 RstSources);
}

/*!
 * @brief Get reset sources trigger interrupt.
 * @note Function ID: DES_MCU_API_308
 * @return reset sources which can trigger RCM interrupt
 */
LOCAL_INLINE uint32 Rcm_Reg_GetResetInterruptSources(void)
{
    return READ_BIT32(CKGEN->RCM_CTRL, (0xFFFFU << CKGEN_RCM_CTRL_SW_RST_INT_EN_Pos)) >> \
           CKGEN_RCM_CTRL_SW_RST_INT_EN_Pos;
}

/*!
 * @brief Set reset delay time.
 * @note Function ID: DES_MCU_API_309
 * @param[in] Time: system reset delay time set
 * @return void
 */
LOCAL_INLINE void Rcm_Reg_SetResetDelayTime(uint32 Time)
{
    MODIFY_REG32(CKGEN->RCM_CTRL, CKGEN_RCM_CTRL_RST_DLY_TIME_Msk, CKGEN_RCM_CTRL_RST_DLY_TIME_Pos, Time);
}

/*!
 * @brief Set ext reset filter value.
 * @note Function ID: DES_MCU_API_310
 * @param[in] FilterVal: filter value: 0~127, means filter width value is N * 128KHz.
 * @return void
 */
LOCAL_INLINE void Rcm_Reg_SetExtResetFilter(uint32 FilterVal)
{
    MODIFY_REG32(CKGEN->RCM_CTRL, CKGEN_RCM_CTRL_EXT_RST_FILTER_VAL_Msk, CKGEN_RCM_CTRL_EXT_RST_FILTER_VAL_Pos, \
                 FilterVal);
}

/*!
 * @brief Enable or disable external reset filter.
 * @note Function ID: DES_MCU_API_311
 * @param[in] IsEnable: enable or disable external reset filter
 *            - true: enable ext reset filter
 *            - false: disable ext reset filter
 * @return void
 */
LOCAL_INLINE void Rcm_Reg_EnableExtResetFilter(boolean IsEnable)
{
    uint32 Enable = (TRUE == IsEnable) ? 1UL : 0UL;
    MODIFY_REG32(CKGEN->RCM_CTRL, CKGEN_RCM_CTRL_EXT_RST_FILTER_EN_Msk, CKGEN_RCM_CTRL_EXT_RST_FILTER_EN_Pos, Enable);
}

/*!
 * @brief Set reset sources trigger reset.
 * @note Function ID: DES_MCU_API_312
 * @param[in] RstSources: reset sources which can trigger RCM reset
 * @return void
 */
LOCAL_INLINE void Rcm_Reg_EnableResetSources(uint32 RstSources)
{
    MODIFY_REG32(CKGEN->RCM_EN, (0xFFFFU << CKGEN_RCM_EN_SW_RST_EN_Pos), CKGEN_RCM_EN_SW_RST_EN_Pos, RstSources);
}

/*!
 * @brief Get reset sources.
 * @note Function ID: DES_MCU_API_313
 * @return reset sources which can trigger RCM reset.
 */
LOCAL_INLINE uint32 Rcm_Reg_GetResetSources(void)
{
    return READ_BIT32(CKGEN->RCM_EN, (0xFFFFU << CKGEN_RCM_EN_SW_RST_EN_Pos)) >> CKGEN_RCM_EN_SW_RST_EN_Pos;
}

/*!
 * @brief Assert or deassert the reset.
 * @note Function ID: DES_MCU_API_315
 * @param[in] ResetId: Reset ID
 * @param[in] ResetState: Reset state, assert or deassert
 * @return void
 */
LOCAL_INLINE void Rcm_Reg_SetResetState(uint32 ResetId, uint32 ResetState)
{
    uint32 SetVal;
    uint32 Base = CKGEN_BASE + 0x20UL + (uint32)(ResetId / 32U * 4U);
    uint32 Pos = (uint32)(ResetId % 32U);
    uint32 Msk = 1UL << Pos;

    SetVal = ((READ_MEM32(Base)) & (~((uint32)Msk))) | (((uint32)ResetState) << Pos);
    WRITE_MEM32(Base, SetVal);
}

/*!
 * @brief Get reset and interrupt status.
 * @note Function ID: DES_MCU_API_317
 * @return value of reset and interrupt status
 */
LOCAL_INLINE uint32 Rcm_Reg_GetResetStatus(void)
{
    return READ_REG32(CKGEN->RCM_STATUS);
}

/*!
 * @brief Get reset and interrupt status.
 * @note Function ID: DES_MCU_API_317
 * @return value of reset and interrupt status
 *         - BIT0:RCM_RESET_INT_STATUS_SW_RST
 *         - BIT1:RCM_RESET_INT_STATUS_LOCKUP_RST
 *         - BIT2:RCM_RESET_INT_STATUS_ACK_ERR_RST
 *         - BIT3:RCM_RESET_INT_STATUS_WDG_RST
 *         - BIT5:RCM_RESET_INT_STATUS_XOSC_LOSS_RST
 *         - BIT6:RCM_RESET_INT_STATUS_PLL_UNLOCK_RST
 *         - BIT7:RCM_RESET_INT_STATUS_VHSI_LOSS_RST
 *         - BIT9:RCM_RESET_INT_STATUS_SMU_ERR_RST
 */
LOCAL_INLINE uint32 Rcm_Reg_GetResetInterruptStatus(void)
{
    return (((READ_REG32(CKGEN->RCM_STATUS)) & 0x0000FFFFUL));
}

/*!
 * @brief Clear all reset interrupt status.
 * @note Function ID: DES_MCU_API_318
 * @return void
 */
LOCAL_INLINE void Rcm_Reg_ClearResetInterruptStatus(void)
{
    CKGEN->RCM_STATUS |= CKGEN_RCM_STATUS_RST_INT_STATUS_CLR_Msk;
}

/*!
 * @brief Clear all reset status.
 * @note Function ID: DES_MCU_API_319
 * @return void
 */
LOCAL_INLINE void Rcm_Reg_ClearResetStatus(void)
{
    CKGEN->RCM_STATUS |= CKGEN_RCM_STATUS_RST_STATUS_CLR_Msk;
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* AC784XX_RCM_REG_H */
