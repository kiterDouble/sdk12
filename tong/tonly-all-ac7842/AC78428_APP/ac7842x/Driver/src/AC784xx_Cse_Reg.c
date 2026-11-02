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
 * @file AC784xx_Cse_Reg.c
 *
 * @brief This file provides cse hardware access functions.
 *
 */
/* ===========================================  INCLUDE FILES  =========================================== */
#if defined(AC7840X) || defined(AC7842X)
#include "AC784xx_Cse_Reg.h"
#if defined(AC7840X)
#include "AC7840x.h"
#elif defined(AC7842X)
#include "AC7842x.h"
#endif

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* =====================================  Functions definition  ===================================== */
/*!
 * @brief Waits for the completion of a CSE command.
 * @note  Function ID:  DES_CSE_API_311
 * @param [in] Timeout: timeout value
 * return none
 */
Hal_StatusType CSE_Reg_WaitCommandCompletion(uint32 Timeout)
{
#if OSIF_TIMEOUT_ENABLE
    Hal_StatusType Stat;

    uint32 TimeOutTicks;
    uint32 TimeOutCurrentTicks;
    uint32 TimeOutElapsedTicks = 0UL;
    TimeOutTicks = OsIf_MicrosToTicks(Timeout);
    TimeOutCurrentTicks = OsIf_GetCounter();
    while (((FLASH->CSESTAT & FLASH_CSESTAT_BSY_Msk) == 1UL) &&
            ((TimeOutElapsedTicks < TimeOutTicks) || (TimeOutElapsedTicks == TimeOutTicks)))
    {
        TimeOutElapsedTicks = OsIf_GetElapsed(&TimeOutCurrentTicks);
    }
    if (TimeOutElapsedTicks > TimeOutTicks)
    {
        Stat = STATUS_TIMEOUT;
    }
    else
    {
        Stat = STATUS_SUCCESS;
    }

    return Stat;
#else
    /*cstat !MISRAC2012-Rule-11.4*/
    while ((FLASH->CSESTAT & FLASH_CSESTAT_BSY_Msk) != 0UL)
    {
        /* Wait until the CCIF flag is set */
    }
    (void)Timeout;

    return STATUS_SUCCESS;

#endif
}

/*!
 * @brief Reads the error bits from PRAM.
 * @note  Function ID:  DES_CSE_API_312
 * @return Error Code after command execution
 */
Hal_StatusType CSE_Reg_ReadErrorBits(void)
{
    uint16 ErrBits = CSE_Reg_ReadCommandHalfWord(FEATURE_CSE_ERROR_BITS_OFFSET);
    Hal_StatusType Stat;

    switch (ErrBits)
    {
    case CSE_DEFAULT_STATUS:
        if (((FLASH->PART & FLASH_PART_CSEKEYSIZE_Msk) >> FLASH_PART_CSEKEYSIZE_Pos) != 0U)
        {
            Stat = STATUS_SUCCESS;
        }
        else
        {
            Stat = STATUS_ERROR;
        }
        break;
    case CSE_NO_ERROR:
    case CSE_NO_ERROR_NO_SECURE_BOOT:
        Stat = STATUS_SUCCESS;
        break;

    case CSE_SEQUENCE_ERROR:
        Stat = STATUS_SEC_SEQUENCE_ERROR;
        break;

    case CSE_KEY_NOT_AVAILABLE:
        Stat = STATUS_SEC_KEY_NOT_AVAILABLE;
        break;

    case CSE_KEY_INVALID:
        Stat = STATUS_SEC_KEY_INVALID;
        break;

    case CSE_KEY_EMPTY:
        Stat = STATUS_SEC_KEY_EMPTY;
        break;

    case CSE_NO_SECURE_BOOT:
        Stat = STATUS_SEC_NO_SECURE_BOOT;
        break;

    case CSE_KEY_WRITE_PROTECTED:
        Stat = STATUS_SEC_KEY_WRITE_PROTECTED;
        break;

    case CSE_KEY_UPDATE_ERROR:
        Stat = STATUS_SEC_KEY_UPDATE_ERROR;
        break;

    case CSE_RNG_SEED:
        Stat = STATUS_SEC_RNG_SEED;
        break;

    case CSE_NO_DEBUGGING:
        Stat = STATUS_SEC_NO_DEBUGGING;
        break;

    case CSE_MEMORY_FAILURE:
        Stat = STATUS_SEC_MEMORY_FAILURE;
        break;

    case CSE_GENERAL_ERROR:
    default:
        Stat = STATUS_ERROR;
        break;
    }

    return Stat;
}

