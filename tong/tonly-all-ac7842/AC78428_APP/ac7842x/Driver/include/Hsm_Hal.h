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

/**
 * @file Hsm_Hal.h
 *
 * @brief This file provides hsm integration functions interface.
 *
 */

#ifndef HSM_HAL_H
#define HSM_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================  Includes  =========================================== */
#if defined (AC7840E)
#include "AC7840e_Hsm_Reg.h"
#else
#include "eHSM_Types_Ip.h"
#include "eHSM_Mailbox_Prtcl_Ip.h"
#include "eHSM_Err_Code_Ip.h"
#include "eHSM_If_Evita_Ip.h"
#include "eHSM_If_Evita_Types_Ip.h"
#include "eHSM_If_Evita_ErrCode_Ip.h"

#include "eHSM_Mgr_Ctx_Ip.h"
#include "eHSM_Com_Struct_Ip.h"
#include "eHSM_If_Evita_Types_Ip.h"
#include "eHSM_Mailbox_Reg_Ip.h"
#include "eHSM_If_Ext_Ip.h"
#include "AC784xx_Hsm_Reg.h"
#include "Core_Hal.h"
#include "Hsm_FW_Version.h"
#endif
#include "Device_Register.h"
/*PRQA S 1536 EOF # AMCE_HSM_1536*/
/*PRQA S 1535 EOF # AMCE_HSM_1535*/
/*PRQA S 3630 EOF # AMCE_HSM_3630*/
/* ============================================  Define  ============================================ */
#if defined (AC7843X)
/**
 * @brief HSM operation success code.
 * @note DES ID: DES_HSM_MACRO_001
 */
#define HSM_SUCCESS  0xA55AU

/**
 * @brief Enable secure boot.
 * @note DES ID: DES_HSM_MACRO_002
 */
#define HSM_SECURE_BOOT_ENABLE                  (0x6EBA5B4BU)
/**
 * @brief Disable secure boot.
 * @note DES ID: DES_HSM_MACRO_003
 */
#define HSM_SECURE_BOOT_DISABLE                  (0U)
/**
 * @brief HSM disable code.
 * @note DES ID: DES_HSM_MACRO_004
 */
#define HSM_DISABLE                             (0xACC94D73U)

/**
 * @brief HSM unnormal mode value.
 * @note DES ID: DES_HSM_MACRO_005
 */
#define HSM_UNNORMAL_MODE                               0xA55AA55AU
/**
 * @brief HSM test mode value.
 * @note DES ID: DES_HSM_MACRO_006
 */
#define HSM_TEST_MODE                                   0xFFFFFFFFU
/**
 * @brief HSM develop mode value.
 * @note DES ID: DES_HSM_MACRO_007
 */
#define HSM_DEVELOP_MODE                                0xBD7E7BEBU
/**
 * @brief HSM manufacture mode value.
 * @note DES ID: DES_HSM_MACRO_008
 */
#define HSM_MANU_MODE                                   0xB93E5BE9U
/**
 * @brief HSM user mode value.
 * @note DES ID: DES_HSM_MACRO_009
 */
#define HSM_USER_MODE                                   0xA83E1369U
/**
 * @brief HSM debug mode value.
 * @note DES ID: DES_HSM_MACRO_010
 */
#define HSM_DEBUG_MODE                                  0x283A0321U
/**
 * @brief HSM destroy mode value.
 * @note DES ID: DES_HSM_MACRO_011
 */
#define HSM_DESTROY_MODE                                0x00000000U

/**
 * @brief Flash key slot length in bytes.
 * @note DES ID: DES_HSM_MACRO_012
 */
#define HSM_FLASH_KEY_SLOT_LEN     56u
/**
 * @brief Maximum number of flash key slots.
 * @note DES ID: DES_HSM_MACRO_013
 */
#define HSM_FLASH_KEY_MAX_SLOT_NUM (DFLASH_PAGE_SIZE / HSM_FLASH_KEY_SLOT_LEN)
/**
 * @brief Number of flash key pages.
 * @note DES ID: DES_HSM_MACRO_014
 */
#define HSM_FLASH_KEY_PAGE_NUM     2u

/**
 * @brief Flash page valid length in bytes.
 * @note DES ID: DES_HSM_MACRO_015
 */
#define HSM_FLASH_PAGE_VALID_LEN   8u
/**
 * @brief Number of flash pages.
 * @note DES ID: DES_HSM_MACRO_016
 */
#define HSM_FLASH_PAGE_NUM         2u
/**
 * @brief Flash page reserved length in bytes.
 * @note DES ID: DES_HSM_MACRO_017
 */
#define HSM_FLASH_PAGE_REVERSE_LEN \
    (DFLASH_PAGE_SIZE - (HSM_FLASH_KEY_SLOT_LEN * HSM_FLASH_KEY_MAX_SLOT_NUM) - HSM_FLASH_PAGE_VALID_LEN)

/**
 * @brief Page valid tag.
 * @note DES ID: DES_HSM_MACRO_018
 */
#define HSM_PAGE_VALID_TAG         (0xAAU)
/**
 * @brief Slot invalid tag.
 * @note DES ID: DES_HSM_MACRO_019
 */
#define HSM_SLOT_INVALID_TAG       (0xA5A5A5A5U)
/**
 * @brief Key data valid tag.
 * @note DES ID: DES_HSM_MACRO_020
 */
#define HSM_KEY_DATA_VALID_TAG     (0x52525252U)

/**
 * @brief Default key page number offset for flash key.
 * @note DES ID: DES_HSM_MACRO_021
 */
#ifndef HSM_FLASH_PAGE_NUM_OFFSET
#define HSM_FLASH_PAGE_NUM_OFFSET  (126U)
#endif
/**
 * @brief Flash key base address.
 * @note DES ID: DES_HSM_MACRO_022
 */
#define HSM_FLASH_KEY_BASE         0x01100000U + (HSM_FLASH_PAGE_NUM_OFFSET * DFLASH_PAGE_SIZE)
/**
 * @brief Flash key offset from DFLASH base.
 * @note DES ID: DES_HSM_MACRO_023
 */
#define HSM_FLASH_KEY_OFFSET       (HSM_FLASH_KEY_BASE - DFLASH_BASE)

/**
 * @brief Maximum number of RAM symmetric keys.
 * @note DES ID: DES_HSM_MACRO_024
 */
#define HSM_RAM_KEY_SYM_MAX_NUM  10u
/**
 * @brief Maximum number of RAM ECC keys.
 * @note DES ID: DES_HSM_MACRO_025
 */
#define HSM_RAM_KEY_ECC_MAX_NUM  5u
/**
 * @brief Maximum number of RAM RSA keys.
 * @note DES ID: DES_HSM_MACRO_026
 */
#define HSM_RAM_KEY_RSA_MAX_NUM  4u
/**
 * @brief Maximum number of RAM keys.
 * @note DES ID: DES_HSM_MACRO_027
 */
#define HSM_RAM_KEY_MAX_NUM (HSM_RAM_KEY_SYM_MAX_NUM + HSM_RAM_KEY_ECC_MAX_NUM + HSM_RAM_KEY_RSA_MAX_NUM)
/**
 * @brief RAM key memory size in bytes.
 * @note DES ID: DES_HSM_MACRO_028
 */
#define HSM_RAM_KEY_MEM_SIZE 800u

/**
 * @brief OTP key cipher AES cipher mask.
 * @note DES ID: DES_HSM_MACRO_029
 */
#define OTP_KEY_CIPHER_AES_CIPHER 0xFFFFDFFFU
/**
 * @brief OTP key cipher SM4 cipher mask.
 * @note DES ID: DES_HSM_MACRO_030
 */
#define OTP_KEY_CIPHER_SM4_CIPHER 0xFFFFCFFFU

/**
 * @brief OTP control code upgrade mask.
 * @note DES ID: DES_HSM_MACRO_031
 */
#define OTP_CTRL_CODE_UPGRADE_MASK  0x000000F0u
/**
 * @brief OTP control code upgrade position.
 * @note DES ID: DES_HSM_MACRO_032
 */
#define OTP_CTRL_CODE_UPGRADE_POS  4u
/**
 * @brief OTP control code verify mask.
 * @note DES ID: DES_HSM_MACRO_033
 */
#define OTP_CTRL_CODE_VERIFY_MASK  0x0000000Fu
/**
 * @brief OTP control code verify position.
 * @note DES ID: DES_HSM_MACRO_034
 */
#define OTP_CTRL_CODE_VERIFY_POS   0u

/**
 * @brief HSM OTP HSM control map address.
 * @note DES ID: DES_HSM_MACRO_035
 */
#define HSM_OTP_HSM_CTRL_MAP_ADDRESS    0x40000330U
/**
 * @brief HSM OTP host control map address.
 * @note DES ID: DES_HSM_MACRO_036
 */
#define HSM_OTP_HOST_CTRL_MAP_ADDRESS   0x40000334U
/* ===========================================  Typedef  ============================================ */
/**
 * @brief HSM random OTP key symmetric, SM2 private key, secp256R1 private key.
 * @note DES ID: DES_HSM_TYP_001
 */
typedef uint32 HSM_OtpKeyType;
#define HSM_RND_OTP_KEY_SYM             1
#define HSM_RND_OTP_KEY_SM2_PRV         2
#define HSM_RND_OTP_KEY_SECP256R1_PRV   4
/**
 * @brief Specifies the OTP key attribute.
 * @note DES ID: DES_HSM_TYP_002
 */
typedef uint32 HSM_OtpKeyAttrType;
#define ASYM_PUBLIC_KEY_ATTR   0U /**< Asymmetric public key */
#define ASYM_PRV_KEY_ATTR      1U /**< Asymmetric private key */
#define SYM_KEY_ATTR           2U /**< Random key is symmetric key */
#endif

#if defined (AC7840E)
typedef uint32 HSM_OtpKeyId;
#define HSM_CHIP_ROOT_KEY_ID                 0U
#define HSM_BL_VERIFY_KEY_ID                 1U
#define HSM_DEBUG_UNLOCK_KEY_ID              2U
#define HSM_CHIP_STATE_UNLOCK_KEY_ID         3U
#define HSM_BL_UNLOCK_KEY_ID                 4U
#define HSM_SECURITY_FW_UNLOCK_KEY_ID        5U
#define HSM_SYM_READ_UNLOCK_KEY_ID           6U
#define HSM_SYM_UPDATE_UNLOCK_KEY_ID         7U
#define HSM_ASYM_UPDATE_UNLOCK_KEY_ID        8U
#define HSM_SWAP_FLAG_UPDATE_UNLOCK_KEY_ID   9U

