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
*   @file  OsIf.c
*
*   @brief This file provides extern OsIf API implement.
*/

/*==============================================INCLUDE FILES=======================================*/
#include "OsIf.h"
#include "OsIf_Irq.h"
#include "OsIf_Time.h"
#include "Ckgen_Hal.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/** @brief systick irq period. For baremetal system,the systick should so slow to reduce cpu occupy*/
#define OSIF_SYSTICK_INTERVAL_MS (50U)

/** @brief one second is 1000000 us*/
#define OSIF_SEC_TO_US (1000000U)

/** @brief one second is 1000 ms*/
#define OSIF_SEC_TO_MS (1000U)
/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/** @brief current cope clock frequency */
static uint32 OsIf_CoreClockFreq;

/** @brief configure systick reload register value */
static uint32 OsIf_CycleCount;

/** @brief primask value */
volatile uint32 OsIf_PriMaskValue = 0U;
/** @brief nested level value */
volatile sint32 OsIf_CriticalNesting = 0;

/*============================================FUNCTION PROTOTYPES===================================*/

/*========================================GLOBAL FUNCTIONS==========================================*/
#if (OS_PLATFORM == OSIF_BAREMETAL)
/**
 * @brief Initialize Systick
 * @note Function ID: DES_OSIF_API_212
 * @param[in] CycleCount: systick reload register value
 * @return void
 */
static void SysTick_Init(uint32 CycleCount)
{
    MODIFY_REG32(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk, SysTick_CTRL_ENABLE_Pos, 0U);
    /*set one systick cycle period to CycleCount counter*/
    WRITE_REG32(SysTick->LOAD, (CycleCount & SysTick_LOAD_RELOAD_Msk) - 1U);
    WRITE_REG32(SysTick->VAL, 0x0U);
    MODIFY_REG32(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk, \
                 0U, 0x5U);
}

/**
 * @brief Deinitialize Systick
 * @note Function ID: DES_OSIF_API_213
 * @return void
 */
LOCAL_INLINE void SysTick_DeInit(void)
{
    MODIFY_REG32(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk, SysTick_CTRL_ENABLE_Pos, 0U);
}
#else
/**
 * @brief Get systick reload value.
 * @note Function ID: DES_OSIF_API_215
 * @return Systick reload value
 */
LOCAL_INLINE uint32 SysTick_GetReLoadValue(void)
{
    return READ_REG32(SysTick->LOAD);
}
#endif /* endif of OS_PLATFORM == OSIF_BAREMETAL */

/**
 * @brief Get systick current value.
 * @note Function ID: DES_OSIF_API_214
 * @return Systick current value
 */
LOCAL_INLINE uint32 SysTick_GetCurrTicks(void)
{
    return READ_REG32(SysTick->VAL);
}

/**
 * @brief Converts a value from microsecond units to ticks units.
 * @note Function ID: DES_OSIF_API_211
 * @param[in] Micros:  microseconds value (multiple of 1000 can be convert to tick)
 * @return uint32: ticks value
 */
LOCAL_INLINE uint32 OsIf_InternalMicrosToTicks(uint32 Micros)
{
    /* core clock is multiple MHZ*/
    uint32 Ticks = Micros * (OsIf_CoreClockFreq / OSIF_SEC_TO_US);/* tick value */

    return Ticks;
}

/**
 * @brief Get the delta time in ticks compared to a reference, and updates the reference.
 * @note Function ID: DES_OSIF_API_210
 * @param[inout] CurrentRef:  reference counter value, updated to current counter value
 * @return The elapsed time
 */
static uint32 OsIf_InternalGetElapsed(uint32 *const CurrentRef)
{
    uint32 Elapsed;/* Elapsed tick value */
    uint32 CurrentVal = SysTick_GetCurrTicks();/* current tick value */

    /*check CurrentVal whether overflow*/
    if (CurrentVal >= *CurrentRef)/*overflow occurred*/
    {
        Elapsed = OsIf_CycleCount - (CurrentVal - *CurrentRef);
    }
    else/* no overflow occurred*/
    {
        Elapsed = *CurrentRef - CurrentVal;
    }
    *CurrentRef = CurrentVal;

    return Elapsed;
}

/**
 * @brief Initialize osif
 * @note Function ID: DES_OSIF_API_208
 * @return void
 */
void OsIf_Init(void)
{
    uint32 CoreClk;/* core clock frequency */
    Hal_StatusType Status;/* Ckgen_Hal_GetFreq return  value */

    /*get the current core clk*/
    Status = Ckgen_Hal_GetFreq(CKGEN_CORE_CLK, &CoreClk);
    /*if system clock changed, so reconfig systick*/
    if ((Status == STATUS_SUCCESS) && (CoreClk != OsIf_CoreClockFreq))
    {
        OsIf_CoreClockFreq = CoreClk;
#if (OS_PLATFORM == OSIF_BAREMETAL)
        OsIf_CycleCount = OsIf_CoreClockFreq / (OSIF_SEC_TO_MS / OSIF_SYSTICK_INTERVAL_MS);
        /*OsIf_CycleCount not more than 0xFFFFFF*/
        SysTick_Init(OsIf_CycleCount);
#else
        OsIf_CycleCount = SysTick_GetReLoadValue() + 1U;
#endif /* endif of OS_PLATFORM == OSIF_BAREMETAL */
    }
}

