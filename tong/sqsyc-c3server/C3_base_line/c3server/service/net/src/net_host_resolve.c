#define _GNU_SOURCE
#include "net_host_resolve.h"
#include "err_code.h"
#include "sr_log.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define DNS_PORT                   (53)
#define DNS_QUERY_TIMEOUT_S        (3)
#define DNS_MAX_SERVERS            (4)
#define DNS_QUERY_BUF_SIZE         (512)
#define DNS_ANSWER_BUF_SIZE        (512)

/**
 * @brief 构造 DNS A 记录查询报文。
 * @param domain 待查询域名。
 * @param buf 输出缓冲区。
 * @param bufSize 缓冲区大小。
 * @param txid 事务 ID。
 * @return 查询报文长度，失败返回负值。
 */
static int DnsBuildQuery(const char *domain, uint8_t *buf, size_t bufSize, uint16_t txid)
{
    size_t pos = 0;
    const char *label = domain;
    const char *dot = NULL;
    size_t segLen = 0;

    if ((domain == NULL) || (buf == NULL) ||
        (bufSize < (12U + strlen(domain) + 2U + 4U)))
    {
        return -1;
    }

    /* Header: ID + flags(0x0100 RD) + QDCOUNT=1 */
    buf[pos++] = (uint8_t)(txid >> 8);
    buf[pos++] = (uint8_t)(txid & 0xFFU);
    buf[pos++] = 0x01;
    buf[pos++] = 0x00;
    buf[pos++] = 0x00; buf[pos++] = 0x01;
    buf[pos++] = 0x00; buf[pos++] = 0x00;
    buf[pos++] = 0x00; buf[pos++] = 0x00;
    buf[pos++] = 0x00; buf[pos++] = 0x00;

    /* QNAME: length-prefixed labels */
    while (label[0] != '\0')
    {
        dot = strchr(label, '.');
        segLen = (dot != NULL) ? (size_t)(dot - label) : strlen(label);
        if ((segLen == 0U) || (segLen > 63U) || ((pos + segLen + 1U) > bufSize))
        {
            return -1;
        }
        buf[pos++] = (uint8_t)segLen;
        (void)memcpy(&buf[pos], label, segLen);
        pos += segLen;
        label = (dot != NULL) ? (dot + 1) : (label + segLen);
    }
    buf[pos++] = 0x00;

    /* QTYPE=A(1), QCLASS=IN(1) */
    buf[pos++] = 0x00; buf[pos++] = 0x01;
    buf[pos++] = 0x00; buf[pos++] = 0x01;

    return (int)pos;
}

/**
 * @brief 跳过 DNS 报文中的域名字段（支持压缩指针）。
 */
static int DnsSkipName(const uint8_t *buf, size_t bufLen, size_t *offset)
{
    size_t idx = 0;
    uint8_t len = 0;
    int ret = -1;

    idx = *offset;
    do
    {
        if ((buf == NULL) || (offset == NULL))
        {
            break;
        }

        while (idx < bufLen)
        {
            len = buf[idx];
            if (len == 0U)
            {
                idx++;
                *offset = idx;
                ret = 0;
                break;
            }
            if ((len & 0xC0U) == 0xC0U)
            {
                if ((idx + 1U) >= bufLen)
                {
                    break;
                }
                idx += 2U;
                *offset = idx;
                ret = 0;
                break;
            }
            if ((len & 0xC0U) != 0U)
            {
                break;
            }
            idx += len + 1U;
            if (idx > bufLen)
            {
                break;
            }
        }
    } while (0);

    return ret;
}

/**
 * @brief 从 DNS 应答中解析 IPv4 A 记录。
 * @return 解析到的 IPv4 数量。
 */
static int DnsParseAnswers(const uint8_t *resp, size_t respLen,
                           char ips[][MAX_IP_LEN], int maxIps)
{
    int ipCount = 0;
    size_t pos = 0;
    uint16_t qdCount = 0;
    uint16_t anCount = 0;
    uint16_t i = 0;
    uint16_t type = 0;
    uint16_t rdLen = 0;
    struct in_addr rdataAddr = {0};

    if ((resp == NULL) || (respLen < 12U) || (ips == NULL) || (maxIps <= 0))
    {
        return 0;
    }

    qdCount = (uint16_t)(((uint16_t)resp[4] << 8) | (uint16_t)resp[5]);
    anCount = (uint16_t)(((uint16_t)resp[6] << 8) | (uint16_t)resp[7]);

    pos = 12U;
    for (i = 0; i < qdCount; i++)
    {
        if (DnsSkipName(resp, respLen, &pos) != 0)
        {
            return 0;
        }
        if ((pos + 4U) > respLen)
        {
            return 0;
        }
        pos += 4U;
    }

    for (i = 0; (i < anCount) && (ipCount < maxIps); i++)
    {
        if (DnsSkipName(resp, respLen, &pos) != 0)
        {
            return ipCount;
        }
        if ((pos + 10U) > respLen)
        {
            return ipCount;
        }
        type = (uint16_t)(((uint16_t)resp[pos] << 8) | (uint16_t)resp[pos + 1]);
        rdLen = (uint16_t)(((uint16_t)resp[pos + 8] << 8) | (uint16_t)resp[pos + 9]);
        pos += 10U;
        if ((pos + rdLen) > respLen)
        {
            return ipCount;
        }
        if ((type == 1U) && (rdLen == sizeof(uint32_t)))
        {
            (void)memcpy(&rdataAddr.s_addr, &resp[pos], sizeof(uint32_t));
            if (inet_ntop(AF_INET, &rdataAddr, ips[ipCount], MAX_IP_LEN) != NULL)
            {
                ipCount++;
            }
        }
        pos += rdLen;
    }

    return ipCount;
}

