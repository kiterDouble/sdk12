/**
 * tsp_config.c - INI配置文件解析实现
 *
 * 支持简单的 [section] key = value 格式
 */

#include "tsp_config.h"
#include "tsp_log.h"
#include "tsp_device.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* 去除首尾空白 */
static char *str_trim(char *s)
{
    if (!s) return NULL;

    /* 去前导空白 */
    while (*s && isspace((unsigned char)*s)) s++;

    if (*s == '\0') return s;

    /* 去尾部空白 */
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';

    return s;
}

/* 去除行尾注释 (# 或 ;) */
static void strip_comment(char *line)
{
    int in_quote = 0;
    for (char *p = line; *p; p++) {
        if (*p == '"') in_quote = !in_quote;
        if (!in_quote && (*p == '#' || *p == ';')) {
            *p = '\0';
            return;
        }
    }
}

/* 在一行中查找 '=' 分割 key 和 value */
static int parse_kv(char *line, char **key, char **val)
{
    char *eq = strchr(line, '=');
    if (!eq) return -1;

    *eq = '\0';
    *key = str_trim(line);
    *val = str_trim(eq + 1);

    /* 去除value两端的引号 */
    size_t vlen = strlen(*val);
    if (vlen >= 2 && (*val)[0] == '"' && (*val)[vlen - 1] == '"') {
        (*val)[vlen - 1] = '\0';
        (*val)++;
    }

    return 0;
}

/**
 * 设置默认值
 */
static void config_set_defaults(tly_config_t *cfg)
{
    memset(cfg, 0, sizeof(*cfg));

    strncpy(cfg->tcp_host, "127.0.0.1", sizeof(cfg->tcp_host) - 1);
    cfg->tcp_port = 8809;
    cfg->tcp_reconnect_interval = 10;

    strncpy(cfg->http_base_url, "http://127.0.0.1:8080/api",
            sizeof(cfg->http_base_url) - 1);

    snprintf(cfg->vin, sizeof(cfg->vin), "%s", "LSNZLFYA1P1000001");
    memset(cfg->iccid, '0', TLY_ICCID_LEN);
    cfg->iccid[TLY_ICCID_LEN] = '\0';
    cfg->sw_version = 1;

    cfg->hw_version = 1;
    cfg->main_mcu_version = 1;
    cfg->sub_mcu_version = 1;
    cfg->non_stage4_version = 1;

    strncpy(cfg->offline_dir, "/var/lib/tly/offline", sizeof(cfg->offline_dir) - 1);
    cfg->offline_max_days = 3;

    strncpy(cfg->ota_save_dir, "/tmp/tly_ota", sizeof(cfg->ota_save_dir) - 1);
    strncpy(cfg->ota_unzip_dir, "/tmp/tly_ota/unzip", sizeof(cfg->ota_unzip_dir) - 1);
    strncpy(cfg->ota_master_config_dir, "/oemapp/config/ota_master", sizeof(cfg->ota_master_config_dir) - 1);
    cfg->ota_use_libcurl = 0;

    cfg->recorder_enable      = 0;
    strncpy(cfg->recorder_dir, "/data/can_log", sizeof(cfg->recorder_dir) - 1);
    cfg->recorder_max_size_mb = 50;
    cfg->recorder_sample_ms   = 10;
    cfg->recorder_channels    = 15;

    cfg->heartbeat_interval = 60;

    strncpy(cfg->log_level, "INFO", sizeof(cfg->log_level) - 1);
    strncpy(cfg->log_path, "/var/log/tly.log", sizeof(cfg->log_path) - 1);
}

/**
 * 按key设置配置项
 */
