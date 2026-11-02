#ifndef _MODEM_SMS_RECORD_DB_API_H_
#define _MODEM_SMS_RECORD_DB_API_H_

#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief 短信记录类型
 */
typedef enum
{
    E_SMS_RECORD_TYPE_RECEIVED = 0,   /**< 接收短信 */
    E_SMS_RECORD_TYPE_SENT,           /**< 发送短信 */
    E_SMS_RECORD_TYPE_ALL             /**< 全部短信（仅用于查询） */
} SmsRecordType_e;

/**
 * @brief 短信记录数据库上下文
 */
typedef struct
{
    void *dev;                  /**< sqlite3* 句柄 */
    pthread_mutex_t lock;       /**< 互斥锁，ctx 生命周期内持久存在 */
    uint8_t lockInited;         /**< lock 是否已完成初始化 */
    char dbPath[256];           /**< 数据库文件路径 */
} SmsRecordDbCtx_t;

/**
 * @brief 初始化短信记录数据库
 * @param ctx 数据库上下文
 * @param dbPath 数据库文件路径
 * @return 0 成功，-1 失败
 */
typedef int (*SmsRecordDbInit_t)(SmsRecordDbCtx_t *ctx, const char *dbPath);

/**
 * @brief 释放短信记录数据库
 * @param ctx 数据库上下文
 * @return 0 成功，-1 失败
 */
typedef int (*SmsRecordDbDeinit_t)(SmsRecordDbCtx_t *ctx);

/**
 * @brief 判断短信记录数据库是否已初始化
 * @param ctx 数据库上下文
 * @return 1 已初始化，0 未初始化，-1 参数错误
 */
typedef int (*SmsRecordDbIsInitialized_t)(SmsRecordDbCtx_t *ctx);

/**
 * @brief 插入一条短信记录
 * @param ctx 数据库上下文
 * @param type 短信类型，E_SMS_RECORD_TYPE_RECEIVED 或 E_SMS_RECORD_TYPE_SENT
 * @param sourceAddr 短信来源地址（手机号）
 * @param msgContent 短信内容
 * @param msgLen 短信内容长度（字节）
 * @return 0 成功，-1 失败
 * @details 时间戳使用当前系统时间（秒，UTC），唯一条目 ID 由数据库自增生成。
 */
typedef int (*SmsRecordDbInsert_t)(SmsRecordDbCtx_t *ctx, SmsRecordType_e type,
                                   const uint8_t *sourceAddr,
                                   const uint8_t *msgContent, int32_t msgLen);

/**
 * @brief 清理旧短信数据
 * @details 仅保留表中最近 recordMaxKeepNum 条记录（按自增 ID），
 *          清理后执行 VACUUM 释放空间。
 * @param ctx 数据库上下文
 * @param recordMaxKeepNum 表中保留的最大记录数
 * @return 0 成功，-1 失败
 */
typedef int (*SmsRecordDbTrim_t)(SmsRecordDbCtx_t *ctx, uint32_t recordMaxKeepNum);

/**
 * @brief 查询短信记录数量
 * @param ctx 数据库上下文
 * @param type 短信类型，E_SMS_RECORD_TYPE_RECEIVED / E_SMS_RECORD_TYPE_SENT / E_SMS_RECORD_TYPE_ALL
 * @return >=0 指定类型的记录数，-1 失败
 */
typedef int32_t (*SmsRecordDbQueryCount_t)(SmsRecordDbCtx_t *ctx, SmsRecordType_e type);

/**
 * @brief 短信记录数据库 API 接口表
 */
typedef struct
{
    SmsRecordDbInit_t              init;
    SmsRecordDbDeinit_t            deinit;
    SmsRecordDbIsInitialized_t     isInitialized;
    SmsRecordDbInsert_t            insert;
    SmsRecordDbTrim_t              trim;
    SmsRecordDbQueryCount_t        queryCount;
} SmsRecordDbApi_t;

extern const SmsRecordDbApi_t g_smsRecordDbApi;

#ifdef __cplusplus
}
#endif

#endif /* _MODEM_SMS_RECORD_DB_API_H_ */
