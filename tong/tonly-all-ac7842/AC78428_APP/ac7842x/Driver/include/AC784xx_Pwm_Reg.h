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
 * @file AC784xx_Pwm_Reg.h
 *
 * @brief This file provides pwm hardware integration interface.
 *
 */

#ifndef AC784XX_PWM_REG_H
#define AC784XX_PWM_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/**
 * @brief Get PWM base address by instance
 * @note DES ID: DES_PWM_API_301
 * @param[in] Instance PWM instance
 * @return PWM base address
 */
LOCAL_INLINE PWM_Type *Pwm_Reg_GetBase(uint8 Instance)
{
    /* PRQA S 0306 ++ */ /* Cast between a pointer to object and an integral type. */
#if defined (PWM_BASE_PTRS)
    PWM_Type *const PwmBaseAddr[PWM_INSTANCE_MAX] = PWM_BASE_PTRS;
#else
    PWM_Type *const PwmBaseAddr[PWM_INSTANCE_MAX] = {PWM0, PWM1, PWM2, PWM3, PWM4, PWM5};
#endif
    /* PRQA S 0306 -- */ /* Cast between a pointer to object and an integral type. */

    PWM_Type *Base = PwmBaseAddr[Instance];

    return Base;
}

/**
 * @brief Set PWM clock source
 * @note DES ID: DES_PWM_API_302
 * @param[in] Base PWM base address
 * @param[in] ClkSource Clock source selection
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetClockSource(PWM_Type *Base, uint8 ClkSource)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_CLKSRC_Msk, PWM_INIT_CLKSRC_Pos, ClkSource);
}

/**
 * @brief Set PWM clock prescaler
 * @note DES ID: DES_PWM_API_303
 * @param[in] Base PWM base address
 * @param[in] ClkPsc Clock prescaler value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetClockPsc(PWM_Type *Base, uint16 ClkPsc)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_CLKPSC_Msk, PWM_INIT_CLKPSC_Pos, ClkPsc);
}

/**
 * @brief Set PWM counter mode
 * @note DES ID: DES_PWM_API_304
 * @param[in] Base PWM base address
 * @param[in] Mode Counter mode (up or up-down)
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetCountMode(PWM_Type *Base, uint8 Mode)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_CNTMODE_Msk, PWM_INIT_CNTMODE_Pos, Mode);
}

/**
 * @brief Set PWM maximum count value
 * @note DES ID: DES_PWM_API_305
 * @param[in] Base PWM base address
 * @param[in] Value Maximum count value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetMaxCountValue(PWM_Type *Base, uint16 Value)
{
    WRITE_REG32(Base->MCVR, Value);
}

/**
 * @brief Get PWM maximum count value
 * @note DES ID: DES_PWM_API_306
 * @param[in] Base PWM base address
 * @return Maximum count value
 */
LOCAL_INLINE uint16 Pwm_Reg_GetMaxCountValue(const PWM_Type *Base)
{
    uint32 Value = READ_BIT32(Base->MCVR, PWM_MCVR_MCVR_Msk);

    return (uint16)Value;
}

/**
 * @brief Set PWM initial count value
 * @note DES ID: DES_PWM_API_307
 * @param[in] Base PWM base address
 * @param[in] Value Initial count value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetInitCountValue(PWM_Type *Base, uint16 Value)
{
    WRITE_REG32(Base->CNTIN, Value);
}

/**
 * @brief Get PWM initial count value
 * @note DES ID: DES_PWM_API_308
 * @param[in] Base PWM base address
 * @return Initial count value
 */
LOCAL_INLINE uint16 Pwm_Reg_GetInitCountValue(const PWM_Type *Base)
{
    uint32 Value = READ_BIT32(Base->CNTIN, PWM_CNTIN_CNTINIT_Msk);

    return (uint16)Value;
}

/**
 * @brief Reset PWM counter
 * @note DES ID: DES_PWM_API_309
 * @param[in] Base PWM base address
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_ResetCounter(PWM_Type *Base)
{
    WRITE_REG32(Base->CNT, 0UL);
}

/**
 * @brief Get current PWM counter value
 * @note DES ID: DES_PWM_API_310
 * @param[in] Base PWM base address
 * @return Current counter value
 */
LOCAL_INLINE uint16 Pwm_Reg_GetCountValue
(
    const PWM_Type *Base
)
{
    uint32 Flag = READ_BIT32(Base->CNT, PWM_CNT_COUNT_Msk);

    return (uint16)Flag;
}

/**
 * @brief Set period dither value
 * @note DES ID: DES_PWM_API_311
 * @param[in] Base PWM base address
 * @param[in] Value Period dither value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetPeriodDither(PWM_Type *Base, uint8 Value)
{
    MODIFY_REG32(Base->DITHER2, PWM_DITHER2_PDHR_Msk, PWM_DITHER2_PDHR_Pos, Value);
}

/**
 * @brief Set counter overflow frequency
 * @note DES ID: DES_PWM_API_312
 * @param[in] Base PWM base address
 * @param[in] Freq Overflow frequency
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetCntOverflowFreq(PWM_Type *Base, uint8 Freq)
{
    MODIFY_REG32(Base->CONF, PWM_CONF_CNTOFNUM_Msk, PWM_CONF_CNTOFNUM_Pos, Freq);
}

/**
 * @brief Enable/disable overflow DMA request
 * @note DES ID: DES_PWM_API_313
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableOverflowDmaReq(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_OFUFDMAEN_Msk, PWM_INIT_OFUFDMAEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable DMA transfer
 * @note DES ID: DES_PWM_API_314
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableDmaTransfer(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->DMACTRL, PWM_DMACTRL_TRANS_EN_Msk, PWM_DMACTRL_TRANS_EN_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set DMA transfer length
 * @note DES ID: DES_PWM_API_315
 * @param[in] Base PWM base address
 * @param[in] Len DMA transfer length
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetDmaTransferLen(PWM_Type *Base, uint8 Len)
{
    MODIFY_REG32(Base->DMACTRL, PWM_DMACTRL_TRANS_LEN_Msk, PWM_DMACTRL_TRANS_LEN_Pos, Len);
}

/**
 * @brief Enable/disable output initialization
 * @note DES ID: DES_PWM_API_316
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableOutputInit(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_INIT_Msk, PWM_FUNCSEL_INIT_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable channel DMA request
 * @note DES ID: DES_PWM_API_317
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableChannelDmaReq(PWM_Type *Base, uint8 Channel, boolean Enable)
{
    MODIFY_REG32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_DMAEN_Msk, PWM_CH0SCR_DMAEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set channel mode selection register
 * @note DES ID: DES_PWM_API_318
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Value Mode selection value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetChannelMSR(PWM_Type *Base, uint8 Channel, uint8 Value)
{
    MODIFY_REG32(Base->CHANNELS[Channel].CHnSCR, \
                 PWM_CH0SCR_MSR0_Msk | PWM_CH0SCR_MSR1_Msk, \
                 PWM_CH0SCR_MSR0_Pos, \
                 ((uint32)Value & 0x03UL));
}

/**
 * @brief Set channel edge level selection register
 * @note DES ID: DES_PWM_API_319
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Value Edge level selection value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetChannelELSR(PWM_Type *Base, uint8 Channel, uint8 Value)
{
    MODIFY_REG32(Base->CHANNELS[Channel].CHnSCR, \
                 PWM_CH0SCR_ELSR0_Msk | PWM_CH0SCR_ELSR1_Msk, \
                 PWM_CH0SCR_ELSR0_Pos, \
                 ((uint32)Value & 0x03UL));
}

/**
 * @brief Set channel value
 * @note DES ID: DES_PWM_API_320
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Value Channel value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetChannelValue(PWM_Type *Base, uint8 Channel, uint16 Value)
{
    WRITE_REG32((Base)->CHANNELS[Channel].CHnV, Value);
}

/**
 * @brief Get channel value
 * @note DES ID: DES_PWM_API_321
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @return Channel value
 */
LOCAL_INLINE uint16 Pwm_Reg_GetChannelValue(const PWM_Type *Base, uint8 Channel)
{
    uint32 Value = READ_BIT32((Base)->CHANNELS[Channel].CHnV, PWM_CH0V_CHCVAL_Msk);

    return (uint16)Value;
}

/**
 * @brief Set channel match dither register
 * @note DES ID: DES_PWM_API_322
 * @param[in] Base PWM base address
 * @param[in] DitherRegNum Dither register number
 * @param[in] RegValue Register value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetChannelMatchDitherReg(PWM_Type *Base, uint8 DitherRegNum, uint32 RegValue)
{
    if (0U == DitherRegNum)
    {
        WRITE_REG32(Base->DITHER0, RegValue);
    }
    else
    {
        WRITE_REG32(Base->DITHER1, RegValue);
    }
}

/**
 * @brief Set channel match dither value
 * @note DES ID: DES_PWM_API_323
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] DitherValue Dither value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetChannelMatchDither(PWM_Type *Base, uint8 Channel, uint8 DitherValue)
{
    if (4U > Channel)
    {
        MODIFY_REG32(Base->DITHER0, \
                     (uint32)PWM_DITHER0_C0DHR_Msk << (PWM_DITHER_VALUE_WIDTH * ((uint32)Channel % 4U)), \
                     (uint32)PWM_DITHER_VALUE_WIDTH * ((uint32)Channel % 4U), \
                     DitherValue);
    }
    else
    {
        MODIFY_REG32(Base->DITHER1, \
                     (uint32)PWM_DITHER0_C0DHR_Msk << (PWM_DITHER_VALUE_WIDTH * ((uint32)Channel % 4U)), \
                     (uint32)PWM_DITHER_VALUE_WIDTH * ((uint32)Channel % 4U), \
                     DitherValue);
    }

}

/**
 * @brief Enable/disable overflow interrupt
 * @note DES ID: DES_PWM_API_324
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableOverflowInterrupt(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_CNTOIE_Msk, PWM_INIT_CNTOIE_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable channel interrupt
 * @note DES ID: DES_PWM_API_325
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableChannelInterrupt(PWM_Type *Base, uint8 Channel, boolean Enable)
{
    MODIFY_REG32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_CHIE_Msk, PWM_CH0SCR_CHIE_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable overflow event
 * @note DES ID: DES_PWM_API_326
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableOverflowEvent(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_COFE_Msk, PWM_INIT_COFE_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable underflow event
 * @note DES ID: DES_PWM_API_327
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableUnderflowEvent(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->INIT, PWM_INIT_CUFE_Msk, PWM_INIT_CUFE_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set channel output initial level
 * @note DES ID: DES_PWM_API_328
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Level Output level
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetChannelOutputInitLevel
(
    PWM_Type *Base,
    uint8 Channel,
    uint8 Level
)
{
    MODIFY_REG32(Base->OUTINIT, (uint32)PWM_OUTINIT_CH0OIV_Msk << (Channel), (Channel), Level);
}

/**
 * @brief Enable/disable channel pair combination
 * @note DES ID: DES_PWM_API_329
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableChannelCombine(PWM_Type *Base, uint8 ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, ((uint32)PWM_MODESEL_PAIR0COMBINEN_Msk << (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable pair channel symmetric mode
 * @note DES ID: DES_PWM_API_330
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnablePairChSymmetric(PWM_Type *Base, uint8 ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, (uint32)PWM_MODESEL_PAIR0SYMEN_Msk << (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), \
                 PWM_MODESEL_PAIR0SYMEN_Pos + (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), \
                 (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable pair channel complement mode
 * @note DES ID: DES_PWM_API_331
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnablePairChComplement(PWM_Type *Base, uint8 ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, ((uint32)PWM_MODESEL_PAIR0COMPEN_Msk << (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 PWM_MODESEL_PAIR0COMPEN_Pos + (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), \
                 (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable pair channel value synchronization
 * @note DES ID: DES_PWM_API_332
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnablePairChValueSync(PWM_Type *Base, uint8 ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, (uint32)PWM_MODESEL_PAIR0SYNCEN_Msk << (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH),
                 PWM_MODESEL_PAIR0SYNCEN_Pos + (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), \
                 (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable pair channel deadtime
 * @note DES ID: DES_PWM_API_333
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnablePairChDeadtime(PWM_Type *Base, uint8 ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, ((uint32)PWM_MODESEL_PAIR0DTEN_Msk << (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (PWM_MODESEL_PAIR0DTEN_Pos + (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set pair channel deadtime
 * @note DES ID: DES_PWM_API_334
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @param[in] Psc Deadtime prescaler
 * @param[in] Value Deadtime value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetPairChDeadtime
(
    PWM_Type *Base,
    uint8 ChannelPair,
    uint8 Psc,
    uint16 Value
)
{
    if (ChannelPair < 2U)
    {
        MODIFY_REG32(Base->DTSET0, \
                     ((uint32)PWM_DTSET0_DTPSC0_Msk << ((uint32)ChannelPair * PWM_DTSET_CONFIG_WIDTH)), \
                     (PWM_DTSET0_DTPSC0_Pos + ((uint32)ChannelPair * PWM_DTSET_CONFIG_WIDTH)), \
                     (uint32)Psc);
        MODIFY_REG32(Base->DTSET0, \
                     ((uint32)PWM_DTSET0_DTVAL0_Msk << ((uint32)ChannelPair * PWM_DTSET_CONFIG_WIDTH)), \
                     (PWM_DTSET0_DTVAL0_Pos + ((uint32)ChannelPair * PWM_DTSET_CONFIG_WIDTH)), \
                     (uint32)Value);
    }
    else if (ChannelPair < 4U)
    {
        MODIFY_REG32(Base->DTSET1, \
                     ((uint32)PWM_DTSET1_DTPS2_Msk << (((uint32)ChannelPair - 2UL) * PWM_DTSET_CONFIG_WIDTH)), \
                     (PWM_DTSET1_DTPS2_Pos + (((uint32)ChannelPair - 2UL) * PWM_DTSET_CONFIG_WIDTH)), \
                     Psc);
        MODIFY_REG32(Base->DTSET1, \
                     ((uint32)PWM_DTSET1_DTVAL2_Msk << (((uint32)ChannelPair - 2UL) * PWM_DTSET_CONFIG_WIDTH)), \
                     (PWM_DTSET1_DTVAL2_Pos + (((uint32)ChannelPair - 2UL) * PWM_DTSET_CONFIG_WIDTH)), \
                     (uint32)Value);
    }
    else
    {
        /* Reserve */
    }
}

