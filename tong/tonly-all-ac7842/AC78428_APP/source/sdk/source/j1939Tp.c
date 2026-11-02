/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: j1939TpSdk.c
 Author: 
 Created Time: 
 Description: J1939 Transport Protocol Implementation
              SAE J1939-21 Transport Protocol
              Supports BAM and RTS/CTS
 Others: 
 *************************************************/

/****************************** include ***************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "j1939Tp.h"
#include "j1939Sdk.h"
#include "logHal.h"
#include "timerHal.h"

/****************************** Macro Definitions ******************************/
/* TP Data Transfer */
#define J1939_TP_DT_DATA_SIZE           7       /* Data bytes per TP.DT frame */

/* TP CM Data Positions */
#define J1939_TP_CM_BYTE_CONTROL        0       /* Control byte position */
#define J1939_TP_CM_BYTE_TOTAL_SIZE_LSB 1       /* Total message size LSB */
#define J1939_TP_CM_BYTE_TOTAL_SIZE_MSB 2       /* Total message size MSB */
#define J1939_TP_CM_BYTE_NUM_PACKETS    3       /* Number of packets */
#define J1939_TP_CM_BYTE_MAX_PACKETS    4       /* Max packets in CTS */
#define J1939_TP_CM_BYTE_PGN_LSB        5       /* PGN LSB */
#define J1939_TP_CM_BYTE_PGN_MID        6       /* PGN Middle */
#define J1939_TP_CM_BYTE_PGN_MSB        7       /* PGN MSB */

/* TP DT Data Positions */
#define J1939_TP_DT_BYTE_SEQ_NUM        0       /* Sequence number */
#define J1939_TP_DT_DATA_START          1       /* Data start position */

/****************************** Global Variables ******************************/
static J1939TpSession_t g_tpSessions[J1939_TP_MAX_SESSIONS];
static J1939TpSendCanMsgCallback_t g_tpSendCallback = NULL;  /* 发送回调函数 */
static J1939TpRxCompleteCallback_t g_tpRxCallback = NULL;
static J1939TpTxCompleteCallback_t g_tpTxCallback = NULL;
static uint8_t g_tpInitialized = 0;

/****************************** Function Declarations *************************/
static J1939TpSession_t* J1939Tp_GetFreeSession(void);
static J1939TpSession_t* J1939Tp_FindSession(uint8_t sourceAddr, uint8_t destAddr);
static void J1939Tp_CloseSession(J1939TpSession_t *session);
static void J1939Tp_AbortSession(J1939TpSession_t *session, J1939TpAbortReason_e reason);

static int16_t J1939Tp_BuildCmMessage(uint8_t control, uint16_t totalSize, 
                                     uint8_t numPackets, uint8_t maxPackets,
                                     uint32_t pgn, uint8_t *data);
static int16_t J1939Tp_BuildDtMessage(uint8_t seqNum, uint8_t *srcData, uint8_t *dstData);

static void J1939Tp_ProcessRts(const J1939Message_t *msg);
static void J1939Tp_ProcessCts(const J1939Message_t *msg);
static void J1939Tp_ProcessEoma(const J1939Message_t *msg);
static void J1939Tp_ProcessBam(const J1939Message_t *msg);
static void J1939Tp_ProcessAbort(const J1939Message_t *msg);
static void J1939Tp_ProcessDt(const J1939Message_t *msg);

static void J1939Tp_SendCts(J1939TpSession_t *session);
static void J1939Tp_SendEoma(J1939TpSession_t *session);
static void J1939Tp_SendAbort(J1939TpSession_t *session, J1939TpAbortReason_e reason);
static int16_t J1939Tp_SendDtPackets(J1939TpSession_t *session);

static void J1939Tp_CheckTimeouts(uint32_t currentTime);

/*************************************************
 Function:        J1939Tp_SendCanMessage
 Description:     通过回调函数发送CAN报文（替换直接调用SDK）
 Input:           pgn - PGN
                  destAddr - 目标地址
                  data - 数据
                  length - 数据长度
 Output:          None
 Return:          J1939_OK on success
 Others:          调用SDK层提供的回调函数
 *************************************************/
static int16_t J1939Tp_SendCanMessage(uint32_t pgn, uint8_t destAddr, 
                                     const uint8_t *data, uint8_t length)
{
    if (g_tpSendCallback == NULL)
    {
        return J1939_ERROR_NOT_INITIALIZED;
    }
    
    return g_tpSendCallback(pgn, destAddr, data, length);
}

/****************************** Private Function Implementations **************/

/*************************************************
 Function:        J1939Tp_GetFreeSession
 Description:     Get a free TP session
 Input:           None
 Output:          None
 Return:          Pointer to free session or NULL
 Others:          
 *************************************************/
