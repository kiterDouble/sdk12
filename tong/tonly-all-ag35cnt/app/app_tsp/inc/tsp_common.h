/**
 * tsp_common.h - 同力云IOT平台对接 通用定义
 *
 * 通用类型、命令枚举、信息标识、错误码等
 * 以及公共消息订阅接口和单调时间工具函数
 */

#ifndef __TSP_COMMON_H__
#define __TSP_COMMON_H__

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include "gnss_ipc_msg.h"
#include "pm_server_api.h"
#include "modem_ipc_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 前向声明 */
struct tly_app;
typedef struct tly_app tly_app_t;

/* ======================== 数据类型 ======================== */

// typedef uint8_t  tly_byte_t;
// typedef uint16_t tly_word_t;
// typedef uint32_t tly_dword_t;

/* ======================== 命令标识 ======================== */

enum tly_cmd_id {
    TLY_CMD_VEHICLE_LOGIN       = 0x01,  /* 车辆登入 (HJ1322-2023) */
    TLY_CMD_REALTIME_REPORT     = 0x02,  /* 实时信息上报 */
    TLY_CMD_REISSUE_REPORT      = 0x03,  /* 补发信息上报 */
    TLY_CMD_VEHICLE_LOGOUT      = 0x04,  /* 车辆登出 */
    TLY_CMD_REMOVE_ALARM        = 0x05,  /* 拆除报警 */
    TLY_CMD_TIME_SYNC           = 0x06,  /* 终端校时 */
    TLY_CMD_PIN_VIN_REQ         = 0x09,  /* 机械环保PIN/VIN码请求 */
    TLY_CMD_LOCK_KEY_REPORT     = 0x0A,  /* 锁车激活秘钥上报 */
    TLY_CMD_BAUDRATE_SET        = 0x35,  /* 波特率设置 */
    TLY_CMD_BATTERY_HEAT_SET    = 0x38,  /* 电池加热设置 */
    TLY_CMD_CLEAR_ECU_FAULT     = 0x43,  /* 清除ECU故障码 */
    TLY_CMD_ECU_FAULT_CODE      = 0x46,  /* 潍柴ECU故障码 */
    TLY_CMD_EV_CMD_FEEDBACK     = 0x47,  /* 电车指令执行反馈 */
    TLY_CMD_ECU_VERSION         = 0x49,  /* ECU版本号 */
    TLY_CMD_WAKEUP_LOCATION     = 0x4F,  /* 定时唤醒心跳位置包 */
    TLY_CMD_VCU_VERSION         = 0x73,  /* VCU版本号 */
    TLY_CMD_TCM_VERSION         = 0x74,  /* TCM版本号 */
    TLY_CMD_MCU_VERSION         = 0x75,  /* MCU版本号 */
    TLY_CMD_CANID_LIST          = 0x77,  /* CANID列表获取(不带通道号) */
    TLY_CMD_VERSION_UPLOAD      = 0x78,  /* 版本号上传 */
    TLY_CMD_CANID_LIST_CH       = 0x7B,  /* CANID列表获取(带通道号) */
    TLY_CMD_CONTROL             = 0x82,  /* 终端控制命令 */
    TLY_CMD_UPGRADE_FAIL        = 0x83,  /* tbox自升级失败 */
    TLY_CMD_APN_REQ             = 0x89,  /* 海外版APN请求 */
    TLY_CMD_FILING_RESULT       = 0x9A,  /* 备案结果上报(非四专用) */
    TLY_CMD_KEY_EXCHANGE        = 0xC0,  /* 非四交换秘钥 */
    TLY_CMD_HEARTBEAT           = 0xC1,  /* 心跳 */
    TLY_CMD_UPGRADE_STATUS      = 0xCF,  /* 升级状态 */
    TLY_CMD_DASHBOARD_STATUS    = 0xE9,  /* 仪表状态上报 */
    TLY_CMD_VIN_REQUEST         = 0xFA,  /* VIN码请求 */
};

/* ======================== 应答标志 ======================== */

enum tly_resp_flag {
    TLY_RESP_SUCCESS   = 0x01,  /* 成功 */
    TLY_RESP_ERROR     = 0x02,  /* 错误 */
    TLY_RESP_DUPLICATE = 0x03,  /* PIN/VIN重复 */
    TLY_RESP_COMMAND   = 0xFE,  /* 命令包(非应答包) */
};

