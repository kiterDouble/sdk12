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
 * @file Gpio_Hal.h
 *
 * @brief This file contains the GPIO HAL interface APIs and definitions.
 *
 */

#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================  INCLUDE FILES  =========================================== */
#include "AC784xx_Gpio_Reg.h"
#include "Gpio_Hal_Types.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/** @def PORTID_A
 *  @brief Port A identifier
 *  @note DES_GPIO_MACRO_004
 */
#define PORTID_A            (0U)

/** @def PORTID_B
 *  @brief Port B identifier
 *  @note DES_GPIO_MACRO_005
 */
#define PORTID_B            (1U)

/** @def PORTID_C
 *  @brief Port C identifier
 *  @note DES_GPIO_MACRO_006
 */
#define PORTID_C            (2U)

/** @def PORTID_D
 *  @brief Port D identifier
 *  @note DES_GPIO_MACRO_007
 */
#define PORTID_D            (3U)

/** @def PORTID_E
 *  @brief Port E identifier
 *  @note DES_GPIO_MACRO_008
 */
#define PORTID_E            (4U)

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== EXPORTED FUNCTIONS ============================================== */

/**
 * @brief Initialize GPIO channels according to configuration settings.
 *
 * @details
 * This function initializes multiple GPIO pins based on the provided configuration array.
 * It configures pin direction, initial value, pull resistors, drive strength, mux mode,
 * interrupt configuration, and pin locking. For pins configured as GPIO (PORT_MUX_AS_GPIO),
 * it sets the direction and initial value.
 *
 * @param[in] PinCount: Number of pins to configure
 * @param[in] Config: Pointer to configuration array
 *
 * @return void
 *
 * @pre
 *     PinCount must not exceed PIN_NUM_MAX
 *     All configuration parameters must be valid
 *
 * @post
 *     GPIO pins are configured according to the provided settings
 *
 * @note
 *     Reentrancy: Non-reentrant
 *     This function must be called during system initialization
 *     DES ID: DES_GPIO_API_001
 */
void Gpio_Hal_ChannelInit(uint8 PinCount, const Gpio_Hal_SettingsConfigType Config[]);

/**
 * @brief Set pull resistor configuration for a specific GPIO pin.
 *
 * @details
 * This function configures the internal pull-up or pull-down resistor for a specific GPIO pin.
 * The pull configuration can be disabled, pull-up enabled, or pull-down enabled.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] PullConfig: Pull configuration setting
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *
 * @post
 *     Pull resistor configuration is updated in hardware register
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     DES ID: DES_GPIO_API_002
 */
void Gpio_Hal_SetPullSel(uint8 PortId, uint8 ChannelId, Port_Hal_PullConfigType PullConfig);

/**
 * @brief Set pin multiplexing mode for a specific GPIO pin.
 *
 * @details
 * This function configures the pin multiplexing mode for a specific GPIO pin,
 * allowing selection between GPIO functionality and alternate functions.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] Mux: Multiplexing mode selection
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *     Mux must be a valid multiplexing mode
 *
 * @post
 *     Pin multiplexing mode is updated in hardware register
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     DES ID: DES_GPIO_API_003
 */
void Gpio_Hal_SetMuxMode(uint8 PortId, uint8 ChannelId, Port_Hal_MuxType Mux);

/**
 * @brief Configure interrupt settings for a specific GPIO pin.
 *
 * @details
 * This function sets the interrupt configuration for a specific GPIO pin,
 * including enabling/disabling interrupts and selecting trigger conditions.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] IntConfig: Interrupt configuration setting
 *
 * @return
 *     STATUS_SUCCESS: Configuration successful
 *     STATUS_ERROR: Invalid configuration parameter
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *
 * @post
 *     Interrupt configuration is updated in hardware register
 *     Corresponding port interrupt is enabled
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     DES ID: DES_GPIO_API_004
 */
Hal_StatusType Gpio_Hal_SetPinIntSel(uint8 PortId, uint8 ChannelId, Port_Hal_InterruptConfigType IntConfig);

/**
 * @brief Get interrupt configuration for a specific GPIO pin.
 *
 * @details
 * This function retrieves the current interrupt configuration for a specific GPIO pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 *
 * @return Current interrupt configuration setting
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant
 *     DES ID: DES_GPIO_API_005
 */
Port_Hal_InterruptConfigType Gpio_Hal_GetPinIntSel(uint8 PortId, uint8 ChannelId);

#ifndef GPIO_SDK_NON_EXTENDED_API
/**
 * @brief Enable or disable digital filter for a specific GPIO pin.
 *
 * @details
 * This function enables or disables the digital filter for a specific GPIO pin
 * to reduce noise and glitches on the signal.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] En: Enable flag (TRUE to enable, FALSE to disable)
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *
 * @post
 *     Digital filter enable state is updated in hardware register
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     DES ID: DES_GPIO_API_006
 */
void Gpio_Hal_EnableDigitalFilter(uint8 PortId, uint8 ChannelId, boolean En);
#endif

