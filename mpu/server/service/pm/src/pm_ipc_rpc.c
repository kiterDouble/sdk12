#include <stdlib.h>
#include <string.h>

#include "pm_ipc_rpc.h"
#include "pm_timer.h"
#include "pm_event.h"
#include "err_code.h"
#include "sr_log.h"
#include "queue_msg.h"
#include "sr_ipc.h"
#include "pm_modem.h"
#include "pm_mcu_control.h"
#include "pm_server_api.h"
#include "pm_device_control.h"

#define EVENTLEN 32

#define OPTION_OK                               (11)
#define OPTION_ERROR                            (12)
#define CREATE_SLEEP_EVENT                      (1)
#define DELETE_SLEEP_EVENT                      (2)
#define CREATE_PM_SLEEP_ALARM                   (1)
#define CREATE_PM_SLEEP_ALARM_EVERYDAY          (2)
#define DESTORY_PM_SLEEP_ALARM                  (3)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID1  (0x7001)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID2  (0x7002)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID3  (0x7003)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID4  (0x7004)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID5  (0x7005)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID6  (0x7006)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID7  (0x7007)

static uint8_t smsSubWakeupType = 0;

typedef struct
{
	uint8_t data;
} WakeupSrcIpc_t;

typedef struct
{
	uint8_t Cmd;
	uint8_t SleepAlarmEventId;
	uint8_t SleepAlarmHour;
	uint8_t SleepAlarmMinute;
	uint32_t SleepAlarmSecond;
	uint16_t WakeupOption;
} PmSleepAlarmIpc_t;

typedef struct
{
	uint8_t Cmd;
	char Event[EVENTLEN + 1];
} PmSleepEventIpc_t;

typedef struct
{
    uint8_t mode;
    char name[EVENTLEN + 1];
} PmWakeDevicesIpc_t;

uint8_t GetSmsSubWakeupType(void)
{
    return smsSubWakeupType;
}

