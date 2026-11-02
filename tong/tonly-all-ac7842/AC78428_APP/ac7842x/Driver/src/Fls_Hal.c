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
*@file Fls_Hal.c
*
*@brief This file provides sdk flash api function
*
*/

/*==============================================INCLUDE FILES=======================================*/
#include "Fls_Hal.h"
#include "AC784xx_Fls_Reg.h"
#include "OsIf_Critical.h"
#if defined (AC7842X)
#include "Spm_Hal.h"
#include "Core_Hal.h"
#elif defined (AC7840E)
#include "Core_Hal.h"
#endif
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*===========================================DEFINES AND MACROS==================================*/
#define FLASH_INVALID_ADDR               (0xFFFFFFFFU)
#define FLASH_NOT_USE_ADDR               (0x40000U)

#if defined (AC7840X)
#define FLASH_PPROT_UNIT_SIZE              (0x8000U)
#define FLASH_PPROT_MAX_UNIT               (32U)
#define FLASH_DPROT_UNIT_SIZE              (0x4000U)
#define FLASH_DPROT_CSE_UNIT_SIZE          (0x2000U)
#define FLASH_DPROT_MAX_UNIT               (8U)
#define FLASH_WPROT_BYTES                  (8U)
#elif defined (AC7842X)
#define FLASH_PPROT_UNIT_SIZE              (0x10000U)
#define FLASH_PPROT_MAX_UNIT               (32U)
#define FLASH_DPROT_UNIT_SIZE              (0x4000U)
#define FLASH_DPROT_CSE_UNIT_SIZE          (0x2000U)
#define FLASH_DPROT_MAX_UNIT               (8U)
#define FLASH_WPROT_BYTES                  (8U)
#elif defined (AC7843X)
#define FLASH_PPROT_UNIT_SIZE              (0x8000U)
#define FLASH_PPROT_MAX_UNIT               (128U)
#define FLASH_DPROT_UNIT_SIZE              (0x4000U)
#define FLASH_DPROT_CSE_UNIT_SIZE          (0x4000U)
#define FLASH_DPROT_MAX_UNIT               (16U)
#define FLASH_WPROT_BYTES                  (24U)
#elif defined (AC7840E)
#define FLASH_PPROT_UNIT_SIZE              (0x4000U)
#define FLASH_PPROT_MAX_UNIT               (36U)
#define FLASH_DPROT_UNIT_SIZE              (0x1000U)
#define FLASH_DPROT_MAX_UNIT               (32U)
#define FLASH_WPROT_BYTES                  (24U)
#endif

/* EFLASH_MODULE hardware algorithm operation commands */
#define FLASH_ERASE_PAGE                         (0x01U)  /**< Erase flash page command */
#define FLASH_ERASE_BLOCK                        (0x02U)  /**< Erase flash block command */
#define FLASH_OPTION_PAGE_ERASE                  (0x03U)  /**< Erase option page command */
#define FLASH_ERASE_ALL_BLOCK_UNSECURE           (0x04U)  /**< Erase all blocks unsecure command */
#define FLASH_ERASE_MULTIPAGE                    (0x05U)  /*!< FLASH multiple pages erase command */
#define FLASH_PROGRAM_PHRASE                     (0x10U)  /**< Program phrase command */
#define FLASH_PROGRAM_SECTION                    (0x11U)  /**< Program section command */
#define FLASH_PROGRAM_ONCE                       (0x12U)  /**< Program once command */
#define FLASH_VERIFY_SECTION                     (0x20U)  /**< Verify section command */
#define FLASH_VERIFY_BLOCK                       (0x21U)  /**< Verify block command */
#define FLASH_VERIFY_ALL_BLOCK                   (0x22U)  /**< Verify all blocks command */
#define FLASH_SECURITY_BY_PASS                   (0x23U)  /**< Security bypass command */
#define FLASH_ERASE_PARTITION                    (0x30U)  /**< Erase partition command */
#define FLASH_PROGRAM_PARTITION                  (0x31U)  /**< Program partition command */
#define FLASH_SET_FLEXRAM                        (0x32U)  /**< Set FlexRAM command */
#define FLASH_PROGRAM_CSE                        (0x33U)  /**< Program CSE command */

/*===========================================VARIABLE DECLARATIONS==================================*/
/*!< The size in byte of Flash memory */
static uint32 S_PFlashSize = PFLASH_BLOCK_SIZE;
static uint32 S_DFlashSize = DFLASH_BLOCK_SIZE;

/* disable write/erase timeout and enbale lvd by default */
/*PRQA S 3218 ++ # AMCE_FLS_3218.*/
static Flash_Config S_FlsCfg = {0U, 0U, TRUE};
/*PRQA S 3218 -- # AMCE_FLS_3218.*/

#if defined (AC7840X) || defined (AC7842X)
#if !defined (__ICCARM__)
/*PRQA S 3218 ++ # AMCE_FLS_3218.*/
static uint32 FLASH_StartCmd_WaitClear_Cpy[128] __attribute__((aligned(4))) = {0};
/*PRQA S 3408 ++ # AMCE_FLS_3408.*/
static void (*FLASH_StartCmd_WaitClr_InRam)(FLASH_Type* Flash) = NULL_PTR;
/*PRQA S 3408 -- # AMCE_FLS_3408.*/
/*PRQA S 3218 -- # AMCE_FLS_3218.*/
#endif
#endif

#if defined (AC7840E)
#if !defined (__ICCARM__)
/*PRQA S 3218 ++ # AMCE_FLS_3218.*/
static uint32 Flash_Hal_StartCmdInRam_Cpy[128] __attribute__((aligned(4))) = {0};
/*PRQA S 3408 ++ # AMCE_FLS_3408.*/
static Hal_StatusType (*Flash_Hal_StartCmdInRamFunc)(FLASH_Type* Flash, uint32 Wait_Timeout) = NULL_PTR;
/*PRQA S 3408 -- # AMCE_FLS_3408.*/
/*PRQA S 3218 -- # AMCE_FLS_3218.*/
#endif
#endif

#if defined (AC7842X)
static volatile uint8 McuLvdFlag = 0U;
#endif
/*============================================FUNCTION PROTOTYPES===================================*/

/*=======================================FUNCTIONS==================================================*/
/*PRQA S 1503 ++ # AMCE_FLS_1503.*/
/*PRQA S 3415 ++ # AMCE_FLS_3415.*/
/*PRQA S 3006 ++ # AMCE_FLS_3006.*/
#if defined (AC7842X)
/**
 * @brief Clear the MCU LVD interrupt status and get the current LVD state.
 * @details This function performs the following operations:
 *          1. Clears the MCU Low Voltage Detect (LVD) interrupt status by writing '1' to bit 22
 *             of register 0xE000E284.
 *          2. Inserts two NOP instructions to ensure the clear operation takes effect.
 *          3. Reads the LVD status from bit 22 of register 0xE000E204 and returns the result.
 *
 * @note The LVD status bit is masked with (1 << 22). If the result is non-zero, it indicates
 *       that the LVD condition is present.
 *       DES_FLS_API_240
 *
 * @param[in] void
 *
 * @return uint32_t The current LVD status:
 *         - 0: No LVD detected.
 *         - Non-zero: LVD condition detected.
 *
 * @pre None
 * @post LVD interrupt status is cleared
 */
static uint32 McuLvdClearAndGetStatus(void)
{
    /*PRQA S 1006 ++ # AMCE_FLS_1006.*/
    MODIFY_REG32(*(volatile uint32 *)0xE000E284U, (uint32)((uint32)0x1U << 22U), 22U, 1U);
    ASM_KEYWORD("nop");
    ASM_KEYWORD("nop");
    return READ_REG32(*(volatile uint32 *)0xE000E204U) & (uint32)((uint32)0x1U << 22U);
    /*PRQA S 1006 -- # AMCE_FLS_1006.*/
}
#endif

#if defined (AC7840X) || defined (AC7842X)
#if defined (__ICCARM__)
/**
* @brief Trigger flash command execution
* @details This function triggers the flash command execution by setting the START bit in the CST register,
*          then waits for the command to complete by checking the START bit clearing and executing
*          a series of NOP instructions.
*
* @note Function is placed in RAM for faster execution on AC7840X platform
*       DES_FLS_API_244
*
* @param [in] void
* @return void
* @retval None
*
* @pre Flash controller must be initialized
* @post Flash command is triggered
*/
__ramfunc void FLASH_StartCmd_WaitClear(FLASH_Type* Flash)
#else
/**
* @brief Trigger flash command execution
* @details This function triggers the flash command execution by setting the START bit in the CST register,
*          then waits for the command to complete by checking the START bit clearing and executing
*          a series of NOP instructions.
*
* @note Function is placed in RAM for faster execution on AC7840X platform
*       DES_FLS_API_244
*
* @param [in] void
* @return void
* @retval None
*
* @pre Flash controller must be initialized
* @post Flash command is triggered
*/
static void FLASH_StartCmd_WaitClear(FLASH_Type* Flash)
#endif
/*PRQA S 3006 -- # AMCE_FLS_3006.*/
{
    volatile uint8 timeout = 16U;
    volatile uint8 CST_Timeout = 10U;

    /* trigger command execute */
    SET_BIT32(Flash->CST, FLASH_CST_START_Msk);
    /*wait trigger register clear finished*/
    while ((0x0U != READ_BIT32(Flash->CST, FLASH_CST_START_Msk)) && (0x0U != CST_Timeout))
    {
        CST_Timeout = CST_Timeout - 1U;
    }

    while(timeout > 0U)
    {
        /*PRQA S 1006 ++ # AMCE_FLS_1006.*/
        ASM_KEYWORD("nop");
        /*PRQA S 1006 -- # AMCE_FLS_1006.*/
        timeout = timeout - 1U;
    };
}
#endif

#if defined (AC7840E)
/**
* @brief Trigger flash command execution in ram (AC7840E version)
* @details This function triggers the flash command execution by setting the START bit in the CST register,
*          then waits for the command to complete by checking the START bit clearing and executing
*          a series of NOP instructions.
*
* @note Function is placed in RAM for AC7840E platform
*       DES_FLS_API_247
*
* @param [in] Flash: Pointer to FLASH register map
* @param [in] Wait_Timeout: Timeout value for flash operation
* @return void
* @retval None
*
* @pre Flash controller must be initialized
* @post Flash command is triggered
*/
#if defined (__ICCARM__)
__ramfunc static Hal_StatusType Flash_Hal_StartCmdInRam(FLASH_Type* Flash, uint32 Wait_Timeout)
#else
static Hal_StatusType Flash_Hal_StartCmdInRam(FLASH_Type* Flash, uint32 Wait_Timeout)
#endif
/*PRQA S 3006 -- # AMCE_FLS_3006.*/
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    volatile uint32 TmpWait_Timeout = Wait_Timeout;
    /* Enable / disable timeout*/
    volatile boolean Timeout_Enable = (0U == TmpWait_Timeout) ? FALSE : TRUE;
    volatile uint8 CommandStatus = 0x0U;
    volatile uint32 Timeout = 2U;

    /* trigger flash */
    WRITE_REG32(Flash->CST, FLASH_CST_START_Msk);

    /*wait cache clear*/
    while (Timeout > 0U)
    {
        /*PRQA S 3387 ++ # AMCE_FLS_3387.*/
        Timeout--;
        /*PRQA S 3387 -- # AMCE_FLS_3387.*/
    }

    /*wait trigger register clear finished*/
    while (0x0U != (uint8)(READ_REG32(Flash->CST) & FLASH_CST_START_Msk))
    {
    }

    while (0x0U == CommandStatus)
    {
        CommandStatus = (uint8)((READ_REG32(Flash->STAT) & FLASH_STAT_CCIF_Msk) >> FLASH_STAT_CCIF_Pos);
        if (TRUE == Timeout_Enable)
        {
            TmpWait_Timeout = TmpWait_Timeout - 1U;
            if (0U == TmpWait_Timeout)
            {
                uint8 CMD = (uint8)((READ_REG32(Flash->CMD) & FLASH_CMD_CMD_Msk) >> FLASH_CMD_CMD_Pos);
                if ((CMD >= FLASH_ERASE_PAGE) && (CMD <= FLASH_OPTION_PAGE_ERASE))
                {
                    /* abort flash erase operation */
                    MODIFY_REG32(Flash->CNFG, FLASH_CNFG_PERABORT_Msk, FLASH_CNFG_PERABORT_Pos, 1U);
                    do
                    {
                        CommandStatus = (uint8)((READ_REG32(Flash->STAT) & FLASH_STAT_CCIF_Msk) >> FLASH_STAT_CCIF_Pos);
                    }
                    while (0x0U == CommandStatus);
                    MODIFY_REG32(Flash->CNFG, FLASH_CNFG_PERABORT_Msk, FLASH_CNFG_PERABORT_Pos, 0U);
                }
                break;
            }
        }
    }

    /*Check flash status whether fail*/
    if ((READ_REG32(Flash->STAT) & FLASH_STAT_ERROR_BITS) != 0x0U)
    {
        RetVal = STATUS_ERROR;
    }
    else if ((TRUE == Timeout_Enable) && (0U == TmpWait_Timeout))
    {
        RetVal = STATUS_TIMEOUT;
    }
    else
    {
        /*do nothing*/
    }

    return RetVal;
}
#endif

/**
* @brief Set cmd to start flash controller process
* @details This function triggers the flash command execution and waits for completion.
*          It handles timeout conditions and checks for errors during flash operations.
*
* @note Function ID:[DES_FLS_API_208]
* @param [in] Wait_Timeout:  flash operation timeout counter
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Command started successfully
* @retval STATUS_ERROR: Error occurred during command execution
* @retval STATUS_TIMEOUT: Command execution timed out
* @retval STATUS_ABORT: Command was aborted
* @pre Flash controller must be initialized
* @post Flash command is executed and completed
*/
/*PRQA S 2889 ++ # AMCE_FLS_2889.*/
static Hal_StatusType Flash_Hal_StartCmd(uint32 Wait_Timeout)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;

#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
    volatile uint32 TmpWait_Timeout = Wait_Timeout;
    /* Enable / disable timeout*/
    volatile boolean Timeout_Enable = (0U == TmpWait_Timeout) ? FALSE : TRUE;
    volatile uint8 CommandStatus = 0x0U;

#if defined (AC7842X)

    /* Check MCU LVD flag. If set, clear the interrupt status.
     * If LVD status is not active, enable LVD IRQ and clear the flag;
     * otherwise, return error status and stop flash write operation.
     */
    if (McuLvdFlag == 1U)
    {
        uint32 McuLvdSts = McuLvdClearAndGetStatus();
        if (McuLvdSts == 0U)
        {
            Core_Hal_EnableIrq(LVD_IRQn);
            Flash_Reg_SetCnfgEmulvdStatusReg(0U);
            McuLvdFlag = 0U;
        }
        else
        {
            return STATUS_ABORT;
        }
    }
#endif /* AC7842X */

    /*trigger flash */
#if defined (AC7840X) || defined (AC7842X)
    OSIF_ENTER_CRITICAL(FLS_HAL_ID1);
#if defined (__ICCARM__)
    FLASH_StartCmd_WaitClear(FLASH);
#else
    if (FLASH_StartCmd_WaitClr_InRam == NULL_PTR)
    {
        /*PRQA S 0326 ++ # AMCE_FLS_0326.*/
        /*PRQA S 0305 ++ # AMCE_FLS_0305.*/
        (void)System_Memcpy(FLASH_StartCmd_WaitClear_Cpy, (void *)((uint32)FLASH_StartCmd_WaitClear & 0xFFFFFFFEU),
                sizeof(FLASH_StartCmd_WaitClear_Cpy));
        FLASH_StartCmd_WaitClr_InRam = (void (*)(FLASH_Type* Flash))((uint32)FLASH_StartCmd_WaitClear_Cpy | 0x1U);
        /*PRQA S 0305 -- # AMCE_FLS_0305.*/
        /*PRQA S 0326 -- # AMCE_FLS_0326.*/
    }
    FLASH_StartCmd_WaitClr_InRam(FLASH);
#endif /* __ICCARM__ */
    OSIF_EXIT_CRITICAL(FLS_HAL_ID1);
#else /* AC7843X */
    volatile uint32 Timeout = 37U; /* the value is ok for check cache clear */

    Flash_Reg_TrigCtrlCmdReg();
    /*wait cache clear*/
    while (Timeout > 0U)
    {
        /*PRQA S 3387 ++ # AMCE_FLS_3387.*/
        Timeout--;
        /*PRQA S 3387 -- # AMCE_FLS_3387.*/
    }
    /*wait trigger register clear finished*/
    while (0x0U != Flash_Reg_GetTrigCtrlCmdRegStatus())
    {
    }
#endif /* AC7843X */
    /*wait trigger flash finished*/
    while (0x0U == CommandStatus)
    {
        CommandStatus = Flash_Reg_GetCommandCompleteStatusReg();
        if (TRUE == Timeout_Enable)
        {
            TmpWait_Timeout = TmpWait_Timeout - 1U;
            if (0U == TmpWait_Timeout)
            {
                break;
            }
        }
    }
    /*Check flash status whether fail*/
    if (Flash_Reg_GetStatusReg() != 0x0U)
    {
        RetVal = STATUS_ERROR;
    }
    else if ((TRUE == Timeout_Enable) && (0U == TmpWait_Timeout))
    {
        RetVal = STATUS_TIMEOUT;
    }
    else
    {
        /*do nothing*/
    }
