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
 * AutoChips Inc. (C) 2024. All rights reserved.
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

/**
 * @file AC784xx_Ckgen_Reg.c
 *
 * @brief ckgen hal source file.
 *
 * @details Comments augmented via MCP C/Doxygen guideline: use `@brief/@param/@return` for functions,
 *          add concise descriptions for macros/enums, and document constraints via `@note`/`@warning` when appropriate.
 */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "AC784xx_Ckgen_Reg.h"
/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* =====================================  Functions definition  ===================================== */

/************          div operation          ************/
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
void Ckgen_Reg_SetCanClkDiv(Ckgen_ClkIdType Clk, uint8 Div)
{
    uint32 Mask;
    uint32 Pos;

#if defined (AC7842X)  || defined (AC7840X)
    Pos = ((uint32)Clk - CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET) >> 1U;
    Mask = 0x1UL << Pos;
#elif defined (AC7840E) /* AC7840X AC7842X */
    Pos = ((uint32)Clk - CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET) >> 1U;
    Mask = 0x3UL << Pos;
#elif defined (AC7843X) /* AC7840E */
    Pos = ((uint32)Clk - CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET) >> 1U;
    Mask = 0x3UL << Pos;
#endif /* AC7843X */
    Ckgen_Reg_SetClkDiv(CKGEN_PERIPH_CLK_DIV_ADDR_OFFSET, Div, Mask, Pos);
}

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
void Ckgen_Reg_SetCanTsClkDiv(Ckgen_ClkIdType Clk, uint8 Div)
{
    uint32 Mask;
    uint32 Pos;

    Pos = (uint32)Clk - CKGEN_PERIPH_CLK_DIV_INDEX_OFFSET;
    Mask = 0x07UL << Pos;
    Ckgen_Reg_SetClkDiv(CKGEN_PERIPH_CLK_DIV_ADDR_OFFSET, Div, Mask, Pos);
}
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
uint8 Ckgen_Reg_GetCanClkDiv(Ckgen_ClkIdType Clk)
{
    uint32 Mask;
    uint32 Pos;
    uint8 Div;

#if defined (AC7842X) || defined (AC7840X) || defined (AC7840E)
    Pos = ((uint32)Clk - CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET) >> 1U;
#elif defined (AC7843X) /* AC7840X AC7842X AC7840E */
    Pos = ((uint32)Clk - CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET) >> 1U;
#endif /* AC7843X */

#if defined (AC7842X) || defined (AC7840X)
    Mask = 0x1UL << Pos;
#elif defined (AC7843X) || defined (AC7840E) /* AC7840X AC7842X  */
    Mask = 0x3UL << Pos;
#endif /* AC7843X */
    Div = Ckgen_Reg_GetClkDiv(CKGEN_PERIPH_CLK_DIV_ADDR_OFFSET, Mask, Pos);

    return Div;
}

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
uint8 Ckgen_Reg_GetCanTsClkDiv(Ckgen_ClkIdType Clk)
{
    uint32 Mask;
    uint32 Pos;
    uint8 Div;

    Pos = (uint32)Clk - CKGEN_PERIPH_CLK_DIV_INDEX_OFFSET;
    Mask = 0x07UL << Pos;
    Div = Ckgen_Reg_GetClkDiv(CKGEN_PERIPH_CLK_DIV_ADDR_OFFSET, Mask, Pos);

    return Div;
}
#endif /* AC7840X AC7842X */

/************          mux operation          ************/

/**
 * @brief Convert spll clock id to register value
 * @note Function ID: DES_CKGEN_API_219
 * @param[in] ClkSrc: clock source id, value can be one of the list value
 *                - CKGEN_HSI_CLK        [40]    [43]
 *                - CKGEN_VHSI_CLK  [40E]    [42]
 *                - CKGEN_HSE_CLK   [40E][40][42][43]
 * @return uint32: mux clock register value
 */
static uint32 Ckgen_Reg_SPLLClkToMuxVal(Ckgen_ClkIdType ClkSrc)
{
    uint32 MuxVal;

#if defined (AC7840X) || defined (AC7843X)
    if (CKGEN_HSI_CLK == ClkSrc)
    {
        MuxVal = CKGEN_PLL_IN_HSI;
    }
#elif defined (AC7842X) || defined (AC7840E) /* AC7840X AC7843X */
    if (CKGEN_VHSI_CLK == ClkSrc)
    {
        MuxVal = CKGEN_PLL_IN_VHSI;
    }
#endif /* AC7842X AC7840E */
    else
    {
        MuxVal = CKGEN_PLL_IN_HSE;
    }

    return MuxVal;
}

