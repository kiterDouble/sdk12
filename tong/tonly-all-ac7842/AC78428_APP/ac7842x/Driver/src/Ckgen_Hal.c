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
 * AutoChips Inc. (C) 2024. All rights reserved.
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
 * @file Ckgen_Hal.c
 *
 * @brief ckgen hal source file.
 */
/* ===========================================  INCLUDE FILES  =========================================== */
#include "Ckgen_Hal.h"
#include "Spm_Hal.h"
#include "AC784xx_Ckgen_Reg.h"
#include "AC784xx_Spm_Reg.h"
#if defined (AC7840X)
#include "AC784xx_Uart_Reg.h"
#endif
/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */
static uint32 HseFreq = 8000000U;

static uint32 RtcClkInFreq = 32768U;

static uint32 PwmExtClk0Freq = 32768U;

static uint32 PwmExtClk1Freq = 32768U;

static uint32 PwmExtClk2Freq = 32768U;
/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* =====================================  Functions definition  ===================================== */
/************          flash control          ************/
/**
 * @brief get flash clock frequency
 * @note Function ID: DES_CKGEN_API_150
 * @return uint32: flash clock frequency
 */
LOCAL_INLINE uint8 Ckgen_Hal_GetFlashClkFreq(void)
{
    return ((uint8)((FLASH->CNFG & FLASH_CNFG_CLKFREQ_Msk) >> FLASH_CNFG_CLKFREQ_Pos));
}

/**
 * @brief set flash clock frequency
 * @note Function ID: DES_CKGEN_API_149
 * @param[in] FlashFreq: flash clock frequency
 * @return void
 */
LOCAL_INLINE void Ckgen_Hal_SetFlashClkFreq(uint8 FlashFreq)
{
    /* Set flash clock */
#if defined (AC7843X)
    if (FlashFreq > 48U)
    {
        MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LATENCY_Msk, FLASH_CNFG_LATENCY_Pos, 0x3U);
        if (FlashFreq > 80U)
        {
            MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LATENCY1_Msk, FLASH_CNFG_LATENCY1_Pos, 0x8U);
        }
        else if (FlashFreq > 70U)
        {
            MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LATENCY1_Msk, FLASH_CNFG_LATENCY1_Pos, 0x7U);
        }
        else
        {
            MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LATENCY1_Msk, FLASH_CNFG_LATENCY1_Pos, 0x6U);
        }
    }
    else
    {
        MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LATENCY_Msk, FLASH_CNFG_LATENCY_Pos, 0x2U);
        if (FlashFreq > 40U)
        {
            MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LATENCY1_Msk, FLASH_CNFG_LATENCY1_Pos, 0x4U);
        }
        else
        {
            MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LATENCY1_Msk, FLASH_CNFG_LATENCY1_Pos, 0x3U);
        }
    }
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_CLKFREQ_Msk, FLASH_CNFG_CLKFREQ_Pos, FlashFreq);
#elif defined (AC7842X)  /* endif of AC7843X */
    /* Set flash clock */
    if (FlashFreq > 85U)
    {
        MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LATENCY_Msk, FLASH_CNFG_LATENCY_Pos, 0x5U);
    }
    else if (FlashFreq > 58U)
    {
        MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LATENCY_Msk, FLASH_CNFG_LATENCY_Pos, 0x4U);
    }
    else
    {
        MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LATENCY_Msk, FLASH_CNFG_LATENCY_Pos, 0x3U);
    }
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_CLKFREQ_Msk, FLASH_CNFG_CLKFREQ_Pos, FlashFreq);
#elif defined (AC7840E) /* endif of AC7842X */
    uint32 Latency;

    Latency = (uint32)FlashFreq;
    Latency /= 42U;
    Latency += 1U;
    if (Latency < 2U)
    {
        Latency = 2U;
    }
    MODIFY_REG32(CKGEN->LP_CLK_MUX, CKGEN_LP_CLK_MUX_LATENCY_Msk, CKGEN_LP_CLK_MUX_LATENCY_Pos, Latency); /*PRQA S 4491,3395,2985 # AMCE_MCU_MODIFY_REG32: mandated register macro form expands to composite bit operations. */
    Latency = (uint32)FlashFreq;
    Latency *= 2U;
    Latency /= 25U;
    Latency += 1U;
    if (Latency < 2U)
    {
        Latency = 2U;
    }
    if (Latency > 15U)
    {
        Latency = 15U;
    }
    MODIFY_REG32(CKGEN->LP_CLK_MUX, CKGEN_LP_CLK_MUX_LATENCY1_Msk, CKGEN_LP_CLK_MUX_LATENCY1_Pos, Latency); /*PRQA S 4491,3395,2985 # AMCE_MCU_MODIFY_REG32 */
    MODIFY_REG32(CKGEN->LP_CLK_MUX, CKGEN_LP_CLK_MUX_CLKFREQ_Msk, CKGEN_LP_CLK_MUX_CLKFREQ_Pos, FlashFreq); /*PRQA S 4491,3395,2985 # AMCE_MCU_MODIFY_REG32 */
#elif defined (AC7840X) /* endif of AC7840E */
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_CLKFREQ_Msk, FLASH_CNFG_CLKFREQ_Pos, FlashFreq);
#endif /* endif of AC7840X */
}

#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
/**
 * @brief get flash lock status
 * @note Function ID: DES_CKGEN_API_148
 * @return uint32: lock status
 */
LOCAL_INLINE uint8 Ckgen_Hal_GetFlashLockStatusReg(void)
{
    uint8 GetLockStatusReg;

    GetLockStatusReg = (uint8)((FLASH->STAT & FLASH_STAT_LOCK_Msk) >> FLASH_STAT_LOCK_Pos);

    return GetLockStatusReg;
}

/**
 * @brief This API sets key value 0x01234567 and 0xac7840 to KEYLK register to lock or unlock EFLASH_MODULE.
 * @note Function ID: DES_CKGEN_API_147
 * @param[in] State: TRUE->lock EFLASH_MODULE,FLASE->unlock EFLASH_MODULE
 * @return Hal_StatusType: STATUS_TIMEOUT or STATUS_SUCCESS
 */
static Hal_StatusType Ckgen_Hal_FlashLockCtrl(boolean State)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint32 TimeoutCount = 0x0U;
    uint8 CtrlLockBit;

    CtrlLockBit = Ckgen_Hal_GetFlashLockStatusReg();
    if (TRUE == State)
    {
        while (0x0U == CtrlLockBit)
        {
            WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY2);
            WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY1);
            if (100U < TimeoutCount)
            {
                RetVal = STATUS_TIMEOUT;
                break;
            }
            CtrlLockBit = Ckgen_Hal_GetFlashLockStatusReg();
            TimeoutCount++;
        }
    }
    else
    {
        while (0x1U == CtrlLockBit)
        {
            WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY1);
            WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY2);
            if (100U < TimeoutCount)
            {
                RetVal = STATUS_TIMEOUT;
                break;
            }
            CtrlLockBit = Ckgen_Hal_GetFlashLockStatusReg();
            TimeoutCount++;
        }
    }

    return RetVal;
}
#endif /* endif of AC7840X AC7842X AC7843X */

/************          clock source freq operation          ************/
/**
 * @brief Get VHSI frequency, if VHSI status is unstable, frequency is 0
 * @note Function ID: DES_CKGEN_API_146
 * @return uint32: VHSI frequency
 */
static uint32 Ckgen_Hal_GetVHSIFreq(void)
{
    boolean Status;/* VHSI clock status */
    uint32 Freq = 0U;/* VHSI clock frequency */

    Status = Spm_Reg_GetVHSIStatus();
    /* VHSI clock is valid. */
    if (TRUE == Status)
    {
        Freq = CKGEN_VHSI_FREQ;
    }

    return Freq;
}

/**
 * @brief Get HSE frequency, if HSE status is unstable, frequency is 0
 * @note Function ID: DES_CKGEN_API_145
 * @return uint32: HSE frequency
 */
static uint32 Ckgen_Hal_GetHSEFreq(void)
{
    boolean Status;/* HSE clock status */
    uint32 Freq = 0U;/* HSE frequency */

    Status = Spm_Reg_GetXOSCStatus();
    /* XOSC clock is valid. */
    if (TRUE == Status)
    {
        Freq = HseFreq;
    }

    return Freq;
}

/**
 * @brief Get HSI frequency, if HSI status is unstable, frequency is 0
 * @note Function ID: DES_CKGEN_API_144
 * @return uint32: HSI frequency
 */
static uint32 Ckgen_Hal_GetHSIFreq(void)
{
    boolean Status; /* HSI clock status */
    uint32 Freq = 0U;/* HSI clock frequency */
#if defined (AC7840X) || defined (AC7843X)
    Status = Spm_Reg_GetHSIStatus();
#elif defined (AC7842X) || defined (AC7840E) /* endif of AC7843X and AC7840X */
    Status = Spm_Reg_GetVHSIStatus();
#endif /* endif of AC7842X AC7840E */
    /* HSI clock is valid. */
    if (TRUE == Status)
    {
        Freq = CKGEN_HSI_FREQ;
    }

    return Freq;
}

/************          clock source status operation          ************/
/**
 * @brief Get clock source current status
 * @note Function ID: DES_CKGEN_API_143
 * @param[in] Clk: clock id, the rang is vhsi hsi hse spll
 * @return Hal_StatusType: status range is the STATUS_CLK_ON STATUS_CLK_OFF, STATUS_CLK_STABLE, STATUS_CLK_UNSTABLE
 */
static Hal_StatusType Ckgen_Hal_GetClkSrcStatus(Ckgen_ClkIdType Clk)
{
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
    Hal_StatusType ReturnValue;/* function return value */
#elif defined (AC7840E) /* endif of AC7840X AC7842X AC7843X */
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */
#endif /* endif of AC7840E */
    boolean Enable;/* whether clock is enable */
    boolean Status;/* whether clock is stable */
    /* processing for different clock sources */
    switch (Clk)
    {
    case CKGEN_VHSI_CLK:
        Enable = Spm_Reg_GetVHSIEnable();
        Status = Spm_Reg_GetVHSIStatus();
        break;
    case CKGEN_HSE_CLK:
        Enable = Spm_Reg_GetXOSCEnable();
        Status = Spm_Reg_GetXOSCStatus();
        break;
    case CKGEN_HSI_CLK:
#if defined (AC7840X) || defined (AC7843X)
        Enable = Spm_Reg_GetHSIEnable();
        Status = Spm_Reg_GetHSIStatus();
#elif defined (AC7842X) || defined (AC7840E) /* endif of AC7840X and AC7843X */
        Enable = Spm_Reg_GetVHSIEnable();
        Status = Spm_Reg_GetVHSIStatus();
#endif /* endif of AC7842X AC7840E */
        break;
#if defined (AC7840X) || defined (AC7843X)
    case CKGEN_HSI_VLPS_CLK:
        Enable = Spm_Reg_GetHSIInVLPSEnable();
        Status = Spm_Reg_GetHSIStatus();
        break;
#elif defined (AC7842X)  /* endif of AC7840X and AC7843X */
    case CKGEN_VHSI_VLPS_CLK:
        Enable = Spm_Reg_GetVHSIInVLPSEnable();
        Status = Spm_Reg_GetVHSIStatus();
        break;
#endif /* endif of AC7842X */
    case CKGEN_SPLL_CLK:
        Enable = Spm_Reg_GetSPLLEnable();
        Status = Spm_Reg_GetSPLLStatus();
        break;
    default:
        Enable = (boolean)FALSE;
        break;
    }
    /* clock enable and stable ,status is STATUS_CLK_STABLE clock enable and not stable ,status is STATUS_CLK_UNSTABLE*/
    ReturnValue = ((TRUE == Enable) ? ((TRUE == Status) ? STATUS_CLK_STABLE : STATUS_CLK_UNSTABLE) : STATUS_CLK_OFF);

    return ReturnValue;
}

/**
 * @brief Wait for the clock soure to stabilize
 * @note Function ID: DES_CKGEN_API_142
 * @param[in] Clk: clock id, the rang is vhsi hsi hse spll
 * @return Hal_StatusType: whether clock is stable, the range is the STATUS_SUCCESS STATUS_ERROR
 */
LOCAL_INLINE Hal_StatusType Ckgen_Hal_WaitClktoStability(Ckgen_ClkIdType Clk)
{
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */
    uint32 TimeOut = CKGEN_STABILIZATION_TIMEOUT;/* timeout value to wait clock stabilize */

    /* Wait for the sysclk clock soure to stabilize */
    while (0UL != TimeOut)
    {
        ReturnValue = Ckgen_Hal_GetClkSrcStatus(Clk);
        if (STATUS_CLK_STABLE == ReturnValue)
        {
            ReturnValue = STATUS_SUCCESS;
            break;
        }
        else
        {
            TimeOut--;
        }
    }
    if (0U == TimeOut)
    {
        ReturnValue = STATUS_ERROR;
    }

    return ReturnValue;
}

