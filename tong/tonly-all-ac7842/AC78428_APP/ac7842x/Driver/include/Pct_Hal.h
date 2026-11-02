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
 * @file Pct_Hal.h
 *
 * @brief This file provides pct functions interface.
 *
 */

/* PRQA S 3630 EOF */ /* AMCE_QAC_3630 */

#ifndef PCT_HAL_H
#define PCT_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "AC784xx_Pct_Reg.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/
#define PCT_HAL_MODULE_ID                    (44U)
#define PCT_HAL_SW_MAJOR_VERSION             (1U)
#define PCT_HAL_SW_MINOR_VERSION             (0U)
#define PCT_HAL_SW_PATCH_VERSION             (0U)

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */
/**
 * @brief Pulse input selection enumeration for PCT in pulse counter mode.
 *
 * @note DES_PCT_TYP_001
 */
typedef enum
{
    PCT_PINSELECT_TRGMUX = 0x00U, /*!< Pulse from TRGMUX trigger */
    PCT_PINSELECT_ALT1 = 0x01U,   /*!< Pulse from pin 1          */
    PCT_PINSELECT_ALT2 = 0x02U,   /*!< Pulse from pin 2          */
    PCT_PINSELECT_ALT3 = 0x03U    /*!< Pulse from pin 3          */
} Pct_PinSelectType;

/**
 * @brief Pulse count polarity enumeration for PCT pulse counter mode.
 *
 * @note DES_PCT_TYP_002
 */
typedef enum
{
    PCT_PINPOLARITY_RISING = 0U,  /*!< Count pulse on rising edge  */
    PCT_PINPOLARITY_FALLING = 1U  /*!< Count pulse on falling edge */
} Pct_PinPolarityType;

/**
 * @brief PCT operating mode enumeration.
 *
 * @note DES_PCT_TYP_003
 */
typedef enum
{
    PCT_MODE_TIMER = 0U,       /*!< Time counter mode      */
    PCT_MODE_PULSECOUNTER = 1U /*!< Pulse counter mode     */
} Pct_ModeType;

/**
 * @brief Prescaler and glitch filter selection enumeration for PCT.
 *
 * @details
 * In timer mode, configures the prescaler divider.
 * In pulse counter mode, configures the glitch filter width.
 *
 * @note DES_PCT_TYP_004
 */
typedef enum
{
    PCT_PRESCALE_2 = 0x00U,                        /*!< Time mode: prescaler 2, Filter mode: invalid          */
    PCT_PRESCALE_4_GLITCHFILTER_2 = 0x01U,         /*!< Time mode: prescaler 4, Filter mode: 2 clocks         */
    PCT_PRESCALE_8_GLITCHFILTER_4 = 0x02U,         /*!< Time mode: prescaler 8, Filter mode: 4 clocks         */
    PCT_PRESCALE_16_GLITCHFILTER_8 = 0x03U,        /*!< Time mode: prescaler 16, Filter mode: 8 clocks        */
    PCT_PRESCALE_32_GLITCHFILTER_16 = 0x04U,       /*!< Time mode: prescaler 32, Filter mode: 16 clocks       */
    PCT_PRESCALE_64_GLITCHFILTER_32 = 0x05U,       /*!< Time mode: prescaler 64, Filter mode: 32 clocks       */
    PCT_PRESCALE_128_GLITCHFILTER_64 = 0x06U,      /*!< Time mode: prescaler 128, Filter mode: 64 clocks      */
    PCT_PRESCALE_256_GLITCHFILTER_128 = 0x07U,     /*!< Time mode: prescaler 256, Filter mode: 128 clocks     */
    PCT_PRESCALE_512_GLITCHFILTER_256 = 0x08U,     /*!< Time mode: prescaler 512, Filter mode: 256 clocks     */
    PCT_PRESCALE_1024_GLITCHFILTER_512 = 0x09U,    /*!< Time mode: prescaler 1024, Filter mode: 512 clocks    */
    PCT_PRESCALE_2048_GLITCHFILTER_1024 = 0x0AU,   /*!< Time mode: prescaler 2048, Filter mode: 1024 clocks   */
    PCT_PRESCALE_4096_GLITCHFILTER_2048 = 0x0BU,   /*!< Time mode: prescaler 4096, Filter mode: 2048 clocks   */
    PCT_PRESCALE_8192_GLITCHFILTER_4096 = 0x0CU,   /*!< Time mode: prescaler 8192, Filter mode: 4096 clocks   */
    PCT_PRESCALE_16384_GLITCHFILTER_8192 = 0x0DU,  /*!< Time mode: prescaler 16384, Filter mode: 8192 clocks  */
    PCT_PRESCALE_32768_GLITCHFILTER_16384 = 0x0EU, /*!< Time mode: prescaler 32768, Filter mode: 16384 clocks */
    PCT_PRESCALE_65536_GLITCHFILTER_32768 = 0x0FU  /*!< Time mode: prescaler 65536, Filter mode: 32768 clocks */
} Pct_PreScalerType;

