#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#include "sr_log.h"
#include "net_shell_utils.h"

#define PING_CMD "ping -c 1 -w 5 "


static bool check_dns_valid_ip(const uint8_t* dns)
{
    struct in_addr  addr4;
    struct in6_addr addr6;

    if (dns == NULL) {
        return false;
    }

    return (inet_pton(AF_INET,  (const char*)dns, &addr4) == 1 ||
            inet_pton(AF_INET6, (const char*)dns, &addr6) == 1);
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

int DebugCmd(const char *cmd)
{
    FILE *fp = NULL;
    int status;

    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        LOG_Debug("can not exec %s.", cmd);
        return -1;
    }

    {
        char outputLine[1024] = {0};
        while (fgets(outputLine, sizeof(outputLine), fp) != NULL)
        {
            LOG_Debug("%s : %s", __FUNCTION__, outputLine);
        }
    }

    status = pclose(fp);
    if (status == -1)
    {
        LOG_Debug("pclose failed for %s", cmd);
        return -1;
    }

    if (!WIFEXITED(status))
    {
        LOG_Debug("%s : command terminated abnormally, cmd=%s, status=%d",
                  __FUNCTION__, cmd, status);
        return -1;
    }

    return (WEXITSTATUS(status) == 0) ? 0 : 1;
}

int32_t CheckNetworkConnectivity(const char* ip, const char* interface)
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


void SetDnsAddr(uint8_t* dnsPathName, uint8_t nameserverPri,uint8_t* priDns, uint8_t* secDns)
{
    if (strcmp(dnsPathName, "NULL") == 0 || strlen(dnsPathName) == 0)
    {
        LOG_Debug("dnsPathName is NULL");
        return;
    }

    char cmd[128] = {0};

    if ((strcmp("0.0.0.0", (char *)priDns)) != 0 && check_dns_valid_ip(priDns))
    {
        if(0 == nameserverPri)
        {
            snprintf(cmd, sizeof(cmd), "sed -i '1inameserver %s' %s", priDns, dnsPathName);
        }
        else
        {
            snprintf(cmd, sizeof(cmd), "echo 'nameserver %s' >> %s", priDns, dnsPathName);
        }
        int status = system(cmd);
        if (status == 0)
        {
            LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, cmd);
        } else {
            LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, cmd);
        }
    }
    if ((strcmp("0.0.0.0", (char *)secDns)) != 0 && check_dns_valid_ip(secDns))
    {
        memset(cmd, 0x00, sizeof(cmd));
        if(0 == nameserverPri)
        {
            snprintf(cmd, sizeof(cmd), "sed -i '1inameserver %s' %s", secDns, dnsPathName);
        }
        else
        {
            snprintf(cmd, sizeof(cmd), "echo 'nameserver %s' >> %s", secDns, dnsPathName);
        }
        int status = system(cmd);
        if (status == 0)
        {
            LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, cmd);
        } else {
            LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, cmd);
        }
    }
}

void AddNameServerToRouteTable(uint8_t dnsRouteSet,uint8_t* priDns, uint8_t* secDns, uint8_t* name)
{
    char cmd[128] = {0x00};

    if(1 == dnsRouteSet)
    {
        if (strcmp("0.0.0.0", (char *)priDns) != 0 && check_dns_valid_ip(priDns))
        {
            snprintf(cmd, sizeof(cmd), "route add %s %s", priDns, name);
            int status = system(cmd);
            if (status == 0)
            {
                LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, cmd);
            } else {
                LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, cmd);
            }
        }
        
        if (strcmp("0.0.0.0", (char *)secDns) != 0 && check_dns_valid_ip(secDns))
        {
            memset(cmd, 0x00, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "route add %s %s", secDns, name);
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

void NetHighSpeedModeSet(uint8_t mode)
{
    char respData[128] = {0};
    char lineBuf[128] = {0};
    int  value = -1;
    FILE *fp = NULL;

    /* 通过 ql_cmd 查询 position 88 的当前值 */
    fp = popen("ql_cmd at AT+MEDCR=1,88", "r");
    if (fp == NULL)
    {
        LOG_Debug("popen ql_cmd failed");
        return;
    }

    /* 找到包含 +MEDCR: 的那一行，例如：+MEDCR:88,0 */
    while (fgets(lineBuf, sizeof(lineBuf), fp) != NULL)
    {
        if (strstr(lineBuf, "+MEDCR:") != NULL)
        {
            /* 保存原始行用于日志 */
            strncpy(respData, lineBuf, sizeof(respData) - 1);
            respData[sizeof(respData) - 1] = '\0';
            break;
        }
    }

    pclose(fp);

    if (respData[0] == '\0')
    {
        LOG_Debug("get medcr position 88 failed, no +MEDCR line");
        return;
    }

    LOG_Debug("get medcr position 88:%s", respData);

    /* 从形如 +MEDCR:88,0 里取出最后一个逗号后的数值(这里是0或3) */
    char *p = strrchr(respData, ',');
    if (p != NULL)
    {
        value = atoi(p + 1);
        LOG_Debug("Extracted value: %d", value);
    }
    else
    {
        LOG_Debug("No comma found in the resp string");
        return;
    }

    /* 根据 mode 和当前值决定是否需要下发设置命令 */
    if (mode == 1U && value != 3) /* 开启高速模式 */
    {
        (void)system("ql_cmd at AT+MEDCR=0,88,3");
    }
    else if (mode == 0U && value != 0) /* 关闭高速模式 */
    {
        (void)system("ql_cmd at AT+MEDCR=0,88,0");
    }
}

int manage_ping(time_t *nextPingTime, time_t currentTime)
{
    int ret = 0;

    if (nextPingTime != NULL)
    {
        if (*nextPingTime != 0 && currentTime >= *nextPingTime)
        {
            LOG_Debug("%s : execute ping .", __FUNCTION__);

            int pingResult = DebugCmd("ping -c 1 -w 1 8.8.8.8");
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
