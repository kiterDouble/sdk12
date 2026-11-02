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
 * @file AC784xx_Ckgen_Reg.h
 *
 * @brief This file provides read/write CKGEN hardware registers.
 *
 */

#ifndef AC784XX_CKGEN_REG_H
#define AC784XX_CKGEN_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
#define CKGEN_REG_BASE           (CKGEN_BASE)
/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
#if defined (AC7840X) || defined (AC7843X)
/**
 * @brief Set bus clock division in vlpr mode
 * @note Function ID: DES_CKGEN_API_300
 * @param[in] Div: clock division to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetVlprModeBusClkDiv(uint8 Div)
{
    MODIFY_REG32(CKGEN->CTRL, CKGEN_CTRL_BUS_CLK_DIV_VLPR_Msk, CKGEN_CTRL_BUS_CLK_DIV_VLPR_Pos, Div);
}

/**
 * @brief Get bus clock division in vlpr mode
 * @note Function ID: DES_CKGEN_API_301
 * @param[in] Div: clock division to set
 * @return void
 */
LOCAL_INLINE uint8 Ckgen_Reg_GetVlprModeBusClkDiv(void)
{
    return (uint8)EXTRACT_BIT32(CKGEN->CTRL, CKGEN_CTRL_BUS_CLK_DIV_VLPR_Msk, CKGEN_CTRL_BUS_CLK_DIV_VLPR_Pos);
}
#endif /* AC7840X AC7843X */
/**
 * @brief Set bus clock division in run mode
 * @note Function ID: DES_CKGEN_API_302
 * @param[in] Div: clock division to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetRunModeBusClkDiv(uint8 Div)
{
    MODIFY_REG32(CKGEN->CTRL, CKGEN_CTRL_BUS_CLK_DIV_RUN_Msk, CKGEN_CTRL_BUS_CLK_DIV_RUN_Pos, Div);
}

/**
 * @brief Get bus clock division in run mode
 * @note Function ID: DES_CKGEN_API_303
 * @return uint8: clock division value
 */
LOCAL_INLINE uint8 Ckgen_Reg_GetRunModeBusClkDiv(void)
{
    return ((uint8)((CKGEN->CTRL & CKGEN_CTRL_BUS_CLK_DIV_RUN_Msk) >> CKGEN_CTRL_BUS_CLK_DIV_RUN_Pos));
}

#if defined (AC7840X) || defined (AC7843X)
/**
 * @brief Set system clock division in vlpr mode
 * @note Function ID: DES_CKGEN_API_304
 * @param[in] Div: clock division to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetVlprModeSysClkDiv(uint8 Div)
{
    MODIFY_REG32(CKGEN->CTRL, CKGEN_CTRL_SYSCLK_DIV_VLPR_Msk, CKGEN_CTRL_SYSCLK_DIV_VLPR_Pos, Div);
}

/**
 * @brief Get system clock division in vlpr mode
 * @note Function ID: DES_CKGEN_API_305
 * @return uint8: clock division value
 */
LOCAL_INLINE uint8 Ckgen_Reg_GetVlprModeSysClkDiv(void)
{
    return ((uint8)((CKGEN->CTRL & CKGEN_CTRL_SYSCLK_DIV_VLPR_Msk) >> CKGEN_CTRL_SYSCLK_DIV_VLPR_Pos));
}
#endif /* AC7840X AC7843X */
/**
 * @brief Set system clock division in run mode
 * @note Function ID: DES_CKGEN_API_306
 * @param[in] Div: clock division to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetRunModeSysClkDiv(uint8 Div)
{
    MODIFY_REG32(CKGEN->CTRL, CKGEN_CTRL_SYSCLK_DIV_Msk, CKGEN_CTRL_SYSCLK_DIV_Pos, Div);
}

/**
 * @brief Get system clock division in run mode
 * @note Function ID: DES_CKGEN_API_307
 * @return uint8: clock division value
 */
LOCAL_INLINE uint8 Ckgen_Reg_GetRunModeSysClkDiv(void)
{
    return ((uint8)((CKGEN->CTRL & CKGEN_CTRL_SYSCLK_DIV_Msk) >> CKGEN_CTRL_SYSCLK_DIV_Pos));
}

/**
 * @brief Set system clock source in run mode
 * @note Function ID: DES_CKGEN_API_308
 * @param[in] SysClkSrc: clock source to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetRunModeSysClkSrc(uint8 SysClkSrc)
{
    MODIFY_REG32(CKGEN->CTRL, CKGEN_CTRL_SYSCLK_SRC_SEL_RUN_Msk, CKGEN_CTRL_SYSCLK_SRC_SEL_RUN_Pos, SysClkSrc);
}

/**
 * @brief Get system clock source in run mode
 * @note Function ID: DES_CKGEN_API_309
 * @return uint8: system clock source value
 */
LOCAL_INLINE uint8 Ckgen_Reg_GetRunModeSysClkSrc(void)
{
    uint32 SysClkSrc = EXTRACT_BIT32(CKGEN->CTRL, CKGEN_CTRL_SYSCLK_SRC_SEL_RUN_Msk, CKGEN_CTRL_SYSCLK_SRC_SEL_RUN_Pos);

    return (uint8)SysClkSrc;
}

#if defined (AC7840X) || defined (AC7843X)
/**
 * @brief Set system clock source in vlpr mode
 * @note Function ID: DES_CKGEN_API_310
 * @param[in] SysClkSrc: clock source to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetVlprModeSysClkSrc(uint8 SysClkSrc)
{
    MODIFY_REG32(CKGEN->CTRL, CKGEN_CTRL_SYSCLK_SRC_SEL_VLPR_Msk, CKGEN_CTRL_SYSCLK_SRC_SEL_VLPR_Pos, SysClkSrc);
}
#endif /* AC7840X AC7843X */
/**
 * @brief Get pll reference clock
 * @note Function ID: DES_CKGEN_API_311
 * @return uint8: pll reference clock register value
 */
LOCAL_INLINE uint8 Ckgen_Reg_GetPllRefClk(void)
{
    return ((uint8)((CKGEN->CTRL & CKGEN_CTRL_PLL_REF_SEL_Msk) >> CKGEN_CTRL_PLL_REF_SEL_Pos));
}

