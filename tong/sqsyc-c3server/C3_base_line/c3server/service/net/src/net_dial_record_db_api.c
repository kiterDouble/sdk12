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

#include "net_dial_record_db_api.h"
#include "sr_log.h"

/*
 * 测试指令
 * sqlite3 手工查询示例（DB 路径：/oemdata/parameters/net_dial_record.db）

 * 查询 APN_ID 对应的所有拨号成功记录：
sqlite3 /oemdata/parameters/net_dial_record.db "SELECT id, datetime(timestamp, 'unixepoch', 'localtime') AS time, apn_id FROM dial_record_apn_1 ORDER BY id DESC;"
sqlite3 /oemdata/parameters/net_dial_record.db "SELECT id, datetime(timestamp, 'unixepoch', 'localtime') AS time, apn_id FROM dial_record_apn_2 ORDER BY id DESC;" 

 * 查询 APN_ID 最近一小时拨号成功次数：
sqlite3 /oemdata/parameters/net_dial_record.db "SELECT COUNT(*) FROM dial_record_apn_1 WHERE timestamp >= strftime('%s', 'now', '-1 hours')"
sqlite3 /oemdata/parameters/net_dial_record.db "SELECT COUNT(*) FROM dial_record_apn_2 WHERE timestamp >= strftime('%s', 'now', '-1 hours')"


 * 查询 APN_ID 拨号成功次数：
sqlite3 /oemdata/parameters/net_dial_record.db "SELECT COUNT(*) FROM dial_record_apn_1 "
sqlite3 /oemdata/parameters/net_dial_record.db "SELECT COUNT(*) FROM dial_record_apn_2 "

 * 手动插入当前时间拨号记录
TS=`date +%s`; sqlite3 /oemdata/parameters/net_dial_record.db "INSERT INTO dial_record_apn_1 (timestamp, apn_id) VALUES ($TS, 2);"
TS=`date +%s`; sqlite3 /oemdata/parameters/net_dial_record.db "INSERT INTO dial_record_apn_2 (timestamp, apn_id) VALUES ($TS, 2);"

 * 断开 APN1
    AT+CGACT=0,2
 * 断开 APN2
    AT+CGACT=0,3

 * 注：<apnId> 替换为实际 APN ID，表名格式为 dial_record_apn_<apnId>。
 */

#define DIAL_RECORD_DB_PATH_LEN     (256)
#define DIAL_RECORD_TABLE_NAME_LEN  (64)
#define DIAL_RECORD_SQL_LEN         (512)

/* 拨号记录表名前缀 */
#define DIAL_RECORD_TABLE_PREFIX        "dial_record_apn"

#define SQL_CREATE_TABLE_FMT \
    "CREATE TABLE IF NOT EXISTS %s (" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT," \
    "timestamp INTEGER NOT NULL," \
    "apn_id INTEGER NOT NULL);"

#define SQL_INSERT_FMT \
    "INSERT INTO %s (timestamp, apn_id) VALUES (?, ?);"

#define SQL_TRIM_FMT \
    "DELETE FROM %s WHERE id NOT IN (" \
    "SELECT id FROM %s ORDER BY id DESC LIMIT %d);"

#define SQL_QUERY_RECENT_COUNT_FMT \
    "SELECT COUNT(*) FROM %s WHERE timestamp >= ?;"

#define SQL_COUNT_FMT \
    "SELECT COUNT(*) FROM %s;"

/**
 * @brief 根据 APN ID 生成表名
 */
static void GetDialRecordTableName(uint8_t apnId, char *nameBuf, size_t bufSize)
{
    snprintf(nameBuf, bufSize, "%s_%u", DIAL_RECORD_TABLE_PREFIX, (unsigned)apnId);
}

/**
 * @brief 确保数据库文件所在目录存在
 */
