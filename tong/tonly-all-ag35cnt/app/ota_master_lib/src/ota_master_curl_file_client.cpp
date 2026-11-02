#include <unistd.h>
#include <cstdio>
#include <cerrno>
#include <memory>
#include "ota_master_curl_file_client.h"
#include "ota_master_utils.h"
#include "ota_master_log.h"

// 错误处理宏
#define CHECK_MIME_PART(form, part)        \
    if (!(part)) {                         \
        OTA_LOG_E("Add mime part failed"); \
        if (form) {                        \
            curl_mime_free(form);          \
        }                                  \
        return OTA_RET_ERR;                \
    }


/**
 * @brief 全局CURL
 */
struct CurlGlobal {
    CurlGlobal()
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~CurlGlobal()
    {
        curl_global_cleanup();
    }
};

/**
 * @brief CURL删除器
 */
struct CurlDeleter {
    void operator()(CURL *curl)
    {
        if (curl) {
            curl_easy_cleanup(curl);
        }
    }
};

using CurlHandle = std::unique_ptr<CURL, CurlDeleter>; // CURL句柄

/**
 * @brief 自定义调试日志回调函数
 * 
 * @param[in] handle CURL句柄
 * @param[in] type 日志类型
 * @param[in] data 日志数据
 * @param[in] size 数据大小
 * @param[in] userptr 用户数据
 * 
 * @return 默认0
 */
static int DebugCallback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr)
{
    // 过滤日志类型（例如仅处理文本类日志）
    if (type == CURLINFO_TEXT || type == CURLINFO_HEADER_IN || type == CURLINFO_HEADER_OUT) {
        // 将日志数据转换为字符串（注意：data可能不包含终止符）
        std::string msg(data, size);
        // 调用应用日志接口（假设应用日志接口为 OTA_LOG_D）
        OTA_LOG_D("%s", msg.c_str());
    }
    return 0;
}

std::string GetHeaderInfoViaCurl(const std::string &url, HeaderCallback headerCb)
{
    if (headerCb == nullptr) {
        OTA_LOG_E("Header callback is nullptr");
        return "";
    }

    // 全局CURL
    CurlGlobal curlGlobal;
    // 初始化CURL
    CurlHandle curl(curl_easy_init());
    if (curl == nullptr) {
        OTA_LOG_E("Init curl failed");
        return "";
    }

    std::string headers;
    // 设置日志输出
    curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);
    // 设置调试回调函数
    curl_easy_setopt(curl.get(), CURLOPT_DEBUGFUNCTION, DebugCallback);
    // 设置下载url
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    // 发送HEAD请求
    curl_easy_setopt(curl.get(), CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, headerCb);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, &headers);
    // 设置禁用证书验证
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);
    CURLcode res = curl_easy_perform(curl.get());
    if (res != CURLE_OK) {
        OTA_LOG_E("Get extension header info failed, error:%d : %s\n", res, curl_easy_strerror(res));
        return "";
    }
    OTA_LOG_D("Get extension header info: %s", headers.c_str());
    return headers;
}

size_t GetDownloadTotalSizeViaCurl(const std::string &url)
{
    // 全局CURL
    CurlGlobal curlGlobal;
    // 初始化CURL
    CurlHandle curl(curl_easy_init());
    if (curl == nullptr) {
        OTA_LOG_E("Init curl failed");
        return 0;
    }

    // 设置日志输出
    curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);
    // 设置调试回调函数
    curl_easy_setopt(curl.get(), CURLOPT_DEBUGFUNCTION, DebugCallback);
    // 设置下载url
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    // 设置只获取头部信息
    curl_easy_setopt(curl.get(), CURLOPT_NOBODY, 1L);
    // 自动处理重定向
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    // 连接超时5秒
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 5L);
    // 设置禁用证书验证
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl.get());
    if (res != CURLE_OK) {
        OTA_LOG_E("Get download file size failed, error code: %d", res);
        return 0;
    }

    long fileSize = 0;
