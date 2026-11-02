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



#include "time_status_api.h"
#include "time_status.h"


/**
 * @brief 检查是否为主模组
 * @return 1: 主模组, 0: 副模组
 */
static inline int IsMasterModule(void)
{
    return (0 == access("/dev/mmcblk0", F_OK));
}

// 模块名称统一为ota_configuration
#define MODULE_NAME "ota_configuration"

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
    StartQueueMsgRcvThread(MODULE_NAME);
    InitCompletion(GetComp());
    AddCarotaConfigModeParam();
    HttpInit();
    InitGetTimeSyncStatusCallBack();

    ret = SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_FOTA_RECEIVE, MqttDomainSwitchRevCallback, NULL);
    if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW Fail ");
        printf("SubscriptionQueueMsg SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW Fail ");
    }

    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, HandlePmSleepCmd, NULL);
    if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY Fail ");
        printf("SubscriptionQueueMsg SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY Fail ");
    }
    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW, HardLineSyncCallBack, NULL);
     if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW Fail ");
        printf("SubscriptionQueueMsg SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW Fail ");
    }
    ret = SubscriptionQueueMsg((SYSQueMsgType_e)0x00311101, get_cmd_num, NULL);
     if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW Fail ");
        printf("SubscriptionQueueMsg SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW Fail ");
    }
    CheckTimeSync();
    return 0;
}

// 主程序入口
int main(int argc, char *argv[])
{
    int ret;
    pthread_t mqtt_threadid;

    LOG_INIT(MODULE_NAME);
    LOG_Debug("Running as %s (%s)", MODULE_NAME, IsMasterModule() ? "Master" : "Slave");
    
    SirunInit();

    ret = CreateThread(&mqtt_threadid,MQTTThread, NULL);
    if (ret < 0)
    {
        LOG_Error("Create MQTT Thread Fail \n");
    }

    // 副模组启动两分钟之后，再向主模组同步域参数(226/227)并写入本地数据库；主模组跳过
    if (!IsMasterModule())
    {
        sleep(120);
        LOG_Debug("Slave start 120s, begin sync domain params from master");
        SyncDomainParamsFromMaster();
    }

    while(1)
	{
		(void)sleep(10);
	}
    pthread_join(mqtt_threadid, NULL);
 
    return 0;
}
