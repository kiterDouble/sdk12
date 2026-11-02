#include <atomic>
#include <sys/stat.h>
#include <unistd.h>
#include "terminal_management_api.h"
#include "sr_log.h"
#include "srTspCallbackRegister.h"
#include "srTspPlatformInterface.h"
#include "terminal_management_data_monitor.h"
#include "terminal_management_data_report.h"

namespace
{
    static std::atomic<bool> g_isInit(false); // 是否初始化
} // namespace

/**
 * @brief 辅助函数-确保配置地址存在，不存在则创建
 *
 * @param dir
 * @return int32_t
 */
static int32_t EnsureDirExists(const char *dir)
{
    if (dir == NULL)
        return RET_ERR;

    /* 1. 检查目录是否已存在 */
    struct stat st;
    if (stat(dir, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            return RET_OK; // 目录已存在，直接成功
        }
        else
        {
            // 存在同名文件，错误
            return RET_ERR;
        }
    }
    else
    {
        // stat 失败，可能是路径不存在，或其他错误（权限等）
        if (errno != ENOENT)
        {
            return RET_ERR; // 不可恢复的错误（如只读文件系统、IO 错误等）
        }
    }

    /* 2. 路径不存在，逐层创建 */
    char tmp[128];
    size_t len = strlen(dir);
    if (len >= 128)
        return RET_ERR;

    memcpy(tmp, dir, len + 1);

    /* 去掉末尾的 '/' */
    if (tmp[len - 1] == '/')
    {
        tmp[len - 1] = '\0';
        len--;
    }

    for (char *p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0)
            {
                if (errno != EEXIST)
                {
                    return RET_ERR; // 创建失败（如只读分区）
                }
                // EEXIST：路径已存在，但可能是文件，需要检查
                struct stat sub_st;
                if (stat(tmp, &sub_st) != 0 || !S_ISDIR(sub_st.st_mode))
                {
                    return RET_ERR; // 存在但不是目录（普通文件），或 stat 失败
                }
            }
            *p = '/';
        }
    }

    /* 最后创建完整路径 */
    if (mkdir(tmp, 0755) != 0)
    {
        if (errno != EEXIST)
        {
            return RET_ERR;
        }
        // EEXIST 时再次确认是目录（可能并发创建）
        struct stat sub_st;
        if (stat(tmp, &sub_st) != 0 || !S_ISDIR(sub_st.st_mode))
        {
            return RET_ERR;
        }
    }

    return RET_OK;
}

int ConfigAddressInit(const char *dataFilePath,
                      const char *taskConfigFilePath,
                      const char *crtStorePath,
                      const char *downloadFolder,
                      const char *unzipFolder)
{
    if (dataFilePath == nullptr ||
        taskConfigFilePath == nullptr ||
        crtStorePath == nullptr ||
        downloadFolder == nullptr ||
        unzipFolder == nullptr)
    {
        LOG_Error("ConfigAddressInit failed, parameter is nullptr\n");
        return RET_ERR;
    }

    if (EnsureDirExists(dataFilePath) == RET_ERR)
    {
        LOG_Error("crtInit failed, dataFilePath does not exist and cannot be created\n");
        return RET_ERR;
    }
    if (EnsureDirExists(taskConfigFilePath) == RET_ERR)
    {
        LOG_Error("crtInit failed, taskConfigFilePath does not exist and cannot be created\n");
        return RET_ERR;
    }
    if (EnsureDirExists(crtStorePath) == RET_ERR)
    {
        LOG_Error("crtInit failed, crtStorePath does not exist and cannot be created\n");
        return RET_ERR;
    }
    if (EnsureDirExists(downloadFolder) == RET_ERR)
    {
        LOG_Error("crtInit failed, downloadFolder does not exist and cannot be created\n");
        return RET_ERR;
    }
    if (EnsureDirExists(unzipFolder) == RET_ERR)
    {
        LOG_Error("crtInit failed, unzipFolder does not exist and cannot be created\n");
        return RET_ERR;
    }

    if (SRTspServiceSetCaSavePath(crtStorePath) != RET_OK)
    {
        LOG_Error("MonitorInit failed, SRTspServiceSetCaSavePath failed\n");
        return RET_ERR;
    }
    if (MonitorCfgPathInit(dataFilePath, taskConfigFilePath, downloadFolder, unzipFolder) != RET_OK)
    {
        LOG_Error("MonitorInit failed, MonitorCfgPathInit failed\n");
        return RET_ERR;
    }

    g_isInit.store(true);

    return RET_OK;
}

int RegisterQueryBaseInfoCallback(const StBaseInfoCallbacks_t *callback)
{
    return TspBaseInfoRegisterCallbacks(callback);
}

int TspClientStart(void)
{
    if (!g_isInit.load())
    {
        LOG_Error("TspClientStart failed: client not initialized\n");
        return RET_ERR;
    }

    // 启动tsp状态机
    if (StartTspStateMachine() != RET_OK)
    {
        LOG_Error("StartTspStateMachine failed\n");
        return RET_ERR;
    }
    return RET_OK;
}

int ServiceStop(void)
{
    if (!g_isInit.load())
    {
        LOG_Error("ServiceStop failed: client not initialized\n");
        return RET_ERR;
    }

    // 停止tsp状态机
    if (StopTspStateMachine() != RET_OK)
    {
        LOG_Error("StopTspStateMachine failed\n");
        return RET_ERR;
    }

    // 停止上下行数据处理
    if (DataProcessingStop() != RET_OK)
    {
        LOG_Error("DataProcessingStop failed\n");
        return RET_ERR;
    }

    // 停止数据组包
    if (DataPackagingStop() != RET_OK)
    {
        LOG_Error("DataPackagingStop failed\n");
        return RET_ERR;
    }
    return RET_OK;
}
