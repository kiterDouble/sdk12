/**
 * tsp_offline_db.c - SQLite离线数据库存储与补发实现
 *
 * 使用SQLite数据库存储离线数据，支持两个数据库：
 * 1. 触发报警数据库 (alarm) - 高优先级，优先补发
 * 2. 实时数据库 (realtime) - 普通优先级
 *
 * SQLite 命令行查看数据：
 *   sqlite3 offline_alarm.db
 *   sqlite3 offline_realtime.db
 *
 * 常用查询命令：
 *   -- 查看表结构
 *   PRAGMA table_info(offline_records);
 *
 *   -- 查看所有记录（带十六进制数据）
 *   SELECT id, timestamp, data_type, data_len, hex(data) FROM offline_records;
 *
 *   -- 以可读格式查看
 *   .mode line
 *   SELECT id, timestamp, data_type, data_len, hex(data) FROM offline_records;
 *
 *   -- 查看特定 ID 的记录
 *   SELECT id, timestamp, data_type, data_len, hex(data) FROM offline_records WHERE id = 1;
 *
 *   -- 查看数据类型统计
 *   SELECT data_type, COUNT(*) as count, SUM(data_len) as total_bytes FROM offline_records GROUP BY data_type;
 *
 *   -- 查看时间范围
 *   SELECT MIN(timestamp) as oldest, MAX(timestamp) as newest, COUNT(*) as total FROM offline_records;
 */

#include "tsp_offline_db.h"
#include "tsp_log.h"
#include "tsp_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <sqlite3.h>
#include <pthread.h>

/* 数据库文件名 */
#define TLY_ALARM_DB_NAME    "offline_alarm.db"
#define TLY_REALTIME_DB_NAME "offline_realtime.db"

/* 数据库表名 */
#define TLY_TABLE_NAME       "offline_records"

/* SQL语句 */
#define SQL_CREATE_TABLE \
    "CREATE TABLE IF NOT EXISTS " TLY_TABLE_NAME " (" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT," \
    "timestamp INTEGER NOT NULL," \
    "data_type INTEGER NOT NULL," \
    "data_len INTEGER NOT NULL," \
    "data BLOB NOT NULL," \
    "created_at INTEGER DEFAULT (strftime('%s','now'))" \
    ");"

#define SQL_CREATE_INDEX_TS \
    "CREATE INDEX IF NOT EXISTS idx_timestamp ON " TLY_TABLE_NAME " (timestamp);"

#define SQL_INSERT \
    "INSERT INTO " TLY_TABLE_NAME " (timestamp, data_type, data_len, data) " \
    "VALUES (?, ?, ?, ?);"

#define SQL_SELECT_OLDEST \
    "SELECT id, timestamp, data_type, data_len, data FROM " TLY_TABLE_NAME " " \
    "ORDER BY timestamp ASC, id ASC LIMIT 1;"

#define SQL_DELETE_BY_ID \
    "DELETE FROM " TLY_TABLE_NAME " WHERE id = ?;"

#define SQL_SELECT_COUNT \
    "SELECT COUNT(*) FROM " TLY_TABLE_NAME ";"

#define SQL_SELECT_STATS \
    "SELECT COUNT(*), MIN(timestamp), MAX(timestamp) FROM " TLY_TABLE_NAME ";"

#define SQL_DELETE_EXPIRED \
    "DELETE FROM " TLY_TABLE_NAME " WHERE timestamp < ?;"

#define SQL_VACUUM \
    "VACUUM;"

/* 数据库上下文 */
typedef struct {
    sqlite3     *db;                /* SQLite句柄 */
    char         path[512];         /* 数据库文件路径 */
    sqlite3_stmt *stmt_insert;      /* 预编译插入语句 */
    sqlite3_stmt *stmt_select;      /* 预编译查询语句 */
    sqlite3_stmt *stmt_delete;      /* 预编译删除语句 */
    sqlite3_stmt *stmt_count;       /* 预编译计数语句 */
    sqlite3_stmt *stmt_stats;       /* 预编译统计语句 */
    sqlite3_stmt *stmt_cleanup;     /* 预编译清理语句 */
} tly_db_context_t;

