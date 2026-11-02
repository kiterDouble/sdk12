#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <glib.h>

#include "log_pack_cfg.h"
#include "sr_log.h"


/**
 * 读取带行内注释的数值字符串，并转换为 uint64。
 * GKeyFile 不识别行内 '#' 注释，对数值项会直接解析失败；
 * 先按字符串读出，再用 strtoull 转换可兼容 "5242880 #注释" 这类写法。
 */
static uint64_t GetUint64FromKeyFile(GKeyFile* keyFile, const char* group,
                                     const char* key, uint64_t defaultVal)
{
    char* strVal = NULL;
    char* endPtr = NULL;
    uint64_t val = defaultVal;

    if (keyFile == NULL || group == NULL || key == NULL) {
        return defaultVal;
    }

    strVal = g_key_file_get_string(keyFile, group, key, NULL);
    if (strVal != NULL) {
        errno = 0;
        val = strtoull(strVal, &endPtr, 10);
        if (endPtr == strVal || errno != 0) {
            val = defaultVal;
        }
        g_free(strVal);
    }

    return val;
}

static int32_t GetIntFromKeyFile(GKeyFile* keyFile, const char* group,
                                 const char* key, int32_t defaultVal)
{
    char* strVal = NULL;
    char* endPtr = NULL;
    long val = defaultVal;

    if (keyFile == NULL || group == NULL || key == NULL) {
        return defaultVal;
    }

    strVal = g_key_file_get_string(keyFile, group, key, NULL);
    if (strVal != NULL) {
        errno = 0;
        val = strtol(strVal, &endPtr, 10);
        if (endPtr == strVal || errno != 0 || val < INT32_MIN || val > INT32_MAX) {
            val = defaultVal;
        }
        g_free(strVal);
    }

    return (int32_t)val;
}

/**
 * 去掉字符串首尾的引号，并截断引号后的行内注释。
 * GKeyFile 对带引号的值不会自动去掉行内 '#'/';' 注释，
 * 例如 "SRTL1234" #加密密码 会被完整读入；本函数提取第一个
 * 双引号对之间的内容，丢弃后续字符。
 */
static void StripQuotes(char* str)
{
    char* start = NULL;
    char* end = NULL;
    size_t len = 0;

    if (str == NULL) {
        return;
    }

    start = strchr(str, '"');
    if (start == NULL) {
        return;
    }

    end = strchr(start + 1, '"');
    if (end == NULL) {
        return;
    }

    len = (size_t)(end - start - 1);
    memmove(str, start + 1, len);
    str[len] = '\0';
}

static void GetActiveFileNameFromRotated(const char* rotated, char* active, size_t activeSize)
{
    size_t len = 0;

    if (rotated == NULL || active == NULL || activeSize == 0) {
        return;
    }

    len = strlen(rotated);
    if (len >= activeSize) {
        LOG_Warn("[%s] rotated name too long, truncate", __func__);
        snprintf(active, activeSize, "%s", rotated);
        return;
    }

    snprintf(active, activeSize, "%s", rotated);
    if (len > 2 && strcmp(rotated + len - 2, ".0") == 0) {
        active[len - 2] = '\0';
    }
}

/**
 * 去掉路径末尾的 '/'，避免后续拼接路径时出现双斜杠。
 * 注意：只修改 str 指向的可写缓冲区，不分配新内存。
 */
static void StripTrailingSlash(char* str)
{
    size_t len = 0;

    if (str == NULL) {
        return;
    }

    len = strlen(str);
    while (len > 1 && str[len - 1] == '/') {
        str[len - 1] = '\0';
        len--;
    }
}