/**
 * @brief PCT compare value read precision selection enumeration.
 *
 * @details
 * Controls the minimum ratio between BUSCLK and FUNCTION_CLK for safe
 * compare value reading to avoid coherency issues.
 *
 * @note DES_PCT_TYP_005
 */
typedef enum
{
    PCT_BCLK_FCLK_4 = 0x00U,      /*!< BUSCLK/FUNCTION CLK >= 4        */
    PCT_BCLK_FCLK_3 = 0x01U,      /*!< BUSCLK/FUNCTION CLK >= 3        */
    PCT_BCLK_FCLK_1 = 0x02U,      /*!< BUSCLK/FUNCTION CLK >= 1        */
} Pct_CVALSelectType;

/**
 * @brief Clock source selection enumeration for PCT.
 *
 * @note DES_PCT_TYP_006
 */
typedef enum
{
    PCT_CLOCKSOURCE_HSIDIV2 = 0x00U,  /*!< HSIDIV2 clock        */
    PCT_CLOCKSOURCE_LSI1K = 0x01U,    /*!< LSI1K clock          */
    PCT_CLOCKSOURCE_RTC = 0x02U,      /*!< RTC clock            */
    PCT_CLOCKSOURCE_PCC = 0x03U,       /*!< PCC configured clock */
    PCT_CLOCK_MAX
} Pct_ClkSrcType;

/**
 * @brief PCT counter units enumeration.
 *
 * @details
 * Defines the units used for the PCT compare value:
 * - TICKS: Raw counter ticks
 * - MICROSECONDS: Time in microseconds
 *
 * @note DES_PCT_TYP_007
 */
typedef enum
{
    PCT_COUNTER_UNITS_TICKS = 0x00U,
    PCT_COUNTER_UNITS_MICROSECONDS = 0x01U
} Pct_CounterUnitsType;

/**
 * @brief Configuration structure for PCT module.
 *
 * @details
 * Contains all parameters required to configure the PCT module in either
 * timer or pulse counter mode.
 *
 * @note DES_PCT_TYP_008
 */
typedef struct
{
    boolean InterruptEnable;             /*!< Enable/Disable interrupt on compare match  */
    boolean FreeRun;                     /*!< Enable/Disable free running mode           */
    Pct_ModeType Mode;                   /*!< Timer or pulse counter mode                */
    Pct_ClkSrcType ClkSrc;               /*!< Clock source selection                     */
    Pct_PreScalerType PreScl;            /*!< Prescaler and filter selection             */
    Pct_CVALSelectType CVALSel;          /*!< Compare value read precision selection     */
    boolean BypassPrescaler;             /*!< Enable/Disable prescaler bypass            */
    uint32 CompareValue;                 /*!< Compare value in selected units             */
    Pct_CounterUnitsType CounterUnits;   /*!< Counter units (ticks or microseconds)      */
    Pct_PinSelectType PinSel;            /*!< Pin selection for pulse counter mode       */
    Pct_PinPolarityType PinPolarity;     /*!< Pin polarity for pulse counter mode        */
    Hal_CallbackType Callback;           /*!< Callback function for compare interrupt    */
} Pct_ConfigType;

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */
/**
 * @brief Initializes the PCT module.
 *
 * @details
 * Enables the PCT peripheral clock and configures the PCT module according
 * to the provided configuration structure. Sets the initial state to IDLE.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 * @param[in] Config: Pointer to the PCT configuration structure
 *
 * @return void
 *
 * @pre
 *     The PCT module must not be already initialized.
 *     The Config pointer must be valid and non-NULL.
 *
 * @post
 *     PCT module is initialized and ready for operation.
 *     Peripheral clock is enabled.
 *
 * @note
 *     This function must be called before any other PCT_Hal functions.
 *     Initialization sets the module state to IDLE.
 *     DES_PCT_API_201
 */
void Pct_Hal_Init(const uint8 Instance, const Pct_ConfigType *const Config);

/**
 * @brief Configures the PCT module with new parameters.
 *
 * @details
 * Applies new configuration settings to the PCT module including mode selection,
 * clock source, prescaler, and interrupt settings. The module must be in IDLE
 * state and not running.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 * @param[in] Config: Pointer to the PCT configuration structure
 *
 * @return Hal_StatusType
 *     - STATUS_SUCCESS: Configuration successfully applied
 *     - STATUS_ERROR: Configuration failed (e.g., invalid free run mode settings)
 *
 * @pre
 *     The PCT module must be initialized via Pct_Hal_Init().
 *     The module must not be running (status == PCT_IDLE).
 *     The Config pointer must be valid and non-NULL.
 *
 * @post
 *     PCT hardware registers are updated with new configuration.
 *     Interrupt may be enabled/disabled based on configuration.
 *
 * @note
 *     Free run mode is not compatible with prescaler enabled.
 *     DES_PCT_API_202
 */