/* 全局变量 */
static struct {
    tly_db_context_t  dbs[TLY_DB_TYPE_COUNT];  /* 两个数据库上下文 */
    pthread_mutex_t   mutex;                    /* 全局锁 */
    char              base_dir[512];            /* 基础目录 */
    int               max_days;                 /* 最大存储天数 */
    int               initialized;              /* 初始化标志 */
} g_offline_db = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .initialized = 0
};

/* 内部函数声明 */
static int ensure_dir(const char *path);
static int db_open(tly_db_type_t db_type);
static void db_close(tly_db_type_t db_type);
static int db_init_schema(tly_db_type_t db_type);
static int db_prepare_statements(tly_db_type_t db_type);
static void db_finalize_statements(tly_db_type_t db_type);
static double db_get_size_mb(tly_db_type_t db_type);
static int db_delete_oldest_half(tly_db_type_t db_type);
static void db_check_size_limit(tly_db_type_t db_type);

/**
 * 确保目录存在
 */
static int ensure_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return 0;
        return -1;
    }
    if (mkdir(path, 0755) != 0) {
        TLY_LOG_E("Failed to create dir %s: %s", path, strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * 打开数据库
 */
static int db_open(tly_db_type_t db_type)
{
    tly_db_context_t *ctx = &g_offline_db.dbs[db_type];
    
    if (ctx->db != NULL) {
        return 0;  /* 已打开 */
    }
    
    int rc = sqlite3_open_v2(ctx->path, &ctx->db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                             NULL);
    if (rc != SQLITE_OK || ctx->db == NULL) {
        TLY_LOG_E("Failed to open %s db: %s",
                  db_type == TLY_DB_TYPE_ALARM ? "alarm" : "realtime",
                  sqlite3_errstr(rc));
        ctx->db = NULL;
        return -1;
    }
    
    /* 设置busy timeout */
    sqlite3_busy_timeout(ctx->db, 300);
    
    /* 启用WAL模式 */
    sqlite3_exec(ctx->db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    sqlite3_exec(ctx->db, "PRAGMA synchronous=NORMAL;", NULL, NULL, NULL);
    
    TLY_LOG_D("Opened %s db: %s",
              db_type == TLY_DB_TYPE_ALARM ? "alarm" : "realtime",
              ctx->path);
    return 0;
}

/**
 * 关闭数据库
 */
static void db_close(tly_db_type_t db_type)
{
    tly_db_context_t *ctx = &g_offline_db.dbs[db_type];
    
    db_finalize_statements(db_type);
    
    if (ctx->db) {
        sqlite3_close_v2(ctx->db);
        ctx->db = NULL;
        TLY_LOG_D("Closed %s db",
                  db_type == TLY_DB_TYPE_ALARM ? "alarm" : "realtime");
    }
}

/**
 * 初始化数据库表结构
 */
static int db_init_schema(tly_db_type_t db_type)
{
    tly_db_context_t *ctx = &g_offline_db.dbs[db_type];
    
    if (!ctx->db) return -1;
    
    char *errmsg = NULL;
    
    /* 创建表 */
    int rc = sqlite3_exec(ctx->db, SQL_CREATE_TABLE, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        TLY_LOG_E("Failed to create table: %s", errmsg ? errmsg : sqlite3_errstr(rc));
        if (errmsg) sqlite3_free(errmsg);
        return -1;
    }
    
    /* 创建索引 */
    rc = sqlite3_exec(ctx->db, SQL_CREATE_INDEX_TS, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        TLY_LOG_E("Failed to create index: %s", errmsg ? errmsg : sqlite3_errstr(rc));
        if (errmsg) sqlite3_free(errmsg);
        return -1;
    }
    
    return 0;
}

/**
 * 预编译SQL语句
 */
static int db_prepare_statements(tly_db_type_t db_type)
{
    tly_db_context_t *ctx = &g_offline_db.dbs[db_type];
    
    if (!ctx->db) return -1;
    
    /* 插入语句 */
    if (sqlite3_prepare_v2(ctx->db, SQL_INSERT, -1, &ctx->stmt_insert, NULL) != SQLITE_OK) {
        TLY_LOG_E("Failed to prepare insert stmt: %s", sqlite3_errmsg(ctx->db));
        return -1;
    }
    
    /* 查询语句 */
    if (sqlite3_prepare_v2(ctx->db, SQL_SELECT_OLDEST, -1, &ctx->stmt_select, NULL) != SQLITE_OK) {
        TLY_LOG_E("Failed to prepare select stmt: %s", sqlite3_errmsg(ctx->db));
        return -1;
    }
    
    /* 删除语句 */
    if (sqlite3_prepare_v2(ctx->db, SQL_DELETE_BY_ID, -1, &ctx->stmt_delete, NULL) != SQLITE_OK) {
        TLY_LOG_E("Failed to prepare delete stmt: %s", sqlite3_errmsg(ctx->db));
        return -1;
    }
    
    /* 计数语句 */
    if (sqlite3_prepare_v2(ctx->db, SQL_SELECT_COUNT, -1, &ctx->stmt_count, NULL) != SQLITE_OK) {
        TLY_LOG_E("Failed to prepare count stmt: %s", sqlite3_errmsg(ctx->db));
        return -1;
    }
    
    /* 统计语句 */
    if (sqlite3_prepare_v2(ctx->db, SQL_SELECT_STATS, -1, &ctx->stmt_stats, NULL) != SQLITE_OK) {
        TLY_LOG_E("Failed to prepare stats stmt: %s", sqlite3_errmsg(ctx->db));
        return -1;
    }
    
    /* 清理语句 */
    if (sqlite3_prepare_v2(ctx->db, SQL_DELETE_EXPIRED, -1, &ctx->stmt_cleanup, NULL) != SQLITE_OK) {
        TLY_LOG_E("Failed to prepare cleanup stmt: %s", sqlite3_errmsg(ctx->db));
        return -1;
    }
    
    return 0;
}

/**
 * 释放预编译语句
 */
static void db_finalize_statements(tly_db_type_t db_type)
{
    tly_db_context_t *ctx = &g_offline_db.dbs[db_type];
    
    if (ctx->stmt_insert) {
        sqlite3_finalize(ctx->stmt_insert);
        ctx->stmt_insert = NULL;
    }
    if (ctx->stmt_select) {
        sqlite3_finalize(ctx->stmt_select);
        ctx->stmt_select = NULL;
    }
    if (ctx->stmt_delete) {
        sqlite3_finalize(ctx->stmt_delete);
        ctx->stmt_delete = NULL;
    }
    if (ctx->stmt_count) {
        sqlite3_finalize(ctx->stmt_count);
        ctx->stmt_count = NULL;
    }
    if (ctx->stmt_stats) {
        sqlite3_finalize(ctx->stmt_stats);
        ctx->stmt_stats = NULL;
    }
    if (ctx->stmt_cleanup) {
        sqlite3_finalize(ctx->stmt_cleanup);
        ctx->stmt_cleanup = NULL;
    }
}

/* ======================== 公共接口 ======================== */

/**
 * @brief 初始化离线数据库
 * @param base_dir 数据库存储目录路径
 * @param max_days 数据最大保存天数，超过此天数的数据将被清理
 * @return 0成功，-1失败
 * @note 会创建两个数据库文件：offline_alarm.db(触发报警)和offline_realtime.db(实时数据)
 */
int tly_offline_db_init(const char *base_dir, int max_days)
{
    if (!base_dir || max_days <= 0) {
        TLY_LOG_E("Invalid params: base_dir=%p, max_days=%d", base_dir, max_days);
        return -1;
    }
    
    pthread_mutex_lock(&g_offline_db.mutex);
    
    if (g_offline_db.initialized) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        TLY_LOG_W("Offline db already initialized");
        return 0;
    }
    
    /* 确保目录存在 */
    if (ensure_dir(base_dir) != 0) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        return -1;
    }
    
    /* 保存配置 */
    strncpy(g_offline_db.base_dir, base_dir, sizeof(g_offline_db.base_dir) - 1);
    g_offline_db.max_days = max_days;
    
    /* 设置数据库路径 */
    snprintf(g_offline_db.dbs[TLY_DB_TYPE_ALARM].path,
             sizeof(g_offline_db.dbs[TLY_DB_TYPE_ALARM].path),
             "%s/%s", base_dir, TLY_ALARM_DB_NAME);
    
    snprintf(g_offline_db.dbs[TLY_DB_TYPE_REALTIME].path,
             sizeof(g_offline_db.dbs[TLY_DB_TYPE_REALTIME].path),
             "%s/%s", base_dir, TLY_REALTIME_DB_NAME);
    
    /* 打开并初始化两个数据库 */
    for (int i = 0; i < TLY_DB_TYPE_COUNT; i++) {
        if (db_open((tly_db_type_t)i) != 0) {
            /* 关闭已打开的数据库 */
            for (int j = 0; j < i; j++) {
                db_close((tly_db_type_t)j);
            }
            pthread_mutex_unlock(&g_offline_db.mutex);
            return -1;
        }
        
        if (db_init_schema((tly_db_type_t)i) != 0) {
            for (int j = 0; j <= i; j++) {
                db_close((tly_db_type_t)j);
            }
            pthread_mutex_unlock(&g_offline_db.mutex);
            return -1;
        }
        
        if (db_prepare_statements((tly_db_type_t)i) != 0) {
            for (int j = 0; j <= i; j++) {
                db_close((tly_db_type_t)j);
            }
            pthread_mutex_unlock(&g_offline_db.mutex);
            return -1;
        }
    }
    
    g_offline_db.initialized = 1;
    pthread_mutex_unlock(&g_offline_db.mutex);
    
    TLY_LOG_I("Offline db initialized: base=%s, max_days=%d", base_dir, max_days);
    return 0;
}

/**
 * @brief 关闭离线数据库，释放所有资源
 * @note 关闭所有数据库连接，释放预编译语句和互斥锁
 */
void tly_offline_db_deinit(void)
{
    pthread_mutex_lock(&g_offline_db.mutex);
    
    if (!g_offline_db.initialized) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        return;
    }
    
    for (int i = 0; i < TLY_DB_TYPE_COUNT; i++) {
        db_close((tly_db_type_t)i);
    }
    
    g_offline_db.initialized = 0;
    pthread_mutex_unlock(&g_offline_db.mutex);
    
    TLY_LOG_I("Offline db deinitialized");
}

