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
 * @file Spm_Hal.c
 *
 * @brief This file provides Hal Spm api.
 *
 * @details Comments augmented via MCP C/Doxygen guideline: document functions with `@brief/@param/@return`,
 *          describe macros and enums concisely, and use `@note`/`@warning` for constraints.
 */
/*==============================================INCLUDE FILES=======================================*/
#include "Spm_Hal.h"
#include "Core_Hal.h"
#include "AC784xx_Ckgen_Reg.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
#ifdef MCU_TIMEOUT_VALUE
#define SPM_TIMEOUT_VALUE MCU_TIMEOUT_VALUE
#else
#define SPM_TIMEOUT_VALUE (50000U)
#endif /* MCU_TIMEOUT_VALUE */

#define REG_DEBUG_MODE_ADDR    0x40008030UL
#define REG_PWR_MGR_SPLL_ADDR  0x4000803CUL
#define REG_PWR_MGR_XOSC_ADDR  0x40008048UL

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/** @brief Spm interrupt handler callback function */
static Hal_CallbackType SpmIsrCallback = NULL_PTR;

/** @brief Lvd interrupt handler callback function */
static Hal_CallbackType LvdIsrCallback = NULL_PTR;

#if defined(AC7842X)
/** @brief Lvd interrupt handler callback to flash */
static Hal_CallbackType LvdFlashCallback = NULL_PTR;
#endif /* AC7842X */

ISR(LVD_IRQHandler);

ISR(SPM_IRQHandler);

#if defined(AC7842X) || defined(AC7843X)
/** @brief Stb interrupt handler callback function */
static Hal_CallbackType StbIsrCallback = NULL_PTR;

ISR(STB_WU_IRQHandler);
#endif /* AC7842X AC7843X */
/*============================================FUNCTION PROTOTYPES===================================*/
/* Implementation of Spm_Hal_Init declared in Spm_Hal.h */
void Spm_Hal_Init(const Spm_ConfigType *ConfigPtr)
{
    DEVICE_ASSERT(NULL_PTR != ConfigPtr);

    if (NULL_PTR != ConfigPtr)
    {
        Spm_Reg_SelectACKTimeoutAction((uint32)ConfigPtr->SleepTimeoutAction);
#if defined(AC7842X) || defined(AC7843X)
        Spm_Reg_SelectStandbyAction((uint32)ConfigPtr->StandbyAction);
#endif /* AC7842X AC7843X */
        Spm_Reg_SelectLVDThreshold((uint32)ConfigPtr->LvdLevel);
        Spm_Reg_SelectLVRThreshold((uint32)ConfigPtr->LvrLevel);
#if defined (AC7843X) || defined (AC7842X) || defined (AC7840X)
        Spm_Reg_SetStandbyWakeupEn(ConfigPtr->StandbyWakeupSource);
#elif defined (AC7840E)
        Spm_Reg_SetStandbyWakeupEn0(ConfigPtr->StandbyWakeupSource0);
        Spm_Reg_SetStandbyWakeupEn1(ConfigPtr->StandbyWakeupSource1);
#endif
        SpmIsrCallback = ConfigPtr->SpmCallback;
#if (STD_ON == SPM_IRQ_CONTROL_INTERNAL)
        /* callback not null pointer, enable interrupt */
        if (NULL_PTR != SpmIsrCallback)
        {
            Core_Hal_ClearPendingIrq(SPM_IRQn);
            Core_Hal_EnableIrq(SPM_IRQn);
        }
#endif /* SPM_IRQ_CONTROL_INTERNAL */
        LvdIsrCallback = ConfigPtr->LvdCallback;
#if defined(AC7842X)
        Core_Hal_ClearPendingIrq(LVD_IRQn);
        Core_Hal_EnableIrq(LVD_IRQn);
#elif defined(AC7840X) || defined(AC7843X) || defined(AC7840E)
#if (STD_ON == LVD_IRQ_CONTROL_INTERNAL)
        /* callback not null pointer, enable interrupt */
        if (NULL_PTR != LvdIsrCallback)
        {
            Core_Hal_ClearPendingIrq(LVD_IRQn);
            Core_Hal_EnableIrq(LVD_IRQn);
        }
#endif /* LVD_IRQ_CONTROL_INTERNAL */
#endif /* AC7842X */
#if defined(AC7842X) || defined(AC7843X)
        StbIsrCallback = ConfigPtr->StbCallback;
#if (STD_ON == STB_IRQ_CONTROL_INTERNAL)
        /* callback not null pointer, enable interrupt */
        if (NULL_PTR != StbIsrCallback)
        {
            Core_Hal_ClearPendingIrq(STB_WU_IRQn);
            Core_Hal_EnableIrq(STB_WU_IRQn);
        }
#endif /* STB_IRQ_CONTROL_INTERNAL */
#endif /* AC7842X AC7843X */
    }
}

