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
 * @file Conf_AC784xx.h
 *
 * @brief This file provides HAL configuration.
 *
 */

#ifndef CONF_AC784XX_H
#define CONF_AC784XX_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*==============================================INCLUDE FILES=======================================*/
#ifdef AUTOSAR_MCAL
#include "Mcal_Cfg.h"
#endif
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
#ifndef CONFIG_UART0_ENABLE
#define CONFIG_UART0_ENABLE     1
#endif

#ifndef CONFIG_UART1_ENABLE
#define CONFIG_UART1_ENABLE     1
#endif

#ifndef CONFIG_UART2_ENABLE
#define CONFIG_UART2_ENABLE     1
#endif

#ifndef CONFIG_UART3_ENABLE
#define CONFIG_UART3_ENABLE     0
#endif

#ifndef CONFIG_UART4_ENABLE
#define CONFIG_UART4_ENABLE     0
#endif

#ifndef CONFIG_UART5_ENABLE
#define CONFIG_UART5_ENABLE     0
#endif

#ifndef CONFIG_UART6_ENABLE
#define CONFIG_UART6_ENABLE     0
#endif

#ifndef CONFIG_UART7_ENABLE
#define CONFIG_UART7_ENABLE     0
#endif

#ifndef CONFIG_EUART0_ENABLE
#define CONFIG_EUART0_ENABLE     1
#endif

#ifndef CONFIG_EUART1_ENABLE
#define CONFIG_EUART1_ENABLE     1
#endif

#ifndef CONFIG_I2C0_ENABLE
#define CONFIG_I2C0_ENABLE      1
#endif

#ifndef CONFIG_I2C1_ENABLE
#define CONFIG_I2C1_ENABLE      1
#endif

#ifndef CONFIG_I2C2_ENABLE
#define CONFIG_I2C2_ENABLE      1
#endif

#ifndef CONFIG_SPI0_ENABLE
#define CONFIG_SPI0_ENABLE      1
#endif

#ifndef CONFIG_SPI1_ENABLE
#define CONFIG_SPI1_ENABLE      1
#endif

#ifndef CONFIG_SPI2_ENABLE
#define CONFIG_SPI2_ENABLE      1
#endif

#ifndef CONFIG_SPI3_ENABLE
#define CONFIG_SPI3_ENABLE      1
#endif

#ifndef CONFIG_SPI4_ENABLE
#define CONFIG_SPI4_ENABLE      1
#endif

#ifndef CONFIG_LIN0_ENABLE
#define CONFIG_LIN0_ENABLE      0
#endif

#ifndef CONFIG_LIN1_ENABLE
#define CONFIG_LIN1_ENABLE      0
#endif

#ifndef CONFIG_LIN2_ENABLE
#define CONFIG_LIN2_ENABLE      0
#endif

#ifndef CONFIG_LIN3_ENABLE
#define CONFIG_LIN3_ENABLE      0
#endif

#ifndef CONFIG_LIN4_ENABLE
#define CONFIG_LIN4_ENABLE      0
#endif

#ifndef CONFIG_LIN5_ENABLE
#define CONFIG_LIN5_ENABLE      0
#endif

#ifndef CONFIG_LIN6_ENABLE
#define CONFIG_LIN6_ENABLE      0
#endif

#ifndef CONFIG_LIN7_ENABLE
#define CONFIG_LIN7_ENABLE      0
#endif

#ifndef CONFIG_CAN0_ENABLE
#define CONFIG_CAN0_ENABLE      1
#endif

#ifndef CONFIG_CAN1_ENABLE
#define CONFIG_CAN1_ENABLE      1
#endif

#ifndef CONFIG_CAN2_ENABLE
#define CONFIG_CAN2_ENABLE      1
#endif

#ifndef CONFIG_CAN3_ENABLE
#define CONFIG_CAN3_ENABLE      1
#endif

#ifndef CONFIG_EI2S0_ENABLE
#define CONFIG_EI2S0_ENABLE      1
#endif

#ifndef CONFIG_EI2S1_ENABLE
#define CONFIG_EI2S1_ENABLE      1
#endif

#if defined (AC7842X) || defined (AC7843X)
#ifndef CONFIG_CAN4_ENABLE
#define CONFIG_CAN4_ENABLE      1
#endif

#ifndef CONFIG_CAN5_ENABLE
#define CONFIG_CAN5_ENABLE      1
#endif
#endif

#ifndef CONFIG_DMA_CHANNEL0_ENABLE
#define CONFIG_DMA_CHANNEL0_ENABLE      1
#endif

#ifndef CONFIG_DMA_CHANNEL1_ENABLE
#define CONFIG_DMA_CHANNEL1_ENABLE      1
#endif

#ifndef CONFIG_DMA_CHANNEL2_ENABLE
#define CONFIG_DMA_CHANNEL2_ENABLE      1
#endif

#ifndef CONFIG_DMA_CHANNEL3_ENABLE
#define CONFIG_DMA_CHANNEL3_ENABLE      1
#endif

#ifndef CONFIG_DMA_CHANNEL4_ENABLE
#define CONFIG_DMA_CHANNEL4_ENABLE      1
#endif

#ifndef CONFIG_DMA_CHANNEL5_ENABLE
#define CONFIG_DMA_CHANNEL5_ENABLE      1
#endif

#ifndef CONFIG_DMA_CHANNEL6_ENABLE
#define CONFIG_DMA_CHANNEL6_ENABLE      1
#endif

#ifndef CONFIG_DMA_CHANNEL7_ENABLE
#define CONFIG_DMA_CHANNEL7_ENABLE      1
#endif

