/**
 * @file iniparser.h
 * @brief 简化版INI文件解析器头文件
 * @description 基于原iniparser的简化实现，移除字典依赖
 */

#ifndef _INIPARSER_H_
#define _INIPARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* 最大键值对数量 */
#define INI_MAX_ENTRIES     256
#define INI_KEY_LEN         128
#define INI_VALUE_LEN       512
#define INI_LINE_LEN        1024

/* 键值对结构 */
typedef struct {
    char key[INI_KEY_LEN];      /* 完整键名: section:key */
    char value[INI_VALUE_LEN];  /* 值 */
} IniEntry;

/* 字典结构 */
typedef struct {
    IniEntry entries[INI_MAX_ENTRIES];
    int count;
} dictionary;

/**
 * @brief 从文件加载INI字典
 * @param ininame INI文件路径
 * @return 字典指针，失败返回NULL
 */
dictionary* iniparser_load(const char *ininame);

/**
 * @brief 释放字典
 * @param d 字典指针
 */
void iniparser_freedict(dictionary *d);

/**
 * @brief 获取字符串值
 * @param d 字典
 * @param key 键名 (格式: "section:key")
 * @param def 默认值
 * @return 字符串值
 */
char* iniparser_getstring(dictionary *d, const char *key, char *def);

/**
 * @brief 获取整型值
 * @param d 字典
 * @param key 键名
 * @param notfound 未找到时的返回值
 * @return 整型值
 */
int iniparser_getint(dictionary *d, const char *key, int notfound);

/**
 * @brief 获取双精度浮点值
 * @param d 字典
 * @param key 键名
 * @param notfound 未找到时的返回值
 * @return 双精度值
 */
double iniparser_getdouble(dictionary *d, const char *key, double notfound);

/**
 * @brief 获取布尔值
 * @param d 字典
 * @param key 键名
 * @param notfound 未找到时的返回值
 * @return 布尔值 (1/0)
 */
int iniparser_getboolean(dictionary *d, const char *key, int notfound);

/**
 * @brief 设置字符串值
 * @param d 字典
 * @param key 键名
 * @param value 值
 * @return 0成功，-1失败
 */
int iniparser_setstring(dictionary *d, const char *key, const char *value);

/**
 * @brief 保存字典到文件
 * @param d 字典
 * @param f 文件指针
 */
void iniparser_dump_ini(dictionary *d, FILE *f);

/**
 * @brief 获取section数量
 * @param d 字典
 * @return section数量
 */
int iniparser_getnsec(dictionary *d);

/**
 * @brief 获取指定索引的section名称
 * @param d 字典
 * @param n 索引
 * @return section名称
 */
char* iniparser_getsecname(dictionary *d, int n);

/**
 * @brief 查找section中是否存在指定key
 * @param d 字典
 * @param sec section名
 * @param key key名
 * @return 1存在，0不存在
 */
int iniparser_find_entry(dictionary *d, const char *sec, const char *key);

/* 向后兼容宏 */
#define iniparser_getstr(d, k)  iniparser_getstring(d, k, NULL)
#define iniparser_setstr        iniparser_setstring

#ifdef __cplusplus
}
#endif

#endif /* _INIPARSER_H_ */
