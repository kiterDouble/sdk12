/* ========================================================================== */
/*  File       : dbDriver.c                                                   */
/*  Module     : SQLite DB Driver (Single Lock + Long Connection)             */
/* -------------------------------------------------------------------------- */
/*  设计目标（车规取向）：                                                     */
/*    - 单进程，多线程读写                                                     */
/*    - 全局仅一个 sqlite3* 长连接（DbOpen 打开一次后复用）                    */
/*    - 全模块仅一把锁 g_dbMutex：粒度大、数量少，避免死锁                     */
/*    - 所有 DB 操作（open/close/recover/prepare/step/finalize/exec）均串行化  */
/*    - 不做备份（避免无限备份/删除失败导致空间不可控）                        */
/*    - 恢复策略：normal open -> cleanup wal/shm -> retry open -> recreate     */
/* -------------------------------------------------------------------------- */
/*  日志规范：                                                                 */
/*    - 英文 key=value 格式，统一前缀：[DB][LEVEL][OP]                         */
/*    - 本工程只有 LOG_Error 时，用前缀区分 INFO/WARN/ERROR                    */
/* ========================================================================== */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "sr_log.h"
#include "err_code.h"
#include "dbDriver.h"
#include "udsTypes.h"
#include "dtcRpcApi.h"
#include "dbBackupService.h"

/* ============================== Parameters ================================= */

static const int kBusyTimeoutMs       = 300;   /* busy timeout：降低瞬时 SQLITE_BUSY/LOCKED */
static const int kWalAutoCkptPages    = 2000;  /* WAL 自动 checkpoint 页数 */
static const int kCkptRetryTimes      = 5;     /* checkpoint 重试次数 */
static const int kCkptBackoffMs       = 50;    /* checkpoint 退避（ms） */

/* ============================== Globals ==================================== */

/* 全局长连接句柄 */
static sqlite3 *g_db = NULL;

/* 单锁策略：同时保护 open/close/recover 与所有 DB 访问 */
static pthread_mutex_t g_dbMutex = PTHREAD_MUTEX_INITIALIZER;

/* ============================== Utilities ================================== */

/**
 * @brief 休眠指定毫秒
 *
 * Step1: 参数校验
 * Step2: usleep 进行毫秒级休眠
 *
 * @param[in] ms 休眠时间（毫秒）
 */
static void DbSleepMs(int ms)
{
    /* Step1: 参数校验 */
    if (ms <= 0)
    {
        return;
    }

    /* Step2: usleep */
    (void)usleep((useconds_t)ms * 1000U);
}

/**
 * @brief 清理 SQLite WAL/SHM 残留文件
 *
 * Step1: 参数校验
 * Step2: 拼接 -wal/-shm 文件名
 * Step3: remove 尝试删除（忽略不存在）
 *
 * @param[in] dbPath 主库路径
 *
 * @note 调用方必须已持有 g_dbMutex
 */
static void DbRemoveWalShmLocked(const char *dbPath)
{
    char wal[512];
    char shm[512];

    /* Step1: 参数校验 */
    if ((dbPath == NULL) || (dbPath[0] == '\0'))
    {
        return;
    }

    /* Step2: 拼接文件名 */
    (void)snprintf(wal, sizeof(wal), "%s-wal", dbPath);
    (void)snprintf(shm, sizeof(shm), "%s-shm", dbPath);

    /* Step3: 删除文件（忽略错误） */
    (void)remove(wal);
    (void)remove(shm);
}

/**
 * @brief 执行无结果 SQL（DDL/PRAGMA）
 *
 * Step1: 参数校验
 * Step2: sqlite3_exec 执行
 * Step3: 错误则输出 rc/sql/errmsg
 *
 * @param[in] db  sqlite3 句柄
 * @param[in] sql SQL 字符串
 *
 * @retval RET_OK  成功
 * @retval RET_ERR 失败
 *
 * @note 调用方必须已持有 g_dbMutex
 */
