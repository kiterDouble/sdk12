#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <pthread.h>
#include <unistd.h>
#include "pm_event.h"
#include "err_code.h"
#include "sr_log.h"
#include "pm_server_api.h"
#include "pm_modem.h"
#include "pm_modem_reboot.h"

#define EVENTLEN    (32)

static int8_t g_updateStateOld = RET_ERR;
static GTimer *g_updateTimer = NULL;
#define MAX_UPDATE_TIME (1800) //30min

static int8_t g_ecallStateOld = RET_ERR;
static GTimer *g_ecallTimer = NULL;
#define MAX_ECALL_TIME (3600) //1hour

typedef struct
{
	char Event[EVENTLEN + 1];
} PmSleepEvent_t;

static GList *g_EventList = NULL;
static pthread_rwlock_t  g_EventListMutex = PTHREAD_RWLOCK_INITIALIZER;
static uint8_t g_PmState = PM_STATE_NORMAL;
static pthread_rwlock_t g_PmStateMutex = PTHREAD_RWLOCK_INITIALIZER;

uint8_t PmSchedGetPmState(void)
{
	pthread_rwlock_rdlock(&g_PmStateMutex);
	uint8_t PmState = g_PmState;
	pthread_rwlock_unlock(&g_PmStateMutex);
	
	return PmState;
}

void PmSchedSetPmState(uint8_t PmState)
{
	pthread_rwlock_wrlock(&g_PmStateMutex);
	if(g_PmState != PmState)
	{
		g_PmState = PmState;
		/* 进入休眠即开启新的休眠唤醒周期，清除非强制重启额度标志 */
		if(PmState == PM_STATE_SLEEP)
		{
			PmModemRebootCountClear();
		}
	}
	pthread_rwlock_unlock(&g_PmStateMutex);
}


int8_t PmSleepEventListIsEmpty(void)
{
    pthread_rwlock_rdlock(&g_EventListMutex);
    int8_t ret = (g_EventList == NULL ? 1 : -1);
    pthread_rwlock_unlock(&g_EventListMutex);
	
    return ret;
}

int8_t PmSleepEventIsExist(char *Event)
{
	if(NULL == Event)
	{
        LOG_Error("%s parameter event is NULL.",__FUNCTION__);
		return RET_ERR;
    }
		
	uint32_t EventLen = strlen(Event);
	if((EventLen == 0U) || (EventLen > EVENTLEN))
	{
		LOG_Error("%s parameter event length is invalid: %u.", __FUNCTION__, EventLen);
		return RET_ERR;
	}
		
	pthread_rwlock_rdlock(&g_EventListMutex);
	
    for(GList *iter = g_EventList; iter != NULL; iter = iter->next)
	{
		PmSleepEvent_t *PmSleepEvent = (PmSleepEvent_t *)iter->data;
		if(PmSleepEvent)
		{
			if(memcmp(PmSleepEvent->Event, Event, EventLen) == 0)
			{
				pthread_rwlock_unlock(&g_EventListMutex);
				return RET_OK;
			}
		}
    }
	
	pthread_rwlock_unlock(&g_EventListMutex);
	
	return RET_ERR;
}

void PrintAllExistSleepEvent()
{
    pthread_rwlock_rdlock(&g_EventListMutex);
    for(GList *iter = g_EventList; iter != NULL; iter = iter->next)
    {
        PmSleepEvent_t *PmSleepEvent = (PmSleepEvent_t *)iter->data;
        if(PmSleepEvent)
        {
            LOG_Debug("PmSleepEvent: %s", PmSleepEvent->Event);
        }
    }
    pthread_rwlock_unlock(&g_EventListMutex);
}

static int8_t AddPmSleepEvent(char *Event)
{
    
	if(NULL == Event)
	{
        LOG_Error("%s parameter event is NULL.",__FUNCTION__);
		return RET_ERR;
    }
    
	PmSleepEvent_t *PmSleepEvent = g_try_malloc0(sizeof(PmSleepEvent_t));
	if(NULL == PmSleepEvent)
	{
        LOG_Error("%s malloc is faild .",__FUNCTION__);
		return RET_ERR;
    }
		
	memcpy(PmSleepEvent->Event, Event, strlen(Event));
    
	g_EventList = g_list_append(g_EventList, PmSleepEvent);
	
	return RET_OK;
}

int8_t SetPmSleepEvent(char *Event)
{
	if(NULL == Event)
	{
        LOG_Error("%s parameter event is NULL.",__FUNCTION__);
		return RET_ERR;
    }
		
	uint32_t EventLen = strlen(Event);
	if((EventLen == 0U) || (EventLen > EVENTLEN))
	{
		LOG_Error("%s parameter event length is invalid: %u.", __FUNCTION__, EventLen);
		return RET_ERR;
	}

	pthread_rwlock_wrlock(&g_EventListMutex);
    
    for(GList *iter = g_EventList; iter != NULL; iter = iter->next)
	{
		PmSleepEvent_t *PmSleepEvent = (PmSleepEvent_t *)iter->data;
		if(PmSleepEvent)
		{
			if(memcmp(PmSleepEvent->Event, Event, EventLen) == 0)
			{
				pthread_rwlock_unlock(&g_EventListMutex);
				return RET_OK;
			}
		}
    }
	int8_t ret = AddPmSleepEvent(Event);
    
	pthread_rwlock_unlock(&g_EventListMutex);
	return ret;
}

