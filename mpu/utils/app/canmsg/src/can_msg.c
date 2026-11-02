#include "can_msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include "crcTool.h"
#include <glib.h>
#include "err_code.h"
#include "sr_log.h"

// 全局状态与资源
static CanMsgConfig_t g_config;
static int g_initialized = 0;
static int g_running = 0;
static pthread_t g_recv_thread;
static int g_tcp_socket = -1;
static struct sockaddr_in g_server_addr;
static pthread_mutex_t g_msg_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_msg_cond = PTHREAD_COND_INITIALIZER;
static CanMsg_t* g_msg_buffer = NULL;
static uint32_t g_buffer_idx = 0;
static uint32_t g_callback_interval = 10; // 回调周期（单位：ms）
static uint64_t g_last_callback_time = 0; // 上次回调的时间戳（单位：ms）

// 同步控制：用于等待响应
static pthread_mutex_t g_sync_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_sync_cond = PTHREAD_COND_INITIALIZER;
static uint8_t g_waiting_sub = 0;  // 等待的响应SUB值（0：不等待，2：等待0x02，4：等待0x04）
static uint8_t g_response_received = 0;  // 响应是否收到（1：收到）
static uint8_t g_response_received_start = 0;  // 响应是否收到（1：收到）
static uint8_t g_response_received_stop = 0;  // 响应是否收到（1：收到）
// 在can_msg.c中添加必要的全局变量补充
static pthread_mutex_t g_transmit_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_transmit_cond = PTHREAD_COND_INITIALIZER;
static uint8_t g_transmit_response_received = 0;


// 新增全局变量
// static pthread_t g_query_thread;
static pthread_t g_queue_thread;
static pthread_mutex_t g_send_mutex = PTHREAD_MUTEX_INITIALIZER;

CanMsg_t* msgs = NULL;
// TCP 服务器配置（客户端模式）
#define TCP_SERVER_IP "192.168.1.9"
#define TCP_SERVER_PORT 5052



// 消息优先级定义（查询消息优先级更高）
typedef enum {
    MSG_PRIORITY_LOW = 0,    // CanMsg_Transmit发送的消息
    MSG_PRIORITY_HIGH = 1    // send_query_message发送的消息
} MsgPriority;

// 发送队列元素
typedef struct {
    uint8_t* data;           // 待发送数据
    uint32_t len;            // 数据长度
    MsgPriority priority;    // 优先级
} SendQueueItem;

// 发送队列（使用双向链表实现）
static GList* g_send_queue = NULL;
static pthread_mutex_t g_queue_mutex = PTHREAD_MUTEX_INITIALIZER;  // 保护队列操作
static pthread_cond_t g_queue_cond = PTHREAD_COND_INITIALIZER;     // 通知发送线程
static pthread_t g_send_thread;                                    // 单独的发送线程



static int send_query_message(uint8_t over); 
static void parse_response_message(const uint8_t* buffer, uint32_t buffer_len);
static int tcp_init(void) ;
static void send_queue_push(uint8_t* data, uint32_t len, MsgPriority priority);




// 接收队列元素定义（存储接收到的原始字节流片段）
typedef struct {
    uint8_t* data;
    uint32_t len;
} RecvQueueItem;

// 全局接收队列及同步锁
static GList* g_recv_queue = NULL;
static pthread_mutex_t g_recv_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

// 释放队列元素内存
static void free_recv_queue_item(RecvQueueItem* item) {
    if (item) {
        free(item->data);
        free(item);
    }
}

// 向队列添加新接收的数据
static void recv_queue_append(const uint8_t* data, uint32_t len) {
    if (!data || len == 0) return;

    RecvQueueItem* item = malloc(sizeof(RecvQueueItem));
    item->data = malloc(len);
    memcpy(item->data, data, len);
    item->len = len;

    pthread_mutex_lock(&g_recv_queue_mutex);
    g_recv_queue = g_list_append(g_recv_queue, item);
    pthread_mutex_unlock(&g_recv_queue_mutex);
}

