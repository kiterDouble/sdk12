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
 * AutoChips Inc. (C) 2022. All rights reserved.
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
 * @file Eep_Hal.c
 * @brief This file provides EEP integration functions.
 *
 */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Eep_Hal.h"
#include "Fls_Hal.h"
#include "Mpu_Hal.h"
#include "OsIf_Critical.h"
#include "AC784xx_Fls_Reg.h"

#if !defined(CONFIG_EEP_DRV_ENABLE) || defined (AC7842X)  || defined (AC7843X) || defined (AC7840E)
/* ============================================  DEFINES AND MACROS  ============================================ */
/*!
 * @brief group size define rule:
 * 1. Total EEP size (Byte) = EEP SRAM size
 * 2. The min EEP_GROUP_SIZE = EEP_SIZE (word) * 2 * EepFlashMinProgByteNum
 * 3. EEP_FLASH_SIZE = EEP_GROUP_SIZE * Total group
 */

/**
 * @def EEP_FLASH_ERASED_VALUE
 * @brief EEP flash erased value
 * @note DES ID: DES_EEP_MACRO_006
 */
#define EEP_FLASH_ERASED_VALUE          (0XFFFFFFFFU)

/**
 * @def EEP_GROUP_FLAG_VALUE
 * @brief EEP flash operate group flag
 * @note DES ID: DES_EEP_MACRO_007
 */
#define EEP_GROUP_FLAG_VALUE            (0x00000000U)

/**
 * @def EEP_RAM_BLOCK_BASE
 * @brief The EEP driver each RAM block base address.
 * @note DES ID: DES_EEP_MACRO_008
 */
#define EEP_RAM_BLOCK_BASE(blk) (EepConfigPtr->EepRamBaseAddr + \
                                 ((uint32)(blk) * (uint32)(EepConfigPtr->EepBlockSize)))

/**
 * @def EEP_RAM_BLOCK_END
 * @brief The EEP driver each RAM block end address.
 * @note DES ID: DES_EEP_MACRO_009
 */
#define EEP_RAM_BLOCK_END(blk) (EepConfigPtr->EepRamBaseAddr + \
                                (((uint32)(blk) + 1U) * (uint32)(EepConfigPtr->EepBlockSize)))

#if defined (AC7840X) && defined (CONFIG_EEP_SAVE_PHY_ADDR) && (CONFIG_EEP_SAVE_PHY_ADDR == 1U)
/*
 * @brief Physical address mode configuration for AC7840X
 *
 * When CONFIG_EEP_SAVE_PHY_ADDR is enabled, EEP stores physical RAM addresses
 * directly in flash instead of logical offsets.
 *
 * The following macros are redefined to operate directly with physical addresses:
 * - EEP_GROUP_LOGIC_ADDR_START: Maps to physical RAM block base address
 * - EEP_GROUP_LOGIC_ADDR_END: Maps to physical RAM block end address
 * - EEP_RAM_ADDR_TO_LOGIC: Identity function (no conversion needed)
 * - EEP_LOGIC_ADDR_TO_RAM: Identity function (no conversion needed)
 * - EEP_LOGIC_ADDR_FROM_FLASH: Identity function (no conversion needed)
 *
 * @note DES ID: DES_EEP_MACRO_023
 */
#define EEP_GROUP_LOGIC_ADDR_START EEP_RAM_BLOCK_BASE
#define EEP_GROUP_LOGIC_ADDR_END EEP_RAM_BLOCK_END
#define EEP_RAM_ADDR_TO_LOGIC(addr) (addr)
#define EEP_LOGIC_ADDR_TO_RAM(addr) (addr)
#define EEP_LOGIC_ADDR_FROM_FLASH(addr) (addr)
#else
/**
 * @def EEP_GROUP_LOGIC_ADDR_START
 * @brief The EEP driver each group logic base address.
 * @note DES ID: DES_EEP_MACRO_017
 */
#define EEP_GROUP_LOGIC_ADDR_START(blk) ((uint32)(blk) * (uint32)(EepConfigPtr->EepBlockSize))

/**
 * @def EEP_GROUP_LOGIC_ADDR_END
 * @brief The EEP driver each group logic end address.
 * @note DES ID: DES_EEP_MACRO_011
 */
#define EEP_GROUP_LOGIC_ADDR_END(blk) (((uint32)(blk) + 1U) * (uint32)(EepConfigPtr->EepBlockSize))

/**
 * @def EEP_RAM_ADDR_TO_LOGIC
 * @brief The EEP ram address to logic address.
 * @note DES ID: DES_EEP_MACRO_012
 */
#define EEP_RAM_ADDR_TO_LOGIC(addr) ((addr) - EepConfigPtr->EepRamBaseAddr)

/**
 * @def EEP_LOGIC_ADDR_TO_RAM
 * @brief The EEP ram address to logic address.
 * @note DES ID: DES_EEP_MACRO_013
 */
#define EEP_LOGIC_ADDR_TO_RAM(addr) ((addr) + EepConfigPtr->EepRamBaseAddr)

/**
 * @def EEP_LOGIC_ADDR_FROM_FLASH
 * @brief The EEP ram address to logic address.
 * @note DES ID: DES_EEP_MACRO_014
 */
#define EEP_LOGIC_ADDR_FROM_FLASH(addr) ((addr)& 0xfffU)
#endif

/**
 * @def EEP_FLASH_GROUP_BASE
 * @brief The EEP driver each flash group base address.
 * @note DES ID: DES_EEP_MACRO_015
 */
#define EEP_FLASH_GROUP_BASE(blk, grp) (EepConfigPtr->EepFlashBaseAddr + \
                                ((((uint32)(blk) * (uint32)(EepConfigPtr->EepGroupNum)) + (grp)) * EepFlashGroupSize))

/**
 * @def EEP_FLASH_GROUP_END
 * @brief The EEP driver each flash group end address.
 * @note DES ID: DES_EEP_MACRO_016
 */
#define EEP_FLASH_GROUP_END(blk, grp) (EepConfigPtr->EepFlashBaseAddr + \
                            ((((uint32)(blk) * (uint32)(EepConfigPtr->EepGroupNum)) + (grp) + 1U) * EepFlashGroupSize))

#define ALIGN_UP(val, align)            (((val) + (align) - 1U) & (~((align) - 1U)))
#define ALIGN_DOWN(val, align)          ((val) & (~(align) - 1U))

#define EEP_XOR_PATTERN                 0x5aa5U
#define EEP_SWITCH_GROUP_THRESHOLD        (20U)

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)

/*
 * EEP record integrity check is configured in Conf_AC784xx.h:
 *   - EEP_CRC16_CHECK_ENABLE : write/verify a CRC16 (poly 0x90D9) over the stored
 *                              16-bit address + 32-bit data; address bit15 is set
 *                              to 1 to mark CRC records (the read path masks it to
 *                              recover the logic address).
 */
#ifdef EEP_CRC16_CHECK_ENABLE
/**
 * @def EEP_CRC16_ADDR_FLAG
 * @brief Stored-address bit15: 1 => record checked with CRC16, 0 => legacy XOR.
 */
#define EEP_CRC16_ADDR_FLAG             (0x8000U)

/**
 * @def EEP_LOGIC_ADDR_MASK
 * @brief Mask to recover the real logic address (strip the CRC flag bit).
 */
#define EEP_LOGIC_ADDR_MASK             (0x7FFFU)

/**
 * @def EEP_CRC16_INIT
 * @brief EEP record CRC16 initial value (non-reflected, no final XOR).
 */
#define EEP_CRC16_INIT                  (0xFFFFU)

/**
 * @def EEP_CHECK_FIELD
 * @brief 16-bit check field of one record: CRC16 over the stored 16-bit address
 *        (including the bit15 flag) and the 32-bit data.
 */
