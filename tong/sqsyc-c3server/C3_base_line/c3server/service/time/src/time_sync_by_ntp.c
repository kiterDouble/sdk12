#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>

#include <pthread.h>
#include <unistd.h>
#include <glib.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "sr_log.h"
#include "net_service_api.h"
#include "pm_server_api.h"

#include "time_status.h"
#include "time_server.h"

/** 2208988800 表示 1900 年到 1970 年之间的总秒数 **/
#define NTP_TIMESTAMP_DELTA 2208988800ull

/**
 * @brief NTP 协议数据包结构
 *
 * 标准 NTP v3 数据包格式（RFC 5905），共 48 字节。
 * 客户端关注的主要字段为 txTm_s（Transmit Timestamp Seconds）。
 */
typedef struct __attribute__((packed)) {
    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.

    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.
}ntp_packet;              // Total: 384 bits or 48 bytes.


/**
 * @brief 从指定 NTP 服务器获取 UTC 时间
 *
 * 构造 NTP 客户端请求包（LI=0, VN=3, MODE=3 -> 0x1b），
 * 通过 UDP socket 发送并接收响应，解析 txTm_s 转换为 Unix 时间戳。
 * 支持多地址轮询（getaddrinfo 返回的地址列表）。
 *
 * @param[in]  ntpHost NTP 服务器主机名或 IP 地址
 * @param[out] ntpTime 输出获取到的 UTC 时间戳（秒）
 * @return int 0 表示成功，-1 表示失败
 */
static int NetNTPTimeGet(char* ntpHost, time_t *ntpTime)
{
    int sockfd = -1;
    int ret = -1;                     // 默认失败
    ntp_packet packet = {0};
    struct addrinfo hints = {0};
    struct addrinfo *res = NULL;
    struct addrinfo *rp = NULL;
    struct timeval tv;

    do {
        // 初始化 NTP 包（第一个字节：LI=0, VN=3, MODE=3 -> 0x1b）
        memset(&packet, 0, sizeof(ntp_packet));
        *((char *)&packet + 0) = 0x1b;

        // 使用 getaddrinfo 解析 NTP 服务器地址（线程安全，支持 IPv4/IPv6）
        hints.ai_family = AF_INET;      // 仅使用 IPv4（NTP 服务器通常配置为 IPv4）
        hints.ai_socktype = SOCK_DGRAM;
        if (getaddrinfo(ntpHost, "123", &hints, &res) != 0) {
            LOG_Error("getaddrinfo(%s) failed", ntpHost);
            break;
        }

        // 遍历地址列表，尝试创建 socket 并通信
        for (rp = res; rp != NULL; rp = rp->ai_next) {
            sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sockfd < 0) {
                LOG_Error("socket() failed: %s", strerror(errno));
                continue;
            }

            // 设置接收超时 2 秒
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                LOG_Error("setsockopt(SO_RCVTIMEO) failed: %s", strerror(errno));
                close(sockfd);
                sockfd = -1;
                continue;
            }

            // 发送请求
            if (sendto(sockfd, &packet, sizeof(packet), 0,
                       rp->ai_addr, rp->ai_addrlen) < 0) {
                LOG_Error("sendto(%s) failed: %s", ntpHost, strerror(errno));
                close(sockfd);
                sockfd = -1;
                continue;
            }

            // 接收响应
            ssize_t recvLen = recv(sockfd, (char*)&packet, sizeof(ntp_packet), 0);
            if (recvLen < 0) {
                LOG_Error("recv(%s) failed: %s (timeout=%ds)", ntpHost, strerror(errno), (int)tv.tv_sec);
                close(sockfd);
                sockfd = -1;
                continue;
            }
            if ((size_t)recvLen != sizeof(ntp_packet)) {
                LOG_Error("recv(%s) incomplete packet, expected %zu, got %zd", ntpHost, sizeof(ntp_packet), recvLen);
                close(sockfd);
                sockfd = -1;
                continue;
            }

            // 成功收到响应，跳出循环
            break;
        }

        if (rp == NULL) {
            // 所有地址都尝试失败
            LOG_Error("NetNTPTimeGet(%s) failed", ntpHost);
            break;
        }

        // 转换网络字节序并计算 Unix 时间戳
        packet.txTm_s = ntohl(packet.txTm_s);
        if (packet.txTm_s < NTP_TIMESTAMP_DELTA) {
            LOG_Error("NTP txTm_s(%u) underflow, invalid timestamp", packet.txTm_s);
            break;
        }
        if (ntpTime)
            *ntpTime = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);

        ret = 0;   // 成功
    } while (0);

    // 清理资源
    if (res != NULL)
        freeaddrinfo(res);
    if (sockfd >= 0)
        close(sockfd);

    return ret;
}

/**
 * @brief 检查网络是否已就绪
 *
 * 调用 GetNetStatusInfo 查询网络拨号状态，检查公网 APN（privateNet == 0）且 dialStatus == 1。
 *
 * @return int32_t 0 表示网络已就绪，非 0 表示未就绪
 */
