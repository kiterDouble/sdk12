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

/* ===========================================  INCLUDE FILES  =========================================== */
#include "AC784xx_Pwm_Reg.h"
#include "Ckgen_Hal.h"
#include "Pwm_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"
/* =========================================== LOCAL VARIABLES ============================================== */
/**
 * @brief PWM overflow interrupt request info
 * @note DES ID: DES_PWM_CONST_101
 */
static const IRQn_Type PwmOverflowIrq[PWM_INSTANCE_MAX] = PWM_OVERFLOW_IRQS;

/**
 * @brief PWM channel interrupt request info
 * @note DES ID: DES_PWM_CONST_102
 */
static const IRQn_Type PwmChannelIrq[PWM_INSTANCE_MAX] = PWM_CHANNEL_IRQS;

/*PRQA S 3218 ++ # File scope static, 'PwmFaultIrq', is only accessed in one function.. */
/**
 * @brief PWM fault interrupt request info
 * @note DES ID: DES_PWM_CONST_103
 */
static const IRQn_Type PwmFaultIrq[PWM_INSTANCE_MAX] = PWM_FAULT_IRQS;

#if defined (AC7843X)
/**
 * @brief PWM detect interrupt request info
 * @note DES ID: DES_PWM_CONST_104
 */
static const IRQn_Type PwmDetectIrq[PWM_INSTANCE_MAX] = PWM_DETECT_IRQS;
#endif
/*PRQA S 3218 -- */

/* PWM base address */
#if defined (PWM_BASE_PTRS)
/**
 * @brief PWM base address
 * @note DES ID: DES_PWM_CONST_105
 */
static PWM_Type *const PwmBaseAddr[PWM_INSTANCE_MAX] = PWM_BASE_PTRS;
#else
/**
 * @brief PWM base address
 * @note DES ID: DES_PWM_CONST_105
 */
static PWM_Type *const PwmBaseAddr[PWM_INSTANCE_MAX] = {PWM0, PWM1, PWM2, PWM3, PWM4, PWM5};
#endif

/*PRQA S 0686 ++ # Array has fewer initializers than its declared size. Default initialization is applied to the remainder of the array elements. */
/**
 * @brief PWM worked in capture mode or not.
 * @note DES ID: DES_PWM_VAR_105
 */
static boolean PwmIsCaptureMode[PWM_INSTANCE_MAX] = {FALSE};

/**
 * @brief PWM overflow callback pointer
 * @note DES ID: DES_PWM_VAR_101
 */
static Pwm_Hal_Callback Pwm_OverflowCallback[PWM_INSTANCE_MAX] = {NULL_PTR};

/**
 * @brief PWM channel callback pointer
 * @note DES ID: DES_PWM_VAR_102
 */
static Pwm_Hal_Callback Pwm_ChannelCallback[PWM_INSTANCE_MAX] = {NULL_PTR};

/**
 * @brief PWM fault callback pointer
 * @note DES ID: DES_PWM_VAR_103
 */
static Pwm_Hal_Callback Pwm_FaultCallback[PWM_INSTANCE_MAX] = {NULL_PTR};

#if defined (AC7843X)
/**
 * @brief PWM quad phase z detect callback pointer
 * @note DES ID: DES_PWM_VAR_104
 */
static Pwm_Hal_Callback Pwm_ZDetectCallback[PWM_INSTANCE_MAX] = {NULL_PTR};
#endif
/*PRQA S 0686 -- */

/**
 * @brief PWM clock ids
 * @note DES ID: DES_PWM_VAR_106
 */
static const Ckgen_BusClkIdType Pwm_ClkId[PWM_INSTANCE_MAX] =
{
    CKGEN_PWM0_BUS_CLK
    , CKGEN_PWM1_BUS_CLK
    , CKGEN_PWM2_BUS_CLK
    , CKGEN_PWM3_BUS_CLK
#if (PWM_INSTANCE_MAX > 4U)
    , CKGEN_PWM4_BUS_CLK
#endif
#if (PWM_INSTANCE_MAX > 5U)
    , CKGEN_PWM5_BUS_CLK
#endif
#if (PWM_INSTANCE_MAX > 6U)
    , CKGEN_PWM6_BUS_CLK
#endif
#if (PWM_INSTANCE_MAX > 7U)
    , CKGEN_PWM7_BUS_CLK
#endif
};

#if defined (AC7840X)
/**
 * @brief PWM instances' clock enabled state.
 * @note DES ID: DES_PWM_VAR_107
 */
static uint16 Pwm_InstancesClkEnabledState = 0U;
#endif

/* =====================================  Functions definition  ===================================== */
/*PRQA S 1503 EOF # AMCE_QAC_NoUsedApi. */

/**
 * @brief Get combine channel 2nd channel type
 * @note DES ID: DES_PWM_API_271
 * @param[in] Channel: channel number
 * @return Pwm_Hal_ChannelType: Pair channel type
 */
static Pwm_Hal_ChannelType Pwm_Hal_GetPairChannel
(
    uint8 Channel
)
{
    /*return channel*/
    Pwm_Hal_ChannelType HalChannel = PWM_CHANNEL_1;

    /*matching channel*/
    switch (Channel)
    {
    case 1U:
        HalChannel = PWM_CHANNEL_1;
        break;
    case 3U:
        HalChannel = PWM_CHANNEL_3;
        break;
    case 5U:
        HalChannel = PWM_CHANNEL_5;
        break;
    case 7U:
        HalChannel = PWM_CHANNEL_7;
        break;
    default:
        /* Illegal param. */
        break;
    }

    return HalChannel;
}

/**
 * @brief Initialize PWM common attributes
 * @note DES ID: DES_PWM_API_272
 * @param[in] Instance: PWM instance
 * @param[in] Config: PWM common configuration pointer
 * @return void
 */
static void Pwm_Hal_InitCommonAttr
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_CommonCfg *Config
)
{
    /* Implementation of Pwm_Hal_InitCommonAttr declared in Pwm_Hal.h */
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];

    /* Set pwm module at reset state and enable bus clock. */
    (void)Ckgen_Hal_EnablePeriphClk(Pwm_ClkId[(uint8)Instance], TRUE);
    /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
    /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
    Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance), RCM_RESET_STATE_ASSERT);
    Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance), RCM_RESET_STATE_DEASSERT);
#if defined (AC7840X)
    Pwm_InstancesClkEnabledState |= ((uint16)1 << (uint8)Instance);
    /* Turning off the clocks of PWMn and PWMn-1 will cause PWMn to fail to turn on the clock (n is an odd number). */
    if (TRUE == ((uint8)Instance % 2U))
    {
        /* If PWM (Instance -1) is not in operation, turn on its clock. */
        if (0U == (Pwm_InstancesClkEnabledState & ((uint16)1 << ((uint8)Instance - 1U))))
        {
            /* Set pwm module at reset state and enable bus clock. */
            (void)Ckgen_Hal_EnablePeriphClk(Pwm_ClkId[(uint8)Instance - 1U], TRUE);
            Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance - 1U), \
                                  RCM_RESET_STATE_ASSERT);
            Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance - 1U), \
                                  RCM_RESET_STATE_DEASSERT);
        }
    }
#endif
    /*PRQA S 4342 -- */
    /*PRQA S 4394 -- */

    /* Set counter related register. */
    Pwm_Reg_ResetCounter(Base);
    Pwm_Reg_SetInitCountValue(Base, Config->MinCount);
    Pwm_Reg_SetMaxCountValue(Base, Config->MaxCount);
    Pwm_Reg_SetPeriodDither(Base, Config->PeriodDither);

    /* Set the number of overflow times to wait for the next CNTOF setting */
    Pwm_Reg_SetCntOverflowFreq(Base, Config->OverflowFreq);

    /* DMA Setting */
    Pwm_Reg_EnableOverflowDmaReq(Base, Config->EnableOverflowDmaReq);
    Pwm_Reg_EnableDmaTransfer(Base, Config->EnableDmaTransLen);
    Pwm_Reg_SetDmaTransferLen(Base, Config->DmaTransLen);

    Core_Hal_EnableIrq(PwmChannelIrq[(uint8)Instance]);
    if (TRUE == Config->EnableOverflowInterrupt)
    {
        Core_Hal_EnableIrq(PwmOverflowIrq[(uint8)Instance]);
    }
    else
    {
        Core_Hal_DisableIrq(PwmOverflowIrq[(uint8)Instance]);
    }
    Pwm_Reg_EnableOverflowInterrupt(Base, Config->EnableOverflowInterrupt);
    Pwm_Reg_EnableOverflowEvent(Base, Config->EnableOverflowEvent);
    Pwm_Reg_EnableUnderflowEvent(Base, Config->EnableUnderflowEvent);
    Pwm_OverflowCallback[(uint8)Instance] = Config->OverflowCallback;
    Pwm_ChannelCallback[(uint8)Instance] = Config->ChannelCallback;

    /*PRQA S 2812 -- */
}

