/**
 * tsp_protocol.h - 二进制协议编解码
 *
 * 数据包格式 (大端字节序):
 *   起始符(2B) + 命令单元(1B) + VIN(17B) + 版本号(1B) + 加密方式(1B)
 *   + 数据长度(2B) + 数据单元(NB) + 校验码(1B)
 */

#ifndef __TSP_PROTOCOL_H__
#define __TSP_PROTOCOL_H__

#include "tsp_common.h"
#include <time.h>

typedef struct tly_app tly_app_t;

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== 协议常量 ======================== */

#define TLY_PKT_START       "##"
#define TLY_PKT_START_LEN   2
#define TLY_PKT_VIN_LEN     17
#define TLY_PKT_HEADER_LEN  24   /* 起始符(2)+命令(1)+VIN(17)+版本(1)+加密(1)+长度(2) */
#define TLY_PKT_MAX_DATA    2048 /* 单包最大数据单元 */
#define TLY_PKT_MAX_SIZE    (TLY_PKT_HEADER_LEN + TLY_PKT_MAX_DATA + 1) /* +1校验 */

/* ======================== 数据包结构 ======================== */

typedef struct tly_packet {
    uint8_t  cmd_id;                   /* 命令标识 */
    char     vin[TLY_PKT_VIN_LEN + 1]; /* 机械环保码, 末尾'\0' */
    uint8_t  sw_version;               /* 终端软件版本号 */
    uint8_t  encrypt_mode;             /* 加密签名方式 */
    uint16_t data_len;                 /* 数据单元长度 */
    uint8_t  *data;                    /* 数据单元(动态分配) */
} tly_packet_t;

/* ======================== 通用编解码 ======================== */

/** 打包: 结构体 → 二进制字节流 */
int  tly_packet_encode(const tly_packet_t *pkt,
                       uint8_t *out_buf, uint32_t *out_len);

/** 拆包: 二进制字节流 → 结构体. 调用者需 tly_packet_free() */
int  tly_packet_decode(const uint8_t *in_buf, uint32_t in_len,
                       tly_packet_t *pkt);

/** 释放 pkt->data */
void tly_packet_free(tly_packet_t *pkt);

/** BCC异或校验: 从cmd_id开始到data最后一字节, 逐字节异或 */
uint8_t tly_bcc_calc(const uint8_t *data, uint32_t len);

/* ======================== 时间编解码 ======================== */

/** struct tm → 6字节协议时间 (年=yy, 0~99) */
int tly_time_encode(const struct tm *tm, uint8_t *buf);

/** 6字节协议时间 → struct tm */
int tly_time_decode(const uint8_t *buf, struct tm *tm);

/** 获取当前时间的6字节编码 */
int tly_time_now(uint8_t *buf);

/* ======================== 大端读写 ======================== */

static inline void tly_write_be16(uint8_t *buf, uint16_t val)
{
    buf[0] = (uint8_t)(val >> 8);
    buf[1] = (uint8_t)(val);
}

static inline void tly_write_be32(uint8_t *buf, uint32_t val)
{
    buf[0] = (uint8_t)(val >> 24);
    buf[1] = (uint8_t)(val >> 16);
    buf[2] = (uint8_t)(val >> 8);
    buf[3] = (uint8_t)(val);
}

static inline uint16_t tly_read_be16(const uint8_t *buf)
{
    return ((uint16_t)buf[0] << 8) | buf[1];
}

static inline uint32_t tly_read_be32(const uint8_t *buf)
{
    return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16)
         | ((uint32_t)buf[2] << 8) | buf[3];
}

static inline uint32_t tly_read_le32(const uint8_t *buf)
{
    return ((uint32_t)buf[3] << 24) | ((uint32_t)buf[2] << 16)
         | ((uint32_t)buf[1] << 8) | buf[0];
}

/* ======================== 车辆登入 (0x01) ======================== */

typedef struct tly_vehicle_login {
    uint8_t  time[6];                  /* 数据采集时间 */
    uint16_t seq;                      /* 登入流水号 */
    char     iccid[TLY_ICCID_LEN + 1]; /* SIM卡ICCID号 */
} tly_vehicle_login_t;

