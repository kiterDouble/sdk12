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
 * @file eep_drv.c
 * @brief This file provides EEP integration functions.
 *
 */

/* ===========================================  Includes  =========================================== */
#include "Eep_Hal.h"
#include "Fls_Hal.h"

#if defined(CONFIG_EEP_DRV_ENABLE) && defined(AC7840X)
/* ============================================  Define  ============================================ */
/* EEP group size define rule:
   1. Total EEP size (Byte) = EEP SRAM size which base address is gEepRamBaseAddr
   2. The min EEP_GROUP_SIZE = EEP_SIZE (word) * 2 * gEepFlashMinProByteNum
   3. EEP_FLASH_SIZE = EEP_GROUP_SIZE * Total group
*/

/* EEP operate group flag */
#define EEP_GROUP_FLAG_VALUE            (0x00000000U)

/* EEP total group number */
#define EEP_GROUPS_NUM                  (4U)

/* EEP FLASH max program unit size */
#define EEP_FLASH_MAX_UNIT_SIZE         (32U)

/** @brief The EEP ram address to logic address. */
#define EEP_LOGIC_ADDR_FROM_FLASH(addr) (addr & 0xfffU)

/* ===========================================  Typedef  ============================================ */

/* ====================================  Functions declaration  ===================================== */

/* ==========================================  Variables  =========================================== */
static uint16 gEepSize = 4096U;
static uint32 gEepRamBaseAddr = 0x14000000U;
static uint32 gEepFlashBaseAddr = 0x1000000U;
static uint32 gEepGroupSize = 16 * 1024U;
uint32 gEepFlashMinProByteNum = 8U;
uint32 gEepFlashDateByteNum = 0U;

volatile uint32 gCurFlashOperatAddr = 0U;
volatile uint16 gCurGroup = 0U;
volatile uint16 gLastGroup = 0U;
uint32 gEepGroupFlag[EEP_FLASH_MAX_UNIT_SIZE / 4U] = {0U};
uint32 gEepGroupAddr[EEP_GROUPS_NUM][2U] = {0U};
eep_ic_type_t gEepICType = EEP_AC78406;
Eep_CallbackType gEepCallback = 0U;

eep_config_t g_EepConfig =
{
    .FlashPageSize = DFLASH_PAGE_SIZE,
    .FlashMinProByte = DFLASH_WRITE_UNIT_SIZE,
    .icType = EEP_AC78406,
};

/** @brief The EEP driver status. */
static Eep_Hal_StatusType Eep_Status = EEP_HAL_UNINIT;
/* ======================================  Functions define  ======================================== */
void EepMemcpy(void *v_dst, const void *v_src, uint32 c)
{
    uint32 len  = c;
    const char *src = (const char *)v_src;
    char *dst = (char *)v_dst;

    /* Simple, byte oriented memcpy. */
    while (0x0U != len--)
    {
        *dst++ = *src++;
    }
}

static void EepMemset(uint8 *s, uint8 c, uint32 count)
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
 * @brief Unlock the FLASH register.
 *
 * @param[in] none
 * @return operation status
 */
eep_status_t EEP_FLASH_UnlockCtrl(void)
{
    return EEP_SUCCESS;
}

/*!
 * @brief Lock the FLASH register.
 *
 * @param[in] none
 * @return none
 */
void EEP_FLASH_LockCtrl(void)
{
}

/*!
 * @brief EEP group verify.
 *
 * @param[in] addr: Start address for verify operation
 * @param[in] size: Verify size (byte)
 * @return operation status
 */
eep_status_t EEP_FLASH_VerifyGroup(uint32 addr, uint32 size)
{
    Hal_StatusType RetVal;

    RetVal = DFlash_Hal_SectionVerify(addr - DFLASH_BASE, size);

    return RetVal == STATUS_SUCCESS ? EEP_SUCCESS : EEP_STATUS_ERROR;
}

/*!
 * @brief EEP erase GROUP.
 *
 * @param[in] addr: Erase sector start address
 * @param[in] size: Erase size in bytes
 * @return operation status
 */

