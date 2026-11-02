#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>
#include "err_code.h"
#include "sr_log.h"
#include "mqtt_get_modem_info.h"

#define  LOG_UPLOAD_URL_API_ADDR        "https://gateway.zelostech.com.cn/data-server/tbox/log_file/upload_url"
#define  LOG_UPLOAD_RATE_LIMIT          (1024 * 1024 * 1024)
#define  LOG_FILE_PATH                  "/mnt/srlog/logstore"
#define  LOG_FILE_PATH_SUB              "/mnt/carota/sublog"
#define  LOG_FILE_EXT                   ".tgz"
#define  LOG_FILE_PATH_TMP              "/mnt/carota/log_upload/"

// 日志文件信息结构体
typedef struct {
    char filePath[256];
    time_t fileTime;
} LogFileInfo_t;

// 自定义数据结构，用于控制读取速率
typedef struct readLogData {
    FILE *fileFp;
    size_t rateLimit;
    size_t totalRead;
    time_t startTime;
    int firstCall;
} UploadLog_t;

typedef struct responseString {
    char *ptr;
    size_t len;
} UploadLogRes_t;

static pthread_mutex_t g_uploadLogLock = PTHREAD_MUTEX_INITIALIZER;
static uint64_t g_uploadLogStartTime = 0;
static uint64_t g_uploadLogEndTime = 0;
static char g_batchNumber[64] = {0};
static bool g_uploading = false;

static int PostTransferStatus(const char* batchNumber, const char* transferStatus);
static int GetUploadUrl(const char* batchNumber, const char* fileName, char* uploadUrl, int urlBufLen);



int32_t SetUpLoadLogFileInfo(uint64_t startTime, uint64_t endTime, const char* batchNumber)
{
    if(NULL == batchNumber || strlen(batchNumber) >= 64)
    {
        LOG_Error("batchNumber is NULL or len >= 64 ");
        //return RET_ERR;
    }

    if(0 == startTime)
    {
        LOG_Error("startTime is 0");
        return RET_ERR;
    }

    if(0 == endTime)
    {
        LOG_Error("endTime is 0");
        return RET_ERR;
    }

    if(endTime < startTime)
    {
        LOG_Error("endTime[%lu] < startTime[%lu]", endTime, startTime);
        return RET_ERR;
    }

    startTime = startTime / 1000;
    endTime = endTime / 1000;

    if((endTime - startTime) > 1800)
    {
        LOG_Error("(endTime[%lu] - startTime[%lu]) > 1800", endTime, startTime);
        //return RET_ERR;
    }

    if (g_uploading)
    {
        LOG_Warn("srlog: Upload is already in progress, ignoring new command");
        /*
        char uploadUrl[1024] = {0};
        if (batchNumber == NULL || GetUploadUrl(batchNumber, "PREVIOUS_TRANSFER_INCOMPLETE.tgz", uploadUrl, sizeof(uploadUrl)) != RET_OK) {
            LOG_Error("srlog: Failed to get upload URL");
            if (NULL != batchNumber) {
                PostTransferStatus(batchNumber, "TRANSFER_FAILED");
            }
            return RET_ERR;
        }
    
        PostTransferStatus(batchNumber, "PREVIOUS_TRANSFER_INCOMPLETE");
        return RET_OK;
        */
    }

    pthread_mutex_lock(&g_uploadLogLock);
    g_uploadLogStartTime = startTime;
    g_uploadLogEndTime = endTime;
    memset(g_batchNumber, 0, 64);
    memcpy(g_batchNumber, batchNumber, strlen(batchNumber));
    pthread_mutex_unlock(&g_uploadLogLock);

    LOG_Debug("log upload info is set, startTime=%lu, endTime=%lu", startTime, endTime);

    return RET_OK;
}


// 将文件名格式 YYYY-MM-DD-HH_MM_SS.tgz 转换为 Unix 时间戳
static time_t ParseLogFileNameToTime(const char *fileName)
{
    struct tm tm = {0};
    int year, mon, day, hour, min, sec;
    
    // 解析文件名格式: YYYY-MM-DD-HH_MM_SS.tgz
    if (sscanf(fileName, "%d-%d-%d-%d_%d_%d.tgz", &year, &mon, &day, &hour, &min, &sec) != 6) {
        return 0;
    }
    
    tm.tm_year = year - 1900;
    tm.tm_mon = mon - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_isdst = -1;
    
    return mktime(&tm);
}