int tly_encode_vehicle_login(const tly_vehicle_login_t *login,
                             uint8_t *buf, uint32_t *len);
int tly_decode_vehicle_login(const uint8_t *buf, uint32_t len,
                             tly_vehicle_login_t *login);

/* ======================== 车辆登出 (0x04) ======================== */

typedef struct tly_vehicle_logout {
    uint8_t  time[6];
    uint16_t seq;
} tly_vehicle_logout_t;

int tly_encode_vehicle_logout(const tly_vehicle_logout_t *logout,
                              uint8_t *buf, uint32_t *len);
int tly_decode_vehicle_logout(const uint8_t *buf, uint32_t len,
                              tly_vehicle_logout_t *logout);

/* ======================== 位置信息 (信息标识 0xAC) ======================== */

typedef struct tly_location {
    uint8_t  status;    /* 定位状态: bit0有效位, bit1南北纬, bit2东西经 */
    uint32_t longitude; /* 经度 ×10^6 */
    uint32_t latitude;  /* 纬度 ×10^6 */
    uint16_t altitude;  /* 海拔(米) */
    uint8_t  gps_speed; /* GPS速度(km/h) */
    uint16_t heading;   /* 方位角(度), 正北为0, 顺时针 */
    uint16_t accel_x;   /* X轴加速度: (原始值+30)*1000 */
    uint16_t accel_y;   /* Y轴加速度 */
    uint16_t accel_z;   /* Z轴加速度 */
    uint8_t  dev_status; /* bit0-2: GPS天线状态, bit3-7: 网络信号 */
} tly_location_t;

int tly_encode_location(const tly_location_t *loc,
                        uint8_t *buf, uint32_t *len);
int tly_decode_location(const uint8_t *buf, uint32_t len,
                        tly_location_t *loc);

/* ======================== 实时信息上报 (0x02) ======================== */

typedef struct tly_info_item {
    uint8_t  info_id;    /* 信息标识 */
    uint8_t  *info_data; /* 信息内容(动态分配) */
    uint32_t info_len;   /* 信息内容长度 */
} tly_info_item_t;

typedef struct tly_realtime_report {
    uint8_t          time[6];  /* 数据采集时间 */
    tly_info_item_t *items;    /* 信息项数组 */
    uint32_t         item_count;
} tly_realtime_report_t;

int  tly_encode_realtime_report(const tly_realtime_report_t *rpt,
                                uint8_t *buf, uint32_t *len);
int  tly_decode_realtime_report(const uint8_t *buf, uint32_t len,
                                tly_realtime_report_t *rpt);
void tly_realtime_report_free(tly_realtime_report_t *rpt);

/* ======================== 拆除报警 (0x05) ======================== */

/* ======================== 版本号上传 (0x78) ======================== */

typedef struct tly_version_upload {
    uint8_t  time[6];
    uint16_t hw_version;         /* 硬件版本号 */
    uint32_t main_mcu_version;   /* 主MCU版本 */
    uint16_t sub_mcu_version;    /* 从MCU版本 */
    uint16_t non_stage4_version; /* 非四版本 */
    uint16_t version_seq;        /* 版本流水号 */
} tly_version_upload_t;

int tly_encode_version_upload(const tly_version_upload_t *ver,
                              uint8_t *buf, uint32_t *len);
int tly_decode_version_upload(const uint8_t *buf, uint32_t len,
                              tly_version_upload_t *ver);

/* ======================== VCU版本号上传 (0x73) ======================== */

typedef struct tly_vcu_version {
    uint8_t  time[6];           /* 时间: yy mm dd hh mm ss */
    uint8_t  version[6];        /* 版本号报文: 6字节 */
} tly_vcu_version_t;

int tly_encode_vcu_version(const tly_vcu_version_t *ver,
                           uint8_t *buf, uint32_t *len);
int tly_decode_vcu_version(const uint8_t *buf, uint32_t len,
                           tly_vcu_version_t *ver);
void tly_vcu_version_dump(const tly_vcu_version_t *ver);

/* ======================== TCU版本号上传 (0x74) ======================== */

#define TLY_TCU_VERSION_MAX_LEN  100  /* TCU版本号最大长度100字节 */

