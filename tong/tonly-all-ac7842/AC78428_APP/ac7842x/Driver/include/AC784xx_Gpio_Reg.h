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
 * @file AC784xx_Gpio_Reg.h
 *
 * @brief This file provides gpio hardware integration interface.
 *
 */

/* PRQA S ALL -- */
/* PRQA S 0306 ++ */ /* Cast between a pointer to volatile object and an integral type. */

#ifndef AC784XX_GPIO_REG_H
#define AC784XX_GPIO_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/**
 * @brief Set the Pin control register.
 *
 * @details
 * This function writes the specified value to the Pin Control Register (PCR)
 * for the given port and channel. The PCR controls various aspects of the pin
 * including pull-up/pull-down resistors, drive strength, mux mode, and interrupt configuration.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] Value: Value to be written to the PCR register
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     PCR register value is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_301
 */
LOCAL_INLINE void Port_Reg_SetPCR(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    WRITE_REG32(Base->PCR[ChannelId], Value);
}

/**
 * @brief Get the Pin control register.
 *
 * @details
 * This function reads the current value of the Pin Control Register (PCR)
 * for the given port and channel. The PCR controls various aspects of the pin
 * including pull-up/pull-down resistors, drive strength, mux mode, and interrupt configuration.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 *
 * @return Current value of the PCR register
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_302
 */
LOCAL_INLINE uint32 Port_Reg_GetPCR(const uint8 PortId, uint32 ChannelId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->PCR[ChannelId]);
}

/**
 * @brief Set the up/down pull of pin.
 *
 * @details
 * This function configures the pull-up or pull-down resistor for a specific GPIO pin.
 * It modifies only the pull configuration bits (PU and PD) in the PCR register while
 * preserving other settings.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] Value: Pull configuration value to set
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     PU and PD bits in PCR register are updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_303
 */
LOCAL_INLINE void Port_Reg_SetPullConfig(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], (PORT_PCR_PU_Msk | PORT_PCR_PD_Msk), 0U, Value);
}

#if !defined(AC7840E)
/**
 * @brief Set the driver strength of pin.
 *
 * @details
 * This function configures the drive strength for a specific GPIO pin.
 * It modifies only the drive strength bit (DSE) in the PCR register while
 * preserving other settings.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] Value: Drive strength value to set
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     DSE bit in PCR register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_304
 */
LOCAL_INLINE void Port_Reg_SetDriverStrength(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], PORT_PCR_DSE_Msk, PORT_PCR_DSE_Pos, Value);
}
#endif

/**
 * @brief Set the Pin mux mode.
 *
 * @details
 * This function configures the pin multiplexing mode for a specific GPIO pin.
 * It determines whether the pin operates as a GPIO or as an alternative function pin.
 * It modifies only the mux mode bits (MUX) in the PCR register while preserving other settings.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] Value: Mux mode value to set
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     MUX bits in PCR register are updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_305
 */
LOCAL_INLINE void Port_Reg_SetPinMux(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], PORT_PCR_MUX_Msk, PORT_PCR_MUX_Pos, Value);
}

/**
 * @brief Set the Pin lock.
 *
 * @details
 * This function configures the pin locking feature for a specific GPIO pin.
 * When locked, the PCR register cannot be modified until the next reset.
 * It modifies only the lock bit (LK) in the PCR register while preserving other settings.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] Value: Lock value to set (0=unlock, 1=lock)
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     LK bit in PCR register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_306
 */
LOCAL_INLINE void Port_Reg_SetPinLock(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], PORT_PCR_LK_Msk, PORT_PCR_LK_Pos, Value);
}

/**
 * @brief Set the Pin interrupt mode.
 *
 * @details
 * This function configures the interrupt mode for a specific GPIO pin.
 * It determines the edge or level sensitivity for generating interrupts.
 * It modifies only the interrupt configuration bits (IRQC) in the PCR register
 * while preserving other settings.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] Value: Interrupt mode value to set
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     IRQC bits in PCR register are updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_307
 */
LOCAL_INLINE void Port_Reg_SetInterruptMode(uint8 PortId, uint32 ChannelId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (uint32)(0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], PORT_PCR_IRQC_Msk, PORT_PCR_IRQC_Pos, Value);
}

#if defined(AC7840X) || defined(AC7842X) || defined(AC7843X)
/**
 * @brief Clear the port control interrupt status flag register.
 *
 * @details
 * This function clears the interrupt status flag (ISF) for a specific GPIO pin.
 * Writing 1 to the ISF bit clears the flag, acknowledging the interrupt.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     ISF bit in PCR register is cleared
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_308
 */
LOCAL_INLINE void Port_Reg_ClearInterruptFlag(uint8 PortId, uint32 ChannelId)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->PCR[ChannelId], PORT_PCR_ISF_Msk, PORT_PCR_ISF_Pos, 1U);
}

/**
 * @brief get the port control interrupt status flag register.
 *
 * @details
 * This function reads the entire PCR register, including the interrupt status flag (ISF).
 * The ISF bit indicates if an interrupt condition has occurred on the pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 *
 * @return Current value of the PCR register
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_335
 */
LOCAL_INLINE uint32 Port_Reg_GetInterruptFlag(uint8 PortId, uint32 ChannelId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->PCR[ChannelId]);
}
#endif
/**
 * @brief Set the port interrupt status flag register.
 *
 * @details
 * This function sets/clears the interrupt status flags for multiple pins in a port.
 * Writing 1 to a bit position clears the corresponding pin's interrupt flag.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] Value: Each bit represents one Pin clear interrupt status clear(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31)
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *
 * @post
 *     ISFR register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_309
 */
LOCAL_INLINE void Port_Reg_SetISFR(uint8 PortId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    WRITE_REG32(Base->ISFR, Value);
}

/**
 * @brief Get the port interrupt status flag register.
 *
 * @details
 * This function reads the interrupt status flags for all pins in a port.
 * Each bit in the returned value represents the interrupt status of the corresponding pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 *
 * @return uint32  Each bit represents one Pin interrupt status interrupted(1) or no interrupt(0) (LSB is Pin 0, MSB is Pin 31)
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_310
 */
LOCAL_INLINE uint32 Port_Reg_GetISFR(const uint8 PortId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->ISFR);
}

/**
 * @brief Set the port digital filter enable register.
 *
 * @details
 * This function enables or disables the digital filter for a specific GPIO pin.
 * The digital filter helps reduce noise on the pin by requiring multiple samples
 * to detect a transition.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] ChannelId: Channel/pin number within the port (0-31)
 * @param[in] En: Enable/disable digital filter (TRUE=enable, FALSE=disable)
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     Corresponding bit in DFER register is updated
 *
 * @note
 *     Reentrancy: Reentrant for different PortId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_311
 */
LOCAL_INLINE void Port_Reg_EnableDigitalFilter(uint8 PortId, uint32 ChannelId, boolean En)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    MODIFY_REG32(Base->DFER, ((uint32)1U << ChannelId), ChannelId, En);
}

/**
 * @brief Set the port digital filter enable register.
 *
 * @details
 * This function configures the digital filter enable bits for multiple pins in a port.
 * Each bit in the value corresponds to a pin in the port, where 1 enables filtering
 * and 0 disables filtering.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] Value: Each bit represents one Pin digital filer enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31)
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *
 * @post
 *     DFER register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_312
 */
LOCAL_INLINE void Port_Reg_SetDFER(uint8 PortId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    WRITE_REG32(Base->DFER, Value);
}

/**
 * @brief Get the port digital filter enable register.
 *
 * @details
 * This function reads the current digital filter enable settings for all pins in a port.
 * Each bit in the returned value represents whether the digital filter is enabled
 * for the corresponding pin.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 *
 * @return uint32 Each bit represents one Pin digital filer enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_313
 */
LOCAL_INLINE uint32 Port_Reg_GetDFER(uint8 PortId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->DFER);
}

/**
 * @brief Set the port digital filter clock register.
 *
 * @details
 * This function configures the clock source for the digital filter in a port.
 * The digital filter uses this clock to sample the pin inputs and detect transitions.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] Value: port digital filter clock Value to set
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *
 * @post
 *     DFCR register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_314
 */
LOCAL_INLINE void Port_Reg_SetDFCR(uint8 PortId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    WRITE_REG32(Base->DFCR, Value);
}

/**
 * @brief Get the port digital filter clock register.
 *
 * @details
 * This function reads the current clock source configuration for the digital filter in a port.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 *
 * @return uint32
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_315
 */
LOCAL_INLINE uint32 Port_Reg_GetDFCR(uint8 PortId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->DFCR);
}

/**
 * @brief Set the port digital filter width register.
 *
 * @details
 * This function configures the width (number of samples) for the digital filter in a port.
 * A larger width provides better noise filtering but increases the propagation delay.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 * @param[in] Value: port digital filter width Value to set
 *
 * @return void
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *
 * @post
 *     DFWR register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_316
 */
LOCAL_INLINE void Port_Reg_SetDFWR(uint8 PortId, uint32 Value)
{
    PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    WRITE_REG32(Base->DFWR, Value);
}

/**
 * @brief Get the port digital filter width register.
 *
 * @details
 * This function reads the current width configuration for the digital filter in a port.
 *
 * @param[in] PortId: Port identifier (0-4 for PORTA-PORTE)
 *
 * @return uint32 digital filter width
 *
 * @pre
 *     PortId must correspond to a valid port instance
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different PortId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_317
 */
