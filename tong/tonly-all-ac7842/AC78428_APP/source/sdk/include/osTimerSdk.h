#ifndef OS_TIMER_SDK_H
#define OS_TIMER_SDK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 返回值约定（与原先 TimerHalIsTimeout 类似） */
#define OS_TIMER_SDK_RET_OK            (0)
#define OS_TIMER_SDK_RET_ERR           (-1)
#define OS_TIMER_SDK_RET_STOPPED       (-2)
#define OS_TIMER_SDK_RET_RUNNING       (1)
#define OS_TIMER_SDK_RET_TIMEOUT       (0)

/* 句柄申请/释放 */
int16_t OsTimerSdkOpen(void);
int16_t OsTimerSdkClose(int16_t timerHandle);

/* 启动/停止（Start 携带 ms） */
int16_t OsTimerSdkStartTime(int16_t timerHandle, uint32_t time_ms);
int16_t OsTimerSdkStopTime(int16_t timerHandle);

/* 查询是否超时
 *  -1: 参数非法
 *  -2: 定时器已停止
 *   1: 正在计时且未到期
 *   0: 已到期
 */
int16_t OsTimerSdkIsTimeout(int16_t timerHandle);

#ifdef __cplusplus
}
#endif

#endif /* OS_TIMER_SDK_H */
