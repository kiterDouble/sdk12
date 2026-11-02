/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: j1939Sdk.c
 Author: 
 Created Time: 
 Description: J1939 Protocol SDK Implementation
              SAE J1939-21 Data Link Layer
              SAE J1939-81 Network Management
 Others: 
 *************************************************/

/****************************** include ***************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "j1939Sdk.h"
#include "j1939Tp.h"
#include "logHal.h"
#include "timerHal.h"

/****************************** Macro Definitions ******************************/
#define J1939_CAN_HANDLE_INVALID        (-1)

/****************************** Type Definitions ******************************/
typedef struct
{
    uint32_t pgn;
    J1939RxCallback_t callback;
    uint8_t registered;
} J1939PgnEntry_t;

typedef struct
{
    J1939DeviceConfig_t config;
    J1939State_e state;
    int16_t canHandle;
    
    /* PGN Registration Table */
    J1939PgnEntry_t pgnTable[J1939_MAX_PGN_NUM];
    uint8_t pgnCount;
    
    /* TX PGN Configuration */
    J1939PgnConfig_t txPgnTable[J1939_MAX_TX_PGN_NUM];
    uint8_t txPgnCount;
    
    /* Statistics */
    uint16_t rxCounter;
    uint16_t txCounter;
    uint16_t errorCounter;
    
    /* Receive Buffer */
    CanHalMsg_t rxBuffer[10];
} J1939Ctrl_t;

/****************************** Global Variables ******************************/
static J1939Ctrl_t g_j1939Ctrl;

/****************************** Function Declarations *************************/
static void J1939ProcessReceivedFrame(const CanHalMsg_t *canMsg);
static void J1939TransmitPeriodicPgns(uint32_t currentTime);
static int16_t J1939FindPgnEntry(uint32_t pgn);
static int16_t J1939FindTxPgnEntry(uint32_t pgn);
static int16_t J1939TpSendCallback(uint32_t pgn, uint8_t destAddr, 
                                    const uint8_t *data, uint8_t length);

/****************************** Private Function Implementations **************/

/*************************************************
 Function:        J1939TpSendCallback
 Description:     TP层发送回调函数 - 供TP层调用以发送CAN报文
 Input:           pgn - PGN
                  destAddr - 目标地址
                  data - 数据
                  length - 数据长度
 Output:          None
 Return:          J1939_OK on success
 Others:          这是TP层和SDK层之间的接口，使用默认优先级3
 *************************************************/
static int16_t J1939TpSendCallback(uint32_t pgn, uint8_t destAddr, 
                                    const uint8_t *data, uint8_t length)
{
    return J1939Sdk_Transmit(pgn, destAddr, data, length);
}

/*************************************************
 Function:        J1939FindPgnEntry
 Description:     Find PGN entry in registration table
 Input:           pgn - Parameter Group Number
 Output:          None
 Return:          Index on success, -1 on failure
 Others:          
 *************************************************/
static int16_t J1939FindPgnEntry(uint32_t pgn)
{
    uint8_t i;
    
    for (i = 0; i < g_j1939Ctrl.pgnCount; i++)
    {
        if (g_j1939Ctrl.pgnTable[i].registered && 
            g_j1939Ctrl.pgnTable[i].pgn == pgn)
        {
            return i;
        }
    }
    
    return -1;
}

/*************************************************
 Function:        J1939FindTxPgnEntry
 Description:     Find TX PGN entry in configuration table
 Input:           pgn - Parameter Group Number
 Output:          None
 Return:          Index on success, -1 on failure
 Others:          
 *************************************************/
static int16_t J1939FindTxPgnEntry(uint32_t pgn)
{
    uint8_t i;
    
    for (i = 0; i < g_j1939Ctrl.txPgnCount; i++)
    {
        if (g_j1939Ctrl.txPgnTable[i].pgn == pgn)
        {
            return i;
        }
    }
    
    return -1;
}

/*************************************************
 Function:        J1939ProcessReceivedFrame
 Description:     Process received CAN frame and dispatch to callback
 Input:           canMsg - Received CAN message
 Output:          None
 Return:          None
 Others:          
 *************************************************/
