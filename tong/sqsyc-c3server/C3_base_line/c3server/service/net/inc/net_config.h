#ifndef _NET_CONFIG_H_
#define _NET_CONFIG_H_

#include <stdint.h>
#include "hal_modem_dial.h"
#include "net_service_api.h"

/*
* @brief include files here
*
*/
#ifdef __cplusplus
extern "C" {
#endif

#define NET_CONFIG_FILE_NAME ("/oemapp/config/net.ini")
#define NET_FIRST_FILE_NAME ("/oemdata/config/net.ini")
#define DNS_PATH_NAME_MAX_LEN      (128)
#define CONFIG_DOMAIN_LEN (1024)

/* 网络异常检查默认参数（单位：秒/次数） */
#define NET_REG_FAIL_TRIGGER_RESET_NET_PERIOD_DEFAULT          150U
#define NET_REG_FAIL_RESET_NET_TRIGGER_RESET_TBOX_COUNT_DEFAULT 30U

#define NET_APN_DIAL_FAIL_TRIGGER_RESET_NET_PERIOD_DEFAULT          60U
#define NET_APN_DIAL_FAIL_RESET_NET_TRIGGER_RESET_TBOX_COUNT_DEFAULT 10U

#define NET_TYPE_CHECK_RESET_NET_PERIOD_DEFAULT          60U
#define NET_TYPE_CHECK_RESET_NET_MAX_COUNT_DEFAULT       3U

#define NET_CONNECTION_PING_CYCLE_DURATION_SECOND_DEFAULT           120U
#define NET_CONNECTION_PING_FAILED_COUNT_TRIGGER_RESET_NET_DEFAULT  5U
#define NET_CONNECTION_PING_FAILED_RESET_NET_COUNT_TRIGGER_RESET_TBOX_DEFAULT 3U

/** 网络异常检查参数 **/
typedef struct{
    /** 驻网异常 **/
    uint8_t netRegCheckEnable;
    struct{
        uint32_t regFailTriggerResetNetPeriod;               /** 驻网失败持续触发重启网络时间 **/
        uint32_t regFailResetNetTriggerResetTboxCount;       /** 驻网失败重启网络触发重启Tbox次数 **/
    }netRegCheckParam;
    /** 拨号异常  **/
    uint8_t apnDialCheckEnable;
    struct{
        uint32_t apnDialFailTriggerResetNetPeriod;             /** APN拨号失败触发重启网络周期 **/
        uint32_t apnDialFailResetNetTriggerResetTboxCount;     /** APN拨号失败重启网络触发重启Tbox次数 **/
    }apnDialCheckParam;
    /** 网络制式异常 **/
    uint8_t netTypeCheckEnable;
    struct{
        uint32_t periodCount;
        uint32_t resetNetPeriod[20];                         /** 2/3G网络制式触发重启网络周期 **/
        uint32_t resetNetMaxCount;                           /** 2/3G网络制式触发重启网络最大次数 **/
    }netTypeCheckParam;
    /** 网络连接异常  **/
    uint8_t netConnectionCheckEnable;
    struct{
        uint32_t pingCycleDurationSecond;                        /**  ping 周期 **/
        uint32_t pingFailedCountTriggerResetNet;                 /** ping 持续失败触发重启网络次数 **/
        uint32_t pingFailedResetNetCountTriggerResetTbox;        /** ping 持续失败重启网络次数后触发重Tbox次数 **/
    }netConnectionCheckParam;
}NetAnomalyCheckParam_t;

typedef struct
{
    uint8_t profileIndex;
    uint8_t pdpType;
    uint8_t authProto;
    uint8_t nameserverPri;
    uint8_t dnsRouteSet;
    uint8_t privateNet;
    char apnName[MODEM_TEXT_LEN];
    char usrName[MODEM_TEXT_LEN];
    char password[MODEM_TEXT_LEN];
    char privateRoute[CONFIG_DOMAIN_LEN];
    char domain[CONFIG_DOMAIN_LEN];
}NetApnInfoCfg_t;

typedef struct
{
    uint8_t ledPinNum;
    uint8_t apnNum;
    uint8_t autoDail;
    uint8_t dnsPathName[DNS_PATH_NAME_MAX_LEN];
    uint8_t highSpeedModeEnable;
    uint8_t supportBeWinnerSimCard;
    int32_t apnDialPriInterval;                   /** apn拨号 一级间隔(秒) **/
    int32_t apnDialSecInterval;                   /** apn拨号 二级间隔(秒) **/
    int32_t apnDialSuccOneHourThreadHoldCount;    /** apn拨号 一小时内成功次数 阀值，小于 使用 一级间隔，大于等 于使用 二级间隔 **/
    NetApnInfoCfg_t apnInfoCfg[NETWORK_APN_NUM];
    NetAnomalyCheckParam_t netAnomalyCheckParam;
} NetUserCfg_t;

int8_t LoadNetConfig(NetUserCfg_t *cfg, uint8_t pathNamLen);
int Ql_GPIO_Init(uint8_t pinNumber, uint8_t pinDirection);
int Ql_GPIO_SetLevel(uint8_t pinNumber, uint8_t pinLevel);
#ifdef __cplusplus
}
#endif

#endif //_NET_CONFIG_H_





