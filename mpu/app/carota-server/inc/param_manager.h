/**
 * @file param_manager.h
 * @brief 参数管理模块对外接口头文件
 * @description 可移植简化版参数管理模块
 */

#ifndef __PARAM_MANAGER_H__
#define __PARAM_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "param_types.h"
#include "param_config.h"

/* 参数回调函数类型 */
typedef void (*ParamUpdateCallback)(PARAM_ID pid, void *pUsr);

/* 参数表项结构 */
typedef struct {
    PARAM_ID    pid;                /* 参数ID */
    const char  *name;              /* INI中的键名 (格式: "section:key") */
    int         type;               /* 类型：PARAM_TYPE_INT/DOUBLE/STRING/ARRAY */
    int         size;               /* 数组大小（标量填1） */
    void        *pdefault;          /* 默认值指针 */
} ParamListTable;

/* 参数表注册信息 */
typedef struct {
    ParamListTable  *table;         /* 参数表 */
    int             count;          /* 表项数量 */
    char            name[PARAM_TABLE_NAME_LEN]; /* 表名 */
    ParamUpdateCallback callback;   /* 变化回调 */
    void            *pUsr;          /* 回调用户数据 */
} ParamTableInfo;

/**
 * @brief 初始化参数管理模块
 * @param ini_path INI文件路径
 * @param default_ini_path 默认INI文件路径（可为NULL）
 * @return 0成功，失败返回错误码
 */
int ParamManagerInit(const char *ini_path, const char *default_ini_path);

/**
 * @brief 释放参数管理模块资源
 * @return 0成功
 */
int ParamManagerRelease(void);

/**
 * @brief 注册参数表
 * @param table 参数表指针
 * @param name 表名
 * @param count 表项数量
 * @param callback 参数变化回调函数（可为NULL）
 * @param pUsr 回调用户数据
 * @return 0成功，失败返回错误码
 * 
 * @note 建议在Init后调用，每个模块调用一次注册自己的参数表
 */
int ParamTableRegister(ParamListTable *table, const char *name, int count, 
                       ParamUpdateCallback callback, void *pUsr);

/**
 * @brief 根据参数ID获取参数值（核心接口）
 * @param pid 参数ID
 * @param index 数组索引（标量填0）
 * @param pValue 输出值缓冲区
 * @return 0成功，失败返回错误码
 * 
 * @note 对于控制参数(0x3000~0x3FFF)，读取后值自动清零
 */
int TBParameterGetByID(PARAM_ID pid, int index, void *pValue);

/**
 * @brief 根据参数ID设置参数值（核心接口）
 * @param pid 参数ID
 * @param index 数组索引（标量填0）
 * @param pValue 输入值指针
 * @return 0成功，失败返回错误码
 * 
 * @note 修改后需要调用ParamManagerSave()持久化到INI文件
 */
int TBParameterSetByID(PARAM_ID pid, int index, void *pValue);

/**
 * @brief 批量获取参数值
 * @param pList 参数列表
 * @param number 参数个数
 * @return 0成功，失败返回错误码
 */
int TBParameterGetByIDList(ParamList *pList, int number);

/**
 * @brief 批量设置参数值
 * @param pList 参数列表
 * @param number 参数个数
 * @return 0成功，失败返回错误码
 */
int TBParameterSetByIDList(ParamList *pList, int number);

/**
 * @brief 将字符串转换为对应类型的值并设置参数
 * @param pid 参数ID
 * @param index 数组索引
 * @param str 字符串值
 * @return 0成功，失败返回错误码
 */
int TBParameterSetFromString(PARAM_ID pid, int index, const char *str);

/**
 * @brief 获取参数值的字符串表示
 * @param pid 参数ID
 * @param index 数组索引
 * @param str 输出缓冲区
 * @param len 缓冲区长度
 * @return 0成功，失败返回错误码
 */
int TBParameterGetAsString(PARAM_ID pid, int index, char *str, int len);

/**
 * @brief 保存所有参数到INI文件
 * @return 0成功，失败返回错误码
 * 
 * @note 只有状态参数(0x0000~0x2FFF)和配置参数(0x4000+)会保存
 *       控制参数(0x3000~0x3FFF)不保存
 */
