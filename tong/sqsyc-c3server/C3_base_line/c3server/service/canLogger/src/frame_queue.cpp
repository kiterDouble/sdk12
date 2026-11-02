/**
 * @file frame_queue.cpp
 * @brief FrameQueue 单例实现：deque + pthread mutex/cond
 */
#include "frame_queue.h"

#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <cstring>
#include <deque>

#include "sr_log.h"

/**
 * @brief FrameQueue 内部状态（PImpl 惯用法）
 */
struct FrameQueue::Impl
{
    std::deque<CanFrameRecord_t>     q;          ///< 帧容器
    pthread_mutex_t                  mtx;        ///< 互斥锁，保护 q 与所有计数器
    pthread_cond_t                   cv;         ///< 非空信号 / 关停广播
    bool                             shutdown;   ///< 是否已关停
    size_t                           dropped;    ///< 累计丢弃帧数（超 maxDepth）
    size_t                           maxDepth;   ///< 队列深度上限
    size_t                           totalPushed;///< 累计入队帧数

    Impl()
        : shutdown(false), dropped(0), maxDepth(200000u), totalPushed(0)
    {
        pthread_mutex_init(&mtx, NULL);
        pthread_cond_init(&cv, NULL);
    }
    ~Impl()
    {
        pthread_mutex_destroy(&mtx);
        pthread_cond_destroy(&cv);
    }
};

static bool IsSameCanFrameRecord(const CanFrameRecord_t &a,
                                 const CanFrameRecord_t &b)
{
    if (a.channel != b.channel || a.flags != b.flags || a.dlc != b.dlc ||
        a.canId != b.canId || a.timestampMs != b.timestampMs ||
        a.timestampUs != b.timestampUs) {
        return false;
    }
    return std::memcmp(a.canData, b.canData, a.dlc) == 0;
}

FrameQueue &FrameQueue::instance()
{
    static FrameQueue inst;
    return inst;
}

FrameQueue::FrameQueue() : impl_(new Impl()) {}
FrameQueue::~FrameQueue() { delete impl_; }

/**
 * @brief 把"从现在起等待 ms 毫秒"转换为绝对 timespec
 * @param[in]  ms 等待毫秒数
 * @param[out] ts 输出绝对时间（用于 pthread_cond_timedwait）
 */
static void msToTimespec(uint32_t ms, struct timespec &ts)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    ts.tv_sec  = now.tv_sec + (time_t)(ms / 1000u);
    ts.tv_nsec = (long)(now.tv_usec * 1000L + (long)(ms % 1000u) * 1000000L);
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }
}

void FrameQueue::pushBatch(const CanFrameRecord_t *frames, size_t count)
{
    /* Guard against FIFO/SPI retransmits that carry the same frame twice. */
    static CanFrameRecord_t last = {};
    static bool lastValid = false;

    pthread_mutex_lock(&impl_->mtx);
    for (size_t i = 0; i < count; ++i) {
        if (lastValid && IsSameCanFrameRecord(last, frames[i])) {
            continue;
        }
        last = frames[i];
        lastValid = true;

        if (impl_->q.size() >= impl_->maxDepth) {
            impl_->q.pop_front();
            impl_->dropped++;
            if ((impl_->dropped % 10000u) == 0u && impl_->dropped > 0) {
                LOG_Warn("FrameQueue overflow, dropped total=%zu", impl_->dropped);
            }
        }
        impl_->q.push_back(frames[i]);
        impl_->totalPushed++;
    }
    pthread_cond_signal(&impl_->cv);
    pthread_mutex_unlock(&impl_->mtx);
}

size_t FrameQueue::popBatch(std::vector<CanFrameRecord_t> &out, size_t maxCount,
                            uint32_t timeoutMs)
{
    struct ts { };
    struct timespec deadline;
    msToTimespec(timeoutMs, deadline);

    pthread_mutex_lock(&impl_->mtx);
    out.clear();
    while (impl_->q.empty() && !impl_->shutdown) {
        int rc = pthread_cond_timedwait(&impl_->cv, &impl_->mtx, &deadline);
        if (rc == ETIMEDOUT) {
            break;
        }
    }

    size_t n = 0;
    if (!impl_->q.empty()) {
        n = impl_->q.size();
        if (n > maxCount) n = maxCount;
        out.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            out.push_back(impl_->q.front());
            impl_->q.pop_front();
        }
    }
    pthread_mutex_unlock(&impl_->mtx);
    return n;
}

void FrameQueue::shutdown()
{
    pthread_mutex_lock(&impl_->mtx);
    impl_->shutdown = true;
    pthread_cond_broadcast(&impl_->cv);
    pthread_mutex_unlock(&impl_->mtx);
}

bool FrameQueue::isShutdown() const
{
    return impl_->shutdown;
}
