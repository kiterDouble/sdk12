#include "upgrade_socket.h"
#include "sr_log.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

// 全局socket上下文实例
async_socket_context_t g_async_socket_ctx = {0};

// 数据缓冲区管理函数
static DataBuffer* create_data_buffer(const char* data, size_t length) {
    DataBuffer* buffer = (DataBuffer*)malloc(sizeof(DataBuffer));
    if (buffer) {
        memcpy(buffer->data, data, length);
        buffer->length = length;
        buffer->next = NULL;
    }
    return buffer;
}

static void destroy_data_buffer(DataBuffer* buffer) {
    if (buffer) {
        free(buffer);
    }
}

// 数据队列操作
static void enqueue_data(async_socket_context_t* ctx, const char* data, size_t length) {
    pthread_mutex_lock(&ctx->queue_mutex);
    
    DataBuffer* new_buffer = create_data_buffer(data, length);
    if (new_buffer) {
        if (ctx->data_queue_tail) {
            ctx->data_queue_tail->next = new_buffer;
            ctx->data_queue_tail = new_buffer;
        } else {
            ctx->data_queue_head = new_buffer;
            ctx->data_queue_tail = new_buffer;
        }
        pthread_cond_signal(&ctx->data_available);
    }
    
    pthread_mutex_unlock(&ctx->queue_mutex);
}

static DataBuffer* dequeue_data(async_socket_context_t* ctx) {
    pthread_mutex_lock(&ctx->queue_mutex);
    
    while (ctx->processor_running && ctx->data_queue_head == NULL) {
        pthread_cond_wait(&ctx->data_available, &ctx->queue_mutex);
    }
    
    DataBuffer* buffer = ctx->data_queue_head;
    if (buffer) {
        ctx->data_queue_head = buffer->next;
        if (ctx->data_queue_head == NULL) {
            ctx->data_queue_tail = NULL;
        }
    }
    
    pthread_mutex_unlock(&ctx->queue_mutex);
    return buffer;
}

// 接收线程函数（生产者）
static void* receiver_thread_func(void* arg) {
    async_socket_context_t* ctx = (async_socket_context_t*)arg;
    char recv_buffer[1024];
    int bytes_received;
    
    LOG_Debug("Receiver thread started");
    
    while (ctx->receiver_running && ctx->connected) {
        bytes_received = recv(ctx->sockfd, recv_buffer, sizeof(recv_buffer) - 1, 0);
        
        if (bytes_received > 0) {
            recv_buffer[bytes_received] = '\0';
            LOG_Debug("Received %d bytes: %s", bytes_received, recv_buffer);
            
            // 将接收到的数据放入队列
            enqueue_data(ctx, recv_buffer, bytes_received);
            
        } else if (bytes_received == 0) {
            // 连接关闭
            LOG_Debug("Server closed connection");
            ctx->connected = 0;
            break;
        } else {
            // 接收错误
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                LOG_Error("Receive error: %s", strerror(errno));
                ctx->connected = 0;
                break;
            }
        }
    }
    
    LOG_Debug("Receiver thread stopping");
    return NULL;
}

// 处理线程函数（消费者）
static void* processor_thread_func(void* arg) {
    async_socket_context_t* ctx = (async_socket_context_t*)arg;
    DataBuffer* buffer;
    
    LOG_Debug("Processor thread started");
    
    while (ctx->processor_running) {
        buffer = dequeue_data(ctx);
        if (buffer && ctx->command_handler) {
            // 调用命令处理函数
            ctx->command_handler(buffer->data, buffer->length);
            destroy_data_buffer(buffer);
        } else if (buffer) {
            // 默认处理：记录日志
            LOG_Debug("Received data (no handler): %.*s", (int)buffer->length, buffer->data);
            destroy_data_buffer(buffer);
        }
    }
    
    // 清理剩余数据
    pthread_mutex_lock(&ctx->queue_mutex);
    while (ctx->data_queue_head) {
        buffer = ctx->data_queue_head;
        ctx->data_queue_head = buffer->next;
        destroy_data_buffer(buffer);
    }
    ctx->data_queue_tail = NULL;
    pthread_mutex_unlock(&ctx->queue_mutex);
    
    LOG_Debug("Processor thread stopping");
    return NULL;
}

// 主要函数实现
int async_socket_init(async_socket_context_t* ctx) {
    if (ctx == NULL) {
        return -1;
    }
    
    memset(ctx, 0, sizeof(async_socket_context_t));
    ctx->sockfd = -1;
    ctx->connected = 0;
    ctx->receiver_running = 0;
    ctx->processor_running = 0;
    
    // 初始化同步原语
    pthread_mutex_init(&ctx->queue_mutex, NULL);
    pthread_cond_init(&ctx->data_available, NULL);
    
    LOG_Debug("Async socket context initialized");
    return 0;
}

