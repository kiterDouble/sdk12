#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <pthread.h>

#include "err_code.h"
#include "pm_server_api.h"
#include "sr_ipc.h"
#include "sr_log.h"

#define EVENTLEN 32
#define OPTION_OK                               (11)
#define OPTION_ERROR                            (12)
#define CREATE_SLEEP_EVENT                      (1)
#define DELETE_SLEEP_EVENT                      (2)
#define CREATE_PM_SLEEP_ALARM                   (1)
#define CREATE_PM_SLEEP_ALARM_EVERYDAY          (2)
#define DESTORY_PM_SLEEP_ALARM                  (3)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID1  (0x7001)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID2  (0x7002)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID3  (0x7003)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID4  (0x7004)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID5  (0x7005)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID6  (0x7006)
#define RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID7  (0x7007)

#define IPC_RECVBUF_SIZE                        (512)

typedef struct
{
    uint8_t data;
} WakeupSrcIpc_t;

typedef struct
{
    uint8_t SleepAlarmEventId;
} PmSleepAlarm_t;

typedef struct
{
    uint8_t Cmd;
    uint8_t SleepAlarmEventId;
    uint8_t SleepAlarmHour;
    uint8_t SleepAlarmMinute;
    uint32_t SleepAlarmSecond;
    uint16_t WakeupOption;
} PmSleepAlarmIpc_t;

typedef struct
{
    char Event[EVENTLEN + 1];
} PmSleepEvent_t;

typedef struct
{
    uint8_t Cmd;
    char Event[EVENTLEN + 1];
} PmSleepEventIpc_t;

typedef struct
{
    uint8_t mode;
    char name[EVENTLEN + 1];
} PmWakeDevicesIpc_t;

static GList *g_TimerList = NULL;
static pthread_mutex_t g_TimerListMutex = PTHREAD_MUTEX_INITIALIZER;
static GList *g_EventList = NULL;
static pthread_mutex_t g_EventListMutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * @brief 获取唤醒源
 *
 * @return uint8_t 唤醒源，取值范围：0-39
 */
uint8_t PmGetWakeupSrc(void)
{
    LOG_Debug("%s : entence.",__FUNCTION__);
    WakeupSrcIpc_t WakeupSrcIpc;
    memset(&WakeupSrcIpc, 0x00, sizeof(WakeupSrcIpc));
    WakeupSrcIpc.data = 1;

    if((sizeof(WakeupSrcIpc_t) + sizeof(PmSleepAlarmIpc_t) + sizeof(PmSleepEventIpc_t)) > IPC_RECVBUF_SIZE)
    {
        LOG_Error("%s : IPC_RECVBUF_SIZE is not enough.",__FUNCTION__);
        return PM_WAKEUP_SOURCE_INVALID;
    }
    uint8_t recvdata[IPC_RECVBUF_SIZE] = "";
    memset(recvdata, 0x00, sizeof(recvdata));
    uint16_t recvlen = 0;

    RpcGetMsgSync(RPCSERVER_TYPE_PMSERVER, RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID1, 0, (uint8_t *)&WakeupSrcIpc, sizeof(WakeupSrcIpc), recvdata, &recvlen);
    if(recvlen == sizeof(WakeupSrcIpc_t))
    {
        memset(&WakeupSrcIpc, 0x00, sizeof(WakeupSrcIpc_t));
        memcpy(&WakeupSrcIpc, recvdata, recvlen);
    }
    else
    {
        LOG_Error("%s : RpcGetMsgSync is failed.",__FUNCTION__);
        return PM_WAKEUP_SOURCE_INVALID;
    }

    LOG_Debug("%s : RpcGetMsgSync is success, wakeupsrc is %d .",__FUNCTION__,WakeupSrcIpc.data);
    return WakeupSrcIpc.data;
}
static int8_t AddPmSleepAlarmEventId(uint8_t SleepAlarmEventId)
{
    PmSleepAlarm_t *PmSleepAlarm = g_try_malloc0(sizeof(PmSleepAlarm_t));
    if(!PmSleepAlarm)
    {
        LOG_Error("%s malloc is faild.",__FUNCTION__);
        return RET_ERR;
    }
        
    PmSleepAlarm->SleepAlarmEventId = SleepAlarmEventId;   
    g_TimerList = g_list_append(g_TimerList, PmSleepAlarm);    

    return RET_OK;
}