// 查找指定路径下时间范围内的日志文件
static int FindLogFilesByTimeRangeInPath(const char* logPath, uint64_t startTime, uint64_t endTime, LogFileInfo_t *logFiles, int maxFiles, int *fileCount)
{
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;
    char fullPath[512] = {0};
    int count = 0;
    
    dir = opendir(logPath);
    if (dir == NULL) {
        LOG_Debug("Failed to open directory: %s", logPath);
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL && (*fileCount + count) < maxFiles) {
        // 检查文件扩展名
        if (strstr(entry->d_name, LOG_FILE_EXT) == NULL) {
            continue;
        }
        
        // 从文件名解析时间
        time_t fileTime = ParseLogFileNameToTime(entry->d_name);
        if (fileTime == 0) {
            LOG_Debug("Failed to parse filename: %s", entry->d_name);
            continue;
        }
        
        // 检查文件是否在时间范围内
        if (fileTime >= (time_t)startTime && fileTime <= (time_t)endTime) {
            snprintf(logFiles[*fileCount + count].filePath, sizeof(logFiles[*fileCount + count].filePath),
                     "%s/%s", logPath, entry->d_name);
            logFiles[*fileCount + count].fileTime = fileTime;
            count++;
        }
    }
    
    closedir(dir);
    return count;
}

// 查找时间范围内的日志文件（双路径搜索：主路径 + 副路径）
static int FindLogFilesByTimeRange(uint64_t startTime, uint64_t endTime, LogFileInfo_t *logFiles, int maxFiles)
{
    int fileCount = 0;
    int masterCount = 0;
    int subCount = 0;
    
    // 搜索主路径 /mnt/srlog/logstore
    masterCount = FindLogFilesByTimeRangeInPath(LOG_FILE_PATH, startTime, endTime, logFiles, maxFiles, &fileCount);
    fileCount += masterCount;
    LOG_Debug("Found %d log files in master path: %s", masterCount, LOG_FILE_PATH);
    
    // 搜索副路径 /mnt/carota/sublog
    subCount = FindLogFilesByTimeRangeInPath(LOG_FILE_PATH_SUB, startTime, endTime, logFiles, maxFiles, &fileCount);
    fileCount += subCount;
    LOG_Debug("Found %d log files in sub path: %s", subCount, LOG_FILE_PATH_SUB);
    
    // 按时间排序（冒泡排序）
    for (int i = 0; i < fileCount - 1; i++) {
        for (int j = 0; j < fileCount - i - 1; j++) {
            if (logFiles[j].fileTime > logFiles[j + 1].fileTime) {
                LogFileInfo_t temp = logFiles[j];
                logFiles[j] = logFiles[j + 1];
                logFiles[j + 1] = temp;
            }
        }
    }
    
    LOG_Info("Total found %d log files (master: %d, sub: %d) in time range [%lu, %lu]",
             fileCount, masterCount, subCount, startTime, endTime);
    
    return fileCount;
}

