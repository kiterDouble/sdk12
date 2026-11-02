/*************************************************
 Copyright ? 2025 SiRun (WuXi). All rights reserved.
 File Name: osekNmSdk.h
 Author: 
 Created Time: 
 Description: Osek Network Management SDK Header File
 Others: Implements standard Osek NM protocol with logical ring
         This file provides unified OSEK NM interface compatible with
         the nmConfig.h protocol selection mechanism.
 *************************************************/

#ifndef _OSEK_NM_SDK_H_
#define _OSEK_NM_SDK_H_

/****************************** include ***************************************/
#include "Std_Types.h"
#include "canHal.h"

/****************************** Macro Definitions ******************************/
#define OSEK_NM_SDK_STATUS_OK           (0)
#define OSEK_NM_SDK_STATUS_ERR          (-1)
#define OSEK_NM_SDK_INVALID_HANDLE      (-1)

#define OSEK_NM_MAX_INSTANCE            (4U)
#define OSEK_NM_MSG_LENGTH              (8U)

/* NM Message Type */
#define OSEK_NM_MSG_TYPE_ALIVE          (0x00)
#define OSEK_NM_MSG_TYPE_RING           (0x01)
#define OSEK_NM_MSG_TYPE_LIMPHOME       (0x02)

/* Opcode Bit Masks */
#define OSEK_NM_OPCODE_BIT_MASK_ALIVE        (0x01U << 0U)
#define OSEK_NM_OPCODE_BIT_MASK_RING         (0x01U << 1U)
#define OSEK_NM_OPCODE_BIT_MASK_LIMPHOME     (0x01U << 2U)
#define OSEK_NM_OPCODE_BIT_MASK_SLEEP_IND    (0x01U << 4U)
#define OSEK_NM_OPCODE_BIT_MASK_SLEEP_ACK    (0x01U << 5U)

/* Sub-Network Wakeup Bit Masks */
#define OSEK_NM_SUB_NET_WAKEUP_PCAN_MASK       (1U << 0U)
#define OSEK_NM_SUB_NET_WAKEUP_ECAN_MASK       (1U << 1U)
#define OSEK_NM_SUB_NET_WAKEUP_BCAN_MASK       (1U << 2U)
#define OSEK_NM_SUB_NET_WAKEUP_ACAN_MASK       (1U << 3U)
#define OSEK_NM_SUB_NET_WAKEUP_ICAN_MASK       (1U << 4U)
#define OSEK_NM_SUB_NET_WAKEUP_TCAN_MASK       (1U << 5U)
#define OSEK_NM_SUB_NET_WAKEUP_LCAN_MASK       (1U << 6U)
#define OSEK_NM_SUB_NET_WAKEUP_ALL             (0x7FU)

/* NM Mode Names */
#define OSEK_NM_MODE_BUS_SLEEP         (0U)
#define OSEK_NM_MODE_AWAKE             (1U)

/* NM State Debug Values */
#define OSEK_NM_STATE_RESET            (0x02U)
#define OSEK_NM_STATE_LIMPHOME         (0x03U)
#define OSEK_NM_STATE_LIMPHOME_PRESLEEP (0x04U)
#define OSEK_NM_STATE_NORMAL           (0x05U)
#define OSEK_NM_STATE_NORMAL_PRESLEEP  (0x06U)

/* Legacy type definitions for compatibility */
/* E_OK: already defined in Std_Types.h - removed to avoid conflict */
/* StatusType: already defined in Std_Types.h as unsigned char - removed to avoid conflict */

typedef void (* NmOsekEventCallBackFun_t)(uint8_t flag);

typedef enum
{
  NMModeName_BusSleep,
  NMModeName_Awake,
}NMModeName;

typedef uint8_t NetIdType;
typedef uint8_t NetworkStatus;

/****************************** Type Definitions ******************************/

typedef void (*OsekNmEventCallBackFun_t)(uint8_t flag);

typedef enum
{
    E_OSEK_NM_STATE_BUS_SLEEP = 0,
    E_OSEK_NM_STATE_NM_ON,
    E_OSEK_NM_STATE_LIMPHOME,
    E_OSEK_NM_STATE_GO_TO_SLEEP,
} OsekNmState_e;

