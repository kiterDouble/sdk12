#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>
#include <string>
#include <array>
#include "ota_master_archive_manage.h"
#include "ota_master_utils.h"
#include "ota_master_log.h"

namespace {
static const constexpr char *DRIVER_FILE_IDENT = "driver";  // 驱动文件标识
} // namespace

/**
 * @brief 是否是压缩文件
 * 
 * @param[in] path 文件路径
 * 
 * @return
 *      true：是压缩文件
 *      false：不是压缩文件
 */
static bool IsCompressedFile(const std::string &path)
{
    const std::array<std::string, 5> exts = {".zip", ".tar", ".tar.gz", ".tar.xz", ".bin"};
    return std::any_of(exts.begin(), exts.end(), [&](const std::string &ext) {
        return path.size() >= ext.size() && std::equal(ext.rbegin(), ext.rend(), path.rbegin(), [](char a, char b) {
                   return std::tolower(a) == std::tolower(b);
               });
    });
}

/**
 * @brief 是否是目标文件
 * 
 * @param[in] fileName 文件名
 * 
 * @return
 *      true：是目标文件
 *      false：不是目标文件
 */
static bool IsTargetFile(const std::string &fileName)
{
    std::string lowerFileName = fileName;
    std::transform(lowerFileName.begin(), lowerFileName.end(), lowerFileName.begin(), ::tolower);
    const auto pos = lowerFileName.rfind('.');
    if (pos == std::string::npos) {
        OTA_LOG_E("File: %s has no extension", fileName.c_str());
        return false;
    }
    // 只识别hex和s19文件，TBox只能是压缩包
    const std::array<std::string, 2> TARGET_EXTS = {".hex", ".s19"};
    const auto ext = lowerFileName.substr(pos);
    return std::any_of(TARGET_EXTS.begin(), TARGET_EXTS.end(), [&](const std::string &targetExt) {
        return ext == targetExt;
    });
}

/**
 * @brief 获取解压缩命令
 * 
 * @param[in] inputFile 待解压缩文件
 * @param[in] outputDir 解压缩输出目录
 * 
 * @return 解压缩命令
 */
static std::string GetDecompressCmd(const std::string &inputFile, const std::string &outputDir)
{
    if (inputFile.find(".zip") != std::string::npos) {
        return "unzip -o \"" + inputFile + "\" -d \"" + outputDir + "\"";
    }

    // .bin文件用unzip解压
    if (inputFile.find(".bin") != std::string::npos || inputFile.find(".BIN") != std::string::npos) {
        return "unzip -o \"" + inputFile + "\" -d \"" + outputDir + "\"";
    }

    if (inputFile.find(".tar") != std::string::npos) {
        if (inputFile.find(".gz") != std::string::npos) {
            return "tar -xzf \"" + inputFile + "\" -C \"" + outputDir + "\"";
        } else if (inputFile.find(".xz") != std::string::npos) {
            return "tar -xJf \"" + inputFile + "\" -C \"" + outputDir + "\"";
        } else {
            // 普通tar包
            return "tar -xf \"" + inputFile + "\" -C \"" + outputDir + "\"";
        }
    }

    return "";
}

int DecompressFile(const std::string &inputFile, const std::string &outputDir)
{
    if (inputFile.empty()) {
        OTA_LOG_E("Decompress file failed, input file is empty");
        return OTA_RET_ERR;
    }

    // 检查输出路径是否存在，不存在则创建
    if (CheckAndCreateDirectory(outputDir) == false) {
        OTA_LOG_E("Create directory failed, path: %s", outputDir.c_str());
        return OTA_RET_ERR;
    }

    // 解压文件
    if (IsCompressedFile(inputFile)) {
        std::string cmd = std::move(GetDecompressCmd(inputFile, outputDir));
        if (cmd.empty()) {
            OTA_LOG_E("Decompress file failed, unsupported file format: %s", inputFile.c_str());
            return OTA_RET_ERR;
        }

        OTA_LOG_D("Decompress file cmd: %s", cmd.c_str());
        int status = system(cmd.c_str());
        OTA_LOG_D("Decompress file status: %d, exit:%d, exit status:%d", status, WIFEXITED(status), WEXITSTATUS(status));
        return (WIFEXITED(status) && (WEXITSTATUS(status) == 0)) ? OTA_RET_OK : OTA_RET_ERR;
    }

    OTA_LOG_D("File: %s is not a compressed file", inputFile.c_str());
    // 若不是压缩包，且符合目标格式文件，直接拷贝文件到目标目录
    if (IsTargetFile(inputFile)) {
        std::string cmd = "cp -f \"" + inputFile + "\" \"" + outputDir + "\"";
        int status = system(cmd.c_str());
        return (WIFEXITED(status) && (WEXITSTATUS(status) == 0)) ? OTA_RET_OK : OTA_RET_ERR;
    }
    OTA_LOG_E("File: %s is not a target file", inputFile.c_str());
    return OTA_RET_ERR;
}

