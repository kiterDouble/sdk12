#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "pm_modem.h"
#include "pm_sched.h"
#include "pm_timer.h"
#include "pm_ipc_rpc.h"
#include "pm_server_api.h"
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_pm.h"
#include "hal_modem_nw.h"
#include "pm_event.h"
#include "pm_device_control.h"
#include "pm_mcu_control.h"
#include "hal_modem_call.h"
#include "hal_modem_sms.h"
#include "pm_timer.h"



#define PM_SCHED_NORMAL 1
#define PM_SCHED_SLEEP 2

static uint8_t g_WakeupSrc = PM_WAKEUP_SOURCE_INVALID;
static pthread_rwlock_t g_WakeupSrcMutex = PTHREAD_RWLOCK_INITIALIZER;

uint8_t PmSchedGetWakeupSrc(void)
{
	pthread_rwlock_rdlock(&g_WakeupSrcMutex);
	uint8_t WakeupSrc = g_WakeupSrc;
	pthread_rwlock_unlock(&g_WakeupSrcMutex);
	
	return WakeupSrc;
}

void PmSchedSetWakeupSrc(uint8_t WakeupSrc)
{
	pthread_rwlock_wrlock(&g_WakeupSrcMutex);
	g_WakeupSrc = WakeupSrc;
	pthread_rwlock_unlock(&g_WakeupSrcMutex);
}

#define PMWAKELOCK "PmModemLock"

static int32_t g_WakeLockFdFlag = 0;
static int32_t g_wakeLockDelayCount = 0;
static pthread_mutex_t g_WakeUpSystemMutex = PTHREAD_MUTEX_INITIALIZER;

void PmModemCallCallBack(const uint8_t *data, const uint16_t len, void* userdata)
{
    PmModemWakeup(PM_WAKEUP_SOURCE_RING,CPU_INVALID_EVENT_ID);
}

void PmModemSmsCallBack(const uint8_t *data, const uint16_t len, void* userdata)
{
    if(GetSmsWakeMode())
    {
        PmModemWakeup(PM_WAKEUP_SOURCE_SMS,CPU_INVALID_EVENT_ID);
    }
}

void PmModemEcallIncomingCallBack(const uint8_t *data, const uint16_t len, void* userdata)
{
	PmModemWakeup(PM_WAKEUP_SOURCE_ECALL_INCOME,CPU_INVALID_EVENT_ID);
}

void PmModemInit(void)
{
    HalModemNwInit();
	HalModemPmWakeLockCreate((uint8_t*)PMWAKELOCK);
	if(g_WakeLockFdFlag == 0)
	{
		HalModemPmWakeLockLocked((uint8_t*)PMWAKELOCK);
		g_WakeLockFdFlag = 1;
	}
	
	HalModemPmAutosleepEnable(1);

}

void PmModemExit(void)
{
	
	HalModemPmAutosleepEnable(0);
	
	if(g_WakeLockFdFlag == 1)
	{
		HalModemPmWakeLockUnlocked((uint8_t*)PMWAKELOCK);
		g_WakeLockFdFlag = 0;
	}

    //hal 接口定义未实现
	//HalModemPmWakeLockDestroy(PMWAKELOCK); 
	HalModemNwDeinit();
}

static void PmModemLock(void)
{
    pthread_mutex_lock(&g_WakeUpSystemMutex);
	if(g_WakeLockFdFlag == 0)
	{
		int32_t ret = HalModemPmWakeLockLocked((uint8_t*)PMWAKELOCK);
        if(ret == RET_OK)
        {
            g_WakeLockFdFlag = 1;
        }
        LOG_Debug("%s : g_WakeLockFdFlag[%d]",__FUNCTION__,g_WakeLockFdFlag);
	}
    LOG_Debug("%s : g_WakeLockFdFlag[%d]",__FUNCTION__,g_WakeLockFdFlag);
    pthread_mutex_unlock(&g_WakeUpSystemMutex);
}

static void PmModemLockAndNotWakeUp(uint8_t WakeupSrc)
{
    pthread_mutex_lock(&g_WakeUpSystemMutex);
	if(g_WakeLockFdFlag == 0)
	{
		int32_t ret = HalModemPmWakeLockLocked((uint8_t*)PMWAKELOCK);
        if(ret == RET_OK && WakeupSrc != PM_WAKEUP_SOURCE_TIMER_HEARTBEAT)
        {
            g_WakeLockFdFlag = 1;
        }
        LOG_Debug("%s : g_WakeLockFdFlag[%d]",__FUNCTION__,g_WakeLockFdFlag);
	}
    LOG_Debug("%s : g_WakeLockFdFlag[%d]",__FUNCTION__,g_WakeLockFdFlag);
    pthread_mutex_unlock(&g_WakeUpSystemMutex);
}

static void PmModemUnLock(void)
{
    pthread_mutex_lock(&g_WakeUpSystemMutex);
	if(g_WakeLockFdFlag == 1)
	{
		int32_t ret = HalModemPmWakeLockUnlocked((uint8_t*)PMWAKELOCK);
        if(ret == RET_OK)
        {
            g_WakeLockFdFlag = 0;
        }
        LOG_Debug("%s : g_WakeLockFdFlag[%d]",__FUNCTION__,g_WakeLockFdFlag);
	}
    else if(1 == HalModemGetPmWakeLockStatus((uint8_t*)PMWAKELOCK))
    {
        GetTimerMinTimesToFreePmLock();
    }
    LOG_Debug("%s : g_WakeLockFdFlag[%d]",__FUNCTION__,g_WakeLockFdFlag);
    pthread_mutex_unlock(&g_WakeUpSystemMutex);
}

