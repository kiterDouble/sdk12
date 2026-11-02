#include "uds_server.h"

typedef uint8_t (*UDSService_t)(UDSServer_t *srv);

/**
 * @name NegativeResponse
 * @brief 生成并发送一个负面响应
 * @param srv 指向UDSServer_t结构体的指针，包含了服务端的相关配置和状态信息。
 * @param response_code 指定的负面响应码。
 * @return 返回传入的response_code。
 */
static uint8_t NegativeResponse(UDSServer_t *srv, uint8_t response_code)
{
    srv->sendBuf[0] = 0x7F;
    srv->sendBuf[1] = srv->recvBuf[0];
    srv->sendBuf[2] = response_code;
    srv->sendSize = UDS_NEG_RESP_LEN;
    return response_code;
}

/**
 * @name NoResponse
 * @brief 无响应处理函数
 *        该函数用于将服务端的发送尺寸设置为0，表示没有响应数据需要发送。
 * @param srv 指向UDSServer_t结构体的指针，用于访问和修改服务端的状态。
 * @return 函数无返回值
 */
static void NoResponse(UDSServer_t *srv)
{
    srv->sendSize = 0;
}

/**
 * @name UDSSecurityAccessLevelIsReserved
 * @brief 检查给定的安全等级是否为服务端保留的安全等级
 * @param srv 指向UDSServer_t结构体的指针，表示服务端配置
 * @param securityLevel 需要检查的安全等级，取值范围为0-63
 * @return 如果给定的安全等级不是服务端保留的安全等级，则返回true；否则返回false。
 */
static bool UDSSecurityAccessLevelIsReserved(const UDSServer_t *srv, uint8_t securityLevel)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return 0;
    }
    securityLevel &= 0x3f;
    return ((srv->securityLevel1 != securityLevel) 
            && (srv->securityLevel2 != securityLevel) 
            && ((srv->securityLevel1 + 1) != securityLevel)
            && ((srv->securityLevel2 + 1) != securityLevel));
}

/**
 * @name DiagnosticSessionControl
 * @brief 处理诊断会话控制请求，根据请求的内容切换不同的诊断会话类型，并响应请求。
 * @param srv 指向UDSServer_t结构体的指针，包含诊断服务端的相关信息和状态。
 * @return 返回执行结果的响应代码。
 */
static uint8_t DiagnosticSessionControl(UDSServer_t *srv)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    if (srv->recvSize < UDS_0X10_REQ_LEN)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    UDSDiagSessionType_e sessType = srv->recvBuf[1] & 0x4F;

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    if ((E_UDS_SESSION_TYPE_DEFAULT != sessType) && (E_UDS_SESSION_TYPE_PROGRAMMING != sessType) && (E_UDS_SESSION_TYPE_EXTENDED != sessType))
    {
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }
    UDSDiagSessCtrlArgs_t args =
    {
        .type = sessType,
        .p2Ms = srv->p2Ms,
        .p2StarMs = srv->p2StarMs,
    };

    uint8_t err = srv->EventProcessCb(srv, E_UDS_SRV_EVT_DiagSessCtrl, &args);

    if (E_UDS_SRV_RESP_CODE_POSITIVE != err)
    {
        return NegativeResponse(srv, err);
    }

    srv->sessionType = sessType;
    srv->p2Ms = args.p2Ms;
    srv->p2StarMs = args.p2StarMs;

    switch (sessType)
    {
    case E_UDS_SESSION_TYPE_DEFAULT:
        break;
    case E_UDS_SESSION_TYPE_PROGRAMMING:
    case E_UDS_SESSION_TYPE_EXTENDED:
    default:
        srv->s3SessionTimeoutTimer = UDSMillis() + srv->s3Ms;
        break;
    }

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_DIAGNOSTIC_SESSION_CONTROL);
    srv->sendBuf[1] = sessType;

    // UDS-1-2013: Table 29
    // resolution: 1ms
    srv->sendBuf[2] = srv->p2Ms >> 8;
    srv->sendBuf[3] = srv->p2Ms;

    // resolution: 10ms
    srv->sendBuf[4] = (srv->p2StarMs / 10) >> 8;
    srv->sendBuf[5] = srv->p2StarMs / 10;

    srv->sendSize = UDS_0X10_RESP_LEN;
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

/**
 * @name ECUReset
 * @brief 处理ECU进行重置请求。
 * @param srv 指向UDS服务端结构体的指针，包含接收缓冲区、事件处理回调等服务相关数据。
 * @return返回执行结果的响应代码。
 */
static uint8_t ECUReset(UDSServer_t *srv)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    uint8_t resetType = srv->recvBuf[1] & 0x3F;

    if (srv->recvSize < UDS_0X11_REQ_MIN_LEN)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    UDSECUResetArgs_t args = {
        .type = resetType,
        .powerDownTime = 0,
    };

    uint8_t err = srv->EventProcessCb(srv, E_UDS_SRV_EVT_EcuReset, &args);

    if (E_UDS_SRV_RESP_CODE_POSITIVE == err)
    {
        srv->notReadyToReceive = true;
        srv->ecuResetScheduled = true;
    }
    else
    {
        return NegativeResponse(srv, err);
    }

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_ECU_RESET);
    srv->sendBuf[1] = resetType;

    if (E_UDS_ECU_RESET_TYPE_ENABLE_RAPID_POWER_SHUT_DOWN == resetType)
    {
        srv->sendBuf[2] = args.powerDownTime;
        srv->sendSize = UDS_0X11_RESP_BASE_LEN + 1;
    }
    else
    {
        srv->sendSize = UDS_0X11_RESP_BASE_LEN;
    }
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

/**
 * @name ClearDiagnosticMessage
 * @brief 处理清除诊断信息请求
 * @param srv 指向UDSServer_t结构体的指针，包含服务端的当前状态和功能。
 * @return 返回执行结果的响应代码。
 */
