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
 * AutoChips Inc. (C) 2026. All rights reserved.
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
 * @file AC784xx_Fls_Reg.h
 *
 * @brief This file provides extern Reg Flash api.
 */

#ifndef AC784XX_FLS_REG_H
#define AC784XX_FLS_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/*Flash error status*/
#define FLASH_STAT_ERROR_BITS (FLASH_STAT_DFDIF_Msk |\
                               FLASH_STAT_PVIOLF_Msk |\
                               FLASH_STAT_VERIFYERR_Msk |\
                               FLASH_STAT_ACCERRF_Msk |\
                               FLASH_STAT_COLLERRF_Msk)
/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

/* =====================================  Functions definition  ===================================== */
/**
* @brief Set eflash lock
* @note Function ID:[DES_FLS_API_049]
* @return None
*/
LOCAL_INLINE void Flash_Reg_Lock(void)
{
    WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY2); /*cstat !MISRAC2012-Rule-11.4*/
    WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY1); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Set eflash unlock
* @note Function ID:[DES_FLS_API_109]
* @return None
*/
LOCAL_INLINE void Flash_Reg_UnLock(void)
{
    WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY1); /*cstat !MISRAC2012-Rule-11.4*/
    WRITE_REG32(FLASH->KEYUNLK, FLASH_UNLOCK_KEY2); /*cstat !MISRAC2012-Rule-11.4*/
}

LOCAL_INLINE void Flash_Reg_ClearStatus(void)
{
    MODIFY_REG32(FLASH->STAT, FLASH_STAT_ERROR_BITS, 0U, FLASH_STAT_ERROR_BITS); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Get the can eflash controler lock status
* @note Function ID:[DES_FLS_API_050]
* @return Lock Status.
*/
LOCAL_INLINE uint8 Flash_Reg_GetLockStatusReg(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->STAT) & FLASH_STAT_LOCK_Msk) >> FLASH_STAT_LOCK_Pos);
}

/**
* @brief Set P-Flash write protection status
* @note Function ID:[DES_FLS_API_053]
* @param [in] ProtectPos: protect bits Postion
* @param [in] ProtectStatus: protect status
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetPFlashProtection(uint8 ProtectPos, uint32 ProtectStatus)
{
#if defined (AC7843X)
    if (ProtectPos == 0U)
    {
        WRITE_REG32(FLASH->P0PROT0, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
    }
    else if (ProtectPos == 1U)
    {
        WRITE_REG32(FLASH->P0PROT1, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
    }
    else if (ProtectPos == 2U)
    {
        WRITE_REG32(FLASH->P1PROT0, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
    }
    else if (ProtectPos == 3U)
    {
        WRITE_REG32(FLASH->P1PROT1, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
    }
    else
    {
        /* Tntentionally empty */
    }
#elif defined (AC7840E)
    if (ProtectPos == 0U)
    {
        WRITE_REG32(FLASH->PPROT, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
    }
    else if (ProtectPos == 1U)
    {
        WRITE_REG32(FLASH->PPROT1, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
    }
    else
    {
         /* Tntentionally empty */
    }
#else
    (void)ProtectPos;
    WRITE_REG32(FLASH->PPROT, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
#endif
}

/**
* @brief Set D-Flash write protection status
* @note Function ID:[DES_FLS_API_054]
* @param [in] ProtectStatus: protect status
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetDFlashProtection(uint16 ProtectStatus)
{
    WRITE_REG32(FLASH->DPROT, ProtectStatus); /*cstat !MISRAC2012-Rule-11.4*/
}

/*!
 * @brief Set command complete interrupt register.
 *
 * @param [in] enable: command complete interrupt enable status
 * @return none
 */
LOCAL_INLINE void Flash_Reg_EnableCompleteIRQ(boolean enable)
{
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_CCIE_Msk, FLASH_CNFG_CCIE_Pos, enable);
}

/*!
 * @brief Set collision error interrupt register.
 *
 * @param [in] enable: collision interrupt enable status
 * @return none
 */
LOCAL_INLINE void Flash_Reg_EnableCollisionIRQ(boolean enable)
{
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_COLLE_Msk, FLASH_CNFG_COLLE_Pos, enable);
}

