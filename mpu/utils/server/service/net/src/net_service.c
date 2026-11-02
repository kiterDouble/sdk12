#include "net_service.h"
#include "net_rpc.h"
#include "err_code.h"
#include "mcu_api.h"
#include "modem_api.h"
#include "modem_ipc_msg.h"
#include "net_config.h"
#include "net_service_api.h"
#include "pm_server_api.h"
#include "sr_log.h"
#include "sr_param.h"
#include "net_route_cache.h"
#include "hal_modem_dial.h" 
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/reboot.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <net/if.h>
#include <sys/wait.h>
#include "net_monitor_ipc_msg.h"

#define PING_CMD "ping -c 1 -w 5 "
#define NET_ROUTE_DEFAULT_PRIORITY (100)
#define NET_STATUS_LENGTH          (17)
#define DNS_PATH_NAME_MAX_LEN      (128)
#define UP                          (1)
#define DOWN                        (2)
static ModemSrvNotify_t g_ModemSrvNotify = {0,0,1,0,0,0};
static uint8_t g_AutoDail = 1;

static int g_PreviousApnSwitched = 0;   // 用来标记是否切换过 APN
static uint8_t g_ApnNum = 0;
static uint8_t g_dnsPathName[DNS_PATH_NAME_MAX_LEN] = {0};
static StrNetInfo_t g_StrNetInfo[NETWORK_APN_NUM] = {0};
static int g_CurrentApnIndex = -1;      // 当前使用的 APN 索引

static MPUPmStateChange_t g_NetPmState = {0};
static uint8_t g_netMonitorStatus[2] = {0};
static int g_PingResult = 1;

#define MODEM_ID (1)
static uint8_t g_modemId = 0; 
static int g_VlanSwitch = UP; 

//#define MODEM_ID (2)
//static int g_VlanSwitch = DOWN; 


void sleep_clock(double seconds) {
    struct timespec req = {
        .tv_sec = (time_t)seconds,
        .tv_nsec = (long)((seconds - (time_t)seconds) * 1e9)
    };
    
    while (clock_nanosleep(CLOCK_MONOTONIC, 0, &req, &req) == EINTR) {
        //被中断继续
    }
}

static int addRoute(const char *ip, const char *dev, uint32_t priority)
{
    char command[256];
    snprintf(command, sizeof(command), "ip route replace %s dev %s metric %u", ip, dev, priority);
    int status = system(command);
    if (status == -1) {
        LOG_Error("Failed to execute command: %s\n", command);
        return RET_ERR;
    }

    return RET_OK;
}

static int getIPFromDomain(const char *domain, const char *dev, uint32_t priority)
{
    struct addrinfo hints, *result, *p;
    int status;
    uint8_t isFailed = 0;
    
    if (domain == NULL || NULL == dev)
    {
        LOG_Error("parameter domain or dev is NULL.\n");
        return RET_ERR;
    }

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET; // Use IPv4
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(domain, NULL, &hints, &result)) != 0) {
        LOG_Error("%s getaddrinfo error: %s\n", domain, gai_strerror(status));
        return RET_ERR;
    }

    for (p = result; p != NULL; p = p->ai_next) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        char ip[INET_ADDRSTRLEN] = {0};
        
        if (inet_ntop(AF_INET, &(ipv4->sin_addr), ip, sizeof(ip)) == NULL) 
        {
            LOG_Error("inet_ntop error\n");
            isFailed = 1;
            continue;
        }

        // Add to routing table
        if (addRoute(ip, dev, priority) != RET_OK) {
            LOG_Error("Failed to add route for IP: %s\n", ip);
            isFailed = 1;
            continue;
        }
        
        LOG_Info("IP address: %s added to route table with dev %s and priority %u\n", ip, dev, priority);
    }

    freeaddrinfo(result);
    
    if(isFailed == 1)
    {
        return RET_ERR;
    }
    else
    {
        return RET_OK;
    }
}

/*************************************************
Function:    CalculateIcmpChecksum
Description: Calculate ICMP checksum for packet validation
Input:       buf - pointer to buffer containing ICMP data
             len - length of buffer in bytes
Output:      None
Return:      Calculated 16-bit checksum
Others:      None
*************************************************/
static uint16_t CalculateIcmpChecksum(uint16_t *buf, int32_t len)
{
    uint32_t sum = 0U;

    while (len > 1)
    {
        sum += *buf++;
        len -= 2;
    }

    if (len == 1)
    {
        sum += *(uint8_t *)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFFU);
    sum += (sum >> 16);

    return (uint16_t)(~sum);
}

/*************************************************
Function:    IcmpSendAndReceive
Description: Send ICMP echo request and receive response
Input:       ip - target IP address string
             timeoutSec - receive timeout in seconds
             packet - buffer to store received packet
             packetSize - size of packet buffer
             device - network device name (e.g., "rmnet_data0"), NULL for no binding
Output:      packet - filled with received data
Return:      Number of bytes received on success, -1 on error
Others:      Uses raw socket, requires root privileges
*************************************************/
static int32_t IcmpSendAndReceive(const char *ip, int32_t timeoutSec, char *packet, size_t packetSize, const char *device)
{
    int32_t sock = -1;
    struct sockaddr_in addr = {0};
    struct icmp icmpHdr = {0};
    struct timeval tv = {0};
    ssize_t recvLen = -1;
    socklen_t addrLen = 0;

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0)
    {
        LOG_Debug("IcmpSendAndReceive: socket failed: %s", strerror(errno));
        return -1;
    }

    // Bind to specific device if provided
    if (device != NULL && device[0] != '\0')
    {
        if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, device, strlen(device)) < 0)
        {
            LOG_Debug("IcmpSendAndReceive: bind to device %s failed: %s", device, strerror(errno));
            close(sock);
            return -1;
        }        
        LOG_Debug("IcmpSendAndReceive: bound to device %s", device);
    }

    tv.tv_sec = timeoutSec;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        LOG_Debug("IcmpSendAndReceive: setsockopt SO_RCVTIMEO failed");
        close(sock);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
    {
        LOG_Debug("IcmpSendAndReceive: inet_pton failed for %s", ip);
        close(sock);
        return -1;
    }

    memset(&icmpHdr, 0, sizeof(icmpHdr));
    icmpHdr.icmp_type = ICMP_ECHO;
    icmpHdr.icmp_code = 0;
    icmpHdr.icmp_id = htons(getpid() & 0xFFFF);
    icmpHdr.icmp_seq = htons(1);
    icmpHdr.icmp_cksum = CalculateIcmpChecksum((uint16_t *)&icmpHdr, sizeof(icmpHdr));

    if (sendto(sock, &icmpHdr, sizeof(icmpHdr), 0,
               (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        LOG_Debug("IcmpSendAndReceive: sendto failed: %s", strerror(errno));
        close(sock);
        return -1;
    }

    addrLen = sizeof(addr);
    recvLen = recvfrom(sock, packet, packetSize, 0,
                        (struct sockaddr *)&addr, &addrLen);

    close(sock);

    return (int32_t)recvLen;
}

/*************************************************
Function:    ParseIcmpReply
Description: Parse ICMP reply packet to verify echo response
Input:       packet - received packet buffer
             len - length of received data
Output:      None
Return:      0 if valid ICMP echo reply, -1 otherwise
Others:      Validates packet structure before parsing
*************************************************/
static int32_t ParseIcmpReply(const char *packet, ssize_t len)
{
    struct ip *ipHdr = NULL;
    struct icmp *icmpReply = NULL;

    if (len < (ssize_t)(sizeof(struct ip) + sizeof(struct icmp)))
    {
        return -1;
    }

    ipHdr = (struct ip *)packet;
    icmpReply = (struct icmp *)(packet + (ipHdr->ip_hl << 2));

    if (icmpReply->icmp_type == ICMP_ECHOREPLY)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/*************************************************
Function:    IcmpPing
Description: Send ICMP ping to check host reachability
Input:       ip - target IP address string
             timeoutSec - timeout in seconds
             device - network device name (e.g., "rmnet_data0"), NULL for no binding
Output:      None
Return:      0 if host reachable, -1 on timeout or error
Others:      Wrapper function combining send, receive and parse
*************************************************/
static int32_t IcmpPing(const char *ip, int32_t timeoutSec, const char *device)
{
    char packet[128] = {0};
    int32_t recvLen = -1;

    if ((NULL == ip) || ('\0' == ip[0]))
    {
        LOG_Error("IcmpPing: ip is NULL or empty");
        return -1;
    }

    recvLen = IcmpSendAndReceive(ip, timeoutSec, packet, sizeof(packet), device);

    if (recvLen < 0)
    {
        if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
        {
            LOG_Debug("IcmpPing: timeout after %d seconds for %s", timeoutSec, ip);
        }
        else
        {
            LOG_Debug("IcmpPing: recv failed for %s", ip);
        }
        return -1;
    }

    if (0 == ParseIcmpReply(packet, recvLen))
    {
        LOG_Debug("IcmpPing: %s is reachable", ip);
        return 0;
    }
    else
    {
        LOG_Debug("IcmpPing: %s unreachable", ip);
        return -1;
    }
}

static int DebugCmd(const char *cmd)
{
    FILE *fp;
    int status;

    if (cmd == NULL || cmd[0] == '\0')
    {
        LOG_Error("DebugCmd: cmd is NULL or empty");
        return -1;
    }

    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        LOG_Debug("can not exec %s.", cmd);
        return -1;
    }

    char outputLine[1024] = {0};
    while (fgets(outputLine, sizeof(outputLine), fp) != NULL)
    {
        LOG_Debug("%s : %s", __FUNCTION__, outputLine);
    }

    status = pclose(fp);
    if (status == -1)
    {
        LOG_Debug("pclose failed for %s", cmd);
        return -1;
    }

    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status) == 0 ? 0 : 1;
    }

    return -1;
}