/**
 * @brief 设置休眠定时器，当定时器超时，唤醒 MPU
 *
 * @param SleepAlarmEventId uint8_t 定时器类型
 * @param SleepAlarmSecond uint32_t 定时器超时时间，取值范围：1-86400，1到86400秒
 * @param WakeupOption uint16_t 唤醒选项
 * @return int8_t 数据发送结果，取值：RET_ERR 失败，RET_OK 成功
 */

int8_t PmCreateSleepAlarm(uint8_t SleepAlarmEventId, uint32_t SleepAlarmSecond, uint16_t WakeupOption)
{
    int8_t ret = RET_ERR;
    
    pthread_mutex_lock(&g_TimerListMutex);    
    for(GList *iter = g_TimerList; iter != NULL; iter = iter->next)
    {
        PmSleepAlarm_t *PmSleepAlarm = (PmSleepAlarm_t *)iter->data;
        if(PmSleepAlarm)
        {
            if(PmSleepAlarm->SleepAlarmEventId == SleepAlarmEventId)
            {
                LOG_Notice("%s , SleepAlarmEventId %d is exist ",__FUNCTION__,SleepAlarmEventId);
                pthread_mutex_unlock(&g_TimerListMutex);
                return RET_ERR;
            }
        }
    }
    
    PmSleepAlarmIpc_t PmSleepAlarmIpc;
    memset(&PmSleepAlarmIpc, 0x00, sizeof(PmSleepAlarmIpc_t));
    PmSleepAlarmIpc.Cmd = CREATE_PM_SLEEP_ALARM;
    PmSleepAlarmIpc.SleepAlarmEventId = SleepAlarmEventId;
    PmSleepAlarmIpc.SleepAlarmSecond = SleepAlarmSecond;
    PmSleepAlarmIpc.WakeupOption = WakeupOption;
    
    if((sizeof(WakeupSrcIpc_t) + sizeof(PmSleepAlarmIpc_t) + sizeof(PmSleepEventIpc_t)) > IPC_RECVBUF_SIZE)
    {
        LOG_Error("%s IPC_RECVBUF_SIZE is not enough.",__FUNCTION__);
        pthread_mutex_unlock(&g_TimerListMutex);
        return ret;
    }
    uint8_t recvdata[IPC_RECVBUF_SIZE] = "";
    memset(recvdata, 0x00, sizeof(recvdata));
    uint16_t recvlen = 0;

    RpcGetMsgSync(RPCSERVER_TYPE_PMSERVER, RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID2, 0, (uint8_t *)&PmSleepAlarmIpc, sizeof(PmSleepAlarmIpc), recvdata, &recvlen);
    if(recvlen == sizeof(PmSleepAlarmIpc_t))
    {
        memset(&PmSleepAlarmIpc, 0x00, sizeof(PmSleepAlarmIpc_t));
        memcpy(&PmSleepAlarmIpc, recvdata, recvlen);
        if(PmSleepAlarmIpc.Cmd == OPTION_OK)
        {
            ret = AddPmSleepAlarmEventId(SleepAlarmEventId);
        }
    }
    
    pthread_mutex_unlock(&g_TimerListMutex);
    LOG_Info("%s is finish, SleepAlarmEventId %d add.",__FUNCTION__,SleepAlarmEventId);
    return ret;

}

/**
 * @brief 设置每日休眠定时器，当定时器超时，唤醒 MPU
 *
 * @param SleepAlarmEventId uint8_t 定时器类型
 * @param SleepAlarmHour uint8_t 定时器超时小时，取值范围：0-23，0到23小时
 * @param SleepAlarmMinute uint8_t 定时器超时分钟，取值范围：0-59，0到59分钟
 * @param WakeupOption uint16_t 唤醒选项
 * @return int8_t 数据发送结果，取值：RET_ERR 失败，RET_OK 成功
 */

