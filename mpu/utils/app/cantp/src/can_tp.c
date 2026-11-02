#include <stdatomic.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>
#include <pthread.h>
#include "mcu_api.h"
#include "err_code.h"
#include "sr_log.h"
// #include "can_tp.h"
#include "inc/can_tp.h"
#include "crcTool.h"
#include <sys/epoll.h>  // 引入 epoll 相关定义（包含 epoll_event 结构体）

#define yang 1
#if yang
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>  // 用于设置超时
#define CANTP_OK (0)
#define CANTP_FAIL (-1)
#define RET_OK 0
#define RET_ERR -1
#define CANTP_SHM_PATH "/canTpLibShm"
#define erro_open 1
#define CANTP_IGNORE_LOCK_TRUE (1)
#define CANTP_IGNORE_LOCK_FALSE (0)
// ACK响应结构体（根据实际协议调整）
typedef struct {
    uint8_t header1;      // 帧头1：0x55
    uint8_t header2;      // 帧头2：0xAA
    uint8_t aid;          // AID（与发送端一致）
    uint8_t mid;          // MID（与发送端一致）
    uint8_t sub;          // SUB（与发送端一致）
    uint8_t reserved;     // 固定为0x00
    uint8_t status;       // 状态码（0x00表示成功）
    uint8_t counterIndex; // 计数器索引
    uint16_t crc;         // CRC校验（覆盖帧头到counterIndex的所有数据）
} TcpAck_t;

typedef struct {
    uint8_t *data;       // 数据缓冲区
    size_t len;          // 数据长度
} TcpRecvData_t;

// 线程安全队列节点
typedef struct TcpDataNode {
    TcpRecvData_t data;
    struct TcpDataNode *next;
} TcpDataNode_t;

// 线程安全队列：用于接收线程与处理线程通信
typedef struct {
    TcpDataNode_t *head;       // 队列头
    TcpDataNode_t *tail;       // 队列尾
    pthread_mutex_t mutex;     // 队列互斥锁
    pthread_cond_t cond;       // 队列条件变量（通知有新数据）
    volatile uint8_t exit_flag;// 退出标志
} TcpDataQueue_t;

// 添加消息解析器
typedef struct {
    uint8_t *buffer;           // 累积缓冲区
    size_t buffer_size;        // 缓冲区大小
    size_t data_len;           // 当前数据长度
    size_t expected_len;       // 期望的消息长度
    uint8_t parse_state;       // 解析状态
} MessageParser_t;

static MessageParser_t g_msg_parser = {0};

static volatile int g_canTpInitialized = 0; // 0表示未初始化，1表示已初始化

typedef enum
{
    E_CANTP_TRANSMIT_OK = 0x0,
    E_CANTP_TRANSMIT_NOT_OK
}CanTpTransmitVal_e;

typedef enum
{
    cantp_init_malloc_erro = 200,
    cantp_init_ipc_erro,
    cantp_init_send_lock_erro,
    cantp_init_resp_lock_erro,
    cantp_init_resp_init_erro,
    cantp_init_CantpCheck_init_erro,
    cantp_init_tcp_connect_erro,
    cantp_init_DataQueue_erro,
    cantp_init_TcpRecvThread_erro,
    cantp_init_TcpProcessThread_erro
}cantp_init;
// 全局数据队列
static TcpDataQueue_t g_tcp_data_queue;

// 处理线程ID
static pthread_t g_process_tid;
// 全局TCP连接状态（独立于第三方库句柄）
static int32_t g_tcp_socket = -1;               // TCP socket描述符（-1表示未连接）
static pthread_mutex_t g_tcp_mutex = PTHREAD_MUTEX_INITIALIZER;  // 线程安全锁
 int8_t countidx = 0;
// 在can_tp.c的全局变量区域添加
static volatile uint8_t g_recv_thread_exit = 0;  // 线程退出标志
static pthread_t g_recv_tid;                     // 接收线程ID
static uint16 count_send = 0;
static volatile uint32 get_rxbk = 0; 
#endif

typedef struct
{
    atomic_int mutex_initialized;
    pthread_mutex_t mutex;
    pid_t pid;    
    time_t lastActive;
}CanTpLibShm_t;

typedef struct
{
    uint8_t countIdx;
    uint8_t threadExit;   
    pthread_t tid;
    pthread_mutex_t sendMutex;
    pthread_mutex_t respMutex;
    pthread_cond_t respCond;
    CanTpLibShm_t *canTpShm;
    CanTp_ReceiveIndicationCbk_t rxIndicationCbk;
    CanTp_TransmitErrorCbk_t transmitErrCbk;
    int32_t ignore;
}CanTpPriv_t;
static CanTpPriv_t *priv = NULL;//CanTpPriv_t *priv
typedef struct
{
    uint8_t reserve;
    uint8_t status;
    uint8_t countIdx;
    uint8_t uds[0];
}CanTpServiceData_t;

#if yang

// 初始化解析器
static int32_t MessageParserInit(MessageParser_t *parser) {
    if (!parser) return RET_ERR;
    
    parser->buffer_size = 4096;  // 初始缓冲区大小
    parser->buffer = malloc(parser->buffer_size);
    if (!parser->buffer) return RET_ERR;
    
    parser->data_len = 0;
    parser->expected_len = 0;
    parser->parse_state = 0;  // 0: 寻找帧头
    
    return RET_OK;
}


// 解析完整消息
static int32_t ParseCompleteMessage(MessageParser_t *parser) {
    if (parser->data_len < 2) return 0;  // 至少需要2字节帧头
    
    // 查找帧头 0x55 0xAA
    size_t start_idx = 0;
    while (start_idx <= parser->data_len - 2) {
        if (parser->buffer[start_idx] == 0x55 && 
            parser->buffer[start_idx + 1] == 0xAA) {
            break;
        }
        start_idx++;
    }
    
    // 移除无效数据
    if (start_idx > 0) {
        memmove(parser->buffer, parser->buffer + start_idx, parser->data_len - start_idx);
        parser->data_len -= start_idx;
    }
    
    // 检查是否找到有效帧头
    if (parser->data_len < 2 || parser->buffer[0] != 0x55 || parser->buffer[1] != 0xAA) {
        return 0;
    }
    
    // 检查是否有足够的数据获取长度字段
    if (parser->data_len < 7) {  // 帧头2 + AID/MID/SUB3 + 长度2
        return 0;  // 等待更多数据
    }
    
    // 提取数据长度
    uint16_t data_len = (parser->buffer[5] << 8) | parser->buffer[6];
    size_t total_msg_len = 2 + 3 + 2 + 1 + data_len + 2;  // 帧头 + ID + 长度 + 保留 + 数据 + CRC
    
    // 检查是否有完整消息
    if (parser->data_len < total_msg_len) {
        return 0;  // 等待更多数据
    }
    
    // 验证CRC
    uint16_t received_crc = (parser->buffer[total_msg_len - 2] << 8) | 
                           parser->buffer[total_msg_len - 1];
    uint16_t calculated_crc = Crc16(parser->buffer, total_msg_len - 2, Crc16Ccitt);
    
    if (received_crc != calculated_crc) {
        LOG_Error("CRC check failed, skipping message");
        // 移除错误的帧头，继续解析
        memmove(parser->buffer, parser->buffer + 2, parser->data_len - 2);
        parser->data_len -= 2;
        return 0;
    }
    
    // 返回完整消息长度
    return total_msg_len;
}
// 初始化数据队列
static int32_t TcpDataQueueInit(TcpDataQueue_t *queue) {
    if (!queue) return RET_ERR;
    queue->head = NULL;
    queue->tail = NULL;
    queue->exit_flag = 0;
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        LOG_Error("TcpDataQueue mutex init failed");
        return RET_ERR;
    }
    if (pthread_cond_init(&queue->cond, NULL) != 0) {
        LOG_Error("TcpDataQueue cond init failed");
        pthread_mutex_destroy(&queue->mutex);
        return RET_ERR;
    }
    return RET_OK;
}

