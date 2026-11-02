
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "crcTool.h"
#include "can_file_save.h"
#include "file_interface.h"
#include "ethernet_interface.h"
#include "sr_log.h"
#include "timer_interface.h"
#include <algorithm>
#include <fcntl.h>
#include "kfifo.h"
#include "thread_interface.h"
#include "socket_interface.h"
#include "system_interface.h"
#include "completion_timeout.h"
using namespace std;

typedef enum
{
    C_STATE_CONNECT = 0,
    C_STATE_SEND_REQ,
    C_STATE_WAIT_REQ_ACK,
    C_STATE_RECV_LOOP,
    C_STATE_EXIT
} ClientState_e;

#define TWO_MINUTES_MS (2 * 60 * 1000*1000) //2分钟数据

void *TcpClientThread(void *arg);
void *HandleBackupCanToAsc(void *arg);
// 线程安全的队列类
void SafeQueue::enqueue(const std::vector<unsigned char> &data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(data);
}

bool SafeQueue::dequeue(std::vector<unsigned char> &data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty())
    {
        return false;
    }
    data = queue_.front();
    queue_.pop();
    return true;
}

void SafeQueue::canenqueue(const std::vector<unsigned char> &candata)
{
    std::lock_guard<std::mutex> lock(canmutex_);
    canqueue_.push(candata);
}

bool SafeQueue::candequeue(std::vector<unsigned char> &candata)
{
    std::lock_guard<std::mutex> lock(canmutex_);
    if (canqueue_.empty())
    {
        return false;
    }
    candata = canqueue_.front();
    canqueue_.pop();
    return true;
}

bool SafeQueue::getcandequeue()
{

    if (canqueue_.empty())
    {
        return false;
    }

    return true;
}

int UDPInit(int &serverSock)
{
#if 1
    struct sockaddr_in serverAddr;
    int opt = 1;
    // 创建套接字
    if ((serverSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        LOG_Error("Socket creation failed");
        return -1;
    }

    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        LOG_Error("setsockopt failed");
        close(serverSock);
        return -1;
    }
    // 设置服务器地址
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    // 绑定套接字
    if (bind(serverSock, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        LOG_Error("Bind failed");
        close(serverSock);
        return -1;
    }
    return 0;
#endif

#if 0
    struct sockaddr_in serverAddr;
    struct ip_mreq mreq;
    int opt = 1;

    /* 创建套接字 */
    serverSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSock < 0)
    {
        LOG_Error("Socket creation failed");
        return -1;
    }

    /* 允许端口复用 */
    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0)
    {
        LOG_Error("setsockopt SO_REUSEADDR failed");
        close(serverSock);
        return -1;
    }

    /* 绑定地址和端口 */
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(serverSock,
             (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0)
    {
        LOG_Error("Bind failed");
        close(serverSock);
        return -1;
    }

    /* 加入组播组 */
    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr("239.255.0.1");   // 组播地址
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);          // 默认网卡

    if (setsockopt(serverSock,
                   IPPROTO_IP,
                   IP_ADD_MEMBERSHIP,
                   &mreq,
                   sizeof(mreq)) < 0)
    {
        LOG_Error("setsockopt IP_ADD_MEMBERSHIP failed");
        close(serverSock);
        return -1;
    }

    LOG_Debug("UDP multicast join success");
#endif
}