/**
 * @brief 存储一条离线记录到指定数据库
 * @param db_type 数据库类型：TLY_DB_TYPE_ALARM(触发报警)或TLY_DB_TYPE_REALTIME(实时)
 * @param data 数据内容指针
 * @param data_len 数据长度
 * @param data_type 数据类型，如TLY_INFO_ALARM、TLY_INFO_FAST等
 * @return 0成功，-1失败
 * @note 线程安全，内部使用互斥锁保护
 */
int tly_offline_db_save(tly_db_type_t db_type,
                        const uint8_t *data, uint32_t data_len,
                        uint32_t data_type)
{
    if (db_type < 0 || db_type >= TLY_DB_TYPE_COUNT) {
        TLY_LOG_E("Invalid db_type: %d", db_type);
        return -1;
    }
    if (!data || data_len == 0) {
        TLY_LOG_E("Invalid data params");
        return -1;
    }
    
    pthread_mutex_lock(&g_offline_db.mutex);
    
    if (!g_offline_db.initialized) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        TLY_LOG_E("Offline db not initialized");
        return -1;
    }
    
    tly_db_context_t *ctx = &g_offline_db.dbs[db_type];
    
    if (!ctx->db || !ctx->stmt_insert) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        TLY_LOG_E("Db not ready: type=%d", db_type);
        return -1;
    }
    
    /* 重置语句 */
    sqlite3_reset(ctx->stmt_insert);
    sqlite3_clear_bindings(ctx->stmt_insert);
    
    /* 绑定参数 */
    time_t now = time(NULL);
    sqlite3_bind_int64(ctx->stmt_insert, 1, (sqlite3_int64)now);
    sqlite3_bind_int(ctx->stmt_insert, 2, (int)data_type);
    sqlite3_bind_int(ctx->stmt_insert, 3, (int)data_len);
    sqlite3_bind_blob(ctx->stmt_insert, 4, data, (int)data_len, SQLITE_STATIC);
    
    /* 执行 */
    int rc = sqlite3_step(ctx->stmt_insert);
    if (rc != SQLITE_DONE) {
        TLY_LOG_E("Failed to insert record: %s", sqlite3_errstr(rc));
        pthread_mutex_unlock(&g_offline_db.mutex);
        return -1;
    }
    
    pthread_mutex_unlock(&g_offline_db.mutex);
    
    TLY_LOG_D("Saved offline record: type=%s, data_type=0x%02X, len=%u",
              db_type == TLY_DB_TYPE_ALARM ? "alarm" : "realtime",
              data_type, data_len);
    return 0;
}