/**
 * @brief Convert spll clock register value to clock id
 * @note Function ID: DES_CKGEN_API_214
 * @param[in] MuxVal: register value
 * @return Ckgen_ClkIdType: mux clock id
 */
static Ckgen_ClkIdType Ckgen_Reg_MuxValToSPLLClk(uint32 MuxVal)
{
    Ckgen_ClkIdType ClkSrc;

#if defined (AC7840X) || defined (AC7843X)
    if (CKGEN_PLL_IN_HSI == MuxVal)
    {
        ClkSrc = CKGEN_HSI_CLK;
    }
#elif defined (AC7842X) || defined(AC7840E)  /* AC7840X AC7843X */
    if (CKGEN_PLL_IN_VHSI == MuxVal)
    {
        ClkSrc = CKGEN_VHSI_CLK;
    }
#endif /* AC7842X AC7840E */
    else
    {
        ClkSrc = CKGEN_HSE_CLK;
    }

    return ClkSrc;
}

/**
 * @brief Convert common clock id to register value
 * @note Function ID: DES_CKGEN_API_218
 * @param[in] ClkSrc: clock source id, value can be one of the list value
 *                - CKGEN_OFF_CLK         [40E][40][42][43]
 *                - CKGEN_HSE_DIV2_CLK    [40E][40][42][43]
 *                - CKGEN_HSI_DIV2_CLK    [40E][40][42][43]
 *                - CKGEN_VHSI_DIV2_CLK   [40E][40][42][43]
 *                - CKGEN_SPLL_DIV2_CLK   [40E][40][42][43]
 * @return uint32: mux clock register value
 */
static uint32 Ckgen_Reg_CommClkToMuxVal(Ckgen_ClkIdType ClkSrc)
{
    uint32 MuxVal;

    switch (ClkSrc)
    {
    case CKGEN_OFF_CLK:
        MuxVal = 0U;
        break;
    case CKGEN_HSE_DIV2_CLK:
        MuxVal = 4U;
        break;
    case CKGEN_HSI_DIV2_CLK:
        MuxVal = 5U;
        break;
    case CKGEN_VHSI_DIV2_CLK:
        MuxVal = 6U;
        break;
    case CKGEN_SPLL_DIV2_CLK:
        MuxVal = 7U;
        break;
    default:
        MuxVal = 0xFFU;
        break;
    }

    return MuxVal;
}

/**
 * @brief Convert common clock register value to clock id
 * @note Function ID: DES_CKGEN_API_213
 * @param[in] MuxVal: register value
 * @return Ckgen_ClkIdType: mux clock id
 */
static Ckgen_ClkIdType Ckgen_Reg_CommMuxValToClk(uint32 MuxVal)
{
    Ckgen_ClkIdType ClkSrc;

    switch (MuxVal)
    {
    case 0U:
        ClkSrc = CKGEN_OFF_CLK;
        break;
    case 4U:
        ClkSrc = CKGEN_HSE_DIV2_CLK;
        break;
    case 5U:
        ClkSrc = CKGEN_HSI_DIV2_CLK;
        break;
    case 6U:
        ClkSrc = CKGEN_VHSI_DIV2_CLK;
        break;
    case 7U:
        ClkSrc = CKGEN_SPLL_DIV2_CLK;
        break;
    default:
        ClkSrc = CKGEN_OFF_CLK;
        break;
    }

    return ClkSrc;
}

/**
 * @brief Convert can clock id to register value
 * @note Function ID: DES_CKGEN_API_217
 * @param[in] ClkSrc: clock source id, value can be one of the list value
 *                - CKGEN_OFF_CLK         [40E][40][42][43]
 *                - CKGEN_HSE_DIV2_CLK    [40E][40][42][43]
 *                - CKGEN_SYS_CLK         [40E][40][42][43]
 *                - CKGEN_VHSI_DIV2_CLK        [40][42][43]
 *                - CKGEN_SPLL_DIV2_CLK   [40E][40][42][43]
 *                - CKGEN_SPLL1_CLK       [40E]
 * @return uint32: mux clock register value
 */
static uint32 Ckgen_Reg_CanClkToMuxVal(Ckgen_ClkIdType ClkSrc)
{
    uint32 MuxVal;

    switch (ClkSrc)
    {
    case CKGEN_OFF_CLK:
        MuxVal = 0U;
        break;
    case CKGEN_SYS_CLK:
        MuxVal = 2U;
        break;
    case CKGEN_SPLL_DIV2_CLK:
        MuxVal = 3U;
        break;
#if defined (AC7840E)
    case CKGEN_SPLL1_CLK:
        MuxVal = 4U;
        break;
#endif
    case CKGEN_HSE_DIV2_CLK:
        MuxVal = 1U;
        break;
    default:
        MuxVal = 0xFFU;
        break;
    }

    return MuxVal;
}

