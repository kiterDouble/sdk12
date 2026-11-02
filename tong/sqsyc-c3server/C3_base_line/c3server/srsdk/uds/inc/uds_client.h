#ifndef __UDS_CLT_H__
#define __UDS_CLT_H__
#include "uds_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UDS_CLIENT_DEFAULT_P2_MS
#define UDS_CLIENT_DEFAULT_P2_MS (150U)
#endif

#ifndef UDS_CLIENT_DEFAULT_P2_STAR_MS
#define UDS_CLIENT_DEFAULT_P2_STAR_MS (1500U)
#endif

_Static_assert(UDS_CLIENT_DEFAULT_P2_STAR_MS > UDS_CLIENT_DEFAULT_P2_MS, "");


typedef enum
{
    E_UDS_SEQ_STATE_DONE = 0,//结束
    E_UDS_SEQ_STATE_RUNNING = 1,//进行中
    E_UDS_SEQ_STATE_GOTONEXT = 2,//下一步
} UDSSeqState_e;

typedef uint32_t UDSSeqState_t;
typedef enum
{
    E_UDS_CLT_REQUEST_STATE_IDLE = 0x0,//kRequestStateIdle 完成
    E_UDS_CLT_REQUEST_STATE_SENDING,           //kRequestStateSending 传输层现在传输数据
    E_UDS_CLT_REQUEST_STATE_AWAIT_SEND_COMPLETE, //kRequestStateAwaitSendComplete 等待传输发送完成
    E_UDS_CLT_REQUEST_STATE_AWAIT_RESPONSE,     //kRequestStateAwaitResponse 等待响应
    E_UDS_CLT_REQUEST_STATE_PROCESS_RESPONSE,   //kRequestStateProcessResponse 处理响应
}UDSClientRequestState_e;

typedef uint8_t UDSClientRequestState_t;

typedef struct
{
    UDSTpHandle_t *tp;
} UDSClientConfig_t;

typedef enum
{
    E_UDS_CLT_SUPPRESS_POS_RESP = 0x1,     // 服务器不应该发送肯定响应
    E_UDS_CLT_FUNCTIONAL = 0x2,            // 发功能请求
    E_UDS_CLT_NEG_RESP_IS_ERR = 0x4,       // 否定响应是属于故障
    E_UDS_CLT_IGNORE_SERVER_TIMINGS = 0x8, // 忽略服务器给的p2和p2_star
}UDSClientOptions_e;

struct UDSClient;

typedef UDSSeqState_t (*UDSClientCallback)(struct UDSClient *client);

typedef struct UDSClient
{
    uint16_t p2Ms;//p2_ms;      // p2 超时时间
    uint32_t p2StarMs;//p2_star_ms; // 0x78 p2* 超时时间
    UDSTpHandle_t *tp;

    // 内状态
    uint32_t peTimer;//p2_timer;
    uint8_t recvBuf[UDS_BUFSIZE_MAX];//recv_buf[UDS_BUFSIZE_MAX];
    uint8_t sendBuf[UDS_BUFSIZE_MAX];
    uint16_t recvBufSize;//recv_buf_size;
    uint16_t sendBufSize;//send_buf_size;
    uint16_t recvSize;//recv_size;
    uint16_t sendSize;//send_size;
    UDSErr_t err;
    UDSClientRequestState_t state;

    uint8_t options;        // enum udsclientoptions
    uint8_t defaultOptions; // enum udsclientoptions
    // a copy of the options at the time a request is made
    uint8_t optionsCopy;//_options_copy; // enum udsclientoptions

    const UDSClientCallback *cbList; // null-terminated list of callback functions
    size_t cbIdx;                    // index of currently active callback function
    void *cbData;                    // a pointer to data available to callbacks

}UDSClient_t;

typedef struct
{
    uint8_t securityAccessType;
    const uint8_t *securitySeed;
    uint16_t securitySeedLength;
}SecurityAccessResponse_t;

typedef struct
{
    size_t maxNumberOfBlockLength;
}RequestDownloadResponse_t;

typedef struct
{
    uint8_t routineControlType;
    uint16_t routineIdentifier;
    const uint8_t *routineStatusRecord;
    uint16_t routineStatusRecordLength;
}RoutineControlResponse_t;

/**
 * @name UDSClientInit
 * @brief UDS client初始化函数
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param cfg 配置参数
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSClientInit(UDSClient_t *client, const UDSClientConfig_t *cfg);
/**
 * @name UDSClientInit
 * @brief UDS client反初始化函数
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSClientDeInit(const UDSClient_t *client);

#define UDS_CLIENT_IDLE (0)
#define UDS_CLIENT_RUNNING (1)

/**
 * name UDSClientPoll
 * @brief client轮询函数
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @return 空闲返回UDS_CLIENT_IDLE, 其他返回UDS_CLIENT_RUNNING
 */
bool UDSClientPoll(UDSClient_t *client);

