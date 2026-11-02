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
 * @file AC784xx_Smu_Reg.h
 *
 * @brief This file provides extern Low level Smu register api.
 *
 */

/*PRQA S 0306 EOF */

#ifndef AC784XX_SMU_REG_H
#define AC784XX_SMU_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/*===============================================  INCLUDE FILES  ============================================*/
#include "Device_Register.h"

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/*!
 * @brief Single point fault register access permission lock.
 * @note      Function ID: DES_SMU_API_100
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SPFaultRegLock(void)
{
    WRITE_REG32(SMU->LKSEQ0, 0U);
}

/*!
 * @brief Single point fault register access permission unlock.
 * @note      Function ID: DES_SMU_API_101
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SPFaultRegUnlock(void)
{
    WRITE_REG32(SMU->LKSEQ0, SMU_UNLOCK_FIRST_VALUE);

    WRITE_REG32(SMU->LKSEQ0, SMU_UNLOCK_SECOND_VALUE);
}
/*!
 * @brief Latent fault register access permission lock.
 * @note      Function ID: DES_SMU_API_102
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_LatentFaultRegLock(void)
{
    WRITE_REG32(SMU->LKSEQ1, 0U);
}

/*!
 * @brief Latent fault register access permission unlock.
 * @note      Function ID: DES_SMU_API_103
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_LatentFaultRegUnlock(void)
{
    WRITE_REG32(SMU->LKSEQ1, SMU_UNLOCK_SECOND_VALUE);

    WRITE_REG32(SMU->LKSEQ1, SMU_UNLOCK_FIRST_VALUE);
}

/*!
 * @brief Enable which single point fault channels could trigger reset.
 * @note      Function ID: DES_SMU_API_104
 * @warning This function should be called at privileged mode.
 * @param[in] SpfChannels: single fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetSPFaultResetEnable(uint32 SpfChannels)
{
    WRITE_REG32(SMU->SFRSTEN, SpfChannels);
}

/*!
 * @brief Get which single point fault channel reset.
 * @note      Function ID: DES_SMU_API_105
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return single point fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetSPFaultResetEnable(void)
{
    return READ_REG32(SMU->SFRSTEN);
}

/*!
 * @brief Enable which latent fault channels could reset.
 * @note      Function ID: DES_SMU_API_106
 * @param[in] LfChannels: latent fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetLatentFaultResetEnable(uint32 LfChannels)
{
    WRITE_REG32(SMU->LFRSTEN, LfChannels);
}

/*!
 * @brief Get which latent fault channel reset function.
 * @note      Function ID: DES_SMU_API_107
 * @param[in] void
 * @return latent fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetLatentFaultResetEnable(void)
{
    return READ_REG32(SMU->LFRSTEN);
}

/*!
 * @brief Set software single point fault channel function.
 * @note      Function ID: DES_SMU_API_108
 * @param[in] SwSpfChannels: software single point fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetSoftwareSPFault(uint32 SwSpfChannels)
{

    WRITE_REG32(SMU->SWSFE, SwSpfChannels);
}

/*!
 * @brief Set software single point fault channel function.
 * @note      Function ID: DES_SMU_API_109
 * @param[in] void
 * @return software single point fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetSoftwareSPFault(void)
{
    return READ_REG32(SMU->SWSFE);
}

/*!
 * @brief Set software single point fault shadow channels function.
 * @note      Function ID: DES_SMU_API_110
 * @param[in] void
 * @return software single point fault shadow channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetSoftwareSPFaultShadow(void)
{
    return READ_REG32(SMU->SWSFES);
}

/*!
 * @brief Set software latent fault channel function.
 * @note      Function ID: DES_SMU_API_111
 * @param[in] SwLfChannels: software latent fault channel.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetSoftwareLatentFault(uint32 SwLfChannels)
{
    WRITE_REG32(SMU->SWLFE, SwLfChannels);
}

/*!
 * @brief Get software latent fault channel function.
 * @note      Function ID: DES_SMU_API_112
 * @param[in] void
 * @return software latent fault channel.
 */