/**
 * @brief Convert can register value to clock id
 * @note Function ID: DES_CKGEN_API_212
 * @param[in] MuxVal: register value
 * @return Ckgen_ClkIdType: mux clock id
 */
static Ckgen_ClkIdType Ckgen_Reg_CanMuxValToClk(uint32 MuxVal)
{
    Ckgen_ClkIdType ClkSrc;

    switch (MuxVal)
    {
    case 0U:
        ClkSrc = CKGEN_OFF_CLK;
        break;
#if defined (AC7840E)
    case 4U:
        ClkSrc = CKGEN_SPLL1_CLK;
        break;
#endif
    case 1U:
        ClkSrc = CKGEN_HSE_DIV2_CLK;
        break;
    case 2U:
        ClkSrc = CKGEN_SYS_CLK;
        break;
    case 3U:
        ClkSrc = CKGEN_SPLL_DIV2_CLK;
        break;
    default:
        ClkSrc = CKGEN_OFF_CLK;
        break;
    }

    return ClkSrc;
}
/**
 * @brief Convert pwm clock id to register value
 * @note Function ID: DES_CKGEN_API_216
 * @param[in] ClkSrc: clock source id, value can be one of the list value
 *                - CKGEN_HSE_DIV1_CLK   [40E][40][42][43]
 *                - CKGEN_HSI_DIV1_CLK   [40E][40][42][43]
 *                - CKGEN_VHSI_DIV1_CLK  [40E][40][42][43]
 *                - CKGEN_SPLL_DIV1_CLK  [40E][40][42][43]
 *                - CKGEN_PWM_EXT_CLK0   [40E][40][42][43]
 *                - CKGEN_PWM_EXT_CLK1   [40E][40][42][43]
 *                - CKGEN_PWM_EXT_CLK2   [40E][40][42][43]
 * @return uint32: mux clock register value
 */
static uint32 Ckgen_Reg_PwmClkToMuxVal(Ckgen_ClkIdType ClkSrc)
{
    uint32 MuxVal;

    switch (ClkSrc)
    {
    case CKGEN_HSE_DIV1_CLK:
        MuxVal = 0U;
        break;
    case CKGEN_HSI_DIV1_CLK:
        MuxVal = 4U;
        break;
    case CKGEN_VHSI_DIV1_CLK:
        MuxVal = 8U;
        break;
    case CKGEN_SPLL_DIV1_CLK:
        MuxVal = 12U;
        break;
    case CKGEN_PWM_EXT_CLK0:
        MuxVal = 16U;
        break;
    case CKGEN_PWM_EXT_CLK1:
        MuxVal = 17U;
        break;
    case CKGEN_PWM_EXT_CLK2:
        MuxVal = 18U;
        break;
    default:
        MuxVal = 0xFFU;
        break;
    }

    return MuxVal;
}

/**
 * @brief Convert pwm clock register value to clock id
 * @note Function ID: DES_CKGEN_API_211
 * @param[in] MuxVal: register value
 * @return Ckgen_ClkIdType: mux clock id
 */
static Ckgen_ClkIdType Ckgen_Reg_PwmMuxValToClk(uint32 MuxVal)
{
    Ckgen_ClkIdType ClkSrc;

    switch (MuxVal)
    {
    case 0U:
        ClkSrc = CKGEN_HSE_DIV1_CLK;
        break;
    case 4U:
        ClkSrc = CKGEN_HSI_DIV1_CLK;
        break;
    case 8U:
        ClkSrc = CKGEN_VHSI_DIV1_CLK;
        break;
    case 12U:
        ClkSrc = CKGEN_SPLL_DIV1_CLK;
        break;
    case 16U:
        ClkSrc = CKGEN_PWM_EXT_CLK0;
        break;
    case 17U:
        ClkSrc = CKGEN_PWM_EXT_CLK1;
        break;
    case 18U:
        ClkSrc = CKGEN_PWM_EXT_CLK2;
        break;
    default:
        ClkSrc = CKGEN_OFF_CLK;
        break;
    }

    return ClkSrc;
}

/**
 * @brief Convert clkout clock id to register value
 * @note Function ID: DES_CKGEN_API_215
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
 * @return uint32: mux clock register value
 */