typedef struct tly_tcu_version {
    uint8_t  version[TLY_TCU_VERSION_MAX_LEN];  /* 版本号报文*/
    uint32_t version_len;                      /* 版本号实际长度 */
} tly_tcu_version_t;

int tly_encode_tcu_version(const tly_tcu_version_t *ver,
                           uint8_t *buf, uint32_t *len);
int tly_decode_tcu_version(const uint8_t *buf, uint32_t len,
                           tly_tcu_version_t *ver);
void tly_tcu_version_dump(const tly_tcu_version_t *ver);

/* ======================== MCU版本号上传 (0x75) ======================== */
/* 数据单元格式: 数量(1B) + [mcu_type(1B) + 版本内容(10B)] × 数量 */

#define TLY_MCU_VER_CONTENT_LEN   10    /* 单个MCU版本内容固定10字节 */
#define TLY_MCU_VER_ITEM_LEN      (1 + TLY_MCU_VER_CONTENT_LEN)  /* mcu_type(1) + 版本(10) = 11 */
#define TLY_MCU_VER_MAX_COUNT     8     /* 最大MCU类型数 */
#define TLY_MCU_VERSION_MAX_LEN   (1 + TLY_MCU_VER_MAX_COUNT * TLY_MCU_VER_ITEM_LEN)  /* 数量(1) + N×11 */

/* 单个MCU版本项 */
typedef struct tly_mcu_ver_item {
    uint8_t mcu_type;                                /* MCU类型 (tly_mcu_ver_type枚举) */
    uint8_t version[TLY_MCU_VER_CONTENT_LEN];        /* 版本内容(10字节,不足补0x00) */
} tly_mcu_ver_item_t;

/* MCU版本数据(多MCU打包) */
typedef struct tly_mcu_version {
    tly_mcu_ver_item_t items[TLY_MCU_VER_MAX_COUNT]; /* MCU版本项数组 */
    uint8_t count;                                   /* 实际MCU数量 */
} tly_mcu_version_t;

int tly_encode_mcu_version(const tly_mcu_version_t *ver,
                           uint8_t *buf, uint32_t *len);
int tly_decode_mcu_version(const uint8_t *buf, uint32_t len,
                           tly_mcu_version_t *ver);
void tly_mcu_version_dump(const tly_mcu_version_t *ver);

/* ======================== 拆除报警 (0x05) ======================== */

typedef struct tly_remove_alarm {
    uint8_t  time[6];
    uint16_t seq;
    uint8_t  status; /* 定位状态位 */
    uint32_t longitude;
    uint32_t latitude;
} tly_remove_alarm_t;

int tly_encode_remove_alarm(const tly_remove_alarm_t *alarm,
                            uint8_t *buf, uint32_t *len);
int tly_decode_remove_alarm(const uint8_t *buf, uint32_t len,
                            tly_remove_alarm_t *alarm);

/* ======================== CANID列表获取-带通道号 (0x7B) ======================== */

#define TLY_CANID_MAX  256

/* 单个CANID条目: CANID值(4B) + 通道号(1B) */
typedef struct tly_canid_entry {
    uint32_t can_id;
    uint8_t  channel;
} tly_canid_entry_t;

/* CANID列表响应 */
typedef struct tly_canid_list_resp {
    uint8_t             fast_freq;   /* 快包采集频次(秒) */
    uint16_t            fast_count;  /* 快包CANID个数 */
    tly_canid_entry_t   fast_ids[TLY_CANID_MAX];
    uint8_t             slow_freq;   /* 慢包采集频次(秒) */
    uint16_t            slow_count;  /* 慢包CANID个数 */
    tly_canid_entry_t   slow_ids[TLY_CANID_MAX];
} tly_canid_list_resp_t;

/* CANID列表请求(1字节类型) */
int tly_encode_canid_request(uint8_t vehicle_type,
                             uint8_t *buf, uint32_t *len);

/* CANID列表响应解析(带通道号 0x7B) */
int tly_decode_canid_response(const uint8_t *buf, uint32_t len,
                              tly_canid_list_resp_t *resp);

