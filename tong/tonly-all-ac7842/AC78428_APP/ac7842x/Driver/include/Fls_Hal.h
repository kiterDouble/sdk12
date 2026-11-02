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
*@file Fls_Hal.h
*
*@brief This file provides sdk flash api function
*
*/

#ifndef FLS_HAL_H
#define FLS_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/**
* @brief flash cse size
* @note DES_FLS_TYP_201
*/
typedef enum
{
    FLASH_CSESIZE_0BYTES,     /**< No CSE partition */
    FLASH_CSESIZE_128BYTES,   /**< 128 bytes CSE partition */
    FLASH_CSESIZE_256BYTES,   /**< 256 bytes CSE partition */
    FLASH_CSESIZE_512BYTES    /**< 512 bytes CSE partition */
} Flash_CseSize;

#if defined (AC7842X)
/**
* @brief dflash partition size
* @note DES_FLS_TYP_205
*/
typedef enum
{
    FLASH_DLFASHSIZE_0KB   = 0xFU,    /**< dflash partition 0KB   */
    FLASH_DLFASHSIZE_32KB  = 0xCU,    /**< dflash partition 32KB  */
    FLASH_DLFASHSIZE_64KB  = 0x8U,    /**< dflash partition 64KB  */
    FLASH_DLFASHSIZE_96KB  = 0x4U,    /**< dflash partition 96KB  */
    FLASH_DLFASHSIZE_128KB = 0x0U     /**< dflash partition 128KB */
} Flash_CsePartSize;
#endif /* AC7842X */

/**
* @brief flash cse information about size and sfe
* @note DES_FLS_TYP_202
*/
/*PRQA S 3630 ++ # AMCE_QAC_3630*/
typedef struct
{
    Flash_CseSize KeySize;       /**< CSE key size configuration */
#if defined (AC7842X)
    Flash_CsePartSize PartSize;  /**< CSE partition size configuration */
#endif /* AC7842X */
    boolean Sfe;                 /**< Secure Flash Enable flag */
} Flash_CseType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/**
 * @brief D-Flash and P-Flash interface compile control macros.
 * @note DES_FLS_VAR_007
 */
#define DFLASH_ENABLE (STD_ON)
#define PFLASH_ENABLE (STD_ON)

/*=========================================ENUM==================================================*/
/**
* @brief flash init configuration information
* @note DES_FLS_TYP_203
*/
/*PRQA S 3630 ++ # AMCE_QAC_3630*/
typedef struct
{
    uint32 Write_Timeout; /**< flash write working the longest time*/
    uint32 Erase_Timeout; /**< flash erase working the longest time*/
    boolean LVD_Enable;   /**< flash lvd dectect enable/disable*/
} Flash_Config;
/*PRQA S 3630 -- # AMCE_QAC_3630*/

/*=====================================GLOBAL VARIABLES==========================================*/

/*=====================================FUNCTION PROTOTYPES========================================*/

/**
* @brief This API set PARTITION SIZE.
* @note Function ID: [DES_FLS_API_206]
* @param [in] CseConfig: Cse configuration info about sfe and cse key size
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Partition set successfully
* @retval STATUS_ERROR: Error occurred during partition setting
* @pre The flash controller must be initialized
* @post The flash partition is configured according to the provided parameters
*/
Hal_StatusType Flash_Hal_SetPartition(const Flash_CseType *CseConfig);

/**
* @brief Initialize flash hardware
* @note Function ID: [DES_FLS_API_201]
* @param [in] Config: flash configure information
* @return void
* @pre None
* @post Flash hardware is initialized with the provided configuration
*/
void Flash_Hal_Init(const Flash_Config *Config);

#ifndef FLS_SDK_NON_EXTENDED_API
/**
* @brief erase write protect infomation flash area
* @note Function ID: [DES_FLS_API_212]
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Erase operation completed successfully
* @retval STATUS_ERROR: Error occurred during erase operation
* @pre Flash controller must be initialized
* @post Write protect information area is erased
*/
Hal_StatusType Flash_Hal_WPInfoErase(void);

