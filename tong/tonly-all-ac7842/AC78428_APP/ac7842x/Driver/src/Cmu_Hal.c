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
 * @file Cmu_Hal.c
 *
 * @brief cmu hal source file.
 *
 * @details Comments augmented via MCP C/Doxygen guideline: document functions with `@brief/@param/@return`,
 *          describe macros and enums concisely, provide module usage and constraints in `@note`/`@warning` when needed.
 */
/*==============================================INCLUDE FILES=======================================*/

#include "Cmu_Hal.h"
#include "OsIf_Critical.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/* cmu module count value */
#define CMU_MODULE_COUNT ((uint8)CMU_MODULE_MAX)

/* cmu window length minimum value */
#define CMU_WINLEN_MIN_VALUE (7U)

/* cmu threshold maximum value */
#define CMU_THRESHOLD_MAX_VALUE (0xFFFFFFU)

/* timeout value to wait cmu work status stable */
#define CMU_WAIT_TIMEOUT (100000U)

/*===================================================ENUMS==========================================*/
/*!
 * @brief cmu state machine
 */
typedef enum
{
    CMU_INITED = 0x0U,
    CMU_UNINITED,
    CMU_RUNNING,
    CMU_ABNORMAL
} Cmu_StateType;
/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
static Cmu_StateType CmuStatus[CMU_MODULE_COUNT] =
{
    CMU_UNINITED,
    CMU_UNINITED,
    CMU_UNINITED
#if defined (AC7840E) || defined (AC7843X)
    , CMU_UNINITED
#endif /* AC7840E AC7843X */
};
/*============================================FUNCTION PROTOTYPES===================================*/

/**
 * @brief Disable cmu module by wait timeout
 * @note Function ID: DES_MCU_API_406
 * @param[in] CmuId: CMU module id
 * @return Hal_StatusType: Disable cmu module status ,STATUS_TIMEOUT or STATUS_SUCCESS
 */
static Hal_StatusType Cmu_Hal_DisabeWithSync(uint8 CmuIdx)
{
#if defined (AC7840X)
    volatile uint8 CmuStatus0;                /* the value of cmu status register */
    volatile uint8 CmuStatus1;                /* the value of cmu status register */
    uint32 Timeout;                           /* timeout value to wait cmu is disabled status */
#endif
    Hal_StatusType RetValue = STATUS_SUCCESS; /* function return value */
#if defined (AC7840X)
    OSIF_ENTER_CRITICAL(CMU_HAL_ID1);
    /* wait cmu work status can be closed */
    for (Timeout = CMU_WAIT_TIMEOUT; 0U != Timeout; Timeout--)
    {
        CmuStatus0 = (uint8)((Cmu_Reg_GetStatus(CmuIdx) & CMU_SR_WORK_Msk) >> CMU_SR_WORK_Pos);
        CmuStatus1 = (uint8)((Cmu_Reg_GetStatus(CmuIdx) & CMU_SR_WORK_Msk) >> CMU_SR_WORK_Pos);
        /*PRQA S 3415 ++ # AMCE_MCU_3415*/
        if ((0U == CmuStatus0) && (1U == CmuStatus1))/* 0 means stop counting, 1 means counting is workoing */
            /*PRQA S 3415 -- */
        {
            Cmu_Reg_Enable(CmuIdx, FALSE);
            break; /*  can be closed only during the counting phase */
        }
    }
    OSIF_EXIT_CRITICAL(CMU_HAL_ID1);
    if (0U == Timeout) /* wait cmu work status equal to IsEnable timeout */
    {
        RetValue = STATUS_TIMEOUT;
    }
#else
    Cmu_Reg_Enable(CmuIdx, FALSE);
#endif
    return RetValue;
}

