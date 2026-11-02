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

/*!
 * @file Device_Types.h
 *
 * @brief This file provides all device type macros, structures and enums.
 *
 */

#ifndef DEVICE_TYPES_H
#define DEVICE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* ===========================================  INCLUDE FILES  =========================================== */
#include "Std_Types.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/**
* @brief setting bits macro.
*/
#ifndef BIT_SHIFT
#define BIT_SHIFT(bit)              ((uint32)1U << (bit))
#endif

/**
* @brief setting bits macro.
*/
#ifndef SET_BIT32
#define SET_BIT32(reg, mask)              ((reg) |= (uint32)(mask))
#endif

/**
* @brief clearing bits macro.
*/
#ifndef CLEAR_BIT32
#define CLEAR_BIT32(reg, mask)            ((reg) &= (~((uint32)(mask))))
#endif

/**
* @brief read bits macro.
*/
#ifndef READ_BIT32
#define READ_BIT32(reg, mask)             ((reg) & ((uint32)(mask)))
#endif

/**
* @brief extract bits macro.
*/
#ifndef EXTRACT_BIT32
#define EXTRACT_BIT32(reg, mask, pos)     (((reg) & ((uint32)(mask))) >> (pos))
#endif

/**
* @brief write register macro.
*/
#ifndef WRITE_REG32
#define WRITE_REG32(reg, value)           ((reg) = (uint32)(value))
#endif

/**
* @brief clear register macro.
*/
#ifndef CLEAR_REG32
#define CLEAR_REG32(reg)                  ((reg) = (uint32)(0U))
#endif

/**
* @brief read bit macro.
*/
#ifndef READ_BIT
#define READ_BIT(reg, bit)             (((reg) >> (bit)) & (uint32)1U)
#endif

/**
* @brief read register macro.
*/
#ifndef READ_REG32
#define READ_REG32(reg)                   ((reg))
#endif

/**
* @brief clear bits and set with new value
*/
#ifndef MODIFY_REG32
#define MODIFY_REG32(reg, mask, pos, value)  WRITE_REG32(((reg)), \
                ((READ_REG32((reg)) & (~((uint32)(mask)))) | ((((uint32)(value)) << (pos)) & ((uint32)(mask)))))
#endif

/**
* @brief read 32 bits memory macro.
*/
#ifndef READ_MEM32
#define READ_MEM32(address)               (*(volatile uint32 *)(address))
#endif

/**
* @brief write 32 bits memory macro.
*/
#ifndef WRITE_MEM32
#define WRITE_MEM32(address, value)       ((*(volatile uint32 *)(address))= (uint32)(value))
#endif

/**
* @brief clear bits and set with new value for memory.
*/
#ifndef MODIFY_MEM32
#define MODIFY_MEM32(address, mask, pos, value)   WRITE_MEM32((address), \
               ((READ_MEM32(address) & (~((uint32)(mask)))) | ((((uint32)(value)) << (pos)) & ((uint32)(mask)))))
#endif

/**
* @brief Interrupt handler function helper macro .
*/
#ifndef ISR
#define ISR(IsrName)        INTERRUPT_FUNC void IsrName(void)
#endif
/* ===========================================  TYPEDEFS  ============================================ */
/**
* @brief global enumeration.
*/
typedef void (*Hal_CallbackType)(void *Args);