// 数据入队
static int32_t TcpDataQueueEnqueue(TcpDataQueue_t *queue, const uint8_t *data, size_t len) {
    if (!queue || !data || len == 0) return RET_ERR;

    // 分配节点和数据缓冲区
    TcpDataNode_t *node = (TcpDataNode_t*)malloc(sizeof(TcpDataNode_t));
    if (!node) return RET_ERR;
    node->data.data = (uint8_t*)malloc(len);
    if (!node->data.data) {
        free(node);
        return RET_ERR;
    }
    memcpy(node->data.data, data, len);
    node->data.len = len;
    node->next = NULL;

    // 加锁入队
    pthread_mutex_lock(&queue->mutex);
    if (!queue->head) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
    pthread_cond_signal(&queue->cond); // 通知处理线程有新数据
    pthread_mutex_unlock(&queue->mutex);
    return RET_OK;
}

// 数据出队（阻塞等待，直到有数据或退出）
static int32_t TcpDataQueueDequeue(TcpDataQueue_t *queue, TcpRecvData_t *data) {
    if (!queue || !data) return RET_ERR;

    pthread_mutex_lock(&queue->mutex);
    // 等待数据或退出信号
    while (!queue->head && !queue->exit_flag) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    if (queue->exit_flag) {
        pthread_mutex_unlock(&queue->mutex);
        return RET_ERR; // 退出标志触发
    }

    // 取出队头数据
    TcpDataNode_t *node = queue->head;
    data->data = node->data.data;
    data->len = node->data.len;
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    free(node); // 释放节点（数据缓冲区由处理线程释放）
    pthread_mutex_unlock(&queue->mutex);
    return RET_OK;
}


static void TcpDataQueueDestroy(TcpDataQueue_t *queue) {
    if (!queue) return;

    // 清理队列剩余节点（此时线程已退出，可安全操作）
    pthread_mutex_lock(&queue->mutex);
    TcpDataNode_t *node = queue->head;
    while (node) {
        TcpDataNode_t *tmp = node;
        node = node->next;
        free(tmp->data.data);  // 释放数据缓冲区
        free(tmp);             // 释放节点
    }
    queue->head = queue->tail = NULL;
    pthread_mutex_unlock(&queue->mutex);

    // 最后销毁同步对象（确保无线程持有或等待）
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);

    LOG_Debug("[%s]TcpDataQueueDestroy completed",__FUNCTION__);
}


static void *TcpProcessThread(void *arg) {
    // CanTpPriv_t *priv = (CanTpPriv_t*)arg;
    if (!priv) {
        LOG_Error("TcpProcessThread invalid priv");
        return NULL;
    }
    uint32_t can_id = 0;
    PduInfoType pdu_info;
    // uint8_t *data = NULL; 
    TcpRecvData_t recv_data;
    LOG_Debug("[%s]TcpProcessThread start (tid: %ld)", pthread_self(),__FUNCTION__); // 打印线程ID
    while (!g_recv_thread_exit) { // 共享退出标志
        // 从队列取数据（阻塞等待）
        #if carota_debug
            LOG_Debug("[%s]carota_TcpProcessThread start\r\n",__FUNCTION__);
        #endif
        if (TcpDataQueueDequeue(&g_tcp_data_queue, &recv_data) != RET_OK) {
            break; // 退出时会返回错误
            // continue;
        }
        LOG_DebugHex(recv_data.data, recv_data.len, "TcpProcessThread recv:");
        switch (recv_data.data[4])
        {
        case 0x03:
            uint8_t *data = &recv_data.data[15];
            can_id = (recv_data.data[11] << 24) | (recv_data.data[12] << 16) | (recv_data.data[13] << 8) | (recv_data.data[14]);
            if (priv->rxIndicationCbk) {
                uint16_t data_len = (recv_data.data[5] << 8) | recv_data.data[6];
                pdu_info.SduDataPtr = data;
                // pdu_info.SduLength = data_len-6;
                pdu_info.SduLength = data_len-7;
                LOG_DebugHex(pdu_info.SduDataPtr, pdu_info.SduLength ,"0x03 send:");
                priv->rxIndicationCbk(can_id, &pdu_info);
                #if carota_debug
                    LOG_Debug("[%s]carota_priv->rxIndicationCbk_start\r\n",__FUNCTION__);
                #endif
            }
            break;
        case 0x02:
            pthread_mutex_lock(&priv->respMutex); // 必须先加锁

                get_rxbk=1;
            pthread_cond_signal(&priv->respCond); // 唤醒等待线程
            
            pthread_mutex_unlock(&priv->respMutex);
            
            LOG_Debug("can_tp,count_send:%d,priv->countIdx:%d,get_rxbk:%d\r\n",count_send,priv->countIdx,get_rxbk);
            break;
        default:
            break;
        }
        // 释放数据缓冲区
        free(recv_data.data);
    }

    // LOG_Debug("TcpProcessThread exit");
    LOG_Debug("[%s]TcpProcessThread exit (tid: %ld)", pthread_self(),__FUNCTION__); // 确认线程退出
    return NULL;
}

// 添加明确的TcpSocketClose函数定义
static void TcpSocketClose(int32_t *sock) {
    if (sock && *sock != -1) {
        close(*sock);
        *sock = -1;
        LOG_Debug("TcpSocketClose: socket %d closed", *sock);
    }
}


// 检查并建立非阻塞TCP连接
static int32_t TcpCheckAndConnect(void) 
{
    // 检查现有连接是否有效（逻辑不变）
    if (g_tcp_socket != -1) {
        int32_t error = 0;
        socklen_t len = sizeof(error);
        // 通过SO_ERROR判断连接状态（非阻塞模式下同样适用）
        if (getsockopt(g_tcp_socket, SOL_SOCKET, SO_ERROR, &error, &len) == 0 && error == 0) {
            return RET_OK;  // 连接有效
        }
        // 连接无效，关闭旧socket
        close(g_tcp_socket);
        g_tcp_socket = -1;
        LOG_Debug("[%s]:TCP connection invalid, close old socket", __FUNCTION__);
    }

    // 1. 创建socket（同阻塞模式）
    int32_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG_Error("%s create socket failed: %s", __FUNCTION__, strerror(errno));
        return RET_ERR;
    }

    // 3. 配置服务器地址（同阻塞模式，保留条件编译）
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5050);
    if (inet_pton(AF_INET, "192.168.1.9", &server_addr.sin_addr) <= 0) {

        LOG_Error("%s invalid server address", __FUNCTION__);
        close(sock);
        return RET_ERR;
    }

    // 4. 非阻塞connect（核心改动2：不会阻塞等待，立即返回）
    int ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        // 非阻塞模式下，connect返回-1且errno为EINPROGRESS是正常状态（连接正在建立）
        if (errno != EINPROGRESS && errno != EWOULDBLOCK) {  // 不同系统可能返回EWOULDBLOCK

            LOG_Error("%s connect failed immediately: %s", __FUNCTION__, strerror(errno));
            close(sock);
            return RET_ERR;
        }
    } else if (ret == 0) {
        // 极少数情况：非阻塞connect立即成功（如连接本地服务）
        g_tcp_socket = sock;
        LOG_Debug("%s connected immediately to 192.168.1.9:5050", __FUNCTION__);
        return RET_OK;
    }

    // 5. 用select等待连接完成（核心改动3：监听socket可写事件）
    fd_set write_fds;  // 非阻塞connect成功/失败时，socket会变为可写
    struct timeval timeout;
    FD_ZERO(&write_fds);
    FD_SET(sock, &write_fds);

    // 设置超时时间（根据需求调整，这里设为3秒）
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;

    // 等待可写事件或超时
    ret = select(sock + 1, NULL, &write_fds, NULL, &timeout);
    if (ret == 0) {
        // 超时：连接未在规定时间内完成

        LOG_Error("%s connect timeout", __FUNCTION__);
        close(sock);
        return RET_ERR;
    } else if (ret < 0) {
        // select出错
            LOG_Error("%s select failed: %s", __FUNCTION__, strerror(errno));
        close(sock);
        return RET_ERR;
    }

    // 6. 检查连接结果（核心改动4：通过SO_ERROR确认是否成功）
    if (FD_ISSET(sock, &write_fds)) {  // socket可写，需进一步判断状态
        int32_t error = 0;
        socklen_t len = sizeof(error);
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) == -1) {
            LOG_Error("%s getsockopt failed: %s", __FUNCTION__, strerror(errno));
            close(sock);
            return RET_ERR;
        }
        if (error != 0) {
            // 连接失败（如拒绝连接、超时等）

            LOG_Error("%s connect failed: %s", __FUNCTION__, strerror(error));
            close(sock);
            return RET_ERR;
        }
        // 连接成功
        g_tcp_socket = sock;
        LOG_Debug("%s connected to 192.168.1.9:5050", __FUNCTION__);
        return RET_OK;
    }

    // 理论上不会走到这里，保险处理
    close(sock);
    return RET_ERR;
}

