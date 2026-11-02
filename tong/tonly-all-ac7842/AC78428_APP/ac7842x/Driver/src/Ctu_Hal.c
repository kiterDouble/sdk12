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
 * @file Ctu_Hal.c
 *
 * @brief ctu hal source file.
 */
/*==============================================INCLUDE FILES=======================================*/
#include "Ctu_Hal.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/* ctu module lock count */
#define CTU_MODULE_LOCK_CNT (23U)

/* ctu module lock count */
#define CTU_MODULE_ARRAY_CNT (21U)

/* ctu module offset */
#define CTU_MODULE_OFFSET (4U)
/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/*============================================FUNCTION PROTOTYPES===================================*/

/* Implementation of Ctu_Hal_EnableSWTrigger declared in Ctu_Hal.h */
void Ctu_Hal_EnableSWTrigger(Ctu_SWTriggerIdType Id, boolean Enable)
{
    Ctu_Reg_SetFunc((uint32)Id, Enable);
}

/* Implementation of Ctu_Hal_EnableFunc declared in Ctu_Hal.h */
void Ctu_Hal_EnableFunc(Ctu_FuncIdType FuncId, boolean Enable)
{
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
    switch (FuncId)
    {
    case CTU_ADC0_INTER_PB13:
        if (TRUE == Enable)
        {
            Ctu_Reg_SetAdc0Inter(1U);
        }
        else
        {
            Ctu_Reg_SetAdc0Inter(0U);
        }
        break;
    case CTU_ADC0_INTER_PB14:
        if (TRUE == Enable)
        {
            Ctu_Reg_SetAdc0Inter(3U);
        }
        else
        {
            Ctu_Reg_SetAdc0Inter(0U);
        }
        break;
    case CTU_ADC1_INTER_PB0:
        if (TRUE == Enable)
        {
            Ctu_Reg_SetAdc1Inter(1U);
        }
        else
        {
            Ctu_Reg_SetAdc1Inter(0U);
        }
        break;
    case CTU_ADC1_INTER_PB1:
        if (TRUE == Enable)
        {
            Ctu_Reg_SetAdc1Inter(3U);
        }
        else
        {
            Ctu_Reg_SetAdc1Inter(0U);
        }
        break;
    default:
        Ctu_Reg_SetFunc((uint32)FuncId, Enable);
        break;
    }
#elif defined(AC7840E)  /* AC7840X AC7842X AC7843X */
    Ctu_Reg_SetFunc((uint32)FuncId, Enable);
#endif
}

/* Implementation of Ctu_Hal_GetModuleTriggerSource declared in Ctu_Hal.h */
Ctu_TriggerSourceType Ctu_Hal_GetModuleTriggerSource(Ctu_TargetModuleType Module) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
    Ctu_TriggerSourceType Source;
    /*PRQA S 4342 ++ # AMCE_MCL_4342 */
    Source = (Ctu_TriggerSourceType)Ctu_Reg_GetModuleTrig((uint32)Module);
    /*PRQA S 4342 -- */
    return Source;
}

/* Implementation of Ctu_Hal_SetModuleTriggerSource declared in Ctu_Hal.h */
Hal_StatusType Ctu_Hal_SetModuleTriggerSource(Ctu_TargetModuleType Module, Ctu_TriggerSourceType Source)
{
    Hal_StatusType Status = STATUS_SUCCESS;

    if (0U == Ctu_Reg_GetModuleLock((uint32)Module))
    {
        Ctu_Reg_SetModuleTrig((uint32)Module, (uint32)Source);
    }
    else
    {
        Status = STATUS_ERROR;
    }
    return Status;
}

/* Implementation of Ctu_Hal_Init declared in Ctu_Hal.h */
void Ctu_Hal_Init(const Ctu_CfgType *CfgPtr)
{
    uint32 Idx;                           /* used for loop*/
    const Ctu_MappingCfgType *MappingCfg; /* ctu mapping configuration */
    boolean IsLock[CTU_MODULE_LOCK_CNT];

    (void)Ckgen_Hal_EnablePeriphClk(CKGEN_CTU_BUS_CLK, TRUE);
    Rcm_Hal_SetResetState(RCM_RESET_ID_CTU, RCM_RESET_STATE_DEASSERT);
    if (NULL_PTR != CfgPtr)
    {
        for (Idx = 0U; Idx < CTU_MODULE_LOCK_CNT; Idx++)
        {
            IsLock[Idx] = FALSE;
        }
        if (0U != CfgPtr->CfgCnt)
        {
            DEVICE_ASSERT(NULL_PTR != CfgPtr->MappingCfgs);
        }
        /* config ctu */
        for (Idx = 0U; Idx < CfgPtr->CfgCnt; Idx++)
        {
            MappingCfg = &CfgPtr->MappingCfgs[Idx];
            Ctu_Reg_SetModuleTrig((uint32)MappingCfg->Module, (uint32)MappingCfg->Source);

            IsLock[((uint32)MappingCfg->Module / CTU_MODULE_OFFSET)] = TRUE;
        }
        for (Idx = 0U; Idx < CTU_MODULE_ARRAY_CNT; Idx++)
        {
            if (Idx > 13U)
            {
                Ctu_Reg_EnableModuleLock((Idx + 2U), IsLock[(Idx + 2U)]);
            }
            else
            {
                Ctu_Reg_EnableModuleLock(Idx, IsLock[Idx]);
            }
        }
    }
}

/* Implementation of Ctu_Hal_DeInit declared in Ctu_Hal.h */
void Ctu_Hal_DeInit(void)
{
    Rcm_Hal_SetResetState(RCM_RESET_ID_CTU, RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState(RCM_RESET_ID_CTU, RCM_RESET_STATE_DEASSERT);
    (void)Ckgen_Hal_EnablePeriphClk(CKGEN_CTU_BUS_CLK, FALSE);
}

/* =============================================  EOF  ============================================== */