int8_t SetIpRoute(StrSetNetRoute_t *SetNetRoute)
{

    if (NULL == SetNetRoute)
    {
        LOG_Error("SetNetRoute is NULL");
        return RET_ERR;
    }

    char route[ROUTE_IP_DOMAIN_LEN] = {0};
    uint32_t priority = SetNetRoute->priority;

    if (SetNetRoute->routeDevice == 0)
    {
        // modem设备
        // if (SetNetRoute->deviceField.apnId < g_ApnNum)
        {
            // 获取对应全局变量g_StrNetInfo中ethName
            uint8_t pidExistFlag = 0;
            uint8_t i = 0;
            for(i = 0; i < g_ApnNum; i++)
            {
                if (SetNetRoute->deviceField.apnId == g_StrNetInfo[i].profileIndex)
                {
                    pidExistFlag = 1;
                    break;
                }
            }
            if (pidExistFlag == 0)
            {
                LOG_Error("Invalid pid");
                return RET_ERR;
            }

            const char *ethName = g_StrNetInfo[i].ethName;
            LOG_Debug("ethName: %s", ethName);
            // 获取路由信息
            strcpy(route, SetNetRoute->route);
            // 给网卡设置路由
            LOG_Debug("Setting route for modem device %s: %s, Priority: %u", ethName, route, priority);
            // 在这里添加设置路由的代码
            char command[1024];
            snprintf(command, sizeof(command), "ip route replace %s dev %s metric %u",
                     route, ethName, priority);
            int status = system(command);
            if (status == 0)
            {
                LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, command);
                CacheRoute(route,ethName);
                return RET_OK;
            } 
            else
            {
                LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, command);
                DebugCmd("route");
                return RET_ERR;
            }
        }
    }
    else if (SetNetRoute->routeDevice == 1)
    {
        // 其他设备
        const char *ethName = SetNetRoute->deviceField.ethName;
        // 获取路由信息
        strcpy(route, SetNetRoute->route);
        // 给网卡设置路由
        LOG_Debug("Setting route for other device %s: %s, Priority: %u", ethName, route, priority);
        // 在这里添加设置路由的代码
        char command[1024];
        snprintf(command, sizeof(command), "ip route replace %s dev %s metric %u", route, ethName, priority);
        int status = system(command);
        if (status == 0)
        {
            LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, command);
            CacheRoute(route,ethName);
            return RET_OK;
        }
        else
        {
            LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, command);
            DebugCmd("route");
            return RET_ERR;
        }
    }
    else
    {
        LOG_Error("Invalid route device type");
    }

    return RET_ERR;
}

int8_t SetDomainRoute(StrSetNetRoute_t *SetNetRoute)
{

    if (NULL == SetNetRoute)
    {
        LOG_Error("SetNetRoute is NULL");
        return RET_ERR;
    }

    uint32_t priority = SetNetRoute->priority;
    
    
    if (SetNetRoute->routeDevice == 0) {
        // modem设备
        // if (SetNetRoute->deviceField.apnId < g_ApnNum)
        {
            // 获取对应全局变量g_StrNetInfo中ethName
            // const char *ethName = g_StrNetInfo[SetNetRoute->deviceField.apnId].ethName;
            uint8_t pidExistFlag = 0;
            uint8_t i = 0;
            LOG_Debug("pid: %d", SetNetRoute->deviceField.apnId);

            for(i = 0; i < g_ApnNum; i++)
            {
                if (SetNetRoute->deviceField.apnId == g_StrNetInfo[i].profileIndex)
                {
                    pidExistFlag = 1;
                    break;
                }
            }
            if (pidExistFlag == 0)
            {
                LOG_Error("Invalid pid");
                return RET_ERR;
            }
            
            LOG_Debug("g_StrNetInfo[%d].profileIndex: %d", i, g_StrNetInfo[i].profileIndex);
            const char *ethName = g_StrNetInfo[i].ethName;
            LOG_Debug("ethName: %s", ethName);

            // 给网卡设置路由
            LOG_Debug("Setting route for modem device %s: %s, Priority: %u", ethName, SetNetRoute->route, priority);
            
            if (getIPFromDomain(SetNetRoute->route,ethName ,priority) == RET_OK)
            {
                CacheRoute(SetNetRoute->route,ethName);
                LOG_Debug("IP address for %s: %s", ethName, SetNetRoute->route);
            }
            else
            {
                LOG_Error("getIPFromDomain is err.");
                return RET_ERR;
            }
        }
    }
    else if (SetNetRoute->routeDevice == 1)
    {
        // 其他设备
        const char *ethName = SetNetRoute->deviceField.ethName;
        // 给网卡设置路由
        LOG_Debug("Setting route for other device %s: %s, Priority: %u", ethName, SetNetRoute->route, priority);

        if (getIPFromDomain(SetNetRoute->route,ethName ,priority) == RET_OK)
        {
            CacheRoute(SetNetRoute->route,ethName);
            LOG_Debug("IP address for %s: %s", ethName, SetNetRoute->route);
        }
        else
        {
            LOG_Error("getIPFromDomain is err.");
            return RET_ERR;
        }
    }
    else
    {
        LOG_Error("Invalid route device type");
        return RET_ERR;
    }

    return RET_OK;
}

StrNetStatus_t FillNetStatusInfo(void)
{
    StrNetStatus_t NetStatus = {0};
    if (g_ApnNum > NETWORK_APN_NUM)
    {
        NetStatus.apnNum = NETWORK_APN_NUM;
    }
    else
    {
        NetStatus.apnNum = g_ApnNum;
    }
    for (int loopNum = 0; loopNum < NetStatus.apnNum; loopNum++)
    {
        NetStatus.netStatusInfo[loopNum].profileIndex = g_StrNetInfo[loopNum].profileIndex;
        NetStatus.netStatusInfo[loopNum].dialStatus = g_StrNetInfo[loopNum].dialStatus;
        NetStatus.netStatusInfo[loopNum].dialTimes = g_StrNetInfo[loopNum].dialTimes;

        // 复制 ip
        strncpy(NetStatus.netStatusInfo[loopNum].ip, g_StrNetInfo[loopNum].ip, NET_ADDR_LENGTH - 1);
        NetStatus.netStatusInfo[loopNum].ip[NET_ADDR_LENGTH - 1] = '\0';

        // 复制 ethName
        strncpy(NetStatus.netStatusInfo[loopNum].ethName, g_StrNetInfo[loopNum].ethName, ETH_NAME_LENGTH - 1);
        NetStatus.netStatusInfo[loopNum].ethName[ETH_NAME_LENGTH - 1] = '\0';

        // 复制 priDns
        strncpy(NetStatus.netStatusInfo[loopNum].priDns, g_StrNetInfo[loopNum].priDns, MODEM_IPADDR_LEN - 1);
        NetStatus.netStatusInfo[loopNum].priDns[MODEM_IPADDR_LEN - 1] = '\0';

        
        // 复制 secDns
        strncpy(NetStatus.netStatusInfo[loopNum].secDns, g_StrNetInfo[loopNum].secDns, MODEM_IPADDR_LEN - 1);
        NetStatus.netStatusInfo[loopNum].secDns[MODEM_IPADDR_LEN - 1] = '\0';

        
        // 复制 gateway
        strncpy(NetStatus.netStatusInfo[loopNum].gateway, g_StrNetInfo[loopNum].gateway, MODEM_IPADDR_LEN - 1);
        NetStatus.netStatusInfo[loopNum].ethName[MODEM_IPADDR_LEN - 1] = '\0';

        LOG_Debug("Profile Index: %d, Dial Status: %d, IP: %s, EthName: %s, PriDns : %s, SecDns : %s, GateWay : %s",
               NetStatus.netStatusInfo[loopNum].profileIndex,
               NetStatus.netStatusInfo[loopNum].dialStatus,
               NetStatus.netStatusInfo[loopNum].ip,
               NetStatus.netStatusInfo[loopNum].ethName,
               NetStatus.netStatusInfo[loopNum].priDns,
               NetStatus.netStatusInfo[loopNum].secDns,
               NetStatus.netStatusInfo[loopNum].gateway);
    }  
    
    return NetStatus;
}


