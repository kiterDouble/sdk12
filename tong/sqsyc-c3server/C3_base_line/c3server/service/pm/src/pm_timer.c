#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <glib.h>
#include <errno.h>
#include <string.h>

#include "pm_timer.h"
#include "err_code.h"
#include "sr_log.h"
#include "pm_server_api.h"
#include "pm_modem.h"
#include "hal_modem_pm.h"

// CLOCK_REALTIME_ALARM, support wakeup system
#define CLOCKID (8)

/* CLOCK_BOOTTIME_ALARM, support wakeup system */
#define CLOCKID_BOOTTIME (9)

#define PM_TIMER_LOCKER_NAME "PmTimerThreadLock"

typedef struct
{
    timer_t TimeId;
    uint8_t SleepAlarmEventId;
	uint16_t WakeupOption;
} PmSleepAlarm_t;

static GList *g_TimerList = NULL;
static pthread_rwlock_t g_TimerListMutex = PTHREAD_RWLOCK_INITIALIZER;

static void TimerThread(union sigval v)
{

	/* 第一时间拿 wakelock 兜底，避免 RTC 唤醒后 TimerThread 尚未派发到 PmModemWakeup
	 * 期间，主循环的 PmModemSleep 把系统按回 suspend */
	int32_t timerLockFd = HalModemPmWakeLockLockedEx((const uint8_t *)PM_TIMER_LOCKER_NAME);

	uint8_t g_TimerWakeupSrc = v.sival_int;

    LOG_Debug("Get RTC WakeUp,EventID:[%d]", g_TimerWakeupSrc);
    
	
	if(g_TimerWakeupSrc == CPU_SEND_HEARTBEAT_SLEEPALARM_EVENT_ID)
	{
        LOG_Debug("rtc wakeup for sending heartbeat");
		PmModemWakeup(PM_WAKEUP_SOURCE_TIMER_HEARTBEAT, g_TimerWakeupSrc);
    }
	else if(g_TimerWakeupSrc == CPU_REBOOT_SLEEPALARM_EVENT_ID)
	{
        LOG_Debug("rtc wakeup for reboot cpu");
		PmModemWakeup(PM_WAKEUP_SOURCE_TIMER_REBOOT, g_TimerWakeupSrc);
	}
	else if(g_TimerWakeupSrc == CPU_WEEK_SLEEPALARM_EVENT_ID)
	{
        LOG_Debug("rtc wakeup for week wakeup cpu");
		PmModemWakeup(PM_WAKEUP_SOURCE_TIMER_WEEK, g_TimerWakeupSrc);
	}
    else if(g_TimerWakeupSrc == CPU_APP_TIMER_BATTERY_HEAT)
    {
        LOG_Debug("rtc wakeup for timer wakeup cpu and mcu");
        PmModemWakeup(PM_WAKEUP_SOURCE_TIMER_MCU, g_TimerWakeupSrc);
    }
    else if(g_TimerWakeupSrc == CPU_APP_TIMER_TCP_KEEPALIVE)
    {
        LOG_Debug("rtc wakeup for timer wakeup cpu and mcu");
        PmModemWakeup(PM_WAKEUP_SOURCE_TIMER_HEARTBEAT, g_TimerWakeupSrc);
    }
	else //ota creat from outside timer api
	{
		LOG_Debug("rtc wakeup api timer wakeup source event id:[%d]", g_TimerWakeupSrc);
		PmModemWakeup(PM_WAKEUP_SOURCE_TIMER, g_TimerWakeupSrc);

        if (g_TimerWakeupSrc == 0x12u) // ota rtc wakeup--app
        {
            LOG_Debug("ota rtc wakeupdone, destory");
            (void)PmDestroySleepAlarm(0x12u);
        }
	}

	/* 派发完毕：把入口加的 wakelock 兜底释放掉，
	 * 后续由 PmModemWakeup/PmModemSleep 各自管理 wakelock 状态 */
	if (timerLockFd >= 0)
	{
		(void)HalModemPmWakeLockUnlockedEx(timerLockFd);
	}
}

int8_t PmSleepAlarmEventIdIsExist(uint8_t SleepAlarmEventId)
{
	pthread_rwlock_rdlock(&g_TimerListMutex);
	
    for(GList *iter = g_TimerList; iter != NULL; iter = iter->next)
	{
		PmSleepAlarm_t *PmSleepAlarm = (PmSleepAlarm_t *)iter->data;
		if(PmSleepAlarm)
		{
			if(PmSleepAlarm->SleepAlarmEventId == SleepAlarmEventId)
			{
                LOG_Debug("SleepAlarmEventId[%d] exist in the list", SleepAlarmEventId);
				pthread_rwlock_unlock(&g_TimerListMutex);
				return RET_OK;
			}
		}
    }
	
	pthread_rwlock_unlock(&g_TimerListMutex);
	
	return RET_ERR;
}