// 自定义读取回调函数，用于控制读取速率
static size_t readLogDataCallback(char *ptr, size_t size, size_t nmemb, void *userp)
{
    UploadLog_t *uploadLog = (UploadLog_t *)userp;
    size_t retcode;
    time_t currentTime = 0;
    double elapsedTime = 0.0;
    double waitTime = 0.0;
    size_t maxAllowed = 0;
    
    size_t bytesToRead = size * nmemb;
    
    // 如果是第一次调用，记录开始时间
    if(uploadLog->firstCall) 
    {
        uploadLog->startTime = time(NULL);
        uploadLog->firstCall = 0;
    }

    // 计算已经过去的时间
    currentTime = time(NULL);
    elapsedTime = difftime(currentTime, uploadLog->startTime);
    
    // 计算允许读取的最大字节数（基于速率限制）
    maxAllowed = (size_t)(uploadLog->rateLimit * elapsedTime) - uploadLog->totalRead;
    
    // 如果已经读取的字节数超过了允许的最大值，等待一段时间
    if (maxAllowed < bytesToRead) 
    {
        if (maxAllowed <= 0) 
        {
            // 需要等待
            waitTime = (double)bytesToRead / uploadLog->rateLimit - elapsedTime;
            if (waitTime > 0 && waitTime < 5) 
            {
                usleep((useconds_t)(waitTime * 1000000)); // 微秒
            }
        } 
        else 
        {
            bytesToRead = maxAllowed;
        }
    }
    
    // 从文件中读取数据
    retcode = fread((void*)ptr, 1, bytesToRead, uploadLog->fileFp);
    
    // 更新已读取的总字节数
    uploadLog->totalRead += retcode;
    
    if(0 == retcode)
    {
        uploadLog->firstCall = 1;
    }
    LOG_Debug("post len %zu", retcode);
    return retcode;
}


/**
 * @brief CURL写入回调函数 — 用于接收GetUploadUrl请求的响应数据
 * @details 将HTTP响应内容追加到用户提供的response缓冲区中，
 *          缓冲区大小限制为2048字节，防止溢出。
 * @param contents HTTP响应内容指针
 * @param size    单元大小（固定为1）
 * @param nmemb   数据单元数量
 * @param userp   指向接收缓冲区的指针
 * @return 实际处理的字节数
 */
static size_t getUploadUrlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    char *response = (char *)userp;

    if (response != NULL && contents != NULL) {
        size_t avail = 2047 - strlen(response);
        if (realsize < avail) {
            strncat(response, (char *)contents, realsize);
        }
    }
    return realsize;
}

static int parseSuccessFromJson(const char *jsonStr)
{
    const char *key = "\"success\":";
    const char *pos = strstr(jsonStr, key);
    if (pos == NULL) {
        return 0;
    }

    pos += strlen(key);
    while (*pos == ' ') pos++;

    if (strncmp(pos, "true", 4) == 0) {
        return 1;
    }
    return 0;
}

/**
 * @brief 从JSON响应字符串中提取uploadUrl字段的值
 * @details 使用轻量级字符串查找方式解析JSON，不依赖第三方JSON库。
 *          先校验 "success":true，再查找 "\"uploadUrl\"" 键并提取其后的字符串值。
 *          示例: {"success":true,"data":{"uploadUrl":"https://..."}}
 * @param jsonStr    JSON响应字符串
 * @param uploadUrl  输出缓冲区，存储提取到的URL
 * @param urlBufLen  输出缓冲区长度
 * @return RET_OK 提取成功，RET_ERR 提取失败、字段不存在或success不为true
 */
static int parseUploadUrlFromJson(const char *jsonStr, char *uploadUrl, int urlBufLen)
{
    if (!parseSuccessFromJson(jsonStr)) {
        LOG_Error("srlog: response success is not true");
        return RET_ERR;
    }

    const char *key = "\"uploadUrl\"";
    const char *pos = strstr(jsonStr, key);
    if (pos == NULL) {
        LOG_Error("srlog: uploadUrl not found in response");
        return RET_ERR;
    }

    pos += strlen(key);
    while (*pos == ' ' || *pos == ':') pos++;

    if (*pos != '"') {
        LOG_Error("srlog: Invalid uploadUrl value in response");
        return RET_ERR;
    }
    pos++;

    const char *end = strchr(pos, '"');
    if (end == NULL) {
        LOG_Error("srlog: Invalid uploadUrl value format");
        return RET_ERR;
    }

    int len = end - pos;
    if (len >= urlBufLen) {
        len = urlBufLen - 1;
    }

    strncpy(uploadUrl, pos, len);
    uploadUrl[len] = '\0';

    LOG_Debug("srlog: Parsed uploadUrl: %s", uploadUrl);
    return RET_OK;
}

/**
 * @brief 从JSON响应字符串中提取code字段的值
 * @details 使用轻量级字符串查找方式解析JSON，查找 "\"code\"" 键并提取其整数值。
 *          示例: {"code":0, "message":"success"}
 * @param jsonStr JSON响应字符串
 * @return code值，如果未找到则返回-1
 */
