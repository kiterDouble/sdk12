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

/*!
 * @file AC784xx_Cse_Reg.h
 *
 * @brief This file provides CSE hardware integration functions.
 *
 */

#ifndef AC784XX_CSE_REG_H
#define AC784XX_CSE_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#if defined(AC7840X) || defined(AC7842X)
/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Types.h"
#include "AC784xx_Fls_Reg.h"

/* ============================================  DEFINES AND MACROS  ============================================ */
#define OSIF_TIMEOUT_ENABLE 0
/*!
 * @brief Error code as SHE specification.
 */
#define CSE_DEFAULT_STATUS                              (0x0000)
#define CSE_NO_ERROR                                    (0x0001U)
#define CSE_NO_ERROR_NO_SECURE_BOOT                     (0x0021U)
#define CSE_SEQUENCE_ERROR                              (0x0002U)
#define CSE_KEY_NOT_AVAILABLE                           (0x0004U)
#define CSE_KEY_INVALID                                 (0x0008U)
#define CSE_KEY_EMPTY                                   (0x0010U)
#define CSE_NO_SECURE_BOOT                              (0x0020U)
#define CSE_KEY_WRITE_PROTECTED                         (0x0040U)
#define CSE_KEY_UPDATE_ERROR                            (0x0080U)
#define CSE_RNG_SEED                                    (0x0100U)
#define CSE_NO_DEBUGGING                                (0x0200U)
#define CSE_MEMORY_FAILURE                              (0x0400U)
#define CSE_GENERAL_ERROR                               (0x1000U)

/*!
 * @brief CSE_PRAM offset of the page length parameter.
 */
#if defined(AC7840X)
#define FEATURE_CSE_PAGE_LENGTH_OFFSET                  (0x0EU)
#define FEATURE_CSE_MESSAGE_LENGTH_OFFSET               (0x0CU)
#define FEATURE_CSE_MAC_LENGTH_OFFSET                   (0x08U)
#define FEATURE_CSE_BOOT_SIZE_OFFSET                    (0x1CU)
#define FEATURE_CSE_BOOT_FLAVOR_OFFSET                  (0x1BU)
#define FEATURE_CSE_FLASH_START_ADDRESS_OFFSET          (0x10U)
#define FEATURE_CSE_VERIFICATION_STATUS_OFFSET          (0x14U)
#define FEATURE_CSE_ERROR_BITS_OFFSET                   (0x04U)
#define FEATURE_CSE_SREG_OFFSET                         (0x2FU)
#elif defined(AC7842X)
#define FEATURE_CSE_PAGE_LENGTH_OFFSET                  (0x0EU)
#define FEATURE_CSE_MESSAGE_LENGTH_OFFSET               (0x0CU)
#define FEATURE_CSE_MAC_LENGTH_OFFSET                   (0x0BU)
#define FEATURE_CSE_BOOT_SIZE_OFFSET                    (0x1CU)
#define FEATURE_CSE_BOOT_FLAVOR_OFFSET                  (0x1BU)
#define FEATURE_CSE_FLASH_START_ADDRESS_OFFSET          (0x10U)
#define FEATURE_CSE_VERIFICATION_STATUS_OFFSET          (0x16U)
#define FEATURE_CSE_ERROR_BITS_OFFSET                   (0x06U)
#define FEATURE_CSE_SREG_OFFSET                         (0x2FU)
#endif

/*!
 * @brief CSE_PRAM offset of page.
 */
#define FEATURE_CSE_PAGE_1_OFFSET                       (0x10U)
#define FEATURE_CSE_PAGE_2_OFFSET                       (0x20U)
#define FEATURE_CSE_PAGE_3_OFFSET                       (0x30U)
#define FEATURE_CSE_PAGE_4_OFFSET                       (0x40U)
#define FEATURE_CSE_PAGE_5_OFFSET                       (0x50U)
#define FEATURE_CSE_PAGE_7_OFFSET                       (0x70U)

#define CSE_PAGE_SIZE_IN_BYTES                          (16U)
#define CSE_DATA_PAGES_AVAILABLE                        (7U)
#define CSE_DATA_BYTES_AVAILABLE                        (112U)
#define CSE_BYTES_TO_FROM_PAGES_SHIFT                   (4U)
#define CSE_BYTES_TO_FROM_BITS_SHIFT                    (3U)
#define CSE_M1_SIZE_IN_BYTES                            (16U)
#define CSE_M2_SIZE_IN_BYTES                            (32U)
#define CSE_M3_SIZE_IN_BYTES                            (16U)
#define CSE_M4_SIZE_IN_BYTES                            (32U)
#define CSE_M5_SIZE_IN_BYTES                            (16U)