/**
 * @brief Enable/disable pair channel invert
 * @note DES ID: DES_PWM_API_335
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnablePairChInvert(PWM_Type *Base, uint8 ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->INVCR, (uint32)PWM_INVCR_PAIR0INVEN_Msk << ChannelPair, ChannelPair,
                 (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set combine center duty type
 * @note DES ID: DES_PWM_API_336
 * @param[in] Base PWM base address
 * @param[in] DutyMode Duty mode
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetCombineCenterDutyType(PWM_Type *Base, uint8 DutyMode)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_PDYTPEN_Msk, PWM_FUNCSEL_PDYTPEN_Pos, DutyMode);
}

/**
 * @brief Set channel match direction
 * @note DES ID: DES_PWM_API_337
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Dir Match direction
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetChannelMatchDir(PWM_Type *Base, uint8 Channel,
        uint8 Dir)
{
    MODIFY_REG32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_DIR_Msk, PWM_CH0SCR_DIR_Pos, Dir);
}

/**
 * @brief Enable/disable pair pulse width measurement
 * @note DES ID: DES_PWM_API_338
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @param[in] State TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnablePairPulseWidthMeasurement(PWM_Type *Base, uint8 ChannelPair,
        boolean State)
{
    MODIFY_REG32(Base->FUNCSEL, (uint32)PWM_FUNCSEL_CHP0PNWEN_Msk << (uint32)ChannelPair,
                 PWM_FUNCSEL_CHP0PNWEN_Pos + (uint32)ChannelPair, State);
}

/**
 * @brief Enable/disable hall sensor interface
 * @note DES ID: DES_PWM_API_339
 * @param[in] Base PWM base address
 * @param[in] State TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableHall(PWM_Type *Base, boolean State)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_HALLEN_Msk, PWM_FUNCSEL_HALLEN_Pos, (FALSE != State) ? 1U : 0U);
}

/**
 * @brief Get hall status
 * @note DES ID: DES_PWM_API_340
 * @param[in] Base PWM base address
 * @return Hall status
 */
LOCAL_INLINE uint32 Pwm_Reg_GetHallStatus(const PWM_Type *Base)
{
    uint32 Status = READ_BIT32((Base)->STR, PWM_STR_HALL_STATUS_Msk);
    Status = Status >> PWM_STR_HALL_STATUS_Pos;

    return Status;
}

/**
 * @brief Enable/disable initialization trigger
 * @note DES ID: DES_PWM_API_341
 * @param[in] Base PWM base address
 * @param[in] State TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableInitTrigger(PWM_Type *Base, boolean State)
{
    MODIFY_REG32(Base->EXTTRIG, PWM_EXTTRIG_INITTRIGEN_Msk, PWM_EXTTRIG_INITTRIGEN_Pos, (FALSE != State) ? 1U : 0U);
}

/**
 * @brief Enable/disable maximum trigger
 * @note DES ID: DES_PWM_API_342
 * @param[in] Base PWM base address
 * @param[in] State TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableMaxTrigger(PWM_Type *Base, boolean State)
{
    MODIFY_REG32(Base->EXTTRIG, PWM_EXTTRIG_MAXTRIGEN_Msk, PWM_EXTTRIG_MAXTRIGEN_Pos, (FALSE != State) ? 1U : 0U);
}

/**
 * @brief Enable/disable channel match trigger
 * @note DES ID: DES_PWM_API_343
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] State TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableChannelMatchTrigger(PWM_Type *Base, uint8 Channel, boolean State)
{
    MODIFY_REG32(Base->EXTTRIG, (uint32)PWM_EXTTRIG_CH0TRIG_Msk << Channel, Channel, (FALSE != State) ? 1U : 0U);
}

/**
 * @brief Set trigger ratio
 * @note DES ID: DES_PWM_API_344
 * @param[in] Base PWM base address
 * @param[in] Ratio Trigger ratio (0-7)
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetTriggerRatio(PWM_Type *Base, uint8 Ratio)
{
    DEVICE_ASSERT(Ratio <= 0x07U);
    MODIFY_REG32(Base->EXTTRIG, PWM_EXTTRIG_TRIGRATIO_Msk, PWM_EXTTRIG_TRIGRATIO_Pos, Ratio);
}

/**
 * @brief Get channel trigger flag
 * @note DES ID: DES_PWM_API_345
 * @param[in] Base PWM base address
 * @return Trigger flag
 */
LOCAL_INLINE uint32 Pwm_Reg_GetChannelTriggerFlag(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32(Base->EXTTRIG, PWM_EXTTRIG_TRIGF_Msk);
    Flag = Flag >> PWM_EXTTRIG_TRIGF_Pos;

    return Flag;
}

