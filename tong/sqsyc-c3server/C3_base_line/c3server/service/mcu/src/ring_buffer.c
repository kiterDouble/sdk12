#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err_code.h"
#include "sr_log.h"
#include "ring_buffer.h"

typedef struct {
    uint8_t *buffer;
    int32_t size;
    int32_t start;
    int32_t end;
    int32_t count;
    pthread_mutex_t mutex;
} RingBufferPriv_t;

/**
 * @name RingBufferCreate
 * @brief 创建一个环形缓冲区
 * @param rbuf 指向RingBuffer_t结构体的指针，该结构体将被初始化为环形缓冲区的句柄。
 * @param bufSize 环形缓冲区的大小（字节为单位）。
 * @return 成功返回RET_OK（假设为0），否则返回RET_ERR（假设为非0值）。
 */
int32_t RingBufferCreate(RingBuffer_t *rbuf, uint32_t bufSize)
{
    int32_t ret = RET_ERR;

    if (!rbuf || (bufSize == 0U))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    RingBufferPriv_t *priv = (RingBufferPriv_t *)malloc(sizeof(RingBufferPriv_t));
    if (!priv)
    {
        LOG_Error("%s Failed to allocate memory for ring buffer", __FUNCTION__);
        return ret;
    }

    priv->buffer = (uint8_t *)malloc(bufSize);
    if (!priv->buffer)
    {
        LOG_Error("%s Failed to allocate memory for buffer in ring buffer", __FUNCTION__);
        free(priv);
        return ret;
    }

    priv->size = (int32_t)bufSize;
    priv->start = 0;
    priv->end = 0;
    priv->count = 0; // 初始化数据个数为0

    if (pthread_mutex_init(&priv->mutex, NULL) != 0)
    {
        LOG_Error("%s Failed to initialize mutex", __FUNCTION__);
        free(priv->buffer);
        free(priv);
        return ret;
    }

    *rbuf = priv;
    ret = RET_OK;
    return ret;
}

/**
 * @name RingBufferGetAvailableSize
 * @brief 获取环形缓冲区中可用空间的大小
 *        本函数用于获取环形缓冲区中尚未被使用的空间大小。在获取大小之前，会首先检查传入的参数是否有效，
 *        并通过互斥锁确保在多线程环境下的安全性。
 * @param rbuf 环形缓冲区的公有结构体指针。
 * @param avaiLen 指向一个无符号整型变量的指针，函数执行成功后，会将可用空间的大小赋值给该变量。
 * @return 函数执行成功返回 RET_OK（定义应为0），参数无效或其他错误返回 RET_ERR（定义应为非0）。
 */
int32_t RingBufferGetAvailableSize(RingBuffer_t rbuf, uint32_t *avaiLen)
{
    RingBufferPriv_t *priv = (RingBufferPriv_t *)rbuf;
    int32_t ret = RET_ERR;

    if ((NULL == priv) || (NULL == avaiLen))
    {
        LOG_Error("%s param invalid", __FUNCTION__);
        return ret;
    }
    (void)pthread_mutex_lock(&priv->mutex);
    *avaiLen = (uint32_t)priv->count;
    (void)pthread_mutex_unlock(&priv->mutex);
    ret = RET_OK;
    return ret;
}

/**
 * @name RingBufferGetFreeSize
 * @brief 获取环形缓冲区中剩余的可用空间大小
 * @param rbuf 环形缓冲区的公有接口结构体
 * @param freeLen 指向一个 uint32_t 类型变量的指针，用于存储获取到的剩余空间大小
 * @return 函数执行状态，RET_OK 表示成功，RET_ERR 表示失败
 */
int32_t RingBufferGetFreeSize(RingBuffer_t rbuf, uint32_t *freeLen)
{
    RingBufferPriv_t *priv = (RingBufferPriv_t *)rbuf;
    int32_t ret = RET_ERR;

    if ((NULL == priv) || (NULL == freeLen))
    {
        LOG_Error("%s param invalid", __FUNCTION__);
        return ret;
    }
    (void)pthread_mutex_lock(&priv->mutex);
    *freeLen = (uint32_t)priv->size - (uint32_t)priv->count;
    (void)pthread_mutex_unlock(&priv->mutex);
    ret = RET_OK;
    return ret;
}

/**
 * @name RingBufferEnqueue
 * @brief 向环形缓冲区中添加数据。
 * @param rbuf 环形缓冲区对象的公有接口。
 * @param data 需要添加到缓冲区的数据指针。
 * @param n 要添加的数据字节的数量。
 * @return 若成功添加数据返回RET_OK，否则返回RET_ERR。
 */
int32_t RingBufferEnqueue(RingBuffer_t rbuf, const uint8_t *data, uint32_t n)
{
    RingBufferPriv_t *priv = (RingBufferPriv_t *)rbuf;
    int32_t ret = RET_ERR;

    if ((NULL == priv) || (NULL == data))
    {
        LOG_Error("%s param invalid", __FUNCTION__);
        return ret;
    }
    (void)pthread_mutex_lock(&priv->mutex);

    if ((n == 0U) || (n > (uint32_t)priv->size))
    {
        LOG_Error("%s Error: Invalid number of bytes to enqueue.", __FUNCTION__);
        (void)pthread_mutex_unlock(&priv->mutex);
        return ret;
    }

    if (priv->count == priv->size) //is full
    {
        LOG_Error("%s Error: Ring buffer is full. Cannot enqueue.", __FUNCTION__);
        (void)pthread_mutex_unlock(&priv->mutex);
        return ret;
    }

    if (((int32_t)n + priv->count) > priv->size)
    {
        LOG_Error("%s Error: Ring buffer free space not enough. Cannot enqueue.", __FUNCTION__);
        (void)pthread_mutex_unlock(&priv->mutex);
        return ret;
    }

    int32_t i;
    for (i = 0; (uint32_t)i < n; ++i)
    {
        priv->buffer[priv->end] = data[i];
        if (priv->size > 0)//缓冲区非空
        {
            priv->end = (priv->end + 1) % priv->size;
        }
        priv->count++;
    }

    (void)pthread_mutex_unlock(&priv->mutex);
    ret = RET_OK;
    return ret;
}

/**
 * @name RingBufferPeek
 * @brief 从环形缓冲区中预览数据，不改变缓冲区状态
 * @param rbuf 环形缓冲区对象
 * @param data 指向接收数据的缓冲区的指针
 * @param n 要读取的数据元素个数
 * @return 若成功返回RET_OK，否则返回RET_ERR
 */
int32_t RingBufferPeek(RingBuffer_t rbuf, uint8_t *data, uint32_t n)
{
    RingBufferPriv_t *priv = (RingBufferPriv_t *)rbuf;
    int32_t ret = RET_ERR;

    if (!priv || !data || (0U == n))
    {
        LOG_Error("%s param invalid", __FUNCTION__);
        return ret;
    }

    (void)pthread_mutex_lock(&priv->mutex);

    if ((n == 0U) || ((uint32_t)priv->count < n))
    {
        LOG_Error("%s Error: Insufficient data available for peek.", __FUNCTION__);
        (void)pthread_mutex_unlock(&priv->mutex);
        return ret;
    }

    int32_t i;
    for (i = 0; (uint32_t)i < n; ++i)
    {
        if (priv->size > 0)//缓冲区非空
        {
            data[i] = priv->buffer[(priv->start + i) % priv->size];
        }
    }

    (void)pthread_mutex_unlock(&priv->mutex);
    ret = RET_OK;
    return ret;
}

/**
 * @name RingBufferDequeue
 * @brief 从环形缓冲区中出队指定数量的数据。
 * @param rbuf 环形缓冲区对象的公有接口指针。
 * @param data 指向接收数据的缓冲区的指针。
 * @param n 指定要出队的数据数量。
 * @return 函数执行状态码，RET_OK 表示成功，RET_ERR 表示错误。
 */
int32_t RingBufferDequeue(RingBuffer_t rbuf, uint8_t *data, uint32_t n)
{
    RingBufferPriv_t *priv = (RingBufferPriv_t *)rbuf;
    int32_t ret = RET_ERR;

    if (!priv || !data)
    {
        LOG_Error("%s param invalid", __FUNCTION__);
        return ret;
    }

    (void)pthread_mutex_lock(&priv->mutex);
    if ((n == 0U) || ((uint32_t)priv->count < n))
    {
        /* 约定：可调用方先用 GetAvailableSize 决定 n */
        (void)pthread_mutex_unlock(&priv->mutex);
        return RET_OK;
    }

    int32_t i;
    for (i = 0; (uint32_t)i < n; ++i)
    {
        data[i] = priv->buffer[priv->start];
        if (priv->size > 0)
        {
            priv->start = (priv->start + 1) % priv->size;
        }
        priv->count--;
    }

    (void)pthread_mutex_unlock(&priv->mutex);
    ret = RET_OK;
    return ret;
}

/**
 * @name RingBufferDestroy
 * @brief 销毁环形缓冲区
 *        本函数用于销毁一个已经创建的环形缓冲区对象，释放相关资源。
 * @param rbuf 环形缓冲区对象的句柄。
 * @return 函数执行状态码。RET_OK 表示成功，RET_ERR 表示失败。
 */
int32_t RingBufferDestroy(RingBuffer_t rbuf)
{
    RingBufferPriv_t *priv = (RingBufferPriv_t *)rbuf;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s param invalid", __FUNCTION__);
        return ret;
    }

    (void)pthread_mutex_destroy(&priv->mutex);
    free(priv->buffer);
    free(priv);
    ret = RET_OK;
    return ret;
}