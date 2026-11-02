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
 * @file clock_cfg.c
 *
 * @brief The file provides variable definitions required for clock configuration.
 *
 */

/* ===========================================  Includes  =========================================== */
#include "clock_cfg.h"

/* ============================================  Define  ============================================ */
#define INIT_LOWPOWER_CLOCK  0U  /* 定义低功耗时钟是否初始化, 0表示不进行初始化, 使用芯片默认值 */

/* ===========================================  Typedef  ============================================ */


/* ==========================================  Variables  =========================================== */

/* 内部高速RC振荡器时钟配置 */
static const Ckgen_ClkSrcCfgType s_internalOscCfg[] =
{
    {
        .Enable = TRUE,             /*!< Enable or disable clock */
        .EnableInVLPS = FALSE,      /*!< Enable or disable In VLPS */
        .OutputDiv1 = 2U,
        .OutputDiv2 = 1U,           /*!< the value ranges 1 2 3 4 ... 63 64 */
        .Clk = CKGEN_HSI_CLK        /*!< clock is VHSI or HSI */
    },
    {
        .Enable = TRUE,             /*!< Enable or disable clock */
        .EnableInVLPS = FALSE,      /*!< VHSI clock AC7843x and AC7840x ignore this config */
        .OutputDiv1 = 2U,
        .OutputDiv2 = 1U,           /*!< the value ranges 1 2 3 4 ... 63 64 */
        .Clk = CKGEN_VHSI_CLK       /*!< clock is VHSI or HSI */
    }
};

/* PLL配置 */
static const Ckgen_PllCfgType s_pllConfig =
{
    .Enable = TRUE,                             /*!< Enable or disable clock*/
    .EnableLD = TRUE,                           /*!< Enable or disable lock detect */
    #if defined (AC7840X)
    .EnableSamp0LD = TRUE,                      /*!< Enable or disable sample0 lock detect */
    .EnableSamp1LD = TRUE,                      /*!< Enable or disable sample1 lock detect*/
    #endif
    .OutputDiv1 = 2U,
    .OutputDiv2 = 2U,                           /*!< the value ranges 1 2 3 4 ... 63 64 */
    #if defined (AC7843X)
    .AdcSpllDiv = 2U,
    #endif
    .ClkSrc = CKGEN_HSE_CLK,                    /*!< clock source is HSE or HSI */
    #if defined (AC7840X)
    .DlySel = CKGEN_PLL_LD_DLY_SEL_2200PS,
    #elif defined (AC7843X)  || defined (AC7842X)
    .DlySel = CKGEN_PLL_LD_DLY_SEL_256CYCLE,
    #endif
    #if defined (AC7840X) || defined (AC7842X)
    .PreDiv = CKGEN_PLL_PRE_DIV_1U,
    .FbkDiv = 120U,                             /*!< the value ranges 5 6 7 ... 254 255 */
    .PosDiv = CKGEN_PLL_POS_DIV_8U
    #elif defined (AC7843X)
    .PreDiv = CKGEN_PLL_PRE_DIV_1U,
    .FbkDiv = 90U,                              /*!< the value ranges 5 6 7 ... 254 255 */
    .PosDiv = CKGEN_PLL_POS_DIV_4U
    #endif
};

/* 外部振荡器时钟配置 */
static const Ckgen_XoscClkCfgType s_xoscConfig =
{
    .Enable = TRUE,             /*!< Enable or disable HSE */
    .EnableBypass = FALSE,      /*!< true:use OSC_IN false:XOSC */
    .EnableMonitor = TRUE,      /*!< HSE disable and Enable EnableMonitor it causes the system reboot */
    .OutputDiv1 = 2U,
    .OutputDiv2 = 1U,           /*!< the value ranges 1 2 3 4 ... 63 64 */
    .Freq = 8000000UL,          /*!< clock frequency */
};