static void config_set(tly_config_t *cfg, const char *key, const char *val)
{
    if (!key || !val) return;

    if (strcmp(key, "host") == 0) {
        strncpy(cfg->tcp_host, val, sizeof(cfg->tcp_host) - 1);
    } else if (strcmp(key, "port") == 0) {
        cfg->tcp_port = atoi(val);
    } else if (strcmp(key, "reconnect_interval") == 0) {
        cfg->tcp_reconnect_interval = atoi(val);
        if (cfg->tcp_reconnect_interval < 1) cfg->tcp_reconnect_interval = 1;
    } else if (strcmp(key, "base_url") == 0) {
        strncpy(cfg->http_base_url, val, sizeof(cfg->http_base_url) - 1);
    } else if (strcmp(key, "vin") == 0) {
        strncpy(cfg->vin, val, sizeof(cfg->vin) - 1);
    } else if (strcmp(key, "iccid") == 0) {
        strncpy(cfg->iccid, val, sizeof(cfg->iccid) - 1);
    } else if (strcmp(key, "sw_version") == 0) {
        cfg->sw_version = (uint8_t)atoi(val);
    } else if (strcmp(key, "hw_version") == 0) {
        cfg->hw_version = (uint16_t)atoi(val);
    } else if (strcmp(key, "main_mcu_version") == 0) {
        cfg->main_mcu_version = (uint32_t)strtoul(val, NULL, 0);
    } else if (strcmp(key, "sub_mcu_version") == 0) {
        cfg->sub_mcu_version = (uint16_t)atoi(val);
    } else if (strcmp(key, "non_stage4_version") == 0) {
        cfg->non_stage4_version = (uint16_t)atoi(val);
    } else if (strcmp(key, "offline_dir") == 0) {
        strncpy(cfg->offline_dir, val, sizeof(cfg->offline_dir) - 1);
    } else if (strcmp(key, "offline_max_days") == 0) {
        cfg->offline_max_days = atoi(val);
        if (cfg->offline_max_days < 1) cfg->offline_max_days = 1;
    } else if (strcmp(key, "save_dir") == 0) {
        strncpy(cfg->ota_save_dir, val, sizeof(cfg->ota_save_dir) - 1);
    } else if (strcmp(key, "unzip_dir") == 0) {
        strncpy(cfg->ota_unzip_dir, val, sizeof(cfg->ota_unzip_dir) - 1);
    } else if (strcmp(key, "ota_master_config_dir") == 0) {
        strncpy(cfg->ota_master_config_dir, val, sizeof(cfg->ota_master_config_dir) - 1);
    } else if (strcmp(key, "use_libcurl") == 0) {
        cfg->ota_use_libcurl = atoi(val);
    } else if (strcmp(key, "recorder_enable") == 0) {
        cfg->recorder_enable = atoi(val);
    } else if (strcmp(key, "recorder_dir") == 0) {
        strncpy(cfg->recorder_dir, val, sizeof(cfg->recorder_dir) - 1);
    } else if (strcmp(key, "recorder_max_size_mb") == 0) {
        cfg->recorder_max_size_mb = atoi(val);
    } else if (strcmp(key, "recorder_sample_ms") == 0) {
        cfg->recorder_sample_ms = atoi(val);
    } else if (strcmp(key, "recorder_channels") == 0) {
        cfg->recorder_channels = atoi(val);
    } else if (strcmp(key, "interval") == 0) {
        cfg->heartbeat_interval = atoi(val);
        if (cfg->heartbeat_interval < 10) cfg->heartbeat_interval = 10;
    } else if (strcmp(key, "level") == 0) {
        strncpy(cfg->log_level, val, sizeof(cfg->log_level) - 1);
    } else if (strcmp(key, "path") == 0) {
        strncpy(cfg->log_path, val, sizeof(cfg->log_path) - 1);
    }
}

int tly_config_load(tly_config_t *cfg, const char *conf_file)
{
    if (!cfg || !conf_file) return TLY_ERR_PARAM;

    config_set_defaults(cfg);

    FILE *fp = fopen(conf_file, "r");
    if (!fp) {
        fprintf(stderr, "Config file %s not found, using defaults\n", conf_file);
        return 0;  /* 默认值仍然可用 */
    }

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        strip_comment(line);
        char *trimmed = str_trim(line);

        /* 跳过空行和节名 */
        if (trimmed[0] == '\0' || trimmed[0] == '[') continue;

        char *key = NULL, *val = NULL;
        if (parse_kv(trimmed, &key, &val) == 0) {
            config_set(cfg, key, val);
        }
    }

    fclose(fp);

    tly_device_get_vin(cfg->vin, sizeof(cfg->vin));
    tly_device_get_iccid(cfg->iccid, sizeof(cfg->iccid));
    cfg->sw_version         = tly_device_get_sw_version();
    cfg->hw_version         = tly_device_get_hw_version();
    cfg->main_mcu_version   = tly_device_get_main_mcu_version();
    cfg->sub_mcu_version    = tly_device_get_sub_mcu_version();
    cfg->non_stage4_version = tly_device_get_non_stage4_version();

    return 0;
}

void tly_config_dump(const tly_config_t *cfg)
{
    if (!cfg) return;

    TLY_LOG_I("========== Configuration ==========");
    TLY_LOG_I("  TCP: %s:%d, reconnect=%ds",
              cfg->tcp_host, cfg->tcp_port, cfg->tcp_reconnect_interval);
    TLY_LOG_I("  HTTP: %s", cfg->http_base_url);
    TLY_LOG_I("  VIN: %s, ICCID: %s, SW_VER: %u", cfg->vin, cfg->iccid, cfg->sw_version);
    TLY_LOG_I("  Version: hw=%u, main_mcu=%u, sub_mcu=%u, non4=%u",
              cfg->hw_version, cfg->main_mcu_version,
              cfg->sub_mcu_version, cfg->non_stage4_version);
    TLY_LOG_I("  Offline: dir=%s, max_days=%d", cfg->offline_dir, cfg->offline_max_days);
    TLY_LOG_I("  OTA: save_dir=%s, unzip_dir=%s, config_dir=%s, use_libcurl=%d",
              cfg->ota_save_dir, cfg->ota_unzip_dir, cfg->ota_master_config_dir, cfg->ota_use_libcurl);
    TLY_LOG_I("  Recorder: enable=%d, dir=%s, max=%dMB, sample=%dms, ch=0x%02X",
              cfg->recorder_enable, cfg->recorder_dir,
              cfg->recorder_max_size_mb, cfg->recorder_sample_ms, cfg->recorder_channels);
    TLY_LOG_I("  Heartbeat: %ds", cfg->heartbeat_interval);
    TLY_LOG_I("  Log: level=%s, path=%s", cfg->log_level, cfg->log_path);
    TLY_LOG_I("====================================");
}