/**
 * @brief Clear channel trigger flag
 * @note DES ID: DES_PWM_API_346
 * @param[in] Base PWM base address
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_ClearChannelTriggerFlag(PWM_Type *Base)
{
#if defined (AC7843X)
    SET_BIT32(Base->EXTTRIG, PWM_EXTTRIG_TRIGF_Msk);
#else
    CLEAR_BIT32(Base->EXTTRIG, PWM_EXTTRIG_TRIGF_Msk);
#endif
}

/**
 * @brief Enable/disable channel output mask
 * @note DES ID: DES_PWM_API_347
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableChannelOutputMask(PWM_Type *Base, uint8 Channel, boolean Enable)
{
    MODIFY_REG32(Base->OMCR, (uint32)PWM_OMCR_CH0OMEN_Msk << Channel, Channel, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set output mask
 * @note DES ID: DES_PWM_API_348
 * @param[in] Base PWM base address
 * @param[in] Mask Output mask
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetOutputMask(PWM_Type *Base, uint8 Mask)
{
    MODIFY_REG32(Base->OMCR, PWM_OMCR_CH_Msk, PWM_OMCR_CH0OMEN_Pos, Mask);
}

/**
 * @brief Get output mask
 * @note DES ID: DES_PWM_API_349
 * @param[in] Base PWM base address
 * @return Output mask
 */
LOCAL_INLINE uint32 Pwm_Reg_GetOutputMask(const PWM_Type *Base)
{
    uint32 Mask = READ_BIT32(Base->OMCR, PWM_OMCR_CH_Msk);

    return Mask;
}

/**
 * @brief Set channel polarity
 * @note DES ID: DES_PWM_API_350
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Polarity Active polarity
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetChannelPolarity(PWM_Type *Base, uint8 Channel,
        uint8 Polarity)
{
    MODIFY_REG32(Base->CHOPOLCR, (uint32)PWM_CHOPOLCR_CH0POL_Msk << Channel, Channel, Polarity);
}

/**
 * @brief Get all channel levels
 * @note DES ID: DES_PWM_API_351
 * @param[in] Base PWM base address
 * @return Channel levels
 */
LOCAL_INLINE uint32 Pwm_Reg_GetAllChannelLevel(const PWM_Type *Base)
{
    uint32 Levels = READ_BIT32(Base->STR, PWM_STR_CH_INPUT_Msk);
    Levels = Levels >> PWM_STR_CH0STS_Pos;

    return Levels;
}

/**
 * @brief Get overflow flag
 * @note DES ID: DES_PWM_API_352
 * @param[in] Base PWM base address
 * @return Overflow flag
 */
LOCAL_INLINE uint32 Pwm_Reg_GetOverflowFlag(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32(Base->INIT, PWM_INIT_CNTOF_Msk);
    Flag = Flag >> PWM_INIT_CNTOF_Pos;

    return Flag;
}

/**
 * @brief Clear overflow flag
 * @note DES ID: DES_PWM_API_353
 * @param[in] Base PWM base address
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_ClearOverflowFlag(PWM_Type *Base)
{
#if defined (AC7843X)
    uint32 Value = READ_REG32(Base->INIT);
    WRITE_REG32(Base->INIT, Value | PWM_INIT_CNTOF_Msk | PWM_INIT_OFDIR_Msk);
#else
    CLEAR_BIT32(Base->INIT, PWM_INIT_CNTOF_Msk);
#endif
}

/**
 * @brief Get overflow direction
 * @note DES ID: DES_PWM_API_354
 * @param[in] Base PWM base address
 * @return Overflow direction
 */
LOCAL_INLINE uint32 Pwm_Reg_GetOverflowDir(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32(Base->INIT, PWM_INIT_OFDIR_Msk);
    Flag = Flag >> PWM_INIT_OFDIR_Pos;

    return Flag;
}

/**
 * @brief Get channel event flag
 * @note DES ID: DES_PWM_API_355
 * @param[in] Base PWM base address
 * @return Channel event flag
 */
LOCAL_INLINE uint32 Pwm_Reg_GetChannelEventFlag(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32(Base->STR, PWM_STR_CH_EVENT_Msk);

    return Flag;
}

/**
 * @brief Clear channel event flag
 * @note DES ID: DES_PWM_API_356
 * @param[in] Base PWM base address
 * @param[in] mask Event mask
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_ClearChannelEventFlag(PWM_Type *Base, uint32 mask)
{
#if defined (AC7843X)
    /* All status bits of this register are cleared by writing 1 */
    WRITE_REG32(Base->STR, mask);
#else
    CLEAR_BIT32(Base->STR, mask);
#endif
}

/**
 * @brief Get channel interrupt flag
 * @note DES ID: DES_PWM_API_357
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @return Channel interrupt flag
 */
LOCAL_INLINE uint32 Pwm_Reg_GetChannelInterruptFlag(const PWM_Type *Base, uint8 Channel)
{
    uint32 Flag = READ_BIT32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_CHIF_Msk);
    Flag = Flag >> PWM_CH0SCR_CHIF_Pos;

    return Flag;
}

/**
 * @brief Clear channel interrupt flag
 * @note DES ID: DES_PWM_API_358
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_ClearChannelInterruptFlag(PWM_Type *Base, uint8 Channel)
{
#if defined (AC7843X)
    SET_BIT32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_CHIF_Msk);
#else
    CLEAR_BIT32(Base->CHANNELS[Channel].CHnSCR, PWM_CH0SCR_CHIF_Msk);
#endif
}

/**
 * @brief Enable/disable channel software control
 * @note DES ID: DES_PWM_API_359
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] State TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableChannelSoftControl
(
    PWM_Type *Base,
    uint8 Channel,
    boolean State
)
{
    MODIFY_REG32(Base->CHOSWCR, ((uint32)PWM_CHOSWCR_CH0SWEN_Msk << (uint32)Channel), (uint32)Channel, (FALSE != State) ? 1U : 0U);
}

/**
 * @brief Set software control enable states mask
 * @note DES ID: DES_PWM_API_360
 * @param[in] Base PWM base address
 * @param[in] Mask Enable mask
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetSoftControlEnableStatesMask
(
    PWM_Type *Base,
    uint8 Mask
)
{
    MODIFY_REG32(Base->CHOSWCR, PWM_CHOSWCR_CHNSWEN_Msk, PWM_CHOSWCR_CH0SWEN_Pos, Mask);
}

/**
 * @brief Read channel software control state
 * @note DES ID: DES_PWM_API_361
 * @param[in] Base PWM base address
 * @return Software control state
 */
LOCAL_INLINE uint32 Pwm_Reg_ReadChannelSoftControlState
(
    const PWM_Type *Base
)
{
    uint32 Flag = READ_BIT32(Base->CHOSWCR, PWM_CHOSWCR_CHNSWEN_Msk);

    return Flag;
}