// 从队列提取指定长度的数据（返回新分配的内存，实际提取长度通过out_len返回）
static uint8_t* recv_queue_extract(uint32_t need_len, uint32_t* out_len) {
    *out_len = 0;
    if (need_len == 0) return NULL;

    pthread_mutex_lock(&g_recv_queue_mutex);

    // 计算队列中总数据长度
    uint32_t total_len = 0;
    GList* iter = g_recv_queue;
    while (iter) {
        RecvQueueItem* item = iter->data;
        total_len += item->len;
        iter = iter->next;
    }

    // 数据不足，直接返回
    if (total_len < need_len) {
        pthread_mutex_unlock(&g_recv_queue_mutex);
        return NULL;
    }

    // 分配内存存储提取的数据
    uint8_t* buffer = malloc(need_len);
    uint32_t copied = 0;
    GList* to_remove = NULL;

    // 从队列中复制数据
    iter = g_recv_queue;
    while (iter && copied < need_len) {
        RecvQueueItem* item = iter->data;
        uint32_t copy_len = (item->len < need_len - copied) ? item->len : need_len - copied;

        memcpy(buffer + copied, item->data, copy_len);
        copied += copy_len;

        if (copy_len < item->len) {
            // 该节点还有剩余数据，截断保留
            uint8_t* new_data = malloc(item->len - copy_len);
            memcpy(new_data, item->data + copy_len, item->len - copy_len);
            free(item->data);
            item->data = new_data;
            item->len -= copy_len;
            break;
        } else {
            // 该节点数据已用完，标记删除
            to_remove = g_list_append(to_remove, item);
        }
        iter = iter->next;
    }

    // 移除已用完的节点
    while (to_remove) {
        RecvQueueItem* item = to_remove->data;
        g_recv_queue = g_list_remove(g_recv_queue, item);
        free_recv_queue_item(item);
        to_remove = g_list_delete_link(to_remove, to_remove);
    }

    *out_len = copied;
    pthread_mutex_unlock(&g_recv_queue_mutex);
    return buffer;
}

// 清空接收队列（重连时使用）
static void recv_queue_clear() {
    pthread_mutex_lock(&g_recv_queue_mutex);
    g_list_foreach(g_recv_queue, (GFunc)free_recv_queue_item, NULL);
    g_list_free(g_recv_queue);
    g_recv_queue = NULL;
    pthread_mutex_unlock(&g_recv_queue_mutex);
}



// 模拟获取系统时间（单位：微秒）
static uint64_t get_current_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec * 1000000 + tv.tv_usec);
}

// 模拟获取系统时间（单位：ms）
static uint64_t get_current_time_ms(void) {
    return get_current_time_us() / 1000;
}

// 打包CAN消息为指定格式（动态设置SUB值）
static int pack_can_message(const CanMsg_t* msg, uint8_t* buffer, uint32_t buffer_len) {
    if (!msg || !buffer) return -1;
    
    // 动态设置AID、MID、SUB（AID=0x05, MID=0x02，SUB根据消息ID区分）
    uint8_t aid = 0x05;
    uint8_t mid = 0x02;
    uint8_t sub = 0x01;  // 默认SUB=0x01
    // 计算消息总长度   3+1+1+4+1+msg->dlc+8
    uint32_t data_len = 3+1+1+4+1+msg->dlc+8;
    uint32_t total_len = 2 + 1 + 1 + 1 + 2 + 1 + data_len + 2; // 完整帧长度
    
    if (buffer_len < total_len) return -2; // 缓冲区不足
    
    uint8_t idx = 0;
    
    // 帧头
    buffer[idx++] = 0x55;
    buffer[idx++] = 0xAA;
    
    // AID, MID, SUB
    buffer[idx++] = aid;
    buffer[idx++] = mid;
    buffer[idx++] = sub;
    
    uint16_t payload_len = data_len; // 包含0x00分隔符
    buffer[idx++] = (payload_len >> 8) & 0xFF;
    buffer[idx++] = payload_len & 0xFF;
    
    // 分隔符
    buffer[idx++] = 0x00;

    // 分隔符与计数（原逻辑保留）
    buffer[idx++] = 0x00;
    buffer[idx++] = 0x00; // count占位
    buffer[idx++] = 0x00; // index占位
    
    // CanBusType
    buffer[idx++] = (uint8_t)msg->bus;
    
    buffer[idx++] = (uint8_t)msg->is_fd ? 1 : 0;
    // CAN ID (4字节)
    buffer[idx++] = (msg->id >> 24) & 0xFF;
    buffer[idx++] = (msg->id >> 16) & 0xFF;
    buffer[idx++] = (msg->id >> 8) & 0xFF;
    buffer[idx++] = msg->id & 0xFF;
    
    // DLC
    buffer[idx++] = msg->dlc;
    
    // CAN数据
    memcpy(&buffer[idx], msg->data, msg->dlc);
    idx += msg->dlc;
    
    // 时间戳 (8字节，微秒)
    buffer[idx++] = (msg->timestamp >> 56) & 0xFF;
    buffer[idx++] = (msg->timestamp >> 48) & 0xFF;
    buffer[idx++] = (msg->timestamp >> 40) & 0xFF;
    buffer[idx++] = (msg->timestamp >> 32) & 0xFF;
    buffer[idx++] = (msg->timestamp >> 24) & 0xFF;
    buffer[idx++] = (msg->timestamp >> 16) & 0xFF;
    buffer[idx++] = (msg->timestamp >> 8) & 0xFF;
    buffer[idx++] = msg->timestamp & 0xFF;
    
    // CRC校验
    uint16_t crc = Crc16(buffer, idx, Crc16Ccitt);
    buffer[idx++] = (crc >> 8) & 0xFF;
    buffer[idx++] = crc & 0xFF;
    // LOG_Debug("发送TCP数据长度: %d\n", idx);
    send_queue_push(buffer, idx, MSG_PRIORITY_LOW);
    usleep(1000);  // 保留原延迟，避免发送过于频繁
    return idx;  // 返回实际打包长度（原逻辑返回send结果，此处调整为队列操作成功）
}