/*!
 * @brief Get Flash ECC 2-bit error interrupt flag.
 *
 * @return Flash ECC 2-bit error interrupt flag
 */
LOCAL_INLINE uint8 Flash_Reg_GetECCIRQFlag(void)
{
    return (uint8)((FLASH->STAT & FLASH_STAT_DFDIF_Msk) >> FLASH_STAT_DFDIF_Pos);
}

/*!
 * @brief Clear Flash ECC 2-bit error interrupt flag.
 *
 * @return none
 */
LOCAL_INLINE void Flash_Reg_ClearECCIRQFlag(void)
{
    FLASH->STAT |= FLASH_STAT_DFDIF_Msk;
}

/**
* @brief Set program or erase start address
* @note Function ID:[DES_FLS_API_055]
* @param [in] Addr: address value
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetAddressReg(uint32 Addr)
{
    WRITE_REG32(FLASH->ADDR, Addr); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Set program or erase length
* @note Function ID:[DES_FLS_API_056]
* @param [in] Len: len value
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetLengthReg(uint32 Len)
{
#if defined (AC7843X)
    WRITE_REG32(FLASH->LEN, ((Len == 0U) ? 0U : (Len - 1U))); /*cstat !MISRAC2012-Rule-11.4*/
#else
    WRITE_REG32(FLASH->LEN, Len); /*cstat !MISRAC2012-Rule-11.4*/
#endif
}

/**
* @brief Get abort page erase status
* @note Function ID:[DES_FLS_API_057]
* @return Abort page erase status
*/
LOCAL_INLINE uint8 Flash_Reg_GetAbortPageEraseStatusReg(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->CNFG) & FLASH_CNFG_PERABORT_Msk) >> FLASH_CNFG_PERABORT_Pos);
}

/**
* @brief Flash bort page erase
* @note Function ID:[DES_FLS_API_058]
* @return None
*/
LOCAL_INLINE void Flash_Reg_AbortPageEraseReg(void)
{
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_PERABORT_Msk, FLASH_CNFG_PERABORT_Pos, 1U); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Get the error status
* @note Function ID:[DES_FLS_API_059]
* @return Status
*/
LOCAL_INLINE uint32 Flash_Reg_GetStatusReg(void)
{
    return (READ_REG32(FLASH->STAT) & FLASH_STAT_ERROR_BITS); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Set command
* @note Function ID:[DES_FLS_API_060]
* @param [in] Cmd: Command value
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetCommandReg(uint32 Cmd)
{
    WRITE_REG32(FLASH->CMD, Cmd); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Trigger the command to start
* @note Function ID:[DES_FLS_API_061]
* @return None
*/
LOCAL_INLINE void Flash_Reg_TrigCtrlCmdReg(void)
{
    WRITE_REG32(FLASH->CST, FLASH_CST_START_Msk); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Get Trigger the FLASH operation command excute status
* @note Function ID:[DES_FLS_API_062]
* @return Trig control cmd reg status
*/
LOCAL_INLINE uint8 Flash_Reg_GetTrigCtrlCmdRegStatus(void)
{
    uint8 TrigCtrlCmdRegStatus;
    TrigCtrlCmdRegStatus = (uint8)(READ_REG32(FLASH->CST) & FLASH_CST_START_Msk); /*cstat !MISRAC2012-Rule-11.4*/
    return TrigCtrlCmdRegStatus;
}

/**
* @brief Get command completed status
* @note Function ID:[DES_FLS_API_063]
* @return Command complete status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCommandCompleteStatusReg(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->STAT) & FLASH_STAT_CCIF_Msk) >> FLASH_STAT_CCIF_Pos);
}

/**
* @brief Get backdoor verify status
* @note Function ID:[DES_FLS_API_110]
* @return Backdoor verify status
*/
LOCAL_INLINE uint8 Flash_Reg_GetBackdoorStatusReg(void)
{
#if !defined(AC7840E)
    return (uint8)((READ_REG32(FLASH->STAT) & FLASH_STAT_BDKERR_Msk) >> FLASH_STAT_BDKERR_Pos);
#else
    return 0U;
#endif
}

#if defined (AC7842X)
/**
* @brief Get cnfg emulvd status
* @note Function ID:[DES_FLS_API_111]
* @return Command complete status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCnfgEmulvdStatusReg(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->CNFG) & FLASH_CNFG_EMULVD_Msk) >> FLASH_CNFG_EMULVD_Pos);
}

/**
* @brief Set cnfg emulvd status
* @note Function ID:[DES_FLS_API_112]
* @param [in] enable: set enulvd function
* @return Command complete status
*/
LOCAL_INLINE void Flash_Reg_SetCnfgEmulvdStatusReg(boolean enable)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_EMULVD_Msk, FLASH_CNFG_EMULVD_Pos, enable);
}
#endif

/*!
 * @brief Set ECC 2-bit error interrupt register.
 *
 * @param [in] enable: ECC 2-bit error interrupt enable status
 * @return none
 */
LOCAL_INLINE void Flash_Reg_EnableECCIRQ(boolean enable)
{
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_DFDIE_Msk, FLASH_CNFG_DFDIE_Pos, enable);
}

