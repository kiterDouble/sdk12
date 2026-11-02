#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <glob.h>
#include <glib.h>

#include "pm_server_api.h"
#include "err_code.h"
#include "sr_param.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "log_pack.h"
#include "log_pack_util.h"
#include "log_pack_compress_and_crypto.h"
#include "log_pm.h"


/**
 * @brief 获取车辆 VIN 码
 *
 * @param vinBuf     输出缓冲区
 * @param vinBufSize 缓冲区大小（至少 TBOX_VIN_SIZE + 1）
 * @return RET_OK 成功，RET_ERR 失败
 */
static int32_t LogPackGetVin(uint8_t* vinBuf, uint32_t vinBufSize)
{
    ParamInfo_t paramInfo = {0};

    if (vinBuf == NULL || vinBufSize < TBOX_VIN_SIZE + 1) {
        LOG_Error("[%s] invalid param", __func__);
        return RET_ERR;
    }

    memset(vinBuf, 0, vinBufSize);
    paramInfo.id = E_ParamId_VIN;
    if (GetParam(&paramInfo) == RET_OK) {
        memcpy(vinBuf, paramInfo.value, TBOX_VIN_SIZE);
        vinBuf[TBOX_VIN_SIZE] = '\0';
    } else {
        LOG_Warn("[%s] GetParam VIN failed, use default VIN", __func__);
        memset(vinBuf, '0', TBOX_VIN_SIZE);
        vinBuf[TBOX_VIN_SIZE] = '\0';
    }

    return RET_OK;
}

/**
 * 根据序号构造 glob 模式，删除最旧的打包文件，直到总大小低于限制。
 *
 * 说明：
 * 1. 文件名格式为 name-index-vin-datetime.log.tgz，index 是单调递增的序号。
 * 2. 使用 glob(name-<oldestIndex>-*.log.tgz) 时，可能同时匹配到 index=1、10、100 等
 *    （因为 glob 没有词边界），所以必须用 sscanf 精确校验 filename 中的 index。
 * 3. 如果 oldestIndex 对应的文件不存在（空洞/已被外部删除），则递增 oldestIndex 继续查找，
 *    不会误删其他序号的文件。
 * 4. 循环结束条件：totalSize <= maxStoreSize，或 oldestIndex 已超过 maxIndex。
 *
 * 返回：本次删除的文件数；出错返回 -1。
 */