Hal_StatusType Pct_Hal_SetConfig(const uint8 Instance, const Pct_ConfigType *const Config);

/**
 * @brief De-initializes the PCT module.
 *
 * @details
 * Disables the PCT interrupt and peripheral clock, returning the module to
 * uninitialized state. The module must be in IDLE state.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 *
 * @return void
 *
 * @pre
 *     The PCT module must be initialized.
 *     The module must not be running (status == PCT_IDLE).
 *
 * @post
 *     PCT peripheral clock is disabled.
 *     Interrupt is disabled.
 *     Module state is set to UNINITED.
 *
 * @note
 *     After de-initialization, Pct_Hal_Init() must be called again before using the module.
 *     DES_PCT_API_203
 */
void Pct_Hal_Deinit(const uint8 Instance);

/**
 * @brief Sets the compare value in counter tick units.
 *
 * @details
 * Updates the PCT compare value using raw counter ticks. The module must be
 * in IDLE state. The new value takes effect immediately.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 * @param[in] Value: The compare value in counter ticks (16-bit unsigned)
 *
 * @return void
 *
 * @pre
 *     The PCT module must be initialized and in IDLE state.
 *
 * @post
 *     PCT compare value register is updated.
 *
 * @note
 *     DES_PCT_API_204
 */
void Pct_Hal_SetCompareValueByCount(const uint8 Instance, const uint16 Value);

/**
 * @brief Gets the compare value in counter tick units.
 *
 * @details
 * Reads the current PCT compare value from the hardware register and returns
 * it in raw counter ticks.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 * @param[out] Value: Pointer to store the compare value in counter ticks
 *
 * @return void
 *
 * @pre
 *     The PCT module must be initialized.
 *     The Value pointer must be valid and non-NULL.
 *
 * @post
 *     Value contains the current compare value in ticks.
 *
 * @note
 *     DES_PCT_API_205
 */
void Pct_Hal_GetCompareValueByCount(const uint8 Instance, uint16 *const Value);

/**
 * @brief Sets the compare value in microseconds.
 *
 * @details
 * Converts the provided microsecond value to counter ticks based on the current
 * clock frequency and prescaler settings, then updates the compare value register.
 * The module must be in IDLE state and configured in timer mode.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 * @param[in] Value: Compare value in microseconds (32-bit unsigned)
 *
 * @return Hal_StatusType
 *     - STATUS_SUCCESS: Conversion and update successful
 *     - STATUS_ERROR: Value exceeds maximum counter range
 *
 * @pre
 *     The PCT module must be initialized in timer mode.
 *     The module must be in IDLE state.
 *
 * @post
 *     PCT compare value register is updated with converted tick count.
 *
 * @note
 *     Conversion accuracy depends on clock frequency and prescaler configuration.
 *     DES_PCT_API_206
 */
Hal_StatusType Pct_Hal_SetCompareValueByUs(const uint8 Instance, const uint32 Value);

/**
 * @brief Gets the compare value in microseconds.
 *
 * @details
 * Reads the current PCT compare value and converts it from counter ticks to
 * microseconds based on clock frequency and prescaler settings.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 * @param[out] Value: Pointer to store the compare value in microseconds
 *
 * @return void
 *
 * @pre
 *     The PCT module must be initialized in timer mode.
 *     The Value pointer must be valid and non-NULL.
 *
 * @post
 *     Value contains the current compare value in microseconds.
 *
 * @note
 *     Conversion accuracy depends on clock frequency and prescaler configuration.
 *     DES_PCT_API_207
 */
void Pct_Hal_GetCompareValueByUs(const uint8 Instance, uint32 *const Value);

/**
 * @brief Gets the PCT compare match interrupt flag.
 *
 * @details
 * Reads the hardware compare flag bit to determine if a compare event has occurred.
 * The flag is set when the counter matches the compare value.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 *
 * @return boolean
 *     - TRUE: Compare match has occurred
 *     - FALSE: No compare match
 *
 * @pre
 *     The PCT module must be initialized.
 *
 * @post
 *     No hardware state changes (read-only operation).
 *
 * @note
 *     The flag is automatically cleared by the hardware on counter reload or
 *     can be manually cleared via Pct_Hal_ClearCompareFlag().
 *     DES_PCT_API_208
 */
boolean Pct_Hal_GetCompareFlag(const uint8 Instance);