#define EEP_CHECK_FIELD(Addr, Data)     ((uint32)Eep_Hal_Crc16((Addr), (Data)))
#else
/**
 * @def EEP_CHECK_FIELD
 * @brief Legacy check field: XOR of the logic address with a fixed pattern.
 */
#define EEP_CHECK_FIELD(Addr, Data)     ((Addr) ^ EEP_XOR_PATTERN)
#endif
#endif

/**
 * @def EEP_FLASH_ERASE_MAX_RETRY
 * @brief Maximum erase attempts per flash page when erase verify fails.
 */
#define EEP_FLASH_ERASE_MAX_RETRY         (3U)

/**
 * @def EEP_NO_PENDING
 * @brief Marker value for "no pending erase group" of a block.
 */
#define EEP_NO_PENDING                    (0xFFU)

#ifdef EEP_MPU_PROTECTION
/*!
 * @brief MPU instance ID used for EEPROM protection
 * @note MPU ID for EEPROM memory protection region
 * @note DES ID: DES_EEP_MACRO_019
 */
#define EEP_MPU_ID                      MPU_ID_0
/*!
 * @brief MPU region ID used for EEPROM protection
 * @note MPU region identifier for EEPROM data area protection
 * @note DES ID: DES_EEP_MACRO_020
 */
#define EEP_MPU_REGION_ID               MPU_REGION_ID_4
#endif

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */
/** @brief The EEP driver a flash group size in bytes. */
static uint32 EepFlashGroupSize = (uint32)EEP_FLASH_GROUP_SIZE;
/** @brief The EEP driver each flash program minimum size in bytes. */
static uint32 EepFlashMinProgByteNum = (uint32)DFLASH_WRITE_UNIT_SIZE;
/** @brief The EEP driver each flash program data size in bytes. */
static uint32 EepFlashDataByteNum = (uint32)(DFLASH_WRITE_UNIT_SIZE >> 1U);
/** @brief The EEP driver RAM block numbers. */
static uint16 EepBlockNum = EEP_BLOCK_MAX_NUM;
/** @brief The EEP driver current flash program address of each block. */
static uint32 CurFlashOperatAddr[EEP_BLOCK_MAX_NUM];
/** @brief The EEP driver current flash program block. */
static uint8 CurFlashGroup[EEP_BLOCK_MAX_NUM];
/** @brief The EEP driver last flash program block. */
static uint8 LastFlashGroup[EEP_BLOCK_MAX_NUM];
/** @brief The EEP driver configuration pointer. */
static const Eep_Hal_ConfigType *EepConfigPtr = NULL_PTR;
/** @brief The EEP driver status. */
static Eep_Hal_StatusType Eep_Status = EEP_HAL_UNINIT;
/** @brief Start address of the EEP flash area (inclusive) */
static uint32 EepFlashAddressStart;
/** @brief End address of the EEP flash area (exclusive) */
static uint32 EepFlashAddressEnd;
/** @brief Per-block old group still awaiting erase (EEP_NO_PENDING = none). */
static uint8 PendingEraseGroup[EEP_BLOCK_MAX_NUM];

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
static Hal_StatusType Eep_Hal_SwichGroup(uint8 Block);
static Hal_StatusType Eep_Hal_TryClearPending(uint8 Block);

/* ======================================  Functions define  ======================================== */
/*PRQA S 1503 ++ # AMCE_EEP_1503*/
/*!
 * @brief Fill memory with a specified value
 * @param[out] s Destination memory address
 * @param[in] c Value to fill
 * @param[in] count Number of bytes to fill
 * @note Function ID: DES_EEP_API_226
 */
static void Eep_Hal_Memset(uint8 *s, uint8 c, uint32 count)
{
    uint8 *ss = s;
    uint32 cnt = count;

    while (0x0U != cnt)
    {
        *ss = c;
        ss++;
        cnt--;
    }
}

/*!
 * @brief Copy memory between buffers
 * @param[out] dst Destination buffer address
 * @param[in] src Source buffer address
 * @param[in] count Number of bytes to copy
 * @note Function ID: DES_EEP_API_209
 */
static void Eep_Hal_Memcpy(uint8 *dst, const uint8 *src, uint32 count)
{
    const uint8 *s = src;
    uint8 *d = dst;
    uint32 cnt = count;

    /* Simple, byte oriented memcpy. */
    while (0x0U != cnt)
    {
        *d = *s;
        d++;
        s++;
        cnt--;
    }
}

#ifdef EEP_CRC16_CHECK_ENABLE
/*!
 * @brief CRC16 lookup table (256 entries), MSB-first.
 * @note Polynomial 0x90D9 (normal form), i.e. Koopman notation 0xC86C.
 */
/*PRQA S 2017,3218 ++  #AMCE_EEP_3218*/
static const uint16 Eep_Crc16Table[256] =
{
    0x0000U, 0x90D9U, 0xB16BU, 0x21B2U, 0xF20FU, 0x62D6U, 0x4364U, 0xD3BDU,
    0x74C7U, 0xE41EU, 0xC5ACU, 0x5575U, 0x86C8U, 0x1611U, 0x37A3U, 0xA77AU,
    0xE98EU, 0x7957U, 0x58E5U, 0xC83CU, 0x1B81U, 0x8B58U, 0xAAEAU, 0x3A33U,
    0x9D49U, 0x0D90U, 0x2C22U, 0xBCFBU, 0x6F46U, 0xFF9FU, 0xDE2DU, 0x4EF4U,
    0x43C5U, 0xD31CU, 0xF2AEU, 0x6277U, 0xB1CAU, 0x2113U, 0x00A1U, 0x9078U,
    0x3702U, 0xA7DBU, 0x8669U, 0x16B0U, 0xC50DU, 0x55D4U, 0x7466U, 0xE4BFU,
    0xAA4BU, 0x3A92U, 0x1B20U, 0x8BF9U, 0x5844U, 0xC89DU, 0xE92FU, 0x79F6U,
    0xDE8CU, 0x4E55U, 0x6FE7U, 0xFF3EU, 0x2C83U, 0xBC5AU, 0x9DE8U, 0x0D31U,
    0x878AU, 0x1753U, 0x36E1U, 0xA638U, 0x7585U, 0xE55CU, 0xC4EEU, 0x5437U,
    0xF34DU, 0x6394U, 0x4226U, 0xD2FFU, 0x0142U, 0x919BU, 0xB029U, 0x20F0U,
    0x6E04U, 0xFEDDU, 0xDF6FU, 0x4FB6U, 0x9C0BU, 0x0CD2U, 0x2D60U, 0xBDB9U,
    0x1AC3U, 0x8A1AU, 0xABA8U, 0x3B71U, 0xE8CCU, 0x7815U, 0x59A7U, 0xC97EU,
    0xC44FU, 0x5496U, 0x7524U, 0xE5FDU, 0x3640U, 0xA699U, 0x872BU, 0x17F2U,
    0xB088U, 0x2051U, 0x01E3U, 0x913AU, 0x4287U, 0xD25EU, 0xF3ECU, 0x6335U,
    0x2DC1U, 0xBD18U, 0x9CAAU, 0x0C73U, 0xDFCEU, 0x4F17U, 0x6EA5U, 0xFE7CU,
    0x5906U, 0xC9DFU, 0xE86DU, 0x78B4U, 0xAB09U, 0x3BD0U, 0x1A62U, 0x8ABBU,
    0x9FCDU, 0x0F14U, 0x2EA6U, 0xBE7FU, 0x6DC2U, 0xFD1BU, 0xDCA9U, 0x4C70U,
    0xEB0AU, 0x7BD3U, 0x5A61U, 0xCAB8U, 0x1905U, 0x89DCU, 0xA86EU, 0x38B7U,
    0x7643U, 0xE69AU, 0xC728U, 0x57F1U, 0x844CU, 0x1495U, 0x3527U, 0xA5FEU,
    0x0284U, 0x925DU, 0xB3EFU, 0x2336U, 0xF08BU, 0x6052U, 0x41E0U, 0xD139U,
    0xDC08U, 0x4CD1U, 0x6D63U, 0xFDBAU, 0x2E07U, 0xBEDEU, 0x9F6CU, 0x0FB5U,
    0xA8CFU, 0x3816U, 0x19A4U, 0x897DU, 0x5AC0U, 0xCA19U, 0xEBABU, 0x7B72U,
    0x3586U, 0xA55FU, 0x84EDU, 0x1434U, 0xC789U, 0x5750U, 0x76E2U, 0xE63BU,
    0x4141U, 0xD198U, 0xF02AU, 0x60F3U, 0xB34EU, 0x2397U, 0x0225U, 0x92FCU,
    0x1847U, 0x889EU, 0xA92CU, 0x39F5U, 0xEA48U, 0x7A91U, 0x5B23U, 0xCBFAU,
    0x6C80U, 0xFC59U, 0xDDEBU, 0x4D32U, 0x9E8FU, 0x0E56U, 0x2FE4U, 0xBF3DU,
    0xF1C9U, 0x6110U, 0x40A2U, 0xD07BU, 0x03C6U, 0x931FU, 0xB2ADU, 0x2274U,
    0x850EU, 0x15D7U, 0x3465U, 0xA4BCU, 0x7701U, 0xE7D8U, 0xC66AU, 0x56B3U,
    0x5B82U, 0xCB5BU, 0xEAE9U, 0x7A30U, 0xA98DU, 0x3954U, 0x18E6U, 0x883FU,
    0x2F45U, 0xBF9CU, 0x9E2EU, 0x0EF7U, 0xDD4AU, 0x4D93U, 0x6C21U, 0xFCF8U,
    0xB20CU, 0x22D5U, 0x0367U, 0x93BEU, 0x4003U, 0xD0DAU, 0xF168U, 0x61B1U,
    0xC6CBU, 0x5612U, 0x77A0U, 0xE779U, 0x34C4U, 0xA41DU, 0x85AFU, 0x1576U,
};
/*PRQA S 2017,3218 ++  #AMCE_EEP_3218*/