/* ======================== 信息标识 ======================== */

enum tly_info_id {
    TLY_INFO_EMISSION_DIAG  = 0x01,  /* 排放控制诊断信息 (HJ1322-2023) */
    TLY_INFO_DPF_SCR_FLOW   = 0x02,  /* 数据流信息: DPF/SCR */
    TLY_INFO_TWC_FLOW       = 0x03,  /* 数据流信息: 三元催化器 */
    TLY_INFO_DPF_SCR_STAT   = 0x04,  /* 数据项统计值: DPF/SCR */
    TLY_INFO_FAST           = 0x90,  /* 快包 */
    TLY_INFO_SLOW           = 0x91,  /* 慢包 */
    TLY_INFO_EV_FAST        = 0xA0,  /* 电车快包 */
    TLY_INFO_EV_SLOW        = 0xA1,  /* 电车慢包 */
    TLY_INFO_ALARM          = 0x97,  /* 报警(燃油车) */
    TLY_INFO_FAULT_CODE     = 0x99,  /* 故障码上报 */
    TLY_INFO_LOCATION       = 0xAC,  /* 位置信息 */
    TLY_INFO_TIRE_PRESSURE  = 0x6E,  /* 胎压上报 */
    TLY_INFO_LOCK_STATUS    = 0xCF,  /* 仪表锁车状态 */
    TLY_INFO_SD_CARD        = 0x8B,  /* SD卡状态 */
    TLY_INFO_BATTERY_HEAT   = 0xCA,  /* 主动电池加热 */
};

/* ======================== 加密方式 ======================== */

enum tly_encrypt_mode {
    TLY_ENCRYPT_NONE    = 0x01,  /* 不加密不签名 */
    TLY_ENCRYPT_SM2     = 0x03,  /* SM2加密且签名 */
    TLY_ENCRYPT_SIGN    = 0xFA,  /* 只签名不加密 */
    TLY_ENCRYPT_INVALID = 0xFF,  /* 无效 */
};

/* ======================== 错误码 ======================== */

enum tly_error {
    TLY_OK              =  0,
    TLY_ERR_PARAM       = -1,   /* 参数错误 */
    TLY_ERR_MEMORY      = -2,   /* 内存分配失败 */
    TLY_ERR_CONNECT     = -3,   /* 连接失败 */
    TLY_ERR_TIMEOUT     = -4,   /* 超时 */
    TLY_ERR_PROTOCOL    = -5,   /* 协议解析错误 */
    TLY_ERR_CHECKSUM    = -6,   /* 校验失败 */
    TLY_ERR_HTTP        = -7,   /* HTTP请求失败 */
    TLY_ERR_CONFIG      = -8,   /* 配置错误 */
    TLY_ERR_CLOSED      = -9,   /* 连接已关闭 */
};

/* ======================== 发动机类型 ======================== */

enum tly_engine_type {
    TLY_ENGINE_WEICHAI    = 0,   /* 潍柴 */
    TLY_ENGINE_YUCHAI     = 1,   /* 玉柴 */
    TLY_ENGINE_CUMMINS    = 2,   /* 康明斯 */
    TLY_ENGINE_MAN        = 3,   /* 曼 */
    TLY_ENGINE_XICHAI     = 4,   /* 锡柴 */
    TLY_ENGINE_ELECTRIC   = 15,  /* 电车 */
};

/* ======================== 锁车类型 ======================== */

enum tly_lock_type {
    TLY_LOCK_UNLOCK       = 0,   /* 解锁 */
    TLY_LOCK_LOCK         = 1,   /* 锁车(禁用) */
    TLY_LOCK_LIMIT_TORQUE = 2,   /* 锁车限扭限速(玉柴) / 激活(其他) */
    TLY_LOCK_ACTIVATE     = 3,   /* 激活 / 取消激活(玉柴) */
    TLY_LOCK_DEACTIVATE   = 4,   /* 取消激活(仅玉柴) */
};

/* ======================== 通用宏 ======================== */

/* ======================== 终端控制子命令 (0x82 数据单元 data[6]) ======================== */

