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
 * @file Core_Hal.c
 *
 * @brief This file provides Hal Core api.
 *
 * @details Comments augmented via MCP C/Doxygen guideline: document functions with `@brief/@param/@return`,
 *          describe macros and enums concisely, and note usage constraints in `@note`/`@warning` when needed.
 */

/*==============================================INCLUDE FILES=======================================*/
#include "Core_Hal.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
static const Core_ConfigType *Core_Hal_ConfigPtr;

/*============================================FUNCTION PROTOTYPES===================================*/
ISR(MCM_IRQHandler);

/* Implementation of Core_Hal_EnableIrq declared in Core_Hal.h */
void Core_Hal_EnableIrq(IRQn_Type IrqNumber)
{
    NVIC_ClearPendingIRQ(IrqNumber);
    NVIC_EnableIRQ(IrqNumber);
}

/* Implementation of Core_Hal_DisableIrq declared in Core_Hal.h */
void Core_Hal_DisableIrq(IRQn_Type IrqNumber)
{
    NVIC_DisableIRQ(IrqNumber);
}

/* Implementation of Core_Hal_IsIrqEnable declared in Core_Hal.h */
boolean Core_Hal_IsIrqEnable(IRQn_Type IrqNumber)
{
    /*cstat !MISRAC2012-Rule-10.5*/
    return (boolean)NVIC_GetEnableIRQ(IrqNumber);
}

/* Implementation of Core_Hal_SetIrqPriority declared in Core_Hal.h */
void Core_Hal_SetIrqPriority(IRQn_Type IrqNumber, uint32 Priority)
{
    NVIC_SetPriority(IrqNumber, Priority);
}

/* Implementation of Core_Hal_GetIrqPriority declared in Core_Hal.h */
uint32 Core_Hal_GetIrqPriority(IRQn_Type IrqNumber)
{
    return NVIC_GetPriority(IrqNumber);
}

/* Implementation of Core_Hal_PerformReset declared in Core_Hal.h */
void Core_Hal_PerformReset(void)
{
    NVIC_SystemReset();
}

/* Implementation of Core_Hal_SetPendingIrq declared in Core_Hal.h */
void Core_Hal_SetPendingIrq(IRQn_Type IrqNumber)
{
    NVIC_SetPendingIRQ(IrqNumber);
}

/* Implementation of Core_Hal_ClearPendingIrq declared in Core_Hal.h */
void Core_Hal_ClearPendingIrq(IRQn_Type IrqNumber)
{
    NVIC_ClearPendingIRQ(IrqNumber);
}

/* Implementation of Core_Hal_IsIrqPending declared in Core_Hal.h */
boolean Core_Hal_IsIrqPending(IRQn_Type IrqNumber)
{
    return (boolean)NVIC_GetPendingIRQ(IrqNumber);
}

/* Implementation of Core_Hal_IsIrqActive declared in Core_Hal.h */
boolean Core_Hal_IsIrqActive(IRQn_Type IrqNumber)
{
    return (boolean)NVIC_GetActive(IrqNumber);
}

/* Implementation of Core_Hal_SetIrqPriorityGrouping declared in Core_Hal.h */
void Core_Hal_SetIrqPriorityGrouping(uint32 PriorityGroup)
{
    NVIC_SetPriorityGrouping(PriorityGroup);
}

/* Implementation of Core_Hal_GetIrqPriorityGrouping declared in Core_Hal.h */
uint32 Core_Hal_GetIrqPriorityGrouping(void)
{
    return NVIC_GetPriorityGrouping();
}

/* Implementation of Core_Hal_SetPriorityMask declared in Core_Hal.h */
void Core_Hal_SetPriorityMask(uint32 PriorityMAsk)
{
    __set_PRIMASK(PriorityMAsk);
}

/* Implementation of Core_Hal_GetPriorityMask declared in Core_Hal.h */
uint32 Core_Hal_GetPriorityMask(void)
{
    return __get_PRIMASK();
}

#if ((defined(__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined(__FPU_USED) && (__FPU_USED == 1U)))
/* Implementation of Core_Hal_GetFpuErrorInfo declared in Core_Hal.h */
Hal_StatusType Core_Hal_GetFpuErrorInfo(Core_FpuErrorInfoType *ErrorPtr)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;
    uint32 FpuError;
