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
 * @file System_AC784xx.c
 *
 * @brief This file provides system clock config integration functions.
 *
 */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Ckgen_Hal.h"
#include "OsIf_Critical.h"

/* ============================================  DEFINES AND MACROS  ============================================ */
/** @brief Sram ECC read enable. */
#define SRAM_ECC_READ_ENABLE     (1U)

/** @brief Sram ECC 2bit error reset enable. */
#define SRAM_ECC_ERR_RST_ENABLE  (0U)

#ifndef WDG_DISABLE
/** @brief Watchdog disable.one means disable, zero means enable */
#define WDG_DISABLE              (1U)
#endif

/** @brief ICache disable */
#define ICACHE_DISABLE            (0U)

/** @brief ICache line 16byte (default is 32byte) */
#define ICACHE_LINE_16BYTE        (1U)

/** @brief DCache disable */
#define DCACHE_DISABLE            (1U)

/** @brief Don't initialize sram after system reset. */
#define NOT_INIT_SRAM_AFTER_RESET (1U)

/*!< Don't initialize sram in user code */
#ifndef NOT_INIT_SRAM
#if defined(AC7840X) || defined(AC7840E) || defined(AC7842X)
  #define NOT_INIT_SRAM           (0U)
#else
  #define NOT_INIT_SRAM           (1U)
#endif
#endif

/** @brief Default system clock. */
#define DEFAULT_SYSTEM_CLOCK     (48000000U)

/** @brief Standby retention base address. */
#define SRAM_RETENTION_BASE      (SRAM_U_BASE - SRAM_RETENTION_SIZE)

#ifndef VECTOR_TABLE_SRAM_ADDR
#if defined(AC7840X) ||  defined(AC7842X)
#define VECTOR_TABLE_SRAM_ADDR (0x2000FC00U)
#elif defined(AC7840E)
#define VECTOR_TABLE_SRAM_ADDR (0x20007C00U)
#elif defined(AC7843X)
#define VECTOR_TABLE_SRAM_ADDR (0x2003FC00U)
#endif
#endif
/* ============================================= TYPEDEFS ================================================ */
/**
 * @brief The type used to copy from source to destination.
 */
typedef struct
{
    uint8 *DstStart; /**< Start address of destination */
    uint8 *SrcStart; /**< Start address of source */
    uint8 *SrcEnd; /**< End address of source */
} Sys_CopyType;

/**
 * @brief The type used to zero initialization.
 */
typedef struct
{
    uint8 *DstStart; /**< Start address of destination */
    uint8 *DstEnd; /**< End address of destination */
} Sys_ZeroType;

/* =========================================== LOCAL VARIABLES ============================================== */
/** @brief Externals declaration. */
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
extern uint32 __Vectors;
#endif

static FlashDeviceType g_dFlashDevice = NON_DEV;
/** @brief Core clock. */
uint32 SystemCoreClock = DEFAULT_SYSTEM_CLOCK;    //PRQA S 1514 # AMCE_SYS_1514

#if defined (COMPILER_GCC) || defined (COMPILER_GHS)
/*PRQA S 3684 ++ # AMCE_SYS_3684*/
extern uint32 __DATA_ROM[];
extern uint32 __DATA_RAM[];
extern uint32 __DATA_END[];

extern uint32 __CODE_RAM[];
extern uint32 __CODE_ROM[];
extern uint32 __CODE_END[];

extern uint32 __BSS_START[];
extern uint32 __BSS_END[];
/*PRQA S 3684 -- # AMCE_SYS_3684*/

/** @brief Copy to RAM table. */
/*PRQA S 3218 ++ # AMCE_SYS_3218*/
static const Sys_CopyType SystemCopyTable[] =
{
    {(uint8 *)__DATA_RAM, (uint8 *)__DATA_ROM, (uint8 *)__DATA_END},
    {(uint8 *)__CODE_RAM, (uint8 *)__CODE_ROM, (uint8 *)__CODE_END},
};

/** @brief Zero RAM table. */
static const Sys_ZeroType SystemZeroTable[] =
{
    {(uint8 *)__BSS_START, (uint8 *)__BSS_END},
};
#else

/** @brief Copy to RAM table. */
static const Sys_CopyType SystemCopyTable[] =
{
    {(uint8 *)0U, (uint8 *)0U, (uint8 *)0U},
};

