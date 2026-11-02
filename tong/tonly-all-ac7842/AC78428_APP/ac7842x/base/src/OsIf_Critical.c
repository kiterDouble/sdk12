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

/**
*   @file  OsIf_Critical.c
*
*   @brief This file provides default OsIf critial API implement.
*/

/*==============================================INCLUDE FILES=======================================*/
#include "OsIf_Critical.h"
#include "OsIf_Irq.h"

#if (CRITICAL_TIME_STATISTIC_EN == STD_ON)
#include "Perf.h"
#endif
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/*PRQA S 0342 ++ # AMCE_SYS_0342*/
#if (CRITICAL_TIME_STATISTIC_EN == STD_ON)
#define CRITICAL_ENTER(AREA_ID) void __attribute__((weak)) SchM_Enter_##AREA_ID##_ProtectDataArea(const char ch[]) \
{ \
    OsIf_SuspendAllInterrupts(); \
    Perf_Reset(); \
    PERF_START();   \
}
#define CRITICAL_EXIT(AREA_ID) void __attribute__((weak)) SchM_Exit_##AREA_ID##_ProtectDataArea(const char ch[]) \
{ \
    PERF_END(ch); \
    OsIf_ResumeAllInterrupts(); \
}
#else
#define CRITICAL_ENTER(AREA_ID) void __attribute__((weak)) SchM_Enter_##AREA_ID##_ProtectDataArea(void) \
{ \
    OsIf_SuspendAllInterrupts(); \
}
#define CRITICAL_EXIT(AREA_ID) void __attribute__((weak)) SchM_Exit_##AREA_ID##_ProtectDataArea(void) \
{ \
    OsIf_ResumeAllInterrupts(); \
}
#endif
/*PRQA S 0342 -- # AMCE_SYS_0342*/
/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

/*========================================GLOBAL FUNCTIONS==========================================*/

/**
 *  User need to replace the default implementation with the optimal implementation for their scenarios.
 *
 *
 */

CRITICAL_ENTER(CAN_MCAL_ID1)
CRITICAL_EXIT(CAN_MCAL_ID1)
CRITICAL_ENTER(WDG_HAL_CS_ID1)
CRITICAL_EXIT(WDG_HAL_CS_ID1)
CRITICAL_ENTER(WDG_HAL_CS_ID2)
CRITICAL_EXIT(WDG_HAL_CS_ID2)
CRITICAL_ENTER(WDG_HAL_CS_ID3)
CRITICAL_EXIT(WDG_HAL_CS_ID3)
CRITICAL_ENTER(DMA_HAL_ID1)
CRITICAL_EXIT(DMA_HAL_ID1)
CRITICAL_ENTER(DMA_HAL_ID2)
CRITICAL_EXIT(DMA_HAL_ID2)
CRITICAL_ENTER(ICU_MCAL_ID1)
CRITICAL_EXIT(ICU_MCAL_ID1)
CRITICAL_ENTER(PWM_MCAL_ID1)
CRITICAL_EXIT(PWM_MCAL_ID1)
CRITICAL_ENTER(CMU_HAL_ID1)
CRITICAL_EXIT(CMU_HAL_ID1)
CRITICAL_ENTER(FLS_HAL_ID1)
CRITICAL_EXIT(FLS_HAL_ID1)
CRITICAL_ENTER(FLS_HAL_ID2)
CRITICAL_EXIT(FLS_HAL_ID2)
CRITICAL_ENTER(FLS_HAL_ID3)
CRITICAL_EXIT(FLS_HAL_ID3)
CRITICAL_ENTER(UART_HAL_ID1)
CRITICAL_EXIT(UART_HAL_ID1)
CRITICAL_ENTER(CRYPTO_MCAL_ID)
CRITICAL_EXIT(CRYPTO_MCAL_ID)
CRITICAL_ENTER(FLSTST_HAL_ID1)
CRITICAL_EXIT(FLSTST_HAL_ID1)
CRITICAL_ENTER(EEP_HAL_ID1)
CRITICAL_EXIT(EEP_HAL_ID1)

/*PRQA S 2053 ++ # AMCE_SYS_2053*/
/**
 * If not use function macro to protect critical section like above, can define function by yourself in below
 * because these function is weak, just need reload this function is ok
void __attribute__((weak)) SchM_Enter_WDG_HAL_CS_ID1_ProtectDataArea(void)
{
    OsIf_SuspendAllInterrupts();
}

void __attribute__((weak)) SchM_Exit_WDG_HAL_CS_ID1_ProtectDataArea(void)
{
    OsIf_ResumeAllInterrupts();
}
*/
/*PRQA S 2053 -- # AMCE_SYS_2053*/