eep_status_t EEP_FLASH_EraseGroup(uint32 addr, uint32 size)
{
    Hal_StatusType RetVal = STATUS_ERROR;
    uint32 flashAddressSart = gEepFlashBaseAddr;
    uint32 flashAddressEnd = gEepFlashBaseAddr + gEepGroupSize * EEP_GROUPS_NUM;

    if ((addr >= flashAddressSart) && ((addr + size) <= flashAddressEnd))
    {
        for (; size > 0; size -= DFLASH_PAGE_SIZE)
        {
            RetVal = DFlash_Hal_PageErase(addr - DFLASH_BASE);
            addr += DFLASH_PAGE_SIZE;
            if (RetVal != STATUS_SUCCESS)
            {
                break;
            }
        }
    }

    return RetVal == STATUS_SUCCESS ? EEP_SUCCESS : EEP_STATUS_ERROR;
}

/*!
 * @brief FLASH program.
 *
 * @param[in] addr: Program start address
 * @param[in] size: Program size in byte
 * @param[in] data: Pointer of data source address from which will be programmed
 * @return operation status
 */
eep_status_t EEP_FLASH_Program(uint32 addr, uint32 size, const uint8 *data)
{
    Hal_StatusType RetVal = STATUS_ERROR;
    uint32 flashAddressSart = gEepFlashBaseAddr;
    uint32 flashAddressEnd = gEepFlashBaseAddr + gEepGroupSize * EEP_GROUPS_NUM;

    if ((addr >= flashAddressSart) && ((addr + size) <= flashAddressEnd))
    {
        RetVal = DFlash_Hal_PageWrite(addr - DFLASH_BASE, size, data);
    }

    return RetVal == STATUS_SUCCESS ? EEP_SUCCESS : EEP_STATUS_ERROR;
}

/*!
 * @brief CSE size determines whether to init eep.
 *
 * @param[in] ram base addr
 * @param[in] flash base addr
 * @return operation status
 */
eep_status_t EEP_FLASH_PreInitCheck(uint32 ramAddr, uint32 flashAddr)
{
    eep_status_t ret = EEP_SUCCESS;

    if ((ramAddr >= 0x14000000U && ramAddr < 0x14001000U) ||
        (flashAddr >= 0x1010000U && flashAddr < 0x1020000U))
    {
        ret = EEP_PARA_ERROR;
    }

    return ret;
}

/*!
 * @brief write data to FLASH, the Flash low 32-bit saved EEP address
 *
 * @param[in] flashAddr: write FLASH start address
 * @param[in] eepAddr: EEP address
 * @param[in] data: data pointer
 * @param[in] size: data byte number
 * @param[in] mode: 0: not write all sram data to FLASH, 1: write all sram data to FLASH
 * @return EEP operate status
 */
static eep_status_t WriteDataToDflash(volatile uint32 *flashAddr, uint32 eepAddr,
                                      const uint8 *data, uint16 size, uint8 mode)
{
    eep_status_t ret = EEP_SUCCESS;
    uint8 newCombineData[EEP_FLASH_MAX_UNIT_SIZE] = {0};
    uint8 byteWidth;
    uint16 i = 0U;
    uint16 tempSize = 0U;
    uint32 logicAddr;

    if (gEepICType == EEP_AC78036)
        byteWidth = 2U;
    else
        byteWidth = 4U;

    while ((size > 0U) && (EEP_SUCCESS == ret))
    {
        /*Filled EEP address to array newCombineData */
        logicAddr = eepAddr - gEepRamBaseAddr;
        for (i = 0U; i < byteWidth; i++)
        {
            newCombineData[i] = (logicAddr >> (i * 8U)) & 0xFFU;
        }
        tempSize = gEepFlashDateByteNum;
        if (size < gEepFlashDateByteNum)
        {
            tempSize = size;
        }
        for (i = 0U; i < tempSize; i++)
        {
            newCombineData[i + byteWidth] = data[i];
        }

        for (i = tempSize; i < gEepFlashDateByteNum; i++)
        {
            if ((eepAddr + i) < (gEepSize + gEepRamBaseAddr))
            {
                newCombineData[i + byteWidth] = *(uint8 *)(eepAddr + i);
            }
            else
            {
                newCombineData[i + byteWidth] = 0xFFU;
            }
        }

        if (1U == mode)
        {
            for (i = 0U; i < gEepFlashDateByteNum; i++)
            {
                if (0xFF != newCombineData[i + byteWidth])
                {
                    break;
                }
            }
        }
        if ((i < gEepFlashDateByteNum) || (0U == mode))
        {
            ret = EEP_FLASH_Program(*flashAddr, gEepFlashMinProByteNum, (uint8 *)newCombineData);
            *flashAddr += gEepFlashMinProByteNum;
        }

        size -= tempSize;
        data += tempSize;
        eepAddr += tempSize;
    }

    return ret;
}