/** @brief Zero RAM table. */
static const Sys_ZeroType SystemZeroTable[] =
{
    {(uint8 *)0U, (uint8 *)0U},
};
#endif
/*PRQA S 3218 -- # AMCE_SYS_3218*/

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* ======================================  Functions define  ======================================== */
#ifdef ATC_DEVICE_ASSERT
extern void Debug_Printf(const char *format, ...);

/** @brief for circular to waiting debug.
 * @param[in] File: assert file name.
 * @param[in] Line: assert line in file.
 * @return void
 */
__attribute__((weak)) void WaitingForDebug(const char *File, uint32 Line)
{
    Debug_Printf("[ASSERT]Detect error on %s %ld.\r\n", File, Line);
    for (;;) {}; //PRQA S 2880 # AMCE_SYS_2880
}
#endif

/**
 * @brief Update system core clock.
 * @note  Function ID: DES_BOOT_API_001
 * @return void
 */
void SystemCoreClockUpdate(void)
{
    (void)Ckgen_Hal_GetFreq(CKGEN_CORE_CLK, &SystemCoreClock);
}

#if defined(AC7843X)
static void SystemWaitFwIdle(void)       //PRQA S 1505 # AMCE_SYS_1505
{
    while (TRUE)
    {
        /*check hsm fw whether updating, if updating will wait*/
        if (0U == READ_BIT(HSM_BOOTROM->HSM_STATUS1, 4U))
        {
            break;
        }
    }
}
#endif
/**
 * @brief Setup the microcontroller system. Initialize the System.
 * @note  Function ID: DES_BOOT_API_000
 * @return void
 */
void SystemInit(void)
{
    /* Disable watchdog */
#if (WDG_DISABLE)
    WDG->CNT = WDG_UNLOCK_FIRST_VALUE;
    WDG->CNT = WDG_UNLOCK_SECOND_VALUE;
    WDG->CS0 &= ~WDG_CS0_EN_Msk;
#endif
#if defined (AC7840X) || defined (AC7840E) || defined (AC7842X)
#if defined (AC7840E) || defined (AC7842X)
    uint32 NoInitSram = (CKGEN->RCM_EN & CKGEN_RCM_EN_NOT_INIT_SRAM_Msk);
#endif
    /* Sram L&U ECC read enable */
#if (SRAM_ECC_READ_ENABLE)
    MCM->MLMDR0 |= MCM_MLMDR0_LREEN_Msk | MCM_MLMDR0_UREEN_Msk;
#endif  /*SRAM_ECC_READ_ENABLE*/
    /* Sram ECC 2bit error reset disable */
#if (SRAM_ECC_ERR_RST_ENABLE)
    CKGEN->RCM_EN = 0x000007EFU;
#else
    CKGEN->RCM_EN = 0x000003EFU;
#endif
#if defined (AC7840E) || defined (AC7842X)
    CKGEN->RCM_EN |= NoInitSram;
#endif
    CKGEN->RCM_CTRL = 0x83000005U;
#elif defined (AC7843X)
#if (ICACHE_DISABLE || ICACHE_LINE_16BYTE || DCACHE_DISABLE)
    uint32 en = CKGEN->RCM_EN;
    CKGEN->RCM_EN = en & 0xFFFFFC10U;
#endif
    /* Disable iCache */
#if (ICACHE_DISABLE || ICACHE_LINE_16BYTE)
    MCM->ICACHE_CFG &= ~MCM_ICACHE_CFG_EN_Msk;
    while ((MCM->ICACHE_CFG & MCM_ICACHE_CFG_CLRDONE_Msk) == 0U)
    {
    }
#endif
    /* Set iCache line to 16byte */
#if (!ICACHE_DISABLE && ICACHE_LINE_16BYTE)
    MCM->CPUCACHE_PROT = 0x78430001U;
    MCM->ICACHE_CFG = 0x28100010U;
    MCM->ICACHE_CFG |= MCM_ICACHE_CFG_EN_Msk;
#endif
    /* Disable dCache */
#if DCACHE_DISABLE
    MCM->DCACHE_CFG &= ~MCM_DCACHE_CFG_EN_Msk;
#endif
    /*RCM_EN has been clear before, so can use '|' in here*/
    CKGEN->RCM_EN |= 0x000003EFU;
    CKGEN->RCM_CTRL = 0x83000005U;
#endif   /*AC7840X or AC7840E or AC7842X or AC7843X*/
    /* Set CP10 and CP11 Full Access */
#if (__FPU_PRESENT == 1U) && (__FPU_USED == 1U)
    SCB->CPACR |= 0xF00000U;
    __ISB(); /* ISB required after CPACR write (ARM DDI0403E: B3.2.1) */
#endif
    /* Relocate vector table */
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
#ifdef VECT_TAB_SRAM
    SCB->VTOR = VECTOR_TABLE_SRAM_ADDR;
#else
    SCB->VTOR = (uint32)&__Vectors;
#endif  /*VECT_TAB_SRAM*/
#endif  /*__VTOR_PRESENT */
#if defined (AC7843X)
    /* Check SPLL status */
    if ((SPM->PWR_MGR_CFG1 & (SPM_PWR_MGR_CFG1_SPLL_RDY_Msk | SPM_PWR_MGR_CFG1_SPLL_EN_Msk)) \
            == SPM_PWR_MGR_CFG1_SPLL_RDY_Msk)
    {
        SPM->PWR_MGR_CFG1 |= SPM_PWR_MGR_CFG1_SPLL_EN_Msk;
        SPM->PWR_MGR_CFG1 &= ~SPM_PWR_MGR_CFG1_SPLL_EN_Msk;
    }
#endif  /*AC7843X*/
}