/* Implementation of Cmu_Hal_Init declared in Cmu_Hal.h */
Hal_StatusType Cmu_Hal_Init(Cmu_ModuleIdType CmuId, const Cmu_CtrlParamType *CtrlParam)
{
    Hal_StatusType RetValue = STATUS_ERROR; /* function return value */
    uint8 CmuIdx = (uint8)CmuId;
    Cmu_StateType CmuState;

    DEVICE_ASSERT(CmuId < CMU_MODULE_MAX);
    DEVICE_ASSERT(NULL_PTR != CtrlParam);
    if ((CmuIdx < CMU_MODULE_COUNT) && (NULL_PTR != CtrlParam))
    {
        CmuState = CmuStatus[CmuIdx];
        DEVICE_ASSERT(CtrlParam->HighThreshold <= CMU_THRESHOLD_MAX_VALUE);
        DEVICE_ASSERT(CtrlParam->LowThreshold <= CMU_THRESHOLD_MAX_VALUE);
        DEVICE_ASSERT(CMU_WINLEN_MIN_VALUE <= CtrlParam->WinLen);

        switch (CmuState)
        {
        case CMU_UNINITED:
        case CMU_INITED:
            Cmu_Reg_ClearStatus(CmuIdx);
            Cmu_Reg_SetWindowLen(CmuIdx, CtrlParam->WinLen);
            Cmu_Reg_SetHighThreshold(CmuIdx, CtrlParam->HighThreshold);
            Cmu_Reg_SetLowThreshold(CmuIdx, CtrlParam->LowThreshold);
            CmuStatus[CmuIdx] = CMU_INITED;
            RetValue = STATUS_SUCCESS;
            break;
        default:
            RetValue = STATUS_ERROR;
            break;
        }
    }

    return RetValue;
}
/* Implementation of Cmu_Hal_Deinit declared in Cmu_Hal.h */
void Cmu_Hal_Deinit(Cmu_ModuleIdType CmuId) //PRQA S 1503 # AMCE_QAC_NoUsedApi
{
#if defined (AC7840X)
    Hal_StatusType RetValue; /* function return value */
#endif /* AC7840X */
    uint8 CmuIdx = (uint8)CmuId;
    Cmu_StateType CmuState;

    DEVICE_ASSERT(CmuId < CMU_MODULE_MAX);
    if (CmuIdx < CMU_MODULE_COUNT)
    {
        CmuState = CmuStatus[CmuIdx];
        if (CMU_RUNNING == CmuState)
        {
#if defined (AC7840X)
            RetValue = Cmu_Hal_DisabeWithSync(CmuIdx);
            if (STATUS_SUCCESS == RetValue)
            {
                CmuStatus[CmuIdx] = CMU_INITED;
            }
            else
            {
                CmuStatus[CmuIdx] = CMU_ABNORMAL;
            }
#else
            (void)Cmu_Hal_DisabeWithSync(CmuIdx);
            CmuStatus[CmuIdx] = CMU_INITED;
#endif /* AC7840X */
        }
        CmuState = CmuStatus[CmuIdx];
        if (CMU_INITED == CmuState)
        {
            Cmu_Reg_ClearStatus(CmuIdx);
            CmuStatus[CmuIdx] = CMU_UNINITED;
        }
    }
}
/* Implementation of Cmu_Hal_Enable declared in Cmu_Hal.h */
Hal_StatusType Cmu_Hal_Enable(Cmu_ModuleIdType CmuId, boolean IsEnable)
{
    Hal_StatusType RetValue = STATUS_SUCCESS; /* function return value */
    uint8 CmuIdx = (uint8)CmuId;
    Cmu_StateType CmuState;

    DEVICE_ASSERT(CmuId < CMU_MODULE_MAX);
    if (CmuIdx < CMU_MODULE_COUNT)
    {
        CmuState = CmuStatus[CmuIdx];
        if (TRUE == IsEnable)
        {
            if (CMU_INITED == CmuState)
            {
                Cmu_Reg_Enable(CmuIdx, TRUE);
                CmuStatus[CmuIdx] = CMU_RUNNING;
            }
            else if (CMU_RUNNING == CmuState)
            {
                RetValue = STATUS_SUCCESS;
            }
            else
            {
                RetValue = STATUS_ERROR;
            }
        }
        else
        {
            if (CMU_RUNNING == CmuState)
            {
#if defined (AC7840X)
                RetValue = Cmu_Hal_DisabeWithSync(CmuIdx);
                if (STATUS_SUCCESS == RetValue)
                {
                    CmuStatus[CmuIdx] = CMU_INITED;
                }
                else
                {
                    CmuStatus[CmuIdx] = CMU_ABNORMAL;
                }
#else
                (void)Cmu_Hal_DisabeWithSync(CmuIdx);
                CmuStatus[CmuIdx] = CMU_INITED;
#endif /* AC7840X */
            }
            else if (CMU_ABNORMAL == CmuState)
            {
#if defined (AC7840X)
                RetValue = Cmu_Hal_DisabeWithSync(CmuIdx);
                if (STATUS_SUCCESS == RetValue)
                {
                    CmuStatus[CmuIdx] = CMU_INITED;
                }
                else
                {
                    CmuStatus[CmuIdx] = CMU_ABNORMAL;
                }
#else
                (void)Cmu_Hal_DisabeWithSync(CmuIdx);
                CmuStatus[CmuIdx] = CMU_INITED;
#endif /* AC7840X */
            }
            else if (CMU_INITED == CmuState)
            {
                RetValue = STATUS_SUCCESS;
            }
            else
            {
                RetValue = STATUS_ERROR;
            }
        }
    }
    else
    {
        RetValue = STATUS_ERROR;
    }

    return RetValue;
}
/* Implementation of Cmu_Hal_ClearErrStatus declared in Cmu_Hal.h */
void Cmu_Hal_ClearErrStatus(Cmu_ModuleIdType CmuId) /*cstat !MISRAC2004-8.10*/ /*cstat !MISRAC2012-Rule-8.7*/
{
    uint8 CmuIdx = (uint8)CmuId;
    Cmu_StateType CmuState;

    DEVICE_ASSERT(CmuId < CMU_MODULE_MAX);
    if (CmuIdx < CMU_MODULE_COUNT)
    {
        CmuState = CmuStatus[CmuIdx];
        if (CMU_UNINITED != CmuState)
        {
            Cmu_Reg_ClearStatus(CmuIdx);
        }
    }
}
/* Implementation of Cmu_Hal_GetErrInfo declared in Cmu_Hal.h */
Hal_StatusType Cmu_Hal_GetErrInfo(Cmu_ModuleIdType CmuId, Cmu_ErrInfoType *ErrInfo)
{
    uint8 StatusRegVal;                     /* the value of cmu status register */
    Hal_StatusType RetValue = STATUS_ERROR; /* function return value */
    uint8 CmuIdx = (uint8)CmuId;
    Cmu_StateType CmuState;

    DEVICE_ASSERT(ErrInfo != NULL_PTR);
    DEVICE_ASSERT(CmuId < CMU_MODULE_MAX);
    if ((CmuIdx < CMU_MODULE_COUNT) && (ErrInfo != NULL_PTR))
    {
        CmuState = CmuStatus[CmuIdx];
        if (CMU_UNINITED != CmuState)
        {
            StatusRegVal = Cmu_Reg_GetStatus(CmuIdx);
            /* cmu status contains fll event */
            if (0U != (StatusRegVal & CMU_SR_FLL_Msk))
            {
                ErrInfo->IsFLL = TRUE;
            }
            else
            {
                ErrInfo->IsFLL = FALSE;
            }
            /* cmu status contains fhh event */
            if (0U != (StatusRegVal & CMU_SR_FHH_Msk))
            {
                ErrInfo->IsFHH = TRUE;
            }
            else
            {
                ErrInfo->IsFHH = FALSE;
            }
            /* cmu status contains ClkLoss event */
            if (0U != (StatusRegVal & CMU_SR_CLKLOSS_Msk))
            {
                ErrInfo->IsClkLoss = TRUE;
            }
            else
            {
                ErrInfo->IsClkLoss = FALSE;
            }
            /* error event occures, the result return STATUS_ERROR */
            if ((TRUE == ErrInfo->IsFLL) || (TRUE == ErrInfo->IsFHH) || (TRUE == ErrInfo->IsClkLoss))
            {
                RetValue = STATUS_ERROR;
            }
            else
            {
                RetValue = STATUS_SUCCESS;
            }
        }
    }

    return RetValue;
}

/* =============================================  EOF  ============================================== */