#else /* AC7840E */
#if defined (__ICCARM__)
    RetVal = Flash_Hal_StartCmdInRam(FLASH, Wait_Timeout);
#else
    if (Flash_Hal_StartCmdInRamFunc == NULL_PTR)
    {
        /*PRQA S 0326 ++ # AMCE_FLS_0326.*/
        /*PRQA S 0305 ++ # AMCE_FLS_0305.*/
        (void)System_Memcpy(Flash_Hal_StartCmdInRam_Cpy, (void *)((uint32)Flash_Hal_StartCmdInRam & 0xFFFFFFFEU),
                sizeof(Flash_Hal_StartCmdInRam_Cpy));
        Flash_Hal_StartCmdInRamFunc = (Hal_StatusType (*)(FLASH_Type* Flash, uint32 Wait_Timeout))
                                    ((uint32)Flash_Hal_StartCmdInRam_Cpy | 0x1U);
        /*PRQA S 0305 -- # AMCE_FLS_0305.*/
        /*PRQA S 0326 -- # AMCE_FLS_0326.*/
    }
    RetVal = Flash_Hal_StartCmdInRamFunc(FLASH, Wait_Timeout);
#endif /* __ICCARM__ */
#endif

    return RetVal;
}
/*PRQA S 2889 -- # AMCE_FLS_2889.*/

#if defined (AC7840X) || ((defined (AC7842X))  && !defined (FLS_SDK_NON_EXTENDED_API))
/**
* @brief Performs software verification of flash memory content
* @details This function checks if the specified flash memory area contains only 0xFF values,
*          which indicates that the area has been properly erased.
*
* @note Function ID:[DES_FLS_API_239]
* @param [in] addr: Start address for verification
* @param [in] size: Size of memory area to verify in bytes
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Memory content verified successfully (all bytes are 0xFF)
* @retval STATUS_ERROR: Verification failed (at least one byte is not 0xFF)
* @pre None
* @post None
*/
static Hal_StatusType Flash_Hal_SW_Verify(uint32 addr, uint32 size)
{
    uint32 i;
    Hal_StatusType RetVal = STATUS_SUCCESS;

    for (i = 0; i < size; i++)
    {
        if (0xFFU != *(uint8 *)(addr + i))
        {
            RetVal = STATUS_ERROR;
            break;
        }
    }
    return RetVal;
}
#endif

#if defined (AC7842X)
/**
* @brief Callback function for LVD (Low Voltage Detection) interrupt
* @details This function is called when an LVD interrupt occurs. It sets the McuLvdFlag to indicate
*          that an LVD event has occurred.
*
* @note DES_FLS_API_241
* @param [in] para: Pointer to callback parameter (unused)
* @return void
* @pre An LVD interrupt must have occurred
* @post McuLvdFlag is set to 1
*/
/*PRQA S 3673 ++ # AMCE_FLS_3673.*/
static void Flash_Hal_LVD_Callback(void *para)
/*PRQA S 3673 -- # AMCE_FLS_3673.*/
{
    (void)para;
    /* Set EmuLvd status register to notify flash controller that LVD occurred */
    Flash_Reg_SetCnfgEmulvdStatusReg(1U);
    McuLvdFlag = 1U;
}
#endif

/**
* @brief Get Flash size based on partition configuration
* @details This function reads the Flash partition configuration from the flash controller or chip id
*          and calculates the actual Flash size based on the configuration value.
*
* @note DES_FLS_API_242
* @param [in] void
* @return void
* @pre Flash controller is initialized
* @post S_DFlashSize variable is updated with the correct D-Flash size
*       S_PFlashSize variable is updated with the correct P-Flash size
*/
static void Flash_Hal_GetFlashSize(void)
{
#if defined (AC7840E)
    uint32 ChipID, ChipSizeVal;

#define CHIP_SIZE_Msk (0x000FF0000U)
#define CHIP_SIZE_Pos (16U)
#define CHIP_SIZE_576KB (0x5AU)
#define CHIP_SIZE_320KB (0x59U)

    ChipID = Core_Hal_GetChipID();
    ChipSizeVal = (ChipID & CHIP_SIZE_Msk) >> CHIP_SIZE_Pos;

    if (ChipSizeVal == CHIP_SIZE_576KB)
    {
        S_PFlashSize = PFLASH_BLOCK_SIZE;
        S_DFlashSize = DFLASH_BLOCK_SIZE;
    }
    else if (ChipSizeVal == CHIP_SIZE_320KB)
    {
        S_PFlashSize = PFLASH_BLOCK_SIZE2;
        S_DFlashSize = DFLASH_BLOCK_SIZE2;
    }
    else
    {
        /* Invalid chip size value, set to default size */
        S_PFlashSize = PFLASH_BLOCK_SIZE;
        S_DFlashSize = DFLASH_BLOCK_SIZE;
    }
#else
    uint8 dFlashPartitionCode = 0U;
    dFlashPartitionCode = Flash_Reg_GetDPartition();
    if ((dFlashPartitionCode <= 0x08U) && ((dFlashPartitionCode & 1U) == 0U))
    {
        S_DFlashSize = (DFLASH_BLOCK_SIZE / 16U) * (uint32)dFlashPartitionCode;
    }
    else
    {
        S_DFlashSize = DFLASH_BLOCK_SIZE;
    }

    S_PFlashSize = PFLASH_BLOCK_SIZE;
#endif
}

/**
* @brief lock flash register write
* @details This function locks the flash controller registers to prevent accidental writes.
*
* @note Function ID:[DES_FLS_API_248]
* @param [in] void
* @return void
* @pre Flash controller must be initialized
* @post Flash controller registers are locked
*/
static void Flash_Hal_LockCtrl(void)
{
    Flash_Reg_Lock();
}

/**
* @brief unlock flash register write
* @details This function unlocks the flash controller registers to allow write operations.
*          It retries unlocking if initially unsuccessful, with a timeout mechanism.
*
* @note Function ID:[DES_FLS_API_209]
* @param [in] void
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Flash controller unlocked successfully
* @retval STATUS_TIMEOUT: Timeout occurred while trying to unlock
* @pre Flash controller must be initialized
* @post Flash controller registers are unlocked
*/
static Hal_StatusType Flash_Hal_UnLockCtrl(void)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    volatile uint32 TimeoutCount = 0x0U;
    volatile uint8 CtrlLockBit;

    CtrlLockBit = Flash_Reg_GetLockStatusReg();
    while (0x1U == CtrlLockBit)
    {
        Flash_Reg_UnLock();

        if (100U < TimeoutCount)
        {
            RetVal = STATUS_TIMEOUT;
            break;
        }
        CtrlLockBit = Flash_Reg_GetLockStatusReg();
        /*PRQA S 3387 ++ # AMCE_FLS_3387.*/
        TimeoutCount++;
        /*PRQA S 3387 -- # AMCE_FLS_3387.*/
    }
    return RetVal;
}

/* Implementation of Flash_Hal_SetPartition declared in Fls_Hal.h */
#if defined (AC7840X) || defined (AC7842X)
Hal_StatusType Flash_Hal_SetPartition(const Flash_CseType *CseConfig)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint32 LowData;
    uint32 HighData;

    DEVICE_ASSERT(CseConfig != NULL_PTR);

#if defined(AC7842X)
    if ((CseConfig->KeySize  != FLASH_CSESIZE_0BYTES)  &&
        (CseConfig->PartSize != FLASH_DLFASHSIZE_0KB)  &&
        (CseConfig->PartSize != FLASH_DLFASHSIZE_32KB) &&
        (CseConfig->PartSize != FLASH_DLFASHSIZE_64KB) &&
        (CseConfig->PartSize != FLASH_DLFASHSIZE_96KB) &&
        (CseConfig->PartSize != FLASH_DLFASHSIZE_128KB))
    {
        RetVal = STATUS_ERROR;
    }
    else