/*!
 * @brief Initialize 8 byte aligned Mem from [Start] to [End] with Zero.
 * @note  Function ID: DES_BOOT_API_003
 * @param[in] Start: Mem start address.
 * @param[in] End: Mem end address.
 * @return void
 */
static void SystemMemZero_Aligned8(uint64 *Start, const uint64 *End)
{
    volatile uint64 *d = (volatile uint64 *)Start; /* Avoid optimized into build-in function. */
#if defined (COMPILER_GCC) || defined (COMPILER_GHS)
    /*enable fpu for gcc compilrer optimized*/
#if (__FPU_PRESENT == 1U) && (__FPU_USED == 1U)
    SCB->CPACR |= 0xF00000U;
    __ISB(); /* ISB required: ensure CPACR visible to pipeline before first FPU insn */
#endif
#endif
    while (d < End)
    {
        *d = 0x0UL;
        d++;
    }
#if defined (COMPILER_GCC) || defined (COMPILER_GHS)
    /*restore fpu old status*/
#if (__FPU_PRESENT == 1U) && (__FPU_USED == 1U)
    SCB->CPACR &= ~0xF00000U;
    __ISB(); /* ISB required: ensure CPACR visible to pipeline before first FPU insn */
#endif
#endif
}

/*!
 * @brief Initialize Mem from [Start] to [End] with Zero.
 * @note  Function ID: DES_BOOT_API_004
 * @param[in] Start: Mem start address.
 * @param[in] End: Mem end address.
 * @return void
 */
static void SystemMemZero(uint8 *Start, const uint8 *End)
{
    uint8 *d = Start;
    uint8 *rem = (uint8 *)((uint32)End - ((uint32)End % 8U));
    /* Zero first unaligned bytes. */
    while ((((uint32)d % 8U) != 0U) && (d < End))
    {
        *d = 0x00U;
        d++;
    }
    /* Zero align bytes. */
    /*PRQA S 3305,0310 ++ # AMCE_SYS_3305,AMCE_SYS_0310*/
    SystemMemZero_Aligned8((uint64 *)d, (uint64 *)rem);
    /*PRQA S 3305,0310 -- # AMCE_SYS_3305,AMCE_SYS_0310*/
    /* Zero remaining unaligned bytes. */
    d = rem;
    while (d < End)
    {
        *d = 0x00U;
        d++;
    }
}

/*!
 * @brief Copy Mem to [DstStart] from [SrcStart] to [SrcEnd].
 * @note  Function ID: DES_BOOT_API_005
 * @param[in] DstStart: Destination mem start address.
 * @param[in] SrcStart: Source mem end address.
 * @param[in] SrcEnd: Source mem value.
 * @return void
 */
static void SystemMemCpy(uint8 *DstStart, const uint8 *SrcStart, const uint8 *SrcEnd)
{
    const uint8 *s = SrcStart;
    uint8 *d = DstStart;
    while (s < SrcEnd)
    {
        *d = *s;
        d++;
        s++;
    }
}

