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
 * @file acmp_hw.h
 *
 * @brief Analog comparator module access register inline function definition.
 *
 */

#ifndef AC784XX_ACMP_REG_H
#define AC784XX_ACMP_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"
#include "Acmp_Hal_Types.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* =====================================  Functions definition  ===================================== */
/**
* @brief Get ACMP base.
*
* @param[in] Instance : ACMP hardware channel ID.
* @return ACMP_Type*: the ACMP base addr.
*/
LOCAL_INLINE ACMP_Type *Acmp_Reg_GetBase(uint8 Instance)
{
    DEVICE_ASSERT(Instance < ACMP_INSTANCE_MAX);
    ACMP_Type *const AcmpBase[ACMP_INSTANCE_MAX] =
    {
        ACMP0,
#if defined (AC7843X)
        ACMP1
#endif
    };

    return AcmpBase[Instance];
}

/*!
 * @brief Set ACMP output pin enable flag.
 *
 * @param [in] Base: acmp module base
 * @param [in] State: enable or disable output pin function
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetOutputPinEnableFlag(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_OUTEN_Msk, ACMP_CR0_OUTEN_Pos, (TRUE == State) ? 1UL : 0UL);
}

/*!
 * @brief Get ACMP output pin enable flag.
 *
 * @param [in] Base: acmp module base
 * @return whether output pin function is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetOutputPinEnableFlag(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR0, ACMP_CR0_OUTEN_Msk) >> ACMP_CR0_OUTEN_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set ACMP output signal selection.
 *
 * @param [in] Base: acmp module base
 * @param [in] select: which signal to output
 *                  - ACMP_COUT
 *                  - ACMP_COUTA
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetOutputSelection(ACMP_Type *Base, Acmp_OutputSelectType Select)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_COS_Msk, ACMP_CR0_COS_Pos, Select);
}

/*!
 * @brief Get ACMP output signal selection.
 *
 * @param [in] Base: acmp module base
 * @return which output signal is selected
 *                  - ACMP_COUT
 *                  - ACMP_COUTA
 */
LOCAL_INLINE Acmp_OutputSelectType Acmp_Reg_GetOutputSelection(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->CR0, ACMP_CR0_COS_Msk) >> ACMP_CR0_COS_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_OutputSelectType)Val;
    /*PRQA S 4342 -- */
}

/*!
 * @brief Set ACMP analog module power enable flag.
 *
 * @param [in] Base: acmp module base
 * @param [in] State: enable or disable acmp analog module
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetEnableFlag(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_EN_Msk, ACMP_CR0_EN_Pos, (TRUE == State) ? 1UL : 0UL);
}

/*!
 * @brief Get ACMP analog module power enable flag.
 *
 * @param [in] Base: acmp module base
 * @return whether acmp analog module is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetEnableFlag(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR0, ACMP_CR0_EN_Msk) >> ACMP_CR0_EN_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set ACMP interrupt enable flag
 *
 * @param [in] Base: acmp module base
 * @param [in] State: enable or disable interrupt function
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetInterruptEnableFlag(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_IE_Msk, ACMP_CR0_IE_Pos, (TRUE == State) ? 1UL : 0UL);
}

/*!
 * @brief Get ACMP interrupt enable flag
 *
 * @param [in] Base: acmp module base
 * @return whether acmp interrupt function is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetInterruptEnableFlag(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR0, ACMP_CR0_IE_Msk) >> ACMP_CR0_IE_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set ACMP trigger mode.
 *
 * @param [in] Base: acmp module base
 * @param [in] Mode: trigger mode
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetTriggerMode(ACMP_Type *Base, Acmp_OutputTriggerType Mode)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_MOD_Msk, ACMP_CR0_MOD_Pos, Mode);
}

/*!
 * @brief Get ACMP trigger mode.
 *
 * @param [in] Base: acmp module base
 * @return acmp current trigger mode
 */