// 修改unpack_can_message函数，添加传输响应通知
static int unpack_can_message(const uint8_t* buffer, uint32_t buffer_len, CanMsg_t* msg) {
    if (!buffer || !msg || buffer_len < 13) return -1; // 最小帧长度检查
    
    // 帧头校验
    if (buffer[0] != 0x55 || buffer[1] != 0xAA) return -2;
    
    uint32_t idx = 2;
    
    // 跳过AID, MID, SUB（后续判断响应时会用到）
    idx += 3;
    
    // 解析后续长度
    uint16_t payload_len = (buffer[idx] << 8) | buffer[idx + 1] + 1;
    idx += 2;
    
    // 检查分隔符
    if (buffer[idx++] != 0x00) return -3;
    
    // 验证总长度
    if (buffer_len != 2 + 3 + 2 + 1 + payload_len + 2) return -4;
    
    // CRC校验
    uint16_t received_crc = (buffer[12] << 8) | buffer[13];
    uint16_t calculated_crc = Crc16(buffer, 11, Crc16Ccitt);
    if (received_crc != calculated_crc) {
        LOG_Debug("CRC Fail: recv 0x%04X, cal 0x%04X", received_crc, calculated_crc);
        return -8;
    }
    
    // 解析成功，通知Transmit
    pthread_mutex_lock(&g_transmit_mutex);
    g_transmit_response_received = 1;
    pthread_cond_signal(&g_transmit_cond);
    pthread_mutex_unlock(&g_transmit_mutex);
    
    return 0; // 解析成功
}