/************          clock source enable operation          ************/
/**
 * @brief Enable source clock
 * @note Function ID: DES_CKGEN_API_141
 * @param[in] Clk: clock id, the rang is vhsi hsi hse spll
 * @param[in] IsEnable : Turn on or turn off clock
 * @return Hal_StatusType: Enable success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
static Hal_StatusType Ckgen_Hal_EnableClkSrc(Ckgen_ClkIdType Clk, boolean IsEnable)
{
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */
#if defined (AC7843X) || defined (AC7842X)
    volatile uint32 Timeout = 100U;
#endif /* endif of AC7843X */

    switch (Clk)/* processing for different clock sources */
    {
    case CKGEN_VHSI_CLK:
        Spm_Reg_EnableVHSI(IsEnable);
        break;
#if defined (AC7840X) || defined (AC7843X)
    case CKGEN_HSI_VLPS_CLK:
        Spm_Reg_EnableHSIInVLPS(IsEnable);
        break;
    case CKGEN_HSI_CLK:
        Spm_Reg_EnableHSI(IsEnable);
        break;
#elif defined (AC7842X)  /* endif of AC7840X and AC7843X */
    case CKGEN_VHSI_VLPS_CLK:
        Spm_Reg_EnableVHSIInVLPS(IsEnable);
        break;
#endif /* endif of AC7842X*/
    case CKGEN_HSE_CLK:
        Spm_Reg_EnableXOSC(IsEnable);
        break;

    case CKGEN_SPLL_CLK:
        Spm_Reg_EnableSPLL(IsEnable);
#if defined (AC7843X) || defined (AC7842X)
        while (Timeout > 0U)
        {
            Timeout = Timeout - 1U;
        }
#endif
        break;

    default:
        ReturnValue = STATUS_ERROR;
        break;
    }

    return ReturnValue;
}

/************          div operation          ************/
/**
 * @brief Convert division actual value to register value
 * @note Function ID: DES_CKGEN_API_140
 * @param[in] Div: Division actual value
 * @return uint32: Division register value
 */
LOCAL_INLINE uint32 Ckgen_Hal_DivToRegValue(uint32 Div)
{
    /* The register value is the actual division value minus one */
    return (Div - 1U);
}

/**
 * @brief Convert division register value to actual value
 * @note Function ID: DES_CKGEN_API_139
 * @param[in] Div: Division register value
 * @return uint32: Division actual value
 */
LOCAL_INLINE uint32 Ckgen_Hal_DivToActualValue(uint32 Div)
{
    /* The actual division value is the register value plus one */
    return (Div + 1U);
}

/**
 * @brief Convert division register value to actual value
 * @note Function ID: DES_CKGEN_API_138
 * @param[in] Div: Division register value
 * @return uint32: Division actual value
 */
LOCAL_INLINE uint32 Ckgen_Hal_ClkoutDivToActualValue(uint32 Div)
{
    /* division actual value */
    uint32 ActualVal;

#if defined (AC7843X) || defined (AC7840E)
    /* register value is 0 1 2 ... 7 actual value is 1 2 4 ... 14 */
    ActualVal = ((Div > 0U) ? (2U * Div) : 1U);
#elif defined (AC7842X)  || defined (AC7840X) /* endif of AC7843X */
    /* [43] register value is 0 1 2 3 4 5 6 7 actual value is 1 2 3 4 5 6 7 8 */
    ActualVal = Div + 1U;
#endif /* endif of AC7842X and AC7840X */

    return ActualVal;
}

/**
 * @brief Convert prediv actual value to register value
 * @note Function ID: DES_CKGEN_API_137
 * @param[in] Div: Division actual value
 * @return uint32: Division register value
 */
LOCAL_INLINE uint32 Ckgen_Hal_PreDivToRegValue(uint32 Div)
{
    /* division register value */
    uint32 RegVal;
#if defined (AC7840X)
    /* actual value is 1 2 4  register value is 0 1 2 */
    RegVal = ((Div > 2U) ? 2U : (Div - 1U)); /* endif of AC7840X */
#elif defined (AC7842X)
    /* [42] actual value is 1 2 3 4  register value is 0 1 2 3 */
    /* [43] actual value is 1 2 3 4 5 6 7 register value is 0 1 2 3 4 5 6 */
    RegVal = Div - 1U;
#elif defined (AC7843X) || defined (AC7840E)
    /* [43] actual value is 1 2 3 4 5 6 7 register value is 1 2 3 4 5 6 7 */
    RegVal = Div;
#endif /* endif of AC7843X AC7840E */

    return RegVal;
}

/**
 * @brief Convert posdiv actual value to register value
 * @note Function ID: DES_CKGEN_API_136
 * @param[in] Div: Division actual value
 * @return uint32: Division register value
 */
LOCAL_INLINE uint32 Ckgen_Hal_PosDivToRegValue(uint32 Div)
{
    /* division register value */
    uint32 RegVal;

    /* actual value is 1 2 4 6...  register value is 0 1 2 3 ... */
    RegVal = ((1U == Div) ? 0U : (Div >> 1U));

    return RegVal;
}

/**
 * @brief Convert prediv register value to actual value
 * @note Function ID: DES_CKGEN_API_135
 * @param[in] Div: Division register value
 * @return uint32: Division actual value
 */
LOCAL_INLINE uint32 Ckgen_Hal_PreDivToActualVal(uint32 Div)
{
    /* division actual value */
    uint32 ActualVal;

#if defined (AC7840X)
    /* register value is 0 1 2   actual value is 1 2 4 */
    ActualVal = (((Div) > 1U) ? 4U : ((Div) + 1U));
#elif defined (AC7842X) /* endif of AC7840X */
    /* [42] register value is 0 1 2 3 actual value is 1 2 3 4  */
    ActualVal = Div + 1U;
#elif defined (AC7843X) || defined (AC7840E) /* endif of AC7842X */
    /* [43] register value is  1 2 3 4 5 6 7 actual value is 1 2 3 4 5 6 7 */
    ActualVal = Div;
#endif /* endif of AC7843X AC7840E */

    return ActualVal;
}

/**
 * @brief Convert posdiv register value to actual value
 * @note Function ID: DES_CKGEN_API_134
 * @param[in] Div: Division register value
 * @return uint32: Division actual value
 */
LOCAL_INLINE uint32 Ckgen_Hal_PosDivToActualVal(uint32 Div)
{
    /* division actual value */
    uint32 ActualVal;

    /* register value is 0 1 2 3 ...  actual value is 1 2 4 6... */
    ActualVal = ((0U == (Div)) ? 1U : ((Div) << 1U));

    return ActualVal;
}

/**
 * @brief Set peripheral clock division
 * @note Function ID: DES_CKGEN_API_006
 * @param[in] Clk: clock id,value can be one of the list value
 *                - CKGEN_CAN0_CLK    [40][42][43]
 *                - CKGEN_CAN1_CLK    [40][42][43]
 *                - CKGEN_CAN2_CLK    [40][42][43]
 *                - CKGEN_CAN3_CLK    [40][42][43]
 *                - CKGEN_CAN4_CLK    [42][43]
 *                - CKGEN_CAN5_CLK    [42][43]
 *                - CKGEN_CAN0_TS_CLK [40][42]
 *                - CKGEN_CAN1_TS_CLK [40][42]
 *                - CKGEN_CAN2_TS_CLK [40][42]
 *                - CKGEN_CAN3_TS_CLK [40][42]
 *                - CKGEN_CAN4_TS_CLK [40][42]
 *                - CKGEN_CAN5_TS_CLK [40][42]
 *                - CKGEN_PCT_CLK     [40][42][43]
 *                - CKGEN_TPIU_CLK    [40][42][43]
 * @param[in] Div: clock division
 * @return Hal_StatusType: Set success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_SetPeriphClkDiv(Ckgen_ClkIdType Clk, uint8 Div)
{
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */
    uint8 Value;

    Value = (uint8)Ckgen_Hal_DivToRegValue(Div);
#if defined (AC7840X)
    if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN3_CLK))
#elif defined (AC7840E)
    if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN1_CLK))
#elif defined (AC7842X)  || (AC7843X) /* endif of AC7840X*/
    if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN5_CLK))
#endif /* endif of AC7842X and AC7843X */
    {
        Ckgen_Reg_SetCanClkDiv(Clk, Value);
    }
#if defined (AC7842X) || defined (AC7840X) || defined (AC7840E)
#if defined (AC7840X)
    else if ((CKGEN_CAN0_TS_CLK <= Clk) && (Clk <= CKGEN_CAN3_TS_CLK))
#elif defined (AC7840E) /* endif of AC7840X*/
    else if ((CKGEN_CAN0_TS_CLK <= Clk) && (Clk <= CKGEN_CAN1_TS_CLK))
#elif defined (AC7842X)  /* endif of AC7840E*/
    else if ((CKGEN_CAN0_TS_CLK <= Clk) && (Clk <= CKGEN_CAN5_TS_CLK))
#endif /* endif of AC7842X*/
    {
        Ckgen_Reg_SetCanTsClkDiv(Clk, Value);
    }
#endif /* endif of AC7842X and AC7840X */
#if defined (AC7842X) || defined (AC7840X) || defined (AC7843X)
    else if (CKGEN_PCT_CLK == Clk)
    {
        Ckgen_Reg_SetPCTClkDiv(Value);
    }
#elif defined (AC7840E) /* endif of AC7842X AC7843X AC7840X */
    else if (CKGEN_EIO_CLK == Clk)
    {
        Ckgen_Reg_SetEIOClkDiv(Value);
    }
#endif /* endif of AC7840E */
    else if (CKGEN_TPIU_CLK == Clk)
    {
        Ckgen_Reg_SetTPIUClkDiv(Value);
    }
    else
    {
        ReturnValue = STATUS_ERROR;
    }

    return ReturnValue;
}

/**
 * @brief Get peripheral clock division
 * @note Function ID: DES_CKGEN_API_133
 * @param[in] Clk: clock id
 * @return uint8: clock division
 */
static uint8 Ckgen_Hal_GetPeriphClkDiv(Ckgen_ClkIdType Clk)
{
    uint32 Value;
    uint8 Div;

#if defined (AC7842X)  || defined (AC7843X)
    if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN5_CLK))
#elif defined (AC7840X) /* endif of AC7842X and AC7843X */
    if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN3_CLK))
#elif defined (AC7840E) /* endif of AC7842X and AC7843X */
    if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN1_CLK))
#endif /* endif of AC7840X */
    {
        Value = Ckgen_Reg_GetCanClkDiv(Clk);
    }
#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
#if defined (AC7840X)
    else if ((CKGEN_CAN0_TS_CLK <= Clk) && (Clk <= CKGEN_CAN3_TS_CLK))
#elif defined (AC7842X)  /* endif of AC7840X*/
    else if ((CKGEN_CAN0_TS_CLK <= Clk) && (Clk <= CKGEN_CAN5_TS_CLK))
#elif defined (AC7840E) /* endif of AC7842X */
    else if ((CKGEN_CAN0_TS_CLK <= Clk) && (Clk <= CKGEN_CAN1_TS_CLK))
#endif /* endif of AC7840E*/
    {
        Value = Ckgen_Reg_GetCanTsClkDiv(Clk);
    }
#endif /* endif of AC7842X and AC7840X */
#if defined (AC7842X) || defined (AC7840X) || defined (AC7843X)
    else if (CKGEN_PCT_CLK == Clk)
    {
        Value = Ckgen_Reg_GetPCTClkDiv();
    }
#elif defined (AC7840E) /* endif of AC7842X AC7843X AC7840X */
    else if (CKGEN_EIO_CLK == Clk)
    {
        Value = Ckgen_Reg_GetEIOClkDiv();
    }
#endif /* endif of AC7840E */
    else if (CKGEN_TPIU_CLK == Clk)
    {
        Value = Ckgen_Reg_GetTPIUClkDiv();
    }
    else
    {
        Value = 0U;
    }

    Div = (uint8)Ckgen_Hal_DivToActualValue(Value);

    return Div;
}

/**
 * @brief Get clock source div2 divider
 * @note Function ID: DES_CKGEN_API_132
 * @param[in] ClkSrc: clock id,value can be one of the list value
 *                - CKGEN_VHSI_DIV2_CLK [40][42][43]
 *                - CKGEN_HSE_DIV2_CLK  [40][42][43]
 *                - CKGEN_HSI_DIV2_CLK  [40][42][43]
 *                - CKGEN_SPLL_DIV2_CLK [40][42][43]
 * @return uint32:divider value
 */
static uint32 Ckgen_Hal_GetSrcClkDiv2Div(Ckgen_ClkIdType ClkSrc)
{
    uint32 Div;

    switch (ClkSrc)
    {
    case CKGEN_VHSI_DIV2_CLK:
        Div = Ckgen_Reg_GetVHSIDIV2ClkDiv();
        break;
    case CKGEN_HSE_DIV2_CLK:
        Div = Ckgen_Reg_GetHSEDIV2ClkDiv();
        break;

    case CKGEN_HSI_DIV2_CLK:
        Div = Ckgen_Reg_GetHSIDIV2ClkDiv();
        break;
    case CKGEN_SPLL_DIV2_CLK:
        Div = Ckgen_Reg_GetSPLLDIV2ClkDiv();
        break;
    default:
        Div = 0U;
        break;
    }

    return Div;
}

