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

 * @file Crc_Hal.h
 *
 * @brief This file provides extern Crc Hal API.
 *
 */
#ifndef CRC_HAL_H
#define CRC_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Crc_Hal_Types.h"
#include "AC784xx_Crc_Reg.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief Initializes the CRC hardware module with the specified configuration.
 *
 * @details
 * This function configures the CRC hardware peripheral with the provided settings,
 * including protocol type, transpose modes, polynomial value, and seed value. It enables
 * the peripheral clock if not already enabled and sets up the CRC controller for subsequent
 * calculations.
 *
 * @param[in] Instance: CRC hardware instance number (must be less than CRC_INSTANCE_MAX).
 * @param[in] ConfigPtr: Pointer to CRC configuration structure containing:
 *                       - Protocol: CRC protocol type (8-bit, 16-bit, or 32-bit)
 *                       - WriteTranspose: Input data transpose mode
 *                       - ReadTranspose: Output data transpose mode
 *                       - FinalXOR: Final XOR enable flag
 *                       - Poly: Polynomial value for CRC calculation
 *                       - Seed: Initial seed value
 *
 * @return void
 *
 * @pre
 *     ConfigPtr must not be NULL.
 *     Instance must be a valid CRC hardware instance.
 *
 * @post
 *     CRC peripheral clock is enabled.
 *     CRC hardware is configured and ready for calculations.
 *
 * @note
 *     This is a HAL layer function for direct hardware initialization.
 *     DES ID: DES_CRC_API_201
 */
void Crc_Hal_Init
(
    uint8 Instance,
    const Crc_ConfigType *ConfigPtr
);

/**
 * @brief Deinitializes the CRC hardware module and disables the peripheral clock.
 *
 * @details
 * This function disables the CRC peripheral clock to reduce power consumption when
 * CRC calculations are no longer needed. It should be called when the CRC module
 * is no longer in use.
 *
 * @return void
 *
 * @pre
 *     CRC module must have been previously initialized via Crc_Hal_Init().
 *
 * @post
 *     CRC peripheral clock is disabled.
 *
 * @note
 *     After calling this function, Crc_Hal_Init() must be called again before
 *     performing any CRC calculations.
 *     DES ID: DES_CRC_API_206
 */
void Crc_Hal_Deinit(void);

/**
 * @brief Calculates CRC checksum over a data block using hardware acceleration.
 *
 * @details
 * This function performs CRC calculation on the provided data buffer using the
 * CRC hardware peripheral. It processes data in 32-bit, 16-bit, and 8-bit chunks
 * for optimal performance. The calculation uses the configuration previously set
 * by Crc_Hal_Init().
 *
 * @param[in] Instance: CRC hardware instance number (must be less than CRC_INSTANCE_MAX).
 * @param[in] DataPtr: Pointer to the data buffer to be processed.
 * @param[in] Length: Number of bytes in the data buffer.
 *
 * @return Calculated CRC checksum value.
 *
 * @pre
 *     CRC module must be initialized via Crc_Hal_Init().
 *     DataPtr must not be NULL.
 *     Instance must be valid.
 *
 * @post
 *     CRC calculation result is available in hardware register.
 *
 * @note
 *     The function processes data in word-aligned chunks for efficiency.
 *     Reentrancy: Not reentrant for the same Instance.
 *     DES ID: DES_CRC_API_202
 */
uint32 Crc_Hal_CalculateCRC
(
    uint8 Instance,
    const uint8 *DataPtr,
    uint32 Length
);

/**
 * @brief Calculates CRC checksum using DMA for large data transfers.
 *
 * @details
 * This function configures and initiates a DMA transfer to calculate CRC over
 * the provided data buffer. The DMA controller handles data movement to the CRC
 * peripheral, reducing CPU overhead. The transfer configuration is determined by
 * the CRC protocol type (32-bit or 16-bit).
 *
 * @param[in] Channel: DMA channel number to use for the transfer.
 * @param[in] DataPtr: Pointer to the source data buffer.
 * @param[in] Length: Number of bytes to process.
 * @param[in] Params: Reserved for callback parameters (currently unused, pass NULL).
 *
 * @return
 *     STATUS_SUCCESS: DMA transfer configured and started successfully.
 *     STATUS_ERROR: DMA configuration failed.
 *
 * @pre
 *     CRC module must be initialized via Crc_Hal_Init().
 *     DMA channel must be available and not in use.
 *     DataPtr must not be NULL.
 *
 * @post
 *     DMA transfer is initiated.
 *     Use Crc_Hal_GetCRCResult() to retrieve result after DMA completion.
 *
 * @note
 *     This function returns immediately after starting DMA transfer.
 *     The caller must monitor DMA completion before reading CRC result.
 *     DES ID: DES_CRC_API_203
 */
Hal_StatusType Crc_Hal_DmaCalculateCRC
(
    uint8 Channel,
    const uint8 *DataPtr,
    uint32 Length,
    const void *Params
);

/**
 * @brief Retrieves the current CRC calculation result from hardware.
 *
 * @details
 * This function reads the CRC result register and returns the calculated checksum
 * value. The result width depends on the configured CRC protocol (8-bit, 16-bit,
 * or 32-bit).
 *
 * @param[in] Instance: CRC hardware instance number (must be less than CRC_INSTANCE_MAX).
 *
 * @return Current CRC calculation result.
 *
 * @pre
 *     CRC calculation must be completed.
 *     Instance must be valid.
 *
 * @post
 *     None.
 *
 * @note
 *     This function can be called after Crc_Hal_CalculateCRC() or after
 *     DMA-based CRC calculation completes.
 *     DES ID: DES_CRC_API_204
 */
uint32 Crc_Hal_GetCRCResult
(
    uint8 Instance
);

/**
 * @brief Sets the initial seed value for CRC calculation.
 *
 * @details
 * This function updates the CRC seed/initial value in the hardware register.
 * The seed value is used as the starting point for subsequent CRC calculations.
 * The function temporarily switches to seed mode, writes the value, then returns
 * to data mode.
 *
 * @param[in] Instance: CRC hardware instance number (must be less than CRC_INSTANCE_MAX).
 * @param[in] Seed: Initial seed value for CRC calculation.
 *
 * @return void
 *
 * @pre
 *     CRC module must be initialized via Crc_Hal_Init().
 *
 * @post
 *     CRC seed register is updated with new value.
 *
 * @note
 *     This function can be used to update the seed between calculations.
 *     DES ID: DES_CRC_API_214
 */
void Crc_Hal_SetSeed(uint8 Instance, uint32 Seed);

/**
 * @brief Retrieves the current CRC hardware configuration.
 *
 * @details
 * This function reads the current CRC hardware settings and populates the
 * provided configuration structure. It retrieves protocol type, transpose modes,
 * XOR mode, polynomial value, and seed value from hardware registers.
 *
 * @param[in] Instance: CRC hardware instance number (must be less than CRC_INSTANCE_MAX).
 * @param[out] ConfigPtr: Pointer to configuration structure to be filled with current settings.
 *
 * @return
 *     STATUS_SUCCESS: Configuration retrieved successfully.
 *     STATUS_ERROR: Configuration retrieval failed (currently unused).
 *
 * @pre
 *     ConfigPtr must not be NULL.
 *     Instance must be valid.
 *
 * @post
 *     ConfigPtr contains current hardware configuration.
 *
 * @note
 *     This function can be used to save current configuration for later restoration.
 *     DES ID: DES_CRC_API_205
 */
Hal_StatusType Crc_Hal_GetConfig
(
    uint8 Instance,
    Crc_ConfigType *ConfigPtr
);

