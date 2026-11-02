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
 * AutoChips Inc. (C) 2024. All rights reserved.
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
#ifndef OSIF_IRQ_H
#define OSIF_IRQ_H

/**
*   @file OsIf_Irq.h
*   @brief This file provides extern OsIf Irq API.
*
*/

#ifdef __cplusplus
extern "C" {
#endif /* endif of __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Std_Types.h"
#include "Device_Register.h"
#if defined(USING_AUTOSAR_OS)
#include "Os.h"
#endif
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/
#define OSIF_IRQ_SW_MAJOR_VERSION             (1U)
#define OSIF_IRQ_SW_MINOR_VERSION             (0U)
#define OSIF_IRQ_SW_PATCH_VERSION             (1U)

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
extern volatile uint32 OsIf_PriMaskValue;
extern volatile sint32 OsIf_CriticalNesting;
/*============================================FUNCTION PROTOTYPES===================================*/

/*========================================GLOBAL FUNCTIONS==========================================*/
/**
 * @brief Resume all interrupts of system
 * @note Function ID: DES_OSIF_API_203
 * @return void
 */
LOCAL_INLINE void OsIf_SuspendAllInterrupts(void)
{
#if defined(USING_AUTOSAR_OS)
    SuspendAllInterrupts();
#else
    uint32 PriMask = __get_PRIMASK();

    __disable_irq();

    if (OsIf_CriticalNesting == 0) /* Save the primask only at the first time. */
    {
        OsIf_PriMaskValue = PriMask;
    }

    OsIf_CriticalNesting += 1;
#endif
}

/**
 * @brief Resume all interrupts of system
 * @note Function ID: DES_OSIF_API_202
 * @return void
 */
LOCAL_INLINE void OsIf_ResumeAllInterrupts(void)
{
#if defined(USING_AUTOSAR_OS)
    ResumeAllInterrupts();
#else
    OsIf_CriticalNesting -= 1;

    if (OsIf_CriticalNesting <= 0) /* Resume count may more than suspend count. */
    {
        __set_PRIMASK(OsIf_PriMaskValue);
    }
#endif
}

#ifdef __cplusplus
}
#endif /* endif of __cplusplus */

#endif /* OSIF_IRQ_H */