/**
 * @brief Set channel software control level
 * @note DES ID: DES_PWM_API_362
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Level Output level
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetChannelSoftControlLevel
(
    PWM_Type *Base,
    uint8 Channel,
    uint8 Level
)
{
    MODIFY_REG32(Base->CHOSWCR, \
                 ((uint32)PWM_CHOSWCR_CH0SWCV_Msk << (uint32)Channel), \
                 (PWM_CHOSWCR_CH0SWCV_Pos + (uint32)Channel), \
                 (uint32)Level);
}

/**
 * @brief Set software control level mask
 * @note DES ID: DES_PWM_API_363
 * @param[in] Base PWM base address
 * @param[in] Mask Level mask
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetSoftControlLevelMask(PWM_Type *Base, uint8 Mask)
{
    MODIFY_REG32(Base->CHOSWCR, PWM_CHOSWCR_CHNSWCV_Msk, PWM_CHOSWCR_CH0SWCV_Pos, Mask);
}

/**
 * @brief Enable/disable synchronization
 * @note DES ID: DES_PWM_API_364
 * @param[in] Base PWM base address
 * @param[in] State TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableSync(PWM_Type *Base, boolean State)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_PWMSYNCEN_Msk, PWM_FUNCSEL_PWMSYNCEN_Pos, (FALSE != State) ? 1U : 0U);
}

/**
 * @brief Set synchronization type
 * @note DES ID: DES_PWM_API_365
 * @param[in] Base PWM base address
 * @param[in] Type Sync type
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetSyncType(PWM_Type *Base, uint8 Type)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_PWMSYNC_Msk, PWM_FUNCSEL_PWMSYNC_Pos, Type);
}

/**
 * @brief Set synchronization mode
 * @note DES ID: DES_PWM_API_366
 * @param[in] Base PWM base address
 * @param[in] Mode Sync mode
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetSyncMode(PWM_Type *Base, uint8 Mode)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_SYNCMODE_Msk, PWM_SYNCONF_SYNCMODE_Pos, Mode);
}

/**
 * @brief Enable/disable sync bypass
 * @note DES ID: DES_PWM_API_367
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableSyncBYP(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_PWM_SYNC_BYPEN_Msk, PWM_SYNC_PWM_SYNC_BYPEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable sync hardware trigger source
 * @note DES ID: DES_PWM_API_368
 * @param[in] Base PWM base address
 * @param[in] HwTrigNum Hardware trigger number
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableSyncHwTriggerSrc(PWM_Type *Base, uint8 HwTrigNum, boolean Enable)
{
    DEVICE_ASSERT(3U > HwTrigNum);
    MODIFY_REG32(Base->SYNC, (uint32)PWM_SYNC_TRIG0_Msk << HwTrigNum, PWM_SYNC_TRIG0_Pos + HwTrigNum, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Trigger software sync
 * @note DES ID: DES_PWM_API_369
 * @param[in] Base PWM base address
 * @param[in] State TRUE to trigger, FALSE to not trigger
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_TrigSoftwareSync(PWM_Type *Base, boolean State)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_SWSYNC_Msk, PWM_SYNC_SWSYNC_Pos, (FALSE != State) ? 1U : 0U);
}

/**
 * @brief Enable/disable max loading point sync
 * @note DES ID: DES_PWM_API_370
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableMaxLoadingPointSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_MAXSYNCP_Msk, PWM_SYNC_MAXSYNCP_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable min loading point sync
 * @note DES ID: DES_PWM_API_371
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableMinLoadingPointSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_MINSYNCP_Msk, PWM_SYNC_MINSYNCP_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable polarity sync
 * @note DES ID: DES_PWM_API_372
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnablePOLSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_PWM_SYNCPOL_Msk, PWM_SYNC_PWM_SYNCPOL_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable output mask sync
 * @note DES ID: DES_PWM_API_373
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableOutputMaskSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_OMSYNCP_Msk, PWM_SYNC_OMSYNCP_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Choose buffer or register to read
 * @note DES ID: DES_PWM_API_374
 * @param[in] Base PWM base address
 * @param[in] ChooseReg TRUE to choose register, FALSE to choose buffer
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_ChooseBufferOrRegisterToRead(PWM_Type *Base, boolean ChooseReg)
{
    MODIFY_REG32(Base->SYNC, PWM_SYNC_PWM_BFRGCHSEN_Msk, PWM_SYNC_PWM_BFRGCHSEN_Pos, (FALSE != ChooseReg) ? 1U : 0U);
}

/**
 * @brief Disable hardware trigger sync after triggered
 * @note DES ID: DES_PWM_API_375
 * @param[in] Base PWM base address
 * @param[in] Disable TRUE to disable, FALSE to enable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_DisableHwTriggerSyncAfterTrigged(PWM_Type *Base, boolean Disable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_HWTRIGMODE_Msk, PWM_SYNCONF_HWTRIGMODE_Pos, (TRUE == Disable) ? 0U : 1U);
}

/**
 * @brief Enable/disable CNTIN sync
 * @note DES ID: DES_PWM_API_376
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableCNTINSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_CNTINC_Msk, PWM_SYNCONF_CNTINC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable INVCR sync
 * @note DES ID: DES_PWM_API_377
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableINVCRSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_INVC_Msk, PWM_SYNCONF_INVC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable CHOSWCR sync
 * @note DES ID: DES_PWM_API_378
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableCHOSWCRSync(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_SWOC_Msk, PWM_SYNCONF_SWOC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set SW polarity trigger mode
 * @note DES ID: DES_PWM_API_379
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetSwPolTriggerMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_SWPOL_Msk, PWM_SYNCONF_SWPOL_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set HW polarity trigger mode
 * @note DES ID: DES_PWM_API_423
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetHwPolTriggerMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_HWPOL_Msk, PWM_SYNCONF_HWPOL_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set CHOSWCR SW sync trigger mode
 * @note DES ID: DES_PWM_API_380
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetCHOSWCRSwSyncTriggerMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_SWVSWSYNC_Msk, PWM_SYNCONF_SWVSWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set CHOSWCR HW sync trigger mode
 * @note DES ID: DES_PWM_API_424
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetCHOSWCRHwSyncTriggerMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_SWVHWSYNC_Msk, PWM_SYNCONF_SWVHWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set INVCR SW sync trigger mode
 * @note DES ID: DES_PWM_API_381
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetInvcrSwSyncTriggerMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_INVSWSYNC_Msk, PWM_SYNCONF_INVSWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set INVCR HW sync trigger mode
 * @note DES ID: DES_PWM_API_425
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetInvcrHwSyncTriggerMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_INVHWSYNC_Msk, PWM_SYNCONF_INVHWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set OMCR SW sync trigger mode
 * @note DES ID: DES_PWM_API_382
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetOmcrSwSyncTriggerMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_OMVSWSYNC_Msk, PWM_SYNCONF_OMVSWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set OMCR HW sync trigger mode
 * @note DES ID: DES_PWM_API_426
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetOmcrHwSyncTriggerMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_OMVHWSYNC_Msk, PWM_SYNCONF_OMVHWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set MCVR SW sync mode
 * @note DES ID: DES_PWM_API_383
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetMcvrSwSyncMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_PWMSVSWSYNC_Msk, PWM_SYNCONF_PWMSVSWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set MCVR HW sync mode
 * @note DES ID: DES_PWM_API_427
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetMcvrHwSyncMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_PWMSVHWSYNC_Msk, PWM_SYNCONF_PWMSVHWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set CNT SW sync trigger mode
 * @note DES ID: DES_PWM_API_384
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetCntSwSyncTriggerMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_CNTVSWSYNC_Msk, PWM_SYNCONF_CNTVSWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set CNT HW sync trigger mode
 * @note DES ID: DES_PWM_API_428
 * @param[in] Base PWM base address
 * @param[in] Mode Sync trigger method
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetCntHwSyncTriggerMode(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->SYNCONF, PWM_SYNCONF_CNTVHWSYNC_Msk, PWM_SYNCONF_CNTVHWSYNC_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable global time base
 * @note DES ID: DES_PWM_API_385
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableGlobalTimeBase(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->CONF, PWM_CONF_GTBEEN_Msk, PWM_CONF_GTBEEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable global time base output
 * @note DES ID: DES_PWM_API_386
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableGlobalTimeBaseOutput(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->CONF, PWM_CONF_GTBEOUT_Msk, PWM_CONF_GTBEOUT_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set input filter prescaler
 * @note DES ID: DES_PWM_API_387
 * @param[in] Base PWM base address
 * @param[in] Psc Input filter prescaler
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetInputFilterPsc(PWM_Type *Base, uint8 Psc)
{
    MODIFY_REG32(Base->CAPFILTER, PWM_CAPFILTER_CAPFPSC_Msk, PWM_CAPFILTER_CAPFPSC_Pos, (uint32)Psc);
}

/**
 * @brief Set channel input filter value
 * @note DES ID: DES_PWM_API_388
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Value Filter value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetChannelInputFilterVal(PWM_Type *Base, uint8 Channel, uint8 Value)
{
    DEVICE_ASSERT(4U > Channel);
    MODIFY_REG32(Base->CAPFILTER, (uint32)PWM_CAPFILTER_CH0CAPFVAL_Msk << (Channel * PWM_CAPFILTER_WIDTH), \
                 (Channel * PWM_CAPFILTER_WIDTH), Value);
}

/**
 * @brief Set capture event prescaler
 * @note DES ID: DES_PWM_API_389
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] EventPsc Event prescaler
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetCaptureEventPsc
(
    PWM_Type *Base,
    uint8 Channel,
    uint8 EventPsc
)
{
    MODIFY_REG32(Base->CONF, \
                 (PWM_CONF_EVENT0_PSC_Msk << (Channel * PWM_CONF_EVENTPSC_WIDTH)), \
                 (PWM_CONF_EVENT0_PSC_Pos + (Channel * PWM_CONF_EVENTPSC_WIDTH)), \
                 (uint32)EventPsc);
}

/**
 * @brief Enable/disable channel event reset
 * @note DES ID: DES_PWM_API_390
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] State TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableChannelEventReset
(
    PWM_Type *Base,
    uint8 Channel,
    boolean State
)
{
    MODIFY_REG32(Base->CHANNELS[Channel].CHnSCR, \
                 PWM_CH0SCR_CHRSTEN_Msk, \
                 PWM_CH0SCR_CHRSTEN_Pos, \
                 (FALSE != State) ? 1U : 0U);
}

/**
 * @brief Enable/disable pair channel dual edge capture
 * @note DES ID: DES_PWM_API_391
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @param[in] State TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnablePairChDualEdgeCapture
(
    PWM_Type *Base,
    uint8 ChannelPair,
    boolean State
)
{
    MODIFY_REG32(Base->MODESEL, \
                 ((uint32)PWM_MODESEL_PAIR0DECAPEN_Msk << (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (PWM_MODESEL_PAIR0DECAPEN_Pos + (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (FALSE != State) ? 1U : 0U);
}

/**
 * @brief Set pair channel dual edge capture
 * @note DES ID: DES_PWM_API_392
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @param[in] State TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetPairChDualEdgeCapture
(
    PWM_Type *Base,
    uint8 ChannelPair,
    boolean State
)
{
    MODIFY_REG32(Base->MODESEL, \
                 ((uint32)PWM_MODESEL_PAIR0DECAP_Msk << (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (PWM_MODESEL_PAIR0DECAP_Pos + (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)), \
                 (FALSE != State) ? 1U : 0U);
}

/**
 * @brief Check if pair channel dual edge capture is enabled
 * @note DES ID: DES_PWM_API_393
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @return TRUE if enabled, FALSE otherwise
 */