/*!
 * @brief Calculate the EEP record CRC16 over logic address and data.
 * @param[in] LogicAddr 16-bit logic address (only the low 16 bits are used)
 * @param[in] Data      32-bit data word
 * @return 16-bit CRC (poly 0x90D9 / Koopman 0xC86C, init 0xFFFF, non-reflected)
 * @details The 6 protected bytes are fed MSB-first:
 *          AddrHi, AddrLo, Data[31:24], Data[23:16], Data[15:8], Data[7:0].
 */
static uint16 Eep_Hal_Crc16(uint32 LogicAddr, uint32 Data)
{
    uint8 Buf[6];
    uint16 Crc = EEP_CRC16_INIT;

    Buf[0] = (uint8)(LogicAddr >> 8U);
    Buf[1] = (uint8)(LogicAddr);
    Buf[2] = (uint8)(Data >> 24U);
    Buf[3] = (uint8)(Data >> 16U);
    Buf[4] = (uint8)(Data >> 8U);
    Buf[5] = (uint8)(Data);

    for (uint8 i = 0U; i < 6U; i++)
    {
/*PRQA S 2985 ++ #AMCE_EEP_2985*/
        Crc = (uint16)((Crc << 8U) ^ Eep_Crc16Table[((Crc >> 8U) ^ Buf[i]) & 0xFFU]);
/*PRQA S 2985 -- #AMCE_EEP_2985*/
    }

    return Crc;
}
#endif

#ifdef ATC_DEVICE_ASSERT
/*!
 * @brief Validate EEP storage configuration
 * @return Configuration validity status
 * @note Function ID: DES_EEP_API_210
 * @details Verifies:
 * - RAM/Flash address ranges when CSE partition is enabled
 * - Full memory region accessibility when CSE is disabled
 */
static Hal_StatusType Eep_Hal_InitCheck(void)
{
    Hal_StatusType Ret = STATUS_ERROR;
    uint32 RamBase = EepConfigPtr->EepRamBaseAddr;
    uint32 RamEnd = RamBase + EepConfigPtr->EepSize;
    uint32 FlashBase = EepConfigPtr->EepFlashBaseAddr;
    uint32 FlashEnd = FlashBase + (EepConfigPtr->EepGroupNum * EepFlashGroupSize * EepBlockNum);

#if defined (AC7840X) || defined (AC7842X)
    /* FlexRAM is used by CSE */
    if (0U != Flash_Reg_GetCSEPartition())
    {
        /* FlexRam 64KB and DFlash last 64KB are used by CSE, the remain partition could be used by EEP. */
        if ((RamBase >= SRAM_L_BASE) && (RamEnd <= (SRAM_U_END + 1U)) \
                && (FlashBase >= DFLASH_BASE) && (FlashEnd <= (DFLASH_BASE + 0x10000U)))
        {
            Ret = STATUS_SUCCESS;
        }
    }
    else
    {
        /* SRAM L/U and FlexRam, all DFlash could be used by EEP. */
        if ((((RamBase >= SRAM_L_BASE) && (RamEnd <= (SRAM_U_END + 1U))) \
                || ((RamBase >= FLEXRAM_BASE) && (RamEnd <= (FLEXRAM_END + 1U)))) \
                && (FlashBase >= DFLASH_BASE) && (FlashEnd <= (DFLASH_END + 1U)))
        {
            Ret = STATUS_SUCCESS;
        }
    }
#elif defined (AC7843X) || defined (AC7840E)
    if (((RamBase >= SRAM_L_BASE) && (RamEnd <= (SRAM_U_END + 1U))) \
        && (FlashBase >= DFLASH_BASE) && (FlashEnd <= (DFLASH_END + 1U)))
    {
        Ret = STATUS_SUCCESS;
    }
#else
    #error "Unsupported MCU"
#endif

    return Ret;
}
#endif

/*!
 * @brief Enable or disable MPU protection for cached data
 * @param[in] Enable MPU protection control flag:
 *        - 0: Disable MPU protection
 *        - Non-zero: Enable MPU protection
 * @return Hal_StatusType
 * @retval STATUS_SUCCESS: Operation completed successfully
 * @note Function ID: DES_EEP_API_227
 * @remark This function controls MPU (Memory Protection Unit) protection for EEPROM cached data
 *         to ensure data integrity during critical operations like init/write/erase operations.
 *         When enabled, it enters a critical section to protect against concurrent access.
 *         When disabled, it exits the critical section.
 */
#ifdef EEP_MPU_PROTECTION
static void Eep_Hal_EnableMpuProt(uint32 Enable)
{
    Mpu_RegionAttrsType RegionAttrs = {
#if !defined (AC7840E)
        .MasterPidEnable = MPU_MASTER_PID_DISABLE,
        .Pid = 0U,
        .PidMask = 0U,
#endif
    };

    if (Enable != 0U)
    {
        RegionAttrs.Permission = MPU_PERMISSION_VALUE(MPU_MASTER_CORE, MPU_ATTR_R, MPU_ATTR_R);
        Mpu_Hal_SetRegionAttrs(EEP_MPU_ID, EEP_MPU_REGION_ID, &RegionAttrs);
        OSIF_EXIT_CRITICAL(EEP_HAL_ID1);
    }
    else
    {
        OSIF_ENTER_CRITICAL(EEP_HAL_ID1);
        RegionAttrs.Permission = MPU_PERMISSION_VALUE(MPU_MASTER_CORE, MPU_ATTR_RW, MPU_ATTR_RW);
        Mpu_Hal_SetRegionAttrs(EEP_MPU_ID, EEP_MPU_REGION_ID, &RegionAttrs);
    }
}
#else
static void Eep_Hal_EnableMpuProt(uint32 Enable)
{
    /* To fix QAC warning: avoid unused parameter */
    (void)Enable;
}
#endif
/*!
 * @brief Erase Flash page with verification
 * @param[in] pageAddress Start address of Flash page to erase
 * @return Operation status (STATUS_SUCCESS/STATUS_ERROR)
 * @note Function ID: DES_EEP_API_225
 * @warning Must be executed within flash operation critical section
 * @remark Operation sequence:
 * 1. Verify page erase status
 * 2. Perform erase if verification fails
 * 3. Re-verify after erase operation
 * 4. Retry erase and re-verify up to EEP_FLASH_ERASE_MAX_RETRY times if verify fails
 */
