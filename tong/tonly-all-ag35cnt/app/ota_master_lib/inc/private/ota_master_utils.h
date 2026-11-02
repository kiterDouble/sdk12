/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-24
 * Description: OTA Master工具类
 *********************************************************/
#ifndef _OTA_MASTER_UTILS_H
#define _OTA_MASTER_UTILS_H

#include <stdint.h>
#include <string>

/**
 * @brief 将16进制字符串转换为uint32_t
 * 
 * @param[in] hexStr 16进制字符串
 * 
 * @return uint32_t
 */
uint32_t HexStringToUint32(const std::string& hexStr);

/**
 * @brief 将16进制字符串转换为字节数组
 * 
 * @param[in] hexStr 16进制字符串
 * @param[out] bytes 字节数组
 * @param[in] bytesSize 字节数组大小
 * 
 * @return uint16_t 字节数组长度
 */
uint16_t HexStrToBytes(const std::string &hexStr, uint8_t *bytes, uint16_t bytesSize);

/**
 * @brief 将uint16_t类型数据转换为16进制字符串
 * 
 * @param[in] value uint16_t类型数据
 * 
 * @return std::string 16进制字符串
 */
std::string Uint16ToHexStr(uint16_t value);

/**
 * @brief 检文件夹是否存在，若不存在则创建
 *
 * @param[in] folder 文件夹路径
 *
 * @return
 *     true：文件夹存在或创建成功
 *    false：文件夹不存在且创建失败
 */
bool CheckAndCreateDirectory(const std::string &folder);

/**
 * @brief 生成文件夹路径
 * 
 * @param[in] rootFolder 根文件夹
 * @param[in] subFolder 子文件夹
 * 
 * @return 文件夹路径
 */
std::string GenerateFolderPath(const std::string &rootFolder, const std::string &subFolder);

/**
 * @brief 获取文件大小
 * 
 * @param[in] filePath 文件路径
 * 
 * @return 文件大小
 */
size_t GetFileSize(const std::string &filePath);

/**
 * @brief 检测文件是否存在
 *
 * @param[in] filePath 文件路径
 *
 * @return 文件是否存在
 */
bool IsFileExist(const std::string &filePath);

/**
 * @brief 删除文件
 * 
 * @param[in] filePath 文件路径
 * 
 * @return 无
 */
void DeleteFile(const std::string &filePath);

/**
 * @brief 删除文件夹
 * 
 * @param[in] folderPath 文件夹路径
 * 
 * @return 无
 */
void DeleteFolder(const std::string &folderPath);

/**
 * @brief 移除字符串中的'\r'、'\n'、'\0'
 * 
 * @param[inout] str 字符串
 * 
 * @return 无
 */
void SanitizeFilename(std::string &str);

/**
 * @brief 将字符串转换为大写
 * 
 * @param[inout] str 字符串
 * 
 * @return 无
 */
void StringToUpper(std::string &str);
#endif // _OTA_MASTER_UTILS_H