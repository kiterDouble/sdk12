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
 * @file Can_Hal_Types.h
 * @brief This file provides can hal types header.
 */

#ifndef CAN_HAL_TYPES_H
#define CAN_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif
/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Register.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* Use CAN_PERFORMANCE_TEST  define here */

/**
 * @def BIT
 * @brief Bit mask macro for bit position x.
 * @note DES_CAN_MACRO_001
 */
#ifndef BIT
#define BIT(x) (1UL << (uint32)(x))
#endif

#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
/**
 * @def CAN_MAX_S_SEG_1
 * @brief CAN max slow bitrate segment 1.
 * @note DES_CAN_MACRO_006
 */
#define CAN_MAX_S_SEG_1    (0xFFUL)
/**
 * @def CAN_MAX_S_SEG_2
 * @brief CAN max slow bitrate segment 2.
 * @note DES_CAN_MACRO_007
 */
#define CAN_MAX_S_SEG_2    (0x7FUL)
/**
 * @def CAN_MAX_S_SJW
 * @brief CAN max slow bitrate SJW.
 * @note DES_CAN_MACRO_008
 */
#define CAN_MAX_S_SJW      (0x7FUL)
/**
 * @def CAN_MAX_S_PRESC
 * @brief CAN max slow bitrate prescaler.
 * @note DES_CAN_MACRO_009
 */
#define CAN_MAX_S_PRESC    (0xFFUL)
/**
 * @def CAN_MAX_F_SEG_1
 * @brief CAN max fast bitrate segment 1.
 * @note DES_CAN_MACRO_010
 */
#define CAN_MAX_F_SEG_1    (0x1FUL)
/**
 * @def CAN_MAX_F_SEG_2
 * @brief CAN max fast bitrate segment 2.
 * @note DES_CAN_MACRO_011
 */
#define CAN_MAX_F_SEG_2    (0xFUL)
/**
 * @def CAN_MAX_F_SJW
 * @brief CAN max fast bitrate SJW.
 * @note DES_CAN_MACRO_012
 */
#define CAN_MAX_F_SJW      (0xFUL)
/**
 * @def CAN_MAX_F_PRESC
 * @brief CAN max fast bitrate prescaler.
 * @note DES_CAN_MACRO_013
 */
#define CAN_MAX_F_PRESC    (0xFFU)

/**
 * @def CAN_RX_FIFO_NUM_MAX
 * @brief CAN max RX FIFO number.
 * @note DES_CAN_MACRO_014
 */
#define CAN_RX_FIFO_NUM_MAX (1UL)

#elif defined (AC7843X)
/**
 * @def CAN_MAX_S_SEG_1
 * @brief CAN max slow bitrate segment 1.
 * @note DES_CAN_MACRO_017
 */
#define CAN_MAX_S_SEG_1    (0xFFUL)
/**
 * @def CAN_MAX_S_SEG_2
 * @brief CAN max slow bitrate segment 2.
 * @note DES_CAN_MACRO_018
 */
#define CAN_MAX_S_SEG_2    (0x7FUL)
/**
 * @def CAN_MAX_S_SJW
 * @brief CAN max slow bitrate SJW.
 * @note DES_CAN_MACRO_019
 */
#define CAN_MAX_S_SJW      (0x7FUL)
/**
 * @def CAN_MAX_S_PRESC
 * @brief CAN max slow bitrate prescaler.
 * @note DES_CAN_MACRO_020
 */
#define CAN_MAX_S_PRESC    (0x1FFUL)
/**
 * @def CAN_MAX_F_SEG_1
 * @brief CAN max fast bitrate segment 1.
 * @note DES_CAN_MACRO_021
 */
#define CAN_MAX_F_SEG_1    (0x1FUL)
/**
 * @def CAN_MAX_F_SEG_2
 * @brief CAN max fast bitrate segment 2.
 * @note DES_CAN_MACRO_022
 */
#define CAN_MAX_F_SEG_2    (0xFUL)
/**
 * @def CAN_MAX_F_SJW
 * @brief CAN max fast bitrate SJW.
 * @note DES_CAN_MACRO_023
 */
