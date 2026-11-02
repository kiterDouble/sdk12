#include <pthread.h>
#include <unistd.h>

#include "pm_server_api.h"
#include "pm_sched.h"
#include "err_code.h"
#include "sr_log.h"
#include "pm_mcu_control.h"
#include "pm_device_control.h"
#include "pm_modem.h"
#include "pm_modem_reboot.h"
#include "pm_io.h"
#include "pm_ipc_rpc.h"
#include "pm_event.h"
#include "pm_timer.h"
#include "sr_sdk.h"

#define PM_SCHED_NORMAL 1
#define PM_SCHED_SLEEP 2

static uint8_t g_WorkMode = PM_WORK_NORMAL;
static pthread_rwlock_t g_WorkModeMutex = PTHREAD_RWLOCK_INITIALIZER;

uint8_t PmSchedGetWorkMode(void)
{
	pthread_rwlock_rdlock(&g_WorkModeMutex);
	uint8_t WorkMode = g_WorkMode;
	pthread_rwlock_unlock(&g_WorkModeMutex);
	
	return WorkMode;
}

void PmSchedSetWorkMode(uint8_t WorkMode)
{
	pthread_rwlock_wrlock(&g_WorkModeMutex);
	g_WorkMode = WorkMode;
	pthread_rwlock_unlock(&g_WorkModeMutex);
}


gboolean PmDoSomethingToSleep(gpointer data)
{
    PmModemSleep();
    return G_SOURCE_CONTINUE;
}

void PmSchedInit(void)
{
    InitSRSdk();
	LoadPmConfig();
	LoadPmAutoRebootConfig();
	PmPowerIoInit();
	PmModemInit();
	PmWakeupIoInit();
	PmIpcInit();

	/* 自重启定时器初始化：按配置在启动时长达到阈值后触发，若处于休眠则唤醒重启 */
	PmModemAutoRebootInit();

	/* 增加监控MCU心跳 */
	PmInitMcuHeartbeatMonitor();

    g_timeout_add(500, PmDoSomethingToSleep, NULL);
    g_timeout_add_seconds(2, MonitorUpdateTime , NULL);
    g_timeout_add_seconds(10, MonitorEcallTime , NULL);
    g_thread_new("keep wakeup", KeepWakeup, NULL);
    usleep(20*1000*1000);
    g_thread_new("WakeupMcuTimeout",CheckWakeUpMcuTimeoutThread,NULL);
    g_thread_new("SyncPowerEvent", PmSyncPowerBlockEventToMcu, NULL);
}

void PmSchedExit(void)
{
	PmIpcExit();
	PmModemExit();
	PmPowerIoExit();
	FreePmConfig();
}

