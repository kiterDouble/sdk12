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
 * @file Mpu_Hal.c
 *
 * @brief This file provides Hal Mpu api.
 *
 */

/*==============================================INCLUDE FILES=======================================*/
#include "Mpu_Hal.h"
/*===============================================FILE VERION========================================*/

/*==============================================LOCAL MACROS========================================*/
#define  MPU_MASTER_PID_ENABLE_MASK  0xF0000000UL
#define  MPU_PERMISSION_MASK         0xFFFFFFFUL
/*=============================================LOCAL TYPEDEFS=======================================*/

/*=============================================LOCAL CONSTANTS======================================*/

/*=============================================LOCAL VARIABLES======================================*/

/*============================================GLOBAL CONSTANTS======================================*/

/*============================================GLOBAL VARIABLES======================================*/

/*=======================================LOCAL FUNCTION PROTOTYPES==================================*/

/*============================================LOCAL FUNCTIONS=======================================*/

/*============================================GLOBAL FUNCTIONS======================================*/
/* Implementation of Mpu_Hal_Init declared in Mpu_Hal.h */
void Mpu_Hal_Init(const Mpu_ConfigType *ConfigPtr) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
    Mpu_RegionIdType RegionId;
    uint8 MpuIdx;
    uint8 RegionIdx;
    const Mpu_ModuleConfigType *ModuleConfig; /*!< mpu module configure pointer */
    const Mpu_RegionConfigType *RegionConfig;
    const Mpu_RegionParamsType *RegionParams; /*!< region attributes pointer */
    Mpu_IdType MpuId;                         /*!< Mpu module id */

    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
    /* PRQA S 2812 ++ # AMCE_MCL_2812*/
    DEVICE_ASSERT(ConfigPtr->ModuleConfigCnt <= (uint8)MPU_ID_MAX);
    /* PRQA S 2812 -- # AMCE_MCL_2812*/

    /* init all mpu */
    for (MpuIdx = 0U; MpuIdx < ConfigPtr->ModuleConfigCnt; MpuIdx++)
    {
        ModuleConfig = &ConfigPtr->ModuleConfig[MpuIdx];
        if (ModuleConfig != NULL_PTR)
        {
            /* PRQA S 2812 ++ # AMCE_MCL_2812*/
            MpuId = ModuleConfig->MpuId ;
            /* PRQA S 2812 -- # AMCE_MCL_2812*/
            /*mpu disable*/
            Mpu_Reg_Disable((uint8)MpuId);
            /*modify rgd0 word2 to disable region 0,for using mpu*/
            Mpu_Reg_SetRegionACCAttrs((uint8)MpuId, (uint8)MPU_REGION_ID_0, 0U);
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
#if MPU_HAS_PROCESS_IDENTIFIER
            for (uint8 MasterIdx = 0U; MasterIdx < (uint8)MPU_MASTER_MAX; MasterIdx++)
            {
                /* only dma mstetr not set pid */
                if ((uint8)MPU_MASTER_DMA != MasterIdx)
                {
                    Mpu_Reg_SetMasterPid((uint8)MpuId, MasterIdx, ModuleConfig->MasterPid[MasterIdx]);
                }
            }
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
#endif
            /* init all region */
            for (RegionIdx = 0U; RegionIdx < ModuleConfig->RegionConfigCnt; RegionIdx++)
            {
                RegionConfig = &ModuleConfig->RegionConfig[RegionIdx];
                RegionId = RegionConfig->RegionId;
                RegionParams = &RegionConfig->RegionParams;
                Mpu_Reg_SetRegionStartAddr((uint8)MpuId, (uint8)RegionId, RegionParams->Addr.StartAddr);
                Mpu_Reg_SetRegionEndAddr((uint8)MpuId, (uint8)RegionId, RegionParams->Addr.EndAddr);
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
#if MPU_HAS_PROCESS_IDENTIFIER
                Mpu_Reg_SetRegionACCAttrs((uint8)MpuId, (uint8)RegionId, \
                                          RegionParams->Attrs.Permission | RegionParams->Attrs.MasterPidEnable);
                Mpu_Reg_SetRegionPid((uint8)MpuId, (uint8)RegionId, (uint8)RegionParams->Attrs.Pid, \
                                     (uint8)RegionParams->Attrs.PidMask);
#else
                Mpu_Reg_SetRegionACCAttrs((uint8)MpuId, (uint8)RegionId, RegionParams->Attrs.Permission);
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
#elif defined (AC7840E)
                Mpu_Reg_SetRegionACCAttrs((uint8)MpuId, (uint8)RegionId, RegionParams->Attrs.Permission);
#endif
                Mpu_Reg_EnableRegion((uint8)MpuId, (uint8)RegionId, TRUE);
            }
            /*mpu enable*/
            Mpu_Reg_Enable((uint8)MpuId);
        }
        else
        {
            /* nothing */
        }
    }
}

