/****************************************************************************
 * Copyright (c) 2025 <SRTL>. All rights reserved.
 *
 * This file is part of <TSP>.
 *
 * Licensed under the  Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: RunJie Xu
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "srTspFWOs.h"

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

/* 消息队列实现 */
SROSStatus SRThreadQueueCreate(SRQueue_t **msgQRef, uint32_t maxSize, uint32_t maxNumber)
{
    if (!msgQRef || maxNumber == 0)
        return SR_FAILURE;

    SRQueue_t *queue = malloc(sizeof(SRQueue_t));
    if (!queue)
        return SR_FAILURE;

    queue->messages = malloc(maxNumber * sizeof(void *));
    if (!queue->messages)
    {
        free(queue);
        return SR_FAILURE;
    }

    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    queue->capacity = maxNumber;
    queue->max_size = maxSize;
    queue->count = 0;
    queue->head = 0;
    queue->tail = 0;
    queue->is_cleared = FALSE;

    *msgQRef = queue;
    return SR_SUCCESS;
}

SROSStatus SRThreadQueueWait(SRQueue_t *msgQRef, uint8_t *recvMsg, uint32_t size, uint32_t timeout)
{
    if (!msgQRef || !recvMsg || size > msgQRef->max_size)
        return SR_FAILURE;

    pthread_mutex_lock(&msgQRef->mutex);

    while (msgQRef->count == 0)
    {
        if (timeout == 0)
        {
            pthread_mutex_unlock(&msgQRef->mutex);
            return SR_FAILURE;
        }

        if (timeout == 0xFFFFFFFF)
        {
            pthread_cond_wait(&msgQRef->cond, &msgQRef->mutex);
        }
        else
        {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += timeout / 1000;
            ts.tv_nsec += (timeout % 1000) * 1000000;
            if (ts.tv_nsec >= 1000000000)
            {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }

            if (pthread_cond_timedwait(&msgQRef->cond, &msgQRef->mutex, &ts) == ETIMEDOUT)
            {
                pthread_mutex_unlock(&msgQRef->mutex);
                return SR_TIMEOUT;
            }
        }
    }

    void *msg = msgQRef->messages[msgQRef->head];
    memcpy(recvMsg, msg, size);
    free(msg);

    msgQRef->head = (msgQRef->head + 1) % msgQRef->capacity;
    msgQRef->count--;

    pthread_mutex_unlock(&msgQRef->mutex);
    return SR_SUCCESS;
}

SROSStatus SRThreadQueueRelease(SRQueue_t *msgQRef, uint32_t size, uint8_t *msgPtr, uint32_t timeout)
{
    if (!msgQRef || !msgPtr || size > msgQRef->max_size)
        return SR_FAILURE;

    void *msg = malloc(size);
    if (!msg)
        return SR_FAILURE;
    memcpy(msg, msgPtr, size);

    pthread_mutex_lock(&msgQRef->mutex);

    while (msgQRef->count == msgQRef->capacity)
    {
        if (timeout == 0)
        {
            free(msg);
            pthread_mutex_unlock(&msgQRef->mutex);
            return SR_FAILURE;
        }

        if (timeout == 0xFFFFFFFF)
        {
            /* 无限等待直到有空位 */
            pthread_cond_wait(&msgQRef->cond, &msgQRef->mutex);
        }
        else
        {
            /* 有超时时间，等待超时则返回 */
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += timeout / 1000;
            ts.tv_nsec += (timeout % 1000) * 1000000;
            if (ts.tv_nsec >= 1000000000)
            {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }

            int ret = pthread_cond_timedwait(&msgQRef->cond, &msgQRef->mutex, &ts);
            if (ret == ETIMEDOUT)
            {
                free(msg);
                pthread_mutex_unlock(&msgQRef->mutex);
                return SR_FAILURE;
            }
        }
    }

    /* 插入消息 */
    msgQRef->messages[msgQRef->tail] = msg;
    msgQRef->tail = (msgQRef->tail + 1) % msgQRef->capacity;
    msgQRef->count++;

    /* 通知消费者 */
    pthread_cond_signal(&msgQRef->cond);

    pthread_mutex_unlock(&msgQRef->mutex);
    return SR_SUCCESS;
}

SROSStatus SRThreadQueueGetCnt(SRQueue_t *msgQRef, uint32_t *cnt_ptr)
{
    if (!msgQRef || !cnt_ptr)
        return SR_FAILURE;
    pthread_mutex_lock(&msgQRef->mutex);
    *cnt_ptr = msgQRef->count;
    pthread_mutex_unlock(&msgQRef->mutex);
    return SR_SUCCESS;
}

SROSStatus SRThreadQueueDelete(SRQueue_t *msgQRef)
{
    if (!msgQRef)
        return SR_FAILURE;

    pthread_mutex_lock(&msgQRef->mutex);
    for (int i = 0; i < msgQRef->count; i++)
    {
        int idx = (msgQRef->head + i) % msgQRef->capacity;
        free(msgQRef->messages[idx]);
    }
    pthread_mutex_unlock(&msgQRef->mutex);

    pthread_mutex_destroy(&msgQRef->mutex);
    pthread_cond_destroy(&msgQRef->cond);
    free(msgQRef->messages);
    free(msgQRef);
    return SR_SUCCESS;
}

/* 互斥锁实现 */
SROSStatus SRThreadMutexCreate(SRMutex_t *mutexRef)
{
    if (!mutexRef)
        return SR_FAILURE;
    pthread_mutex_init(&mutexRef->mutex, NULL);
    mutexRef->is_cleared = FALSE;
    return SR_SUCCESS;
}

SROSStatus SRThreadMutexLock(SRMutex_t *mutexRef)
{
    if (!mutexRef)
        return SR_FAILURE;
    pthread_mutex_lock(&mutexRef->mutex);
    return SR_SUCCESS;
}

SROSStatus SRThreadMutexUnlock(SRMutex_t *mutexRef)
{
    if (!mutexRef)
        return SR_FAILURE;
    pthread_mutex_unlock(&mutexRef->mutex);
    return SR_SUCCESS;
}

SROSStatus SRThreadMutexDelete(SRMutex_t *mutexRef)
{
    if (!mutexRef || mutexRef->is_cleared)
        return SR_FAILURE;
    pthread_mutex_destroy(&mutexRef->mutex);
    mutexRef->is_cleared = TRUE;
    return SR_SUCCESS;
}

/* 线程休眠 */
void SRThreadSleepMs(uint32_t ms)
{
    usleep(ms * 1000);
}
