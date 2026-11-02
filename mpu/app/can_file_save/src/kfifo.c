#include "kfifo.h"
#include <stdlib.h>
#include <string.h>

#define uatomic_init(a)      pthread_spin_init((a), PTHREAD_PROCESS_PRIVATE)
#define uatomic_lock(a)      pthread_spin_lock(a)
#define uatomic_unlock(a)    pthread_spin_unlock(a)

static inline int is_power_of_2(uint32_t x)
{
    return x && !(x & (x - 1));
}

/* ── 字节流初始化（兼容旧接口） ── */
int kfifo_init(kfifo_t *fifo, uint32_t size)
{
    if (!fifo || !is_power_of_2(size))
        return -1;

    fifo->buffer = (uint8_t *)malloc(size);
    if (!fifo->buffer)
        return -1;

    fifo->size = size;
    fifo->in = 0;
    fifo->out = 0;
    fifo->elem_size = 0;

    if (uatomic_init(&fifo->lock) != 0) {
        free(fifo->buffer);
        fifo->buffer = NULL;
        return -1;
    }
    return 0;
}

/* ── 帧对齐初始化 ── */
int kfifo_init_elem(kfifo_t *fifo, uint32_t size, uint32_t elem_size)
{
    if (!fifo || !is_power_of_2(size) || elem_size == 0)
        return -1;

    fifo->buffer = (uint8_t *)malloc(size);
    if (!fifo->buffer)
        return -1;

    fifo->size = size;
    fifo->in = 0;
    fifo->out = 0;
    fifo->elem_size = elem_size;

    if (uatomic_init(&fifo->lock) != 0) {
        free(fifo->buffer);
        fifo->buffer = NULL;
        return -1;
    }
    return 0;
}

void kfifo_free(kfifo_t *fifo)
{
    if (!fifo) return;
    if (fifo->buffer) {
        free(fifo->buffer);
        fifo->buffer = NULL;
    }
    fifo->size = 0;
    fifo->in = 0;
    fifo->out = 0;
    fifo->elem_size = 0;
    pthread_spin_destroy(&fifo->lock);
}

void kfifo_reset(kfifo_t *fifo)
{
    if (!fifo) return;
    uatomic_lock(&fifo->lock);
    fifo->in = 0;
    fifo->out = 0;
    uatomic_unlock(&fifo->lock);
}

uint32_t kfifo_len(kfifo_t *fifo)
{
    if (!fifo) return 0;
    uatomic_lock(&fifo->lock);
    uint32_t len = fifo->in - fifo->out;
    uatomic_unlock(&fifo->lock);
    return len;
}

uint32_t kfifo_avail(kfifo_t *fifo)
{
    if (!fifo) return 0;
    uatomic_lock(&fifo->lock);
    uint32_t avail = fifo->size - (fifo->in - fifo->out);
    uatomic_unlock(&fifo->lock);
    return avail;
}

/* ── 写入（带帧对齐覆盖） ── */
uint32_t kfifo_in(kfifo_t *fifo, const void *buf, uint32_t len)
{
    uint32_t l;
    uint32_t avail;

    if (!fifo || !buf || len == 0)
        return 0;
    if (len > fifo->size)
        return 0;

    // 帧模式：写入长度必须为帧大小的整数倍
    if (fifo->elem_size > 0 && (len % fifo->elem_size != 0))
        return 0;

    uatomic_lock(&fifo->lock);

    avail = fifo->size - (fifo->in - fifo->out);
    if (len > avail) {
        uint32_t need = len - avail;

        if (fifo->elem_size > 0) {
            // 直接按帧数丢弃（不向下取整，uint32 溢出后 out 的偏移是自洽的）
            uint32_t extra_frames = (need + fifo->elem_size - 1) / fifo->elem_size;
            fifo->out += extra_frames * fifo->elem_size;
        } else {
            fifo->out += need;
        }
    }

    // 环形拷贝
    l = (len < (fifo->size - (fifo->in & (fifo->size - 1)))) ?
        len : (fifo->size - (fifo->in & (fifo->size - 1)));

    memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buf, l);
    memcpy(fifo->buffer, (uint8_t *)buf + l, len - l);

    fifo->in += len;

    uatomic_unlock(&fifo->lock);
    return len;
}

