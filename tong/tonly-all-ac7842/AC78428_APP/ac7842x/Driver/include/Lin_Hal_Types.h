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
 * @file Lin_Hal_Types.h
 *
 * @brief This file provides lin module used hardware Types.
 */

#ifndef LIN_HAL_TYPES_H
#define LIN_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/
/**
 * @def LIN_MAKE_PARITY
 * @brief Make parity for PID.
 * @note DES ID: DES_LIN_MACRO_008
 */
#define LIN_MAKE_PARITY         (0U)

/**
 * @def LIN_CHECK_PARITY
 * @brief Check parity for PID.
 * @note DES ID: DES_LIN_MACRO_009
 */
#define LIN_CHECK_PARITY        (1U)

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/
/**
 * @brief LIN node mode type.
 * @note DES ID: DES_LIN_TYP_002
 */
typedef enum
{
    LIN_MASTER = 0U,                     /*!< Set the node type as MASTER */
    LIN_SLAVE = 1U                       /*!< Set the node type as SLAVE */
} Lin_ModeType;

/**
 * @brief LIN break length for LIN master transmission.
 * @note DES ID: DES_LIN_TYP_003
 */
typedef enum
{
    BREAK_LENGTH_13BIT = 0U,             /*!< Break length 13 bits */
    BREAK_LENGTH_14BIT,                  /*!< Break length 14 bits */
    BREAK_LENGTH_15BIT,                  /*!< Break length 15 bits */
    BREAK_LENGTH_16BIT,                  /*!< Break length 16 bits */
    BREAK_LENGTH_17BIT,                  /*!< Break length 17 bits */
    BREAK_LENGTH_18BIT,                  /*!< Break length 18 bits */
    BREAK_LENGTH_19BIT,                  /*!< Break length 19 bits */
    BREAK_LENGTH_20BIT,                  /*!< Break length 20 bits */
    BREAK_LENGTH_21BIT,                  /*!< Break length 21 bits */
    BREAK_LENGTH_22BIT,                  /*!< Break length 22 bits */
    BREAK_LENGTH_23BIT,                  /*!< Break length 23 bits */
    BREAK_LENGTH_24BIT,                  /*!< Break length 24 bits */
    BREAK_LENGTH_25BIT,                  /*!< Break length 25 bits */
    BREAK_LENGTH_26BIT,                  /*!< Break length 26 bits */
    BREAK_LENGTH_27BIT,                  /*!< Break length 27 bits */
    BREAK_LENGTH_28BIT                   /*!< Break length 28 bits */
} Lin_BreakLengthType;

/**
 * @brief LIN break threshold for LIN slave detection.
 * @note DES ID: DES_LIN_TYP_004
 */
typedef enum
{
    BREAK_THRESHOLD_10BIT = 0U,          /*!< 10-bit break length threshold */
    BREAK_THRESHOLD_11BIT = 1U           /*!< 11-bit break length threshold */
} Lin_BreakThresholdType;

/**
 * @brief LIN event identifier enumeration.
 * @note DES ID: DES_LIN_TYP_005
 */
typedef enum
{
    LIN_NO_EVENT = 0x00U,                /*!< No event occurred */
    LIN_WAKEUP_SIGNAL = 0x01U,           /*!< Received a wakeup signal */
    LIN_SYNC_OK = 0x02U,                 /*!< Sync byte is correct */
    LIN_SYNC_ERROR = 0x03U,              /*!< Sync byte error detected */
    LIN_PID_OK = 0x04U,                  /*!< PID is correct */
    LIN_PID_ERROR = 0x05U,               /*!< PID error detected */
    LIN_FRAME_ERROR = 0x06U,             /*!< Frame error detected */
    LIN_READBACK_ERROR = 0x07U,          /*!< Readback data error detected */
    LIN_CHECKSUM_ERROR = 0x08U,          /*!< Checksum error detected */
    LIN_TX_COMPLETED = 0x09U,            /*!< Transmission completed */
    LIN_RX_COMPLETED = 0x0AU,            /*!< Reception completed */
    LIN_RX_OVERRUN = 0x0BU,              /*!< Receive overrun detected */
    LIN_NOISE_ERROR = 0x0CU,             /*!< Noise error detected */
    LIN_TIMEOUT_ERROR = 0x0DU            /*!< Timeout error detected */
} Lin_EventIdType;

/**
 * @brief LIN node state enumeration.
 * @note DES ID: DES_LIN_TYP_006
 */
