#include <stdio.h>
#include <sys/stat.h>

#include "terminal_management_data_packag.h"
#include "terminal_management_data_monitor.h"
#include "terminal_management_data_encode.h"
#include "sr_log.h"

// #include "pbase64.h"

static SendQueueCollection_t g_sendQueue;

#define MAX_PRIORITY_LEVEL 4
static sqlite3 *g_priorityDbs[MAX_PRIORITY_LEVEL] = {NULL}; // 优先级补发数据库句柄
static char g_priorityDbNames[MAX_PRIORITY_LEVEL][512];     // 优先级补发数据库名称
static char g_currentDbPath[512];                           // 补发数据库存储路径

static sqlite3 *g_historyFaultDb = NULL; // 历史故障数据库句柄
static char g_historyFaultDbName[512];   // 历史故障数据库名称
static char g_historyFaultDbPath[512];   // 历史故障数据库路径

static pthread_mutex_t g_dbMutex = PTHREAD_MUTEX_INITIALIZER;

// 待发送队列处理
/********************************************************************************************************************/

/**
 * @brief 创建终端管理平台的发送队列
 *
 * @return int
 */
int InitSendQueue()
{
    int32_t ret = RET_ERR;

    ret = SRThreadQueueCreate(&(g_sendQueue.P0), sizeof(LoadData_t),
                              SR_QUEUE_CALLBACK_QUEUE_DEFAULT_MAX_CAPACITY);
    if (RET_OK != ret)
    {
        printf("P0 Queue create failed %d\n", ret);
        LOG_Error("P0 Queue create failed %d\n", ret);
        return RET_ERR;
    }

    ret = SRThreadQueueCreate(&(g_sendQueue.P1), sizeof(LoadData_t),
                              SR_QUEUE_CALLBACK_QUEUE_DEFAULT_MAX_CAPACITY);
    if (RET_OK != ret)
    {
        printf("P1 Queue create failed %d\n", ret);
        LOG_Error("P1 Queue create failed %d\n", ret);
        return RET_ERR;
    }

    ret = SRThreadQueueCreate(&(g_sendQueue.P2), sizeof(LoadData_t),
                              SR_QUEUE_CALLBACK_QUEUE_DEFAULT_MAX_CAPACITY);
    if (RET_OK != ret)
    {
        printf("P2 Queue create failed %d\n", ret);
        LOG_Error("P2 Queue create failed %d\n", ret);
        return RET_ERR;
    }

    ret = SRThreadQueueCreate(&(g_sendQueue.P3), sizeof(LoadData_t),
                              SR_QUEUE_CALLBACK_QUEUE_DEFAULT_MAX_CAPACITY);
    if (RET_OK != ret)
    {
        printf("P3 Queue create failed %d\n", ret);
        LOG_Error("P3 Queue create failed %d\n", ret);
        return RET_ERR;
    }

    ret = SRThreadQueueCreate(&(g_sendQueue.P4), sizeof(LoadData_t),
                              SR_QUEUE_CALLBACK_QUEUE_DEFAULT_MAX_CAPACITY);
    if (RET_OK != ret)
    {
        printf("P4 Queue create failed %d\n", ret);
        LOG_Error("P4 Queue create failed %d\n", ret);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @brief 销毁终端管理平台的所有发送队列
 */
void DeinitSendQueue(void)
{
    if (g_sendQueue.P0)
    {
        SRThreadQueueDelete(g_sendQueue.P0);
        g_sendQueue.P0 = NULL;
    }
    if (g_sendQueue.P1)
    {
        SRThreadQueueDelete(g_sendQueue.P1);
        g_sendQueue.P1 = NULL;
    }
    if (g_sendQueue.P2)
    {
        SRThreadQueueDelete(g_sendQueue.P2);
        g_sendQueue.P2 = NULL;
    }
    if (g_sendQueue.P3)
    {
        SRThreadQueueDelete(g_sendQueue.P3);
        g_sendQueue.P3 = NULL;
    }
    if (g_sendQueue.P4)
    {
        SRThreadQueueDelete(g_sendQueue.P4);
        g_sendQueue.P4 = NULL;
    }
}

/**
 * @brief 获取待发送队列中暂存的数据条数
 *
 * @param queueIdx 队列优先级
 * @return int
 */
int GetQueueDataCnt(int queueIdx)
{
    SRQueue_t *queue = NULL;
    uint32_t queueCnt = 0;
    switch (queueIdx)
    {
    case 0:
        queue = g_sendQueue.P0;
        break;
    case 1:
        queue = g_sendQueue.P1;
        break;
    case 2:
        queue = g_sendQueue.P2;
        break;
    case 3:
        queue = g_sendQueue.P3;
        break;
    case 4:
        queue = g_sendQueue.P4;
        break;
    default:
        return RET_ERR;
    }
    SRThreadQueueGetCnt(queue, &queueCnt);
    return queueCnt;
}

/**
 * @brief 获取待发送队列中的数据
 *
 * @param loadData buff缓存待发送队列中的数据
 * @param buffLen buff数据长度
 * @param queueIdx 队列优先级
 * @return int
 */
int GetQueueData(LoadData_t *loadData, int buffLen, int queueIdx)
{
    int ret = SR_FAILURE;
    SRQueue_t *queue = NULL;

    if (!loadData || buffLen < sizeof(LoadData_t))
        return RET_ERR;

    switch (queueIdx)
    {
    case 0:
        queue = g_sendQueue.P0;
        break;
    case 1:
        queue = g_sendQueue.P1;
        break;
    case 2:
        queue = g_sendQueue.P2;
        break;
    case 3:
        queue = g_sendQueue.P3;
        break;
    case 4:
        queue = g_sendQueue.P4;
        break;
    default:
        return RET_ERR;
    }

    ret = SRThreadQueueWait(queue, (uint8_t *)loadData, sizeof(LoadData_t), E_SR_NO_WAIT);
    if (ret != SR_SUCCESS)
    {
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @brief 向待发送队列中插入数据
 *
 * @param loadData 待发送的数据
 * @param queueIdx 队列优先级
 * @return int
 */
int AddQueueData(LoadData_t loadData, int queueIdx)
{
    int ret = RET_ERR;
    SRQueue_t *queue = NULL;

    switch (queueIdx)
    {
    case 0:
        queue = g_sendQueue.P0;
        break;
    case 1:
        queue = g_sendQueue.P1;
        break;
    case 2:
        queue = g_sendQueue.P2;
        break;
    case 3:
        queue = g_sendQueue.P3;
        break;
    case 4:
        queue = g_sendQueue.P4;
        break;
    default:
        return RET_ERR;
    }

    ret = SRThreadQueueRelease(queue, sizeof(LoadData_t), (uint8_t *)&loadData, E_SR_NO_WAIT);
    if (ret != SR_SUCCESS)
    {
        LOG_Error("%s() add data to P%d queue failed\n", __func__, queueIdx);
        return RET_ERR;
    }
    return RET_OK;
}

// 辅助函数
/********************************************************************************************************************/

static long GetFileSize(const std::string &filePath)
{
    struct stat st;
    if (stat(filePath.c_str(), &st) != 0)
    {
        return -1;
    }
    return (long)st.st_size;
}

/**
 * @brief 从备份数据库的文件名中解析优先级、时间戳和数据库类型
 * @details 格式1（补发库）: "reissue_backup_P{优先级}_{时间戳}.db"
 *          格式2（历史故障库）: "history_fault_backup_{时间戳}.db"
 *
 * @param filename     备份文件名（不含路径）
 * @param outTimestamp 输出参数，存放解析出的时间戳
 * @param outDbType    输出参数，0-补发库，1-历史故障库，-1-未知
 * @return 成功返回优先级（补发库返回0~3，历史故障库返回0），格式不匹配返回 -1
 */
static int GetBackupFileInfo(const char *filename, time_t *outTimestamp, int *outDbType)
{
    int priority = -1;
    *outDbType = -1;

    // 尝试匹配补发备份文件格式
    if (sscanf(filename, "reissue_backup_P%d_%ld.db", &priority, outTimestamp) == 2)
    {
        if (priority >= 0 && priority < MAX_PRIORITY_LEVEL)
        {
            *outDbType = 0;
            return priority;
        }
    }

    // 尝试匹配历史故障备份文件格式
    if (sscanf(filename, "history_fault_backup_%ld.db", outTimestamp) == 1)
    {
        *outDbType = 1;
        priority = 0;
        return priority;
    }

    return RET_ERR;
}

// 补发数据库处理
/********************************************************************************************************************/

/**
 * @brief 创建新的当前数据库文件
 *
 * @param dbPath
 * @return int
 */
static int CreateNewCurrentDb(const char *dbPath)
{
    sqlite3 *db;
    if (sqlite3_open(dbPath, &db) != SQLITE_OK)
    {
        LOG_Error("%s() Failed to open db:%s\n", __func__, dbPath);
        return RET_ERR;
    }
    const char *sql = "CREATE TABLE IF NOT EXISTS data (id INTEGER PRIMARY KEY AUTOINCREMENT, msg TEXT);";
    char *errMsg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK)
    {
        LOG_Error("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return RET_ERR;
    }
    sqlite3_close(db);
    return RET_OK;
}

/**
 * @brief 获取指定已打开数据库中的记录总数
 *
 * @param db 已打开的数据库句柄（必须非空）
 * @return 记录数，若 db 为空或查询失败返回0
 */
static int GetCurrentRecordCount(sqlite3 *db)
{
    if (!db)
        return 0;
    sqlite3_stmt *stmt;
    int count = 0;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM data", -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            count = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return count;
}

/**
 * @brief 管理备份文件目录，确保补发备份文件总大小不超过500MB，历史故障备份文件不超过100MB
 *
 * @param backupDir 备份文件目录
 */
static void ManageBackupFiles(const char *backupDir)
{
    DIR *dir = opendir(backupDir);
    if (!dir)
        return;

    typedef struct
    {
        char name[512];
        int dbType; // 0-补发，1-历史故障
        time_t timestamp;
        long long size;
    } BackupFile;
    BackupFile files[1024];
    int fileCount = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && fileCount < 1024)
    {
        time_t ts = 0;
        int dbType = -1;
        // 使用新的 GetBackupFileInfo 统一解析文件名
        int prio = GetBackupFileInfo(entry->d_name, &ts, &dbType);
        if (prio >= 0) // 解析成功，dbType 已设置（0 或 1）
        {
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s%s", backupDir, entry->d_name);
            long long size = (long long)GetFileSize(std::string(fullpath));
            if (size > 0)
            {
                strcpy(files[fileCount].name, entry->d_name);
                files[fileCount].dbType = dbType;
                files[fileCount].timestamp = ts;
                files[fileCount].size = size;
                fileCount++;
            }
        }
    }
    closedir(dir);

    // 按类型和时间戳排序（先历史故障，后补发，同类型按时间戳升序）
    for (int i = 0; i < fileCount - 1; i++)
    {
        for (int j = i + 1; j < fileCount; j++)
        {
            int swap = 0;
            if (files[i].dbType > files[j].dbType)
            {
                swap = 1;
            }
            else if (files[i].dbType == files[j].dbType)
            {
                if (files[i].timestamp > files[j].timestamp)
                    swap = 1;
            }
            if (swap)
            {
                BackupFile tmp = files[i];
                files[i] = files[j];
                files[j] = tmp;
            }
        }
    }

    // 分别计算补发库和历史故障库的总大小
    long long reissueTotalSize = 0;
    long long historyTotalSize = 0;
    for (int i = 0; i < fileCount; i++)
    {
        if (files[i].dbType == 0)
            reissueTotalSize += files[i].size;
        else
            historyTotalSize += files[i].size;
    }

    // 清理补发备份文件
    int idx = 0;
    while (reissueTotalSize > MAX_REISSUE_BACKUP_TOTAL_BYTES && idx < fileCount)
    {
        if (files[idx].dbType == 0)
        {
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s%s", backupDir, files[idx].name);
            if (remove(fullpath) == 0)
            {
                reissueTotalSize -= files[idx].size;
                LOG_Warn("Deleted reissue backup: %s (size %lld)\n",
                         files[idx].name, files[idx].size);
            }
        }
        idx++;
    }

    // 清理历史故障备份文件
    idx = 0;
    while (historyTotalSize > MAX_HISTORY_FAULT_BACKUP_TOTAL_BYTES && idx < fileCount)
    {
        if (files[idx].dbType == 1)
        {
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s%s", backupDir, files[idx].name);
            if (remove(fullpath) == 0)
            {
                historyTotalSize -= files[idx].size;
                LOG_Warn("Deleted history backup: %s (size %lld)\n",
                         files[idx].name, files[idx].size);
            }
        }
        idx++;
    }
}

/**
 * @brief 轮转数据库：将当前数据库重命名为备份文件，并新建空数据库
 *
 * @param priority 优先级（补发库：0~3；历史故障库：该参数忽略）
 * @param dbType   数据库类型：0-补发，1-历史故障
 */
static void RotateDatabase(int priority, int dbType)
{
    sqlite3 *db = NULL;
    char *dbPath = NULL;

    if (dbType == 0) // 补发库
    {
        if (priority < 0 || priority >= MAX_PRIORITY_LEVEL)
            return;
        db = g_priorityDbs[priority];
        dbPath = g_priorityDbNames[priority];
    }
    else if (dbType == 1) // 历史故障库
    {
        db = g_historyFaultDb;
        dbPath = g_historyFaultDbPath;
    }
    else
    {
        return;
    }

    if (!db)
        return;

    sqlite3_close(db);
    if (dbType == 0)
        g_priorityDbs[priority] = NULL;
    else
        g_historyFaultDb = NULL;

    // 检查当前数据库记录数
    int count = 0;
    sqlite3 *tmpDb;
    if (sqlite3_open(dbPath, &tmpDb) == SQLITE_OK)
    {
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(tmpDb, "SELECT COUNT(*) FROM data", -1, &stmt, NULL) == SQLITE_OK)
        {
            if (sqlite3_step(stmt) == SQLITE_ROW)
                count = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
        }
        sqlite3_close(tmpDb);
    }

    if (count == 0)
    {
        // 空数据库，无需备份，直接重新打开
        if (dbType == 0)
            sqlite3_open(dbPath, &g_priorityDbs[priority]);
        else
            sqlite3_open(dbPath, &g_historyFaultDb);
        return;
    }

    // 生成备份文件名
    long long timestamp = (long long)time(NULL);
    char backupPath[512];
    if (dbType == 0)
    {
        snprintf(backupPath, sizeof(backupPath), "%sreissue_backup_P%d_%lld.db",
                 g_currentDbPath, priority, timestamp);
    }
    else
    {
        snprintf(backupPath, sizeof(backupPath), "%shistory_faule_backup_%lld.db",
                 g_currentDbPath, timestamp);
    }

    // 重命名当前数据库为备份文件
    if (rename(dbPath, backupPath) != 0)
    {
        LOG_Error("%s() Failed to rename database to backup\n", __func__);
        // 重新打开
        if (dbType == 0)
            sqlite3_open(dbPath, &g_priorityDbs[priority]);
        else
            sqlite3_open(dbPath, &g_historyFaultDb);
        return;
    }

    // 新建空数据库
    CreateNewCurrentDb(dbPath);
    if (dbType == 0)
        sqlite3_open(dbPath, &g_priorityDbs[priority]);
    else
        sqlite3_open(dbPath, &g_historyFaultDb);

    // 清理旧备份文件
    ManageBackupFiles(g_currentDbPath);
}

/**
 * @brief 从指定数据库文件中获取并删除id最小的一条记录
 * @param dbPath   数据库文件路径
 * @param outData  输出参数，存放获取到的数据
 * @return 成功返回1,无数据返回0,出错返回-1
 */
static int FetchOneRecordFromDb(const char *dbPath, LoadData_t *outData)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ret = -1;
    int rc;
    int cleanup_done = 0;

    do
    {
        // 1. 打开数据库
        rc = sqlite3_open(dbPath, &db);
        if (rc != SQLITE_OK)
        {
            LOG_Error("%s: sqlite3_open failed: %s", __func__, sqlite3_errmsg(db));
            ret = -1;
            break;
        }

        // 2. 准备查询语句
        const char *selectSql = "SELECT id, msg FROM data ORDER BY id LIMIT 1";
        rc = sqlite3_prepare_v2(db, selectSql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            LOG_Error("%s: sqlite3_prepare_v2 failed: %s", __func__, sqlite3_errmsg(db));
            ret = -1;
            break;
        }

        // 3. 执行查询
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW)
        {
            ret = (rc == SQLITE_DONE) ? 0 : -1;
            if (rc != SQLITE_DONE)
            {
                LOG_Error("%s: sqlite3_step failed: %s", __func__, sqlite3_errmsg(db));
            }
            break;
        }

        // 4. 读取数据
        int id = sqlite3_column_int(stmt, 0);
        const void *blob = sqlite3_column_blob(stmt, 1);
        int blob_size = sqlite3_column_bytes(stmt, 1);

        if (!blob || blob_size <= 0)
        {
            ret = 0;
            break;
        }

        if (blob_size > (int)sizeof(outData->data))
        {
            LOG_Error("%s: BLOB size %d exceeds buffer %zu", __func__, blob_size, sizeof(outData->data));
            ret = -1;
            break;
        }

        // 5. 拷贝数据
        memcpy(outData->data, blob, blob_size);
        outData->len = blob_size;

        // 6. 释放查询语句后再执行 DELETE
        sqlite3_finalize(stmt);
        stmt = NULL;

        char deleteSql[128];
        snprintf(deleteSql, sizeof(deleteSql), "DELETE FROM data WHERE id = %d", id);
        char *errMsg = NULL;
        rc = sqlite3_exec(db, deleteSql, NULL, NULL, &errMsg);
        if (rc != SQLITE_OK)
        {
            LOG_Error("%s: DELETE failed: %s", __func__, errMsg ? errMsg : "unknown");
            sqlite3_free(errMsg);
            ret = -1;
            break;
        }
        ret = 1; // 成功读取并删除

    } while (0);

    if (!cleanup_done)
    {
        if (stmt)
        {
            sqlite3_finalize(stmt);
        }
        if (db)
        {
            sqlite3_close(db);
        }
        cleanup_done = 1;
    }

    return ret;
}

