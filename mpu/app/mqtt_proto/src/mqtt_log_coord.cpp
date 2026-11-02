#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <regex.h>
#include <sys/stat.h>
#include <time.h>
#include <stdarg.h>
#include "civetweb.h"
#include "sr_log.h"
#include "pm_server_api.h"
#include "mqtt_srlog_upload.h"
#include "mqtt_log_coord.h"
#include "mqtt_connect.h"

/* ==================== 测试日志记录功能（测试完成后可整体注释掉） ==================== */
/* 测试日志开关：设为0可关闭所有测试日志（必须在文件作用域定义） */
#define TEST_LOG_ENABLE 0

/* 测试日志路径定义 */
#define TEST_LOG_FILE_PATH_MASTER "/mnt/srlog/logstore/testlog.txt"
#define TEST_LOG_FILE_PATH_SLAVE  "/usrdata/log/logstore/testlog.txt"

/**
 * @brief 写入测试日志到文件（根据主副模组自动选择路径）
 * @param fmt 格式化字符串
 * @param ... 可变参数
 *
 * 注意：此函数为测试专用，测试完成后可整体注释掉
 */
static void TestLog_Write(const char *fmt, ...)
{
    /* 测试日志开关检查 */
    if (!TEST_LOG_ENABLE) {
        return;
    }

    const char *logPath = NULL;
    
    /* 根据主副模组选择日志路径 */
    if (access("/dev/mmcblk0", F_OK) == 0) {
        logPath = TEST_LOG_FILE_PATH_MASTER;  /* 主模组 */
    } else {
        logPath = TEST_LOG_FILE_PATH_SLAVE;   /* 副模组 */
    }
    
    /* 获取当前时间 */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timeBuf[64] = {0};
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_info);
    
    /* 打开日志文件（追加模式） */
    FILE *fp = fopen(logPath, "a");
    if (fp == NULL) {
        return;  /* 文件打开失败，静默返回 */
    }
    
    /* 写入时间戳 */
    fprintf(fp, "[%s] ", timeBuf);
    
    /* 写入日志内容 */
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
    
    fprintf(fp, "\n");
    fclose(fp);
}

/* 测试日志宏定义（测试完成后可整体注释掉） */
#define TEST_LOG(fmt, ...) TestLog_Write("[TEST] " fmt, ##__VA_ARGS__)
/* ==================== 测试日志记录功能结束 ==================== */

// 外部函数声明（mqtt_connect.cpp中定义，C++链接）
extern gint ZelosControlCanFileUpload(uint64_t startTime, uint64_t endTime, const char *batchNumber);

// 自定义写回调：将数据写入 char 缓冲区
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    char *response = (char *)userp;
    
    // 追加数据到缓冲区（注意不要溢出）
    if (realsize < 256) {
        strncat(response, (char *)contents, realsize);
    }
    
    return realsize;
}

// ==================== 配置常量 ====================
#define HTTP_SERVER_PORT "30001"
#define MASTER_IP "192.168.1.201"
#define SLAVE_IP "192.168.1.202"
#define LOG_COORDINATION_PATH "/mqtt_log_coord"
#define MASTER_URL "http://" MASTER_IP ":" HTTP_SERVER_PORT LOG_COORDINATION_PATH

#define WAIT_MASTER_TIMEOUT_SEC 10      // 等待主模组通知超时时间
#define WAIT_MASTER_ONLINE_TIMEOUT 60   // 等待主模组上线每次超时(秒)
#define ONLINE_CHECK_INTERVAL 60        // 在线检查间隔(秒)
#define ONLINE_CHECK_MAX_RETRIES 3      // 在线检查最大重试次数
#define WAIT_SLAVE_REPLY_TIMEOUT 10     // 主模组等待副模组回复超时(秒)

#define BATCH_NUMBER_MAX_LEN 64         // batchNumber最大长度
#define BATCH_NUMBER_REGEX "^[A-Za-z0-9_-]+$"  // batchNumber合法格式正则表达式

// ==================== 全局变量 ====================
static struct mg_context *g_httpCtx = NULL;
static pthread_mutex_t g_logCoordLock = PTHREAD_MUTEX_INITIALIZER;

// 副模组等待主模组通知状态
static int g_waitingForMaster = 0;
static char g_pendingBatchNumber[64] = {0};
static uint64_t g_pendingStartTime = 0;
static uint64_t g_pendingEndTime = 0;

// 主模组通知状态（副模组收到）
static int g_hasMasterNotify = 0;
static char g_masterNotifyBatch[64] = {0};
static uint64_t g_masterNotifyStart = 0;
static uint64_t g_masterNotifyEnd = 0;
static time_t g_masterNotifyTime = 0;  // 记录收到通知的时间戳

// 副模组回复状态（主模组收到）
static int g_hasSlaveReply = 0;
static char g_slaveReplyBatch[64] = {0};
static uint64_t g_slaveReplyStart = 0;
static uint64_t g_slaveReplyEnd = 0;

// 委托上传状态
static int g_hasUploadDelegate = 0;
static char g_delegateBatchNumber[64] = {0};
static uint64_t g_delegateStartTime = 0;
static uint64_t g_delegateEndTime = 0;

// 额外打包任务（主模组收到不同batchNumber时执行）
static int g_hasExtraUploadTask = 0;
static char g_extraBatchNumber[64] = {0};
static uint64_t g_extraStartTime = 0;
static uint64_t g_extraEndTime = 0;

// 主模组已处理的batchNumber记录（用于避免重复处理）
static char g_processedBatchNumber[64] = {0};
static int g_hasProcessedBatch = 0;

// ==================== 辅助函数 ====================

/**
 * @brief 校验batchNumber格式合法性（已注释，恢复老代码风格）
 * @param batchNumber 批次号
 * @retval  RET_OK    格式合法
 * @retval  RET_ERR   格式不合法
 *
 * 格式要求：
 * - 不能为空
 * - 长度不能超过64字节
 * - 只能包含字母、数字、下划线、连字符
 *
 * 注意：此功能已注释，恢复老代码风格，暂不进行合法性检查
 */