#endif /* defined(AC7842X) */
    {
        OSIF_ENTER_CRITICAL(FLS_HAL_ID3);
        (void)Flash_Hal_UnLockCtrl();

        /* Create CSE partition*/
        /*PRQA S 2812 ++ # AMCE_FLS_2812.*/
#if defined(AC7842X)
        if ((FLASH_CSESIZE_0BYTES != CseConfig->KeySize) &&
            (CseConfig->PartSize != FLASH_DLFASHSIZE_128KB))
#else /* AC7840X */
        if (FLASH_CSESIZE_0BYTES != CseConfig->KeySize)
#endif /* AC7840X */
        /*PRQA S 2812 -- # AMCE_FLS_2812.*/
        {
#if defined(AC7840X)
            LowData = (uint32)(0x208U | ((uint32)CseConfig->Sfe << 24U));
#else /* AC7842X */
            LowData = (uint32)(0x200U | ((uint32)CseConfig->Sfe << 24U) | ((uint32)CseConfig->PartSize));
#endif /* AC7840X */
            HighData = (uint32)CseConfig->KeySize;
            /* Clear RDCOLERR & ACCERR & FPVIOL flag in EFLASH_MODULE status register.
            Write 1 to clear */
            Flash_Reg_ClearStatus();
            /* Passing parameter to the command */
            Flash_Reg_SetCommandReg(FLASH_PROGRAM_PARTITION);

            Flash_Reg_SetLowDataReg(LowData);
            Flash_Reg_SetHighDataReg(HighData);

            RetVal = Flash_Hal_StartCmd(0U);
        }
        else /* Delete CSE Partition */
        {
            /* Clear RDCOLERR & ACCERR & FPVIOL flag in EFLASH_MODULE status register.
            Write 1 to clear */
            Flash_Reg_ClearStatus();
            /* Passing parameter to the command */
            Flash_Reg_SetCommandReg(FLASH_ERASE_PARTITION);
            RetVal = Flash_Hal_StartCmd(0U);
        }
        (void)Flash_Hal_LockCtrl();
        OSIF_EXIT_CRITICAL(FLS_HAL_ID3);
        Flash_Hal_GetFlashSize();
    }

    return RetVal;
}
#else /* AC7843X AC7840E */
Hal_StatusType Flash_Hal_SetPartition(const Flash_CseType *CseConfig)
{
    (void)CseConfig;
    return STATUS_UNSUPPORTED;
}
#endif

/* Implementation of Flash_Hal_Init declared in Fls_Hal.h */
/*PRQA S 3006 ++ # # AMCE_FLS_3006*/
void Flash_Hal_Init(const Flash_Config *Config)
{

    /* Use the default configuration when the Fls_Cfg is empty and uninitialized */
    if (Config != NULL_PTR)
    {
        S_FlsCfg.Write_Timeout = Config->Write_Timeout;
        S_FlsCfg.Erase_Timeout = Config->Erase_Timeout;
        S_FlsCfg.LVD_Enable = Config->LVD_Enable;
    }

    (void)Flash_Hal_UnLockCtrl();

#if defined (AC7842X)
    Spm_Hal_SetFlashLVDCallback((Hal_CallbackType)Flash_Hal_LVD_Callback);
#else
    /*PRQA S 2812 ++ # AMCE_FLS_2812.*/
    Flash_Reg_SetLVDDetection(S_FlsCfg.LVD_Enable);
    /*PRQA S 2812 -- # AMCE_FLS_2812.*/
#endif
    (void)Flash_Hal_LockCtrl();

    Flash_Hal_GetFlashSize();

    /* Wait flash ready. */
    /*PRQA S 1006 ++ # AMCE_FLS_1006.*/
    while (0U == Flash_Reg_GetCommandCompleteStatusReg())
    {
        __asm("nop");
    }
    /*PRQA S 1006 -- # AMCE_FLS_1006.*/
}
/*PRQA S 3006 -- # # AMCE_FLS_3006*/

/**
* @brief Execute flash erase operation
* @details This function performs flash erase operations based on the command type and address provided.
*          It checks if the previous command is completed before executing the new command.
*
* @note DES_FLS_API_243
* @param [in] CommandType: Type of erase command to execute
* @param [in] Address: Address where erase operation should be performed
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Erase operation completed successfully
* @retval STATUS_ERROR: Error occurred during erase operation
* @pre Flash controller must be initialized and unlocked
* @post Specified flash area is erased
*/
static Hal_StatusType Flash_Hal_Erase
(
    uint8 CommandType,
    uint32 Address
)
{
    Hal_StatusType RetVal;

    /* Check CCIF to verify the previous command is completed */
    if (Flash_Reg_GetCommandCompleteStatusReg() != 0U)
    {
        /* Clear RDCOLERR & ACCERR & FPVIOL flag in EFLASH_MODULE status register.
        Write 1 to clear */
        Flash_Reg_ClearStatus();

        /* Passing parameter to the command */
        Flash_Reg_SetCommandReg(CommandType);
        if (CommandType != FLASH_ERASE_ALL_BLOCK_UNSECURE)
        {
#if defined (AC7840E)
            FLASH->CMDCFG = (Address >= DFLASH_BASE_ADDRESS) ? 0U : 1U;
#endif
            Flash_Reg_SetAddressReg(Address);
        }
        RetVal = Flash_Hal_StartCmd(S_FlsCfg.Erase_Timeout);
    }
    else
    {
        RetVal = STATUS_ERROR;
    }

    return RetVal;
}

#ifndef FLS_SDK_NON_EXTENDED_API
/* Implementation of Flash_Hal_WPInfoErase declared in Fls_Hal.h */
Hal_StatusType Flash_Hal_WPInfoErase(void)
{
    Hal_StatusType RetVal;
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Erase(FLASH_OPTION_PAGE_ERASE, WP_INFO_BASE);
    (void)Flash_Hal_LockCtrl();

    return RetVal;
}
#endif

/* Implementation of Flash_Hal_AllErase declared in Fls_Hal.h */
Hal_StatusType Flash_Hal_AllErase(void)
{
    Hal_StatusType RetVal;

    /*has not CSE partition can erase all block, 0xF means no cse partition*/
    if (0xfU == Flash_Reg_GetDPartition())
    {
#if defined (AC7842X) || defined (AC7840E)
        /* Disable Lockup reset */
        CKGEN->RCM_EN &= 0xFFFFFFFDU;
#endif
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Erase(FLASH_ERASE_ALL_BLOCK_UNSECURE, FLASH_INVALID_ADDR);
        (void)Flash_Hal_LockCtrl();
    }
    else
    {
        RetVal = STATUS_ERROR;
    }

    return RetVal;
}

/**
* @brief Write data to flash memory
* @details This function writes data to flash memory using the specified command type.
*          It handles programming of phrases/sections and manages the address and data pointers.
*
* @note Function ID:[DES_FLS_API_203]
* @param [in] CommandType: flash command type
* @param [in] Address: Write Address
* @param [in] Length: Data length
* @param [in] DataPtr: Data pointer
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Write operation completed successfully
* @retval STATUS_ERROR: Error occurred during write operation
* @retval STATUS_TIMEOUT: Write operation timed out
* @pre Flash controller must be initialized and unlocked
* @post Data is written to flash memory at the specified address
*/
static Hal_StatusType Flash_Hal_Write
(
    uint8 CommandType,
    uint32 Address,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint32 TmpAddress = Address;
    uint32 DataLength = Length;
    uint32 i = 0;

    /*PRQA S 0310 ++ # AMCE_FLS_3010.*/
    /*PRQA S 3305 ++ # AMCE_FLS_3305.*/
    const uint32 *Flash_SourceAddressPtr = (const uint32 *)(DataPtr);
    /*PRQA S 3305 -- # AMCE_FLS_3305.*/
    /*PRQA S 0310 -- # AMCE_FLS_3010.*/
    /* Check CCIF to verify the previous command is completed */
    if ((((Address % PFLASH_WRITE_UNIT_SIZE) != 0U) ||
        ((Length % PFLASH_WRITE_UNIT_SIZE) != 0U)) && (Address != FLASH_INVALID_ADDR))
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else
    {
        while ((STATUS_SUCCESS == RetVal) && (DataLength > 0U))
        {
            if (Flash_Reg_GetCommandCompleteStatusReg() != 0x0U)
            {
                /* Clear RDCOLERR & ACCERR & FPVIOL flag in EFLASH_MODULE status register.
                Write 1 to clear */
                Flash_Reg_ClearStatus();

                /* Passing parameter to the command */
                Flash_Reg_SetCommandReg(CommandType);
                if (CommandType != FLASH_SECURITY_BY_PASS)
                {
#if defined (AC7840E)
                    FLASH->CMDCFG = (Address >= DFLASH_BASE_ADDRESS) ? 0U : 1U;
#endif
                    Flash_Reg_SetAddressReg(TmpAddress);
                }

                Flash_Reg_SetLowDataReg(Flash_SourceAddressPtr[i]);
                Flash_Reg_SetHighDataReg(Flash_SourceAddressPtr[i + 1U]);

                /* It is time to do word programming */
                RetVal = Flash_Hal_StartCmd(S_FlsCfg.Write_Timeout);

                /* start internal erase/program sequence */
                if (CommandType != FLASH_SECURITY_BY_PASS)
                {
                    /*This may compare with */
                    TmpAddress += PFLASH_WRITE_UNIT_SIZE;
                }
                DataLength -= PFLASH_WRITE_UNIT_SIZE;
                i += 2U;
            }
            else
            {
                RetVal = STATUS_ERROR;
            }
        }
    }

    return RetVal;
}

