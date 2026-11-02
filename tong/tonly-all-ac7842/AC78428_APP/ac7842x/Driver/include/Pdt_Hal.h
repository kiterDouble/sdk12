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
 * AutoChips Inc. (C) 2021. All rights reserved.
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
 * @file Pdt_Hal.h
 *
 * @brief This file provides PDT integration functions interface.
 *
 */

/* PRQA S 3630 EOF */ /* AMCE_QAC_3630 */

#ifndef PDT_HAL_H
#define PDT_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "AC784xx_Pdt_Reg.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/
#define PDT_HAL_MODULE_ID                    (43U)
#define PDT_HAL_SW_MAJOR_VERSION             (1U)
#define PDT_HAL_SW_MINOR_VERSION             (0U)
#define PDT_HAL_SW_PATCH_VERSION             (0U)

#define  PDT_HARDWARE_TRIGGER  (0U)               /*!< Select hardware trigger */
#define  PDT_SOFTWARE_TRIGGER  (1U)                /*!< Select software trigger */
/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */
/**
 * @brief PDT clock multiplication factor enumeration.
 *
 * @details
 * Specifies the multiplication factor applied to the PDT clock signal
 * to adjust the overall timer frequency.
 *
 * @note DES_PDT_TYP_001
 */
typedef enum
{
    PDT_CLK_PREMULT_FACT_AS_1 = 0U,    /*!< Multiplication factor is 1  */
    PDT_CLK_PREMULT_FACT_AS_10,        /*!< Multiplication factor is 10 */
    PDT_CLK_PREMULT_FACT_AS_20,        /*!< Multiplication factor is 20 */
    PDT_CLK_PREMULT_FACT_AS_40         /*!< Multiplication factor is 40 */
} Pdt_ClkPrescalerMultFactorType;

/**
 * @brief PDT clock prescaler division factor enumeration.
 *
 * @details
 * Specifies the division factor applied to the PDT input clock to derive
 * the effective timer clock frequency.
 *
 * @note DES_PDT_TYP_002
 */
typedef enum
{
    PDT_CLK_PREDIV_BY_1 = 0U,   /*!< Clock divided by MULT */
    PDT_CLK_PREDIV_BY_2,        /*!< Clock divided by 2 * MULT */
    PDT_CLK_PREDIV_BY_4,        /*!< Clock divided by 4 * MULT */
    PDT_CLK_PREDIV_BY_8,        /*!< Clock divided by 8 * MULT */
    PDT_CLK_PREDIV_BY_16,       /*!< Clock divided by 16 * MULT */
    PDT_CLK_PREDIV_BY_32,       /*!< Clock divided by 32 * MULT */
    PDT_CLK_PREDIV_BY_64,       /*!< Clock divided by 64 * MULT */
    PDT_CLK_PREDIV_BY_128       /*!< Clock divided by 128 * MULT */
} Pdt_ClockPrescalerDivType;

/**
 * @brief PDT load mode selection enumeration.
 *
 * @details
 * Specifies when the PDT modulus and interrupt delay values are loaded from
 * the shadow registers into the working registers.
 *
 * @note DES_PDT_TYP_003
 */
typedef enum
{
    PDT_LOAD_VAL_IMMEDIATELY = 0U,                       /*!< Load immediately after 1 is written to LDOK */
    PDT_LOAD_VAL_AT_MODULO_COUNTER,                      /*!< Load when CNT = MODE + 1, after 1 is written to LDOK */
    PDT_LOAD_VAL_AT_NEXT_TRIGGER,                        /*!< Load when a trigger input event is detected, after 1 is written to LDOK */
    PDT_LOAD_VAL_AT_MODULO_COUNTER_OR_NEXT_TRIGGER       /*!< Load when either the CNT = MODE + 1, or a trigger input event is detected, after 1 is written to LDOK */
} Pdt_LoadValueModeType;

/**
 * @brief PDT pulse output configuration structure.
 *
 * @details
 * Contains parameters for configuring the PDT pulse output feature including
 * delay values for high and low periods and enable control.
 *
 * @note DES_PDT_TYP_004
 */
typedef struct
{
    uint16 Poly1;                   /*!< Delay value for high period (0-65535) */
    uint16 Poly2;                   /*!< Delay value for low period (0-65535)  */
    boolean PulseOutEnable;         /*!< Pulse output enable flag              */
} Pdt_PulseOutConfigType;

/**
 * @brief PDT delay channel enumeration.
 *
 * @details
 * Enumerates the 8 available delay channels (0-7) for PDT trigger delay configuration.
 *
 * @note DES_PDT_TYP_005
 */