/**
 * @brief 向指定数据库添加一条数据
 *
 * @param loadData 待缓存的数据
 * @param type     优先级 0:P0故障 1:P1异常 2:P2正常 3:P3周期（仅在 dbType=0 时生效）
 * @param dbType   数据库类型：0-补发数据库，1-历史故障数据库
 */
void CacheAdd(LoadData_t loadData, int type, int dbType)
{
    sqlite3 *db = NULL;

    if (dbType == 0)
    {
        // 补发数据库：
        if (type < 0 || type >= MAX_PRIORITY_LEVEL)
            return;
        pthread_mutex_lock(&g_dbMutex);
        db = g_priorityDbs[type];
    }
    else if (dbType == 1)
    {
        // 历史故障数据库：
        if (type != 0 && type != 3)
            return;
        pthread_mutex_lock(&g_dbMutex);
        db = g_historyFaultDb;
    }
    else
    {
        return;
    }

    if (!db)
    {
        pthread_mutex_unlock(&g_dbMutex);
        return;
    }

    // 检查记录数上限，触发轮转
    int count = GetCurrentRecordCount(db);
    if (count >= MAX_RECORDS_PER_REISSUE_DB)
    {
        // 根据 dbType 调用轮转
        if (dbType == 0)
        {
            RotateDatabase(type, 0);
            db = g_priorityDbs[type];
        }
        else
        {
            RotateDatabase(0, 1); // priority 参数无效
            db = g_historyFaultDb;
        }

        if (!db)
        {
            pthread_mutex_unlock(&g_dbMutex);
            return;
        }
    }

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO data (msg) VALUES (?)";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_blob(stmt, 1, loadData.data, loadData.len, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            LOG_Error("%s() Failed to insert data into %s DB\n",
                      __func__, (dbType == 0) ? "reissue" : "history fault");
        }
        sqlite3_finalize(stmt);
    }
    pthread_mutex_unlock(&g_dbMutex);
}