/* Implementation of Spm_Hal_GetCurrentPowerMode declared in Spm_Hal.h */
Spm_PowerModeType Spm_Hal_GetCurrentPowerMode(void)
{
    Spm_PowerModeType PowerMode; /* current power mode */

    /*PRQA S 4342 ++ # AMCE_MCU_4342 */
    PowerMode = (Spm_PowerModeType)Spm_Reg_GetPowerMode();
    /*PRQA S 4342 -- */

    return PowerMode;
}

/*!
 * @brief Set power mode.
 * @note  Function ID: DES_MCU_API_503
 * @param[in] Mode: power mode.
 *            - SPM_MODE_RUN
 *            - SPM_MODE_VLPR
 *            - SPM_MODE_STOP1
 *            - SPM_MODE_STOP2
 *            - SPM_MODE_VLPS
 *            - SPM_MODE_STANDBY
 * @return Set power mode pass or failed.
 */
/*PRQA S 3006 ++ # AMCE_MCU_3006 */
Hal_StatusType Spm_Hal_SetPowerMode(Spm_PowerModeType Mode)
{
    /* function return value */
    Hal_StatusType Status = STATUS_ERROR;
    /* previous power mode */
    Spm_PowerModeType PreMode;
    /* next power mode */
    Spm_PowerModeType NextMode;
    /* systick control register */
    uint32 SystickCtrl;
#if defined(AC7840X)
    uint32 PA5RegValue = 0U;
    uint32 RcmRegValue = 0U;
    uint32 ChipId;
    uint32 RegValue;
    boolean GpioIrqEnable = FALSE;
    boolean RcmIrqEnable = FALSE;
#elif defined(AC7842X) || defined(AC7843X) /* AC7840X */
    /* Rcm enable register */
    uint32 RcmEn = 0U;
#endif  /* AC7842X AC7843X */

    DEVICE_ASSERT(Mode < SPM_MODE_MAX);
#if defined(AC7840X)
    ChipId = Core_Hal_GetChipID();
    WRITE_MEM32(REG_DEBUG_MODE_ADDR, 0x78400001U);
    WRITE_MEM32(REG_PWR_MGR_XOSC_ADDR, 0x01122008U);
    /* set next mode is SPM_MODE_VLPS */
    if (SPM_MODE_VLPS == Mode)
    {
        /* spll status is enabled */
        if (TRUE == Spm_Reg_GetSPLLStatus())
        {
            /* spll clock source is HSE */
            if (1U == Ckgen_Reg_GetPllRefClk())
            {
                WRITE_MEM32(REG_PWR_MGR_SPLL_ADDR, 0x00024202U);
            }
        }
        if ((0x08U != ChipId) && (0x09U != ChipId))
        {
            /* Enable PA5 interrupt. */
            GpioIrqEnable = Core_Hal_IsIrqEnable(PORTA_IRQn);
            RegValue = PORTA->PCR[5U];
            PA5RegValue = RegValue;

            RegValue &= ~(PORT_PCR_IRQC_Msk);
            RegValue |= (0xAUL << PORT_PCR_IRQC_Pos);
            PORTA->PCR[5U] = RegValue;
            Core_Hal_EnableIrq(PORTA_IRQn);

            /* Enable RCM wdg interrupt. */
            RcmIrqEnable = Core_Hal_IsIrqEnable(RCM_IRQn);
            RegValue = CKGEN->RCM_CTRL;
            RcmRegValue = RegValue;
            RegValue |= (CKGEN_RCM_CTRL_RST_GLB_INT_EN_Msk | CKGEN_RCM_CTRL_WDG_RST_INT_EN_Msk |
            CKGEN_RCM_CTRL_EXT_RST_FILTER_EN_Msk);
            RegValue &= ~(CKGEN_RCM_CTRL_RST_DLY_TIME_Msk);
            RegValue |= (0x1UL << CKGEN_RCM_CTRL_RST_DLY_TIME_Pos);
            /* set ext reset filter to 5. */
            RegValue &= ~(CKGEN_RCM_CTRL_EXT_RST_FILTER_VAL_Msk);
            RegValue |= (0x5U << CKGEN_RCM_CTRL_EXT_RST_FILTER_VAL_Pos);
            CKGEN->RCM_CTRL = RegValue;
            Core_Hal_EnableIrq(RCM_IRQn);
        }
    }
#elif defined(AC7843X) /* AC7840X */
    Spm_Reg_SetIoSuspend(0U);
    if (SPM_MODE_STANDBY == Mode)
    {
        Spm_Reg_SetIoSuspend(1U);
        /* Clear standby wakeup status */
        Spm_Reg_ClearStandbyWakeupFlag();
    }
    RcmEn = READ_REG32(CKGEN->RCM_EN);
    WRITE_REG32(CKGEN->RCM_EN, RcmEn & (~(CKGEN_RCM_EN_PLL_UNLOCK_RST_EN_Msk | CKGEN_RCM_EN_XOSC_LOSS_RST_EN_Msk)));
    if ((SPM_MODE_VLPS == Mode) || (SPM_MODE_STANDBY == Mode))
    {
        /* spll status is enabled */
        if (TRUE == Spm_Reg_GetSPLLStatus())
        {
            WRITE_MEM32(REG_DEBUG_MODE_ADDR, 0x78430001U);
            WRITE_MEM32(REG_PWR_MGR_SPLL_ADDR, 0x00534341U);
        }
    }
#elif defined(AC7842X) /* AC7843X */
    if ((SPM_MODE_VLPS == Mode) || (SPM_MODE_STANDBY == Mode))
    {
        RcmEn = READ_REG32(CKGEN->RCM_EN);
        WRITE_REG32(CKGEN->RCM_EN, RcmEn & (~(CKGEN_RCM_EN_PLL_UNLOCK_RST_EN_Msk | CKGEN_RCM_EN_XOSC_LOSS_RST_EN_Msk)));
    }
    if (SPM_MODE_STANDBY == Mode)
    {
        Spm_Reg_SetIoSuspend(1U);
        /* Clear standby wakeup status */
        Spm_Reg_ClearStandbyWakeupFlag();
    }
    else
    {
        Spm_Reg_SetIoSuspend(0U);
    }
#elif defined(AC7840E)
    if (SPM_MODE_STANDBY == Mode)
    {
        Spm_Reg_SetIoSuspend(1U);
        /* Disable LVR */
        SET_BIT32(ANA->LVR_CFG0, ANA_LVR_CFG0_LVRDIS_Msk);
    }
    else
    {
        Spm_Reg_SetIoSuspend(0U);
    }
    Spm_Reg_SetSramLpEn();
#endif                 /* AC7842X */

    /*PRQA S 4342 ++ # AMCE_MCU_4342*/
    PreMode = (Spm_PowerModeType)Spm_Reg_GetPowerMode();
    /*PRQA S 4342 -- */
    /* Save and disable systick */
    SystickCtrl = READ_REG32(SysTick->CTRL);
    CLEAR_BIT32(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
    /* Clear systick pending bit */
    SET_BIT32(SCB->ICSR, SCB_ICSR_PENDSTCLR_Msk);
    Spm_Reg_SetPowerMode((uint32)Mode);
    /* Deep Sleep or standby */
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
    if (SPM_MODE_STOP1 <= Mode)
#elif defined(AC7840E)  /* AC7840X AC7842X AC7843X */
    if (SPM_MODE_VLPS <= Mode)
#endif
    {
        SET_BIT32(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);
        __WFI();
    }
    /*PRQA S 4342 ++ # AMCE_MCU_4342 */
    NextMode = (Spm_PowerModeType)Spm_Reg_GetPowerMode();
    /*PRQA S 4342 -- */
#if defined (AC7840X) || defined (AC7843X)
    /* Going to RUN or VLPR mode, need wait transition done */
    if ((SPM_MODE_RUN == Mode) || (SPM_MODE_VLPR == Mode))
#elif defined(AC7842X)  || defined (AC7840E)/* AC7840X AC7843X */
    if (SPM_MODE_RUN == Mode)
#endif                 /* AC7842X */
    {
        /* waiting for the transition to target mode  */
        for (uint32 Timeout = SPM_TIMEOUT_VALUE; Timeout > 0U; Timeout--)
        {
            if (NextMode == Mode) /* transition target mode done */
            {
                Status = STATUS_SUCCESS;
                break;
            }
            /*PRQA S 4342 ++ # AMCE_MCU_4342 */
            NextMode = (Spm_PowerModeType)Spm_Reg_GetPowerMode();
            /*PRQA S 4342 -- */
        }
    }
    else if (NextMode == PreMode) /* Going back to previous mode after wakeup from VLPS or STOP */
    {
        Status = STATUS_SUCCESS;
    }
    else
    {
        Status = STATUS_ERROR;
    }
#if defined(AC7840X)
    /* AUTOTEST SPLL DIV2*/
    if (SPM_MODE_VLPR != Mode)
    {
        if (TRUE == Spm_Reg_GetSPLLStatus()) /* spll status is enabled */
        {
            MODIFY_REG32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_SPLL_DIV2_Msk, CKGEN_CLK_DIV2_SPLL_DIV2_Pos, 0U);
            for (uint32 i = 0U; i < 100U; i++)
            {
                __NOP();
            }
            MODIFY_REG32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_SPLL_DIV2_Msk, CKGEN_CLK_DIV2_SPLL_DIV2_Pos, 1U);
        }
    }
    /* On VLPS mode, rollback PA5,Rcm. */
    if ((SPM_MODE_VLPS == Mode) && (0x08U != ChipId) && (0x09U != ChipId))
    {
        PORTA->PCR[5U] = PA5RegValue;
        CKGEN->RCM_CTRL = RcmRegValue;
        if (FALSE == GpioIrqEnable)
        {
            Core_Hal_DisableIrq(PORTA_IRQn);
        }
        if (FALSE == RcmIrqEnable)
        {
            Core_Hal_DisableIrq(RCM_IRQn);
        }
    }
