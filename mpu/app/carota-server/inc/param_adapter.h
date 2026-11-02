/**
 * @file param_adapter.h
 * @brief 参数管理模块兼容适配层（独立版本）
 * @description 提供与原TB参数接口的兼容适配，不依赖原项目头文件
 */

#ifndef __PARAM_ADAPTER_H__
#define __PARAM_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 包含 param_manager.h，它会提供底层实现 */
#include "param_manager.h"

/* ==================== 兼容类型定义（原 tbparamlist.h 中的定义）==================== */

/* 参数值最大长度 */
#define MPARAM_MAX_PSRRING      72
#define MPARAM_MAX_NAME         64
#define P_MAX_PARAM_KEY         512
#define P_MAX_INI_FILE          2

/* 参数ID类型（与原项目兼容） */
typedef uint16_t TBPARAM_ID_DEF;

/* INI值类型定义 */
typedef enum {
    TYPE_INIVALUE_INT = 0,      /* 整型 */
    TYPE_INIVALUE_DOUBLE,       /* 双精度浮点 */
    TYPE_INIVALUE_BOOL,         /* 布尔型 */
    TYPE_INIVALUE_STRING,       /* 字符串 */
    TYPE_INIVALUE_OTHER,        /* 其他类型 */
    TYPE_INIVALUE_FLOAT,        /* 单精度浮点（扩展） */
    TYPE_INIVALUE_INTARRAY      /* 整型数组（扩展） */
} TBP_INIVALUE_TYPE;

/* 参数表索引定义（与原项目兼容） */
#define PARAM_INI_CONFIG_INDEX      0   /* 可恢复参数表索引 */
#define PARAM_INI_DEVICE_INDEX      1   /* 设备固定参数表索引 */

/* 参数列表表项（与原项目 TBParamListTable 兼容） */
typedef struct {
    TBPARAM_ID_DEF pid;                         /* 参数ID */
    unsigned char pname[MPARAM_MAX_PSRRING];    /* 参数名称 */
    TBP_INIVALUE_TYPE ptype;                    /* 参数类型 */
    unsigned int pSize;                         /* 参数大小 */
    union {
        unsigned char pdefault[MPARAM_MAX_PSRRING]; /* 默认值 */
        unsigned int pdefaultInt;
        double pdefaultDouble;
        float pdefaultFloat;
    };
} TBParamListTable;

/* 参数列表项（与原项目 TBParamList 兼容） */
typedef struct {
    TBPARAM_ID_DEF pid;             /* 参数ID */
    int index;                      /* 数组索引 */
    union {
        unsigned char valueStr[MPARAM_MAX_PSRRING];
        unsigned int valueInt;
        unsigned int valueIntArray[MPARAM_MAX_PSRRING/4];
        unsigned short valueShort;
        unsigned char valueChar;
        float valueFloat;
        double valueDouble;
    };
} TBParamList;

/* 回调函数类型定义 */
typedef int (*tbparameterupdate)(void *pUsrArg, TBParamList *pList, int number, int beset);

/* ==================== 适配层API ==================== */

/**
 * @brief 适配层初始化
 * @param ini_path INI文件路径
 * @return 0成功
 */
int TBParameterAdapterInit(const char *ini_path);

/**
 * @brief 适配层释放
 */
int TBParameterAdapterRelease(void);

/**
 * @brief 注册参数表（适配原接口）
 * @param index INI文件索引
 * @param pParamListTable 参数表
 * @param pListTablename 表名
 * @param size 表项数量
 * @param updateCallback 更新回调
 * @param pUsr 用户数据
 * @return 0成功
 */
int TBParameterUpdateNotifyRegister0(int index, 
                                      TBParamListTable *pParamListTable,
                                      unsigned char *pListTablename,
                                      int size,
                                      tbparameterupdate updateCallback,
                                      void *pUsr);

/**
 * @brief 简化版注册宏（与原接口兼容）
 */
#define TBParameterUpdateNotifyRegister(index, listTable, callback, usr) \
    TBParameterUpdateNotifyRegister0(index, listTable, (unsigned char *)#listTable, \
                                      sizeof(listTable)/sizeof(TBParamListTable), callback, usr)

/**
 * @brief 批量获取参数（适配原接口 - 使用 TBParamList）
 * @param pList 参数列表
 * @param number 参数个数
 * @return 0成功
 */