LOCAL_INLINE Acmp_OutputTriggerType Acmp_Reg_GetTriggerMode(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR0, ACMP_CR0_MOD_Msk) >> ACMP_CR0_MOD_Pos;
    /* 2 is the falling edge mode like 0 */
    if (2U == tmp)
    {
        tmp = (uint32)ACMP_FALLING_EDGE;
    }
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_OutputTriggerType)tmp;
    /*PRQA S 4342 -- */
}

#if !defined(AC7840E)
/*!
 * @brief Set ACMP using LSI clock as function clock enable flag.
 *
 * @param [in] Base: acmp module base
 * @param [in] State: enable or disable ACMP using LSI clock as function clock
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetUsingLSIEnableFlag(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_LSICLK_EN_Msk, ACMP_CR0_LSICLK_EN_Pos, (TRUE == State) ? 1UL : 0UL);
#if defined (AC7843X)
    MODIFY_REG32(Base->CR0, ACMP_CR0_LSICLK_SEL_Msk, ACMP_CR0_LSICLK_SEL_Pos, (TRUE == State) ? 1UL : 0UL);
#endif
}

/*!
 * @brief Get ACMP using LSI clock as function clock enable flag.
 *
 * @param [in] Base: acmp module base
 * @return whether ACMP using LSI clock as function clock
 */
LOCAL_INLINE boolean Acmp_Reg_GetUsingLSIEnableFlag(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR0, ACMP_CR0_LSICLK_EN_Msk) >> ACMP_CR0_LSICLK_EN_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}
#endif

/*!
 * @brief Set ACMP positive input channel selection.
 *
 * @param [in] Base: acmp module base
 * @param [in] Channel: channel that need connect to the positive input
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetPositiveInputPin(ACMP_Type *Base, Acmp_ChNumberType Channel)
{
    MODIFY_REG32(Base->CR1, ACMP_CR1_PSEL_Msk, ACMP_CR1_PSEL_Pos, Channel);
}

/*!
 * @brief Get ACMP positive input channel selection.
 *
 * @param [in] Base: acmp module base
 * @return current channel that connect to the positive input.
 */
LOCAL_INLINE Acmp_ChNumberType Acmp_Reg_GetPositiveInputPin(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->CR1, ACMP_CR1_PSEL_Msk) >> ACMP_CR1_PSEL_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_ChNumberType)Val;
    /*PRQA S 4342 -- */
}

/*!
 * @brief Set ACMP negative input channel selection.
 *
 * @param [in] Base: acmp module base
 * @param [in] channel: channel that need connect to the negative input
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetNegativeInputPin(ACMP_Type *Base, Acmp_ChNumberType channel)
{
    MODIFY_REG32(Base->CR1, ACMP_CR1_NSEL_Msk, ACMP_CR1_NSEL_Pos, channel);
}

/*!
 * @brief Get ACMP negative input channel selection.
 *
 * @param [in] Base: acmp module base
 * @return current channel that connect to the negative input.
 */
LOCAL_INLINE Acmp_ChNumberType Acmp_Reg_GetNegativeInputPin(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->CR1, ACMP_CR1_NSEL_Msk) >> ACMP_CR1_NSEL_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342*/
    return (Acmp_ChNumberType)Val;
    /*PRQA S 4342 -- */
}

/*!
 * @brief Set ACMP hysteresis mode.
 *
 * @param [in] Base: acmp module base
 * @param [in] mode: hysteresis mode
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetHysteresisMode(ACMP_Type *Base, Acmp_HysteresisModeType mode)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_HYSTMOD_Msk, ACMP_CR0_HYSTMOD_Pos, mode);
}

/*!
 * @brief Get ACMP hysteresis mode.
 *
 * @param [in] Base: acmp module base
 * @return current hysteresis mode setting
 */
LOCAL_INLINE Acmp_HysteresisModeType Acmp_Reg_GetHysteresisMode(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->CR0, ACMP_CR0_HYSTMOD_Msk) >> ACMP_CR0_HYSTMOD_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_HysteresisModeType)Val;
    /*PRQA S 4342 -- */
}

