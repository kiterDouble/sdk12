#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <libgen.h>
#include <sqlite3.h>
#include <pthread.h>

#include "modem_sms_record_db_api.h"
#include "sr_log.h"

/*
 * 测试指令
 * sqlite3 手工查询示例（DB 路径：/oemdata/parameters/modem_sms_record.db）

 * 查询所有短信记录：
sqlite3 /oemdata/parameters/modem_sms_record.db "SELECT id, datetime(timestamp, 'unixepoch', 'localtime') AS time, source_addr, msg_len, msg_content FROM sms_record ORDER BY id DESC;"

 * 查询短信记录总数：
sqlite3 /oemdata/parameters/modem_sms_record.db "SELECT COUNT(*) FROM sms_record"

 * 手动插入当前时间接收短信记录
TS=`date +%s`; sqlite3 /oemdata/parameters/modem_sms_record.db "INSERT INTO sms_record (timestamp, type, source_addr, msg_content, msg_len) VALUES ($TS, 0, '+86138xxxx', 'test', 4);"
 */

#define SMS_RECORD_DB_PATH_LEN      (256)
#define SMS_RECORD_TABLE_NAME_LEN   (64)
#define SMS_RECORD_SQL_LEN          (512)

#define SMS_RECORD_MAX_ADDRESS_LEN  (252)
#define SMS_RECORD_MAX_CONTENT_LEN  (1440)

/* 短信记录表名 */
#define SMS_RECORD_TABLE_NAME       "sms_record"

#define SQL_CREATE_TABLE \
    "CREATE TABLE IF NOT EXISTS " SMS_RECORD_TABLE_NAME " (" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT," \
    "timestamp INTEGER NOT NULL," \
    "type INTEGER NOT NULL," \
    "source_addr TEXT NOT NULL," \
    "msg_content BLOB NOT NULL," \
    "msg_len INTEGER NOT NULL);"

#define SQL_INSERT_FMT \
    "INSERT INTO " SMS_RECORD_TABLE_NAME " (timestamp, type, source_addr, msg_content, msg_len) VALUES (?, ?, ?, ?, ?);"

#define SQL_TRIM_FMT \
    "DELETE FROM " SMS_RECORD_TABLE_NAME " WHERE id NOT IN (" \
    "SELECT id FROM " SMS_RECORD_TABLE_NAME " ORDER BY id DESC LIMIT %d);"

#define SQL_COUNT_FMT \
    "SELECT COUNT(*) FROM " SMS_RECORD_TABLE_NAME " WHERE type = ?;"

#define SQL_COUNT_ALL_FMT \
    "SELECT COUNT(*) FROM " SMS_RECORD_TABLE_NAME ";"

/**
 * @brief 计算来源地址有效长度（不超过最大长度，遇到 '\0' 终止）
 */
static uint32_t GetSourceAddrLen(const uint8_t *sourceAddr)
{
    uint32_t i = 0;

    if (sourceAddr == NULL)
    {
        return 0;
    }

    for (i = 0; i < SMS_RECORD_MAX_ADDRESS_LEN; i++)
    {
        if (sourceAddr[i] == '\0')
        {
            break;
        }
    }

    return i;
}

/**
 * @brief 确保数据库文件所在目录存在
 */
static int EnsureDbDirectory(const char *dbPath)
{
    char pathCopy[SMS_RECORD_DB_PATH_LEN] = {0};
    struct stat st = {0};
    char *dir = NULL;

    if (dbPath == NULL || strlen(dbPath) == 0)
    {
        return -1;
    }

    strncpy(pathCopy, dbPath, sizeof(pathCopy) - 1);
    pathCopy[sizeof(pathCopy) - 1] = '\0';

    dir = dirname(pathCopy);
    if (dir == NULL || strlen(dir) == 0 || strcmp(dir, ".") == 0)
    {
        return 0;
    }

    if (stat(dir, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            return 0;
        }
        LOG_Error("Path exists but is not a directory: %s", dir);
        return -1;
    }

    if (mkdir(dir, 0755) != 0)
    {
        LOG_Error("mkdir failed: %s, errno=%d", dir, errno);
        return -1;
    }

    return 0;
}

/**
 * @brief 创建短信记录表（若不存在）
 */
static int CreateSmsRecordTable(sqlite3 *db)
{
    int ret = -1;
    int rc = 0;
    char *errMsg = NULL;

    do {
        if (db == NULL)
        {
            LOG_Error("Invalid db handle");
            break;
        }

        rc = sqlite3_exec(db, SQL_CREATE_TABLE, NULL, NULL, &errMsg);
        if (rc != SQLITE_OK)
        {
            LOG_Error("Create table %s failed: %s", SMS_RECORD_TABLE_NAME, errMsg);
            sqlite3_free(errMsg);
            break;
        }

        ret = 0;
    } while (0);

    return ret;
}

/**
 * @brief 一次性初始化 ctx->lock
 * @details 该锁在 ctx 生命周期内持久存在，deinit 不销毁，
 *          支持 init/deinit 反复调用。
 */
