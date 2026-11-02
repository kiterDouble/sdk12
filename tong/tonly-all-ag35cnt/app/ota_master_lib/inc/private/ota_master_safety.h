/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-07
 * Description: OTA Master状态机管理
 *********************************************************/
#ifndef _OTA_MASTER_SAFETY_H
#define _OTA_MASTER_SAFETY_H

#include <stdint.h>
#include <string>

/**
 * AES128 ECB加密
 *
 * @param[in] key     加密KEY
 * @param[in] input   加密源数据
 * @param[out] output  加密后输出数据
 *
 * @return 加密后数据长度
 */
int AES128ECBEncrypt(const std::string &key, const std::string &input, uint8_t **output);

/**
 * AES128 ECB解密
 *
 * @param[in] key     解密KEY
 * @param[in] input   解密源数据
 * @param[out] output  解密后输出数据
 *
 * @return 解密后数据长度
 */
int AES128ECBDecrypt(const std::string &key, const std::string &input, uint8_t **output);

/**
 * 计算文件的MD5
 * 
 * @param[in] filePath 文件绝对路径 
 *
 * @return  文件的MD5码
 */
std::string CalculateMD5(const std::string &filePath);

/**
 * MD5校验
 *
 * @param[in] filePath   待校验的文件路径
 * @param[in] targetMD5  目标MD5
 *
 * @return
 *      OTA_RET_OK：校验成功
 *      OTA_RET_ERR：校验失败
 */
int CheckMD5(const std::string &filePath, const std::string &targetMD5);

#endif // _OTA_MASTER_SAFETY_H
