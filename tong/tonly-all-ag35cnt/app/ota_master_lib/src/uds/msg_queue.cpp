#include <cstring>
#include "msg_queue.h"
#include "ota_master_log.h"
#include "ota_master_error_code.h"

MessageQueue::MessageQueue() {}

MessageQueue::~MessageQueue() {}

/**
 * @brief 发送消息到队列
 * @param data 要发送的数据缓冲区
 * @param len 数据长度
 * @return OTA_RET_OK 发送成功
 *         OTA_RET_ERR 发送失败(参数无效)
 */
int MessageQueue::Send(const uint8_t *data, uint32_t len)
{
    if (!data || !len) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    std::lock_guard<std::mutex> lock(mLock);
    mQueue.push(std::vector<uint8_t>(data, data + len));
    mNotEmpty.notify_one();
    return OTA_RET_OK;
}

/**
 * @brief 从队列接收消息
 * @param data 接收数据的缓冲区
 * @param plen 输入缓冲区大小，输出实际接收的数据长度
 * @param ms 超时时间(毫秒)
 *          0: 永久等待直到收到数据
 *          >0: 等待指定的毫秒数
 * @return OTA_RET_OK 接收成功
 *         OTA_RET_ERR 接收失败(参数无效或超时)
 */
int MessageQueue::Recv(uint8_t *data, uint32_t *plen, long ms)
{
    if (!data || !plen) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    std::unique_lock<std::mutex> lock(mLock);
    if (mQueue.empty()) {
        if (ms == 0) {
            mNotEmpty.wait(lock);
        } else {
            auto timeout = std::chrono::milliseconds(ms);
            if (mNotEmpty.wait_for(lock, timeout) == std::cv_status::timeout) {
                return OTA_RET_ERR;
            }
        }
    }

    if (!mQueue.empty()) {
        std::vector<uint8_t> msg = mQueue.front();
        mQueue.pop();
        memcpy(data, msg.data(), msg.size());
        *plen = msg.size();
        return OTA_RET_OK;
    }

    return OTA_RET_ERR;
}

/**
 * @brief 清空队列
 * @return OTA_RET_OK 清空成功
 *         OTA_RET_ERR 清空失败
 */
void MessageQueue::Clear()
{
    std::lock_guard<std::mutex> lock(mLock);
    std::queue<std::vector<uint8_t>>().swap(mQueue);
}