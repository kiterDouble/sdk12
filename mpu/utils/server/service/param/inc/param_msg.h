/**
 * @file param_msg.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief 参数枚举定义
 * @version 0.1
 * @date 2023-12-27
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __PARAM_MSG_H__
#define __PARAM_MSG_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 参数类型为整形
 * 
 */
#define PARAM_TYPE_INT (1)

/**
 * @brief 参数类型为整形字符串型
 * 
 */
#define PARAM_TYPE_STR (0)

/**
 * @brief oemapp中参数数据库文件保存目录
 * 
 */
#define PARAM_DICT_OEMAPP_PATH "/oemapp/parameters"

/**
 * @brief oemapp中参数数据库文件绝对路径
 * 
 */
#define PARAM_FILE_OEMAPP_PATH "/oemapp/parameters/paramsDb"

/**
 * @brief 参数数据库文件保存目录
 * 
 */
#define PARAM_DICT_PATH "/oemdata/parameters"

/**
 * @brief 参数数据库文件绝对路径
 * 
 */
#define PARAM_FILE_PATH "/oemdata/parameters/paramsDb"

/**
 * @brief 参数备份数据库文件保存目录
 * 
 */
#define PARAM_DICT_BACKUP_PATH "/mnt/emmc/parameters"

/**
 * @brief 参数备份数据库文件绝对路径
 * 
 */
#define PARAM_FILE_BACKUP_PATH "/mnt/emmc/parameters/paramsDb"

/**
 * @brief 参数文件检查间隔（秒）
 * 
 */
#define BACKUP_CHECK_SECOND (30)

#ifdef __cplusplus
}
#endif

#endif
