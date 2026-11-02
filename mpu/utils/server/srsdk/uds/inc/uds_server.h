#ifndef __UDS_SRV_H__
#define __UDS_SRV_H__
#include "uds_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UDS_SERVER_DEFAULT_P2_MS
#define UDS_SERVER_DEFAULT_P2_MS (50)
#endif

#ifndef UDS_SERVER_DEFAULT_P2_STAR_MS
#define UDS_SERVER_DEFAULT_P2_STAR_MS (2000)
#endif

#ifndef UDS_SERVER_DEFAULT_S3_MS
#define UDS_SERVER_DEFAULT_S3_MS (5000)
#endif

_Static_assert(0 < UDS_SERVER_DEFAULT_P2_MS &&
                   UDS_SERVER_DEFAULT_P2_MS < UDS_SERVER_DEFAULT_P2_STAR_MS &&
                   UDS_SERVER_DEFAULT_P2_STAR_MS < UDS_SERVER_DEFAULT_S3_MS,
               "");

#ifndef UDS_SERVER_DEFAULT_XFER_DATA_MAX_BLOCKLENGTH
/*! ISO14229-1:2013 Table 396. This parameter is used by the requestDownload positive response
message to inform the client how many data bytes (maxNumberOfBlockLength) to include in each
TransferData request message from the client. */
#define UDS_SERVER_DEFAULT_XFER_DATA_MAX_BLOCKLENGTH UDS_BUFSIZE_MAX
#endif

typedef enum
{
    E_UDS_SRV_EVT_DiagSessCtrl,         // UDSDiagSessCtrlArgs_t * 0x10
    E_UDS_SRV_EVT_EcuReset,             // UDSECUResetArgs_t * 0x11
    E_UDS_SRV_EVT_ClearDiagnostic,       // 0x14
    E_UDS_SRV_EVT_ReadDataByIdent,      // UDSRDBIArgs_t * 0x22
    E_UDS_SRV_EVT_CommCtrl,             // UDSCommCtrlArgs_t * 0x28 
    E_UDS_SRV_EVT_SecAccessRequestSeed, // UDSSecAccessRequestSeedArgs_t * 0x27
    E_UDS_SRV_EVT_SecAccessValidateKey, // UDSSecAccessValidateKeyArgs_t * 0x27
    E_UDS_SRV_EVT_WriteDataByIdent,     // UDSWDBIArgs_t * 0x2e
    E_UDS_SRV_EVT_RoutineCtrl,          // UDSRoutineCtrlArgs_t* 0x31
    E_UDS_SRV_EVT_RequestDownload,      // UDSRequestDownloadArgs_t* 0x34
    E_UDS_SRV_EVT_RequestUpload,        // UDSRequestUploadArgs_t * 0x35
    E_UDS_SRV_EVT_TransferData,         // UDSTransferDataArgs_t * 0x36
    E_UDS_SRV_EVT_RequestTransferExit,  // UDSRequestTransferExitArgs_t * 0x37
    E_UDS_SRV_EVT_SessionTimeout,       // NULL
    E_UDS_SRV_EVT_PowerDown,            // UDSPowerDownArgs_t *
    E_UDS_SRV_EVT_ControlDTCSetting     // 0x85
}UDSServerEvent_e;

typedef UDSServerEvent_e UDSServerEvent_t;

typedef struct
{
    uint32_t pending_timer;
    bool wait_pending_complete;
    uint8_t sid;
}UdsSericePending_t;

typedef struct UDSServer
{
    UDSTpHandle_t *tp;
    uint8_t (*EventProcessCb)(struct UDSServer *srv, UDSServerEvent_t event, const void *arg);
    uint8_t recvBuf[UDS_BUFSIZE_MAX];
    uint8_t sendBuf[UDS_BUFSIZE_MAX];
    uint16_t recvSize;
    uint16_t sendSize;
    uint16_t recvBufSize;
    uint16_t sendBufSize;

    /**
     * @brief \~chinese 服务器时间参数（毫秒） \~ Server time constants (milliseconds) \~
     */
    uint16_t p2Ms;      // Default P2_server_max timing supported by the server for
                         // the activated diagnostic session.
    uint32_t p2StarMs; // Enhanced (NRC 0x78) P2_server_max supported by the
                         // server for the activated diagnostic session.
    uint16_t s3Ms;      // Session timeout

    bool ecuResetScheduled;            // indicates that an ECUReset has been scheduled
    uint32_t ecuResetTimer;            // for delaying resetting until a response
                                       // has been sent to the client
    uint32_t p2Timer;                 // for rate limiting server responses
    uint32_t s3SessionTimeoutTimer; // for knowing when the diagnostic
                                       // session has timed out

    /**
     * @brief UDS-1-2013: Table 407 - 0x36 TransferData Supported negative
     * response codes requires that the server keep track of whether the
     * transfer is active
     */
    bool xferIsActive;
    // UDS-1-2013: 14.4.2.3, Table 404: The blockSequenceCounter parameter
    // value starts at 0x01
    uint8_t xferBlockSequenceCounter;
    size_t xferTotalBytes;  // total transfer size in bytes requested by the client
    size_t xferByteCounter; // total number of bytes transferred

    /**
     * @brief public subset of server state for user handlers
     */
    uint8_t sessionType;
    uint8_t securityLevel; // Current SecurityAccess (0x27) level
    uint8_t securityLevel1;
    uint8_t securityLevel2;
    // this variable set to true when a user handler returns 0x78
    // requestCorrectlyReceivedResponsePending. After a response has been sent on the transport
    // layer, this variable is set to false and the user handler will be called again. It is the
    // responsibility of the user handler to track the call count.
    bool RCRRP;

    // UDS-1 2013 defines the following conditions under which the server does not
    // process incoming requests:
    // - not ready to receive (Table A.1 0x78)
    // - not accepting request messages and not sending responses (9.3.1)
    //
    // when this variable is set to true, incoming ISO-TP data will not be processed.
    bool notReadyToReceive;
    UdsSericePending_t pending;
    uint8_t loadType;//0x34 download, 0x35 upload.
} UDSServer_t;