/*!
 * @brief look for the first data which is not 0xFFFFFFFF form end to start address
 *
 * @param[in] endAddr: end address
 * @param[in] validDataAddr: valid data start address
 * @return none
 */
static void GetValidDataStartAddr(uint8 group, volatile uint32 *groupOperateAddr)
{
    uint32 i;
    uint32 tempSize = (gEepGroupSize / gEepFlashMinProByteNum) - 1U;
    uint32 tempAddr, addr = 0U;
    uint32 tempGroupEndAddr;

    tempGroupEndAddr = gEepGroupAddr[group][1U] - gEepFlashMinProByteNum;
    *groupOperateAddr = gEepGroupAddr[group][0U];
    for (i = 0U; i < tempSize; i++)
    {
        tempAddr = tempGroupEndAddr - (i + 1U) * gEepFlashMinProByteNum;
        /* Check address is not 0xFFFFFFFF */
        if (gEepICType == EEP_AC78406)
        {
            if (0xFFFFFFFFU !=  *((volatile uint32 *)(tempAddr)))
            {
                *groupOperateAddr = tempAddr + gEepFlashMinProByteNum;
                break;
            }
        }
        else if (gEepICType == EEP_AC78036)
        {
            addr = (*((volatile uint32 *)(tempAddr)) << 16) >> 16;
            if (0xFFFFU != addr)
            {
                *groupOperateAddr = tempAddr + gEepFlashMinProByteNum;
                break;
            }
        }
    }
}

/*!
 * @brief get the flag value of operate of current group
 *
 * @param[in] group: EEP group index
 * @return the flag value
 */
static uint32 GetEepGroupFlagVal(uint16 group)
{
    return (*(volatile uint32 *)(gEepGroupAddr[group][1U] - gEepFlashMinProByteNum));
}

/*!
 * @brief check the current group index and caculate the count of the group flag
 *
 * @param[out] index: group index
 * @return the count of the group flag
 */
static uint8 CheckOperateGroup(uint8 *index)
{
    uint8 cnt  = 0U;
    uint16 i;

    for (i = 0U; i < EEP_GROUPS_NUM; i++)
    {
        if (EEP_GROUP_FLAG_VALUE == GetEepGroupFlagVal(i))
        {
            cnt++;
            *index = i;
        }
    }

    return cnt;
}

/*!
 * @brief Load flash data to RAM
 *
 * @param[in] group: EEP group index
 * @param[in] operateAddr: EEP group operate address
 * @return EEP operate status
 */
static eep_status_t LoadFlashDataToSram(uint8 group, uint32 operateAddr)
{
    uint32 i, j, addr = 0U;
    uint32 temp = 0U;
    eep_status_t ret = EEP_SUCCESS;

    if (gEepICType == EEP_AC78406)
    {
        for (i = gEepGroupAddr[group][0U]; i < operateAddr; i += gEepFlashMinProByteNum)
        {
            for (j = 0U; j < gEepFlashDateByteNum; j++)
            {
                temp = *(volatile uint32 *)(i);
                temp = EEP_LOGIC_ADDR_FROM_FLASH(temp);
                /* maybe the temp is 0xFFFFFFFF */
                if ((temp < gEepSize) && ((temp % 4) == 0U))
                {
                    *(volatile uint8 *)(gEepRamBaseAddr + temp + j) = *(volatile uint8 *)(i + 4U + j);
                }
            }
        }
    }
    else if (gEepICType == EEP_AC78036)
    {
        for (i = gEepGroupAddr[group][0U]; i < operateAddr; i += 4)
        {
            for (j = 0U; j < 2; j++)
            {
                temp = *(volatile uint32 *)(i);
                temp = (temp << 16) >> 16;
                /* maybe the temp is 0xFFFF */
                if ((temp >= (gEepRamBaseAddr & 0xffff)) &&
                    (temp < (gEepRamBaseAddr + (gEepSize & 0xffff))) && ((temp % 4) == 0U))
                {
                    addr = ((gEepRamBaseAddr >> 16) << 16) + temp;
                    *(volatile uint8 *)(addr + j) = *(volatile uint8 *)(i + 2U + j);
                }
            }
        }
    }

    return ret;
}
/*!
 * @brief Load EEP data to SRAM from EEP group
 *
 * @param[in] curGroupOperateAddr: current group flash operate address
 * @param[in] lastGroupOperateAddr: last group flash operate address
 * @return EEP operate status
 */
