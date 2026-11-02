#ifndef __UDS_SERVICE_H__
#define __UDS_SERVICE_H__
#include <stdint.h>
#include <stdbool.h>

enum {
    UDS_SERVICE_RESULT_OK = 0,                     // 正确
    UDS_SERVICE_RESULT_ERROR_TP = -1,              // TP层出现错误
    UDS_SERVICE_RESULT_ERROR_PARAMETER = -2,       // 入参错误
    UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT = -3,  // 响应数据格式错误
    UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE = -4 // ECU负响应
} UdsServiceResult_t;

typedef enum {
    UDS_SESSION_DEFAULT = 0x01,     // 默认会话
    UDS_SESSION_PROGRAMMING = 0x02, // 编程会话
    UDS_SESSION_EXTENDED = 0x03,    // 扩展会话
    UDS_SESSION_DEFAULT_NOACK = 0x81,     // 广播默认会话
    UDS_SESSION_PROGRAMMING_NOACK = 0x82, // 广播编程会话
    UDS_SESSION_EXTENDED_NOACK = 0x83,    // 广播扩展会话
} UdsSessionSub_t;

typedef enum {
    UDS_ECURESET_HARDRESET = 0x01,                // 硬重置
    UDS_ECURESET_KEYOFFONRESET = 0x02,            // 断电重置
    UDS_ECURESET_SOFTRESET = 0x03,                // 软重置
    UDS_ECURESET_ENABLERAPIDPOWERSHUTDOWN = 0x04, // 启用快速断电
    UDS_ECURESET_DISABLERAPIDPOWERSHTDOWN = 0x05, // 禁用快速断电
} UdsEcuResetSub_t;

typedef enum {
    UDS_ROUTINECTRL_STARTROUTINE = 0x01,         //	启动例程
    UDS_ROUTINECTRL_STOPROUTINE = 0x02,          // 停止例程
    UDS_ROUTINECTRL_REQUESTROUTINERESULT = 0x03, // 请求例程结果
} UdsRoutineCtrlSub_t;

typedef enum {
    UDS_CTRLDTCSETTING_ON = 0x01,        // 打开DTC设置
    UDS_CTRLDTCSETTING_OFF = 0x02,       // 关闭DTC设置
    UDS_CTRLDTCSETTING_ON_NOACK = 0x81,  // 广播打开DTC设置
    UDS_CTRLDTCSETTING_OFF_NOACK = 0x82, // 广播关闭DTC设置
} UdsCtrlDtcSettingSub_t;

typedef void *UdsSrvHandle_t; // uds 句柄

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief 初始化UDS服务
 * @param hdl UDS服务句柄指针
 * @param funcId 功能ID
 * @return int 返回初始化结果
 *         UDS_SERVICE_RESULT_OK 成功
 *         其他值 失败
 */
int UdsSrvInit(UdsSrvHandle_t *hdl, uint32_t funcId);

/**
 * @brief 诊断会话控制请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param isNeedRes 是否需要响应 （针对多拉功能寻址需要回复增加）
 * @param subfun 子功能类型
 * @return int 返回执行结果
 */
int DiagnosticSessionControlReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, bool isNeedRes,
                                UdsSessionSub_t subfun);

/**
 * @brief ECU复位请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param subfun 复位类型
 * @return int 返回执行结果
 */
int EcuResetReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, UdsEcuResetSub_t subfun);

/**
 * @brief 清除故障码请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
* @param resId 响应CAN ID
 * @param channel CAN通道
 * @param clearFlag 清除标志
 * @return int 返回执行结果
 */
int ClearDtcReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint32_t clearFlag);

/**
 * @brief 通过标识符读取数据请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param did 数据标识符
 * @param data 数据缓冲区
 * @param len 数据长度指针
 * @return int 返回执行结果
 */
int ReadDataByIdentifierReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint32_t did,
                            uint8_t data[], uint32_t *len);

/**
 * @brief 安全访问请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param secLevel 安全等级
 * @param data 数据缓冲区
 * @param len 数据长度指针
 * @return int 返回执行结果
 */
int SecurityAccessReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t secLevel,
                      uint8_t data[], uint32_t *len);

/**
 * @brief 通信控制请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param controlType 控制类型
 * @param commnicationType 通信类型
 * @return int 返回执行结果
 */
int CommunicationControlReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t controlType,
                            uint8_t commnicationType);

/**
 * @brief 通信控制(0x28), 可指定单次等响应超时
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param controlType 控制类型
 * @param commnicationType 通信类型
 * @param resTimeoutMs 单次等响应超时(ms), 0=默认(UDSTP_RES_TIMEOUT_MS)
 * @return int 返回执行结果
 */
int CommunicationControlReqTimeout(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel,
                                   uint8_t controlType, uint8_t commnicationType, uint32_t resTimeoutMs);

/**
 * @brief 通过标识符写数据请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param did 数据标识符
 * @param data 数据缓冲区
 * @param len 数据长度
 * @return int 返回执行结果
 */
int WriteDataByIdentifierReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint32_t did,
                             uint8_t data[], uint32_t len);

/**
 * @brief 例程控制请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param subfub 子功能类型
 * @param routineId 例程ID
 * @param data 数据缓冲区
 * @param len 数据长度指针
 * @return int 返回执行结果
 */
int RoutineControlReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, UdsRoutineCtrlSub_t subfub,
                      uint32_t routineId, uint8_t data[], uint32_t *len);

/**
 * @brief 请求下载请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param dataFormatId 数据格式ID
 * @param addressAndLengthFormatId 地址和长度格式ID
 * @param memaddr 内存地址
 * @param memsize 内存大小
 * @param pMaxNumberOfBlockLength 最大块长度指针
 * @return int 返回执行结果
 */
int RequestDownloadReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t dataFormatId,
                       uint8_t addressAndLengthFormatId, const uint8_t memaddr[], const uint8_t memsize[],
                       uint32_t *pMaxNumberOfBlockLength);

/**
 * @brief 传输数据请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param blockSequenceCounter 块序列计数器
 * @param data 数据缓冲区
 * @param len 数据长度
 * @param nrc 输出否定响应码(可为NULL)
 * @return int 返回执行结果
 */
int TransferDataReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t blockSequenceCounter,
                    const uint8_t data[], uint32_t len, uint8_t *nrc);

/**
 * @brief 请求传输退出请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param data 数据缓冲区
 * @param len 数据长度指针
 * @return int 返回执行结果
 */
int RequestTransferExitReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t data[],
                           uint32_t *len);

/**
 * @brief 测试仪在线请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param suppressResponse 是否抑制响应: true=0x80(不需要响应), false=0x00(需要响应)
 * @return int 返回执行结果
 */
int TesterPresentReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, bool suppressResponse);

/**
 * @brief 控制DTC设置请求
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param subfub 子功能类型
 * @return int 返回执行结果
 */
int ControlDTCSettingReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel,
                         UdsCtrlDtcSettingSub_t subfub);

/**
 * @brief 控制DTC设置(0x85), 可指定单次等响应超时
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param subfub 子功能类型
 * @param resTimeoutMs 单次等响应超时(ms), 0=默认(UDSTP_RES_TIMEOUT_MS)
 * @return int 返回执行结果
 */
int ControlDTCSettingReqTimeout(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel,
                                UdsCtrlDtcSettingSub_t subfub, uint32_t resTimeoutMs);

/**
 * @brief 通过状态掩码读取DTC信息
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param statusMask 状态掩码
 * @param len 数据长度指针
 * @param dtcCode DTC代码缓冲区
 * @return int 返回执行结果
 */
int ReadDTCInfoByStatusMask(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t statusMask,
                            uint32_t *len, uint8_t dtcCode[]);

/**
 * @brief 读取标识符数据
 * @param hdl UDS服务句柄
 * @param canId CAN ID
 * @param resId 响应CAN ID
 * @param channel CAN通道
 * @param did 数据标识符
 * @param len 数据长度指针
 * @param didInfo 数据标识符信息缓冲区
 * @return int 返回执行结果
 */
int ReadReadDataByIdentifier(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint16_t did,
                             uint32_t *len, uint8_t didInfo[]);

/**
 * @brief 反初始化UDS服务
 * @param hdl UDS服务句柄
 * @return int 返回执行结果
 */
int UdsSrvDeinit(UdsSrvHandle_t hdl);

/**
 * @brief 清空UDS响应消息队列
 *
 * @param hdl UdsSrv句柄
 * @return int 执行结果 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int UdsSrvClearQueue(UdsSrvHandle_t hdl);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif