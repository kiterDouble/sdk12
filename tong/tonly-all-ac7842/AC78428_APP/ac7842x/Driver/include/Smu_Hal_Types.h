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

/*!
 * @file Smu_Hal_Types.h
 *
 * @brief This file provides Smu hal types header.
 *
 */

#ifndef SMU_HAL_TYPES_H
#define SMU_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*!
 * @brief SMU module configuration structure.
 */
typedef struct
{
    uint8 Reset_Count; /*!< SMU reset threshold count. */
    uint32 Reset_Spfs; /*!< Hardware and Software single point faults enable. */
    uint32 Reset_Lfs; /*!< Hardware and Software latent faults enable.       */
#if defined (AC7843X)
    uint8 Reset_HsmCount; /*!< SMU HSM reset threshold count. */
    uint32 Reset_HsmSpfs; /*!< HSM Hardware and Software single point faults enable. */
    uint32 Irq_HsmSpfSrc; /*!< SMU HSM SPF interrupt sources.      */
#endif
#if defined (AC7840E) || defined (AC7842X)  ||  defined (AC7843X)
    uint32 Irq_SpfSrc; /*!< SMU SPF interrupt sources.      */
    uint32 Irq_LfSrc; /*!< SMU LF interrupt sources.      */
    Hal_CallbackType Irq_Callback; /*!< SMU interrupt callback function. */
#endif
#if defined (AC7840E)
    uint32 Fsp_Ctrl;
    uint32 Fsp_SpfSrc; /*!< SMU SPF FSP sources.     */
    uint32 Fsp_LfSrc; /*!< SMU LF FSP sources.      */
#endif
} Smu_ConfigType;

/*!
 * @brief SMU module information structure.
 */
typedef struct
{
    uint8 Count;                /*!< SMU reset count. */
    uint32 Spfs;                 /*!< Hardware and Software single point faults status.*/
    uint32 Lfs;                  /*!< Hardware and Software latent faults status.      */
    uint32 SwSpfs;               /*!< Software latent faults status. */
    uint32 SwLfs;                /*!< Software latent faults status. */
#if defined (AC7843X)
    uint8 HsmCount;             /*!< SMU HSM reset count. */
    uint32 HsmSpfs;              /*!< HSM Hardware and Software single point faults status.*/
    uint32 HsmSwSpfs;            /*!< HSM Software latent faults status. */
#endif
#if defined (AC7840E)
    uint8 FspState;             /*!< FSP state: 1 for fault state, 0 for non-fault state. */
#endif
} Smu_FaultInfoType;

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* SMU_HAL_TYPES_H */