int async_socket_connect(async_socket_context_t* ctx, const char* server_ip, int port) {
    if (ctx == NULL || server_ip == NULL) {
        LOG_Error("Invalid parameters");
        return -1;
    }
    
    // 创建socket
    ctx->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->sockfd < 0) {
        LOG_Error("Failed to create socket: %s", strerror(errno));
        return -1;
    }
    
    // 设置服务器地址
    memset(&ctx->server_addr, 0, sizeof(ctx->server_addr));
    ctx->server_addr.sin_family = AF_INET;
    ctx->server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &ctx->server_addr.sin_addr) <= 0) {
        LOG_Error("Invalid address or address not supported: %s", server_ip);
        close(ctx->sockfd);
        ctx->sockfd = -1;
        return -1;
    }
    
    // 连接服务器
    if (connect(ctx->sockfd, (struct sockaddr*)&ctx->server_addr, sizeof(ctx->server_addr)) < 0) {
        LOG_Error("Connection failed: %s", strerror(errno));
        close(ctx->sockfd);
        ctx->sockfd = -1;
        return -1;
    }
    
    ctx->connected = 1;
    LOG_Debug("Connected to server %s:%d", server_ip, port);
    
    // 自动启动监听线程
    async_socket_start_listening(ctx);
    
    return ctx->sockfd;
}

int async_socket_send(async_socket_context_t* ctx, const void* data, size_t length) {
    if (ctx == NULL || data == NULL || length == 0) {
        LOG_Error("Invalid parameters");
        return -1;
    }
    
    if (!ctx->connected || ctx->sockfd < 0) {
        LOG_Error("Not connected to server");
        return -1;
    }
    
    int ret = send(ctx->sockfd, data, length, 0);
    if (ret < 0) {
        LOG_Error("Send failed: %s", strerror(errno));
        return -1;
    }
    
    LOG_Debug("Sent %d bytes", ret);
    return ret;
}

int async_socket_send_string(async_socket_context_t* ctx, const char* data) {
    if (data == NULL) {
        return -1;
    }
    return async_socket_send(ctx, data, strlen(data));
}

int async_socket_start_listening(async_socket_context_t* ctx) {
    if (!ctx || !ctx->connected) {
        return -1;
    }
    
    ctx->receiver_running = 1;
    ctx->processor_running = 1;
    
    // 创建接收线程
    int ret = pthread_create(&ctx->receiver_thread, NULL, receiver_thread_func, ctx);
    if (ret != 0) {
        LOG_Error("Failed to create receiver thread: %d", ret);
        ctx->receiver_running = 0;
        ctx->processor_running = 0;
        return -1;
    }
    
    // 创建处理线程
    ret = pthread_create(&ctx->processor_thread, NULL, processor_thread_func, ctx);
    if (ret != 0) {
        LOG_Error("Failed to create processor thread: %d", ret);
        ctx->receiver_running = 0;
        ctx->processor_running = 0;
        pthread_join(ctx->receiver_thread, NULL);
        return -1;
    }
    
    LOG_Debug("Listening threads started");
    return 0;
}

int async_socket_stop_listening(async_socket_context_t* ctx) {
    if (!ctx) {
        return -1;
    }
    
    // 停止接收线程
    ctx->receiver_running = 0;
    if (ctx->receiver_thread) {
        pthread_join(ctx->receiver_thread, NULL);
        ctx->receiver_thread = 0;
    }
    
    // 停止处理线程
    ctx->processor_running = 0;
    pthread_cond_broadcast(&ctx->data_available); // 唤醒等待的处理线程
    if (ctx->processor_thread) {
        pthread_join(ctx->processor_thread, NULL);
        ctx->processor_thread = 0;
    }
    
    LOG_Debug("Listening threads stopped");
    return 0;
}

void async_socket_set_command_handler(async_socket_context_t* ctx, 
                                    void (*handler)(const char* command, size_t length)) {
    if (ctx) {
        ctx->command_handler = handler;
    }
}

void async_socket_close(async_socket_context_t* ctx) {
    if (ctx == NULL) {
        return;
    }
    
    // 停止监听线程
    async_socket_stop_listening(ctx);
    
    // 关闭socket
    if (ctx->sockfd >= 0) {
        close(ctx->sockfd);
        ctx->sockfd = -1;
        ctx->connected = 0;
    }
    
    // 销毁同步原语
    pthread_mutex_destroy(&ctx->queue_mutex);
    pthread_cond_destroy(&ctx->data_available);
    
    LOG_Debug("Socket closed");
}

int async_socket_is_connected(async_socket_context_t* ctx) {
    if (ctx == NULL) {
        return 0;
    }
    return ctx->connected;
}

int async_socket_get_fd(async_socket_context_t* ctx) {
    if (ctx == NULL) {
        return -1;
    }
    return ctx->sockfd;
}