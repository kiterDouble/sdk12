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
 * @file CSE_Hal.h
 *
 * @brief This file provides CSE integration functions interface.
 *
 */

#ifndef CSE_HAL_H
#define CSE_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"
#include "System_AC784xx.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/*!
 * @brief Specify the KeyID to be used to implement the requested cryptographic operation.
 */
typedef enum
{
    CSE_SECRET_KEY = 0x0U, /*!< secret key */
    CSE_MASTER_ECU, /*!< master key for authorization key to use */
    CSE_BOOT_MAC_KEY, /*!< boot mac key for calculate boot mac value*/
    CSE_BOOT_MAC, /*!< store boot mac value */
    CSE_KEY_1, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_2, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_3, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_4, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_5, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_6, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_7, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_8, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_9, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_10, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_RAM_KEY = 0xFU, /*!< ram key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_11 = 0x14U, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_12, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_13, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_14, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_15, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_16, /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
    CSE_KEY_17 /*!< user key for encryption/decryption/generateMac/verifyMAC etc */
} Cse_KeyId;

/*!
 * @brief Specifies the boot type for the BOOT_DEFINE command.
 */
typedef enum
{
    CSE_BOOT_STRICT, /*!< strict secure boot mode */
    CSE_BOOT_SERIAL, /*!< serial secure boot mode */
    CSE_BOOT_PARALLEL, /*!< parallel secure boot mode */
    CSE_BOOT_NODEFINE, /*!< not boot define master key not init yet or non-cse enable part */
} Cse_BootMode;

/*!
 * @brief secure boot status.
 */
typedef enum
{
    CSE_BOOT_OK, /*!< secure boot success status */
    CSE_BOOT_FAILURE, /*!< secure boot fail status */
} Cse_BootStatus;

/*!
 * @brief the attributes for Keys.
 */
typedef enum
{
    KEY_ATTR_NONE = 0x00U, /*!< the key empty attribute */
    KEY_ATTR_VERIFY_ONLY = 0x01U, /*!< the key verify only attribute*/
    KEY_ATTR_WILDCARD = 0x02U, /*!< the key wildcard attribute*/
    KEY_ATTR_KEY_USAGE = 0x04U, /*!< the key key usage attribute */
    KEY_ATTR_DEBUG_PROTECT = 0x08U, /*!< the key debug protect attribute */
    KEY_ATTR_BOOT_PROTECT = 0x10U, /*!< the key boot protect attribute */
    KEY_ATTR_WRITE_PROTECT = 0x20U, /*!< the key write protect attribute */
} Cse_KeyAttr;

#if defined(AC7842X)
#define MATCH_KEYATTR(x) ((x) << 1)
#else
#define MATCH_KEYATTR(x) (x)
#endif
/*!
 * @brief Cse message input/output.
 */
typedef struct
{
    const uint8 *InputPtr; /*!< pointer input buffer.*/
    uint32 InputLength; /*!< Input length, align 16 bytes.*/
    uint8 *OutputPtr; /*!< Output buffer.*/
    uint32 OutputLength; /*!< Output buffer length, align 16 bytes.*/
    uint8 *Iv; /*!< the IV of the command in execution (using CBC mode) */
    const uint8 *SecondaryInputPtr; /*!< Secondary input buffer for CMAC verify.*/
    uint32 SecondaryInputLength; /*!< Secondary input buffer length.*/
    boolean *VerifyStatus; /*!< the verify mac status result */
} Cse_InputOutputType;

#ifdef HSM_SUPPORT_SW_RSA
typedef uint32 HSM_KeyId;
#define HSM_FLASH_KEY_RSA_0      30U      /**< Flash RSA key 0 */
#define HSM_RAM_KEY_RSA_0        54U      /**< RAM RSA key 0 */

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
 * @brief Specifies the asymmetric algorithm type.
 * @note DES ID: DES_HSM_TYP_026
 */
