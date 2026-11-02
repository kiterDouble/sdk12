#ifndef _NET_ROUTE_UTILS_H_
#define _NET_ROUTE_UTILS_H_

#include <stdint.h>


int addRoute(const char *ip, const char *dev, uint32_t priority);

/**
 * @brief 删除指定目的地址的全部已有路由（不限网卡、metric）。
 * @param route 目标路由（IPv4地址或CIDR网段）。
 * @return RET_OK 成功（含原本就没有路由的情况），RET_ERR 参数非法。
 */
int delRoute(const char *route);

/**
 * @brief 检查 IP 路由是否存在于主路由表中
 * @param route 目标路由（IPv4地址、CIDR网段或"default"）
 * @param dev 网卡名
 * @param metric 优先级；传入 0 表示不检查 metric
 * @retval 1 存在
 * @retval 0 不存在
 */
int CheckRouteExists(const char *route, const char *dev, uint32_t metric);

void DeleteDefaultRoute();

void SetDefaultRoute(const char* gateway, const char* interface);


#endif