/* Implementation of Pwm_Hal_InitOutputMode declared in Pwm_Hal.h */
void Pwm_Hal_InitOutputMode
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_OutputCfg *Config
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */
    if (NULL_PTR != Config)
    {
        PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
        const Pwm_Hal_OutputChannelCfg *ChCfg;
        uint32 ChannelDitherValue[2U] = {0U, 0U};
        Pwm_Hal_ChannelType Combine2ndChannel = PWM_CHANNEL_1;
        Pwm_Hal_ChannelPairType ChPair = PWM_CHANNEL_PAIR_0;

        Pwm_Hal_InitCommonAttr(Instance, Config->CommonCfg.BaseCfg);

        /* Set module up count or up-down count mode*/
        Pwm_Reg_SetCountMode(Base, (uint8)Config->CommonCfg.CountMode);

        /* trigger function init */
        Pwm_Reg_EnableInitTrigger(Base, Config->CommonCfg.EnableInitTrigger);
        Pwm_Reg_EnableMaxTrigger(Base, Config->CommonCfg.EnableMaxTrigger);
        Pwm_Reg_SetTriggerRatio(Base, Config->CommonCfg.TriggerRatio);

        Pwm_Reg_SetCombineCenterDutyType(Base, (uint8)Config->CommonCfg.CombineCenterDutyMode);

        for (uint8 i = 0U; i < Config->ChannelNum; i++)
        {
            ChCfg = &Config->ChannelCfg[i];

            switch (ChCfg->ChannelMode)
            {
            case OUTPUT_INDEPENDENT:
                Pwm_Reg_SetChannelOutputInitLevel(Base, (uint8)ChCfg->Channel, \
                    (uint8)ChCfg->IndependentChnCfg->InitLevel);
                Pwm_Reg_SetChannelMSR(Base, (uint8)ChCfg->Channel, 0x02U);
                Pwm_Reg_SetChannelELSR(Base, (uint8)ChCfg->Channel, \
                    (0x01U << (uint32)ChCfg->IndependentChnCfg->LevelMode));
                Pwm_Reg_EnableChannelDmaReq(Base, (uint8)ChCfg->Channel, \
                    ChCfg->IndependentChnCfg->EnableChnEventDmaReq);
                Pwm_Reg_SetChannelPolarity(Base, (uint8)ChCfg->Channel, \
                    (uint8)ChCfg->IndependentChnCfg->ActivePolarity);
                Pwm_Reg_EnableChannelMatchTrigger(Base, (uint8)ChCfg->Channel, \
                    ChCfg->IndependentChnCfg->EnableMatchTrigger);
                Pwm_Reg_EnableChannelInterrupt(Base, (uint8)ChCfg->Channel, ChCfg->IndependentChnCfg->EnableInterrupt);

                Pwm_Reg_SetChannelValue(Base, (uint8)ChCfg->Channel, ChCfg->IndependentChnCfg->ChnValue);
                ChannelDitherValue[(uint8)ChCfg->Channel / 4U] |= (\
                        ((uint32)ChCfg->IndependentChnCfg->ChnDither & PWM_DITHER0_C0DHR_Msk) << \
                        (((uint32)ChCfg->Channel % 4U) * PWM_DITHER_VALUE_WIDTH));

                break;
            case OUTPUT_COMBINE:
                /* Check the channel is even number */
                DEVICE_ASSERT(0U == ((uint8)ChCfg->Channel % 2U));

                Combine2ndChannel = Pwm_Hal_GetPairChannel((uint8)ChCfg->Channel + 1U);
                /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
                /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
                ChPair = (Pwm_Hal_ChannelPairType)((uint8)ChCfg->Channel >> 0x01U);
                /*PRQA S 4342 -- */
                /*PRQA S 4394 -- */

                /* Set init output level */
                Pwm_Reg_SetChannelOutputInitLevel(Base, (uint8)ChCfg->Channel, \
                                                  (uint8)ChCfg->ChnPairCfg->Ch1stInitLevel);
                Pwm_Reg_SetChannelOutputInitLevel(Base, (uint8)Combine2ndChannel, \
                                                  (uint8)ChCfg->ChnPairCfg->Ch2ndInitLevel);

                Pwm_Reg_SetChannelELSR(Base, (uint8)ChCfg->Channel, (0x01U << (uint32)ChCfg->ChnPairCfg->LevelMode));

                Pwm_Reg_EnableChannelCombine(Base, (uint8)ChPair, TRUE);
                Pwm_Reg_EnablePairChSymmetric(Base, (uint8)ChPair, ChCfg->ChnPairCfg->EnableSymmetric);
                Pwm_Reg_EnablePairChComplement(Base, (uint8)ChPair, ChCfg->ChnPairCfg->EnableChComplementation);

                /* Set channels match dir */
                Pwm_Reg_SetChannelMatchDir(Base, (uint8)ChCfg->Channel, (uint8)ChCfg->ChnPairCfg->Ch1stMatchDir);
                Pwm_Reg_SetChannelMatchDir(Base, (uint8)Combine2ndChannel, (uint8)ChCfg->ChnPairCfg->Ch2ndMatchDir);

                /* Enable deadtime */
                Pwm_Reg_EnablePairChDeadtime(Base, (uint8)ChPair, ChCfg->ChnPairCfg->EnableDeadTime);
                Pwm_Reg_SetPairChDeadtime(Base, (uint8)ChPair, \
                                          (uint8)ChCfg->ChnPairCfg->TimePsc, \
                                          (uint16)ChCfg->ChnPairCfg->DeadTimeValue);

                Pwm_Reg_EnableChannelDmaReq(Base, (uint8)ChCfg->Channel, ChCfg->ChnPairCfg->EnableCh1stEventDmaReq);
                Pwm_Reg_EnableChannelDmaReq(Base, (uint8)Combine2ndChannel, ChCfg->ChnPairCfg->EnableCh2ndEventDmaReq);

                Pwm_Reg_SetChannelPolarity(Base, (uint8)ChCfg->Channel, (uint8)ChCfg->ChnPairCfg->Ch1stActivePolarity);
                Pwm_Reg_SetChannelPolarity(Base, (uint8)Combine2ndChannel, \
                    (uint8)ChCfg->ChnPairCfg->Ch2ndActivePolarity);

                /* enable or disable channel match trigger */
                Pwm_Reg_EnableChannelMatchTrigger(Base, (uint8)ChCfg->Channel, \
                                                  ChCfg->ChnPairCfg->EnableCh1stMatchTrigger);
                Pwm_Reg_EnableChannelMatchTrigger(Base, (uint8)Combine2ndChannel, \
                                                  ChCfg->ChnPairCfg->EnableCh2ndMatchTrigger);

                /* enable or disable channel Interrupt */
                Pwm_Reg_EnableChannelInterrupt(Base, (uint8)ChCfg->Channel, ChCfg->ChnPairCfg->EnableCh1stInterrupt);
                Pwm_Reg_EnableChannelInterrupt(Base, (uint8)Combine2ndChannel, ChCfg->ChnPairCfg->EnableCh2ndInterrupt);

                /* Set channels match value */
                Pwm_Reg_SetChannelValue(Base, (uint8)ChCfg->Channel, ChCfg->ChnPairCfg->Ch1stValue);
                Pwm_Reg_SetChannelValue(Base, (uint8)Combine2ndChannel, ChCfg->ChnPairCfg->Ch2ndValue);
                ChannelDitherValue[(uint8)ChCfg->Channel / 4U] |= (\
                        ((uint32)ChCfg->ChnPairCfg->Ch1stDither & PWM_DITHER0_C0DHR_Msk) << \
                        (((uint32)ChCfg->Channel % 4U) * PWM_DITHER_VALUE_WIDTH));
                ChannelDitherValue[(uint8)ChCfg->Channel / 4U] |= (\
                        ((uint32)ChCfg->ChnPairCfg->Ch2ndDither & PWM_DITHER0_C0DHR_Msk) << \
                        ((((uint32)ChCfg->Channel % 4U) + 1U) * PWM_DITHER_VALUE_WIDTH));

                break;
            case OUTPUT_COMPARE:
                Pwm_Reg_SetChannelOutputInitLevel(Base, (uint8)ChCfg->Channel, (uint8)ChCfg->CompareChnCfg->InitLevel);
                Pwm_Reg_SetChannelMSR(Base, (uint8)ChCfg->Channel, 0x01U);
                Pwm_Reg_SetChannelELSR(Base, (uint8)ChCfg->Channel, (uint8)ChCfg->CompareChnCfg->Action);
                Pwm_Reg_EnableChannelInterrupt(Base, (uint8)ChCfg->Channel, ChCfg->CompareChnCfg->EnableInterrupt);
                Pwm_Reg_SetChannelPolarity(Base, (uint8)ChCfg->Channel, (uint8)ChCfg->CompareChnCfg->ActivePolarity);
                Pwm_Reg_EnableChannelMatchTrigger(Base, (uint8)ChCfg->Channel, \
                    ChCfg->CompareChnCfg->EnableMatchTrigger);
                Pwm_Reg_EnableChannelDmaReq(Base, (uint8)ChCfg->Channel, ChCfg->CompareChnCfg->EnableChnEventDmaReq);
                Pwm_Reg_SetChannelValue(Base, (uint8)ChCfg->Channel, ChCfg->CompareChnCfg->ChnValue);

                break;
            default:
                /* Useless param. */
                break;
            }
        }
        Pwm_Reg_SetChannelMatchDitherReg(Base, 0U, ChannelDitherValue[0U]);
        Pwm_Reg_SetChannelMatchDitherReg(Base, 1U, ChannelDitherValue[1U]);

        Pwm_Reg_EnableOutputInit(Base, Config->CommonCfg.InitOutput);

        Pwm_Reg_SetClockPsc(Base, Config->CommonCfg.BaseCfg->Prescaler);
        Pwm_Reg_SetClockSource(Base, (uint8)Config->CommonCfg.BaseCfg->ClockSource);
    }
    /*PRQA S 2812 -- */
}