static int32_t LogPackCleanupOldest(LogSrPackTask_t* task)
{
    char pattern[LOG_FULLPATH_MAX_SIZE] = {0};
    char name[32] = {0};
    char vin[20] = {0};
    char dateTimeStr[20] = {0};
    glob_t g;
    int globRet = 0;
    uint32_t deleted = 0;
    uint64_t index = 0;
    uint32_t matched = 0;
    uint64_t size = 0;
    size_t k = 0;

    if (task == NULL || task->logPackPath[0] == '\0') {
        return -1;
    }

    /* maxStoreSize 为 0 视为不限制，直接返回 */
    if (task->maxStoreSize == 0) {
        return 0;
    }

    LOG_Warn("[%s] task[%s] cleanup start: totalSize:%llu maxStoreSize:%llu oldestIndex:%" PRIu64 " maxIndex:%" PRIu64,
             __func__, task->name,
             (unsigned long long)task->idx.totalSize,
             (unsigned long long)task->maxStoreSize,
             task->idx.oldestIndex, task->idx.maxIndex);

    /* 持续删除最旧 index 的所有文件，直到总大小低于上限 */
    while (task->idx.totalSize > task->maxStoreSize) {
        /* 按 oldestIndex 构造 glob 模式，注意 glob 会匹配前缀相同的更大序号 */
        /* glob 模式用 *.log*.tgz，同时匹配 .log.tgz 和 .log.crypto.tgz */
        snprintf(pattern, sizeof(pattern), "%s/%s-%" PRIu64 "-*.log*.tgz",
                 task->logPackPath, task->name, task->idx.oldestIndex);

        memset(&g, 0, sizeof(g));
        globRet = glob(pattern, 0, NULL, &g);
        if (globRet != 0) {
            if (globRet == GLOB_NOMATCH) {
                LOG_Warn("[%s] task[%s] no file for index:%" PRIu64 " pattern:%s",
                         __func__, task->name, task->idx.oldestIndex, pattern);
            } else {
                LOG_Error("[%s] task[%s] glob failed:%d pattern:%s",
                          __func__, task->name, globRet, pattern);
            }
            globfree(&g);
            task->idx.oldestIndex++;
            if (task->idx.oldestIndex > task->idx.maxIndex) {
                break;
            }
            continue;
        }

        if (g.gl_pathc == 0) {
            globfree(&g);
            task->idx.oldestIndex++;
            if (task->idx.oldestIndex > task->idx.maxIndex) {
                break;
            }
            continue;
        }

        /* 删除 glob 结果中所有精确匹配本 task 且 index == oldestIndex 的文件 */
        matched = 0;
        for (k = 0; k < g.gl_pathc; k++) {
            const char* baseName = strrchr(g.gl_pathv[k], '/');
            if (baseName != NULL) {
                baseName++;
            } else {
                baseName = g.gl_pathv[k];
            }

            memset(name, 0, sizeof(name));
            memset(vin, 0, sizeof(vin));
            memset(dateTimeStr, 0, sizeof(dateTimeStr));
            index = 0;

            if (sscanf(baseName, LOG_FILENAME_PARSE_FORMAT_TGZ, name, &index, vin, dateTimeStr) != 4 ||
                strncmp(name, task->name, sizeof(task->name)) != 0 || index != task->idx.oldestIndex) {
                continue;
            }

            if (access(g.gl_pathv[k], F_OK) != 0) {
                continue;
            }

            size = GetFileSize(g.gl_pathv[k]);
            if (remove(g.gl_pathv[k]) != 0) {
                LOG_Warn("[%s] task[%s] remove file failed:%s errno:%d",
                         __func__, task->name, g.gl_pathv[k], errno);
                continue;
            }
            task->idx.totalSize -= (size <= task->idx.totalSize) ? size : task->idx.totalSize;
            if (task->idx.fileCount > 0) {
                task->idx.fileCount--;
            }
            deleted++;
            matched++;
            LOG_Warn("[%s] task[%s] auto remove file:%s totalSize:%llu fileCount:%u",
                     __func__, task->name, g.gl_pathv[k],
                     (unsigned long long)task->idx.totalSize, task->idx.fileCount);
        }

        if (matched == 0) {
            LOG_Warn("[%s] task[%s] no exact match for index:%" PRIu64 " in glob result",
                     __func__, task->name, task->idx.oldestIndex);
        }

        globfree(&g);
        /* 无论当前 index 是否删除成功，都向前推进 oldestIndex，避免无限循环 */
        task->idx.oldestIndex++;
        if (task->idx.oldestIndex > task->idx.maxIndex) {
            break;
        }
    }

    if (task->idx.totalSize > task->maxStoreSize) {
        LOG_Error("[%s] task[%s] cleanup exit but totalSize:%llu still > maxStoreSize:%llu",
                  __func__, task->name,
                  (unsigned long long)task->idx.totalSize,
                  (unsigned long long)task->maxStoreSize);
    }

    return (int32_t)deleted;
}

/**
 * 启动时（或唤醒后）全量扫描一次打包目录，重建轻量索引。
 *
 * 说明：
 * 1. 不记录每个文件的完整信息，只累计 totalSize / maxIndex / oldestIndex / fileCount。
 * 2. 扫描时只识别属于本 task 的 .tgz 文件（通过 filename 中的 name 字段过滤），
 *    因此多个 task 共用一个进程时也不会互相干扰。
 * 3. 即使 index 不连贯（存在空洞），也能正确找到实际的最小/最大序号。
 * 4. 扫描完成后，如果 totalSize 超过 maxStoreSize，会调用 LogPackCleanupOldest()
 *    从最旧文件开始清理。
 * 5. 运行期间若发生外部删文件等导致 idx 漂移，会在下一次唤醒后重新 reconcile。
 *
 * 返回：扫描到的文件数；出错返回 -1。
 */
