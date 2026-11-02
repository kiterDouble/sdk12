#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "crcTool.h"
#include "can_file_save.h"
#include "file_interface.h"
#include "ethernet_interface.h"
#include "sr_log.h"
#include <algorithm>
#include "kfifo.h"
#include "thread_interface.h"
#include "timer_interface.h"
#include "socket_interface.h"
#include "system_interface.h"
#include <cinttypes>

#define MAX_SEND_FRAMES 1024 // 一次批量发送的最大 CAN 帧数

typedef enum
{
    STATE_WAIT_REQ = 0,
    STATE_SEND_REQ_ACK,
    STATE_SEND_CAN_DATA,
    STATE_WAIT_CAN_ACK,
    STATE_SEND_END,
    STATE_WAIT_END_ACK,
    STATE_DISCONNECT
} ServerState_e;

typedef struct
{
    uint32_t elem_size;   /* 元素大小 */
    uint64_t target_time; /* 目标时间戳 */
    int find_greater;     /* 1: 查找大于等于目标时间的元素, 0: 查找大于目标时间的元素 */
} TimeCompareContext_t;

/* 全局变量：记录已发送但未确认的数据大小（字节数） */
static uint32_t g_pending_fifo_out_size = 0;
void SubmoduleHandleQueue(SafeQueue &queue)
{
    Can_frame_t canframe;
    std::vector<unsigned char> dataBuffer;

    while (1)
    {
        if (!queue.dequeue(dataBuffer))
        {
            DelayMS(5);
            continue;
        }

#if 0
        LOG_DebugHex(dataBuffer, dataBuffer.size(), "MCU send DATA");
#endif

        /* 基础长度校验 */
        if (dataBuffer.size() < 10)
        {
            LOG_Error("data too short");
            continue;
        }

        /* 包头校验 */
        if (dataBuffer[0] != 0x55 || dataBuffer[1] != 0xaa)
        {
            LOG_Error("data head incorrect");
            continue;
        }

        uint16_t srvLen = ((dataBuffer[5] & 0xFF) << 8) | (dataBuffer[6] & 0xFF);

        /* 长度校验 */
        if (dataBuffer.size() < (size_t)(srvLen + 10))
        {
            LOG_Error("data size invalid, srvLen=%u size=%zu",
                      srvLen, dataBuffer.size());
            continue;
        }

        /* CRC校验 */
        uint16_t crc16Val = ((dataBuffer[dataBuffer.size() - 2] & 0xFF) << 8) | (dataBuffer[dataBuffer.size() - 1] & 0xFF);

        uint16_t crcval = Crc16((uint8_t *)&dataBuffer[0], srvLen  + 8, Crc16Ccitt);

        if (crc16Val != crcval)
        {
            LOG_Error("CRC FAIL mcu crc:%x mpu crc:%x", crc16Val, crcval);
            continue;
        }

        srvLen = srvLen -8;
        /* 数据长度必须为15整数倍 */
        if (srvLen % 15 != 0)
        {
            LOG_Error("srvLen %% 15 != 0");
            continue;
        }

    
        uint64_t tmp = 0;
        memcpy(&tmp, &dataBuffer[8], 8);
        uint64_t mcutime = be64toh(tmp);
        int frameCnt = srvLen / 15;
        const uint8_t *payload = &dataBuffer[8 + 8];
        AppContext_t *ctx = GetAppContext();
        if (COLLECT_STATUS_RUNNING == GetCollectStatus())
        {
            for (int i = 0; i < frameCnt; i++)
            {
                const uint8_t *frame = payload + i * 15;
                memset(&canframe, 0, sizeof(canframe));
                canframe.can_id = (frame[2] << 24) | (frame[3] << 16) | (frame[4] << 8) | frame[5];
                int  off= (uint64_t)i * FRAME_INTERVAL_US;
                off = (off > 1000) ? 1000 : off;
                canframe.timestamp = MCU_TIME_TO_US(mcutime) + off;
                canframe.channel = frame[0];
                canframe.direction = frame[1];
                canframe.dlc = frame[6];
                memcpy(canframe.data, &frame[7], 8);
                int ret = kfifo_in(&ctx->can_fifo, &canframe, sizeof(canframe));
                if (ret != sizeof(canframe))
                {
                    LOG_Error("kfifo full");
                }
            }
        }
    }
}

