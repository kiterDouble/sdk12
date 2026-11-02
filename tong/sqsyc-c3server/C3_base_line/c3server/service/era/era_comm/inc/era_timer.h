#ifndef _ECALL_TIMER_H_
#define _ECALL_TIMER_H_

#include <time.h>
#include <signal.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief  InitTimer 
 * @param[in]  timerId       定时器ID
 * @param[in]  notifyFunction  定时器触发时的回调函数
 * @return     RET_OK    成功; RET_ERR   失败
 * @details  初始化定时器
 */
extern int32_t InitTimer(timer_t *timerId,void(*notifyFunction)(union sigval val));

/**
 * @brief  Timer1SecTimerinterval 
 * @param[in]  timerId       定时器ID
 * @return     RET_OK    成功; RET_ERR   失败
 * @details  设置定时器间隔为1秒
 */
extern int32_t Timer1SecTimerinterval(timer_t *timerId);

/**
 * @brief  Timer1SecTimerOneTime 
 * @param[in]  timerId       定时器ID
 * @return     RET_OK    成功; RET_ERR   失败
 * @details  设置定时器为一次触发
 */
extern int32_t Timer1SecTimerOneTime(timer_t *timerId);

/**
 * @brief  TimerXSecTimer 
 * @param[in]  timerId       定时器ID
 * @param[in]  sec           秒
 * @param[in]  interval     间隔
 * @return     RET_OK    成功; RET_ERR   失败
 * @details  设置定时器为x秒
 */
extern int32_t TimerXSecTimer(timer_t *timerId,unsigned sec,int interval);

/**
 * @brief  StopTimer 
 * @param[in]  timerId       定时器ID
 * @return     RET_OK    成功; RET_ERR   失败
 * @details  停止定时器
 */
extern int32_t StopTimer(timer_t *timerId);

/**
 * @brief  TimerDeleteTimer 
 * @param[in]  timerId       定时器ID
 * @return     RET_OK    成功; RET_ERR   失败
 * @details  删除定时器
 */
extern int32_t TimerDeleteTimer(timer_t timerId);

#ifdef __cplusplus
}
#endif

#endif // !_ECALL_TIMER_H_
