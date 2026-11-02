#include "mqtt_log_sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <algorithm>
#include <vector>
#include <set>
#include <string>
#include "sr_log.h"

/* ==================== SCP 传输配置宏定义 ==================== */

/** @brief SCP 目标主机 IP 地址 */
#define SCP_TARGET_HOST "192.168.1.201"

/** @brief SCP 目标路径（远程主机上的目录） */
#define SCP_TARGET_PATH "/mnt/carota/sublog"

/** @brief SSH 登录用户名 */
#define SCP_USERNAME "apps"

/** @brief SCP 传输超时时间（秒），使用 timeout 命令限制整个 scp 进程的最大运行时间 */
#define SCP_TRANSFER_TIMEOUT 30

/** @brief 单个文件传输最大重试次数，超过此次数后判定该文件传输失败 */
#define SCP_RETRY_MAX 3

/** @brief 连续失败告警阈值，连续失败次数达到此值时输出警告日志 */
#define SCP_CONSECUTIVE_FAIL_THRESHOLD 5

/** @brief 每轮传输循环之间的休眠间隔（秒） */
#define SCP_INTERVAL_SECONDS 60

/** @brief 每轮循环最多传输的文件数量，避免一次性传输过多文件 */
#define SCP_MAX_FILES_PER_CYCLE 50

/** @brief 远程目录文件数量检查间隔（秒） */
#define REMOTE_DIR_CHECK_INTERVAL 300

/** @brief 远程目录最大文件数量，超过此数量将删除最旧的文件 */
#define REMOTE_DIR_MAX_FILES 200

/** @brief 日志文件所在目录（当前工作目录） */
#define LOG_DIR "/oemdata/log/logstore/"

/** @brief 已传输成功文件的历史记录文件名，用于避免重复传输 */
#define SYNC_LOG_HISTORY_FILE  "/oemdata/log/logstore/SyncLogHistory.txt"

// 判断是否为主模组
static inline int IsMasterModule(void)
{
    return (0 == access("/dev/mmcblk0", F_OK));
}

// 加载已传输成功的文件列表
static std::set<std::string> loadSyncLogHistory()
{
    std::set<std::string> history;
    FILE *fp = fopen(SYNC_LOG_HISTORY_FILE, "r");
    if (fp != NULL)
    {
        char line[256];
        while (fgets(line, sizeof(line), fp) != NULL)
        {
            // 去除换行符
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n')
            {
                line[len - 1] = '\0';
            }
            if (strlen(line) > 0)
            {
                history.insert(std::string(line));
            }
        }
        fclose(fp);
    }
    return history;
}

// 保存传输成功的文件到历史记录（使用临时文件+rename实现原子写入）
static void saveSyncLogHistory(const std::set<std::string> &history)
{
    // 构建临时文件名
    char temp_file[512];
    snprintf(temp_file, sizeof(temp_file), "%s.tmp", SYNC_LOG_HISTORY_FILE);

    // 清理可能残留的临时文件
    remove(temp_file);

    FILE *fp = fopen(temp_file, "w");
    if (fp == NULL)
    {
        LOG_Error("SyncLogFileThread: failed to open temp file %s", temp_file);
        return;
    }

    for (std::set<std::string>::const_iterator it = history.begin(); it != history.end(); ++it)
    {
        fprintf(fp, "%s\n", it->c_str());
    }

    // 确保数据写入磁盘
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);

    // 原子替换
    if (rename(temp_file, SYNC_LOG_HISTORY_FILE) != 0)
    {
        LOG_Error("SyncLogFileThread: failed to rename %s to %s", temp_file, SYNC_LOG_HISTORY_FILE);
        remove(temp_file); // 清理临时文件
    }
}

// 获取目录下所有 .tgz 文件
static std::vector<std::string> getTgzFiles()
{
    std::vector<std::string> tgz_files;
    DIR *dir = opendir(LOG_DIR);
    if (dir == NULL)
    {
        LOG_Error("SyncLogFileThread: cannot open log directory %s", LOG_DIR);
        return tgz_files;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string filename = entry->d_name;
        if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".tgz")
        {
            tgz_files.push_back(filename);
        }
    }
    closedir(dir);

    // 按文件名排序
    std::sort(tgz_files.begin(), tgz_files.end());
    return tgz_files;
}

// 网络连通性检测
static bool checkNetworkConnectivity()
{
    char ping_cmd[256];
    snprintf(ping_cmd, sizeof(ping_cmd), "ping -c 2 -W 3 %s > /dev/null 2>&1", SCP_TARGET_HOST);
    int ret = system(ping_cmd);
    if (ret == -1)
    {
        LOG_Error("SyncLogFileThread: system() failed for ping command");
        return false;
    }
    return (WEXITSTATUS(ret) == 0);
}

