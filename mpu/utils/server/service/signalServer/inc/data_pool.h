/**
 * @file data_pool.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief 
 * @version 0.1
 * @date 2023-12-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _UPGRADE_DATA_POOL_H_
#define _UPGRADE_DATA_POOL_H_
#include "common.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif
typedef void (*SignalParserCb)(uint32_t idx, gpointer pUserData);

#define SIGNAL_TYPE_FLOAT (1)
#define SIGNAL_TYPE_UINT8 (2)
#define SIGNAL_TYPE_UINT16 (3)
#define SIGNAL_TYPE_UINT32 (4)
#define SIGNAL_TYPE_UINT64 (5)
#define SIGNAL_TYPE_INT8 (6)
#define SIGNAL_TYPE_INT16 (7)
#define SIGNAL_TYPE_INT32 (8)
#define SIGNAL_TYPE_INT64 (9)

/**
 * @brief 存储CAN数据解析的配置数据（从配置文件中获取）
 */
typedef struct
{
    uint32_t id; // 0~28 canid,29~31 channel id
    int32_t encode;
    int32_t sigIdx;
    int32_t startBit;
    int32_t dataLen;
    char signalTypeName[7]; //"float""int8""int16""int32""int64""uint8""uint16""uint32""uint64"
    int8_t signalType;      //"float"==1,"uint8"==2,"uint16"==3,"uint32"==4,"uint64"==5,"int8"==6,"int16"==7,"int32"==8,"int64"==9
    double offset;
    double accuracy;
    int32_t defaultValue;
    int32_t multiple;
    int32_t multipleMax;
    int32_t multipleIndex;
    bool realUpload;
    bool setInvalid;
    double invalidValue;
} Dp_Signal_Data_t, *P_Dp_Signal_Data_t;

/**
 * @brief can信号值结构
 * 
 */
typedef union
{
    float floatVal;
    uint64_t uint64Val;
} Dp_Value_u;

/**
 * @brief 存储CAN数据解析的配置数据
 * 
 */
typedef struct
{
    Dp_Signal_Data_t stSignal;
    Dp_Value_u value;
    bool issigned; // true:(value类型为int8-64&&当前值为负数)，false:(value类型为float||uint8-64||(int8-64&&当前值为正数))
    uint32_t canCircle;
} Dp_Signal_t, *P_Dp_Signal_t;

/**
 * @brief 信号服务结构
 * 
 */
typedef struct 
{
    char version[128];
    GHashTable *pSpeedTable; //idx---Dp_Signal_t*
    GHashTable *pSigNameTable;//signalName---Dp_Signal_t*
    GHashTable *pParserView; //"channelId"<<29|"canId"---假设该id下有8个信号，那么value就代表这个id下的第1个信号在pAll中的保存位置
    GSList *pAll;            //所有的信号数据(P_Dp_Signal_t类型)都会在pAll链表里面保存一份
    GArray *canIdFilter;     //保存mcu过滤canid的动态数组
}Dp_Data_t,*P_Dp_Data_t;


/**
 * @brief 初始化dbcjson
 * 
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpLoadConfig(void);

/**
 * @brief 初始化structjson
 * 
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t dealSendCanSignalMsgs(void);

/**
 * @brief 获取当前dbcjson中的所有CAN ID
 * 
 * @param pData canid数组
 * @param pLen canid数量
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetCanid(OUT uint32_t **pData, OUT uint32_t *pLen);

/**
 * @brief 获取当前structjson中的canid过滤列表
 * 
 * @param pData canid数组
 * @param pLen canid数量
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetFilterCanid(OUT uint32_t **pData, OUT uint32_t *pLen);

/**
 * @brief 处理can原始数据并调用信号处理函数
 * 
 * @param key CAN存储key
 * @param pData CAN数据
 * @param sigParserCb 信号处理函数
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpCanSignalForeach(IN uint32_t key, OUT GSList **firstSignal);

/**
 * @brief 获取指定can的编码类型
 * 
 * @param ch can通道
 * @param canid canid
 * @param pType 编码类型
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetCanEncodeType(IN uint8_t ch, IN uint32_t canid, OUT uint8_t *pType);
/**
 * @brief 检查指定idx的信号是否存在
 * 
 * @param idx 
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetSignalCheckIdx(IN uint32_t idx);

/**
 * @brief 获取指定idx信号的id(CAN存储key)
 * 
 * @param idx 
 * @param pId 获取到的CAN存储key
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetSignalId(IN uint32_t idx, OUT uint32_t *pId);

/**
 * @brief 获取指定idx信号的编码方式
 * 
 * @param idx 
 * @param pEncode 获取到的编码方式
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetSignalEncode(IN uint32_t idx, OUT int32_t *pEncode);

/**
 * @brief 获取指定idx信号的开始bit位
 * 
 * @param idx 
 * @param pStartBit 获取到的开始bit位
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetSignalStartBit(IN uint32_t idx, OUT int32_t *pStartBit);

/**
 * @brief 获取指定idx信号的bit长度
 * 
 * @param idx 
 * @param pDataLen 获取到的bit长度
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetSignalDataLen(IN uint32_t idx, OUT int32_t *pDataLen);

/**
 * @brief 获取指定idx信号值的偏移量
 * 
 * @param idx 
 * @param pType 获取到的信号值的偏移量
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetSignalOffset(IN uint32_t idx, OUT double *pType);

/**
 * @brief 获取指定idx信号值的因子
 * 
 * @param idx 
 * @param pType 获取到的信号值的因子
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetSignalAccuracy(IN uint32_t idx, OUT double *pType);

/**
 * @brief 获取指定idx信号值的默认值
 * 
 * @param idx 
 * @param pType 获取到的信号值的默认值
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetSignalDefaultValue(IN uint32_t idx, OUT int32_t *pType);

/**
 * @brief 获取指定idx信号值的真实值
 * 
 * @param idx 
 * @param pValue 获取到的信号值的真实值
 * @param issigned 是否有符号
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetSignalValue(IN uint32_t idx, OUT Dp_Value_u *pValue, bool *issigned);

/**
 * @brief 设置指定idx信号的信号值
 * 
 * @param idx 
 * @param val 设置信号值
 * @param issigned 是否有符号
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpSetSignalValue(IN uint32_t idx, IN Dp_Value_u val, bool issigned);

/**
 * @brief 设置指定信号的信号值
 * 
 * @param signal P_Dp_Signal_t信号数据指针
 * @param val 设置信号值
 * @param issigned 是否有符号
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t setSignalValueAndSigned(IN P_Dp_Signal_t signal, IN Dp_Value_u val, bool issigned);

/**
 * @brief 获取指定idx的信号数据
 * 
 * @param idx 
 * @param signalData 获取到的信号数据
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetSignal(IN uint32_t idx, OUT Dp_Signal_Data_t *signalData);

/**
 * @brief 获取指定idx的信号结构地址
 * 
 * @param idx 
 * @param signalData 获取到的信号数据
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t DpGetSignalAddr(IN uint32_t idx, OUT P_Dp_Signal_t *signalData);


void *RealDataUploadHandle(void *arg);


int32_t CanParserAndStructDataLock();

int32_t CanParserAndStructDataUnlock();

#ifdef __cplusplus
}
#endif
#endif
