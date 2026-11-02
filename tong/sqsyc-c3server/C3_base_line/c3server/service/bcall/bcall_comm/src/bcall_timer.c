#include <stdio.h>
#include <string.h>

#include "sr_log.h"
#include "err_code.h"
#include "bcall_timer.h"

/**
* @brief 初始化定时器
*
* @param timerId 定时器句柄
* @param notifyFunction 定时器触发时调用的函数
* @return int32_t 返回值，如果初始化成功则返回RET_OK，否则返回RET_ERR
*/
int32_t InitTimer(timer_t *timerId,void(*notifyFunction)(union sigval val))
{
    struct sigevent event;

    if(NULL == notifyFunction || NULL == timerId)
    {
        LOG_Error(" %s:Parameter not available ", __func__);
        return RET_ERR;
    }

    memset(&event, 0, sizeof(struct sigevent));

    event.sigev_notify=SIGEV_THREAD;
    event.sigev_notify_function = notifyFunction;
    if(timer_create(CLOCK_MONOTONIC,&event,timerId) == -1)
    {
        LOG_Error(" %s:timer_create is failed ", __func__);
        return RET_ERR;
    }

    LOG_Info(" %s is success ", __func__);
    return RET_OK;
}
/**
* @brief 启动定时器
*
* @param timerId 定时器句柄
* @param it      定时器参数
* @return int32_t 返回值，如果启动成功则返回RET_OK，否则返回RET_ERR
*/
static int32_t StartTimer(timer_t *timerId, struct itimerspec *it)
{
    if (NULL == it || NULL == timerId){
        LOG_Error(" %s:Parameter not available ", __func__);
        return RET_ERR;
    }
    if (timer_settime(*timerId,0,it,NULL) == -1)
    {
        LOG_Error(" %s:timer_settime is failed ", __func__);
        return RET_ERR;
    }
    LOG_Info(" %s is success ", __func__);
    return RET_OK;
}
/**
* @brief 停止定时器
*
* @param timerId 定时器句柄
* @return int32_t 返回值，如果停止成功则返回RET_OK，否则返回RET_ERR
*/
int32_t StopTimer(timer_t *timerId)
{
    struct itimerspec it;

    if (NULL == timerId) {
        LOG_Error(" %s: Parameter not available ", __func__);
        return RET_ERR;
    }

    memset(&it, 0, sizeof(struct itimerspec));

    if (timer_settime(*timerId, 0, &it, NULL) == -1) {
        LOG_Error(" %s: timer_settime is failed ", __func__);
        return RET_ERR;
    }

    LOG_Info(" %s is success ", __func__);
    return RET_OK;
}
/**
* @brief 设置定时器1秒的周期性触发
*
* @param timerId 定时器句柄
* @return int32_t 返回值，如果设置成功则返回RET_OK，否则返回RET_ERR
*/
int32_t Timer1SecTimerinterval(timer_t *timerId)
{
    if(RET_ERR == TimerXSecTimer(timerId,1,1))
    {
        LOG_Error(" %s is failed ", __func__);
        return RET_ERR;
    }

    LOG_Info(" %s is success ", __func__);
    return RET_OK;
}
/**
* @brief 设置定时器1秒的单次触发
*
* @param timerId 定时器句柄
* @return int32_t 返回值，如果设置成功则返回RET_OK，否则返回RET_ERR
*/
int32_t Timer1SecTimerOneTime(timer_t *timerId)
{    
    if(RET_ERR == TimerXSecTimer(timerId,1,0))
    {
        LOG_Error(" %s is failed ", __func__);
        return RET_ERR;
    }

    LOG_Info(" %s is success ", __func__);
    return RET_OK;
}
/**
* @brief 设置定时器X秒的周期性触发
*
* @param timerId 定时器句柄
* @param sec     定时器持续时间（秒）
* @param interval 定时器周期（秒），如果为0，则表示一次性触发
* @return int32_t 返回值，如果设置成功则返回RET_OK，否则返回RET_ERR
*/
int32_t TimerXSecTimer(timer_t *timerId,uint32_t sec,int32_t interval)
{
    struct itimerspec it;

    if (NULL == timerId){
        LOG_Error(" %s:Parameter not available ", __func__);
        return RET_ERR;
    }

    it.it_value.tv_sec = sec;
    it.it_value.tv_nsec = 0;
    if(0 == interval)
    {
    	it.it_interval.tv_sec = 0;
    	it.it_interval.tv_nsec = 0;
    }
    else
    {
    	it.it_interval.tv_sec = sec;
    	it.it_interval.tv_nsec = 0;
    }

    if(RET_ERR == StartTimer(timerId,&it))
    {
        LOG_Error(" %s:StartTimer is failed ", __func__);
        return RET_ERR;
    }

    LOG_Info(" %s is success ", __func__);
    return RET_OK;
}
/**
* @brief 删除定时器
*
* @param timerId 定时器句柄
* @return int32_t 返回值，如果删除成功则返回RET_OK，否则返回RET_ERR
*/
int32_t TimerDeleteTimer(timer_t timerId)
{
    if(-1 == timer_delete(timerId))
    {
        LOG_Error(" %s:timer_delete is failed ", __func__);
        return RET_ERR;
    }

    LOG_Info(" %s is success ", __func__);
    return RET_OK;
}