/**
 * @brief Calculates 8-bit CRC using software implementation (SAE J1850).
 *
 * @details
 * This function performs CRC8 calculation using software algorithm based on
 * SAE J1850 standard (polynomial 0x1D). It supports three calculation modes:
 * table-based with 16 entries (compact), table-based with 256 entries (fast),
 * or runtime calculation (minimal memory). The function can process data in
 * multiple calls by maintaining state through the start value.
 *
 * @param[in] Crc_DataPtr: Pointer to input data buffer.
 * @param[in] Crc_Length: Number of bytes to process.
 * @param[in] Crc_StartValue8: Starting CRC value (previous result for continued calculation).
 * @param[in] Crc_IsFirstCall: TRUE for first call (uses initial value 0xFF), FALSE for continuation.
 * @param[in] Mode: Calculation method:
 *                  - CRC_TABLE_16_BYTE_MODE: Use 16-entry lookup table
 *                  - CRC_TABLE_256_BYTE_MODE: Use 256-entry lookup table
 *                  - CRC_RUNTIME_MODE: Calculate without lookup table
 *
 * @return Calculated 8-bit CRC value with final XOR 0xFF applied.
 *
 * @pre
 *     Crc_DataPtr must not be NULL.
 *     Mode must be a valid CRC calculation mode.
 *
 * @post
 *     None.
 *
 * @note
 *     This is a pure software implementation, no hardware required.
 *     Reentrancy: Reentrant.
 *     Complies with AUTOSAR R4.0 CRC SWS specification.
 *     DES ID: DES_CRC_API_207
 */
uint8 Crc_Hal_CalculateCRC8
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint8 Crc_StartValue8,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief Calculates 8-bit CRC using software implementation with 0x2F polynomial.
 *
 * @details
 * This function performs CRC8H2F calculation using software algorithm with
 * polynomial 0x2F. It supports three calculation modes: table-based with 16
 * entries (compact), table-based with 256 entries (fast), or runtime calculation
 * (minimal memory). The function can process data in multiple calls by maintaining
 * state through the start value.
 *
 * @param[in] Crc_DataPtr: Pointer to input data buffer.
 * @param[in] Crc_Length: Number of bytes to process.
 * @param[in] Crc_StartValue8H2F: Starting CRC value (previous result for continued calculation).
 * @param[in] Crc_IsFirstCall: TRUE for first call (uses initial value 0xFF), FALSE for continuation.
 * @param[in] Mode: Calculation method:
 *                  - CRC_TABLE_16_BYTE_MODE: Use 16-entry lookup table
 *                  - CRC_TABLE_256_BYTE_MODE: Use 256-entry lookup table
 *                  - CRC_RUNTIME_MODE: Calculate without lookup table
 *
 * @return Calculated 8-bit CRC value with final XOR 0xFF applied.
 *
 * @pre
 *     Crc_DataPtr must not be NULL.
 *     Mode must be a valid CRC calculation mode.
 *
 * @post
 *     None.
 *
 * @note
 *     This is a pure software implementation, no hardware required.
 *     Reentrancy: Reentrant.
 *     Uses polynomial 0x2F as specified in AUTOSAR standard.
 *     DES ID: DES_CRC_API_208
 */
uint8 Crc_Hal_CalculateCRC8H2F
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint8 Crc_StartValue8H2F,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief Calculates 16-bit CRC using software implementation (CCITT standard).
 *
 * @details
 * This function performs CRC16 calculation using software algorithm with
 * polynomial 0x1021 (CCITT standard). It supports three calculation modes:
 * table-based with 16 entries (compact), table-based with 256 entries (fast),
 * or runtime calculation (minimal memory). The function can process data in
 * multiple calls by maintaining state through the start value.
 *
 * @param[in] Crc_DataPtr: Pointer to input data buffer.
 * @param[in] Crc_Length: Number of bytes to process.
 * @param[in] Crc_StartValue16: Starting CRC value (previous result for continued calculation).
 * @param[in] Crc_IsFirstCall: TRUE for first call (uses initial value 0xFFFF), FALSE for continuation.
 * @param[in] Mode: Calculation method:
 *                  - CRC_TABLE_16_BYTE_MODE: Use 16-entry lookup table
 *                  - CRC_TABLE_256_BYTE_MODE: Use 256-entry lookup table
 *                  - CRC_RUNTIME_MODE: Calculate without lookup table
 *
 * @return Calculated 16-bit CRC value (no final XOR applied).
 *
 * @pre
 *     Crc_DataPtr must not be NULL.
 *     Mode must be a valid CRC calculation mode.
 *
 * @post
 *     None.
 *
 * @note
 *     This is a pure software implementation, no hardware required.
 *     Reentrancy: Reentrant.
 *     Complies with AUTOSAR CRC SWS specification.
 *     DES ID: DES_CRC_API_209
 */