/**
 * @brief 从时间戳最早的数据库（包含当前库及所有备份库）中取一条补发数据
 * @details 按优先级 P0 > P1 > P2 > P3 遍历所有数据库文件，优先从高优先级、最早的文件中获取数据。
 * @param loadData
 * @return RET_OK: 数据库无数据或获取到实际数据(需自行根据入参进行判断)，RET_ERR: 操作出错
 */
int CacheGet(LoadData_t *loadData)
{
    if (!loadData)
        return RET_ERR;

    pthread_mutex_lock(&g_dbMutex);

    for (int prio = 0; prio < MAX_PRIORITY_LEVEL; prio++)
    {
        typedef struct
        {
            char path[512];
            time_t dbTimestamp;
        } DbFileEntry;
        DbFileEntry dbFiles[1024];
        int fileCount = 0;

        // 当前补发数据库文件
        if (access(g_priorityDbNames[prio], F_OK) == 0)
        {
            strcpy(dbFiles[fileCount].path, g_priorityDbNames[prio]);
            dbFiles[fileCount].dbTimestamp = 0;
            fileCount++;
        }

        // 扫描备份目录，收集属于当前优先级的补发备份文件（排除历史故障备份）
        DIR *dir = opendir(g_currentDbPath);
        if (dir)
        {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL && fileCount < 1024)
            {
                time_t ts = 0;
                int dbType = -1;
                int fprio = GetBackupFileInfo(entry->d_name, &ts, &dbType);
                // 只收集补发备份（dbType == 0）且优先级匹配的文件
                if (fprio >= 0 && dbType == 0 && fprio == prio)
                {
                    snprintf(dbFiles[fileCount].path, sizeof(dbFiles[fileCount].path),
                             "%s%s", g_currentDbPath, entry->d_name);
                    dbFiles[fileCount].dbTimestamp = ts;
                    fileCount++;
                }
            }
            closedir(dir);
        }

        // 按时间戳升序排序
        for (int i = 0; i < fileCount - 1; i++)
        {
            for (int j = i + 1; j < fileCount; j++)
            {
                if (dbFiles[i].dbTimestamp > dbFiles[j].dbTimestamp)
                {
                    DbFileEntry tmp = dbFiles[i];
                    dbFiles[i] = dbFiles[j];
                    dbFiles[j] = tmp;
                }
            }
        }

        int got = 0;
        for (int i = 0; i < fileCount; i++)
        {
            got = FetchOneRecordFromDb(dbFiles[i].path, loadData);
            if (got == 1)
            {
                pthread_mutex_unlock(&g_dbMutex);
                return RET_OK;
            }
            else if (got == 0)
            {
                // 该文件无数据，若是备份文件则删除
                if (strstr(dbFiles[i].path, "reissue_backup_") != NULL)
                {
                    remove(dbFiles[i].path);
                }
            }
            else
            {
                // 出错，记录日志但继续尝试其他文件
                LOG_Error("%s() error reading db: %s\n", __func__, dbFiles[i].path);
            }
        }
    }

    pthread_mutex_unlock(&g_dbMutex);
    // 所有优先级均无数据
    return RET_OK;
}