/* CANID列表响应解析(不带通道号 0x77) */
int tly_decode_canid_response_no_ch(const uint8_t *buf, uint32_t len,
                                    tly_canid_list_resp_t *resp);

void tly_canid_list_resp_dump(const tly_canid_list_resp_t *resp);

/* ======================== 电车快包 (信息标识 0xA0) ======================== */

#define TLY_EV_FRAME_MAX  64

/* 单个CAN帧 */
typedef struct tly_ev_can_frame {
    uint32_t can_id;      /* CANID值 */
    uint8_t  msg_len;     /* 报文长度 (低4位) */
    uint8_t  channel;     /* 通道号 (高4位) */
    uint8_t  raw[8];      /* 原始报文(最多8字节) */
} tly_ev_can_frame_t;

/* 电车快包整体 */
typedef struct tly_ev_fast_packet {
    uint8_t             frame_count;
    tly_ev_can_frame_t  frames[TLY_EV_FRAME_MAX];
} tly_ev_fast_packet_t;

int tly_decode_ev_fast_packet(const uint8_t *buf, uint32_t len,
                              tly_ev_fast_packet_t *pkt);

void tly_ev_fast_packet_dump(const tly_ev_fast_packet_t *pkt);

/* ======================== 终端控制 - 锁车 (0x82, data[6]=0x80/0x90/0x92/0x94/0x70/0x93) ======================== */

typedef struct tly_lock_control {
    uint8_t  time[6];   /* 数据采集时间 */
    uint8_t  ctrl_cmd;  /* 控制指令: 发动机厂商标识 */
    uint8_t  lock_type; /* 锁车类型: 0解锁/1锁车/2激活/3取消激活 */
    uint8_t  torque;    /* 扭矩(仅玉柴) */
    uint16_t rpm;       /* 发动机转速(默认800, 0~800) */
    uint8_t  auto_lock; /* 是否自动锁车(预留) */
} tly_lock_control_t;

int tly_decode_lock_control(const uint8_t *buf, uint32_t len,
                            tly_lock_control_t *lock);

void tly_lock_control_dump(const tly_lock_control_t *lock);

/* ======================== 终端控制 - OTA升级 (0x82, data[6]=OTA命令码) ======================== */

#define TLY_OTA_STR_MAX  64
#define TLY_OTA_URL_MAX  128

/* OTA ECU 协议信息 */
typedef struct tly_ota_protocol_info {
    uint8_t  can_port;                          /* CAN端口 */
    uint32_t physical_id;                       /* 物理寻址ID */
    uint32_t function_id;                       /* 功能寻址ID */
    uint32_t response_id;                       /* 应答ID */
} tly_ota_protocol_info_t;

typedef struct tly_ota_info {
    uint8_t  time[6];                           /* 数据采集时间 */
    uint8_t  ota_cmd;                           /* OTA命令码 */
    char     username[TLY_OTA_STR_MAX];         /* FTP用户名 */
    char     password[TLY_OTA_STR_MAX];         /* FTP密码 */
    uint8_t  ip[6];                             /* IP地址 */
    uint16_t port;                              /* 端口 */
    char     mid[TLY_OTA_STR_MAX];              /* MID */
    char     yversion[TLY_OTA_STR_MAX];         /* 源版本号 */
    char     gversion[TLY_OTA_STR_MAX];         /* 目标版本号 */
    char     url[TLY_OTA_URL_MAX];              /* FTP文件下载地址 */
    uint16_t upgrade_time;                      /* 升级时间 */
    tly_ota_protocol_info_t protocol_info;      /* ECU刷写协议信息 */
} tly_ota_info_t;

int tly_decode_ota_info(const uint8_t *buf, uint32_t len,
                        tly_ota_info_t *ota);

void tly_ota_info_dump(const tly_ota_info_t *ota);

/* ======================== 升级状态 (0xCF) ======================== */

typedef struct tly_upgrade_status {
    uint8_t  time[6];           /* 时间: yy mm dd hh mm ss */
    uint8_t  ecu_type;          /* ECU类型 */
    uint8_t  process_status;    /* 进程状态 */
    uint8_t  error_status;      /* 错误状态 */
    uint16_t download_percent;  /* 下载进度百分比 */
    uint16_t upgrade_percent;   /* 升级进度百分比 */
} tly_upgrade_status_t;