/**
 * @brief Get clock source div1 divider
 * @note Function ID: DES_CKGEN_API_131
 * @param[in] ClkSrc: clock id,value can be one of the list value
 *                - CKGEN_VHSI_DIV1_CLK [40][42][43]
 *                - CKGEN_HSE_DIV1_CLK  [40][42][43]
 *                - CKGEN_HSI_DIV1_CLK  [40][42][43]
 *                - CKGEN_SPLL_DIV1_CLK [40][42][43]
 * @return uint32: divider value
 */
static uint32 Ckgen_Hal_GetSrcClkDiv1Div(Ckgen_ClkIdType ClkSrc)
{
    uint32 Div;

    switch (ClkSrc)
    {
    case CKGEN_VHSI_DIV1_CLK:
        Div = Ckgen_Reg_GetVHSIDIV1ClkDiv();
        break;
    case CKGEN_HSE_DIV1_CLK:
        Div = Ckgen_Reg_GetHSEDIV1ClkDiv();
        break;
    case CKGEN_HSI_DIV1_CLK:
        Div = Ckgen_Reg_GetHSIDIV1ClkDiv();
        break;
    case CKGEN_SPLL_DIV1_CLK:
        Div = Ckgen_Reg_GetSPLLDIV1ClkDiv();
        break;
    default:
        Div = 0U;
        break;
    }

    return Div;
}

/**
 * @brief Get clock source division
 * @note Function ID: DES_CKGEN_API_130
 * @param[in] Clk: clock id
 * @return uint8: clock division
 */
static uint8 Ckgen_Hal_GetSrcClkDiv(Ckgen_ClkIdType Clk)
{
    uint8 Div = 0U;

    if ((CKGEN_SPLL_DIV1_CLK <= Clk) && (Clk <= CKGEN_HSE_DIV1_CLK))
    {
        Div = (uint8)Ckgen_Hal_GetSrcClkDiv1Div(Clk);
    }
    else if ((CKGEN_SPLL_DIV2_CLK <= Clk) && (Clk <= CKGEN_HSE_DIV2_CLK))
    {
        Div = (uint8)Ckgen_Hal_GetSrcClkDiv2Div(Clk);
    }
#if defined (AC7843X)
    else if ((CKGEN_ADC_SPLLDIV_CLK == Clk))
    {
        Div = (uint8)Ckgen_Reg_GetAdcSpllDiv();
    }
#endif /* endif of AC7843X */
    else
    {
        /* nothing */
    }

    Div = (uint8)Ckgen_Hal_DivToActualValue(Div);

    return Div;
}

/************          mux operation          ************/
/**
 * @brief sys clock id to register value
 * @note Function ID: DES_CKGEN_API_129
 * @param[in] ClkSrc: clock source id
 * @return uint32: mux clock register value
 */
static uint8 Ckgen_Hal_SysClkSrcToMuxVal(Ckgen_ClkIdType ClkSrc)
{
    uint8 MuxVal;

    switch (ClkSrc)
    {
    case CKGEN_VHSI_CLK:
        MuxVal = 0U;
        break;
    case CKGEN_HSE_CLK:
        MuxVal = 1U;
        break;
    case CKGEN_HSI_CLK:
        MuxVal = 2U;
        break;
    case CKGEN_SPLL_CLK:
        MuxVal = 3U;
        break;
    default:
        MuxVal = 0xFFU;
        break;
    }

    return MuxVal;
}

/**
 * @brief LSI clock id to register value
 * @note Function ID: DES_CKGEN_API_128
 * @param[in] ClkSrc: clock source id
 * @return uint32: mux clock register value
 */
static uint32 Ckgen_Hal_LSIClkToMuxVal(Ckgen_ClkIdType ClkSrc)
{
    uint32 MuxVal;

    switch (ClkSrc)
    {
    case CKGEN_LSI_128K_CLK:
        MuxVal = 0U;
        break;
    case CKGEN_LSI_32K_CLK:
        MuxVal = 1U;
        break;
    case CKGEN_LSI_1K_CLK:
        MuxVal = 2U;
        break;
    case CKGEN_OFF_CLK:
        MuxVal = 3U;
        break;
    default:
        MuxVal = 0xFFU;
        break;
    }

    return MuxVal;
}

#if defined (AC7840X)
/**
 * @brief RTC clock id to register value
 * @note Function ID: DES_CKGEN_API_127
 * @param[in] Ckgen_ClkIdType: clock source id
 * @return uint32: mux clock register value
 */
static uint32 Ckgen_Hal_RTCClkToMuxVal(Ckgen_ClkIdType ClkSrc)
{
    uint32 MuxVal;

    switch (ClkSrc)
    {
    case CKGEN_HSE_DIV1_CLK:
        MuxVal = 0U;
        break;
    case CKGEN_VHSI_DIV1_CLK:
        MuxVal = 1U;
        break;
    case CKGEN_LSI_32K_CLK:
        MuxVal = 2U;
        break;
    case CKGEN_RTC_CLKIN:
        MuxVal = 3U;
        break;
    default:
        MuxVal = 0xFFU;
        break;
    }

    return MuxVal;
}
#endif /* endif of AC7840X */

/**
 * @brief SysClk clock register value to clock id
 * @note Function ID: DES_CKGEN_API_126
 * @param[in] MuxVal: register value
 * @return Ckgen_ClkIdType: mux clock id
 */
static Ckgen_ClkIdType Ckgen_Hal_MuxValToSysClkSrc(uint32 MuxVal)
{
    Ckgen_ClkIdType ClkSrc;

    switch (MuxVal)
    {
    case 0U:
        ClkSrc = CKGEN_VHSI_CLK;
        break;
    case 1U:
        ClkSrc = CKGEN_HSE_CLK;
        break;
    case 2U:
        ClkSrc = CKGEN_HSI_CLK;
        break;
    case 3U:
        ClkSrc = CKGEN_SPLL_CLK;
        break;
    default:
        ClkSrc = CKGEN_OFF_CLK;
        break;
    }

    return ClkSrc;
}

#if defined (AC7840X)
/**
 * @brief RTC clock register value to clock id
 * @note Function ID: DES_CKGEN_API_125
 * @param[in] uint32: register value
 * @return Ckgen_ClkIdType: mux clock id
 */
static Ckgen_ClkIdType Ckgen_Hal_MuxValToRTCClk(uint32 MuxVal)
{
    Ckgen_ClkIdType ClkSrc;

    switch (MuxVal)
    {
    case 0U:
        ClkSrc = CKGEN_HSE_DIV1_CLK;
        break;
    case 1U:
        ClkSrc = CKGEN_VHSI_DIV1_CLK;
        break;
    case 2U:
        ClkSrc = CKGEN_LSI_32K_CLK;
        break;
    case 3U:
        ClkSrc = CKGEN_RTC_CLKIN;
        break;
    default:
        ClkSrc = CKGEN_OFF_CLK;
        break;
    }

    return ClkSrc;
}
#endif /* endif of AC7840X */

/**
 * @brief LSI clock register value to clock id
 * @note Function ID: DES_CKGEN_API_124
 * @param[in] MuxVal: register value
 * @return Ckgen_ClkIdType: mux clock id
 */
static Ckgen_ClkIdType Ckgen_Hal_MuxValToLSIClk(uint32 MuxVal)
{
    Ckgen_ClkIdType ClkSrc;

    switch (MuxVal)
    {
    case 0U:
        ClkSrc = CKGEN_LSI_128K_CLK;
        break;
    case 1U:
        ClkSrc = CKGEN_LSI_32K_CLK;
        break;
    case 2U:
        ClkSrc = CKGEN_LSI_1K_CLK;
        break;
    case 3U:
        ClkSrc = CKGEN_OFF_CLK;
        break;
    default:
        ClkSrc = CKGEN_OFF_CLK;
        break;
    }

    return ClkSrc;
}

/**
 * @brief Set peripheral clock mux
 * @note Function ID: DES_CKGEN_API_008
 * @param[in] Clk: clock id
 * @param[in] ClkSrc: clock source
 * @return Hal_StatusType: Operation success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_SetPeriphClkMux(Ckgen_ClkIdType Clk, Ckgen_ClkIdType ClkSrc)
{
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */

#if defined (AC7840X) || defined (AC7842X)
    if (Clk <= CKGEN_ADC1_CLK)
#elif defined (AC7840E)
    if (Clk <= CKGEN_ADC0_CLK)
#elif defined (AC7843X) /* endif of AC7840X and AC7842X */
    if (Clk <= CKGEN_SPI4_CLK)
#endif /* endif of AC7843X */
    {
        Ckgen_Reg_SetCommPeriphClkMux(Clk, ClkSrc);
    }
#if defined (AC7840X)
    else if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN3_CLK))
#elif defined (AC7840E)
    else if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN1_CLK))
#elif defined (AC7842X) || defined (AC7843X) /* endif of AC7840X */
    else if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN5_CLK))
#endif /* endif of AC7842X and AC7843X */
    {
        Ckgen_Reg_SetCanClkMux(Clk, ClkSrc);
    }
#if defined (AC7840X)
    else if ((CKGEN_PCT_CLK <= Clk) && (Clk <= CKGEN_UART3_CLK))
#elif defined (AC7840E)
    else if ((CKGEN_EIO_CLK <= Clk) && (Clk <= CKGEN_UART2_CLK))
#elif defined (AC7842X)  /* endif of AC7840X */
    else if ((CKGEN_PCT_CLK <= Clk) && (Clk <= CKGEN_SPI3_CLK))
#elif defined (AC7843X) /* endif of AC7842X */
    else if ((CKGEN_UART0_CLK <= Clk) && (Clk <= CKGEN_UART7_CLK))
#endif /* endif of AC7843X */
    {
        Ckgen_Reg_SetCommPeriphClkMux(Clk, ClkSrc);
    }
#if defined (AC7840X) || defined (AC7842X)
    else if ((CKGEN_PWM0_CLK <= Clk) && (Clk <= CKGEN_PWM5_CLK))
#elif defined(AC7840E)
    else if ((CKGEN_PWM0_CLK <= Clk) && (Clk <= CKGEN_PWM3_CLK))
#elif defined (AC7843X) /* endif of AC7840X and AC7842X */
    else if ((CKGEN_PWM0_CLK <= Clk) && (Clk <= CKGEN_PWM7_CLK))
#endif /* endif of AC7843X */
    {
        Ckgen_Reg_SetPwmClkMux(Clk, ClkSrc);
    }
    else
    {
        ReturnValue = STATUS_ERROR;
    }

    return ReturnValue;
}

static Ckgen_ClkIdType Ckgen_Hal_GetPeriphClkMux(Ckgen_ClkIdType Clk)
{
    Ckgen_ClkIdType ClkSrc;/* source clock id */

#if defined (AC7840X) || defined (AC7842X)
    if (Clk <= CKGEN_ADC1_CLK)
#elif defined(AC7840E)
    if (Clk <= CKGEN_ADC0_CLK)
#elif defined (AC7843X) /* endif of AC7840X and AC7842X */
    if (Clk <= CKGEN_SPI4_CLK)
#endif /* endif of AC7843X */
    {
        ClkSrc = Ckgen_Reg_GetCommPeriphClkMux(Clk);
    }
#if defined (AC7840X)
    else if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN3_CLK))
#elif defined(AC7840E)
    else if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN1_CLK))
#elif defined (AC7842X)  || defined (AC7843X) /* endif of AC7840X */
    else if ((CKGEN_CAN0_CLK <= Clk) && (Clk <= CKGEN_CAN5_CLK))
#endif /* endif of AC7843X and AC7842X */
    {
        ClkSrc = Ckgen_Reg_GetCanClkMux(Clk);
    }
#if defined (AC7840X)
    else if ((CKGEN_PCT_CLK <= Clk) && (Clk <= CKGEN_UART3_CLK))
#elif defined(AC7840E)
    else if ((CKGEN_EIO_CLK <= Clk) && (Clk <= CKGEN_UART2_CLK))
#elif defined (AC7842X)  /* endif of AC7840X */
    else if ((CKGEN_PCT_CLK <= Clk) && (Clk <= CKGEN_SPI3_CLK))
#elif defined (AC7843X) /* endif of AC7842X */
    else if ((CKGEN_UART0_CLK <= Clk) && (Clk <= CKGEN_UART7_CLK))
#endif /* endif of AC7843X */
    {
        ClkSrc = Ckgen_Reg_GetCommPeriphClkMux(Clk);
    }
#if defined (AC7840X) || defined (AC7842X)
    else if ((CKGEN_PWM0_CLK <= Clk) && (Clk <= CKGEN_PWM5_CLK))
#elif defined(AC7840E)
    else if ((CKGEN_PWM0_CLK <= Clk) && (Clk <= CKGEN_PWM3_CLK))