/**
 * @brief 初始化所有数据库（补发数据库 + 历史故障数据库）
 *
 * @param dbFolder 数据库存放目录
 */
void InitAllDatabases(const char *dbFolder)
{
    snprintf(g_currentDbPath, sizeof(g_currentDbPath), "%s", dbFolder);

    /* ---------- 补发数据库 P0 ~ P3 ---------- */
    for (int prio = 0; prio < MAX_PRIORITY_LEVEL; prio++)
    {
        char baseName[64];
        snprintf(baseName, sizeof(baseName), "reissue_current_P%d.db", prio);
        snprintf(g_priorityDbNames[prio], sizeof(g_priorityDbNames[prio]),
                 "%s%s", dbFolder, baseName);

        if (access(g_priorityDbNames[prio], F_OK) != 0)
        {
            CreateNewCurrentDb(g_priorityDbNames[prio]);
        }
        if (sqlite3_open(g_priorityDbNames[prio], &g_priorityDbs[prio]) != SQLITE_OK)
        {
            LOG_Error("%s() Failed to open priority P%d db\n", __func__, prio);
            g_priorityDbs[prio] = NULL;
            continue;
        }
        int count = GetCurrentRecordCount(g_priorityDbs[prio]);
        if (count >= MAX_RECORDS_PER_REISSUE_DB)
        {
            RotateDatabase(prio, 0);
        }
    }

    /* ---------- 历史故障数据库 ---------- */
    snprintf(g_historyFaultDbPath, sizeof(g_historyFaultDbPath),
             "%shistory_fault.db", dbFolder);

    if (access(g_historyFaultDbPath, F_OK) != 0)
    {
        CreateNewCurrentDb(g_historyFaultDbPath);
    }
    if (sqlite3_open(g_historyFaultDbPath, &g_historyFaultDb) != SQLITE_OK)
    {
        LOG_Error("%s() Failed to open history fault db\n", __func__);
        g_historyFaultDb = NULL;
    }
    else
    {
        int count = GetCurrentRecordCount(g_historyFaultDb);
        if (count >= MAX_RECORDS_PER_REISSUE_DB)
        {
            RotateDatabase(0, 1);
        }
    }
}