// 优化后的TCP接收线程（使用队列处理拆包分包）
static void* can_tcp_recv_thread(void* arg) {
    (void)arg;
    uint8_t recv_buf[1024] = {0};

    while (g_running) {
        // 接收TCP数据并加入队列
        if (g_tcp_socket != -1) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(g_tcp_socket, &read_fds);
            struct timeval tv = {3, 0}; // 3秒超时
            int select_ret = select(g_tcp_socket + 1, &read_fds, NULL, NULL, &tv);

            if (select_ret < 0) {
                LOG_Debug("select err: %s\n", strerror(errno));
                sleep(1);
                continue;
            }

            if (select_ret > 0 && FD_ISSET(g_tcp_socket, &read_fds)) {
                int recv_len = recv(g_tcp_socket, recv_buf, sizeof(recv_buf), 0);
                if (recv_len < 0) {
                    LOG_Debug("TCP connect erro: %s\n", strerror(errno));
                    sleep(1);
                    continue;
                } else if (recv_len == 0) {
                    LOG_Debug("TCP connect fail,retry\n");
                    close(g_tcp_socket);
                    tcp_init();
                    recv_queue_clear(); // 重连时清空队列，避免数据混乱
                    sleep(1);
                    continue;
                }

                // 将新接收的数据加入队列
                recv_queue_append(recv_buf, recv_len);
                LOG_DebugHex(recv_buf, recv_len, "can_tcp_recv_recv_buf:");
                memset(recv_buf, 0, sizeof(recv_buf));
            }
        } else {
            // 未连接时尝试重连
            if (tcp_init() == 0) {
                LOG_Debug("TCP retry success\n");
            } else {
                LOG_Debug("TCP retry fail retry 1s\n");
                sleep(1);
                recv_queue_clear();
            }
        }

        // 从队列中解析完整帧
        while (1) {
            // 步骤1：查找帧头（至少需要2字节）
            uint32_t header_len;
            uint8_t* header = recv_queue_extract(2, &header_len);
            if (header_len < 2) {
                // 数据不足2字节，放回队列（如果有数据）
                if (header_len > 0) {
                    recv_queue_append(header, header_len);
                }
                free(header);
                break; // 退出循环，等待更多数据
            }
            // 验证帧头（0x55 0xAA）
            if (header[0] != 0x55 || header[1] != 0xAA) {
                // 帧头错误，丢弃第一个字节，继续查找
                free(header);
                uint32_t dummy_len;
                uint8_t* dummy = recv_queue_extract(1, &dummy_len);
                free(dummy);
                continue;
            }
            free(header); // 帧头正确，释放临时内存
            // 步骤2：提取AID、MID、SUB（3字节）和payload_len（2字节）
            uint32_t meta_len;
            uint8_t* meta = recv_queue_extract(5, &meta_len); // 3+2=5字节
            if (meta_len < 5) {
                // 数据不足，放回已提取的部分+帧头
                if (meta_len > 0) {
                    recv_queue_append(meta, meta_len);
                }
                free(meta);
                uint8_t temp_header[2] = {0x55, 0xAA};
                recv_queue_append(temp_header, 2);
                break;
            }
            // 解析元数据（AID=meta[0], MID=meta[1], SUB=meta[2]）
            uint8_t aid = meta[0];
            uint8_t mid = meta[1];
            uint8_t sub = meta[2];
            uint16_t payload_len = (meta[3] << 8) | meta[4]; // 后续数据长度（不含CRC）
            free(meta);
            // 新增：校验 payload_len 合理性（根据实际最大帧长调整，此处假设不超过1024）
            if (payload_len > 1024) {
                LOG_Debug("fail payload_len: %d,abort", payload_len);
                // 丢弃当前不完整的帧，继续查找下一个帧头
                continue;
            }
            // 步骤3：提取分隔符（1字节）
            uint32_t separator_len;
            uint8_t* separator = recv_queue_extract(1, &separator_len);
            if (separator_len < 1) {
                // 数据不足，放回已提取的元数据和帧头
                if (separator_len > 0) {
                    recv_queue_append(separator, separator_len);
                }
                free(separator);
                // 放回元数据和帧头
                uint8_t temp_meta[5] = {aid, mid, sub, (payload_len>>8)&0xFF, payload_len&0xFF};
                recv_queue_append(temp_meta, 5);
                uint8_t temp_header[2] = {0x55, 0xAA};
                recv_queue_append(temp_header, 2);
                break;
            }
            // 验证分隔符
            if (separator[0] != 0x00) {
                LOG_Debug("0x00 fail");
                free(separator);
                // 丢弃当前帧头，继续查找下一个
                continue;
            }
            free(separator);

            // 步骤4：提取完整帧的剩余部分（payload_len + CRC2字节）
            uint32_t frame_remain_len;
            uint8_t* frame_remain = recv_queue_extract(payload_len + 2, &frame_remain_len);
            if (frame_remain_len < payload_len + 2) {
                // 数据不足，放回已提取的部分+元数据+帧头+分隔符
                if (frame_remain != NULL && frame_remain_len > 0) { // 明确检查非NULL
                    recv_queue_append(frame_remain, frame_remain_len);
                }
                free(frame_remain); // 即使为NULL，free也是安全的
                // 放回分隔符、元数据和帧头（顺序必须与原始帧一致）
                uint8_t temp_sep[1] = {0x00};
                recv_queue_append(temp_sep, 1);
                uint8_t temp_meta[5] = {aid, mid, sub, (payload_len>>8)&0xFF, payload_len&0xFF};
                recv_queue_append(temp_meta, 5);
                uint8_t temp_header[2] = {0x55, 0xAA};
                recv_queue_append(temp_header, 2);
                break;
            }

            // 拼接完整帧时严格检查长度，避免越界
            uint32_t total_frame_len = 2 + 5 + 1 + payload_len + 2; // 帧头(2) + 元数据(5) + 分隔符(1) + 数据(payload_len) + CRC(2)
            uint8_t* complete_frame = malloc(total_frame_len);
            if (!complete_frame) {
                LOG_Debug("complete_frame malloc fail");
                free(frame_remain);
                continue;
            }
            // 复制帧头
            complete_frame[0] = 0x55;
            complete_frame[1] = 0xAA;
            // 复制元数据
            complete_frame[2] = aid;
            complete_frame[3] = mid;
            complete_frame[4] = sub;
            complete_frame[5] = (payload_len >> 8) & 0xFF;
            complete_frame[6] = payload_len & 0xFF;
            // 复制分隔符
            complete_frame[7] = 0x00;
            memcpy(complete_frame + 8, frame_remain, payload_len + 2);
            free(frame_remain); // 此处释放不再使用的frame_remain
            LOG_DebugHex(complete_frame, total_frame_len, "can_tcp_recv_thread_fix:");
            if (aid == 0x05 && mid == 0x02 && sub == 0x04) {
                // 处理响应消息（SUB=0x04）
                LOG_DebugHex(complete_frame, total_frame_len, "can_msg_0x04:");
                parse_response_message(complete_frame, total_frame_len);
            } 
            else if((aid == 0x05 && mid == 0x02 && sub == 0x02))
            {
                // pthread_mutex_lock(&g_transmit_mutex);
                g_transmit_response_received = 1;
                pthread_cond_signal(&g_transmit_cond);
                LOG_DebugHex(complete_frame, total_frame_len, "can msg tcp send data:");
                // pthread_mutex_unlock(&g_transmit_mutex);
            }
            else if((aid == 0x05 && mid == 0x02 && sub == 0x06))
            {
                if ((g_response_received_stop == 1))
                {
                    g_response_received_stop = 2;
                    g_response_received = 1;
                    pthread_cond_signal(&g_sync_cond);
                    LOG_DebugHex(complete_frame, total_frame_len, "can msg tcp stop data:");
                    // return;
                }
            }
            free(complete_frame);
        }

        // 定时触发回调（按配置的flush_cycle）
        uint64_t current_time = get_current_time_ms();
        if (current_time - g_last_callback_time >= g_config.flush_cycle) {
            pthread_mutex_lock(&g_msg_mutex);
            if (g_buffer_idx > 0 && g_config.recv_indication_cbk) {
                g_config.recv_indication_cbk(g_msg_buffer, g_buffer_idx);
                g_buffer_idx = 0;
            }
            pthread_mutex_unlock(&g_msg_mutex);
            g_last_callback_time = current_time;
        }

        usleep(100); // 降低CPU占用
    }

    recv_queue_clear();
    return NULL;
}