#define CAN_MAX_F_SJW      (0xFUL)
/**
 * @def CAN_MAX_F_PRESC
 * @brief CAN max fast bitrate prescaler.
 * @note DES_CAN_MACRO_024
 */
#define CAN_MAX_F_PRESC    (0x1FUL)

/**
 * @def CAN_DEDICATED_BUFFER_DEPTH_NUM
 * @brief CAN dedicated RX/TX buffer depth number.
 * @note DES_CAN_MACRO_025
 */
#define CAN_DEDICATED_BUFFER_DEPTH_NUM (0x1U)

/**
 * @def CAN_STD_FILTER_NUM_MAX
 * @brief CAN standard filter number max.
 * @note DES_CAN_MACRO_026
 */
#define CAN_STD_FILTER_NUM_MAX (128UL)
/**
 * @def CAN_EXT_FILTER_NUM_MAX
 * @brief CAN extended filter number max.
 * @note DES_CAN_MACRO_027
 */
#define CAN_EXT_FILTER_NUM_MAX (64UL)
/**
 * @def CAN_FILTER_NUM_MAX
 * @brief CAN filter number max.
 * @note DES_CAN_MACRO_028
 */
#define CAN_FILTER_NUM_MAX (CAN_STD_FILTER_NUM_MAX + CAN_STD_FILTER_NUM_MAX)

/**
 * @def CAN_TX_FIFO_NUM_MAX
 * @brief CAN TX FIFO number max.
 * @note DES_CAN_MACRO_029
 */
#define CAN_TX_FIFO_NUM_MAX (1UL)
/**
 * @def CAN_TX_BUF_NUM_MAX
 * @brief CAN TX buffer number max.
 * @note DES_CAN_MACRO_030
 */
#define CAN_TX_BUF_NUM_MAX (32UL)
/**
 * @def CAN_TX_EVENT_FIFO_DEPTH_MAX
 * @brief CAN TX event FIFO depth max.
 * @note DES_CAN_MACRO_031
 */
#define CAN_TX_EVENT_FIFO_DEPTH_MAX (32UL)

/**
 * @def CAN_RX_FIFO_NUM_MAX
 * @brief CAN RX FIFO number max.
 * @note DES_CAN_MACRO_032
 */
#define CAN_RX_FIFO_NUM_MAX (2UL)
/**
 * @def CAN_RX_FIFO_DEPTH_MAX
 * @brief CAN RX FIFO depth max.
 * @note DES_CAN_MACRO_033
 */
#define CAN_RX_FIFO_DEPTH_MAX (64UL)
/**
 * @def CAN_RX_DEDICATED_BUFFER_NUM_MAX
 * @brief CAN RX dedicated buffer number max.
 * @note DES_CAN_MACRO_034
 */
#define CAN_RX_DEDICATED_BUFFER_NUM_MAX (64UL)
/**
 * @def CAN_RX_BUF_NUM_MAX
 * @brief CAN RX buffer number max.
 * @note DES_CAN_MACRO_035
 */
#define CAN_RX_BUF_NUM_MAX (66UL)
#endif
/*===================================================ENUMS==========================================*/

/* PRQA S 1535 ++ # AMCE_QAC_1535.*/
/**
 * @brief CAN error mask type enumeration.
 * @note DES_CAN_TYP_001
 */
typedef enum
{
    CAN_BUSERR_BIT_MASK   = 0x1,   /*!< Bit error */
    CAN_BUSERR_FORM_MASK  = 0x2,   /*!< Form error */
    CAN_BUSERR_STUFF_MASK = 0x4,   /*!< Stuff error */
    CAN_BUSERR_ACK_MASK   = 0x8,   /*!< Ack error */
    CAN_BUSERR_CRC_MASK   = 0x10,  /*!< CRC error */
    CAN_BUSERR_OTHER_MASK = 0x20,  /*!< Other error */
    CAN_ARBIT_ERR_MASK    = 0x40,  /*!< Arbitration error */
    CAN_OVERFLOW_ERR_MASK = 0x80,  /*!< Receive FIFO overflow error */
    CAN_ECC_WARN_MASK     = 0x100, /*!< ECC warning (CAN_VERMEM_MDWIF_Msk) */
    CAN_ECC_ERR_MASK      = 0x200  /*!< ECC error (CAN_VERMEM_MDEIF_Msk) */
} Can_ErrorMaskType;