int tly_encode_upgrade_status(const tly_upgrade_status_t *status,
                              uint8_t *buf, uint32_t *len);
int tly_decode_upgrade_status(const uint8_t *buf, uint32_t len,
                              tly_upgrade_status_t *status);
void tly_upgrade_status_dump(const tly_upgrade_status_t *status);

/* ======================== 终端控制 - 子命令分发 ======================== */

enum tly_ctrl_type {
    TLY_CTRL_TYPE_LOCK,         /* 锁车 */
    TLY_CTRL_TYPE_OTA,          /* 远程OTA升级 */
    TLY_CTRL_TYPE_BATTERY_HEAT, /* 电池加热 */
    TLY_CTRL_TYPE_SD_CARD,      /* SD卡数据获取 */
    TLY_CTRL_TYPE_VIN_UNBIND,   /* VIN解绑 */
    TLY_CTRL_TYPE_TIME_SYNC,    /* 主动授时 */
    TLY_CTRL_TYPE_VIN_BIND,     /* VIN绑定 */
    TLY_CTRL_TYPE_OTHER,        /* 其他(SD卡/授时等) */
};

/**
 * 判断0x82数据单元的子命令类型
 * @return tly_ctrl_type 枚举值
 */
int tly_ctrl_classify(const uint8_t *data, uint32_t data_len);

/* ======================== 终端控制 - 电池加热 ======================== */

typedef struct tly_battery_heat {
    uint8_t  time[6];       /* 时间: yy mm dd hh mm ss */
    uint8_t  cmd;           /* 命令字: 0xCA */
    uint32_t time_second;   /* 加热时长(秒), 有效值 3600-18000 */
} tly_battery_heat_t;

int  tly_decode_battery_heat(const uint8_t *buf, uint32_t len,
                             tly_battery_heat_t *heat);
void tly_battery_heat_dump(const tly_battery_heat_t *heat);

/* ======================== 终端控制 - SD卡数据获取 ======================== */

typedef struct tly_sd_card {
    uint8_t  time[6];                   /* 数据采集时间 */
    uint8_t  cmd;                       /* 命令字: 0x29 */
    char     username[TLY_OTA_STR_MAX]; /* FTP用户名 */
    char     password[TLY_OTA_STR_MAX]; /* FTP密码 */
    uint8_t  ip[6];                     /* IP地址 */
    uint16_t port;                      /* 端口 */
    char     url[TLY_OTA_STR_MAX];      /* FTP文件上传地址 */
    uint8_t  begin_time[6];             /* 采集开始时间 */
    uint16_t time_count;                /* 采集时长(秒), 0-600 */
} tly_sd_card_t;

int  tly_decode_sd_card(const uint8_t *buf, uint32_t len, tly_sd_card_t *sd);
void tly_sd_card_dump(const tly_sd_card_t *sd);

/* ======================== 终端控制 - VIN解绑 ======================== */

#define TLY_VIN_UNBIND_LEN  17

typedef struct tly_vin_unbind {
    uint8_t time[6];                        /* 数据采集时间 */
    uint8_t cmd;                            /* 命令字: 0x95 */
    char    target_vin[TLY_VIN_UNBIND_LEN + 1]; /* 待解绑VIN码 */
} tly_vin_unbind_t;

int  tly_decode_vin_unbind(const uint8_t *buf, uint32_t len, tly_vin_unbind_t *vu);
void tly_vin_unbind_dump(const tly_vin_unbind_t *vu);

/* ======================== 终端控制 - VIN绑定 ======================== */

#define TLY_VIN_BIND_LEN  17

typedef struct tly_vin_bind {
    uint8_t time[6];                        /* 数据采集时间 */
    uint8_t cmd;                            /* 命令字: 0x96 */
    char    target_vin[TLY_VIN_BIND_LEN + 1]; /* 待绑定VIN码 */
} tly_vin_bind_t;

int  tly_decode_vin_bind(const uint8_t *buf, uint32_t len, tly_vin_bind_t *vb);
void tly_vin_bind_dump(const tly_vin_bind_t *vb);