/* Implementation of Pwm_Hal_InitInputMode declared in Pwm_Hal.h */
void Pwm_Hal_InitInputMode
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_InputCfg *Config
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */
    if (NULL_PTR != Config)
    {
        PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
        const Pwm_Hal_InputChannelCfg *ChCfg;
        Pwm_Hal_ChannelType Combine2ndChannel = PWM_CHANNEL_1;
        Pwm_Hal_ChannelPairType ChPair = PWM_CHANNEL_PAIR_0;

        Pwm_Hal_InitCommonAttr(Instance, Config->CommonCfg.BaseCfg);

        /* Set module up count or up-down count mode*/
        Pwm_Reg_SetCountMode(Base, (uint8)UP_COUNT);

        if (TRUE != Config->CommonCfg.EnableHall)
        {
            PwmIsCaptureMode[(uint8)Instance] = TRUE;
        }

        for (uint8 i = 0; i < Config->ChannelNum; i++)
        {
            ChCfg = &Config->ChannelCfg[i];

            if (INPUT_SINGLE == ChCfg->ChannelMode)
            {
                Pwm_Reg_SetChannelELSR(Base, (uint8)ChCfg->Channel, (uint8)ChCfg->CaptureEdge);
                Pwm_Reg_SetChannelMSR(Base, (uint8)ChCfg->Channel, 0U);
                Pwm_Reg_SetCaptureEventPsc(Base, (uint8)ChCfg->Channel, (uint8)ChCfg->EventPsc);
                Pwm_Reg_EnableChannelEventReset(Base, (uint8)ChCfg->Channel, ChCfg->EnableCounterReset);
                Pwm_Reg_EnableChannelDmaReq(Base, (uint8)ChCfg->Channel, ChCfg->EnableChnEventDmaReq);
                Pwm_Reg_EnableChannelInterrupt(Base, (uint8)ChCfg->Channel, ChCfg->EnableInterrupt);
            }
            else if (INPUT_DUAL == ChCfg->ChannelMode)
            {
                /* Check the channel is even number */
                DEVICE_ASSERT(0U == ((uint8)ChCfg->Channel % 2U));

                Combine2ndChannel = Pwm_Hal_GetPairChannel((uint8)ChCfg->Channel + 1U);
                /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
                /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
                ChPair = (Pwm_Hal_ChannelPairType)((uint8)ChCfg->Channel >> 1U);
                /*PRQA S 4342 -- */
                /*PRQA S 4394 -- */
                /* Set measure type */
                switch (ChCfg->DualInputMeasureType)
                {
                case PWM_POSITIVE_PLUSE_WIDTH_MEASURE:
                    Pwm_Reg_SetChannelELSR(Base, (uint8)ChCfg->Channel, 1U);
                    Pwm_Reg_SetChannelELSR(Base, (uint8)Combine2ndChannel, 2U);
                    break;

                case PWM_NEGATIVE_PLUSE_WIDTH_MEASURE:
                    Pwm_Reg_SetChannelELSR(Base, (uint8)ChCfg->Channel, 2U);
                    Pwm_Reg_SetChannelELSR(Base, (uint8)Combine2ndChannel, 1U);
                    break;

                case PWM_RISING_EDGE_PERIOD_MEASURE:
                    Pwm_Reg_SetChannelELSR(Base, (uint8)ChCfg->Channel, 1U);
                    Pwm_Reg_SetChannelELSR(Base, (uint8)Combine2ndChannel, 1U);
                    break;

                case PWM_FALLING_EDGE_PERIOD_MEASURE:
                    Pwm_Reg_SetChannelELSR(Base, (uint8)ChCfg->Channel, 2U);
                    Pwm_Reg_SetChannelELSR(Base, (uint8)Combine2ndChannel, 2U);
                    break;

                case PWM_BOTH_EDGE_DUTY_CYCLE_MEASURE:
                    Pwm_Reg_SetChannelELSR(Base, (uint8)ChCfg->Channel, 3U);
                    Pwm_Reg_SetChannelELSR(Base, (uint8)Combine2ndChannel, 3U);
                    break;

                default:
                    /* Illegal param. */
                    break;
                }

                Pwm_Reg_SetChannelMSR(Base, (uint8)ChCfg->Channel, (uint8)ChCfg->DualInputContinuousMode);
                Pwm_Reg_SetChannelMSR(Base, (uint8)Combine2ndChannel, (uint8)ChCfg->DualInputContinuousMode);
                Pwm_Reg_SetCaptureEventPsc(Base, (uint8)Combine2ndChannel, (uint8)ChCfg->EventPsc);
                Pwm_Reg_EnableChannelDmaReq(Base, (uint8)Combine2ndChannel, ChCfg->EnableChnEventDmaReq);

                Pwm_Reg_EnablePairChDualEdgeCapture(Base, (uint8)ChPair, TRUE);
                Pwm_Reg_SetPairChDualEdgeCapture(Base, (uint8)ChPair, TRUE);

                if (PWM_BOTH_EDGE_DUTY_CYCLE_MEASURE == ChCfg->DualInputMeasureType)
                {
                    Pwm_Reg_EnableChannelEventReset(Base, (uint8)ChCfg->Channel, ChCfg->EnableCounterReset);
                    Pwm_Reg_EnableChannelEventReset(Base, (uint8)Combine2ndChannel, ChCfg->EnableCounterReset);
                    Pwm_Reg_EnableChannelInterrupt(Base, (uint8)ChCfg->Channel, ChCfg->EnableInterrupt);
                }
                else
                {
                    Pwm_Reg_EnableChannelEventReset(Base, (uint8)Combine2ndChannel, ChCfg->EnableCounterReset);
                    /* Just set channel n+1 interrupt */
                    Pwm_Reg_EnableChannelInterrupt(Base, (uint8)Combine2ndChannel, ChCfg->EnableInterrupt);
                    if (TRUE == ChCfg->EnablePulseWidthMeasure)
                    {
                        Pwm_Reg_EnableChannelEventReset(Base, (uint8)ChCfg->Channel, ChCfg->EnableCounterReset);
                    }
                }
                Pwm_Reg_EnablePairPulseWidthMeasurement(Base, (uint8)ChPair, ChCfg->EnablePulseWidthMeasure);

                if (TRUE == Config->CommonCfg.EnableHall)
                {
                    /* config CH2 - phaze C */
                    /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
                    Pwm_Reg_EnablePairChDualEdgeCapture(Base, 1U, TRUE);
                    Pwm_Reg_SetPairChDualEdgeCapture(Base, 1U, TRUE);
                    /*PRQA S 4342 -- */

                    Pwm_Reg_EnableChannelEventReset(Base, (uint8)ChCfg->Channel, ChCfg->EnableCounterReset);
                    Pwm_Reg_EnableChannelInterrupt(Base, (uint8)ChCfg->Channel, ChCfg->EnableInterrupt);

                }
                Pwm_Reg_EnableHall(Base, Config->CommonCfg.EnableHall);
            }
            else
            {
                /* reserve */
            }

            /* Enable filtering for input channels */
            if (PWM_CHANNEL_4 > ChCfg->Channel)
            {
                Pwm_Reg_SetChannelInputFilterVal(Base, (uint8)ChCfg->Channel, ChCfg->FilterValue);
            }
        }

        Pwm_Reg_SetInputFilterPsc(Base, (uint8)Config->CommonCfg.FilterPsc);

        /* Enable clock */
        Pwm_Reg_SetClockPsc(Base, Config->CommonCfg.BaseCfg->Prescaler);
        Pwm_Reg_SetClockSource(Base, (uint8)Config->CommonCfg.BaseCfg->ClockSource);
    }
    /*PRQA S 2812 -- */
}