#elif defined (AC7843X) /* endif of AC7840X and AC7842X */
    else if ((CKGEN_PWM0_CLK <= Clk) && (Clk <= CKGEN_PWM7_CLK))
#endif /* endif of AC7843X */
    {
        ClkSrc = Ckgen_Reg_GetPwmClkMux(Clk);
    }
    else
    {
        ClkSrc = CKGEN_OFF_CLK;
    }

    return ClkSrc;
}

/**
 * @brief Get clock source mux
 * @note Function ID: DES_CKGEN_API_123
 * @param[in] Clk: clock id
 * @return Ckgen_ClkIdType: mux clock id
 */
static Ckgen_ClkIdType Ckgen_Hal_GetClkSrcMux(Ckgen_ClkIdType Clk)
{
    uint32 RegVal;/* register value */
    Ckgen_ClkIdType ClkSrc = CKGEN_OFF_CLK;/* source clock id */
    Spm_PowerModeType PowerMode;

    /* get lsi clock source */
    if (CKGEN_LSI_CLK == Clk)
    {
        RegVal = Ckgen_Reg_GetLSIMux();
        ClkSrc = Ckgen_Hal_MuxValToLSIClk(RegVal);
    }
#if defined (AC7840X)
    else if (CKGEN_RTC_CLK == Clk)/* get rtc clock source */
    {
        RegVal = Ckgen_Reg_GetRtcMux();
        ClkSrc = Ckgen_Hal_MuxValToRTCClk(RegVal);
    }
#endif /* endif of AC7840X */
    else if ((CKGEN_CORE_CLK == Clk) || (CKGEN_SYS_CLK == Clk)) /* get core clock source */
    {
        PowerMode = Spm_Hal_GetCurrentPowerMode();
        if (SPM_MODE_RUN == PowerMode)
        {
            RegVal = Ckgen_Reg_GetRunModeSysClkSrc();
            ClkSrc = Ckgen_Hal_MuxValToSysClkSrc(RegVal);
        }
#if defined (AC7840X) || defined (AC7843X)
        else
        {
            ClkSrc = CKGEN_HSI_CLK;
        }
#endif /* endif of AC7840X and AC7843X */
    }
    else if (CKGEN_CLK_OUT == Clk)
    {
        ClkSrc = Ckgen_Reg_GetClkoutClkMux();
    }
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
    else if (CKGEN_SPLL_CLK == Clk)
#elif defined(AC7840E)
    else if ((CKGEN_SPLL_CLK == Clk) || (CKGEN_SPLL1_CLK == Clk))
#endif
    {
        ClkSrc = Ckgen_Reg_GetSPLLClkMux();
    }
    else
    {
        /* nothing */
    }

    return ClkSrc;
}

/**
 * @brief Get clock source
 * @note Function ID: DES_CKGEN_API_007
 * @param[in] Clk: clock id
 * @return Ckgen_ClkIdType: clock source
 */
Ckgen_ClkIdType Ckgen_Hal_GetClkMux(Ckgen_ClkIdType Clk)   //PRQA S 1505 # AMCE_MCU_1505
{
    Ckgen_ClkIdType ClkSrc;

    if (Clk < CKGEN_CLK_OUT)
    {
        ClkSrc = Ckgen_Hal_GetPeriphClkMux(Clk);
    }
    else
    {
        ClkSrc = Ckgen_Hal_GetClkSrcMux(Clk);
    }

    return ClkSrc;
}

/************          periph clock enable operation          ************/
/**
 * @brief Enable clock
 * @note Function ID: DES_CKGEN_API_004
 * @param[in] Clk: clock id
 * @param[in] IsEnable: turn the clock on or off
 * @return Hal_StatusType: Operation success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_EnablePeriphClk(Ckgen_BusClkIdType Clk, boolean IsEnable)
{
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */

    Ckgen_Reg_SetPeriphEnable((uint32)Clk, IsEnable);

    return ReturnValue;
}

/**
 * @brief Get Pll input clock frequency
 * @note Function ID: DES_CKGEN_API_122
 * @return uint32: Pll input clock frequency
 */
static uint32 Ckgen_Hal_GetPllInFreq(void)
{
    uint8 PllRefSel;/* PLL refernce clock source */
    uint32 Freq; /* Pll input clock frequency */

    /* Get PLL refernce clock source */
    PllRefSel = Ckgen_Reg_GetPllRefClk();
    /* PLL clock source is HSI */
#if defined (AC7840X) || defined (AC7843X)
    if (CKGEN_PLL_IN_HSI == PllRefSel)
    {
        Freq = Ckgen_Hal_GetHSIFreq();
    }
#elif defined (AC7842X) || defined (AC7840E)  /* endif of AC7840X AC7843X */
    if (CKGEN_PLL_IN_VHSI == PllRefSel)
    {
        Freq = Ckgen_Hal_GetVHSIFreq();
    }
#endif /* endif of AC7842X AC7840E */
    else/* PLL clock source is HSE */
    {
        Freq = Ckgen_Hal_GetHSEFreq();
    }

    return Freq;
}

#if CKGEN_ENABLE_CHECK_PARAM
/**
 * @brief Check whether the division of clock is correct
 * @note Function ID: DES_CKGEN_API_121
 * @param[in] Clk: clock id
 * @param[in] Div: clock division
 * @return Hal_StatusType: Check success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
static Hal_StatusType Ckgen_Hal_CheckClkDiv(Ckgen_ClkIdType Clk, uint32 Div)
{
    Hal_StatusType Ret; /* function return value */

    switch (Clk)/* processing for different clock sources */
    {
    case CKGEN_CLK_OUT:
    case CKGEN_CAN0_TS_CLK:
    case CKGEN_CAN1_TS_CLK:
    case CKGEN_CAN2_TS_CLK:
    case CKGEN_CAN3_TS_CLK:
        Ret = (((Div < CKGEN_CAN_TS_CLK_DIV_MIN) || (Div > CKGEN_CAN_TS_CLK_DIV_MAX)) ? STATUS_ERROR : STATUS_SUCCESS);
        break;
    case CKGEN_CAN0_CLK:
    case CKGEN_CAN1_CLK:
    case CKGEN_CAN2_CLK:
    case CKGEN_CAN3_CLK:
        Ret = (((Div < CKGEN_CAN_CLK_DIV_MIN) || (Div > CKGEN_CAN_CLK_DIV_MAX)) ? STATUS_ERROR : STATUS_SUCCESS);
        break;
    case CKGEN_TPIU_CLK:
    case CKGEN_PCT_CLK:
        Ret = (((Div < CKGEN_PCT_CLK_DIV_MIN) || (Div > CKGEN_PCT_CLK_DIV_MAX)) ? STATUS_ERROR : STATUS_SUCCESS);
        break;
    default:
        Ret = (((Div < CKGEN_SOURCE_CLK_DIV_MIN) || (Div > CKGEN_SOURCE_CLK_DIV_MAX)) ? STATUS_ERROR : STATUS_SUCCESS);
        break;
    }

    return Ret;
}
#endif /* endif of CKGEN_ENABLE_CHECK_PARAM */

/**
 * @brief Get Pll clock frequency.
 * @note Function ID: DES_CKGEN_API_120
 * @return uint32: Pll clock frequency
 */
static uint32 Ckgen_Hal_GetPllFreq(void)
{
    boolean Status; /* Pll clock is stable */
    uint32 PreDiv, FbkDiv, PosDiv;/* Div register value */
    uint32 Freq = 0U;/* Pll frequency */

    Status = Spm_Reg_GetSPLLStatus();
    /* System PLL is valid. */
    if (TRUE == Status)
    {
        Freq = Ckgen_Hal_GetPllInFreq();
        /* If source frequency is valid. */
        if (0U != Freq)
        {
            Freq = Freq / 1000000UL;
            PreDiv = Ana_Reg_GetPllPreDivVal();
            FbkDiv = Ana_Reg_GetPllFbkDivVal();
            PosDiv = Ana_Reg_GetPllPosDivVal();
            Freq = (Freq * FbkDiv) / (Ckgen_Hal_PreDivToActualVal(PreDiv) * Ckgen_Hal_PosDivToActualVal(PosDiv));
            Freq = Freq * 1000000UL;
        }
    }

    return Freq;
}

#if defined (AC7840E)
/**
 * @brief Get Pll clock frequency.
 * @note Function ID: DES_CKGEN_API_120
 * @return uint32: Pll clock frequency
 */
static uint32 Ckgen_Hal_GetPllFreq_1(void)
{
    boolean Status; /* Pll clock is stable */
    uint32 PreDiv, FbkDiv, PosDiv;/* Div register value */
    uint32 Freq = 0U;/* Pll frequency */

    Status = Spm_Reg_GetSPLLStatus();
    /* System PLL is valid. */
    if (TRUE == Status)
    {
        Freq = Ckgen_Hal_GetPllInFreq();
        /* If source frequency is valid. */
        if (0U != Freq)
        {
            Freq = Freq / 1000000UL;
            PreDiv = Ana_Reg_GetPllPreDivVal();
            FbkDiv = Ana_Reg_GetPllFbkDivVal();
            PosDiv = Ana_Reg_GetPllPosDiv1Val();
            Freq = (Freq * FbkDiv) / (Ckgen_Hal_PreDivToActualVal(PreDiv) * Ckgen_Hal_PosDivToActualVal(PosDiv));
            Freq = Freq * 1000000UL;
        }
    }

    return Freq;
}
#endif

/**
 * @brief Get source clock frequency.
 * @note Function ID: DES_CKGEN_API_119
 * @param[in] Clk: clock id
 * @return uint32: clock frequency
 */
static uint32 Ckgen_Hal_GetClkSrcFreq(Ckgen_ClkIdType Clk)
{
    uint32 Freq;/* frequency value */

    switch (Clk)/* processing for different clock sources */
    {
    case CKGEN_HSE_CLK:
    case CKGEN_HSE_DIV1_CLK:
    case CKGEN_HSE_DIV2_CLK:
        Freq = Ckgen_Hal_GetHSEFreq();
        break;
#if defined (AC7840X) || defined (AC7843X)
    case CKGEN_HSI_CLK:
    case CKGEN_HSI_VLPS_CLK:
    case CKGEN_HSI_DIV1_CLK:
    case CKGEN_HSI_DIV2_CLK:
#elif defined (AC7842X) || defined (AC7840E) /* endif of AC7840X AC7843X */
    case CKGEN_VHSI_8M_CLK:
    case CKGEN_VHSI_8M_DIV1_CLK:
    case CKGEN_VHSI_8M_DIV2_CLK:
#endif /* endif of AC7842X AC7840E */
        Freq = Ckgen_Hal_GetHSIFreq();
        break;
    case CKGEN_VHSI_CLK:
    case CKGEN_VHSI_DIV1_CLK:
    case CKGEN_VHSI_DIV2_CLK:
        Freq = Ckgen_Hal_GetVHSIFreq();
        break;
    case CKGEN_SPLL_CLK:
    case CKGEN_SPLL_DIV1_CLK:
    case CKGEN_SPLL_DIV2_CLK:
#if defined (AC7843X)
    case CKGEN_ADC_SPLLDIV_CLK:
#endif /* endif of AC7843X */
        Freq = Ckgen_Hal_GetPllFreq();
        break;
#if defined (AC7840E)
    case CKGEN_SPLL1_CLK:
        Freq = Ckgen_Hal_GetPllFreq_1();
        break;
#endif /* endif of AC7840E */
    case CKGEN_LSI_128K_CLK:
        Freq = LSI_128K_FREQUENCY;
        break;
    case CKGEN_LSI_32K_CLK:
        Freq = LSI_32K_FREQUENCY;
        break;
    case CKGEN_LSI_1K_CLK:
        Freq = LSI_1K_FREQUENCY;
        break;
    case CKGEN_RTC_CLKIN:
        Freq = RtcClkInFreq;
        break;
    case CKGEN_PWM_EXT_CLK0:
        Freq = PwmExtClk0Freq;
        break;
    case CKGEN_PWM_EXT_CLK1:
        Freq = PwmExtClk1Freq;
        break;
    case CKGEN_PWM_EXT_CLK2:
        Freq = PwmExtClk2Freq;
        break;
    default:
        Freq = 0U;
        break;
    }

    return Freq;
}

/**
 * @brief Get common clock frequency, including sys and bus clock
 * @note Function ID: DES_CKGEN_API_118
 * @param[in] Clk: clock id
 * @return uint32: clock frequency
 */