int8_t PmCreateSleepAlarmEveryDay(uint8_t SleepAlarmEventId, uint8_t SleepAlarmHour, uint8_t SleepAlarmMinute, uint16_t WakeupOption)
{
    int8_t ret = RET_ERR;
    
    pthread_mutex_lock(&g_TimerListMutex);
    for(GList *iter = g_TimerList; iter != NULL; iter = iter->next)
    {
        PmSleepAlarm_t *PmSleepAlarm = (PmSleepAlarm_t *)iter->data;
        if(PmSleepAlarm)
        {
            if(PmSleepAlarm->SleepAlarmEventId == SleepAlarmEventId)
            {
                LOG_Notice("%s , SleepAlarmEventId %d is exist ",__FUNCTION__,SleepAlarmEventId);
                pthread_mutex_unlock(&g_TimerListMutex);
                return ret;
            }
        }
    }
    
    PmSleepAlarmIpc_t PmSleepAlarmIpc;
    memset(&PmSleepAlarmIpc, 0x00, sizeof(PmSleepAlarmIpc_t));
    PmSleepAlarmIpc.Cmd = CREATE_PM_SLEEP_ALARM_EVERYDAY;
    PmSleepAlarmIpc.SleepAlarmEventId = SleepAlarmEventId;
    PmSleepAlarmIpc.SleepAlarmHour = SleepAlarmHour;
    PmSleepAlarmIpc.SleepAlarmMinute = SleepAlarmMinute;
    PmSleepAlarmIpc.WakeupOption = WakeupOption;
    
    if((sizeof(WakeupSrcIpc_t) + sizeof(PmSleepAlarmIpc_t) + sizeof(PmSleepEventIpc_t)) > IPC_RECVBUF_SIZE)
    {
        LOG_Error("%s IPC_RECVBUF_SIZE is not enough.",__FUNCTION__);
        pthread_mutex_unlock(&g_TimerListMutex);
        return ret;
    }
    uint8_t recvdata[IPC_RECVBUF_SIZE] = "";
    memset(recvdata, 0x00, sizeof(recvdata));
    uint16_t recvlen = 0;
    
    RpcGetMsgSync(RPCSERVER_TYPE_PMSERVER, RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID2, 0, (uint8_t *)&PmSleepAlarmIpc, sizeof(PmSleepAlarmIpc), recvdata, &recvlen);
    if(recvlen == sizeof(PmSleepAlarmIpc_t))
    {
        memset(&PmSleepAlarmIpc, 0x00, sizeof(PmSleepAlarmIpc_t));
        memcpy(&PmSleepAlarmIpc, recvdata, recvlen);
        if(PmSleepAlarmIpc.Cmd == OPTION_OK)
        {
            ret = AddPmSleepAlarmEventId(SleepAlarmEventId);
        }
    }
    pthread_mutex_unlock(&g_TimerListMutex);
    LOG_Info("%s is finish, SleepAlarmEventId %d add.",__FUNCTION__,SleepAlarmEventId);
    return ret;
}
static void FreePmSleepAlarm(PmSleepAlarm_t *PmSleepAlarm)
{
    if(PmSleepAlarm)
        g_free(PmSleepAlarm);

}

/**
 * @brief 清除休眠定时器
 *
 * @param SleepAlarmEventId uint8_t 定时器类型
 * @return int8_t 数据发送结果，取值：RET_ERR 失败，RET_OK 成功
 */

