#ifndef __ETHERNET_INTERFACE_H__
#define __ETHERNET_INTERFACE_H__


#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <cstring>


#ifdef __cplusplus
extern "C" {
#endif




/* ==================== 网络配置常量 ==================== */
#define SERVER_PORT           5000      /* UDP服务器端口 */
#define DATA_LENGTH           1500      /* 固定数据长度 */

/* ==================== CAN数据配置常量 ==================== */
#define MAX_FRAME_NUM         10000     /* 最大CAN帧数量 */
#define MAX_DATA_BUF_SIZE     (30 * 1024) /* 最大数据缓冲区大小 */
#define MAX_CAN_DLC           64        /* CAN最大DLC */

/* ==================== 超时配置常量 ==================== */
#define DEFAULT_RECV_TIMEOUT  1000      /* 默认接收超时(ms) */
#define DEFAULT_ACK_TIMEOUT   1000      /* ACK超时(ms) */
#define DEFAULT_CONNECT_TMO   5000      /* 连接超时(ms) */
#define RETRY_INTERVAL        1         /* 重试间隔(s) */
#define MAX_RETRY_COUNT       3         /* 最大重试次数 */

/* ==================== TCP客户端配置常量 ==================== */
#define TCP_CLIENT_IP         "192.168.1.202"  /* TCP客户端目标IP */
#define TCP_CLIENT_PORT       9000              /* TCP客户端目标端口 */

/* ==================== 写入配置常量 ==================== */
#define WRITE_DELAY_MS        300       /* 写入延迟(ms) */
#define BACKUP_CHECK_DELAY_MS 1000      /* 备份检查延迟(ms) */




// 线程安全的队列类
class SafeQueue {
public:
    void enqueue(const std::vector<unsigned char>& data);
    bool dequeue(std::vector<unsigned char>& data); 
    void canenqueue(const std::vector<unsigned char>& candata) ;
    bool candequeue(std::vector<unsigned char>& candata); 
    bool getcandequeue();
private:
    std::queue<std::vector<unsigned char>> queue_;
    std::mutex mutex_;
    std::queue<std::vector<unsigned char>> canqueue_;
    std::mutex canmutex_;
};

int EthernetInit();




/* 数据包类型定义 */
typedef enum
{
    TYPE_SEND_ACK = 1,          /* 发送 ACK */
    TYPE_RECV_ACK,              /* 接收 ACK */
    TYPE_RECV_CAN_DATA,         /* 接收 CAN 数据 */
    TYPE_RECV_CAN_DATA_ACK,     /* 接收 CAN 数据 ACK */
    TYPE_SEND_END,              /* 发送结束 */
    TYPE_SEND_RECV_ACK          /* 发送接收 ACK */
} Type_e;

/* 返回值定义 */
#define RECV_ERROR           -1            /* 接收错误 */
#define RECV_CONTINUE        0             /* 继续接收 */
#define RECV_END             1             /* 收到结束指令 */





typedef struct
{
    uint8_t type;//1、发送ack 2、接收 ack 3、接收can 数据。4、接收can 数据ack 5、发送接收 
    uint16_t frame_count;
    uint16_t data_len;
} PacketHeader_t;

void handleServer(SafeQueue &queue);
int protocol_recv_header(int fd, PacketHeader_t *hdr, int timeout_ms);
#ifdef __cplusplus
}
#endif
#endif