/*PRQA S 2995 ++ # AMCE_SYS_2995 */
/*!
 * @brief Initialize the SRAM to 0 for ECC.
 * @note  Function ID: DES_BOOT_API_006
 * @param[in] StackBase: Main stack start address.
 * @param[in] StackEnd:  Main stack end address.
 * @return void
 */
static void SystemInitAllRam(uint32 StackBase, uint32 StackEnd)
{
#if defined(AC7843X)
    /*if hsm fw updating,will wait*/
    SystemWaitFwIdle();
#endif
    /* Not init RAM when system reset by check boot reason is POR| LVR | standby wakeup,
     * or CKGEN_RCM_EN_NOT_INIT_SRAM_Msk.
     */
#if (!NOT_INIT_SRAM)
#if defined (AC7840X)
    if ((0U != (CKGEN->RCM_STATUS & (CKGEN_RCM_STATUS_POR_RST_FLAG_Msk | CKGEN_RCM_STATUS_LVR_RST_FLAG_Msk)))
            || (0U != (SPM->STB_WP_STATUS & 0x1FFFFU)))
#elif  defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
    if ((0U == (CKGEN->RCM_EN & CKGEN_RCM_EN_NOT_INIT_SRAM_Msk))
            || (0U != (SPM->STB_WP_STATUS & 0x1FFFFU)))
#endif
    {
        uint32 Sram_End, Sram_Start;
        uint32 Sram_Flag =((READ_MEM32(0xE0080400U) >> 24U) & 0x0FU);
        if (0x0EU == Sram_Flag)
        {
#if defined (AC7840E)
            Sram_Start = 0x1FFFC000U;
            Sram_End = 0x20003FFFU;
#elif defined (AC7842X)
            Sram_Start = 0x1FFE8000U;
            Sram_End = 0x20016FFFU;
#else
            Sram_Start = SRAM_L_BASE;
            Sram_End = SRAM_U_END;
#endif
        }
        else if (0x0DU == Sram_Flag)
        {
#if defined (AC7840E)
            Sram_Start = 0x1FFFA000U;
            Sram_End = 0x20005FFFU;
#elif defined (AC7842X)
            Sram_Start = 0x1FFEC000U;
            Sram_End = 0x20012FFFU;
#else
            Sram_Start = SRAM_L_BASE;
            Sram_End = SRAM_U_END;
#endif
        }
        else
        {
            Sram_Start = SRAM_L_BASE;
            Sram_End = SRAM_U_END;
        }
        /* Initialize SRAM. */
        if (0U == (SPM->STB_WP_STATUS & 0x1FFFFU)) /* POR. */
        {
            SystemMemZero_Aligned8((uint64 *)Sram_Start, (uint64 *)StackBase);
            SystemMemZero_Aligned8((uint64 *)StackEnd, (uint64 *)Sram_End);
        }
        else /* Standby wakeup skip the retention SRAM. */
        {
            if (StackEnd <= SRAM_RETENTION_BASE)
            {
                /* Stack locate in SRAM_L but not in retention. */
                SystemMemZero_Aligned8((uint64 *)Sram_Start, (uint64 *)StackBase);
                SystemMemZero_Aligned8((uint64 *)StackEnd, (uint64 *)SRAM_RETENTION_BASE);
                SystemMemZero_Aligned8((uint64 *)SRAM_U_BASE, (uint64 *)Sram_End);
            }
            else if (StackBase >= SRAM_U_BASE)
            {
                /* Stack locate in SRAM_U. */
                SystemMemZero_Aligned8((uint64 *)Sram_Start, (uint64 *)SRAM_RETENTION_BASE);
                SystemMemZero_Aligned8((uint64 *)SRAM_U_BASE, (uint64 *)StackBase);
                SystemMemZero_Aligned8((uint64 *)StackEnd, (uint64 *)Sram_End);
            }
            else if ((StackBase < SRAM_RETENTION_BASE) && (StackEnd > SRAM_RETENTION_BASE))
            {
                /* Stack location in SRAM_L and retention. */
                SystemMemZero_Aligned8((uint64 *)Sram_Start, (uint64 *)StackBase);
                SystemMemZero_Aligned8((uint64 *)SRAM_U_BASE, (uint64 *)Sram_End);
            }
            else if ((StackBase < SRAM_U_BASE) && (StackEnd > SRAM_U_BASE))
            {
                /* Stack location in SRAM_U and retention. */
                SystemMemZero_Aligned8((uint64 *)Sram_Start, (uint64 *)SRAM_RETENTION_BASE);
                SystemMemZero_Aligned8((uint64 *)StackEnd, (uint64 *)Sram_End);
            }
            else
            {
                /* Stack location in retention fullly. */
                SystemMemZero_Aligned8((uint64 *)Sram_Start, (uint64 *)SRAM_RETENTION_BASE);
                SystemMemZero_Aligned8((uint64 *)SRAM_U_BASE, (uint64 *)Sram_End);
            }
        }
#if defined (AC7840X) || defined (AC7842X)
        /* Initialize FlexRAM if not used by CSE. */
        if (READ_BIT32(FLASH->PART, FLASH_PART_EPART_Msk) == FLASH_PART_EPART_Msk)
        {
            SystemMemZero_Aligned8((uint64 *)FLEXRAM_BASE, (uint64 *)(FLEXRAM_BASE + FLEX_RAM_SIZE));
        }
#endif
    }
#else /* NOT_INIT_SRAM */
    /* BootROM initialize RAM AC7843 */
    (void)StackBase;
    (void)StackEnd;
#endif
#if NOT_INIT_SRAM_AFTER_RESET
#if defined (AC7840E) || defined (AC7842X) || defined (AC7843X)
    /* Write the no init sram flag */
    CKGEN->RCM_EN |= CKGEN_RCM_EN_NOT_INIT_SRAM_Msk;
#endif
#endif
}
/*PRQA S 2995 -- # AMCE_SYS_2995 */

