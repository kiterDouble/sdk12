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

#ifndef EEP_DRV_H
#define EEP_DRV_H
/*!
* @file eep_drv.h
* @brief This file provides EEP integration functions interface.
*
*/
#ifdef __cplusplus
extern "C" {
#endif
/* ===========================================  Includes  =========================================== */
#include "Device_Register.h"

#if defined(CONFIG_EEP_DRV_ENABLE) && defined(AC7840X)
/* ============================================  Define  ============================================ */
#define EEP_DRV_SW_MAJOR_VERSION             (2U)
#define EEP_DRV_SW_MINOR_VERSION             (1U)
#define EEP_DRV_SW_PATCH_VERSION             (9U)

/** @brief EEP min size in byte */
#define EEP_MIN_SIZE                    (512U)

/** @brief EEP max size in byte */
#define EEP_MAX_SIZE                    (4096U)

/* ===========================================  Typedef  ============================================ */
/*!< Callback typedef for eep module */
typedef void (*Eep_CallbackType)(void);

/*!
 * @brief IC type code.
 */
typedef enum
{
    EEP_AC78406 = 406U,
    EEP_AC78036 = 36U,
} eep_ic_type_t;

/*!
 * @brief EEP operate status.
 */
typedef enum
{
    EEP_SUCCESS         = 0U,   /*!< EEP operate successfully */
    EEP_STATUS_ERROR    = 1U,   /*!< EEP operate status error which can check FLASH status register */
    EEP_PARA_ERROR      = 2U,   /*!< EEP input parameter error */
    EEP_TIMEOUT         = 3U,   /*!< EEP operate timeout when unlock FLASH */
    EEP_BUSY            = 4U,   /*!< EEP operate busy when FLASH command not finished */
    EEP_OTHER_ERROR     = 40U,  /*!< EEP other error, normally this will not occur */
} eep_status_t;

/*!
 * @brief EEP configuration structure.
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

typedef struct
{
    uint32 FlashPageSize;     /*!< The FLASH page size (byte) */
    uint32 FlashMinProByte;   /*!< The FLASH min program byte */
    eep_ic_type_t icType;     /*!< The IC type */
}eep_config_t;

/*!
 * @brief EEP operate status.
 */
typedef enum
{
    EEP_HAL_UNINIT = 0x00U, /*!< EEP operate not initial. */
    EEP_HAL_INITIALIZED = 0x01U, /*!< EEP operate initialized. */
    EEP_HAL_IDLE = 0x02U, /*!< EEP operate idle when FLASH command has been finished. */
    EEP_HAL_TIMEOUT = 0x03U, /*!< EEP operate timeout when FLASH command excute timeout. */
    EEP_HAL_BUSY = 0x04U, /*!< EEP operate busy when FLASH command not finished. */
} Eep_Hal_StatusType;

/*!
 * @brief EEP version structure.
 */
typedef struct
{
    uint32 Major;
    uint32 Minor;
    uint32 Patch;
} Eep_Hal_VersionType;

/* ====================================  Functions declaration  ===================================== */
/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/*!
 * @brief EEP initialize
 * @note  Function ID: [DES_EEP_API_200]
 * @param [in] ConfigPtr: EEP parameter struct
 * @return EEP operate status
 */
Hal_StatusType Eep_Hal_Init(const Eep_Hal_ConfigType *config);

/*!
 * @brief EEP Write
 * @note  Function ID: [DES_EEP_API_202]
 * @param [in] Addr: EEP address
 * @param [in] Data: data buffer pointer
 * @param [in] Size: the size of write data(uint is byte)
 * @return EEP operate status
 */
Hal_StatusType Eep_Hal_Write(uint32 Addr, const uint8 *Data, uint16 Size);

/*!
 * @brief EEP read
 * @note  Function ID: [DES_EEP_API_201]
 * @param [in] Addr: EEP read address
 * @param [out] Data: Point to the buffer ready to read the data from EEP
 * @param [in] Size: the number of read data(uint is byte)
 * @return EEP operate status
 */
Hal_StatusType Eep_Hal_Read(uint32 Addr, uint8 *Data, uint16 Size);

/*!
 * @brief Erase all EEP space
 * @note  Function ID: [DES_EEP_API_203]
 * @return EEP operate status
 */
Hal_StatusType Eep_Hal_Erase(void);

/*!
 * @brief Reload flash data to EEP
 * @note  Function ID: [DES_EEP_API_205]
 * @return EEP operate status
 */
Hal_StatusType Eep_Hal_Refresh(void);

/*!
 * @brief Get EEP current status
 * @note  Function ID: [DES_EEP_API_204]
 * @return EEP current status busy or idle
 */
Eep_Hal_StatusType Eep_Hal_GetStatus(void);

/*!
 * @brief Get EEP sdk version
 * @param [in] Version: EEP version struct
 * @return void
 */
void EEP_Hal_GetVersion(Eep_Hal_VersionType *Version);

/*!
 * @brief Release the acquired resource when Eep_Hal is aborted abnormally.
 * @note Function ID: DES_EEP_API_207
 */
inline void Eep_Hal_AbortFreeRes(void) {}

#endif

#ifdef __cplusplus
}
#endif
#endif /* _EEP_DRV_H */

/* =============================================  EOF  ============================================== */
