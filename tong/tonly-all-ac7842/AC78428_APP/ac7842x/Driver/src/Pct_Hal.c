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
 * AutoChips Inc. (C) 2025. All rights reserved.
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
 * @file pct_drv.c
 *
 * @brief This file provides pct integration functions interface.
 *
 */

/* ===========================================  Includes  =========================================== */
#include "Pct_Hal.h"
#include "Ckgen_Hal.h"
#include "Core_Hal.h"
#include "Rtc_Hal.h"
/* ============================================  Define  ============================================ */
#define PCT_MAX_CMR_NTICKS                     (PCT_CMR_CMR_Msk + 1UL)

/* ===========================================  Typedef  ============================================ */
/*!
 * @brief PCT state type
 */
typedef enum
{
    PCT_UNINITED = 0x0U, /**< Un initialization status. */
    PCT_IDLE, /**< PCT has been initialized and is idle */
    PCT_RUNNING, /**< PCT is running.*/
} Pct_StateType;
/* ==========================================  Variables  =========================================== */
/* Table of Base addresses for PCT Instances */
static PCT_Type* const PctBase[PCT_INSTANCE_MAX] = PCT_BASE_PTRS;

/* pct callback pointer */
static Hal_CallbackType PctCallback= NULL_PTR;

/** @brief Variable of PCT state.*/
static Pct_StateType PctStatus = PCT_UNINITED;
/* ====================================  Functions declaration  ===================================== */

/* =====================================  Functions definition  ===================================== */
/*!
 * @brief Gets the prescaler coefficient value.
 *
 * @details
 * Calculates the effective prescaler divider based on the prescaler register
 * setting and bypass flag.
 *
 * @param[in] Base: Pointer to PCT hardware base address
 *
 * @return uint8: Prescaler coefficient value
 *
 * @note Internal helper function (static)
 */
LOCAL_INLINE uint8 PCT_Hal_GetPrescaler(const PCT_Type* const Base)
{
    uint8 PreVal = 0U;
    uint8 Bypass = 0U;

    Bypass = PCT_Reg_GetBypass(Base);
    if (0U == Bypass)
    {
        PreVal = PCT_Reg_GetPrescaler(Base);
        PreVal += 1U;
    }

    return PreVal;
}

/**
 * @brief Gets the PCT working frequency.
 *
 * @details
 * Determines the effective working frequency based on the selected clock source
 * and system clock configuration.
 *
 * @param[in] Base: Pointer to PCT hardware base address
 *
 * @return uint32: PCT working frequency in Hz
 *
 * @note Internal helper function (static)
 */
static uint32 Pct_Hal_GetWorkFreq(const PCT_Type* const Base)
{
    uint32 Freq = 0U;
    uint8 Sel;
    Ckgen_ClkIdType const ClkReg2CkgenId[PCT_CLOCK_MAX] = {CKGEN_HSI_DIV2_CLK,
                                                      CKGEN_LSI_1K_CLK,
                                                      (Ckgen_ClkIdType)0U,    /* PRQA S 4342 # AMCE_GPT_4342*/
                                                      CKGEN_PCT_CLK};

    /*PCT has been initialized.*/
    if (PCT_UNINITED != PctStatus)
    {
        Sel = PCT_Reg_GetClockSelect(Base);
        if (PCT_CLOCKSOURCE_RTC == (Pct_ClkSrcType)Sel)  /* PRQA S 4342 # AMCE_GPT_4342*/
        {
            Freq = Rtc_Hal_GetWorkFreq();
        }
        else
        {
            (void)Ckgen_Hal_GetFreq(ClkReg2CkgenId[Sel], &Freq);
        }
    }

    return Freq;
}

/**
 * @brief Converts PCT counter ticks to microseconds.
 *
 * @details
 * Calculates the equivalent time in microseconds from a counter tick count,
 * accounting for clock frequency and prescaler settings.
 *
 * @param[in] Base: Pointer to PCT hardware base address
 * @param[in] Count: Counter tick value to convert
 * @param[out] Us: Pointer to store the result in microseconds
 *
 * @return Hal_StatusType
 *     - STATUS_SUCCESS: Conversion successful
 *     - STATUS_ERROR: Conversion resulted in overflow
 *
 * @note Internal helper function (static)
 */