typedef uint32 HSM_KeyId;
#define HSM_FLASH_KEK_KEY        10U      /**< Flash KEK key */
#define HSM_FLASH_KEK_AUTH_KEY   11U      /**< Flash KEK auth code key */
#define HSM_FLASH_KEY_SYM_0      12U      /**< Flash symmetric key 0 */
#define HSM_FLASH_KEY_SYM_1      13U      /**< Flash symmetric key 1 */
#define HSM_FLASH_KEY_SYM_2      14U      /**< Flash symmetric key 2 */
#define HSM_FLASH_KEY_SYM_3      15U      /**< Flash symmetric key 3 */
#define HSM_FLASH_KEY_SYM_4      16U      /**< Flash symmetric key 4 */
#define HSM_FLASH_KEY_SYM_5      17U      /**< Flash symmetric key 5 */
#define HSM_FLASH_KEY_SYM_6      18U      /**< Flash symmetric key 6 */
#define HSM_FLASH_KEY_SYM_7      19U      /**< Flash symmetric key 7 */
#define HSM_FLASH_KEY_SYM_8      20U      /**< Flash symmetric key 8 */
#define HSM_FLASH_KEY_SYM_9      21U      /**< Flash symmetric key 9 */
#define HSM_RAM_KEY_SYM_0        22U      /**< RAM symmetric key 0 */
#define HSM_FLASH_KEY_RSA_0      30U      /**< Flash RSA key 0 */
#else
/**
 * @brief Specifies the OTP key index.
 * @note DES ID: DES_HSM_TYP_003
 */
/*PRQA S 1535 ++ # AMCE_HSM_1535 */
typedef uint32 HSM_OtpKeyId;
/*PRQA S 1535 -- # AMCE_HSM_1535 */
#define CHIP_ROOT_KEY              0U           /**< Chip root key */
#define DEVICE_ROOT_KEY            1U           /**< Device root key */
#define USER_ROOT_KEY              2U           /**< User root key */
#define HSM_DEBUG_KEY              3U           /**< HSM debug key */
#define HSM_FW_VERIFY_KEY          4U           /**< HSM firmware verify key */
#define HSM_ENCRYPT_KEY            5U           /**< HSM encrypt key */
#define HSM_UPGRADE_ENCRYPT_KEY    6U           /**< HSM upgrade encrypt key */
#define HSM_UPGRADE_VERIFY_KEY     7U           /**< HSM upgrade verify key */
#define HSM_PRIVATE_KEY            8U           /**< HSM private key */
#define HOST_DEBUG_KEY             9U           /**< Host debug key */
#define HOST_FW_VERIFY_KEY         10U          /**< Host firmware verify key */
#define HOST_ENCRYPT_KEY           11U          /**< Host encrypt key */
#define HOST_UPGRADE_ENCRYPT_KEY   12U          /**< Host upgrade encrypt key */
#define HOST_UPGRADE_VERIFY_KEY    13U          /**< Host upgrade verify key */
#define HOST_PRIVATE_KEY           14U          /**< Host private key */
#define HSM_SECRET_KEY             15U          /**< HSM secret key */
#define USER_AUTH_KEY              16U          /**< User authentication key */

/**
 * @brief Specifies the Flash and RAM key index for HSM.
 * @note DES ID: DES_HSM_TYP_004
 */
typedef uint32 HSM_KeyId;
#define HSM_FLASH_KEY_SYM_0      0U      /**< Flash symmetric key 0 */
#define HSM_FLASH_KEY_SYM_1      1U      /**< Flash symmetric key 1 */
#define HSM_FLASH_KEY_SYM_2      2U      /**< Flash symmetric key 2 */
#define HSM_FLASH_KEY_SYM_3      3U      /**< Flash symmetric key 3 */
#define HSM_FLASH_KEY_SYM_4      4U      /**< Flash symmetric key 4 */
#define HSM_FLASH_KEY_SYM_5      5U      /**< Flash symmetric key 5 */
#define HSM_FLASH_KEY_SYM_6      6U      /**< Flash symmetric key 6 */
#define HSM_FLASH_KEY_SYM_7      7U      /**< Flash symmetric key 7 */
#define HSM_FLASH_KEY_SYM_8      8U      /**< Flash symmetric key 8 */
#define HSM_FLASH_KEY_SYM_9      9U      /**< Flash symmetric key 9 */
#define HSM_FLASH_KEY_ECC_0      10U     /**< Flash ECC key 0 */
#define HSM_FLASH_KEY_ECC_1      11U     /**< Flash ECC key 1 */
#define HSM_FLASH_KEY_ECC_2      12U     /**< Flash ECC key 2 */
#define HSM_FLASH_KEY_ECC_3      13U     /**< Flash ECC key 3 */
#define HSM_FLASH_KEY_ECC_4      14U     /**< Flash ECC key 4 */
#define HSM_FLASH_KEY_ECC_5      15U     /**< Flash ECC key 5 */
#define HSM_FLASH_KEY_ECC_6      16U     /**< Flash ECC key 6 */
#define HSM_FLASH_KEY_ECC_7      17U     /**< Flash ECC key 7 */
#define HSM_FLASH_KEY_ECC_8      18U     /**< Flash ECC key 8 */
#define HSM_FLASH_KEY_ECC_9      19U     /**< Flash ECC key 9 */
#define HSM_FLASH_KEY_SM2_0      20U     /**< Flash SM2 key 0 */
#define HSM_FLASH_KEY_SM2_1      21U     /**< Flash SM2 key 1 */
#define HSM_FLASH_KEY_SM2_2      22U     /**< Flash SM2 key 2 */
#define HSM_FLASH_KEY_SM2_3      23U     /**< Flash SM2 key 3 */
#define HSM_FLASH_KEY_SM2_4      24U     /**< Flash SM2 key 4 */
#define HSM_FLASH_KEY_SM2_5      25U     /**< Flash SM2 key 5 */
#define HSM_FLASH_KEY_SM2_6      26U     /**< Flash SM2 key 6 */
#define HSM_FLASH_KEY_SM2_7      27U     /**< Flash SM2 key 7 */
#define HSM_FLASH_KEY_SM2_8      28U     /**< Flash SM2 key 8 */
#define HSM_FLASH_KEY_SM2_9      29U     /**< Flash SM2 key 9 */
#define HSM_FLASH_KEY_RSA_0      30U     /**< Flash RSA key 0 */
#define HSM_FLASH_KEY_RSA_1      31U     /**< Flash RSA key 1 */
#define HSM_FLASH_KEY_RSA_2      32U     /**< Flash RSA key 2 */
#define HSM_FLASH_KEY_RSA_3      33U     /**< Flash RSA key 3 */
#define HSM_RAM_KEY_SYM_0        34U     /**< RAM symmetric key 0 */
#define HSM_RAM_KEY_SYM_1        35U     /**< RAM symmetric key 1 */
#define HSM_RAM_KEY_SYM_2        36U     /**< RAM symmetric key 2 */
#define HSM_RAM_KEY_SYM_3        37U     /**< RAM symmetric key 3 */
#define HSM_RAM_KEY_SYM_4        38U     /**< RAM symmetric key 4 */
#define HSM_RAM_KEY_SYM_5        39U     /**< RAM symmetric key 5 */
#define HSM_RAM_KEY_SYM_6        40U     /**< RAM symmetric key 6 */
#define HSM_RAM_KEY_SYM_7        41U     /**< RAM symmetric key 7 */
#define HSM_RAM_KEY_SYM_8        42U     /**< RAM symmetric key 8 */
#define HSM_RAM_KEY_SYM_9        43U     /**< RAM symmetric key 9 */
#define HSM_RAM_KEY_ECC_0        44U     /**< RAM ECC key 0 */
#define HSM_RAM_KEY_ECC_1        45U     /**< RAM ECC key 1 */
#define HSM_RAM_KEY_ECC_2        46U     /**< RAM ECC key 2 */
#define HSM_RAM_KEY_ECC_3        47U     /**< RAM ECC key 3 */
#define HSM_RAM_KEY_ECC_4        48U     /**< RAM ECC key 4 */
#define HSM_RAM_KEY_SM2_0        49U     /**< RAM SM2 key 0 */
#define HSM_RAM_KEY_SM2_1        50U     /**< RAM SM2 key 1 */
#define HSM_RAM_KEY_SM2_2        51U     /**< RAM SM2 key 2 */
#define HSM_RAM_KEY_SM2_3        52U     /**< RAM SM2 key 3 */
#define HSM_RAM_KEY_SM2_4        53U     /**< RAM SM2 key 4 */
#define HSM_RAM_KEY_RSA_0        54U     /**< RAM RSA key 0 */
#define HSM_RAM_KEY_RSA_1        55U     /**< RAM RSA key 1 */
#define HSM_RAM_KEY_RSA_2        56U     /**< RAM RSA key 2 */
#define HSM_RAM_KEY_RSA_3        57U     /**< RAM RSA key 3 */

/**
 * @brief Specifies the secure boot information.
 * @note DES ID: DES_HSM_TYP_005
 */
typedef struct
{
    __IOM uint8 InitLSram;         /**< Enable initialization of lower SRAM */
    __IOM uint8 InitUSram;         /**< Enable initialization of upper SRAM */
    __IOM uint8 SwitchClock;       /**< Enable clock switching */
    __IOM uint8 LogEnable;         /**< Enable log output */
    __IOM uint8 StbWaitHSM;        /**< Standby wait for HSM */
    __IOM uint8 StbWaitVerify;     /**< Standby wait for verification */
    __IOM uint16 WdgTimeout;       /**< Watchdog timeout value */
    __IOM uint32 BootAddr;         /**< Boot address */
    __IOM uint32 VerifySize;       /**< Size of verification area */
    __IOM uint32 SignAddr;         /**< Signature address */
    __IOM uint32 PubKeyAddr;       /**< Public key address */
    __IOM uint32 HeaderAddr;       /**< Header address */
    __IOM uint32 HeaderSize;       /**< Header size */
    __IOM uint8 VersionUpdateEn;   /**< Enable version update */
    __IOM uint8 ResetDisable;      /**< Disable reset */
    __IOM uint8 PllFreq;           /**< PLL frequency setting */
    __IOM uint8 Reserve;          /**< Reserved */
    __IOM uint32 VersionAddr;      /**< Version address */
    __IOM uint32 CheckSum[2];      /**< CheckSum[0]:sum value, CheckSum[1]:default value is 0xFFFFFFFF*/
} HSM_BootCfgType;