/**
* @brief Get P-Flash write protection status
* @note Function ID:[DES_FLS_API_064]
* @param [in] ProtectPos: Get Pflash Protection status
* @return PFlash protection
*/
LOCAL_INLINE uint32 Flash_Reg_GetPFlashProtection(uint8 ProtectPos)
{
    uint32 prot;
#if defined (AC7843X)
    if (ProtectPos < 4U)
    {
        prot = READ_REG32(FLASH->P0PROT0 + ((uint32)ProtectPos * 4U));
    }
    else
    {
        prot = 0;
    }
#elif defined (AC7840E)
    if (ProtectPos < 2U)
    {
        prot = READ_REG32(FLASH->PPROT + ((uint32)ProtectPos * 4U));
    }
    else
    {
        prot = 0;
    }
#else
    (void)ProtectPos;
    prot = READ_REG32(FLASH->PPROT); /*cstat !MISRAC2012-Rule-11.4*/
#endif
    return prot;
}

/**
* @brief Get the error status
* @note Function ID:[DES_FLS_API_065]
* @return DFlash protection
*/
LOCAL_INLINE uint32 Flash_Reg_GetDFlashProtection(void)
{
    return (READ_REG32(FLASH->DPROT) & FLASH_DPROT_DPROT_Msk); /*cstat !MISRAC2012-Rule-11.4*/
}

/**
* @brief Set low 32-bit data
* @note Function ID:[DES_FLS_API_066]
* @param [in] Data: Data write to regsiter
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetLowDataReg(uint32 Data)
{
    WRITE_REG32(FLASH->DATA0, Data); /*cstat !MISRAC2012-Rule-11.4*/
}

/*!
 * @brief get low 32-bit data register value.
 *
 * @return Low 32-bit data register value
 */
LOCAL_INLINE uint32 Flash_Reg_GetLowDataReg(void)
{
    return (READ_REG32(FLASH->DATA0));
}

/*!
 * @brief Get high 32-bit data register value.
 *
 * @return High 32-bit data register value
 */
LOCAL_INLINE uint32 Flash_Reg_GetHighDataReg(void)
{
    return (READ_REG32(FLASH->DATA1));
}

/**
* @brief Set high 32-bit data
* @note Function ID:[DES_FLS_API_067]
* @param [in] Data: Data write to regsiter
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetHighDataReg(uint32 Data)
{
    WRITE_REG32(FLASH->DATA1, Data); /*cstat !MISRAC2012-Rule-11.4*/
}

#if defined (AC7843X) || defined(AC7840E)
LOCAL_INLINE uint8 Flash_Reg_GetDPartition(void)
{
    return 0xFU;
}

LOCAL_INLINE uint8 Flash_Reg_GetEPartition(void)
{
    return 0xFU;
}

LOCAL_INLINE uint8 Flash_Reg_GetCSEPartition(void)
{
    return 0xFU;
}

#if defined (AC7843X)
/*!
 * @brief Get command completed status of all flash bank, exclude D-Flash bank1 when HSM exist.
 *
 * @return 1: Command is completed
 *         0: Command is not completed
 */
