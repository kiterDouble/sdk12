#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "pm_modem.h"
#include "pm_modem_reboot.h"
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



#define PM_SCHED_NORMAL 1
#define PM_SCHED_SLEEP 2
#define SLEEP_NORMAL 1
#define SLEEP_QUICKLY 2

static uint8_t g_WakeupSrc = PM_WAKEUP_SOURCE_INVALID;
static uint8_t g_pmSleepFlag = SLEEP_NORMAL;
static pthread_rwlock_t g_WakeupSrcMutex = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t g_pmSleepFlagRwMutex = PTHREAD_RWLOCK_INITIALIZER;

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

static pthread_mutex_t g_WakeUpSystemMutex = PTHREAD_MUTEX_INITIALIZER;

void PmModemCallCallBack(const uint8_t *data, const uint16_t len, void* userdata)
{
	if (GetDialWakeMode())
	{
		PmModemWakeup(PM_WAKEUP_SOURCE_RING,CPU_INVALID_EVENT_ID);
	}
}

void PmModemSmsCallBack(const uint8_t *data, const uint16_t len, void* userdata)
{
	LOG_Debug("PmModemSmsCallBack");
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
	if(g_WakeLockFdFlag == 0)
	{
		HalModemPmWakeLockLocked((uint8_t*)PMWAKELOCK);
		g_WakeLockFdFlag = 1;
	}
}

static void PmModemUnLock(void)
{
	if(g_WakeLockFdFlag == 1)
	{
		HalModemPmWakeLockUnlocked((uint8_t*)PMWAKELOCK);
		g_WakeLockFdFlag = 0;
	}
}