/* Implementation of Pwm_Hal_InitQuadDecoderMode declared in Pwm_Hal.h */
void Pwm_Hal_InitQuadDecoderMode
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_QuadDecoderCfg *Config
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(Config != NULL_PTR);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];

    Pwm_Hal_InitCommonAttr(Instance, Config->BaseCfg);

    Pwm_Reg_SetInputFilterPsc(Base, (uint8)Config->FilterPsc);
    /* Set polarity for Phase A and Phase B */
    Pwm_Reg_SetQuadPhaseAPolarity(Base, (uint8)Config->PhaseAConfig.Polarity);
    Pwm_Reg_SetQuadPhaseBPolarity(Base, (uint8)Config->PhaseBConfig.Polarity);
    Pwm_Reg_SetQuadPhaseZPolarity(Base, (uint8)Config->PhaseZConfig.Polarity);
    Pwm_Reg_EnableQuadPhaseZResetCnt(Base, Config->EnablePhaseZReset);
#if defined (AC7843X)
    Pwm_Reg_SetQuadPhaseZResetMode(Base, (uint8)Config->PhaseZResetMode);
    Pwm_Reg_EnableQuadPhaseZInterrupt(Base, Config->EnablePhaseZInterrupt);
    Pwm_ZDetectCallback[(uint8)Instance] = Config->ZDetectCallback;

    if ((uint32)Instance < PWM_INSTANCE_MAX)
    {
        if (TRUE == Config->EnablePhaseZInterrupt)
        {
            Core_Hal_EnableIrq(PwmDetectIrq[Instance]);
        }
        else
        {
            Core_Hal_DisableIrq(PwmDetectIrq[Instance]);
        }
    }
#endif

    /* phaseA corresponding channel 0 */
    Pwm_Reg_SetChannelInputFilterVal(Base, (uint8)PWM_CHANNEL_0, Config->PhaseAConfig.FilterValue);

    /* phaseB corresponding channel 1 */
    Pwm_Reg_SetChannelInputFilterVal(Base, (uint8)PWM_CHANNEL_1, Config->PhaseBConfig.FilterValue);

    /* phaseZ corresponding channel 2 */
    Pwm_Reg_SetChannelInputFilterVal(Base, (uint8)PWM_CHANNEL_2, Config->PhaseZConfig.FilterValue);

    Pwm_Reg_SetQuadEncodeMode(Base, (uint8)Config->Mode);
    Pwm_Reg_EnableQuadDecoder(Base, Config->EnableQuad);

    Pwm_Reg_SetClockPsc(Base, Config->BaseCfg->Prescaler);
    Pwm_Reg_SetClockSource(Base, (uint8)Config->BaseCfg->ClockSource);

    /*PRQA S 2812 -- */
}

/* Implementation of Pwm_Hal_DeInit declared in Pwm_Hal.h */
void Pwm_Hal_DeInit
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    if ((uint32)Instance < PWM_INSTANCE_MAX)
    {
        Core_Hal_DisableIrq(PwmOverflowIrq[Instance]);
        Core_Hal_DisableIrq(PwmChannelIrq[Instance]);
        Core_Hal_DisableIrq(PwmFaultIrq[Instance]);
#if defined (AC7843X)
        Core_Hal_DisableIrq(PwmDetectIrq[Instance]);
#endif

        /* Set pwm module at reset state and disable bus clock. */
        /*PRQA S 4394 ++ # convert unsigned to enum to ensure that there will be no problems in the current code.*/
        /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
        Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance), RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance), RCM_RESET_STATE_DEASSERT);
        (void)Ckgen_Hal_EnablePeriphClk(Pwm_ClkId[(uint8)Instance], FALSE);
#if defined (AC7840X)
        if (1U == ((uint8)Instance % 2U))
        {
            /* If PWM (Instance -1) is not in operation, turn off its clock. */
            if (0U == (Pwm_InstancesClkEnabledState & ((uint16)1 << ((uint8)Instance - 1U))))
            {
                Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance - 1U), \
                                      RCM_RESET_STATE_ASSERT);
                Rcm_Hal_SetResetState((Rcm_ResetIDType)((uint8)RCM_RESET_ID_PWM0 + (uint8)Instance - 1U), \
                                      RCM_RESET_STATE_DEASSERT);
                (void)Ckgen_Hal_EnablePeriphClk(Pwm_ClkId[(uint8)Instance - 1U], FALSE);
            }
        }
        Pwm_InstancesClkEnabledState &= (~((uint16)1 << (uint8)Instance));
#endif
        /*PRQA S 4342 -- */
        /*PRQA S 4394 -- */
        PwmIsCaptureMode[(uint8)Instance] = FALSE;

        Core_Hal_ClearPendingIrq(PwmOverflowIrq[Instance]);
        Core_Hal_ClearPendingIrq(PwmChannelIrq[Instance]);
        Core_Hal_ClearPendingIrq(PwmFaultIrq[Instance]);
#if defined (AC7843X)
        Core_Hal_ClearPendingIrq(PwmDetectIrq[Instance]);
#endif
    }
}

/* Implementation of Pwm_Hal_SetClockSource declared in Pwm_Hal.h */
void Pwm_Hal_SetClockSource
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ClockSourceType ClkSource,
    uint16 ClkPsc
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetClockPsc(Base, ClkPsc);
    Pwm_Reg_SetClockSource(Base, (uint8)ClkSource);
}

/* Implementation of Pwm_Hal_ResetCounter declared in Pwm_Hal.h */
void Pwm_Hal_ResetCounter
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ResetCounter(Base);
}

/* Implementation of Pwm_Hal_GetCountValue declared in Pwm_Hal.h */
uint16 Pwm_Hal_GetCountValue
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint16 Value = Pwm_Reg_GetCountValue(Base);

    return Value;
}

