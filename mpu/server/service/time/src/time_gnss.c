#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "gnss_ipc_msg.h"
#include "queue_msg.h"
#include "sr_ipc.h"
#include "err_code.h"
#include "sr_log.h"


static uint32_t g_gnssTime[10] = {0};
static uint32_t g_gnssTimeIndex = 0;
static uint8_t g_gnssStatus = 0; //1 定位有效，2 定位无效
static pthread_mutex_t g_gnssLock = PTHREAD_MUTEX_INITIALIZER;
static uint8_t g_startGnssTime = 0; //1 启用gnss时间， 2关闭gnss时间
static uint8_t g_gnssTimeSyncStatus = 0; // 1 gnss时间已同步， 2gnss时间未同步



/**
 * @name StartGnssTimeSync
 * @brief  开启Gnss时间同步
 * @param 
 * @return 成功返回0，失败返回小于0 
 */
int32_t StartGnssTimeSync()
{
    pthread_mutex_lock(&g_gnssLock);
    g_startGnssTime = 1;
    pthread_mutex_unlock(&g_gnssLock);

    return RET_OK;
}


/**
 * @name StopGnssTimeSync
 * @brief  关闭Gnss时间同步
 * @param 
 * @return 成功返回0，失败返回小于0 
 */
int32_t StopGnssTimeSync()
{
    pthread_mutex_lock(&g_gnssLock);
    g_startGnssTime = 2;
    pthread_mutex_unlock(&g_gnssLock);
    return RET_OK;
}


/**
 * @name GetGnssTimeSyncStatus
 * @brief  获取GNSS时间同步的状态
 * @param syncStatus 存储时间同步状态的指针
 * @return 成功返回0，失败返回小于0 
 */
int32_t GetGnssTimeSyncStatus(uint8_t *syncStatus)
{
    if(NULL == syncStatus)
    {
        LOG_Error("%s argv syncStatus  is NULL", __FUNCTION__);
        return RET_ERR;
    }

    pthread_mutex_lock(&g_gnssLock);
    *syncStatus = g_gnssTimeSyncStatus;
    pthread_mutex_unlock(&g_gnssLock);
    return RET_OK;
}


/**
 * @name GnssDataCallback
 * @brief  获取GNSS数据的 IPC 回调函数
 * @param 
 * @return  
 */
void GnssDataCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    GnssLocation_t *pGnssData = NULL;
    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(dataLen != sizeof(GnssLocation_t))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_GNSS_LOCATIONINFO);
    
    pGnssData = (GnssLocation_t *)recvData;
    pthread_mutex_lock(&g_gnssLock);
    if(0x00 != (pGnssData->gpsState & 0x3F))
    {
       g_gnssStatus = 0x01;//有效定位
    }
    else
    {
        g_gnssStatus = 0x02;//无效定位
    }

    g_gnssTimeIndex++;
    if(10 == g_gnssTimeIndex)
    {
        g_gnssTimeIndex = 0;
    }
    g_gnssTime[g_gnssTimeIndex] = pGnssData->timestamp;
    
    pthread_mutex_unlock(&g_gnssLock);
}


/**
 * @name InitGnssIpcCallback
 * @brief  初始化GNSS IPC 回调函数
 * @param 
 * @return 成功返回0，失败返回小于0 
 */
int32_t InitGnssIpcCallback()
{
    SubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO, GnssDataCallback, NULL);
    return RET_OK;
}


/**
 * @name DestoryGnssIpcCallback
 * @brief  销毁GNSS IPC 回调函数
 * @param 
 * @return 成功返回0，失败返回小于0 
 */
int32_t DestoryGnssIpcCallback()
{
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO);
    return RET_OK;
}


/**
 * @name SetSystemTime
 * @brief  设置系统时间
 * @param 
 * @return　0： 连续， 其他：非连续
 */
static int32_t SetSystemTime(time_t newtime)
{
    struct timeval tv;

    memset(&tv, 0, sizeof(struct timeval));

    tv.tv_sec = newtime;

    LOG_Debug("set system time :%s ", ctime((time_t *) &tv.tv_sec));
    if (settimeofday(&tv, NULL) != 0)
    {
        LOG_Error("settimeofday error : %s", strerror(errno));
        return RET_ERR;
    }

    LOG_Debug("set system time :%s ", ctime((time_t *) &tv.tv_sec));
    return RET_OK;
}


/**
 * @name JudgeGnssTimeIsInvalid
 * @brief  判断GNSS时间是否10S连续
 * @param 
 * @return　0： 连续， 其他：非连续
 */
static int32_t JudgeGnssTimeIsInvalid()
{
    static time_t gnssTime = 0;
    int32_t count = 0;
    int32_t index = 0;
    time_t oldTime = 0;
    
    if(abs(gnssTime - g_gnssTime[g_gnssTimeIndex]) == 0)
    {
        LOG_Error("Gnss time not renew");
        return RET_ERR;
    }

    gnssTime = g_gnssTime[g_gnssTimeIndex];

    oldTime = g_gnssTime[g_gnssTimeIndex];
    LOG_Debug("g_gnssTimeIndex :%d", g_gnssTimeIndex);
    for(index = g_gnssTimeIndex + 1; index < 10 && count < 9; index++, count++)
    {
        LOG_Debug("index :%d", index);
        if(abs(g_gnssTime[index] - oldTime) > 9)
        {
            LOG_Error("Gnss time not continuous");
            return RET_ERR;
        }
        oldTime = g_gnssTime[index];
    }

    for(index = 0; index < 10 && count < 9; index++, count++)
    {
        LOG_Debug("index :%d", index);
        if(abs(g_gnssTime[index] - oldTime) > 9)
        {
            LOG_Error("Gnss time not continuous");
            return RET_ERR;
        }
        oldTime = g_gnssTime[index];
    }

    return RET_OK;
}


/**
 * @name GnssTimeManageThread
 * @brief  GNSS时间管理线程函数
 * @param 
 * @return
 */
void *GnssTimeManageThread(void *arg)
{
    time_t oldTime = 0;
    time_t newTime = 0;
    
    while(1)
    {
        pthread_mutex_lock(&g_gnssLock);
        if(1 == g_gnssStatus)
        {
            if(JudgeGnssTimeIsInvalid() < 0)
            {
                LOG_Error("Gnss Time not continuous");
                g_gnssTimeSyncStatus = 2;

            }
            else if(1 == g_startGnssTime)
            {
                time(&newTime);
                oldTime = g_gnssTime[g_gnssTimeIndex] + 28800;
                if(abs(newTime - oldTime) > 2)
                {
                    SetSystemTime(oldTime);
                }
                g_gnssTimeSyncStatus = 1;
                LOG_Debug("g_gnssTimeIndex :%d", g_gnssTimeIndex);
                LOG_Debug("systemTime :%ld, GnssTime: %ld", newTime, oldTime);
            }
        }
        pthread_mutex_unlock(&g_gnssLock);

        sleep(1);
    }
}