LOCAL_INLINE uint8 Flash_Reg_GetEFCmdCompleteStatus(void)
{
    return (uint8)((FLASH->STAT & FLASH_STAT_EFCCIF_Msk) >> FLASH_STAT_EFCCIF_Pos);
}
#endif

/*!
 * @brief Clear command complete interrupt register.
 *
 * @return none
 */
LOCAL_INLINE void Flash_Reg_ClearCompleteIRQ(void)
{
    FLASH->STAT |= FLASH_STAT_CMDONOFF_Msk;
}
#else /* AC7840X AC7842X */
/**
* @brief Get DFLASH Partition
* @note Function ID:[DES_FLS_API_113]
* @return PFLASH PARTITION SIZE
*/
LOCAL_INLINE uint8 Flash_Reg_GetDPartition(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->PART) & FLASH_PART_DPART_Msk) >> FLASH_PART_DPART_Pos);
}

/**
* @brief Get FlexRAM Partition
* @note Function ID:[DES_FLS_API_114]
* @return DFLASH PARTITION SIZE
*/
LOCAL_INLINE uint8 Flash_Reg_GetEPartition(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->PART) & FLASH_PART_EPART_Msk) >> FLASH_PART_EPART_Pos);
}

/**
* @brief Get CSE Partition
* @note Function ID:[DES_FLS_API_115]
* @return CSE PARTITION SIZE
*/
LOCAL_INLINE uint8 Flash_Reg_GetCSEPartition(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((READ_REG32(FLASH->PART) & FLASH_PART_CSEKEYSIZE_Msk) >> FLASH_PART_CSEKEYSIZE_Pos);
}

/*!
 * @brief Get FlexRAM ECC 1-bit error flag.
 *
 * @return Flash FlexRAM ECC 1-bit error interrupt flag
 */
LOCAL_INLINE uint8 FLASH_GetFRAMECC1BitFlag(void)
{
    return (uint8)((FLASH->STAT & FLASH_STAT_FRAMECC1F_Msk) >> FLASH_STAT_FRAMECC1F_Pos);
}

/*!
 * @brief Clear FlexRAM ECC 1-bit error flag.
 *
 * @return none
 */
LOCAL_INLINE void FLASH_ClearFRAMECC1BitFlag(void)
{
    FLASH->STAT |= FLASH_STAT_FRAMECC1F_Msk;
}

/*!
 * @brief Get FlexRAM ECC 2-bit error flag.
 *
 * @return Flash FlexRAM ECC 2-bit error interrupt flag
 */
LOCAL_INLINE uint8 FLASH_GetFRAMECC2BitFlag(void)
{
    return (uint8)((FLASH->STAT & FLASH_STAT_FRAMECC2F_Msk) >> FLASH_STAT_FRAMECC2F_Pos);
}

/*!
 * @brief Clear FlexRAM ECC 2-bit error flag.
 *
 * @return none
 */
LOCAL_INLINE void FLASH_ClearFRAMECC2BitFlag(void)
{
    FLASH->STAT |= FLASH_STAT_FRAMECC2F_Msk;
}
#endif

/**
* @brief Set LVD Detection
* @note Function ID:[DES_FLS_API_068]
* @param [in] IsEnable: True enable LVD Detection, false disable rtc module
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetLVDDetection(boolean IsEnable)
{
    /*cstat !MISRAC2012-Rule-10.5 !MISRAC2012-Rule-11.4*/
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_LVDDETEN_Msk, FLASH_CNFG_LVDDETEN_Pos,
                 (uint32)IsEnable);
}

/**
* @brief Get Ecc Error Address Info
* @note Function ID:[DES_FLS_API_116]
* @return None
*/
LOCAL_INLINE uint32 Flash_Reg_ECCErrorAddr(void)
{
    return READ_REG32(FLASH->DFADR); /*cstat !MISRAC2012-Rule-10.5 !MISRAC2012-Rule-11.4*/
}