/**
 * @brief CAN frame RTR type enumeration.
 * @note DES_CAN_TYP_002
 */
typedef enum
{
    CAN_CTRL_DATA_FRAME   = 0x0U, /*!< Data frame */
    CAN_CTRL_REMOTE_FRAME         /*!< Remote frame */
} Can_CtrlFrameType;

/**
 * @brief CAN frame DLC value type enumeration.
 * @note DES_CAN_TYP_003
 */
typedef enum
{
    CAN_MSG_DLC_12_BYTES = 0x09U, /*!< 12 bytes data length */
    CAN_MSG_DLC_16_BYTES,         /*!< 16 bytes data length */
    CAN_MSG_DLC_20_BYTES,         /*!< 20 bytes data length */
    CAN_MSG_DLC_24_BYTES,         /*!< 24 bytes data length */
    CAN_MSG_DLC_32_BYTES,         /*!< 32 bytes data length */
    CAN_MSG_DLC_48_BYTES,         /*!< 48 bytes data length */
    CAN_MSG_DLC_64_BYTES          /*!< 64 bytes data length */
} Can_MsgDlcType;
/* PRQA S 1535 -- # AMCE_QAC_1535.*/

/**
 * @brief CAN extend mode type enumeration.
 * @note DES_CAN_TYP_004
 */
typedef enum
{
    CAN_EXTMODE_OFF              = 0x0U, /*!< Extend mode off */
    CAN_EXTMODE_LISTENING,               /*!< Listening mode */
    CAN_EXTMODE_LOOPBACK_INTERNEL,       /*!< Internal loopback mode */
    CAN_EXTMODE_LOOPBACK_EXTERNEL        /*!< External loopback mode */
} Can_ExtendModeType;

/**
 * @brief CAN error state type enumeration.
 * @note DES_CAN_TYP_005
 */
typedef enum
{
    CAN_ERR_STATE_ACTIVE  = 0x0U, /*!< Error state active */
    CAN_ERR_STATE_PASSIVE,        /*!< Error state passive */
    CAN_ERR_STATE_BUSOFF          /*!< Error state busoff */
} Can_DevErrorStateType;

/**
 * @brief CAN work state type enumeration.
 * @note DES_CAN_TYP_006
 */
typedef enum
{
    CAN_STATE_STOP    = 0x0U, /*!< Stop state */
    CAN_STATE_RUNNING,         /*!< Running state */
    CAN_STATE_STANDBY          /*!< Standby state */
} Can_HalStateType;

/**
 * @brief CAN frame type enumeration.
 * @note DES_CAN_TYP_007
 */
typedef enum
{
    CAN_STANDARD_MSG = 0x0U, /*!< Standard frame */
    CAN_EXTENDED_MSG         /*!< Extended frame */
} Can_MessageIdType;

/**
 * @brief CAN filter type enumeration.
 * @note DES_CAN_TYP_008
 */
typedef enum
{
    STD_FRAME = 0U, /*!< Only receive standard frame ID */
    EXT_FRAME,      /*!< Only receive extended frame ID */
#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
    STD_EXT_FRAME   /*!< Receive standard/extended frame ID */
#endif
} Can_FilterFrameType;

#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
/* PRQA S 1535 ++ # AMCE_QAC_1535.*/
/**
 * @brief CAN tx buf type enumeration.
 * @note DES_CAN_TYP_009
 */
typedef enum
{
    CAN_TX_BUF_PRIMARY = 0U, /*!< Tx primary buffer */
    CAN_TX_BUF_SECONDARY     /*!< Tx secondary buffer */
} Can_TxBufType;
/* PRQA S 1535 -- # AMCE_QAC_1535.*/

/**
 * @brief CAN tx secondary buf amount.
 * @note DES_CAN_TYP_010
 */
typedef enum
{
    CAN_TX_SEC_ONE = 0U,    /*!< Transmit secondary one message  */
    CAN_TX_SEC_ALL          /*!< Transmit secondary all messages */
} Can_TxSecAmountType;
#endif

