#ifndef _NET_HOST_RESOLVE_H_
#define _NET_HOST_RESOLVE_H_

#include "net_service_api.h"

/**
 * @brief 通过指定网卡向指定 DNS 服务器查询域名的 IPv4 地址。
 * @param domain 待查询域名。
 * @param dev 绑定网卡名，DNS 报文强制从该网卡发出。
 * @param priDns 主 DNS，为空或无效时跳过。
 * @param secDns 备 DNS，为空或无效时跳过。
 * @param ips 输出解析到的 IPv4 地址列表。
 * @param maxIps 最大可输出 IP 数量。
 * @return 解析到的 IP 数量，失败返回 RET_ERR。
 */
int ResolveDomainByDns(const char *domain, const char *dev,
                       const char *priDns, const char *secDns,
                       char ips[][MAX_IP_LEN], int maxIps);

#endif /* _NET_HOST_RESOLVE_H_ */
