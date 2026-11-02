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
* @file Ckgen_Hal.h
*
* @brief Ckgen hal interface for SDK
*
*/

#ifndef CKGEN_HAL_H
#define CKGEN_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Ckgen_Reg.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/*!
 * @brief peripheral clock mux configure structure.
 */
typedef struct
{
    Ckgen_ClkIdType Clk; /*!< to be configured clock id */
    Ckgen_ClkIdType ClkSrc; /*!< clock source id */
} Ckgen_ClkMuxCfgType;

/*!
 * @brief peripheral clock division configure structure.
 */
typedef struct
{
    uint8 Div; /*!< clock division */
    Ckgen_ClkIdType Clk; /*!< to be configured clock id */
} Ckgen_ClkDivCfgType;

/*!
 * @brief clkout clock configure structure.
 */
typedef struct
{
    boolean Enable; /*!< Enable or disable clock */
    Ckgen_ClkoutDivType Div; /*!< clock division */
    Ckgen_ClkIdType ClkSrc; /*!< clock source id */
} Ckgen_ClkoutCfgType;

/*!
 * @brief low power clock configure structure.
 */
typedef struct
{
    boolean EnableConfigured;
#if defined (AC7840X)
    Ckgen_ClkIdType RTCClkSrc; /*!< clock source id */
#endif /* AC7840X */
    Ckgen_ClkIdType LSIClkSrc; /*!< clock source id */
} Ckgen_LPClkCfgType;

/*!
 * @brief external clock frequency configure structure.
 */
typedef struct
{
    uint32 RtcClkInFreq;
    uint32 PwmExtClk0Freq;
    uint32 PwmExtClk1Freq;
    uint32 PwmExtClk2Freq;
} Ckgen_ExternalClkFreqCfgType;

/*!
 * @brief Pll clock configure structure.
 */
typedef struct
{
    boolean Enable; /*!< Enable or disable clock*/
    boolean EnableLD; /*!< Enable or disable lock detect */
#if defined (AC7840X) || defined (AC7842X)
    boolean EnableSamp0LD; /*!< Enable or disable sample0 lock detect */
    boolean EnableSamp1LD; /*!< Enable or disable sample1 lock detect*/
#endif /* AC7840X AC7842X */
    uint8 OutputDiv1; /*!< the value ranges 1 2 3 4 ... 63 64 [42][43]: 1 2 4 ...64 */
    uint8 OutputDiv2; /*!< the value ranges 1 2 3 4 ... 63 64 */
#if defined (AC7843X)
    uint8 AdcSpllDiv;
#endif /* AC7843X */
    Ckgen_ClkIdType ClkSrc; /*!< clock source is HSE or HSI */
    Ckgen_PllLdDlySelType DlySel;
    Ckgen_PllPreDivType PreDiv;
    uint8 FbkDiv; /*!< the value ranges 5 6 7 ... 254 255 */
    Ckgen_PllPosDivType PosDiv;
#if defined (AC7840E)
    Ckgen_PllPosDivType PosDiv1;
#endif /* AC7840E */
} Ckgen_PllCfgType;

/*!
 * @brief clock source configure structure.
 */
typedef struct
{
    boolean Enable; /*!< Enable or disable clock */
    boolean EnableInVLPS; /*!< [40][43]:HSI [42]:VHSI Enable or disable In VLPS */
    uint8 OutputDiv1; /*!< the value ranges 1 2 3 4 ... 63 64 [42][43]: 1 2 4 ...64 */
    uint8 OutputDiv2; /*!< the value ranges 1 2 3 4 ... 63 64 */
    Ckgen_ClkIdType Clk; /*!< clock is VHSI or HSI */
} Ckgen_ClkSrcCfgType;

/*!
 * @brief Xosc clock configure structure.
 */
typedef struct
{
    boolean Enable; /*!< Enable or disable HSE */
    boolean EnableBypass; /*!< true:use OSC_IN false:OSC_IN ??? */
    boolean EnableMonitor; /*!< HSE disable and Enable EnableMonitor it causes the system reboot */
    uint8 OutputDiv1; /*!< [40] the value ranges 1 2 3 4 ... 63 64 [42][43]: 1 2 4 ...64 */
    uint8 OutputDiv2; /*!< the value ranges 1 2 3 4 ... 63 64 */
    uint32 Freq; /*!< clock frequency */
} Ckgen_XoscClkCfgType;

/*!
 * @brief system clock configure structure.
 */
typedef struct
{
    Ckgen_SysClkModeType Mode; /*!< the value ranges RUN or VLPR mode */
    Ckgen_ClkIdType ClkSrc; /*!< the value ranges RUN: VHSI, HSE, HSI, PLL; VLPR: HSI*/
    uint8 SysDiv; /*!< the value ranges 1 2 3 4 ... 15 16 */
    uint8 BusDiv; /*!< the value ranges 1 2 3 4 ... 15 16 */
} Ckgen_SysClkCfgType;

/*!
 * @brief Distribute clock configure structure.
 */
