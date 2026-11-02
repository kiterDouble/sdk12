#include<iostream>
#include "sr_ipc.h"
#include "queue_msg.h"
#include "mqtt.h"
#include "mqtt_thread.h"
#include "sr_sdk.h"
#include "modem_function.h"
#include "system_interface.h"
#include <unistd.h>
#include "mqtt_thread.h"
#include "thread_interface.h"
#include "mqtt_data_handler.h"
#include "mcu_interface.h"
#include "sr_log.h"
#include "modem_ipc_msg.h"
#include "common_functions.h"
#include "completion_timeout.h"
#include "http.h"
#include "cycle_evet_handle.h"
#include "pki_sdk.h"
#include "time_status_api.h"
#include "time_status.h"


int CheckTimeSync()
{
    int ret = 0;
    MpuTimeSync_t timeSync;
    LOG_Debug("CheckTimeSync start\n");
    
    while (1)
    {
        ret = GetTimeSyncStatus(&timeSync);
        if(ret != 0)
        {
            LOG_Debug("GetTimeSyncStatus fail\n");
            continue;
        }
        LOG_Debug("CheckTimeSync start ..%d",timeSync.timeSyncState);
        if(timeSync.timeSyncState == TIME_SYNC_OK)
        {
            LOG_Debug("Time synchronization successful ");
            DestoryGetTimeSyncStatusCallBack();
            return 0;
        }
        sleep(2);
    }
    
    return 0;
}


int SirunInit()
{
    int ret = 0;
    InitSRSdk();
    StartQueueMsgRcvThread("remote monitor");
    InitCompletion(GetComp());
    //InitCompletion(GetOtaComp());
    HttpInit();
    InitGetTimeSyncStatusCallBack();
    AddCarotaConfigModeParam();
    ret = SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_DOTA_RECEIVE, MqttDomainSwitchRevCallback, NULL);
    if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW Fail ");
    }

    ret = SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_SYNC_CAN_FRAME_SYNC, CANIdCallback, NULL);
    if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW Fail ");
    }

    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, PmStatusCallback, NULL);
    if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY Fail ");
    }

    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW, HardLineSyncCallBack, NULL);
    if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW Fail ");
    }

    ret = SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CTROL_GATEWAY_REQ, get_cmd_num, NULL);
     if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_MCU_VEHICLE_CTROL_GATEWAY_REQ Fail ");
    }

    InitSignalIpcCallback();
    CheckTimeSync();
    SiRunPKIEngineInit();
    return 0;
}

// 主程序入口
int main(int argc, char *argv[])
{
    int ret;
    pthread_t mqtt_threadid;
   
    LOG_INIT("remote_monitor");
  
    SirunInit();

    ret = CreateThread(&mqtt_threadid,MQTTThread, NULL);
    if (ret < 0)
    {
        LOG_Error("Create MQTT Thread Fail \n");
    }

   
    while(1)
	{
		(void)sleep(10);
	}
    pthread_join(mqtt_threadid, NULL);
 
    return 0;
}
