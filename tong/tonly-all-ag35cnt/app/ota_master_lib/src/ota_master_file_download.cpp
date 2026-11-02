#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <atomic>
#include <array>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <fstream>
#include "ota_master_file_download.h"
#include "ota_master_curl_file_client.h"
#include "ota_master_inner_code.h"
#include "ota_master_utils.h"
#include "ota_master_log.h"

static constexpr const char *BACKUP_MD5_FILE = "backup.md5"; // 备份文件MD5文件名
static constexpr const char *MD5_BEGIN = "MD5 = ";           // MD5起始标志
static constexpr const char *SYMBOL_CRLF = "\n";             // 换行符

static std::atomic<bool> g_isFileDownloading(false);         // 文件下载状态

/**
 * @brief 从下载URL提取文件扩展名
 *
 * @param[in] downloadUrl 下载URL
 *
 * @return 下载文件扩展名
 */
static std::string ExtractExtensionFromUrl(const std::string &downloadUrl)
{
    OTA_LOG_D("Extract download file extension from url: %s", downloadUrl.c_str());
    // 1. 分离路径和参数
    auto paramPos = downloadUrl.find('?');
    std::string url = (paramPos != std::string::npos) ? downloadUrl.substr(0, paramPos) : downloadUrl;

    // 2. 提取文件名
    auto lastSlash = url.find_last_of('/');
    std::string filename = (lastSlash == std::string::npos) ? url : url.substr(lastSlash + 1);

    // 3. 支持复合扩展名（如.tar.gz）
    const std::array<std::string, 3> compoundExt = {".tar.gz", ".tar.bz2", ".tar.xz"};
    for (const auto &ext : compoundExt) {
        if (filename.size() >= ext.size() && filename.compare(filename.size() - ext.size(), ext.size(), ext) == 0) {
            // 去掉开头的点
            std::string res(ext.substr(1));
            SanitizeFilename(res);
            OTA_LOG_D("Extract compound extension: %s", res.c_str());
            return res;
        }
    }

    // 4. 常规扩展名提取
    size_t lastDot = filename.find_last_of('.');
    if (lastDot != std::string::npos && lastDot != 0) {
        std::string res(filename.substr(lastDot + 1));
        SanitizeFilename(res);
        OTA_LOG_D("Extract extension: %s", res.c_str());
        return res;
    }
    OTA_LOG_E("Extract extension failed, filename: %s", filename.c_str());
    return "";
}

/**
 * @brief 头部回调函数
 * 
 * @param[in] buffer 数据缓冲区
 * @param[in] size 数据块大小
 * @param[in] nitems 数据块数量
 * @param[in] userdata 用户数据
 * 
 * @return 数据大小
 */
static size_t HeaderInfoCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    std::string *headers = static_cast<std::string *>(userdata);
    if (headers == nullptr) {
        OTA_LOG_E("Header callback failed, headers is nullptr");
        return 0;
    }
    if (std::string(buffer).find("Content-Disposition") == std::string::npos) {
        return nitems * size;
    }
    headers->append(buffer, nitems * size);
    return nitems * size;
}

/**
 * @brief 获取下载文件扩展名
 * 
 * @param[in] url 下载URL
 * 
 * @return 下载文件扩展名
 */
static std::string GetDownloadFileExtension(const std::string &url)
{
    std::string extension = ExtractExtensionFromUrl(url);
    if (!extension.empty()) {
        return extension;
    }
    OTA_LOG_E("Extract download file extension from url failed");

    std::string headers = GetHeaderInfoViaCurl(url, HeaderInfoCallback);
    if (headers.empty()) {
        // 若获取头部信息失败，重试一次
        OTA_LOG_E("Get download file extension header info failed, retry once");
        sleep(1);
        headers = GetHeaderInfoViaCurl(url, HeaderInfoCallback);
        if (headers.empty()) {
            OTA_LOG_E("Get download file extension header info failed again");
            return "";
        }
    }
    extension = ExtractExtensionFromUrl(headers);
    if (!extension.empty()) {
        return extension;
    }
    OTA_LOG_E("Get download file extension from header: %s failed", headers.c_str());
    return "";
}

/**
 * @brief 获取存储路径剩余空间（单位：字节）
 *
 * @param[in] path 存储路径
 *
 * @return 存储路径剩余空间
 */
static uint64_t GetFreeSpace(const std::string &path)
{
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) == 0) {
        return stat.f_bsize * stat.f_bavail;
    }
    return 0;
}

