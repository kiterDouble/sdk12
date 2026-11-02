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

/*!
 * @file ac7843x_features.h
 *
 * @brief This file provides chip specific module features.
 *
 */

#ifndef AC7843X_FEATURES_H
#define AC7843X_FEATURES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* =========================================================================================================================== */
/* ================                                            MCM                                            ================ */
/* =========================================================================================================================== */
/*!< Vector table size 16 + 126 */
#define VECTOR_TABLE_SIZE          (142U)
/*!< Vector table sram address, the address area don't be used by IRAM2, user can define */
#define VECTOR_TABLE_SRAM_ADDR     (0x20000000U)

/*!
 * @brief Core interrupt type.
 */
typedef enum
{
    FIOC_IRQ = MCM_MISCR_FIOC_Msk, /*!< FPU invalid operation cumulative exception. */
    FDZC_IRQ = MCM_MISCR_FDZC_Msk, /*!< FPU division by zero cumulative exception.  */
    FOFC_IRQ = MCM_MISCR_FOFC_Msk, /*!< FPU overflow cumulative exception.          */
    FUFC_IRQ = MCM_MISCR_FUFC_Msk, /*!< FPU underflow cumulative exception.         */
    FIXC_IRQ = MCM_MISCR_FIXC_Msk, /*!< FPU inexact cumulative exception.           */
    FIDC_IRQ = MCM_MISCR_FIDC_Msk, /*!< FPU input denormal cumulative exception.    */
} Core_InterruptType;                      //PRQA S 1535 # AMCE_SYS_1535

/*!
 * @brief Bist type.
 */
typedef enum
{
    BIST_ABIST,                       /*!< Analog BIST. */
    BIST_MBIST                        /*!< Memory BIST. */
} Core_BistType;                  //PRQA S 1535 # AMCE_SYS_1535

/*!
 * @brief Bist execute status type.
 */
typedef enum
{
    BIST_NORUN = 0x00U,          /*!< BIST is not run. */
    BIST_OK    = 0x01U,          /*!< BIST is ok.      */
    BIST_BUSY  = 0x02U,          /*!< BIST is busy.    */
    BIST_ERROR = 0x03U,          /*!< BIST is error.   */
} Core_BistStatusType;

/* =========================================================================================================================== */
/* ================                                           CKGEN                                           ================ */
/* =========================================================================================================================== */
/*!< HSI clock frequency(8MHz) */
#define CKGEN_HSI_FREQ             (8000000U)

/*!< VHSI clock frequency(48MHz) */
#define CKGEN_VHSI_FREQ            (48000000U)

/*!< Auto change system clock when XOSC is detected loss or SPLL is detected unlock;
     This function needs to enable the corresponding interrupt and disable trigger reset */
#define CKGEN_AUTO_CHANGE_CLK      (0U)

#define CKGEN_TIMEOUT_READY_VALUE         (50000U)

/* timeout value to wait for clock source to stabilize */
#define CKGEN_STABILIZATION_TIMEOUT        (100000U)

/* div1 and div2 clock div maximum value. */
#define CKGEN_SOURCE_CLK_DIV_MAX            (64U)

/* div1 and div2 clock div minimum value. */
#define CKGEN_SOURCE_CLK_DIV_MIN            (1U)

/* bus and sys clock div maximum value. */
#define CKGEN_BUS_SYSCLK_DIV_MAX            (16U)

/* bus and sys clock div minimum value. */
#define CKGEN_BUS_SYSCLK_DIV_MIN            (1U)

/* pct clock div maximum value. */
#define CKGEN_PCT_CLK_DIV_MAX               (16U)

/* pct clock div minimum value. */
#define CKGEN_PCT_CLK_DIV_MIN               (1U)

/* can_ts and clkout clock div maximum value. */
#define CKGEN_CAN_TS_CLK_DIV_MAX            (8U)

/* can_ts and clkout clock div minimum value. */
#define CKGEN_CAN_TS_CLK_DIV_MIN            (1U)

/* can clock div maximum value. */
#define CKGEN_CAN_CLK_DIV_MAX               (2U)

/* can clock div minimum value. */
#define CKGEN_CAN_CLK_DIV_MIN               (1U)

/* spll posdiv maximum value. */
#define CKGEN_SPLL_POSDIV_MAX               (62U)

/* spll posdiv minimum value. */
#define CKGEN_SPLL_POSDIV_MIN               (1U)

/* spll fbkdiv maximum value. */
#define CKGEN_SPLL_FBKDIV_MAX               (255U)

/* spll fbkdiv minimum value. */
#define CKGEN_SPLL_FBKDIV_MIN               (5U)

/* spll vso clock maximum frequency. */
#define CKGEN_SPLL_VCO_FREQ_MAX              (1200000000U)

/* spll vco clock minimum frequency. */
#define CKGEN_SPLL_VCO_FREQ_MIN              (400000000U)

/* spll reference clock maximum frequency. */
#define CKGEN_SPLL_IN_CLK_FREQ_MAX           (48000000U)

/* spll input clock minimum frequency. */
#define CKGEN_SPLL_IN_CLK_FREQ_MIN           (4000000U)

/* sys clock maximum frequency. */
#define CKGEN_SYS_CLK_FREQ_MAX               (180000000U)

/* spll reference clock maximum frequency. */
#define CKGEN_SPLL_REF_FREQ_MAX              (18000000U)

/* bus clock maximum frequency. */
#define CKGEN_BUS_CLK_FREQ_MAX               (90000000U)

/* LSI 128K fixed clock frequency. */
#define LSI_128K_FREQUENCY                   (128000U)

/* LSI 32K fixed clock frequency. */
#define LSI_32K_FREQUENCY                    (32000U)

/* LSI 1K fixed clock frequency. */
#define LSI_1K_FREQUENCY                     (1000U)

/* error value indicates that it is no division or mux register */
#define CKGEN_INVALID_INDEX_VALUE              (0XFFU)

/* register value indicates that it is sysclk mux is HSI in vlpr mode */
#define CKGEN_VLPR_SYSCLK_HSI_REG_VAL        (2U)

/* register value indicates that it is PLL input is HSI */
#define CKGEN_PLL_IN_HSI                     (0U)

/* register value indicates that it is PLL input is HSE */
#define CKGEN_PLL_IN_HSE                     (1U)

#define CKGEN_ENABLE_CHECK_PARAM             (0U)

#define  CKGEN_PERIPH_CLK_MUX0_ADDR_OFFSET    (0x34U)
#define  CKGEN_PERIPH_CLK_MUX0_INDEX_OFFSET   (0U)

#define  CKGEN_PERIPH_CLK_MUX1_ADDR_OFFSET    (0x38)
#define  CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET   (32U)

#define  CKGEN_PERIPH_CLK_MUX2_ADDR_OFFSET    (0x3C)
#define  CKGEN_PERIPH_CLK_MUX2_INDEX_OFFSET   (64U)

#define  CKGEN_PERIPH_CLK_MUX3_ADDR_OFFSET    (0x40)
#define  CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET   (96U)

#define  CKGEN_PERIPH_CLK_MUX4_ADDR_OFFSET    (0x44)
#define  CKGEN_PERIPH_CLK_MUX4_INDEX_OFFSET   (128U)

#define  CKGEN_PERIPH_CLK_MUX5_ADDR_OFFSET    (0x48)
#define  CKGEN_PERIPH_CLK_MUX5_INDEX_OFFSET   (160U)

#define  CKGEN_PERIPH_CLK_MUX6_ADDR_OFFSET    (0x4C)
#define  CKGEN_PERIPH_CLK_MUX6_INDEX_OFFSET   (192U)

#define  CKGEN_PERIPH_CLK_DIV_ADDR_OFFSET     (0x54)
#define  CKGEN_PERIPH_CLK_DIV_INDEX_OFFSET    (224U)

#define  CKGEN_CLK_OUT_CFG_ADDR_OFFSET        (0x50)
#define  CKGEN_CLK_OUT_CFG_INDEX_OFFSET       (256U)

#define  CKGEN_CLK_DIV1_ADDR_OFFSET           (0x2C)
#define  CKGEN_CLK_DIV1_INDEX_OFFSET          (288U)

#define  CKGEN_CLK_DIV2_ADDR_OFFSET           (0x30)
#define  CKGEN_CLK_DIV2_INDEX_OFFSET          (320U)

/*!
 * @brief configure system clock type mode
 */
typedef enum {
    CKGEN_SYS_CLK_MODE_RUN = 0x0U,
    CKGEN_SYS_CLK_MODE_VLPR,
    CKGEN_SYS_CLK_MODE_MAX,
} Ckgen_SysClkModeType;

/*!
 * @brief PLL prediv value
 */
typedef enum {
    CKGEN_PLL_PRE_DIV_1U = 1U,
    CKGEN_PLL_PRE_DIV_2U = 2U,
    CKGEN_PLL_PRE_DIV_3U = 3U,
    CKGEN_PLL_PRE_DIV_4U = 4U,
    CKGEN_PLL_PRE_DIV_5U = 5U,
    CKGEN_PLL_PRE_DIV_6U = 6U,
    CKGEN_PLL_PRE_DIV_7U = 7U,
} Ckgen_PllPreDivType;

/*!
 * @brief Define the enum of clock output divider.
 */
typedef enum
{
    CKGEN_CLKOUT_DIV_BY_1 = 0U,        /*!< Divided by 1 */
    CKGEN_CLKOUT_DIV_BY_2 = 1U,        /*!< Divided by 2 */
    CKGEN_CLKOUT_DIV_BY_4 = 2U,        /*!< Divided by 4 */
    CKGEN_CLKOUT_DIV_BY_6 = 3U,        /*!< Divided by 6 */
    CKGEN_CLKOUT_DIV_BY_8 = 4U,        /*!< Divided by 8 */
    CKGEN_CLKOUT_DIV_BY_10 = 5U,       /*!< Divided by 10 */
    CKGEN_CLKOUT_DIV_BY_12 = 6U,       /*!< Divided by 12 */
    CKGEN_CLKOUT_DIV_BY_14 = 7U        /*!< Divided by 14 */
} Ckgen_ClkoutDivType;

/*!
 * @brief PLL posdiv value
 */
typedef enum {
    CKGEN_PLL_POS_DIV_1U = 1U,
    CKGEN_PLL_POS_DIV_2U = 2U,
    CKGEN_PLL_POS_DIV_4U = 4U,
    CKGEN_PLL_POS_DIV_6U = 6U,
    CKGEN_PLL_POS_DIV_8U = 8U,
    CKGEN_PLL_POS_DIV_10U = 10U,
    CKGEN_PLL_POS_DIV_12U = 12U,
    CKGEN_PLL_POS_DIV_14U = 14U,
    CKGEN_PLL_POS_DIV_16U = 16U,
    CKGEN_PLL_POS_DIV_18U = 18U,
    CKGEN_PLL_POS_DIV_20U = 20U,
    CKGEN_PLL_POS_DIV_22U = 22U,
    CKGEN_PLL_POS_DIV_24U = 24U,
    CKGEN_PLL_POS_DIV_26U = 26U,
    CKGEN_PLL_POS_DIV_28U = 28U,
    CKGEN_PLL_POS_DIV_30U = 30U,
    CKGEN_PLL_POS_DIV_32U = 32U,
    CKGEN_PLL_POS_DIV_34U = 34U,
    CKGEN_PLL_POS_DIV_36U = 36U,
    CKGEN_PLL_POS_DIV_38U = 38U,
    CKGEN_PLL_POS_DIV_40U = 40U,
    CKGEN_PLL_POS_DIV_42U = 42U,
    CKGEN_PLL_POS_DIV_44U = 24U,
    CKGEN_PLL_POS_DIV_46U = 46U,
    CKGEN_PLL_POS_DIV_48U = 48U,
    CKGEN_PLL_POS_DIV_50U = 50U,
    CKGEN_PLL_POS_DIV_52U = 52U,
    CKGEN_PLL_POS_DIV_54U = 54U,
    CKGEN_PLL_POS_DIV_56U = 56U,
    CKGEN_PLL_POS_DIV_58U = 58U,
    CKGEN_PLL_POS_DIV_60U = 60U,
    CKGEN_PLL_POS_DIV_62U = 62U,
} Ckgen_PllPosDivType;

/*!
 * @brief pll lock detect delay select
 */
typedef enum {
    CKGEN_PLL_LD_DLY_SEL_64CYCLE = 0x0U,
    CKGEN_PLL_LD_DLY_SEL_128CYCLE,
    CKGEN_PLL_LD_DLY_SEL_256CYCLE,
    CKGEN_PLL_LD_DLY_SEL_1024CYCLE,
} Ckgen_PllLdDlySelType;

 /**
 * example: CKGEN_TIMER_CLK  equal to (0x00FF09FFU)
 * bit(0-7) is enable idx  bit(8-15) is mux idx  bit(16-23) is div idx  bit(24-31) is clk idx
 *  FF                          09                      FF                           00
 */
