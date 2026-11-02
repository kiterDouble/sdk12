#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "time_ntp.h"
#include "err_code.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "pm_server_api.h"
#include "net_service_api.h"
#include "queue_msg.h"

#define  NTP_SERVICE_START_CMD      "chronyd -f /oemapp/config/chrony.conf"
#define  NTP_SERVICE_RESTART_CMD    "killall chronyd ; chronyd -f /oemapp/config/chrony.conf"
#define  GET_NTP_STATUS_CMD         "chronyc sources -v | grep \"\\^\\*\""
#define  RESTART_NTP_TIME_SEC       (120)

static pthread_mutex_t g_ntpLock = PTHREAD_MUTEX_INITIALIZER;
static uint8_t g_ntpStatus = 0;     // 1 ntp 有效，2 无效
static MPUPmStateChange_t g_TimePmState = {0};
static volatile uint8_t g_dialConnected = 0; // 新增：0=未拨号, 1=拨号成功
static uint8_t g_ntpStarted = 0;    // 新增：0=未启动, 1=已启动

void TimeGetPmPowerState(const uint8_t* data, const uint16_t len, void* userdata)
{
    if(sizeof(MPUPmStateChange_t) == len && NULL != data)
    {
        memcpy(&g_TimePmState, data, len);
    }
    else
    {
        LOG_Debug("%s len error!",__FUNCTION__);
    }
}

int32_t GetNtpSyncStatus(uint8_t *ntpStatus)
{
    if(NULL == ntpStatus)
    {
        return RET_ERR;
    }

    pthread_mutex_lock(&g_ntpLock);
    *ntpStatus = g_ntpStatus;
    pthread_mutex_unlock(&g_ntpLock);

    return RET_OK;
}

static int32_t SrGetSystemRetValue(const char *cmd)
{
	char cmdBuff[128] = {0};
	FILE *fileFp;
    int32_t cmdStatus;
	
	if(NULL == cmd)
	{
		LOG_Error("%s cmd is NULL", __FUNCTION__);
		return RET_ERR;
	}
    
    LOG_Debug("start exec cmd :%s", cmd);

	fileFp = popen(cmd, "r");
	if(NULL == fileFp)
	{
		LOG_Error("%s popen error :%s", __FUNCTION__, strerror(errno));
		return RET_ERR;
	}
	
	while(fgets(cmdBuff, 128, fileFp) != NULL)
	{
		LOG_Debug("get cmd ret: %s", cmdBuff);
	}
	
	cmdStatus = pclose(fileFp);
	if(-1 == cmdStatus)
	{
		LOG_Error("%s pclose error :%s", __FUNCTION__, strerror(errno));
		return RET_ERR;
	}
	
	if(WIFEXITED(cmdStatus))
	{  
        LOG_Debug("Command exited with status %d", WEXITSTATUS(cmdStatus));
		return WEXITSTATUS(cmdStatus);
    }
	else if(WIFSIGNALED(cmdStatus))
	{  
        LOG_Error("Command was terminated by signal %d", WTERMSIG(cmdStatus));
		return RET_ERR;
    }
	
	LOG_Error("Command was terminated not find signal" );
	return RET_ERR;
}

void TimeGetNetStatus(const uint8_t* data, const uint16_t len, void* userdata)
{
    if(NULL == data || len != sizeof(StrNetStatus_t))
    {
        LOG_Debug("%s argv is invalid", __FUNCTION__);
        return;
    }
    
    StrNetStatus_t *netStatus = (StrNetStatus_t *)data;
    uint8_t hasDialConnected = 0;
    uint8_t apnNum = (netStatus->apnNum > NETWORK_APN_NUM) ? NETWORK_APN_NUM : netStatus->apnNum;
    
    for(uint8_t i = 0; i < apnNum; i++)
    {
        if(netStatus->netStatusInfo[i].dialStatus == 1)
        {
            hasDialConnected = 1;
            break;
        }
    }
    
    __atomic_store_n(&g_dialConnected, hasDialConnected, __ATOMIC_SEQ_CST);
    LOG_Debug("%s dial connected: %d", __FUNCTION__, hasDialConnected);
}

void *ChronyManageThread(void *arg)
{
    int32_t retValue = 0;
    uint32_t ntpInvalidCount = 0; 
    int32_t restartRetValue = 0;
    
    SubscriptionQueueMsg(SYS_QUE_TYPE_NET_STATUS_NOTIFY, TimeGetNetStatus, NULL);
    LOG_Info("ChronyManageThread started, waiting for dial connection...");

    while(1)
    {
    
        if (PM_STATE_SLEEP == g_TimePmState.PmState)
        {
            sleep(1);
            continue;
        }
        
        uint8_t dialStatus = __atomic_load_n(&g_dialConnected, __ATOMIC_SEQ_CST);
        
        if(dialStatus == 0)
        {
            LOG_Debug("Dial not connected, waiting...");
            sleep(5);
            continue;
        }
        
        if(g_ntpStarted == 0)
        {
            SrGetSystemRetValue(NTP_SERVICE_START_CMD);
            g_ntpStarted = 1;
            LOG_Info("Dial connected, chronyd started");
        }
        
        retValue = SrGetSystemRetValue(GET_NTP_STATUS_CMD);
        if(0 == retValue)
        {
            pthread_mutex_lock(&g_ntpLock);
            g_ntpStatus = 1;
            pthread_mutex_unlock(&g_ntpLock);
            restartRetValue = -1;
            ntpInvalidCount = 0;
        }
        else
        {
            pthread_mutex_lock(&g_ntpLock);
            g_ntpStatus = 2;
            pthread_mutex_unlock(&g_ntpLock);
            ntpInvalidCount++;
            LOG_Debug("ntpInvalidCount :%d, restartRetValue:%d", ntpInvalidCount, restartRetValue);
        }

        if(ntpInvalidCount >= RESTART_NTP_TIME_SEC && 0 != restartRetValue)
        {
            restartRetValue = SrGetSystemRetValue(NTP_SERVICE_RESTART_CMD);
        }

        sleep(1);
    }
}
