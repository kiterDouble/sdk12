#ifndef ASC_FILE_PROCESSOR_H
#define ASC_FILE_PROCESSOR_H

#include <stdint.h>

// 处理CAN ASC文件上传（替换shell脚本功能）
// 参数:
//   endTime - 结束时间（Unix时间戳）
//   startTime - 开始时间（Unix时间戳）
//   canAscPath - CAN ASC文件所在目录
//   uploadTmpDir - 临时上传目录
// 返回值: 0成功，-1失败
int ProcessCanAscFiles(uint64_t startTime,uint64_t endTime,  const char *canAscPath, const char *uploadTmpDir);

#endif // ASC_FILE_PROCESSOR_H