/**
 * @name UDSSendECUReset
 * @brief 0x11数据组包发送
 * @param client  UDSClient_t结构体
 * @param type 重启类型
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendECUReset(UDSClient_t *client, uint8_t type);

/**
 * @name UDSSendDiagSessCtrl
 * @brief 0x10数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param mode 会话模式
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendDiagSessCtrl(UDSClient_t *client, UDSDiagSessionType_e mode);

/**
 * @name UDSSendClearDtcInfo
 * @brief 0x14数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param groupOfDtc 要清除的DTC组或特定DTC
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendClearDtcInfo(UDSClient_t *client, uint32_t groupOfDtc);

/**
 * @name UDSSendSecurityAccess
 * @brief 0x27数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param level 安全级别
 * @param data 安全seed或key
 * @param size data长度
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendSecurityAccess(UDSClient_t *client, uint8_t level, uint8_t *data, uint16_t size);

/**
 * @name UDSSendCommCtrl
 * @brief 0x28数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param ctrl 通讯控制类型
 * @param comm 通讯类型
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendCommCtrl(UDSClient_t *client, UDSCommunicationControlType_e ctrl,
                        UDSCommunicationType_e comm);

/**
 * @name UDSSendRDBI
 * @brief 0x22数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param didList did列表
 * @param numDataIdentifiers did列表条目
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendRDBI(UDSClient_t *client, const uint16_t *didList,
                     const uint16_t numDataIdentifiers);

/**
 * @name UDSSendWDBI
 * @brief 0x2e数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param dataIdentifier did数据标识
 * @param data 写入数据
 * @param size 写入数据长度
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendWDBI(UDSClient_t *client, uint16_t dataIdentifier, const uint8_t *data,
                     uint16_t size);

/**
 * @name UDSSendTesterPresent
 * @brief 0x3e数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param sub 子功能
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendTesterPresent(UDSClient_t *client, uint8_t sub);

/**
 * @name UDSSendRoutineCtrl
 * @brief 0x31数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param type 控制类型
 * @param routineIdentifier routine标识
 * @param data routine数据
 * @param size routine数据长度
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendRoutineCtrl(UDSClient_t *client, RoutineControlType_e type,
                            uint16_t routineIdentifier, const uint8_t *data, uint16_t size);

/**
 * @name UDSSendRequestDownload
 * @brief 0x34数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param dataFormatIdentifier 数据格式标识
 * @param addressAndLengthFormatIdentifier 每半个字节单独编码
 * @param memoryAddress 内存地址的起始地址
 * @param memorySize 内存大小
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendRequestDownload(UDSClient_t *client, uint8_t dataFormatIdentifier,
                                uint8_t addressAndLengthFormatIdentifier, size_t memoryAddress,
                                size_t memorySize);

/**
 * @name UDSSendRequestUpload
 * @brief 0x35数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param dataFormatIdentifier 数据格式标识
 * @param addressAndLengthFormatIdentifier 每半个字节单独编码
 * @param memoryAddress 内存地址的起始地址
 * @param memorySize 内存大小
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendRequestUpload(UDSClient_t *client, uint8_t dataFormatIdentifier,
                              uint8_t addressAndLengthFormatIdentifier, size_t memoryAddress,
                              size_t memorySize);

/**
 * @name UDSSendTransferData
 * @brief 0x36数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param blockSequenceCounter 发送块计数
 * @param blockLength 块长度
 * @param data 发送数据指针
 * @param size 发送数据长度
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendTransferData(UDSClient_t *client, uint8_t blockSequenceCounter,
                             const uint16_t blockLength, const uint8_t *data, uint16_t size);

/**
 * @name UDSSendTransferDataStream
 * @brief 0x36数据流组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param blockSequenceCounter 发送块计数
 * @param blockLength 块长度
 * @param fd 文件指针
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendTransferDataStream(UDSClient_t *client, uint8_t blockSequenceCounter,
                                   const uint16_t blockLength, FILE *fd);

/**
 * @name UDSSendRequestTransferExit
 * @brief 0x37组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param data transferRequestParameterRecord数据
 * @param size transferRequestParameterRecord数据长度
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendRequestTransferExit(UDSClient_t *client, const uint8_t *data, uint16_t size);

/**
 * @name UDSCtrlDTCSetting
 * @brief 0x85组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param dtcSettingType：子功能
 * @param data：故障码设置控制可选记录
 * @param len:可选记录条目
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSCtrlDTCSetting(UDSClient_t *client, DTCSettingType_e dtcSettingType,
                           uint8_t *data, uint16_t size);

/**
 * @name UDSUnpackRDBIResponse
 * @brief 解包0x22响应
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param did did
 * @param data 数据缓存指针
 * @param size 数据缓存大小
 * @param offset 数据偏移
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSUnpackRDBIResponse(const UDSClient_t *client, uint16_t did, uint8_t *data,
                               uint16_t size, uint16_t *offset);

/**
 * @name UDSUnpackSecurityAccessResponse
 * @brief 解包0x27响应
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param resp 0x27响应
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSUnpackSecurityAccessResponse(const UDSClient_t *client,
                                        SecurityAccessResponse_t *resp);

/**
 * @name UDSUnpackRequestDownloadResponse
 * @brief 解包0x34响应
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param resp 0x34响应
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSUnpackRequestDownloadResponse(const UDSClient_t *client,
                                        RequestDownloadResponse_t *resp);

/**
 * @name UDSUnpackRoutineControlResponse
 * @brief 解包0x31响应
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param resp 0x31响应
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSUnpackRoutineControlResponse(const UDSClient_t *client,
                                        RoutineControlResponse_t *resp);

/**
 * @name UDSClientAwaitIdle
 * @brief 请求发送后等待响应
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @return UDSSeqState_e定义的枚举值
 */
UDSSeqState_t UDSClientAwaitIdle(UDSClient_t *client);

/**
 * @name UDSConfigDownload
 * @brief 下载请求配置
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param dataFormatIdentifier 数据格式标识
 * @param addressAndLengthFormatIdentifier 每半个字节单独编码
 * @param memoryAddress 内存地址的起始地址
 * @param memorySize 内存大小
 * @param fd 文件fd
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSConfigDownload(UDSClient_t *client, uint8_t dataFormatIdentifier,
                           uint8_t addressAndLengthFormatIdentifier, size_t memoryAddress,
                           size_t memorySize, FILE *fd);

/**
 * @name UDSKeepSession
 * @brief 保持会话
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSKeepSession(UDSClient_t *client);
#ifdef __cplusplus
}
#endif
#endif