/**
 * @brief Host image secure upgrade information.
 * @note DES ID: DES_HSM_TYP_006
 * @param[in] StorageAlg: 0: RSA2048 with SHA256, if support encryption, AES128 CBC is used.
        1: SM2 with SM3, if support encryption, SM4 CBC is used.
        2: AES128 CMAC, if support encryption, AES128 CBC is used.
        3: SM4 CMAC, if support encryption, SM4 CBC is used.
 * @param[in] UpgradeAlg: 0: RSA2048 with SHA256, if support encryption,AES128 CBC is used.
        1: SM2 with SM3, if support encryption, SM4 CBC is used.
        2: reserved.
        3: reserved.
        4: AES128 CMAC, if support encryption, AES128 CBC is used.
        5: SM4 CMAC, if support encryption, SM4 CBC is used.
 * @param[in] StorageEncryptionFlag: 0: The storage image doesn't need to be encrypted.
        1: The storage image needs to be encrypted.
 * @param[in] UpgradeDecryptionFlag: 0: The upgrade image is not encrypted.
        1: The upgrade image is encrypted.
 * @param[in] ProcessMode: 1: For INIT, input parameters will be checked.
        2: For UPDATE, this mode will deal with image data.
        4: For FINISH, this mode will check signature or generate new cmac signature.
        8: For ONEPASS, this mode the whole upgrade just work over by one command.
 * @param[in] CheckVersionFlag: 0: The upgrade doesn't need to check version counter.
        1: The upgrade needs to check version counter.
 * @param[in] UpgradeSignAddr: The signature of upgrade image.
 * @param[in] UpgradeSignSize: The signature size in bytes.
 * @param[in] UpgradeVersionAddr: : (Optional)The version of upgrade image.
 * @param[in] UpgradeVersionSize: The version size The version size.
 * @param[in] UpgradeImageAddr: The input encrypted (optional) upgrade image.
 * @param[in] UpgradeImageSize: The size of upgrade image in bytes.
 * @param[in] StorageImageAddr: The address for output encrypted (optional) image. If the upgrade image is not
        encrypted, and the output storage doesn't  need encryption. This can be NULL.
 * @param[in] StorageImageSize: The buffer size of output image in bytes.
 * @param[in] MacSignAddr: (Optional)The generated signature of output image. This is only valid when generating
              signature of output image with symmetric algorithm and process_mode is FINISH.
 * @param[in] MacSignSize: The size of generated signature of output image.
 * @param[in] UpgradePubkeyAddr: (Optional)The public key raw data.Only support SM2 and RSA2048. For SM2, the first
        byte '0x04' should be included. It is only valid for signature with asymmetric algorithm.
 * @param[in] UpgradePubkeySize: The size of public key in bytes.
 * @param[in] UpgradeIvAddr: (Optional)Iv for upgrading image decryption.
 * @param[in] UpgradeIvSize: The size of upgrade decryption iv in bytes.
 * @param[in] StorageIvAddr: (Optional)Iv for storage image encryption.
 * @param[in] StorageIvSize:  The size of storage encryption iv in bytes.
 * @param[in] HeaderAddr: (Optional)The upgrade image header, not encrypted but signed.
 * @param[in] HeaderSize: The size of upgrade image header in bytes.
 * @param[in] CtxAddr: ctx_addr: Host address of the context for upgrade.size should be 1.6Kbyte.
 * @param[in] CtxSize: The size of context in bytes.
 */
typedef struct HSM_SecureUpgradeType__
{
    uint8 StorageAlg;                /**< Storage algorithm type (START_MODE used)*/
    uint8 UpgradeAlg;                /**< Upgrade algorithm type (START_MODE used)*/
    uint8 StorageEncryptionFlag;     /**< Storage encryption flag (START_MODE used)*/
    uint8 UpgradeDecryptionFlag;     /**< Upgrade decryption flag (START_MODE used)*/
    uint8 ProcessMode;               /**< Process mode (ALL used)*/
    uint8 CheckVersionFlag;          /**< Check version flag (START_MODE used)*/
    uint8 Rev[2];                    /**< Reserved bytes */
    uint32 UpgradeSignAddr;          /**< Upgrade signature address (START_MODE used)*/
    uint32 UpgradeSignSize;          /**< Upgrade signature size (START_MODE used)*/
    uint32 UpgradeVersionAddr;       /**< Upgrade version address (START_MODE used)*/
    uint32 UpgradeVersionSize;       /**< Upgrade version size (START_MODE used)*/
    uint32 UpgradeImageAddr;         /**< Upgrade image address (UPDATE or FINISH MODE used)*/
    uint32 UpgradeImageSize;         /**< Upgrade image size (UPDATE or FINISH MODE used)*/
    uint32 StorageImageAddr;         /**< Storage image address (UPDATE or FINISH MODE used)*/
    uint32 StorageImageSize;         /**< Storage image size (UPDATE or FINISH MODE used)*/
    uint32 MacSignAddr;              /**< MAC signature address (FINISH MODE used)*/
    uint32 MacSignSize;              /**< MAC signature size (FINISH MODE used)*/
    uint32 UpgradePubkeyAddr;        /**< Upgrade public key address (START MODE used)*/
    uint32 UpgradePubkeySize;        /**< Upgrade public key size (START MODE used)*/
    uint32 UpgradeIvAddr;            /**< Upgrade IV address (START MODE used)*/
    uint32 UpgradeIvSize;            /**< Upgrade IV size (START MODE used)*/
    uint32 StorageIvAddr;            /**< Storage IV address (START MODE used)*/
    uint32 StorageIvSize;            /**< Storage IV size (START MODE used)*/
    uint32 HeaderAddr;               /**< Header address (START MODE used)*/
    uint32 HeaderSize;               /**< Header size (START MODE used)*/
    uint32 CtxAddr;                  /**< Context address (ALL used)*/
    uint32 CtxSize;                  /**< Context size (ALL used)*/
}HSM_SecureUpgradeType;

/**
 * @brief Host image verify information.
 * @note DES ID: DES_HSM_TYP_007
 * @param[in] UpdateVersionFlag: 0: it means not updating version counter in OTP if verify soc success;
    other: it means updating version counter in OTP if verify soc success and the version counter is
    higher than OTP version counter.
 * @param[in] Type: Soc image verification or secure boot in Non-AutoVerify.
        1: For image verification.
        2: For secure boot.
 * @param[in] StorageAlg: 0: RSA2048 with SHA256, if support encryption, AES128 CBC is used.
        1: SM2 with SM3, if support encryption, SM4 CBC is
        used.
        2: AES128 CMAC, if support encryption, AES128 CBC
        is used.
        3: SM4 CMAC, if support encryption, SM4 CBC is used.
 * @param[in] StorageEncryptionFlag:  This is not supported now.
        0: The storage image doesn't need to be decrypted.
        1: The storage image needs to be decrypted.
 * @param[in] VersionAddr: Soc version counter.
 * @param[in] VersionSize: Length of Soc version counter,should be 16byte
 * @param[in] PubkeyAddr: : (Optional) The public key raw data. Only support SM2 and RSA2048. For SM2, the first byte
        '0x04' should be included. It is only valid for signature with asymmetric algorithm.
 * @param[in] PubkeySize: The size of public key in bytes. max value is 320 bytes.
 * @param[in] StorageImageAddr: The input encrypted (optional) storage image.
 * @param[in] StorageImageSize: The size of storage image in bytes.
 * @param[in] HeaderAddr: (Optional) The upgrade image header, not encrypted but signed.
 * @param[in] HeaderSize: The size of upgrade image header in bytes.
 * @param[in] StorageIvAddr: (Optional) Iv for storage image encryption.
 * @param[in] StorageIvSize: The size of storage encryption iv in bytes.
 * @param[in] StorageSignAddr: The signature of storage image.Support both symmetric and asymmetric algorithms.
 * @param[in] StorageSignSize: The signature size in bytes.
 */
typedef struct HSM_ImageVerifyType__
{
    uint8 UpdateVersionFlag;         /**< Update version flag */
    uint8 Type;                      /**< Verify type */
    uint8 StorageAlg;                /**< Storage algorithm type */
    uint8 StorageEncryptionFlag;     /**< Storage encryption flag */
    uint32 VersionAddr;              /**< Version address */
    uint32 VersionSize;              /**< Version size */
    uint32 PubkeyAddr;               /**< Public key address */
    uint32 PubkeySize;               /**< Public key size */
    uint32 StorageImageAddr;         /**< Storage image address */
    uint32 StorageImageSize;         /**< Storage image size */
    uint32 HeaderAddr;               /**< Header address */
    uint32 HeaderSize;               /**< Header size */
    uint32 StorageIvAddr;            /**< Storage IV address */
    uint32 StorageIvSize;            /**< Storage IV size */
    uint32 StorageSignAddr;          /**< Storage signature address */
    uint32 StorageSignSize;          /**< Storage signature size */
}HSM_ImageVerifyType;

/**
 * @brief Config OTP control bit for OTP key encrypt algorithm.
 * @note DES ID: DES_HSM_TYP_008
 */
typedef uint32 HSM_OtpKeyCipherAlgo;
#define OTP_KEY_CIPHER_AES   0U /**< The encrypt key use AES128 ECB */
#define OTP_KEY_CIPHER_SM4   1U /**< The encrypt key use SM4 ECB */
#endif

/**
 * @brief RNG algorithm type.
 * @note DES ID: DES_HSM_TYP_009
 */
typedef uint32 HSM_RndAlgo;
#define HSM_SM4_CTRDRBG   0U /**< Use SM4 */
#define HSM_AES_CTRDRBG   1U /**< Use AES128 */

/**
 * @brief Process mode type.
 * @note DES ID: DES_HSM_TYP_010
 */
typedef uint8 HSM_ProcessMode;
#define START_MODE        0x01U /**< Init phase */
#define UPDATE_MODE       0x02U /**< Update phase, will more loop to update input data */
#define FINISH_MODE       0x04U /**< Finish phase */
#define FW_ONEPASS_MODE   0x07U /**< One phase flow, for input data size < 1024 bytes */
#define BL_ONEPASS_MODE   0x08U /**< One phase flow , for bootloader*/

/**
 * @brief Specifies the boot type for the BOOT_DEFINE command.
 * @note DES ID: DES_HSM_TYP_011
 */
typedef uint8 HSM_CipherDirection;
#define HSM_ENCRYPTION   0U /**< Encrypt request */
#define HSM_DECRYPTION   1U /**< Decrypt request */

/**
 * @brief Specifies the generate/verify MAC value request type.
 * @note DES ID: DES_HSM_TYP_012
 */
typedef uint32 HSM_MacDirection;
#define HSM_MAC_GEN               0U /**< Generate MAC request */
#define HSM_MAC_VERIFY            1U /**< Verify MAC request */

/**
 * @brief Specifies the generate/verify signature value request type.
 * @note DES ID: DES_HSM_TYP_013
 */
typedef uint32 HSM_SignDirection;
#define HSM_SIGN    0U /**< Generate signature request */
#define HSM_VERIFY  1U /**< Verify signature request */

/**
 * @brief Specifies the symmetric algorithm cipher mode, support AES/SM4.
 * @note DES ID: DES_HSM_TYP_014
 */
typedef uint8 HSM_CipherMode;
#define ECB_MODE   1U /**< AES or SM4 encrypt/decrypt algorithm ECB mode */
#define CBC_MODE   3U /**< AES or SM4 encrypt/decrypt algorithm CBC mode */
#define CFB_MODE   4U /**< AES or SM4 encrypt/decrypt algorithm CFB mode */
#define OFB_MODE   5U /**< AES or SM4 encrypt/decrypt algorithm OFB mode */
#define CTR_MODE   6U /**< AES or SM4 encrypt/decrypt algorithm CTR mode */
#define GCM_MODE   10U /**< AES or SM4 encrypt/decrypt algorithm GCM mode */
#define CCM_MODE   11U /**< AES or SM4 encrypt/decrypt algorithm CCM mode */
#if defined (AC7840E)
#define CFB8_MODE   12U /**< AES or SM4 encrypt/decrypt algorithm CFB8 mode */
#define CFB128_MODE 13U /**< AES or SM4 encrypt/decrypt algorithm CFB128 mode */
#endif

/**
 * @brief Specifies the padding type for symmetric/asymmetric algorithms.
 * @note DES ID: DES_HSM_TYP_015
 */
typedef uint32 HSM_PaddingType;
#define HSM_NOPADDING      0U /**< No padding */
#define HSM_PSASSA_PSS     1U /**< PSS padding for RSA signature and verify */
#define HSM_PKCS7          2U /**< PKCS7 padding for symmetric algorithms */
#define HSM_ONEWITHZEROS   3U /**< One with zeros padding(ISO-7816-4) for symmetric algorithms */
#define HSM_OAEP           4U /**< OAEP padding for RSA Cipher */
/**
 * @brief Specifies the key algorithm type.
 * @note DES ID: DES_HSM_TYP_016
 */