#if defined(AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
    /* Get all float pointer exception status flags by read the MCM_MISCR register. */
    FpuError = Mcm_Reg_GetInterruptStatus() >> MCM_MISCR_FIOC_Pos;
    if (FpuError != 0U)
    {
        ReturnVal = STATUS_ERROR;

        /* Put the exception status to the structure if the pointer is not NULL. */
        if (NULL_PTR != ErrorPtr)
        {
            ErrorPtr->Err_Status = FpuError;
        }
    }
#if defined(AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif

    return ReturnVal;
}

/* Implementation of Core_Hal_ClearFpuErrorInfo declared in Core_Hal.h */
/*PRQA S 3006 ++ # AMCE_MCU_3006 */
void Core_Hal_ClearFpuErrorInfo(void)                               //PRQA S 1505 # AMCE_MCU_1505
{
    volatile uint32 *Fpscr = (volatile uint32 *)(FPU->FPCAR + 0x40U);
    register uint32 RegFPSCR = __get_FPSCR();

    /* Clear exceptions at low 8 bits. */
    RegFPSCR &= 0x0FFFFF00U;

    /* Clear exceptions at FPSCR register. */
    __set_FPSCR(RegFPSCR);

    /* Clear exceptions at FPSCR location at stack to avoid endless FPU interrupt. */
    *Fpscr = RegFPSCR;
}
/*PRQA S 3006 -- */
#endif

#if defined(AC7840X) || defined(AC7842X) || defined(AC7843X)
/* Implementation of Core_Hal_GetFlashCacheErrorInfo declared in Core_Hal.h */
Hal_StatusType Core_Hal_GetFlashCacheErrorInfo(Core_EccErrorInfoType *ErrorPtr)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;

#if defined(AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (--Timeout > 0U)
    {
        /* used for delay */
    }
#endif
#if defined(AC7840X) || defined(AC7842X)
    uint32 ErrStatus;
    /* Get cache error status flags by read the MCM_MCPESR register. */
    ErrStatus = Mcm_Reg_GetFlashCacheEccErrorStatus();
    if (ErrStatus != 0U)
    {
        ReturnVal = STATUS_ERROR;

        /* Put the error information to the structure if the pointer is not NULL. */
        if (NULL_PTR != ErrorPtr)
        {
            ErrorPtr->Err_Status = ErrStatus;
            ErrorPtr->Err_Addr = Mcm_Reg_GetFlashCacheEccErrorAddress();
        }
    }
#endif
#if defined(AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif

    return ReturnVal;
}

/* Implementation of Core_Hal_ClearFlashCacheErrorInfo declared in Core_Hal.h */
void Core_Hal_ClearFlashCacheErrorInfo(void)
{
#if defined(AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (--Timeout > 0U)
    {
        /* used for delay */
    }
#endif
#if defined(AC7840X) || defined(AC7842X)
    Mcm_Reg_ClearFlashCacheEccError();
#endif
#if defined(AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif
}
#endif

#if defined(AC7843X)
/* Implementation of Core_Hal_EnableICache declared in Core_Hal.h */
void Core_Hal_EnableICache(boolean En)
{
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
    if (En == TRUE)
    {
        SET_BIT32(MCM->ICACHE_CFG, MCM_ICACHE_CFG_EN_Msk);
    }
    else
    {
        CLEAR_BIT32(MCM->ICACHE_CFG, MCM_ICACHE_CFG_EN_Msk);
    }
    CKGEN->RCM_EN = RcmEnRegVal;
    /*add delay for icache clear finished*/
    Core_Hal_InvalidICache();
}

/* Implementation of Core_Hal_InvalidICache declared in Core_Hal.h */
void Core_Hal_InvalidICache(void)
{
    volatile uint32 i = 5U;
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }

    SET_BIT32(MCM->ICACHE_CFG, MCM_ICACHE_CFG_INVALID_Msk);
    while ((MCM->ICACHE_CFG & MCM_ICACHE_CFG_CLRDONE_Msk) == 0U)
    {
        if (i > 0U)
        {
            i = i - 1U;
        }
        else
        {
            break;
        }
    }
    CKGEN->RCM_EN = RcmEnRegVal;
}