/**
 * @brief 获取文件下载路径
 *
 * @param[in] ecuConfig  ECU配置信息
 *
 * @return 文件下载路径
 */
static std::string GetUpgradeFileDownloadPath(const OtaTaskEcuConfigInfo &ecuConfig)
{
    // 若downloadPath不为空，直接返回
    if (!ecuConfig.upgradeFile.downloadPath.empty()) {
        return ecuConfig.upgradeFile.downloadPath;
    }

    if (ecuConfig.upgradeFile.downloadUrl.empty()) {
        OTA_LOG_E("Get file download path failed, download url is empty");
        return "";
    }

    // 1. 获取下载文件扩展名
    auto extension = GetDownloadFileExtension(ecuConfig.upgradeFile.downloadUrl);
    if (extension.empty()) {
        OTA_LOG_E("Extract download file extension form %s failed", ecuConfig.upgradeFile.downloadUrl.c_str());
        return "";
    }

    // 2. 生成下载文件路径，这里要确保folderPath最后是以'/'结尾
    std::string downloadFilePath = ecuConfig.dlFolder;
    if (downloadFilePath.back() != '/') {
        downloadFilePath += "/";
    }

    std::string ecuIdHexStr = Uint16ToHexStr(ecuConfig.ecuId);
    return downloadFilePath + ecuConfig.ecuName + "_" + ecuIdHexStr + "." + extension;
}

/**
 * @brief 获取备份文件路径
 * 
 * 通过在升级文件名末尾追加"_backup"字符串生成备份文件名
 * 
 * @param[in] ecuConfig  ECU配置信息
 * 
 * @return 备份文件路径
 */
static std::string GetBackupFilePath(const OtaTaskEcuConfigInfo &ecuConfig)
{
    std::string upgradeFilePath = ecuConfig.upgradeFile.downloadPath;
    if (upgradeFilePath.empty()) {
        OTA_LOG_E("Get backup file path failed, upgrade file path is empty");
        return "";
    }

    auto extPos = upgradeFilePath.find('.');
    if (extPos == std::string::npos) {
        OTA_LOG_E("Get backup file path failed, upgrade file[%s] path has no extension", upgradeFilePath.c_str());
        return "";
    }

    return upgradeFilePath.substr(0, extPos) + "_backup" + upgradeFilePath.substr(extPos);
}

/**
 * @brief 读备份文件MD5
 * 
 * @param[in] folderPath 下载文件存储目录
 * 
 * @return 备份文件MD5
 */
static std::string ReadBackupFileMd5(const std::string &folderPath)
{
    // 生成备份文件MD5文件路径
    std::string md5File =
        (folderPath.back() == '/') ? folderPath + BACKUP_MD5_FILE : folderPath + "/" + BACKUP_MD5_FILE;
    std::ifstream is(md5File);
    if (!is.is_open()) {
        OTA_LOG_E("Open backup file MD5 file:%s failed", md5File.c_str());
        return "";
    }
    // 按照MD5_BEGIN和MD5_END格式读取MD5
    std::string md5;
    std::string line;
    while (std::getline(is, line)) {
        auto pos = line.find(MD5_BEGIN);
        if (pos != std::string::npos) {
            pos += strlen(MD5_BEGIN);
            md5 = line.substr(pos, md5.find(SYMBOL_CRLF, pos) - pos);
            break;
        }
    }
    is.close();
    return md5;
}

/**
 * @brief 写备份文件MD5
 * 
 * @param[in] folderPath 下载文件存储目录
 * @param[in] md5 MD5值
 * 
 * @return 无
 */
static void WriteBackupFileMD5(const std::string &folderPath, const std::string &md5)
{
    // 生成备份文件MD5文件路径
    std::string md5FilePath =
        (folderPath.back() == '/') ? folderPath + BACKUP_MD5_FILE : folderPath + "/" + BACKUP_MD5_FILE;
    // 默认模式为 std::ios::out | std::ios::trunc
    std::ofstream os(md5FilePath);
    if (!os.is_open()) {
        OTA_LOG_E("Open backup file MD5 file:%s failed", md5FilePath.c_str());
        return;
    }
    std::string md5Content = MD5_BEGIN + md5 + SYMBOL_CRLF;
    // 写入MD5
    os << md5Content;
    os.close();
    OTA_LOG_D("Write backup file MD5: %s to file:%s success", md5Content.c_str(), md5FilePath.c_str());
}

