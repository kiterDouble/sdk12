#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <string>
#include <mutex>
#include <sys/stat.h> 
#include "ota_master_log.h"
#include "ota_master_archive_manage.h"
#include "ota_master_curl_file_client.h"
#include "ota_master_utils.h"
#include "ota_master_data.h"

static constexpr const char *LOG_FILE_NAME_PREFIX = "ota_master_log";
static constexpr const char *LOG_FILE_FORMAT = ".log";
static constexpr const char *LOG_BACKUP_FILE_FORMAT = ".bak";
static constexpr uint64_t MAX_LOG_FILE_SIZE = 100ULL * 1024 * 1024;

static std::mutex g_logMutex;
static uint16_t g_maxLogLineSize = 0;
static std::string g_logTag;
static std::string g_logFileDir;
static std::string g_logFilePath;
static std::ofstream g_logFileOs;
static bool g_isLogInitialized = false;

static OtaLogErrorCode OpenOtaLogFile(void);

/**
 * 获取日志输出时间（精度：毫秒级）
 * 
 * 输出格式 00:00:00.000（时:分:秒.毫秒）
 *
 * @param 无
 *
 * @return 日志输出时间
 */
static std::string GetCurrentLoggerTime(void) {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    struct tm tmInfo;
    if (localtime_r(&t, &tmInfo) == nullptr) {
        // 若 localtime_r 失败，则返回默认时间
        return "00:00:00.000";
    }

    char timeBuffer[13] = {0};
    // 获取毫秒级时间
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    // 格式化并填充到 buffer
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d.%03d", tmInfo.tm_hour, tmInfo.tm_min, tmInfo.tm_sec,
             static_cast<int>(milliseconds.count()));
    return std::string(timeBuffer);
}

/**
 * 获取文件上传时间
 * 
 * 输出格式 2101010000（年月日时分）
 * 
 * @param 无
 * 
 * @return 文件上传时间
 */
static std::string GetFileUploadDateTime(void)
{
    time_t now = time(nullptr);
    struct tm tmInfo;
    localtime_r(&now, &tmInfo);
    if (localtime_r(&now, &tmInfo) == nullptr) {
        return "2501010000";
    }
    char timeBuf[16] = {0};
    strftime(timeBuf, sizeof(timeBuf), "%y%m%d%H%M", &tmInfo);
    return std::string(timeBuf);
}

/**
 * 日志等级转换为对等字符串
 * 
 * @param[in] level 日志输出等级
 *
 * @return 日志输出等级字符串形式
 */