typedef enum
{
    E_OSEK_NM_SUBSTATE_NONE = 0,
    E_OSEK_NM_SUBSTATE_WAIT_RING,
    E_OSEK_NM_SUBSTATE_SEND_RING,
    E_OSEK_NM_SUBSTATE_WAIT_ACK,
} OsekNmSubState_e;

/* Extended state machine states (from net_manage_osek.c) */
typedef enum
{
    E_OSEK_NM_STATE_NM_OFF = 0,
    E_OSEK_NM_STATE_NM_ON_INIT,
    E_OSEK_NM_STATE_NM_SHUTDOWN,
    /* NMOn states */
    E_OSEK_NM_STATE_NM_INIT,
    E_OSEK_NM_STATE_NM_AWAKE,
    E_OSEK_NM_STATE_NM_NORMAL_PRESLEEP,
    E_OSEK_NM_STATE_NM_NORMAL_INIT_BUS_SLEEP,
    E_OSEK_NM_STATE_NM_TWBS_NORMAL,      /* Wait Twait BusSleep */
    E_OSEK_NM_STATE_NM_LIMPHOME_PRESLEEP,
    E_OSEK_NM_STATE_NM_LIMPHOME_INIT_BUS_SLEEP,
    E_OSEK_NM_STATE_NM_TWBS_LIMPHOME,    /* Wait Twait BusSleep LimpHome */
    E_OSEK_NM_STATE_NM_BUS_SLEEP,
    E_OSEK_NM_STATE_NM_BUS_SLEEP_WAKEUP,
    /* NMAwake states */
    E_OSEK_NM_STATE_NM_INIT_RESET,
    E_OSEK_NM_STATE_NM_RESET,
    E_OSEK_NM_STATE_NM_INIT_NORMAL,
    E_OSEK_NM_STATE_NM_NORMAL,
    E_OSEK_NM_STATE_NM_INIT_LIMPHOME,
    E_OSEK_NM_STATE_NM_LIMPHOME,
} OsekNmExtState_e;

typedef enum
{
    E_OSEK_NM_MSG_TYPE_UNKNOWN = 0,
    E_OSEK_NM_MSG_TYPE_LIMPHOME_MSG,
    E_OSEK_NM_MSG_TYPE_RING_MSG,
    E_OSEK_NM_MSG_TYPE_ALIVE_MSG,
} OsekNmMsgType_e;

typedef struct
{
    uint8_t NodeId;
    uint8_t canChannel;
    uint32_t nmBaseAddress;
    uint32_t nmNodeIdMax;             /* Maximum node ID for NM message filter, default: 255 */
    uint32_t nmRingCycleTime;         /* Ttype: Typical time interval between two ring messages */
    uint32_t nmTimeoutTime;           /* Tmax: Maximum time interval between two ring messages */
    uint32_t nmLimpHomeTime;          /* Terror: Time interval between two ring messages with NMLimpHome, default: 1000ms */
    uint32_t nmWaitBusSleepTime;      /* Twbs: Time the NM waits before transmission in NMBusSleep */
    uint32_t nmTxTime;                /* Ttx: Transmission retry timer, default: 10ms */
    uint32_t rxLimit;                 /* Reception counter limit, default: 5 */
    uint32_t txLimit;                 /* Transmission counter limit, default: 5 */
    uint32_t busOffQuickTime;
    uint32_t busOffSlowTime;
    uint8_t busOffQuickTimes;
    uint32_t busOffErrorEventLimitCount; /* BusOff error event limit count, default: 2 */
    OsekNmEventCallBackFun_t limpHomeEventCallBack;  /* LimpHome state change callback, optional */
    OsekNmEventCallBackFun_t canBusErrorCallBack;    /* CAN bus error callback */
} OsekNmParameter_t;

