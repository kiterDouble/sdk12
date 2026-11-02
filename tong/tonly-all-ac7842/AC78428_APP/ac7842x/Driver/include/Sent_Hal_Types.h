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

#ifndef SENT_HAL_TYPES_H
#define SENT_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/
/**
 * @brief Define the sent channel ID.
 * @note DES ID: DES_SENT_TYP_010
 */
typedef enum
{
    SENT_CHANNEL_0 = 0x00U, /*!< channel 0 */
    SENT_CHANNEL_1 = 0x01U, /*!< channel 1 */
#if defined (AC7843X)
    SENT_CHANNEL_2 = 0x02U, /*!< channel 2 */
    SENT_CHANNEL_3 = 0x03U, /*!< channel 3 */
#endif
} Sent_ChannelIdType;

/**
 * @brief Define the sent frame check type.
 * @note DES ID: DES_SENT_TYP_011
 */
typedef enum
{
    SENT_FRAME_CHECK_PAST_SYNC_PULSE   = 0x00U, /*!< frame check against past sync pulse */
    SENT_FRAME_CHECK_FUTURE_SYNC_PULSE = 0x01U, /*!< frame check against future sync pulse */
} Sent_FrameCheckModeType;

/**
 * @brief Define the sent serial msg type.
 * @note DES ID: DES_SENT_TYP_012
 */
typedef enum
{
    SENT_SHORT_SERIAL_MSG    = 0x00U, /*!< short serial message type */
    SENT_ENHANCED_SERIAL_MSG = 0x01U, /*!< enhanced serial message type */
} Sent_SerialMsgType;

/**
 * @brief Define the sent channel state.
 * @note DES ID: DES_SENT_TYP_013
 */
typedef enum
{
    SENT_CHANNEL_STOP         = 0x00U, /*!< channel stopped */
    SENT_CHANNEL_INITIALIZED  = 0x01U, /*!< channel initialized, means configured and enabled */
    SENT_CHANNEL_RUNNING      = 0x02U, /*!< channel running, fallback status from synchronized */
    SENT_CHANNEL_SYNCHRONIZED = 0x03U, /*!< channel synchronized, frequency in range */
} Sent_ChannelStatusType;

/**
 * @brief Define the configuration bit type of enhanced serial msg.
 * @note DES ID: DES_SENT_TYP_014
 */
typedef enum
{
    SENT_ENHANCED_CONFIG_0 = 0x00U, /*!< enhanced serial message config bit equals to 0 */
    SENT_ENHANCED_CONFIG_1 = 0x01U, /*!< enhanced serial message config bit equals to 1 */
} Sent_EnhancedConfigType;

/**
 * @brief Define the nibble position type for data storage.
 * @note DES ID: DES_SENT_TYP_015
 */
typedef enum
{
    SENT_NIBBLE_POSITION_0 = 0x00U, /*!< store data nibble in position 0 */
    SENT_NIBBLE_POSITION_1 = 0x01U, /*!< store data nibble in position 1 */
    SENT_NIBBLE_POSITION_2 = 0x02U, /*!< store data nibble in position 2 */
    SENT_NIBBLE_POSITION_3 = 0x03U, /*!< store data nibble in position 3 */
    SENT_NIBBLE_POSITION_4 = 0x04U, /*!< store data nibble in position 4 */
    SENT_NIBBLE_POSITION_5 = 0x05U, /*!< store data nibble in position 5 */
    SENT_NIBBLE_POSITION_6 = 0x06U, /*!< store data nibble in position 6 */
    SENT_NIBBLE_POSITION_7 = 0x07U, /*!< store data nibble in position 7 */
} Sent_NibblePositonType;

/**
 * @brief Define the input source select.
 * @note DES ID: DES_SENT_TYP_016
 */
typedef enum
{
    SENT_INPUT_DATA_SELECT_0 = 0x00U, /*!< input data from source 0 */
    SENT_INPUT_DATA_SELECT_1 = 0x01U  /*!< input data from source 1 */
} Sent_InputDataSelectType;

/**
 * @brief Define the trigger source select.
 * @note DES ID: DES_SENT_TYP_017
 */
typedef enum
{
    SENT_EXTERNAL_TRIGGER_0 = 0x00U, /*!< external trigger 0 */
    SENT_EXTERNAL_TRIGGER_1 = 0x01U, /*!< external trigger 1 */
    SENT_EXTERNAL_TRIGGER_2 = 0x02U, /*!< external trigger 2 */
    SENT_EXTERNAL_TRIGGER_3 = 0x03U  /*!< external trigger 3 */
} Sent_ExternalTriggerType;

/**
 * @brief Define the spc time base.
 * @note DES ID: DES_SENT_TYP_018
 */