/**
 * @brief 检测剩余存储空间是否足够
 * 
 * @param[in] folderPath 下载文件存储目录
 * @param[in] downloadSize 文件大小
 * 
 * @return 
 *      true：剩余存储空间足够
 *      false：剩余存储空间不足
 */
static bool IfRemainingStorageSpaceIsSufficient(const std::string &folderPath, uint32_t downloadSize)
{
    OTA_LOG_D("Check remaining storage space is sufficient");
    // 检测存储空间
    auto freeSpace = GetFreeSpace(folderPath);
    OTA_LOG_D("Free storage space: %lu bytes", freeSpace);
    if (freeSpace < downloadSize) {
        OTA_LOG_E("Download file failed, free space: %lu, download size: %u", freeSpace, downloadSize);
        return false;
    }
    return true;
}

/**
 * @brief 写下载数据回调函数
 *
 * @param[in] contents 下载数据
 * @param[in] size 数据块大小
 * @param[in] nmemb 数据块数量
 * @param[in] userdata 用户数据
 *
 * @return 写入数据大小
 */
static size_t DownloadWriteCallback(void *contents, size_t size, size_t nmemb, void *userdata)
{
    FILE *file = static_cast<FILE *>(userdata);
    if (file == nullptr) {
        OTA_LOG_E("Write download data failed, file is nullptr\n");
        return 0;
    }

    if (contents == nullptr) {
        OTA_LOG_E("Write download data failed, contents is nullptr\n");
        return 0;
    }
    size_t written = fwrite(contents, size, nmemb, file);
    // 确保数据落盘
    fflush(file);
    // 写文件的数据长度与下载的文件数据长度不一致，则返回0触发CURL_WRITE_ERROR
    return (written == nmemb) ? size * nmemb : 0;
}

/**
 * @brief 下载进度回调函数
 * 
 * @param[in] clientp 用户数据
 * @param[in] dltotal 下载总大小
 * @param[in] dlnow 当前下载大小
 * @param[in] ultotal 上传总大小
 * @param[in] ulnow 当前上传大小
 * 
 * @return 0
 */
static int DownloadProgressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal,
                                    curl_off_t ulnow)
{
    if (!g_isFileDownloading.load()) {
        OTA_LOG_E("Download progress aborted, file download is stopped\n");
        return 1;
    }

    CurlProgressData *data = static_cast<CurlProgressData *>(clientp);
    if (data == nullptr) {
        return 0;
    }

    // 下载总大小为0时，使用预设的总大小
    curl_off_t total = (dltotal <= 0) ? data->totalSize : (dltotal + data->lastProgress);
    if (total <= 0) {
        return 0;
    }

    static curl_off_t currentProgress = 0;
    curl_off_t download = dlnow + data->lastProgress;
    curl_off_t progress = (download * 100) / total;
    if (currentProgress != progress) {
        currentProgress = progress;
        OTA_LOG_D("Download total size: %d, downloaded size: %.d, progress-> %d%%", static_cast<int>(total),
                  static_cast<int>(download), static_cast<int>(progress));
    }
    return 0;
}

/**
 * @brief 下载文件
 *
 * @param[in] folderPath 下载文件存储目录
 * @param[in] fileInfo ECU升级文件信息
 * @param[in] downloadingCb 下载中回调
 *
 * @return 下载错误码
 */
