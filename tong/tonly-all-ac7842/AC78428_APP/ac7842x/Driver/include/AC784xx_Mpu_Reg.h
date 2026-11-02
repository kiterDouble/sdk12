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

/*!
 * @file AC784xx_Mpu_Reg.h
 *
 * @brief This file provides extern Low level Mpu register api.
 *
 */

#ifndef AC784XX_MPU_REG_H
#define AC784XX_MPU_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
static MPU_Type *const MpuBase[MPU_INSTANCE_MAX] = MPU_BASE_PTRS;

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Enable MPU.
 * @note      Function ID: DES_MPU_API_301
 * @param[in] MpuIdx: MPU number, 0~1.
 * @return void
 */
LOCAL_INLINE void Mpu_Reg_Enable(uint8 MpuIdx)
{
    WRITE_REG32(MpuBase[MpuIdx]->CESR, MPU_CESR_VLD_Msk);
}

/*!
 * @brief Disable MPU.
 * @note      Function ID: DES_MPU_API_302
 * @param[in] MpuIdx: MPU number, 0~1.
 * @return void
 */
LOCAL_INLINE void Mpu_Reg_Disable(uint8 MpuIdx)
{
    WRITE_REG32(MpuBase[MpuIdx]->CESR, 0U);
}

/*!
 * @brief  Get the MPU error status.
 * @note      Function ID: DES_MPU_API_303
 * @param[in] MpuIdx: MPU number, 0~1.
 * @return uint32 : MPU error status.
 */
LOCAL_INLINE uint32 Mpu_Reg_GetStatus(uint8 MpuIdx)
{
    return READ_REG32(MpuBase[MpuIdx]->CESR);
}

/*!
 * @brief Clear the MPU error status.
 * @note      Function ID: DES_MPU_API_304
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] Status: the MPU error status.
 * @return void
 */
LOCAL_INLINE void Mpu_Reg_ClearStatus(uint8 MpuIdx, uint32 Status)
{
    WRITE_REG32(MpuBase[MpuIdx]->CESR, Status);
}

/*!
 * @brief  Get the MPU port error address.
 * @note      Function ID: DES_MPU_API_305
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] PortIdx: MPU Slave port number.
 * @return uint32: MPU error address.
 */
LOCAL_INLINE uint32 Mpu_Reg_GetErrorAddr(uint8 MpuIdx, uint8 PortIdx)
{
    return READ_REG32(MpuBase[MpuIdx]->EAR[PortIdx]);
}

/*!
 * @brief  Get the MPU port error information.
 * @note      Function ID: DES_MPU_API_306
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] PortIdx: MPU Slave port number.
 * @return uint32: MPU error information.
 */
LOCAL_INLINE uint32 Mpu_Reg_GetErrorInfo(uint8 MpuIdx, uint8 PortIdx)
{
    return READ_REG32(MpuBase[MpuIdx]->EDR[PortIdx]);
}

/*!
 * @brief  Clear the MPU port error information.
 * @note      Function ID: DES_MPU_API_307
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] PortIdx: MPU Slave port number.
 * @return void
 */
LOCAL_INLINE void Mpu_Reg_ClearErrorInfo(uint8 MpuIdx, uint8 PortIdx)
{
    SET_BIT32(MpuBase[MpuIdx]->EDR[PortIdx], MPU_EDR0_EFLG_Msk);
}

/*!
 * @brief  Set the MPU master PID.
 * @note      Function ID: DES_MPU_API_308
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] MasterNum: MPU amster number
 * @param[in] Pid: pid value
 * @return void
 */
LOCAL_INLINE void Mpu_Reg_SetMasterPid(uint8 MpuIdx, uint8 MasterNum, uint8 Pid)
{
#if !defined(AC7840E)
    uint32 Msk = ((uint32)MPU_MPID_M0PID_Msk << (MPU_MPID_M1PID_Pos * MasterNum));

    MODIFY_REG32(MpuBase[MpuIdx]->MPID, Msk, (MPU_MPID_M1PID_Pos * MasterNum), Pid);
#else
    (void)MpuIdx;
    (void)MasterNum;
    (void)Pid;
#endif
}