static void SmsRecordDbLockInitOnce(SmsRecordDbCtx_t *ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    if (!ctx->lockInited)
    {
        if (pthread_mutex_init(&ctx->lock, NULL) == 0)
        {
            ctx->lockInited = 1;
        }
        else
        {
            LOG_Error("pthread_mutex_init failed, errno: %d", errno);
        }
    }
}

static int Sqlite3SmsRecordDbInit(SmsRecordDbCtx_t *ctx, const char *dbPath)
{
    int ret = -1;
    int rc = 0;
    sqlite3 *db = NULL;

    do {
        if (ctx == NULL || dbPath == NULL || strlen(dbPath) == 0)
        {
            LOG_Error("Invalid input parameters");
            break;
        }

        SmsRecordDbLockInitOnce(ctx);
        if (!ctx->lockInited)
        {
            LOG_Error("ctx lock init failed");
            break;
        }

        if (pthread_mutex_lock(&ctx->lock) != 0)
        {
            LOG_Error("pthread_mutex_lock failed, errno: %d", errno);
            break;
        }

        do {
            /* 若已持有旧句柄，先关闭 */
            if (ctx->dev != NULL)
            {
                sqlite3_close((sqlite3 *)ctx->dev);
                ctx->dev = NULL;
            }

            if (EnsureDbDirectory(dbPath) != 0)
            {
                LOG_Error("EnsureDbDirectory failed: %s", dbPath);
                break;
            }

            rc = sqlite3_open(dbPath, &db);
            if (rc != SQLITE_OK)
            {
                LOG_Error("sqlite3_open failed: %s", sqlite3_errmsg(db));
                break;
            }

            memset(ctx->dbPath, 0, sizeof(ctx->dbPath));
            strncpy(ctx->dbPath, dbPath, sizeof(ctx->dbPath) - 1);
            ctx->dbPath[sizeof(ctx->dbPath) - 1] = '\0';
            ctx->dev = (void *)db;
            ret = 0;
        } while (0);

        pthread_mutex_unlock(&ctx->lock);
    } while (0);

    return ret;
}

static int Sqlite3SmsRecordDbDeinit(SmsRecordDbCtx_t *ctx)
{
    int ret = -1;

    if (ctx == NULL)
    {
        LOG_Error("Invalid input parameters");
        return -1;
    }

    if (!ctx->lockInited)
    {
        /* 锁未初始化说明 DB 也未打开，直接返回成功 */
        return 0;
    }

    if (pthread_mutex_lock(&ctx->lock) != 0)
    {
        LOG_Error("pthread_mutex_lock failed, errno: %d", errno);
        return -1;
    }

    do {
        if (ctx->dev != NULL)
        {
            sqlite3 *db = (sqlite3 *)ctx->dev;
            if (sqlite3_close(db) != SQLITE_OK)
            {
                LOG_Error("sqlite3_close failed: %s", sqlite3_errmsg(db));
                break;
            }
            ctx->dev = NULL;
        }

        memset(ctx->dbPath, 0, sizeof(ctx->dbPath));
        ret = 0;
    } while (0);

    pthread_mutex_unlock(&ctx->lock);
    return ret;
}

static int Sqlite3SmsRecordDbIsInitialized(SmsRecordDbCtx_t *ctx)
{
    int initialized = -1;

    if (ctx == NULL)
    {
        LOG_Error("Invalid input parameters");
        return -1;
    }

    if (!ctx->lockInited)
    {
        return 0;
    }

    if (pthread_mutex_lock(&ctx->lock) != 0)
    {
        LOG_Error("pthread_mutex_lock failed, errno: %d", errno);
        return -1;
    }

    initialized = (ctx->dev != NULL) ? 1 : 0;
    pthread_mutex_unlock(&ctx->lock);

    return initialized;
}