/*!
 * @brief Writes command in bytes to PRAM.
 * @note  Function ID:  DES_CSE_API_301
 * @param [in] Offset: The offset in bytes at which the bytes shall be written
 * @param [in] Bytes: The buffer containing the bytes to be written
 * @param [in] NumBytes: The number in bytes to be written
 * @return none
 */
void CSE_Reg_WriteCommandBytes(uint8 Offset, const uint8 *Bytes, uint8 NumBytes)
{
#if defined (AC7840X)
    uint8 BlockSize = CSE_PAGE_SIZE_IN_BYTES;
    uint8 Blocks = NumBytes / BlockSize;
    uint8 BlockIndex;
    uint8 Start = 0U;
    uint8 End = 0U;
    uint8 RegOff = 0U;
    uint8 Mod = NumBytes % BlockSize;
    uint8 UnAlign = 0U;

    /*will reverse bytes sequence*/
    for (BlockIndex = 0u; BlockIndex < Blocks; BlockIndex++)
    {
        Start = BlockIndex * BlockSize;
        End = ((BlockIndex + 1u) * BlockSize) - 1u;
        while (TRUE)
        {
            /*PRQA S 2985 ++ #the upper layer call guarantees that a null pointer will never appear.*/
            CSE_PRAM->RAMn[(Offset + RegOff) >> 2U].DATA_32 = CSE_PRAM_RAMn_DATA_32_BYTE_0(Bytes[End - 3u]) | \
                    CSE_PRAM_RAMn_DATA_32_BYTE_1(Bytes[End - 2u]) | \
                    CSE_PRAM_RAMn_DATA_32_BYTE_2(Bytes[End - 1u]) | \
                    CSE_PRAM_RAMn_DATA_32_BYTE_3(Bytes[End - 0u]);
            /*PRQA S 2985 -- #the upper layer call guarantees that a null pointer will never appear.*/
            RegOff = RegOff + 4u;
            if (End > (Start + 4u))
            {
                End = (End - 4U);
            }
            else
            {
                break;
            }
        }
    }

    if (Mod > 0u)
    {
        Start = NumBytes - Mod;

        End = NumBytes - 1u;
        while ((End - Start) >= 4U)
        {
            /*PRQA S 2985 ++ #the upper layer call guarantees that a null pointer will never appear.*/
            CSE_PRAM->RAMn[(Offset + RegOff) >> 2U].DATA_32 = CSE_PRAM_RAMn_DATA_32_BYTE_0(Bytes[End - 3u]) | \
                    CSE_PRAM_RAMn_DATA_32_BYTE_1(Bytes[End - 2u]) | \
                    CSE_PRAM_RAMn_DATA_32_BYTE_2(Bytes[End - 1u]) | \
                    CSE_PRAM_RAMn_DATA_32_BYTE_3(Bytes[End - 0u]);
            /*PRQA S 2985 -- #the upper layer call guarantees that a null pointer will never appear.*/
            RegOff = RegOff + 4U;
            End = End - 4u;
        }

        UnAlign = 3U - (End - Start);
        while (Start <= End)
        {
            CSE_Reg_WriteCommandByte(Offset + RegOff + UnAlign, Bytes[Start]);
            RegOff = RegOff + 1u;

            Start = Start + 1u;
        }
    }
#elif defined (AC7842X)
    uint8 i = 0U;
    uint8 LeftNum = NumBytes % CSE_PAGE_SIZE_IN_BYTES;
    uint8 BlockSize = NumBytes - LeftNum;
    uint8 Unalign_Offset = 16U - LeftNum;

    while ((i + 3U) < BlockSize)
    {
        /*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        /*PRQA S 2985 ++ #the upper layer call guarantees that a null pointer will never appear.*/
        CSE_PRAM->RAMn[(Offset + i) >> 2U].DATA_32 = CSE_PRAM_RAMn_DATA_32_BYTE_0(Bytes[i + 3U]) | \
                CSE_PRAM_RAMn_DATA_32_BYTE_1(Bytes[i + 2U]) | \
                CSE_PRAM_RAMn_DATA_32_BYTE_2(Bytes[i + 1U]) | \
                CSE_PRAM_RAMn_DATA_32_BYTE_3(Bytes[i]);
         /*PRQA S 2985 -- #the upper layer call guarantees that a null pointer will never appear.*/
        i = (uint8)(i + 4U);
    }

    while (i < (BlockSize + LeftNum))
    {
        CSE_Reg_WriteCommandByte(Offset + i + Unalign_Offset, Bytes[i]);
        i++;
    }
#endif
}

