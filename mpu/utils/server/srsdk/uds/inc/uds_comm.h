#ifndef __UDS_COMM_H__
#define __UDS_COMM_H__
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "err_code.h"
#include "sr_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UDS_TP_MTU
#define UDS_TP_MTU (4095U)
#endif
#define UDS_BUFSIZE_MAX UDS_TP_MTU


#define UDS_MAX_DIAGNOSTIC_SERVICES 0x7F

#define UDS_RESPONSE_SID_OF(request_sid) (request_sid + 0x40)
#define UDS_REQUEST_SID_OF(response_sid) (response_sid - 0x40)

#define UDS_NEG_RESP_LEN 3U
#define UDS_0X10_REQ_LEN 2U
#define UDS_0X10_RESP_LEN 6U
#define UDS_0X11_REQ_MIN_LEN 2U
#define UDS_0X11_RESP_BASE_LEN 2U
#define UDS_0X14_REQ_MIN_LEN 4U
#define UDS_0X14_RESP_BASE_LEN 1U
#define UDS_0X22_RESP_BASE_LEN 1U
#define UDS_0X27_REQ_BASE_LEN 2U
#define UDS_0X27_RESP_BASE_LEN 2U
#define UDS_0X28_REQ_BASE_LEN 3U
#define UDS_0X28_RESP_LEN 2U
#define UDS_0X2E_REQ_BASE_LEN 3U
#define UDS_0X2E_REQ_MIN_LEN 4U
#define UDS_0X2E_RESP_LEN 3U
#define UDS_0X31_REQ_MIN_LEN 4U
#define UDS_0X31_RESP_MIN_LEN 4U
#define UDS_0X34_REQ_BASE_LEN 3U
#define UDS_0X34_RESP_BASE_LEN 2U
#define UDS_0X35_REQ_BASE_LEN 3U
#define UDS_0X35_RESP_BASE_LEN 2U
#define UDS_0X36_REQ_BASE_LEN 2U
#define UDS_0X36_RESP_BASE_LEN 2U
#define UDS_0X37_REQ_BASE_LEN 1U
#define UDS_0X37_RESP_BASE_LEN 1U
#define UDS_0X3E_REQ_MIN_LEN 2U
#define UDS_0X3E_RESP_LEN 2U
#define UDS_0X85_REQ_BASE_LEN 2U
#define UDS_0X85_RESP_LEN 2U


typedef enum
{
    E_UDS_ERR = RET_ERR,                 // 通用错误
    E_UDS_OK = RET_OK,                   // 成功
    E_UDS_ERR_TIMEOUT = RET_ERR_UDS,              // 请求超时
    E_UDS_ERR_NEG_RESP = RET_ERR_UDS - 0x01,             // 否定响应
    E_UDS_ERR_DID_MISMATCH = RET_ERR_UDS - 0x02,         // 响应DID对不上期待的DID
    E_UDS_ERR_SID_MISMATCH = RET_ERR_UDS - 0x03,         // 请求和响应SID对不上
    E_UDS_ERR_SUBFUNCTION_MISMATCH = RET_ERR_UDS - 0x04, // 请求和响应SubFunction对不上
    E_UDS_ERR_TPORT = RET_ERR_UDS - 0x05,                // 传输层错误
    E_UDS_ERR_FILE_IO = RET_ERR_UDS - 0x06,              // 文件IO错误
    E_UDS_ERR_RESP_TOO_SHORT = RET_ERR_UDS - 0x07,       // 响应太短
    E_UDS_ERR_BUFSIZ = RET_ERR_UDS - 0x08,               // 缓冲器不够大
    E_UDS_ERR_INVALID_ARG = RET_ERR_UDS - 0x09,          // 参数不对、没发
    E_UDS_ERR_BUSY = RET_ERR_UDS - 0x0A,                 // 正在忙、没发
} UDSErr_e;