/* Implementation of Mpu_Hal_Deinit declared in Mpu_Hal.h */
void Mpu_Hal_Deinit(void) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
    uint8 MpuIdx;
    uint8 RegionIdx;

    /* deinit all mpu */
    /* PRQA S 2877 ++ # AMCE_MCL_2877*/
    for (MpuIdx = 0U; MpuIdx < MPU_INSTANCE_MAX; MpuIdx++)
    {
        Mpu_Reg_Disable(MpuIdx);
        /* deinit all region */
        for (RegionIdx = 1U; RegionIdx < (uint8)MPU_REGION_ID_MAX; RegionIdx++)
        {
            Mpu_Reg_EnableRegion(MpuIdx, RegionIdx, FALSE);
        }
    }
    /* PRQA S 2877 -- # AMCE_MCL_2877*/
}

/* Implementation of Mpu_Hal_SetRegionAddr declared in Mpu_Hal.h */
void Mpu_Hal_SetRegionAddr(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, const Mpu_RegionAddrConfigType *AddrConfigPtr) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
    DEVICE_ASSERT(MpuId < MPU_ID_MAX);
    DEVICE_ASSERT(RegionId < MPU_REGION_ID_MAX);
    DEVICE_ASSERT(AddrConfigPtr != NULL_PTR);
    Mpu_Reg_EnableRegion((uint8)MpuId, (uint8)RegionId, FALSE);
    /* PRQA S 2812 ++ # AMCE_MCL_2812*/
    Mpu_Reg_SetRegionStartAddr((uint8)MpuId, (uint8)RegionId, AddrConfigPtr->StartAddr);
    /* PRQA S 2812 -- # AMCE_MCL_2812*/
    Mpu_Reg_SetRegionEndAddr((uint8)MpuId, (uint8)RegionId, AddrConfigPtr->EndAddr);
    Mpu_Reg_EnableRegion((uint8)MpuId, (uint8) RegionId, TRUE);
}

/* Implementation of Mpu_Hal_GetRegionInfo declared in Mpu_Hal.h */
void Mpu_Hal_GetRegionInfo(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, Mpu_RegionParamsType *RegionParamsPtr) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
#if MPU_HAS_PROCESS_IDENTIFIER
    uint32 RegionPid;
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
#endif
    uint32 RegionAttrs;
    DEVICE_ASSERT(RegionParamsPtr != NULL_PTR);
    DEVICE_ASSERT(MpuId < MPU_ID_MAX);
    DEVICE_ASSERT(RegionId < MPU_REGION_ID_MAX);

    /* PRQA S 2812 ++ # AMCE_MCL_2812*/
    RegionParamsPtr->Addr.StartAddr = Mpu_Reg_GetRegionStartAddr((uint8)MpuId, (uint8)RegionId);
    /* PRQA S 2812 -- # AMCE_MCL_2812*/
    RegionParamsPtr->Addr.EndAddr = Mpu_Reg_GetRegionEndAddr((uint8)MpuId, (uint8)RegionId);
    RegionAttrs = Mpu_Reg_GetRegionAttrs((uint8)MpuId, (uint8)RegionId);
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
#if MPU_HAS_PROCESS_IDENTIFIER
    RegionPid = Mpu_Reg_GetRegionPid((uint8)MpuId, (uint8)RegionId);
    RegionParamsPtr->Attrs.PidMask = (uint8)((RegionPid & MPU_RGD0_WORD3_PIDMASK_Msk) >> MPU_RGD0_WORD3_PIDMASK_Pos);
    RegionParamsPtr->Attrs.Pid = (uint8)((RegionPid & MPU_RGD0_WORD3_PID_Msk) >> MPU_RGD0_WORD3_PID_Pos);
    RegionParamsPtr->Attrs.MasterPidEnable = RegionAttrs & MPU_MASTER_PID_ENABLE_MASK;
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
#endif
    RegionParamsPtr->Attrs.Permission = RegionAttrs & MPU_PERMISSION_MASK;
}