int ParamManagerSave(void);

/**
 * @brief 恢复所有参数为默认值
 * @return 0成功
 */
int ParamManagerRestoreDefault(void);

/**
 * @brief 判断参数ID是否为控制参数（读后清零）
 * @param pid 参数ID
 * @return 1是控制参数，0不是
 */
static inline int ParamIsControlType(PARAM_ID pid) {
    return (pid >= PARAM_ID_CTRL_MIN && pid <= PARAM_ID_CTRL_MAX);
}

/**
 * @brief 判断参数是否需要持久化
 * @param pid 参数ID
 * @return 1需要持久化，0不需要
 */
static inline int ParamNeedPersist(PARAM_ID pid) {
    return ((pid >= PARAM_ID_STATUS_MIN && pid <= PARAM_ID_STATUS_MAX) ||
            (pid >= PARAM_ID_CONFIG_MIN && pid <= PARAM_ID_CONFIG_MAX));
}

/* 便捷宏定义 */

/**
 * @brief 注册参数表（简化宏）
 * @param table 参数表名
 * @param callback 回调函数（可为NULL）
 * @param usr 用户数据
 */
#define PARAM_TABLE_REGISTER(table, callback, usr) \
    ParamTableRegister(table, #table, sizeof(table)/sizeof(ParamListTable), callback, usr)

/**
 * @brief 获取整型参数（直接返回值，失败返回0）
 */
static inline int ParamGetInt(PARAM_ID pid) {
    int val = 0;
    TBParameterGetByID(pid, 0, &val);
    return val;
}

/**
 * @brief 获取浮点参数（直接返回值，失败返回0.0）
 */
static inline double ParamGetDouble(PARAM_ID pid) {
    double val = 0.0;
    TBParameterGetByID(pid, 0, &val);
    return val;
}

/**
 * @brief 获取字符串参数（使用内部静态缓冲区）
 * @return 成功返回字符串指针，失败返回NULL
 * @note 返回的指针指向内部静态缓冲区，下次调用会覆盖，如需保存请立即拷贝
 */
static inline const char* ParamGetString(PARAM_ID pid) {
    static char buf[PARAM_VALUE_MAX_LEN];
    if (TBParameterGetByID(pid, 0, buf) == 0) {
        return buf;
    }
    return NULL;
}

/**
 * @brief 获取字符串参数到用户缓冲区
 * @param pid 参数ID
 * @param buf 用户缓冲区
 * @param len 缓冲区大小（必须 >= PARAM_VALUE_MAX_LEN）
 * @return 成功返回buf，失败返回NULL
 */
static inline const char* ParamGetStringCopy(PARAM_ID pid, char *buf, int len) {
    if (!buf || len < PARAM_VALUE_MAX_LEN) return NULL;
    if (TBParameterGetByID(pid, 0, buf) == 0) {
        return buf;
    }
    return NULL;
}

/**
 * @brief 获取整型参数到变量（传统方式）
 */
#define PARAM_GET_INT(pid, val) \
    TBParameterGetByID(pid, 0, &(val))

/**
 * @brief 设置整型参数
 */
#define PARAM_SET_INT(pid, val) \
    do { int _v = (val); TBParameterSetByID(pid, 0, &_v); } while(0)

/**
 * @brief 获取浮点参数到变量（传统方式）
 */
#define PARAM_GET_DOUBLE(pid, val) \
    TBParameterGetByID(pid, 0, &(val))

/**
 * @brief 设置浮点参数
 */
#define PARAM_SET_DOUBLE(pid, val) \
    do { double _v = (val); TBParameterSetByID(pid, 0, &_v); } while(0)

/**
 * @brief 获取字符串参数到缓冲区
 */
#define PARAM_GET_STRING(pid, buf, len) \
    TBParameterGetByID(pid, 0, (buf))

/**
 * @brief 设置字符串参数
 */
#define PARAM_SET_STRING(pid, str) \
    TBParameterSetByID(pid, 0, (void*)(str))

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_MANAGER_H__ */
