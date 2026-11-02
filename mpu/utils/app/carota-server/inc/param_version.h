/**
 * @file param_version.h
 * @brief 参数管理模块版本信息
 */

#ifndef __PARAM_VERSION_H__
#define __PARAM_VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 版本号定义 */
#define PARAM_MANAGER_VERSION_MAJOR     1
#define PARAM_MANAGER_VERSION_MINOR     0
#define PARAM_MANAGER_VERSION_PATCH     0

/* 版本字符串 */
#define PARAM_MANAGER_VERSION_STRING    "1.0.0"

/* 版本号组合 (用于条件编译比较) */
#define PARAM_MANAGER_VERSION           ((PARAM_MANAGER_VERSION_MAJOR << 16) | \
                                         (PARAM_MANAGER_VERSION_MINOR << 8) | \
                                         PARAM_MANAGER_VERSION_PATCH)

/* 功能特性标志 */
#define PARAM_FEATURE_INI_STORAGE       0x01   /* 支持INI文件存储 */
#define PARAM_FEATURE_THREAD_SAFE       0x02   /* 支持线程安全 */
#define PARAM_FEATURE_CALLBACK          0x04   /* 支持参数变化回调 */
#define PARAM_FEATURE_BATCH_OP          0x08   /* 支持批量操作 */
#define PARAM_FEATURE_CONTROL_PARAM     0x10   /* 支持控制参数（读后清零） */
#define PARAM_FEATURE_ARRAY_PARAM       0x20   /* 支持数组参数 */
#define PARAM_FEATURE_ADAPTER           0x40   /* 支持TB接口适配层 */

/**
 * @brief 获取版本字符串
 * @return 版本字符串，如 "1.0.0"
 */
static inline const char* ParamManagerGetVersionString(void) {
    return PARAM_MANAGER_VERSION_STRING;
}

/**
 * @brief 获取版本号
 * @return 版本号，格式为 (主版本 << 16) | (次版本 << 8) | 修订版本
 */
static inline int ParamManagerGetVersion(void) {
    return PARAM_MANAGER_VERSION;
}

/**
 * @brief 检查是否支持某个特性
 * @param feature 特性标志
 * @return 1支持，0不支持
 */
static inline int ParamManagerHasFeature(int feature) {
    switch (feature) {
        case PARAM_FEATURE_INI_STORAGE:     return 1;
        case PARAM_FEATURE_THREAD_SAFE:     return 1;
        case PARAM_FEATURE_CALLBACK:        return 1;
        case PARAM_FEATURE_BATCH_OP:        return 1;
        case PARAM_FEATURE_CONTROL_PARAM:   return 1;
        case PARAM_FEATURE_ARRAY_PARAM:     return 1;
        case PARAM_FEATURE_ADAPTER:         return 1;
        default:                            return 0;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_VERSION_H__ */
