#ifndef __UDS_TP_H__
#define __UDS_TP_H__
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include "msg_queue.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief UDS MCU-MPU传输协议子命令
 */
enum UdsTpSubCmd {
    UDSTP_SUB_CMD_MPU_TO_MCU = 0x01,     // MPU->MCU
    UDSTP_SUB_CMD_MPU_TO_MCU_ACK = 0x02, // MCU回复MPU->MCU的ACK
    UDSTP_SUB_CMD_MCU_TO_MPU = 0x03,     // MCU->MPU
    UDSTP_SUB_CMD_MCU_TO_MPU_ACK = 0x04  // MPU回复MCU->MPU的ACK
};

/**
 * @brief UDSTP 使用参数
 */
enum UdsTpSubCmdDefinitions {
    UDSTP_DATA_BUFFER_SIZE = 4096,      // UDS会话接收和发送最大长度
    UDSTP_ACK_TRY_TIMES = 3,            // MPU消息发出后 ，在超时时间内未收到ACK计一次，最多三次
    UDSTP_RES_TRY_TIMES = 3,            // MPU消息发出后 ，在超时时间内未收到RES计一次，最多三次
    UDSTP_ACK_TIMEOUT_MS = 2000,        // MPU消息发出接收ACK超时时间，从mcu收到回复ACK，改为MCU发出到CAN上再回ACK ,由200->3000
    UDSTP_RES_TIMEOUT_MS = 400,         // MPU消息发出接收RES超时时间(单次), 总等待 = 该值 × UDSTP_RES_TRY_TIMES
    UDSTP_RES_PENDING_TRY_TIMES = 5000, // NRC 为78 时最大重试接收次数,由于IPK依赖性校验78太多，由100->5000
    UDSTP_RES_PENDING_MS = 5000,        // NRC 为78 时单次重试接收超时时间
    UDSTP_MSG_BUFFER_SIZE = 5000,       // MCU与MPU交互接收和发送最大长度
    UDSTP_MSG_HEADER_SEND_SIZE = 12,    // UDS消息发送头长度
    UDSTP_MSG_HEADER_RECV_SIZE = 7      // UDS消息接收头长度
};

typedef struct {
    uint32_t canId;                     // ECU 对应CAN ID (ECU CANID 或者 功能ID)
    uint32_t resId;                     // ECU 回复响应的CAN ID
    uint8_t canChannel;                 // CAN 通道
    uint32_t txLen;                     // 发送长度
    uint32_t rxLen;                     // 接收长度
    uint8_t tx[UDSTP_DATA_BUFFER_SIZE]; // 发送缓冲区
    uint8_t rx[UDSTP_DATA_BUFFER_SIZE]; // 接收缓冲区
} UdsTpData_t;

typedef struct {
    uint32_t canId;                       // CAN ID
    uint32_t resId;                       // ECU 回复响应的CAN ID
    uint8_t canChannel;                   // CAN 通道
    uint8_t counterIdx;                   // 计数器索引，用于匹配请求和响应
    uint32_t len;                         // 数据长度
    uint8_t data[UDSTP_DATA_BUFFER_SIZE]; // 数据缓冲区
} UdsTpRes_t, UdsTpReq_t;

typedef void *UdsTpHandle_t;

class UdsTp {
    friend int ReceiveUdsDataAndProcess(const uint8_t *data, uint16_t dataLen);
    friend int ReceiveMcuAck(const uint8_t *data, uint16_t dataLen);
    friend int UdsTpClearQueue(UdsTpHandle_t tpHdl);
    friend int UdsTpInit(UdsTpHandle_t *tpHdl);

public:
    UdsTp();
    ~UdsTp();
    static UdsTp *GetInstance();
    int Init();
    int ServiceExec(UdsTpData_t &uData);
    int ServiceExecNoRes(const UdsTpData_t &uData);
    void SetResTimeoutMs(uint32_t ms) { resTimeoutMs = ms; }

private:
    std::mutex sendMutex;           // 发送互斥锁
    std::mutex thMutex;             // thCond 的互斥锁
    std::condition_variable thCond; // UDS发送等待ACK时信号量
    MessageQueue* msgQueue;         // msgQueue成员变量，指针类型
    std::atomic<uint8_t> counterIdx;// MPU-MCU counterIdx, 0~255, MPU做client端，MCU不会主动请求，所以自增在MPU
    uint32_t resTimeoutMs = 0;      // 单次等响应超时(ms); 0=使用默认 UDSTP_RES_TIMEOUT_MS
    int Transmit(const UdsTpReq_t &req);
};

/**
 * @brief 初始化UdsTp对象
 *
 * 该函数用于创建并初始化一个UdsTp对象，并将其指针存储在传入的句柄指针中。
 * 如果传入的句柄指针为空，或者UdsTp对象初始化失败，函数将返回错误码。
 *
 * @param tpHdl 指向UdsTp句柄的指针，用于存储新创建的UdsTp对象的指针
 * @return int 初始化结果，成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int UdsTpInit(UdsTpHandle_t *tpHdl);
/**
 * @brief 执行UDS服务
 *
 * @param tpHdl UdsTp句柄
 * @param uData UdsTp数据
 * @return int 执行结果 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int UdsTpSeviceExec(UdsTpHandle_t tpHdl, UdsTpData_t &uData);
/**
 * @brief 清空UDS响应消息队列
 *
 * @param tpHdl UdsTp句柄
 * @return int 执行结果 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int UdsTpClearQueue(UdsTpHandle_t tpHdl);
/**
 * @brief 执行无响应的UDS服务
 *
 * @param tpHdl UdsTp句柄
 * @param uData UdsTp数据
 * @return int 执行结果 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int UdsTpSeviceExecNoRes(UdsTpHandle_t tpHdl, const UdsTpData_t &uData);
/**
 * @brief 设置单次等响应超时(ms), 0=使用默认值(UDSTP_RES_TIMEOUT_MS)
 *
 * @param tpHdl UdsTp句柄
 * @param ms 超时毫秒数, 0=默认
 */
void UdsTpSetResTimeout(UdsTpHandle_t tpHdl, uint32_t ms);
/**
 * @brief 反初始化UdsTp
 *
 * @param tpHdl UdsTp句柄
 * @return int 反初始化结果 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int UdsTpDeinit(UdsTpHandle_t tpHdl);
/**
 * @brief 接收UDS数据并进行处理
 *
 * 该函数用于接收来自MCU的消息，如果接收到的是MCU到MPU的消息，则将消息解析并发送到消息队列中。
 *
 * @param data 接收到的数据指针
 * @param dataLen 接收到的数据长度
 * @return int 处理结果，成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int ReceiveUdsDataAndProcess(const uint8_t *data, uint16_t dataLen);
/**
 * @brief 接收MCU的确认消息
 *
 * 该函数用于接收来自MCU的消息，如果接收到的是确认消息，则通知等待的线程，调用时需使用先GetInstance获取实例。
 *
 * @param data 接收到的数据指针
 * @param dataLen 接收到的数据长度
 * @return int 处理结果，成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int ReceiveMcuAck(const uint8_t *data, uint16_t dataLen);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif