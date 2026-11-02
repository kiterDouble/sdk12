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
 * @file Adc_Hal.h
 * @brief This file provides all adc hal api.
 */
#ifndef ADC_HAL_H
#define ADC_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Adc_Hal_Types.h"
#include "AC784xx_Adc_Reg.h"

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/**
 * @brief Initialize ADC hardware module with provided configuration.
 *
 * @details
 * Initializes the low-level ADC hardware with the parameters provided in
 * Adc_InitConfigType. Enables peripheral clocks, configures interrupt handlers,
 * and applies a soft reset to ensure the ADC module starts from a known state.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] Config: Pointer to ADC initialization configuration structure.
 *
 * @return void
 *
 * @pre Instance shall be a valid hardware instance and Config shall not be NULL.
 * @post ADC peripheral clock enabled, interrupt configured, and hardware reset completed.
 *
 * @note DES ID: DES_ADC_API_201
 */
void Adc_Hal_Init(uint8 Instance, const Adc_InitConfigType *Config);

/**
 * @brief Deinitialize ADC hardware module.
 *
 * @details
 * De-initializes ADC hardware by disabling clocks, interrupts and DMA channels
 * associated with the ADC instance. Performs a soft reset and disables the
 * peripheral clock to save power.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 *
 * @return void
 *
 * @pre ADC hardware instance must be idle with no ongoing conversions.
 * @post ADC hardware resources released, clocks disabled, and interrupts cleared.
 *
 * @note DES ID: DES_ADC_API_202
 */
void Adc_Hal_Deinit(uint8 Instance);

/**
 * @brief Configure ADC converter parameters.
 *
 * @details
 * Applies converter-level configuration to the ADC hardware, including clock divider,
 * resolution, data alignment, voltage reference, DMA enable, hardware averaging,
 * and interleave mode. Validates clock frequency constraints.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] Config: Pointer to converter configuration structure.
 *
 * @return void
 *
 * @pre ADC hardware must be initialized via Adc_Hal_Init.
 * @post ADC converter configured according to Config parameters.
 *
 * @note DES ID: DES_ADC_API_203
 */
void Adc_Hal_ConfigConverter(uint8 Instance, const Adc_ConverterConfigType *Config);

/**
 * @brief Configure ADC channel parameters.
 *
 * @details
 * Configures a single ADC channel including sample time, channel assignment to
 * sequence, interrupt enable, and internal channel source selection. Supports
 * both regular and injection sequences.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] Config: Pointer to channel configuration structure.
 *
 * @return void
 *
 * @pre ADC converter must be configured and Config->SeqIndex must be valid.
 * @post Channel configured and assigned to the specified sequence position.
 *
 * @note DES ID: DES_ADC_API_204
 */
void Adc_Hal_ConfigChannel(uint8 Instance, const Adc_ChanConfigType *Config);

/**
 * @brief Configure ADC group mode parameters.
 *
 * @details
 * Applies group-level configuration including scan mode, continuous mode,
 * discontinuous mode, trigger sources, sequence lengths, and DMA settings.
 * Supports both regular and injection groups.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] Config: Pointer to group configuration structure.
 *
 * @return void
 *
 * @pre ADC channels must be configured before calling this function.
 * @post Group mode parameters applied to hardware registers.
 *
 * @note DES ID: DES_ADC_API_205
 */
void Adc_Hal_ConfigGroup(uint8 Instance, const Adc_GroupConfigType *Config);

/**
 * @brief Configure ADC Analog Monitor (AMO) parameters.
 *
 * @details
 * Configures the analog monitoring feature including trigger mode, thresholds,
 * offsets, and interrupt enable for regular/injection groups or single channel.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] Config: Pointer to AMO configuration structure.
 *
 * @return void
 *
 * @pre Upper threshold must be greater than lower threshold.
 * @post AMO configured and ready to trigger on threshold violations.
 *
 * @note DES ID: DES_ADC_API_206
 */
void Adc_Hal_ConfigAmo(uint8 Instance, const Adc_AmoConfigType *Config);