static void FreePmSleepEvent(PmSleepEvent_t *PmSleepEvent)
{
    if(PmSleepEvent)
        g_free(PmSleepEvent);
}

int8_t CleanPmSleepEvent(char *Event)
{
	if(NULL == Event)
	{
        LOG_Error("%s parameter event is NULL.",__FUNCTION__);
		return RET_ERR;
    }
		
	uint32_t EventLen = strlen(Event);
	if((EventLen == 0U) || (EventLen > EVENTLEN))
	{
		LOG_Error("%s parameter event length is invalid: %u.", __FUNCTION__, EventLen);
		return RET_ERR;
	}
		
	pthread_rwlock_wrlock(&g_EventListMutex);
	
    for(GList *iter = g_EventList; iter != NULL; iter = iter->next)
	{
		PmSleepEvent_t *PmSleepEvent = (PmSleepEvent_t *)iter->data;
		if(PmSleepEvent)
		{
			if(memcmp(PmSleepEvent->Event, Event, EventLen) == 0)
			{
				g_EventList = g_list_remove_link(g_EventList, iter);
				g_list_free_full(iter, (GDestroyNotify)FreePmSleepEvent);
				pthread_rwlock_unlock(&g_EventListMutex);
				return RET_OK;
			}
		}
    }
	
	pthread_rwlock_unlock(&g_EventListMutex);
	
	return RET_OK;
}

gpointer KeepWakeup()
{
    LOG_Debug("set KeepWakeup event");
    SetPmSleepEvent("keepWakeup");
    sleep(20);
    LOG_Debug("clean KeepWakeup event");
    CleanPmSleepEvent("keepWakeup");

    return NULL;
}

gboolean MonitorUpdateTime()
{  
    gdouble elapsedTime = 0;
    int8_t g_updateStateNow = RET_ERR; 

    g_updateStateNow =  PmSleepEventIsExist( PM_SLEEP_EVENT_UPGRADE );
    
    /* form no update to update*/
    if ( ( g_updateStateNow == RET_OK ) && ( g_updateStateOld == RET_ERR ) )
    {
        /*start timer of monitering update timer*/    
        if ( g_updateTimer == NULL )
        {
            g_updateTimer = g_timer_new();
            LOG_Debug("start timer for checking update time");
        }
        else
        {
            g_timer_destroy( g_updateTimer );
            g_updateTimer = g_timer_new();
            LOG_Debug("destroy old timer and start timer for checking update time");
        }
    }

    /* form update to no  update*/
    if ( ( g_updateStateNow == RET_ERR )  && ( g_updateStateOld == RET_OK) )
    {
        /* delet timer of monitering udate time*/
        if (g_updateTimer != NULL )
        {
            g_timer_destroy( g_updateTimer );
            g_updateTimer = NULL;
            LOG_Debug("destroy timer for checking update time");
        }
    }
    
    /*check update time*/
    if ( g_updateStateNow == RET_OK )
    {
        elapsedTime = g_timer_elapsed(g_updateTimer, NULL);
        
        if ( elapsedTime > MAX_UPDATE_TIME )
        {
            LOG_Debug("update timer > %d seconds, clean sleep event", MAX_UPDATE_TIME);
            CleanPmSleepEvent( PM_SLEEP_EVENT_UPGRADE );
            //system("sys_reboot");
        }
    }

    g_updateStateOld = g_updateStateNow; 
    
    return TRUE;
}


gboolean MonitorEcallTime()
{  
    gdouble elapsedTime = 0;
    int8_t g_ecallStateNow = RET_ERR; 

    g_ecallStateNow =  PmSleepEventIsExist( PM_SLEEP_EVENT_ECALL );
    
    /* form no update to update*/
    if ( ( g_ecallStateNow == RET_OK ) && ( g_ecallStateOld == RET_ERR ) )
    {
        /*start timer of monitering update timer*/    
        if ( g_ecallTimer == NULL )
        {
            g_ecallTimer = g_timer_new();
            LOG_Debug("start timer for checking ecall time");
        }
        else
        {
            g_timer_destroy( g_ecallTimer );
            g_ecallTimer = g_timer_new();
            LOG_Debug("destroy old timer and start timer for checking ecall time");
        }
    }

    /* form update to no  update*/
    if ( ( g_ecallStateNow == RET_ERR )  && ( g_ecallStateOld == RET_OK) )
    {
        /* delet timer of monitering udate time*/
        if (g_ecallTimer != NULL )
        {
            g_timer_destroy( g_ecallTimer );
            g_ecallTimer = NULL;
            LOG_Debug("destroy timer for checking ecall time");
        }
    }
    
    /*check update time*/
    if ( g_ecallStateNow == RET_OK )
    {
        elapsedTime = g_timer_elapsed(g_ecallTimer, NULL);
        
        if ( elapsedTime > MAX_ECALL_TIME )
        {
            LOG_Debug("ecall timer > %d seconds, clean sleep event", MAX_ECALL_TIME);
            CleanPmSleepEvent( PM_SLEEP_EVENT_ECALL );
            //system("sys_reboot");
        }
    }

    g_ecallStateOld = g_ecallStateNow; 
    
    return TRUE;
}