static int parseCodeFromJson(const char *jsonStr)
{
    const char *key = "\"code\"";
    const char *pos = strstr(jsonStr, key);
    if (pos == NULL) {
        return -1;
    }

    pos += strlen(key);
    while (*pos == ' ' || *pos == ':') pos++;

    return atoi(pos);
}

/**
 * @brief 向远端平台请求获取日志文件上传的预签名URL
 * @details 通过HTTP POST请求向平台 upload_url API 发送请求，
 *          获取对象存储的预签名上传地址（含X-Amz-Signature参数）。
 *          请求格式: {"batchNumber":"...", "fileName":"..."}
 *          响应格式: {"success":true,"errorCode":null,"message":null,"data":{"batchNumber":"...", "uploadUrl":"https://...?X-Amz-Signature=...", "expireTime":"..."}}
 *          注意: 禁用SSL验证（CURLOPT_SSL_VERIFYPEER/CURLOPT_SSL_VERIFYHOST=0L），
 *                适用于嵌入式设备对性能的场景，生产环境可根据需要开启验证。
 * @param batchNumber 批次号
 * @param fileName    日志文件名
 * @param uploadUrl   输出缓冲区，存储获取到的预签名URL
 * @param urlBufLen   输出缓冲区长度
 * @return RET_OK 获取成功，RET_ERR 获取失败
 */
int GetUploadUrl(const char* batchNumber, const char* fileName, char* uploadUrl, int urlBufLen)
{
    CURL *curl;
    CURLcode res;
    char jsonData[512] = {0};
    char response[2048] = {0};
    long httpCode = 0;

    if (NULL == batchNumber || NULL == fileName || NULL == uploadUrl) {
        LOG_Error("srlog: Invalid parameters");
        return RET_ERR;
    }

    /* 构建JSON请求体 */
    snprintf(jsonData, sizeof(jsonData),
             "{\"batchNumber\":\"%s\",\"fileName\":\"%s\"}", batchNumber, fileName);

    LOG_Debug("srlog: GetUploadUrl request: %s", jsonData);

    /* 初始化CURL并开始请求 */
    curl = curl_easy_init();
    if (curl == NULL) {
        LOG_Error("curl_easy_init failed");
        return RET_ERR;
    }

    /* 设置HTTP头部: Content-Type: application/json */
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    /* 配置请求参数 */
    curl_easy_setopt(curl, CURLOPT_URL, LOG_UPLOAD_URL_API_ADDR);        // 目标API地址
    curl_easy_setopt(curl, CURLOPT_POST, 1L);                             // POST方法
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData);                 // JSON请求体
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);                  // 头部信息
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getUploadUrlWriteCallback); // 响应回调
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);          // 回调参数
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);                         // 总超时30秒
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);                  // 连接超时30秒
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);                   // 跳过SSL证书验证
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);                   // 跳过SSL主机验证
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);                     // 请求完成后不复用连接
    curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);                    // 强制新建连接
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 0L);                    // 关闭 TCP keepalive


    /* 执行请求 */
    res = curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        LOG_Error("curl_easy_perform failed: %s", curl_easy_strerror(res));
        return RET_ERR;
    }

    if (httpCode != 200) {
        LOG_Error("HTTP error: %ld", httpCode);
        return RET_ERR;
    }

    LOG_Debug("srlog: GetUploadUrl response: %s", response);

    /* 提取uploadUrl字段（内部已校验success字段） */
    if (parseUploadUrlFromJson(response, uploadUrl, urlBufLen) != RET_OK) {
        LOG_Error("Failed to parse uploadUrl from response");
        return RET_ERR;
    }

    LOG_Info("srlog: Get upload URL success: %s", uploadUrl);
    return RET_OK;
}

#define  LOG_UPLOAD_COMPLETE_URL_ADDR   "https://gateway.zelostech.com.cn/data-server/tbox/log_file/upload_complete"

