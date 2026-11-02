#ifndef  _MQTT_COMMON_H__
#define  _MQTT_COMMON_H__


#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <queue>
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <glib.h>
#include "sr_log.h"
#include "err_code.h"
#include "common_functions.h"
#include "sr_ipc.h"
#include <stdint.h>
#include <dirent.h>

#ifdef __cplusplus
extern "C" {
#endif


#define PM_MQTT_APP_NAME                "mqtt_proto"

#define IN  //input param
#define OUT //output param
#define INOUT//input & output param

// 模组升级状态定义
#define MODEM_UPGRADE_STATUS_INVALID    0   // 无效状态
#define MODEM_UPGRADE_STATUS_COMPLETED  1   // 升级完成
#define MODEM_UPGRADE_STATUS_FAILED     2   // 升级失败
#define MODEM_UPGRADE_STATUS_REQUESTED  4   // 升级请求
#define MODEM_UPGRADE_STATUS_UPGRADING  5   // 升级中

unsigned char *StrToHex(unsigned char *pbDest, const unsigned char *pbSrc,int nLen);

unsigned char *HexToStr(unsigned char *pbDest, const unsigned char *pbSrc,int nLen);

int HexToStr2(unsigned char *pbDest, int destLen, const unsigned char *pbSrc,int nLen);

int GetSystemRetValue(const char *cmd);

int GetFileContent(const char *filePathName,char *content, int contentLen);

int GetFotaTmpFileContentAndEmptyFile(const char *filePath, char *content, int contentLen);

int GetMd5sumValue(char *md5SumValue, int md5SumLen, char *md5Data, int md5DataLen);

gint NumToBCDCode(char *numData, int numDataLen, char *bcdData, int bcdDataLen);

int GetBJTime(char *data, int dataLen);

/**
 * @brief Get the Pm State object
 * 
 * @retval	RET_ERR     Error
 *          SGMW_PM_STATE_INIT     INIT
 *          SGMW_PM_STATE_SLEEP    Sleep
 *          SGMW_PM_STATE_NORMAL   normal
 */
uint8_t GetPmState();

/**
 * @brief Set the Pm State object
 * 
 * @retval	RET_ERR     Error
 *          SGMW_PM_STATE_INIT     INIT
 *          SGMW_PM_STATE_SLEEP    Sleep
 *          SGMW_PM_STATE_NORMAL   normal
 */
void SetPmState(uint8_t pmState);

//更新唤醒源
void SetWakeupSrc(uint8_t WakeupSrc);
//获取当前唤醒源
uint8_t GetWakeupSrc(void);
//设置一次性RTC 唤醒标志
uint8_t SetOnceRtcflag(void);
//获取一次性RTC 唤醒标志
uint8_t GetOnceRtcflag(void);
//清除一次性RTC 唤醒标志
uint8_t ClearOnceRtcflag(void);
//返回是否发送过1次MQTT
uint8_t GetOneMqttFlag(void);
//设置/清除已发 1 次 MQTT 标志
uint8_t SetOneMqttFlag(uint8_t mqttFlag);

/**
 * @brief 设置或清除 RTC 上报休眠事件
 * @param[in] flag 操作标志：1-设置事件，0-清除事件
 * 
 * @note 内部使用互斥锁保证线程安全，只在设置过事件后才能清除事件
 */
void SetOrClearRtcSleepEvent(uint8_t flag);

//主模组获取升级状态
int ReadMainModemUpgradeStatus(void);
//副模组接口返回当前升级状态
int ReadSlaveModemUpgradeStatusFromUrl(void);
int ReadSlaveModemUpgradeStatus(void);
//主模组升级进度
uint8_t ReadMainUpProgress(void);
//副模组升级进度
uint8_t ReadSlaveUpProgress(void);

/**
 * @brief 周期性获取副模组升级状态线程函数
 * @param[in] arg 线程参数（未使用）
 */
void* curlGetUpgradeStatusThread(void *arg);

/**
 * @brief 获取缓存的副模组升级状态
 * @retval 升级状态值
 */
int GetCachedSlaveModemUpgradeStatus(void);

/**
 * @brief 更新缓存的副模组升级状态
 * @param[in] status 新的升级状态
 */
void UpdateCachedSlaveModemUpgradeStatus(int status);

/**
 * @brief 通用保存数据到文件函数
 * 
 * @param[in] data 需要保存的数据指针
 * @param[in] dataSize 数据大小
 * @param[in] fileName 文件名
 * @retval	RET_OK      Success
 *          RET_ERR     Error
 */
int32_t SaveInfoToFile(const void* data, size_t dataSize, const char* fileName);

/**
 * @brief 通用从文件加载数据函数
 * 
 * @param[out] data 加载数据的目标缓冲区
 * @param[in] dataSize 数据大小
 * @param[in] fileName 文件名
 * @retval	RET_OK      Success
 *          RET_ERR     Error
 */
int32_t LoadInfoFromFile(void* data, size_t dataSize, const char* fileName);

/**
 * @brief 检查数据是否为预设值（全0）
 * 
 * @param[in] data 需要检查的数据
 * @param[in] dataSize 数据大小
 * @retval	true        是预设值
 *          false       不是预设值
 */
bool IsPresetValue(const void* data, size_t dataSize);

#ifdef __cplusplus
}
#endif
#endif		