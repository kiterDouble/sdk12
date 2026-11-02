#ifndef _NET_DIAL_RECORD_DB_API_H_
#define _NET_DIAL_RECORD_DB_API_H_

#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief 拨号记录数据库上下文
 */
typedef struct
{
    void *dev;                  /**< sqlite3* 句柄 */
    pthread_mutex_t lock;       /**< 互斥锁，ctx 生命周期内持久存在 */
    uint8_t lockInited;         /**< lock 是否已完成初始化 */
    char dbPath[256];           /**< 数据库文件路径 */
} DialRecordDbCtx_t;

/**
 * @brief 初始化拨号记录数据库
 * @param ctx 数据库上下文
 * @param dbPath 数据库文件路径
 * @return 0 成功，-1 失败
 */
typedef int (*DialRecordDbInit_t)(DialRecordDbCtx_t *ctx, const char *dbPath);

/**
 * @brief 释放拨号记录数据库
 * @param ctx 数据库上下文
 * @return 0 成功，-1 失败
 */
typedef int (*DialRecordDbDeinit_t)(DialRecordDbCtx_t *ctx);

/**
 * @brief 插入一条拨号成功记录
 * @param ctx 数据库上下文
 * @param apnId APN ID
 * @return 0 成功，-1 失败
 * @details 时间戳使用当前系统时间（秒，UTC），唯一条目 ID 由数据库自增生成。
 */
typedef int (*DialRecordDbInsert_t)(DialRecordDbCtx_t *ctx, uint8_t apnId);

/**
 * @brief 清理指定 APN 的旧数据
 * @details 仅保留指定 APN 表中最近 recordMaxKeepNum 条记录（按自增 ID），
 *          清理后执行 VACUUM 释放空间。
 * @param ctx 数据库上下文
 * @param apnId APN ID
 * @param recordMaxKeepNum 每个 APN 表保留的最大记录数
 * @return 0 成功，-1 失败
 */
typedef int (*DialRecordDbTrim_t)(DialRecordDbCtx_t *ctx, uint8_t apnId, uint32_t recordMaxKeepNum);

/**
 * @brief 判断拨号记录数据库是否已初始化
 * @param ctx 数据库上下文
 * @return 1 已初始化，0 未初始化，-1 参数错误
 */
typedef int (*DialRecordDbIsInitialized_t)(DialRecordDbCtx_t *ctx);

/**
 * @brief 查询指定 APN 在最近一段时间内的拨号成功次数
 * @param ctx 数据库上下文
 * @param apnId APN ID
 * @param durationSeconds 最近时长（秒）
 * @return >=0 最近 durationSeconds 内的记录数，-1 失败
 */
typedef int32_t (*DialRecordDbQueryRecentCount_t)(DialRecordDbCtx_t *ctx, uint8_t apnId, uint32_t durationSeconds);

/**
 * @brief 查询指定 APN 的总拨号成功次数
 * @param ctx 数据库上下文
 * @param apnId APN ID
 * @return >=0 总记录数，-1 失败
 */
typedef int32_t (*DialRecordDbQueryCount_t)(DialRecordDbCtx_t *ctx, uint8_t apnId);

/**
 * @brief 拨号记录数据库 API 接口表
 */
typedef struct
{
    DialRecordDbInit_t              init;
    DialRecordDbDeinit_t            deinit;
    DialRecordDbIsInitialized_t     isInitialized;
    DialRecordDbInsert_t            insert;
    DialRecordDbTrim_t              trim;
    DialRecordDbQueryRecentCount_t  queryRecentCount;
    DialRecordDbQueryCount_t        queryCount;
} DialRecordDbApi_t;

extern const DialRecordDbApi_t g_dialRecordDbApi;

#ifdef __cplusplus
}
#endif

#endif /* _NET_DIAL_RECORD_DB_API_H_ */