/**
 * @brief Set pll reference clock
 * @note Function ID: DES_CKGEN_API_312
 * @param[in] PllRefClk: pll reference clock register value
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetPllRefClk(uint32 PllRefClk)
{
    MODIFY_REG32(CKGEN->CTRL, CKGEN_CTRL_PLL_REF_SEL_Msk, CKGEN_CTRL_PLL_REF_SEL_Pos, PllRefClk);
}

/**
 * @brief Enable XOSC monitor.
 * @note Function ID: DES_CKGEN_API_313
 * @param[in] IsEnable: TRUE is enable, FALSE is disable
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_EnableXOSCMonitor(boolean IsEnable)
{
    uint32 Value = (TRUE == IsEnable) ? 1U : 0U;
    MODIFY_REG32(CKGEN->CTRL, CKGEN_CTRL_XOSC_MON_EN_Msk, CKGEN_CTRL_XOSC_MON_EN_Pos, Value);
}

/**
 * @brief Enable CTRL reg lock.
 * @note Function ID: DES_CKGEN_API_314
 * @param[in] IsEnable: TRUE is enable, FALSE is disable
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_EnableCTRLRegLock(boolean IsEnable)
{
    uint32 Value = (TRUE == IsEnable) ? 1U : 0U;
    MODIFY_REG32(CKGEN->CTRL, CKGEN_CTRL_LOCK_Msk, CKGEN_CTRL_LOCK_Pos, Value);
}

/**
 * @brief Get pll clock division value
 * @note Function ID: DES_CKGEN_API_315
 * @param[in] Mask: register value mask
 * @param[in] Pos: register position
 * @return uint32: pll clock division value
 */
LOCAL_INLINE uint32 Ana_Reg_GetPllDivVal(uint32 Mask, uint32 Pos)
{
    return ((ANA->SPLL_CFG0 & Mask) >> Pos);
}

/**
 * @brief Set pll clock division value
 * @note Function ID: DES_CKGEN_API_316
 * @param[in] Value: division value to set
 * @param[in] Mask: register value mask
 * @param[in] Pos: register position
 * @return void
 */
LOCAL_INLINE void Ana_Reg_SetPllDivVal(uint32 Value, uint32 Mask, uint32 Pos)
{
    MODIFY_REG32(ANA->SPLL_CFG0, Mask, Pos, Value);
}

/**
 * @brief Set pll clock lock parameter
 * @note Function ID: DES_CKGEN_API_317
 * @param[in] Value: value to set
 * @param[in] Mask: register value mask
 * @param[in] Pos: register position
 * @return void
 */
LOCAL_INLINE void Ana_Reg_SetPllLockParam(uint32 Value, uint32 Mask, uint32 Pos)
{
#if defined (AC7840X) || defined (AC7843X)
    MODIFY_REG32(ANA->SPLL_CFG1, Mask, Pos, Value);
#elif defined (AC7842X)
    MODIFY_REG32(ANA->SPLL_CFG0, Mask, Pos, Value);
#else
    (void)Value;
    (void)Mask;
    (void)Pos;
#endif
}

/**
 * @brief Set clock source div1 division
 * @note Function ID: DES_CKGEN_API_318
 * @param[in] Value: division value to set
 * @param[in] Mask: register value mask
 * @param[in] Pos: register position
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetClkSrcDiv1(uint32 Value, uint32 Mask, uint32 Pos)
{
    MODIFY_REG32(CKGEN->CLK_DIV1, Mask, Pos, Value);
}

/**
 * @brief Set clock source div2 division
 * @note Function ID: DES_CKGEN_API_319
 * @param[in] Value: division value to set
 * @param[in] Mask: register value mask
 * @param[in] Pos: register position
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetClkSrcDiv2(uint32 Value, uint32 Mask, uint32 Pos)
{
    MODIFY_REG32(CKGEN->CLK_DIV2, Mask, Pos, Value);
}

/**
 * @brief Get clock source div1 division
 * @note Function ID: DES_CKGEN_API_320
 * @param[in] Mask: register value mask
 * @param[in] Pos: register position
 * @return uint8: clock source div1 division
 */
LOCAL_INLINE uint8 Ckgen_Reg_GetClkSrcDiv1(uint32 Mask, uint32 Pos)
{
    uint32 Div1 = EXTRACT_BIT32(CKGEN->CLK_DIV1, Mask, Pos);

    return (uint8)Div1;
}

/**
 * @brief Get clock source div2 division
 * @note Function ID: DES_CKGEN_API_321
 * @param[in] Mask: register value mask
 * @param[in] Pos: register position
 * @return uint8: clock source div2 division
 */
LOCAL_INLINE uint8 Ckgen_Reg_GetClkSrcDiv2(uint32 Mask, uint32 Pos)
{
    uint32 Div2 = EXTRACT_BIT32(CKGEN->CLK_DIV2, Mask, Pos);

    return (uint8)Div2;
}

/**
 * @brief Set clock mux
 * @note Function ID: DES_CKGEN_API_322
 * @param[in] Offset: clock idx offset
 * @param[in] Value:  value to set
 * @param[in] Mask: register value mask
 * @param[in] Pos: register position
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetClkMux(uint32 Offset, uint32 Value, uint32 Mask, uint32 Pos)
{
    uint32 Base = CKGEN_REG_BASE + Offset;

    MODIFY_MEM32(Base, Mask, Pos, Value);/*cstat !MISRAC2012-Rule-11.4*/
}

/**
 * @brief Get clock mux
 * @note Function ID: DES_CKGEN_API_323
 * @param[in] Offset: clock idx offset
 * @param[in] Mask: register value mask
 * @param[in] Pos: register position
 * @return uint8: clock mux
 */
LOCAL_INLINE uint8 Ckgen_Reg_GetClkMux(uint32 Offset, uint32 Mask, uint32 Pos)
{
    uint32 Base = CKGEN_REG_BASE + Offset;

    return (uint8)((READ_MEM32(Base) & Mask) >> Pos);/*cstat !MISRAC2012-Rule-11.4*/
}

/**
 * @brief Set clock division
 * @note Function ID: DES_CKGEN_API_324
 * @param[in] Offset: clock idx offset
 * @param[in] Value:  value to set
 * @param[in] Mask: register value mask
 * @param[in] Pos: register position
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetClkDiv(uint32 Offset, uint32 Value, uint32 Mask, uint32 Pos)
{
    uint32 Base = CKGEN_REG_BASE + Offset;

    MODIFY_MEM32(Base, Mask, Pos, Value);/*cstat !MISRAC2012-Rule-11.4*/
}