static uint8_t ClearDiagnosticMessage(UDSServer_t *srv)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    if (srv->recvSize < UDS_0X14_REQ_MIN_LEN)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }
    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    UDSClearDiagnosticArgs_t args =
    {
        .data = &srv->recvBuf[1]
    };

    uint8_t err = srv->EventProcessCb(srv, E_UDS_SRV_EVT_ClearDiagnostic, &args);
    if (E_UDS_SRV_RESP_CODE_POSITIVE != err)
    {
        return NegativeResponse(srv, err);
    }

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION);
    srv->sendSize = UDS_0X14_RESP_BASE_LEN;
    return E_UDS_SRV_RESP_CODE_POSITIVE;
 
}

/**
 * @name UdsSafeCopy
 * @brief 安全复制数据到发送缓冲区
 * @param srv 指向UDSServer_t结构体的指针，表示当前的UDS服务端实例。
 * @param src 指向需要复制的数据源的指针。
 * @param count 需要复制的数据字节长度。
 * @return 返回执行结果的响应代码。
 */
static uint8_t UdsSafeCopy(UDSServer_t *srv, const void *src, uint16_t count)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    if (count <= srv->sendBufSize - srv->sendSize)
    {
        memmove(srv->sendBuf + srv->sendSize, src, count);
        srv->sendSize += count;
        return E_UDS_SRV_RESP_CODE_POSITIVE;
    }
    return E_UDS_SRV_RESP_CODE_RESPONSE_TOO_LONG;
}

/**
 * @name ReadDataByIdentifier
 * @brief 处理0x22服务请求,根据标识符读取数据。
 * @param srv 指向UDSServer_t结构体的指针，包含服务端的配置和状态信息。
 * @return 返回执行结果的响应代码。
 */
static uint8_t ReadDataByIdentifier(UDSServer_t *srv)
{
    uint8_t numDIDs;
    
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_READ_DATA_BY_IDENTIFIER);
    srv->sendSize = 1;

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    if (0 != (srv->recvSize - 1) % sizeof(uint16_t))
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    numDIDs = srv->recvSize / sizeof(uint16_t);

    if (0 == numDIDs)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    for (int32_t did = 0; did < numDIDs; did++)
    {
        uint16_t idx = 1 + did * 2;
        uint16_t dataId = (srv->recvBuf[idx] << 8) + srv->recvBuf[idx + 1];

        if (srv->sendSize + 3 > srv->sendBufSize)
        {
            return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_RESPONSE_TOO_LONG);
        }
        uint8_t *copylocation = srv->sendBuf + srv->sendSize;
        copylocation[0] = dataId >> 8;
        copylocation[1] = dataId;
        srv->sendSize += 2;

        UDSRDBIArgs_t args =
        {
            .dataId = dataId,
            .copy = UdsSafeCopy,
        };

        uint8_t ret = srv->EventProcessCb(srv, E_UDS_SRV_EVT_ReadDataByIdent, &args);

        if (E_UDS_SRV_RESP_CODE_POSITIVE != ret)
        {
            return NegativeResponse(srv, ret);
        }
    }
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

/**
 * @name SecurityAccess
 * @brief 处理安全访问请求，根据请求的子功能执行相应的安全操作。
 * @param srv 指向UDS服务端结构体的指针，包含请求和响应缓冲区以及事件处理回调函数。
 * @return 返回执行结果的响应代码。
 */
static uint8_t SecurityAccess(UDSServer_t *srv)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    uint8_t subFunction = srv->recvBuf[1];

    if (UDSSecurityAccessLevelIsReserved(srv, subFunction))
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF( E_UDS_SID_SECURITY_ACCESS);
    srv->sendBuf[1] = subFunction;
    srv->sendSize = UDS_0X27_RESP_BASE_LEN;

    // Even: sendKey
    if (0 == subFunction % 2)
    {
        uint8_t requestedLevel = subFunction - 1;
        UDSSecAccessValidateKeyArgs_t args =
        {
            .level = requestedLevel,
            .key = &srv->recvBuf[UDS_0X27_REQ_BASE_LEN],
            .len = srv->recvSize - UDS_0X27_REQ_BASE_LEN,
        };

        uint8_t response = srv->EventProcessCb(srv, E_UDS_SRV_EVT_SecAccessValidateKey, &args);

        if (E_UDS_SRV_RESP_CODE_POSITIVE != response)
        {
            return NegativeResponse(srv, response);
        }


        srv->securityLevel = requestedLevel;
        srv->sendSize = UDS_0X27_RESP_BASE_LEN;
        return E_UDS_SRV_RESP_CODE_POSITIVE;
    }
    else// Odd: requestSeed
    {
        /* If a server supports security, but the requested security level is already unlocked when
        a SecurityAccess ‘requestSeed’ message is received, that server shall respond with a
        SecurityAccess ‘requestSeed’ positive response message service with a seed value equal to
        zero (0). The server shall never send an all zero seed for a given security level that is
        currently locked. The client shall use this method to determine if a server is locked for a
        particular security level by checking for a non-zero seed.
        */
        if (subFunction == srv->securityLevel)
        {
            // Table 52 sends a response of length 2. Use a preprocessor define if this needs
            // customizing by the user.
            const uint8_t already_unlocked[] = {0x00, 0x00};
            return UdsSafeCopy(srv, already_unlocked, sizeof(already_unlocked));
        }
        else
        {
            
            UDSSecAccessRequestSeedArgs_t args =
            {
                .level = subFunction,
                .dataRecord = &srv->recvBuf[UDS_0X27_REQ_BASE_LEN],
                .len = srv->recvSize - UDS_0X27_REQ_BASE_LEN,
                .copySeed = UdsSafeCopy,
            };
            
            uint8_t response = srv->EventProcessCb(srv, E_UDS_SRV_EVT_SecAccessRequestSeed, &args);
            if (E_UDS_SRV_RESP_CODE_POSITIVE != response)
            {
                return NegativeResponse(srv, response);
            }

            if (srv->sendSize <= UDS_0X27_RESP_BASE_LEN)
            { // no data was copied
                return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE);
            }
            return E_UDS_SRV_RESP_CODE_POSITIVE;
        }
    }
    return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE);
}