static int DbExecNoResultLocked(sqlite3 *db, const char *sql)
{
    char *errmsg = NULL;
    int rc;

    /* Step1: 参数校验 */
    if ((db == NULL) || (sql == NULL))
    {
        return RET_ERR;
    }

    /* Step2: 执行 SQL */
    rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);

    /* Step3: 错误处理 */
    if (rc != SQLITE_OK)
    {
        if (errmsg != NULL)
        {
            LOG_Error("[DB][ERROR][EXEC] rc=%d(%s) sql=%s errmsg=%s",
                      rc, sqlite3_errstr(rc), sql, errmsg);
            sqlite3_free(errmsg);
        }
        else
        {
            LOG_Error("[DB][ERROR][EXEC] rc=%d(%s) sql=%s",
                      rc, sqlite3_errstr(rc), sql);
        }
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @brief 设置连接级 PRAGMA
 *
 * Step1: 开启 extended_result_codes
 * Step2: 设置 busy_timeout
 * Step3: 设置 journal_mode=WAL
 * Step4: 设置 synchronous=NORMAL
 * Step5: 设置 wal_autocheckpoint
 *
 * @param[in] db sqlite3 句柄
 *
 * @retval RET_OK  成功
 * @retval RET_ERR 失败
 *
 * @note 调用方必须已持有 g_dbMutex
 */
static int DbApplyPragmasLocked(sqlite3 *db)
{
    int ret = RET_ERR;

    do
    {
        /* Step1: 参数校验 */
        if (db == NULL)
        {
            break;
        }

        /* Step2: 扩展错误码 */
        (void)sqlite3_extended_result_codes(db, 1);

        /* Step3: busy_timeout */
        (void)sqlite3_busy_timeout(db, kBusyTimeoutMs);

        /* Step4: WAL + synchronous */
        (void)sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
        (void)sqlite3_exec(db, "PRAGMA synchronous=NORMAL;", NULL, NULL, NULL);

        /* Step5: wal_autocheckpoint */
        {
            char sql[64];
            (void)snprintf(sql, sizeof(sql), "PRAGMA wal_autocheckpoint=%d;", kWalAutoCkptPages);
            (void)sqlite3_exec(db, sql, NULL, NULL, NULL);
        }

        ret = RET_OK;

    } while (0);

    return ret;
}

/**
 * @brief 启动恢复：checkpoint(TRUNCATE)
 *
 * Step1: 参数校验
 * Step2: 执行 wal_checkpoint(TRUNCATE)
 * Step3: BUSY/LOCKED 退避重试
 * Step4: 其他错误输出日志并返回
 * Step5: 重试耗尽输出日志
 *
 * @param[in] db sqlite3 句柄
 *
 * @note 调用方必须已持有 g_dbMutex
 */
static void DbStartupCheckpointLocked(sqlite3 *db)
{
    int i;

    /* Step1: 参数校验 */
    if (db == NULL)
    {
        return;
    }

    /* Step2: 多次尝试 checkpoint(TRUNCATE) */
    for (i = 0; i < kCkptRetryTimes; i++)
    {
        int rc = sqlite3_exec(db, "PRAGMA wal_checkpoint(TRUNCATE);", NULL, NULL, NULL);

        /* Step3: 成功返回 */
        if (rc == SQLITE_OK)
        {
            return;
        }

        /* Step4: BUSY/LOCKED -> 退避重试 */
        if ((rc == SQLITE_BUSY) || (rc == SQLITE_LOCKED))
        {
            DbSleepMs(kCkptBackoffMs);
            continue;
        }

        /* Step5: 其他错误 -> 记录并返回 */
        LOG_Error("[DB][ERROR][CKPT] rc=%d(%s) cmd=wal_checkpoint(TRUNCATE)",
                  rc, sqlite3_errstr(rc));
        return;
    }

    /* Step6: 重试耗尽 */
    LOG_Error("[DB][WARN][CKPT] retry_exhausted cmd=wal_checkpoint(TRUNCATE)");
}

/**
 * @brief 确保数据表存在
 *
 * Step1: 参数校验
 * Step2: 执行 CREATE TABLE IF NOT EXISTS
 *
 * @param[in] db sqlite3 句柄
 *
 * @retval RET_OK  成功
 * @retval RET_ERR 失败
 *
 * @note 调用方必须已持有 g_dbMutex
 */
static int DbEnsureSchemaOnDbLocked(sqlite3 *db)
{
    static const char *DDL =
        "CREATE TABLE IF NOT EXISTS did_store ("
        "  did   INTEGER PRIMARY KEY,"
        "  value BLOB NOT NULL"
        ");";

    /* Step1: 参数校验 */
    if (db == NULL)
    {
        return RET_ERR;
    }

    /* Step2: 执行建表语句 */
    return DbExecNoResultLocked(db, DDL);
}

/**
 * @brief 关闭全局数据库连接
 *
 * Step1: 判空
 * Step2: sqlite3_close_v2
 * Step3: 失败输出日志（通常意味着 stmt 未 finalize）
 * Step4: 置空 g_db
 *
 * @note 调用方必须已持有 g_dbMutex
 */
static void DbCloseGlobalLocked(void)
{
    /* Step1: 判空 */
    if (g_db == NULL)
    {
        return;
    }

    /* Step2: close_v2 */
    {
        int rc = sqlite3_close_v2(g_db);

        /* Step3: 失败日志 */
        if (rc != SQLITE_OK)
        {
            LOG_Error("[DB][ERROR][CLOSE] rc=%d(%s) hint=possible_unfinalized_stmt",
                      rc, sqlite3_errstr(rc));
        }
    }

    /* Step4: 置空句柄 */
    g_db = NULL;
}

/**
 * @brief 重建数据库（新建库 + PRAGMA + checkpoint + schema）
 *
 * Step1: 关闭旧连接
 * Step2: open_v2 创建/打开新库
 * Step3: 设置 PRAGMA
 * Step4: 启动 checkpoint(TRUNCATE)
 * Step5: 建表/校验 schema
 * Step6: 失败则关闭并置空
 *
 * @retval RET_OK  成功
 * @retval RET_ERR 失败
 *
 * @note 调用方必须已持有 g_dbMutex
 */
static int DbRecreateDatabaseLocked(void)
{
    int ret = RET_ERR;
    int rc;

    do
    {
        /* Step1: 关闭旧连接 */
        DbCloseGlobalLocked();

        /* Step2: 打开/创建新库 */
        rc = sqlite3_open_v2(UDS_DB_PATH,
                             &g_db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                             NULL);
        if ((rc != SQLITE_OK) || (g_db == NULL))
        {
            LOG_Error("[DB][ERROR][RECREATE] phase=open_failed rc=%d(%s) path=%s",
                      rc, sqlite3_errstr(rc), UDS_DB_PATH);
            DbCloseGlobalLocked();
            break;
        }

        /* Step3: PRAGMA */
        if (DbApplyPragmasLocked(g_db) != RET_OK)
        {
            LOG_Error("[DB][ERROR][RECREATE] phase=apply_pragmas_failed path=%s", UDS_DB_PATH);
            break;
        }

        /* Step4: checkpoint */
        DbStartupCheckpointLocked(g_db);

        /* Step5: schema */
        if (DbEnsureSchemaOnDbLocked(g_db) != RET_OK)
        {
            LOG_Error("[DB][ERROR][RECREATE] phase=ensure_schema_failed path=%s", UDS_DB_PATH);
            break;
        }

        /* Step6: 成功 */
        ret = RET_OK;

    } while (0);

    /* Step7: 失败收尾 */
    if (ret != RET_OK)
    {
        DbCloseGlobalLocked();
    }

    return ret;
}

/**
 * @brief 打开数据库（带恢复策略）
 *
 * Step1: 已打开则直接返回成功
 * Step2: normal open 成功则 PRAGMA + checkpoint + schema
 * Step3: normal open 失败则 close，cleanup wal/shm，retry open
 * Step4: retry open 成功则 PRAGMA + checkpoint + schema
 * Step5: retry 失败则 cleanup wal/shm，recreate（不备份）
 * Step6: 失败收尾，确保句柄不会半开半关
 *
 * @retval RET_OK  成功
 * @retval RET_ERR 失败
 *
 * @note 调用方必须已持有 g_dbMutex（本函数内部不加锁）
 */
static int DbOpenWithRecoveryLocked(void)
{
    int ret = RET_ERR;
    int rc;

    do
    {
        /* Step1: 已打开则返回 */
        if (g_db != NULL)
        {
            ret = RET_OK;
            break;
        }

        /* ==================== Step2: normal open ==================== */
        rc = sqlite3_open_v2(UDS_DB_PATH,
                             &g_db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                             NULL);

        if ((rc == SQLITE_OK) && (g_db != NULL))
        {
            /* Step2.1: PRAGMA */
            if (DbApplyPragmasLocked(g_db) != RET_OK)
            {
                LOG_Error("[DB][ERROR][OPEN] phase=apply_pragmas_failed path=%s", UDS_DB_PATH);
                break;
            }

            /* Step2.2: checkpoint */
            DbStartupCheckpointLocked(g_db);

            /* Step2.3: schema */
            if (DbEnsureSchemaOnDbLocked(g_db) != RET_OK)
            {
                LOG_Error("[DB][ERROR][OPEN] phase=ensure_schema_failed path=%s", UDS_DB_PATH);
                break;
            }

            /* Step2.4: 成功 */
            ret = RET_OK;
            break;
        }

        /* Step2.5: normal open 失败日志 */
        LOG_Error("[DB][ERROR][OPEN] phase=normal_open rc=%d(%s) path=%s",
                  rc, sqlite3_errstr(rc), UDS_DB_PATH);

        /* Step2.6: 关闭并置空 */
        DbCloseGlobalLocked();

        /* ==================== Step3: cleanup and retry ==================== */
        /* Step3.1: 清理 wal/shm */
        DbRemoveWalShmLocked(UDS_DB_PATH);

        /* Step3.2: retry open */
        rc = sqlite3_open_v2(UDS_DB_PATH,
                             &g_db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                             NULL);

        if ((rc == SQLITE_OK) && (g_db != NULL))
        {
            /* Step3.3: PRAGMA */
            if (DbApplyPragmasLocked(g_db) != RET_OK)
            {
                LOG_Error("[DB][ERROR][OPEN] phase=retry_apply_pragmas_failed path=%s", UDS_DB_PATH);
                break;
            }

            /* Step3.4: checkpoint */
            DbStartupCheckpointLocked(g_db);

            /* Step3.5: schema */
            if (DbEnsureSchemaOnDbLocked(g_db) != RET_OK)
            {
                LOG_Error("[DB][ERROR][OPEN] phase=retry_ensure_schema_failed path=%s", UDS_DB_PATH);
                break;
            }

            /* Step3.6: 成功 */
            ret = RET_OK;
            break;
        }

        /* Step3.7: retry 失败日志 */
        LOG_Error("[DB][ERROR][OPEN] phase=retry_after_cleanup rc=%d(%s) path=%s",
                  rc, sqlite3_errstr(rc), UDS_DB_PATH);

        /* Step3.8: 关闭并置空 */
        DbCloseGlobalLocked();

        /* ==================== Step4: recreate without backup ==================== */
        /* Step4.1: 记录恢复触发（轻量采证） */
        LOG_Error("[DB][WARN][OPEN] phase=recovery_triggered action=recreate path=%s", UDS_DB_PATH);

        /* Step4.2: 再次清理 wal/shm（确保干净） */
        DbRemoveWalShmLocked(UDS_DB_PATH);

        /* Step4.3: 重建 */
        ret = DbRecreateDatabaseLocked();
        if (ret != RET_OK)
        {
            LOG_Error("[DB][ERROR][OPEN] phase=recreate_failed hint=check_fs_permission_storage");
            break;
        }

        /* Step4.4: 成功 */
        ret = RET_OK;

    } while (0);

    /* Step5: 总失败收尾（避免句柄半开半关） */
    if (ret != RET_OK)
    {
        DbCloseGlobalLocked();
    }

    return ret;
}

/* ============================== Public APIs ================================= */

/**
 * @brief 打开数据库（全局长连接，带恢复策略）
 *
 * Step1: 加 DB 全局锁（单锁策略）
 * Step2: 调用 DbOpenWithRecoveryLocked 打开/恢复
 * Step3: 解锁并返回
 *
 * @param[in] path 数据库路径（兼容保留；当前使用 UDS_DB_PATH）
 *
 * @retval RET_OK  成功（或已打开）
 * @retval RET_ERR 失败
 */
int DbOpen(const char *path)
{
    int ret;

    (void)path;

    /* Step1: 加锁 */
    (void)pthread_mutex_lock(&g_dbMutex);

    /* Step2: 打开/恢复（内部不加锁） */
    ret = DbOpenWithRecoveryLocked();

    /* Step3: 解锁 */
    (void)pthread_mutex_unlock(&g_dbMutex);

    return ret;
}

/**
 * @brief 关闭数据库（释放长连接）
 *
 * Step1: 加 DB 全局锁
 * Step2: close 并置空句柄
 * Step3: 解锁
 */
void DbClose(void)
{
    /* Step1: 加锁 */
    (void)pthread_mutex_lock(&g_dbMutex);

    /* Step2: close */
    DbCloseGlobalLocked();

    /* Step3: 解锁 */
    (void)pthread_mutex_unlock(&g_dbMutex);
}

/**
 * @brief 确保数据表结构存在
 *
 * Step1: 加 DB 全局锁
 * Step2: 校验 db 句柄
 * Step3: 确保 schema
 * Step4: 解锁并返回
 *
 * @retval NRC_NONE_E           成功
 * @retval NRC_GENERAL_REJECT_E 失败
 */
int DbEnsureSchema(void)
{
    int ret = (int)NRC_GENERAL_REJECT_E;

    /* Step1: 加锁 */
    (void)pthread_mutex_lock(&g_dbMutex);

    do
    {
        /* Step2: 校验句柄 */
        if (g_db == NULL)
        {
            LOG_Error("[DB][ERROR][SCHEMA] phase=no_db_handle");
            break;
        }

        /* Step3: ensure schema */
        if (DbEnsureSchemaOnDbLocked(g_db) != RET_OK)
        {
            LOG_Error("[DB][ERROR][SCHEMA] phase=ensure_schema_failed");
            break;
        }

        /* Step4: 成功 */
        ret = (int)NRC_NONE_E;

    } while (0);

    /* Step5: 解锁 */
    (void)pthread_mutex_unlock(&g_dbMutex);

    return ret;
}

/**
 * @brief 写入或更新 DID 数据（BLOB）
 *
 * Step1: 参数校验
 * Step2: 加 DB 全局锁
 * Step3: 校验 db 句柄
 * Step4: prepare SQL
 * Step5: bind 参数（did + blob）
 * Step6: step 执行
 * Step7: finalize
 * Step8: 解锁并返回
 *
 * @param[in] did  DID
 * @param[in] buf  数据
 * @param[in] len  数据长度
 *
 * @retval NRC_NONE_E           成功
 * @retval NRC_GENERAL_REJECT_E 失败
 *
 * @note 不使用 sqlite3_exec：需要 bind + BLOB，且拼 SQL 不安全
 */

int DbUpsertValueByDid(uint16_t did, const uint8_t *buf, uint16_t len){
    return DbUpsertValueByDidEx(did, buf, len, E_DB_BAK_UPDATE_ENABLE);
}

int DbUpsertValueByDidEx(uint16_t did,
                         const uint8_t *buf,
                         uint16_t len,
                         DbBackupUpdateFlag_e updateBackup)
{
    int ret = (int)NRC_GENERAL_REJECT_E;
    sqlite3_stmt *st = NULL;

    static const char *SQL =
        "INSERT INTO did_store(did,value) VALUES(?1,?2) "
        "ON CONFLICT(did) DO UPDATE SET value=excluded.value;";

    do
    {
        /* Step1: 参数校验 */
        if ((buf == NULL) || (len == 0U))
        {
            LOG_Error("[DB][ERROR][UPSERT] phase=invalid_param did=0x%04X len=%u",
                      did, (unsigned)len);
            break;
        }

        /* Step2: 加锁 */
        (void)pthread_mutex_lock(&g_dbMutex);

        /* Step3: 校验句柄 */
        if (g_db == NULL)
        {
            LOG_Error("[DB][ERROR][UPSERT] phase=no_db_handle did=0x%04X", did);
            (void)pthread_mutex_unlock(&g_dbMutex);
            break;
        }

        /* Step4: prepare */
        if (sqlite3_prepare_v2(g_db, SQL, -1, &st, NULL) != SQLITE_OK)
        {
            LOG_Error("[DB][ERROR][UPSERT] phase=prepare did=0x%04X errmsg=%s",
                      did, sqlite3_errmsg(g_db));
            (void)pthread_mutex_unlock(&g_dbMutex);
            break;
        }

        /* Step5: bind */
        (void)sqlite3_bind_int(st, 1, (int)did);
        (void)sqlite3_bind_blob(st, 2, (const void *)buf, (int)len, SQLITE_TRANSIENT);

        /* Step6: step */
        {
            int rc = sqlite3_step(st);
            if (rc != SQLITE_DONE)
            {
                LOG_Error("[DB][ERROR][UPSERT] phase=step did=0x%04X rc=%d(%s) errcode=%d xerr=%d errmsg=%s",
                          did,
                          rc, sqlite3_errstr(rc),
                          sqlite3_errcode(g_db),
                          sqlite3_extended_errcode(g_db),
                          sqlite3_errmsg(g_db));
                (void)sqlite3_finalize(st);
                st = NULL;
                (void)pthread_mutex_unlock(&g_dbMutex);
                break;
            }
        }

        /* Step7: finalize */
        (void)sqlite3_finalize(st);
        st = NULL;

        /* Step8: 解锁并成功 */
        (void)pthread_mutex_unlock(&g_dbMutex);
        ret = (int)NRC_NONE_E;

    } while (0);

    /* Step9: 兜底 finalize */
    if (st != NULL)
    {
        (void)sqlite3_finalize(st);
    }

    if (updateBackup == E_DB_BAK_UPDATE_ENABLE) {
        DbBackupNotifyBackup();
    }

    return ret;
}

/**
 * @brief 读取 DID 数据（BLOB）
 *
 * Step1: 参数校验
 * Step2: 加 DB 全局锁
 * Step3: 校验 db 句柄
 * Step4: prepare SQL
 * Step5: bind did
 * Step6: step 获取行
 * Step7: 读取 blob 并检查缓冲区长度
 * Step8: finalize
 * Step9: 解锁并返回
 *
 * @param[in]     did     DID
 * @param[out]    buf     输出缓冲区
 * @param[in,out] io_len  输入为 buf 容量；输出为实际长度（或所需长度）
 *
 * @retval NRC_NONE_E                 成功
 * @retval NRC_REQUEST_OUT_OF_RANGE_E 未找到
 * @retval NRC_RESPONSE_TOO_LONG_E    缓冲区不足
 * @retval NRC_GENERAL_REJECT_E       失败
 */
int DbReadValueByDid(uint16_t did, uint8_t *buf, uint16_t *io_len)
{
    int ret = (int)NRC_GENERAL_REJECT_E;
    sqlite3_stmt *st = NULL;

    static const char *SQL = "SELECT value FROM did_store WHERE did=?1;";

    do
    {
        /* Step1: 参数校验 */
        if ((buf == NULL) || (io_len == NULL))
        {
            LOG_Error("[DB][ERROR][READ] phase=invalid_param did=0x%04X", did);
            break;
        }

        /* Step2: 加锁 */
        (void)pthread_mutex_lock(&g_dbMutex);

        /* Step3: 校验句柄 */
        if (g_db == NULL)
        {
            LOG_Error("[DB][ERROR][READ] phase=no_db_handle did=0x%04X", did);
            (void)pthread_mutex_unlock(&g_dbMutex);
            break;
        }

        /* Step4: prepare */
        if (sqlite3_prepare_v2(g_db, SQL, -1, &st, NULL) != SQLITE_OK)
        {
            LOG_Error("[DB][ERROR][READ] phase=prepare did=0x%04X errmsg=%s",
                      did, sqlite3_errmsg(g_db));
            (void)pthread_mutex_unlock(&g_dbMutex);
            break;
        }

        /* Step5: bind did */
        (void)sqlite3_bind_int(st, 1, (int)did);

        /* Step6: step */
        if (sqlite3_step(st) != SQLITE_ROW)
        {
            (void)sqlite3_finalize(st);
            st = NULL;
            (void)pthread_mutex_unlock(&g_dbMutex);

            ret = (int)NRC_REQUEST_OUT_OF_RANGE_E;
            break;
        }

        /* Step7: blob copy + length check */
        {
            const void *p = sqlite3_column_blob(st, 0);
            int n = sqlite3_column_bytes(st, 0);

            if ((uint32_t)n > (uint32_t)(*io_len))
            {
                *io_len = (uint16_t)n;

                (void)sqlite3_finalize(st);
                st = NULL;
                (void)pthread_mutex_unlock(&g_dbMutex);

                ret = (int)NRC_RESPONSE_TOO_LONG_E;
                break;
            }

            if ((p != NULL) && (n > 0))
            {
                (void)memcpy(buf, p, (size_t)n);
            }
            *io_len = (uint16_t)n;
        }

        /* Step8: finalize */
        (void)sqlite3_finalize(st);
        st = NULL;

        /* Step9: 解锁并成功 */
        (void)pthread_mutex_unlock(&g_dbMutex);
        ret = (int)NRC_NONE_E;

    } while (0);

    /* Step10: 兜底 finalize */
    if (st != NULL)
    {
        (void)sqlite3_finalize(st);
    }

    return ret;
}

/**
 * @brief 删除 DID 数据
 *
 * Step1: 加 DB 全局锁
 * Step2: 校验 db 句柄
 * Step3: prepare SQL
 * Step4: bind did
 * Step5: step 执行
 * Step6: sqlite3_changes 判断是否删除成功
 * Step7: finalize
 * Step8: 解锁并返回
 *
 * @param[in] did DID
 *
 * @retval NRC_NONE_E                 删除成功
 * @retval NRC_REQUEST_OUT_OF_RANGE_E 未找到
 * @retval NRC_GENERAL_REJECT_E       失败
 */
int DbDeleteByDid(uint16_t did)
{
    int ret = (int)NRC_GENERAL_REJECT_E;
    sqlite3_stmt *st = NULL;

    static const char *SQL = "DELETE FROM did_store WHERE did=?1;";

    /* Step1: 加锁 */
    (void)pthread_mutex_lock(&g_dbMutex);

    do
    {
        /* Step2: 校验句柄 */
        if (g_db == NULL)
        {
            LOG_Error("[DB][ERROR][DELETE] phase=no_db_handle did=0x%04X", did);
            break;
        }

        /* Step3: prepare */
        if (sqlite3_prepare_v2(g_db, SQL, -1, &st, NULL) != SQLITE_OK)
        {
            LOG_Error("[DB][ERROR][DELETE] phase=prepare did=0x%04X errmsg=%s",
                      did, sqlite3_errmsg(g_db));
            break;
        }

        /* Step4: bind did */
        (void)sqlite3_bind_int(st, 1, (int)did);

        /* Step5: step */
        {
            int rc = sqlite3_step(st);
            if (rc != SQLITE_DONE)
            {
                LOG_Error("[DB][ERROR][DELETE] phase=step did=0x%04X rc=%d(%s) errcode=%d xerr=%d errmsg=%s",
                          did,
                          rc, sqlite3_errstr(rc),
                          sqlite3_errcode(g_db),
                          sqlite3_extended_errcode(g_db),
                          sqlite3_errmsg(g_db));
                break;
            }
        }

        /* Step6: changes 判断 */
        {
            int changed = sqlite3_changes(g_db);
            ret = (changed > 0) ? (int)NRC_NONE_E : (int)NRC_REQUEST_OUT_OF_RANGE_E;
        }

    } while (0);

    /* Step7: finalize */
    if (st != NULL)
    {
        (void)sqlite3_finalize(st);
        st = NULL;
    }

    /* Step8: 解锁 */
    (void)pthread_mutex_unlock(&g_dbMutex);

    return ret;
}

/**
 * @brief 删除数据库文件（内部实现：调用方已持锁）
 *
 * Step1: close 长连接
 * Step2: remove 删除主库文件
 * Step3: 清理 -wal/-shm 残留
 *
 * @param[in] path 数据库路径（NULL/空则使用 UDS_DB_PATH）
 *
 * @retval RET_OK  成功
 * @retval RET_ERR 失败
 *
 * @note 调用方必须已持有 g_dbMutex
 */
static int DbDeleteDatabaseLocked(const char *path)
{
    int ret = RET_ERR;
    const char *p = ((path != NULL) && (path[0] != '\0')) ? path : UDS_DB_PATH;

    do
    {
        /* Step1: 关闭长连接 */
        DbCloseGlobalLocked();

        /* Step2: 删除主库文件 */
        if (remove(p) != 0)
        {
            LOG_Error("[DB][ERROR][FILE] op=delete path=%s errno=%d(%s)",
                      p, errno, strerror(errno));
            break;
        }

        /* Step3: 清理 wal/shm */
        DbRemoveWalShmLocked(p);

        /* Step4: 成功 */
        ret = RET_OK;

    } while (0);

    return ret;
}

/**
 * @brief 删除数据库文件（对外接口）
 *
 * Step1: 加 DB 全局锁
 * Step2: 调用 DbDeleteDatabaseLocked
 * Step3: 解锁并返回
 *
 * @param[in] path 数据库路径（NULL/空则使用 UDS_DB_PATH）
 *
 * @retval RET_OK  成功
 * @retval RET_ERR 失败
 */
int DbDeleteDatabase(const char *path)
{
    int ret;

    /* Step1: 加锁 */
    (void)pthread_mutex_lock(&g_dbMutex);

    /* Step2: 删除 */
    ret = DbDeleteDatabaseLocked(path);

    /* Step3: 解锁 */
    (void)pthread_mutex_unlock(&g_dbMutex);

    return ret;
}

/**
 * @brief 重置数据库（内部实现：调用方已持锁）
 *
 * Step1: 删除数据库文件
 * Step2: DbOpenWithRecoveryLocked 打开（带恢复策略）
 * Step3: 确保 schema
 *
 * @param[in] path 数据库路径（NULL/空则使用 UDS_DB_PATH）
 *
 * @retval RET_OK  成功
 * @retval RET_ERR 失败
 *
 * @note 调用方必须已持有 g_dbMutex
 */
static int DbResetDatabaseLocked(const char *path)
{
    int ret = RET_ERR;

    do
    {
        /* Step1: 删除 */
        if (DbDeleteDatabaseLocked(path) != RET_OK)
        {
            break;
        }

        /* Step2: 打开（带恢复） */
        if (DbOpenWithRecoveryLocked() != RET_OK)
        {
            break;
        }

        /* Step3: schema */
        if (g_db == NULL)
        {
            break;
        }
        if (DbEnsureSchemaOnDbLocked(g_db) != RET_OK)
        {
            break;
        }

        ret = RET_OK;

    } while (0);

    return ret;
}

/**
 * @brief 重置数据库（删除 -> 打开 -> 建表）
 *
 * Step1: 加 DB 全局锁
 * Step2: 调用 DbResetDatabaseLocked
 * Step3: 解锁并返回
 *
 * @param[in] path 数据库路径（NULL/空则使用 UDS_DB_PATH）
 *
 * @retval RET_OK  成功
 * @retval RET_ERR 失败
 */
int DbResetDatabase(const char *path)
{
    int ret;

    /* Step1: 加锁 */
    (void)pthread_mutex_lock(&g_dbMutex);

    /* Step2: reset */
    ret = DbResetDatabaseLocked(path);

    /* Step3: 解锁 */
    (void)pthread_mutex_unlock(&g_dbMutex);

    return ret;
}
