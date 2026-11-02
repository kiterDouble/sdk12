#if 0
#include <stdatomic.h>

#include <sys/syscall.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>
#include <pthread.h>
#include "mcu_api.h"
#include "err_code.h"
#include "sr_log.h"
#include "can_tp.h"

#define CANTP_SHM_PATH "/canTpLibShm"

#define CANTP_IGNORE_LOCK_TRUE (1)
#define CANTP_IGNORE_LOCK_FALSE (0)

typedef struct
{
    atomic_int mutex_initialized;
    pthread_mutex_t mutex;
    pid_t pid;    
    time_t lastActive;
}CanTpLibShm_t;

typedef struct
{
    uint8_t countIdx;
    uint8_t threadExit;   
    pthread_t tid;
    pthread_mutex_t sendMutex;
    pthread_mutex_t respMutex;
    pthread_cond_t respCond;
    CanTpLibShm_t *canTpShm;
    CanTpReceiveCb_t rxCb;
    CanTpTransmitErrorCb_t transErrCb;
    int32_t ignore;
}CanTpPriv_t;

typedef struct
{
    uint8_t reserve;
    uint8_t status;
    uint8_t countIdx;
    uint8_t uds[0];
}CanTpServiceData_t;

/**
 * @name CanTpInitLibShm
 * @brief 初始化共享内存
 *        本函数用于初始化CAN传输层的共享内存。如果共享内存已经初始化，则函数会直接返回成功。
 *        否则，它将尝试创建和映射一个新的共享内存段用于CAN传输层的通信。
 * 
 * @param priv 指向CanTpPriv_t结构体的指针，该结构体包含了CAN传输层的私有数据。
 * @return 返回RET_OK表示成功初始化，RET_ERR表示初始化失败。
 */
