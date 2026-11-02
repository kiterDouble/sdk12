/*
 * FreeRTOS Kernel V10.6.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */


#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

#if (defined(__ARMCC_VERSION) || defined(__GNUC__) || defined(__ICCARM__))
    #include <stdint.h>
#endif

#define DEV_ASSERT                            configASSERT
#define AC7840X_CORE_CLOCK                    (120000000UL)

/* Constants that describe the hardware and memory usage. */
#define configCPU_CLOCK_HZ                    AC7840X_CORE_CLOCK    /* CPU频率 */
#define configTICK_RATE_HZ                    ((TickType_t)1000)    /* 时钟节拍频率,这里设置为1000,周期就是1ms */
#define configTOTAL_HEAP_SIZE                 ((size_t)10*1024)     /* 系统所有总的堆大小 */
#define configMINIMAL_STACK_SIZE              ((uint16_t)128)       /* 空闲任务使用的堆栈大小 */
#define configSUPPORT_DYNAMIC_ALLOCATION      1                     /* 支持动态内存申请 */
#define configSUPPORT_STATIC_ALLOCATION       1                     /* 静态内存申请 */

/* Constants related to the behaviour or the scheduler. */
#define configMAX_PRIORITIES                  20    /* 可使用的最大优先级 */
#define configUSE_PREEMPTION                  1     /* 1使用抢占式内核,0使用协程 */
#define configUSE_TIME_SLICING                1
#define configIDLE_SHOULD_YIELD               0
#define configMAX_TASK_NAME_LEN               (10)  /* 任务名字字符串长度 */
#define configUSE_16_BIT_TICKS                0     /* 系统节拍计数器变量数据类型 */
                                                    /* 1表示为16位无符号整形,0表示为32位无符号整形 */

/* Software timer definitions. */
#define configUSE_TIMERS                      1     /* 为1时启用软件定时器 */
#define configTIMER_TASK_PRIORITY             2     /* 软件定时器优先级 */
#define configTIMER_QUEUE_LENGTH              5     /* 不为0时表示启用队列记录,具体的值是可以记录的队列和信号量最大数目 */
#define configTIMER_TASK_STACK_DEPTH          (configMINIMAL_STACK_SIZE * 2)  /* 软件定时器任务堆栈大小 */

/* Constants that build features in or out. */
#define configUSE_MUTEXES                     1     /* 为1时使用互斥信号量 */
#define configUSE_RECURSIVE_MUTEXES           1     /* 为1时使用递归互斥信号量 */
#define configUSE_COUNTING_SEMAPHORES         1     /* 为1时使用计数信号量 */
#define configUSE_QUEUE_SETS                  1     /* 将其设置为 1,包含队列集合功能(可以阻塞或挂起多个队列和信号量)或设置为0,忽略队列集合功能 */
#define configUSE_TASK_NOTIFICATIONS          1     /* 使能任务间直接的消息传递,包含信号量,事件标志组和消息邮箱 */
#define configUSE_TRACE_FACILITY              1
#define configUSE_STATS_FORMATTING_FUNCTIONS  1 
#define configUSE_TICKLESS_IDLE               0     /* Tickless低功耗模式 */
#define configUSE_APPLICATION_TASK_TAG        0     /* 使能此配置将添加额外的结构体成员和函数,以此来协助可视化和跟踪,在使用IAR中的FreeRTOS插件时要使能这个配置,否则无法显示任务栈的使用情况 */
#define configUSE_NEWLIB_REENTRANT            0     /* 每个任务创建的时候将分配Newlib的重入结构体 */
#define configUSE_CO_ROUTINES                 0     /* 将其设置为 1,将在构建中包含协程功能 */

/* Constants provided for debugging and optimisation assistance. */
#define configCHECK_FOR_STACK_OVERFLOW        0     /* 堆栈溢出检测 */
#define configQUEUE_REGISTRY_SIZE             0     /* 通过此定义来设置可以注册的信号量和消息队列个数(调试用) */
// #define configASSERT( x )                     if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* Constants that define which hook (callback) functions should be used. */
#define configUSE_IDLE_HOOK                   1
#define configUSE_TICK_HOOK                   0
#define configUSE_DAEMON_TASK_STARTUP_HOOK    0
#define configUSE_MALLOC_FAILED_HOOK          0

/* Port specific configuration. */
#define configENABLE_MPU                      0
#define configENABLE_FPU                      1
#define configENABLE_MVE                      0
#define configENABLE_TRUSTZONE                1
#define configMINIMAL_SECURE_STACK_SIZE       ((uint32_t)1024)
#define configRUN_FREERTOS_SECURE_ONLY        0

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
    /* __NVIC_PRIO_BITS will be specified when CMSIS is being used. */
    #define configPRIO_BITS                     __NVIC_PRIO_BITS
#else
    /* 7 priority levels */
    #define configPRIO_BITS                     4
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority" function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY       0x0F

/* The highest interrupt priority that can be used by any interrupt service
 * routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT
 * CALL INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A
 * HIGHER PRIORITY THAN THIS! (higher priorities are lower numeric values). */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY  2

/* Interrupt priorities used by the kernel port layer itself.  These are generic
 * to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY               (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
 * See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY          (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Set the following definitions to 1 to include the API function, or zero
 * to exclude the API function.  NOTE:  Setting an INCLUDE_ parameter to 0 is
 * only necessary if the linker does not automatically remove functions that are
 * not referenced anyway. */
#define INCLUDE_vTaskPrioritySet              1
#define INCLUDE_uxTaskPriorityGet             1
#define INCLUDE_vTaskDelete                   1
#define INCLUDE_vTaskSuspend                  1
#define INCLUDE_xTaskDelayUntil               1
#define INCLUDE_vTaskDelay                    1
#define INCLUDE_xTaskGetIdleTaskHandle        1
#define INCLUDE_xTaskAbortDelay               1
#define INCLUDE_xQueueGetMutexHolder          1
#define INCLUDE_xSemaphoreGetMutexHolder      1
#define INCLUDE_xTaskGetHandle                1
#define INCLUDE_uxTaskGetStackHighWaterMark   1
#define INCLUDE_uxTaskGetStackHighWaterMark2  1
#define INCLUDE_eTaskGetState                 1
#define INCLUDE_xTaskResumeFromISR            1
#define INCLUDE_xTimerPendFunctionCall        1
#define INCLUDE_xTaskGetSchedulerState        1
#define INCLUDE_xTaskGetCurrentTaskHandle     1

/* Map the FreeRTOS port interrupt handlers to their CMSIS standard names. */
#define xPortPendSVHandler                    PendSV_Handler
#define vPortSVCHandler                       SVC_Handler
#define xPortSysTickHandler                   SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
