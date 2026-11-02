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

// CLOCK_REALTIME_ALARM, support wakeup system
#define CLOCKID (8)
#define PMRTCLOCK "PMRTCLOCK"

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
	uint8_t g_TimerWakeupSrc = v.sival_int;
	
	if(g_TimerWakeupSrc == CPU_SEND_HEARTBEAT_SLEEPALARM_EVENT_ID)
	{
        HalModemPmWakeLockLocked((uint8_t*)"PmModemLock");
		PmModemWakeup(PM_WAKEUP_SOURCE_TIMER_HEARTBEAT, g_TimerWakeupSrc);
        HalModemPmWakeLockUnlocked((uint8_t*)"PmModemLock");
        LOG_Debug("rtc wakeup for sending heartbeat");
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
	else //ota creat from outside timer api
	{
		LOG_Debug("rtc wakeup api timer wakeup source event id:[%d]", g_TimerWakeupSrc);
		PmModemWakeup(PM_WAKEUP_SOURCE_TIMER, g_TimerWakeupSrc);
	}
    LOG_Debug("Get RTC WakeUp,EventID:[%d]", g_TimerWakeupSrc); 
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
    it.it_interval.tv_sec = SleepAlarmSecond;
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
	return RET_OK;
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
	
    for(GList *iter = g_TimerList; iter != NULL; iter = iter->next)
	{
		PmSleepAlarm_t *PmSleepAlarm = (PmSleepAlarm_t *)iter->data;
		if(PmSleepAlarm)
		{
			if(PmSleepAlarm->SleepAlarmEventId == SleepAlarmEventId)
			{
				timer_delete(PmSleepAlarm->TimeId);
				g_TimerList = g_list_remove_link(g_TimerList, iter);
				g_list_free_full(iter, (GDestroyNotify)FreePmSleepAlarm);
			}
		}
    }
	
	pthread_rwlock_unlock(&g_TimerListMutex);	
	return RET_OK;
}

int64_t GetTimerIrqTimesFromCurrent(timer_t timer_id)
{
    struct itimerspec remaining_time;
    struct timespec current_time;
    struct timespec next_expiry;

    // 1. 获取定时器剩余时间（下次触发的相对时间）
    if (timer_gettime(timer_id, &remaining_time) == -1) {
        LOG_Error("timer_gettime 失败");
        return;
    }

    // 如果 it_value 为0，说明定时器已停止
    if (remaining_time.it_value.tv_sec == 0 && remaining_time.it_value.tv_nsec == 0) {
        LOG_Error("定时器已停止，无下次中断时间\n");
        return;
    }

    // // 2. 获取当前系统时间（CLOCK_REALTIME 时钟，与定时器时钟一致）
    // if (clock_gettime(CLOCK_REALTIME, &current_time) == -1) {
    //     LOG_Error("clock_gettime 失败");
    //     return;
    // }

    // // 3. 计算下次中断的绝对时间（当前时间 + 剩余时间）
    // // 先加纳秒，处理进位（1秒 = 1e9纳秒）
    // next_expiry.tv_nsec = current_time.tv_nsec + remaining_time.it_value.tv_nsec;
    // next_expiry.tv_sec = current_time.tv_sec + remaining_time.it_value.tv_sec;
    // if (next_expiry.tv_nsec >= 1000000000) {
    //     next_expiry.tv_sec += 1;
    //     next_expiry.tv_nsec -= 1000000000;
    // }

    // // 打印结果（秒 + 纳秒，也可转换为可读时间格式）
    // LOG_Error("下次定时器中断时间：%ld 秒 + %ld 纳秒（绝对时间）\n",
    //        next_expiry.tv_sec, next_expiry.tv_nsec);
    // LOG_Error("距离现在还有：%ld 秒 + %ld 纳秒（相对时间）\n\n",
    //        remaining_time.it_value.tv_sec, remaining_time.it_value.tv_nsec);
    LOG_Error("距离现在还有：%ld 秒", remaining_time.it_value.tv_sec);
    return remaining_time.it_value.tv_sec;
}


int32_t GetTimerMinTimesToFreePmLock()
{
    int64_t minTimes = 0;
    int64_t remainingTime = 0;

	pthread_rwlock_wrlock(&g_TimerListMutex);
	
    for(GList *iter = g_TimerList; iter != NULL; iter = iter->next)
	{
		PmSleepAlarm_t *PmSleepAlarm = (PmSleepAlarm_t *)iter->data;
		if(PmSleepAlarm)
		{
			if(PmSleepAlarm->TimeId >= 0)
			{
				remainingTime = GetTimerIrqTimesFromCurrent(PmSleepAlarm->TimeId);
			}

            if(0 == minTimes && 0 != remainingTime)
            {
                minTimes = remainingTime;
            }

            if(remainingTime <= minTimes)
            {
                minTimes = remainingTime;
            }
		}
    }
	
    if(minTimes >= 10)
    {
        sleep(2);// 如果是其他唤醒源时等待2S，处理如果时数据唤醒，APP唤醒需要时间，并且唤醒时需要上报心跳。定时器中断时间会重置。
        HalModemPmWakeLockUnlocked((uint8_t*)"PmModemLock");
    }
    LOG_Error("minTimes %ld 秒", minTimes);
	pthread_rwlock_unlock(&g_TimerListMutex);	
	return RET_OK;
}