typedef enum {
    /************          PERI_CLK_MUX0          ************/
    CKGEN_TIMER_CLK         = CKGEN_PERIPH_CLK_MUX0_INDEX_OFFSET + CKGEN_PERI_CLK_MUX0_TIMER_MUX_Pos,
    CKGEN_PCT_CLK           = CKGEN_PERIPH_CLK_MUX0_INDEX_OFFSET + CKGEN_PERI_CLK_MUX0_PCT_MUX_Pos,
    CKGEN_EIO_CLK           = CKGEN_PERIPH_CLK_MUX0_INDEX_OFFSET + CKGEN_PERI_CLK_MUX0_EIO_MUX_Pos,
    /************          PERI_CLK_MUX1          ************/
    CKGEN_I2C0_CLK          = CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET + CKGEN_PERI_CLK_MUX1_I2C0_MUX_Pos,
    CKGEN_I2C1_CLK          = CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET + CKGEN_PERI_CLK_MUX1_I2C1_MUX_Pos,
    CKGEN_I2C2_CLK          = CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET + CKGEN_PERI_CLK_MUX1_I2C2_MUX_Pos,
    CKGEN_ADC0_CLK          = CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET + CKGEN_PERI_CLK_MUX1_ADC0_MUX_Pos,
    CKGEN_ADC1_CLK          = CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET + CKGEN_PERI_CLK_MUX1_ADC1_MUX_Pos,
    /************          PERI_CLK_MUX2          ************/
    CKGEN_SPI0_CLK          = CKGEN_PERIPH_CLK_MUX2_INDEX_OFFSET + CKGEN_PERI_CLK_MUX2_SPI0_MUX_Pos,
    CKGEN_SPI1_CLK          = CKGEN_PERIPH_CLK_MUX2_INDEX_OFFSET + CKGEN_PERI_CLK_MUX2_SPI1_MUX_Pos,
    CKGEN_SPI2_CLK          = CKGEN_PERIPH_CLK_MUX2_INDEX_OFFSET + CKGEN_PERI_CLK_MUX2_SPI2_MUX_Pos,
    CKGEN_SPI3_CLK          = CKGEN_PERIPH_CLK_MUX2_INDEX_OFFSET + CKGEN_PERI_CLK_MUX2_SPI3_MUX_Pos,
    CKGEN_SPI4_CLK          = CKGEN_PERIPH_CLK_MUX2_INDEX_OFFSET + CKGEN_PERI_CLK_MUX2_SPI4_MUX_Pos,
    /************          PERI_CLK_MUX3          ************/
    CKGEN_CAN0_CLK          = CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET + CKGEN_PERI_CLK_MUX3_CAN0_MUX_Pos,
    CKGEN_CAN1_CLK          = CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET + CKGEN_PERI_CLK_MUX3_CAN1_MUX_Pos,
    CKGEN_CAN2_CLK          = CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET + CKGEN_PERI_CLK_MUX3_CAN2_MUX_Pos,
    CKGEN_CAN3_CLK          = CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET + CKGEN_PERI_CLK_MUX3_CAN3_MUX_Pos,
    CKGEN_CAN4_CLK          = CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET + CKGEN_PERI_CLK_MUX3_CAN4_MUX_Pos,
    CKGEN_CAN5_CLK          = CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET + CKGEN_PERI_CLK_MUX3_CAN5_MUX_Pos,
    /************          PERI_CLK_MUX4          ************/
    CKGEN_UART0_CLK         = CKGEN_PERIPH_CLK_MUX4_INDEX_OFFSET + CKGEN_PERI_CLK_MUX4_UART0_MUX_Pos,
    CKGEN_UART1_CLK         = CKGEN_PERIPH_CLK_MUX4_INDEX_OFFSET + CKGEN_PERI_CLK_MUX4_UART1_MUX_Pos,
    CKGEN_UART2_CLK         = CKGEN_PERIPH_CLK_MUX4_INDEX_OFFSET + CKGEN_PERI_CLK_MUX4_UART2_MUX_Pos,
    CKGEN_UART3_CLK         = CKGEN_PERIPH_CLK_MUX4_INDEX_OFFSET + CKGEN_PERI_CLK_MUX4_UART3_MUX_Pos,
    CKGEN_UART4_CLK         = CKGEN_PERIPH_CLK_MUX4_INDEX_OFFSET + CKGEN_PERI_CLK_MUX4_UART4_MUX_Pos,
    CKGEN_UART5_CLK         = CKGEN_PERIPH_CLK_MUX4_INDEX_OFFSET + CKGEN_PERI_CLK_MUX4_UART5_MUX_Pos,
    CKGEN_UART6_CLK         = CKGEN_PERIPH_CLK_MUX4_INDEX_OFFSET + CKGEN_PERI_CLK_MUX4_UART6_MUX_Pos,
    CKGEN_UART7_CLK         = CKGEN_PERIPH_CLK_MUX4_INDEX_OFFSET + CKGEN_PERI_CLK_MUX4_UART7_MUX_Pos,
    /************          PERI_CLK_MUX5          ************/
    CKGEN_PWM0_CLK          = CKGEN_PERIPH_CLK_MUX5_INDEX_OFFSET + CKGEN_PERI_CLK_MUX5_PWM0_EXT_Pos,
    CKGEN_PWM1_CLK          = CKGEN_PERIPH_CLK_MUX5_INDEX_OFFSET + CKGEN_PERI_CLK_MUX5_PWM1_EXT_Pos,
    CKGEN_PWM2_CLK          = CKGEN_PERIPH_CLK_MUX5_INDEX_OFFSET + CKGEN_PERI_CLK_MUX5_PWM2_EXT_Pos,
    CKGEN_PWM3_CLK          = CKGEN_PERIPH_CLK_MUX5_INDEX_OFFSET + CKGEN_PERI_CLK_MUX5_PWM3_EXT_Pos,
    CKGEN_PWM4_CLK          = CKGEN_PERIPH_CLK_MUX5_INDEX_OFFSET + CKGEN_PERI_CLK_MUX5_PWM4_EXT_Pos,
    CKGEN_PWM5_CLK          = CKGEN_PERIPH_CLK_MUX5_INDEX_OFFSET + CKGEN_PERI_CLK_MUX5_PWM5_EXT_Pos,
    /************          PERI_CLK_MUX6          ************/
    CKGEN_PWM6_CLK          = CKGEN_PERIPH_CLK_MUX6_INDEX_OFFSET + CKGEN_PERI_CLK_MUX6_PWM6_EXT_Pos,
    CKGEN_PWM7_CLK          = CKGEN_PERIPH_CLK_MUX6_INDEX_OFFSET + CKGEN_PERI_CLK_MUX6_PWM7_EXT_Pos,
    /************          PERI_CLK_DIV          ************/
    CKGEN_TPIU_CLK          = CKGEN_PERIPH_CLK_DIV_INDEX_OFFSET + CKGEN_PERI_CLK_DIV_TPIU_DIV_Pos,
    /************          CLK_OUT_CFG          ************/
    CKGEN_CLK_OUT           = CKGEN_CLK_OUT_CFG_INDEX_OFFSET + CKGEN_CLK_OUT_CFG_MUX1_Pos,
    /************          CLK_DIV1          ************/
    CKGEN_SPLL_DIV1_CLK     = CKGEN_CLK_DIV1_INDEX_OFFSET + CKGEN_CLK_DIV1_SPLL_DIV1_Pos,
    CKGEN_VHSI_DIV1_CLK     = CKGEN_CLK_DIV1_INDEX_OFFSET + CKGEN_CLK_DIV1_VHSI_DIV1_Pos,
    CKGEN_HSI_DIV1_CLK      = CKGEN_CLK_DIV1_INDEX_OFFSET + CKGEN_CLK_DIV1_HSI_DIV1_Pos,
    CKGEN_HSE_DIV1_CLK      = CKGEN_CLK_DIV1_INDEX_OFFSET + CKGEN_CLK_DIV1_HSE_DIV1_Pos,
    /************          CLK_DIV2          ************/
    CKGEN_SPLL_DIV2_CLK     = CKGEN_CLK_DIV2_INDEX_OFFSET + CKGEN_CLK_DIV2_SPLL_DIV2_Pos,
    CKGEN_VHSI_DIV2_CLK     = CKGEN_CLK_DIV2_INDEX_OFFSET + CKGEN_CLK_DIV2_VHSI_DIV2_Pos,
    CKGEN_HSI_DIV2_CLK      = CKGEN_CLK_DIV2_INDEX_OFFSET + CKGEN_CLK_DIV2_HSI_DIV2_Pos,
    CKGEN_HSE_DIV2_CLK      = CKGEN_CLK_DIV2_INDEX_OFFSET + CKGEN_CLK_DIV2_HSE_DIV2_Pos,
    CKGEN_ADC_SPLLDIV_CLK   = CKGEN_CLK_DIV2_INDEX_OFFSET + CKGEN_CLK_DIV2_ADC_SPLLDIV_Pos,
    CKGEN_LSI_CLK,
    CKGEN_SYS_CLK,
    CKGEN_BUS_CLK,
    CKGEN_CORE_CLK,
    CKGEN_FLASH_CLK,
    CKGEN_HSE_CLK,
    CKGEN_HSI_CLK,
    CKGEN_VHSI_CLK,
    CKGEN_SPLL_CLK,
    CKGEN_HSI_VLPS_CLK,
    CKGEN_LSI_1K_CLK,
    CKGEN_LSI_32K_CLK,
    CKGEN_LSI_128K_CLK,
    CKGEN_RTC_CLKIN,
    CKGEN_PWM_EXT_CLK0,
    CKGEN_PWM_EXT_CLK1,
    CKGEN_PWM_EXT_CLK2,
    CKGEN_OFF_CLK,
} Ckgen_ClkIdType;

