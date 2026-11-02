#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "err_code.h"
#include "sr_log.h"
#include "mqtt_get_modem_info.h"

// #define  LOG_FILE_POST_ADDR             "http://gateway-uat.zelostech.com.cn/data-server/tbox/upload_log_file"
#define  LOG_FILE_POST_ADDR             "http://gateway.zelostech.com.cn/data-server/tbox/upload_log_file"
#define  LOG_UPLOAD_RATE_LIMIT          (1024 * 1024 * 1024)
#define  LOG_FILE_PATH                  "/mnt/srlog/logstore"
#define  LOG_FILE_PATH_SUB              "/usrdata/log/logstore"
#define  LOG_FILE_EXT                   ".tgz"
#define  LOG_FILE_PATH_TMP              "/tmp/log_upload/"

// 日志文件信息结构体
typedef struct {
    char filePath[256];
    time_t fileTime;
} LogFileInfo_t;

// 自定义数据结构，用于控制读取速率
typedef struct readLogData {
    FILE *fileFp;
    size_t rateLimit; // 字节/秒
    size_t totalRead;
    time_t startTime;
} UploadLog_t;

typedef struct responseString {
    char *ptr;
    size_t len;
} UploadLogRes_t;

static pthread_mutex_t g_uploadLogLock = PTHREAD_MUTEX_INITIALIZER;
static uint64_t g_uploadLogStartTime = 0;
static uint64_t g_uploadLogEndTime = 0;
static char g_batchNumber[64] = {0};


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

    if((endTime - startTime) > 1800)
    {
        LOG_Error("(endTime[%lu] - startTime[%lu]) > 1800", endTime, startTime);
        //return RET_ERR;
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

// 获取当前模组的日志路径
static const char* GetLogFilePath(void)
{
    if(0 == access("/dev/mmcblk0", F_OK)) {
        // 主模组 - 使用EMMC路径
        return LOG_FILE_PATH;
    } else {
        // 副模组 - 使用/usrdata路径
        return LOG_FILE_PATH_SUB;
    }
}

// 查找时间范围内的日志文件
static int FindLogFilesByTimeRange(uint64_t startTime, uint64_t endTime, LogFileInfo_t *logFiles, int maxFiles)
{
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;
    int fileCount = 0;
    char fullPath[512] = {0};
    const char* logPath = GetLogFilePath();
    
    dir = opendir(logPath);
    if (dir == NULL) {
        LOG_Error("Failed to open directory: %s", logPath);
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL && fileCount < maxFiles) {
        // 检查文件扩展名
        if (strstr(entry->d_name, LOG_FILE_EXT) == NULL) {
            continue;
        }
        
        // 从文件名解析时间
        time_t fileTime = ParseLogFileNameToTime(entry->d_name);
        if (fileTime == 0) {
            LOG_Debug("Failed to parse filename: %s", entry->d_name);
            continue;
        }else{
            // 成功解析文件名中的时间戳，记录详细信息用于调试
            char timeStr[64] = {0};
            struct tm *tm_info = localtime(&fileTime);
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
            //LOG_Debug("Successfully parsed log filename '%s' to timestamp: %ld (%s)", entry->d_name, fileTime, timeStr);
        }
        
        // 检查文件是否在时间范围内
        // 文件时间应该在 [startTime, endTime] 范围内
        if (fileTime >= (time_t)startTime && fileTime <= (time_t)endTime) {
            snprintf(logFiles[fileCount].filePath, sizeof(logFiles[fileCount].filePath),
                     "%s/%s", logPath, entry->d_name);
            logFiles[fileCount].fileTime = fileTime;
            fileCount++;
            char timeStr[64] = {0};
            struct tm *tm_info = localtime(&fileTime);
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
            //LOG_Debug("Found log file: %s, timestamp: %ld (%s)", entry->d_name, fileTime, timeStr);
        }
    }
    
    closedir(dir);
    
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
    static int firstCall = 1;
    size_t maxAllowed = 0;
    
    size_t bytesToRead = size * nmemb;
    
    // 如果是第一次调用，记录开始时间
    if(firstCall) 
    {
        uploadLog->startTime = time(NULL);
        firstCall = 0;
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
        firstCall = 1;
    }
    LOG_Debug("post len %zu", retcode);
    //uint8_t* dataPtr = (uint8_t*)(ptr + retcode - 1024);
    //LOG_DebugHex(dataPtr, 1024, "post_data");
    return retcode;
}


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

int PostLogFileToZelos(const char* curlAddr, const char* filePath, char *batchNumber, uint64_t startTime) 
{
    CURL *curl;
    CURLcode res;
    UploadLog_t readLogData;
    UploadLogRes_t resStr;
    long fileSize = 0;
    uint8_t carNum[8] = {0};
    char uploadFileName[128] = {0};
    char tmpBuff[128] = {0};
    time_t stTmpTime;
    struct tm *pstRetTime;
    const char *originalFileName = NULL;
    
    if(NULL == curlAddr)
    {
        LOG_Error("curlAddr is NULL");
        return RET_ERR;
    }

    if(NULL == filePath)
    {
        LOG_Error("filePath is NULL");
        return RET_ERR;
    }

    if(NULL == batchNumber)
    {
        LOG_Error("batchNumber is NULL");
        //return RET_ERR;
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

    // 获取文件大小
    fseek(fileFp, 0, SEEK_END);
    fileSize = ftell(fileFp);
    fseek(fileFp, 0, SEEK_SET);
    
    // 初始化libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    LOG_Debug("start post file: %s", filePath);
    // 创建curl句柄
    curl = curl_easy_init();
    if(curl)
    {
        // 使用 mime API 替代旧的 formadd API，正确处理二进制数据
        curl_mime *mime = curl_mime_init(curl);
        curl_mimepart *part = NULL;
        
        // 添加字符串参数到表单 - batchNumber
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "batchNumber");
        curl_mime_data(part, batchNumber ? batchNumber : "", CURL_ZERO_TERMINATED);
        
        // 准备自定义读取数据结构
        readLogData.fileFp = fileFp;
        readLogData.rateLimit = LOG_UPLOAD_RATE_LIMIT;
        readLogData.totalRead = 0;
        
        // 添加文件参数 - 使用 mime API 正确处理二进制文件
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "file");
        curl_mime_filename(part, tmpBuff);
        curl_mime_type(part, "application/gzip");
        // 使用回调函数提供数据，避免 mime API 内部读取导致的二进制数据损坏
        curl_mime_data_cb(part, fileSize, readLogDataCallback, NULL, NULL, &readLogData);
        
        // 设置目标URL
        curl_easy_setopt(curl, CURLOPT_URL, curlAddr);
        
        // 设置写入回调函数和数据
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resStr);

        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);

        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 0L); // 关闭 TCP keepalive
        curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L); // 强制新建连接

        // 设置连接超时（建议 10-30 秒）
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);

        // 设置整个请求超时（包括上传）（建议 60-300 秒）
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); // 5分钟超时

        // 可选：设置低速传输超时（防止卡住）
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1024L);     // 每秒至少 1KB
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);        // 否则超时
        // 设置 mime 数据
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        
        // 如果需要，可以设置其他选项
        // 例如，设置详细模式以查看请求详情
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        LOG_Debug("开始上传文件，大小: %ld 字节，文件名: %s\n", fileSize, tmpBuff);
        
        // 执行请求
        res = curl_easy_perform(curl);
        
        // 检查错误
        if(res != CURLE_OK)
        {
            LOG_Error("curl_easy_perform() 失败: %s\n", curl_easy_strerror(res));
        }
        else
        {
            LOG_Debug("上传完成，总耗时: %.2f 秒\n", difftime(time(NULL), readLogData.startTime));
        }
        
        // 清理 mime
        curl_mime_free(mime);
        
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
    
    return (res == CURLE_OK) ? RET_OK : RET_ERR;
}