enum tly_ctrl_cmd {
    TLY_CTRL_LOCK_CUMMINS  = 0x70,  /* 康明斯锁车 */
    TLY_CTRL_LOCK_YUCHAI   = 0x92,  /* 玉柴锁车 */
    TLY_CTRL_LOCK_MAN      = 0x93,  /* 曼锁车 */
    TLY_CTRL_LOCK_WEICHAI  = 0x90,  /* 潍柴锁车 */
    TLY_CTRL_LOCK_XICHAI   = 0x94,  /* 锡柴锁车 */
    TLY_CTRL_LOCK_EV       = 0x80,  /* 电车锁车 */
    TLY_CTRL_SD_CARD       = 0x29,  /* SD卡数据获取 */
    TLY_CTRL_VIN_BIND      = 0xBF,  /* VIN绑定 */
    TLY_CTRL_TIME_SET      = 0xC4,  /* 主动授时 */
    TLY_CTRL_BATTERY_HEAT  = 0xCA,  /* 主动电池加热 */
    TLY_CTRL_VIN_UNBIND    = 0x95,  /* VIN解绑 */
};

/* ======================== OTA升级命令码 (data[6]) ======================== */

enum tly_ota_cmd {
    TLY_OTA_TBOX           = 0x01,  /* TBOX升级 */
    TLY_OTA_TBOX_EXT       = 0x11,  /* TBOX升级(扩展) */
    TLY_OTA_ECU_WEICHAI    = 0x40,  /* ECU潍柴 */
    TLY_OTA_VCU_NORMAL     = 0x50,  /* VCU(立即) */
    TLY_OTA_VCU_YK_NORMAL  = 0x51,  /* 易控VCU(立即) */
    TLY_OTA_VCU_BLK_NORMAL = 0x52,  /* 铂雷科VCU(立即) */
    TLY_OTA_TMS_NORMAL     = 0x53,  /* TMS(立即) */
    TLY_OTA_TCU_NORMAL     = 0x54,  /* TCU(立即) */
    TLY_OTA_VCU_FIRST      = 0x5A,  /* VCU首次刷写 */
    TLY_OTA_TCU            = 0x5B,  /* TCU(强制) */
    TLY_OTA_TMS            = 0x5C,  /* TMS(强制) */
    TLY_OTA_VCU_BOLEIKE    = 0x5D,  /* 铂雷科VCU(强制) */
    TLY_OTA_VCU_YIKONG     = 0x5E,  /* 易控VCU(强制) */
    TLY_OTA_VCU            = 0x5F,  /* VCU(强制) */
    TLY_OTA_MCU1_TM1       = 0x60,  /* MCU1_TM1升级 */
    TLY_OTA_MCU1_TM2       = 0x61,  /* MCU1_TM2升级 */
    TLY_OTA_MCU1_PDU       = 0x62,  /* MCU1_PDU升级 */
    TLY_OTA_MCU1_EPS       = 0x63,  /* MCU1_EPS升级 */
    TLY_OTA_MCU1_ACM       = 0x64,  /* MCU1_ACM升级 */
    TLY_OTA_MCU1_DCDC      = 0x65,  /* MCU1_DCDC升级 */
    TLY_OTA_MCU2_TM1       = 0x66,  /* MCU2_TM1升级 */
    TLY_OTA_MCU2_TM2       = 0x67,  /* MCU2_TM2升级 */
    TLY_OTA_MCU2_PDU       = 0x68,  /* MCU2_PDU升级 */
    TLY_OTA_MCU2_EPS       = 0x69,  /* MCU2_EPS升级 */
    TLY_OTA_MCU2_ACM       = 0x6A,  /* MCU2_ACM升级 */
    TLY_OTA_MCU2_DCDC      = 0x6B,  /* MCU2_DCDC升级 */
    TLY_OTA_UD_TM1         = 0x6C,  /* UD_TM1升级 */
};

/* ======================== ECU类型定义 (0xCF升级状态上报ecu_type) ======================== */

