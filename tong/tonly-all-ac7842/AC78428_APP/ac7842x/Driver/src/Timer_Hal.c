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
 * @file Timer_Hal.c
 *
 * @brief Timer HAL source file.
 */
/*==============================================INCLUDE FILES=======================================*/
#include "Timer_Hal.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Ctu_Hal.h"
#include "Core_Hal.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/** @brief Register value for timer mode 0.
 *  @note DES ID: DES_GPT_MACRO_520
 */
#define TIMER_CHANNEL_CTRL_MODE_0       (0x00UL)

/** @brief Register value for timer mode 1.
 *  @note DES ID: DES_GPT_MACRO_521
 */
#define TIMER_CHANNEL_CTRL_MODE_1       (0x10UL)

/** @brief Register value for timer mode 2
 *  @note DES ID: DES_GPT_MACRO_522
 */
#define TIMER_CHANNEL_CTRL_MODE_2       (0x20UL)

/** @brief Register value for timer mode 3
 *  @note DES ID: DES_GPT_MACRO_523
 */
#define TIMER_CHANNEL_CTRL_MODE_3       (0x30UL)

/** @brief Initialized flag of Timer HAL
 *  @note DES ID: DES_GPT_MACRO_524
 */
#define TIMER_STATUS_INITED 0x100U

/** @brief Started flag of timer 0
 *  @note DES ID: DES_GPT_MACRO_525
 */
#define TIMER_STATUS_CH0_STARTED  0x01UL

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/** @brief Timer status variable.
 *  @note DES ID: DES_GPT_VAR_501
 */
static uint32 TimerStatus = 0U;

/** @brief Callback pointer array.
 *  @note DES ID: DES_GPT_VAR_502
 */
static Hal_CallbackType TimerChannel_Callback[TIMER_CHANNEL_MAX] = {NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR};

/** @brief parameters array for callback function.
 *  @note DES ID: DES_GPT_VAR_503
 */
static void *CbArgs[TIMER_CHANNEL_MAX] = {NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR};

/** @brief Configuration array of timers.
 *  @note DES_GPT_VAR_504
 */
static Timer_Channel_ConfigType TimerCfg[TIMER_CHANNEL_MAX] =
{
    {
        .Mode = TIMER_MODE_0,
        .Config = TIMER_IRQ_EN,
        .TriggerSrc = (uint32)TRIG_SOURCE_TIMER_CH0
    },
    {
        .Mode = TIMER_MODE_0,
        .Config = TIMER_IRQ_EN,
        .TriggerSrc = (uint32)TRIG_SOURCE_TIMER_CH0
    },
    {
        .Mode = TIMER_MODE_0,
        .Config = TIMER_IRQ_EN,
        .TriggerSrc = (uint32)TRIG_SOURCE_TIMER_CH0
    },
    {
        .Mode = TIMER_MODE_0,
        .Config = TIMER_IRQ_EN,
        .TriggerSrc = (uint32)TRIG_SOURCE_TIMER_CH0
    },
};

/*=============================FUNCTION PROTOTYPES==================================*/
/**
* @brief Set interrupt  hardware base on interrupt configuration.
* @note Function ID: DES_GPT_API_524
* @param [in]  Channel: Timer channel index.
*@param [in] Config: Configuration bits of timer interrupt.
* @return void
*/
static void Timer_Hal_ApplyIntConfig(uint32 Channel, uint32 Config);

/**
 * @brief  Timer channel interrupt handler
 *
 * @details This function is the common interrupt service routine (ISR) for all timer channels.
 *          It checks if an interrupt has occurred on the specified channel by reading the status register (SR).
 *          If an interrupt flag is set, it clears the flag and calls the user-installed callback function with its argument.
 *
 * @param[in] Channel : Timer channel index (0-3) that triggered the interrupt.
 *
 * @return void
 *
 * @pre   An interrupt has been triggered on the specified timer channel.
 * @post  The interrupt flag is cleared and the user callback function is executed (if installed).
 *
 * @note Function ID: DES_GPT_API_525
 * @note Service ID: none
 */
