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
 * @file Eim_Hal.c
 *
 * @brief This file provides Hal Eim api.
 *
 */

/*==============================================INCLUDE FILES=======================================*/

#include "AC784xx_Eim_Reg.h"
#include "Eim_Hal.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
#ifndef SRAM_L_BASE
#define SRAM_L_BASE               (SRAM_L_START_ADDRESS)
#endif

#ifndef SRAM_L_END
#define SRAM_L_END                (SRAM_L_END_ADDRESS)
#endif

#ifndef SRAM_U_BASE
#define SRAM_U_BASE               (SRAM_U_START_ADDRESS)
#endif

#ifndef SRAM_U_END
#define SRAM_U_END                (SRAM_U_END_ADDRESS)
#endif

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/*!
 * @brief Lock EIM.
 * @note  Function ID: DES_EIM_API_001
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Eim_Hal_Lock(void)
{
    Eim_Reg_EnableGlobal(FALSE);
}

/*!
 * @brief Unlock EIM.
 * @note  Function ID: DES_EIM_API_002
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Eim_Hal_Unlock(void)
{
    Eim_Reg_EnableGlobal(TRUE);
}

/*!
 * @brief Setup EIM configuration.
 * @note  Function ID: DES_EIM_API_003
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: EIM channel.
 * @param[in] ConfigPtr: pointer of EIM channel configuration.
 * @return void
 */
void Eim_Hal_SetupConfig(Eim_ChannelType Channel, const Eim_ChannelConfigType *ConfigPtr)
{
    DEVICE_ASSERT(Channel < EIM_CHANNEL_MAX);
    DEVICE_ASSERT(ConfigPtr != NULL_PTR);

    if ((Channel < EIM_CHANNEL_MAX) && (ConfigPtr != NULL_PTR))
    {
        Eim_Reg_ConfigWord0(Channel, ConfigPtr->Eim_Word[0]);
        Eim_Reg_ConfigWord1(Channel, ConfigPtr->Eim_Word[1]);
        Eim_Reg_ConfigWord2(Channel, ConfigPtr->Eim_Word[2]);
    }
}

/*!
 * @brief Inject Fault to test ECC.
 * @note  Function ID: DES_EIM_API_004
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: EIM channel.
 * @param[in] Offset: EIM channel offset.
 * @return Value of EIM channel offset.
 */
uint32 Eim_Hal_InjectFault(Eim_ChannelType Channel, uint32 Offset)
{
    uint32 Addr = Offset;
    uint32 EnMsk = 0U;
    uint32 Val;

    if (Channel == EIM_CHANNEL_SRAML)
    {
        Addr += SRAM_L_BASE;
        EnMsk = EIM_CTRL_EIM_CHEN_EIM0_EN_Msk;
        DEVICE_ASSERT(Addr < SRAM_L_END);
    }
    else if (Channel == EIM_CHANNEL_SRAMU)
    {
        Addr += SRAM_U_BASE;
        EnMsk = EIM_CTRL_EIM_CHEN_EIM1_EN_Msk;
        DEVICE_ASSERT(Addr < SRAM_U_END);
    }
#if defined (AC7843X)
    else if (Channel == EIM_CHANNEL_BOOTROM)
    {
        /* Addr += BOOTROM_BASE */
        EnMsk = EIM_CTRL_EIM_CHEN_EIM2_EN_Msk;
        DEVICE_ASSERT(Addr < BOOTROM_END);
    }
#endif
    else
    {
        DEVICE_ASSERT(Channel < EIM_CHANNEL_MAX);
    }

    /* Below process should not be interrupt. */
    SET_BIT32(EIM_CTRL->EIM_CHEN, EnMsk);
    Val = READ_MEM32(Addr);
    WRITE_REG32(EIM_CTRL->EIM_CHEN, 0U);

    return Val;
}

/*!
 * @brief  Clear fault to test ECC.
 * @note  Function ID: DES_EIM_API_005
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: EIM channel.
 * @return void
 */
void Eim_Hal_ClearFault(Eim_ChannelType Channel)
{
    DEVICE_ASSERT(Channel < EIM_CHANNEL_MAX);

    if (Channel < EIM_CHANNEL_MAX)
    {
        Eim_Reg_ConfigWord0(Channel, 0U);
        Eim_Reg_ConfigWord1(Channel, 0U);
        Eim_Reg_ConfigWord2(Channel, 0U);
    }
}

#if defined (AC7843X)
/*!
 * @brief Setup E2E EIM configuration.
 * @note  Function ID: DES_EIM_API_006
 * @warning This function should be called at privileged mode.
 * @param[in] ConfigPtr: pointer of E2E EIM channel configuration.
 * @return void
 */
void Eim_Hal_SetupE2EConfig(const Eim_ChannelConfigType *ConfigPtr)
{
    DEVICE_ASSERT(ConfigPtr != NULL_PTR);

    if (ConfigPtr != NULL_PTR)
    {
        Eim_Reg_ConfigE2EWord0(ConfigPtr->Eim_Word[0]);
        Eim_Reg_ConfigE2EWord1(ConfigPtr->Eim_Word[1]);
    }
}

/*!
 * @brief Inject Fault to test E2E ECC.
 * @note  Function ID: DES_EIM_API_007
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: E2E EIM channel.
 * @return void
 */
void Eim_Hal_InjectE2EFault(Eim_E2EChannelType Channel)
{
    DEVICE_ASSERT(Channel < EIM_E2E_CHANNEL_MAX);
    DEVICE_ASSERT(Channel >= EIM_E2E_CHANNEL0);

    SET_BIT32(E2E_EIM->E2E_EIM_EN_INJ_ECC, (1UL << (uint32)Channel));
}

/*!
 * @brief  Clear fault to test E2E ECC.
 * @note  Function ID: DES_EIM_API_008
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Eim_Hal_ClearE2EFault(void)
{
    Eim_Reg_ConfigE2EWord0(0U);
    Eim_Reg_ConfigE2EWord1(0U);
}
#endif