/*!
 * @brief Set ACMP hysteresis voltage type.
 *
 * @param [in] Base: acmp module base
 * @param [in] Vol: hysteresis voltage type
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetHysteresisVoltage(ACMP_Type *Base, Acmp_HysteresisType Vol)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_HYSTVOL_Msk, ACMP_CR0_HYSTVOL_Pos, Vol);
}

/*!
 * @brief Get ACMP hysteresis voltage type.
 *
 * @param [in] Base: acmp module base
 * @return current hysteresis voltage type
 */
LOCAL_INLINE Acmp_HysteresisType Acmp_Reg_GetHysteresisVoltage(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->CR0, ACMP_CR0_HYSTVOL_Msk) >> ACMP_CR0_HYSTVOL_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_HysteresisType)Val;
    /*PRQA S 4342 -- */
}

/*!
 * @brief Set low pass filter bandwidth type.
 *
 * @param [in] Base: acmp module base
 * @param [in] Freq: low pass filter bandwidth
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetLowPassFilter(ACMP_Type *Base, Acmp_LowPassFilterType Freq)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_LPF_Msk, ACMP_CR0_LPF_Pos, Freq);
}

/*!
 * @brief Get low pass filter bandwidth type.
 *
 * @param [in] Base: acmp module base
 * @return low pass filter bandwidth
 */
LOCAL_INLINE Acmp_LowPassFilterType Acmp_Reg_GetLowPassFilter(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->CR0, ACMP_CR0_LPF_Msk) >> ACMP_CR0_LPF_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_LowPassFilterType)Val;
    /*PRQA S 4342 -- */
}

/*!
 * @brief Set ACMP digital filter enable flag.
 *
 * @param [in] Base: acmp module base
 * @param [in] State: enable or disable digital filter
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetFilterEnableFlag(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_FILTEN_Msk, ACMP_CR0_FILTEN_Pos, (TRUE == State) ? 1UL : 0UL);
}

/*!
 * @brief Get ACMP digital filter enable flag.
 *
 * @param [in] Base: acmp module base
 * @return whether digital filter is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetFilterEnableFlag(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR0, ACMP_CR0_FILTEN_Msk) >> ACMP_CR0_FILTEN_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set digital filter clock prescaler.
 *
 * @param [in] Base: acmp module base
 * @param [in] Psc: digital filter prescaler
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetFilterClockPrescaler(ACMP_Type *Base, Acmp_FilterDivideType Psc)
{
    MODIFY_REG32(Base->CLK, ACMP_CLK_PSC_Msk, ACMP_CLK_PSC_Pos, Psc);
}

/*!
 * @brief Get digital filter clock prescaler.
 *
 * @param [in] Base: acmp module base
 * @return current digital filter prescaler setting
 */
LOCAL_INLINE Acmp_FilterDivideType Acmp_Reg_GetFilterClockPrescaler(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->CLK, ACMP_CLK_PSC_Msk) >> ACMP_CLK_PSC_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_FilterDivideType)Val;
    /*PRQA S 4342 -- */
}

/*!
 * @brief Set digital filter length.
 *
 * @param [in] Base: acmp module base
 * @param [in] FilterCnt: filter count setting
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetFilterLength(ACMP_Type *Base, uint8 FilterCnt)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_FILTER_CNT_Msk, ACMP_CR0_FILTER_CNT_Pos, FilterCnt);
}

/*!
 * @brief Get digital filter length.
 *
 * @param [in] Base: acmp module base
 * @return current filter count setting
 */
LOCAL_INLINE uint8 Acmp_Reg_GetFilterLength(const ACMP_Type *Base)
{
    return (uint8)(READ_BIT32(Base->CR0, ACMP_CR0_FILTER_CNT_Msk) >> ACMP_CR0_FILTER_CNT_Pos);
}