// 接收并验证ACK响应（适配新格式，超时2秒）
static int32_t TcpReceiveAck(TcpAck_t *ack, uint8_t expected_aid, uint8_t expected_mid, uint8_t expected_sub) {
    if (!ack || g_tcp_socket == -1) {
        LOG_Error("%s invalid param or socket not connected", __FUNCTION__);
        return RET_ERR;
    }

    // 设置接收超时为2秒
    struct timeval timeout = {2, 0};  // 2秒超时
    if (setsockopt(g_tcp_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        LOG_Warn("%s set recv timeout failed: %s", __FUNCTION__, strerror(errno));
        // 超时设置失败仍继续，可能导致阻塞
    }

    // 实际ACK总长度：9字节（2帧头 + 3(AID/MID/SUB) + 1(reserved) + 1(status) + 1(counter) + 2(CRC)）
    uint8_t ack_buffer[sizeof(TcpAck_t)] = {0};
    ssize_t recv_len = recv(g_tcp_socket, ack_buffer, sizeof(ack_buffer), 0);
    if (recv_len != sizeof(ack_buffer)) {
        LOG_Error("%s ACK receive length mismatch (recv=%zd, expected=%zu): %s",__FUNCTION__, recv_len, sizeof(ack_buffer),(recv_len < 0) ? strerror(errno) : "incomplete data");
        return RET_ERR;
    }

    // 解析ACK数据（按实际格式偏移）
    ack->header1 = ack_buffer[0];
    ack->header2 = ack_buffer[1];
    ack->aid = ack_buffer[2];
    ack->mid = ack_buffer[3];
    ack->sub = ack_buffer[4];
    ack->reserved = ack_buffer[5];
    ack->status = ack_buffer[6];
    ack->counterIndex = ack_buffer[7];
    ack->crc = (ack_buffer[8] << 8) | ack_buffer[9];

    // 1. 验证帧头
    if (ack->header1 != 0x55 || ack->header2 != 0xAA) {
        LOG_Error("%s ACK header invalid (0x%02X, 0x%02X), expected (0x55, 0xAA)",__FUNCTION__, ack->header1, ack->header2);

        return RET_ERR;
    }

    // 2. 验证AID/MID/SUB与发送端一致（防止串包）
    if (ack->aid != expected_aid || ack->mid != expected_mid) 
    {
        LOG_Error("%s ACK ID mismatch (aid=0x%02X, mid=0x%02X, sub=0x%02X), expected (0x%02X, 0x%02X, 0x%02X)",__FUNCTION__, ack->aid, ack->mid, ack->sub,
                  expected_aid, expected_mid, expected_sub);
        return RET_ERR;
    }

    // 3. 验证reserved字段为0x00
    if (ack->reserved != 0x00) {
        LOG_Error("%s ACK reserved field invalid (0x%02X), expected 0x00",__FUNCTION__, ack->reserved);
        return RET_ERR;
    }
    LOG_Debug("%s ACK verified successfully (status=0x%02X, counter=%hhu)",__FUNCTION__, ack->status, ack->counterIndex);
    return RET_OK;
}


// 核心函数实现（仅发送，不接收ACK）
int32_t can_tp_eth(const McuMesg_t *msg) {
    LOG_Debug("[%s]carota_CanTp_can_tp_eth_start\r\n",__FUNCTION__);
    // 参数校验
    if (!msg || !msg->payload || msg->payloadLen == 0) {
        LOG_Error("%s invalid param (msg=%p, payload=%p, len=%u)",__FUNCTION__, msg, msg->payload, msg->payloadLen);
        return RET_ERR;
    }
    LOG_Debug("[%s]carota_msg || !msg->payload || msg->payloadLent\r\n",__FUNCTION__);
    // 加锁保证线程安全
    if (pthread_mutex_lock(&g_tcp_mutex) != 0) {
        LOG_Error("%s mutex lock failed", __FUNCTION__);
        return RET_ERR;
    }
    LOG_Debug("[%s]carota_g_tcp_mutex start\r\n",__FUNCTION__);
    uint32_t total_len = 2 + 3 + 2 + msg->payloadLen + 1 + 2;//最终版本
    uint8_t *send_buffer = (uint8_t *)malloc(total_len);
    if (!send_buffer) {
        LOG_Error("%s malloc failed (size=%u)", __FUNCTION__, total_len);
        pthread_mutex_unlock(&g_tcp_mutex);
        return RET_ERR;
    }
    LOG_Debug("[%s]carota_g_tcp_mutex over\r\n",__FUNCTION__);
    // 填充数据帧
    uint32_t offset = 0;
    // ① 帧头 0x55, 0xAA
    send_buffer[offset++] = 0x55;
    send_buffer[offset++] = 0xAA;

    // ② AID/MID/SUB（从msg->type解析）
    uint8_t aid = (msg->type >> 16) & 0xFF;
    uint8_t mid = (msg->type >> 8) & 0xFF;
    uint8_t sub = (msg->type & 0xFF);
    send_buffer[offset++] = aid;
    send_buffer[offset++] = mid;
    send_buffer[offset++] = sub;

    // ③ payload长度（2字节，大端模式）
    send_buffer[offset++] = (msg->payloadLen >> 8) & 0xFF;  // 高8位
    send_buffer[offset++] = msg->payloadLen & 0xFF;         // 低8位

    send_buffer[offset++] = 0x00;//临时注释，需要保留该字节
    // ④ payload数据
    memcpy(&send_buffer[offset], msg->payload, msg->payloadLen);
    offset += msg->payloadLen;

    // ⑤ CRC16校验（覆盖帧头到payload的所有数据）
    uint16_t crc = Crc16(send_buffer, offset, Crc16Ccitt);
    send_buffer[offset++] = (crc >> 8) & 0xFF;  // CRC高8位
    send_buffer[offset++] = crc & 0xFF;         // CRC低8位

    LOG_DebugHex(send_buffer, total_len, "TCP send data");
    ssize_t sent = send(g_tcp_socket, send_buffer, total_len,MSG_NOSIGNAL);
    LOG_Debug("[%s]carota_CanTp_send:[%d][%d]\r\n",__FUNCTION__,sent,total_len);
    if (sent != total_len) {
        int f_ret=close(g_tcp_socket);  // 发送失败，关闭连接下次重连
        LOG_Debug("[%s]carota_CanTp_send_fail,close socket:%d\r\n",__FUNCTION__,f_ret);
        g_tcp_socket = -1;
        free(send_buffer);
        // can_tp_eth_cleanup();
        pthread_mutex_unlock(&g_tcp_mutex);
        return RET_ERR;
    }

    // 清理资源
    // 发送数据后释放锁
    free(send_buffer);
    pthread_mutex_unlock(&g_tcp_mutex);
    LOG_Debug("[%s]carota_CanTp_can_tp_eth_over\r\n",__FUNCTION__);
    return RET_OK;
}
// 程序退出时关闭TCP连接
void can_tp_eth_cleanup(void) {
    if (pthread_mutex_lock(&g_tcp_mutex) == 0) {
        if (g_tcp_socket != -1) {
            close(g_tcp_socket);
            g_tcp_socket = -1;
            LOG_Debug("%s TCP connection closed", __FUNCTION__);
        }
        pthread_mutex_unlock(&g_tcp_mutex);
    }
}

// 改进的接收线程
static void *TcpRecvThread(void *arg) {
    if (!priv) return NULL;
    
    uint8_t recv_buf[1024];
    ssize_t recv_len;
    
    // 初始化消息解析器
    if (MessageParserInit(&g_msg_parser) != RET_OK) {
        LOG_Error("[%s]MessageParserInit failed",__FUNCTION__);
        return NULL;
    }
    
    while (!g_recv_thread_exit) {
        if (TcpCheckAndConnect() != RET_OK) {
            sleep(1);
            continue;
        }
        
        recv_len = recv(g_tcp_socket, recv_buf, sizeof(recv_buf), 0);
        if (recv_len <= 0) {
            if (recv_len < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                LOG_Warn("[%s]Tcp recv failed: %s", strerror(errno),__FUNCTION__);
                TcpSocketClose(&g_tcp_socket);
            }
            sleep(1);
            continue;
        }
        
        // 将数据追加到解析器缓冲区
        if (g_msg_parser.data_len + recv_len > g_msg_parser.buffer_size) {
            // 扩展缓冲区
            size_t new_size = g_msg_parser.buffer_size * 2;
            uint8_t *new_buffer = realloc(g_msg_parser.buffer, new_size);
            if (!new_buffer) {
                LOG_Error("[%s]Buffer realloc failed",__FUNCTION__);
                continue;
            }
            g_msg_parser.buffer = new_buffer;
            g_msg_parser.buffer_size = new_size;
        }
        
        memcpy(g_msg_parser.buffer + g_msg_parser.data_len, recv_buf, recv_len);
        g_msg_parser.data_len += recv_len;
        
        // 解析完整消息
        while (1) {
            int32_t msg_len = ParseCompleteMessage(&g_msg_parser);
            if (msg_len <= 0) break;
            
            LOG_DebugHex(g_msg_parser.buffer, msg_len, "[TcpRecvThread] Parsed msg before enqueue:");
            
            // 提取完整消息并入队
            if (TcpDataQueueEnqueue(&g_tcp_data_queue, g_msg_parser.buffer, msg_len) != RET_OK) {
                LOG_Error("TcpDataQueueEnqueue failed");
            }
            
            // 移除已处理的消息
            memmove(g_msg_parser.buffer, g_msg_parser.buffer + msg_len, 
                   g_msg_parser.data_len - msg_len);
            g_msg_parser.data_len -= msg_len;
            usleep(10);//适当延时
        }
    }
    
    // 清理解析器
    free(g_msg_parser.buffer);
    g_msg_parser.buffer = NULL;
    g_msg_parser.data_len = 0;
    g_msg_parser.buffer_size = 0;
    
    return NULL;
}

#endif
/**
 * @name CanTpInitLibShm
 * @brief 初始化共享内存
 *        本函数用于初始化CAN传输层的共享内存。如果共享内存已经初始化，则函数会直接返回成功。
 *        否则，它将尝试创建和映射一个新的共享内存段用于CAN传输层的通信。
 * 
 * @param priv 指向CanTpPriv_t结构体的指针，该结构体包含了CAN传输层的私有数据。
 * @return 返回RET_OK表示成功初始化，RET_ERR表示初始化失败。
 */
static int32_t CanTpInitLibShm(CanTpPriv_t *priv)
{
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }

    if (priv->canTpShm != MAP_FAILED)
    {
        LOG_Error("%s shm hava init.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }
    LOG_Debug("CANTP_SHM_PATH:%s\r\n",CANTP_SHM_PATH);
    // int32_t fd = shm_open(CANTP_SHM_PATH, O_CREAT | O_RDWR, 0644);
    int32_t fd = shm_open(CANTP_SHM_PATH, O_CREAT | O_RDWR, 0775);
    if (fd == -1)
    {
        LOG_Error("Open cantpLibShm File Error:%s", g_strerror(errno));
        return ret;
    }
    if (ftruncate(fd, sizeof(CanTpLibShm_t)) != 0)
    {
        LOG_Error("ftruncate TreeShm File error:%s", g_strerror(errno));
        close(fd);
        return ret;
    }
    priv->canTpShm = mmap(NULL, sizeof(CanTpLibShm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (priv->canTpShm == MAP_FAILED)
    {
        LOG_Error("mmap cantpLibShm Error:%s", g_strerror(errno));
        close(fd);
        return ret;
    }

    if (atomic_load(&(priv->canTpShm->mutex_initialized)) == 0)
    {
        int32_t expected = 0;
        priv->canTpShm->pid = -1;
        if (atomic_compare_exchange_strong(&(priv->canTpShm->mutex_initialized), &expected, 1))
        {
            pthread_mutexattr_t mutex_attr;
            pthread_mutexattr_init(&mutex_attr);
            pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(&priv->canTpShm->mutex, &mutex_attr);
            pthread_mutexattr_destroy(&mutex_attr);
        }
    }

    ret = RET_OK;
    close(fd);
    return ret;
}

/**
 * @name CanTpCheckActived
 * @brief 线程函数，用于检查CAN传输层的活动状态。
 *        本函数是一个线程函数，用于检查CAN传输层的活动状态。它通过定期检查CAN传输层的活动状态来保持活动状态。
 *        如果CAN传输层被激活，则函数会检查上次活动时间是否超过300秒，如果超过300秒，则认为CAN传输层被激活。
 *        如果CAN传输层被激活，则函数会检查上次活动
 *        时间是否超过300秒，如果超过300秒，则认为CAN传输层被激活。
 * @param args  参数
 * @return 函数无返回值
 */
static void* CanTpCheckActived(void* args)
{
    CanTpPriv_t *priv = (CanTpPriv_t *)args;
    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return NULL;
    }

    while (!priv->threadExit)
    {

        if ((CANTP_IGNORE_LOCK_FALSE == priv->ignore) && (getpid() == priv->canTpShm->pid))
        {        
            if ((priv->canTpShm->lastActive > 0) && ((time(NULL) - priv->canTpShm->lastActive) > 300))
            {
                pthread_mutex_lock(&priv->canTpShm->mutex);
                priv->canTpShm->pid = -1;
                pthread_mutex_unlock(&priv->canTpShm->mutex);                
            }
        }
        usleep(500000);
    }
    return NULL;
}

/**
 * @name CanTpRecvResp
 * @brief 处理接收到的CAN TP响应。
 * @param data 接收到的数据指针。
 * @param len 接收到的数据长度。
 * @param userdata 用户数据指针，用于访问私有数据。
 * @return 函数无返回值
 */
static void CanTpRecvResp(const uint8_t* data, const uint16_t len, void* userdata)
{
    CanTpPriv_t *priv = (CanTpPriv_t *)userdata;
    CanTpServiceData_t *srvData = (CanTpServiceData_t *)data;
    
    if (!priv || !srvData || (len < (sizeof(CanTpServiceData_t) + 4)))
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return;
    }

    if ((CANTP_IGNORE_LOCK_FALSE == priv->ignore) && (getpid() != priv->canTpShm->pid))
    {
        LOG_Error("%s unlocked.", __FUNCTION__);
        return;        
    }


    //send ack
    uint8_t buffer[sizeof(CanTpServiceData_t)] = {0x0, 0x0, priv->countIdx};
    //CanTpServiceData_t srvdata = {.reserve = 0x0,.status = 0x0, .countIdx = priv->countIdx};
    McuMesg_t mesg;
    mesg.type = SYS_QUE_TYPE_MCU_CANTP_MCU_TO_MPU_ACK;//0x00070104
    mesg.payloadLen = sizeof(CanTpServiceData_t);
    mesg.payload = (uint8_t*)buffer;
    McuSrvSendMesg(&mesg);
    LOG_DebugHex(mesg.payload, mesg.payloadLen, "resp ack");
    LOG_DebugHex(data, len, "CanTpRecvResp");
    uint32_t canId = (srvData->uds[0] & 0xFF) << 24;
    canId += (srvData->uds[1] & 0xFF) << 16;
    canId += (srvData->uds[2] & 0xFF) << 8;
    canId += srvData->uds[3] & 0xFF;
    PduInfoType pdu_info;
    pdu_info.SduDataPtr = (void *)&srvData->uds[4];
    pdu_info.SduLength = len - sizeof(CanTpServiceData_t) - 4 ;
    priv->rxIndicationCbk(canId,&pdu_info);
}

static int32_t CanTpTransSend(CanTpPriv_t *priv, uint32 canId, uint8_t *data, uint16 dataLen, uint32_t repeat, uint8_t package)
{
    int32_t ret = RET_ERR;
    LOG_Debug("[%s]carota_CanTpTransSend_start\r\n",__FUNCTION__);
    if (!priv || !data || (dataLen == 0))
    {
        LOG_Debug("%s invalid param.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[5000] = {0x0,};
    CanTpServiceData_t *srvData = (CanTpServiceData_t *)buffer;
    srvData->reserve = package;
    srvData->status = repeat ? 0x80 : 0x0;
    srvData->countIdx = priv->countIdx;
    count_send = srvData->countIdx;
    srvData->uds[0] = (canId >> 24) & 0xFF;
    srvData->uds[1] = (canId >> 16) & 0xFF;
    srvData->uds[2] = (canId >> 8) & 0xFF;
    srvData->uds[3] = canId & 0xFF;
    memcpy(&srvData->uds[4], data, dataLen);

    McuMesg_t mesg;
    mesg.type = SYS_QUE_TYPE_MCU_CANTP_MPU_TO_MCU;
    mesg.payloadLen = dataLen + sizeof(CanTpServiceData_t) + 4;
    mesg.payload = buffer;

    LOG_Debug("[%s]carota_can_tp_eth_start:%d\r\n",__FUNCTION__,ret);

    // 发送前重置响应标志
    get_rxbk = 0;

    // 发送消息
    if(RET_OK != can_tp_eth(&mesg))
    {   
        LOG_Debug("%s McuSrvSendMesg fail.", __FUNCTION__);
        return ret;
    }
    LOG_Debug("[%s]carota_can_tp_eth_over1\r\n",__FUNCTION__);
    if ((mesg.payloadLen == 9) && (dataLen >= 2) && (data[0] == 0x3E) && (data[1] == 0x80))
    {
        LOG_Debug("[%s] 0x3E 0x80 no replay:%d", __FUNCTION__,ret);
        ret = RET_OK;
        return ret;
    }
    else
    {
        uint64_t timeout_ms = 3000;
        struct timespec outtime;
        uint64_t nsec = 0;
        int wait_ret = 0;

        clock_gettime(CLOCK_MONOTONIC, &outtime);
        nsec = outtime.tv_nsec + (timeout_ms % 1000) * 1000000;
        outtime.tv_sec = outtime.tv_sec + timeout_ms / 1000 + nsec / 1000000000;
        outtime.tv_nsec = nsec % 1000000000;

        LOG_Debug("[%s]carota_respMutex_start, get_rxbk=%d,%d\r\n",__FUNCTION__, get_rxbk,ret);
        pthread_mutex_lock(&priv->respMutex);
        
        // 标准条件变量等待模式：循环检查条件
        while (get_rxbk == 0) {
            wait_ret = pthread_cond_timedwait(&priv->respCond, &priv->respMutex, &outtime);
            
            if (wait_ret != 0) {
                
                break;
            }
        }

        if (get_rxbk) {
            // 成功收到响应
            ret = RET_OK;
            priv->countIdx++;
            LOG_Debug("[%s] Response received successfully\r\n", __FUNCTION__);
        } else {
            // 超时或错误
            if (wait_ret == ETIMEDOUT) {
                LOG_Debug("[%s] cantp transmit time out\r\n", __FUNCTION__);
            } else {
                LOG_Debug("[%s] pthread_cond_timedwait failed, ret=%d\r\n", __FUNCTION__, wait_ret);
            }
            priv->transmitErrCbk(NTFRSLT_E_TIMEOUT_A);
        }
        
        // 重置响应标志
        get_rxbk = 0;
        pthread_mutex_unlock(&priv->respMutex);
    }

#if carota_debug
    LOG_Debug("[%s]carota_CanTpTransSend_over:%d\r\n",__FUNCTION__,ret);
#endif
    return ret;
}

/**
 * @name CanTp_Transmit  新版本，支持分包传输，单包最大1200
 * @brief CAN-TP发送函数
 * @param hdl CanTp处理句柄，用于访问私有数据
 * @param canId CAN消息的ID
 * @param data 指向要发送数据的指针
 * @param dataLen 要发送的数据长度
 * @return int32_t 返回传输结果，RET_OK表示成功，其他值表示失败
 */


Std_ReturnType CanTp_Transmit(uint32 canId, PduInfoType *pduInfoPtr)
{
    int32_t ret = RET_ERR;
    
    // 检查模块是否已初始化
    if (!g_canTpInitialized) {
        LOG_Error("[%s] Module not initialized.", __FUNCTION__);
        return RET_ERR;
    }
    
    PduInfoType *Cantp_tran_data = NULL;
    Cantp_tran_data = (PduInfoType*)malloc(sizeof(PduInfoType));
    if (Cantp_tran_data == NULL) {
    LOG_Debug("[%s]carota_CanTp_Cantp_tran_data failed\r\n",__FUNCTION__);
        return RET_ERR;
    }
    LOG_Debug("[%s]carota_CanTp_Transmit_start\r\n",__FUNCTION__);
    // 参数合法性检查
    if (!priv || !pduInfoPtr->SduDataPtr || (pduInfoPtr->SduLength == 0))
    {
        LOG_Debug("%s invalid param.", __FUNCTION__);
        free(Cantp_tran_data);
        Cantp_tran_data = NULL;
        return RET_ERR;
    }
    else
    {
        Cantp_tran_data->SduLength = pduInfoPtr->SduLength;
        Cantp_tran_data->SduDataPtr = (uint8*)malloc(pduInfoPtr->SduLength * sizeof(uint8));
        if (Cantp_tran_data->SduDataPtr == NULL) 
        {
            LOG_Debug("%s Cantp_tran_data invalid malloc.", __FUNCTION__);
            free(Cantp_tran_data);
            Cantp_tran_data = NULL;
            return RET_ERR;
        }
        memcpy(Cantp_tran_data->SduDataPtr, pduInfoPtr->SduDataPtr, pduInfoPtr->SduLength);
    }
    
    LOG_Debug("[%s]carota_pduInfoPtr->SduDataPtr || (pduInfoPtr->SduLength over\r\n",__FUNCTION__);
    priv->canTpShm->lastActive = time(NULL);
    LOG_Debug("[%s]carota_priv->sendMutex_start\r\n",__FUNCTION__);
    pthread_mutex_lock(&priv->sendMutex);

    uint16_t totalLen = Cantp_tran_data->SduLength;  // 总数据长度
    const uint16_t MAX_PACKAGE_LEN = 1200;      // 单包最大长度
    usleep(1000*10);
    if (totalLen > MAX_PACKAGE_LEN)
    {
        LOG_Debug("[%s]carota_diff_package_start\r\n",__FUNCTION__);
        // 计算总包数（向上取整）
        uint8_t totalPackages = (totalLen + MAX_PACKAGE_LEN - 1) / MAX_PACKAGE_LEN;
        uint8_t packageNum;
        ret = RET_OK;  // 先假设所有包都能发送成功

        // 循环发送每一包
        for (packageNum = 1; packageNum <= totalPackages; packageNum++)
        {
            uint32_t sendtimes = 3;  // 每包最多重试3次
            uint32_t repeat = 0;
            uint16_t offset = (packageNum - 1) * MAX_PACKAGE_LEN;  // 当前包数据偏移量
            // 计算当前包长度（最后一包可能小于1200）
            uint16_t currentLen = (packageNum == totalPackages) ? 
                                 (totalLen - offset) : MAX_PACKAGE_LEN;
            uint8_t *currentData = Cantp_tran_data->SduDataPtr + offset;  // 当前包数据指针
            int32_t pkgRet = RET_ERR;

            // 重试发送当前包
            while (sendtimes-- > 0)
            {
                if (RET_OK == CanTpTransSend(priv, canId, currentData, currentLen, repeat, packageNum))
                {
                    pkgRet = RET_OK;
                    break;  // 当前包发送成功，退出重试循环
                }
                repeat = 1;  // 第一次失败后，后续重试标记为repeat=1
            }

            // 如果当前包经重试后仍失败，整体传输失败
            if (pkgRet != RET_OK)
            {
                ret = RET_ERR;
                break;
            }
            usleep(1000*10);//每发一包给50ms延时
        }
        LOG_Debug("[%s]carota_diff_package_over\r\n",__FUNCTION__);
        // 所有包都发送成功时，触发回调
        if (ret == RET_OK)
        {
            usleep(1000);
            priv->transmitErrCbk(NTFRSLT_OK);
            
        }
        LOG_Debug("[%s]carota_diff_package_over2\r\n",__FUNCTION__);
    }
    else
    {
        LOG_Debug("[%s]carota_single_package_start\r\n",__FUNCTION__);
        // 数据长度 <= 1200，单包传输（package=0）
        uint32_t sendtimes = 1;
        uint32_t repeat = 0;
        while (sendtimes-- > 0)
        {
            if (RET_OK == CanTpTransSend(priv, canId, Cantp_tran_data->SduDataPtr, Cantp_tran_data->SduLength, repeat, 0))
            {
                ret = RET_OK;
                usleep(1000);
                priv->transmitErrCbk(NTFRSLT_OK);
                
                break;
            }
            repeat = 1;
        }
    LOG_Debug("[%s]carota_single_package_over\r\n",__FUNCTION__);
    }

    pthread_mutex_unlock(&priv->sendMutex);
    LOG_Debug("[%s]carota_priv->sendMutex_over\r\n",__FUNCTION__);
    free(Cantp_tran_data);
    Cantp_tran_data = NULL;
    LOG_Debug("[%s]clean Cantp_data:%d\r\n",__FUNCTION__,ret);
    usleep(1000*50);
    return ret;
}

/**
 * @name CanTpLock
 * @brief CAN TP加锁
 * @param hdl CanTpHandle_t类型，CAN-TP的句柄，用于标识特定的CAN-TP实例。
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t CanTpLock(void)
{
    // CanTpPriv_t *priv = (CanTpPriv_t *)hdl;
    int32_t ret = RET_ERR;
    LOG_Debug("[%s]CanTpLock start\r\n",__FUNCTION__);
    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }
    LOG_Debug("[%s]CanTpLock priv start\r\n",__FUNCTION__);
    if (priv->ignore)
    {
        LOG_Debug("%s ignore, return success.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }
    if (priv->canTpShm->pid == getpid())
    {
        ret = RET_OK;
        return ret;
    }
    LOG_Debug("[%s]CanTpLock getpid start\r\n",__FUNCTION__);
    pthread_mutex_lock(&priv->canTpShm->mutex);
    if ((priv->canTpShm->pid != -1) && (syscall(SYS_tkill, priv->canTpShm->pid, 0) != -1))
    {
        LOG_Error("%s lock fail,process:%d have locked.", __FUNCTION__, priv->canTpShm->pid);
        pthread_mutex_unlock(&priv->canTpShm->mutex);
        return ret; 
    }

    priv->canTpShm->lastActive = time(NULL);
    priv->canTpShm->pid = getpid();
    pthread_mutex_unlock(&priv->canTpShm->mutex);
    LOG_Debug("[%s]CanTpLock getpid start\r\n",__FUNCTION__);
    ret = RET_OK;
    return ret;    
}

/**
 * @name CanTpUnlock
 * @brief CAN TP解锁
 * @param hdl CanTpHandle_t类型，CAN TP的处理句柄，指向CanTpPriv_t结构体的指针
 * @return 操作成功返回RET_OK，失败返回RET_ERR。 
 */
int32_t CanTpUnlock(void)
{
    int32_t ret = RET_ERR;
    LOG_Debug("[%s]CanTpUnlock CanTpUnlock start\r\n",__FUNCTION__);
    if (!priv)
    {
        LOG_Debug("%s invalid param.", __FUNCTION__);
        return ret;
    }
    LOG_Debug("%s CanTpUnlock success", __FUNCTION__);
    if (priv->ignore)
    {
        LOG_Debug("%s ignore, return success.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }
    LOG_Debug("%s CanTpUnlock success2", __FUNCTION__);
    if (priv->canTpShm->pid != getpid())
    {
        LOG_Error("%s unlock fail,locked task:%d, curr:%d.", __FUNCTION__, priv->canTpShm->pid, getpid());
        return ret;
    }
    LOG_Debug("%s CanTpUnlock success3", __FUNCTION__);
    pthread_mutex_lock(&priv->canTpShm->mutex);
    priv->canTpShm->pid = -1;
    priv->canTpShm->lastActive = 0;
    pthread_mutex_unlock(&priv->canTpShm->mutex);
    LOG_Debug("%s CanTpUnlock param4.ret = %d", __FUNCTION__,RET_OK);
    ret = RET_OK;
    return ret;  
}


/**
 * @name CanTpSetLockIgnore
 * @brief 设置忽略锁状态，锁定状态由上层APP控制
 * @param hdl CanTp处理句柄，用于标识和操作特定的CanTp实例。
 * @return int32_t 返回执行结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t CanTpSetLockIgnore(void)
{
    // CanTpPriv_t *priv = (CanTpPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }

    priv->ignore = CANTP_IGNORE_LOCK_TRUE;
    ret = RET_OK;
    // 返回执行结果
    return ret;
}

Std_ReturnType CanTp_Init(CanTp_ReceiveIndicationCbk_t rxIndicationCbk, CanTp_TransmitErrorCbk_t transmitErrCbk)
{
    // CanTpPriv_t *priv = NULL;
    int32_t ret = RET_ERR;
    LOG_INIT("carota_cantp");
    LOG_Debug("[%s]carota_cantp_start\r\n",__FUNCTION__);
    // 分配私有数据结构
    priv = (CanTpPriv_t *)calloc(1, sizeof(CanTpPriv_t));
    if (!priv)
    {
        LOG_Error("%s calloc priv fail.", __FUNCTION__);
        return cantp_init_malloc_erro;        
    }
    LOG_Debug("[%s]cantp_malloc_ok\r\n",__FUNCTION__);
    // 初始化共享内存
    priv->canTpShm = MAP_FAILED;
    if (RET_ERR == CanTpInitLibShm(priv))
    {
        LOG_Error("%s CanTpInitLibShm fail.", __FUNCTION__);
        free(priv);
        return cantp_init_ipc_erro;
    }
    LOG_Debug("[%s]cantp_Shm_ok\r\n",__FUNCTION__);
    // 初始化发送互斥锁
    if (0 != pthread_mutex_init(&priv->sendMutex, NULL))
    {
        LOG_Error("%s pthread_mutex_init sendMutex fail.", __FUNCTION__);
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
        free(priv);
        return cantp_init_send_lock_erro;        
    }
    LOG_Debug("[%s]cantp_mutex_ok\r\n",__FUNCTION__);
    // 初始化响应互斥锁
    if (0 != pthread_mutex_init(&priv->respMutex, NULL))
    {
        LOG_Error("%s pthread_mutex_init respMutex fail.", __FUNCTION__);
        pthread_mutex_destroy(&priv->sendMutex);
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
        free(priv);
        return cantp_init_resp_lock_erro;        
    }
    LOG_Debug("[%s]cantp_mutex_ok\r\n",__FUNCTION__);
    // 初始化响应条件变量（绑定单调时钟）
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    if (0 != pthread_cond_init(&priv->respCond, &cattr))
    {
        LOG_Error("%s pthread_cond_init respCond fail.", __FUNCTION__);
        pthread_mutex_destroy(&priv->respMutex);
        pthread_mutex_destroy(&priv->sendMutex);
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
        free(priv);
        pthread_condattr_destroy(&cattr); // 补充销毁未使用的属性
        return cantp_init_resp_init_erro;          
    }
    pthread_condattr_destroy(&cattr); // 初始化成功后销毁属性
    LOG_Debug("[%s]cantp_cond_ok\r\n",__FUNCTION__);

    // 创建 CanTpCheckActived 线程
    if (0 != pthread_create(&priv->tid, NULL, CanTpCheckActived, (void *)priv))
    {
        LOG_Error("%s pthread_create CanTpCheckActived fail.", __FUNCTION__); // 原日志错误（写的是respCond）
        pthread_cond_destroy(&priv->respCond);
        pthread_mutex_destroy(&priv->respMutex);
        pthread_mutex_destroy(&priv->sendMutex);
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
        free(priv);
        return cantp_init_CantpCheck_init_erro;           
    }
    LOG_Debug("[%s]cantp_CanTpCheckActived_ok\r\n",__FUNCTION__);
    // 初始化私有数据
    priv->rxIndicationCbk = rxIndicationCbk;
    priv->transmitErrCbk = transmitErrCbk;
    priv->countIdx = 0;
    priv->ignore = CANTP_IGNORE_LOCK_FALSE;


    // 检查并建立TCP连接（新增返回值检查）
    if (RET_ERR == TcpCheckAndConnect())
    {
        LOG_Error("%s TcpCheckAndConnect fail.", __FUNCTION__);
        // 清理已创建的资源
        pthread_cancel(priv->tid);
        pthread_join(priv->tid, NULL);
        pthread_cond_destroy(&priv->respCond);
        pthread_mutex_destroy(&priv->respMutex);
        pthread_mutex_destroy(&priv->sendMutex);
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
        free(priv);
        return cantp_init_tcp_connect_erro;
    }
    LOG_Debug("[%s]cantp_TcpCheckAndConnect_ok\r\n",__FUNCTION__);
    // 初始化数据队列
    if (TcpDataQueueInit(&g_tcp_data_queue) != RET_OK)
    {
        LOG_Error("%s TcpDataQueueInit fail.", __FUNCTION__);
        // 补充完整资源清理
        pthread_cancel(priv->tid);
        pthread_join(priv->tid, NULL);
        pthread_cond_destroy(&priv->respCond);
        pthread_mutex_destroy(&priv->respMutex);
        pthread_mutex_destroy(&priv->sendMutex);
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
        free(priv);
        return cantp_init_DataQueue_erro;
    }
    LOG_Debug("[%s]cantp_TcpDataQueue_ok\r\n",__FUNCTION__);

    // 启动接收线程（确保只创建一次）
    g_recv_thread_exit = 0;
    if (g_recv_tid == 0)
    {
        if (pthread_create(&g_recv_tid, NULL, TcpRecvThread, priv) != 0)
        {
            LOG_Error("Failed to create TcpRecvThread");
            // 清理队列和已创建资源
            TcpDataQueueDestroy(&g_tcp_data_queue);
            pthread_cancel(priv->tid);
            pthread_join(priv->tid, NULL);
            pthread_cond_destroy(&priv->respCond);
            pthread_mutex_destroy(&priv->respMutex);
            pthread_mutex_destroy(&priv->sendMutex);
            munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
            priv->canTpShm = NULL;
            free(priv);
            return cantp_init_TcpRecvThread_erro;
        }
    }
    else
    {
        LOG_Debug("TcpRecvThread already running");
    }
    LOG_Debug("[%s]TcpRecvThread\r\n",__FUNCTION__);
    // 启动处理线程
    if (pthread_create(&g_process_tid, NULL, TcpProcessThread, priv) != 0)
    {
        LOG_Error("Create TcpProcessThread failed");
        // 终止接收线程并清理
        g_recv_thread_exit = 1;
        pthread_cancel(g_recv_tid);
        pthread_join(g_recv_tid, NULL);
        g_recv_tid = 0; // 重置线程ID
        TcpDataQueueDestroy(&g_tcp_data_queue);
        // 清理priv相关资源
        pthread_cancel(priv->tid);
        pthread_join(priv->tid, NULL);
        pthread_cond_destroy(&priv->respCond);
        pthread_mutex_destroy(&priv->respMutex);
        pthread_mutex_destroy(&priv->sendMutex);
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
        free(priv);
        return cantp_init_TcpProcessThread_erro;
    }
    LOG_Debug("[%s]TcpProcessThread\r\n",__FUNCTION__);
    // 保存句柄并返回成功
    // *hdl = priv;
    ret = RET_OK;
    // 锁定CAN TP通道
    ret = CanTpLock();
    if (RET_ERR == ret) {
        LOG_Debug("[%s]CanTpLock failed.",__FUNCTION__);
        CanTpDeinit();
        return ret;        
    }
    
    g_canTpInitialized = 1;
    return ret;
}

int32_t CanTpDeinit(void) {
    if (!priv) {
        LOG_Error("[%s]CanTpDeinit: invalid handle",__FUNCTION__);
        return RET_ERR;
    }
    LOG_Debug("[%s]CanTpDeinit start\r\n",__FUNCTION__);


    // 1. 设置全局退出标志（通知线程主动退出）
    g_recv_thread_exit = 1; // 通知接收线程和处理线程退出

    if (priv->tid != 0) {
        // printf("Step 2.1: Cancel CanTpCheckActived thread (tid: %lu) \r\n", (unsigned long)priv->tid);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
        pthread_cancel(priv->tid);
        
        // 限时等待线程退出（500ms）
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 5 * 10000000;
        if (ts.tv_nsec >= 10*10000000) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 10*10000000;
        }
        int ret = pthread_timedjoin_np(priv->tid, NULL, &ts);
        if (ret != 0) {
            LOG_Debug("[%s]Step 2.1: CanTpCheckActived thread join timeout \r\n",__FUNCTION__);
        }
        priv->tid = 0;
    }
    LOG_Debug("[%s]CanTpDeinit start2\r\n",__FUNCTION__);
    // 2.2 终止接收线程（g_recv_tid）
    if (g_recv_tid != 0) {
        LOG_Debug("[%s]Step 2.2: Cancel TcpRecvThread (tid: %lu) \r\n", (unsigned long)g_recv_tid,__FUNCTION__);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
        pthread_cancel(g_recv_tid);
        
        // 限时等待
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 5 * 10000000;
        if (ts.tv_nsec >= 10*10000000) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 10*10000000;
        }
        int ret = pthread_timedjoin_np(g_recv_tid, NULL, &ts);
        if (ret != 0) {
            LOG_Debug("[%s]Step 2.2: TcpRecvThread join timeout \r\n",__FUNCTION__);
        }
        g_recv_tid = 0;
    }
    LOG_Debug("[%s]CanTpDeinit g_recv_tid over\r\n",__FUNCTION__);
    // 2.3 终止处理线程（g_process_tid）
    if (g_process_tid != 0) {
        // printf("Step 2.3: Cancel TcpProcessThread (tid: %lu) \r\n", (unsigned long)g_process_tid);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
        pthread_cancel(g_process_tid);
        
        // 限时等待
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 5 * 10000000;
        if (ts.tv_nsec >= 10*10000000) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 10*10000000;
        }
        int ret = pthread_timedjoin_np(g_process_tid, NULL, &ts);
        if (ret != 0) {
            LOG_Debug("[%s]Step 2.3: TcpProcessThread join timeout \r\n",__FUNCTION__);
        }
        g_process_tid = 0;
    }
    LOG_Debug("[%s]CanTpDeinit g_process_tid over\r\n",__FUNCTION__);

    // 3. 清理 TCP 连接资源
    int lock_ret = pthread_mutex_trylock(&g_tcp_mutex); // 非阻塞获取锁
    if (lock_ret == 0) {
        if (g_tcp_socket != -1) {
            int ctcp=close(g_tcp_socket);
            g_tcp_socket = -1;
            LOG_Debug("[%s]Step 3: TCP socket closed %d\r\n",__FUNCTION__,ctcp);
        }
        pthread_mutex_unlock(&g_tcp_mutex);
    } else {
        LOG_Debug("[%s]Step 3: Skip socket close (lock busy) \r\n",__FUNCTION__);
    }
    LOG_Debug("[%s]CanTpDeinit g_tcp_mutex over\r\n",__FUNCTION__);
    // 4. 销毁数据队列（唤醒等待线程并清理节点）
    // printf("Step 4: Destroy TcpDataQueue \r\n");
    lock_ret = pthread_mutex_trylock(&g_tcp_data_queue.mutex);
    if (lock_ret == 0) {
        g_tcp_data_queue.exit_flag = 1; // 设置退出标志
        pthread_cond_broadcast(&g_tcp_data_queue.cond); // 唤醒所有等待的处理线程
        pthread_mutex_unlock(&g_tcp_data_queue.mutex);

        // 等待线程退出后再清理节点（短暂延迟）
        usleep(100000); // 100ms

        // 再次加锁清理剩余节点
        pthread_mutex_lock(&g_tcp_data_queue.mutex);
        TcpDataNode_t *node = g_tcp_data_queue.head;
        while (node) {
            TcpDataNode_t *tmp = node;
            node = node->next;
            free(tmp->data.data);
            free(tmp);
        }
        g_tcp_data_queue.head = g_tcp_data_queue.tail = NULL;
        pthread_mutex_unlock(&g_tcp_data_queue.mutex);

        // 销毁队列同步对象
        pthread_mutex_destroy(&g_tcp_data_queue.mutex);
        pthread_cond_destroy(&g_tcp_data_queue.cond);
        // printf("Step 4: TcpDataQueue destroyed \r\n");
    } else {
        LOG_Debug("[%s]Step 4: Skip queue destroy (lock busy) \r\n",__FUNCTION__);
    }
    LOG_Debug("[%s]CanTpDeinit g_tcp_data_queue over\r\n",__FUNCTION__);
    // 5. 释放共享内存（mmap 映射的资源）
    if (priv->canTpShm != MAP_FAILED && priv->canTpShm != NULL) {
        // printf("Step 5: Unmap shared memory \r\n");
        munmap(priv->canTpShm, sizeof(CanTpLibShm_t));
        priv->canTpShm = NULL;
    }

    LOG_Debug("[%s]CanTpDeinit canTpShm over\r\n",__FUNCTION__);

    // 6. 销毁 priv 中的同步对象并释放内存
    // printf("Step 6: Destroy priv resources \r\n");
    // 尝试解锁并销毁互斥锁
    if (pthread_mutex_trylock(&priv->sendMutex) == 0) {
        pthread_mutex_unlock(&priv->sendMutex);
    }
    pthread_mutex_destroy(&priv->sendMutex);
    LOG_Debug("[%s]CanTpDeinit destroy(&priv->sendMutex) over\r\n",__FUNCTION__);
    if (pthread_mutex_trylock(&priv->respMutex) == 0) {
        pthread_mutex_unlock(&priv->respMutex);
    }
    pthread_mutex_destroy(&priv->respMutex);
    LOG_Debug("[%s]CanTpDeinit destroy(&priv->respMutex)over\r\n",__FUNCTION__);
    pthread_cond_destroy(&priv->respCond);
    LOG_Debug("[%s]CanTpDeinit destroy(&priv->respCond) over\r\n",__FUNCTION__);
    // 释放 priv 结构体
    free(priv);
    // hdl = NULL;
    
    g_canTpInitialized = 0;
    return RET_OK;
}

Std_ReturnType CanTp_Close(void)
{
    int32_t ret = RET_ERR;
    usleep(200*1000);
    ret = CanTpUnlock();
    LOG_Debug("CanTpUnlock1():%d\r\n",ret);
    if (ret == -1)
    {
        LOG_Error("[%s]CanTpUnlock Fail \r\n",__FUNCTION__);
        // 即使解锁失败，也要继续执行反初始化
    }
    
    ret = CanTpDeinit();
    LOG_Debug("CanTpDeinit():%d\r\n",ret);
    if (ret == -1)
    {
        LOG_Error("[%s]CanTpDeinit Fail \r\n",__FUNCTION__);
        // 即使反初始化失败，也要重置状态标志
    }
    LOG_Debug("[%s]CanTpDeinit success \r\n",__FUNCTION__);
    
    g_canTpInitialized = 0;
    return RET_OK;
}





















    