int __TBParameterAdapterGetByIDList(TBParamList *pList, int number);

/**
 * @brief 批量获取参数（与原接口完全兼容）
 * @param pList 参数列表（TBParamList数组）
 * @param number 参数个数
 * @return 0成功
 *
 * 注意：这是一个宏，将旧接口调用转发到适配层实现
 */
#define TBParameterGetByIDList(pList, number) \
    __TBParameterAdapterGetByIDList((TBParamList *)(pList), number)

/* 向后兼容：旧函数名映射到新函数 */
#define TBParameterAdapterGetByIDList __TBParameterAdapterGetByIDList

/**
 * @brief 批量设置参数（适配原接口 - 使用 TBParamList）
 * @param pList 参数列表
 * @param number 参数个数
 * @return 0成功
 */
int TBParameterAdapterSetByIDList(TBParamList *pList, int number);

/**
 * @brief 字符串方式批量获取（适配原接口 - 使用 TBParamList）
 */
int TBParameterAdapterStringGetByIDList(TBParamList *pList, int number);

/**
 * @brief 字符串方式批量设置（适配原接口 - 使用 TBParamList）
 */
int TBParameterAdapterStringSetByIDList(TBParamList *pList, int number);

/**
 * @brief 恢复默认参数
 */
int TBParameterDefaultSet(void);

/**
 * @brief 保存参数到INI
 */
int TBParameterSave(void);

/**
 * @brief 模块初始化（兼容旧版）
 */
int TBParameterInit(void);

/**
 * @brief 模块释放（兼容旧版）
 */
int TBParameterRelease(void);

/* ==================== 便捷宏定义 ==================== */

/* 快速访问宏 */
#define XPIntV()        (listtable[0].valueInt)
#define XPIntIV(i)      (listtable[i].valueInt)
#define XPStringV()     (listtable[0].valueStr)
#define XPStringIV(i)   (listtable[i].valueStr)

#define XPidSetI(i,id)  listtable[i].pid = id

/* 保存字符串宏 */
#define XSaveString(p,i,s,l) \
do { \
    listtable[i].pid = p; \
    TBParameterAdapterGetByIDList(&listtable[i], 1); \
    memcpy(listtable[i].valueStr, (char *)(s), (l)); \
    listtable[i].valueStr[l] = '\0'; \
    TBParameterAdapterSetByIDList(&listtable[i], 1); \
} while(0)

/* 获取字符串宏 */
#define XGetString(p,i,s,ptrl) \
do { \
    listtable[i].pid = p; \
    TBParameterAdapterGetByIDList(&listtable[i], 1); \
    if((ptrl) && *(ptrl)) { \
        memcpy((char *)(s), (char *)listtable[i].valueStr, *(ptrl)); \
    } else { \
        strncpy((char *)(s), (char *)listtable[i].valueStr, MPARAM_MAX_PSRRING - 1); \
        ((char *)(s))[MPARAM_MAX_PSRRING - 1] = '\0'; \
        if(ptrl) { \
            *(ptrl) = strlen((char *)(s)); \
        } \
    } \
} while(0)

/* 字符串方式保存宏 */
#define XStrSaveString(p,i,s,l) \
do { \
    listtable[i].pid = p; \
    TBParameterAdapterStringGetByIDList(&listtable[i], 1); \
    memcpy((char *)listtable[i].valueStr, (char *)(s), (l)); \
    listtable[i].valueStr[l] = '\0'; \
    TBParameterAdapterStringSetByIDList(&listtable[i], 1); \
} while(0)

/* 字符串方式获取宏 */
#define XStrGetString(p,i,s,ptrl) \
do { \
    listtable[i].pid = p; \
    TBParameterAdapterStringGetByIDList(&listtable[i], 1); \
    if((ptrl) && *(ptrl)) { \
        memcpy((char *)(s), (char *)listtable[i].valueStr, *(ptrl)); \
    } else { \
        strncpy((char *)(s), (char *)listtable[i].valueStr, MPARAM_MAX_PSRRING - 1); \
        ((char *)(s))[MPARAM_MAX_PSRRING - 1] = '\0'; \
        if(ptrl) { \
            *(ptrl) = strlen((char *)(s)); \
        } \
    } \
} while(0)

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_ADAPTER_H__ */