static Hal_StatusType PCT_Hal_CountToUs(const PCT_Type* const Base, const uint16 Count, uint32* const Us)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint8 PrescVal;
    uint64 Counts;
    uint64 UsReal = 0U;
    uint32 Freq = 0U;

    Freq = Pct_Hal_GetWorkFreq(Base);
    if (Freq != 0U)
    {
        PrescVal = PCT_Hal_GetPrescaler(Base);
        Counts = ((uint64)Count + 1U) << PrescVal;
        UsReal = Counts * 1000000U / Freq;
    }

    if (UsReal <= 0xFFFFFFFFU)
    {
        *Us = (uint32)UsReal;
    }
    else
    {
        *Us = 0xFFFFFFFFU;
        Ret = STATUS_ERROR;
    }

    return Ret;
}

/**
 * @brief Converts microseconds to PCT counter ticks.
 *
 * @details
 * Calculates the equivalent counter tick count from a time value in microseconds,
 * accounting for clock frequency and prescaler settings. Includes rounding for accuracy.
 *
 * @param[in] Base: Pointer to PCT hardware base address
 * @param[in] Us: Time value in microseconds to convert
 * @param[out] Count: Pointer to store the result in ticks
 *
 * @return Hal_StatusType
 *     - STATUS_SUCCESS: Conversion successful
 *     - STATUS_ERROR: Conversion resulted in overflow or invalid configuration
 *
 * @note Internal helper function (static)
 */
static Hal_StatusType PCT_Hal_UsToCount(const PCT_Type* const Base, const uint32 Us, uint16* const Count)
{
    uint8 PrescVal = 0U;
    uint64 CountsNum = 0U;
    uint64 NTicks = 0U;
    uint64 PrescCoff;
    uint32 Freq = 0U;
    Hal_StatusType Ret = STATUS_SUCCESS;

    Freq = Pct_Hal_GetWorkFreq(Base);
    if (Freq != 0U)
    {
        PrescVal = PCT_Hal_GetPrescaler(Base);
        /* get prescaler cofficient */
        PrescCoff = ((uint64)1U << PrescVal);
        /* Rounding the countsNum value for better accuracy */
        CountsNum =(((uint64)Freq * (uint64)Us) + 500000U) / 1000000U;
        /* Rounding the NTicks value for better accuracy */
        NTicks = ((CountsNum + (PrescCoff >> 1U)) / PrescCoff);
    }

    if (NTicks > PCT_MAX_CMR_NTICKS)
    {
        *Count = PCT_CMR_CMR_Msk;
        Ret = STATUS_ERROR;
    }
    else if (NTicks == 0U)
    {
        *Count = 0U;
        Ret = STATUS_ERROR;
    }
    else
    {
        *Count = (uint16)(NTicks - 1U);
    }

    return Ret;
}

/* Implementation of Pct_Hal_InstallCallback declared in Pct_Hal.h */
void Pct_Hal_InstallCallback(const Hal_CallbackType Func)   //PRQA S 1505 # AMCE_GPT_1505
{
    PctCallback = Func;
}