static void J1939ProcessReceivedFrame(const CanHalMsg_t *canMsg)
{
    J1939Message_t j1939Msg;
    int16_t entryIndex;
    uint8_t priority;
    uint32_t pgn;
    uint8_t sa;
    uint8_t da;
    
    /* Parse 29-bit CAN ID */
    if (J1939_ParseCanId(canMsg->canId, &priority, &pgn, &sa, &da) != J1939_OK)
    {
        return;
    }
    
    /* Skip if source address is our own */
    if (sa == g_j1939Ctrl.config.sourceAddress)
    {
        return;
    }
    
    /* Build J1939 message */
    j1939Msg.pgn = pgn;
    j1939Msg.sourceAddr = sa;
    j1939Msg.destAddr = da;
    j1939Msg.priority = priority;
    j1939Msg.length = canMsg->dlc;
    
    if (j1939Msg.length > 8)
    {
        j1939Msg.length = 8;
    }
    
    memcpy(j1939Msg.data, canMsg->canData, j1939Msg.length);
    
    /* Handle Transport Protocol PGNs internally */
    if (pgn == J1939_PGN_TP_CONNECTION_MANAGE)
    {
        /* TP.CM - Connection Management */
        J1939Tp_ProcessCmMessage(&j1939Msg);
        g_j1939Ctrl.rxCounter++;
        return;
    }
    else if (pgn == J1939_PGN_TP_DATA_TRANSFER)
    {
        /* TP.DT - Data Transfer */
        J1939Tp_ProcessDtMessage(&j1939Msg);
        g_j1939Ctrl.rxCounter++;
        return;
    }
    
    /* Find and call registered callback */
    entryIndex = J1939FindPgnEntry(pgn);
    if (entryIndex >= 0 && g_j1939Ctrl.pgnTable[entryIndex].callback != NULL)
    {
        g_j1939Ctrl.pgnTable[entryIndex].callback(&j1939Msg);
    }
    
    g_j1939Ctrl.rxCounter++;
}

/*************************************************
 Function:        J1939TransmitPeriodicPgns
 Description:     Transmit periodic PGNs based on configured periods
 Input:           currentTime - Current timestamp
 Output:          None
 Return:          None
 Others:          Called from cycle process
 *************************************************/
static void J1939TransmitPeriodicPgns(uint32_t currentTime)
{
    uint8_t i;
    J1939PgnConfig_t *pgnCfg;
    
    for (i = 0; i < g_j1939Ctrl.txPgnCount; i++)
    {
        pgnCfg = &g_j1939Ctrl.txPgnTable[i];
        
        if (pgnCfg->txPeriod == 0)
        {
            continue;  /* Event triggered, not periodic */
        }
        
        if ((currentTime - pgnCfg->lastTxTime) >= pgnCfg->txPeriod)
        {
            /* Time to transmit - application should fill data */
            pgnCfg->lastTxTime = currentTime;
        }
    }
}

/****************************** Public Function Implementations ***************/

/*************************************************
 Function:        J1939Sdk_Init
 Description:     Initialize J1939 SDK
 Input:           config - Device configuration
 Output:          None
 Return:          J1939_OK on success, error code on failure
 Others:          Must be called before using other J1939 functions
 *************************************************/