/**
 * @name CommunicationControl
 * @brief 处理通信控制请求，根据接收到的请求类型执行相应的通信控制操作。
 * @param srv 指向UDSServer_t结构体的指针，包含服务相关的信息和数据。
 * @return 返回执行结果的响应代码。
 */
static uint8_t CommunicationControl(UDSServer_t *srv)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    uint8_t controlType = srv->recvBuf[1] & 0x7F;
    uint8_t communicationType = srv->recvBuf[2];

    if (srv->recvSize < UDS_0X28_REQ_BASE_LEN)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    UDSCommCtrlArgs_t args =
    {
        .ctrlType = controlType,
        .commType = communicationType,
    };

    uint8_t err = srv->EventProcessCb(srv, E_UDS_SRV_EVT_CommCtrl, &args);
    if (E_UDS_SRV_RESP_CODE_POSITIVE != err)
    {
        return NegativeResponse(srv, err);
    }

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_COMMUNICATION_CONTROL);
    srv->sendBuf[1] = controlType;
    srv->sendSize = UDS_0X28_RESP_LEN;
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

/**
 * @name WriteDataByIdentifier
 * @brief 处理通过特定标识符写入数据的请求,根据标识符写入数据。
 * @param srv 指向UDS服务端结构体的指针，包含接收到的数据和必要的处理函数。
 * @return 返回执行结果的响应代码。
 */
static uint8_t WriteDataByIdentifier(UDSServer_t *srv)
{
    uint16_t dataLen = 0;
    uint16_t dataId = 0;
    uint8_t err = E_UDS_SRV_RESP_CODE_POSITIVE;

    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    /* UDS-1 2013 Figure 21 Key 1 */
    if (srv->recvSize < UDS_0X2E_REQ_MIN_LEN)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    dataId = (srv->recvBuf[1] << 8) + srv->recvBuf[2];
    dataLen = srv->recvSize - UDS_0X2E_REQ_BASE_LEN;

    UDSWDBIArgs_t args =
    {
        .dataId = dataId,
        .data = &srv->recvBuf[UDS_0X2E_REQ_BASE_LEN],
        .len = dataLen,
    };

    err = srv->EventProcessCb(srv, E_UDS_SRV_EVT_WriteDataByIdent, &args);
    if (E_UDS_SRV_RESP_CODE_POSITIVE != err)
    {
        return NegativeResponse(srv, err);
    }

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_WRITE_DATA_BY_IDENTIFIER);
    srv->sendBuf[1] = dataId >> 8;
    srv->sendBuf[2] = dataId;
    srv->sendSize = UDS_0X2E_RESP_LEN;
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}


/**
 * @name RoutineControl
 * @brief 处理车辆诊断通信中的例行控制请求
 * @param srv 指向UDS服务端结构体的指针，包含接收缓冲区、发送缓冲区和事件处理回调函数等信息。
 * @return 返回执行结果的响应代码。
 */
static uint8_t RoutineControl(UDSServer_t *srv)
{
    uint8_t err = E_UDS_SRV_RESP_CODE_POSITIVE;

    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    if (srv->recvSize < UDS_0X31_REQ_MIN_LEN)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    uint8_t routineControlType = srv->recvBuf[1] & 0x7F;
    uint16_t routineIdentifier = (srv->recvBuf[2] << 8) + srv->recvBuf[3];

    UDSRoutineCtrlArgs_t args =
    {
        .ctrlType = routineControlType,
        .id = routineIdentifier,
        .optionRecord = &srv->recvBuf[UDS_0X31_REQ_MIN_LEN],
        .len = srv->recvSize - UDS_0X31_REQ_MIN_LEN,
        .copyStatusRecord = UdsSafeCopy,
    };

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_ROUTINE_CONTROL);
    srv->sendBuf[1] = routineControlType;
    srv->sendBuf[2] = routineIdentifier >> 8;
    srv->sendBuf[3] = routineIdentifier;
    srv->sendSize = UDS_0X31_RESP_MIN_LEN;

    switch (routineControlType)
    {
    case E_UDS_ROUTINE_CONTROL_TYPE_START:
    case E_UDS_ROUTINE_CONTROL_TYPE_STOP:
    case E_UDS_ROUTINE_CONTROL_TYPE_REQUEST_RESULTS:
        err = srv->EventProcessCb(srv, E_UDS_SRV_EVT_RoutineCtrl, &args);
        if (E_UDS_SRV_RESP_CODE_POSITIVE != err)
        {
            return NegativeResponse(srv, err);
        }
        break;
    default:
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_REQUEST_OUT_OF_RANGE);
    }
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

/**
 * @name ResetTransfer
 * @brief 重置传输状态
 * @param srv 指向UDSServer_t结构体的指针，表示要重置传输状态的UDS服务端实例。
 * @return 函数无返回值
 */
static void ResetTransfer(UDSServer_t *srv)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    srv->xferBlockSequenceCounter = 1;
    srv->xferByteCounter = 0;
    srv->xferTotalBytes = 0;
    srv->xferIsActive = false;
}

/**
 * @name RequestDownload
 * @brief 处理车辆网络中的请求下载服务
 * @param srv 指向UDSServer_t结构体的指针，包含了服务的上下文信息，如接收缓冲区、事件处理回调等。
 * @return 返回执行结果的响应代码。
 */
