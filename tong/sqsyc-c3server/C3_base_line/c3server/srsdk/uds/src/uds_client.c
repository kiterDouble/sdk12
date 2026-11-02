#include "uds_client.h"

/**
 * @name ClearRequestContext
 * @brief 清除请求上下文
 * @param client 指向UDS客户端结构体的指针。
 * @return 函数无返回值
 */
static void ClearRequestContext(UDSClient_t *client)
{
    if (!client || !client->tp)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    (void)memset(client->recvBuf, 0, client->recvBufSize);
    (void)memset(client->sendBuf, 0, client->sendBufSize);
    client->recvSize = 0U;
    client->sendSize = 0U;
    client->state = E_UDS_CLT_REQUEST_STATE_IDLE;
    client->err = E_UDS_OK;
}

/**
 * @name UDSClientInit
 * @brief UDS client初始化函数
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param cfg 配置参数
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSClientInit(UDSClient_t *client, const UDSClientConfig_t *cfg)
{
    if (!client || !cfg || !cfg->tp)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    (void)memset(client, 0, sizeof(*client));

    client->p2Ms = UDS_CLIENT_DEFAULT_P2_MS;
    client->p2StarMs = UDS_CLIENT_DEFAULT_P2_STAR_MS;
    client->recvBufSize = (uint16_t)sizeof(client->recvBuf);
    client->sendBufSize = (uint16_t)sizeof(client->sendBuf);

    if (client->p2StarMs < client->p2Ms)
    {
        client->p2StarMs = client->p2Ms;
    }

    client->tp = cfg->tp;
    ClearRequestContext(client);
    return E_UDS_OK;
}

/**
 * @name UDSClientInit
 * @brief UDS client反初始化函数
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSClientDeInit(const UDSClient_t *client)
{
    if (!client || !client->tp)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    //do noting.
    return E_UDS_OK;
}

/**
 * @name ChangeState
 * @brief 更改UDS客户端当前的状态。
 * @param client 指向UDS客户端结构体的指针，用于标识需要更改状态的客户端。
 * @param state 欲设置的客户端请求状态，为UDSClientRequestState_e枚举类型。
 * @return 函数无返回值
 */
static void ChangeState(UDSClient_t *client, UDSClientRequestState_e state)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }
    client->state = (uint8_t)state;
}

/**
 * @name ClientValidateResponse
 * @brief 检查响应是否为有效的UDS响应
 * @param ctx 指向UDS客户端上下文的指针，用于验证响应。
 * @return UDSErr_t 返回UDS错误代码，表示响应的验证结果。
 */
static UDSErr_t ClientValidateResponse(const UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    if (client->recvSize < 1U)
    {
        return E_UDS_ERR_RESP_TOO_SHORT;
    }

    if (0x7FU == client->recvBuf[0])//否定响应
    { 
        if (client->recvSize < 2U)
        {
            return E_UDS_ERR_RESP_TOO_SHORT;
        } 
        else if (client->sendBuf[0] != client->recvBuf[1])
        {
            return E_UDS_ERR_SID_MISMATCH;
        }
        else if (E_UDS_SRV_RESP_CODE_REQUEST_COLLECTLY_RECEIVED_RESP_PENDING == (UDSSrvRespCode_e)client->recvBuf[2])
        {
            return E_UDS_OK;
        }
        else if (0U != (client->optionsCopy & (uint8_t)E_UDS_CLT_NEG_RESP_IS_ERR))
        {
            return E_UDS_ERR_NEG_RESP;
        }
        else
        {
            //do nothing
            ;
        }
    }
    else// 肯定响应
    { 
        if (UDS_RESPONSE_SID_OF(client->sendBuf[0]) != client->recvBuf[0])
        {
            return E_UDS_ERR_SID_MISMATCH;
        }
        switch ((UDSDiagnosticServiceId_e)client->sendBuf[0])
        {
        case E_UDS_SID_ECU_RESET:
            if (client->recvSize < 2U)
            {
                return E_UDS_ERR_RESP_TOO_SHORT;
            }
            else if (client->sendBuf[1] != client->recvBuf[1])
            {
                return E_UDS_ERR_SUBFUNCTION_MISMATCH;
            }
            else
            {
                //do nothing
                ;
            }
            break;
        default:
            break;
        }
        
    }

    return E_UDS_OK;
}

/**
 * @name ClientHandleResponse
 * @brief 处理验证过的服务端响应
 * @param client 指向UDS客户端结构体的
 * @return 函数无返回值
 */