typedef enum
{
    E_UDS_SID_DIAGNOSTIC_SESSION_CONTROL = 0x10,
    E_UDS_SID_ECU_RESET = 0x11,
    E_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION = 0x14,
    E_UDS_SID_READ_DTC_INFORMATION = 0x19,
    E_UDS_SID_READ_DATA_BY_IDENTIFIER = 0x22,
    E_UDS_SID_READ_MEMORY_BY_ADDRESS = 0x23,
    E_UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER = 0x24,
    E_UDS_SID_SECURITY_ACCESS = 0x27,
    E_UDS_SID_COMMUNICATION_CONTROL = 0x28,
    E_UDS_SID_READ_PERIODIC_DATA_BY_IDENTIFIER = 0x2A,
    E_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER = 0x2C,
    E_UDS_SID_WRITE_DATA_BY_IDENTIFIER = 0x2E,
    E_UDS_SID_INPUT_CONTROL_BY_IDENTIFIER = 0x2F,
    E_UDS_SID_ROUTINE_CONTROL = 0x31,
    E_UDS_SID_REQUEST_DOWNLOAD = 0x34,
    E_UDS_SID_REQUEST_UPLOAD = 0x35,
    E_UDS_SID_TRANSFER_DATA = 0x36,
    E_UDS_SID_REQUEST_TRANSFER_EXIT = 0x37,
    E_UDS_SID_REQUEST_FILE_TRANSFER = 0x38,
    E_UDS_SID_WRITE_MEMORY_BY_ADDRESS = 0x3D,
    E_UDS_SID_TESTER_PRESENT = 0x3E,
    E_UDS_SID_ACCESS_TIMING_PARAMETER = 0x83,
    E_UDS_SID_SECURED_DATA_TRANSMISSION = 0x84,
    E_UDS_SID_CONTROL_DTC_SETTING = 0x85,
    E_UDS_SID_RESPONSE_ON_EVENT = 0x86,
}UDSDiagnosticServiceId_e;

typedef enum
{
    E_UDS_SESSION_TYPE_DEFAULT = 0x01,//kDefaultSession
    E_UDS_SESSION_TYPE_PROGRAMMING = 0x02,//kProgrammingSession
    E_UDS_SESSION_TYPE_EXTENDED = 0x03,//kExtendedDiagnostic
    E_UDS_SESSION_TYPE_SAFETYSYSTEM = 0x04,//kSafetySystemDiagnostic
}UDSDiagSessionType_e;