static uint8_t RequestDownload(UDSServer_t *srv)
{
    uint8_t err;
    size_t memoryAddress = 0;
    size_t memorySize = 0;

    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    if (srv->xferIsActive)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_CONDITIONS_NOT_CORRECT);
    }

    if (srv->recvSize < UDS_0X34_REQ_BASE_LEN)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    uint8_t dataFormatIdentifier = srv->recvBuf[1];
    uint8_t memorySizeLength = (srv->recvBuf[2] & 0xF0) >> 4;
    uint8_t memoryAddressLength = srv->recvBuf[2] & 0x0F;

    if (memorySizeLength == 0 || memorySizeLength > sizeof(memorySize))
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_REQUEST_OUT_OF_RANGE);
    }

    if (memoryAddressLength == 0 || memoryAddressLength > sizeof(memoryAddress))
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_REQUEST_OUT_OF_RANGE);
    }

    if (srv->recvSize < UDS_0X34_REQ_BASE_LEN + memorySizeLength + memoryAddressLength)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    for (int32_t byteIdx = 0; byteIdx < memoryAddressLength; byteIdx++)
    {
        uint8_t byte = srv->recvBuf[UDS_0X34_REQ_BASE_LEN + byteIdx];
        uint8_t shiftBytes = memoryAddressLength - 1 - byteIdx;
        memoryAddress |= byte << (8 * shiftBytes);
    }

    for (int32_t byteIdx = 0; byteIdx < memorySizeLength; byteIdx++)
    {
        uint8_t byte = srv->recvBuf[UDS_0X34_REQ_BASE_LEN + memoryAddressLength + byteIdx];
        uint8_t shiftBytes = memorySizeLength - 1 - byteIdx;
        memorySize |= byte << (8 * shiftBytes);
    }

    UDSRequestDownloadArgs_t args =
    {
        .addr = (void *)memoryAddress,
        .size = memorySize,
        .dataFormatIdentifier = dataFormatIdentifier,
        .maxNumberOfBlockLength = UDS_SERVER_DEFAULT_XFER_DATA_MAX_BLOCKLENGTH,
    };

    err = srv->EventProcessCb(srv, E_UDS_SRV_EVT_RequestDownload, &args);

    if (args.maxNumberOfBlockLength < 3)
    {
        LOG_Error("ERROR: maxNumberOfBlockLength too short");
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE);
    }

    if (E_UDS_SRV_RESP_CODE_POSITIVE != err)
    {
        return NegativeResponse(srv, err);
    }

    ResetTransfer(srv);
    srv->xferIsActive = true;
    srv->xferTotalBytes = memorySize;
    srv->loadType = E_UDS_SID_REQUEST_DOWNLOAD;
    // ISO-14229-1:2013 Table 401:
    uint8_t lengthFormatIdentifier = sizeof(args.maxNumberOfBlockLength) << 4;

    /* ISO-14229-1:2013 Table 396: maxNumberOfBlockLength
    This parameter is used by the requestDownload positive response message to
    inform the client how many data bytes (maxNumberOfBlockLength) to include in
    each TransferData request message from the client. This length reflects the
    complete message length, including the service identifier and the
    data-parameters present in the TransferData request message.
    */
    if (args.maxNumberOfBlockLength > UDS_TP_MTU)
    {
        args.maxNumberOfBlockLength = UDS_TP_MTU;
    }

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_REQUEST_DOWNLOAD);
    srv->sendBuf[1] = lengthFormatIdentifier;
    for (uint8_t idx = 0; idx < sizeof(args.maxNumberOfBlockLength); idx++)
    {
        uint8_t shiftBytes = sizeof(args.maxNumberOfBlockLength) - 1 - idx;
        uint8_t byte = args.maxNumberOfBlockLength >> (shiftBytes * 8);
        srv->sendBuf[UDS_0X34_RESP_BASE_LEN + idx] = byte;
    }
    srv->sendSize = UDS_0X34_RESP_BASE_LEN + sizeof(args.maxNumberOfBlockLength);
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

/**
 * @name RequestUpload
 * @brief 处理请求上传服务的请求
 * @param srv 指向UDSServer_t结构体的指针，包含服务相关的上下文信息。
 * @return 返回执行结果的响应代码。
 */
