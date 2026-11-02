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
 * AutoChips Inc. (C) 2021. All rights reserved.
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
 * @file Pdt_Hal.c
 *
 * @brief This file provides PDT integration functions.
 *
 */

/* ===========================================  Includes  =========================================== */
#include "Pdt_Hal.h"
#include "Ckgen_Hal.h"
#include "Core_Hal.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */
/*!
 * @brief PDT state type
 */
typedef enum
{
    PDT_UNINITED = 0x0U, /**< Un-initialization status. */
    PDT_IDLE,            /**< PDT has been initialized and is idle */
    PDT_RUNNING,         /**< PDT is running.*/
} Pdt_StateType;

/* ==========================================  Variables  =========================================== */
/* PDT base address */
static PDT_Type *const PdtBase[PDT_INSTANCE_MAX] = PDT_BASE_PTRS;

/* PRQA S 0686 ++ # AMCE_GPT_0686*/
/* PDT callback pointer */
static Hal_CallbackType PdtCallback[PDT_INSTANCE_MAX] = {NULL_PTR};
/* PRQA S 0686 -- # AMCE_GPT_0686*/
/* PDT interrupt request info */
static const IRQn_Type PdtIrq[PDT_INSTANCE_MAX] = {PDT0_IRQn, PDT1_IRQn};

/** @brief Variable of PDT state.*/
static Pdt_StateType PdtStatus[PDT_INSTANCE_MAX] = {PDT_UNINITED, PDT_UNINITED};

/* PDT ckgen clk info */
static const Ckgen_BusClkIdType PdtCkgenClks[PDT_INSTANCE_MAX] = PDT_CKGEN_CLOCKS;

/* ====================================  Functions declaration  ===================================== */

/* =====================================  Functions definition  ===================================== */
/* Implementation of Pdt_Hal_ConfigTimer declared in Pdt_Hal.h */
void Pdt_Hal_ConfigTimer(const uint8 Instance, const Pdt_TimerConfigType * const Config)    //PRQA S 1505 # AMCE_GPT_1505
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    /* PRQA S 2842,2812 ++ # AMCE_GPT_2842,AMCE_GPT_2812 */
    DEVICE_ASSERT(Config->LoadValueMode <= PDT_LOAD_VAL_AT_MODULO_COUNTER_OR_NEXT_TRIGGER);
    DEVICE_ASSERT(Config->ClkPreDiv <= PDT_CLK_PREDIV_BY_128);
    DEVICE_ASSERT(Config->ClkPreMultFactor <= PDT_CLK_PREMULT_FACT_AS_40);
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_RUNNING);

    PDT_Type *const Base = PdtBase[Instance];
    uint32 Sc = 0U;

    /* PRQA S 2986 ++ # AMCE_GPT_2986 */
    Sc |= (((uint32)Config->LoadValueMode << PDT_SC_LDMOD_Pos) \
        | ((uint32)Config->TriggerInput << PDT_SC_TRGSEL_Pos) \
        | ((uint32)Config->IntEnable << PDT_SC_PDTIE_Pos) \
        | ((uint32)Config->ContinuousModeEnable << PDT_SC_CONT_Pos) \
        | ((uint32)Config->ClkPreMultFactor) \
        | ((uint32)Config->ClkPreDiv << PDT_SC_PRE_Pos));
    /* PRQA S 2986 -- # AMCE_GPT_2986 */
    /*should not effect trigger delay bypass bit*/
    Sc |= READ_BIT32(Base->SC, PDT_SC_Bypass_En_Msk);

    WRITE_REG32(Base->SC, Sc);

    PDT_Reg_SetTimerModulusValue(Base, Config->ModeVal);
    /* Register callback function */
    PdtCallback[Instance] = Config->Callback;

    if (TRUE == Config->IntEnable)
    {
        PDT_Reg_SetValueForTimerInterrupt(Base, Config->IntDelayVal);
        Core_Hal_EnableIrq(PdtIrq[Instance]);
    }
    else
    {
        Core_Hal_DisableIrq(PdtIrq[Instance]);
    }
    /* PRQA S 2842,2812 -- # AMCE_GPT_2842,AMCE_GPT_2812 */
}

