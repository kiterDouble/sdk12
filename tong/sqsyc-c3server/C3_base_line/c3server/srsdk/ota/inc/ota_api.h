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
    UPGRADE_MCU_NEED_RESET = 3,
    UPGRADE_MCU_SUCCESS = 4,
    UPGRADE_MCU_FAILED = 5,
    UPGRADE_MPU_FLASHING = 6,
    UPGRADE_MPU_NEED_BACKUP = 7,
    UPGRADE_MPU_BACKUP = 8,
    UPGRADE_MPU_FAILED = 9,
    UPGRADE_MPU_SUCCESS = 10,
    UPGRADE_IS_BUSY = 11,
    UPGRADE_MCU_FLASH_COMPLETED = 12
}UpgradeStatus_E;

typedef enum{
    UPGRADE_MCU_FLAG = 1,
    UPGRADE_MPU_FLAG = 2,
    UPGRADE_ALL_FLAG = 3
}UpgradeFlag_E;


typedef enum
{
    RTC_WAKEUP_REBOOT = 0x01,
    REMOTE_CONTROL_REBOOT = 0x02,
    OTA_MASTER_REBOOT = 0x03,
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
 * @name OtaRestoreAOrBPartition
 * @brief 还原A或B分区
 * @return int32_t 返回操作的结果状态码。成功时返回0，失败时返回错误码。
 */
int32_t OtaRestoreAOrBPartition(void);

/**
 * @name OtaSwitchAOrBPartition
 * @brief 切换分区，调用子函数系统重启切换分区
 * @param 
 *      
 * @return 成功返回0，失败返回小于0，不满足切分区条件。
 */
int32_t SrTboxSystemReboot(const char *appName, RebootEvet_t event);

#ifdef __cplusplus
}
#endif

#endif //_OTA_API_H_