typedef enum
{
    PDT_DLY_0 = 0U,   /*!< PDT delay channel 0 */
    PDT_DLY_1,        /*!< PDT delay channel 1 */
    PDT_DLY_2,        /*!< PDT delay channel 2 */
    PDT_DLY_3,        /*!< PDT delay channel 3 */
    PDT_DLY_4,        /*!< PDT delay channel 4 */
    PDT_DLY_5,        /*!< PDT delay channel 5 */
    PDT_DLY_6,        /*!< PDT delay channel 6 */
    PDT_DLY_7         /*!< PDT delay channel 7 */
} Pdt_DlyChannelType;                 //PRQA S 1535 # AMCE_PDT_1535

/**
 * @brief PDT timer configuration structure.
 *
 * @details
 * Contains all parameters required to configure the PDT timer module including
 * clock settings, trigger sources, interrupt configuration, and callback function.
 *
 * @note DES_PDT_TYP_006
 */
typedef struct
{
    boolean ContinuousModeEnable;                     /*!< Continuous mode enable flag                    */
    boolean IntEnable;                                /*!< Timer interrupt enable flag                    */
    Pdt_LoadValueModeType LoadValueMode;              /*!< Value load timing mode selection               */
    Pdt_ClockPrescalerDivType ClkPreDiv;              /*!< Clock prescaler division factor                */
    Pdt_ClkPrescalerMultFactorType ClkPreMultFactor;  /*!< Clock multiplication factor for prescaler      */
    uint8 TriggerInput;                               /*!< Trigger input source selection                 */
    uint16 ModeVal;                                   /*!< Timer modulus value (compare value)            */
    uint16 IntDelayVal;                               /*!< Interrupt delay value                          */
    Hal_CallbackType Callback;                        /*!< Timer interrupt callback function pointer      */
} Pdt_TimerConfigType;

/**
 * @brief PDT trigger delay configuration structure.
 *
 * @details
 * Contains trigger delay parameters for all 8 delay channels including
 * enable control and bypass option.
 *
 * @note DES_PDT_TYP_007
 */
typedef struct
{
    boolean TriggerDelayBypassEn;     /*!< Trigger delay bypass enable flag    */
    uint8 DelayEnable;                /*!< Delay enable mask for channels 0-7  */
    uint16 Dly[PDT_DLY_MAX];          /*!< Delay values for channels 0-7       */
} Pdt_TriggerDelayConfigType;

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */
/**
 * @brief Initializes the PDT module.
 *
 * @details
 * Enables the PDT peripheral clock and configures the PDT module according
 * to the provided configuration structure. Sets the initial state to IDLE.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] Config: Pointer to the PDT timer configuration structure
 *
 * @return void
 *
 * @pre
 *     The PDT module must not be already initialized.
 *     The Config pointer must be valid and non-NULL.
 *
 * @post
 *     PDT module is initialized and peripheral clock is enabled.
 *     Module state is set to IDLE.
 *
 * @note
 *     This function must be called before any other PDT_Hal functions.
 *     DES_PDT_API_201
 */
void Pdt_Hal_Init(const uint8 Instance, const Pdt_TimerConfigType * const Config);

/**
 * @brief Configures the PDT module with new parameters.
 *
 * @details
 * Applies new timer configuration settings to the PDT module including clock
 * settings, load mode, and interrupt configuration.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] Config: Pointer to the PDT timer configuration structure
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *     The module must not be running (status == PDT_IDLE).
 *     The Config pointer must be valid and non-NULL.
 *
 * @post
 *     PDT hardware registers are updated with new configuration.
 *     Interrupt enable/disable status is updated based on configuration.
 *
 * @note
 *     DES_PDT_API_202
 */
void Pdt_Hal_ConfigTimer(const uint8 Instance, const Pdt_TimerConfigType * const Config);

/**
 * @brief Starts the PDT timer.
 *
 * @details
 * Enables the PDT counter by setting the enable bit in the control register.
 * The module must be in IDLE state. Once started, the timer increments based
 * on the configured clock source.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized and in IDLE state.
 *
 * @post
 *     PDT counter is enabled and incrementing.
 *     Module state changes to RUNNING.
 *
 * @note
 *     DES_PDT_API_203
 */
void Pdt_Hal_Start(const uint8 Instance);

/**
 * @brief Stops the PDT timer.
 *
 * @details
 * Disables the PDT counter by clearing the enable bit in the control register.
 * The module must be in RUNNING state. Counter incrementation is halted and
 * the counter value is frozen at its current value.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 *
 * @return void
 *
 * @pre
 *     The PDT module must be running (state == RUNNING).
 *
 * @post
 *     PDT counter is disabled and stopped.
 *     Module state changes to IDLE.
 *     Counter value is frozen at its current value.
 *
 * @note
 *     DES_PDT_API_204
 */
void Pdt_Hal_Stop(const uint8 Instance);