typedef struct
{
    uint8_t (*EventProcessCb)(UDSServer_t *srv, UDSServerEvent_t event, const void *arg);
    UDSTpHandle_t *tp;
    uint8_t securityLevel1;
    uint8_t securityLevel2;
} UDSServerConfig_t;

typedef struct
{
    const UDSDiagSessionType_e type; /*! requested session type */
    uint16_t p2Ms;                           /*! optional: p2 timing override */
    uint32_t p2StarMs;                      /*! optional: p2* timing override */
} UDSDiagSessCtrlArgs_t;

typedef struct
{
    const UDSECUResetType_e type; /**< \~chinese 客户端请求的复位类型 \~english reset type requested by client */
    uint8_t powerDownTime; /**< Optional response: notify client of time until shutdown (0-254) 255
                              indicates that a time is not available. */
} UDSECUResetArgs_t;

typedef struct
{
    const UDSECUResetType_e type; /**< \~chinese 客户端请求的复位类型 \~english reset type requested by client */
} UDSPowerDownArgs_t;

typedef struct
{
    const uint16_t dataId; /*! RDBI Data Identifier */
    uint8_t (*copy)(UDSServer_t *srv, const void *src,
                    uint16_t count); /*! function for copying data */
} UDSRDBIArgs_t;

typedef struct
{
    const uint8_t *const data;
}UDSClearDiagnosticArgs_t;

typedef struct
{
    const uint16_t dataId;     /*! WDBI Data Identifier */
    const uint8_t *const data; /*! pointer to data */
    const uint16_t len;        /*! length of data */
} UDSWDBIArgs_t;

typedef struct
{
    UDSCommunicationControlType_e ctrlType;
    UDSCommunicationType_e commType;
} UDSCommCtrlArgs_t;

typedef struct
{
    const uint8_t level;             /*! requested security level */
    const uint8_t *const dataRecord; /*! pointer to request data */
    const uint16_t len;              /*! size of request data */
    uint8_t (*copySeed)(UDSServer_t *srv, const void *src, uint16_t len); /*! function for copying data */
} UDSSecAccessRequestSeedArgs_t;

typedef struct
{
    const uint8_t level;      /*! security level to be validated */
    const uint8_t *const key; /*! key sent by client */
    const uint16_t len;       /*! length of key */
} UDSSecAccessValidateKeyArgs_t;

typedef struct
{
    const uint8_t ctrlType;      /*! routineControlType */
    const uint16_t id;           /*! routineIdentifier */
    const uint8_t *optionRecord; /*! optional data */
    const uint16_t len;          /*! length of optional data */
    uint8_t (*copyStatusRecord)(UDSServer_t *srv, const void *src,
                                uint16_t len); /*! function for copying response data */
} UDSRoutineCtrlArgs_t;

typedef struct
{
    const void *addr;                   /*! requested address */
    const size_t size;                  /*! requested download size */
    const uint8_t dataFormatIdentifier; /*! optional specifier for format of data */
    uint16_t maxNumberOfBlockLength; /*! response: inform client how many data bytes to send in each
                                        `TransferData` request */
} UDSRequestDownloadArgs_t, UDSRequestUploadArgs_t;

typedef struct
{
    const uint8_t *const data; /*! transfer data */
    const uint16_t len;        /*! transfer data length */
    uint8_t (*copyResponse)(UDSServer_t *srv, const void *src, uint16_t len); /*! function for copying transfer data response data (optional) */
} UDSTransferDataArgs_t;

typedef struct {
    const uint8_t *const data; /*! request data */
    const uint16_t len;        /*! request data length */
    uint8_t (*copyResponse)(UDSServer_t *srv, const void *src, uint16_t len); /*! function for copying response data (optional) */
} UDSRequestTransferExitArgs_t;

/**
 * @name UDSServerInit
 * @brief 该函数用于初始化UDS服务端，配置服务端的基本行为和参数。
 * @param srv 指向UDSServer_t结构体的指针，该结构体代表一个UDS服务端实例。
 * @param cfg 指向UDSServerConfig_t结构体的常量指针，该结构体包含了服务端的配置信息。
 * @return 返回操作结果，成功返回E_UDS_OK，错误返回相应的错误码。
 */
int32_t UDSServerInit(UDSServer_t *srv, const UDSServerConfig_t *cfg);
/**
 * @name UDSServerPoll
 * @brief 对UDS服务端进行轮询处理。
 * @param srv 指向UDSServer_t结构体的指针，代表一个UDS服务端实例。
 * @return 函数无返回值
 */
void UDSServerPoll(UDSServer_t *srv);
/**
 * @name UDSServerDeInit
 * @brief 反初始化UDS服务端。
 * @param srv 指向UDSServer_t结构体的指针，表示需要被反初始化的UDS服务端实例。
 * @return 返回操作结果，成功返回E_UDS_OK，错误返回相应的错误码。
 */
int32_t UDSServerDeInit(const UDSServer_t *srv);
#ifdef __cplusplus
}
#endif
#endif