// 文件存在性检查
static bool checkSourceFileExists(const std::string &filename)
{
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s%s", LOG_DIR, filename.c_str());

    struct stat st;
    if (stat(filepath, &st) != 0)
    {
        LOG_Error("SyncLogFileThread: source file '%s' not found", filepath);
        return false;
    }
    return true;
}

// 执行 SCP 传输（捕获并打印命令输出）
static bool executeScpTransfer(const std::string &filename)
{
    // 构建完整文件路径
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s%s", LOG_DIR, filename.c_str());

    // 构建内部 scp 命令
    char inner_scp_cmd[1536];
    snprintf(inner_scp_cmd, sizeof(inner_scp_cmd),
             "scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null %s %s@%s:%s",
             filepath, SCP_USERNAME, SCP_TARGET_HOST, SCP_TARGET_PATH);

    // 使用 su - apps -c 执行命令，并保留超时逻辑
    char scp_cmd[1792];
    snprintf(scp_cmd, sizeof(scp_cmd),
             "timeout %d su - apps -c \"%s\" 2>&1",
             SCP_TRANSFER_TIMEOUT, inner_scp_cmd);

    FILE *pipe = popen(scp_cmd, "r");
    if (pipe == NULL)
    {
        LOG_Error("SyncLogFileThread: failed to execute scp command");
        return false;
    }

    char buffer[256];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        output += buffer;
    }

    int ret = pclose(pipe);
    int exit_code = WEXITSTATUS(ret);

    // 打印 scp 输出（成功或失败信息）
    if (!output.empty())
    {
        // 去除末尾换行
        while (!output.empty() && (output.back() == '\n' || output.back() == '\r'))
        {
            output.pop_back();
        }
        if (exit_code == 0)
        {
            LOG_Info("SyncLogFileThread: scp success: %s", output.c_str());
        }
        else
        {
            LOG_Error("SyncLogFileThread: scp failed (exit_code=%d): %s", exit_code, output.c_str());
        }
    }
    else if (exit_code != 0)
    {
        LOG_Error("SyncLogFileThread: scp failed (exit_code=%d), no output", exit_code);
    }

    return (exit_code == 0);
}

// 获取本地目录文件列表（按修改时间排序，最旧的在前）
static std::vector<std::string> getLocalDirFiles()
{
    std::vector<std::string> files;
    DIR *dir = opendir(SCP_TARGET_PATH);
    if (dir == NULL)
    {
        LOG_Error("SyncLogFileThread: cannot open directory %s", SCP_TARGET_PATH);
        return files;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name(entry->d_name);
        if (name == "." || name == "..")
        {
            continue;
        }
        // 只保留 .tgz 文件
        if (name.size() > 4 && name.substr(name.size() - 4) == ".tgz")
        {
            files.push_back(name);
        }
    }
    closedir(dir);

    // 按修改时间排序（使用 stat 获取文件信息）
    std::sort(files.begin(), files.end(), [](const std::string &a, const std::string &b) {
        struct stat sa, sb;
        char path_a[512], path_b[512];
        snprintf(path_a, sizeof(path_a), "%s/%s", SCP_TARGET_PATH, a.c_str());
        snprintf(path_b, sizeof(path_b), "%s/%s", SCP_TARGET_PATH, b.c_str());
        int ret_a = stat(path_a, &sa);
        int ret_b = stat(path_b, &sb);
        if (ret_a != 0 || ret_b != 0)
        {
            // stat 失败的文件排在后面
            return (ret_a == 0);
        }
        return sa.st_mtime < sb.st_mtime; // 最旧的在前
    });

    return files;
}

// 删除本地目录中最旧的文件
static bool deleteOldestLocalFile(const std::string &filename)
{
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", SCP_TARGET_PATH, filename.c_str());
    if (remove(filepath) == 0)
    {
        return true;
    }
    LOG_Error("SyncLogFileThread: failed to delete %s", filepath);
    return false;
}

// 检查并清理本地目录文件
static void checkAndCleanupLocalDir()
{
    std::vector<std::string> local_files = getLocalDirFiles();

    if (local_files.empty())
    {
        LOG_Debug("SyncLogFileThread: local directory is empty");
        return;
    }

    LOG_Debug("SyncLogFileThread: local directory has %zu file(s)", local_files.size());

    // 如果文件数量超过阈值，删除最旧的
    while ((int)local_files.size() > REMOTE_DIR_MAX_FILES)
    {
        const std::string &oldest_file = local_files.front();
        LOG_Info("SyncLogFileThread: local directory exceeds limit, deleting oldest file: %s", oldest_file.c_str());

        if (deleteOldestLocalFile(oldest_file))
        {
            LOG_Info("SyncLogFileThread: deleted %s successfully", oldest_file.c_str());
            local_files.erase(local_files.begin());
        }
        else
        {
            LOG_Error("SyncLogFileThread: failed to delete %s", oldest_file.c_str());
            break;
        }
    }
}