int8_t PmModemWakeup(uint8_t WakeupSrc, uint8_t WakeupOption)
{
 
    PmModemLockAndNotWakeUp(WakeupSrc);
    SetPmSleepEvent("PmModemWakeup");
    LOG_Debug("inter %s : wakeupsrc : %hhu,WakeupOption : %hhu",__FUNCTION__,WakeupSrc,WakeupOption);
	pthread_mutex_lock(&g_WakeUpSystemMutex);
    
    //HSM 唤醒
    //HsmEnterNormalMode();
    PmSchedSetWakeupSrc(WakeupSrc);

	uint8_t WorkMode = PmSchedGetWorkMode();
	uint8_t PmState = PmSchedGetPmState();

	if(PmState == PM_STATE_SLEEP)
	{
        LOG_Debug("cur sleep mode is Sleep,and then wakeup system ,and wakeup source is %d.",WakeupSrc);
		
		if(WakeupSrc == PM_WAKEUP_SOURCE_TIMER_HEARTBEAT)
		{
			PmIpcSendPmState(WorkMode, PM_STATE_SLEEP, WakeupSrc, WakeupOption);
			/*SetPmSleepEvent("keepWakeup1");
			usleep(2*1000*1000);
			CleanPmSleepEvent("keepWakeup1");*/
		}
		else if(WakeupSrc == PM_WAKEUP_SOURCE_TIMER_REBOOT)
		{
			// to do
            LOG_Debug("inter %s : wakeupsrc : %hhu,WakeupOption : %hhu",__FUNCTION__,WakeupSrc,WakeupOption);
        }
		else
		{
            PmSchedSetPmState(PM_STATE_NORMAL);
            if(WakeupSrc == PM_WAKEUP_SOURCE_TIMER_WEEK || WakeupSrc == PM_WAKEUP_SOURCE_TIMER)
            {
                LOG_Debug("inter %s : wakeupsrc : %hhu,WakeupOption : %hhu",__FUNCTION__,WakeupSrc,WakeupOption);
                PmIpcSendPmState(WorkMode, PM_STATE_NORMAL, WakeupSrc, WakeupOption);
            }
            else
            {
                PmIpcSendPmState(WorkMode, PM_STATE_NORMAL, WakeupSrc, CPU_INVALID_EVENT_ID);
            }
		}
		
		if(WakeupSrc == PM_WAKEUP_SOURCE_SMS || WakeupSrc == PM_WAKEUP_SOURCE_RING || WakeupSrc == PM_WAKEUP_SOURCE_ECALL_INCOME)
		{
			PmSchedWakeUpMcu();
        }

        if(WakeupSrc != PM_WAKEUP_SOURCE_TIMER && WakeupSrc != PM_WAKEUP_SOURCE_DATA 
            && WakeupSrc != PM_WAKEUP_SOURCE_TIMER_HEARTBEAT && WakeupSrc != PM_WAKEUP_SOURCE_TIMER_WEEK
            && WakeupSrc != PM_WAKEUP_SOURCE_OTA && WakeupSrc != PM_WAKEUP_SOURCE_DTC)
        {
            PmWorkAllModules();
        }
	}
	else
	{
		if(WakeupSrc == PM_WAKEUP_SOURCE_TIMER)
		{
            LOG_Debug("inter %s : wakeupsrc : %hhu,WakeupOption : %hhu",__FUNCTION__,WakeupSrc,WakeupOption);
			PmIpcSendPmState(WorkMode, PM_STATE_NORMAL, WakeupSrc, WakeupOption);
        }
        if(WakeupSrc == PM_WAKEUP_SOURCE_SMS)
        {
            LOG_Debug("%s : PM_WAKEUP_SOURCE_SMS wake up in PM_STATE_NORMAL",__FUNCTION__);
            PmSchedWakeUpMcu();
        }
			
	}

    HalModemForbidInd(0);
	pthread_mutex_unlock(&g_WakeUpSystemMutex);
    LOG_Debug("inter %s : wakeupsrc : %hhu,WakeupOption : %hhu",__FUNCTION__,WakeupSrc,WakeupOption);

    CleanPmSleepEvent("PmModemWakeup");
	return RET_OK;
}
static int SaveSleepLog(void)
{
    const char* command = "awk '$6!= 0 {print $1\" \"$6}' /sys/kernel/debug/wakeup_sources";

    static time_t last_log_time = 0;
    time_t current_time = time(NULL);

    
    if (difftime(current_time, last_log_time) < 10)
    {
        return RET_OK;
    }

    
    last_log_time = current_time;

    FILE* fp = popen(command, "r");
    if (fp == NULL)
    {
        LOG_Error("%s popen failed", __FUNCTION__);
        return RET_ERR;
    }

    char buffer[512] = {0};
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        LOG_Debug("%s", buffer);
    }

    pclose(fp);
    return RET_OK;
}

void PmModemSleep(void)
{   

	uint8_t PmState = PmSchedGetPmState();
	if(PmState == PM_STATE_SLEEP)
	{
		uint8_t Ret = PmSleepEventListIsEmpty();
		if(Ret == 1)
		{
            pthread_mutex_lock(&g_WakeUpSystemMutex);
            PmTboxSleepSetModules();
            pthread_mutex_unlock(&g_WakeUpSystemMutex);
            // PmSchedSetWakeupSrc(PM_WAKEUP_SOURCE_INVALID);
            //HSM 休眠
            //HsmEnterSleepMode();
            LOG_Info("%s sleep event is not exist,into sleep.",__FUNCTION__);
            PmState = PmSchedGetPmState();
            if(PmState == PM_STATE_SLEEP)
            {
                HalModemForbidInd(0xC3);
                PmModemUnLock();
            }
            
            SaveSleepLog();
		}
        else
        {
            LOG_Info("%s sleep event is exist,can not sleep.",__FUNCTION__);
        }
	}

}