static uint8_t RequestUpload(UDSServer_t *srv)
{
    uint8_t err;
    size_t memoryAddress = 0;
    size_t memorySize = 0;

    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    if (srv->xferIsActive)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_CONDITIONS_NOT_CORRECT);
    }

    if (srv->recvSize < UDS_0X35_REQ_BASE_LEN)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    uint8_t dataFormatIdentifier = srv->recvBuf[1];
    uint8_t memorySizeLength = (srv->recvBuf[2] & 0xF0) >> 4;
    uint8_t memoryAddressLength = srv->recvBuf[2] & 0x0F;

    if (memorySizeLength == 0 || memorySizeLength > sizeof(memorySize))
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_REQUEST_OUT_OF_RANGE);
    }

    if (memoryAddressLength == 0 || memoryAddressLength > sizeof(memoryAddress))
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_REQUEST_OUT_OF_RANGE);
    }

    if (srv->recvSize < UDS_0X35_REQ_BASE_LEN + memorySizeLength + memoryAddressLength)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    for (int32_t byteIdx = 0; byteIdx < memoryAddressLength; byteIdx++)
    {
        uint8_t byte = srv->recvBuf[UDS_0X35_REQ_BASE_LEN + byteIdx];
        uint8_t shiftBytes = memoryAddressLength - 1 - byteIdx;
        memoryAddress |= byte << (8 * shiftBytes);
    }

    for (int32_t byteIdx = 0; byteIdx < memorySizeLength; byteIdx++)
    {
        uint8_t byte = srv->recvBuf[UDS_0X35_REQ_BASE_LEN + memoryAddressLength + byteIdx];
        uint8_t shiftBytes = memorySizeLength - 1 - byteIdx;
        memorySize |= byte << (8 * shiftBytes);
    }

    UDSRequestUploadArgs_t args =
    {
        .addr = (void *)memoryAddress,
        .size = memorySize,
        .dataFormatIdentifier = dataFormatIdentifier,
        .maxNumberOfBlockLength = UDS_SERVER_DEFAULT_XFER_DATA_MAX_BLOCKLENGTH,
    };

    err = srv->EventProcessCb(srv, E_UDS_SRV_EVT_RequestUpload, &args);

    if (args.maxNumberOfBlockLength < 3)
    {
        LOG_Error("ERROR: maxNumberOfBlockLength too short");
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE);
    }

    if (E_UDS_SRV_RESP_CODE_POSITIVE != err)
    {
        return NegativeResponse(srv, err);
    }

    ResetTransfer(srv);
    srv->xferIsActive = true;
    srv->xferTotalBytes = memorySize;
    srv->loadType = E_UDS_SID_REQUEST_UPLOAD;
    // ISO-14229-1:2013 Table 401:
    uint8_t lengthFormatIdentifier = sizeof(args.maxNumberOfBlockLength) << 4;

    /* ISO-14229-1:2013 Table 396: maxNumberOfBlockLength
    This parameter is used by the requestDownload positive response message to
    inform the client how many data bytes (maxNumberOfBlockLength) to include in
    each TransferData request message from the client. This length reflects the
    complete message length, including the service identifier and the
    data-parameters present in the TransferData request message.
    */
    if (args.maxNumberOfBlockLength > UDS_TP_MTU)
    {
        args.maxNumberOfBlockLength = UDS_TP_MTU;
    }

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_REQUEST_UPLOAD);
    srv->sendBuf[1] = lengthFormatIdentifier;
    for (uint8_t idx = 0; idx < sizeof(args.maxNumberOfBlockLength); idx++)
    {
        uint8_t shiftBytes = sizeof(args.maxNumberOfBlockLength) - 1 - idx;
        uint8_t byte = args.maxNumberOfBlockLength >> (shiftBytes * 8);
        srv->sendBuf[UDS_0X35_RESP_BASE_LEN + idx] = byte;
    }
    srv->sendSize = UDS_0X35_RESP_BASE_LEN + sizeof(args.maxNumberOfBlockLength);
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

/**
 * @name TransferData
 * @brief 处理数据传输请求。
 * @param srv 指向UDSServer_t结构体的指针，包含服务端的各种状态和数据。
 * @return 返回执行结果的响应代码。
 */
static uint8_t TransferData(UDSServer_t *srv)
{
    uint8_t err = E_UDS_SRV_RESP_CODE_POSITIVE;

    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    uint16_t request_data_len = srv->recvSize - UDS_0X36_REQ_BASE_LEN;

    if (srv->recvSize < UDS_0X36_REQ_BASE_LEN)
    {
        err = E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT;
        ResetTransfer(srv);
        return NegativeResponse(srv, err);
    }

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    uint8_t blockSequenceCounter = srv->recvBuf[1];

    if (!srv->xferIsActive)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_UPLOAD_DOWNLOAD_NOT_ACCEPTED);
    }

    if (!srv->RCRRP)
    {
        if (blockSequenceCounter != srv->xferBlockSequenceCounter)
        {
            err = E_UDS_SRV_RESP_CODE_SEQUENCE_ERROR;
            ResetTransfer(srv);
            return NegativeResponse(srv, err);
        }
        else
        {
            srv->xferBlockSequenceCounter++;
        }
    }
    if (srv->xferByteCounter + request_data_len > srv->xferTotalBytes)
    {
        err = E_UDS_SRV_RESP_CODE_TRANSFER_DATA_SUSPENDED;
        ResetTransfer(srv);
        return NegativeResponse(srv, err);
    }

    UDSTransferDataArgs_t args =
    {
        .data = &srv->recvBuf[UDS_0X36_REQ_BASE_LEN],
        .len = srv->recvSize - UDS_0X36_REQ_BASE_LEN,
        .copyResponse = UdsSafeCopy,
    };

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_TRANSFER_DATA);
    srv->sendBuf[1] = blockSequenceCounter;
    srv->sendSize = UDS_0X36_RESP_BASE_LEN;

    err = srv->EventProcessCb(srv, E_UDS_SRV_EVT_TransferData, &args);

    switch (err)
    {
    case E_UDS_SRV_RESP_CODE_POSITIVE:
        if(srv->loadType == E_UDS_SID_REQUEST_DOWNLOAD)
        {
            srv->xferByteCounter += request_data_len;
        }
        return E_UDS_SRV_RESP_CODE_POSITIVE;
    case E_UDS_SRV_RESP_CODE_REQUEST_COLLECTLY_RECEIVED_RESP_PENDING:
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_REQUEST_COLLECTLY_RECEIVED_RESP_PENDING);
    default:
        err = E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }

    ResetTransfer(srv);
    return NegativeResponse(srv, err);
}

/**
 * @name RequestTransferExit
 * @brief 处理请求传输退出的请求。
 * @param srv 指向UDSServer_t结构体的指针，包含服务端的相关信息和状态。
 * @return 返回执行结果的响应代码。
 */