static int8_t AddPmSleepAlarmEventId(timer_t TimeId, uint8_t SleepAlarmEventId, uint16_t WakeupOption)
{
	PmSleepAlarm_t *PmSleepAlarm = g_try_malloc0(sizeof(PmSleepAlarm_t));
	if(NULL == PmSleepAlarm)
	{
        LOG_Error("%s Malloc mem error:[%s]",__FUNCTION__, g_strerror(errno));
		return RET_ERR;
    }
	
	PmSleepAlarm->TimeId = TimeId;
	PmSleepAlarm->SleepAlarmEventId = SleepAlarmEventId;
	PmSleepAlarm->WakeupOption = WakeupOption;
	g_TimerList = g_list_append(g_TimerList, PmSleepAlarm);
    
	return RET_OK;
}

int8_t CreatePmSleepAlarm(uint8_t SleepAlarmEventId, uint32_t SleepAlarmSecond, uint16_t WakeupOption)
{
    pthread_rwlock_wrlock(&g_TimerListMutex);
    
    for(GList *iter = g_TimerList; iter != NULL; iter = iter->next)
	{
		PmSleepAlarm_t *PmSleepAlarm = (PmSleepAlarm_t *)iter->data;
		if(PmSleepAlarm)
		{
			if(PmSleepAlarm->SleepAlarmEventId == SleepAlarmEventId)
			{
                LOG_Debug("SleepAlarmEventId[%d] exist in the list", SleepAlarmEventId);
				pthread_rwlock_unlock(&g_TimerListMutex);
				return RET_ERR;
			}
		}
    }
	
	struct sigevent evp;
    memset(&evp, 0x00, sizeof(struct sigevent));
    evp.sigev_value.sival_int = SleepAlarmEventId;
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = TimerThread;
	
	timer_t TimeId = 0;
	int32_t ret = timer_create(CLOCKID, &evp, &TimeId);
	if(ret == -1)
	{
        LOG_Error("timer_create  failed:Eventid[%d]-timeOut[%d]-[%s]", SleepAlarmEventId, SleepAlarmSecond, g_strerror(errno));
        pthread_rwlock_unlock(&g_TimerListMutex);
		return RET_ERR;
	}

    struct itimerspec it;
    it.it_interval.tv_sec = 0; // 不循环，触发后自动失效
    it.it_interval.tv_nsec = 0;
    it.it_value.tv_sec = SleepAlarmSecond;
    it.it_value.tv_nsec = 0;
	
	ret = timer_settime(TimeId, 0, &it, NULL);
	if(ret == -1)
	{
        LOG_Error("timer_settime  failed:Eventid[%d]-timeOut[%d]-[%s]", SleepAlarmEventId, SleepAlarmSecond, g_strerror(errno));
        timer_delete(TimeId);
        pthread_rwlock_unlock(&g_TimerListMutex);
		return RET_ERR;
	}
    
    if(RET_OK != AddPmSleepAlarmEventId(TimeId, SleepAlarmEventId,WakeupOption))
	{
        LOG_Error("%s AddPmSleepAlarmEventId To List Error",__FUNCTION__);
        timer_delete(TimeId);
        pthread_rwlock_unlock(&g_TimerListMutex);
		return RET_ERR;
	}
	
	pthread_rwlock_unlock(&g_TimerListMutex);
    LOG_Debug("CreatePmSleepAlarm Success:Eventid[%d]-timeOut[%ds]-timeID[%lu]", SleepAlarmEventId, SleepAlarmSecond,
              (unsigned long)TimeId);
    return RET_OK;
}


/**
 * @brief 设置休眠定时器（CLOCK_BOOTTIME_ALARM），当定时器超时，唤醒 MPU
 *        与CreatePmSleepAlarm的区别：基于开机时长计时（含休眠），不受系统对时跳变影响
 *
 * @param sleepAlarmEventId uint8_t 定时器类型
 * @param sleepAlarmSecond uint32_t 定时器超时时间，单位秒
 * @param wakeupOption uint16_t 唤醒选项
 * @return int8_t 数据发送结果，取值：RET_ERR 失败，RET_OK 成功
 */