static int32_t CanTpInitLibShm(CanTpPriv_t *priv)
{
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }

    if (priv->canTpShm != MAP_FAILED)
    {
        LOG_Error("%s shm hava init.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    int32_t fd = shm_open(CANTP_SHM_PATH, O_CREAT | O_RDWR, 0644);

    if (fd == -1)
    {
        LOG_Error("Open cantpLibShm File Error:%s", g_strerror(errno));
        return ret;
    }
    if (ftruncate(fd, sizeof(CanTpLibShm_t)) != 0)
    {
        LOG_Error("ftruncate TreeShm File error:%s", g_strerror(errno));
        close(fd);
        return ret;
    }
    priv->canTpShm = mmap(NULL, sizeof(CanTpLibShm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (priv->canTpShm == MAP_FAILED)
    {
        LOG_Error("mmap cantpLibShm Error:%s", g_strerror(errno));
        close(fd);
        return ret;
    }

    if (atomic_load(&(priv->canTpShm->mutex_initialized)) == 0)
    {
        int32_t expected = 0;
        priv->canTpShm->pid = -1;
        if (atomic_compare_exchange_strong(&(priv->canTpShm->mutex_initialized), &expected, 1))
        {
            pthread_mutexattr_t mutex_attr;
            pthread_mutexattr_init(&mutex_attr);
            pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(&priv->canTpShm->mutex, &mutex_attr);
            pthread_mutexattr_destroy(&mutex_attr);
        }
    }

    ret = RET_OK;
    close(fd);
    return ret;
}

/**
 * @name CanTpCheckActived
 * @brief 线程函数，用于检查CAN传输层的活动状态。
 *        本函数是一个线程函数，用于检查CAN传输层的活动状态。它通过定期检查CAN传输层的活动状态来保持活动状态。
 *        如果CAN传输层被激活，则函数会检查上次活动时间是否超过300秒，如果超过300秒，则认为CAN传输层被激活。
 *        如果CAN传输层被激活，则函数会检查上次活动
 *        时间是否超过300秒，如果超过300秒，则认为CAN传输层被激活。
 * @param args  参数
 * @return 函数无返回值
 */
static void* CanTpCheckActived(void* args)
{
    CanTpPriv_t *priv = (CanTpPriv_t *)args;
    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return NULL;
    }

    while (!priv->threadExit)
    {

        if ((CANTP_IGNORE_LOCK_FALSE == priv->ignore) && (getpid() == priv->canTpShm->pid))
        {        
            if ((priv->canTpShm->lastActive > 0) && ((time(NULL) - priv->canTpShm->lastActive) > 300))
            {
                pthread_mutex_lock(&priv->canTpShm->mutex);
                priv->canTpShm->pid = -1;
                pthread_mutex_unlock(&priv->canTpShm->mutex);                
            }
        }
        usleep(500000);
    }
    return NULL;
}

/**
 * @name canTp_send_ack
 * @brief 发送确认应答
 *        此函数用于在CAN传输协议中发送确认应答。它会根据传入的服务数据和用户数据执行相应的操作，
 *        包括条件性地触发传输错误回调。
 * @param data 指向包含服务数据的缓冲区的指针。
 * @param len 服务数据的长度。如果长度小于CanTpServiceData_t的大小，函数将记录错误并返回。
 * @param userdata 指向用户数据的指针，在此函数中被转换为CanTpPriv_t类型的指针以访问私有数据。
 * @return 函数无返回值
 */
static void CanTpSendAck(const uint8_t* data, const uint16_t len, void* userdata)
{
    CanTpPriv_t *priv = (CanTpPriv_t *)userdata;
    const CanTpServiceData_t *srvData = (CanTpServiceData_t *)data;
    
    if (!priv || !srvData || (len < sizeof(CanTpServiceData_t)))
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return;
    }

    if ((CANTP_IGNORE_LOCK_FALSE == priv->ignore) && (getpid() != priv->canTpShm->pid))
    {
        LOG_Error("%s unlocked.", __FUNCTION__);
        return;        
    }

    LOG_DebugHex(data, len, "CanTpSendAck");
    pthread_mutex_lock(&priv->respMutex);
    pthread_cond_signal(&priv->respCond);
    pthread_mutex_unlock(&priv->respMutex);
    LOG_Debug("CanTpSendAck:countIdx:%d,status:%d,reserve:%d, PRIV IDX:%d", srvData->countIdx, srvData->status, srvData->reserve, priv->countIdx);
    if (srvData->countIdx == priv->countIdx)
    {
        priv->transErrCb(E_CANTP_TRANSMIT_OK);
    }
    else
    {
        priv->transErrCb(E_CANTP_TRANSMIT_NOT_OK);
    }     
}

/**
 * @name CanTpRecvResp
 * @brief 处理接收到的CAN TP响应。
 * @param data 接收到的数据指针。
 * @param len 接收到的数据长度。
 * @param userdata 用户数据指针，用于访问私有数据。
 * @return 函数无返回值
 */
static void CanTpRecvResp(const uint8_t* data, const uint16_t len, void* userdata)
{
    CanTpPriv_t *priv = (CanTpPriv_t *)userdata;
    CanTpServiceData_t *srvData = (CanTpServiceData_t *)data;
    
    if (!priv || !srvData || (len < (sizeof(CanTpServiceData_t) + 4)))
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return;
    }

    if ((CANTP_IGNORE_LOCK_FALSE == priv->ignore) && (getpid() != priv->canTpShm->pid))
    {
        LOG_Error("%s unlocked.", __FUNCTION__);
        return;        
    }


    //send ack
    uint8_t buffer[sizeof(CanTpServiceData_t)] = {0x0, 0x0, priv->countIdx};
    //CanTpServiceData_t srvdata = {.reserve = 0x0,.status = 0x0, .countIdx = priv->countIdx};
    McuMesg_t mesg;
    mesg.type = SYS_QUE_TYPE_MCU_CANTP_MCU_TO_MPU_ACK;//0x00070104
    mesg.payloadLen = sizeof(CanTpServiceData_t);
    mesg.payload = (uint8_t*)buffer;
    McuSrvSendMesg(&mesg);
    LOG_DebugHex(mesg.payload, mesg.payloadLen, "resp ack");
    LOG_DebugHex(data, len, "CanTpRecvResp");
    uint32_t canId = (srvData->uds[0] & 0xFF) << 24;
    canId += (srvData->uds[1] & 0xFF) << 16;
    canId += (srvData->uds[2] & 0xFF) << 8;
    canId += srvData->uds[3] & 0xFF;
    priv->rxCb(canId, (void *)&srvData->uds[4], len - sizeof(CanTpServiceData_t) - 4 );
}

/**
 * @name CanTpInit
 * @brief CAN-TP初始化。
 * @param hdl 指向CanTpHandle_t的指针，用于存储协议栈的句柄。
 * @param rxCb 接收回调函数，用于处理接收到的CAN消息。
 * @param transErrCb 传输错误回调函数，用于处理传输过程中的错误。
 * @return 成功返回RET_OK，失败返回RET_ERR。
 */
