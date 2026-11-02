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
 * @brief specification of PLATFORM Driver.
 */

#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/**
* @brief          8bit Type Processor
* @implements     CPU_TYPE_enumeration
*/
#define CPU_TYPE_8 8

/**
* @brief          16bit Type Processor
* @implements     CPU_TYPE_enumeration
*/
#define CPU_TYPE_16 16

/**
* @brief          32bit Type Processor
* @implements     CPU_TYPE_enumeration
*/
#define CPU_TYPE_32 32

/**
* @brief          64bit Type Processor
* @implements     CPU_TYPE_enumeration
*/
#define CPU_TYPE_64 64

/**
* @brief          MSB First Processor
* @implements     CPU_BIT_ORDER_enumeration
*/
#define MSB_FIRST 0

/**
* @brief          LSB First Processor
* @implements     CPU_BIT_ORDER_enumeration
*/
#define LSB_FIRST 1

/**
* @brief          HIGH_BYTE_FIRST Processor
* @implements     CPU_BYTE_ORDER_enumeration
*/
#define HIGH_BYTE_FIRST 0

/**
* @brief          LOW_BYTE_FIRST Processor
* @implements     CPU_BYTE_ORDER_enumeration
*/
#define LOW_BYTE_FIRST 1

/**
* @brief          Processor type
* @implements     CPU_TYPE_enumeration
*/
#define CPU_TYPE (CPU_TYPE_32)
/**
* @brief          Bit order on register level.
* @implements     CPU_BIT_ORDER_enumeration
*/
#define CPU_BIT_ORDER  (LSB_FIRST)

/**
* @brief The byte order on memory level shall be indicated in the platform types header file using
*        the symbol CPU_BYTE_ORDER.
* @implements     CPU_BYTE_ORDER_enumeration
*/
#define CPU_BYTE_ORDER (LOW_BYTE_FIRST)

#ifndef TRUE
/**
* @brief Boolean true value
* @implements TRUE_FALSE_enumeration
*/
#define TRUE 1U
#endif
#ifndef FALSE
/**
* @brief Boolean false value
* @implements TRUE_FALSE_enumeration
*/
#define FALSE 0U
#endif

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
#if (CPU_TYPE == CPU_TYPE_64)
/**
* @brief The standard AUTOSAR type boolean shall be implemented on basis of an eight bits long
*        unsigned integer.
* @implements boolean_type
*/
typedef unsigned char boolean;

/**
* @brief Unsigned 8 bit integer with range of 0 ..+255 (0x00..0xFF) -
*        8 bit
* @implements uint8_type
*/
typedef unsigned char uint8;

/**
* @brief Unsigned 16 bit integer with range of 0 ..+65535 (0x0000..0xFFFF) -
*        16 bit
* @implements uint16_type
*/
typedef unsigned short uint16;

/**
* @brief Unsigned 32 bit integer with range of 0 ..+4294967295 (0x00000000..0xFFFFFFFF) -
*        32 bit
* @implements uint32_type
*/
typedef unsigned int uint32;

/**
* @brief Unsigned 64 bit integer with range of 0..18446744073709551615 (0x0000000000000000..0xFFFFFFFFFFFFFFFF)-
*        64 bit
*
*/
typedef unsigned long long uint64;

/**
* @brief Signed 8 bit integer with range of -128 ..+127 (0x80..0x7F) -
*        7 bit + 1 sign bit
* @implements sint8_type
*/
typedef signed char sint8;

/**
* @brief Signed 16 bit integer with range of -32768 ..+32767 (0x8000..0x7FFF) -
*        15 bit + 1 sign bit
* @implements sint16_type
*/
typedef signed short sint16;

/**
* @brief Signed 32 bit integer with range of -2147483648.. +2147483647 (0x80000000..0x7FFFFFFF) -
*        31 bit + 1 sign bit
* @implements sint32_type
*/
typedef signed int sint32;

/**
* @brief Signed 64 bit integer with range of -9223372036854775808..9223372036854775807 (0x8000000000000000..0x7FFFFFFFFFFFFFFF )-
*        63 bit + 1 sign bit
*
*/
typedef signed long long sint64;

/**
* @brief Unsigned integer at least 8 bit long. Range of at least 0 ..+255 (0x00..0xFF) -
*        8 bit
* @implements uint8_least_type
*/
typedef unsigned int uint8_least;