/* Implementation of Pct_Hal_SetConfig declared in Pct_Hal.h */
Hal_StatusType Pct_Hal_SetConfig(const uint8 Instance, const Pct_ConfigType *const Config)    //PRQA S 1505 # AMCE_GPT_1505
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);
    DEVICE_ASSERT(Config->CVALSel <= PCT_BCLK_FCLK_1);   /* PRQA S 2812 # AMCE_GPT_2812 */
    DEVICE_ASSERT(Config->Mode <= PCT_MODE_PULSECOUNTER);
    DEVICE_ASSERT(Config->ClkSrc < PCT_CLOCK_MAX);
    DEVICE_ASSERT(Config->PreScl <= PCT_PRESCALE_65536_GLITCHFILTER_32768);
    DEVICE_ASSERT(Config->PinSel <= PCT_PINSELECT_ALT3);
    DEVICE_ASSERT(Config->PinPolarity <= PCT_PINPOLARITY_FALLING);

    DEVICE_ASSERT(PCT_RUNNING != PctStatus);

    uint32 Pct_Csr = 0U;
    uint32 Pct_Psr = 0U;
    uint16 Counts = 0U;
    Hal_StatusType Status = STATUS_SUCCESS;
    PCT_Type* const Base = PctBase[Instance];    /* PRQA S 2842 # AMCE_GPT_2842 */

    /*free run mode does not support prescaler*/
    DEVICE_ASSERT(!((Config->FreeRun == TRUE) && (Config->BypassPrescaler == FALSE)));

    if (PCT_MODE_TIMER == Config->Mode)
    {
        Pct_Psr = ((uint32)Config->CVALSel << PCT_PSR_CVAL_SEL_Pos);
    }
    else
    {
        DEVICE_ASSERT(PCT_COUNTER_UNITS_TICKS == Config->CounterUnits);

        Pct_Csr = (((uint32)Config->PinSel << PCT_CSR_IPS_Pos) \
                | ((uint32)Config->PinPolarity << PCT_CSR_PPS_Pos));
        /*pulse mode, cval must set PCT_BCLK_FCLK_4 */
        Pct_Psr = ((uint32)PCT_BCLK_FCLK_4 << PCT_PSR_CVAL_SEL_Pos);
    }
    Pct_Csr |= (((uint32)Config->FreeRun << PCT_CSR_FRE_Pos) \
            | ((uint32)Config->Mode << PCT_CSR_MS_Pos) \
            | ((uint32)Config->InterruptEnable << PCT_CSR_CIE_Pos));
    /* PRQA S 2985 ++  # AMCE_GPT_2985 */
    Pct_Psr |= (((uint32)Config->ClkSrc << PCT_PSR_PCS_Pos) \
            | ((uint32)Config->BypassPrescaler << PCT_PSR_BYP_Pos) \
            | ((uint32)Config->PreScl << PCT_PSR_PS_VAL_Pos));
    /* PRQA S 2985 --  # AMCE_GPT_2985 */
    /*set to register, for PCT_Hal_UsToCount can get the register value */
    WRITE_REG32(Base->CSR, Pct_Csr);
    WRITE_REG32(Base->PSR, Pct_Psr);

    if (PCT_COUNTER_UNITS_MICROSECONDS == Config->CounterUnits)
    {
        Status = PCT_Hal_UsToCount(Base, Config->CompareValue, &Counts);
        DEVICE_ASSERT(STATUS_SUCCESS == Status);

        PCT_Reg_SetCompareValue(Base, Counts);
    }
    else
    {
        PCT_Reg_SetCompareValue(Base, (uint16)Config->CompareValue);
    }

    if (Config->Callback != NULL_PTR)
    {
        Pct_Hal_InstallCallback(Config->Callback);
    }

    if (Config->InterruptEnable == TRUE)
    {
        Core_Hal_EnableIrq(PCT_IRQn);
    }
    else
    {
        Core_Hal_DisableIrq(PCT_IRQn);
    }

    return Status;
}

/* Implementation of Pct_Hal_Init declared in Pct_Hal.h */
void Pct_Hal_Init(const uint8 Instance, const Pct_ConfigType *const Config)
{
    DEVICE_ASSERT(Config != NULL_PTR);

    if (PCT_UNINITED == PctStatus)
    {
        /* Enable PCT clock */
        (void)Ckgen_Hal_EnablePeriphClk(CKGEN_PCT_BUS_CLK, TRUE);

        (void)Pct_Hal_SetConfig(Instance, Config);
        PctStatus = PCT_IDLE;
    }
}

/* Implementation of Pct_Hal_Deinit declared in Pct_Hal.h */
void Pct_Hal_Deinit(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);

    if (PCT_IDLE == PctStatus)
    {
        /* Disable PCT clock */
        Core_Hal_DisableIrq(PCT_IRQn);
        (void)Ckgen_Hal_EnablePeriphClk(CKGEN_PCT_BUS_CLK, FALSE);
        PctStatus = PCT_UNINITED;
    }
}

boolean Pct_Hal_GetCompareFlag(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);

    const PCT_Type * const Base = PctBase[Instance];   /* PRQA S 2842 # AMCE_GPT_2842 */

    return PCT_Reg_GetCompareFlag(Base);
}

void Pct_Hal_ClearCompareFlag(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);

    PCT_Type * const  Base = PctBase[Instance];  /* PRQA S 2842 # AMCE_GPT_2842*/

    PCT_Reg_ClearCompareFlag(Base);
}

/* Implementation of Pct_Hal_SetCompareValueByCount declared in Pct_Hal.h */
void Pct_Hal_SetCompareValueByCount(const uint8 Instance, const uint16 Value)
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);

    PCT_Type* const Base = PctBase[Instance]; /* PRQA S 2842 # AMCE_GPT_2842*/

    if (PCT_IDLE == PctStatus)
    {
        PCT_Reg_SetCompareValue(Base, Value);
    }
}

/* Implementation of Pct_Hal_GetCompareValueByCount declared in Pct_Hal.h */
void Pct_Hal_GetCompareValueByCount(const uint8 Instance, uint16 *const Value)
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);
    DEVICE_ASSERT(Value != NULL_PTR);

    const PCT_Type* const Base = PctBase[Instance]; /* PRQA S 2842 # AMCE_GPT_2842 */

    *Value = PCT_Reg_GetCompareValue(Base);    /* PRQA S 2812 # AMCE_GPT_2812 */
}

