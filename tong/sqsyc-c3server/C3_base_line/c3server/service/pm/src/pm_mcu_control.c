#ifndef _GNU_SOURCE
#define _GNU_SOURCE  /* CLOCK_BOOTTIME */
#endif

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <glib.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include "err_code.h"
#include "sr_log.h"
#include "mcu_api.h"
#include "pm_mcu_control.h"
#include "pm_server_api.h"
#include "pm_device_control.h"
#include "pm_event.h"
#include "pm_sched.h"
#include "pm_ipc_rpc.h"
#include "pm_modem.h"
#include "pm_modem_reboot.h"
#include "pm_timer.h"
#include "hal_modem_pm.h"

#define WAKEUP_MCU_TIMEOUT             10u
#define WAKEUP_MCU_MAX_COUNT           6u
#define MCU_HEARTBEAT_TIMEOUT_SEC      60u   /* 连续 60 秒没收到心跳 */
#define MCU_HEARTBEAT_INTERVAL_SEC     1u    /* MCU 1 秒发一次心跳 */
#define MCU_HEARTBEAT_ALIVE_TIMEOUT_SEC 5u   /* 5 秒没收到心跳认为 MCU 不在线 */

/*wakeup mcu timer*/
static GTimer *g_wakeUpMcuTimer = NULL;
static gint g_wakeUpMcuCount = 0;

static sem_t g_mcuHeartbeatSem;
static gboolean g_mcuHeartbeatSemInit = FALSE;
static gboolean g_mcuHeartbeatThreadRunning = TRUE;
static gboolean g_mcuHeartbeatAlive = FALSE;

G_LOCK_DEFINE_STATIC(g_wakeUpMcuLock);

void PmSchedWakeUpMcu(void)
{
    uint32_t mcuWakePin = 0;
    
    LOG_Debug("start WakeUp Mcu");

    mcuWakePin = GetMcuWakePin();
    //唤醒MCU
    
    char gpioCmdBuffer[100] = {0};
    
    memset(gpioCmdBuffer, 0, sizeof(gpioCmdBuffer));
    snprintf(gpioCmdBuffer, sizeof(gpioCmdBuffer), "echo %u > /sys/class/gpio/export", mcuWakePin);
    int status = system(gpioCmdBuffer);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.", __FUNCTION__, gpioCmdBuffer);
    } else {
        LOG_Debug("%s : exec cmd[ %s ] failed.", __FUNCTION__, gpioCmdBuffer);
    }
    
    memset(gpioCmdBuffer, 0, sizeof(gpioCmdBuffer));
    snprintf(gpioCmdBuffer, sizeof(gpioCmdBuffer), "echo out > /sys/class/gpio/gpio%u/direction", mcuWakePin);
    status = system(gpioCmdBuffer);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.", __FUNCTION__, gpioCmdBuffer);
    } else {
        LOG_Debug("%s : exec cmd[ %s ] failed.", __FUNCTION__, gpioCmdBuffer);
    }

    memset(gpioCmdBuffer, 0, sizeof(gpioCmdBuffer));
    snprintf(gpioCmdBuffer, sizeof(gpioCmdBuffer), "echo 0 > /sys/class/gpio/gpio%u/value", mcuWakePin);
    status = system(gpioCmdBuffer);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.", __FUNCTION__, gpioCmdBuffer);
    } else {
        LOG_Debug("%s : exec cmd[ %s ] failed.", __FUNCTION__, gpioCmdBuffer);
    }

    usleep(500000);
    
    memset(gpioCmdBuffer, 0, sizeof(gpioCmdBuffer));
    snprintf(gpioCmdBuffer, sizeof(gpioCmdBuffer), "echo 1 > /sys/class/gpio/gpio%u/value", mcuWakePin);
    status = system(gpioCmdBuffer);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.", __FUNCTION__, gpioCmdBuffer);
    } else {
        LOG_Debug("%s : exec cmd[ %s ] failed.", __FUNCTION__, gpioCmdBuffer);
    }
    
    G_LOCK(g_wakeUpMcuLock);
    g_wakeUpMcuCount++;
    if (g_wakeUpMcuTimer == NULL)
    {
        g_wakeUpMcuTimer = g_timer_new();
    }
    else
    {
        g_timer_start(g_wakeUpMcuTimer);
    }
    G_UNLOCK(g_wakeUpMcuLock);
    
    return;
}