/**
 * @brief 校验并收集有效的 IPv4 DNS 服务器地址。
 *        仅使用传入的 priDns/secDns，不再解析 /etc/resolv.conf。
 */
static int GetDnsServersForDev(const char *priDns, const char *secDns,
                               char servers[][INET_ADDRSTRLEN], int maxServers)
{
    int count = 0;
    struct in_addr addr = {0};

    if ((servers == NULL) || (maxServers <= 0))
    {
        return 0;
    }

    if ((priDns != NULL) && (priDns[0] != '\0') &&
        (inet_pton(AF_INET, priDns, &addr) == 1))
    {
        (void)strncpy(servers[count], priDns, INET_ADDRSTRLEN - 1);
        servers[count][INET_ADDRSTRLEN - 1] = '\0';
        count++;
    }

    if ((count < maxServers) && (secDns != NULL) && (secDns[0] != '\0') &&
        (inet_pton(AF_INET, secDns, &addr) == 1))
    {
        (void)strncpy(servers[count], secDns, INET_ADDRSTRLEN - 1);
        servers[count][INET_ADDRSTRLEN - 1] = '\0';
        count++;
    }

    return count;
}

int ResolveDomainByDns(const char *domain, const char *dev,
                       const char *priDns, const char *secDns,
                       char ips[][MAX_IP_LEN], int maxIps)
{
    char servers[DNS_MAX_SERVERS][INET_ADDRSTRLEN] = {{0}};
    int serverCount = 0;
    int i = 0;
    int j = 0;
    int ipCount = 0;
    uint16_t txid = 0;
    int sockFd = -1;
    struct sockaddr_in srvAddr = {0};
    uint8_t query[DNS_QUERY_BUF_SIZE] = {0};
    uint8_t answer[DNS_ANSWER_BUF_SIZE] = {0};
    int queryLen = 0;
    ssize_t sendLen = 0;
    ssize_t recvLen = 0;
    struct timeval tv = {0};

    if ((domain == NULL) || (dev == NULL) || (ips == NULL) || (maxIps <= 0))
    {
        return RET_ERR;
    }

    serverCount = GetDnsServersForDev(priDns, secDns, servers, DNS_MAX_SERVERS);
    if (serverCount <= 0)
    {
        LOG_Error("%s: no valid DNS server for dev=%s", __FUNCTION__, dev);
        return RET_ERR;
    }

    txid = (uint16_t)((uint32_t)time(NULL) ^ (uint32_t)getpid());

    for (i = 0; i < serverCount; i++)
    {
        sockFd = -1;
        do
        {
            queryLen = DnsBuildQuery(domain, query, sizeof(query), txid + (uint16_t)i);
            if (queryLen <= 0)
            {
                break;
            }

            sockFd = socket(AF_INET, SOCK_DGRAM, 0);
            if (sockFd < 0)
            {
                LOG_Error("%s: socket failed, errno=%d", __FUNCTION__, errno);
                break;
            }

            /* 强制 DNS 报文从指定网卡发出 */
            if (setsockopt(sockFd, SOL_SOCKET, SO_BINDTODEVICE, dev, strlen(dev)) < 0)
            {
                LOG_Error("%s: SO_BINDTODEVICE dev=%s failed, errno=%d",
                          __FUNCTION__, dev, errno);
                break;
            }

            tv.tv_sec = DNS_QUERY_TIMEOUT_S;
            tv.tv_usec = 0;
            if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
            {
                LOG_Error("%s: SO_RCVTIMEO failed, errno=%d", __FUNCTION__, errno);
                break;
            }

            (void)memset(&srvAddr, 0, sizeof(srvAddr));
            srvAddr.sin_family = AF_INET;
            srvAddr.sin_port = htons(DNS_PORT);
            if (inet_pton(AF_INET, servers[i], &srvAddr.sin_addr) != 1)
            {
                break;
            }

            sendLen = sendto(sockFd, query, (size_t)queryLen, 0,
                             (struct sockaddr *)&srvAddr, sizeof(srvAddr));
            if (sendLen != queryLen)
            {
                LOG_Error("%s: sendto DNS server %s failed, errno=%d",
                          __FUNCTION__, servers[i], errno);
                break;
            }

            recvLen = recvfrom(sockFd, answer, sizeof(answer), 0, NULL, NULL);
            if (recvLen < 0)
            {
                LOG_Error("%s: recvfrom DNS server %s timeout or failed, errno=%d",
                          __FUNCTION__, servers[i], errno);
                break;
            }

            ipCount = DnsParseAnswers(answer, (size_t)recvLen, ips, maxIps);
        } while (0);

        if (sockFd >= 0)
        {
            close(sockFd);
            sockFd = -1;
        }

        if (ipCount > 0)
        {
            LOG_Info("%s: resolve domain=%s via dev=%s success, dns=%s, ipCount=%d",
                     __FUNCTION__, domain, dev, servers[i], ipCount);
            for (j = 0; j < ipCount; j++)
            {
                LOG_Info("%s: resolved ip[%d]=%s", __FUNCTION__, j, ips[j]);
            }
            return ipCount;
        }
    }

    LOG_Error("%s: resolve domain=%s via dev=%s failed", __FUNCTION__, domain, dev);
    return RET_ERR;
}