/*!
 * @brief Writes a command half word to PRAM.
 * @note  Function ID:  DES_CSE_API_302
 * @param [in] Offset: The offset in half word at which the bytes shall be written
 * @param [in] HalfWord: The buffer containing the half word to be written
 * @return none
 */
void CSE_Reg_WriteCommandHalfWord(uint8 Offset, uint16 HalfWord)
{
    uint32 Tmp;

    Tmp = CSE_PRAM->RAMn[(Offset >> 2U)].DATA_32;
    /*PRQA S 2985 ++ #the upper layer call guarantees that a null pointer will never appear.*/
    if ((Offset & 2U) != 0U)
    {
#if defined (AC7840X)
        Tmp = Tmp & ~CSE_LOWER_HALF_MASK;
        Tmp = Tmp | ((((uint32) HalfWord) << CSE_LOWER_HALF_SHIFT) & CSE_LOWER_HALF_MASK);
#elif defined (AC7842X)
        Tmp = Tmp & ~CSE_UPPER_HALF_MASK;
        Tmp = Tmp | ((((uint32) HalfWord) << CSE_UPPER_HALF_SHIFT) & CSE_UPPER_HALF_MASK);
#endif
    }
    else
    {
#if defined (AC7840X)
        Tmp = Tmp & ~CSE_UPPER_HALF_MASK;
        Tmp = Tmp | ((((uint32) HalfWord) << CSE_UPPER_HALF_SHIFT) & CSE_UPPER_HALF_MASK);
#elif defined (AC7842X)
        Tmp = Tmp & ~CSE_LOWER_HALF_MASK;
        Tmp = Tmp | ((((uint32) HalfWord) << CSE_LOWER_HALF_SHIFT) & CSE_LOWER_HALF_MASK);
#endif
    }
    /*PRQA S 2985 -- #the upper layer call guarantees that a null pointer will never appear.*/
    CSE_PRAM->RAMn[(Offset >> 2U)].DATA_32 = Tmp;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
}

/*PRQA S 1505 ++ #ensure no problem.*/
/*!
 * @brief Writes a command byte to PRAM.
 * @note  Function ID:  DES_CSE_API_303
 * @param [in] Offset: The offset in bytes at which the bytes shall be written
 * @param [in] Byte: The buffer containing a byte to be written
 * @return none
 */
void CSE_Reg_WriteCommandByte(uint8 Offset, uint8 Byte)
{
    switch (Offset & 0x3U)
    {
#if defined (AC7840X)
    case 0x0U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8HU =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x1U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8HL =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x2U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8LU =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x3U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8LL =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;
#elif defined (AC7842X)
    case 0x3U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8HU =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x2U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8HL =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x1U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8LU =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x0U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8LL =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;
#endif
    default:
        /* Impossible to get here */
        break;

    }
}
/*PRQA S 1505 -- #ensure no problem.*/

#if defined (AC7842X)
/*!
 * @brief Writes a command byte to PRAM for Secureboot Define.
 * @note  Function ID:  DES_CSE_API_303
 * @param [in] Offset: The offset in bytes at which the bytes shall be written
 * @param [in] Byte: The buffer containing a byte to be written
 * @return none
 */