void *UploadLogFileThread(void *pArg)
{
    LogFileInfo_t logFiles[500]; // 最多支持500个文件
    int fileCount = 0;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    char batchNumber[64] = {0};

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
            // 副模组 - 检查/usrdata路径
            if(0 != access("/usrdata/log", F_OK)) {
                sleep(30);
                LOG_Debug("log upload not exec, /usrdata/log not exist");
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
        } else {
            LOG_Debug("Found %d log files to upload", fileCount);
            
            // 将多个日志文件打包成一个文件上传
            char packCmd[512] = {0};
            char packedFilePath[256] = {0};
            char fileListPath[256] = {0};
            int ret = RET_OK;
            
            // 创建临时目录
            snprintf(packCmd, sizeof(packCmd), "mkdir -p %s", LOG_FILE_PATH_TMP);
            SrGetSystemRetValue(packCmd);
            
            // 构建打包文件名: /tmp/log_upload/log_[batchNumber]_[startTime].tgz
            snprintf(packedFilePath, sizeof(packedFilePath), "%slog_%s_%lu.tgz",
                     LOG_FILE_PATH_TMP, batchNumber, startTime);
            
            // 构建文件列表文件路径
            snprintf(fileListPath, sizeof(fileListPath), "%sfilelist_%lu.txt",
                     LOG_FILE_PATH_TMP, startTime);
            
            // 将文件列表写入临时文件（避免命令行过长）
            FILE *fp = fopen(fileListPath, "w");
            if (fp == NULL) {
                LOG_Error("Failed to create file list: %s", fileListPath);
                continue;
            }
            for (int i = 0; i < fileCount; i++) {
                fprintf(fp, "%s\n", logFiles[i].filePath);
            }
            fclose(fp);
            
            // 打包命令: tar -czf [输出文件] -T [文件列表]
            snprintf(packCmd, sizeof(packCmd), "tar -czf %s -T %s", packedFilePath, fileListPath);
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
                    // 上传打包后的文件
                    ret = PostLogFileToZelos(LOG_FILE_POST_ADDR, packedFilePath, batchNumber, startTime);
                    if (ret != RET_OK) {
                        LOG_Error("Failed to upload packed file: %s", packedFilePath);
                    }
                }
                
                // 删除临时打包文件
                snprintf(packCmd, sizeof(packCmd), "rm -f %s", packedFilePath);
                SrGetSystemRetValue(packCmd);
            }
            
            // 删除文件列表文件
            snprintf(packCmd, sizeof(packCmd), "rm -f %s", fileListPath);
            SrGetSystemRetValue(packCmd);
        }

        // 清空上传参数
        pthread_mutex_lock(&g_uploadLogLock);
        g_uploadLogStartTime = 0;
        g_uploadLogEndTime = 0;
        memset(g_batchNumber, 0, 64);
        pthread_mutex_unlock(&g_uploadLogLock);

        LOG_Debug("log upload end, uploaded %d files", fileCount);
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