static void ClientHandleResponse(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    if (0x7FU == client->recvBuf[0])
    {
        if (E_UDS_SRV_RESP_CODE_REQUEST_COLLECTLY_RECEIVED_RESP_PENDING == (UDSSrvRespCode_e)client->recvBuf[2])
        {
            LOG_Debug("got RCRRP, setting p2 timer");
            client->peTimer = UDSMillis() + client->p2StarMs;
            (void)memset(client->recvBuf, 0, client->recvBufSize);
            client->recvSize = 0U;
            ChangeState(client, E_UDS_CLT_REQUEST_STATE_AWAIT_RESPONSE);
            return;
        }
        else
        {
            ;
        }
    }
    else
    {
        uint8_t respSid = client->recvBuf[0];
        uint8_t reqSid = UDS_RESPONSE_SID_OF(respSid);
        switch ((UDSDiagnosticServiceId_e)reqSid)
        {
        case E_UDS_SID_DIAGNOSTIC_SESSION_CONTROL:
        {
            if (client->recvSize < UDS_0X10_RESP_LEN)
            {
                LOG_Debug("Error: SID %x response too short",
                              E_UDS_SID_DIAGNOSTIC_SESSION_CONTROL);
                client->err = E_UDS_ERR_RESP_TOO_SHORT;
                ChangeState(client, E_UDS_CLT_REQUEST_STATE_IDLE);
                return;
            }

            if (0U != (client->optionsCopy & (uint8_t)E_UDS_CLT_IGNORE_SERVER_TIMINGS))
            {
                ChangeState(client, E_UDS_CLT_REQUEST_STATE_IDLE);
                return;
            }

            uint16_t p2 = ((uint16_t)client->recvBuf[2] << 8U) + (uint16_t)client->recvBuf[3];
            uint32_t p2_star = (((uint32_t)client->recvBuf[4] << 8U) + (uint32_t)client->recvBuf[5]) * 10U;
            LOG_Debug("received new timings: p2: %u, p2*: %u", p2, p2_star);
            client->p2Ms = p2;
            client->p2StarMs = p2_star;
            break;
        }
        default:
            break;
        }
    }
    ChangeState(client, E_UDS_CLT_REQUEST_STATE_IDLE);
}

/**
 * @name PollLowLevel
 * @brief 执行客户端请求状态机
 * @param client 指向UDS客户端结构体的指针
 * @return 函数无返回值
 */
static void PollLowLevel(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }
    uint32_t tpStatus = client->tp->poll(client->tp);
    switch ((UDSClientRequestState_e)client->state)
    {
    case E_UDS_CLT_REQUEST_STATE_IDLE:
        client->options = client->defaultOptions;
        break;
    case E_UDS_CLT_REQUEST_STATE_SENDING:
    {
        uint32_t tpAddrType = E_UDS_TP_ADDR_TYPE_PHYSICAL;
        ssize_t ret = 0;

        if (0U != (client->optionsCopy & (uint8_t)E_UDS_CLT_FUNCTIONAL))
        {
            tpAddrType = E_UDS_TP_ADDR_TYPE_FUNCTION;
        }        
        ret = client->tp->send(client->tp, client->sendBuf, client->sendSize, tpAddrType);
        if (ret < 0)
        {
            client->err = E_UDS_ERR_TPORT;
            LOG_Debug("tport err: %d", ret);
        }
        else if (0 == ret)
        {
            LOG_Debug("send in progress...");
            ; // 等待发送成功
        }
        else if ((ssize_t)client->sendSize == ret)
        {
            ChangeState(client, E_UDS_CLT_REQUEST_STATE_AWAIT_SEND_COMPLETE);
        }
        else
        {
            client->err = E_UDS_ERR_BUFSIZ;
        }
        break;
    }
    case E_UDS_CLT_REQUEST_STATE_AWAIT_SEND_COMPLETE:
    {
        if (0U != (client->optionsCopy & (uint8_t)E_UDS_CLT_FUNCTIONAL))
        {
            // "The Functional addressing is applied only to single frame transmission"
            // Specification of Diagnostic Communication (Diagnostic on CAN - Network Layer)
            ChangeState(client, E_UDS_CLT_REQUEST_STATE_IDLE);
        }

        if (0U != (tpStatus & (uint32_t)E_UDS_TP_SEND_INPROGRESS))
        {
            ; // await send complete
        }
        else
        {
            if (0U != (client->optionsCopy & (uint8_t)E_UDS_CLT_SUPPRESS_POS_RESP))
            {
                ChangeState(client, E_UDS_CLT_REQUEST_STATE_IDLE);
            }
            else
            {
                ChangeState(client, E_UDS_CLT_REQUEST_STATE_AWAIT_RESPONSE);
                client->peTimer = UDSMillis() + client->p2Ms;
            }
        }
        break;
    }
    case E_UDS_CLT_REQUEST_STATE_AWAIT_RESPONSE:
    {
        uint32_t tpAddrType = (uint32_t)E_UDS_TP_ADDR_TYPE_PHYSICAL;
        ssize_t ret = client->tp->recv(client->tp, client->recvBuf, client->recvBufSize, &tpAddrType);

        if (E_UDS_TP_ADDR_TYPE_FUNCTION == (UDSTpAddr_e)tpAddrType)
        {
            break;
        }
        if (ret < 0)
        {
            client->err = E_UDS_ERR_TPORT;
            ChangeState(client, E_UDS_CLT_REQUEST_STATE_IDLE);
        }
        else if (0 == ret)
        {
            if (0 != UDSTimeAfter(UDSMillis(), client->peTimer))
            {
                client->err = E_UDS_ERR_TIMEOUT;
                ChangeState(client, E_UDS_CLT_REQUEST_STATE_IDLE);
            }
        }
        else
        {
            client->recvSize = (uint16_t)ret;
            ChangeState(client, E_UDS_CLT_REQUEST_STATE_PROCESS_RESPONSE);
        }
        break;
    }
    case E_UDS_CLT_REQUEST_STATE_PROCESS_RESPONSE:
    {
        client->err = ClientValidateResponse(client);
        if (E_UDS_OK == client->err)
        {
            ClientHandleResponse(client);
        }
        else
        {
            ChangeState(client, E_UDS_CLT_REQUEST_STATE_IDLE);
        }
        break;
    }

    default:
        break;
    }
}

