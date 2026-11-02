#ifndef __LOG_SERVER_UTIL_H
#define __LOG_SERVER_UTIL_H

#include <stdint.h>

uint32_t GetUtcTimestamp();

int GetUtcDateTimeStr(uint8_t* dateTimeStrBuffer,int32_t bufferSize);

/*
* 判断文件存在并且空闲
* return: 0:空闲； else：非空闲
*/
int IsFileExistsAndIdel(const char *filename);

uint64_t GetFileSize(const char *filename);

// 检查目录是否存在，不存在则创建
void CheckAndCreateDirectory(const char *dir_path);

#endif