static void CleanWakeUpMcuTimer()
{
    G_LOCK(g_wakeUpMcuLock);
    if (NULL != g_wakeUpMcuTimer)
    {
        g_timer_stop(g_wakeUpMcuTimer);
        g_timer_destroy(g_wakeUpMcuTimer);
        g_wakeUpMcuTimer = NULL;
    }
    g_wakeUpMcuCount = 0;
    G_UNLOCK(g_wakeUpMcuLock);
}

static void CheckWakeUpMcuTimeout(void)
{
    gdouble elapsedTime = 0;
    gint wakeupCount = 0;
    G_LOCK(g_wakeUpMcuLock);
    if (g_wakeUpMcuTimer != NULL)
    {
        elapsedTime = g_timer_elapsed(g_wakeUpMcuTimer, NULL);
    }
    wakeupCount = g_wakeUpMcuCount;
    G_UNLOCK(g_wakeUpMcuLock);
    if ((wakeupCount != 0) && (elapsedTime >= WAKEUP_MCU_TIMEOUT))
    {
        LOG_Warn("WakeUpMcu,But Mcu not request wakeupsource ,elapsedTime:[%f],wakeupCount:[%d]", elapsedTime, wakeupCount);
        LOG_Debug("Retry wakeup MCU");
        PmSchedWakeUpMcu();
    }
    if (wakeupCount > WAKEUP_MCU_MAX_COUNT)
    {
        /*Reset MCU*/
        LOG_Warn("Wakeup MCU Max Time:[%d],and reset mcu", wakeupCount);
        //ResetMcu();
        CleanWakeUpMcuTimer();
    }
}

static void PmForceSleepForMcuLost(void)
{
    LOG_Error("MCU heartbeat lost for %u seconds, force MPU sleep", MCU_HEARTBEAT_TIMEOUT_SEC);

    // 升级MCU期间不触发强制休眠
    if (RET_OK == PmSleepEventIsExist(PM_SLEEP_EVENT_UPGRADE))
    {
        LOG_Notice("MCU heartbeat lost but upgrade in progress, skip force sleep");
        return;
    }

    uint8_t WorkMode = PmSchedGetWorkMode();
    PmIpcSendPmState(WorkMode, PM_STATE_SLEEP, PM_WAKEUP_SOURCE_INVALID, CPU_INVALID_EVENT_ID);
    usleep(1*1000*1000);
	PmSchedSetPmState(PM_STATE_SLEEP);
    // PmModemSleep();
}

gpointer CheckWakeUpMcuTimeoutThread(gpointer arg)
{
    while(1)
    {
        CheckWakeUpMcuTimeout();
        usleep(1*1000*1000);
    }
}


/**
 * @brief 获取系统开机时长（秒）
 *
 * @details
 * 调用 Linux 的 sysinfo() 接口，读取系统自开机以来的运行秒数。
 *
 * @param[out] uptimeSeconds 输出参数，返回开机时长，单位：秒
 *
 * @return int32_t
 * @retval 0  获取成功
 * @retval -1 获取失败
 */
static int32_t GetSystemUptimeSeconds(uint64_t *uptimeSeconds)
{
    struct sysinfo sysInfo;

    if (uptimeSeconds == NULL)
    {
        return -1;
    }

    (void)memset(&sysInfo, 0, sizeof(sysInfo));

    if (sysinfo(&sysInfo) != 0)
    {
        return -1;
    }

    *uptimeSeconds = (uint64_t)sysInfo.uptime;
    return 0;
}

/**
 * @brief 获取系统非休眠工作时长（秒）
 *
 * @details
 * 调用 Linux 的 clock_gettime() 接口，读取系统自开机以来的非休眠运行秒数。
 *
 * @param[out] workSeconds 输出参数，返回开机时长，单位：秒
 *
 * @return int32_t
 * @retval 0  获取成功
 * @retval -1 获取失败
 */
static int32_t GetSystemWorktimeSeconds(uint64_t *workSeconds)
{
    struct timespec monoTime = {0};

    if (workSeconds == NULL)
    {
        return -1;
    }

    /* CLOCK_MONOTONIC：自上次重启的运行时长（休眠期间停走） */
    if(clock_gettime(CLOCK_MONOTONIC, &monoTime) != 0)
    {
        LOG_Error("clock_gettime(CLOCK_MONOTONIC) failed: %s", strerror(errno));
        return -1;
    }

    *workSeconds = (uint64_t)monoTime.tv_sec;
    return 0;
}

