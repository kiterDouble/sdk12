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
 * @file Esm_Hal.c
 *
 * @brief This file provides Hal Esm api.
 *
 */

/*==============================================INCLUDE FILES=======================================*/

#include "AC784xx_Esm_Reg.h"
#include "AC784xx_Eim_Reg.h"
#include "Esm_Hal.h"

#ifdef IRQ_CONTROL_IN_ESM_HAL
#include "Core_Hal.h"
#endif

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/*!
 * @brief pointer of ESM configuration.
 */
static const Esm_ConfigType *Esm_Hal_ConfigPtr;

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Initialize ESM module.
 * @note  Function ID: DES_ESM_API_001
 * @warning This function should be called at privileged mode.
 * @param[in] ConfigPtr: pointer of ESM driver configuration.
 * @return void
 */
void Esm_Hal_Init(const Esm_ConfigType *ConfigPtr)
{
    DEVICE_ASSERT(ConfigPtr != NULL_PTR);

    if (ConfigPtr != NULL_PTR)
    {
        Esm_Reg_SetInterruptSources(ConfigPtr->Irq_Src);

        Esm_Hal_ConfigPtr = ConfigPtr;

#if defined (IRQ_CONTROL_IN_ESM_HAL) && (IRQ_CONTROL_IN_ESM_HAL == 1)
        if ((ConfigPtr->Irq_Src & (uint32)ESM_1BIT_IRQS) != 0U)
        {
            Core_Hal_EnableIrq(ECC_1BIT_ERROR_IRQn);
        }

        if ((ConfigPtr->Irq_Src & (uint32)ESM_2BIT_IRQS) != 0U)
        {
            Core_Hal_EnableIrq(ECC_2BIT_ERROR_IRQn);
        }
#endif
    }
}

/*!
 * @brief De-Initialize ESM module.
 * @note  Function ID: DES_ESM_API_004
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Esm_Hal_DeInit(void)
{
    DEVICE_ASSERT(Esm_Hal_ConfigPtr != NULL_PTR);

#if defined (IRQ_CONTROL_IN_ESM_HAL) && (IRQ_CONTROL_IN_ESM_HAL == 1)
    if ((Esm_Hal_ConfigPtr->Irq_Src & (uint32)ESM_1BIT_IRQS) != 0U)
    {
        Core_Hal_DisableIrq(ECC_1BIT_ERROR_IRQn);
        Core_Hal_ClearPendingIrq(ECC_1BIT_ERROR_IRQn);
    }

    if ((Esm_Hal_ConfigPtr->Irq_Src & (uint32)ESM_2BIT_IRQS) != 0U)
    {
        Core_Hal_DisableIrq(ECC_2BIT_ERROR_IRQn);
        Core_Hal_ClearPendingIrq(ECC_2BIT_ERROR_IRQn);
    }
#endif

    Esm_Hal_ConfigPtr = NULL_PTR;

    Esm_Reg_SetInterruptSources(0U);
}

/*!
 * @brief Get current ESM error Information.
 * @note  Function ID: DES_ESM_API_002
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: ESM channel.
 * @param[out] ErrorPtr: Pointer to where to store the error information of ESM.
 * @return error status.
 *            -STATUS_SUCCESS: no error occurred.
 *            -STATUS_ERROR: error detected.
 */
Hal_StatusType Esm_Hal_GetErrorInfo(Esm_ChannelType Channel, Esm_ErrorInfoType *ErrorPtr)
{
    uint32 Addr = 0U; //Channel Base Address;

    if (Channel == ESM_CHANNEL_SRAML)
    {
        Addr = SRAM_L_BASE;
    }
    else if (Channel == ESM_CHANNEL_SRAMU)
    {
        Addr = SRAM_U_BASE;
    }
#if defined (AC7843X)
    else if (Channel == ESM_CHANNEL_BOOTROM)
    {
        /* Addr = BOOTROM_BASE */
    }
#endif
    else
    {
        DEVICE_ASSERT(Channel < ESM_CHANNEL_MAX);
    }

    Hal_StatusType ReturnVal = STATUS_SUCCESS;

    uint32 Status = Esm_Reg_GetErrorStatus(Channel);
    if (Status != 0U) //000b means no error
    {
        if (ErrorPtr != NULL_PTR)
        {
            if ((Status == 1U) || (Status == 5U)) // x01b means 2bit error
            {
                ErrorPtr->Err_Status = (uint32)ESM_2BIT_ERR;
                ErrorPtr->Err_Addr = Esm_Reg_Get2bitErrorAddress(Channel);
            }
            else // 010/011b/11xb/100b means 1bit error
            {
                ErrorPtr->Err_Status = (uint32)ESM_1BIT_ERR;
                ErrorPtr->Err_Addr = Esm_Reg_Get1bitErrorAddress(Channel);
            }

            ErrorPtr->Err_Addr = (ErrorPtr->Err_Addr << 2U) + Addr;
        }

        ReturnVal = STATUS_ERROR;
    }

    return ReturnVal;
}

/*!
 * @brief Clear ESM error information.
 * @note  Function ID: DES_ESM_API_003
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: ESM channel.
 * @return void
 */
void Esm_Hal_ClearErrorInfo(Esm_ChannelType Channel)
{
    DEVICE_ASSERT(Channel < ESM_CHANNEL_MAX);

    Esm_Reg_ClearErrorInfo(Channel);
}