#ifndef FLS_SDK_NON_EXTENDED_API
/* Implementation of Flash_Hal_WriteOtp declared in Fls_Hal.h */
#if defined (AC7843X) || defined (AC7840E)
Hal_StatusType Flash_Hal_WriteOtp
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    (void)Offset;
    (void)Length;
    (void)DataPtr;
    return STATUS_UNSUPPORTED;
}
#else /*AC7840X AC7842X */
Hal_StatusType Flash_Hal_WriteOtp
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal;
    DEVICE_ASSERT(DataPtr != NULL_PTR);
    DEVICE_ASSERT((Offset + Length) <= OTP_INFO_SIZE);

    uint32 WriteAddress = OTP_INFO_BASE + Offset;
    if ((Offset + Length) > OTP_INFO_SIZE)
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Write(FLASH_PROGRAM_ONCE, WriteAddress, Length, DataPtr);
        (void)Flash_Hal_LockCtrl();
    }
    return RetVal;
}
#endif

/* Implementation of Flash_Hal_ErrorStatusGetAndClear declared in Fls_Hal.h */
uint32 Flash_Hal_ErrorStatusGetAndClear(uint32 *Addr)
{
    uint32 RetVal;

    DEVICE_ASSERT(NULL_PTR != Addr);

    RetVal = Flash_Reg_GetStatusReg();
    /*clear error status*/
    Flash_Reg_ClearStatus();
    /*get ecc error address*/
    if (FLASH_STAT_DFDIF_Msk == (RetVal & FLASH_STAT_DFDIF_Msk))
    {
        /*PRQA S 2812 ++ # AMCE_FLS_2812.*/
        *Addr = Flash_Reg_ECCErrorAddr();
        /*PRQA S 2812 -- # AMCE_FLS_2812.*/
    }
    return RetVal;
}
#endif

/**
* @brief Verify flash memory content
* @details This function verifies the content of flash memory using the specified command type.
*          It checks if the flash content matches the expected values.
*
* @note Function ID:[DES_FLS_API_215]
* @param [in] CommandType: flash command type for verification
* @param [in] Address: Address to verify
* @param [in] Length: Length of data to verify
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Verification successful
* @retval STATUS_ERROR: Verification failed
* @retval STATUS_BUSY: Flash controller is busy
* @pre Flash controller must be initialized
* @post None
*/
static Hal_StatusType Flash_Hal_Verify
(
    uint8 CommandType,
    uint32 Address,
    uint32 Length
)
{
    Hal_StatusType RetVal;

    /*Check flash whether process complete*/
    if (0x0U == Flash_Reg_GetCommandCompleteStatusReg())
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        Flash_Reg_ClearStatus();

        Flash_Reg_SetCommandReg(CommandType);
        if (CommandType != FLASH_VERIFY_ALL_BLOCK)
        {
            Flash_Reg_SetAddressReg(Address);
            Flash_Reg_SetLengthReg(Length >> 3U); /*one means eight address length, so need divide 8 */
        }

        RetVal = Flash_Hal_StartCmd(0U);
    }
    return RetVal;
}

#ifndef FLS_SDK_NON_EXTENDED_API
/* Implementation of Flash_Hal_AllVerify declared in Fls_Hal.h */
Hal_StatusType Flash_Hal_AllVerify(void)
{
    Hal_StatusType RetVal = STATUS_ERROR;

#if defined (AC7840X) || defined (AC7843X) || defined (AC7840E)
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Verify(FLASH_VERIFY_ALL_BLOCK, FLASH_INVALID_ADDR, 8U);
    (void)Flash_Hal_LockCtrl();

#if defined (AC7840X) || defined (AC7842X)
    if (STATUS_ERROR == RetVal)
    {
        /* software to verify */
        RetVal = Flash_Hal_SW_Verify(PFLASH_BASE, S_PFlashSize);
        if (STATUS_SUCCESS == RetVal)
        {
            RetVal = Flash_Hal_SW_Verify(DFLASH_BASE, S_DFlashSize);
        }
    }
#endif

#else /* AC7842X */
        /* software to verify */
        RetVal = Flash_Hal_SW_Verify(PFLASH_BASE, S_PFlashSize);
        if (STATUS_SUCCESS == RetVal)
        {
            RetVal = Flash_Hal_SW_Verify(DFLASH_BASE, S_DFlashSize);
        }
#endif

    return RetVal;
}
#endif

/* Implementation of Flash_Hal_ReadProtectSet declared in Fls_Hal.h */

#if defined (AC7840X) || defined (AC7843X) || defined (AC7842X)
Hal_StatusType Flash_Hal_ReadProtectSet(void)
{
    uint32 Val[2U];
    Hal_StatusType RetVal;

    /*enable Read protect*/
    Val[0] = (READ_MEM32(COMP_CTRL_BASE) & 0xFFFFFFFCU);
    Val[1] = 0xFFFFFFFFU;
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, COMP_CTRL_BASE, 8U, (uint8 *)Val);
    (void)Flash_Hal_LockCtrl();

    return RetVal;
}
#else /* AC7840E */
Hal_StatusType Flash_Hal_ReadProtectSet(void)
{
    return STATUS_UNSUPPORTED;
}
#endif

#ifndef FLS_SDK_NON_EXTENDED_API
/* Implementation of Flash_Hal_WriteProtectSet declared in Fls_Hal.h */
Hal_StatusType Flash_Hal_WriteProtectSet(boolean En, const uint8 *Data)
{
    Hal_StatusType RetVal;

    if (TRUE == En)
    {
        DEVICE_ASSERT(Data != NULL_PTR);
        /*enable write protect */
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, WP_INFO_BASE, FLASH_WPROT_BYTES, Data);
        (void)Flash_Hal_LockCtrl();
    }
    else
    {
        /*disable write protect */
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Erase(FLASH_OPTION_PAGE_ERASE, WP_INFO_BASE);
        (void)Flash_Hal_LockCtrl();
    }

    return RetVal;
}
#endif

/* Implementation of Flash_Hal_BackdoorSet declared in Fls_Hal.h */
#if defined (AC7840X) || defined (AC7843X) || defined (AC7842X)
Hal_StatusType Flash_Hal_BackdoorSet(const uint8 *Data)
{
    uint32 Val[2U];
    Hal_StatusType RetVal;

    Val[0] = (READ_MEM32(COMP_CTRL_BASE) & 0xFFFFFFCFU);
    Val[1] = 0xFFFFFFFFU;
    (void)Flash_Hal_UnLockCtrl();
    /*enable backdoor */
    RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, COMP_CTRL_BASE, 8U, (uint8 *)Val);
    if (STATUS_SUCCESS == RetVal)
    {
        /*set backdoor key*/
        RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, BACKDOOR_KEY_BASE, 8U, Data);
    }
    (void)Flash_Hal_LockCtrl();

    return RetVal;
}

/* Implementation of Flash_Hal_BackdoorVerify declared in Fls_Hal.h */
Hal_StatusType Flash_Hal_BackdoorVerify(const uint8 *Data)
{
    Hal_StatusType RetVal = STATUS_ERROR;
    DEVICE_ASSERT(Data != NULL_PTR);

    /*backdoor whether enable ,enable can verify,disable return error*/
    if ((READ_MEM32(COMP_CTRL_BASE) & 0x30U) != 0x30U)
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Write(FLASH_SECURITY_BY_PASS, FLASH_INVALID_ADDR, 8U, Data);
        (void)Flash_Hal_LockCtrl();
        if ((RetVal == STATUS_SUCCESS) && (Flash_Reg_GetBackdoorStatusReg() != 0U))
        {
            RetVal = STATUS_ERROR;
        }
    }
    return RetVal;
}
#else /* AC7840E */
Hal_StatusType Flash_Hal_BackdoorSet(const uint8 *Data)
{
    (void)Data;
    return STATUS_UNSUPPORTED;
}
Hal_StatusType Flash_Hal_BackdoorVerify(const uint8 *Data)
{
    (void)Data;
    return STATUS_UNSUPPORTED;
}
#endif

#ifndef FLS_SDK_NON_EXTENDED_API
/* Implementation of Flash_Hal_EraseCseInfo declared in Fls_Hal.h */
#if defined (AC7840X) || defined (AC7842X)
Hal_StatusType Flash_Hal_EraseCseInfo(void)
{
    Hal_StatusType RetVal = STATUS_BUSY;

    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, CSE_SECRET_KEY_BASE);
    (void)Flash_Hal_LockCtrl();

    return RetVal;
}