/**
* @brief write flash area with OTP
* @note Function ID: [DES_FLS_API_222]
* @param [in] Offset: offset in one physical page
* @param [in] Length: Flash write length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Write operation completed successfully
* @retval STATUS_ERROR: Error occurred during write operation
* @pre Flash controller must be initialized and unlocked
* @post OTP area is programmed with the provided data
*/
Hal_StatusType Flash_Hal_WriteOtp
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
);

/**
* @brief get error status and clear
* @note Function ID: [DES_FLS_API_214]
* @param [out] Addr: output the ecc error address
* @return error status register value
* @retval Error status register value
* @pre Flash controller must be initialized
* @post Error status is cleared
*/
uint32 Flash_Hal_ErrorStatusGetAndClear(uint32 *Addr);

/**
* @brief verify all flash area
* @note Function ID: [DES_FLS_API_216]
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Verification successful
* @retval STATUS_ERROR: Verification failed
* @pre Flash controller must be initialized
* @post None
*/
Hal_StatusType Flash_Hal_AllVerify(void);

/**
* @brief disable/enable write protect information (need reboot work)
* @note Function ID: [DES_FLS_API_235]
* @param [in] En: disable/enable write protect configuration
* @param [in] Data: pflash & dflash write protect config value
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Write protect configuration set successfully
* @retval STATUS_ERROR: Error occurred during configuration
* @pre Flash controller must be initialized
* @post Write protect settings are applied
*/
Hal_StatusType Flash_Hal_WriteProtectSet(boolean En, const uint8 *Data);

/**
* @brief Flash erase the UID,secret key and lock areas.
* @note Function ID: [DES_FLS_API_236]
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Operation was successful
* @retval STATUS_ERROR: Operation failed
* @retval STATUS_BUSY: Operation was busy
* @pre Flash controller must be initialized and unlocked
* @post CSE information area is erased
*/
Hal_StatusType Flash_Hal_EraseCseInfo(void);

/**
* @brief FLASH program uid,secret key,lock.
* @note Function ID: [DES_FLS_API_237]
* @param [in] Addr: Program start address
* @param [in] Length: Program size in byte
* @param [in] Data: Pointer of data source address from which will be programmed
* @return operation status
*        - STATUS_SUCCESS:     Operation was successful.
*        - STATUS_ERROR:       Operation was failure.
*        - STATUS_BUSY:        Operation was busy.
* @pre Flash controller must be initialized and unlocked
* @post CSE information is programmed at the specified address
*/
Hal_StatusType Flash_Hal_ProgramCseInfo(uint32 Addr, uint32 Length, const uint8 *Data);

/**
* @brief FLASH program swap.
* @note Function ID: [DES_FLS_API_238]
* @param [in] En: swap config A/B
* @return operation status
*        - STATUS_SUCCESS:     Operation was successful.
*        - STATUS_ERROR:       Operation was failure.
*        - STATUS_BUSY:        Operation was busy.
* @pre Flash controller must be initialized
* @post Flash bank swap configuration is applied
*/
Hal_StatusType Flash_Hal_Swap(boolean En);

/**
* @brief Erase one page from given flash info
* @note Function ID: [DES_FLS_API_246]
* @param [in] Offset: offset in pFlash block
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Erase operation completed successfully
* @retval STATUS_ERROR: Error occurred during erase operation
* @pre Flash controller must be initialized and unlocked
* @post Flash info page at the specified offset is erased
*/
Hal_StatusType Flash_Hal_InfoErase(uint32 Offset);

#endif

/**
* @brief erase all flash area
* @note Function ID: [DES_FLS_API_213]
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Erase operation completed successfully
* @retval STATUS_ERROR: Error occurred during erase operation or CSE partition exists
* @pre Flash controller must be initialized and unlocked
* @post All flash areas except CSE partition are erased
*/
Hal_StatusType Flash_Hal_AllErase(void);

/**
* @brief enable read protect function
* @note Function ID: [DES_FLS_API_217]
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Read protection enabled successfully
* @retval STATUS_ERROR: Error occurred during operation
* @pre Flash controller must be initialized and unlocked
* @post Read protection is enabled
*/
Hal_StatusType Flash_Hal_ReadProtectSet(void);