int8_t PmDestroySleepAlarm(uint8_t SleepAlarmEventId)
{
    int8_t ret = RET_ERR;
    
    pthread_mutex_lock(&g_TimerListMutex);
    
    for(GList *iter = g_TimerList; iter != NULL; iter = iter->next)
    {
        PmSleepAlarm_t *PmSleepAlarm = (PmSleepAlarm_t *)iter->data;
        if(PmSleepAlarm)
        {
            if(PmSleepAlarm->SleepAlarmEventId == SleepAlarmEventId)
            {
                PmSleepAlarmIpc_t PmSleepAlarmIpc;
                memset(&PmSleepAlarmIpc, 0x00, sizeof(PmSleepAlarmIpc_t));
                PmSleepAlarmIpc.Cmd = DESTORY_PM_SLEEP_ALARM;
                PmSleepAlarmIpc.SleepAlarmEventId = SleepAlarmEventId;
                
                if((sizeof(WakeupSrcIpc_t) + sizeof(PmSleepAlarmIpc_t) + sizeof(PmSleepEventIpc_t)) > IPC_RECVBUF_SIZE)
                {
                    LOG_Error("%s IPC_RECVBUF_SIZE is not enough.",__FUNCTION__);
                    pthread_mutex_unlock(&g_TimerListMutex);
                    return ret;
                }
                uint8_t recvdata[IPC_RECVBUF_SIZE] = "";
                memset(recvdata, 0x00, sizeof(recvdata));
                uint16_t recvlen = 0;
                
                RpcGetMsgSync(RPCSERVER_TYPE_PMSERVER, RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID2, 0, (uint8_t *)&PmSleepAlarmIpc, sizeof(PmSleepAlarmIpc), recvdata, &recvlen);
                if(recvlen == sizeof(PmSleepAlarmIpc_t))
                {
                    memset(&PmSleepAlarmIpc, 0x00, sizeof(PmSleepAlarmIpc_t));
                    memcpy(&PmSleepAlarmIpc, recvdata, recvlen);
                    if(PmSleepAlarmIpc.Cmd == OPTION_OK && PmSleepAlarmIpc.SleepAlarmEventId == SleepAlarmEventId)
                    {
                        g_TimerList = g_list_remove_link(g_TimerList, iter);
                        g_list_free_full(iter, (GDestroyNotify)FreePmSleepAlarm);
                        LOG_Debug("%s : RpcGetMsgSync is success.",__FUNCTION__);
                        ret = RET_OK;
                    }
                }
                
                break;
            }
        }
    }
    
    pthread_mutex_unlock(&g_TimerListMutex);
    
    return ret;
}
static int8_t AddPmSleepEvent(const char *Event)
{
    if(NULL == Event)
    {
        LOG_Error("%s parameter Event is NULL.",__FUNCTION__);
        return RET_ERR;
    }
    
    PmSleepEvent_t *PmSleepEvent = g_try_malloc0(sizeof(PmSleepEvent_t));
    if(!PmSleepEvent)
    {
        LOG_Error("%s malloc is faild.",__FUNCTION__);
        return RET_ERR;
    }

    if(strlen(Event) <= EVENTLEN)
    {
        memcpy(PmSleepEvent->Event, Event, strlen(Event));
        g_EventList = g_list_append(g_EventList, PmSleepEvent);
        return RET_OK;
    }
    
    LOG_Error("%s event length is more than 32.",__FUNCTION__);
    return RET_ERR;

}

/**
 * @brief 设置阻止休眠的事件，当有阻止休眠的事件时，MPU 无法休眠
 *
 * @param Event char* 事件名称，描述事件的字符串
 * @return int8_t 数据发送结果，取值：RET_ERR 失败，RET_OK 成功
 */

int8_t PmSetSleepEvent(const char *Event)
{
    int8_t ret = RET_ERR;
    
    if(NULL == Event)
    {
        LOG_Error("%s parameter Event is NULL.",__FUNCTION__);
        return ret;
    }
        
    uint32_t EventLen = strlen(Event);
    if(EventLen > EVENTLEN)
    {
        LOG_Error("%s event length is more than 32.",__FUNCTION__);
        return ret;
    }
        
    pthread_mutex_lock(&g_EventListMutex);
    
    for(GList *iter = g_EventList; iter != NULL; iter = iter->next)
    {
        PmSleepEvent_t *PmSleepEvent = (PmSleepEvent_t *)iter->data;
        if(PmSleepEvent)
        {
            if(memcmp(PmSleepEvent->Event, Event, EventLen) == 0)
            {
                LOG_Notice("%s , PmSleepEvent %s is exist ",__FUNCTION__,Event);
                pthread_mutex_unlock(&g_EventListMutex);
                return RET_ERR;
            }
        }
    }
    
    PmSleepEventIpc_t PmSleepEventIpc;
    memset(&PmSleepEventIpc, 0x00, sizeof(PmSleepEventIpc_t));
    PmSleepEventIpc.Cmd = CREATE_SLEEP_EVENT;
    memcpy(PmSleepEventIpc.Event, Event, EventLen);
        
    if((sizeof(WakeupSrcIpc_t) + sizeof(PmSleepAlarmIpc_t) + sizeof(PmSleepEventIpc_t)) > IPC_RECVBUF_SIZE)
    {
        LOG_Error("%s IPC_RECVBUF_SIZE is not enough.",__FUNCTION__);
        pthread_mutex_unlock(&g_EventListMutex);
        return ret;
    }
    uint8_t recvdata[IPC_RECVBUF_SIZE] = "";
    memset(recvdata, 0x00, sizeof(recvdata));
    uint16_t recvlen = 0;
    
    RpcGetMsgSync(RPCSERVER_TYPE_PMSERVER, RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID3, 0, (uint8_t *)&PmSleepEventIpc, sizeof(PmSleepEventIpc), recvdata, &recvlen);
    if(recvlen == sizeof(PmSleepEventIpc_t))
    {
        memset(&PmSleepEventIpc, 0x00, sizeof(PmSleepEventIpc_t));
        memcpy(&PmSleepEventIpc, recvdata, recvlen);
        if(PmSleepEventIpc.Cmd == OPTION_OK && memcmp(PmSleepEventIpc.Event, Event, EventLen) == 0)
        {
            ret = AddPmSleepEvent(Event);
            LOG_Debug("%s : RpcGetMsgSync is success.",__FUNCTION__);
        }
    }
    
    
    pthread_mutex_unlock(&g_EventListMutex);
    return ret;
}
static void FreePmSleepEvent(PmSleepEvent_t *PmSleepEvent)
{
    if(PmSleepEvent)
        g_free(PmSleepEvent);

}

