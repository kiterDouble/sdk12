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
 * @file AC784xx_Rtc_Reg.h
 *
 * @brief rtc access register inline function definition.
 *
 */

#ifndef AC784XX_RTC_REG_H
#define AC784XX_RTC_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/**
 * @brief Write RTC's CTRL register
 * @note Function ID : DES_GPT_API_650
 * @param[in] Value: Value set to CTRL register
 */
LOCAL_INLINE void Rtc_Reg_WriteCTRL(uint32 Value)
{
    WRITE_REG32(RTC->CTRL, Value);
}

/**
 * @brief Read RTC's CTRL regiser
 * @note Function ID : DES_GPT_API_651
 * @return uint32: value of CTRL register
 */
LOCAL_INLINE uint32 Rtc_Reg_ReadCTRL(void)
{
    return READ_REG32(RTC->CTRL);
}

/**
 * @brief Write RTC's alarm register
 * @note Function ID : DES_GPT_API_652
 * @param[in] Value: Value set to alart register
 */
LOCAL_INLINE void Rtc_Reg_WriteAlarm(uint32 Value)
{
    WRITE_REG32(RTC->TAR, Value);
}

/**
 * @brief Read RTC's alarm regiser
 * @note Function ID : DES_GPT_API_653
 * @return uint32: value of alarm register
 */
LOCAL_INLINE uint32 Rtc_Reg_ReadAlarm(void)
{
    return READ_REG32(RTC->TAR);
}

/**
 * @brief Write RTC's TC register
 * @note Function ID : DES_GPT_API_654
 * @param[in] Value: Value set to alart register
 */
LOCAL_INLINE void Rtc_Reg_WriteTC(uint32 Value)
{
    WRITE_REG32(RTC->TC, Value);
}

/**
 * @brief Read RTC's TC regiser
 * @note Function ID : DES_GPT_API_655
 * @return uint32: value of TC register
 */
LOCAL_INLINE uint32 Rtc_Reg_ReadTC(void)
{
    return READ_REG32(RTC->TC);
}

/**
 * @brief Write RTC's PSR register
 * @note Function ID : DES_GPT_API_656
 * @param[in] Value: Value set to alart register
 */
LOCAL_INLINE void Rtc_Reg_WritePSR(uint32 Value)
{
    WRITE_REG32(RTC->PSR, Value);
}
/**
 * @brief Read RTC's PSR regiser
 * @note Function ID : DES_GPT_API_657
 * @return uint32: value of PSR register
 */

LOCAL_INLINE uint32 Rtc_Reg_ReadPSR(void)
{
    return READ_REG32(RTC->PSR);
}

/**
 * @brief Read RTC's PSCregiser
 * @note Function ID : DES_GPT_API_658
 * @return uint32: value of PSC register
 */

LOCAL_INLINE uint32 Rtc_Reg_ReadPSC(void)
{
    return READ_REG32(RTC->PSC);
}

/**
 * @brief Write RTC's alarm register
 * @note Function ID : DES_GPT_API_659
 * @param[in] Value: Value set to alart register
 */
LOCAL_INLINE void Rtc_Reg_WriteSR(uint32 Value)
{
    WRITE_REG32(RTC->SR, Value);
}

/**
 * @brief Read RTC's SR regiser
 * @note Function ID:
 * @return uint32: value of SR register
 */
LOCAL_INLINE uint32 Rtc_Reg_ReadSR(void)
{
    return READ_REG32(RTC->SR);
}

/**
 * @brief Write RTC's alarm register
 * @note Function ID : DES_GPT_API_660
 * @param[in] Value: Value set to alart register
 */
LOCAL_INLINE void Rtc_Reg_WriteLR(uint32 Value)
{
    WRITE_REG32(RTC->LR, Value);
}

/**
 * @brief Read RTC's LR regiser
 * @note Function ID : DES_GPT_API_661
 * @return uint32: value of LR register
 */

LOCAL_INLINE uint32 Rtc_Reg_ReadLR(void)
{
    return READ_REG32(RTC->LR);
}

/**
 * @brief Set rtc software reset
 * @note Function ID : DES_GPT_API_662
 * @param[in] None
 * @param[inout] None
 * @param[out] None
 * @return None
 */
LOCAL_INLINE void Rtc_Reg_Reset(void)
{
    MODIFY_REG32(RTC->CTRL, RTC_CTRL_SWR_Msk, RTC_CTRL_SWR_Pos, 1U);
    MODIFY_REG32(RTC->CTRL, RTC_CTRL_SWR_Msk, RTC_CTRL_SWR_Pos, 0U);
}

/**
 * @brief SEnable or disable rtc module
 * @note Function ID : DES_GPT_API_663
 * @param[in] IsEnable: true enable rtc module; false disable rtc module
 * @param[inout] None
 * @param[out] None
 * @return None
 */
LOCAL_INLINE void Rtc_Reg_Enable(boolean IsEnable)
{
    /* PRQA S 4559  ++ # AMCE_QAC_4559*/
    MODIFY_REG32(RTC->SR, RTC_SR_TCE_Msk, RTC_SR_TCE_Pos, (IsEnable ? 1U : 0U));
    /* PRQA S 4559  -- # AMCE_QAC_4559*/
}

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* AC784XX_RTC_REG_H */

/* =============================================  EOF  ============================================== */