typedef enum
{
    /* Generic error codes */
    STATUS_SUCCESS                         = 0x00U,    /*!< Generic operation success status */
    STATUS_ERROR                           = 0x01U,    /*!< Generic operation failure status */
    STATUS_BUSY                            = 0x02U,    /*!< Generic operation busy status */
    STATUS_TIMEOUT                         = 0x03U,    /*!< Generic operation timeout status */
    STATUS_UNSUPPORTED                     = 0x04U,    /*!< Generic operation unsupported status */
    STATUS_IDLE                            = 0x05U,    /*!< Generic operation idle status */
    STATUS_ABORT                           = 0x06U,    /*!< Generic operation abort status */
    STATUS_ADDRS_INVALID                   = 0x07U,    /*!< Generic operation addrs invalid status */
    STATUS_CONFIG                          = 0x08U,    /*!< Generic operation configed status */
    /* MCU specific error codes */
    STATUS_MCU_GATED_OFF                   = 0x10U,    /*!< Module is gated off */
    STATUS_MCU_TRANSITION_FAILED           = 0x11U,    /*!< Error occurs during transition. */
    STATUS_MCU_INVALID_STATE               = 0x12U,    /*!< Unsupported in current state. */
    STATUS_MCU_NOTIFY_BEFORE_ERROR         = 0x13U,    /*!< Error occurs during send "BEFORE" notification. */
    STATUS_MCU_NOTIFY_AFTER_ERROR          = 0x14U,    /*!< Error occurs during send "AFTER" notification. */
    STATUS_CLK_ON                          = 0x15U,    /*!< Clock is enabled state*/
    STATUS_CLK_OFF                         = 0x16U,    /*!< Clock is disabled state */
    STATUS_CLK_UNSTABLE                    = 0x17U,    /*!< Clock is stabled state */
    STATUS_CLK_STABLE                      = 0x18U,    /*!< Clock is distabled state */
    /* I2C specific error codes */
    STATUS_I2C_RECEIVED_NACK               = 0x20U,    /*!< NACK signal received  */
    STATUS_I2C_TX_UNDERRUN                 = 0x21U,    /*!< TX underrun error */
    STATUS_I2C_RX_OVERRUN                  = 0x22U,    /*!< RX overrun error */
    STATUS_I2C_ARBITRATION_LOST            = 0x23U,    /*!< Arbitration lost */
    STATUS_I2C_ABORTED                     = 0x24U,    /*!< A transfer was aborted */
    STATUS_I2C_BUS_BUSY                    = 0x25U,    /*!< I2C bus is busy, cannot start transfer */
    /* Security specific error codes */
    STATUS_SEC_SEQUENCE_ERROR              = 0x32U,    /*!< The sequence of commands or subcommands is out of
                                                             sequence */
    STATUS_SEC_KEY_NOT_AVAILABLE           = 0x33U,    /*!< A key is locked due to failed boot measurement or
                                                             an active debugger */
    STATUS_SEC_KEY_INVALID                 = 0x34U,    /*!< A function is called to perform an operation with
                                                             a key that is not allowed for the given operation */
    STATUS_SEC_KEY_EMPTY                   = 0x35U,    /*!< Attempt to use a key that has not been initialized yet */
    STATUS_SEC_NO_SECURE_BOOT              = 0x36U,    /*!< The conditions for a secure boot process are not met */
    STATUS_SEC_KEY_WRITE_PROTECTED         = 0x37U,    /*!< Request for updating a write protected key slot,
                                                             or activating debugger with write protected key(s) */
    STATUS_SEC_KEY_UPDATE_ERROR            = 0x38U,    /*!< Key update did not succeed due to errors in
                                                             verification of the messages */
    STATUS_SEC_RNG_SEED                    = 0x39U,    /*!< Returned by CMD_RND and CMD_DEBUG if the seed has not
                                                             been initialized before */
    STATUS_SEC_NO_DEBUGGING                = 0x3AU,    /*!< DEBUG command authentication failed */
    STATUS_SEC_MEMORY_FAILURE              = 0x3CU,    /*!< General memory technology failure
                                                            (multibit ECC error, common fault detected) */
    STATUS_SEC_HSM_INTERNAL_MEMORY_ERROR   = 0x40U,    /*!< An internal memory error encountered while
                                                             executing the command */
    STATUS_SEC_INVALID_COMMAND             = 0x41U,    /*!< Command value out of range */
    STATUS_SEC_TRNG_ERROR                  = 0x42U,    /*!< One or more statistical tests run on the TRNG output failed */
    STATUS_SEC_HSM_FLASH_BLOCK_ERROR       = 0x43U,    /*!< Error reading, programming or erasing one of the HSM flash blocks */
    STATUS_SEC_INTERNAL_CMD_ERROR          = 0x44U,    /*!< An internal command processor error while executing a command */
    STATUS_SEC_MAC_LENGTH_ERROR            = 0x45U,    /*!< MAC/Message length out of range */
    STATUS_SEC_INVALID_ARG                 = 0x46U,    /*!< Invalid command argument */
    STATUS_SEC_TRNG_CLOCK_ERROR            = 0x47U,    /*!< TRNG not provided with a stable clock */
    /* SPI specific error codes */
    STATUS_SPI_TX_UNDERRUN                 = 0x50U,    /*!< TX underrun error */
    STATUS_SPI_RX_OVERRUN                  = 0x51U,    /*!< RX overrun error */
    STATUS_SPI_ABORTED                     = 0x52U,    /*!< A transfer was aborted */
    /* UART specific error codes */
    STATUS_UART_TX_UNDERRUN                = 0x60U,    /*!< TX underrun error */
    STATUS_UART_RX_OVERRUN                 = 0x61U,    /*!< RX overrun error */
    STATUS_UART_ABORTED                    = 0x62U,    /*!< A transfer was aborted */
    STATUS_UART_FRAMING_ERROR              = 0x63U,    /*!< Framing error */
    STATUS_UART_PARITY_ERROR               = 0x64U,    /*!< Parity error */
    STATUS_UART_BREAK_ERROR                = 0x65U,    /*!< Break error */
    STATUS_UART_NOISE_ERROR                = 0x66U,    /*!< Noise error */
    /* I2S specific error codes */
    STATUS_I2S_TX_UNDERRUN                 = 0x70U,    /*!< TX underrun error */
    STATUS_I2S_RX_OVERRUN                  = 0x71U,    /*!< RX overrun error */
    STATUS_I2S_ABORTED                     = 0x72U,    /*!< A transfer was aborted */
    /* SMU error codes */
    STATUS_SMU_FAULT_OCCURRED              = 0x80U,    /*!< Smu fault Occured */
    /*HSM error codes*/
    HSM_WRONG_KEY_HANDLE                   = 0x92U,    /*!< Given key handle is unknown or wrong (e.g.,not for this algorithm or this mode) */
    HSM_ALL_SESSIONS_OCCUPIED              = 0x93U,    /*!< No resources left for an additional parallel session (or no parallel processing at all) */
    HSM_ALGORITHM_ERROR                    = 0x94U,    /*!< Given algorithm or algorithm mode not available */
    HSM_WRONG_IV                           = 0x95U,    /*!< Given IV does not fit the given algorithm */
    HSM_AUTHORIZATION_FAILED               = 0x96U,    /*!< Given authorization value was wrong */
    HSM_WRONG_SESSION_HANDLE               = 0x97U,    /*!< Given session handle is unknown or wrong */
    HSM_WRONG_CHUNK_SIZE                   = 0x98U,    /*!< Given chunk size is wrong (cf. returns on initialization) */
    HSM_MAC_LENGTH_OVERSIZE                = 0x99U,    /*!< Given MAC length for verification is greater than MAC */
    HSM_WRONG_PARAM                        = 0x9aU,    /*!< Input parameter error */
    HSM_PRNG_REQUEST_OVERSIZE              = 0x9bU,    /*!< Requested number of random bytes exceeds PRNG limit */
    HSM_TRNG_SEED_FAILURE                  = 0x9cU,    /*!< PRNG was unable to retrieve true random seed from TRNG */
    HSM_ALL_COUNTERS_OCCUPIED              = 0x9dU,    /*!< No resources left to create an additional counter */
    HSM_UNKNOWN_COUNTER_ID                 = 0x9eU,    /*!< Given counter identifier is unknown */
    HSM_INVALID_COUNTER_INCREMENTATION     = 0x9fU,    /*!< Given counter incrementation is invalid (e.g., too large) */
    HSM_STATUS_TYPE_NOT_AVAILABLE          = 0xa0U,    /*!< Requested status type is (currently) not available for this module */
    HSM_KEK_AUTHID_FAIL                    = 0xa1U,    /*!< Hsm KEK auth id error */
    HSM_FLASH_ID_INVALID                   = 0xa2U,    /*!< Hsm flash id invalid */
    HSM_INVALID_KEY_SIZE                   = 0xa3U,    /*!< Given key size is invalid (e.g., too small or too large) */
    HSM_ALL_KEY_SPACE_OCCUPIED             = 0xa4U,    /*!< No resources left to create an additional key */
    HSM_INVALID_KEY_FLAG                   = 0xa5U,    /*!< Given key flag is invalid (e.g., wrong combination) */
    HSM_WRONG_REMOTE_KEY_HANDLE            = 0xa6U,    /*!< Given remote key handle is unknown or wrong(e.g., not for this algorithm or this mode) */
    HSM_WRONG_KEY_COMBINATION              = 0xa7U,    /*!< Keys do not fit the algorithm (e.g., RSA key vs. ECDH) */
    HSM_WRONG_AUTHORIZATION                = 0xa8U,    /*!< Given authorization structure does not fit key flags (e.g., authorization definition for a certain flag missing) */
    HSM_TRANSPORT_IMPOSSIBLE               = 0xa9U,    /*!< Given transport key is not a capable transport key or use flag cannot be transported or migrated */
    HSM_REMOVE_IMPOSSIBLE                  = 0xaaU,    /*!< Key is not allowed to become removed */
    HSM_INVALID_MSG_SIZE                   = 0xabU,    /*!< Given message imprint size is invalid (e.g., too small or too large) */
    HSM_CLOCK_NOT_SYNCHRONIZED             = 0xacU,    /*!< EVITA UTC clock is not synchronized yet */
    HSM_INVALID_TIME_STAMP                 = 0xadU,    /*!< Given time stamp could not be interpreted correctly */
    HSM_INVALID_UTC_TIME                   = 0xaeU,    /*!< Given UTC time could not be interpreted correctly */
    HSM_UTC_CHALLENGE_EXPIRED              = 0xafU,    /*!< Challenge was not requested or is expired already */
    HSM_UTC_SYNCHRONIZATION_FAILED         = 0xb0U,    /*!< Synchronization failed due to wrong signature or wrong challenge etc. */
    HSM_WRONG_CERT_KEY_HANDLE              = 0xb1U,    /*!< Given certification key handle is unknown or wrong (e.g., not enabled for signing) */
    HSM_WRONG_ALGO_TYPE                    = 0xb2U,    /*!< Given certification key handle is unknown or wrong (e.g., not enabled for signing) */
    HSM_WRONG_OTP_WRITE_ERROR              = 0xb3U,    /*!< Otp write error */
    HSM_WRONG_OTP_READ_ERROR               = 0xb4U,    /*!< Otp read error  */
    HSM_WRONG_GET_RND_KEY_ERROR            = 0xb5U,    /*!< Get random key error */
    HSM_WRONG_ENCRYPT_KEY_ERROR            = 0xb6U,    /*!< Encrypt Key error */
    HSM_FIRMWARE_UPDATE_FAIL               = 0xb7U,    /*!< Hsm Firmware update error */
    HSM_ECB_CBC_CFB128_MODE_FAIL           = 0xb8U,    /*!< Hsm ECB/CBC/CFB128 mode error */
    HSM_CFB8_OFB_CTR_MODE_FAIL             = 0xb9U,    /*!< Hsm CFB8/OFB/CTR mode error */
    HSM_CMAC_MODE_FAIL                     = 0xbaU,    /*!< Hsm CMAC mode error */
    HSM_AUTH_MODE_FAIL                     = 0xbbU,    /*!< Hsm AUTH mode error */
    HSM_RANDOM_LENGTH_FAIL                 = 0xbcU,    /*!< Hsm random length error */
    HSM_CPU_UNLOCK_DEBUG_FAIL              = 0xbdU,    /*!< Hsm cpu unlock debug error */
    HSM_SM4_AES256_ENABLE_FAIL             = 0xbeU,    /*!< Hsm sm4 and aes256 together enable error */

    /* Fls specific error codes */
    STATUS_FLS_RESET_AND_RECON_REQUIRED    = 0xc0U,    /*!< Flash swap page not enabled, MCU reset required and reconfigurate swap operation */
} Hal_StatusType;

/* ==========================================  LOCAL VARIABLES  =========================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DEVICE_TYPES_H */
/* =============================================  EOF  ============================================== */
