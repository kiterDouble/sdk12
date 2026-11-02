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

#ifndef __SR_TSPFW_OS_H__
#define __SR_TSPFW_OS_H__
#include <stdint.h>
#include <pthread.h>

#define SR_SUCCESS 0
#define SR_FAILURE -1
#define SR_TIMEOUT -99
#define SR_QUEUE_CALLBACK_QUEUE_DEFAULT_MAX_CAPACITY (10)

#ifdef __cplusplus
extern "C"
{
#endif
	typedef int SROSStatus;

	typedef struct
	{
		pthread_mutex_t mutex;
		pthread_cond_t cond;
		void **messages;
		int capacity;
		int max_size;
		int count;
		int head;
		int tail;
		int is_cleared;
	} SRQueue_t;

	typedef struct
	{
		pthread_mutex_t mutex;
		int is_cleared;
	} SRMutex_t;

	typedef struct
	{
		void *q;
	} SRMsgQueueInternal;

	typedef enum
	{
		E_SR_WAIT_FOREVER = (int)0xFFFFFFFF,
		E_SR_NO_WAIT = 0
	} SRWait_e;

	/* 创建互斥锁 */
	SROSStatus SRThreadMutexCreate(SRMutex_t *mutexRef);
	/* 请求互斥锁 */
	SROSStatus SRThreadMutexLock(SRMutex_t *mutexRef);
	/* 释放互斥锁 */
	SROSStatus SRThreadMutexUnlock(SRMutex_t *mutexRef);
	/* 销毁互斥锁 */
	SROSStatus SRThreadMutexDelete(SRMutex_t *mutexRef);
	/* 创建消息队列 */
	SROSStatus SRThreadQueueCreate(SRQueue_t **msgQRef, uint32_t maxSize, uint32_t maxNumber);
	/* 消息队列接收消息 */
	SROSStatus SRThreadQueueWait(SRQueue_t *msgQRef, uint8_t *recvMsg, uint32_t size, uint32_t timeout);
	/* 发送消息到队列 */
	SROSStatus SRThreadQueueRelease(SRQueue_t *msgQRef, uint32_t size, uint8_t *msgPtr, uint32_t timeout);
	/* 获取队列当前容量 */
	SROSStatus SRThreadQueueGetCnt(SRQueue_t *msgQRef, uint32_t *cnt_ptr);
	/* 销毁消息队列 */
	SROSStatus SRThreadQueueDelete(SRQueue_t *msgQRef);
	/* 线程休眠 x 毫秒 */
	void SRThreadSleepMs(uint32_t ms);
	/* 线程休眠 x 秒 */
	void SRThreadSleepS(uint32_t s);
#ifdef __cplusplus
}
#endif
#endif