#if 0
static int ValidateBatchNumber(const char *batchNumber)
{
    if (NULL == batchNumber || strlen(batchNumber) == 0) {
        LOG_Error("ValidateBatchNumber: batchNumber is NULL or empty");
        return RET_ERR;
    }
    
    if (strlen(batchNumber) >= BATCH_NUMBER_MAX_LEN) {
        LOG_Error("ValidateBatchNumber: batchNumber length [%zu] >= %d", strlen(batchNumber), BATCH_NUMBER_MAX_LEN);
        return RET_ERR;
    }
    
    // 使用正则表达式校验格式
    regex_t regex;
    int ret = regcomp(&regex, BATCH_NUMBER_REGEX, REG_EXTENDED);
    if (ret != 0) {
        LOG_Error("ValidateBatchNumber: regcomp failed");
        return RET_ERR;
    }
    
    ret = regexec(&regex, batchNumber, 0, NULL, 0);
    regfree(&regex);
    
    if (ret != 0) {
        LOG_Error("ValidateBatchNumber: batchNumber format invalid: %s", batchNumber);
        return RET_ERR;
    }
    
    LOG_Info("ValidateBatchNumber: batchNumber format valid: %s", batchNumber);
    return RET_OK;
}
#endif

/**
 * @brief 从JSON字符串中提取字符串值
 * @retval  RET_OK    成功
 * @retval  RET_ERR   提取失败
 */
static int ExtractJsonString(const char *json, const char *key, char *value, int value_len)
{
    char search_key[128] = {0};
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    
    const char *pos = strstr(json, search_key);
    if (pos == NULL) return RET_ERR;
    
    pos += strlen(search_key);
    while (*pos == ' ' || *pos == ':') pos++;
    
    if (*pos != '"') return RET_ERR;
    pos++;
    
    const char *end = strchr(pos, '"');
    if (end == NULL) return RET_ERR;
    
    int len = (int)(end - pos);
    if (len >= value_len) len = value_len - 1;
    
    strncpy(value, pos, len);
    value[len] = '\0';
    return RET_OK;
}

/**
 * @brief 从JSON字符串中提取数字值
 * @retval  RET_OK    成功
 * @retval  RET_ERR   提取失败
 */
static int ExtractJsonNumber(const char *json, const char *key, uint64_t *value)
{
    char search_key[128] = {0};
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    
    const char *pos = strstr(json, search_key);
    if (pos == NULL) return RET_ERR;
    
    pos += strlen(search_key);
    while (*pos == ' ' || *pos == ':') pos++;
    
    *value = atoll(pos);
    return RET_OK;
}

// ==================== HTTP Server 相关 ====================

/**
 * @brief 写回调（接收请求体）
 */
