#ifndef __MODEM_IPC_MSG_H__
#define __MODEM_IPC_MSG_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t simCardStatus;
    uint32_t simAppStatus;
    uint32_t dataRegStatus;
    uint8_t csq;
    uint8_t antennaStatus;
    uint8_t secondaryAntennaStatus;
}ModemSrvNotify_t;

typedef enum MODEM_NW_SIGNAL_STRENGTH_LEVEL_ENUM
{
     NW_SIGNAL_STRENGTH_LEVEL_MIN = -1,
     NW_SIGNAL_STRENGTH_LEVEL_NONE = 0,
     NW_SIGNAL_STRENGTH_LEVEL_POOR = 1,
     NW_SIGNAL_STRENGTH_LEVEL_MODERATE = 2,
     NW_SIGNAL_STRENGTH_LEVEL_GOOD = 3,
     NW_SIGNAL_STRENGTH_LEVEL_GREAT = 4,
     NW_SIGNAL_STRENGTH_LEVEL_MAX
}MODEM_NW_SIGNAL_STRENGTH_LEVEL_E;

typedef struct
{
    uint32_t sigType; //2:5G,1:4G,0:2.3G
    int32_t rssi;
    int32_t rsrp;
    int32_t rsrq;
    int32_t sinr;
    int32_t ecio;
    int32_t rscp;
    MODEM_NW_SIGNAL_STRENGTH_LEVEL_E level;
}ModemSrvLteSigInfo_t; //兼容5G，4G，3G信号数据

typedef union
{
    ModemSrvLteSigInfo_t lteSiginfo;
}ModemSrvExtSigInfo_t;

typedef struct
{
    int32_t msgLen;
    uint8_t msgContent[1440];
    uint8_t sourceAddr[252];
}ModemSrvSms_t;

typedef struct
{
    int32_t status;
    int32_t callId;
    uint8_t phoneNum[81];
}ModemSrvCallStatus_t;

typedef struct
{
    uint16_t preTime;
    uint16_t sleepTime;
    uint16_t acqTime;
    uint8_t cirCnt;
    uint8_t sleepStep;
    uint8_t acqStep;
    uint8_t adjCnt;
    uint8_t idleInterval;
}ModemSrvQoosCfg_t;

typedef struct
{
    int32_t result;
    uint32_t dataLen;
    uint8_t data[0];
}ModemSrvRpcRes_t;

typedef enum
{
    E_MODEM_SRV_JOB_TYPE_SIM = 0x0,
    E_MODEM_SRV_JOB_TYPE_NW,
    E_MODEM_SRV_JOB_TYPE_DMS,
    E_MODEM_SRV_JOB_TYPE_CALL,
    E_MODEM_SRV_JOB_TYPE_SMS,
    E_MODEM_SRV_JOB_TYPE_ANT,
    E_MODEM_SRV_JOB_TYPE_MAX    
}ModemSrvJobType_e;

typedef enum
{
    E_MODEM_SRV_RPC_SUB_CMD_GET_IMSI = 0x0,
    E_MODEM_SRV_RPC_SUB_CMD_GET_ICCID,
    E_MODEM_SRV_RPC_SUB_CMD_GET_IMEI,
    E_MODEM_SRV_RPC_SUB_CMD_GET_AIRPLANE_MODE,
    E_MODEM_SRV_RPC_SUB_CMD_SET_AIRPLANE_MODE,
    E_MODEM_SRV_RPC_SUB_CMD_GET_QOOS_ENABLE,
    E_MODEM_SRV_RPC_SUB_CMD_SET_QOOS_ENABLE,
    E_MODEM_SRV_RPC_SUB_CMD_GET_QOOS_CFG,
    E_MODEM_SRV_RPC_SUB_CMD_SET_QOOS_CFG,
    E_MODEM_SRV_RPC_SUB_CMD_GET_EXT_SIGNAL_INFO,
    E_MODEM_SRV_RPC_SUB_CMD_GET_OPERATOR,
    E_MODEM_SRV_RPC_SUB_CMD_SEND_SMS_MESG,
    E_MODEM_SRV_RPC_SUB_CMD_START_CALL,
    E_MODEM_SRV_RPC_SUB_CMD_ACCEPT_CALL,
    E_MODEM_SRV_RPC_SUB_CMD_END_CALL,
    E_MODEM_SRV_RPC_SUB_CMD_DEINIT_JOB,
    E_MODEM_SRV_RPC_SUB_CMD_GET_MSISDN,
    E_MODEM_SRV_RPC_SUB_CMD_MAX
}ModemSrvRpcSubCmd_e;

typedef enum
{
    E_MODEM_SRV_ANT_STATUS_CIRCUIT_NORMAL = 0x0,
    E_MODEM_SRV_ANT_STATUS_CIRCUIT_OPEN,
    E_MODEM_SRV_ANT_STATUS_CIRCUIT_SHORT
}ModemSrvAntStatus_e;

typedef enum
{
    E_MODEM_SRV_ANT_INTERNAL = 0x0,
    E_MODEM_SRV_ANT_EXTERNEL
}ModemSrvAntType_e;

typedef enum
{
    E_MODEM_SRV_CALL_STATUS_ACTIVE = 0x0,
    E_MODEM_SRV_CALL_STATUS_HOLDING,
    E_MODEM_SRV_CALL_STATUS_DIALING,
    E_MODEM_SRV_CALL_STATUS_ALERTING,
    E_MODEM_SRV_CALL_STATUS_INCOMING,
    E_MODEM_SRV_CALL_STATUS_WAITING,
    E_MODEM_SRV_CALL_STATUS_OFFERING,
    E_MODEM_SRV_CALL_STATUS_DISCONNECTING,
    E_MODEM_SRV_CALL_STATUS_DISCONNECTED
}ModemSrvCallStatus_e;

typedef enum
{
    E_MODEM_SRV_DMS_AIRPLANE_OFF = 0x0,
    E_MODEM_SRV_DMS_AIRPLANE_ON,
    E_MODEM_SRV_DMS_AIRPLANE_UNKOWN
}ModemSrvDmsMode_e;

typedef enum
{
    E_MODEM_SRV_DMS_QOOS_ENABLE_STATE_OFF = 0x0,
    E_MODEM_SRV_DMS_QOOS_ENABLE_STATE_ON_NO_SERVICE,
    E_MODEM_SRV_DMS_QOOS_ENABLE_STATE_ON
}ModemSrvDmsQoosEnableStatus_e;

typedef enum
{
    E_MODEM_SRV_NW_REG_STATUS_REGISTERED = 0x0,
    E_MODEM_SRV_NW_REG_STATUS_UNREGISTERED
}ModemSrvNwRegStatus_e;


typedef enum
{
    E_MODEM_SRV_SIM_CARD_STATUS_PRESENT = 0x0,
    E_MODEM_SRV_SIM_CARD_STATUS_ABSENT,
    E_MODEM_SRV_SIM_CARD_STATUS_ERROR,
    E_MODEM_SRV_SIM_CARD_STATUS_BLOCKED,
}ModemSrvSimCardStatus_e;

typedef enum
{
    E_MODEM_SRV_SIM_APP_STATUS_READY = 0x0,
    E_MODEM_SRV_SIM_APP_STATUS_ERROR
}ModemSrvSimAppStatus_e;
#ifdef __cplusplus
}
#endif
#endif