static Hal_StatusType Eep_Hal_EraseFlash(uint32 pageAddress)
{
    Hal_StatusType groupStatus = STATUS_ERROR;

    if ((pageAddress >= EepFlashAddressStart) && (pageAddress <= (EepFlashAddressEnd - EepFlashGroupSize)))
    {
        const uint32 pagesPerGroup = EepFlashGroupSize / DFLASH_PAGE_SIZE;

        /* All pages will be attempted even if one fails; report error if any page fails */
        groupStatus = STATUS_SUCCESS;

        /* Erase and verify next group */
        for (uint32 pageIndex = 0; pageIndex < pagesPerGroup; pageIndex++)
        {
            const uint32 pageBase = pageAddress + (pageIndex * DFLASH_PAGE_SIZE);
            Hal_StatusType status;

            status = DFlash_Hal_SectionVerify(pageBase - DFLASH_BASE, DFLASH_PAGE_SIZE);
            if (STATUS_SUCCESS == status)
            {
                continue;
            }

            /* Erase and verify, retry up to EEP_FLASH_ERASE_MAX_RETRY times if verify fails */
            for (uint32 retry = 0U; retry < EEP_FLASH_ERASE_MAX_RETRY; retry++)
            {
                status = DFlash_Hal_PageErase(pageBase - DFLASH_BASE);
                if (STATUS_SUCCESS == status)
                {
                    status = DFlash_Hal_SectionVerify(pageBase - DFLASH_BASE, DFLASH_PAGE_SIZE);
                }

                if (STATUS_SUCCESS == status)
                {
                    break;
                }
            }

            if (STATUS_SUCCESS != status)
            {
                /* keep erasing remaining pages, but report error for this group */
                groupStatus = status;
            }
        }
    }

    return groupStatus;
}

/*!
 * @brief Finish a pending old-group erase left by a previous switch/recovery.
 * @param[in] Block Memory block index
 * @return STATUS_SUCCESS if there is no pending erase or it was cleared
 * @remark Keeps the "at most 1 valid + 1 pending" invariant so valid groups
 *         never accumulate beyond two. Page-level retry still applies inside
 *         Eep_Hal_EraseFlash.
 */
static Hal_StatusType Eep_Hal_TryClearPending(uint8 Block)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    if (PendingEraseGroup[Block] != EEP_NO_PENDING)
    {
        Ret = Eep_Hal_EraseFlash(EEP_FLASH_GROUP_BASE(Block, PendingEraseGroup[Block]));
        if (STATUS_SUCCESS == Ret)
        {
            PendingEraseGroup[Block] = EEP_NO_PENDING;
        }
    }

    return Ret;
}

/*!
 * @brief Write data to Flash with EEP mapping
 * @param[in,out] FlashAddr Pointer to current Flash address (auto-incremented)
 * @param[in] Addr Virtual EEP address
 * @param[in] Size Data length in bytes
 * @param[in] Flush Operation mode:
 *        - 0: Normal write
 *        - 1: Force write all valid data
 * @param[in] updateFlag Write mode selector:
 *        - 0: Combined address+data write
 *        - 1: Direct data write
 * @return Operation status
 * @note Function ID: DES_EEP_API_211
 * @remark Supports two write modes:
 * - Combined mode: Pack 32-bit address + 32-bit data per Flash phrase
 * - Direct mode: Write raw data blocks
 */
static Hal_StatusType Eep_Hal_WriteDataToDflash(volatile uint32 *FlashAddr, uint32 Addr, uint16 Size,
                        uint32 Flush, uint32 updateFlag)
{
    Hal_StatusType Ret = STATUS_ERROR;
    uint32 CurAddr = Addr;
    uint16 CurSize = ALIGN_UP(Size, 4U);

    DEVICE_ASSERT((Addr % 4U) == 0U); //Alignment 4Byte
    if ((*FlashAddr >= EepFlashAddressStart) && ((*FlashAddr + CurSize) <= EepFlashAddressEnd))
    {
        if (updateFlag != 0U)
        {
            Ret = DFlash_Hal_PageWrite(*FlashAddr - DFLASH_BASE, CurSize,  (const uint8 *)CurAddr);
        }
        else
        {
            uint32 NewCombineData[EEP_FLASH_MAX_UNIT_SIZE] = {0};
            uint32 j = 0U;
            Ret = STATUS_SUCCESS;

            for (uint16 i = 0U; i < CurSize; i += 4U)
            {
                uint32 tmp;
                tmp = *(volatile uint32 *)CurAddr;
                if ((Flush == 0U) || ((Flush == 1U) && (tmp != EEP_FLASH_ERASED_VALUE)))
                {
#if defined (AC7842X)  || defined (AC7843X) || defined (AC7840E)
                    uint32 StoredAddr = EEP_RAM_ADDR_TO_LOGIC(CurAddr);
#ifdef EEP_CRC16_CHECK_ENABLE
                    StoredAddr |= EEP_CRC16_ADDR_FLAG;   /* mark this record as CRC-checked */
#endif
                    NewCombineData[j] = (StoredAddr << 16U) | (tmp >> 16U);
                    NewCombineData[j + 1U] = (tmp << 16U) | EEP_CHECK_FIELD(StoredAddr, tmp);
#else
                    NewCombineData[j] = EEP_RAM_ADDR_TO_LOGIC(CurAddr);
                    NewCombineData[j + 1U] = tmp;
#endif
                    j += 2U;
                }
                CurAddr += 4U;

                if (j == EEP_FLASH_MAX_UNIT_SIZE)
                {
                    Ret = DFlash_Hal_PageWrite(*FlashAddr - DFLASH_BASE, j * 4U,  (const uint8 *)NewCombineData);
                    if (Ret != STATUS_SUCCESS)
                    {
                        break;
                    }
                    *FlashAddr += j * 4U;
                    j = 0U;
                }
            }

            if ((j != 0U) && (Ret == STATUS_SUCCESS))
            {
                Ret = DFlash_Hal_PageWrite(*FlashAddr - DFLASH_BASE, j * 4U,  (const uint8 *)NewCombineData);
                *FlashAddr += j * 4U;
            }
        }
    }

    return Ret;
}

/*!
 * @brief Retrieve group flag value
 * @param[in] Block Memory block index
 * @param[in] Group Flash group index
 * @param[in] high Flag portion selector:
 *        - 0: Lower 32-bit flag
 *        - 1: Upper 32-bit flag
 * @param[out] Flag A pointer used to store the read flag value.
 * @return Hal_StatusType
 * @note Function ID: DES_EEP_API_212
 */
static Hal_StatusType Eep_Hal_GetGroupFlagVal(uint8 Block, uint8 Group, uint32 High, uint32 *Flag)
{
    uint32 FlashAddr = (EEP_FLASH_GROUP_BASE(Block, Group) + EepFlashGroupSize - EepFlashMinProgByteNum);

    FlashAddr = (High != 0U) ? (FlashAddr + EepFlashDataByteNum) : FlashAddr;
    return DFlash_Hal_Read((FlashAddr - DFLASH_BASE), sizeof(uint32), (uint8 *)Flag);
}