/**
 * @brief Get clock division
 * @note Function ID: DES_CKGEN_API_325
 * @param[in] Offset: clock idx offset
 * @param[in] Mask: register value mask
 * @param[in] Pos: register position
 * @return uint8: clock division
 */
LOCAL_INLINE uint8 Ckgen_Reg_GetClkDiv(uint32 Offset, uint32 Mask, uint32 Pos)
{
    uint32 Base = CKGEN_REG_BASE + Offset;

    return (uint8)((READ_MEM32(Base) & Mask) >> Pos);/*cstat !MISRAC2012-Rule-11.4*/
}

/**
 * @brief Enable/Disbale  Periph clock
 * @note Function ID: DES_CKGEN_API_326
 * @param[in] Periph: peripheral id
 * @param[in] Enable: enable or disable
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetPeriphEnable(uint32 Periph, boolean Enable)
{
    uint32 Base = CKGEN_REG_BASE + 0x08U + (Periph / 32U * 0x04U);
    uint32 Pos = Periph % 32U;
    uint32 Msk = 1UL << Pos;
    uint32 Value = (TRUE == Enable) ? 1U : 0U;

    MODIFY_MEM32(Base, Msk, Pos, Value);
}

/**
 * @brief Get Periph clock enable status
 * @note Function ID: DES_CKGEN_API_327
 * @param[in] Periph: peripheral id
 * @return uint8: clock enable status
 */
LOCAL_INLINE boolean Ckgen_Reg_GetPeriphEnable(uint32 Periph)
{
    uint32 Base = CKGEN_REG_BASE + 0x08U + (Periph / 32U * 0x04U);
    uint32 Pos = Periph % 32U;
    uint32 Msk = 1UL << Pos;

    return ((1U == ((READ_MEM32(Base) & Msk) >> Pos)) ? TRUE : FALSE);
}

/**
 * @brief Enable/Disbale clkout output
 * @note Function ID: DES_CKGEN_API_328
 * @param[in] Enable: enable or disable
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_EnableClkout(uint8 Enable)
{
    MODIFY_REG32(CKGEN->CLK_OUT_CFG, CKGEN_CLK_OUT_CFG_ENABLE_Msk, CKGEN_CLK_OUT_CFG_ENABLE_Pos, Enable);
}

#if defined (AC7843X)
/**
 * @brief Set adc spll divider
 * @note Function ID: DES_CKGEN_API_329
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetAdcSpllDiv(uint32 Div)
{
    MODIFY_REG32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_ADC_SPLLDIV_Msk, CKGEN_CLK_DIV2_ADC_SPLLDIV_Pos, Div);
}

/**
 * @brief Get adc spll divider
 * @note Function ID: DES_CKGEN_API_330
 * @return uint32: adc spll divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetAdcSpllDiv(void)
{
    return EXTRACT_BIT32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_ADC_SPLLDIV_Msk, CKGEN_CLK_DIV2_ADC_SPLLDIV_Pos);
}
#endif /* AC7843X */

/**
 * @brief Set low power clock mux
 * @note Function ID: DES_CKGEN_API_331
 * @param[in] Val: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetLPMux(uint32 Val)
{
#if defined (AC7843X) || defined (AC7842X) || defined (AC7840E)
    MODIFY_REG32(CKGEN->LP_CLK_MUX, CKGEN_LP_CLK_MUX_LSI_CLK_MUX_Msk, CKGEN_LP_CLK_MUX_LSI_CLK_MUX_Pos, Val);
#elif defined (AC7840X) /* AC7843X AC7842X AC7840E */
    MODIFY_REG32(CKGEN->LP_CLK_MUX, CKGEN_LP_CLK_MUX_RTC_AND_LSI_CLK_MUX_Msk, \
                 CKGEN_LP_CLK_MUX_RTC_AND_LSI_CLK_MUX_Pos, Val);
#endif /* AC7840X */
}

/**
 * @brief Get lsi clock mux
 * @note Function ID: DES_CKGEN_API_332
 * @return uint32: lsi clock mux
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetLSIMux(void)
{
    return EXTRACT_BIT32(CKGEN->LP_CLK_MUX, CKGEN_LP_CLK_MUX_LSI_CLK_MUX_Msk, CKGEN_LP_CLK_MUX_LSI_CLK_MUX_Pos);
}

#if defined (AC7840X)
/**
 * @brief Get Rtc clock mux
 * @note Function ID: DES_CKGEN_API_333
 * @return uint32: Rtc clock mux
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetRtcMux(void)
{
    return EXTRACT_BIT32(CKGEN->LP_CLK_MUX, CKGEN_LP_CLK_MUX_RTC_CLK_MUX_Msk, CKGEN_LP_CLK_MUX_RTC_CLK_MUX_Pos);
}
#endif /* AC7840X */

#if defined (AC7843X) || defined (AC7842X) || defined (AC7840X)
/**
 * @brief Get PCT clock divider
 * @note Function ID: DES_CKGEN_API_334
 * @return uint32: PCT clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetPCTClkDiv(void)
{
    return EXTRACT_BIT32(CKGEN->PERI_CLK_DIV, CKGEN_PERI_CLK_DIV_PCT_DIV_Msk, CKGEN_PERI_CLK_DIV_PCT_DIV_Pos);
}

/**
 * @brief Set PCT clock divider.
 * @note Function ID: DES_CKGEN_API_335
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetPCTClkDiv(uint32 Div)
{
    MODIFY_REG32(CKGEN->PERI_CLK_DIV, CKGEN_PERI_CLK_DIV_PCT_DIV_Msk, CKGEN_PERI_CLK_DIV_PCT_DIV_Pos, Div);
}

#elif defined (AC7840E) /* endif of AC7842X AC7843X AC7840X */

/*!
 * @brief Set EIO function clock divider.
 *
 * @param[in] divider: EIO clock divider
 * @return none
 */
LOCAL_INLINE void Ckgen_Reg_SetEIOClkDiv(uint32 Div)
{
    MODIFY_REG32(CKGEN->PERI_CLK_DIV1, CKGEN_PERI_CLK_DIV1_EIO_DIV_Msk, CKGEN_PERI_CLK_DIV1_EIO_DIV_Pos, Div);
}