/**
 * @brief Clears the PCT compare match interrupt flag.
 *
 * @details
 * Writes a 1 to the hardware compare flag bit to clear any pending compare
 * match interrupt. This must be called after handling the compare event.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 *
 * @return void
 *
 * @pre
 *     The PCT module must be initialized.
 *     A compare match should have occurred.
 *
 * @post
 *     The compare flag bit is cleared in hardware.
 *
 * @note
 *     This function should be called in the compare interrupt handler or in
 *     polling mode after detecting a compare match.
 *     DES_PCT_API_209
 */
void Pct_Hal_ClearCompareFlag(const uint8 Instance);

/**
 * @brief Gets the PCT module operational status.
 *
 * @details
 * Returns the current state of the PCT module indicating whether it is running,
 * idle, or in an error state.
 *
 * @return Hal_StatusType
 *     - STATUS_BUSY: PCT is running
 *     - STATUS_IDLE: PCT is initialized but not running
 *     - STATUS_ERROR: PCT is in an undefined or error state
 *
 * @pre
 *     None
 *
 * @post
 *     No hardware state changes (read-only operation).
 *
 * @note
 *     DES_PCT_API_210
 */
Hal_StatusType Pct_Hal_GetStatus(void);

/**
 * @brief Enables or disables the PCT compare match interrupt.
 *
 * @details
 * Controls whether the PCT compare match event generates an interrupt signal.
 * When enabled, the compare match triggers an interrupt request.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 * @param[in] Enable: Interrupt enable flag
 *     - TRUE: Enable compare match interrupt
 *     - FALSE: Disable compare match interrupt
 *
 * @return void
 *
 * @pre
 *     The PCT module must be initialized.
 *
 * @post
 *     PCT interrupt enable bit in hardware is updated.
 *     If enabled, compare matches will generate interrupt signals.
 *
 * @note
 *     The interrupt must be enabled in the NVIC separately for actual interrupt handling.
 *     DES_PCT_API_211
 */
void Pct_Hal_SetInterrupt(const uint8 Instance, const boolean Enable);

/**
 * @brief Gets the current PCT counter value.
 *
 * @details
 * Reads the real-time counter value from the PCT hardware. This represents
 * the current count in counter tick units.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 *
 * @return uint16
 *     The current counter value in ticks (16-bit unsigned integer)
 *
 * @pre
 *     The PCT module must be initialized.
 *
 * @post
 *     No hardware state changes (read-only operation).
 *
 * @note
 *     This value increments based on the configured clock source and prescaler.
 *     When in free run mode, the counter wraps at 0xFFFF.
 *     DES_PCT_API_212
 */
uint16 Pct_Hal_GetCurrentValue(const uint8 Instance);

/**
 * @brief Starts the PCT counter.
 *
 * @details
 * Enables the counter by setting the enable bit in the control status register.
 * The module must be in IDLE state. Once started, the counter increments based
 * on the configured clock source.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 *
 * @return void
 *
 * @pre
 *     The PCT module must be initialized and in IDLE state.
 *
 * @post
 *     PCT counter is enabled and incrementing.
 *     Module state changes to RUNNING.
 *
 * @note
 *     The counter can be stopped with Pct_Hal_Stop().
 *     DES_PCT_API_213
 */
void Pct_Hal_Start(const uint8 Instance);

/**
 * @brief Stops the PCT counter.
 *
 * @details
 * Disables the counter by clearing the enable bit in the control status register.
 * The module must be in RUNNING state. Stops counter incrementation and freezes
 * the current counter value.
 *
 * @param[in] Instance: PCT instance number (0 or 1)
 *
 * @return void
 *
 * @pre
 *     The PCT module must be running (state == RUNNING).
 *
 * @post
 *     PCT counter is disabled and stopped.
 *     Module state changes to IDLE.
 *     Counter value is frozen at its current value.
 *
 * @note
 *     The counter can be restarted with Pct_Hal_Start().
 *     DES_PCT_API_214
 */
void Pct_Hal_Stop(const uint8 Instance);

/**
 * @brief Installs the PCT compare interrupt callback function.
 *
 * @details
 * Registers a user-defined callback function that will be invoked when a
 * PCT compare match interrupt occurs. The callback is called from the PCT
 * interrupt service routine.
 *
 * @param[in] Func: Pointer to the callback function. Set to NULL to disable callbacks.
 *     Callback signature: void (*Hal_CallbackType)(void *Args)
 *
 * @return void
 *
 * @pre
 *     The PCT module should be initialized before installing a callback.
 *
 * @post
 *     The callback function is stored and will be called on compare interrupts.
 *
 * @note
 *     The callback function must be non-blocking and execute quickly.
 *     Interrupt must be enabled separately via Pct_Hal_EnableInterrupt().
 *     DES_PCT_API_215
 */
void Pct_Hal_InstallCallback(const Hal_CallbackType Func);

ISR(PCT_IRQHandler);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PCT_HAL_H */
/* =============================================  EOF  ============================================== */
