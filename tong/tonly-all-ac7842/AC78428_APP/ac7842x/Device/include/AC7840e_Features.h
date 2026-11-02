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
 * @file ac7840e_features.h
 *
 * @brief This file provides chip specific module features.
 *
 */

#ifndef AC7840E_FEATURES_H
#define AC7840E_FEATURES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*PRQA S 1535 EOF # AMCE_SYS_1535 */
/* =========================================================================================================================== */
/* ================                                            MCM                                            ================ */
/* =========================================================================================================================== */
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
} Core_InterruptType;

/*!
 * @brief Bist type.
 */
typedef enum
{
    BIST_ABIST,                       /*!< Analog BIST. */
    BIST_MBIST                        /*!< Memory BIST. */
} Core_BistType;

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
/* Number of instances of the CKGEN module */
#define CKGEN_INSTANCE_MAX              (1U)

/* HSI clock frequency(8MHz) */
#define CKGEN_HSI_FREQ                  (8000000U)

/* VHSI clock frequency(48MHz) */
#define CKGEN_VHSI_FREQ                 (48000000U)

/* Auto select HSI clock for SPLL when enable XOSC is fail */
#define CKGEN_AUTO_SEL_HSI              (0U)

/* Auto change SPLL reference clock to HSI clock when XOSC is detected loss(just for XOSC=8MHz);
     Auto change system clock to VHSI clock when SPLL is detected unlock */
#define CKGEN_AUTO_CHANGE_CLK           (0U)

/* Auto test clock */
#define CKGEN_AUTO_TEST_CLK             (1U)

/* timeout value to wait for value ready*/
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
#define CKGEN_SPLL_VCO_FREQ_MAX              (1500000000U)

/* spll vco clock minimum frequency. */
#define CKGEN_SPLL_VCO_FREQ_MIN              (500000000U)

/* spll reference clock maximum frequency. */
#define CKGEN_SPLL_IN_CLK_FREQ_MAX           (48000000U)

/* spll input clock minimum frequency. */
#define CKGEN_SPLL_IN_CLK_FREQ_MIN           (4000000U)

/* sys clock maximum frequency. */
#define CKGEN_SYS_CLK_FREQ_MAX               (120000000U)

/* spll reference clock maximum frequency. */
#define CKGEN_SPLL_REF_FREQ_MAX              (12000000U)

/* bus clock maximum frequency. */
#define CKGEN_BUS_CLK_FREQ_MAX               (60000000U)

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

/* register value indicates that it is PLL input is VHSI */
#define CKGEN_PLL_IN_VHSI                    (0U)

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

#define  CKGEN_PERIPH_CLK_DIV_ADDR_OFFSET     (0x4C)
#define  CKGEN_PERIPH_CLK_DIV_INDEX_OFFSET    (224U)

#define  CKGEN_CLK_OUT_CFG_ADDR_OFFSET        (0x48)
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
    CKGEN_SYS_CLK_MODE_MAX,
} Ckgen_SysClkModeType;

/*!
 * @brief Define the enum of clock output divider.
 */
typedef enum
{
    CKGEN_CLKOUT_DIV_BY_1 = 0U,     /*!< Divided by 1 */
    CKGEN_CLKOUT_DIV_BY_2 = 1U,     /*!< Divided by 2 */
    CKGEN_CLKOUT_DIV_BY_4 = 2U,     /*!< Divided by 4 */
    CKGEN_CLKOUT_DIV_BY_6 = 3U,     /*!< Divided by 6 */
    CKGEN_CLKOUT_DIV_BY_8 = 4U,     /*!< Divided by 8 */
    CKGEN_CLKOUT_DIV_BY_10 = 5U,     /*!< Divided by 10 */
    CKGEN_CLKOUT_DIV_BY_12 = 6U,     /*!< Divided by 12 */
    CKGEN_CLKOUT_DIV_BY_14 = 7U      /*!< Divided by 14 */
} Ckgen_ClkoutDivType;

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