/*!
 * @brief  Get the MPU master PID.
 * @note      Function ID: DES_MPU_API_309
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] MasterNum: MPU amster number
 * @return uint8: master Pid
 */
LOCAL_INLINE uint8 Mpu_Reg_GetMasterPid(uint8 MpuIdx, uint8 MasterNum)
{
#if !defined(AC7840E)
    return (uint8)(READ_BIT32(MpuBase[MpuIdx]->MPID, ((uint32)MPU_MPID_M0PID_Msk << (MasterNum * MPU_MPID_M1PID_Pos))) & 0xFFU);
#else
    (void)MpuIdx;
    (void)MasterNum;
    return 0U;
#endif
}

/*!
 * @brief  Set the MPU region start address.
 * @note      Function ID: DES_MPU_API_310
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @param[in] Addr: region start address.
 * @return void
 */
LOCAL_INLINE void Mpu_Reg_SetRegionStartAddr(uint8 MpuIdx, uint8 RegionIdx, uint32 Addr)
{
    WRITE_REG32(MpuBase[MpuIdx]->WORD0_RGD[RegionIdx], Addr);
}

/*!
 * @brief  Get the MPU region start address.
 * @note      Function ID: DES_MPU_API_311
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @return uint32 : MPU region start address.
 */
LOCAL_INLINE uint32 Mpu_Reg_GetRegionStartAddr(uint8 MpuIdx, uint8 RegionIdx)
{
    return READ_REG32(MpuBase[MpuIdx]->WORD0_RGD[RegionIdx]);
}

/*!
 * @brief  Set the MPU region end address.
 * @note      Function ID: DES_MPU_API_312
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @param[in] Addr: region end address.
 * @return void
 */
LOCAL_INLINE void Mpu_Reg_SetRegionEndAddr(uint8 MpuIdx, uint8 RegionIdx, uint32 Addr)
{
    WRITE_REG32(MpuBase[MpuIdx]->WORD1_RGD[RegionIdx], Addr);
}

/*!
 * @brief  Get the MPU region end address.
 * @note      Function ID: DES_MPU_API_313
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @return uint32 : MPU region end address.
 */
LOCAL_INLINE uint32 Mpu_Reg_GetRegionEndAddr(uint8 MpuIdx, uint8 RegionIdx)
{
    return READ_REG32(MpuBase[MpuIdx]->WORD1_RGD[RegionIdx]);
}

/*!
 * @brief  Set the MPU region attributes.
 * @note      Function ID: DES_MPU_API_314
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @param[in] Attrs: region attributes.
 * @return none
 */
LOCAL_INLINE void Mpu_Reg_SetRegionAttrs(uint8 MpuIdx, uint8 RegionIdx, uint32 Attrs)
{
    WRITE_REG32(MpuBase[MpuIdx]->WORD2_RGD[RegionIdx], Attrs);
}

/*!
 * @brief  Set the MPU region attributes.
 * @note      Function ID: DES_MPU_API_315
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @return uint32 : region attributes
 */
LOCAL_INLINE uint32 Mpu_Reg_GetRegionAttrs(uint8 MpuIdx, uint8 RegionIdx)
{
    return READ_REG32(MpuBase[MpuIdx]->WORD2_RGD[RegionIdx]);
}

/*!
 * @brief  Set the MPU region pid.
 * @note Function ID: DES_MPU_API_316
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @param[in] PidMask: Pid Mask
 * @return none
 */
LOCAL_INLINE void Mpu_Reg_SetRegionPid(uint8 MpuIdx, uint8 RegionIdx, uint8 Pid, uint8 PidMask)
{
#if !defined(AC7840E)
    MODIFY_REG32(MpuBase[MpuIdx]->WORD3_RGD[RegionIdx], MPU_RGD0_WORD3_PID_Msk, MPU_RGD0_WORD3_PID_Pos, Pid);
    MODIFY_REG32(MpuBase[MpuIdx]->WORD3_RGD[RegionIdx], MPU_RGD0_WORD3_PIDMASK_Msk, MPU_RGD0_WORD3_PIDMASK_Pos, PidMask);
#else
    (void)MpuIdx;
    (void)RegionIdx;
    (void)Pid;
    (void)PidMask;
#endif
}