static int tcp_init(void) {
    // 关闭现有连接
    if (g_tcp_socket != -1) {
        close(g_tcp_socket);
        g_tcp_socket = -1;
    }

    g_tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_tcp_socket < 0) {
        LOG_Debug("creat socker fail: %s", strerror(errno));
        return -1;
    }

    // 禁用Nagle算法，避免小包合并
    int flag = 1;
    if (setsockopt(g_tcp_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
        LOG_Debug("set TCP_NODELAY fail: %s", strerror(errno));
        // 非致命错误，但可能仍有粘包风险
    }

    memset(&g_server_addr, 0, sizeof(g_server_addr));
    g_server_addr.sin_family = AF_INET;
    g_server_addr.sin_port = htons(TCP_SERVER_PORT);
    if (inet_pton(AF_INET, TCP_SERVER_IP, &g_server_addr.sin_addr) <= 0) {
        close(g_tcp_socket);
        g_tcp_socket = -1;
        LOG_Debug("fail address");
        return -2;
    }

    if (connect(g_tcp_socket, (struct sockaddr*)&g_server_addr, sizeof(g_server_addr)) < 0) {
        close(g_tcp_socket);
        g_tcp_socket = -1;
        LOG_Debug("connect address fail: %s", strerror(errno));
        return -3;
    }

    LOG_Debug("TCP connect success : %s:%d", TCP_SERVER_IP, TCP_SERVER_PORT);
    return 0;
}

// 组装并发送查询消息
static int send_query_message(uint8_t over) {
    if (g_tcp_socket == -1 || g_config.filter_count == 0 || !g_config.filters[0].ids) {
        LOG_Debug("send reques fail");
        return -1;
    }

    // 获取过滤器信息
    CanFilter_t* filter = &g_config.filters[0];
    uint32_t id_count = filter->id_count;
    uint32_t* ids = filter->ids;

    if (id_count == 0) {
        LOG_Debug("filters is NULL");
        return -2;
    }
    pthread_mutex_lock(&g_sync_mutex);
    g_waiting_sub = 0x04;
    g_response_received = 0;
    pthread_mutex_unlock(&g_sync_mutex);
    // 计算消息长度
    uint32_t can_ids_len = id_count * 4;

    uint32_t payload_len = 1 + 3 + 2 + can_ids_len + 2; // 0x00 + 3字节0 + 2字节计数 + CAN IDs + 2 字节时间间隔

    uint32_t total_len = 2 + 3 + 2 + 1 + 3 + 2 + can_ids_len + 2 + 2; // 完整帧长度
    uint8_t buffer[total_len];
    uint32_t idx = 0;

    // pthread_mutex_lock(&g_send_mutex);
    // 组装帧头
    buffer[idx++] = 0x55;
    buffer[idx++] = 0xAA;
    
    // AID, MID, SUB
    buffer[idx++] = 0x05;
    buffer[idx++] = 0x02;
    if (over == 2)
    {
        buffer[idx++] = 0x05;
    }
    else
    {
        buffer[idx++] = 0x03;
    }
    // 长度信息（总长度-8）
    uint16_t len_field = total_len - 8;
    buffer[idx++] = (len_field >> 8) & 0xFF;
    buffer[idx++] = len_field & 0xFF;
    
    // 分隔符和默认字段
    buffer[idx++] = 0x00;
    buffer[idx++] = 0x00;
    buffer[idx++] = 0x00;
    buffer[idx++] = 0x00;
    
    // CANID总数
    buffer[idx++] = (id_count >> 8) & 0xFF;
    buffer[idx++] = id_count & 0xFF;
    
    // CANID列表
    for (uint32_t i = 0; i < id_count; i++) {
        buffer[idx++] = (ids[i] >> 24) & 0xFF;
        buffer[idx++] = (ids[i] >> 16) & 0xFF;
        buffer[idx++] = (ids[i] >> 8) & 0xFF;
        buffer[idx++] = ids[i] & 0xFF;
    }

    if(over == 0)
    {
        //新增时间间隔
        buffer[idx++] = (g_config.flush_cycle >> 8) & 0xFF;
        buffer[idx++] = g_config.flush_cycle & 0xFF;
    }
    else
    {
        //新增时间间隔
        buffer[idx++] = 0x00;
        buffer[idx++] = 0x00;
    }

    // CRC校验
    uint16_t crc = Crc16(buffer, idx, Crc16Ccitt);
    buffer[idx++] = (crc >> 8) & 0xFF;
    buffer[idx++] = crc & 0xFF;
    
    send_queue_push(buffer, idx, MSG_PRIORITY_HIGH);
    return 0;
}