/* Implementation of Pwm_Hal_SetMaxCountValue declared in Pwm_Hal.h */
void Pwm_Hal_SetMaxCountValue
(
    Pwm_Hal_InstanceType Instance,
    uint16 Value
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetMaxCountValue(Base, Value);
}

/* Implementation of Pwm_Hal_GetMaxCountValue declared in Pwm_Hal.h */
uint16 Pwm_Hal_GetMaxCountValue
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint16 Value = Pwm_Reg_GetMaxCountValue(Base);

    return Value;
}

/* Implementation of Pwm_Hal_SetInitCountValue declared in Pwm_Hal.h */
void Pwm_Hal_SetInitCountValue
(
    Pwm_Hal_InstanceType Instance,
    uint16 Value
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetInitCountValue(Base, Value);
}

/* Implementation of Pwm_Hal_GetInitCountValue declared in Pwm_Hal.h */
uint16 Pwm_Hal_GetInitCountValue
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint16 Value = Pwm_Reg_GetInitCountValue(Base);

    return Value;
}

/* Implementation of Pwm_Hal_SetChannelValue declared in Pwm_Hal.h */
void Pwm_Hal_SetChannelValue
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    uint16 Value
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelValue(Base, (uint8)Channel, Value);
}

/* Implementation of Pwm_Hal_GetChannelValue declared in Pwm_Hal.h */
uint16 Pwm_Hal_GetChannelValue
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint16 ChV = Pwm_Reg_GetChannelValue(Base, (uint8)Channel);

    return ChV;
}

/* Implementation of Pwm_Hal_EnableChannelInterrupt declared in Pwm_Hal.h */
void Pwm_Hal_EnableChannelInterrupt
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Mask = (0x1UL << (uint8)Channel);
    Pwm_Reg_ClearChannelEventFlag(Base, Mask);
    Pwm_Reg_ClearChannelInterruptFlag(Base, (uint8)Channel);
    Pwm_Reg_EnableChannelInterrupt(Base, (uint8)Channel, Enable);
}

/* Implementation of Pwm_Hal_EnableOverflowInterrupt declared in Pwm_Hal.h */
void Pwm_Hal_EnableOverflowInterrupt
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableOverflowInterrupt(Base, Enable);
    if (TRUE == Enable)
    {
        Core_Hal_EnableIrq(PwmOverflowIrq[(uint8)Instance]);
    }
    else
    {
        Core_Hal_DisableIrq(PwmOverflowIrq[(uint8)Instance]);
    }
}

/* Implementation of Pwm_Hal_EnableOverflowEvent declared in Pwm_Hal.h */
void Pwm_Hal_EnableOverflowEvent
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableOverflowEvent(Base, Enable);
}

/* Implementation of Pwm_Hal_EnableUnderflowEvent declared in Pwm_Hal.h */
void Pwm_Hal_EnableUnderflowEvent
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableUnderflowEvent(Base, Enable);
}

/*PRQA S 1505 ++ # AMCE_QAC_1505. */
/* Implementation of Pwm_Hal_GetChannelInterruptFlag declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetChannelInterruptFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Flag = Pwm_Reg_GetChannelEventFlag(Base);

    return Flag;
}
/*PRQA S 1505 -- # AMCE_QAC_1505. */

/* Implementation of Pwm_Hal_ClearChannelInterruptFlag declared in Pwm_Hal.h */
void Pwm_Hal_ClearChannelInterruptFlag
(
    Pwm_Hal_InstanceType Instance,
    uint32 Mask
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearChannelEventFlag(Base, Mask);
}

/* Implementation of Pwm_Hal_GetOverflowFlag declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetOverflowFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Flag = Pwm_Reg_GetOverflowFlag(Base);

    return Flag;
}

/*PRQA S 1505 ++ # AMCE_QAC_1505. */
/* Implementation of Pwm_Hal_ClearOverflowFlag declared in Pwm_Hal.h */
void Pwm_Hal_ClearOverflowFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearOverflowFlag(Base);
}

/* Implementation of Pwm_Hal_GetOverflowDir declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetOverflowDir
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Flag = Pwm_Reg_GetOverflowDir(Base);

    return Flag;
}
/*PRQA S 1505 -- # AMCE_QAC_1505. */

/* Implementation of Pwm_Hal_GetAllChannelLevel declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetAllChannelLevel
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Levels = Pwm_Reg_GetAllChannelLevel(Base);

    return Levels;
}

/* Implementation of Pwm_Hal_EnableChannelDmaRequest declared in Pwm_Hal.h */
void Pwm_Hal_EnableChannelDmaRequest
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelDmaReq(Base, (uint8)Channel, Enable);
}

/* Implementation of Pwm_Hal_EnableOverflowDmaRequest declared in Pwm_Hal.h */
void Pwm_Hal_EnableOverflowDmaRequest
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableOverflowDmaReq(Base, Enable);
}

/* Implementation of Pwm_Hal_InitSyncConfigSet declared in Pwm_Hal.h */
void Pwm_Hal_InitSyncConfigSet
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_SyncCfg *ConfigPtr
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(NULL_PTR != ConfigPtr);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */

    uint8 i = 0U;
    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    boolean IsLoadPointNotSet = ((FALSE == ConfigPtr->EnableMaxLoadingPoint) && \
        (FALSE == ConfigPtr->EnableMinLoadingPoint)) ? TRUE : FALSE;

    /* Set synchronization trigger mode */
    if (PWM_SYNC_TRIGGER_SOFTWARE == ConfigPtr->TriggerType)
    {
        Pwm_Reg_SetSwPolTriggerMode(Base, TRUE);
        Pwm_Reg_SetCHOSWCRSwSyncTriggerMode(Base, TRUE);
        Pwm_Reg_SetInvcrSwSyncTriggerMode(Base, TRUE);
        Pwm_Reg_SetOmcrSwSyncTriggerMode(Base, TRUE);
        Pwm_Reg_SetMcvrSwSyncMode(Base, TRUE);
    }
    else
    {
        Pwm_Reg_SetHwPolTriggerMode(Base, TRUE);
        Pwm_Reg_SetCHOSWCRHwSyncTriggerMode(Base, TRUE);
        Pwm_Reg_SetInvcrHwSyncTriggerMode(Base, TRUE);
        Pwm_Reg_SetOmcrHwSyncTriggerMode(Base, TRUE);
        Pwm_Reg_SetMcvrHwSyncMode(Base, TRUE);
    }

    Pwm_Reg_EnablePOLSync(Base, ConfigPtr->EnablePolaritySync);
    Pwm_Reg_EnableCHOSWCRSync(Base, ConfigPtr->EnableSwOutputCtrlSync);
    Pwm_Reg_EnableINVCRSync(Base, ConfigPtr->EnableDualChannelInvertSync);
    Pwm_Reg_EnableOutputMaskSync(Base, ConfigPtr->EnableOutputMaskSync);
    Pwm_Reg_EnableCNTINSync(Base, ConfigPtr->EnableCounterInitSync);

    for (i = 0U; i < (uint8)PWM_CHANNEL_PAIR_NUM; i++)
    {
        /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
        Pwm_Reg_EnablePairChValueSync(Base, i, ConfigPtr->EnablePairedChnValSync[i]);
        /*PRQA S 4342 -- */
    }

    if (PWM_SYNC_TRIGGER_SOFTWARE == ConfigPtr->TriggerType)
    {
        /* update immediately when no load point is set */
        Pwm_Reg_SetCntSwSyncTriggerMode(Base, IsLoadPointNotSet);
    }
    else
    {
        Pwm_Reg_SetCntHwSyncTriggerMode(Base, IsLoadPointNotSet);
    }

    Pwm_Reg_EnableMaxLoadingPointSync(Base, ConfigPtr->EnableMaxLoadingPoint);
    Pwm_Reg_EnableMinLoadingPointSync(Base, ConfigPtr->EnableMinLoadingPoint);

    Pwm_Reg_EnableSyncHwTriggerSrc(Base, 0U, ConfigPtr->EnableHwSync0);
    Pwm_Reg_EnableSyncHwTriggerSrc(Base, 1U, ConfigPtr->EnableHwSync1);
    Pwm_Reg_EnableSyncHwTriggerSrc(Base, 2U, ConfigPtr->EnableHwSync2);
    Pwm_Reg_DisableHwTriggerSyncAfterTrigged(Base, ConfigPtr->DisableHwTriggerAfterTrigged);

    Pwm_Reg_SetSyncType(Base, 0U);
    Pwm_Reg_EnableSync(Base, ConfigPtr->EnableSync);
    Pwm_Reg_EnableSyncBYP(Base, ConfigPtr->EnableSyncBypass);

    /*PRQA S 2812 -- */

    Pwm_Reg_SetSyncMode(Base, (uint8)PWM_SYNC_MODE_ENHANCED);
}