typedef enum
{
    E_UDS_SRV_RESP_CODE_POSITIVE = 0,//kPositiveResponse
    E_UDS_SRV_RESP_CODE_GENERAL_REJECT = 0x10,//kGeneralReject
    E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED = 0x11,//kServiceNotSupported
    E_UDS_SRV_RESP_CODE_SUBFUN_NOT_SUPPORTED = 0x12,//kSubFunctionNotSupported
    E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT = 0x13,//kIncorrectMessageLengthOrInvalidFormat
    E_UDS_SRV_RESP_CODE_RESPONSE_TOO_LONG = 0x14,//kResponseTooLong
    E_UDS_SRV_RESP_CODE_BUSY_REPEAT_REQUEST = 0x21,//kBusyRepeatRequest
    E_UDS_SRV_RESP_CODE_CONDITIONS_NOT_CORRECT = 0x22,//kConditionsNotCorrect
    E_UDS_SRV_RESP_CODE_SEQUENCE_ERROR = 0x24,//kRequestSequenceError
    E_UDS_SRV_RESP_CODE_RESPONESE_FROM_SUBNET_COMPONENT = 0x25,//kNoResponseFromSubnetComponent
    E_UDS_SRV_RESP_CODE_FAILURE_PREVENTS_EXECUTION_OF_REQUESTED_ACTION = 0x26,//kFailurePreventsExecutionOfRequestedAction
    E_UDS_SRV_RESP_CODE_REQUEST_OUT_OF_RANGE = 0x31,//kRequestOutOfRange
    E_UDS_SRV_RESP_CODE_SECURITY_ACCESS_DENIED = 0x33,//kSecurityAccessDenied
    E_UDS_SRV_RESP_CODE_INVALID_KEY = 0x35,//kInvalidKey
    E_UDS_SRV_RESP_CODE_EXCEED_NUMBER_OF_ATTEMPTS = 0x36,//kExceedNumberOfAttempts
    E_UDS_SRV_RESP_CODE_REQUIRED_TIME_DELAY_NOT_EXPIRED = 0x37,//kRequiredTimeDelayNotExpired
    E_UDS_SRV_RESP_CODE_UPLOAD_DOWNLOAD_NOT_ACCEPTED = 0x70,//kUploadDownloadNotAccepted
    E_UDS_SRV_RESP_CODE_TRANSFER_DATA_SUSPENDED = 0x71,//kTransferDataSuspended
    E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE = 0x72,//kGeneralProgrammingFailure
    E_UDS_SRV_RESP_CODE_WRONG_BLOCK_SEQUENCE_COUNTER = 0x73,//kWrongBlockSequenceCounter
    E_UDS_SRV_RESP_CODE_REQUEST_COLLECTLY_RECEIVED_RESP_PENDING = 0x78,//kRequestCorrectlyReceived_ResponsePending
    E_UDS_SRV_RESP_CODE_SUBFUN_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7E,//kSubFunctionNotSupportedInActiveSession
    E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7F,//kServiceNotSupportedInActiveSession
    E_UDS_SRV_RESP_CODE_RPM_TOO_HIGH = 0x81,//kRpmTooHigh
    E_UDS_SRV_RESP_CODE_RPM_TOO_LOW = 0x82,//kRpmTooLow
    E_UDS_SRV_RESP_CODE_ENGINE_IS_RUNNING = 0x83,//kEngineIsRunning
    E_UDS_SRV_RESP_CODE_ENGINE_IS_NOT_RUNNING = 0x84,//kEngineIsNotRunning
    E_UDS_SRV_RESP_CODE_ENGINE_RUN_TIME_TOO_LOW = 0x85,//kEngineRunTimeTooLow
    E_UDS_SRV_RESP_CODE_TEMPERATURE_TOO_HIGH = 0x86,//kTemperatureTooHigh
    E_UDS_SRV_RESP_CODE_TEMPERATURE_TOO_LOW = 0x87,//kTemperatureTooLow
    E_UDS_SRV_RESP_CODE_VEHICLE_SPEED_TOO_HIGH = 0x88,//kVehicleSpeedTooHigh
    E_UDS_SRV_RESP_CODE_VEHICLE_SPEED_TOO_LOW = 0x89,//kVehicleSpeedTooLow
    E_UDS_SRV_RESP_CODE_THROTTLE_PEDAL_TOO_HIGH = 0x8A,//kThrottlePedalTooHigh
    E_UDS_SRV_RESP_CODE_THROTTLE_PEDAL_TOO_LOW = 0x8B,//kThrottlePedalTooLow
    E_UDS_SRV_RESP_CODE_TRANSMISSTION_RANGE_NOT_IN_NEUTRAL = 0x8C,//kTransmissionRangeNotInNeutral
    E_UDS_SRV_RESP_CODE_TRANSMISSTION_RANGE_NOT_IN_GEAR = 0x8D,//kTransmissionRangeNotInGear
    E_UDS_SRV_RESP_CODE_ISO_SAE_RESERVED = 0x8E,//kISOSAEReserved
    E_UDS_SRV_RESP_CODE_BRAKE_SWITCH_NOT_CLOSED = 0x8F,//kBrakeSwitchNotClosed
    E_UDS_SRV_RESP_CODE_SHIFTER_LEVELR_NOT_IN_PARK = 0x90,//kShifterLeverNotInPark
    E_UDS_SRV_RESP_CODE_TORQUE_CONVERTER_CLUTCH_LOACKED = 0x91,//kTorqueConverterClutchLocked
    E_UDS_SRV_RESP_CODE_VOLTAGE_TOO_HIGH = 0x92,//kVoltageTooHigh
    E_UDS_SRV_RESP_CODE_VOLTAGE_TOO_LOW = 0x93//kVoltageTooLow
}UDSSrvRespCode_e;

/**
 * @brief LEV_RT_
 * @addtogroup ecuReset_0x11
 */
typedef enum
{
    E_UDS_ECU_RESET_TYPE_HARD_RESET = 0x01,//kHardReset
    E_UDS_ECU_RESET_TYPE_KEY_OFF_ON_RESET = 0x02,//kKeyOffOnReset
    E_UDS_ECU_RESET_TYPE_SOFT_RESET = 0x03,//kSoftReset
    E_UDS_ECU_RESET_TYPE_ENABLE_RAPID_POWER_SHUT_DOWN = 0x04,//kEnableRapidPowerShutDown
    E_UDS_ECU_RESET_TYPE_DISABLE_RAPID_POWER_SHUT_DOWN = 0x05,//kDisableRapidPowerShutDown
}UDSECUResetType_e;

/**
 * @addtogroup securityAccess_0x27
 */
typedef enum
{
    E_UDS_SECURITY_ACCESS_TYPE_REQUEST_SEED = 0x01,//kRequestSeed
    E_UDS_SECURITY_ACCESS_TYPE_SEND_KEY = 0x02,//kSendKey
}UDSSecurityAccessType_e;

/**
 * @addtogroup communicationControl_0x28
 */