static uint32 Ckgen_Hal_GetCommClkSrcFreq(Ckgen_ClkIdType Clk)
{
    uint32 Freq;/* clock frequency value */
    Ckgen_ClkIdType ClkId = Clk;/* clock source */
    uint8 MuxVa;
    Spm_PowerModeType PowerMode = Spm_Hal_GetCurrentPowerMode();

    /* the clock id rang is CKGEN_SYS_CLK ... CKGEN_FLASH_CLK */
    if ((Clk <= CKGEN_FLASH_CLK) && (CKGEN_SYS_CLK <= Clk))
    {
        if (SPM_MODE_RUN == PowerMode)
        {
            MuxVa = Ckgen_Reg_GetRunModeSysClkSrc();
            ClkId = Ckgen_Hal_MuxValToSysClkSrc(MuxVa);
        }
#if defined (AC7840X) || defined (AC7843X)
        else
        {
            ClkId = CKGEN_HSI_CLK;
        }
#endif /* endif of AC7843X AC7840X */
    }
    Freq = Ckgen_Hal_GetClkSrcFreq(ClkId);

    return Freq;
}

/**
 * @brief Get CanTs clock source frequency
 * @note Function ID: DES_CKGEN_API_117
 * @param[in] Clk: clock id
 * @return uint32: clock frequency
 */
static uint32 Ckgen_Hal_GetClkSrcDivFreq(Ckgen_ClkIdType Clk)
{
    uint32 ClkFreq;/* clock frequency value */
    uint32 Div;/* register value */

    ClkFreq = Ckgen_Hal_GetCommClkSrcFreq(Clk);
    Div = Ckgen_Hal_GetSrcClkDiv(Clk);
    ClkFreq /= Div;

    return ClkFreq;
}

/**
 * @brief Get CanTs clock source frequency
 * @note Function ID: DES_CKGEN_API_116
 * @param[in] Clk: clock id
 * @return uint32: clock frequency
 */
static uint32 Ckgen_Hal_GetSysAndBusFreq(Ckgen_ClkIdType Clk)
{
    uint32 ClkFreq;/* clock frequency value */
    uint8 BusClkDiv = 0U;/* bus clock division */
    uint8 SysClkDiv = 0U;/* bus clock division */
    Spm_PowerModeType PowerMode;

    PowerMode = Spm_Hal_GetCurrentPowerMode();
    ClkFreq = Ckgen_Hal_GetCommClkSrcFreq(Clk);
    /* get bus clock frequency */
    if (CKGEN_BUS_CLK == Clk)
    {
        if (SPM_MODE_RUN == PowerMode)
        {
            BusClkDiv = Ckgen_Reg_GetRunModeBusClkDiv();
        }
#if defined (AC7840X) || defined (AC7843X)
        else
        {
            BusClkDiv = Ckgen_Reg_GetVlprModeBusClkDiv();
        }
#endif /* endif of AC7843X AC7840X */
        ClkFreq /= Ckgen_Hal_DivToActualValue(BusClkDiv);
    }
    if (SPM_MODE_RUN == PowerMode)
    {
        SysClkDiv = Ckgen_Reg_GetRunModeSysClkDiv();
    }
#if defined (AC7840X) || defined (AC7843X)
    else
    {
        SysClkDiv = Ckgen_Reg_GetVlprModeSysClkDiv();
    }
#endif /* endif of AC7843X AC7840X */
    ClkFreq /= Ckgen_Hal_DivToActualValue(SysClkDiv);

    return ClkFreq;
}

#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
/**
 * @brief Get CanTs clock source frequency
 * @note Function ID: DES_CKGEN_API_115
 * @param[in] Clk: clock id
 * @return uint32: clock frequency
 */
static uint32 Ckgen_Hal_GetCanTsClkSrcFreq(Ckgen_ClkIdType Clk)
{
    uint32 ClkFreq;/* clock frequency value */
    uint32 Div;/* register value */
    Ckgen_ClkIdType ClkSrc;/* clock source value */
    Ckgen_ClkIdType CurrentClk;/* can clock id of can ts clock  */

    switch (Clk)/* processing for different clock sources */
    {
    case CKGEN_CAN0_TS_CLK:/* canTs ckock source originates from can clock */
        CurrentClk = CKGEN_CAN0_CLK;
        break;
    case CKGEN_CAN1_TS_CLK:
        CurrentClk = CKGEN_CAN1_CLK;
        break;
#if defined (AC7840X) || defined (AC7842X)
    case CKGEN_CAN2_TS_CLK:
        CurrentClk = CKGEN_CAN2_CLK;
        break;
    case CKGEN_CAN3_TS_CLK:
        CurrentClk = CKGEN_CAN3_CLK;
        break;
#endif
#if defined (AC7842X)
    case CKGEN_CAN4_TS_CLK:
        CurrentClk = CKGEN_CAN4_CLK;
        break;
    case CKGEN_CAN5_TS_CLK:
        CurrentClk = CKGEN_CAN5_CLK;
        break;
#endif /* endif of AC7842X */
    default:
        CurrentClk = CKGEN_OFF_CLK;
        break;
    }
    ClkSrc = Ckgen_Hal_GetClkMux(CurrentClk);
    ClkFreq = Ckgen_Hal_GetCommClkSrcFreq(ClkSrc);
    if ((CKGEN_SPLL_DIV1_CLK <= ClkSrc) && (ClkSrc < CKGEN_LSI_CLK))
    {
        ClkFreq = Ckgen_Hal_GetClkSrcDivFreq(ClkSrc);
    }
    Div = Ckgen_Hal_GetPeriphClkDiv(CurrentClk);
    ClkFreq /= Div;

    Div = Ckgen_Hal_GetPeriphClkDiv(Clk);
    ClkFreq /= Div;

    return ClkFreq;
}
#endif /* endif of AC7842X AC7840X AC7840E */

/**
 * @brief Get CanTs clock source frequency
 * @note Function ID: DES_CKGEN_API_114
 * @param[in] Clk: clock id
 * @return uint32: clock frequency
 */
static uint32 Ckgen_Hal_GetPeriphClkFreq(Ckgen_ClkIdType Clk)
{
    uint32 ClkFreq;/* clock frequency value */
    uint32 Div;/* register value */
    Ckgen_ClkIdType ClkSrc;/* clock source value */

    ClkSrc = Ckgen_Hal_GetClkMux(Clk);
    ClkFreq = Ckgen_Hal_GetCommClkSrcFreq(ClkSrc);
    if ((CKGEN_SPLL_DIV1_CLK <= ClkSrc) && (ClkSrc < CKGEN_LSI_CLK))
    {
        ClkFreq = Ckgen_Hal_GetClkSrcDivFreq(ClkSrc);
    }
    Div = Ckgen_Hal_GetPeriphClkDiv(Clk);
    ClkFreq /= Div;

    return ClkFreq;
}

/**
 * @brief Get CanTs clock source frequency
 * @note Function ID: DES_CKGEN_API_113
 * @return uint32: clock frequency
 */
static uint32 Ckgen_Hal_GetClkoutClkFreq(void)
{
    uint32 ClkFreq;/* clock frequency value */
    uint32 Div;/* register value */
    Ckgen_ClkIdType ClkSrc;/* clock source value */

    ClkSrc = Ckgen_Reg_GetClkoutClkMux();
#if defined (AC7840X)
    /* rtc and lsi clock need to get src clk */
    if ((CKGEN_LSI_CLK == ClkSrc) || (CKGEN_RTC_CLK == ClkSrc))
#elif defined (AC7843X) || defined (AC7842X) || defined (AC7840E)  /* endif of AC7840X */
    if (CKGEN_LSI_CLK == ClkSrc)
#endif /* endif of AC7842X AC7843X AC7840E */
    {
        ClkSrc = Ckgen_Hal_GetClkSrcMux(ClkSrc);
    }

    if ((CKGEN_SPLL_DIV1_CLK <= ClkSrc) && (ClkSrc < CKGEN_LSI_CLK))
    {
        ClkFreq = Ckgen_Hal_GetClkSrcDivFreq(ClkSrc);
    }
    else if ((CKGEN_SYS_CLK == ClkSrc) || (CKGEN_BUS_CLK == ClkSrc))
    {
        ClkFreq = Ckgen_Hal_GetSysAndBusFreq(ClkSrc);
    }
    else
    {
        ClkFreq = Ckgen_Hal_GetCommClkSrcFreq(ClkSrc);
    }
    Div = Ckgen_Reg_GetClkoutClkDiv();
    ClkFreq /= Ckgen_Hal_ClkoutDivToActualValue(Div);

    return ClkFreq;
}

/**
 * @brief Get CanTs clock source frequency
 * @note Function ID: DES_CKGEN_API_112
 * @return uint32: clock frequency
 */
static uint32 Ckgen_Hal_GetLSIClkFreq(void)
{
    uint32 ClkFreq;/* clock frequency value */
    Ckgen_ClkIdType ClkSrc;/* clock source value */
    uint32 MuxVal;

    MuxVal = Ckgen_Reg_GetLSIMux();
    ClkSrc = Ckgen_Hal_MuxValToLSIClk(MuxVal);
    ClkFreq = Ckgen_Hal_GetCommClkSrcFreq(ClkSrc);

    return ClkFreq;
}

#if defined (AC7840X)
/**
 * @brief Get CanTs clock source frequency
 * @note Function ID: DES_CKGEN_API_111
 * @return uint32: clock frequency
 */
static uint32 Ckgen_Hal_GetRTCClkFreq(void)
{
    uint32 ClkFreq;/* clock frequency value */
    Ckgen_ClkIdType ClkSrc;/* clock source value */
    uint32 MuxVal;

    MuxVal = Ckgen_Reg_GetRtcMux();
    ClkSrc = Ckgen_Hal_MuxValToRTCClk(MuxVal);
    ClkFreq = Ckgen_Hal_GetCommClkSrcFreq(ClkSrc);
    if ((CKGEN_SPLL_DIV1_CLK <= ClkSrc) && (ClkSrc < CKGEN_LSI_CLK))
    {
        ClkFreq = Ckgen_Hal_GetClkSrcDivFreq(ClkSrc);
    }
    return ClkFreq;
}
#endif /* endif of AC7840X */

/**
 * @brief Set sysclk clock source
 * @note Function ID: DES_CKGEN_API_110
 * @param[in] Mode: RUN or VLPR mode
 * @param[in] ClkSrc: sysclk clock source id
 * @return void
 */
static void Ckgen_Hal_SetSysClkSrc(Ckgen_SysClkModeType Mode, Ckgen_ClkIdType ClkSrc)
{
    uint8 MuxVal;/* index in the MuxRes */

    /* Disable lock bit field */
    Ckgen_Reg_EnableCTRLRegLock(FALSE);
    /* set run mode sysclk src */
    if (CKGEN_SYS_CLK_MODE_RUN == Mode)
    {
        MuxVal = Ckgen_Hal_SysClkSrcToMuxVal(ClkSrc);
        Ckgen_Reg_SetRunModeSysClkSrc(MuxVal);
    }
#if defined (AC7840X) || defined (AC7843X)
    else/* set vlpr mode sysclk src */
    {
        Ckgen_Reg_SetVlprModeSysClkSrc(CKGEN_VLPR_SYSCLK_HSI_REG_VAL);
    }
#endif /* endif of AC7843X AC7840X */
    Ckgen_Reg_EnableCTRLRegLock(TRUE);
}

#if defined (AC7840X)
/**
 * @brief Construct Uart Test Register value
 * @note Function ID: DES_CKGEN_API_109
 * @return void
 */
static void Ckgen_Hal_ConstructUartTestReg(UART_Type *Base, uint32 ClkFreq)
{
    /* Set the baudrate */
    Uart_Reg_SetBaudRateDivisor(Base, (float)ClkFreq / 16.0F / 125000.0F);
    Uart_Reg_SetBitCountPerChar(Base, UART_8_BITS_PER_CHAR);
    Uart_Reg_SetStopBitCount(Base, UART_ONE_STOP_BIT);
    Uart_Reg_SetTransmitterCmd(Base, TRUE);
    Uart_Reg_SetReceiverCmd(Base, TRUE);
    Uart_Reg_SetFIFO(Base, 0x01U);
    Uart_Reg_SetInterruptEn(Base, 0x0U);
}
/*PRQA S 3006 ++ # AMCE_MCU_3006*/
/**
 * @brief spll clock autotest
 * @note Function ID: DES_CKGEN_API_109
 * @return void
 */
