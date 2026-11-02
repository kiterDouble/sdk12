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
 * AutoChips Inc. (C) 2021. All rights reserved.
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
 * @file  AC784xx_Pdt_Reg.h
 *
 * @brief This file provides PDT hardware integration functions.
 *
 */

/* PRQA S 4304 EOF */ /* AMCE_GPT_4304 */

#ifndef AC784XX_PDT_REG_H
#define AC784XX_PDT_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "Device_Register.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */

/* =====================================  Functions definition  ===================================== */
/*!
 * @brief Set PDT multiplication factor for prescaler.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] Mult: multiplication factor select for prescaler
 *            - PDT_CLK_PREMULT_FACT_AS_1
 *            - PDT_CLK_PREMULT_FACT_AS_10
 *            - PDT_CLK_PREMULT_FACT_AS_20
 *            - PDT_CLK_PREMULT_FACT_AS_40
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetMultForPrescaler(PDT_Type * const Base, const uint8 Mult)
{
    MODIFY_REG32(Base->SC, PDT_SC_MULT_Msk, PDT_SC_MULT_Pos, Mult);
}

/*!
 * @brief Set PDT clock prescaler.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] Psc: prescaler divide
 *            - PDT_CLK_PREDIV_BY_1
 *            - PDT_CLK_PREDIV_BY_2
 *            - PDT_CLK_PREDIV_BY_4
 *            - PDT_CLK_PREDIV_BY_8
 *            - PDT_CLK_PREDIV_BY_16
 *            - PDT_CLK_PREDIV_BY_32
 *            - PDT_CLK_PREDIV_BY_64
 *            - PDT_CLK_PREDIV_BY_128
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetClockPrescaler(PDT_Type * const Base, const uint8 Psc)
{
    MODIFY_REG32(Base->SC, PDT_SC_PRE_Msk, PDT_SC_PRE_Pos, Psc);
}

/*!
 * @brief Set PDT count mode.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] State: continuous mode enable
 *            - TRUE
 *            - FALSE
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetCountMode(PDT_Type * const Base, const boolean State)
{
    MODIFY_REG32(Base->SC, PDT_SC_CONT_Msk, PDT_SC_CONT_Pos, State);
}

/*!
 * @brief Set PDT load values cmd.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @return none
 */
LOCAL_INLINE void PDT_Reg_LoadValuesCmd(PDT_Type * const Base)
{
    MODIFY_REG32(Base->SC, PDT_SC_LDOK_Msk, PDT_SC_LDOK_Pos, TRUE);
}

/*!
 * @brief Set PDT load mode.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] Mode: PDT load mode
 *            - PDT_LOAD_VAL_IMMEDIATELY
 *            - PDT_LOAD_VAL_AT_MODULO_COUNTER
 *            - PDT_LOAD_VAL_AT_NEXT_TRIGGER
 *            - PDT_LOAD_VAL_AT_MODULO_COUNTER_OR_NEXT_TRIGGER
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetLoadMode(PDT_Type * const Base, const uint8 Mode)
{
    MODIFY_REG32(Base->SC, PDT_SC_LDMOD_Msk, PDT_SC_LDMOD_Pos, Mode);
}

/*!
 * @brief Set PDT module interrupt enable.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] State: enabling state
 *            - TRUE
 *            - FALSE
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetInterruptEnable(PDT_Type * const Base, const boolean State)
{
    MODIFY_REG32(Base->SC, PDT_SC_PDTIE_Msk, PDT_SC_PDTIE_Pos, State);
}

/*!
 * @brief Get PDT timer interrupt flag.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @return PDT interrupt flag
 */
#define PDT_Reg_GetTimerIntFlag(Base)         (READ_BIT32((Base)->SC, PDT_SC_PDTIF_Msk) >> PDT_SC_PDTIF_Pos)

/*!
 * @brief Clear PDT timer interrupt flag.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @return none
 */
#ifdef AC7843X
#define PDT_Reg_ClearTimerIntFlag(Base)         SET_BIT32((Base)->SC, PDT_SC_PDTIF_Msk)
#else
#define PDT_Reg_ClearTimerIntFlag(Base)         CLEAR_BIT32((Base)->SC, PDT_SC_PDTIF_Msk)
#endif

/*!
 * @brief PDT Enable.
 *
 * @param[in] Base: the PDT base address
 *               - PDT0
 *               - PDT1
 * @return none
 */