#if defined (AC7843X)
/**
 * @brief CAN frame data size enumeration.
 * @note DES_CAN_TYP_011
 */
typedef enum
{
    CAN_DATA_SIZE_8  = 0U, /*!< 8 bytes */
    CAN_DATA_SIZE_12,      /*!< 12 bytes */
    CAN_DATA_SIZE_16,      /*!< 16 bytes */
    CAN_DATA_SIZE_20,      /*!< 20 bytes */
    CAN_DATA_SIZE_24,      /*!< 24 bytes */
    CAN_DATA_SIZE_32,      /*!< 32 bytes */
    CAN_DATA_SIZE_48,      /*!< 48 bytes */
    CAN_DATA_SIZE_64       /*!< 64 bytes */
} Can_DataSizeType;

/**
 * @brief CAN RX FIFO mode enumeration.
 * @note DES_CAN_TYP_012
 */
typedef enum
{
    BLOCK_MODE    = 0U, /*!< Blocking mode */
    OVERWRITE_MODE       /*!< Overwrite mode */
} Can_RxFifoModeType; // PRQA S 1535 # AMCE_QAC_1535.

/**
 * @brief CAN MRAM type enumeration.
 * @note DES_CAN_TYP_013
 */
typedef enum
{
    DEDICATED_RX_BUFFER, /*!< Dedicated RX buffer */
    RX_FIFO0,            /*!< RX FIFO 0 */
    RX_FIFO1,            /*!< RX FIFO 1 */
    TX_BUFFER,           /*!< TX buffer */
    TX_EVENT_FIFO,       /*!< TX event FIFO */
    STD_FILTER,          /*!< Standard ID filter */
    EXT_FILTER           /*!< Extended ID filter */
} Can_MramType;

/**
 * @brief CAN TX buffer type enumeration (AC7843X).
 * @note DES_CAN_TYP_014
 */
typedef enum
{
    TX_BUF_TYPE_DEDICATED, /*!< Dedicated TX buffer */
    TX_BUF_TYPE_FIFO       /*!< TX FIFO */
} Can_TxBufferType; // PRQA S 1535 # AMCE_QAC_1535.

/**
 * @brief CAN RX buffer type enumeration (AC7843X).
 * @note DES_CAN_TYP_015
 */
typedef enum
{
    RX_BUF_TYPE_DEDICATED, /*!< Dedicated RX buffer */
    RX_BUF_TYPE_FIFO0,     /*!< RX FIFO 0 */
    RX_BUF_TYPE_FIFO1      /*!< RX FIFO 1 */
} Can_RxBufferType; // PRQA S 1535 # AMCE_QAC_1535.

/**
 * @brief CAN filter type enumeration (AC7843X).
 * @note DES_CAN_TYP_016
 */
typedef enum
{
    FILTER_RANGE = 0U,              /*!< Range filter */
    FILTER_DUAL,                    /*!< Dual ID filter */
    FILTER_CLASSIC,                 /*!< Classic filter */
    STD_DISABLE_OR_EXT_RANGE        /*!< STD: disable; EXT: range filter with XIDAM mask not applied */
} Can_FilterType;

/**
 * @brief CAN filter element configuration enumeration (AC7843X).
 * @note DES_CAN_TYP_017
 */
typedef enum
{
    DISABLE_FILTER = 0U,            /*!< Filter disable */
    STORE_IN_RXFIFO0,               /*!< Store in RX FIFO 0 if filter matches */
    STORE_IN_RXFIFO1,               /*!< Store in RX FIFO 1 if filter matches */
    REJECT_MATCH_ID,                /*!< Reject ID if filter matches */
    SET_PRIORITY,                   /*!< Set priority if filter matches */
    SET_PRIORITY_STORE_IN_RXFIFO0,  /*!< Set priority and store in FIFO 0 if filter matches */
    SET_PRIORITY_STORE_IN_RXFIFO1,  /*!< Set priority and store in FIFO 1 if filter matches */
    STORE_IN_RXBUFFER               /*!< Store into RX buffer or as debug message */
} Can_FilterElementCfgType;

/**
 * @brief CAN RX filter no-match handler enumeration (AC7843X).
 * @note DES_CAN_TYP_018
 */