uint16 Crc_Hal_CalculateCRC16
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint16 Crc_StartValue16,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief Calculates 16-bit CRC using software implementation (ARC variant).
 *
 * @details
 * This function performs CRC16 ARC calculation using software algorithm with
 * reflected polynomial 0xA001 (reflected from 0x8005). The ARC variant uses
 * a reflected algorithm processing data LSB first. It supports three calculation
 * modes: table-based with 16 entries (compact), table-based with 256 entries
 * (fast), or runtime calculation (minimal memory).
 *
 * @param[in] Crc_DataPtr: Pointer to input data buffer.
 * @param[in] Crc_Length: Number of bytes to process.
 * @param[in] Crc_StartValue16: Starting CRC value (previous result for continued calculation).
 * @param[in] Crc_IsFirstCall: TRUE for first call (uses initial value 0x0000), FALSE for continuation.
 * @param[in] Mode: Calculation method:
 *                  - CRC_TABLE_16_BYTE_MODE: Use 16-entry lookup table
 *                  - CRC_TABLE_256_BYTE_MODE: Use 256-entry lookup table
 *                  - CRC_RUNTIME_MODE: Calculate without lookup table
 *
 * @return Calculated 16-bit CRC value (no final XOR applied).
 *
 * @pre
 *     Crc_DataPtr must not be NULL.
 *     Mode must be a valid CRC calculation mode.
 *
 * @post
 *     None.
 *
 * @note
 *     This is a pure software implementation, no hardware required.
 *     Reentrancy: Reentrant.
 *     Uses reflected algorithm for efficient table-based calculation.
 *     DES ID: DES_CRC_API_210
 */
uint16 Crc_Hal_CalculateCRC16ARC
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint16 Crc_StartValue16,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief Calculates 32-bit CRC using software implementation (IEEE-802.3).
 *
 * @details
 * This function performs CRC32 calculation using software algorithm with
 * reflected polynomial 0xEDB88320 (reflected from 0x04C11DB7, IEEE-802.3/Ethernet
 * standard). The function uses a reflected algorithm processing data LSB first.
 * It supports three calculation modes: table-based with 16 entries (compact),
 * table-based with 256 entries (fast), or runtime calculation (minimal memory).
 *
 * @param[in] Crc_DataPtr: Pointer to input data buffer.
 * @param[in] Crc_Length: Number of bytes to process.
 * @param[in] Crc_StartValue32: Starting CRC value (previous result for continued calculation).
 * @param[in] Crc_IsFirstCall: TRUE for first call (uses initial value 0xFFFFFFFF), FALSE for continuation.
 * @param[in] Mode: Calculation method:
 *                  - CRC_TABLE_16_BYTE_MODE: Use 16-entry lookup table
 *                  - CRC_TABLE_256_BYTE_MODE: Use 256-entry lookup table
 *                  - CRC_RUNTIME_MODE: Calculate without lookup table
 *
 * @return Calculated 32-bit CRC value with final XOR 0xFFFFFFFF applied.
 *
 * @pre
 *     Crc_DataPtr must not be NULL.
 *     Mode must be a valid CRC calculation mode.
 *
 * @post
 *     None.
 *
 * @note
 *     This is a pure software implementation, no hardware required.
 *     Reentrancy: Reentrant.
 *     Compatible with IEEE-802.3 CRC32 Ethernet standard.
 *     DES ID: DES_CRC_API_211
 */