/**
 * @brief 关闭所有数据库（补发 + 历史故障）
 */
void CloseAllDatabases(void)
{
    for (int prio = 0; prio < MAX_PRIORITY_LEVEL; prio++)
    {
        if (g_priorityDbs[prio])
        {
            sqlite3_close(g_priorityDbs[prio]);
            g_priorityDbs[prio] = NULL;
        }
    }
    if (g_historyFaultDb)
    {
        sqlite3_close(g_historyFaultDb);
        g_historyFaultDb = NULL;
    }
    return;
}

// 组包
/********************************************************************************************************************/

/**
 * @brief 终端管理平台周期数据组包
 *
 * @param data
 */
void PeriodicPack(const MonitoringData_t rawData)
{
    LOG_Debug("----handle <TimingPack>");

    // 1. 构建 FullReport 消息
    net::sirun::tboxs3::report::FullReport report;

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(report.mutable_static_info(), rawData.tboxDbInfo);
    EncodeWorkStatus(report.mutable_work(), rawData.mcuStatusData);
    EncodeExternalConnect(report.mutable_external_connect(), rawData);
    EncodeInnerHealth(report.mutable_inner_health(), rawData);
    EncodeLogReporter(report.mutable_logreporter(), rawData);
    report.set_gathertime(::time(nullptr)); // 数据采集时间

    // 3. 序列化为二进制
    std::string binary;
    if (!report.SerializeToString(&binary))
    {
        LOG_Error("FullReport SerializeToString failed");
        return;
    }

    // 4. 声明 LoadData_t 并检查长度是否超限
    LoadData_t loadData = {0};
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Binary data too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }

    // 5. 填充 LoadData_t
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 3;

    // 6. 加入 P3 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 7. 故障状态下，缓存历史故障数据库
    if (IsInFaultState())
    {
        CacheAdd(loadData, loadData.priority, 1);
    }
}