/**
 * @brief 取出最早的一条待发记录（优先从触发报警数据库取）
 * @param record 输出参数，记录信息（包含id、timestamp、data_type、data_len）
 * @param buf 数据缓冲区，用于存储记录的数据内容
 * @param buf_size 缓冲区大小
 * @param out_db_type 输出参数，实际取出的数据库类型
 * @return 0成功，-1无记录或其他错误
 * @note 优先从alarm数据库取，无数据时再从realtime数据库取
 */
int tly_offline_db_pop_oldest(tly_offline_db_record_t *record,
                              uint8_t *buf, uint32_t buf_size,
                              tly_db_type_t *out_db_type)
{
    if (!record || !buf || buf_size == 0 || !out_db_type) {
        TLY_LOG_E("Invalid params");
        return -1;
    }
    
    pthread_mutex_lock(&g_offline_db.mutex);
    
    if (!g_offline_db.initialized) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        TLY_LOG_E("Offline db not initialized");
        return -1;
    }
    
    /* 优先从触发报警数据库取 */
    tly_db_type_t db_order[] = {TLY_DB_TYPE_ALARM, TLY_DB_TYPE_REALTIME};
    
    for (int i = 0; i < 2; i++) {
        tly_db_type_t db_type = db_order[i];
        tly_db_context_t *ctx = &g_offline_db.dbs[db_type];
        
        if (!ctx->db || !ctx->stmt_select) {
            continue;
        }
        
        /* 重置语句 */
        sqlite3_reset(ctx->stmt_select);
        
        /* 执行查询 */
        int rc = sqlite3_step(ctx->stmt_select);
        if (rc == SQLITE_ROW) {
            /* 获取记录 */
            record->id = sqlite3_column_int64(ctx->stmt_select, 0);
            record->timestamp = (time_t)sqlite3_column_int64(ctx->stmt_select, 1);
            record->data_type = (uint32_t)sqlite3_column_int(ctx->stmt_select, 2);
            record->data_len = (uint32_t)sqlite3_column_int(ctx->stmt_select, 3);
            
            /* 获取数据 */
            const void *blob = sqlite3_column_blob(ctx->stmt_select, 4);
            int blob_len = sqlite3_column_bytes(ctx->stmt_select, 4);
            
            if (blob && blob_len > 0) {
                uint32_t copy_len = (record->data_len < buf_size) ? record->data_len : buf_size;
                memcpy(buf, blob, copy_len);
                
                *out_db_type = db_type;
                
                pthread_mutex_unlock(&g_offline_db.mutex);
                
                TLY_LOG_D("Popped offline record: type=%s, id=%lld, ts=%ld, len=%u",
                          db_type == TLY_DB_TYPE_ALARM ? "alarm" : "realtime",
                          (long long)record->id, (long)record->timestamp, record->data_len);
                return 0;
            }
        }
    }
    
    pthread_mutex_unlock(&g_offline_db.mutex);
    return -1;  /* 无记录 */
}

