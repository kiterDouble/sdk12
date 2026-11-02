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
 * @file AC784xx_Wdg_Reg.h
 *
 * @brief This file provides definitions & interfaces for read/write WDG hardware registers.
 *
 */

#ifndef AC784XX_WDG_REG_H
#define AC784XX_WDG_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

#define WDG_MODULE                                 ((WDG_Type *)               WDG_BASE)
/***************************************************************************************************/
#define WDG_CS0_EN_ENABLE                    (0x00000080U)
#define WDG_CS0_EN_DISABLE                   (0x00000000U)
#define WDG_CS0_DBG_ENABLE                   (0x00000002U)
#define WDG_CS0_DBG_DISABLE                  (0x00000000U)
#define WDG_CS0_STOP_ENABLE                  (0x00000001U)
#define WDG_CS0_STOP_DISABLE                 (0x00000000U)
#define WDG_CS0_UPDATE_ENABLE                (0x00000020U)
#define WDG_CS0_UPDATE_DISABLE               (0x00000000U)
#define WDG_CS0_TST_ENABLE                   (0x00000010U)
#define WDG_CS0_TST_DISABLE                  (0x00000000U)
#define WDG_CS0_INT_ENABLE                   (0x00000040U)
#define WDG_CS0_INT_DISABLE                  (0x00000000U)
#define WDG_CS1_LSI_CLK                      (0x00000000U)
#define WDG_CS1_BUS_CLK                      (0x00000001U)
#define WDG_CS1_HSE_CLK                      (0x00000003U)
#define WDG_CS1_HSI_CLK                      (0x00000002U)
#define WDG_CS1_WIN_ENABLE                   (0x00000080U)
#define WDG_CS1_FLG_ENABLE                   (0x00000040U)
#define WDG_CS1_PRES_ENABLE                  (0x00000010U)
#define WDG_CNT_UNLOCK1                      (0xE064D987U)
#define WDG_CNT_UNLOCK2                      (0x868A8478U)
#define WDG_CNT_TRIGGER1                     (0x7908AD15U)
#define WDG_CNT_TRIGGER2                     (0x5AD5A879U)
#define WDG_PRESCALER_VALUE                  (0x00000100U)
#define WDG_TICKS_INIT_VALUE                 (0x00000400U)
#define WDOG_MIN_VALUE_TIMEOUT_U32           (0x00000100U)

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/**
 * @brief Read  value of CNT register
 * @note    Function ID:  DES_WDG_API_50
 *
 * @return value of CNT register
 */
LOCAL_INLINE uint32 Wdg_Reg_ReadCNT(void)
{
    return WDG_MODULE->CNT;
}

/**
 * @brief  Set WDG  CNT register
 * @note    Function ID:  DES_WDG_API_51
 *
 * @param[in] Value: Value set to CNT register
 * @return none
 */
LOCAL_INLINE void Wdg_Reg_WriteCNT(uint32 Value)
{
    WDG_MODULE->CNT = Value;
}

/**
 * @brief Read  value of TOVAL register
 * @note    Function ID:  DES_WDG_API_52
 *
 * @return value of TOVAL register
 */
LOCAL_INLINE uint32 Wdg_Reg_ReadTOVAL(void)
{
    return WDG_MODULE->TOVAL;
}

/**
 * @brief  Set WDG  TOVAL register
 * @note    Function ID:  DES_WDG_API_53
 *
 * @param[in] Value: Value set to TOVAL register
 * @return none
 */
LOCAL_INLINE void Wdg_Reg_WriteTOVAL(uint32 Value)
{
    WDG_MODULE->TOVAL = Value;
}

/**
 * @brief Read  value of CS0 register
 * @note    Function ID:  DES_WDG_API_54
 *
 * @return value of CS0 register
 */
LOCAL_INLINE uint32 Wdg_Reg_ReadCS0(void)
{
    return WDG_MODULE->CS0; /*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
}

/**
 * @brief  Set WDG  CS0 register
 * @note    Function ID:  DES_WDG_API_55
 *
 * @param[in] Value: Value set to CS0 register
 * @return none
 */
LOCAL_INLINE void Wdg_Reg_WriteCS0(uint32 Value)
{
    WDG_MODULE->CS0 = Value;
}

/**
 * @brief Read  value of CS1 register
 * @note    Function ID:  DES_WDG_API_56
 *
 * @return value of CS1 register
 */
LOCAL_INLINE uint32 Wdg_Reg_ReadCS1(void)
{
    return WDG_MODULE->CS1;
}

/**
 * @brief  Set WDG  CS1 register
 * @note    Function ID:  DES_WDG_API_57
 *
 * @param[in] Value: Value set to CS1 register
 * @return none
 */
LOCAL_INLINE void Wdg_Reg_WriteCS1(uint32 Value)
{
    WDG_MODULE->CS1 = Value;
}

/**
 * @brief Read  value of WIN register
 * @note    Function ID:  DES_WDG_API_58
 *
 * @return value of WIN register
 */
LOCAL_INLINE uint32 Wdg_Reg_ReadWIN(void)
{
    return WDG_MODULE->WIN;
}

/**
 * @brief  Set WDG  WIN register
 * @note    Function ID:  DES_WDG_API_59
 *
 * @param[in] Value: Value set to WIN register
 * @return none
 */
LOCAL_INLINE void Wdg_Reg_WriteWIN(uint32 Value)
{
    WDG_MODULE->WIN = Value;
}

/* =====================================  Functions definition  ===================================== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AC784XX_WDG_REG_H*/

/* =============================================  EOF  ============================================== */