static int8_t NetInit(void)
{
    if (strcmp(g_dnsPathName, "NULL") == 0 || strlen(g_dnsPathName) == 0)
    {
        LOG_Debug("g_dnsPathName is NULL");
    }
    else
    {
        char cmd[256] = {0};
        snprintf(cmd, sizeof(cmd), "echo \"\" > %s", g_dnsPathName);
        system(cmd);
    }
    
    // system("iptables -t filter -F");
    // system("iptables -t nat -F");

    // 根据 g_ApnNum 的值添加相应数量的 MASQUERADE 规则
#if 0
    int8_t isFailed = 0;
    for (uint8_t i = 0; i < g_ApnNum; ++i)
    {
        char command[128] = {0};
        snprintf(command, sizeof(command), "iptables -t nat -A POSTROUTING -o %s -j MASQUERADE", g_StrNetInfo[i].ethName);
        int status = system(command);
        if (status == 0)
        {
            LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, command);
        }
        else
        {
            LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, command);
            isFailed = 1;
        }
    }

    if (isFailed == 1)
    {
        return RET_ERR;
    }
    else
    {
        return RET_OK;
    }
#else
    return RET_OK;
#endif
}

static void SetDnsAddr(uint8_t nameserverPri,const HalModemNetInfo_t *netInfo)
{
    if (strcmp(g_dnsPathName, "NULL") == 0 || strlen(g_dnsPathName) == 0)
    {
        LOG_Debug("g_dnsPathName is NULL");
        return;
    }

    char cmd[128] = {0};

    if ((strcmp("0.0.0.0", (char *)netInfo->priDns)) != 0)
    {
        if(0 == nameserverPri)
        {
            snprintf(cmd, sizeof(cmd), "sed -i '1inameserver %s' %s", netInfo->priDns, g_dnsPathName);
        }
        else
        {
            snprintf(cmd, sizeof(cmd), "echo 'nameserver %s' >> %s", netInfo->priDns, g_dnsPathName);
        }
        int status = system(cmd);
        if (status == 0)
        {
            LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, cmd);
        } else {
            LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, cmd);
        }

        memset(cmd, 0, 128);
        if(0 == nameserverPri)
        {
            snprintf(cmd, sizeof(cmd), "echo 'nameserver %s'> %s", netInfo->priDns, "/tmp/ql_lan_resolv.conf");
        }
        else
        {
            snprintf(cmd, sizeof(cmd), "echo 'nameserver %s' > %s", netInfo->priDns, "/tmp/ql_lan_resolv.conf");
        }
        status = system(cmd);
        if (status == 0)
        {
            LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, cmd);
        } else {
            LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, cmd);
        }
    }
    if ((strcmp("0.0.0.0", (char *)netInfo->secDns)) != 0)
    {
        memset(cmd, 0x00, sizeof(cmd));
        if(0 == nameserverPri)
        {
            snprintf(cmd, sizeof(cmd), "sed -i '1inameserver %s' %s", netInfo->secDns, g_dnsPathName);
        }
        else
        {
            snprintf(cmd, sizeof(cmd), "echo 'nameserver %s' >> %s", netInfo->secDns, g_dnsPathName);
        }
        int status = system(cmd);
        if (status == 0)
        {
            LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, cmd);
        } else {
            LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, cmd);
        }

        memset(cmd, 0, 128);
        if(0 == nameserverPri)
        {
            snprintf(cmd, sizeof(cmd), "echo 'nameserver %s'>> %s", netInfo->secDns, "/tmp/ql_lan_resolv.conf");
        }
        else
        {
            snprintf(cmd, sizeof(cmd), "echo 'nameserver %s' >> %s", netInfo->secDns, "/tmp/ql_lan_resolv.conf");
        }
        status = system(cmd);
        if (status == 0)
        {
            LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, cmd);
        } else {
            LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, cmd);
        }
        
        memset(cmd, 0, 128);
        snprintf(cmd, sizeof(cmd), "echo 'nameserver %s' >> %s", "223.5.5.5", "/tmp/ql_lan_resolv.conf");
        system(cmd);

        memset(cmd, 0, 128);
        snprintf(cmd, sizeof(cmd), "echo 'nameserver %s' >> %s", "114.114.114.114", "/tmp/ql_lan_resolv.conf");
        system(cmd);

        memset(cmd, 0, 128);
        snprintf(cmd, sizeof(cmd), "echo 'nameserver %s' >> %s", "180.76.76.76", "/tmp/ql_lan_resolv.conf");
        system(cmd);

    }
}

static void AddNameServerToRouteTable(uint8_t dnsRouteSet,const HalModemNetInfo_t *netInfo)
{
    char cmd[128] = {0x00};

    if(1 == dnsRouteSet)
    {
        if (strcmp("0.0.0.0", (char *)netInfo->priDns) != 0)
        {
            snprintf(cmd, sizeof(cmd), "route add %s %s", netInfo->priDns, netInfo->name);
            int status = system(cmd);
            if (status == 0)
            {
                LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, cmd);
            } else {
                LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, cmd);
            }
        }
        
        if (strcmp("0.0.0.0", (char *)netInfo->secDns) != 0)
        {
            memset(cmd, 0x00, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "route add %s %s", netInfo->secDns, netInfo->name);
            int status = system(cmd);
            if (status == 0)
            {
                LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, cmd);
            } else {
                LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, cmd);
            }
        }
    }
}

static int AddRouteIpToInterface(char *ip, char *dev)
{
    char *tmp1 = ip;
    char *tmp2 = NULL;
    int count = 0;
    char ipForSet[CONFIG_DOMAIN_LEN] = {0};
    char cmdSetRoute[128 + CONFIG_DOMAIN_LEN + ETH_NAME_LENGTH] = {0};
    int8_t isFailed = 0;

    if (ip == NULL || NULL == dev)
    {
        return RET_ERR;
    }

    while (NULL != strstr(tmp1, ";"))
    {
        tmp2 = strstr(tmp1, ";");
        if (tmp2 != NULL)
        {
            count = tmp2 - tmp1;
            memset(ipForSet, 0, sizeof(ipForSet));
            memcpy(ipForSet, tmp2 - count, count);
            LOG_Debug(" count %d ip : %s ", count, ipForSet);
            snprintf(cmdSetRoute ,sizeof(cmdSetRoute), "route add %s dev %s metric %u", ipForSet, dev,(uint32_t)NET_ROUTE_DEFAULT_PRIORITY);
            int status = system(cmdSetRoute);
            if (status == 0)
            {
                LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, cmdSetRoute);
            } else {
                LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, cmdSetRoute);
                isFailed = 1;
            }
            tmp1 = tmp2 + 1;
        }
    }

    if (isFailed == 1)
    {
        return RET_ERR;
    }
    else
    {
        return RET_OK;
    }
}
static int AddDomainIpToInterface(char *domain, char *dev)
{
    char *tmp1 = domain;
    char *tmp2 = NULL;
    int count = 0;
    char ipForSet[CONFIG_DOMAIN_LEN] = {0};
    // char cmdSetRoute[128 + NET_DOMAIN_LENGTH + ETH_NAME_LENGTH] = {0};
    int8_t isFailed = 0;

    if (domain == NULL || NULL == dev)
    {
        return RET_ERR;
    }

    while (NULL != strstr(tmp1, ";"))
    {
        tmp2 = strstr(tmp1, ";");
        if (tmp2 != NULL)
        {
            count = tmp2 - tmp1;
            memset(ipForSet, 0, sizeof(ipForSet));
            memcpy(ipForSet, tmp2 - count, count);
            LOG_Debug(" count %d domain : %s ", count, ipForSet);
            if ((getIPFromDomain(ipForSet, dev, (uint32_t)NET_ROUTE_DEFAULT_PRIORITY)) == RET_ERR)
            {
                LOG_Error("getIPFromDomain is err.");
                isFailed = 1;
                tmp1 = tmp2 + 1;
                continue;
            }
            tmp1 = tmp2 + 1;
        }
    }

    if (isFailed == 1)
    {
        return RET_ERR;
    }
    else
    {
        return RET_OK;
    }
}