/* Implementation of Core_Hal_GetICacheErrorInfo declared in Core_Hal.h */
Hal_StatusType Core_Hal_GetICacheErrorInfo(Core_EccErrorInfoType *ErrorPtr)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;
    uint32 ErrStatus;
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
    /* Get cache error status flags by read the MCM_DCACHE_CFG register. */
    ErrStatus = READ_BIT32(MCM->ICACHE_CFG, MCM_ICACHE_CFG_ECCSTA_Msk) >> MCM_ICACHE_CFG_ECCSTA_Pos;
    if (ErrStatus != 0U)
    {
        ReturnVal = STATUS_ERROR;

        /* Put the error information to the structure if the pointer is not NULL. */
        if (NULL_PTR != ErrorPtr)
        {
            ErrorPtr->Err_Status = ErrStatus;
            ErrorPtr->Err_Addr = 0U; // No Address supplied.
        }
    }
    CKGEN->RCM_EN = RcmEnRegVal;

    return ReturnVal;
}

/* Implementation of Core_Hal_ClearICacheErrorInfo declared in Core_Hal.h */
void Core_Hal_ClearICacheErrorInfo(void)
{
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
    SET_BIT32(MCM->ICACHE_CFG, MCM_ICACHE_CFG_ECCCLR_Msk);
    CKGEN->RCM_EN = RcmEnRegVal;
}

/* Implementation of Core_Hal_EnableDCache declared in Core_Hal.h */
void Core_Hal_EnableDCache(boolean En)
{
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }

    if (En == TRUE)
    {
        SET_BIT32(MCM->DCACHE_CFG, MCM_DCACHE_CFG_EN_Msk);
    }
    else
    {
        CLEAR_BIT32(MCM->DCACHE_CFG, MCM_DCACHE_CFG_EN_Msk);
    }
    CKGEN->RCM_EN = RcmEnRegVal;
    /*dcache clear finished*/
    Core_Hal_InvalidDCache();
}

/* Implementation of Core_Hal_InvalidDCache declared in Core_Hal.h */
void Core_Hal_InvalidDCache(void)
{
    volatile uint32 i = 5U;
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }

    SET_BIT32(MCM->DCACHE_CFG, MCM_DCACHE_CFG_INVALID_Msk);
    while ((MCM->DCACHE_CFG & MCM_DCACHE_CFG_CLRDONE_Msk) == 0U)
    {
        if (i > 0U)
        {
            i = i - 1U;
        }
        else
        {
            break;
        }
    }

    CKGEN->RCM_EN = RcmEnRegVal;
}

/* Implementation of Core_Hal_GetDCacheErrorInfo declared in Core_Hal.h */
Hal_StatusType Core_Hal_GetDCacheErrorInfo(Core_EccErrorInfoType *ErrorPtr)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;
    uint32 ErrStatus;
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }

    /* Get cache error status flags by read the MCM_DCACHE_CFG register. */
    ErrStatus = READ_BIT32(MCM->DCACHE_CFG, MCM_DCACHE_CFG_ECCSTA_Msk) >> MCM_DCACHE_CFG_ECCSTA_Pos;
    if (ErrStatus != 0U)
    {
        ReturnVal = STATUS_ERROR;

        /* Put the error information to the structure if the pointer is not NULL. */
        if (NULL_PTR != ErrorPtr)
        {
            ErrorPtr->Err_Status = ErrStatus;
            ErrorPtr->Err_Addr = 0U; // No Address supplied.
        }
    }
    CKGEN->RCM_EN = RcmEnRegVal;

    return ReturnVal;
}

/* Implementation of Core_Hal_ClearDCacheErrorInfo declared in Core_Hal.h */
void Core_Hal_ClearDCacheErrorInfo(void)
{
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }

    SET_BIT32(MCM->DCACHE_CFG, MCM_DCACHE_CFG_ECCCLR_Msk);
    CKGEN->RCM_EN = RcmEnRegVal;
}
#endif

