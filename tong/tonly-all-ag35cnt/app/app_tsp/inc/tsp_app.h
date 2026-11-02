/**
 * tsp_app.h - 应用主控与状态机
 *
 * 管理应用生命周期、状态机、心跳、数据包分发
 */

#ifndef __TSP_APP_H__
#define __TSP_APP_H__

#include "tsp_common.h"
#include "tsp_config.h"
#include "tsp_tcp.h"
#include "tsp_http.h"
#include "tsp_recorder.h"
#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== 应用状态 ======================== */

typedef enum tly_app_state {
    TLY_STATE_INIT          = 0,  /* 初始化 */
    TLY_STATE_TCP_CONNECTING,     /* TCP连接中 */
    TLY_STATE_CONNECTED,          /* TCP已连接 */
    TLY_STATE_RUNNING,            /* 正常运行 */
    TLY_STATE_RECONNECTING,       /* 断线重连中 */
    TLY_STATE_STOPPING,           /* 正在停止 */
} tly_app_state_t;

/* ======================== 应用上下文 ======================== */

typedef struct tly_app {
    struct event_base *evbase;
    tly_config_t       config;
    tly_tcp_conn_t    *tcp;
    tly_app_state_t    state;

    struct event      *ev_heartbeat;    /* 心跳定时器 */
    struct event      *ev_report;       /* 实时信息上报定时器 */
    struct event      *ev_sigterm;      /* SIGTERM处理 */
    struct event      *ev_sigint;       /* SIGINT处理 */

    /* 统计 */
    uint32_t           pkt_recv_count;  /* 收包计数 */
    uint32_t           pkt_send_count;  /* 发包计数 */
    uint32_t           err_count;       /* 错误计数 */
    uint16_t           login_seq;       /* 登入流水号 */
    uint16_t           version_seq;     /* 版本号流水号 */
    uint16_t           alarm_seq;       /* 拆除报警流水号 */
    int                logged_in;       /* 是否已登入 */
    int                canid_received;  /* 是否已收到CANID列表 */

    /* CANID缓存 */
    tly_canid_list_resp_t canid_cache;  /* 平台下发的CANID列表 */

    /* 离线补发 */
    int                reissue_sending; /* 正在补发中 */
    struct event      *ev_reissue;      /* 补发定时器 */

    /* 电池加热RTC闹钟ID */
    uint8_t            battery_heat_alarm_id; /* 0表示未创建 */
    uint8_t            battery_heat_alarm_on; /* 1表示已接收到远控 */
    uint32_t           battery_heat_alarm_time_second; /* RTC闹钟时间 */
    uint8_t            battery_heat_alarm_arrived; /* 1表示已接收到MCU唤醒事件 */ 

    /* TCP长连接RTC闹钟ID */
    uint8_t            tcp_keepalive_alarm_id; /* 0表示未创建 */
    uint8_t            tcp_keepalive_alarm_arrived; /* 1表示已接收到MCU唤醒事件 */

    /* CAN录制 */
    tly_recorder_t     recorder;
    /* 故障检测定时器 */
    struct event      *ev_fault_check;  /* 故障检测定时器 */
    struct event      *ev_remove_alarm_check;  /* 拆车报警检测定时器 */

    /* 拆车报警检测状态 */
    int                remove_alarm_detected;   /* 是否已触发 */
    int                remove_alarm_confirm_cnt; /* KL30低电压连续检测计数 */
} tly_app_t;

/* ======================== 生命周期 ======================== */

/**
 * 初始化应用
 * @param app       应用上下文
 * @param conf_file 配置文件路径
 * @return 0成功, 负数失败
 */
int  tly_app_init(tly_app_t *app, const char *conf_file);

/**
 * 启动TCP连接并进入事件循环(阻塞)
 * @return 0正常退出, 负数异常
 */
int  tly_app_run(tly_app_t *app);

/**
 * 请求停止应用(可从信号处理或外部调用)
 */
void tly_app_stop(tly_app_t *app);

/**
 * 销毁应用, 释放所有资源
 */
void tly_app_destroy(tly_app_t *app);

/* ======================== 数据包处理 ======================== */

/**
 * 分发收到的数据包(按cmd_id路由到处理函数)
 */
void tly_app_dispatch_packet(tly_app_t *app, const tly_packet_t *pkt);

/* ======================== 便捷API: 发送心跳 ======================== */

/**
 * 发送心跳包(0xC1)
 */
int  tly_app_send_heartbeat(tly_app_t *app);

/**
 * 发送通用应答
 * @param cmd_id    原始命令标识
 * @param resp_flag 应答标志
 * @param data      应答数据(可为NULL)
 * @param data_len  应答数据长度
 */
int  tly_app_send_response(tly_app_t *app, uint8_t cmd_id,
                           uint8_t resp_flag,
                           const uint8_t *data, uint32_t data_len);

/**
 * 发送拆除报警(0x05), 上行: 终端→平台
 * @param status    定位状态位 (bit0:有效/无效, bit1:东西经, bit2:南北纬, bit3:连接/断开)
 * @param longitude 经度 ×10^6
 * @param latitude  纬度 ×10^6
 */
int  tly_app_send_remove_alarm(tly_app_t *app,
                               uint8_t status,
                               uint32_t longitude,
                               uint32_t latitude);

/**
 * 发送车辆登出(0x04), 上行: 终端→平台
 * 通常在终端关机或主动断开前调用
 */
int  tly_app_send_vehicle_logout(tly_app_t *app);

/**
 * 发送TCU版本号(0x74), 上行: 终端→平台
 * @param version    版本号原始报文数据
 * @param version_len 版本号数据长度
 * @note 数据格式: 主版本号n字节，读取版本号原始报文直接转发后台
 */
int  tly_app_send_tcu_version(tly_app_t *app,
                               const uint8_t *version,
                               uint32_t version_len);

/**
 * @brief 发送MCU软件版本号(打包多个MCU类型)
 * @param app 应用上下文
 * @param mcu_ver MCU版本数据(含多个MCU类型的版本项)
 * @note 命令码 0x75; 数据格式: 数量 + [mcu_type + 版本10字节] × N
 *       读取失败的MCU用默认值"2002"填充
 */
int  tly_app_send_mcu_version(tly_app_t *app,
                              const tly_mcu_version_t *mcu_ver);

/**
 * 请求VIN码(0xFA), 上行: 终端→平台
 * 数据内容为空, 平台将返回VIN码
 */
int  tly_app_get_vin(tly_app_t *app);

/* ======================== 电池加热闹钟 ======================== */

/**
 * 删除电池加热RTC闹钟
 * @param app 应用上下文
 */
void tly_app_delete_battery_heat_alarm(tly_app_t *app);

/**
 * 设置电池加热RTC闹钟
 * @param app 应用上下文
 * @return 0成功, 负数失败
 */
int tly_app_set_battery_heat_alarm(tly_app_t *app);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_APP_H__ */
