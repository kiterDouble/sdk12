/*************************************************
 Copyright © 2025 SiRun (WuXi). All rights reserved.
 File Name: j1939Tp.h
 Author: 
 Created Time: 
 Description: J1939 Transport Protocol SDK Header
              SAE J1939-21 Transport Protocol
              Supports BAM and RTS/CTS
 Others: 
 *************************************************/

#ifndef _J1939_TP_H_
#define _J1939_TP_H_

/****************************** include ***************************************/
#include <stdint.h>
#include "j1939Sdk.h"

/****************************** Macro Definitions ******************************/
/* TP Control Bytes */
#define J1939_TP_CM_RTS                 0x10    /* Request to Send */
#define J1939_TP_CM_CTS                 0x11    /* Clear to Send */
#define J1939_TP_CM_EOMA                0x13    /* End of Message Ack */
#define J1939_TP_CM_BAM                 0x20    /* Broadcast Announce Message */
#define J1939_TP_CM_ABORT               0xFF    /* Connection Abort */

/* TP Data Transfer */
#define J1939_TP_DT                     0x10

/* TP Timeouts (ms) */
#define J1939_TP_T1_TIMEOUT             750     /* Response timeout */
#define J1939_TP_T2_TIMEOUT             1250    /* Sender response timeout */
#define J1939_TP_T3_TIMEOUT             1250    /* Receiver response timeout */
#define J1939_TP_T4_TIMEOUT             1050    /* Hold timeout */

/* TP Limits */
#define J1939_TP_MAX_SESSIONS           4       /* Max concurrent TP sessions */
#define J1939_TP_MAX_PACKETS            255     /* Max packets per message */
#define J1939_TP_PACKET_SIZE            7       /* Data bytes per TP.DT packet */
#define J1939_TP_MIN_MESSAGE_SIZE       9       /* Min size to use TP */

/* J1939 Address */
#define J1939_TP_GLOBAL_ADDRESS         0xFF    /* Broadcast address */

/* TP Connection States */
typedef enum
{
    J1939_TP_STATE_IDLE = 0,                /* 空闲状态：会话未激活 */
    J1939_TP_STATE_TX_RTS_SENT,             /* 发送端：已发送 RTS，等待 CTS */
    J1939_TP_STATE_TX_CTS_RECEIVED,         /* 发送端：已收到 CTS，准备发送数据 */
    J1939_TP_STATE_TX_DT_SENDING,           /* 发送端：正在发送 DT 数据包 */
    J1939_TP_STATE_TX_WAIT_CTS,             /* 发送端：等待下一个 CTS */
    J1939_TP_STATE_TX_WAIT_EOMA,            /* 发送端：已发送完数据，等待 EOMA */
    J1939_TP_STATE_RX_WAIT_RTS,             /* 接收端：等待 RTS */
    J1939_TP_STATE_RX_SEND_CTS,             /* 接收端：发送 CTS */
    J1939_TP_STATE_RX_RECEIVING,            /* 接收端：正在接收 DT 数据包 */
    J1939_TP_STATE_RX_COMPLETE,             /* 接收端：数据接收完成 */
    J1939_TP_STATE_BAM_TX,                  /* BAM 广播发送状态 */
    J1939_TP_STATE_BAM_RX,                  /* BAM 广播接收状态 */
    J1939_TP_STATE_ERROR                    /* 错误状态：传输异常 */
} J1939TpState_e;

/* TP Abort Reasons */
typedef enum
{
    J1939_TP_ABORT_BUSY = 1,                /* 资源忙：会话正在处理其他传输 */
    J1939_TP_ABORT_RESOURCE,                /* 资源不足：内存或会话数不足 */
    J1939_TP_ABORT_TIMEOUT,                 /* 超时：等待响应超时 */
    J1939_TP_ABORT_EOA,                     /* EOA错误：End of Acknowledgment异常 */
    J1939_TP_ABORT_PGN,                     /* PGN错误：参数组号不匹配 */
    J1939_TP_ABORT_SIZE,                    /* 大小错误：数据长度超出限制 */
    J1939_TP_ABORT_OTHER                    /* 其他错误：未分类的异常 */
} J1939TpAbortReason_e;

/* TP Session Structure */
typedef struct
{
    uint8_t  inUse;                         /* 会话使用标志，1=会话正在使用，0=空闲 */
    J1939TpState_e state;                   /* TP连接状态，表示当前会话的状态机状态 */
    uint8_t  isBroadcast;                   /* 广播标志，1=BAM广播传输，0=RTS/CTS点对点传输 */
    uint32_t pgn;                           /* 参数组号(PGN)，标识传输的数据类型 */
    uint8_t  sourceAddr;                    /* 源地址，发送方的节点地址 */
    uint8_t  destAddr;                      /* 目标地址，接收方的节点地址(广播时为0xFF) */
    uint16_t totalSize;                     /* 总数据大小，本次传输的数据总长度(字节) */
    uint8_t  totalPackets;                  /* 总包数，需要发送的DT数据包总数 */
    uint8_t  maxPackets;                    /* 最大包数，CTS中允许一次发送的最大包数 */
    uint8_t  nextPacket;                    /* 下一包序号，接下来要发送或接收的包序号 */
    uint8_t  lastPacket;                    /* 最后一包序号，本次CTS中最后一包的序号 */
    uint32_t lastActivityTime;              /* 最后活动时间，用于超时检测的时间戳 */
    uint8_t  data[J1939_MAX_DATA_LENGTH];   /* 数据缓冲区，存储传输的完整数据(最大1785字节) */
    uint16_t dataIndex;                     /* 数据索引，当前已接收或发送的数据偏移量 */
} J1939TpSession_t;

/* TP Callback Types */
/* TP发送回调 - 由SDK层实现，TP层调用以发送CAN报文 */
typedef int16_t (*J1939TpSendCanMsgCallback_t)(uint32_t pgn, uint8_t destAddr, 
                                                const uint8_t *data, uint8_t length);
/* TP传输完成回调 */
typedef void (*J1939TpTxCompleteCallback_t)(uint32_t pgn, uint8_t destAddr, 
                                            uint8_t status);
/* TP接收完成回调 */                                            
typedef void (*J1939TpRxCompleteCallback_t)(uint32_t pgn, uint8_t sourceAddr,
                                            const uint8_t *data, uint16_t length);

/****************************** Function Declarations *************************/

/* TP Initialization */
int16_t J1939Tp_Init(J1939TpSendCanMsgCallback_t sendCallback);
int16_t J1939Tp_Deinit(void);

/* TP Transmission */
int16_t J1939Tp_Transmit(uint32_t pgn, uint8_t destAddr, const uint8_t *data, 
                        uint16_t length);
int16_t J1939Tp_TransmitBam(uint32_t pgn, const uint8_t *data, uint16_t length);

/* TP Reception */
int16_t J1939Tp_Receive(uint32_t pgn, uint8_t *data, uint16_t *length, 
                       uint32_t timeout);
void J1939Tp_RegisterRxCallback(J1939TpRxCompleteCallback_t callback);
void J1939Tp_RegisterTxCallback(J1939TpTxCompleteCallback_t callback);

/* TP Message Processing */
void J1939Tp_ProcessCmMessage(const J1939Message_t *msg);
void J1939Tp_ProcessDtMessage(const J1939Message_t *msg);

/* TP Cycle Processing */
void J1939Tp_CycleProcess(uint32_t currentTime);

/* TP Status */
uint8_t J1939Tp_IsBusy(void);
uint8_t J1939Tp_GetActiveSessions(void);

#endif /* _J1939_TP_SDK_H_ */