int8_t PmModemWakeup(uint8_t WakeupSrc, uint8_t WakeupOption)
{
    SetPmSleepEvent("PmModemWakeup");
    LOG_Debug("inter %s : wakeupsrc : %hhu,WakeupOption : %hhu",__FUNCTION__,WakeupSrc,WakeupOption);
	pthread_mutex_lock(&g_WakeUpSystemMutex);
    
    PmModemLock();
    
    PmSchedSetWakeupSrc(WakeupSrc);

	uint8_t WorkMode = PmSchedGetWorkMode();
	uint8_t PmState = PmSchedGetPmState();

	if(PmState == PM_STATE_SLEEP)
	{
        LOG_Debug("cur sleep mode is Sleep,and then wakeup system ,and wakeup source is %d.",WakeupSrc);
		
		if(WakeupSrc == PM_WAKEUP_SOURCE_TIMER_HEARTBEAT)
		{
			PmIpcSendPmState(WorkMode, PM_STATE_SLEEP, WakeupSrc, WakeupOption);
			SetPmSleepEvent("keepWakeup1");
			usleep(3*1000*1000);
			CleanPmSleepEvent("keepWakeup1");
		}
		else if(WakeupSrc == PM_WAKEUP_SOURCE_TIMER_REBOOT)
		{
            /** 自重启定时器触发，MCU同步唤醒源后会重启MPU和MCU **/
            LOG_Debug("inter %s : wakeupsrc(TIMER_REBOOT) : %hhu,WakeupOption : %hhu",__FUNCTION__,WakeupSrc,WakeupOption);
            PmSchedSetPmState(PM_STATE_NORMAL);
			/** 不广播唤醒事件，很快就会重启，避免唤醒其他服务和应用**/
			// PmIpcSendPmState(WorkMode, PM_STATE_NORMAL, WakeupSrc, WakeupOption);
        }
		else
		{
            PmSchedSetPmState(PM_STATE_NORMAL);
            if(WakeupSrc == PM_WAKEUP_SOURCE_TIMER_WEEK || WakeupSrc == PM_WAKEUP_SOURCE_TIMER || WakeupSrc == PM_WAKEUP_SOURCE_TIMER_MCU)
            {
                LOG_Debug("inter %s : wakeupsrc : %hhu,WakeupOption : %hhu",__FUNCTION__,WakeupSrc,WakeupOption);
                PmIpcSendPmState(WorkMode, PM_STATE_NORMAL, WakeupSrc, WakeupOption);
            }
            else
            {
                PmIpcSendPmState(WorkMode, PM_STATE_NORMAL, WakeupSrc, CPU_INVALID_EVENT_ID);
            }
		}
		
		if(WakeupSrc == PM_WAKEUP_SOURCE_SMS || WakeupSrc == PM_WAKEUP_SOURCE_RING ||
			WakeupSrc == PM_WAKEUP_SOURCE_TIMER_REBOOT ||
			WakeupSrc == PM_WAKEUP_SOURCE_ECALL_INCOME || WakeupSrc == PM_WAKEUP_SOURCE_DATA || WakeupSrc == PM_WAKEUP_SOURCE_TIMER_MCU)
		{
			PmSchedWakeUpMcu();
        }

        if(WakeupSrc != PM_WAKEUP_SOURCE_TIMER && WakeupSrc != PM_WAKEUP_SOURCE_DATA 
			&& WakeupSrc != PM_WAKEUP_SOURCE_TIMER_REBOOT
            && WakeupSrc != PM_WAKEUP_SOURCE_TIMER_HEARTBEAT && WakeupSrc != PM_WAKEUP_SOURCE_TIMER_WEEK
            && WakeupSrc != PM_WAKEUP_SOURCE_OTA && WakeupSrc != PM_WAKEUP_SOURCE_DTC)
        {
            PmWorkAllModules();
        }
	}
	else
	{
		if(WakeupSrc == PM_WAKEUP_SOURCE_TIMER || WakeupSrc == PM_WAKEUP_SOURCE_TIMER_MCU)
		{
            LOG_Debug("inter %s : wakeupsrc : %hhu,WakeupOption : %hhu",__FUNCTION__,WakeupSrc,WakeupOption);
			PmIpcSendPmState(WorkMode, PM_STATE_NORMAL, WakeupSrc, WakeupOption);
        }
        if(WakeupSrc == PM_WAKEUP_SOURCE_SMS || WakeupSrc == PM_WAKEUP_SOURCE_DATA || WakeupSrc == PM_WAKEUP_SOURCE_TIMER_MCU)
        {
            LOG_Debug("%s : %d wake up in PM_STATE_NORMAL",__FUNCTION__, WakeupSrc);
            if (PmMcuHeartbeatIsAlive() == FALSE)
            {
                PmSchedWakeUpMcu();
            }
            else
            {
                LOG_Debug("%s : %d MCU already awake, skip WakeUpMcu", __FUNCTION__, WakeupSrc);
            }
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
    const char* command = "awk '$7!= 0 {print $1\" \"$7}' /sys/kernel/debug/wakeup_sources";

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

void PmModemSleepQuicklyFlagSet(void)
{
	pthread_rwlock_wrlock(&g_pmSleepFlagRwMutex);
	g_pmSleepFlag = SLEEP_QUICKLY;
	pthread_rwlock_unlock(&g_pmSleepFlagRwMutex);
}

void PmModemSleepQuicklyFlagClear(void)
{
	pthread_rwlock_wrlock(&g_pmSleepFlagRwMutex);
	g_pmSleepFlag = SLEEP_NORMAL;
	pthread_rwlock_unlock(&g_pmSleepFlagRwMutex);
}

uint8_t PmModemSleepQuicklyFlagGet(void)
{
	uint8_t flag = SLEEP_NORMAL;

	pthread_rwlock_wrlock(&g_pmSleepFlagRwMutex);
	flag = g_pmSleepFlag;
	pthread_rwlock_unlock(&g_pmSleepFlagRwMutex);

	return flag;
}

void PmModemSleep(void)
{
	static uint32_t blockedLogCount = 0U;
	static uint32_t meetRebootConditionCount = 0U;
	pthread_mutex_lock(&g_WakeUpSystemMutex);
	uint8_t PmState = PmSchedGetPmState();
	if(PmState == PM_STATE_SLEEP)
	{
		uint8_t ret = 1; // 1:无休眠锁 -1：休眠锁
		if (PmModemSleepQuicklyFlagGet() == SLEEP_QUICKLY)
		{
			ret = 1;
		}
		else
		{
			ret = PmSleepEventListIsEmpty();
		}

		if(ret == 1)
		{
			blockedLogCount = 0U;
            PmTboxSleepSetModules();
            PmSchedSetWakeupSrc(PM_WAKEUP_SOURCE_INVALID);
            LOG_Info("%s sleep event is not exist,into sleep.",__FUNCTION__);
            if (PmModemRebootCondCheck() == 1)
            {
				if ((++meetRebootConditionCount) > 10){
					/** 满足重启条件，本次进入休眠执行重启, 加上10次判定，避免定时重启rtc唤醒刚触发，还未和MCU同步唤醒源，就立即执行了重启 **/
					meetRebootConditionCount = 0;
					(void) PmModemReboot("autoTimerReboot", 0x01);
					sleep(5);
				}
            }
            else
            {
				HalModemForbidInd(0xC3);
				PmModemUnLock();
				SaveSleepLog();
            }
		}
        else
        {
			/* 调度周期为 500ms，每 10 次（约 5s）打印一次完整阻塞列表。 */
			if ((blockedLogCount++ % 10U) == 0U)
			{
				LOG_Info("%s sleep event is exist,can not sleep.",__FUNCTION__);
				PrintAllExistSleepEvent();
			}
		}
	}
	else
	{
		meetRebootConditionCount = 0;
		blockedLogCount = 0U;
	}
	pthread_mutex_unlock(&g_WakeUpSystemMutex);
}

