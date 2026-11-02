
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#include "msg_queue.h"
#include "sr_log.h"


/**
 * @brief 初始化消息队列（传入上下文指针，支持多队列）
 *
 * 初始化互斥锁和条件变量（使用 CLOCK_MONOTONIC 避免系统时间跳变影响）。
 *
 * @param[in,out] queue   队列上下文（需用户提前分配，栈/堆均可）
 * @param[in]     max_len 队列最大长度（如 100）
 * @return int 成功返回 0，失败返回 -1
 */
int MsgQueueInit(MsgQueue_t *queue, int max_len)
{
    pthread_condattr_t cond_attr;

    // 前置检查
    if (queue == NULL) {
        LOG_Error("Msg queue queue is NULL!");
        return -1;
    }
    if (max_len <= 0) {
        LOG_Error("Invalid max_len=%d", max_len);
        return -1;
    }
    if (queue->inited) {
        LOG_Warn("Msg queue queue has been inited already");
        return 0;
    }

    // 初始化上下文参数
    memset(queue, 0, sizeof(MsgQueue_t)); // 先清空
    queue->max_len = max_len;
    queue->cur_len = 0;
    queue->head = NULL;
    queue->tail = NULL;

    // 初始化互斥锁
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        LOG_Error("Init mutex failed! errno=%d", errno);
        return -1;
    }

    // 初始化条件变量属性（指定 CLOCK_MONOTONIC 时钟）
    if (pthread_condattr_init(&cond_attr) != 0) {
        LOG_Error("Init cond attr failed! errno=%d", errno);
        pthread_mutex_destroy(&queue->mutex); // 回滚互斥锁
        return -1;
    }

    // 设置条件变量使用 CLOCK_MONOTONIC 时钟
    if (pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC) != 0) {
        LOG_Error("Set cond clock to CLOCK_MONOTONIC failed! errno=%d", errno);
        pthread_condattr_destroy(&cond_attr); // 销毁属性
        pthread_mutex_destroy(&queue->mutex); // 回滚互斥锁
        return -1;
    }

    // 用带时钟属性的参数初始化条件变量
    if (pthread_cond_init(&queue->cond, &cond_attr) != 0) {
        LOG_Error("Init cond failed! errno=%d", errno);
        pthread_condattr_destroy(&cond_attr); // 销毁属性
        pthread_mutex_destroy(&queue->mutex); // 回滚互斥锁
        return -1;
    }

    // 销毁条件变量属性（初始化完成后不再需要）
    pthread_condattr_destroy(&cond_attr);

    queue->inited = 1;
    LOG_Info("Msg queue init success (queue=%p, max len: %d)", queue, max_len);
    return 0;
}

/**
 * @brief 发送消息到指定队列
 *
 * 构造消息节点，若 len > 0 则深拷贝数据。线程安全，支持多生产者。
 *
 * @param[in,out] queue 队列上下文（已初始化）
 * @param[in]     type  消息类型（通常用于标识授时源）
 * @param[in]     data  消息数据（len>0 时拷贝，len=0 时传递指针）
 * @param[in]     len   数据长度（字节）
 * @return int 成功返回 0，失败返回 -1（如队列满或内存分配失败）
 */
int MsgQueueSend(MsgQueue_t *queue, uint32_t type, void *data, int len)
{
    // 前置检查
    if (queue == NULL || !queue->inited) {
        LOG_Error("Msg queue queue is NULL or not inited!");
        return -1;
    }
    if (len < 0) {
        LOG_Error("Invalid len=%d", len);
        return -1;
    }
    if (len > 0 && data == NULL) {
        LOG_Error("data is NULL but len=%d>0", len);
        return -1;
    }

    // 构造消息节点
    MsgNode_t *node = (MsgNode_t *)malloc(sizeof(MsgNode_t));
    if (node == NULL) {
        LOG_Error("Malloc node failed!");
        return -1;
    }
    memset(node, 0, sizeof(MsgNode_t));

    // 填充消息内容
    node->msg.type = type;
    node->msg.len = len;
    node->msg.data = NULL;

    if (len > 0) {
        node->msg.data = malloc(len);
        if (node->msg.data == NULL) {
            LOG_Error("Malloc msg data failed! len=%d", len);
            free(node);
            return -1;
        }
        memcpy(node->msg.data, data, len);
    } else {
        node->msg.data = data;
    }

    // 加锁操作队列
    pthread_mutex_lock(&queue->mutex);

    // 检查队列是否已满
    if (queue->cur_len >= queue->max_len) {
        LOG_Error("Msg queue full (queue=%p, cur len: %d, max len: %d)", queue, queue->cur_len, queue->max_len);
        pthread_mutex_unlock(&queue->mutex);
        // 释放已分配的资源
        if (len > 0 && node->msg.data != NULL) {
            free(node->msg.data);
        }
        free(node);
        return -1;
    }

    // 将节点加入队列尾部
    if (queue->tail == NULL) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
    queue->cur_len++;

    // 唤醒阻塞的接收线程
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);

    return 0;
}

