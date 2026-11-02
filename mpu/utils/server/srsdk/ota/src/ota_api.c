#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "err_code.h"
#include "ota_api.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "mcu_api.h"  
#include "queue_msg.h" 
#include "hal_modem_upgrade.h"
#include "ota_upgrade_mcu.h"

typedef struct
{
    atomic_int mutex_initialized;
    pthread_mutex_t mutex;
    pid_t pid;    
    time_t lastActive;
}OtaLibShm_t;

#define OTA_SHM_PATH "/otaLibShm"
#define OTA_MPU_PATH_NAME "/oemdata/upgrade/update.zip"

static OtaLibShm_t *g_OtaLibShm = MAP_FAILED;
static int32_t g_UpgradeStatus = UPGRADE_INIT;
static GThread *g_UpgradeThread = NULL;
static pthread_mutex_t g_upgradeLock = PTHREAD_MUTEX_INITIALIZER;
static char g_upgradeFilePath[UPGRADE_PATH_MAX_LEN] = {0};
static UpgradeFlag_E g_upgradeFlag = 0;

void IpcSendUpgradeState(UpgradeStatus_E UpgradeStatus)
{
    UpgradeStatus_E tmpUpgradeStatus = UPGRADE_INIT;
    tmpUpgradeStatus = UpgradeStatus;
    SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_SGMW_UPGRADE_STATUS_SYNC, (void *)&tmpUpgradeStatus, sizeof(UpgradeStatus_E));
}

/**
 * @name OtaInitLibShm
 * @brief 初始化共享内存
 * 
 * @return int32_t 返回操作结果，操作成功返回RET_OK，失败返回RET_ERR。
 */
