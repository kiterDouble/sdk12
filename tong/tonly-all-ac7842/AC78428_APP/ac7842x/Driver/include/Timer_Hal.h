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
 * @file Timer_Hal.h
 *
 * @brief Timer HAL header file
 */

#ifndef TIMER_HAL_H
#define TIMER_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Timer_Reg.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/**
 * @brief Timer oneshot enable
 * @note DES ID: DES_GPT_MACRO_501
 */
#define TIMER_ONESHOT_EN 0x01U

/**
 * @brief Timer debug enable
 * @note DES ID: DES_GPT_MACRO_502
 */
#define TIMER_DBG_EN     0x02U

/**
 * @brief Link enable
 * @note DES ID: DES_GPT_MACRO_503
 */
#define TIMER_CHN_EN     0x04U

/**
 * @brief TROT enable
 * @note DES ID: DES_GPT_MACRO_504
 */
#define TIMER_TROT       0x08U

/**
 * @brief TSOT enable
 * @note DES ID: DES_GPT_MACRO_505
 */
#define TIMER_TSOT       0x10U

/**
 * @brief Timer interrupt enable config
 * @note DES ID: DES_GPT_MACRO_506
 */
#define TIMER_IRQ_EN     0x20U

/**
 * @brief Timer interrupt flag
 * @note DES ID: DES_GPT_MACRO_507
 */
#define TIMER_INT_EN     0x01U

/**
 * @brief Status code for timer wrong state
 * @note DES ID: DES_GPT_MACRO_508
 */
#define STATUS_TIMER_WRONG_STATE 0x91

/**
 * @brief Status code for timer wrong parameter
 * @note DES ID: DES_GPT_MACRO_509
 */
#define STATUS_TIMER_WRONG_PARAM 0x92

/*===================================================ENUMS==========================================*/
/*!
 * @brief Timer Clock source type
 * @note DES ID: DES_GPT_TYP_501
 */
typedef enum
{
    TIMER_CLOCK_SPLL = 0U, /*!< Timer uses SPLL DIV2 as clock source.*/
    TIMER_CLOCK_VHSI, /*!< Timer uses VHSI DIV2 as clock source.*/
    TIMER_CLOCK_HSI, /*!< Timer uses HSI as clock source.*/
    TIMER_CLOCK_HSE, /*!< Timer uses HSE DIV2 clock source.*/
    TIMER_CLOCK_INVALID, /*!< Invalid clock source.*/
} Timer_ClockSourceType;

/*!
 * @brief Timer work mode.
 * @note DES ID: DES_GPT_TYP_502
 */
typedef enum
{
    TIMER_MODE_0 = 0U, /*!< Timer work mode : 32 bit period timer.*/
    TIMER_MODE_1, /*!< Timer work mode : dual 16 bit period timer.*/
    TIMER_MODE_2, /*!< Timer work mode : 32 bit pulse accumulator.*/
    TIMER_MODE_3, /*!< Timer work mode : 32 bit pulse capturer.*/
    TIMER_MODE_NUM, /*!< Number of timer work modes.*/
} Timer_ModeType;

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*!
 * @brief Timer channel configuration type
 * @note DES ID: DES_GPT_TYP_503
 */
typedef struct
{
    Timer_ModeType Mode; /*!< Work mode of timer.*/
    uint32 Config; /*!< Timer configuraiton.   It is conbinations(bitwise OR) of
                              TIMER_ONESHOT_EN/TIMER_DBG_EN/TIMER_CHN_EN/TIMER_TROT/TIMER_TSOT. */
    uint32 TriggerSrc; /*!< Trigger source of timer.*/
} Timer_Channel_ConfigType;

/*===========================================VARIABLE DECLARATIONS==================================*/
/**
 * @brief Initialize timer module.
 * @details This function initializes the timer module with the specified clock source.
 * @note Function ID: DES_GPT_API_501
 * @note Service ID: none
 * @param [in] Clk: Specify clock source of timer.
 *            TIMER_CLOCK_INVALID: doesn't set clock source.
 * @return Hal_StatusType. if successful return STATUS_SUCCESS or return STATUS_TIMER_WRONG_STATE.
 * @see Timer_ClockSourceType
 * @pre Timer isn't initialized.
 */
Hal_StatusType Timer_Hal_Init(Timer_ClockSourceType Clk);

/**
 * @brief Deinit the timer driver.
 * @details This function deinitializes the timer driver.
 * @note Function ID: DES_GPT_API_502
 * @note Service ID: none
 * @return Hal_StatusType. if successful return STATUS_SUCCESS or return STATUS_TIMER_WRONG_STATE.
 * @pre Timer has been initialized and no timer channel is started.
 */
Hal_StatusType Timer_Hal_DeInit(void);