/**
* @brief Set LVD Detection
* @note Function ID:[DES_FLS_API_117]
* @param [in] IsEnable: True enable LVD Detection, false disable rtc module
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetEMDFIE(boolean IsEnable)
{
    /*cstat !MISRAC2012-Rule-10.5 !MISRAC2012-Rule-11.4*/
    MODIFY_REG32(FLASH->CNFG, FLASH_CNFG_EMDFIE_Msk, FLASH_CNFG_EMDFIE_Pos, (uint32)IsEnable);
}

#if defined (AC7842X) || defined (AC7843X)
/**
* @brief Get Swap status
* @note Function ID:[DES_FLS_API_118]
* @return None
*/
LOCAL_INLINE uint32 Flash_Reg_GetSwapStatus(void)
{
    /*cstat !MISRAC2012-Rule-10.5 !MISRAC2012-Rule-11.4*/
    return READ_REG32(FLASH->SWAPSTAT) & FLASH_SWAP_FLAG_Msk;
}
#endif

/*!
 * @brief Get the flash read protection status.
 *
 * @return 2b'11: Read protection is disabled
 *         2b'00/2b'01/2b'10: Read protection is enabled
 */
LOCAL_INLINE uint8 Flash_Reg_GetReadProtectStatus(void)
{
    /*cstat !MISRAC2012-Rule-11.4*/
    return (uint8)((FLASH->SEC & FLASH_SEC_RDPROT_Msk) >> FLASH_SEC_RDPROT_Pos);
}

/**
 * @brief Get the backdoor key verify enable status.
 *
 * @return 2b'11: Backdoor key can not disable read protection
 *         2b'00/2b'01/2b'10: Backdoor key can enable read protection
 * @note Function ID:[DES_FLS_API_108]
 */
LOCAL_INLINE uint8 Flash_Reg_GetBackdoorKeyStatus(void)
{
#if !defined(AC7840E)
    return (uint8)((FLASH->SEC & FLASH_SEC_BDKEN_Msk) >> FLASH_SEC_BDKEN_Pos);
#else
    return 0U;
#endif
}

/*!
 * @brief Clear Flash ECC 2-bit error address.
 *
 * @return none
 */
LOCAL_INLINE void Flash_Reg_ClearECCErrorAddress(void)
{
    FLASH->DFADR = 0;
}

/*!
 * @brief Get collision error interrupt flag.
 *
 * @return Collision error interrupt flag
 */
LOCAL_INLINE uint8 Flash_Reg_GetCollIRQFlag(void)
{
    return (uint8)((FLASH->STAT & FLASH_STAT_COLLERRF_Msk) >> FLASH_STAT_COLLERRF_Pos);
}

/*!
 * @brief Clear collision error interrupt flag.
 *
 * @return none
 */
LOCAL_INLINE void Flash_Reg_ClearCollIRQFlag(void)
{
    FLASH->STAT |= FLASH_STAT_COLLERRF_Msk;
}

/*!
 * @brief force ECC 2-bit error detected, code should run in SRAM, usually used it for test.
 *
 * @return none
 */
LOCAL_INLINE void Flash_Reg_ForceECCErrorDetect(void)
{
    FLASH->CNFG |= FLASH_CNFG_EMDFIE_Msk;
}

/*!
 * @brief Get flash CNFG register value.
 *
 * @return CNFG register value
 */
LOCAL_INLINE uint32 Flash_Reg_GetCnfg(void)
{
    return FLASH->CNFG;
}

#if defined(AC7840E)
/**
* @brief Trigger a refresh operationfor the SYM Key Area.
* @note Function ID:[DES_FLS_API_090]
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetFLSK(void)
{
    MODIFY_REG32(FLASH->SECCTL, FLASH_SECCTL_FSK_Msk, FLASH_SECCTL_FSK_Pos, 1U);
}

/**
* @brief Set OTP Select register value
* @details This function selects between real OTP page and simulated OTP page.
* This function is only effective when EMLK is in locked state.
* @note Function ID:[DES_FLS_API_091]
* @param [in] OTPSel: OTP selection value
*         - TRUE (1): Select real OTP page
*         - FALSE (0): Select simulated OTP page
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetOTPSelReg(boolean OTPSel)
{
    MODIFY_REG32(FLASH->SECCTL, FLASH_SECCTL_OTPSEL_Msk, FLASH_SECCTL_OTPSEL_Pos, (uint32)OTPSel);
}

/**
* @brief Get OTP Select register value
* @details This function retrieves the current OTP selection status.
*          Returns 1 if real OTP page is selected, 0 if simulated OTP page is selected.
* @note Function ID:[DES_FLS_API_092]
* @return OTP selection status
*/
LOCAL_INLINE uint8 Flash_Reg_GetOTPSelReg(void)
{
    return (uint8)((READ_REG32(FLASH->SECCTL) & FLASH_SECCTL_OTPSEL_Msk) >> FLASH_SECCTL_OTPSEL_Pos);
}

