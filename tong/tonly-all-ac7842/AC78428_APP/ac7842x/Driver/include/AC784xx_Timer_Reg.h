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
 * @file AC784xx_Timer_Reg.h
 * @brief Timer access register inline function definition.
 */

#ifndef AC784XX_TIMER_REG_H
#define AC784XX_TIMER_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* =====================================  Functions definition  ===================================== */

/**
 * @brief Read  timer module control register.
 * @note    Function ID : DES_GPT_API_550
 *
 * @return value of control register.
 */
LOCAL_INLINE uint32 Timer_Reg_ReadCR(void)
{
    return TIMER_CTRL->CR;
}

/**
 * @brief  Settimer module control register
 * @note    Function ID : DES_GPT_API_551
 * @param[in] Value: Value set to control register
 * @return none
 */
LOCAL_INLINE void Timer_Reg_WriteCR(uint32 Value)
{
    TIMER_CTRL->CR = Value;
}

/**
 * @brief Read  interrupt status register.
 * @note    Function ID : DES_GPT_API_552
 * @return value of interrupt status register
 */
LOCAL_INLINE uint32 Timer_Reg_ReadSR(void)
{
    return TIMER_CTRL->SR;
}

/**
 * @brief  Set interrupt status register.
 * @note    Function ID : DES_GPT_API_553
 * @param[in] Value: Value set to interrupt status register
 * @return none
 */
LOCAL_INLINE void Timer_Reg_WriteSR(uint32 Value)
{
    TIMER_CTRL->SR = Value;
}

/**
 * @brief Read  IER register.
 * @note    Function ID : DES_GPT_API_554
 *
 * @return value of IER register
 */
LOCAL_INLINE uint32 Timer_Reg_ReadIER(void)
{
    return TIMER_CTRL->IER;
}

/**
 * @brief  Set IER register.
 * @note    Function ID : DES_GPT_API_555
 * @param[in] Value: Value set to register
 * @return none
 */
LOCAL_INLINE void Timer_Reg_WriteIER(uint32 Value)
{
    TIMER_CTRL->IER = Value;
}

/**
 * @brief Read   ENR register.
 * @note    Function ID : DES_GPT_API_556
 * @return value of  ENR register
 */
LOCAL_INLINE uint32 Timer_Reg_ReadENR(void)
{
    return TIMER_CTRL->ENR;
}

/**
 * @brief  Set ENR register.
 * @note    Function ID : DES_GPT_API_557
 * @param[in] Value: Value set to register
 * @return none
 */
LOCAL_INLINE void Timer_Reg_WriteENR(uint32 Value)
{
    TIMER_CTRL->ENR = Value;
}

/**
 * @brief Read  TVAL register.
 * @note    Function ID:
 * @param[in] Base: Base address to TIMER_CHANNEL_Type.
 * @return value of  TVAL register
 */
LOCAL_INLINE uint32 Timer_Reg_ReadTVAL(const TIMER_CHANNEL_Type *Base)
{
    return Base->TVAL;
}

/**
 * @brief  Set TVAL register.
 * @note    Function ID : DES_GPT_API_558
 * @param[in] Base: Base address to TIMER_CHANNEL_Type.
 * @param[in] Value: Value set to register
 * @return none
 */
LOCAL_INLINE void Timer_Reg_WriteTVAL(TIMER_CHANNEL_Type *Base, uint32 Value)
{
    Base->TVAL = Value;
}

/**
 * @brief Read CVAL register.
 * @note    Function ID : DES_GPT_API_559
 * @param[in] Base: Base address to TIMER_CHANNEL_Type.
 * @return Value of CVAL register
 */
LOCAL_INLINE uint32 Timer_Reg_ReadCVAL(const TIMER_CHANNEL_Type *Base)
{
    return Base->CVAL;
}

/**
 * @brief  Set CVAL register.
 * @note    Function ID : DES_GPT_API_560
 * @param[in] Base: Base address to TIMER_CHANNEL_Type.
 * @param[in] Value: Value set to register
 * @return none
 */
LOCAL_INLINE void Timer_Reg_WriteCVAL(TIMER_CHANNEL_Type *Base, uint32 Value)
{
    Base->CVAL = Value;
}

/**
 * @brief Read CTRL register.
 * @note    Function ID : DES_GPT_API_561
 * @param[in] Base: Base address to TIMER_CHANNEL_Type.
 *
 * @return value of CTRL register
 */
LOCAL_INLINE uint32 Timer_Reg_ReadCTRL(const TIMER_CHANNEL_Type *Base)
{
    return Base->CTRL;
}

/**
 * @brief  Set CTRL register.
 * @note    Function ID : DES_GPT_API_562
 * @param[in] Base: Base address to TIMER_CHANNEL_Type.
 * @param[in] Value: Value set to register
 * @return none
 */
LOCAL_INLINE void Timer_Reg_WriteCTRL(TIMER_CHANNEL_Type *Base, uint32 Value)
{
    Base->CTRL = Value;
}

/**
 * @brief  Get base address of a timer channel.
 * @note    Function ID : DES_GPT_API_563
 * @param[in] Channel: Channel index.
 * @return Pointer to structure of TIMER_CHANNEL_Type.
 */
LOCAL_INLINE TIMER_CHANNEL_Type *Timer_Reg_GetChBase(uint32 Channel)
{
    /* PRQA S 1840  ++ # AMCE_QAC_1840*/
    return (TIMER_CHANNEL_Type *)(TIMER_CHANNEL0_BASE + (0x10 * Channel));
    /* PRQA S 1840  -- # AMCE_QAC_1840*/
}

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* AC784XX_ADC_REG_H */
/* =============================================  EOF  ============================================== */