enum tly_ecu_type {
    TLY_ECU_TYPE_TBOX_MAIN    = 0x00,  /* TBOX主MCU */
    TLY_ECU_TYPE_TBOX_SLAVE   = 0x01,  /* TBOX从MCU */
    TLY_ECU_TYPE_TMS          = 0xA1,  /* TMS */
    TLY_ECU_TYPE_TCU          = 0xA2,  /* TCU */
    TLY_ECU_TYPE_VCU_TONGLY   = 0xC1,  /* 同力自用VCU */
    TLY_ECU_TYPE_ECU_WEICHAI  = 0xD1,  /* 潍柴发动机ECU */
    TLY_ECU_TYPE_MCU_TM       = 0xE0,  /* 法士特MCU1_TM软件升级 */
    TLY_ECU_TYPE_MCU_TM1      = 0xE1,  /* 法士特MCU1_TM1软件升级 */
    TLY_ECU_TYPE_MCU_TM2      = 0xE2,  /* 法士特MCU1_TM2软件升级 */
    TLY_ECU_TYPE_MCU_PD2      = 0xE3,  /* 法士特MCU1_PD2软件升级 */
    TLY_ECU_TYPE_MCU_EPS      = 0xE4,  /* 法士特MCU1_EPS软件升级 */
    TLY_ECU_TYPE_MCU_ACM      = 0xE5,  /* 法士特MCU1_ACM软件升级 */
    TLY_ECU_TYPE_MCU_DCDC     = 0xE6,  /* 法士特MCU1_DCDC软件升级 */
    TLY_ECU_TYPE_MCU_TM8      = 0xE7,  /* 法士特MCU1_TM8软件升级 */
    TLY_ECU_TYPE_MCU_TM9      = 0xE8,  /* 法士特MCU1_TM9软件升级 */
    TLY_ECU_TYPE_MCU2_TM1      = 0xF2,  /* 法士特MCU2_TM1软件升级 */
    TLY_ECU_TYPE_MCU2_TM2      = 0xF3,  /* 法士特MCU2_TM2软件升级 */
    TLY_ECU_TYPE_MCU2_PDU      = 0xF4,  /* 法士特MCU2_PDU软件升级 */
    TLY_ECU_TYPE_MCU2_ACM      = 0xF5,  /* MCU2_ACM软件升级 */
    TLY_ECU_TYPE_MCU2_DCDC     = 0xF6,  /* MCU2_DCDC软件升级 */
    TLY_ECU_TYPE_MCU2_EPS      = 0xF7,  /* MCU2_EPS软件升级 */
    TLY_ECU_TYPE_UD_TM1       = 0xF1,  /* UD_TM1软件升级 */
};

/* ======================== MCU版本上报类型 (0x75数据单元中的mcu_type字段) ======================== */

enum tly_mcu_ver_type {
    TLY_MCU_VER_TYPE_TM1  = 0x01,  /* MCU1_TM1 */
    TLY_MCU_VER_TYPE_PDU  = 0x02,  /* MCU1_PDU */
    TLY_MCU_VER_TYPE_EPS  = 0x03,  /* MCU1_EPS */
    TLY_MCU_VER_TYPE_ACM  = 0x04,  /* MCU1_ACM */
};

/**
 * @brief OTA命令码转ECU类型
 * @param ota_cmd OTA命令码
 * @return ECU类型值，用于0xCF升级状态上报
 */
static inline uint8_t tly_ota_cmd_to_ecu_type(uint8_t ota_cmd)
{
    switch (ota_cmd)
    {
        /* TBOX主模块 */
        case TLY_OTA_TBOX:
        case TLY_OTA_TBOX_EXT:
            return TLY_ECU_TYPE_TBOX_MAIN;

        /* TMS热管理 */
        case TLY_OTA_TMS:
        case TLY_OTA_TMS_NORMAL:
            return TLY_ECU_TYPE_TMS;

        /* TCU变速箱 */
        case TLY_OTA_TCU:
        case TLY_OTA_TCU_NORMAL:
            return TLY_ECU_TYPE_TCU;

        /* 全系VCU（通利、易控、博雷克） */
        case TLY_OTA_VCU:
        case TLY_OTA_VCU_NORMAL:
        case TLY_OTA_VCU_FIRST:
        case TLY_OTA_VCU_YIKONG:
        case TLY_OTA_VCU_YK_NORMAL:
        case TLY_OTA_VCU_BOLEIKE:
        case TLY_OTA_VCU_BLK_NORMAL:
            return TLY_ECU_TYPE_VCU_TONGLY;

        /* 潍柴ECU */
        case TLY_OTA_ECU_WEICHAI:
            return TLY_ECU_TYPE_ECU_WEICHAI;

        /* TM主MCU */
        case TLY_OTA_MCU1_TM1:
            return TLY_ECU_TYPE_MCU_TM1;

        case TLY_OTA_MCU1_TM2:
            return TLY_ECU_TYPE_MCU_TM2;

        case TLY_OTA_MCU1_PDU:
            return TLY_ECU_TYPE_MCU_PD2;
        case TLY_OTA_MCU1_EPS:
            return TLY_ECU_TYPE_MCU_EPS;
        case TLY_OTA_MCU1_ACM:
            return TLY_ECU_TYPE_MCU_ACM;
        case TLY_OTA_MCU1_DCDC:
            return TLY_ECU_TYPE_MCU_DCDC;

        /* 第二路MCU */
        case TLY_OTA_MCU2_TM1:
            return TLY_ECU_TYPE_MCU2_TM1;
        case TLY_OTA_MCU2_TM2:
            return TLY_ECU_TYPE_MCU2_TM2;
        case TLY_OTA_MCU2_PDU:
            return TLY_ECU_TYPE_MCU2_PDU;
        case TLY_OTA_MCU2_EPS:
            return TLY_ECU_TYPE_MCU2_EPS;
        case TLY_OTA_MCU2_ACM:
            return TLY_ECU_TYPE_MCU2_ACM;
        case TLY_OTA_MCU2_DCDC:
            return TLY_ECU_TYPE_MCU2_DCDC;

        /* 大运TM1 */
        case TLY_OTA_UD_TM1:
            return TLY_ECU_TYPE_UD_TM1;

        /* 未定义OTA指令，返回ota_cmd */
        default:
            return ota_cmd;
    }
}

