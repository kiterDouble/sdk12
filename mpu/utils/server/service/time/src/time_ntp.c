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

#define  NTP_SERVICE_START_CMD      "chronyd -f /oemapp/config/chrony.conf"
#define  NTP_SERVICE_RESTART_CMD    "killall chronyd ; chronyd -f /oemapp/config/chrony.conf"
#define  GET_NTP_STATUS_CMD         "chronyc sources -v | grep \"\\^\\*\""
#define  RESTART_NTP_TIME_SEC       (120)

static pthread_mutex_t g_ntpLock = PTHREAD_MUTEX_INITIALIZER;
static uint8_t g_ntpStatus = 0;     // 1 ntp 有效，2 无效
static MPUPmStateChange_t g_TimePmState = {0};

/**
 * @name TimeGetPmPowerState
 * @brief 从给定数据中获取PM电源状态
 * @param data 指向包含PM电源状态数据的缓冲区的指针。
 * @param len 数据缓冲区的长度。
 * @param userdata 用户自定义数据的指针，本函数中未使用。
 * @return 无返回值 
 */
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
/**
 * @name GetNtpSyncStatus
 * @brief  获取NTP时间同步状态
 * @param ntpStatus  用于存储NTP状态的指针
 * @return 成功返回0，失败返回小于0 
 */
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

/**
 * @name SrGetSystemRetValue
 * @brief  执行shell 命令，并获取返回值
 * @param 
 * @return -1 执行命令失败，其他值为命令的返回值
 */
static int32_t SrGetSystemRetValue(const char *cmd)
{
	char cmdBuff[128] = {0};
	FILE *fileFp;				// 文件指针  
    int32_t cmdStatus;			// 用于存储命令的退出状态
	
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



/**
 * @name ChronyManageThread
 * @brief  NTP时间同步管理线程函数
 * @param 
 * @return 
 */
void *ChronyManageThread(void *arg)
{
    int32_t retValue = 0;
    uint32_t ntpInvalidCount = 0; 
    int32_t restartRetValue = 0;
    
    //获取网络状态，启动ntp服务
    SrGetSystemRetValue(NTP_SERVICE_START_CMD);

    while(1)
    {
    
        if (PM_STATE_SLEEP == g_TimePmState.PmState)
        {
            //LOG_Error("%s modem is sleep.", __FUNCTION__);
            sleep(1);
            continue;
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