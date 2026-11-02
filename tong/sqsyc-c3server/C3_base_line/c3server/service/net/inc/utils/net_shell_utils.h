#ifndef _NET_SHELL_UTILS_H_
#define _NET_SHELL_UTILS_H_

#include <stdint.h>


int DebugCmd(const char *cmd);

int32_t CheckNetworkConnectivity(const char* ip, const char* interface);

void SetDnsAddr(uint8_t* dnsPathName, uint8_t nameserverPri,uint8_t* priDns, uint8_t* secDns);

void AddNameServerToRouteTable(uint8_t dnsRouteSet,uint8_t* priDns, uint8_t* secDns, uint8_t* name);

void NetHighSpeedModeSet(uint8_t mode);

int manage_ping(time_t *nextPingTime, time_t currentTime);

#endif