/**
 * @brief Gets the PDT module operational status.
 *
 * @details
 * Returns the current state of the PDT module indicating whether it is running,
 * idle, or in an error state.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 *
 * @return Hal_StatusType
 *     - STATUS_BUSY: PDT is running
 *     - STATUS_IDLE: PDT is initialized but not running
 *     - STATUS_ERROR: PDT is in an undefined or error state
 *
 * @pre
 *     None
 *
 * @post
 *     No hardware state changes (read-only operation).
 *
 * @note
 *     DES_PDT_API_205
 */
Hal_StatusType Pdt_Hal_GetStatus(const uint8 Instance);

/**
 * @brief Triggers the PDT module via software.
 *
 * @details
 * Generates a software trigger event that can be used to control PDT operation
 * based on the configured trigger mode.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *
 * @post
 *     A software trigger event is generated.
 *
 * @note
 *     The effect depends on the trigger input configuration set during initialization.
 *     DES_PDT_API_206
 */
void Pdt_Hal_SoftTriggerCmd(const uint8 Instance);

/**
 * @brief Loads PDT modulus and interrupt delay values.
 *
 * @details
 * Triggers the load command which causes the PDT to load the modulus and
 * interrupt delay values according to the configured load mode.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized and not running.
 *
 * @post
 *     Load command is generated. Values are loaded based on load mode configuration.
 *
 * @note
 *     DES_PDT_API_207
 */
void Pdt_Hal_LoadValuesCmd(const uint8 Instance);

/**
 * @brief De-initializes the PDT module.
 *
 * @details
 * Disables the PDT interrupt and peripheral clock, clearing the callback and
 * returning the module to uninitialized state.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 *
 * @return void
 *
 * @pre
 *     The PDT module should be initialized.
 *
 * @post
 *     PDT peripheral clock is disabled.
 *     Interrupt is disabled.
 *     Callback function pointer is cleared.
 *     Module state is set to UNINITED.
 *
 * @note
 *     After de-initialization, Pdt_Hal_Init() must be called again to use the module.
 *     DES_PDT_API_208
 */
void Pdt_Hal_Deinit(const uint8 Instance);

/**
 * @brief Gets the PDT timer current value.
 *
 * @details
 * Reads the real-time counter value from the PDT hardware register.
 * This represents the current count in timer ticks.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 *
 * @return uint32
 *     The current PDT counter value (32-bit unsigned integer)
 *
 * @pre
 *     The PDT module must be initialized.
 *
 * @post
 *     No hardware state changes (read-only operation).
 *
 * @note
 *     DES_PDT_API_209
 */
uint32 Pdt_Hal_GetTimerValue(const uint8 Instance);

/**
 * @brief Gets the PDT timer interrupt flag.
 *
 * @details
 * Reads the hardware interrupt flag bit to determine if a PDT interrupt event
 * has been triggered. The flag is set when the timer reaches the interrupt delay value.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 *
 * @return boolean
 *     - TRUE: Timer interrupt has occurred
 *     - FALSE: No timer interrupt
 *
 * @pre
 *     The PDT module must be initialized.
 *
 * @post
 *     No hardware state changes (read-only operation).
 *
 * @note
 *     DES_PDT_API_210
 */
boolean Pdt_Hal_GetTimerIntFlag(const uint8 Instance);

/**
 * @brief Clears the PDT timer interrupt flag.
 *
 * @details
 * Writes a 1 to the hardware interrupt flag bit to clear any pending timer
 * interrupt. This must be called after handling the interrupt event.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *     A timer interrupt should have occurred.
 *
 * @post
 *     The timer interrupt flag bit is cleared in hardware.
 *
 * @note
 *     This function should be called in the interrupt handler or in polling mode.
 *     DES_PDT_API_211
 */
void Pdt_Hal_ClearTimerIntFlag(const uint8 Instance);

/**
 * @brief Sets the PDT timer modulus (compare) value.
 *
 * @details
 * Updates the timer modulus value which determines when the timer overflows
 * or triggers an interrupt (depending on configuration).
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] Value: Timer modulus value (16-bit unsigned)
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *
 * @post
 *     PDT modulus register is updated with the new value.
 *
 * @note
 *     DES_PDT_API_212
 */
void Pdt_Hal_SetTimerModulusValue(const uint8 Instance, const uint16 Value);

/**
 * @brief Sets the PDT timer interrupt delay value.
 *
 * @details
 * Updates the interrupt delay value which specifies at what count the timer
 * interrupt flag is set.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] Value: Interrupt delay value (16-bit unsigned)
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *
 * @post
 *     PDT interrupt delay register is updated with the new value.
 *
 * @note
 *     DES_PDT_API_213
 */
void Pdt_Hal_SetValueForTimerInterrupt(const uint8 Instance, const uint16 Value);