static std::string LogLevelToString(OtaMasterLogLevel level)
{
    switch (level) {
        case OTA_LOG_DEBUG:
            return "DEBUG";
        case OTA_LOG_INFO:
            return "INFO";
        case OTA_LOG_WARN:
            return "WARN";
        case OTA_LOG_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

/**
 * 检查日志文件大小，没超过阈值不作任何处理，超过阈值则备份并重新创建一个新的空日志文件
 * 
 * @return 成功返回OTA_LOG_OK，失败返回对应错误码
 */
static void CheckIfLogFileSizeExceedsUpperLimit()
{
    // 检测文件是否存在
    if (access(g_logFilePath.c_str(), F_OK) != 0){
        perror("Ota log file not exist, can not backup");
        return;
    }

    // 检查文件是否存在并获取大小
    size_t fileSize = 0;
    struct stat info = {0};
    if (stat(g_logFilePath.c_str(), &info) == 0 && S_ISREG(info.st_mode)) {
        fileSize = info.st_size;
    }

    if (fileSize < MAX_LOG_FILE_SIZE) {
        return;
    }

    if (g_logFileOs.is_open()) {
        g_logFileOs.close();
    }

    std::string backupSuffix = "_" + GetFileUploadDateTime() + LOG_BACKUP_FILE_FORMAT;
    std::string backupLogFilePath = g_logFilePath + backupSuffix;
    if (rename(g_logFilePath.c_str(), backupLogFilePath.c_str()) != 0) {
        perror("rename ota log file failed");
        return;
    }
    (void)OpenOtaLogFile();
}

/**
 * 打开日志存储文件，若文件不存在则创建
 */
static OtaLogErrorCode OpenOtaLogFile(void)
{
    if (!g_isLogInitialized) {
        perror("Ota log uninitialized");
        return OTA_LOG_NOT_INITIALIZES;
    }

    if (g_logFileOs.is_open()) {
        return OTA_LOG_OK;
    }

    g_logFileOs.open(g_logFilePath.c_str(), std::ios::app);
    if (!g_logFileOs.is_open()) {
        perror("Failed to create log file\n");
        g_isLogInitialized = false;
        g_logFilePath = "";
        return OTA_LOG_FILE_CREATION_FAILED;
    }
    return OTA_LOG_OK;
}

/**
 * 备份日志文件
 * 
 * @param 无
 *
 * @return  无
 */
static void BackupOtaLogFileIfNeeded(void)
{
    OTA_LOG_D("Backup ota log file");
    std::lock_guard<std::mutex> locker(g_logMutex);
    if (!g_isLogInitialized) {
        perror("Ota log uninitialized, can not backup");
        return;
    }

    if (access(g_logFilePath.c_str(), F_OK) != 0){
        perror("Ota log file not exist, can not backup");
        return;
    }

    if (g_logFileOs.is_open()) {
        g_logFileOs.close();
    }

    std::string backupSuffix = "_" + GetFileUploadDateTime() + LOG_BACKUP_FILE_FORMAT;
    std::string backupLogFilePath = g_logFilePath + backupSuffix;
    
    if (rename(g_logFilePath.c_str(), backupLogFilePath.c_str()) != 0) {
        perror("rename ota log file failed");
        return;
    }
}

/**
 * 上传日志文件响应回调函数
 * 
 * @param[in] buffer 数据缓冲区
 * @param[in] size 数据块大小
 * @param[in] nmemb 数据块数量
 * @param[in] userp 用户数据
 * 
 * @return 数据大小
 */
static size_t LogUploadWriteCallback(void *buffer, size_t size, size_t nmemb, void *userp)
{
    std::string *response = static_cast<std::string *>(userp);
    if (response == nullptr) {
        OTA_LOG_E("Log upload response is null");
        return size * nmemb;
    }
    response->append(static_cast<char *>(buffer), size * nmemb);
    return size * nmemb;
}

/**
 * 上传日志文件进度回调函数
 * 
 * @param[in] clientp 用户数据
 * @param[in] dltotal 下载总大小
 * @param[in] dlnow 当前下载大小
 * @param[in] ultotal 上传总大小
 * @param[in] ulnow 当前上传大小
 * 
 * @return 0
 */
static int LogUploadProgressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    if (ultotal <= 0) {
        return 0;
    }
    OTA_LOG_D("Uploaded: %d", (curl_off_t)((ulnow * 100.0) / ultotal));
    return 0;
}

/**
 * 复制OTA日志文件到上传文件
 * 
 * @param 无
 * 
 * @return 上传文件路径
 */
static std::string CopyOtaLogFileToUploadFile(void)
{
    std::string uploadDate = std::move(GetFileUploadDateTime());
    std::string uploadFilePath = g_logFileDir.back() == '/'
                                     ? g_logFileDir + LOG_FILE_NAME_PREFIX + "_" + uploadDate + LOG_FILE_FORMAT
                                     : g_logFileDir + "/" + LOG_FILE_NAME_PREFIX + "_" + uploadDate + LOG_FILE_FORMAT;
    int result = 0;
    std::string command;
    {
        std::lock_guard<std::mutex> locker(g_logMutex);
        command = "cp \"" + g_logFilePath + "\" \"" + uploadFilePath + "\"";
        result = std::system(command.c_str());
    }

    if (result) {
        OTA_LOG_E("Copy log file failed, command:%s, result:%d", command.c_str(), result);
        return "";
    }
    return uploadFilePath;
}

OtaLogErrorCode OtaMasterLogInit(const char *tag, const char *logDir)
{
    (void)tag;     // tag参数不再使用，固定使用"otaMasterApi"
    (void)logDir;  // logDir参数不再使用，日志路径由log.conf配置

    std::lock_guard<std::mutex> locker(g_logMutex);
    if (g_isLogInitialized) {
        return OTA_LOG_ALREADY_INITIALIZED;
    }

    // 使用zlog初始化日志
    LOG_INIT("otaMasterApi");
    g_logTag = "otaMasterApi";
    g_isLogInitialized = true;
    return OTA_LOG_OK;
}

void OtaMasterLogFinish(void)
{
    OTA_LOG_D("Finish ota log file");
    std::lock_guard<std::mutex> locker(g_logMutex);
    if (!g_isLogInitialized) {
        return;
    }

   
    g_isLogInitialized = false;
}

OtaLogErrorCode OtaMasterLogReset(void)
{
    OTA_LOG_D("Reset ota log file");
    BackupOtaLogFileIfNeeded();
    std::lock_guard<std::mutex> locker(g_logMutex);
    return OpenOtaLogFile();
}

void OtaMasterUploadLog(const char *uploadUrl, const char *taskId)
{
    {
        std::lock_guard<std::mutex> locker(g_logMutex);
        if (!g_isLogInitialized) {
            perror("Ota log uninitialized, can not upload");
            return;
        }
    }

    if (uploadUrl == nullptr) {
        OTA_LOG_E("Upload log file failed, upload url is null");
        return;
    }

    if (taskId == nullptr) {
        OTA_LOG_E("Upload log file failed, task id is null");
        return;
    }

    std::string uploadFilePath = std::move(CopyOtaLogFileToUploadFile());
    if (uploadFilePath.empty()) {
        OTA_LOG_E("Copy log file failed");
        return;
    }

    // 压缩日志文件
    std::string copressLogPath = CompressFileToZip(uploadFilePath, g_logFileDir);
    if (copressLogPath.empty()) {
        OTA_LOG_E("Compress log file failed");
        return;
    }

    // 上传压缩后的日志文件
    OTA_LOG_D("Start upload ota log file:%s to %s", copressLogPath.c_str(), uploadUrl);
    if (UploadFileViaFormData(uploadUrl, copressLogPath, taskId, LogUploadWriteCallback, LogUploadProgressCallback) !=
        OTA_RET_OK) {
        OTA_LOG_E("Upload log filefailed");
    } else {
        OTA_LOG_D("Upload log file success");
    }
    // 删除上传的日志文件
    DeleteFile(copressLogPath);
    DeleteFile(uploadFilePath);
}

void OtaMasterLogPrint(OtaMasterLogLevel level, const char *file, int line, const char *format, ...)
{
    std::lock_guard<std::mutex> locker(g_logMutex);
    if (!g_isLogInitialized) {
        perror("Log module not initialized!");
        return;
    }

    // 写入日志前检查文件大小，超过100MB则自动备份并创建新文件
    CheckIfLogFileSizeExceedsUpperLimit();
    
    // 获取当前时间和日志等级对应的字符串
    std::string timeStr = GetCurrentLoggerTime();
    std::string levelStr = LogLevelToString(level);

    //提取文件名，过滤前面文件夹路径
    std::string filePath = file;
    auto pos = filePath.find_last_of('/');
    std::string filename = (pos != std::string::npos) ? filePath.substr(pos + 1) : filePath;

    // 格式化日志输出
    char logger[1024] = {0};
    va_list args;
    va_start(args, format);
    int headerLen = snprintf(logger, sizeof(logger), "%s %s %s(%d)[%s-%d] :", timeStr.c_str(), levelStr.c_str(),
                             g_logTag.c_str(), getpid(), filename.c_str(), line);
    if (headerLen < sizeof(logger)) {
        vsnprintf(logger + headerLen, sizeof(logger) - headerLen, format, args);
    }
    va_end(args);

    // 确保字符串以 null 结尾并在末尾加换行符
    size_t loggerLen = strnlen(logger, sizeof(logger) - 1);
    if (loggerLen < sizeof(logger) - 1) {
        logger[loggerLen] = '\n';
        logger[loggerLen + 1] = '\0';
    } else {
        logger[sizeof(logger) - 2] = '\n';
        logger[sizeof(logger) - 1] = '\0';
    }

    if (!g_logFileOs.is_open()) {
        perror("OTA Log not open");
        return;
    }
    g_logFileOs.write(logger, loggerLen + 1);
    g_logFileOs.flush();
}