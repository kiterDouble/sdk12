#include "dbBackupService.h"
#include "sr_log.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   /* access */
#include <fcntl.h>    /* F_OK 也可能需要（有的系统在 unistd.h 里） */
#include <errno.h>
#include <sqlite3.h>

/* =======================================================================
 * SQLite 在线热备份
 *
 * 参考 SirunTboxSDK param_backup.c v2.0 策略：
 * - 使用 sqlite3_backup API 而非文件拷贝，保证拷贝的是某一时刻的完整快照，
 *   彻底解决 SQLite WAL 模式下文件拷贝可能拷到事务中间态的问题。
 * - 写同目录临时文件 → rename，保证目标文件要么是旧的完整版本、要么是新的，
 *   中途断电/异常不会留下半截损坏文件。
 * ======================================================================= */

#define DBBK_PATH_MAX_LEN      (512U)
#define DBBK_HEALTH_CHECK_NS   (30U * 1000U * 1000U)  /* 健康巡检周期 30 秒 */

/*
 * 验证数据库完整性。
 * quickMode=TRUE: 仅 quick_check（校验 B-tree 结构，速度快）；
 * quickMode=FALSE: integrity_check（全量页扫描，能发现位翻转等物理损坏，较慢）。
 * 成功返回 TRUE，损坏或无法打开返回 FALSE。
 */
