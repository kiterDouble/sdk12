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

typedef struct
{
    uint32_t sigType;
    int8_t rssi;
    int16_t ecio;
    int8_t rscp;
}ModemSrvGsmSigInfo_t;

typedef struct
{
    uint32_t sigType;
    int8_t rssi;
    int8_t rsrq;
    int16_t rsrp;
    int16_t sinr;
}ModemSrvLteSigInfo_t;

typedef union
{
    ModemSrvLteSigInfo_t lteSiginfo;
    ModemSrvGsmSigInfo_t gsmSiginfo;
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
    int timeSec1;       // 第一轮搜网间隔时间，范围1-225秒
    int timeSec2;       // 第二轮搜网间隔时间，范围1-225秒
    int timeSec3;       // 第三轮搜网间隔时间，范围1-225秒
}ModemSrvQoosCfg_t;

typedef struct
{
    float cpuTemperature;   // CPU温度，单位：°C
    float cpuOccupancy;     // CPU占用率，单位：%
}ModemSrvCpuInfo_t;

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
    E_MODEM_SRV_RPC_SUB_CMD_GET_CPU_INFO,
    E_MODEM_SRV_RPC_SUB_CMD_MAX
}ModemSrvRpcSubCmd_e;

typedef enum
{
    E_MODEM_SRV_ANT_STATUS_CIRCUIT_NORMAL = 0x0,    //天线正常
    E_MODEM_SRV_ANT_STATUS_CIRCUIT_SHORT, //天线短路
    E_MODEM_SRV_ANT_STATUS_CIRCUIT_OPEN //天线开路
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