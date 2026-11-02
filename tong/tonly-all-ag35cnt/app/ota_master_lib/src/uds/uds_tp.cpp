#include <chrono>
#include <cstring>
#include <iostream>
#include "uds_tp.h"
#include "ota_master_app_callback.h"
#include "ota_master_error_code.h"
#include "ota_master_log.h"

/**
 * @brief 打印UDS数据，带前缀
 *
 * @param data 数据指针
 * @param dataLen 数据长度
 * @param counterIdx 当前计数索引
 * @param prefix 日志前缀
 */
static void PrintUdsDataWithPrefix(const uint8_t *data, uint32_t dataLen, uint32_t counterIdx, const char *prefix)
{
    if (!data || !dataLen) {
        return;
    }

    uint32_t len;
    char str[64] = {0x0};

    snprintf(str, sizeof(str), "%s counterIdx: %d ", prefix, counterIdx);
    len = (dataLen > 64) ? 64 : dataLen;
    OTA_LOG_HEX(data, len, str);
}

/**
 * @brief UdsTp构造函数
 */
UdsTp::UdsTp() : counterIdx(0), msgQueue(new MessageQueue()) {}

/**
 * @brief UdsTp析构函数
 */
UdsTp::~UdsTp()
{
    thCond.notify_all();
}

