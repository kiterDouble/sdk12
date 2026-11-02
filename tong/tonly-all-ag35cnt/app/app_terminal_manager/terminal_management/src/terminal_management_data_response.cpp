#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <atomic>
#include <sys/prctl.h>
#include <curl/curl.h>
#include <sys/stat.h>

#include "queue_msg.h"
#include "mcu_api.h"
#include "cJSON.h"
#include "srTspFWService.h"
#include "srTspCallbackRegister.h"
#include "srTspFWMqttClient.h"
#include "srTspRemoteProcess.h"
#include "terminal_management_data_report.h"
#include "terminal_management_data_response.h"
#include "srTspEncryptAndDecrypt.h"
#include "terminal_management_data_monitor.h"

#define SINGLE_FILE_UPLOAD_THRESHOLD (10 * 1024 * 1024) // 单文件上传阈值10M
#define RECV_MCU_MSG_TIMEOUT_MS (3000)                  // 接收MCU响应的超时时间

struct MemoryStruct
{
    char *memory;
    size_t size;
}; // 用于保存 HTTP 响应数据的回调

typedef enum
{
    UPLOAD_MODE_SINGLE = 0, // 单文件上传
    UPLOAD_MODE_MULTIPART   // 分片上传
} UploadMode;

/**
 * @brief 日志分片上传所需数据
 */
typedef struct
{
    uint64_t partSize = 0;          // 服务器建议设置的每个分片的大小
    std::vector<std::string> etags; // 已成功上传分片的MD5值列表（ETag）

    FILE *fp = nullptr;       // 当前打开的待上传文件指针
    long remaining = 0;       // 文件剩余未读取的字节数
    int partNumber = 1;       // 分片编号
    std::vector<char> buffer; // 读取分片的临时缓冲区（大小 = partSize）
    bool finished = false;    // true 表示所有分片已上传完毕（或空文件）
} SliceUploadContext;

pthread_cond_t g_recvedMcuRebootAckCond = PTHREAD_COND_INITIALIZER;    // 用于通知接收到MCU重启ACK回执
pthread_mutex_t g_recvedMcuRebootAckMutex = PTHREAD_MUTEX_INITIALIZER; // 接收到MCU重启ACK回执的互斥锁

// 辅助函数
// ******************************************************************************************************************************************************//

/**
 * @brief 向平台发送命令执行结果
 *
 * @param cmdId
 * @param result
 * @param errCode
 * @param errLog
 */
void SendCommandResponse(const std::string &cmdId,
                         net::sirun::tboxs3::connect::command::ControlResult result,
                         int32_t errCode,
                         const std::string &errLog)
{
    net::sirun::tboxs3::connect::command::S3Command response;
    response.set_command_id(cmdId);
    auto *uplink = response.mutable_uplink();
    uplink->set_result(result);
    uplink->set_err_code(errCode);
    uplink->set_err_log(errLog);
    uplink->set_uplink_report_time((int64_t)::time(nullptr));

    std::string binary;
    if (!response.SerializeToString(&binary))
    {
        SR_TSP_LOG_E("Failed to serialize command response");
        return;
    }

    SRMqttMessage_t reqMsg = SRMqttMessage_initializer;
    char snBuffer[64] = {0};
    TspBaseInfoGetSn(snBuffer, sizeof(snBuffer));
    snprintf(reqMsg.topicName, sizeof(reqMsg.topicName), "%s%s", SR_TOPIC_PREFIX_T2S_COMMAND, snBuffer);
    reqMsg.topicLen = strlen(reqMsg.topicName);
    /* 若加密会话已激活则加密 payload */
    if (EncryptionIsActive())
    {
        uint8_t *encrypted = NULL;
        int encLen = EncryptPayload(
            reinterpret_cast<const uint8_t *>(binary.data()), binary.size(), &encrypted);
        if (encLen > 0 && encLen <= (int)sizeof(reqMsg.payload))
        {
            memcpy(reqMsg.payload, encrypted, (size_t)encLen);
            reqMsg.payloadLen = (size_t)encLen;
            free(encrypted);
        }
        else
        {
            SR_TSP_LOG_E("SendCommandResponse: encrypt failed (encLen=%d)", encLen);
            if (encrypted)
                free(encrypted);
            return;
        }
    }
    else
    {
        memcpy(reqMsg.payload, binary.data(), binary.size());
        reqMsg.payloadLen = binary.size();
    }

    if (SRTspServiceSendMessage(&reqMsg) != RET_OK)
    {
        SR_TSP_LOG_E("Failed to send command response");
    }
}

/**
 * @brief 简单的通配符匹配（支持 * 和 ?）
 *
 * @param pattern 通配符匹配规则
 * @param str 待确认的字符串
 * @return int 1：匹配
 *         int 0：不匹配
 */
static int WildcardMatch(const char *pattern, const char *str)
{
    if (!pattern || !str)
        return RET_OK;
    while (*pattern && *str)
    {
        if (*pattern == '*')
        {
            pattern++;
            if (!*pattern)
                return 1; // 末尾 '*' 匹配剩余全部
            while (*str)
            {
                if (WildcardMatch(pattern, str))
                    return 1;
                str++;
            }
            return RET_OK;
        }
        else if (*pattern == '?' || *pattern == *str)
        {
            pattern++;
            str++;
        }
        else
        {
            return RET_OK;
        }
    }
    // 处理尾部可能剩余的 '*'
    while (*pattern == '*')
        pattern++;
    return *pattern == '\0' && *str == '\0';
}

/**
 * @brief CURL命令 HTTP响应回调
 *
 * @param contents
 * @param size
 * @param nmemb
 * @param userp
 * @return size_t
 */
