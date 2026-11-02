#include <sys/stat.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "ota_master_utils.h"
#include "ota_master_error_code.h"
#include "ota_master_log.h"

uint32_t HexStringToUint32(const std::string &hexStr)
{
    if (hexStr.empty()) {
        OTA_LOG_E("hexStr is empty.\n");
        return 0;
    }
    unsigned long value = 0;
    value = std::stoul(hexStr, nullptr, 16); // 按16进制解析
    if (value > 0xFFFFFFFF) {
        OTA_LOG_E("hexStr[%s] exceeds uint32_t range", hexStr.c_str());
        return 0;
    }
    return static_cast<uint32_t>(value);
}

uint16_t HexStrToBytes(const std::string &hexStr, uint8_t *bytes, uint16_t bytesSize)
{
    if (bytes == nullptr) {
        OTA_LOG_E("bytes is nullptr.\n");
        return 0;
    }

    if ((hexStr.length() / 2) > bytesSize) {
        OTA_LOG_E("Bytes size[%d] is not large enough to store the binary data[%d]\n", bytesSize, hexStr.length() / 2);
        return 0;
    }

    for (size_t i = 0; i < hexStr.length(); i += 2) {
        std::sscanf(hexStr.substr(i, 2).c_str(), "%02hhx", &bytes[i / 2]);
    }
    return (hexStr.length() / 2);
}

int BytesToHexStr(const uint8_t *bytes, uint32_t bytesLen, std::string &outStr)
{
    if (bytes == nullptr) {
        OTA_LOG_E("bytes is nullptr, bytes to hex string failed.\n");
        return OTA_RET_ERR;
    }

    std::stringstream stream;
    for (size_t i = 0; i < bytesLen; i++) {
        stream << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(bytes[i]);
    }
    outStr = stream.str();
    return OTA_RET_ERR;
}

std::string Uint16ToHexStr(uint16_t value)
{
    char result[5] = {0};
    snprintf(result, sizeof(result), "%04X", value);
    return result;
}

bool CheckAndCreateDirectory(const std::string &folder)
{
    if (folder.empty()) {
        OTA_LOG_E("folder is empty.\n");
        return false;
    }

    struct stat st;
    if (stat(folder.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    OTA_LOG_E("%s is not exist, create it", folder.c_str());
    if (mkdir(folder.c_str(), 0755) == 0) {
        return true;
    }
    OTA_LOG_E("Create %s failed, error: %s", folder.c_str(), strerror(errno));
    return false;
}

std::string GenerateFolderPath(const std::string &rootFolder, const std::string &subFolder)
{
    if (rootFolder.empty() || subFolder.empty()) {
        OTA_LOG_E("rootFolder or subFolder is empty.\n");
        return "";
    }

    return (rootFolder.back() != '/') ? rootFolder + "/" + subFolder : rootFolder + subFolder;
}

size_t GetFileSize(const std::string &filePath)
{
    if (filePath.empty()) {
        OTA_LOG_E("Get file size failed, filePath is empty.\n");
        return 0;
    }

    struct stat info;
    if (stat(filePath.c_str(), &info) == 0 && S_ISREG(info.st_mode)) {
        return info.st_size;
    }
    return 0;
}

bool IsFileExist(const std::string &filePath)
{
    return (access(filePath.c_str(), F_OK) == 0);
}

void DeleteFile(const std::string &filePath)
{
    if (filePath.empty()) {
        OTA_LOG_E("Delete failed, filePath is empty.\n");
        return;
    }

    if (!IsFileExist(filePath)) {
        OTA_LOG_E("File[%s] is not exist, no need to delete\n", filePath.c_str());
        return;
    }

    if (remove(filePath.c_str()) != 0) {
        OTA_LOG_E("Delete file %s failed, error: %s", filePath.c_str(), strerror(errno));
        return;
    }
    OTA_LOG_D("Delete file %s success", filePath.c_str());
}

void DeleteFolder(const std::string &folderPath)
{
    if (folderPath.empty()) {
        OTA_LOG_E("Delete failed, folderPath is empty.\n");
        return;
    }

    if (!IsFileExist(folderPath)) {
        OTA_LOG_E("Folder[%s] is not exist, no need to delete\n", folderPath.c_str());
        return;
    }

    std::string cmd = "rm -rf " + folderPath;
    int status = system(cmd.c_str());
    if (WIFEXITED(status) && (WEXITSTATUS(status) == 0)) {
        OTA_LOG_D("Delete folder %s success\n", folderPath.c_str());
        return;
    }
    OTA_LOG_E("Delete folder %s failed\n", folderPath.c_str());
}

void SanitizeFilename(std::string &str)
{
    auto isInvalid = [](char c) { return c == '\r' || c == '\n' || c == '\0'; };
    str.erase(std::remove_if(str.begin(), str.end(), isInvalid), str.end());
}

void StringToUpper(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::toupper(c); });
}