typedef enum
{
    LIN_NODE_STATE_UNINIT = 0x00U,                /*!< Uninitialized state */
    LIN_NODE_STATE_SLEEP_MODE = 0x01U,            /*!< Sleep mode state */
    LIN_NODE_STATE_IDLE = 0x02U,                  /*!< Idle state */
    LIN_NODE_STATE_SEND_BREAK_FIELD = 0x03U,      /*!< Sending break field state */
    LIN_NODE_STATE_SEND_SYNC = 0x04U,             /*!< Sending sync byte state */
    LIN_NODE_STATE_RECV_SYNC = 0x05U,             /*!< Receiving sync byte state */
    LIN_NODE_STATE_SEND_PID = 0x06U,              /*!< Sending PID state */
    LIN_NODE_STATE_RECV_PID = 0x07U,              /*!< Receiving PID state */
    LIN_NODE_STATE_RECV_DATA = 0x08U,             /*!< Receiving data state */
    LIN_NODE_STATE_RECV_DATA_COMPLETED = 0x09U,   /*!< Reception completed state */
    LIN_NODE_STATE_SEND_DATA = 0x0AU,             /*!< Sending data state */
    LIN_NODE_STATE_SEND_DATA_COMPLETED = 0x0BU    /*!< Transmission completed state */
} Lin_NodeStateType;

/**
 * @brief LIN frame checksum model type.
 * @note DES ID: DES_LIN_TYP_007
 */
typedef enum
{
    LIN_ENHANCED_CS,                     /*!< Enhanced checksum mode */
    LIN_CLASSIC_CS                       /*!< Classic checksum mode */
} Lin_FrameCsModelType;

/**
 * @brief LIN frame response type.
 * @note DES ID: DES_LIN_TYP_008
 */
typedef enum
{
    LIN_FRAMERESPONSE_TX,                /*!< Response is generated from this node */
    LIN_FRAMERESPONSE_RX,                /*!< Response is generated from another node and is relevant for this node */
    LIN_FRAMERESPONSE_IGNORE             /*!< Response is generated from one slave to another slave */
} Lin_FrameResponseType;

/**
 * @brief LIN transfer status enumeration.
 * @note DES ID: DES_LIN_TYP_009
 */
typedef enum
{
    LIN_NOT_OK = 0,                      /*!< Development or production error occurred */
    LIN_TX_OK,                           /*!< Successful transmission */
    LIN_TX_BUSY,                         /*!< Ongoing transmission (Header or Response) */
    LIN_TX_HEADER_ERROR,                 /*!< Erroneous header transmission such as mismatch between sent and read back data, identifier parity error, or physical bus error */
    LIN_TX_ERROR,                        /*!< Erroneous transmission such as mismatch between sent and read back data or physical bus error */
    LIN_RX_OK,                           /*!< Reception of correct response */
    LIN_RX_BUSY,                         /*!< Ongoing reception: at least one response byte has been received, but the checksum byte has not been received */
    LIN_RX_ERROR,                        /*!< Erroneous reception such as framing error, overrun error, or checksum error */
    LIN_RX_NO_RESPONSE,                  /*!< No response byte has been received so far */
    LIN_OPERATIONAL,                     /*!< Normal operation: the related LIN channel is ready to transmit next header, no data from previous frame available */
    LIN_CH_SLEEP                         /*!< Sleep mode operation: wake-up detection from slave nodes is enabled */
} Lin_StatusType;

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/**
 * @brief LIN channel information structure.
 * @note DES ID: DES_LIN_TYP_010
 */
typedef struct
{
    uint8 *RxBuff;                       /*!< Pointer to the buffer of received data */
    uint8 CurrentPid;                    /*!< Current protected identifier */
    Lin_EventIdType CurrentEventId;      /*!< Current event identifier */
} Lin_ChannelInfoType;

/**
 * @brief LIN callback function pointer type.
 * @note DES ID: DES_LIN_TYP_011
 */
typedef void (* Lin_CallbackType)(const uint8 Instance, const Lin_ChannelInfoType *LinInfo);

/**
 * @brief LIN channel configuration structure.
 * @note DES ID: DES_LIN_TYP_012
 */
typedef struct
{
    uint32 BaudRate;                     /*!< LIN baud rate value */
    boolean AutoBaudEnable;              /*!< Auto-baud function enable flag */
    Lin_ModeType ModeType;               /*!< Node mode: Master or Slave */
    Lin_BreakLengthType BreakLength;     /*!< LIN break length for master node */
    Lin_BreakThresholdType BreakThreshold; /*!< LIN break detection threshold for slave node */
    Lin_CallbackType Callback;           /*!< Callback function pointer */
} Lin_ChannelConfigType;

/**
 * @brief LIN Protocol Data Unit structure.
 * @note DES ID: DES_LIN_TYP_013
 */
typedef struct
{
    uint8 Pid;                           /*!< LIN frame protected identifier */
    Lin_FrameCsModelType Cs;             /*!< Checksum model type */
    Lin_FrameResponseType Drc;           /*!< Data response configuration */
    uint8 Dl;                            /*!< Data length in bytes */
    uint8 *SduPtr;                       /*!< Pointer to Service Data Unit */
} Lin_PduType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

#ifdef __cplusplus
}
#endif
#endif
/*============================================EOF===================================================*/