static int32_t LogPackRebuildIdx(LogSrPackTask_t* task)
{
    struct dirent *entry = NULL;
    DIR *dir = NULL;
    uint64_t index = 0;
    char name[32] = {0};
    char vin[20] = {0};
    char dateTimeStr[20] = {0};
    char fileAbsolutePath[LOG_FULLPATH_MAX_SIZE] = {0};

    if (task == NULL || task->logPackPath[0] == '\0') {
        return -1;
    }

    /* 重置本 task 的轻量索引，准备重新统计 */
    task->idx.maxIndex = 0;
    task->idx.oldestIndex = UINT64_MAX;
    task->idx.totalSize = 0;
    task->idx.fileCount = 0;

    /* 打开打包目录，不存在则报错 */
    dir = opendir(task->logPackPath);
    if (dir == NULL) {
        LOG_Error("opendir %s failed", task->logPackPath);
        return -1;
    }

    /* 遍历目录，只统计属于本 task 的 .tgz 文件 */
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        memset(name, 0, sizeof(name));
        memset(vin, 0, sizeof(vin));
        memset(dateTimeStr, 0, sizeof(dateTimeStr));
        index = 0;

        /* 按 name-index-vin-datetime.log.tgz 格式解析文件名 */
        if (sscanf(entry->d_name, LOG_FILENAME_PARSE_FORMAT_TGZ,
                   name, &index, vin, dateTimeStr) != 4) {
            continue;
        }

        /* 过滤不属于本 task 的文件，避免多个 task 互相干扰 */
        if (strncmp(name, task->name, sizeof(task->name)) != 0) {
            continue;
        }

        /* 累加文件大小和数量，并记录最小/最大序号 */
        snprintf(fileAbsolutePath, sizeof(fileAbsolutePath), "%s/%s",
                 task->logPackPath, entry->d_name);
        task->idx.totalSize += GetFileSize(fileAbsolutePath);
        task->idx.fileCount++;

        if (index > task->idx.maxIndex) {
            task->idx.maxIndex = index;
        }
        if (index < task->idx.oldestIndex) {
            task->idx.oldestIndex = index;
        }
    }
    closedir(dir);

    /* 目录为空时，把 oldestIndex 置为 0，避免后续清理逻辑异常 */
    if (task->idx.fileCount == 0) {
        task->idx.oldestIndex = 0;
    }

    /* 把当前最大序号作为下一次打包的起始序号 */
    task->index = task->idx.maxIndex;

    LOG_Warn("[%s] task[%s] rebuild idx: maxIndex:%" PRIu64 " oldestIndex:%" PRIu64 " totalSize:%llu fileCount:%u",
             __func__, task->name,
             task->idx.maxIndex, task->idx.oldestIndex,
             (unsigned long long)task->idx.totalSize, task->idx.fileCount);

    if (task->idx.totalSize > task->maxStoreSize) {
        LogPackCleanupOldest(task);
    }

    return (int32_t)task->idx.fileCount;
}

/** 
 * 压缩和加密日志文件
 * **/
