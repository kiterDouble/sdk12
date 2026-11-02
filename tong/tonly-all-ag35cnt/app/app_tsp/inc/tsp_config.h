/**
 * tsp_config.h - INI配置文件解析
 */

#ifndef __TSP_CONFIG_H__
#define __TSP_CONFIG_H__

#include "tsp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TLY_PATH_MAX  256
#define TLY_HOST_MAX  128
#define TLY_URL_MAX   256
#define TLY_VIN_MAX   18

typedef struct tly_config {
    /* TCP连接 */
    char     tcp_host[TLY_HOST_MAX];
    int      tcp_port;
    int      tcp_reconnect_interval;   /* 秒, 默认10 */

    /* HTTP服务 */
    char     http_base_url[TLY_URL_MAX];

    /* 终端信息 */
    char     vin[TLY_VIN_MAX];
    char     iccid[TLY_ICCID_LEN + 1];   /* SIM卡ICCID号 */
    uint8_t  sw_version;

    /* 版本信息(用于版本号上传 0x78) */
    uint16_t hw_version;               /* 硬件版本号, 默认1 */
    uint32_t main_mcu_version;         /* 主MCU版本, 默认1 */
    uint16_t sub_mcu_version;          /* 从MCU版本, 默认1 */
    uint16_t non_stage4_version;       /* 非四版本, 默认1 */

    /* 离线存储 */
    char     offline_dir[TLY_PATH_MAX];
    int      offline_max_days;          /* 最大存储天数, 默认3 */

    /* OTA升级 */
    char     ota_save_dir[TLY_PATH_MAX];
    char     ota_unzip_dir[TLY_PATH_MAX];           /* OTA解压目录 */
    char     ota_master_config_dir[TLY_PATH_MAX];   /* OTA Master配置目录 */
    int      ota_use_libcurl;

    /* CAN录制 */
    int      recorder_enable;
    char     recorder_dir[TLY_PATH_MAX];
    int      recorder_max_size_mb;
    int      recorder_sample_ms;
    int      recorder_channels;              /* 0=curl命令, 1=libcurl API */

    /* 心跳 */
    int      heartbeat_interval;       /* 秒, 默认60 */

    /* 日志 */
    char     log_level[16];
    char     log_path[TLY_PATH_MAX];
} tly_config_t;

/**
 * 加载配置文件
 * @param cfg       输出配置结构
 * @param conf_file INI配置文件路径
 * @return 0成功, 负数失败
 */
int  tly_config_load(tly_config_t *cfg, const char *conf_file);

/**
 * 打印当前配置到日志
 */
void tly_config_dump(const tly_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_CONFIG_H__ */
