/**
 * @file param_config.h
 * @brief 参数ID枚举定义（示例）
 * @description 用户可根据实际需求修改此文件定义项目参数
 */

#ifndef __PARAM_CONFIG_H__
#define __PARAM_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "param_types.h"

/**
 * 参数ID定义
 * 
 * 范围说明:
 * - 0x0000 ~ 0x2FFF: 状态参数（持久化到INI，只读或运行时写入）
 * - 0x3000 ~ 0x3FFF: 控制参数（内存存储，读后清零）
 * - 0x4000 ~ 0x7FFF: 配置参数（持久化到INI，可配置）
 */

typedef enum {
    /* ========== 状态参数 (0x0000 ~ 0x2FFF) ========== */
    
    /* 系统状态 */
    PARAM_ID_SYS_STATUS             = 0x0001,   /* 系统运行状态 */
    PARAM_ID_SYS_UPTIME             = 0x0002,   /* 系统运行时间 */
    PARAM_ID_SYS_ERROR_CODE         = 0x0003,   /* 系统错误码 */
    
    /* 网络状态 */
    PARAM_ID_NET_LINK_STATUS        = 0x0010,   /* 网络连接状态 */
    PARAM_ID_NET_SIGNAL_STRENGTH    = 0x0011,   /* 信号强度 */
    PARAM_ID_NET_IP_ADDR            = 0x0012,   /* IP地址 */
    
    /* 设备状态 */
    PARAM_ID_DEV_TEMPERATURE        = 0x0020,   /* 设备温度 */
    PARAM_ID_DEV_VOLTAGE            = 0x0021,   /* 设备电压 */
    PARAM_ID_DEV_BATTERY_LEVEL      = 0x0022,   /* 电池电量 */
    
    /* ========== 控制参数 (0x3000 ~ 0x3FFF) ========== */
    /* 注意: 控制参数读后自动清零 */
    
    PARAM_ID_CTRL_REBOOT            = 0x3001,   /* 重启命令 */
    PARAM_ID_CTRL_RESET_CONFIG      = 0x3002,   /* 恢复出厂设置 */
    PARAM_ID_CTRL_CLEAR_DATA        = 0x3003,   /* 清除数据 */
    PARAM_ID_CTRL_ENTER_SLEEP       = 0x3004,   /* 进入休眠 */
    PARAM_ID_CTRL_WAKEUP            = 0x3005,   /* 唤醒设备 */
    PARAM_ID_CTRL_LED_ON            = 0x3006,   /* 开指示灯 */
    PARAM_ID_CTRL_LED_OFF           = 0x3007,   /* 关指示灯 */
    
    /* ========== 配置参数 (0x4000 ~ 0x7FFF) ========== */
    
    /* 设备信息 */
    PARAM_ID_CFG_DEVICE_ID          = 0x4001,   /* 设备ID */
    PARAM_ID_CFG_DEVICE_NAME        = 0x4002,   /* 设备名称 */
    PARAM_ID_CFG_SERIAL_NUMBER      = 0x4003,   /* 序列号 */
    PARAM_ID_CFG_FIRMWARE_VER       = 0x4004,   /* 固件版本 */
    
    /* 网络配置 */
    PARAM_ID_CFG_NET_MODE           = 0x4010,   /* 网络模式 */
    PARAM_ID_CFG_NET_SERVER_ADDR    = 0x4011,   /* 服务器地址 */
    PARAM_ID_CFG_NET_SERVER_PORT    = 0x4012,   /* 服务器端口 */
    PARAM_ID_CFG_NET_HEARTBEAT_INT  = 0x4013,   /* 心跳间隔 */
    
    /* 采集配置 */
    PARAM_ID_CFG_SAMPLE_INTERVAL    = 0x4020,   /* 采样间隔(ms) */
    PARAM_ID_CFG_UPLOAD_INTERVAL    = 0x4021,   /* 上报间隔(s) */
    PARAM_ID_CFG_DATA_BUF_SIZE      = 0x4022,   /* 数据缓冲区大小 */
    
    /* 告警配置 */
    PARAM_ID_CFG_ALARM_TEMP_HIGH    = 0x4030,   /* 高温告警阈值 */
    PARAM_ID_CFG_ALARM_TEMP_LOW     = 0x4031,   /* 低温告警阈值 */
    PARAM_ID_CFG_ALARM_VOLT_HIGH    = 0x4032,   /* 高压告警阈值 */
    PARAM_ID_CFG_ALARM_VOLT_LOW     = 0x4033,   /* 低压告警阈值 */
    
    PARAM_ID_MAX                    = 0x7FFF
} PARAM_ID_DEF;

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_CONFIG_H__ */