static void Ckgen_Hal_AutoTest(void)
{
    uint32 ClkMux;  /* clock mux register value */
    uint32 ClkEn;   /* clock enable register value*/
    uint32 ClkDiv;  /* clock division */

    volatile uint32 TimeOut;           /* timeou value */
    volatile uint32 ClkFreq;  /* clock frequency */
    uint32 UartRegDivLVal;    /* Divisor low 8 bits register */
    uint32 UartRegDivHVal;    /* Divisor high 8 bits register */
    uint32 UartRegDivFracVal; /* Uart Fractional Divider Address */
    uint32 UartRegLcr0Val;    /* UART control register 0 */
    uint32 UartRegLcr1Val;    /* UART control register 1 */
    uint32 UartRegFcrVal;     /* FIFO Control Register */
    uint32 UartRegIerVal;     /* Interrupt Enable register */
    uint32 UartRegIdleVal;    /* Uart IDLE register */
    uint32 UartRegDmaEnVal;   /* UART DMA enable register */
    boolean Status;/* HSE clock status */

    WRITE_MEM32(0x40085148U, 0x00000001U);
    ClkDiv = CKGEN->CLK_DIV2;
    ClkMux = CKGEN->PERI_CLK_MUX2;
    ClkEn = CKGEN->PERI_CLK_EN0;
    Status = Spm_Reg_GetXOSCStatus();
    /* XOSC clock is valid. */
    if (TRUE == Status)
    {
        MODIFY_REG32(CKGEN->CLK_DIV2, 0x3FU, 0U, 1U);
        MODIFY_REG32(CKGEN->CLK_DIV2, 0xFC0000U, 18U, 0U);
        /* 0x47 :UART0 SPLL  UART1:HSE   0x47 :UART0 SPLL  UART1:HSI  */
        MODIFY_REG32(CKGEN->PERI_CLK_MUX2, 0xFFU, 0U, 0x47U);
    }
    else
    {
        MODIFY_REG32(CKGEN->CLK_DIV2, 0x3FU, 0U, 1U);
        MODIFY_REG32(CKGEN->CLK_DIV2, 0x3F000U, 12U, 0U);
        /* 0x47 :UART0 SPLL  UART1:HSE   0x47 :UART0 SPLL  UART1:HSI  */
        MODIFY_REG32(CKGEN->PERI_CLK_MUX2, 0xFFU, 0U, 0x57U);
    }
    SET_BIT32(CKGEN->PERI_CLK_EN0, 0x00000003U);

    UartRegDivLVal = READ_MEM32(0x40018204U);
    UartRegDivHVal = READ_MEM32(0x40018208U);
    UartRegDivFracVal = READ_MEM32(0x40018244U);
    UartRegLcr0Val = READ_MEM32(0x4001820CU);
    UartRegLcr1Val = READ_MEM32(0x40018210U);
    UartRegFcrVal = READ_MEM32(0x40018214U);
    UartRegIerVal = READ_MEM32(0x4001821CU);
    UartRegIdleVal = READ_MEM32(0x40018258U);
    UartRegDmaEnVal = READ_MEM32(0x40018240U);

    CLEAR_BIT32(CKGEN->PERI_SFT_RST0, 0x00000003U);
    SET_BIT32(CKGEN->PERI_SFT_RST0, 0x00000003U);
    __NOP();
    ClkFreq = Ckgen_Hal_GetClkSrcDivFreq(CKGEN_SPLL_DIV2_CLK);
    Ckgen_Hal_ConstructUartTestReg(UART0, ClkFreq);
    if (TRUE == Status)
    {
        ClkFreq = Ckgen_Hal_GetClkSrcDivFreq(CKGEN_HSE_DIV2_CLK);
    }
    else
    {
        ClkFreq = Ckgen_Hal_GetClkSrcDivFreq(CKGEN_HSI_DIV2_CLK);
    }
    Ckgen_Hal_ConstructUartTestReg(UART1, ClkFreq);
    __NOP();
    TimeOut = CKGEN_TIMEOUT_READY_VALUE;
    while ((0U == (READ_MEM32(0x40018220U) & 0x20U)) && (TimeOut > 0U))
    {
        TimeOut--;
    }
    WRITE_MEM32(0x40018200U, 0xAAU);
    TimeOut = CKGEN_TIMEOUT_READY_VALUE;
    while ((0U == (READ_MEM32(0x40018020U) & 1U)) && (TimeOut > 0U))
    {
        TimeOut--;
    }
    if (0xAAU != (uint8)READ_MEM32(0x40018000U))
    {
        WRITE_MEM32(0xE000ED0CU, (READ_MEM32(0xE000ED0CU) & 0x700U) | 0x05FA0004U);
    }
    CLEAR_BIT32(CKGEN->PERI_SFT_RST0, 0x00000003U);
    SET_BIT32(CKGEN->PERI_SFT_RST0, 0x00000003U);

    WRITE_MEM32(0x40018204U, UartRegDivLVal);
    WRITE_MEM32(0x40018208U, UartRegDivHVal);
    WRITE_MEM32(0x40018244U, UartRegDivFracVal);
    WRITE_MEM32(0x4001820CU, UartRegLcr0Val);
    WRITE_MEM32(0x40018210U, UartRegLcr1Val);
    WRITE_MEM32(0x40018214U, UartRegFcrVal);
    WRITE_MEM32(0x4001821CU, UartRegIerVal);
    WRITE_MEM32(0x40018258U, UartRegIdleVal);
    WRITE_MEM32(0x40018240U, UartRegDmaEnVal);

    WRITE_REG32(CKGEN->PERI_CLK_EN0, ClkEn);
    WRITE_REG32(CKGEN->PERI_CLK_MUX2, ClkMux);
    WRITE_REG32(CKGEN->CLK_DIV2, ClkDiv);
    WRITE_MEM32(0x40085148U, 0x00000000U);
    MODIFY_REG32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_SPLL_DIV2_Msk, CKGEN_CLK_DIV2_SPLL_DIV2_Pos, 0U);
    for (uint32 i = 0U; i < 100U; i++)
    {
        __NOP();
    }
    MODIFY_REG32(CKGEN->CLK_DIV2, CKGEN_CLK_DIV2_SPLL_DIV2_Msk, CKGEN_CLK_DIV2_SPLL_DIV2_Pos, 1U);
}
/*PRQA S 3006 -- */
#endif /* endif of AC7840X */

/**
 * @brief Set sys and bus clock division
 * @note Function ID: DES_CKGEN_API_108
 * @param[in] CfgPtr: The pointer to the Ckgen_SysClkCfgType structure
 * @return void
 */
static void Ckgen_Hal_SetSysAndBusDiv(const Ckgen_SysClkCfgType *CfgPtr)
{
    uint8 SysDiv = (uint8)Ckgen_Hal_DivToRegValue(CfgPtr->SysDiv);/* register sysclk division value */
    uint8 BusDiv = (uint8)Ckgen_Hal_DivToRegValue(CfgPtr->BusDiv);/* register bus clock division value */

    /* Disable lock bit field */
    Ckgen_Reg_EnableCTRLRegLock(FALSE);
    /* set run mode division */
    if (CKGEN_SYS_CLK_MODE_RUN == CfgPtr->Mode)
    {
        Ckgen_Reg_SetRunModeSysClkDiv(SysDiv);
        Ckgen_Reg_SetRunModeBusClkDiv(BusDiv);
    }
#if defined (AC7840X) || defined (AC7843X)
    else/* set vlpr mode division */
    {
        Ckgen_Reg_SetVlprModeSysClkDiv(SysDiv);
        Ckgen_Reg_SetVlprModeBusClkDiv(BusDiv);
    }
#endif /* endif of AC7843X AC7840X */
    /* Enable lock bit field */
    Ckgen_Reg_EnableCTRLRegLock(TRUE);
}

/**
 * @brief Check whether the configuration of sysclk is correct
 * @note Function ID: DES_CKGEN_API_107
 * @param[in] CfgPtr: The pointer to the Ckgen_SysClkCfgType structure
 * @return Hal_StatusType: Check success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
static Hal_StatusType Ckgen_Hal_CheckSysClkCfg(const Ckgen_SysClkCfgType *CfgPtr)
{
    uint32 TargetSysSrcFreq, TargetBusFreq, CfgSysDiv, CfgBusDiv;/* frequency and division value */
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */
    Ckgen_ClkIdType CfgClkSrc;/* configuration source clock */

    CfgSysDiv = CfgPtr->SysDiv;
    CfgBusDiv = CfgPtr->BusDiv;
    CfgClkSrc = CfgPtr->ClkSrc;
    /* if the sys div or bus div is not in range, return the result is error */
    if ((CfgSysDiv > CKGEN_BUS_SYSCLK_DIV_MAX) || (CfgSysDiv < CKGEN_BUS_SYSCLK_DIV_MIN) \
            || (CfgBusDiv > CKGEN_BUS_SYSCLK_DIV_MAX) || (CfgBusDiv < CKGEN_BUS_SYSCLK_DIV_MIN))
    {
        ReturnValue = STATUS_ERROR;
    }
    /* if there an error in the above conditions, there is no need to continue judging */
    if (STATUS_ERROR != ReturnValue)
    {
        TargetSysSrcFreq = Ckgen_Hal_GetClkSrcFreq(CfgClkSrc);
        /* if get sysclk src freq is correct, the result depends on subsequent judgments */
        if (0U != TargetSysSrcFreq)
        {
            /* sysclk src freq need no more than sys clock maximum, bus clk freq no more than bus clock maximum*/
            TargetBusFreq = TargetSysSrcFreq / (CfgSysDiv * CfgBusDiv);
            if ((TargetSysSrcFreq > CKGEN_SYS_CLK_FREQ_MAX) || (TargetBusFreq > CKGEN_BUS_CLK_FREQ_MAX))
            {
                ReturnValue = STATUS_ERROR;
            }
        }
        else/* if get sysclk src freq error, the result is error*/
        {
            ReturnValue = STATUS_ERROR;
        }
    }

    return ReturnValue;
}

/**
 * @brief Set sysclk no need to Check parameters range
 * @note Function ID: DES_CKGEN_API_106
 * @param[in] CfgPtr: The pointer to the Ckgen_SysClkCfgType structure
 * @return Hal_StatusType: Set success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
static Hal_StatusType Ckgen_Hal_SetSysClkNoCheck(const Ckgen_SysClkCfgType *CfgPtr)
{
    uint32 CurrentFlashFreq;
    uint32 TargetFlashFreq;
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
    Hal_StatusType ReturnValue;/* function return value */
#elif defined (AC7840E) /* endif of AC7840X AC7842X AC7843X */
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */
#endif /* endif of AC7840E */
    Ckgen_SysClkModeType ConfigSysClkMode;/* configuration clock mode */
#if defined (AC7840X)
    boolean SpllStatus = Spm_Reg_GetSPLLStatus();/* spll status */
#endif /* endif of AC7840X */
    Spm_PowerModeType CurrentPowerMode = Spm_Hal_GetCurrentPowerMode();/* current power mode */
    Ckgen_SysClkModeType CurrentRunningMode;/* configuration SysClk mode */
    ConfigSysClkMode = CfgPtr->Mode;

#if defined (AC7843X)
    TargetFlashFreq = Ckgen_Hal_GetClkSrcFreq(CfgPtr->ClkSrc) / CfgPtr->SysDiv / CfgPtr->BusDiv;
#elif defined (AC7840X) || defined (AC7840E) || defined (AC7842X)  /* endif of AC7843X */
    TargetFlashFreq = Ckgen_Hal_GetClkSrcFreq(CfgPtr->ClkSrc) / CfgPtr->SysDiv;
#endif /* endif of AC7842X AC7840X AC7840E */

    /* if current power mode is SPM_MODE_RUN, config mode is CKGEN_SYS_CLK_MODE_RUN */
    if (SPM_MODE_RUN == CurrentPowerMode)
    {
        CurrentRunningMode = CKGEN_SYS_CLK_MODE_RUN;
    }
#if defined (AC7840X) || defined (AC7843X)
    else if (SPM_MODE_VLPR == CurrentPowerMode)
    {
        CurrentRunningMode = CKGEN_SYS_CLK_MODE_VLPR;
    }
#endif /* endif of AC7843X AC7840X */
    else
    {
        CurrentRunningMode = CKGEN_SYS_CLK_MODE_MAX;
    }
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
    ReturnValue = Ckgen_Hal_FlashLockCtrl(FALSE);
#endif /* endif of AC7840E */
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
    if (STATUS_SUCCESS == ReturnValue) /*PRQA S 2991,2995 # AMCE_MCU_2991_2995: flash lock control can fail on target hardware. */
#endif /* endif of AC7840X AC7842X AC7843X */
    {
        /* convert HZ to MHZ*/
        TargetFlashFreq = TargetFlashFreq / 1000000U;
        /* currnet flash module clk freq, uint is MHZ */
        CurrentFlashFreq = Ckgen_Hal_GetFlashClkFreq();
        /* currnet flash clk freq no more than SysClkFreq and current configuration sysclk mode is euqal to Mode, \
            set flash clk freq first and value is equal to SysClkFreq */
        if (CurrentFlashFreq >= TargetFlashFreq)
        {
            Ckgen_Hal_SetSysClkSrc(ConfigSysClkMode, CfgPtr->ClkSrc);
            Ckgen_Hal_SetSysAndBusDiv(CfgPtr);
        }
        if ((CurrentRunningMode == ConfigSysClkMode))
        {
            Ckgen_Hal_SetFlashClkFreq((uint8)TargetFlashFreq);
        }
        /* currnet flash clk freq more than SysClkFreq and current configuration sysclk mode is euqal to Mode, \
            set flash clk freq last and value is equal to SysClkFreq */
        if (CurrentFlashFreq < TargetFlashFreq)
        {
            Ckgen_Hal_SetSysAndBusDiv(CfgPtr);
            Ckgen_Hal_SetSysClkSrc(ConfigSysClkMode, CfgPtr->ClkSrc);
        }
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
        ReturnValue = Ckgen_Hal_FlashLockCtrl(TRUE);
#endif /* endif of AC7840X AC7842X AC7843X */
    }