static int request_handler(struct mg_connection *conn, void *cbdata)
{
    LOG_Info("request_handler: received HTTP request");
    TEST_LOG("HTTP request received on port 30001");
    
    const char *content_length = mg_get_header(conn, "Content-Length");
    if (content_length == NULL) {
        LOG_Error("request_handler: Missing Content-Length header");
        mg_send_http_error(conn, 400, "application/json", "{\"error\": \"Missing Content-Length\"}");
        return 400;
    }

    int len = atoi(content_length);
    if (len <= 0 || len >= 2048) {
        LOG_Error("request_handler: Invalid Content-Length: %d", len);
        mg_send_http_error(conn, 400, "application/json", "{\"error\": \"Invalid Content-Length\"}");
        return 400;
    }

    char post_data[2048] = {0};
    mg_read(conn, post_data, len);
    post_data[len] = '\0';

    LOG_Info("request_handler: received request body: %s", post_data);
    TEST_LOG("Request body: %s", post_data);

    // 提取cmdType
    char cmdType[64] = {0};
    if (ExtractJsonString(post_data, "cmdType", cmdType, sizeof(cmdType)) != 0) {
        LOG_Error("request_handler: Missing cmdType in request");
        mg_send_http_error(conn, 400, "application/json", "{\"error\": \"Missing cmdType\"}");
        return 400;
    }

    LOG_Info("request_handler: cmdType=%s", cmdType);
    TEST_LOG("cmdType=%s", cmdType);

    // 提取公共字段
    char batchNumber[64] = {0};
    uint64_t logStart = 0;
    uint64_t logEnd = 0;
    ExtractJsonString(post_data, "batchNumber", batchNumber, sizeof(batchNumber));
    ExtractJsonNumber(post_data, "logStart", &logStart);
    ExtractJsonNumber(post_data, "logEnd", &logEnd);

    if (batchNumber[0]) {
        LOG_Info("request_handler: batchNumber=%s, logStart=%lu, logEnd=%lu", batchNumber, logStart, logEnd);
        TEST_LOG("batchNumber=%s, logStart=%lu, logEnd=%lu", batchNumber, logStart, logEnd);
    }

    int ret = 200;
    char response[512] = {0};

    // 根据cmdType处理不同消息
    /* ==================== 测试用：模拟MQTT消息触发（测试完成后可删除） ==================== */
#if TEST_LOG_ENABLE
    if (strcmp(cmdType, "LOG_MQTT_SIMULATE") == 0) {
        TEST_LOG("LOG_MQTT_SIMULATE received, will trigger upload flow");
        if (batchNumber[0] && logStart && logEnd) {
            if (access("/dev/mmcblk0", F_OK) == 0) {
                TEST_LOG("Master module: calling MasterHandleLogUploadAsync");
                MasterHandleLogUploadAsync(batchNumber, logStart, logEnd);
                snprintf(response, sizeof(response),
                         "{\"result\": \"master simulation started\", \"batchNumber\": \"%s\"}", batchNumber);
            } else {
                TEST_LOG("Slave module: calling SlaveHandleLogUploadAsync");
                SlaveHandleLogUploadAsync(batchNumber, logStart, logEnd);
                snprintf(response, sizeof(response),
                         "{\"result\": \"slave simulation started\", \"batchNumber\": \"%s\"}", batchNumber);
            }
        } else {
            ret = 400;
            snprintf(response, sizeof(response),
                     "{\"error\": \"Missing batchNumber/logStart/logEnd\"}");
        }
    } else
#endif
    if (strcmp(cmdType, "LOG_MASTER_QUERY") == 0) {
        // 副模组查询主模组是否在线
        LOG_Info("request_handler: Processing LOG_MASTER_QUERY from slave module");
        TEST_LOG("LOG_MASTER_QUERY: checking master online status");
        snprintf(response, sizeof(response),
                 "{\"cmdType\": \"LOG_MASTER_ONLINE\", \"status\": \"online\"}");
        LOG_Info("request_handler: Responding to slave module query: online");
        TEST_LOG("LOG_MASTER_ONLINE response sent");

    } else if (strcmp(cmdType, "LOG_UPLOAD_DELEGATE") == 0) {
        // 副模组委托主模组上传日志
        LOG_Info("request_handler: Processing LOG_UPLOAD_DELEGATE from slave module");
        TEST_LOG("LOG_UPLOAD_DELEGATE: slave delegating upload to master");
        if (batchNumber[0] && logStart && logEnd) {
            // 保存委托信息
            pthread_mutex_lock(&g_logCoordLock);
            strncpy(g_delegateBatchNumber, batchNumber, 63);
            g_delegateStartTime = logStart;
            g_delegateEndTime = logEnd;
            g_hasUploadDelegate = 1;
            pthread_mutex_unlock(&g_logCoordLock);

            LOG_Info("request_handler: Saved delegate task: batchNumber=%s, startTime=%lu, endTime=%lu",
                     batchNumber, logStart, logEnd);
                    TEST_LOG("Delegate task saved: batchNumber=%s", batchNumber);

            // 执行日志上传
            LOG_Info("request_handler: Executing delegated log upload: batchNumber=%s", batchNumber);
            TEST_LOG("Executing ZelosControlCanFileUpload for delegate");
            ZelosControlCanFileUpload(logStart, logEnd, batchNumber);

            snprintf(response, sizeof(response),
                     "{\"cmdType\": \"LOG_UPLOAD_ACCEPT\", \"result\": \"accepted\"}");
            LOG_Info("request_handler: Accepted and completed log upload delegate: batchNumber=%s", batchNumber);
            TEST_LOG("LOG_UPLOAD_ACCEPT response sent");
        } else {
            LOG_Error("request_handler: Missing batchNumber/logStart/logEnd in LOG_UPLOAD_DELEGATE");
            ret = 400;
            snprintf(response, sizeof(response),
                     "{\"error\": \"Missing batchNumber/logStart/logEnd\"}");
        }

    } else if (strcmp(cmdType, "LOG_MASTER_NOTIFY") == 0) {
        // 主模组通知副模组
        LOG_Info("request_handler: Processing LOG_MASTER_NOTIFY from master module");
        TEST_LOG("LOG_MASTER_NOTIFY: master notifying slave");
        pthread_mutex_lock(&g_logCoordLock);
        // 先清除旧的通知状态，避免旧batchNumber被误用
        if (g_hasMasterNotify) {
            LOG_Info("request_handler: Clearing old master notify state: batchNumber=%s", g_masterNotifyBatch);
        }
        g_hasMasterNotify = 0;
        memset(g_masterNotifyBatch, 0, sizeof(g_masterNotifyBatch));
        g_masterNotifyStart = 0;
        g_masterNotifyEnd = 0;
        
        if (batchNumber[0]) {
            strncpy(g_masterNotifyBatch, batchNumber, 63);
            g_masterNotifyStart = logStart;
            g_masterNotifyEnd = logEnd;
            g_masterNotifyTime = time(NULL);  // 记录收到时间
            g_hasMasterNotify = 1;
            LOG_Info("request_handler: Received master notify: batchNumber=%s, startTime=%lu, endTime=%lu",
                     batchNumber, logStart, logEnd);
                    TEST_LOG("Master notify recorded: batchNumber=%s", batchNumber);
        } else {
            LOG_Error("request_handler: Empty batchNumber in LOG_MASTER_NOTIFY");
        }
        pthread_mutex_unlock(&g_logCoordLock);

        snprintf(response, sizeof(response),
                 "{\"cmdType\": \"LOG_MASTER_NOTIFY_ACK\", \"result\": \"received\"}");
        LOG_Info("request_handler: Sent LOG_MASTER_NOTIFY_ACK to master module");
        TEST_LOG("LOG_MASTER_NOTIFY_ACK response sent");

    } else if (strcmp(cmdType, "LOG_SLAVE_BATCH_REPLY") == 0) {
        // 副模组回复batchNumber给主模组
        LOG_Info("request_handler: Processing LOG_SLAVE_BATCH_REPLY from slave module");
        TEST_LOG("LOG_SLAVE_BATCH_REPLY: slave replying to master");
        pthread_mutex_lock(&g_logCoordLock);
        if (batchNumber[0]) {
            strncpy(g_slaveReplyBatch, batchNumber, 63);
            g_slaveReplyStart = logStart;
            g_slaveReplyEnd = logEnd;
            g_hasSlaveReply = 1;
            LOG_Info("request_handler: Received slave batch reply: batchNumber=%s, startTime=%lu, endTime=%lu",
                     batchNumber, logStart, logEnd);
                    TEST_LOG("Slave batch reply recorded: batchNumber=%s", batchNumber);
        } else {
            LOG_Error("request_handler: Empty batchNumber in LOG_SLAVE_BATCH_REPLY");
        }
        pthread_mutex_unlock(&g_logCoordLock);

        snprintf(response, sizeof(response),
                 "{\"cmdType\": \"LOG_SLAVE_BATCH_REPLY_ACK\", \"result\": \"received\"}");
        LOG_Info("request_handler: Sent LOG_SLAVE_BATCH_REPLY_ACK to slave module");
        TEST_LOG("LOG_SLAVE_BATCH_REPLY_ACK response sent");

    } else if (strcmp(cmdType, "LOG_UPLOAD_COMPLETE") == 0) {
        // 主模组通知副模组上传完成
        LOG_Info("request_handler: Processing LOG_UPLOAD_COMPLETE from master module");
        TEST_LOG("LOG_UPLOAD_COMPLETE: master notifying slave");
        snprintf(response, sizeof(response),
                 "{\"cmdType\": \"LOG_UPLOAD_COMPLETE_ACK\", \"result\": \"received\"}");
        LOG_Info("request_handler: Received upload complete notify, sent ACK");
        TEST_LOG("LOG_UPLOAD_COMPLETE_ACK response sent");

    } else {
        LOG_Error("request_handler: Unknown cmdType: %s", cmdType);
        ret = 400;
        snprintf(response, sizeof(response),
                 "{\"error\": \"Unknown cmdType: %s\"}", cmdType);
    }

    // 先发送响应头，再发送响应体（修复Content-Length为0的问题）
    mg_printf(conn, "HTTP/1.1 %d OK\r\n", ret);
    mg_printf(conn, "Content-Type: application/json\r\n");
    mg_printf(conn, "Content-Length: %zu\r\n", strlen(response));
    mg_printf(conn, "Connection: close\r\n");
    mg_printf(conn, "\r\n");
    mg_printf(conn, "%s", response);

    LOG_Info("request_handler: request processing completed, ret=%d", ret);
    TEST_LOG("HTTP response sent, ret=%d", ret);
    return ret;
}