/**
 * @brief Configure digital filter settings for a GPIO port.
 *
 * @details
 * This function configures the digital filter settings for a GPIO port,
 * including clock source, filter width, and which pins have filtering enabled.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] Config: Pointer to digital filter configuration structure
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     Config must point to a valid configuration structure
 *
 * @post
 *     Digital filter settings are applied to the port
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     DES ID: DES_GPIO_API_007
 */
void Gpio_Hal_ConfigDigitalFilter(uint8 PortId, const Port_Hal_DigitalFilterCfgType *Config);

#ifndef GPIO_SDK_NON_EXTENDED_API
/**
 * @brief Get the direction of a specific GPIO pin.
 *
 * @details
 * This function retrieves the current direction configuration of a specific GPIO pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 *
 * @return Current direction of the pin
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant
 *     DES ID: DES_GPIO_API_008
 */
Port_Hal_DataDirectionType Gpio_Hal_GetPinDirection(uint8 PortId, uint8 ChannelId);
#endif

/**
 * @brief Get the direction of all pins in a GPIO port.
 *
 * @details
 * This function retrieves the current direction configuration of all pins in a GPIO port.
 * Each bit in the returned value represents the direction of the corresponding pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 *
 * @return Bitmask representing the direction of all pins in the port
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant
 *     DES ID: DES_GPIO_API_009
 */
Gpio_Hal_ChannelType Gpio_Hal_GetPinsDirection(uint8 PortId);

/**
 * @brief Set the direction of a specific GPIO pin.
 *
 * @details
 * This function sets the direction of a specific GPIO pin to input, output, or unspecified.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] Direction: Direction to set for the pin
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *     Direction must be a valid direction value
 *
 * @post
 *     Pin direction is updated in hardware registers
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     DES ID: DES_GPIO_API_010
 */
void Gpio_Hal_SetPinDirection(uint8 PortId, uint8 ChannelId, Port_Hal_DataDirectionType Direction);

#ifndef GPIO_SDK_NON_EXTENDED_API
/**
 * @brief Set the direction of multiple GPIO pins in a port.
 *
 * @details
 * This function sets the direction of multiple GPIO pins in a port using a bitmask.
 * Each bit in the PinsDir parameter corresponds to a pin in the port.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] PinsDir: Bitmask specifying direction for each pin (1=output, 0=input)
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *
 * @post
 *     Direction of specified pins is updated in hardware registers
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     DES ID: DES_GPIO_API_011
 */
void Gpio_Hal_SetPinsDirection(uint8 PortId, uint32 PinsDir);
#endif

/**
 * @brief Write a logic level to a specific GPIO pin.
 *
 * @details
 * This function sets the output level of a specific GPIO pin to high or low.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] Value: Logic level to write (GPIO_LOW_LEVEL or GPIO_HIGH_LEVEL)
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *     Value must be a valid logic level
 *
 * @post
 *     Pin output level is updated in hardware register
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     DES ID: DES_GPIO_API_012
 */
void Gpio_Hal_WritePin(uint8 PortId, uint8 ChannelId, Gpio_Hal_LevelType Value);

/**
 * @brief Write logic levels to multiple GPIO pins in a port.
 *
 * @details
 * This function sets the output levels of multiple GPIO pins in a port using a bitmask.
 * Each bit in the Pins parameter corresponds to a pin in the port.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] Pins: Bitmask specifying output level for each pin (1=high, 0=low)
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *
 * @post
 *     Output levels of specified pins are updated in hardware register
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     DES ID: DES_GPIO_API_013
 */
void Gpio_Hal_WritePins(uint8 PortId, Gpio_Hal_ChannelType Pins);

#ifndef GPIO_SDK_NON_EXTENDED_API
/**
 * @brief Read the logic level of a specific GPIO pin.
 *
 * @details
 * This function reads the current input level of a specific GPIO pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 *
 * @return Current logic level of the pin
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant
 *     DES ID: DES_GPIO_API_014
 */
Gpio_Hal_LevelType Gpio_Hal_ReadPin(uint8 PortId, uint8 ChannelId);
#endif

/**
 * @brief Read the logic levels of all GPIO pins in a port.
 *
 * @details
 * This function reads the current input levels of all GPIO pins in a port.
 * Each bit in the returned value represents the logic level of the corresponding pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 *
 * @return Bitmask representing the logic levels of all pins in the port
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant
 *     DES ID: DES_GPIO_API_015
 */
Gpio_Hal_ChannelType Gpio_Hal_ReadPins(uint8 PortId);

#ifndef GPIO_SDK_NON_EXTENDED_API
/**
 * @brief Toggle the logic level of a specific GPIO pin.
 *
 * @details
 * This function toggles the output level of a specific GPIO pin.
 * If the pin was high, it becomes low; if it was low, it becomes high.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *
 * @post
 *     Pin output level is toggled in hardware register
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     DES ID: DES_GPIO_API_016
 */
