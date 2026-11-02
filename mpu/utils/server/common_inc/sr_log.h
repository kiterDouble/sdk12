/**
 * @file sr_log.h
 * @author jiajiangze
 * @brief
 * @version 0.1
 * @date 2023-09-15
 *
 * @copyright Copyright (SiRun) 2023
 *
 */
#ifndef _SR_LOG_H
#define _SR_LOG_H

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include "zlog.h"

#define ZLOG_CONF_FILE_PATH ("/oemdata/log/log.conf")

#define LOG_INIT(rule) dzlog_init(ZLOG_CONF_FILE_PATH, rule)
#define LOG_Debug(...) dzlog_debug(__VA_ARGS__)
#define LOG_Info(...) dzlog_info(__VA_ARGS__)
#define LOG_Notice(...) dzlog_notice(__VA_ARGS__)
#define LOG_Warn(...) dzlog_warn(__VA_ARGS__)
#define LOG_Error(...) dzlog_error(__VA_ARGS__)
#define LOG_Fatal(...) dzlog_fatal(__VA_ARGS__)

#define LOG_DebugHex(data, len, text)                          \
    do                                                                    \
    {                                                                     \
        char strBuff[1024] = {0};                                         \
        uint16_t count = 0, i = 0;                                        \
        for (count = 0, i = 0; i < len && count <= 1021; i++, count += 3) \
        {                                                                 \
            snprintf(&strBuff[count], 1024 - count, "%02X ", data[i]);    \
        }                                                                 \
        dzlog_debug("%s:[%d]:%s", text, len, strBuff);                  \
    } while (0)

/**
 * @brief 运行时设置日志级别，传入参数支持"DEBUG", "INFO", "NOTICE", "WARN", "ERROR", "FATAL"
 * 当设置了一个级别后，进程中的所有日志都只会输出这个级别及以上的日志
 * 比如，设置了"NOTICE"，则之后只会输出"NOTICE", "WARN", "ERROR", "FATAL"这四个级别的日志
 * 日志级别从低到高依次是"DEBUG", "INFO", "NOTICE", "WARN", "ERROR", "FATAL"
 * 注意：该接口只能在LOG_INIT之后调用
 *
 */
#define LOG_SET_LEVEL(level)                                                       \
    do                                                                             \
    {                                                                              \
        FILE *file = fopen(ZLOG_CONF_FILE_PATH, "r");                              \
        if (!file)                                                                 \
        {                                                                          \
            LOG_Error("[LOG_SET_LEVEL]Failed to open log.conf");                   \
            break;                                                                 \
        }                                                                          \
        fseek(file, 0, SEEK_END);                                                  \
        long file_size = ftell(file);                                              \
        fseek(file, 0, SEEK_SET);                                                  \
        char *content = (char *)malloc(file_size + 1);                             \
        char *content_p = content;                                                 \
        if (!content || fread(content, 1, file_size, file) != file_size)           \
        {                                                                          \
            LOG_Error("[LOG_SET_LEVEL]Failed to read log.conf");                   \
            free(content);                                                         \
            fclose(file);                                                          \
            break;                                                                 \
        }                                                                          \
        content[file_size] = '\0';                                                 \
        fclose(file);                                                              \
        size_t newContentSize = file_size + 100;                                   \
        char *newContent = (char *)malloc(newContentSize);                         \
        if (!newContent)                                                           \
        {                                                                          \
            LOG_Error("[LOG_SET_LEVEL]Failed to allocate memory for new content"); \
            free(content);                                                         \
            break;                                                                 \
        }                                                                          \
        memset(newContent, 0, newContentSize);                                     \
        char *pos = content;                                                       \
        char *newPos = newContent;                                                 \
        const char *target = ".DEBUG";                                             \
        size_t targetLen = strlen(target);                                         \
        while ((pos = strstr(pos, target)) != NULL)                                \
        {                                                                          \
            size_t segmentLen = pos - content;                                     \
            memcpy(newPos, content, segmentLen);                                   \
            newPos += segmentLen;                                                  \
            memcpy(newPos, ".", 1);                                                \
            newPos += 1;                                                           \
            memcpy(newPos, level, strlen(level));                                  \
            newPos += strlen(level);                                               \
            pos += targetLen;                                                      \
            content = pos;                                                         \
        }                                                                          \
        strcpy(newPos, content);                                                   \
        zlog_reload(newContent);                                                   \
        free(content_p);                                                           \
        free(newContent);                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif
