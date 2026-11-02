#ifndef _NET_ROUTE_DOMAIN_H_
#define _NET_ROUTE_DOMAIN_H_

#include <stdint.h>
#include "net_service_api.h"
#include "net_route_cache.h"

int getIPFromDomain(const char *domain, const char *dev, uint32_t priority,
                           const char *priDns, const char *secDns,
                           StrAddNetRouteResp_t *resp);

/**
 * @brief 执行 IP 路由设置（带验证）
 * @param route 目标路由
 * @param ethName 网卡名
 * @param priority 优先级
 * @retval RET_OK 成功
 * @retval RET_ERR 失败
 */
int8_t DoSetIpRoute(const char *route, const char *ethName, uint32_t priority, StrAddNetRouteResp_t *resp);

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
                               StrAddNetRouteResp_t *resp);

int AddRouteIpToInterface(char *ip, char *dev);

int AddDomainIpToInterface(char *domain, char *dev, const char *priDns, const char *secDns);

#endif