static uint8_t RequestTransferExit(UDSServer_t *srv)
{
    uint8_t err = E_UDS_SRV_RESP_CODE_POSITIVE;

    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    if (!srv->xferIsActive)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_UPLOAD_DOWNLOAD_NOT_ACCEPTED);
    }

    if (srv->recvSize < UDS_0X37_REQ_BASE_LEN)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_REQUEST_TRANSFER_EXIT);
    srv->sendSize = UDS_0X37_RESP_BASE_LEN;

    UDSRequestTransferExitArgs_t args =
    {
        .data = &srv->recvBuf[UDS_0X37_REQ_BASE_LEN],
        .len = srv->recvSize - UDS_0X37_REQ_BASE_LEN,
        .copyResponse = UdsSafeCopy,
    };

    err = srv->EventProcessCb(srv, E_UDS_SRV_EVT_RequestTransferExit, &args);

    switch (err)
    {
    case E_UDS_SRV_RESP_CODE_POSITIVE:
        ResetTransfer(srv);
        return E_UDS_SRV_RESP_CODE_POSITIVE;
    case E_UDS_SRV_RESP_CODE_REQUEST_COLLECTLY_RECEIVED_RESP_PENDING:
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_REQUEST_COLLECTLY_RECEIVED_RESP_PENDING);
    default:
        ResetTransfer(srv);
        return NegativeResponse(srv, err);
    }
}

/**
 * @name TesterPresent
 * @brief 处理会话保持请求
 * @param srv 指向UDSServer_t结构体的指针，包含服务端的当前状态和配置。
 * @return 返回执行结果的响应代码。
 */
static uint8_t TesterPresent(UDSServer_t *srv)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    if (srv->recvSize < UDS_0X3E_REQ_MIN_LEN)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    srv->s3SessionTimeoutTimer = UDSMillis() + srv->s3Ms;
    uint8_t zeroSubFunction = srv->recvBuf[1];
    if (zeroSubFunction == 0x80)
    {
        return E_UDS_SRV_RESP_CODE_POSITIVE;
    }
  
    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_TESTER_PRESENT);
    srv->sendBuf[1] = zeroSubFunction & 0x3F;
    srv->sendSize = UDS_0X3E_RESP_LEN;
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

/**
 * @name ControlDTCSetting
 * @brief 处理车辆诊断系统中的DTC（故障代码）设置控制请求。
 * @param srv 指向UDS服务端结构体的指针，包含接收到的请求数据和回调函数等信息。
 * @return 返回执行结果的响应代码。
 */
static uint8_t ControlDTCSetting(UDSServer_t *srv)
{
    uint8_t err = E_UDS_SRV_RESP_CODE_POSITIVE;

    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    if (srv->recvSize < UDS_0X85_REQ_BASE_LEN)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT);
    }

    if (NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }
    uint8_t dtcSettingType = srv->recvBuf[1] & 0x3F;

    err = srv->EventProcessCb(srv, E_UDS_SRV_EVT_ControlDTCSetting, NULL);
    if (E_UDS_SRV_RESP_CODE_POSITIVE != err)
    {
        return NegativeResponse(srv, err);
    }
    
    srv->sendBuf[0] = UDS_RESPONSE_SID_OF(E_UDS_SID_CONTROL_DTC_SETTING);
    srv->sendBuf[1] = dtcSettingType;
    srv->sendSize = UDS_0X85_RESP_LEN;
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}
/**
 * @name GetServiceForSID
 * @brief 根据给定的SID（服务标识符）获取匹配的内部服务处理程序。
 * @param sid 服务标识符。
 * @return 如果找到匹配的服务，则返回指向UDSService_t的指针；如果没有找到匹配的服务，则返回NULL。
 */
static UDSService_t GetServiceForSID(uint8_t sid)
{
    typedef struct
    {
        UDSDiagnosticServiceId_e sid;
        UDSService_t service;
    }UdsSrvServiceMap_t;

    UdsSrvServiceMap_t mapArr[] = 
    {
        {E_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, DiagnosticSessionControl},
        {E_UDS_SID_ECU_RESET, ECUReset},
        {E_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION, ClearDiagnosticMessage},
        {E_UDS_SID_READ_DATA_BY_IDENTIFIER, ReadDataByIdentifier},
        {E_UDS_SID_SECURITY_ACCESS, SecurityAccess},
        {E_UDS_SID_COMMUNICATION_CONTROL, CommunicationControl},
        {E_UDS_SID_WRITE_DATA_BY_IDENTIFIER, WriteDataByIdentifier},
        {E_UDS_SID_ROUTINE_CONTROL, RoutineControl},
        {E_UDS_SID_REQUEST_DOWNLOAD, RequestDownload},
        {E_UDS_SID_REQUEST_UPLOAD, RequestUpload},
        {E_UDS_SID_TRANSFER_DATA, TransferData},
        {E_UDS_SID_REQUEST_TRANSFER_EXIT, RequestTransferExit},
        {E_UDS_SID_TESTER_PRESENT, TesterPresent},
        {E_UDS_SID_CONTROL_DTC_SETTING, ControlDTCSetting}
    };

    int32_t len = sizeof(mapArr)/sizeof(UdsSrvServiceMap_t);
    int32_t i = 0;

    for (; i < len; i++)
    {
        if (sid == mapArr[i].sid)
        {
            return mapArr[i].service;
        }
    }

    return NULL;

}
/**
 * @name UDSServerCheckSupportSubFun
 * @brief 检查给定的服务标识符（SID）是否被支持的子函数。
 * @param sid 要检查的服务标识符。
 * @return 如果SID被支持，则返回E_UDS_OK；否则返回E_UDS_ERR。
 */
static int32_t UDSServerCheckSupportSubFun(uint8_t sid)
{
    uint8_t supported[] = 
    {
        E_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, 
        E_UDS_SID_ECU_RESET, 
        E_UDS_SID_READ_DTC_INFORMATION, 
        E_UDS_SID_SECURITY_ACCESS, 
        E_UDS_SID_COMMUNICATION_CONTROL, 
        E_UDS_SID_ROUTINE_CONTROL, 
        E_UDS_SID_TESTER_PRESENT, 
        E_UDS_SID_ACCESS_TIMING_PARAMETER, 
        E_UDS_SID_SECURED_DATA_TRANSMISSION, 
        E_UDS_SID_CONTROL_DTC_SETTING, 
        E_UDS_SID_RESPONSE_ON_EVENT
    };
    int32_t i = 0;
    int32_t len = sizeof(supported) / sizeof(uint8_t);

    for (; i < len; i++)
    {
        if (supported[i] == sid)
        {
            return E_UDS_OK;
        }
    }

    return E_UDS_ERR;
}
/**
 * @name UDSServerCheckWithoutSupportSubFun
 * @brief 检查给定的服务标识符（SID）是否不支持子功能。
 * @param sid 要检查的服务标识符（Service Identifier）。
 * @return 如果指定的SID在支持的范围内，则返回E_UDS_OK；否则返回E_UDS_ERR。
 */