#define CSE_UPPER_HALF_MASK                             (0xFFFF0000UL)
#define CSE_UPPER_HALF_SHIFT                            (0x10UL)
#define CSE_LOWER_HALF_MASK                             (0xFFFFUL)
#define CSE_LOWER_HALF_SHIFT                            (0UL)

#define CSE_COMMAND_BASE_TIME_US    0xFFFFUL
#define CSE_SYNC_TIME_ZERO          1UL
#define CSE_PFLASH_SIZE                                  *((volatile uint32 *)(0x0102383c))
#define CSE_PFLASH_SIZE_1M                               (0x0eUL)
#define CSE_PFLASH_SIZE_1_5M                             (0x0dUL)

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/*!
 * @brief Writes command in bytes to PRAM.
 * @note  Function ID:  DES_CSE_API_301
 * @param [in] Offset: The offset in bytes at which the bytes shall be written
 * @param [in] Bytes: The buffer containing the bytes to be written
 * @param [in] NumBytes: The number in bytes to be written
 * @return none
 */
void CSE_Reg_WriteCommandBytes(uint8 Offset, const uint8 *Bytes, uint8 NumBytes);

/*!
 * @brief Writes a command half word to PRAM.
 * @note  Function ID:  DES_CSE_API_302
 * @param [in] Offset: The offset in half word at which the bytes shall be written
 * @param [in] HalfWord: The buffer containing the half word to be written
 * @return none
 */
void CSE_Reg_WriteCommandHalfWord(uint8 Offset, uint16 HalfWord);

/*!
 * @brief Writes a command byte to PRAM.
 * @note  Function ID:  DES_CSE_API_303
 * @param [in] Offset: The offset in bytes at which the bytes shall be written
 * @param [in] Byte: The buffer containing a byte to be written
 * @return none
 */
void CSE_Reg_WriteCommandByte(uint8 Offset, uint8 Byte);

#if defined(AC7842X)
/*!
 * @brief Writes a command byte to PRAM for Secureboot Define.
 * @note  Function ID:  DES_CSE_API_303
 * @param [in] Offset: The offset in bytes at which the bytes shall be written
 * @param [in] Byte: The buffer containing a byte to be written
 * @return none
 */
void CSE_Reg_WriteBootDefinedByte(uint8 Offset, uint8 Byte);
#endif

/*!
 * @brief Writes command in words to PRAM.
 * @note  Function ID:  DES_CSE_API_304
 * @param [in] Offset: The offset in bytes at which the bytes shall be written
 * @param [in] Words: The buffer containing the words to be written
 * @param [in] NumWords: The number of words to be written
 * @return none
 */
void CSE_Reg_WriteCommandWords(uint8 Offset, const uint32 *Words, uint8 NumWords);

/*!
 * @brief Read command in bytes from PRAM.
 * @note  Function ID:  DES_CSE_API_305
 * @param [in] Offset: The offset in bytes at which the bytes shall be read
 * @param [out] Bytes: The buffer containing the bytes read from PRAM
 * @param [in] NumBytes: The number in bytes to be read
 * @return none
 */
void CSE_Reg_ReadCommandBytes(uint8 Offset, uint8 *Bytes, uint8 NumBytes);

/*!
 * @brief Read a command half word from CSE_PRAM at a 16-bit aligned offset.
 * @note  Function ID:  DES_CSE_API_306
 * @param [in] Offset: CSE PRAM offset
 * @return none
 */
uint16 CSE_Reg_ReadCommandHalfWord(uint8 Offset);

/*!
 * @brief Read command in bytes from PRAM.
 * @note  Function ID:  DES_CSE_API_307
 * @param [in] Offset: The offset in bytes at which the bytes shall be read
 * @return Value of command byte
 */
uint8 CSE_Reg_ReadCommandByte(uint8 Offset);

/*!
 * @brief Writes the command header to PRAM and waits for excute completion.
 * @note  Function ID:  DES_CSE_API_309
 * @param [in] FuncId: funciton ID
 * @param [in] FuncFormat: The offset in bytes at which the bytes shall be read
 * @param [in] CallSeq: The offset in bytes at which the bytes shall be read
 * @param [in] KeyId: The offset in bytes at which the bytes shall be read
 * @return none
 */
