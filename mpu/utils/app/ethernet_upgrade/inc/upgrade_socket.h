#ifndef _ASYNC_SOCKET_H_
#define _ASYNC_SOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

// 数据缓冲区结构
typedef struct DataBuffer {
    char data[1024];
    size_t length;
    struct DataBuffer* next;
} DataBuffer;

// 异步socket上下文结构体
typedef struct {
    int sockfd;
    int connected;
    struct sockaddr_in server_addr;
    
    // 线程相关
    pthread_t receiver_thread;      // 数据接收线程
    pthread_t processor_thread;     // 数据处理线程
    int receiver_running;
    int processor_running;
    
    // 数据队列
    DataBuffer* data_queue_head;
    DataBuffer* data_queue_tail;
    pthread_mutex_t queue_mutex;
    pthread_cond_t data_available;
    
    // 回调函数指针
    void (*command_handler)(const char* command, size_t length);
    
} async_socket_context_t;

// 全局socket上下文
extern async_socket_context_t g_async_socket_ctx;

// 函数声明
int async_socket_init(async_socket_context_t* ctx);
int async_socket_connect(async_socket_context_t* ctx, const char* server_ip, int port);
int async_socket_send(async_socket_context_t* ctx, const void* data, size_t length);
int async_socket_send_string(async_socket_context_t* ctx, const char* data);
void async_socket_close(async_socket_context_t* ctx);

// 线程控制函数
int async_socket_start_listening(async_socket_context_t* ctx);
int async_socket_stop_listening(async_socket_context_t* ctx);

// 命令处理设置
void async_socket_set_command_handler(async_socket_context_t* ctx, 
                                    void (*handler)(const char* command, size_t length));

// 辅助函数
int async_socket_is_connected(async_socket_context_t* ctx);
int async_socket_get_fd(async_socket_context_t* ctx);

#ifdef __cplusplus
}
#endif

#endif // _ASYNC_SOCKET_H_