static int CompressAndCryptoLogFile(LogSrCtx_t* ctx, LogSrPackTask_t* task,
                                     const uint8_t* vin,
                                     char *srcLogFilePath,
                                     char latestTgzFileName[LOG_FILENAME_MAX_SIZE])
{
    /** 日志保存在压缩包中文件名 **/
    char compressedLogTxtFileName[LOG_FILENAME_MAX_SIZE] = {0};
    /** 日志压缩包文件名 **/
    char compressedLogTgzFileName[LOG_FILENAME_MAX_SIZE] = {0};
    /** 日志压缩包完整路径 **/
    char compressedLogTgzFilePath[LOG_FULLPATH_MAX_SIZE] = {0};

    uint8_t dateTimeStr[20] = {0};
    Crypto_interface_t* cryptoInterface = NULL;
    int ret = 0;

    if (ctx == NULL || task == NULL || vin == NULL) {
        LOG_Error("invalid context");
        return -1;
    }

    /* cryptoFlag 开启但全局加密接口未绑定（配置缺失/不匹配）时，降级为仅压缩 */
    if (task->cryptoFlag != 0 && ctx->cryptoInterface != NULL) {
        cryptoInterface = ctx->cryptoInterface;
    } else if (task->cryptoFlag != 0 && ctx->cryptoInterface == NULL) {
        LOG_Warn("[%s] task[%s] cryptoFlag enabled but no crypto interface available, fallback to plain tgz", __func__, task->name);
    }

    GetUtcDateTimeStr(dateTimeStr, sizeof(dateTimeStr));

    sprintf(compressedLogTxtFileName, LOG_FILENAME_FORMAT_TXT,
            task->name, task->index + 1, vin, dateTimeStr);
    sprintf(compressedLogTgzFileName,
            (cryptoInterface != NULL) ? LOG_FILENAME_FORMAT_CRYPTO_TGZ : LOG_FILENAME_FORMAT_TGZ,
            task->name, task->index + 1, vin, dateTimeStr);
    snprintf(compressedLogTgzFilePath, sizeof(compressedLogTgzFilePath), "%s/%s",
             task->logPackPath, compressedLogTgzFileName);

    ret = CompressAndEncryptFile(cryptoInterface,
                                 srcLogFilePath,
                                 compressedLogTxtFileName, compressedLogTgzFilePath,
                                 task->cryptoPassword);

    if (ret == 0 && latestTgzFileName != NULL) {
        snprintf(latestTgzFileName, LOG_FILENAME_MAX_SIZE, "%s", compressedLogTgzFileName);
    }

    return ret;
}

static int32_t LogPackTaskInit(LogSrPackTask_t* task)
{
    if (task == NULL) {
        return -1;
    }

    CheckAndCreateDirectory(task->logPath);
    CheckAndCreateDirectory(task->logPackPath);

    LogPackRebuildIdx(task);

    LOG_Warn("[%s] task[%s] index:%" PRIu64, __func__, task->name, task->index);

    return 0;
}

static int32_t LogPackTaskProcess(LogSrCtx_t* ctx, LogSrPackTask_t* task,
                                   char latestTgzFileName[LOG_FILENAME_MAX_SIZE])
{
    char rotatedLogFilePath[LOG_FULLPATH_MAX_SIZE] = {0};
    uint8_t vin[TBOX_VIN_SIZE + 1] = {0};
    GTimer *costTimer = NULL;
    int32_t result = -2;
    int ret = 0;

    do {
        if (ctx == NULL || task == NULL) {
            LOG_Error("invalid param");
            result = -1;
            break;
        }

        snprintf(rotatedLogFilePath, sizeof(rotatedLogFilePath), "%s/%s",
                 task->logPath, task->logRotatedFileName);

        /** 判断日志流转文件是否生成，并且文件空闲 **/
        if (IsFileExistsAndIdel(rotatedLogFilePath) != 0) {
            result = 1; /* 跳过 */
            break;
        }

        /* 确认需要压缩后再获取 VIN，获取失败时使用默认 VIN */
        (void)LogPackGetVin(vin, sizeof(vin));

        costTimer = g_timer_new();
        if (costTimer == NULL) {
            LOG_Error("g_timer_new failed");
            result = -2;
            break;
        }

        /* ++++ 压缩日志，添加休眠锁 ++++ */
        LOG_Warn("[%s] task[%s] Prepare to CompressAndCryptoLogFile", __func__, task->name);
        g_timer_start(costTimer);
        PmSetSleepEvent(LOG_SERVER_TAG);

        ret = CompressAndCryptoLogFile(ctx, task, vin, rotatedLogFilePath, latestTgzFileName);

        PmCleanSleepEvent(LOG_SERVER_TAG);
        g_timer_stop(costTimer);
        /* ---- 压缩日志，释放休眠锁 ---- */

        if (ret != 0) {
            LOG_Error("[%s] task[%s] compress failed, ret:%d", __func__, task->name, ret);
            result = -2;
            break;
        }

        LOG_Warn("[%s] task[%s] nextIndex:%" PRIu64 " cost:%.2lf s",
                 __func__, task->name, task->index + 1, g_timer_elapsed(costTimer, NULL));

        result = 0;
    } while (0);

    if (costTimer != NULL) {
        g_timer_destroy(costTimer);
    }

    return result;
}