/**
* @brief Set EM Lock register value
* @details This function controls the simulation chip lock state.
*          When Lock is TRUE (1), the simulation chip enters locked state.
* @note Function ID:[DES_FLS_API_093]
* @param [in] Lock: Lock state value
*         - TRUE (1): Simulation chip enters locked state
*         - FALSE (0): Simulation chip is unlocked
* @return None
*/LOCAL_INLINE void Flash_Reg_SetEMLKReg(boolean Lock)
{
    MODIFY_REG32(FLASH->SECCTL, FLASH_SECCTL_EMLK_Msk, FLASH_SECCTL_EMLK_Pos, (uint32)Lock);
}

/**
* @brief Get EM Lock register value
* @details This function retrieves the current EM lock status.
*          Returns 1 if simulation chip is in locked state, 0 if unlocked.
* @note Function ID:[DES_FLS_API_094]
* @return EM lock status
*         - 1: Simulation chip is in locked state
*         - 0: Simulation chip is unlocked
*/
LOCAL_INLINE uint8 Flash_Reg_GetEMLKReg(void)
{
    return (uint8)((READ_REG32(FLASH->SECCTL) & FLASH_SECCTL_EMLK_Msk) >> FLASH_SECCTL_EMLK_Pos);
}

/**
* @brief Set CFG SEL register value
* @details This function sets the OTP configuration selection value.
*          CFG_SEL determines which OTP configuration data is visible in FLASH_CFGVIEW register.
* @note Function ID:[DES_FLS_API_095]
* @param [in] CfgSel: Configuration selection value (0-15)
*         - 0: CFG_VIEW[31:0]: swap configuration address offset
*         - 1: CFG_VIEW[9:0]: Firmware area page count
*         - 2: CFG_VIEW[9:0]: Bootloader area page count
*         - 3: CFG_VIEW[31:0]: Firmware area start address
*         - 4: CFG_VIEW[9:0]: Asymmetric key area page count
*         - 5: CFG_VIEW[9:8]: Authentication algorithm, CFG_VIEW[5:4]: Debug port unlock algorithm, CFG_VIEW[1:0]: Secure boot algorithm
* @return None
*/
LOCAL_INLINE void Flash_Reg_SetCFGSelReg(uint8 CfgSel)
{
    MODIFY_REG32(FLASH->CFGSEL, FLASH_CFGSEL_CFG_SEL_Msk, FLASH_CFGSEL_CFG_SEL_Pos, (uint32)CfgSel);
}

/**
* @brief Get CFG SEL register value
* @details This function retrieves the current OTP configuration selection value.
* @note Function ID:[DES_FLS_API_096]
* @return Configuration selection value (0-15)
*/
LOCAL_INLINE uint8 Flash_Reg_GetCFGSelReg(void)
{
    return (uint8)((READ_REG32(FLASH->CFGSEL) & FLASH_CFGSEL_CFG_SEL_Msk) >> FLASH_CFGSEL_CFG_SEL_Pos);
}

/**
* @brief Get CFG VIEW register value
* @details This function retrieves the OTP configuration view data.
*          The meaning of the returned value depends on the current CFG_SEL setting.
* @note Function ID:[DES_FLS_API_097]
* @return OTP configuration view data (32-bit)
*/
LOCAL_INLINE uint32 Flash_Reg_GetCFGViewReg(void)
{
    return READ_REG32(FLASH->CFGVIEW);
}