static size_t CurlWriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = static_cast<char *>(realloc(mem->memory, mem->size + realsize + 1));
    if (!ptr)
    {
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return RET_OK;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

/**
 * @brief 清理缓存文件
 *
 * @param filePath
 * @return int
 */
static int CleanCacheFile(const char *filePath)
{
    if (!filePath)
        return RET_ERR;

    if (access(filePath, F_OK) == 0)
    {
        if (unlink(filePath) != 0)
        {
            return RET_ERR;
        }
    }
    return RET_OK;
}

/**
 * @brief 截取文件名称
 *
 * @param filePath 文件路径
 * @return const char* 文件名称
 */
static const char *GetFileName(const char *filePath)
{
    char fileName[512];
    strncpy(fileName, filePath, sizeof(fileName) - 1);
    fileName[sizeof(fileName) - 1] = '\0';
    return basename(fileName);
}

// 日志文件归档
// ******************************************************************************************************************************************************//

/**
 * @brief 从目录中收集匹配 pattern 的 .tgz 文件，记录文件名列表及全局时间戳。
 *
 * @param logstorePath  目录路径
 * @param pattern       文件名匹配模式（如 "*.tgz"）
 * @param fileList      输出参数，返回文件名数组（需调用方释放）
 * @param fileCount     输出参数，返回文件数量
 * @param minMtime      输出参数，返回最早修改时间
 * @param maxMtime      输出参数，返回最晚修改时间
 * @param errMsg        输出参数，返回错误信息
 * @return RET_OK 成功，-1 失败
 */
static int CollectTgzFiles(const char *logstorePath,
                           const char *pattern,
                           char ***fileList,
                           int *fileCount,
                           long long *minMtime,
                           long long *maxMtime,
                           std::string &errMsg)
{
    if (!logstorePath || !fileList || !fileCount || !minMtime || !maxMtime)
    {
        errMsg = std::string("[") + __func__ + "] Invalid arguments";
        return RET_ERR;
    }

    DIR *dir = opendir(logstorePath);
    if (!dir)
    {
        errMsg = std::string("[") + __func__ + "] opendir failed: " + strerror(errno);
        return RET_ERR;
    }

    *fileList = NULL;
    *fileCount = 0;
    *minMtime = LLONG_MAX;
    *maxMtime = 0;

    const char *usePattern = pattern ? pattern : "*.tgz";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_REG && entry->d_type != DT_UNKNOWN)
            continue;

        const char *name = entry->d_name;

        // 使用通配符匹配
        if (!WildcardMatch(usePattern, name))
            continue;

        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", logstorePath, name);

        struct stat st;
        if (stat(fullPath, &st) != 0)
        {
            fprintf(stderr, "stat failed for %s: %s\n", fullPath, strerror(errno));
            continue;
        }

        // 更新全局时间戳范围
        if (st.st_mtime < *minMtime)
            *minMtime = st.st_mtime;
        if (st.st_mtime > *maxMtime)
            *maxMtime = st.st_mtime;

        // 添加文件名到列表
        char **tmp = (char **)realloc(*fileList, (*fileCount + 1) * sizeof(char *));
        if (!tmp)
        {
            errMsg = std::string("[") + __func__ + "] realloc failed: " + strerror(errno);
            closedir(dir);
            return RET_ERR;
        }
        *fileList = tmp;
        (*fileList)[*fileCount] = strdup(name);
        if (!(*fileList)[*fileCount])
        {
            errMsg = std::string("[") + __func__ + "] strdup failed: " + strerror(errno);
            closedir(dir);
            return RET_ERR;
        }
        (*fileCount)++;
    }

    if (*fileCount == 0)
    {
        errMsg = std::string("[") + __func__ + "] No " + pattern + " files found";
    }
    closedir(dir);
    return RET_OK;
}

/**
 * @brief 根据时间范围和边界条件筛选文件列表，决定是否包含 sirunlog。
 *
 * @param logstorePath  文件所在目录
 * @param fileList      输入的文件名数组
 * @param fileCount     输入的文件数量
 * @param startTime     起始时间（秒）
 * @param endTime       结束时间（秒）
 * @param minMtime      全局最早时间戳
 * @param maxMtime      全局最晚时间戳
 * @param filteredFiles 输出参数，筛选后的文件名数组（需调用方释放）
 * @param filteredCount 输出参数，筛选后的文件数量
 * @param includeSirunlog 输出参数，是否需包含 /sdcard/log/sirunlog
 * @param errMsg        输出参数，返回错误信息
 * @return RET_OK 成功，-1 失败
 */
static int FilterFilesByTime(const char *logstorePath,
                             char **fileList, int fileCount,
                             long long *startTime, long long endTime,
                             long long minMtime, long long maxMtime,
                             char ***filteredFiles, int *filteredCount,
                             int *includeSirunlog,
                             std::string &errMsg)
{
    if (!logstorePath || !fileList || !startTime || !filteredFiles || !filteredCount || !includeSirunlog)
    {
        errMsg = std::string("[") + __func__ + "] Invalid arguments";
        return RET_ERR;
    }

    *filteredFiles = NULL;
    *filteredCount = 0;
    *includeSirunlog = 0;

    if (fileCount == 0)
    {
        fprintf(stderr, "No files to filter\n");
        return RET_OK;
    }

    // 边界处理：若 endTime 比所有文件都新，则标记需要 sirunlog
    if (endTime > maxMtime)
    {
        printf("endTime is newer than all files, will include /sdcard/log/sirunlog\n");
        *includeSirunlog = 1;
    }

    // 筛选文件
    for (int i = 0; i < fileCount; i++)
    {
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", logstorePath, fileList[i]);

        struct stat st;
        if (stat(fullPath, &st) != 0)
        {
            fprintf(stderr, "stat failed for %s: %s\n", fullPath, strerror(errno));
            continue;
        }

        if (st.st_mtime >= *startTime && st.st_mtime <= endTime)
        {
            char **tmp = (char **)realloc(*filteredFiles, (*filteredCount + 1) * sizeof(char *));
            if (!tmp)
            {
                errMsg = std::string("[") + __func__ + "] realloc failed: " + strerror(errno);
                return RET_ERR;
            }
            *filteredFiles = tmp;
            (*filteredFiles)[*filteredCount] = strdup(fileList[i]);
            if (!(*filteredFiles)[*filteredCount])
            {
                errMsg = std::string("[") + __func__ + "] strdup failed: " + strerror(errno);
                return RET_ERR;
            }
            (*filteredCount)++;
        }
    }

    if (*filteredCount == 0 && !(*includeSirunlog))
    {
        // 无文件可打包
        fprintf(stderr, "No files matched the final time range.\n");
    }
    return RET_OK;
}

/**
 * @brief 创建 tar 归档压缩包。
 *
 * @param logstorePath   文件所在的基准目录
 * @param outFilename    输出的完整压缩包路径
 * @param filteredFiles  需打包的文件名数组（仅文件名）
 * @param filteredCount  文件数量
 * @param includeSirunlog 是否额外包含 /sdcard/log/sirunlog 目录
 * @param errMsg         输出参数，返回错误信息
 * @return RET_OK 成功，-1 失败
 */