/**
* @brief  Unsigned integer at least 16 bit long. Range of at least 0 ..+65535 (0x0000..0xFFFF) -
*         16 bit
* @implements uint16_least_type
*/
typedef unsigned int uint16_least;

/**
* @brief Unsigned integer at least 32 bit long. Range of at least 0 ..+4294967295
*       (0x00000000..0xFFFFFFFF) - 32 bit
* @implements uint32_least_type
*/
typedef unsigned int uint32_least;

/**
* @brief Signed integer at least 8 bit long. Range - at least -128 ..+127.
*        At least 7 bit + 1 bit sign
* @implements sint8_least_type
*/
typedef signed int sint8_least;

/**
* @brief Signed integer at least 16 bit long. Range - at least -32768 ..+32767.
*        At least 15 bit + 1 bit sign
* @implements sint16_least_type
*/
typedef signed int sint16_least;

/**
* @brief Signed integer at least 32 bit long. Range - at least -2147483648.. +2147483647.
*       At least 31 bit + 1 bit sign
* @implements sint32_least_type
*/
typedef signed int sint32_least;

/**
* @brief 32bit long floating point data type
* @implements float32_type
*/
typedef float float32;

/**
* @brief 64bit long floating point data type
* @implements float64_type
*/
typedef double float64;

#elif (CPU_TYPE == CPU_TYPE_32)

/**
* @brief The standard AUTOSAR type boolean shall be implemented on basis of an eight bits long
*        unsigned integer.
* @implements boolean_type
*/
typedef unsigned char boolean; /*cstat !MISRAC2012-Dir-4.6_b*/

/**
* @brief Unsigned 8 bit integer with range of 0 ..+255 (0x00..0xFF) -
*        8 bit
* @implements uint8_type
*/
typedef unsigned char uint8;

/**
* @brief Unsigned 16 bit integer with range of 0 ..+65535 (0x0000..0xFFFF) -
*        16 bit
* @implements uint16_type
*/
typedef unsigned short uint16;

/**
* @brief Unsigned 32 bit integer with range of 0 ..+4294967295 (0x00000000..0xFFFFFFFF) -
*        32 bit
* @implements uint32_type
*/
typedef unsigned int uint32;

/**
* @brief Unsigned 64 bit integer with range of 0..18446744073709551615 (0x0000000000000000..0xFFFFFFFFFFFFFFFF)-
*        64 bit
*
*/
typedef unsigned long long uint64;

/**
* @brief Signed 8 bit integer with range of -128 ..+127 (0x80..0x7F) -
*        7 bit + 1 sign bit
* @implements sint8_type
*/
typedef signed char sint8;

/**
* @brief Signed 16 bit integer with range of -32768 ..+32767 (0x8000..0x7FFF) -
*        15 bit + 1 sign bit
* @implements sint16_type
*/
typedef signed short sint16;

/**
* @brief Signed 32 bit integer with range of -2147483648.. +2147483647 (0x80000000..0x7FFFFFFF) -
*        31 bit + 1 sign bit
* @implements sint32_type
*/
typedef signed int sint32;

/**
* @brief Signed 64 bit integer with range of -9223372036854775808..9223372036854775807 (0x8000000000000000..0x7FFFFFFFFFFFFFFF )-
*        63 bit + 1 sign bit
*
*/
typedef signed long long sint64;     //PRQA S 1535 # AMCE_SYS_1535
/**
* @brief Unsigned integer at least 8 bit long. Range of at least 0 ..+255 (0x00..0xFF) -
*        8 bit
* @implements uint8_least_type
*/
typedef unsigned long uint8_least;     //PRQA S 1535 # AMCE_SYS_1535

/**
* @brief  Unsigned integer at least 16 bit long. Range of at least 0 ..+65535 (0x0000..0xFFFF) -
*         16 bit
* @implements uint16_least_type
*/
typedef unsigned long uint16_least;     //PRQA S 1535 # AMCE_SYS_1535

/**
* @brief Unsigned integer at least 32 bit long. Range of at least 0 ..+4294967295
*       (0x00000000..0xFFFFFFFF) - 32 bit
* @implements uint32_least_type
*/
typedef unsigned long uint32_least;     //PRQA S 1535 # AMCE_SYS_1535

/**
* @brief Signed integer at least 8 bit long. Range - at least -128 ..+127.
*        At least 7 bit + 1 bit sign
* @implements sint8_least_type
*/
typedef signed long sint8_least;     //PRQA S 1535 # AMCE_SYS_1535

