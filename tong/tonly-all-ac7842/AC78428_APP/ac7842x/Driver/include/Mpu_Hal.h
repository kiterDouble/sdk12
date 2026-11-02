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
 * @file Mpu_Hal.h
 *
 * @brief This file provides extern Hal Mpu api.
 *
 */

#ifndef MPU_HAL_H
#define MPU_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Mpu_Reg.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*!
 * @brief MPU Region range and attribute.
 */
typedef struct
{
    uint32 StartAddr; /*!< region start address */
    uint32 EndAddr; /*!< region end address */
} Mpu_RegionAddrConfigType;

/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/*!
 * @brief MPU Error Information structure.
 */
typedef struct
{
    uint32 Addr; /*!< error address */
    uint32 ErrorInfoal; /*!< error information */
} Mpu_ErrorInfoType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/

/*!
 * @brief MPU region attributes.
 */
typedef struct
{
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
#if MPU_HAS_PROCESS_IDENTIFIER
    uint32 MasterPidEnable;
    uint32 Pid; /*!< region pid */
    uint32 PidMask; /*!< region pid mask */
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
#endif
    uint32 Permission; /*!< region permission */
} Mpu_RegionAttrsType;

/*!
 * @brief MPU Region range and attribute.
 */
typedef struct
{
    Mpu_RegionAddrConfigType Addr;
    Mpu_RegionAttrsType Attrs; /*!< region attributes pointer */
} Mpu_RegionParamsType;

/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/*!
 * @brief MPU Region range and attribute.
 */
typedef struct
{
    Mpu_RegionIdType RegionId; /**< Mpu Region num*/
    Mpu_RegionParamsType RegionParams; /*!< region attributes pointer */
} Mpu_RegionConfigType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/

/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/*!
 * @brief MPU configuration structure
 */
typedef struct
{
    uint8 RegionConfigCnt; /*!< configuration region count */
    Mpu_IdType MpuId; /*!< Mpu module id */
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
#if MPU_HAS_PROCESS_IDENTIFIER
    uint8 MasterPid[MPU_MASTER_MAX]; /*!< region pid */
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
#endif
    const Mpu_RegionConfigType *RegionConfig; /*!< mpu module configure pointer */
} Mpu_ModuleConfigType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/

/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/*!
 * @brief MPU Region range and attribute.
 */
typedef struct
{
    uint8 ModuleConfigCnt; /*!< configuration module count */
    const Mpu_ModuleConfigType *ModuleConfig; /*!< mpu module configure pointer */
} Mpu_ConfigType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/
/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief Mpu module init.
 * @note Function ID: DES_MCL_API_401
 * @param[in] ConfigPtr: mpu configuration
 * @return void
 */
void Mpu_Hal_Init(const Mpu_ConfigType *ConfigPtr);

/**
 * @brief Mpu module deinit.
 * @note  Function ID:DES_MCL_API_402
 * @return void
 */
void Mpu_Hal_Deinit(void);

/**
 * @brief set region address configuration.
 * @note Function ID: DES_MCL_API_403
 * @param[in] MpuId: mpu id
 * @param[in] RegionId: region id
 * @param[in] AddrConfigPtr: region address configuration pointer
 * @return void
 */
void Mpu_Hal_SetRegionAddr(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, const Mpu_RegionAddrConfigType *AddrConfigPtr);

/**
 * @brief set region attribution.
 * @note Function ID: DES_MCL_API_406
 * @param[in] MpuId: mpu id
 * @param[in] RegionId: region id
 * @param[in] AttrsPtr: region attribute configuration pointer
 * @return void
 */
void Mpu_Hal_SetRegionAttrs(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, const Mpu_RegionAttrsType *AttrsPtr);

/**
 * @brief get region info.
 * @note Function ID:DES_MCL_API_404
 * @param[in] MpuId: mpu id
 * @param[in] RegionId: region id
 * @param[out] RegionParamsPtr: region params pointer
 * @return void
 */
void Mpu_Hal_GetRegionInfo(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, Mpu_RegionParamsType *RegionParamsPtr);

/**
 * @brief enable/disbale region.
 * @note Function ID: DES_MCL_API_405
 * @param[in] MpuId: mpu id
 * @param[in] RegionId: region id
 * @param[in] Enable: enable/disable region configuration
 * @return void
 */
void Mpu_Hal_EnableRegion(Mpu_IdType MpuId, Mpu_RegionIdType RegionId, boolean Enable);

#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
#if MPU_HAS_PROCESS_IDENTIFIER
/**
 * @brief set Master Pid.
 * @note Function ID:DES_MCL_API_407
 * @param[in] MpuId: mpu id
 * @param[in] Master: master id
 * @param[in] Pid: region pid and master pid match,will hit
 * @return void
 */
void Mpu_Hal_SetMasterPid(Mpu_IdType MpuId, Mpu_MasterType Master, uint8 Pid);
#endif /* MPU_HAS_PROCESS_IDENTIFIER */
#endif

/**
 * @brief get slave error info.
 * @note Function ID: DES_MCL_API_408
 * @param[in] Slave: slave id
 * @param[in] ErrInfoPtr: Error info struction of Slave
 * @return Hal_StatusType: Is there an error, STATUS_SUCCESS stands for no error; STATUS_ERROR is a error
 */
Hal_StatusType Mpu_Hal_GetSlaveErrorInfo(Mpu_SlaveType Slave, Mpu_ErrorInfoType *ErrInfoPtr);

/**
 * @brief Clear Error status of Port .
 * @note Function ID: DES_MCL_API_409
 * @param[in] Slave: slave id
 * @return void
 */
void Mpu_Hal_ClearSlaveError(Mpu_SlaveType Slave);

/**
 * @brief get error slave id.
 * @note Function ID: DES_MCL_API_410
 * @param[out] Slave : slave id
 * @return Hal_StatusType: Is there an error, STATUS_SUCCESS stands for no error; STATUS_ERROR is a error
 */
Hal_StatusType Mpu_Hal_GetErrorSlaveId(Mpu_SlaveType *Slave);
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MPU_HAL_H */
/*============================================EOF===================================================*/