/*!
 * @brief Get EIO function clock divider.
 *
 * @param[in] none
 * @return EIO clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetEIOClkDiv(void)
{
    return EXTRACT_BIT32(CKGEN->PERI_CLK_DIV1, CKGEN_PERI_CLK_DIV1_EIO_DIV_Msk, CKGEN_PERI_CLK_DIV1_EIO_DIV_Pos);
}
#endif

/**
 * @brief Get TPIU clock divider
 * @note Function ID: DES_CKGEN_API_336
 * @return uint32: TPIU clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetTPIUClkDiv(void)
{
    return EXTRACT_BIT32(CKGEN->PERI_CLK_DIV, CKGEN_PERI_CLK_DIV_TPIU_DIV_Msk, CKGEN_PERI_CLK_DIV_TPIU_DIV_Pos);
}

/**
 * @brief Set TPIU clock divider.
 * @note Function ID: DES_CKGEN_API_337
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetTPIUClkDiv(uint32 Div)
{
    MODIFY_REG32(CKGEN->PERI_CLK_DIV, CKGEN_PERI_CLK_DIV_TPIU_DIV_Msk, CKGEN_PERI_CLK_DIV_TPIU_DIV_Pos, Div);
}

/**
 * @brief Get clockout clock divider
 * @note Function ID: DES_CKGEN_API_338
 * @return uint32: clockout clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetClkoutClkDiv(void)
{
    return EXTRACT_BIT32(CKGEN->CLK_OUT_CFG, CKGEN_CLK_OUT_CFG_DIV_Msk, CKGEN_CLK_OUT_CFG_DIV_Pos);
}

/**
 * @brief Set clockout clock divider.
 * @note Function ID: DES_CKGEN_API_339
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetClkoutClkDiv(uint32 Div)
{
    MODIFY_REG32(CKGEN->CLK_OUT_CFG, CKGEN_CLK_OUT_CFG_DIV_Msk, CKGEN_CLK_OUT_CFG_DIV_Pos, Div);
}

/**
 * @brief Get SPLL div1 clock divider
 * @note Function ID: DES_CKGEN_API_340
 * @return uint32: SPLL div1 clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetSPLLDIV1ClkDiv(void)
{
    return EXTRACT_BIT32(CKGEN->CLK_DIV1, CKGEN_CLK_DIV1_SPLL_DIV1_Msk, CKGEN_CLK_DIV1_SPLL_DIV1_Pos);
}

/**
 * @brief Set SPLL div1 clock divider.
 * @note Function ID: DES_CKGEN_API_341
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetSPLLDIV1ClkDiv(uint32 Div)
{
    MODIFY_REG32(CKGEN->CLK_DIV1, CKGEN_CLK_DIV1_SPLL_DIV1_Msk, CKGEN_CLK_DIV1_SPLL_DIV1_Pos, Div);
}

/**
 * @brief Get SPLL div2 clock divider
 * @note Function ID: DES_CKGEN_API_342
 * @return uint32: SPLL div2 clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetSPLLDIV2ClkDiv(void)
{
    return EXTRACT_BIT32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_SPLL_DIV2_Msk, CKGEN_CLK_DIV2_SPLL_DIV2_Pos);
}

/**
 * @brief Set SPLL div2 clock divider.
 * @note Function ID: DES_CKGEN_API_343
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetSPLLDIV2ClkDiv(uint32 Div)
{
    MODIFY_REG32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_SPLL_DIV2_Msk, CKGEN_CLK_DIV2_SPLL_DIV2_Pos, Div);
}

/**
 * @brief Get HSE div1 clock divider
 * @note Function ID: DES_CKGEN_API_344
 * @return uint32: HSE div1 clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetHSEDIV1ClkDiv(void)
{
    return EXTRACT_BIT32(CKGEN->CLK_DIV1, CKGEN_CLK_DIV1_HSE_DIV1_Msk, CKGEN_CLK_DIV1_HSE_DIV1_Pos);
}

/**
 * @brief Set HSE div1 clock divider.
 * @note Function ID: DES_CKGEN_API_345
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetHSEDIV1ClkDiv(uint32 Div)
{
    MODIFY_REG32(CKGEN->CLK_DIV1, CKGEN_CLK_DIV1_HSE_DIV1_Msk, CKGEN_CLK_DIV1_HSE_DIV1_Pos, Div);
}

/**
 * @brief Get HSE div2 clock divider
 * @note Function ID: DES_CKGEN_API_346
 * @return uint32: HSE div2 clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetHSEDIV2ClkDiv(void)
{
    return EXTRACT_BIT32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_HSE_DIV2_Msk, CKGEN_CLK_DIV2_HSE_DIV2_Pos);
}

/**
 * @brief Set HSE div2 clock divider.
 * @note Function ID: DES_CKGEN_API_347
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetHSEDIV2ClkDiv(uint32 Div)
{
    MODIFY_REG32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_HSE_DIV2_Msk, CKGEN_CLK_DIV2_HSE_DIV2_Pos, Div);
}

/**
 * @brief Get VHSI div1 clock divider
 * @note Function ID: DES_CKGEN_API_348
 * @return uint32: VHSI div1 clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetVHSIDIV1ClkDiv(void)
{
    return EXTRACT_BIT32(CKGEN->CLK_DIV1, CKGEN_CLK_DIV1_VHSI_DIV1_Msk, CKGEN_CLK_DIV1_VHSI_DIV1_Pos);
}

/**
 * @brief Set VHSI div1 clock divider.
 * @note Function ID: DES_CKGEN_API_349
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetVHSIDIV1ClkDiv(uint32 Div)
{
    MODIFY_REG32(CKGEN->CLK_DIV1, CKGEN_CLK_DIV1_VHSI_DIV1_Msk, CKGEN_CLK_DIV1_VHSI_DIV1_Pos, Div);
}

/**
 * @brief Get VHSI div2 clock divider
 * @note Function ID: DES_CKGEN_API_350
 * @return uint32: VHSI div2 clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetVHSIDIV2ClkDiv(void)
{
    return EXTRACT_BIT32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_VHSI_DIV2_Msk, CKGEN_CLK_DIV2_VHSI_DIV2_Pos);
}

/**
 * @brief Set VHSI div2 clock divider.
 * @note Function ID: DES_CKGEN_API_351
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetVHSIDIV2ClkDiv(uint32 Div)
{
    MODIFY_REG32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_VHSI_DIV2_Msk, CKGEN_CLK_DIV2_VHSI_DIV2_Pos, Div);
}

/**
 * @brief Get HSI div1 clock divider
 * @note Function ID: DES_CKGEN_API_352
 * @return uint32: HSI div1 clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetHSIDIV1ClkDiv(void)
{
#if defined (AC7840X) || defined (AC7843X)
    return EXTRACT_BIT32(CKGEN->CLK_DIV1, CKGEN_CLK_DIV1_HSI_DIV1_Msk, CKGEN_CLK_DIV1_HSI_DIV1_Pos);
#elif defined (AC7842X) || defined (AC7840E)
    return EXTRACT_BIT32(CKGEN->CLK_DIV1, CKGEN_CLK_DIV1_VHSI_8M_DIV1_Msk, CKGEN_CLK_DIV1_VHSI_8M_DIV1_Pos);
#endif
}

/**
 * @brief Set HSI div1 clock divider.
 * @note Function ID: DES_CKGEN_API_353
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetHSIDIV1ClkDiv(uint32 Div)
{
#if defined (AC7840X) || defined (AC7843X)
    MODIFY_REG32(CKGEN->CLK_DIV1, CKGEN_CLK_DIV1_HSI_DIV1_Msk, CKGEN_CLK_DIV1_HSI_DIV1_Pos, Div);
#elif defined (AC7842X) || defined (AC7840E) /* AC7840X AC7843X */
    MODIFY_REG32(CKGEN->CLK_DIV1, CKGEN_CLK_DIV1_VHSI_8M_DIV1_Msk, CKGEN_CLK_DIV1_VHSI_8M_DIV1_Pos, Div);
#endif /* AC7842X */
}

/**
 * @brief Get HSI div2 clock divider
 * @note Function ID: DES_CKGEN_API_354
 * @return uint32: HSI div2 clock divider
 */
LOCAL_INLINE uint32 Ckgen_Reg_GetHSIDIV2ClkDiv(void)
{
#if defined (AC7840X) || defined (AC7843X)
    return EXTRACT_BIT32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_HSI_DIV2_Msk, CKGEN_CLK_DIV2_HSI_DIV2_Pos);
#elif defined (AC7842X) || defined (AC7840E) /* AC7840X AC7843X */
    return EXTRACT_BIT32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_VHSI_8M_DIV2_Msk, CKGEN_CLK_DIV2_VHSI_8M_DIV2_Pos);
#endif /* AC7842X */
}

/**
 * @brief Set HSI div2 clock divider.
 * @note Function ID: DES_CKGEN_API_355
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ckgen_Reg_SetHSIDIV2ClkDiv(uint32 Div)
{
#if defined (AC7840X) || defined (AC7843X)
    MODIFY_REG32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_HSI_DIV2_Msk, CKGEN_CLK_DIV2_HSI_DIV2_Pos, Div);
#elif defined (AC7842X) || defined (AC7840E) /* AC7840X AC7843X */
    MODIFY_REG32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_VHSI_8M_DIV2_Msk, CKGEN_CLK_DIV2_VHSI_8M_DIV2_Pos, Div);
#endif /* AC7842X */
}

#if defined (AC7840E)
LOCAL_INLINE void Ckgen_Reg_SetFlashLatency(uint32 Latency)
{
    MODIFY_REG32(CKGEN->LP_CLK_MUX, CKGEN_LP_CLK_MUX_LATENCY_Msk, CKGEN_LP_CLK_MUX_LATENCY_Pos, Latency);
}

LOCAL_INLINE void Ckgen_Reg_SetFlashInfoLatency(uint32 Latency)
{
    MODIFY_REG32(CKGEN->LP_CLK_MUX, CKGEN_LP_CLK_MUX_LATENCY1_Msk, CKGEN_LP_CLK_MUX_LATENCY1_Pos, Latency);
}

LOCAL_INLINE void Ckgen_Reg_SetFlashClkFreq(uint32 ClkFreq)
{
    MODIFY_REG32(CKGEN->LP_CLK_MUX, CKGEN_LP_CLK_MUX_CLKFREQ_Msk, CKGEN_LP_CLK_MUX_CLKFREQ_Pos, ClkFreq);
}
#endif /* AC7840E */

/**
 * @brief Get can clock mux
 * @note Function ID: DES_CKGEN_API_200
 * @param[in] Clk: clock id, value can be one of the list value
 *                - CKGEN_CAN0_CLK          [40E][40][42][43]
 *                - CKGEN_CAN1_CLK          [40E][40][42][43]
 *                - CKGEN_CAN2_CLK               [40][42][43]
 *                - CKGEN_CAN3_CLK               [40][42][43]
 *                - CKGEN_CAN4_CLK                   [42][43]
 *                - CKGEN_CAN5_CLK                   [42][43]
 * @return Ckgen_ClkIdType: mux clock id
 *                - CKGEN_OFF_CLK           [40E][40][42][43]
 *                - CKGEN_HSE_DIV2_CLK      [40E][40][42][43]
 *                - CKGEN_SYS_CLK           [40E][40][42][43]
 *                - CKGEN_VHSI_DIV2_CLK          [40][42][43]
 *                - CKGEN_SPLL_DIV2_CLK     [40E][40][42][43]
 *                - CKGEN_SPLL1_CLK         [40E]
 */
Ckgen_ClkIdType Ckgen_Reg_GetCanClkMux(Ckgen_ClkIdType Clk);

/**
 * @brief Get pwm clock mux
 * @note Function ID: DES_CKGEN_API_201
 * @param[in] Clk: clock id, value can be one of the list value
 *                - CKGEN_PWM0_CLK          [40E][40][42][43]
 *                - CKGEN_PWM1_CLK          [40E][40][42][43]
 *                - CKGEN_PWM2_CLK          [40E][40][42][43]
 *                - CKGEN_PWM3_CLK          [40E][40][42][43]
 *                - CKGEN_PWM4_CLK               [40][42][43]
 *                - CKGEN_PWM5_CLK               [40][42][43]
 *                - CKGEN_PWM6_CLK                       [43]
 *                - CKGEN_PWM7_CLK                       [43]
 * @return Ckgen_ClkIdType: mux clock id
 *                - CKGEN_HSE_DIV1_CLK      [40E][40][42][43]
 *                - CKGEN_HSI_DIV1_CLK      [40E][40][42][43]
 *                - CKGEN_VHSI_DIV1_CLK     [40E][40][42][43]
 *                - CKGEN_SPLL_DIV1_CLK     [40E][40][42][43]
 *                - CKGEN_PWM_EXT_CLK0      [40E][40][42][43]
 *                - CKGEN_PWM_EXT_CLK1      [40E][40][42][43]
 *                - CKGEN_PWM_EXT_CLK2      [40E][40][42][43]
 */
