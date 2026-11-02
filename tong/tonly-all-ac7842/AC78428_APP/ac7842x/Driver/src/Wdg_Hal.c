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
 * @file Wdg_Hal.c
 *
 * @brief This file provides WDG HAL functions.
 *
 */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Wdg_Hal.h"
#include "Ckgen_Hal.h"
#include "OsIf_Critical.h"
#include "OsIf_Time.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"
/* ============================================  DEFINES AND MACROS  ============================================ */
/** @brief  Define shift number  for tranformation between clock tick and time.
 *  @note DES ID: DES_WDG_MACRO_220
 */
#define WDG_PRESCALER_CLOCK_SHIFT 0x08U

/** @brief  Max value of WDG timeout register
 *  @note DES ID: DES_WDG_MACRO_221
 */
#define WDG_HAL_MAX_TIMEOUT  0xFFFFU

/** @brief  Frequency of LSI.
 *  @note DES ID: DES_WDG_MACRO_222
 */
#define LSI_FREQUENCY 128U /* 128KHz / 1000*/

/** @brief  For frequency transformation KHz <->Hz
 *  @note DES ID: DES_WDG_MACRO_223
 */
#define FREQUENCY_1KHZ   (1000U)

/** @brief  Delay time in microsecond for clock stable.
 *  @note DES ID: DES_WDG_MACRO_224
 */
#define WDG_HAL_DELAY_TIME  (50U)

/* ============================================= TYPEDEFS =============================================== */

/* =========================================== LOCAL VARIABLES ============================================== */
/** @brief Variable for storing running state(enable) of WDG
 *  @note DES ID: DES_WDG_VAR_210
 */
static boolean WdgEnabled = FALSE;

/*PRQA S 3218 ++ # AMCE_QAC_3218*/
/** @brief Variable for storing the pointer of callback function.
 *  @note DES ID: DES_WDG_VAR_211
 */
static Hal_CallbackType Wdg_IsrCallback = NULL_PTR;

/** @brief Variable for storing the parameter passed to callback function.
 *  @note DES ID: DES_WDG_VAR_212
 */
static void *CallbackArgs = NULL_PTR;
/*PRQA S 3218 -- # AMCE_QAC_3218*/

/** @brief Variable for storing interrupt enabled config.
 *  @note DES ID: DES_WDG_VAR_213
 */
static boolean WdgInterruptEnabled = FALSE;

/*=============================FUNCTION PROTOTYPES==================================*/
/**
 * @brief Calculate CS0 value for WDG register CS0
 *
 * @details This function calculates the CS0 value for the WDG register CS0.
 *
 * @param[in] ConfigPtr : Pointer to WDG hardware config.
 *
 * @return uint32 : CS0 value
 *
 * @pre WDG driver must be initialized.
 * @post CS0 value is calculated based on the configuration.
 *
 * @note Function ID: DES_WDG_API_251
 */
static uint32 Wdg_Hal_CalculateCs0(const Wdg_HalConfigType *ConfigPtr);

/**
 * @brief Calculate CS1/WIN/TOVAL value for WDG registers CS1/WIN/TOVAL
 *
 * @details This function calculates the CS1, WIN and TOVAL values for the WDG registers.
 *
 * @param[in] ConfigPtr : Pointer to WDG hardware config.
 * @param[out] WinPtr : Pointer for saving value of register WIN.
 * @param[out] TovalPtr : Pointer for saving value of register TOVAL.
 *
 * @return uint32 : CS1 value
 *
 * @pre WDG driver must be initialized.
 * @post CS1, WIN and TOVAL values are calculated based on the configuration.
 *
 * @note Function ID: DES_WDG_API_252
 */
static uint32 Wdg_Hal_CalculateOthers(const Wdg_HalConfigType *ConfigPtr, uint32 *WinPtr, uint32 *TovalPtr);

/*=========================GLOBAL FUNCTION  IMPLEMENTATIONS==========================*/