/**
 * @brief 1、TBOX备用电池低压触发事件组包
 *
 */
void BatteryLowVoltageTriggerEventPack(const MonitoringData_t rawData)
{
    LOG_Debug("----handle <Trigger> BatteryLowVoltage");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::TBOX_LOW_BATTERY_VOLTAGE_BELOW_CRITICAL_VALUE);
    event->set_state(net::sirun::tboxs3::report::EventState::START);
    event->set_event_time(::time(nullptr));

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 1;

    // 5. 加入 P1 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 缓存异常事件磁盘标志
    SetStatusFlagToDisk(1, NULL);
}

/**
 * @brief 1、TBOX备用电池低压恢复事件组包
 *
 */
void BatteryLowVoltageRecoveryEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Recovery> BatteryLowVoltage\n");
    LOG_Debug("----handle <Recovery> BatteryLowVoltage");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::TBOX_LOW_BATTERY_VOLTAGE_BELOW_CRITICAL_VALUE);
    event->set_state(net::sirun::tboxs3::report::EventState::end);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 1;

    // 5. 加入 P1 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 清除异常事件磁盘标志
    ResetStatusFlagToDisk(1);
    return;
}

/**
 * @brief 2、GPS信号丢失触发事件组包
 *
 */
void GpsSignalLostTriggerEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Trigger> GpsSignalLost\n");
    LOG_Debug("----handle <Trigger> GpsSignalLost");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::GPS_SIGNAL_INTERRUPTION);
    event->set_state(net::sirun::tboxs3::report::EventState::START);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 1;

    // 5. 加入 P1 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 缓存异常事件磁盘标志
    SetStatusFlagToDisk(2, NULL);
    return;
}