/*!
 * @brief  Set the MPU region pid.
 * @note      Function ID: DES_MPU_API_317
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @return uint32: region pid
 */
LOCAL_INLINE uint32 Mpu_Reg_GetRegionPid(uint8 MpuIdx, uint8 RegionIdx)
{
    return MpuBase[MpuIdx]->WORD3_RGD[RegionIdx];
}

/*!
 * @brief  Enable the MPU region.
 * @note      Function ID: DES_MPU_API_318
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @param[in] En: 0 disable  1:enable
 * @return none
 */
LOCAL_INLINE void Mpu_Reg_EnableRegion(uint8 MpuIdx, uint8 RegionIdx, boolean En)
{
    MODIFY_REG32(MpuBase[MpuIdx]->WORD3_RGD[RegionIdx], MPU_RGD0_WORD3_VLD_Msk, MPU_RGD0_WORD3_VLD_Pos, En);
}

/*!
 * @brief  Enable the MPU region PID.
 * @note      Function ID: DES_MPU_API_320
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @param[in] MasterNum: MPU Master number
 * @return void
 */
LOCAL_INLINE void Mpu_Reg_EnableRegionPid(uint8 MpuIdx, uint8 RegionIdx, uint8 MasterNum)
{
#if !defined(AC7840E)
    SET_BIT32(MpuBase[MpuIdx]->WORD2_RGD[RegionIdx], (MPU_RGD0_WORD2_M0PE_Msk >> MasterNum));
#else
    (void)MpuIdx;
    (void)RegionIdx;
    (void)MasterNum;
#endif
}

/*!
 * @brief  Disable the MPU region PID.
 * @note  Function ID: DES_MPU_API_321
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @param[in] MasterNum: MPU Master number
 * @return void
 */
LOCAL_INLINE void Mpu_Reg_DisableRegionPid(uint8 MpuIdx, uint8 RegionIdx, uint8 MasterNum)
{
#if !defined(AC7840E)
    CLEAR_BIT32(MpuBase[MpuIdx]->WORD2_RGD[RegionIdx], (MPU_RGD0_WORD2_M0PE_Msk >> MasterNum));
#else
    (void)MpuIdx;
    (void)RegionIdx;
    (void)MasterNum;
#endif
}

/*!
 * @brief Set the MPU region attributes.
 * @note Function ID: DES_MPU_API_322
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @param[in] Attrs: attributes.
 * @return void
 */
LOCAL_INLINE void Mpu_Reg_SetRegionACCAttrs(uint8 MpuIdx, uint8 RegionIdx, uint32 Attrs)
{
    WRITE_REG32(MpuBase[MpuIdx]->RGDAAC[RegionIdx], Attrs);
}

/*!
 * @brief Set the MPU region attributes.
 * @note Function ID: DES_MPU_API_323
 * @param[in] MpuIdx: MPU number, 0~1.
 * @param[in] RegionIdx: MPU Region number, 0~7.
 * @return uint32: attributes.
 */
LOCAL_INLINE uint32 Mpu_Reg_GetRegionACCAttrs(uint8 MpuIdx, uint8 RegionIdx)
{
    return READ_REG32(MpuBase[MpuIdx]->RGDAAC[RegionIdx]);
}

/*!
 * @brief  Disable the MPU region PID.
 * @note   Function ID: DES_MPU_API_324
 * @param[in] MpuIdx: MPU number, 0~1.
 * @return uint32: 0:disable 1:enable
 */
LOCAL_INLINE uint32 Mpu_Reg_GetmpuEnable(uint8 MpuIdx)
{
    return EXTRACT_BIT32(MpuBase[MpuIdx]->CESR, MPU_CESR_VLD_Msk, MPU_CESR_VLD_Pos);
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* AC784XX_MPU_REG_H */
