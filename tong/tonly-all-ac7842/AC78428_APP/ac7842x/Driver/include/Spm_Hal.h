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
 * @file Spm_Hal.h
 *
 * @brief This file provides extern Hal Spm api.
 *
 */

#ifndef SPM_HAL_H
#define SPM_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Spm_Reg.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/* spm irq control internal(STD_ON) or external(STD_OFF) */
#define SPM_IRQ_CONTROL_INTERNAL           (STD_ON)

/* lvd irq control internal(STD_ON) or external(STD_OFF) */
#define LVD_IRQ_CONTROL_INTERNAL           (STD_ON)

/* Stb irq control internal(STD_ON) or external(STD_OFF) */
#define STB_IRQ_CONTROL_INTERNAL           (STD_ON)
/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/*!
 * @brief SPM module configuration structure.
 */
typedef struct
{
    Spm_SleepTimeoutActionType SleepTimeoutAction; /*!< what action after sleep timeout */
    Spm_ThresholdType LvdLevel; /*!< LVD threshold */
    Spm_ThresholdType LvrLevel; /*!< LVR threshold */
#if defined (AC7842X)  || defined (AC7843X)
    Spm_StandbyActionType StandbyAction; /*!< what action after standby wake up */
#endif /* AC7842X AC7843X */
    /*!< StandbyWakeupSource value
     *    - BIT0-1:PA12 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT2-3:PB0 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT4-5:PB1 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT6-7:PB12 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT8-9:PD3 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT10-11:PC2 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT12-13:PC3 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT14-15:PC6 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT16-17:PC7 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT18-19:PC16 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT20-21:PC17 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT22-23:PD6 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT24-25:PD7 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT26-27:PE4 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT28-29:PE5 wakeup enable type, the value range is Spm_StbWakeupEnableType
     *    - BIT30-31:RTC wakeup enable type, the value range is Spm_StbWakeupEnableType without SPM_STP_WAKEUP_FALLING
     */
#if defined (AC7843X) || defined (AC7842X) || defined (AC7840X)
    uint32 StandbyWakeupSource; /*!< Standby Mode Wakeup Enable */
#elif defined (AC7840E)
    uint32 StandbyWakeupSource0; /*!< Standby Mode Wakeup Enable */
    uint32 StandbyWakeupSource1; /*!< Standby Mode Wakeup Enable */
#endif
    Hal_CallbackType SpmCallback; /*!< SPM interrupt callback */
    Hal_CallbackType LvdCallback; /*!< LVD interrupt callback */
#if defined (AC7842X) || defined (AC7843X)
    Hal_CallbackType StbCallback; /*!< STB interrupt callback */
#endif /* AC7842X AC7843X */
} Spm_ConfigType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/
/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Initialize Spm module.
 * @note  Function ID: DES_MCU_API_501
 * @param[in] ConfigPtr: Spm Configuration
 * @return void
 */
void Spm_Hal_Init(const Spm_ConfigType *ConfigPtr);

/*!
 * @brief Get Current power mode.
 * @note  Function ID: DES_MCU_API_502
 * @return Current power mode.
 *            - SPM_MODE_RUN
 *            - SPM_MODE_VLPR
 *            - SPM_MODE_STOP1
 *            - SPM_MODE_STOP2
 *            - SPM_MODE_VLPS
 *            - SPM_MODE_STANDBY
 */
Spm_PowerModeType Spm_Hal_GetCurrentPowerMode(void);

/*!
 * @brief Set power mode.
 * @note  Function ID: DES_MCU_API_503
 * @param[in] Mode: power mode.
 *            - SPM_MODE_RUN
 *            - SPM_MODE_VLPR
 *            - SPM_MODE_STOP1
 *            - SPM_MODE_STOP2
 *            - SPM_MODE_VLPS
 *            - SPM_MODE_STANDBY
 * @return Set power mode pass or failed.
 */
Hal_StatusType Spm_Hal_SetPowerMode(Spm_PowerModeType Mode);

/*!
 * @brief Get standby wakeup status.
 * @note  Function ID: DES_MCU_API_504
 * @return the reset status.
 *         - BIT0:SPM_WAKEUP_STATUS_PA12
 *         - BIT1:SPM_WAKEUP_STATUS_PB0
 *         - BIT2:SPM_WAKEUP_STATUS_PB1
 *         - BIT3:SPM_WAKEUP_STATUS_PB12
 *         - BIT4:SPM_WAKEUP_STATUS_PD3
 *         - BIT5:SPM_WAKEUP_STATUS_PC2
 *         - BIT6:SPM_WAKEUP_STATUS_PC3
 *         - BIT7:SPM_WAKEUP_STATUS_PC6
 *         - BIT8:SPM_WAKEUP_STATUS_PC7
 *         - BIT9:SPM_WAKEUP_STATUS_PC16
 *         - BIT10:SPM_WAKEUP_STATUS_PC17
 *         - BIT11:SPM_WAKEUP_STATUS_PD6
 *         - BIT12:SPM_WAKEUP_STATUS_PD7
 *         - BIT13:SPM_WAKEUP_STATUS_PE4
 *         - BIT14:SPM_WAKEUP_STATUS_PE5
 *         - BIT15:SPM_WAKEUP_STATUS_RTC
 *         - BIT16:SPM_WAKEUP_STATUS_FlAG
 */
uint32 Spm_Hal_GetStandbyWakeupStatus(void);

/*!
 * @brief Clear standby wakeup status.
 * @note  Function ID: DES_MCU_API_505
 * @return void
 */
void Spm_Hal_ClearStandbyWakeupStatus(void);

#if defined (AC7842X)
/*!
 * @brief Set Lvd callback to flash.
 * @note  Function ID: DES_MCU_API_506
 * @return void
 */
void Spm_Hal_SetFlashLVDCallback(Hal_CallbackType Callback);
#endif /* AC7842X */

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* SPM_HAL_H */
