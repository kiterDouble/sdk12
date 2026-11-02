/**
 * @file param_util.h
 * @brief 参数工具函数头文件
 *
 * 提供读取系统版本信息等辅助函数。
 */

#ifndef __PARAM_UTIL_H__
#define __PARAM_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MPU_VERSION_FILE_PATH "/oemapp/version/MPU_VERSION"
#define MPU_PROJECT_VER_LEN       3

#define MPU_SQSYC_PROJECT_NUM 234
#define MPU_TONLY_PROJECT_NUM 235
#define MPU_VE21_PROJECT_NUM 256

/**
 * @brief 读取 MPU 版本文件并获取前三个字节
 *
 * 读取 /oemapp/version/MPU_VERSION 文件内容，将前三个字节转换为 int32_t 返回。
 * 当文件不存在、读取失败或内容不足三字节时，默认返回 0。
 *
 * @return int32_t 版本号，失败时返回 0
 */
int32_t ParamUtilGetMpuProjectNo(void);

#ifdef __cplusplus
}
#endif

#endif