int32_t LogPackLoadCfg(const char* cfgPath, LogSrCtx_t* ctx)
{
    GKeyFile* keyFile = NULL;
    GError* error = NULL;
    int32_t result = -1;
    uint64_t logNum = 0;
    uint32_t i = 0;
    char groupName[32] = {0};
    char* baseCryptoName = NULL;

    do {
        if (ctx == NULL || cfgPath == NULL) {
            LOG_Error("invalid param");
            break;
        }

        if (ctx->tasks != NULL) {
            free(ctx->tasks);
            ctx->tasks = NULL;
            ctx->taskCnt = 0;
        }

        keyFile = g_key_file_new();
        if (keyFile == NULL) {
            LOG_Error("g_key_file_new failed");
            break;
        }

        if (!g_key_file_load_from_file(keyFile, cfgPath, G_KEY_FILE_NONE, &error)) {
            LOG_Error("load cfg[%s] failed: %s", cfgPath, error ? error->message : "unknown");
            break;
        }

        LOG_Info("load cfg from [%s]", cfgPath);

        logNum = GetUint64FromKeyFile(keyFile, "base", "logNum", 0);
        if (logNum == 0 || logNum > LOG_PACK_TASK_MAX_NUM) {
            LOG_Error("invalid logNum:%llu", (unsigned long long)logNum);
            break;
        }

        ctx->taskCnt = (uint32_t)logNum;

        /* 解析全局检查间隔，缺失或非法时默认 10 秒 */
        ctx->checkInterval = (uint32_t)GetUint64FromKeyFile(keyFile, "base", "checkInterval", 10);
        if (ctx->checkInterval == 0) {
            ctx->checkInterval = 10;
        }

        /* 解析开机/唤醒后禁止打包的延迟时长，缺失或非法时使用默认值 */
        ctx->bootUpDelay = (uint32_t)GetUint64FromKeyFile(keyFile, "base", "bootUpDelay", 60);
        if (ctx->bootUpDelay == 0) {
            ctx->bootUpDelay = 60;
        }

        ctx->wakeUpDelay = (uint32_t)GetUint64FromKeyFile(keyFile, "base", "wakeUpDelay", 30);
        if (ctx->wakeUpDelay == 0) {
            ctx->wakeUpDelay = 30;
        }

        /* 解析全局加密方式名称，由 log_pack_main.c 解析为具体接口 */
        baseCryptoName = g_key_file_get_string(keyFile, "base", "crypto", NULL);
        if (baseCryptoName != NULL) {
            StripQuotes(baseCryptoName);
            strncpy(ctx->cryptoName, baseCryptoName, sizeof(ctx->cryptoName) - 1);
        }

        ctx->tasks = (LogSrPackTask_t*)calloc(ctx->taskCnt, sizeof(LogSrPackTask_t));
        if (ctx->tasks == NULL) {
            LOG_Error("calloc tasks failed");
            break;
        }

        for (i = 0; i < ctx->taskCnt; i++) {
            LogSrPackTask_t* task = &ctx->tasks[i];
            char* name = NULL;
            char* password = NULL;
            char* logPath = NULL;
            char* logPackPath = NULL;
            char* rotatedName = NULL;
            int32_t cryptoFlagVal = 0;

            snprintf(groupName, sizeof(groupName), "log%u", i);

            name = g_key_file_get_string(keyFile, groupName, "name", NULL);
            password = g_key_file_get_string(keyFile, groupName, "cryptoPassword", NULL);
            logPath = g_key_file_get_string(keyFile, groupName, "logPath", NULL);
            logPackPath = g_key_file_get_string(keyFile, groupName, "logPackPath", NULL);
            rotatedName = g_key_file_get_string(keyFile, groupName, "logRotatedFileName", NULL);

            StripQuotes(name);
            StripQuotes(password);
            StripQuotes(logPath);
            StripQuotes(logPackPath);
            StripQuotes(rotatedName);

            /* 去掉路径末尾斜杠，避免拼接时产生双斜杠导致 glob 失败 */
            StripTrailingSlash(logPath);
            StripTrailingSlash(logPackPath);

            if (name != NULL) {
                strncpy(task->name, name, sizeof(task->name) - 1);
            }
            if (password != NULL && password[0] != '\0') {
                strncpy(task->cryptoPassword, password, sizeof(task->cryptoPassword) - 1);
            } else {
                strncpy(task->cryptoPassword, LOG_CRYPTO_DEFAULT_PASSWORD, sizeof(task->cryptoPassword) - 1);
                LOG_Warn("[%s] task[%s] cryptoPassword is empty, use default", __func__, task->name);
            }
            if (logPath != NULL) {
                strncpy(task->logPath, logPath, sizeof(task->logPath) - 1);
            }
            if (logPackPath != NULL) {
                strncpy(task->logPackPath, logPackPath, sizeof(task->logPackPath) - 1);
            }
            if (rotatedName != NULL) {
                strncpy(task->logRotatedFileName, rotatedName, sizeof(task->logRotatedFileName) - 1);
                GetActiveFileNameFromRotated(rotatedName, task->logActiveFileName, sizeof(task->logActiveFileName));
            }

            task->maxStoreSize = GetUint64FromKeyFile(keyFile, groupName, "maxStoreSize", LOG_PACK_MAX_STORE_SIZE_DEFAULT);
            if (task->maxStoreSize == 0) {
                LOG_Warn("[%s] task[%s] maxStoreSize is 0, use default:%llu",
                         __func__, task->name,
                         (unsigned long long)LOG_PACK_MAX_STORE_SIZE_DEFAULT);
                task->maxStoreSize = LOG_PACK_MAX_STORE_SIZE_DEFAULT;
            }
            if (task->maxStoreSize < LOG_PACK_MAX_STORE_SIZE_MIN) {
                LOG_Warn("[%s] task[%s] maxStoreSize:%llu less than min:%llu, use min",
                         __func__, task->name,
                         (unsigned long long)task->maxStoreSize,
                         (unsigned long long)LOG_PACK_MAX_STORE_SIZE_MIN);
                task->maxStoreSize = LOG_PACK_MAX_STORE_SIZE_MIN;
            }
            /**
             * 解析 task 级加密开关。
             * 具体使用哪种加密接口由 [base] 中的 crypto 和 log_pack_main.c 决定。
             */
            cryptoFlagVal = GetIntFromKeyFile(keyFile, groupName, "cryptoFlag", 0);
            if (cryptoFlagVal != 0 && cryptoFlagVal != 1) {
                LOG_Warn("[%s] task[%s] invalid cryptoFlag:%d, use default 0", __func__, task->name, cryptoFlagVal);
                cryptoFlagVal = 0;
            }
            task->cryptoFlag = (uint8_t)cryptoFlagVal;
            task->index = 0;

            g_free(name);
            g_free(password);
            g_free(logPath);
            g_free(logPackPath);
            g_free(rotatedName);
        }

        result = 0;
    } while (0);

    if (result != 0 && ctx != NULL && ctx->tasks != NULL) {
        free(ctx->tasks);
        ctx->tasks = NULL;
        ctx->taskCnt = 0;
    }

    if (baseCryptoName != NULL) {
        g_free(baseCryptoName);
    }

    if (error != NULL) {
        g_error_free(error);
    }
    if (keyFile != NULL) {
        g_key_file_free(keyFile);
    }

    return result;
}
