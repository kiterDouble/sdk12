#ifndef MQTT_QUEUE_H
#define MQTT_QUEUE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// 数据缓冲区结构
typedef struct {
    uint8_t *data;       // 数据缓冲区
    size_t len;          // 数据长度
} DataBuffer_t;

// 队列节点结构
typedef struct DataNode {
    DataBuffer_t data;
    struct DataNode *next;
} DataNode_t;

// 队列控制结构
typedef struct {
    DataNode_t *head;            // 队列头
    DataNode_t *tail;            // 队列尾
    pthread_mutex_t mutex;       // 队列互斥锁
    pthread_cond_t cond;         // 队列条件变量（通知有新数据）
    volatile uint8_t exit_flag;  // 退出标志
} DataQueue_t;

/**
 * @brief 初始化数据队列
 * @param queue 队列指针
 * @return 0成功，-1失败
 */
int DataQueueInit(DataQueue_t *queue);

/**
 * @brief 数据入队
 * @param queue 队列指针
 * @param data 数据指针
 * @param len 数据长度
 * @return 0成功，-1失败
 */
int DataQueueEnqueue(DataQueue_t *queue, const uint8_t *data, size_t len);

/**
 * @brief 数据出队（阻塞等待，直到有数据或退出）
 * @param queue 队列指针
 * @param data 输出数据缓冲区
 * @return 0成功，-1失败（退出或参数错误）
 */
int DataQueueDequeue(DataQueue_t *queue, DataBuffer_t *data);

/**
 * @brief 非阻塞数据出队
 * @param queue 队列指针
 * @param data 输出数据缓冲区
 * @return 0成功，-1失败（队列为空或参数错误）
 */
int DataQueueDequeueNonBlock(DataQueue_t *queue, DataBuffer_t *data);

/**
 * @brief 销毁队列（清理剩余节点和同步对象）
 * @param queue 队列指针
 */
void DataQueueDestroy(DataQueue_t *queue);

/**
 * @brief 设置队列退出标志并唤醒所有等待线程
 * @param queue 队列指针
 */
void DataQueueSignalExit(DataQueue_t *queue);

#ifdef __cplusplus
}
#endif

#endif /* MQTT_QUEUE_H */
