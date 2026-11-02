#include <string.h>

#include "sr_log.h"
#include "time_server.h"
#include "time_status.h"

#define CFG_GROUP_TIME_JUMP_EVENT   "timeJumpEvent"
#define CFG_GROUP_NTP               "ntp"
#define CFG_GROUP_GNSS              "gnss"
#define CFG_GROUP_RTC               "rtc"
#define CFG_GROUP_TSP               "tsp"

#define CFG_KEY_THRESHOLD           "threshold"
#define CFG_KEY_HOSTS               "hosts"
#define CFG_KEY_ACTIVATION          "activation"
#define CFG_KEY_PRIORITY            "priority"
#define CFG_KEY_DEV                 "dev"

/**
 * @brief 解析时间管理配置文件
 *
 * 使用 GLib GKeyFile 解析 time.ini，读取各授时源配置。
 * 若某配置项缺失或解析失败，使用默认值（如 TIME_SOURCE_DEFAULT_ACTIVATION_SEC、TIME_JUMP_DEFAULT_THRESHOLD）。
 *
 * @param[in]  cfgPath 配置文件路径
 * @param[out] cfg     输出配置结构体
 * @return int32_t 0 表示成功，-1 表示失败
 */
int32_t TimeCfgParse(const char* cfgPath, TimeServerConfig_t* cfg)
{
    GKeyFile *keyFile = NULL;
    GError *error = NULL;
    int32_t ret = -1;

    LOG_Info("%s enter, cfgPath=%s", __func__, cfgPath ? cfgPath : "NULL");

    do {
        if (cfgPath == NULL || cfg == NULL) {
            LOG_Error("%s invalid param", __func__);
            break;
        }

        /* 初始化配置结构体 */
        memset(cfg, 0, sizeof(TimeServerConfig_t));

        /* 创建新的 GKeyFile 对象 */
        keyFile = g_key_file_new();
        if (keyFile == NULL) {
            LOG_Error("%s g_key_file_new failed", __func__);
            break;
        }

        /* 加载配置文件 */
        if (!g_key_file_load_from_file(keyFile, cfgPath, G_KEY_FILE_NONE, &error)) {
            LOG_Error("%s load cfg failed: %s", __func__, error ? error->message : "unknown");
            break;
        }

        /* 解析 timeJumpEvent 配置 */
        if (g_key_file_has_group(keyFile, CFG_GROUP_TIME_JUMP_EVENT)) {
            cfg->timeJumpEventThreshold = g_key_file_get_integer(keyFile, CFG_GROUP_TIME_JUMP_EVENT, CFG_KEY_THRESHOLD, &error);
            if (error != NULL) {
                cfg->timeJumpEventThreshold = TIME_JUMP_DEFAULT_THRESHOLD;
                g_clear_error(&error);
            }
        } else {
            cfg->timeJumpEventThreshold = TIME_JUMP_DEFAULT_THRESHOLD;
        }

        /* 解析 tsp 配置 */
        if (g_key_file_has_group(keyFile, CFG_GROUP_TSP)) {
            /* 解析 TSP 活性时间 */
            if (g_key_file_has_key(keyFile, CFG_GROUP_TSP, CFG_KEY_ACTIVATION, NULL)) {
                cfg->tspCfg.priorityActivation.activation = g_key_file_get_integer(keyFile, CFG_GROUP_TSP, CFG_KEY_ACTIVATION, &error);
                if (error != NULL) {
                    cfg->tspCfg.priorityActivation.activation = TIME_SOURCE_DEFAULT_ACTIVATION_SEC;
                    g_clear_error(&error);
                }
            } else {
                cfg->tspCfg.priorityActivation.activation = TIME_SOURCE_DEFAULT_ACTIVATION_SEC;
            }

            /* 解析 TSP 优先级 */
            if (g_key_file_has_key(keyFile, CFG_GROUP_TSP, CFG_KEY_PRIORITY, NULL)) {
                cfg->tspCfg.priorityActivation.priority = g_key_file_get_integer(keyFile, CFG_GROUP_TSP, CFG_KEY_PRIORITY, &error);
                if (error != NULL) {
                    cfg->tspCfg.priorityActivation.priority = 0;
                    g_clear_error(&error);
                }
            } else {
                cfg->tspCfg.priorityActivation.priority = 0;
            }
        }

        /* 解析 ntp 配置 */
        if (g_key_file_has_group(keyFile, CFG_GROUP_NTP)) {
            gchar **hostsList = NULL;
            gsize hostCount = 0;
            int hostIndex = 0;

            /* 解析 NTP 服务器地址列表 (GKeyFile 原生支持 ; 或 , 分隔的列表) */
            hostsList = g_key_file_get_string_list(keyFile, CFG_GROUP_NTP, CFG_KEY_HOSTS, &hostCount, &error);
            if (hostsList != NULL && error == NULL) {
                for (gsize i = 0; i < hostCount && hostIndex < TIME_SOURCE_NTP_HOST_MAX_NUM; i++) {
                    gchar *host = g_strstrip(hostsList[i]);
                    if (host != NULL && host[0] != '\0') {
                        strncpy(cfg->ntpCfg.host[hostIndex].name, host, TIME_SOURCE_NTP_HOST_BUF_MAX_SIZE - 1);
                        cfg->ntpCfg.host[hostIndex].name[TIME_SOURCE_NTP_HOST_BUF_MAX_SIZE - 1] = '\0';
                        hostIndex++;
                    }
                }
                cfg->ntpCfg.hostNum = hostIndex;
                g_strfreev(hostsList);
            } else {
                if (error != NULL) {
                    g_clear_error(&error);
                }
                cfg->ntpCfg.hostNum = 0;
            }

            /* 解析 NTP 活性时间 */
            if (g_key_file_has_key(keyFile, CFG_GROUP_NTP, CFG_KEY_ACTIVATION, NULL)) {
                cfg->ntpCfg.priorityActivation.activation = g_key_file_get_integer(keyFile, CFG_GROUP_NTP, CFG_KEY_ACTIVATION, &error);
                if (error != NULL) {
                    cfg->ntpCfg.priorityActivation.activation = TIME_SOURCE_DEFAULT_ACTIVATION_SEC;
                    g_clear_error(&error);
                }
            } else {
                cfg->ntpCfg.priorityActivation.activation = TIME_SOURCE_DEFAULT_ACTIVATION_SEC;
            }

            /* 解析 NTP 优先级 */
            if (g_key_file_has_key(keyFile, CFG_GROUP_NTP, CFG_KEY_PRIORITY, NULL)) {
                cfg->ntpCfg.priorityActivation.priority = g_key_file_get_integer(keyFile, CFG_GROUP_NTP, CFG_KEY_PRIORITY, &error);
                if (error != NULL) {
                    cfg->ntpCfg.priorityActivation.priority = 0;
                    g_clear_error(&error);
                }
            } else {
                cfg->ntpCfg.priorityActivation.priority = 0;
            }
        }

        /* 解析 gnss 配置 */
        if (g_key_file_has_group(keyFile, CFG_GROUP_GNSS)) {
            /* 解析 GNSS 活性时间 */
            if (g_key_file_has_key(keyFile, CFG_GROUP_GNSS, CFG_KEY_ACTIVATION, NULL)) {
                cfg->gnssCfg.priorityActivation.activation = g_key_file_get_integer(keyFile, CFG_GROUP_GNSS, CFG_KEY_ACTIVATION, &error);
                if (error != NULL) {
                    cfg->gnssCfg.priorityActivation.activation = TIME_SOURCE_DEFAULT_ACTIVATION_SEC;
                    g_clear_error(&error);
                }
            } else {
                cfg->gnssCfg.priorityActivation.activation = TIME_SOURCE_DEFAULT_ACTIVATION_SEC;
            }

            /* 解析 GNSS 优先级 */
            if (g_key_file_has_key(keyFile, CFG_GROUP_GNSS, CFG_KEY_PRIORITY, NULL)) {
                cfg->gnssCfg.priorityActivation.priority = g_key_file_get_integer(keyFile, CFG_GROUP_GNSS, CFG_KEY_PRIORITY, &error);
                if (error != NULL) {
                    cfg->gnssCfg.priorityActivation.priority = 0;
                    g_clear_error(&error);
                }
            } else {
                cfg->gnssCfg.priorityActivation.priority = 0;
            }
        }

        /* 解析 rtc 配置 */
        if (g_key_file_has_group(keyFile, CFG_GROUP_RTC)) {
            gchar *devStr = g_key_file_get_string(keyFile, CFG_GROUP_RTC, CFG_KEY_DEV, &error);
            if (devStr != NULL && error == NULL) {
                strncpy(cfg->rtcDev, devStr, sizeof(cfg->rtcDev) - 1);
                cfg->rtcDev[sizeof(cfg->rtcDev) - 1] = '\0';
                g_free(devStr);
            } else {
                if (error != NULL) {
                    g_clear_error(&error);
                }
            }
        }

        ret = 0;
    } while (0);

    /* 释放 GKeyFile 对象 */
    if (keyFile != NULL) {
        g_key_file_free(keyFile);
    }
    if (error != NULL) {
        g_error_free(error);
    }

    if (ret == 0) {
        LOG_Info("%s timeJumpEventThreshold=%d", __func__, cfg->timeJumpEventThreshold);
        LOG_Info("%s  ntp cfg: hostNum=%d, activation=%d, priority=%d", __func__,
                cfg->ntpCfg.hostNum,
                cfg->ntpCfg.priorityActivation.activation,
                cfg->ntpCfg.priorityActivation.priority);
        for (int i = 0; i < cfg->ntpCfg.hostNum; i++) {
            LOG_Info("%s      host[%d]=%s", __func__, i, cfg->ntpCfg.host[i].name);
        }
        LOG_Info("%s  tsp cfg: activation=%d, priority=%d",__func__,
                cfg->tspCfg.priorityActivation.activation,
                cfg->tspCfg.priorityActivation.priority);
        LOG_Info("%s gnss cfg: activation=%d, priority=%d", __func__,
                cfg->gnssCfg.priorityActivation.activation,
                cfg->gnssCfg.priorityActivation.priority);
        LOG_Info("%s  rtc cfg: dev=%s", __func__, cfg->rtcDev);
    } else {
        LOG_Error("%s failed, ret=%d", __func__, ret);
    }

    LOG_Info("%s exit, ret=%d", __func__, ret);

    return ret;
}