int16_t J1939Sdk_Init(const J1939DeviceConfig_t *config)
{
    CanHalMsgFilter_t filter;
    int16_t ret;

    if (config == NULL)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    if (g_j1939Ctrl.state != J1939_STATE_UNINITIALIZED)
    {
        return J1939_ERROR;
    }
    
    /* Copy configuration */
    memcpy(&g_j1939Ctrl.config, config, sizeof(J1939DeviceConfig_t));
    
    /* Initialize control structure */
    g_j1939Ctrl.state = J1939_STATE_INITIALIZED;
    g_j1939Ctrl.canHandle = J1939_CAN_HANDLE_INVALID;
    g_j1939Ctrl.pgnCount = 0;
    g_j1939Ctrl.txPgnCount = 0;
    g_j1939Ctrl.rxCounter = 0;
    g_j1939Ctrl.txCounter = 0;
    g_j1939Ctrl.errorCounter = 0;
    
    memset(g_j1939Ctrl.pgnTable, 0, sizeof(g_j1939Ctrl.pgnTable));
    memset(g_j1939Ctrl.txPgnTable, 0, sizeof(g_j1939Ctrl.txPgnTable));
    memset(g_j1939Ctrl.rxBuffer, 0, sizeof(g_j1939Ctrl.rxBuffer));
    
    /* Open CAN channel */
    g_j1939Ctrl.canHandle = CanHalOpen(J1939_CAN_CHANNEL);
    if (g_j1939Ctrl.canHandle < 0)
    {
        g_j1939Ctrl.state = J1939_STATE_ERROR;
        return J1939_ERROR;
    }
    
    /* Configure CAN filter for extended frames (29-bit) */
    filter.useStdFilter = 0;
    filter.useExdtFilter = 1;
    filter.canIdExtdMin = 0x00000000;
    filter.canIdExtdMax = 0x1FFFFFFF;
    filter.canIdStdMin = 0;
    filter.canIdStdMax = 0;
    
    if (CanHalSetFilter(g_j1939Ctrl.canHandle, &filter) != 0)
    {
        g_j1939Ctrl.state = J1939_STATE_ERROR;
        return J1939_ERROR;
    }
    
    /* Set RX buffer */
    CanHalSetRxBuffer(g_j1939Ctrl.canHandle, g_j1939Ctrl.rxBuffer, 10);
    
    /* Initialize Transport Protocol layer with send callback */
    ret = J1939Tp_Init(J1939TpSendCallback);
    if (ret != J1939_OK)
    {
        g_j1939Ctrl.state = J1939_STATE_ERROR;
        return J1939_ERROR;
    }
    
    g_j1939Ctrl.state = J1939_STATE_RUNNING;
    
    TBOX_PRINT("J1939 SDK initialized on CAN%d, SA=0x%02X\r\n", 
               J1939_CAN_CHANNEL + 1, config->sourceAddress);
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Sdk_Deinit
 Description:     Deinitialize J1939 SDK
 Input:           None
 Output:          None
 Return:          J1939_OK on success
 Others:          
 *************************************************/
int16_t J1939Sdk_Deinit(void)
{
    if (g_j1939Ctrl.canHandle >= 0)
    {
        /* Close CAN channel */
        g_j1939Ctrl.canHandle = J1939_CAN_HANDLE_INVALID;
    }
    
    g_j1939Ctrl.state = J1939_STATE_UNINITIALIZED;
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Sdk_RegisterPgn
 Description:     Register a PGN with callback function
 Input:           pgn - Parameter Group Number
                  callback - Callback function for received messages
 Output:          None
 Return:          J1939_OK on success, error code on failure
 Others:          
 *************************************************/
int16_t J1939Sdk_RegisterPgn(uint32_t pgn, J1939RxCallback_t callback)
{
    int16_t entryIndex;
    
    if (g_j1939Ctrl.state != J1939_STATE_RUNNING)
    {
        return J1939_ERROR_NOT_INITIALIZED;
    }
    
    if (callback == NULL)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    /* Check if already registered */
    entryIndex = J1939FindPgnEntry(pgn);
    if (entryIndex >= 0)
    {
        /* Update existing entry */
        g_j1939Ctrl.pgnTable[entryIndex].callback = callback;
        return J1939_OK;
    }
    
    /* Find empty slot */
    if (g_j1939Ctrl.pgnCount >= J1939_MAX_PGN_NUM)
    {
        return J1939_ERROR_NO_RESOURCE;
    }
    
    /* Add new entry */
    g_j1939Ctrl.pgnTable[g_j1939Ctrl.pgnCount].pgn = pgn;
    g_j1939Ctrl.pgnTable[g_j1939Ctrl.pgnCount].callback = callback;
    g_j1939Ctrl.pgnTable[g_j1939Ctrl.pgnCount].registered = 1;
    g_j1939Ctrl.pgnCount++;
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Sdk_UnregisterPgn
 Description:     Unregister a PGN
 Input:           pgn - Parameter Group Number
 Output:          None
 Return:          J1939_OK on success, error code on failure
 Others:          
 *************************************************/
int16_t J1939Sdk_UnregisterPgn(uint32_t pgn)
{
    int16_t entryIndex;
    
    if (g_j1939Ctrl.state != J1939_STATE_RUNNING)
    {
        return J1939_ERROR_NOT_INITIALIZED;
    }
    
    entryIndex = J1939FindPgnEntry(pgn);
    if (entryIndex < 0)
    {
        return J1939_ERROR_PGN_NOT_FOUND;
    }
    
    /* Mark as unregistered */
    g_j1939Ctrl.pgnTable[entryIndex].registered = 0;
    g_j1939Ctrl.pgnTable[entryIndex].callback = NULL;
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Sdk_ConfigTxPgn
 Description:     Configure periodic transmission of a PGN
 Input:           pgn - Parameter Group Number
                  priority - Priority (0-7)
                  period - Transmission period in ms (0=event triggered)
 Output:          None
 Return:          J1939_OK on success, error code on failure
 Others:          
 *************************************************/
int16_t J1939Sdk_ConfigTxPgn(uint32_t pgn, uint8_t priority, uint16_t period)
{
    int16_t entryIndex;
    
    if (g_j1939Ctrl.state != J1939_STATE_RUNNING)
    {
        return J1939_ERROR_NOT_INITIALIZED;
    }
    
    if (priority > 7)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    /* Check if already configured */
    entryIndex = J1939FindTxPgnEntry(pgn);
    if (entryIndex >= 0)
    {
        /* Update existing entry */
        g_j1939Ctrl.txPgnTable[entryIndex].priority = priority;
        g_j1939Ctrl.txPgnTable[entryIndex].txPeriod = period;
        return J1939_OK;
    }
    
    /* Find empty slot */
    if (g_j1939Ctrl.txPgnCount >= J1939_MAX_TX_PGN_NUM)
    {
        return J1939_ERROR_NO_RESOURCE;
    }
    
    /* Add new entry */
    g_j1939Ctrl.txPgnTable[g_j1939Ctrl.txPgnCount].pgn = pgn;
    g_j1939Ctrl.txPgnTable[g_j1939Ctrl.txPgnCount].priority = priority;
    g_j1939Ctrl.txPgnTable[g_j1939Ctrl.txPgnCount].txPeriod = period;
    g_j1939Ctrl.txPgnTable[g_j1939Ctrl.txPgnCount].lastTxTime = 0;
    g_j1939Ctrl.txPgnCount++;
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Sdk_Transmit
 Description:     Transmit a J1939 message
 Input:           pgn - Parameter Group Number
                  destAddr - Destination address (0xFF for broadcast)
                  data - Data payload
                  length - Data length (max 8 bytes for single frame)
 Output:          None
 Return:          J1939_OK on success, error code on failure
 Others:          For multi-frame messages, use J1939TpTransmit
 *************************************************/
int16_t J1939Sdk_Transmit(uint32_t pgn, uint8_t destAddr, 
                         const uint8_t *data, uint16_t length)
{
    return J1939Sdk_TransmitPri(pgn, destAddr, g_j1939Ctrl.config.priority, 
                               data, length);
}

/*************************************************
 Function:        J1939Sdk_TransmitPri
 Description:     Transmit a J1939 message with specified priority
 Input:           pgn - Parameter Group Number
                  destAddr - Destination address (0xFF for broadcast)
                  priority - Priority (0-7)
                  data - Data payload
                  length - Data length (max 8 bytes for single frame)
 Output:          None
 Return:          J1939_OK on success, error code on failure
 Others:          
 *************************************************/
int16_t J1939Sdk_TransmitPri(uint32_t pgn, uint8_t destAddr, uint8_t priority,
                            const uint8_t *data, uint16_t length)
{
    CanHalMsg_t canMsg;
    uint32_t canId;
    
    if (g_j1939Ctrl.state != J1939_STATE_RUNNING)
    {
        return J1939_ERROR_NOT_INITIALIZED;
    }
    
    if (data == NULL || length == 0 || length > 8)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    /* Build CAN ID from PGN */
    canId = J1939_MakeCanId(priority, pgn, g_j1939Ctrl.config.sourceAddress);
    
    /* Prepare CAN message */
    canMsg.canId = canId;
    canMsg.dlc = length;
    memcpy(canMsg.canData, data, length);
    canMsg.txFlag = 0;
    
    /* Transmit via CAN HAL */
    if (CanHalTransmitQueued(g_j1939Ctrl.canHandle, canMsg.canId,
                       canMsg.canData, canMsg.dlc, 0, CAN_TX_PRIO_NORMAL) != 0)
    {
        g_j1939Ctrl.errorCounter++;
        return J1939_ERROR;
    }
    
    g_j1939Ctrl.txCounter++;
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939Sdk_ProcessRxMessage
 Description:     Process a received CAN message (called from HAL callback)
 Input:           canMsg - Received CAN message
 Output:          None
 Return:          None
 Others:          Can be called directly or from interrupt
 *************************************************/
void J1939Sdk_ProcessRxMessage(const CanHalMsg_t *canMsg)
{
    if (g_j1939Ctrl.state != J1939_STATE_RUNNING)
    {
        return;
    }
    
    if (canMsg == NULL)
    {
        return;
    }
    
    J1939ProcessReceivedFrame(canMsg);
}

/*************************************************
 Function:        J1939Sdk_CycleProcess
 Description:     J1939 cycle processing function
 Input:           cycleTime - Cycle time in ms
 Output:          None
 Return:          None
 Others:          Must be called periodically (e.g., every 5-10ms)
 *************************************************/
void J1939Sdk_CycleProcess(uint32_t cycleTime)
{
    CanHalMsg_t rxMsg;
    int16_t result;
    uint32_t currentTime;
    
    (void)cycleTime;
    
    if (g_j1939Ctrl.state != J1939_STATE_RUNNING)
    {
        return;
    }
    
    /* Get current time */
    currentTime = xTaskGetTickCount();  /* Use FreeRTOS tick instead of TimerHalGetMs */
    
    /* Process received messages */
    do {
        result = CanHalReceive(g_j1939Ctrl.canHandle, &rxMsg, 0);
        if (result == 0)
        {
            J1939Sdk_ProcessRxMessage(&rxMsg);
        }
    } while (result == 0);
    
    /* Process periodic transmissions */
    J1939TransmitPeriodicPgns(currentTime);
    
    /* Process Transport Protocol timeouts */
    J1939Tp_CycleProcess(currentTime);
}

/*************************************************
 Function:        J1939_MakeCanId
 Description:     Build 29-bit CAN ID from J1939 parameters
 Input:           priority - Priority (0-7)
                  pgn - Parameter Group Number
                  sa - Source Address
 Output:          None
 Return:          29-bit CAN ID
 Others:          
 *************************************************/
uint32_t J1939_MakeCanId(uint8_t priority, uint32_t pgn, uint8_t sa)
{
    uint32_t canId = 0;
    
    /* Priority: bits 26-28 */
    canId |= ((uint32_t)(priority & 0x07) << 26);
    
    /* PGN: bits 8-25 */
    canId |= ((pgn & 0x03FFFF) << 8);
    
    /* Source Address: bits 0-7 */
    canId |= (sa & 0xFF);
    
    return canId;
}

/*************************************************
 Function:        J1939_ParseCanId
 Description:     Parse 29-bit CAN ID into J1939 parameters
 Input:           canId - 29-bit CAN ID
                  priority - Output: Priority
                  pgn - Output: PGN
                  sa - Output: Source Address
                  da - Output: Destination Address
 Output:          priority, pgn, sa, da
 Return:          J1939_OK on success
 Others:          For PDU2 format (broadcast), da will be 0xFF
 *************************************************/
int16_t J1939_ParseCanId(uint32_t canId, uint8_t *priority, uint32_t *pgn, 
                        uint8_t *sa, uint8_t *da)
{
    uint8_t pf;
    uint8_t ps;
    
    if (priority == NULL || pgn == NULL || sa == NULL || da == NULL)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    /* Extract Priority: bits 26-28 */
    *priority = (uint8_t)((canId >> 26) & 0x07);
    
    /* Extract PDU Format: bits 16-23 */
    pf = (uint8_t)((canId >> 16) & 0xFF);
    
    /* Extract PDU Specific: bits 8-15 */
    ps = (uint8_t)((canId >> 8) & 0xFF);
    
    /* Extract Source Address: bits 0-7 */
    *sa = (uint8_t)(canId & 0xFF);
    
    /* Calculate PGN based on PDU Format */
    if (pf < 240)
    {
        /* PDU1 Format: PS = Destination Address */
        *pgn = ((uint32_t)pf << 8);
        *da = ps;
    }
    else
    {
        /* PDU2 Format: PS = Group Extension */
        *pgn = (((uint32_t)pf << 8) | ps);
        *da = J1939_GLOBAL_ADDRESS;
    }
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939_IsBroadcastPgn
 Description:     Check if PGN is broadcast type (PDU2 format)
 Input:           pgn - Parameter Group Number
 Output:          None
 Return:          1 if broadcast, 0 if destination specific
 Others:          
 *************************************************/
uint8_t J1939_IsBroadcastPgn(uint32_t pgn)
{
    uint8_t pf = (uint8_t)((pgn >> 8) & 0xFF);
    return (pf >= 240) ? 1 : 0;
}

/*************************************************
 Function:        J1939_IsDestinationSpecific
 Description:     Check if PGN is destination specific (PDU1 format)
 Input:           pgn - Parameter Group Number
 Output:          None
 Return:          1 if destination specific, 0 if broadcast
 Others:          
 *************************************************/
uint8_t J1939_IsDestinationSpecific(uint32_t pgn)
{
    return J1939_IsBroadcastPgn(pgn) ? 0 : 1;
}

/*************************************************
 Function:        J1939_SetSourceAddress
 Description:     Set J1939 source address
 Input:           address - New source address (0-253)
 Output:          None
 Return:          J1939_OK on success
 Others:          
 *************************************************/
int16_t J1939_SetSourceAddress(uint8_t address)
{
    if (address > J1939_MAX_ADDRESS)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    g_j1939Ctrl.config.sourceAddress = address;
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939_GetSourceAddress
 Description:     Get current J1939 source address
 Input:           None
 Output:          None
 Return:          Current source address
 Others:          
 *************************************************/
uint8_t J1939_GetSourceAddress(void)
{
    return g_j1939Ctrl.config.sourceAddress;
}

/*************************************************
 Function:        J1939_ClaimAddress
 Description:     Claim a J1939 address (TODO: implement address claim protocol)
 Input:           address - Address to claim
 Output:          None
 Return:          J1939_OK on success
 Others:          Currently just sets the address
 *************************************************/
int16_t J1939_ClaimAddress(uint8_t address)
{
    return J1939_SetSourceAddress(address);
}

/*************************************************
 Function:        J1939Sdk_GetState
 Description:     Get J1939 SDK state
 Input:           None
 Output:          None
 Return:          Current state
 Others:          
 *************************************************/
J1939State_e J1939Sdk_GetState(void)
{
    return g_j1939Ctrl.state;
}

/*************************************************
 Function:        J1939Sdk_GetRxCounter
 Description:     Get received message counter
 Input:           None
 Output:          None
 Return:          RX counter value
 Others:          
 *************************************************/
uint16_t J1939Sdk_GetRxCounter(void)
{
    return g_j1939Ctrl.rxCounter;
}

/*************************************************
 Function:        J1939Sdk_GetTxCounter
 Description:     Get transmitted message counter
 Input:           None
 Output:          None
 Return:          TX counter value
 Others:          
 *************************************************/
uint16_t J1939Sdk_GetTxCounter(void)
{
    return g_j1939Ctrl.txCounter;
}

/*************************************************
 Function:        J1939Sdk_GetStatistics
 Description:     Get J1939 statistics
 Input:           None
 Output:          rxCount, txCount, errCount
 Return:          None
 Others:          
 *************************************************/
void J1939Sdk_GetStatistics(uint16_t *rxCount, uint16_t *txCount, uint16_t *errCount)
{
    if (rxCount != NULL)
    {
        *rxCount = g_j1939Ctrl.rxCounter;
    }
    
    if (txCount != NULL)
    {
        *txCount = g_j1939Ctrl.txCounter;
    }
    
    if (errCount != NULL)
    {
        *errCount = g_j1939Ctrl.errorCounter;
    }
}
