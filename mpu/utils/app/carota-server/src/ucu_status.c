#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "sr_log.h"
#include "err_code.h"
#include "sr_ipc.h"
#include "sdk/mcu_api.h"
#include "sgmw_ipc.h"
#include "ucu_status.h"
#include "ucu_upgrade.h"


static int g_ucuStatus = UCU_STATUS_NORMAL;
static int g_gwStatus = GW_STATUS_NORMAL;
/**
 * 整车状态全局变量
*/
static int g_vehicleStatus = 0;
static gint g_canLockType = CAN_LOCKED_NONE;
//static struct timespec g_dotaTimeout;
static pthread_mutex_t g_Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_lockMutex = PTHREAD_MUTEX_INITIALIZER;
static UcuStatusChangedCb g_changedCb = NULL;
static unsigned char covert2ucumode[] = {UCU_STATUS_NORMAL, UCU_STATUS_FOTA, UCU_STAUTS_OBD, UCU_STATUS_DOTA};
static unsigned char covert2gwmode[] = {GW_STATUS_NORMAL, GW_STATUS_FOTA, GW_STAUTS_OBD, GW_STATUS_DOTA};
static struct timespec g_changeLockTimeout;
static gint g_changeLock = 0;

static inline int ucuStatusConvert2UcuMode(unsigned char value)
{
    return covert2ucumode[value];
}

static inline int ucuStatusConvert2GwMode(unsigned char value)
{
    return covert2gwmode[value];
}

int UcuStatusClearStatus(int status);


static void UcuStatusMcuMsgProc(const uint8_t* data, const uint16_t len, void* userdata)
{
    const uint8_t *pdata = data;
    int status;

    if ((pdata[0] & 0x0F) > GW_STATUS_DOTA)
    {
        LOG_Debug("pdata[0] :%d", pdata[0]);
        return;
    }
    // LOG_Debug("pdata[0] :%d", pdata[0]);
    // LOG_Debug("pdata[1] :%d", pdata[1]);
    // LOG_Debug("pdata[2] :%d", pdata[2]);
    g_vehicleStatus = (int)pdata[0];
    status = g_vehicleStatus & 0x0F;
    printf("UcuStatusMcuMsgProc:%d\r\n",status); 
    //****************************************************************
    //适配网关异常退出时，tbox自身ucustatus回到normal

    // if (((g_gwStatus == GW_STATUS_FOTA) || (g_gwStatus == GW_STATUS_DOTA))
    //     &&(((g_vehicleStatus & 0xF0) >>4) == GW_STATUS_NORMAL)
    //     &&(status != UCU_STATUS_NORMAL))
    // {
    //     if(g_gwStatus == GW_STATUS_FOTA)
    //     {
    //         UcuStatusClearStatus(UCU_STATUS_FOTA);
    //         LOG_Error("gw abnormal exit GW_STATUS_FOTA,clear ucustatus to normal.");
    //     }
    //     else if (g_gwStatus == GW_STATUS_DOTA)
    //     {
    //         UcuStatusClearStatus(UCU_STATUS_DOTA);
    //         LOG_Error("gw abnormal exit GW_STATUS_DOTA,clear ucustatus to normal.");
    //     }                             
    // }
    //***************************************************************
    g_gwStatus = (g_vehicleStatus & 0xF0) >>4;  //网关状态
    LOG_Debug("get g_vehicleStatus :%d", g_vehicleStatus);

    pthread_mutex_lock(&g_Mutex);
    if (g_changedCb && (g_ucuStatus != status))
    {
        g_changedCb(g_ucuStatus, status);
    }
    g_ucuStatus = status;    
   
    pthread_mutex_unlock(&g_Mutex);
}

static gint checkLockCondition(gint type)
{
    int status = UCU_STATUS_NORMAL;
    int ret = RET_ERR;

    UcuStatusGetStatus(&status);
    LOG_Debug("%s called.status:%d,type:%d.", __FUNCTION__, status, type);
    if (CAN_LOCKED_FOTA == type)
    {
        if ((UCU_STATUS_NORMAL == status) || (UCU_STATUS_FOTA == status))
        {
            ret = RET_OK;
        }
    } 
    else if (CAN_LOCKED_DOTA == type)
    {
        if ((UCU_STATUS_NORMAL == status) || (UCU_STATUS_DOTA == status))
        {
            ret = RET_OK;
        } 
    }
    return ret;
}

int UcuStatusSetStatus(int status)
{
    unsigned char ucuStatus;
    int ret = RET_ERR;
	LOG_Debug("%s called, status=%d.", __FUNCTION__, status);

    pthread_mutex_lock(&g_Mutex);
    if (status == g_ucuStatus)
    {
        ret = RET_OK;
        goto error_exit;
    }

    if ((status < UCU_STATUS_NORMAL) || (status > UCU_STATUS_DOTA)
        || (g_ucuStatus != UCU_STATUS_NORMAL))
    {
        // LOG_Error("%s Set check fail, status:%d, g_ucuStatus:%d.", __FUNCTION__, status, g_ucuStatus);
        printf("%s Set check fail, status:%d, g_ucuStatus:%d.", __FUNCTION__, status, g_ucuStatus);
        ret = -1;
        goto error_exit;
    }
    // LOG_Debug("set status %d", status);
    printf("set status %d", status);
    ucuStatus = ucuStatusConvert2GwMode(status & 0xff);
    // LOG_Debug("set ucuStatus %d", ucuStatus);
    printf("set ucuStatus %d \r\n", ucuStatus);

    McuMesg_t mesg = 
    {
        .type = SYS_QUE_TYPE_SGMW_GET_OBD_STATUS,
        .payloadLen = sizeof(ucuStatus),
        .payload = (uint8_t *)&ucuStatus,
    };

    McuSrvSendMesg(&mesg);

    LOG_Debug("%s called end.", __FUNCTION__);
    ret = RET_OK;

error_exit:
    pthread_mutex_unlock(&g_Mutex);
    return ret;
}

int UcuStatusClearStatus(int status)
{
    unsigned char ucuStatus;
    int ret = RET_ERR;
	LOG_Debug("%s called, status=%d.", __FUNCTION__, status);

    pthread_mutex_lock(&g_Mutex);
    if (status != g_ucuStatus)
    {
        LOG_Error("%s clear fail.status:%d, g_ucuStatus:%d.", __FUNCTION__, status, g_ucuStatus);
        goto error_exit;
    }

    ucuStatus = ucuStatusConvert2GwMode(UCU_STATUS_NORMAL & 0xff);

    McuMesg_t mesg = 
    {
        .type = SYS_QUE_TYPE_SGMW_GET_OBD_STATUS,
        .payloadLen = sizeof(ucuStatus),
        .payload = (uint8_t *)&ucuStatus,
    };

    McuSrvSendMesg(&mesg);
    LOG_Debug("%s called end.", __FUNCTION__);
    ret = RET_OK;
error_exit:
    pthread_mutex_unlock(&g_Mutex);
    return ret;
}

int UcuStatusGetStatus(int *status)
{
    pthread_mutex_lock(&g_Mutex);
	LOG_Debug("%s called, g_ucuStatus:%d.", __FUNCTION__, g_ucuStatus);
    *status = g_ucuStatus;
    pthread_mutex_unlock(&g_Mutex);
    return RET_OK;
}

int GWStatusGetStatus(int *status)
{
    pthread_mutex_lock(&g_Mutex);
	LOG_Debug("%s called, g_gwStatus:%d.", __FUNCTION__, g_gwStatus);
    *status = g_gwStatus;
    pthread_mutex_unlock(&g_Mutex);
    return RET_OK;
}
/**
 * Dbus数据填充获取接口
*/
int RpcGetVehicleStatus(int *status)
{
    pthread_mutex_lock(&g_Mutex);
	LOG_Debug("%s called, g_vehicleStatus:%d.", __FUNCTION__, g_vehicleStatus);
    *status = g_vehicleStatus;
    pthread_mutex_unlock(&g_Mutex);
    return RET_OK;
}