typedef enum {
    /************          PERI_CLK_EN0          ************/
    CKGEN_UART0_BUS_CLK     = CKGEN_PERI_CLK_EN0_UART0_EN_Pos,
    CKGEN_UART1_BUS_CLK     = CKGEN_PERI_CLK_EN0_UART1_EN_Pos,
    CKGEN_UART2_BUS_CLK     = CKGEN_PERI_CLK_EN0_UART2_EN_Pos,
    CKGEN_UART3_BUS_CLK     = CKGEN_PERI_CLK_EN0_UART3_EN_Pos,
    CKGEN_UART4_BUS_CLK     = CKGEN_PERI_CLK_EN0_UART4_EN_Pos,
    CKGEN_UART5_BUS_CLK     = CKGEN_PERI_CLK_EN0_UART5_EN_Pos,
    CKGEN_UART6_BUS_CLK     = CKGEN_PERI_CLK_EN0_UART6_EN_Pos,
    CKGEN_UART7_BUS_CLK     = CKGEN_PERI_CLK_EN0_UART7_EN_Pos,
    CKGEN_SPI0_BUS_CLK      = CKGEN_PERI_CLK_EN0_SPI0_EN_Pos,
    CKGEN_SPI1_BUS_CLK      = CKGEN_PERI_CLK_EN0_SPI1_EN_Pos,
    CKGEN_SPI2_BUS_CLK      = CKGEN_PERI_CLK_EN0_SPI2_EN_Pos,
    CKGEN_SPI3_BUS_CLK      = CKGEN_PERI_CLK_EN0_SPI3_EN_Pos,
    CKGEN_SPI4_BUS_CLK      = CKGEN_PERI_CLK_EN0_SPI4_EN_Pos,
    CKGEN_I2C0_BUS_CLK      = CKGEN_PERI_CLK_EN0_I2C0_EN_Pos,
    CKGEN_I2C1_BUS_CLK      = CKGEN_PERI_CLK_EN0_I2C1_EN_Pos,
    CKGEN_I2C2_BUS_CLK      = CKGEN_PERI_CLK_EN0_I2C2_EN_Pos,
    CKGEN_PCT_BUS_CLK       = CKGEN_PERI_CLK_EN0_PCT_EN_Pos,
    CKGEN_PWM0_BUS_CLK      = CKGEN_PERI_CLK_EN0_PWM0_EN_Pos,
    CKGEN_PWM1_BUS_CLK      = CKGEN_PERI_CLK_EN0_PWM1_EN_Pos,
    CKGEN_PWM2_BUS_CLK      = CKGEN_PERI_CLK_EN0_PWM2_EN_Pos,
    CKGEN_PWM3_BUS_CLK      = CKGEN_PERI_CLK_EN0_PWM3_EN_Pos,
    CKGEN_PWM4_BUS_CLK      = CKGEN_PERI_CLK_EN0_PWM4_EN_Pos,
    CKGEN_PWM5_BUS_CLK      = CKGEN_PERI_CLK_EN0_PWM5_EN_Pos,
    CKGEN_PWM6_BUS_CLK      = CKGEN_PERI_CLK_EN0_PWM6_EN_Pos,
    CKGEN_PWM7_BUS_CLK      = CKGEN_PERI_CLK_EN0_PWM7_EN_Pos,
    /************          PERI_CLK_EN1          ************/
    CKGEN_RTC_BUS_CLK       = 32U + CKGEN_PERI_CLK_EN1_RTC_EN_Pos,
    CKGEN_DMA_BUS_CLK       = 32U + CKGEN_PERI_CLK_EN1_DMA_EN_Pos,
    CKGEN_GPIO_BUS_CLK      = 32U + CKGEN_PERI_CLK_EN1_GPIO_EN_Pos,
    CKGEN_WDG_BUS_CLK       = 32U + CKGEN_PERI_CLK_EN1_WDG_EN_Pos,
    CKGEN_EWDG_BUS_CLK      = 32U + CKGEN_PERI_CLK_EN1_EWDG_EN_Pos,
    CKGEN_CRC_BUS_CLK       = 32U + CKGEN_PERI_CLK_EN1_CRC_EN_Pos,
    CKGEN_CAN0_BUS_CLK      = 32U + CKGEN_PERI_CLK_EN1_CAN0_EN_Pos,
    CKGEN_CAN1_BUS_CLK      = 32U + CKGEN_PERI_CLK_EN1_CAN1_EN_Pos,
    CKGEN_CAN2_BUS_CLK      = 32U + CKGEN_PERI_CLK_EN1_CAN2_EN_Pos,
    CKGEN_CAN3_BUS_CLK      = 32U + CKGEN_PERI_CLK_EN1_CAN3_EN_Pos,
    CKGEN_CAN4_BUS_CLK      = 32U + CKGEN_PERI_CLK_EN1_CAN4_EN_Pos,
    CKGEN_CAN5_BUS_CLK      = 32U + CKGEN_PERI_CLK_EN1_CAN5_EN_Pos,
    CKGEN_CAN_CTRL_BUS_CLK  = 32U + CKGEN_PERI_CLK_EN1_CAN_CTRL_EN_Pos,
    /************          PERI_CLK_EN2          ************/
    CKGEN_CTU_BUS_CLK       = 64U + CKGEN_PERI_CLK_EN2_CTU_EN_Pos,
    CKGEN_ACMP0_BUS_CLK     = 64U + CKGEN_PERI_CLK_EN2_ACMP0_EN_Pos,
    CKGEN_ACMP1_BUS_CLK     = 64U + CKGEN_PERI_CLK_EN2_ACMP1_EN_Pos,
    CKGEN_PDT0_BUS_CLK      = 64U + CKGEN_PERI_CLK_EN2_PDT0_EN_Pos,
    CKGEN_PDT1_BUS_CLK      = 64U + CKGEN_PERI_CLK_EN2_PDT1_EN_Pos,
    CKGEN_ADC0_BUS_CLK      = 64U + CKGEN_PERI_CLK_EN2_ADC0_EN_Pos,
    CKGEN_ADC1_BUS_CLK      = 64U + CKGEN_PERI_CLK_EN2_ADC1_EN_Pos,
    CKGEN_TIMER_BUS_CLK     = 64U + CKGEN_PERI_CLK_EN2_TIMER_EN_Pos,
    CKGEN_EIO_BUS_CLK       = 64U + CKGEN_PERI_CLK_EN2_EIO_EN_Pos,
    CKGEN_SENT_BUS_CLK      = 64U + CKGEN_PERI_CLK_EN2_SENT_EN_Pos,
    CKGEN_SMU_BUS_CLK       = 64U + CKGEN_PERI_CLK_EN2_SMU_EN_Pos,
    CKGEN_SMU_HSM_BUS_CLK   = 64U + CKGEN_PERI_CLK_EN2_SMU_HSM_EN_Pos,
} Ckgen_BusClkIdType;

/* =========================================================================================================================== */
/* ================                                            MPU                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the MPU module */
#define MPU_INSTANCE_MAX           (2U)
/** @brief user instruction error*/
#define MPU_ERR_USER_INSTRUCTION   1U
/** @brief user data error*/
#define MPU_ERR_USER_DATA          2U
/** @brief super instruction error*/
#define MPU_ERR_SUPER_INSTRUCTION  4U
/** @brief super data error*/
#define MPU_ERR_SUPER_DATA         8U

/** @brief generate attribution order by user and super permission*/
#define MPU_PERMISSION_VALUE(MasterId, UserPermission, SuperPermission) \
    (((uint32)(UserPermission) << ((uint32)(MasterId) * 6U)) |          \
    ((uint32)(SuperPermission) << (((uint32)(MasterId) * 6U) + 3U)))

/*!< Has process identifier support */
#define MPU_HAS_PROCESS_IDENTIFIER (1U)

/*!< Region disable master pid */
#define MPU_MASTER_PID_DISABLE    0U

/*!< Region enable master hsm pid  */
#define MPU_MASTER_PID_EN_HSM        MPU_RGD0_WORD2_M3PE_Msk

/*!< Region enable master debugger pid  */
#define MPU_MASTER_PID_EN_DEBUGGER    MPU_RGD0_WORD2_M1PE_Msk

/*!< Region enable master debugger pid  */
#define MPU_MASTER_PID_EN_CORE        MPU_RGD0_WORD2_M0PE_Msk

/*!< Array of mpu base addresses */
#define MPU_BASE_PTRS              {MPU0, MPU1}

typedef enum
{
    MPU_ID_0 = 0U, /*!< mpu 0 */
    MPU_ID_1 = 1U, /*!< mpu 1 */
    MPU_ID_MAX
} Mpu_IdType;

typedef enum
{
    MPU_MASTER_CORE     = 0U, /*!< The MPU Logical Bus Master Number for core bus master */
    MPU_MASTER_DEBUGGER = 1U, /*!< The MPU Logical Bus Master Number for Debugger master */
    MPU_MASTER_DMA      = 2U, /*!< The MPU Logical Bus Master Number for DMA master */
    MPU_MASTER_HSM      = 3U, /*!< The MPU Logical Bus Master Number for HSM master */
    MPU_MASTER_MAX
} Mpu_MasterType;

typedef enum
{
    MPU_SLAVE_SRAM_L    = 1U,
    MPU_SLAVE_SRAM_U    = 2U,
    MPU_SLAVE_PFLASH    = 3U,
    MPU_SLAVE_DFLASH    = 4U,
    MPU_SLAVE_MAX
} Mpu_SlaveType;

typedef enum
{
    MPU_REGION_ID_0     = 0U,
    MPU_REGION_ID_1     = 1U,
    MPU_REGION_ID_2     = 2U,
    MPU_REGION_ID_3     = 3U,
    MPU_REGION_ID_4     = 4U,
    MPU_REGION_ID_5     = 5U,
    MPU_REGION_ID_6     = 6U,
    MPU_REGION_ID_7     = 7U,
    MPU_REGION_ID_MAX
} Mpu_RegionIdType;

/*!
 * @brief MPU attribute enumeration.
 */
typedef enum
{
    MPU_ATTR_NONE = 0U,     /*!< None permission */
    MPU_ATTR_X = 1U,        /*!< Executable permission */
    MPU_ATTR_R = 2U,        /*!< Readable permission */
    MPU_ATTR_RX = 3U,       /*!< Readable and Executable permission */
    MPU_ATTR_W = 4U,        /*!< Writeable permission */
    MPU_ATTR_WX = 5U,       /*!< Writeable and Executable permission */
    MPU_ATTR_RW = 6U,       /*!< Readable and Writeable permission */
    MPU_ATTR_RWX = 7U       /*!< Readable, Writeable and Writeable permission  */
} Mpu_PermissionType;                //PRQA S 1535 # AMCE_SYS_1535

/* =========================================================================================================================== */
/* ================                                            PBR                                            ================ */
/* =========================================================================================================================== */
/*!
 * @brief pbr master id
 */
typedef enum
{
    PBR_MASTER_CORE = 0x0U,
    PBR_MASTER_DEBUGGER,
    PBR_MASTER_DMA,
    PBR_MASTER_HSM,
    PBR_MASTER_MAX
} Pbr_MasterIdType;

/*!
 * @brief pbr peripheral id
 */
typedef enum
{
    PBR_PERIPH_PWM0 = 0U,/*PACRA*/
    PBR_PERIPH_PWM1,
    PBR_PERIPH_PWM2,
    PBR_PERIPH_PWM3,
    PBR_PERIPH_PWM4,
    PBR_PERIPH_PWM5,
    PBR_PERIPH_PWM6,
    PBR_PERIPH_PWM7,
    PBR_PERIPH_CRC = 8U,/*PACRA*/
    PBR_PERIPH_GPIO,
    PBR_PERIPH_PDT0,
    PBR_PERIPH_PDT1,
    PBR_PERIPH_MPU,
    PBR_PERIPH_EIM,
    PBR_PERIPH_SMU,
    PBR_PERIPH_CKGEN = 32U,/*PACRC*/
    PBR_PERIPH_FLASH = 34U,
    PBR_PERIPH_ADC0,
    PBR_PERIPH_ADC1,
    PBR_PERIPH_ACMP0,
    PBR_PERIPH_CTU,
    PBR_PERIPH_SPM = 45U,/*PACRD 40 + 5*/
    PBR_PERIPH_RTC,
    PBR_PERIPH_EIO,
    PBR_PERIPH_WDG = 48U,/*PACRE*/
    PBR_PERIPH_EWDG,
    PBR_PERIPH_SPI0,
    PBR_PERIPH_SPI1,
    PBR_PERIPH_SPI2 = 52U,
    PBR_PERIPH_I2C0 = 54U,
    PBR_PERIPH_ACMP1 = 56U,/*PACRF*/
    PBR_PERIPH_SPI3,
    PBR_PERIPH_SPI4,
    PBR_PERIPH_I2C1,
    PBR_PERIPH_I2C2,
    PBR_PERIPH_TIMER = 64U,/*PACRG*/
    PBR_PERIPH_DMA,
    PBR_PERIPH_UART0,
    PBR_PERIPH_UART1,
    PBR_PERIPH_UART2,
    PBR_PERIPH_UART3,
    PBR_PERIPH_PCT = 72U,/*PACRH*/
    PBR_PERIPH_CMU = 73U,
    PBR_PERIPH_MRAM0 = 80U,/*PACRI*/
    PBR_PERIPH_MRAM1,
    PBR_PERIPH_UART4,
    PBR_PERIPH_UART5,
    PBR_PERIPH_UART6,
    PBR_PERIPH_UART7,
    PBR_PERIPH_CAN0,
    PBR_PERIPH_CAN1,
    PBR_PERIPH_CAN2 = 88U,/*PACRJ*/
    PBR_PERIPH_CAN3,
    PBR_PERIPH_CAN4,
    PBR_PERIPH_CAN5,
    PBR_PERIPH_CAN_CTRL,
    PBR_PERIPH_SENT,
    PBR_PERIPH_MAX
} Pbr_PeriphIdType;
/* =========================================================================================================================== */
/* ================                                            CMU                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the CMU module */
#define CMU_INSTANCE_MAX           (4U)
/*!< Array of CMU base addresses */
#define CMU_BASE_PTRS              {CMU_VHSI, CMU_HSE, CMU_PLL, CMU_HSI}
/* =========================================================================================================================== */
/* ================                                            RCM                                            ================ */
/* =========================================================================================================================== */
typedef enum
{
    RCM_RESET_ID_UART0 = 0U, /*!< Soft reset UART0                  */
    RCM_RESET_ID_UART1 = 1U, /*!< Soft reset UART1                  */
    RCM_RESET_ID_UART2 = 2U, /*!< Soft reset UART2                  */
    RCM_RESET_ID_UART3 = 3U, /*!< Soft reset UART3                  */
    RCM_RESET_ID_UART4 = 4U, /*!< Soft reset Uart4                  */
    RCM_RESET_ID_UART5 = 5U, /*!< Soft reset Uart5                  */
    RCM_RESET_ID_UART6 = 6U, /*!< Soft reset Uart6                  */
    RCM_RESET_ID_UART7 = 7U, /*!< Soft reset Uart7                  */
    RCM_RESET_ID_SPI0 = 8U, /*!< Soft reset SPI0                    */
    RCM_RESET_ID_SPI1 = 9U, /*!< Soft reset SPI1                    */
    RCM_RESET_ID_SPI2 = 10U, /*!< Soft reset SPI2                   */
    RCM_RESET_ID_SPI3 = 11U, /*!< Soft reset SPI3                   */
    RCM_RESET_ID_SPI4 = 12U, /*!< Soft reset SPI4                   */
    RCM_RESET_ID_I2C0 = 16U, /*!< Soft reset I2C0                   */
    RCM_RESET_ID_I2C1 = 17U, /*!< Soft reset I2C1                   */
    RCM_RESET_ID_I2C2 = 18U, /*!< Soft reset I2C2                   */
    RCM_RESET_ID_PCT = 20U, /*!< Soft reset PCT                     */
    RCM_RESET_ID_PWM0 = 24U, /*!< Soft reset PWM0                   */
    RCM_RESET_ID_PWM1 = 25U, /*!< Soft reset PWM1                   */
    RCM_RESET_ID_PWM2 = 26U, /*!< Soft reset PWM2                   */
    RCM_RESET_ID_PWM3 = 27U, /*!< Soft reset PWM3                   */
    RCM_RESET_ID_PWM4 = 28U, /*!< Soft reset PWM4                   */
    RCM_RESET_ID_PWM5 = 29U, /*!< Soft reset PWM5                   */
    RCM_RESET_ID_PWM6 = 30U, /*!< Soft reset PWM6                   */
    RCM_RESET_ID_PWM7 = 31U, /*!< Soft reset PWM7                   */

    RCM_RESET_ID_DMA = 34U, /*!< Soft reset DMA                     */
    RCM_RESET_ID_GPIO = 37U, /*!< Soft reset GPIO                   */
    RCM_RESET_ID_WDG = 38U, /*!< Soft reset WDG                     */
    RCM_RESET_ID_EWDG = 39U, /*!< Soft reset EWDG                   */
    RCM_RESET_ID_CRC = 40U, /*!< Soft reset CRC                     */
    RCM_RESET_ID_CAN0 = 41U, /*!< Soft reset CAN0                   */
    RCM_RESET_ID_CAN1 = 42U, /*!< Soft reset CAN1                   */
    RCM_RESET_ID_CAN2 = 43U, /*!< Soft reset CAN2                   */
    RCM_RESET_ID_CAN3 = 44U, /*!< Soft reset CAN3                   */
    RCM_RESET_ID_CAN4 = 45U, /*!< Soft reset CAN4                   */
    RCM_RESET_ID_CAN5 = 46U, /*!< Soft reset CAN5                   */
    RCM_RESET_ID_CAN_CTRL = 47U, /*!< Soft reset CAN CTRL           */

    RCM_RESET_ID_CTU = 65U, /*!< Soft reset CTU                     */
    RCM_RESET_ID_ACMP0 = 71U, /*!< Soft reset ACMP                  */
    RCM_RESET_ID_ACMP1 = 72U, /*!< Soft reset ACMP                  */
    RCM_RESET_ID_PDT0 = 73U, /*!< Soft reset PDT0                   */
    RCM_RESET_ID_PDT1 = 74U, /*!< Soft reset PDT1                   */
    RCM_RESET_ID_ADC0 = 75U, /*!< Soft reset ADC0                   */
    RCM_RESET_ID_ADC1 = 76U, /*!< Soft reset ADC1                   */
    RCM_RESET_ID_TIMER = 77U, /*!< Soft reset TIMER                 */
    RCM_RESET_ID_EIO = 78U, /*!< Soft reset EIO                     */
    RCM_RESET_ID_SENT = 79U, /*!< Soft reset SENT                   */
    RCM_RESET_ID_MAX
} Rcm_ResetIDType;
/* =========================================================================================================================== */
/* ================                                            SPM                                            ================ */
/* =========================================================================================================================== */
/*!
 * @brief Power mode
 */