static uint32 Ckgen_Reg_ClkoutClkToMuxVal(Ckgen_ClkIdType ClkSrc)
{
    uint32 MuxVal;

    switch (ClkSrc)
    {
    case CKGEN_HSE_CLK:
        MuxVal = 0U;
        break;
#if defined (AC7840X) || defined (AC7843X)
    case CKGEN_HSI_CLK:
        MuxVal = 1U;
        break;
#endif /* AC7840X AC7843X */
    case CKGEN_VHSI_CLK:
        MuxVal = 2U;
        break;
    case CKGEN_SPLL_CLK:
        MuxVal = 3U;
        break;
    case CKGEN_FLASH_CLK:
        MuxVal = 4U;
        break;
#if defined (AC7840X)
    case CKGEN_RTC_CLK:
        MuxVal = 0x10U;
        break;
#endif /* AC7840X */
    case CKGEN_LSI_CLK:
        MuxVal = 0x20U;
        break;
    case CKGEN_LSI_128K_CLK:
        MuxVal = 0x30U;
        break;
    case CKGEN_HSE_DIV2_CLK:
        MuxVal = 0x40U;
        break;
    case CKGEN_VHSI_DIV2_CLK:
        MuxVal = 0x50U;
        break;
#if defined (AC7840X) || defined (AC7843X)
    case CKGEN_HSI_DIV2_CLK:
        MuxVal = 0x60U;
        break;
#endif /* AC7840X AC7843X */
    case CKGEN_SPLL_DIV2_CLK:
        MuxVal = 0x70U;
        break;
    case CKGEN_BUS_CLK:
        MuxVal = 0x80U;
        break;
    case CKGEN_SYS_CLK:
        MuxVal = 0xA0U;
        break;
#if defined (AC7843X)
    case CKGEN_ADC_SPLLDIV_CLK:
        MuxVal = 0xB0U;
        break;
#elif defined (AC7840E) /* AC7843X */
    case CKGEN_SPLL1_CLK:
        MuxVal = 0xB0U;
        break;
#endif /* AC7840E */
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
    case CKGEN_HSE_DIV1_CLK:
        MuxVal = 0xC0U;
        break;
    case CKGEN_VHSI_DIV1_CLK:
#if defined (AC7842X) || defined (AC7840E)
        MuxVal = 0xE0U;
#elif defined (AC7843X) /* AC7842X */
        MuxVal = 0xD0U;
#endif /* AC7843X */
        break;
#endif /* AC7842X AC7843X */
#if defined (AC7843X)
    case CKGEN_HSI_DIV1_CLK:
        MuxVal = 0xE0U;
        break;
#endif /* AC7843X */
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
    case CKGEN_SPLL_DIV1_CLK:
        MuxVal = 0xF0U;
        break;
#endif /* AC7842X AC7843X */
    default:
        MuxVal = 0xFFU;
        break;
    }

    return MuxVal;
}

/**
 * @brief Convert clkout clock register value to clock id
 * @note Function ID: DES_CKGEN_API_210
 * @param[in] MuxVal: register value
 * @return Ckgen_ClkIdType: mux clock id
 */
static Ckgen_ClkIdType Ckgen_Reg_ClkoutMuxValToClk(uint32 MuxVal)
{
    Ckgen_ClkIdType ClkSrc = CKGEN_OFF_CLK;

    switch (MuxVal)
    {
    case 0U:
        ClkSrc = CKGEN_HSE_CLK;
        break;
#if defined (AC7840X) || defined (AC7843X)
    case 1U:
        ClkSrc = CKGEN_HSI_CLK;
        break;
#endif /* AC7840X AC7843X */
    case 2U:
        ClkSrc = CKGEN_VHSI_CLK;
        break;
    case 3U:
        ClkSrc = CKGEN_SPLL_CLK;
        break;
    case 4U:
        ClkSrc = CKGEN_FLASH_CLK;
        break;
#if defined (AC7840X)
    case 0x10U:
        ClkSrc = CKGEN_RTC_CLK;
        break;
#endif /* AC7840X */
    case 0x20U:
        ClkSrc = CKGEN_LSI_CLK;
        break;
    case 0x30U:
        ClkSrc = CKGEN_LSI_128K_CLK;
        break;
    case 0x40U:
        ClkSrc = CKGEN_HSE_DIV2_CLK;
        break;
    case 0x50U:
        ClkSrc = CKGEN_VHSI_DIV2_CLK;
        break;
#if defined (AC7840X) || defined (AC7843X)
    case 0x60U:
        ClkSrc = CKGEN_HSI_DIV2_CLK;
        break;
#endif /* AC7840X AC7843X */
    case 0x70U:
        ClkSrc = CKGEN_SPLL_DIV2_CLK;
        break;
    case 0x80U:
        ClkSrc = CKGEN_BUS_CLK;
        break;
    case 0xA0U:
        ClkSrc = CKGEN_SYS_CLK;
        break;
#if defined (AC7843X)
    case 0xB0U:
        ClkSrc = CKGEN_ADC_SPLLDIV_CLK;
        break;
#endif /* AC7843X */
#if defined (AC7840E)
    case 0xB0U:
        ClkSrc = CKGEN_SPLL1_CLK;
        break;
#endif /* AC7840E */
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
    case 0xC0U:
        ClkSrc = CKGEN_HSE_DIV1_CLK;
        break;
#if defined (AC7842X) || defined (AC7840E)
    case 0xE0U:
        ClkSrc = CKGEN_VHSI_DIV1_CLK;
        break;
#elif defined (AC7843X) /* AC7842X AC7840E */
    case 0xD0U:
        ClkSrc = CKGEN_VHSI_DIV1_CLK;
        break;
#endif /* AC7843X */

#endif /* AC7843X AC7842X */
#if defined (AC7843X)
    case 0xE0U:
        ClkSrc = CKGEN_HSI_DIV1_CLK;
        break;
#endif /* AC7843X */
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
    case 0xF0U:
        ClkSrc = CKGEN_SPLL_DIV1_CLK;
        break;
#endif /* AC7843X AC7842X */
    default:
        ClkSrc = CKGEN_OFF_CLK;
        break;
    }

    return ClkSrc;
}