/* 低功耗时钟配置 */
/* 注意低功耗时钟每次上电后只可配置一次, 若需要修改配置则需要重新上下电; 默认值为CKGEN_LSI_128K_CLK, 若需要配置为其他值可以修改后开启条件编译 */
#if defined (INIT_LOWPOWER_CLOCK) && (INIT_LOWPOWER_CLOCK == 1U)
static const Ckgen_LPClkCfgType s_lpClkCfg =
{
    #if defined (AC7840X)
    .RTCClkSrc = CKGEN_LSI_32K_CLK,         /*!< clock source id */
    #endif
    .LSIClkSrc = CKGEN_LSI_128K_CLK         /*!< clock source id */
};
#endif

/* 外部灌入时钟频率 */
static const Ckgen_ExternalClkFreqCfgType s_extClkCfg =
{
    .RtcClkInFreq = 0U,
    .PwmExtClk0Freq = 0U,
    .PwmExtClk1Freq = 0U,
    .PwmExtClk2Freq = 0U
};

/* 时钟输出配置(调试使用) */
static const Ckgen_ClkoutCfgType s_clkOutCfg =
{
    .Enable = FALSE,                /*!< Enable or disable clock */
    .Div = CKGEN_CLKOUT_DIV_BY_2,   /*!< clock division, if value is not 0 will be used first for calculation */
    .ClkSrc = CKGEN_SPLL_DIV2_CLK   /*!< clock source id */
};

/* 部分模块分频配置 */
static const Ckgen_ClkDivCfgType s_clkDivCfg[] =
{
    /* CAN功能时钟分频配置, 注意最大不能超过80MHz */
    {
        #if defined (AC7840X) || defined (AC7842X)
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #elif defined (AC7843X)
        .Div = 3U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #endif
        .Clk = CKGEN_CAN0_CLK           /*!< to be configured clock id */
    },
    {
        #if defined (AC7840X) || defined (AC7842X)
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #elif defined (AC7843X)
        .Div = 3U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #endif
        .Clk = CKGEN_CAN1_CLK           /*!< to be configured clock id */
    },
    {
        #if defined (AC7840X) || defined (AC7842X)
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #elif defined (AC7843X)
        .Div = 3U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #endif
        .Clk = CKGEN_CAN2_CLK           /*!< to be configured clock id */
    },
    {
        #if defined (AC7840X) || defined (AC7842X)
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #elif defined (AC7843X)
        .Div = 3U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #endif
        .Clk = CKGEN_CAN3_CLK           /*!< to be configured clock id */
    },
    #if defined (AC7842X) || defined (AC7843X)
    {
        #if defined (AC7842X)
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #elif defined (AC7843X)
        .Div = 3U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #endif
        .Clk = CKGEN_CAN4_CLK           /*!< to be configured clock id */
    },
    {
        #if defined (AC7842X)
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #elif defined (AC7843X)
        .Div = 3U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        #endif
        .Clk = CKGEN_CAN5_CLK           /*!< to be configured clock id */
    },
    #endif

    /* CAN时间戳时钟分频设置 */
    #if defined (AC7840X) || defined (AC7842X)
    {
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        .Clk = CKGEN_CAN0_TS_CLK        /*!< to be configured clock id */
    },
    {
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        .Clk = CKGEN_CAN1_TS_CLK        /*!< to be configured clock id */
    },
    {
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        .Clk = CKGEN_CAN2_TS_CLK        /*!< to be configured clock id */
    },
    {
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        .Clk = CKGEN_CAN3_TS_CLK        /*!< to be configured clock id */
    },
    #endif
    #if defined (AC7842X)
    {
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        .Clk = CKGEN_CAN4_TS_CLK        /*!< to be configured clock id */
    },
    {
        .Div = 2U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        .Clk = CKGEN_CAN5_TS_CLK        /*!< to be configured clock id */
    },
    #endif

    {
        .Div = 1U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        .Clk = CKGEN_PCT_CLK            /*!< to be configured clock id */
    },
    {
        .Div = 1U,                      /*!< clock division, if value is not 0 will be used first for calculation */
        .Clk = CKGEN_TPIU_CLK           /*!< to be configured clock id */
    }
};