/* Implementation of Pwm_Hal_EnableSync declared in Pwm_Hal.h */
void Pwm_Hal_EnableSync
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableSync(Base, Enable);
}

/* Implementation of Pwm_Hal_EnableSyncBypass declared in Pwm_Hal.h */
void Pwm_Hal_EnableSyncBypass
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableSyncBYP(Base, Enable);
}

/* Implementation of Pwm_Hal_TrigSoftwareSync declared in Pwm_Hal.h */
void Pwm_Hal_TrigSoftwareSync
(
    Pwm_Hal_InstanceType Instance,
    boolean IsTrigger
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_TrigSoftwareSync(Base, IsTrigger);
}

/* Implementation of Pwm_Hal_EnableGlobalTimeBase declared in Pwm_Hal.h */
void Pwm_Hal_EnableGlobalTimeBase
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableGlobalTimeBase(Base, Enable);
}

/* Implementation of Pwm_Hal_EnableGlobalTimeBaseOutput declared in Pwm_Hal.h */
void Pwm_Hal_EnableGlobalTimeBaseOutput
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableGlobalTimeBaseOutput(Base, Enable);
}

/* Implementation of Pwm_Hal_EnableInitTrigger declared in Pwm_Hal.h */
void Pwm_Hal_EnableInitTrigger
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableInitTrigger(Base, Enable);
}

/* Implementation of Pwm_Hal_EnableMaxTrigger declared in Pwm_Hal.h */
void Pwm_Hal_EnableMaxTrigger
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableMaxTrigger(Base, Enable);
}

/* Implementation of Pwm_Hal_EnableChannelMatchTrigger declared in Pwm_Hal.h */
void Pwm_Hal_EnableChannelMatchTrigger
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelMatchTrigger(Base, (uint8)Channel, Enable);
}

/* Implementation of Pwm_Hal_SetTriggerRatio declared in Pwm_Hal.h */
void Pwm_Hal_SetTriggerRatio
(
    Pwm_Hal_InstanceType Instance,
    uint8 TriggerRatio
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetTriggerRatio(Base, TriggerRatio);
}

/* Implementation of Pwm_Hal_SetChannelCaptureEdge declared in Pwm_Hal.h */
void Pwm_Hal_SetChannelCaptureEdge
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_EdgeType Edge
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelELSR(Base, (uint8)Channel, (uint8)Edge);
}

/* Implementation of Pwm_Hal_SetChannelInputFilterVal declared in Pwm_Hal.h */
void Pwm_Hal_SetChannelInputFilterVal
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    uint8 Value
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelInputFilterVal(Base, (uint8)Channel, Value);
}

/* Implementation of Pwm_Hal_SetInputFilterPsc declared in Pwm_Hal.h */
void Pwm_Hal_SetInputFilterPsc
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_InputFilterPscType Prescaler
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetInputFilterPsc(Base, (uint8)Prescaler);
}

/* Implementation of Pwm_Hal_EnableChannelEventReset declared in Pwm_Hal.h */
void Pwm_Hal_EnableChannelEventReset
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelEventReset(Base, (uint8)Channel, Enable);
}

/* Implementation of Pwm_Hal_GetHallStatus declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetHallStatus
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Status = Pwm_Reg_GetHallStatus(Base);

    return Status;
}

/* Implementation of Pwm_Hal_SetChannelCompareAction declared in Pwm_Hal.h */
void Pwm_Hal_SetChannelCompareAction
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_CompareActionType Action
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];

    Pwm_Reg_SetChannelELSR(Base, (uint8)Channel, (uint8)Action);
}

/* Implementation of Pwm_Hal_SetSoftControlEnableStates declared in Pwm_Hal.h */
void Pwm_Hal_SetSoftControlEnableStates
(
    Pwm_Hal_InstanceType Instance,
    uint8 EnableMask
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetSoftControlEnableStatesMask(Base, EnableMask);
}

/* Implementation of Pwm_Hal_EnableChannelSoftControl declared in Pwm_Hal.h */
void Pwm_Hal_EnableChannelSoftControl
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelSoftControl(Base, (uint8)Channel, Enable);
}

/* Implementation of Pwm_Hal_EnableCombineChannelSoftControl declared in Pwm_Hal.h */
void Pwm_Hal_EnableCombineChannelSoftControl
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint8 Mask = (uint8)Pwm_Reg_ReadChannelSoftControlState(Base);
    if (TRUE == Enable)
    {
        Mask = (Mask | (0x03U << (uint8)Channel));
    }
    else
    {
        Mask = (Mask & (~(0x03U << (uint8)Channel)));
    }
    Pwm_Reg_SetSoftControlEnableStatesMask(Base, Mask);
}

/* Implementation of Pwm_Hal_SetSoftControlLevels declared in Pwm_Hal.h */
void Pwm_Hal_SetSoftControlLevels
(
    Pwm_Hal_InstanceType Instance,
    uint8 LevelMask
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetSoftControlLevelMask(Base, LevelMask);
}

/* Implementation of Pwm_Hal_SetChannelSoftControlLevel declared in Pwm_Hal.h */
void Pwm_Hal_SetChannelSoftControlLevel
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_OutputLevelType Level
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelSoftControlLevel(Base, (uint8)Channel, (uint8)Level);
}

/* Implementation of Pwm_Hal_SetChannelMatchDither declared in Pwm_Hal.h */
void Pwm_Hal_SetChannelMatchDither
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    uint8 DitherValue
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelMatchDither(Base, (uint8)Channel, DitherValue);
}

/* Implementation of Pwm_Hal_SetPeriodDither declared in Pwm_Hal.h */
void Pwm_Hal_SetPeriodDither
(
    Pwm_Hal_InstanceType Instance,
    uint8 MaxCountDitherValue
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetPeriodDither(Base, MaxCountDitherValue);
}

/* Implementation of Pwm_Hal_EnableChannelOutputMask declared in Pwm_Hal.h */
void Pwm_Hal_EnableChannelOutputMask
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelOutputMask(Base, (uint8)Channel, Enable);
}

/* Implementation of Pwm_Hal_SetOutputMask declared in Pwm_Hal.h */
void Pwm_Hal_SetOutputMask
(
    Pwm_Hal_InstanceType Instance,
    uint8 Mask
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetOutputMask(Base, Mask);
}

/* Implementation of Pwm_Hal_GetOutputMask declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetOutputMask
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    uint32 Mask = Pwm_Reg_GetOutputMask(Base);

    return Mask;
}

/* Implementation of Pwm_Hal_SetChannelPolarity declared in Pwm_Hal.h */
void Pwm_Hal_SetChannelPolarity
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    Pwm_Hal_ActivePolarityType Polarity
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetChannelPolarity(Base, (uint8)Channel, (uint8)Polarity);
}

/* Implementation of Pwm_Hal_SetDeadtime declared in Pwm_Hal.h */
void Pwm_Hal_SetDeadtime
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelPairType ChannelPair,
    Pwm_Hal_DeadTimePscType Prescaler,
    uint16 Value
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetPairChDeadtime(Base, (uint8)ChannelPair, (uint8)Prescaler, Value);
}

/* Implementation of Pwm_Hal_EnableChannelPairSymmetric declared in Pwm_Hal.h */
void Pwm_Hal_EnableChannelPairSymmetric
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelPairType ChannelPair,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnablePairChSymmetric(Base, (uint8)ChannelPair, Enable);
}

