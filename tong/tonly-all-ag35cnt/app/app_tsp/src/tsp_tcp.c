/**
 * tsp_tcp.c - TCP连接管理实现
 *
 * 基于libevent bufferevent, 三态收包状态机解决粘包
 */

#include "tsp_tcp.h"
#include "tsp_log.h"

#include <stdlib.h>
#include <event2/buffer.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <event2/buffer.h>

/* 接收缓冲区初始容量 */
#define TLY_RECV_BUF_INIT_CAP   4096

/* ======================== 内部函数声明 ======================== */

static void tcp_read_cb(struct bufferevent *bev, void *user_data);
static void tcp_event_cb(struct bufferevent *bev, short events, void *user_data);
static void tcp_reconnect_cb(evutil_socket_t fd, short events, void *user_data);
static void tcp_reset_recv(tly_tcp_conn_t *conn);
static int  tcp_try_connect(tly_tcp_conn_t *conn);
static void tcp_schedule_reconnect(tly_tcp_conn_t *conn);

/* ======================== 创建/销毁 ======================== */

tly_tcp_conn_t *tly_tcp_create(struct event_base *evbase)
{
    if (!evbase) return NULL;

    tly_tcp_conn_t *conn = (tly_tcp_conn_t *)calloc(1, sizeof(*conn));
    if (!conn) return NULL;

    conn->evbase   = evbase;
    conn->bev      = NULL;
    conn->connected = 0;
    conn->recv_state = TLY_RECV_HEADER;
    conn->expect_len = TLY_PKT_HEADER_LEN;

    conn->recv_buf = (uint8_t *)malloc(TLY_RECV_BUF_INIT_CAP);
    if (!conn->recv_buf) {
        free(conn);
        return NULL;
    }
    conn->recv_cap = TLY_RECV_BUF_INIT_CAP;
    conn->recv_len = 0;

    return conn;
}

void tly_tcp_set_callbacks(tly_tcp_conn_t *conn,
                           tly_tcp_on_packet_cb on_packet,
                           tly_tcp_on_status_cb on_status,
                           void *user_ctx)
{
    if (!conn) return;
    conn->on_packet = on_packet;
    conn->on_status = on_status;
    conn->user_ctx  = user_ctx;
}

static void tcp_reset_recv(tly_tcp_conn_t *conn)
{
    conn->recv_state   = TLY_RECV_HEADER;
    conn->recv_len     = 0;
    conn->expect_len   = TLY_PKT_HEADER_LEN;
    conn->pending_cmd  = 0;
    conn->pending_data_len = 0;
}

/* ======================== 连接 ======================== */