void CSE_Reg_WriteBootDefinedByte(uint8 Offset, uint8 Byte)
{
    switch (Offset & 0x3U)
    {
    case 0x0U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8HU =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x1U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8HL =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x2U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8LU =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x3U:
        CSE_PRAM->RAMn[Offset >> 2U].ACCESS8BIT.DATA_8LL =
            Byte;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;
    default:
        /* Impossible to get here */
        break;
    }
}
#endif

/*!
 * @brief Writes command in words to PRAM.
 * @note  Function ID:  DES_CSE_API_304
 * @param [in] Offset: The offset in bytes at which the bytes shall be written
 * @param [in] Words: The buffer containing the words to be written
 * @param [in] NumWords: The number of words to be written
 * @return none
 */
void CSE_Reg_WriteCommandWords(uint8 Offset, const uint32 *Words, uint8 NumWords)
{
    uint8 i = 0U;
    uint8 AlignedOffset = (uint8)(Offset >> 2U);

    while (i < NumWords)
    {
        CSE_PRAM->RAMn[AlignedOffset + i].DATA_32 = Words[i];/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        i++;
    }
}

/*!
 * @brief Read command in bytes from PRAM.
 * @note  Function ID:  DES_CSE_API_305
 * @param [in] Offset: The offset in bytes at which the bytes shall be read
 * @param [out] Bytes: The buffer containing the bytes read from PRAM
 * @param [in] NumBytes: The number in bytes to be read
 * @return none
 */
void CSE_Reg_ReadCommandBytes(uint8 Offset, uint8 *Bytes, uint8 NumBytes)
{
    uint32 Tmp;
#if defined (AC7840X)
    /*will reverse bytes sequence max 128 bytes to*/
    uint8 BlockSize = CSE_PAGE_SIZE_IN_BYTES;
    uint8 Blocks = NumBytes / BlockSize;
    uint8 BlockIndex;
    uint8 Start;
    uint8 End;
    uint8 RegOff = 0u;
    /*will reverse bytes sequence*/
    for (BlockIndex = 0u; BlockIndex < Blocks; BlockIndex++)
    {
        Start = BlockIndex * BlockSize;
        End = ((BlockIndex + 1u) * BlockSize) - 1u;
        while (TRUE)
        {
            Tmp = CSE_PRAM->RAMn[(Offset + RegOff) >>
                                 2U].DATA_32;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
            Bytes[End - 3U] = (uint8)((Tmp & CSE_PRAM_RAMn_DATA_32_BYTE_0_MASK) >> CSE_PRAM_RAMn_DATA_32_BYTE_0_SHIFT);
            Bytes[End - 2U] = (uint8)((Tmp & CSE_PRAM_RAMn_DATA_32_BYTE_1_MASK) >> CSE_PRAM_RAMn_DATA_32_BYTE_1_SHIFT);
            Bytes[End - 1U] = (uint8)((Tmp & CSE_PRAM_RAMn_DATA_32_BYTE_2_MASK) >> CSE_PRAM_RAMn_DATA_32_BYTE_2_SHIFT);
            /*PRQA S 2985 ++ #the upper layer call guarantees that a null pointer will never appear.*/
            Bytes[End] = (uint8)((Tmp & CSE_PRAM_RAMn_DATA_32_BYTE_3_MASK) >> CSE_PRAM_RAMn_DATA_32_BYTE_3_SHIFT);
            /*PRQA S 2985 -- #the upper layer call guarantees that a null pointer will never appear.*/
            RegOff = RegOff + 4U;

            if (End > (Start + 4u))
            {
                End = End - 4U;
            }
            else
            {
                break;
            }
        }
    }
#elif defined (AC7842X)
    uint8 i = 0U;

    while ((i + 3U) < NumBytes)
    {
        Tmp = CSE_PRAM->RAMn[(Offset + i) >> 2U].DATA_32;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        Bytes[i + 3U] = (uint8)((Tmp & CSE_PRAM_RAMn_DATA_32_BYTE_0_MASK) >> CSE_PRAM_RAMn_DATA_32_BYTE_0_SHIFT);
        Bytes[i + 2U] = (uint8)((Tmp & CSE_PRAM_RAMn_DATA_32_BYTE_1_MASK) >> CSE_PRAM_RAMn_DATA_32_BYTE_1_SHIFT);
        Bytes[i + 1U] = (uint8)((Tmp & CSE_PRAM_RAMn_DATA_32_BYTE_2_MASK) >> CSE_PRAM_RAMn_DATA_32_BYTE_2_SHIFT);
        /*PRQA S 2985 ++ #the upper layer call guarantees that a null pointer will never appear.*/
        Bytes[i] = (uint8)((Tmp & CSE_PRAM_RAMn_DATA_32_BYTE_3_MASK) >> CSE_PRAM_RAMn_DATA_32_BYTE_3_SHIFT);
        /*PRQA S 2985 ++ #the upper layer call guarantees that a null pointer will never appear.*/
        i = (uint8)(i + 4U);
    }
    while (i < NumBytes)
    {
        Bytes[i] = CSE_Reg_ReadCommandByte(Offset + i);
        i++;
    }
#endif
}