/**
 * @brief Trigger regular group conversion by software.
 *
 * @details
 * Initiates a software-triggered conversion for the regular group by setting
 * the appropriate start bit in the control register.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 *
 * @return void
 *
 * @pre Regular group must be configured with software trigger source.
 * @post Regular group conversion started.
 *
 * @note DES ID: DES_ADC_API_207
 */
void Adc_Hal_SwTriggerRegularConvert(uint8 Instance);

/**
 * @brief Trigger injection group conversion by software.
 *
 * @details
 * Initiates a software-triggered conversion for the injection group by setting
 * the appropriate start bit in the control register.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 *
 * @return void
 *
 * @pre Injection group must be configured with software trigger source.
 * @post Injection group conversion started.
 *
 * @note DES ID: DES_ADC_API_208
 */
void Adc_Hal_SwTriggerInjectConvert(uint8 Instance);

/**
 * @brief Read ADC conversion result for specified sequence index.
 *
 * @details
 * Retrieves the conversion result from the data register corresponding to
 * the specified sequence index. Supports both regular and injection sequences.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] SeqIndex: Sequence index (regular: 0-15, injection: 16-19).
 *
 * @return uint16: ADC conversion result value.
 *
 * @pre Conversion must be complete for the specified sequence index.
 * @post Conversion result read from hardware register.
 *
 * @note DES ID: DES_ADC_API_209
 */
uint16 Adc_Hal_GetSeqResult(uint8 Instance, Adc_SequenceType SeqIndex);

#ifndef ADC_SDK_NON_EXTENDED_API
/**
 * @brief Get ADC idle status flag.
 *
 * @details
 * Checks the status register to determine if the ADC is currently idle
 * (not performing any conversion).
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 *
 * @return boolean: TRUE if ADC is idle, FALSE if ADC is busy converting.
 *
 * @pre ADC hardware must be initialized.
 * @post No state change.
 *
 * @note DES ID: DES_ADC_API_210
 */
boolean Adc_Hal_GetIdleFlag(uint8 Instance);

/**
 * @brief Get conversion complete flag for specified sequence.
 *
 * @details
 * Reads the end-of-conversion (EOC) flag from the status register for the
 * specified sequence index. Supports both regular and injection sequences.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] SeqIndex: Sequence index (regular: 0-15, injection: 16-19).
 *
 * @return boolean: TRUE if conversion complete, FALSE if not complete.
 *
 * @pre Sequence must have been started.
 * @post No state change.
 *
 * @note DES ID: DES_ADC_API_211
 */
boolean Adc_Hal_GetConvertCompleteFlag(uint8 Instance, Adc_SequenceType SeqIndex);

/**
 * @brief Clear conversion complete flag for specified sequence.
 *
 * @details
 * Clears the end-of-conversion (EOC) flag in the status register for the
 * specified sequence index by writing to the clear register.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] SeqIndex: Sequence index (regular: 0-15, injection: 16-19).
 *
 * @return void
 *
 * @pre Conversion complete flag must be set.
 * @post EOC flag cleared in hardware status register.
 *
 * @note DES ID: DES_ADC_API_212
 */
void Adc_Hal_ClearConvertCompleteFlag(uint8 Instance, Adc_SequenceType SeqIndex);

/**
 * @brief Get trigger conflict status flag.
 *
 * @details
 * Checks if a trigger conflict has occurred, indicating that a new trigger
 * arrived while a previous conversion was still in progress.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 *
 * @return boolean: TRUE if trigger conflict detected, FALSE otherwise.
 *
 * @pre ADC hardware must be configured with hardware triggers.
 * @post No state change.
 *
 * @note DES ID: DES_ADC_API_213
 */
boolean Adc_Hal_GetTriggerConflictFlag(uint8 Instance);

/**
 * @brief Clear trigger conflict status flag.
 *
 * @details
 * Clears the trigger conflict flag in the status register by writing
 * to the clear register.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 *
 * @return void
 *
 * @pre Trigger conflict flag must be set.
 * @post Trigger conflict flag cleared in hardware status register.
 *
 * @note DES ID: DES_ADC_API_214
 */
