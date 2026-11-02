/**
 * tsp_log.c - 日志模块实现
 */

#include "tsp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

static tly_log_level_t g_log_level = TLY_LOG_INFO;
static FILE           *g_log_fp    = NULL;

static const char *level_strings[] = {
    "DEBUG", "INFO", "WARN", "ERROR",
};

tly_log_level_t tly_log_level_from_str(const char *str)
{
    if (!str) return TLY_LOG_INFO;

    if (strcasecmp(str, "DEBUG") == 0) return TLY_LOG_DEBUG;
    if (strcasecmp(str, "INFO")  == 0) return TLY_LOG_INFO;
    if (strcasecmp(str, "WARN")  == 0) return TLY_LOG_WARN;
    if (strcasecmp(str, "ERROR") == 0) return TLY_LOG_ERROR;

    return TLY_LOG_INFO;
}

int tly_log_init(tly_log_level_t level, const char *path)
{
    g_log_level = level;

    if (path && path[0] != '\0') {
        g_log_fp = fopen(path, "a");
        if (!g_log_fp) {
            fprintf(stderr, "Failed to open log file %s: %s\n",
                    path, strerror(errno));
            g_log_fp = NULL;
            return -1;
        }
        /* 行缓冲, 保证每条日志即时落盘 */
        setvbuf(g_log_fp, NULL, _IOLBF, 0);
    }

    return 0;
}

void tly_log(tly_log_level_t level, const char *fmt, ...)
{
    if (level < g_log_level) return;

    FILE *fp = g_log_fp ? g_log_fp : stdout;

    /* 时间戳 */
    time_t now = time(NULL);
    struct tm tm_buf;
    localtime_r(&now, &tm_buf);

    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);

    fprintf(fp, "%s [%s] ", time_str, level_strings[level]);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);

    fprintf(fp, "\n");
}

void tly_log_hex(tly_log_level_t level, const char *prefix,
                 const uint8_t *data, uint32_t len)
{
    if (level < g_log_level || !data || len == 0) return;

    FILE *fp = g_log_fp ? g_log_fp : stdout;

    time_t now = time(NULL);
    struct tm tm_buf;
    localtime_r(&now, &tm_buf);

    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);

    char hex[16 * 3 + 1];
    for (uint32_t off = 0; off < len; off += 16) {
        uint32_t chunk = len - off;
        if (chunk > 16) chunk = 16;

        int pos = 0;
        for (uint32_t i = 0; i < chunk; i++) {
            pos += snprintf(hex + pos, sizeof(hex) - (size_t)pos,
                            "%02X ", data[off + i]);
        }

        fprintf(fp, "%s [%s] %s %04X: %s\n",
                time_str, level_strings[level],
                prefix ? prefix : "", off, hex);
    }
}

void tly_log_close(void)
{
    if (g_log_fp) {
        fflush(g_log_fp);
        fclose(g_log_fp);
        g_log_fp = NULL;
    }
}