static ServerState_e server_handle_wait_req(int fd, TimeRange_t *req)
{
    PacketHeader_t hdr;
    int len;
    if (protocol_recv_header(fd, &hdr, DEFAULT_RECV_TIMEOUT) < 0)
    {
        LOG_Error("recv req hdr fail\n");
        return STATE_DISCONNECT;
    }

    if (hdr.type != TYPE_SEND_ACK)
    {
        LOG_Error("invalid req type=%d, expected=%d\n", hdr.type, TYPE_SEND_ACK);
        return STATE_DISCONNECT;
    }

    if (hdr.data_len > 0 && hdr.data_len < 256)
    {
        uint8_t body[256];

        if (stream_rcvtmo(fd, body, hdr.data_len, &len, DEFAULT_RECV_TIMEOUT) < 0)
        {
            LOG_Error("recv body fail\n");
            return STATE_DISCONNECT;
        }

        if (len < 0 || (size_t)len < sizeof(TimeRange_t))
        {
            LOG_Error("invalid len=%d, expected>=%zu\n", len, sizeof(TimeRange_t));
            return STATE_DISCONNECT;
        }

        memcpy(req, body, sizeof(TimeRange_t));
        LOG_Debug("recv req: start_time=%" PRIu64 ", end_time=%" PRIu64 "\n", req->start_time, req->end_time);
    }
    else
    {
        LOG_Error("invalid data_len=%u, expected 0<data_len<256\n", hdr.data_len);
        return STATE_DISCONNECT;
    }

    SetCollectStatus(COLLECT_STATUS_STOP);
    LOG_Debug("stop collect, ready to send data\n");
    return STATE_SEND_REQ_ACK;
}

static int server_send_with_retry(int fd, const void *data, int size, int *retryCnt)
{
    int len;
    while (1)
    {
        if (stream_sndto(fd, data, size, &len) == 0)
        {
            *retryCnt = 0;
            return 0;
        }

        (*retryCnt)++;
        LOG_Debug("send fail, retry %d/%d\n", *retryCnt, MAX_RETRY_COUNT);

        if (*retryCnt >= MAX_RETRY_COUNT)
        {
            *retryCnt = 0;
            return -1;
        }
        usleep(1000);
    }
    return 0;
}

static int server_recv_with_retry(int fd, PacketHeader_t *hdr, int *retryCnt, uint8_t expectedType)
{
    if (protocol_recv_header(fd, hdr, DEFAULT_ACK_TIMEOUT) < 0)
    {
        (*retryCnt)++;
        LOG_Debug("recv timeout, retry %d/%d, expected type=%d\n", *retryCnt, MAX_RETRY_COUNT, expectedType);
        if (*retryCnt >= MAX_RETRY_COUNT)
        {
            LOG_Error("recv timeout after %d retries, expected type=%d\n", MAX_RETRY_COUNT, expectedType);
            *retryCnt = 0;
            return -2;
        }
        return -1;
    }

    if (hdr->type != expectedType)
    {
        LOG_Error("invalid ack type=%d, expected=%d\n", hdr->type, expectedType);
        return -2;
    }

    LOG_Debug("recv ack type=%d success\n", hdr->type);
    *retryCnt = 0;
    return 0;
}

static ServerState_e server_handle_send_req_ack(int fd, int *retryCnt)
{
    PacketHeader_t ack = {.type = TYPE_RECV_ACK};

    if (server_send_with_retry(fd, &ack, sizeof(ack), retryCnt) < 0)
    {
        LOG_Error("send req ack fail after %d retries\n", MAX_RETRY_COUNT);
        return STATE_DISCONNECT;
    }
    LOG_Debug("send req ack success, ready to send can data\n");
    return STATE_SEND_CAN_DATA;
}

static int compare_timestamp(const void *elem, void *context)
{
    TimeCompareContext_t *ctx = (TimeCompareContext_t *)context;
    Can_frame_t *frame = (Can_frame_t *)elem;

    if (ctx->find_greater)
    {
        /* 查找 timestamp >= target_time 的元素 */
        return (frame->timestamp >= ctx->target_time) ? 1 : 0;
    }
    else
    {
        /* 查找 timestamp > target_time 的元素 */
        return (frame->timestamp > ctx->target_time) ? 1 : 0;
    }
}

/* ====== 辅助函数：调试打印 FIFO 时间戳范围 ====== */
static void debug_print_fifo_range(kfifo_t *fifo, const TimeRange_t *req)
{
    uint32_t fifo_len = kfifo_len(fifo);
    if (fifo_len < sizeof(Can_frame_t))
    {
        LOG_Debug("FIFO is empty or has incomplete frame, len=%u\n", fifo_len);
        return;
    }

    Can_frame_t first_frame = {0};
    Can_frame_t last_frame = {0};
    uint32_t peek_ret;

    peek_ret = kfifo_peek(fifo, &first_frame, sizeof(Can_frame_t));
    if (peek_ret != sizeof(Can_frame_t))
    {
        LOG_Debug("Failed to peek first frame, ret=%u\n", peek_ret);
        return;
    }

    peek_ret = kfifo_peek_at(fifo, &last_frame, sizeof(Can_frame_t), fifo_len - sizeof(Can_frame_t));
    if (peek_ret != sizeof(Can_frame_t))
    {
        LOG_Debug("Failed to peek last frame, ret=%u\n", peek_ret);
        return;
    }

    /* 合法性校验：Unix 微秒时间戳合理范围 2020~2035 (1.5e15 ~ 2.1e15) */
    if (first_frame.timestamp < 1500000000000000ULL ||
        first_frame.timestamp > 2100000000000000ULL) {
        LOG_Error("FIFO first_ts out of range! ts=%lu (0x%lX), fifo_len=%u, fifo_out mod24=%u",
                  first_frame.timestamp, first_frame.timestamp,
                  fifo_len, fifo_len % sizeof(Can_frame_t));
    }
    if (last_frame.timestamp < 1500000000000000ULL ||
        last_frame.timestamp > 2100000000000000ULL) {
        LOG_Error("FIFO last_ts out of range! ts=%lu (0x%lX), fifo_len=%u",
                  last_frame.timestamp, last_frame.timestamp, fifo_len);
    }

    char first_ts_buf[32], last_ts_buf[32], start_buf[32], end_buf[32];
    FormatTimestamp(first_frame.timestamp, first_ts_buf, sizeof(first_ts_buf));
    FormatTimestamp(last_frame.timestamp, last_ts_buf, sizeof(last_ts_buf));
    FormatTimestamp(req->start_time, start_buf, sizeof(start_buf));
    FormatTimestamp(req->end_time, end_buf, sizeof(end_buf));

    LOG_Debug("FIFO range: first_ts=%s, last_ts=%s, req_range=[%s, %s]\n",
              first_ts_buf, last_ts_buf, start_buf, end_buf);
}

/* ====== 辅助函数：从 FIFO 获取帧数据 ====== */
typedef struct {
    uint32_t frame_count;      /* 获取的帧数 */
    uint32_t consumed_bytes;   /* 消耗的字节数 */
    int end_time_reached;      /* 是否到达结束时间 */
} FetchResult_t;

static FetchResult_t fetch_frames_from_fifo(kfifo_t *fifo, Can_frame_t *frames,
                                            uint32_t max_frames, const TimeRange_t *req)
{
    FetchResult_t result = {0};

    TimeCompareContext_t skip_ctx = {
        .elem_size = sizeof(Can_frame_t),
        .target_time = req->start_time,
        .find_greater = 0};
    TimeCompareContext_t stop_ctx = {
        .elem_size = sizeof(Can_frame_t),
        .target_time = req->end_time,
        .find_greater = 0};

    result.frame_count = kfifo_peek_range(fifo, frames, max_frames,
                                          &result.consumed_bytes, &result.end_time_reached,
                                          compare_timestamp, &skip_ctx,
                                          compare_timestamp, &stop_ctx);

    /* 打印跳过的帧数 */
    if (result.consumed_bytes > result.frame_count * sizeof(Can_frame_t))
    {
        uint32_t skipped = result.consumed_bytes - result.frame_count * sizeof(Can_frame_t);
        LOG_Debug("skipped %u bytes (%u frames) before start_time\n",
                  skipped, skipped / (uint32_t)sizeof(Can_frame_t));
    }

    if (result.end_time_reached)
    {
        LOG_Debug("end_time reached\n");
    }

    return result;
}

/* ====== 辅助函数：构建 CAN 数据包 ====== */
typedef struct {
    PacketHeader_t hdr;
    uint8_t *buffer;           /* 数据包缓冲区 */
    size_t total_len;          /* 数据包总长度 */
} PacketBuildResult_t;

static PacketBuildResult_t build_can_data_packet(uint8_t *send_buf, uint32_t frame_count)
{
    PacketBuildResult_t result = {0};

    result.hdr.type = TYPE_RECV_CAN_DATA;
    result.hdr.frame_count = htons((uint16_t)frame_count);
    result.hdr.data_len = htons((uint16_t)(frame_count * sizeof(Can_frame_t)));

    result.buffer = send_buf;
    result.total_len = sizeof(PacketHeader_t) + frame_count * sizeof(Can_frame_t);

    /* 直接在缓冲区头部写入包头 */
    memcpy(send_buf, &result.hdr, sizeof(PacketHeader_t));

    return result;
}