Hal_StatusType CSE_Reg_WriteCmdAndWait(uint8 FuncId, uint8 FuncFormat,
                                       uint8 CallSeq, uint8 KeyId);

/*!
 * @brief Waits for the completion of a CSE command.
 * @note  Function ID:  DES_CSE_API_311
 * @param [in] Timeout: timeout value
 * return none
 */
Hal_StatusType CSE_Reg_WaitCommandCompletion(uint32 Timeout);

/*!
 * @brief Reads the error bits from PRAM.
 * @note  Function ID:  DES_CSE_API_312
 * @return Error Code after command execution
 */
Hal_StatusType CSE_Reg_ReadErrorBits(void);

/* =====================================  Functions definition  ===================================== */
/*!
 * @brief Enables/Disables the command completion interrupt.
 *
 * @param [in] Enable: Enable/Disable the command completion interrupt
 * @return none
 */
static inline void CSE_Reg_SetInterrupt(boolean Enable)
{
    /*add flash unlock ctrl*/
    Flash_Reg_UnLock();
    /*enable cse interrupts */
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_CSECCIE_Msk, FLASH_CNFG_CSECCIE_Pos, (uint32)Enable);
    /*add flash lock ctrl*/
    Flash_Reg_Lock();
}

/*!
 * @brief Writes the command header to PRAM.
 * @note  Function ID:  DES_CSE_API_310
 * @param [in] FuncId: the ID of the operation to be started
 * @param [in] FuncFormat: specifies how the data is transferred to/from the CSE
 * @param [in] CallSeq: specifies if the information is the first or a following function call
 * @param [in] KeyId: keyID that will be used to perform the requested cryptographic operation
 * @return none
 */
static inline void CSE_Reg_WriteCommandHeader(uint8 FuncId, uint8 FuncFormat,
        uint8 CallSeq, uint8 KeyId)
{
#if defined (AC7840X)
    /*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
    CSE_PRAM->RAMn[0].DATA_32 = CSE_PRAM_RAMn_DATA_32_BYTE_0(KeyId) | \
                                CSE_PRAM_RAMn_DATA_32_BYTE_1(CallSeq) | \
                                CSE_PRAM_RAMn_DATA_32_BYTE_2(FuncFormat) | \
                                CSE_PRAM_RAMn_DATA_32_BYTE_3(FuncId);

#elif defined(AC7842X)
    CSE_PRAM->RAMn[0].DATA_32 = CSE_PRAM_RAMn_DATA_32_BYTE_3(KeyId) | \
                                CSE_PRAM_RAMn_DATA_32_BYTE_2(CallSeq) | \
                                CSE_PRAM_RAMn_DATA_32_BYTE_1(FuncFormat) | \
                                CSE_PRAM_RAMn_DATA_32_BYTE_0(FuncId);
#endif
}

/*!
 * @brief Returns the content of the status register.
 * @return Value of the status register
 */
static inline uint32 CSE_Reg_GetStatus(void)
{
    return (uint32)(FLASH->CSESTAT);/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
}

/*!
 * @brief Return command completed status.
 * @return 1: Command is completed
 *         0: Command is not completed
 */
static inline uint8 CSE_Reg_GetCmdCompleteStatus(void)
{
    return (uint8)((FLASH->CSESTAT & FLASH_CSESTAT_CCIF_Msk) >> FLASH_CSESTAT_CCIF_Pos);
}

/*!
 * @brief Clear command completed interrupt flag.
 * @return none
 */
static inline void CSE_Reg_ClearCmdCompleteStatus(void)
{
    Flash_Reg_UnLock();
    MODIFY_REG32(FLASH->CSESTAT, FLASH_CSESTAT_CCIF_Msk, FLASH_CSESTAT_CCIF_Pos, 1U);
    Flash_Reg_Lock();
}

/*!
 * @brief get Pflash Max size.
 * @return none
 */
static inline uint32 CSE_Reg_GetPFlashSize(void)
{
    uint32 PSize = 0u;
    uint8 PType = 0u;

    PType = (uint8)(CSE_PFLASH_SIZE & 0x0fu);

    if (PType == CSE_PFLASH_SIZE_1M)
    {
        PSize = 1048576u;
    }
    else if (PType == CSE_PFLASH_SIZE_1_5M)
    {
        PSize = 1572864u;
    }
    else
    {
        PSize = 2097152u;
    }

    return PSize;
}
#endif
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _AC784XX_CSE_REG_H */

/* =============================================  EOF  ============================================== */