LOCAL_INLINE uint32 Smu_Reg_GetSoftwareLatentFault(void)
{
    return READ_REG32(SMU->SWLFE);
}

/*!
 * @brief Get software latent fault shadow channel function.
 * @note      Function ID: DES_SMU_API_113
 * @param[in] void
 * @return software latent fault shadow channel.
 */
LOCAL_INLINE uint32 Smu_Reg_GetSoftwareLatentFaultShadow(void)
{
    return READ_REG32(SMU->SWLFES);
}

/*!
 * @brief Set SMU reset counter value.
 * @note      Function ID: DES_SMU_API_114
 * @param[in] Val: reset counter value which SHALL be 0~15.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetResetCounterThreshold(uint8 Val)
{
    WRITE_REG32(SMU->SRSTCNTVAL, Val);
}

/*!
 * @brief Set SMU reset counter value.
 * @note      Function ID: DES_SMU_API_115
 * @param[in] void
 * @return reset counter value which SHALL be 0~15.
 */
LOCAL_INLINE uint8 Smu_Reg_GetResetCounterThreshold(void)
{
    return (uint8)READ_REG32(SMU->SRSTCNTVAL);
}

/*!
 * @brief Get SMU single point fault status.
 * @note      Function ID: DES_SMU_API_116
 * @param[in] void
 * @return SMU single point fault status.
 */
LOCAL_INLINE uint32 Smu_Reg_GetSPFaultStatus(void)
{
    return READ_REG32(SMU->SFES);
}

/*!
 * @brief Set SMU single point fault status.
 * @note      Function ID: DES_SMU_API_117
 * @param[in] SpfChannels: single fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetSPFaultStatus(uint32 SpfChannels)
{
    WRITE_REG32(SMU->SFES, SpfChannels);
}

/*!
 * @brief Get SMU single point fault status.
 * @note      Function ID: DES_SMU_API_118
 * @param[in] void
 * @return SMU single point fault status.
 */
LOCAL_INLINE uint32 Smu_Reg_GetSPFaultStatusShadow(void)
{
    return READ_REG32(SMU->SFESS);
}

/*!
 * @brief Set SMU single point fault status.
 * @note      Function ID: DES_SMU_API_119
 * @param[in] SpfChannels: single fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetSPFaultStatusShadow(uint32 SpfChannels)
{
    WRITE_REG32(SMU->SFESS, SpfChannels);
}

/*!
 * @brief Get SMU latent fault status.
 * @note      Function ID: DES_SMU_API_120
 * @param[in] void
 * @return SMU latent fault status.
 */
LOCAL_INLINE uint32 Smu_Reg_GetLatentFaultStatus(void)
{
    return READ_REG32(SMU->LFES);
}

/*!
 * @brief Set SMU latent fault status.
 * @note      Function ID: DES_SMU_API_121
 * @param[in] LfChannels: latent fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetLatentFaultStatus(uint32 LfChannels)
{

    WRITE_REG32(SMU->LFES, LfChannels);
}

/*!
 * @brief Get SMU latent fault status.
 * @note      Function ID: DES_SMU_API_122
 * @param[in] void
 * @return SMU latent fault status.
 */
LOCAL_INLINE uint32 Smu_Reg_GetLatentFaultStatusShadow(void)
{
    return READ_REG32(SMU->LFESS);
}

/*!
 * @brief Set SMU latent fault status.
 * @note      Function ID: DES_SMU_API_123
 * @param[in] LfChannels: latent fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetLatentFaultStatusShadow(uint32 LfChannels)
{
    WRITE_REG32(SMU->LFESS, LfChannels);
}

/*!
 * @brief Get SMU reset counter status.
 * @note      Function ID: DES_SMU_API_124
 * @param[in] void
 * @return SMU reset counter.
 */