/* ====== 辅助函数：发送 CAN 数据包 ====== */
static ServerState_e send_can_data_packet(int fd, int *retryCnt,
                                          const PacketBuildResult_t *packet,
                                          const FetchResult_t *fetch_result)
{
    LOG_Debug("send can data: %u frames, total %zu bytes (pending pop size: %u)\n",
              fetch_result->frame_count, packet->total_len, fetch_result->consumed_bytes);

    if (server_send_with_retry(fd, packet->buffer, packet->total_len, retryCnt) < 0)
    {
        LOG_Error("send can data fail after %d retries\n", MAX_RETRY_COUNT);
        return STATE_DISCONNECT;
    }

    return fetch_result->end_time_reached ? STATE_SEND_END : STATE_WAIT_CAN_ACK;
}

/* ====== STATE_SEND_CAN_DATA ====== */
static ServerState_e server_handle_send_can_data(int fd, int *retryCnt, const TimeRange_t *req)
{
    AppContext_t *ctx = GetAppContext();

    /* 静态发送缓冲区（约24KB，单线程服务器上下文安全） */
    static uint8_t s_send_buf[sizeof(PacketHeader_t) + MAX_SEND_FRAMES * sizeof(Can_frame_t)];
    Can_frame_t *frames = (Can_frame_t *)(s_send_buf + sizeof(PacketHeader_t));

    /* 调试：打印 FIFO 时间戳范围 */
    debug_print_fifo_range(&ctx->can_fifo, req);

    /* 从 FIFO 获取帧数据 */
    FetchResult_t fetch_result = fetch_frames_from_fifo(&ctx->can_fifo, frames,
                                                        MAX_SEND_FRAMES, req);

    /* 无帧数据，发送 END */
    if (fetch_result.frame_count == 0)
    {
        LOG_Debug("no frames in time range, send END\n");
        return STATE_SEND_END;
    }

    /* 记录待确认数据量 */
    g_pending_fifo_out_size = fetch_result.consumed_bytes;

    /* 构建数据包 */
    PacketBuildResult_t packet = build_can_data_packet(s_send_buf, fetch_result.frame_count);

    /* 发送数据包 */
    ServerState_e next_state = send_can_data_packet(fd, retryCnt, &packet, &fetch_result);
    if (next_state == STATE_DISCONNECT)
    {
        g_pending_fifo_out_size = 0;
    }

    return next_state;
}

static ServerState_e server_handle_wait_can_ack(int fd, int *retryCnt)
{
    AppContext_t *ctx = GetAppContext();
    PacketHeader_t hdr;
    int ret = server_recv_with_retry(fd, &hdr, retryCnt, TYPE_RECV_CAN_DATA_ACK);
    if (ret == -1)
    {
        LOG_Debug("wait can ack timeout, retry send (data still in FIFO)\n");
        /* 超时，数据仍在 FIFO 中，可以重试发送 */
        /* 注意：不清除 g_pending_fifo_out_size，因为下次发送会重新计算 */
        return STATE_SEND_CAN_DATA; /* 超时，重试发送 */
    }
    if (ret == -2)
    {
        /* 超过重试次数，需要移除 FIFO 中的数据，避免数据堆积 */
        if (g_pending_fifo_out_size > 0)
        {
            /* 使用静态缓冲区避免栈溢出（约24KB）
             * 注意：此函数在单线程服务器上下文中调用，静态缓冲区安全 */
            static uint8_t s_discard_buf[MAX_SEND_FRAMES * sizeof(Can_frame_t)];
            kfifo_out(&ctx->can_fifo, s_discard_buf, g_pending_fifo_out_size);
            LOG_Debug("discarded %u bytes from FIFO after max retries\n", g_pending_fifo_out_size);
            g_pending_fifo_out_size = 0;
        }
        return STATE_DISCONNECT; /* 超过重试次数或类型错误 */
    }

    /* 收到 ACK，从 FIFO 中移除已确认的数据 */
    if (g_pending_fifo_out_size > 0)
    {
        /* 使用静态缓冲区避免栈溢出（约24KB）
         * 注意：此函数在单线程服务器上下文中调用，静态缓冲区安全 */
        static uint8_t s_ack_buf[MAX_SEND_FRAMES * sizeof(Can_frame_t)];
        uint32_t popped = kfifo_out(&ctx->can_fifo, s_ack_buf, g_pending_fifo_out_size);
        LOG_Debug("recv can data ack, popped %u bytes from FIFO (%zu frames)\n",
                  popped, popped / sizeof(Can_frame_t));
        g_pending_fifo_out_size = 0;
    }
    else
    {
        LOG_Debug("recv can data ack, continue send\n");
    }
    return STATE_SEND_CAN_DATA;
}