Ckgen_ClkIdType Ckgen_Reg_GetPwmClkMux(Ckgen_ClkIdType Clk);

/**
 * @brief get common peripheral clock mux
 * @note Function ID: DES_CKGEN_API_202
 * @param[in] Clk: clock id, value can be one of the list value
 *                - CKGEN_I2C0_CLK          [40E][40][42][43]
 *                - CKGEN_I2C1_CLK          [40E]    [42][43]
 *                - CKGEN_I2C2_CLK                       [43]
 *                - CKGEN_TIMER_CLK         [40E][40][42][43]
 *                - CKGEN_SPI0_CLK          [40E][40][42][43]
 *                - CKGEN_SPI1_CLK          [40E][40][42][43]
 *                - CKGEN_SPI2_CLK          [40E][40][42][43]
 *                - CKGEN_SPI3_CLK                   [42][43]
 *                - CKGEN_SPI4_CLK                       [43]
 *                - CKGEN_ADC0_CLK          [40E][40][42][43]
 *                - CKGEN_ADC1_CLK               [40][42][43]
 *                - CKGEN_PCT_CLK                [40]    [43]
 *                - CKGEN_EIO_CLK           [40E][40][42][43]
 *                - CKGEN_UART0_CLK         [40E][40][42][43]
 *                - CKGEN_UART1_CLK         [40E][40][42][43]
 *                - CKGEN_UART2_CLK         [40E][40][42][43]
 *                - CKGEN_UART3_CLK              [40][42][43]
 *                - CKGEN_UART4_CLK                      [43]
 *                - CKGEN_UART5_CLK                      [43]
 *                - CKGEN_UART6_CLK                      [43]
 *                - CKGEN_UART7_CLK                      [43]
 * @return Ckgen_ClkIdType: mux clock id
 *                - CKGEN_OFF_CLK           [40E][40][42][43]
 *                - CKGEN_HSE_DIV2_CLK      [40E][40][42][43]
 *                - CKGEN_HSI_DIV2_CLK      [40E][40][42][43]
 *                - CKGEN_VHSI_DIV2_CLK     [40E][40][42][43]
 *                - CKGEN_SPLL_DIV2_CLK     [40E][40][42][43]
 *                - CKGEN_ADC_SPLLDIV_CLK                [43]
 */
Ckgen_ClkIdType Ckgen_Reg_GetCommPeriphClkMux(Ckgen_ClkIdType Clk);

/**
 * @brief Set can clock mux
 * @note Function ID: DES_CKGEN_API_203
 * @param[in] Clk: clock id, value can be one of the list value
 *                - CKGEN_CAN0_CLK          [40E][40][42][43]
 *                - CKGEN_CAN1_CLK          [40E][40][42][43]
 *                - CKGEN_CAN2_CLK               [40][42][43]
 *                - CKGEN_CAN3_CLK               [40][42][43]
 *                - CKGEN_CAN4_CLK                   [42][43]
 *                - CKGEN_CAN5_CLK                   [42][43]
 * @param[in] ClkSrc: clock id, value can be one of the list value
 *                - CKGEN_OFF_CLK           [40E][40][42][43]
 *                - CKGEN_HSE_DIV2_CLK      [40E][40][42][43]
 *                - CKGEN_SYS_CLK           [40E][40][42][43]
 *                - CKGEN_VHSI_DIV2_CLK     [40E][40][42][43]
 *                - CKGEN_SPLL_DIV2_CLK     [40E][40][42][43]
 * @return void
 */
void Ckgen_Reg_SetCanClkMux(Ckgen_ClkIdType Clk, Ckgen_ClkIdType ClkSrc);

/**
 * @brief Set pwm clock mux
 * @note Function ID: DES_CKGEN_API_204
 * @param[in] Clk: clock id, value can be one of the list value
 *                - CKGEN_PWM0_CLK          [40E][40][42][43]
 *                - CKGEN_PWM1_CLK          [40E][40][42][43]
 *                - CKGEN_PWM2_CLK          [40E][40][42][43]
 *                - CKGEN_PWM3_CLK          [40E][40][42][43]
 *                - CKGEN_PWM4_CLK               [40][42][43]
 *                - CKGEN_PWM5_CLK               [40][42][43]
 *                - CKGEN_PWM6_CLK                       [43]
 *                - CKGEN_PWM7_CLK                       [43]
 * @param[in] ClkSrc: clock id, value can be one of the list value
 *                - CKGEN_HSE_DIV1_CLK      [40E][40][42][43]
 *                - CKGEN_HSI_DIV1_CLK      [40E][40][42][43]
 *                - CKGEN_VHSI_DIV1_CLK     [40E][40][42][43]
 *                - CKGEN_SPLL_DIV1_CLK     [40E][40][42][43]
 *                - CKGEN_PWM_EXT_CLK0      [40E][40][42][43]
 *                - CKGEN_PWM_EXT_CLK1      [40E][40][42][43]
 *                - CKGEN_PWM_EXT_CLK2      [40E][40][42][43]
 * @return void
 */
void Ckgen_Reg_SetPwmClkMux(Ckgen_ClkIdType Clk, Ckgen_ClkIdType ClkSrc);

