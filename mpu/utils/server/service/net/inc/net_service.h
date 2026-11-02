#ifndef _NET_SERVICE_H_
#define _NET_SERVICE_H_

#include "hal_modem_dial.h"
#include "net_service_api.h"
#include "modem_api.h"
#include "pm_server_api.h"
#include <glib.h>
#include <netinet/in.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CONFIG_DOMAIN_LEN (1024)

typedef struct
{
    uint8_t profileIndex;
    uint8_t pdpType;
    uint8_t authProto;
    uint8_t nameserverPri;
    uint8_t dnsRouteSet;
    uint32_t dialTimes;
    char apnName[MODEM_TEXT_LEN];
    char usrName[MODEM_TEXT_LEN];
    char password[MODEM_TEXT_LEN];
    uint8_t dialStatus;
    char ip[MODEM_IPADDR_LEN];
    char gateway[MODEM_IPADDR_LEN];
    char priDns[MODEM_IPADDR_LEN];
    char secDns[MODEM_IPADDR_LEN];
    char privateRoute[CONFIG_DOMAIN_LEN];
    char ethName[ETH_NAME_LENGTH];
    char domain[CONFIG_DOMAIN_LEN];
} StrNetInfo_t;

typedef struct
{
    int32_t callId;
    HalModemNetInfo_t netInfo;
} DialCB_t;

StrNetStatus_t FillNetStatusInfo(void);
int8_t SetIpRoute(StrSetNetRoute_t *SetNetRoute);
int8_t SetDomainRoute(StrSetNetRoute_t *SetNetRoute);
void NetModemSrvInfoCallback(const uint8_t *data, const uint16_t len, void *userdata);
gpointer NetMonitor(gpointer pArg);
gpointer NetStatusSyncMcu(gpointer pArg);
gpointer SyncModemAliveStatus(gpointer pArg);
gpointer ModulePeriodicNetworkCheck(gpointer pArg);
gpointer DnsCheckMonitor(gpointer pArg);
void StartNetService(void);
/**
 * @name NetPmChangeCallback
 * @brief 从给定数据中获取PM电源状态
 * @param data 指向包含PM电源状态数据的缓冲区的指针。
 * @param len 数据缓冲区的长度。
 * @param userdata 用户自定义数据的指针，本函数中未使用。
 * @return 无返回值
 */
void NetPmChangeCallback(const uint8_t *data, const uint16_t len, void *userdata);
void NetMonitorStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

#ifdef __cplusplus
}
#endif

#endif //_NET_SERVICE_H_