static int32_t UDSServerCheckWithoutSupportSubFun(uint8_t sid)
{
    uint8_t supported[] = 
    {
        E_UDS_SID_READ_DATA_BY_IDENTIFIER, 
        E_UDS_SID_READ_MEMORY_BY_ADDRESS, 
        E_UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER, 
        E_UDS_SID_READ_PERIODIC_DATA_BY_IDENTIFIER, 
        E_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER, 
        E_UDS_SID_WRITE_DATA_BY_IDENTIFIER, 
        E_UDS_SID_REQUEST_DOWNLOAD, 
        E_UDS_SID_REQUEST_UPLOAD, 
        E_UDS_SID_TRANSFER_DATA, 
        E_UDS_SID_REQUEST_TRANSFER_EXIT, 
        E_UDS_SID_REQUEST_FILE_TRANSFER, 
        E_UDS_SID_WRITE_MEMORY_BY_ADDRESS, 
        E_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION, 
        E_UDS_SID_INPUT_CONTROL_BY_IDENTIFIER 
    };
    int32_t i = 0;
    int32_t len = sizeof(supported) / sizeof(uint8_t);

    for (; i < len; i++)
    {
        if (supported[i] == sid)
        {
            return E_UDS_OK;
        }
    }

    return E_UDS_ERR;
}

/**
 * @name UDSServerCheckFunSchemeResponse
 * @brief 检查响应是否有效。
 * @param response 服务端返回的响应码，表明了服务的处理结果。
 * @param addressingScheme 地址方案标识，用于指明请求采用的地址方案。
 * @return 返回 E_UDS_OK 表示响应有效，否则返回 E_UDS_ERR 表示响应无效。
 */
static int32_t UDSServerCheckFunSchemeResponse(uint8_t response, const uint8_t addressingScheme)
{
    if ((E_UDS_TP_ADDR_TYPE_FUNCTION == addressingScheme) &&
        ((E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED == response) ||
        (E_UDS_SRV_RESP_CODE_SUBFUN_NOT_SUPPORTED == response) ||
        (E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION == response) ||
        (E_UDS_SRV_RESP_CODE_SUBFUN_NOT_SUPPORTED_IN_ACTIVE_SESSION == response) ||
        (E_UDS_SRV_RESP_CODE_REQUEST_OUT_OF_RANGE == response)))// TODO: *not yet a NRC 0x78 response sent*
    {
        return E_UDS_OK;
    }

    return E_UDS_ERR;
}
/**
 * @name EvaluateServiceResponse
 * @brief 如果服务存在，则调用该服务并根据规范修改响应。
 * @param srv 指向UDS服务端结构体的指针，用于访问服务端的相关数据和功能
 * @param addressingScheme 地址方案，用于确定服务的寻址方式
 * @return 返回服务响应代码，根据服务执行情况和规范要求确定响应代码
 */
static uint8_t EvaluateServiceResponse(UDSServer_t *srv, const uint8_t addressingScheme)
{
    uint8_t response = E_UDS_SRV_RESP_CODE_POSITIVE;
    bool suppressResponse = false;
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }

    uint8_t sid = srv->recvBuf[0];
    UDSService_t service = GetServiceForSID(sid);

    if (srv->sessionType != E_UDS_SESSION_TYPE_DEFAULT)//reset s3Ms
    {
        srv->s3SessionTimeoutTimer = UDSMillis() + srv->s3Ms;
    }

    if (NULL == service || NULL == srv->EventProcessCb)
    {
        return NegativeResponse(srv, E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED);
    }

    response = E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;

    if (E_UDS_OK == UDSServerCheckSupportSubFun(sid))
    {
        /* check minimum length of message with sub-function */
        if (srv->recvSize >= 2)
        {
            /* get sub-function parameter value without bit 7 */
            // switch (ctx->req.as.raw[0] & 0x7F)
            //{
            //}
            // Let the service callback determine whether or not the sub-function parameter value is
            // supported
            response = service(srv);
        } 
        else
        {
            /* NRC 0x13: incorrectMessageLengthOrInvalidFormat */
            response = E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT;
        }

        bool suppressPosRspMsgIndicationBit = srv->recvBuf[1] & 0x80;

        /* test if positive response is required and if responseCode is positive 0x00 */
        if ((suppressPosRspMsgIndicationBit) && (response == E_UDS_SRV_RESP_CODE_POSITIVE) && (true))// TODO: *not yet a NRC 0x78 response sent*
        {
            suppressResponse = true;
        }
        else
        {
            suppressResponse = false;
        }
    }

    if (E_UDS_OK == UDSServerCheckWithoutSupportSubFun(sid))
    {
        response = service(srv);
    }

    if (E_UDS_OK == UDSServerCheckFunSchemeResponse(response, addressingScheme))// TODO: *not yet a NRC 0x78 response sent*
    {
        //suppressResponse = true; /* Suppress negative response message */
        NoResponse(srv);
    }
    else
    {
        if (suppressResponse)
        { /* Suppress positive response message */
            NoResponse(srv);
        }
        else
        { /* send negative or positive response */
            ;
        }
    }
    return response;
}