// 新增：解析响应消息
static void parse_response_message(const uint8_t* buffer, uint32_t buffer_len)
{
    if (!buffer || buffer_len < 10) return;
    
    // // 验证帧头
    if (buffer[4] == g_waiting_sub) 
    {
        g_response_received = 1;
        pthread_cond_signal(&g_sync_cond);
    }
    // pthread_mutex_unlock(&g_sync_mutex);
    if (g_response_received_start == 1)
    {
        g_response_received_start = 0;
        return;
        /* code */
    }
    uint32_t idx = 11;
    // 解析长度字段（总长度-8）
    uint16_t len_field = (buffer[5] << 8) | buffer[6];
    
    // 获取过滤器信息
    if (g_config.filter_count == 0 || !g_config.filters[0].ids) return;
    CanFilter_t* filter = &g_config.filters[0];
    uint32_t x = filter->id_count;
    // 2. 释放上一次的旧内存（若存在）
    if (msgs != NULL) {
        free(msgs);
        msgs = NULL;
        msgs = 0;
    }
    msgs = malloc(sizeof(CanMsg_t) * x);
    if (!msgs) return;
    
    // 记录接收时间戳
    uint64_t timestamp = get_current_time_us();
    
    // 解析X组CAN消息
    uint32_t msg_count = 0;
    for (uint32_t i = 0; i < x && idx < buffer_len - 2; i++) {
        // 检查剩余长度是否足够
        if (idx + 4 > buffer_len - 2) break; // 至少需要标志(1)+ID(4)+DLC(1)
        
        // 解析CAN ID
        msgs[msg_count].id = (buffer[idx] << 24) | (buffer[idx + 1] << 16) |
                           (buffer[idx + 2] << 8) | buffer[idx + 3];
        idx += 4;
        
        // 解析DLC
        msgs[msg_count].dlc = buffer[idx++];
        if (msgs[msg_count].dlc > CAN_MSG_MAX_PAYLOAD_LEN) continue;
        
        // 检查数据长度
        if (idx + msgs[msg_count].dlc > buffer_len - 2) break;
        
        // 解析数据
        memcpy(msgs[msg_count].data, &buffer[idx], msgs[msg_count].dlc);
        idx += msgs[msg_count].dlc;
        
        // 设置其他字段
        msgs[msg_count].bus = filter->bus;
        // LOG_Debug("filter->bus:%d,%lx\r\n",filter->bus,msgs[msg_count].id);
        msgs[msg_count].timestamp = timestamp;
        
        msg_count++;
    }

    // 调用回调函数
    if (msg_count > 0&& g_config.recv_indication_cbk) 
    {
        g_config.recv_indication_cbk(msgs, msg_count);
    }
    
    // free(msgs);
}


// 向队列添加消息（按优先级插入，高优先级在前）
static void send_queue_push(uint8_t* data, uint32_t len, MsgPriority priority) {
    if (!data || len == 0) return;

    // 分配队列元素
    SendQueueItem* item = malloc(sizeof(SendQueueItem));
    item->data = malloc(len);
    memcpy(item->data, data, len);
    item->len = len;
    item->priority = priority;

    // 加锁保护队列
    pthread_mutex_lock(&g_queue_mutex);

    // 高优先级消息插入队头，低优先级插入队尾
    if (priority == MSG_PRIORITY_HIGH) {
        g_send_queue = g_list_prepend(g_send_queue, item);  // 高优先级放前面
    } else {
        g_send_queue = g_list_append(g_send_queue, item);   // 低优先级放后面
    }

    // 通知发送线程有新消息
    pthread_cond_signal(&g_queue_cond);
    pthread_mutex_unlock(&g_queue_mutex);
}