typedef struct
{
    uint8 SysClkCfgCnt;
    uint16 ClkDivCfgCnt;
    uint16 ClkMuxCfgCnt;
    const Ckgen_ClkDivCfgType *ClkDivCfgs;
    const Ckgen_ClkMuxCfgType *ClkMuxCfgs;
    const Ckgen_SysClkCfgType *SysClkCfgs;
    const Ckgen_ClkoutCfgType *ClkoutCfg;
} Ckgen_ClkDistributeCfgType;

/*!
 * @brief Clock tree configure structure.
 */
typedef struct
{
    uint8 ClkSrcCfgCnt;
    const Ckgen_ClkSrcCfgType *ClkSrcCfgs;
    const Ckgen_PllCfgType *pllCfg;
    const Ckgen_XoscClkCfgType *XoscCfg;
    const Ckgen_LPClkCfgType *LPCfg;
    const Ckgen_ExternalClkFreqCfgType *ExtClkCfg;
} Ckgen_ClkTreeCfgType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/
/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

/**
 * @brief Get clock status
 * @note Function ID: DES_CKGEN_API_009
 * @param[in] Clk: clock id, the value ranges are listed below:
 *  - CKGEN_VHSI_CLK
 *  - CKGEN_HSI_CLK
 *  - CKGEN_HSI_VLPS_CLK
 *  - CKGEN_SPLL_CLK
 *  - CKGEN_HSE_CLK
 * @return Hal_StatusType: clock status range is the STATUS_CLK_ON STATUS_CLK_OFF STATUS_CLK_STABLE STATUS_CLK_UNSTABLE
 */
Hal_StatusType Ckgen_Hal_GetClkStatus(Ckgen_ClkIdType Clk);

/**
 * @brief Set the peripheral clock of the source
 * @note Function ID: DES_CKGEN_API_008
 * @param[in] Clk: clock id
 * @param[in] ClkSrc: clock source
 * @return Hal_StatusType: Set success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_SetPeriphClkMux(Ckgen_ClkIdType Clk, Ckgen_ClkIdType ClkSrc);

/**
 * @brief Get the clock source of the clock
 * @note Function ID: DES_CKGEN_API_007
 * @param[in] Clk: clock id, the value ranges are listed below:
 * @return Ckgen_ClkIdType: clock source id
 */
Ckgen_ClkIdType Ckgen_Hal_GetClkMux(Ckgen_ClkIdType Clk);

/**
 * @brief Set peripheral clock division
 * @note Function ID: DES_CKGEN_API_006
 * @param[in] Clk: clock id,value can be one of the list value
 *                - CKGEN_CAN0_CLK    [40][42][43]
 *                - CKGEN_CAN1_CLK    [40][42][43]
 *                - CKGEN_CAN2_CLK    [40][42][43]
 *                - CKGEN_CAN3_CLK    [40][42][43]
 *                - CKGEN_CAN4_CLK    [42][43]
 *                - CKGEN_CAN5_CLK    [42][43]
 *                - CKGEN_CAN0_TS_CLK [40][42]
 *                - CKGEN_CAN1_TS_CLK [40][42]
 *                - CKGEN_CAN2_TS_CLK [40][42]
 *                - CKGEN_CAN3_TS_CLK [40][42]
 *                - CKGEN_CAN4_TS_CLK [40][42]
 *                - CKGEN_CAN5_TS_CLK [40][42]
 *                - CKGEN_PCT_CLK     [40][42][43]
 *                - CKGEN_TPIU_CLK    [40][42][43]
 * @param[in] Div: clock division
 * @return Hal_StatusType: Set success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_SetPeriphClkDiv(Ckgen_ClkIdType Clk, uint8 Div);

/**
 * @brief Get clock frequency
 * @note Function ID: DES_CKGEN_API_005
 * @param[in] Clk: clock id, the value ranges are listed below:
 * @param[out] Freq: the pointer to the uint32 value
 * @return Hal_StatusType: Get success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_GetFreq(Ckgen_ClkIdType Clk, uint32 *Freq);

/**
 * @brief Enable or disbale bus clock
 * @note Function ID: DES_CKGEN_API_004
 * @param[in] Clk: clock id
 * bus clock ranges are listed below:
 * @param[in] IsEnable: turn the clock on or off
 * @return Hal_StatusType: Operation success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_EnablePeriphClk(Ckgen_BusClkIdType Clk, boolean IsEnable);

/**
 * @brief Set sysclk
 * @note Function ID: DES_CKGEN_API_003
 * @param[in] CfgPtr: The pointer to the Ckgen_SysClkCfgType structure
 * @return Hal_StatusType: Set success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_SetSysClk(const Ckgen_SysClkCfgType *CfgPtr);

/**
 * @brief Wait clock to Stable and distribute system and module clock
 * @note Function ID: DES_CKGEN_API_002
 * @param[in] CfgPtr: the pointer to the Ckgen_ClkDistributeCfgType structure
 * @return Hal_StatusType: Distribute success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_DistributeClk(const Ckgen_ClkDistributeCfgType *CfgPtr);

/**
 * @brief Initialize clocks source
 * @note Function ID: DES_CKGEN_API_001
 * @param[in] CfgPtr: the pointer to the Ckgen_AllClkCfgType structure
 * @return Hal_StatusType: Distribute success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_InitClk(const Ckgen_ClkTreeCfgType *CfgPtr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CKGEN_HAL_H */
