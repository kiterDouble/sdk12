/**
 * @file param_backup.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief 参数文件备份
 * @version 0.1
 * @date 2023-12-27
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __PARAM_BACKUP_H__
#define __PARAM_BACKUP_H__
#ifdef __cplusplus
extern "C"
{
#endif
#include <glib.h>
#include "param_msg.h"
#include "stdint.h"
#include "stdbool.h"

/**
 * @brief 检查文件是否存在
 * 
 * @param filePath 文件绝对路径
 * @return true 文件存在
 * @return false 文件不存在
 */
bool param_fileIsExist(const char* filePath);

/**
 * @brief 检查数据库核心参数是否合法
 *
 * @param dbfile 文件绝对路径
 * @return true 合法
 * @return false 不合法
 */
bool param_loadAndCheckKeyParamValid(const char *dbfile);

/**
 * @brief 检查并创建空目录
 * 
 * @param dirName 目录的绝对路径
 */
void checkAndMkdir(const char *dirName);

/**
 * @brief 定期检查备份文件的线程函数
 * 
 * @param pArg 线程参数
 * @return void* 线程返回值
 */
void* paramBackupCallBack(void* pArg);
#ifdef __cplusplus
}
#endif

#endif