/**
 * @brief 从指定队列接收消息（毫秒级超时）
 *
 * 线程安全，支持单消费者阻塞等待。使用 CLOCK_MONOTONIC 计时，不受系统时间跳变影响。
 *
 * @param[in,out] queue   队列上下文（已初始化）
 * @param[out]    msg     输出参数：接收到的消息（调用者需使用 MsgFree 释放动态数据）
 * @param[in]     timeout 超时时间（ms）：0=不阻塞立即返回，-1=永久阻塞，>0=超时等待
 * @return int 成功返回 0，超时返回 -2，失败返回 -1
 */
int MsgQueueRecv(MsgQueue_t *queue, Msg_t *msg, uint32_t timeout)
{
    // 前置检查
    if (queue == NULL || !queue->inited) {
        LOG_Error("Msg queue queue is NULL or not inited!");
        return -1;
    }
    if (msg == NULL) {
        LOG_Error("msg pointer is NULL!");
        return -1;
    }

    // 加锁
    pthread_mutex_lock(&queue->mutex);

    // 循环等待消息
    while (queue->cur_len == 0) {
        if (timeout == 0) {
            pthread_mutex_unlock(&queue->mutex);
            LOG_Debug("Msg queue empty (queue=%p, non-block)", queue);
            return -2;
        } else if (timeout == (uint32_t)-1) {
            pthread_cond_wait(&queue->cond, &queue->mutex);
        } else {
            // 毫秒级超时转换（单调时钟，避免系统时间影响）
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);

            ts.tv_sec += timeout / 1000;
            ts.tv_nsec += (timeout % 1000) * 1000000;
            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }

            // 带超时等待
            int ret = pthread_cond_timedwait(&queue->cond, &queue->mutex, &ts);
            if (ret == ETIMEDOUT) {
                pthread_mutex_unlock(&queue->mutex);
                return -2;
            } else if (ret != 0) {
                pthread_mutex_unlock(&queue->mutex);
                LOG_Error("Cond timedwait failed (queue=%p)! errno=%d", queue, ret);
                return -1;
            }
        }
    }

    // 取出队列头部节点
    MsgNode_t *node = queue->head;
    queue->head = node->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    queue->cur_len--;

    // 拷贝消息到输出参数
    memcpy(msg, &node->msg, sizeof(Msg_t));
    free(node); // 释放节点

    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

/**
 * @brief 销毁指定的消息队列
 *
 * 加锁清空队列中所有节点（释放动态分配的数据），然后销毁互斥锁和条件变量。
 *
 * @param[in,out] queue 队列上下文
 */
void MsgQueueDeinit(MsgQueue_t *queue)
{
    if (queue == NULL || !queue->inited) {
        return;
    }

    pthread_mutex_lock(&queue->mutex);

    // 清空队列所有节点
    MsgNode_t *node = queue->head;
    while (node != NULL) {
        MsgNode_t *tmp = node;
        node = node->next;
        if (tmp->msg.len > 0 && tmp->msg.data != NULL) {
            free(tmp->msg.data);
        }
        free(tmp);
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->cur_len = 0;

    pthread_mutex_unlock(&queue->mutex);

    // 销毁锁和条件变量
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
    queue->inited = 0;

    LOG_Info("Msg queue deinit success (queue=%p)", queue);
}

/**
 * @brief 释放消息中动态分配的数据
 *
 * 调用 MsgQueueRecv 后，若消息包含动态数据（len > 0），应调用此函数释放。
 *
 * @param[in,out] msg 要释放的消息
 */
void MsgFree(Msg_t *msg)
{
    if (msg == NULL) {
        LOG_Warn("Msg pointer is NULL, skip free");
        return;
    }

    if (msg->len > 0 && msg->data != NULL) {
        free(msg->data);
        msg->data = NULL;
        msg->len = 0;
    }
}
