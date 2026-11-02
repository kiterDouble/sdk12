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
 * @file AC784xx_Eim_Reg.h
 *
 * @brief This file provides extern Low level Eim register api.
 *
 */

/*PRQA S 0306 EOF */

#ifndef AC784XX_EIM_REG_H
#define AC784XX_EIM_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"
#include "Esm_Hal_Types.h"
#include "Eim_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/*!
 * @brief EIM channel structure.
 */
static EIM_CHANNEL_Type *const Eim_Channel[EIM_CHANNEL_MAX] = EIM_BASE_PTRS;

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Globally enable/disable error injection function.
 * @note      Function ID: DES_EIM_API_100
 * @warning This function should be called at privileged mode.
 * @param[in] En: enable/disable.
 *            TRUE: enable.
 *            FALSE: disable.
 * @return void
 */
LOCAL_INLINE void Eim_Reg_EnableGlobal(boolean En)
{
    if (En == TRUE)
    {
        SET_BIT32(EIM_CTRL->EIM_GLB_ENABLE, EIM_CTRL_EIM_GLB_ENABLE_GLB_EN_Msk);
    }
    else
    {
        CLEAR_BIT32(EIM_CTRL->EIM_GLB_ENABLE, EIM_CTRL_EIM_GLB_ENABLE_GLB_EN_Msk);
    }
}

/*!
 * @brief Get error injection function globally enable/disable state.
 * @note      Function ID: DES_EIM_API_101
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return Enable/disable state.
 *            TRUE: enable.
 *            FALSE: disable.
 */
LOCAL_INLINE boolean Eim_Reg_GetEnableGlobal(void)
{
    return (boolean)READ_BIT32(EIM_CTRL->EIM_GLB_ENABLE, EIM_CTRL_EIM_GLB_ENABLE_GLB_EN_Msk);
}

/*!
 * @brief Enable/disable the corresponding error injection channel.
 * @note      Function ID: DES_EIM_API_102
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: error injection channel.
 * @param[in] En: enable/disable.
 *            TRUE: enable.
 *            FALSE: disable.
 * @return void
 */
LOCAL_INLINE void Eim_Reg_EnableChannel(Eim_ChannelType Channel, boolean En)
{
    DEVICE_ASSERT(Channel < EIM_CHANNEL_MAX);
    uint32 EnMsk = 0U;

    if (Channel == EIM_CHANNEL_SRAML)
    {
        EnMsk = EIM_CTRL_EIM_CHEN_EIM0_EN_Msk;
    }
    else if (Channel == EIM_CHANNEL_SRAMU)
    {
        EnMsk = EIM_CTRL_EIM_CHEN_EIM1_EN_Msk;
    }
#if defined (AC7843X)
    else if (Channel == EIM_CHANNEL_BOOTROM)
    {
        EnMsk = EIM_CTRL_EIM_CHEN_EIM2_EN_Msk;
    }
#endif
    else
    {
        DEVICE_ASSERT(EnMsk != 0U);
    }

    if (En == TRUE)
    {
        SET_BIT32(EIM_CTRL->EIM_CHEN, EnMsk);
    }
    else
    {
        CLEAR_BIT32(EIM_CTRL->EIM_CHEN, EnMsk);
    }
}

/*!
 * @brief Configure Injection word0 (Data).
 * @note      Function ID: DES_EIM_API_103
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: error injection channel.
 * @param[in] Pattern: injection pattern to check bit flip.
 * @return void
 */
LOCAL_INLINE void Eim_Reg_ConfigWord0(Eim_ChannelType Channel, uint32 Pattern)
{
    DEVICE_ASSERT(Channel < EIM_CHANNEL_MAX);

    WRITE_REG32(Eim_Channel[Channel]->EIM_INJ_DATA, Pattern);
}

/*!
 * @brief Configure Injection word1 (Address).
 * @note      Function ID: DES_EIM_API_104
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: error injection channel.
 * @param[in] Pattern: injection pattern to check bit flip.
 * @return void
 */
LOCAL_INLINE void Eim_Reg_ConfigWord1(Eim_ChannelType Channel, uint32 Pattern)
{
    DEVICE_ASSERT(Channel < EIM_CHANNEL_MAX);

    WRITE_REG32(Eim_Channel[Channel]->EIM_INJ_ADDR, Pattern);
}

/*!
 * @brief Configure Injection word2 (Ecc Check).
 * @note      Function ID: DES_EIM_API_105
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: error injection channel.
 * @param[in] Pattern: injection pattern to check bit flip.
 * @return void
 */
LOCAL_INLINE void Eim_Reg_ConfigWord2(Eim_ChannelType Channel, uint32 Pattern)
{
    DEVICE_ASSERT(Channel < EIM_CHANNEL_MAX);

    WRITE_REG32(Eim_Channel[Channel]->EIM_INJ_ECC, Pattern);
}

#if defined (AC7843X)
/*!
 * @brief Enable/disable the corresponding E2E error injection channel.
 * @note      Function ID: DES_EIM_API_106
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: E2E error injection channel.
 * @param[in] En: enable/disable.
 *            TRUE: enable.
 *            FALSE: disable.
 * @return void
 */
LOCAL_INLINE void Eim_Reg_EnableE2EChannel(Eim_E2EChannelType Channel, boolean En)
{
    DEVICE_ASSERT(Channel < EIM_E2E_CHANNEL_MAX);
    DEVICE_ASSERT(Channel >= EIM_E2E_CHANNEL0);

    if (En == TRUE)
    {
        SET_BIT32(E2E_EIM->E2E_EIM_EN_INJ_ECC, (1UL << (uint32)Channel));
    }
    else
    {
        CLEAR_BIT32(E2E_EIM->E2E_EIM_EN_INJ_ECC, (1UL << (uint32)Channel));
    }
}
/*!
 * @brief Configure E2E Injection word0 (Data and address).
 * @note      Function ID: DES_EIM_API_107
 * @warning This function should be called at privileged mode.
 * @param[in] Pattern: injection pattern to check bit flip.
 * @return void
 */
LOCAL_INLINE void Eim_Reg_ConfigE2EWord0(uint32 Pattern)
{
    WRITE_REG32(E2E_EIM->E2E_EIM_INJ_DA, Pattern);
}

/*!
 * @brief Configure E2E Injection word1 (Channel and Ecc).
 * @note      Function ID: DES_EIM_API_108
 * @warning This function should be called at privileged mode.
 * @param[in] Pattern: injection pattern to check bit flip.
 * @return void
 */
LOCAL_INLINE void Eim_Reg_ConfigE2EWord1(uint32 Pattern)
{
    WRITE_REG32(E2E_EIM->E2E_EIM_EN_INJ_ECC, Pattern);
}
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* AC784XX_EIM_REG_H */