/**
 * @brief Set spll clock mux
 * @note Function ID: DES_CKGEN_API_209
 * @param[in] ClkSrc: clock source id, value can be one of the list value
 *                - CKGEN_HSI_CLK        [40]    [43]
 *                - CKGEN_VHSI_CLK  [40E]    [42]
 *                - CKGEN_HSE_CLK   [40E][40][42][43]
 * @return void
 */
void Ckgen_Reg_SetSPLLClkMux(Ckgen_ClkIdType ClkSrc)
{
    uint32 Value;

    Ckgen_Reg_EnableCTRLRegLock(FALSE);
    Value = Ckgen_Reg_SPLLClkToMuxVal(ClkSrc);
    Ckgen_Reg_SetPllRefClk(Value);
    Ckgen_Reg_EnableCTRLRegLock(TRUE);
}

/**
 * @brief get spll mux
 * @note Function ID: DES_CKGEN_API_208
 * @return Ckgen_ClkIdType: mux clock id
 *                - CKGEN_HSI_CLK        [40]    [43]
 *                - CKGEN_VHSI_CLK  [40E]    [42]
 *                - CKGEN_HSE_CLK   [40E][40][42][43]
 */
Ckgen_ClkIdType Ckgen_Reg_GetSPLLClkMux(void)
{
    uint32 Value;
    Ckgen_ClkIdType ClkSrc;

    Value = Ckgen_Reg_GetPllRefClk();
    ClkSrc = Ckgen_Reg_MuxValToSPLLClk(Value);

    return ClkSrc;
}

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
void Ckgen_Reg_SetClkoutClkMux(Ckgen_ClkIdType ClkSrc)
{
    uint32 Value;
    uint32 Mask;
    uint32 Pos;

    Pos = (uint32)CKGEN_CLK_OUT - CKGEN_CLK_OUT_CFG_INDEX_OFFSET;
    Mask = 0xFF;
    Value = Ckgen_Reg_ClkoutClkToMuxVal(ClkSrc);
    Ckgen_Reg_SetClkMux(CKGEN_CLK_OUT_CFG_ADDR_OFFSET, Value, Mask, Pos);
}

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
Ckgen_ClkIdType Ckgen_Reg_GetClkoutClkMux(void)
{
    uint32 Value;
    uint32 Mask;
    uint32 Pos;
    Ckgen_ClkIdType ClkSrc;

    Pos = (uint32)CKGEN_CLK_OUT - CKGEN_CLK_OUT_CFG_INDEX_OFFSET;
    Mask = 0xFF;
    Value = Ckgen_Reg_GetClkMux(CKGEN_CLK_OUT_CFG_ADDR_OFFSET, Mask, Pos);
    ClkSrc = Ckgen_Reg_ClkoutMuxValToClk(Value);

    return ClkSrc;
}

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
void Ckgen_Reg_SetCommPeriphClkMux(Ckgen_ClkIdType Clk, Ckgen_ClkIdType ClkSrc)
{
    uint32 Value;
    uint32 Mask;
    uint32 Pos;

    /* clock MUX0 configure */
#if defined (AC7842X)  || defined (AC7840X)
    if (Clk <= CKGEN_ADC1_CLK)
#elif defined(AC7840E)
    if (Clk <= CKGEN_ADC0_CLK)
#elif defined (AC7843X) /* AC7840X AC7842X */
    if (Clk <= CKGEN_EIO_CLK)
#endif /* AC7843X */
    {
        Pos = (uint32)Clk;
        Mask = 0x7UL << Pos;
#if defined(AC7840E)
        if ((CKGEN_ADC0_CLK == Clk) && (CKGEN_SPLL1_CLK == ClkSrc))
        {
            Value = Ckgen_Reg_CommClkToMuxVal(CKGEN_SPLL_DIV2_CLK);
        }
        else
#endif
        {
            Value = Ckgen_Reg_CommClkToMuxVal(ClkSrc);
        }
        Ckgen_Reg_SetClkMux(CKGEN_PERIPH_CLK_MUX0_ADDR_OFFSET, Value, Mask, Pos);
    }/* clock MUX1 configure */
#if defined (AC7843X)
    if ((CKGEN_I2C0_CLK <= Clk) && (Clk <= CKGEN_ADC1_CLK))
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET;

        Mask = 0x7UL << Pos;
        if ((((CKGEN_ADC0_CLK == Clk) || (CKGEN_ADC1_CLK == Clk))) && (CKGEN_ADC_SPLLDIV_CLK == ClkSrc))
        {
            Value = Ckgen_Reg_CommClkToMuxVal(CKGEN_SPLL_DIV2_CLK);
        }
        else
        {
            Value = Ckgen_Reg_CommClkToMuxVal(ClkSrc);
        }
        Ckgen_Reg_SetClkMux(CKGEN_PERIPH_CLK_MUX1_ADDR_OFFSET, Value, Mask, Pos);
    }