typedef enum
{
    SPM_MODE_RUN = 0U,
    SPM_MODE_VLPR = 1U,
    SPM_MODE_STOP1 = 2U,
    SPM_MODE_STOP2 = 3U,
    SPM_MODE_VLPS = 4U,
    SPM_MODE_STANDBY = 5U,
    SPM_MODE_MAX = 6U
} Spm_PowerModeType;

/*!
 * @brief Enter sleep timeout action
 */
typedef enum
{
    SPM_INTERRUPT = 0U, /*!< sleep timeout triggers interrupt */
    SPM_RESET /*!< sleep timeout mcu will reset */
} Spm_SleepTimeoutActionType;

/*!
 * @brief Enter sleep timeout action
 */
typedef enum
{
    SPM_STANDBY_RESET = 0U, /*!< wakeup mcu reset */
    SPM_STANDBY_CONTINUE /*!< wakeup mcu going done */
} Spm_StandbyActionType;

/*!
 * @brief Low voltage detect threshold type
 */
typedef enum
{
    SPM_LOW = 0U, /*!< Select low voltage gear */
    SPM_HIGH /*!< Select high voltage gear */
} Spm_ThresholdType;

/*!
 * @brief Standby wakeup enable type
 */
typedef enum
{
    SPM_STP_WAKEUP_DISABLE = 0U,
    SPM_STP_WAKEUP_RISING,
    SPM_STP_WAKEUP_FALLING,
    SPM_STP_WAKEUP_DUAL_EDGES
} Spm_StbWakeupEnableType;    //PRQA S 1535 # AMCE_MCU_1535

/* =========================================================================================================================== */
/* ================                                            SMU                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the SMU module */
#define SMU_INSTANCE_COUNT         (1U)
/*!< Array of SMU base addresses */
#define SMU_BASE_PTRS              {SMU}
/*!< Array of SMU IRQs */
#define SMU_IRQS                   {SMU_IRQn}
/*!< Array of SMU ckgen interface clocks */
#define SMU_CKGEN_CLOCKS           {CLK_SMU}
/*!< SMU unlock key */
#define SMU_UNLOCK_FIRST_VALUE     (0xA5U)
#define SMU_UNLOCK_SECOND_VALUE    (0x5AU)
#define SMU_HSM_UNLOCK_FIRST_VALUE (0xC3U)
#define SMU_HSM_UNLOCK_SECOND_VALUE (0x3CU)

/*!< Number of the SMU reset count threshold. */
#define SMU_RESET_THRESHOLD        (15U)

/*!
 * @brief SMU fault type.
 */
typedef enum
{
    SMU_HWSPF,              /*!< Hardware single point fault types. */
    SMU_HWLF,               /*!< Hardware latent fault types. */
    SMU_SWSPF,              /*!< Software single point fault types. */
    SMU_SWLF,               /*!< Software latent fault types. */
    SMU_HSM_HWSPF,          /*!< HSM Hardware single point fault types. */
    SMU_HSM_SWSPF           /*!< HSM Hardware single point fault types. */
} Smu_FaultType;

/*!
 * @brief SMU hardware single point fault type.
 */
typedef enum
{
    SMU_HWSPF_PLL_UNLOCK,    /*!< Analog PLL unlock */
    SMU_HWSPF_CMU_PLL,       /*!< CMU detected System PLL unlock */
    SMU_HWSPF_CMU_HSE,       /*!< CMU detected HSE unlock */
    SMU_HWSPF_CMU_VHSI,      /*!< CMU detected VHSI unlock */
    SMU_HWSPF_SRAM_2BITS,    /*!< SRAM ECC 2 bits error */
    SMU_HWSPF_SRAM_1BIT,     /*!< SRAM ECC 1 bit error */
    SMU_HWSPF_E2E,           /*!< E2E parity error */
    SMU_HWSPF_DMA_ECC,       /*!< DMA parity error */
    SMU_HWSPF_FLASH_ECC,     /*!< Flash ECC error */
    SMU_HWSPF_PDT1,          /*!< PDT1, the number of triggers is inconsisten with the cofigured numher error */
    SMU_HWSPF_PDT0,          /*!< PDT0, the number of triggers is inconsisten with the cofigured numher error */
    SMU_HWSPF_WDG,           /*!< Watchdog timeout */
    SMU_HWSPF_LVD,           /*!< Low voltage detection of VVDD after MTCMOS long chain */
    SMU_HWSPF_DIGLDO,        /*!< Low voltage detection of digital LDO */
    SMU_HWSPF_FLHLDO,        /*!< Low voltage detection of flash LDO */
    SMU_HWSPF_NULL_15,       /*!< Reserved NULL channel */
    SMU_HWSPF_CPU,           /*!< CPU Lock Up */
    SMU_HWSPF_NULL_17,       /*!< Reserved NULL channel */
    SMU_HWSPF_CMU_HSI,       /*!< CMU detected HSI unlock */
    SMU_HWSPF_BOOTROM_1BIT,  /*!< BOOTROM ECC 1 bit error */
    SMU_HWSPF_BOOTROM_2BIT,  /*!< BOOTROM ECC 2 bits error */
    SMU_HWSPF_NULL_21,       /*!< Reserved NULL channel */
    SMU_HWSPF_NULL_22,       /*!< Reserved NULL channel */
    SMU_HWSPF_CPU_DCACHE,    /*!< CPU DCACHE error */
    SMU_HWSPF_CPU_ICACHE,    /*!< CPU ICACHE error */
    SMU_HWSPF_MAX            /*!< Max number of SMU supported hardward single point fault */
} Smu_HwSpfType;               //PRQA S 1535 # AMCE_SYS_1535

/*!
 * @brief SMU hardware latent fault type.
 */
typedef enum
{
    SMU_HWLF_SRAM_EIM2BITS,  /*!< SRAM EIM 2 bits ECC error */
    SMU_HWLF_SRAM_EIM1BIT,   /*!< SRAM EIM 1 bit ECC error */
    SMU_HWLF_FLASH_EIM2BITS, /*!< Flash EIM ECC error */
    SMU_HWLF_ABIST,          /*!< ABIST error */
    SMU_HWLF_MBIST,          /*!< MBIST error */
    SMU_HWLF_BOOTROM_EIM1BIT, /*!< BOOTROM EIM 1 bit ECC error */
    SMU_HWLF_BOOTROM_EIM2BITS, /*!< BOOTROM EIM 2 bits ECC error */
    SMU_HWLF_E2E_EIM2BITS,   /*!< E2E EIM 2 bits ECC error */
    SMU_HWLF_MAX             /*!< Max number of SMU supported hardward latent fault */
} Smu_HwLfType;               //PRQA S 1535 # AMCE_SYS_1535

/*!
 * @brief SMU HSM hardware single point fault type.
 */
typedef enum
{
    SMU_HSM_HWSPF_IROM_1BIT,         /*!< Analog pll unlock */
    SMU_HSM_HWSPF_IRAM_1BIT,         /*!< CMU detected System PLL unlock */
    SMU_HSM_HWSPF_DRAM_1BIT,         /*!< CMU detected HSE unlock */
    SMU_HSM_HWSPF_PKE0_1BIT,         /*!< CMU detected VHSI unlock */
    SMU_HSM_HWSPF_PKE1_1BIT,         /*!< SRAM ECC 1 bit error */
    SMU_HSM_HWSPF_NULL_5,            /*!< Reserved NULL channel */
    SMU_HSM_HWSPF_NULL_6,            /*!< Reserved NULL channel */
    SMU_HSM_HWSPF_IROM_MBIT,         /*!< Analog pll unlock */
    SMU_HSM_HWSPF_IRAM_MBIT,         /*!< CMU detected System PLL unlock */
    SMU_HSM_HWSPF_DRAM_MBIT,         /*!< CMU detected HSE unlock */
    SMU_HSM_HWSPF_PKE0_MBIT,         /*!< CMU detected VHSI unlock */
    SMU_HSM_HWSPF_PKE1_MBIT,         /*!< SRAM ECC 1 bit error */
    SMU_HSM_HWSPF_NULL_12,           /*!< Reserved NULL channel */
    SMU_HSM_HWSPF_NULL_13,           /*!< Reserved NULL channel */
    SMU_HSM_HWSPF_DMA0_PARITY,       /*!< SRAM ECC 1 bit error */
    SMU_HSM_HWSPF_DMA1_PARITY,       /*!< SRAM ECC 1 bit error */
    SMU_HSM_HWSPF_CFG_PARITY,        /*!< SRAM ECC 1 bit error */
    SMU_HSM_HWSPF_NVM_PARITY,        /*!< SRAM ECC 1 bit error */
    SMU_HSM_HWSPF_BUSMATRIX_PARITY,  /*!< SRAM ECC 1 bit error */
    SMU_HSM_HWSPF_OTP_PARITY,        /*!< SRAM ECC 1 bit error */
    SMU_HSM_HWSPF_MAILBOX0_PARITY,   /*!< SRAM ECC 1 bit error */
    SMU_HSM_HWSPF_MAILBOX1_PARITY,   /*!< SRAM ECC 1 bit error */
    SMU_HSM_HWSPF_SOC_PARITY,        /*!< SRAM ECC 1 bit error */
    SMU_HSM_HWSPF_MAX                /*!< Max number of SMU supported hardward single point fault */
} Smu_HwSpfHsmType;                 //PRQA S 1535 # AMCE_SYS_1535

/*!
 * @brief SMU software single point fault type.
 */
typedef enum
{
    SMU_SWSPF_CHANNEL_0,     /*!< Not defined Software single point fault */
    SMU_SWSPF_CHANNEL_1,     /*!< Not defined Software single point fault */
    SMU_SWSPF_CHANNEL_2,     /*!< Not defined Software single point fault */
    SMU_SWSPF_CHANNEL_3,     /*!< Not defined Software single point fault */
    SMU_SWSPF_CHANNEL_4,     /*!< Not defined Software single point fault */
    SMU_SWSPF_CHANNEL_5,     /*!< Not defined Software single point fault */
    SMU_SWSPF_CHANNEL_6,     /*!< Not defined Software single point fault */
    SMU_SWSPF_CHANNEL_7,     /*!< Not defined Software single point fault */
    SMU_SWSPF_MAX
} Smu_SwSpfType;              //PRQA S 1535 # AMCE_SYS_1535

/*!
 * @brief SMU software latent fault type.
 */
typedef enum
{
    SMU_SWLF_CHANNEL_0,      /*!< Not defined Software latent fault */
    SMU_SWLF_CHANNEL_1,      /*!< Not defined Software latent fault */
    SMU_SWLF_CHANNEL_2,      /*!< Not defined Software latent fault */
    SMU_SWLF_CHANNEL_3,      /*!< Not defined Software latent fault */
    SMU_SWLF_CHANNEL_4,      /*!< Not defined Software latent fault */
    SMU_SWLF_CHANNEL_5,      /*!< Not defined Software latent fault */
    SMU_SWLF_CHANNEL_6,      /*!< Not defined Software latent fault */
    SMU_SWLF_CHANNEL_7,      /*!< Not defined Software latent fault */
    SMU_SWLF_MAX
} Smu_SwLfType;              //PRQA S 1535 # AMCE_SYS_1535

/*!
 * @brief SMU HSM software single point fault type.
 */
typedef enum
{
    SMU_HSM_SWSPF_CHANNEL_0, /*!< Not defined HSM Software single point fault */
    SMU_HSM_SWSPF_CHANNEL_1, /*!< Not defined HSM Software single point fault */
    SMU_HSM_SWSPF_CHANNEL_2, /*!< Not defined HSM Software single point fault */
    SMU_HSM_SWSPF_CHANNEL_3, /*!< Not defined HSM Software single point fault */
    SMU_HSM_SWSPF_CHANNEL_4, /*!< Not defined HSM Software single point fault */
    SMU_HSM_SWSPF_CHANNEL_5, /*!< Not defined HSM Software single point fault */
    SMU_HSM_SWSPF_CHANNEL_6, /*!< Not defined HSM Software single point fault */
    SMU_HSM_SWSPF_CHANNEL_7, /*!< Not defined HSM Software single point fault */
    SMU_HSM_SWSPF_MAX
} Smu_Hsm_SwSpfType;                   //PRQA S 1535 # AMCE_SYS_1535
/* =========================================================================================================================== */
/* ================                                            ESM                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the ESM module */
#define ESM_INSTANCE_COUNT        (1U)
/*!< Array of EIM base addresses */
#define ESM_BASE_PTRS             {ECC_SRAM}
/*!< Array of ESM IRQs */
#define ESM_IRQS                  {ECC_1BIT_ERROR_IRQn, ECC_2BIT_ERROR_IRQn}
/*!< 1 bit ESM IRQs */
#define ESM_1BIT_IRQS             ((uint32)ESM_CHANNEL0_1BIT_IRQ| (uint32)ESM_CHANNEL1_1BIT_IRQ | (uint32)ESM_CHANNEL2_1BIT_IRQ)
/*!< 2 bits ESM IRQs */
#define ESM_2BIT_IRQS             ((uint32)ESM_CHANNEL0_2BIT_IRQ | (uint32)ESM_CHANNEL1_2BIT_IRQ| (uint32)ESM_CHANNEL2_2BIT_IRQ)

/*!
 * @brief ESM channel type.
 */
typedef enum
{
    ESM_CHANNEL_SRAML,                 /*!< Sram_L Channel. */
    ESM_CHANNEL_SRAMU,                 /*!< Sram_U Channel. */
    ESM_CHANNEL_BOOTROM,               /*!< Boot ROM Channel. */
    ESM_CHANNEL_MAX                    /*!< Channel Number of ESM module */
} Esm_ChannelType;

/*!
 * @brief ESM interrupt source type.
 */
typedef enum
{
    ESM_CHANNEL0_1BIT_IRQ = ECC_SRAM_ECC_ERR_CTRL_ECC0_1BIT_IRQ_EN_Msk, /*!< Sram_L Channel 1bit ECC error.  */
    ESM_CHANNEL0_2BIT_IRQ = ECC_SRAM_ECC_ERR_CTRL_ECC0_2BIT_IRQ_EN_Msk, /*!< Sram_L Channel 2bit ECC error.  */
    ESM_CHANNEL1_1BIT_IRQ = ECC_SRAM_ECC_ERR_CTRL_ECC1_1BIT_IRQ_EN_Msk, /*!< Sram_U Channel 1bit ECC error.  */
    ESM_CHANNEL1_2BIT_IRQ = ECC_SRAM_ECC_ERR_CTRL_ECC1_2BIT_IRQ_EN_Msk, /*!< Sram_U Channel 2bit ECC error.  */
    ESM_CHANNEL2_1BIT_IRQ = ECC_SRAM_ECC_ERR_CTRL_ECC2_1BIT_IRQ_EN_Msk, /*!< BootRom Channel 1bit ECC error. */
    ESM_CHANNEL2_2BIT_IRQ = ECC_SRAM_ECC_ERR_CTRL_ECC2_2BIT_IRQ_EN_Msk, /*!< BootRom Channel 2bit ECC error. */
} Esm_InterruptType;             //PRQA S 1535 # AMCE_SYS_1535

/* =========================================================================================================================== */
/* ================                                            EIM                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the EIM module */
#define EIM_INSTANCE_COUNT         (1U)
/*!< Array of EIM base addresses */
#define EIM_BASE_PTRS              {EIM_CHANNEL0, EIM_CHANNEL1, EIM_CHANNEL2}

/*!
 * @brief EIM channel type.
 */
typedef enum
{
    EIM_CHANNEL_SRAML,                 /*!< Sram_L Channel. */
    EIM_CHANNEL_SRAMU,                 /*!< Sram_U Channel. */
    EIM_CHANNEL_BOOTROM,               /*!< Boot ROM Channel. */
    EIM_CHANNEL_MAX                    /*!< Channel Number of EIM module */
} Eim_ChannelType;

/*!
 * @brief EIM E2E channel type.
 */
typedef enum
{
    EIM_E2E_CHANNEL0 = E2E_EIM_EN_INJ_ECC_BS0ADR_INJEN_Pos, /*!< Bus slave 0 address Channel. */
    EIM_E2E_CHANNEL1 = E2E_EIM_EN_INJ_ECC_BS1ADR_INJEN_Pos, /*!< Bus slave 1 address Channel. */
    EIM_E2E_CHANNEL2 = E2E_EIM_EN_INJ_ECC_BS4ADR_INJEN_Pos, /*!< Bus slave 4 address Channel. */
    EIM_E2E_CHANNEL3 = E2E_EIM_EN_INJ_ECC_BS2ADR_INJEN_Pos, /*!< Bus slave 2 address Channel. */
    EIM_E2E_CHANNEL4 = E2E_EIM_EN_INJ_ECC_BS2WD_INJEN_Pos,  /*!< Bus slave 2 data Channel.    */
    EIM_E2E_CHANNEL5 = E2E_EIM_EN_INJ_ECC_BS3ADR_INJEN_Pos, /*!< Bus slave 3 address Channel. */
    EIM_E2E_CHANNEL6 = E2E_EIM_EN_INJ_ECC_BS3WD_INJEN_Pos,  /*!< Bus slave 3 data Channel.    */
    EIM_E2E_CHANNEL7 = E2E_EIM_EN_INJ_ECC_BS5ADR_INJEN_Pos, /*!< Bus slave 5 address Channel. */
    EIM_E2E_CHANNEL8 = E2E_EIM_EN_INJ_ECC_BS5WD_INJEN_Pos,  /*!< Bus slave 5 data Channel.    */
    EIM_E2E_CHANNEL9 = E2E_EIM_EN_INJ_ECC_BS6ADR_INJEN_Pos, /*!< Bus slave 6 address Channel. */
    EIM_E2E_CHANNEL10 = E2E_EIM_EN_INJ_ECC_BS6WD_INJEN_Pos, /*!< Bus slave 6 data Channel.    */
    EIM_E2E_CHANNEL11 = E2E_EIM_EN_INJ_ECC_BM0RD_INJEN_Pos, /*!< Bus master 0 Channel. */
    EIM_E2E_CHANNEL12 = E2E_EIM_EN_INJ_ECC_BM1RD_INJEN_Pos, /*!< Bus master 1 Channel. */
    EIM_E2E_CHANNEL13 = E2E_EIM_EN_INJ_ECC_BM2RD_INJEN_Pos, /*!< Bus master 2 Channel. */
    EIM_E2E_CHANNEL14 = E2E_EIM_EN_INJ_ECC_BM3RD_INJEN_Pos, /*!< Bus master 3 Channel. */
    EIM_E2E_CHANNEL15 = E2E_EIM_EN_INJ_ECC_BM4RD_INJEN_Pos, /*!< Bus master 4 Channel. */
    EIM_E2E_CHANNEL16 = E2E_EIM_EN_INJ_ECC_BM5RD_INJEN_Pos, /*!< Bus master 5 Channel. */
    EIM_E2E_CHANNEL_MAX               /*!< E2E Channel Number of EIM module */
} Eim_E2EChannelType;

/* =========================================================================================================================== */
/* ================                                          FLASH                                            ================ */
/* =========================================================================================================================== */
/*!< FLASH controler unlock key */
#define FLASH_UNLOCK_KEY1          (0xac7843U)
#define FLASH_UNLOCK_KEY2          (0x01234567U)
/*!< P-Flash page size in byte */
#define PFLASH_PAGE_SIZE           (0x00000800U)
/*!< D-Flash page size in byte */
#define DFLASH_PAGE_SIZE           (0x00000400U)
/*!< Flash info base address */
#define FLASH_INFO_ADDR_BASE       (0x1000000U)
/*!< Flash info address range */
#define FLASH_INFO_ADDR_SIZE       (0x7FFFU)

/*!< P-Flash program unit size */
#define PFLASH_WRITE_UNIT_SIZE     (8U)
/*!< D-Flash program unit size */
#define DFLASH_WRITE_UNIT_SIZE     (8U)

/*!< Eep-Flash Management Unit */
#define EEP_FLASH_GROUP_SIZE        (DFLASH_PAGE_SIZE * 2U)
/* =========================================================================================================================== */
/* ================                                           GPIO                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the GPIO module */
#define PORT_INSTANCE_MAX          (5U)// 43  GPIO_INSTANCE_MAX
/*!< Array of GPIO base addresses */
#define GPIO_BASE_PTRS             {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE}

/*!< Array of GPIO IRQs */
#define PORT_IRQS                  {PORTA_IRQn, PORTB_IRQn, PORTC_IRQn, PORTD_IRQn, PORTE_IRQn}

/* =========================================================================================================================== */
/* ================                                           PORT                                            ================ */
/* =========================================================================================================================== */
/*!< Array of GPIO port base addresses */
#define GPIO_PORT_BASE_PTRS        {PORTA, PORTB, PORTC, PORTD, PORTE}

/* =========================================================================================================================== */
/* ================                                           CAN                                             ================ */
/* =========================================================================================================================== */
/*!< Number of group of the CAN module */
#define CAN_GROUP_MAX              (2U)
/*!< Number of CAN module of the group 0 */
#define CAN_GROUP0_INSTANCE_MAX    (4U)
/*!< Number of CAN module of the group 1 */
#define CAN_GROUP1_INSTANCE_MAX    (2U)
/*!< Number of instances of the CAN module */
#define CAN_INSTANCE_MAX           (6U)
/*!< MRAM0 base address */
#define CAN_MRAM0_BASE             (0x40013000U)
/*!< MRAM1 base address */
#define CAN_MRAM1_BASE             (0x40015000U)
#define CAN_MRAM_BASE              {CAN_MRAM0_BASE, CAN_MRAM1_BASE}
/*!< MRAM0 size */
#define CAN_MRAM0_MAX              (0x2000U)
/*!< MRAM1 size */
#define CAN_MRAM1_MAX              (0x1000U)
/*!< Array of CAN base addresses */
#define CAN_BASE_PTRS              {CAN0, CAN1, CAN2, CAN3, CAN4, CAN5}
/*!< Array of CAN IRQs */
#define CAN_IRQS                   {CAN0_IRQn, CAN1_IRQn, CAN2_IRQn, CAN3_IRQn, CAN4_IRQn, CAN5_IRQn}
/*!< Array of CAN wakeup IRQs */
#define CAN_WAKEUP_IRQS            {CAN0_WAKEUP_IRQn, CAN1_WAKEUP_IRQn, CAN2_WAKEUP_IRQn, CAN3_WAKEUP_IRQn, CAN4_WAKEUP_IRQn, CAN5_WAKEUP_IRQn}
/*!< Array of CAN DMU IRQs */
#define CAN_DMU_IRQS               {CAN0_DMU_IRQn, CAN1_DMU_IRQn, CAN2_DMU_IRQn, CAN3_DMU_IRQn, CAN4_DMU_IRQn, CAN5_DMU_IRQn}
/*!< Array of CAN ckgen interface clocks */
#define CAN_CKGEN_CLOCKS           {CLK_CAN0, CLK_CAN1, CLK_CAN2, CLK_CAN3, CLK_CAN4, CLK_CAN5}
/*!< Array of CAN soft resets */
#define CAN_SOFT_RESETS            {SRST_CAN0, SRST_CAN1, SRST_CAN2, SRST_CAN3, SRST_CAN4, SRST_CAN5}
/*!< Array of CAN clock names */
#define CAN_CLOCK_NAMES            {CAN0_CLK, CAN1_CLK, CAN2_CLK, CAN3_CLK, CAN4_CLK, CAN5_CLK}

/* =========================================================================================================================== */
/* ================                                           UART                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the UART module */
#define UART_INSTANCE_MAX          (8U)
/*!< Array of UART base addresses */
#define UART_BASE_PTRS             {UART0, UART1, UART2, UART3, UART4, UART5, UART6, UART7}
/*!< Array of UART IRQs */
#define UART_IRQS                  {UART0_IRQn, UART1_IRQn, UART2_IRQn, UART3_IRQn, UART4_IRQn, UART5_IRQn, UART6_IRQn, UART7_IRQn}
/*!< Array of UART ckgen interface clocks */
#define UART_CKGEN_CLOCKS          {CLK_UART0, CLK_UART1, CLK_UART2, CLK_UART3, CLK_UART4, CLK_UART5, CLK_UART6, CLK_UART7}
/*!< Array of UART soft resets */
#define UART_SOFT_RESETS           {SRST_UART0, SRST_UART1, SRST_UART2, SRST_UART3, SRST_UART4, SRST_UART5, SRST_UART6, SRST_UART7}
/*!< Array of UART clock names */
#define UART_CLOCK_NAMES           {UART0_CLK, UART1_CLK, UART2_CLK, UART3_CLK, UART4_CLK, UART5_CLK, UART6_CLK, UART7_CLK}

/* =========================================================================================================================== */
/* ================                                           SENT                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the SENT module */
#define SENT_INSTANCE_MAX          (4U)
/*!< Array of SENT base addresses */
#define SENT_BASE_PTRS             {SENT_CHANNEL0, SENT_CHANNEL1, SENT_CHANNEL2, SENT_CHANNEL3}
/*!< Array of SENT IRQs */
#define SENT_IRQS                  {SENT_CHANNEL0_IRQn, SENT_CHANNEL1_IRQn, SENT_CHANNEL2_IRQn, SENT_CHANNEL3_IRQn}
/*!< Array of SENT ckgen interface clocks */
#define SENT_CKGEN_CLOCKS          {CLK_SENT}
/*!< Array of SENT soft resets */
#define SENT_SOFT_RESETS           {SRST_SENT}
/*!< Array of SENT clock names */
#define SENT_CLOCK_NAMES           {SENT_CLK}

/* =========================================================================================================================== */
/* ================                                           I2C                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the I2C module */
#define I2C_INSTANCE_MAX           (3U)
/*!< Array of I2C base addresses */
#define I2C_BASE_PTRS              {I2C0, I2C1, I2C2}
/*!< Array of I2C IRQs */
#define I2C_IRQS                   {I2C0_IRQn, I2C1_IRQn, I2C2_IRQn}
/*!< Array of I2C ckgen interface clocks */
#define I2C_CKGEN_CLOCKS           {CLK_I2C0, CLK_I2C1, CLK_I2C2}
/*!< Array of I2C soft resets */
#define I2C_SOFT_RESETS            {SRST_I2C0, SRST_I2C1, SRST_I2C2}
/*!< Array of I2C clock names */
#define I2C_CLOCK_NAMES            {I2C0_CLK, I2C1_CLK, I2C2_CLK}

/* =========================================================================================================================== */
/* ================                                           SPI                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the SPI module */
#define SPI_INSTANCE_MAX           (5U)
/*!< Array of SPI base addresses */
#define SPI_BASE_PTRS              {SPI0, SPI1, SPI2, SPI3, SPI4}
/*!< Array of SPI IRQs */
#define SPI_IRQS                   {SPI0_IRQn, SPI1_IRQn, SPI2_IRQn, SPI3_IRQn, SPI4_IRQn}
/*!< Array of SPI ckgen interface clocks */
#define SPI_CKGEN_CLOCKS           {CLK_SPI0, CLK_SPI1, CLK_SPI2, CLK_SPI3, CLK_SPI4}
/*!< Array of SPI soft resets */
#define SPI_SOFT_RESETS            {SRST_SPI0, SRST_SPI1, SRST_SPI2, SRST_SPI3, SRST_SPI4}
/*!< Array of SPI clock names */
#define SPI_CLOCK_NAMES            {SPI0_CLK, SPI1_CLK, SPI2_CLK, SPI3_CLK, SPI4_CLK}

/* =========================================================================================================================== */
/* ================                                           EIO                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the EIO module */
#define EIO_INSTANCE_COUNT         (1U)
/*!< Define the maximum number of shifters for any EIO instance. */
#define EIO_MAX_SHIFTER_COUNT      (4U)
/*!< Array of EIO IRQs */
#define EIO_IRQS                   {EIO_IRQn}
/*!< Array of EIO clock names */
#define EIO_CLOCK_NAMES            {EIO_CLK}
/*!< Array of EIO base addresses */
#define EIO_BASE_PTRS              {EIO}

/* =========================================================================================================================== */
/* ================                                           CRC                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the CRC module */
#define CRC_INSTANCE_MAX           (1U)
/*!< Array of CRC base addresses */
#define CRC_BASE_PTRS              {CRC}
/*!< Array of CRC ckgen interface clocks */
#define CRC_CKGEN_CLOCKS           {CLK_CRC}
/*!< Array of CRC soft resets */
#define CRC_SOFT_RESETS            {SRST_CRC}

/* =========================================================================================================================== */
/* ================                                           RTC                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the RTC module */
#define RTC_INSTANCE_MAX           (1U)
/*!< Array of RTC base addresses */
#define RTC_BASE_PTRS              {RTC}
/*!< Array of RTC IRQs */
#define RTC_IRQS                   {RTC_IRQn}
/*!< Array of RTC ckgen interface clocks */
#define RTC_CKGEN_CLOCKS           {CLK_RTC}

/* =========================================================================================================================== */
/* ================                                           WDG                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the WDG module */
#define WDG_INSTANCE_MAX           (1U)
/*!< Array of WDG base addresses */
#define WDG_BASE_PTRS              {WDG}
/*!< Array of WDG IRQs */
#define WDG_IRQS                   {WDG_IRQn}
/*!< Array of WDG ckgen interface clocks */
#define WDG_CKGEN_CLOCKS           {CLK_WDG}
/*!< Array of WDG soft resets */
#define WDG_SOFT_RESETS            {SRST_WDG}
/* The reset value of the wdg window register */
#define WDG_WIN_RESET_DEFALUT_VALUE  (0x0U)
/* The reset value of the wdg timeout register */
#define WDG_TIMEOUT_RESET_DEFAULT_VALUE  (0x5000U)
/* The first 32-bit value used for unlocking the wdg */
#define WDG_UNLOCK_FIRST_VALUE     (0xE064D987U)
/* The second 32-bit value used for unlocking the wdg */
#define WDG_UNLOCK_SECOND_VALUE    (0x868A8478U)
/* The first 32-bit value used for feed the wdg */
#define WDG_FEED_FIRST_VALUE       (0x7908AD15U)
/* The second 32-bit value used for feed the wdg */
#define WDG_FEED_SECOND_VALUE      (0x5AD5A879U)
/* The default reset value of WDG CS0 register */
#define WDG_CS0_RESET_VALUE        (0x20U)
/* The default reset value of WDG CS1 register */
#define WDG_CS1_RESET_VALUE        (0x0U)
/* The default reset value of WDG TOVAL register */
#define WDG_TOVAL_RESET_VALUE      (0x5000U)
/* The default reset value of WDG WIN register */
#define WDG_WIN_RESET_VALUE        (0x0U)

/* =========================================================================================================================== */
/* ================                                           EWDG                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the EWDG module */
#define EWDG_INSTANCE_MAX          (1U)
/*!< Array of EWDG IRQs */
#define EWDG_BASE_PTRS             {EWDG}
/*!< Array of EWDG IRQs */
#define EWDG_IRQS                  {EWDG_IRQn}
/*!< Array of EWDG ckgen interface clocks */
#define EWDG_CKGEN_CLOCKS          {CLK_EWDG}
/*!< Array of EWDG soft resets */
#define EWDG_SOFT_RESETS           {SRST_EWDG}
/*!< EWDG refresh key values */
#define EWDG_KEY_FIRST_BYTE        (0xB4U)
#define EWDG_KEY_SECOND_BYTE       (0x2CU)
/*!< EWDG CMPH CMPL limit values */
#define EWDG_CMPH_MAX_VALUE        (0xFEU)
#define EWDG_CMPL_MIN_VALUE        (0x00U)

/* =========================================================================================================================== */
/* ================                                            DMA                                            ================ */
/* =========================================================================================================================== */
#define DMA_TRANSFER_LENGTH_MAX             (32767U)

#define DMA_CH_MAX                          (16U)

#define DMA_VIRTUAL_CH_MAX                  (20U)

#define DMA_ERROR_EVENT                     (DMA_CHANNEL_STATUS_DBE_Msk | DMA_CHANNEL_STATUS_SBE_Msk \
                                                        | DMA_CHANNEL_STATUS_DOE_Msk | DMA_CHANNEL_STATUS_SOE_Msk \
                                                        | DMA_CHANNEL_STATUS_DAE_Msk | DMA_CHANNEL_STATUS_SAE_Msk \
                                                        | DMA_CHANNEL_STATUS_CLE_Msk)

#define DMA_FINISH_EVENT                    (DMA_CHANNEL_STATUS_FINISH_Msk)

#define DMA_HALF_FINISH_EVENT               (DMA_CHANNEL_STATUS_HALF_FINISH_Msk)

#define  DMA_REQ_ALWAYS_ENABLED             (120U)

/*!
 * @brief DMA interrupt source type.
 */
typedef enum
{
    DMA_IRQ_NONE = 0U,                                                              /*!< DMA disable all interrupt. */
    DMA_FINISH_IRQ = DMA_CHANNEL_INTEN_FINISH_INTERRUPT_ENABLE_Msk,                 /*!< DMA finish interrupt. */
    DMA_HALF_FINISH_IRQ = DMA_CHANNEL_INTEN_HALF_FINISH_INTERRUPT_ENABLE_Msk,       /*!< DMA half finish interrupt. */
    DMA_ERROR_IRQ = DMA_CHANNEL_INTEN_TRANS_ERROR_INTERRUPT_ENABLE_Msk,             /*!< DMA error interrupt. */
} Dma_IrqSrcType;               //PRQA S 1535 # AMCE_SYS_1535

/*!
 * @brief DMA request for the DMA channel.
 */