/**
 * @brief 删除已成功发送的记录
 * @param db_type 数据库类型
 * @param id 记录ID（由pop_oldest返回）
 * @return 0成功，-1失败
 * @note 发送成功后应调用此函数删除记录
 */
int tly_offline_db_delete(tly_db_type_t db_type, int64_t id)
{
    if (db_type < 0 || db_type >= TLY_DB_TYPE_COUNT) {
        TLY_LOG_E("Invalid db_type: %d", db_type);
        return -1;
    }
    
    pthread_mutex_lock(&g_offline_db.mutex);
    
    if (!g_offline_db.initialized) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        TLY_LOG_E("Offline db not initialized");
        return -1;
    }
    
    tly_db_context_t *ctx = &g_offline_db.dbs[db_type];
    
    if (!ctx->db || !ctx->stmt_delete) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        return -1;
    }
    
    /* 重置语句 */
    sqlite3_reset(ctx->stmt_delete);
    sqlite3_clear_bindings(ctx->stmt_delete);
    
    /* 绑定参数 */
    sqlite3_bind_int64(ctx->stmt_delete, 1, (sqlite3_int64)id);
    
    /* 执行 */
    int rc = sqlite3_step(ctx->stmt_delete);
    if (rc != SQLITE_DONE) {
        TLY_LOG_E("Failed to delete record: %s", sqlite3_errstr(rc));
        pthread_mutex_unlock(&g_offline_db.mutex);
        return -1;
    }
    
    pthread_mutex_unlock(&g_offline_db.mutex);
    
    TLY_LOG_D("Deleted offline record: type=%s, id=%lld",
              db_type == TLY_DB_TYPE_ALARM ? "alarm" : "realtime",
              (long long)id);
    return 0;
}