#else
#if defined (AC7842X)  || defined (AC7840X)
    /* clock MUX1 configure */
    else if ((CKGEN_PCT_CLK == Clk) || (CKGEN_EIO_CLK == Clk))
#elif defined(AC7840E)
    else if (CKGEN_EIO_CLK == Clk)
#endif /* AC7840X AC7842X */
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET;
        Mask = 0x7UL << Pos;
        Value = Ckgen_Reg_CommClkToMuxVal(ClkSrc);
        Ckgen_Reg_SetClkMux(CKGEN_PERIPH_CLK_MUX1_ADDR_OFFSET, Value, Mask, Pos);
    }
#endif /* AC7843X */
    /* clock MUX2 configure */
#if defined (AC7842X)
    else if ((CKGEN_UART0_CLK <= Clk) && (Clk <= CKGEN_SPI3_CLK))
#elif defined (AC7843X) /* AC7842X */
    else if ((CKGEN_SPI0_CLK <= Clk) && (Clk <= CKGEN_SPI4_CLK))
#elif defined (AC7840X) /* AC7843X */
    else if ((CKGEN_UART0_CLK <= Clk) && (Clk <= CKGEN_UART3_CLK))
#elif defined(AC7840E)
    else if ((CKGEN_UART0_CLK <= Clk) && (Clk <= CKGEN_UART2_CLK))
#endif /* AC7840X */
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX2_INDEX_OFFSET;
        Mask = 0x7UL << Pos;
        Value = Ckgen_Reg_CommClkToMuxVal(ClkSrc);
        Ckgen_Reg_SetClkMux(CKGEN_PERIPH_CLK_MUX2_ADDR_OFFSET, Value, Mask, Pos);
    }
#if defined (AC7843X)
    /* clock MUX4 configure */
    else if ((CKGEN_UART0_CLK <= Clk) && (Clk <= CKGEN_UART7_CLK))
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX4_INDEX_OFFSET;
        Mask = 0x7UL << Pos;
        Value = Ckgen_Reg_CommClkToMuxVal(ClkSrc);
        Ckgen_Reg_SetClkMux(CKGEN_PERIPH_CLK_MUX4_ADDR_OFFSET, Value, Mask, Pos);
    }
#endif /* AC7843X */
    else
    {
        /* nothing */
    }
}

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
void Ckgen_Reg_SetPwmClkMux(Ckgen_ClkIdType Clk, Ckgen_ClkIdType ClkSrc)
{
    uint32 Value;
    uint32 Mask;
    uint32 Pos;

#if defined (AC7842X) || defined (AC7840X) || defined (AC7840E)
    Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET;
    Mask = 0x1FUL << Pos;
    Value = Ckgen_Reg_PwmClkToMuxVal(ClkSrc);
    Ckgen_Reg_SetClkMux(CKGEN_PERIPH_CLK_MUX3_ADDR_OFFSET, Value, Mask, Pos);
#endif /* AC7840X AC7842X AC7840E */
#if defined (AC7843X)
    if ((CKGEN_PWM0_CLK <= Clk) && (Clk <= CKGEN_PWM5_CLK))
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX5_INDEX_OFFSET;
        Mask = 0x1FUL << Pos;
        Value = Ckgen_Reg_PwmClkToMuxVal(ClkSrc);
        Ckgen_Reg_SetClkMux(CKGEN_PERIPH_CLK_MUX5_ADDR_OFFSET, Value, Mask, Pos);
    }
    else
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX6_INDEX_OFFSET;
        Mask = 0x1FUL << Pos;
        Value = Ckgen_Reg_PwmClkToMuxVal(ClkSrc);
        Ckgen_Reg_SetClkMux(CKGEN_PERIPH_CLK_MUX6_ADDR_OFFSET, Value, Mask, Pos);
    }