static J1939TpSession_t* J1939Tp_GetFreeSession(void)
{
    uint8_t i;
    
    for (i = 0; i < J1939_TP_MAX_SESSIONS; i++)
    {
        if (!g_tpSessions[i].inUse)
        {
            memset(&g_tpSessions[i], 0, sizeof(J1939TpSession_t));
            g_tpSessions[i].inUse = 1;
            return &g_tpSessions[i];
        }
    }
    
    return NULL;
}

/*************************************************
 Function:        J1939Tp_FindSession
 Description:     Find TP session by addresses
 Input:           sourceAddr - Source address
                  destAddr - Destination address
 Output:          None
 Return:          Pointer to session or NULL
 Others:          
 *************************************************/
static J1939TpSession_t* J1939Tp_FindSession(uint8_t sourceAddr, uint8_t destAddr)
{
    uint8_t i;
    
    for (i = 0; i < J1939_TP_MAX_SESSIONS; i++)
    {
        if (g_tpSessions[i].inUse &&
            g_tpSessions[i].sourceAddr == sourceAddr &&
            g_tpSessions[i].destAddr == destAddr)
        {
            return &g_tpSessions[i];
        }
    }
    
    return NULL;
}

/*************************************************
 Function:        J1939Tp_CloseSession
 Description:     Close a TP session
 Input:           session - Session to close
 Output:          None
 Return:          None
 Others:          
 *************************************************/
static void J1939Tp_CloseSession(J1939TpSession_t *session)
{
    if (session != NULL)
    {
        session->inUse = 0;
        session->state = J1939_TP_STATE_IDLE;
    }
}

/*************************************************
 Function:        J1939Tp_AbortSession
 Description:     Abort a TP session
 Input:           session - Session to abort
                  reason - Abort reason
 Output:          None
 Return:          None
 Others:          
 *************************************************/
static void J1939Tp_AbortSession(J1939TpSession_t *session, J1939TpAbortReason_e reason)
{
    if (session == NULL)
    {
        return;
    }
    
    /* Send abort message for non-broadcast sessions */
    if (!session->isBroadcast && session->state != J1939_TP_STATE_IDLE)
    {
        J1939Tp_SendAbort(session, reason);
    }
    
    /* Notify callback if TX session */
    if (session->state == J1939_TP_STATE_TX_RTS_SENT ||
        session->state == J1939_TP_STATE_TX_DT_SENDING ||
        session->state == J1939_TP_STATE_BAM_TX)
    {
        if (g_tpTxCallback != NULL)
        {
            g_tpTxCallback(session->pgn, session->destAddr, J1939_ERROR);
        }
    }
    
    session->state = J1939_TP_STATE_ERROR;
    J1939Tp_CloseSession(session);
}

/*************************************************
 Function:        J1939Tp_BuildCmMessage
 Description:     Build TP.CM message data
 Input:           control - Control byte
                  totalSize - Total message size
                  numPackets - Number of packets
                  maxPackets - Max packets in CTS
                  pgn - Parameter Group Number
                  data - Output data buffer
 Output:          data
 Return:          J1939_OK on success
 Others:          
 *************************************************/