#if defined (AC7843X)
typedef uint32 HSM_KeyAlgoType;
#define HSM_ALGO_TYPE_NONE      0U /**< None */
#define HSM_ALGO_TYPE_RANDOM    1U /**< Random key */
#define HSM_ALGO_TYPE_SYM       2U /**< Symmetric key */
#define HSM_ALGO_TYPE_SM2       3U /**< SM2 key */
#define HSM_ALGO_TYPE_ECC       4U /**< ECC key */
#define HSM_ALGO_TYPE_DH        5U /**< DH key */
#define HSM_ALGO_TYPE_RSA_CRT   6U /**< RSA CRT key */
#define HSM_ALGO_TYPE_RSA_COMM  7U /**< RSA common key */
#define HSM_ALGO_TYPE_END       10U /**< End of key types */
#endif

/**
 * @brief Specifies the set or generate key for algorithm type.
 * @note DES ID: DES_HSM_TYP_017
 */
#if defined(AUTOSAR_MCAL)
typedef uint32 HSM_AlgoType;
#endif
typedef uint32 HSM_KeyAlgo;
#define HSM_ALG_RANDOM           0U /**< Random key */
#define HSM_ALG_AES_128          4U /**< AES 128 algorithm */
#if defined (AC7843X)
#define HSM_ALG_AES_192          5U /**< AES 192 algorithm */
#endif
#define HSM_ALG_AES_256          6U /**< AES 256 algorithm */
#define HSM_ALG_SM4              7U /**< SM4 algorithm */
#if defined (AC7843X)
#define HSM_ALG_SM2              8U /**< SM2 algorithm */
#define HSM_ALG_RSA_1024         9U /**< RSA 1024 algorithm */
#endif
#define HSM_ALG_RSA_2048         10U /**< RSA 2048 algorithm */
#if defined (AC7843X)
#define HSM_ALG_RSA_1024_CRT     13U /**< RSA 1024 CRT algorithm */
#endif
#define HSM_ALG_RSA_2048_CRT     14U /**< RSA 2048 CRT algorithm */
#if defined (AC7843X)
#define HSM_ALG_DH               17U /**< DH algorithm */
#define HSM_ALG_SECP256R1        27U /**< SECP256R1 algorithm */
#define HSM_ALG_SECP384R1        28U /**< SECP384R1 algorithm */
#endif
#define HSM_ALG_END              31U /**< End of algorithms */

#if defined (AC7843X)
/**
 * @brief Specifies the key storage type.
 * @note DES ID: DES_HSM_TYP_018
 */
typedef uint32 HSM_KeyStorageType;
#define HSM_KEY_TYPE_NVM   0U /**< Store in flash */
#define HSM_KEY_TYPE_RAM   1U /**< Store in RAM */
#endif

/**
 * @brief Specifies the hash algorithm type.
 * @note DES ID: DES_HSM_TYP_019
 */
typedef uint32 HSM_HashAlgoType;
#define HSM_SM3           0U /**< SM3 hash algorithm */
#define HSM_SHA256        2U /**< SHA-256 hash algorithm */
#define HSM_SHA384        3U /**< SHA-384 hash algorithm */
#define HSM_SHA512        4U /**< SHA-512 hash algorithm */
#define HSM_SHA1          5U /**< SHA-1 hash algorithm */
#define HSM_SHA224        6U /**< SHA-224 hash algorithm */
#define HSM_SHA512_224    7U /**< SHA-512/224 hash algorithm */
#define HSM_SHA512_256    8U /**< SHA-512/256 hash algorithm */
#define HSM_INVALID_ALG   0xFFU /**< Invalid hash algorithm */

/**
 * @brief Specifies the symmetric algorithm type.
 * @note DES ID: DES_HSM_TYP_020
 */
typedef uint32 HSM_SymAlgoType;
#define HSM_AES_128   5U /**< AES 128 algorithm */
#define HSM_AES_256   7U /**< AES 256 algorithm */
#define HSM_SM4       8U /**< SM4 algorithm */

/**
 * @brief Specifies the asymmetric algorithm type.
 * @note DES ID: DES_HSM_TYP_026
 */
typedef uint32 HSM_AsymAlgoType;
#define HSM_ASYM_SM2               0U /**< SM2 asymmetric algorithm */
#define HSM_ASYM_ECDSA             1U /**< ECDSA asymmetric algorithm */
#define HSM_ASYM_RSA               2U /**< RSA asymmetric algorithm */
#define HSM_ASYM_RSA_CIPHER_1024   3U /**< RSA 1024 asymmetric algorithm only for dec or enc flow */
#define HSM_ASYM_RSA_CIPHER_2048   4U /**< RSA 2048 asymmetric algorithm only for dec or enc flow*/

#if defined (AC7843X)
/**
 * @brief Specifies the derive algorithm type.
 * @note DES ID: DES_HSM_TYP_021
 */
typedef uint32 HSM_KdfType;
#define HSM_KEY_DERIVE_KDFX963   0U /**< KDFX963 key derivation function */
#define HSM_KEY_DERIVE_PBKDF2    1U /**< PBKDF2 key derivation function */

/**
 * @brief Specifies the HSM life cycle type.
 * @note DES ID: DES_HSM_TYP_022
 */
typedef uint8 HSM_LifeCycleType;
#define HSM_LIFE_CYCLE_UNNORMAL_MODE   0x00U /**< Unnormal mode */
#define HSM_LIFE_CYCLE_TEST_MODE       0x01U /**< Test mode */
#define HSM_LIFE_CYCLE_DEV_MODE        0x02U /**< Development mode */
#define HSM_LIFE_CYCLE_MANU_MODE       0x03U /**< Manufacture mode */
#define HSM_LIFE_CYCLE_USER_MODE       0x04U /**< User mode */
#define HSM_LIFE_CYCLE_DEBUG_MODE      0x05U /**< Debug mode */
#define HSM_LIFE_CYCLE_DESTORY_MODE    0x06U /**< Destroy mode */
#endif

/**
 * @brief Specifies the derive algorithm type.
 * @note DES ID: DES_HSM_TYP_021
 */
typedef uint8 HSM_CrtMode;
#define HSM_RSA_CALC_NORMAL    0U /**< unuse CRT Mode(Normal) to calculate rsa decrypt and signature, key format [n,e,d] */
#define HSM_RSA_CALC_CRT       1U /**< CRT Mode to calculate rsa decrypt and signature, key format [n,e,p,q,dp,dq,u] */

/**
 * @brief Specifies the asymmetric algorithm config structure.
 * @note DES ID: DES_HSM_TYP_023
 */
typedef struct
{
    HSM_KeyId KeyId; /**< Key index */
    const uint8 *AuthValue; /**< Key authentication value */
    uint8 AuthValueSize; /**< Key authentication value size */
    boolean Async; /**< the variable is reserve. 0: sync, 1: async */
    const uint8 *Sm2UserIdBuf; /**< the user Id. just for sm2 algorithm */
    uint8 Sm2UserIdLen; /**< the user Id length, length < 64bytes. just for sm2 algorithm */
    uint8 *SaltLen; /**< salt value length address, only for RSA PSS used. the Salt length value must <= 128 bytes and Salt length + Hash digest length must < RSA Key.n length(moudle).RSA1024 n length = 128, RSA2048 n length = 256 */
    HSM_CrtMode CrtMode; /**< only support RSA Decrypt or Signature. 0: Not CRT Mode, 1: CRT Mode*/
    HSM_PaddingType Padding; /**< Data padding */
    HSM_SignDirection SignDir; /**< Request operation, e.g. sign/verify */
    HSM_CipherDirection CipherDir; /**< Request operation, e.g. decrypt/encrypt */
    HSM_HashAlgoType HAlgo; /**< Hash algorithm for asymmetric algorithm mode, e.g. SHA1 for RSA/ECC */
    HSM_AsymAlgoType AsymAlgo; /**< Asymmetric algorithm type, e.g. ECC/RSA */
} HSM_AsymCfgType;

/**
 * @brief Specifies the symmetric algorithm config structure.
 * @note DES ID: DES_HSM_TYP_024
 */
typedef struct
{
    HSM_KeyId KeyId; /**< Key index */
    const uint8 *AuthValue; /**< Key authentication value */
    uint8 AuthValueSize; /**< Key authentication value size */
    uint8 *Iv;   /**< Initialization vector, fixed length 16 */
    uint8 IvLen; /*!< Initialization vector length.GCM:12. CCM:13-15 */
    boolean Async; /**< the variable is reserve. 0: sync, 1: async */
    HSM_PaddingType Padding; /**< Data padding */
    HSM_CipherDirection CipherDir; /**< Request operation, e.g. decrypt/encrypt */
    HSM_CipherMode CMode; /**< For symmetric algorithm and CMAC algorithm, e.g. ECB/CBC */
    HSM_SymAlgoType SymAlgo; /**< Symmetric algorithm type, e.g. AES_128/AES_256 */
} HSM_SymCfgType;

#if defined (AC7843X)
/**
 * @brief Specifies the Hash MAC algorithm config structure.
 * @note DES ID: DES_HSM_TYP_025
 */
typedef struct
{
    uint32 KeyId; /**< Key index */
    const uint8 *AuthValue; /**< Key authentication value */
    uint8 AuthValueSize; /**< Key authentication value size */
    boolean Async; /**< the variable is reserve. 0: sync, 1: async */
    HSM_MacDirection MacDir; /**< Request operation, e.g. generate/verify */
    HSM_HashAlgoType HAlgo; /**< Hash algorithm for asymmetric algorithm mode, e.g. SHA1 for RSA/ECC */
} HSM_HMacCfgType;
#endif

/**
 * @brief Specifies the Cipher MAC algorithm config structure.
 * @note DES ID: DES_HSM_TYP_027
 */
typedef struct
{
    HSM_KeyId KeyId; /**< Key index */
    const uint8 *AuthValue; /**< Key authentication value */
    uint8 AuthValueSize; /**< Key authentication value size */
    boolean Async; /**< the variable is reserve. 0: sync, 1: async */
    HSM_MacDirection MacDir; /**< Request operation, e.g. generate/verify */
    HSM_SymAlgoType SymAlgo; /**< Symmetric algorithm type, e.g. AES_128/AES_256 */
} HSM_CMacCfgType;

/**
 * @brief The basic input and output structure.
 * @note DES ID: DES_HSM_TYP_028
 */
typedef struct
{
    const uint8 *InBuf; /**< Input buffer */
    uint32 InBufLen; /**< Input buffer length */
    uint8 *OutBuf; /**< Output buffer */
    uint32 *OutBufLen; /**< Output buffer length */
} HSM_InOutType;

/**
 * @brief The input and output structure about generate/verify MAC interface.
 * @note DES ID: DES_HSM_TYP_029
 */
typedef struct
{
    HSM_InOutType BasicInOut; /**< Basic input/output */
    const uint8 *MacInBuf; /**< MAC input buffer */
    uint32 MacInBufLen; /**< MAC input buffer length */
    boolean *Vry; /**< Verify result */
} HSM_InOutMacType;

