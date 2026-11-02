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
 * @brief 拷贝文件
 * 
 * @param destPath 目标路径
 * @param sourcePath 原始路径
 * @return true 拷贝成功
 * @return false 拷贝失败
 */
bool param_fileCopy(const char *destPath, const char *sourcePath);

/**
 * @brief 检查并创建空目录
 * 
 * @param dirName 目录的绝对路径
 */
void checkAndMkdir(const char *dirName);

/**
 * @brief 标记有参数更新，通知备份线程立即备份
 *
 * 由业务线程在写 DB 成功后调用，仅设置脏标志+信号，不做文件 I/O。
 */
void ParamBackupMarkDirty(void);

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
