#ifndef __LOG_PM_H__
#define __LOG_PM_H__

#include <stdint.h>
#include "log_pack.h"

/**
 * @brief 原子读取 PM 状态
 *
 * @param ctx 日志服务器上下文
 * @return 当前 PM 状态
 */
int32_t PmStateGet(LogSrCtx_t* ctx);

/**
 * @brief 原子设置 PM 状态
 *
 * @param ctx 日志服务器上下文
 * @param state 新的 PM 状态
 */
void PmStateSet(LogSrCtx_t* ctx, int32_t state);

/**
 * @brief 加锁重启 wakeUpTimer（唤醒时调用）
 *
 * @param ctx 日志服务器上下文
 */
void PmWakeUpTimerRestart(LogSrCtx_t* ctx);

/**
 * @brief 加锁读取 bootUpTimer 已用时间
 *
 * @param ctx 日志服务器上下文
 * @return 已用秒数；timer 未创建或参数非法时返回 999
 */
gdouble PmBootUpTimerElapsed(LogSrCtx_t* ctx);

/**
 * @brief 加锁读取 wakeUpTimer 已用时间
 *
 * @param ctx 日志服务器上下文
 * @return 已用秒数；timer 未创建或参数非法时返回 999
 */
gdouble PmWakeUpTimerElapsed(LogSrCtx_t* ctx);

/**
 * @brief 原子读取 needReconcile 标志
 *
 * @param ctx 日志服务器上下文
 * @return 当前标志值
 */
gint PmNeedReconcileGet(LogSrCtx_t* ctx);

/**
 * @brief 原子设置 needReconcile 标志
 *
 * @param ctx 日志服务器上下文
 * @param value 新的标志值
 */
void PmNeedReconcileSet(LogSrCtx_t* ctx, gint value);

/**
 * @brief 订阅电源状态变化通知
 *
 * @param data 用户上下文（通常为 LogSrCtx_t*）
 * @return 0 成功，其他失败
 */
int32_t LogPmStateMonitorInit(void* data);

/**
 * @brief 取消订阅电源状态变化通知
 *
 * @param data 用户上下文（当前未使用，保持接口对称）
 * @return 0 成功，其他失败
 */
int32_t LogPmStateMonitorDeinit(void* data);

#endif /* __LOG_PM_H__ */