void PmMcuReqMpuSleepCallBack(const uint8_t *data, const uint16_t len, void* userdata)
{
    #define FORBIDDEN_SLEEP_SECONDS_FROM_POWERUP (120)
    #define SLEEP_MODE_NORMAL   (0)
    #define SLEEP_MODE_DEEP     (1)
    #define SLEEP_MODE_FAST     (2)
    uint8_t mcuBuf[4] = {0};
    uint8_t sleepMode = 0;
    uint64_t uptimeSeconds = 0;
    uint64_t workSeconds = 0;

    /* MCU request is 2 bytes: [index, sleepMode] */
    if(2 != len)
    {
        LOG_Error("%s recv mcu data leng is wrong.",__FUNCTION__);
        return;
    }
    else
    {
        memcpy(mcuBuf,data,len);
    }

    // 休眠模式读取
    sleepMode = mcuBuf[1];

    (void)GetSystemWorktimeSeconds(&workSeconds);
    int32_t getTickRet = GetSystemUptimeSeconds(&uptimeSeconds);

    LOG_Debug("%s - index : %d,sleepmode : %d, uptimeSeconds: %llu, workSeconds: %llu",__FUNCTION__,mcuBuf[0], sleepMode, uptimeSeconds, workSeconds);
    if ((RET_OK == PmSleepEventIsExist( PM_SLEEP_EVENT_UPGRADE )) ||  (RET_OK == PmSleepEventIsExist( PM_SLEEP_EVENT_ECALL)) \
        || ((sleepMode != SLEEP_MODE_FAST) && (getTickRet != -1) && (uptimeSeconds < FORBIDDEN_SLEEP_SECONDS_FROM_POWERUP)))
    {
        LOG_Notice("MCU request cpu sleep but cpu is updating or ecalling");
        return;
    }
    
    /* check auto reboot condition; tell MCU to reboot MPU and MCU after sleep if needed */
    mcuBuf[2] = (PmModemRebootCondCheck() == 1) ? 1 : 0;
    /** reserved **/
    mcuBuf[3] = 0;

    McuMesg_t mcuMesg = {0};
    mcuMesg.type = SYS_QUE_TYPE_MCU_SYNC_POWER_SLEEP_REQ_RES;
    mcuMesg.payload = mcuBuf;
    mcuMesg.payloadLen = sizeof(mcuBuf);
	McuSrvSendMesg(&mcuMesg);

    // 休眠模式
	if(sleepMode == SLEEP_MODE_NORMAL)
	{
        PmModemSleepQuicklyFlagClear();

        uint8_t WorkMode = PmSchedGetWorkMode();
        PmIpcSendPmState(WorkMode, PM_STATE_SLEEP, PM_WAKEUP_SOURCE_INVALID, CPU_INVALID_EVENT_ID);
        usleep(1*1000*1000);
		PmSchedSetPmState(PM_STATE_SLEEP);
	}
    //深度休眠模式
	else if(sleepMode == SLEEP_MODE_DEEP)
	{
        HalModemPmDeepSleepPowerDown(0);
	}
    // 快速休眠模式
    else if(sleepMode == SLEEP_MODE_FAST)
	{
        PmModemSleepQuicklyFlagSet();
        
        uint8_t WorkMode = PmSchedGetWorkMode();
        PmIpcSendPmState(WorkMode, PM_STATE_SLEEP, PM_WAKEUP_SOURCE_INVALID, CPU_INVALID_EVENT_ID);
        usleep(1*1000*1000);
		PmSchedSetPmState(PM_STATE_SLEEP);
	}
    else
    {
        LOG_Error("%s can not recognize sleepmode:[%d].",__FUNCTION__,mcuBuf[1]);
    }
}