LOCAL_INLINE uint8 Smu_Reg_GetResetCounter(void)
{
    return (uint8)READ_REG32(SMU->SRSTCNT);
}

/*!
 * @brief Clear SMU reset counter
 * @note      Function ID: DES_SMU_API_125
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_ClearResetCounter(void)
{
    /* Clear SMU reset counter */
    WRITE_REG32(SMU->SRSTCNT, 0xFFFFFFFFU);
}

/*!
 * @brief Get SMU reset counter shadow status.
 * @note      Function ID: DES_SMU_API_126
 * @param[in] void
 * @return SMU reset counter shadow
 */
LOCAL_INLINE uint8 Smu_Reg_GetResetCounterShadow(void)
{
    return (uint8)READ_REG32(SMU->SRSTCNTS);
}

/*!
 * @brief Clear SMU reset counter shadow.
 * @note      Function ID: DES_SMU_API_127
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_ClearResetCounterShadow(void)
{
    /* Clear SMU reset counter shadow */
    WRITE_REG32(SMU->SRSTCNTS, 0xFFFFFFFFU);
}

/*!
 * @brief Set SMU patch check0.
 * @note      Function ID: DES_SMU_API_128
 * @param[in] SpfChannels: Hardware and software Single point fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetPathCheck0(uint32 SpfChannels)
{
    WRITE_REG32(SMU->PATHCHK0, SpfChannels);
}

/*!
 * @brief Get SMU reset patch check0.
 * @note      Function ID: DES_SMU_API_129
 * @param[in] void
 * @return Hardware and software Single point fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetPathCheck0(void)
{
    return READ_REG32(SMU->PATHCHK0);
}

/*!
 * @brief Set SMU patch check1.
 * @note      Function ID: DES_SMU_API_130
 * @param[in] LfChannels: Hardware and software Single point fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetPathCheck1(uint32 LfChannels)
{
    WRITE_REG32(SMU->PATHCHK1, LfChannels);
}

/*!
 * @brief Get SMU reset patch check1.
 * @note      Function ID: DES_SMU_API_131
 * @param[in] void
 * @return Hardware and software Single point fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetPathCheck1(void)
{
    return READ_REG32(SMU->PATHCHK1);
}

#if defined (AC7840E)
/*!
 * @brief Enable FSP function.
 * @note      Function ID: DES_SMU_API_136
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetFspEnable(void)
{
    SET_BIT32(SMU->FSPCR, SMU_FSPCR_FSPEN_Msk);
}

/*!
 * @brief Disable FSP function.
 * @note      Function ID: DES_SMU_API_137
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetFspDisable(void)
{
    CLEAR_BIT32(SMU->FSPCR, SMU_FSPCR_FSPEN_Msk);
}

/*!
 * @brief Enable FSP0 output.
 * @note      Function ID: DES_SMU_API_138
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetFspOut0Enable(void)
{
    SET_BIT32(SMU->FSPCR, SMU_FSPCR_FSPOUT0EN_Msk);
}

/*!
 * @brief Disable FSP0 output.
 * @note      Function ID: DES_SMU_API_139
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetFspOut0Disable(void)
{
    CLEAR_BIT32(SMU->FSPCR, SMU_FSPCR_FSPOUT0EN_Msk);
}

/*!
 * @brief Enable FSP1 output.
 * @note      Function ID: DES_SMU_API_140
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetFspOut1Enable(void)
{
    SET_BIT32(SMU->FSPCR, SMU_FSPCR_FSPOUT1EN_Msk);
}

/*!
 * @brief Disable FSP1 output.
 * @note      Function ID: DES_SMU_API_141
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetFspOut1Disable(void)
{
    CLEAR_BIT32(SMU->FSPCR, SMU_FSPCR_FSPOUT1EN_Msk);
}

/*!
 * @brief Set FSP mode control.
 * @note      Function ID: DES_SMU_API_142
 * @param[in] Mode: FSP mode, 0 or 1.
 *                  1: Fault state FSP0=1, FSP1=0; Non-fault state is opposite.
 *                  0: Fault state FSP0=0, FSP1=1; Non-fault state is opposite.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetFspMode(uint8 Mode)
{
    if (Mode != 0U)
    {
        SET_BIT32(SMU->FSPCR, SMU_FSPCR_FSPMODE_Msk);
    }
    else
    {
        CLEAR_BIT32(SMU->FSPCR, SMU_FSPCR_FSPMODE_Msk);
    }
}

/*!
 * @brief Get FSP mode control.
 * @note      Function ID: DES_SMU_API_143
 * @param[in] void
 * @return FSP mode: 0 or 1.
 */
