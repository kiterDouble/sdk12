#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <stdint.h>

typedef struct {
    uint32_t type;        // 消息类型
    int len;              // 数据长度（0=静态数据，>0=动态分配数据）
    void *data;           // 消息数据
} Msg_t;

// 队列节点结构体（链表实现）
typedef struct _MsgNode {
    Msg_t msg;                  // 消息内容
    struct _MsgNode *next;      // 下一个节点
} MsgNode_t;

// 消息队列上下文
typedef struct {
    MsgNode_t *head;            // 队列头
    MsgNode_t *tail;            // 队列尾
    int max_len;                // 队列最大长度
    int cur_len;                // 当前队列长度
    pthread_mutex_t mutex;      // 互斥锁（保护队列访问）
    pthread_cond_t cond;        // 条件变量（实现阻塞等待）
    int inited;                 // 初始化标记
} MsgQueue_t;

/**
 * @brief 初始化消息队列（传入上下文指针，支持多队列）
 * @param queue 队列上下文（需用户提前分配，栈/堆均可）
 * @param max_len 队列最大长度（如100）
 * @return 成功返回0，失败返回-1
 */
int MsgQueueInit(MsgQueue_t *queue, int max_len);

/**
 * @brief 发送消息到指定队列
 * @param queue 队列上下文（已初始化）
 * @param type 消息类型
 * @param data 消息数据（len>0时拷贝，len=0时传递指针）
 * @param len 数据长度
 * @return 成功返回0，失败返回-1
 */
int MsgQueueSend(MsgQueue_t *queue, uint32_t type, void *data, int len);

/**
 * @brief 从指定队列接收消息（毫秒级超时）
 * @param queue 队列上下文（已初始化）
 * @param msg 输出参数：接收的消息
 * @param timeout 超时时间（ms，0=不阻塞，-1=永久阻塞）
 * @return 成功返回0，超时返回-2，失败返回-1
 */
int MsgQueueRecv(MsgQueue_t *queue, Msg_t *msg, uint32_t timeout);

/**
 * @brief 销毁指定的消息队列
 * @param queue 队列上下文
 */
void MsgQueueDeinit(MsgQueue_t *queue);


/**
 * @brief 释放消息的动态数据（保持不变）
 * @param msg 要释放的消息
 */
void MsgFree(Msg_t *msg);

#endif