LOCAL_INLINE boolean Pwm_Reg_IsPairChDualEdgeCaptureEnabled
(
    const PWM_Type *Base,
    uint8 ChannelPair
)
{
    uint32 Flag = READ_BIT32(Base->MODESEL, \
                             ((uint32)PWM_MODESEL_PAIR0DECAPEN_Msk << (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH)));
    Flag = Flag >> (PWM_MODESEL_PAIR0DECAPEN_Pos + (ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH));

    return (0U != Flag) ? TRUE : FALSE;
}

/**
 * @brief Set fault mode
 * @note DES ID: DES_PWM_API_394
 * @param[in] Base PWM base address
 * @param[in] Mode Fault control mode
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetFaultMode(PWM_Type *Base, uint8 Mode)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_FAULTMODE_Msk, PWM_FUNCSEL_FAULTMODE_Pos, Mode);
}

/**
 * @brief Enable/disable fault input filter
 * @note DES ID: DES_PWM_API_395
 * @param[in] Base PWM base address
 * @param[in] FaultInputId Fault input ID
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableFaultInputFilter(PWM_Type *Base, uint8 FaultInputId, boolean Enable)
{
    MODIFY_REG32(Base->FFAFER, (uint32)PWM_FFAFER_FF0EN_Msk << FaultInputId,
                 PWM_FFAFER_FF0EN_Pos + FaultInputId, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set fault input filter value
 * @note DES ID: DES_PWM_API_396
 * @param[in] Base PWM base address
 * @param[in] Value Filter value
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetFaultInputFilterValue(PWM_Type *Base, uint8 Value)
{
    MODIFY_REG32(Base->FFAFER, PWM_FFAFER_FFVAL_Msk, PWM_FFAFER_FFVAL_Pos, Value);
}

/**
 * @brief Enable/disable fault pin input
 * @note DES ID: DES_PWM_API_397
 * @param[in] Base PWM base address
 * @param[in] FaultInputId Fault input ID
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableFaultPinInput(PWM_Type *Base, uint8 FaultInputId, boolean Enable)
{
    MODIFY_REG32(Base->FFAFER, (uint32)PWM_FFAFER_FER0EN_Msk << FaultInputId, FaultInputId,
                 (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Set fault input polarity
 * @note DES ID: DES_PWM_API_398
 * @param[in] Base PWM base address
 * @param[in] FaultInputId Fault input ID
 * @param[in] Polarity Active polarity
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetFaultInputPolarity(PWM_Type *Base, uint8 FaultInputId,
        uint8 Polarity)
{
    MODIFY_REG32(Base->FLTPOL, (uint32)PWM_FLTPOL_FLT0POL_Msk << FaultInputId, FaultInputId, Polarity);
}

/**
 * @brief Enable/disable fault interrupt
 * @note DES ID: DES_PWM_API_399
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableFaultInterrupt(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_FAULTIE_Msk, PWM_FUNCSEL_FAULTIE_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable fault high-Z output
 * @note DES ID: DES_PWM_API_400
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableFaultHizOutput(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_FAULTHIZEN_Msk, PWM_FUNCSEL_FAULTHIZEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable channel high-Z output
 * @note DES ID: DES_PWM_API_401
 * @param[in] Base PWM base address
 * @param[in] Channel PWM channel
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableChannelHizOutput(PWM_Type *Base, uint8 Channel, boolean Enable)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_CH0HIZEN_Msk << (Channel),
                 PWM_FUNCSEL_CH0HIZEN_Pos + (Channel), (FALSE != Enable) ? 1U : 0U);
}

#if defined (AC7843X) || defined (AC7842X) || defined (AC7840E)
/**
 * @brief Recover zero or full duty after fault cleared
 * @note DES ID: DES_PWM_API_402
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_RecoverZeroOrFullDutyAfterFaultCleared(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_FAULTCTRLEN_Msk, PWM_FUNCSEL_FAULTCTRLEN_Pos, (FALSE != Enable) ? 1U : 0U);
}
#endif

/**
 * @brief Enable/disable pair channel fault control
 * @note DES ID: DES_PWM_API_403
 * @param[in] Base PWM base address
 * @param[in] ChannelPair PWM channel pair
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnablePairChFaultCtrl(PWM_Type *Base, uint8 ChannelPair, boolean Enable)
{
    MODIFY_REG32(Base->MODESEL, (uint32)PWM_MODESEL_PAIR0FAULTEN_Msk << ((uint32)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH),
                 PWM_MODESEL_PAIR0FAULTEN_Pos + ((uint32)ChannelPair * PWM_COMBINE_PAIR_CONFIG_WIDTH), \
                 (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Get fault pin flag
 * @note DES ID: DES_PWM_API_404
 * @param[in] Base PWM base address
 * @param[in] FaultInputId Fault input ID
 * @return Fault pin flag
 */
