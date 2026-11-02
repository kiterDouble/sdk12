/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: osTimerSdk.c
 Author: 
 Created Time: 
 Description: 
 Others: 
 *************************************************/
/****************************** include ***************************************/
#include "osTimerSdk.h"
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

/****************************** Macro Definitions ******************************/
#ifndef OS_TIMER_SDK_INSTANCE_NUMBER
#define OS_TIMER_SDK_INSTANCE_NUMBER        (32)
#endif

/****************************** Type Definitions ******************************/
typedef struct
{
    uint8_t    useFlag;       /* 0: free, 1: used */
    uint8_t    onFlag;        /* 0: stopped, 1: running */
    TickType_t expiryTick;    /* absolute tick when timeout */
    uint32_t   periodMs;      /* for debug/trace only */
} OsTimerSdkItem_t;

/****************************** Global Variables ******************************/
static OsTimerSdkItem_t g_osTimerSdk[OS_TIMER_SDK_INSTANCE_NUMBER];

/****************************** Function Declarations *************************/
static bool OsTimerSdkIsExpired(TickType_t now, TickType_t expiry);
static bool OsTimerSdkHandleValid(int16_t h);

/****************************** Private Function Implementations ***************/
/*************************************************
  Function:       OsTimerSdkIsExpired
  Description:    溢出安全的定时器过期判断
  Input:          now: 当前时间戳
                  expiry: 过期时间戳
  Output:         None
  Return:         true: 定时器已过期
                  false: 定时器未过期
  Others:         使用无符号整数比较确保在tick值溢出情况下仍能正确判断
 *************************************************/
static bool OsTimerSdkIsExpired(TickType_t now, TickType_t expiry)
{
    return ((TickType_t)(now - expiry) < (TickType_t)0x80000000UL);
}

/*************************************************
  Function:       OsTimerSdkHandleValid
  Description:    验证定时器句柄是否有效
  Input:          h: 要验证的定时器句柄
  Output:         None
  Return:         true: 句柄有效
                  false: 句柄无效
  Others:         通过检查句柄是否在[0, OS_TIMER_SDK_INSTANCE_NUMBER)范围内验证有效性
 *************************************************/
static bool OsTimerSdkHandleValid(int16_t h)
{
    return ((h >= 0) && (h < (int16_t)OS_TIMER_SDK_INSTANCE_NUMBER));
}

/****************************** Public Function Implementations ***************/
/*************************************************
  Function:       OsTimerSdkOpen
  Description:    打开一个定时器实例
  Input:          None
  Output:         None
  Return:         成功返回定时器句柄，失败返回-1
  Others:         在临界区中操作，寻找第一个未使用的定时器实例
 *************************************************/
int16_t OsTimerSdkOpen(void)
{
    int16_t handle = -1;
    int16_t i;

    taskENTER_CRITICAL();
    for (i = 0; i < (int16_t)OS_TIMER_SDK_INSTANCE_NUMBER; i++)
    {
        if (g_osTimerSdk[i].useFlag == 0U)
        {
            g_osTimerSdk[i].useFlag   = 1U;
            g_osTimerSdk[i].onFlag    = 0U;
            g_osTimerSdk[i].expiryTick = (TickType_t)0U;
            g_osTimerSdk[i].periodMs  = 0U;
            handle = i;
            break;
        }
    }
    taskEXIT_CRITICAL();

    return handle;
}

/*************************************************
  Function:       OsTimerSdkClose
  Description:    关闭一个定时器实例
  Input:          timerHandle - 定时器句柄
  Output:         None
  Return:         OS_TIMER_SDK_RET_OK 成功，OS_TIMER_SDK_RET_ERR 失败
  Others:         在临界区中操作，重置定时器的所有状态
 *************************************************/
int16_t OsTimerSdkClose(int16_t timerHandle)
{
    if (!OsTimerSdkHandleValid(timerHandle))
    {
        return OS_TIMER_SDK_RET_ERR;
    }

    taskENTER_CRITICAL();
    g_osTimerSdk[timerHandle].useFlag    = 0U;
    g_osTimerSdk[timerHandle].onFlag     = 0U;
    g_osTimerSdk[timerHandle].expiryTick = (TickType_t)0U;
    g_osTimerSdk[timerHandle].periodMs   = 0U;
    taskEXIT_CRITICAL();

    return OS_TIMER_SDK_RET_OK;
}