#if defined (AC7840X)
    /* if spll status is on and mode is run mode, do clk autotest */
    if ((CKGEN_SYS_CLK_MODE_RUN == ConfigSysClkMode) && (TRUE == SpllStatus))
    {
        Ckgen_Hal_AutoTest();
    }
#endif /* endif of AC7840X */

    return ReturnValue;
}

/**
 * @brief Set sysclk
 * @note Function ID: DES_CKGEN_API_003
 * @param[in] CfgPtr: The pointer to the Ckgen_SysClkCfgType structure
 * @return Hal_StatusType: Set success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_SetSysClk(const Ckgen_SysClkCfgType *CfgPtr)
{
    Hal_StatusType ReturnValue = STATUS_ERROR;/* function return value */

    DEVICE_ASSERT(NULL_PTR != CfgPtr);
    if (NULL_PTR != CfgPtr)
    {
        /* Wait for the sysclk clock soure to stabilize */
        ReturnValue = Ckgen_Hal_WaitClktoStability(CfgPtr->ClkSrc);
        DEVICE_ASSERT(STATUS_SUCCESS == ReturnValue);
        ReturnValue = Ckgen_Hal_CheckSysClkCfg(CfgPtr);
        DEVICE_ASSERT(STATUS_SUCCESS == ReturnValue);
        /* the parameters of sysclk is correct */
        if (STATUS_SUCCESS == ReturnValue)
        {
            ReturnValue = Ckgen_Hal_SetSysClkNoCheck(CfgPtr);
        }
    }

    return ReturnValue;
}

/**
 * @brief Get clock frequency
 * @note Function ID: DES_CKGEN_API_005
 * @param[in] Clk: clock id
 * @param[in] Freq: the pointer to the uint32 value
 * @return Hal_StatusType: Get success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_GetFreq(Ckgen_ClkIdType Clk, uint32 *Freq)
{
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */
    uint32 ClkFreq = 0U;/* clock frequency */

    DEVICE_ASSERT(NULL_PTR != Freq);
#if defined (AC7840X) || defined (AC7842X) || defined(AC7840E)
#if defined (AC7840X)
    if ((CKGEN_CAN0_TS_CLK <= Clk) && (Clk <= CKGEN_CAN3_TS_CLK))
#elif defined (AC7840E)
    /* the clock id rang is CKGEN_CAN0_TS_CLK ... CKGEN_CAN3_TS_CLK */
    if ((CKGEN_CAN0_TS_CLK <= Clk) && (Clk <= CKGEN_CAN1_TS_CLK))
#elif defined (AC7842X)  /* endif of AC7840X */
    /* the clock id rang is CKGEN_CAN0_TS_CLK ... CKGEN_CAN5_TS_CLK */
    if ((CKGEN_CAN0_TS_CLK <= Clk) && (Clk <= CKGEN_CAN5_TS_CLK))
#endif /* endif of AC7842X */
    {
        ClkFreq = Ckgen_Hal_GetCanTsClkSrcFreq(Clk);
    }
#endif /* endif of AC7840X AC7842X */
#if defined (AC7842X)  || defined (AC7840X) || defined (AC7840E)
    else if (Clk < CKGEN_CLK_OUT)
#elif defined (AC7843X) /* endif of AC7840X AC7842X */
    if (Clk < CKGEN_CLK_OUT)
#endif /* endif of AC7843X */
    {
        ClkFreq = Ckgen_Hal_GetPeriphClkFreq(Clk);
    }
    else if (CKGEN_CLK_OUT == Clk)
    {
        ClkFreq = Ckgen_Hal_GetClkoutClkFreq();
    }
    else if (CKGEN_LSI_CLK == Clk)
    {
        ClkFreq = Ckgen_Hal_GetLSIClkFreq();
    }
#if defined (AC7840X)
    else if (CKGEN_RTC_CLK == Clk)
    {
        ClkFreq = Ckgen_Hal_GetRTCClkFreq();
    }
#endif /* endif of AC7840X */
    else if ((CKGEN_SPLL_DIV1_CLK <= Clk) && (Clk < CKGEN_LSI_CLK))
    {
        ClkFreq = Ckgen_Hal_GetClkSrcDivFreq(Clk);
    }
    else if ((CKGEN_SYS_CLK <= Clk) && (Clk < CKGEN_HSE_CLK))
    {
        ClkFreq = Ckgen_Hal_GetSysAndBusFreq(Clk);
    }
    else if ((CKGEN_HSE_CLK <= Clk) && (Clk < CKGEN_OFF_CLK))
    {
        ClkFreq = Ckgen_Hal_GetCommClkSrcFreq(Clk);
    }
    else
    {
        /* nothing */
    }

    /* clock frequency is 0U, result is error */
    if (0U == ClkFreq)
    {
        ReturnValue = STATUS_ERROR;
    }

    if (NULL_PTR != Freq)
    {
        *Freq = ClkFreq;
    }
    else
    {
        ReturnValue = STATUS_ERROR;
    }

    return ReturnValue;
}

/**
 * @brief Get clock status
 * @note Function ID: DES_CKGEN_API_009
 * @param[in] Clk: clock id, the range is bus clock clkout and hse hsi vhsi spll
 * @return Hal_StatusType: clock status range is the STATUS_CLK_ON STATUS_CLK_OFF STATUS_CLK_STABLE STATUS_CLK_UNSTABLE
 */
Hal_StatusType Ckgen_Hal_GetClkStatus(Ckgen_ClkIdType Clk)
{
    Hal_StatusType ReturnValue;/* function return value */

    ReturnValue = Ckgen_Hal_GetClkSrcStatus(Clk);

    return ReturnValue;
}

#if CKGEN_ENABLE_CHECK_PARAM
/**
 * @brief Check whether the division of pll is correct
 * @note Function ID: DES_CKGEN_API_105
 * @param[in] CfgPtr: The pointer to the Ckgen_PllCfgType structure
 * @return Hal_StatusType: Check success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
static Hal_StatusType Ckgen_Hal_CheckPllClkDiv(const Ckgen_PllCfgType *CfgPtr)
{
    Hal_StatusType ReturnValue;/* function return value */
    uint32 PreDiv = CfgPtr->PreDiv;/* actual PreDiv value */
    uint32 PosDiv = CfgPtr->PosDiv;/* actual PosDiv value */
    uint32 FbkDiv = CfgPtr->FbkDiv;/* actual FbkDiv value */
    uint32 VCOFreq, PllInFreq, PllRefFreq;/* frequency value */

    /* SPLL posdiv valid values rang is 1 2 4 6 8 10 ....62 */
    if ((PosDiv < CKGEN_SPLL_POSDIV_MIN) || (PosDiv > CKGEN_SPLL_POSDIV_MAX))
    {
        ReturnValue = STATUS_ERROR;
    }
    else
    {
        if ((1U == PosDiv) || (0U == (PosDiv % 2U)))
        {
            ReturnValue = STATUS_SUCCESS;
        }
        else
        {
            ReturnValue = STATUS_ERROR;
        }
    }
    if (STATUS_SUCCESS == ReturnValue)
    {
        /* SPLL prediv valid values rang is 1 2 4 */
        ReturnValue = (((4U == PreDiv) || (2U == PreDiv) || (1U == PreDiv)) ? STATUS_SUCCESS : STATUS_ERROR);
        if (STATUS_SUCCESS == ReturnValue)
        {
            /* SPLL fbkdiv valid values rang is CKGEN_SPLL_FBKDIV_MIN CKGEN_SPLL_FBKDIV_MAX*/
            if ((FbkDiv < CKGEN_SPLL_FBKDIV_MIN) || (FbkDiv > CKGEN_SPLL_FBKDIV_MAX))
            {
                ReturnValue = STATUS_ERROR;
            }
        }
    }
    if (STATUS_SUCCESS == ReturnValue)
    {
        PllInFreq = Ckgen_Hal_GetPllInFreq();
        /* PllInFreq is 4MHZ ~ 48MHZ or Div value is incorrect, return directly */
        if ((PllInFreq >= CKGEN_SPLL_IN_CLK_FREQ_MIN) && (PllInFreq <= CKGEN_SPLL_IN_CLK_FREQ_MAX))
        {
            PllRefFreq = PllInFreq / PreDiv;
            if (PllRefFreq < CKGEN_SPLL_REF_FREQ_MAX)
            {
                ReturnValue = STATUS_ERROR;
            }
            else
            {
                VCOFreq = PllRefFreq * FbkDiv;
                /* VCOFreq is 0.5GHZ ~ 1.5GHZ, PllRefFreq no more than 12MHZ */
                if ((VCOFreq < CKGEN_SPLL_VCO_FREQ_MIN) || (VCOFreq > CKGEN_SPLL_VCO_FREQ_MAX))
                {
                    ReturnValue = STATUS_ERROR;
                }
                else
                {
                    ReturnValue = STATUS_SUCCESS;
                }
            }
        }
        else
        {
            ReturnValue = STATUS_ERROR;
        }
    }

    return ReturnValue;
}
#endif /* endif of CKGEN_ENABLE_CHECK_PARAM */

/**
 * @brief Set Pll clock no need to check Cfg parameters range
 * @note Function ID: DES_CKGEN_API_104
 * @param[in] CfgPtr: The pointer to the Ckgen_PllCfgType structure
 * @return void
 */
static void Ckgen_Hal_SetPllClkNoCheck(const Ckgen_PllCfgType *CfgPtr)
{
    uint32 PreDiv, PosDiv;
#if defined (AC7840E)
    uint32 PosDiv1;
#endif /* endif of AC7840E */
    uint32 EnableLD = (TRUE == CfgPtr->EnableLD) ? 1U : 0U;
#if defined (AC7840X)
    uint32 EnableSamp0LD = (TRUE == CfgPtr->EnableSamp0LD) ? 1U : 0U;
    uint32 EnableSamp1LD = (TRUE == CfgPtr->EnableSamp1LD) ? 1U : 0U;
#endif /* endif of AC7840X */
#if defined (AC7843X) || defined (AC7842X) || defined (AC7840E)
    volatile uint32 Timeout = 50U;
#endif /* endif of AC7843X */

    /* Ensure SPLL is disabled status */
    (void)Ckgen_Hal_EnableClkSrc(CKGEN_SPLL_CLK, FALSE);
#if defined (AC7840X) || defined (AC7843X) || defined (AC7840E)
    Ana_Reg_SetPllLockParam(0U, (uint32)ANA_SPLL_CFG1_LD_EN_Msk, (uint32)ANA_SPLL_CFG1_LD_EN_Pos);
#elif defined (AC7842X)  /* endif of AC7840X AC7843X AC7840E */
    Ana_Reg_SetPllLockParam(0U, (uint32)ANA_SPLL_CFG0_LD_EN_Msk, (uint32)ANA_SPLL_CFG0_LD_EN_Pos);
#endif /* endif of AC7842X */
    Ckgen_Reg_SetSPLLClkMux(CfgPtr->ClkSrc);
    /* modify CKGEN_SPLL_CLK after disable reg lock need to enable reg lock */
    PreDiv = Ckgen_Hal_PreDivToRegValue((uint32)CfgPtr->PreDiv);
    PosDiv = Ckgen_Hal_PosDivToRegValue((uint32)CfgPtr->PosDiv);
#if defined (AC7840E)
    PosDiv1 = Ckgen_Hal_PosDivToRegValue((uint32)CfgPtr->PosDiv1);
    Ana_Reg_SetPllPosDiv1Val(PosDiv1);
#endif /* endif of AC7840E */
    Ana_Reg_SetPllPreDivVal(PreDiv);
    Ana_Reg_SetPllPosDivVal(PosDiv);
    Ana_Reg_SetPllFbkDivVal(CfgPtr->FbkDiv);
#if defined (AC7840X) || defined (AC7843X) || defined (AC7840E)
    Ana_Reg_SetPllLockParam(EnableLD, (uint32)ANA_SPLL_CFG1_LD_EN_Msk, (uint32)ANA_SPLL_CFG1_LD_EN_Pos);
#elif defined (AC7842X)  /* endif of AC7840X AC7843X AC7840E */
    Ana_Reg_SetPllLockParam(EnableLD, (uint32)ANA_SPLL_CFG0_LD_EN_Msk, (uint32)ANA_SPLL_CFG0_LD_EN_Pos);
#endif /* endif of AC7842X */

#if defined (AC7840X)
    Ana_Reg_SetPllLockParam(EnableSamp0LD, (uint32)ANA_SPLL_CFG1_LD_SAMP0_EN_Msk, \
                            (uint32)ANA_SPLL_CFG1_LD_SAMP0_EN_Pos);
    Ana_Reg_SetPllLockParam(EnableSamp1LD, (uint32)ANA_SPLL_CFG1_LD_SAMP1_EN_Msk, \
                            (uint32)ANA_SPLL_CFG1_LD_SAMP1_EN_Pos);
#endif /* endif of AC7840X */
#if defined (AC7840X) || defined (AC7843X) || defined (AC7840E)
    Ana_Reg_SetPllLockParam((uint32)CfgPtr->DlySel, (uint32)ANA_SPLL_CFG1_LD_DLY_SEL_Msk, \
                            (uint32)ANA_SPLL_CFG1_LD_DLY_SEL_Pos);
#elif defined (AC7842X)  /* endif of AC7840X AC7843X AC7840E */
    Ana_Reg_SetPllLockParam((uint32)CfgPtr->DlySel, (uint32)ANA_SPLL_CFG0_LD_DLY_SEL_Msk, \
                            (uint32)ANA_SPLL_CFG0_LD_DLY_SEL_Pos);
#endif /* endif of AC7842X */
#if defined (AC7843X)
    Ckgen_Reg_SetAdcSpllDiv(Ckgen_Hal_DivToRegValue(CfgPtr->AdcSpllDiv));
#endif /* endif of AC7843X */
    Ckgen_Reg_SetSPLLDIV1ClkDiv(Ckgen_Hal_DivToRegValue(CfgPtr->OutputDiv1));
    Ckgen_Reg_SetSPLLDIV2ClkDiv(Ckgen_Hal_DivToRegValue(CfgPtr->OutputDiv2));
#if defined (AC7843X) || defined (AC7842X) || defined (AC7840E)
    while (Timeout > 0U)
    {
        Timeout = Timeout - 1U;
    }
#endif
    (void)Ckgen_Hal_EnableClkSrc(CKGEN_SPLL_CLK, CfgPtr->Enable);
}

