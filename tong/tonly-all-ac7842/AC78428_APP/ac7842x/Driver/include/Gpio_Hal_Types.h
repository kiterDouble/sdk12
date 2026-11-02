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

/**
 * @file Gpio_HalTypes.h
 *
 * @brief This file provides extern Hal gpio type.
 *
 */

#ifndef GPIO_HAL_TYPES_H
#define GPIO_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*===============================================  INCLUDE FILES  ============================================*/
#include "Device_Register.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/**
 * @brief Type of a gpio channel.
 * @note DES_GPIO_VAR_001
 */
typedef uint32 Gpio_Hal_ChannelType;

/**
 * @brief Type of port interrupt callback.
 * @note DES_GPIO_VAR_002
 */
typedef void (*Port_CallbackType)(uint8 PortId, uint32 Status);

/**
 * @brief Type of a gpio pin levels.
 * @note DES_GPIO_TYP_001
 */
typedef enum
{
    GPIO_LOW_LEVEL = 0x0U,      /*!< General level 0.*/
    GPIO_HIGH_LEVEL = 0x1U,     /*!< General level 1.*/
    GPIO_MAX_LEVEL = 0x2U       /*!< General unspecified level. */
} Gpio_Hal_LevelType;

/**
 * @brief Type of gpio data direction.
 * @note DES_GPIO_TYP_002
 */
typedef enum
{
    GPIO_INPUT_DIRECTION = 0x0U,        /*!< General purpose input direction.*/
    GPIO_OUTPUT_DIRECTION = 0x1U,       /*!< General purpose output direction.*/
    GPIO_UNSPECIFIED_DIRECTION = 0x2U   /*!< General purpose unspecified direction. */
} Port_Hal_DataDirectionType;

/**
 * @brief Internal pull-up/down resistor.
 * @note DES_GPIO_TYP_003
 */
typedef enum
{
    PORT_INTERNAL_PULL_NOT_ENABLED = 0U,        /*!< internal pull-down or pull-up resistor is not enabled.*/
    PORT_INTERNAL_PULL_UP_ENABLED = 1U,         /*!< internal pull-up resistor is enabled. */
    PORT_INTERNAL_PULL_DOWN_ENABLED = 2U        /*!< internal pull-down resistor is enabled. */
} Port_Hal_PullConfigType;

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840X)
/**
 * @brief Configures the drive strength.
 * @note DES_GPIO_TYP_004
 */
typedef enum
{
    PORT_LOW_DRIVE_STRENGTH = 0U,       /*!< low drive strength  */
    PORT_HIGH_DRIVE_STRENGTH = 1U       /*!< high drive strength */
} Port_Hal_DriveStrengthType;
#endif
/**
 * @brief Configures the pin mux.
 * @note DES_GPIO_TYP_005
 */
typedef enum
{
    PORT_MUX_ALT0 = 0U,         /*!< gpio is disabled or as an analog funciton */
    PORT_MUX_AS_GPIO = 1U,      /*!< gpio is configured as GPIO*/
    PORT_MUX_ALT2 = 2U,         /*!< chip-specific*/
    PORT_MUX_ALT3 = 3U,         /*!< chip-specific*/
    PORT_MUX_ALT4 = 4U,         /*!< chip-specific*/
    PORT_MUX_ALT5 = 5U,         /*!< chip-specific*/
    PORT_MUX_ALT6 = 6U,         /*!< chip-specific*/
    PORT_MUX_ALT7 = 7U          /*!< chip-specific */
} Port_Hal_MuxType;

/**
 * @brief Configures the interrupt generation condition.
 * @note DES_GPIO_TYP_006
 */
typedef enum
{
    PORT_DMA_INT_DISABLED = 0x0U,   /*!< Interrupt/DMA request is disabled. */
    PORT_DMA_RISING_EDGE = 0x1U,    /*!< DMA request on rising edge.*/
    PORT_DMA_FALLING_EDGE = 0x2U,   /*!< DMA request on falling edge.*/
    PORT_DMA_EITHER_EDGE = 0x3U,    /*!< DMA request on either edge.*/
    PORT_INT_RISING_EDGE = 0x9U,    /*!< Interrupt on rising edge.*/
    PORT_INT_FALLING_EDGE = 0xAU,   /*!< Interrupt on falling edge.*/
    PORT_INT_EITHER_EDGE = 0xBU     /*!< Interrupt on either edge. */
} Port_Hal_InterruptConfigType;

/**
 * @brief Clock source for the digital input filters.
 * @note DES_GPIO_TYP_007
 */
typedef enum
{
    PORT_DIGITAL_FILTER_BUS_CLOCK = 0U,         /*!< Select bus clock */
    PORT_DIGITAL_FILTER_LSI_128K_CLOCK = 1U     /*!< Select LSI 128k clock */
} Port_Hal_DigitalFilterClockType;

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
/**
 * @brief Outpull otyper resistor.
 * @note DES_GPIO_TYP_008
 */
typedef enum
{
    GPIO_OUTPUT_PUSH_PULL = 0U,     /*!< otyp resistor is set push-pull. */
    GPIO_OUTPUT_OPEN_DRAIN = 1U,    /*!< otyp resistor is set open-drain. */
} Port_Hal_OutPutType;
#endif

/**
 * @brief The digital filter configuration.
 * @note DES_GPIO_TYP_009
 */
/*PRQA S 3630 ++ # AMCE_QAC_3630*/
typedef struct
{
    Port_Hal_DigitalFilterClockType Clock;  /*!< The digital filter clock for port */
    uint8 Width;                            /*!< The digital filter width value */
    uint32 PinMask;                         /*!< Mask of pins for which digital filter is enabled */
} Port_Hal_DigitalFilterCfgType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/

/**
 * @brief Defines the gpio configuration structure.
 * @note DES_GPIO_TYP_010
 */
/*PRQA S 3630 ++ # AMCE_QAC_3630*/
typedef struct
{
    uint8 PortId;                               /*!< Port index. */
    uint8 ChannelId;                            /*!< Pin index. */
    Port_Hal_PullConfigType PullConfig;         /*!< Internal pull-up/down resistor selection. */
#if !defined(AC7840E)
    Port_Hal_DriveStrengthType DriveStrength;   /*!< Configures the drive strength. */
#endif
    Port_Hal_MuxType Mux;                       /*!< Pin mux selection. */
    boolean PinLock;                            /*!< Lock pin control register or not.*/
    Port_Hal_InterruptConfigType IntConfig;     /*!< Interrupt generation condition.*/
    Port_Hal_DataDirectionType Direction;       /*!< Configures the port data direction.*/
    Gpio_Hal_LevelType InitValue;               /*!< Initial value*/
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
    Port_Hal_OutPutType OutputType;             /*!< Out Put Type*/
#endif
} Gpio_Hal_SettingsConfigType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/

/* ========================================== LOCAL VARIABLES =========================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* GPIO_HAL_TYPES_H */
