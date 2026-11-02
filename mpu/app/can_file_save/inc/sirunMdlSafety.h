/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: bcs
 * Created Time: 2025-11-04
 * Description: 进程调用栈信息打印以及coredump文件生成
 *********************************************************/

#ifndef _SIRUN_MDL_SAFETY_H_
#define _SIRUN_MDL_SAFETY_H_
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

/**
 * @brief 设置存放coredump文件路径信息(调用后将会在指定路径生成coredump文件)
 *        注意！！！最好只在调试时候使用，避免进程挂死后，再被拉起，重复生成过多coredump文件，导致emmc空间被占满影响业务逻辑。
 *
 * @param[in] ppath: 文件路径
 *
 * @return 0 – 成功，其他 – 失败
 * 
 */
extern int32_t SirunSetCoreDumpInner(const int8_t *ppath);

/**
 * @brief 初始化异常信号捕获机制
 *
 * @param 无
 *
 * @return 0 – 成功，其他 – 失败
 *
 */
extern int32_t SirunSignalCatchInit(void);


#ifdef __cplusplus
}
#endif

#endif