/**
* @brief Get CFGDEC register value
* @details This function retrieves the OTP configuration decode register value.
* @note Function ID:[DES_FLS_API_098]
* @return CFGDEC register value (32-bit)
*/
LOCAL_INLINE uint32 Flash_Reg_GetCFGDECReg(void)
{
    return READ_REG32(FLASH->CFGDEC);
}

/**
* @brief Get ROM Code Enable status from CFGDEC register
* @note Function ID:[DES_FLS_API_099]
* @return ROM Code Enable status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCFGDECRomCodeEn(void)
{
    return (uint8)((READ_REG32(FLASH->CFGDEC) & FLASH_CFGDEC_ROMCODEEN_Msk) >> FLASH_CFGDEC_ROMCODEEN_Pos);
}

/**
* @brief Get Bootloader Enable status from CFGDEC register
* @note Function ID:[DES_FLS_API_100]
* @return Bootloader Enable status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCFGDECBLEn(void)
{
    return (uint8)((READ_REG32(FLASH->CFGDEC) & FLASH_CFGDEC_BLEN_Msk) >> FLASH_CFGDEC_BLEN_Pos);
}

/**
* @brief Get Firmware Enable status from CFGDEC register
* @note Function ID:[DES_FLS_API_101]
* @return Firmware Enable status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCFGDECFWEn(void)
{
    return (uint8)((READ_REG32(FLASH->CFGDEC) & FLASH_CFGDEC_FWEN_Msk) >> FLASH_CFGDEC_FWEN_Pos);
}

/**
* @brief Get Bootloader as OTP status from CFGDEC register
* @note Function ID:[DES_FLS_API_102]
* @return Bootloader as OTP status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCFGDECBLASOTP(void)
{
    return (uint8)((READ_REG32(FLASH->CFGDEC) & FLASH_CFGDEC_BLASOTP_Msk) >> FLASH_CFGDEC_BLASOTP_Pos);
}

/**
* @brief Get Firmware as OTP status from CFGDEC register
* @note Function ID:[DES_FLS_API_103]
* @return Firmware as OTP status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCFGDECFWASOTP(void)
{
    return (uint8)((READ_REG32(FLASH->CFGDEC) & FLASH_CFGDEC_FWASOTP_Msk) >> FLASH_CFGDEC_FWASOTP_Pos);
}

/**
* @brief Get Secure Boot Enable status from CFGDEC register
* @note Function ID:[DES_FLS_API_104]
* @return Secure Boot Enable status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCFGDECSecBootEn(void)
{
    return (uint8)((READ_REG32(FLASH->CFGDEC) & FLASH_CFGDEC_SECBOOTEN_Msk) >> FLASH_CFGDEC_SECBOOTEN_Pos);
}

/**
* @brief Get Secure Boot Bypass status from CFGDEC register
* @note Function ID:[DES_FLS_API_105]
* @return Secure Boot Bypass status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCFGDECSecBootByp(void)
{
    return (uint8)((READ_REG32(FLASH->CFGDEC) & FLASH_CFGDEC_SECBOOTBYP_Msk) >> FLASH_CFGDEC_SECBOOTBYP_Pos);
}

/**
* @brief Get Debug Auth Way status from CFGDEC register
* @note Function ID:[DES_FLS_API_106]
* @return Debug Auth Way status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCFGDECDbgAuthWay(void)
{
    return (uint8)((READ_REG32(FLASH->CFGDEC) & FLASH_CFGDEC_DBGAUTHWAY_Msk) >> FLASH_CFGDEC_DBGAUTHWAY_Pos);
}

/**
* @brief Get HSM Enable status from CFGDEC register
* @note Function ID:[DES_FLS_API_107]
* @return HSM Enable status
*/
LOCAL_INLINE uint8 Flash_Reg_GetCFGDECHSMEn(void)
{
    return (uint8)((READ_REG32(FLASH->CFGDEC) & FLASH_CFGDEC_HSMEN_Msk) >> FLASH_CFGDEC_HSMEN_Pos);
}

#endif
/*cstat +MISRAC2012-Rule-10.5*/
#ifdef __cplusplus
}
#endif
#endif
/*============================================EOF==========================================*/