static int32_t OtaInitLibShm()
{
    int32_t ret = RET_ERR;

    if (g_OtaLibShm != MAP_FAILED)
    {
        LOG_Error("%s shm hava init.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    int32_t fd = shm_open(OTA_SHM_PATH, O_CREAT | O_RDWR, 0644);

    if (fd == -1)
    {
        LOG_Error("Open cantpLibShm File Error:%s", g_strerror(errno));
        return ret;
    }
    if (ftruncate(fd, sizeof(OtaLibShm_t)) != 0)
    {
        LOG_Error("ftruncate TreeShm File error:%s", g_strerror(errno));
        close(fd);
        return ret;
    }

    g_OtaLibShm = mmap(NULL, sizeof(OtaLibShm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (g_OtaLibShm == MAP_FAILED)
    {
        LOG_Error("mmap cantpLibShm Error:%s", g_strerror(errno));
        close(fd);
        return ret;
    }

    if (atomic_load(&(g_OtaLibShm->mutex_initialized)) == 0)
    {
        int32_t expected = 0;
        g_OtaLibShm->pid = -1;
        if (atomic_compare_exchange_strong(&(g_OtaLibShm->mutex_initialized), &expected, 1))
        {
            pthread_mutexattr_t mutex_attr;
            pthread_mutexattr_init(&mutex_attr);
            pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(&g_OtaLibShm->mutex, &mutex_attr);
            pthread_mutexattr_destroy(&mutex_attr);
        }
    }

    ret = RET_OK;
    close(fd);
    return ret;
}

/**
 * @name UpgradeTaskCreate
 * @brief 创建升级任务
 * 
 * @return int32_t 返回操作结果，操作成功返回RET_OK，失败返回RET_ERR。
 */
static int32_t UpgradeTaskCreate()
{
    int32_t retValue = RET_ERR;

    if(NULL == g_OtaLibShm)
    {
        LOG_Error("%s lock fail,g_OtaLibShm is NULL .", __FUNCTION__);
        return retValue;
    }

    pthread_mutex_lock(&g_OtaLibShm->mutex);
    if (g_OtaLibShm->pid != -1)
    {
        retValue = syscall(SYS_tkill,  g_OtaLibShm->pid, 0);
        if(retValue == 0)
        {
            LOG_Error("%s lock fail,process:%d have locked.", __FUNCTION__, g_OtaLibShm->pid);
            pthread_mutex_unlock(&g_OtaLibShm->mutex);
            return RET_ERR;
        }
        g_OtaLibShm->pid = -1;
    }

    if(g_OtaLibShm->pid == syscall(SYS_gettid))
    {
        pthread_mutex_unlock(&g_OtaLibShm->mutex);
        LOG_Error("%s lock fail,pid:%d the same ", __FUNCTION__, g_OtaLibShm->pid);
        return RET_OK;
    }

    g_OtaLibShm->lastActive = time(NULL);
    g_OtaLibShm->pid = syscall(SYS_gettid);
    pthread_mutex_unlock(&g_OtaLibShm->mutex);

    return RET_OK;
}
/**
 * @name UpgradeTaskDestroy
 * @brief 销毁升级任务
 * 
 * @return int32_t 返回操作结果，操作成功返回RET_OK，失败返回RET_ERR。
 */
static int32_t UpgradeTaskDestroy()
{
    int32_t retValue = RET_ERR;

    if(NULL == g_OtaLibShm)
    {
        LOG_Error("%s lock fail,g_OtaLibShm is NULL .", __FUNCTION__);
        return retValue;
    }

    pthread_mutex_lock(&g_OtaLibShm->mutex);
    if (g_OtaLibShm->pid != -1)
    {
        retValue = syscall(SYS_tkill,  g_OtaLibShm->pid, 0);
        if(retValue == 0)
        {
            LOG_Error("%s unlock fail,process:%d have locked.", __FUNCTION__, g_OtaLibShm->pid);
            pthread_mutex_unlock(&g_OtaLibShm->mutex);
            return retValue;
        }
    }

    g_OtaLibShm->lastActive = time(NULL);
    g_OtaLibShm->pid = -1;
    pthread_mutex_unlock(&g_OtaLibShm->mutex);

    return RET_OK;
}


/**
 * @name UpgradeThreadFunc
 * @brief 升级线程函数
 * 
 * @param arg 指向升级标志的指针，用于指示需要执行的升级类型。
 * @return gpointer 总是返回NULL，因为该线程在完成任务后会退出。
 */
gpointer UpgradeThreadFunc(gpointer arg)
{
    UpgradeFlag_E upgradeFlag = UPGRADE_BT_MCU_MPU_FLAG;
    int32_t retValue = RET_ERR;

    LOG_Debug("upgrade thread start");

    if(NULL == arg)
    {
        LOG_Error("%s arg is NULL .", __FUNCTION__);
        g_thread_exit(NULL);
        return NULL;
    }
    
    upgradeFlag = *((UpgradeFlag_E *)arg);
    LOG_Debug("upgradeFlag :%d", upgradeFlag);

    retValue = OtaInitLibShm();
    if(retValue < RET_OK)
    {
        LOG_Error("%s OtaInitLibShm error.", __FUNCTION__);
        g_thread_exit(NULL);
        return NULL;
    }
    
    retValue = UpgradeTaskCreate();
    if(retValue < RET_OK)
    {
        LOG_Error("%s upgrade is busy ", __FUNCTION__);
        g_UpgradeStatus = UPGRADE_IS_BUSY;
        g_thread_exit(NULL);
        return NULL;
    }

    pthread_mutex_lock(&g_upgradeLock);
    g_UpgradeStatus = UPGRADE_START;
    pthread_mutex_unlock(&g_upgradeLock);
     /*升级蓝牙代码UPGRADE_BT_FLAG*/
    if(0 != (upgradeFlag & UPGRADE_BT_FLAG))
    {
        pthread_mutex_lock(&g_upgradeLock);
        g_UpgradeStatus = UPGRADE_BT_FLASHING;
        IpcSendUpgradeState((UpgradeStatus_E)g_UpgradeStatus);
        pthread_mutex_unlock(&g_upgradeLock);
        retValue = UpgradeBtProcess(g_upgradeFilePath);
        if(retValue < RET_OK)
        {
            LOG_Error("%s upgrade bt is failed ", __FUNCTION__);
            pthread_mutex_lock(&g_upgradeLock);
            g_UpgradeStatus = UPGRADE_BT_FAILED;
            pthread_mutex_unlock(&g_upgradeLock);
            g_thread_exit(NULL);
            return NULL;
        }
        else
        {
            pthread_mutex_lock(&g_upgradeLock);
            g_UpgradeStatus = UPGRADE_BT_SUCCESS;
            pthread_mutex_unlock(&g_upgradeLock);
        }
    }

    if(0 != (upgradeFlag & UPGRADE_MCU_FLAG))
    {
        pthread_mutex_lock(&g_upgradeLock);
        g_UpgradeStatus = UPGRADE_MCU_FLASHING;
        IpcSendUpgradeState((UpgradeStatus_E)g_UpgradeStatus);
        pthread_mutex_unlock(&g_upgradeLock);
        retValue = UpgradeMcuProcess(g_upgradeFilePath);
        if(retValue < RET_OK)
        {
            LOG_Error("%s upgrade mcu is failed ", __FUNCTION__);
            pthread_mutex_lock(&g_upgradeLock);
            g_UpgradeStatus = UPGRADE_MCU_FAILED;
            pthread_mutex_unlock(&g_upgradeLock);
            g_thread_exit(NULL);
            return NULL;
        }
        else
        {
            pthread_mutex_lock(&g_upgradeLock);
            g_UpgradeStatus = UPGRADE_MCU_SUCCESS;
            pthread_mutex_unlock(&g_upgradeLock);
        }
    }

    if(0 != (upgradeFlag & UPGRADE_MPU_FLAG))
    {
        pthread_mutex_lock(&g_upgradeLock);
        g_UpgradeStatus = UPGRADE_MPU_FLASHING;
        pthread_mutex_unlock(&g_upgradeLock);
        retValue = HalModemUpgradeImpl((const uint8_t *)g_upgradeFilePath);
        if(retValue != RET_OK)
        {
            LOG_Error("%s upgrade mpu is failed ", __FUNCTION__);
            pthread_mutex_lock(&g_upgradeLock);
            g_UpgradeStatus = UPGRADE_MPU_FAILED;
            pthread_mutex_unlock(&g_upgradeLock);
            g_thread_exit(NULL);
            return NULL;
        }
        else
        {
            pthread_mutex_lock(&g_upgradeLock);
            g_UpgradeStatus = UPGRADE_MPU_NEED_BACKUP;
            pthread_mutex_unlock(&g_upgradeLock);
        }
    }
    
    pthread_mutex_lock(&g_upgradeLock);
    if(UPGRADE_START == g_UpgradeStatus)
    {
        g_UpgradeStatus = UPGRADE_INIT;
    }
    pthread_mutex_unlock(&g_upgradeLock);
    LOG_Debug("upgrade thread  exit");
    g_thread_exit(NULL);
    return NULL;
}
/**
 * @name OtaStartUpgrade
 * @brief  开始升级指令
 * @param 
 *      filePathName: 升级包的绝对路径名称 路径下需放 升级文件，MPU的升级包名称 update.zip  MCU升级文件
 *      upgradeFlag ：升级标记 UPGRADE_MCU_FLAG 升级MCU  
 *                            UPGRADE_MPU_FLAG 升级MPU
 *                            UPGRADE_ALL_FLAG MCU和CPU都升级
 * @return 成功返回0，失败返回小于0 直接返回不阻塞，升级状态通过 OtaGetUpgradeStatus 获取
 */
int32_t OtaStartUpgrade(const char *filePath, UpgradeFlag_E upgradeFlag)
{
    int32_t upgradeStatus = 0;
    static UpgradeFlag_E tmpUpgradeFlag = 0;

    if(NULL == filePath || strlen(filePath) >= UPGRADE_PATH_MAX_LEN)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    tmpUpgradeFlag = upgradeFlag;
    if(tmpUpgradeFlag < UPGRADE_MCU_FLAG || tmpUpgradeFlag > UPGRADE_BT_MCU_MPU_FLAG)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    //获取MPU升级状态，在升级状态时返回Error
    upgradeStatus = HalModemGetUpgradeStatus((uint8_t *)filePath);
    if(upgradeStatus < 0)
    {
        LOG_Error("%s HalModemGetUpgradeStatus error", __FUNCTION__);
        return RET_ERR;
    }

    if(HAl_UPGRADE_MPU_FLASHING    == upgradeStatus || 
       HAl_UPGRADE_MPU_NEED_BACKUP == upgradeStatus || 
       HAl_UPGRADE_MPU_BACKUPING   == upgradeStatus || 
       HAl_UPGRADE_MPU_NO_PATH     == upgradeStatus)
    {
        LOG_Error("%s hal upgrade mpu status upgrade not allowed ", __FUNCTION__);
        return RET_ERR;
    }

    //判断消息队列回调资源是否初始化 在有升级MCU时 未初始化返回error
    if(ipcIsFirstCall() && (tmpUpgradeFlag & UPGRADE_MCU_FLAG))
    {
        LOG_Error("%s have upgrade mcu ,but not init mcu msg, upgrade not allowed ", __FUNCTION__);
        return RET_ERR;
    }

    if(ipcIsFirstCall() && (tmpUpgradeFlag & UPGRADE_BT_FLAG))
    {
        LOG_Error("%s have upgrade bt ,but not init mcu msg, upgrade not allowed ", __FUNCTION__);
        return RET_ERR;
    }

    pthread_mutex_lock(&g_upgradeLock);
    if(NULL != g_UpgradeThread)
    {
        LOG_Error("%s NULL != g_UpgradeThread", __FUNCTION__);
        pthread_mutex_unlock(&g_upgradeLock);
        return RET_ERR;
    }

    g_UpgradeThread = g_thread_new("UpgradeThreadFunc", UpgradeThreadFunc, &tmpUpgradeFlag);
    if(NULL == g_UpgradeThread)
    {
        LOG_Error("%s g_thread_new error", __FUNCTION__);
        pthread_mutex_unlock(&g_upgradeLock);
        return RET_ERR;
    }
    memset(g_upgradeFilePath, 0, UPGRADE_PATH_MAX_LEN);
    memcpy(g_upgradeFilePath, filePath, strlen(filePath));
    g_upgradeFlag = tmpUpgradeFlag;
    pthread_mutex_unlock(&g_upgradeLock);

    return RET_OK;
}
/**
 * @name OtaGetUpgradeStatus
 * @brief 获取升级状态
 * @param 
 *      upgradeStatus: 出参 返回当前升级状态。
 * @return 成功返回0，失败返回小于0
 */
int32_t OtaGetUpgradeStatus(const char *filePath, UpgradeStatus_E *upgradeStatus)
{
    int32_t retValue = RET_ERR;
    int32_t mpuUpgradeStatus = 0;

    if(NULL == filePath || NULL == upgradeStatus)
    {
        LOG_Error("%s arg is invalid", __FUNCTION__);
        return retValue;
    }

    pthread_mutex_lock(&g_upgradeLock);

    if(NULL == g_UpgradeThread)
    {
        mpuUpgradeStatus = HalModemGetUpgradeStatus((uint8_t *)filePath);
        if(mpuUpgradeStatus < RET_OK)
        {
            LOG_Error("%s get hal modem upgrade status err", __FUNCTION__);
            pthread_mutex_unlock(&g_upgradeLock);
            return mpuUpgradeStatus;
        }
        if(HAl_UPGRADE_MPU_FLASHING == mpuUpgradeStatus )
        {
            g_UpgradeStatus = UPGRADE_IS_BUSY;
        }

        if(HAl_UPGRADE_MPU_OK == mpuUpgradeStatus)
        {
            g_UpgradeStatus = UPGRADE_MPU_SUCCESS;
        }

        if(HAl_UPGRADE_MPU_FAILED == mpuUpgradeStatus)
        {
            g_UpgradeStatus = UPGRADE_MPU_FAILED;
        }

        if(HAl_UPGRADE_MPU_BACKUPING == mpuUpgradeStatus || 
           HAl_UPGRADE_MPU_NEED_BACKUP == mpuUpgradeStatus)
        {
            g_UpgradeStatus = UPGRADE_MPU_BACKUP;
        }
        *upgradeStatus = g_UpgradeStatus;
    }
    else
    {
        *upgradeStatus = g_UpgradeStatus;

        if(UPGRADE_BT_FLAG == (g_upgradeFlag) && \
          (UPGRADE_BT_SUCCESS == g_UpgradeStatus || \
          UPGRADE_BT_FAILED  == g_UpgradeStatus))
        {
            UpgradeTaskDestroy();
            g_thread_unref(g_UpgradeThread);
            g_UpgradeThread = NULL;
            g_UpgradeStatus = UPGRADE_INIT;
        }

        if(UPGRADE_MCU_FLAG == (g_upgradeFlag) && \
          (UPGRADE_MCU_SUCCESS == g_UpgradeStatus || \
          UPGRADE_MCU_FAILED  == g_UpgradeStatus))
        {
            UpgradeTaskDestroy();
            g_thread_unref(g_UpgradeThread);
            g_UpgradeThread = NULL;
            g_UpgradeStatus = UPGRADE_INIT;
        }

        if(UPGRADE_MPU_FLAG == (g_upgradeFlag & UPGRADE_MPU_FLAG) && \
           UPGRADE_MPU_FAILED == g_UpgradeStatus)
        {
            UpgradeTaskDestroy();
            g_thread_unref(g_UpgradeThread);
            g_UpgradeThread = NULL;
            g_UpgradeStatus = UPGRADE_INIT;
        }
    }
    IpcSendUpgradeState((UpgradeStatus_E)g_UpgradeStatus);
    pthread_mutex_unlock(&g_upgradeLock);

    return RET_OK;
}

/**
 * @name OtaSwitchAOrBPartition
 * @brief 切换A或B分区
 * 
 * @return int32_t 返回操作的结果状态码。成功时返回0，失败时返回错误码。
 */
int32_t OtaSwitchAOrBPartition()
{
    int32_t retValue = 0;

    LOG_Debug("%s start", __FUNCTION__);
    pthread_mutex_lock(&g_upgradeLock);

    if(UPGRADE_MPU_NEED_BACKUP == g_UpgradeStatus)
    {
        retValue = HalModemSwitch();
    }
    else
    {
        retValue = RET_ERR;
    }
    pthread_mutex_unlock(&g_upgradeLock);

    LOG_Debug("%s retvalue :%d ", __FUNCTION__, retValue);
    return retValue;
}
/**
 * @name OtaSwitchAOrBPartition
 * @brief 切换A或B分区
 * 
 * @return int32_t 返回操作的结果状态码。成功时返回0，失败时返回错误码。
 */
int32_t OtaSwitchAOrBPartition2()
{
    int32_t retValue = 0;

    LOG_Debug("%s start", __FUNCTION__);
    pthread_mutex_lock(&g_upgradeLock);

    retValue = HalModemSwitch();
    
    pthread_mutex_unlock(&g_upgradeLock);

    LOG_Debug("%s retvalue :%d ", __FUNCTION__, retValue);
    return retValue;
}


/**
 * @name OtaSwitchAOrBPartition
 * @brief 系统重启
 * @param 
 *      
 * @return 成功返回0，失败返回小于0，不满足切分区条件。 不能代替OtaSwitchAOrBPartition
 */
int32_t SrTboxSystemReboot(const char *appName, RebootEvet_t event)
{
    LOG_Debug("%s start", __FUNCTION__);
    LOG_Debug("%s  appName:%s rebootEvet:%d", __FUNCTION__, appName, event);

    sleep(2);
    system("reboot");

    return RET_OK;
}

// ==================== 并发升级支持（使用实际函数）====================
// 并发升级上下文结构
typedef struct {
    GThread *thread;
    pthread_mutex_t mutex;
    UpgradeStatus_E status;
    char filePath[UPGRADE_PATH_MAX_LEN];
    int isActive;
    time_t startTime;
} ConcurrentUpgradeCtx_t;

// 独立的MCU和MPU并发上下文
static ConcurrentUpgradeCtx_t g_mcuConcurrentCtx = {
    .thread = NULL,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .status = UPGRADE_INIT,
    .filePath = "",
    .isActive = 0,
    .startTime = 0
};

static ConcurrentUpgradeCtx_t g_mpuConcurrentCtx = {
    .thread = NULL,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .status = UPGRADE_INIT,
    .filePath = "",
    .isActive = 0,
    .startTime = 0
};

static int32_t GetFotainfoStatus(void)
{
    FILE *fp = NULL;
    char cmd[256] = {0};
    char output[64] = {0};
    int status = -1;
    
    snprintf(cmd, sizeof(cmd), "fotainfo --get-stat");
    
    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        LOG_Error("%s popen failed", __FUNCTION__);
        return -1;
    }
    
    if (fgets(output, sizeof(output), fp) != NULL)
    {
        char *start = strchr(output, '(');
        char *end = strchr(output, ')');
        
        if (start != NULL && end != NULL && start < end)
        {
            status = atoi(start + 1);
            LOG_Debug("fotainfo output: %s, parsed status: %d", output, status);
        }
        else
        {
            LOG_Error("%s parse fotainfo output failed: %s", __FUNCTION__, output);
        }
    }
    else
    {
        LOG_Error("%s read fotainfo output failed", __FUNCTION__);
    }
    
    pclose(fp);
    return status;
}

// 前向声明实际的升级函数
extern int32_t UpgradeMcuProcess(const char *filePath);
extern int32_t HalModemUpgradeImpl(const uint8_t *filePath);

// MCU并发升级线程函数
static gpointer McuConcurrentThreadFunc(gpointer data) {
    ConcurrentUpgradeCtx_t *ctx = (ConcurrentUpgradeCtx_t*)data;
    
    LOG_Debug("McuConcurrentThreadFunc started");
    
    pthread_mutex_lock(&ctx->mutex);
    ctx->isActive = 1;
    ctx->status = UPGRADE_MCU_FLASHING;
    ctx->startTime = time(NULL);
    pthread_mutex_unlock(&ctx->mutex);
    
    // 调用实际的MCU升级函数
    int32_t result = UpgradeMcuProcess(ctx->filePath);
    
    pthread_mutex_lock(&ctx->mutex);
    if (result == RET_OK) {
        ctx->status = UPGRADE_MCU_SUCCESS;
    } else {
        ctx->status = UPGRADE_MCU_FAILED;
    }
    ctx->thread = NULL;
    ctx->isActive = 0;
    pthread_mutex_unlock(&ctx->mutex);
    
    LOG_Debug("McuConcurrentThreadFunc finished with result: %d", result);
    return NULL;
}

// MPU并发升级线程函数
static gpointer MpuConcurrentThreadFunc(gpointer data) {
    ConcurrentUpgradeCtx_t *ctx = (ConcurrentUpgradeCtx_t*)data;
    
    LOG_Debug("MpuConcurrentThreadFunc started");
    
    pthread_mutex_lock(&ctx->mutex);
    ctx->isActive = 1;
    ctx->status = UPGRADE_MPU_FLASHING;
    ctx->startTime = time(NULL);
    pthread_mutex_unlock(&ctx->mutex);
    
    // 调用实际的MPU升级函数
    int32_t result = HalModemUpgradeImpl((const uint8_t *)ctx->filePath);
    
    pthread_mutex_lock(&ctx->mutex);
    if (result == RET_OK) {
        ctx->status = UPGRADE_MPU_NEED_BACKUP;
    } else {
        ctx->status = UPGRADE_MPU_FAILED;
    }
    ctx->thread = NULL;
    ctx->isActive = 0;
    pthread_mutex_unlock(&ctx->mutex);
    
    LOG_Debug("MpuConcurrentThreadFunc finished with result: %d", result);
    return NULL;
}

// 并发升级启动接口
int32_t ConcurrentOtaStartUpgrade(const char *filePath, UpgradeFlag_E upgradeFlag) {
    if (!filePath || strlen(filePath) >= UPGRADE_PATH_MAX_LEN) {
        LOG_Error("ConcurrentOtaStartUpgrade: invalid parameters");
        return RET_ERR;
    }
    
    int32_t result = RET_ERR;
    
    // MCU并发升级
    if (upgradeFlag & UPGRADE_MCU_FLAG) {
        pthread_mutex_lock(&g_mcuConcurrentCtx.mutex);
        if (!g_mcuConcurrentCtx.isActive && g_mcuConcurrentCtx.thread == NULL) {
            strncpy(g_mcuConcurrentCtx.filePath, filePath, UPGRADE_PATH_MAX_LEN - 1);
            g_mcuConcurrentCtx.status = UPGRADE_START;
            
            g_mcuConcurrentCtx.thread = g_thread_new("mcu_concurrent", 
                                                    McuConcurrentThreadFunc, 
                                                    &g_mcuConcurrentCtx);
            if (g_mcuConcurrentCtx.thread) {
                result = RET_OK;
                LOG_Debug("Started MCU concurrent upgrade");
            } else {
                LOG_Error("Failed to create MCU concurrent thread");
            }
        } else {
            LOG_Error("MCU concurrent upgrade already running");
        }
        pthread_mutex_unlock(&g_mcuConcurrentCtx.mutex);
    }
    
    // MPU并发升级
    if (upgradeFlag & UPGRADE_MPU_FLAG) {
        pthread_mutex_lock(&g_mpuConcurrentCtx.mutex);
        if (!g_mpuConcurrentCtx.isActive && g_mpuConcurrentCtx.thread == NULL) {
            strncpy(g_mpuConcurrentCtx.filePath, filePath, UPGRADE_PATH_MAX_LEN - 1);
            g_mpuConcurrentCtx.status = UPGRADE_START;
            
            g_mpuConcurrentCtx.thread = g_thread_new("mpu_concurrent",MpuConcurrentThreadFunc,&g_mpuConcurrentCtx);
            if (g_mpuConcurrentCtx.thread) {
                result = RET_OK;
                LOG_Debug("Started MPU concurrent upgrade");
                printf("Started MPU concurrent upgrade\n");
            } else {
                LOG_Error("Failed to create MPU concurrent thread");
                printf("Failed to create MPU concurrent thread\n");
            }
        } else {
            LOG_Error("MPU concurrent upgrade already running");
            printf("MPU concurrent upgrade already running\n");
        }
        pthread_mutex_unlock(&g_mpuConcurrentCtx.mutex);
    }
    
    return result;
}

// 并发升级状态获取接口
int32_t ConcurrentOtaGetUpgradeStatus(const char *filePath, UpgradeStatus_E *status, UpgradeFlag_E flag) {
    if (!filePath || !status) {
        return RET_ERR;
    }
    
    if (flag == UPGRADE_MCU_FLAG) {
        pthread_mutex_lock(&g_mcuConcurrentCtx.mutex);
        *status = g_mcuConcurrentCtx.status;
        if (!g_mcuConcurrentCtx.isActive && g_mcuConcurrentCtx.thread == NULL) {
            *status = UPGRADE_INIT;
        }
        pthread_mutex_unlock(&g_mcuConcurrentCtx.mutex);
        return RET_OK;
    } else if (flag == UPGRADE_MPU_FLAG) {
        pthread_mutex_lock(&g_mpuConcurrentCtx.mutex);
        *status = g_mpuConcurrentCtx.status;
        if (!g_mpuConcurrentCtx.isActive && g_mpuConcurrentCtx.thread == NULL) {
            *status = UPGRADE_INIT;
        }
        pthread_mutex_unlock(&g_mpuConcurrentCtx.mutex);
        return RET_OK;
    }
    
    return RET_ERR;
}

// 并发升级停止接口
int32_t ConcurrentOtaStopUpgrade(UpgradeFlag_E flag) {
    if (flag == UPGRADE_MCU_FLAG) {
        pthread_mutex_lock(&g_mcuConcurrentCtx.mutex);
        if (g_mcuConcurrentCtx.thread) {
            g_mcuConcurrentCtx.isActive = 0;
            g_mcuConcurrentCtx.status = UPGRADE_MCU_FAILED;
            LOG_Debug("Stopping MCU concurrent upgrade");
        }
        pthread_mutex_unlock(&g_mcuConcurrentCtx.mutex);
        return RET_OK;
    } else if (flag == UPGRADE_MPU_FLAG) {
        pthread_mutex_lock(&g_mpuConcurrentCtx.mutex);
        if (g_mpuConcurrentCtx.thread) {
            g_mpuConcurrentCtx.isActive = 0;
            g_mpuConcurrentCtx.status = UPGRADE_MPU_FAILED;
            LOG_Debug("Stopping MPU concurrent upgrade");
        }
        pthread_mutex_unlock(&g_mpuConcurrentCtx.mutex);
        return RET_OK;
    }
    
    return RET_ERR;
}

// 同时获取MCU和MPU升级状态（原子操作）
int32_t ConcurrentOtaGetAllUpgradeStatus(const char *filePath, UpgradeStatus_E *mcuStatus, UpgradeStatus_E *mpuStatus) {
    if (!filePath || !mcuStatus || !mpuStatus) {
        LOG_Error("ConcurrentOtaGetAllUpgradeStatus: invalid parameters");
        return RET_ERR;
    }
    
    pthread_mutex_lock(&g_mcuConcurrentCtx.mutex);
    pthread_mutex_lock(&g_mpuConcurrentCtx.mutex);
    
    *mcuStatus = g_mcuConcurrentCtx.status;
    if (!g_mcuConcurrentCtx.isActive && g_mcuConcurrentCtx.thread == NULL) {
        *mcuStatus = UPGRADE_INIT;
    }
    
    int32_t fotainfoStatus = GetFotainfoStatus();
    LOG_Debug("fotainfo status: %d", fotainfoStatus);
    if (fotainfoStatus >= 0) {
        if(fotainfoStatus == 1)
        {
             *mpuStatus = UPGRADE_MPU_FLASHING;
        }else if(fotainfoStatus == 2)
        {
            *mpuStatus = UPGRADE_MPU_NEED_BACKUP;
        }else if(fotainfoStatus == 3)
        {
            *mpuStatus = UPGRADE_MPU_FAILED;
        }
    } else {
        *mpuStatus = g_mpuConcurrentCtx.status;
        if (!g_mpuConcurrentCtx.isActive && g_mpuConcurrentCtx.thread == NULL) {
            *mpuStatus = UPGRADE_INIT;
        }
    }
    
    pthread_mutex_unlock(&g_mpuConcurrentCtx.mutex);
    pthread_mutex_unlock(&g_mcuConcurrentCtx.mutex);
    
    LOG_Debug("Concurrent get all status - MCU: %d, MPU: %d", *mcuStatus, *mpuStatus);
    return RET_OK;
}
