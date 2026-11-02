/**
 * tsp_log.h - 日志模块
 */

#ifndef __TSP_LOG_H__
#define __TSP_LOG_H__

#include <stdarg.h>
#include <stdint.h>

#include "sr_log.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tly_log_level {
    TLY_LOG_DEBUG = 0,
    TLY_LOG_INFO,
    TLY_LOG_WARN,
    TLY_LOG_ERROR,
} tly_log_level_t;

/**
 * 初始化日志模块
 * @param level 最低输出级别
 * @param path  日志文件路径, NULL则输出到stdout
 * @return 0成功, 负数失败
 */
int  tly_log_init(tly_log_level_t level, const char *path);

/**
 * 写日志
 */
void tly_log(tly_log_level_t level, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

/**
 * 关闭日志模块, 释放资源
 */
void tly_log_close(void);

/**
 * 以hex格式打印原始数据到日志
 * @param level 日志级别
 * @param prefix 每行前缀描述
 * @param data  数据指针
 * @param len   数据长度
 */
void tly_log_hex(tly_log_level_t level, const char *prefix,
                 const uint8_t *data, uint32_t len);

/**
 * 将日志级别字符串转为枚举值
 */
tly_log_level_t tly_log_level_from_str(const char *str);

/* 便捷宏 */
#if 0
#define TLY_LOG_D(fmt, ...) tly_log(TLY_LOG_DEBUG, "[D] " fmt, ##__VA_ARGS__)
#define TLY_LOG_I(fmt, ...) tly_log(TLY_LOG_INFO,  "[I] " fmt, ##__VA_ARGS__)
#define TLY_LOG_W(fmt, ...) tly_log(TLY_LOG_WARN,  "[W] " fmt, ##__VA_ARGS__)
#define TLY_LOG_E(fmt, ...) tly_log(TLY_LOG_ERROR, "[E] " fmt, ##__VA_ARGS__)
#else
#define TLY_LOG_D(...) LOG_Debug(__VA_ARGS__)
#define TLY_LOG_I(...) LOG_Info(__VA_ARGS__)
#define TLY_LOG_W(...) LOG_Warn(__VA_ARGS__)
#define TLY_LOG_E(...) LOG_Error(__VA_ARGS__)
#define TLY_LOG_Dump(text, data, len) LOG_DebugHex(data, len, text)

#endif

#ifdef __cplusplus
}
#endif

#endif /* __TSP_LOG_H__ */