typedef enum
{
    DMA_REQ_DISABLE = 0U,                  /*!< Triggered by input dege capture or output count reaching the threshold on pwm1 channel 7. */
    DMA_REQ_I2C0_RX = 9U,                  /*!< I2C0 receive register/FIFO not full. */
    DMA_REQ_I2C0_TX = 10U,                 /*!< I2C0 transmit register/FIFO not full. */
    DMA_REQ_I2C1_RX = 11U,                 /*!< I2C1 receive register/FIFO not full. */
    DMA_REQ_I2C1_TX = 12U,                 /*!< I2C1 transmit register/FIFO not full. */
    DMA_REQ_I2C2_RX = 13U,                 /*!< I2C2 receive register/FIFO not full. */
    DMA_REQ_I2C2_TX = 14U,                 /*!< I2C2 transmit register/FIFO not full. */
    DMA_REQ_PWM1_CHANNEL_0 = 17U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm1 channel 0. */
    DMA_REQ_PWM1_CHANNEL_1 = 18U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm1 channel 1. */
    DMA_REQ_PWM1_CHANNEL_2 = 19U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm1 channel 2. */
    DMA_REQ_PWM1_CHANNEL_3 = 20U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm1 channel 3. */
    DMA_REQ_PWM1_CHANNEL_4 = 21U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm1 channel 4. */
    DMA_REQ_PWM1_CHANNEL_5 = 22U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm1 channel 5. */
    DMA_REQ_PWM1_CHANNEL_6 = 23U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm1 channel 6. */
    DMA_REQ_PWM1_CHANNEL_7 = 24U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm1 channel 7. */
    DMA_REQ_PWM1_UNDER_OR_OVER_FLOW = 25U, /*!< Underflow or overflow occurs on pwm1 any channel*/
    DMA_REQ_PWM2_CHANNEL_0 = 26U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm2 channel 0. */
    DMA_REQ_PWM2_CHANNEL_1 = 27U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm2 channel 1. */
    DMA_REQ_PWM2_CHANNEL_2 = 28U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm2 channel 2. */
    DMA_REQ_PWM2_CHANNEL_3 = 29U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm2 channel 3. */
    DMA_REQ_PWM2_CHANNEL_4 = 30U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm2 channel 4. */
    DMA_REQ_PWM2_CHANNEL_5 = 31U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm2 channel 5. */
    DMA_REQ_PWM2_CHANNEL_6 = 32U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm2 channel 6. */
    DMA_REQ_PWM2_CHANNEL_7 = 33U,          /*!< Triggered by input dege capture or output count reaching the threshold on pwm2 channel 7. */
    DMA_REQ_PWM2_UNDER_OR_OVER_FLOW = 34U, /*!< Underflow or overflow occurs on pwm2 any channel*/
    DMA_REQ_PWM0_OR_CH0_CH7 = 35U,         /*!< Triggered by input dege capture or output count reaching the threshold on pwm0 any channel. */
    DMA_REQ_PWM0_UNDER_OR_OVER_FLOW = 36U,
    DMA_REQ_PWM3_OR_CH0_CH7 = 37U,
    DMA_REQ_PWM3_UNDER_OR_OVER_FLOW = 38U,
    DMA_REQ_PWM4_OR_CH0_CH7 = 39U,
    DMA_REQ_PWM4_UNDER_OR_OVER_FLOW = 40U,
    DMA_REQ_PWM5_OR_CH0_CH7 = 41U,
    DMA_REQ_PWM5_UNDER_OR_OVER_FLOW = 42U,
    DMA_REQ_PWM6_OR_CH0_CH7 = 43U,
    DMA_REQ_PWM6_UNDER_OR_OVER_FLOW = 44U,
    DMA_REQ_PWM7_OR_CH0_CH7 = 45U,
    DMA_REQ_PWM7_UNDER_OR_OVER_FLOW = 46U,
    DMA_REQ_UART0_RX = 51U,
    DMA_REQ_UART0_TX = 52U,
    DMA_REQ_UART1_RX = 53U,
    DMA_REQ_UART1_TX = 54U,
    DMA_REQ_UART2_RX = 55U,
    DMA_REQ_UART2_TX = 56U,
    DMA_REQ_UART3_RX = 57U,
    DMA_REQ_UART3_TX = 58U,
    DMA_REQ_UART4_RX = 59U,
    DMA_REQ_UART4_TX = 60U,
    DMA_REQ_UART5_RX = 61U,
    DMA_REQ_UART5_TX = 62U,
    DMA_REQ_UART6_RX = 63U,
    DMA_REQ_UART6_TX = 64U,
    DMA_REQ_UART7_RX = 65U,
    DMA_REQ_UART7_TX = 66U,
    DMA_REQ_PORTA = 71U,
    DMA_REQ_PORTB = 72U,
    DMA_REQ_PORTC = 73U,
    DMA_REQ_PORTD = 74U,
    DMA_REQ_PORTE = 75U,
    DMA_REQ_SPI0_RX = 79U,
    DMA_REQ_SPI0_TX = 80U,
    DMA_REQ_SPI1_RX = 81U,
    DMA_REQ_SPI1_TX = 82U,
    DMA_REQ_SPI2_RX = 83U,
    DMA_REQ_SPI2_TX = 84U,
    DMA_REQ_SPI3_RX = 85U,
    DMA_REQ_SPI3_TX = 86U,
    DMA_REQ_SPI4_RX = 87U,
    DMA_REQ_SPI4_TX = 88U,
    DMA_REQ_CAN0_RX0 = 91U,
    DMA_REQ_CAN0_RX1 = 92U,
    DMA_REQ_CAN1_RX0 = 93U,
    DMA_REQ_CAN1_RX1 = 94U,
    DMA_REQ_CAN2_RX0 = 95U,
    DMA_REQ_CAN2_RX1 = 96U,
    DMA_REQ_CAN3_RX0 = 97U,
    DMA_REQ_CAN3_RX1 = 98U,
    DMA_REQ_CAN4_RX0 = 99U,
    DMA_REQ_CAN4_RX1 = 100U,
    DMA_REQ_CAN5_RX0 = 101U,
    DMA_REQ_CAN5_RX1 = 102U,
    DMA_REQ_EIO_SHIFTER0 = 107U,
    DMA_REQ_EIO_SHIFTER1 = 108U,
    DMA_REQ_EIO_SHIFTER2 = 109U,
    DMA_REQ_EIO_SHIFTER3 = 110U,
    DMA_REQ_ADC0 = 113U,
    DMA_REQ_ADC1 = 114U,
    DMA_REQ_CRC = 121U,
    DMA_REQ_MEM2MEM = 122U
} Dma_RequestSourceType;

/**
 * @brief DMA channel priority setting
 */
typedef enum
{
    DMA_CHANNEL_PRIORITY_LOW = 0U,  /*!< 0: DMA priority low */
    DMA_CHANNEL_PRIORITY_MEDIUM,    /*!< 1: DMA priority medium */
    DMA_CHANNEL_PRIORITY_HIGH,      /*!< 2: DMA priority high */
    DMA_CHANNEL_PRIORITY_VERY_HIGH  /*!< 3: DMA priority very high */
} Dma_ChannelPriorityType;

/**
 * @brief DMA transfer unit size configuration
 */
typedef enum
{
    DMA_TRANSFER_UNIT_1B = 0x0U,    /*!< 0: Signle transfer data size is 8 bit*/
    DMA_TRANSFER_UNIT_2B = 0x1U,    /*!< 0: Signle transfer data size is 16 bit*/
    DMA_TRANSFER_UNIT_4B = 0x2U     /*!< 0: Signle transfer data size is 32 bit*/
} Dma_TransferUnitType;

/**
 * @brief Type for the DMA transfer.
 */
typedef enum
{
    DMA_TRANSFER_PERIPH2MEM = 0U,   /*!< Transfer from peripheral to memory */
    DMA_TRANSFER_MEM2PERIPH,        /**< Transfer from memory to peripheral */
    DMA_TRANSFER_MEM2MEM,           /**< Transfer from memory to memory */
    DMA_TRANSFER_PERIPH2PERIPH      /**< Transfer from peripheral to peripheral */
} Dma_TransferType;

/**
 * @brief Type for the DMA global reset.
 */
typedef enum
{
    DMA_CHANNEL_SW_RESET = 0U, /*!< Signle channel software reset */
    DMA_CHANNEL_HW_RESET,      /*!< Signle channel hardware reset */
} Dma_ChannelResetType;

/**
 * @brief Type for the DMA global reset.
 */
typedef enum
{
    DMA_GLOBAL_SW_RESET = 0U, /*!< global software reset */
    DMA_GLOBAL_HW_RESET,      /*!< global hardware reset */
} Dma_GlobalResetType;          //PRQA S 1535 # AMCE_SYS_1535

/* =========================================================================================================================== */
/* ================                                           ADC                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the ADC module */
#define ADC_INSTANCE_MAX           (2U)
/*!< Arrays of ADC base address */
#define ADC_BASE                   {ADC0_BASE, ADC0_BASE}
/*!< Arrays of ADC base address ptrs*/
#define ADC_BASE_PTRS              {ADC0, ADC1}
/*!< Arrays of ADC ckgen interface clocks */
#define ADC_CKGEN_CLOCKS           {CKGEN_ADC0_CLK, CKGEN_ADC1_CLK}
/*!< Arrays of ADC soft resets */
#define ADC_SOFT_RESETS            {SRST_ADC0, SRST_ADC1}
/*!< Array of ADC IRQs */
#define ADC_IRQS                   {ADC0_IRQn, ADC1_IRQn}
/*!< Array of ADC DMA requests */
#define ADC_DMA_REQEUSTS           {DMA_REQ_ADC0, DMA_REQ_ADC1}
/*!< Array of ADC clock names */
#define ADC_CLOCK_NAMES            {ADC0_CLK, ADC1_CLK}
/*!< Arrays of ADC ckgen interface bus clocks */
#define ADC_CKGEN_BUS_CLKS         {CKGEN_ADC0_BUS_CLK, CKGEN_ADC1_BUS_CLK}
/*!< Max clock frequence of ADC function clock */
#define ADC_CLOCK_FREQ_MAX_RUNTIME (30000000U)
/*!< Array of ADC CTU triger modules [Instance][Type]: [0]=Regular, [0]=Injection */
#define ADC_CTU_TRIG_BASE          {{TRIG_SEL_ADC0_REGULAR0, TRIG_SEL_ADC0_INJECTION0}, \
                                    {TRIG_SEL_ADC1_REGULAR0, TRIG_SEL_ADC1_INJECTION0}}
/*!< Number of ADC group modes (Regular and Inject) */
#define ADC_GROUP_MODE_COUNT       (2U)

/* =========================================================================================================================== */
/* ================                                           ACMP                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the ACMP module */
#define ACMP_INSTANCE_MAX          (2U)
/*!< Arrays of ACMP base address */
#define ACMP_BASE_PTRS             {ACMP0, ACMP1}
/*!< Arrays of ACMP ckgen interface clocks */
#define ACMP_CKGEN_CLOCKS          {CLK_ACMP0, CLK_ACMP1}
/*!< Arrays of ACMP soft resets */
#define ACMP_SOFT_RESETS           {SRST_ACMP0, SRST_ACMP1}
/*!< Array of ACMP IRQs */
#define ACMP_IRQS                  {ACMP0_IRQn, ACMP1_IRQn}

/* =========================================================================================================================== */
/* ================                                           PWM                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the PWM module */
#define PWM_INSTANCE_MAX           (8U)
/*!< Array of PWM base addresses */
#define PWM_BASE_PTRS              {PWM0, PWM1, PWM2, PWM3, PWM4, PWM5, PWM6, PWM7}
/*!< Array of PWM Overflow IRQs */
#define PWM_OVERFLOW_IRQS          {PWM0_OVERFLOW_IRQn, PWM1_OVERFLOW_IRQn, PWM2_OVERFLOW_IRQn, PWM3_OVERFLOW_IRQn, PWM4_OVERFLOW_IRQn, PWM5_OVERFLOW_IRQn, PWM6_OVERFLOW_IRQn, PWM7_OVERFLOW_IRQn}
/*!< Array of PWM Channel IRQs */
#define PWM_CHANNEL_IRQS           {PWM0_CHANNEL_IRQn, PWM1_CHANNEL_IRQn, PWM2_CHANNEL_IRQn, PWM3_CHANNEL_IRQn, PWM4_CHANNEL_IRQn, PWM5_CHANNEL_IRQn, PWM6_CHANNEL_IRQn, PWM7_CHANNEL_IRQn}
/*!< Array of PWM Fault IRQs */
#define PWM_FAULT_IRQS             {PWM0_FAULT_IRQn, PWM1_FAULT_IRQn, PWM2_FAULT_IRQn, PWM3_FAULT_IRQn, PWM4_FAULT_IRQn, PWM5_FAULT_IRQn, PWM6_FAULT_IRQn, PWM7_FAULT_IRQn}
/*!< Array of PWM Detect IRQs */
#define PWM_DETECT_IRQS            {PWM0_DETECT_IRQn, PWM1_DETECT_IRQn, PWM2_DETECT_IRQn, PWM3_DETECT_IRQn, PWM4_DETECT_IRQn, PWM5_DETECT_IRQn, PWM6_DETECT_IRQn, PWM7_DETECT_IRQn}
/*!< Array of PWM ckgen interface clocks */
#define PWM_CKGEN_CLOCKS           {CLK_PWM0, CLK_PWM1, CLK_PWM2, CLK_PWM3, CLK_PWM4, CLK_PWM5, CLK_PWM6, CLK_PWM7}
/*!< Array of PWM soft resets */
#define PWM_SOFT_RESETS            {SRST_PWM0, SRST_PWM1, SRST_PWM2, SRST_PWM3, SRST_PWM4, SRST_PWM5, SRST_PWM6, SRST_PWM7}

/* =========================================================================================================================== */
/* ================                                           PDT                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the PDT module */
#define PDT_INSTANCE_MAX           (2U)
/*!< Array of PDT base addresses */
#define PDT_BASE_PTRS              {PDT0, PDT1}
/*!< Array of PDT IRQs */
#define PDT_IRQS                   {PDT0_IRQn, PDT1_IRQn}
/*!< Array of PDT ckgen interface clocks */
#define PDT_CKGEN_CLOCKS           {CKGEN_PDT0_BUS_CLK, CKGEN_PDT1_BUS_CLK}
/*!< Array of PDT soft resets */
#define PDT_SOFT_RESETS            {SRST_PDT0, SRST_PDT1}

/* =========================================================================================================================== */
/* ================                                           TIMER                                           ================ */
/* =========================================================================================================================== */
/*!< Channel Number of timer module */
#define TIMER_CHANNEL_MAX          (4U)
/*!< Number of instances of the timer module */
#define TIMER_INSTANCE_COUNT       (1U)
/*!< Array of timer base addresses */
#define TIMER_BASE_PTRS            {TIMER_CHANNEL0, TIMER_CHANNEL1, TIMER_CHANNEL2, TIMER_CHANNEL3}
/*!< Array of timer IRQs */
#define TIMER_IRQS                 {TIMER_CHANNEL0_IRQn, TIMER_CHANNEL1_IRQn, TIMER_CHANNEL2_IRQn, TIMER_CHANNEL3_IRQn}
/*!< Array of timer clock names */
#define TIMER_CLOCK_NAMES          {TIMER_CLK}