/**
 * @brief Set common peripheral clkout clock mux
 * @note Function ID: DES_CKGEN_API_205
 * @param[in] Clk: clock id, value can be one of the list value
 *                - CKGEN_I2C0_CLK          [40E][40][42][43]
 *                - CKGEN_I2C1_CLK          [40E]    [42][43]
 *                - CKGEN_I2C2_CLK                       [43]
 *                - CKGEN_TIMER_CLK         [40E][40][42][43]
 *                - CKGEN_SPI0_CLK          [40E][40][42][43]
 *                - CKGEN_SPI1_CLK          [40E][40][42][43]
 *                - CKGEN_SPI2_CLK          [40E][40][42][43]
 *                - CKGEN_SPI3_CLK                   [42][43]
 *                - CKGEN_SPI4_CLK                       [43]
 *                - CKGEN_ADC0_CLK          [40E][40][42][43]
 *                - CKGEN_ADC1_CLK               [40][42][43]
 *                - CKGEN_PCT_CLK                [40]    [43]
 *                - CKGEN_EIO_CLK           [40E][40][42][43]
 *                - CKGEN_UART0_CLK         [40E][40][42][43]
 *                - CKGEN_UART1_CLK         [40E][40][42][43]
 *                - CKGEN_UART2_CLK         [40E][40][42][43]
 *                - CKGEN_UART3_CLK              [40][42][43]
 *                - CKGEN_UART4_CLK                      [43]
 *                - CKGEN_UART5_CLK                      [43]
 *                - CKGEN_UART6_CLK                      [43]
 *                - CKGEN_UART7_CLK                      [43]
 * @param[in] ClkSrc: clock id, value can be one of the list value
 *                - CKGEN_OFF_CLK           [40E][40][42][43]
 *                - CKGEN_HSE_DIV2_CLK      [40E][40][42][43]
 *                - CKGEN_HSI_DIV2_CLK      [40E][40][42][43]
 *                - CKGEN_VHSI_DIV2_CLK     [40E][40][42][43]
 *                - CKGEN_SPLL_DIV2_CLK     [40E][40][42][43]
 *                - CKGEN_ADC_SPLLDIV_CLK                [43]
 * @return void
 */
void Ckgen_Reg_SetCommPeriphClkMux(Ckgen_ClkIdType Clk, Ckgen_ClkIdType ClkSrc);

/**
 * @brief get clockout mux
 * @note Function ID: DES_CKGEN_API_206
 * @return Ckgen_ClkIdType: mux clock id
 *                - CKGEN_HSE_CLK           [40E][40][42][43]
 *                - CKGEN_HSI_CLK                [40]    [43]
 *                - CKGEN_VHSI_CLK          [40E][40][42][43]
 *                - CKGEN_SPLL_CLK          [40E][40][42][43]
 *                - CKGEN_FLASH_CLK         [40E][40][42][43]
 *                - CKGEN_RTC_CLK                [40]
 *                - CKGEN_LSI_CLK           [40E][40][42][43]
 *                - CKGEN_LSI_128K_CLK      [40E][40][42][43]
 *                - CKGEN_HSE_DIV2_CLK      [40E][40][42][43]
 *                - CKGEN_VHSI_DIV2_CLK     [40E][40][42][43]
 *                - CKGEN_HSI_DIV2_CLK           [40]    [43]
 *                - CKGEN_SPLL_DIV2_CLK     [40E][40][42][43]
 *                - CKGEN_BUS_CLK           [40E][40][42][43]
 *                - CKGEN_SYS_CLK           [40E][40][42][43]
 *                - CKGEN_ADC_SPLLDIV_CLK                [43]
 *                - CKGEN_HSE_DIV1_CLK      [40E]    [42][43]
 *                - CKGEN_VHSI_DIV1_CLK     [40E]    [42][43]
 *                - CKGEN_HSI_DIV1_CLK                   [43]
 *                - CKGEN_SPLL_DIV1_CLK     [40E]    [42][43]
 *                - CKGEN_SPLL1_CLK         [40E]
 */
Ckgen_ClkIdType Ckgen_Reg_GetClkoutClkMux(void);

/**
 * @brief Set clkout clock mux
 * @note Function ID: DES_CKGEN_API_207
 * @param[in] ClkSrc: clock source id, value can be one of the list value
 *                - CKGEN_HSE_CLK           [40E][40][42][43]
 *                - CKGEN_HSI_CLK                [40]    [43]
 *                - CKGEN_VHSI_CLK          [40E][40][42][43]
 *                - CKGEN_SPLL_CLK          [40E][40][42][43]
 *                - CKGEN_FLASH_CLK         [40E][40][42][43]
 *                - CKGEN_RTC_CLK                [40]
 *                - CKGEN_LSI_CLK           [40E][40][42][43]
 *                - CKGEN_LSI_128K_CLK      [40E][40][42][43]
 *                - CKGEN_HSE_DIV2_CLK      [40E][40][42][43]
 *                - CKGEN_VHSI_DIV2_CLK     [40E][40][42][43]
 *                - CKGEN_HSI_DIV2_CLK           [40]    [43]
 *                - CKGEN_SPLL_DIV2_CLK     [40E][40][42][43]
 *                - CKGEN_BUS_CLK           [40E][40][42][43]
 *                - CKGEN_SYS_CLK           [40E][40][42][43]
 *                - CKGEN_ADC_SPLLDIV_CLK                [43]
 *                - CKGEN_HSE_DIV1_CLK      [40E]    [42][43]
 *                - CKGEN_VHSI_DIV1_CLK     [40E]    [42][43]
 *                - CKGEN_HSI_DIV1_CLK                   [43]
 *                - CKGEN_SPLL_DIV1_CLK     [40E]    [42][43]
 *                - CKGEN_SPLL1_CLK         [40E]
 * @return void
 */
void Ckgen_Reg_SetClkoutClkMux(Ckgen_ClkIdType ClkSrc);

/**
 * @brief get spll mux
 * @note Function ID: DES_CKGEN_API_208
 * @return Ckgen_ClkIdType: mux clock id
 *                - CKGEN_HSI_CLK        [40]    [43]
 *                - CKGEN_VHSI_CLK  [40E]    [42]
 *                - CKGEN_HSE_CLK   [40E][40][42][43]
 */
Ckgen_ClkIdType Ckgen_Reg_GetSPLLClkMux(void);

/**
 * @brief Set spll clock mux
 * @note Function ID: DES_CKGEN_API_209
 * @param[in] ClkSrc: clock source id, value can be one of the list value
 *                - CKGEN_HSI_CLK        [40]    [43]
 *                - CKGEN_VHSI_CLK  [40E]    [42]
 *                - CKGEN_HSE_CLK   [40E][40][42][43]
 * @return void
 */
void Ckgen_Reg_SetSPLLClkMux(Ckgen_ClkIdType ClkSrc);