static eep_status_t SortEepDataToSram(uint32 curGroupOperateAddr, uint32 lastGroupOperateAddr)
{
    eep_status_t ret = EEP_SUCCESS;
    /*
        1. Flash min width 64, 32-bit address + 32-bit data(AC784x)
        2. Flash min width 32, 32-bit address + 32-bit data(AC70x or AC781x)
        3. Flash min width 128, 32-bit address + 96-bit data(AC787x)
    */
    EepMemset((uint8 *)gEepRamBaseAddr, 0xff, gEepSize);

    ret = LoadFlashDataToSram(gLastGroup, lastGroupOperateAddr);

    if (EEP_SUCCESS == ret)
    {
        ret = LoadFlashDataToSram(gCurGroup, curGroupOperateAddr);
    }

    return ret;
}

/*!
 * @brief update EEP group index and complete data copy
 *
 * @param[in] none
 * @return EEP operate status
 */
static eep_status_t UpdateOperateAddr(void)
{
    eep_status_t ret = EEP_SUCCESS;
    uint8 index = 0U;
    uint8 cnt;
    uint32 lastGroupOperateAddr = 0U;
    uint32 tempAddr = 0U;
    uint32 tmpCurFlashOperatAddr;

    cnt = CheckOperateGroup(&index);
    switch (cnt)
    {
    case 0U:/* First used eep write operate flag in the first group */
        ret = EEP_FLASH_Program(gEepGroupAddr[0U][1U] - gEepFlashMinProByteNum,
                                gEepFlashMinProByteNum, (uint8 *)gEepGroupFlag);
        gCurGroup = 0U;
        gLastGroup = EEP_GROUPS_NUM - 1U;
        break;

    case 1U:
        if (0U == index)
        {
            gCurGroup = 0U;
            gLastGroup = EEP_GROUPS_NUM - 1U;
        }
        else
        {
            gCurGroup = index;
            gLastGroup = index - 1U;
        }
        break;

    case 2U:
        /* indicate copy fail last time from the previous group to the next group
          then it should copy again and erase the previous group, cnt=2 so index never = 1 at least
        */
        if ((EEP_GROUP_FLAG_VALUE == GetEepGroupFlagVal(index - 1U)) &&
                (EEP_GROUP_FLAG_VALUE == GetEepGroupFlagVal(index)))
        {
            gCurGroup = index;
            gLastGroup = index - 1U;

        }/*indicate copy fail last time from the lasst group to the first group
               then it should copy again and erase the last group, index = the last group
            */
        else if
        ((EEP_GROUP_FLAG_VALUE == GetEepGroupFlagVal(0U)) &&
                (EEP_GROUP_FLAG_VALUE  == GetEepGroupFlagVal(index)))
        {
            gCurGroup = 0U;
            gLastGroup = index;
        }
        else
        {
            ret = EEP_OTHER_ERROR;
        }
        break;

    default:
        ret = EEP_OTHER_ERROR;
        break;
    }

    GetValidDataStartAddr(gCurGroup , &gCurFlashOperatAddr);
    GetValidDataStartAddr(gLastGroup , &lastGroupOperateAddr);
    if (EEP_SUCCESS == ret)
    {
        ret = SortEepDataToSram(gCurFlashOperatAddr, lastGroupOperateAddr);
    }

    if ((EEP_SUCCESS == ret) && (2U == cnt))
    {
        for (tmpCurFlashOperatAddr = gCurFlashOperatAddr;
            tmpCurFlashOperatAddr > gEepGroupAddr[gCurGroup][0U];
            tmpCurFlashOperatAddr -=  gEepFlashMinProByteNum)
       {
           if (EEP_AC78406 == gEepICType)
           {
               uint32 logicAddr;
               tempAddr = *(volatile uint32 *)(tmpCurFlashOperatAddr - gEepFlashMinProByteNum) + 4U;
               logicAddr = EEP_LOGIC_ADDR_FROM_FLASH(tempAddr);
               /* Check the temp address is valid or not, maybe the temp is erased && addr 4byte aligned */
               if ((logicAddr < gEepSize) && ((logicAddr % 4) == 0U))
               {
                   /* valid data */
                   tempAddr = gEepRamBaseAddr + logicAddr;
                   break;
               }
           }
           else
           {
               tempAddr =*(volatile uint32 *)(tmpCurFlashOperatAddr - gEepFlashMinProByteNum);
               tempAddr = ((tempAddr << 16) >> 16) + ((gEepRamBaseAddr >> 16) << 16) + 4U;

               /* Check the temp address is valid or not, maybe the temp is erased && addr 4byte aligned */
               if ((tempAddr > (gEepRamBaseAddr & 0xffff)) &&
                   (tempAddr <= (gEepRamBaseAddr + (gEepSize & 0xffff))) &&
                   ((tempAddr % 4) == 0U))
               {
                   /* valid data */
                   break;
               }
           }

       }

       if (gEepGroupAddr[gCurGroup][0U] == tmpCurFlashOperatAddr)
       {
           tempAddr = gEepRamBaseAddr;
       }

        ret = WriteDataToDflash(&gCurFlashOperatAddr, tempAddr, (uint8 *)tempAddr,
                                gEepRamBaseAddr + gEepSize - tempAddr, 1U);
        if (EEP_SUCCESS == ret)
        {
            /* erase last group */
            ret = EEP_FLASH_EraseGroup(gEepGroupAddr[gLastGroup][0U], gEepGroupSize);
        }
    }

    return ret;
}