int8_t CreatePmSleepAlarmByBootTime(uint8_t sleepAlarmEventId, uint32_t sleepAlarmSecond, uint16_t wakeupOption)
{
    int8_t ret = RET_ERR;
    int32_t timerRet = 0;
    uint8_t idExist = 0;
    timer_t timeId = 0;
    struct sigevent evp = {0};
    struct itimerspec it = {0};
    GList *iter = NULL;
    PmSleepAlarm_t *pmSleepAlarm = NULL;

    pthread_rwlock_wrlock(&g_TimerListMutex);

    do
    {
        for(iter = g_TimerList; iter != NULL; iter = iter->next)
        {
            pmSleepAlarm = (PmSleepAlarm_t *)iter->data;
            if((pmSleepAlarm != NULL) && (pmSleepAlarm->SleepAlarmEventId == sleepAlarmEventId))
            {
                LOG_Debug("sleepAlarmEventId[%d] exist in the list", sleepAlarmEventId);
                idExist = 1;
                break;
            }
        }
        if(idExist != 0)
        {
            break;
        }

        evp.sigev_value.sival_int = sleepAlarmEventId;
        evp.sigev_notify = SIGEV_THREAD;
        evp.sigev_notify_function = TimerThread;

        timerRet = timer_create(CLOCKID_BOOTTIME, &evp, &timeId);
        if(timerRet == -1)
        {
            LOG_Error("timer_create BOOTTIME failed:Eventid[%d]-timeOut[%d]-[%s]", sleepAlarmEventId, sleepAlarmSecond,
                      g_strerror(errno));
            break;
        }

        it.it_interval.tv_sec = 0; /* 不循环，触发后自动失效 */
        it.it_value.tv_sec = sleepAlarmSecond;

        timerRet = timer_settime(timeId, 0, &it, NULL);
        if(timerRet == -1)
        {
            LOG_Error("timer_settime BOOTTIME failed:Eventid[%d]-timeOut[%d]-[%s]", sleepAlarmEventId, sleepAlarmSecond,
                      g_strerror(errno));
            timer_delete(timeId);
            break;
        }

        ret = AddPmSleepAlarmEventId(timeId, sleepAlarmEventId, wakeupOption);
        if(ret != RET_OK)
        {
            LOG_Error("%s AddPmSleepAlarmEventId To List Error", __FUNCTION__);
            timer_delete(timeId);
            break;
        }

        LOG_Debug("CreatePmSleepAlarmByBootTime Success:Eventid[%d]-timeOut[%ds]-timeId[%lu]", sleepAlarmEventId,
                  sleepAlarmSecond, (unsigned long)timeId);
    } while(0);

    pthread_rwlock_unlock(&g_TimerListMutex);

    return ret;
}

static uint32_t CalculateHowLong2SetTime(uint8_t sleepAlarmHour, uint8_t sleepAlarmMinute)
{
    time_t ts;
    struct tm* ptm = NULL;
    long total_sec = 0, tmp_setTime2Sec, now_time2Sec;

    ts = time(NULL);
    ptm = localtime(&ts);//get current time

    tmp_setTime2Sec = sleepAlarmHour*3600 + sleepAlarmMinute*60;//将设置的闹钟时间转化为秒
    now_time2Sec = ptm->tm_hour*3600 + ptm->tm_min*60 + ptm->tm_sec;//将当前时间的时分秒转化为秒


    if(tmp_setTime2Sec == now_time2Sec)//当前时间与设置的闹钟时间相同
    {
        total_sec = 24*3600;
    }
    else if (tmp_setTime2Sec > now_time2Sec)//当前时间早于闹钟时间
    {
        total_sec = tmp_setTime2Sec - now_time2Sec;
    }
    else//当前时间晚于闹钟时间
    {
        total_sec = 24*3600 - (now_time2Sec - tmp_setTime2Sec);
    }
    
    LOG_Debug("tmp_setTime2Sec=%ldS now_time2Sec=%ldS total_sec=%ldS", tmp_setTime2Sec,now_time2Sec,total_sec);
    return total_sec;
}