/**
 * @brief 启动HTTP Server
 * @retval  RET_OK    成功
 * @retval  RET_ERR   HTTP Server启动失败
 */
int StartLogCoordinationHttpServer(void)
{
    if (g_httpCtx != NULL) {
        LOG_Info("HTTP Server already started");
        return RET_OK;
    }

    struct mg_callbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));

    const char *options[] = {
        "listening_ports", HTTP_SERVER_PORT,
        "request_timeout_ms", "5000",
        "num_threads", "2",
        0
    };

    g_httpCtx = mg_start(&callbacks, 0, options);
    if (g_httpCtx == NULL) {
        LOG_Error("Failed to start HTTP Server on port %s", HTTP_SERVER_PORT);
        return RET_ERR;
    }

    mg_set_request_handler(g_httpCtx, LOG_COORDINATION_PATH, request_handler, 0);

    LOG_Info("HTTP Server started on port %s, path: %s", HTTP_SERVER_PORT, LOG_COORDINATION_PATH);
    return RET_OK;
}

/**
 * @brief 停止HTTP Server
 */
void StopLogCoordinationHttpServer(void)
{
    if (g_httpCtx != NULL) {
        mg_stop(g_httpCtx);
        g_httpCtx = NULL;
        LOG_Info("HTTP Server stopped");
    }
}

// ==================== HTTP Client 相关 ====================

/**
 * @brief HTTP POST请求封装
 * @retval  HTTP状态码    成功
 * @retval  RET_ERR       curl初始化或执行失败
 */
