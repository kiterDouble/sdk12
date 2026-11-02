#ifndef __INDEPENDENT_MCU_UPGRADE_H__
#define __INDEPENDENT_MCU_UPGRADE_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief MCU升级状态枚举
 */
typedef enum {
    MCU_UPGRADE_IDLE = 0,           // 空闲状态
    MCU_UPGRADE_INITIALIZING,       // 初始化中
    MCU_UPGRADE_CHECKING_STATUS,    // 检查MCU状态中
    MCU_UPGRADE_SENDING_DATA,       // 发送升级数据中
    MCU_UPGRADE_PROGRAMMING,        // 编程中
    MCU_UPGRADE_COMPLETE,           // 升级完成
    MCU_UPGRADE_ERROR               // 错误状态
} McuUpgradeStatus_t;

/**
 * @brief MCU升级控制结构体
 */
typedef struct {
    char firmware_path[256];        // 固件路径
    bool need_reboot;               // 是否需要重启
    McuUpgradeStatus_t status;      // 当前状态
    int32_t progress;               // 进度百分比
    int32_t error_code;             // 错误代码
} McuUpgradeCtrl_t;

/**
 * @name IndependentMcuUpgradeInit
 * @brief 初始化独立的MCU升级模块
 * 
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败
 */
int32_t IndependentMcuUpgradeInit(void);

/**
 * @name IndependentMcuUpgradeDeinit
 * @brief 反初始化独立的MCU升级模块
 * 
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败
 */
int32_t IndependentMcuUpgradeDeinit(void);

/**
 * @name StartIndependentMcuUpgrade
 * @brief 开始独立的MCU升级流程
 * 
 * @param ctrl 指向升级控制结构体的指针
 * @return int32_t 返回RET_OK表示成功启动，RET_ERR表示失败
 */
int32_t StartIndependentMcuUpgrade(McuUpgradeCtrl_t *ctrl);

/**
 * @name QueryIndependentMcuUpgradeStatus
 * @brief 查询当前MCU升级状态
 * 
 * @param[out] status 指向状态结构体的指针
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败
 */
int32_t QueryIndependentMcuUpgradeStatus(McuUpgradeCtrl_t *status);

/**
 * @name ControlMcuReboot
 * @brief 控制MCU重启
 * 
 * @param need_reboot 是否需要重启
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败
 */
int32_t ControlMcuReboot(bool need_reboot);

#ifdef __cplusplus
}
#endif

#endif