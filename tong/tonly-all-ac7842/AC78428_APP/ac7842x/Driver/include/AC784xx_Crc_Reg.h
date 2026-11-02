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
/**

 * @file AC784xx_Crc_Reg.h
 *
 * @brief This file provides extern Crc Reg API.
 *
 */
#ifndef AC784XX_CRC_REG_H
#define AC784XX_CRC_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
static CRC_Type *const Base[CRC_INSTANCE_MAX] = {CRC};
/*============================================FUNCTION PROTOTYPES===================================*/

/* ======================================  Functions definition  ==================================== */

/*cstat -MISRAC2012-Rule-11.4 fix A cast between a pointer type and an integral type*/
/**
 * @brief Config CRC CTRL register.
 * @note  Function ID : DES_CRC_API_301
 * @param[in] Instance: CRC Hardware Device instance
 * @param[in] Value: config Value
 * @return void
 */
LOCAL_INLINE void Crc_Reg_SetCtrl(uint8 Instance, uint32 Value)
{
    Base[Instance]->CTRL = Value;
}

/**
 * @brief  Set CRC Poly register.
 * @note  Function ID : DES_CRC_API_302
 * @param[in] Instance: CRC Hardware Device instance
 * @param[in] Value: polynomial Value
 * @return void
 */
LOCAL_INLINE void Crc_Reg_SetPoly(uint8 Instance, uint32 Value)
{
    Base[Instance]->POLY = Value;
}

/**
 * @brief  Get CRC Poly register Value.
 * @note  Function ID : DES_CRC_API_303
 * @param[in] Instance: CRC Hardware Device instance
 * @return polynomial Value
 */
LOCAL_INLINE uint32 Crc_Reg_GetPoly(uint8 Instance)
{
    return (Base[Instance]->POLY);
}

/**
 * @brief  Sets the 32 bits of CRC data register.
 * @note  Function ID : DES_CRC_API_304
 * @param[in] Instance: CRC Hardware Device instance
 * @param[in] Value: data Value
 * @return void
 */
LOCAL_INLINE void Crc_Reg_SetData(uint8 Instance, uint32 Value)
{
    Base[Instance]->DATAn.DATA32 = Value;
}

/**
 * @brief  Get CRC Data register Value.
 * @note  Function ID : DES_CRC_API_305
 * @param[in] Instance: CRC Hardware Device instance
 * @return data Value
 */
LOCAL_INLINE uint32 Crc_Reg_GetData(uint8 Instance)
{
    return (Base[Instance]->DATAn.DATA32);
}

/**
 * @brief  Sets the low 16 bits of CRC data register
 * @note  Function ID : DES_CRC_API_306
 * @param[in] Instance: CRC Hardware Device instance
 * @param[in] Value: data Value
 * @return void
 */
LOCAL_INLINE void Crc_Reg_SetDataL(uint8 Instance, uint16 Value)
{
    Base[Instance]->DATAn.DATA16.L = Value;
}

/**
 * @brief   Sets the low 8 bits of CRC data register.
 * @note  Function ID : DES_CRC_API_307
 * @param[in] Instance: CRC Hardware Device instance
 * @param[in] Value: data Value
 * @return void
 */
LOCAL_INLINE void Crc_Reg_SetDataLL(uint8 Instance, uint8 Value)
{
    Base[Instance]->DATAn.DATA8.LL = Value;
}

/**
 * @brief Select crc protocol mode between crc16 or crc32.
 * @note  Function ID : DES_CRC_API_308
 * @param[in] Instance: CRC Hardware Device instance
 * @param[in] Type: crc check mode select
 * @return void
 */
LOCAL_INLINE void Crc_Reg_SetProtocolType(uint8 Instance, uint8 Type)
{
    MODIFY_REG32(Base[Instance]->CTRL, CRC_CTRL_TCRC_Msk, CRC_CTRL_TCRC_Pos, Type);
}