/*!
 * @brief Initialize the SRAM.
 * @note  Function ID: DES_BOOT_API_002
 * @param[in] StackBase: Main stack start address.
 * @param[in] StackEnd:  Main stack end address.
 * @return void
 */
void SystemInitRam(uint32 StackBase, uint32 StackEnd) //PRQA S 1503 # AMCE_SYS_1503
{
    uint8 i;

    /* Initialize the SRAM to 0 for ECC. */
    SystemInitAllRam(StackBase, StackEnd);
    /* Copy initialized data from ROM to RAM. */
    for (i = 0; i < (sizeof(SystemCopyTable) / sizeof(SystemCopyTable[0])); i++)
    {
        SystemMemCpy(SystemCopyTable[i].DstStart, SystemCopyTable[i].SrcStart, SystemCopyTable[i].SrcEnd);
    }
#ifdef VECT_TAB_SRAM
    SystemMemCpy((uint8 *)VECTOR_TABLE_SRAM_ADDR, \
    (uint8 *)(&__Vectors), (uint8 *)((uint32)(&__Vectors) + (((uint32)(MAX_IRQn) + 16U) << 2U)));
#endif
    /*PRQA S 2877 ++ # AMCE_SYS_2877*/
    /* Initialize bss to zero. */
    for (i = 0; i < (sizeof(SystemZeroTable) / sizeof(SystemZeroTable[0])); i++)
    {
        SystemMemZero(SystemZeroTable[i].DstStart, SystemZeroTable[i].DstEnd);
    }
    /*PRQA S 2877 -- # AMCE_SYS_2877*/
}

/*!
 * @brief DFlash Lock
 * @note  Function ID: DES_BOOT_API_010
 * @param[in] Device_Type: Device for access flash memory
 * @return FlashDeviceType
 */
FlashDeviceType System_FlsDeviceTryLock(FlashDeviceType Device_Type)
{
    FlashDeviceType Ret = NON_DEV;

    OSIF_ENTER_CRITICAL(FLS_HAL_ID2);
    if (g_dFlashDevice == NON_DEV)
    {
        g_dFlashDevice = Device_Type;
        Ret = g_dFlashDevice;
    }
    OSIF_EXIT_CRITICAL(FLS_HAL_ID2);

    return Ret;
}

/*!
 * @brief DFlash UnLock
 * @note  Function ID: DES_BOOT_API_011
 * @param[in] Device_Type: Device for access flash memory
 * @return FlashDeviceType
 */