/*!
 * @brief EEP Write
 *
 * @param[in] addr: EEP address
 * @param[in] data: data buffer pointer
 * @param[in] size: write byte number
 * @return EEP operate status
 */
static eep_status_t EEP_WriteExt(uint32 addr, const uint8 *data, uint16 size)
{
    eep_status_t ret = EEP_SUCCESS;
    uint32 leftSize = 0U;
    uint16 tempSize = 0U;
    uint32 tmpCurFlashOperatAddr;

    tmpCurFlashOperatAddr = gCurFlashOperatAddr;
    leftSize = (gEepGroupAddr[gCurGroup][1U] - gEepFlashMinProByteNum - tmpCurFlashOperatAddr) / gEepFlashMinProByteNum;
    /*
        check write data not enough one FLASH min write unit size - 4(EEP addr)
        According to the code, size is never more than gEepFlashDateByteNum
    */
    tempSize = (uint16)(size / gEepFlashDateByteNum);
    if (tempSize == 0U)
    {
        tempSize = 1U;
    }

    /* check space is enough, every word combine with write address will use 8 FLASH address */
    if (leftSize >= tempSize)
    {
        /* has enough space */
        ret = WriteDataToDflash(&gCurFlashOperatAddr, addr, data, size, 0U);
    }
    else
    {
        if (0U != gEepCallback)
        {
            gEepCallback();
        }
        /* verify whether next eep group is eraseed, normally
           the following erase step will not need to be executed
        */
        ret = EEP_FLASH_VerifyGroup(gEepGroupAddr[(gCurGroup + 1U) % EEP_GROUPS_NUM][0U], gEepGroupSize);
        if (EEP_SUCCESS != ret)
        {
            ret = EEP_FLASH_EraseGroup(gEepGroupAddr[(gCurGroup + 1U) % EEP_GROUPS_NUM][0U], gEepGroupSize);
        }
        if (EEP_SUCCESS != ret)
        {
            return ret;
        }
        /* write group operate flag (64-bit 0) to the next group*/
        ret = EEP_FLASH_Program(gEepGroupAddr[(gCurGroup + 1U) % EEP_GROUPS_NUM][1U] - gEepFlashMinProByteNum,
                                gEepFlashMinProByteNum, (uint8 *)gEepGroupFlag);
        /* write all the SRAM data to next group */
        if (EEP_SUCCESS == ret)
        {
            gCurFlashOperatAddr = gEepGroupAddr[(gCurGroup + 1U) % EEP_GROUPS_NUM][0U];
            /* write data to the next group */
            ret = WriteDataToDflash(&gCurFlashOperatAddr, gEepRamBaseAddr, (uint8 *)gEepRamBaseAddr, gEepSize, 1U);
        }
        /* copy current group data to next group */
        if (EEP_SUCCESS == ret)
        {
            gLastGroup = gCurGroup;
            gCurGroup = (gCurGroup + 1U) % EEP_GROUPS_NUM;
            if (EEP_SUCCESS == ret)
            {
                ret = EEP_FLASH_EraseGroup(gEepGroupAddr[gLastGroup][0U], gEepGroupSize);
            }
        }
    }

    return ret;
}

