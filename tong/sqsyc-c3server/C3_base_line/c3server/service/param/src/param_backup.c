/**
 * @file param_backup.c
 * @brief 参数数据库备份管理模块
 *
 * 参考 SirunTboxSDK param_backup.c v2.0 策略：
 * - 数据库文件使用 SQLite Online Backup API 拷贝，保证拷贝的是某一时刻的完整快照，
 *   彻底解决文件级拷贝可能拷到事务中间态导致备份损坏的问题。
 * - 写同目录临时文件 → rename，保证目标文件要么是旧的完整版本、要么是新的，
 *   中途断电/异常不会留下半截损坏文件。
 * - 备份/恢复前用 PRAGMA quick_check 校验完整性，避免操作损坏的数据库。
 */

#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include "sr_log.h"
#include "err_code.h"
#include "param_database.h"
#include "param_sync.h"

/* 由 param_backup.c 定义，param_database.c 通过 extern 引用 */
pthread_mutex_t paramfileLock = PTHREAD_MUTEX_INITIALIZER;

#define PARAM_BACKUP_PATH_MAX_LEN (512U)

bool param_fileIsExist(const char *filePath)
{
    if (access(filePath, F_OK) == 0)
    {
        LOG_Debug("File %s exists.", filePath);
        return true;
    }
    else
    {
        LOG_Debug("File %s does not exist.", filePath);
        return false;
    }
}

/**
 * @brief 验证数据库完整性
 *
 * quickMode=true: 仅 quick_check（校验 B-tree 结构，速度快）；
 * quickMode=false: integrity_check（全量页扫描，能发现位翻转等物理损坏，较慢）。
 * @return true 数据库可用，false 损坏或无法打开
 */
static bool param_verifyIntegrityEx(const char *dbPath, bool quickMode)
{
    bool isValid = false;
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = quickMode ? "PRAGMA quick_check;" : "PRAGMA integrity_check;";

    if (dbPath == NULL)
    {
        return false;
    }

    if (sqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
    {
        LOG_Error("verify: open failed (%s)", dbPath);
        if (db != NULL)
        {
            sqlite3_close_v2(db);
        }
        return false;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK &&
        sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *res = (const char *)sqlite3_column_text(stmt, 0);
        isValid = (res != NULL && strcmp(res, "ok") == 0);
        if (!isValid)
        {
            LOG_Error("verify: %s failed (%s): %s",
                      quickMode ? "quick_check" : "integrity_check",
                      dbPath, res ? res : "NULL");
        }
    }

    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
    }
    sqlite3_close_v2(db);
    return isValid;
}

/**
 * @brief 两级完整性校验：先 quick_check（快），失败再升级到 integrity_check（慢但准）
 */
static bool param_verifyIntegrity(const char *dbPath)
{
    if (param_verifyIntegrityEx(dbPath, true))
    {
        return true;
    }
    return param_verifyIntegrityEx(dbPath, false);
}

/**
 * @brief 清理 SQLite WAL/SHM sidecar 文件
 *
 * Online Backup 产物是自包含的主库文件（无 -wal/-shm），
 * 但旧文件的 sidecar 可能残留，留着会导致下次打开时 replay 损坏数据。
 */
static void param_removeWalShm(const char *dbPath)
{
    char sidecar[PARAM_BACKUP_PATH_MAX_LEN];

    if ((dbPath == NULL) || (dbPath[0] == '\0'))
    {
        return;
    }

    (void)snprintf(sidecar, sizeof(sidecar), "%s-wal", dbPath);
    (void)unlink(sidecar);

    (void)snprintf(sidecar, sizeof(sidecar), "%s-shm", dbPath);
    (void)unlink(sidecar);
}

/**
 * @brief 使用 SQLite Online Backup API 执行热备份
 *
 * 原子流程：打开源(只读) → 创建 dst.tmp → backup_step(-1) → 关闭 → rename。
 * 成功返回 true，失败返回 false（失败时清理临时文件，目标文件不受影响）。
 */
