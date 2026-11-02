#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <glib.h>
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
#include "pm_timer.h"
#include "hal_modem_pm.h"

#define WAKEUP_MCU_TIMEOUT 10
#define WAKEUP_MCU_MAX_COUNT 6

/*wakeup mcu timer*/
static GTimer *g_wakeUpMcuTimer = NULL;
static gint g_wakeUpMcuCount = 0;

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
    snprintf(gpioCmdBuffer, sizeof(gpioCmdBuffer), "echo 1 > /sys/class/gpio/gpio%u/value", mcuWakePin);
    status = system(gpioCmdBuffer);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.", __FUNCTION__, gpioCmdBuffer);
    } else {
        LOG_Debug("%s : exec cmd[ %s ] failed.", __FUNCTION__, gpioCmdBuffer);
    }

    usleep(500000);
    
    memset(gpioCmdBuffer, 0, sizeof(gpioCmdBuffer));
    snprintf(gpioCmdBuffer, sizeof(gpioCmdBuffer), "echo 0 > /sys/class/gpio/gpio%u/value", mcuWakePin);
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
    g_timer_destroy(g_wakeUpMcuTimer);
    g_wakeUpMcuTimer = NULL;
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
gpointer CheckWakeUpMcuTimeoutThread(gpointer arg)
{
    while(1)
    {
        CheckWakeUpMcuTimeout();
        usleep(1*1000*1000);
    }
}

void PmMcuReqMpuSleepCallBack(const uint8_t *data, const uint16_t len, void* userdata)
{
	uint8_t mcuBuf[2] = {0};

    if(sizeof(mcuBuf) != len)
    {
        LOG_Error("%s recv mcu data leng is wrong.",__FUNCTION__);
        return;
    }
    else
    {
        memcpy(mcuBuf,data,len);
    }

    LOG_Debug("%s - index : %d,sleepmode : %d",__FUNCTION__,mcuBuf[0],mcuBuf[1]);
    if ( (RET_OK == PmSleepEventIsExist( PM_SLEEP_EVENT_UPGRADE )) ||  (RET_OK == PmSleepEventIsExist( PM_SLEEP_EVENT_ECALL )) )
    {
        LOG_Notice("MCU requecte cpu sleep but cpu is updating or ecalling");
        return;
    }
    
    McuMesg_t mcuMesg = {0};
    mcuMesg.type = SYS_QUE_TYPE_MCU_SYNC_POWER_SLEEP_REQ_RES;
    mcuMesg.payload = mcuBuf;
    mcuMesg.payloadLen = sizeof(mcuBuf);
	McuSrvSendMesg(&mcuMesg);

    //休眠模式
	if(mcuBuf[1] == 0)
	{
        uint8_t WorkMode = PmSchedGetWorkMode();
        PmIpcSendPmState(WorkMode, PM_STATE_SLEEP, PM_WAKEUP_SOURCE_INVALID, CPU_INVALID_EVENT_ID);
        usleep(1*1000*1000);
		PmSchedSetPmState(PM_STATE_SLEEP);
	}
    //深度休眠模式
	else if(mcuBuf[1] == 1)
	{
        HalModemPmDeepSleepPowerDown(0);
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
    if(37==mcuBuf[0])
	{
		mcuBuf[2] = 0x00;
		mcuBuf[3] = 0x00;		
		McuMesg_t mcuMesg = {0};
		mcuMesg.type = SYS_QUE_TYPE_MCU_SYNC_POWER_WAKEUPSOURCE_REQ_RES;
		mcuMesg.payload = mcuBuf;
		mcuMesg.payloadLen = sizeof(mcuBuf);
		McuSrvSendMesg(&mcuMesg);
		uint8_t PmState = PmSchedGetPmState();
		if((PmState == PM_STATE_SLEEP))
		{
			LOG_Debug("Cur PowerState is Sleep,and Get MCU Notify wakeup source,then wakeUpSystem");
			PmModemWakeup(mcuBuf[0],CPU_INVALID_EVENT_ID);
		}		
	}
	else
	{
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
}

void PmMcuSyncPowerMode(const uint8_t *data, const uint16_t len, void* userdata)
{
	uint8_t mcuBuf[4] = {0};

    LOG_DebugHex(data, len, "PmStatus recv mcu");
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

    LOG_Debug("from MCU - workmode : %d ,sleepmode : %d",mcuBuf[0],mcuBuf[1]);
	uint8_t WorkMode = PmSchedGetWorkMode();
    uint8_t PmState = PmSchedGetPmState();
    PmIpcSendPmState(WorkMode, PmState, PM_WAKEUP_SOURCE_INVALID, CPU_INVALID_EVENT_ID);

}

gpointer PmSyncPowerBlockEventToMcu(gpointer arg)
{
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
            
            LOG_Debug("to MCU - type : %x ,Counter : %d blockEvent : %d ",mcuMesg.type,mcuBuf[0],mcuBuf[1]);
        }
        else
        {
            LOG_Debug("%s : go to sleep.",__FUNCTION__);
        }
        
        usleep(1*1000*1000);
    }
    
    return NULL;
}