/**
 * @name SendRequest
 * @brief 向服务器发送一个请求，并处理相关的状态转换
 *        如果请求中配置了抑制正响应选项，则会相应地修改发送缓冲区的位模式。
 * @param client 指向UDS客户端结构体的指针，用于存储客户端的状态和选项等。
 * @return 返回UDS错误码，如果操作成功则返回E_UDS_OK，如果参数无效则返回E_UDS_ERR。
 */
static UDSErr_t SendRequest(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    client->optionsCopy = client->options;

    if (0U != (client->optionsCopy & (uint8_t)E_UDS_CLT_SUPPRESS_POS_RESP))
    {
        // UDS-1:2013 8.2.2 Table 11
        client->sendBuf[1] |= 0x80U;
    }

    ChangeState(client, E_UDS_CLT_REQUEST_STATE_SENDING);
    PollLowLevel(client); // poll once to begin sending immediately
    return E_UDS_OK;
}

#define PRE_REQUEST_CHECK()                                                                        \
    if (E_UDS_CLT_REQUEST_STATE_IDLE != (UDSClientRequestState_e)client->state)                                             \
    {                                                                                              \
        return E_UDS_ERR_BUSY;                                                                       \
    }                                                                                              \
    ClearRequestContext(client);

/**
 * @name UDSSendECUReset
 * @brief 0x11数据组包发送
 * @param client  UDSClient_t结构体
 * @param type 重启类型
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendECUReset(UDSClient_t *client, uint8_t type)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    PRE_REQUEST_CHECK();
    client->sendBuf[0] = (uint8_t)E_UDS_SID_ECU_RESET;
    client->sendBuf[1] = type;
    client->sendSize = 2U;
    return SendRequest(client);
}

/**
 * @name UDSSendDiagSessCtrl
 * @brief 0x10数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param mode 会话模式
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendDiagSessCtrl(UDSClient_t *client, UDSDiagSessionType_e mode)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    PRE_REQUEST_CHECK();
    client->sendBuf[0] = (uint8_t)E_UDS_SID_DIAGNOSTIC_SESSION_CONTROL;
    client->sendBuf[1] = (uint8_t)mode;
    client->sendSize = 2U;
    return SendRequest(client);
}

/**
 * @name UDSSendClearDtcInfo
 * @brief 0x14数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param groupOfDtc 要清除的DTC组或特定DTC
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendClearDtcInfo(UDSClient_t *client, uint32_t groupOfDtc)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    PRE_REQUEST_CHECK();
    client->sendBuf[0] = (uint8_t)E_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION;
    client->sendBuf[1] = (uint8_t)((groupOfDtc >> 16U) & 0xffU);
    client->sendBuf[2] = (uint8_t)((groupOfDtc >> 8U) & 0xffU);
    client->sendBuf[3] = (uint8_t)(groupOfDtc & 0xffU);
    client->sendSize = UDS_0X14_REQ_MIN_LEN;
    return SendRequest(client);
}

/**
 * @name UDSSendCommCtrl
 * @brief 0x28数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param ctrl 通讯控制类型
 * @param comm 通讯类型
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendCommCtrl(UDSClient_t *client, UDSCommunicationControlType_e ctrl,
                        UDSCommunicationType_e comm)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    PRE_REQUEST_CHECK();
    client->sendBuf[0] = (uint8_t)E_UDS_SID_COMMUNICATION_CONTROL;
    client->sendBuf[1] = (uint8_t)ctrl;
    client->sendBuf[2] = (uint8_t)comm;
    client->sendSize = 3U;
    return SendRequest(client);
}
/**
 * @name UDSSendTesterPresent
 * @brief 0x3e数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param sub 子功能
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendTesterPresent(UDSClient_t *client, uint8_t sub)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    PRE_REQUEST_CHECK();
    client->sendBuf[0] =  (uint8_t)E_UDS_SID_TESTER_PRESENT;
    client->sendBuf[1] = sub;
    client->sendSize = 2U;
    return SendRequest(client);
}

/**
 * @name UDSSendRDBI
 * @brief 0x22数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param didList did列表
 * @param numDataIdentifiers did列表条目
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendRDBI(UDSClient_t *client, const uint16_t *didList,
                     const uint16_t numDataIdentifiers)
{
    if (!client || !didList)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    PRE_REQUEST_CHECK();
    client->sendBuf[0] =  (uint8_t)E_UDS_SID_READ_DATA_BY_IDENTIFIER;
    for (uint32_t i = 0U; i < numDataIdentifiers; i++)
    {
        uint16_t offset = 1U + (uint16_t)sizeof(uint16_t) * (uint16_t)i;
        if (offset + 2U > client->sendBufSize)
        {
            return E_UDS_ERR_INVALID_ARG;
        }
        (client->sendBuf + offset)[0] =  (uint8_t)((didList[i] & 0xFF00U) >> 8U);
        (client->sendBuf + offset)[1] =  (uint8_t)(didList[i] & 0xFFU);
    }
    client->sendSize = 1U + (numDataIdentifiers * (uint16_t)sizeof(uint16_t));
    return SendRequest(client);
}
/**
 * @name UDSSendWDBI
 * @brief 0x2e数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param dataIdentifier did数据标识
 * @param data 写入数据
 * @param size 写入数据长度
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendWDBI(UDSClient_t *client, uint16_t dataIdentifier, const uint8_t *data, uint16_t size)
{

    if (!data || (size == 0U))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    PRE_REQUEST_CHECK();
    client->sendBuf[0] =  (uint8_t)E_UDS_SID_WRITE_DATA_BY_IDENTIFIER;
    if (client->sendBufSize <= 3U || size > client->sendBufSize - 3U)
    {
        return E_UDS_ERR_BUFSIZ;
    }
    client->sendBuf[1] =  (uint8_t)((dataIdentifier & 0xFF00U) >> 8U);
    client->sendBuf[2] =  (uint8_t)(dataIdentifier & 0xFFU);
    (void)memmove(&client->sendBuf[3], data, size);
    client->sendSize = 3U + size;
    return SendRequest(client);
}

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
UDSErr_t UDSSendRoutineCtrl(UDSClient_t *client, RoutineControlType_e type, uint16_t routineIdentifier, const uint8_t *data, uint16_t size)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    PRE_REQUEST_CHECK();
    client->sendBuf[0] =  (uint8_t)E_UDS_SID_ROUTINE_CONTROL;
    client->sendBuf[1] =  (uint8_t)type;
    client->sendBuf[2] =  (uint8_t)(routineIdentifier >> 8U);
    client->sendBuf[3] = (uint8_t)routineIdentifier;

    if (size > 0U)
    {
        if (size > client->sendBufSize - UDS_0X31_REQ_MIN_LEN)
        {
            return E_UDS_ERR_BUFSIZ;
        }
        (void)memmove(&client->sendBuf[UDS_0X31_REQ_MIN_LEN], data, size);
    }
    client->sendSize = UDS_0X31_REQ_MIN_LEN + size;
    return SendRequest(client);
}

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
                                uint8_t addressAndLengthFormatIdentifier, size_t memoryAddress, size_t memorySize)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    PRE_REQUEST_CHECK();
    uint8_t numMemorySizeBytes = (addressAndLengthFormatIdentifier & 0xF0U) >> 4U;
    uint8_t numMemoryAddressBytes = addressAndLengthFormatIdentifier & 0x0FU;

    client->sendBuf[0] = E_UDS_SID_REQUEST_DOWNLOAD;
    client->sendBuf[1] = dataFormatIdentifier;
    client->sendBuf[2] = addressAndLengthFormatIdentifier;

    uint8_t *ptr = &client->sendBuf[UDS_0X34_REQ_BASE_LEN];

    for (int32_t i = (int32_t)numMemoryAddressBytes - 1; i >= 0; i--)
    {
        size_t tmpMemAddr = (memoryAddress & ((size_t)0xFFU << (8U * (size_t)i))) >> (8U * (size_t)i);
        *ptr = (uint8_t)tmpMemAddr;
        ptr++;
    }

    for (int32_t i = (int32_t)numMemorySizeBytes - 1; i >= 0; i--)
    {
        size_t tmpMemorySize = (memorySize & ((size_t)0xFFU << (8U * (size_t)i))) >> (8U * (size_t)i);
        *ptr = (uint8_t)tmpMemorySize;
        ptr++;
    }

    client->sendSize = (uint16_t)UDS_0X34_REQ_BASE_LEN + (uint16_t)numMemoryAddressBytes + (uint16_t)numMemorySizeBytes;
    return SendRequest(client);
}

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
                            uint8_t addressAndLengthFormatIdentifier, size_t memoryAddress, size_t memorySize)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    PRE_REQUEST_CHECK();
    uint8_t numMemorySizeBytes = (addressAndLengthFormatIdentifier & 0xF0U) >> 4U;
    uint8_t numMemoryAddressBytes = addressAndLengthFormatIdentifier & 0x0FU;

    client->sendBuf[0] = E_UDS_SID_REQUEST_UPLOAD;
    client->sendBuf[1] = dataFormatIdentifier;
    client->sendBuf[2] = addressAndLengthFormatIdentifier;

    uint8_t *ptr = &client->sendBuf[UDS_0X35_REQ_BASE_LEN];

    for (int32_t i = (int32_t)numMemoryAddressBytes - 1; i >= 0; i--)
    {
        size_t tmpMemAddr = (memoryAddress & ((size_t)0xFF << (8U * (size_t)i))) >> (8U * (size_t)i);
        *ptr = (uint8_t)tmpMemAddr;
        ptr++;
    }

    for (int32_t i = (int32_t)numMemorySizeBytes - 1; i >= 0; i--)
    {
        size_t tmpMemSize = (memorySize & ((size_t)0xFFU << (8U * (size_t)i))) >> (8U * (size_t)i);
        *ptr = (uint8_t)tmpMemSize;
        ptr++;
    }

    client->sendSize = (uint16_t)UDS_0X35_REQ_BASE_LEN + (uint16_t)numMemoryAddressBytes + (uint16_t)numMemorySizeBytes;
    return SendRequest(client);
}

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
                            const uint16_t blockLength, const uint8_t *data, uint16_t size)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    PRE_REQUEST_CHECK();

    client->sendBuf[0] = E_UDS_SID_TRANSFER_DATA;
    client->sendBuf[1] = blockSequenceCounter;
    (void)memmove(&client->sendBuf[UDS_0X36_REQ_BASE_LEN], data, size);
    LOG_Debug("size: %d, blocklength: %d", size, blockLength);
    client->sendSize = UDS_0X36_REQ_BASE_LEN + size;
    return SendRequest(client);
}

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
                                   const uint16_t blockLength, FILE *fd)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    PRE_REQUEST_CHECK();
    client->sendBuf[0] = E_UDS_SID_TRANSFER_DATA;
    client->sendBuf[1] = blockSequenceCounter;

    uint16_t size = (uint16_t)fread(&client->sendBuf[2], 1U, (size_t)blockLength - 2U, fd);
    LOG_Debug("size: %d, blocklength: %d", size, blockLength);
    client->sendSize = UDS_0X36_REQ_BASE_LEN + size;
    return SendRequest(client);
}

/**
 * @name UDSSendRequestTransferExit
 * @brief 0x37组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param data transferRequestParameterRecord数据
 * @param size transferRequestParameterRecord数据长度
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendRequestTransferExit(UDSClient_t *client, const uint8_t *data, uint16_t size)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    PRE_REQUEST_CHECK();
    client->sendBuf[0] = E_UDS_SID_REQUEST_TRANSFER_EXIT;
    client->sendSize = UDS_0X37_REQ_BASE_LEN;
    if (size > 0U)
    {
        (void)memmove(&client->sendBuf[UDS_0X37_REQ_BASE_LEN], data, size);
    }
    client->sendSize += size;
    return SendRequest(client);
}

/**
 * @name UDSCtrlDTCSetting
 * @brief 0x85组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param dtcSettingType：子功能
 * @param dtcSettingControlOptionRecord：故障码设置控制可选记录
 * @param len:可选记录条目
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSCtrlDTCSetting(UDSClient_t *client, DTCSettingType_e dtcSettingType, uint8_t *data, uint16_t size)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    PRE_REQUEST_CHECK();
    if (0x00U == (uint8_t)dtcSettingType || 0x7FU == (uint8_t)dtcSettingType ||
        (0x03U <= (uint8_t)dtcSettingType && (uint8_t)dtcSettingType <= 0x3FU))
    {
        LOG_Error("%s dtcSettingType error.", __FUNCTION__);
        return E_UDS_ERR;
    }

    client->sendBuf[0] = (uint8_t)E_UDS_SID_CONTROL_DTC_SETTING;
    client->sendBuf[1] = (uint8_t)dtcSettingType;
    if (size > 0U)
    {
        if (size > client->sendBufSize - 2U)
        {
            return E_UDS_ERR_BUFSIZ;
        }
        (void)memmove(&client->sendBuf[2], data, size);
        
    }
    client->sendSize = UDS_0X85_REQ_BASE_LEN + size;
    return SendRequest(client);
}

/**
 * @name UDSSendSecurityAccess
 * @brief 0x27数据组包发送
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param level 安全级别
 * @param data 安全seed或key
 * @param size data长度
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSSendSecurityAccess(UDSClient_t *client, uint8_t level, uint8_t *data, uint16_t size)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    PRE_REQUEST_CHECK();
    client->sendBuf[0] = E_UDS_SID_SECURITY_ACCESS;
    client->sendBuf[1] = level;
    if (size > 0U)
    {
        if (!data)
        {
            LOG_Error("%s param invalid.", __FUNCTION__);
            return E_UDS_ERR;
        }
        if (size > client->sendBufSize - UDS_0X27_REQ_BASE_LEN)
        {
            return E_UDS_ERR_BUFSIZ;
        }
        (void)memmove(&client->sendBuf[UDS_0X27_REQ_BASE_LEN], data, size);        
    }

    client->sendSize = UDS_0X27_REQ_BASE_LEN + size;
    return SendRequest(client);
}

typedef struct {
    uint8_t dataFormatIdentifier;
    uint8_t addressAndLengthFormatIdentifier;
    size_t memoryAddress;
    size_t memorySize;
    FILE *fd;
    uint8_t blockSequenceCounter;
    uint16_t blockLength;
} UDSClientDownloadSequence_t;

/**
 * @name RequestDownload
 * @brief 请求下载, 向服务端发送请求下载的指令，启动下载流程。
 * @param client 指向UDS客户端结构体的指针，用于获取和发送下载请求的数据。
 * @return 返回下载流程的当前状态，如果客户端指针为空则返回错误状态。
 */
