/**
 * @file logger_cfg.cpp
 * @brief 基于 GLib GKeyFile 的 INI 配置加载实现
 *
 * @details 段名固定为 "canLogger"，缺字段时使用 LoggerCfg 默认值。
 */
#include "logger_cfg.h"

#include <glib.h>

#include "err_code.h"
#include "sr_log.h"

/**
 * @brief 从 INI 文件加载配置
 *
 * @details 单字段读取失败时清错误并保留 cfg 中已设默认值；
 *          仅文件无法打开或非 INI 格式时返回 RET_ERR。
 *
 * @param[out] cfg  输出配置
 * @param[in]  path INI 文件绝对路径
 * @return RET_OK 成功，RET_ERR 失败
 */
int LoadLoggerConfig(LoggerCfg &cfg, const std::string &path)
{
    GKeyFile *kf = g_key_file_new();
    if (!kf) {
        LOG_Error("g_key_file_new failed");
        return RET_ERR;
    }

    GError *err = NULL;
    if (!g_key_file_load_from_file(kf, path.c_str(), G_KEY_FILE_NONE, &err)) {
        LOG_Error("load %s failed: %s", path.c_str(),
                  err ? err->message : "unknown");
        if (err) g_error_free(err);
        g_key_file_free(kf);
        return RET_ERR;
    }

    static const char *SEC = "canLogger";
    GError *e2 = NULL;

    cfg.enable = (g_key_file_get_integer(kf, SEC, "enable", &e2) != 0);
    if (e2) { cfg.enable = true;  g_clear_error(&e2); }

    gchar *s = g_key_file_get_string(kf, SEC, "filePath", &e2);
    if (e2) { g_clear_error(&e2); }
    else if (s) { cfg.filePath = s; g_free(s); }

    s = g_key_file_get_string(kf, SEC, "filePrefix", &e2);
    if (e2) { g_clear_error(&e2); }
    else if (s) { cfg.filePrefix = s; g_free(s); }

    cfg.maxSizeMB = (uint32_t)g_key_file_get_integer(kf, SEC, "maxSizeMB", &e2);
    if (e2) { cfg.maxSizeMB = 100u; g_clear_error(&e2); }

    cfg.maxFileCount = (uint32_t)g_key_file_get_integer(kf, SEC, "maxFileCount", &e2);
    if (e2) { cfg.maxFileCount = 10u; g_clear_error(&e2); }

    cfg.maxTotalSizeMB = (uint32_t)g_key_file_get_integer(kf, SEC, "maxTotalSizeMB", &e2);
    if (e2) { cfg.maxTotalSizeMB = 5120u; g_clear_error(&e2); }

    cfg.flushIntervalMs = (uint32_t)g_key_file_get_integer(kf, SEC, "flushIntervalMs", &e2);
    if (e2) { cfg.flushIntervalMs = 500u; g_clear_error(&e2); }

    cfg.channelMask = (uint32_t)g_key_file_get_integer(kf, SEC, "channelMask", &e2);
    if (e2) { cfg.channelMask = 0xFFFFFFFFu; g_clear_error(&e2); }

    s = g_key_file_get_string(kf, SEC, "logRule", &e2);
    if (e2) { g_clear_error(&e2); }
    else if (s) { cfg.logRule = s; g_free(s); }

    g_key_file_free(kf);
    return RET_OK;
}