static int tcp_try_connect(tly_tcp_conn_t *conn)
{
    if (conn->bev) {
        bufferevent_free(conn->bev);
        conn->bev = NULL;
    }

    /* 创建socket bufferevent - 启用线程安全选项 */
    conn->bev = bufferevent_socket_new(conn->evbase, -1,
                                       BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
    if (!conn->bev) {
        TLY_LOG_E("Failed to create bufferevent");
        return TLY_ERR_MEMORY;
    }

    bufferevent_setcb(conn->bev, tcp_read_cb, NULL, tcp_event_cb, conn);
    bufferevent_enable(conn->bev, EV_READ | EV_WRITE);

    /* 设置读写超时 (可选) */
    struct timeval read_tv = {300, 0};  /* 5分钟无数据则超时 */
    bufferevent_set_timeouts(conn->bev, &read_tv, NULL);

    /* 发起连接 */
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(conn->port);

    if (inet_pton(AF_INET, conn->host, &sin.sin_addr) != 1) {
        /* 可能是域名, 尝试解析 */
        struct addrinfo hints, *res = NULL;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int rc = getaddrinfo(conn->host, NULL, &hints, &res);
        if (rc != 0 || !res) {
            TLY_LOG_E("Cannot resolve host: %s (%s)", conn->host,
                      gai_strerror(rc));
            return TLY_ERR_CONNECT;
        }

        memcpy(&sin, res->ai_addr, sizeof(struct sockaddr_in));
        sin.sin_port = htons(conn->port);
        freeaddrinfo(res);
    }

    int rc = bufferevent_socket_connect(conn->bev,
                                        (struct sockaddr *)&sin, sizeof(sin));
    if (rc < 0) {
        TLY_LOG_E("Connect failed to %s:%d", conn->host, conn->port);
        return TLY_ERR_CONNECT;
    }

    TLY_LOG_I("Connecting to %s:%d ...", conn->host, conn->port);
    return 0;
}

int tly_tcp_connect(tly_tcp_conn_t *conn, const char *host, int port,
                    int reconnect_interval)
{
    if (!conn || !host) return TLY_ERR_PARAM;

    MPUPmStateChange_t pmState = {0};
    CommonMsg_GetPmState(&pmState);
    if (pmState.PmState == PM_STATE_SLEEP && pmState.WakeupSrc != PM_WAKEUP_SOURCE_TIMER_HEARTBEAT) {
        TLY_LOG_W("TCP connect requested while in sleep state, will not connect");
        return TLY_ERR_CLOSED;
    }

    if (port <= 0 || port > 65535) {
        TLY_LOG_E("Invalid port %d (must be 1-65535)", port);
        return TLY_ERR_PARAM;
    }

    strncpy(conn->host, host, sizeof(conn->host) - 1);
    conn->host[sizeof(conn->host) - 1] = '\0';
    conn->port = port;
    conn->reconnect_interval = reconnect_interval;
    conn->reconnect_backoff  = 1;

    int rc = tcp_try_connect(conn);
    if (rc != 0) {
        TLY_LOG_W("Initial connect failed, will retry in %d seconds",
                  conn->reconnect_interval);
        tcp_schedule_reconnect(conn);
    }
    return 0;
}

void tly_tcp_disconnect(tly_tcp_conn_t *conn)
{
    if (!conn) return;

    if (conn->ev_reconnect) {
        event_del(conn->ev_reconnect);
        event_free(conn->ev_reconnect);
        conn->ev_reconnect = NULL;
    }

    if (conn->bev) {
        bufferevent_free(conn->bev);
        conn->bev = NULL;
    }

    conn->connected = 0;
    tcp_reset_recv(conn);
}

/* ======================== 断线重连 ======================== */

static void tcp_schedule_reconnect(tly_tcp_conn_t *conn)
{
    if (conn->ev_reconnect) {
        event_del(conn->ev_reconnect);
        event_free(conn->ev_reconnect);
        conn->ev_reconnect = NULL;
    }

    if (conn->reconnect_backoff > 60)
        conn->reconnect_backoff = 60;

    int delay = conn->reconnect_interval * conn->reconnect_backoff;
    if (delay > 60) delay = 60;  /* 最大60秒 */
    if (delay < 1)  delay = 1;   /* 下限保护: tv_sec<=0 会被libevent当成"立即触发" */

    TLY_LOG_W("Reconnecting in %d seconds...", delay);

    struct timeval tv = {delay, 0};
    conn->ev_reconnect = event_new(conn->evbase, -1, EV_TIMEOUT,
                                   tcp_reconnect_cb, conn);
    if (conn->ev_reconnect) {
        evtimer_add(conn->ev_reconnect, &tv);
    }

    conn->reconnect_backoff *= 2;
}

static void tcp_reconnect_cb(evutil_socket_t fd, short events, void *user_data)
{
    (void)fd;
    (void)events;

    tly_tcp_conn_t *conn = (tly_tcp_conn_t *)user_data;

    TLY_LOG_I("Attempting reconnect...");
    int rc = tcp_try_connect(conn);
    if (rc != 0) {
        tcp_schedule_reconnect(conn);
    }
}

/* ======================== 数据发送 ======================== */

int tly_tcp_send(tly_tcp_conn_t *conn, const uint8_t *data, uint32_t len)
{
    if (!conn || !data || len == 0) return TLY_ERR_PARAM;
    if (!conn->connected || !conn->bev) return TLY_ERR_CLOSED;

    int rc = bufferevent_write(conn->bev, data, len);
    if (rc != 0) {
        TLY_LOG_E("Failed to send %u bytes", len);
        return TLY_ERR_CONNECT;
    }

    return 0;
}

int tly_tcp_send_packet(tly_tcp_conn_t *conn, const tly_packet_t *pkt)
{
    if (!conn || !pkt) return TLY_ERR_PARAM;
    if (!conn->connected) return TLY_ERR_CLOSED;

    uint8_t buf[TLY_PKT_MAX_SIZE];
    uint32_t len = 0;

    int rc = tly_packet_encode(pkt, buf, &len);
    if (rc != 0) {
        TLY_LOG_E("Failed to encode packet: cmd=0x%02X, rc=%d",
                  pkt->cmd_id, rc);
        return rc;
    }

    TLY_LOG_D("Sending packet: cmd=0x%02X, len=%u", pkt->cmd_id, len);
    return tly_tcp_send(conn, buf, len);
}

/* ======================== 数据接收 (粘包处理) ======================== */

/**
 * 处理已收到的完整包头
 * 从包头中提取 data_len, 切换到收数据状态
 */
static int process_header(tly_tcp_conn_t *conn)
{
    /* 校验起始符 */
    if (conn->recv_buf[0] != 0x23 || conn->recv_buf[1] != 0x23) {
        TLY_LOG_W("Invalid start marker, resetting receiver");
        tcp_reset_recv(conn);
        return TLY_ERR_PROTOCOL;
    }

    /* 提取包头各字段 */
    conn->pending_cmd = conn->recv_buf[2];
    memcpy(conn->pending_vin, conn->recv_buf + 3, TLY_PKT_VIN_LEN);
    conn->pending_vin[TLY_PKT_VIN_LEN] = '\0';
    conn->pending_sw_ver  = conn->recv_buf[20];
    conn->pending_encrypt = conn->recv_buf[21];
    conn->pending_data_len = tly_read_be16(conn->recv_buf + 22);

    TLY_LOG_D("Header received: cmd=0x%02X, VIN=%.17s, data_len=%u",
              conn->pending_cmd, conn->pending_vin, conn->pending_data_len);

    if (conn->pending_data_len > TLY_PKT_MAX_DATA) {
        TLY_LOG_W("Data length %u exceeds max %u, resetting",
                  conn->pending_data_len, TLY_PKT_MAX_DATA);
        tcp_reset_recv(conn);
        return TLY_ERR_PROTOCOL;
    }

    /* 切换到收数据状态: 保持包头在recv_buf前部, 数据从包头之后接着写 */
    conn->recv_state = TLY_RECV_DATA;
    conn->expect_len = TLY_PKT_HEADER_LEN + conn->pending_data_len;
    /* recv_len保持为TLY_PKT_HEADER_LEN不变, 不重置 */

    if (conn->pending_data_len == 0) {
        conn->recv_state   = TLY_RECV_CHECKSUM;
        conn->expect_len   = TLY_PKT_HEADER_LEN + 1;
    }

    return 0;
}

/**
 * 处理已收到的完整数据包(包头+数据+校验)
 */
static void process_complete_packet(tly_tcp_conn_t *conn)
{
    uint32_t total = TLY_PKT_HEADER_LEN + conn->pending_data_len + 1;

    TLY_LOG_Dump("RECV PKT RAW:", conn->recv_buf, total);

    tly_packet_t pkt;
    int rc = tly_packet_decode(conn->recv_buf, total, &pkt);
    if (rc == 0) {
        TLY_LOG_D("Packet decoded: cmd=0x%02X, VIN=%.17s, data_len=%u",
                  pkt.cmd_id, pkt.vin, pkt.data_len);
        if (conn->on_packet) {
            conn->on_packet(&pkt, conn->user_ctx);
        }
        tly_packet_free(&pkt);
    } else {
        TLY_LOG_W("Packet decode failed: rc=%d", rc);
    }

    tcp_reset_recv(conn);
}

/**
 * 从bufferevent读取数据, 驱动状态机
 */
static void tcp_read_cb(struct bufferevent *bev, void *user_data)
{
    (void)bev;
    tly_tcp_conn_t *conn = (tly_tcp_conn_t *)user_data;

    struct evbuffer *input = bufferevent_get_input(bev);

    while (1) {
        /* 确保缓冲区足够 */
        uint32_t need = conn->expect_len - conn->recv_len;
        if (need == 0) break;

        /* 确保缓冲区容量 */
        uint32_t total_need = conn->recv_len + need;
        if (total_need > conn->recv_cap) {
            uint32_t new_cap = conn->recv_cap;
            while (new_cap < total_need) new_cap *= 2;
            uint8_t *new_buf = (uint8_t *)realloc(conn->recv_buf, new_cap);
            if (!new_buf) {
                TLY_LOG_E("Recv buffer realloc failed");
                tcp_reset_recv(conn);
                return;
            }
            conn->recv_buf = new_buf;
            conn->recv_cap = new_cap;
        }

        /* 尝试读取 */
        size_t avail = evbuffer_get_length(input);
        if (avail == 0) break;

        size_t to_read = (need < avail) ? need : avail;
        int n = evbuffer_remove(input,
                                conn->recv_buf + conn->recv_len,
                                to_read);
        if (n <= 0) break;

        conn->recv_len += n;

        /* 检查是否收齐当前阶段 */
        if (conn->recv_len < conn->expect_len) continue;

        /* 根据状态机切换 */
        switch (conn->recv_state) {
        case TLY_RECV_HEADER:
            if (process_header(conn) != 0) {
                return;  /* 无效包头, 已重置 */
            }
            break;

        case TLY_RECV_DATA:
            conn->recv_state = TLY_RECV_CHECKSUM;
            conn->expect_len = conn->recv_len + 1;
            break;

        case TLY_RECV_CHECKSUM:
            process_complete_packet(conn);
            break;
        }
    }
}

/* ======================== 连接事件 ======================== */

static void tcp_event_cb(struct bufferevent *bev, short events, void *user_data)
{
    (void)bev;
    tly_tcp_conn_t *conn = (tly_tcp_conn_t *)user_data;

    if (events & BEV_EVENT_CONNECTED) {
        TLY_LOG_I("TCP connected to %s:%d", conn->host, conn->port);
        conn->connected = 1;
        conn->reconnect_backoff = 1;

        if (conn->ev_reconnect) {
            event_del(conn->ev_reconnect);
            event_free(conn->ev_reconnect);
            conn->ev_reconnect = NULL;
        }

        if (conn->on_status) {
            conn->on_status(1, conn->user_ctx);
        }
    } else if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
        conn->connected = 0;

        if (events & BEV_EVENT_ERROR) {
            int err = EVUTIL_SOCKET_ERROR();
            TLY_LOG_W("TCP error: %s (%d)", evutil_socket_error_to_string(err), err);
        }
        if (events & BEV_EVENT_EOF) {
            TLY_LOG_W("TCP connection closed by remote");
        }

        if (conn->bev) {
            bufferevent_free(conn->bev);
            conn->bev = NULL;
        }

        tcp_reset_recv(conn);

        if (conn->on_status) {
            conn->on_status(0, conn->user_ctx);
        }

        tcp_schedule_reconnect(conn);
    }
}

/* ======================== 状态查询 ======================== */

int tly_tcp_is_connected(const tly_tcp_conn_t *conn)
{
    return conn ? conn->connected : 0;
}

/* ======================== 销毁 ======================== */

void tly_tcp_destroy(tly_tcp_conn_t *conn)
{
    if (!conn) return;

    tly_tcp_disconnect(conn);

    free(conn->recv_buf);
    conn->recv_buf = NULL;
    free(conn);
}