static OtaFileDownloadErrorCode DownloadFile(const std::string &folderPath, OtaTaskEcuUpgradeFileInfo &fileInfo,
                                             DownloadingCallback downloadingCb)
{
    // 获取文件已下载大小
    auto downloadedSize = GetFileSize(fileInfo.downloadPath);
    OTA_LOG_D("File:%s downloaded size: %lu", fileInfo.downloadPath.c_str(), downloadedSize);
    // 获取文件总大小
    auto fileTotalSize = fileInfo.downloadSize;
    if (fileTotalSize == 0) {
        fileTotalSize = GetDownloadTotalSizeViaCurl(fileInfo.downloadUrl);
        if (fileTotalSize == 0) {
            // 若获取文件大小失败，重试一次
            OTA_LOG_W("Get download file total size failed, retry once");
            sleep(1);
            fileTotalSize = GetDownloadTotalSizeViaCurl(fileInfo.downloadUrl);
        }
    }
    if (fileTotalSize == 0) {
        OTA_LOG_E("Get download file total size failed");
        g_isFileDownloading.store(false);
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_SPACE_NOT_ENOUGH;
    }
    fileInfo.downloadSize = fileTotalSize;
    // 文件已下载完成
    if (downloadedSize >= fileTotalSize) {
        OTA_LOG_D("Download file is already downloaded");
        g_isFileDownloading.store(false);
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK;
    }

    // 检测存储空间
    if (IfRemainingStorageSpaceIsSufficient(folderPath, (fileTotalSize - downloadedSize)) == false) {
        g_isFileDownloading.store(false);
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_SPACE_NOT_ENOUGH;
    }

    // 打开文件
    FileHandle fileHandle(fopen(fileInfo.downloadPath.c_str(), "ab+"));
    if (fileHandle == nullptr) {
        OTA_LOG_E("Open file %s failed", fileInfo.downloadPath.c_str());
        g_isFileDownloading.store(false);
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_FILE_PATH_ERROR;
    }
    // 回调通知下载中
    if (downloadingCb) {
        downloadingCb(fileInfo);
    }

    CurlFileInfo curlFileInfo = {fileHandle.get(), fileInfo.downloadUrl};
    CurlProgressData data = {static_cast<curl_off_t>(downloadedSize), static_cast<curl_off_t>(fileTotalSize)};
    // 下载文件
    auto error = DownloadFileViaCurl(curlFileInfo, data, DownloadWriteCallback, DownloadProgressCallback);
    if (error != OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK) {
        OTA_LOG_E("Download file:%s failed, error code: %d", fileInfo.downloadPath.c_str(), error);
        g_isFileDownloading.store(false);
        return error;
    }
    g_isFileDownloading.store(false);
    OTA_LOG_D("Download file:%s success", fileInfo.downloadPath.c_str());
    return OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK;
}

/**
 * @brief 检测备份文件是否已存在，若已存在则检查MD5码是否一致
 * 
 * @param[in] folderPath 下载文件存储目录
 * @param[in] backupFilePath 备份文件路径
 * @param[inout] config ECU配置信息
 * 
 * @return 
 *      true：备份文件已存在，且MD5码一致，无需下载新的备份包
 *      false：备份文件不存在，或备份文件存在但MD5码不一致，需要下载新的备份包
 */
bool CheckIfOtaBackupFileAlreadyExist(const std::string &folderPath, const std::string &backupFilePath,
                                      OtaTaskEcuConfigInfo &config)
{
    OTA_LOG_D("Check if OTA backup file:%s already exist", backupFilePath.c_str());
    // 备份文件不存在时，返回false，表示需要下载新的备份包
    if (IsFileExist(backupFilePath) == false) {
        OTA_LOG_E("ECU[0x%04x]:%s backup file is not exist, need to download new backup file", config.ecuId,
                      config.ecuName.c_str());
        return false;
    }
    // 备份文件存在时：
    // 1.获取本地备份文件MD5的值，若获取失败则删除备份文件，返回false，表示需要下载新的备份包
    auto md5 = ReadBackupFileMd5(folderPath);
    if (md5.empty()) {
        OTA_LOG_E("Read exist backup file MD5 failed");
        OTA_LOG_E("Delete exist backup file: %s", backupFilePath.c_str());
        DeleteFile(backupFilePath);
        OTA_LOG_E("Need to download new backup file due to MD5 read failure");
        return false;
    }
    // 2.获取备份文件md5的值成功时
    // 比较平台下发的MD5的值与本地备份包的MD5的值不同时，删除本地备份包，准备下载新的备份包
    if (md5 != config.backupFile.downloadMD5) {
        OTA_LOG_E("ECU[0x%04x]:%s backup file MD5 mismatch. Platform MD5: %s, Local MD5: %s", 
                config.ecuId, config.ecuName.c_str(), config.backupFile.downloadMD5.c_str(), md5.c_str());
        OTA_LOG_E("Delete exist backup file ");
        // 删除本地备份文件，准备下载新的备份包
        DeleteFile(backupFilePath);
        OTA_LOG_E("Need to download new backup file due to MD5 mismatch");
        return false;
    }

    // 3.获取备份文件md5的值成功且检查md5码一致，返回true，表示无需下载
    config.backupFile.downloadMD5 = md5;
    config.backupFile.downloadPath = backupFilePath;
    OTA_LOG_D("ECU[0x%04x]:%s backup file already exist, file path:%s , md5:%s", config.ecuId, config.ecuName.c_str(),
              backupFilePath.c_str(), md5.c_str());
    OTA_LOG_D("MD5 verification passed, no need to download new backup file");
    return true;
}