/*!
 * @brief Set ACMP window mode enable setting.
 *
 * @param [in] Base: acmp module base
 * @param [in] State: enable or disable window mode
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetWindowMode(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_WINEN_Msk, ACMP_CR0_WINEN_Pos, (TRUE == State) ? 1UL : 0UL);
}

/*!
 * @brief Get ACMP window mode enable setting.
 *
 * @param [in] Base: acmp module base
 * @return whether window mode is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetWindowMode(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR0, ACMP_CR0_WINEN_Msk) >> ACMP_CR0_WINEN_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set ACMP result invertion.
 *
 * @param [in] Base: acmp module base
 * @param [in] state: enable or disable result invertion
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetInvertMode(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_INVT_Msk, ACMP_CR0_INVT_Pos, (TRUE == State) ? 1UL : 0UL);
}

/*!
 * @brief Get ACMP result invertion.
 *
 * @param [in] Base: acmp module base
 * @return whether result invertion is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetInvertMode(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR0, ACMP_CR0_INVT_Msk) >> ACMP_CR0_INVT_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set ACMP hall output enable flag.
 *
 * @param [in] Base: acmp module base
 * @param [in] State: enable or disable hall output enable flag
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetHallOutputEnableFlag(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR0, ACMP_CR0_OPE_Msk, ACMP_CR0_OPE_Pos, (TRUE == State) ? 1UL : 0UL);
}

/*!
 * @brief Get ACMP hall output enable flag.
 *
 * @param [in] Base: acmp module base
 * @return whether hall output enable flag is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetHallOutputEnableFlag(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR0, ACMP_CR0_OPE_Msk) >> ACMP_CR0_OPE_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set ACMP hall output A channel.
 *
 * @param [in] Base: acmp module base
 * @param [in] HallCh: hall channel
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetHallOutputA(ACMP_Type *Base, Acmp_ChNumberType HallCh)
{
    MODIFY_REG32(Base->OPA, ACMP_OPA_OPASEL_Msk, ACMP_OPA_OPASEL_Pos, HallCh);
}

/*!
 * @brief Get ACMP hall output A channel.
 *
 * @param [in] Base: acmp module base
 * @return input channel of hall output
 */
LOCAL_INLINE Acmp_ChNumberType Acmp_Reg_GetHallOutputA(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->OPA, ACMP_OPA_OPASEL_Msk) >> ACMP_OPA_OPASEL_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_ChNumberType)Val;
    /*PRQA S 4342 -- */
}

/*!
 * @brief Set ACMP hall output B channel.
 *
 * @param [in] Base: acmp module base
 * @param [in] HallCh: hall channel
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetHallOutputB(ACMP_Type *Base, Acmp_ChNumberType HallCh)
{
    MODIFY_REG32(Base->OPB, ACMP_OPB_OPBSEL_Msk, ACMP_OPB_OPBSEL_Pos, HallCh);
}

/*!
 * @brief Get ACMP hall output B channel.
 *
 * @param [in] Base: acmp module base
 * @return input channel of hall output
 */
LOCAL_INLINE Acmp_ChNumberType Acmp_Reg_GetHallOutputB(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->OPB, ACMP_OPB_OPBSEL_Msk) >> ACMP_OPB_OPBSEL_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_ChNumberType)Val;
    /*PRQA S 4342 -- */
}

/*!
 * @brief Set ACMP hall output C channel.
 *
 * @param [in] Base: acmp module base
 * @param [in] HallCh: hall channel
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetHallOutputC(ACMP_Type *Base, Acmp_ChNumberType HallCh)
{
    MODIFY_REG32(Base->OPC, ACMP_OPC_OPCSEL_Msk, ACMP_OPC_OPCSEL_Pos, HallCh);
}

/*!
 * @brief Get ACMP hall output C channel.
 *
 * @param [in] Base: acmp module base
 * @return input channel of hall output
 */
LOCAL_INLINE Acmp_ChNumberType Acmp_Reg_GetHallOutputC(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->OPC, ACMP_OPC_OPCSEL_Msk) >> ACMP_OPC_OPCSEL_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_ChNumberType)Val;
    /*PRQA S 4342 -- */
}