/* ── 读取 ── */
uint32_t kfifo_out(kfifo_t *fifo, void *buf, uint32_t len)
{
    uint32_t l;

    if (!fifo || !buf || len == 0)
        return 0;

    uatomic_lock(&fifo->lock);

    if (len > (fifo->in - fifo->out))
        len = fifo->in - fifo->out;

    l = (len < (fifo->size - (fifo->out & (fifo->size - 1)))) ?
        len : (fifo->size - (fifo->out & (fifo->size - 1)));

    memcpy(buf, fifo->buffer + (fifo->out & (fifo->size - 1)), l);
    memcpy((uint8_t *)buf + l, fifo->buffer, len - l);

    fifo->out += len;

    uatomic_unlock(&fifo->lock);
    return len;
}

/* ── 查看 ── */
uint32_t kfifo_peek(kfifo_t *fifo, void *buf, uint32_t len)
{
    uint32_t l;
    uint32_t out;

    if (!fifo || !buf || len == 0)
        return 0;

    uatomic_lock(&fifo->lock);

    if (len > (fifo->in - fifo->out))
        len = fifo->in - fifo->out;

    out = fifo->out;

    l = (len < (fifo->size - (out & (fifo->size - 1)))) ?
        len : (fifo->size - (out & (fifo->size - 1)));

    memcpy(buf, fifo->buffer + (out & (fifo->size - 1)), l);
    memcpy((uint8_t *)buf + l, fifo->buffer, len - l);

    uatomic_unlock(&fifo->lock);
    return len;
}

/* ── 带偏移量的查看，不移除数据 ── */
uint32_t kfifo_peek_at(kfifo_t *fifo, void *buf, uint32_t len, uint32_t offset)
{
    uint32_t l;
    uint32_t out;
    uint32_t available;

    if (!fifo || !buf || len == 0 || !fifo->buffer)
        return 0;

    uatomic_lock(&fifo->lock);

    available = fifo->in - fifo->out;

    /* offset 超出可用数据范围，返回 0 */
    if (offset >= available) {
        uatomic_unlock(&fifo->lock);
        return 0;
    }

    /* 调整 len 以支持部分读取（与 kfifo_peek 行为一致） */
    if (len > available - offset)
        len = available - offset;

    /* 计算实际读取位置 */
    out = fifo->out + offset;

    l = (len < (fifo->size - (out & (fifo->size - 1)))) ?
        len : (fifo->size - (out & (fifo->size - 1)));

    memcpy(buf, fifo->buffer + (out & (fifo->size - 1)), l);
    memcpy((uint8_t *)buf + l, fifo->buffer, len - l);

    uatomic_unlock(&fifo->lock);
    return len;
}

/* ── 按帧跳过，直到 compare 返回非0 ── */
uint32_t kfifo_skip_to_match(kfifo_t *fifo, kfifo_compare_fn compare, void *context)
{
    uint32_t skipped = 0;
    uint32_t elem_size;
    uint8_t *elem_buf;

    if (!fifo || !compare)
        return 0;

    elem_size = fifo->elem_size;
    if (elem_size == 0)
        return 0;

    elem_buf = (uint8_t *)malloc(elem_size);
    if (!elem_buf)
        return 0;

    uatomic_lock(&fifo->lock);

    while ((fifo->in - fifo->out) >= elem_size) {
        uint32_t out = fifo->out;
        uint32_t l = (elem_size < (fifo->size - (out & (fifo->size - 1)))) ?
                     elem_size : (fifo->size - (out & (fifo->size - 1)));

        memcpy(elem_buf, fifo->buffer + (out & (fifo->size - 1)), l);
        memcpy(elem_buf + l, fifo->buffer, elem_size - l);

        if (compare(elem_buf, context) != 0) {
            uatomic_unlock(&fifo->lock);
            free(elem_buf);
            return skipped;
        }

        fifo->out += elem_size;
        skipped += elem_size;
    }

    uatomic_unlock(&fifo->lock);
    free(elem_buf);
    return skipped;
}

