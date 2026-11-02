/**
 * tsp_offline_db.h - SQLite离线数据库存储与补发
 *
 * 使用SQLite数据库存储离线数据，支持两个数据库：
 * 1. 触发报警数据库 (alarm) - 高优先级，优先补发
 * 2. 实时数据库 (realtime) - 普通优先级
 *
 * 数据库表结构:
 *   - id: INTEGER PRIMARY KEY AUTOINCREMENT
 *   - timestamp: INTEGER (秒级时间戳)
 *   - data_type: INTEGER (数据类型，如TLY_INFO_ALARM等)
 *   - data: BLOB (原始数据内容)
 *   - data_len: INTEGER (数据长度)
 */

#ifndef __TSP_OFFLINE_DB_H__
#define __TSP_OFFLINE_DB_H__

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 数据库大小限制 (MB) */
#define TLY_DB_MAX_SIZE_MB      100  /* 单个数据库最大大小 */
#define TLY_DB_WARNING_SIZE_MB  80   /* 告警阈值 */

/* 数据库类型 */
typedef enum {
    TLY_DB_TYPE_ALARM    = 0,  /* 触发报警数据库 - 高优先级 */
    TLY_DB_TYPE_REALTIME = 1,  /* 实时数据库 - 普通优先级 */
    TLY_DB_TYPE_COUNT    = 2,  /* 数据库数量 */
} tly_db_type_t;

/* 离线记录结构 */
typedef struct tly_offline_db_record {
    int64_t  id;               /* 记录ID */
    time_t   timestamp;        /* 时间戳 */
    uint32_t data_type;        /* 数据类型 */
    uint32_t data_len;         /* 数据长度 */
} tly_offline_db_record_t;

/* 离线统计结构 */
typedef struct tly_offline_db_stats {
    uint32_t alarm_count;      /* 触发报警记录数 */
    uint32_t realtime_count;   /* 实时记录数 */
    time_t   alarm_oldest;     /* 触发报警最早时间 */
    time_t   alarm_newest;     /* 触发报警最新时间 */
    time_t   realtime_oldest;  /* 实时最早时间 */
    time_t   realtime_newest;  /* 实时最新时间 */
} tly_offline_db_stats_t;

/**
 * 初始化离线数据库
 * @param base_dir 数据库存储目录
 * @param max_days 最大存储天数
 * @return 0成功, -1失败
 */
int tly_offline_db_init(const char *base_dir, int max_days);

/**
 * 关闭离线数据库
 */
void tly_offline_db_deinit(void);

/**
 * 存储一条离线记录
 * @param db_type  数据库类型 (TLY_DB_TYPE_ALARM 或 TLY_DB_TYPE_REALTIME)
 * @param data     数据内容
 * @param data_len 数据长度
 * @param data_type 数据类型 (如 TLY_INFO_ALARM, TLY_INFO_FAST 等)
 * @return 0成功, -1失败
 */
int tly_offline_db_save(tly_db_type_t db_type,
                        const uint8_t *data, uint32_t data_len,
                        uint32_t data_type);

/**
 * 取出最早的一条待发记录（优先从触发报警数据库取）
 * @param record   输出记录信息
 * @param buf      输出数据缓冲区
 * @param buf_size 缓冲区大小
 * @param out_db_type 输出实际取出的数据库类型
 * @return 0成功, -1无记录, 其他错误码
 */
int tly_offline_db_pop_oldest(tly_offline_db_record_t *record,
                              uint8_t *buf, uint32_t buf_size,
                              tly_db_type_t *out_db_type);

/**
 * 删除已成功发送的记录
 * @param db_type  数据库类型
 * @param id       记录ID
 * @return 0成功, -1失败
 */
int tly_offline_db_delete(tly_db_type_t db_type, int64_t id);

/**
 * 获取离线数据统计
 * @param stats 输出统计信息
 * @return 0成功, -1失败
 */
int tly_offline_db_get_stats(tly_offline_db_stats_t *stats);

/**
 * 清理过期记录
 * @return 清理的记录总数
 */
int tly_offline_db_cleanup(void);

/**
 * 获取指定数据库的待发送记录数
 * @param db_type 数据库类型
 * @return 记录数, -1表示错误
 */
int tly_offline_db_get_count(tly_db_type_t db_type);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_OFFLINE_DB_H__ */
