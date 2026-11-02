#ifndef __HAL_MODEM_PM_H__
#define __HAL_MODEM_PM_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name HalModemPmAutosleepEnable
 * @brief 启用或禁用modem的自动休眠功能。
 * @param enable 一个布尔值，0 表示禁用，1 表示启用。其他值将被视为无效。
 * @return 返回操作的结果状态。RET_OK 表示成功，RET_ERR 表示失败。
 */
int32_t HalModemPmAutosleepEnable(int8_t enable);

/**
 * @name HalModemPmWakeLockCreate
 * @brief 创建Modem电源管理唤醒锁。
 * @param name 唤醒锁的名称，类型为uint8_t数组。
 * @return 回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemPmWakeLockCreate(const uint8_t *name);

/**
 * @name HalModemPmWakeLockLocked
 * @brief 锁定模组电源管理的唤醒锁
 * @param name: 指向要锁定的唤醒锁名称的指针
 * @return : 锁定操作的结果状态码，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemPmWakeLockLocked(const uint8_t *name);

/**
 * @name HalModemPmWakeLockUnlocked
 * @brief Modem电源模式解锁定。
 * @param name 指向要解锁的唤醒锁名称的指针。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemPmWakeLockUnlocked(const uint8_t *name);

/**
 * @name HalModemPmWakeLockLockedEx
 * @brief 独立 wakelock 的锁定接口，每次调用按 name 创建底层 wakelock 并加锁，
 *        与 HalModemPmWakeLockLocked 维护的全局锁互不影响，便于多处独立持锁。
 * @param name 指向唤醒锁名称的指针。
 * @return 成功返回底层 wakelock 的 fd（>= 0），失败返回 -1。返回的 fd 需配对传给
 *         HalModemPmWakeLockUnlockedEx 释放。
 */
int32_t HalModemPmWakeLockLockedEx(const uint8_t *name);

/**
 * @name HalModemPmWakeLockUnlockedEx
 * @brief 独立 wakelock 的解锁接口，与 HalModemPmWakeLockUnlocked 互不影响。
 *        会解锁并销毁该 fd 对应的底层 wakelock。
 * @param fd 由 HalModemPmWakeLockLockedEx 返回的 wakelock fd。
 * @return RET_OK 表示成功，RET_ERR 表示失败。
 */
int32_t HalModemPmWakeLockUnlockedEx(int32_t fd);

/**
 * @name HalModemPmWakeLockDestroy
 * @brief 销毁Modem的电源管理唤醒锁。
 * @param name 指向要销毁的唤醒锁名称的指针。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemPmWakeLockDestroy(const uint8_t *name);

/**
 * @name HalModemForbidInd
 * @brief 设置modem禁止掩码。
 * @param ForbidIndMask 禁止指示掩码。
 * @return 函数执行状态码。返回RET_OK表示成功。
 */
int32_t HalModemForbidInd(uint32_t ForbidIndMask);

void HalModemPmDeepSleepPowerDown(int mode);


#ifdef __cplusplus
}
#endif
#endif