#elif defined(AC7842X) /* AC7840X */
    if ((SPM_MODE_VLPS == Mode) || (SPM_MODE_STANDBY == Mode))
    {
        WRITE_REG32(CKGEN->RCM_EN, RcmEn);
    }
#elif defined(AC7843X) /* AC7842X */
    WRITE_REG32(CKGEN->RCM_EN, RcmEn);
#elif defined(AC7840E) /* AC7843X */
    /* Enable LVR */
    CLEAR_BIT32(ANA->LVR_CFG0, ANA_LVR_CFG0_LVRDIS_Msk);
#endif                 /* AC7840E */
    /* Restore systick */
    WRITE_REG32(SysTick->CTRL, SystickCtrl);

    return Status;
}
/*PRQA S 3006 -- */

/* Implementation of Spm_Hal_GetStandbyWakeupStatus declared in Spm_Hal.h */
uint32 Spm_Hal_GetStandbyWakeupStatus(void)
{
    return Spm_Reg_GetStandbyWakeupStatus();
}

/* Implementation of Spm_Hal_ClearStandbyWakeupStatus declared in Spm_Hal.h */
void Spm_Hal_ClearStandbyWakeupStatus(void)
{
    Spm_Reg_ClearStandbyWakeupStatus();
}

/*!
 * @brief Spm Sleep time out interrupt.
 * @note  Function ID: DES_MCU_API_508
 */