/**
 * 日志自动压缩加密任务线程。
 *
 * 说明：
 * 1. 线程启动时，对每个 task 调用 LogPackTaskInit()，通过 LogPackRebuildIdx() 全量扫描
 *    一次打包目录，建立内存中的轻量索引（不保存每个文件的完整信息）。
 * 2. 主循环每 10s 执行一轮；刚开机 60s 内、刚唤醒 30s 内、PM 非 NORMAL 状态、FOTA 期间
 *    均跳过打包，避免影响系统稳定性。
 * 3. 每次唤醒度过 30s 延迟后，会检查 needReconcile 标志；若为 1，则对每个 task 重新
 *    调用 LogPackRebuildIdx()，纠正运行期间可能产生的大小/数量漂移。
 * 4. 每轮最多只压缩一个 task 的一个文件，然后更新 task->idx（maxIndex/totalSize/fileCount
 *    /oldestIndex），再调用 LogPackCleanupOldest() 按需删除最旧文件。
 * 5. task->idx 的更新统一在本函数中完成，LogPackTaskProcess / CompressAndCryptoLogFile
 *    只负责压缩并返回最新 .tgz 文件名，不修改 task 状态。
 */
static void* LogStoreTaskFunc(void* user_data)
{
    uint32_t i = 0;
    uint32_t compressFailCount = 0;
    int32_t processRet = 0;
    uint64_t newSize = 0;
    LogSrCtx_t* ctx = (LogSrCtx_t*)user_data;
    LogSrPackTask_t* task = NULL;
    char latestTgzFileName[LOG_FILENAME_MAX_SIZE] = {0};
    char latestTgzPath[LOG_FULLPATH_MAX_SIZE] = {0};
    char srcLogFilePath[LOG_FULLPATH_MAX_SIZE] = {0};

    if (ctx == NULL || ctx->taskCnt == 0 || ctx->tasks == NULL) {
        LOG_Error("invalid context");
        return 0;
    }

    if (ctx->taskCnt > LOG_PACK_TASK_MAX_NUM) {
        LOG_Error("taskCnt[%u] exceeds max[%d]", ctx->taskCnt, LOG_PACK_TASK_MAX_NUM);
        return 0;
    }

    /* 线程启动时，为每个 task 重建索引（全量扫描一次） */
    for (i = 0; (i < ctx->taskCnt && i < LOG_PACK_TASK_MAX_NUM); i++) {
        LogPackTaskInit(&ctx->tasks[i]);
    }

    /* 主循环：按 checkInterval 间隔检查是否需要打包 */
    while (g_atomic_int_get(&ctx->logThreadQuitFlag) == 0) {
        
        /* 每次循环休眠 checkInterval 秒，退出标志置位时立即结束 */
        for (i = 0; i < ctx->checkInterval; i++) {
            sleep(1);
            if (g_atomic_int_get(&ctx->logThreadQuitFlag) != 0) {
                break;
            }
        }

        /* PM 非 NORMAL 时不处理，避免在休眠/异常状态下操作文件 */
        if (PmStateGet(ctx) != PM_STATE_NORMAL) {
            continue;
        }

        /** 判断当前是否正在执行Fota **/
        if (PmIsSleepEventExists(PM_SLEEP_EVENT_UPGRADE) == 1) {
            LOG_Warn("[%s] duration otaState not allow to compress log file", __func__);
            continue;
        }
        
        /** 刚开机，不允许执行日志打包（时长由配置决定，默认 60s）**/
        if (PmBootUpTimerElapsed(ctx) <= ctx->bootUpDelay) {
            LOG_Warn("[%s] duration bootup %us not allow to compress log file", __func__, ctx->bootUpDelay);
            continue;
        }

        /** 刚唤醒，不允许执行日志打包（时长由配置决定，默认 30s）**/
        if (PmWakeUpTimerElapsed(ctx) <= ctx->wakeUpDelay) {
            LOG_Warn("[%s] duration wakeup after %us not allow to compress log file", __func__, ctx->wakeUpDelay);
            continue;
        }

        /** 唤醒后重新 reconcile 索引，纠正运行期漂移 */
        if (PmNeedReconcileGet(ctx)) {
            for (i = 0; (i < ctx->taskCnt && i < LOG_PACK_TASK_MAX_NUM); i++) {
                LogPackRebuildIdx(&ctx->tasks[i]);
            }
            PmNeedReconcileSet(ctx, 0);
        }

        /* 依次尝试每个 task，本轮最多只成功打包一个文件 */
        for (i = 0; (i < ctx->taskCnt && i < LOG_PACK_TASK_MAX_NUM); i++) {
            task = &ctx->tasks[i];
            memset(latestTgzFileName, 0, sizeof(latestTgzFileName));

            /* 尝试压缩当前 task 的轮转日志 */
            processRet = LogPackTaskProcess(ctx, task, latestTgzFileName);
            if (processRet == 0) {
                /* 压缩成功，清除连续失败计数 */
                compressFailCount = 0;

                /* 删除已处理的源日志 */
                memset(srcLogFilePath, 0, sizeof(srcLogFilePath));
                snprintf(srcLogFilePath, sizeof(srcLogFilePath), "%s/%s",
                         task->logPath, task->logRotatedFileName);
                if (access(srcLogFilePath, F_OK) == 0) {
                    LOG_Warn("[%s] task[%s] remove src log after success:%s",
                             __func__, task->name, srcLogFilePath);
                    remove(srcLogFilePath);
                }

                /* 打包成功，序号前进 1 */
                task->index += 1;

                /* 压缩成功，根据返回的文件名更新本 task 的轻量索引 */
                snprintf(latestTgzPath, sizeof(latestTgzPath), "%s/%s",
                         task->logPackPath, latestTgzFileName);
                newSize = GetFileSize(latestTgzPath);
                task->idx.maxIndex = task->index;
                task->idx.totalSize += newSize;
                task->idx.fileCount++;
                if (task->idx.fileCount == 1 || task->idx.oldestIndex == 0) {
                    task->idx.oldestIndex = task->idx.maxIndex;
                }
                LOG_Warn("[%s] task[%s] add new file:%s size:%llu totalSize:%llu fileCount:%u",
                         __func__, task->name, latestTgzFileName,
                         (unsigned long long)newSize,
                         (unsigned long long)task->idx.totalSize,
                         task->idx.fileCount);

                LogPackCleanupOldest(task);
                break; /** 每次循环，最多执行一次文件打包，避免持续占用cpu资源 **/
            }  else if(processRet ==1 ) {
                /* processRet == 1 表示无文件或文件忙，跳过不处理 */
            } else {
                /* 压缩失败，累计连续失败计数 */
                compressFailCount++;
                LOG_Warn("[%s] task[%s] compress failed, compressFailCount:%u",__func__, task->name, compressFailCount);

                /* 连续失败三次后，后续每次失败都删除源日志避免无限重试 */
                if (compressFailCount >= 3) {
                    memset(srcLogFilePath, 0, sizeof(srcLogFilePath));
                    snprintf(srcLogFilePath, sizeof(srcLogFilePath), "%s/%s", task->logPath, task->logRotatedFileName);
                    if (access(srcLogFilePath, F_OK) == 0) {
                        LOG_Warn("[%s] task[%s] remove src log after 3 consecutive failures:%s", __func__, task->name, srcLogFilePath);
                        remove(srcLogFilePath);
                    }
                }
            }
        }
    }

    return 0;
}