/**
 * @brief 获取离线数据统计信息
 * @param stats 输出参数，包含两个数据库的记录数、最早/最新时间
 * @return 0成功，-1失败
 */
int tly_offline_db_get_stats(tly_offline_db_stats_t *stats)
{
    if (!stats) {
        return -1;
    }
    
    memset(stats, 0, sizeof(*stats));
    
    pthread_mutex_lock(&g_offline_db.mutex);
    
    if (!g_offline_db.initialized) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        return -1;
    }
    
    for (int i = 0; i < TLY_DB_TYPE_COUNT; i++) {
        tly_db_context_t *ctx = &g_offline_db.dbs[i];
        
        if (!ctx->db || !ctx->stmt_stats) {
            continue;
        }
        
        /* 重置语句 */
        sqlite3_reset(ctx->stmt_stats);
        
        /* 执行查询 */
        if (sqlite3_step(ctx->stmt_stats) == SQLITE_ROW) {
            int count = sqlite3_column_int(ctx->stmt_stats, 0);
            sqlite3_int64 oldest = sqlite3_column_int64(ctx->stmt_stats, 1);
            sqlite3_int64 newest = sqlite3_column_int64(ctx->stmt_stats, 2);
            
            if (i == TLY_DB_TYPE_ALARM) {
                stats->alarm_count = (uint32_t)count;
                stats->alarm_oldest = (time_t)oldest;
                stats->alarm_newest = (time_t)newest;
            } else {
                stats->realtime_count = (uint32_t)count;
                stats->realtime_oldest = (time_t)oldest;
                stats->realtime_newest = (time_t)newest;
            }
        }
    }
    
    pthread_mutex_unlock(&g_offline_db.mutex);
    return 0;
}

/**
 * @brief 清理过期记录
 * @return 清理的记录总数
 * @note 删除超过max_days天的记录，并执行VACUUM释放空间
 */
int tly_offline_db_cleanup(void)
{
    pthread_mutex_lock(&g_offline_db.mutex);
    
    if (!g_offline_db.initialized) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        return 0;
    }
    
    time_t now = time(NULL);
    time_t cutoff = now - (time_t)g_offline_db.max_days * 86400;
    int total_deleted = 0;
    
    for (int i = 0; i < TLY_DB_TYPE_COUNT; i++) {
        tly_db_context_t *ctx = &g_offline_db.dbs[i];
        
        if (!ctx->db || !ctx->stmt_cleanup) {
            continue;
        }
        
        /* 重置语句 */
        sqlite3_reset(ctx->stmt_cleanup);
        sqlite3_clear_bindings(ctx->stmt_cleanup);
        
        /* 绑定参数 */
        sqlite3_bind_int64(ctx->stmt_cleanup, 1, (sqlite3_int64)cutoff);
        
        /* 执行 */
        int rc = sqlite3_step(ctx->stmt_cleanup);
        if (rc == SQLITE_DONE) {
            int deleted = sqlite3_changes(ctx->db);
            total_deleted += deleted;
            
            if (deleted > 0) {
                TLY_LOG_D("Cleaned up %d expired records from %s db",
                          deleted, i == TLY_DB_TYPE_ALARM ? "alarm" : "realtime");
            }
        }
        
        /* 执行VACUUM释放空间 */
        sqlite3_exec(ctx->db, SQL_VACUUM, NULL, NULL, NULL);
        
        /* 执行WAL checkpoint，截断WAL文件防止累积 */
        sqlite3_exec(ctx->db, "PRAGMA wal_checkpoint(TRUNCATE);", NULL, NULL, NULL);
        
        /* 检查数据库大小限制 */
        db_check_size_limit((tly_db_type_t)i);
    }
    
    pthread_mutex_unlock(&g_offline_db.mutex);
    
    if (total_deleted > 0) {
        TLY_LOG_I("Offline cleanup: deleted %d expired records (> %d days)",
                  total_deleted, g_offline_db.max_days);
    }
    
    return total_deleted;
}