void PmMcuReqSyncWakeupSourceCallBack(const uint8_t *data, const uint16_t len, void* userdata)
{
	uint8_t mcuBuf[4] = {0};

    if(len < 1 || len > 4)
    {
        LOG_Error("%s recv mcu data leng is wrong. len : %d.",__FUNCTION__,len);
        return;
    }
    else
    {
        memcpy(mcuBuf,data,len);
    }
    LOG_Debug("%s - wakeupSource : %d",__FUNCTION__,mcuBuf[0]);
    
    CleanWakeUpMcuTimer();
    
	uint8_t PmState = PmSchedGetPmState();
    if((PmState == PM_STATE_SLEEP))
    {
        LOG_Debug("Cur PowerState is Sleep,and Get MCU Notify wakeup source,then wakeUpSystem");
        PmModemWakeup(mcuBuf[0],CPU_INVALID_EVENT_ID);
    }

    mcuBuf[2] = PmSchedGetWakeupSrc();
    if(PM_WAKEUP_SOURCE_SMS == mcuBuf[2])
    {
         mcuBuf[3] = GetSmsSubWakeupType();
    }
    McuMesg_t mcuMesg = {0};
    mcuMesg.type = SYS_QUE_TYPE_MCU_SYNC_POWER_WAKEUPSOURCE_REQ_RES;
    mcuMesg.payload = mcuBuf;
    mcuMesg.payloadLen = sizeof(mcuBuf);
	McuSrvSendMesg(&mcuMesg);
    
}

void PmMcuSyncPowerMode(const uint8_t *data, const uint16_t len, void* userdata)
{
	uint8_t mcuBuf[2] = {0};

    static uint8_t mcuLastBuf[2] = {0xFF, 0xFF};
    static uint32_t syncPowerModeLogCnt = 0;
    int syncStateChanged = 0;

    if(sizeof(mcuBuf) != len)
    {
        LOG_Error("%s recv mcu data leng is wrong.",__FUNCTION__);
        return;
    }
    else
    {
        memcpy(mcuBuf,data,len);
    }
	
	if(mcuBuf[0] != 0)
	{
		PmSchedSetWorkMode(mcuBuf[1]);
    }

    syncPowerModeLogCnt++;
    syncStateChanged = memcmp(mcuLastBuf, mcuBuf, sizeof(mcuLastBuf));
    if (syncStateChanged)
    {
        memcpy(mcuLastBuf, mcuBuf, sizeof(mcuLastBuf));
    }

    if ((syncPowerModeLogCnt >= 5) || (syncStateChanged != 0))
    {
        syncPowerModeLogCnt = 0;
        LOG_Debug("from MCU - workmode : %d ,sleepmode : %d",mcuBuf[0],mcuBuf[1]);
    }

	uint8_t WorkMode = PmSchedGetWorkMode();
    uint8_t PmState = PmSchedGetPmState();
    PmIpcSendPmState(WorkMode, PmState, PM_WAKEUP_SOURCE_INVALID, CPU_INVALID_EVENT_ID);

    /* 收到 MCU 心跳 -> 通知监控线程（喂狗） */
    if (g_mcuHeartbeatSemInit == TRUE)
    {
        if (sem_post(&g_mcuHeartbeatSem) != 0)
        {
            LOG_Error("sem_post g_mcuHeartbeatSem failed, errno=%d", errno);
        }
    }
}

gpointer PmSyncPowerBlockEventToMcu(gpointer arg)
{
    static uint8_t lastBlockEvent = 0xFF;
    static uint32_t blockEventLogCnt = 0;

    while(1)
    {
        uint8_t mcuBuf[2] = {0};
        
        if (RET_OK == PmSleepEventIsExist(PM_SLEEP_EVENT_ECALL))
        {
            mcuBuf[1] |= 0x01;
        }
        if (RET_OK == PmSleepEventIsExist(PM_SLEEP_EVENT_UPGRADE))
        {
            mcuBuf[1] |= 0x02;
        }

        if (PM_STATE_NORMAL == PmSchedGetPmState())
        {
            McuMesg_t mcuMesg = {0};
            mcuMesg.type = SYS_QUE_TYPE_MCU_SYNC_POWER_BLOCK_EVENT;
            mcuMesg.payload = mcuBuf;
            mcuMesg.payloadLen = sizeof(mcuBuf);
            McuSrvSendMesg(&mcuMesg);
            
            blockEventLogCnt++;
            if ((blockEventLogCnt >= 5) || (mcuBuf[1] != lastBlockEvent))
            {
                LOG_Debug("to MCU - type : %x ,Counter : %d blockEvent : %d ",mcuMesg.type,mcuBuf[0],mcuBuf[1]);
                lastBlockEvent = mcuBuf[1];
                blockEventLogCnt = 0;
            }
        }
        else
        {
            LOG_Debug("%s : go to sleep.",__FUNCTION__);
        }
        
        usleep(1*1000*1000);
    }
    
    return NULL;
}

