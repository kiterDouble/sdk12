#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__
#include <cstdint>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>

class MessageQueue {
public:
    MessageQueue();
    ~MessageQueue();
    int Send(const uint8_t *data, uint32_t len);
    int Recv(uint8_t *data, uint32_t *plen, long ms);
    void Clear();

private:
    std::queue<std::vector<uint8_t>> mQueue; // UDS回复消息队列
    std::mutex mLock;                        // 队列锁
    std::condition_variable mNotEmpty;       // 队列非空条件变量
};

#endif