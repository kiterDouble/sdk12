/**
 * tsp_tcp.h - TCP连接管理
 *
 * 基于libevent的TCP客户端, 支持断线重连和粘包处理
 */

#ifndef __TSP_TCP_H__
#define __TSP_TCP_H__

#include "tsp_protocol.h"
#include <event2/event.h>
#include <event2/bufferevent.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 前向声明, 避免头文件循环依赖 */
struct tly_packet;

/* ======================== 收包状态机 ======================== */

typedef enum tly_recv_state {
    TLY_RECV_HEADER,     /* 等待包头(24字节) */
    TLY_RECV_DATA,       /* 等待数据单元(N字节) */
    TLY_RECV_CHECKSUM,   /* 等待校验码(1字节) */
} tly_recv_state_t;

/* ======================== 回调类型 ======================== */

/**
 * 收到完整数据包回调
 * @param pkt  解码后的数据包(由框架分配, 回调结束后自动释放)
 * @param ctx  用户上下文
 */
typedef void (*tly_tcp_on_packet_cb)(const struct tly_packet *pkt, void *ctx);

/**
 * 连接状态变更回调
 * @param connected 1=已连接, 0=已断开
 * @param ctx       用户上下文
 */
typedef void (*tly_tcp_on_status_cb)(int connected, void *ctx);

/* ======================== TCP连接结构 ======================== */

typedef struct tly_tcp_conn {
    struct event_base *evbase;
    struct bufferevent *bev;

    /* 收包状态机 */
    tly_recv_state_t    recv_state;
    uint8_t             *recv_buf;
    uint32_t            recv_len;
    uint32_t            recv_cap;
    uint32_t            expect_len;
    uint8_t             pending_cmd;
    uint16_t            pending_data_len;
    uint8_t             pending_encrypt;
    char                pending_vin[TLY_PKT_VIN_LEN + 1];
    uint8_t             pending_sw_ver;

    /* 连接参数 */
    char                host[128];
    int                 port;
    int                 reconnect_interval;
    struct event        *ev_reconnect;
    int                 reconnect_backoff; /* 当前退避倍数 */
    int                 connected;

    /* 回调 */
    tly_tcp_on_packet_cb on_packet;
    tly_tcp_on_status_cb on_status;
    void                *user_ctx;
} tly_tcp_conn_t;

/* ======================== API ======================== */

/**
 * 创建TCP连接管理器
 * @param evbase  libevent事件循环
 * @return 连接对象, NULL失败
 */
tly_tcp_conn_t *tly_tcp_create(struct event_base *evbase);

/**
 * 设置回调
 */
void tly_tcp_set_callbacks(tly_tcp_conn_t *conn,
                           tly_tcp_on_packet_cb on_packet,
                           tly_tcp_on_status_cb on_status,
                           void *user_ctx);

/**
 * 连接远程服务器
 */
int  tly_tcp_connect(tly_tcp_conn_t *conn, const char *host, int port,
                     int reconnect_interval);

/**
 * 断开连接
 */
void tly_tcp_disconnect(tly_tcp_conn_t *conn);

/**
 * 发送原始数据
 * @return 0成功, 负数失败
 */
int  tly_tcp_send(tly_tcp_conn_t *conn, const uint8_t *data, uint32_t len);

/**
 * 发送数据包(自动编码)
 */
int  tly_tcp_send_packet(tly_tcp_conn_t *conn, const tly_packet_t *pkt);

/**
 * 销毁连接对象, 释放所有资源
 */
void tly_tcp_destroy(tly_tcp_conn_t *conn);

/**
 * 获取连接状态
 */
int  tly_tcp_is_connected(const tly_tcp_conn_t *conn);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_TCP_H__ */