typedef enum
{
    ACCEPT_RXFIFO0 = 0U, /*!< Store in RX FIFO 0 */
    ACCEPT_RXFIFO1,      /*!< Store in RX FIFO 1 */
    REJECT_NOMATCH_ID    /*!< Reject message */
} Can_NoMacthFramesType;
#endif
/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/*PRQA S 3630 ++ # AMCE_CAN_3630. */
/**
 * @brief CAN TX/RX frame information structure.
 * @note DES_CAN_TYP_022
 */
typedef struct
{
    boolean MsgFdf;                /*!< FD format indicator (FDF) */
    boolean MsgRtr;                /*!< Remote transmission request (RTR) */
    boolean MsgBrs;                /*!< Bit rate switch (BRS) */
    Can_MessageIdType MsgIdType;   /*!< Frame type: standard or extended */
    uint8 MsgDataLen;              /*!< Frame data length */
    uint8 *MsgDataPtr;             /*!< Frame data pointer */
    uint32 MsgRts;                 /*!< Receive time stamp (RTS), RX only */
    uint32 MsgId;                  /*!< CAN identifier */
    uint8 FdPaddingValue;          /*!< CAN FD padding value, TX only */
#if defined (AC7843X)
    boolean EfcAnmf;               /*!< TX: event FIFO control (EFC); RX: accepted non-matching frame (ANMF) */
    uint8 MmFidx;                  /*!< TX: message marker (MM); RX: filter index (FIDX) */
    uint8 HwBufIdx;                /*!< Tx: Msg transmit used hw buffer index; RX: not use*/
#endif
} Can_MessageInfoType;

/**
 * @brief CAN IRQ callback argument structure.
 * @note DES_CAN_TYP_023
 */
typedef struct
{
    uint8 Instance;                /*!< CAN HW unit index */
    uint32 CommonIrqFlagMasks;     /*!< Common IRQ flag masks, see CAN_COMMON_IRQ_FLAG_MASKS */
#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
    uint32 EccIrqFlagMasks;        /*!< ECC IRQ flag masks, see CAN_ECC_IRQ_FLAG_MASKS */
#elif defined (AC7843X)
    uint32 RxNewData1;             /*!< RX new data1 */
    uint32 RxNewData2;             /*!< RX new data2 */
    uint32 TxBTO;                  /*!< TXBTO register value */
#endif
} Can_IrqCBParams;

#ifndef CAN_SDK_NON_EXTENDED_API
/**
 * @brief CAN timestamp configuration structure.
 * @note DES_CAN_TYP_024
 */
typedef struct
{
    boolean En;         /*!< Timestamp enable, affects if TTS/RTS is valid */
    boolean ExtClkSrc;  /*!< Use external clock source */
    boolean PosEnd;     /*!< Timestamp position select: false=SOF, true=EOF */
    uint8 ExtClkDiv;    /*!< External clock source division, 0:clk/1 ... 7:clk/8 */
} Can_TimeStampType;
#endif

#if defined (AC7843X)
/**
 * @brief CAN DMU IRQ callback argument structure.
 * @note DES_CAN_TYP_025
 */
typedef struct
{
    uint8 Instance;            /*!< CAN HW unit index */
    uint32 DmuIrqFlagMasks;    /*!< DMU IRQ flag masks */
} Can_DmuIrqCBParams;

/**
 * @brief CAN global filter configuration structure.
 * @note DES_CAN_TYP_026
 */
typedef struct
{
    boolean rejectStdRemote;        /*!< Reject standard remote frames */
    boolean rejectExtRemote;        /*!< Reject extended remote frames */
    Can_NoMacthFramesType std;      /*!< Accept non-matching standard frames */
    Can_NoMacthFramesType ext;      /*!< Accept non-matching extended frames */
} Can_GlobalFilterConfigType;

/**
 * @brief CAN device argument structure.
 * @note DES_CAN_TYP_027
 */
typedef struct
{
    uint8 BufferType;   /*!< Buffer type */
    uint8 BufferIndex;  /*!< Buffer index */
} Can_BufferInfoType;

/**
 * @brief CAN TX buffer configuration structure.
 * @note DES_CAN_TYP_028
 */