// 监控MCU心跳，连续60秒收不到心跳，强制休眠
gpointer PmMonitorMcuHeartbeat(gpointer arg)
{
    uint32_t lostCount = 0;
    struct timespec ts;
    int32_t ret;

    LOG_Debug("PmMonitorMcuHeartbeat thread start");

    while (g_mcuHeartbeatThreadRunning == TRUE)
    {
        if (g_mcuHeartbeatSemInit == FALSE)
        {
            /* 信号量还没初始化好，稍微睡一会儿再看 */
            sleep(1);
            continue;
        }

        uint8_t PmState = PmSchedGetPmState();
	    if(PmState == PM_STATE_SLEEP)
        {
            LOG_Debug("PmMonitorMcuHeartbeat sleep, stop count");
            lostCount = 0;
            sleep(1);
            continue;
        }

        /* 计算 1 秒超时时间点 */
        if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
        {
            LOG_Error("clock_gettime failed, errno=%d", errno);
            sleep(1);
            continue;
        }

        ts.tv_sec += MCU_HEARTBEAT_INTERVAL_SEC;

        /* 等待 1 秒内是否有心跳到来（sem_post） */
        ret = sem_timedwait(&g_mcuHeartbeatSem, &ts);
        if (ret == 0)
        {
            /* 正常收到心跳：丢包计数清零 */
            g_atomic_int_set(&g_mcuHeartbeatAlive, TRUE);
            if (lostCount >= 5u)
            {
                LOG_Debug("MCU heartbeat recovered, lostCount reset from %u", lostCount);
            }
            lostCount = 0u;
        }
        else
        {
            if (errno == ETIMEDOUT)
            {
                /* 1 秒内没有心跳，累计丢失次数 */
                lostCount++;
                if (lostCount >= 5u)
                {
                    LOG_Debug("MCU heartbeat timeout, count=%u", lostCount);
                }

                if (lostCount >= MCU_HEARTBEAT_ALIVE_TIMEOUT_SEC)
                {
                    /* 5 秒无心跳 -> MCU 视为不在线 */
                    g_atomic_int_set(&g_mcuHeartbeatAlive, FALSE);
                }

                if (lostCount >= MCU_HEARTBEAT_TIMEOUT_SEC)
                {
                    /* 连续 60 秒无心跳 -> 强制休眠 MPU */
                    PmForceSleepForMcuLost();

                    /* 根据业务决定：
                     *  1) 如果休眠后不会再跑到这里，可以 break;
                     *  2) 如果希望醒来后继续监控，可以将 lostCount 置 0。
                     */
                    lostCount = 0u;
                }
            }
            else if (errno == EINTR)
            {
                /* 被信号打断，忽略继续循环 */
                continue;
            }
            else
            {
                LOG_Error("sem_timedwait error, errno=%d", errno);
                sleep(1);
            }
        }
    }

    LOG_Debug("PmMonitorMcuHeartbeat thread exit");
    return NULL;
}

gboolean PmInitMcuHeartbeatMonitor(void)
{
    if (g_mcuHeartbeatSemInit == TRUE)
    {
        return TRUE;
    }

    if (sem_init(&g_mcuHeartbeatSem, 0, 0) != 0)
    {
        LOG_Error("sem_init g_mcuHeartbeatSem failed, errno=%d", errno);
        return FALSE;
    }

    g_mcuHeartbeatSemInit = TRUE;
    g_mcuHeartbeatThreadRunning = TRUE;

    /* 创建监控线程（glib 线程） */
    if (g_thread_new("PmMonitorMcuHeartbeat", PmMonitorMcuHeartbeat, NULL) == NULL)
    {
        LOG_Error("create PmMonitorMcuHeartbeat thread failed");
        g_mcuHeartbeatSemInit = FALSE;
        (void)sem_destroy(&g_mcuHeartbeatSem);
        return FALSE;
    }

    LOG_Debug("PmInitMcuHeartbeatMonitor success");
    return TRUE;
}

gboolean PmMcuHeartbeatIsAlive(void)
{
    return g_atomic_int_get(&g_mcuHeartbeatAlive);
}
