#ifndef _NET_SERVICE_API_H_
#define _NET_SERVICE_API_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NET_ADDR_LENGTH                             (16)
#define ETH_NAME_LENGTH                             (16)
#define NETWORK_APN_NUM                             (5)
#define MAX_ETH_NAME_LEN                            (64)
#define MAX_BIND_ETH_NUM                            (10)
#define ROUTE_IP_DOMAIN_LEN                         (64)
typedef struct{
    uint8_t profileIndex;
    uint8_t dialStatus;
    uint32_t dialTimes;
    char ip[NET_ADDR_LENGTH];
    char ethName[ETH_NAME_LENGTH];
    char gateway[NET_ADDR_LENGTH];
    char priDns[NET_ADDR_LENGTH];
    char secDns[NET_ADDR_LENGTH];
}StrNetStatusInfo_t;

//订阅SYS_QUE_TYPE_NET_STATUS_NOTIFY消息，每次拨号都会发布以下StrNetStatus_t结构数据
typedef struct{
    uint8_t apnNum;
    StrNetStatusInfo_t netStatusInfo[NETWORK_APN_NUM];
}StrNetStatus_t;

typedef union {
    uint8_t apnId; // modem网卡apn序号，取值0-4，与配置文件中对应
    char ethName[ETH_NAME_LENGTH]; // 其他网卡的网卡名称(wifi，以太网)
} UnionDeviceField_t;

typedef struct {
    uint8_t routeType; // 0:ip 1:domain
    uint32_t priority;
    char route[ROUTE_IP_DOMAIN_LEN];
    uint8_t routeDevice; // 0:modem 1:other
    UnionDeviceField_t deviceField;
} StrSetNetRoute_t;

typedef struct{
    int count;
    char devs[MAX_BIND_ETH_NUM][MAX_ETH_NAME_LEN];
}StrNetRouteBind_t;

/* 飞行模式触发来源 */
#define FLIGHT_MODE_SRC_NONE              (0x00)
#define FLIGHT_MODE_SRC_REG_FAIL          (0x01)
#define FLIGHT_MODE_SRC_DIAL_FAIL         (0x02)
#define FLIGHT_MODE_SRC_MQTT_DISCONNECT   (0x03)

/**
 * @brief 请求net_server执行飞行模式切换（统一入口）
 * @param source 触发来源，FLIGHT_MODE_SRC_xxx
 * @retval RET_OK 切换已执行, RET_ERR 被冷却期/限频拒绝
 */
extern int32_t RequestNetFlightMode(uint8_t source);

extern int32_t GetNetStatusInfo(StrNetStatus_t *netStatus,int32_t timeOut);
extern int32_t AddNetRouteConfig(StrSetNetRoute_t *setNetRoute,int32_t timeOut);
extern int32_t QueryNetRouteBind(StrNetRouteBind_t *routeBind,char IP[ROUTE_IP_DOMAIN_LEN],int32_t timeout);

#ifdef __cplusplus
}
#endif

#endif //_NET_SERVICE_API_H_