/* Implementation of Flash_Hal_ProgramCseInfo declared in Fls_Hal.h */
Hal_StatusType Flash_Hal_ProgramCseInfo
(
    uint32 Addr,
    uint32 Length,
    const uint8 *Data
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint32 TmpAddr = Addr;
    uint32 TmpLength = Length;
    uint32 i = 0;
    /*PRQA S 0310 ++ # AMCE_FLS_3010.*/
    /*PRQA S 3305 ++ # AMCE_FLS_3305.*/
    const uint32 *DataPtr = (const uint32 *)Data;
    /*PRQA S 3305 -- # AMCE_FLS_3305.*/
    /*PRQA S 0310 -- # AMCE_FLS_3010.*/
    DEVICE_ASSERT(0U == (TmpLength % PFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT((TmpAddr >= CSE_SECRET_KEY_BASE) && (TmpAddr <= CSE_LOCK_BASE));
    DEVICE_ASSERT(NULL_PTR != Data);

    if(0U != (TmpLength % PFLASH_WRITE_UNIT_SIZE))
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else
    {
        (void)Flash_Hal_UnLockCtrl();
        while ((TmpLength > 0U) && (STATUS_SUCCESS == RetVal))
        {
            /* Check if command is completed by CCIF */
            if (Flash_Reg_GetCommandCompleteStatusReg() != 0U)
            {
                /* Write 1 to clear COLLERR & ACCERR & FPVIOL flag in FLASH status register */
                Flash_Reg_ClearStatus();

                /* Set flash execute command */
                Flash_Reg_SetCommandReg(FLASH_PROGRAM_CSE);
                Flash_Reg_SetAddressReg(TmpAddr);

                Flash_Reg_SetLowDataReg(DataPtr[i]);
                Flash_Reg_SetHighDataReg(DataPtr[i + 1U]);

                /* Calling FLASH start command sequence function to execute the command */
                RetVal = Flash_Hal_StartCmd(0U);

                /* Update program address for next operation */
                TmpAddr += PFLASH_WRITE_UNIT_SIZE;
                /* Update size for next operation */
                TmpLength -= PFLASH_WRITE_UNIT_SIZE;
                /* Increase the data source pointer by 8 */
                i += 2U;
            }
            else
            {
                RetVal = STATUS_BUSY;
            }
        }
        (void)Flash_Hal_LockCtrl();
    }
    return RetVal;
}
#else /*AC7843X AC7840E */
Hal_StatusType Flash_Hal_EraseCseInfo(void)
{
    return STATUS_UNSUPPORTED;
}
Hal_StatusType Flash_Hal_ProgramCseInfo
(
    uint32 Addr,
    uint32 Length,
    const uint8 *Data
)
{
    (void)Addr;
    (void)Length;
    (void)Data;
    return STATUS_UNSUPPORTED;
}
#endif

/* Implementation of Flash_Hal_Swap declared in Fls_Hal.h */
#if defined (AC7842X) || defined (AC7843X)
Hal_StatusType Flash_Hal_Swap(boolean En)
{
    uint32 Data[4U];
    Hal_StatusType RetVal;

    /* program swap location */
    if (En == TRUE)
    {
        Data[0] = 0x00U;
        Data[1] = 0x00U;
        Data[2] = 0x00U;
        Data[3] = 0x00U;
        (void)Flash_Hal_UnLockCtrl();
        /* config swap location */
        RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, SWAP_INFO_BASE, 16U, (uint8 *)Data);
        (void)Flash_Hal_LockCtrl();
    }
    else /* erase swap location */
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, SWAP_INFO_BASE);
        (void)Flash_Hal_LockCtrl();
    }

    return RetVal;
}
#elif defined (AC7840E)
Hal_StatusType Flash_Hal_Swap(boolean En)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    /*PRQA S 3678 ++ # AMCE_FLS_3678.*/
    uint32 SwapKey[2] = {0xAC09D7CBU, 0xBE2675E0U};
    /*PRQA S 3678 -- # AMCE_FLS_3678.*/
    uint32 SwapBaseAddr = DFLASH_BASE + S_DFlashSize - DFLASH_SWAP_BLOCK_SIZE;
    uint32 ConfigurationAddress;
    uint32 EraseAddress;
    uint8 SwapBState = (En == 1U) ? 1U : 0U;

    /* Check if swap functionality is enabled in hardware */
    if (0U == (FLASH->SWAPST & FLASH_SWAPST_EN_Msk))
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    /* Both Swap Pages 0 and 1 are disabled */
    else if (0U == ((FLASH->SWAPST & FLASH_SWAPST_PAGEEN_Msk) >> FLASH_SWAPST_PAGEEN_Pos))
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, SwapBaseAddr, DFLASH_WRITE_UNIT_SIZE, (uint8 *)SwapKey);
        (void)Flash_Hal_LockCtrl();
        if (RetVal ==  STATUS_SUCCESS)
        {
            RetVal = STATUS_FLS_RESET_AND_RECON_REQUIRED;
        }
    }
    /* Check if swap is already in requested state */
    else if (SwapBState == (uint8)(FLASH->SWAPST & FLASH_SWAPST_FLAG_Msk))
    {
        RetVal = STATUS_SUCCESS;
    }
    else
    {
        (void)Flash_Hal_UnLockCtrl();

        /* Determine if configuration change is needed based on remaining swap operations */
        uint8 NeedConfigurationChange =
            (((FLASH->SWAPST & FLASH_SWAPST_RESNUN_Msk) >> FLASH_SWAPST_RESNUN_Pos) < 2U) ? 1U : 0U;

        /* Determine current page state for address calculation */
        uint8 CurrentPageState = (uint8)((FLASH->SWAPST & FLASH_SWAPST_PAGEEN_Msk) >> FLASH_SWAPST_PAGEEN_Pos);

        /* Swap Page 1 is enabled */
        if ((CurrentPageState == 3U) || (CurrentPageState == 2U))
        {
            ConfigurationAddress = (NeedConfigurationChange == 1U) ? SwapBaseAddr : (SwapBaseAddr + DFLASH_PAGE_SIZE);
            EraseAddress = (NeedConfigurationChange == 1U) ? (SwapBaseAddr + DFLASH_PAGE_SIZE) : 0x0U;
        }
        /* Swap Page 0 is enabled */
        else if (CurrentPageState == 1U)
        {
            ConfigurationAddress = (NeedConfigurationChange == 1U)? (SwapBaseAddr + DFLASH_PAGE_SIZE) : SwapBaseAddr;
            EraseAddress = (NeedConfigurationChange == 1U) ? SwapBaseAddr : 0x0U;
        }
        /* Swap Page 0 1 diabled */
        else
        {
            RetVal = STATUS_ERROR;
        }

        if (RetVal == STATUS_SUCCESS)
        {
            if (NeedConfigurationChange == 1U)
            {
                /* Full configuration change: erase, write both locations, then erase old page */
                RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, ConfigurationAddress);

                /* Write first swap key location if previous operation was successful */
                if (STATUS_SUCCESS == RetVal)
                {
                    RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, ConfigurationAddress,
                                             DFLASH_WRITE_UNIT_SIZE, (uint8 *)SwapKey);
                }

                /* Write second swap key location if previous operation was successful */
                if (STATUS_SUCCESS == RetVal)
                {
                    RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, ConfigurationAddress + 0x10U,
                                             DFLASH_WRITE_UNIT_SIZE, (uint8 *)SwapKey);
                }

                /* Erase old page if applicable and previous operations were successful */
                if ((STATUS_SUCCESS == RetVal))
                {
                    RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, EraseAddress);
                }
            }
            else
            {
                /* Simple toggle: only program second location */
                RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, ConfigurationAddress + 0x10U,
                                         DFLASH_WRITE_UNIT_SIZE, (uint8 *)SwapKey);
            }
        }

        (void)Flash_Hal_LockCtrl();
    }

    return RetVal;
}
#else
Hal_StatusType Flash_Hal_Swap(boolean En)
{
    (void)En;
    return STATUS_UNSUPPORTED;
}
#endif
#endif

/* Implementation of Flash_Hal_InfoRead declared in Fls_Hal.h */
void Flash_Hal_InfoRead
(
    uint32 Offset,
    uint32 Length,
    uint8 *DataPtr
)
{
    uint32 TargetAddress;
    uint32 DataLength = Length;
    uint8 *TmpDataPtr = DataPtr;

    DEVICE_ASSERT(Offset < FLASH_INFO_ADDR_SIZE);
    DEVICE_ASSERT((Offset + Length) <= FLASH_INFO_ADDR_SIZE);
    DEVICE_ASSERT(DataPtr != NULL_PTR);
    TargetAddress = FLASH_INFO_ADDR_BASE + Offset;

    /* End of read pointer */
    while (0x0U < DataLength)
    {
        /* read flash location */
        *TmpDataPtr = *(uint8 *)TargetAddress;
        TmpDataPtr++;
        TargetAddress++;
        DataLength -= 0x1U;
    }
}

/* Implementation of Flash_Hal_InfoWrite declared in Fls_Hal.h */
Hal_StatusType Flash_Hal_InfoWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal;
    uint32 TargetAddress;

    DEVICE_ASSERT(DataPtr != NULL_PTR);
    DEVICE_ASSERT((Offset + Length) <= FLASH_INFO_ADDR_SIZE);
    DEVICE_ASSERT(0U == (Offset % PFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT(0U == (Length % PFLASH_WRITE_UNIT_SIZE));

    TargetAddress = FLASH_INFO_ADDR_BASE + Offset;
    if ((Offset + Length) > FLASH_INFO_ADDR_SIZE)
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, TargetAddress, Length, DataPtr);
        (void)Flash_Hal_LockCtrl();
    }
    return RetVal;
}