void *SyncLogFileThread(void *pArg)
{
    (void)pArg;
    LOG_Info("SyncLogFileThread started");

    int isMasterModule = IsMasterModule();

    if (isMasterModule)
    {
        // 判定为主模组，执行目录创建，定期删除旧文件
        // 创建 SCP_TARGET_PATH 目录并设置权限
        char mkdir_cmd[256];
        char chmod_cmd[256];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", SCP_TARGET_PATH);
        snprintf(chmod_cmd, sizeof(chmod_cmd), "chmod 777 %s", SCP_TARGET_PATH);

        if (system(mkdir_cmd) != 0)
        {
            LOG_Error("SyncLogFileThread: failed to create directory %s", SCP_TARGET_PATH);
        }
        if (system(chmod_cmd) != 0)
        {
            LOG_Error("SyncLogFileThread: failed to set permissions for %s", SCP_TARGET_PATH);
        }
        LOG_Info("SyncLogFileThread: created and set permissions for %s", SCP_TARGET_PATH);

        while (true)
        {
            checkAndCleanupLocalDir();
            sleep(REMOTE_DIR_CHECK_INTERVAL);
        }
    }
    else
    {
        // 判定为副模组，执行日志同步逻辑
        int consecutive_fail_count = 0;
        std::set<std::string> sync_history = loadSyncLogHistory();

        while (true)
        {
            // 1. 网络连通性检测
            if (!checkNetworkConnectivity())
            {
                LOG_Error("SyncLogFileThread: network unreachable, skip this cycle");
                consecutive_fail_count++;
                sleep(SCP_INTERVAL_SECONDS);
                continue;
            }

            // 2. 获取所有 .tgz 文件
            std::vector<std::string> tgz_files = getTgzFiles();
            if (tgz_files.empty())
            {
                LOG_Debug("SyncLogFileThread: no .tgz files found, skip this cycle");
                sleep(SCP_INTERVAL_SECONDS);
                continue;
            }

            // 3. 收集未传输的文件（每次最多 SCP_MAX_FILES_PER_CYCLE 个）
            std::vector<std::string> pending_files;
            for (size_t i = 0; i < tgz_files.size(); i++)
            {
                const std::string &file = tgz_files[i];
                if (sync_history.find(file) == sync_history.end())
                {
                    pending_files.push_back(file);
                    if ((int)pending_files.size() >= SCP_MAX_FILES_PER_CYCLE)
                    {
                        break;
                    }
                }
            }

            if (pending_files.empty())
            {
                LOG_Debug("SyncLogFileThread: all .tgz files have been transferred, skip this cycle");
                sleep(SCP_INTERVAL_SECONDS);
                continue;
            }

            LOG_Info("SyncLogFileThread: found %zu pending file(s) to transfer", pending_files.size());

            // 4. 逐个传输文件
            for (size_t i = 0; i < pending_files.size(); i++)
            {
                const std::string &target_file = pending_files[i];

                // 4.1 源文件存在性检查
                if (!checkSourceFileExists(target_file))
                {
                    consecutive_fail_count++;
                    continue;
                }

                // 4.2 重试机制
                bool transfer_success = false;
                for (int retry = 0; retry < SCP_RETRY_MAX; retry++)
                {
                    LOG_Info("SyncLogFileThread: executing scp transfer for %s (attempt %d/%d)...",
                             target_file.c_str(), retry + 1, SCP_RETRY_MAX);
                    if (executeScpTransfer(target_file))
                    {
                        transfer_success = true;
                        break;
                    }
                    LOG_Error("SyncLogFileThread: scp transfer failed, retry %d/%d", retry + 1, SCP_RETRY_MAX);
                    sleep(2);
                }

                // 4.3 结果处理和连续失败告警
                if (transfer_success)
                {
                    LOG_Info("SyncLogFileThread: scp transfer success for %s", target_file.c_str());
                    sync_history.insert(target_file);
                    saveSyncLogHistory(sync_history);
                    consecutive_fail_count = 0;
                }
                else
                {
                    consecutive_fail_count++;
                    LOG_Error("SyncLogFileThread: scp transfer failed for %s after %d retries",
                              target_file.c_str(), SCP_RETRY_MAX);

                    if (consecutive_fail_count >= SCP_CONSECUTIVE_FAIL_THRESHOLD)
                    {
                        LOG_Error("SyncLogFileThread: WARNING! Consecutive failures reached %d, "
                                  "there may be a serious problem with the network or remote host",
                                  consecutive_fail_count);
                    }
                }
            }

            sleep(SCP_INTERVAL_SECONDS);
        }
    }

    return NULL;
}