/* =========================================================================================================================== */
/* ================                                            PCT                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the PCT module */
#define PCT_INSTANCE_MAX         (1U)
/*!< Array of PCT base addresses */
#define PCT_BASE_PTRS              {PCT}
/*!< Array of PCT IRQs */
#define PCT_IRQS                   {PCT_IRQn}
/*!< Array of PCT clock names */
#define PCT_CLOCK_NAMES            {PCT_CLK}

/* =========================================================================================================================== */
/* ================                                            CTU                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the CTU module */
#define CTU_INSTANCE_MAX           (1U)
/*!< Array of CTU base addresses */
#define CTU_BASE_PTRS              {CTU}
/*!< Array of CTU ckgen interface clocks */
#define CTU_CKGEN_CLOCKS           {CLK_CTU}
/*!< Array of CTU soft resets */
#define CTU_SOFT_RESETS            {SRST_CTU}

/*!
 * @brief Ctu Func id
 */
typedef enum
{
    CTU_UART0_RX_FILTER = 0U,               /*!< Enable UART0 RX FILTER FUNCTION .*/
    CTU_UART1_RX_FILTER,                    /*!< Enable UART1 RX FILTER FUNCTION */
    CTU_UART0_TX_MODULATION,
    CTU_UART1_TX_MODULATION,
    CTU_RTC_CLK_CAP = 31U,
    CTU_PWM1_CH0_OUTSEL_ACMP0 = 32U,
    CTU_PWM3_CH0_OUTSEL_ACMP0,
    CTU_PWM2_OUTSEL_HALL,
    CTU_PWM5_CH0_OUTSEL_ACMP1 = 36U,
    CTU_PWM7_CH0_OUTSEL_ACMP1,
    CTU_PWM6_OUTSEL_HALL,
    CTU_PWM0_SYNC = 40U,
    CTU_PWM1_SYNC,
    CTU_PWM2_SYNC,/* idx is 10 */
    CTU_PWM3_SYNC,
    CTU_PWM4_SYNC,
    CTU_PWM5_SYNC,
    CTU_PWM6_SYNC,
    CTU_PWM7_SYNC,
    CTU_PWM0_FAULT0,
    CTU_PWM0_FAULT1,
    CTU_PWM0_FAULT2,
    CTU_PWM1_FAULT0 = 52U,
    CTU_PWM1_FAULT1,
    CTU_PWM1_FAULT2,
    CTU_PWM2_FAULT0 = 56U,
    CTU_PWM2_FAULT1,
    CTU_PWM2_FAULT2,
    CTU_PWM3_FAULT0 = 60U,
    CTU_PWM3_FAULT1,
    CTU_PWM3_FAULT2,
    CTU_PWM0_CH0_OUTSEL_PWM1_CH1 = 96U,
    CTU_PWM0_CH1_OUTSEL_PWM1_CH1,
    CTU_PWM0_CH2_OUTSEL_PWM1_CH1,
    CTU_PWM0_CH3_OUTSEL_PWM1_CH1,
    CTU_PWM0_CH4_OUTSEL_PWM1_CH1,
    CTU_PWM0_CH5_OUTSEL_PWM1_CH1,
    CTU_PWM0_CH6_OUTSEL_PWM1_CH1,
    CTU_PWM0_CH7_OUTSEL_PWM1_CH1,
    CTU_PWM3_CH0_OUTSEL_PWM2_CH1,
    CTU_PWM3_CH1_OUTSEL_PWM2_CH1,
    CTU_PWM3_CH2_OUTSEL_PWM2_CH1,
    CTU_PWM3_CH3_OUTSEL_PWM2_CH1,
    CTU_PWM3_CH4_OUTSEL_PWM2_CH1,
    CTU_PWM3_CH5_OUTSEL_PWM2_CH1,
    CTU_PWM3_CH6_OUTSEL_PWM2_CH1,
    CTU_PWM3_CH7_OUTSEL_PWM2_CH1,
    CTU_ADC_SIMU_REG = 128U,
    CTU_ADC_SIMU_INJ,
    CTU_ADC0_INTER_PB13,
    CTU_ADC0_INTER_PB14,
    CTU_ADC1_INTER_PB0,
    CTU_ADC1_INTER_PB1,
} Ctu_FuncIdType;

/*!
 * @brief Ctu software trigger id
 */
typedef enum
{
    CTU_SW_TRIG1 = 64U,     /*!< Trigger software 1 .*/
    CTU_SW_TRIG2,           /*!< Trigger software 2 .*/
    CTU_SW_TRIG3,           /*!< Trigger software 3 .*/
    CTU_SW_TRIG4,           /*!< Trigger software 4 .*/
} Ctu_SWTriggerIdType;

/*!
 * @brief Ctu Target module
 */
typedef enum
{
    TRIG_SEL_DMA_CH0 = 0U,
    TRIG_SEL_DMA_CH1,
    TRIG_SEL_DMA_CH2,
    TRIG_SEL_DMA_CH3,
    TRIG_SEL_EXT_OUT0 = 4U,
    TRIG_SEL_EXT_OUT1,
    TRIG_SEL_EXT_OUT2,
    TRIG_SEL_EXT_OUT3,
    TRIG_SEL_EXT_OUT4 = 8U,
    TRIG_SEL_EXT_OUT5,/* idx is 10 */
    TRIG_SEL_EXT_OUT6,
    TRIG_SEL_EXT_OUT7,
    TRIG_SEL_ADC0_REGULAR0 = 12U,
    TRIG_SEL_ADC0_REGULAR1,
    TRIG_SEL_ADC0_REGULAR2,
    TRIG_SEL_ADC0_REGULAR3,
    TRIG_SEL_ADC0_INJECTION0 = 16U,
    TRIG_SEL_ADC0_INJECTION1,
    TRIG_SEL_ADC0_INJECTION2,
    TRIG_SEL_ADC0_INJECTION3,/* idx is 20 */
    TRIG_SEL_ADC1_REGULAR0 = 20U,
    TRIG_SEL_ADC1_REGULAR1,
    TRIG_SEL_ADC1_REGULAR2,
    TRIG_SEL_ADC1_REGULAR3,
    TRIG_SEL_ADC1_INJECTION0 = 24U,
    TRIG_SEL_ADC1_INJECTION1,
    TRIG_SEL_ADC1_INJECTION2,
    TRIG_SEL_ADC1_INJECTION3,
    TRIG_SEL_ACMP0 = 28U,
    TRIG_SEL_ACMP1,
    TRIG_SEL_PWM0_TRIG0 = 32U,/* idx is 30 */
    TRIG_SEL_PWM0_FAULT0,
    TRIG_SEL_PWM0_FAULT1,
    TRIG_SEL_PWM0_FAULT2,
    TRIG_SEL_PWM1_TRIG0 = 36U,
    TRIG_SEL_PWM1_FAULT0,
    TRIG_SEL_PWM1_FAULT1,
    TRIG_SEL_PWM1_FAULT2,
    TRIG_SEL_PWM2_TRIG0 = 40U,
    TRIG_SEL_PWM2_FAULT0,
    TRIG_SEL_PWM2_FAULT1,/* idx is 40 */
    TRIG_SEL_PWM2_FAULT2,
    TRIG_SEL_PWM3_TRIG0 = 44U,
    TRIG_SEL_PWM3_FAULT0,
    TRIG_SEL_PWM3_FAULT1,
    TRIG_SEL_PWM3_FAULT2,
    TRIG_SEL_PWM4_TRIG0 = 48U,
    TRIG_SEL_PWM5_TRIG0 = 52U,
    TRIG_SEL_PWM6_TRIG0 = 56U,
    TRIG_SEL_PWM7_TRIG0 = 60U,
    TRIG_SEL_TIMER_CH0 = 64U,
    TRIG_SEL_TIMER_CH1,
    TRIG_SEL_TIMER_CH2,/* idx is 50 */
    TRIG_SEL_TIMER_CH3,
    TRIG_SEL_PCT0 = 68U,
    TRIG_SEL_UART0 = 72U,
    TRIG_SEL_UART1 = 76U,
    TRIG_SEL_PDT0 = 80U,
    TRIG_SEL_PDT1 = 84U,
    TRIG_SEL_EIO_TIMER0 = 88U,
    TRIG_SEL_EIO_TIMER1,
    TRIG_SEL_EIO_TIMER2,
    TRIG_SEL_EIO_TIMER3,/* idx is 60 */
} Ctu_TargetModuleType;

/*!
 * @brief Ctu Trigger source
 */
typedef enum
{
    TRIG_SOURCE_DISABLE = 0U,
    TRIG_SOURCE_ENABLE,
    TRIG_SOURCE_EXT_IN0,
    TRIG_SOURCE_EXT_IN1,
    TRIG_SOURCE_EXT_IN2,
    TRIG_SOURCE_EXT_IN3,
    TRIG_SOURCE_EXT_IN4,
    TRIG_SOURCE_EXT_IN5,
    TRIG_SOURCE_EXT_IN6,
    TRIG_SOURCE_EXT_IN7,
    TRIG_SOURCE_EXT_IN8,/* idx is 10 */
    TRIG_SOURCE_EXT_IN9,
    TRIG_SOURCE_EXT_IN10,
    TRIG_SOURCE_EXT_IN11,
    TRIG_SOURCE_ACMP0_OUT,/* idx is 14 */
    TRIG_SOURCE_ACMP1_OUT,
    TRIG_SOURCE_TIMER_CH0 = 16U,
    TRIG_SOURCE_TIMER_CH1,
    TRIG_SOURCE_TIMER_CH2,
    TRIG_SOURCE_TIMER_CH3,
    TRIG_SOURCE_PCT0_TRIG,/* idx is 20 */
    TRIG_SOURCE_PWM0_INIT_TRIG,
    TRIG_SOURCE_PWM0_MATCH_TRIG,
    TRIG_SOURCE_PWM0_MAX_TRIG,
    TRIG_SOURCE_PWM1_INIT_TRIG,
    TRIG_SOURCE_PWM1_MATCH_TRIG,
    TRIG_SOURCE_PWM1_MAX_TRIG,
    TRIG_SOURCE_PWM2_INIT_TRIG,
    TRIG_SOURCE_PWM2_MATCH_TRIG,
    TRIG_SOURCE_PWM2_MAX_TRIG,
    TRIG_SOURCE_PWM3_INIT_TRIG,/* idx is 30 */
    TRIG_SOURCE_PWM3_MATCH_TRIG,
    TRIG_SOURCE_PWM3_MAX_TRIG,
    TRIG_SOURCE_PWM4_INIT_TRIG,
    TRIG_SOURCE_PWM4_MATCH_TRIG,
    TRIG_SOURCE_PWM4_MAX_TRIG,
    TRIG_SOURCE_PWM5_INIT_TRIG,
    TRIG_SOURCE_PWM5_MATCH_TRIG,
    TRIG_SOURCE_PWM5_MAX_TRIG,/* idx is 38 */
    TRIG_SOURCE_PWM6_INIT_TRIG,
    TRIG_SOURCE_PWM6_MATCH_TRIG,/* idx is 40 */
    TRIG_SOURCE_PWM6_MAX_TRIG,
    TRIG_SOURCE_PWM7_INIT_TRIG,
    TRIG_SOURCE_PWM7_MATCH_TRIG,
    TRIG_SOURCE_PWM7_MAX_TRIG,
    TRIG_SOURCE_ADC0_EOC = 45U,
    TRIG_SOURCE_ADC0_IEOC,
    TRIG_SOURCE_ADC0_AMO,
    TRIG_SOURCE_ADC1_EOC,
    TRIG_SOURCE_ADC1_IEOC,
    TRIG_SOURCE_ADC1_AMO,/* idx is 50 */
    TRIG_SOURCE_PDT0_TRIG,
    TRIG_SOURCE_PDT0_PLUSE_OUT,
    TRIG_SOURCE_PDT1_TRIG,
    TRIG_SOURCE_PDT1_PLUSE_OUT,
    TRIG_SOURCE_RTC_ALARM_TRIG,
    TRIG_SOURCE_RTC_PRESCALER_TRIG,
    TRIG_SOURCE_EIO_TRIG0,
    TRIG_SOURCE_EIO_TRIG1,
    TRIG_SOURCE_EIO_TRIG2,
    TRIG_SOURCE_EIO_TRIG3,/* idx is 60 */
    TRIG_SOURCE_SW_TRIG0,
    TRIG_SOURCE_SW_TRIG1,
    TRIG_SOURCE_SW_TRIG2,
    TRIG_SOURCE_SW_TRIG3,
} Ctu_TriggerSourceType;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AC7843X_FEATURES_H */

/* =============================================  EOF  ============================================== */