/**
 * @brief Set Pll clock
 * @note Function ID: DES_CKGEN_API_103
 * @param[in] CfgPtr: The pointer to the Ckgen_PllCfgType structure
 * @return void
 */
static void Ckgen_Hal_SetPllClk(const Ckgen_PllCfgType *CfgPtr)
{
#if CKGEN_ENABLE_CHECK_PARAM
    Hal_StatusType ReturnValue;/* function return value */
#endif /* endif of CKGEN_ENABLE_CHECK_PARAM */
    DEVICE_ASSERT(NULL_PTR != CfgPtr);
#if CKGEN_ENABLE_CHECK_PARAM
    ReturnValue = Ckgen_Hal_CheckPllClkDiv(CfgPtr);
    DEVICE_ASSERT(STATUS_SUCCESS == ReturnValue);
#endif /* endif of CKGEN_ENABLE_CHECK_PARAM */
    Ckgen_Hal_SetPllClkNoCheck(CfgPtr);
}

/**
 * @brief Set clock source and division
 * @note Function ID: DES_CKGEN_API_102
 * @param[in] CfgPtr: The pointer to the Ckgen_ClkCfgType structure
 * @return void
 */
static void Ckgen_Hal_SetClkSrc(const Ckgen_ClkSrcCfgType *CfgPtr)
{
    DEVICE_ASSERT((CKGEN_HSI_CLK == CfgPtr->Clk) || (CKGEN_VHSI_CLK == CfgPtr->Clk));

    if (CKGEN_HSI_CLK == CfgPtr->Clk)
    {
        Ckgen_Reg_SetHSIDIV1ClkDiv(Ckgen_Hal_DivToRegValue(CfgPtr->OutputDiv1));
        Ckgen_Reg_SetHSIDIV2ClkDiv(Ckgen_Hal_DivToRegValue(CfgPtr->OutputDiv2));
#if defined (AC7840X) || defined (AC7843X)
        Spm_Reg_EnableHSIInVLPS(CfgPtr->EnableInVLPS);
#endif /* endif of AC7840X AC7843X */
    }
    else
    {
        Ckgen_Reg_SetVHSIDIV1ClkDiv(Ckgen_Hal_DivToRegValue(CfgPtr->OutputDiv1));
        Ckgen_Reg_SetVHSIDIV2ClkDiv(Ckgen_Hal_DivToRegValue(CfgPtr->OutputDiv2));
#if defined (AC7842X)
        Spm_Reg_EnableVHSIInVLPS(CfgPtr->EnableInVLPS);
#endif /* endif of AC7842X */
    }
    (void)Ckgen_Hal_EnableClkSrc(CfgPtr->Clk, CfgPtr->Enable);
}

/**
 * @brief Set XOSC
 * @note Function ID: DES_CKGEN_API_101
 * @param[in] CfgPtr: The pointer to the Ckgen_XoscClkCfgType structure
 * @return Hal_StatusType: set xosc success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
static Hal_StatusType Ckgen_Hal_SetXosc(const Ckgen_XoscClkCfgType *CfgPtr)
{
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */
    DEVICE_ASSERT(!((FALSE == CfgPtr->Enable) && (TRUE == CfgPtr->EnableMonitor)));

    if (TRUE == CfgPtr->Enable)/* Wait for the sysclk clock soure to stabilize */
    {
        Spm_Reg_EnableXOSCBypassMode(CfgPtr->EnableBypass);
        Spm_Reg_EnableXOSC(CfgPtr->Enable);
        if (TRUE == CfgPtr->EnableMonitor)
        {
            ReturnValue = Ckgen_Hal_WaitClktoStability(CKGEN_HSE_CLK);
            DEVICE_ASSERT(STATUS_SUCCESS == ReturnValue);
        }
        /* clock already stabilize */
        if (STATUS_SUCCESS == ReturnValue)
        {
            /* Disable lock bit field */
            Ckgen_Reg_EnableCTRLRegLock(FALSE);
            Ckgen_Reg_EnableXOSCMonitor(CfgPtr->EnableMonitor);
            /* Enable lock bit field */
            Ckgen_Reg_EnableCTRLRegLock(TRUE);
            HseFreq = CfgPtr->Freq;
            Ckgen_Reg_SetHSEDIV1ClkDiv(Ckgen_Hal_DivToRegValue(CfgPtr->OutputDiv1));
            Ckgen_Reg_SetHSEDIV2ClkDiv(Ckgen_Hal_DivToRegValue(CfgPtr->OutputDiv2));
        }
    }
    else
    {
        /* Disable lock bit field */
        Ckgen_Reg_EnableCTRLRegLock(FALSE);
        Ckgen_Reg_EnableXOSCMonitor(CfgPtr->EnableMonitor);
        /* Enable lock bit field */
        Ckgen_Reg_EnableCTRLRegLock(TRUE);
        Spm_Reg_EnableXOSCBypassMode(CfgPtr->EnableBypass);
        Spm_Reg_EnableXOSC(CfgPtr->Enable);
        HseFreq = 0U;
    }

    return ReturnValue;
}

/**
 * @brief Set low power clock source
 * @note Function ID: DES_CKGEN_API_100
 * @param[in] LPCfg: the pointer to the Ckgen_LPClkCfgType structure
 * @return Hal_StatusType: Set success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
static void Ckgen_Hal_SetLpClkSrc(const Ckgen_LPClkCfgType *LPCfg)
{
    uint32 MuxVal;
#if defined (AC7843X) || defined (AC7842X) || defined (AC7840E)
    MuxVal = Ckgen_Hal_LSIClkToMuxVal(LPCfg->LSIClkSrc);
#elif defined (AC7840X) /* endif of AC7842X AC7843X AC7840E */
    MuxVal = Ckgen_Hal_LSIClkToMuxVal(LPCfg->LSIClkSrc) | (Ckgen_Hal_RTCClkToMuxVal(LPCfg->RTCClkSrc) << 2U);
#endif /* endif of AC7840X */
    Ckgen_Reg_SetLPMux(MuxVal);
}

/**
 * @brief Wait clock to Stable and distribute system and module clock
 * @note Function ID: DES_CKGEN_API_002
 * @param[in] CfgPtr: the pointer to the Ckgen_ClkDistributeCfgType structure
 * @return Hal_StatusType: Distribute success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */
Hal_StatusType Ckgen_Hal_DistributeClk(const Ckgen_ClkDistributeCfgType *CfgPtr)
{

    uint8 Idx;/* used for loop*/
    Hal_StatusType ReturnValue = STATUS_SUCCESS;/* function return value */

    if (NULL_PTR != CfgPtr)
    {
        /* Step1: Initialize SysClk clock */
        for (Idx = 0; Idx < CfgPtr->SysClkCfgCnt; Idx++)
        {
            /* Enabe the clock soure of sysclk */
            ReturnValue = Ckgen_Hal_EnableClkSrc(CfgPtr->SysClkCfgs[Idx].ClkSrc, TRUE);
            if (STATUS_ERROR == ReturnValue)
            {
                break;
            }
            else
            {
                ReturnValue = Ckgen_Hal_SetSysClk(&CfgPtr->SysClkCfgs[Idx]);
            }
        }
        /* Step2: Initialize modlue mux clock */
        for (Idx = 0; Idx < CfgPtr->ClkMuxCfgCnt; Idx++)
        {
            (void)Ckgen_Hal_SetPeriphClkMux(CfgPtr->ClkMuxCfgs[Idx].Clk, CfgPtr->ClkMuxCfgs[Idx].ClkSrc);
        }
        /* Step3: Initialize modlue div */
        for (Idx = 0; Idx < CfgPtr->ClkDivCfgCnt; Idx++)
        {
            (void)Ckgen_Hal_SetPeriphClkDiv(CfgPtr->ClkDivCfgs[Idx].Clk, CfgPtr->ClkDivCfgs[Idx].Div);
        }
        /* Step4: Initialize clkout */
        if (NULL_PTR != CfgPtr->ClkoutCfg)
        {
            Ckgen_Reg_SetClkoutClkMux(CfgPtr->ClkoutCfg->ClkSrc);
            Ckgen_Reg_SetClkoutClkDiv((uint32)(CfgPtr->ClkoutCfg->Div));
            Ckgen_Reg_EnableClkout((TRUE == CfgPtr->ClkoutCfg->Enable) ? 1U : 0U);
        }
    }
    else
    {
        ReturnValue = STATUS_ERROR;
    }

    return ReturnValue;
}

/**
 * @brief Initialize clocks source
 * @note Function ID: DES_CKGEN_API_001
 * @param[in] CfgPtr: the pointer to the Ckgen_AllClkCfgType structure
 * @return Hal_StatusType: Distribute success or not, the range is the STATUS_SUCCESS STATUS_ERROR
 */

Hal_StatusType Ckgen_Hal_InitClk(const Ckgen_ClkTreeCfgType *CfgPtr)
{
    uint8 Idx;/* used for loop */
    Hal_StatusType ReturnValue = STATUS_ERROR;/* function return value */

    DEVICE_ASSERT(NULL_PTR != CfgPtr);
    if (NULL_PTR != CfgPtr)
    {
        /* Step1: Initialize the XOSC */
        if (NULL_PTR != CfgPtr->XoscCfg)
        {
            ReturnValue = Ckgen_Hal_SetXosc(CfgPtr->XoscCfg);
        }
        if (STATUS_SUCCESS == ReturnValue)
        {
            /* Step2: Initialize the clock source,such as HSI VHSI HSE */
            DEVICE_ASSERT(NULL_PTR != CfgPtr->ClkSrcCfgs);
            if (NULL_PTR != CfgPtr->ClkSrcCfgs)
            {
                for (Idx = 0; Idx < CfgPtr->ClkSrcCfgCnt; Idx++)
                {
                    Ckgen_Hal_SetClkSrc(&CfgPtr->ClkSrcCfgs[Idx]);
                }
            }
            /* Step3: Initialize the pll clock */
            DEVICE_ASSERT(NULL_PTR != CfgPtr->pllCfg);
            if (NULL_PTR != CfgPtr->pllCfg)
            {
                if (TRUE == CfgPtr->pllCfg->Enable) /* enable spll need wait clock source is stable */
                {
                    /* Wait for the clk clock soure to stabilize */
                    ReturnValue = Ckgen_Hal_WaitClktoStability(CfgPtr->pllCfg->ClkSrc);
                    DEVICE_ASSERT(STATUS_SUCCESS == ReturnValue);
                }
                /* Step4: can set pll clock */
                if (STATUS_SUCCESS == ReturnValue)
                {
                    Ckgen_Hal_SetPllClk(CfgPtr->pllCfg);
                }
                /* Step5:  set low power clock */
                if (NULL_PTR != CfgPtr->LPCfg)
                {
                    Ckgen_Hal_SetLpClkSrc(CfgPtr->LPCfg);
                }
                /* Step5:  set external clock freq */
                if (NULL_PTR != CfgPtr->ExtClkCfg)
                {
                    RtcClkInFreq = CfgPtr->ExtClkCfg->RtcClkInFreq;
                    PwmExtClk0Freq = CfgPtr->ExtClkCfg->PwmExtClk0Freq;
                    PwmExtClk1Freq = CfgPtr->ExtClkCfg->PwmExtClk1Freq;
                    PwmExtClk2Freq = CfgPtr->ExtClkCfg->PwmExtClk2Freq;
                }
            }
            else
            {
                ReturnValue = STATUS_ERROR;
            }
        }
    }

    return ReturnValue;
}
/* =============================================  EOF  ============================================== */