void Gpio_Hal_TogglePin(uint8 PortId, uint8 ChannelId);
#endif

/**
 * @brief Toggle the logic levels of multiple GPIO pins in a port.
 *
 * @details
 * This function toggles the output levels of multiple GPIO pins in a port using a bitmask.
 * Each bit in the Pins parameter corresponds to a pin in the port.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] Pins: Bitmask specifying which pins to toggle
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *
 * @post
 *     Output levels of specified pins are toggled in hardware register
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     DES ID: DES_GPIO_API_017
 */
void Gpio_Hal_TogglePins(uint8 PortId, Gpio_Hal_ChannelType Pins);

#ifndef GPIO_SDK_NON_EXTENDED_API
/**
 * @brief Get the current output level of a specific GPIO pin.
 *
 * @details
 * This function retrieves the current output level setting of a specific GPIO pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 *
 * @return Current output level of the pin
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant
 *     DES ID: DES_GPIO_API_018
 */
Gpio_Hal_LevelType Gpio_Hal_GetPinOutputLevel(uint8 PortId, uint8 ChannelId);
#endif

/**
 * @brief Get the current output levels of all GPIO pins in a port.
 *
 * @details
 * This function retrieves the current output level settings of all GPIO pins in a port.
 * Each bit in the returned value represents the output level of the corresponding pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 *
 * @return Bitmask representing the output levels of all pins in the port
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant
 *     DES ID: DES_GPIO_API_019
 */
Gpio_Hal_ChannelType Gpio_Hal_GetPinsOutputLevel(uint8 PortId);

#ifndef GPIO_SDK_NON_EXTENDED_API
/**
 * @brief Set a GPIO pin to high impedance state.
 *
 * @details
 * This function configures a GPIO pin to high impedance state by disabling
 * both input and output drivers, setting the pin multiplexing to GPIO mode,
 * and disabling pull resistors.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] En: Enable flag (TRUE to set high-Z, FALSE to restore input)
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *
 * @post
 *     Pin is configured to high impedance state or restored to input
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     DES ID: DES_GPIO_API_020
 */
void Gpio_Hal_SetHighZ(uint8 PortId, uint8 ChannelId, boolean En);
#endif

/**
 * @brief Install an interrupt callback function for a GPIO port.
 *
 * @details
 * This function registers a callback function to be called when an interrupt
 * occurs on a specific GPIO port. The callback function will receive the
 * port identifier and interrupt status as parameters.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] Function: Pointer to the callback function
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     Function must be a valid function pointer
 *
 * @post
 *     Callback function is registered for the specified port
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     DES ID: DES_GPIO_API_021
 */
void Gpio_Hal_InstallCallback(uint8 PortId, const Port_CallbackType Function);

/**
 * @brief Clear the interrupt status flag for a specific GPIO pin.
 *
 * @details
 * This function clears the interrupt status flag for a specific GPIO pin,
 * acknowledging that the interrupt has been handled.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *
 * @post
 *     Interrupt status flag is cleared
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     DES ID: DES_GPIO_API_022
 */
void Gpio_Hal_ClearIntStatus(uint8 PortId, uint8 ChannelId);

#ifndef GPIO_SDK_NON_EXTENDED_API
#if defined(AC7840X) || defined(AC7842X) || defined(AC7843X)
/**
 * @brief Get the interrupt status flag for a specific GPIO pin.
 *
 * @details
 * This function retrieves the current interrupt status flag for a specific GPIO pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 *
 * @return Interrupt status flag value (0=not pending, 1=pending)
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *     ChannelId must be less than PORT_PCR_COUNT
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant
 *     DES ID: DES_GPIO_API_023
 */
uint8 Gpio_Hal_GetIntStatus(uint8 PortId, uint8 ChannelId);
#endif
/**
 * @brief Clear the output levels of multiple GPIO pins in a port.
 *
 * @details
 * This function clears (sets to low) the output levels of multiple GPIO pins
 * in a port using a bitmask. Each bit in the Pins parameter corresponds to a pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] Pins: Bitmask specifying which pins to clear
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *
 * @post
 *     Output levels of specified pins are set to low
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     DES ID: DES_GPIO_API_024
 */
void Gpio_Hal_ClearPins(uint8 PortId, Gpio_Hal_ChannelType Pins);

/**
 * @brief Set the output levels of multiple GPIO pins in a port.
 *
 * @details
 * This function sets (to high) the output levels of multiple GPIO pins
 * in a port using a bitmask. Each bit in the Pins parameter corresponds to a pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] Pins: Bitmask specifying which pins to set
 *
 * @return void
 *
 * @pre
 *     PortId must be less than PORT_INSTANCE_MAX
 *
 * @post
 *     Output levels of specified pins are set to high
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     DES ID: DES_GPIO_API_025
 */
void Gpio_Hal_SetPins(uint8 PortId, Gpio_Hal_ChannelType Pins);
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* GPIO_HAL_H */

/* =============================================  EOF  ============================================== */