/* ====== STATE_SEND_END ====== */
static ServerState_e server_handle_send_end(int fd, int *retryCnt)
{
    /* 先尝试接收可能残留的 TYPE_RECV_CAN_DATA_ACK，避免后续状态机收到错误类型 */
    /* 使用较短超时，避免不必要的延迟 */
    PacketHeader_t hdr;
    if (protocol_recv_header(fd, &hdr, 500) == 0) /* 500ms 短超时 */
    {
        if (hdr.type == TYPE_RECV_CAN_DATA_ACK)
        {
            LOG_Debug("consumed pending CAN_DATA_ACK before send END\n");
        }
        else
        {
            LOG_Debug("unexpected type=%d before send END, expected=%d\n",
                      hdr.type, TYPE_RECV_CAN_DATA_ACK);
        }
    }

    hdr.type = TYPE_SEND_END;
    hdr.data_len = 0;
    hdr.frame_count = 0;

    if (server_send_with_retry(fd, &hdr, sizeof(hdr), retryCnt) < 0)
    {
        LOG_Error("send end fail after %d retries\n", MAX_RETRY_COUNT);
        return STATE_DISCONNECT;
    }

    LOG_Debug("send TYPE_SEND_END success, wait for TYPE_SEND_RECV_ACK\n");
    return STATE_WAIT_END_ACK;
}

/* ====== STATE_WAIT_END_ACK ====== */
static ServerState_e server_handle_wait_end_ack(int fd, int *retryCnt)
{
    PacketHeader_t hdr;
    int ret = server_recv_with_retry(fd, &hdr, retryCnt, TYPE_SEND_RECV_ACK);
    if (ret == -1)
    {
        LOG_Debug("wait end ack timeout, retry send END\n");
        return STATE_SEND_END; /* 超时，重试发送 */
    }
    if (ret == -2)
        return STATE_DISCONNECT; /* 超过重试次数或类型错误 */

    LOG_Debug("recv TYPE_SEND_RECV_ACK, transfer complete\n");
    // session->normal_complete = 1; /* 标记正常完成，不需要恢复读指针 */
    return STATE_DISCONNECT;
}

static int AcceptClient(int listenfd)
{
    struct sockaddr_in cliAddr;
    socklen_t len = sizeof(cliAddr);
    int fd = accept(listenfd, (struct sockaddr *)&cliAddr, &len);
    if (fd >= 0)
    {
        LOG_Debug("accept client: %s:%d\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));
    }
    return fd;
}

void *TcpServerThread(void *arg)
{

    int ret = 0;
    int listenfd = 0;
    int port = 9000;
    char *ip4 = NULL;
    ret = server_create(ip4, port, 10, &listenfd);
    if (ret != 0)
    {
        LOG_Error("server_create failed on port %d\n", port);
        return NULL;
    }
    LOG_Debug("server listening on port %d\n", port);
    
    TimeRange_t req;
   
    AppContext_t *ctx = GetAppContext();
    while (1)
    {
        SetCollectStatus(COLLECT_STATUS_RUNNING);
        int connfd = AcceptClient(listenfd);
        if (connfd < 0)
        {
            //LOG_Error("accept failed, retry in 1s\n");
            sleep(1);
            continue;
        }

        LOG_Debug("client connected, waiting for request\n");
        ServerState_e state = STATE_WAIT_REQ;
        int retryCnt = 0;
        while (state != STATE_DISCONNECT)
        {
            switch (state)
            {
            case STATE_WAIT_REQ:
                state = server_handle_wait_req(connfd, &req);
                break;

            case STATE_SEND_REQ_ACK:
                state = server_handle_send_req_ack(connfd, &retryCnt);
                break;

            case STATE_SEND_CAN_DATA:
                state = server_handle_send_can_data(connfd, &retryCnt, &req);
                break;

            case STATE_WAIT_CAN_ACK:
                state = server_handle_wait_can_ack(connfd, &retryCnt);
                break;

            case STATE_SEND_END:
                SetCollectStatus(COLLECT_STATUS_RUNNING);
                state = server_handle_send_end(connfd, &retryCnt);
                break;
            case STATE_WAIT_END_ACK:
                state = server_handle_wait_end_ack(connfd, &retryCnt);
                break;

            default:
                LOG_Error("unknown state=%d, disconnect\n", state);
                state = STATE_DISCONNECT;
                break;
            }
        }
        close(connfd);
        LOG_Debug("client disconnected\n");
    }
    
    close(listenfd);
    return NULL;
}