#if LIBCURL_VERSION_NUM >= 0x073700
    curl_off_t cl = 0;
    res = curl_easy_getinfo(curl.get(), CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &cl);
    fileSize = (long)cl;
#else
    double total = 0;
    res = curl_easy_getinfo(curl.get(), CURLINFO_CONTENT_LENGTH_DOWNLOAD, &total);
    fileSize = (long)total;
#endif
    if (res != CURLE_OK) {
        OTA_LOG_E("Get download file size failed, error code: %d", res);
        return 0;
    }
    OTA_LOG_D("Get download file[%s] size: %f", url.c_str(), fileSize);
    return static_cast<size_t>(fileSize);
}

OtaFileDownloadErrorCode DownloadFileViaCurl(const CurlFileInfo &fileInfo, const CurlProgressData &data,
                                         WriteCallback writeCb, ProgressCallback processCb)
{
    if (fileInfo.file == nullptr) {
        OTA_LOG_E("File handle is nullptr");
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_FILE_PATH_ERROR;
    }

    // 全局CURL
    CurlGlobal curlGlobal;
    // 初始化CURL
    CurlHandle curl(curl_easy_init());
    if (curl == nullptr) {
        OTA_LOG_E("Init curl failed");
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_CURL_ERROR;
    }

    // 设置断点续传起始位置
    if (data.lastProgress != 0) {
        OTA_LOG_D("Resume from offset: %ld", data.lastProgress);
        curl_easy_setopt(curl.get(), CURLOPT_RESUME_FROM_LARGE, static_cast<curl_off_t>(data.lastProgress));
    } else {
        // 新文件时truncate确保文件为空
        ftruncate(fileno(fileInfo.file), 0);
    }
    curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);                      // 设置日志输出
    curl_easy_setopt(curl.get(), CURLOPT_DEBUGFUNCTION, DebugCallback);     // 设置调试回调函数
    curl_easy_setopt(curl.get(), CURLOPT_URL, fileInfo.url.c_str());        // 设置下载url
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, fileInfo.file);         // 设置下载文件路径
    if (writeCb) {
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeCb);       // 设置回调
    }
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 5L);               // 连接超时5秒
    curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_LIMIT, 1L);              // 传输限速1B/s
    curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_TIME, 10L);              // 低速持续10秒则超时
    curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);                   // 设置进度回调函数
    if (processCb) {
        curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, processCb);  // 进度回调
        curl_easy_setopt(curl.get(), CURLOPT_PROGRESSDATA, &data);          // 传递结构体
    }
    // 设置禁用证书验证
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);

    // 开始下载
    CURLcode res = curl_easy_perform(curl.get());
    if (res != CURLE_OK) {
        OTA_LOG_E("Download file:%s failed, error[%d]: %s", fileInfo.url.c_str(), res, curl_easy_strerror(res));
        if (res == CURLE_COULDNT_CONNECT || res == CURLE_OPERATION_TIMEOUTED) {
            return OtaFileDownloadErrorCode::OTA_DOWNLOAD_NETWORK_ABNORMAL;
        } else if (res == CURLE_ABORTED_BY_CALLBACK) {
            return OtaFileDownloadErrorCode::OTA_DOWNLOAD_STOPED;
        }
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_CURL_ERROR;
    }
    OTA_LOG_D("Download file:%s success", fileInfo.url.c_str());
    return OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK;
}