/*!
 * @brief Set ACMP DAC module enable flag.
 *
 * @param [in] Base: acmp module base
 * @param [in] State: enable or disable DAC module
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetDACEnableFlag(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR2, ACMP_CR2_DACEN_Msk, ACMP_CR2_DACEN_Pos, (TRUE == State) ? 1UL : 0UL);
}

/*!
 * @brief Get ACMP DAC module enable flag.
 *
 * @param [in] Base: acmp module base
 * @return whether DAC module is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetDACEnableFlag(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR2, ACMP_CR2_DACEN_Msk) >> ACMP_CR2_DACEN_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set ACMP DAC output code value.
 *
 * @param [in] Base: acmp module base
 * @param [in] Value: DAC output code value
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetDACOutput(ACMP_Type *Base, uint8 Value)
{
    MODIFY_REG32(Base->CR2, ACMP_CR2_DACVAL_Msk, ACMP_CR2_DACVAL_Pos, Value);
}

/*!
 * @brief Get ACMP DAC output code value.
 *
 * @param [in] Base: acmp module base
 * @return current DAC output code value
 */
LOCAL_INLINE uint8 Acmp_Reg_GetDACOutput(const ACMP_Type *Base)
{
    return (uint8)(READ_BIT32(Base->CR2, ACMP_CR2_DACVAL_Msk) >> ACMP_CR2_DACVAL_Pos);
}

/*!
 * @brief Set ACMP DAC reference source.
 *
 * @param [in] Base: acmp module base
 * @param [in] Ref: reference source
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetDACReference(ACMP_Type *Base, Acmp_VoltageReferenceType Ref)
{
    MODIFY_REG32(Base->CR2, ACMP_CR2_DACREF_Msk, ACMP_CR2_DACREF_Pos, Ref);
}

/*!
 * @brief Get ACMP DAC reference source.
 *
 * @param [in] Base: acmp module base
 * @return DAC reference source
 */
LOCAL_INLINE Acmp_VoltageReferenceType Acmp_Reg_GetDACReference(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->CR2, ACMP_CR2_DACREF_Msk) >> ACMP_CR2_DACREF_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_VoltageReferenceType)Val;
    /*PRQA S 4342 -- */
}

