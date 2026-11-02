#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sys/wait.h>
#include "err_code.h"
#include "sr_log.h"

#include "net_interface_utils.h"
#include "net_route_utils.h"

/* 同一目的地址可能因 dev/metric 不同存在多条路由，删除时循环到没有为止 */
#define ROUTE_DEL_LOOP_MAX (8)

int addRoute(const char *ip, const char *dev, uint32_t priority)
{
    char command[256] = {0};
    int status = 0;

    if ((ip == NULL) || (dev == NULL) || (ip[0] == '\0') || (dev[0] == '\0'))
    {
        LOG_Error("%s: invalid parameter, ip=%p, dev=%p", __FUNCTION__, ip, dev);
        return RET_ERR;
    }

    snprintf(command, sizeof(command), "ip route replace %s dev %s metric %u", ip, dev, priority);
    status = system(command);
    if (status == -1)
    {
        LOG_Error("%s: system failed, command=%s, errno=%d(%s)",
                  __FUNCTION__, command, errno, strerror(errno));
        return RET_ERR;
    }

    if (!WIFEXITED(status))
    {
        LOG_Error("%s: command terminated abnormally, command=%s, status=%d",
                  __FUNCTION__, command, status);
        return RET_ERR;
    }

    if (WEXITSTATUS(status) != 0)
    {
        LOG_Error("%s: command failed, command=%s, exitCode=%d",
                  __FUNCTION__, command, WEXITSTATUS(status));
        return RET_ERR;
    }

    return RET_OK;
}


/**
 * @brief 删除指定目的地址的全部已有路由（不限网卡、metric）。
 * @param route 目标路由（IPv4地址或CIDR网段）。
 * @return RET_OK 成功（含原本就没有路由的情况），RET_ERR 参数非法。
 */
int delRoute(const char *route)
{
    char command[256] = {0};
    int status = 0;
    int deleted = 0;
    int i = 0;

    if ((route == NULL) || (route[0] == '\0'))
    {
        LOG_Error("%s: invalid parameter, route=%p", __FUNCTION__, route);
        return RET_ERR;
    }

    /* 与 CheckRouteExists 保持一致的 shell 防御 */
    if ((strchr(route, ';') != NULL) || (strchr(route, '&') != NULL) ||
        (strchr(route, '|') != NULL) || (strchr(route, '`') != NULL))
    {
        LOG_Error("%s: invalid character in route[%s]", __FUNCTION__, route);
        return RET_ERR;
    }

    for (i = 0; i < ROUTE_DEL_LOOP_MAX; i++)
    {
        snprintf(command, sizeof(command), "ip route del %s 2>/dev/null", route);
        status = system(command);
        if ((status == -1) || !WIFEXITED(status) || (WEXITSTATUS(status) != 0))
        {
            /* 没有更多匹配条目 */
            break;
        }
        deleted++;
    }

    if (deleted > 0)
    {
        LOG_Info("%s: deleted %d old route(s) for %s", __FUNCTION__, deleted, route);
    }

    return RET_OK;
}


/**
 * @brief 检查 IP 路由是否存在于主路由表中
 * @param route 目标路由（IPv4地址、CIDR网段或"default"）
 * @param dev 网卡名
 * @param metric 优先级；传入 0 表示不检查 metric
 * @retval 1 存在
 * @retval 0 不存在
 */
int CheckRouteExists(const char *route, const char *dev, uint32_t metric)
{
    FILE *fp = NULL;
    char buffer[1024] = {0};
    char cmd[256] = {0};
    char routeWithMask[64] = {0};
    char metricStr[32] = {0};
    int found = 0;
    int cmdLen = 0;
    size_t routeLen = 0;

    if ((route == NULL) || (dev == NULL) || (route[0] == '\0') || (dev[0] == '\0'))
    {
        LOG_Warn("%s: invalid parameter, route=%p, dev=%p", __FUNCTION__, route, dev);
        return 0;
    }

    /* 简单防御，避免异常字符进入 shell */
    if ((strchr(route, ';') != NULL) || (strchr(route, '&') != NULL) ||
        (strchr(route, '|') != NULL) || (strchr(route, '`') != NULL) ||
        (strchr(dev, ';') != NULL) || (strchr(dev, '&') != NULL) ||
        (strchr(dev, '|') != NULL) || (strchr(dev, '`') != NULL))
    {
        LOG_Warn("%s: invalid character in route[%s] or dev[%s]", __FUNCTION__, route, dev);
        return 0;
    }

    cmdLen = snprintf(cmd, sizeof(cmd), "ip route show %s dev %s 2>/dev/null", route, dev);
    if ((cmdLen < 0) || (cmdLen >= (int)sizeof(cmd)))
    {
        LOG_Warn("%s: snprintf cmd failed, route=%s, dev=%s", __FUNCTION__, route, dev);
        return 0;
    }

    if (metric != 0)
    {
        (void)snprintf(metricStr, sizeof(metricStr), "metric %u", metric);
    }

    routeLen = strlen(route);
    (void)snprintf(routeWithMask, sizeof(routeWithMask), "%s/32", route);

    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        LOG_Warn("%s: popen failed, cmd=%s, errno=%d(%s)",
                 __FUNCTION__, cmd, errno, strerror(errno));
        return 0;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        int prefixMatch = 0;

        if (strcmp(route, "default") == 0)
        {
            prefixMatch = (strncmp(buffer, "default", strlen("default")) == 0);
        }
        else if (strchr(route, '/') != NULL)
        {
            prefixMatch = (strncmp(buffer, route, routeLen) == 0);
        }
        else
        {
            if ((strncmp(buffer, route, routeLen) == 0) &&
                ((buffer[routeLen] == ' ') || (buffer[routeLen] == '\t') || (buffer[routeLen] == '/')))
            {
                prefixMatch = 1;
            }

            if (!prefixMatch && strncmp(buffer, routeWithMask, strlen(routeWithMask)) == 0)
            {
                prefixMatch = 1;
            }
        }

        if (!prefixMatch)
        {
            continue;
        }

        if (metric != 0)
        {
            char *p = strstr(buffer, metricStr);
            if (p == NULL)
            {
                continue;
            }
            /* 防止 "metric 100" 误匹配 "metric 1000" */
            char tail = p[strlen(metricStr)];
            if ((tail != '\0') && (tail != ' ') && (tail != '\t') && (tail != '\n'))
            {
                continue;
            }
        }

        found = 1;
        break;
    }

    (void)pclose(fp);

    LOG_Debug("%s: route[%s] dev[%s] metric[%u] %s",
              __FUNCTION__, route, dev, metric, (found != 0) ? "exists" : "not found");

    return found;
}


void DeleteDefaultRoute()
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


void SetDefaultRoute(const char* gateway, const char* interface)
{
    
    if(NULL == gateway || NULL == interface)
    {
        LOG_Error("%s : paramater is NULL.",__FUNCTION__);
        return;
    }

    
    if (WaitInterfaceReady(interface, NULL, 600U) != RET_OK)
    {
        LOG_Error("%s : interface %s not ready", __FUNCTION__, interface);
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
