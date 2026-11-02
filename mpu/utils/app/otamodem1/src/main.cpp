#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "sr_ipc.h"
#include "queue_msg.h"
#include "http_thread.h"
#include "sr_sdk.h"
#include "https.h"
#include "sr_log.h"
#include "modem_function.h"
#include "system_interface.h"
#include "https_interface.h"
#include "thread_interface.h"
#include "time_interface.h"
#include "completion_timeout.h"
#include "http_queue.h"
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


int IsNetworkAvailable() 
{
    int ret = 0;
    while (1)
    {
        ret = system("ping -c 1 -W 5 auth.zelostech.com.cn ");
        if (ret == 0) 
        {
            return 0; // 网络可用

        } else {

            sleep(2); // 网络不可用
        }

    }
    return 0; 
}


int SirunInit()
{
    int ret = 0;
    LOG_Debug("Sirun Init Start");
    InitSRSdk();

    ret = StartQueueMsgRcvThread("jiushi");
    if(ret != 0)
    {
        LOG_Error("StartQueueMsgRcvThread Fail ");
        return -1;
    }

    InitGetTimeSyncStatusCallBack();

    CheckTimeSync();
    IsNetworkAvailable();
    InitCompletion(GetComp());
    InitCompletion(GetOtaComp());
    
    RpcStartMsgServer((RPCServerType_e)RPCSERVER_TYPE_OTAAPP, RpcOtaAppCallBack, 4, NULL);
    ret = HttpsInit();
    if(ret != 0)
    {
        LOG_Error("Https Init Fail ");
        return -1;
    }

    ret = ThreadInit();
    if(ret != 0)
    {
        LOG_Error("Thread Init Fail ");
        return -1;
    }

    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY, GetModemSrvInfo, NULL);
    if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY Fail ");
    }
   
    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW, HardLineSyncCallBack, NULL);
    if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW Fail ");
    }
    
    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, HandlePmSleepCmd, NULL);
    if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY Fail ");
    }
    
    ret = TokenTimerInit(TimerHandler);
    if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY Fail ");
        return -1;
    }

    return 0;
}


int main(int argc, char *argv[])
{
    
    int ret;
    pthread_t https_threadid;
    LOG_INIT("modem_a");
    if(access(EMMCDEVICENODE, F_OK) != 0)
    {
        LOG_Debug("主模组 代码 ...\n");
        BlockTheProcess();
        return 0;
    }

    LOG_Debug("主模组 代码开始运行 ...\n");
    ret = SirunInit();
    if(ret != 0)
    {
        LOG_Error("SirunInit Fail");
        return -1;
    }

    ret = CreateThread(&https_threadid,HTTPSThread, NULL);
    if (ret < 0)
    {
        LOG_Error("Create HTTPS Thread Fail \n");
    }

    while(1)
    {
        sleep(10);
    }
    CleanupCompletion(GetComp());
    CleanupCompletion(GetOtaComp());
    PthreadMutexDestroy();
    return 0;
}

