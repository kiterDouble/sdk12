#ifndef __DOIP_MESSAGE_H__
#define __DOIP_MESSAGE_H__
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include "err_code.h"
#include "sr_log.h"


#ifdef __cplusplus
extern "C" {
#endif

#define DOIP_OK (0)
#define DOIP_FAIL (-1)

#define DOIP_PORT (13400)
typedef enum
{
    E_PL_TYPE_HEADER_NEGTIVE_ACK = 0x0000,
    E_PL_TYPE_VEHICLE_INDENTIFY_REQ = 0x0001,
    E_PL_TYPE_VEHICLE_INDENTIFY_EID_REQ = 0x0002,
    E_PL_TYPE_VEHICLE_INDENTIFY_VIN_REQ = 0x0003,
    E_PL_TYPE_ANNOUNCE_OR_IDENTIFYRES_RES = 0x0004,
    E_PL_TYPE_ROUTING_ACTIVATION_REQ = 0x0005,
    E_PL_TYPE_ROUTING_ACTIVE_RES = 0x0006,
    E_PL_TYPE_ALIVE_CHECK_REQ = 0x0007,
    E_PL_TYPE_ALIVE_CHECK_RES = 0x0008,
    E_PL_TYPE_ENTITY_STATUS_REQ = 0x4001,
    E_PL_TYPE_ENTITY_STATUS_RES = 0x4002,
    E_PL_TYPE_POWER_MODE_REQ = 0x4003,
    E_PL_TYPE_POWER_MODE_RES = 0x4004,
    E_PL_TYPE_DIAGNOSTIC_REQ = 0x8001,
    E_PL_TYPE_DIAG_POSITIVE_ACK_RES = 0x8002,
    E_PL_TYPE_DIAG_NEGATIVE_ACK_RES = 0x8003
}DoipPayloadType_e;

/*bytes to payload type*/
#define BYTES_TO_UINT16(hbyte,lbyte) (((hbyte) << 8) + (lbyte))
/*bytes to payload length*/
#define BYTES_TO_UINT32(hhbyte,hlbyte,lhbyte,llbyte) (((hhbyte) << 24) + ((hlbyte) << 16) + ((lhbyte) << 8) + (llbyte))

typedef enum
{
    E_PL_HEADER_NEGTIVE_ACK_LEN = 0x01,
    E_PL_VEHICLE_INDENTIFY_LEN = 0x00,
    E_PL_VEHICLE_INDENTIFY_EID_LEN = 0x06,
    E_PL_VEHICLE_INDENTIFY_VIN_LEN = 0x11,
    E_PL_VEHICLE_INDENTIFY_RES_MIN_LEN = 0x20,
    E_PL_VEHICLE_INDENTIFY_RES_LEN = 0x21,
    E_PL_ROUTING_ACTIVATION_REQ_LEN = 0x07,
    E_PL_ROUTING_ACTIVATION_RES_LEN = 0x0d,
    E_PL_ALIVE_CHECK_REQ_LEN = 0x00,
    E_PL_ALIVE_CHECK_RES_LEN = 0x02,
    E_PL_ENTITY_STATUS_REQ_LEN = 0x00,
    E_PL_ENTITY_STATUS_RES_LEN = 0x07,
    E_PL_POWER_MODE_REQ_LEN = 0x00,
    E_PL_POWER_MODE_RES_LEN = 0x01,
    E_PL_DIAGNOSTIC_REQ_MIN_LEN = 0x05,/*diagnostic payload max size*/
    E_PL_DIAGNOSTIC_ACK_REQ_MIN_LEN = 0x05,
    E_PL_DIAGNOSTIC_NACK_REQ_MIN_LEN = 0x05,
}DoipPayloadDataLen_e;

typedef enum
{
    E_GDHNAE_INCORRECT_PATTERN_FORMAT = 0x00,
    E_GDHNAE_UNKNOWN_PAYLOAD_TYPE = 0x01,
    E_GDHNAE_MESSAGE_TOO_LARGE = 0x02,
    E_GDHNAE_OUT_OF_MEMORY = 0x03,
    E_GDHNAE_INVALID_PAYLOAD_LENGTH = 0x4,
    E_GDHNAE_RESERVED_BY_ISO13400 = 0x05
}GenericDoipHeaderNegativeAcknowledError_e;

typedef enum
{
    E_RARC_UNKNOWN_SOURCE_ADDRESS = 0x00,
    E_RARC_ALL_SOCKETS_ARE_REGISTERED_AND_ACTIVE = 0x01,
    E_RARC_SA_WAS_RECEIVED_ON_THE_ALREADY_ACTIVATED = 0x02,
    E_RARC_SA_IS_ALREADY_REGISTERED_AND_ACTIVE = 0x03,
    E_RARC_MISSING_AUTHENTICATION = 0x04,
    E_RARC_REJECTED_CONFIRMATION = 0x05,
    E_RARC_UNSUPPORTED_ROUTING_ACTIVATION_TYPE = 0x06,
    E_RARC_SUCCESSFULLY_ACTIVATED = 0x10,
    E_RARC_WILL_BE_ACTIVATED = 0x11
}RoutineActivationResCode_e;

typedef enum
{
    E_ESNT_GATEWAY = 0x00,
    E_ESNT_NODE = 0x01,
    E_ESNT_RESERVER = 0x02
}EntityStatusNodeType_e;

typedef enum
{
    E_DPM_NOT_READY = 0x00,
    E_DPM_READY = 0x01,
    E_DPM_NOT_SUPPORTED = 0x02,
    E_DPM_RESERVER = 0x03
}DiagnosticPowerMode_e;

typedef enum
{
    E_DMPAC_SUCCESS = 0x00,
    E_DMPAC_ISO_RESERVE
}DiagMesgAckCode_e;

typedef enum
{
    E_DMNAC_ISO_RESERVE0 = 0x00,
    E_DMNAC_ISO_RESERVE1 = 0x01,
    E_DMNAC_INVALID_SOURCE_ADDRESS = 0x02,
    E_DMNAC_UNKNOWN_TARGET_ADDRESS = 0x03,
    E_DMNAC_DIAGNOSTIC_MESSAGE_TOO_LAGRGE = 0x04,
    E_DMNAC_TARGET_UNREACHABLE = 0x05,
    E_DMNAC_UNKNOW_NETWORK = 0x06,
    E_DMNAC_TRANSPORT_PROTOCAL_ERROR = 0x07
}DiagMesgNackCode_e;

typedef struct
{
    uint8_t protocal;
    uint8_t reverse;
    uint8_t type[2];
    uint8_t length[4];
    uint8_t payload[0];
}DoipPacket_t;

typedef struct 
{
    uint8_t protocalType;/*protocal type*/
    uint8_t typeReverse;/*protocal type reverse*/
    uint16_t payloadType;/*payload type*/
    uint32_t payloadLen;/*payload length*/
}DoipMsgHeader_t;

typedef struct
{
    uint8_t nack;
}GeneralHeaderNack_t;

typedef struct
{
    uint8_t eid[6];
}VehicleIDReqWithEid_t;

typedef struct
{
    uint8_t vin[17];
}VehicleIDReqWithVin_t;

typedef struct 
{
    uint8_t vin[17];
    uint8_t logicAddr[2];
    uint8_t eid[6];
    uint8_t gid[6];
    uint8_t actionReq;
}VehicleAnnoRes_t, VehicleIDRes_t;

typedef struct
{
    uint8_t sourceAddr[2];
    uint8_t activeType;
    uint8_t reservedByISO13400[4];
    uint8_t reservedByOEM[4];
}RoutineActivationReq_t;

typedef struct
{
    uint8_t testEquipment[2];
    uint8_t entityEquipment[2];
    uint8_t code;
    uint8_t reservedByISO13400[4];
    uint8_t reservedByOEM[4];
}RoutineActivationRes_t;

typedef struct
{
    uint8_t sourceAddr[2];
}AliveCheckRes_t;

typedef struct
{
    uint8_t nodeType;
    uint8_t mcts;
    uint8_t ncts;
    uint8_t mds[4];
}VehicleEntityStatusRes_t;

typedef struct
{
    uint8_t powerMode;
}PowerModeInfoRes_t;

typedef struct
{
    uint8_t sourceAddr[2];
    uint8_t targetAddr[2];
    uint8_t userdata[0];
}DiagnosticMessage_t;

typedef struct
{
    uint8_t sourceAddr[2];
    uint8_t targetAddr[2];
    uint8_t code;
    uint8_t userdata[0];
}DiagnosticMessageAck_t, DiagnosticMessageNack_t;

typedef int32_t (*CheckPayloadLenCallback)(const uint8_t *data, uint32_t dataLen);

typedef struct
{
    DoipPayloadType_e type;
    CheckPayloadLenCallback checkLenCb;
}CheckLen_t;

/**
 * @name DoipParserHeader
 * @brief 解析DOIP消息头
 *        本函数用于解析接收到的DOIP（Diagnostic Over IP）消息的头部信息，提取出协议类型、消息方向、负载类型和负载长度等关键信息。
 * @param data 指向接收到的原始DOIP数据包的指针。
 * @param dataLen 接收到的数据包长度。
 * @param header 指向用于存储解析后消息头信息的结构体的指针。
 * @return 若解析成功，返回RET_OK（假设为0）；若参数无效或数据包长度不足，返回RET_ERR（假设为非0值）。
 */
int32_t DoipParserHeader(const uint8_t *data, uint32_t dataLen, DoipMsgHeader_t *header);

/**
 * @name DoipGenerateHeader
 * @brief 生成DOIP消息头
 *        本函数用于根据给定的消息类型和长度生成一个DOIP消息的头部结构体。
 * @param type 消息负载类型，枚举类型DoipPayloadType_e。
 * @param length 消息负载的长度，单位为字节。
 * @param header 指向接收消息头的结构体的指针。
 * @return 函数执行状态，RET_OK表示成功，RET_ERR表示错误。
 */
int32_t DoipGenerateHeader(DoipPayloadType_e type, uint32_t length, DoipMsgHeader_t *header);


/**
 * @name DoipGeneratePacket
 * @brief 生成DOIP包。
 *        根据提供的DOIP消息头和负载数据，构建一个DOIP包。
 * @param header 指向DOIP消息头的常量指针。消息头包含协议类型、负载类型、负载长度等信息。
 * @param payload 指向负载数据的常量字节数组指针。如果消息头指定的负载长度大于0，此参数不能为空。
 * @param data 指向用于存储生成的DOIP包的字节数组的指针。
 * @param dataLen 指向用于存储生成的DOIP包长度的整型变量的指针。
 * @return 函数执行状态码。RET_OK表示成功，RET_ERR表示错误。
 */
int32_t DoipGeneratePacket(const DoipMsgHeader_t *header, 
                            const uint8_t *payload, 
                            uint8_t *data, 
                            uint32_t *dataLen);

#ifdef __cplusplus
}
#endif
#endif