int PostTransferStatus(const char* batchNumber, const char* transferStatus)
{
    CURL *curl;
    CURLcode res;
    char jsonData[512] = {0};
    char response[2048] = {0};
    long httpCode = 0;

    if (NULL == batchNumber || NULL == transferStatus) {
        LOG_Error("srlog: Invalid parameters");
        return RET_ERR;
    }

    snprintf(jsonData, sizeof(jsonData),
             "{\"batchNumber\":\"%s\",\"transferStatus\":\"%s\"}", batchNumber, transferStatus);

    LOG_Info("srlog: PostTransferStatus request: %s", jsonData);

    curl = curl_easy_init();
    if (curl == NULL) {
        LOG_Error("curl_easy_init failed");
        return RET_ERR;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, LOG_UPLOAD_COMPLETE_URL_ADDR);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getUploadUrlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
    curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 0L);

    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        LOG_Error("srlog: PostTransferStatus curl_easy_perform failed: %s", curl_easy_strerror(res));
        return RET_ERR;
    }

    if (httpCode != 200) {
        LOG_Error("srlog: PostTransferStatus HTTP error: %ld", httpCode);
        return RET_ERR;
    }

    LOG_Info("srlog: PostTransferStatus response: %s", response);

    if (!parseSuccessFromJson(response)) {
        LOG_Error("srlog: PostTransferStatus response success is not true");
        return RET_ERR;
    }

    LOG_Info("srlog: Post transfer status success: batchNumber=%s, transferStatus=%s", batchNumber, transferStatus);
    return RET_OK;
}

// 接收HTTP响应回调
size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) 
{
    char resBuff[1024] = {0};
    size_t realsize = size * nmemb;
    UploadLogRes_t *s = (UploadLogRes_t *)userp;

    if(NULL != contents && realsize < 1024)
    {
        memcpy(resBuff, contents, realsize);
    }

    LOG_Debug("post res [%zu]:%s", realsize, (char*)contents);
    return realsize;
}