static int Sqlite3SmsRecordDbInsert(SmsRecordDbCtx_t *ctx, SmsRecordType_e type,
                                    const uint8_t *sourceAddr,
                                    const uint8_t *msgContent, int32_t msgLen)
{
    int ret = -1;
    int rc = 0;
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char sql[SMS_RECORD_SQL_LEN] = {0};
    time_t now = 0;
    uint32_t sourceAddrLen = 0;

    if (ctx == NULL || sourceAddr == NULL ||
        (msgLen > 0 && msgContent == NULL) ||
        msgLen < 0 || msgLen > SMS_RECORD_MAX_CONTENT_LEN ||
        (type != E_SMS_RECORD_TYPE_RECEIVED && type != E_SMS_RECORD_TYPE_SENT))
    {
        LOG_Error("Invalid input parameters");
        return -1;
    }

    if (pthread_mutex_lock(&ctx->lock) != 0)
    {
        LOG_Error("pthread_mutex_lock failed, errno: %d", errno);
        return -1;
    }

    do {
        if (ctx->dev == NULL)
        {
            LOG_Error("DB not initialized");
            break;
        }

        db = (sqlite3 *)ctx->dev;

        if (CreateSmsRecordTable(db) != 0)
        {
            break;
        }

        snprintf(sql, sizeof(sql), SQL_INSERT_FMT);

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            LOG_Error("sqlite3_prepare_v2 insert failed: %s", sqlite3_errmsg(db));
            break;
        }

        now = time(NULL);
        sourceAddrLen = GetSourceAddrLen(sourceAddr);

        sqlite3_bind_int64(stmt, 1, (sqlite3_int64)now);
        sqlite3_bind_int(stmt, 2, (int)type);
        sqlite3_bind_text(stmt, 3, (const char *)sourceAddr, (int)sourceAddrLen, SQLITE_STATIC);
        sqlite3_bind_blob(stmt, 4, msgContent, (int)msgLen, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, (int)msgLen);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            LOG_Error("sqlite3_step insert failed: %s", sqlite3_errmsg(db));
            break;
        }

        ret = 0;
    } while (0);

    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }
    pthread_mutex_unlock(&ctx->lock);

    return ret;
}

static int Sqlite3SmsRecordDbTrim(SmsRecordDbCtx_t *ctx, uint32_t recordMaxKeepNum)
{
    int ret = -1;
    int rc = 0;
    sqlite3 *db = NULL;
    char *errMsg = NULL;
    char sql[SMS_RECORD_SQL_LEN] = {0};

    if (ctx == NULL || recordMaxKeepNum == 0)
    {
        LOG_Error("Invalid input parameters");
        return -1;
    }

    if (pthread_mutex_lock(&ctx->lock) != 0)
    {
        LOG_Error("pthread_mutex_lock failed, errno: %d", errno);
        return -1;
    }

    do {
        if (ctx->dev == NULL)
        {
            LOG_Error("DB not initialized");
            break;
        }

        db = (sqlite3 *)ctx->dev;

        if (CreateSmsRecordTable(db) != 0)
        {
            break;
        }

        snprintf(sql, sizeof(sql), SQL_TRIM_FMT, (int)recordMaxKeepNum);
        rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
        if (rc != SQLITE_OK)
        {
            LOG_Error("Trim table %s failed: %s", SMS_RECORD_TABLE_NAME, errMsg);
            sqlite3_free(errMsg);
            break;
        }

        rc = sqlite3_exec(db, "VACUUM;", NULL, NULL, NULL);
        if (rc != SQLITE_OK)
        {
            LOG_Error("sqlite3_exec VACUUM failed: %s", sqlite3_errmsg(db));
            break;
        }

        ret = 0;
    } while (0);

    pthread_mutex_unlock(&ctx->lock);
    return ret;
}

static int32_t Sqlite3SmsRecordDbQueryCount(SmsRecordDbCtx_t *ctx, SmsRecordType_e type)
{
    int32_t ret = -1;
    int rc = 0;
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char sql[SMS_RECORD_SQL_LEN] = {0};
    int isAll = 0;

    if (ctx == NULL ||
        (type != E_SMS_RECORD_TYPE_RECEIVED && type != E_SMS_RECORD_TYPE_SENT && type != E_SMS_RECORD_TYPE_ALL))
    {
        LOG_Error("Invalid input parameters");
        return -1;
    }

    isAll = (type == E_SMS_RECORD_TYPE_ALL) ? 1 : 0;

    if (pthread_mutex_lock(&ctx->lock) != 0)
    {
        LOG_Error("pthread_mutex_lock failed, errno: %d", errno);
        return -1;
    }

    do {
        if (ctx->dev == NULL)
        {
            LOG_Error("DB not initialized");
            break;
        }

        db = (sqlite3 *)ctx->dev;

        if (CreateSmsRecordTable(db) != 0)
        {
            break;
        }

        snprintf(sql, sizeof(sql), isAll ? SQL_COUNT_ALL_FMT : SQL_COUNT_FMT);

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            LOG_Error("sqlite3_prepare_v2 query count failed: %s", sqlite3_errmsg(db));
            break;
        }

        if (!isAll)
        {
            sqlite3_bind_int(stmt, 1, (int)type);
        }

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW)
        {
            ret = sqlite3_column_int(stmt, 0);
        }
        else
        {
            LOG_Error("sqlite3_step query count failed: %s", sqlite3_errmsg(db));
            break;
        }
    } while (0);

    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }
    pthread_mutex_unlock(&ctx->lock);

    return ret;
}

const SmsRecordDbApi_t g_smsRecordDbApi =
{
    .init              = Sqlite3SmsRecordDbInit,
    .deinit            = Sqlite3SmsRecordDbDeinit,
    .isInitialized     = Sqlite3SmsRecordDbIsInitialized,
    .insert            = Sqlite3SmsRecordDbInsert,
    .trim              = Sqlite3SmsRecordDbTrim,
    .queryCount        = Sqlite3SmsRecordDbQueryCount,
};

