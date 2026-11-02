#ifndef _OTA_API_H_
#define _OTA_API_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define  UPGRADE_FILE_PAHT      "/oemdata/upgrade/"
#define  UPGRADE_PATH_MAX_LEN   (128)

typedef enum{
    UPGRADE_INIT = 0,
    UPGRADE_START = 1,
    UPGRADE_MCU_FLASHING = 2,
    UPGRADE_MCU_SUCCESS = 3,
    UPGRADE_MCU_FAILED = 4,
    UPGRADE_MPU_FLASHING = 5,
    UPGRADE_MPU_NEED_BACKUP = 6,
    UPGRADE_MPU_BACKUP = 7,
    UPGRADE_MPU_FAILED = 8,
    UPGRADE_MPU_SUCCESS = 9,
    UPGRADE_BT_FLASHING = 10,
    UPGRADE_BT_SUCCESS = 11,
    UPGRADE_BT_FAILED = 12,
    UPGRADE_IS_BUSY = 13
}UpgradeStatus_E;

typedef enum{
    UPGRADE_MCU_FLAG = 1,
    UPGRADE_MPU_FLAG = 2,
    UPGRADE_ALL_FLAG = 3,
    UPGRADE_MPU_MCU_FLAG = 3,
    UPGRADE_BT_FLAG = 4,
    UPGRADE_BT_MCU_FLAG = 5,
    UPGRADE_BT_MPU_FLAG = 6,
    UPGRADE_BT_MCU_MPU_FLAG = 7
}UpgradeFlag_E;


typedef enum
{
    RTC_WAKEUP_REBOOT = 0x01,
    REMOTE_CONTROL_REBOOT = 0x02,
    MAX_END_REBOOT = 0xFF
}RebootEvet_t;

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
int32_t OtaStartUpgrade(const char *filePathName, UpgradeFlag_E upgradeFlag);

/**
 * @name OtaGetUpgradeStatus
 * @brief 获取升级状态
 * @param 
 *      upgradeStatus: 出参 返回当前升级状态。
 * @return 成功返回0，失败返回小于0
 */
int32_t OtaGetUpgradeStatus(const char *filePath, UpgradeStatus_E *upgradeStatus);


/**
 * @name OtaSwitchAOrBPartition
 * @brief 切换分区，调用子函数系统重启切换分区
 * @param 
 *      
 * @return 成功返回0，失败返回小于0，不满足切分区条件。
 */
int32_t OtaSwitchAOrBPartition();

/**
 * @name OtaSwitchAOrBPartition2
 * @brief 切换分区，调用子函数系统重启切换分区
 * @param 
 *      
 * @return 成功返回0，失败返回小于0，不满足切分区条件。
 */
int32_t OtaSwitchAOrBPartition2();
/**
 * @name OtaSwitchAOrBPartition
 * @brief 切换分区，调用子函数系统重启切换分区
 * @param 
 *      
 * @return 成功返回0，失败返回小于0，不满足切分区条件。
 */
int32_t SrTboxSystemReboot(const char *appName, RebootEvet_t event);

// ==================== 并发升级接口 ====================
/**
 * @name ConcurrentOtaStartUpgrade
 * @brief 启动并发升级（与原有接口完全独立）
 * @param filePath: 升级包路径
 * @param upgradeFlag: 升级标志（UPGRADE_MCU_FLAG | UPGRADE_MPU_FLAG）
 * @return 成功返回0，失败返回小于0
 */
int32_t ConcurrentOtaStartUpgrade(const char *filePath, UpgradeFlag_E upgradeFlag);

/**
 * @name ConcurrentOtaGetUpgradeStatus
 * @brief 获取并发升级状态
 * @param filePath: 升级包路径
 * @param status: 输出状态
 * @param flag: 模块标识（UPGRADE_MCU_FLAG 或 UPGRADE_MPU_FLAG）
 * @return 成功返回0，失败返回小于0
 */
int32_t ConcurrentOtaGetUpgradeStatus(const char *filePath, UpgradeStatus_E *status, UpgradeFlag_E flag);

/**
 * @name ConcurrentOtaGetAllUpgradeStatus
 * @brief 同时获取MCU和MPU升级状态（原子操作，线程安全）
 * @param filePath: 升级包路径
 * @param mcuStatus: MCU升级状态输出
 * @param mpuStatus: MPU升级状态输出  
 * @return 成功返回0，失败返回小于0
 */
int32_t ConcurrentOtaGetAllUpgradeStatus(const char *filePath, UpgradeStatus_E *mcuStatus, UpgradeStatus_E *mpuStatus);

/**
 * @name ConcurrentOtaStopUpgrade
 * @brief 停止并发升级
 * @param flag: 模块标识
 * @return 成功返回0，失败返回小于0
 */
int32_t ConcurrentOtaStopUpgrade(UpgradeFlag_E flag);

#ifdef __cplusplus
}
#endif
#endif // _OTA_API_H_


