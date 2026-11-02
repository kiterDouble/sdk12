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
 * @brief specification of Standard Driver.
 */
#ifndef STD_TYPES_H
#define STD_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Platform_Types.h"
#include "Compiler.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/**
* @brief Physical state 5V or 3.3V
* @implements SymbolDefinitions_enumeration
*/
#define STD_HIGH    0x01U

/**
* @brief Physical state 0V.
* @implements SymbolDefinitions_enumeration
*/
#define STD_LOW     0x00U

/**
* @brief Logical state active.
* @implements SymbolDefinitions_enumeration
*/
#define STD_ACTIVE  0x01U

/**
* @brief Logical state idle.
* @implements SymbolDefinitions_enumeration
*/
#define STD_IDLE    0x00U

/**
* @brief ON State.
* @implements SymbolDefinitions_enumeration
*/
#define STD_ON      0x01U

/**
* @brief OFF state.
* @implements SymbolDefinitions_enumeration
*/
#define STD_OFF     0x00U

/**
* @brief Return code for failure/error.
* @implements SymbolDefinitions_enumeration
*/
#define E_NOT_OK    0x01U

/**
* @brief The compiler abstraction shall provide the NULL_PTR define with a void pointer to zero
*        definition.
*/
#define NULL_PTR ((void *)0)
/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/**
* @brief Because E_OK is already defined within OSEK, the symbol E_OK has to be shared. To avoid
*        name clashes and redefinition problems, the symbols have to be defined in the following way
*       (approved within implementation).
*/
#ifndef STATUSTYPEDEFINED
    #define STATUSTYPEDEFINED
    /**
    * @brief Success return code
    */
    #define E_OK   0x00U
    typedef unsigned char StatusType; /* OSEK compliance */  //PRQA S 1535 # AMCE_SYS_1535
#endif
#define    E_BUSY  2U
/**
* @brief This type can be used as standard API return type which is shared between the RTE and the
*        BSW modules.
* @implements Std_ReturnType_type
*/
typedef uint8 Std_ReturnType;

/**
* @brief This type shall be used to request the version of a BSW module using the
*       <ModuleName>_GetVersionInfo() function.
* @implements Std_VersionInfoType_structure
*/
typedef struct /*PRQA S 3630 # AMCE_QAC_3630*/
{
    uint16 vendorID;              /**< @brief vendor ID */
    uint16 moduleID;              /**< @brief BSW module ID */
    uint8 sw_major_version;       /**< @brief BSW module software major version */
    uint8 sw_minor_version;       /**< @brief BSW module software minor version */
    uint8 sw_patch_version;       /**< @brief BSW module software patch version */
} Std_VersionInfoType;

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

typedef uint8 Std_TransformerErrorCode;
typedef uint8 Std_TransformerClass;
#define STD_TRANSFORMER_UNSPECIFIED 0x00
#define STD_TRANSFORMER_SERIALIZER  0x01
#define STD_TRANSFORMER_SAFETY      0x02
#define STD_TRANSFORMER_SECURITY    0x03
#define STD_TRANSFORMER_CUSTOM      0xFF

/**
 * @brief     Std_TransformerError represents a transformer error in the context of a certain transformer chain.
 */
typedef struct
{
    Std_TransformerErrorCode errorCode;
    Std_TransformerClass transformerClass;
} Std_TransformerError;                              //PRQA S 1535 # AMCE_SYS_1535

/**
 * @brief     The data type Std_TransformerForwardCode represents a forwarded transformer
 *            codein the context of a certain transformer chain
 */
typedef uint8 Std_TransformerForwardCode;               //PRQA S 1535 # AMCE_SYS_1535
#define E_SAFETY_INVALID_REP 0x01
#define E_SAFETY_INVALID_CRC 0x02
#define E_SAFETY_INVALID_SEQ 0x03
#ifdef __cplusplus
}
#endif

#endif
