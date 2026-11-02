#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
static pthread_t g_mcuMonitorThread;
static volatile int g_mcuMonitorRun = 0;
static volatile int g_partitionSwitched = 0;

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

/* MCU 升级运行步骤监控线程 */
static void *McuProcessMonitorThread(void *arg)
{
    (void)arg;
    McuProcessStep_E curStep = McuStepGetVern;
    while (g_mcuMonitorRun != 0) {
        curStep = GetMcuProcessStep();
        // step 6: 完成验证后设置 UPGRADE_MCU_FLASH_COMPLETED
        if (curStep == McuStepFlashComplete) {
            LOG_Info("[MCU_MONITOR] MCU flash complete step detected.\n");
            pthread_mutex_lock(&g_upgradeLock);
            g_UpgradeStatus = UPGRADE_MCU_FLASH_COMPLETED;
            pthread_mutex_unlock(&g_upgradeLock);
            LOG_Info("[MCU_MONITOR] Set status: FLASH_COMPLETED\n");
        }
        if (curStep == McuStepRestMcu) {
            LOG_Info("[MCU_MONITOR] MCU reset step detected.\n");
            pthread_mutex_lock(&g_upgradeLock);
            g_UpgradeStatus = UPGRADE_MCU_NEED_RESET;
            pthread_mutex_unlock(&g_upgradeLock);
            LOG_Info("[MCU_MONITOR] Set status: NEED_RESET\n");
            // 在MCU复位前完成MPU分区切换
            if(g_upgradeFlag & UPGRADE_MPU_FLAG)
            {
                LOG_Info("[MCU_MONITOR] Switching partition before MCU reset...\n");
                OtaSwitchAOrBPartition();
                LOG_Info("[MCU_MONITOR] Partition switch done.\n");
            }
            break;
        }
        sleep(1);
    }
    return NULL;
}

/**
 * @name StartMcuProcessMonitor
 * @brief  启动MCU升级运行步骤监控线程
 * @param 无
 * @return int32_t 返回操作结果，操作成功返回RET_OK，失败返回RET_ERR。
 */
static int32_t StartMcuProcessMonitor(void)
{
    g_mcuMonitorRun = 1;
    if (pthread_create(&g_mcuMonitorThread, NULL, McuProcessMonitorThread, NULL) != 0) {
        g_mcuMonitorRun = 0;
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @name StopMcuProcessMonitor
 * @brief  停止MCU运行步骤监控线程
 * @param 无
 * @return 无
 */
static void StopMcuProcessMonitor(void)
{
    g_mcuMonitorRun = 0;
    (void)pthread_join(g_mcuMonitorThread, NULL);
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
    UpgradeFlag_E upgradeFlag = UPGRADE_ALL_FLAG;
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

    if(0 != (upgradeFlag & UPGRADE_MCU_FLAG))
    {
        // 启动MCU升级运行步骤监控线程
        if(StartMcuProcessMonitor() != RET_OK) \
        {
            LOG_Error("%s StartMcuProcessMonitor failed ", __FUNCTION__);
            pthread_mutex_lock(&g_upgradeLock);
            g_UpgradeStatus = UPGRADE_MCU_FAILED;
            pthread_mutex_unlock(&g_upgradeLock);
            g_thread_exit(NULL);
            return NULL;
        }
        pthread_mutex_lock(&g_upgradeLock);
        g_UpgradeStatus = UPGRADE_MCU_FLASHING;
        pthread_mutex_unlock(&g_upgradeLock);
        retValue = UpgradeMcuProcess(g_upgradeFilePath);
        if(retValue < RET_OK)
        {
            LOG_Error("%s upgrade mcu is failed ", __FUNCTION__);
            pthread_mutex_lock(&g_upgradeLock);
            g_UpgradeStatus = UPGRADE_MCU_FAILED;
            pthread_mutex_unlock(&g_upgradeLock);
            g_thread_exit(NULL);
            StopMcuProcessMonitor();
            return NULL;
        }
        else
        {
            LOG_Debug("%s upgrade mcu is success", __FUNCTION__);
            pthread_mutex_lock(&g_upgradeLock);
            g_UpgradeStatus = UPGRADE_MCU_SUCCESS;
            pthread_mutex_unlock(&g_upgradeLock);
            StopMcuProcessMonitor();
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
    if(tmpUpgradeFlag < UPGRADE_MCU_FLAG || tmpUpgradeFlag > UPGRADE_ALL_FLAG)
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
    if(!ipcIsFirstCall() && (tmpUpgradeFlag & UPGRADE_MCU_FLAG))
    {
        LOG_Error("%s have upgrade mcu ,but not init mcu msg, upgrade not allowed ", __FUNCTION__);
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
        if(HAl_UPGRADE_MPU_FLASHING == mpuUpgradeStatus || 
           HAl_UPGRADE_MPU_NEED_BACKUP == mpuUpgradeStatus)
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

        if(HAl_UPGRADE_MPU_BACKUPING == mpuUpgradeStatus)
        {
            g_UpgradeStatus = UPGRADE_MPU_BACKUP;
        }
        *upgradeStatus = g_UpgradeStatus;
    }
    else
    {
        *upgradeStatus = g_UpgradeStatus;

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

        if(UPGRADE_ALL_FLAG == g_upgradeFlag && \
           (UPGRADE_MCU_SUCCESS == g_UpgradeStatus || \
            UPGRADE_MCU_FAILED  == g_UpgradeStatus))
        {
            UpgradeTaskDestroy();
            g_thread_unref(g_UpgradeThread);
            g_UpgradeThread = NULL;
            g_UpgradeStatus = UPGRADE_INIT;
        }
    }

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

    if(g_partitionSwitched)
    {
        LOG_Info("%s already switched by monitor thread, skip", __FUNCTION__);
        return RET_OK;
    }

    pthread_mutex_lock(&g_upgradeLock);

    if(UPGRADE_MPU_NEED_BACKUP == g_UpgradeStatus || UPGRADE_MCU_NEED_RESET == g_UpgradeStatus)
    {
        LOG_Debug("%s HalModemSwitch start", __FUNCTION__);
        retValue = HalModemSwitch();
        LOG_Debug("%s HalModemSwitch end", __FUNCTION__);
        if(retValue == RET_OK)
        {
            g_partitionSwitched = 1;
        }
    }
    else
    {
        retValue = RET_ERR;
    }
    LOG_Debug("%s retvalue :%d ", __FUNCTION__, retValue);
    pthread_mutex_unlock(&g_upgradeLock);
    return retValue;
}

/**
 * @name OtaRestoreAOrBPartition
 * @brief 还原A或B分区
 * @return int32_t 返回操作的结果状态码。成功时返回0，失败时返回错误码。
 */
int32_t OtaRestoreAOrBPartition(void)
{
    int32_t retValue = 0;

    LOG_Debug("%s start", __FUNCTION__);
    pthread_mutex_lock(&g_upgradeLock);

    retValue = HalModemRestore();

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