/**
 * @brief 2、GPS信号丢失恢复事件组包
 *
 */
void GpsSignalLostRecoveryEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Recovery> GpsSignalLost\n");
    LOG_Debug("----handle <Recovery> GpsSignalLost");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::GPS_SIGNAL_INTERRUPTION);
    event->set_state(net::sirun::tboxs3::report::EventState::end);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 1;

    // 5. 加入 P1 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 清除异常事件磁盘标志
    ResetStatusFlagToDisk(2);
    return;
}

/**
 * @brief 3、CAN总线通讯中断触发事件组包
 *
 */
void CanBusCommInterruptTriggerEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Trigger> CanBusCommInterrupt\n");
    LOG_Debug("----handle <Trigger> CanBusCommInterrupt");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::CAN_COMMUNICATION_INTERRUPTION);
    event->set_state(net::sirun::tboxs3::report::EventState::START);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeInnerHealth(event->mutable_health(), rawData);

    // 2. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 3. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 1;

    // 4. 加入 P1 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 缓存异常事件磁盘标志
    SetStatusFlagToDisk(3, NULL);
    return;
}

/**
 * @brief 3、CAN总线通讯中断恢复事件组包
 *
 */
void CanBusCommInterruptRecoveryEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Recovery> CanBusCommInterrupt\n");
    LOG_Debug("----handle <Recovery> CanBusCommInterrupt");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::CAN_COMMUNICATION_INTERRUPTION);
    event->set_state(net::sirun::tboxs3::report::EventState::end);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeInnerHealth(event->mutable_health(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 1;

    // 5. 加入 P1 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 清除异常事件磁盘标志
    ResetStatusFlagToDisk(3);
    return;
}

/**
 * @brief 4、车辆意外高压下电触发事件组包
 *
 */
void VehicleUnexpectedHvPowerDownTriggerEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Trigger> VehicleUnexpectedHvPowerDown\n");
    LOG_Debug("----handle <Trigger> VehicleUnexpectedHvPowerDown");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::VEHICLE_ACCIDENTALLY_DEENERGIZED_UNDER_HIGH_VOLTAGE);
    event->set_state(net::sirun::tboxs3::report::EventState::START);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 1;

    // 5. 加入 P1 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 缓存异常事件磁盘标志
    SetStatusFlagToDisk(4, NULL);
    return;
}

/**
 * @brief 4、车辆意外高压下电恢复事件组包
 *
 */
void VehicleUnexpectedHvPowerDownRecoveryEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Recovery> VehicleUnexpectedHvPowerDown\n");
    LOG_Debug("----handle <Recovery> VehicleUnexpectedHvPowerDown");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::VEHICLE_ACCIDENTALLY_DEENERGIZED_UNDER_HIGH_VOLTAGE);
    event->set_state(net::sirun::tboxs3::report::EventState::end);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 1;

    // 5. 加入 P1 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 清除异常事件磁盘标志
    ResetStatusFlagToDisk(4);
    return;
}

/**
 * @brief 7、车辆无法与CAN通信触发事件组包
 *
 */
void VehicleCanCommFailTriggerEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Trigger> VehicleCanCommFail\n");
    LOG_Debug("----handle <Trigger> VehicleCanCommFail");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::CANNOT_COMMUNICATE_WITH_CAN);
    event->set_state(net::sirun::tboxs3::report::EventState::START);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);
    EncodeInnerHealth(event->mutable_health(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 0;

    // 5. 加入 P0 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 缓存历史故障数据库和磁盘标志
    CacheAdd(loadData, loadData.priority, 1);
    SetStatusFlagToDisk(7, NULL);
    return;
}

/**
 * @brief 7、车辆无法与CAN通信恢复事件组包
 *
 */
void VehicleCanCommFailRecoveryEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Recovery> VehicleCanCommFail\n");
    LOG_Debug("----handle <Recovery> VehicleCanCommFail");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::CANNOT_COMMUNICATE_WITH_CAN);
    event->set_state(net::sirun::tboxs3::report::EventState::end);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);
    EncodeInnerHealth(event->mutable_health(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 0;

    // 5. 加入 P0 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 缓存历史故障数据库，清除故障事件磁盘标志
    CacheAdd(loadData, loadData.priority, 1);
    ResetStatusFlagToDisk(7);
    return;
}

/**
 * @brief 8、GPS模块故障触发事件组包
 *
 */
void GpsModuleFaultTriggerEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Trigger> GpsModuleFault\n");
    LOG_Debug("----handle <Trigger> GpsModuleFault");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::GPS_MODULE_FAULT);
    event->set_state(net::sirun::tboxs3::report::EventState::START);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 0;

    // 5. 加入 P0 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 缓存历史故障数据库和磁盘标志
    CacheAdd(loadData, loadData.priority, 1);
    SetStatusFlagToDisk(8, NULL);
    return;
}