/**
 * @name ProcessLink
 * @brief 请求数据数据处理
 * @param srv 指向UDSServer_t结构体的指针，服务的上下文
 * @param tpAddrType 传输层地址类型，用于指定数据发送的方式
 * @return 函数无返回值
 */
static void ProcessLink(UDSServer_t *srv, uint32_t tpAddrType)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }
    uint8_t response = EvaluateServiceResponse(srv, tpAddrType);

    if (E_UDS_SRV_RESP_CODE_REQUEST_COLLECTLY_RECEIVED_RESP_PENDING == response)
    {
        srv->RCRRP = true;
        srv->notReadyToReceive = true;
        srv->pending.pending_timer =  UDSMillis() + UDS_SERVER_DEFAULT_P2_STAR_MS;
        srv->pending.wait_pending_complete = true;
        srv->pending.sid = srv->recvBuf[0];
    } else
    {
        srv->RCRRP = false;
    }

    if (srv->sendSize)
    {
        //int32_t result = srv->tp->send(srv->tp, srv->sendBuf, srv->sendSize, tpAddrType);
        //assert(result == srv->sendSize); // how should it be handled if send fails?
        srv->tp->send(srv->tp, srv->sendBuf, srv->sendSize, tpAddrType);
    }
}

/**
 * @name UDSServerInit
 * @brief 该函数用于初始化UDS服务端，配置服务端的基本行为和参数。
 * @param srv 指向UDSServer_t结构体的指针，该结构体代表一个UDS服务端实例。
 * @param cfg 指向UDSServerConfig_t结构体的常量指针，该结构体包含了服务端的配置信息。
 * @return 返回操作结果，成功返回E_UDS_OK，错误返回相应的错误码。
 */
UDSErr_t UDSServerInit(UDSServer_t *srv, const UDSServerConfig_t *cfg)
{
    if (!srv || !cfg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
    }
    memset(srv, 0, sizeof(UDSServer_t));
    srv->recvBufSize = sizeof(srv->recvBuf);
    srv->sendBufSize = sizeof(srv->sendBuf);
    srv->p2Ms = UDS_SERVER_DEFAULT_P2_MS;
    srv->p2StarMs = UDS_SERVER_DEFAULT_P2_STAR_MS;
    srv->s3Ms = UDS_SERVER_DEFAULT_S3_MS;
    srv->EventProcessCb = cfg->EventProcessCb;
    srv->sessionType = E_UDS_SESSION_TYPE_DEFAULT;

    // Initialize p2Timer to an already past time, otherwise the server's
    // response to incoming messages will be delayed.
    srv->p2Timer = UDSMillis() - srv->p2Ms;

    // Set the session timeout for s3 milliseconds from now.
    srv->s3SessionTimeoutTimer = UDSMillis() + srv->s3Ms;
    srv->tp = cfg->tp;
    srv->securityLevel1 = cfg->securityLevel1;
    srv->securityLevel2 = cfg->securityLevel2;

    return E_UDS_OK;
}

/**
 * @name UDSServerDeInit
 * @brief 反初始化UDS服务端。
 * @param srv 指向UDSServer_t结构体的指针，表示需要被反初始化的UDS服务端实例。
 * @return 返回操作结果，成功返回E_UDS_OK，错误返回相应的错误码。
 */
int32_t UDSServerDeInit(const UDSServer_t *srv)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    return E_UDS_OK;
}

/**
 * @name UDSServerPoll
 * @brief 对UDS服务端进行轮询处理。
 * @param srv 指向UDSServer_t结构体的指针，代表一个UDS服务端实例。
 * @return 函数无返回值
 */
void UDSServerPoll(UDSServer_t *srv)
{
    if (!srv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }
    // UDS-1-2013 Figure 38: Session Timeout (S3)
    if (E_UDS_SESSION_TYPE_DEFAULT != srv->sessionType &&
        UDSTimeAfter(UDSMillis(), srv->s3SessionTimeoutTimer))
    {
        srv->EventProcessCb(srv,E_UDS_SRV_EVT_SessionTimeout, NULL);
    }


    uint32_t tpStatus = srv->tp->poll(srv->tp);
    if (tpStatus & E_UDS_TP_SEND_INPROGRESS)
    {
        return;
    }

    // If the user service handler responded RCRRP and the send link is now idle,
    // the response has been sent and the long-running service can now be called.
    if (srv->RCRRP)
    {
        if (srv->pending.wait_pending_complete)
        {
            if (UDSTimeAfter(UDSMillis(), srv->pending.pending_timer))
            {
                unsigned char msg[] = {0x7f, srv->pending.sid, 0x78};
                srv->tp->send(srv->tp, msg, sizeof(msg), E_UDS_TP_ADDR_TYPE_PHYSICAL);
                srv->pending.pending_timer =  UDSMillis() + 3500;
                srv->s3SessionTimeoutTimer = UDSMillis() + srv->s3Ms;
            }
        }
        else
        {
            if (srv->sendSize)
            {
                srv->tp->send(srv->tp, srv->sendBuf, srv->sendSize, E_UDS_TP_ADDR_TYPE_PHYSICAL);
            }
            srv->RCRRP = false;
            srv->notReadyToReceive = false;
        }
        return;
    }

    if (srv->notReadyToReceive)
    {
        return;
    }

    // new data may be processed only after p2 has elapsed
    //size = 0;
    uint32_t tpAddrType = E_UDS_TP_ADDR_TYPE_PHYSICAL;
    if (UDSTimeAfter(UDSMillis(), srv->p2Timer))
    {
        int32_t size = srv->tp->recv(srv->tp, srv->recvBuf, srv->recvBufSize, &tpAddrType);
        if (size > 0)
        {
            srv->recvSize = size;
            ProcessLink(srv, tpAddrType);
            srv->p2Timer = UDSMillis() + srv->p2Ms;
        }
        else
        {
            //LOG_Error("%s recv fail.", __FUNCTION__);
        }
    }

}