/**
 * @brief The input and output structure about generate/verify signature interface.
 * @note DES ID: DES_HSM_TYP_030
 */
typedef struct
{
    HSM_InOutType BasicInOut; /**< Basic input/output */
    const uint8 *SignInBuf; /**< Signature input buffer */
    uint32 SignInBufLen; /**< Signature input buffer length */
    boolean *Vry; /**< Verify result */
} HSM_InOutSignType;

#if defined (AC7843X)
/**
 * @brief The key flags.
 * @note DES ID: DES_HSM_TYP_031
 */
typedef struct
{
    uint16 use_flags; /**< Attributes of use flags, only the least significant 10 bits are valid */
    uint8 trnsp_flags; /**< Attributes of transport flags, only the least significant 2 bits are valid */
    uint32 auth_flag; /**< Attributes of auth flags */
    uint8 auth_size; /**< Size of auth value */
    uint16 auth_value_exist_flags; /**< If value is 0 means element has auth_value data else indicates auth_value data exist in other element, NOTE now this flag only use on OTP key */
    uint8 auth_value[32]; /**< Auth data, only valid when auth_flags isn't 0 */
} HSM_KeyFlagsElementType;

/**
 * @brief The key attribute config structure.
 * @note DES ID: DES_HSM_TYP_032
 */
typedef struct
{
    HSM_KeyFlagsElementType sign; /**< Sign flags */
    HSM_KeyFlagsElementType verify; /**< Verify flags */
    HSM_KeyFlagsElementType encrypt; /**< Encrypt flags */
    HSM_KeyFlagsElementType decrypt; /**< Decrypt flags */
    HSM_KeyFlagsElementType timestamp; /**< Timestamp flags */
    HSM_KeyFlagsElementType secureboot; /**< Secure boot flags */
    HSM_KeyFlagsElementType securestorage; /**< Secure storage flags */
    HSM_KeyFlagsElementType dhkey; /**< DH key flags */
    HSM_KeyFlagsElementType utcsync; /**< UTC sync flags */
    HSM_KeyFlagsElementType transport; /**< Transport flags */
    HSM_KeyFlagsElementType remove; /**< Remove flags */
} HSM_KeyUsagesType;

/**
 * @brief The key active attribute config structure.
 * @note DES ID: DES_HSM_TYP_033
 */
typedef struct {
    boolean sign; /**< Sign permission */
    boolean verify; /**< Verify permission */
    boolean encrypt; /**< Encrypt permission */
    boolean decrypt; /**< Decrypt permission */
    boolean timestamp; /**< Timestamp permission */
    boolean secureboot; /**< Secure boot permission */
    boolean securestorage; /**< Secure storage permission */
    boolean createkey; /**< Create key permission */
    boolean utcsync; /**< UTC sync permission */
    boolean transport; /**< Transport permission */
    boolean remove; /**< Remove permission */
}HSM_KeyActUseFlagsTypeSec;

/**
 * @brief The ECC public key structure.
 * @note DES ID: DES_HSM_TYP_034
 */
typedef struct
{
    uint8 p[132]; /**< ECC public key, e.g. SECP256/384 */
}HSM_EccPubKeyType;

/**
 * @brief The RSA public key structure.
 * @note DES ID: DES_HSM_TYP_035
 */
typedef struct
{
    uint8 n[256]; /**< RSA modulus */
    uint8 e[256]; /**< RSA public exponent */
}HSM_RsaPubKeyType;

/**
 * @brief The DH algorithm parameter.
 * @note DES ID: DES_HSM_TYP_036
 */
typedef struct
{
    uint8 p[512]; /**< DH parameter p */
    uint8 q[512]; /**< DH parameter q */
    uint8 g[512]; /**< DH parameter g */
}HSM_DhParamType;

/**
 * @brief The DH public key structure.
 * @note DES ID: DES_HSM_TYP_037
 */
typedef struct Hsm_DhPubKeyType_
{
    uint8 pub[512]; /**< DH public key */
    HSM_DhParamType DhParam; /**< DH parameters */
}HSM_DhPubKeyType;

/**
 * @brief The public key union for all algorithms.
 * @note DES ID: DES_HSM_TYP_038
 */
typedef union Hsm_PubKeyDataType_
{
    HSM_RsaPubKeyType Rsa; /**< RSA public key */
    HSM_EccPubKeyType Ecc; /**< ECC public key */
    HSM_DhPubKeyType Dh; /**< DH public key */
}HSM_PubKeyDataType;

/**
 * @brief The DH private key structure.
 * @note DES ID: DES_HSM_TYP_039
 */
typedef struct
{
    uint8 Priv[512]; /**< DH private key */
}HSM_DhPriKeyType;

/**
 * @brief The RSA private key structure when using CRT.
 * @note DES ID: DES_HSM_TYP_040
 */
typedef struct Hsm_RsaCrtType_
{
    uint8 p[128]; /**< RSA CRT parameter p */
    uint8 q[128]; /**< RSA CRT parameter q */
    uint8 dp[128]; /**< RSA CRT parameter dp */
    uint8 dq[128]; /**< RSA CRT parameter dq */
    uint8 u[128]; /**< RSA CRT parameter u */
}HSM_RsaPrvCrtType;

/**
 * @brief The private key union for all algorithms.
 * @note DES ID: DES_HSM_TYP_041
 */
typedef union Hsm_PriKeyDataType_
{
    uint8 SymKey[1024]; /**< Symmetric key */
    uint8 EccKey[66]; /**< ECC private key */
    uint8 RsaD[256]; /**< RSA private key D */
    uint8 KdfKey[512]; /**< KDF key */
    uint8 DhKey[512]; /**< DH private key */
    HSM_DhPriKeyType Dh; /**< DH private key structure */
    HSM_RsaPrvCrtType RsaCtr; /**< RSA CRT private key */
}HSM_PriKeyDataType;

/**
 * @brief The secret key config structure for key import or export by secret.
 * @note DES ID: DES_HSM_TYP_042
 */
typedef struct
{
    HSM_KeyAlgo KeyAlgo; /**< Key algorithm */
    HSM_KeyActUseFlagsTypeSec *SetKeyUseFlag; /**< Key usage flags */
    uint8 *AuthValue; /**< Authentication value */
    uint32 *AuthVAlueSize; /**< Authentication value size */
    uint8 *SecretKeyBlob; /**< Only for export/import secret buffer, buffer size must >= 3924 bytes */
    uint32 *SetKeyBlobSize; /**< Only for export/import buffer size, buffer size must >= 3924 bytes */
} HSM_SecretKeyCfgType;

/**
 * @brief The derive key config structure.
 * @note DES ID: DES_HSM_TYP_043
 */
typedef struct
{
    HSM_KdfType Kdf; /**< Key derivation function type */
    uint32 KeySize; /**< Key size */
    uint32 ValidUntil; /**< Key valid time when HSM time support open be used */
    uint8 KeyUsageSize; /**< Key usage size */
    HSM_KeyFlagsElementType *KeyUsage; /**< Key usage flags */
    uint32 SaltDataSize; /**< Salt data size */
    uint8 *SaltData; /**< Salt data */
    HSM_KeyId KeyId; /**< Key ID */
} HSM_DeriveKeyCfgType;

/**
 * @brief The generate random key config structure by random generator in HSM.
 * @note DES ID: DES_HSM_TYP_044
 */
typedef struct
{
    HSM_KeyAlgo KeyAlgo; /**< Key algorithm */
    uint32 KeySize; /**< Key size */
    uint32 ValidUntil; /**< Key valid time */
    uint8 KeyUsageSize; /**< Key usage size */
    HSM_KeyFlagsElementType *KeyUsage; /**< Key usage flags */
    HSM_KeyId KeyId; /**< Key ID */
} HSM_GenKeyCfgType;

/**
 * @brief Key active use flags structure
 * @note DES ID: DES_HSM_TYP_034
 */
typedef struct {
    boolean Encrypt;            /**< Encrypt permission for AES/SM4 */
    boolean Decrypt;            /**< Decrypt permission for AES/SM4 */
    boolean Sign;               /**< Sign permission for ECDSA/SM2/RSA/AES_CMAC/SM4_CMAC */
    boolean Verify;             /**< Verify permission for ECDSA/SM2/RSA/AES_CMAC/SM4_CMAC */
    boolean KeyExch;            /**< key exchange flag when asymmerical key (ECC) */
    boolean KeyCreation;        /**< Key creation permission */
    boolean Remove;             /**< Remove flag */
} HSM_KeyActUseFlagsType;
#endif

/**
 * @brief The set plain key config structure.
 * @note DES ID: DES_HSM_TYP_045
 */
typedef struct
{
    HSM_KeyAlgo KeyAlgo; /**< Key generate algorithm */
    uint8 *PrivKey; /**< Pointer to the buffer, the private key store in buffer */
    uint32 PrivKeyLen; /**< Private key length */
    uint8 *PubKey; /**< Pointer to the buffer, the public key store in buffer */
    uint32 PubKeyLen; /**< Public key length */
#if defined (AC7843X)
    HSM_KeyActUseFlagsType *KeyUsages; /**< Key usages for attribute, e.g. enc/dec */
#endif
    const uint8 *AuthValue; /**< Key authentication value */
    uint8 AuthValueSize; /**< Key authentication value size */
    HSM_KeyId KeyId; /**< Key ID according to key algorithm type */
} HSM_PlainKeyCfgType;

/**
 * @brief Input and output structure for AEAD.
 * @note DES ID: DES_HSM_TYP_059
 */
typedef struct
{
    HSM_InOutType BasicInOut;  /*!< Basic input/output */
    const uint8 *AssoData;     /*!< Associated data */
    uint8 AssoDataLen;         /*!< Associated data length */
    uint8 *Tag;                /*!< authentication message */
    uint8 TagLen;              /*!< authentication length */
} HSM_InOutSymType;

#if defined (AC7843X)
/**
 * @brief The IRQ number enumeration.
 * @note DES ID: DES_HSM_TYP_046
 */
typedef uint32 HSM_IrqNum;
#define HSM_CLR_S2H_NOTE_HOST_IRQ   0U /**< HSM clear S2H note host IRQ */
#define S2H_NOTE_COLLSION_HOST_IRQ  1U /**< S2H note collision host IRQ */
#define HOST_SET_S2H_NOTE_HSM_IRQ   2U /**< Host set S2H note HSM IRQ */
#define S2H_NOTE_COLLSION_HSM_IRQ   3U /**< S2H note collision HSM IRQ */
#define HSM_SET_H2S_NOTE_HOST_IRQ   4U /**< HSM set H2S note host IRQ */
#define H2S_NOTE_COLLSION_HOST_IRQ  5U /**< H2S note collision host IRQ */
#define SOC_CLEAR_H2S_NOTE_HSM_IRQ  6U /**< SOC clear H2S note HSM IRQ */
#define H2S_NOTE_COLLSION_HSM_IRQ   7U /**< H2S note collision HSM IRQ */
#define HSM_IRQ_NUM                 8U /**< HSM IRQ number */
#endif

/**
 * @brief Debug authentication algorithm type.
 * @note DES ID: DES_HSM_TYP_047
 */