/*!
 * @brief Locate valid data start address
 * @param[in] Block Memory block index
 * @param[in] Group Flash group index
 * @param[in,out] GroupOperateAddr Valid data start address pointer
 * @return Hal_StatusType
 * @note Function ID: DES_EEP_API_213
 * @remark Reverse search algorithm:
 * 1. Traverse from group end to start
 * 2. Determine boundary at first non-erased value
 */
static Hal_StatusType Eep_Hal_GetGroupValidDataStartAddr(uint8 Block, uint8 Group, volatile uint32 *GroupOperateAddr)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    /* return group 0 start address if valid address was not found.*/
    *GroupOperateAddr = EEP_FLASH_GROUP_BASE(Block, Group);

    /* find valid address reverse.*/
    uint32 Addr = EEP_FLASH_GROUP_BASE(Block, Group) + EepFlashGroupSize - (2U * EepFlashMinProgByteNum);
    for (; Addr >= EEP_FLASH_GROUP_BASE(Block, Group); Addr -= EepFlashMinProgByteNum)
    {
        /* Check value was erased  */
        uint32 Data;
        Ret = DFlash_Hal_Read((Addr - DFLASH_BASE), sizeof(uint32), (uint8 *)&Data);
        if ((EEP_FLASH_ERASED_VALUE != Data) || (Ret != STATUS_SUCCESS))
        {
            if (Ret == STATUS_SUCCESS)
            {
                *GroupOperateAddr = Addr + EepFlashMinProgByteNum;
            }
            break;
        }
    }

    return Ret;
}

/*!
 * @brief Calculate valid flash group count and update pointers
 * @param[in] Block Memory block index
 * @param[out] ValidCnt a pointer used to store GroupValidCnt
 * @return Hal_StatusType
 * @note Function ID: DES_EEP_API_214
 * @remark Validation criteria:
 * - FlagL != Erased value (0xFFFFFFFF)
 * - FlagH == Group flag value (0x00000000)
 */
static Hal_StatusType Eep_Hal_UpdateOperateAddr(uint8 Block, uint8 *ValidCnt)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    Hal_StatusType Ret1 = STATUS_SUCCESS;
    uint8 GroupValidCnt = 0U;
    uint8 FirstValidIndex = 0U;
    uint8 FlashGroupCur = 0U;
    uint8 FlashGroupLast = 0U;
    uint32 FlagL, FlagH;

    for (uint8 i = 0U; i < EepConfigPtr->EepGroupNum; i++)
    {
        Ret = Eep_Hal_GetGroupFlagVal(Block, i, 0, &FlagL);
        Ret1 = Eep_Hal_GetGroupFlagVal(Block, i, 1, &FlagH);

        if ((Ret != STATUS_SUCCESS) || (Ret1 != STATUS_SUCCESS))
        {
            Ret = (Ret != STATUS_SUCCESS) ? Ret : Ret1;
            break;
        }

        if ((EEP_FLASH_ERASED_VALUE != FlagL) &&
            (EEP_GROUP_FLAG_VALUE == FlagH))
        {
            GroupValidCnt++;

            if (GroupValidCnt == 1U) /* Normal case */
            {
                FlashGroupCur = i;
                FlashGroupLast = i;
                FirstValidIndex = i;
            }
            else if (GroupValidCnt == 2U)
            {
                if ((i == (EepConfigPtr->EepGroupNum - 1U)) &&
                    (FirstValidIndex == 0U))
                {
                    FlashGroupLast = i;
                }
                else
                {
                    FlashGroupCur = i;
                }
            }
            else /* GroupValidCnt > 2 */
            {
                /* report error */
                FlashGroupCur  = 0U;
                FlashGroupLast = 0U;
            }
        }
    }

    CurFlashGroup[Block] = FlashGroupCur;
    LastFlashGroup[Block] = FlashGroupLast;
    *ValidCnt = GroupValidCnt;

    return Ret;
}

/*!
 * @brief Load flash data to RAM mirror
 * @param[in] Block Target RAM block index
 * @param[in] Group Source flash group index
 * @param[in] OperateAddr Flash group operation boundary address
 * @return Hal_StatusType
 * @note Function ID: DES_EEP_API_215
 * @remark Performs:
 * 1. Iterates through flash group addresses
 * 2. Validates address mapping between flash and RAM
 * 3. Copies valid data entries to RAM
 * @details For each flash entry:
 * - Checks 4-byte alignment validity
 * - Verifies RAM address range
 * - Executes direct memory copy when conditions met
 */
static Hal_StatusType Eep_Hal_LoadDataToRam(uint8 Block, uint8 Group, uint32 OperateAddr)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    /*PRQA S 0771 ++ # AMCE_EEP_0771*/
    for (uint32 Addr = EEP_FLASH_GROUP_BASE(Block, Group); Addr < OperateAddr; Addr += EepFlashMinProgByteNum)
    {
#if defined (AC7842X)  || defined (AC7843X) || defined (AC7840E)
        uint32 Temp1, Temp2, Data;
        uint32 LogicAddr;
        uint32 LogicAddrCheck;
        uint32 ExpCheck;

        Ret = DFlash_Hal_Read((Addr - DFLASH_BASE), sizeof(uint32), (uint8 *)&Temp1);
        if (Ret == STATUS_SUCCESS)
        {
            Ret = DFlash_Hal_Read((Addr + EepFlashDataByteNum - DFLASH_BASE), sizeof(uint32), (uint8 *)&Temp2);
        }

        if (Ret != STATUS_SUCCESS)
        {
            break;
        }

        LogicAddr = Temp1 >> 16U;
        Data = (Temp1 << 16U)|(Temp2 >> 16U);
        LogicAddrCheck = Temp2 & 0xffffU;

#ifdef EEP_CRC16_CHECK_ENABLE
        /* CRC16 over the stored address (bit15 == 1) + data; mask bit15 to
         * recover the real logic address. */
        ExpCheck = EEP_CHECK_FIELD(LogicAddr, Data);
        LogicAddr &= EEP_LOGIC_ADDR_MASK;
#else
        ExpCheck = EEP_CHECK_FIELD(LogicAddr, Data);
#endif

        if ((LogicAddr >= EEP_GROUP_LOGIC_ADDR_START(Block)) &&
            (LogicAddr < EEP_GROUP_LOGIC_ADDR_END(Block)) && ((LogicAddr % 4U) == 0U) &&
            (LogicAddrCheck == ExpCheck))
        {
            *(volatile uint32 *)(EEP_LOGIC_ADDR_TO_RAM(LogicAddr)) = Data;
        }
        else
        {
            continue;
        }
#else
        uint32 Temp, Data;

        Ret = DFlash_Hal_Read((Addr - DFLASH_BASE), sizeof(uint32), (uint8 *)&Temp);
        Temp = EEP_LOGIC_ADDR_FROM_FLASH(Temp);
        if (Ret != STATUS_SUCCESS)
        {
            break;
        }

        /* Check the temp address is valid or not, maybe the temp is erased && addr 4byte aligned */
        /*PRQA S 1840 ++ # AMCE_EEP_1840*/
        if ((Temp >= EEP_GROUP_LOGIC_ADDR_START(Block)) &&
            (Temp < EEP_GROUP_LOGIC_ADDR_END(Block)) && ((Temp % 4) == 0U))
        /*PRQA S 1840 -- # AMCE_EEP_1840*/
        {
            Ret = DFlash_Hal_Read((Addr + EepFlashDataByteNum - DFLASH_BASE), sizeof(uint32), (uint8 *)&Data);
            if (Ret != STATUS_SUCCESS)
            {
                break;
            }
            *(volatile uint32 *)(EEP_LOGIC_ADDR_TO_RAM(Temp)) = Data;
        }
        else
        {
            continue;
        }
#endif
    }
    /*PRQA S 0771 -- # AMCE_EEP_0771*/
    return Ret;
}