typedef struct
{
    uint8 DepthNum;         /*!< FIFO depth number */
    boolean TxCmpIrqEn;     /*!< TXBTIE bit enable */
    boolean TxCnlIrqEn;     /*!< TXBCIE bit enable */
} Can_TxBufferConfigType;

/**
 * @brief CAN RX buffer configuration structure.
 * @note DES_CAN_TYP_029
 */
typedef struct
{
    uint8 DepthNum; /*!< FIFO depth number */
} Can_RxBufferConfigType;

/**
 * @brief CAN frame data size configuration structure.
 * @note DES_CAN_TYP_030
 */
typedef struct
{
    Can_DataSizeType txBufferDataSize;    /*!< TX buffer data size */
    Can_DataSizeType rxDbufferDataSize;   /*!< Dedicated RX buffer data size */
    Can_DataSizeType rxFifo0DataSize;     /*!< RX FIFO 0 data size */
    Can_DataSizeType rxFifo1DataSize;     /*!< RX FIFO 1 data size */
} Can_DataSizeConfigType;

/**
 * @brief CAN TX event FIFO element information structure.
 * @note DES_CAN_TYP_031
 */
typedef struct
{
    uint8 ESI;     /*!< Error state indicator */
    uint8 XTD;     /*!< Extended identifier */
    uint8 RTR;     /*!< Remote transmission request */
    uint8 MM;      /*!< Message marker */
    uint8 EFC;     /*!< Event type */
    uint8 FDF;     /*!< FD format */
    uint8 BRS;     /*!< Bit rate switch */
    uint8 DLC;     /*!< Data length code */
    uint16 TXTS;   /*!< TX timestamp */
    uint32 ID;     /*!< Identifier */
} Can_MsgEventType; // PRQA S 1535 # AMCE_QAC_1535.
#endif

/**
 * @brief CAN filter config args structure.
 * @note DES_CAN_TYP_032
 */
typedef struct
{
    uint32 ID1; /*!< filter ID1 or Code*/
    uint32 ID2; /*!< filter ID2 or Mask*/
    Can_FilterFrameType FrameType;/*!< filter accept frame type*/
#if defined (AC7843X)
    Can_FilterType Type; /*!< filter type */
    Can_FilterElementCfgType FEC; /*!< filter element configuration */
#endif
} Can_FilterParamsType;

typedef struct
{
    uint8 FiltersNum; /*!< can standard filters num */
    Can_FilterParamsType *FiltersParamsPtr; /*!< can extended filters args array ptr*/
#if defined (AC7843X)
    Can_GlobalFilterConfigType GlobalFilter;/*!< Global filter config */
    uint32 XIDAM; /*!< extended id acceptance Mask */
#endif
} Can_FiltersConfigType;

/**
 * @brief CAN bitrate config args structure.
 * @note DES_CAN_TYP_033
 */
typedef struct
{
    uint16 Presc; /*!< bitrate args: Presc(BRP) 0:clk/1 ... 255:clk/256 */
    uint8 Seg1; /*!< bitrate args: Seg1 0 ~ 0xFF*/
    uint8 Seg2; /*!< bitrate args: Seg2 0 ~ 0x7F*/
    uint8 Sjw; /*!< bitrate args: Sjw 0 ~ 0x7F*/
} Can_BitrateParamsType;

/**
 * @brief CAN baudrate config args structure.
 * @note DES_CAN_TYP_034
 */
typedef struct
{
    Can_BitrateParamsType NormalBitrate; /*!< Can normal bitrate */
    Can_BitrateParamsType DataBitrate; /*!< Can data bitrate */
    uint8 SspOffset; /*!< Can SSPOFF:0 disable others:1~0x7F */
#if defined (AC7843X)
    uint8 TDCFWL; /*!< Can transmitter delay compensation filter window length */
#endif
} Can_BaudrateConfigType;

/**
 * @brief CAN rx fifo buf dma config args structure.
 * @note DES_CAN_TYP_035
 */
typedef struct
{
    uint8 FifoId; /*!< Can rx fifo id */
    uint32 DmaDstAddr; /*!< Can dma dest address, it's length must one frame info size */
    Hal_CallbackType DmaCb; /*!< Can dma callback, user can swap DmaDstAddr and start dma again */
    void *DmaCbArgs; /*!< Can dma callback args, if needed */
} Can_RxFifoBufDmaConfigType;

