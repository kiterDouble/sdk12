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
 * AutoChips Inc. (C) 2025. All rights reserved.
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
 * @file AC784xx_Pct_Reg.h
 *
 * @brief This file provides pct reg interface.
 *
 */

/* PRQA S 4304,4394 EOF */ /* AMCE_GPT_4304,AMCE_GPT_4394 */

#ifndef AC784XX_PCT_REG_H
#define AC784XX_PCT_REG_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "Device_Register.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */

/* =====================================  Functions definition  ===================================== */
/*!
 * @brief Gets the compare flag.
 *
 * @param[in] Base: Base address of PCT
 * @return The compare flag
 */
LOCAL_INLINE boolean PCT_Reg_GetCompareFlag(const PCT_Type* const Base)
{
    return (boolean)(READ_BIT32(Base->CSR, PCT_CSR_CF_Msk) >> PCT_CSR_CF_Pos);
}

/*!
 * @brief Clears the compare flag.
 *
 * @param[in] Base: Base address of PCT
 * @return none
 */
LOCAL_INLINE void PCT_Reg_ClearCompareFlag(PCT_Type* const Base)
{
    SET_BIT32(Base->CSR, PCT_CSR_CF_Msk);
}

/*!
 * @brief Gets if the interrupt enabled.
 *
 * @param[in] Base: Base address of PCT
 * @return Interrupt Enable state
 * @return none
 */
LOCAL_INLINE boolean PCT_Reg_GetInterruptEnable(const PCT_Type* const Base)
{
    return (boolean)(READ_BIT32(Base->CSR, PCT_CSR_CIE_Msk) >> PCT_CSR_CIE_Pos);
}

/*!
 * @brief Configures if the iInterrupt enabled.
 *
 * @param[in] Base: Base address of PCT
 * @param[in] enable: enable & disable interrupt control
 * @return none
 */
LOCAL_INLINE void PCT_Reg_SetInterrupt(PCT_Type* const Base, const boolean Enable)
{
    MODIFY_REG32(Base->CSR, PCT_CSR_CIE_Msk, PCT_CSR_CIE_Pos, Enable);
}

/*!
 * @brief Gets the pin selection for pulse counter mode.
 *
 * @param[in] Base: Base address of PCT
 * @return pulse pin selection
 */
LOCAL_INLINE uint8 PCT_Reg_GetPinSelect(const PCT_Type* const Base)
{
    return ((uint8)(READ_BIT32(Base->CSR, PCT_CSR_IPS_Msk) >> PCT_CSR_IPS_Pos));
}

/*!
 * @brief Selects pin selection for pulse counter mode.
 *
 * @param[in] Base: Base address of PCT
 * @param[in] pinsel: pin selection to be configured
 * @return none
 */
LOCAL_INLINE void PCT_Reg_SetPinSelect(PCT_Type* const Base, const uint8 PinSel)
{
    MODIFY_REG32(Base->CSR, PCT_CSR_IPS_Msk, PCT_CSR_IPS_Pos, PinSel);
}

/*!
 * @brief Gets pin polarity for pulse counter mode.
 *
 * @param[in] Base: Base address of PCT
 * @return pulse pin polarity got
 */
LOCAL_INLINE uint8 PCT_Reg_GetPinPolarity(const PCT_Type* const Base)
{
    return ((uint8)(READ_BIT32(Base->CSR, PCT_CSR_PPS_Msk) >> PCT_CSR_PPS_Pos));
}

/*!
 * @brief Sets pin polarity for pulse counter mode.
 *
 * @param[in] Base: Base address of PCT
 * @param[in] pol : the pin polarity to be configured
 * @return none
 */
LOCAL_INLINE void PCT_Reg_SetPinPolarity(PCT_Type* const Base, const uint8 Pol)
{
    MODIFY_REG32(Base->CSR, PCT_CSR_PPS_Msk, PCT_CSR_PPS_Pos, Pol);
}

/*!
 * @brief Gets if PCT is in free running state.
 *
 * @param[in] Base: Base address of PCT
 * @return Free running mode state
 */
LOCAL_INLINE boolean PCT_Reg_GetFreeRunning(const PCT_Type* const Base)
{
    return (boolean)(READ_BIT32(Base->CSR, PCT_CSR_FRE_Msk) >> PCT_CSR_FRE_Pos);
}

/*!
 * @brief Sets the free running state of PCT.
 *
 * @param[in] Base : Base address of PCT
 * @param[in] enable: enable & disable free running state
 * @return none
 */
LOCAL_INLINE void PCT_Reg_SetFreeRunning(PCT_Type* const Base, const boolean Enable)
{
    MODIFY_REG32(Base->CSR, PCT_CSR_FRE_Msk, PCT_CSR_FRE_Pos, (uint32)Enable);
}

/*!
 * @brief Gets the work mode of PCT.
 *
 *
 * @param[in] Base: Base address of PCT
 * @return work mode
 */
LOCAL_INLINE uint8 PCT_Reg_GetWorkMode(const PCT_Type* const Base)
{
    return ((uint8)(READ_BIT32(Base->CSR, PCT_CSR_MS_Msk) >> PCT_CSR_MS_Pos));
}

/*!
 * @brief Sets the work mode.
 *
 * @param[in] Base: Base address of PCT
 * @param[in] mode: work mode to be configured
 * @return none
 */
LOCAL_INLINE void PCT_Reg_SetWorkMode(PCT_Type* const Base, const uint8 Mode)
{
    MODIFY_REG32(Base->CSR, PCT_CSR_MS_Msk, PCT_CSR_MS_Pos, Mode);
}