LOCAL_INLINE uint8 Smu_Reg_GetFspMode(void)
{
    return (uint8)((READ_REG32(SMU->FSPCR) & SMU_FSPCR_FSPMODE_Msk) >> SMU_FSPCR_FSPMODE_Pos);
}

/*!
 * @brief Release FSP fault state to non-fault state.
 * @note      Function ID: DES_SMU_API_144
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetFspRelease(void)
{
    SET_BIT32(SMU->FSPCR, SMU_FSPCR_FSPRELEASE_Msk);
}

/*!
 * @brief Get FSP state.
 * @note      Function ID: DES_SMU_API_145
 * @param[in] void
 * @return FSP state: 1 for fault state, 0 for non-fault state.
 */
LOCAL_INLINE uint8 Smu_Reg_GetFspState(void)
{
    return (uint8)((READ_REG32(SMU->FSPCR) & SMU_FSPCR_FSPSTATE_Msk) >> SMU_FSPCR_FSPSTATE_Pos);
}

/*!
 * @brief Set FSP control register.
 * @note      Function ID: DES_SMU_API_146
 * @param[in] Value: FSP control register value.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetFspControl(uint32 Value)
{
    WRITE_REG32(SMU->FSPCR, Value);
}

/*!
 * @brief Get FSP control register.
 * @note      Function ID: DES_SMU_API_147
 * @param[in] void
 * @return FSP control register value.
 */
LOCAL_INLINE uint32 Smu_Reg_GetFspControl(void)
{
    return READ_REG32(SMU->FSPCR);
}