typedef uint32 HSM_DebugAuthAlgoType;
#define HSM_DEBUG_AUTH_ALG_SM2_WITH_SM3              1U /**< SM2 with SM3 algorithm */
#define HSM_DEBUG_AUTH_ALG_ECCSECP256R1_WITH_SHA256  2U /**< ECC SECP256R1 with SHA256 algorithm */
#define HSM_DEBUG_AUTH_ALG_SM4_CMAC                  3U /**< SM4 CMAC algorithm */
#define HSM_DEBUG_AUTH_ALG_AES128_CMAC               4U /**< AES128 CMAC algorithm */

/**
 * @brief The get challenge used type.
 * @note DES ID: DES_HSM_TYP_048
 */
typedef uint32 HSM_ChallengeType;
#define HSM_CHALLENGE_TYPE_INVALID     0U /**< Invalid challenge type */
#define HSM_CHALLENGE_TYPE_TIME_SYNC   1U /**< Time sync challenge type */
#define HSM_CHALLENGE_TYPE_EHSM_DEBUG  2U /**< eHSM debug challenge type */
#define HSM_CHALLENGE_TYPE_SHE_DEBUG   3U /**< SHE debug challenge type */
#define HSM_CHALLENGE_TYPE_SOC_DEBUG   4U /**< SOC debug challenge type */
#define HSM_CHALLENGE_TYPE_USER_AUTH   5U /**< User authentication challenge type */
#define HSM_CHALLENGE_TYPE_MAX         6U /**< Maximum challenge type */

/**
 * @brief The debug auth config structure.
 * @note DES ID: DES_HSM_TYP_049
 */
typedef struct
{
#if defined (AC7840E)
    HSM_KeyId keyId; /**< key id */
    uint32 *respBuf; /**< response bug */
#else
    HSM_ChallengeType Type; /**< Challenge type */
    uint32 SignatureSize; /**< Signature size */
    uint8 *Signature; /**< Signature data */
    HSM_DebugAuthAlgoType Alg; /**< Debug authentication algorithm */
    uint32 PubKeySize; /**< Public key size */
    uint8 *PubKey; /**< Public key data */
#endif
} HSM_DebugAuthConfigType;

#if defined (AC7843X)
/**
 * @brief The key status in HSM.
 * @note DES ID: DES_HSM_TYP_050
 */
typedef struct
{
    HSM_KeyId TargetKeyId; /**< Target key ID */
    HSM_KeyId CertificationKeyId; /**< Certification key ID */
    uint32 CertificationAuthSize; /**< Certification authentication size */
    uint8 *CertificationAuth; /**< Certification authentication data */
    uint32 *KeyStatusSize; /**< Key status size */
    ehsm_key_status_st *KeyStatus; /**< Key status structure */
}HSM_KeyStatusType;

/**
 * @brief The HSM and host upgrade algorithm or verify algorithm.
 * @note DES ID: DES_HSM_TYP_051
 */
typedef uint32 HSM_OtpCtrlUpgradeAlgoType;
#define CODE_UPGRADE_RSA_2048_WITH_AES128_CBC      0U /**< Indicate RSA 2048 for signature (e,n) and AES128 CBC for encrypt/decrypt */
#define CODE_UPGRADE_SM2_WITH_SM4_CBC              1U /**< Indicate SM2 for signature and SM4 CBC for encrypt/decrypt */
#define CODE_UPGRADE_AES128_CMAC_WITH_AES128_CBC   4U /**< Indicate AES128 CMAC for signature and AES128 CBC for encrypt/decrypt */
#define CODE_UPGRADE_SM4_CMAC_WITH_SM4_CBC         5U /**< Indicate SM4 CMAC for signature and SM4 CBC for encrypt/decrypt */
#define CODE_UPGRADE_RESERVE                     0xFU /**< reserve value */
/**
 * @brief The HSM and host secure boot flow verify image algorithm.
 * @note DES ID: DES_HSM_TYP_052
 */
typedef uint32 HSM_OtpCtrlVerifyAlgoType;
#define CODE_VERIFY_RSA_2048_WITH_SHA256   0U /**< Indicate RSA 2048 for signature (e,n) and SHA256 for hash */
#define CODE_VERIFY_SM2_WITH_SM3           1U /**< Indicate SM2 for signature and SM3 for hash */
#define CODE_VERIFY_AES128_CMAC            2U /**< Indicate AES128 CMAC for signature */
#define CODE_VERIFY_SM4_CMAC               3U /**< Indicate SM4 CMAC for signature */
#define CODE_VERIFY_RESERVE              0xFU /**< reserve value */
/**
 * @brief Specifies the image type host image or HSM firmware image.
 * @note DES ID: DES_HSM_TYP_053
 */
typedef uint32 HSM_ImageType;
#define HSM_IMAGE_HOST_TYPE   0U /**< Host image type */
#define HSM_IMAGE_HSM_TYPE    1U /**< HSM firmware image type */

/**
 * @brief Hsm image install struct.
 * @note DES ID: DES_HSM_TYP_054
 * @param[in] ProMode: 1: START
                    2: UPDATE
                    4: FINISH
                    8: ONEPASS
 * @param[in] ImageAddr:
                   For installation, it is the host address of the total image for ONEPASS.
                   For upgrading, it is the header of image for START, or the encrypted part
                   of image for UPDATE, or the last encrypted part (may be null) of image for FINISH
 * @param[in] ImageSize: The size of upgrade image in bytes
 * @param[in] CtxAddr: Host address of the context for streaming approach
 * @param[in] CtxSize: The size of context in bytes
 */
typedef struct HSM_HsmImageInstallType__
{
    HSM_ProcessMode ProMode;
    uint32 ImageAddr;
    uint32 ImageSize;
    uint32 CtxAddr;
    uint32 CtxSize;
}HSM_HsmImageInstallType;

/**
 * @brief Hsm image verify struct.
 * @note DES ID: DES_HSM_TYP_055
 * @param[in] ProMode: 1: START
                    2: UPDATE
                    4: FINISH
                    8: ONEPASS
 * @param[in] VerifyType: 0x00: For image verification.
                    0x03: For secure boot.
 * @param[in] ImageAddr:
                   For installation, it is the host address of the total image for ONEPASS.
                   For upgrading, it is the header of image for START, or the encrypted part
                   of image for UPDATE, or the last encrypted part (may be null) of image for FINISH
 * @param[in] ImageSize: The size of upgrade image in bytes
 * @param[in] CtxAddr: Host address of the context for streaming approach
 * @param[in] CtxSize: The size of context in bytes
 */
typedef struct HSM_HsmImageVerifyType__
{
    HSM_ProcessMode ProMode;
    uint8 VerifyType;
    uint32 ImageAddr;
    uint32 ImageSize;
    uint32 CtxAddr;
    uint32 CtxSize;
}HSM_HsmImageVerifyType;

/**
 * @brief HSM OTP key for extern key symmetric, SM2 private key, secp256R1 private key.
 * @note DES ID: DES_HSM_TYP_056
 */
typedef uint32 HSM_ExternOtpKeyType;
#define HSM_EXTERN_OTP_KEY_SYM                    1
#define HSM_EXTERN_OTP_KEY_HASH_OF_PUBKEY         2
#endif

/**
 * @brief Key handle info structure. Size: 40 bytes.
 * @note DES ID: DES_HSM_TYP_058
 */
typedef struct
{
    HSM_KeyId KeyId; /**< Key handle ID by HSM */
    uint8 AuthValue[32]; /**< Authentication value */
    uint32 AuthSize; /**< Authentication size */
}HSM_KeyHandleInfoType;

/**
 * @brief hsm module init function.
 * @note  Function ID: DES_HSM_API_001
 * @param [in] HsmIrqEn: hsm irq enable/disable 1:enable 0:disable(default)
 * @param [in] HsmIrqPri: hsm irq priority.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  HSM initialization successful
 * @retval     STATUS_ERROR                    HSM initialization failed or timeout
 */
Hal_StatusType HSM_Hal_Init(boolean HsmIrqEn, uint8 HsmIrqPri);

#if defined (AC7843X)
#if !defined(AUTOSAR_MCAL)
/**
 * @brief hsm module deinit function.
 * @note  Function ID: DES_HSM_API_002
 * @return void
 */
void HSM_Hal_Deinit(void);
#endif
#endif

/**
 * @brief Aes Decrypt/Encrypt function.
 * @note  Function ID: DES_HSM_API_003
 * @param [in] CfgPtr: Aes Algo config
            KeyId: Key Index.
            Iv: init vector 16 byte
            Padding:data padding type.
            CipherDir :request opeartion, eg.decrypt/encrypt.
            CMode:for sym algo and CMAC algo, eg.ECB/CBC.
            SymAlgo: symmetric algo type, eg.AES_128/AES_256
 * @param [in] InOutPtr: Input and output config.
                InBuf: input buffer
                InBufLen: input buffer length.
                    1. InBufLen<1024 when ProMode = FW_ONEPASS_MODE and Padding == HSM_NOPADDING
                    2. InBufLen<=1024 when ProMode = FW_ONEPASS_MODE and Padding != HSM_NOPADDING
                OutBuf: output buffer
                OutBufLen: output buffer length
 * @param [in] ProMode: process mode.
 * @return Hal_StatusType:
 * @retval     STATUS_SUCCESS                  AES operation successful
 * @retval     STATUS_ERROR                    Operation failed
 * @retval     HSM_WRONG_ALGO_TYPE             Invalid algorithm type
 */
Hal_StatusType HSM_Hal_AesCipher(const HSM_SymCfgType *CfgPtr,
                const HSM_InOutSymType* InOutPtr, HSM_ProcessMode ProMode);

/**
 * @brief Sm4 Decrypt/Encrypt function.
 * @note  Function ID: DES_HSM_API_004
 * @param [in] CfgPtr: sm4 Algo config
            KeyId: Key Index.
            Iv: init vector, 16byte
            Padding:data padding type.
            CipherDir :request opeartion, eg.decrypt/encrypt.
            CMode:for sym algo and CMAC algo, eg.ECB/CBC.
            SymAlgo: symmetric algo type, eg.SM4
 * @param [in] InOutPtr: Input and output config.
                InBuf: input buffer
                InBufLen: input buffer length.
                    1. InBufLen<1024 when ProMode = FW_ONEPASS_MODE and Padding == HSM_NOPADDING
                    2. InBufLen<=1024 when ProMode = FW_ONEPASS_MODE and Padding != HSM_NOPADDING
                OutBuf: output buffer
                OutBufLen: output buffer length
 * @param [in] ProMode: process mode.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  SM4 operation successful
 * @retval     STATUS_ERROR                    Operation failed
 * @retval     HSM_WRONG_ALGO_TYPE             Invalid algorithm type
 */
Hal_StatusType HSM_Hal_Sm4Cipher(const HSM_SymCfgType *CfgPtr,
            const HSM_InOutSymType* InOutPtr, HSM_ProcessMode ProMode);

/**
 * @brief Cipher Mac Generate/Verify mac function.
 * @note  Function ID: DES_HSM_API_005
 * @param [in] CfgPtr: Algo config
            KeyId: Key Index.
            MacDir :request opeartion, eg.decrypt/encrypt.
            SymAlgo: symmetric algo type, eg.AES_128/SM4
 * @param [in] InOutPtr: Input and output config.
                BasicInOut:
                    InBuf: input buffer
                    InBufLen: input buffer length
                    OutBuf: output buffer
                    OutBufLen: output buffer length
                MacInBuf: store Mac value when verify
                MacInBufLen: Mac length
                Vry: verify result.
 * @param [in] ProMode: process mode.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  MAC operation successful
 * @retval     STATUS_ERROR                    Operation failed
 * @retval     HSM_WRONG_KEY_HANDLE            Invalid key handle
 */