int32_t CanTpInit(CanTpHandle_t *hdl, CanTpReceiveCb_t rxCb, CanTpTransmitErrorCb_t transErrCb)
{
    CanTpPriv_t *priv = NULL;
    int32_t ret = RET_ERR;

    if (!hdl || !rxCb || !transErrCb)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }

    priv = (CanTpPriv_t *) calloc(1, sizeof(CanTpPriv_t));

    if (!priv)
    {
        LOG_Error("%s calloc priv fail.", __FUNCTION__);
        return ret;        
    }

    priv->canTpShm = MAP_FAILED;
    if(RET_ERR == CanTpInitLibShm(priv))
    {
        LOG_Error("%s CanTpInitLibShm fail.", __FUNCTION__);
        free(priv);
        return ret;
    }

    if (0 != pthread_mutex_init(&priv->sendMutex, NULL))
    {
        LOG_Error("%s pthread_mutex_init sendMutex fail.", __FUNCTION__);
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
        free(priv);
        return ret;        
    }

    if (0 != pthread_mutex_init(&priv->respMutex, NULL))
    {
        LOG_Error("%s pthread_mutex_init respMutex fail.", __FUNCTION__);
        pthread_mutex_destroy(&priv->sendMutex);
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
        free(priv);
        return ret;        
    }

    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    if (0 != pthread_cond_init(&priv->respCond, &cattr))
    {
        LOG_Error("%s pthread_cond_init respCond fail.", __FUNCTION__);
        pthread_mutex_destroy(&priv->respMutex);
        pthread_mutex_destroy(&priv->sendMutex);
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
        free(priv);
        return ret;          
    }
    
    if (0 != pthread_create(&priv->tid, NULL, CanTpCheckActived, (void *)priv))
    {
        LOG_Error("%s pthread_cond_init respCond fail.", __FUNCTION__);
        pthread_cond_destroy(&priv->respCond);
        pthread_mutex_destroy(&priv->respMutex);
        pthread_mutex_destroy(&priv->sendMutex);
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
        free(priv);
        return ret;           
    }

    priv->rxCb = rxCb;
    priv->transErrCb = transErrCb;
    priv->countIdx = 0;
    priv->ignore = CANTP_IGNORE_LOCK_FALSE;
    StartQueueMsgRcvThread("cantp");
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_MPU_TO_MCU_ACK, CanTpSendAck, (void*)priv);//0x070102
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_MCU_TO_MPU, CanTpRecvResp, (void*)priv);//0x070103
    *hdl = priv;
    ret = RET_OK;
    return ret;
}

/**
 * @name CanTpTransSend
 * @brief 发送CAN传输消息
 * @param priv 指向CanTpPriv_t结构体的指针，包含CAN传输的私有数据
 * @param canId CAN消息的ID
 * @param data 指向要发送数据的指针
 * @param dataLen 要发送数据的长度
 * @param repeat 是否重复发送的标志
 * @return int32_t 返回执行结果，RET_OK表示成功，其他值表示失败
 */
static int32_t CanTpTransSend(CanTpPriv_t *priv, uint32_t canId, uint8_t *data, uint32_t dataLen, uint32_t repeat)
{
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[5000] = {0x0,};
    CanTpServiceData_t *srvData = (CanTpServiceData_t *)buffer;
    srvData->status = repeat ? 0x80 : 0x0;
    srvData->countIdx = priv->countIdx;
    srvData->uds[0] = (canId >> 24) & 0xFF;
    srvData->uds[1] = (canId >> 16) & 0xFF;
    srvData->uds[2] = (canId >> 8) & 0xFF;
    srvData->uds[3] = canId & 0xFF;
    memcpy(& srvData->uds[4], data, dataLen);

    McuMesg_t mesg;
    mesg.type = SYS_QUE_TYPE_MCU_CANTP_MPU_TO_MCU;//0x00070101
    mesg.payloadLen = dataLen + sizeof(CanTpServiceData_t) + 4;
    mesg.payload = buffer;

    LOG_DebugHex(mesg.payload, mesg.payloadLen, "CanTpTransSend");
    if(RET_OK != McuSrvSendMesg(&mesg))
    {
        LOG_Error("%s McuSrvSendMesg fail.", __FUNCTION__);
        return ret;
    }

    uint64_t timeout_ms = 200;
    struct timespec outtime;
    uint64_t nsec = 0;

    clock_gettime(CLOCK_MONOTONIC, &outtime);
    nsec = outtime.tv_nsec + (timeout_ms % 1000) * 1000000;
    outtime.tv_sec = outtime.tv_sec + timeout_ms / 1000 + nsec / 1000000000;
    outtime.tv_nsec = nsec % 1000000000;

    pthread_mutex_lock(&priv->respMutex);
    if (pthread_cond_timedwait(&priv->respCond, &priv->respMutex, &outtime) != 0)
    {
        LOG_Error("CanTp_Transmit send timeout!");
    }
    else
    {
        ret = RET_OK;
        priv->countIdx++;
    }
    pthread_mutex_unlock(&priv->respMutex);
    return ret;

}