/* Implementation of Pdt_Hal_Init declared in Pdt_Hal.h */
void Pdt_Hal_Init(const uint8 Instance, const Pdt_TimerConfigType * const Config)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /* PRQA S 2842 ++ # AMCE_GPT_2842 */
    if (PDT_UNINITED == PdtStatus[Instance])
    {
#ifdef AC7840X
        /* Enable PDT clock */
        (void)Ckgen_Hal_EnablePeriphClk(PdtCkgenClks[0U], TRUE);
        (void)Ckgen_Hal_EnablePeriphClk(PdtCkgenClks[1U], TRUE);
#else
        /* Enable PDT clock */
        (void)Ckgen_Hal_EnablePeriphClk(PdtCkgenClks[Instance], TRUE);
#endif

        Pdt_Hal_ConfigTimer(Instance, Config);
        PdtStatus[Instance] = PDT_IDLE;
    }
    /* PRQA S 2842 -- # AMCE_GPT_2842 */
}

/* Implementation of Pdt_Hal_Start declared in Pdt_Hal.h */
void Pdt_Hal_Start(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    /* PRQA S 2842 ++ # AMCE_GPT_2842 */
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_UNINITED);

    PDT_Type *const Base = PdtBase[Instance];
    if (PDT_IDLE == PdtStatus[Instance])
    {
        PDT_Reg_Enable(Base, TRUE);
        PdtStatus[Instance] = PDT_RUNNING;
    }
    /* PRQA S 2842 -- # AMCE_GPT_2842 */
}

/* Implementation of Pdt_Hal_Stop declared in Pdt_Hal.h */
void Pdt_Hal_Stop(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    /* PRQA S 2842 ++ # AMCE_GPT_2842 */
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_UNINITED);

    PDT_Type *const Base = PdtBase[Instance];
    if (PDT_RUNNING == PdtStatus[Instance])
    {
        PDT_Reg_Enable(Base, FALSE);
        PdtStatus[Instance] = PDT_IDLE;
    }
    /* PRQA S 2842 -- # AMCE_GPT_2842 */
}

/* Implementation of Pdt_Hal_LoadValuesCmd declared in Pdt_Hal.h */
void Pdt_Hal_LoadValuesCmd(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    /* PRQA S 2842 ++ # AMCE_GPT_2842 */
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_UNINITED);

    PDT_Type *const Base = PdtBase[Instance];

    PDT_Reg_LoadValuesCmd(Base);
    /* PRQA S 2842 -- # AMCE_GPT_2842 */

}

/* Implementation of Pdt_Hal_SoftTriggerCmd declared in Pdt_Hal.h */
void Pdt_Hal_SoftTriggerCmd(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);

    PDT_Type *const Base = PdtBase[Instance];                   /* PRQA S 2842 # AMCE_GPT_2842 */

    PDT_Reg_SoftTriggerCmd(Base);
}

/* Implementation of Pdt_Hal_Deinit declared in Pdt_Hal.h */
void Pdt_Hal_Deinit(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    /* PRQA S 2842 ++ # AMCE_GPT_2842 */
    Core_Hal_DisableIrq(PdtIrq[Instance]);
    (void)Ckgen_Hal_EnablePeriphClk(PdtCkgenClks[Instance], FALSE);
    /* Clear callback */
    PdtCallback[Instance] = NULL_PTR;
    PdtStatus[Instance] = PDT_UNINITED;
    /* PRQA S 2842 -- # AMCE_GPT_2842 */
}

/* Implementation of Pdt_Hal_GetTimerValue declared in Pdt_Hal.h */
uint32 Pdt_Hal_GetTimerValue(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    /* PRQA S 2842 ++ # AMCE_GPT_2842 */
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_UNINITED);

    const PDT_Type *const Base = PdtBase[Instance];

    return PDT_Reg_GetTimerValue(Base);
    /* PRQA S 2842 -- # AMCE_GPT_2842 */
}

/* Implementation of Pdt_Hal_GetTimerIntFlag declared in Pdt_Hal.h */
boolean Pdt_Hal_GetTimerIntFlag(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    boolean Flag;

    const PDT_Type *const Base = PdtBase[Instance];                   /* PRQA S 2842 # AMCE_GPT_2842 */
    Flag = (boolean)PDT_Reg_GetTimerIntFlag(Base);
    return Flag;
}

