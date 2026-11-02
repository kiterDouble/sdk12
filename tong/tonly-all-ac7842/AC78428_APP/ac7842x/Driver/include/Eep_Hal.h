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

/**
 * @file Eep_Hal.h
 *
 * @brief This file provides EEP API definitions and types.
 *
 * @details This header file contains the API declarations, data types, and constants
 *          for the EEPROM Hardware Abstraction Layer (HAL). It implements EEPROM
 *          emulation using flash memory and RAM mirroring techniques.
 */

#ifndef EEP_HAL_H
#define EEP_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Types.h"
#include "Conf_AC784xx.h"

#if defined(AUTOSAR_MCAL)
#include "Eep_Cfg.h"
#if (EEP_NOT_HAL_LIB == STD_ON)
#define CONFIG_EEP_DRV_ENABLE
#endif
#endif

#if defined(CONFIG_EEP_DRV_ENABLE) && defined(AC7840X)
#include "eep_drv.h"
#else

/* ============================================  DEFINES AND MACROS  ============================================ */
/**
 * @def EEP_MIN_SIZE
 * @brief EEP minimum size in byte
 * @note DES ID: DES_EEP_MACRO_001
 */
#define EEP_MIN_SIZE                    (512U)

#ifndef CONFIG_EEP_MAX_SIZE_KB
#define CONFIG_EEP_MAX_SIZE_KB          (4U)
#endif

/**
 * @def EEP_MAX_SIZE
 * @brief EEP maximum size in byte, derived from CONFIG_EEP_MAX_SIZE_KB
 * @note DES ID: DES_EEP_MACRO_002
 */

/**
 * @def EEP_BLOCK_MAX_NUM
 * @brief EEP block maximum number, derived from CONFIG_EEP_MAX_SIZE_KB
 * @note DES ID: DES_EEP_MACRO_003
 */

#if (CONFIG_EEP_MAX_SIZE_KB == 8U)
#define EEP_MAX_SIZE                    (8192U)
#define EEP_BLOCK_MAX_NUM               (16U)
#elif (CONFIG_EEP_MAX_SIZE_KB == 6U)
#define EEP_MAX_SIZE                    (6144U)
#define EEP_BLOCK_MAX_NUM               (12U)
#elif (CONFIG_EEP_MAX_SIZE_KB == 4U)
#define EEP_MAX_SIZE                    (4096U)
#define EEP_BLOCK_MAX_NUM               (8U)
#else
#error "Unsupported CONFIG_EEP_MAX_SIZE_KB: must be 4, 6, or 8"
#endif

/**
 * @def EEP_GROUP_MAX_NUM
 * @brief EEP group maximum number
 * @note DES ID: DES_EEP_MACRO_004
 */
#define EEP_GROUP_MAX_NUM               (4U)

/**
 * @def EEP_FLASH_MAX_UNIT_SIZE
 * @brief EEP flash maximum program unit size
 * @note DES ID: DES_EEP_MACRO_005
 */
#define EEP_FLASH_MAX_UNIT_SIZE         (16U)

/*!
 * @def EEP_MPU_PROTECTION
 * @brief Macro to enable MPU (Memory Protection Unit) protection for EEPROM data.
 *
 * @details This macro is defined based on the following conditions:
 *          - When using AUTOSAR MCAL, MPU protection is enabled only if EEP_MPU_PROTECTION_ENABLE is set to STD_ON
 *          - When not using AUTOSAR MCAL, MPU protection is always enabled
 *
 * @note MPU protection ensures data integrity during EEPROM operations by controlling
 *       memory access permissions to EEPROM-related memory regions.
 * @note DES ID: DES_EEP_MACRO_010
 */
#if defined(AUTOSAR_MCAL)
#if (EEP_MPU_PROTECTION_ENABLE == STD_ON)
#ifndef EEP_MPU_PROTECTION
#define EEP_MPU_PROTECTION
#endif
#endif
#endif

/* ============================================= TYPEDEFS ================================================ */
/*!
* @brief Callback type for when current block have not enough buffer to storage.
*/
typedef void (*Eep_CallbackType)(void);

/*!
* @brief EEP operate status.
* @note DES ID: DES_EEP_HAL_TYP_001
*/
typedef enum
{
    EEP_HAL_UNINIT = 0x00U, /*!< EEP operate not initial. */
    EEP_HAL_INITIALIZED = 0x01U, /*!< EEP operate initialized. */
    EEP_HAL_IDLE = 0x02U, /*!< EEP operate idle when FLASH command has been finished. */
    EEP_HAL_TIMEOUT = 0x03U, /*!< EEP operate timeout when FLASH command excute timeout. */
    EEP_HAL_BUSY = 0x04U, /*!< EEP operate busy when FLASH command not finished. */
} Eep_Hal_StatusType;

/* PRQA S 3630 ++ # AMCE_QAC_3630*/
/*!
* @brief EEP configuration structure.
* @note DES ID: DES_EEP_HAL_TYP_002
*/
typedef struct
{
    uint16 EepSize; /*!< The total size of EEP (byte), range: 512, 1024, 2048, 4096.  */
    uint16 EepBlockSize; /*!< The block size of EEP (byte), range: 1~1023, recommend: 512. */
    uint16 EepGroupNum; /*!< The backup number of EEP, range: 2~4, recommend: 4.          */
    uint32 EepRamBaseAddr; /*!< The base address of RAM used for EEP emulation.              */
    uint32 EepFlashBaseAddr; /*!< The base address of FLASH used for EEP emulation.            */
    Eep_CallbackType Callback; /*!< Callback when current block have not enough buffer to storage. */
} Eep_Hal_ConfigType;