/*!
 * @brief Enable which single point fault channels could trigger FSP output.
 * @note      Function ID: DES_SMU_API_148
 * @param[in] SpfChannels: single point fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetSPFaultFspEnable(uint32 SpfChannels)
{
    WRITE_REG32(SMU->FSPSFCR, SpfChannels);
}

/*!
 * @brief Get which single point fault channels could trigger FSP output.
 * @note      Function ID: DES_SMU_API_149
 * @param[in] void
 * @return single point fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetSPFaultFspEnable(void)
{
    return READ_REG32(SMU->FSPSFCR);
}

/*!
 * @brief Enable which latent fault channels could trigger FSP output.
 * @note      Function ID: DES_SMU_API_150
 * @param[in] LfChannels: latent fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetLatentFaultFspEnable(uint32 LfChannels)
{
    WRITE_REG32(SMU->FSPLFCR, LfChannels);
}

/*!
 * @brief Get which latent fault channels could trigger FSP output.
 * @note      Function ID: DES_SMU_API_151
 * @param[in] void
 * @return latent fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetLatentFaultFspEnable(void)
{
    return READ_REG32(SMU->FSPLFCR);
}
#endif

#if defined (AC7840E) || defined (AC7842X) || defined (AC7843X)
/*!
 * @brief Enable which single point fault channels could trigger interrupt.
 * @note      Function ID: DES_SMU_API_132
 * @param[in] SpfChannels: single fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetSPFaultInterruptEnable(uint32 SpfChannels)
{
    WRITE_REG32(SMU->SFINTEN, SpfChannels);
}

/*!
 * @brief Get which single point fault channel interrupt.
 * @note      Function ID: DES_SMU_API_133
 * @param[in] void
 * @return single point fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetSPFaultInterruptEnable(void)
{
    return READ_REG32(SMU->SFINTEN);
}

/*!
 * @brief Enable which latent fault channels could interrupt.
 * @note      Function ID: DES_SMU_API_134
 * @param[in] LfChannels: latent fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetLatentFaultInterruptEnable(uint32 LfChannels)
{
    WRITE_REG32(SMU->LFINTEN, LfChannels);
}

/*!
 * @brief Get which latent fault channel interrupt function.
 * @note      Function ID: DES_SMU_API_135
 * @param[in] void
 * @return latent fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetLatentFaultInterruptEnable(void)
{
    return READ_REG32(SMU->LFINTEN);
}
#endif

#if defined (AC7843X)
/*!
 * @brief HSM Single point fault register access permission lock.
 * @note      Function ID: DES_SMU_API_100
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_HsmSPFaultRegLock(void)
{
    WRITE_REG32(SMU->HSM_LKSEQ0, 0U);
}

/*!
 * @brief Single point fault register access permission unlock.
 * @note      Function ID: DES_SMU_API_101
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_HsmSPFaultRegUnlock(void)
{
    WRITE_REG32(SMU->HSM_LKSEQ0, SMU_HSM_UNLOCK_FIRST_VALUE);

    WRITE_REG32(SMU->HSM_LKSEQ0, SMU_HSM_UNLOCK_SECOND_VALUE);
}

/*!
 * @brief Enable which single point fault channels could trigger reset.
 * @note      Function ID: DES_SMU_API_104
 * @param[in] SpfChannels: single fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetHsmSPFaultResetEnable(uint32 SpfChannels)
{
    WRITE_REG32(SMU->HSM_SFRSTEN, SpfChannels);
}

/*!
 * @brief Get which single point fault channel reset.
 * @note      Function ID: DES_SMU_API_105
 * @param[in] void
 * @return single point fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetHsmSPFaultResetEnable(void)
{
    return READ_REG32(SMU->HSM_SFRSTEN);
}

/*!
 * @brief Enable which single point fault channels could trigger interrupt.
 * @note      Function ID: DES_SMU_API_132
 * @param[in] SpfChannels: single fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetHsmSPFaultInterruptEnable(uint32 SpfChannels)
{
    WRITE_REG32(SMU->HSM_SFINTEN, SpfChannels);
}

/*!
 * @brief Get which single point fault channel interrupt.
 * @note      Function ID: DES_SMU_API_133
 * @param[in] void
 * @return single point fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetHsmSPFaultInterruptEnable(void)
{
    return READ_REG32(SMU->HSM_SFINTEN);
}

/*!
 * @brief Set software single point fault channel function.
 * @note      Function ID: DES_SMU_API_108
 * @param[in] SwSpfChannels: software single point fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetHsmSoftwareSPFault(uint32 SwSpfChannels)
{

    WRITE_REG32(SMU->SWSFE, SwSpfChannels);
}

/*!
 * @brief Set software single point fault channel function.
 * @note      Function ID: DES_SMU_API_109
 * @param[in] void
 * @return software single point fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetHsmSoftwareSPFault(void)
{
    return READ_REG32(SMU->HSM_SWSFE);
}

/*!
 * @brief Set software single point fault shadow channels function.
 * @note      Function ID: DES_SMU_API_110
 * @param[in] void
 * @return software single point fault shadow channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetHsmSoftwareSPFaultShadow(void)
{
    return READ_REG32(SMU->HSM_SWSFES);
}

/*!
 * @brief Set SMU reset counter value.
 * @note      Function ID: DES_SMU_API_114
 * @param[in] Val: reset counter value which SHALL be 0~15.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetHsmResetCounterThreshold(uint8 Val)
{
    WRITE_REG32(SMU->HSM_SRSTCNTVAL, Val);
}

/*!
 * @brief Set SMU reset counter value.
 * @note      Function ID: DES_SMU_API_115
 * @param[in] void
 * @return reset counter value which SHALL be 0~15.
 */