static int CreateTarArchive(const char *logstorePath,
                            const char *outFilename,
                            char **filteredFiles,
                            int filteredCount,
                            int includeSirunlog,
                            std::string &errMsg)
{
    if (!outFilename || (filteredCount != 0 && !filteredFiles))
    {
        errMsg = std::string("[") + __func__ + "] Invalid arguments";
        return RET_ERR;
    }

    if (filteredCount == 0 && !includeSirunlog)
    {
        // 无待压缩的日志文件，视为失败
        errMsg = std::string("[") + __func__ + "] No files to archive";
        return RET_ERR;
    }

    // 构造命令
    char cmd[8192];
    int written = snprintf(cmd, sizeof(cmd), "tar czf %s", outFilename);
    if (written >= sizeof(cmd))
    {
        errMsg = std::string("[") + __func__ + "] Command buffer too small";
        return RET_ERR;
    }

    for (int i = 0; i < filteredCount; i++)
    {
        size_t remaining = sizeof(cmd) - strlen(cmd) - 1;
        int n = snprintf(cmd + strlen(cmd), remaining, " -C %s %s", logstorePath, filteredFiles[i]);
        if (n < 0 || (size_t)n >= remaining)
        {
            errMsg = std::string("[") + __func__ + "] File list too long for command buffer";
            return RET_ERR;
        }
    }

    if (includeSirunlog)
    {
        size_t remaining = sizeof(cmd) - strlen(cmd) - 1;
        int n = snprintf(cmd + strlen(cmd), remaining, " /sdcard/log/sirunlog");
        if (n < 0 || (size_t)n >= remaining)
        {
            errMsg = std::string("[") + __func__ + "] Failed to append sirunlog path";
            return RET_ERR;
        }
    }

    // 执行压缩归档命令
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    if (result != 0)
    {
        errMsg = std::string("[") + __func__ + "] tar command failed (exit code " + std::to_string(result) + ")";
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @brief 根据时间戳需求压缩归档日志文件
 *
 * @param logstorePath
 * @param startTime
 * @param endTime
 * @param cmdId
 * @param outFilename
 * @param outSize
 * @param errMsg
 * @return int
 */
static int ArchiveLogFiles(const char *logstorePath,
                           long long startTime, long long endTime, const char *cmdId,
                           char *outFilename, size_t outSize, std::string &errMsg)
{
    int ret = RET_ERR;
    char **fileList = NULL;               // 日志目录中的日志文件列表
    int fileCount = 0;                    // 日志文件个数
    long long minMtime = 0, maxMtime = 0; // 日志文件的最小时间戳、最大时间戳

    char **filteredFiles = NULL; // 归档日志文件列表
    int filteredCount = 0;       // 归档日志文件个数
    int includeSirunlog = 0;     // 包含实时日志文件标志位

    if (!logstorePath || !cmdId || !outFilename || outSize == 0)
    {
        errMsg = std::string("[") + __func__ + "] Invalid arguments";
        return RET_ERR;
    }

    printf("[LogUpload] start:%lld end:%lld\n", startTime, endTime);
    snprintf(outFilename, outSize, "%s/%s.tgz", logstorePath, cmdId);

    do
    {
        // 步骤1：筛选出日志文件
        if (CollectTgzFiles(logstorePath, "*.tgz", &fileList, &fileCount,
                            &minMtime, &maxMtime, errMsg) != RET_OK)
        {
            break;
        }
        if (fileCount == 0)
        {
            includeSirunlog = 1;
            break;
        }

        // 步骤2：时间戳筛选及边界处理
        if (FilterFilesByTime(logstorePath, fileList, fileCount,
                              &startTime, endTime,
                              minMtime, maxMtime,
                              &filteredFiles, &filteredCount,
                              &includeSirunlog, errMsg) != RET_OK)
        {
            break;
        }

        // 步骤3：创建归档
        if (CreateTarArchive(logstorePath, outFilename,
                             filteredFiles, filteredCount,
                             includeSirunlog, errMsg) != RET_OK)
        {
            break;
        }

        ret = 0; // 全部成功
    } while (0);

    // 统一释放资源（无论成功或失败）
    for (int i = 0; i < fileCount; i++)
        free(fileList[i]);
    free(fileList);

    for (int i = 0; i < filteredCount; i++)
        free(filteredFiles[i]);
    free(filteredFiles);

    return ret;
}

// 重启类命令实现
// ******************************************************************************************************************************************************//

/**
 * @brief MCU重启
 *
 * @param cmdId
 */
static void HandlerMcuRestart(const std::string &cmdId)
{
    McuMesg_t sendData;
    uint8_t reqData[1] = {0};
    uint16_t index = 0;
    reqData[index++] = 0;
    sendData.payload = reqData;
    sendData.payloadLen = index;
    sendData.type = SYS_QUE_TYPE_MCU_SYNC_REBOOT_MCU_REQ;
    LOG_DebugHex(reqData, index, "HandlerMcuRestart");

    int ret = RET_ERR;
    std::string msg = "restart finish";
    do
    {
#ifdef REBOOT_MCU_CARRY_CPU
        ret = SetStatusFlagToDisk(21, cmdId.c_str());
        if (ret != RET_OK)
        {
            LOG_Error("%s() SetStatusFlagToDisk err", __func__);
            msg = "save rebootCmdACK to disk err";
            break;
        }
        system("sync && sleep 1");
#endif
        /* 通知MCU重启的操作 */
        pthread_mutex_lock(&g_recvedMcuRebootAckMutex);
        ret = McuSrvSendMesg(&sendData);
        if (ret != RET_OK)
        {
            pthread_mutex_unlock(&g_recvedMcuRebootAckMutex);
            LOG_Error("%s() McuSrvSendMesg err", __func__);
            msg = "Send msg to MCU faild";
            break;
        }
#ifndef REBOOT_MCU_CARRY_CPU
        /* 等待接收MCU重启后的回执 */
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += RECV_MCU_MSG_TIMEOUT_MS / 1000;
        ts.tv_nsec += (RECV_MCU_MSG_TIMEOUT_MS % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000)
        {
            // 处理纳秒溢出
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        if (pthread_cond_timedwait(&g_recvedMcuRebootAckCond, &g_recvedMcuRebootAckMutex, &ts) == ETIMEDOUT)
        {
            pthread_mutex_unlock(&g_recvedMcuRebootAckMutex);
            ret = RET_TIMEOUT;
            msg = "recv MCU msg time out";
            break;
        }
        pthread_mutex_unlock(&g_recvedMcuRebootAckMutex);

#endif
        ret = RET_OK;
    } while (0);

    if (ret != RET_OK)
    {
#ifdef REBOOT_MCU_CARRY_CPU
        ResetStatusFlagToDisk(21);
#endif
        printf("[%s] errMsg:%s\n", __func__, msg.c_str());
        SendCommandResponse(cmdId, net::sirun::tboxs3::connect::command::ControlResult::command_failed, 0, msg);
    }
#ifndef REBOOT_MCU_CARRY_CPU
    else
    {
        SendCommandResponse(cmdId, net::sirun::tboxs3::connect::command::ControlResult::success_done, 0, msg);
    }
#endif

    RemoveTaskCmdId(cmdId.c_str());
    return;
}

/**
 * @brief CPU重启
 *
 * @param cmdId
 */
static void HandlerCpuRestart(const std::string &cmdId)
{
    int ret = RET_ERR;
    std::string msg = "restart finish";
    do
    {
        ret = SetStatusFlagToDisk(22, cmdId.c_str());
        if (ret != RET_OK)
        {
            LOG_Error("%s() SetStatusFlagToDisk err", __func__);
            msg = "save rebootCmdACK to disk err";
            break;
        }
        system("sleep 1 && sync && reboot &");
    } while (0);

    if (ret != RET_OK)
    {
        printf("[%s] errMsg:%s\n", __func__, msg.c_str());
        SendCommandResponse(cmdId, net::sirun::tboxs3::connect::command::ControlResult::command_failed, 0, msg);
    }
    return;
}

/**
 * @brief TBOX重启
 *
 * @param cmdId
 */
static void HandlerTboxRestart(const std::string &cmdId)
{
    // MCU重启
    McuMesg_t sendData;
    uint8_t reqData[1] = {0};
    uint16_t index = 0;
    reqData[index++] = 0;
    sendData.payload = reqData;
    sendData.payloadLen = index;
    sendData.type = SYS_QUE_TYPE_MCU_SYNC_REBOOT_MCU_REQ;
    LOG_DebugHex(reqData, index, "HandlerTboxRestart");

    int ret = RET_ERR;
    std::string msg = "restart finish";
    do
    {
#ifdef REBOOT_MCU_CARRY_CPU
        ret = SetStatusFlagToDisk(20, cmdId.c_str());
        if (ret != RET_OK)
        {
            LOG_Error("%s() SetStatusFlagToDisk err", __func__);
            msg = "save rebootCmdACK to disk err";
            break;
        }
        system("sync && sleep 1");
#endif
        /* 通知MCU重启的操作 */
        pthread_mutex_lock(&g_recvedMcuRebootAckMutex);
        ret = McuSrvSendMesg(&sendData);
        if (ret != RET_OK)
        {
            pthread_mutex_unlock(&g_recvedMcuRebootAckMutex);
            LOG_Error("%s() McuSrvSendMesg err", __func__);
            msg = "Send msg to MCU faild";
            break;
        }
#ifndef REBOOT_MCU_CARRY_CPU
        /* 等待接收MCU重启后的回执 */
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += RECV_MCU_MSG_TIMEOUT_MS / 1000;
        ts.tv_nsec += (RECV_MCU_MSG_TIMEOUT_MS % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000)
        {
            // 处理纳秒溢出
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        if (pthread_cond_timedwait(&g_recvedMcuRebootAckCond, &g_recvedMcuRebootAckMutex, &ts) == ETIMEDOUT)
        {
            pthread_mutex_unlock(&g_recvedMcuRebootAckMutex);
            ret = RET_TIMEOUT;
            msg = "recv MCU msg time out";
            break;
        }
        pthread_mutex_unlock(&g_recvedMcuRebootAckMutex);

        // CPU重启
        ret = SetStatusFlagToDisk(20, cmdId.c_str());
        if (ret != RET_OK)
        {
            LOG_Error("%s() SetStatusFlagToDisk err", __func__);
            msg = "save rebootCmdACK to disk err";
            break;
        }
        system("sleep 1 && sync && reboot &");

#endif
        ret = RET_OK;
    } while (0);

    if (ret != RET_OK)
    {
#ifdef REBOOT_MCU_CARRY_CPU
        ResetStatusFlagToDisk(20);
#endif
        printf("[%s] errMsg:%s\n", __func__, msg.c_str());
        SendCommandResponse(cmdId, net::sirun::tboxs3::connect::command::ControlResult::command_failed, 0, msg);
    }
    return;
}

/**
 * @brief 重启类命令在完成重启后重新上电时发送的ACK回执
 *
 * @param cmdId
 */
void HandlerRebootCmdAck(const std::string &cmdId)
{
    SendCommandResponse(cmdId, net::sirun::tboxs3::connect::command::ControlResult::success_done, 0, "restart finish");

    MonitorConfigPath path = {0};
    MonitorConfig_Get(&path);
    char filePath[256] = "";
    snprintf(filePath, sizeof(filePath), "%s%s", path.taskConfigFilePath, "rebootCmdACK");
    remove(filePath);
    return;
}

// 日志上传命令实现
// ******************************************************************************************************************************************************//

/**
 * @brief 上传单文件到日志服务器
 *
 * @param token
 * @param commandId
 * @param sn
 * @param filePath
 * @param errMsg
 * @return int
 */
static int UploadSingleFile(const char *token, const char *commandId,
                            const char *sn, const char *filePath,
                            std::string &errMsg)
{
    int ret = RET_ERR;
    CURL *curl = NULL;
    CURLcode res = CURLE_FAILED_INIT;
    curl_mime *form = NULL;
    cJSON *root = NULL;
    struct MemoryStruct chunk;
    long httpCode = 0;

    // 初始化响应接收缓冲区
    chunk.memory = static_cast<char *>(malloc(1));
    chunk.size = 0;

    curl = curl_easy_init();

    do
    {
        /* 0. 资源初始化检查 */
        if (chunk.memory == NULL)
        {
            errMsg = std::string("[") + __func__ + "] malloc response buffer failed";
            break;
        }
        if (curl == NULL)
        {
            errMsg = std::string("[") + __func__ + "] curl_easy_init() failed";
            break;
        }

        /* 1. 配置基础请求参数 */
        curl_easy_setopt(curl, CURLOPT_URL,
                         "http://121.40.204.137:8312/static-service/api/v1/log/upload/single");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        /* 2. 设置响应接收回调 */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* 3. 超时配置 */
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

        /* 4. 调试日志配置 */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_STDERR, stderr);

        /* 5. 构建 multipart/form-data 表单 */
        form = curl_mime_init(curl);
        if (form == NULL)
        {
            errMsg = std::string("[") + __func__ + "] curl_mime_init failed";
            break;
        }
        // 字段: token
        curl_mimepart *fieldToken = curl_mime_addpart(form);
        curl_mime_name(fieldToken, "token");
        curl_mime_data(fieldToken, token, CURL_ZERO_TERMINATED);
        printf("[%s] field: token = %s\n", __func__, token);
        // 字段: commandId
        curl_mimepart *fieldCmdId = curl_mime_addpart(form);
        curl_mime_name(fieldCmdId, "commandId");
        curl_mime_data(fieldCmdId, commandId, CURL_ZERO_TERMINATED);
        printf("[%s] field: commandId = %s\n", __func__, commandId);
        // 字段: sn
        curl_mimepart *fieldSn = curl_mime_addpart(form);
        curl_mime_name(fieldSn, "sn");
        curl_mime_data(fieldSn, sn, CURL_ZERO_TERMINATED);
        printf("[%s] field: sn = %s\n", __func__, sn);
        // 字段: file
        curl_mimepart *field = curl_mime_addpart(form);
        curl_mime_name(field, "file");
        curl_mime_filedata(field, filePath);
        printf("[%s] field: file = %s\n", __func__, filePath);
        // 绑定表单到请求
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        /* 6. 执行 POST 请求 */
        printf("[%s] Before curl_easy_perform\n", __func__);
        fflush(stdout);
        res = curl_easy_perform(curl);
        printf("[%s] After curl_easy_perform\n", __func__);
        fflush(stdout);

        /* 7. 响应检查 */
        if (res != CURLE_OK)
        {
            errMsg = std::string("[") + __func__ + "] curl_easy_perform failed: " +
                     curl_easy_strerror(res) + " (code " + std::to_string((int)res) + ")";
            break;
        }
        // 校验 HTTP 状态码
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        printf("[%s] HTTP response code: %ld\n", __func__, httpCode);
        if (httpCode < 200 || httpCode >= 300)
        {
            errMsg = std::string("[") + __func__ + "] Upload failed with HTTP " + std::to_string(httpCode);
            break;
        }
        // 解析响应 JSON 并校验业务结果
        printf("[%s] Server Response: %s\n", __func__, chunk.memory ? chunk.memory : "(null)");
        LOG_Debug("[%s] Server Response: %s", __func__, chunk.memory ? chunk.memory : "(null)");
        root = cJSON_Parse(chunk.memory);
        if (root == NULL)
        {
            const char *error_ptr = cJSON_GetErrorPtr();
            errMsg = std::string("[") + __func__ + "] JSON parse error";
            if (error_ptr != NULL)
            {
                errMsg += " before: " + std::string(error_ptr);
            }
            break;
        }
        cJSON *success_item = cJSON_GetObjectItem(root, "success");
        if (success_item == NULL)
        {
            errMsg = std::string("[") + __func__ + "] JSON missing 'success' field";
            break;
        }
        if (!cJSON_IsTrue(success_item))
        {
            errMsg = std::string("[") + __func__ + "] success is not true";
            break;
        }
        ret = RET_OK;

    } while (0);

    /* 统一资源销毁 */
    if (root != NULL)
        cJSON_Delete(root);
    if (form != NULL)
        curl_mime_free(form);
    if (curl != NULL)
        curl_easy_cleanup(curl);
    if (chunk.memory != NULL)
        free(chunk.memory);

    return ret;
}

/**
 * @brief 初始化分片上传
 */
static int InitMultipartUpload(const char *token, const char *commandId,
                               const char *sn, const char *archiveFile,
                               std::string &uploadId, uint64_t &partSize,
                               std::string &errMsg)
{
    int ret = RET_ERR;
    CURL *curl = NULL;
    CURLcode res = CURLE_FAILED_INIT;
    curl_mime *form = NULL;
    cJSON *root = NULL;
    struct MemoryStruct chunk;
    long httpCode = 0;

    // 初始化响应接收缓冲区
    chunk.memory = static_cast<char *>(malloc(1));
    chunk.size = 0;

    curl = curl_easy_init();

    do
    {
        /* 0. 资源初始化检查 */
        if (chunk.memory == NULL)
        {
            errMsg = std::string("[") + __func__ + "] malloc response buffer failed";
            break;
        }
        if (curl == NULL)
        {
            errMsg = std::string("[") + __func__ + "] curl_easy_init() failed";
            break;
        }

        /* 1. 请求 URL */
        curl_easy_setopt(curl, CURLOPT_URL,
                         "http://121.40.204.137:8312/static-service/api/v1/log/upload/multipart/init");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        /* 2. 设置响应接收回调 */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* 3. 超时配置 */
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        /* 4. 构建 multipart/form-data 表单 */
        form = curl_mime_init(curl);
        if (form == NULL)
        {
            errMsg = std::string("[") + __func__ + "] curl_mime_init failed";
            break;
        }
        // 字段: token
        curl_mimepart *fieldToken = curl_mime_addpart(form);
        curl_mime_name(fieldToken, "token");
        curl_mime_data(fieldToken, token, CURL_ZERO_TERMINATED);
        // 字段: commandId
        curl_mimepart *fieldCmdId = curl_mime_addpart(form);
        curl_mime_name(fieldCmdId, "commandId");
        curl_mime_data(fieldCmdId, commandId, CURL_ZERO_TERMINATED);
        // 字段: sn
        curl_mimepart *fieldSn = curl_mime_addpart(form);
        curl_mime_name(fieldSn, "sn");
        curl_mime_data(fieldSn, sn, CURL_ZERO_TERMINATED);
        // 字段: fileName
        curl_mimepart *fieldFileName = curl_mime_addpart(form);
        curl_mime_name(fieldFileName, "fileName");
        curl_mime_data(fieldFileName, GetFileName(archiveFile), CURL_ZERO_TERMINATED);
        // 绑定表单到请求
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        /* 5. 执行 POST 请求 */
        res = curl_easy_perform(curl);

        /* 6. 响应检查 */
        if (res != CURLE_OK)
        {
            errMsg = std::string("[") + __func__ + "] curl_easy_perform failed: " + curl_easy_strerror(res);
            break;
        }
        // 校验 HTTP 状态码
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        printf("[%s] HTTP response code: %ld\n", __func__, httpCode);
        if (httpCode < 200 || httpCode >= 300)
        {
            errMsg = std::string("[") + __func__ + "] upload init failed with HTTP " + std::to_string(httpCode);
            break;
        }
        // 解析 JSON 提取 uploadId 和 partSize
        printf("[%s] Server Response: %s\n", __func__, chunk.memory ? chunk.memory : "(null)");
        LOG_Debug("[%s] Server Response: %s", __func__, chunk.memory ? chunk.memory : "(null)");
        root = cJSON_Parse(chunk.memory);
        if (root == NULL)
        {
            errMsg = std::string("[") + __func__ + "] cJSON_Parse failed";
            break;
        }
        cJSON *data = cJSON_GetObjectItem(root, "data");
        if (data == NULL || !cJSON_IsObject(data))
        {
            errMsg = std::string("[") + __func__ + "] missing 'data' object";
            break;
        }
        cJSON *uploadIdItem = cJSON_GetObjectItem(data, "uploadId");
        cJSON *partSizeItem = cJSON_GetObjectItem(data, "partSize");
        if (uploadIdItem == NULL || partSizeItem == NULL || !cJSON_IsString(uploadIdItem))
        {
            errMsg = std::string("[") + __func__ + "] missing uploadId";
            break;
        }
        if (cJSON_IsNumber(partSizeItem))
            partSize = (uint64_t)partSizeItem->valueint;
        else if (cJSON_IsString(partSizeItem))
            partSize = (uint64_t)atoll(partSizeItem->valuestring);
        else
        {
            errMsg = std::string("[") + __func__ + "] partSize invalid type";
            break;
        }
        uploadId = uploadIdItem->valuestring;

        ret = RET_OK;

    } while (0);

    /* 统一资源销毁 */
    if (root != NULL)
        cJSON_Delete(root);
    if (form != NULL)
        curl_mime_free(form);
    if (curl != NULL)
        curl_easy_cleanup(curl);
    if (chunk.memory != NULL)
        free(chunk.memory);

    return ret;
}

/**
 * @brief 上传单个分片
 */
static int UploadPart(const char *token, const char *uploadId, int partNumber,
                      const char *archiveFile, const void *data,
                      size_t dataSize, std::string &etag,
                      std::string &errMsg)
{
    int ret = RET_ERR;
    CURL *curl = NULL;
    CURLcode res = CURLE_FAILED_INIT;
    curl_mime *form = NULL;
    cJSON *root = NULL;
    struct MemoryStruct chunk;
    long httpCode = 0;

    // 初始化响应接收缓冲区
    chunk.memory = static_cast<char *>(malloc(1));
    chunk.size = 0;

    curl = curl_easy_init();

    do
    {
        /* 0. 资源初始化检查 */
        if (chunk.memory == NULL)
        {
            errMsg = std::string("[") + __func__ + "] malloc response buffer failed";
            break;
        }
        if (curl == NULL)
        {
            errMsg = std::string("[") + __func__ + "] curl_easy_init() failed";
            break;
        }

        /* 1. 请求 URL */
        curl_easy_setopt(curl, CURLOPT_URL,
                         "http://121.40.204.137:8312/static-service/api/v1/log/upload/multipart/part");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        /* 2. 设置响应接收回调 */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* 3. 超时配置 */
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

        /* 4. 构建 multipart/form-data 表单 */
        form = curl_mime_init(curl);
        if (form == NULL)
        {
            errMsg = std::string("[") + __func__ + "] curl_mime_init failed";
            break;
        }
        // 字段: token
        curl_mimepart *fieldToken = curl_mime_addpart(form);
        curl_mime_name(fieldToken, "token");
        curl_mime_data(fieldToken, token, CURL_ZERO_TERMINATED);
        // 字段: uploadId
        curl_mimepart *fieldUploadId = curl_mime_addpart(form);
        curl_mime_name(fieldUploadId, "uploadId");
        curl_mime_data(fieldUploadId, uploadId, CURL_ZERO_TERMINATED);
        // 字段: partNumber
        char partNumStr[16] = {0};
        snprintf(partNumStr, sizeof(partNumStr), "%d", partNumber);
        curl_mimepart *fieldPartNum = curl_mime_addpart(form);
        curl_mime_name(fieldPartNum, "partNumber");
        curl_mime_data(fieldPartNum, partNumStr, CURL_ZERO_TERMINATED);
        // 字段: file
        curl_mimepart *fieldFile = curl_mime_addpart(form);
        curl_mime_name(fieldFile, "file");
        curl_mime_data(fieldFile, (const char *)data, dataSize);
        curl_mime_filename(fieldFile, GetFileName(archiveFile));
        // 绑定表单到请求
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        /* 5. 执行 POST 请求 */
        res = curl_easy_perform(curl);

        /* 6. 响应检查 */
        if (res != CURLE_OK)
        {
            errMsg = std::string("[") + __func__ + "] curl_easy_perform failed: " + curl_easy_strerror(res);
            break;
        }
        // 校验 HTTP 状态码
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        printf("[%s] HTTP response code: %ld\n", __func__, httpCode);
        if (httpCode < 200 || httpCode >= 300)
        {
            errMsg = std::string("[") + __func__ + "] upload part failed with HTTP " + std::to_string(httpCode);
            break;
        }
        // 解析 JSON 提取 etag
        printf("[%s] Server Response: %s\n", __func__, chunk.memory ? chunk.memory : "(null)");
        LOG_Debug("[%s] Server Response: %s", __func__, chunk.memory ? chunk.memory : "(null)");
        root = cJSON_Parse(chunk.memory);
        if (root == NULL)
        {
            errMsg = std::string("[") + __func__ + "] cJSON_Parse failed";
            break;
        }
        cJSON *dataItem = cJSON_GetObjectItem(root, "data");
        if (dataItem == NULL || !cJSON_IsObject(dataItem))
        {
            errMsg = std::string("[") + __func__ + "] missing 'data' object";
            break;
        }
        cJSON *etagItem = cJSON_GetObjectItem(dataItem, "etag");
        if (etagItem == NULL || !cJSON_IsString(etagItem))
        {
            errMsg = std::string("[") + __func__ + "] missing 'etag' field";
            break;
        }

        etag = etagItem->valuestring;

        ret = RET_OK;

    } while (0);

    /* 统一资源销毁 */
    if (root != NULL)
        cJSON_Delete(root);
    if (form != NULL)
        curl_mime_free(form);
    if (curl != NULL)
        curl_easy_cleanup(curl);
    if (chunk.memory != NULL)
        free(chunk.memory);

    return ret;
}

/**
 * @brief 根据 ctx 信息，获取并上传单个分片的数据
 *
 * @param token         鉴权令牌
 * @param archiveFile   待上传文件的完整路径
 * @param uploadId      服务器返回的分片上传任务ID
 * @param ctx           分片上传所需数据
 * @param errMsg        输出错误信息
 * @return RET_OK 成功，RET_ERR 失败
 */
int UploadSlice(const char *token,
                const char *archiveFile,
                const std::string &uploadId,
                SliceUploadContext &ctx,
                std::string &errMsg)
{
    // 首次调用：打开文件并初始化状态
    if (ctx.fp == nullptr)
    {
        struct stat st;
        if (stat(archiveFile, &st) != 0)
        {
            errMsg = std::string("[") + __func__ + "] stat failed: " + strerror(errno);
            return RET_ERR;
        }
        long fileSize = st.st_size;
        if (fileSize == 0)
        {
            ctx.finished = true;
            return RET_OK; // 空文件，直接标记完成
        }

        ctx.fp = fopen(archiveFile, "rb");
        if (!ctx.fp)
        {
            errMsg = std::string("[") + __func__ + "] fopen failed: " + strerror(errno);
            return RET_ERR;
        }
        ctx.remaining = fileSize;
        ctx.partNumber = 1;
        ctx.buffer.resize(ctx.partSize);
    }

    // 读取一个分片
    size_t readSize = (ctx.remaining >= (long)ctx.partSize) ? (size_t)ctx.partSize : (size_t)ctx.remaining;
    size_t actualRead = fread(ctx.buffer.data(), 1, readSize, ctx.fp);
    if (actualRead != readSize)
    {
        errMsg = std::string("[") + __func__ + "] fread failed";
        fclose(ctx.fp);
        ctx.fp = nullptr;
        return RET_ERR;
    }

    // 上传该分片
    std::string etag;
    int ret = UploadPart(token, uploadId.c_str(), ctx.partNumber,
                         archiveFile, ctx.buffer.data(), actualRead, etag, errMsg);
    if (ret != RET_OK)
    {
        fclose(ctx.fp);
        ctx.fp = nullptr;
        return RET_ERR;
    }
    ctx.etags.push_back(etag);

    // 更新状态
    ctx.remaining -= actualRead;
    ctx.partNumber++;
    if (ctx.remaining <= 0)
    {
        ctx.finished = true;
        fclose(ctx.fp);
        ctx.fp = nullptr;
    }
    return RET_OK;
}

/**
 * @brief 合并分片完成上传
 */
static int CompleteMultipartUpload(const char *token, const char *uploadId,
                                   const std::vector<std::string> &etags,
                                   std::string &errMsg)
{
    int ret = RET_ERR;
    CURL *curl = NULL;
    CURLcode res = CURLE_FAILED_INIT;
    curl_mime *form = NULL;
    cJSON *root = NULL;
    struct MemoryStruct chunk;
    long httpCode = 0;

    // 初始化响应接收缓冲区
    chunk.memory = static_cast<char *>(malloc(1));
    chunk.size = 0;

    curl = curl_easy_init();

    do
    {
        /* 0. 资源初始化检查 */
        if (chunk.memory == NULL)
        {
            errMsg = std::string("[") + __func__ + "] malloc response buffer failed";
            break;
        }
        if (curl == NULL)
        {
            errMsg = std::string("[") + __func__ + "] curl_easy_init() failed";
            break;
        }

        /* 1. 请求 URL */
        curl_easy_setopt(curl, CURLOPT_URL,
                         "http://121.40.204.137:8312/static-service/api/v1/log/upload/multipart/complete");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        /* 2. 设置响应接收回调 */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* 3. 超时配置 */
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        /* 4. 构建 multipart/form-data 表单 */
        form = curl_mime_init(curl);
        if (form == NULL)
        {
            errMsg = std::string("[") + __func__ + "] curl_mime_init failed";
            break;
        }
        // 构造 parts 的 JSON 字符串
        std::string jsonParts = "[";
        for (size_t i = 0; i < etags.size(); i++)
        {
            if (i > 0)
                jsonParts += ",";
            jsonParts += "{\"partNumber\":" + std::to_string(i + 1) +
                         ",\"etag\":\"" + etags[i] + "\"}";
        }
        jsonParts += "]";
        // 字段: token
        curl_mimepart *fieldToken = curl_mime_addpart(form);
        curl_mime_name(fieldToken, "token");
        curl_mime_data(fieldToken, token, CURL_ZERO_TERMINATED);
        // 字段: uploadId
        curl_mimepart *fieldUploadId = curl_mime_addpart(form);
        curl_mime_name(fieldUploadId, "uploadId");
        curl_mime_data(fieldUploadId, uploadId, CURL_ZERO_TERMINATED);
        // 字段: parts
        curl_mimepart *fieldParts = curl_mime_addpart(form);
        curl_mime_name(fieldParts, "parts");
        curl_mime_data(fieldParts, jsonParts.c_str(), jsonParts.length());
        // 绑定表单到请求
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        /* 5. 执行 POST 请求 */
        res = curl_easy_perform(curl);

        /* 6. 响应检查 */
        if (res != CURLE_OK)
        {
            errMsg = std::string("[") + __func__ + "] curl_easy_perform failed: " + curl_easy_strerror(res);
            break;
        }
        // 校验 HTTP 状态码
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        printf("[%s] HTTP response code: %ld\n", __func__, httpCode);
        if (httpCode < 200 || httpCode >= 300)
        {
            errMsg = std::string("[") + __func__ + "] complete failed with HTTP " + std::to_string(httpCode);
            break;
        }
        printf("[%s] Server Response: %s\n", __func__, chunk.memory ? chunk.memory : "(null)");
        LOG_Debug("[%s] Server Response: %s", __func__, chunk.memory ? chunk.memory : "(null)");
        // 解析响应 JSON 并校验业务结果
        root = cJSON_Parse(chunk.memory);
        if (root != NULL)
        {
            cJSON *success_item = cJSON_GetObjectItem(root, "success");
            if (success_item != NULL && cJSON_IsTrue(success_item))
            {
                ret = RET_OK;
            }
            else
            {
                errMsg = std::string("[") + __func__ + "] success is not true or missing";
            }
        }
        else
        {
            errMsg = std::string("[") + __func__ + "] cJSON_Parse failed";
        }
    } while (0);

    /* 统一资源销毁 */
    if (root != NULL)
        cJSON_Delete(root);
    if (form != NULL)
        curl_mime_free(form);
    if (curl != NULL)
        curl_easy_cleanup(curl);
    if (chunk.memory != NULL)
        free(chunk.memory);

    return ret;
}

/**
 * @brief 取消分片上传
 */
static int CancelMultipartUpload(const char *token, const char *uploadId)
{
    int ret = RET_ERR;
    CURL *curl = NULL;
    CURLcode res = CURLE_FAILED_INIT;
    curl_mime *form = NULL;
    struct MemoryStruct chunk;
    long httpCode = 0;

    // 初始化响应接收缓冲区
    chunk.memory = static_cast<char *>(malloc(1));
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    do
    {
        /* 0. 资源初始化检查 */
        if (chunk.memory == NULL)
        {
            fprintf(stderr, "[%s] malloc response buffer failed\n", __func__);
            break;
        }
        if (curl == NULL)
        {
            fprintf(stderr, "[%s] curl_easy_init() failed\n", __func__);
            break;
        }

        /* 1. 请求 URL */
        curl_easy_setopt(curl, CURLOPT_URL,
                         "http://121.40.204.137:8312/static-service/api/v1/log/upload/multipart/cancel");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        /* 2. 设置响应接收回调 */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* 3. 超时配置 */
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        /* 4. 构建 multipart/form-data 表单 */
        form = curl_mime_init(curl);
        if (form == NULL)
        {
            fprintf(stderr, "[%s] curl_mime_init failed\n", __func__);
            break;
        }
        // 字段: token
        curl_mimepart *fieldToken = curl_mime_addpart(form);
        curl_mime_name(fieldToken, "token");
        curl_mime_data(fieldToken, token, CURL_ZERO_TERMINATED);
        // 字段: uploadId
        curl_mimepart *fieldUploadId = curl_mime_addpart(form);
        curl_mime_name(fieldUploadId, "uploadId");
        curl_mime_data(fieldUploadId, uploadId, CURL_ZERO_TERMINATED);
        // 绑定表单到请求
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        /* 5. 执行 POST 请求 */
        res = curl_easy_perform(curl);

        /* 6. 响应检查 */
        if (res != CURLE_OK)
        {
            fprintf(stderr, "[%s] curl_easy_perform failed: %s\n", __func__, curl_easy_strerror(res));
            break;
        }
        // 校验 HTTP 状态码
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        printf("[%s] HTTP response code: %ld\n", __func__, httpCode);
        if (httpCode < 200 || httpCode >= 300)
        {
            fprintf(stderr, "[%s] cancel failed with HTTP %ld\n", __func__, httpCode);
            break;
        }
        printf("[%s] Server Response: %s\n", __func__, chunk.memory ? chunk.memory : "(null)");
        LOG_Debug("[%s] Server Response: %s", __func__, chunk.memory ? chunk.memory : "(null)");

        ret = RET_OK;
    } while (0);

    /* 统一资源销毁 */
    if (form != NULL)
        curl_mime_free(form);
    if (curl != NULL)
        curl_easy_cleanup(curl);
    if (chunk.memory != NULL)
        free(chunk.memory);

    return ret;
}

/**
 * @brief 日志上传
 *
 * @param cmdId
 * @param params
 */
void HandleLogUpload(TaskArgs *ta, const net::sirun::tboxs3::connect::command::LogCollectParams &params)
{
    int ret = RET_ERR;
    bool taskFinishFlag = false; // 任务完成标志位
    bool taskCanceled = false;   // 任务被取消标志位
    int steps = 0;               // 任务步骤

    char archiveFile[128] = ""; // 归档文件路径

    std::string uploadId;          // 服务器分配的上传任务ID
    std::string failReason = "OK"; // 报错原因
    UploadMode uploadMode = UPLOAD_MODE_SINGLE;

    SliceUploadContext sliceCtx;

    while (!taskFinishFlag)
    {
        if (IsCanceledTask(ta->cmdId.c_str()))
        {
            taskCanceled = true;
            steps = -1; // 跳过任务步骤
            failReason = "Task cancelled";
        }

        switch (steps)
        {
        case 0: /* 日志文件归档 */
        {
            LOG_Debug("[%s] steps:%d", __func__, steps);
            ret = ArchiveLogFiles("/sdcard/log/logstore",
                                  params.start_time(), params.end_time(), ta->cmdId.c_str(),
                                  archiveFile, sizeof(archiveFile), failReason);
            if (ret == RET_OK)
            {
                printf("Created package: %s\n", archiveFile);
                steps++;
            }
            break;
        }
        case 1: /* 上报方式选择(单文件上传/分片上传) */
        {
            LOG_Debug("[%s] steps:%d", __func__, steps);
            struct stat st;
            if (stat(archiveFile, &st) == 0)
            {
                steps = st.st_size > SINGLE_FILE_UPLOAD_THRESHOLD ? 3 : 2;
            }
            else
            {
                // stat失败的处理逻辑
                steps = 2; // 默认单文件上传
            }
            uploadMode = (3 == steps ? UPLOAD_MODE_MULTIPART : UPLOAD_MODE_SINGLE);
            break;
        }
        case 2: /* 单个日志文件上传 */
        {
            LOG_Debug("[%s] steps:%d", __func__, steps);
            ret = UploadSingleFile(params.token().c_str(), ta->cmdId.c_str(), ta->sn.c_str(),
                                   archiveFile, failReason);
            taskFinishFlag = true;
            break;
        }
        case 3: /* 分片上传-初始化 */
        {
            LOG_Debug("[%s] steps:%d", __func__, steps);
            ret = InitMultipartUpload(params.token().c_str(), ta->cmdId.c_str(), ta->sn.c_str(),
                                      archiveFile, uploadId, sliceCtx.partSize, failReason);
            if (ret == RET_OK)
                steps++;
            break;
        }
        case 4: /* 分片上传-上传单个分片 */
        {
            LOG_Debug("[%s] steps:%d-%d", __func__, steps, sliceCtx.partNumber);
            ret = UploadSlice(params.token().c_str(), archiveFile,
                              uploadId, sliceCtx, failReason);
            if (ret != RET_OK)
                break;

            if (sliceCtx.finished)
                // 所有分片均已上传
                steps++;
            else
                // 还有分片未上传
                continue;

            break;
        }
        case 5: /* 分片上传-3：合并分片 */
        {
            LOG_Debug("[%s] steps:%d", __func__, steps);
            ret = CompleteMultipartUpload(params.token().c_str(), uploadId.c_str(),
                                          sliceCtx.etags, failReason);
            taskFinishFlag = true;
            break;
        }
        default:
            break;
        }

        if (taskFinishFlag || ret != RET_OK || taskCanceled)
        {
            LOG_Debug("[%s] taskFinishFlag:%d||ret:%d != OK||taskCanceled:%d, CleanCacheFile",
                      __func__, taskFinishFlag, ret, taskCanceled);
            /* 资源销毁 */
            CleanCacheFile(archiveFile);
            if (UPLOAD_MODE_MULTIPART == uploadMode && (ret != RET_OK || taskCanceled) && !uploadId.empty())
            {
                // 在选择分片上报且完成初始化并获取到非空 uploadId 的前提下，上报失败或任务被取消时，均发送取消分片上传
                LOG_Debug("[%s] uploadMode:%d||ret:%d != OK||!uploadId.empty():%d, CancelMultipartUpload",
                          __func__, uploadMode, ret, !uploadId.empty());
                printf("[LogUpload] Multipart upload failed, calling CancelMultipartUpload for uploadId: %s\n", uploadId.c_str());
                CancelMultipartUpload(params.token().c_str(), uploadId.c_str());
            }
        }

        if (taskCanceled)
        {
            // 任务取消后的善后操作：发送"任务已被取消"的回执
            SendCommandResponse(ta->cmdId, net::sirun::tboxs3::connect::command::ControlResult::command_failed, -1, failReason);
            RemoveCanceledTask(ta->cmdId.c_str());
            return;
        }
        if (ret != RET_OK)
        {
            std::string msg = "Failure at steps:" + std::to_string(steps) +
                              ", reason:" + failReason;
            printf("[%s] errMsg:%s\n", __func__, msg.c_str());
            SendCommandResponse(ta->cmdId, net::sirun::tboxs3::connect::command::ControlResult::command_failed, 0, msg);
            return;
        }
    }

    // 全部完成
    SendCommandResponse(ta->cmdId, net::sirun::tboxs3::connect::command::ControlResult::success_done, 0, "Log uploaded");
}

// 任务线程
// ******************************************************************************************************************************************************//

/**
 * @brief MCU重启线程入口
 *
 * @param arg
 * @return void*
 */
void McuRestartTaskThread(void *arg)
{
    prctl(PR_SET_NAME, "McuRestart");
    TaskArgs *ta = static_cast<TaskArgs *>(arg);

    HandlerMcuRestart(ta->cmdId);

    RemoveTaskCmdId(ta->cmdId.c_str());
    delete ta;

    return;
}

/**
 * @brief CPU重启线程入口
 *
 * @param arg
 * @return void*
 */
void CpuRestartTaskThread(void *arg)
{
    prctl(PR_SET_NAME, "CpuRestart");
    TaskArgs *ta = static_cast<TaskArgs *>(arg);

    HandlerCpuRestart(ta->cmdId);

    RemoveTaskCmdId(ta->cmdId.c_str());
    delete ta;

    return;
}

/**
 * @brief TBOX重启线程入口
 *
 * @param arg
 * @return void*
 */
void TboxRestartTaskThread(void *arg)
{
    prctl(PR_SET_NAME, "TboxRestart");
    TaskArgs *ta = static_cast<TaskArgs *>(arg);

    HandlerTboxRestart(ta->cmdId);

    RemoveTaskCmdId(ta->cmdId.c_str());
    delete ta;

    return;
}

/**
 * @brief 日志上传线程入口
 *
 * @param arg
 * @return void*
 */
void LogUploadTaskThread(void *arg)
{
    prctl(PR_SET_NAME, "LogUpload");
    TaskArgs *ta = static_cast<TaskArgs *>(arg);

    HandleLogUpload(static_cast<TaskArgs *>(arg), ta->params);

    RemoveTaskCmdId(ta->cmdId.c_str());
    delete ta;

    return;
}