typedef enum {
    /************          PERI_CLK_MUX0          ************/
    CKGEN_I2C0_CLK      = CKGEN_PERIPH_CLK_MUX0_INDEX_OFFSET + CKGEN_PERI_CLK_MUX0_I2C0_MUX_Pos,
    CKGEN_I2C1_CLK      = CKGEN_PERIPH_CLK_MUX0_INDEX_OFFSET + CKGEN_PERI_CLK_MUX0_I2C1_MUX_Pos,
    CKGEN_TIMER_CLK     = CKGEN_PERIPH_CLK_MUX0_INDEX_OFFSET + CKGEN_PERI_CLK_MUX0_TIMER_MUX_Pos,
    CKGEN_SPI0_CLK      = CKGEN_PERIPH_CLK_MUX0_INDEX_OFFSET + CKGEN_PERI_CLK_MUX0_SPI0_MUX_Pos,
    CKGEN_SPI1_CLK      = CKGEN_PERIPH_CLK_MUX0_INDEX_OFFSET + CKGEN_PERI_CLK_MUX0_SPI1_MUX_Pos,
    CKGEN_SPI2_CLK      = CKGEN_PERIPH_CLK_MUX0_INDEX_OFFSET + CKGEN_PERI_CLK_MUX0_SPI2_MUX_Pos,
    CKGEN_ADC0_CLK      = CKGEN_PERIPH_CLK_MUX0_INDEX_OFFSET + CKGEN_PERI_CLK_MUX0_ADC0_MUX_Pos,
    /************          PERI_CLK_MUX1          ************/
    CKGEN_CAN0_CLK      = CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET + CKGEN_PERI_CLK_MUX1_CAN0_MUX_Pos,
    CKGEN_CAN1_CLK      = CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET + CKGEN_PERI_CLK_MUX1_CAN1_MUX_Pos,
    CKGEN_EIO_CLK       = CKGEN_PERIPH_CLK_MUX1_INDEX_OFFSET + CKGEN_PERI_CLK_MUX1_EIO_MUX_Pos,
    /************          PERI_CLK_MUX2          ************/
    CKGEN_UART0_CLK     = CKGEN_PERIPH_CLK_MUX2_INDEX_OFFSET + CKGEN_PERI_CLK_MUX2_UART0_MUX_Pos,
    CKGEN_UART1_CLK     = CKGEN_PERIPH_CLK_MUX2_INDEX_OFFSET + CKGEN_PERI_CLK_MUX2_UART1_MUX_Pos,
    CKGEN_UART2_CLK     = CKGEN_PERIPH_CLK_MUX2_INDEX_OFFSET + CKGEN_PERI_CLK_MUX2_UART2_MUX_Pos,
    /************          PERI_CLK_MUX3          ************/
    CKGEN_PWM0_CLK      = CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET + CKGEN_PERI_CLK_MUX3_PWM0_EXT_Pos,
    CKGEN_PWM1_CLK      = CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET + CKGEN_PERI_CLK_MUX3_PWM1_EXT_Pos,
    CKGEN_PWM2_CLK      = CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET + CKGEN_PERI_CLK_MUX3_PWM2_EXT_Pos,
    CKGEN_PWM3_CLK      = CKGEN_PERIPH_CLK_MUX3_INDEX_OFFSET + CKGEN_PERI_CLK_MUX3_PWM3_EXT_Pos,
    /************          PERI_CLK_DIV          ************/
    CKGEN_CAN0_TS_CLK   = CKGEN_PERIPH_CLK_DIV_INDEX_OFFSET + CKGEN_PERI_CLK_DIV_CAN0_TS_DIV_Pos,
    CKGEN_CAN1_TS_CLK   = CKGEN_PERIPH_CLK_DIV_INDEX_OFFSET + CKGEN_PERI_CLK_DIV_CAN1_TS_DIV_Pos,
    CKGEN_TPIU_CLK      = CKGEN_PERIPH_CLK_DIV_INDEX_OFFSET + CKGEN_PERI_CLK_DIV_TPIU_DIV_Pos,
    /************          CLK_OUT_CFG          ************/
    CKGEN_CLK_OUT       = CKGEN_CLK_OUT_CFG_INDEX_OFFSET + CKGEN_CLK_OUT_CFG_MUX1_Pos,
    /************          CLK_DIV1          ************/
    CKGEN_SPLL_DIV1_CLK = CKGEN_CLK_DIV1_INDEX_OFFSET + CKGEN_CLK_DIV1_SPLL_DIV1_Pos,
    CKGEN_VHSI_DIV1_CLK = CKGEN_CLK_DIV1_INDEX_OFFSET + CKGEN_CLK_DIV1_VHSI_DIV1_Pos,
    CKGEN_VHSI_8M_DIV1_CLK  = CKGEN_CLK_DIV1_INDEX_OFFSET + CKGEN_CLK_DIV1_VHSI_8M_DIV1_Pos,
    CKGEN_HSI_DIV1_CLK      = CKGEN_CLK_DIV1_INDEX_OFFSET + CKGEN_CLK_DIV1_VHSI_8M_DIV1_Pos,
    CKGEN_HSE_DIV1_CLK  = CKGEN_CLK_DIV1_INDEX_OFFSET + CKGEN_CLK_DIV1_HSE_DIV1_Pos,
    /************          CLK_DIV2          ************/
    CKGEN_SPLL_DIV2_CLK = CKGEN_CLK_DIV2_INDEX_OFFSET + CKGEN_CLK_DIV2_SPLL_DIV2_Pos,
    CKGEN_VHSI_DIV2_CLK = CKGEN_CLK_DIV2_INDEX_OFFSET + CKGEN_CLK_DIV2_VHSI_DIV2_Pos,
    CKGEN_VHSI_8M_DIV2_CLK  = CKGEN_CLK_DIV2_INDEX_OFFSET + CKGEN_CLK_DIV2_VHSI_8M_DIV2_Pos,
    CKGEN_HSI_DIV2_CLK      = CKGEN_CLK_DIV2_INDEX_OFFSET + CKGEN_CLK_DIV2_VHSI_8M_DIV2_Pos,
    CKGEN_HSE_DIV2_CLK  = CKGEN_CLK_DIV2_INDEX_OFFSET + CKGEN_CLK_DIV2_HSE_DIV2_Pos,
    /************          CLK Name          ************/
    CKGEN_LSI_CLK = 352U,
    CKGEN_SYS_CLK,
    CKGEN_BUS_CLK,
    CKGEN_CORE_CLK,
    CKGEN_FLASH_CLK,
    CKGEN_HSE_CLK,
    CKGEN_VHSI_8M_CLK       = 358U,
    CKGEN_HSI_CLK           = 358U,
    CKGEN_VHSI_CLK,
    CKGEN_SPLL_CLK,
    CKGEN_SPLL1_CLK,
    CKGEN_LSI_1K_CLK ,
    CKGEN_LSI_32K_CLK,
    CKGEN_LSI_128K_CLK,
    CKGEN_PWM_EXT_CLK0,
    CKGEN_PWM_EXT_CLK1,
    CKGEN_PWM_EXT_CLK2,
    CKGEN_RTC_CLKIN,
    CKGEN_OFF_CLK,
} Ckgen_ClkIdType;

typedef enum {
    /************          PERI_CLK_EN0          ************/
    CKGEN_UART0_BUS_CLK = CKGEN_PERI_CLK_EN0_UART0_EN_Pos,
    CKGEN_UART1_BUS_CLK = CKGEN_PERI_CLK_EN0_UART1_EN_Pos,
    CKGEN_UART2_BUS_CLK = CKGEN_PERI_CLK_EN0_UART2_EN_Pos,
    CKGEN_SPI0_BUS_CLK = CKGEN_PERI_CLK_EN0_SPI0_EN_Pos,
    CKGEN_SPI1_BUS_CLK = CKGEN_PERI_CLK_EN0_SPI1_EN_Pos,
    CKGEN_SPI2_BUS_CLK = CKGEN_PERI_CLK_EN0_SPI2_EN_Pos,
    CKGEN_I2C0_BUS_CLK = CKGEN_PERI_CLK_EN0_I2C0_EN_Pos,
    CKGEN_I2C1_BUS_CLK = CKGEN_PERI_CLK_EN0_I2C1_EN_Pos,
    CKGEN_PWM0_BUS_CLK = CKGEN_PERI_CLK_EN0_PWM0_EN_Pos,
    CKGEN_PWM1_BUS_CLK = CKGEN_PERI_CLK_EN0_PWM1_EN_Pos,
    CKGEN_PWM2_BUS_CLK = CKGEN_PERI_CLK_EN0_PWM2_EN_Pos,
    CKGEN_PWM3_BUS_CLK = CKGEN_PERI_CLK_EN0_PWM3_EN_Pos,
    /************          PERI_CLK_EN1          ************/
    CKGEN_RTC_BUS_CLK = 32U + CKGEN_PERI_CLK_EN1_RTC_EN_Pos,
    CKGEN_DMA_BUS_CLK = 32U + CKGEN_PERI_CLK_EN1_DMA_EN_Pos,
    CKGEN_GPIO_BUS_CLK = 32U + CKGEN_PERI_CLK_EN1_GPIO_EN_Pos,
    CKGEN_WDG_BUS_CLK = 32U + CKGEN_PERI_CLK_EN1_WDG_EN_Pos,
    CKGEN_CRC_BUS_CLK = 32U + CKGEN_PERI_CLK_EN1_CRC_EN_Pos,
    CKGEN_CAN0_BUS_CLK = 32U + CKGEN_PERI_CLK_EN1_CAN0_EN_Pos,
    CKGEN_CAN1_BUS_CLK = 32U + CKGEN_PERI_CLK_EN1_CAN1_EN_Pos,
    /************          PERI_CLK_EN2          ************/
    CKGEN_CTU_BUS_CLK = 64U + CKGEN_PERI_CLK_EN2_CTU_EN_Pos,
    CKGEN_ACMP0_BUS_CLK = 64U + CKGEN_PERI_CLK_EN2_ACMP_EN_Pos,
    CKGEN_ADC0_BUS_CLK = 64U + CKGEN_PERI_CLK_EN2_ADC0_EN_Pos,
    CKGEN_TIMER_BUS_CLK = 64U + CKGEN_PERI_CLK_EN2_TIMER_EN_Pos,
    CKGEN_EIO_BUS_CLK = 64U + CKGEN_PERI_CLK_EN2_EIO_EN_Pos,
    CKGEN_SENT_BUS_CLK = 64U + CKGEN_PERI_CLK_EN2_SENT_EN_Pos,
    CKGEN_SMU_BUS_CLK = 64U + CKGEN_PERI_CLK_EN2_SMU_EN_Pos,
} Ckgen_BusClkIdType;

/* =========================================================================================================================== */
/* ================                                            MPU                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the MPU module */
#define MPU_INSTANCE_MAX           (1U)
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
#define MPU_HAS_PROCESS_IDENTIFIER (0U)

/*!< Array of mpu base addresses */
#define MPU_BASE_PTRS              {MPU}

typedef enum
{
    MPU_ID_0 = 0U, /*!< mpu module 0 */
    MPU_ID_MAX
} Mpu_IdType;

typedef enum
{
    MPU_MASTER_CORE     = 0U, /*!< The MPU Logical Bus Master Number for core bus master */
    MPU_MASTER_DEBUGGER = 1U, /*!< The MPU Logical Bus Master Number for Debugger master */
    MPU_MASTER_DMA      = 2U, /*!< The MPU Logical Bus Master Number for DMA master */
    MPU_MASTER_MAX
} Mpu_MasterType;

typedef enum
{
    MPU_SLAVE_FLASH     = 0U,
    MPU_SLAVE_SRAM_L    = 1U,
    MPU_SLAVE_SRAM_U    = 2U,
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
} Mpu_PermissionType;
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
    PBR_PERIPH_CRC = 8U,/*PACRB*/
    PBR_PERIPH_GPIO,
    PBR_PERIPH_MPU = 12U,
    PBR_PERIPH_EIM,
    PBR_PERIPH_SMU,
    PBR_PERIPH_HSM,
    PBR_PERIPH_CKGEN = 32U,/*PACRC*/
    PBR_PERIPH_FLASH = 34U,
    PBR_PERIPH_ADC0,
    PBR_PERIPH_ACMP0 = 37U,
    PBR_PERIPH_CTU,
    PBR_PERIPH_CAN0,
    PBR_PERIPH_CAN1 = 40U,/*PACRD*/
    PBR_PERIPH_SPM = 45U,
    PBR_PERIPH_RTC,
    PBR_PERIPH_EIO,
    PBR_PERIPH_WDG = 48U,/*PACRE*/
    PBR_PERIPH_SPI0 = 50U,
    PBR_PERIPH_SPI1,
    PBR_PERIPH_SPI2,
    PBR_PERIPH_I2C0 = 54U,
    PBR_PERIPH_I2C1,
    PBR_PERIPH_TIMER = 64U,/*PACRF*/
    PBR_PERIPH_DMA,
    PBR_PERIPH_UART0,
    PBR_PERIPH_UART1,
    PBR_PERIPH_UART2,
    PBR_PERIPH_CMU = 73U,/*PACRG*/
    PBR_PERIPH_SENT = 79U,
    PBR_PERIPH_MAX
} Pbr_PeriphIdType;
/* =========================================================================================================================== */
/* ================                                            CMU                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the CMU module */
#define CMU_INSTANCE_MAX           (4U)
/*!< Array of CMU base addresses */
#define CMU_BASE_PTRS              {CMU_VHSI, CMU_HSE, CMU_PLL, CMU_PLL1}

/* =========================================================================================================================== */
/* ================                                            RCM                                            ================ */
/* =========================================================================================================================== */
typedef enum
{
    RCM_RESET_ID_UART0 = 0U, /*!< Soft reset UART0                  */
    RCM_RESET_ID_UART1 = 1U, /*!< Soft reset UART1                  */
    RCM_RESET_ID_UART2 = 2U, /*!< Soft reset UART2                  */
    RCM_RESET_ID_SPI0 = 6U, /*!< Soft reset SPI0                    */
    RCM_RESET_ID_SPI1 = 7U, /*!< Soft reset SPI1                    */
    RCM_RESET_ID_SPI2 = 8U, /*!< Soft reset SPI2                    */
    RCM_RESET_ID_I2C0 = 9U, /*!< Soft reset I2C0                    */
    RCM_RESET_ID_I2C1 = 10U, /*!< Soft reset I2C1                   */
    RCM_RESET_ID_PWM0 = 15U, /*!< Soft reset PWM0                   */
    RCM_RESET_ID_PWM1 = 16U, /*!< Soft reset PWM1                   */
    RCM_RESET_ID_PWM2 = 17U, /*!< Soft reset PWM2                   */
    RCM_RESET_ID_PWM3 = 18U, /*!< Soft reset PWM3                   */

    RCM_RESET_ID_DMA = 34U, /*!< Soft reset DMA                     */
    RCM_RESET_ID_GPIO = 37U, /*!< Soft reset GPIO                   */
    RCM_RESET_ID_WDG = 38U, /*!< Soft reset WDG                     */
    RCM_RESET_ID_CRC = 40U, /*!< Soft reset CRC                     */
    RCM_RESET_ID_CAN0 = 41U, /*!< Soft reset CAN0                   */
    RCM_RESET_ID_CAN1 = 42U, /*!< Soft reset CAN1                   */

    RCM_RESET_ID_CTU = 65U, /*!< Soft reset CTU                     */
    RCM_RESET_ID_ACMP0 = 72U, /*!< Soft reset ACMP                  */
    RCM_RESET_ID_ADC0 = 75U, /*!< Soft reset ADC0                   */
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
#define SMU_CKGEN_CLOCKS           {CKGEN_SMU_BUS_CLK}
/*!< SMU unlock key */
#define SMU_UNLOCK_FIRST_VALUE     (0xA5U)
#define SMU_UNLOCK_SECOND_VALUE    (0x5AU)
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
    SMU_SWLF                /*!< Software latent fault types. */
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
    SMU_HWSPF_NULL_9,        /*!< Reserved NULL channel */
    SMU_HWSPF_NULL_10,       /*!< Reserved NULL channel */
    SMU_HWSPF_WDG,           /*!< Watchdog timeout */
    SMU_HWSPF_NULL_12,       /*!< Reserved NULL channel */
    SMU_HWSPF_NULL_13,       /*!< Reserved NULL channel */
    SMU_HWSPF_FLHLDO,        /*!< FLHLDO low voltage detection */
    SMU_HWSPF_CACHE,         /*!< Cache ECC error */
    SMU_HWSPF_CPU,           /*!< CPU Lock Up */
    SMU_HWSPF_FLEX_ECC,      /*!< Flex SRAM ECC error */
    SMU_HWSPF_CAN0_ECC,      /*!< CAN0 ECC error */
    SMU_HWSPF_CAN1_ECC,      /*!< CAN1 ECC error */
    SMU_HWSPF_NULL_20,       /*!< Reserved NULL channel */
    SMU_HWSPF_NULL_21,       /*!< Reserved NULL channel */
    SMU_HWSPF_DIGLDO,        /*!< DIGLDO low voltage detection */
    SMU_HWSPF_DLVD,          /*!< DLVD */
    SMU_HWSPF_DLVRB,         /*!< DLVRB */
    SMU_HWSPF_CMU_PLL1,      /*!< PLL1 Clock Monitor, PLL1 clock lost detected by CMU module */
    SMU_HWSPF_MAX            /*!< Max number of SMU supported hardward single point fault */
} Smu_HwSpfType;

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
    SMU_HWLF_MAX             /*!< Max number of SMU supported hardward latent fault */
} Smu_HwLfType;

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
} Smu_SwSpfType;

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
} Smu_SwLfType;

/* =========================================================================================================================== */
/* ================                                            ESM                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the ESM module */
/*!< Number of instances of the ESM module */
#define ESM_INSTANCE_COUNT        (1U)
/*!< Array of ESM IRQs */
#define ESM_IRQS                  {ECC_1BIT_ERROR_IRQn, ECC_2BIT_ERROR_IRQn}
/*!< 1 bit ESM IRQs */
#define ESM_1BIT_IRQS             ((uint32)ESM_CHANNEL0_1BIT_IRQ | (uint32)ESM_CHANNEL1_1BIT_IRQ)
/*!< 2 bits ESM IRQs */
#define ESM_2BIT_IRQS             ((uint32)ESM_CHANNEL0_2BIT_IRQ | (uint32)ESM_CHANNEL1_2BIT_IRQ)