/**
* @brief set backdoor key
* @note Function ID: [DES_FLS_API_233]
* @param [in] Data: backdoor key
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Backdoor key set successfully
* @retval STATUS_ERROR: Error occurred during operation
* @pre Flash controller must be initialized and unlocked
* @post Backdoor key is programmed
*/
Hal_StatusType Flash_Hal_BackdoorSet(const uint8 *Data);

/**
* @brief verify backdoor
* @note Function ID: [DES_FLS_API_234]
* @param [in] Data:  backdoor key value
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Backdoor key verification successful
* @retval STATUS_ERROR: Backdoor key verification failed
* @pre Flash controller must be initialized and unlocked
* @post None
*/
Hal_StatusType Flash_Hal_BackdoorVerify(const uint8 *Data);

/**
* @brief Read one or more bytes from given flash info
* @note Function ID: [DES_FLS_API_204]
* @param [in] Offset: offset in pFlash block
* @param [in] Length: Flash read length
* @param [out] DataPtr: source data buffer address
* @return void
* @pre Flash controller must be initialized
* @post Data is read from flash info area into the provided buffer
*/
void Flash_Hal_InfoRead
(
    uint32 Offset,
    uint32 Length,
    uint8 *DataPtr
);

/**
* @brief Write one or more bytes from given flash info
* @note Function ID: [DES_FLS_API_205]
* @param [in] Offset: offset in pFlash block
* @param [in] Length: Flash read length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Write operation completed successfully
* @retval STATUS_ERROR: Error occurred during write operation
* @pre Flash controller must be initialized and unlocked
* @post Data is written to flash info area from the provided buffer
*/
Hal_StatusType Flash_Hal_InfoWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
);

#if (PFLASH_ENABLE == STD_ON)
/**
* @brief erase pflash area with page erase
* @note Function ID: [DES_FLS_API_218]
* @param [in] Offset: offset in pFlash block
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Erase operation completed successfully
* @retval STATUS_ERROR: Error occurred during erase operation
* @pre Flash controller must be initialized and unlocked
* @post P-Flash page at the specified offset is erased
*/
Hal_StatusType PFlash_Hal_PageErase(uint32 Offset);

/**
* @brief write pflash area with page program
* @note Function ID: [DES_FLS_API_220]
* @param [in] Offset: offset in pFlash block
* @param [in] Length: Flash write length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Write operation completed successfully
* @retval STATUS_ERROR: Error occurred during write operation
* @pre Flash controller must be initialized and unlocked
* @post Data is programmed to P-Flash at the specified offset
*/
Hal_StatusType PFlash_Hal_PageWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
);

/**
* @brief verify pflash area with section verify
* @note Function ID: [DES_FLS_API_223]
* @param [in] Offset: offset in pFlash block
* @param [in] Length: Flash verify length
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Verification successful
* @retval STATUS_ERROR: Verification failed
* @pre Flash controller must be initialized
* @post None
*/
Hal_StatusType PFlash_Hal_SectionVerify
(
    uint32 Offset,
    uint32 Length
);

/**
* @brief Read one or more bytes from given pflash
* @note Function ID: [DES_FLS_API_245]
* @param [in] Offset: offset in pFlash block
* @param [in] Length: Flash read length
* @param [out] DataPtr: source data buffer address
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Read operation completed successfully
* @retval STATUS_ERROR: Error occurred during read operation
* @pre Flash controller must be initialized
* @post Data is read from P-Flash into the provided buffer
*/
Hal_StatusType PFlash_Hal_Read
(
    uint32 Offset,
    uint32 Length,
    uint8 *DataPtr
);

#ifndef FLS_SDK_NON_EXTENDED_API
/**
* @brief erase pflash area with block erase
* @note Function ID: [DES_FLS_API_219]
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Erase operation completed successfully
* @retval STATUS_ERROR: Error occurred during erase operation or CSE partition exists
* @pre Flash controller must be initialized and unlocked
* @post P-Flash block is erased
*/
Hal_StatusType PFlash_Hal_BlockErase(void);

/**
* @brief write pflash area with section program
* @note Function ID: [DES_FLS_API_221]
* @param [in] Offset: offset in pFlash block
* @param [in] Length: Flash write length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Write operation completed successfully
* @retval STATUS_ERROR: Error occurred during write operation
* @pre Flash controller must be initialized and unlocked
* @post Data is programmed to P-Flash section at the specified offset
*/
Hal_StatusType PFlash_Hal_SectionWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
);