#if defined (AC7842X)  || defined (AC7843X)
/*!
 * @brief Set ACMP DAC output to pin.
 *
 * @param [in] Base: acmp module base
 * @param [in] Enable: enable or disable DAC output
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetDACOutputEnableFlag(ACMP_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->CR2, ACMP_CR2_DACTOPIN_EN_Msk, ACMP_CR2_DACTOPIN_EN_Pos, (TRUE == Enable) ? 1UL : 0UL);
}

/*!
 * @brief Get ACMP DAC output enable flag.
 *
 * @param [in] Base: acmp module base
 * @return whether DAC output is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetDACOutputEnableFlag(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR2, ACMP_CR2_DACTOPIN_EN_Msk) >> ACMP_CR2_DACTOPIN_EN_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set ACMP DAC buffer.
 *
 * @param [in] Base: acmp module base
 * @param [in] Enable: enable or disable DAC buffer
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetDACBufferEnableFlag(ACMP_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->CR2, ACMP_CR2_DAC_BUF_EN_Msk, ACMP_CR2_DAC_BUF_EN_Pos, (TRUE == Enable) ? 1UL : 0UL);
    MODIFY_REG32(Base->CR2, ACMP_CR2_DAC_BUFBYPASS_EN_Msk, ACMP_CR2_DAC_BUFBYPASS_EN_Pos, \
                 (TRUE == Enable) ? 0UL : 1UL);
}

/*!
 * @brief Get ACMP DAC buffer enable flag.
 *
 * @param [in] Base: acmp module base
 * @return whether DAC output is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetDACBufferEnableFlag(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR2, ACMP_CR2_DAC_BUF_EN_Msk) >> ACMP_CR2_DAC_BUF_EN_Pos;
    uint32 tmp1 = READ_BIT32(Base->CR2, ACMP_CR2_DAC_BUFBYPASS_EN_Msk) >> ACMP_CR2_DAC_BUFBYPASS_EN_Pos;
    return ((1U == tmp) && (0U == tmp1)) ? TRUE : FALSE;
}
#endif

/*!
 * @brief Set ACMP positive polling enable flag. Should not enable positive polling and negative polling.
 *
 * @param [in] Base: acmp module base
 * @param [in] State: enable or disable positive polling
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetPositivePollingEnableFlag(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR3, ACMP_CR3_PSPLEN_Msk, ACMP_CR3_PSPLEN_Pos, (TRUE == State) ? 1UL : 0UL);
}

/*!
 * @brief Get ACMP positive polling enable flag.
 *
 * @param [in] Base: acmp module base
 * @return whether positive polling is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetPositivePollingEnableFlag(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR3, ACMP_CR3_PSPLEN_Msk) >> ACMP_CR3_PSPLEN_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set ACMP negative polling enable flag. Should not enable positive polling and negative polling.
 *
 * @param [in] Base: acmp module base
 * @param [in] State: enable or disable negative polling
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetNegativePollingEnableFlag(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR3, ACMP_CR3_NSPLEN_Msk, ACMP_CR3_NSPLEN_Pos, (TRUE == State) ? 1UL : 0UL);
}

/*!
 * @brief Get ACMP negative polling enable flag.
 *
 * @param [in] Base: acmp module base
 * @return whether negative polling is enabled
 */
