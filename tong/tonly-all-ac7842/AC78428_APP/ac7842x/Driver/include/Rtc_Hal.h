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

/*!
 * @file Rtc_Hal.h
 *
 * @brief rtc hal define.
 */

#ifndef RTC_HAL_H
#define RTC_HAL_H
#ifdef __cplusplus
extern "C" {
#endif
/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Rtc_Reg.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/**
 * @brief Enable flag of RTC timer overflow interrupt
 * @note DES ID: DES_GPT_MACRO_601
 */
#define RTC_INT_TIMER_EN     0x01U

/**
 * @brief Enable flag(bit) of RTC alarm interrupt
 * @note DES ID: DES_GPT_MACRO_602
 */
#define RTC_INT_ALARM_EN     0x02U

/**
 * @brief Enable flag of RTC prescaler overflow interrupt
 * @note DES ID: DES_GPT_MACRO_603
 */
#define RTC_INT_PRESCALER_EN 0x04U

/**
 * @brief Enable flag of invalid interrupt
 * @note DES ID: DES_GPT_MACRO_604
 */
#define RTC_INT_INVALID_EN 0x08U

/**
 * @brief Enable flags of RTC all interrupt
 * @note DES ID: DES_GPT_MACRO_605
 */
#define RTC_INT_ALL_EN       0x0FU

/**
 * @brief Event of timer overflow for RTC callback
 * @note DES ID: DES_GPT_MACRO_606
 */
#define RTC_EVENT_TIMER      RTC_INT_TIMER_EN

/**
 * @brief Event of alarm for RTC callback
 * @note DES ID: DES_GPT_MACRO_607
 */
#define RTC_EVENT_ALARM      RTC_INT_ALARM_EN

/**
 * @brief Event of RTC prescaler overflow for RTC callback
 * @note DES ID: DES_GPT_MACRO_608
 */
#define RTC_EVENT_PRESCALER  RTC_INT_PRESCALER_EN

/**
 * @brief Event of RTC invalid interrupt for RTC callback
 * @note DES ID: DES_GPT_MACRO_609
 */
#define RTC_EVENT_INVALID  RTC_INT_INVALID_EN

/*===================================================ENUMS==========================================*/

/*!
 * @brief RTC Clock source type
 * @note DES ID: DES_GPT_TYP_601
 */
typedef enum
{
    RTC_CLOCK_HSE = 0U, /*!< RTC uses HSE DIV1 as clock source.*/
    RTC_CLOCK_VHSI, /*!< RTC uses VHSI DIV1 as clock source.*/
    RTC_CLOCK_LSI32K, /*!< RTC uses LSI32K as clock source.*/
    RTC_CLOCK_CLKIN, /*!< RTC uses RTC_CLKIN as clock source.*/
#if defined (AC7840X)
    RTC_CLOCK_STB_LSI32K, /*!< RTC uses SPM LSI32K as clock source.*/
    RTC_CLOCK_STB_CLKIN, /*!< RTC uses SPM RTC_CLKIN as clock source.*/
#endif
    RTC_CLOCK_INVALID, /*!< Invalid clock source.*/
} Rtc_ClockSourceType;

/*!
 * @brief RTC output type
 * @note DES ID: DES_GPT_TYP_602
 */
typedef enum
{
    RTC_OUTPUT_DISABLED = 0U, /*!< RTC clock output is disabled.*/
    RTC_OUTPUT_CLOCK, /*!< RTC clock output is enabled.*/
    RTC_OUTPUT_PRESCALER, /*!< RTC prescaler output is enabled*/
} Rtc_ClockOutputType;

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*!
 * @brief rtc callback function prototype.
 * @param [in] Event: Event conbinations (bitwise OR) of RTC_EVENT_TIMER/RTC_EVENT_ALARM/RTC_EVENT_PRESCALER.
 * @return void
 */
typedef void (*Rtc_Hal_CallbackType)(uint32 Event);
/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief Initialize rtc module
 * @note Function ID: DES_GPT_API_601
 * @param [in] Clk: Specify RTC work clock source.
 *            RTC_CLOCK_INVALID: doesn't set clock source.
 * @return Hal_StatusType: Initialize success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Rtc_Hal_Init(Rtc_ClockSourceType Clk);

/**
 * @brief Deinitialize rtc module
 * @note Function ID: DES_GPT_API_602
 * @return Hal_StatusType: STATUS_SUCCESS: success;  STATUS_ERROR: RTC isn't initialized or is started.
 */
Hal_StatusType Rtc_Hal_DeInit(void);

/**
 * @brief Start RTC
 * @note Function ID: DES_GPT_API_603
 * @param [in] Unit:  Value for RTC prescaler. And it is the unit of RTC real time register.
 * @return Hal_StatusType: STATUS_SUCCESS: success;  STATUS_ERROR: RTC isn't initialized or has been started.
 */
Hal_StatusType Rtc_Hal_Start(uint32 Unit);

/**
 * @brief Stop RTC
 * @note Function ID: DES_GPT_API_604
 * @return Hal_StatusType: STATUS_SUCCESS: success;  STATUS_ERROR: RTC isn't initialized or isn't started.
 */
Hal_StatusType Rtc_Hal_Stop(void);

/**
 * @brief Pause RTC
 * @pre Pre-condition: RTC is started.
 * @note Function ID: DES_GPT_API_613
 * @return Hal_StatusType: STATUS_SUCCESS: success;  STATUS_ERROR: RTC isn't started.
 */
Hal_StatusType Rtc_Hal_Pause(void);

/**
 * @brief  Get RTC real time value.
 * @note Function ID: DES_GPT_API_605
 * @return uint32: Value of RTC timer register.
 */
uint32 Rtc_Hal_GetCurrentValue(void);

/**
 * @brief Set alarm count
 * @note Function ID: DES_GPT_API_606
 * @param [in] Value:  Count for alarm.
 * @return Hal_StatusType: Set success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Rtc_Hal_SetAlarm(uint32 Value);

/**
 * @brief Set RTC real time counter.
 * @pre Pre-condition: RTC isn't started.
 * @note Function ID: DES_GPT_API_612
 * @param [in] Value:  Value of RTC real timer.
 * @return void
 */
void Rtc_Hal_SetCurrentValue(uint32 Value);

/**
 * @brief Install counter overflow interrupt callback.
 * @note Function ID: DES_GPT_API_607
 * @param [in] Func: the callback function
 * @return void
 */
void Rtc_Hal_InstallCallback(const Rtc_Hal_CallbackType Func);

/**
 * @brief Enable or disable rtc interrupt
 * @note Function ID: DES_GPT_API_608
 * @param [in] InterruptBits: RTC interrupt config.  Bit 0: RTC Timer overflow interrupt enable;
               Bit 1: Alarm interrupt enable; Bit 2:  Prescaler interrupt enable.
 * @return void
 */
void Rtc_Hal_EnableInterrupt(uint32 InterruptBits);

/**
 * @brief Cofigure RTC output.
 * @note Function ID: DES_GPT_API_609
 * @param [in] OutputCfg : RTC output configuration. 0 : Disabled, 1: RTC clock output, 2: RTC prescaler output.
 * @return Hal_StatusType: STATUS_SUCCESS: success;
 */
Hal_StatusType Rtc_Hal_SetConfig(Rtc_ClockOutputType OutputCfg);

/**
 * @brief Get rtc work frequence
 * @note Function ID: DES_GPT_API_610
 * @return uint32: work frequence
 */
uint32 Rtc_Hal_GetWorkFreq(void);

/**
 * @brief Get prescaler value.
 * @note Function ID: DES_GPT_API_611
 * @return uint32: Value of RTC prescaler register.
 */
uint32 Rtc_Hal_GetPrescalerValue(void);

#ifdef __cplusplus
}
#endif
#endif /* RTC_HAL_H */

/* =============================================  EOF  ============================================== */