LOCAL_INLINE uint8 Smu_Reg_GetHsmResetCounterThreshold(void)
{
    return (uint8)READ_REG32(SMU->HSM_SRSTCNTVAL);
}

/*!
 * @brief Get SMU single point fault status.
 * @note      Function ID: DES_SMU_API_116
 * @param[in] void
 * @return SMU single point fault status.
 */
LOCAL_INLINE uint32 Smu_Reg_GetHsmSPFaultStatus(void)
{
    return READ_REG32(SMU->HSM_SFES);
}

/*!
 * @brief Set SMU single point fault status.
 * @note      Function ID: DES_SMU_API_117
 * @param[in] SpfChannels: single fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetHsmSPFaultStatus(uint32 SpfChannels)
{
    WRITE_REG32(SMU->HSM_SFES, SpfChannels);
}

/*!
 * @brief Get SMU single point fault status.
 * @note      Function ID: DES_SMU_API_118
 * @param[in] void
 * @return SMU single point fault status.
 */
LOCAL_INLINE uint32 Smu_Reg_GetHsmSPFaultStatusShadow(void)
{
    return READ_REG32(SMU->HSM_SFESS);
}

/*!
 * @brief Set SMU single point fault status.
 * @note      Function ID: DES_SMU_API_119
 * @param[in] SpfChannels: single fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetHsmSPFaultStatusShadow(uint32 SpfChannels)
{
    WRITE_REG32(SMU->HSM_SFESS, SpfChannels);
}

/*!
 * @brief Get SMU reset counter status.
 * @note      Function ID: DES_SMU_API_124
 * @param[in] void
 * @return SMU reset counter.
 */
LOCAL_INLINE uint8 Smu_Reg_GetHsmResetCounter(void)
{
    return (uint8)READ_REG32(SMU->HSM_SRSTCNT);
}

/*!
 * @brief Clear SMU reset counter
 * @note      Function ID: DES_SMU_API_125
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_ClearHsmResetCounter(void)
{
    /* Clear SMU reset counter */
    WRITE_REG32(SMU->HSM_SRSTCNT, 0xFFFFFFFFU);
}

/*!
 * @brief Get SMU reset counter shadow status.
 * @note      Function ID: DES_SMU_API_126
 * @param[in] void
 * @return SMU reset counter shadow
 */
LOCAL_INLINE uint8 Smu_Reg_GetHsmResetCounterShadow(void)
{
    return (uint8)READ_REG32(SMU->HSM_SRSTCNTS);
}

/*!
 * @brief Clear SMU reset counter shadow.
 * @note      Function ID: DES_SMU_API_127
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Smu_Reg_ClearHsmResetCounterShadow(void)
{
    /* Clear SMU reset counter shadow */
    WRITE_REG32(SMU->HSM_SRSTCNTS, 0xFFFFFFFFU);
}

/*!
 * @brief Set SMU patch check0.
 * @note      Function ID: DES_SMU_API_128
 * @param[in] SpfChannels: Hardware and software Single point fault channels.
 * @return void
 */
LOCAL_INLINE void Smu_Reg_SetHsmPathCheck0(uint32 SpfChannels)
{
    WRITE_REG32(SMU->HSM_PATHCHK0, SpfChannels);
}

/*!
 * @brief Get SMU reset patch check0.
 * @note      Function ID: DES_SMU_API_129
 * @param[in] void
 * @return Hardware and software Single point fault channels.
 */
LOCAL_INLINE uint32 Smu_Reg_GetHsmPathCheck0(void)
{
    return READ_REG32(SMU->HSM_PATHCHK0);
}
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* AC784XX_SMU_REG_H */