static bool param_dbHotBackup(const char *destPath, const char *sourcePath)
{
    bool result = false;
    sqlite3 *pSrc = NULL;
    sqlite3 *pDst = NULL;
    sqlite3_backup *pBak = NULL;
    char tmpPath[PARAM_BACKUP_PATH_MAX_LEN];

    if ((sourcePath == NULL) || (destPath == NULL))
    {
        return false;
    }

    /* Step 1: 以只读模式打开源数据库 */
    if (sqlite3_open_v2(sourcePath, &pSrc, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
    {
        LOG_Error("hot backup: open src failed (%s)", sourcePath);
        if (pSrc != NULL)
        {
            sqlite3_close_v2(pSrc);
        }
        return false;
    }

    /* Step 2: 创建临时目标文件（与目标同目录，保证 rename 原子） */
    (void)snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", destPath);
    if (sqlite3_open_v2(tmpPath, &pDst,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK)
    {
        LOG_Error("hot backup: open dst.tmp failed (%s)", tmpPath);
        if (pDst != NULL)
        {
            sqlite3_close_v2(pDst);
            (void)unlink(tmpPath);
        }
        sqlite3_close_v2(pSrc);
        return false;
    }

    /* Step 3: 执行在线备份 */
    do
    {
        pBak = sqlite3_backup_init(pDst, "main", pSrc, "main");
        if (pBak == NULL)
        {
            LOG_Error("hot backup: backup_init failed: %s", sqlite3_errmsg(pDst));
            break;
        }

        /* -1 表示一次性拷贝所有页面（参数数据库小，无需分批） */
        if (sqlite3_backup_step(pBak, -1) != SQLITE_DONE)
        {
            LOG_Error("hot backup: backup_step failed: %s", sqlite3_errmsg(pDst));
            sqlite3_backup_finish(pBak);
            pBak = NULL;
            break;
        }
        sqlite3_backup_finish(pBak);
        pBak = NULL;

        /* Step 4: 关闭两个句柄后执行原子 rename */
        sqlite3_close_v2(pDst);
        pDst = NULL;
        sqlite3_close_v2(pSrc);
        pSrc = NULL;

        if (rename(tmpPath, destPath) != 0)
        {
            LOG_Error("hot backup: rename failed (%s -> %s), errno=%d", tmpPath, destPath, errno);
            (void)unlink(tmpPath);
            break;
        }

        /* rename 成功后清理目标路径可能残留的旧 WAL/SHM sidecar */
        param_removeWalShm(destPath);

        result = true;
        LOG_Debug("hot backup: ok (%s -> %s)", sourcePath, destPath);
    } while (0);

    /* 清理 */
    if (pDst != NULL)
    {
        sqlite3_close_v2(pDst);
        (void)unlink(tmpPath);
    }
    if (pSrc != NULL)
    {
        sqlite3_close_v2(pSrc);
    }

    return result;
}

/**
 * @brief 拷贝数据库文件
 *
 * 优先使用 SQLite Online Backup API（保证一致性快照）；
 * 若源文件不是有效 SQLite 数据库导致 Online Backup 失败，
 * 回退到 g_file_copy 字节拷贝（兼容非数据库文件/异常场景）。
 */
bool param_fileCopy(const char *destPath, const char *sourcePath)
{
    LOG_Debug("File copy:%s to %s", sourcePath, destPath);

    /* 优先尝试 SQLite 在线热备份 */
    if (param_dbHotBackup(destPath, sourcePath))
    {
        return true;
    }

    /* 回退：g_file_copy 字节拷贝 */
    LOG_Warn("hot backup failed, fallback to g_file_copy: %s -> %s", sourcePath, destPath);
    bool status = false;
    GError *error = NULL;
    GFile *source = g_file_new_for_path(sourcePath);
    GFile *destination = g_file_new_for_path(destPath);

    if (!g_file_copy(source, destination, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error))
    {
        LOG_Error("Error copying file: %s", error->message);
        g_error_free(error);
    }
    else
    {
        LOG_Debug("File copied successfully (fallback).");
        status = true;
    }

    g_object_unref(source);
    g_object_unref(destination);
    return status;
}

void checkAndMkdir(const char *dirName)
{
    if (!g_file_test(dirName, G_FILE_TEST_IS_DIR))
    {
        if (g_mkdir_with_parents(dirName, 0755) == 0)
        {
            LOG_Debug("Directory created successfully or already exists: %s", dirName);
        }
        else
        {
            LOG_Debug("Error creating directory: %s", dirName);
        }
    }
    else
    {
        LOG_Debug("Directory already exists: %s", dirName);
    }
}

/* =======================================================================
 * 备份通知机制（参考 SirunTboxSDK param_backup.c v2.0）
 *
 * 业务线程写 DB 成功后调用 ParamBackupMarkDirty() 通知备份线程，
 * 备份线程立即热备份（跳过完整性检查，刚写入的数据极大概率完好）；
 * 30s 内无写入时执行健康巡检（quick_check 双库，损坏则恢复）。
 * ======================================================================= */

static pthread_mutex_t g_backupMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_backupCond  = PTHREAD_COND_INITIALIZER;
static volatile bool   g_needBackup  = false;

/**
 * @brief 标记有参数更新，唤醒备份线程立即备份
 *
 * 由业务线程（SetParamsValue/SetUserParams/SetParamsSources）在写 DB 成功后调用。
 * 仅设置脏标志 + cond_signal，不做文件 I/O，写路径开销极小。
 */
void ParamBackupMarkDirty(void)
{
    pthread_mutex_lock(&g_backupMutex);
    g_needBackup = true;
    pthread_cond_signal(&g_backupCond);
    pthread_mutex_unlock(&g_backupMutex);
}

/**
 * @brief 等待备份通知或超时
 * @return true=被 MarkDirty 唤醒，false=超时
 */
static bool WaitNotifyOrTimeout(int32_t timeoutSec)
{
    bool notified = false;
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeoutSec;

    pthread_mutex_lock(&g_backupMutex);

    /* 快速路径：脏标志已置位则无需等待 */
    if (g_needBackup)
    {
        notified = true;
    }
    else
    {
        int ret = pthread_cond_timedwait(&g_backupCond, &g_backupMutex, &ts);
        notified = (ret == 0);
    }

    pthread_mutex_unlock(&g_backupMutex);
    return notified;
}

/**
 * @brief 原子检查并清除脏标志
 * @return true=有待处理的备份请求，false=无
 */
static bool CheckAndClearDirty(void)
{
    bool dirty = false;

    pthread_mutex_lock(&g_backupMutex);
    dirty = g_needBackup;
    g_needBackup = false;
    pthread_mutex_unlock(&g_backupMutex);

    return dirty;
}

/* =======================================================================
 * 备份线程内部子流程
 * ======================================================================= */

/**
 * @brief 处理通知触发的备份
 *
 * 被 MarkDirty 唤醒后：直接热备份，不做完整性检查。
 * 理由：参数刚写入成功，主数据库大概率完好，省去 quick_check 的 IO 开销。
 */
static void HandleNotifiedBackup(void)
{
    static int backUpFailedCount = 0;
    static time_t lastBackupAttempt = 0;
    time_t now = time(NULL);

    if (!param_fileIsExist(PARAM_FILE_PATH))
    {
        LOG_Error("Main DB missing during backup notification!");
        return;
    }

    LOG_Debug("Backup notified, performing hot backup");
    if (!param_dbHotBackup(PARAM_FILE_BACKUP_PATH, PARAM_FILE_PATH))
    {
        /* 备份失败，重新标记脏，下个周期重试 */
        LOG_Error("Hot backup failed (count=%d), will retry next cycle", (backUpFailedCount + 1));

        /* 前 5 次且距离上次尝试已超过 30s 才重试，避免 tight loop */
        if (backUpFailedCount < 5 && llabs((long long)(now - lastBackupAttempt)) >= BACKUP_CHECK_SECOND)
        {
            ParamBackupMarkDirty();
            backUpFailedCount++;
        }
        else if (backUpFailedCount >= 5)
        {
            LOG_Warn("Backup retry limit reached, stop auto-retry until next param write");
        }
    }
    else
    {
        backUpFailedCount = 0;
        lastBackupAttempt = 0;
    }
    lastBackupAttempt = now;
}

/**
 * @brief 处理超时触发的健康检查
 *
 * 30s 无参数更新时执行，检查主库和备份库完整性，发现损坏自动恢复。
 */
static void HandleHealthCheck(void)
{
    bool mainExists = param_fileIsExist(PARAM_FILE_PATH);
    bool bkExists   = param_fileIsExist(PARAM_FILE_BACKUP_PATH);
    bool mainValid  = false;
    bool bkValid    = false;

    if (mainExists)
    {
        mainValid = param_verifyIntegrity(PARAM_FILE_PATH);
    }
    if (bkExists)
    {
        bkValid = param_verifyIntegrity(PARAM_FILE_BACKUP_PATH);
    }

    if (!mainValid && bkValid)
    {
        /* 主库坏、备份好 → 从备份恢复主库 */
        LOG_Warn("Main DB invalid, restoring from backup");
        if (mainExists)
        {
            if (unlink(PARAM_FILE_PATH) != 0)
            {
                LOG_Error("Failed to remove corrupted main DB: errno=%d", errno);
            }
        }
        param_dbHotBackup(PARAM_FILE_PATH, PARAM_FILE_BACKUP_PATH);
    }
    else if (mainValid && !bkValid)
    {
        /* 主库好、备份坏 → 重建备份 */
        LOG_Warn("Backup DB invalid, recreating");
        if (bkExists)
        {
            if (unlink(PARAM_FILE_BACKUP_PATH) != 0)
            {
                LOG_Error("Failed to remove corrupted backup DB: errno=%d", errno);
            }
        }
        param_dbHotBackup(PARAM_FILE_BACKUP_PATH, PARAM_FILE_PATH);
    }
    else if (!mainValid && !bkValid)
    {
        /* 双库都坏/都不存在 */
        LOG_Error("Both databases are %s!", (mainExists || bkExists) ? "corrupted" : "missing");
    }
    else
    {
        /* 双库都健康，检查是否有遗留的未处理脏标志 */
        if (CheckAndClearDirty())
        {
            param_dbHotBackup(PARAM_FILE_BACKUP_PATH, PARAM_FILE_PATH);
        }
    }
}

/* =======================================================================
 * 备份线程入口
 * ======================================================================= */

void *paramBackupCallBack(void *pArg)
{
    (void)pArg;
    checkAndMkdir(PARAM_DICT_BACKUP_PATH);

    while (1)
    {
        /* 阻塞等待：被 MarkDirty 唤醒或 30 秒超时 */
        bool notified = WaitNotifyOrTimeout(BACKUP_CHECK_SECOND);

        /* 持文件锁，防止与业务线程的文件操作竞争 */
        pthread_mutex_lock(&paramfileLock);

        if (notified && CheckAndClearDirty())
        {
            /* 通知触发 → 快速热备份 */
            HandleNotifiedBackup();
        }
        else
        {
            /* 超时触发 → 健康巡检 */
            HandleHealthCheck();
        }

        pthread_mutex_unlock(&paramfileLock);
    }

    return NULL;
}