/**
 * @brief 8、GPS模块故障恢复事件组包
 *
 */
void GpsModuleFaultRecoveryEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Recovery> GpsModuleFault\n");
    LOG_Debug("----handle <Recovery> GpsModuleFault");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::GPS_MODULE_FAULT);
    event->set_state(net::sirun::tboxs3::report::EventState::end);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 0;

    // 5. 加入 P0 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 缓存历史故障数据库，清除故障事件磁盘标志
    CacheAdd(loadData, loadData.priority, 1);
    ResetStatusFlagToDisk(8);
    return;
}

/**
 * @brief 9、小电池无法充电触发事件组包
 *
 */
void BatteryChargeFailTriggerEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Trigger> BatteryChargeFail\n");
    LOG_Debug("----handle <Trigger> BatteryChargeFail");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::SMALL_BATTERY_CANNOT_CHARGED);
    event->set_state(net::sirun::tboxs3::report::EventState::START);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 1;

    // 5. 加入 P1 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 缓存历史故障数据库和磁盘标志
    CacheAdd(loadData, loadData.priority, 1);
    SetStatusFlagToDisk(9, NULL);
    return;
}

/**
 * @brief 9、小电池无法充电恢复事件组包
 *
 */
void BatteryChargeFailRecoveryEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Recovery> BatteryChargeFail\n");
    LOG_Debug("----handle <Recovery> BatteryChargeFail");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::SMALL_BATTERY_CANNOT_CHARGED);
    event->set_state(net::sirun::tboxs3::report::EventState::end);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 1;

    // 5. 加入 P1 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);

    // 6. 缓存历史故障数据库，清除故障事件磁盘标志
    CacheAdd(loadData, loadData.priority, 1);
    ResetStatusFlagToDisk(9);
    return;
}

/**
 * @brief 10、车辆启动状态触发事件组包
 *
 */
void VehicleStartTriggerEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Trigger> VehicleStart\n");
    LOG_Debug("----handle <Trigger> VehicleStart");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::VEHICLE_START);
    event->set_state(net::sirun::tboxs3::report::EventState::START);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);
    EncodeInnerHealth(event->mutable_health(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 2;

    // 5. 加入 P2 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);
    return;
}

/**
 * @brief 11、车辆速度大于0触发事件组包
 *
 */
void VehicleSpeedGreater0TriggerEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Trigger> VehicleSpeedGreater0\n");
    LOG_Debug("----handle <Trigger> VehicleSpeedGreater0");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::VEHICLE_DRIVING);
    event->set_state(net::sirun::tboxs3::report::EventState::START);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 2;

    // 5. 加入 P2 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);
    return;
}

/**
 * @brief 12、车辆速度大于100触发事件组包
 *
 */
void VehicleSpeedGreater100TriggerEventPack(const MonitoringData_t rawData)
{
    // printf("----handle <Trigger> VehicleSpeedGreater100\n");
    LOG_Debug("----handle <Trigger> VehicleSpeedGreater100");

    LoadData_t loadData = {0};

    // 1. 构建 EventReport 消息
    net::sirun::tboxs3::report::EventReport event_report;
    net::sirun::tboxs3::report::Event *event = event_report.add_events();
    event->set_type(net::sirun::tboxs3::report::EventType::VEHICLE_DRIVING_AT_HIGH_SPEED);
    event->set_state(net::sirun::tboxs3::report::EventState::START);
    event->set_event_time(::time(nullptr));
    // static_info、external_connect 等字段待填充

    // 2. 调用编码函数填充各子消息
    EncodeStaticInfo(event->mutable_static_info(), rawData.tboxDbInfo);
    EncodeExternalConnect(event->mutable_external_connect(), rawData);

    // 3. 序列化为二进制
    std::string binary;
    if (!event_report.SerializeToString(&binary))
    {
        LOG_Error("EventReport SerializeToString failed");
        return;
    }

    // 4. 检查长度并填充 LoadData_t
    if (binary.size() > sizeof(loadData.data))
    {
        LOG_Error("Event binary too large: %zu bytes (max %zu)", binary.size(), sizeof(loadData.data));
        return;
    }
    memcpy(loadData.data, binary.data(), binary.size());
    loadData.len = binary.size();
    loadData.priority = 2;

    // 5. 加入 P2 队列
    AddQueueData(loadData, loadData.priority);
    printf("P%d queue add protobuf data, data_len=%zu\n", loadData.priority, loadData.len);
    return;
}