/*!
 * @brief Initialize RAM with flash data
 * @param[in] Block Memory block index
 * @return Hal_StatusType
 * @note Function ID: DES_EEP_API_216
 * @remark Operation sequence:
 * 1. Clear RAM block with 0xFF
 * 2. Load data from both current and last groups
 * 3. Update operation address pointer
 */
static Hal_StatusType Eep_Hal_SortRamData(uint8 Block, uint8 Cnt)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint8 CurFlashGroupIndex = CurFlashGroup[Block];

    Eep_Hal_EnableMpuProt(0U);
    Eep_Hal_Memset((uint8 *)(EEP_RAM_BLOCK_BASE(Block)), 0xFFU, EepConfigPtr->EepBlockSize);

    if ((Cnt == 1U) || (Cnt == 2U))
    {
        Ret = Eep_Hal_GetGroupValidDataStartAddr(Block, CurFlashGroupIndex, &CurFlashOperatAddr[Block]);
        if (Ret == STATUS_SUCCESS)
        {
            Ret = Eep_Hal_LoadDataToRam(Block, CurFlashGroupIndex, CurFlashOperatAddr[Block]);
        }
    }
    else
    {
        CurFlashOperatAddr[Block] = EEP_FLASH_GROUP_BASE(Block, CurFlashGroup[Block]);
    }
    Eep_Hal_EnableMpuProt(1U);

    return Ret;
}

/*!
 * @brief Update EEP group indexes and synchronize data
 * @param[in] Block Memory block index
 * @return Operation status
 * @note Function ID: DES_EEP_API_217
 * @remark Handles three scenarios:
 * - First initialization of group flags
 * - Normal single group operation
 * - Group rotation with data migration
 */
static Hal_StatusType Eep_Hal_UpdateAll(uint8 Block)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint8 GroupValidCnt;

    Ret = Eep_Hal_UpdateOperateAddr(Block, &GroupValidCnt);
    if (Ret == STATUS_SUCCESS)
    {
        Ret = Eep_Hal_SortRamData(Block, GroupValidCnt);
    }

    if (Ret == STATUS_SUCCESS)
    {
        if (GroupValidCnt == 0U)/* First used eep write operate flag in the first group */
        {
            const uint32 Flag[2] = {0U, 0U};
            volatile uint32 operateFlagAddr = (EEP_FLASH_GROUP_BASE(Block,CurFlashGroup[Block]) +
                                        EepFlashGroupSize - EepFlashMinProgByteNum);

            /* blank block: no old group to erase */
            PendingEraseGroup[Block] = EEP_NO_PENDING;
            Ret = Eep_Hal_EraseFlash(EEP_FLASH_GROUP_BASE(Block, CurFlashGroup[Block]));
            if (Ret == STATUS_SUCCESS)
            {
                Ret = Eep_Hal_WriteDataToDflash(&operateFlagAddr, (uint32)Flag, (uint16)EepFlashMinProgByteNum, 0, 1);
            }
        }
        else if (GroupValidCnt == 1U) /* Normal case */
        {
            uint32 LeftSize = (EEP_FLASH_GROUP_BASE(Block, CurFlashGroup[Block]) + EepFlashGroupSize
                        - CurFlashOperatAddr[Block] - EepFlashMinProgByteNum) / 2U;

            /* only one valid group: nothing pending to erase */
            PendingEraseGroup[Block] = EEP_NO_PENDING;
            if (LeftSize < EEP_SWITCH_GROUP_THRESHOLD)
            {
                Ret = Eep_Hal_SwichGroup(Block);
            }
        }
        else if (GroupValidCnt == 2U)
        {
            /* Interrupted switch: the newest group is valid, data is safe. If erasing
             * the old group fails transiently, record it as pending instead of failing
             * refresh/init; it gets retried on the next write (Eep_Hal_TryClearPending). */
            if (Eep_Hal_EraseFlash(EEP_FLASH_GROUP_BASE(Block, LastFlashGroup[Block])) == STATUS_SUCCESS)
            {
                PendingEraseGroup[Block] = EEP_NO_PENDING;
            }
            else
            {
                PendingEraseGroup[Block] = LastFlashGroup[Block];
            }
        }
        else /*  Three or more Valid groups are not supported */
        {
            Ret = STATUS_UNSUPPORTED;
        }
    }

    return Ret;
}

/**
 * @brief Switch valid data to a new Group.
 * @param[in] Block Memory block index
 * @return EEP_SUCCESS on success, non-zero error code on failure.
 * @note Function ID: DES_EEP_API_220
 */
static Hal_StatusType Eep_Hal_SwichGroup(uint8 Block)
{
    Hal_StatusType Ret;
    uint8 CurFlashGroupIndex = CurFlashGroup[Block];
    uint8 NextFlsGroup = (uint8)(((uint32)CurFlashGroupIndex + 1U) % (EepConfigPtr->EepGroupNum));

    if (NULL_PTR != EepConfigPtr->Callback)
    {
        EepConfigPtr->Callback();
    }

    /* Erase and verify next group */
    Ret = Eep_Hal_EraseFlash(EEP_FLASH_GROUP_BASE(Block, NextFlsGroup));
    if (STATUS_SUCCESS == Ret)
    {
        /* write group operate flag (64-bit 0) to the next group*/
        uint32 Flag[2] = {0U, 0U};
        volatile uint32 operateFlagAddr;

        Ret  = Eep_Hal_GetGroupFlagVal(Block, CurFlashGroupIndex, 0, &Flag[0]);
        if (STATUS_SUCCESS == Ret)
        {
            Flag[0] += 1U;
        }

        /* write all the SRAM data to the next group, NEED optimization */
        CurFlashOperatAddr[Block] = EEP_FLASH_GROUP_BASE(Block, NextFlsGroup);

        Ret = Eep_Hal_WriteDataToDflash(&CurFlashOperatAddr[Block], EEP_RAM_BLOCK_BASE(Block),
                                        (uint16)EepConfigPtr->EepBlockSize, 1, 0);
        /* write data to the next group */
        if (STATUS_SUCCESS == Ret)
        {
            operateFlagAddr = EEP_FLASH_GROUP_BASE(Block, NextFlsGroup) +
            EepFlashGroupSize - EepFlashMinProgByteNum;

            Ret = Eep_Hal_WriteDataToDflash(&operateFlagAddr, (uint32)Flag, (uint16)EepFlashMinProgByteNum, 0, 1);
        }

        /* update pointers and erase the previous group */
        if (STATUS_SUCCESS == Ret)
        {
            LastFlashGroup[Block] = CurFlashGroupIndex;
            CurFlashGroup[Block] = NextFlsGroup;

            /* The new group is fully written and valid; data is safe. If erasing
             * the old group fails transiently, record it as pending instead of
             * failing the switch, so stale valid groups never accumulate (cnt > 2).
             * Ret stays STATUS_SUCCESS: the switch itself succeeded. */
            if (Eep_Hal_EraseFlash(EEP_FLASH_GROUP_BASE(Block, CurFlashGroupIndex)) == STATUS_SUCCESS)
            {
                PendingEraseGroup[Block] = EEP_NO_PENDING;
            }
            else
            {
                PendingEraseGroup[Block] = CurFlashGroupIndex;
            }
        }
    }

    return Ret;
}

/*!
 * @brief Atomic write operation unit
 * @param[in] Block Memory block index
 * @param[in] Addr Starting address in EEP space
 * @param[in] Data Data buffer pointer
 * @param[in] Size Data length in bytes
 * @return Operation status
 * @note Function ID: DES_EEP_API_218
 * @remark Key operations:
 * 1. Address alignment handling
 * 2. Flash space availability check
 * 3. Group switching when space exhausted
 */