FlashDeviceType System_FlsDeviceUnlock(FlashDeviceType Device_Type)
{
    FlashDeviceType Ret = NON_DEV;

    OSIF_ENTER_CRITICAL(FLS_HAL_ID2);
    if (g_dFlashDevice != Device_Type)
    {
        Ret = g_dFlashDevice;
    }
    g_dFlashDevice = NON_DEV;
    OSIF_EXIT_CRITICAL(FLS_HAL_ID2);

    return Ret;
}

/*!
 * @brief DFlash Free Lock.
 * @note  Function ID: DES_BOOT_API_012
 * @param[in] Device_Type: Device for access flash memory
 * @return FlashDeviceType
 */
FlashDeviceType System_FlsDeviceFreelock(FlashDeviceType Device_Type)
{
    FlashDeviceType Ret = NON_DEV;

    OSIF_ENTER_CRITICAL(FLS_HAL_ID2);
    if (g_dFlashDevice == Device_Type)
    {
        g_dFlashDevice = NON_DEV;
    }
    Ret = g_dFlashDevice;
    OSIF_EXIT_CRITICAL(FLS_HAL_ID2);

    return Ret;
}

/**
 * @brief copy src addr to dest addr
 * @note  Function ID : DES_BOOT_API_013
 * @param[in] Dest: dest  addr
 * @param[in] Src: src addr
 * @param[in] Len: copy len
 * @return dest address
 */
void *System_Memcpy(void *Dest, const void *Src, uint32 Len)
{
    uint8 *D = (uint8 *)Dest;        //PRQA S 0316 # AMCE_QAC_0316
    const uint8 *S = (const uint8 *)Src;   //PRQA S 0316 # AMCE_QAC_0316
    void *Ret = Dest;
     uint32 N = Len;

    if ((NULL_PTR == Dest) || (NULL_PTR == Src))
    {
        Ret = NULL_PTR;
    }
    else
    {
        /* deal address no align case */
        while (((((uint32)S % sizeof(uint32)) != 0U) || (((uint32)D % sizeof(uint32)) != 0U)) && (N > 0U))
        {
            *D = *S;
            S++;
            D++;
            N--;
        }

        /*copy order by 4 bytes*/
        uint32 *Wd = (uint32*)D;        //PRQA S 0310,3305 # AMCE_SYS_0310,AMCE_SYS_3305
        const uint32 *Ws = (const uint32 *)S;  //PRQA S 0310,3305 # AMCE_SYS_0310,AMCE_SYS_3305
        while (N >= 4U)
        {
            *Wd = *Ws;
            Wd++;
            Ws++;
            N -= 4U;
        }

        /*deal with no align data */
        D = (uint8 *)Wd;
        S = (const uint8 *)Ws;
        while (N > 0U) {
            *D = *S;
             D++;
             S++;
             N--;
        }

        /*Data memory barrier ensure access with order*/
        __DMB();
    }

    return Ret;
}
/**
 * @brief set a memory to value
 * @note  Function ID : DES_BOOT_API_014
 * @param[in] Addr: dest memory addr
 * @param[in] Val: set val
 * @param[in] N: len
 * @return dest address
 */
void *System_Memset(void *Addr, uint8 Val, uint32 Len)
{
    uint8 *D = (uint8 *)Addr;  //PRQA S 0316 # AMCE_QAC_0316
    uint32 N = Len;

    if (Addr != NULL_PTR)
    {
        /*  deal address no align case  */
        while ((((uint32)D % sizeof(uint32)) != 0U) && (N > 0U))
        {
            *D = Val;
            D++;
            N--;
        }

        /*set order by 4 bytes*/
        uint32 Fill_Val = (((uint32)Val << 24U) | ((uint32)Val << 16) | ((uint32)Val << 8) | (uint32)Val);   //PRQA S 0499 # AMCE_SYS_0499
        uint32 *Wd = (uint32 *)D;   //PRQA S 0310,3305 # AMCE_SYS_0310,AMCE_SYS_3305
        while (N >= 4U) {
            *Wd = Fill_Val;
             Wd++;
            N -= 4U;
        }

        /*deal with no align data*/
        D = (uint8 *)Wd;
        while(N > 0U)
        {
            *D = Val;
             D++;
             N--;
        }
        /*Data memory barrier ensure access with order*/
        __DMB();
    }
    return Addr;
}

/* =============================================  EOF  ============================================== */