static void *DialCBThread(void *dialCBInfo)
{
    sleep_clock(3);
    DialCB_t *tmp = (DialCB_t *)dialCBInfo;
    HalModemNetInfo_t *netInfo = &tmp->netInfo;
    int32_t callId = tmp->callId;
    uint8_t netInfoIndex = 0xFF;

    LOG_Debug("%s : [net] profile id %d",__FUNCTION__, callId);
    // if(callId < 0 || callId > g_ApnNum)
    // {
    //     LOG_Error("%s : wrong callId value : %d",__FUNCTION__, callId);
    //     free(dialCBInfo);
    //     return NULL;
    // }

    for(uint8_t i = 0; i < g_ApnNum; i++)
    {
        if(g_StrNetInfo[i].profileIndex == callId)
        {
            netInfoIndex = i;
            break;
        }
    }

    if (netInfoIndex >= g_ApnNum)
    {
        LOG_Error("%s : wrong netInfoIndex value : %d",__FUNCTION__, netInfoIndex);
        free(dialCBInfo);
        dialCBInfo = NULL;
        return NULL;
    }

    // 检查是否是有效的拨号状态
    if (netInfo->dialStatus == 1)
    {
        LOG_Debug("[net] profile id %d, netInfoIndex:%d", callId, netInfoIndex);
        LOG_Debug("[net] [%d]eth name:            %s", callId, netInfo->name);
        LOG_Debug("[net] [%d]IP address:          %s", callId, netInfo->ip);
        LOG_Debug("[net] [%d]Gateway address:     %s", callId, netInfo->gateway);
        LOG_Debug("[net] [%d]Primary DNS address: %s", callId, netInfo->priDns);
        LOG_Debug("[net] [%d]Second DNS address:  %s", callId, netInfo->secDns);
        
        // 获取IP地址和网关到g_StrNetInfo
        strncpy(g_StrNetInfo[netInfoIndex].ip, (char *)netInfo->ip, MODEM_IPADDR_LEN);
        g_StrNetInfo[netInfoIndex].ip[MODEM_IPADDR_LEN - 1] = '\0';
        strncpy(g_StrNetInfo[netInfoIndex].gateway, (char *)netInfo->gateway, MODEM_IPADDR_LEN);
        g_StrNetInfo[netInfoIndex].gateway[MODEM_IPADDR_LEN - 1] = '\0';
        strncpy(g_StrNetInfo[netInfoIndex].ethName, (char *)netInfo->name, ETH_NAME_LENGTH);
        g_StrNetInfo[netInfoIndex].ethName[ETH_NAME_LENGTH - 1] = '\0';
        strncpy(g_StrNetInfo[netInfoIndex].priDns, (char *)netInfo->priDns, MODEM_IPADDR_LEN);
        g_StrNetInfo[netInfoIndex].priDns[MODEM_IPADDR_LEN - 1] = '\0';
        strncpy(g_StrNetInfo[netInfoIndex].secDns, (char *)netInfo->secDns, MODEM_IPADDR_LEN);
        g_StrNetInfo[netInfoIndex].secDns[MODEM_IPADDR_LEN - 1] = '\0';
        g_StrNetInfo[netInfoIndex].dialTimes = ((g_StrNetInfo[netInfoIndex].dialTimes + 1) % 65535);

        // 添加DNS到/etc/resolv.conf
        SetDnsAddr(g_StrNetInfo[netInfoIndex].nameserverPri,netInfo);

        // 添加DNS到路由表
        AddNameServerToRouteTable(g_StrNetInfo[netInfoIndex].dnsRouteSet,netInfo);

        // 添加路由IP到网卡
        AddRouteIpToInterface(g_StrNetInfo[netInfoIndex].privateRoute, g_StrNetInfo[netInfoIndex].ethName);

        // 添加域名IP到网卡
        AddDomainIpToInterface(g_StrNetInfo[netInfoIndex].domain, g_StrNetInfo[netInfoIndex].ethName);

        
        //0718FXL同步QOS相关
        LOG_Debug("[net] IP address:  %s", netInfo->ip);
        char cmd[256];
        if(0 == access("/oemdata/qoos", F_OK))
        {
            snprintf(cmd, sizeof(cmd), "sh /oemapp/scripts/iptables.sh rmnet_data0 %s", netInfo->ip);
            GetSystemRetValue(cmd);
        }
        

        // 设置当前Apn拨号状态为连接状态
        g_StrNetInfo[netInfoIndex].dialStatus = 1;

        LOG_Info("apn%d dail is success,dialStatus is connect.",(netInfoIndex));
    }
    else
    {
        // 设置当前Apn拨号状态为断开状态
        g_StrNetInfo[netInfoIndex].dialStatus = 0;
        g_StrNetInfo[netInfoIndex].dialTimes = ((g_StrNetInfo[netInfoIndex].dialTimes + 1) % 65535);
        LOG_Info("apn%d dail is failed,dialStatus is disconnect.",(netInfoIndex));
    }
    NetIpcSendNetState();
    free(dialCBInfo);
    dialCBInfo = NULL;
    return NULL;
}

static void DataCallStateCallback(int32_t callId, const HalModemNetInfo_t *netInfo)
{
    LOG_Debug("[net] profile id %d\n", callId);
    DialCB_t *dialCBInfo = malloc(sizeof(DialCB_t));
    dialCBInfo->callId = callId;
    memmove(&dialCBInfo->netInfo, netInfo, sizeof(HalModemNetInfo_t));

    pthread_t threadId = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&threadId, &attr, DialCBThread, (void *)dialCBInfo);
}

// 拨号初始化（只需执行一次）
static int32_t DialInit(void)
{
    HalModemDialParam_t dialParam = {0};
    dialParam.apnNum = g_ApnNum;

    for (uint8_t i = 0; i < g_ApnNum; ++i)
    {
        dialParam.apnInfo[i].callId = g_StrNetInfo[i].profileIndex;
        strncpy((char *)dialParam.apnInfo[i].apnName, g_StrNetInfo[i].apnName, MODEM_TEXT_LEN - 1);
        strncpy((char *)dialParam.apnInfo[i].userName, g_StrNetInfo[i].usrName, MODEM_TEXT_LEN - 1);
        strncpy((char *)dialParam.apnInfo[i].userPasswd, g_StrNetInfo[i].password, MODEM_TEXT_LEN - 1);
        dialParam.apnInfo[i].authType = g_StrNetInfo[i].authProto;
    }

    // 初始化拨号模块
    if (HalModemDialInit() != RET_OK)
    {
        LOG_Error("%s : HalModemDialInit is err.",__FUNCTION__);
        return RET_ERR;
    }

    // 设置拨号参数
    if (HalModemDialSetParam(&dialParam) != RET_OK)
    {
        LOG_Error("%s : HalModemDialSetParam is err.",__FUNCTION__);
        HalModemDialDeinit();
        return RET_ERR;
    }

    // 设置拨号状态回调
    if (HalModemDialSetStatusCallback(DataCallStateCallback) != RET_OK) {
        LOG_Error("%s : HalModemDialSetStatusCallback is err.",__FUNCTION__);
        HalModemDialDeinit();
        return RET_ERR;
    }

    return RET_OK;
}