void Adc_Hal_ClearTriggerConflictFlag(uint8 Instance);
#endif

/**
 * @brief Initialize converter configuration structure to default values.
 *
 * @details
 * Sets all fields of the Adc_ConverterConfigType structure to safe default
 * values including clock divider, resolution, alignment, voltage reference,
 * and hardware averaging settings.
 *
 * @param[out] Config: Pointer to converter configuration structure to initialize.
 *
 * @return void
 *
 * @pre Config pointer shall not be NULL.
 * @post Config structure populated with default values.
 *
 * @note DES ID: DES_ADC_API_215
 */
void Adc_Hal_InitConverterStruct(Adc_ConverterConfigType *const Config);

#ifndef ADC_SDK_NON_EXTENDED_API
/**
 * @brief Initialize group configuration structure to default values.
 *
 * @details
 * Sets all fields of the Adc_GroupConfigType structure to safe default
 * values including sequence lengths, trigger sources, and conversion modes.
 *
 * @param[out] Config: Pointer to group configuration structure to initialize.
 *
 * @return void
 *
 * @pre Config pointer shall not be NULL.
 * @post Config structure populated with default values.
 *
 * @note DES ID: DES_ADC_API_216
 */
void Adc_Hal_InitGroupStruct(Adc_GroupConfigType *const Config);
#endif

/**
 * @brief Initialize channel configuration structure to default values.
 *
 * @details
 * Sets all fields of the Adc_ChanConfigType structure to safe default
 * values including sample time, channel number, interrupt enable, and
 * internal channel source.
 *
 * @param[out] Config: Pointer to channel configuration structure to initialize.
 *
 * @return void
 *
 * @pre Config pointer shall not be NULL.
 * @post Config structure populated with default values.
 *
 * @note DES ID: DES_ADC_API_217
 */
void Adc_Hal_InitChanStruct(Adc_ChanConfigType *const Config);

#ifndef ADC_SDK_NON_EXTENDED_API
/**
 * @brief Initialize Analog Monitor configuration structure to default values.
 *
 * @details
 * Sets all fields of the Adc_AmoConfigType structure to safe default
 * values including trigger mode, thresholds, offsets, and interrupt settings.
 *
 * @param[out] Config: Pointer to AMO configuration structure to initialize.
 *
 * @return void
 *
 * @pre Config pointer shall not be NULL.
 * @post Config structure populated with default values.
 *
 * @note DES ID: DES_ADC_API_218
 */
void Adc_Hal_InitAMOStruct(Adc_AmoConfigType *const Config);
#endif

/**
 * @brief Configure regular group parameters in hardware.
 *
 * @details
 * Applies regular group specific configuration including scan mode, continuous
 * mode, trigger source, and sequence length to the hardware registers.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] Config: Pointer to group configuration structure.
 *
 * @return void
 *
 * @pre ADC hardware must be initialized and channels configured.
 * @post Regular group parameters written to hardware registers.
 *
 * @note DES ID: DES_ADC_API_219
 */
void Adc_Hal_ConfigRegularGroup(uint32 Instance, const Adc_GroupConfigType* Config);

/**
 * @brief Configure injection group parameters in hardware.
 *
 * @details
 * Applies injection group specific configuration including scan mode, interval
 * mode, trigger source, and sequence length to the hardware registers.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] Config: Pointer to group configuration structure.
 *
 * @return void
 *
 * @pre ADC hardware must be initialized and channels configured.
 * @post Injection group parameters written to hardware registers.
 *
 * @note DES ID: DES_ADC_API_220
 */
void Adc_Hal_ConfigInjectGroup(uint32 Instance, const Adc_GroupConfigType* Config);