static Hal_StatusType Eep_Hal_WriteUnit(uint8 Block, uint32 Addr, const uint8 *Data, uint16 Size)
{
    Hal_StatusType Ret;
    uint32 CurAddr = Addr;
    uint16 CurSize = Size;

    DEVICE_ASSERT((0U != EepFlashMinProgByteNum) && (0U != EepFlashDataByteNum));

    /* Update to RAM */
    Eep_Hal_EnableMpuProt(0U);
    Eep_Hal_Memcpy((uint8 *)CurAddr, Data, Size);
    Eep_Hal_EnableMpuProt(1U);

    /* Opportunistically finish any pending old-group erase left by a previous
     * switch/recovery. */
    (void)Eep_Hal_TryClearPending(Block);

    /* Update to Dflash */
    uint8 CurFlashGroupIndex = CurFlashGroup[Block];
    uint16 AdditionalSize;
    uint16 LeftSize;

    AdditionalSize = (uint16)(CurAddr % EepFlashDataByteNum); /* Align down by EepFlashDataByteNum */
    if (AdditionalSize != 0U)
    {
        CurAddr -= AdditionalSize;
        CurSize += AdditionalSize;
    }

    LeftSize = (uint16)(EEP_FLASH_GROUP_BASE(Block, CurFlashGroupIndex) + EepFlashGroupSize
                        - CurFlashOperatAddr[Block] - EepFlashMinProgByteNum) / 2U;

    /* check available space is enough, every word combine with write address will use 8 FLASH address */
    if (LeftSize >= CurSize) /* enough */
    {
        Ret = Eep_Hal_WriteDataToDflash(&CurFlashOperatAddr[Block], CurAddr, CurSize, 0, 0);
    }
    else if (PendingEraseGroup[Block] != EEP_NO_PENDING)
    {
        /* Not enough space, but a previous old group is still pending erase (the
         * opportunistic clear above failed). Must NOT start a new switch, otherwise
         * stale valid groups accumulate (cnt > 2). Report failure; data is already
         * in the RAM mirror and gets flushed once the pending erase clears. */
        Ret = STATUS_TIMEOUT;
    }
    else /* not enough, no pending: safe to switch */
    {
        Ret = Eep_Hal_SwichGroup(Block);
    }

    return Ret;
}

/* Implementation of Eep_Hal_Write declared in Eep_Hal.h */
Hal_StatusType Eep_Hal_Write(uint32 Addr, const uint8 *Data, uint16 Size)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint32 CurAddr;
    uint16 CurSize = Size;
    const uint8 *CurData = (const uint8 *)Data;
    uint8 Block;

    DEVICE_ASSERT(NULL_PTR != EepConfigPtr);
    DEVICE_ASSERT(NULL_PTR != Data);
    DEVICE_ASSERT(EepConfigPtr->EepSize >= (Addr + Size));
    DEVICE_ASSERT(0U != EepConfigPtr->EepBlockSize);

    Ret = (NULL_PTR == EepConfigPtr) ? STATUS_ERROR :
          ((EepConfigPtr->EepSize < (Addr + Size)) ? STATUS_ERROR :
           ((EepConfigPtr->EepBlockSize == 0U) ? STATUS_ERROR : STATUS_SUCCESS));
    Ret = (NULL_PTR == Data) ? STATUS_ERROR : Ret;

    if (Ret == STATUS_SUCCESS)
    {
        /*PRQA S 2985 ++ # AMCE_EEP_2985*/
        CurAddr = Addr + EEP_RAM_BLOCK_BASE(0);
        /*PRQA S 2985 -- # AMCE_EEP_2985*/
        Block = (uint8)(Addr / EepConfigPtr->EepBlockSize);

        Eep_Status = EEP_HAL_BUSY;

        /* not cross RAM units. */
        for (uint16 RemainSize = Size; RemainSize > 0U; RemainSize -= CurSize)
        {
            CurSize = (uint16)(EEP_RAM_BLOCK_END(Block) - CurAddr);
            if (CurSize > RemainSize)
            {
                CurSize = RemainSize;
            }

            Ret = Eep_Hal_WriteUnit(Block, CurAddr, CurData, CurSize);
            if (Ret != STATUS_SUCCESS)
            {
                break;
            }

            CurAddr += CurSize;
            /*PRQA S 0488 ++ # AMCE_EEP_0488*/
            CurData += CurSize;
            /*PRQA S 0488 -- # AMCE_EEP_0488*/
            Block++;
        }

        if (Ret == STATUS_SUCCESS)
        {
            Eep_Status = EEP_HAL_IDLE;
        }
        else
        {
            Eep_Status = EEP_HAL_TIMEOUT;
        }
    }

    return Ret;
}

/* Implementation of Eep_Hal_Read declared in Eep_Hal.h */
Hal_StatusType Eep_Hal_Read(uint32 Addr, uint8 *Data, uint16 Size)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    DEVICE_ASSERT(NULL_PTR != EepConfigPtr);
    DEVICE_ASSERT(NULL_PTR != Data);
    DEVICE_ASSERT(EepConfigPtr->EepSize >= (Addr + Size));

    Ret = (NULL_PTR == EepConfigPtr) ? STATUS_ERROR :
          ((EepConfigPtr->EepSize < (Addr + Size)) ? STATUS_ERROR :
           ((NULL_PTR == Data) ? STATUS_ERROR : STATUS_SUCCESS));

    if (Ret == STATUS_SUCCESS)
    {
        /*PRQA S 2985 ++ # AMCE_EEP_2985*/
        Eep_Hal_Memcpy(Data, (const uint8 *)(EEP_RAM_BLOCK_BASE(0) + Addr), Size);
        /*PRQA S 2985 -- # AMCE_EEP_2985*/
    }

    return Ret;
}

/*!
 * @brief Internal EEP data refresh mechanism
 * @return Operation status (STATUS_SUCCESS/STATUS_ERROR)
 * @note Function ID: DES_EEP_API_219
 * @remark Performs multi-block refresh by:
 * 1. Iterating through all configured memory blocks
 * 2. Updating group indexes and data synchronization
 * 3. Maintaining data consistency across flash and RAM
 */
static Hal_StatusType Eep_Hal_InternalRefresh(void)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    DEVICE_ASSERT(NULL_PTR != EepConfigPtr);

    for (uint8 Block = 0U; Block < EepBlockNum; Block++)
    {
        Ret = Eep_Hal_UpdateAll(Block);
        if (Ret != STATUS_SUCCESS)
        {
            break;
        }
    }

    return Ret;
}

/* Implementation of Eep_Hal_Erase declared in Eep_Hal.h */
Hal_StatusType Eep_Hal_Erase(void)
{
    Hal_StatusType Ret;

    DEVICE_ASSERT(NULL_PTR != EepConfigPtr);
    DEVICE_ASSERT(0x0U != EepConfigPtr->EepBlockSize);

    Ret = (NULL_PTR == EepConfigPtr) ? STATUS_ERROR :
          ((EepConfigPtr->EepBlockSize == 0U) ? STATUS_ERROR : STATUS_SUCCESS);

    if (STATUS_SUCCESS == Ret)
    {
        Eep_Status = EEP_HAL_BUSY;

        /* erase dflash */
        for (uint16 i = 0U; i < (EepConfigPtr->EepGroupNum * EepBlockNum); i++)
        {
            Ret = Eep_Hal_EraseFlash(EepConfigPtr->EepFlashBaseAddr + (i * EepFlashGroupSize));
            if (Ret != STATUS_SUCCESS)
            {
                break;
            }
        }
    }

    /* erase RAM */
    if (STATUS_SUCCESS == Ret)
    {
        Ret = Eep_Hal_InternalRefresh();
    }

    if (Ret == STATUS_SUCCESS)
    {
        Eep_Status = EEP_HAL_IDLE;
    }
    else
    {
        Eep_Status = EEP_HAL_TIMEOUT;
    }

    return Ret;
}