static int HttpPostJson(const char *url, const char *json_data, char *response, int response_len)
{
    CURL *curl;
    CURLcode res;
    long http_code = 0;

    curl = curl_easy_init();
    if (!curl) {
        LOG_Error("curl_easy_init failed");
        return RET_ERR;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

    if (response && response_len > 0) {
        response[0] = '\0';  // 清空缓冲区
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    } else {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    }

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("curl_easy_perform failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return RET_ERR;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    return (int)http_code;
}

// ==================== 业务逻辑 ====================

/**
 * @brief 判断当前是否为主模组
 */
static int IsMasterModule(void)
{
    return (0 == access("/dev/mmcblk0", F_OK));
}

/**
 * @brief 查询主模组是否在线
 * @retval  1   在线
 * @retval  0   不在线
 */
int CheckMasterOnline(void)
{
    char response[256] = {0};
    char query_json[256] = {0};
    
    snprintf(query_json, sizeof(query_json),
             "{\"cmdType\": \"LOG_MASTER_QUERY\", \"timestamp\": %ld}", time(NULL));

    TEST_LOG("Sending LOG_MASTER_QUERY to master at %s", MASTER_URL);
    int http_code = HttpPostJson(MASTER_URL, query_json, response, sizeof(response));
    TEST_LOG("Master query response: http_code=%d, response=%s", http_code, response);
    
    if (http_code == 200 && strstr(response, "LOG_MASTER_ONLINE")) {
        LOG_Info("Master module is online");
        TEST_LOG("Master module is online (response contains LOG_MASTER_ONLINE)");
        return 1;  // 在线
    }
    
    LOG_Debug("Master module not online, http_code=%d", http_code);
    TEST_LOG("Master module not online, http_code=%d", http_code);
    return 0;  // 不在线
}

/**
 * @brief 通知主模组上传日志
 * @retval  RET_OK        成功
 * @retval  RET_ERR       HTTP请求失败
 */
int NotifyMasterUpload(const char *batchNumber, uint64_t startTime, uint64_t endTime)
{
    char response[256] = {0};
    char delegate_json[512] = {0};
    
    snprintf(delegate_json, sizeof(delegate_json),
             "{\"cmdType\": \"LOG_UPLOAD_DELEGATE\", "
             "\"batchNumber\": \"%s\", "
             "\"logStart\": %lu, "
             "\"logEnd\": %lu, "
             "\"source\": \"slave\", "
             "\"timestamp\": %ld}",
             batchNumber, startTime, endTime, time(NULL));

    TEST_LOG("Sending LOG_UPLOAD_DELEGATE to master: batchNumber=%s", batchNumber);
    int http_code = HttpPostJson(MASTER_URL, delegate_json, response, sizeof(response));
    TEST_LOG("Master delegate response: http_code=%d, response=%s", http_code, response);
    
    if (http_code == 200 && strstr(response, "LOG_UPLOAD_ACCEPT")) {
        LOG_Info("Master accepted log upload: batchNumber=%s", batchNumber);
        TEST_LOG("Master accepted log upload: batchNumber=%s", batchNumber);
        return RET_OK;
    }
    
    LOG_Error("Master failed to accept log upload: http_code=%d, response=%s", http_code, response);
    TEST_LOG("Master failed to accept log upload: http_code=%d", http_code);
    return RET_ERR;
}

/**
 * @brief 发送主模组通知给副模组
 * @retval  RET_OK    成功
 * @retval  RET_ERR   HTTP请求失败
 */
int NotifySlaveModule(const char *batchNumber, uint64_t startTime, uint64_t endTime)
{
    char response[256] = {0};
    char notify_json[512] = {0};
    char slave_url[128] = {0};
    
    snprintf(slave_url, sizeof(slave_url), "http://" SLAVE_IP ":" HTTP_SERVER_PORT LOG_COORDINATION_PATH);
    
    snprintf(notify_json, sizeof(notify_json),
             "{\"cmdType\": \"LOG_MASTER_NOTIFY\", "
             "\"batchNumber\": \"%s\", "
             "\"logStart\": %lu, "
             "\"logEnd\": %lu, "
             "\"timestamp\": %ld}",
             batchNumber, startTime, endTime, time(NULL));

    TEST_LOG("Sending LOG_MASTER_NOTIFY to slave at %s: batchNumber=%s", slave_url, batchNumber);
    int http_code = HttpPostJson(slave_url, notify_json, response, sizeof(response));
    TEST_LOG("Slave notify response: http_code=%d, response=%s", http_code, response);
    
    if (http_code == 200) {
        LOG_Info("Slave module received notify: batchNumber=%s", batchNumber);
        TEST_LOG("Slave module received notify: batchNumber=%s", batchNumber);
        return RET_OK;
    }
    
    LOG_Error("Failed to notify slave module: http_code=%d", http_code);
    TEST_LOG("Failed to notify slave module: http_code=%d", http_code);
    return RET_ERR;
}

/**
 * @brief 副模组回复batchNumber给主模组
 * @retval  RET_OK    成功
 * @retval  RET_ERR   HTTP请求失败
 */
int ReplyBatchToMaster(const char *batchNumber, uint64_t startTime, uint64_t endTime)
{
    char response[256] = {0};
    char reply_json[512] = {0};
    char master_url[128] = {0};
    
    snprintf(master_url, sizeof(master_url), "http://" MASTER_IP ":" HTTP_SERVER_PORT LOG_COORDINATION_PATH);
    
    snprintf(reply_json, sizeof(reply_json),
             "{\"cmdType\": \"LOG_SLAVE_BATCH_REPLY\", "
             "\"batchNumber\": \"%s\", "
             "\"logStart\": %lu, "
             "\"logEnd\": %lu, "
             "\"timestamp\": %ld}",
             batchNumber, startTime, endTime, time(NULL));

    int http_code = HttpPostJson(master_url, reply_json, response, sizeof(response));
    
    if (http_code == 200) {
        LOG_Info("Master received slave batch reply: batchNumber=%s", batchNumber);
        return RET_OK;
    }
    
    LOG_Error("Failed to reply batch to master: http_code=%d", http_code);
    return RET_ERR;
}

/**
 * @brief 清除协同状态（比较batchNumber后调用，防止旧batchNumber被误用）
 */
void ClearCoordinationState(void)
{
    LOG_Info("ClearCoordinationState: clearing all coordination state");
    
    pthread_mutex_lock(&g_logCoordLock);
    
    // 清除主模组通知状态
    if (g_hasMasterNotify) {
        LOG_Debug("ClearCoordinationState: clearing master notify state: batchNumber=%s", g_masterNotifyBatch);
    }
    g_hasMasterNotify = 0;
    memset(g_masterNotifyBatch, 0, sizeof(g_masterNotifyBatch));
    g_masterNotifyStart = 0;
    g_masterNotifyEnd = 0;
    g_masterNotifyTime = 0;
    
    // 清除副模组回复状态
    if (g_hasSlaveReply) {
        LOG_Debug("ClearCoordinationState: clearing slave reply state: batchNumber=%s", g_slaveReplyBatch);
    }
    g_hasSlaveReply = 0;
    memset(g_slaveReplyBatch, 0, sizeof(g_slaveReplyBatch));
    g_slaveReplyStart = 0;
    g_slaveReplyEnd = 0;
    
    // 清除委托上传状态
    if (g_hasUploadDelegate) {
        LOG_Debug("ClearCoordinationState: clearing upload delegate state: batchNumber=%s", g_delegateBatchNumber);
    }
    g_hasUploadDelegate = 0;
    memset(g_delegateBatchNumber, 0, sizeof(g_delegateBatchNumber));
    g_delegateStartTime = 0;
    g_delegateEndTime = 0;
    
    // 清除额外打包任务状态
    if (g_hasExtraUploadTask) {
        LOG_Debug("ClearCoordinationState: clearing extra upload task state: batchNumber=%s", g_extraBatchNumber);
    }
    g_hasExtraUploadTask = 0;
    memset(g_extraBatchNumber, 0, sizeof(g_extraBatchNumber));
    g_extraStartTime = 0;
    g_extraEndTime = 0;
    
    pthread_mutex_unlock(&g_logCoordLock);
    
    LOG_Info("ClearCoordinationState: all coordination state cleared");
}

/**
 * @brief 唤醒MCU给主模组上电
 * @retval  RET_OK    成功
 * @retval  RET_ERR   MCU唤醒失败
 */
static int WakeupMCUForMaster(void)
{
    LOG_Info("WakeupMCUForMaster: waking up MCU for master module power-on");
    
    // 同步数据唤醒源并唤醒系统（与现有代码mqtt_connect.cpp:704保持一致）
    PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
    LOG_Info("WakeupMCUForMaster: system wakeup completed");
    
    // 唤醒MCU（name="mcu"时mode参数无效，传入POWER_MODE或WAKE_MODE均可）
    int8_t ret = PmWakeupSetDevices("mcu", POWER_MODE);
    if (ret != RET_OK) {
        LOG_Error("WakeupMCUForMaster: PmWakeupSetDevices failed: ret=%d", ret);
        return RET_ERR;
    }
    
    // 等待MCU启动
    sleep(1);
    LOG_Info("WakeupMCUForMaster: MCU wakeup success");
    return RET_OK;
}

/**
 * @brief 等待主模组上线（带重试，每次60秒，共3次）
 * @retval  1   主模组在线
 * @retval  0   超时
 */
static int WaitForMasterOnline(void)
{
    int retry_count = 0;
    
    while (retry_count < ONLINE_CHECK_MAX_RETRIES) {
        LOG_Info("Checking master online status, attempt %d/%d (wait %d seconds each)",
                 retry_count + 1, ONLINE_CHECK_MAX_RETRIES, ONLINE_CHECK_INTERVAL);
        
        if (CheckMasterOnline()) {
            return 1;  // 主模组在线
        }
        
        retry_count++;
        if (retry_count < ONLINE_CHECK_MAX_RETRIES) {
            LOG_Info("Master not online, waiting %d seconds before retry...", ONLINE_CHECK_INTERVAL);
            sleep(ONLINE_CHECK_INTERVAL);
        }
    }
    
    LOG_Error("Master module not online after %d attempts (%d seconds each)",
              ONLINE_CHECK_MAX_RETRIES, ONLINE_CHECK_INTERVAL);
    return 0;  // 超时
}

/**
 * @brief 检查是否有主模组通知
 * @retval  1   有通知
 * @retval  0   无通知
 */
static int CheckMasterNotify(const char *batchNumber, uint64_t *startTime, uint64_t *endTime)
{
    pthread_mutex_lock(&g_logCoordLock);
    
    if (g_hasMasterNotify) {
        // 检查通知是否过期（超过5秒认为过期）
        time_t now = time(NULL);
        if (now - g_masterNotifyTime > 5) {
            LOG_Info("CheckMasterNotify: master notify expired (age=%ld seconds), clearing", now - g_masterNotifyTime);
            TEST_LOG("Master notify expired: batchNumber=%s (age=%ld seconds)", g_masterNotifyBatch, now - g_masterNotifyTime);
            g_hasMasterNotify = 0;
            memset(g_masterNotifyBatch, 0, sizeof(g_masterNotifyBatch));
            g_masterNotifyStart = 0;
            g_masterNotifyEnd = 0;
            g_masterNotifyTime = 0;
            pthread_mutex_unlock(&g_logCoordLock);
            return 0;  // 通知已过期
        }
        
        LOG_Info("CheckMasterNotify: received master notify: batchNumber=%s, startTime=%lu, endTime=%lu",
                 g_masterNotifyBatch, g_masterNotifyStart, g_masterNotifyEnd);
        strncpy((char*)batchNumber, g_masterNotifyBatch, 63);
        *startTime = g_masterNotifyStart;
        *endTime = g_masterNotifyEnd;
        g_hasMasterNotify = 0;  // 清除通知状态
        pthread_mutex_unlock(&g_logCoordLock);
        LOG_Info("CheckMasterNotify: returning notify status");
        return 1;  // 有通知
    }
    
    pthread_mutex_unlock(&g_logCoordLock);
    LOG_Debug("CheckMasterNotify: no master notify available");
    return 0;  // 无通知
}

/**
 * @brief 主模组等待副模组回复
 * @retval  1   收到回复
 * @retval  0   超时
 */
static int WaitForSlaveReply(char *replyBatch, uint64_t *replyStart, uint64_t *replyEnd, int timeout_sec)
{
    int wait_count = 0;
    
    LOG_Info("WaitForSlaveReply: waiting for slave reply, timeout=%d seconds", timeout_sec);
    
    while (wait_count < timeout_sec) {
        sleep(1);
        wait_count++;
        
        LOG_Debug("WaitForSlaveReply: waiting, elapsed=%d/%d seconds", wait_count, timeout_sec);
        
        pthread_mutex_lock(&g_logCoordLock);
        if (g_hasSlaveReply) {
            LOG_Info("WaitForSlaveReply: received slave reply: batchNumber=%s, startTime=%lu, endTime=%lu",
                     g_slaveReplyBatch, g_slaveReplyStart, g_slaveReplyEnd);
            strncpy(replyBatch, g_slaveReplyBatch, 63);
            *replyStart = g_slaveReplyStart;
            *replyEnd = g_slaveReplyEnd;
            g_hasSlaveReply = 0;  // 清除回复状态
            pthread_mutex_unlock(&g_logCoordLock);
            return 1;  // 收到回复
        }
        pthread_mutex_unlock(&g_logCoordLock);
    }
    
    LOG_Info("WaitForSlaveReply: timeout waiting for slave reply after %d seconds", timeout_sec);
    return 0;  // 超时
}

/**
 * @brief 副模组收到MQTT日志上传指令后的处理入口
 *
 * @param batchNumber 批次号
 * @param startTime 开始时间戳
 * @param endTime 结束时间戳
 * @retval  RET_OK    成功委托主模组上传
 * @retval  RET_ERR   失败（已降级为自己上传）
 */
int SlaveHandleLogUpload(const char *batchNumber, uint64_t startTime, uint64_t endTime)
{
    LOG_Info("SlaveHandleLogUpload: entry, batchNumber=[%s], startTime=%lu, endTime=%lu",
             batchNumber ? batchNumber : "NULL", startTime, endTime);
    TEST_LOG("Function entry: SlaveHandleLogUpload");
    TEST_LOG("batchNumber=[%s], startTime=%lu, endTime=%lu", batchNumber, startTime, endTime);
    
    if (IsMasterModule()) {
        LOG_Error("SlaveHandleLogUpload: This function should only be called on slave module");
        TEST_LOG("Module check failed: this is master module, not slave");
        return RET_ERR;
    }
    TEST_LOG("Module check passed: this is slave module");

    LOG_Info("SlaveHandleLogUpload: Slave module received log upload request: batchNumber=[%s]", batchNumber);
    TEST_LOG("Slave received log upload request: batchNumber=[%s]", batchNumber);

    // 步骤1：等待主模组HTTP通知（10秒）
    LOG_Info("SlaveHandleLogUpload: Waiting for master notify, timeout=%d seconds", WAIT_MASTER_TIMEOUT_SEC);
    TEST_LOG("Waiting for master notify, timeout=%d seconds", WAIT_MASTER_TIMEOUT_SEC);
    
    int wait_count = 0;
    int received_master_notify = 0;
    char notifyBatch[64] = {0};
    uint64_t notifyStart = 0;
    uint64_t notifyEnd = 0;
    
    while (wait_count < WAIT_MASTER_TIMEOUT_SEC) {
        sleep(1);
        wait_count++;
        
        LOG_Debug("SlaveHandleLogUpload: waiting for master notify, elapsed=%d/%d seconds",
                  wait_count, WAIT_MASTER_TIMEOUT_SEC);
        
        // 检查是否有主模组通知
        if (CheckMasterNotify(notifyBatch, &notifyStart, &notifyEnd)) {
            received_master_notify = 1;
            LOG_Info("SlaveHandleLogUpload: Received master notify: batchNumber=[%s], startTime=%lu, endTime=%lu",
                     notifyBatch, notifyStart, notifyEnd);
                    TEST_LOG("Received master notify: batchNumber=[%s]", notifyBatch);
                    break;
        }
    }

    if (received_master_notify) {
        // 收到主模组通知，副模组不做任何动作
        LOG_Info("SlaveHandleLogUpload: Received master notify within %d seconds, slave module will do nothing",
                 wait_count);
                TEST_LOG("Received master notify within %d seconds, slave will do nothing", wait_count);
        
        // 清除主模组通知状态，防止旧batchNumber被误用
        pthread_mutex_lock(&g_logCoordLock);
        g_hasMasterNotify = 0;
        memset(g_masterNotifyBatch, 0, sizeof(g_masterNotifyBatch));
        g_masterNotifyStart = 0;
        g_masterNotifyEnd = 0;
        g_masterNotifyTime = 0;
        pthread_mutex_unlock(&g_logCoordLock);
        LOG_Debug("SlaveHandleLogUpload: Cleared master notify state");
        TEST_LOG("Cleared master notify state");
        
        TEST_LOG("RET_OK (received master notify)");
        return RET_OK;
    }

    // 步骤2：超时未收到通知，唤醒MCU给主模组上电
    LOG_Info("SlaveHandleLogUpload: Timeout waiting for master notify after %d seconds, wakeup MCU",
             WAIT_MASTER_TIMEOUT_SEC);
    TEST_LOG("Timeout waiting for master notify after %d seconds, will wakeup MCU", WAIT_MASTER_TIMEOUT_SEC);
    if (WakeupMCUForMaster() != RET_OK) {
        LOG_Error("SlaveHandleLogUpload: Failed to wakeup MCU");
        TEST_LOG("MCU wakeup failed");
        return RET_ERR;
    }
    LOG_Info("SlaveHandleLogUpload: MCU wakeup success");
    TEST_LOG("MCU wakeup success");

    // 步骤3：等待主模组上线（每次60秒，3次重试）
    LOG_Info("SlaveHandleLogUpload: Waiting for master module to come online (max %d attempts, %d seconds each)...",
             ONLINE_CHECK_MAX_RETRIES, ONLINE_CHECK_INTERVAL);
    TEST_LOG("Waiting for master module to come online (max %d attempts, %d seconds each)",
             ONLINE_CHECK_MAX_RETRIES, ONLINE_CHECK_INTERVAL);
    if (!WaitForMasterOnline()) {
        LOG_Error("SlaveHandleLogUpload: Master module not online after %d attempts (%d seconds each)",
                  ONLINE_CHECK_MAX_RETRIES, ONLINE_CHECK_INTERVAL);
                 TEST_LOG("Master module not online after %d attempts", ONLINE_CHECK_MAX_RETRIES);
                 TEST_LOG("RET_ERR (master not online)");
                 return RET_ERR;
    }
    LOG_Info("SlaveHandleLogUpload: Master module is online");
    TEST_LOG("Master module is online");

    // 步骤4：通知主模组上传日志
    LOG_Info("SlaveHandleLogUpload: Notifying master module to upload log: batchNumber=[%s]", batchNumber);
    TEST_LOG("Notifying master module to upload log: batchNumber=[%s]", batchNumber);
    if (NotifyMasterUpload(batchNumber, startTime, endTime) != RET_OK) {
        LOG_Error("SlaveHandleLogUpload: Failed to notify master module");
        TEST_LOG("Failed to notify master module");
        return RET_ERR;
    }

    LOG_Info("SlaveHandleLogUpload: Successfully delegated log upload to master module: batchNumber=[%s]", batchNumber);
    TEST_LOG("Successfully delegated log upload to master module");
    TEST_LOG("RET_OK (delegated to master)");
    return RET_OK;
}

/**
 * @brief 异步线程执行函数，调用SlaveHandleLogUpload
 */
typedef struct {
    char batchNumber[64];
    uint64_t startTime;
    uint64_t endTime;
} SlaveUploadThreadArg_t;

static void *SlaveHandleLogUploadThread(void *arg)
{
    SlaveUploadThreadArg_t *uploadArg = (SlaveUploadThreadArg_t *)arg;
    if (uploadArg == NULL) {
        return NULL;
    }
    
    LOG_Info("SlaveHandleLogUploadThread: started in separate thread, batchNumber=[%s]", uploadArg->batchNumber);
    
    int ret = SlaveHandleLogUpload(uploadArg->batchNumber, uploadArg->startTime, uploadArg->endTime);
    
    LOG_Info("SlaveHandleLogUploadThread: completed, ret=%d, batchNumber=[%s]", ret, uploadArg->batchNumber);
    
    free(uploadArg);
    return NULL;
}

int SlaveHandleLogUploadAsync(const char *batchNumber, uint64_t startTime, uint64_t endTime)
{
    pthread_t threadId;
    pthread_attr_t attr;
    
    SlaveUploadThreadArg_t *uploadArg = (SlaveUploadThreadArg_t *)malloc(sizeof(SlaveUploadThreadArg_t));
    if (uploadArg == NULL) {
        LOG_Error("SlaveHandleLogUploadAsync: Failed to allocate memory for thread argument");
        return RET_ERR;
    }
    
    strncpy(uploadArg->batchNumber, batchNumber, 63);
    uploadArg->batchNumber[63] = '\0';
    uploadArg->startTime = startTime;
    uploadArg->endTime = endTime;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    int ret = pthread_create(&threadId, &attr, SlaveHandleLogUploadThread, uploadArg);
    pthread_attr_destroy(&attr);
    
    if (ret != 0) {
        LOG_Error("SlaveHandleLogUploadAsync: Failed to create thread: %s", strerror(ret));
        free(uploadArg);
        return RET_ERR;
    }
    
    LOG_Info("SlaveHandleLogUploadAsync: Created separate thread for batchNumber=[%s]", batchNumber);
    return RET_OK;
}

/**
 * @brief 主模组收到MQTT日志上传指令后的处理入口
 *
 * @param batchNumber 批次号
 * @param startTime 开始时间戳
 * @param endTime 结束时间戳
 * @retval  RET_OK    成功
 * @retval  RET_ERR   失败
 */
int MasterHandleLogUpload(const char *batchNumber, uint64_t startTime, uint64_t endTime)
{
    LOG_Info("MasterHandleLogUpload: entry, batchNumber=[%s], startTime=%lu, endTime=%lu",
             batchNumber ? batchNumber : "NULL", startTime, endTime);
    TEST_LOG("Function entry: MasterHandleLogUpload");
    TEST_LOG("batchNumber=[%s], startTime=%lu, endTime=%lu", batchNumber, startTime, endTime);
    
    if (!IsMasterModule()) {
        LOG_Error("MasterHandleLogUpload: This function should only be called on master module");
        TEST_LOG("Module check failed: this is slave module, not master");
        return RET_ERR;
    }
    TEST_LOG("Module check passed: this is master module");

    // 检查是否已经处理过该batchNumber（可能是副模组委托的）
    pthread_mutex_lock(&g_logCoordLock);
    if (g_hasProcessedBatch && batchNumber && strcmp(g_processedBatchNumber, batchNumber) == 0) {
        LOG_Info("MasterHandleLogUpload: batchNumber=[%s] already processed, ignoring", batchNumber);
        TEST_LOG("batchNumber=[%s] already processed, ignoring", batchNumber);
        pthread_mutex_unlock(&g_logCoordLock);
        TEST_LOG("RET_OK (already processed)");
        return RET_OK;
    }
    pthread_mutex_unlock(&g_logCoordLock);
    TEST_LOG("batchNumber=[%s] not processed yet, continuing", batchNumber);

    LOG_Info("MasterHandleLogUpload: Master module received log upload request: batchNumber=[%s]", batchNumber);
    TEST_LOG("Master received log upload request: batchNumber=[%s]", batchNumber);

    // 等待3秒再通知副模组，避免副模组先收到MQTT消息导致时序问题
    LOG_Info("MasterHandleLogUpload: Waiting 3 seconds before notifying slave module...");
    TEST_LOG("Waiting 3 seconds before notifying slave module...");
    sleep(3);
    TEST_LOG("3 seconds wait completed");

    // 步骤1：通知副模组（不管副模组是否回复）
    LOG_Info("MasterHandleLogUpload: Notifying slave module: batchNumber=[%s]", batchNumber);
    TEST_LOG("Notifying slave module: batchNumber=[%s]", batchNumber);
    int notify_ret = NotifySlaveModule(batchNumber, startTime, endTime);
    if (notify_ret != RET_OK) {
        LOG_Error("MasterHandleLogUpload: NotifySlaveModule failed, ret=%d", notify_ret);
        TEST_LOG("NotifySlaveModule failed, ret=%d", notify_ret);
    } else {
        LOG_Debug("MasterHandleLogUpload: NotifySlaveModule success");
        TEST_LOG("NotifySlaveModule success");
    }

    // 步骤2：执行主模组日志上传（与老代码一致）
    LOG_Info("MasterHandleLogUpload: Executing main upload: batchNumber=[%s], startTime=%lu, endTime=%lu",
             batchNumber, startTime, endTime);
        TEST_LOG("Executing main upload: batchNumber=[%s]", batchNumber);
        ZelosControlCanFileUpload(startTime, endTime, batchNumber);
        LOG_Info("MasterHandleLogUpload: Main upload completed: batchNumber=[%s]", batchNumber);
        TEST_LOG("Main upload completed: batchNumber=[%s]", batchNumber);

    // 记录已处理的batchNumber
    pthread_mutex_lock(&g_logCoordLock);
    if (batchNumber) {
        strncpy(g_processedBatchNumber, batchNumber, sizeof(g_processedBatchNumber) - 1);
        g_processedBatchNumber[sizeof(g_processedBatchNumber) - 1] = '\0';
        g_hasProcessedBatch = 1;
    }
    pthread_mutex_unlock(&g_logCoordLock);
    TEST_LOG("Recorded processed batchNumber=[%s]", batchNumber);

    // 上传完成后清除已处理记录，允许后续相同batchNumber的合法重试
    pthread_mutex_lock(&g_logCoordLock);
    g_hasProcessedBatch = 0;
    memset(g_processedBatchNumber, 0, sizeof(g_processedBatchNumber));
    pthread_mutex_unlock(&g_logCoordLock);
    TEST_LOG("Cleared processed batchNumber record");

    TEST_LOG("RET_OK (upload completed)");
    return RET_OK;
}

/**
 * @brief 异步线程执行函数，调用MasterHandleLogUpload
 */
typedef struct {
    char batchNumber[64];
    uint64_t startTime;
    uint64_t endTime;
} MasterUploadThreadArg_t;

static void *MasterHandleLogUploadThread(void *arg)
{
    MasterUploadThreadArg_t *uploadArg = (MasterUploadThreadArg_t *)arg;
    if (uploadArg == NULL) {
        return NULL;
    }
    
    LOG_Info("MasterHandleLogUploadThread: started in separate thread, batchNumber=[%s]", uploadArg->batchNumber);
    
    int ret = MasterHandleLogUpload(uploadArg->batchNumber, uploadArg->startTime, uploadArg->endTime);
    
    LOG_Info("MasterHandleLogUploadThread: completed, ret=%d, batchNumber=[%s]", ret, uploadArg->batchNumber);
    
    free(uploadArg);
    return NULL;
}

int MasterHandleLogUploadAsync(const char *batchNumber, uint64_t startTime, uint64_t endTime)
{
    pthread_t threadId;
    pthread_attr_t attr;
    
    MasterUploadThreadArg_t *uploadArg = (MasterUploadThreadArg_t *)malloc(sizeof(MasterUploadThreadArg_t));
    if (uploadArg == NULL) {
        LOG_Error("MasterHandleLogUploadAsync: Failed to allocate memory for thread argument");
        return RET_ERR;
    }
    
    strncpy(uploadArg->batchNumber, batchNumber, 63);
    uploadArg->batchNumber[63] = '\0';
    uploadArg->startTime = startTime;
    uploadArg->endTime = endTime;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    int ret = pthread_create(&threadId, &attr, MasterHandleLogUploadThread, uploadArg);
    pthread_attr_destroy(&attr);
    
    if (ret != 0) {
        LOG_Error("MasterHandleLogUploadAsync: Failed to create thread: %s", strerror(ret));
        free(uploadArg);
        return RET_ERR;
    }
    
    LOG_Info("MasterHandleLogUploadAsync: Created separate thread for batchNumber=[%s]", batchNumber);
    return RET_OK;
}

/**
 * @brief 获取委托上传状态（供主模组内部调用）
 * @retval  1   有委托任务
 * @retval  0   无委托任务
 */
int GetUploadDelegateStatus(char *batchNumber, uint64_t *startTime, uint64_t *endTime)
{
    pthread_mutex_lock(&g_logCoordLock);
    if (g_hasUploadDelegate) {
        LOG_Info("GetUploadDelegateStatus: has delegate task: batchNumber=%s, startTime=%lu, endTime=%lu",
                 g_delegateBatchNumber, g_delegateStartTime, g_delegateEndTime);
        strncpy(batchNumber, g_delegateBatchNumber, 63);
        *startTime = g_delegateStartTime;
        *endTime = g_delegateEndTime;
        g_hasUploadDelegate = 0;
        pthread_mutex_unlock(&g_logCoordLock);
        return 1;  // 有委托
    }
    pthread_mutex_unlock(&g_logCoordLock);
    LOG_Debug("GetUploadDelegateStatus: no delegate task available");
    return 0;  // 无委托
}

/**
 * @brief 获取额外打包任务状态
 * @retval  1   有额外任务
 * @retval  0   无额外任务
 */
int GetExtraUploadTask(char *batchNumber, uint64_t *startTime, uint64_t *endTime)
{
    pthread_mutex_lock(&g_logCoordLock);
    if (g_hasExtraUploadTask) {
        LOG_Info("GetExtraUploadTask: has extra upload task: batchNumber=%s, startTime=%lu, endTime=%lu",
                 g_extraBatchNumber, g_extraStartTime, g_extraEndTime);
        strncpy(batchNumber, g_extraBatchNumber, 63);
        *startTime = g_extraStartTime;
        *endTime = g_extraEndTime;
        g_hasExtraUploadTask = 0;
        pthread_mutex_unlock(&g_logCoordLock);
        return 1;  // 有额外任务
    }
    pthread_mutex_unlock(&g_logCoordLock);
    LOG_Debug("GetExtraUploadTask: no extra upload task available");
    return 0;  // 无额外任务
}