/**
 * @brief 初始化 LogSrCtx_t 中指定的全局加密接口
 */
static int32_t CryptoInterfaceInit(LogSrCtx_t* ctx)
{
    if (ctx == NULL || g_cryptoInterfaces == NULL || g_cryptoInterfaces[0] == NULL) {
        LOG_Error("no crypto interface available");
        return RET_ERR;
    }

    if (ctx->cryptoInterface == NULL) {
        return RET_OK;
    }

    if (ctx->cryptoInterface->crypto_init == NULL) {
        return RET_OK;
    }

    if (ctx->cryptoInterface->crypto_init() != 0) {
        LOG_Error("crypto_init failed for %s", ctx->cryptoInterface->name ? ctx->cryptoInterface->name : "unknown");
        return RET_ERR;
    }

    LOG_Info("crypto interface %s initialized", ctx->cryptoInterface->name ? ctx->cryptoInterface->name : "unknown");
    return RET_OK;
}

/**
 * @brief 释放 LogSrCtx_t 中指定的全局加密接口
 */
static void CryptoInterfaceDeinit(LogSrCtx_t* ctx)
{
    if (ctx == NULL || ctx->cryptoInterface == NULL || ctx->cryptoInterface->crypto_deinit == NULL) {
        return;
    }

    ctx->cryptoInterface->crypto_deinit();
    LOG_Info("crypto interface %s deinitialized", ctx->cryptoInterface->name ? ctx->cryptoInterface->name : "unknown");
}