/*!
 * @brief Gets if PCT is enabled.
 *
 * @param[in] Base: Base address of PCT
 * @return The enable state of the PCT
 */
LOCAL_INLINE boolean PCT_Reg_GetEnable(const PCT_Type* const Base)
{
    return (boolean)(READ_BIT32(Base->CSR, PCT_CSR_PCT_EN_Msk) >> PCT_CSR_PCT_EN_Pos);
}

/*!
 * @brief Enable/Disable PCT and starts counting.
 *
 * @param[in] Base: Base address of PCT
 * @param[in] enable: enable & disable PCT
 * @return none

 */
LOCAL_INLINE void PCT_Reg_SetEnable(PCT_Type* const Base, const boolean Enable)
{
    MODIFY_REG32(Base->CSR, PCT_CSR_PCT_EN_Msk, PCT_CSR_PCT_EN_Pos, ((uint8)Enable & 0x1U));  /* PRQA S 4391 # AMCE_GPT_4391 */
}

/*!
 * @brief Gets the prescaler/flter value.
 *
 * @param[in] Base: Base address of PCT
 * @return The prescaler/filter value
 */
LOCAL_INLINE uint8 PCT_Reg_GetPrescaler(const PCT_Type* const Base)
{
    return ((uint8)(READ_BIT32(Base->PSR, PCT_PSR_PS_VAL_Msk) >> PCT_PSR_PS_VAL_Pos));
}

/*!
 * @brief Sets the prescaler/filter value.
 *
 * @param[in] Base : Base address of PCT
 * @param[in] presc: the Prescaler value to be configured
 * @return none
 */
LOCAL_INLINE void PCT_Reg_SetPrescaler(PCT_Type* const Base, const uint8 Value)
{
    MODIFY_REG32(Base->PSR, PCT_PSR_PS_VAL_Msk, PCT_PSR_PS_VAL_Pos, Value);
}

/*!
 * @brief Sets Read cval register presivison.
 *
 * @param[in] Base: Base address of PCT
 * @param[in] sel: Read cval register presivison value
 * @return none
 */
LOCAL_INLINE void PCT_Reg_SetCvalSel(PCT_Type* const Base, const uint8 Sel)
{
    MODIFY_REG32(Base->PSR, PCT_PSR_CVAL_SEL_Msk, PCT_PSR_CVAL_SEL_Pos, Sel);
}

/*!
 * @brief Gets Read cval register presivison.
 *
 * @param[in] Base: Base address of PCT
 * @return pct sel value
 */
LOCAL_INLINE uint8 PCT_Reg_GetCvalSel(const PCT_Type* const Base)
{
    return ((uint8)(READ_BIT32(Base->PSR, PCT_PSR_CVAL_SEL_Msk) >> PCT_PSR_CVAL_SEL_Pos));
}

/*!
 * @brief Gets if the prescaler/filter is bypassed.
 *
 * @param[in] Base: Base address of PCT
 * @return The prescaler/filter bypass state

 */
LOCAL_INLINE boolean PCT_Reg_GetBypass(const PCT_Type* const Base)
{
    return ((READ_BIT32(Base->PSR, PCT_PSR_BYP_Msk) >> PCT_PSR_BYP_Pos) != 0U);   /* PRQA S 4404 # AMCE_GPT_4404 */
}

/*!
 * @brief Sets if the prescaler/filter is bypassed.
 *
 * @param[in] Base : Base address of PCT
 * @param[in] enable: the prescaler/filter bypass state to be configured
 * @return none
 */
LOCAL_INLINE void PCT_Reg_SetBypass(PCT_Type* const Base, const boolean Enable)
{
    MODIFY_REG32(Base->PSR, PCT_PSR_BYP_Msk, PCT_PSR_BYP_Pos, Enable);
}

/*!
 * @brief Gets the PCT input Clock selection.
 *
 * @param[in] Base: Base address of PCT
 * @return The Clock source of PCT
 */
LOCAL_INLINE uint8 PCT_Reg_GetClockSelect(const PCT_Type* const Base)
{
    return ((uint8)(READ_BIT32(Base->PSR, PCT_PSR_PCS_Msk) >> PCT_PSR_PCS_Pos));
}

/*!
 * @brief Sets the PCT input Clock selection.
 *
 * @param[in] Base: Base address of PCT
 * @param[in] clocksel: the clock cource to be configured
 * @return none
 */
LOCAL_INLINE void PCT_Reg_SetClockSelect(PCT_Type* const Base, uint8 ClockSel)
{
    MODIFY_REG32(Base->PSR, PCT_PSR_PCS_Msk, PCT_PSR_PCS_Pos, ClockSel);
}

/*!
 * @brief Gets the compare value.
 *
 * @param[in] Base: Base address of PCT
 * @return The compare value got
 */
LOCAL_INLINE uint16 PCT_Reg_GetCompareValue(const PCT_Type* const Base)
{
    return (uint16)READ_REG32(Base->CMR);
}

/*!
 * @brief Sets the compare value
 *
 * @param[in] Base: Base address of PCT
 * @param[in] compval: The Compare Value to be configured
 * @return none
 */
LOCAL_INLINE void PCT_Reg_SetCompareValue(PCT_Type* const Base, const uint16 Val)
{
    WRITE_REG32(Base->CMR, Val);
}

/*!
 * @brief Gets the current counter Value.
 *
 * @param[in] Base: Base address of PCT
 * @return The current counter Value got
 */
LOCAL_INLINE uint16 PCT_Reg_GetCounterValue(const PCT_Type* const Base)
{
    return (uint16)READ_REG32(Base->CNR);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AC784XX_PCT_REG_H */

/* =============================================  EOF  ============================================== */
