#ifndef __HAL_MODEM_UPGRADE_H__
#define __HAL_MODEM_UPGRADE_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define UPGRADE_SCRIPT_PATH_NAME        "/oemapp/scripts/start_upgrade_mpu.sh"
#define UPGRADE_STA_SCRIPT_PATH_NAME    "/oemapp/scripts/get_upgrade_status.sh"
#define UPGRADE_LOG_NAME                "upgrade_mpu.log"

typedef enum{
    HAl_UPGRADE_MPU_OK,
    HAl_UPGRADE_MPU_FLASHING,
    HAl_UPGRADE_MPU_NEED_BACKUP,
    HAl_UPGRADE_MPU_FAILED,
    HAl_UPGRADE_MPU_BACKUPING,
    HAl_UPGRADE_MPU_NO_PATH
}UpgradeMpuStatus_E;

/**
 * @name HalModemUpgradeImpl
 * @brief 模组fota升级执行
 * @param filePath:输入函数, 升级文件路径
 * @return 成功返回2, 其他或小于0 执行升级脚本失败
 *         cevb 0:刷写成功 1:参数个数不正确 2:不满足升级条件 3:指定升级路径不成功 4:指定打印时间不成功 5:执行升级指令不成功 6:升级失败 7:升级状态错乱 
 *         cet 0:刷写成功 1:升级初始化失败 2:升级包不存在 3:下载段数据失败 4:分区状态错误
 */
int32_t HalModemUpgradeImpl(const uint8_t *filePath);



/**
 * @name HalModemGetUpgradeStatus
 * @brief 获取模组升级状态
 * @param filePath 指向待检查文件路径的指针。该文件路径不应为空，且长度不应超过512个字符。
 * @return 0:成功 1:升级中  2:需备份  3:升级失败  4:备份中  5:指令中缺少升级包路径
 */
int32_t HalModemGetUpgradeStatus(const uint8_t *filePath);

/**
 * @name HalModemSwitch
 * @brief 切换Modem分区。
 * @param null
 * @return 返回操作的结果状态码。成功时返回RET_OK，失败时返回RET_ERR。
 */
int32_t HalModemSwitch(void);

/**
 * @name HalModemRestore
 * @brief 还原Modem分区。
 * @return 返回操作的结果状态码。成功时返回RET_OK，失败时返回RET_ERR。
 */
int32_t HalModemRestore(void);
#ifdef __cplusplus
}
#endif

#endif