/**
* @brief Signed integer at least 16 bit long. Range - at least -32768 ..+32767.
*        At least 15 bit + 1 bit sign
* @implements sint16_least_type
*/
typedef signed long sint16_least;     //PRQA S 1535 # AMCE_SYS_1535

/**
* @brief Signed integer at least 32 bit long. Range - at least -2147483648.. +2147483647.
*       At least 31 bit + 1 bit sign
* @implements sint32_least_type
*/
typedef signed long sint32_least;    //PRQA S 1535 # AMCE_SYS_1535

/**
* @brief 32bit long floating point data type
* @implements float32_type
*/
typedef float float32;

/**
* @brief 64bit long floating point data type
* @implements float64_type
*/
typedef double float64;    //PRQA S 1535 # AMCE_SYS_1535

#elif (CPU_TYPE == CPU_TYPE_16)
/**
* @brief The standard AUTOSAR type boolean shall be implemented on basis of an eight bits long
*        unsigned integer.
* @implements boolean_type
*/
typedef unsigned char boolean;

/**
* @brief Unsigned 8 bit integer with range of 0 ..+255 (0x00..0xFF) -
*        8 bit
* @implements uint8_type
*/
typedef unsigned char uint8;

/**
* @brief Unsigned 16 bit integer with range of 0 ..+65535 (0x0000..0xFFFF) -
*        16 bit
* @implements uint16_type
*/
typedef unsigned short uint16;

/**
* @brief Unsigned 32 bit integer with range of 0 ..+4294967295 (0x00000000..0xFFFFFFFF) -
*        32 bit
* @implements uint32_type
*/
typedef unsigned long uint32;

/**
* @brief Unsigned 64 bit integer with range of 0..18446744073709551615 (0x0000000000000000..0xFFFFFFFFFFFFFFFF)-
*        64 bit
*
*/
typedef unsigned long long uint64;

/**
* @brief Signed 8 bit integer with range of -128 ..+127 (0x80..0x7F) -
*        7 bit + 1 sign bit
* @implements sint8_type
*/
typedef signed char sint8;

/**
* @brief Signed 16 bit integer with range of -32768 ..+32767 (0x8000..0x7FFF) -
*        15 bit + 1 sign bit
* @implements sint16_type
*/
typedef signed short sint16;

/**
* @brief Signed 32 bit integer with range of -2147483648.. +2147483647 (0x80000000..0x7FFFFFFF) -
*        31 bit + 1 sign bit
* @implements sint32_type
*/
typedef signed long sint32;

/**
* @brief Signed 64 bit integer with range of -9223372036854775808..9223372036854775807 (0x8000000000000000..0x7FFFFFFFFFFFFFFF )-
*        63 bit + 1 sign bit
*
*/
typedef signed long long sint64;

/**
* @brief Unsigned integer at least 8 bit long. Range of at least 0 ..+255 (0x00..0xFF) -
*        8 bit
* @implements uint8_least_type
*/
typedef unsigned long uint8_least;

/**
* @brief  Unsigned integer at least 16 bit long. Range of at least 0 ..+65535 (0x0000..0xFFFF) -
*         16 bit
* @implements uint16_least_type
*/
typedef unsigned long uint16_least;

/**
* @brief Unsigned integer at least 32 bit long. Range of at least 0 ..+4294967295
*       (0x00000000..0xFFFFFFFF) - 32 bit
* @implements uint32_least_type
*/
typedef unsigned long uint32_least;

/**
* @brief Signed integer at least 8 bit long. Range - at least -128 ..+127.
*        At least 7 bit + 1 bit sign
* @implements sint8_least_type
*/
typedef signed long sint8_least;

/**
* @brief Signed integer at least 16 bit long. Range - at least -32768 ..+32767.
*        At least 15 bit + 1 bit sign
* @implements sint16_least_type
*/
typedef signed long sint16_least;

/**
* @brief Signed integer at least 32 bit long. Range - at least -2147483648.. +2147483647.
*       At least 31 bit + 1 bit sign
* @implements sint32_least_type
*/
typedef signed long sint32_least;

/**
* @brief 32bit long floating point data type
* @implements float32_type
*/
typedef float float32;

/**
* @brief 64bit long floating point data type
* @implements float64_type
*/
typedef double float64;

#endif
/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

#ifdef __cplusplus
}
#endif

#endif