// 接收固定长度数据并将其放入队列的函数
void handleServer(SafeQueue &queue)
{
    unsigned char buffer[DATA_LENGTH] = {0};
    ssize_t bytesReceived = 0;
    int serverSock = 0;
    int ret = 0;
    int retryCount = 0;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    do
    {
        ret = UDPInit(serverSock);
        if (ret < 0)
        {
            LOG_Error("UDP Init Fail");
        }
        retryCount++;
        if (retryCount >= 5)
        {
            LOG_Error("Exceeded maximum retry attempts. Exiting.");
            break;
        }

    } while (ret != 0);

    while (1)
    {
        LOG_Debug("Waiting for client connection...");
        while (true)
        {

            memset(buffer, 0, sizeof(buffer));
            bytesReceived = recvfrom(serverSock, buffer, DATA_LENGTH, 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
            if (bytesReceived < 0)
            {
                LOG_Error("recvfrom failed with error: %s  bytesReceived = %zd", strerror(errno), bytesReceived);
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                {
                    continue;
                }
                close(serverSock);
                UDPInit(serverSock);
                break;
            }
            // LOG_Debug("recv bytesReceived = %d",bytesReceived);
            std::vector<unsigned char> data(buffer, buffer + bytesReceived);
            queue.enqueue(data);
        }
    }
    close(serverSock);
}

void SubcontractingHandleQueue(SafeQueue &queue)
{
    static int flag = 0;
    while (1)
    {
        // 主线程读取队列中的数据并打印
        DelayMS(1);
        std::vector<unsigned char> dataBuffer;
        if (queue.dequeue(dataBuffer))
        {
#if 0
            LOG_DebugHex(dataBuffer, dataBuffer.size(), "MCU send DATA");
#endif
            if (dataBuffer[0] == 0x55 && dataBuffer[1] == 0xaa)
            {
                uint16_t srvLen = ((dataBuffer[5] & 0xff) << 8) + (dataBuffer[6] & 0xff);
                
                // uint16_t crc16Val = ((dataBuffer[srvLen + 8] & 0xff ) << 8) + (dataBuffer[srvLen + 9] & 0xff);
                uint16_t crc16Val = ((dataBuffer[dataBuffer.size() - 2] & 0xff) << 8) + (dataBuffer[dataBuffer.size() - 1] & 0xff);
                uint16_t crcval = Crc16((uint8_t *)&dataBuffer[0], srvLen  + 8, Crc16Ccitt);

                if (crc16Val == Crc16((uint8_t *)&dataBuffer[0], srvLen  + 8, Crc16Ccitt))
                {
                    // LOG_Debug("CRC Ok");
                }
                else
                {
                    LOG_Error("CRC FAIL mcu crc:%x mpu crc:%x ", crc16Val, crcval);
                    continue;
                }
                srvLen =srvLen - 8;
                uint64_t tmp = 0;
                memcpy(&tmp, &dataBuffer[8], 8);
                uint64_t mcutime = be64toh(tmp);
                if (GetAppContext()->modem_type == MAIN_MODEM)
                {
                    if (flag == 0)
                    {
                        AppContext_t *ctx = GetAppContext();
                        ctx->time_range.end_time = MCU_TIME_TO_US(mcutime);
                        flag = 1;
                        pthread_t threadId;
                        CreateDetachedThread(&threadId, TcpClientThread, &ctx->time_range);
                        pthread_t writeid;
                        CreateDetachedThread(&writeid, HandleBackupCanToAsc, NULL);

                        LOG_Debug("canlog: CreateDetachedThread TcpClientThread HandleBackupCanToAsc");
                    }
                }
                if (srvLen % 15 != 0)
                {
                    LOG_Error("srvLen %% 15 != 0");
                }
                for (int i = 0; i < srvLen / 15; i++)
                {
                    std::vector<unsigned char> Buffer;
                    Buffer.resize(50);
                    int  off= (uint64_t)i * FRAME_INTERVAL_US;
                    off = (off > 1000) ? 1000 : off;
                    long long time = MCU_TIME_TO_US(mcutime) + off;
                    memcpy(&(Buffer[0]), &time, 8);
                    memcpy(&(Buffer[8]), &(dataBuffer[(i * 15 + 8 + 8)]), 15);
                    queue.canenqueue(Buffer);
                }
            }
            else
            {
                LOG_Error("data head incorrect...");
            }
        }
    }
}

void WriteHandle(SafeQueue &queue)
{
    Can_frame_t *canframe = (Can_frame_t *)malloc(sizeof(Can_frame_t) * (MAX_FRAME_NUM + 1));
    if (canframe == NULL)
    {
        LOG_Error("malloc canframe failed");
        return;
    }
    int ret = 0;
    int count = 0;
    int time_sync_done = 0; /* 标记时间同步是否已完成 */

    while (1)
    {
        DelayMS(WRITE_DELAY_MS);

        memset(canframe, 0, sizeof(Can_frame_t) * (MAX_FRAME_NUM + 1));

        for (int i = 0; i < MAX_FRAME_NUM; i++)
        {
            std::vector<unsigned char> dataBuffer;
            if (queue.candequeue(dataBuffer))
            {
                // 暂定can 如果can fd 需要mcu 修改
                canframe[i].timestamp = 0;
                memcpy(&(canframe[i].timestamp), &(dataBuffer[0]), 8);
                canframe[i].channel = dataBuffer[8];
                canframe[i].can_id = (dataBuffer[10] << 24) | (dataBuffer[11] << 16) | (dataBuffer[12] << 8) | dataBuffer[13];
                canframe[i].direction = dataBuffer[9];
                canframe[i].dlc = dataBuffer[14];
                memcpy(canframe[i].data, &dataBuffer[15], 8);
                count++;
            }
            else
            {
                break;
            }
        }

        if (count > 0)
        {
            /* 等待系统时间同步后再写文件（只执行一次） */
            if (!time_sync_done)
            {
                int wait_count = 0;
                const int max_wait_count = 200; /* 最长等待 20 秒 (200 * 0.1s) */

                while (wait_count < max_wait_count)
                {
                    struct timeval tv;
                    gettimeofday(&tv, NULL);
                    time_t sys_seconds = tv.tv_sec;
                    struct tm tm_info;
                    localtime_r(&sys_seconds, &tm_info);

                    /* 检查系统时间是否已同步 (tm_year >= 120 表示 2020 年及以后) */
                    if (tm_info.tm_year < 120)
                    {
                        LOG_Debug("WriteHandle: system time not sync yet (year=%d), waiting... (wait %d/%d)",
                                  tm_info.tm_year + 1900, wait_count, max_wait_count);
                        DelayMS(100);
                        wait_count++;
                        continue;
                    }

                    /* 系统时间已同步，跳出等待循环 */
                    LOG_Debug("WriteHandle: system time OK (year=%d), proceed", tm_info.tm_year + 1900);
                    DelayMS(120);
                    break;
                }

                if (wait_count >= max_wait_count)
                {
                    LOG_Error("WriteHandle: wait system time sync timeout (20s), force proceed");
                }
                else if (wait_count > 0)
                {
                    LOG_Debug("WriteHandle: waited %d times (%.1f sec) for time sync", wait_count, wait_count * 0.1);
                }

                /* 系统时间同步后，强制重置文件上下文，确保用正确时间创建文件 */
                AppContext_t *ctx = GetAppContext();
                if (ctx->main_file_ctx.current_file)
                {
                    PthreadMutexLock();
                    fflush(ctx->main_file_ctx.current_file);
                    fclose(ctx->main_file_ctx.current_file);
                    ctx->main_file_ctx.current_file = NULL;
                    PthreadMutexUnLock();
                    LOG_Debug("WriteHandle: close old file with wrong timestamp");
                }
                memset(ctx->main_file_ctx.bkfilename, 0, sizeof(ctx->main_file_ctx.bkfilename));

                /* 将第一帧时间戳替换为 MPU 系统时间 */
                struct timeval tv;
                gettimeofday(&tv, NULL);
                canframe[0].timestamp = (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec;
                LOG_Debug("WriteHandle: replace first frame timestamp to MPU time: %lu us", canframe[0].timestamp);

                time_sync_done = 1;
            }

            AppContext_t *ctx = GetAppContext();
            ret = WriteDataWithRotation(&ctx->main_file_ctx, canframe, count);
            if (ret < 0)
            {
                LOG_Error("Write Data With Rotation fail");
            }
            LOG_Debug("Write Data With Rotation Success!");
            count = 0;
        }
    }
    if (canframe)
    {
        free(canframe);
        canframe = NULL;
    }
}

int EthernetInit()
{
    SafeQueue queue;
    std::vector<std::thread> threads;

    threads.emplace_back(handleServer, std::ref(queue));
    threads.emplace_back(SubcontractingHandleQueue, std::ref(queue));
    threads.emplace_back(WriteHandle, std::ref(queue));
    while (1)
    {
        sleep(10);
    }

    // 等待所有线程完成
    for (auto &t : threads)
    {
        t.join();
    }

    return 0;
}

int protocol_recv_header(int fd, PacketHeader_t *hdr, int timeout_ms)
{
    int len = 0;

    if (!hdr)
        return -1;

    memset(hdr, 0, sizeof(PacketHeader_t));

    if (stream_rcvtmo(fd, hdr, sizeof(PacketHeader_t), &len, timeout_ms) < 0 ||
        len <= 0 || (size_t)len != sizeof(PacketHeader_t))
    {
        LOG_Error("recv header failed, len=%d\n", len);
        return -1;
    }

    hdr->frame_count = ntohs(hdr->frame_count);
    hdr->data_len = ntohs(hdr->data_len);

    return 0;
}


static int protocol_recv_can_data(int fd, const PacketHeader_t *hdr, int timeout_ms)
{
    if (!hdr || hdr->frame_count == 0)
        return RECV_ERROR;

    size_t expected = (size_t)hdr->frame_count * sizeof(Can_frame_t);
    if (expected != hdr->data_len || hdr->data_len > MAX_DATA_BUF_SIZE)
    {
        LOG_Error("invalid data len: expected=%zu, received=%u\n", expected, hdr->data_len);
        return RECV_ERROR;
    }

   
    uint8_t buf[hdr->data_len];
    int remaining = hdr->data_len;
    uint8_t *buf_ptr = buf;
    int len = 0;
    while (remaining > 0)
    {

        int ret = stream_rcvtmo(fd, buf_ptr, remaining, &len, timeout_ms);

        if (ret == 0 || ret == -1)
        {
            break;
        }
        if (len > 0 && ret == -2)
        {
            // 读到了一部分数据，更新指针继续读剩余数据
            buf_ptr += len;
            remaining -= len;
            LOG_Debug(" read: len %d, remaining %d, retrying...", len, remaining);
            continue; // 继续循环读剩下的
        }

        LOG_Debug("recv body fail: ret=%d, got %d, expected %u\n", ret, len, hdr->data_len);
        return RECV_ERROR;
    }

    AppContext_t *ctx = GetAppContext();
    int written = kfifo_in(&ctx->can_fifo, buf, hdr->data_len);
    if (written != (int)hdr->data_len)
    {
        LOG_Debug("fifo full or partial write: written=%d, need=%u\n", written, hdr->data_len);
        return RECV_ERROR;
    }

    // 调试日志（按需开启）
    // const Can_frame_t *frames = (const Can_frame_t *)buf;
    // for (uint16_t i = 0; i < hdr->frame_count; i++) {
    //     LOG_Debug("can_id: 0x%X", frames[i].can_id);
    // }

    return RECV_CONTINUE;
}

static int transport_handle_retry(int *retryCnt, int *fd)
{
    (*retryCnt)++;

    if (*retryCnt >= MAX_RETRY_COUNT)
    {
        if (*fd >= 0)
        {
            close(*fd);
            *fd = -1;
        }
        *retryCnt = 0;
        return -1;
    }

    return 0;
}

static int transport_send_simple_packet(int fd, uint8_t type)
{
    int len = 0;
    PacketHeader_t hdr = {0};
    hdr.type = type;
    return stream_sndto(fd, &hdr, sizeof(hdr), &len);
}

static ClientState_e transport_connect(int *fd, int *retryCnt)
{
    *fd = connect_server(TCP_CLIENT_IP, TCP_CLIENT_PORT);

    if (*fd >= 0)
    {
        *retryCnt = 0;
        return C_STATE_SEND_REQ;
    }
    sleep(3);
    return C_STATE_CONNECT;
}

static ClientState_e domain_send_req(int *fd, TimeRange_t *req, int *retryCnt)
{
    int len = 0;
    PacketHeader_t hdr = {0};
    uint8_t buf[256] = {0};

    hdr.type = TYPE_SEND_ACK; // TYPE_SEND_ACK
    hdr.frame_count = htons(0);
    hdr.data_len = htons(sizeof(TimeRange_t));
    SetBackupFinishFlag(BACKUP_NOT_FINISH);
    memcpy(buf, &hdr, sizeof(hdr));
    memcpy(buf + sizeof(hdr), req, sizeof(TimeRange_t));
    LOG_Debug("start time =%lu end tim =%lu", req->start_time, req->end_time);
    if (stream_sndto(*fd, buf, sizeof(hdr) + sizeof(TimeRange_t), &len) < 0)
    {
        if (transport_handle_retry(retryCnt, fd) < 0)
            return C_STATE_CONNECT;

        return C_STATE_SEND_REQ;
    }

    *retryCnt = 0;
    return C_STATE_WAIT_REQ_ACK;
}

static ClientState_e domain_wait_req_ack(int *fd, PacketHeader_t *hdr, int *retryCnt)
{
    if (protocol_recv_header(*fd, hdr, DEFAULT_ACK_TIMEOUT) < 0 || hdr->type != TYPE_RECV_ACK)
    {
        if (transport_handle_retry(retryCnt, fd) < 0)
            return C_STATE_CONNECT;

        return C_STATE_WAIT_REQ_ACK;
    }

    *retryCnt = 0;
    return C_STATE_RECV_LOOP;
}

static ClientState_e domain_recv_loop(int *fd, PacketHeader_t *hdr, int *retryCnt)
{
    if (protocol_recv_header(*fd, hdr, DEFAULT_RECV_TIMEOUT) < 0)
    {
        if (transport_handle_retry(retryCnt, fd) < 0)
            return C_STATE_CONNECT;

        return C_STATE_RECV_LOOP;
    }

    if (hdr->type == TYPE_RECV_CAN_DATA)
    {
        if (protocol_recv_can_data(*fd, hdr, DEFAULT_RECV_TIMEOUT) == RECV_ERROR)
        {
            if (transport_handle_retry(retryCnt, fd) < 0)
                return C_STATE_CONNECT;
            return C_STATE_RECV_LOOP;
        }

        if (transport_send_simple_packet(*fd, TYPE_RECV_CAN_DATA_ACK) < 0) // ACK
        {
            if (transport_handle_retry(retryCnt, fd) < 0)
                return C_STATE_CONNECT;
        }
        return C_STATE_RECV_LOOP;
    }
    else if (hdr->type == TYPE_SEND_END)
    {
        if (transport_send_simple_packet(*fd, TYPE_SEND_RECV_ACK) < 0)
        {
            if (transport_handle_retry(retryCnt, fd) < 0)
                return C_STATE_CONNECT;

            return C_STATE_RECV_LOOP;
        }
        LOG_Debug("type=%d", hdr->type);
        SetBackupFinishFlag(BACKUP_FINISH);
        close(*fd);
        *fd = -1;
        return C_STATE_EXIT;
    }

    LOG_Error("unknown type=%d", hdr->type);
    if (transport_handle_retry(retryCnt, fd) < 0)
        return C_STATE_CONNECT;

    return C_STATE_RECV_LOOP;
}

void *TcpClientThread(void *arg)
{
    TimeRange_t *req = (TimeRange_t *)arg;

    int fd = -1;
    int retryCnt = 0;
    PacketHeader_t hdr;
    if (req->start_time > req->end_time || req->end_time >= req->start_time + TWO_MINUTES_MS)
    {
        req->start_time = req->end_time - TWO_MINUTES_MS;
    }
    ClientState_e state = C_STATE_CONNECT;

    while (state != C_STATE_EXIT)
    {
        switch (state)
        {
        case C_STATE_CONNECT:
            state = transport_connect(&fd, &retryCnt);
            break;

        case C_STATE_SEND_REQ:
            state = domain_send_req(&fd, req, &retryCnt);
            break;

        case C_STATE_WAIT_REQ_ACK:
            state = domain_wait_req_ack(&fd, &hdr, &retryCnt);
            break;

        case C_STATE_RECV_LOOP:
            state = domain_recv_loop(&fd, &hdr, &retryCnt);
            break;

        default:
            state = C_STATE_EXIT;
            break;
        }
    }

    if (fd >= 0)
        close(fd);
    return NULL;
}

void *HandleBackupCanToAsc(void *arg)
{
    Can_frame_t *canframe = (Can_frame_t *)malloc(sizeof(Can_frame_t) * (MAX_FRAME_NUM + 1));
    if (canframe == NULL)
    {
        LOG_Error("malloc canframe failed");
        return NULL;
    }

    int ret = 0;
    int count = 0;
    int time_sync_done = 0; /* 标记时间同步是否已完成 */
    AppContext_t *ctx = GetAppContext();
    while (1)
    {
        count = 0;
        /* 等待数据，或者判断退出 */
        while (kfifo_len(&ctx->can_fifo) < sizeof(Can_frame_t))
        {
            if (GetBackupFinishFlag() == BACKUP_FINISH)
            {
                LOG_Debug("fifo empty and recv finished, thread exit");
                //free(canframe);
                break;
                //return NULL;
            }

            DelayMS(1000);
        }

        /* 尽量取空FIFO */
        while (count < MAX_FRAME_NUM)
        {
            if (kfifo_out(&ctx->can_fifo, &canframe[count], sizeof(Can_frame_t)) != sizeof(Can_frame_t))
            {
                break;
            }

            count++;
        }

        if (count > 0)
        {
            /* 等待系统时间同步后再写文件（只执行一次） */
            if (!time_sync_done)
            {
                int wait_count = 0;
                const int max_wait_count = 200; /* 最长等待 20 秒 (200 * 0.1s) */

                while (wait_count < max_wait_count)
                {
                    struct timeval tv;
                    gettimeofday(&tv, NULL);
                    time_t sys_seconds = tv.tv_sec;
                    struct tm tm_info;
                    localtime_r(&sys_seconds, &tm_info);

                    /* 检查系统时间是否已同步 (tm_year >= 120 表示 2020 年及以后) */
                    if (tm_info.tm_year < 120)
                    {
                        LOG_Debug("HandleBackupCanToAsc: system time not sync yet (year=%d), waiting... (wait %d/%d)",
                                  tm_info.tm_year + 1900, wait_count, max_wait_count);
                        DelayMS(100);
                        wait_count++;
                        continue;
                    }

                    /* 系统时间已同步，跳出等待循环 */
                    LOG_Debug("HandleBackupCanToAsc: system time OK (year=%d), proceed", tm_info.tm_year + 1900);
                    break;
                }

                if (wait_count >= max_wait_count)
                {
                    LOG_Error("HandleBackupCanToAsc: wait system time sync timeout (20s), force proceed");
                }
                else if (wait_count > 0)
                {
                    LOG_Debug("HandleBackupCanToAsc: waited %d times (%.1f sec) for time sync", wait_count, wait_count * 0.1);
                }

                /* 系统时间同步后，强制重置文件上下文，确保用正确时间创建文件 */
                if (ctx->backup_file_ctx.current_file)
                {
                    PthreadMutexLock();
                    fflush(ctx->backup_file_ctx.current_file);
                    fclose(ctx->backup_file_ctx.current_file);
                    ctx->backup_file_ctx.current_file = NULL;
                    PthreadMutexUnLock();
                    LOG_Debug("HandleBackupCanToAsc: close old file with wrong timestamp");
                }
                memset(ctx->backup_file_ctx.bkfilename, 0, sizeof(ctx->backup_file_ctx.bkfilename));

                /* 将第一帧时间戳替换为 MPU 系统时间 */
                struct timeval tv;
                gettimeofday(&tv, NULL);
                canframe[0].timestamp = (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec;
                LOG_Debug("HandleBackupCanToAsc: replace first frame timestamp to MPU time: %lu us", canframe[0].timestamp);

                time_sync_done = 1;
            }

            ret = WriteDataWithRotation(&ctx->backup_file_ctx, canframe, count);
            if (ret < 0)
            {
                LOG_Error("WriteDataWithRotation fail");
            }
        }

        /* 写完最后一批后退出 */
        if (kfifo_len(&ctx->can_fifo) == 0 && GetBackupFinishFlag() == BACKUP_FINISH)
        {
            LOG_Debug("all backup data write finish, thread exit");
            break;
        }
    }
    if(canframe)
    {
        free(canframe);
    }
    LOG_Debug(" HandleBackupCanToAsc end");
     /* 关闭备份文件，确保数据完整写入磁盘 */
    if (ctx->backup_file_ctx.current_file)
    {
        PthreadMutexLock();
        fflush(ctx->backup_file_ctx.current_file);
        fclose(ctx->backup_file_ctx.current_file);
        ctx->backup_file_ctx.current_file = NULL;
        PthreadMutexUnLock();
    }
    /* 压缩 backup_file_ctx 写入的最后一个文件 */
    if (ctx->backup_file_ctx.bkfilename[0] != '\0')
    {
        LOG_Debug("HandleBackupCanToAsc end, compressing file: %s", ctx->backup_file_ctx.bkfilename);
        ret = TarGzCompressSingleFile(ctx->backup_file_ctx.bkfilename);
        if (ret < 0)
        {
            LOG_Error("TarGzCompressSingleFile Fail for %s", ctx->backup_file_ctx.bkfilename);
        }
    }
    else
    {
        LOG_Debug("HandleBackupCanToAsc end, no file to compress");
    }
    return NULL;
}