/* Implementation of Wdg_Hal_Init declared in Wdg_Hal.h */
void Wdg_Hal_Init(const Wdg_HalConfigType *ConfigPtr)
{
    uint32 Win;
    uint32 Toval;
    uint32 Cs0;
    uint32 Cs1;

    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
    if (FALSE == WdgEnabled)
    {
        Rcm_Hal_SetResetState(RCM_RESET_ID_WDG, RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(RCM_RESET_ID_WDG, RCM_RESET_STATE_DEASSERT);
        Cs0 = Wdg_Hal_CalculateCs0(ConfigPtr);
        Cs1 = Wdg_Hal_CalculateOthers(ConfigPtr, &Win, &Toval);

        /* WDG Interrupt is enabled */
        if (TRUE == WdgInterruptEnabled)
        {
            Core_Hal_SetIrqPriority(WDG_IRQn, 0U);
            Core_Hal_ClearPendingIrq(WDG_IRQn);
            Core_Hal_EnableIrq(WDG_IRQn);
        }
        else
        {
            Core_Hal_ClearPendingIrq(WDG_IRQn);
            Core_Hal_DisableIrq(WDG_IRQn);
        }
        OSIF_ENTER_CRITICAL(WDG_HAL_CS_ID1);
        /* Unlock WDG register.  */
        Wdg_Reg_WriteCNT(WDG_CNT_UNLOCK1);
        Wdg_Reg_WriteCNT(WDG_CNT_UNLOCK2);

        /* WDG register must be written within 128 bus clock*/

        /* Write WIN & TOVAL register*/
        Wdg_Reg_WriteWIN(Win);
        Wdg_Reg_WriteTOVAL(Toval);

        /* Write CS1 register*/
        Wdg_Reg_WriteCS1(Cs1);

        /* Write CS0 register and WDG will start to count*/
        Wdg_Reg_WriteCS0(Cs0);

        OSIF_EXIT_CRITICAL(WDG_HAL_CS_ID1);
        /* Wait for WDG clock to stabize.*/
        OsIf_UDelay(WDG_HAL_DELAY_TIME);
        WdgEnabled = TRUE;
    }
}

/* Implementation of Wdg_Hal_DeInit declared in Wdg_Hal.h */
void Wdg_Hal_DeInit(void)
{
    if (TRUE == WdgEnabled)
    {
        OSIF_ENTER_CRITICAL(WDG_HAL_CS_ID3);

        /* Disable WDG IRQ.  */
        Core_Hal_ClearPendingIrq(WDG_IRQn);
        Core_Hal_DisableIrq(WDG_IRQn);

        /* Unlock WDG register.  */
        Wdg_Reg_WriteCNT(WDG_CNT_UNLOCK1);
        Wdg_Reg_WriteCNT(WDG_CNT_UNLOCK2);

        /* WDG register must be written within 128 bus clock*/

        Wdg_Reg_WriteCS1(WDG_CS1_FLG_ENABLE);
        /* Write CS0 register. (Disable WDG)*/
        Wdg_Reg_WriteCS0(WDG_CS0_UPDATE_ENABLE);

        OSIF_EXIT_CRITICAL(WDG_HAL_CS_ID3);

        /* Wait for WDG clock to stabize.*/
        OsIf_UDelay(WDG_HAL_DELAY_TIME);
        WdgEnabled = FALSE;
    }
}

/* Implementation of Wdg_Hal_Feed declared in Wdg_Hal.h */
void Wdg_Hal_Feed(void)
{
    OSIF_ENTER_CRITICAL(WDG_HAL_CS_ID2);
    Wdg_Reg_WriteCNT(WDG_CNT_TRIGGER1);
    Wdg_Reg_WriteCNT(WDG_CNT_TRIGGER2);
    OSIF_EXIT_CRITICAL(WDG_HAL_CS_ID2);
}
#ifndef WDG_SDK_NON_EXTENDED_API
/* Implementation of Wdg_Hal_InstallCallback declared in Wdg_Hal.h */
void Wdg_Hal_InstallCallback(const Hal_CallbackType Func, void *Args)
{
    Wdg_IsrCallback = Func;
    CallbackArgs = Args;
}
#endif

/* Implementation of WDG_IRQHandler declared in Wdg_Hal.h */
ISR(WDG_IRQHandler)
{
    uint32 Cs1;
    Cs1 = Wdg_Reg_ReadCS1();

    /* WDG interrupt flag is set.*/
    if (0U != (WDG_CS1_FLG_ENABLE & Cs1))
    {
        /* User interrupt callback is set.*/
        if (NULL_PTR != Wdg_IsrCallback)
        {
            Wdg_IsrCallback(CallbackArgs);
        }

        OSIF_ENTER_CRITICAL(WDG_HAL_CS_ID3);

        /* Unlock WDG register.  */
        Wdg_Reg_WriteCNT(WDG_CNT_UNLOCK1);
        Wdg_Reg_WriteCNT(WDG_CNT_UNLOCK2);

        /* Clear WDG interrupt flag*/
        Wdg_Reg_WriteCS1(Cs1);
        OSIF_EXIT_CRITICAL(WDG_HAL_CS_ID3);
    }
}

/*========================STATIC  FUNCTION  IMPLEMENTATIONS========================*/

/* Implementation of Wdg_Hal_GetFrequency declared in Wdg_Hal.h */
uint32 Wdg_Hal_GetFrequency(Wdg_ClockSourceType ClockSource)
{
    uint32 ClockFrequency = LSI_FREQUENCY;
    Hal_StatusType Res; /* function return value */
    switch (ClockSource)
    {
    case WDG_CLOCK_BUS:
        /* Use Bus clock as WDG clock */
        Res = Ckgen_Hal_GetFreq(CKGEN_BUS_CLK, &ClockFrequency);
        break;
    case WDG_CLOCK_HSI:
        /* Use HSI as WDG clock */
        Res = Ckgen_Hal_GetFreq(CKGEN_HSI_CLK, &ClockFrequency);
        break;
    case WDG_CLOCK_HSE:
        /* Use HSE as WDG clock */
        Res = Ckgen_Hal_GetFreq(CKGEN_HSE_CLK, &ClockFrequency);
        break;
    default:
        /* LSI */
        Res = Ckgen_Hal_GetFreq(CKGEN_LSI_CLK, &ClockFrequency);
        break;
    }
    if (STATUS_SUCCESS == Res)
    {
        ClockFrequency /= FREQUENCY_1KHZ;
    }
    return ClockFrequency;
}

/* Implementation of Wdg_Hal_CalculateCs0 declared in Wdg_Hal.c */
static uint32 Wdg_Hal_CalculateCs0(const Wdg_HalConfigType *ConfigPtr)
{
    uint32 Cs0 = WDG_CS0_EN_ENABLE | WDG_CS0_UPDATE_ENABLE;
    /* Fast test function isn't enabled.*/
    /* Window mode is enabled*/
    if (0U != (ConfigPtr->Config & WDG_CONFIG_DBG_EN))
    {
        Cs0 |= WDG_CS0_DBG_ENABLE;
    }

    /* Window mode is enabled*/
    if (0U != (ConfigPtr->Config & WDG_CONFIG_LP_EN))
    {
        Cs0 |= WDG_CS0_STOP_ENABLE;
    }

    /* WDG Interrupt is enabled */
    if (TRUE == WdgInterruptEnabled)
    {
        Cs0 |= WDG_CS0_INT_ENABLE;
    }
    /* Fast test function is enabled.*/
    if (0U != (ConfigPtr->Config & WDG_CONFIG_FAST_TEST))
    {
        Cs0 |= WDG_CS0_TST_ENABLE;
    }
    return Cs0;
}

/**
 * @brief Calculate CS1/WIN/TOVAL value for WDG registers CS1/WIN/TOVAL
 *
 * @details This function calculates the CS1, WIN and TOVAL values for the WDG registers.
 *
 * @param[in] ConfigPtr : Pointer to WDG hardware config.
 * @param[out] WinPtr : Pointer for saving value of register WIN.
 * @param[out] TovalPtr : Pointer for saving value of register TOVAL.
 *
 * @return uint32 : CS1 value
 *
 * @pre WDG driver must be initialized.
 * @post CS1, WIN and TOVAL values are calculated based on the configuration.
 *
 * @note Function ID: DES_WDG_API_252
 */
static uint32 Wdg_Hal_CalculateOthers(const Wdg_HalConfigType *ConfigPtr, uint32 *WinPtr, uint32 *TovalPtr)
{
    uint32 Cs1 = WDG_CS1_FLG_ENABLE; /* Interrupt flag clear*/
    /* Calculate CS1*/
    uint32 ClockFrequency = Wdg_Hal_GetFrequency(ConfigPtr->ClockSource);
    switch (ConfigPtr->ClockSource)
    {
    case WDG_CLOCK_BUS:
        /* Use Bus clock as WDG clock */
        Cs1 |= WDG_CS1_BUS_CLK;
        break;
    case WDG_CLOCK_HSI:
        /* Use LSI as WDG clock */
        Cs1 |= WDG_CS1_HSI_CLK;
        break;
    case WDG_CLOCK_HSE:
        /* Use HSE as WDG clock */
        Cs1 |= WDG_CS1_HSE_CLK;
        break;
    default:
        /* LSI */
        break;
    }
    /* Window mode is enabled*/
    if (0U != (ConfigPtr->Config & WDG_CONFIG_WIN_EN))
    {
        Cs1 |= WDG_CS1_WIN_ENABLE;
    }

    *WinPtr = ConfigPtr->WindowValue * ClockFrequency;
    *TovalPtr = ConfigPtr->TimeoutValue * ClockFrequency;

    /* 256 prescaler is enabled*/
    if (0U != (ConfigPtr->Config & WDG_CONFIG_PRESCALER_EN))
    {
        Cs1 |= WDG_CS1_PRES_ENABLE;
        *WinPtr >>= WDG_PRESCALER_CLOCK_SHIFT;
        *TovalPtr >>= WDG_PRESCALER_CLOCK_SHIFT;
    }

    /* Fast test function is enabled.*/
    if (0U != (ConfigPtr->Config & WDG_CONFIG_FAST_TEST))
    {
        *TovalPtr = WDG_HAL_MAX_TIMEOUT; /* WDG will timeout in 64us*/
    }
    return Cs1;
}

#ifndef WDG_SDK_NON_EXTENDED_API
/* Implementation of Wdg_Hal_EnableInterrupt declared in Wdg_Hal.h */
void Wdg_Hal_EnableInterrupt(boolean Enable)
{
    /* WDG interrupt config is changed.*/
    if (WdgInterruptEnabled != Enable)
    {
        WdgInterruptEnabled = Enable;

        /* WDG is enabled.*/
        if (TRUE == WdgEnabled)
        {
            uint32 Cs0;
            Cs0 = Wdg_Reg_ReadCS0();

            /* WDG interrupt is enabled */
            if (TRUE == WdgInterruptEnabled)
            {
                Core_Hal_SetIrqPriority(WDG_IRQn, 0U);
                Core_Hal_ClearPendingIrq(WDG_IRQn);
                Core_Hal_EnableIrq(WDG_IRQn);
                Cs0 |= WDG_CS0_INT_ENABLE;
            }
            else /* WDG interrupt is disabled.*/
            {
                Core_Hal_DisableIrq(WDG_IRQn);
                Core_Hal_ClearPendingIrq(WDG_IRQn);
                Cs0 &= ~WDG_CS0_INT_ENABLE;
            }
            OSIF_ENTER_CRITICAL(WDG_HAL_CS_ID3);

            /* Unlock WDG register.  */
            Wdg_Reg_WriteCNT(WDG_CNT_UNLOCK1);
            Wdg_Reg_WriteCNT(WDG_CNT_UNLOCK2);

            /* Update interrupt enable bit of CS0*/
            Wdg_Reg_WriteCS0(Cs0);
            OSIF_EXIT_CRITICAL(WDG_HAL_CS_ID3);
        }
    }
}
#endif
/* =============================================  EOF  ============================================== */