// 拨号任务线程函数
static gpointer DialTaskThread(gpointer arg)
{
    LOG_Info("[net] DialTaskThread started, waiting for network registration...");

    // 步骤1: 等待注网成功
    while (1)
    {
        // 检查注网状态
        if (g_ModemSrvNotify.dataRegStatus == E_MODEM_SRV_NW_REG_STATUS_REGISTERED)
        {
            LOG_Info("[net] Network registered successfully, dataRegStatus=%d, csq=%d",
                     g_ModemSrvNotify.dataRegStatus, g_ModemSrvNotify.csq);
            break;
        }

        LOG_Debug("[net] Waiting for network registration, dataRegStatus=%d, csq=%d",
                  g_ModemSrvNotify.dataRegStatus, g_ModemSrvNotify.csq);
        sleep(5);
    }

    // 步骤2: 初始化拨号（失败则重试）
    while (1)
    {
        LOG_Info("[net] Initializing dial...");
        int32_t ret = DialInit();

        if (ret == RET_OK)
        {
            LOG_Info("[net] DialInit completed successfully");
            break;
        }
        else
        {
            LOG_Error("[net] DialInit failed, ret=%d, retrying in 5 seconds...", ret);
            sleep(5);
        }
    }

    // 步骤3: 循环拨号，每路独立重试直到所有DialStart接口调用成功
    // 使用结构体记录每路拨号的profileIndex和成功状态
    typedef struct {
        uint8_t profileIndex;  // 对应的profileIndex(callId)
        uint8_t success;       // 拨号成功标志，0=未成功，1=已成功
    } DialStartStatus_t;

    DialStartStatus_t dialStartSuccess[NETWORK_APN_NUM] = {0};
    uint8_t allSuccess = 0;

    // 初始化：记录每路APN的profileIndex
    for (uint8_t i = 0; i < g_ApnNum; i++)
    {
        dialStartSuccess[i].profileIndex = g_StrNetInfo[i].profileIndex;
        dialStartSuccess[i].success = 0;
    }

    while (allSuccess != 1)
    {
        LOG_Info("[net] Starting dial round, g_ApnNum=%d", g_ApnNum);

        // 遍历所有APN，对未成功的调用DialStart接口
        for (uint8_t i = 0; i < g_ApnNum; i++)
        {
            // 获取这一路的callId (profileIndex)
            int32_t callId = dialStartSuccess[i].profileIndex;

            // 边界检查：防止无效callId
            if (callId <= 0)
            {
                LOG_Error("[net] Invalid callId=%d for APN[%d], skip", callId, i);
                continue;
            }

            // 如果这一路DialStart接口已经调用成功，跳过
            if (dialStartSuccess[i].success == 1)
            {
                LOG_Debug("[net] APN[%d] callId=%d DialStart already succeeded, skip",
                         i, callId);
                continue;
            }

            LOG_Info("[net] Calling DialStart for APN[%d] callId=%d, apnName=%s",
                     i, callId, g_StrNetInfo[i].apnName);

            // 调用DialStart接口
            int32_t ret = HalModemDialStartEx(callId); 
            if (ret == RET_OK)
            {
                LOG_Info("[net] APN[%d] callId=%d DialStart interface call succeeded", i, callId);
                dialStartSuccess[i].success = 1; // 标记成功
            }
            else
            {
                LOG_Error("[net] APN[%d] callId=%d DialStart interface call failed, ret=%d, stopping this route",
                         i, callId, ret);
                // DialStart接口调用失败，停止这一路
                HalModemDialStopEx(callId);
            }
        }

        
        // 检查是否所有APN的DialStart接口都调用成功
        allSuccess = 1;
        for (uint8_t i = 0; i < g_ApnNum; i++)
        {
            if (dialStartSuccess[i].success != 1)
            {
                allSuccess = 0;
                LOG_Info("[net] APN[%d] callId=%d DialStart interface not yet succeeded, will retry next round",
                        i, dialStartSuccess[i].profileIndex);
            }
        }

        // 如果还有失败的，等待5秒后继续下一轮
        if (allSuccess != 1)
        {
            LOG_Info("[net] Not all DialStart calls succeeded, waiting 5 seconds before next round...");
            sleep(5);
        }
    }

    LOG_Info("[net] All APNs dial succeeded, DialTaskThread completed successfully");
    return NULL;
}

// 创建拨号任务线程的辅助函数
static int32_t CreateDialTaskThread(void)
{
    GThread *thread = g_thread_new("DialTask", DialTaskThread, NULL);
    if (thread == NULL)
    {
        LOG_Error("[net] Failed to create dial task thread");
        return RET_ERR;
    }

    // 分离线程，使其在完成后自动清理资源
    g_thread_unref(thread);
    LOG_Info("[net] Dial task thread created successfully");
    return RET_OK;
}

static int32_t NetDataCall(void)
{
    ParamInfo_t paramInfo = {0};

    // 设置参数ID
    paramInfo.id = 40;

    // 判断是否需要自动拨号
    if (g_AutoDail)
    {
        // 创建拨号任务线程
        return CreateDialTaskThread();
    }
    else
    {
        while (1)
        {
            // 获取允许拨号参数,参数管理实现
            int32_t ret = GetParam(&paramInfo);
            if (ret == RET_OK)
            {
                if (1 == atoi((char *)paramInfo.value))
                {
                    // 创建拨号任务线程
                    return CreateDialTaskThread();
                }
            }
            sleep(5);
        }
    }
}

// 检查指定 DNS 服务器的可达性
static int check_dns_server(const char *dns_ip) 
{
    char cmd[256] = {0};
    
    if (!dns_ip || strlen(dns_ip) == 0) 
    {
        snprintf(cmd, sizeof(cmd), PING_CMD "47.97.113.255 > /dev/null 2>&1");
    }
    else
    {
        snprintf(cmd, sizeof(cmd), PING_CMD "%s > /dev/null 2>&1", dns_ip);
    }
    int status = system(cmd);
    LOG_Debug("%s : %s : ping result - [status:%d].",__FUNCTION__,cmd,status);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
        LOG_Debug("%s : is success",__FUNCTION__);
        return 1; // ping 成功
    }
    
    LOG_Debug("%s : is FAILED",__FUNCTION__);
    return 0; // ping 失败
}

static int CheckDialStatus(void)
{
    for (int i = 0; i < g_ApnNum; i++)
    {
        if (g_StrNetInfo[i].dialStatus != 1)
        {
            return 0;
        }
    }
    return 1;
}

static int manage_ping(time_t *nextPingTime, time_t currentTime)
{
    int ret = 0;

    if (nextPingTime != NULL)
    {
        if (*nextPingTime != 0 && currentTime >= *nextPingTime)
        {
            LOG_Debug("%s : execute ping .", __FUNCTION__);
            
            int32_t pingResult = IcmpPing("8.8.8.8", 1, NULL);
            if (pingResult == 0)
            {
                LOG_Debug("%s :  PING succeeded,net connect.", __FUNCTION__);
            }
            else
            {
                LOG_Debug("%s :  PING failed,net disconnect.", __FUNCTION__);
                ret = -1;
            }
            *nextPingTime = currentTime + 120;
        }
        else if (*nextPingTime == 0)
        {
            *nextPingTime = currentTime + 120;
        }
    }

    return ret;
}

static int32_t CheckNetworkConnectivityWithRetry()
{
    static uint8_t flightModeSwitchCount = 0;      // 记录切换飞行模式次数
    static time_t lastDialFailTime = 0;            // 记录上次拨号失败的时间戳
    static time_t nextPingTime = 0;
    
    int32_t ret = CheckDialStatus();                // 检查拨号状态
    time_t currentTime = time(NULL);                // 获取当前时间

    if (ret == 0) // 拨号失败
    {
        LOG_Debug("%s : Dialing failed, checking retry conditions.",__FUNCTION__);

        // 如果是第一次拨号失败，记录时间
        if (lastDialFailTime == 0)
        {
            lastDialFailTime = currentTime;
        }

        // 计算拨号失败持续时间，超过60秒（1分钟）则切换飞行模式
        if (currentTime - lastDialFailTime >= 30) 
        {
            LOG_Debug("%s : Dialing failed for more than 1 minute, switching flight mode.",__FUNCTION__);

            // 切换飞行模式
            ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_ON);
            
            sleep_clock(2);
            
            ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_OFF);

            flightModeSwitchCount++;

            // 如果切换飞行模式超过10次，重启模块
            if (flightModeSwitchCount > 1000)
            {
                LOG_Debug("%s : Flight mode switch count exceeded 10 times, restarting module.",__FUNCTION__);
                // 重新启动模块
                
                sync();                
                if (reboot(RB_AUTOBOOT) == -1) 
                {
                    LOG_Error("%s : reboot failed.",__FUNCTION__);
                }
                
                flightModeSwitchCount = 0;  // 重置切换次数

            }

            
            lastDialFailTime = currentTime; // 切换飞行模式后重置拨号失败时间
            nextPingTime = 0; // 拨号失败后重置 ping 时间
        }
    }
    else if (ret == 1) // 拨号成功
    {
        LOG_Debug("%s : Dialing succeeded, start pinging 8.8.8.8.",__FUNCTION__);

        // 启动ping 8.8.8.8，每2分钟一次
        manage_ping(&nextPingTime, currentTime);

        // 拨号成功后，重置飞行模式切换次数和拨号失败时间
        flightModeSwitchCount = 0;
        lastDialFailTime = 0;
    }

    return ret; // 返回拨号状态
}