/*!
 * @brief Read a command half word from CSE_PRAM at a 16-bit aligned offset.
 * @note  Function ID:  DES_CSE_API_306
 * @param [in] Offset: CSE PRAM offset
 * @return none
 */
uint16 CSE_Reg_ReadCommandHalfWord(uint8 Offset)
{
    uint16 HalfWord;

    if ((Offset & 2U) != 0U)
    {
        HalfWord = (uint16)((CSE_PRAM->RAMn[(Offset >> 2U)].DATA_32 & CSE_UPPER_HALF_MASK) >>
                            CSE_UPPER_HALF_SHIFT);/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
    }
    else
    {
        HalfWord = (uint16)((CSE_PRAM->RAMn[(Offset >> 2U)].DATA_32 & CSE_LOWER_HALF_MASK) >>
                            CSE_LOWER_HALF_SHIFT);/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
    }

    return HalfWord;
}

/*!
 * @brief Read command in bytes from PRAM.
 * @note  Function ID:  DES_CSE_API_307
 * @param [in] Offset: The offset in bytes at which the bytes shall be read
 * @return Value of command byte
 */
uint8 CSE_Reg_ReadCommandByte(uint8 Offset)
{
    uint8 Byte = 0U;

    switch (Offset & 0x3U)
    {
#if defined(AC7840X)
    case 0x0U:
        Byte = CSE_PRAM->RAMn[Offset >>
                              2U].ACCESS8BIT.DATA_8HU;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x1U:
        Byte = CSE_PRAM->RAMn[Offset >>
                              2U].ACCESS8BIT.DATA_8HL;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x2U:
        Byte = CSE_PRAM->RAMn[Offset >>
                              2U].ACCESS8BIT.DATA_8LU;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x3U:
        Byte = CSE_PRAM->RAMn[Offset >>
                              2U].ACCESS8BIT.DATA_8LL;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    default:
        /* Impossible to get here */
        break;
#elif defined(AC7842X)
    case 0x3U:
        Byte = CSE_PRAM->RAMn[Offset >>
                              2U].ACCESS8BIT.DATA_8HU;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x2U:
        Byte = CSE_PRAM->RAMn[Offset >>
                              2U].ACCESS8BIT.DATA_8HL;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x1U:
        Byte = CSE_PRAM->RAMn[Offset >>
                              2U].ACCESS8BIT.DATA_8LU;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    case 0x0U:
        Byte = CSE_PRAM->RAMn[Offset >>
                              2U].ACCESS8BIT.DATA_8LL;/*cstat !MISRAC2012-Rule-11.4 Conver to hardware register address*/
        break;

    default:
        /* Impossible to get here */
        break;
#endif
    }

    return Byte;
}

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
                                       uint8 CallSeq, uint8 KeyId)
{
    Hal_StatusType Stat;
    /*PRQA S 2985 ++ #the upper layer call guarantees that a null pointer will never appear.*/
#if defined (AC7840X)
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
    /*PRQA S 2985 -- #the upper layer call guarantees that a null pointer will never appear.*/
    Stat = (Hal_StatusType)CSE_Reg_WaitCommandCompletion((uint32)CSE_COMMAND_BASE_TIME_US);

    return Stat;
}
#endif /*AC7840X or AC7842X*/
/* =============================================  EOF  ============================================== */