typedef enum
{
    SENT_LAST_SYNC = 0x00U, /*!< based on last received sync frequency */
    SENT_NOMINAL   = 0x01U  /*!< based on nominal frequency */
} Sent_SpcTimeBaseType;

/**
 * @brief Define the spc trigger mode.
 * @note DES ID: DES_SENT_TYP_019
 */
typedef enum
{
    SENT_SPC_TRIGGER_OFF          = 0x00U, /*!< no pulse generated */
    SENT_SPC_TRIGGER_IMMEDIATE    = 0x01U, /*!< pulse starts immediately */
    SENT_SPC_TRIGGER_FALLING_EDGE = 0x02U, /*!< pulse starts each time the
                                                first falling edge of calibartion pulse */
    SENT_SPC_TRIGGER_EXTERNAL     = 0x03U  /*!< pulse starts after external trigger */
} Sent_SpcTriggerModeType;

/**
 * @brief SENT channel interrupt event.
 * @note DES ID: DES_SENT_TYP_020
 */
enum
{
    SENT_EVENT_RECEIVE_SUCCESS                 = 0x1UL,    /*!< Rx CRC checked correct */
    SENT_EVENT_RECEIVE_DATA                    = 0x2UL,    /*!< Rx data nibble done */
    SENT_EVENT_RECEIVE_SERIAL_MSG              = 0x4UL,    /*!< Rx serial message done */
    SENT_EVENT_RECEIVE_FRAME_DONE              = 0x8UL,    /*!< Rx fast channel frame done including pause pulse */
    SENT_EVENT_RECEIVE_FIFO_HALF_FULL          = 0x10UL,   /*!< Rx fifo is half full */
    SENT_EVENT_TRANSMIT_DATA                   = 0x20UL,   /*!< Tx buffer done */
    SENT_EVENT_ERROR_RECEIVE_FIFO_OVERFLOW     = 0x40UL,   /*!< Rx fifo overflow */
    SENT_EVENT_ERROR_TRANSMIT_BUFFER_UNDERFLOW = 0x80UL,   /*!< Tx buffer underfow */
    SENT_EVENT_ERROR_FREQUENCY_RANGE           = 0x100UL,  /*!< frequncy range error */
    SENT_EVENT_ERROR_FREQUENCY_DRIFT           = 0x200UL,  /*!< frequncy drift error */
    SENT_EVENT_ERROR_NIBBLE_NUMBER             = 0x400UL,  /*!< nibble number error */
    SENT_EVENT_ERROR_NIBBLE_DATA               = 0x800UL,  /*!< nibble value error */
    SENT_EVENT_ERROR_NIBBLE_CRC                = 0x1000UL, /*!< crc nibble error */
    SENT_EVENT_ERROR_SERIAL_CRC                = 0x2000UL, /*!< serial crc error */
    SENT_EVENT_ERROR_SHORT_START_BIT           = 0x4000UL, /*!< short serial start bit error */
    SENT_EVENT_ERROR_ENHANCED_START_BIT        = 0x8000UL, /*!< enhanced serial start bit error */
    SENT_EVENT_ERROR_WATCH_DOG                 = 0x10000UL /*!< watchdog timeout error */
};

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/**
 * @brief Define the sent fifo status.
 * @note DES ID: DES_SENT_TYP_008
 */
typedef struct
{
    uint16 FrameDataFifoAvail;     /*!< fifo free space */
    boolean FrameDataFifoFull;     /*!< indicate fifo full */
    boolean FrameDataFifoEmpty;    /*!< indicate fifo empty */
    boolean FrameDataFifoHalfFull; /*!< indicate fifo half full */
} Sent_FifoStatusType;

/**
 * @brief Define the sent serial frame.
 * @note DES ID: DES_SENT_TYP_006
 */
typedef struct
{
    uint8 SerialMsgCRC;                /*!< Serial message CRC */
    uint8 SerialMsgID;                 /*!< Serial message ID */
    uint16 SerialMsgData;              /*!< Serial message data */
    Sent_EnhancedConfigType ConfigBit; /*!< Configuration bit */
} Sent_SerialFrameType;

/**
 * @brief Define the sent frame data.
 * @note DES ID: DES_SENT_TYP_007
 */
typedef struct
{
    uint32 Data;             /*!< frame data nibble value, if frame length is bigger
                                  than 8, then data represent data value for every 8 nibbles */
    uint32 TimeStamp;        /*!< module time stamp */
    uint32 TotalFrameLength; /*!< frame time length */
    uint8 StatusNibble;      /*!< frame status nibble value */
    uint8 FrameDataCRC;      /*!< frame data CRC */
} Sent_FrameDataType;

/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/**
 * @brief Define the sent message.
 * @note DES ID: DES_SENT_TYP_021
 */
typedef struct
{
    Sent_SerialFrameType SerialFrame; /*!< serial message frame */
    Sent_FrameDataType DataFrame;     /*!< fast message frame */
    Sent_FifoStatusType FifoStatus;   /*!< fifo status */
} Sent_MsgType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/