/**
 * @brief Enable or disable DMA for ADC transfers.
 *
 * @details
 * Controls the DMA enable bit in the ADC control register to enable or
 * disable DMA-based transfer of conversion results.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] Enable: TRUE to enable DMA, FALSE to disable.
 *
 * @return void
 *
 * @pre ADC hardware must be initialized.
 * @post DMA enable flag updated in hardware control register.
 *
 * @note DES ID: DES_ADC_API_221
 */
void Adc_Hal_DmaEnable(uint32 Instance, boolean Enable);

#ifndef ADC_SDK_NON_EXTENDED_API
/**
 * @brief Get parity value for specified sequence index.
 *
 * @details
 * Retrieves the parity check value from the hardware register for the
 * specified sequence index, used for data integrity verification.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] SeqIndex: Sequence index (regular: 0-15, injection: 16-19).
 *
 * @return uint8: Parity value for the sequence.
 *
 * @pre Conversion must be complete for the specified sequence.
 * @post No state change.
 *
 * @note DES ID: DES_ADC_API_222
 */
uint8 Adc_Hal_GetParityVal(uint8 Instance, Adc_SequenceType SeqIndex);

/**
 * @brief Get ADC hardware base address pointer.
 *
 * @details
 * Returns a pointer to the ADC hardware register base address for direct
 * register access by upper layers if needed.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 *
 * @return ADC_Type*: Pointer to ADC hardware register base.
 *
 * @pre Instance must be valid (0 or 1).
 * @post No state change.
 *
 * @note DES ID: DES_ADC_API_223
 */
ADC_Type *Adc_Hal_GetBase(uint8 Instance);

/**
 * @brief Perform ADC hardware self-test by measuring the internal Bandgap reference voltage.
 *
 * @details
 * Validates ADC hardware integrity by measuring the internal Bandgap reference and
 * checking the result is within +/-10% of the expected nominal value supplied by the caller.
 * The ADC bus clock is disabled at the end to reset all registers.
 *
 * @param[in] Instance:   ADC hardware instance number (0 or 1).
 * @param[in] VrefMv:     ADC full-scale reference voltage in mV (e.g. 3300 for VDDA).
 *
 * @return boolean: TRUE if the Bandgap voltage is within tolerance, FALSE on out-of-range
 *                  result or conversion timeout.
 *
 * @pre  None. This function enables the ADC bus clock internally.
 * @post ADC bus clock is disabled; all ADC registers are reset to default.
 *       Caller must re-initialize the ADC before further use.
 *
 * @note DES ID: DES_ADC_API_235
 */
boolean Adc_Hal_SelfTest(uint8 Instance, uint16 VrefMv);

/**
 * @brief Set offset value for injection group sequence.
 *
 * @details
 * Programs the offset value for the specified injection sequence, which is
 * automatically subtracted from the conversion result by hardware.
 *
 * @param[in] Instance: ADC hardware instance number (0 or 1).
 * @param[in] SeqIndex: Injection sequence index (16-19).
 * @param[in] Offset: Offset value to be applied (0-4095).
 *
 * @return void
 *
 * @pre SeqIndex must be a valid injection sequence index.
 * @post Offset value written to injection offset register.
 *
 * @note DES ID: DES_ADC_API_225
 */
void Adc_Hal_SetInjectOffset(uint8 Instance, Adc_SequenceType SeqIndex, uint16 Offset);

/**
 * @brief Convert ADC raw value from temperature sensor to Celsius.
 *
 * @details
 * Applies the temperature sensor calibration formula to convert the ADC
 * raw result into a temperature value in degrees Celsius.
 *
 * @param[in] Value: ADC conversion result from internal temperature sensor.
 * @param[in] RefVoltage: Reference voltage used by ADC in millivolts.
 *
 * @return float32: Temperature value in degrees Celsius.
 *
 * @pre Value must be from a valid temperature sensor conversion.
 * @post No state change.
 *
 * @note DES ID: DES_ADC_API_226
 */
float32 Adc_Hal_ConvertToTemperature(const uint16 Value, const float32 RefVoltage);
#endif
#ifdef __cplusplus
}
#endif
#endif /*ADC_HAL_H*/

/* =============================================  EOF  ============================================== */