Hal_StatusType HSM_Hal_CipherMac(const HSM_CMacCfgType *CfgPtr, HSM_InOutMacType *InOutPtr, HSM_ProcessMode ProMode);

/**
 * @brief Rsa Decrypt/Encrypt function.
 * @note  Function ID: DES_HSM_API_006
 * @param [in] CfgPtr: rsa Algo config.
 * @param [in] InOutPtr: Input and output config.
                InBuf: input buffer
                InBufLen: input buffer length.
                    RSA1024: InBufLen < 128 when NO PADDING
                    RSA2048: InBufLen < 256 when NO PADDING
                OutBuf: output buffer
                OutBufLen: output buffer length
 * @param [in] ProMode: process mode.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  RSA operation successful
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_RsaCipher(const HSM_AsymCfgType *CfgPtr, const HSM_InOutType *InOutPtr, HSM_ProcessMode ProMode);

#if defined (AC7843X)
/**
 * @brief Sm2 Decrypt/Encrypt function.
 * @note  Function ID: DES_HSM_API_007
 * @param [in] CfgPtr: sm2 Algo config.
 * @param [in] InOutPtr: Input and output config. only support ciphertext order C1C3C2.
 * @param [in] ProMode: process mode.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  SM2 operation successful
 * @retval     STATUS_ERROR                    Operation failed
 * @retval     HSM_WRONG_CHUNK_SIZE            Invalid chunk size
 */
Hal_StatusType HSM_Hal_Sm2Cipher(const HSM_AsymCfgType *CfgPtr, const HSM_InOutType *InOutPtr, HSM_ProcessMode ProMode);

/**
 * @brief ECC Sign/Verify function.
 * @note  Function ID: DES_HSM_API_008
 * @param [in] CfgPtr: ecc Algo config.
 * @param [in] InOutPtr: Input and output config.
                BasicInOut
                    InBuf: input buffer
                    InBufLen: input buffer length
                    OutBuf: output buffer
                    OutBufLen: output buffer length
                SignInBuf: store signature value when verify
                SignInBufLen: signature length
                Vry: verify result.
 * @param [in] ProMode: process mode.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  ECC operation successful
 * @retval     STATUS_ERROR                    Operation failed
 * @retval     HSM_WRONG_KEY_HANDLE            Invalid key handle
 */
Hal_StatusType HSM_Hal_EccSign(const HSM_AsymCfgType *CfgPtr, const HSM_InOutSignType *InOutPtr,
                                        HSM_ProcessMode ProMode);
#endif

/**
 * @brief Rsa Sign/Verify function.
 * @note  Function ID: DES_HSM_API_009
 * @param [in] CfgPtr: rsa config.
 *              SaltLen: only for RSA PSS used.the Salt length value must <= 128 bytes and Salt length + Hash digest length must < RSA Key.n length(moudle).RSA1024 n length = 128, RSA2048 n length = 256.
 * @param [in] InOutPtr: rsa Input and output config.
 * @param [in] ProMode: process mode.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  RSA sign/verify operation successful
 * @retval     STATUS_ERROR                    Operation failed
 * @retval     HSM_WRONG_KEY_HANDLE            Invalid key handle
 */
Hal_StatusType HSM_Hal_RsaSign(const HSM_AsymCfgType *CfgPtr, const HSM_InOutSignType *InOutPtr,
                                        HSM_ProcessMode ProMode);
#if defined (AC7843X)
/**
 * @brief SM2 Sign/Verify function.
 * @note  Function ID: DES_HSM_API_010
 * @param [in] CfgPtr: Sm2 Algo config.
 * @param [in] InOutPtr: Input and output config.
 * @param [in] ProMode: process mode.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  SM2 sign/verify operation successful
 * @retval     STATUS_ERROR                    Operation failed
 * @retval     HSM_WRONG_KEY_HANDLE            Invalid key handle
 */
Hal_StatusType HSM_Hal_Sm2Sign(const HSM_AsymCfgType *CfgPtr, const HSM_InOutSignType *InOutPtr,
                                        HSM_ProcessMode ProMode);
#endif

/**
 * @brief generate message digest(hash value) function.
 * @note  Function ID: DES_HSM_API_011
 * @param [in] Algo: hash algorithm type.
 * @param [in] InOutPtr: Input and output config.
 * @param [in] Async: FALSE:Sync Flow, TRUE:Async Flow.
 * @param [in] ProMode: process mode.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Hash operation successful
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_Hash(const HSM_HashAlgoType Algo, const HSM_InOutType *InOutPtr,
        boolean Async, HSM_ProcessMode ProMode);

#if defined (AC7843X)
/**
 * @brief generate message hash mac function use hash algo
 * @note  Function ID: DES_HSM_API_012
 * @param [in] CfgPtr: hash Algo config.
 * @param [in] InOutPtr: Input and output config.
 * @param [in] ProMode: process mode.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Hash MAC operation successful
 * @retval     STATUS_ERROR                    Operation failed
 * @retval     HSM_WRONG_KEY_HANDLE            Invalid key handle
 */
Hal_StatusType HSM_Hal_HashMac(const HSM_HMacCfgType *CfgPtr, const HSM_InOutMacType *InOutPtr,
                                        HSM_ProcessMode ProMode);
#endif

/**
 * @brief set plain key to hsm mem.
 * @note  Function ID: DES_HSM_API_013
 * @param [in] CfgPtr: plain key config.
            PrivKey:if set Rsa Private Key, CRT: Format[p,q,dp,dq,u], Not CRT: Format [d]. all bytes combination
            PubKey:[n,e] all bytes combination
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Plain key set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_SetPlainKey(const HSM_PlainKeyCfgType *CfgPtr);

#if defined (AC7843X)
/**
 * @brief generate key for algo, eg.aes/sm2.
 * @note  Function ID: DES_HSM_API_014
 * @param [in] CfgPtr: generate key config.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Key generation successful
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_GenerateKey(const HSM_GenKeyCfgType *CfgPtr);

#if !defined(AUTOSAR_MCAL)
/**
 * @brief derive key
 * @note  Function ID: DES_HSM_API_015
 * @param [in] ParentKeyInfo: parent key handle.
 * @param [in] ParentKeyAlgo: parent key algo.
 * @param [in] TargetKeyPtr: Target key config.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Key derivation successful
 * @retval     STATUS_ERROR                    Operation failed
 * @retval     HSM_WRONG_KEY_HANDLE            Invalid parent key handle
 */
Hal_StatusType HSM_Hal_DeriveKey(const HSM_KeyHandleInfoType *ParentKeyInfo, HSM_KeyAlgo ParentKeyAlgo,
        const HSM_DeriveKeyCfgType *TargetKeyPtr);
/**
 * @brief set secret key to hsm
 * @note  Function ID: DES_HSM_API_016
 * @param [in] TpKeyId: transport key id.
 * @param [in] AuthKeyId: auth key id.
 * @param [in] ImportKeyPtr: Target key config.
 * @param [in] KeyId: Target key id.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Secret key set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_SetSecretKey(const HSM_KeyHandleInfoType *TpKeyId, const HSM_KeyHandleInfoType *AuthKeyId,
            const HSM_SecretKeyCfgType *ImportKeyPtr, HSM_KeyId KeyId);

/**
 * @brief get secret key to hsm
 * @note  Function ID: DES_HSM_API_017
 * @param [in] TpKeyInfo: transport key id.
 * @param [in] AuthKeyInfo: auth key id.
 * @param [in] TargetKeyId: Target key id.
 * @param [in] ExportKeyCfg: export key blob config
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Secret key retrieved successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_GetSecretkey(const HSM_KeyHandleInfoType *TpKeyInfo, const HSM_KeyHandleInfoType *AuthKeyInfo,
    HSM_KeyId TargetKeyId, const HSM_SecretKeyCfgType *ExportKeyCfg);

/**
 * @brief get pubkey form private key
 * @note  Function ID: DES_HSM_API_018
 * @param [in] KeyHandle: Target key handle.
 * @param [out] PubKey: the pubkey key buffer.
 * @param [out] PubKeySize: the pubkey key size.
 * @param [in] Algo: target key algo.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Public key retrieved successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_GetPubKeyFromPrvKey(const HSM_KeyHandleInfoType *KeyHandle,
            uint8 *PubKey, uint32 *PubKeySize, HSM_KeyAlgo Algo);

/**
 * @brief generate DH key.
 * @note  Function ID: DES_HSM_API_019
 * @param [in] LocalKeyHandle: local key handle.
 * @param [out] RemotePubKey: the pubkey key buffer from remote.
 * @param [out] RemotePubKeySize: the remote pubkey key size.
 * @param [in] ParKeyAlgo: generate dh key config.
 * @param [in] TargetKeyPtr: generate dh key config.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  DH key generation successful
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_GenerateDHKey(const HSM_KeyHandleInfoType *LocalKeyHandle,
        const uint8 *RemotePubKey, uint32 RemotePubKeySize,
        HSM_KeyAlgo ParKeyAlgo ,const HSM_GenKeyCfgType *TargetKeyPtr);

/**
 * @brief Key Copy.
 * @note  Function ID: DES_HSM_API_052
 * @param [in] SrcKeyId: source key id.
 * @param [in] DestKeyId: destination key id
 * @param [in] DestKeyUsage: destination key usages.
 * @param [in] DestKeyUsageSize: destination key usages size.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  DH key generation successful
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_KeyCopy(const HSM_KeyHandleInfoType *KeyHandle, HSM_KeyId DestKeyId,
        HSM_KeyFlagsElementType *DestKeyUsage, uint32 DestKeyUsageSize);
#endif
#endif

/**
 * @brief Remove Key.
 * @note  Function ID: DES_HSM_API_020
 * @param [in] KeyHandle: Key information
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Key removed successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_RemoveKey(const HSM_KeyHandleInfoType *KeyHandle);

#if defined (AC7843X)
#if !defined(AUTOSAR_MCAL)
/**
 * @brief direction notify HSM to remove all nvm key.
 * @note  Function ID: DES_HSM_API_021
 * @return     boolean
 * @retval     STATUS_SUCCESS                  Key removed successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
void HSM_Hal_RemoveAllNvmKeyByHsm(void);
#endif
#endif

/**
 * @brief get random.
 * @note  Function ID: DES_HSM_API_022
 * @param [in] Algo: trng or prng use algo type
 * @param [in] RndSize: random size, max size is 128 bytes
 * @param [out] Rnd: random information
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Random generation successful
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_GetRnd(HSM_RndAlgo Algo, uint32 RndSize, uint8 *Rnd);

#if !defined(AUTOSAR_MCAL)
#if defined (AC7843X)
/**
 * @brief set secure boot config
 * @note  Function ID: DES_HSM_API_023
 * @param [in] CfgPtr: Secureboot Config
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Secure boot configuration set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_SetSecureBootCfg(const HSM_BootCfgType *CfgPtr);

/**
 * @brief enable secureboot
 * @note  Function ID: DES_HSM_API_024
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Secure boot enabled successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_EnableSecureBoot(void);

/**
 * @brief disable secureboot
 * @note  Function ID: DES_HSM_API_025
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Secure boot disabled successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_DisableSecureBoot(void);

/**
 * @brief Get LifeCycle by read OTP
 * @note  Function ID: DES_HSM_API_026
 * @return HSM_LifeCycleType: Current HSM life cycle type.
 */