/**
 * @brief Deinitialize osif
 * @note Function ID: DES_OSIF_API_209
 * @return void
 */
void OsIf_Deinit(void)
{
#if (OS_PLATFORM == OSIF_BAREMETAL)
    SysTick_DeInit();
#endif  /* endif of OS_PLATFORM == OSIF_BAREMETAL */
    OsIf_CycleCount = 0U;
    OsIf_CoreClockFreq = 0U;
}

/**
 * @brief Get the current value of the counter.
 * @note Function ID: DES_OSIF_API_207
 * @return The current value of the counter
 */
uint32 OsIf_GetCounter(void)
{
    uint32 Counter = 0U;/* current tick value */
    if (SysTick_CTRL_ENABLE_Msk == (READ_REG32(SysTick->CTRL) & SysTick_CTRL_ENABLE_Msk))
    {
        if (0U != OsIf_CoreClockFreq)/* CoreClockFreq is not zero, osif be inited */
        {
            Counter = SysTick_GetCurrTicks();
        }
    }
    else
    {
        Counter = 1U;
    }
    return Counter;
}

/**
 * @brief Get the delta time in ticks compared to a reference, and updates the reference.
 * @note Function ID: DES_OSIF_API_206
 * @param[inout] CurrentRef:  reference counter value, updated to current counter value
 * @return The elapsed time
 */
uint32 OsIf_GetElapsed(uint32 *const CurrentRef)
{
    uint32 Elapsed = 0U;/* elapsed tick value */
    if (SysTick_CTRL_ENABLE_Msk == (READ_REG32(SysTick->CTRL) & SysTick_CTRL_ENABLE_Msk))
    {
        /*check pointer whether is null and CoreClockFreq is not zero*/
        if ((NULL_PTR != CurrentRef) && (0UL != OsIf_CoreClockFreq))
        {
            Elapsed = OsIf_InternalGetElapsed(CurrentRef);
        }
    }
    else
    {
    /*this value is for systick disable ,not effect origin timeout logic*/
#if defined(COMPILER_ARM_CLANG)
        Elapsed = 20U;
#elif  defined(COMPILER_IAR)
        Elapsed = 27U;
#elif  defined(COMPILER_GCC) || defined(COMPILER_GHS)
        Elapsed = 17U;
#else
        Elapsed = 20U;
#endif
    }
    return Elapsed;
}

/**
 * @brief Microseconds delay.
 * @note Function ID: DES_OSIF_API_205
 * @param[in] Micros:  microseconds value
 * @return void
 */
void OsIf_UDelay(uint32 Micros)
{
    uint32 ExpectedTicks = OsIf_InternalMicrosToTicks(Micros);/* expected tick value */
    uint32 CurrentTicks = SysTick_GetCurrTicks();/* current tick value */
    uint32 ElapsedTicks = 0U;/* elapsed tick value */
    if (SysTick_CTRL_ENABLE_Msk == (READ_REG32(SysTick->CTRL) & SysTick_CTRL_ENABLE_Msk))
    {
        if (0UL != OsIf_CoreClockFreq)/* CoreClockFreq is not zero, osif be inited */
        {
            while (ElapsedTicks < ExpectedTicks)/* accumulate to expected value */
            {
                ElapsedTicks += OsIf_InternalGetElapsed(&CurrentTicks);
            }
        }
    }
    else
    {
#if defined(COMPILER_ARM_CLANG)
       /*one for means 3 instruction, so divide 3*/
       uint32 Cycle = 3U;
#elif  defined(COMPILER_IAR)
       uint32 Cycle = 8U;
#elif defined(COMPILER_GCC) || defined(COMPILER_GHS)
       uint32 Cycle = 5U;
#else
       uint32 Cycle = 3U;
#endif
        /*for cycle for delay when systick not on*/
        uint32 Count = Micros * (OsIf_CoreClockFreq / 1000000U / Cycle);
        for (uint32 i = 0U; i < Count; i++)
        {
            __NOP();
        }
    }
}

/**
 * @brief Converts a value from microsecond units to ticks units.
 * @note Function ID: DES_OSIF_API_204
 * @param[in] Micros:  microseconds value (multiple of 1000 can be convert to tick)
 * @return uint32: ticks value
 */
uint32 OsIf_MicrosToTicks(uint32 Micros)
{
    uint32 Ticks = 0U;/* tick value */
    /* CoreClockFreq is not zero, osif be inited */
    if (0U != OsIf_CoreClockFreq)
    {
        Ticks = OsIf_InternalMicrosToTicks(Micros);
    }
    return Ticks;
}

/**
 * @brief get current cpu id
 * @note Function ID: DES_OSIF_API_201
 * @return uint16: current cpu id
 */
uint16 OsIf_GetCoreId(void)
{
    return 0U;
}