/**
 * @brief Enables or disables the PDT pulse output.
 *
 * @details
 * Controls whether the PDT generates a pulse output signal based on the
 * timer operation.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] Enable: Pulse output enable flag
 *     - TRUE: Enable pulse output
 *     - FALSE: Disable pulse output
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *
 * @post
 *     Pulse output enable bit in hardware is updated.
 *
 * @note
 *     DES_PDT_API_214
 */
void Pdt_Hal_SetCmpPulseOutEnable(const uint8 Instance, const boolean Enable);

/**
 * @brief Sets the PDT pulse output delay value for high period.
 *
 * @details
 * Configures the duration of the high period for the PDT pulse output signal.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] Value: High period delay value (16-bit unsigned)
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *
 * @post
 *     PDT pulse output high delay register is updated.
 *
 * @note
 *     DES_PDT_API_215
 */
void Pdt_Hal_SetCmpPulseOutDelayForHigh(const uint8 Instance, const uint16 Value);

/**
 * @brief Sets the PDT pulse output delay value for low period.
 *
 * @details
 * Configures the duration of the low period for the PDT pulse output signal.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] Value: Low period delay value (16-bit unsigned)
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *
 * @post
 *     PDT pulse output low delay register is updated.
 *
 * @note
 *     DES_PDT_API_216
 */
void Pdt_Hal_SetCmpPulseOutDelayForLow(const uint8 Instance, const uint16 Value);

/**
 * @brief Sets the PDT trigger delay value for a specific channel.
 *
 * @details
 * Configures the delay value for one of the 8 PDT trigger delay channels.
 * The delay is applied when the trigger event is detected.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] Chn: Delay channel index (0-7)
 * @param[in] Value: Trigger delay value (16-bit unsigned)
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *     Channel number must be valid (0-7).
 *
 * @post
 *     PDT trigger delay register for the specified channel is updated.
 *
 * @note
 *     DES_PDT_API_217
 */
void Pdt_Hal_SetTriggerDelayValue(const uint8 Instance, const uint8 Chn, const uint16 Value);

/**
 * @brief Configures all PDT trigger delay settings.
 *
 * @details
 * Applies comprehensive trigger delay configuration including delay values for
 * all 8 channels, enable mask, and bypass control.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] DelayConfig: Pointer to the PDT trigger delay configuration structure
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *     The DelayConfig pointer must be valid and non-NULL.
 *
 * @post
 *     PDT trigger delay configuration registers are updated with values from DelayConfig.
 *
 * @note
 *     DES_PDT_API_218
 */
void Pdt_Hal_ConfigTriggerDelay(const uint8 Instance, const Pdt_TriggerDelayConfigType *DelayConfig);

/**
 * @brief Configures the PDT pulse output settings.
 *
 * @details
 * Applies pulse output configuration including high/low delay values and
 * pulse output enable control.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] Config: Pointer to the PDT pulse output configuration structure
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *     The Config pointer must be valid and non-NULL.
 *
 * @post
 *     PDT pulse output configuration registers are updated.
 *
 * @note
 *     DES_PDT_API_219
 */
void Pdt_Hal_ConfigPulseOut(const uint8 Instance, const Pdt_PulseOutConfigType *Config);

/**
 * @brief Enables or disables the PDT interrupt.
 *
 * @details
 * Controls whether timer interrupt events generate interrupt signals to the processor.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] Enable: Interrupt enable flag
 *     - TRUE: Enable PDT interrupts
 *     - FALSE: Disable PDT interrupts
 *
 * @return void
 *
 * @pre
 *     The PDT module must be initialized.
 *
 * @post
 *     Interrupt enable bit in hardware is updated.
 *
 * @note
 *     DES_PDT_API_220
 */
void Pdt_Hal_EnableInterrupt(const uint8 Instance, const boolean Enable);

/**
 * @brief Installs the PDT interrupt callback function.
 *
 * @details
 * Registers a user-defined callback function that will be invoked when a
 * PDT timer interrupt occurs. The callback is called from the PDT interrupt
 * service routine.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 * @param[in] Func: Pointer to the callback function. Set to NULL to disable callbacks.
 *     Callback signature: void (*Hal_CallbackType)(void *Args)
 *
 * @return void
 *
 * @pre
 *     The PDT module should be initialized before installing a callback.
 *
 * @post
 *     The callback function is stored and will be called on timer interrupts.
 *
 * @note
 *     The callback function must be non-blocking and execute quickly.
 *     Interrupt must be enabled separately via Pdt_Hal_EnableInterrupt().
 *     DES_PDT_API_221
 */
void Pdt_Hal_InstallCallback(const uint8 Instance, const Hal_CallbackType Func);

ISR(PDT0_IRQHandler);

ISR(PDT1_IRQHandler);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PDT_HAL_H */

/* =============================================  EOF  ============================================== */