int PostLogFileToZelos(const char* filePath, char *batchNumber, uint64_t startTime)
{
    CURL *curl;
    CURLcode res = CURLE_FAILED_INIT;
    UploadLog_t readLogData;
    UploadLogRes_t resStr;
    long fileSize = 0;
    uint8_t carNum[8] = {0};
    char uploadFileName[128] = {0};
    char tmpBuff[128] = {0};
    time_t stTmpTime;
    struct tm *pstRetTime;
    const char *originalFileName = NULL;

    if(NULL == filePath)
    {
        LOG_Error("filePath is NULL");       
        return RET_ERR;
    }

    if(NULL == batchNumber)
    {
        LOG_Error("batchNumber is NULL");
    }

    // 打开要上传的文件
    FILE *fileFp = fopen(filePath, "rb");
    if(NULL == fileFp)
    {
        LOG_Error("无法打开文件: %s", filePath);
        return RET_ERR;
    }
    
    if(FALSE == GetCarNum(carNum, 8))
    {
        LOG_Error("%s ,GetCarNum error", __FUNCTION__);
        fclose(fileFp);
        return RET_ERR;
    }

    
    stTmpTime = startTime;
    stTmpTime += 28800;
    pstRetTime = gmtime(&stTmpTime);

    // 从完整路径中提取原始文件名
    originalFileName = strrchr(filePath, '/');
    if (originalFileName != NULL) {
        originalFileName++; // 跳过 '/'
    } else {
        originalFileName = filePath;
    }

    // 根据主副模组区分文件名
    const char* moduleSuffix = (0 == access("/dev/mmcblk0", F_OK)) ? "-M" : "-S";
    
    snprintf(tmpBuff, sizeof(tmpBuff), "%s-%04d%02d%02d%02d%02d%02d%s%s", carNum,
             (pstRetTime->tm_year + 1900), (pstRetTime->tm_mon + 1), pstRetTime->tm_mday,
             pstRetTime->tm_hour, pstRetTime->tm_min, pstRetTime->tm_sec, moduleSuffix, LOG_FILE_EXT);

    /* 请求预签名上传 URL（PUT 方式） */
    char uploadUrl[1024] = {0};
    if (batchNumber == NULL || GetUploadUrl(batchNumber, tmpBuff, uploadUrl, sizeof(uploadUrl)) != RET_OK) {
        LOG_Error("srlog: Failed to get upload URL");
        fclose(fileFp);
        if (NULL != batchNumber) {
            PostTransferStatus(batchNumber, "TRANSFER_FAILED");
        }
        return RET_ERR;
    }
    LOG_Info("srlog: Using dynamic upload URL for PUT upload");

    // 获取文件大小
    fseek(fileFp, 0, SEEK_END);
    fileSize = ftell(fileFp);
    fseek(fileFp, 0, SEEK_SET);
    
    // 初始化libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    LOG_Debug("start upload via PUT: %s", filePath);
    // 创建curl句柄
    curl = curl_easy_init();
    if(curl)
    {
        // 准备自定义读取数据结构（速率限制）
        readLogData.firstCall = 1;
        readLogData.fileFp = fileFp;
        readLogData.rateLimit = LOG_UPLOAD_RATE_LIMIT;
        readLogData.totalRead = 0;
        readLogData.startTime = time(NULL);

        LOG_Info("PUT upload: fileSize=%ld, fileName=%s", fileSize, tmpBuff);
        curl_easy_setopt(curl, CURLOPT_URL, uploadUrl);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);                                // PUT 方法
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fileSize);    // 文件大小
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readLogDataCallback);        // 读回调（速率限制）
        curl_easy_setopt(curl, CURLOPT_READDATA, &readLogData);

        // 设置写入回调函数和数据
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resStr);

        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
        curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1800L);           // 30分钟超时
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1024L);   // 每秒至少 1KB
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 262144L);      // 256KB 读写缓冲区
        
        LOG_Debug("开始 PUT 上传文件，大小: %ld 字节，文件名: %s\n", fileSize, tmpBuff);
        
        // 执行请求
        res = curl_easy_perform(curl);
        
        // 检查错误
        if(res != CURLE_OK)
        {
            LOG_Error("curl_easy_perform() 失败: %s\n", curl_easy_strerror(res));
        }
        else
        {
            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            LOG_Info("上传完成，HTTP状态码: %ld，总耗时: %.2f 秒\n", httpCode, difftime(time(NULL), readLogData.startTime));
        }
        
        // 清理curl句柄
        curl_easy_cleanup(curl);
    }
    else
    {
        LOG_Error("curl_easy_init() error");
    }
    
    // 关闭文件
    fclose(fileFp);
    
    // 全局清理
    curl_global_cleanup();

    int ret = (res == CURLE_OK) ? RET_OK : RET_ERR;
    if (batchNumber != NULL) {
        if (ret == RET_OK) {
            PostTransferStatus(batchNumber, "TRANSFER_COMPLETED");
        } else {
            PostTransferStatus(batchNumber, "TRANSFER_FAILED");
        }
    }
    
    return ret;
}