/**
 * @brief 获取指定授时源的配置活性时间
 *
 * @param[in] cfg        配置结构体指针
 * @param[in] timeSource 授时源类型（E_TIME_SYNC_SRC_NTP/GNSS/TSP）
 * @return int32_t 活性时间（秒），默认返回 TIME_SOURCE_DEFAULT_ACTIVATION_SEC
 */
int32_t TimeCfgGetSyncSourceActivation(TimeServerConfig_t* cfg, uint8_t timeSource)
{
    switch (timeSource) {
        case E_TIME_SYNC_SRC_NTP:
            return cfg->ntpCfg.priorityActivation.activation;
        case E_TIME_SYNC_SRC_GNSS:
            return cfg->gnssCfg.priorityActivation.activation;
        case E_TIME_SYNC_SRC_TSP:
            return cfg->tspCfg.priorityActivation.activation;
        default:
            return TIME_SOURCE_DEFAULT_ACTIVATION_SEC;
    }
}

/**
 * @brief 获取指定授时源的配置优先级
 *
 * @param[in] cfg        配置结构体指针
 * @param[in] timeSource 授时源类型（E_TIME_SYNC_SRC_NTP/GNSS/TSP）
 * @return int32_t 优先级，默认返回 0
 */
int32_t TimeCfgGetSyncSourcePriority(TimeServerConfig_t* cfg, uint8_t timeSource)
{
    switch (timeSource) {
        case E_TIME_SYNC_SRC_NTP:
            return cfg->ntpCfg.priorityActivation.priority;
        case E_TIME_SYNC_SRC_GNSS:
            return cfg->gnssCfg.priorityActivation.priority;
        case E_TIME_SYNC_SRC_TSP:
            return cfg->tspCfg.priorityActivation.priority;
        default:
            return 0;
    }
}