/* Implementation of Mpu_Hal_EnableRegion declared in Mpu_Hal.h */
void Mpu_Hal_EnableRegion(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, boolean Enable) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
    DEVICE_ASSERT(MpuId < MPU_ID_MAX);
    DEVICE_ASSERT(RegionId < MPU_REGION_ID_MAX);

    Mpu_Reg_Disable((uint8)MpuId);
    Mpu_Reg_EnableRegion((uint8)MpuId, (uint8)RegionId, Enable);
    Mpu_Reg_Enable((uint8)MpuId);
}

#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
#if MPU_HAS_PROCESS_IDENTIFIER
/* Implementation of Mpu_Hal_SetMasterPid declared in Mpu_Hal.h */
void Mpu_Hal_SetMasterPid(Mpu_IdType MpuId, Mpu_MasterType Master, uint8 Pid)
{
    DEVICE_ASSERT(MpuId < MPU_ID_MAX);
    DEVICE_ASSERT(Master < MPU_MASTER_MAX);

    Mpu_Reg_SetMasterPid((uint8)MpuId, (uint8)Master, Pid);
}
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
#endif

/* Implementation of Mpu_Hal_SetRegionAttrs declared in Mpu_Hal.h */
void Mpu_Hal_SetRegionAttrs(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, const Mpu_RegionAttrsType *AttrsPtr) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
    DEVICE_ASSERT(MpuId < MPU_ID_MAX);
    DEVICE_ASSERT(RegionId < MPU_REGION_ID_MAX);
    DEVICE_ASSERT(AttrsPtr != NULL_PTR);
    if (AttrsPtr != NULL_PTR)
    {
        Mpu_Reg_EnableRegion((uint8)MpuId, (uint8)RegionId, FALSE);
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
#if MPU_HAS_PROCESS_IDENTIFIER
        Mpu_Reg_SetRegionAttrs((uint8)MpuId, (uint8)RegionId, (AttrsPtr->Permission | AttrsPtr->MasterPidEnable));
        Mpu_Reg_SetRegionPid((uint8)MpuId, (uint8)RegionId, (uint8)AttrsPtr->Pid, (uint8)AttrsPtr->PidMask);
#else
        Mpu_Reg_SetRegionAttrs((uint8)MpuId, (uint8)RegionId, AttrsPtr->Permission);
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
#elif defined (AC7840E)
        Mpu_Reg_SetRegionAttrs((uint8)MpuId, (uint8)RegionId, AttrsPtr->Permission);
#endif
        Mpu_Reg_EnableRegion((uint8)MpuId, (uint8)RegionId, TRUE);
    }
}

/**
 * @brief Convert MPU slave id to MPU module id.
 * @details
 * Maps the slave port id to the corresponding MPU module id.
 * On AC7843X devices, slaves PFLASH, DFLASH use MPU_ID_1,
 * while other slaves use MPU_ID_0.
 *
 * @param[in] Slave: Slave port id
 * @return Mpu_IdType: Corresponding MPU module id
 *
 * @note
 *     DES_MCL_VAR_202
 */
LOCAL_INLINE Mpu_IdType Mpu_Hal_SlaveToMpu(Mpu_SlaveType Slave)
{
    Mpu_IdType MpuId = MPU_ID_0;
#if defined(AC7843X)
    /* calculate MPU 1 slave idx */
    if (Slave > MPU_SLAVE_SRAM_U)
    {
        MpuId = MPU_ID_1;
    }
#else
    (void)Slave;
#endif /* AC7843X */

    return MpuId;
}

/**
 * @brief Convert MPU slave id to register index.
 * @details
 * Converts the slave port id to the corresponding register index.
 * On AC7843X devices, slaves with id > MPU_SLAVE_SRAM_U are adjusted
 * by subtracting 2 from their value.
 *
 * @param[in] Slave: Slave port id
 * @return uint8: Register index for the slave
 *
 * @note
 *     DES_MCL_VAR_203
 */
LOCAL_INLINE uint8 Mpu_Hal_SlaveToRegIdx(Mpu_SlaveType Slave)
{
    uint8 SlaveRegIdx = (uint8)Slave;

#if defined(AC7843X)
    /* calculate MPU 1 slave idx */
    if (Slave > MPU_SLAVE_SRAM_U)
    {
        SlaveRegIdx = (uint8)Slave - 2U;
    }
#endif /* AC7843X */

    return SlaveRegIdx;
}

/* Implementation of Mpu_Hal_GetErrorSlaveId declared in Mpu_Hal.h */
Hal_StatusType Mpu_Hal_GetErrorSlaveId(Mpu_SlaveType *Slave) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
    uint32 Status;
    uint8 MpuIdx;
    Hal_StatusType ReturnValue = STATUS_SUCCESS; /* function return value */
    *Slave = MPU_SLAVE_MAX;