/* Implementation of Core_Hal_GetUUID declared in Core_Hal.h */
void Core_Hal_GetUUID(uint32 *uuidBuffer)
{
    uint32 i;

    if (uuidBuffer != NULL_PTR)
    {
        for (i = 0U; i < 4U; i++)
        {
            uuidBuffer[i] = READ_MEM32(UUID_BASE + (i * 4U));
        }
    }
}

/* Implementation of Core_Hal_GetChipID declared in Core_Hal.h */
uint64 Core_Hal_GetChipID(void)
{
    uint64 ChipId = READ_MEM32(TYPEID_BASE);
    ChipId |= ((uint64)READ_MEM32(TYPEID_BASE + 4U) << 32U);

    return ChipId;
}

/* Implementation of Core_Hal_EnableNMI declared in Core_Hal.h */
void Core_Hal_EnableNMI(boolean En) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
#if defined(AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
    Mcm_Reg_EnableNmi(En);
#if defined(AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif
}

/* Implementation of Core_Hal_GetMBistExecStatus declared in Core_Hal.h */
Core_BistStatusType Core_Hal_GetMBistExecStatus(void)
{
    Core_BistStatusType ReturnVal;
    uint32 Result;
#if defined(AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
    /* Get the MBIST execute result from MCM_MBIST register. */
    Result = Mcm_Reg_GetMbistStatus();
#if defined(AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif
    switch (Result)
    {
    case 0U:
        ReturnVal = BIST_NORUN;
        break;
    case 1U:
        ReturnVal = BIST_OK;
        break;
    case 3U:
        ReturnVal = BIST_ERROR;
        break;
    default:
        ReturnVal = BIST_BUSY;
        break;
    }

    return ReturnVal;
}

/* Implementation of Core_Hal_Init declared in Core_Hal.h */
void Core_Hal_Init(const Core_ConfigType *ConfigPtr)
{
    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
#if defined(AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;
#endif
    if (ConfigPtr != NULL_PTR)
    {
#if defined(AC7843X)
        CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
        while (Timeout > 0U)
        {
            Timeout = Timeout - 1U;
        }
#endif
        Mcm_Reg_SetInterruptSources(ConfigPtr->Irq_Src);
#if defined(AC7843X)
        CKGEN->RCM_EN = RcmEnRegVal;
#endif
        Core_Hal_ConfigPtr = ConfigPtr;
#ifdef IRQ_CONTROL_IN_CORE_HAL
        if (ConfigPtr->Irq_Src != 0U)
        {
            Core_Hal_EnableIrq(MCM_IRQn);
        }
#endif
    }
}

/* Implementation of Core_Hal_DeInit declared in Core_Hal.h */
void Core_Hal_DeInit(void)
{
#if defined(AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
#ifdef IRQ_CONTROL_IN_CORE_HAL
    if (Core_Hal_ConfigPtr->Irq_Src != 0U)
    {
        Core_Hal_DisableIrq(MCM_IRQn);
    }
#endif

    Mcm_Reg_SetInterruptSources(0U);
#if defined(AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif
}

/*!
 * @brief MCM handler.
 * @note  Function ID: DES_CORE_API_230
 * @return void
 */
ISR(MCM_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    uint32 IntStatus;
#if defined(AC7843X)
    volatile uint32 Timeout = 60U;
    uint32 RcmEnRegVal = CKGEN->RCM_EN;

    CKGEN->RCM_EN = RcmEnRegVal & 0xFFFFFC10U;
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
    /* Read the interrupt status to identify wheter the interrupt is valid. */
    IntStatus = Mcm_Reg_GetInterruptStatus();
#if defined(AC7843X)
    CKGEN->RCM_EN = RcmEnRegVal;
#endif
    if (IntStatus != 0U)
    {
        /* Clear Error interrupt flag, state and address. */
        Core_Hal_ClearFpuErrorInfo();

        if ((Core_Hal_ConfigPtr != NULL_PTR) && (Core_Hal_ConfigPtr->Irq_Callback != NULL_PTR))
        {
            Core_Hal_ConfigPtr->Irq_Callback((void *)&IntStatus);
        }
    }
}

/* Implementation of Core_Hal_InitDWT declared in Core_Hal.h */
void Core_Hal_InitDWT(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    __DSB();
}