/*!
* @brief EEP version structure.
* @note DES ID: DES_EEP_HAL_TYP_003
*/
typedef struct
{
    uint32 Major; /*!< Major version number */
    uint32 Minor; /*!< Minor version number */
    uint32 Patch; /*!< Patch version number */
} Eep_Hal_VersionType;

/**
 * @brief Group information structure
 *
 * Holds information for a single group, including ID, start address,
 * and remaining writable size.
 * @note DES ID: DES_EEP_HAL_TYP_004
 */
typedef struct{
    uint8  GroupId;       /**< Group ID */
    uint32 StartAddr;     /**< Group start address */
    uint32 RemainSize;    /**< Remaining writable size in bytes */
} Eep_Hal_GroupInfo;

/**
 * @brief Group table structure
 *
 * Holds information for all groups, as well as the currently active group.
 * @note DES ID: DES_EEP_HAL_TYP_005
 */
typedef struct{
    uint8  GroupNum;      /**< Total number of groups */
    uint32 GroupSize;     /**< Size of each group in bytes */
    Eep_Hal_GroupInfo Groups[EEP_BLOCK_MAX_NUM]; /**< Group information array */
} Eep_Hal_GroupTable;
/* PRQA S 3630 -- # AMCE_QAC_3630*/

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/*!
* @brief Initialize EEP driver
* @param[in] ConfigPtr Pointer to configuration structure
* @return Initialization status
* @note Function ID: DES_EEP_API_200
* @warning Configuration must persist after initialization
* @remark Performs:
* 1. Parameter validation
* 2. Memory mapping setup
* 3. Initial data synchronization
*/
Hal_StatusType Eep_Hal_Init(const Eep_Hal_ConfigType *ConfigPtr);

/*!
* @brief Main EEP write interface
* @param[in] Addr Virtual EEP address (offset within configured space)
* @param[in] Data Pointer to source data buffer
* @param[in] Size Data length in bytes
* @return Operation status (STATUS_SUCCESS/STATUS_ERROR)
* @note Function ID: DES_EEP_API_202
* @warning Address range must be within configured EEP size
* @remark Implements:
* 1. Address validation
* 2. Block-wise write operations
* 3. Status tracking
*/
Hal_StatusType Eep_Hal_Write(uint32 Addr, const uint8 *Data, uint16 Size);

/*!
* @brief Main EEP read interface
* @param[in] Addr Virtual EEP address
* @param[out] Data Data buffer pointer
* @param[in] Size Data length in bytes
* @return Operation status
* @note Function ID: DES_EEP_API_201
* @remark Direct memory read from RAM mirror
*/
Hal_StatusType Eep_Hal_Read(uint32 Addr, uint8 *Data, uint16 Size);

/*!
* @brief Erase Flash page with verification
* @param[in] pageAddress Start address of Flash page to erase
* @return Operation status (STATUS_SUCCESS/STATUS_ERROR)
* @note Function ID: DES_EEP_API_207
* @warning Must be executed within flash operation critical section
* @remark Operation sequence:
* 1. Verify page erase status
* 2. Perform erase if verification fails
* 3. Re-verify after erase operation
*/
Hal_StatusType Eep_Hal_Erase(void);

#ifndef EEP_SDK_NON_EXTENDED_API
/*!
* @brief Refresh EEP data from flash
* @return Operation status
* @note Function ID: DES_EEP_API_205
* @remark Internal mechanism for:
* 1. Data consistency recovery
* 2. Post-erase reinitialization
* 3. Multi-block synchronization
*/
Hal_StatusType Eep_Hal_Refresh(void);
#endif

/*!
* @brief Retrieve EEP driver status
* @return Current driver state
* @note Function ID: DES_EEP_API_204
* @retval EEP_HAL_UNINIT Driver not initialized
* @retval EEP_HAL_INITIALIZED Initialized but inactive
* @retval EEP_HAL_BUSY Write operation in progress
* @retval EEP_HAL_IDLE Ready for operations
*/
Eep_Hal_StatusType Eep_Hal_GetStatus(void);

/*!
* @brief Release the acquired resource when Eep_Hal is aborted abnormally.
* @note Function ID: DES_EEP_API_208
*/
void Eep_Hal_AbortFreeRes(void);

/*!
* @brief Get driver version information
* @param[out] Version Pointer to version structure
* @note Function ID: DES_EEP_API_206
* @remark Populates:
* - Major version
* - Minor version
* - Patch version
*/
void EEP_Hal_GetVersion(Eep_Hal_VersionType *Version);

/**
 * @brief Force switch to a specified group
 *
 * Forcefully switches to the group specified by the group mask.
 *
 * @param[in] EepGroupMask Target group mask (use 1U << groupId to generate)
 * @return STATUS_SUCCESS on success
 * @return OTHER_ERROR if the mask is invalid or multiple groups are set
 * @note Function ID: DES_EEP_API_221
 */
Hal_StatusType Eep_Hal_ForceSwitchGroup(uint32 EepGroupMask);

/**
 * @brief Get the remaining writable length of the active group
 *
 * Retrieves the remaining writable length of the currently active group.
 *
 * @param[out] EepGroupTable Pointer to the group table
 * @return STATUS_SUCCESS on success
 * @note Function ID: DES_EEP_API_222
 */
Hal_StatusType Eep_Hal_GetGroupFreeLength(Eep_Hal_GroupTable *EepGroupTable);

#endif

#ifdef __cplusplus
}
#endif

#endif /* EEP_HAL_H */

/* =============================================  EOF  ============================================== */