void *UploadLogFileThread(void *pArg)
{
    LogFileInfo_t logFiles[500]; // 最多支持500个文件
    int fileCount = 0;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    char batchNumber[64] = {0};

    LOG_Debug("srlog thread start");

    while(1)
    {

        if(g_uploadLogStartTime == 0)
        {
            sleep(1);
            continue;
        }

        // 检查存储设备是否存在（主模组检查EMMC，副模组检查/usrdata）
        if(0 == access("/dev/mmcblk0", F_OK)) {
            // 主模组 - 检查EMMC挂载点
            if(0 != access("/mnt/srlog", F_OK)) {
                sleep(30);
                LOG_Debug("log upload not exec, /mnt/srlog not exist");
                continue;
            }
        } else {
            // 副模组 - 检查/mnt/carota路径
            if(0 != access("/mnt/carota", F_OK)) {
                sleep(30);
                LOG_Debug("log upload not exec, /mnt/carota not exist");
                continue;
            }
        }

        // 获取上传参数
        pthread_mutex_lock(&g_uploadLogLock);
        startTime = g_uploadLogStartTime;
        endTime = g_uploadLogEndTime;
        memcpy(batchNumber, g_batchNumber, 64);
        pthread_mutex_unlock(&g_uploadLogLock);

        LOG_Debug("log upload start, startTime=%lu, endTime=%lu", startTime, endTime);

        // 查找时间范围内的日志文件
        fileCount = FindLogFilesByTimeRange(startTime, endTime, logFiles, 500);
        
        if (fileCount == 0) {
            LOG_Debug("No log files found in time range [%lu, %lu]", startTime, endTime);
            
            char packCmd[1024] = {0};
            char packedFilePath[256] = {0};
            char emptyFilePath[256] = {0};
            int ret = RET_OK;
            
            // 构建打包文件名: /mnt/carota/log_upload/log_[batchNumber]_[startTime].tgz
            snprintf(packedFilePath, sizeof(packedFilePath), "%slog_%s_%lu.tgz",
                     LOG_FILE_PATH_TMP, batchNumber, startTime);
            
            // 在 /mnt/srlog/logstore 目录下生成空文件
            snprintf(emptyFilePath, sizeof(emptyFilePath), "%s/log_is_empty.txt", LOG_FILE_PATH);
            snprintf(packCmd, sizeof(packCmd), "touch %s", emptyFilePath);
            SrGetSystemRetValue(packCmd);
            
            // 打包空文件
            snprintf(packCmd, sizeof(packCmd), "tar -czf %s -C %s log_is_empty.txt", packedFilePath, LOG_FILE_PATH);
            LOG_Debug("srlog: Pack empty log command: %s", packCmd);
            
            ret = SrGetSystemRetValue(packCmd);
            if (ret != 0) {
                LOG_Error("Failed to pack empty log file, ret=%d", ret);
            } else {
                if (access(packedFilePath, F_OK) != 0) {
                    LOG_Error("Packed file not exist: %s", packedFilePath);
                } else {
                    LOG_Debug("srlog: Packed empty log file created: %s", packedFilePath);
                    g_uploading = true;
                    ret = PostLogFileToZelos(packedFilePath, batchNumber, startTime);
                    if (ret != RET_OK) {
                        LOG_Error("Failed to upload packed empty log file: %s", packedFilePath);
                    }
                    g_uploading = false;
                }
                
                // 删除临时打包文件
                snprintf(packCmd, sizeof(packCmd), "rm -f %s", packedFilePath);
                SrGetSystemRetValue(packCmd);
                
                // 删除空文件
                snprintf(packCmd, sizeof(packCmd), "rm -f %s", emptyFilePath);
                SrGetSystemRetValue(packCmd);
            }
        } else {
            LOG_Debug("Found %d log files to upload", fileCount);
            
            // 将多个日志文件打包成一个文件上传
            // 双路径打包：master/ 和 sub/ 同级目录
            char packCmd[1024] = {0};
            char packedFilePath[256] = {0};
            char masterDirPath[256] = {0};
            char subDirPath[256] = {0};
            char masterFileList[256] = {0};
            char subFileList[256] = {0};
            int ret = RET_OK;
            int masterFileCount = 0;
            int subFileCount = 0;
            
            // 创建临时目录结构: /mnt/carota/log_upload/master/ 和 /mnt/carota/log_upload/sub/
            snprintf(packCmd, sizeof(packCmd), "mkdir -p %smaster/ %ssub/", LOG_FILE_PATH_TMP, LOG_FILE_PATH_TMP);
            int mkdirRet = SrGetSystemRetValue(packCmd);
            LOG_Debug("mkdir ret=%d, cmd=%s", mkdirRet, packCmd);
            if (mkdirRet != 0) {
                LOG_Error("Failed to create temp directories: %s, ret=%d", packCmd, mkdirRet);
                continue;
            }
            
            // 构建打包文件名: /mnt/carota/log_upload/log_[batchNumber]_[startTime].tgz
            snprintf(packedFilePath, sizeof(packedFilePath), "%slog_%s_%lu.tgz",
                     LOG_FILE_PATH_TMP, batchNumber, startTime);
            
            // 构建文件列表文件路径
            snprintf(masterFileList, sizeof(masterFileList), "%smaster_filelist_%lu.txt",
                     LOG_FILE_PATH_TMP, startTime);
            snprintf(subFileList, sizeof(subFileList), "%ssub_filelist_%lu.txt",
                     LOG_FILE_PATH_TMP, startTime);
            
            // 将文件列表按来源路径分开写入两个临时文件
            FILE *masterFp = fopen(masterFileList, "w");
            FILE *subFp = fopen(subFileList, "w");
            if (masterFp == NULL || subFp == NULL) {
                LOG_Error("Failed to create file lists");
                if (masterFp) fclose(masterFp);
                if (subFp) fclose(subFp);
                continue;
            }
            
            for (int i = 0; i < fileCount; i++) {
                const char *srcPath = logFiles[i].filePath;
                const char *fileName = strrchr(srcPath, '/');
                if (fileName) fileName++; else fileName = srcPath;
                
                // 根据来源路径判断是master还是sub
                if (strstr(srcPath, LOG_FILE_PATH_SUB) != NULL) {
                    // 来自副路径的文件，复制到 sub/ 目录
                    fprintf(subFp, "%s\n", srcPath);
                    subFileCount++;
                } else {
                    // 来自主路径的文件，复制到 master/ 目录
                    fprintf(masterFp, "%s\n", srcPath);
                    masterFileCount++;
                }
            }
            fclose(masterFp);
            fclose(subFp);
            
            LOG_Info("File count: master=%d, sub=%d", masterFileCount, subFileCount);
            
            // 复制文件到对应目录
            if (masterFileCount > 0) {
                snprintf(packCmd, sizeof(packCmd), "cat %s | while read f; do cp \"$f\" %smaster/; done", masterFileList, LOG_FILE_PATH_TMP);
                SrGetSystemRetValue(packCmd);
            }
            if (subFileCount > 0) {
                snprintf(packCmd, sizeof(packCmd), "cat %s | while read f; do cp \"$f\" %ssub/; done", subFileList, LOG_FILE_PATH_TMP);
                SrGetSystemRetValue(packCmd);
            }
            
            // 打包整个 /mnt/carota/log_upload/ 目录（包含 master/ 和 sub/）
            snprintf(packCmd, sizeof(packCmd), "cd %s && tar -czf %s master/ sub/", LOG_FILE_PATH_TMP, packedFilePath);
            LOG_Debug("Pack command: %s", packCmd);
            
            ret = SrGetSystemRetValue(packCmd);
            if (ret != 0) {
                LOG_Error("Failed to pack log files, ret=%d", ret);
            } else {
                // 检查打包后的文件是否存在
                if (access(packedFilePath, F_OK) != 0) {
                    LOG_Error("Packed file not exist: %s", packedFilePath);
                } else {
                    LOG_Debug("Packed file created: %s", packedFilePath);
                    g_uploading = true;
                    ret = PostLogFileToZelos(packedFilePath, batchNumber, startTime);
                    if (ret != RET_OK) {
                        LOG_Error("Failed to upload packed file: %s", packedFilePath);
                    }
                    g_uploading = false;

                }
                
                // 删除临时打包文件
                snprintf(packCmd, sizeof(packCmd), "rm -f %s", packedFilePath);
                SrGetSystemRetValue(packCmd);
            }
            
            // 清理临时目录和文件列表
            snprintf(packCmd, sizeof(packCmd), "rm -rf %smaster/ %ssub/ %s %s",
                     LOG_FILE_PATH_TMP, LOG_FILE_PATH_TMP, masterFileList, subFileList);
            SrGetSystemRetValue(packCmd);
        }

        // 清空上传参数
        pthread_mutex_lock(&g_uploadLogLock);
        g_uploading = false;
        g_uploadLogStartTime = 0;
        g_uploadLogEndTime = 0;
        memset(g_batchNumber, 0, 64);
        pthread_mutex_unlock(&g_uploadLogLock);

    }
}


// 兼容旧接口，将 SetUpLoadCanFileInfo 映射到新的 SetUpLoadLogFileInfo
int32_t SetUpLoadCanFileInfo(uint64_t startTime, uint64_t endTime, const char* batchNumber)
{
    return SetUpLoadLogFileInfo(startTime, endTime, batchNumber);
}

// 兼容旧接口，将 UploadCanFileThread 映射到新的 UploadLogFileThread
void *UploadCanFileThread(void *pArg)
{
    return UploadLogFileThread(pArg);
}
