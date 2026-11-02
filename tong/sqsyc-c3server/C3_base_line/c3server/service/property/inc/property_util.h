#ifndef __PROPERTY_UTIL_H__
#define __PROPERTY_UTIL_H__

/*
* @brief 检查并创建输入文件/文件夹 所在路径是否存在，不存在则创建
* @param filePath: 文件/文件夹路径
*/
int CreateFileDirectory(const char* filePath);

#endif