HSM_LifeCycleType HSM_Hal_GetLifeCycle(void);

/**
 * @brief Set LifeCycle by otp write in host. need lifecycle = TEST/DEV.
 * @note  Function ID: DES_HSM_API_027
 * @param [in] LcIndex: lifecycle type
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Lifecycle set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_SetLifeCycle(HSM_LifeCycleType LcIndex);

/**
 * @brief Set LifeCycle by HSM.
 * @note  Function ID: DES_HSM_API_028
 * @param [in] LcIndex: lifecycle type
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Lifecycle set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_ChangeLifecycleByHsmFw(HSM_LifeCycleType LcIndex);
#endif

#if defined (AC7840E)
/*!
 * @brief HSM get UID.
 *
 * @param[out] The HSM UID pointer
 * @return none
 */
void HSM_Hal_GetUID(uint32 *uid);
#endif

/**
 * @brief Debug authentication
 * @note  Function ID: DES_HSM_API_028
 * @param [in] CfgPtr: Debug Auth config information struct
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Debug authentication successful
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_DebugAuth(HSM_DebugAuthConfigType *CfgPtr);

#if defined (AC7840E)
Hal_StatusType HSM_Hal_GetChallenge(HSM_OtpKeyId keyId, uint8 *ChBuf, uint32 ChSize);
#else
/**
 * @brief get challenge
 * @note  Function ID: DES_HSM_API_029
 * @param [in] Type: challenge type
 * @param [out] ChBuf: challenge buf
 * @param [in] ChSize: challenge buf size
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Challenge retrieved successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_GetChallenge(HSM_ChallengeType Type, uint8 *ChBuf, uint32 ChSize);
#endif
#endif

/**
 * @brief irq callback.
 * @note  Function ID: DES_HSM_API_030
 * @param [in] Callback: callback function
 * @param [in] Args: args
 * @return void
 */
void HSM_Hal_InstallCallback(const Hal_CallbackType Callback, void *Args);

#if defined (AC7843X)
#if !defined(AUTOSAR_MCAL)
/**
 * @brief get access hsm path lock state.
 * @note  Function ID: DES_HSM_API_031
 * @return boolean: TRUE if locked, FALSE otherwise.
 */
boolean HSM_Hal_GetLockState(void);

/**
 * @brief Lock access hsm path.
 * @note  Function ID: DES_HSM_API_032
 * @return void
 */
void HSM_Hal_Lock(void);

/**
 * @brief unlock access hsm path.
 * @note  Function ID: DES_HSM_API_037
 * @return void
 */
void HSM_Hal_Unlock(void);

/**
 * @brief get key status for specific key id
 * @note  Function ID: DES_HSM_API_033
 * @param [in] Status: key status struct
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Key status retrieved successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_GetKeyStatus(const HSM_KeyStatusType *Status);

/**
 * @brief Host image upgrade.
 * @note  Function ID: DES_HSM_API_034
 * @param[in] HostUpgradePara: Pointer to host image upgrade parameter
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Host image upgrade successful
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_HostImageSecureUpgrade(const HSM_SecureUpgradeType *HostUpgradePara);

/**
 * @brief Host image uggrade verify.
 * @note  Function ID: DES_HSM_API_035
 * @param[in] HostVerifyPara: Pointer to host image verify parameter
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Host image verification successful
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_HostImageSecureVerify(const HSM_ImageVerifyType *HostVerifyPara);
#endif

/**
 * @brief Get HSM firmware version.
 * @note  Function ID: DES_HSM_API_036
 * @param[out] Version HSM firmware version
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  HSM firmware version retrieved successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_GetHsmFwVersion(const uint32 *Version);

#if !defined(AUTOSAR_MCAL)
/**
 * @brief read otp
 * @note  Function ID: DES_HSM_API_037
 * @param [in] OtpAddr: otp address value
 * @param [in] BytesLen: read bytes len
 * @param [out] Data: Secureboot Config
 * @return     Hal_StautsType
 * @retval     STATUS_SUCCESS                  OTP read successful
 * @retval     HSM_WRONG_OTP_READ_ERROR       Operation failed
 */
Hal_StatusType HSM_Hal_OtpRead(uint32 OtpAddr, uint32 BytesLen, uint8 *Data);

/**
 * @brief write otp
 * @note  Function ID: DES_HSM_API_038
 * @param [in] OtpAddr: otp address value
 * @param [in] BytesLen: read bytes len
 * @param [out] Data: Secureboot Config
 * @return     uint32
 * @retval     STATUS_SUCCESS                  OTP write successful
 * @retval     HSM_WRONG_OTP_WRITE_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_OtpWrite(uint32 OtpAddr, uint32 BytesLen, uint8 *Data);

/**
 * @brief only reset hsm hardware and firmware.
 * @note  Function ID: DES_HSM_API_039
 * @return none
 */
void HSM_Hal_ResetHsm(void);

/**
 * @brief Get LifeCycle by read HSM STATUS Register
 * @note  Function ID: DES_HSM_API_040
 * @return HSM_LifeCycleType: Current HSM life cycle type.
 */
HSM_LifeCycleType HSM_Hal_GetLifeCycleByReg(void);
#endif
/*PRQA S 3332 ++ # AMCE_HSM_3332*/
#if HSM_HAL_SUPPORT_OTP_INTERFACE
/*PRQA S 3332 -- # AMCE_HSM_3332*/
/**
 * @brief Otp Hw Ctrl, Set encrypt otp key algo
 * @note  Function ID: DES_HSM_API_041
 * @param[in] Algo: encrypt algo type.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  OTP key cipher algorithm set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_SetOtpKeyCipherAlgo(HSM_OtpKeyCipherAlgo Algo);

/**
 * @brief Otp Hw Ctrl, set image secure upgrade algo type
 * @note  Function ID: DES_HSM_API_042
 * @param[in] UpgradeAlgo: secure upgrade algo type
 * @param[in] VerifyAlgo: verify algo type
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  HSM OTP control set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_SetHsmOtpCtrl(HSM_OtpCtrlUpgradeAlgoType UpgradeAlgo, HSM_OtpCtrlVerifyAlgoType VerifyAlgo);

/**
 * @brief Otp Hw Ctrl, set image secure verify algo type
 * @note  Function ID: DES_HSM_API_043
 * @param[in] UpgradeAlgo: secure upgrade algo type
 * @param[in] VerifyAlgo: verify algo type
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  Host OTP control set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_SetHostOtpCtrl(HSM_OtpCtrlUpgradeAlgoType UpgradeAlgo, HSM_OtpCtrlVerifyAlgoType VerifyAlgo);

/**
 * @brief set external key or hash value(raw data key) to otp key slot
 * @note  Function ID: DES_HSM_API_044
 * @param[in] KeyId: otp key id.
 * @param[in] ExternKey: when KeyAttr is SYM_ATTR, is key raw data; when KeyAttr is ASYM_ATTR, the key hash value.
 * @param[in] ExternKeyLen: the key data or hash value length.
 * @param[in] KeyLevel: otp key level.
 * @param[in] KeyAttr: otp key attribute.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  OTP external key set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_SetOtpExternalKey(HSM_OtpKeyId KeyId, const uint8 *ExternKey, uint32 ExternKeyLen,
                    uint32 KeyAttr);

/**
 * @brief set external key or hash value(raw data key) to otp key slot
 * @note  Function ID: DES_HSM_API_045
 * @param[in] KeyId: otp key id. only support:
 * @param[in] ExternKey: when KeyType is SYM_ATTR, is key raw data; when KeyAttr is ASYM_ATTR, the key hash value.
 * @param[in] KeyType: extern otp key type.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  OTP external key set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_SetOtpExternalKeyByHsmFw(HSM_OtpKeyId KeyId, const uint8 *ExternKey,
                                                HSM_ExternOtpKeyType KeyType);

/**
 * @brief set random key to otp key slot
 * @note  Function ID: DES_HSM_API_046
 * @param[in] KeyId: otp key id.
 * @param[in] KeyLevel: otp key level.
 * @param[in] KeyType: support random key type.
 * @param[in] KeyAttr: otp key attribute.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  OTP random key set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_SetOtpRndKey(HSM_OtpKeyId KeyId, uint32 KeyAttr);

/**
 * @brief set random key to otp key slot in HSM_LIFE_CYCLE_DEV_MODE and HSM_LIFE_CYCLE_MANU_MODE.
 * @note  Function ID: DES_HSM_API_047
 * @param[in] KeyId: otp key id.
 * @return     Hal_StatusType
 * @retval     STATUS_SUCCESS                  OTP random key set successfully
 * @retval     STATUS_ERROR                    Operation failed
 */
Hal_StatusType HSM_Hal_SetOtpRndKeyByHsmFw(HSM_OtpKeyId KeyId);
#endif

#if !defined(AUTOSAR_MCAL)
/*!
 * @brief HSM image install command.
 *
 * @note  Function ID: DES_HSM_API_050
 * @param[in] InstallPtr: Pointer to HSM image install parameters.
 *      ProMode: 1: START, 2: UPDATE, 4: FINISH, 8: ONEPASS.
 *      ImageAddr:
 *          In ONEPASS mode, this is the host address of the complete image.
 *          In streaming mode, this is the image header in START, encrypted payload in UPDATE,
 *          and last encrypted payload (can be 0-size) in FINISH.
 *      ImageSize: Image size in bytes for current process stage.
 *      CtxAddr: Host address of streaming context (context buffer size should be 1.6 KBytes).
 *      CtxSize: Context size in bytes.
 * @return Hal_StatusType
 * @retval STATUS_SUCCESS  Installation command executed successfully.
 * @retval STATUS_ERROR    Invalid parameter or command execution failed.
 */
Hal_StatusType HSM_Hal_HsmImageSecureInstall(const HSM_HsmImageInstallType *InstallPtr);
/*!
 * @brief HSM image verify command.
 *
 * @note  Function ID: DES_HSM_API_051
 * @param[in] VerifyPtr: Pointer to HSM image verify parameters.
 *      ProMode: 1: START, 2: UPDATE, 4: FINISH, 8: ONEPASS.
 *      VerifyType: 0x00 for image verification, 0x03 for secure boot verification.
 *      ImageAddr:
 *          In ONEPASS mode, this is the host address of the complete image.
 *          In streaming mode, this is the image header in START, encrypted payload in UPDATE,
 *          and last encrypted payload (can be 0-size) in FINISH.
 *      ImageSize: Image size in bytes for current process stage.
 *      CtxAddr: Host address of streaming context.
 *      CtxSize: Context size in bytes.
 * @return Hal_StatusType
 * @retval STATUS_SUCCESS  Verify command executed successfully.
 * @retval STATUS_ERROR    Invalid parameter or command execution failed.
 */
Hal_StatusType HSM_Hal_HsmImageSecureVerify(const HSM_HsmImageVerifyType *VerifyPtr);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* HSM_DRV_H */

/* =============================================  EOF  ============================================== */