/* Implementation of Pct_Hal_SetCompareValueByUs declared in Pct_Hal.h */
Hal_StatusType Pct_Hal_SetCompareValueByUs(const uint8 Instance, const uint32 Value)
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);

    Hal_StatusType Ret   = STATUS_SUCCESS;
    PCT_Type* const Base  = PctBase[Instance];     /* PRQA S 2842 # AMCE_GPT_2842*/
    uint16 CmpVal = 0U;

    DEVICE_ASSERT(PCT_MODE_TIMER == (Pct_ModeType)PCT_Reg_GetWorkMode(Base)); /* PRQA S 4342 # AMCE_GPT_4342 */

    if (PCT_IDLE == PctStatus)
    {
        Ret = PCT_Hal_UsToCount(Base, Value, &CmpVal);
        DEVICE_ASSERT(STATUS_SUCCESS == Ret);

        PCT_Reg_SetCompareValue(Base, CmpVal);
    }
    return Ret;
}

/* Implementation of Pct_Hal_GetCompareValueByUs declared in Pct_Hal.h */
void Pct_Hal_GetCompareValueByUs(const uint8 Instance, uint32 *const Value)
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);
    DEVICE_ASSERT(Value != NULL_PTR);

    const PCT_Type* const Base = PctBase[Instance];    /* PRQA S 2842 # AMCE_GPT_2842*/
    Hal_StatusType Ret;
    uint16 CmpValue;

    DEVICE_ASSERT(PCT_MODE_TIMER == (Pct_ModeType)PCT_Reg_GetWorkMode(Base));  /* PRQA S 4342 # AMCE_GPT_4342*/

    CmpValue = PCT_Reg_GetCompareValue(Base);

    Ret = PCT_Hal_CountToUs(Base, CmpValue, Value);
    DEVICE_ASSERT(STATUS_SUCCESS == Ret);
}

/* Implementation of Pct_Hal_SetInterrupt declared in Pct_Hal.h */
void Pct_Hal_SetInterrupt(const uint8 Instance, const boolean Enable)
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);

    PCT_Type* const Base = PctBase[Instance];    /* PRQA S 2842 # AMCE_GPT_2842*/

    PCT_Reg_SetInterrupt(Base, Enable);
}

/* Implementation of Pct_Hal_GetCurrentValue declared in Pct_Hal.h */
uint16 Pct_Hal_GetCurrentValue(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);

    PCT_Type const*  Base = PctBase[Instance];   /* PRQA S 2842 # AMCE_GPT_2842*/
    uint16 CounterVal = PCT_Reg_GetCounterValue(Base);

    return CounterVal;
}

Hal_StatusType Pct_Hal_GetStatus(void)
{
    Hal_StatusType Status = STATUS_IDLE;

    if (PCT_RUNNING == PctStatus)
    {
        Status = STATUS_BUSY;
    }
    else if (PCT_IDLE == PctStatus)
    {
        Status = STATUS_IDLE;
    }
    else
    {
        Status = STATUS_ERROR;
    }

    return Status;
}

/* Implementation of Pct_Hal_Start declared in Pct_Hal.h */
void Pct_Hal_Start(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);
    DEVICE_ASSERT(PCT_IDLE == PctStatus);
    PCT_Type* const Base = PctBase[Instance];   /* PRQA S 2842 # AMCE_GPT_2842*/

    if (PCT_IDLE == PctStatus)
    {
        PCT_Reg_SetEnable(Base, TRUE);
        PctStatus = PCT_RUNNING;
    }
}

/* Implementation of Pct_Hal_Stop declared in Pct_Hal.h */
void Pct_Hal_Stop(const uint8 Instance)
{
    DEVICE_ASSERT(Instance < PCT_INSTANCE_MAX);
    DEVICE_ASSERT(PCT_RUNNING == PctStatus);
    PCT_Type* const Base = PctBase[Instance];   /* PRQA S 2842 # AMCE_GPT_2842*/

    if (PCT_RUNNING == PctStatus)
    {
        PCT_Reg_SetEnable(Base, FALSE);
        PctStatus = PCT_IDLE;
    }
}

/*!
 * @brief PCT common interrupt service routine.
 *
 * @return none
 */
ISR(PCT_IRQHandler)
{
    boolean CmpFlag = 0U;

    PCT_Type* const Base = PctBase[0U];
    CmpFlag = PCT_Reg_GetCompareFlag(Base);

    if (TRUE == CmpFlag)
    {
        /* clear timeout status */
        PCT_Reg_ClearCompareFlag(Base);
        if (NULL_PTR != PctCallback)
        {
            PctCallback(NULL_PTR);
        }
    }
}
/* =============================================  EOF  ============================================== */