/* ======================== 终端控制 - 主动授时 ======================== */

typedef struct tly_time_sync {
    uint8_t  time[6];       /* 时间: yy mm dd hh mm ss */
    uint8_t  cmd;           /* 命令字: 0xC4 */
    uint32_t time_second;   /* 自动校时周期(秒), 有效值 300-604800 */
} tly_time_sync_t;

int  tly_decode_time_sync(const uint8_t *buf, uint32_t len, tly_time_sync_t *ts);
void tly_time_sync_dump(const tly_time_sync_t *ts);

/* ======================== 应答包封装 ======================== */

/**
 * 构造应答包
 * @param cmd_id    原始命令标识
 * @param vin       VIN码
 * @param resp_flag 应答标志
 * @param sw_ver    软件版本
 * @param encrypt   加密方式
 * @param data      应答数据(可为NULL)
 * @param data_len  应答数据长度
 * @param out_buf   输出缓冲区
 * @param out_len   输出长度
 */
int tly_encode_response(uint8_t cmd_id, const char *vin,
                        uint8_t resp_flag, uint8_t sw_ver,
                        uint8_t encrypt,
                        const uint8_t *data, uint32_t data_len,
                        uint8_t *out_buf, uint32_t *out_len);

/* ======================== 升级失败上报 (0x83) ======================== */

/**
 * @brief 升级失败数据结构
 * @note 数据单元格式：
 *       - 执行时间: 6字节 (年月日时分秒，BCD码)
 *       - 执行结果: 1字节 (0=成功, 1=失败)
 *       - 失败错误码: 1字节
 */
typedef struct {
    uint8_t time[6];        /* 执行时间: 年月日时分秒 */
    uint8_t result;         /* 执行结果: 0=成功, 1=失败 */
    uint8_t error_code;     /* 失败错误码 */
} tly_upgrade_fail_t;

/**
 * @brief 编码升级失败数据单元
 * @param[in] fail_info 升级失败信息
 * @param[out] buf 输出缓冲区 (至少8字节)
 * @param[out] len 输出数据长度
 * @return 0: 成功, -1: 失败
 */
int tly_encode_upgrade_fail(const tly_upgrade_fail_t *fail_info,
                            uint8_t *buf, uint32_t *len);

/**
 * @brief 解码升级失败数据单元
 * @param[in] buf 输入缓冲区
 * @param[in] len 输入数据长度
 * @param[out] fail_info 升级失败信息
 * @return 0: 成功, -1: 失败
 */
int tly_decode_upgrade_fail(const uint8_t *buf, uint32_t len,
                            tly_upgrade_fail_t *fail_info);

/**
 * @brief 发送升级失败上报
 * @param[in] app 应用上下文
 * @param[in] result 执行结果 (0=成功, 1=失败)
 * @param[in] error_code 失败错误码
 * @return 0: 成功, -1: 失败
 * @note 使用 TLY_CMD_UPGRADE_FAIL (0x83) 命令
 */
int tly_ota_send_upgrade_fail(tly_app_t *app, uint8_t result, uint8_t error_code);

/* ======================== 定时唤醒心跳位置包 (0x4F) ======================== */

typedef struct tly_wakeup_location {
    uint8_t  gps_status;       /* GPS定位状态: bit0=定位状态, bit1=南北纬, bit2=东西经 */
    uint32_t longitude;         /* 经度 ×10^6 */
    uint32_t latitude;          /* 纬度 ×10^6 */
    uint16_t altitude;          /* 海拔(米) */
    uint8_t  speed;             /* 速度(km/h) */
    uint16_t heading;           /* 航向角(度) */
    uint16_t  accel_x;           /* X轴加速度 */
    uint16_t  accel_y;           /* Y轴加速度 */
    uint16_t  accel_z;           /* Z轴加速度 */
    uint8_t  dev_status;        /* bit0-2=天线状态, bit3-7=网络信号强度 */
} tly_wakeup_location_t;

int tly_encode_wakeup_location(const tly_wakeup_location_t *loc,
                               uint8_t *buf, uint32_t *len);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_PROTOCOL_H__ */