#ifndef WDG_SDK_NON_EXTENDED_API
/**
 * @brief  Set configuration information for a timer channel.
 * @details This function sets the configuration information for a timer channel.
 * @note Function ID: DES_GPT_API_503
 * @note Service ID: none
 * @param [in]  Channel: Timer channel index.
 * @param [in]  ConfigPtr: Channel configuration.
 * @return Hal_StatusType. if successful return STATUS_SUCCESS or return STATUS_TIMER_WRONG_PARAM.
 * @see Timer_Channel_ConfigType
 */
Hal_StatusType Timer_Hal_SetConfig(uint32 Channel, const Timer_Channel_ConfigType *ConfigPtr);

/**
 * @brief  Get configuration information of a timer channel.
 * @details This function gets the configuration information of a timer channel.
 * @note Function ID: DES_GPT_API_504
 * @note Service ID: none
 * @param [in]  Channel: Timer channel index.
 * @param [out]  ConfigPtr: Channel configuration.
 * @return Hal_StatusType. if successful return STATUS_SUCCESS or return STATUS_TIMER_WRONG_PARAM.
 */
Hal_StatusType Timer_Hal_GetConfig(uint32 Channel, Timer_Channel_ConfigType *ConfigPtr);
#endif

/**
 * @brief  Start a channel.
 * @details This function starts a timer channel with the specified timeout value.
 * @note Function ID: DES_GPT_API_505
 * @note Service ID: none
 * @param [in]  Channel: Timer channel index.
 * @param [in]  Timeout: Timeout value to be set.
 * @return Hal_StatusType. if successful return STATUS_SUCCESS or
 *         return STATUS_TIMER_WRONG_PARAM/STATUS_TIMER_WRONG_STATE.
 */
Hal_StatusType Timer_Hal_Start(uint32 Channel, uint32 Timeout);

/**
 * @brief  Stop a channel.
 * @details This function stops a timer channel.
 * @note Function ID: DES_GPT_API_506
 * @note Service ID: none
 * @param [in]  Channel: Timer channel index.
 * @return Hal_StatusType. if successful return STATUS_SUCCESS or
 *         return STATUS_TIMER_WRONG_PARAM/STATUS_TIMER_WRONG_STATE.
 */
Hal_StatusType Timer_Hal_Stop(uint32 Channel);

/**
 * @brief  Returns the time already elapsed.
 * @details This function returns the time already elapsed for a timer channel.
 * @note Function ID: DES_GPT_API_507
 * @note Service ID: none
 * @param [in]  Channel: Timer channel index.
 * @return uint32: Elapsed timer value (in number of ticks).
 */
uint32 Timer_Hal_GetCurrentValue(uint32 Channel);

#ifndef WDG_SDK_NON_EXTENDED_API
/**
 * @brief  Returns the time remaining until the target time is reached.
 * @details This function returns the time remaining until the target time is reached for a timer channel.
 * @note Function ID: DES_GPT_API_508
 * @note Service ID: none
 * @param [in]  Channel: Timer channel index.
 * @return Remaining value of timer.
 */
uint32 Timer_Hal_GetRemainingValue(uint32 Channel);
#endif
/**
 * @brief  Enable channel interrupt.
 * @details This function enables or disables the interrupt for a timer channel.
 * @note Function ID: DES_GPT_API_509
 * @note Service ID: none
 * @param [in]  Channel: Timer channel index.
 * @param [in]  InterruptBits: 0: Disabled, 1:  Enabled.
 * @return void
 */
void Timer_Hal_EnableInterrupt(uint32 Channel, uint32 InterruptBits);

/**
 * @brief  Install timer callback.
 * @details This function installs a callback function for a timer channel.
 * @note Function ID: DES_GPT_API_510
 * @note Service ID: none
 * @param [in]  Channel: Timer channel ID.
 * @param [in]  Func: Pointer to a function.
 * @param [in]  Args: Pointer to void pointer parameter for callback funciton.
 * @return void
 */
void Timer_Hal_InstallCallback
(
    uint32 Channel,
    Hal_CallbackType Func,
    void *Args
);

#ifndef WDG_SDK_NON_EXTENDED_API
/**
 * @brief  Reset current value of timer.
 * @details This function resets the current value of a timer channel.
 * @note Function ID: DES_GPT_API_511
 * @note Service ID: none
 * @param [in]  Channel: Timer channel index.
 * @return Hal_StatusType. if successful return STATUS_SUCCESS or
 *         return STATUS_TIMER_WRONG_PARAM/STATUS_TIMER_WRONG_STATE.
 */
Hal_StatusType Timer_Hal_ResetValue(uint32 Channel);
#endif

/**
 * @brief  Translate microseconds to tick of timer base on clock source.
 * @details This function converts microseconds to timer ticks based on the clock source.
 * @note Function ID: DES_GPT_API_512
 * @note Service ID: none
 * @param [in]  Micros:  Microseconds
 * @return Tick value for the specified microseconds
 */
uint32 Timer_Hal_MicrosToTicks(uint32 Micros);

#ifdef __cplusplus
}
#endif

#endif /*TIMER_HAL_H*/
/*============================================EOF===================================================*/