static gboolean DbVerifyIntegrity(const char *dbPath, gboolean quickMode)
{
    gboolean isValid = FALSE;
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = quickMode ? "PRAGMA quick_check;" : "PRAGMA integrity_check;";

    if (dbPath == NULL)
    {
        return FALSE;
    }

    if (sqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
    {
        LOG_Error("[DB] verify: open failed (%s)", dbPath);
        if (db != NULL)
        {
            sqlite3_close_v2(db);
        }
        return FALSE;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK &&
        sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *res = (const char *)sqlite3_column_text(stmt, 0);
        isValid = (res != NULL && strcmp(res, "ok") == 0);
        if (!isValid)
        {
            LOG_Error("[DB] verify: %s failed (%s): %s",
                      quickMode ? "quick_check" : "integrity_check",
                      dbPath, res ? res : "NULL");
        }
    }

    if (stmt != NULL)
    {
        (void)sqlite3_finalize(stmt);
    }
    (void)sqlite3_close_v2(db);
    return isValid;
}

/*
 * 两级完整性校验：先 quick_check（快），失败再升级到 integrity_check（慢但准）。
 * 参考 SirunTboxSDK param_backup.c v2.0 策略。
 */
static gboolean DbVerifyIntegrityTwoLevel(const char *dbPath)
{
    if (DbVerifyIntegrity(dbPath, TRUE))
    {
        return TRUE;
    }
    return DbVerifyIntegrity(dbPath, FALSE);
}

/*
 * 清理 SQLite WAL/SHM sidecar 文件。
 * 主库被 rename 替换（Online Backup 恢复）后，旧的 -wal/-shm 可能残留。
 * 若不清理，SQLite 打开新主库时会 replay 旧 WAL 的页，导致数据损坏。
 * 本函数在恢复主库成功后调用。
 */
static void RemoveWalShmFiles(const char *dbPath)
{
    char sidecar[DBBK_PATH_MAX_LEN];

    if ((dbPath == NULL) || (dbPath[0] == '\0'))
    {
        return;
    }

    (void)snprintf(sidecar, sizeof(sidecar), "%s-wal", dbPath);
    (void)unlink(sidecar);

    (void)snprintf(sidecar, sizeof(sidecar), "%s-shm", dbPath);
    (void)unlink(sidecar);
}

/*
 * 提取文件路径的目录部分。
 * 例如：/a/b/c.txt -> /a/b 
 */
static gchar *ExtractDirPath(const char *filePath)
{
    char *lastSlash = NULL;

    if ((filePath == NULL) || (filePath[0] == '\0'))
    {
        return NULL;
    }

    lastSlash = strrchr(filePath, '/');
    if (lastSlash == NULL)
    {
        return g_strdup(".");
    }

    return g_strndup(filePath, (gsize)(lastSlash - filePath));
}

/*
 * 确保目标文件所在目录存在，不存在则创建。
 * 成功返回 TRUE，失败返回 FALSE。
 */
static gboolean EnsureDirExists(const char *filePath)
{
    gchar *dirPath = NULL;
    int ret = 0;

    if ((filePath == NULL) || (filePath[0] == '\0'))
    {
        return FALSE;
    }

    dirPath = ExtractDirPath(filePath);
    if (dirPath == NULL)
    {
        return FALSE;
    }

    /* 检查目录是否存在 */
    if (access(dirPath, F_OK) != 0)
    {
        /* 目录不存在，尝试创建（-p 模式，递归创建父目录） */
        gchar *mkdirCmd = g_strdup_printf("mkdir -p '%s'", dirPath);
        if (mkdirCmd == NULL)
        {
            g_free(dirPath);
            return FALSE;
        }

        ret = system(mkdirCmd);
        g_free(mkdirCmd);

        if (ret != 0)
        {
            LOG_Error("[DB] backup: create dir failed (%s), ret=%d", dirPath, ret);
            g_free(dirPath);
            return FALSE;
        }
        LOG_Info("[DB] backup: created dir (%s)", dirPath);
    }

    g_free(dirPath);
    return TRUE;
}

/*
 * 使用 SQLite Online Backup API 执行热备份。
 * 接口语义与文件拷贝一致：srcPath → dstPath，成功返回 TRUE。
 * 原子流程：打开源(只读) → 创建 dst.tmp → backup_step(-1) → 关闭 → rename。
 */
static gboolean CopyByCpRf(const char *srcPath, const char *dstPath)
{
    gboolean result = FALSE;
    sqlite3 *pSrc = NULL;
    sqlite3 *pDst = NULL;
    sqlite3_backup *pBak = NULL;
    char tmpPath[DBBK_PATH_MAX_LEN];

    if ((srcPath == NULL) || (dstPath == NULL))
    {
        return FALSE;
    }

    /* Step 1: 以只读模式打开源数据库 */
    if (sqlite3_open_v2(srcPath, &pSrc, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
    {
        LOG_Error("[DB] backup: open src failed (%s)", srcPath);
        if (pSrc != NULL)
        {
            sqlite3_close_v2(pSrc);
        }
        return FALSE;
    }

    /* 确保目标目录存在 */
    if (EnsureDirExists(dstPath) == FALSE)
    {
        LOG_Error("[DB] backup: ensure dir failed (%s)", dstPath);
        sqlite3_close_v2(pSrc);
        return FALSE;
    }

    /* Step 2: 创建临时目标文件（与目标同目录，保证 rename 原子） */
    (void)snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", dstPath);
    if (sqlite3_open_v2(tmpPath, &pDst,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK)
    {
        LOG_Error("[DB] backup: open dst.tmp failed (%s)", tmpPath);
        if (pDst != NULL)
        {
            sqlite3_close_v2(pDst);
            (void)unlink(tmpPath);
        }
        sqlite3_close_v2(pSrc);
        return FALSE;
    }

    /* Step 3: 执行在线备份 */
    do
    {
        pBak = sqlite3_backup_init(pDst, "main", pSrc, "main");
        if (pBak == NULL)
        {
            LOG_Error("[DB] backup: backup_init failed: %s", sqlite3_errmsg(pDst));
            break;
        }

        /* -1 表示一次性拷贝所有页面（DID 数据库小，无需分批） */
        if (sqlite3_backup_step(pBak, -1) != SQLITE_DONE)
        {
            LOG_Error("[DB] backup: backup_step failed: %s", sqlite3_errmsg(pDst));
            (void)sqlite3_backup_finish(pBak);
            pBak = NULL;
            break;
        }
        (void)sqlite3_backup_finish(pBak);
        pBak = NULL;

        /* Step 4: 关闭两个句柄后执行原子 rename */
        (void)sqlite3_close_v2(pDst);
        pDst = NULL;
        (void)sqlite3_close_v2(pSrc);
        pSrc = NULL;

        if (rename(tmpPath, dstPath) != 0)
        {
            LOG_Error("[DB] backup: rename failed (%s -> %s), errno=%d", tmpPath, dstPath, errno);
            (void)unlink(tmpPath);
            break;
        }

        /* rename 成功后清理目标路径可能残留的旧 WAL/SHM sidecar。
         * Online Backup 产物是自包含的主库文件（无 -wal/-shm），
         * 但旧文件的 sidecar 可能残留，留着会导致下次打开时 replay 损坏数据。 */
        RemoveWalShmFiles(dstPath);

        result = TRUE;
        LOG_Debug("[DB] backup: hot backup ok (%s -> %s)", srcPath, dstPath);
    } while (0);

    /* 清理 */
    if (pDst != NULL)
    {
        (void)sqlite3_close_v2(pDst);
        (void)unlink(tmpPath);
    }
    if (pSrc != NULL)
    {
        (void)sqlite3_close_v2(pSrc);
    }

    return result;
}

static gboolean FileExists(const char *path)
{
    if (path == NULL)
    {
        return FALSE;
    }

    return (access(path, F_OK) == 0) ? TRUE : FALSE;

}

/* =======================================================================
 * DbBackupService 状态
 * ======================================================================= */

/* 异步备份请求队列 */
static GAsyncQueue *g_queue   = NULL;
static GThread     *g_thread  = NULL;
static gboolean     g_running = FALSE;
static gboolean     g_inited  = FALSE;

/* 路径配置 */
static gchar *g_dbPath     = NULL;
static gchar *g_backupPath = NULL;

/*
 * 全局互斥锁：
 * - 保护 g_inited/g_running/g_queue/g_thread/g_dbPath/g_backupPath
 * - 串行化 Init/DeInit/Start/Stop/Notify/Check
 * - 串行化 Backup/Restore/Check（避免与后台线程并发）
 */
static GMutex g_dbFileMutex;

/* =======================================================================
 * 内部动作（要求：调用者已持锁或内部自行加锁）
 * ======================================================================= */

static void BackupDatabaseLocked(void)
{
    /* 调用者保证已持 g_dbFileMutex */
    do
    {
        if (FileExists(g_dbPath) == FALSE)
        {
            break;
        }

        if (CopyByCpRf(g_dbPath, g_backupPath) == FALSE)
        {
            LOG_Error("[DB] backup failed (hot backup)");
        }

    } while (0);
}

/*
 * 定时健康巡检（参考 SirunTboxSDK param_backup.c v2.0 HandleHealthCheck）。
 * 双库状态决策：
 * - 主库好、备份坏 → 从主库重建备份（修复 SD 卡位翻转等静默损坏）
 * - 主库坏、备份好 → 从备份恢复主库
 * - 双库都坏 → 仅记录错误
 * - 双库都好 → 正常
 * 调用者保证已持 g_dbFileMutex。
 */
static void HealthCheckLocked(void)
{
    gboolean dbExist = FileExists(g_dbPath);
    gboolean bakExist = FileExists(g_backupPath);
    gboolean dbValid = FALSE;
    gboolean bakValid = FALSE;

    if (dbExist)
    {
        dbValid = DbVerifyIntegrityTwoLevel(g_dbPath);
    }
    if (bakExist)
    {
        bakValid = DbVerifyIntegrityTwoLevel(g_backupPath);
    }

    if (dbValid && !bakExist)
    {
        /* 主库好、备份不存在 → 创建备份 */
        LOG_Info("[DB] backup not exist, creating from main DB");
        if (CopyByCpRf(g_dbPath, g_backupPath) == FALSE)
        {
            LOG_Error("[DB] create backup failed");
        }
    }
    else if (dbValid && bakExist && !bakValid)
    {
        /* 主库好、备份存在但损坏 → 先删除损坏的备份，再从主库重建 */
        LOG_Warn("[DB] backup corrupted, rebuilding from main DB");
        (void)unlink(g_backupPath);
        if (CopyByCpRf(g_dbPath, g_backupPath) == FALSE)
        {
            LOG_Error("[DB] rebuild backup failed");
        }
    }
    else if (!dbValid && bakValid)
    {
        /* 主库坏、备份好 → 从备份恢复主库 */
        LOG_Warn("[DB] main DB corrupted, restoring from backup");
        if (dbExist)
        {
            (void)unlink(g_dbPath);
        }
        if (CopyByCpRf(g_backupPath, g_dbPath) == FALSE)
        {
            LOG_Error("[DB] restore from backup failed");
        }
    }
    else if (!dbValid && !bakValid)
    {
        LOG_Error("[DB] both DB and backup are %s",
                  (dbExist || bakExist) ? "corrupted" : "missing");
    }
    else
    {
        /* 双库都健康，无需操作 */
    }
}

static gboolean RestoreDatabaseImmediateLocked(void)
{
    gboolean ret = FALSE;

    /* 调用者保证已持 g_dbFileMutex */
    do
    {
        if (FileExists(g_backupPath) == FALSE)
        {
            LOG_Fatal("[DB] restore failed: backup not exist");
            break;
        }

        /* 恢复前先验证备份文件完整性（两级校验），避免用损坏的备份覆盖主库 */
        if (DbVerifyIntegrityTwoLevel(g_backupPath) == FALSE)
        {
            LOG_Fatal("[DB] restore failed: backup corrupted");
            break;
        }

        if (CopyByCpRf(g_backupPath, g_dbPath) == FALSE)
        {
            LOG_Fatal("[DB] restore failed (hot backup)");
            break;
        }

        ret = TRUE;

    } while (0);

    return ret;
}

/* =======================================================================
 * 备份线程
 * ======================================================================= */

static gpointer DbBackupThread(gpointer data)
{
    (void)data;

    while (1)
    {
        /* 带超时等待：被通知唤醒则备份；30s 超时则执行健康巡检 */
        gpointer msg = g_async_queue_timeout_pop(g_queue, DBBK_HEALTH_CHECK_NS);

        g_mutex_lock(&g_dbFileMutex);

        if (g_running == FALSE)
        {
            g_mutex_unlock(&g_dbFileMutex);
            break;
        }

        if (msg != NULL)
        {
            /* 收到通知 → 执行备份 */
            BackupDatabaseLocked();
        }
        else
        {
            /* 超时无通知 → 健康巡检（检测备份/主库静默损坏） */
            HealthCheckLocked();
        }

        g_mutex_unlock(&g_dbFileMutex);
    }

    return NULL;
}

/* =======================================================================
 * 对外接口
 * ======================================================================= */

gboolean DbBackupServiceInit(const DbBackupConfig_t *cfg)
{
    gboolean ret = FALSE;
    gchar *dbTmp = NULL;
    gchar *bakTmp = NULL;

    if ((cfg == NULL) ||
        (cfg->dbFilePath == NULL) ||
        (cfg->backupFilePath == NULL))
    {
        LOG_Error("[DB] init failed: invalid config");
        return FALSE;
    }

    g_mutex_lock(&g_dbFileMutex);

    do
    {
        if (g_inited != FALSE)
        {
            ret = TRUE;
            break;
        }

        dbTmp  = g_strdup(cfg->dbFilePath);
        bakTmp = g_strdup(cfg->backupFilePath);

        if ((dbTmp == NULL) || (bakTmp == NULL))
        {
            LOG_Fatal("[DB] init failed: no memory");
            ret = FALSE;
            break;
        }

        /* 提交：一次性更新全局，避免半初始化状态 */
        g_dbPath     = dbTmp;
        g_backupPath = bakTmp;
        dbTmp = NULL;
        bakTmp = NULL;

        g_inited = TRUE;

        LOG_Info("[DB] init ok");
        ret = TRUE;

    } while (0);

    g_mutex_unlock(&g_dbFileMutex);

    /* 失败路径释放临时资源（双保险） */
    if (dbTmp != NULL)
    {
        g_free(dbTmp);
        dbTmp = NULL;
    }
    if (bakTmp != NULL)
    {
        g_free(bakTmp);
        bakTmp = NULL;
    }

    return ret;
}

gboolean DbBackupServiceStart(void)
{
    gboolean ret = FALSE;

    g_mutex_lock(&g_dbFileMutex);

    do
    {
        if ((g_inited == FALSE) || (g_running != FALSE))
        {
            ret = FALSE;
            break;
        }

        g_queue = g_async_queue_new();
        if (g_queue == NULL)
        {
            LOG_Fatal("[DB] start failed: queue create error");
            ret = FALSE;
            break;
        }

        g_running = TRUE;

        g_thread = g_thread_new("db-backup", DbBackupThread, NULL);
        if (g_thread == NULL)
        {
            LOG_Fatal("[DB] start failed: thread create error");
            g_running = FALSE;

            g_async_queue_unref(g_queue);
            g_queue = NULL;

            ret = FALSE;
            break;
        }

        LOG_Info("[DB] service started");
        ret = TRUE;

    } while (0);

    g_mutex_unlock(&g_dbFileMutex);

    return ret;
}

void DbBackupServiceStop(void)
{
    GThread *threadToJoin = NULL;
    GAsyncQueue *queueToUnref = NULL;

    g_mutex_lock(&g_dbFileMutex);

    do
    {
        if (g_running == FALSE)
        {
            break;
        }

        g_running = FALSE;

        /* 保存句柄，锁外 join/unref（避免潜在死锁/长时间占锁） */
        threadToJoin = g_thread;
        queueToUnref = g_queue;

        /* 唤醒线程退出 */
        if (g_queue != NULL)
        {
            (void)g_async_queue_push(g_queue, GINT_TO_POINTER(1));
        }

        g_thread = NULL;
        g_queue  = NULL;

    } while (0);

    g_mutex_unlock(&g_dbFileMutex);

    /* 锁外收尾 */
    if (threadToJoin != NULL)
    {
        (void)g_thread_join(threadToJoin);
        g_thread_unref(threadToJoin);
        threadToJoin = NULL;
    }

    if (queueToUnref != NULL)
    {
        g_async_queue_unref(queueToUnref);
        queueToUnref = NULL;
    }

    LOG_Info("[DB] service stopped");
}

void DbBackupServiceDeInit(void)
{
    /* 先停服务（内部已处理并发） */
    DbBackupServiceStop();

    g_mutex_lock(&g_dbFileMutex);

    do
    {
        if (g_dbPath != NULL)
        {
            g_free(g_dbPath);
            g_dbPath = NULL;
        }

        if (g_backupPath != NULL)
        {
            g_free(g_backupPath);
            g_backupPath = NULL;
        }

        g_inited = FALSE;

    } while (0);

    g_mutex_unlock(&g_dbFileMutex);
}

void DbBackupNotifyBackup(void)
{
    g_mutex_lock(&g_dbFileMutex);

    do
    {
        if ((g_running == FALSE) || (g_queue == NULL))
        {
            break;
        }

        (void)g_async_queue_push(g_queue, GINT_TO_POINTER(1));

    } while (0);

    g_mutex_unlock(&g_dbFileMutex);
}

gboolean DbBackupCheckAndRestore(void)
{
    gboolean ret = FALSE;
    gboolean dbExist;
    gboolean bakExist;

    g_mutex_lock(&g_dbFileMutex);

    do
    {
        if (g_inited == FALSE)
        {
            LOG_Error("[DB] check failed: service not initialized");
            ret = FALSE;
            break;
        }

        dbExist  = FileExists(g_dbPath);
        bakExist = FileExists(g_backupPath);

        if (dbExist != FALSE)
        {
            ret = TRUE;
            break;
        }

        if ((dbExist == FALSE) && (bakExist != FALSE))
        {
            LOG_Notice("[DB] db missing, restoring from backup");

            if (RestoreDatabaseImmediateLocked() == FALSE)
            {
                ret = FALSE;
                break;
            }

            LOG_Notice("[DB] restored from backup");
            ret = TRUE;
            break;
        }

        LOG_Fatal("[DB] no db and no backup");
        ret = FALSE;

    } while (0);

    g_mutex_unlock(&g_dbFileMutex);

    return ret;
}