/* Implementation of Pwm_Hal_EnableChannelPairInvert declared in Pwm_Hal.h */
void Pwm_Hal_EnableChannelPairInvert
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelPairType ChannelPair,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnablePairChInvert(Base, (uint8)ChannelPair, Enable);
}

/* Implementation of Pwm_Hal_InitFaultControl declared in Pwm_Hal.h */
void Pwm_Hal_InitFaultControl
(
    Pwm_Hal_InstanceType Instance,
    const Pwm_Hal_FaultCfg *ConfigPtr
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);
    DEVICE_ASSERT(NULL_PTR != ConfigPtr);

    /*PRQA S 2812 ++ # Apparent: Dereference of NULL pointer. */

    if ((uint32)Instance < PWM_INSTANCE_MAX)
    {
        PWM_Type *Base = PwmBaseAddr[Instance];
        uint8 FaultInputId = (uint8)PWM_FAULT_INPUT_0;

        if (TRUE == ConfigPtr->EnableInterrupt)
        {
            Core_Hal_EnableIrq(PwmFaultIrq[Instance]);
        }
        else
        {
            Core_Hal_DisableIrq(PwmFaultIrq[Instance]);
        }

        Pwm_Reg_EnableFaultInterrupt(Base, ConfigPtr->EnableInterrupt);
        Pwm_FaultCallback[(uint8)Instance] = ConfigPtr->FaultCallback;

        /* Control the output channels. */
        Pwm_Reg_EnableFaultHizOutput(Base, ConfigPtr->EnableHiz);
        for (uint8 i = 0U; i < (uint8)PWM_CHANNEL_PAIR_NUM; i++)
        {
            /*PRQA S 4394 ++ # A composite expression of 'essentially unsigned' type is being cast to a different type category. */
            /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
            Pwm_Reg_EnablePairChFaultCtrl(Base, i, ConfigPtr->EnableChannelOutputCtrl[i]);
            /*PRQA S 4342 -- */
            /*PRQA S 4394 -- */
        }

        Pwm_Reg_SetFaultMode(Base, (uint8)ConfigPtr->FaultCtrlMode);
        Pwm_Reg_SetInputFilterPsc(Base, (uint8)ConfigPtr->FilterPsc);
        Pwm_Reg_SetFaultInputFilterValue(Base, ConfigPtr->FilterValue);

#if defined (AC7843X) || defined (AC7842X) || defined (AC7840E)
        Pwm_Reg_RecoverZeroOrFullDutyAfterFaultCleared(Base, TRUE);
#endif

        for (FaultInputId = (uint8)PWM_FAULT_INPUT_0; FaultInputId < (uint8)PWM_FAULT_INPUT_MAX; FaultInputId++)
        {
            /*PRQA S 4394 ++ # A composite expression of 'essentially unsigned' type is being cast to a different type category. */
            /*PRQA S 4342 ++ # An expression of 'essentially unsigned' type is being cast to enum type. */
            Pwm_Reg_EnableFaultInputFilter(Base, FaultInputId, \
                                           ConfigPtr->FaultPinCfg[FaultInputId].EnableFaultFilter);
            Pwm_Reg_SetFaultInputPolarity(Base, FaultInputId, \
                                          (uint8)ConfigPtr->FaultPinCfg[FaultInputId].FaultPolarity);
            Pwm_Reg_EnableFaultPinInput(Base, FaultInputId, \
                                        ConfigPtr->FaultPinCfg[FaultInputId].EnableFaultInput);
            /*PRQA S 4342 -- */
            /*PRQA S 4394 -- */
        }
    }

    /*PRQA S 2812 -- */
}

/* Implementation of Pwm_Hal_EnableFaultPinInput declared in Pwm_Hal.h */
void Pwm_Hal_EnableFaultPinInput
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableFaultPinInput(Base, (uint8)FaultInputId, Enable);
}

/* Implementation of Pwm_Hal_EnableFaultInputFilter declared in Pwm_Hal.h */
void Pwm_Hal_EnableFaultInputFilter
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableFaultInputFilter(Base, (uint8)FaultInputId, Enable);
}

/* Implementation of Pwm_Hal_SetFaultInputPolarity declared in Pwm_Hal.h */
void Pwm_Hal_SetFaultInputPolarity
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId,
    Pwm_Hal_ActivePolarityType ActivePolarity
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetFaultInputPolarity(Base, (uint8)FaultInputId, (uint8)ActivePolarity);
}

/* Implementation of Pwm_Hal_GetFaultPinFlag declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetFaultPinFlag
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetFaultPinFlag(Base, (uint8)FaultInputId);
}

/* Implementation of Pwm_Hal_ClearFaultPinFlag declared in Pwm_Hal.h */
void Pwm_Hal_ClearFaultPinFlag
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_FaultInputIdType FaultInputId
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearFaultPinFlag(Base, (uint8)FaultInputId);
}

/* Implementation of Pwm_Hal_GetFaultFlag declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetFaultFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetFaultFlag(Base);
}

/* Implementation of Pwm_Hal_ClearFaultFlag declared in Pwm_Hal.h */
void Pwm_Hal_ClearFaultFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearFaultFlag(Base);
}

/* Implementation of Pwm_Hal_EnableChannelHizOutput declared in Pwm_Hal.h */
void Pwm_Hal_EnableChannelHizOutput
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_ChannelType Channel,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableChannelHizOutput(Base, (uint8)Channel, Enable);
}

/* Implementation of Pwm_Hal_GetQuadCountingDir declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetQuadCountingDir
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetQuadCountDir(Base);
}

/* Implementation of Pwm_Hal_GetQuadOverflowDir declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetQuadOverflowDir
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetQuadOverflowDir(Base);
}

/* Implementation of Pwm_Hal_GetQuadPhaseZFlag declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetQuadPhaseZFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetQuadPhaseZFlag(Base);
}

/* Implementation of Pwm_Hal_ClearQuadPhaseZFlag declared in Pwm_Hal.h */
void Pwm_Hal_ClearQuadPhaseZFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearQuadPhaseZFlag(Base);
}

/* Implementation of Pwm_Hal_GetDetectEventFlag declared in Pwm_Hal.h */
uint32 Pwm_Hal_GetDetectEventFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    const PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    return Pwm_Reg_GetQuadPhaseZFlag(Base);
}

/* Implementation of Pwm_Hal_ClearDetectEventFlag declared in Pwm_Hal.h */
void Pwm_Hal_ClearDetectEventFlag
(
    Pwm_Hal_InstanceType Instance
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_ClearQuadPhaseZFlag(Base);
}

/* Implementation of Pwm_Hal_EnableWriteProtection declared in Pwm_Hal.h */
void Pwm_Hal_EnableWriteProtection
(
    Pwm_Hal_InstanceType Instance,
    boolean Enable
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_EnableWriteProtection(Base, Enable);
}

/* Implementation of Pwm_Hal_SetDebugMode declared in Pwm_Hal.h */
void Pwm_Hal_SetDebugMode
(
    Pwm_Hal_InstanceType Instance,
    Pwm_Hal_DebugModeType Mode
)
{
    DEVICE_ASSERT((uint32)Instance < PWM_INSTANCE_MAX);

    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Pwm_Reg_SetDebugMode(Base, (uint8)Mode);
}

/**
 * @brief Overflow Interrupt handler
 * @note DES ID: DES_PWM_API_273
 * @param[in] Instance: PWM instance
 * @return void
 */
static void Pwm_Hal_Overflow_IrqHandler
(
    Pwm_Hal_InstanceType Instance
)
{
    /*overflow event flag*/
    uint32 Flag;
    Flag = Pwm_Hal_GetOverflowDir(Instance);
    Pwm_Hal_ClearOverflowFlag(Instance);
    /*overflow callback is not null*/
    if (NULL_PTR != Pwm_OverflowCallback[(uint8)Instance])
    {
        Pwm_OverflowCallback[Instance](Instance, Flag, (void *) NULL_PTR);
    }
}

/**
 * @brief Channel Interrupt handler
 * @note DES ID: DES_PWM_API_274
 * @param[in] Instance: PWM instance
 * @return void
 */