// 从队列取出消息（优先取高优先级）
static SendQueueItem* send_queue_pop() {
    pthread_mutex_lock(&g_queue_mutex);
    if (g_list_length(g_send_queue) == 0) {
        pthread_mutex_unlock(&g_queue_mutex);
        return NULL;
    }

    // 取第一个元素（高优先级消息始终在前面）
    GList* first = g_send_queue;
    SendQueueItem* item = (SendQueueItem*)first->data;
    g_send_queue = g_list_remove_link(g_send_queue, first);
    g_list_free(first);  // 释放链表节点（数据由调用者释放）

    pthread_mutex_unlock(&g_queue_mutex);
    return item;
}

// 发送线程：循环从队列取消息并发送
static void* can_send_thread(void* arg) {
    (void)arg;
    while (g_running) {
        pthread_mutex_lock(&g_queue_mutex);
        // 等待队列有消息（带超时，避免退出时阻塞）
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += 1;  // 1秒超时
        pthread_cond_timedwait(&g_queue_cond, &g_queue_mutex, &timeout);
        pthread_mutex_unlock(&g_queue_mutex);

        // 处理队列中的消息
        while (g_running) {
            SendQueueItem* item = send_queue_pop();
            if (!item) break;

            // 发送数据（原发送逻辑）
            if (g_tcp_socket != -1) {
                LOG_DebugHex(item->data, item->len, "can msg tcp send:data");
                int ret = send(g_tcp_socket, item->data, item->len, 0);
                if (ret < 0) {
                    LOG_Debug("TCP send fail: %s\n", strerror(errno));
                }
            }
            // 释放资源
            free(item->data);
            free(item);
            usleep(1000);  // 轻微延迟，避免过度占用CPU
        }
    }
    return NULL;
}


// 模块初始化
int CanMsg_Init(const CanMsgConfig_t* config) {
    LOG_INIT("canmsg");
    LOG_Debug("canmsg_so_start\r\n");
    if (g_initialized) {
        LOG_Debug("CAN already init");
        return -1;
    }
    if (!config) {
        LOG_Debug("parm is NULL");
        g_config.error_indication_cbk(CAN_ERROR_FILTER);
        return -2;
    }
    
    // 复制配置
    memcpy(&g_config, config, sizeof(CanMsgConfig_t));
    
    // 验证配置参数
    if (g_config.buffer_size == 0) {
        LOG_Debug("buffer is 0");
        g_config.error_indication_cbk(CAN_ERROR_OVERRUN);
        return -3;
    }
    if (g_config.flush_cycle == 0) {
        LOG_Debug("flush_cycle is 0");
        g_config.error_indication_cbk(CAN_ERROR_UNKNOWN);
        return -3;
    }
    
    // 初始化缓冲区
    g_msg_buffer = malloc(sizeof(CanMsg_t) * g_config.buffer_size);
    if (!g_msg_buffer) {
        LOG_Debug("malloc fail");
        g_config.error_indication_cbk(CAN_ERROR_OVERRUN);
        return -4;
    }
    
    // 初始化TCP连接
    if (tcp_init() != 0) {
        free(g_msg_buffer);
        g_msg_buffer = NULL;
        return -5;
    }
    
    g_initialized = 1;
    LOG_Debug("CAN init success");
    return 0;
}

// 修改CanMsg_Start函数
int CanMsg_Start(void) {
    // 保留原有启动逻辑（如启动接收线程等）
    if (g_initialized == 0 || g_running != 0) {
        return -1;
    }
    
    g_running = 1;
    if (pthread_create(&g_recv_thread, NULL, can_tcp_recv_thread, NULL) != 0) {
        g_running = 0;
        g_config.error_indication_cbk(CAN_ERROR_UNKNOWN);
        return -2;
    }
    
    if (pthread_create(&g_send_thread, NULL, can_send_thread, NULL) != 0) {
        pthread_join(g_recv_thread, NULL);
        g_config.error_indication_cbk(CAN_ERROR_UNKNOWN);
        g_running = 0;
        return -3;
    }

    // 新增：发送配置消息并等待响应
    int retry_count = 0;
    int success = 0;
    while (retry_count < 3) {
        // 发送查询消息(over=0)
        int ret = send_query_message(0);
        if (ret != 0) {
            retry_count++;
            continue;
        }

        // 等待3秒超时
        pthread_mutex_lock(&g_sync_mutex);
        g_response_received = 0;
        g_response_received_start = 1;
        struct timespec timeout;
        uint64_t now_ms = get_current_time_ms();
        timeout.tv_sec = (now_ms + 3000) / 1000;
        timeout.tv_nsec = ((now_ms + 3000) % 1000) * 1000000;
        
        int cond_ret = pthread_cond_timedwait(&g_sync_cond, &g_sync_mutex, &timeout);
        if (cond_ret == 0 && g_response_received) {
            success = 1;
        }
        pthread_mutex_unlock(&g_sync_mutex);

        if (success) {
            break;
        }
        retry_count++;
    }

    if (!success) {
        g_config.error_indication_cbk(CAN_ERROR_TIMEOUT); // CAN_ERROR_TIMEOUT对应3
        CanMsg_Stop();
        return -4;
    }

    return 0;
}