/**
 * @brief Callback for SENT module.
 * @note DES ID: DES_SENT_TYP_022
 */
typedef void (*Sent_CallbackType)(Sent_ChannelIdType ChannelId, uint32 Event);

/**
 * @brief Callback for SENT module fast data.
 * @note DES ID: DES_SENT_TYP_023
 */
typedef void (*Sent_RxCallbackType)(Sent_ChannelIdType ChannelId, uint32 Event, Sent_MsgType *Msg);

/**
 * @brief Define the sent channel io control config.
 * @note DES ID: DES_SENT_TYP_002
 */
typedef struct
{
    boolean InputInvertEnable;              /*!< channel input invert enable */
    boolean OutputInvertEnable;             /*!< channel output invert enable */
    uint8 GlitchFilterDepth;                /*!< gltich filter depth */
    Sent_InputDataSelectType Input;         /*!< pulse starts after external trigger */
    Sent_ExternalTriggerType TriggerSource; /*!< external trigger source select */
} Sent_IoCtrlType;

/**
 * @brief Define the sent nibble position config.
 * @note DES ID: DES_SENT_TYP_009
 */
typedef struct
{
    Sent_NibblePositonType Nibble0Position; /*!< nibble 0 Position */
    Sent_NibblePositonType Nibble1Position; /*!< nibble 1 Position */
    Sent_NibblePositonType Nibble2Position; /*!< nibble 2 Position */
    Sent_NibblePositonType Nibble3Position; /*!< nibble 3 Position */
    Sent_NibblePositonType Nibble4Position; /*!< nibble 4 Position */
    Sent_NibblePositonType Nibble5Position; /*!< nibble 5 Position */
    Sent_NibblePositonType Nibble6Position; /*!< nibble 6 Position */
    Sent_NibblePositonType Nibble7Position; /*!< nibble 7 Position */
} Sent_NibblePointerType;

/**
 * @brief Define the sent channel receive config.
 * @note DES ID: DES_SENT_TYP_003
 */
typedef struct
{
    boolean PausePulseEnable;               /*!< pause pulse enable */
    boolean AlternateCRCEnable;             /*!< alternate CRC metheod enable */
    boolean StatusIncludedInCRC;            /*!< status included in CRC */
    boolean SerialDataEnable;               /*!< serial message process enable */
    boolean SerialCRCEnable;                /*!< serial message CRC enable */
    boolean FastCRCEnable;                  /*!< fast channel CRC enable */
    Sent_FrameCheckModeType FrameCheckMode; /*!< consecutive frame check mode */
    uint8 FrameLength;                      /*!< data nibbles per frame */
    boolean ZeroAugumentInCRCDisable;       /*!< CRC with zero nibble disable */
    Sent_SerialMsgType SerialMsgType;       /*!< serial message type */
    boolean DriftErrorDisable;              /*!< ignore drift error mode */
    boolean FDFL;                           /*!< frequency drift check based on frame length */
} Sent_ReceiveCtrlType;

/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/**
 * @brief Define the sent channel transmit config.
 * @note DES ID: DES_SENT_TYP_004
 */
typedef struct
{
    uint8 PulseLength;             /*!< pulse length */
    uint8 PulseDelayLength;        /*!< pause delay length */
    Sent_SpcTriggerModeType Mode;  /*!< spc trigger mode */
    Sent_SpcTimeBaseType TimeBase; /*!< time base */
} Sent_TransmitCtrlType;

/**
 * @brief SENT module configuration structure.
 * @note DES ID: DES_SENT_TYP_000
 */
typedef struct
{
    uint16 FractionDivider;     /*!< module frantional divider */
    uint32 TimeStampPreDivider; /*!< timestamp divider */
} Sent_ModuleConfigType;

/**
 * @brief SENT channel configuration structure.
 * @note DES ID: DES_SENT_TYP_001
 */
typedef struct
{
    uint16 WatchdogTimerLimit;            /*!< watch dog timeout value */
    Sent_IoCtrlType IoControl;            /*!< input output control property */
    Sent_ReceiveCtrlType RxControl;       /*!< receive control property */
    float32 TickUnit;                     /*!< desired tick time unit, unit:seconds */
    uint32 EnabledInterrupts;             /*!< interrupts control */
    Sent_NibblePointerType NibblePointer; /*!< nibble position control */
    Sent_RxCallbackType RxCallback;       /*!< sent receive callback function */
    Sent_CallbackType TxCallback;         /*!< sent transmit callback user data */
    Sent_CallbackType ErrCallback;        /*!< sent channel error callback function */
} Sent_ChannelConfigType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

#ifdef __cplusplus
}
#endif

#endif
/*============================================EOF===================================================*/