/**
* @brief verify pflash area with block verify
* @note Function ID: [DES_FLS_API_224]
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Verification successful
* @retval STATUS_ERROR: Verification failed
* @pre Flash controller must be initialized
* @post None
*/
Hal_StatusType PFlash_Hal_BlockVerify(void);

#endif

#endif

#if (DFLASH_ENABLE == STD_ON)
/**
* @brief erase dflash area with page erase
* @note  Function ID : DES_FLS_API_225
* @param [in] Offset: offset in dFlash block
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Erase operation completed successfully
* @retval STATUS_ERROR: Error occurred during erase operation
* @retval STATUS_BUSY: Device is busy
* @pre Flash controller must be initialized and unlocked
* @post D-Flash page at the specified offset is erased
*/
Hal_StatusType DFlash_Hal_PageErase(uint32 Offset);

/**
* @brief write dflash area with page program
* @note Function ID: [DES_FLS_API_227]
* @param [in] Offset: offset in dFlash block
* @param [in] Length: Flash write length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Write operation completed successfully
* @retval STATUS_ERROR: Error occurred during write operation
* @retval STATUS_BUSY: Device is busy
* @retval STATUS_UNSUPPORTED: Parameters out of range
* @pre Flash controller must be initialized and unlocked
* @post Data is programmed to D-Flash at the specified offset
*/
Hal_StatusType DFlash_Hal_PageWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
);

/**
* @brief verify dflash area with section verify
* @note Function ID: [DES_FLS_API_229]
* @param [in] Offset: offset in dFlash block
* @param [in] Length: Flash verify length
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Verification successful
* @retval STATUS_ERROR: Verification failed
* @retval STATUS_BUSY: Device is busy
* @retval STATUS_UNSUPPORTED: Parameters out of range
* @pre Flash controller must be initialized
* @post None
*/
Hal_StatusType DFlash_Hal_SectionVerify
(
    uint32 Offset,
    uint32 Length
);

/**
* @brief Read one or more bytes from given dflash
* @note Function ID: [DES_FLS_API_231]
* @param [in] Offset: offset in dFlash block
* @param [in] Length: Flash read length
* @param [out] DataPtr: source data buffer address
* @return void
* @retval STATUS_SUCCESS: Read operation completed successfully
* @retval STATUS_ERROR: Error occurred during read operation
* @retval STATUS_BUSY: Device is busy
* @pre Flash controller must be initialized
* @post Data is read from D-Flash into the provided buffer
*/
Hal_StatusType DFlash_Hal_Read
(
    uint32 Offset,
    uint32 Length,
    uint8 *DataPtr
);

#ifndef FLS_SDK_NON_EXTENDED_API
/**
* @brief erase dflash area with block erase
* @note Function ID: [DES_FLS_API_226]
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Erase operation completed successfully
* @retval STATUS_ERROR: Error occurred during erase operation or CSE partition exists
* @retval STATUS_BUSY: Device is busy
* @pre Flash controller must be initialized and unlocked
* @post D-Flash block is erased
*/
Hal_StatusType DFlash_Hal_BlockErase(void);

/**
* @brief write dflash area with section program
* @note Function ID: [DES_FLS_API_228]
* @param [in] Offset: offset in dFlash block
* @param [in] Length: Flash write length
* @param [in] DataPtr: source data buffer address
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Write operation completed successfully
* @retval STATUS_ERROR: Error occurred during write operation
* @retval STATUS_BUSY: Device is busy
* @retval STATUS_UNSUPPORTED: Parameters out of range
* @pre Flash controller must be initialized and unlocked
* @post Data is programmed to D-Flash section at the specified offset
*/
Hal_StatusType DFlash_Hal_SectionWrite
(
    uint32 Offset,
    uint32 Length,
    const uint8 *DataPtr
);

/**
* @brief verify dflash area with block verify
* @note Function ID: [DES_FLS_API_230]
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Verification successful
* @retval STATUS_ERROR: Verification failed
* @retval STATUS_BUSY: Device is busy
* @pre Flash controller must be initialized
* @post None
*/
Hal_StatusType DFlash_Hal_BlockVerify(void);

#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* FLS_HAL_H */
