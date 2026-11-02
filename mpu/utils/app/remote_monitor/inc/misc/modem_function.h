#ifndef _MODEM_FUNCTION_H_
#define _MODEM_FUNCTION_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#pragma pack(1)
typedef enum 
{
    SIM_CARD_STATUS_PRESENT = 0, //已插卡
    SIM_CARD_STATUS_ABSENT,  //未插卡
    SIM_CARD_STATUS_ERROR //卡出错
}TBoxSimState;

typedef enum 
{
    SIM_AIRPLANE_OFF = 0, //正常模式
    SIM_AIRPLANE_ON, //飞行模式
    SIM_AIRPLANE_UNKOWN  //其他异常
} TBoxSimMode;

typedef struct TBoxNetWorkProviderStruct
{
    uint16_t mcc;
    uint8_t mnc;
    uint8_t providerLen;
    uint8_t  networkProvider[32];
} TBoxNetWorkProvider_t;

typedef struct TBoxSIMInfoStruct
{
    uint8_t imei[16];
    uint8_t imsi[16];
    uint8_t iccid[21];
    
} TBoxSIMInfo_t;

typedef enum 
{
    TYPE_GSM = 0,
    TYPE_LTE,
} TBoxNetType;

typedef struct TBoxSignalStrengthStruct
{
    TBoxNetType netType;
    int8_t signalStrength;  //CSQ 1~31 or 99
} TBoxSignalStrength_t;

#pragma pack()

/**
 * @brief 订阅广播信息
 * @return 
*/
// void InitIpcProcess(void* userdata);

/**
 * @brief 获取网络供应商
 * @return 成功 RET_OK 失败 RET_ERR
*/
int32_t GetNetworkProvider(TBoxNetWorkProvider_t *netProvider);

/**
 * @brief 获取sim卡信息（IMSI、ICCID、IMEI）
 * @return 成功 RET_OK 失败 RET_ERR
*/
int32_t GetSimInfo(TBoxSIMInfo_t *simInfo);

/**
 * @brief 获取sim卡信号类型
 * @return 成功 RET_OK 失败 RET_ERR
*/
int32_t GetSimNetType(TBoxNetType *netType);

/**
 * @brief 获取信号强度
 * @return 成功 RET_OK 失败 RET_ERR
*/
int32_t GetSignalStrehgth(uint8_t *strength);

/**
 * @brief 获取sim卡状态
*/
int32_t GetSimStatus(TBoxSimState *status);

/**
 * @brief 获取sim卡模式
 * @return 成功 RET_OK 失败 RET_ERR
*/
int32_t GetSimMode(TBoxSimMode *mode);

/**
 * @brief modem 回调
*/
void GetModemSrvInfo(const uint8_t *data, const uint16_t len, void* userdata);

#ifdef __cplusplus
}
#endif


#endif