static UDSSeqState_t RequestDownload(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    const UDSClientDownloadSequence_t *plSeq = (UDSClientDownloadSequence_t *)client->cbData;
    (void)UDSSendRequestDownload(client, plSeq->dataFormatIdentifier,
                           plSeq->addressAndLengthFormatIdentifier, plSeq->memoryAddress,
                           plSeq->memorySize);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

/**
 * @name CheckRequestDownloadResponse
 * @brief 检查请求下载响应的状态
 *        本函数用于处理从服务器接收的请求下载响应，根据响应内容更新客户端状态，并决定下一步的动作。
 * @param client 指向UDS客户端结构体的指针，用于获取和更新客户端的状态及数据。
 * @return 返回UDS序列状态，根据处理结果决定是继续、结束还是出现错误。
 */
static UDSSeqState_t CheckRequestDownloadResponse(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    UDSClientDownloadSequence_t *plSeq = (UDSClientDownloadSequence_t *)client->cbData;
    RequestDownloadResponse_t resp = {0};
    UDSErr_t err = UDSUnpackRequestDownloadResponse(client, &resp);
    if (0 != err)
    {
        client->err = err;
        return E_UDS_SEQ_STATE_DONE;
    }
    plSeq->blockLength = (uint16_t)resp.maxNumberOfBlockLength;
    if (0U == resp.maxNumberOfBlockLength)
    {
        client->err = E_UDS_ERR;
        return E_UDS_SEQ_STATE_DONE;
    }
    return E_UDS_SEQ_STATE_GOTONEXT;
}

/**
 * @name PrepareToTransfer
 * @brief 准备进行传输的函数,初始化与传输相关的序列状态。
 * @param client 指向UDS客户端结构体的指针，用于获取或设置客户端相关数据。
 * @return 返回UDS序列状态，如果操作成功返回E_UDS_SEQ_STATE_GOTONEXT，如果客户端指针无效返回E_UDS_ERR。
 */
static UDSSeqState_t PrepareToTransfer(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    UDSClientDownloadSequence_t *plSeq = (UDSClientDownloadSequence_t *)client->cbData;
    plSeq->blockSequenceCounter = 1;
    return E_UDS_SEQ_STATE_GOTONEXT;
}

/**
 * @name TransferData
 * @brief 传输数据函数
 * @param client 指向UDS客户端结构体的指针，包含传输所需的客户端信息和状态。
 * @return UDSSeqState_t 函数返回一个表示数据传输状态的枚举值，
 *         可能的返回值包括传输完成、进行中或发生错误。
 */
static UDSSeqState_t TransferData(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    UDSClientDownloadSequence_t *plSeq = (UDSClientDownloadSequence_t *)client->cbData;
    if (E_UDS_CLT_REQUEST_STATE_IDLE == (UDSClientRequestState_e)client->state)
    {
        if (0 != ferror(plSeq->fd))
        {
            (void)fclose(plSeq->fd);
            client->err = E_UDS_ERR_FILE_IO; // 读取文件故障
            return E_UDS_SEQ_STATE_DONE;
        } 
        else if (0 != feof(plSeq->fd))
        { // 传完了
            return E_UDS_SEQ_STATE_GOTONEXT;
        }
        else
        {
            (void)UDSSendTransferDataStream(client, plSeq->blockSequenceCounter++, plSeq->blockLength,
                                      plSeq->fd);
        }
    }
    return E_UDS_SEQ_STATE_RUNNING;
}

/**
 * @name RequestTransferExit
 * @brief 请求退出传输状态
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @return 返回UDS序列状态枚举值E_UDS_SEQ_STATE_GOTONEXT，表示执行此操作后将进入下一个状态。
 */
static UDSSeqState_t RequestTransferExit(UDSClient_t *client)
{
    (void)UDSSendRequestTransferExit(client, NULL, 0);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

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
                        uint8_t addressAndLengthFormatIdentifier, size_t memoryAddress, size_t memorySize, FILE *fd)
{
    if (!client || !fd)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    static const UDSClientCallback callbacks[] =
    {
        RequestDownload, UDSClientAwaitIdle,  CheckRequestDownloadResponse, PrepareToTransfer,
        TransferData,    RequestTransferExit, UDSClientAwaitIdle, NULL
    };
    static UDSClientDownloadSequence_t seq = {0};
    (void)memset(&seq, 0, sizeof(seq));
    seq.blockSequenceCounter = 1;
    seq.dataFormatIdentifier = dataFormatIdentifier;
    seq.addressAndLengthFormatIdentifier = addressAndLengthFormatIdentifier;
    seq.memoryAddress = memoryAddress;
    seq.memorySize = memorySize;
    seq.fd = fd;
    client->cbList = callbacks;
    client->cbIdx = 0;
    client->cbData = &seq;
    return E_UDS_OK;
}

/**
 * @name UDSUnpackSecurityAccessResponse
 * @brief 解包0x27响应
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param resp 0x27响应
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSUnpackSecurityAccessResponse(const UDSClient_t *client, SecurityAccessResponse_t *resp)
{
    if (!client || !resp)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    if (UDS_RESPONSE_SID_OF((uint8_t)E_UDS_SID_SECURITY_ACCESS) != client->recvBuf[0])
    {
        return E_UDS_ERR_SID_MISMATCH;
    }
    if (client->recvSize < UDS_0X27_RESP_BASE_LEN)
    {
        return E_UDS_ERR_RESP_TOO_SHORT;
    }
    resp->securityAccessType = client->recvBuf[1];
    resp->securitySeedLength = client->recvSize - UDS_0X27_RESP_BASE_LEN;
    resp->securitySeed = (resp->securitySeedLength == 0U) ? NULL : &client->recvBuf[2];
    return E_UDS_OK;
}

/**
 * @name UDSUnpackRoutineControlResponse
 * @brief 解包0x31响应
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param resp 0x31响应
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSUnpackRoutineControlResponse(const UDSClient_t *client, RoutineControlResponse_t *resp)
{
    if (!client || !resp)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    
    if (UDS_RESPONSE_SID_OF((uint8_t)E_UDS_SID_ROUTINE_CONTROL) != client->recvBuf[0])
    {
        return E_UDS_ERR_SID_MISMATCH;
    }
    if (client->recvSize < UDS_0X31_RESP_MIN_LEN)
    {
        return E_UDS_ERR_RESP_TOO_SHORT;
    }
    resp->routineControlType = client->recvBuf[1];
    resp->routineIdentifier = ((uint16_t)client->recvBuf[2] << 8U) + client->recvBuf[3];
    resp->routineStatusRecordLength = client->recvSize - UDS_0X31_RESP_MIN_LEN;
    resp->routineStatusRecord =
        (resp->routineStatusRecordLength == 0U) ? NULL : &client->recvBuf[UDS_0X31_RESP_MIN_LEN];
    return E_UDS_OK;
}

/**
 * @name UDSUnpackRequestDownloadResponse
 * @brief 解包0x34响应
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @param resp 0x34响应
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSUnpackRequestDownloadResponse(const UDSClient_t *client, RequestDownloadResponse_t *resp)
{
    if (!client || !resp)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    
    if (UDS_RESPONSE_SID_OF((uint8_t)E_UDS_SID_REQUEST_DOWNLOAD) != client->recvBuf[0])
    {
        return E_UDS_ERR_SID_MISMATCH;
    }
    if (client->recvSize < UDS_0X34_RESP_BASE_LEN)
    {
        return E_UDS_ERR_RESP_TOO_SHORT;
    }
    uint8_t maxNumberOfBlockLengthSize = (client->recvBuf[1] & 0xF0U) >> 4U;

    if (sizeof(resp->maxNumberOfBlockLength) < maxNumberOfBlockLengthSize)
    {
        LOG_Debug("WARNING: sizeof(maxNumberOfBlockLength) > sizeof(size_t)");
        return E_UDS_ERR;
    }
    resp->maxNumberOfBlockLength = 0;
    for (uint8_t byteIdx = 0; byteIdx < maxNumberOfBlockLengthSize; byteIdx++)
    {
        uint8_t byte = client->recvBuf[UDS_0X34_RESP_BASE_LEN + byteIdx];
        uint8_t shiftBytes = maxNumberOfBlockLengthSize - 1U - byteIdx;
        resp->maxNumberOfBlockLength |= (size_t)byte << (8U * shiftBytes);
    }
    return E_UDS_OK;
}

/**
 * name UDSClientPoll
 * @brief client轮询函数
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @return 空闲返回UDS_CLIENT_IDLE, 其他返回UDS_CLIENT_RUNNING
 */
bool UDSClientPoll(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return (bool)UDS_CLIENT_IDLE;
    }
    PollLowLevel(client);

    if (0 != client->err)
    {
        return (bool)UDS_CLIENT_IDLE;
    }

    if (E_UDS_CLT_REQUEST_STATE_IDLE != (UDSClientRequestState_e)client->state)
    {
        return (bool)UDS_CLIENT_RUNNING;
    }

    if (NULL == client->cbList)
    {
        return (bool)UDS_CLIENT_IDLE;
    }

    UDSClientCallback activeCallback = client->cbList[client->cbIdx];

    if (NULL == activeCallback)
    {
        return (bool)UDS_CLIENT_IDLE;
    }

    UDSSeqState_t state = activeCallback(client);

    int32_t ret = UDS_CLIENT_IDLE;
    switch ((UDSSeqState_e)state)
    {
    case E_UDS_SEQ_STATE_DONE:
        ret = UDS_CLIENT_IDLE;
        break;
    case E_UDS_SEQ_STATE_RUNNING:
        ret = UDS_CLIENT_RUNNING;
        break;
    case E_UDS_SEQ_STATE_GOTONEXT:
    {
        client->cbIdx += 1U;
        ret = UDS_CLIENT_RUNNING;
        break;
    }
    default:
        ret = UDS_CLIENT_IDLE;
        break;
    }

    return (bool)ret;
}

