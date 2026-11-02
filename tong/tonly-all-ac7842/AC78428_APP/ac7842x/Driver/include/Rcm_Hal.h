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
 * @file Rcm_Hal.h
 *
 * @brief This file provides extern Hal Rcm api.
 *
 */

#ifndef RCM_HAL_H
#define RCM_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* endif of __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Rcm_Reg.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/*!< rcm reset filter max value     */
#define RESET_FILTER_MAX_VALUE (127UL)

/** @brief rcm irq control internal(STD_ON) or external(STD_OFF)*/
#define RCM_IRQ_CONTROL_INTERNAL           (STD_ON)
/*===================================================ENUMS==========================================*/

/*!
 * @brief Reset delay time enumeration.
 */
typedef enum
{
    RCM_RESET_DELAY_8_128KHz = 0U, /*!< Reset delay 8   cycles * 128K     */
    RCM_RESET_DELAY_32_128KHz, /*!< Reset delay 32  cycles * 128K     */
    RCM_RESET_DELAY_128_128KHz, /*!< Reset delay 128 cycles * 128K     */
    RCM_RESET_DELAY_512_128KHz /*!< Reset delay 512 cycles * 128K     */
} Rcm_DelayType;

typedef enum
{
    RCM_RESET_STATE_ASSERT = 0U, /*!< Assert                           */
    RCM_RESET_STATE_DEASSERT = 1U, /*!< Deassert                         */
} Rcm_ResetStateType;
/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/*!
 * @brief Rcm module configuration structure.
 */
typedef struct
{
    uint32 InterruptSource; /*!< sources can generate interrupt before reset */
    uint32 ResetSource; /*!< sources can generate reset                  */
    uint8 FilterValue; /*!< filter value of external reset pid          */
    Hal_CallbackType RcmCallback; /*!< callback function                         */
    Rcm_DelayType DelayTime; /*!< delay time before reset                     */
} Rcm_ConfigType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/
/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Initialize reset module.
 * @note  Function ID: DES_MCU_API_401
 * @param[in] ConfigPtr: Rcm configuration.
 * @return void
 */
void Rcm_Hal_Init(const Rcm_ConfigType *ConfigPtr);

/*!
 * @brief Get reset statue.
 * @note  Function ID: DES_MCU_API_402
 * @return value of reset and interrupt status
 *         - BIT0:RCM_RESET_STATUS_POR_RST
 *         - BIT1:RCM_RESET_STATUS_LVR_RST
 *         - BIT2:RCM_RESET_STATUS_SW_RST
 *         - BIT3:RCM_RESET_STATUS_LOCKUP_RST
 *         - BIT4:RCM_RESET_STATUS_ACK_ERR_RST
 *         - BIT5:RCM_RESET_STATUS_WDG_RST
 *         - BIT7:RCM_RESET_STATUS_XOSC_LOSS_RST
 *         - BIT8:RCM_RESET_STATUS_PLL_UNLOCK_RST
 *         - BIT9:RCM_RESET_STATUS_VHSI_LOSS_RST
 *         - BIT10:RCM_RESET_STATUS_EXT_RST
 *         - BIT11:RCM_RESET_STATUS_SMU_ERR_RST
 *         - BIT12:RCM_RESET_STATUS_ECC2_ERR_RST
 */
uint32 Rcm_Hal_GetResetStatus(void);

/*!
 * @brief Clear All reset status.
 * @note  Function ID:DES_MCU_API_403
 * @return void
 */
void Rcm_Hal_ClearResetStatus(void);

/*!
 * @brief Assert or deassert the reset.
 * @note  Function ID:DES_MCU_API_404
 * @param[in] ResetId: Reset ID
 * @param[in] ResetState: Reset state, assert or deassert
 * @return void
 */
void Rcm_Hal_SetResetState(Rcm_ResetIDType ResetId, Rcm_ResetStateType ResetState);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* RCM_HAL_H */
/*============================================EOF===================================================*/