LOCAL_INLINE uint32 Port_Reg_GetDFWR(uint8 PortId)
{
    const PORT_Type *Base;

    Base = (PORT_Type *)(PORTA_BASE + (0x100U * (uint32)PortId));
    return READ_REG32(Base->DFWR);
}

/**
 * @brief write the port output data register
 *
 * @details
 * This function writes a value to the Port Output Data Register (PODR).
 * Each bit in the value corresponds to a pin in the port, where 1 sets the pin high
 * and 0 sets the pin low.
 *
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin output Value high(1) or low(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @return void
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     PODR register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_318
 */
LOCAL_INLINE void Gpio_Reg_SetPODR(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PODR, Value);
}

/**
 * @brief Get the port output data register
 *
 * @details
 * This function reads the current value of the Port Output Data Register (PODR).
 * Each bit in the returned value represents the current output state of the corresponding pin.
 *
 * @param[in] GpioId: Gpio id
 *
 * @return uint32 Each bit represents one Pin output Value high(1) or low(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_319
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPODR(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PODR);
}

/**
 * @brief Set the port set output data register
 *
 * @details
 * This function writes to the Port Set Output Register (PSOR).
 * Writing 1 to a bit position sets the corresponding pin high without affecting other pins.
 * Writing 0 has no effect.
 *
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin set Value set high(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @return void
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     Corresponding pins are set high according to Value
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_320
 */
LOCAL_INLINE void Gpio_Reg_SetPSOR(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PSOR, Value);
}

/**
 * @brief Get the port set output data register
 *
 * @details
 * This function reads the current value of the Port Set Output Register (PSOR).
 *
 * @param[in] GpioId: Gpio id
 *
 * @return uint32 Each bit represents one Pin set Value set high(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_321
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPSOR(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PSOR);
}

/**
 * @brief Set the port clear output data register
 *
 * @details
 * This function writes to the Port Clear Output Register (PROR).
 * Writing 1 to a bit position clears the corresponding pin (sets it low) without affecting other pins.
 * Writing 0 has no effect.
 *
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin reset Value reset(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @return void
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     Corresponding pins are cleared (set low) according to Value
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_322
 */
LOCAL_INLINE void Gpio_Reg_SetPROR(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PROR, Value);
}

/**
 * @brief Get the port clear output data register
 *
 * @details
 * This function reads the current value of the Port Clear Output Register (PROR).
 *
 * @param[in] GpioId: Gpio id
 *
 * @return uint32 Each bit represents one Pin reset Value reset(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_323
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPROR(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PROR);
}

/**
 * @brief Set the port toggle output data register
 *
 * @details
 * This function writes to the Port Toggle Output Register (PIOR).
 * Writing 1 to a bit position toggles the corresponding pin state without affecting other pins.
 * Writing 0 has no effect.
 *
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin toggle Value toggle(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @return void
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     Corresponding pins are toggled according to Value
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_324
 */
LOCAL_INLINE void Gpio_Reg_SetPIOR(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PIOR, Value);
}

/**
 * @brief Get the port toggle output data register
 *
 * @details
 * This function reads the current value of the Port Toggle Output Register (PIOR).
 *
 * @param[in] GpioId: Gpio id
 *
 * @return uint32 Each bit represents one Pin toggle Value toggle(1) or not effect(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_325
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPIOR(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PIOR);
}

/**
 * @brief Set the port input data register
 *
 * @details
 * This function writes to the Port Input Data Register (PIDR). Note that writing to
 * input registers typically has no effect on actual pin states, but might be used
 * for testing or simulation purposes.
 *
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin input Value high(1) or low(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @return void
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     PIDR register is updated with the provided Value (if writable)
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_326
 */
LOCAL_INLINE void Gpio_Reg_SetPIDR(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PIDR, Value);
}

/**
 * @brief Get the port input data register
 *
 * @details
 * This function reads the current value of the Port Input Data Register (PIDR).
 * Each bit in the returned value represents the current input state of the corresponding pin.
 *
 * @param[in] GpioId: Gpio id
 *
 * @return uint32 Each bit represents one Pin input Value high(1) or low(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_327
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPIDR(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PIDR);
}

/**
 * @brief Set the port ouput enable register
 *
 * @details
 * This function writes to the Port Output Enable Register (POER).
 * Each bit in the value corresponds to a pin in the port, where 1 enables output
 * for that pin and 0 disables output (input mode).
 *
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin output enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @return void
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     POER register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_328
 */
LOCAL_INLINE void Gpio_Reg_SetPOER(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->POER, Value);
}