static void PmRpcCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    int8_t RerurnValue = RET_ERR;
	
	if(subcmd == RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID1)
	{
        LOG_Debug("api request wakeup src.");
		WakeupSrcIpc_t WakeupSrcIpc;
		memset(&WakeupSrcIpc, 0x00, sizeof(WakeupSrcIpc));
		memcpy(&WakeupSrcIpc, reqData, reqLen);
		
		if(WakeupSrcIpc.data == 1)
		{
			memset(&WakeupSrcIpc, 0x00, sizeof(WakeupSrcIpc));
			WakeupSrcIpc.data = PmSchedGetWakeupSrc();
			memcpy(respData, (uint8_t *)&WakeupSrcIpc, sizeof(WakeupSrcIpc_t));
			*respLen = sizeof(WakeupSrcIpc_t);
		}
	}
	else if(subcmd == RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID2)
	{
        LOG_Debug("api request alarm src.");
		PmSleepAlarmIpc_t PmSleepAlarmIpc;
		memset(&PmSleepAlarmIpc, 0x00, sizeof(PmSleepAlarmIpc_t));
		memcpy(&PmSleepAlarmIpc, reqData, reqLen);
		
		if(PmSleepAlarmIpc.Cmd == CREATE_PM_SLEEP_ALARM)
		{
			RerurnValue = CreatePmSleepAlarm(PmSleepAlarmIpc.SleepAlarmEventId, PmSleepAlarmIpc.SleepAlarmSecond, PmSleepAlarmIpc.WakeupOption);
        }
		else if(PmSleepAlarmIpc.Cmd == CREATE_PM_SLEEP_ALARM_EVERYDAY)
		{
			RerurnValue = CreatePmSleepAlarmEveryDay(PmSleepAlarmIpc.SleepAlarmEventId, PmSleepAlarmIpc.SleepAlarmHour, PmSleepAlarmIpc.SleepAlarmMinute, PmSleepAlarmIpc.WakeupOption);
        }
		else if(PmSleepAlarmIpc.Cmd == DESTORY_PM_SLEEP_ALARM)
		{
			RerurnValue = DestroyPmSleepAlarm(PmSleepAlarmIpc.SleepAlarmEventId);
        }

        if(RET_OK == RerurnValue)
        {
            PmSleepAlarmIpc.Cmd = OPTION_OK;
        }
        else
        {
            PmSleepAlarmIpc.Cmd = OPTION_ERROR;
        }

        memcpy(respData, (uint8_t *)&PmSleepAlarmIpc, sizeof(PmSleepAlarmIpc_t));
        *respLen = sizeof(PmSleepAlarmIpc_t);        
	}
	else if(subcmd == RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID3)
	{
        LOG_Debug("api request sleep event src.");
		PmSleepEventIpc_t PmSleepEventIpc;
		memset(&PmSleepEventIpc, 0x00, sizeof(PmSleepEventIpc_t));
		memcpy(&PmSleepEventIpc, reqData, reqLen);
		
		if(PmSleepEventIpc.Cmd == CREATE_SLEEP_EVENT)
			RerurnValue = SetPmSleepEvent(PmSleepEventIpc.Event);
		else if(PmSleepEventIpc.Cmd == DELETE_SLEEP_EVENT)
			RerurnValue = CleanPmSleepEvent(PmSleepEventIpc.Event);

        if(RET_OK == RerurnValue)
        {
            PmSleepEventIpc.Cmd = OPTION_OK;
        }
        else
        {
            PmSleepEventIpc.Cmd = OPTION_ERROR;
        }
        
		memcpy(respData, (uint8_t *)&PmSleepEventIpc, sizeof(PmSleepEventIpc_t));
		*respLen = sizeof(PmSleepEventIpc_t);
	}
	else if(subcmd == RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID4)
	{
		smsSubWakeupType = *reqData;
        
        LOG_Debug("%s : api request sms wakeup. smsSubWakeupType : %d.",__FUNCTION__,smsSubWakeupType);
        
        RerurnValue = PmModemWakeup(PM_WAKEUP_SOURCE_SMS,CPU_INVALID_EVENT_ID);
                
		memcpy(respData, (uint8_t *)&RerurnValue, sizeof(int8_t));
		*respLen = sizeof(int8_t);
    }
    else if(subcmd == RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID5)
    {
        LOG_Debug("api request wake devices.");
        
		PmWakeDevicesIpc_t PmWakeDevicesIpc;
		memset(&PmWakeDevicesIpc, 0x00, sizeof(PmWakeDevicesIpc_t));
		memcpy(&PmWakeDevicesIpc, reqData, reqLen);
		
        RerurnValue = PmWakeupDevices(PmWakeDevicesIpc.name,PmWakeDevicesIpc.mode);
		memcpy(respData, (uint8_t *)&RerurnValue, sizeof(int8_t));
		*respLen = sizeof(int8_t);
    }
    else if(subcmd == RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID6)
    {
        LOG_Debug("api request sync data wake source and wakeup system.");
        
		uint8_t WakeupSrc = *reqData;
        
        RerurnValue = PmModemWakeup(WakeupSrc,CPU_INVALID_EVENT_ID);
		memcpy(respData, (uint8_t *)&RerurnValue, sizeof(int8_t));
		*respLen = sizeof(int8_t);
    }
	else if(subcmd == RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID7)
	{
        LOG_Debug("api request system sleep.");
        
        PmSchedSetPmState(PM_STATE_SLEEP);
                
		*respData = RET_OK;
		*respLen = sizeof(int8_t);
    }
	
}

void PmIpcSendPmState(uint8_t WorkMode, uint8_t PmState, uint8_t WakeupSrc, uint8_t SleepAlarmEventId)
{
	MPUPmStateChange_t MPUPmStateChange;
	memset(&MPUPmStateChange, 0x00, sizeof(MPUPmStateChange));
	
	MPUPmStateChange.WorkMode = WorkMode;
	MPUPmStateChange.PmState = PmState;
	MPUPmStateChange.WakeupSrc = WakeupSrc;
	MPUPmStateChange.SleepAlarmEventId = SleepAlarmEventId;
	
	SendMsgToAllQueue(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, (void *)&MPUPmStateChange, sizeof(MPUPmStateChange_t));
}

void PmIpcInit(void)
{
	RpcStartMsgServer(RPCSERVER_TYPE_PMSERVER, PmRpcCallBack,4, NULL);
	
	StartQueueMsgRcvThread("PmThread");
	SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_POWER_SLEEP_REQ_RES, PmMcuReqMpuSleepCallBack,NULL);
	SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_POWER_WAKEUPSOURCE_REQ_RES, PmMcuReqSyncWakeupSourceCallBack,NULL);
	SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_POWER_STATUS, PmMcuSyncPowerMode,NULL);
	SubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_CALL_NOTIFY, PmModemCallCallBack,NULL);
	SubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SMS_NOTIFY, PmModemSmsCallBack,NULL);
	SubscriptionQueueMsg(SYS_QUE_TYPE_ECALL_INCOMING_NOTIFY, PmModemEcallIncomingCallBack,NULL);
}

void PmIpcExit(void)
{
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SMS_NOTIFY);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_CALL_NOTIFY);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_POWER_STATUS);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_POWER_WAKEUPSOURCE_REQ_RES);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_POWER_SLEEP_REQ_RES);
	return;
}