OtaFileDownloadErrorCode StartDownloadOtaUpgradeFile(DownloadingCallback downloadingCb, OtaTaskEcuConfigInfo &config)
{
    OTA_LOG_D("Start download OTA upgrade file ---->");
    g_isFileDownloading.store(true);
    if (CheckAndCreateDirectory(config.dlFolder) == false) {
        g_isFileDownloading.store(false);
        OTA_LOG_E("OTA download directory[%s] is not exist", config.dlFolder.c_str());
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_FOLDER_EMPTY;
    }

    if (config.upgradeFile.downloadUrl.empty()) {
        OTA_LOG_E("Ecu[0x%04x]-[%s] upgrade file URL is empty, unable to download", config.ecuId,
                  config.ecuName.c_str());
        g_isFileDownloading.store(false);
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_INVALID_ADDRESS;
    }

    if (config.upgradeFile.downloadState == OtaFileDownloadStatus::OTA_FILE_DOWNLOAD_SUCCESS) {
        OTA_LOG_E("Ecu[0x%04x]-[%s] upgrade file is downloaded", config.ecuId, config.ecuName.c_str());
        g_isFileDownloading.store(false);
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK;
    }

    // 生成下载路径
    auto downloadPath = GetUpgradeFileDownloadPath(config);
    if (downloadPath.empty()) {
        OTA_LOG_E("Generate download path failed");
        g_isFileDownloading.store(false);
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_FILE_PATH_ERROR;
    }
    config.upgradeFile.downloadPath = downloadPath;
    OTA_LOG_D("Download OTA upgrade file, URL: %s, Save path: %s", config.upgradeFile.downloadUrl.c_str(), downloadPath.c_str());
    return DownloadFile(config.dlFolder, config.upgradeFile, downloadingCb);
}

OtaFileDownloadErrorCode StartDownloadOtaBackupFile(DownloadingCallback downloadingCb, OtaTaskEcuConfigInfo &config)
{
    OTA_LOG_D("Start download OTA backup file ---->");
    g_isFileDownloading.store(true);
    if (CheckAndCreateDirectory(config.dlFolder) == false) {
        g_isFileDownloading.store(false);
        OTA_LOG_E("OTA download directory[%s] is not exist", config.dlFolder.c_str());
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_FOLDER_EMPTY;
    }

    if (config.backupFile.downloadState == OtaFileDownloadStatus::OTA_FILE_DOWNLOAD_SUCCESS) {
        OTA_LOG_E("OTA backup file is downloaded");
        g_isFileDownloading.store(false);
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK;
    }

    // 生成下载路径
    auto downloadPath = GetBackupFilePath(config);
    if (downloadPath.empty()) {
        OTA_LOG_E("Generate backup file download path failed");
        g_isFileDownloading.store(false);
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_FILE_PATH_ERROR;
    }

    // 平台下发的备份文件还未下载时，检测本地备份文件是否已存在
    if (config.backupFile.downloadState == OtaFileDownloadStatus::OTA_FILE_NOT_DOWNLOAD) {
        // 检测备份文件已存在并且MD5码一致时，返回true，表示无需下载
        if (CheckIfOtaBackupFileAlreadyExist(config.dlFolder, downloadPath, config)) {
            OTA_LOG_D("ECU[0x%04x]:%s backup file: %s already exist,and the md5 check passed,no need to download", config.ecuId,
                      config.ecuName.c_str(), downloadPath.c_str());
            g_isFileDownloading.store(false);
            config.isLocalBackup = 1;
            return OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK;
        }
    }
    
    // 检测备份文件下载地址是否为空, 认为不需要下载备份包
    if (config.backupFile.downloadUrl.empty()) {
        OTA_LOG_D("Ecu[0x%04x]-[%s] backup file URL is empty, uno need to download", config.ecuId,
                  config.ecuName.c_str());
        g_isFileDownloading.store(false);
        config.backupFile.downloadPath.clear();
        return OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK;
    }

    config.backupFile.downloadPath = downloadPath;
    OTA_LOG_D("Download OTA backup file, URL: %s, Save path: %s", config.backupFile.downloadUrl.c_str(),
              downloadPath.c_str());
    return DownloadFile(config.dlFolder, config.backupFile, downloadingCb);
}