/**
 * @brief CAN init config args structure.
 * @note DES_CAN_TYP_036
 */
typedef struct
{
    boolean EccEn; /*!< Can ECC enable */
    boolean FdEn; /*!< Can FD enable */
    boolean FdIsoEn; /*!< Can FD ISO mode enable*/
    boolean RxOverWrite; /*!< Can rx overwrite enable, true: overwrite old frame when rx fifo overflow,*/
                         /*false: new frame ignore when rx fifo overflow */
    uint8 RxFifoBufDmaConfigNum; /*!< Can rx fifo buf dam config num */
    Can_RxFifoBufDmaConfigType *RxFifoBufDmaConfigPtr; /*!< Can rx fifo buf dam config args */
    Can_BaudrateConfigType *BaudrateConfigPtr; /*!< Can Baudrate config args */
    Can_FiltersConfigType Filters; /*!< Can Filters config args */
#if defined (AC7843X)
    uint8 RxBuffersNum; /*!< can rx buffers num*/
    uint8 TxBuffersNum; /*!< can tx buffers num*/
    Can_RxBufferConfigType *RxBufferConfigPtr; /*!< Can rx buffer config args */
    Can_TxBufferConfigType *TxBufferConfigPtr; /*!< Can tx buffer config args */
    Can_BufferInfoType *RxBuffersInfoPtr; /*!< User alloc memory to hal and cannot release it after init,*/
                                          /* args fill by hal, user no need cfg it */
    Can_BufferInfoType *TxBuffersInfoPtr; /*!< User alloc memory to hal and cannot release it after init,*/
                                          /*args fill by hal, user no need cfg it */
    Can_DataSizeConfigType DataSize; /*!< Frame data size args */
    uint8 TxEventFifoNum; /*!< Tx event Fifo number */
    uint8 TxEventFifoWm;  /*!< Tx event Fifo watermark, EFWM */
#endif
    Hal_CallbackType WakeupIrqCallback; /*!< Can wakeup irq callback, NULL not enable irq */
    Hal_CallbackType IrqCallback; /*!< Can irq callback(inlude common irq masks && ecc irq masks),*/
                                  /*NULL not enable irq */
    uint32 CommonIrqEnMasks; /*!< common irq enable masks, refer to CAN_COMMON_IRQ_EN_MASKS */
#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
    boolean BOREC;                  /*!<busoff auto recovery disable*/
    Can_TxSecAmountType TxSecAmount;/*!<CAN tx secondary buf amount*/
    uint32 EccIrqEnMasks; /*!< ecc irq enable masks, Only EccEn Enable need cfg, refer to CAN_ECC_IRQ_EN_MASKS*/
#else
    uint32 DmuIrqEnMasks; /*!< dmu irq enable masks, refer to reg:DMUIE */
    Hal_CallbackType DmuIrqCallback; /*!< DmuIrqEnMasks != 0U must dmu irq callback != NULL */
#endif
} Can_HalConfigType;

#if defined CAN_PERFORMANCE_TEST
typedef struct
{
    uint32 rxAllCount;
    uint32 rxCurrentCount;
    uint32 txAllCount;
    uint32 txCurrentCount;
    uint32 txPeriod0;
    uint32 txPeriod1;
    uint32 txPeriod2;
    uint32 MsgId; /*!< CAN identifier */
    uint32 txPFCnt0;
    uint32 txPFCnt1;
    uint32 txPFCnt2;
    uint64 TimerCount;
    uint32 CountPeriod;
    uint8 abortTask;
    uint64 TimeTxMin;
    uint64 TimeTxMax;
    uint64 TimeTxAvg;
    uint64 TimeRxMin;
    uint64 TimeRxMax;
    uint64 TimeRxAvg;
} Can_PerformaceTestType;
extern Can_PerformaceTestType CanPerformaceTest;
extern uint64 OsIf_GetCurrentTimeUS(void);
#endif
/*PRQA S 3630 -- # AMCE_CAN_3630. */
#ifdef __cplusplus
}
#endif

#endif