/**
 * @name UDSClientAwaitIdle
 * @brief 请求发送后等待响应
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @return UDSSeqState_e定义的枚举值
 */
UDSSeqState_t UDSClientAwaitIdle(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    if (0 != client->err)
    {
        return E_UDS_SEQ_STATE_DONE;
    }

    if (E_UDS_CLT_REQUEST_STATE_IDLE == (UDSClientRequestState_e)client->state)
    {
        return E_UDS_SEQ_STATE_GOTONEXT;
    }

    return E_UDS_SEQ_STATE_RUNNING;
}

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
UDSErr_t UDSUnpackRDBIResponse(const UDSClient_t *client, uint16_t did, uint8_t *data, uint16_t size, uint16_t *offset)
{
    if (!client || !data || !offset)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    if (0U == *offset)
    {
        *offset = UDS_0X22_RESP_BASE_LEN;
    }

    if (*offset + sizeof(did) > client->recvSize)
    {
        return E_UDS_ERR_RESP_TOO_SHORT;
    }

    uint16_t theirDID = ((uint16_t)client->recvBuf[*offset] << 8U) + (uint16_t)client->recvBuf[*offset + 1U];
    if (theirDID != did)
    {
        return E_UDS_ERR_DID_MISMATCH;
    }

    if (*offset + sizeof(uint16_t) + size > client->recvSize)
    {
        return E_UDS_ERR_RESP_TOO_SHORT;
    }

    (void)memmove(data, &client->recvBuf[*offset + sizeof(uint16_t)], size);

    *offset += (uint16_t)sizeof(uint16_t) + size;
    return E_UDS_OK;
}

/**
 * @name UDSKeepSession
 * @brief 保持会话
 * @param client 指向UDS客户端结构体的指针，用于标识当前通信客户端。
 * @return UDSErr_e定义的枚举值
 */
UDSErr_t UDSKeepSession(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    ssize_t ret = 0;
    uint8_t data[] = {0x3e, 0x80};
    ret = client->tp->send(client->tp, data, sizeof(data), E_UDS_TP_ADDR_TYPE_FUNCTION);
    if ((size_t)ret == sizeof(data))
    {
        return E_UDS_OK;
    }
    else
    {
        LOG_Error("send fail...");
        return E_UDS_ERR;
    }   
}