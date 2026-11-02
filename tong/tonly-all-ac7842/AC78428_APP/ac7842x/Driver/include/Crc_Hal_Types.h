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
 * AutoChips Inc. (C) 2022. All rights reserved.
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
/******************************************************************************
* SPECIFICATION(S) : specification of DIO Driver, AUTOSAR Release 4.4.0
******************************************************************************/
/**

 * @file Crc_Hal_Types.h
 *
 * @brief This file provides extern Crc macro enum and structure info for hal/mcal.
 *
 */
#ifndef CRC_HAL_TYPES_H
#define CRC_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/**
 * @def CRC_TABLE_16_BYTE_MODE
 * @brief CRC calculation mode using 16-byte lookup table.
 * @note DES ID: DES_CRC_MACRO_002
 */
#define    CRC_TABLE_16_BYTE_MODE          0x0U

/**
 * @def CRC_TABLE_256_BYTE_MODE
 * @brief CRC calculation mode using 256-byte lookup table.
 * @note DES ID: DES_CRC_MACRO_003
 */
#define    CRC_TABLE_256_BYTE_MODE         0x1U

/**
 * @def CRC_RUNTIME_MODE
 * @brief CRC calculation mode at runtime without lookup table.
 * @note DES ID: DES_CRC_MACRO_004
 */
#define    CRC_RUNTIME_MODE                0x2U

/**
 * @def CRC_HARDWARE_MODE
 * @brief CRC calculation mode using hardware acceleration.
 * @note DES ID: DES_CRC_MACRO_005
 */
#define    CRC_HARDWARE_MODE               0x3U

/**************************CRC initial value**************************/
/**
 * @def CRC_INITIAL_VALUE8
 * @brief Initial value for SAE J1850 CRC8 calculation.
 * @note DES ID: DES_CRC_MACRO_006
 */
#define CRC_INITIAL_VALUE8    0xFFU

/**
 * @def CRC_INITIAL_VALUE8H2F
 * @brief Initial value for CRC8 calculation with polynomial 0x2F.
 * @note DES ID: DES_CRC_MACRO_007
 */
#define CRC_INITIAL_VALUE8H2F 0xFFU

/**
 * @def CRC_INITIAL_VALUE16
 * @brief Initial value for CRC16 calculation.
 * @note DES ID: DES_CRC_MACRO_008
 */
#define CRC_INITIAL_VALUE16   0xFFFFU

/**
 * @def CRC_INITIAL_VALUE16ARC
 * @brief Initial value for CRC16 ARC calculation.
 * @note DES ID: DES_CRC_MACRO_009
 */
#define CRC_INITIAL_VALUE16ARC   (0x0U)

/**
 * @def CRC_INITIAL_VALUE32
 * @brief Initial value for CRC32 calculation.
 * @note DES ID: DES_CRC_MACRO_010
 */
#define CRC_INITIAL_VALUE32   0xFFFFFFFFU

/**
 * @def CRC_INITIAL_VALUE64
 * @brief Initial value for CRC64 calculation.
 * @note DES ID: DES_CRC_MACRO_011
 */
#define CRC_INITIAL_VALUE64   0xFFFFFFFFFFFFFFFFULL

/**
 * @def CRC_DATA_IS_DATA
 * @brief Indicates that DATA register contains data for CRC calculation.
 * @note DES ID: DES_CRC_MACRO_012
 */
#define CRC_DATA_IS_DATA (0x0U)

/**
 * @def CRC_DATA_IS_SEED
 * @brief Indicates that DATA register contains seed value for CRC calculation.
 * @note DES ID: DES_CRC_MACRO_013
 */
#define CRC_DATA_IS_SEED (0x1U)
/*===================================================ENUMS==========================================*/
/**
 * @brief CRC protocol width enumeration.
 * @note DES ID: DES_CRC_TYP_001
 */
typedef enum
{
    CRC_PROTOCOL_16BIT = 0x00U, /*!< CRC protocol 16-bit mode */
    CRC_PROTOCOL_32BIT          /*!< CRC protocol 32-bit mode */
} Crc_ProtocolType;

/**
 * @brief CRC data transpose mode enumeration.
 * @note DES ID: DES_CRC_TYP_002
 */
typedef enum
{
    CRC_TRANSPOSE_NONE = 0x00U,  /*!< No transpose applied to CRC data */
    CRC_TRANSPOSE_BITS,          /*!< Bit-level transpose applied to CRC data */
    CRC_TRANSPOSE_BITS_BYTES,    /*!< Both bit and byte level transpose applied to CRC data */
    CRC_TRANSPOSE_BYTES          /*!< Byte-level transpose applied to CRC data */
} Crc_TransposeType;

/**
 * @brief CRC hardware configuration structure.
 * @note DES ID: DES_CRC_TYP_003
 */
/*PRQA S 3630 ++ #AMCE_QAC_3630*/
typedef struct
{
    Crc_ProtocolType Protocol;        /*!< CRC protocol width (16-bit or 32-bit) */
    Crc_TransposeType WriteTranspose; /*!< Transpose mode for input data */
    Crc_TransposeType ReadTranspose;  /*!< Transpose mode for output data */
    boolean FinalXOR;                 /*!< Enable/disable XOR operation on final result */
    uint32 Poly;                      /*!< CRC polynomial value */
    uint32 Seed;                      /*!< CRC seed/initial value */
} Crc_ConfigType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/
/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

#ifdef __cplusplus
}
#endif

#endif