#ifndef FLS_SDK_NON_EXTENDED_API

/* Implementation of Flash_Hal_InfoErase declared in Fls_Hal.h */
Hal_StatusType Flash_Hal_InfoErase(uint32 Offset)
{
    Hal_StatusType RetVal;
    uint32 TargetAddress;

    DEVICE_ASSERT(0U == (Offset % PFLASH_PAGE_SIZE));
    TargetAddress = FLASH_INFO_ADDR_BASE + Offset;
    if (Offset < FLASH_INFO_ADDR_SIZE)
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, TargetAddress);
        (void)Flash_Hal_LockCtrl();
    }
    else
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    return RetVal;
}
#endif

#ifdef PFLASH_ENABLE
/* Implementation of PFlash_Hal_PageErase declared in Fls_Hal.h */
Hal_StatusType PFlash_Hal_PageErase(uint32 Offset)
{
    Hal_StatusType RetVal;
    uint32 TargetAddr;

    DEVICE_ASSERT(Offset < S_PFlashSize);
    DEVICE_ASSERT(0U == (Offset % PFLASH_PAGE_SIZE));

    /*PRQA S 2986 ++ # AMCE_FLS_2986.*/
    TargetAddr = PFLASH_BASE + Offset;
    /*PRQA S 2986 -- # AMCE_FLS_2986.*/
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, TargetAddr);
    (void)Flash_Hal_LockCtrl();
    return RetVal;
}

/**
* @brief erase pflash area with block erase
* @details This function erases a complete P-Flash block. It can only be used when there is no CSE partition.
*
* @note Function ID:[DES_FLS_API_219]
* @param [in] void
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Erase operation completed successfully
* @retval STATUS_ERROR: Error occurred during erase operation or CSE partition exists
* @pre Flash controller must be initialized and unlocked
* @post P-Flash block is erased
*/
/*PRQA S 3408 ++ # AMCE_FLS_3408.*/
Hal_StatusType PFlash_Hal_BlockErase(void)
{
    Hal_StatusType RetVal;

    /*has not CSE partition can erase all block, 0xF means no cse partition*/
    if (0xfU == Flash_Reg_GetDPartition())
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Erase(FLASH_ERASE_BLOCK, PFLASH_BASE);
        (void)Flash_Hal_LockCtrl();
    }
    else
    {
        RetVal = STATUS_ERROR;
    }

    return RetVal;
}
/*PRQA S 3408 -- # AMCE_FLS_3408.*/

/* Implementation of PFlash_Hal_PageWrite declared in Fls_Hal.h */
Hal_StatusType PFlash_Hal_PageWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal;
    DEVICE_ASSERT(DataPtr != NULL_PTR);
    DEVICE_ASSERT(0U == (Offset % PFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT(0U == (Length % PFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT((Offset + Length) <= S_PFlashSize);

    /*PRQA S 2986 ++ # AMCE_FLS_2986.*/
    uint32 WriteAddress = PFLASH_BASE + Offset;
    if ((Offset + Length) > S_PFlashSize)
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else
    {
        /*PRQA S 2986 -- # AMCE_FLS_2986.*/
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, WriteAddress, Length, DataPtr);
        (void)Flash_Hal_LockCtrl();
    }
    return RetVal;
}

#ifndef FLS_SDK_NON_EXTENDED_API
/* Implementation of PFlash_Hal_SectionWrite declared in Fls_Hal.h */
#if defined (AC7843X) || defined (AC7840E)
Hal_StatusType PFlash_Hal_SectionWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    (void)Offset;
    (void)Length;
    (void)DataPtr;
    return STATUS_UNSUPPORTED;
}
#else /* AC7840X AC7842X */
Hal_StatusType PFlash_Hal_SectionWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_ERROR;
    uint32 Len = Length;
    uint32 Addr = FLEXRAM_BASE;
    const uint8 *DPtr = DataPtr;

    DEVICE_ASSERT(((Offset % PFLASH_PAGE_SIZE) + Length) <= PFLASH_PAGE_SIZE);
    DEVICE_ASSERT(0U == (Offset % PFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT(0U == (Length % PFLASH_WRITE_UNIT_SIZE));

    DEVICE_ASSERT(DataPtr != NULL_PTR);

    /*PRQA S 2986 ++ # AMCE_FLS_2986.*/
    uint32 WriteAddress = PFLASH_BASE + Offset;

    if ((0U != (Length % PFLASH_WRITE_UNIT_SIZE)) || ((Length + Offset) > S_PFlashSize))
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else if (0xFU != Flash_Reg_GetEPartition())
    {
        /*if true means CSE not use flex ram*/
        RetVal = STATUS_ERROR;
    }
    else
    {
        /*PRQA S 2986 -- # AMCE_FLS_2986.*/
        (void)Flash_Hal_UnLockCtrl();
        /*set flex ram*/
        Flash_Reg_SetCommandReg(FLASH_SET_FLEXRAM);
        Flash_Reg_SetAddressReg(0xFFU);
        RetVal = Flash_Hal_StartCmd(0U);
        /*set data to flex ram*/
        while (Len > 0U)
        {
            *(uint8 *)Addr = *DPtr;
            DPtr++;
            Addr++;
            Len--;
        }

        if (STATUS_SUCCESS == RetVal)
        {
            Addr = FLEXRAM_BASE;
            Flash_Reg_SetLengthReg(Length / PFLASH_WRITE_UNIT_SIZE);
            RetVal = Flash_Hal_Write(FLASH_PROGRAM_SECTION, WriteAddress, PFLASH_WRITE_UNIT_SIZE, (uint8 *)Addr);
        }

        (void)Flash_Hal_LockCtrl();
    }
    return RetVal;
}
#endif
#endif

/* Implementation of PFlash_Hal_SectionVerify declared in Fls_Hal.h */
Hal_StatusType PFlash_Hal_SectionVerify
(
    uint32 Offset,
    uint32 Length
)
{
    Hal_StatusType RetVal;
    DEVICE_ASSERT((Offset + Length) <= S_PFlashSize);

    /*PRQA S 2986 ++ # AMCE_FLS_2986.*/
    uint32 WriteAddress = PFLASH_BASE + Offset;
    /*PRQA S 2986 -- # AMCE_FLS_2986.*/
    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Verify(FLASH_VERIFY_SECTION, WriteAddress, Length);
    (void)Flash_Hal_LockCtrl();

#if defined (AC7840X)
    if (STATUS_ERROR == RetVal)
    {
        /* software to verify */
        RetVal = Flash_Hal_SW_Verify(WriteAddress, Length);
    }
#endif
    return RetVal;
}

#ifndef FLS_SDK_NON_EXTENDED_API
/* Implementation of PFlash_Hal_BlockVerify declared in Fls_Hal.h */
Hal_StatusType PFlash_Hal_BlockVerify(void)
{
    Hal_StatusType RetVal;

    (void)Flash_Hal_UnLockCtrl();
    RetVal = Flash_Hal_Verify(FLASH_VERIFY_BLOCK, PFLASH_BASE, 8U);
    (void)Flash_Hal_LockCtrl();

#if defined (AC7840X)
    if (STATUS_ERROR == RetVal)
    {
        /* software to verify */
        RetVal = Flash_Hal_SW_Verify(PFLASH_BASE, S_PFlashSize);
    }
#endif
    return RetVal;
}
#endif

/* Implementation of PFlash_Hal_Read declared in Fls_Hal.h */
Hal_StatusType PFlash_Hal_Read
(
    uint32 Offset,
    uint32 Length,
    uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint32 ReadAddress;
    uint32 DataLength = Length;
    uint8 *TmpDataPtr = DataPtr;

    DEVICE_ASSERT((Offset + Length) <= S_PFlashSize);
    DEVICE_ASSERT(DataPtr != NULL_PTR);

    /*PRQA S 2986 ++ # AMCE_FLS_2986.*/
    ReadAddress = PFLASH_BASE + Offset;
    /*PRQA S 2986 -- # AMCE_FLS_2986.*/
    /* End of read pointer */
    while (0x0U < DataLength)
    {
        /* read flash location */
        *TmpDataPtr = *(uint8 *)ReadAddress;
        TmpDataPtr++;
        ReadAddress++;
        DataLength -= 0x1U;
    }

    return RetVal;
}
#endif

#ifdef DFLASH_ENABLE
/* Implementation of DFlash_Hal_PageErase declared in Fls_Hal.h */
Hal_StatusType DFlash_Hal_PageErase(uint32 Offset)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint32 TargetAddr;
    FlashDeviceType DevLock;

    DEVICE_ASSERT(Offset < S_DFlashSize);
    DEVICE_ASSERT(0U == (Offset % DFLASH_PAGE_SIZE));
    if (Offset < S_DFlashSize)
    {
        TargetAddr = DFLASH_BASE + Offset;

        DevLock = System_FlsDeviceTryLock(FLS_DEV);
        if (DevLock != FLS_DEV)
        {
            RetVal = STATUS_BUSY;
        }
        else
        {
            (void)Flash_Hal_UnLockCtrl();
            RetVal = Flash_Hal_Erase(FLASH_ERASE_PAGE, TargetAddr);
            (void)Flash_Hal_LockCtrl();
            (void)System_FlsDeviceUnlock(FLS_DEV);
        }
    }
    else
    {
        RetVal = STATUS_UNSUPPORTED;
    }

    return RetVal;
}

#ifndef FLS_SDK_NON_EXTENDED_API
/**
* @brief erase dflash area with block erase
* @details This function erases a complete D-Flash block. It can only be used when there is no CSE partition.
*
* @note Function ID: [DES_FLS_API_226]
* @param [in] void
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Erase operation completed successfully
* @retval STATUS_ERROR: Error occurred during erase operation or CSE partition exists
* @retval STATUS_BUSY: Device is busy
* @pre Flash controller must be initialized and unlocked
* @post D-Flash block is erased
*/
Hal_StatusType DFlash_Hal_BlockErase(void)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    FlashDeviceType DevLock;

    /*has not CSE partition can erase all block, 0xF means no cse partition*/
    if (0xfU == Flash_Reg_GetDPartition())
    {
        DevLock = System_FlsDeviceTryLock(FLS_DEV);
        if (DevLock != FLS_DEV)
        {
            RetVal = STATUS_BUSY;
        }
        else
        {
            (void)Flash_Hal_UnLockCtrl();
            RetVal = Flash_Hal_Erase(FLASH_ERASE_BLOCK, DFLASH_BASE);
            (void)Flash_Hal_LockCtrl();
            (void)System_FlsDeviceUnlock(FLS_DEV);
        }
    }
    else
    {
        RetVal = STATUS_ERROR;
    }

    return RetVal;
}
#endif