/* =========================================================================================================================== */
/* ================                                            SRAM                                           ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the SRAM module */
#define SRAM_INSTANCE_COUNT        (1U)
/*!< Channel Number of SRAM module */
#define SRAM_CHANNEL_MAX           (2U)
/*!< Array of SRAM IRQs */
#define SRAM_IRQS                  {ECC_SRAM_1BIT_ERROR_IRQn, ECC_SRAM_2BIT_ERROR_IRQn}
/*!< SRAM L start address */
#define SRAM_L_START_ADDRESS       (0x1FFF8000U)
/*!< SRAM L end address */
#define SRAM_L_END_ADDRESS         (0x1FFFFFFFU)
/*!< SRAM U start address */
#define SRAM_U_START_ADDRESS       (0x20000000U)
/*!< SRAM U end address */
#define SRAM_U_END_ADDRESS         (0x20007FFFU)
/*!< SRAM retention size (1KB) */
#define SRAM_RETENTION_SIZE        (0x400U)
/*!
 * @brief ESM channel type.
 */
typedef enum
{
    ESM_CHANNEL_SRAML,                 /*!< Sram_L Channel. */
    ESM_CHANNEL_SRAMU,                 /*!< Sram_U Channel. */
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
} Esm_InterruptType;

/* =========================================================================================================================== */
/* ================                                            EIM                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the EIM module */
#define EIM_INSTANCE_COUNT         (1U)
/*!< Array of EIM base addresses */
#define EIM_BASE_PTRS              {EIM_CHANNEL0, EIM_CHANNEL1}

/*!
 * @brief EIM channel type.
 */
typedef enum
{
    EIM_CHANNEL_SRAML,                 /*!< Sram_L Channel. */
    EIM_CHANNEL_SRAMU,                 /*!< Sram_U Channel. */
    EIM_CHANNEL_MAX                    /*!< Channel Number of EIM module */
} Eim_ChannelType;

/* =========================================================================================================================== */
/* ================                                          FLASH                                            ================ */
/* =========================================================================================================================== */
/*!< FLASH controler unlock key */
#define FLASH_UNLOCK_KEY1          (0xac7842U)
#define FLASH_UNLOCK_KEY2          (0x01234567U)
/*!< Based address of P-FLASH */
#define PFLASH_BASE_ADDRESS        (0x00000000U)
/*!< Based address of D-FLASH */
#define DFLASH_BASE_ADDRESS        (0x01000000U)
/*!< P-Flash size in byte */
#define PFLASH_TOTAL_SIZE          (0x00090000U)
/*!< D-Flash size in byte */
#define DFLASH_TOTAL_SIZE          (0x00020000U)
/*!< P-Flash page size in byte */
#define PFLASH_PAGE_SIZE           (0x00000800U)
/*!< D-Flash page size in byte */
#define DFLASH_PAGE_SIZE           (0x00000400U)
/*!< Flash info base address */
#define FLASH_INFO_ADDR_BASE       (0x1022000U)
/*!< Flash info address range */
#define FLASH_INFO_ADDR_SIZE       (0x4000U)

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
#define PORT_INSTANCE_MAX          (5U)
/*!< Max number of the pin of GPIO module instances */
#define GPIO_PIN_MAX               (32U)
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
/*!< Number of instances of the CAN module */
#define CAN_INSTANCE_MAX           (2U)
/*!< Array of CAN base addresses */
#define CAN_BASE_PTRS              {CAN0, CAN1}
/*!< Array of CAN IRQs */
#define CAN_IRQS                   {CAN0_IRQn, CAN1_IRQn}
/*!< Array of CAN wakeup IRQs */
#define CAN_WAKEUP_IRQS            {CAN0_WAKEUP_IRQn, CAN1_WAKEUP_IRQn}
/*!< Array of CAN ckgen interface clocks */
#define CAN_CKGEN_CLOCKS           {CLK_CAN0, CLK_CAN1}
/*!< Array of CAN soft resets */
#define CAN_SOFT_RESETS            {SRST_CAN0, SRST_CAN1}
/*!< Array of CAN clock names */
#define CAN_CLOCK_NAMES            {CAN0_CLK, CAN1_CLK}
/*!< CAN receive fifo count */
#define CANFD_RECEIVE_FIFO_COUNT   (13U)
/*!< CAN transmit secondary buffer count (6 STB) */
#define CANFD_TRANSMIT_FIFO_COUNT  (6U)
/*!< CAN max filter number */
#define CANFD_FILTER_NUM_MAX       (60U)
/*!< CAN receive fifo count */
#define CAN_RECEIVE_FIFO_COUNT     (21U)
/*!< CAN transmit secondary buffer count (20 STB) */
#define CAN_TRANSMIT_FIFO_COUNT    (20U)
/*!< CAN max filter number */
#define CAN_FILTER_NUM_MAX         (31U)

/* =========================================================================================================================== */
/* ================                                           UART                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the UART module */
#define UART_INSTANCE_MAX          (3U)
/*!< Number of instances of the LIN module */
#define LIN_INSTANCE_MAX           (3U)
/*!< Array of UART base addresses */
#define UART_BASE_PTRS             {UART0, UART1, UART2}
/*!< Array of UART IRQs */
#define UART_IRQS                  {UART0_IRQn, UART1_IRQn, UART2_IRQn}
/*!< Array of UART ckgen interface clocks */
#define UART_CKGEN_CLOCKS          {CLK_UART0, CLK_UART1, CLK_UART2}
/*!< Array of UART soft resets */
#define UART_SOFT_RESETS           {SRST_UART0, SRST_UART1, SRST_UART2}
/*!< Array of UART clock names */
#define UART_CLOCK_NAMES           {UART0_CLK, UART1_CLK, UART2_CLK}

/* =========================================================================================================================== */
/* ================                                           SENT                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the SENT module */
#define SENT_INSTANCE_MAX          (2U)
/*!< Array of SENT base addresses */
#define SENT_BASE_PTRS             {SENT_CHANNEL0, SENT_CHANNEL1}
/*!< Array of SENT IRQs */
#define SENT_IRQS                  {SENT_CHANNEL0_IRQn, SENT_CHANNEL1_IRQn}
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
#define I2C_INSTANCE_MAX           (2U)
/*!< Array of I2C base addresses */
#define I2C_BASE_PTRS              {I2C0, I2C1}
/*!< Array of I2C IRQs */
#define I2C_IRQS                   {I2C0_IRQn, I2C1_IRQn}
/*!< Array of I2C ckgen interface clocks */
#define I2C_CKGEN_CLOCKS           {CLK_I2C0, CLK_I2C1}
/*!< Array of I2C soft resets */
#define I2C_SOFT_RESETS            {SRST_I2C0, SRST_I2C1}
/*!< Array of I2C clock names */
#define I2C_CLOCK_NAMES            {I2C0_CLK, I2C1_CLK}

/* =========================================================================================================================== */
/* ================                                           SPI                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the SPI module */
#define SPI_INSTANCE_MAX           (3U)
/*!< Array of SPI base addresses */
#define SPI_BASE_PTRS              {SPI0, SPI1, SPI2}
/*!< Array of SPI IRQs */
#define SPI_IRQS                   {SPI0_IRQn, SPI1_IRQn, SPI2_IRQn}
/*!< Array of SPI ckgen interface clocks */
#define SPI_CKGEN_CLOCKS           {CLK_SPI0, CLK_SPI1, CLK_SPI2}
/*!< Array of SPI soft resets */
#define SPI_SOFT_RESETS            {SRST_SPI0, SRST_SPI1, SRST_SPI2}
/*!< Array of SPI clock names */
#define SPI_CLOCK_NAMES            {SPI0_CLK, SPI1_CLK, SPI2_CLK}

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
/* ================                                            DMA                                            ================ */
/* =========================================================================================================================== */
#define DMA_TRANSFER_LENGTH_MAX             (524287U)
/*!< Number of instances of the DMA module */
#define DMA_INSTANCE_MAX           (1U)
/*!< Number of channel of the DMA module */
#define DMA_CH_MAX                 (16U)
/*!< Number of virtual channel of all DMA module */
#define DMA_VIRTUAL_CH_MAX         (20U)
/*!< Array of DMA channel base addresses */
#define DMA_VIRTUAL_CH_BASE_PTRS   {DMA0_CHANNEL0, DMA0_CHANNEL1, DMA0_CHANNEL2, DMA0_CHANNEL3, \
                                    DMA0_CHANNEL4, DMA0_CHANNEL5, DMA0_CHANNEL6, DMA0_CHANNEL7, \
                                    DMA0_CHANNEL8, DMA0_CHANNEL9, DMA0_CHANNEL10, DMA0_CHANNEL11, \
                                    DMA0_CHANNEL12, DMA0_CHANNEL13, DMA0_CHANNEL14, DMA0_CHANNEL15}
/*!< Array of DMA IRQs */
#define DMA_IRQS   {DMA0_CHANNEL0_IRQn, DMA0_CHANNEL1_IRQn, DMA0_CHANNEL2_IRQn, DMA0_CHANNEL3_IRQn, \
                    DMA0_CHANNEL4_IRQn, DMA0_CHANNEL5_IRQn, DMA0_CHANNEL6_IRQn, DMA0_CHANNEL7_IRQn, \
                    DMA0_CHANNEL8_IRQn, DMA0_CHANNEL9_IRQn, DMA0_CHANNEL10_IRQn, DMA0_CHANNEL11_IRQn, \
                    DMA0_CHANNEL12_IRQn, DMA0_CHANNEL13_IRQn, DMA0_CHANNEL14_IRQn, DMA0_CHANNEL15_IRQn}
/*!< Array of DMA ckgen interface clocks */
#define DMA_CKGEN_CLOCKS           {CLK_DMA0}
/*!< Array of DMA soft resets */
#define DMA_SOFT_RESETS            {SRST_DMA0}

#define DMA_ERROR_EVENT                     (DMA_CHANNEL_STATUS_DBE_Msk | DMA_CHANNEL_STATUS_SBE_Msk \
                                                        | DMA_CHANNEL_STATUS_DOE_Msk | DMA_CHANNEL_STATUS_SOE_Msk \
                                                        | DMA_CHANNEL_STATUS_DAE_Msk | DMA_CHANNEL_STATUS_SAE_Msk \
                                                        | DMA_CHANNEL_STATUS_CLE_Msk)

#define DMA_FINISH_EVENT                    (DMA_CHANNEL_STATUS_FINISH_Msk)
/*!
 * @brief DMA interrupt source type.
 */
typedef enum
{
    DMA_IRQ_NONE = 0U,                                                              /*!< DMA disable all interrupt. */
    DMA_FINISH_IRQ = DMA_CHANNEL_INTEN_FINISH_INTERRUPT_ENABLE_Msk,                 /*!< DMA finish interrupt. */
    DMA_ERROR_IRQ = DMA_CHANNEL_INTEN_TRANS_ERROR_INTERRUPT_ENABLE_Msk,             /*!< DMA error interrupt. */
} Dma_IrqSrcType;

/*!
 * @brief DMA request for the DMA channel.
 */
typedef enum
{
    DMA_REQ_DISABLE = 0U,
    DMA_REQ_UART0_RX = 1U,
    DMA_REQ_UART0_TX = 2U,
    DMA_REQ_UART1_RX = 3U,
    DMA_REQ_UART1_TX = 4U,
    DMA_REQ_UART2_RX = 5U,
    DMA_REQ_UART2_TX = 6U,
    DMA_REQ_EIO_SHIFTER0 = 13U,
    DMA_REQ_EIO_SHIFTER1 = 14U,
    DMA_REQ_EIO_SHIFTER2 = 15U,
    DMA_REQ_EIO_SHIFTER3 = 16U,
    DMA_REQ_PWM1_CHANNEL_0 = 17U,
    DMA_REQ_PWM1_CHANNEL_1 = 18U,
    DMA_REQ_PWM1_CHANNEL_2 = 19U,
    DMA_REQ_PWM1_CHANNEL_3 = 20U,
    DMA_REQ_PWM1_CHANNEL_4 = 21U,
    DMA_REQ_PWM1_CHANNEL_5 = 22U,
    DMA_REQ_PWM1_CHANNEL_6 = 23U,
    DMA_REQ_PWM1_CHANNEL_7 = 24U,
    DMA_REQ_PWM1_UNDER_OR_OVER_FLOW = 25U,
    DMA_REQ_PWM2_CHANNEL_0 = 26U,
    DMA_REQ_PWM2_CHANNEL_1 = 27U,
    DMA_REQ_PWM2_CHANNEL_2 = 28U,
    DMA_REQ_PWM2_CHANNEL_3 = 29U,
    DMA_REQ_PWM2_CHANNEL_4 = 30U,
    DMA_REQ_PWM2_CHANNEL_5 = 31U,
    DMA_REQ_PWM2_CHANNEL_6 = 32U,
    DMA_REQ_PWM2_CHANNEL_7 = 33U,
    DMA_REQ_PWM2_UNDER_OR_OVER_FLOW = 34U,
    DMA_REQ_PWM0_OR_CH0_CH7 = 35U,
    DMA_REQ_PWM0_UNDER_OR_OVER_FLOW = 36U,
    DMA_REQ_PWM3_OR_CH0_CH7 = 37U,
    DMA_REQ_PWM3_UNDER_OR_OVER_FLOW = 38U,
    DMA_REQ_HSM_RX = 43U,
    DMA_REQ_HSM_TX = 44U,
    DMA_REQ_SPI0_RX = 45U,
    DMA_REQ_SPI0_TX = 46U,
    DMA_REQ_SPI1_RX = 47U,
    DMA_REQ_SPI1_TX = 48U,
    DMA_REQ_SPI2_RX = 49U,
    DMA_REQ_SPI2_TX = 50U,
    DMA_REQ_ADC0 = 53U,
    DMA_REQ_I2C0_RX = 55U,
    DMA_REQ_I2C0_TX = 56U,
    DMA_REQ_I2C1_RX = 57U,
    DMA_REQ_I2C1_TX = 58U,
    DMA_REQ_PORTA = 59U,
    DMA_REQ_PORTB = 60U,
    DMA_REQ_PORTC = 61U,
    DMA_REQ_PORTD = 62U,
    DMA_REQ_PORTE = 63U,
    DMA_REQ_CAN0_RX = 64U,
    DMA_REQ_CAN1_RX = 65U,
    DMA_REQ_ALWAYS_ENABLED = 72U,
    DMA_REQ_CRC = 73U,
    DMA_REQ_MEM2MEM = 74U
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
} Dma_GlobalResetType;

/**
 * @brief DMA channel priority setting
 */
typedef enum
{
    DMA_DAISY_TRIGGER_FINISH = 0U,  /*!< 0: DMA priority low */
    DMA_DAISY_TRIGGER_HALF_FINISH = 1U,    /*!< 1: DMA priority medium */
    DMA_DAISY_TRIGGER_BOTH = 3U,  /*!< 3: DMA priority very high */
} Dma_DaisyTriggerType;

/* =========================================================================================================================== */
/* ================                                           ADC                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the ADC module */
#define ADC_INSTANCE_MAX           (1U)
/*!< Arrays of ADC base address */
#define ADC_BASE                   {ADC0_BASE}
/*!< Arrays of ADC base address ptrs*/
#define ADC_BASE_PTRS              {ADC0}
/*!< Arrays of ADC ckgen interface clocks */
#define ADC_CKGEN_CLOCKS           {CKGEN_ADC0_CLK}
/*!< Arrays of ADC soft resets */
#define ADC_SOFT_RESETS            {SRST_ADC0}
/*!< Array of ADC IRQs */
#define ADC_IRQS                   {ADC0_IRQn}
/*!< Array of ADC DMA requests */
#define ADC_DMA_REQEUSTS           {DMA_REQ_ADC0}
/*!< Array of ADC clock names */
#define ADC_CLOCK_NAMES            {ADC0_CLK}
/*!< Arrays of ADC ckgen interface bus clocks */
#define ADC_CKGEN_BUS_CLKS         {CKGEN_ADC0_BUS_CLK}
/*!< Max clock frequence of ADC function clock */
#define ADC_CLOCK_FREQ_MAX_RUNTIME (30000000U)
/*!< Array of ADC CTU triger modules [Instance][Type]: [0]=Regular, [0]=Injection */
#define ADC_CTU_TRIG_BASE          {{TRIG_SEL_ADC0_REGULAR0, TRIG_SEL_ADC0_INJECTION0}}
/*!< Number of ADC group modes (Regular and Inject) */
#define ADC_GROUP_MODE_COUNT       (2U)