/**
 * @brief 初始化UdsTp
 *
 * @return int 初始化结果，成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int UdsTp::Init()
{
    if (!msgQueue) {
        OTA_LOG_E("%s CreateMsgQueue fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    msgQueue->Clear();
    
    // 重置counterIdx，确保每次OTA会话开始时与MCU端同步    
    counterIdx.store(0);
    OTA_LOG_D("%s counterIdx reset to 0.\n", __FUNCTION__);
    
    return OTA_RET_OK;
}

/**
 * @brief 发送UDS请求
 *
 * @param req UDS请求
 * @return int 发送结果，成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int UdsTp::Transmit(const UdsTpReq_t &req)
{
    int ret = OTA_RET_ERR;

    if (req.len == 0) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    OTA_LOG_D("[UdsTp::Transmit] canId: 0x%08X, resId: 0x%08X, channel: %d, len: %d",
              req.canId, req.resId, req.canChannel, req.len);

    //防止心跳和其他UDS 服务同时发送,导致ACK 解析冲突
    std::unique_lock<std::mutex> sendLock(sendMutex);
    uint8_t buffer[UDSTP_MSG_BUFFER_SIZE] = {0x0};
    uint64_t timeout_ms = UDSTP_ACK_TIMEOUT_MS;
    uint32_t sendtimes = UDSTP_ACK_TRY_TIMES;
    uint32_t repeat = 0;

    while (sendtimes-- > 0) {
        uint8_t currentCounter = counterIdx.load();
        buffer[0] = 0x0;
        buffer[1] = repeat ? 0x80 : 0x00;
        buffer[2] = currentCounter;
        buffer[3] = (req.canId >> 24) & 0xFF;
        buffer[4] = (req.canId >> 16) & 0xFF;
        buffer[5] = (req.canId >> 8) & 0xFF;
        buffer[6] = req.canId & 0xFF;
        buffer[7] = (req.resId >> 24) & 0xFF;
        buffer[8] = (req.resId >> 16) & 0xFF;
        buffer[9] = (req.resId >> 8) & 0xFF;
        buffer[10] = req.resId & 0xFF;
        buffer[11] = req.canChannel;
        memcpy(&buffer[UDSTP_MSG_HEADER_SEND_SIZE], req.data, req.len);
        PrintUdsDataWithPrefix(buffer, req.len + UDSTP_MSG_HEADER_SEND_SIZE, currentCounter,
                               "[UDS TX] Send UDS request");
        std::unique_lock<std::mutex> lock(thMutex);
        auto outtime = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_ms);
        SendUdsDataCallback(buffer, req.len + UDSTP_MSG_HEADER_SEND_SIZE);
        if (thCond.wait_until(lock, outtime) == std::cv_status::timeout) {
            OTA_LOG_E("currentCounter [%d] receive ACK timeout, retry\n", currentCounter);
        } else {
            ret = OTA_RET_OK;
            break;
        }
        repeat = 1;
    }
    return ret;
}

/**
 * @brief 执行无响应的UDS服务
 *
 * @param uData UDS数据
 * @return int 执行结果，成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int UdsTp::ServiceExecNoRes(const UdsTpData_t &uData)
{
    UdsTpReq_t req;

    req.len = uData.txLen;
    req.canId = uData.canId;
    req.resId = uData.resId;
    req.canChannel = uData.canChannel;
    memcpy(req.data, uData.tx, uData.txLen);
    return Transmit(req);
}

/**
 * @brief 执行有响应的UDS服务,无78 响应时超时时间为200ms * 3 = 600ms
 *
 * @param uData UDS数据
 * @return int 执行结果，成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int UdsTp::ServiceExec(UdsTpData_t &uData)
{
    UdsTpRes_t res;
    uint8_t times = UDSTP_RES_TRY_TIMES;
    uint32_t len;
    bool responseReceived = false;
    uint32_t resTimeout = resTimeoutMs ? resTimeoutMs : UDSTP_RES_TIMEOUT_MS; // 单次等响应超时

    if (!uData.txLen) {
        OTA_LOG_E("%s param invalid.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 保存发送请求时的 counterIdx，用于验证响应匹配
    // counterIdx 在收到ACK后会自增，所以响应的 counterIdx 应该等于发送时的值
    uint8_t expectedCounterIdx = counterIdx.load();

    if (OTA_RET_ERR == ServiceExecNoRes(uData)) {
        return OTA_RET_ERR;
    }
    memset(&res, 0x0, sizeof(UdsTpRes_t));
    while (times > 0) {
        times--;
        if (0 != msgQueue->Recv((uint8_t *)&res, &len, resTimeout)) {
            OTA_LOG_E("----%s RecvMsgQueue fail, Retry times has %d\n", __FUNCTION__, times);
            continue;
        }
        
        // 验证响应的counterIdx是否匹配当前请求        
        if (res.counterIdx != expectedCounterIdx) {
            OTA_LOG_W("----%s counterIdx mismatch: expected %d, got %d. Discarding stale response.\n",
                      __FUNCTION__, expectedCounterIdx, res.counterIdx);
            times++;  // 恢复本次重试机会,残留响应不消耗retry次数
            continue;
        }
        
        if ((res.len == 3) && (res.data[0] == 0x7f) && (res.data[2] == 0x78)) {
            int pendingtimes = UDSTP_RES_PENDING_TRY_TIMES;
            while (pendingtimes > 0) {
                pendingtimes--;
                if (0 != msgQueue->Recv((uint8_t *)&res, &len, UDSTP_RES_PENDING_MS)) {
                    OTA_LOG_E("----%s RecvMsgQueue fail.\n", __FUNCTION__);
                    return OTA_RET_ERR;
                }

                // 验证pending响应的counterIdx是否匹配
                // 说明: 7F xx 78 表示"请求已收到,请稍等", ECU处理完成后会回最终响应。
                //       部分场景(如MCU转发时counterIdx被并发请求污染)最终响应的counterIdx可能与本请求
                //       不一致; 此时只要响应的SID仍属于本请求, 就应采纳, 避免把ECU的成功响应当stale丢弃。
                if (res.counterIdx != expectedCounterIdx) {
                    bool sameRequest = false;
                    if (res.len >= 1) {
                        if (res.data[0] == (uint8_t)(uData.tx[0] + 0x40)) {
                            sameRequest = true; // 本请求的肯定响应
                        } else if ((res.data[0] == 0x7f) && (res.len >= 2) && (res.data[1] == uData.tx[0])) {
                            sameRequest = true; // 本请求的否定响应(含7F xx 78挂起)
                        }
                    }
                    if (!sameRequest) {
                        OTA_LOG_W(
                            "----%s counterIdx mismatch in pending: expected %d, got %d. Discarding stale response.\n",
                            __FUNCTION__, expectedCounterIdx, res.counterIdx);
                        pendingtimes++; // 恢复本次重试机会,残留响应不消耗retry次数
                        continue;
                    }
                    OTA_LOG_W("----%s pending: counterIdx mismatch(expected %d, got %d) but SID matches, accept.\n",
                              __FUNCTION__, expectedCounterIdx, res.counterIdx);
                }

                if ((res.len == 3) && (res.data[0] == 0x7f) && (res.data[2] == 0x78)) {
                    continue;
                }
                break;
            }

            if (0 == pendingtimes) {
                OTA_LOG_E("----%s RecvMsgQueue fail.\n", __FUNCTION__);
                return OTA_RET_ERR;
            }
        }

        memcpy(uData.rx, res.data, res.len);
        uData.rxLen = res.len;
        responseReceived = true;
        break;
    }
    
    // 检查是否成功收到响应
    if (!responseReceived) {
        OTA_LOG_E("----%s No response received after all retries.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    
    return OTA_RET_OK;
}

/**
 * @brief 获取UdsTp实例
 *
 * 该函数用于获取UdsTp实例，单例模式
 *
 * @return UdsTp* UdsTp实例指针
 */