#ifndef CONFIG_DMA_CHANNEL8_ENABLE
#define CONFIG_DMA_CHANNEL8_ENABLE      1
#endif

#ifndef CONFIG_DMA_CHANNEL9_ENABLE
#define CONFIG_DMA_CHANNEL9_ENABLE      1
#endif

#ifndef CONFIG_DMA_CHANNEL10_ENABLE
#define CONFIG_DMA_CHANNEL10_ENABLE     1
#endif

#ifndef CONFIG_DMA_CHANNEL11_ENABLE
#define CONFIG_DMA_CHANNEL11_ENABLE     1
#endif

#ifndef CONFIG_DMA_CHANNEL12_ENABLE
#define CONFIG_DMA_CHANNEL12_ENABLE     1
#endif

#ifndef CONFIG_DMA_CHANNEL13_ENABLE
#define CONFIG_DMA_CHANNEL13_ENABLE     1
#endif

#ifndef CONFIG_DMA_CHANNEL14_ENABLE
#define CONFIG_DMA_CHANNEL14_ENABLE     1
#endif

#ifndef CONFIG_DMA_CHANNEL15_ENABLE
#define CONFIG_DMA_CHANNEL15_ENABLE     1
#endif

#ifndef CONFIG_SENT_CHANNEL0_ENABLE
#define CONFIG_SENT_CHANNEL0_ENABLE     1
#endif

#ifndef CONFIG_SENT_CHANNEL1_ENABLE
#define CONFIG_SENT_CHANNEL1_ENABLE     1
#endif

#ifndef CONFIG_SENT_CHANNEL2_ENABLE
#define CONFIG_SENT_CHANNEL2_ENABLE     1
#endif

#ifndef CONFIG_SENT_CHANNEL3_ENABLE
#define CONFIG_SENT_CHANNEL3_ENABLE     1
#endif

#ifndef IRQ_CONTROL_IN_CORE_HAL
#define IRQ_CONTROL_IN_CORE_HAL         1
#endif

#ifndef IRQ_CONTROL_IN_ESM_HAL
#define IRQ_CONTROL_IN_ESM_HAL          0
#endif

#ifndef IRQ_CONTROL_IN_SMU_HAL
#define IRQ_CONTROL_IN_SMU_HAL          0
#endif

#ifndef CRITICAL_TIME_STATISTIC_EN
#define CRITICAL_TIME_STATISTIC_EN    0U
#endif

#ifndef CONFIG_EEP_MAX_SIZE_KB
#define CONFIG_EEP_MAX_SIZE_KB          (4U)
#endif

/* EEP record integrity check (CRC16 poly 0x90D9 over address + data) for the
 * combine-format chips (AC7840E/42X/43X). On by default for AC7840E; pre-define
 * EEP_CRC16_CHECK_ENABLE to force it on for any combine-format chip. */
#if !defined (EEP_CRC16_CHECK_ENABLE) && defined (AC7840E)
#define EEP_CRC16_CHECK_ENABLE
#endif

#ifdef AUTOSAR_MCAL
#ifndef CAN_SDK_NON_EXTENDED_API
#define CAN_SDK_NON_EXTENDED_API
#endif

#ifndef EEP_SDK_NON_EXTENDED_API
#define EEP_SDK_NON_EXTENDED_API
#endif

#ifndef CRC_SDK_NON_EXTENDED_API
#define CRC_SDK_NON_EXTENDED_API
#endif

#ifndef ADC_SDK_NON_EXTENDED_API
#define ADC_SDK_NON_EXTENDED_API
#endif

#ifndef GPIO_SDK_NON_EXTENDED_API
#define GPIO_SDK_NON_EXTENDED_API
#endif

#ifndef I2C_SDK_NON_EXTENDED_API
#define I2C_SDK_NON_EXTENDED_API
#endif

#ifndef SPI_SDK_NON_EXTENDED_API
#define SPI_SDK_NON_EXTENDED_API
#endif

#ifndef LIN_SDK_NON_EXTENDED_API
#define LIN_SDK_NON_EXTENDED_API
#endif

#ifndef UART_SDK_NON_EXTENDED_API
#define UART_SDK_NON_EXTENDED_API
#endif

#ifndef ACMP_SDK_NON_EXTENDED_API
#define ACMP_SDK_NON_EXTENDED_API
#endif

#ifndef SENT_SDK_NON_EXTENDED_API
#define SENT_SDK_NON_EXTENDED_API
#endif

#ifndef CSE_SDK_NON_EXTENDED_API
#define CSE_SDK_NON_EXTENDED_API
#endif

#ifndef PWM_SDK_NON_EXTENDED_API
#define PWM_SDK_NON_EXTENDED_API
#endif

#ifndef FLS_SDK_NON_EXTENDED_API
#define FLS_SDK_NON_EXTENDED_API
#endif

#ifndef WDG_SDK_NON_EXTENDED_API
#define WDG_SDK_NON_EXTENDED_API
#endif

#ifndef GPT_SDK_NON_EXTENDED_API
#define GPT_SDK_NON_EXTENDED_API
#endif

#ifndef SW_CRC_SUPPORT
#define SW_CRC_SUPPORT
#endif

#else /*AUTOSAR_MCAL*/
#if defined(AC7840X)
/* eep save physical sram address, off by default */
#ifndef CONFIG_EEP_SAVE_PHY_ADDR
#define CONFIG_EEP_SAVE_PHY_ADDR     0U
#endif
#endif

#endif /*AUTOSAR_MCAL*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CONF_AC784XX_H */

/* =============================================  EOF  ============================================== */