static int EnsureDbDirectory(const char *dbPath)
{
    char pathCopy[DIAL_RECORD_DB_PATH_LEN] = {0};
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
 * @brief 为指定 APN 创建记录表（若不存在）
 */
static int CreateApnRecordTable(sqlite3 *db, uint8_t apnId)
{
    int ret = -1;
    int rc = 0;
    char *errMsg = NULL;
    char tableName[DIAL_RECORD_TABLE_NAME_LEN] = {0};
    char sql[DIAL_RECORD_SQL_LEN] = {0};

    do {
        if (db == NULL)
        {
            LOG_Error("Invalid db handle");
            break;
        }

        GetDialRecordTableName(apnId, tableName, sizeof(tableName));
        snprintf(sql, sizeof(sql), SQL_CREATE_TABLE_FMT, tableName);

        rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
        if (rc != SQLITE_OK)
        {
            LOG_Error("Create table %s failed: %s", tableName, errMsg);
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
static void DialRecordDbLockInitOnce(DialRecordDbCtx_t *ctx)
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

static int Sqlite3DialRecordDbInit(DialRecordDbCtx_t *ctx, const char *dbPath)
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

        DialRecordDbLockInitOnce(ctx);
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

static int Sqlite3DialRecordDbDeinit(DialRecordDbCtx_t *ctx)
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

static int Sqlite3DialRecordDbIsInitialized(DialRecordDbCtx_t *ctx)
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

static int Sqlite3DialRecordDbInsert(DialRecordDbCtx_t *ctx, uint8_t apnId)
{
    int ret = -1;
    int rc = 0;
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char tableName[DIAL_RECORD_TABLE_NAME_LEN] = {0};
    char sql[DIAL_RECORD_SQL_LEN] = {0};
    time_t now = 0;

    if (ctx == NULL)
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

        if (CreateApnRecordTable(db, apnId) != 0)
        {
            break;
        }

        GetDialRecordTableName(apnId, tableName, sizeof(tableName));
        snprintf(sql, sizeof(sql), SQL_INSERT_FMT, tableName);

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            LOG_Error("sqlite3_prepare_v2 insert failed: %s", sqlite3_errmsg(db));
            break;
        }

        now = time(NULL);
        sqlite3_bind_int64(stmt, 1, (sqlite3_int64)now);
        sqlite3_bind_int(stmt, 2, (int)apnId);

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

/**
 * @brief 对单个 APN 表执行保留最近 N 条记录的清理
 */
static int TrimSingleApnTable(sqlite3 *db, const char *tableName, uint32_t recordMaxKeepNum)
{
    int ret = -1;
    int rc = 0;
    char *errMsg = NULL;
    char sql[DIAL_RECORD_SQL_LEN] = {0};

    do {
        if (db == NULL || tableName == NULL || strlen(tableName) == 0)
        {
            LOG_Error("Invalid input parameters");
            break;
        }

        snprintf(sql, sizeof(sql), SQL_TRIM_FMT, tableName, tableName, (int)recordMaxKeepNum);
        rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
        if (rc != SQLITE_OK)
        {
            LOG_Error("Trim table %s failed: %s", tableName, errMsg);
            sqlite3_free(errMsg);
            break;
        }

        ret = 0;
    } while (0);

    return ret;
}

static int Sqlite3DialRecordDbTrim(DialRecordDbCtx_t *ctx, uint8_t apnId, uint32_t recordMaxKeepNum)
{
    int ret = -1;
    int rc = 0;
    sqlite3 *db = NULL;
    char tableName[DIAL_RECORD_TABLE_NAME_LEN] = {0};

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

        if (CreateApnRecordTable(db, apnId) != 0)
        {
            break;
        }

        GetDialRecordTableName(apnId, tableName, sizeof(tableName));
        if (TrimSingleApnTable(db, tableName, recordMaxKeepNum) != 0)
        {
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

static int32_t Sqlite3DialRecordDbQueryRecentCount(DialRecordDbCtx_t *ctx, uint8_t apnId,
                                                    uint32_t durationSeconds)
{
    int32_t ret = -1;
    int rc = 0;
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char tableName[DIAL_RECORD_TABLE_NAME_LEN] = {0};
    char sql[DIAL_RECORD_SQL_LEN] = {0};
    time_t now = 0;
    sqlite3_int64 cutoff = 0;

    if (ctx == NULL || durationSeconds == 0)
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

        if (CreateApnRecordTable(db, apnId) != 0)
        {
            break;
        }

        GetDialRecordTableName(apnId, tableName, sizeof(tableName));
        snprintf(sql, sizeof(sql), SQL_QUERY_RECENT_COUNT_FMT, tableName);

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            LOG_Error("sqlite3_prepare_v2 query count failed: %s", sqlite3_errmsg(db));
            break;
        }

        now = time(NULL);
        cutoff = (sqlite3_int64)(now - (time_t)durationSeconds);
        if (cutoff < 0)
        {
            cutoff = 0;
        }

        sqlite3_bind_int64(stmt, 1, cutoff);

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

static int32_t Sqlite3DialRecordDbQueryCount(DialRecordDbCtx_t *ctx, uint8_t apnId)
{
    int32_t ret = -1;
    int rc = 0;
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char tableName[DIAL_RECORD_TABLE_NAME_LEN] = {0};
    char sql[DIAL_RECORD_SQL_LEN] = {0};

    if (ctx == NULL)
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

        if (CreateApnRecordTable(db, apnId) != 0)
        {
            break;
        }

        GetDialRecordTableName(apnId, tableName, sizeof(tableName));
        snprintf(sql, sizeof(sql), SQL_COUNT_FMT, tableName);

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            LOG_Error("sqlite3_prepare_v2 query count failed: %s", sqlite3_errmsg(db));
            break;
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

const DialRecordDbApi_t g_dialRecordDbApi =
{
    .init              = Sqlite3DialRecordDbInit,
    .deinit            = Sqlite3DialRecordDbDeinit,
    .isInitialized     = Sqlite3DialRecordDbIsInitialized,
    .insert            = Sqlite3DialRecordDbInsert,
    .trim              = Sqlite3DialRecordDbTrim,
    .queryRecentCount  = Sqlite3DialRecordDbQueryRecentCount,
    .queryCount        = Sqlite3DialRecordDbQueryCount,
};