#endif /* AC7843X */
}

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
void Ckgen_Reg_SetCanClkMux(Ckgen_ClkIdType Clk, Ckgen_ClkIdType ClkSrc)
{
    uint32 Value;
    uint32 Mask;
    uint32 Pos;

#if defined (AC7840X) || defined (AC7842X)
    Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET;
    Mask = 0x3UL << Pos;
    Value = Ckgen_Reg_CanClkToMuxVal(ClkSrc);
    Ckgen_Reg_SetClkMux(CKGEN_PERIPH_CLK_MUX1_ADDR_OFFSET, Value, Mask, Pos);
#elif defined (AC7840E) /* AC7840X AC7842X*/
    Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET;
    Mask = 0x7UL << Pos;
    Value = Ckgen_Reg_CanClkToMuxVal(ClkSrc);
    Ckgen_Reg_SetClkMux(CKGEN_PERIPH_CLK_MUX1_ADDR_OFFSET, Value, Mask, Pos);
#elif defined (AC7843X) /* AC7840E*/
    Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET;
    Mask = 0x3UL << Pos;
    Value = Ckgen_Reg_CanClkToMuxVal(ClkSrc);
    Ckgen_Reg_SetClkMux(CKGEN_PERIPH_CLK_MUX3_ADDR_OFFSET, Value, Mask, Pos);
#endif /* AC7843X */
}

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
Ckgen_ClkIdType Ckgen_Reg_GetCommPeriphClkMux(Ckgen_ClkIdType Clk)
{
    uint32 Value;
    uint32 Mask;
    uint32 Pos;
    Ckgen_ClkIdType ClkSrc;

#if defined (AC7842X)  || defined (AC7840X)
    if (Clk <= CKGEN_ADC1_CLK)
#elif  defined (AC7840E) /* AC7840X AC7842X */
    if (Clk <= CKGEN_ADC0_CLK)
#elif defined (AC7843X) /* AC7840E */
    if (Clk <= CKGEN_EIO_CLK)
#endif /* AC7843X */
    {
        Pos = (uint32)Clk;
        Mask = 0x7UL << Pos;
        Value = Ckgen_Reg_GetClkMux(CKGEN_PERIPH_CLK_MUX0_ADDR_OFFSET, Mask, Pos);
        ClkSrc = Ckgen_Reg_CommMuxValToClk(Value);
#if defined (AC7840E)
        if ((CKGEN_ADC0_CLK == Clk)&& (CKGEN_SPLL_DIV2_CLK == ClkSrc))
        {
            ClkSrc = CKGEN_SPLL1_CLK;
        }
#endif
    }
#if defined (AC7843X)
    else if ((CKGEN_I2C0_CLK <= Clk) && (Clk <= CKGEN_ADC1_CLK))
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET;
        Mask = 0x7UL << Pos;
        Value = Ckgen_Reg_GetClkMux(CKGEN_PERIPH_CLK_MUX1_ADDR_OFFSET, Mask, Pos);
        ClkSrc = Ckgen_Reg_CommMuxValToClk(Value);
        if ((((CKGEN_ADC0_CLK == Clk) || (CKGEN_ADC1_CLK == Clk))) && (CKGEN_SPLL_DIV2_CLK == ClkSrc))
        {
            ClkSrc = CKGEN_ADC_SPLLDIV_CLK;
        }
    }
#else
#if defined (AC7842X)  || defined (AC7840X)
    else if ((CKGEN_PCT_CLK == Clk) || (CKGEN_EIO_CLK == Clk))
#elif  defined (AC7840E)
    /* clock MUX1 configure */
    else if (CKGEN_EIO_CLK == Clk)
#endif /* AC7840X AC7842X */
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET;
        Mask = 0x7UL << Pos;
        Value = Ckgen_Reg_GetClkMux(CKGEN_PERIPH_CLK_MUX1_ADDR_OFFSET, Mask, Pos);
        ClkSrc = Ckgen_Reg_CommMuxValToClk(Value);
    }
#endif /* AC7843X */
    /* clock MUX2 configure */
#if defined (AC7842X)
    else if ((CKGEN_UART0_CLK <= Clk) && (Clk <= CKGEN_SPI3_CLK))
#elif defined (AC7843X) /* AC7842X */
    else if ((CKGEN_SPI0_CLK <= Clk) && (Clk <= CKGEN_SPI4_CLK))
#elif defined (AC7840X) /* AC7843X */
    else if ((CKGEN_UART0_CLK <= Clk) && (Clk <= CKGEN_UART3_CLK))
#elif  defined (AC7840E)
    else if ((CKGEN_UART0_CLK <= Clk) && (Clk <= CKGEN_UART2_CLK))
#endif /* AC7840X */
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX2_INDEX_OFFSET;
        Mask = 0x7UL << Pos;
        Value = Ckgen_Reg_GetClkMux(CKGEN_PERIPH_CLK_MUX2_ADDR_OFFSET, Mask, Pos);
        ClkSrc = Ckgen_Reg_CommMuxValToClk(Value);
    }
    /* clock MUX3 configure */
#if defined (AC7843X)
    /* clock MUX4 configure */
    else if ((CKGEN_UART0_CLK <= Clk) && (Clk <= CKGEN_UART7_CLK))
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX4_INDEX_OFFSET;
        Mask = 0x7UL << Pos;
        Value = Ckgen_Reg_GetClkMux(CKGEN_PERIPH_CLK_MUX4_ADDR_OFFSET, Mask, Pos);
        ClkSrc = Ckgen_Reg_CommMuxValToClk(Value);
    }
#endif /* AC7843X */
    else
    {
        ClkSrc = CKGEN_OFF_CLK;
    }

    return ClkSrc;
}

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
Ckgen_ClkIdType Ckgen_Reg_GetPwmClkMux(Ckgen_ClkIdType Clk)
{
    uint32 Value;
    uint32 Mask;
    uint32 Pos;
    Ckgen_ClkIdType ClkSrc;

#if defined (AC7842X) || defined (AC7840X) || defined (AC7840E)
    Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET;
    Mask = 0x1FUL << Pos;
    Value = Ckgen_Reg_GetClkMux(CKGEN_PERIPH_CLK_MUX3_ADDR_OFFSET, Mask, Pos);
    ClkSrc = Ckgen_Reg_PwmMuxValToClk(Value);
#endif /* AC7842X AC7840X */
#if defined (AC7843X)
    if ((CKGEN_PWM0_CLK <= Clk) && (Clk <= CKGEN_PWM5_CLK))
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX5_INDEX_OFFSET;
        Mask = 0x1FUL << Pos;
        Value = Ckgen_Reg_GetClkMux(CKGEN_PERIPH_CLK_MUX5_ADDR_OFFSET, Mask, Pos);
        ClkSrc = Ckgen_Reg_PwmMuxValToClk(Value);
    }
    else
    {
        Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX6_INDEX_OFFSET;
        Mask = 0x1FUL << Pos;
        Value = Ckgen_Reg_GetClkMux(CKGEN_PERIPH_CLK_MUX6_ADDR_OFFSET, Mask, Pos);
        ClkSrc = Ckgen_Reg_PwmMuxValToClk(Value);
    }
#endif /* AC7843X */

    return ClkSrc;
}

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
Ckgen_ClkIdType Ckgen_Reg_GetCanClkMux(Ckgen_ClkIdType Clk)
{
    uint32 Value;
    uint32 Mask;
    uint32 Pos;
    Ckgen_ClkIdType ClkSrc;

#if defined (AC7840X) || defined (AC7842X)
    Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET;
    Mask = 0x3UL << Pos;
    Value = Ckgen_Reg_GetClkMux(CKGEN_PERIPH_CLK_MUX1_ADDR_OFFSET, Mask, Pos);
    ClkSrc = Ckgen_Reg_CanMuxValToClk(Value);
#elif defined (AC7840E)
    Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET;
    Mask = 0x7UL << Pos;
    Value = Ckgen_Reg_GetClkMux(CKGEN_PERIPH_CLK_MUX1_ADDR_OFFSET, Mask, Pos);
    ClkSrc = Ckgen_Reg_CanMuxValToClk(Value);
#elif defined (AC7843X)
    Pos = (uint32)Clk - CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET;
    Mask = 0x3UL << Pos;
    Value = Ckgen_Reg_GetClkMux(CKGEN_PERIPH_CLK_MUX3_ADDR_OFFSET, Mask, Pos);
    ClkSrc = Ckgen_Reg_CanMuxValToClk(Value);
#endif

    return ClkSrc;
}
/* =============================================  EOF  ============================================== */