typedef uint32 HSM_AsymAlgoType;
#define HSM_ASYM_SM2               0U /**< SM2 asymmetric algorithm */
#define HSM_ASYM_ECDSA             1U /**< ECDSA asymmetric algorithm */
#define HSM_ASYM_RSA               2U /**< RSA asymmetric algorithm */
#define HSM_ASYM_RSA_CIPHER_1024   3U /**< RSA 1024 asymmetric algorithm only for dec or enc flow */
#define HSM_ASYM_RSA_CIPHER_2048   4U /**< RSA 2048 asymmetric algorithm only for dec or enc flow*/

/**
 * @brief Specifies the generate/verify signature value request type.
 * @note DES ID: DES_HSM_TYP_013
 */
typedef uint32 HSM_SignDirection;
#define HSM_SIGN    0U /**< Generate signature request */
#define HSM_VERIFY  1U /**< Verify signature request */

/**
 * @brief Specifies the boot type for the BOOT_DEFINE command.
 * @note DES ID: DES_HSM_TYP_011
 */
typedef uint8 HSM_CipherDirection;
#define HSM_ENCRYPTION   0U /**< Encrypt request */
#define HSM_DECRYPTION   1U /**< Decrypt request */

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

/**
 * @brief The set plain key config structure.
 * @note DES ID: DES_HSM_TYP_045
 */
typedef struct
{
    uint8 *PrivKey; /**< Pointer to the buffer, the private key store in buffer */
    uint32 PrivKeyLen; /**< Private key length */
    uint8 *PubKey; /**< Pointer to the buffer, the public key store in buffer */
    uint32 PubKeyLen; /**< Public key length */
} HSM_PlainKeyCfgType;

/**
 * @brief Specifies the asymmetric algorithm config structure.
 * @note DES ID: DES_HSM_TYP_023
 */
typedef struct
{
    HSM_KeyId KeyId; /**< Key index */
	HSM_PaddingType Padding; /**< Data padding */
	HSM_CipherDirection CipherDir; /**< Request operation, e.g. decrypt/encrypt */
    HSM_SignDirection SignDir; /**< Request operation, e.g. sign/verify */
	HSM_AsymAlgoType AsymAlgo; /**< Asymmetric algorithm type, e.g. ECC/RSA */
} HSM_AsymCfgType;
#endif

/*!
 * @brief Key info content, eg. M1~M5.
 */
typedef struct
{
    uint8 *M1; /*!< M1: pointer to the 128-bit M1 message buffer.*/
    uint8 *M2; /*!< M2: pointer to the 256-bit M2 message buffer.*/
    uint8 *M3; /*!< M3: pointer to the 128-bit M3 message buffer.*/
    uint8 *M4; /*!< M4: pointer to the 256-bit M4 message buffer.*/
    uint8 *M5; /*!< M5: pointer to the 128-bit M5 message buffer.*/
} Cse_SheKeyInfoType;

/*!
 * @brief Input Key info content.
 */
