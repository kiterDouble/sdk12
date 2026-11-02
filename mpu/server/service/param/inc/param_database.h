/**
 * @file param_database.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief 数据库操作方法
 * @version 0.1
 * @date 2023-12-27
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __PARAM_DATABASE_H__
#define __PARAM_DATABASE_H__
#ifdef __cplusplus
extern "C"
{
#endif
#include <glib.h>
#include "param_msg.h"
#include "sr_param.h"

/**
 * @brief 从数据库进行初始化 读数据库
 * 
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t ParamsDbInit(void);

/**
 * @brief 加密数据初始化
 * 
 * @return int32_t 
 */
void ParamsEncryptInit(void);

/**
 * @brief 获取一个参数
 * 
 * @param para 方向IO 传入para.id，得到一个参数数据
 * @return true 获取成功
 * @return false 获取失败
 */
bool GetParams(ParamInfo_t *para);

/**
 * @brief 获取参数来源
 * 
 * @param paramIndex 参数id
 * @return uint8_t 参数来源 PARAM_SOURCES_MODEM PARAM_SOURCES_MCU PARAM_SOURCES_DB PARAM_SOURCES_USER
 */
uint8_t GetParamsSources(DeviceParamEnum paramIndex);

/**
 * @brief 获取参数值
 * 
 * @param paramIndex 参数id
 * @param paramValue 参数值
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t GetParamsValue(DeviceParamEnum paramIndex, void* paramValue);

/**
 * @brief 设置参数值
 * 
 * @param paramName 参数id
 * @param paramValue 参数值
 * @param syncToMcu 是否同步mcu标识
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t SetParamsValue(DeviceParamEnum paramName, void* paramValue, bool syncToMcu);

/**
 * @brief 获取参数类型
 * 
 * @param paramIndex 参数id
 * @param type 参数类型
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t GetParamsValueType(DeviceParamEnum paramIndex, uint8_t *type);

/**
 * @brief 设置用户参数
 * 
 * @param param 用户参数
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t SetUserParams(ParamInfo_t param);

/**
 * @brief 删除用户参数
 * 
 * @param id 参数id
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DeleteUserParamWithId(const uint32_t id);

/**
 * @brief 获取sql返回的行数
 * 
 * @param dbfile dbfile文件绝对路径
 * @param sql sql语句
 * @return int32_t sql返回的行数
 */
int32_t GetCountNumBySql(const char *dbfile,const char *sql);

/**
 * @brief 执行sql但并不关心任何sql的返回
 * 
 * @param sql sql语句
 * @return int32_t sql执行结果
 */
int32_t ExecNoReturnSql(const char *sql);

/**
 * @brief 删除sql表格
 * 
 * @param tableName table名称
 * @return int32_t sql执行结果
 */
int32_t DropTable(const char *tableName);

/**
 * @brief 拷贝指定表格为新的备份表格
 * 
 * @param tableName 要备份的表名
 * @return int32_t sql执行结果
 */
int32_t CreateBakTableFrom(const char *tableName);

/**
 * @brief 删除指定的表格中的所有数据
 * 
 * @param tableName 表格名
 * @return int32_t sql执行结果
 */
int32_t DeleateAllElementFromTaleb(const char *tableName);

/**
 * @brief 从备份表中选择数据，将其更新到目标表中
 * 
 * @param tableName 要操作的表名
 * @param bakName 备份表名
 * @return int32_t sql执行结果
 */
int32_t UpdateFromBakDb(const char *tableName, const char *bakName);

/**
 * @brief 从备份表中将所有用户数据拷贝出来给目标表
 * 
 * @param tableName 要操作的表名
 * @param bakName 备份表名
 * @return int32_t sql执行结果
 */
int32_t UpdateUserParamsFromBakDb(const char *tableName, const char *bakName);

/**
 * @brief 优化参数数据库文件大小
 * 
 */
void FreeSpace(void);

/*
 * @brief 数据库健康检查函数
 *
 */
bool ParamDatabaseHealthCheck(const char* dbPath);

/*
 * @brief 加载单个参数
 *
 */
bool ParamDatabaseLoadSingleCell(char* dbfile, int32_t targetId, ParamInfo_t *paramCell);

/*
 * @brief 数据库备份
 *
 */
bool ParamDatabaseBackUp(const char *dstDbPath,const char *srcDbPath);


#ifdef __cplusplus
}
#endif

#endif