/** 
 * 初始化加载日志保存的等级和使能状态，开启日志自动压缩加密的任务线程
 * **/
int32_t LogStoreTaskInit(LogSrCtx_t* ctx)
{
    int32_t result = RET_ERR;

    do {
        if (ctx == NULL || g_cryptoInterfaces == NULL || g_cryptoInterfaces[0] == NULL) {
            LOG_Error("invalid log server context");
            break;
        }

        /** init -- load openssl algorithms */
        if (CryptoInterfaceInit(ctx) != RET_OK) {
            LOG_Error("CryptoInterfaceInit failed");
            break;
        }

        /** init -- start log pack task thread */
        if (ctx->tasks == NULL || ctx->taskCnt == 0) {
            LOG_Error("invalid log task config");
            CryptoInterfaceDeinit(ctx);
            break;
        }

        g_atomic_int_set(&ctx->logThreadQuitFlag, 0);
        ctx->logThread = g_thread_new("LogStoreTaskFunc", LogStoreTaskFunc, ctx);
        if (ctx->logThread == NULL) {
            LOG_Error("g_thread_new failed");
            CryptoInterfaceDeinit(ctx);
            break;
        }

        result = RET_OK;
    } while (0);

    return result;
}

void LogStoreTaskDeinit(LogSrCtx_t* ctx)
{
    do {
        if (ctx == NULL) {
            break;
        }

        /** 通知并等待工作线程退出 */
        if (ctx->logThread != NULL) {
            g_atomic_int_set(&ctx->logThreadQuitFlag, 1);
            g_thread_join(ctx->logThread);
            ctx->logThread = NULL;
        }

        /** deinit -- release openssl algorithms */
        CryptoInterfaceDeinit(ctx);
    } while (0);
}