void StopOtaFileDownload(void)
{
    OTA_LOG_D("Stop OTA file download");
    g_isFileDownloading.store(false);
}

void BackupDownloadedOtaFile(const OtaTaskConfigInfo &taskConfig)
{
    OTA_LOG_D("Backup downloaded OTA file");
    auto BackupFile = [&](const OtaTaskEcuConfigInfo &config) {
        // 删除解压目录
        OTA_LOG_D("Delete upgrade decompress folder: %s", config.upgradeFile.decompressFolder.c_str());
        DeleteFolder(config.upgradeFile.decompressFolder);
        OTA_LOG_D("Delete backup decompress folder: %s", config.backupFile.decompressFolder.c_str());
        DeleteFolder(config.backupFile.decompressFolder);
        // 检测下载文件是否存在
        if (!IsFileExist(config.upgradeFile.downloadPath)) {
            OTA_LOG_E("ECU[0x%04x]:%s upgrade file is not exist, unable to backup upgrade file", config.ecuId,
                      config.ecuName.c_str());
            return;
        }

        // 生成备份文件路径
        std::string backupPath = GetBackupFilePath(config);
        if (backupPath.empty()) {
            OTA_LOG_E("Get ECU[0x%04x]:%s backup download path failed, unable to backup upgrade file", config.ecuId,
                      config.ecuName.c_str());
                      return;
        }
        // 删除原备份文件
        std::remove(backupPath.c_str());

        // 备份升级文件
        if (std::rename(config.upgradeFile.downloadPath.c_str(), backupPath.c_str()) != 0) {
            OTA_LOG_E("Backup upgrade file %s to %s failed", config.upgradeFile.downloadPath.c_str(), backupPath.c_str());
            return;
        }
        OTA_LOG_D("Backup upgrade file %s to %s success", config.upgradeFile.downloadPath.c_str(), backupPath.c_str());
        // 写备份文件MD5
        WriteBackupFileMD5(config.dlFolder, config.upgradeFile.downloadMD5);
    };

    // 备份高压升级文件
    for (const auto &config : taskConfig.highVoltEcuTasksInfo) {
        BackupFile(config);
    }

    // 备份低压升级文件
    for (const auto &config : taskConfig.lowVoltEcuTasksInfo) {
        BackupFile(config);
    }
}

void DeleteDownloadedOtaFile(const OtaTaskConfigInfo &taskConfig, bool isDelLocalBackupFile)
{
    OTA_LOG_D("Delete downloaded OTA file, isDelLocalBackupFile: %s", isDelLocalBackupFile ? "true" : "false");
    auto ExecuteDelete = [&](const OtaTaskEcuConfigInfo &config) {
        // 删除升级文件
        OTA_LOG_D("Delete upgrade file: %s", config.upgradeFile.downloadPath.c_str());
        DeleteFile(config.upgradeFile.downloadPath);
        // 删除解压目录
        OTA_LOG_D("Delete upgrade decompress folder: %s", config.upgradeFile.decompressFolder.c_str());
        DeleteFolder(config.upgradeFile.decompressFolder);
        // 删除备份文件解压目录
        OTA_LOG_D("Delete backup decompress folder: %s", config.backupFile.decompressFolder.c_str());
        DeleteFolder(config.backupFile.decompressFolder);

        // 如果不是本地备份文件，需要删除下载的备份文件
        if (!isDelLocalBackupFile && config.isLocalBackup) {
            OTA_LOG_D("ECU[0x%04x]:%s backup file is local, no need to delete backup file", config.ecuId,
                      config.ecuName.c_str());
            return;
        }

        if (config.backupFile.downloadPath.empty()) {
            OTA_LOG_E("ECU[0x%04x]:%s backup file is not exist, unable to delete backup file", config.ecuId,
                      config.ecuName.c_str());
            return;
        }
        // 删除备份文件
        OTA_LOG_D("Delete backup file: %s", config.backupFile.downloadPath.c_str());
        DeleteFile(config.backupFile.downloadPath);
    };

    for (const auto &config : taskConfig.highVoltEcuTasksInfo) {
        ExecuteDelete(config);
    }

    for (const auto &config : taskConfig.lowVoltEcuTasksInfo) {
        ExecuteDelete(config);
    }
}