#define TLY_ICCID_LEN       20
#define TLY_ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))
#define TLY_MIN(a, b)        ((a) < (b) ? (a) : (b))
#define TLY_MAX(a, b)        ((a) > (b) ? (a) : (b))

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 硬线状态信息结构体
 */
typedef struct {
    uint8_t  kl15status;      /* KL15状态 */
    uint32_t kl30voltage;     /* KL30电压 (单位: mV) */
    uint16_t fastCharge;      /* 快充状态 */
    uint8_t  OBD;             /* OBD诊断仪接入状态: 0-未接入, 1-已接入 */
    uint8_t  kl30status;      /* KL30状态 */
} HardLineSyncInfo_t;

/* ======================== 公共消息订阅接口 ======================== */

/**
 * @brief 获取GNSS位置信息
 * @param[out] location 输出的位置信息结构体
 * @return 0-成功, -1-参数错误
 */
int CommonMsg_GetGnssLocation(GnssLocation_t *location);

/**
 * @brief 获取硬线状态信息
 * @param[out] info 输出的硬线状态信息
 * @return 0-成功, -1-参数错误
 */
int CommonMsg_GetHardLineInfo(HardLineSyncInfo_t *info);

/**
 * @brief 获取PM电源状态信息
 * @param[out] pmState 输出的PM电源状态信息
 * @return 0-成功, -1-参数错误
 */
int CommonMsg_GetPmState(MPUPmStateChange_t *pmState);

/**
 * @brief 获取net服务信息
 * @param[out] modemInfo 输出的Modem服务信息
 * @return 0-成功, -1-参数错误
 */
int CommonMsg_GetModemSrvInfo(ModemSrvNotify_t *modemInfo);

/**
 * @brief 初始化并订阅公共消息队列
 * @param[in] app TSP应用上下文指针
 * @description 订阅以下公共消息:
 *              - SYS_QUE_TYPE_GNSS_LOCATIONINFO: GNSS定位信息
 *              - SYS_QUE_TYPE_MCU_SYNC_KLINE: 硬线状态同步
 *              - SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY: PM电源状态通知
 * @return 0-成功, 其他-失败
 */
int CommonMsg_Subscribe(tly_app_t *app);

/**
 * @brief 取消订阅公共消息队列
 * @return 0-成功, 其他-失败
 */
int CommonMsg_Unsubscribe(void);

/* ======================== 单调时间工具函数 ======================== */

/**
 * @brief 获取单调时间（秒）
 * @return 单调时间秒数，失败返回0
 * @note 使用CLOCK_MONOTONIC时钟，不受系统时间调整影响
 */
time_t tly_monotonic_time_sec(void);

/**
 * @brief 获取单调时间（毫秒）
 * @return 单调时间毫秒数，失败返回0
 * @note 使用CLOCK_MONOTONIC时钟，不受系统时间调整影响
 */
uint64_t tly_monotonic_time_ms(void);

/**
 * @brief 获取单调时间（微秒）
 * @return 单调时间微秒数，失败返回0
 * @note 使用CLOCK_MONOTONIC时钟，不受系统时间调整影响
 */
uint64_t tly_monotonic_time_us(void);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_COMMON_H__ */