gpointer NetMonitor(gpointer pArg)
{
    int32_t ret = 0;
    uint8_t csq = 0;
    uint16_t noRegWithSignalCount = 0;

    //等待系统启动
    sleep_clock(60);
    
    while (1)
    {
        sleep_clock(5);
        if (PM_STATE_SLEEP == g_NetPmState.PmState)
        {
            LOG_Debug("net is sleep,so not check modem status.");
            continue;
        }
        
        LOG_Debug("net dataRegStatus : %d",g_ModemSrvNotify.dataRegStatus);
        if(E_MODEM_SRV_NW_REG_STATUS_UNREGISTERED == g_ModemSrvNotify.dataRegStatus || (E_MODEM_SRV_NW_REG_STATUS_REGISTERED == g_ModemSrvNotify.dataRegStatus && g_ModemSrvNotify.csq == 99))
        {
            noRegWithSignalCount++;          
        }
        else if (E_MODEM_SRV_NW_REG_STATUS_REGISTERED == g_ModemSrvNotify.dataRegStatus && g_ModemSrvNotify.csq != 99)
        {
            noRegWithSignalCount = 0;
            CheckNetworkConnectivityWithRetry();
            continue;
        }

        LOG_Debug("noRegWithSignalCount : %d", noRegWithSignalCount);
        if (noRegWithSignalCount > 6)
        {
            csq = g_ModemSrvNotify.csq;
            LOG_Debug("net csq : %d",csq);
        
            if(csq > 9)
            {
                ret = ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_ON);
                if (RET_ERR == ret)
                {
                    LOG_Debug("ModemSrvSetAirplaneMode : E_MODEM_SRV_DMS_AIRPLANE_ON is failed. ");
                }   
                
                sleep_clock(2);

                ret = ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_OFF);
                if (RET_ERR == ret)
                {
                    LOG_Debug("ModemSrvSetAirplaneMode : E_MODEM_SRV_DMS_AIRPLANE_OFF is failed. ");
                }

                noRegWithSignalCount = 0;
            }
        }
    }
}

int32_t SendMcuNetStatusSync(void)
{
    int32_t retValue = RET_OK;
    uint8_t payLoad[NET_STATUS_LENGTH] = {0};
    McuMesg_t McuMsg = {0};
    uint8_t apnStatus = 0;
    uint8_t antStatus = 0;
    ModemSrvExtSigInfo_t modemSrvExtSigInfo = {0};

    if (PM_STATE_SLEEP == g_NetPmState.PmState)
    {
        LOG_Debug("SendMcuNetStatusSync: PM_STATE_SLEEP");
        return RET_OK;
    }
    ModemSrvGetExtSignalInfo(&modemSrvExtSigInfo);
    for (int i = 0; i < NETWORK_APN_NUM; i++)
    {
        if (g_StrNetInfo[i].dialStatus == 1)
        {
            apnStatus |= (1 << i);
        }
    }
    antStatus = (g_ModemSrvNotify.antennaStatus | (g_ModemSrvNotify.secondaryAntennaStatus << 4));

    McuMsg.type = SYS_QUE_TYPE_MCU_SYNC_NETWORK_STATUS;
    McuMsg.payloadLen = NET_STATUS_LENGTH;
    payLoad[0] = g_ModemSrvNotify.dataRegStatus;//驻网状态
    payLoad[1] = g_ModemSrvNotify.csq;//信号质量
    //RSSI
    payLoad[2] = (modemSrvExtSigInfo.lteSiginfo.rssi >> 8) & 0xFF;
    payLoad[3] = modemSrvExtSigInfo.lteSiginfo.rssi & 0xFF;
    //RSRP
    payLoad[4] = (modemSrvExtSigInfo.lteSiginfo.rsrp >> 8) & 0xFF;
    payLoad[5] = modemSrvExtSigInfo.lteSiginfo.rsrp & 0xFF;
    //RSRQ
    payLoad[6] = (modemSrvExtSigInfo.lteSiginfo.rsrq >> 8) & 0xFF;
    payLoad[7] = modemSrvExtSigInfo.lteSiginfo.rsrq & 0xFF;
    //SNR
    payLoad[8] = (modemSrvExtSigInfo.lteSiginfo.sinr >> 8) & 0xFF;
    payLoad[9] = modemSrvExtSigInfo.lteSiginfo.sinr & 0xFF;
    //netType
    payLoad[10] = modemSrvExtSigInfo.lteSiginfo.sigType & 0xFF;
    //apnCounter
    payLoad[11] = g_ApnNum;
    //apnstatus
    payLoad[12] = apnStatus;
    //SIMCARD
    payLoad[13] = g_ModemSrvNotify.simCardStatus;
    //modemstatus
    payLoad[14] = antStatus;

    //USBstatus
    //payLoad[15] = g_usbSta;
    payLoad[15] = 0x00;
    //phystatus
    //payLoad[16] = g_phySta;
    payLoad[16] = 0x00;
    McuMsg.payload = payLoad;
    LOG_DebugHex(payLoad , 17 , "network");
    if (RET_ERR == McuSrvSendMesg(&McuMsg))
    {
        retValue = RET_ERR;
        LOG_Error(" %s:McuSrvSendMesg faild!", __func__);
    }
    // LOG_Debug("%s - Creg:%d,Csq:%d,netType:%d,ApnCounter:%d,ApnStatus:%d,antStatus:%d",
    // __func__,payLoad[0],payLoad[1],payLoad[2],payLoad[3],payLoad[4],payLoad[5]);
    return retValue;
}

gpointer NetStatusSyncMcu(gpointer pArg)
{
    while (1)
    {
        SendMcuNetStatusSync();
        usleep(1000000);
    }
}

static void DeleteDefaultRoute()
{
    char command[256] = {0};
    snprintf(command, sizeof(command), "route del default");

    FILE* fp = popen(command, "r");
    if (fp == NULL)
    {
        LOG_Error("popen failed while deleting default route");
        return;
    }

    pclose(fp);
    LOG_Info("Deleted old default route success.");
}

static void SetDefaultRoute(const char* gateway, const char* interface)
{
    
    if(NULL == gateway || NULL == interface)
    {
        LOG_Error("%s : paramater is NULL.",__FUNCTION__);
        return;
    }
    
    DeleteDefaultRoute();

    char command[256] = {0};
    snprintf(command, sizeof(command), "route add default gw %s dev %s", gateway, interface);

    FILE* fp = popen(command, "r");
    if (fp == NULL)
    {
        LOG_Error("popen failed while adding new default route");
        return;
    }

    pclose(fp);
    LOG_Info("Default route set via gateway: %s on interface: %s", gateway, interface);
}
static int32_t CheckNetworkConnectivity(const char* ip, const char* interface)
{
    char command[256] = {0};
    
    if(NULL == ip || NULL == interface)
    {
        LOG_Error("%s : paramater is NULL.",__FUNCTION__);
        return 0;
    }
    
    snprintf(command, sizeof(command), "ping -I %s -c 1 -W 1 %s", interface, ip);
    FILE* fp = popen(command, "r");
    if (fp == NULL)
    {
        LOG_Error("popen failed");
        return 0;
    }

    char result[1024] = {0};
    int success = 0;
    while (fgets(result, sizeof(result), fp) != NULL)
    {
        if (strstr(result, "1 received") != NULL)
        {
            success = 1;
            LOG_Info("%s : %s is connect.",__FUNCTION__,interface);
            break;
        }
    }

    pclose(fp);
    return success;
}

static void PollApnStatus()
{
    int currentDefaultApn = -1;
    int backupApn = -1;

    for (int i = 0; i < g_ApnNum; i++)
    {
        if (strstr(g_StrNetInfo[i].privateRoute, "default") != NULL)
        {
            currentDefaultApn = i;
            break;
        }
    }

    if (currentDefaultApn == -1)
    {
        LOG_Error("No default APN found.");
        return;
    }


    if (CheckNetworkConnectivity(g_StrNetInfo[currentDefaultApn].priDns, g_StrNetInfo[currentDefaultApn].ethName))
    {
        if (1 == g_PreviousApnSwitched)
        {
            LOG_Info("Restoring original default APN: %s", g_StrNetInfo[currentDefaultApn].apnName);
            SetDefaultRoute(g_StrNetInfo[currentDefaultApn].gateway, g_StrNetInfo[currentDefaultApn].ethName);
            g_PreviousApnSwitched = 0;
            g_CurrentApnIndex = currentDefaultApn;
        }
        else
        {
            LOG_Debug("Default APN is still connected, no action needed.");
            return;
        }
    }
    else
    {
        if (1 == g_PreviousApnSwitched && g_CurrentApnIndex != -1)
        {
            if (CheckNetworkConnectivity(g_StrNetInfo[g_CurrentApnIndex].priDns, g_StrNetInfo[g_CurrentApnIndex].ethName))
            {
                LOG_Debug("Current switched APN is still connected, no need to switch.");
                return;
            }
        }

        for (int i = 0; i < g_ApnNum; i++)
        {
            if (i != currentDefaultApn && CheckNetworkConnectivity(g_StrNetInfo[i].priDns, g_StrNetInfo[i].ethName))
            {
                if (i == g_CurrentApnIndex)
                {
                    LOG_Debug("Backup APN is the same as the currently switched APN. No need to switch.");
                }
                else
                {
                    backupApn = i;
                    break;
                }
            }
        }

        if (backupApn != -1)
        {
            LOG_Info("Switching to backup APN: %s", g_StrNetInfo[backupApn].apnName);
            SetDefaultRoute(g_StrNetInfo[backupApn].gateway, g_StrNetInfo[backupApn].ethName);
            g_PreviousApnSwitched = 1;
            g_CurrentApnIndex = backupApn;
        }
        else
        {
            LOG_Error("No APNs are currently available.");
        }
    }
}

gpointer DnsCheckMonitor(gpointer pArg)
{
    usleep(60000000);
    while (1)
    {
        PollApnStatus();
        sleep(10);
    }
}

void NetModemSrvInfoCallback(const uint8_t *data, const uint16_t len, void *userdata)
{
    if (sizeof(ModemSrvNotify_t) == len && NULL != data)
    {
        memcpy(&g_ModemSrvNotify, data, len);
    }
    else
    {
        LOG_Error("%s recv modem data leng is wrong.",__FUNCTION__);
    }

    return;
}

void StartNetService(void)
{
    // 加载配置
    LoadNetConfig(g_StrNetInfo, &g_ApnNum, &g_AutoDail, g_dnsPathName, DNS_PATH_NAME_MAX_LEN);

    if(0 != access("/dev/mmcblk0", F_OK)) {
        g_modemId = 2;
        LOG_Debug("g_modemId = %d", g_modemId);
    } else {
        g_modemId = 1;
        LOG_Debug("g_modemId = %d", g_modemId);
    }

    if(g_modemId == 2)
    {
        strcpy(g_StrNetInfo[0].apnName, "ctnet");
    }

    // 初始化网络配置
    NetInit();

    // 发起拨号逻辑
    NetDataCall();
}

/**
 * @name NetPmChangeCallback
 * @brief 从给定数据中获取PM电源状态
 * @param data 指向包含PM电源状态数据的缓冲区的指针。
 * @param len 数据缓冲区的长度。
 * @param userdata 用户自定义数据的指针，本函数中未使用。
 * @return 无返回值
 */
void NetPmChangeCallback(const uint8_t *data, const uint16_t len, void *userdata)
{
    if (sizeof(MPUPmStateChange_t) == len && NULL != data)
    {
        memcpy(&g_NetPmState, data, len);
    }
    else
    {
        LOG_Debug("%s len error!",__FUNCTION__);
    }
}

void GetModemSrvInfo(ModemSrvNotify_t *modemSrvNotify)
{
    memcpy(modemSrvNotify, &g_ModemSrvNotify, sizeof(ModemSrvNotify_t));
}

static int disable_vlan(void) 
{
    int ret = 0;

    if(g_VlanSwitch == UP)
    {
        ret |= system("ifconfig bridge1 down");
        //ret |= system("ip addr del 192.168.1.1/24 dev bridge1");
        if (ret == 0)
        {
            LOG_Debug("%s : bridge1 disabled",__FUNCTION__);
        } 
        else 
        {
            LOG_Debug("%s : Error bridge1 disabled",__FUNCTION__);
        }
        
        g_VlanSwitch = DOWN;
    }
    else
    {
        LOG_Debug("%s : info bridge1  already disabled",__FUNCTION__);
    }
    
    return ret;
}

static int enable_vlan(void) 
{
    int ret = 0;

    
    if(g_VlanSwitch == DOWN)
    {
        ret |= system("ifconfig bridge1 up");
        if (ret == 0) 
        {
            LOG_Debug("%s : bridge1 enabled",__FUNCTION__);
        } 
        else
        {
            LOG_Debug("%s : Error bridge1enabled",__FUNCTION__);
        } 
        
        g_VlanSwitch = UP;
    }
    else
    {
        LOG_Debug("%s : info bridge1 already enable",__FUNCTION__);
    }
    
    return ret;
}

void NetGetMcuChangeModemCmd(const uint8_t *data, const uint16_t len, void *userdata)
{
	uint8_t mcuBuf[2] = {0};

    if(sizeof(mcuBuf) != len)
    {
        LOG_Error("%s recv mcu data leng is wrong.",__FUNCTION__);
        return;
    }
    else
    {
        memcpy(mcuBuf,data,len);
    }
    
    LOG_Debug("%s - modemID: %d,isAlive : %d",__FUNCTION__,mcuBuf[0],mcuBuf[1]);

    if(g_modemId == 1) //主模组
    {
        //主模组不做处理，mcu应该也不会转发，转发也不做处理
    }
    else if(g_modemId == 2) //副模组
    {
        //副模组处理切换逻辑
        
        if(mcuBuf[0] == 1 && mcuBuf[1] == 1) //模组1网络可达，关闭模组2的vlan
        {
            //关闭模组2的vlan
            disable_vlan();
        }
        else if (mcuBuf[0] == 1 && mcuBuf[1] == 2)//模组1网络不可达，开启模组2的vlan
        {
            //开始模组2的vlan
            enable_vlan();
        }
        else
        {
            //模组2暂时不会收到自己的状态，根据模组1的状态来切换模组2的状态
        }
    }

    //回复mcu，模组2下行不通，暂时不回复.
   
    
}
gpointer ModulePeriodicNetworkCheck(gpointer pArg)
{
    while(1)
    {
        g_PingResult = IcmpPing("8.8.8.8", 1 ,NULL);
        if (g_PingResult == 0)
        {
            LOG_Debug("%s :  PING succeeded,net connect.", __FUNCTION__);
        }
        else
        {
            LOG_Debug("%s :  PING failed,net disconnect.", __FUNCTION__);
        }
        
        sleep_clock(1);
    }
}

int is_mounted(const char *path) {
    // 打开 /proc/mounts 文件
    FILE *fp = fopen("/proc/mounts", "r");
    if (fp == NULL) {
        perror("无法打开 /proc/mounts 文件");
        return -1;
    }

    // 缓冲区用于存储每一行的内容
    char line[1024];
    int found = 0;

    // 按行读取 /proc/mounts 文件内容
    while (fgets(line, sizeof(line), fp)) {
        // 查找挂载点字段（通常是第2个字段）
        char device[1024], mount_point[1024], fs_type[1024], options[1024];
        unsigned long long offset;
        int ret = sscanf(line, "%s %s %s %s %*d %*d", device, mount_point, fs_type, options);
        if (ret != 4) {
            // 如果解析失败，跳过此行
            continue;
        }
        
        // 比较挂载点是否匹配
        if (strcmp(mount_point, path) == 0) {
            found = 1;
            break;
        }
    }

    fclose(fp);
    return found;
}