int UcuStatudCantpLock(int type)
{
    int ret = RET_OK;

    LOG_Debug("%s called, type=%d.", __FUNCTION__, type);

    if(checkLockCondition(type) == 0)
    {
        pthread_mutex_lock(&g_lockMutex);
        if(g_canLockType == CAN_LOCKED_NONE)
        {
            g_canLockType = type;
        }
        else
        {
            ret = RET_ERR;
            LOG_Error("%s UcuStatudCantpLock failed current lock type: %d", __FUNCTION__, g_canLockType);
        }
        pthread_mutex_unlock(&g_lockMutex);
    }
    else
    {
        ret = RET_ERR;
        LOG_Error("%s UcuStatudCantpLock failed current ucustatus: %d", __FUNCTION__, g_ucuStatus);
    }

    return ret;
}

int UcuStatudCantpUnlock(int type)
{
    int ret =  RET_OK;

    LOG_Debug("%s called, type=%d.", __FUNCTION__, type);

    pthread_mutex_lock(&g_lockMutex);
    if (type == g_canLockType)
    {
        g_canLockType = CAN_LOCKED_NONE;
    }
    else
    {
        ret = RET_ERR;
        LOG_Error("%s UcuStatudCantpUnlock failed current lock type: %d", __FUNCTION__, g_canLockType);
    }
    pthread_mutex_unlock(&g_lockMutex);

    return ret;
}

int UcuStatudGetCantpLockType(int *canLockType) 
{
    pthread_mutex_lock(&g_lockMutex);
    LOG_Debug("%s called, canLockType:%d.", __FUNCTION__, g_canLockType);
    *canLockType = g_canLockType;
    pthread_mutex_unlock(&g_lockMutex);
    return RET_OK;
}

void UcuStatusChangedCheck(int old, int new)
{
    pthread_mutex_lock(&g_lockMutex);
    if ((UCU_STATUS_NORMAL == old) && (CAN_LOCKED_NONE != g_canLockType))
    {
        if (UCU_STAUTS_OBD == new)
        {
            g_canLockType = CAN_LOCKED_NONE;            
        }

        if (((UCU_STATUS_FOTA == new) && (CAN_LOCKED_FOTA != g_canLockType))
            || ((UCU_STATUS_DOTA == new) && (CAN_LOCKED_DOTA != g_canLockType)))
        {
            g_changeLock = 1;
            clock_gettime(CLOCK_MONOTONIC, &g_changeLockTimeout);
            g_changeLockTimeout.tv_sec +=5;
        }
    }
    LOG_Debug("%s called, new:%d, old:%d, lockType:%d.", __FUNCTION__, new, old, g_canLockType);
    pthread_mutex_unlock(&g_lockMutex);
}

void CanLockCheckTimeout(int *lockType)
{
    struct timespec curTime;
    clock_gettime(CLOCK_MONOTONIC, &curTime);
    pthread_mutex_lock(&g_lockMutex);
    if ((g_changeLock == 1) && (timespec_compare(&curTime, &g_changeLockTimeout) > 0))
    {
        g_changeLock = 0;
        g_canLockType = CAN_LOCKED_NONE; 
    }
    *lockType = g_canLockType;
    pthread_mutex_unlock(&g_lockMutex);
}

int UcuStatusInit(UcuStatusChangedCb Cb)
{
    //将全局变量g_ucuStatus初始化为normal，订阅010b
    g_ucuStatus = UCU_STATUS_NORMAL;
    SubscriptionQueueMsg(SYS_QUE_TYPE_SGMW_GET_OBD_STATUS, UcuStatusMcuMsgProc, NULL);
    g_changedCb = Cb;
    return RET_OK;
}