static void Timer_Hal_IrqHandler(uint32 Channel);

/*=========================GLOBAL FUNCTION  IMPLEMENTATIONS==========================*/
/* Implementation of Timer_Hal_Init declared in Timer_Hal.h */
Hal_StatusType Timer_Hal_Init(Timer_ClockSourceType Clk)
{
    /* PRQA S 4332  ++ # AMCE_QAC_4332*/
    Hal_StatusType Res = (Hal_StatusType)(STATUS_TIMER_WRONG_STATE);
    /* PRQA S 4332  -- # AMCE_QAC_4332*/
    /* Timer is un-initialization.*/
    if (0U == TimerStatus)
    {
        /* Enable timer bus clock.*/
        Res = Ckgen_Hal_EnablePeriphClk(CKGEN_TIMER_BUS_CLK, TRUE);

        /*Timer clock is valid.*/
        if (TIMER_CLOCK_INVALID != Clk)
        {
            const Ckgen_ClkIdType ClkId2CkgenId[4U] = {CKGEN_SPLL_DIV2_CLK, CKGEN_VHSI_DIV2_CLK,
                                        CKGEN_HSI_DIV2_CLK, CKGEN_HSE_DIV2_CLK
                                       };
            Res = Ckgen_Hal_SetPeriphClkMux(CKGEN_TIMER_CLK, ClkId2CkgenId[Clk]);
        }
        Rcm_Hal_SetResetState(RCM_RESET_ID_TIMER, RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(RCM_RESET_ID_TIMER, RCM_RESET_STATE_DEASSERT);

        Timer_Reg_WriteCR(TIMER_CTRL_CR_MC_EN_Msk);
        TimerStatus = TIMER_STATUS_INITED;
    }
    return Res;
}

/* Implementation of Timer_Hal_DeInit declared in Timer_Hal.h */
Hal_StatusType Timer_Hal_DeInit(void)
{
    /* PRQA S 4332  ++ # AMCE_QAC_4332*/
    Hal_StatusType Res = (Hal_StatusType)(STATUS_TIMER_WRONG_STATE);
    /* PRQA S 4332  -- # AMCE_QAC_4332*/
    /* Timer is initialized and no timer is started.*/
    if (TIMER_STATUS_INITED == TimerStatus)
    {
        Timer_Reg_WriteCR(0U);
        /* Enable Timer bus clock. */
        Res = Ckgen_Hal_EnablePeriphClk(CKGEN_TIMER_BUS_CLK, FALSE);
        Rcm_Hal_SetResetState(RCM_RESET_ID_TIMER, RCM_RESET_STATE_ASSERT);
        TimerStatus = 0U;
    }
    return Res;
}

#ifndef WDG_SDK_NON_EXTENDED_API
/* Implementation of Timer_Hal_SetConfig declared in Timer_Hal.h */
Hal_StatusType Timer_Hal_SetConfig(uint32 Channel, const Timer_Channel_ConfigType *ConfigPtr)
{
    /* PRQA S 4332  ++ # AMCE_QAC_4332*/
    Hal_StatusType Res = (Hal_StatusType)(STATUS_TIMER_WRONG_PARAM);
    /* PRQA S 4332  -- # AMCE_QAC_4332*/
    /* Channel is valid and ConfigPtr is valid.*/
    if ((Channel < TIMER_CHANNEL_MAX) && (NULL_PTR != ConfigPtr))
    {
        TimerCfg[Channel] = *ConfigPtr;
        Res = STATUS_SUCCESS;
    }
    return Res;
}

/* Implementation of Timer_Hal_GetConfig declared in Timer_Hal.h */
Hal_StatusType Timer_Hal_GetConfig(uint32 Channel, Timer_Channel_ConfigType *ConfigPtr)
{
    /* PRQA S 4332  ++ # AMCE_QAC_4332*/
    Hal_StatusType Res = (Hal_StatusType)(STATUS_TIMER_WRONG_PARAM);
    /* PRQA S 4332  -- # AMCE_QAC_4332*/
    /* Channel is valid and ConfigPtr is valid.*/
    if ((Channel < TIMER_CHANNEL_MAX) && (NULL_PTR != ConfigPtr))
    {
        *ConfigPtr = TimerCfg[Channel] ;
        Res = STATUS_SUCCESS;
    }
    return Res;
}
#endif

/* Implementation of Timer_Hal_Start declared in Timer_Hal.h */
Hal_StatusType Timer_Hal_Start(uint32 Channel, uint32 Timeout)
{
    Hal_StatusType Res = STATUS_SUCCESS;
    /* Channel is out of range.*/
    if (Channel >= TIMER_CHANNEL_MAX)
    {
        /* PRQA S 4332  ++ # AMCE_QAC_4332*/
        Res = (Hal_StatusType)(STATUS_TIMER_WRONG_PARAM);
        /* PRQA S 4332  -- # AMCE_QAC_4332*/
    }
    else if ((TIMER_STATUS_INITED != (TimerStatus & TIMER_STATUS_INITED))
             || (0U != (TimerStatus & (TIMER_STATUS_CH0_STARTED << Channel))))
        /* Invalid states(Timer isn't initialized or timer channel has been started.*/
    {
        /* PRQA S 4332  ++ # AMCE_QAC_4332*/
        Res = (Hal_StatusType)(STATUS_TIMER_WRONG_STATE);
        /* PRQA S 4332 --  # AMCE_QAC_4332*/
    }
    else /* Channel is valid and state is valid. */
    {
        TIMER_CHANNEL_Type *TimerBaseAddrPtr;
        const Timer_Channel_ConfigType *ConfigPtr = &TimerCfg[Channel];
        uint32 Value = 0U;
        uint32 FuncClkFreq = 0U;
        uint32 BusClkFreq = 0U;

        /* Set timeout value for timer*/
        TimerBaseAddrPtr = Timer_Reg_GetChBase(Channel);
        Timer_Reg_WriteTVAL(TimerBaseAddrPtr, Timeout);

        /* Calculate value of control register. */
        Ckgen_Hal_GetFreq(CKGEN_TIMER_CLK, & FuncClkFreq);
        Ckgen_Hal_GetFreq(CKGEN_BUS_CLK, & BusClkFreq);

        /* Calculate work mode. */
        switch (ConfigPtr->Mode)
        {
        case TIMER_MODE_1:
            Value |= TIMER_CHANNEL_CTRL_MODE_1;
            break;
        case TIMER_MODE_2:
            Value |= TIMER_CHANNEL_CTRL_MODE_2;
            break;
        case TIMER_MODE_3:
            Value |= TIMER_CHANNEL_CTRL_MODE_3;
            break;
        case TIMER_MODE_0:
        default:
            Value |= TIMER_CHANNEL_CTRL_MODE_0;
            break;
        }

        /* Set CVAL_SEL based on BUSCLK/Function clk ratio.
         * 00b: BUSCLK/Function clk >= 4
         * 01b: BUSCLK/Function clk >= 3
         * 10b: BUSCLK/Function clk >= 1
         * Pulse accumulator mode (TIMER_MODE_2) must set CVAL_SEL to 00.
         */
        if (TIMER_MODE_2 == ConfigPtr->Mode)
        {
            Value |= (0x00U << TIMER_CHANNEL_CTRL_CVAL_SEL_Pos);
        }
        else
        {
            if ((FuncClkFreq > 0U) && (BusClkFreq / FuncClkFreq >= 4U))
            {
                Value |= (0x00U << TIMER_CHANNEL_CTRL_CVAL_SEL_Pos);
            }
            else if ((FuncClkFreq > 0U) && (BusClkFreq / FuncClkFreq >= 3U))
            {
                Value |= (0x01U << TIMER_CHANNEL_CTRL_CVAL_SEL_Pos);
            }
            else
            {
                Value |= (0x02U << TIMER_CHANNEL_CTRL_CVAL_SEL_Pos);
            }
        }

        /* One shot mode*/
        if (TIMER_ONESHOT_EN == (ConfigPtr->Config & TIMER_ONESHOT_EN))
        {
            /* Set  TSOI bit */
            Value |= TIMER_CHANNEL_CTRL_TSOI_Msk;
        }
        /* TROT is set.*/
        if (TIMER_TROT == (ConfigPtr->Config & TIMER_TROT))
        {
            /* Set  TROT bit */
            Value |= TIMER_CHANNEL_CTRL_TROT_Msk;
        }
        /* TsOT is set.*/
        if (TIMER_TSOT == (ConfigPtr->Config & TIMER_TSOT))
        {
            /* Set  TSOT bit */
            Value |= TIMER_CHANNEL_CTRL_TSOT_Msk;
        }
        /* Link mode is enabled.*/
        if (TIMER_CHN_EN == (ConfigPtr->Config & TIMER_CHN_EN))
        {
            /* Set chain enable bit. */
            Value |= TIMER_CHANNEL_CTRL_CHN_EN_Msk;
        }
        /* Internal trigger source..*/
        if ((ConfigPtr->TriggerSrc >= (uint32) TRIG_SOURCE_TIMER_CH0)
                && (ConfigPtr->TriggerSrc <= (uint32) TRIG_SOURCE_TIMER_CH3))
        {
            /* Set internal tringger source. */
            Value |= TIMER_CHANNEL_CTRL_TRG_SRC_Msk;
            Value |= (ConfigPtr->TriggerSrc - (uint32) TRIG_SOURCE_TIMER_CH0) <<
                     TIMER_CHANNEL_CTRL_TTRG_SEL_Pos;
        }
        else /* No internal trigger source.*/
        {
            /*Trigger source is 0U (disabled).*/
            if (0U != ConfigPtr->TriggerSrc)
            {
                /* PRQA S 4342  ++ # AMCE_QAC_4342*/
                /* PRQA S 4394  ++ # AMCE_QAC_4394*/
                (void)Ctu_Hal_SetModuleTriggerSource((Ctu_TargetModuleType)((uint32)TRIG_SEL_TIMER_CH0 + Channel),
                                               (Ctu_TriggerSourceType)ConfigPtr->TriggerSrc);
                 /* PRQA S 4342 --*/
                 /* PRQA S 4394 --*/
            }
        }
        /* Set control regiser of timer channel.*/
        Timer_Reg_WriteCTRL(TimerBaseAddrPtr, Value);

        Value = Timer_Reg_ReadCR();
        /* Enable working in debug halt mode.*/
        if (TIMER_DBG_EN == (ConfigPtr->Config & TIMER_DBG_EN))
        {
            Value |= TIMER_CTRL_CR_DBG_EN_Msk;
        }
        else
        {
            Value &= ~TIMER_CTRL_CR_DBG_EN_Msk;
        }
        Timer_Reg_WriteCR(Value);

        /* Apply interrupt setting.*/
        Timer_Hal_ApplyIntConfig(Channel, ConfigPtr->Config);

        /* Enable timer (channel)*/
        Value = Timer_Reg_ReadENR();

        Timer_Reg_WriteENR(Value & (~((uint32)TIMER_CTRL_ENR_TEN0_Msk << Channel))); /* Disable */
        Timer_Reg_WriteENR(Value | ((uint32)TIMER_CTRL_ENR_TEN0_Msk << Channel)); /* Enable */
        TimerStatus |= TIMER_STATUS_CH0_STARTED << Channel; /* Set start status for timer channel.*/
    }
    return Res;
}

/* Implementation of Timer_Hal_Stop declared in Timer_Hal.h */
Hal_StatusType Timer_Hal_Stop(uint32 Channel)
{
    Hal_StatusType Res = STATUS_SUCCESS;
    /* Channel is out of range..*/
    if (Channel >= TIMER_CHANNEL_MAX)
    {
        /* PRQA S 4332  ++ # AMCE_QAC_4332*/
        Res = (Hal_StatusType)(STATUS_TIMER_WRONG_PARAM);
        /* PRQA S 4332  -- # AMCE_QAC_4332*/
    }
    else if ((TIMER_STATUS_INITED != (TimerStatus & TIMER_STATUS_INITED))
             || (0U == (TimerStatus & (TIMER_STATUS_CH0_STARTED << Channel))))
        /* Invalid states(timer channel hasn't been started.*/
    {
        /* PRQA S 4332  ++ # AMCE_QAC_4332*/
        Res = (Hal_StatusType)(STATUS_TIMER_WRONG_STATE);
        /* PRQA S 4332  -- # AMCE_QAC_4332*/
    }
    else/* Channel is valid and state is valid. */
    {
        /* Stop timer channel.*/
        uint32 Value;
        Value = Timer_Reg_ReadENR();
        Timer_Reg_WriteENR(Value & (~((uint32)TIMER_CTRL_ENR_TEN0_Msk << Channel))); /* Disable */

        /* Disable interrupt. */
        Timer_Hal_ApplyIntConfig(Channel, 0U);
        TimerStatus &= ~(TIMER_STATUS_CH0_STARTED << Channel); /* Clear start state of timer channel*/
    }
    return Res;
}

/* Implementation of Timer_Hal_GetCurrentValue declared in Timer_Hal.h */
uint32 Timer_Hal_GetCurrentValue(uint32 Channel)
{
    uint32 CurrentValue;
    const TIMER_CHANNEL_Type *TimerBaseAddrPtr;
    DEVICE_ASSERT(Channel < TIMER_CHANNEL_MAX);

    TimerBaseAddrPtr = Timer_Reg_GetChBase(Channel);
    CurrentValue = Timer_Reg_ReadCVAL(TimerBaseAddrPtr);

    return CurrentValue;
}

#ifndef WDG_SDK_NON_EXTENDED_API
/* Implementation of Timer_Hal_GetRemainingValue declared in Timer_Hal.h */
uint32 Timer_Hal_GetRemainingValue(uint32 Channel)
{
    uint32 RemainingValue, TargetValue, CurrentValue;
    const TIMER_CHANNEL_Type *TimerBaseAddrPtr;

    TimerBaseAddrPtr = Timer_Reg_GetChBase(Channel);
    TargetValue = Timer_Reg_ReadTVAL(TimerBaseAddrPtr);
    CurrentValue = Timer_Reg_ReadCVAL(TimerBaseAddrPtr);
    RemainingValue = TargetValue - CurrentValue;

    return RemainingValue;
}
#endif

/* Implementation of Timer_Hal_EnableInterrupt declared in Timer_Hal.h */
void Timer_Hal_EnableInterrupt(uint32 Channel, uint32 InterruptBits)
{
    DEVICE_ASSERT(Channel < TIMER_CHANNEL_MAX);
    /* Channel is valid.*/
    if (Channel < TIMER_CHANNEL_MAX)
    {
        uint32 Config = 0U;
        /* Interrupt is enabled.*/
        if (TIMER_INT_EN == (TIMER_INT_EN & InterruptBits))
        {
            Config |= TIMER_IRQ_EN;
        }
        /*Interrupt config is changd.*/
        if (Config != (TimerCfg[Channel].Config & TIMER_IRQ_EN))
        {
            TimerCfg[Channel].Config &= ~TIMER_IRQ_EN;
            TimerCfg[Channel].Config |= Config;

            /* The timer has been started.*/
            if (0U != (TimerStatus & (TIMER_STATUS_CH0_STARTED << Channel)))
            {
                Timer_Hal_ApplyIntConfig(Channel, Config);
            }
        }
    }
}

/* Implementation of Timer_Hal_InstallCallback declared in Timer_Hal.h */
void Timer_Hal_InstallCallback(uint32 Channel, Hal_CallbackType Func, void *Args)
{
    if (Channel < TIMER_CHANNEL_MAX)
    {
        TimerChannel_Callback[Channel] = Func;
        CbArgs[Channel] = Args;
    }
}

#ifndef WDG_SDK_NON_EXTENDED_API
/* Implementation of Timer_Hal_ResetValue declared in Timer_Hal.h */
Hal_StatusType Timer_Hal_ResetValue(uint32 Channel)
{
    TIMER_CHANNEL_Type *TimerBaseAddrPtr;

    if (Channel < TIMER_CHANNEL_MAX)
    {
        TimerBaseAddrPtr = Timer_Reg_GetChBase(Channel);
        Timer_Reg_WriteCVAL(TimerBaseAddrPtr, 0U);
    }
    return STATUS_SUCCESS;
}
#endif

/* Implementation of Timer_Hal_MicrosToTicks declared in Timer_Hal.h */
uint32 Timer_Hal_MicrosToTicks(uint32 Micros)
{
    uint32 Value = 0U;
    Hal_StatusType Res = Ckgen_Hal_GetFreq(CKGEN_TIMER_CLK, & Value);
    if (STATUS_SUCCESS == Res)
    {
        Value /= 1000000U;
        Value *= Micros;
    }
    return Value;
}

/* Implementation of TIMER_Channel0_IRQHandler declared in Timer_Hal.h */
/* PRQA S 3408  ++ # AMCE_QAC_3408*/
void TIMER_Channel0_IRQHandler(void)
{
    Timer_Hal_IrqHandler(0U);
}

/* Implementation of TIMER_Channel1_IRQHandler declared in Timer_Hal.h */
void TIMER_Channel1_IRQHandler(void)
{
    Timer_Hal_IrqHandler(1U);
}

/* Implementation of TIMER_Channel2_IRQHandler declared in Timer_Hal.h */
void TIMER_Channel2_IRQHandler(void)
{
    Timer_Hal_IrqHandler(2U);
}

/* Implementation of TIMER_Channel3_IRQHandler declared in Timer_Hal.h */
void TIMER_Channel3_IRQHandler(void)
{
    Timer_Hal_IrqHandler(3U);
}
/* PRQA S 3408  -- # AMCE_QAC_3408*/

/*========================STATIC  FUNCTION  IMPLEMENTATIONS========================*/
static void Timer_Hal_ApplyIntConfig(uint32 Channel, uint32 Config)
{
    /* Clear IRQ status and disable/enable interrupt for timer channel.*/
    Timer_Reg_WriteSR((uint32)0x01U << Channel);

    uint32 Value = Timer_Reg_ReadIER();
    /* PRQA S 4342 ++ # AMCE_QAC_4342 */
    /* PRQA S 4394 ++# AMCE_QAC_4394*/
    IRQn_Type TimerIrq = (IRQn_Type)((uint32)TIMER_CHANNEL0_IRQn + Channel);
    /* Interrupt is enabled.*/
    if (TIMER_IRQ_EN == (Config & TIMER_IRQ_EN))
    {
        Core_Hal_ClearPendingIrq(TimerIrq);
        Core_Hal_EnableIrq(TimerIrq);
        Value |= (uint32)TIMER_CTRL_ENR_TEN0_Msk << Channel;
    }
    else /* Interrupt is disabled.*/
    {
        Value &= ~((uint32)TIMER_CTRL_ENR_TEN0_Msk << Channel);
        Core_Hal_DisableIrq(TimerIrq);
        Core_Hal_ClearPendingIrq(TimerIrq);
    }
    /* Set IRQ enabled regiser for timer channel.*/
    Timer_Reg_WriteIER(Value);
}

static void Timer_Hal_IrqHandler(uint32 Channel)
{
    uint32 IrqFlag = Timer_Reg_ReadSR();
    IrqFlag = (IrqFlag >> Channel) & 0x01U;

    if (0x0U != IrqFlag)
    {
        Timer_Reg_WriteSR((uint32)0x01U << Channel);

        if (NULL_PTR != TimerChannel_Callback[Channel])
        {
            TimerChannel_Callback[Channel](CbArgs[Channel]);
        }
    }
}
