/**
 * @file param_types.h
 * @brief 参数管理基础类型定义
 * @description 可移植简化版参数管理模块 - 基础类型定义
 */

#ifndef __PARAM_TYPES_H__
#define __PARAM_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* 错误码定义 */
typedef enum {
    PARAM_ERR_OK            = 0,    /* 成功 */
    PARAM_ERR_NULL          = -1,   /* 空指针 */
    PARAM_ERR_PARAM         = -2,   /* 参数错误 */
    PARAM_ERR_NOT_FOUND     = -3,   /* 参数未找到 */
    PARAM_ERR_TYPE          = -4,   /* 类型错误 */
    PARAM_ERR_SIZE          = -5,   /* 大小错误 */
    PARAM_ERR_RANGE         = -6,   /* 范围错误 */
    PARAM_ERR_MEMORY        = -7,   /* 内存错误 */
    PARAM_ERR_FILE          = -8,   /* 文件错误 */
    PARAM_ERR_MUTEX         = -9,   /* 锁错误 */
    PARAM_ERR_INIT          = -10,  /* 未初始化 */
} PARAM_ERR_E;

/* 参数类型定义 */
typedef enum {
    PARAM_TYPE_INT      = 0,    /* 整型 */
    PARAM_TYPE_DOUBLE   = 1,    /* 浮点型 */
    PARAM_TYPE_STRING   = 2,    /* 字符串 */
    PARAM_TYPE_ARRAY_INT    = 3,    /* 整型数组 */
    PARAM_TYPE_ARRAY_DOUBLE = 4,    /* 浮点数组 */
} PARAM_TYPE_E;

/* 参数ID范围定义 */
#define PARAM_ID_STATUS_MIN     0x0000      /* 状态参数起始 */
#define PARAM_ID_STATUS_MAX     0x2FFF      /* 状态参数结束 */

#define PARAM_ID_CTRL_MIN       0x3000      /* 控制参数起始 */
#define PARAM_ID_CTRL_MAX       0x3FFF      /* 控制参数结束 */

#define PARAM_ID_CONFIG_MIN     0x4000      /* 配置参数起始 */
#define PARAM_ID_CONFIG_MAX     0x7FFF      /* 配置参数结束 */

/* 参数ID类型 */
typedef uint16_t PARAM_ID;

/* 参数值最大长度 */
#define PARAM_VALUE_MAX_LEN     256
#define PARAM_NAME_MAX_LEN      64
#define PARAM_TABLE_NAME_LEN    32
#define PARAM_MAX_CALLBACKS     16
#define PARAM_MAX_TABLES        8

/* 参数列表结构体 - 用于批量操作 */
typedef struct {
    PARAM_ID    pid;                    /* 参数ID */
    int         type;                   /* 参数类型 */
    int         index;                  /* 数组索引 */
    union {
        int     valueInt;               /* 整型值 */
        double  valueDouble;            /* 浮点值 */
        char    valueStr[PARAM_VALUE_MAX_LEN];  /* 字符串值 */
        int     valueIntArray[PARAM_VALUE_MAX_LEN/sizeof(int)]; /* 整型数组 */
        double  valueDoubleArray[PARAM_VALUE_MAX_LEN/sizeof(double)]; /* 浮点数组 */
    };
} ParamList;

/* 参数值联合体 */
typedef union {
    int     i;
    double  d;
    char    s[PARAM_VALUE_MAX_LEN];
    int     ai[PARAM_VALUE_MAX_LEN/sizeof(int)];
    double  ad[PARAM_VALUE_MAX_LEN/sizeof(double)];
} ParamValue;

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_TYPES_H__ */