/* Implementation of Pdt_Hal_ClearTimerIntFlag declared in Pdt_Hal.h */
void Pdt_Hal_ClearTimerIntFlag(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);

    PDT_Type *const Base = PdtBase[Instance];                  /* PRQA S 2842 # AMCE_GPT_2842 */

    PDT_Reg_ClearTimerIntFlag(Base);
}

/* Implementation of Pdt_Hal_SetTimerModulusValue declared in Pdt_Hal.h */
void Pdt_Hal_SetTimerModulusValue(const uint8 Instance, const uint16 Value)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    /* PRQA S 2842 ++ # AMCE_GPT_2842 */
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_UNINITED);

    PDT_Type *const Base = PdtBase[Instance];

    PDT_Reg_SetTimerModulusValue(Base, Value);
    /* PRQA S 2842 -- # AMCE_GPT_2842 */
}

/* Implementation of Pdt_Hal_SetValueForTimerInterrupt declared in Pdt_Hal.h */
void Pdt_Hal_SetValueForTimerInterrupt(const uint8 Instance, const uint16 Value)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    /* PRQA S 2842 ++ # AMCE_GPT_2842 */
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_UNINITED);

    PDT_Type *const Base = PdtBase[Instance];

    PDT_Reg_SetValueForTimerInterrupt(Base, Value);
    /* PRQA S 2842 -- # AMCE_GPT_2842 */
}

/* Implementation of Pdt_Hal_SetCmpPulseOutEnable declared in Pdt_Hal.h */
void Pdt_Hal_SetCmpPulseOutEnable(const uint8 Instance, const boolean Enable)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    /* PRQA S 2842 ++ # AMCE_GPT_2842 */
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_UNINITED);

    PDT_Type *const Base = PdtBase[Instance];

    PDT_Reg_SetCmpPulseOutEnable(Base, Enable);
    /* PRQA S 2842 -- # AMCE_GPT_2842 */
}

/* Implementation of Pdt_Hal_SetCmpPulseOutDelayForHigh declared in Pdt_Hal.h */
void Pdt_Hal_SetCmpPulseOutDelayForHigh(const uint8 Instance, const uint16 Value)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    /* PRQA S 2842 ++ # AMCE_GPT_2842 */
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_UNINITED);

    PDT_Type *const Base = PdtBase[Instance];

    PDT_Reg_SetCmpPulseOutDelayForHigh(Base, Value);
    /* PRQA S 2842 -- # AMCE_GPT_2842 */
}

/* Implementation of Pdt_Hal_SetCmpPulseOutDelayForLow declared in Pdt_Hal.h */
void Pdt_Hal_SetCmpPulseOutDelayForLow(const uint8 Instance, const uint16 Value)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    /* PRQA S 2842 ++ # AMCE_GPT_2842 */
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_UNINITED);

    PDT_Type *const Base = PdtBase[Instance];

    PDT_Reg_SetCmpPulseOutDelayForLow(Base, Value);
    /* PRQA S 2842 -- # AMCE_GPT_2842 */
}

/* Implementation of Pdt_Hal_SetTriggerDelayValue declared in Pdt_Hal.h */
void Pdt_Hal_SetTriggerDelayValue(const uint8 Instance, const uint8 Chn, const uint16 Value)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    DEVICE_ASSERT(Chn < PDT_DLY_MAX);

    PDT_Type *const Base = PdtBase[Instance];         /* PRQA S 2842 # AMCE_GPT_2842 */

    PDT_Reg_SetTriggerDelayValue(Base, Chn, Value);
}

/* Implementation of Pdt_Hal_ConfigTriggerDelay declared in Pdt_Hal.h */
void Pdt_Hal_ConfigTriggerDelay(const uint8 Instance, const Pdt_TriggerDelayConfigType *DelayConfig)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    /* PRQA S 2842,2812 ++ # AMCE_GPT_2842 */
    DEVICE_ASSERT(DelayConfig != NULL_PTR);
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_UNINITED);

    PDT_Type *const Base = PdtBase[Instance];         /* PRQA S 2842 # AMCE_GPT_2842 */
    uint8 i = 0U;

    for (i = 0U; i < PDT_DLY_MAX; i++)
    {
        PDT_Reg_SetTriggerDelayValue(Base, i, DelayConfig->Dly[i]);
    }
    PDT_Reg_SetTriggerDelayEnable(Base, DelayConfig->DelayEnable);
    PDT_Reg_SetTriggerDelayBypass(Base, DelayConfig->TriggerDelayBypassEn);
    /* PRQA S 2842,2812 -- # AMCE_GPT_2842 */
}

