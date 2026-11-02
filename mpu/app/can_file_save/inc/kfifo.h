#ifndef __KFIFO_H__
#define __KFIFO_H__

#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t            *buffer;
    uint32_t            size;        /* 必须 2^N */
    uint32_t            in;          /* 写偏移累计 */
    uint32_t            out;         /* 读偏移累计 */
    uint32_t            elem_size;   /* 帧大小(字节)，0=字节流 */
    pthread_spinlock_t  lock;
} kfifo_t;

/* 初始化 */
int  kfifo_init(kfifo_t *fifo, uint32_t size);
int  kfifo_init_elem(kfifo_t *fifo, uint32_t size, uint32_t elem_size);
void kfifo_free(kfifo_t *fifo);
void kfifo_reset(kfifo_t *fifo);

/* 状态 */
uint32_t kfifo_len(kfifo_t *fifo);
uint32_t kfifo_avail(kfifo_t *fifo);

/* 数据操作 */
uint32_t kfifo_in(kfifo_t *fifo, const void *buf, uint32_t len);
uint32_t kfifo_out(kfifo_t *fifo, void *buf, uint32_t len);
uint32_t kfifo_peek(kfifo_t *fifo, void *buf, uint32_t len);
uint32_t kfifo_peek_at(kfifo_t *fifo, void *buf, uint32_t len, uint32_t offset);

/* 跳过帧 */
typedef int (*kfifo_compare_fn)(const void *elem, void *context);
uint32_t kfifo_skip_to_match(kfifo_t *fifo, kfifo_compare_fn compare, void *context);

/* 原子操作：在单次持锁内完成跳过+批量peek，消除竞态 */
uint32_t kfifo_peek_range(kfifo_t *fifo, void *frames_out, uint32_t max_frames,
                          uint32_t *consumed_bytes, int *end_reached,
                          kfifo_compare_fn skip_fn, void *skip_ctx,
                          kfifo_compare_fn stop_fn, void *stop_ctx);

/* 工具 */
uint64_t roundup_pow_of_two_u64(uint64_t x);

#ifdef __cplusplus
}
#endif

#endif