/* ── 原子操作：单次持锁内完成跳过 + 批量peek ──
 * 注意：此函数只 peek（查看）数据，不移除数据。
 * Phase 1 跳过的帧会被立即移除（更新 fifo->out）。
 * Phase 2 收集的帧不会被移除，需要调用者在收到 ACK 后调用 kfifo_out 移除。
 * consumed_bytes 返回总共消耗的字节数（跳过 + 收集），用于调用者后续移除。
 */
uint32_t kfifo_peek_range(kfifo_t *fifo, void *frames_out, uint32_t max_frames,
                          uint32_t *consumed_bytes, int *end_reached,
                          kfifo_compare_fn skip_fn, void *skip_ctx,
                          kfifo_compare_fn stop_fn, void *stop_ctx)
{
    uint32_t frame_count = 0;
    uint32_t skipped_bytes = 0;
    uint32_t elem_size;
    uint8_t *scan_buf;
    uint8_t *out_buf = (uint8_t *)frames_out;

    if (!fifo || !frames_out || max_frames == 0 || !skip_fn || !stop_fn || !consumed_bytes || !end_reached)
        return 0;

    elem_size = fifo->elem_size;
    if (elem_size == 0)
        return 0;

    *consumed_bytes = 0;
    *end_reached = 0;

    scan_buf = (uint8_t *)malloc(elem_size);
    if (!scan_buf)
        return 0;

    uatomic_lock(&fifo->lock);

    uint32_t pos = fifo->out;

    /* Phase 1: 跳过帧，直到 skip_fn 返回非0 */
    while ((fifo->in - pos) >= elem_size) {
        uint32_t idx = pos & (fifo->size - 1);
        uint32_t first = (elem_size < (fifo->size - idx)) ? elem_size : (fifo->size - idx);
        uint32_t second = elem_size - first;

        memcpy(scan_buf, fifo->buffer + idx, first);
        if (second)
            memcpy(scan_buf + first, fifo->buffer, second);

        if (skip_fn(scan_buf, skip_ctx) != 0)
            break;

        pos += elem_size;
        skipped_bytes += elem_size;
    }

    /* Phase 1 跳过的帧立即移除，防止生产者覆盖旧数据 */
    if (skipped_bytes > 0) {
        fifo->out = pos;
    }

    /* Phase 2: 收集帧，直到 stop_fn 返回非0 */
    while (frame_count < max_frames && (fifo->in - pos) >= elem_size) {
        uint32_t idx = pos & (fifo->size - 1);
        uint32_t first = (elem_size < (fifo->size - idx)) ? elem_size : (fifo->size - idx);
        uint32_t second = elem_size - first;

        memcpy(scan_buf, fifo->buffer + idx, first);
        if (second)
            memcpy(scan_buf + first, fifo->buffer, second);

        if (stop_fn(scan_buf, stop_ctx) != 0) {
            *end_reached = 1;
            break;
        }

        memcpy(out_buf + frame_count * elem_size, scan_buf, elem_size);
        frame_count++;
        pos += elem_size;
    }

    /* Phase 2 收集的帧不移除，由调用者在收到 ACK 后调用 kfifo_out 移除
     * consumed_bytes 只包含 Phase 2 收集的字节数（Phase 1 已移除）
     */
    *consumed_bytes = frame_count * elem_size;

    uatomic_unlock(&fifo->lock);
    free(scan_buf);
    return frame_count;
}

uint64_t roundup_pow_of_two_u64(uint64_t x)
{
    if (x == 0) return 1;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    x++;
    return x;
}
