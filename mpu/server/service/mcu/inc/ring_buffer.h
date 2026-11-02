
#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__
#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef void* RingBuffer_t;

/**
 * @name RingBufferCreate
 * @brief 创建一个环形缓冲区
 * @param rbuf 指向RingBuffer_t结构体的指针，该结构体将被初始化为环形缓冲区的句柄。
 * @param bufSize 环形缓冲区的大小（字节为单位）。
 * @return 成功返回RET_OK（假设为0），否则返回RET_ERR（假设为非0值）。
 */
int32_t RingBufferCreate(RingBuffer_t *rbuf, uint32_t bufSize);

/**
 * @name RingBufferGetAvailableSize
 * @brief 获取环形缓冲区中可用空间的大小
 *        本函数用于获取环形缓冲区中尚未被使用的空间大小。在获取大小之前，会首先检查传入的参数是否有效，
 *        并通过互斥锁确保在多线程环境下的安全性。
 * @param rbuf 环形缓冲区的公有结构体指针。
 * @param avaiLen 指向一个无符号整型变量的指针，函数执行成功后，会将可用空间的大小赋值给该变量。
 * @return 函数执行成功返回 RET_OK（定义应为0），参数无效或其他错误返回 RET_ERR（定义应为非0）。
 */
int32_t RingBufferGetAvailableSize(RingBuffer_t rbuf, uint32_t *avaiLen);

/**
 * @name RingBufferGetFreeSize
 * @brief 获取环形缓冲区中剩余的可用空间大小
 * @param rbuf 环形缓冲区的公有接口结构体
 * @param freeLen 指向一个 uint32_t 类型变量的指针，用于存储获取到的剩余空间大小
 * @return 函数执行状态，RET_OK 表示成功，RET_ERR 表示失败
 */
int32_t RingBufferGetFreeSize(RingBuffer_t rbuf, uint32_t *freeLen);

/**
 * @name RingBufferEnqueue
 * @brief 向环形缓冲区中添加数据。
 * @param rbuf 环形缓冲区对象的公有接口。
 * @param data 需要添加到缓冲区的数据指针。
 * @param n 要添加的数据字节的数量。
 * @return 若成功添加数据返回RET_OK，否则返回RET_ERR。
 */
int32_t RingBufferEnqueue(RingBuffer_t rbuf, const uint8_t *data, uint32_t n);


/**
 * @name RingBufferPeek
 * @brief 从环形缓冲区中预览数据，不改变缓冲区状态
 * @param rbuf 环形缓冲区对象
 * @param data 指向接收数据的缓冲区的指针
 * @param n 要读取的数据元素个数
 * @return 若成功返回RET_OK，否则返回RET_ERR
 */
int32_t RingBufferPeek(RingBuffer_t rbuf, uint8_t *data, uint32_t n);


/**
 * @name RingBufferDequeue
 * @brief 从环形缓冲区中出队指定数量的数据。
 * @param rbuf 环形缓冲区对象的公有接口指针。
 * @param data 指向接收数据的缓冲区的指针。
 * @param n 指定要出队的数据数量。
 * @return 函数执行状态码，RET_OK 表示成功，RET_ERR 表示错误。
 */
int32_t RingBufferDequeue(RingBuffer_t rbuf, uint8_t *data, uint32_t n);

/**
 * @name RingBufferDestroy
 * @brief 销毁环形缓冲区
 *        本函数用于销毁一个已经创建的环形缓冲区对象，释放相关资源。
 * @param rbuf 环形缓冲区对象的句柄。
 * @return 函数执行状态码。RET_OK 表示成功，RET_ERR 表示失败。
 */
int32_t RingBufferDestroy(RingBuffer_t rbuf);
#ifdef __cplusplus
}
#endif
#endif