#if defined(AC7840X) || defined(AC7842X) || defined(AC7840E)
    MpuIdx = (uint8)MPU_ID_0;
    Status = Mpu_Reg_GetStatus(MpuIdx);
    /* MPU_SLAVE_FLASH occured error */
    if ((Status & MPU_CESR_SPERR0_Msk) != 0U)
    {
        *Slave = MPU_SLAVE_FLASH;
    }
    /* MPU_SLAVE_SRAM_L occured error */
    else if ((Status & MPU_CESR_SPERR1_Msk) != 0U)
    {
        *Slave = MPU_SLAVE_SRAM_L;
    }
    /* MPU_SLAVE_SRAM_U occured error */
    else if ((Status & MPU_CESR_SPERR2_Msk) != 0U)
    {
        *Slave = MPU_SLAVE_SRAM_U;
    }
    else
    {
        /*  nothing */
    }
#elif defined(AC7843X) /* AC7840X AC7842X */
    for (MpuIdx = (uint8)MPU_ID_0; MpuIdx < (uint8)MPU_ID_MAX; MpuIdx++)
    {
        Status = Mpu_Reg_GetStatus((uint8)MpuIdx);
        /* MPU slave 0 occured error */
        if ((Status & MPU_CESR_SPERR1_Msk) != 0U)
        {
            /* MPU 0 slave idx */
            if ((uint8)MPU_ID_0 == MpuIdx)
            {
                *Slave = MPU_SLAVE_SRAM_L;
            }
            else /* MPU 1 slave idx */
            {
                *Slave = MPU_SLAVE_PFLASH;
            }
        }
        /* MPU slave 1 occured error */
        else if ((Status & MPU_CESR_SPERR2_Msk) != 0U)
        {
            /* MPU 0 slave idx */
            if ((uint8)MPU_ID_0 == MpuIdx)
            {
                *Slave = MPU_SLAVE_SRAM_U;
            }
            else /* MPU 1 slave idx */
            {
                *Slave = MPU_SLAVE_DFLASH;
            }
        }
        else
        {
            /*  nothing */
        }
        if (MPU_SLAVE_MAX != *Slave)
        {
            break;
        }
    }
#endif                 /* AC7843X */
    /* mpu no error */
    if (MPU_SLAVE_MAX != *Slave)
    {
        ReturnValue = STATUS_ERROR;
    }

    return ReturnValue;
}

/* Implementation of Mpu_Hal_GetSlaveErrorInfo declared in Mpu_Hal.h */
Hal_StatusType Mpu_Hal_GetSlaveErrorInfo(Mpu_SlaveType Slave, Mpu_ErrorInfoType *ErrInfoPtr) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
    Mpu_IdType MpuId;
    uint32 ErrorInfoal;
    uint8 SlaveRegIdx;
    Hal_StatusType ReturnValue = STATUS_SUCCESS; /* function return value */

    DEVICE_ASSERT(ErrInfoPtr != NULL_PTR);
    DEVICE_ASSERT(Slave < MPU_SLAVE_MAX);
    MpuId = Mpu_Hal_SlaveToMpu(Slave);
    SlaveRegIdx = Mpu_Hal_SlaveToRegIdx(Slave);
    ErrorInfoal = Mpu_Reg_GetErrorInfo((uint8)MpuId, SlaveRegIdx);
    /* mpu occured error */
    if ((ErrorInfoal & MPU_EDR2_EFLG_Msk) != 0U)
    {
        /* PRQA S 2812 ++ # AMCE_MCL_2812*/
        ErrInfoPtr->Addr = Mpu_Reg_GetErrorAddr((uint8)MpuId, SlaveRegIdx);
        /* PRQA S 2812 -- # AMCE_MCL_2812*/
        ErrInfoPtr->ErrorInfoal = ErrorInfoal;
        ReturnValue = STATUS_ERROR;
    }

    return ReturnValue;
}

/* Implementation of Mpu_Hal_ClearSlaveError declared in Mpu_Hal.h */
void Mpu_Hal_ClearSlaveError(Mpu_SlaveType Slave) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
    uint32 Status;
    Mpu_IdType MpuId;

    DEVICE_ASSERT(Slave < MPU_SLAVE_MAX);
    MpuId = Mpu_Hal_SlaveToMpu(Slave);
    Mpu_Reg_ClearErrorInfo((uint8)MpuId, (uint8)Slave);

    /*clear interrupt status*/
    Status = Mpu_Reg_GetStatus((uint8)MpuId);
    Mpu_Reg_ClearStatus((uint8)MpuId, Status);
}
