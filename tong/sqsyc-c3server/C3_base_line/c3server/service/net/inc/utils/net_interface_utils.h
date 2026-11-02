#ifndef _NET_INTERFACE_UTILS_H_
#define _NET_INTERFACE_UTILS_H_


#include <stdint.h>
#include <stddef.h>

/**
 * @brief 获取指定网卡是否处于UP状态。
 * @param dev 网卡名。
 * @return RET_OK 成功，RET_ERR 失败。
 */
int GetInterfaceFlags(const char *dev, short *flags);


/**
 * @brief 获取网卡上的第一个IPv4地址。
 * @param dev 网卡名。
 * @param ip 输出IP地址。
 * @param ipLen 输出缓冲区长度。
 * @return RET_OK 成功，RET_ERR 失败。
 */
int GetInterfaceIpv4(const char *dev, char *ip, size_t ipLen);


/**
 * @brief 等待拨号网卡真正ready。
 * @param dev 网卡名。
 * @param expectedIp 期望IP，可为NULL。
 * @param timeoutMs 超时时间，单位毫秒。
 * @return RET_OK 成功，RET_ERR 失败。
 */
int WaitInterfaceReady(const char *dev, const char *expectedIp, uint32_t timeoutMs);


#endif