/*!
 * @brief EEP Write
 *
 * @param[in] addr: EEP address
 * @param[in] data: data buffer pointer
 * @param[in] size: the size of write data(uint is byte)
 * @return EEP operate status
 *  0U: Successfully
 *  1U: FLASH status error, can Check error by register 0x40002000U
 *  2U: Input parameter error
 *  3U: timeout
 *  4U: other error
 */
Hal_StatusType Eep_Hal_Write(uint32 Addr, const uint8 *Data, uint16 Size)
{
    eep_status_t ret = EEP_SUCCESS;
    uint8 i = 0U;
    uint32 tempAddr = 0U;
    uint8 dateUnitSize = 0U;
    uint8 realLen = 0U;

    if ((0U == Data) || (gEepSize <= Addr) || (gEepSize < (Addr + Size)))
    {
        return STATUS_ERROR;
    }

    Eep_Status = EEP_HAL_BUSY;

    ret = EEP_FLASH_UnlockCtrl();
    while ((Size > 0U) && (EEP_SUCCESS == ret))
    {
        dateUnitSize = gEepFlashDateByteNum;
        tempAddr = Addr - (Addr % gEepFlashDateByteNum);
        realLen = gEepFlashDateByteNum - (Addr % gEepFlashDateByteNum);
        if (Size <  realLen)
        {
            dateUnitSize = Size;
            realLen = Size;
        }

        /* if current write data with EEP data is the same (the lenght is the min FLASH write uint),
            if no ,will write data, else, will write data
        */
        for (i = 0; i < realLen; i++)
        {
            if (Data[i] != *(volatile uint8 *)(Addr + gEepRamBaseAddr + i))
            {
                break;
            }
        }

        if (i < dateUnitSize)
        {
            for (i = 0; i < realLen; i++)
            {
                *(volatile uint8 *)(Addr + gEepRamBaseAddr + i) = Data[i];
            }
            ret = EEP_WriteExt((tempAddr + gEepRamBaseAddr),
                               (uint8 *)(tempAddr + gEepRamBaseAddr), dateUnitSize);
        }
        /* Update destination address */
        Addr += realLen;
        /* Update size */
        Size -= realLen;
        /* Update data */
        Data += realLen;
    }
    EEP_FLASH_LockCtrl();

    Eep_Status = (ret == EEP_SUCCESS) ? EEP_HAL_IDLE : EEP_HAL_TIMEOUT;

    return (ret == EEP_SUCCESS) ? STATUS_SUCCESS : STATUS_ERROR;
}

/*!
 * @brief EEP read
 *
 * @param[in] addr: EEP read address
 * @param[in] data: Point to the buffer ready to read the data from EEP
 * @param[in] size: the number of read data(uint is byte)
 * @return EEP operate status
 */
Hal_StatusType Eep_Hal_Read(uint32 Addr, uint8 *Data, uint16 Size)
{
    uint32 i;

    if ((0U == Data) || (gEepSize <= Addr) || (gEepSize < (Addr + Size)))
    {
        return STATUS_ERROR;
    }

    for (i = 0U; i < Size; i++)
    {
        /* check data effective */
        Data[i] = *(volatile uint8 *)(gEepRamBaseAddr + Addr + i);
    }

    return STATUS_SUCCESS;
}

/*!
 * @brief erase all EEP space
 *
 * @param[in] none
 * @return EEP operate status
 */
Hal_StatusType Eep_Hal_Erase(void)
{
    eep_status_t ret = EEP_SUCCESS;
    uint16 i;

    ret = EEP_FLASH_UnlockCtrl();
    if (EEP_SUCCESS == ret)
    {
        Eep_Status = EEP_HAL_BUSY;

        for (i = 0U; i < EEP_GROUPS_NUM; i++)
        {
            /* erase eep space */
            ret = EEP_FLASH_VerifyGroup(gEepFlashBaseAddr + i * gEepGroupSize, gEepGroupSize);
            if (EEP_SUCCESS != ret)
            {
                ret = EEP_FLASH_EraseGroup(gEepFlashBaseAddr + i * gEepGroupSize, gEepGroupSize);
            }
            if (EEP_SUCCESS != ret)
            {
                break;
            }
        }
    }
    if (EEP_SUCCESS == ret)
    {
        for (i = 0U; i < gEepSize; i += 4U)
        {
            *(volatile uint32 *)(gEepRamBaseAddr + i) = 0xFFFFFFFFU;
        }
    }
    EEP_FLASH_LockCtrl();

    Eep_Status = (ret == EEP_SUCCESS) ? EEP_HAL_IDLE : EEP_HAL_TIMEOUT;

    return (ret == EEP_SUCCESS) ? STATUS_SUCCESS : STATUS_ERROR;
}