uint32 Crc_Hal_CalculateCRC32
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint32 Crc_StartValue32,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief Calculates 32-bit CRC using software implementation with P4 polynomial.
 *
 * @details
 * This function performs CRC32P4 calculation using software algorithm with
 * reflected polynomial 0xC8DF352F (reflected from 0xF4ACFB13). The P4 variant
 * is specified in AUTOSAR standard. The function uses a reflected algorithm
 * processing data LSB first. It supports three calculation modes: table-based
 * with 16 entries (compact), table-based with 256 entries (fast), or runtime
 * calculation (minimal memory).
 *
 * @param[in] Crc_DataPtr: Pointer to input data buffer.
 * @param[in] Crc_Length: Number of bytes to process.
 * @param[in] Crc_StartValue32: Starting CRC value (previous result for continued calculation).
 * @param[in] Crc_IsFirstCall: TRUE for first call (uses initial value 0xFFFFFFFF), FALSE for continuation.
 * @param[in] Mode: Calculation method:
 *                  - CRC_TABLE_16_BYTE_MODE: Use 16-entry lookup table
 *                  - CRC_TABLE_256_BYTE_MODE: Use 256-entry lookup table
 *                  - CRC_RUNTIME_MODE: Calculate without lookup table
 *
 * @return Calculated 32-bit CRC value with final XOR 0xFFFFFFFF applied.
 *
 * @pre
 *     Crc_DataPtr must not be NULL.
 *     Mode must be a valid CRC calculation mode.
 *
 * @post
 *     None.
 *
 * @note
 *     This is a pure software implementation, no hardware required.
 *     Reentrancy: Reentrant.
 *     Complies with AUTOSAR CRC SWS specification.
 *     DES ID: DES_CRC_API_212
 */
uint32 Crc_Hal_CalculateCRC32P4
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint32 Crc_StartValue32,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/**
 * @brief Calculates 64-bit CRC using software implementation (CRC-64-ECMA).
 *
 * @details
 * This function performs CRC64 calculation using software algorithm with
 * reflected polynomial 0xC96C5795D7870F42 (reflected from 0x42F0E1EBA9EA3693,
 * CRC-64-ECMA standard). The function uses a reflected algorithm processing
 * data LSB first. It supports three calculation modes: table-based with 16
 * entries (compact), table-based with 256 entries (fast), or runtime calculation
 * (minimal memory).
 *
 * @param[in] Crc_DataPtr: Pointer to input data buffer.
 * @param[in] Crc_Length: Number of bytes to process.
 * @param[in] Crc_StartValue64: Starting CRC value (previous result for continued calculation).
 * @param[in] Crc_IsFirstCall: TRUE for first call (uses initial value 0xFFFFFFFFFFFFFFFF), FALSE for continuation.
 * @param[in] Mode: Calculation method:
 *                  - CRC_TABLE_16_BYTE_MODE: Use 16-entry lookup table
 *                  - CRC_TABLE_256_BYTE_MODE: Use 256-entry lookup table
 *                  - CRC_RUNTIME_MODE: Calculate without lookup table
 *
 * @return Calculated 64-bit CRC value with final XOR 0xFFFFFFFFFFFFFFFF applied.
 *
 * @pre
 *     Crc_DataPtr must not be NULL.
 *     Mode must be a valid CRC calculation mode.
 *
 * @post
 *     None.
 *
 * @note
 *     This is a pure software implementation, no hardware required.
 *     Reentrancy: Reentrant.
 *     Based on CRC-64-ECMA standard.
 *     DES ID: DES_CRC_API_213
 */
uint64 Crc_Hal_CalculateCRC64
(
    const uint8 *Crc_DataPtr,
    uint32 Crc_Length,
    uint64 Crc_StartValue64,
    boolean Crc_IsFirstCall,
    uint8 Mode
);

/* ======================================  Functions definition  ==================================== */

/*==================================================================================================
 *                                        GLOBAL FUNCTIONS
==================================================================================================*/
#ifdef __cplusplus
}
#endif

#endif /* CRC_HAL_H */