/**
 * @brief Get the port output enable register
 *
 * @details
 * This function reads the current value of the Port Output Enable Register (POER).
 * Each bit in the returned value represents whether output is enabled for the corresponding pin.
 *
 * @param[in] GpioId: Gpio id
 *
 * @return uint32 Each bit represents one Pin output enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_329
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPOER(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->POER);
}

/**
 * @brief Set the port input enable register
 *
 * @details
 * This function writes to the Port Input Enable Register (PIER).
 * Each bit in the value corresponds to a pin in the port, where 1 enables input
 * for that pin and 0 disables input.
 *
 * @param[in] GpioId: Gpio id
 * @param[in] Value: the port Value to set.Each bit represents one Pin input enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @return void
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     PIER register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_330
 */
LOCAL_INLINE void Gpio_Reg_SetPIER(uint8 GpioId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    WRITE_REG32(Base->PIER, Value);
}

/**
 * @brief Get the port input enable register
 *
 * @details
 * This function reads the current value of the Port Input Enable Register (PIER).
 * Each bit in the returned value represents whether input is enabled for the corresponding pin.
 *
 * @param[in] GpioId: Gpio id
 *
 * @return uint32 Each bit represents one Pin input enable(1) or disable(0) (LSB is Pin 0, MSB is Pin 31).
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *
 * @post
 *     None
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId values
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_331
 */
LOCAL_INLINE uint32 Gpio_Reg_GetPIER(const uint8 GpioId)
{
    const GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    return READ_REG32(Base->PIER);
}

/**
 * @brief write the port input enable register
 *
 * @details
 * This function modifies a specific bit in the Port Input Enable Register (PIER).
 * It enables or disables input for a single pin without affecting other pins.
 *
 * @param[in] GpioId: Gpio id
 * @param[in] ChannelId: channel id of port
 * @param[in] Value: 1 means enable , 0 means disable
 *
 * @return void
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     Specified bit in PIER register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_332
 */
LOCAL_INLINE void Gpio_Reg_ModifyPIER(uint8 GpioId, uint32 ChannelId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    MODIFY_REG32(Base->PIER, ((uint32)1U << ChannelId), ChannelId, Value);
}

/**
 * @brief write the port output enable register
 *
 * @details
 * This function modifies a specific bit in the Port Output Enable Register (POER).
 * It enables or disables output for a single pin without affecting other pins.
 *
 * @param[in] GpioId: Gpio id
 * @param[in] ChannelId: channel id of port
 * @param[in] Value: 1 means enable , 0 means disable
 *
 * @return void
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     Specified bit in POER register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_333
 */
LOCAL_INLINE void Gpio_Reg_ModifyPOER(uint8 GpioId, uint32 ChannelId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    MODIFY_REG32(Base->POER, ((uint32)1U << ChannelId), ChannelId, Value);
}

/**
 * @brief write the port output data register
 *
 * @details
 * This function modifies a specific bit in the Port Output Data Register (PODR).
 * It sets or clears a single pin output without affecting other pins.
 *
 * @param[in] GpioId: Gpio id
 * @param[in] ChannelId: channel id of port
 * @param[in] Value: 1 means high level , 0 means low level
 *
 * @return void
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     Specified bit in PODR register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId/ChannelId combinations
 *     This function directly accesses hardware registers
 *     DES ID: DES_GPIO_API_334
 */
LOCAL_INLINE void Gpio_Reg_ModifyPODR(uint8 GpioId, uint32 ChannelId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    MODIFY_REG32(Base->PODR, ((uint32)1U << ChannelId), ChannelId, Value);
}

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
/**
 * @brief Set the port output type register
 *
 * @details
 * This function configures the output type for a specific GPIO pin.
 * It can set the pin to either push-pull or open-drain mode.
 *
 * @param[in] GpioId: Gpio id
 * @param[in] ChannelId: channel id of port
 * @param[in] Value:  0 means push-pull , 1 means open-drain.
 *
 * @return void
 *
 * @pre
 *     GpioId must correspond to a valid GPIO instance
 *     ChannelId must be less than the maximum number of pins per port
 *
 * @post
 *     Specified bit in OTYPER register is updated with the provided Value
 *
 * @note
 *     Reentrancy: Reentrant for different GpioId/ChannelId combinations
 *     This function is only available on AC7842X,AC7840E and AC7843X devices
 *     DES ID: DES_GPIO_API_336
 */
LOCAL_INLINE void Port_Reg_SetOTYPER(uint8 GpioId, uint32 ChannelId, uint32 Value)
{
    GPIO_Type *Base;

    Base = (GPIO_Type *)(GPIOA_BASE + (0x40U * (uint32)GpioId));
    MODIFY_REG32(Base->OTYPER, ((uint32)1U << ChannelId), ChannelId, Value);
}
#endif

/* =====================================  Functions definition  ===================================== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AC784XX_GPIO_REG_H*/

/* =============================================  EOF  ============================================== */