#if defined (AC7843X)
/*!
 * @brief Get current E2E ECC error Information.
 * @note  Function ID: DES_ESM_API_006
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: E2E EIM channel.
 * @param[out] ErrorPtr: Pointer to where to store the error information of E2E ECC.
 * @return error status.
 *            -STATUS_SUCCESS: no error occurred.
 *            -STATUS_ERROR: error detected.
 */
Hal_StatusType Esm_Hal_GetE2EErrorInfo(Eim_E2EChannelType Channel, Esm_ErrorInfoType *ErrorPtr)
{
    DEVICE_ASSERT(Channel < EIM_E2E_CHANNEL_MAX);
    DEVICE_ASSERT(Channel >= EIM_E2E_CHANNEL0);

    Hal_StatusType ReturnVal = STATUS_SUCCESS;

    uint32 Status;

    if (Channel < EIM_E2E_CHANNEL11)
    {
        Status = (Esm_Reg_GetE2EErrorStatus0() >> (((uint32)Channel - (uint32)EIM_E2E_CHANNEL0) * 2U)) & 3U;
    }
    else
    {
        Status = (Esm_Reg_GetE2EErrorStatus1() >> (((uint32)Channel - (uint32)EIM_E2E_CHANNEL11) * 2U)) & 3U;
    }

    if (Status != 0U) //00b means no error
    {
        if (ErrorPtr != NULL_PTR)
        {
            ErrorPtr->Err_Status = Status;
            ErrorPtr->Err_Addr = 0U;
        }

        ReturnVal = STATUS_ERROR;
    }

    return ReturnVal;
}

/*!
 * @brief Clear E2E ECC error information.
 * @note  Function ID: DES_ESM_API_007
 * @warning This function should be called at privileged mode.
 * @param[in] Channel: E2E EIM channel.
 * @return void
 */
void Esm_Hal_ClearE2EErrorInfo(Eim_E2EChannelType Channel)
{
    DEVICE_ASSERT(Channel < EIM_E2E_CHANNEL_MAX);
    DEVICE_ASSERT(Channel >= EIM_E2E_CHANNEL0);

    if (Channel < EIM_E2E_CHANNEL11)
    {
        WRITE_REG32(ECC_SRAM->E2E_SLAVE_STA, 3UL << (((uint32)Channel - (uint32)EIM_E2E_CHANNEL0) * 2U));
    }
    else
    {
        WRITE_REG32(ECC_SRAM->E2E_MASTER_STA, 3UL << (((uint32)Channel - (uint32)EIM_E2E_CHANNEL11) * 2U));
    }
}
#endif

#if defined (IRQ_CONTROL_IN_ESM_HAL) && (IRQ_CONTROL_IN_ESM_HAL == 1)
/*!
 * @brief ECC 1 bit IRQ handler.
 * @note  Function ID: DES_ESM_API_004
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
ISR(ECC_1BIT_IRQHandler) //PRQA S 1503,3408 # it is handler.
{
    /* Disable globally ECC injection function to avoid receive ECC fault recursively.*/
    Eim_Reg_EnableGlobal(FALSE);

    for (Esm_ChannelType Channel = ESM_CHANNEL_SRAML; Channel < ESM_CHANNEL_MAX;
            Channel++) //PRQA S 4527 # it is used to iterate channels.
    {
        uint32 IntStatus;

        /* Read the interrupt status to identify wheter the interrupt is valid. */
        IntStatus = Esm_Reg_GetInterruptStatus(Channel);
        if ((IntStatus & (uint32)ESM_1BIT_ERR) != 0U)
        {
            Hal_StatusType Ret;
            Esm_ErrorInfoType Error;

            /* Read more error information. */
            Ret = Esm_Hal_GetErrorInfo(Channel, &Error);
            if (Ret != STATUS_SUCCESS)
            {
                /* Clear Error interrupt flag, state and address. */
                Esm_Reg_ClearErrorInfo(Channel);

                if ((Esm_Hal_ConfigPtr != NULL_PTR) && (Esm_Hal_ConfigPtr->Irq_1bitCallback != NULL_PTR))
                {
                    Esm_Hal_ConfigPtr->Irq_1bitCallback((void *)&Error);
                }
            }
        }
    }
}

/*!
 * @brief ECC 2 bit IRQ handler.
 * @note  Function ID: DES_ESM_API_005
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
ISR(ECC_2BIT_IRQHandler) //PRQA S 1503,3408 # it is handler.
{
    /* Disable globally ECC injection function to avoid receive ECC fault recursively.*/
    Eim_Reg_EnableGlobal(FALSE);

    for (Esm_ChannelType Channel = ESM_CHANNEL_SRAML; Channel < ESM_CHANNEL_MAX;
            Channel++) //PRQA S 4527 # it is used to iterate channels.
    {
        uint32 IntStatus;

        /* Read the interrupt status to identify wheter the interrupt is valid. */
        IntStatus = Esm_Reg_GetInterruptStatus(Channel);
        if ((IntStatus & (uint32)ESM_2BIT_ERR) != 0U)
        {
            Hal_StatusType Ret;
            Esm_ErrorInfoType Error;

            /* Read more error information. */
            Ret = Esm_Hal_GetErrorInfo(Channel, &Error);
            if (Ret != STATUS_SUCCESS)
            {
                /* Clear Error interrupt flag, state and address. */
                Esm_Reg_ClearErrorInfo(Channel);

                if ((Esm_Hal_ConfigPtr != NULL_PTR) && (Esm_Hal_ConfigPtr->Irq_2bitCallback != NULL_PTR))
                {
                    Esm_Hal_ConfigPtr->Irq_2bitCallback((void *)&Error);
                }
            }
        }
    }
}
#endif