/**
 * @name CanTpTransmit
 * @brief CAN-TP发送函数
 * @param hdl CanTp处理句柄，用于访问私有数据
 * @param canId CAN消息的ID
 * @param data 指向要发送数据的指针
 * @param dataLen 要发送的数据长度
 * @return int32_t 返回传输结果，RET_OK表示成功，其他值表示失败
 */
int32_t CanTpTransmit(CanTpHandle_t hdl, uint32_t canId, uint8_t *data, uint32_t dataLen)
{
    CanTpPriv_t *priv = (CanTpPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }

    if ((CANTP_IGNORE_LOCK_FALSE == priv->ignore) && (getpid() != priv->canTpShm->pid))
    {
        LOG_Error("%s unlocked, can not transmit.", __FUNCTION__);
        return ret;        
    }

    priv->canTpShm->lastActive = time(NULL);
    pthread_mutex_lock(&priv->sendMutex);
    uint32_t sendtimes = 3;
    uint32_t repeat = 0;

    while (sendtimes-- > 0)
    {
        if (RET_OK == CanTpTransSend(priv, canId, data, dataLen, repeat))
        {
            ret = RET_OK;
            break;
        }
        repeat = 1;
    }
    pthread_mutex_unlock(&priv->sendMutex);

    return ret;
}

/**
 * @name CanTpDeinit
 * @brief 反初始化CAN-TP协议栈。
 * @param hdl 协议栈实例的句柄。
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败。
 */
int32_t CanTpDeinit(CanTpHandle_t hdl)
{
    CanTpPriv_t *priv = (CanTpPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }

    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_MPU_TO_MCU_ACK);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_MCU_TO_MPU);
    priv->threadExit = 1;
    pthread_join(priv->tid, NULL);
    pthread_cond_destroy(&priv->respCond);
    pthread_mutex_destroy(&priv->respMutex);
    pthread_mutex_destroy(&priv->sendMutex);
    munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
    priv->canTpShm = NULL;
    free(priv);
    ret = RET_OK;
    return ret;
}

/**
 * @name CanTpLock
 * @brief CAN TP加锁
 * @param hdl CanTpHandle_t类型，CAN-TP的句柄，用于标识特定的CAN-TP实例。
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t CanTpLock(CanTpHandle_t hdl)
{
    CanTpPriv_t *priv = (CanTpPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }

    if (priv->ignore)
    {
        LOG_Debug("%s ignore, return success.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }
    if (priv->canTpShm->pid == getpid())
    {
        ret = RET_OK;
        return ret;
    }

    pthread_mutex_lock(&priv->canTpShm->mutex);
    if ((priv->canTpShm->pid != -1) && (syscall(SYS_tkill, priv->canTpShm->pid, 0) != -1))
    {
        LOG_Error("%s lock fail,process:%d have locked.", __FUNCTION__, priv->canTpShm->pid);
        pthread_mutex_unlock(&priv->canTpShm->mutex);
        return ret; 
    }
    
    priv->canTpShm->lastActive = time(NULL);
    priv->canTpShm->pid = getpid();
    pthread_mutex_unlock(&priv->canTpShm->mutex);
    ret = RET_OK;
    return ret;    
}

/**
 * @name CanTpUnlock
 * @brief CAN TP解锁
 * @param hdl CanTpHandle_t类型，CAN TP的处理句柄，指向CanTpPriv_t结构体的指针
 * @return 操作成功返回RET_OK，失败返回RET_ERR。 
 */
int32_t CanTpUnlock(CanTpHandle_t hdl)
{
    CanTpPriv_t *priv = (CanTpPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }

    if (priv->ignore)
    {
        LOG_Debug("%s ignore, return success.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    if (priv->canTpShm->pid != getpid())
    {
        LOG_Error("%s unlock fail,locked task:%d, curr:%d.", __FUNCTION__, priv->canTpShm->pid, getpid());
        return ret;
    }

    pthread_mutex_lock(&priv->canTpShm->mutex);
    priv->canTpShm->pid = -1;
    priv->canTpShm->lastActive = 0;
    pthread_mutex_unlock(&priv->canTpShm->mutex);
    ret = RET_OK;
    return ret;  
}


/**
 * @name CanTpSetLockIgnore
 * @brief 设置忽略锁状态，锁定状态由上层APP控制
 * @param hdl CanTp处理句柄，用于标识和操作特定的CanTp实例。
 * @return int32_t 返回执行结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t CanTpSetLockIgnore(CanTpHandle_t hdl)
{
    CanTpPriv_t *priv = (CanTpPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }

    priv->ignore = CANTP_IGNORE_LOCK_TRUE;
    ret = RET_OK;
    // 返回执行结果
    return ret;
}
    #endif