typedef enum
{
    E_UDS_COMM_CTL_TYPE_ENABLE_RX_AND_TX = 0x00,//kEnableRxAndTx
    E_UDS_COMM_CTL_TYPE_ENABLE_RX_AND_DISABLE_TX = 0x01,//kEnableRxAndDisableTx
    E_UDS_COMM_CTL_TYPE_DISABLE_RX_AND_ENABLE_TX = 0x02,//kDisableRxAndEnableTx
    E_UDS_COMM_CTL_TYPE_DISABLE_RX_AND_TX = 0x03,//kDisableRxAndTx
}UDSCommunicationControlType_e;

/**
 * @addtogroup communicationControl_0x28
 */
typedef enum
{
    E_UDS_COMM_TYPE_NORMAL_COMM_MESSAGES = 0x01,//kNormalCommunicationMessages
    E_UDS_COMM_TYPE_NETWORK_MANAGEMENT_COMM_MESSAGES = 0x2,//kNetworkManagementCommunicationMessages
    E_UDS_COMM_TYPE_NETWORK_MANAGEMENT_AND_NORMAL_COMM_MESSAGES = 0x3,//kNetworkManagementCommunicationMessagesAndNormalCommunicationMessages
}UDSCommunicationType_e;

/**
 * @addtogroup routineControl_0x31
 */
typedef enum
{
    E_UDS_ROUTINE_CONTROL_TYPE_START = 0x01,//kStartRoutine
    E_UDS_ROUTINE_CONTROL_TYPE_STOP = 0x02,//kStopRoutine
    E_UDS_ROUTINE_CONTROL_TYPE_REQUEST_RESULTS = 0x03,//kRequestRoutineResults
}RoutineControlType_e;

/**
 * @addtogroup controlDTCSetting_0x85
 */
typedef enum
{
    E_UDS_DTC_SETTING_ON = 0x01,//kDTCSettingON
    E_UDS_DTC_SETTING_OFF = 0x02,//kDTCSettingOFF
}DTCSettingType_e;

typedef enum
{
    E_UDS_TP_ADDR_TYPE_PHYSICAL = 0x01,//kTpAddrTypePhysical
    E_UDS_TP_ADDR_TYPE_FUNCTION = 0x02,//kTpAddrTypeFunctional
}UDSTpAddr_e;

typedef enum
{
    E_UDS_TP_IDLE = 0x00000000,
    E_UDS_TP_SEND_INPROGRESS = 0x00000001,
}UDSTpStatusFlags_e;

typedef int32_t UDSErr_t;
/**
 * @brief 传输层句柄
 */
typedef struct UDSTpHandle
{
    /**
     * @brief 接收
     * @param hdl: pointer to transport handle
     * @param buf: if data is available, it will be copied here
     * @param count: the implementation should not copy more than count bytes into buf
     * @param ta_type: the addressing type of the data received or error encountered is written to
     * this location. If no data is received, nothing is written
     * @return 接收了多少个字节。
     * < 0 故障
     * == 0 没有数据
     * > 0 接收了、返回值是大小
     */
    ssize_t (*recv)(struct UDSTpHandle *hdl, uint8_t *buf, size_t count, uint32_t *tpAddrType);
    /**
     * @brief 发送
     * @param hdl: pointer to transport handle
     * @param buf: pointer to data to be sent
     * @param count: number of bytes to be sent
     * @param ta_type: the addressing type to use
     * @return 发送了多少个字节。
     * < 0 故障
     * >= 0 发送成功了
     */
    ssize_t (*send)(struct UDSTpHandle *hdl, const uint8_t *buf, size_t count, uint32_t tpAddrType);
    /**
     * @brief 轮询
     */
    uint32_t (*poll)(struct UDSTpHandle *hdl);
    void *impl; // opaque pointer to transport implementation
} UDSTpHandle_t;

/**
 * @name UDSTimeAfter
 * @brief 判断时间a是否在时间b之前
 * @param a 时间点a
 * @param b 时间点b
 * @return 如果时间a在时间b之前，则返回1；否则返回0。
 */
static inline int32_t UDSTimeAfter(uint32_t a, uint32_t b)
{
    return ((int32_t)((int32_t)(b) - (int32_t)(a)) < 0);
}

/**
 * @name UDSMillis
 * @brief 获取当前毫秒数(使用单调时钟)。
 * @param null
 * @return 返回当前时间的毫秒表示。
 */
uint32_t UDSMillis(void);

/**
 * @name SleepMillis
 * @brief 以毫秒为单位进行睡眠。
 * @param tms 指定睡眠的毫秒数。
 * @return 返回0表示成功，非0表示失败。 
 */
int32_t SleepMillis(uint32_t tms);
#ifdef __cplusplus
}
#endif
#endif