/**
 * @brief 获取指定数据库的待发送记录数
 * @param db_type 数据库类型
 * @return 记录数，-1表示错误
 */
int tly_offline_db_get_count(tly_db_type_t db_type)
{
    if (db_type < 0 || db_type >= TLY_DB_TYPE_COUNT) {
        return -1;
    }
    
    pthread_mutex_lock(&g_offline_db.mutex);
    
    if (!g_offline_db.initialized) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        return -1;
    }
    
    tly_db_context_t *ctx = &g_offline_db.dbs[db_type];
    
    if (!ctx->db || !ctx->stmt_count) {
        pthread_mutex_unlock(&g_offline_db.mutex);
        return -1;
    }
    
    /* 重置语句 */
    sqlite3_reset(ctx->stmt_count);
    
    /* 执行查询 */
    int count = 0;
    if (sqlite3_step(ctx->stmt_count) == SQLITE_ROW) {
        count = sqlite3_column_int(ctx->stmt_count, 0);
    }
    
    pthread_mutex_unlock(&g_offline_db.mutex);
    
    return count;
}

/**
 * @brief 获取数据库文件大小 (MB)
 */
static double db_get_size_mb(tly_db_type_t db_type)
{
    tly_db_context_t *ctx = &g_offline_db.dbs[db_type];
    struct stat st;
    if (stat(ctx->path, &st) == 0) {
        return (double)st.st_size / (1024.0 * 1024.0);
    }
    return 0.0;
}

/**
 * @brief 强制删除最旧的一半记录
 */
static int db_delete_oldest_half(tly_db_type_t db_type)
{
    tly_db_context_t *ctx = &g_offline_db.dbs[db_type];
    if (!ctx->db) return 0;
    
    /* 获取总记录数 */
    int total_count = 0;
    sqlite3_reset(ctx->stmt_count);
    if (sqlite3_step(ctx->stmt_count) == SQLITE_ROW) {
        total_count = sqlite3_column_int(ctx->stmt_count, 0);
    }
    
    if (total_count <= 1) return 0;
    
    /* 删除最旧的一半 */
    int delete_count = total_count / 2;
    char sql[256];
    snprintf(sql, sizeof(sql),
             "DELETE FROM %s WHERE id IN (SELECT id FROM %s ORDER BY timestamp ASC LIMIT %d);",
             TLY_TABLE_NAME, TLY_TABLE_NAME, delete_count);
    
    char *errmsg = NULL;
    int rc = sqlite3_exec(ctx->db, sql, NULL, NULL, &errmsg);
    if (rc == SQLITE_OK) {
        sqlite3_exec(ctx->db, SQL_VACUUM, NULL, NULL, NULL);
        TLY_LOG_W("Force deleted %d oldest records from %s db (size limit)",
                  delete_count, db_type == TLY_DB_TYPE_ALARM ? "alarm" : "realtime");
        return delete_count;
    } else {
        TLY_LOG_E("Failed to delete oldest records: %s", errmsg ? errmsg : sqlite3_errstr(rc));
        if (errmsg) sqlite3_free(errmsg);
    }
    return 0;
}

/**
 * @brief 检查并限制数据库大小
 */
static void db_check_size_limit(tly_db_type_t db_type)
{
    double size_mb = db_get_size_mb(db_type);
    
    if (size_mb > TLY_DB_MAX_SIZE_MB) {
        TLY_LOG_E("DB size limit exceeded: %.1f MB > %d MB, force cleanup",
                  size_mb, TLY_DB_MAX_SIZE_MB);
        db_delete_oldest_half(db_type);
    } else if (size_mb > TLY_DB_WARNING_SIZE_MB) {
        TLY_LOG_W("DB size warning: %.1f MB > %d MB",
                  size_mb, TLY_DB_WARNING_SIZE_MB);
    }
}