LOCAL_INLINE uint32 Pwm_Reg_GetFaultPinFlag(const PWM_Type *Base, uint8 FaultInputId)
{
    uint32 Flag = READ_BIT32((Base)->FDSR, ((uint32)PWM_FDSR_FAULTDF0_Msk << (uint32)FaultInputId));
    Flag = Flag >> (PWM_FDSR_FAULTDF0_Pos + (uint32)FaultInputId);

    return Flag;
}

/**
 * @brief Clear fault pin flag
 * @note DES ID: DES_PWM_API_405
 * @param[in] Base PWM base address
 * @param[in] FaultInputId Fault input ID
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_ClearFaultPinFlag(PWM_Type *Base, uint8 FaultInputId)
{
    DEVICE_ASSERT(4U > FaultInputId);

#if defined (AC7843X)
    MODIFY_REG32(Base->FDSR, (PWM_FDSR_FAULTDF_Msk | PWM_FDSR_WPEN_Msk | PWM_FDSR_FAULTDF3_Msk | \
                              PWM_FDSR_FAULTDF2_Msk | PWM_FDSR_FAULTDF1_Msk | PWM_FDSR_FAULTDF0_Msk), \
                 (uint32)FaultInputId, 1UL);
#else
    CLEAR_BIT32((Base)->FDSR, ((uint32)PWM_FDSR_FAULTDF0_Msk << FaultInputId));
#endif
}

/**
 * @brief Get fault flag
 * @note DES ID: DES_PWM_API_406
 * @param[in] Base PWM base address
 * @return Fault flag
 */
LOCAL_INLINE uint32 Pwm_Reg_GetFaultFlag(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32((Base)->FDSR, PWM_FDSR_FAULTDF_Msk);
    Flag = Flag >> (PWM_FDSR_FAULTDF_Pos);

    return Flag;
}

/**
 * @brief Clear fault flag
 * @note DES ID: DES_PWM_API_407
 * @param[in] Base PWM base address
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_ClearFaultFlag(PWM_Type *Base)
{
#if defined (AC7843X)
    MODIFY_REG32(Base->FDSR, (PWM_FDSR_FAULTDF_Msk | PWM_FDSR_WPEN_Msk | PWM_FDSR_FAULTDF3_Msk | \
                              PWM_FDSR_FAULTDF2_Msk | PWM_FDSR_FAULTDF1_Msk | PWM_FDSR_FAULTDF0_Msk), \
                 PWM_FDSR_FAULTDF_Pos, 1UL);
#else
    CLEAR_BIT32((Base)->FDSR, PWM_FDSR_FAULTDF_Msk);
#endif
}

/**
 * @brief Get fault input status
 * @note DES ID: DES_PWM_API_408
 * @param[in] Base PWM base address
 * @return Fault input status
 */
LOCAL_INLINE uint32 Pwm_Reg_GetFaultInputStatus(const PWM_Type *Base)
{
    uint32 Flag = READ_BIT32((Base)->FDSR, PWM_FDSR_FAULTIN_Msk);
    Flag = Flag >> (PWM_FDSR_FAULTIN_Pos);

    return Flag;
}

