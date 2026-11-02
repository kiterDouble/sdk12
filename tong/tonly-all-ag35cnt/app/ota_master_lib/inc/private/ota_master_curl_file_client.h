/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-03-14
 * Description: libcurl文件传输客户端
 *********************************************************/
#ifndef _OTA_MASTER_CURL_FILE_CLIENT_H
#define _OTA_MASTER_CURL_FILE_CLIENT_H

#include <stdint.h>
#include <unistd.h>
#include <curl/curl.h>
#include <string>
#include <memory>
#include "ota_master_inner_code.h"

/**
 * @brief 进度数据
 */
struct CurlProgressData {
    curl_off_t lastProgress; // 上次下载位置
    curl_off_t totalSize;    // 预设的总大小（当服务器未返回dltotal时使用）
};

/**
 * @brief 文件信息
 */
struct CurlFileInfo {
    FILE *file;      // 文件句柄
    std::string url; // 下载URL
};

/**
 * @brief 文件删除器
 */
struct FileDeleter {
    void operator()(FILE *fd) const
    {
        if (fd) {
            fclose(fd);
        }
    }
};

/**
 * @brief 文件句柄
 */
using FileHandle = std::unique_ptr<FILE, FileDeleter>;

/**
 * @brief 写入回调函数
 */
using WriteCallback = size_t (*)(void *, size_t, size_t, void *);

/**
 * @brief 头部回调函数
 */
using HeaderCallback = size_t (*)(char *, size_t, size_t, void *);

/**
 * @brief 进度回调函数
 */
using ProgressCallback = int (*)(void *, curl_off_t, curl_off_t, curl_off_t, curl_off_t);

/**
 * @brief 通过CURL获取文件扩展名
 * 
 * @param[in] url 下载URL
 * 
 * @return 文件扩展名
 */
std::string GetHeaderInfoViaCurl(const std::string &url, HeaderCallback headerCb);

/**
 * @brief 获取下载文件总大小
 *
 * @param[in] url 下载URL
 *
 * @return 下载文件大小
 */
size_t GetDownloadTotalSizeViaCurl(const std::string &url);

/**
 * @brief 通过CURL下载文件
 * 
 * @param[in] file 文件句柄
 * @param[in] data 进度数据
 * @param[in] url 下载URL
 * @param[in] writeCb 写入回调函数
 * @param[in] processCb 进度回调函数
 * 
 * @return 下载结果
 */
OtaFileDownloadErrorCode DownloadFileViaCurl(const CurlFileInfo &fileInfo, const CurlProgressData &data,
                                             WriteCallback writeCb, ProgressCallback processCb);

/**
 * @brief 使用POST方式上传文件
 * 
 * @param[in] url 上传URL
 * @param[in] filePath 上传文件路径
 * @param[in] writeCb 写入回调函数
 * @param[in] processCb 进度回调函数
 * 
 * @return
 *      OTA_RET_OK：上传成功
 *      OTA_RET_ERR：上传失败
 */
int UploadFileViaPost(const std::string &url, const std::string &filePath, WriteCallback writeCb,
                      ProgressCallback processCb);

                      /**
 * @brief 使用formdata方式上传文件
 *
 * @param[in] url 上传URL
 * @param[in] filePath 上传文件路径
 * @param[in] taskId 任务ID
 * @param[in] writeCb 写入回调函数
 * @param[in] processCb 进度回调函数
 * 
 * @return
 *      OTA_RET_OK：上传成功
 *      OTA_RET_ERR：上传失败
 */
int UploadFileViaFormData(const std::string &url, const std::string &filePath, const std::string &taskId,
                          WriteCallback writeCb, ProgressCallback processCb);

#endif // _OTA_MASTER_CURL_FILE_CLIENT_H