static void Pwm_Hal_Channel_IrqHandler
(
    Pwm_Hal_InstanceType Instance
)
{
    /*channel event flag*/
    uint32 Flag;
    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];
    Flag = Pwm_Hal_GetChannelInterruptFlag(Instance);

    /*channel event is true*/
    if (0UL != (Flag & PWM_STR_CH_EVENT_Msk))
    {
        if (TRUE == PwmIsCaptureMode[Instance])
        {
            /* The interrupt flag should be cleared by the user code */

            /* If the interrupt flag of all channels is directly cleared in ISR, the interrupt flag of the first edge of
               the low-frequency dual channel capture mode may be mistakenly cleared by the ISR of the high-frequency
               capture mode channel before the second edge capture is completed */
        }
        /* other mode */
        else
        {
            Pwm_Reg_ClearChannelEventFlag(Base, Flag);
        }
    }
    /*channel callback is not null*/
    if (NULL_PTR != Pwm_ChannelCallback[Instance])
    {
        Pwm_ChannelCallback[Instance](Instance, Flag, (void *) NULL_PTR);
    }
}

/**
 * @brief Fault Interrupt handler
 * @note DES ID: DES_PWM_API_275
 * @param[in] Instance: PWM instance
 * @return void
 */
static void Pwm_Hal_Fault_IrqHandler
(
    Pwm_Hal_InstanceType Instance
)
{
    /* store channel status and clear */
    uint32 Flag = Pwm_Hal_GetFaultFlag(Instance);
    if (TRUE == Flag)
    {
        /* clear fault flag will affect the output behavior, so it is oprated by the user*/
    }

    if (NULL_PTR != Pwm_FaultCallback[Instance])
    {
        Pwm_FaultCallback[(uint8)Instance](Instance, Flag, (void *) NULL_PTR);
    }
}

#if defined (AC7843X)
/**
 * @brief Phase Z detect Interrupt handler
 * @note DES ID: DES_PWM_API_276
 * @param[in] Instance: PWM instance
 * @return void
 */
static void Pwm_Hal_Detect_IrqHandler
(
    Pwm_Hal_InstanceType Instance
)
{
    PWM_Type *Base = PwmBaseAddr[(uint8)Instance];

    uint32 Flag = Pwm_Reg_GetQuadPhaseZFlag(Base);
    if (Flag != 0U)
    {
        Pwm_Reg_ClearQuadPhaseZFlag(Base);
    }

    if (NULL_PTR != Pwm_ZDetectCallback[Instance])
    {
        /* callback */
        Pwm_ZDetectCallback[Instance](Instance, Flag, (void *) NULL_PTR);
    }
}
#endif

/*PRQA S 3408 ++ # IRQHandler is used in startup.s */

/* Implementation of PWM0_Overflow_IRQHandler declared in Pwm_Hal.h */
void PWM0_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_0);
}

/* Implementation of PWM1_Overflow_IRQHandler declared in Pwm_Hal.h */
void PWM1_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_1);
}

/* Implementation of PWM2_Overflow_IRQHandler declared in Pwm_Hal.h */
void PWM2_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_2);
}

/* Implementation of PWM3_Overflow_IRQHandler declared in Pwm_Hal.h */
void PWM3_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_3);
}

#if (PWM_INSTANCE_MAX > 4U)
/* Implementation of PWM4_Overflow_IRQHandler declared in Pwm_Hal.h */
void PWM4_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_4);
}
#endif

#if (PWM_INSTANCE_MAX > 5U)
/* Implementation of PWM5_Overflow_IRQHandler declared in Pwm_Hal.h */
void PWM5_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_5);
}
#endif

#if (PWM_INSTANCE_MAX > 6U)
/* Implementation of PWM6_Overflow_IRQHandler declared in Pwm_Hal.h */
void PWM6_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_6);
}
#endif

#if (PWM_INSTANCE_MAX > 7U)
/* Implementation of PWM7_Overflow_IRQHandler declared in Pwm_Hal.h */
void PWM7_Overflow_IRQHandler(void)
{
    Pwm_Hal_Overflow_IrqHandler(PWM_INSTANCE_7);
}
#endif

/* Implementation of PWM0_Channel_IRQHandler declared in Pwm_Hal.h */
void PWM0_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_0);
}

/* Implementation of PWM1_Channel_IRQHandler declared in Pwm_Hal.h */
void PWM1_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_1);
}

/* Implementation of PWM2_Channel_IRQHandler declared in Pwm_Hal.h */
void PWM2_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_2);
}

/* Implementation of PWM3_Channel_IRQHandler declared in Pwm_Hal.h */
void PWM3_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_3);
}

#if (PWM_INSTANCE_MAX > 4U)
/* Implementation of PWM4_Channel_IRQHandler declared in Pwm_Hal.h */
void PWM4_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_4);
}
#endif

#if (PWM_INSTANCE_MAX > 5U)
/* Implementation of PWM5_Channel_IRQHandler declared in Pwm_Hal.h */
void PWM5_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_5);
}
#endif

#if (PWM_INSTANCE_MAX > 6U)
/* Implementation of PWM6_Channel_IRQHandler declared in Pwm_Hal.h */
void PWM6_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_6);
}
#endif

#if (PWM_INSTANCE_MAX > 7U)
/* Implementation of PWM7_Channel_IRQHandler declared in Pwm_Hal.h */
void PWM7_Channel_IRQHandler(void)
{
    Pwm_Hal_Channel_IrqHandler(PWM_INSTANCE_7);
}
#endif

/* Implementation of PWM0_Fault_IRQHandler declared in Pwm_Hal.h */
void PWM0_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_0);
}

/* Implementation of PWM1_Fault_IRQHandler declared in Pwm_Hal.h */
void PWM1_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_1);
}

/* Implementation of PWM2_Fault_IRQHandler declared in Pwm_Hal.h */
void PWM2_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_2);
}

/* Implementation of PWM3_Fault_IRQHandler declared in Pwm_Hal.h */
void PWM3_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_3);
}

#if (PWM_INSTANCE_MAX > 4U)
/* Implementation of PWM4_Fault_IRQHandler declared in Pwm_Hal.h */
void PWM4_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_4);
}
#endif

#if (PWM_INSTANCE_MAX > 5U)
/* Implementation of PWM5_Fault_IRQHandler declared in Pwm_Hal.h */
void PWM5_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_5);
}
#endif

#if (PWM_INSTANCE_MAX > 6U)
/* Implementation of PWM6_Fault_IRQHandler declared in Pwm_Hal.h */
void PWM6_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_6);
}
#endif

#if (PWM_INSTANCE_MAX > 7U)
/* Implementation of PWM7_Fault_IRQHandler declared in Pwm_Hal.h */
void PWM7_Fault_IRQHandler(void)
{
    Pwm_Hal_Fault_IrqHandler(PWM_INSTANCE_7);
}
#endif

#if defined (AC7843X)
/* Implementation of PWM0_Detect_IRQHandler declared in Pwm_Hal.h */
void PWM0_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_0);
}

/* Implementation of PWM1_Detect_IRQHandler declared in Pwm_Hal.h */
void PWM1_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_1);
}

/* Implementation of PWM2_Detect_IRQHandler declared in Pwm_Hal.h */
void PWM2_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_2);
}

/* Implementation of PWM3_Detect_IRQHandler declared in Pwm_Hal.h */
void PWM3_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_3);
}

/* Implementation of PWM4_Detect_IRQHandler declared in Pwm_Hal.h */
void PWM4_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_4);
}

/* Implementation of PWM5_Detect_IRQHandler declared in Pwm_Hal.h */
void PWM5_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_5);
}

/* Implementation of PWM6_Detect_IRQHandler declared in Pwm_Hal.h */
void PWM6_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_6);
}

/* Implementation of PWM7_Detect_IRQHandler declared in Pwm_Hal.h */
void PWM7_Detect_IRQHandler(void)
{
    Pwm_Hal_Detect_IrqHandler(PWM_INSTANCE_7);
}
#endif

/*PRQA S 3408 -- */

/* =============================================  EOF  ============================================== */