/**
 * @brief Set quadrature encode mode
 * @note DES ID: DES_PWM_API_409
 * @param[in] Base PWM base address
 * @param[in] Mode Quadrature mode
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetQuadEncodeMode(PWM_Type *Base, uint8 Mode)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_QUADMODE_Msk, PWM_QDI_QUADMODE_Pos, Mode);
}

/**
 * @brief Set quadrature phase A polarity
 * @note DES ID: DES_PWM_API_410
 * @param[in] Base PWM base address
 * @param[in] Polarity Phase polarity
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetQuadPhaseAPolarity(PWM_Type *Base, uint8 Polarity)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_PHAPOL_Msk, PWM_QDI_PHAPOL_Pos, Polarity);
}

/**
 * @brief Set quadrature phase B polarity
 * @note DES ID: DES_PWM_API_411
 * @param[in] Base PWM base address
 * @param[in] Polarity Phase polarity
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetQuadPhaseBPolarity(PWM_Type *Base, uint8 Polarity)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_PHBPOL_Msk, PWM_QDI_PHBPOL_Pos, Polarity);
}

/**
 * @brief Set quadrature phase Z polarity
 * @note DES ID: DES_PWM_API_412
 * @param[in] Base PWM base address
 * @param[in] Polarity Phase polarity
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetQuadPhaseZPolarity(PWM_Type *Base, uint8 Polarity)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_PHZPOL_Msk, PWM_QDI_PHZPOL_Pos, Polarity);
}

/**
 * @brief Enable/disable quadrature phase Z reset counter
 * @note DES ID: DES_PWM_API_413
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableQuadPhaseZResetCnt(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_PHZRSTEN_Msk, PWM_QDI_PHZRSTEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Enable/disable quadrature decoder
 * @note DES ID: DES_PWM_API_414
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableQuadDecoder(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_QDIEN_Msk, PWM_QDI_QDIEN_Pos, (FALSE != Enable) ? 1U : 0U);
}

/**
 * @brief Get quadrature count direction
 * @note DES ID: DES_PWM_API_415
 * @param[in] Base PWM base address
 * @return Count direction
 */
LOCAL_INLINE uint32 Pwm_Reg_GetQuadCountDir(const PWM_Type *Base)
{
    uint32 DirFlag = READ_BIT32((Base)->QDI, PWM_QDI_QUADIR_Msk);
    DirFlag = DirFlag >> (PWM_QDI_QUADIR_Pos);

    return DirFlag;
}

/**
 * @brief Get quadrature overflow direction
 * @note DES ID: DES_PWM_API_416
 * @param[in] Base PWM base address
 * @return Overflow direction
 */
LOCAL_INLINE uint32 Pwm_Reg_GetQuadOverflowDir(const PWM_Type *Base)
{
    uint32 DirFlag = READ_BIT32((Base)->QDI, PWM_QDI_CNTOFDIR_Msk);
    DirFlag = DirFlag >> (PWM_QDI_CNTOFDIR_Pos);

    return DirFlag;
}

/**
 * @brief Get quadrature phase Z flag
 * @note DES ID: DES_PWM_API_417
 * @param[in] Base PWM base address
 * @return Phase Z flag
 */
LOCAL_INLINE uint32 Pwm_Reg_GetQuadPhaseZFlag(const PWM_Type *Base)
{
    uint32 Status = READ_BIT32((Base)->QDI, PWM_QDI_PHZSTS_Msk);
    Status = Status >> (PWM_QDI_PHZSTS_Pos);

    return Status;
}

/**
 * @brief Clear quadrature phase Z flag
 * @note DES ID: DES_PWM_API_418
 * @param[in] Base PWM base address
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_ClearQuadPhaseZFlag(PWM_Type *Base)
{
#if defined (AC7843X)
    SET_BIT32((Base)->QDI, PWM_QDI_PHZSTS_Msk);
#else
    CLEAR_BIT32((Base)->QDI, PWM_QDI_PHZSTS_Msk);
#endif
}

#if defined (AC7843X)
/**
 * @brief Set quadrature phase Z reset mode
 * @note DES ID: DES_PWM_API_419
 * @param[in] Base PWM base address
 * @param[in] Mode Reset mode
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetQuadPhaseZResetMode(PWM_Type *Base, uint8 Mode)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_PHZRSTMODE_Msk, PWM_QDI_PHZRSTMODE_Pos, Mode);
}

/**
 * @brief Enable/disable quadrature phase Z interrupt
 * @note DES ID: DES_PWM_API_420
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableQuadPhaseZInterrupt(PWM_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->QDI, PWM_QDI_DETECTIRQEN_Msk, PWM_QDI_DETECTIRQEN_Pos, (FALSE != Enable) ? 1U : 0U);
}
#endif

/**
 * @brief Enable/disable write protection
 * @note DES ID: DES_PWM_API_421
 * @param[in] Base PWM base address
 * @param[in] Enable TRUE to enable, FALSE to disable
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_EnableWriteProtection(PWM_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
#if defined (AC7843X)
        MODIFY_REG32(Base->FDSR, (PWM_FDSR_FAULTDF_Msk | PWM_FDSR_WPEN_Msk | PWM_FDSR_FAULTDF3_Msk | \
                                  PWM_FDSR_FAULTDF2_Msk | PWM_FDSR_FAULTDF1_Msk | PWM_FDSR_FAULTDF0_Msk), \
                     PWM_FDSR_WPEN_Pos, 1UL);
#else
        MODIFY_REG32(Base->FDSR, PWM_FDSR_WPEN_Msk, PWM_FDSR_WPEN_Pos, 1U);
#endif
    }
    else
    {
        MODIFY_REG32(Base->FUNCSEL, PWM_FUNCSEL_WPDIS_Msk, PWM_FUNCSEL_WPDIS_Pos, 1U);
    }
}

/**
 * @brief Set debug mode
 * @note DES ID: DES_PWM_API_422
 * @param[in] Base PWM base address
 * @param[in] Mode Debug mode
 * @return void
 */
LOCAL_INLINE void Pwm_Reg_SetDebugMode(PWM_Type *Base, uint8 Mode)
{
    MODIFY_REG32(Base->DITHER2, PWM_DITHER2_DEBUGCTRL_Msk, PWM_DITHER2_DEBUGCTRL_Pos, Mode);
}

#ifdef __cplusplus
}
#endif
#endif /* AC784XX_PWM_REG_H */