int UploadFileViaPost(const std::string &url, const std::string &filePath, WriteCallback writeCb,
                      ProgressCallback processCb)
{
    auto fileSize = GetFileSize(filePath);
    OTA_LOG_D("Upload file:%s, size:%ld", filePath.c_str(), fileSize);
    if (fileSize == 0) {
        OTA_LOG_E("Upload file:%s via curl failed, get file size failed", filePath.c_str());
        return OTA_RET_ERR;
    }

    // 全局CURL
    CurlGlobal curlGlobal;
    // 初始化CURL
    CurlHandle curl(curl_easy_init());
    if (curl == nullptr) {
        OTA_LOG_E("Upload file:%s via curl failed, init curl failed", filePath.c_str());
        return OTA_RET_ERR;
    }

    // 打开文件
    FileHandle file(fopen(filePath.c_str(), "ab+"));
    if (file == nullptr) {
        OTA_LOG_E("Open file %s failed", filePath.c_str());
        return OTA_RET_ERR;
    }

    curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);                  // 设置日志输出
    curl_easy_setopt(curl.get(), CURLOPT_DEBUGFUNCTION, DebugCallback); // 设置调试回调函数
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());             // 设置上传url
    curl_easy_setopt(curl.get(), CURLOPT_UPLOAD, 1L);                   // 上传文件
    curl_easy_setopt(curl.get(), CURLOPT_READDATA, file.get());         // 上传的文件句柄
    curl_easy_setopt(curl.get(), CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(fileSize)); // 上传文件大小
    if (writeCb) {
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeCb); // 设置回调
    }

    std::string response;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);             // 处理响应
    if (processCb) {
        curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, processCb);  // 进度回调
        curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);               // 启用进度回调
    }
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 10L);              // 连接超时 10 秒

    // 执行请求
    CURLcode res = curl_easy_perform(curl.get());
    if (res != CURLE_OK) {
        OTA_LOG_E("Upload file:%s via curl failed, error[%d]: %s", filePath.c_str(), res, curl_easy_strerror(res));
        return OTA_RET_ERR;
    }
    OTA_LOG_D("Upload file:%s via curl success, response:%s", filePath.c_str(), response.c_str());
    return OTA_RET_OK;
}

int UploadFileViaFormData(const std::string &url, const std::string &filePath, const std::string &taskId,
                          WriteCallback writeCb, ProgressCallback processCb)
{
    OTA_LOG_I("Upload file via form data, task:%s, url:%s, filePath:%s", taskId.c_str(), url.c_str(), filePath.c_str());
    // 全局CURL
    CurlGlobal curlGlobal;
    // 初始化CURL
    CurlHandle curl(curl_easy_init());
    if (curl == nullptr) {
        OTA_LOG_E("Upload file:%s via curl failed, init curl failed", filePath.c_str());
        return OTA_RET_ERR;
    }

    curl_mime *form = curl_mime_init(curl.get());
    if (form == nullptr) {
        OTA_LOG_E("Init mime failed");
        return OTA_RET_ERR;
    }

    // 文件字段
    curl_mimepart *fileField = curl_mime_addpart(form);
    CHECK_MIME_PART(form, fileField);
    curl_mime_name(fileField, "file");
    curl_mime_filedata(fileField, filePath.c_str());
    // curl_mime_filename(fileField, "OTA_MASTER_LOG.zip");
    curl_mime_type(fileField, "application/zip");

    // 任务ID字段
    curl_mimepart *taskField = curl_mime_addpart(form);
    CHECK_MIME_PART(form, taskField);
    curl_mime_name(taskField, "taskId");
    curl_mime_data(taskField, taskId.c_str(), CURL_ZERO_TERMINATED);

    // 描述字段
    curl_mimepart *descField = curl_mime_addpart(form);
    CHECK_MIME_PART(form, descField);
    curl_mime_name(descField, "description");
    curl_mime_data(descField, "ZIP Archive", CURL_ZERO_TERMINATED);

    // 设置选项
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_MIMEPOST, form);
    // 跳过证书验证（测试环境）
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);
    if (writeCb) {
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeCb); // 设置回调
    }

    std::string response;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);             // 处理响应
    if (processCb) {
        curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, processCb);  // 进度回调
        curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);               // 启用进度回调
    }
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 10L);              // 连接超时 10 秒

    // 执行请求
    CURLcode res = curl_easy_perform(curl.get());
    if (res != CURLE_OK) {
        OTA_LOG_E("Upload file:%s via curl failed, error[%d]: %s", filePath.c_str(), res, curl_easy_strerror(res));
        curl_mime_free(form);
        return OTA_RET_ERR;
    }
    curl_mime_free(form);
    OTA_LOG_D("Upload file:%s via curl success, response:%s", filePath.c_str(), response.c_str());
    return OTA_RET_OK;
}