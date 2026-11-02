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
 * @file Rcm_Hal.c
 *
 * @brief This file provides Hal Rcm api.
 *
 * @details Comments augmented via MCP C/Doxygen guideline: document functions with `@brief/@param/@return`,
 *          describe macros and enums concisely, and surface constraints via `@note`/`@warning` when needed.
 */
/*==============================================INCLUDE FILES=======================================*/
#include "Rcm_Hal.h"
#include "Core_Hal.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/** @brief Rcm interrupt handler callback function */
static Hal_CallbackType Rcm_IsrCallback = NULL_PTR;

ISR(RCM_IRQHandler);
/*============================================FUNCTION PROTOTYPES===================================*/
/* Implementation of Rcm_Hal_Init declared in Rcm_Hal.h */
void Rcm_Hal_Init(const Rcm_ConfigType *ConfigPtr)
{
    uint8 filter;

    DEVICE_ASSERT(NULL_PTR != ConfigPtr);
    if (NULL_PTR != ConfigPtr)
    {
        Rcm_Reg_EnableResetInterrupts(ConfigPtr->InterruptSource >> CKGEN_RCM_CTRL_SW_RST_INT_EN_Pos);
        Rcm_Reg_EnableResetSources(ConfigPtr->ResetSource);

        filter = (uint8)(ConfigPtr->FilterValue);
        if (filter != 0U) /* Enable external pin input filtering */
        {
#if defined(AC7840X)
            /* Val = 0 means 127 cycles */
            if (filter >= RESET_FILTER_MAX_VALUE)
            {
                filter = 0;
            }
            else /* Val = N means N-1 cycles */
            {
                filter++;
            }
#endif /* AC7840X */
            Rcm_Reg_SetExtResetFilter(filter);

            Rcm_Reg_EnableExtResetFilter(TRUE);
        }
        else /* Disable external pin input filtering */
        {
            Rcm_Reg_EnableExtResetFilter(FALSE);
        }
        Rcm_Reg_SetResetDelayTime((uint32)ConfigPtr->DelayTime);
        Rcm_IsrCallback = ConfigPtr->RcmCallback;
#if (STD_ON == RCM_IRQ_CONTROL_INTERNAL)
        /* Enable global reset interrupt if the interrupt source is not zero */
        if (NULL_PTR != Rcm_IsrCallback)
        {
            Core_Hal_EnableIrq(RCM_IRQn);
        }
#endif /* endif of RCM_IRQ_CONTROL_INTERNAL */
        /* Enable global reset interrupt if the interrupt source is not zero */
        if (ConfigPtr->InterruptSource != 0U)
        {
            Rcm_Reg_EnableGlobalResetInterrupt(TRUE);
        }
    }
}

/* Implementation of Rcm_Hal_GetResetStatus declared in Rcm_Hal.h */
uint32 Rcm_Hal_GetResetStatus(void)
{
    return Rcm_Reg_GetResetStatus();
}

/* Implementation of Rcm_Hal_ClearResetStatus declared in Rcm_Hal.h */
void Rcm_Hal_ClearResetStatus(void)
{
    Rcm_Reg_ClearResetStatus();
}

/* Implementation of Rcm_Hal_SetResetState declared in Rcm_Hal.h */
void Rcm_Hal_SetResetState(Rcm_ResetIDType ResetId, Rcm_ResetStateType ResetState)
{
    Rcm_Reg_SetResetState((uint32)ResetId, (uint32)ResetState);
}

/*!
 * @brief reset interrupt.
 * @note  Function ID: DES_MCU_API_405
 * @return void
 */
ISR(RCM_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    /* rcm callback data */
    uint32 Status = Rcm_Reg_GetResetInterruptStatus();

    Rcm_Reg_ClearResetInterruptStatus();
    /* callback not null pointer, so callback to user */
    if (Rcm_IsrCallback != NULL_PTR)
    {
        Rcm_IsrCallback((void *)&Status);
    }
}