// 修改CanMsg_Stop函数
void CanMsg_Stop(void) {
    if (g_running == 0) {
        return;
    }

    // 新增：发送停止消息并等待响应
    int retry_count = 0;
    int success = 0;
    while (retry_count < 3) {
        // 发送查询消息(over=1)
        int ret = send_query_message(2);
        if (ret != 0) {
            retry_count++;
            continue;
        }

        // 等待3秒超时
        pthread_mutex_lock(&g_sync_mutex);
        g_response_received = 0;
        g_response_received_stop = 1;
        struct timespec timeout;
        uint64_t now_ms = get_current_time_ms();
        timeout.tv_sec = (now_ms + 3000) / 1000;
        timeout.tv_nsec = ((now_ms + 3000) % 1000) * 1000000;
        
        int cond_ret = pthread_cond_timedwait(&g_sync_cond, &g_sync_mutex, &timeout);
        if (cond_ret == 0 && g_response_received && (g_response_received_stop == 2)) {
            
            success = 1;
        }
        pthread_mutex_unlock(&g_sync_mutex);

        if (success) {
            break;
        }
        retry_count++;
    }

    if (!success) {
        g_config.error_indication_cbk(CAN_ERROR_TIMEOUT);
    }
    // sleep(1);
    // 保留原有停止逻辑
    g_running = 0;
    pthread_cond_signal(&g_queue_cond); // 唤醒发送线程
    pthread_join(g_recv_thread, NULL);
    pthread_join(g_send_thread, NULL);
    close(g_tcp_socket);
    g_tcp_socket = -1;
    g_config.error_indication_cbk(CAN_ERROR_BUS_OFF);
}



// 修改CanMsg_Transmit函数
int CanMsg_Transmit(CanMsg_t* msg) {
    if (!msg || g_initialized == 0 || g_running == 0) {
        return -1;
    }

    // 验证DLC合法性
    if ((msg->is_fd && msg->dlc > CANFD_MAX_PAYLOAD_LEN) || 
        (!msg->is_fd && msg->dlc > CAN_MAX_PAYLOAD_LEN)) {
        g_config.error_indication_cbk(CAN_ERROR_INVALID_DLC);
        return -2;
    }

    // 打包并发送消息
    uint8_t buffer[512];
    int pack_len = pack_can_message(msg, buffer, sizeof(buffer));
    if (pack_len <= 0) {
        return -3;
    }

    // 等待解析成功的响应
    pthread_mutex_lock(&g_transmit_mutex);
    g_transmit_response_received = 0;
    struct timespec timeout;
    uint64_t now_ms = get_current_time_ms();
    timeout.tv_sec = (now_ms + 1000) / 1000; // 1秒超时
    timeout.tv_nsec = ((now_ms + 1000) % 1000) * 1000000;
    
    int cond_ret = pthread_cond_timedwait(&g_transmit_cond, &g_transmit_mutex, &timeout);
    if (cond_ret != 0 || !g_transmit_response_received) {
        g_config.error_indication_cbk(CAN_ERROR_TIMEOUT);//超时反馈
        pthread_mutex_unlock(&g_transmit_mutex);
        return -4;
    }
    pthread_mutex_unlock(&g_transmit_mutex);

    return 0;
}


// 模块反初始化
void CanMsg_Deinit() {
    if (!g_initialized) return;
    
    g_running = 0;
    // 等待线程结束
    if (g_recv_thread) {
        pthread_join(g_recv_thread, NULL);
        g_recv_thread = 0;
    }
    
    // 关闭连接
    if (g_tcp_socket != -1) {
        close(g_tcp_socket);
        g_tcp_socket = -1;
    }
    
    // 释放资源
    free(g_msg_buffer);
    g_msg_buffer = NULL;
    g_buffer_idx = 0;
    g_initialized = 0;
    
    LOG_Debug("CAN Deint success");
}