/*PRQA S 3630 ++ #AMCE_HSM_3630*/
typedef struct
{
    uint8 *AuthKey; /*!< authorization key.*/
    Cse_KeyId AuthKeyId; /*!< authorization key id.*/
    uint8 *NewKey; /*!< the new key will update to slot.*/
    Cse_KeyId NewKeyId; /*!< the new key id.*/
    Cse_KeyAttr NewKeyAttr; /*!< the new key Attr.*/
    uint32 Count; /*!< the key to be update count value.*/
} Cse_InputKeyInfoType;
/*PRQA S 3630 -- #AMCE_HSM_3630*/
/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/*!
 * @brief Initialize CSE module
 * @note  Function ID:  DES_CSE_API_001
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_Init(void);

/*!
 * @brief Uninitialize CSE module
 * @note  Function ID:  DES_CSE_API_002
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
#ifndef CSE_SDK_NON_EXTENDED_API
Hal_StatusType CSE_Hal_Deinit(void);
#endif

#ifdef HSM_SUPPORT_SW_RSA
Hal_StatusType HSM_Hal_RsaSign(const HSM_AsymCfgType *CfgPtr, const HSM_InOutSignType *InOutPtr,
                                      HSM_ProcessMode ProMode);
Hal_StatusType HSM_Hal_InstallRsaKey(const HSM_PlainKeyCfgType *CfgPtr);
Hal_StatusType HSM_Hal_Hash(const HSM_HashAlgoType Algo, const HSM_InOutType *InOutPtr,
        boolean Async, HSM_ProcessMode ProMode);
Hal_StatusType HSM_Hal_RsaCipher(const HSM_AsymCfgType *CfgPtr, const HSM_InOutType *InOutPtr,
                                         HSM_ProcessMode ProMode);
#endif

/*!
 * @brief Performs the AES-128 encryption in ECB mode of the input plain text with the Key ID and returns the cipher text.
 * @note  Function ID:  DES_CSE_API_003
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_EncryptECB(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs);

/*!
 * @brief Performs the AES-128 decryption in ECB mode of the input cipher text with the Key ID and returns the plain text.
 * @note  Function ID:  DES_CSE_API_004
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_DecryptECB(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs);

/*!
 * @brief Performs the AES-128 encryption in CBC mode of the input plain text with the Key ID and returns the cipher text.
 * @note  Function ID:  DES_CSE_API_005
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_EncryptCBC(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs);

/*!
 * @brief Performs the AES-128 decryption in CBC mode of the input cipher text with the Key ID and returns the plain text.
 * @note  Function ID:  DES_CSE_API_006
 * @param [in] Ptr: a structure contatining input and output. eg.plaintext/ciphertext.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_DecryptCBC(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs);

/*!
 * @brief Generate the MAC of a given message using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_007
 * @param [in] Ptr: a structure contatining input and output. eg.message/cmac.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_GenerateMAC(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs);

/*!
 * @brief Verifies the MAC of a given message using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_008
 * @param [in] Ptr: a structure contatining input and output. eg.message/cmac.
 * @param [in] KeyId: the index of the key used in cse command.
 * @param [in] TimeoutUs: timeout value for sync flow.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_VerifyMAC(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId, uint32 TimeoutUs);

/*!
 * @brief Exports the RAM_KEY into a format protected by SECRET_KEY.
 * @note  Function ID:  DES_CSE_API_010
 * @param [out] Ptr: store M1~M5 value.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_GetRamKey(const Cse_SheKeyInfoType *Ptr);

/*!
 * @brief Extends the seed of the PRNG.
 * @note  Function ID:  DES_CSE_API_011
 * @param [out] Rnd: pointer to a 128-bit buffer containing the PRNG value
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_GenerateRnd(uint8 *Rnd);

/*!
 * @brief Generate the MAC of a given message(locate in FLASH) using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_012
 * @param [in] Ptr: a structure contatining input and output, Message and Addr must be 16bytes align. eg.message/cmac.
 * @param [in] KeyId: the index of the key used in cse command.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_GenerateMACAddrMode(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId);

/*!
 * @brief Verifies the MAC of a given message(locate in FLASH) using CMAC with AES-128.
 * @note  Function ID:  DES_CSE_API_013
 * @param [in] Ptr: a structure contatining input and output. eg.message/cmac.
 * @param [in] KeyId: the index of the key used in cse command.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
#ifndef CSE_SDK_NON_EXTENDED_API
Hal_StatusType CSE_Hal_VerifyMACAddrMode(const Cse_InputOutputType *Ptr, Cse_KeyId KeyId);

/*!
 * @brief Extends the seed of the PRNG.
 * @note  Function ID:  DES_CSE_API_014
 * @param [in] Entropy: pointer to a 128-bit buffer containing the entropy
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_ExtendRNGSeed(const uint8 *Entropy);
#endif

/*!
 * @brief Get the UID from CSE module.
 * @note  Function ID:  DES_CSE_API_015
 * @param [in] Challenge: pointer to the 128-bit buffer containing challenge data
 * @param [out] Uid: pointer to 120 bit buffer containing UID data
 * @param [out] Sreg: Value of the status register.
 * @param [out] Mac: pointer to the 128 bit buffer containing MAC data
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_GetID(const uint8 *Challenge, uint8 *Uid, uint8 *Sreg, uint8 *Mac);

/*!
 * @brief Compresses the given messages.
 * @note  Function ID:  DES_CSE_API_016
 * @param [in] Msg: pointer to the messages which will be compressed
 * @param [in] MsgLen: number in bits of message
 * @param [out] MpCompress: pointer to the 128 bit buffer storing the compressed data
 * @param [in] TimeoutUs: timeout value in milliseconds
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_MPCompress(const uint8 *Msg, uint16 MsgLen, uint8 *MpCompress, uint32 TimeoutUs);

/*!
 * @brief Mark failure/success boot verification.
 * @note  Function ID:  DES_CSE_API_017
 * @param [in] Status: secure boot flow status
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_SetSecureBootStatus(Cse_BootStatus Status);

/*!
 * @brief Define bootcode size and secure boot type.
 * @note  Function ID:  DES_CSE_API_018
 * @param [in] BootSize: number of blocks of 128-bit data to check on boot
 * @param [in] BootMode: secure boot type
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_SetSecureBootMode(uint32 BootSize, Cse_BootMode BootMode);

/*!
 * @brief Cancels a previously launched asynchronous command
 * @note  Function ID:  DES_CSE_API_019
 * @return void
 */
