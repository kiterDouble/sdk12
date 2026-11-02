/**
 * tsp_http.h - HTTP REST API客户端
 *
 * 封装IOT平台的7个HTTP接口(均为GET方式, JSON返回)
 */

#ifndef __TSP_HTTP_H__
#define __TSP_HTTP_H__

#include "tsp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== HTTP响应 ======================== */

#define TLY_HTTP_CODE_SUCCESS 0
#define TLY_HTTP_CODE_FAIL    500
#define TLY_HTTP_CODE_WARN    301

typedef struct tly_http_resp {
    int   code;          /* 0=成功, 500=失败, 301=警告 */
    char *msg;           /* 描述信息 */
    char *data_json;     /* data字段原始JSON字符串 */
} tly_http_resp_t;

/** 释放响应资源 */
void tly_http_resp_free(tly_http_resp_t *resp);

/** curl全局初始化(应用启动时调用一次) */
int  tly_http_global_init(void);

/** curl全局清理(应用退出时调用) */
void tly_http_global_cleanup(void);

/* ======================== 锁车指令 ======================== */

typedef struct tly_lock_params {
    const char *tbox_vin;       /* 终端号(必填) */
    int         lock_type;      /* 锁车类型(必填) */
    int         engine_speed;   /* 发动机转速(选填, 默认800) */
    int         torque;         /* 扭矩(选填, 仅玉柴) */
    int         engine_type;    /* 发动机类型(选填) */
    const char *operator_name;  /* 操作人(选填) */
    const char *vin;            /* 车架号(选填) */
} tly_lock_params_t;

int tly_http_lock(tly_http_resp_t *resp,
                  const char *base_url,
                  const tly_lock_params_t *params);

/* ======================== 解绑指令 ======================== */

int tly_http_unbind(tly_http_resp_t *resp,
                    const char *base_url,
                    const char *new_tbox_vin,
                    const char *old_tbox_vin,
                    int engine_type,
                    const char *operator_name);

/* ======================== OTA升级 ======================== */

typedef struct tly_ota_params {
    const char *tbox_vin;     /* 终端号 */
    const char *ftp_url;      /* FTP文件地址 */
    const char *username;     /* FTP账号 */
    const char *password;     /* FTP密码 */
    int         vehicle_type; /* 0=油车, 1=电车, 3=从机 */
    int         upgrade_type; /* 1=休眠, 2=立即, 3=强制 */
    int         device_type;  /* 1=TBOX, 2=ECU, 3=VCU */
} tly_ota_params_t;

int tly_http_ota_upgrade(tly_http_resp_t *resp,
                         const char *base_url,
                         const tly_ota_params_t *params);

/* ======================== 获取SD卡数据 ======================== */

int tly_http_get_sd_data(tly_http_resp_t *resp,
                         const char *base_url,
                         const char *tbox_vin,
                         const char *ftp_url,
                         const char *username,
                         const char *password,
                         int vehicle_type,
                         const char *begin_time,
                         int time_count);

/* ======================== Seed指令(潍柴秘钥) ======================== */

int tly_http_seed_key(tly_http_resp_t *resp,
                      const char *base_url,
                      const char *tbox_vin,
                      const char *seed);

/* ======================== 主动授时 ======================== */

int tly_http_time_sync(tly_http_resp_t *resp,
                       const char *base_url,
                       const char *tbox_vin,
                       const char *cmd_type,
                       const char *vehicle_type,
                       int time_second);

/* ======================== 电池加热 ======================== */

int tly_http_battery_heat(tly_http_resp_t *resp,
                          const char *base_url,
                          const char *tbox_vin,
                          const char *cmd_type);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_HTTP_H__ */