ISR(SPM_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    /* used for spm callback data */
    uint32 Status = Spm_Reg_GetModuleSleepACKStatus();

    Spm_Reg_ClearACKTimeOutFlag();
    /* SpmIsrCallback not null pointer, callback to user */
    if (SpmIsrCallback != NULL_PTR)
    {
        SpmIsrCallback((void *)&Status);
    }
}

#if defined(AC7842X)
/* Implementation of Spm_Hal_SetFlashLVDCallback declared in Spm_Hal.h */
void Spm_Hal_SetFlashLVDCallback(Hal_CallbackType Callback)
{
    LvdFlashCallback = Callback;
}
#endif /* AC7842X */

/*!
 * @brief Lvd interrupt.
 * @note  Function ID: DES_MCU_API_507
 */
ISR(LVD_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    /* used for lvd callback data */
    uint32 Threshold = Spm_Reg_GetLVDThreshold();

    /* LvdIsrCallback not null pointer, callback to user */
    if (LvdIsrCallback != NULL_PTR)
    {
        LvdIsrCallback((void *)&Threshold);
    }
#if defined(AC7842X)
    /* LvdFlashCallback not null pointer, callback to Flash */
    if (LvdFlashCallback != NULL_PTR)
    {
        LvdFlashCallback((void *)&Threshold);
    }
#endif /* AC7842X */
}

#if defined(AC7842X) || defined(AC7843X)
/*!
 * @brief Spm Low power detect interrupt.
 * @note  Function ID: DES_MCU_API_509
 */
ISR(STB_WU_IRQHandler)
{
    /* used for stb callback data */
    uint32 Status = Spm_Reg_GetStandbyWakeupStatus();
    Spm_Reg_ClearStandbyWakeupFlag();

    /* StbIsrCallback not null pointer, callback to user */
    if (StbIsrCallback != NULL_PTR)
    {
        StbIsrCallback((void *)&Status);
    }
}
#endif /* AC7842X AC7843X */
