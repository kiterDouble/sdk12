/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-03-11
 * Description: 文件解压缩处理，支持.zip、.tar、.tar.gz、.tar.xz格式解压，支持.zip格式压缩
 *********************************************************/
#ifndef _OTA_MASTER_ARCHIVE_MANAGE_H
#define _OTA_MASTER_ARCHIVE_MANAGE_H

#include <string>
#include <vector>

/**
 * @brief 解压缩文件
 * 
 * @param[in] inputFile 待解压缩文件
 * @param[in] outputDir 解压缩输出目录
 * 
 * @return
 *      OTA_RET_OK：解压缩成功
 *      其他：解压缩失败
 */
int DecompressFile(const std::string &inputFile, const std::string &outputDir);

/**
 * @brief 压缩文件为.zip格式
 * 
 * @param[in] inputFile 待压缩文件
 * @param[in] outputDir 压缩输出目录
 * 
 * @return 压缩文件路径
 */
std::string CompressFileToZip(const std::string &inputFile, const std::string &outputDir);

/**
 * @brief 查找ECU驱动和升级文件
 * 
 * @param[out] driverFile 驱动文件
 * @param[out] upgradeFile 升级文件
 * 
 * @return
 *      OTA_RET_OK：查找成功
 *      OTA_RET_ERR：查找失败
 */
int FindEcuDriverAndUpgradeFile(const std::string &dir, std::string &driverFile, std::vector<std::string> &upgradeFile);
#endif // _OTA_MASTER_ARCHIVE_MANAGE_H