/**
 * @brief Get crc protocol mode between crc16 or crc32.
 * @note  Function ID : DES_CRC_API_309
 * @param[in] Instance: CRC Hardware Device instance
 * @return crc protocol mode
 */
LOCAL_INLINE uint8 Crc_Reg_GetProtocolType(uint8 Instance)
{
    return (uint8)((Base[Instance]->CTRL & CRC_CTRL_TCRC_Msk) >> CRC_CTRL_TCRC_Pos);
}

/**
 * @brief Select Seed or data mode
 * @note  Function ID : DES_CRC_API_310
 * @param[in] Instance: CRC Hardware Device instance
 * @param[in] Mode: mode select
 * @return void
 */
LOCAL_INLINE void Crc_Reg_SetSeedOrDataMode(uint8 Instance, uint8 Mode)
{
    MODIFY_REG32(Base[Instance]->CTRL, CRC_CTRL_WAS_Msk, CRC_CTRL_WAS_Pos, Mode);
}

/**
 * @brief Set CRC write transpose mode.
 * @note  Function ID : DES_CRC_API_311
 * @param[in] Instance: CRC Hardware Device instance
 * @param[in] TransType: write transpose mode
 * @return void
 */
LOCAL_INLINE void Crc_Reg_SetWriteTranspose(uint8 Instance, uint8 TransType)
{
    MODIFY_REG32(Base[Instance]->CTRL, CRC_CTRL_TOTW_Msk, CRC_CTRL_TOTW_Pos, TransType);
}

/**
 * @brief Get CRC write transpose mode.
 * @note  Function ID : DES_CRC_API_312
 * @param[in] Instance: CRC Hardware Device instance
 * @return write transpose mode
 */
LOCAL_INLINE uint8 Crc_Reg_GetWriteTranspose(uint8 Instance)
{
    return (uint8)((Base[Instance]->CTRL & CRC_CTRL_TOTW_Msk) >> CRC_CTRL_TOTW_Pos);
}

/**
 * @brief Set CRC read transpose mode.
 * @note  Function ID : DES_CRC_API_313
 * @param[in] Instance: CRC Hardware Device instance
 * @param[in] TransType: read transpose mode
 * @return void
 */
LOCAL_INLINE void Crc_Reg_SetReadTranspose(uint8 Instance, uint8 TransType)
{
    MODIFY_REG32(Base[Instance]->CTRL, CRC_CTRL_TOTR_Msk, CRC_CTRL_TOTR_Pos, TransType);
}

/**
 * @brief Get CRC read transpose mode
 * @note  Function ID : DES_CRC_API_314
 * @param[in] Instance: CRC Hardware Device instance
 * @return read transpose mode
 */
LOCAL_INLINE uint8 Crc_Reg_GetReadTranspose(uint8 Instance)
{
    return (uint8)((Base[Instance]->CTRL & CRC_CTRL_TOTR_Msk) >> CRC_CTRL_TOTR_Pos);
}

/**
 * @brief Set CRC result xor mode.
 * @note  Function ID : DES_CRC_API_315
 * @param[in] Instance: CRC Hardware Device instance
 * @param[in] En: result xor mode enable/disable
 * @return void
 */
LOCAL_INLINE void Crc_Reg_SetResultXorMode(uint8 Instance, boolean En)
{
    MODIFY_REG32(Base[Instance]->CTRL, CRC_CTRL_FXOR_Msk, CRC_CTRL_FXOR_Pos, En);
}

/**
 * @brief Get CRC read xor mode.
 * @note  Function ID : DES_CRC_API_314
 * @param[in] Instance: CRC Hardware Device instance
 * @return read xor mode
 */
LOCAL_INLINE boolean Crc_Reg_GetResultXorMode(uint8 Instance)
{
    return (boolean)((Base[Instance]->CTRL & CRC_CTRL_FXOR_Msk) >> CRC_CTRL_FXOR_Pos);
}
/*cstat +MISRAC2012-Rule-11.4*/
/*==================================================================================================
 *                                        GLOBAL FUNCTIONS
==================================================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* AC784XX_CRC_REG_H */
