#ifndef __TSP_OFFLINE_H__
#define __TSP_OFFLINE_H__

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TLY_OFFLINE_NAME_MAX  64

typedef struct tly_offline_record {
    char     filename[TLY_OFFLINE_NAME_MAX];
    time_t   timestamp;
    uint32_t data_len;
} tly_offline_record_t;

typedef struct tly_offline_stats {
    uint32_t total_count;
    time_t   oldest_time;
    time_t   newest_time;
} tly_offline_stats_t;

/**
 * 初始化离线存储目录(创建 pending 子目录)
 */
int  tly_offline_init(const char *base_dir);

/**
 * 存储一条离线记录
 * @param data     数据单元内容(与0x02 realtime report data格式一致)
 * @param data_len 数据长度
 * @return 0成功
 */
int  tly_offline_save(const char *base_dir, int max_days,
                      const uint8_t *data, uint32_t data_len);

/**
 * 取最早的一条待发记录
 * @param record 输出记录信息
 * @param buf    输出数据内容缓冲区
 * @param buf_size 缓冲区大小
 * @return 0成功, -1无记录, 其他错误码
 */
int  tly_offline_pop_oldest(const char *base_dir, int max_days,
                            tly_offline_record_t *record,
                            uint8_t *buf, uint32_t buf_size);

/**
 * 删除已成功发送的记录
 */
int  tly_offline_delete(const char *base_dir, const char *filename);

/**
 * 统计待发记录
 */
int  tly_offline_get_stats(const char *base_dir, int max_days,
                           tly_offline_stats_t *stats);

/**
 * 清理过期记录(超过 max_days)
 * @return 清理的记录数
 */
int  tly_offline_cleanup(const char *base_dir, int max_days);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_OFFLINE_H__ */