/**
 * @name GetSystemRetValue
 * @brief 获取系统命令执行的返回值
 *        本函数通过调用system()函数执行指定的命令，并返回该命令的执行状态。
 * @param cmd 指向要执行的命令字符串的指针。
 * @return 如果执行成功，返回true；如果执行失败，返回false。
 */
gboolean GetSystemRetValue(const char *cmd)
{
	pid_t status;
	
	if(NULL == cmd)
	{
		return false;
	}
	LOG_Debug("%s exe start", cmd);
	status = system((char *)cmd);
	LOG_Debug("%s exe end:%d", cmd, status);
	if(-1 == status)
	{
        
		return false;
	}
	
	if(!WIFEXITED(status))
	{
		return false;
	}
	
	return true;
}


gpointer SyncModemAliveStatus(gpointer pArg)
{    
    //sleep_clock(60);
    if(0 != access("/dev/mmcblk0", F_OK)) {
        g_modemId = 2;
        LOG_Debug("g_modemId = %d", g_modemId);
    } else {
        g_modemId = 1;
        LOG_Debug("g_modemId = %d", g_modemId);
    }

    // do
    // {
    //     sleep(1);
    //     LOG_Debug("g_modemId = %d", g_modemId);
    // } while (0 == g_StrNetInfo[0].dialStatus);
    
    sleep(3);

    if(g_modemId == 1)
    {
        // // GetSystemRetValue("sh /oemapp/scripts/vlan_set_modem1.sh 1 192.168.201.1 192.168.201.2");
        // // GetSystemRetValue("sh /oemapp/scripts/vlan_set_modem1.sh 1 192.168.205.10 192.168.205.15");
        // GetSystemRetValue("vconfig add eth0 201");
        // sleep(1);
        // // GetSystemRetValue("brctl delif bridge0 eth0");
        // GetSystemRetValue("ifconfig eth0.201 192.168.201.1 netmask 255.255.255.0 broadcast 192.168.201.255 up");
        // sleep(1);
        // GetSystemRetValue("vconfig add eth0 205");
        // sleep(1);
        // // GetSystemRetValue("brctl delif bridge0 eth0");
        // GetSystemRetValue("ifconfig eth0.205 192.168.205.10 netmask 255.255.255.0 broadcast 192.168.205.255 up");
        // sleep(1);
        // GetSystemRetValue("ip link add name bridge1 type bridge");
        // sleep(1);
        // GetSystemRetValue("ip link add name bridge2 type bridge");
        // sleep(1);
        // // GetSystemRetValue("ip link add name bridge3 type bridge");
        // // sleep(1);
        // GetSystemRetValue("ifconfig bridge1 192.168.1.201 netmask 255.255.255.255 broadcast 192.168.1.255 up");
        // sleep(1);
        // GetSystemRetValue("ip link set eth0 master bridge2");
        // sleep(1);
        // GetSystemRetValue("ifconfig bridge2 192.168.1.88 netmask 255.255.255.0 broadcast 192.168.1.255 up");
        // // GetSystemRetValue("ifconfig bridge1 192.168.1.1 netmask 255.255.255.0 broadcast 192.168.1.255 up");
        // //GetSystemRetValue("sh /oemapp/scripts/vlan_set_modem1.sh 1 192.168.1.1 192.168.1.4");
        // sleep(1);
        // GetSystemRetValue("ifconfig eth0 down");
        // sleep(1);
        // GetSystemRetValue("ifconfig eth0 up");
        // sleep(1);
        // system("touch /oemdata/log/udhcpd.leases;udhcpd -f /oemapp/config/udhcpd_bridge2.conf &");
        // sleep(1);
        // GetSystemRetValue("/oemapp/scripts/monitor_dns_1.sh");
    }
    else if(g_modemId == 2)
    {
        // // GetSystemRetValue("sh /oemapp/scripts/vlan_set_modem2.sh 1 192.168.202.1 192.168.202.2");
        // // GetSystemRetValue("sh /oemapp/scripts/vlan_set_modem2.sh 1 192.168.205.15 192.168.202.10");
        // GetSystemRetValue("vconfig add eth0 202");
        // sleep(1);
        // // GetSystemRetValue("brctl delif bridge0 eth0");
        // GetSystemRetValue("ifconfig eth0.202 192.168.202.1 netmask 255.255.255.0 broadcast 192.168.202.255 up");
        // sleep(1);
        // GetSystemRetValue("vconfig add eth0 205");
        // sleep(1);
        // // GetSystemRetValue("brctl delif bridge0 eth0");
        // GetSystemRetValue("ifconfig eth0.205 192.168.205.15 netmask 255.255.255.0 broadcast 192.168.205.255 up");
        // sleep(1);
        // GetSystemRetValue("ip link add name bridge1 type bridge");
        // sleep(1);
        // GetSystemRetValue("ifconfig bridge1 192.168.1.202 netmask 255.255.255.255 broadcast 192.168.1.255 up");
        // sleep(1);
        // GetSystemRetValue("ip link add name bridge2 type bridge");
        // sleep(1);
        // GetSystemRetValue("ip link set eth0 master bridge2");
        // sleep(1);
        // GetSystemRetValue("ifconfig bridge2 192.168.1.86 netmask 255.255.255.0 broadcast 192.168.1.255 up");
        // // GetSystemRetValue("ifconfig bridge1 192.168.1.1 netmask 255.255.255.0 broadcast 192.168.1.255 down");
        // //GetSystemRetValue("sh /oemapp/scripts/vlan_set_modem2.sh 1 192.168.1.1 192.168.1.4");
        // sleep(1);
        // GetSystemRetValue("ifconfig eth0 down");
        // sleep(1);
        // GetSystemRetValue("ifconfig eth0 up");
        // sleep(1);
        // // GetSystemRetValue("ifconfig bridge1 down");
        // sleep(1);
        // GetSystemRetValue("/oemapp/scripts/monitor_dns_1.sh");
    }


    sleep(3);

    while (1)
    {
        // if(g_modemId == 1) //主模组
        // {
        //     //发送模组网络状态
        //     uint8_t mcuBuf[2] = {0};
        //     mcuBuf[0] = g_modemId;   //模组1
        //     mcuBuf[1] = g_PingResult == 0 ? 1 : 2;  //网络状态
        //     if(g_PingResult == 0)
        //     {
        //         //模组1在网
        //         enable_vlan();
        //     }
        //     else
        //     {
        //         //模组1不在网
        //         disable_vlan();
        //     }
        //     McuMesg_t mcuMesg = {0};
        //     mcuMesg.type = SYS_QUE_TYPE_MCU_SYNC_MODEM_ALIVE;
        //     mcuMesg.payload = mcuBuf;
        //     mcuMesg.payloadLen = sizeof(mcuBuf);
        //     McuSrvSendMesg(&mcuMesg);
        //     LOG_Debug("%s -  modemID: %d,isAlive : %d",__FUNCTION__,mcuBuf[0],mcuBuf[1]);
        // }
        // else if (g_modemId == 2) //副模组
        // {
        //     //副模组暂时不发送心跳
        // }
        
        sleep_clock(1);
    }
}

void NetMonitorStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    if((NULL == recvData) || (sizeof(NetState_t) != dataLen))
    {
        LOG_Error("%s argv is invalid, dataLen = %d", __FUNCTION__, dataLen);
        return;
    }
    NetState_t tmpBuf;
    memcpy(&tmpBuf, recvData, dataLen);

    g_netMonitorStatus[0] =  tmpBuf.url1 << 7 | tmpBuf.url2 << 6 | tmpBuf.url3 << 5;
    g_netMonitorStatus[1] =  tmpBuf.gateway << 7 | tmpBuf.xxka << 6 | tmpBuf.xxkb << 5 | tmpBuf.onlineLanguageModule << 4 |
        tmpBuf.bluetoothToEthernetModule << 3 |  tmpBuf.module1 << 2 | tmpBuf.module2 << 1;
    
    LOG_Debug("g_netMonitorStatus:%#x %#x",g_netMonitorStatus[0],g_netMonitorStatus[1]);
}


