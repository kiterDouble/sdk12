/**
 * @file frame_queue.h
 * @brief 生产/消费解耦的有界 CAN 帧队列（单例）
 *
 * @details
 * IPC 接收线程调用 pushBatch 入队，BLF 写线程调用 popBatch 出队。
 * 内部用 std::deque + pthread mutex/cond 实现；超过 maxDepth 时
 * 丢弃最旧帧以保证实时性。
 */
#ifndef _FRAME_QUEUE_H_
#define _FRAME_QUEUE_H_

#include <cstdint>
#include <cstddef>
#include <vector>

#include "can_raw_frame.h"

/**
 * @brief 单例有界帧队列
 *
 * @details 多线程安全；深度上限 200000，溢出采用"丢最旧"策略。
 * 优雅退出通过 shutdown() + broadcast 唤醒所有等待者。
 */
class FrameQueue
{
public:
    /**
     * @brief 获取单例引用
     * @return 全局唯一 FrameQueue 实例
     */
    static FrameQueue &instance();

    /**
     * @brief 批量入队
     * @param[in] frames 帧数组指针，不能为 NULL
     * @param[in] count  帧数量
     *
     * @note 队列满时丢弃最旧帧并累计 dropped 计数；每丢 10000 帧打一次警告。
     */
    void pushBatch(const CanFrameRecord_t *frames, size_t count);

    /**
     * @brief 批量出队（带超时）
     * @param[out] out      输出 vector，调用前内容会被 clear
     * @param[in]  maxCount 一次最多取的帧数
     * @param[in]  timeoutMs 等待超时（毫秒），超时返回 0
     * @return 实际取出的帧数（0 表示超时或队列空且未关停）
     *
     * @note 队列空且未 shutdown 时阻塞等待，被 shutdown 唤醒后立即返回。
     */
    size_t popBatch(std::vector<CanFrameRecord_t> &out, size_t maxCount,
                    uint32_t timeoutMs);

    /**
     * @brief 通知队列进入关停状态
     *
     * @details 置 shutdown 标志并 broadcast 唤醒所有阻塞的 popBatch。
     */
    void shutdown();

    /**
     * @brief 查询是否已关停
     * @return true 表示已调用过 shutdown()
     */
    bool isShutdown() const;

private:
    FrameQueue();
    ~FrameQueue();
    FrameQueue(const FrameQueue &) = delete;
    FrameQueue &operator=(const FrameQueue &) = delete;

    struct Impl;
    Impl *impl_;
};

#endif