std::string CompressFileToZip(const std::string &inputFile, const std::string &outputDir)
{
    if (!IsFileExist(inputFile)) {
        OTA_LOG_E("Compress file failed, file:%s not exist", inputFile.c_str());
        return "";
    }
    // 检查输出路径是否存在，不存在则创建
    if (CheckAndCreateDirectory(outputDir) == false) {
        OTA_LOG_E("Create directory failed, path: %s", outputDir.c_str());
        return "";
    }

    auto pos = inputFile.find_last_of("/");
    std::string filename = inputFile.substr(pos + 1);
    pos = filename.find_last_of(".");
    filename = filename.substr(0, pos);
    std::string outputFile = (outputDir.back() == '/') ? outputDir + filename + ".zip" : outputDir + "/" + filename + ".zip";
    std::string cmd = "zip -j \"" + outputFile + "\" \"" + inputFile + "\"";
    int status = system(cmd.c_str());
    OTA_LOG_D("Compress file status: %d, exit:%d, exit status:%d", status, WIFEXITED(status), WEXITSTATUS(status));
    if (WIFEXITED(status) && (WEXITSTATUS(status) == 0)) {
        OTA_LOG_D("Compress input file:%s to zip success\n", inputFile.c_str());
        return (outputDir.back() == '/') ? outputDir + filename + ".zip" : outputDir + "/" + filename + ".zip";
    }
    OTA_LOG_E("Compress input file:%s to zip failed\n", inputFile.c_str());
    return "";
}

int FindEcuDriverAndUpgradeFile(const std::string &dir, std::string &driverFile, std::vector<std::string> &upgradeFile)
{
    OTA_LOG_D("Find ECU driver and upgrade file from dir: %s", dir.c_str());
    DIR *dp = opendir(dir.c_str());
    if (dp == nullptr) {
        OTA_LOG_E("Open directory failed, path: %s", dir.c_str());
        return OTA_RET_ERR;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename == "." || filename == "..") {
            continue;
        }

        // 递归搜索子目录
        if (entry->d_type == DT_DIR) {
            std::string subDir = (dir.back() == '/') ? (dir + filename) : (dir + "/" + filename);
            // 递归搜索子目录，若找到则直接返回
            if (FindEcuDriverAndUpgradeFile(subDir, driverFile, upgradeFile) == OTA_RET_OK) {
                break;
            }
            continue;
        }

        // 查找目标文件
        std::string filePath = (dir.back() == '/') ? (dir + filename) : (dir + "/" + filename);
        if (!IsTargetFile(filename)) {
            OTA_LOG_E("%s is not a target file", filename.c_str());
            continue;
        }

        std::string lowerFilePath = filePath;
        std::transform(lowerFilePath.begin(), lowerFilePath.end(), lowerFilePath.begin(), ::tolower);
        if (lowerFilePath.find(DRIVER_FILE_IDENT) != std::string::npos) {
            driverFile = filePath;
            OTA_LOG_D("Find driver file: %s", filePath.c_str());
        } else {
            upgradeFile.push_back(filePath);
            OTA_LOG_D("Find upgrade file: %s", filePath.c_str());
        }
    }
    closedir(dp);
    return (driverFile.empty() && upgradeFile.empty()) ? OTA_RET_ERR : OTA_RET_OK;
}