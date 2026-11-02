#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "err_code.h"
#include "sr_log.h"
#include "net_service_api.h"
#include "net_host_resolve.h"
#include "net_route_cache.h"
#include "net_route_utils.h"
#include "net_shell_utils.h"
#include "net_route_domain.h"

#define NET_ROUTE_DEFAULT_PRIORITY (100)
#define ROUTE_SET_RETRY_DELAY_MS   (200)
#define ROUTE_SET_RETRY_MAX        (3)

/**
 * @brief 检查域名路由是否存在于缓存中
 * @param domain 域名
 * @param dev 网卡名
 * @retval 1 存在
 * @retval 0 不存在
 */
static int CheckDomainRouteExists(const char *domain, const char *dev)
{
    char devs[MAX_BIND_ETH_NUM][MAX_ETH_NAME_LEN] = {0};
    int count = 0;
    int i;

    if (!domain || !dev) {
        return 0;
    }

    /* 查询路由缓存 */
    if (QueryRoute(domain, devs, &count, MAX_BIND_ETH_NUM) != RET_OK) {
        return 0;
    }

    for (i = 0; i < count; i++) {
        if (strcmp(devs[i], dev) == 0) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief 把一个已生效的IP追加到应答结构体。
 * @param resp 应答结构体（可为NULL）。
 * @param ip 待追加的IP字符串。
 * @return RET_OK / RET_ERR
 */
static int AddResolvedIp(StrAddNetRouteResp_t *resp, const char *ip)
{
    int idx = 0;

    if ((resp == NULL) || (ip == NULL) || (ip[0] == '\0'))
    {
        return RET_ERR;
    }

    if (resp->ipCount >= MAX_RESOLVE_IP_NUM)
    {
        LOG_Warn("%s: ip list full, ignore %s", __FUNCTION__, ip);
        return RET_ERR;
    }

    idx = resp->ipCount;
    (void)strncpy(resp->ip[idx], ip, MAX_IP_LEN - 1);
    resp->ip[idx][MAX_IP_LEN - 1] = '\0';
    resp->ipCount++;

    LOG_Debug("%s: add resolved ip[%d]=%s", __FUNCTION__, idx, ip);
    return RET_OK;
}



int getIPFromDomain(const char *domain, const char *dev, uint32_t priority,
                           const char *priDns, const char *secDns,
                           StrAddNetRouteResp_t *resp)
{
    char resolvedIps[MAX_RESOLVE_IP_NUM][MAX_IP_LEN] = {{0}};
    int resolvedCount = 0;
    int totalIpCount = 0;
    int addSuccessCount = 0;
    int verifySuccessCount = 0;
    int i = 0;
    struct in_addr addr;
    int hasPriDns = 0;
    int hasSecDns = 0;

    if ((domain == NULL) || (dev == NULL) || (domain[0] == '\0') || (dev[0] == '\0'))
    {
        LOG_Error("%s: parameter domain or dev is invalid.", __FUNCTION__);
        return RET_ERR;
    }

    hasPriDns = ((priDns != NULL) && (priDns[0] != '\0') && (inet_pton(AF_INET, priDns, &addr) == 1));
    hasSecDns = ((secDns != NULL) && (secDns[0] != '\0') && (inet_pton(AF_INET, secDns, &addr) == 1));

    if (hasPriDns || hasSecDns)
    {
        /* 传入了有效 DNS：通过指定 DNS 查询 */
        resolvedCount = ResolveDomainByDns(domain, dev, priDns, secDns, resolvedIps, MAX_RESOLVE_IP_NUM);
        if (resolvedCount <= 0)
        {
            LOG_Error("%s: resolve domain=%s via dev=%s failed", __FUNCTION__, domain, dev);
            return RET_ERR;
        }
    }
    else
    {
        /* 两个 DNS 都无效：沿用原系统解析逻辑 */
        struct addrinfo hints;
        struct addrinfo *result = NULL;
        struct addrinfo *p = NULL;
        int status = 0;

        (void)memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;      /* 只处理 IPv4 */
        hints.ai_socktype = SOCK_STREAM;

        status = getaddrinfo(domain, NULL, &hints, &result);
        if (status != 0)
        {
            LOG_Error("%s: getaddrinfo failed, domain=%s, err=%s",
                      __FUNCTION__, domain, gai_strerror(status));
            return RET_ERR;
        }

        for (p = result; (p != NULL) && (resolvedCount < MAX_RESOLVE_IP_NUM); p = p->ai_next)
        {
            struct sockaddr_in *ipv4 = NULL;
            char ip[INET_ADDRSTRLEN] = {0};

            if ((p->ai_addr == NULL) || (p->ai_family != AF_INET))
            {
                continue;
            }

            ipv4 = (struct sockaddr_in *)p->ai_addr;
            if (inet_ntop(AF_INET, &(ipv4->sin_addr), ip, sizeof(ip)) == NULL)
            {
                LOG_Error("%s: inet_ntop failed, domain=%s", __FUNCTION__, domain);
                continue;
            }

            (void)strncpy(resolvedIps[resolvedCount], ip, MAX_IP_LEN - 1);
            resolvedIps[resolvedCount][MAX_IP_LEN - 1] = '\0';
            resolvedCount++;
        }

        freeaddrinfo(result);

        if (resolvedCount <= 0)
        {
            LOG_Error("%s: getaddrinfo resolved no IPv4 for domain=%s", __FUNCTION__, domain);
            return RET_ERR;
        }
    }

    for (i = 0; i < resolvedCount; i++)
    {
        const char *ip = resolvedIps[i];

        totalIpCount++;

        LOG_Debug("%s: domain=%s resolved ip=%s, dev=%s, priority=%u",
                  __FUNCTION__, domain, ip, dev, priority);

        /* 目的地址+网卡+metric 完全一致的路由已存在，直接视为成功，无需删除再添加 */
        if (CheckRouteExists(ip, dev, priority) != 0)
        {
            addSuccessCount++;
            verifySuccessCount++;
            (void)AddResolvedIp(resp, ip);
            LOG_Info("%s: route already exists, skip, ip=%s via %s priority=%u",
                     __FUNCTION__, ip, dev, priority);
            continue;
        }

        /* 先删除该解析 IP 可能挂在其他网卡/metric 上的旧路由，再重新添加 */
        (void)delRoute(ip);

        /* 添加 IP 路由 */
        if (addRoute(ip, dev, priority) != RET_OK)
        {
            LOG_Error("%s: addRoute failed for ip=%s, dev=%s", __FUNCTION__, ip, dev);
            continue;
        }

        addSuccessCount++;

        /* 等待路由落表 */
        usleep(ROUTE_SET_RETRY_DELAY_MS * 1000);

        /* 校验该 IP 是否已真正出现在路由表中 */
        if (CheckRouteExists(ip, dev, priority) != 0)
        {
            verifySuccessCount++;
            (void)AddResolvedIp(resp, ip);
            LOG_Info("%s: route verified, ip=%s via %s priority=%u",
                     __FUNCTION__, ip, dev, priority);
        }
        else
        {
            LOG_Error("%s: route verify failed, ip=%s via %s",
                      __FUNCTION__, ip, dev);
        }
    }

    LOG_Debug("%s: domain=%s totalIpCount=%d addSuccessCount=%d verifySuccessCount=%d",
              __FUNCTION__, domain, totalIpCount, addSuccessCount, verifySuccessCount);

    /*
     * 判定策略：
     * 1. 至少解析出一个 IPv4
     * 2. 至少有一个 IP 加路由成功
     * 3. 至少有一个 IP 在路由表中校验成功
     */
    if ((totalIpCount > 0) && (addSuccessCount > 0) && (verifySuccessCount > 0))
    {
        return RET_OK;
    }

    return RET_ERR;
}


/**
 * @brief 执行 IP 路由设置（带验证）
 * @param route 目标路由
 * @param ethName 网卡名
 * @param priority 优先级
 * @retval RET_OK 成功
 * @retval RET_ERR 失败
 */
int8_t DoSetIpRoute(const char *route, const char *ethName, uint32_t priority, StrAddNetRouteResp_t *resp)
{
    if ((route == NULL) || (ethName == NULL) ||
        (route[0] == '\0') || (ethName[0] == '\0'))
    {
        LOG_Error("%s : invalid parameter, route=%p, ethName=%p",
                  __FUNCTION__, route, ethName);
        return RET_ERR;
    }

    LOG_Debug("%s : set route=%s via %s metric=%u",
              __FUNCTION__, route, ethName, priority);

    /* 目的地址+网卡+metric 完全一致的路由已存在，直接返回成功，无需删除再添加 */
    if (CheckRouteExists(route, ethName, priority) != 0)
    {
        LOG_Info("%s : route %s via %s metric=%u already exists, skip",
                 __FUNCTION__, route, ethName, priority);
        CacheRoute(route, ethName);
        AddResolvedIp(resp, route);
        return RET_OK;
    }

    /* 先删除同一目的地址的旧路由（可能挂在其他网卡/metric 上），再重新添加 */
    (void)delRoute(route);

    /* 统一复用 addRoute() 执行路由下发 */
    if (addRoute(route, ethName, priority) != RET_OK)
    {
        LOG_Warn("%s : addRoute failed, route=%s, ethName=%s, priority=%u",
                 __FUNCTION__, route, ethName, priority);
        return RET_ERR;
    }

    /* 命令执行后等待一小会再验证 */
    usleep(ROUTE_SET_RETRY_DELAY_MS * 1000);

    /* 验证路由是否存在 */
    if (CheckRouteExists(route, ethName, priority) != 0)
    {
        LOG_Info("%s : route %s via %s verified successfully",
                 __FUNCTION__, route, ethName);
        CacheRoute(route, ethName);
        AddResolvedIp(resp, route);
        return RET_OK;
    }

    LOG_Error("%s : route %s via %s not found in route table",
              __FUNCTION__, route, ethName);
    DebugCmd("ip route show");
    return RET_ERR;
}


/**
 * @brief 执行域名路由设置（按解析得到的 IP 路由是否成功入表判断）
 * @param domain 域名
 * @param ethName 网卡名
 * @param priority 优先级
 * @retval RET_OK 成功
 * @retval RET_ERR 失败
 */
int8_t DoSetDomainRoute(const char *domain, const char *ethName, uint32_t priority,
                               const char *priDns, const char *secDns,
                               StrAddNetRouteResp_t *resp)
{
    if ((domain == NULL) || (ethName == NULL) ||
        (domain[0] == '\0') || (ethName[0] == '\0'))
    {
        LOG_Error("%s : invalid parameter, domain=%p, ethName=%p",
                  __FUNCTION__, domain, ethName);
        return RET_ERR;
    }

    LOG_Debug("%s : domain=%s via %s, priority=%u",__FUNCTION__, domain, ethName, priority);

    /*
     * 域名路由最终落地的是一个或多个 IP 路由，
     * 因此这里直接以 getIPFromDomain() 中的“加路由+回读校验”结果为准。
     * getIPFromDomain() 内部会判断 priDns/secDns 是否有效：
     * 至少有一个有效则绑定网卡向该 DNS 查询；两个都无效则回退到 getaddrinfo。
     */
    if (getIPFromDomain(domain, ethName, priority, priDns, secDns, resp) != RET_OK)
    {
        LOG_Error("%s : domain route %s via %s failed, no resolved IP route verified",
                  __FUNCTION__, domain, ethName);
        DebugCmd("ip route show");
        return RET_ERR;
    }

    /* 缓存仅作为辅助记录，不再作为主成功判据 */
    CacheRoute(domain, ethName);

    if (CheckDomainRouteExists(domain, ethName) != 0)
    {
        LOG_Info("%s : domain route %s via %s verified successfully",
                 __FUNCTION__, domain, ethName);
    }
    else
    {
        LOG_Warn("%s : domain route %s via %s added successfully, but cache verify failed",
                 __FUNCTION__, domain, ethName);
    }

    return RET_OK;
}

int AddRouteIpToInterface(char *ip, char *dev)
{
    char *tmp1 = ip;
    char *tmp2 = NULL;
    int count = 0;
    char ipForSet[ROUTE_IP_DOMAIN_LEN] = {0};
    char cmdSetRoute[128 + ROUTE_IP_DOMAIN_LEN + ETH_NAME_LENGTH] = {0};
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

int AddDomainIpToInterface(char *domain, char *dev, const char *priDns, const char *secDns)
{
    char *tmp1 = domain;
    char *tmp2 = NULL;
    int count = 0;
    char ipForSet[128 + ROUTE_IP_DOMAIN_LEN + ETH_NAME_LENGTH] = {0};
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
            if ((getIPFromDomain(ipForSet, dev, (uint32_t)NET_ROUTE_DEFAULT_PRIORITY, priDns, secDns, NULL)) == RET_ERR)
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



