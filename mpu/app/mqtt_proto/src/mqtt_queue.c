#include "mqtt_queue.h"
#include "sr_log.h"

// 初始化数据队列
int DataQueueInit(DataQueue_t *queue) {
    if (!queue) return -1;
    queue->head = NULL;
    queue->tail = NULL;
    queue->exit_flag = 0;
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        LOG_Error("MqttQueuePack: DataQueue mutex init failed");
        return -1;
    }
    if (pthread_cond_init(&queue->cond, NULL) != 0) {
        LOG_Error("MqttQueuePack: DataQueue cond init failed");
        pthread_mutex_destroy(&queue->mutex);
        return -1;
    }
    return 0;
}

// 数据入队
int DataQueueEnqueue(DataQueue_t *queue, const uint8_t *data, size_t len) {
    if (!queue || !data || len == 0) return -1;

    // 分配节点和数据缓冲区
    DataNode_t *node = (DataNode_t*)malloc(sizeof(DataNode_t));
    if (!node) return -1;
    node->data.data = (uint8_t*)malloc(len);
    if (!node->data.data) {
        free(node);
        return -1;
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
    return 0;
}

// 数据出队（阻塞等待，直到有数据或退出）
int DataQueueDequeue(DataQueue_t *queue, DataBuffer_t *data) {
    if (!queue || !data) return -1;

    pthread_mutex_lock(&queue->mutex);
    // 等待数据或退出信号
    while (!queue->head && !queue->exit_flag) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    if (queue->exit_flag) {
        pthread_mutex_unlock(&queue->mutex);
        return -1; // 退出标志触发
    }

    // 取出队头数据
    DataNode_t *node = queue->head;
    data->data = node->data.data;
    data->len = node->data.len;
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    free(node); // 释放节点（数据缓冲区由处理线程释放）
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

// 非阻塞数据出队
int DataQueueDequeueNonBlock(DataQueue_t *queue, DataBuffer_t *data) {
    if (!queue || !data) return -1;

    pthread_mutex_lock(&queue->mutex);
    if (!queue->head) {
        pthread_mutex_unlock(&queue->mutex);
        return -1; // 队列为空
    }

    // 取出队头数据
    DataNode_t *node = queue->head;
    data->data = node->data.data;
    data->len = node->data.len;
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    free(node);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

// 获取队列中最新的数据（队尾），如果队列中还有多余数据则删除，并打印消息
// 阻塞等待，直到有数据或退出
int DataQueueDequeueLastest(DataQueue_t *queue, DataBuffer_t *data) {
    if (!queue || !data) return -1;

    pthread_mutex_lock(&queue->mutex);
    // 等待数据或退出信号
    while (!queue->tail && !queue->exit_flag) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    if (queue->exit_flag) {
        pthread_mutex_unlock(&queue->mutex);
        return -1; // 退出标志触发
    }

    // 取出队尾数据
    DataNode_t *node = queue->tail;
    data->data = node->data.data;
    data->len = node->data.len;

    // 如果队列中还有多余数据（head != tail），则删除多余数据
    if (queue->head != queue->tail) {
        DataNode_t *tmp = queue->head;
        int dropped_count = 0;
        while (tmp != node) {
            DataNode_t *next = tmp->next;
            free(tmp->data.data);
            free(tmp);
            tmp = next;
            dropped_count++;
        }
        LOG_Warn("MqttQueuePack: DataQueueDequeueLastest dropped %d old data(s)", dropped_count);
    }

    queue->head = NULL;
    queue->tail = NULL;
    free(node); // 释放节点（数据缓冲区由调用者释放）
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

// 销毁队列
void DataQueueDestroy(DataQueue_t *queue) {
    if (!queue) return;

    // 1. 先设置退出标志并唤醒所有等待线程（防止死锁）
    pthread_mutex_lock(&queue->mutex);
    queue->exit_flag = 1;
    pthread_cond_broadcast(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);

    // 2. 短暂等待，让等待线程退出（可选，取决于使用场景）
    // 注意：这里无法确保线程已退出，调用者应先 pthread_join 再调用此函数

    // 3. 清理队列剩余节点
    pthread_mutex_lock(&queue->mutex);
    DataNode_t *node = queue->head;
    while (node) {
        DataNode_t *tmp = node;
        node = node->next;
        free(tmp->data.data);  // 释放数据缓冲区
        free(tmp);             // 释放节点
    }
    queue->head = queue->tail = NULL;
    pthread_mutex_unlock(&queue->mutex);

    // 4. 销毁同步对象
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);

    LOG_Debug("MqttQueuePack: DataQueueDestroy completed");
}

// 设置队列退出标志并唤醒所有等待线程
void DataQueueSignalExit(DataQueue_t *queue) {
    if (!queue) return;
    pthread_mutex_lock(&queue->mutex);
    queue->exit_flag = 1;
    pthread_cond_broadcast(&queue->cond); // 唤醒所有等待的处理线程
    pthread_mutex_unlock(&queue->mutex);
}