/**
 * @brief 清除阻止休眠的事件，当没有阻止休眠的事件时，MPU 自动进入休眠
 *
 * @param Event char* 事件名称，描述事件的字符串
 */

int8_t PmCleanSleepEvent(const char *Event)
{
    int8_t ret = RET_ERR;
    
    if(NULL == Event)
    {
        LOG_Error("%s parameter Event is NULL.",__FUNCTION__);
        return ret;
    }
    
    uint32_t EventLen = strlen(Event);
    if(EventLen > EVENTLEN)
    {
        LOG_Error("%s event length is more than 32.",__FUNCTION__);
        return ret;
    }
        
    pthread_mutex_lock(&g_EventListMutex);
    
    for(GList *iter = g_EventList; iter != NULL; iter = iter->next)
    {
        PmSleepEvent_t *PmSleepEvent = (PmSleepEvent_t *)iter->data;
        if(PmSleepEvent)
        {
            if(memcmp(PmSleepEvent->Event, Event, EventLen) == 0)
            {
                PmSleepEventIpc_t PmSleepEventIpc;
                memset(&PmSleepEventIpc, 0x00, sizeof(PmSleepEventIpc_t));
                PmSleepEventIpc.Cmd = DELETE_SLEEP_EVENT;
                memcpy(PmSleepEventIpc.Event, Event, EventLen);
                
                if((sizeof(WakeupSrcIpc_t) + sizeof(PmSleepAlarmIpc_t) + sizeof(PmSleepEventIpc_t)) > IPC_RECVBUF_SIZE)
                {
                    LOG_Error("%s IPC_RECVBUF_SIZE is not enough.",__FUNCTION__);
                    pthread_mutex_unlock(&g_EventListMutex);
                    return ret;
                }
                uint8_t recvdata[IPC_RECVBUF_SIZE] = "";
                memset(recvdata, 0x00, sizeof(recvdata));
                uint16_t recvlen = 0;
                
                RpcGetMsgSync(RPCSERVER_TYPE_PMSERVER, RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID3, 0, (uint8_t *)&PmSleepEventIpc, sizeof(PmSleepEventIpc), recvdata, &recvlen);
                if(recvlen == sizeof(PmSleepEventIpc_t))
                {
                    memset(&PmSleepEventIpc, 0x00, sizeof(PmSleepEventIpc));
                    memcpy(&PmSleepEventIpc, recvdata, recvlen);
                    if(PmSleepEventIpc.Cmd == OPTION_OK && memcmp(PmSleepEventIpc.Event, Event, EventLen) == 0)
                    {
                        g_EventList = g_list_remove_link(g_EventList, iter);
                        g_list_free_full(iter, (GDestroyNotify)FreePmSleepEvent);
                        LOG_Debug("%s : RpcGetMsgSync is success.",__FUNCTION__);
                        ret = RET_OK;
                    }
                }
                
                break;
            }
        }
    }
    
    pthread_mutex_unlock(&g_EventListMutex);
    
    return ret;
}
/**
 * @brief 当配置文件pm_config.json中的sms_wake_mode字段配置为0时候，短信唤醒需要手动调用PmSmsSleepSet进行设置
 *
 * @param void
 * @return int8_t 接口调用结果，取值：RET_ERR 失败，RET_OK 成功
 */
int8_t PmSmsSleepSet(void)
{
    uint8_t recvdata = 0;
    uint16_t recvlen = 0;
    
    RpcGetMsgSync(RPCSERVER_TYPE_PMSERVER, RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID4, 0, NULL, 0, &recvdata, &recvlen);
    int8_t rcvData = recvdata;
    if(rcvData == sizeof(int8_t))
    {
       return rcvData;
    }
    else
    {
        LOG_Error("%s : RpcGetMsgSync is failed.",__FUNCTION__);
        return RET_ERR;
    }
}