/*************************************************
  Function:       OsTimerSdkStartTime
  Description:    启动一个定时器
  Input:          timerHandle - 定时器句柄
                  time_ms - 定时时间（毫秒）
  Output:         None
  Return:         OS_TIMER_SDK_RET_OK 成功，OS_TIMER_SDK_RET_ERR 失败
  Others:         将毫秒转换为tick，确保至少1个tick，设置到期时间和启动标志
 *************************************************/
int16_t OsTimerSdkStartTime(int16_t timerHandle, uint32_t time_ms)
{
    TickType_t now;
    TickType_t ticks;

    if (!OsTimerSdkHandleValid(timerHandle))
    {
        return OS_TIMER_SDK_RET_ERR;
    }

    /* 句柄未 open */
    if (g_osTimerSdk[timerHandle].useFlag == 0U)
    {
        return OS_TIMER_SDK_RET_ERR;
    }

    /* ms -> tick（至少 1 tick，避免被截断成 0） */
    ticks = pdMS_TO_TICKS(time_ms);
    if ((time_ms != 0U) && (ticks == (TickType_t)0U))
    {
        ticks = (TickType_t)1U;
    }

    now = xTaskGetTickCount();

    taskENTER_CRITICAL();
    g_osTimerSdk[timerHandle].expiryTick = now + ticks; /* Start 时装载到期时间 */
    g_osTimerSdk[timerHandle].onFlag     = 1U;
    g_osTimerSdk[timerHandle].periodMs   = time_ms;
    taskEXIT_CRITICAL();

    return OS_TIMER_SDK_RET_OK;
}

/*************************************************
  Function:       OsTimerSdkStopTime
  Description:    停止一个定时器
  Input:          timerHandle - 定时器句柄
  Output:         None
  Return:         OS_TIMER_SDK_RET_OK 成功，OS_TIMER_SDK_RET_ERR 失败
  Others:         在临界区中操作，清除定时器的启动标志
 *************************************************/
int16_t OsTimerSdkStopTime(int16_t timerHandle)
{
    if (!OsTimerSdkHandleValid(timerHandle))
    {
        return OS_TIMER_SDK_RET_ERR;
    }

    if (g_osTimerSdk[timerHandle].useFlag == 0U)
    {
        return OS_TIMER_SDK_RET_ERR;
    }

    taskENTER_CRITICAL();
    g_osTimerSdk[timerHandle].onFlag = 0U;
    taskEXIT_CRITICAL();

    return OS_TIMER_SDK_RET_OK;
}

/*************************************************
  Function:       OsTimerSdkIsTimeout
  Description:    检查定时器是否超时
  Input:          timerHandle - 定时器句柄
  Output:         None
  Return:         OS_TIMER_SDK_RET_OK 超时，OS_TIMER_SDK_RET_ERR 错误，
                  其他值表示未超时
  Others:         在临界区中获取定时器状态，使用溢出安全的比较方法
 *************************************************/
int16_t OsTimerSdkIsTimeout(int16_t timerHandle)
{
    uint8_t on;
    TickType_t expiry;
    TickType_t now;

    if (!OsTimerSdkHandleValid(timerHandle))
    {
        return OS_TIMER_SDK_RET_ERR;
    }

    if (g_osTimerSdk[timerHandle].useFlag == 0U)
    {
        return OS_TIMER_SDK_RET_ERR;
    }

    taskENTER_CRITICAL();
    on     = g_osTimerSdk[timerHandle].onFlag;
    expiry = g_osTimerSdk[timerHandle].expiryTick;
    taskEXIT_CRITICAL();

    if (on == 0U)
    {
        return OS_TIMER_SDK_RET_STOPPED;
    }

    now = xTaskGetTickCount();

    if (OsTimerSdkIsExpired(now, expiry))
    {
        return OS_TIMER_SDK_RET_TIMEOUT; /* 0 */
    }

    return OS_TIMER_SDK_RET_RUNNING;     /* 1 */
}