/*!
 * @brief EEP initialize
 *
 * @param[in] config: EEP parameter struct
 * @return EEP operate status
 */
Hal_StatusType Eep_Hal_Init(const Eep_Hal_ConfigType *config)
{
    eep_status_t ret = EEP_SUCCESS;
    uint16 i;

    gEepSize = config->EepSize;
    if ((gEepSize % 128U) || (gEepSize > 4096U))
    {
        return STATUS_ERROR;
    }

    gEepFlashMinProByteNum = g_EepConfig.FlashMinProByte;
    gEepICType = g_EepConfig.icType;
    gEepFlashDateByteNum = (gEepICType == EEP_AC78036) ?
                           (gEepFlashMinProByteNum - 2U) :
                           (gEepFlashMinProByteNum - 4U);

    gEepFlashBaseAddr = config->EepFlashBaseAddr;
    gEepCallback = config->Callback;

    gEepGroupSize = gEepSize * gEepFlashMinProByteNum;
    if (gEepGroupSize < g_EepConfig.FlashPageSize)
    {
        gEepGroupSize = g_EepConfig.FlashPageSize;
    }

    gEepRamBaseAddr = config->EepRamBaseAddr;
    for (i = 0U; i < EEP_GROUPS_NUM; i++)
    {
        gEepGroupAddr[i][0U] = gEepFlashBaseAddr + i * gEepGroupSize;
        gEepGroupAddr[i][1U] = gEepFlashBaseAddr + (i + 1U) * gEepGroupSize;
    }

    for (i = 0U; i < (gEepFlashMinProByteNum >> 2U); i++)
    {
        gEepGroupFlag[i] = EEP_GROUP_FLAG_VALUE;
    }

    ret = EEP_FLASH_PreInitCheck(gEepRamBaseAddr, gEepFlashBaseAddr);
    if (ret != EEP_SUCCESS)
    {
        return STATUS_ERROR;
    }

    ret = EEP_FLASH_UnlockCtrl();
    if (EEP_SUCCESS == ret)
    {
        ret = UpdateOperateAddr();
    }

    EEP_FLASH_LockCtrl();

    Eep_Status = (ret == EEP_SUCCESS) ? EEP_HAL_INITIALIZED : EEP_HAL_UNINIT;

    return (ret == EEP_SUCCESS) ? STATUS_SUCCESS : STATUS_ERROR;
}

/*!
 * @brief Reload flash data to eeprom
 *
 * @param[in] none
 * @return EEP operate status
 */
Hal_StatusType Eep_Hal_Refresh(void)
{
    eep_status_t ret = EEP_SUCCESS;
    uint32 tmpCurFlashOperatAddr;

    tmpCurFlashOperatAddr = gCurFlashOperatAddr;
    EepMemset((uint8 *)gEepRamBaseAddr, 0xff, gEepSize);

    ret = LoadFlashDataToSram(gCurGroup, tmpCurFlashOperatAddr);

    return (ret == EEP_SUCCESS) ? STATUS_SUCCESS : STATUS_ERROR;
}

/*!
 * @brief Get EEP current status
 * @note  Function ID: [DES_EEP_API_204]
 * @return EEP current status busy or idle
 */
/*PRQA S 1503 ++*/
Eep_Hal_StatusType Eep_Hal_GetStatus(void)
{
    return Eep_Status;
}

/*!
 * @brief Get EEP sdk version
 * @param [in] Version: EEP version struct
 * @return void
 */
void EEP_Hal_GetVersion(Eep_Hal_VersionType *Version)
{
    if (Version != NULL_PTR)
    {
        Version->Major = EEP_DRV_SW_MAJOR_VERSION;
        Version->Minor = EEP_DRV_SW_MINOR_VERSION;
        Version->Patch = EEP_DRV_SW_PATCH_VERSION;
    }
}

#endif
/* =============================================  EOF  ============================================== */