UdsTp *UdsTp::GetInstance()
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    static UdsTp instance;
    return &instance;
}

int ReceiveMcuAck(const uint8_t *data, uint16_t dataLen)
{
    UdsTp *instance = UdsTp::GetInstance();
    uint8_t expectedCounter = instance->counterIdx.load();
    uint8_t receivedCounter = data[2];
    
    if (receivedCounter != expectedCounter) {        
        if (receivedCounter == 0 && expectedCounter > 0) {
            OTA_LOG_W("%s MCU may have restarted. Syncing counterIdx from [%x] to [0].\n",
                      __FUNCTION__, expectedCounter);
            // 同步到MCU的counterIdx
            instance->counterIdx.store(0);
            expectedCounter = 0;
        } else {
            OTA_LOG_E("%s expectedCounter:[%x], received:[%x], ACK mismatch.\n", __FUNCTION__,
                      expectedCounter, receivedCounter);
            return OTA_RET_ERR;
        }
    }
    std::unique_lock<std::mutex> lock(instance->thMutex);
    instance->counterIdx++;
    instance->thCond.notify_one();
    PrintUdsDataWithPrefix(data, dataLen, data[2], "[UDS RX] Received UDS ACK");
    return OTA_RET_OK;
}

int ReceiveUdsDataAndProcess(const uint8_t *data, uint16_t dataLen)
{
    UdsTp *instance = UdsTp::GetInstance();
    if (!instance || !instance->msgQueue) {
        OTA_LOG_E("%s invalid instance or queue.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    if (!data || (dataLen < UDSTP_MSG_HEADER_RECV_SIZE)) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    PrintUdsDataWithPrefix(data, dataLen, data[2], "[UDS RX] Received UDS response");

    UdsTpRes_t res;
    uint8_t resAck[3] = {0x0};

    resAck[2] = data[2];
    PrintUdsDataWithPrefix(resAck, sizeof(resAck), data[2], "[UDS TX] Send ACK");
    SendUdsDataAckCallback(resAck, sizeof(resAck));

    res.canId = (data[3] << 24) | (data[4] << 16) | (data[5] << 8) | data[6];
    res.counterIdx = data[2];  // 保存 counterIdx 用于匹配请求和响应
    res.len = dataLen - UDSTP_MSG_HEADER_RECV_SIZE;
    memcpy(res.data, &data[UDSTP_MSG_HEADER_RECV_SIZE], res.len);
    if (instance->msgQueue->Send((uint8_t *)&res, sizeof(UdsTpRes_t)) != 0) {
        OTA_LOG_E("%s failed to send message.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    return OTA_RET_OK;
}

int UdsTpInit(UdsTpHandle_t *tpHdl)
{
    if (!tpHdl) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    UdsTp *priv = UdsTp::GetInstance();
    if (priv->Init() != OTA_RET_OK) {
        return OTA_RET_ERR;
    }

    // 每次OTA会话开始时重置counterIdx，确保与MCU端同步    
    priv->counterIdx.store(0);
    OTA_LOG_D("%s counterIdx reset to 0 for new OTA session.\n", __FUNCTION__);

    *tpHdl = priv;
    return OTA_RET_OK;
}

int UdsTpSeviceExec(UdsTpHandle_t tpHdl, UdsTpData_t &uData)
{
    UdsTp *priv = static_cast<UdsTp *>(tpHdl);
    return priv->ServiceExec(uData);
}

int UdsTpSeviceExecNoRes(UdsTpHandle_t tpHdl, const UdsTpData_t &uData)
{
    UdsTp *priv = static_cast<UdsTp *>(tpHdl);
    return priv->ServiceExecNoRes(uData);
}

void UdsTpSetResTimeout(UdsTpHandle_t tpHdl, uint32_t ms)
{
    UdsTp *priv = static_cast<UdsTp *>(tpHdl);
    if (!priv) {
        return;
    }
    priv->SetResTimeoutMs(ms);
}

int UdsTpDeinit(UdsTpHandle_t tpHdl)
{
    return OTA_RET_OK;
}

int UdsTpClearQueue(UdsTpHandle_t tpHdl)
{
    UdsTp *priv = static_cast<UdsTp *>(tpHdl);
    if (!priv || !priv->msgQueue) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    
    priv->msgQueue->Clear();
    OTA_LOG_D("%s message queue cleared.\n", __FUNCTION__);
    return OTA_RET_OK;
}