/* Implementation of DFlash_Hal_PageWrite declared in Fls_Hal.h */
Hal_StatusType DFlash_Hal_PageWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    FlashDeviceType DevLock;
    DEVICE_ASSERT(DataPtr != NULL_PTR);
    DEVICE_ASSERT((Offset + Length) <= S_DFlashSize);
    DEVICE_ASSERT(0U == (Offset % DFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT(0U == (Length % DFLASH_WRITE_UNIT_SIZE));

    uint32 WriteAddress = DFLASH_BASE + Offset;
    if ((Offset + Length) > S_DFlashSize)
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else
    {
        DevLock = System_FlsDeviceTryLock(FLS_DEV);
        if (DevLock != FLS_DEV)
        {
            RetVal = STATUS_BUSY;
        }
        else
        {
            (void)Flash_Hal_UnLockCtrl();
            RetVal = Flash_Hal_Write(FLASH_PROGRAM_PHRASE, WriteAddress, Length, DataPtr);
            (void)Flash_Hal_LockCtrl();
            (void)System_FlsDeviceUnlock(FLS_DEV);
        }
    }
    return RetVal;
}

#ifndef FLS_SDK_NON_EXTENDED_API
/* Implementation of DFlash_Hal_SectionWrite declared in Fls_Hal.h */
#if defined (AC7843X) || defined (AC7840E)
Hal_StatusType DFlash_Hal_SectionWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    (void)Offset;
    (void)Length;
    (void)DataPtr;
    return STATUS_UNSUPPORTED;
}
#else
Hal_StatusType DFlash_Hal_SectionWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    FlashDeviceType DevLock;
    uint32 Len = Length;
    uint32 Addr = FLEXRAM_BASE;
    const uint8 *DPtr = DataPtr;
    DEVICE_ASSERT(((Offset % DFLASH_PAGE_SIZE) + Length) <= DFLASH_PAGE_SIZE);
    DEVICE_ASSERT(0U == (Offset % DFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT(0U == (Length % DFLASH_WRITE_UNIT_SIZE));
    DEVICE_ASSERT(DataPtr != NULL_PTR);

    uint32 WriteAddress = DFLASH_BASE + Offset;
    if (((Offset + Length) > S_DFlashSize) || (0U != (Length % DFLASH_WRITE_UNIT_SIZE)))
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else
    {
        DevLock = System_FlsDeviceTryLock(FLS_DEV);
        if (DevLock != FLS_DEV)
        {
            RetVal = STATUS_BUSY;
        }
        else
        {
            (void)Flash_Hal_UnLockCtrl();
            /*set flex ram*/
            Flash_Reg_SetCommandReg(FLASH_SET_FLEXRAM);
            Flash_Reg_SetAddressReg(0xFFU);
            RetVal = Flash_Hal_StartCmd(0U);
            /*set data to flex ram*/
            while (Len > 0U)
            {
                *(uint8 *)Addr = *DPtr;
                DPtr++;
                Addr++;
                Len--;
            }

            if (STATUS_SUCCESS == RetVal)
            {
                Flash_Reg_SetLengthReg(Length / DFLASH_WRITE_UNIT_SIZE);
                RetVal = Flash_Hal_Write(FLASH_PROGRAM_SECTION, WriteAddress,
                                     DFLASH_WRITE_UNIT_SIZE, (uint8 *)FLEXRAM_BASE);
            }

            (void)Flash_Hal_LockCtrl();
            (void)System_FlsDeviceUnlock(FLS_DEV);
        }
    }
    return RetVal;
}
#endif
#endif

/* Implementation of DFlash_Hal_SectionVerify declared in Fls_Hal.h */
Hal_StatusType DFlash_Hal_SectionVerify
(
    uint32 Offset,
    uint32 Length
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    FlashDeviceType DevLock;
    if ((Offset + Length) > S_DFlashSize)
    {
        RetVal = STATUS_UNSUPPORTED;
    }
    else
    {
        uint32 WriteAddress = DFLASH_BASE + Offset;

        DevLock = System_FlsDeviceTryLock(FLS_DEV);
        if (DevLock != FLS_DEV)
        {
            RetVal = STATUS_BUSY;
        }
        else
        {
            (void)Flash_Hal_UnLockCtrl();
            RetVal = Flash_Hal_Verify(FLASH_VERIFY_SECTION, WriteAddress, Length);
            (void)Flash_Hal_LockCtrl();
#if defined (AC7840X)
            if (STATUS_ERROR == RetVal)
            {
                /* software to verify */
                RetVal = Flash_Hal_SW_Verify(WriteAddress, Length);
            }
#endif
            (void)System_FlsDeviceUnlock(FLS_DEV);
        }
    }

    return RetVal;
}
#ifndef FLS_SDK_NON_EXTENDED_API

/* Implementation of DFlash_Hal_BlockVerify declared in Fls_Hal.h */
Hal_StatusType DFlash_Hal_BlockVerify(void)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    FlashDeviceType DevLock;

    DevLock = System_FlsDeviceTryLock(FLS_DEV);
    if (DevLock != FLS_DEV)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        (void)Flash_Hal_UnLockCtrl();
        RetVal = Flash_Hal_Verify(FLASH_VERIFY_BLOCK, DFLASH_BASE, 8U);
        (void)Flash_Hal_LockCtrl();

#if defined (AC7840X)
        if (STATUS_ERROR == RetVal)
        {
            /* software to verify */
            RetVal = Flash_Hal_SW_Verify(DFLASH_BASE, S_DFlashSize);
        }
#endif
        (void)System_FlsDeviceUnlock(FLS_DEV);
    }

    return RetVal;
}
#endif
/* Implementation of DFlash_Hal_Read declared in Fls_Hal.h */
Hal_StatusType DFlash_Hal_Read
(
    uint32 Offset,
    uint32 Length,
    uint8 *DataPtr
)
{
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint32 ReadAddress;
    FlashDeviceType DevLock;
    uint32 DataLength = Length;
    uint8 *TmpDataPtr = DataPtr;
    DEVICE_ASSERT((Offset + Length) <= S_DFlashSize);
    DEVICE_ASSERT(DataPtr != NULL_PTR);

    ReadAddress = DFLASH_BASE + Offset;
    DevLock = System_FlsDeviceTryLock(FLS_DEV);
    if (DevLock != FLS_DEV)
    {
        RetVal = STATUS_BUSY;
    }
    else
    {
        /* End of read pointer */
        while (0x0U < DataLength)
        {
            /* read flash location */
            *TmpDataPtr = *(volatile uint8 *)ReadAddress;
            TmpDataPtr++;
            ReadAddress++;
            DataLength -= 0x1U;
        }
        (void)System_FlsDeviceUnlock(FLS_DEV);
    }

    return RetVal;
}
/*PRQA S 1503 -- # AMCE_FLS_1503.*/
#endif