static int32_t IsNetworkReady(void)
{
    int32_t ret = -1;
    int32_t netReady = -1;
    StrNetStatus_t netStatus = {0};
    uint8_t i = 0u;
 
    do
    {
        ret = GetNetStatusInfo(&netStatus, 2000);
        if (ret != 0)
        {
            break;
        }

        for (i = 0U; i < netStatus.apnNum; i++)
        {
            if (netStatus.netStatusInfo[i].privateNet == 0U)
            {
                if (netStatus.netStatusInfo[i].dialStatus == 1U)
                {
                    netReady = 0;
                    break;
                }
            }
        }
    } while (0);
    
    return netReady;
}

/**
 * @brief NTP 时间同步线程主函数
 *
 * 循环执行：
 * 1. 检查当前授时源优先级是否低于 NTP（或相同但即将过期）
 * 2. 检查网络是否就绪（IsNetworkReady）
 * 3. 调用 NetNTPTimeGet 从配置的 NTP 服务器获取时间
 * 4. 成功则发送 E_TIME_SYNC_SRC_NTP 事件到 Manage 线程；失败则轮询下一服务器
 *
 * @param[in] user_data 指向 TimeSrvCtx_t 的指针
 * @return void* 始终返回 NULL
 */
static void* TimeSyncNtpTaskThreadFunc(void* user_data)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*)user_data;
    time_t ntpTimeStamp = 0;
    int hostIndex = 0;
    int ret = 0;

    TimeSyncTaskThreadData_t* threadData = (TimeSyncTaskThreadData_t*)ctx->ntpTaskThreadData;

    while(g_atomic_int_get(&threadData->taskQuitFlag) == 0)
    {
        if (ctx->config.ntpCfg.hostNum <= 0){
            LOG_Error("No Valid Ntp host");
            sleep(5);
            continue;
        }

        /** 休眠 **/
        if(g_atomic_int_get(&ctx->pmState) != PM_STATE_NORMAL){
            sleep(1);
            continue;
        }        

        /* 当前 授时源 优先级 低于 Ntp 授时 优先级
         * 或者 Ntp 授时即将过期，重新开始授时 */
        if(g_atomic_int_get(&ctx->priority) < ctx->config.ntpCfg.priorityActivation.priority || 
            (g_atomic_int_get(&ctx->priority) == ctx->config.ntpCfg.priorityActivation.priority && g_atomic_int_get(&ctx->timeSyncSourceAboutToExpire)))
        {
            /** 网络已就绪 **/
            if (IsNetworkReady() == 0){
    
                ntpTimeStamp = 0;
                ret = NetNTPTimeGet(ctx->config.ntpCfg.host[hostIndex].name, &ntpTimeStamp);
                if(ret == 0 && ntpTimeStamp != 0)
                {
                    LOG_Warn("NetNTPTimeGet(%s) success, ntpTimeStamp=%ld", ctx->config.ntpCfg.host[hostIndex].name, ntpTimeStamp);
                    MsgQueueSend(&ctx->timeSyncEventQueue, E_TIME_SYNC_SRC_NTP, &ntpTimeStamp, sizeof(time_t));  
                }
                else
                {
                    LOG_Error("NetNTPTimeGet(%s) failed", ctx->config.ntpCfg.host[hostIndex].name);
                    hostIndex = (hostIndex + 1) % ctx->config.ntpCfg.hostNum;
                }
            }
        }

        sleep(10);
    }

    return NULL;
}

/**
 * @brief 初始化 NTP 时间同步线程
 *
 * 分配线程控制数据结构，创建 GLib 线程。
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功，-1 表示失败
 */
int InitTimeSyncNtpTask(void* arg)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*)arg;
    TimeSyncTaskThreadData_t* threadData = NULL;

    if(arg == NULL)
    {
        LOG_Error("input empty is arg");
        return -1;
    }

    threadData = (TimeSyncTaskThreadData_t*)malloc(sizeof(TimeSyncTaskThreadData_t));
    if (threadData == NULL) {
        LOG_Error("ntpTaskThreadData malloc failed");
        return -1;
    }
    threadData->taskQuitFlag = 0;
    threadData->taskThread = NULL;
    ctx->ntpTaskThreadData = threadData;
        
    threadData->taskThread = g_thread_new("time_sync_by_ntp", TimeSyncNtpTaskThreadFunc, (void*) arg);
    if (threadData->taskThread == NULL)
    {
        LOG_Error("Create TimeSyncNtpTaskThread failed");
        free(threadData);
        ctx->ntpTaskThreadData = NULL;
        return -1;
    }

    return 0;
}

/**
 * @brief 释放 NTP 时间同步线程
 *
 * 设置线程退出标志，等待线程结束，释放资源。
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功
 */
int ReleaseTimeSyncNtpTask(void* arg)
{
    TimeSrvCtx_t* ctx = (TimeSrvCtx_t*)arg;
    TimeSyncTaskThreadData_t* threadData = NULL;

    if (ctx == NULL) {
        return 0;
    }

    threadData = (TimeSyncTaskThreadData_t*)ctx->ntpTaskThreadData;
    if (threadData == NULL) {
        return 0;
    }

    g_atomic_int_set(&threadData->taskQuitFlag, 1);

    if (threadData->taskThread != NULL){
        g_thread_join(threadData->taskThread);
        g_thread_unref(threadData->taskThread);
        threadData->taskThread = NULL;
    }

    free(threadData);
    ctx->ntpTaskThreadData = NULL;

    return 0;
}