/* Implementation of Eep_Hal_Init declared in Eep_Hal.h */
Hal_StatusType Eep_Hal_Init(const Eep_Hal_ConfigType *ConfigPtr)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    /* Save EEP driver configuration. */
    EepConfigPtr = ConfigPtr;

    /* PRQA S 2812 ++ # AMCE_EEP_2812*/
    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
    DEVICE_ASSERT((ConfigPtr->EepSize % EEP_MIN_SIZE) == 0U);
    DEVICE_ASSERT(ConfigPtr->EepSize <= EEP_MAX_SIZE);

    DEVICE_ASSERT(ConfigPtr->EepBlockSize > 0U);
    DEVICE_ASSERT(ConfigPtr->EepBlockSize < (EepFlashGroupSize / 2U));
    DEVICE_ASSERT((ConfigPtr->EepBlockSize % EepFlashDataByteNum) == 0U);

    DEVICE_ASSERT(ConfigPtr->EepGroupNum > 1U);
    DEVICE_ASSERT(ConfigPtr->EepGroupNum <= EEP_GROUP_MAX_NUM);

    DEVICE_ASSERT((EepFlashGroupSize % DFLASH_PAGE_SIZE) == 0U);
#ifdef ATC_DEVICE_ASSERT
    DEVICE_ASSERT(STATUS_SUCCESS == Eep_Hal_InitCheck());
#endif
    /* PRQA S 2812 -- # AMCE_EEP_2812*/

    if (ConfigPtr == NULL_PTR)
    {
        Ret = STATUS_ERROR;
    }
    else
    {
        Ret = ((ConfigPtr->EepSize % EEP_MIN_SIZE) != 0U) ? STATUS_ERROR :
              ((ConfigPtr->EepSize > EEP_MAX_SIZE) ? STATUS_ERROR : Ret);
        Ret = (ConfigPtr->EepBlockSize >= (EepFlashGroupSize / 2U)) ? STATUS_ERROR :
              (((ConfigPtr->EepBlockSize % EepFlashDataByteNum) != 0U) ? STATUS_ERROR : Ret);
        Ret = (ConfigPtr->EepGroupNum < 1U) ? STATUS_ERROR :
              ((ConfigPtr->EepGroupNum > EEP_GROUP_MAX_NUM) ? STATUS_ERROR : Ret);

        /*PRQA S 2832 ++ # AMCE_EEP_2832*/
        EepBlockNum = (ConfigPtr->EepSize + ConfigPtr->EepBlockSize - 1U) / (ConfigPtr->EepBlockSize);
        /*PRQA S 2832 -- # AMCE_EEP_2832*/
        DEVICE_ASSERT(EepBlockNum <= EEP_BLOCK_MAX_NUM);

        Ret = (EepBlockNum > EEP_BLOCK_MAX_NUM) ? STATUS_ERROR :
              (((EepFlashGroupSize % DFLASH_PAGE_SIZE) != 0U) ? STATUS_ERROR : Ret);

        EepFlashAddressStart = EepConfigPtr->EepFlashBaseAddr;
        EepFlashAddressEnd = EepConfigPtr->EepFlashBaseAddr +
                 (uint32)(EepBlockNum * EepFlashGroupSize * EepConfigPtr->EepGroupNum);

        for (uint8 i = 0U; i < EEP_BLOCK_MAX_NUM; i++)
        {
            PendingEraseGroup[i] = EEP_NO_PENDING;
        }
    }

    if (Ret == STATUS_SUCCESS)
    {
        Ret = Eep_Hal_InternalRefresh();

        Eep_Status = (Ret == STATUS_SUCCESS) ? EEP_HAL_INITIALIZED : EEP_HAL_UNINIT;
    }

    return Ret;
}

/* Implementation of Eep_Hal_Refresh declared in Eep_Hal.h */
#ifndef EEP_SDK_NON_EXTENDED_API
Hal_StatusType Eep_Hal_Refresh(void)
{
    return Eep_Hal_InternalRefresh();
}
#endif

/* Implementation of Eep_Hal_GetStatus declared in Eep_Hal.h */
Eep_Hal_StatusType Eep_Hal_GetStatus(void)
{
    return Eep_Status;
}

/* Implementation of Eep_Hal_AbortFreeRes declared in Eep_Hal.h */
void Eep_Hal_AbortFreeRes(void)
{
/*PRQA S 3200 ++ # AMCE_EEP_3200*/
    System_FlsDeviceFreelock(FLS_DEV);
/*PRQA S 3200 -- # AMCE_EEP_3200*/
}

/* Implementation of EEP_Hal_GetVersion declared in Eep_Hal.h */
void EEP_Hal_GetVersion(Eep_Hal_VersionType *Version)
{
    if (Version != NULL_PTR)
    {
        Version->Major = EEP_HAL_SW_MAJOR_VERSION;
        Version->Minor = EEP_HAL_SW_MINOR_VERSION;
        Version->Patch = EEP_HAL_SW_PATCH_VERSION;
    }
}

/* Implementation of Eep_Hal_ForceSwitchGroup declared in Eep_Hal.h */
Hal_StatusType Eep_Hal_ForceSwitchGroup(uint32 EepGroupMask)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    if (EepConfigPtr == NULL_PTR)
    {
        Ret = STATUS_ERROR;
    }

    if (STATUS_SUCCESS == Ret)
    {
        for (uint8 i = 0; i < EepBlockNum; i++)
        {
            /* PRQA S 1891 ++ # AMCE_EEP_1891*/
            if ((EepGroupMask & (1U << i)) == 0U)
            /* PRQA S 1891 -- # AMCE_EEP_1891*/
            {
                continue;
            }

            /* A previous switch may have left the old group pending erase (still a
             * valid group). Finish it first; if it still cannot be erased, do NOT
             * start another switch, otherwise valid groups accumulate (cnt > 2). */
            (void)Eep_Hal_TryClearPending(i);
            if (PendingEraseGroup[i] != EEP_NO_PENDING)
            {
                Ret = STATUS_TIMEOUT;
            }
            else
            {
                 Ret = Eep_Hal_SwichGroup(i);
            }

            if (Ret != STATUS_SUCCESS)
            {
                break;
            }
        }
    }

    return Ret;
}

/* Implementation of Eep_Hal_GetGroupFreeLength declared in Eep_Hal.h */
Hal_StatusType Eep_Hal_GetGroupFreeLength(Eep_Hal_GroupTable *EepGroupTable)
{
    Hal_StatusType Ret = STATUS_SUCCESS;

    if (EepGroupTable == (void *)0U)
    {
        Ret = STATUS_ERROR;
    }
    else if (EepConfigPtr == NULL_PTR)
    {
        Ret = STATUS_ERROR;
    }
    else
    {
        Eep_Hal_Memset((uint8 *)EepGroupTable, 0, sizeof(Eep_Hal_GroupTable));

        EepGroupTable->GroupNum = (uint8)EepBlockNum;
        EepGroupTable->GroupSize = EepConfigPtr->EepBlockSize;

        for (uint8 i = 0; i < EepGroupTable->GroupNum; i++)
        {
            EepGroupTable->Groups[i].GroupId = i;
            EepGroupTable->Groups[i].StartAddr = i * EepGroupTable->GroupSize;
            EepGroupTable->Groups[i].RemainSize = (EEP_FLASH_GROUP_BASE(i, CurFlashGroup[i]) + EepFlashGroupSize
                                                - CurFlashOperatAddr[i] - EepFlashMinProgByteNum) / 2U;
        }
    }

    return Ret;
}
#endif
/*PRQA S 1503 -- # AMCE_EEP_1503*/
/* =============================================  EOF  ============================================== */
