#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "mcu_api.h"
#include "time_gnss.h"
#include "time_ntp.h"
#include "sr_log.h"
#include "err_code.h"
#include "sr_ipc.h"
#include "time_status.h"


#define  TIME_SYNC_INVALID_TIME     (7200)

typedef void *(threadFun)(void *);



/**
 * @name BroadcastTimeSyncStatus
 * @brief  广播时间同步状态及时间源
 * @param timeSync 时间同步结构体
 * @return 0 成功
 */
int32_t BroadcastTimeSyncStatus(MpuTimeSync_t timeSync)
{
    uint8_t serviceData[2] = {0};
    
    serviceData[0] = timeSync.timeSyncState;
    serviceData[1] = timeSync.timeSyncSrc;
    SendMsgToAllQueue(SYS_QUE_TYPE_TIME_STATUS_NOTIFY, serviceData, 2);
    return RET_OK;
}


/**
 * @name SyncTimeStatusToMcu
 * @brief  同步时间同步状态及时间源至MCU
 * @param timeSync 时间同步结构体
 * @return 0 成功 其他：失败
 */
int32_t SyncTimeStatusToMcu(MpuTimeSync_t timeSync)
{
    uint8_t serviceData[8] = {0};
    time_t  systemTime = 0;
    McuMesg_t msg;

    
    time(&systemTime);

    serviceData[0] = (systemTime >> 24) & 0xFF;
    serviceData[1] = (systemTime >> 16) & 0xFF;
    serviceData[2] = (systemTime >> 8) & 0xFF;
    serviceData[3] = systemTime & 0xFF;

    serviceData[4] = 13;


    if(E_TIME_SYNC_SRC_GNSS == timeSync.timeSyncSrc)
    {
        serviceData[5] = 0x20;
    }
    else if(E_TIME_SYNC_SRC_NTP == timeSync.timeSyncSrc)
    {
        serviceData[5] = 0x10;
    }
    else
    {
        serviceData[5] = 0x00;
    }

    if(TIME_SYNC_OK == timeSync.timeSyncState)
    {
        serviceData[5] |= 0x01;
    }
    else
    {
        serviceData[5] = 0x00;
    }

    msg.type = SYS_QUE_TYPE_MCU_SYNC_TIME;
    msg.payloadLen = 6;
    msg.payload = (uint8_t *)serviceData;

    return McuSrvSendMesg(&msg);
}

/**
 * @name RtkCreateThread
 * @brief  创建线程函数
 * @param 
 *          threadId  存储线程ID
 *          fun       线程执行函数
 *          threadArg 传递给线程的指令
 * @return 0 成功 其他：失败
 */
static int32_t RtkCreateThread(pthread_t *threadId, threadFun fun, void *threadArg)
{
	pthread_attr_t attr;
	int32_t retValue = -1;

	if(NULL == threadId )
	{
		LOG_Error("%s argv threadId  is NULL", __FUNCTION__);
		return RET_ERR;
	}

	pthread_attr_init(&attr);

	//创建线程
	retValue = pthread_create(threadId, &attr, fun,(void *)threadArg);
	if(retValue < 0)
	{
		LOG_Error("pthread_create %s",strerror(errno));
		return RET_ERR;
	}

	LOG_Debug("rtk thread success");
	return RET_OK;
}


int32_t main()
{
    pthread_t ntpManageThread;
	pthread_t gnssManageThread;
    uint8_t ntpStatus = 0;
    uint8_t gnssSyncStatus = 0;
    uint8_t gnssStartFlag = 1;
    uint8_t timeSyncFlag = 0;
    MpuTimeSync_t timeSync;
    time_t oldTime = 0;
    time_t newTime = 0;

	LOG_INIT("timeServer");

	StartQueueMsgRcvThread("timeServer_signal_handle");

    SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY,TimeGetPmPowerState,NULL);
	InitGnssIpcCallback();

	RtkCreateThread(&ntpManageThread, ChronyManageThread, NULL);
	RtkCreateThread(&gnssManageThread, GnssTimeManageThread, NULL);

    timeSync.timeSyncSrc = E_TIME_SYNC_SRC_NONE;
    timeSync.timeSyncState = TIME_SYNC_NOT_OK;
    LOG_Debug("time status : %d, %d", timeSync.timeSyncState, timeSync.timeSyncSrc);

	while(1)
	{
        GetNtpSyncStatus(&ntpStatus);
        if(1 == ntpStatus)
        {
            timeSync.timeSyncSrc = E_TIME_SYNC_SRC_NTP;
            timeSync.timeSyncState = TIME_SYNC_OK;
            timeSyncFlag = 1;
            if(1 != gnssStartFlag)
            {
                gnssStartFlag = 1;
                StopGnssTimeSync();
            }
        }
        else
        {
            if(1 == gnssStartFlag)
            {
                StartGnssTimeSync();
                gnssStartFlag = 2;
            }
            GetGnssTimeSyncStatus(&gnssSyncStatus);
            if(1 == gnssSyncStatus)
            {
                timeSync.timeSyncSrc = E_TIME_SYNC_SRC_GNSS;
                timeSync.timeSyncState = TIME_SYNC_OK;
                timeSyncFlag = 1;
            }
            else
            {
                time(&newTime);
                if(1 == timeSyncFlag)
                {
                    oldTime = newTime;
                    timeSyncFlag = 0;
                }

                if(abs(newTime - oldTime) >= TIME_SYNC_INVALID_TIME && 0 == timeSyncFlag)
                {
                    timeSync.timeSyncSrc = E_TIME_SYNC_SRC_NONE;
                    // timeSync.timeSyncState = TIME_SYNC_NOT_OK;
                }
            }
        }

        SyncTimeStatusToMcu(timeSync);
        BroadcastTimeSyncStatus(timeSync);
        LOG_Debug("time status : %d, %d", timeSync.timeSyncState, timeSync.timeSyncSrc);
		sleep(1);
	}
	DestoryGnssIpcCallback();

	pthread_join(ntpManageThread, NULL);
	pthread_join(gnssManageThread, NULL);


    return 0;
}