#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "err_code.h"
#include "sr_log.h"

#include "net_interface_utils.h"

/**
 * @brief 获取指定网卡是否处于UP状态。
 * @param dev 网卡名。
 * @return RET_OK 成功，RET_ERR 失败。
 */
int GetInterfaceFlags(const char *dev, short *flags)
{
    int sockFd = -1;
    struct ifreq ifr;

    if ((dev == NULL) || (flags == NULL))
    {
        LOG_Error("%s: invalid parameter", __FUNCTION__);
        return RET_ERR;
    }

    sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd < 0)
    {
        LOG_Error("%s: socket failed, errno=%d", __FUNCTION__, errno);
        return RET_ERR;
    }

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", dev);
    if (ioctl(sockFd, SIOCGIFFLAGS, &ifr) < 0)
    {
        LOG_Error("%s: ioctl SIOCGIFFLAGS failed, dev=%s, errno=%d", __FUNCTION__, dev, errno);
        close(sockFd);
        return RET_ERR;
    }

    *flags = ifr.ifr_flags;
    close(sockFd);
    return RET_OK;
}


/**
 * @brief 获取网卡上的第一个IPv4地址。
 * @param dev 网卡名。
 * @param ip 输出IP地址。
 * @param ipLen 输出缓冲区长度。
 * @return RET_OK 成功，RET_ERR 失败。
 */
int GetInterfaceIpv4(const char *dev, char *ip, size_t ipLen)
{
    struct ifaddrs *ifAddrList = NULL;
    struct ifaddrs *ifa = NULL;

    if ((dev == NULL) || (ip == NULL) || (ipLen == 0U))
    {
        LOG_Error("%s: invalid parameter", __FUNCTION__);
        return RET_ERR;
    }

    ip[0] = '\0';
    if (getifaddrs(&ifAddrList) != 0)
    {
        LOG_Error("%s: getifaddrs failed, errno=%d", __FUNCTION__, errno);
        return RET_ERR;
    }

    for (ifa = ifAddrList; ifa != NULL; ifa = ifa->ifa_next)
    {
        if ((ifa->ifa_name == NULL) || (ifa->ifa_addr == NULL))
        {
            continue;
        }

        if (strcmp(ifa->ifa_name, dev) != 0)
        {
            continue;
        }

        if (ifa->ifa_addr->sa_family != AF_INET)
        {
            continue;
        }

        if (inet_ntop(AF_INET,
                      &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
                      ip,
                      ipLen) == NULL)
        {
            freeifaddrs(ifAddrList);
            LOG_Error("%s: inet_ntop failed, dev=%s", __FUNCTION__, dev);
            return RET_ERR;
        }

        freeifaddrs(ifAddrList);
        return RET_OK;
    }

    freeifaddrs(ifAddrList);
    return RET_ERR;
}


/**
 * @brief 等待拨号网卡真正ready。
 * @param dev 网卡名。
 * @param expectedIp 期望IP，可为NULL。
 * @param timeoutMs 超时时间，单位毫秒。
 * @return RET_OK 成功，RET_ERR 失败。
 */
int WaitInterfaceReady(const char *dev, const char *expectedIp, uint32_t timeoutMs)
{
    uint32_t waitedMs = 0U;
    short flags = 0;
    char currentIp[INET_ADDRSTRLEN] = {0};

    if (dev == NULL)
    {
        LOG_Error("%s: dev is NULL", __FUNCTION__);
        return RET_ERR;
    }

    while (waitedMs <= timeoutMs)
    {
        if ((GetInterfaceFlags(dev, &flags) == RET_OK) &&
            ((flags & IFF_UP) != 0) &&
            (GetInterfaceIpv4(dev, currentIp, sizeof(currentIp)) == RET_OK))
        {
            if ((expectedIp == NULL) || (expectedIp[0] == '\0') || (strcmp(expectedIp, currentIp) == 0))
            {
                LOG_Info("%s: interface %s ready, ip=%s", __FUNCTION__, dev, currentIp);
                return RET_OK;
            }
        }

        usleep(200 * 1000);
        waitedMs += 200U;
    }

    LOG_Error("%s: interface %s not ready in %u ms", __FUNCTION__, dev, timeoutMs);
    return RET_ERR;
}