void CSE_Hal_CancelCommand(void);

/*!
 * @brief Installs a callback function for CSE driver.
 * @note  Function ID:  DES_CSE_API_020
 * @param [in] Func: The function to be invoked
 * @param [in] Args: The parameter to be passed to the callback function
 * @return none
 */
void CSE_Hal_InstallCallback(const Hal_CallbackType Func, void *Args);

/*!
 * @brief Asynchronously get operation status.
 * @note  Function ID:  DES_CSE_API_021
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
#ifndef CSE_SDK_NON_EXTENDED_API
Hal_StatusType CSE_Hal_GetAsyncCmdStatus(void);
#endif

/*!
 * @brief Erase all keys stored in the CSE.
 * @note  Function ID:  DES_CSE_API_022
 * @param [in] MasterKey: Master Key in chip.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_ResetKey(const uint8 *MasterKey);

/*!
 * @brief Updates an internal key to CSE module.
 * @note  Function ID:  DES_CSE_API_023
 * @param [in] KeyId: the key to be updated
 * @param [in] InPtr: store M1~M5 value.
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_LoadKey(Cse_KeyId KeyId, const Cse_SheKeyInfoType *InPtr);

/*!
 * @brief Updates the RAM key memory slot with a 128-bit plaintext.
 * @note  Function ID:  DES_CSE_API_024
 * @param [in] PlainKey: pointer to the 128-bit plain text
 * @return operation status
 *         - STATUS_SUCCESS: operation was successful
 *         - Other Valule: operation was fail
 */
Hal_StatusType CSE_Hal_LoadPlainKey(const uint8 *PlainKey);

/*!
 * @brief Calculate M1 To M5.
 * @note  Function ID:  DES_CSE_API_044
 * @param [in] InPtr: the key content to be updated.
 * @param [out] OutPtr: store M1~M5 value.
 * @return Hal_StatusType
 */
Hal_StatusType CSE_Hal_CalcM1ToM5(const Cse_InputKeyInfoType *InPtr, const Cse_SheKeyInfoType *OutPtr);

#ifndef CSE_SDK_NON_EXTENDED_API
/*!
 * @brief Returns the content of the status register.
 * @return Value of the status register
 */
uint32 CSE_Hal_GetStatus(void);

/*!
 * @brief Returns the content of the CSESTAT BOK bit value.
 * @return Value of the status register
 */
uint32 CSE_Hal_GetBOKStatus(void);

/*!
 * @brief Returns the content of the CSESTAT BFN bit value.
 * @return Value of the status register
 */
uint32 CSE_Hal_GetBFNStatus(void);

/*!
 * @brief Returns the content of the CSESTAT BIN bit value.
 * @return Value of the status register
 */
uint32 CSE_Hal_GetBINStatus(void);

/*!
 * @brief Returns the content of the CSESTAT SB bit value.
 * @return Value of the status register
 */
uint32 CSE_Hal_GetSBStatus(void);

/*!
 * @brief Returns the content of the CSESTAT BSY bit value.
 *
 * @param[in] none
 * @return Value of the status register
 */
uint32 CSE_Hal_GetBSYStatus(void);
#endif
#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif /* _CSE_Hal_H */

/* =============================================  EOF  ============================================== */
