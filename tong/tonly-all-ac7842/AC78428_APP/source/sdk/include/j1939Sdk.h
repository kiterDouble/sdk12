/*************************************************
 Copyright © 2025 SiRun (WuXi). All rights reserved.
 File Name: j1939Sdk.h
 Author: 
 Created Time: 
 Description: J1939 Protocol SDK Header File
              SAE J1939-21 (Data Link Layer)
              SAE J1939-81 (Network Management)
 Others: 
 *************************************************/

#ifndef _J1939_SDK_H_
#define _J1939_SDK_H_

/****************************** include ***************************************/
#include <stdint.h>
#include "canHal.h"

/****************************** Macro Definitions ******************************/
/* J1939 Protocol Constants */
#define J1939_CAN_CHANNEL               TBOX_CAN_CHANNEL_4
#define J1939_BAUDRATE                  E_CAN_500K

/* Address Constants */
#define J1939_GLOBAL_ADDRESS            0xFF    /* Broadcast address */
#define J1939_NULL_ADDRESS              0xFE    /* Cannot claim address */
#define J1939_MAX_ADDRESS               0xFD    /* Max valid source address */

/* PGN Constants */
#define J1939_PGN_ADDRESS_CLAIMED       0x00EE00
#define J1939_PGN_ADDRESS_COMMAND       0x00FED8
#define J1939_PGN_TP_CONNECTION_MANAGE  0x00EC00
#define J1939_PGN_TP_DATA_TRANSFER      0x00EB00
#define J1939_PGN_REQUEST               0x00EA00
#define J1939_PGN_ACKNOWLEDGMENT        0x00E800

/* PDU Format Constants */
#define J1939_PF_PDU1_MAX               239     /* Max PDU1 format (destination specific) */
#define J1939_PF_PDU2_MIN               240     /* Min PDU2 format (broadcast) */

/* Priority Constants */
#define J1939_PRIORITY_HIGH             0
#define J1939_PRIORITY_DEFAULT          3
#define J1939_PRIORITY_LOW              6
#define J1939_PRIORITY_LOWEST           7

/* Data Length Constants */
#define J1939_MAX_DATA_LENGTH           1785    /* Max TP data length */
#define J1939_STD_DATA_LENGTH           8       /* Standard CAN frame length */

/* Configuration Limits */
#define J1939_MAX_PGN_NUM               32      /* Max registered PGNs */
#define J1939_MAX_TX_PGN_NUM            16      /* Max periodic TX PGNs */

/* Return Codes */
#define J1939_OK                        0
#define J1939_ERROR                     (-1)
#define J1939_ERROR_INVALID_PARAM       (-2)
#define J1939_ERROR_NO_RESOURCE         (-3)
#define J1939_ERROR_NOT_INITIALIZED     (-4)
#define J1939_ERROR_PGN_NOT_FOUND       (-5)
#define J1939_ERROR_BUFFER_FULL         (-6)

/****************************** Type Definitions ******************************/

/* J1939 Device Configuration */
typedef struct
{
    uint8_t  sourceAddress;           /* Source address (0-253) */
    uint64_t deviceName;              /* 64-bit device NAME */
    uint8_t  preferredAddress;        /* Preferred address */
    uint8_t  priority;                /* Default priority (0-7) */
} J1939DeviceConfig_t;

/* J1939 PGN Configuration */
typedef struct
{
    uint32_t pgn;                     /* Parameter Group Number */
    uint8_t  priority;                /* Priority (0-7) */
    uint16_t dataLength;              /* Data length (8 or variable) */
    uint16_t txPeriod;                /* TX period in ms (0=event triggered) */
    uint32_t lastTxTime;              /* Last TX timestamp */
} J1939PgnConfig_t;

/* J1939 Message Structure */
typedef struct
{
    uint32_t pgn;                     /* PGN */
    uint8_t  sourceAddr;              /* Source address */
    uint8_t  destAddr;                /* Destination address (0xFF=broadcast) */
    uint8_t  priority;                /* Priority */
    uint8_t  data[1785];              /* Data payload */
    uint16_t length;                  /* Actual data length */
} J1939Message_t;

/* J1939 NAME Structure (64-bit) */
typedef union
{
    uint64_t nameValue;
    struct
    {
        uint32_t identityNumber;      /* Bits 0-20 */
        uint16_t manufacturerCode;    /* Bits 21-31 */
        uint8_t  ecuInstance;         /* Bits 32-34 */
        uint8_t  functionInstance;    /* Bits 35-38 */
        uint8_t  vehicleSystem;       /* Bits 39-45 */
        uint8_t  vehicleSystemInstance; /* Bits 46-50 */
        uint8_t  industryGroup;       /* Bits 51-53 */
        uint8_t  arbitraryAddressCapable; /* Bit 54 */
        uint8_t  function;            /* Bits 55-61 */
        uint8_t  reserved;            /* Bits 62-63 */
    } fields;
} J1939Name_t;

/* Callback Function Types */
typedef void (*J1939RxCallback_t)(const J1939Message_t *msg);
typedef void (*J1939TxCompleteCallback_t)(uint32_t pgn, uint8_t status);

/* J1939 State */
typedef enum
{
    J1939_STATE_UNINITIALIZED = 0,
    J1939_STATE_INITIALIZED,
    J1939_STATE_RUNNING,
    J1939_STATE_ERROR
} J1939State_e;

/****************************** Function Declarations *************************/

/* Initialization and Configuration */
int16_t J1939Sdk_Init(const J1939DeviceConfig_t *config);
int16_t J1939Sdk_Deinit(void);
int16_t J1939Sdk_RegisterPgn(uint32_t pgn, J1939RxCallback_t callback);
int16_t J1939Sdk_UnregisterPgn(uint32_t pgn);
int16_t J1939Sdk_ConfigTxPgn(uint32_t pgn, uint8_t priority, uint16_t period);

/* Message Transmission */
int16_t J1939Sdk_Transmit(uint32_t pgn, uint8_t destAddr, 
                         const uint8_t *data, uint16_t length);
int16_t J1939Sdk_TransmitPri(uint32_t pgn, uint8_t destAddr, uint8_t priority,
                            const uint8_t *data, uint16_t length);

/* Message Reception */
int16_t J1939Sdk_Receive(J1939Message_t *msg, uint32_t timeout_ms);
void J1939Sdk_ProcessRxMessage(const CanHalMsg_t *canMsg);

/* Period Processing */
void J1939Sdk_CycleProcess(uint32_t cycleTime);

/* Utility Functions */
uint32_t J1939_MakeCanId(uint8_t priority, uint32_t pgn, uint8_t sa);
int16_t J1939_ParseCanId(uint32_t canId, uint8_t *priority, uint32_t *pgn, 
                        uint8_t *sa, uint8_t *da);
uint8_t J1939_IsBroadcastPgn(uint32_t pgn);
uint8_t J1939_IsDestinationSpecific(uint32_t pgn);

/* Address Management */
int16_t J1939_SetSourceAddress(uint8_t address);
uint8_t J1939_GetSourceAddress(void);
int16_t J1939_ClaimAddress(uint8_t address);

/* Status and Diagnostics */
J1939State_e J1939Sdk_GetState(void);
uint16_t J1939Sdk_GetRxCounter(void);
uint16_t J1939Sdk_GetTxCounter(void);
void J1939Sdk_GetStatistics(uint16_t *rxCount, uint16_t *txCount, uint16_t *errCount);

#endif /* _J1939_SDK_H_ */