int8_t CreatePmSleepAlarmEveryDay(uint8_t SleepAlarmEventId, uint8_t SleepAlarmHour, uint8_t SleepAlarmMinute, uint16_t WakeupOption)
{
    timer_t timeID = 0;
    long interval_sec = 24*60*60;//24小时的秒数
    long it_value_sec = 0;
    struct sigevent evp;
    gint ret = RET_OK;
    
    pthread_rwlock_wrlock(&g_TimerListMutex);
    for(GList *iter = g_TimerList; iter != NULL; iter = iter->next)
	{
		PmSleepAlarm_t *PmSleepAlarm = (PmSleepAlarm_t *)iter->data;
		if(PmSleepAlarm)
		{
			if(PmSleepAlarm->SleepAlarmEventId == SleepAlarmEventId)
			{
                LOG_Debug("SleepAlarmEventId[%d] exist in the list", SleepAlarmEventId);
				pthread_rwlock_unlock(&g_TimerListMutex);
				return RET_ERR;
			}
		}
    }

    memset(&evp, 0, sizeof(struct sigevent));
    evp.sigev_value.sival_int = SleepAlarmEventId;
    evp.sigev_notify = SIGEV_THREAD;          // The thread notifying way -- dispatch a new thread.
    evp.sigev_notify_function = TimerThread; // The thread address

    if (timer_create(CLOCKID, &evp, &timeID) == -1)
    {
        LOG_Error("timer_create  failed:Eventid[%d]-timeOut[%dh %dmin]-[%s]", SleepAlarmEventId, SleepAlarmHour,SleepAlarmMinute, g_strerror(errno));
        pthread_rwlock_unlock(&g_TimerListMutex);
        return RET_ERR;
    }

    if((SleepAlarmHour > 24 || SleepAlarmHour < 0) || (SleepAlarmMinute > 60 || SleepAlarmMinute < 0))
    {
        LOG_Error("%s : SleepAlarmHour(%d) or SleepAlarmMinute(%d) format is wrong. ",__FUNCTION__,SleepAlarmHour,SleepAlarmMinute);
        timer_delete(timeID);
        pthread_rwlock_unlock(&g_TimerListMutex);
        return RET_ERR;
    }
    it_value_sec = CalculateHowLong2SetTime(SleepAlarmHour,SleepAlarmMinute);
    
    struct itimerspec it;
    it.it_interval.tv_sec = interval_sec; //periodic or not, assign it_interval to it_value once the it_value time expiry
    it.it_interval.tv_nsec = 0;
    it.it_value.tv_sec = it_value_sec;
    it.it_value.tv_nsec = 0;

    if (timer_settime(timeID, 0, &it, NULL) == -1)
    {
        LOG_Error("timer_settime  failed:Eventid[%d]-timeOut[%dh %dmin]-[%s]", SleepAlarmEventId, SleepAlarmHour,SleepAlarmMinute, g_strerror(errno));
        timer_delete(timeID);
        pthread_rwlock_unlock(&g_TimerListMutex);
        return RET_ERR;
    }
    LOG_Debug("CreateSleepAlarm Success:Eventid[%d]-timeOut[%dh %dmin]-timeID[%lu]", SleepAlarmEventId, SleepAlarmHour,SleepAlarmMinute, (unsigned long)timeID);

    if(RET_OK != AddPmSleepAlarmEventId(timeID, SleepAlarmEventId,WakeupOption))
    {
        LOG_Error("Add SleepEvent To List Error");
        timer_delete(timeID);
        ret = RET_ERR;
    }

    pthread_rwlock_unlock(&g_TimerListMutex);
    return ret;
}

static void FreePmSleepAlarm(PmSleepAlarm_t *PmSleepAlarm)
{
    if(PmSleepAlarm)
        g_free(PmSleepAlarm);
}
int8_t DestroyPmSleepAlarm(uint8_t SleepAlarmEventId)
{
    pthread_rwlock_wrlock(&g_TimerListMutex);

    for (GList *iter = g_TimerList; iter != NULL; iter = iter->next)
    {
        PmSleepAlarm_t *pmSleepAlarm = (PmSleepAlarm_t *)iter->data;
        if ((pmSleepAlarm != NULL) && (pmSleepAlarm->SleepAlarmEventId == SleepAlarmEventId))
        {
            LOG_Info("DestroyPmSleepAlarm: Start destroying timer, EventId[%d], TimeId[%lu]",
                        SleepAlarmEventId, (unsigned long)pmSleepAlarm->TimeId);

            /* 先停止定时器，避免在删除过程中触发 */
            struct itimerspec stop_timer = {0};
            if (timer_settime(pmSleepAlarm->TimeId, 0, &stop_timer, NULL) == -1) {
                LOG_Warn("DestroyPmSleepAlarm: Failed to stop timer before delete, EventId[%d], error[%s]",
                            SleepAlarmEventId, strerror(errno));
            } else {
                LOG_Debug("DestroyPmSleepAlarm: Timer stopped successfully, EventId[%d]", SleepAlarmEventId);
            }

            /* 删除定时器 */
            if (timer_delete(pmSleepAlarm->TimeId) == -1) {
                LOG_Error("DestroyPmSleepAlarm: timer_delete failed, EventId[%d], TimeId[%lu], error[%s]",
                            SleepAlarmEventId, (unsigned long)pmSleepAlarm->TimeId, strerror(errno));
            }
            LOG_Debug("DestroyPmSleepAlarm: timer_delete success, EventId[%d]", SleepAlarmEventId);

            /* 先释放 data，再删除 link（或反过来，但要一致且不再使用 iter） */
            FreePmSleepAlarm(pmSleepAlarm);
            g_TimerList = g_list_delete_link(g_TimerList, iter);

            LOG_Debug("DestroyPmSleepAlarm complte :Eventid[%d]", SleepAlarmEventId);
            break;
        }
    }

    pthread_rwlock_unlock(&g_TimerListMutex);
	return RET_OK;
}