LOCAL_INLINE void PDT_Reg_Enable(PDT_Type * const Base, const boolean Enable)
{
    MODIFY_REG32(Base->SC, PDT_SC_PDTEN_Msk, PDT_SC_PDTEN_Pos, ((uint32)Enable & 0x1U));
}

/*!
 * @brief Set PDT SoftTrigger Cmd.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SoftTriggerCmd(PDT_Type * const Base)
{
    MODIFY_REG32(Base->SC, PDT_SC_SWTRIG_Msk, PDT_SC_SWTRIG_Pos, TRUE);
}

/*!
 * @brief Set PDT trigger input source.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] Source: PDT trigger input source
 *            - PDT_HARDWARE_TRIGGER
 *            - PDT_SOFTWARE_TRIGGER
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetTriggerSource(PDT_Type * const Base, const uint8 Source)
{
    MODIFY_REG32(Base->SC, PDT_SC_TRGSEL_Msk, PDT_SC_TRGSEL_Pos, Source);
}

/*!
 * @brief Set PDT Trigger delay bypass Enable.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] State: bypass enable
 *            - TRUE
 *            - FALSE
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetTriggerDelayBypass(PDT_Type * const Base, const boolean State)
{
    MODIFY_REG32(Base->SC, PDT_SC_Bypass_En_Msk, PDT_SC_Bypass_En_Pos, State);
}

/*!
 * @brief Set PDT Timer Modulus Value.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] Value: mod value
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetTimerModulusValue(PDT_Type * const Base, const uint16 Value)
{
    MODIFY_REG32(Base->MOD, PDT_MOD_MOD_Msk, PDT_MOD_MOD_Pos, Value);
}

/*!
 * @brief Set PDT trigger Delay Value.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] Chn: delay channel(0~7)
 * @param[in] Value: trigger delay value
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetTriggerDelayValue(PDT_Type * const Base, const uint8 Chn, const uint16 Value)
{
    MODIFY_REG32(Base->DLY[Chn].DLY, PDT_DLY0_DLY0_Msk, PDT_DLY0_DLY0_Pos, Value);
}

/*!
 * @brief Set PDT interrupt delay value.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] Value: intrrupt delay value
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetValueForTimerInterrupt(PDT_Type * const Base, const uint16 Value)
{
    MODIFY_REG32(Base->IDLY, PDT_IDLY_IDLY_Msk, PDT_IDLY_IDLY_Pos, Value);
}

/*!
 * @brief Set PDT trigger delay enable.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] DlyEnable: all 8 channel dly enable mask
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetTriggerDelayEnable(PDT_Type * const Base, const uint8 DlyEnable)
{
    MODIFY_REG32(Base->DLYn_EN, PDT_DLY_EN_Msk, PDT_DLYn_EN_DLY0_En_Pos, DlyEnable);
}

/*!
 * @brief Set PDT pulse-out enable.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @param[in] State: pulse-out enable
 *            - TRUE
 *            - FALSE
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetCmpPulseOutEnable(PDT_Type * const Base, const boolean State)
{
    MODIFY_REG32(Base->POEN, PDT_POEN_POEN_Msk, PDT_POEN_POEN_Pos, State);
}

/*!
 * @brief Set PDT pulse-out delay value for goes high.
 *
 * @param[in] Base: PDT module
 *            - PDT0
 *            - PDT1
 * @param[in] Value: delay value for goes high
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetCmpPulseOutDelayForHigh(PDT_Type * const Base, const uint16 Value)
{
    Base->PULSE.ACCESS16BIT.DLY1 = Value;
}

/*!
 * @brief Set PDT pulse-out delay value for goes low.
 *
 * @param[in] Base: PDT module
 *            - PDT0
 *            - PDT1
 * @param[in] Value: delay value for goes low
 * @return none
 */
LOCAL_INLINE void PDT_Reg_SetCmpPulseOutDelayForLow(PDT_Type * const Base, const uint16 Value)
{
   Base->PULSE.ACCESS16BIT.DLY2 = Value;
}

/*!
 * @brief Get timer value.
 *
 * @param[in] Base: the PDT base address
 *            - PDT0
 *            - PDT1
 * @return timer counting value
 */
#define PDT_Reg_GetTimerValue(Base)         (READ_BIT32((Base)->CNT, PDT_CNT_CNT_Msk) >> PDT_CNT_CNT_Pos)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PDT_HW_H */

/* =============================================  EOF  ============================================== */
