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
#ifndef OSIF_CRITICAL_H
#define OSIF_CRITICAL_H

/**
*   @file OsIf_Critical.h
*   @brief This file provides extern OsIf  API.
*
*/

#ifdef __cplusplus
extern "C" {
#endif /* endif of __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Std_Types.h"
#include "Conf_AC784xx.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/
#define OSIF_CRITICAL_SW_MAJOR_VERSION             (1U)
#define OSIF_CRITICAL_SW_MINOR_VERSION             (1U)
#define OSIF_CRITICAL_SW_PATCH_VERSION             (0U)

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/** @brief  ID1 for Watchdog Critical Section */
#define WDG_HAL_CS_ID1

/** @brief  ID2 for Watchdog Critical Section */
#define WDG_HAL_CS_ID2

/** @brief  ID3 for Watchdog Critical Section */
#define WDG_HAL_CS_ID3

/** @brief  ID1 for DMA Critical Section */
#define DMA_HAL_ID1

/** @brief  ID2 for DMA Critical Section */
#define DMA_HAL_ID2

/** @brief  ID1 for PWM Critical Section */
#define PWM_MCAL_ID1

/** @brief  ID1 for ICU Critical Section */
#define ICU_MCAL_ID1

/** @brief  ID for Crypto Critical Section */
#define CRYPTO_MCAL_ID

/** @brief  ID1 for CMU Critical Section */
#define CMU_HAL_ID1

/** @brief  ID1 for FLS Critical Section */
#define FLS_HAL_ID1
#define FLS_HAL_ID2
#define FLS_HAL_ID3

/** @brief  ID1 for FLS Critical Section */
#define UART_HAL_ID1

/** @brief  ID1 for FlsTst Critical Section */
#define FLSTST_HAL_ID1

/** @brief  ID1 for EEP Critical Section */
#define EEP_HAL_ID1

/** @brief  ID1 for CAN Critical Section */
#define CAN_MCAL_ID1

/*PRQA S 0342 ++ # AMCE_SYS_0342 */
#if (CRITICAL_TIME_STATISTIC_EN == STD_ON)
/** @brief enter ctirical protect area */
#define OSIF_ENTER_CRITICAL(AREA_ID)          SchM_Enter_##AREA_ID##_ProtectDataArea(__func__)

/** @brief exit ctirical protect area */
#define OSIF_EXIT_CRITICAL(AREA_ID)           SchM_Exit_##AREA_ID##_ProtectDataArea(__func__)

#define OSIF_ENTER_CRITICAL_PROTOTYPES(AREA_ID)       void SchM_Enter_##AREA_ID##_ProtectDataArea(const char ch[])
#define OSIF_EXIT_CRITICAL_PROTOTYPES(AREA_ID)        void SchM_Exit_##AREA_ID##_ProtectDataArea(const char ch[])
#else
/** @brief enter ctirical protect area */
#define OSIF_ENTER_CRITICAL(AREA_ID)          SchM_Enter_##AREA_ID##_ProtectDataArea()

/** @brief exit ctirical protect area */
#define OSIF_EXIT_CRITICAL(AREA_ID)           SchM_Exit_##AREA_ID##_ProtectDataArea()

#define OSIF_ENTER_CRITICAL_PROTOTYPES(AREA_ID)       void SchM_Enter_##AREA_ID##_ProtectDataArea(void)
#define OSIF_EXIT_CRITICAL_PROTOTYPES(AREA_ID)        void SchM_Exit_##AREA_ID##_ProtectDataArea(void)
#endif
/*PRQA S 0342 -- # AMCE_SYS_0342*/
/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
OSIF_ENTER_CRITICAL_PROTOTYPES(WDG_HAL_CS_ID1);
OSIF_EXIT_CRITICAL_PROTOTYPES(WDG_HAL_CS_ID1);
OSIF_ENTER_CRITICAL_PROTOTYPES(WDG_HAL_CS_ID2);
OSIF_EXIT_CRITICAL_PROTOTYPES(WDG_HAL_CS_ID2);
OSIF_ENTER_CRITICAL_PROTOTYPES(WDG_HAL_CS_ID3);
OSIF_EXIT_CRITICAL_PROTOTYPES(WDG_HAL_CS_ID3);
OSIF_ENTER_CRITICAL_PROTOTYPES(DMA_HAL_ID1);
OSIF_EXIT_CRITICAL_PROTOTYPES(DMA_HAL_ID1);
OSIF_ENTER_CRITICAL_PROTOTYPES(DMA_HAL_ID2);
OSIF_EXIT_CRITICAL_PROTOTYPES(DMA_HAL_ID2);
OSIF_ENTER_CRITICAL_PROTOTYPES(PWM_MCAL_ID1);
OSIF_EXIT_CRITICAL_PROTOTYPES(PWM_MCAL_ID1);
OSIF_ENTER_CRITICAL_PROTOTYPES(ICU_MCAL_ID1);
OSIF_EXIT_CRITICAL_PROTOTYPES(ICU_MCAL_ID1);
OSIF_ENTER_CRITICAL_PROTOTYPES(CMU_HAL_ID1);
OSIF_EXIT_CRITICAL_PROTOTYPES(CMU_HAL_ID1);
OSIF_ENTER_CRITICAL_PROTOTYPES(FLS_HAL_ID1);
OSIF_EXIT_CRITICAL_PROTOTYPES(FLS_HAL_ID1);
OSIF_ENTER_CRITICAL_PROTOTYPES(FLS_HAL_ID2);
OSIF_EXIT_CRITICAL_PROTOTYPES(FLS_HAL_ID2);
OSIF_ENTER_CRITICAL_PROTOTYPES(FLS_HAL_ID3);
OSIF_EXIT_CRITICAL_PROTOTYPES(FLS_HAL_ID3);
OSIF_ENTER_CRITICAL_PROTOTYPES(UART_HAL_ID1);
OSIF_EXIT_CRITICAL_PROTOTYPES(UART_HAL_ID1);
OSIF_ENTER_CRITICAL_PROTOTYPES(CRYPTO_MCAL_ID);
OSIF_EXIT_CRITICAL_PROTOTYPES(CRYPTO_MCAL_ID);
OSIF_ENTER_CRITICAL_PROTOTYPES(FLSTST_HAL_ID1);
OSIF_EXIT_CRITICAL_PROTOTYPES(FLSTST_HAL_ID1);
OSIF_ENTER_CRITICAL_PROTOTYPES(EEP_HAL_ID1);
OSIF_EXIT_CRITICAL_PROTOTYPES(EEP_HAL_ID1);
OSIF_ENTER_CRITICAL_PROTOTYPES(CAN_MCAL_ID1);
OSIF_EXIT_CRITICAL_PROTOTYPES(CAN_MCAL_ID1);
/*========================================GLOBAL FUNCTIONS==========================================*/

#ifdef __cplusplus
}
#endif /* endif of __cplusplus */

#endif /* OSIF_CRITICAL_H */
