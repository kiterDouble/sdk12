/**
 * @brief 
 * 
 * @file sr_cache_tsp.c
 * @author lujiefeng
 * @date 2018-08-28
 */

#include <glib.h>
#include <string.h>
#include "tsp_cache_queue.h"
#include "sr_log.h"
#include "err_code.h"


#if defined __cplusplus
extern "C" {
#endif


typedef struct
{
    uint32_t dataLen;
    uint8_t data[0];
} CacheDataStr;

GAsyncQueue *CreadNewCacheQueue()
{
    return g_async_queue_new();
}

void CacheDataToQueue(GAsyncQueue *queue, uint8_t *u8buff, uint32_t u32length)
{
    if (queue == NULL)
    {
        LOG_Error("queue is NULL");
        return;
    }
    if (u8buff == NULL)
    {
        LOG_Error("The Data Point is NULL");
        return;
    }
    if (u32length <= 0)
    {
        LOG_Error("The Data Length is invalid");
        return;
    }
    CacheDataStr *data = (CacheDataStr *)g_try_malloc0(sizeof(CacheDataStr) + u32length);
    if (data == NULL)
    {
        LOG_Error("New Cache Data Failed");
        return;
    }
    data->dataLen = u32length;
    g_memmove(data->data, u8buff, u32length);
    g_async_queue_push(queue, (gpointer)data);
    LOG_Debug("Cache Recv Data Success");
    return;
}

int32_t GetDataFromQueue(GAsyncQueue *queue, uint8_t *u8buff, uint32_t *u32length)
{
    if (queue == NULL)
    {
        LOG_Error("queue is NULL");
        return -1;
    }
    if (u8buff == NULL)
    {
        LOG_Error("The Data Point is NULL");
        return -1;
    }
    CacheDataStr *popTemp = (CacheDataStr *)g_async_queue_pop(queue);
    g_memmove(u8buff, popTemp->data, popTemp->dataLen);
    *u32length = popTemp->dataLen;
    g_free((gpointer)popTemp);
    LOG_Debug("Pop Data from Queue Success");
    return 0;
}

int32_t GetDataWithTimeOutFromQueue(GAsyncQueue *queue, uint8_t *u8buff, uint32_t *u32length, uint32_t ms)
{
    if (queue == NULL)
    {
        LOG_Error("queue is NULL");
        return -1;
    }
    if (u8buff == NULL)
    {
        LOG_Error("The Data Point is NULL");
        return -1;
    }
    CacheDataStr *popTemp = (CacheDataStr *)g_async_queue_timeout_pop(queue, ms * 1000);
    if (popTemp == NULL)
    {
        return -1;
    }
    g_memmove(u8buff, popTemp->data, popTemp->dataLen);
    *u32length = popTemp->dataLen;
    g_free((gpointer)popTemp);
    LOG_Debug("Pop Data from Queue Success with timeout %d", ms);
    return 0;
}

void CacheDataToQueueWithoutMalloc(GAsyncQueue *queue, gpointer pData)
{
    if (queue == NULL)
    {
        LOG_Error("queue is NULL");
        return;
    }
    if (pData == NULL)
    {
        LOG_Error("The Data Point is NULL");
        return;
    }

    g_async_queue_push(queue, (gpointer)pData);
    LOG_Debug("Cache Recv Data Success");
    return;
}

int32_t GetDataFromQueueWithoutFree(GAsyncQueue *queue, gpointer *pData)
{
    if (queue == NULL)
    {
        LOG_Error("queue is NULL");
        return -1;
    }
    if (pData == NULL)
    {
        LOG_Error("The Data Point is NULL");
        return -1;
    }
    CacheDataStr *popTemp = (CacheDataStr *)g_async_queue_pop(queue);
    if (popTemp == NULL)
    {
        return -1;
    }
    *pData = (gpointer) popTemp;
    LOG_Debug("Pop Data from Queue Success");
    return 0;
}

int32_t GetDataWithTimeOutFromQueueWithoutFree(GAsyncQueue *queue, gpointer *pData, uint32_t ms)
{
    if (queue == NULL)
    {
        LOG_Error("queue is NULL");
        return -1;
    }
    if (pData == NULL)
    {
        LOG_Error("The Data Point is NULL");
        return -1;
    }
    CacheDataStr *popTemp = (CacheDataStr *)g_async_queue_timeout_pop(queue, ms * 1000);
    if (popTemp == NULL)
    {
        return -1;
    }
    *pData = (gpointer) popTemp;
    LOG_Debug("Pop Data from Queue Success with timeout %d", ms);
    return 0;
}

int32_t GetLengthFromQueue(GAsyncQueue *queue)
{
    return g_async_queue_length(queue);
}


#if defined __cplusplus
}
#endif