/* Implementation of Pdt_Hal_GetStatus declared in Pdt_Hal.h */
Hal_StatusType Pdt_Hal_GetStatus(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);

    Hal_StatusType Status = STATUS_IDLE;
    /* PRQA S 2842,2812 ++ # AMCE_GPT_2842 */
    if (PDT_RUNNING == PdtStatus[Instance])
    {
        Status = STATUS_BUSY;
    }
    else if (PDT_IDLE == PdtStatus[Instance])
    {
        Status = STATUS_IDLE;
    }
    else
    {
        Status = STATUS_ERROR;
    }
    /* PRQA S 2842,2812 -- # AMCE_GPT_2842 */

    return Status;
}

/* Implementation of Pdt_Hal_EnableInterrupt declared in Pdt_Hal.h */
void Pdt_Hal_EnableInterrupt(const uint8 Instance, const boolean Enable)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);

    PDT_Type *const Base = PdtBase[Instance];    /* PRQA S 2842 # AMCE_GPT_2842 */

    PDT_Reg_SetInterruptEnable(Base, Enable);
}

/* Implementation of Pdt_Hal_ConfigPulseOut declared in Pdt_Hal.h */
void Pdt_Hal_ConfigPulseOut(const uint8 Instance, const Pdt_PulseOutConfigType *Config)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    /* PRQA S 2842,2812 ++ # AMCE_GPT_2842 */
    DEVICE_ASSERT(PdtStatus[Instance] != PDT_UNINITED);

    PDT_Type *const Base = PdtBase[Instance];

    WRITE_REG32(Base->PULSE.PODLY, (((uint32)Config->Poly2 << PDT_PODLY_PODLY2_Pos) | Config->Poly1));
    PDT_Reg_SetCmpPulseOutEnable(Base, Config->PulseOutEnable);
    /* PRQA S 2842,2812 -- AMCE_GPT_2842*/
}

/* Implementation of Pdt_Hal_InstallCallback declared in Pdt_Hal.h */
void Pdt_Hal_InstallCallback(const uint8 Instance, const Hal_CallbackType Func)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);

    PdtCallback[Instance] = Func;   /* PRQA S 2842 # AMCE_GPT_2842 */
}

/**
 * @brief PDT interrupt service routine handler.
 *
 * @details
 * Common interrupt handler for PDT timer interrupts. Checks the interrupt flag,
 * clears it, and invokes the registered callback function if one is installed.
 *
 * @param[in] Instance: PDT instance number (0 or 1)
 *
 * @return void
 *
 * @pre
 *     Must be called from the PDT interrupt context (ISR).
 *     A callback function should be installed via Pdt_Hal_InstallCallback().
 *
 * @post
 *     Interrupt flag is cleared and callback is executed if installed.
 *
 * @note
 *     This function is typically called by specific instance ISRs (PDT0_IRQHandler, PDT1_IRQHandler).
 *     DES_PDT_API_222
 */
static void Pdt_Hal_IRQHandler(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PDT_INSTANCE_MAX);

    uint32 IntFlag = 0U;

    PDT_Type * const Base = PdtBase[Instance];  /* PRQA S 2842 # AMCE_GPT_2842 */
    /* store device status */
    IntFlag = PDT_Reg_GetTimerIntFlag(Base);

    if (1U == IntFlag)
    {
        /* clear device status*/
        PDT_Reg_ClearTimerIntFlag(Base);
        if (NULL_PTR != PdtCallback[Instance])
        {
            PdtCallback[Instance]((void *)((uint32)Instance));  /* PRQA S 0326 # AMCE_GPT_0326 */
        }
    }
}

ISR(PDT0_IRQHandler)
{
    Pdt_Hal_IRQHandler(0U);
}

ISR(PDT1_IRQHandler)
{
    Pdt_Hal_IRQHandler(1U);
}
/* =============================================  EOF  ============================================== */