/* Legacy configuration structure for compatibility with net_manage_osek.h */
typedef struct
{
  uint32_t netManageBaseAddress;/*Relative identification of the node-specific NM messages */
  uint32_t netManageNodeIdMax;
  uint8_t netManageNodeId;  /*Relative identification of the node-specific NM messages */
  uint32_t netManageTtype;  /*Typical time interval between two ring messages */
  uint32_t netManageTmax;   /*Maximum time interval between two ring messages */
  uint32_t netManageTerror; /*Time interval between two ring messages with NMLimpHome identification */
  uint32_t netManageTwaitBusSleep;/*Time the NM waits before transmission in NMBusSleep//unit:ms */
  uint32_t netManageTtx;
  uint32_t rx_limit;
  uint32_t tx_limit;  
  uint8_t canChannel;
  uint32_t busOffQuickTime;
  uint32_t busOffSlowTime;
  uint32_t busOffQuickTimes;
  NmOsekEventCallBackFun_t limpHomeEventCallBack;
  NmOsekEventCallBackFun_t canBusErrorCallBack;
  uint32_t busOffErrorEventLimitCount;
}struc_netManageOsekConfigure;

typedef struct
{
    uint8_t busSleep:1;
    uint8_t NMactive:1;
    uint8_t sleep_ind:1;
    uint8_t sleep_ack:1;
} OsekNmNetworkStatus_t;

/****************************** Function Declarations *************************/

/* Configuration and Initialization */
int16_t OsekNmSdkConfig(const OsekNmParameter_t *pNmConfig, uint8_t NmNumber);
uint16_t NetManageOsekInit(const struc_netManageOsekConfigure* pNetManageConfigure,uint8_t netManageConfigureNumber);
void OsekNmSdkCycleProcess(void);
void NetManageOsekCycleProcess(void);

/* Network Control APIs */
int16_t OsekNmSdkNetworkRequest(uint8_t LocalWakeup);
int16_t OsekNmSdkNetworkRelease(int16_t nmChannelHandle);
int16_t OsekNmSdkEnableTx(uint8_t nmID);
int16_t OsekNmSdkDisableTx(uint8_t nmID);

/* Status Query APIs */
int16_t OsekNmSdkGetSleepStatus(uint8_t nmID, uint8_t *sleepState);
int16_t OsekNmSdkGetCanStatus(uint8_t nmID, uint8_t *ReceiveState);
uint8_t OsekNmSdkGetBusState(void);
int16_t OsekNmSdkGetNetManageReceiveStatus(uint8_t nmID, uint8_t *ReceiveState);
void OsekNmSdkGetNetManageReceiveStatusClear(uint8_t nmID);
void OsekNmSdkGetNetManageCanChannel(uint8_t nmID, uint8_t *channelFlag);

/* Wakeup Control APIs */
int16_t OsekNmSdkSetWakeupRequest(uint8_t wakeupRequestFlags);
int16_t OsekNmSdkClearWakeupRequest(void);

/* OSEK NM Standard APIs (compatible with net_manage_osek.h) */
StatusType OsekNmStartNM(NetIdType NetId);
StatusType OsekNmStopNM(NetIdType NetId);
StatusType OsekNmGotoMode(NetIdType NetId,NMModeName NewMode);
StatusType OsekNmTalkNM(NetIdType NetId);
StatusType OsekNmSilentNM(NetIdType NetId);
StatusType OsekNmGetStatusSleep(NetIdType NetId,uint8_t *sleepState);

/* All Channel APIs */
StatusType OsekNmAllStartNM(void);
StatusType OsekNmAllStopNM(void);
StatusType OsekNmAllGotoMode(NMModeName NewMode);
StatusType OsekNmAllTalkNM(void);
StatusType OsekNmAllSilentNM(void);
StatusType OsekNmAllGetStatusSleep(uint8_t *sleepState);
StatusType OsekLimpHomeSleepAck(uint8_t *sleepAck);

/* CAN Status APIs */
StatusType OsekNm_GetAllCanStatus(uint8_t *ReceiveState);
StatusType OsekNm_GetNetManageReceiveStatus(uint8_t *ReceiveState);
void OsekNm_NetManageReceiveStatusClear(void);
void OsekNm_NetManageCanChannel(uint8_t *channelFlag);

/* Communication Control APIs */
StatusType Nm_AllEnableCommunication(void);
StatusType Nm_AllDisableCommunication(void);

/* Debug Support */
void OsekNmSdkSetDebugPrint(uint32_t cycleTime_ms);
void NetManageOsekSetDebugPrint(uint32_t cycleTime_ms);

#endif /* _OSEK_NM_SDK_H_ */