LOCAL_INLINE boolean Acmp_Reg_GetNegativePollingEnableFlag(const ACMP_Type *Base)
{
    uint32 tmp = READ_BIT32(Base->CR3, ACMP_CR3_NSPLEN_Msk) >> ACMP_CR3_NSPLEN_Pos;
    return (tmp != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set ACMP polling sequence
 *
 * @param [in] Base: acmp module base
 * @param [in] Seq: channel sequence that need to poll, each bit corresponding to one channel
 *                  bit 0 represent channel 0 and other bit is likewise.
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetPollingSequence(ACMP_Type *Base, uint16 Seq)
{
    MODIFY_REG32(Base->CR4, ACMP_CR4_PLSEQ_Msk, ACMP_CR4_PLSEQ_Pos, Seq);
}

/*!
 * @brief Get ACMP polling sequence
 *
 * @param [in] Base: acmp module base
 * @return current polling channel sequence
 */
LOCAL_INLINE uint16 Acmp_Reg_GetPollingSequence(const ACMP_Type *Base)
{
    return (uint16)(READ_BIT32(Base->CR4, ACMP_CR4_PLSEQ_Msk) >> ACMP_CR4_PLSEQ_Pos);
}

/*!
 * @brief Set ACMP polling clock divide.
 *
 * @param [in] Base: acmp module base
 * @param [in] Div: polling clock divide
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetPollingFreqDiv(ACMP_Type *Base, Acmp_PollingClkDivideType Div)
{
    MODIFY_REG32(Base->FD, ACMP_FD_PLFD_Msk, ACMP_FD_PLFD_Pos, Div);
}

/*!
 * @brief Get ACMP polling clock divide.
 *
 * @param [in] Base: acmp module base
 * @return current polling clock divide setting
 */
LOCAL_INLINE Acmp_PollingClkDivideType Acmp_Reg_GetPollingFreqDiv(const ACMP_Type *Base)
{
    uint32 Val = (READ_BIT32(Base->FD, ACMP_FD_PLFD_Msk) >> ACMP_FD_PLFD_Pos);
    /*PRQA S 4342 ++ # AMCE_ACMP_4342.*/
    return (Acmp_PollingClkDivideType)Val;
    /*PRQA S 4342 -- */
}

/*!
 * @brief Get ACMP compare result in normal compare mode.
 *
 * @param [in] Base: acmp module base
 * @return compare result
 */
LOCAL_INLINE uint8 Acmp_Reg_GetOutputData(const ACMP_Type *Base)
{
    return (uint8)(READ_BIT32(Base->DR, ACMP_DR_O_Msk) >> ACMP_DR_O_Pos);
}

/*!
 * @brief Get ACMP polling result.
 *
 * @param [in] Base: acmp module base
 * @return polling channel result. Bits of Channels that not polling will be 0
 */
LOCAL_INLINE uint16 Acmp_Reg_GetPollingData(const ACMP_Type *Base)
{
    return (uint16)READ_BIT32(Base->DR, ACMP_DR_POLLING_O_Msk);
}

/*!
 * @brief Get ACMP normal comparator mode interrupt status.
 *
 * @param [in] Base: acmp module base
 * @return normal mode interrupt status
 */
LOCAL_INLINE uint8 Acmp_Reg_GetOutputStatus(const ACMP_Type *Base)
{
    return (uint8)(READ_BIT32(Base->SR, ACMP_SR_F_Msk) >> ACMP_SR_F_Pos);
}

/*!
 * @brief Clear ACMP normal comparator mode interrupt status.
 *
 * @param [in] Base: acmp module base
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_ClearOutputStatus(ACMP_Type *Base)
{
    WRITE_REG32(Base->SR, ACMP_SR_F_Msk);
}

/*!
 * @brief Get ACMP polling mode interrupt status.
 *
 * @param [in] Base: acmp module base
 * @return polling mode interrupt status
 */
LOCAL_INLINE uint16 Acmp_Reg_GetPollingStatus(const ACMP_Type *Base)
{
    return (uint16)READ_BIT32(Base->SR, ACMP_SR_POLLING_F_Msk);
}

/*!
 * @brief Clear ACMP polling mode interrupt status.
 *
 * @param [in] Base: acmp module base
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_ClearPollingStatus(ACMP_Type *Base)
{
    WRITE_REG32(Base->SR, ACMP_SR_POLLING_F_Msk);
}

/*!
 * @brief Get ACMP interrupt status.
 *
 * @param [in] Base: acmp module base
 * @return interrupt status
 */
LOCAL_INLINE uint32 Acmp_Reg_GetInterruptStatus(const ACMP_Type *Base)
{
    return READ_REG32(Base->SR);
}

/*!
 * @brief Clear ACMP interrupt status.
 *
 * @param [in] Base: acmp module base
 * @param [in] Status: interrupt status, write 1 to clear interrupt status
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_ClearInterruptStatus(ACMP_Type *Base, uint32 Status)
{
    WRITE_REG32(Base->SR, Status);
}

#if defined (AC7840X)
/*!
 * @brief Config ACMP lowpower mode enable flag
 *
 * @param [in] State: enable or disable lowpower mode
 * @return void
 */
LOCAL_INLINE void Acmp_Reg_SetLPEnableFlag(boolean State)
{
    MODIFY_REG32(ANA->ACMPDAC_CFG, ANA_ACMPDAC_CFG_RG_MCU_ACMPDAC_LP_EN_Msk, ANA_ACMPDAC_CFG_RG_MCU_ACMPDAC_LP_EN_Pos, \
                 (TRUE == State) ? 1U : 0U);
}
#endif

#if defined (AC7842X)
/*!
 * @brief Config ACMP lowpower mode enable flag
 *
 * @param [in] Base: acmp module base
 * @param[in] State: enable or disable lowpower mode
 */
LOCAL_INLINE void Acmp_Reg_SetLPEnableFlag(ACMP_Type *Base, boolean State)
{
    MODIFY_REG32(Base->CR2, ACMP_CR2_LP_EN_Msk, ACMP_CR2_LP_EN_Pos, State);
}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AC784XX_ACMP_REG_H */

/* =============================================  EOF  ============================================== */