static int16_t J1939Tp_BuildCmMessage(uint8_t control, uint16_t totalSize, 
                                     uint8_t numPackets, uint8_t maxPackets,
                                     uint32_t pgn, uint8_t *data)
{
    if (data == NULL)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    data[J1939_TP_CM_BYTE_CONTROL] = control;
    data[J1939_TP_CM_BYTE_TOTAL_SIZE_LSB] = (uint8_t)(totalSize & 0xFF);
    data[J1939_TP_CM_BYTE_TOTAL_SIZE_MSB] = (uint8_t)((totalSize >> 8) & 0xFF);
    data[J1939_TP_CM_BYTE_NUM_PACKETS] = numPackets;
    data[J1939_TP_CM_BYTE_MAX_PACKETS] = maxPackets;
    data[J1939_TP_CM_BYTE_PGN_LSB] = (uint8_t)(pgn & 0xFF);
    data[J1939_TP_CM_BYTE_PGN_MID] = (uint8_t)((pgn >> 8) & 0xFF);
    data[J1939_TP_CM_BYTE_PGN_MSB] = (uint8_t)((pgn >> 16) & 0xFF);
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Tp_BuildDtMessage
 Description:     Build TP.DT message data
 Input:           seqNum - Sequence number
                  srcData - Source data
                  dstData - Destination data buffer
 Output:          dstData
 Return:          J1939_OK on success
 Others:          
 *************************************************/
static int16_t J1939Tp_BuildDtMessage(uint8_t seqNum, uint8_t *srcData, uint8_t *dstData)
{
    if (srcData == NULL || dstData == NULL)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    dstData[J1939_TP_DT_BYTE_SEQ_NUM] = seqNum;
    memcpy(&dstData[J1939_TP_DT_DATA_START], srcData, J1939_TP_DT_DATA_SIZE);
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Tp_SendCts
 Description:     Send CTS (Clear To Send) message
 Input:           session - TP session
 Output:          None
 Return:          None
 Others:          
 *************************************************/
static void J1939Tp_SendCts(J1939TpSession_t *session)
{
    uint8_t data[8];
    uint8_t numPacketsToSend;
    
    if (session == NULL)
    {
        return;
    }
    
    /* Calculate number of packets to request */
    numPacketsToSend = session->totalPackets - session->nextPacket + 1;
    if (numPacketsToSend > session->maxPackets)
    {
        numPacketsToSend = session->maxPackets;
    }
    
    /* Build CTS message */
    J1939Tp_BuildCmMessage(J1939_TP_CM_CTS, 0, session->nextPacket, 
                          numPacketsToSend, session->pgn, data);
    
    /* Send via callback */
    J1939Tp_SendCanMessage(J1939_PGN_TP_CONNECTION_MANAGE, session->sourceAddr,
                          data, 8);
}

/*************************************************
 Function:        J1939Tp_SendEoma
 Description:     Send End of Message Acknowledgment
 Input:           session - TP session
 Output:          None
 Return:          None
 Others:          
 *************************************************/
static void J1939Tp_SendEoma(J1939TpSession_t *session)
{
    uint8_t data[8];
    
    if (session == NULL)
    {
        return;
    }
    
    /* Build EOMA message */
    J1939Tp_BuildCmMessage(J1939_TP_CM_EOMA, session->totalSize, 
                          session->totalPackets, 0xFF, session->pgn, data);
    
    /* Send via callback */
    J1939Tp_SendCanMessage(J1939_PGN_TP_CONNECTION_MANAGE, session->sourceAddr,
                          data, 8);
}

/*************************************************
 Function:        J1939Tp_SendAbort
 Description:     Send Abort message
 Input:           session - TP session
                  reason - Abort reason
 Output:          None
 Return:          None
 Others:          
 *************************************************/
static void J1939Tp_SendAbort(J1939TpSession_t *session, J1939TpAbortReason_e reason)
{
    uint8_t data[8];
    
    if (session == NULL)
    {
        return;
    }
    
    /* Build Abort message */
    J1939Tp_BuildCmMessage(J1939_TP_CM_ABORT, 0, (uint8_t)reason,
                          0xFF, session->pgn, data);

    /* Send via callback */
    J1939Tp_SendCanMessage(J1939_PGN_TP_CONNECTION_MANAGE, session->sourceAddr,
                          data, 8);
}

/*************************************************
 Function:        J1939Tp_SendDtPackets
 Description:     Send TP.DT packets
 Input:           session - TP session
 Output:          None
 Return:          J1939_OK on success
 Others:          
 *************************************************/
static int16_t J1939Tp_SendDtPackets(J1939TpSession_t *session)
{
    uint8_t data[8];
    uint8_t packetsToSend;
    uint8_t i;
    uint16_t dataOffset;
    int16_t ret;
    
    if (session == NULL)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    /* Calculate packets to send */
    packetsToSend = session->lastPacket - session->nextPacket + 1;
    if (packetsToSend > session->maxPackets)
    {
        packetsToSend = session->maxPackets;
    }
    
    /* Send packets */
    for (i = 0; i < packetsToSend; i++)
    {
        dataOffset = (session->nextPacket - 1) * J1939_TP_DT_DATA_SIZE;
        
        /* Build DT message */
        ret = J1939Tp_BuildDtMessage(session->nextPacket, 
                                    &session->data[dataOffset], data);
        if (ret != J1939_OK)
        {
            return ret;
        }
        
        /* Send via callback */
        ret = J1939Tp_SendCanMessage(J1939_PGN_TP_DATA_TRANSFER, session->destAddr,
                                    data, 8);
        if (ret != J1939_OK)
        {
            return ret;
        }
        
        session->nextPacket++;
    }
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Tp_ProcessRts
 Description:     Process RTS (Request To Send) message
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          Receiver side
 *************************************************/
static void J1939Tp_ProcessRts(const J1939Message_t *msg)
{
    J1939TpSession_t *session;
    uint16_t totalSize;
    uint16_t numPackets;
    uint32_t pgn;
    
    if (msg == NULL || msg->length < 8)
    {
        return;
    }
    
    /* Extract parameters */
    totalSize = msg->data[J1939_TP_CM_BYTE_TOTAL_SIZE_LSB] | 
                ((uint16_t)msg->data[J1939_TP_CM_BYTE_TOTAL_SIZE_MSB] << 8);
    numPackets = msg->data[J1939_TP_CM_BYTE_NUM_PACKETS];
    pgn = (uint32_t)msg->data[J1939_TP_CM_BYTE_PGN_LSB] |
          ((uint32_t)msg->data[J1939_TP_CM_BYTE_PGN_MID] << 8) |
          ((uint32_t)msg->data[J1939_TP_CM_BYTE_PGN_MSB] << 16);
    
    /* Validate parameters */
    if (totalSize > J1939_MAX_DATA_LENGTH || numPackets == 0 || 
        numPackets > J1939_TP_MAX_PACKETS)
    {
        return;
    }
    
    /* Check if session already exists */
    session = J1939Tp_FindSession(msg->sourceAddr, J1939_GetSourceAddress());
    if (session != NULL)
    {
        /* Close existing session */
        J1939Tp_CloseSession(session);
    }
    
    /* Get new session */
    session = J1939Tp_GetFreeSession();
    if (session == NULL)
    {
        /* No free session - send abort */
        J1939TpSession_t tempSession;
        memset(&tempSession, 0, sizeof(J1939TpSession_t));
        tempSession.sourceAddr = msg->sourceAddr;
        tempSession.destAddr = J1939_GetSourceAddress();
        tempSession.pgn = pgn;
        J1939Tp_SendAbort(&tempSession, J1939_TP_ABORT_RESOURCE);
        return;
    }
    
    /* Setup session */
    session->isBroadcast = 0;
    session->pgn = pgn;
    session->sourceAddr = msg->sourceAddr;
    session->destAddr = J1939_GetSourceAddress();
    session->totalSize = totalSize;
    session->totalPackets = numPackets;
    session->maxPackets = J1939_TP_MAX_PACKETS;  /* Default max packets */
    session->nextPacket = 1;
    session->dataIndex = 0;
    session->state = J1939_TP_STATE_RX_SEND_CTS;
    session->lastActivityTime = xTaskGetTickCount();
    
    /* Send CTS */
    J1939Tp_SendCts(session);
    session->state = J1939_TP_STATE_RX_RECEIVING;
}

/*************************************************
 Function:        J1939Tp_ProcessCts
 Description:     Process CTS (Clear To Send) message
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          Transmitter side
 *************************************************/
static void J1939Tp_ProcessCts(const J1939Message_t *msg)
{
    J1939TpSession_t *session;
    uint8_t nextPacket;
    uint8_t numPackets;
    
    if (msg == NULL || msg->length < 8)
    {
        return;
    }
    
    /* Find session */
    session = J1939Tp_FindSession(J1939_GetSourceAddress(), msg->sourceAddr);
    if (session == NULL)
    {
        return;
    }
    
    /* Check session state */
    if (session->state != J1939_TP_STATE_TX_WAIT_CTS)
    {
        return;
    }
    
    /* Extract parameters */
    nextPacket = msg->data[J1939_TP_CM_BYTE_NUM_PACKETS];
    numPackets = msg->data[J1939_TP_CM_BYTE_MAX_PACKETS];
    
    /* Validate */
    if (nextPacket != session->nextPacket || numPackets == 0)
    {
        J1939Tp_AbortSession(session, J1939_TP_ABORT_OTHER);
        return;
    }
    
    /* Update session */
    session->maxPackets = numPackets;
    session->lastPacket = session->nextPacket + numPackets - 1;
    if (session->lastPacket > session->totalPackets)
    {
        session->lastPacket = session->totalPackets;
    }
    session->state = J1939_TP_STATE_TX_DT_SENDING;
    session->lastActivityTime = xTaskGetTickCount();
    
    /* Send DT packets */
    if (J1939Tp_SendDtPackets(session) != J1939_OK)
    {
        J1939Tp_AbortSession(session, J1939_TP_ABORT_OTHER);
        return;
    }
    
    /* Check if complete */
    if (session->nextPacket > session->totalPackets)
    {
        session->state = J1939_TP_STATE_TX_WAIT_EOMA;
    }
    else
    {
        session->state = J1939_TP_STATE_TX_WAIT_CTS;
    }
}

/*************************************************
 Function:        J1939Tp_ProcessEoma
 Description:     Process End of Message Acknowledgment
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          Transmitter side
 *************************************************/
static void J1939Tp_ProcessEoma(const J1939Message_t *msg)
{
    J1939TpSession_t *session;
    
    if (msg == NULL || msg->length < 8)
    {
        return;
    }
    
    /* Find session */
    session = J1939Tp_FindSession(J1939_GetSourceAddress(), msg->sourceAddr);
    if (session == NULL)
    {
        return;
    }
    
    /* Check session state */
    if (session->state != J1939_TP_STATE_TX_WAIT_EOMA)
    {
        return;
    }
    
    /* Notify callback */
    if (g_tpTxCallback != NULL)
    {
        g_tpTxCallback(session->pgn, session->destAddr, J1939_OK);
    }
    
    /* Close session */
    J1939Tp_CloseSession(session);
}

/*************************************************
 Function:        J1939Tp_ProcessBam
 Description:     Process BAM (Broadcast Announce Message)
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          Receiver side - broadcast
 *************************************************/
static void J1939Tp_ProcessBam(const J1939Message_t *msg)
{
    J1939TpSession_t *session;
    uint16_t totalSize;
    uint16_t numPackets;
    uint32_t pgn;
    
    if (msg == NULL || msg->length < 8)
    {
        return;
    }
    
    /* Extract parameters */
    totalSize = msg->data[J1939_TP_CM_BYTE_TOTAL_SIZE_LSB] | 
                ((uint16_t)msg->data[J1939_TP_CM_BYTE_TOTAL_SIZE_MSB] << 8);
    numPackets = msg->data[J1939_TP_CM_BYTE_NUM_PACKETS];
    pgn = (uint32_t)msg->data[J1939_TP_CM_BYTE_PGN_LSB] |
          ((uint32_t)msg->data[J1939_TP_CM_BYTE_PGN_MID] << 8) |
          ((uint32_t)msg->data[J1939_TP_CM_BYTE_PGN_MSB] << 16);
    
    /* Validate */
    if (totalSize > J1939_MAX_DATA_LENGTH || numPackets == 0 || 
        numPackets > J1939_TP_MAX_PACKETS)
    {
        return;
    }
    
    /* Check if session already exists */
    session = J1939Tp_FindSession(msg->sourceAddr, J1939_TP_GLOBAL_ADDRESS);
    if (session != NULL)
    {
        J1939Tp_CloseSession(session);
    }
    
    /* Get new session */
    session = J1939Tp_GetFreeSession();
    if (session == NULL)
    {
        return;  /* Silently ignore BAM if no resources */
    }
    
    /* Setup session */
    session->isBroadcast = 1;
    session->pgn = pgn;
    session->sourceAddr = msg->sourceAddr;
    session->destAddr = J1939_TP_GLOBAL_ADDRESS;
    session->totalSize = totalSize;
    session->totalPackets = numPackets;
    session->nextPacket = 1;
    session->dataIndex = 0;
    session->state = J1939_TP_STATE_BAM_RX;
    session->lastActivityTime = xTaskGetTickCount();
}

/*************************************************
 Function:        J1939Tp_ProcessAbort
 Description:     Process Abort message
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          
 *************************************************/
static void J1939Tp_ProcessAbort(const J1939Message_t *msg)
{
    J1939TpSession_t *session;
    
    if (msg == NULL)
    {
        return;
    }
    
    /* Find session (try both directions) */
    session = J1939Tp_FindSession(J1939_GetSourceAddress(), msg->sourceAddr);
    if (session == NULL)
    {
        session = J1939Tp_FindSession(msg->sourceAddr, J1939_GetSourceAddress());
    }
    
    if (session != NULL)
    {
        /* Notify callback for TX sessions */
        if (session->state == J1939_TP_STATE_TX_RTS_SENT ||
            session->state == J1939_TP_STATE_TX_DT_SENDING)
        {
            if (g_tpTxCallback != NULL)
            {
                g_tpTxCallback(session->pgn, session->destAddr, J1939_ERROR);
            }
        }
        
        J1939Tp_CloseSession(session);
    }
}

/*************************************************
 Function:        J1939Tp_ProcessDt
 Description:     Process TP.DT (Data Transfer) message
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          Receiver side
 *************************************************/
static void J1939Tp_ProcessDt(const J1939Message_t *msg)
{
    J1939TpSession_t *session;
    uint8_t seqNum;
    uint16_t dataOffset;
    uint16_t bytesToCopy;
    
    if (msg == NULL || msg->length < 8)
    {
        return;
    }
    
    /* Get sequence number */
    seqNum = msg->data[J1939_TP_DT_BYTE_SEQ_NUM];
    
    /* Find session (try destination specific and broadcast) */
    session = J1939Tp_FindSession(msg->sourceAddr, J1939_GetSourceAddress());
    if (session == NULL)
    {
        session = J1939Tp_FindSession(msg->sourceAddr, J1939_TP_GLOBAL_ADDRESS);
    }
    
    if (session == NULL)
    {
        return;  /* No active session */
    }
    
    /* Validate sequence number */
    if (seqNum != session->nextPacket)
    {
        if (!session->isBroadcast)
        {
            J1939Tp_AbortSession(session, J1939_TP_ABORT_OTHER);
        }
        return;
    }
    
    /* Calculate data position */
    dataOffset = (seqNum - 1) * J1939_TP_DT_DATA_SIZE;
    bytesToCopy = J1939_TP_DT_DATA_SIZE;
    
    /* Check for last packet */
    if (seqNum == session->totalPackets)
    {
        bytesToCopy = session->totalSize - dataOffset;
    }
    
    /* Copy data */
    if ((dataOffset + bytesToCopy) <= J1939_MAX_DATA_LENGTH)
    {
        memcpy(&session->data[dataOffset], 
               &msg->data[J1939_TP_DT_DATA_START], bytesToCopy);
        session->dataIndex += bytesToCopy;
    }
    
    session->nextPacket++;
    session->lastActivityTime = xTaskGetTickCount();
    
    /* Check if complete */
    if (seqNum == session->totalPackets)
    {
        /* Message complete */
        if (!session->isBroadcast)
        {
            /* Send EOMA for non-broadcast */
            J1939Tp_SendEoma(session);
        }
        
        /* Notify callback */
        if (g_tpRxCallback != NULL)
        {
            g_tpRxCallback(session->pgn, session->sourceAddr, 
                          session->data, session->totalSize);
        }
        
        J1939Tp_CloseSession(session);
    }
    else if (!session->isBroadcast && seqNum == session->lastPacket)
    {
        /* Need more packets - send next CTS */
        session->state = J1939_TP_STATE_RX_SEND_CTS;
        J1939Tp_SendCts(session);
        session->state = J1939_TP_STATE_RX_RECEIVING;
    }
}

/*************************************************
 Function:        J1939Tp_CheckTimeouts
 Description:     Check and handle TP timeouts
 Input:           currentTime - Current timestamp
 Output:          None
 Return:          None
 Others:          
 *************************************************/
static void J1939Tp_CheckTimeouts(uint32_t currentTime)
{
    uint8_t i;
    J1939TpSession_t *session;
    uint32_t elapsedTime;
    
    for (i = 0; i < J1939_TP_MAX_SESSIONS; i++)
    {
        session = &g_tpSessions[i];
        
        if (!session->inUse)
        {
            continue;
        }
        
        elapsedTime = currentTime - session->lastActivityTime;
        
        switch (session->state)
        {
            case J1939_TP_STATE_TX_RTS_SENT:
                /* Waiting for CTS */
                if (elapsedTime > J1939_TP_T3_TIMEOUT)
                {
                    J1939Tp_AbortSession(session, J1939_TP_ABORT_TIMEOUT);
                }
                break;
                
            case J1939_TP_STATE_TX_WAIT_CTS:
                /* Waiting for CTS after sending DT */
                if (elapsedTime > J1939_TP_T3_TIMEOUT)
                {
                    J1939Tp_AbortSession(session, J1939_TP_ABORT_TIMEOUT);
                }
                break;
                
            case J1939_TP_STATE_TX_WAIT_EOMA:
                /* Waiting for End of Message Ack */
                if (elapsedTime > J1939_TP_T1_TIMEOUT)
                {
                    J1939Tp_AbortSession(session, J1939_TP_ABORT_TIMEOUT);
                }
                break;
                
            case J1939_TP_STATE_RX_RECEIVING:
                /* Waiting for DT packets */
                if (elapsedTime > J1939_TP_T1_TIMEOUT)
                {
                    J1939Tp_AbortSession(session, J1939_TP_ABORT_TIMEOUT);
                }
                break;
                
            case J1939_TP_STATE_BAM_RX:
                /* BAM receive timeout */
                if (elapsedTime > J1939_TP_T2_TIMEOUT)
                {
                    J1939Tp_CloseSession(session);
                }
                break;
                
            default:
                break;
        }
    }
}

/****************************** Public Function Implementations ***************/

/*************************************************
 Function:        J1939Tp_Init
 Description:     Initialize J1939 Transport Protocol
 Input:           sendCallback - CAN报文发送回调函数
 Output:          None
 Return:          J1939_OK on success
 Others:          回调函数由SDK层提供，用于发送CAN报文
 *************************************************/
int16_t J1939Tp_Init(J1939TpSendCanMsgCallback_t sendCallback)
{
    if (g_tpInitialized)
    {
        return J1939_OK;
    }
    
    if (sendCallback == NULL)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    /* Clear all sessions */
    memset(g_tpSessions, 0, sizeof(g_tpSessions));
    
    g_tpSendCallback = sendCallback;
    g_tpRxCallback = NULL;
    g_tpTxCallback = NULL;
    g_tpInitialized = 1;
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Tp_Deinit
 Description:     Deinitialize J1939 Transport Protocol
 Input:           None
 Output:          None
 Return:          J1939_OK on success
 Others:          
 *************************************************/
int16_t J1939Tp_Deinit(void)
{
    uint8_t i;
    
    /* Close all sessions */
    for (i = 0; i < J1939_TP_MAX_SESSIONS; i++)
    {
        if (g_tpSessions[i].inUse)
        {
            J1939Tp_CloseSession(&g_tpSessions[i]);
        }
    }
    
    g_tpRxCallback = NULL;
    g_tpTxCallback = NULL;
    g_tpInitialized = 0;
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Tp_Transmit
 Description:     Transmit data using TP (RTS/CTS)
 Input:           pgn - Parameter Group Number
                  destAddr - Destination address
                  data - Data to transmit
                  length - Data length
 Output:          None
 Return:          J1939_OK on success
 Others:          For data > 8 bytes
 *************************************************/
int16_t J1939Tp_Transmit(uint32_t pgn, uint8_t destAddr, 
                        const uint8_t *data, uint16_t length)
{
    J1939TpSession_t *session;
    uint8_t numPackets;
    uint8_t cmData[8];
    int16_t ret;
    
    if (!g_tpInitialized)
    {
        return J1939_ERROR_NOT_INITIALIZED;
    }
    
    if (data == NULL || length == 0 || length > J1939_MAX_DATA_LENGTH)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    /* For short messages, use direct transmission */
    if (length <= 8)
    {
        return J1939Tp_SendCanMessage(pgn, destAddr, data, length);
    }
    
    /* Calculate number of packets */
    numPackets = (uint8_t)((length + J1939_TP_DT_DATA_SIZE - 1) / J1939_TP_DT_DATA_SIZE);
    
    /* Get free session */
    session = J1939Tp_GetFreeSession();
    if (session == NULL)
    {
        return J1939_ERROR_NO_RESOURCE;
    }
    
    /* Setup session */
    session->isBroadcast = 0;
    session->pgn = pgn;
    session->sourceAddr = J1939_GetSourceAddress();
    session->destAddr = destAddr;
    session->totalSize = length;
    session->totalPackets = numPackets;
    session->maxPackets = J1939_TP_MAX_PACKETS;
    session->nextPacket = 1;
    session->dataIndex = length;
    memcpy(session->data, data, length);
    
    /* Build and send RTS */
    ret = J1939Tp_BuildCmMessage(J1939_TP_CM_RTS, length, numPackets, 
                                J1939_TP_MAX_PACKETS, pgn, cmData);
    if (ret != J1939_OK)
    {
        J1939Tp_CloseSession(session);
        return ret;
    }
    
    ret = J1939Tp_SendCanMessage(J1939_PGN_TP_CONNECTION_MANAGE, destAddr,
                                cmData, 8);
    if (ret != J1939_OK)
    {
        J1939Tp_CloseSession(session);
        return ret;
    }

    session->state = J1939_TP_STATE_TX_RTS_SENT;
    session->lastActivityTime = xTaskGetTickCount();
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Tp_TransmitBam
 Description:     Transmit data using BAM (Broadcast)
 Input:           pgn - Parameter Group Number
                  data - Data to transmit
                  length - Data length
 Output:          None
 Return:          J1939_OK on success
 Others:          Broadcast transmission, no CTS/EOMA
 *************************************************/
int16_t J1939Tp_TransmitBam(uint32_t pgn, const uint8_t *data, uint16_t length)
{
    // J1939TpSession_t *session;
    uint8_t numPackets;
    uint8_t cmData[8];
    uint8_t i;
    uint16_t dataOffset;
    uint8_t dtData[8];
    int16_t ret;
    
    if (!g_tpInitialized)
    {
        return J1939_ERROR_NOT_INITIALIZED;
    }
    
    if (data == NULL || length == 0 || length > J1939_MAX_DATA_LENGTH)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    /* Calculate number of packets */
    numPackets = (uint8_t)((length + J1939_TP_DT_DATA_SIZE - 1) / J1939_TP_DT_DATA_SIZE);
    
    /* Send BAM */
    ret = J1939Tp_BuildCmMessage(J1939_TP_CM_BAM, length, numPackets, 0xFF, pgn, cmData);
    if (ret != J1939_OK)
    {
        return ret;
    }
    
    ret = J1939Tp_SendCanMessage(J1939_PGN_TP_CONNECTION_MANAGE, J1939_TP_GLOBAL_ADDRESS,
                                cmData, 8);
    if (ret != J1939_OK)
    {
        return ret;
    }

    /* Wait T2 (50-200ms) before sending DT packets */
    vTaskDelay(pdMS_TO_TICKS(50));
    
    /* Send DT packets */
    for (i = 0; i < numPackets; i++)
    {
        dataOffset = i * J1939_TP_DT_DATA_SIZE;
        
        memset(dtData, 0xFF, 8);
        dtData[0] = i + 1;  /* Sequence number */
        
        /* Copy data */
        if (dataOffset < length)
        {
            uint16_t bytesToCopy = length - dataOffset;
            if (bytesToCopy > J1939_TP_DT_DATA_SIZE)
            {
                bytesToCopy = J1939_TP_DT_DATA_SIZE;
            }
            memcpy(&dtData[1], &data[dataOffset], bytesToCopy);
        }
        
        ret = J1939Tp_SendCanMessage(J1939_PGN_TP_DATA_TRANSFER, J1939_TP_GLOBAL_ADDRESS,
                                    dtData, 8);
        if (ret != J1939_OK)
        {
            return ret;
        }

        /* T3 delay between packets (10-200ms) */
        if (i < (numPackets - 1))
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    
    /* Notify callback */
    if (g_tpTxCallback != NULL)
    {
        g_tpTxCallback(pgn, J1939_TP_GLOBAL_ADDRESS, J1939_OK);
    }
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Tp_ProcessCmMessage
 Description:     Process TP.CM (Connection Management) message
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          Called by J1939 SDK when PGN 0xEC00 received
 *************************************************/
void J1939Tp_ProcessCmMessage(const J1939Message_t *msg)
{
    uint8_t control;
    
    if (msg == NULL || msg->length < 8)
    {
        return;
    }
    
    control = msg->data[J1939_TP_CM_BYTE_CONTROL];
    
    switch (control)
    {
        case J1939_TP_CM_RTS:
            J1939Tp_ProcessRts(msg);
            break;
            
        case J1939_TP_CM_CTS:
            J1939Tp_ProcessCts(msg);
            break;
            
        case J1939_TP_CM_EOMA:
            J1939Tp_ProcessEoma(msg);
            break;
            
        case J1939_TP_CM_BAM:
            J1939Tp_ProcessBam(msg);
            break;
            
        case J1939_TP_CM_ABORT:
            J1939Tp_ProcessAbort(msg);
            break;
            
        default:
            break;
    }
}

/*************************************************
 Function:        J1939Tp_ProcessDtMessage
 Description:     Process TP.DT (Data Transfer) message
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          Called by J1939 SDK when PGN 0xEB00 received
 *************************************************/
void J1939Tp_ProcessDtMessage(const J1939Message_t *msg)
{
    J1939Tp_ProcessDt(msg);
}

/*************************************************
 Function:        J1939Tp_CycleProcess
 Description:     TP cycle processing
 Input:           currentTime - Current timestamp
 Output:          None
 Return:          None
 Others:          Must be called periodically
 *************************************************/
void J1939Tp_CycleProcess(uint32_t currentTime)
{
    if (!g_tpInitialized)
    {
        return;
    }
    
    J1939Tp_CheckTimeouts(currentTime);
}

/*************************************************
 Function:        J1939Tp_RegisterRxCallback
 Description:     Register TP RX complete callback
 Input:           callback - Callback function
 Output:          None
 Return:          None
 Others:          
 *************************************************/
void J1939Tp_RegisterRxCallback(J1939TpRxCompleteCallback_t callback)
{
    g_tpRxCallback = callback;
}

/*************************************************
 Function:        J1939Tp_RegisterTxCallback
 Description:     Register TP TX complete callback
 Input:           callback - Callback function
 Output:          None
 Return:          None
 Others:          
 *************************************************/
void J1939Tp_RegisterTxCallback(J1939TpTxCompleteCallback_t callback)
{
    g_tpTxCallback = callback;
}

/*************************************************
 Function:        J1939Tp_IsBusy
 Description:     Check if TP is busy
 Input:           None
 Output:          None
 Return:          1 if busy, 0 if idle
 Others:          
 *************************************************/
uint8_t J1939Tp_IsBusy(void)
{
    uint8_t i;
    
    for (i = 0; i < J1939_TP_MAX_SESSIONS; i++)
    {
        if (g_tpSessions[i].inUse)
        {
            return 1;
        }
    }
    
    return 0;
}

/*************************************************
 Function:        J1939Tp_GetActiveSessions
 Description:     Get number of active TP sessions
 Input:           None
 Output:          None
 Return:          Number of active sessions
 Others:          
 *************************************************/
uint8_t J1939Tp_GetActiveSessions(void)
{
    uint8_t i;
    uint8_t count = 0;
    
    for (i = 0; i < J1939_TP_MAX_SESSIONS; i++)
    {
        if (g_tpSessions[i].inUse)
        {
            count++;
        }
    }
    
    return count;
}