/**
 * @brief 唤醒pm_config.json配置文件中modules列表中的相关外设，配置文件中外设name参数要与函数传参一致，否则找不到设备
 *
 * @param name 要唤醒外设的字符串名称，最大长度为32Bytes,如果要设置唤醒mcu则name传参字符串mcu，此时mode参数无效可以随便传入WAKE_MODE或是POWER_MODE。
 *        mode 需要设置外设的模式，WAKE_MODE为唤醒模式，POWER_MODE为上电模式，需要与例表中tbox_sleep字段设置相对应，否则找不到相应模式和管脚
 * @return int8_t 接口调用结果，取值：RET_ERR 失败，RET_OK 成功
 */
int8_t PmWakeupSetDevices(const char * name,uint8_t mode)
{
    uint8_t recvdata = 0;
    uint16_t recvlen = 0;

    if(NULL == name)
    {
        LOG_Error("%s parameter name is NULL.",__FUNCTION__);
        return RET_ERR;
    }

    if((WAKE_MODE != mode) && (POWER_MODE != mode))
    {
        LOG_Error("%s parameter mode is error.",__FUNCTION__);
        return RET_ERR;
    }
        
    uint32_t NameLen = strlen(name);
    if(NameLen > EVENTLEN)
    {
        LOG_Error("%s name length is more than 32.",__FUNCTION__);
        return RET_ERR;
    }

    
    PmWakeDevicesIpc_t PmWakeDevicesIpc;
    memset(&PmWakeDevicesIpc, 0x00, sizeof(PmWakeDevicesIpc_t));
    PmWakeDevicesIpc.mode = mode;
    memcpy(PmWakeDevicesIpc.name, name, NameLen);
    
    RpcGetMsgSync(RPCSERVER_TYPE_PMSERVER, RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID5, 0,(uint8_t *)&PmWakeDevicesIpc, sizeof(PmWakeDevicesIpc), &recvdata, &recvlen);
    if(recvlen == sizeof(uint8_t))
    {
       return recvdata;
    }
    else
    {
        LOG_Error("%s : RpcGetMsgSync is failed.",__FUNCTION__);
        return RET_ERR;
    }
}


/**
 * @brief 同步网络数据唤醒源并唤醒系统
 *
 * @param WakeupSrc 需要同步的唤醒源 当前为PM_WAKEUP_SOURCE_DATA,PM_WAKEUP_SOURCE_OTA,PM_WAKEUP_SOURCE_DTC唤醒源
 * @return int8_t 接口调用结果，取值：RET_ERR 失败，RET_OK 成功
 */
int8_t PmSyncDataWakeSourceAndWakeupSystem(uint8_t WakeupSrc)
{
    uint8_t recvdata = 0;
    uint16_t recvlen = 0;

    
    if(WakeupSrc < 0 || WakeupSrc >= PM_WAKEUP_SOURCE_END)
    {
        LOG_Error("%s parameter WakeupSrc is not in limit.",__FUNCTION__);
        return RET_ERR;
    }
    
    RpcGetMsgSync(RPCSERVER_TYPE_PMSERVER, RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID6, 0, &WakeupSrc, sizeof(WakeupSrc), &recvdata, &recvlen);
    if(recvlen == sizeof(uint8_t))
    {
       return recvdata;
    }
    else
    {
        LOG_Error("%s : RpcGetMsgSync is failed.",__FUNCTION__);
        return RET_ERR;
    }
}

/**
 * @brief 切换系统进入休眠状态
 *
 * @return int8_t 接口调用结果，取值：RET_ERR 失败，RET_OK 成功
 */
int8_t PmSleepSystem(void)
{
    uint8_t recvdata = 0;
    uint16_t recvlen = 0;
    
    RpcGetMsgSync(RPCSERVER_TYPE_PMSERVER, RPCSERVER_WAKEUPSRC_MSGTYPE_SUBCMD_ID7, 0, NULL, 0, &recvdata, &recvlen);
    int8_t rcvData = recvdata;
    if(rcvData == sizeof(int8_t))
    {
       return rcvData;
    }
    else
    {
        LOG_Error("%s : RpcGetMsgSync is failed.",__FUNCTION__);
        return RET_ERR;
    }
}