/* =========================================================================================================================== */
/* ================                                           ACMP                                            ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the ACMP module */
#define ACMP_INSTANCE_MAX          (1U)
/*!< Arrays of ACMP base address */
#define ACMP_BASE_PTRS             {ACMP0}
/*!< Arrays of ACMP ckgen interface clocks */
#define ACMP_CKGEN_CLOCKS          {CLK_ACMP0}
/*!< Arrays of ACMP soft resets */
#define ACMP_SOFT_RESETS           {SRST_ACMP0}
/*!< Array of ACMP IRQs */
#define ACMP_IRQS                  {ACMP0_IRQn}

/* =========================================================================================================================== */
/* ================                                           PWM                                             ================ */
/* =========================================================================================================================== */
/*!< Number of instances of the PWM module */
#define PWM_INSTANCE_MAX           (4U)
/*!< Array of PWM base addresses */
#define PWM_BASE_PTRS              {PWM0, PWM1, PWM2, PWM3}
/*!< Array of PWM Overflow IRQs */
#define PWM_OVERFLOW_IRQS          {PWM0_OVERFLOW_IRQn, PWM1_OVERFLOW_IRQn, PWM2_OVERFLOW_IRQn, PWM3_OVERFLOW_IRQn}
/*!< Array of PWM Channel IRQs */
#define PWM_CHANNEL_IRQS           {PWM0_CHANNEL_IRQn, PWM1_CHANNEL_IRQn, PWM2_CHANNEL_IRQn, PWM3_CHANNEL_IRQn}
/*!< Array of PWM Fault IRQs */
#define PWM_FAULT_IRQS             {PWM0_FAULT_IRQn, PWM1_FAULT_IRQn, PWM2_FAULT_IRQn, PWM3_FAULT_IRQn}
/*!< Array of PWM ckgen interface clocks */
#define PWM_CKGEN_CLOCKS           {CLK_PWM0, CLK_PWM1, CLK_PWM2, CLK_PWM3}
/*!< Array of PWM soft resets */
#define PWM_SOFT_RESETS            {SRST_PWM0, SRST_PWM1, SRST_PWM2, SRST_PWM3}
/*!< Array of PWM operation mode */
#define PWM_OP_MODE                {PWM_MODE_MODULATION, PWM_MODE_MODULATION, PWM_MODE_MODULATION, PWM_MODE_MODULATION}

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
    CTU_UART1_RX_FILTER,
    CTU_UART0_TX_MODULATION,
    CTU_UART1_TX_MODULATION,
    CTU_PWM1_CH0_OUTSEL_ACMP0,
    CTU_PWM3_CH0_OUTSEL_ACMP0,
    CTU_PWM2_OUTSEL_HALL,
    CTU_PWM0_SYNC = 9U,
    CTU_PWM1_SYNC,
    CTU_PWM2_SYNC,
    CTU_PWM3_SYNC,
    CTU_PWM0_FAULT0 = 15U,
    CTU_PWM0_FAULT1,
    CTU_PWM0_FAULT2,
    CTU_PWM1_FAULT0 = 19U,
    CTU_PWM1_FAULT1,
    CTU_PWM1_FAULT2,
    CTU_PWM2_FAULT0 = 23U,
    CTU_PWM2_FAULT1,
    CTU_PWM2_FAULT2,
    CTU_PWM3_FAULT0 = 27U,
    CTU_PWM3_FAULT1,
    CTU_PWM3_FAULT2,
    CTU_RTC_CLK_CAP = 31U,
    CTU_PWM0_CH0_OUTSEL_PWM1_CH1 = 64U,
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
    CTU_PWM3_CH7_OUTSEL_PWM2_CH1
} Ctu_FuncIdType;

/*!
 * @brief Ctu software trigger id
 */
typedef enum
{
    CTU_SW_TRIG1 = 32U,     /*!< Trigger software 1 .*/
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
    TRIG_SEL_ACMP0 = 28U,
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
    TRIG_SEL_TIMER_CH0 = 64U,
    TRIG_SEL_TIMER_CH1,
    TRIG_SEL_TIMER_CH2,/* idx is 50 */
    TRIG_SEL_TIMER_CH3,
    TRIG_SEL_UART0 = 72U,
    TRIG_SEL_UART1 = 76U,
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
    TRIG_SOURCE_TIMER_CH0 = 16U,
    TRIG_SOURCE_TIMER_CH1,
    TRIG_SOURCE_TIMER_CH2,
    TRIG_SOURCE_TIMER_CH3,
    TRIG_SOURCE_PWM0_INIT_TRIG = 21U,
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
    TRIG_SOURCE_ADC0_EOC = 45U,
    TRIG_SOURCE_ADC0_IEOC,
    TRIG_SOURCE_ADC0_AMO,
    TRIG_SOURCE_RTC_ALARM_TRIG = 55U,
    TRIG_SOURCE_RTC_PRESCALER_TRIG,
    TRIG_SOURCE_EIO_TRIG0,
    TRIG_SOURCE_EIO_TRIG1,
    TRIG_SOURCE_EIO_TRIG2,
    TRIG_SOURCE_EIO_TRIG3,/* idx is 60 */
    TRIG_SOURCE_SW_TRIG0,
    TRIG_SOURCE_SW_TRIG1,
    TRIG_SOURCE_SW_TRIG2,
    TRIG_SOURCE_SW_TRIG3,
    TRIG_SOURCE_DMA_CH0,/* idx is 65 */
    TRIG_SOURCE_DMA_CH1,
    TRIG_SOURCE_DMA_CH2,
    TRIG_SOURCE_DMA_CH3,
} Ctu_TriggerSourceType;
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AC7840E_FEATURES_H */

/* =============================================  EOF  ============================================== */