#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
/**
 * @brief Get can time stamp clock division
 * @note Function ID: DES_CKGEN_API_220
 * @param[in] Clk: clock id,value can be one of the list value
 *                - CKGEN_CAN0_TS_CLK [40E][40][42]
 *                - CKGEN_CAN1_TS_CLK [40E][40][42]
 *                - CKGEN_CAN2_TS_CLK      [40][42]
 *                - CKGEN_CAN3_TS_CLK      [40][42]
 *                - CKGEN_CAN4_TS_CLK          [42]
 *                - CKGEN_CAN5_TS_CLK          [42]
 * @return uint8: clock division
 */
uint8 Ckgen_Reg_GetCanTsClkDiv(Ckgen_ClkIdType Clk);
#endif /* AC7840X AC7842X */

/**
 * @brief Get can clock division
 * @note Function ID: DES_CKGEN_API_221
 * @param[in] Clk: clock id,value can be one of the list value
 *                - CKGEN_CAN0_CLK [40E][40][42][43]
 *                - CKGEN_CAN1_CLK [40E][40][42][43]
 *                - CKGEN_CAN2_CLK      [40][42][43]
 *                - CKGEN_CAN3_CLK      [40][42][43]
 *                - CKGEN_CAN4_CLK          [42][43]
 *                - CKGEN_CAN5_CLK          [42][43]
 * @return uint8: clock division
 */
uint8 Ckgen_Reg_GetCanClkDiv(Ckgen_ClkIdType Clk);

/**
 * @brief Set can clock division
 * @note Function ID: DES_CKGEN_API_223
 * @param[in] Clk: clock id,value can be one of the list value
 *                - CKGEN_CAN0_CLK [40E][40][42][43]
 *                - CKGEN_CAN1_CLK [40E][40][42][43]
 *                - CKGEN_CAN2_CLK      [40][42][43]
 *                - CKGEN_CAN3_CLK      [40][42][43]
 *                - CKGEN_CAN4_CLK          [42][43]
 *                - CKGEN_CAN5_CLK          [42][43]
 * @param[in] Div: clock division
 * @return void
 */
void Ckgen_Reg_SetCanClkDiv(Ckgen_ClkIdType Clk, uint8 Div);

#if defined (AC7842X)  || defined (AC7840X) || defined (AC7840E)
/**
 * @brief Set can time stamp clock division
 * @note Function ID: DES_CKGEN_API_222
 * @param[in] Clk: clock id,value can be one of the list value
 *                - CKGEN_CAN0_TS_CLK [40E][40][42]
 *                - CKGEN_CAN1_TS_CLK [40E][40][42]
 *                - CKGEN_CAN2_TS_CLK      [40][42]
 *                - CKGEN_CAN3_TS_CLK      [40][42]
 *                - CKGEN_CAN4_TS_CLK          [42]
 *                - CKGEN_CAN5_TS_CLK          [42]
 * @param[in] Div: clock division
 * @return void
 */
void Ckgen_Reg_SetCanTsClkDiv(Ckgen_ClkIdType Clk, uint8 Div);
#endif /* AC7840X AC7842X */

#if defined (AC7840E)
/**
 * @brief Get posdivider value
 * @note Function ID: DES_CKGEN_API_356
 * @return uint32: posdivider
 */
LOCAL_INLINE uint32 Ana_Reg_GetPllPosDiv1Val(void)
{
    return EXTRACT_BIT32(ANA->SPLL_CFG1, ANA_SPLL_CFG1_POSDIV1_Msk, ANA_SPLL_CFG1_POSDIV1_Pos);
}

/**
 * @brief Set pll posdivider value.
 * @note Function ID: DES_CKGEN_API_357
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ana_Reg_SetPllPosDiv1Val(uint32 Div)
{
    MODIFY_REG32(ANA->SPLL_CFG1, ANA_SPLL_CFG1_POSDIV1_Msk, ANA_SPLL_CFG1_POSDIV1_Pos, Div);
}
#endif

/**
 * @brief Get posdivider value
 * @note Function ID: DES_CKGEN_API_356
 * @return uint32: posdivider
 */
LOCAL_INLINE uint32 Ana_Reg_GetPllPosDivVal(void)
{
    return EXTRACT_BIT32(ANA->SPLL_CFG0, ANA_SPLL_CFG0_POSDIV_Msk, ANA_SPLL_CFG0_POSDIV_Pos);
}

/**
 * @brief Set pll posdivider value.
 * @note Function ID: DES_CKGEN_API_357
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ana_Reg_SetPllPosDivVal(uint32 Div)
{
    MODIFY_REG32(ANA->SPLL_CFG0, ANA_SPLL_CFG0_POSDIV_Msk, ANA_SPLL_CFG0_POSDIV_Pos, Div);
}

/**
 * @brief Get predivider value
 * @note Function ID: DES_CKGEN_API_358
 * @return uint32: predivider
 */
LOCAL_INLINE uint32 Ana_Reg_GetPllPreDivVal(void)
{
    return EXTRACT_BIT32(ANA->SPLL_CFG0, ANA_SPLL_CFG0_PREDIV_Msk, ANA_SPLL_CFG0_PREDIV_Pos);
}

/**
 * @brief Set pll predivider value.
 * @note Function ID: DES_CKGEN_API_359
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ana_Reg_SetPllPreDivVal(uint32 Div)
{
    MODIFY_REG32(ANA->SPLL_CFG0, ANA_SPLL_CFG0_PREDIV_Msk, ANA_SPLL_CFG0_PREDIV_Pos, Div);
}

/**
 * @brief Get fbkdivider value
 * @note Function ID: DES_CKGEN_API_360
 * @return uint32: fbkdivider
 */
LOCAL_INLINE uint32 Ana_Reg_GetPllFbkDivVal(void)
{
    return EXTRACT_BIT32(ANA->SPLL_CFG0, ANA_SPLL_CFG0_FBKDIV_Msk, ANA_SPLL_CFG0_FBKDIV_Pos);
}

/**
 * @brief Set pll fbkdivider value.
 * @note Function ID: DES_CKGEN_API_361
 * @param[in] Div: division value to set
 * @return void
 */
LOCAL_INLINE void Ana_Reg_SetPllFbkDivVal(uint32 Div)
{
    MODIFY_REG32(ANA->SPLL_CFG0, ANA_SPLL_CFG0_FBKDIV_Msk, ANA_SPLL_CFG0_FBKDIV_Pos, Div);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AC784XX_CKGEN_REG_H*/

/* =============================================  EOF  ============================================== */
