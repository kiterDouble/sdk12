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
 * @file AC784xx_Esm_Reg.h
 *
 * @brief This file provides extern Low level Esm register api.
 *
 */

/*PRQA S 0306 EOF */

#ifndef AC784XX_ESM_REG_H
#define AC784XX_ESM_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"
#include "Esm_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/*!
 * @brief ESM channel status offset.
 */
static const uint8 Esm_ChannelStatusOffset[ESM_CHANNEL_MAX] =
{
    0x00U,
    0x08U,
#if defined (AC7843X)
    0x24U,
#endif
};

/*!
 * @brief ESM channel status0 and status1 register address.
 */
#define ESM_CHANNEL_STATUS0(ch) ((uint32 *)(ECC_SRAM_BASE + (uint32)(Esm_ChannelStatusOffset[ch])))
#define ESM_CHANNEL_STATUS1(ch) ((uint32 *)(ECC_SRAM_BASE + (uint32)(Esm_ChannelStatusOffset[ch]) + 4U))

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Get ESM error status.
 * @note      Function ID: DES_ESM_API_101
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: ESM Channel, See Esm_ChannelType definition.
 * @return ECC error status.
 */
LOCAL_INLINE uint32 Esm_Reg_GetErrorStatus(Esm_ChannelType Channel)
{
    DEVICE_ASSERT(Channel < ESM_CHANNEL_MAX);

    return (READ_MEM32(ESM_CHANNEL_STATUS0(Channel)) & ECC_SRAM_CH0_STATUS0_ERR_ST_Msk) >> ECC_SRAM_CH0_STATUS0_ERR_ST_Pos;
}

/*!
 * @brief Get ECC interrupt status.
 * @note      Function ID: DES_ESM_API_102
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: ESM Channel, See Esm_ChannelType definition.
 * @return ECC interrupt status.
 */
LOCAL_INLINE uint32 Esm_Reg_GetInterruptStatus(Esm_ChannelType Channel)
{
    DEVICE_ASSERT(Channel < ESM_CHANNEL_MAX);

    return (READ_MEM32(ESM_CHANNEL_STATUS1(Channel)) & (ECC_SRAM_CH0_STATUS1_ERR_STS0_Msk |
            ECC_SRAM_CH0_STATUS1_ERR_STS1_Msk))
           >> ECC_SRAM_CH0_STATUS1_ERR_STS0_Pos;
}
/*!
 * @brief Get Ecc 1bit error raw address.
 * @note      Function ID: DES_ESM_API_103
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: ESM Channel, See Esm_ChannelType definition.
 * @return ECC 1 bit error address.
 */
LOCAL_INLINE uint32 Esm_Reg_Get1bitErrorAddress(Esm_ChannelType Channel)
{
    DEVICE_ASSERT(Channel < ESM_CHANNEL_MAX);

    return (READ_MEM32(ESM_CHANNEL_STATUS0(Channel)) & ECC_SRAM_CH0_STATUS0_ERR_ADDR0_Msk) >>
           ECC_SRAM_CH0_STATUS0_ERR_ADDR0_Pos;
}

/*!
 * @brief Get Ecc 2bit error raw address.
 * @note      Function ID: DES_ESM_API_104
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: ESM Channel, See Esm_ChannelType definition.
 * @return ECC 2 bits error address.
 */
LOCAL_INLINE uint32 Esm_Reg_Get2bitErrorAddress(Esm_ChannelType Channel)
{
    DEVICE_ASSERT(Channel < ESM_CHANNEL_MAX);

    return (READ_MEM32(ESM_CHANNEL_STATUS1(Channel)) & ECC_SRAM_CH0_STATUS1_ERR_ADDR1_Msk) >>
           ECC_SRAM_CH0_STATUS1_ERR_ADDR1_Pos;
}

/*!
 * @brief Set ESM interrupt trigger sources.
 * @note      Function ID: DES_ESM_API_105
 * @warning This function should be called at privileged mode.
 * @param[in] IntSources: Interrupt sources which can trigger ESM interrupt.
 * @return void
 */
LOCAL_INLINE void Esm_Reg_SetInterruptSources(uint32 IntSources)
{
    WRITE_REG32(ECC_SRAM->ECC_ERR_CTRL, IntSources);
}

/*!
 * @brief Get ESM interrupt trigger sources.
 * @note      Function ID: DES_ESM_API_111
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return Interrupt sources which can trigger ESM interrupt.
 */
LOCAL_INLINE uint32 Esm_Reg_GetInterruptSources(void)
{
    return READ_REG32(ECC_SRAM->ECC_ERR_CTRL);
}
/*!
 * @brief Clear ESM ecc error address and status information.
 * @note      Function ID: DES_ESM_API_106
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: ESM Channel, See Esm_ChannelType definition.
 * @return void
 */
LOCAL_INLINE void Esm_Reg_ClearErrorInfo(Esm_ChannelType Channel)
{
    DEVICE_ASSERT(Channel < ESM_CHANNEL_MAX);

    uint8 Offset;

    if (Channel == ESM_CHANNEL_SRAMU)
    {
        Offset = 8U;
    }
#if defined (AC7843X)
    else if (Channel == ESM_CHANNEL_BOOTROM)
    {
        Offset = 24U;
    }
#endif
    else
    {
        Offset = 0U;
    }

    SET_BIT32(ECC_SRAM->ECC_ERR_CTRL, \
              (uint32)(ECC_SRAM_ECC_ERR_CTRL_ECC0_ST_CLR1_Msk | ECC_SRAM_ECC_ERR_CTRL_ECC0_ST_CLR2_Msk) << Offset);
}

#if defined (AC7843X)
/*!
 * @brief Get ESM E2E Status0
 * @note      Function ID: DES_ESM_API_107
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return E2E Status0
 */
LOCAL_INLINE uint32 Esm_Reg_GetE2EErrorStatus0(void)
{
    return READ_REG32(ECC_SRAM->E2E_SLAVE_STA);
}

/*!
 * @brief Set ESM E2E Status1.
 * @note      Function ID: DES_ESM_API_108
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return E2E Status1
 */
LOCAL_INLINE uint32 Esm_Reg_GetE2EErrorStatus1(void)
{
    return READ_REG32(ECC_SRAM->E2E_MASTER_STA);
}

/*!
 * @brief Clear ESM Status0.
 * @note      Function ID: DES_ESM_API_109
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Esm_Reg_ClearE2EErrorStatus0(void)
{
    WRITE_REG32(ECC_SRAM->E2E_SLAVE_STA, 0xFFFFFFFFU);
}

/*!
 * @brief Clear ESM Status1.
 * @note      Function ID: DES_ESM_API_110
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
LOCAL_INLINE void Esm_Reg_ClearE2EErrorStatus1(void)
{
    WRITE_REG32(ECC_SRAM->E2E_MASTER_STA, 0xFFFFFFFFU);
}
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* AC784XX_ESM_REG_H */