/* 外设模块时钟源以及部分模块时钟分频配置 */
static const Ckgen_ClkMuxCfgType s_moduleClkCfg[] =
{
    /* TIMER时钟配置 */
    {
        .Clk = CKGEN_TIMER_CLK,             /*!< to be configured clock id */
        .ClkSrc = CKGEN_SPLL_DIV2_CLK       /*!< clock source id */
    },
    /* PCT时钟配置, 结合s_clkDivCfg中的分频配置使用 */
    {
        .Clk = CKGEN_PCT_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    /* EIO时钟配置 */
    {
        .Clk = CKGEN_EIO_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    /* I2C时钟配置 */
    {
        .Clk = CKGEN_I2C0_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    #if defined (AC7842X) || defined (AC7843X)
    {
        .Clk = CKGEN_I2C1_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    #endif
    #if defined (AC7843X)
    {
        .Clk = CKGEN_I2C2_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    #endif
    /* ADC时钟配置, 不能超过BUS_CLK [AC7843x Typical: 90MHz] [AC7840/2x Typical: 60MHz] */
    {
        .Clk = CKGEN_ADC0_CLK,
        #if defined (AC7843X)
        .ClkSrc = CKGEN_ADC_SPLLDIV_CLK                 /* 结合s_pllConfig.AdcSpllDiv中的分频配置使用 */
        #elif defined (AC7840X) || defined (AC7842X)
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
        #endif
    },
    {
        .Clk = CKGEN_ADC1_CLK,
        #if defined (AC7843X)
        .ClkSrc = CKGEN_ADC_SPLLDIV_CLK
        #elif defined (AC7840X) || defined (AC7842X)
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
        #endif
    },
    /* SPI时钟配置 */
    {
        .Clk = CKGEN_SPI0_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    {
        .Clk = CKGEN_SPI1_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    {
        .Clk = CKGEN_SPI2_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    #if defined (AC7842X) || defined (AC7843X)
    {
        .Clk = CKGEN_SPI3_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    #endif
    #if defined (AC7843X)
    {
        .Clk = CKGEN_SPI4_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    #endif
    /* CAN功能时钟配置, 注意最大不能超过80MHz, 结合s_clkDivCfg中的分频配置使用 */
    {
        .Clk = CKGEN_CAN0_CLK,
        .ClkSrc = CKGEN_SYS_CLK
    },
    {
        .Clk = CKGEN_CAN1_CLK,
        .ClkSrc = CKGEN_SYS_CLK
    },
    {
        .Clk = CKGEN_CAN2_CLK,
        .ClkSrc = CKGEN_SYS_CLK
    },
    {
        .Clk = CKGEN_CAN3_CLK,
        .ClkSrc = CKGEN_SYS_CLK
    },
    #if defined (AC7842X) || defined (AC7843X)
    {
        .Clk = CKGEN_CAN4_CLK,
        .ClkSrc = CKGEN_SYS_CLK
    },
    {
        .Clk = CKGEN_CAN5_CLK,
        .ClkSrc = CKGEN_SYS_CLK
    },
    #endif
    /* UART时钟配置 */
    {
        .Clk = CKGEN_UART0_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    {
        .Clk = CKGEN_UART1_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    {
        .Clk = CKGEN_UART2_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    {
        .Clk = CKGEN_UART3_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    #if defined (AC7843X)
    {
        .Clk = CKGEN_UART4_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    {
        .Clk = CKGEN_UART5_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    {
        .Clk = CKGEN_UART6_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    {
        .Clk = CKGEN_UART7_CLK,
        .ClkSrc = CKGEN_SPLL_DIV2_CLK
    },
    #endif
    /* PWM时钟配置 PWM寄存器的PWM_INIT[CLKSRC]配置为0b11时, 此处配置才生效 */
    {
        .Clk = CKGEN_PWM0_CLK,
        .ClkSrc = CKGEN_SPLL_DIV1_CLK
    },
    {
        .Clk = CKGEN_PWM1_CLK,
        .ClkSrc = CKGEN_SPLL_DIV1_CLK
    },
    {
        .Clk = CKGEN_PWM2_CLK,
        .ClkSrc = CKGEN_SPLL_DIV1_CLK
    },
    {
        .Clk = CKGEN_PWM3_CLK,
        .ClkSrc = CKGEN_SPLL_DIV1_CLK
    },
    {
        .Clk = CKGEN_PWM4_CLK,
        .ClkSrc = CKGEN_SPLL_DIV1_CLK
    },
    {
        .Clk = CKGEN_PWM5_CLK,
        .ClkSrc = CKGEN_SPLL_DIV1_CLK
    },
    #if defined (AC7843X)
    {
        .Clk = CKGEN_PWM6_CLK,
        .ClkSrc = CKGEN_SPLL_DIV1_CLK
    },
    {
        .Clk = CKGEN_PWM7_CLK,
        .ClkSrc = CKGEN_SPLL_DIV1_CLK
    },
    #endif
};

/* 不同运行模式下的系统时钟配置 */
static const Ckgen_SysClkCfgType s_systemClkCfg[] =
{
    {
        .Mode = CKGEN_SYS_CLK_MODE_RUN,     /*!< the value ranges RUN or VLPR mode */
        .ClkSrc = CKGEN_SPLL_CLK,           /*!< the value ranges RUN: VHSI, HSE, HSI, PLL; VLPR: HSI*/
        .SysDiv = 1U,                       /*!< the value ranges 1 2 3 4 ... 15 16 */
        .BusDiv = 2U                        /*!< the value ranges 1 2 3 4 ... 15 16 */
    },
    #if defined (AC7840X)  || defined (AC7843X)
    {
        .Mode = CKGEN_SYS_CLK_MODE_VLPR,    /*!< the value ranges RUN or VLPR mode */
        .ClkSrc = CKGEN_HSI_CLK,            /*!< the value ranges RUN: VHSI, HSE, HSI, PLL; VLPR: HSI*/
        .SysDiv = 1U,                       /*!< the value ranges 1 2 3 4 ... 15 16 */
        .BusDiv = 2U                        /*!< the value ranges 1 2 3 4 ... 15 16 */
    }
    #endif
};

/* Clock Source配置 */
const Ckgen_ClkTreeCfgType g_clockGenCfg =
{
    .ClkSrcCfgCnt = (sizeof(s_internalOscCfg) / sizeof(Ckgen_ClkSrcCfgType)),
    .ClkSrcCfgs = s_internalOscCfg,
    .pllCfg = &s_pllConfig,
    .XoscCfg = &s_xoscConfig,
    #if defined (INIT_LOWPOWER_CLOCK) && (INIT_LOWPOWER_CLOCK == 1U)
    .LPCfg = &s_lpClkCfg, /* 注意低功耗时钟每次上电后只可配置一次, 若需要修改配置则需要重新上下电; 默认值为CKGEN_LSI_128K_CLK, 若需要配置为其他值可以修改后开启条件编译 */
    #else
    .LPCfg = NULL_PTR,
    #endif
    .ExtClkCfg = &s_extClkCfg
};

/* 运行时系统时钟配置和外设模块时钟配置 */
const Ckgen_ClkDistributeCfgType g_clkDistributeCfg =
{
    .SysClkCfgCnt = (sizeof(s_systemClkCfg) / sizeof(Ckgen_SysClkCfgType)),
    .ClkDivCfgCnt = (sizeof(s_clkDivCfg) / sizeof(Ckgen_ClkDivCfgType)),
    .ClkMuxCfgCnt = (sizeof(s_moduleClkCfg) / sizeof(Ckgen_ClkMuxCfgType)),
    .ClkDivCfgs = s_clkDivCfg,
    .ClkMuxCfgs = s_moduleClkCfg,
    .SysClkCfgs = s_systemClkCfg,
    .ClkoutCfg = &s_clkOutCfg
};

/* ====================================  Functions declaration  ===================================== */


/* ======================================  Functions define  ======================================== */


/* =============================================  EOF  ============================================== */
