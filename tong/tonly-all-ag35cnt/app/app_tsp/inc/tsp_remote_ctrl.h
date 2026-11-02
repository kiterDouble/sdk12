/**
 * @file    tsp_remote_ctrl.h
 * @brief   远控命令处理模块
 *
 * 整体链路分为4步：
 * 1. 连接线程收到远控下行后调用预处理
 * 2. 预处理完成判重、FIFO入队
 * 3. 远控线程逐条取队列，按MCU协议打包并等待ACK/Result
 * 4. 执行结果上报到平台
 */

#ifndef __TSP_REMOTE_CTRL_H__
#define __TSP_REMOTE_CTRL_H__

#include "tsp_common.h"
#include "tsp_protocol.h"
#include "pm_server_api.h"
#include <stdbool.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 前向声明，避免头文件循环依赖 */
typedef struct tly_app tly_app_t;

/* ======================== 数据结构 ======================== */

/** 远控请求完整结构 */
typedef struct {
    uint8_t                 ctrl_cmd;       /* 控制指令 */
    uint8_t                 lock_type;      /* 锁车类型: 0解锁/1锁车/2限扭矩/3激活/4取消激活 */
    uint32_t                timeout;        /* 业务超时时间(毫秒) */
} tly_remote_ctrl_req_t;

/** MCU命令结构 */
typedef struct {
    uint8_t     cmd_data[4];        /* 4字节MCU命令: data[0]=ctrl_cmd, data[1]=cmd_type, data[2]=lock_type, data[3]=预留 */
} tly_mcu_command_t;

/** MCU响应结构 */
typedef struct {
    uint8_t     result;             /* 0=成功, 1=失败 */
    uint8_t     fail_reason;        /* 失败原因 */
} tly_mcu_response_t;

/** 远控执行结果码 */
enum tly_remote_ctrl_result {
    TLY_REMOTE_CTRL_RESULT_SUCCESS   = 0,  /* 成功 */
    TLY_REMOTE_CTRL_RESULT_EXECUTING = 1,  /* 执行中 */
    TLY_REMOTE_CTRL_RESULT_TIMEOUT   = 2,  /* 超时/失败 */
};

/** 执行结果结构 */
typedef struct {
    uint32_t    code;               /* 结果码 */
    uint32_t    fail_reason;        /* 失败原因 */
} tly_process_result_t;

/** 远控命令节点（用于队列） */
typedef struct {
    tly_remote_ctrl_req_t   req;        /* 请求数据 */
    tly_mcu_command_t       mcu_cmd;    /* MCU命令 */
} tly_remote_ctrl_node_t;

/** MCU回调消息缓存节点 */
typedef struct {
    uint8_t     *payload;
    uint32_t    payload_len;
} tly_mcu_queue_msg_t;

/** 远控待执行FIFO缓存 */
typedef struct {
    tly_remote_ctrl_node_t  nodes[25];  /* 25条缓存 */
    uint32_t                count;
    uint32_t                head;
    uint32_t                tail;
    pthread_mutex_t         mutex;
    bool                    inited;
} tly_remote_ctrl_cache_t;

/** 当前执行态快照 */
typedef struct {
    bool                    executing;
    tly_remote_ctrl_req_t   current_req;
    pthread_mutex_t         mutex;
    bool                    inited;
} tly_remote_ctrl_exec_state_t;

/* ======================== 对外接口 ======================== */

/**
 * @brief 上报远控执行结果到平台
 * @param req    远控请求
 * @param result 执行结果
 */
void remote_ctrl_report_result(const tly_remote_ctrl_req_t *req, const tly_process_result_t *result);

/**
 * @brief 初始化远控模块
 * @param app 应用上下文，用于上报执行结果
 * @return 0成功，-1失败
 */
int remote_ctrl_init(tly_app_t *app);

/**
 * @brief 反初始化远控模块
 */
void remote_ctrl_deinit(void);

/**
 * @brief 发送远控命令到MCU
 * @param app   应用上下文
 * @param lock  锁车控制结构体（包含ctrl_cmd, lock_type等）
 */
void send_remote_cmd(tly_app_t *app, const tly_lock_control_t *lock);

/**
 * @brief 设置休眠阻止
 * @description 在远控执行前调用，阻止系统进入休眠，并启动定时器
 */
void remote_ctrl_set_sleep_event(void);

/**
 * @brief 清除休眠阻止
 * @description 在远控执行完成后调用，立即清除休眠阻止
 */
void remote_ctrl_clean_sleep_event(void);

/**
 * @brief 处理电池加热RTC唤醒
 * @param[in] pmData PM电源状态数据
 * @param[in] app 应用上下文
 * @description 当检测到电池加热定时器唤醒时，触发MCU唤醒并发送电池加热命令
 */
void remote_ctrl_handle_battery_heat_wakeup(const MPUPmStateChange_t *pmData, tly_app_t *app);

/**
 * @brief 远控执行线程入口
 * @param data 线程参数
 * @return NULL
 */
gpointer remote_ctrl_exec_thread(gpointer data);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_REMOTE_CTRL_H__ */
