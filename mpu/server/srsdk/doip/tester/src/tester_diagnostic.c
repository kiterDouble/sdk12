#include <errno.h>
#include "tester_diagnostic.h"
#include "ring_buffer.h"

typedef struct
{
    uint32_t isRouteActive;
    pthread_mutex_t mutex;
    int32_t fds[2];
    struct event *local;
    struct event *ev;
    DoipTesterCbs_t funs;
    DoipTesterNode_t node;
    RingBuffer_t rBuf;
}TesterDiagPriv_t;

/**
 * @name TesterDiagPackageProc
 * @brief 测试设备数据包处理回调
 * @param priv 私有数据
 * @param data 数据
 * @param dataLen 数据长度
 * @return 函数返回值
 */
typedef int32_t (*TesterDiagPackageProc)(TesterDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen);

typedef struct
{
    uint32_t payloadType;
    TesterDiagPackageProc procCb;
}TesterDiagProc_t;

/**
 * @name TesterDiagWrite
 * @brief 测试设备诊断写入
 *        该函数用于通过指定的诊断通道写入数据。此功能可用于向设备发送诊断命令或数据。
 * @param priv 指向TesterDiagPriv_t结构体的指针，包含诊断写作所需的私有数据。
 * @param data 指向要写入数据的缓冲区的指针。
 * @param dataLen 要写入的数据长度。
 * @return int32_t 返回函数执行状态。RET_OK表示成功，RET_ERR表示错误。
 */
static int32_t TesterDiagWrite(TesterDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    int32_t retVal = write(event_get_fd(priv->ev), data, dataLen);
    LOG_Debug("%s retVal:%d.", __FUNCTION__, retVal);
    ret = RET_OK;
    return ret;
}

/**
 * @name TesterDiagSendError
 * @brief 向指定的套接字发送doip 0000消息。
 * @param fd 文件描述符，用于标识接收错误信息的套接字。
 * @param err 错误码
 * @return 函数无返回值
 */
static void TesterDiagSendError(evutil_socket_t fd, uint32_t err)
{
    uint8_t package[128] = {0x0,};
    uint32_t pLen = sizeof(package);
    GeneralHeaderNack_t ghNack;
    ghNack.nack = err;

    DoipGenMessage0000(&ghNack, package, &pLen);
    int32_t retVal = write(fd, package, pLen);
    LOG_Debug("%s retVal:%d.", __FUNCTION__, retVal);
}

/**
 * @name TesterDiagNackResProc
 * @brief 测试设备诊断NACK处理函数
 *        本函数用于解析并处理来自测试设备的诊断NACK响应。首先，它会验证输入参数的有效性，
 *        然后使用DoipParserMessage0000函数解析传入的数据。如果解析成功，将记录NACK代码并返回成功状态。
 * @param priv 指向TesterDiagPriv_t结构体的指针，包含测试设备的私有数据。
 * @param data 指向需要解析的数据的指针。
 * @param dataLen 数据长度。
 * @return 函数执行状态，RET_OK表示成功，其他值表示失败。
 */
static int32_t TesterDiagNackResProc(TesterDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    GeneralHeaderNack_t ghNack;
    int32_t retVal = DoipParserMessage0000(data, dataLen, &ghNack);
    if (RET_OK != retVal)
    {
        LOG_Error("%s DoipParserMessage0000 fail.", __FUNCTION__);
        return ret;
    }
    LOG_Debug("%s,nack:%d.", __FUNCTION__, ghNack.nack);
    ret = RET_OK;
    return ret;
}

/**
 * @name TesterDiagRoutingActivationResProc
 * @brief 路由激活结果处理函数
 *        本函数用于处理路由的激活结果。根据传入的数据解析结果，更新路由状态，并根据激活结果调用相应的连接回调函数。
 * @param priv 指向TesterDiagPriv_t结构体的指针，包含测试设备的私有数据和功能函数指针。
 * @param data 指向包含数据的缓冲区的指针。
 * @param dataLen 数据缓冲区的长度。
 * @return 返回处理结果，RET_OK表示成功，其他值表示失败。
 */
static int32_t TesterDiagRoutingActivationResProc(TesterDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    RoutineActivationRes_t res;

    int32_t retVal =DoipParserMessage0006(data, dataLen, &res);
    if (RET_OK != retVal)
    {
        LOG_Error("%s DoipParserMessage0006 fail.", __FUNCTION__);
        return ret;
    }

    if(res.code == E_RARC_SUCCESSFULLY_ACTIVATED)
    {
        priv->isRouteActive = 1;
        ret = RET_OK;
        priv->funs.conFunc(priv->node.targetLa, E_DOIP_TESTER_CONNECT_SUCCESS);
    }
    else
    {
        priv->isRouteActive = 0;
        int32_t fd = event_get_fd(priv->ev);
        event_del(priv->ev);
        event_free(priv->ev);
        priv->ev = NULL;
        shutdown(fd, SHUT_RDWR);
        close(fd);        
        priv->funs.conFunc(priv->node.targetLa, E_DOIP_TESTER_CONNECT_FAIL);
    }
    
    return ret;
}

/**
 * @name TesterDiagAliveCheckReqProc
 * @brief 诊断存活检查请求处理函数
 * @param priv 指向TesterDiagPriv_t结构体的指针，包含私有诊断数据
 * @param data 指向请求数据的指针
 * @param dataLen 请求数据的长度
 * @return 函数执行状态，RET_OK表示成功，其他值表示失败
 */
static int32_t TesterDiagAliveCheckReqProc(TesterDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    int32_t retVal = DoipParserMessage0007(data, dataLen);
    if (RET_OK != retVal)
    {
        LOG_Error("%s DoipParserMessage0007 fail.", __FUNCTION__);
        return ret;
    }

    uint8_t package[128] = {0x0,};
    uint32_t packageLen = sizeof(package);
    AliveCheckRes_t resPL;
    resPL.sourceAddr[0] = (priv->node.sourceLa >> 8) & 0xff;
    resPL.sourceAddr[1] = priv->node.sourceLa & 0xff;
    DoipGenMessage0008(&resPL, package, &packageLen);
    ret = TesterDiagWrite(priv, package, packageLen);
    return ret;
}

/**
 * @name TesterDiagDiagnosticProc
 * @brief 测试设备诊断处理函数。
 * @param priv 指向TesterDiagPriv_t结构体的指针，包含测试设备的私有数据。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据的长度。
 * @return  返回处理结果，RET_OK表示成功，RET_ERR表示失败。
 */
static int32_t TesterDiagDiagnosticProc(TesterDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    int32_t retVal = DoipParserMessage8001(data, dataLen);
    if (RET_OK != retVal)
    {
        LOG_Error("%s DoipParserMessage8001 fail.", __FUNCTION__);
        return ret;
    }

    DoipPacket_t *packet = (DoipPacket_t *) data;
    DiagnosticMessage_t *diagPL = (DiagnosticMessage_t*)packet->payload;
    uint32_t entityLa = BYTES_TO_UINT16(diagPL->sourceAddr[0], diagPL->sourceAddr[1]);
    uint32_t testerLa = BYTES_TO_UINT16(diagPL->targetAddr[0], diagPL->targetAddr[1]);

    if (testerLa != priv->node.sourceLa)
    {
        ret = RET_ERR;
        LOG_Error("%s addr check fail.targetLa:%d.", __FUNCTION__, testerLa);
        return ret;
    }
    uint32_t len = BYTES_TO_UINT32(packet->length[0], packet->length[1], packet->length[2], packet->length[3]);
    priv->funs.rxFunc(entityLa, diagPL->userdata, len - sizeof(DiagnosticMessage_t));
    ret = RET_OK;
    return ret;
}

/**
 * @name TesterDiagDiagnosticAckProc
 * @brief 诊断确认消息处理函数。
 * @param priv 指向TesterDiagPriv_t结构体的指针，包含测试设备的私有数据和功能。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据的长度。
 * @return 返回处理结果，RET_OK表示成功，其他值表示失败。
 */
static int32_t TesterDiagDiagnosticAckProc(TesterDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    int32_t retVal = DoipParserMessage8002(data, dataLen);
    if (RET_OK != retVal)
    {
        LOG_Error("%s DoipParserMessage8002 fail.", __FUNCTION__);
        return ret;
    }

    DoipPacket_t *packet = (DoipPacket_t *) data;
    DiagnosticMessageAck_t *diagAckPL = (DiagnosticMessageAck_t*)packet->payload;
    uint32_t entityLa = BYTES_TO_UINT16(diagAckPL->sourceAddr[0], diagAckPL->sourceAddr[1]);
    LOG_Debug("%s code:%d.", __FUNCTION__, diagAckPL->code);
    priv->funs.txFunc(entityLa, 0);
    ret = RET_OK;
    return ret;
}

/**
 * @name TesterDiagDiagnosticNackProc
 * @brief 诊断否定响应处理函数。
 * @param priv 指向TesterDiagPriv_t结构体的指针，包含测试设备的私有功能和状态。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据的长度。
 * @return 返回处理结果，RET_OK表示成功，其他值表示失败。
 */
static int32_t TesterDiagDiagnosticNackProc(TesterDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    int32_t retVal = DoipParserMessage8003(data, dataLen);
    if (RET_OK != retVal)
    {
        LOG_Error("%s DoipParserMessage8003 fail.", __FUNCTION__);
        return ret;
    }

    DoipPacket_t *packet = (DoipPacket_t *) data;
    DiagnosticMessageNack_t *diagNackPL = (DiagnosticMessageNack_t*)packet->payload;
    uint32_t entityLa = BYTES_TO_UINT16(diagNackPL->sourceAddr[0], diagNackPL->sourceAddr[1]);

    priv->funs.txFunc(entityLa, 1);
    LOG_Debug("%s code:%d.", __FUNCTION__, diagNackPL->code);
    ret = RET_OK;
    return ret;
}


/**
 * @name TesterDiagLocalReadCb
 * @brief 诊断local读取事件处理函数
 *        此函数被用于从指定的文件描述符中读取数据，并将读取到的数据写入到与事件相关联的文件描述符中。
 * @param fd 文件描述符，用于读取数据。
 * @param events 事件类型，指示读取操作完成或发生错误。
 * @param arg 用户自定义参数，通常为指向 TesterDiagPriv_t 结构体的指针，用于访问私有数据。
 * @return 函数无返回值
 */
static void TesterDiagLocalReadCb(evutil_socket_t fd, short events, void *arg)
{
    TesterDiagPriv_t *priv = (TesterDiagPriv_t *) arg;

    if(!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    uint8_t bytes[0x1000] = {0x0,};
    int32_t readn = 0;

    readn = read(fd, bytes, sizeof(bytes));
    if(readn > 0)
    {
        if (!priv->ev)
        {
            return;
        }
        int32_t retVal = write(event_get_fd(priv->ev), bytes, readn);
        LOG_Debug("%s retVal:%d.", __FUNCTION__, retVal);
    }
}

/**
 * @name TesterDiagRequestProc
 * @brief 处理测试设备诊断请求的进程。
 * @param priv 指向TesterDiagPriv_t结构体的指针，包含诊断过程的私有数据。
 * @return 函数无返回值
 */
static void TesterDiagRequestProc(TesterDiagPriv_t *priv)
{
    static const TesterDiagProc_t procCbs[] = 
    {
        {E_PL_TYPE_HEADER_NEGTIVE_ACK, TesterDiagNackResProc},
        {E_PL_TYPE_ROUTING_ACTIVE_RES, TesterDiagRoutingActivationResProc},
        {E_PL_TYPE_ALIVE_CHECK_REQ, TesterDiagAliveCheckReqProc},
        {E_PL_TYPE_DIAGNOSTIC_REQ, TesterDiagDiagnosticProc},
        {E_PL_TYPE_DIAG_POSITIVE_ACK_RES, TesterDiagDiagnosticAckProc},
        {E_PL_TYPE_DIAG_NEGATIVE_ACK_RES, TesterDiagDiagnosticNackProc}
    };

    if(!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    uint32_t avaiSize = 0;
    DoipMsgHeader_t header;
    uint8_t bytes[sizeof(DoipPacket_t)];

    RingBufferGetAvailableSize(priv->rBuf, &avaiSize);
    RingBufferPeek(priv->rBuf, bytes, sizeof(bytes));
    if (RET_ERR == DoipParserHeader(bytes, sizeof(bytes), &header))
    {
        LOG_Error("%s DoipParserHeader fail.", __FUNCTION__);
        int32_t status = E_DOIP_TESTER_CONNECT_FAIL;
        if (priv->isRouteActive)
        {
            status = E_DOIP_TESTER_DISCONECTED;
        }
        priv->isRouteActive = 0;
        int32_t fd = event_get_fd(priv->ev);
        event_del(priv->ev);
        event_free(priv->ev);
        priv->ev = NULL;
        shutdown(fd, SHUT_RDWR);
        close(fd);        
        priv->funs.conFunc(priv->node.targetLa, status);        
        return;
    }
    
    if (avaiSize < (header.payloadLen + sizeof(bytes)))
    {
        LOG_Debug("%s have not recv enough data.", __FUNCTION__);
        return;
    }

    uint32_t packageLen = header.payloadLen + sizeof(bytes);
    uint8_t *package = (uint8_t *)calloc(1, packageLen);
    if (!package)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return;
    }

    RingBufferDequeue(priv->rBuf, package, packageLen);
    uint32_t i = 0;   
    uint32_t err;

    if (RET_ERR == DoipCheckHeader(package, packageLen, &err))
    {
        TesterDiagSendError(event_get_fd(priv->ev), err);

        if (E_GDHNAE_INCORRECT_PATTERN_FORMAT ==  err)
        {
            int32_t status = E_DOIP_TESTER_CONNECT_FAIL;
            if (priv->isRouteActive)
            {
                status = E_DOIP_TESTER_DISCONECTED;
            }
            priv->isRouteActive = 0;
            int32_t fd = event_get_fd(priv->ev);
            event_del(priv->ev);
            event_free(priv->ev);
            priv->ev = NULL;
            shutdown(fd, SHUT_RDWR);
            close(fd);        
            priv->funs.conFunc(priv->node.targetLa, status);
        }
        return;
    }

    for (; i < sizeof(procCbs) / sizeof(TesterDiagProc_t); i++)
    {
        if (header.payloadType == procCbs[i].payloadType)
        {
            procCbs[i].procCb(priv, package, packageLen);
            free(package);
            return;
        }
    }

    TesterDiagSendError(event_get_fd(priv->ev), E_GDHNAE_UNKNOWN_PAYLOAD_TYPE);
    free(package);
}

/**
 * @name TesterDiagReadn
 * @brief 从指定的文件描述符中读取数据到缓冲区
 * @param fd 文件描述符
 * @param data 用于存储读取数据的缓冲区指针
 * @param datalen 要读取的数据长度
 * @return 读取成功返回RET_OK，否则返回RET_ERR
 */
static int32_t TesterDiagReadn(int32_t fd, uint8_t *data, uint32_t datalen)
{
    size_t nleft = 0;
    ssize_t readn = 0;
    uint8_t *ptr = NULL;  
    int32_t ret = RET_ERR;

    if (!data || (datalen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
 
    ptr = data;
    nleft = datalen;
 
    while (nleft > 0)
    {
        readn = read (fd, ptr, nleft);

        if (readn > 0)
        {
            nleft -= readn;
            ptr += readn;
        }
        else
        {
            if((readn < 0) && (errno == EINTR))
            {
                continue;
            }

            LOG_Error("%s read error.readn:%d", __FUNCTION__, readn);
            break;            
        }
    }
    
    if (0 == nleft)
    {
        ret = RET_OK;
    }

    return ret;
}

/**
 * @name TesterDiagReadCb
 * @brief 测试设备诊断读取回调函数
 *        此函数被用于处理诊断消息的读取，首先尝试读取报文头，若成功则继续读取报文负载。
 *        如果读取过程中发生错误，则断开连接并调用连接回调函数。
 * @param fd 文件描述符，用于网络通信的数据报文读取
 * @param events 事件类型，表明了触发回调的原因（如读就绪）
 * @param arg 用户自定义参数，此处为TesterDiagPriv_t结构体指针，用于访问私有数据
 * @return 函数无返回值
 */
static void TesterDiagReadCb(evutil_socket_t fd, short events, void *arg)
{
    TesterDiagPriv_t *priv = (TesterDiagPriv_t *) arg;

    if(!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }
    uint8_t bytes[sizeof(DoipPacket_t)] = {0x0,};

    if (DOIP_OK != TesterDiagReadn(fd, bytes, sizeof(bytes)))
    {
        LOG_Error("%s read header fail.", __FUNCTION__);
            int32_t status = E_DOIP_TESTER_CONNECT_FAIL;
            if (priv->isRouteActive)
            {
                status = E_DOIP_TESTER_DISCONECTED;
            }
            priv->isRouteActive = 0;
            int32_t fd = event_get_fd(priv->ev);
            event_del(priv->ev);
            event_free(priv->ev);
            priv->ev = NULL;
            shutdown(fd, SHUT_RDWR);
            close(fd);        
            priv->funs.conFunc(priv->node.targetLa, status);
        return;
    }

    RingBufferEnqueue(priv->rBuf, bytes, sizeof(bytes));

    DoipMsgHeader_t header;
    uint8_t offset = 0;

    DoipParserHeader(bytes, sizeof(bytes), &header);
    while(offset < header.payloadLen)
    {
        uint8_t buffer[1024] = {0x0};
        int32_t readn = sizeof(buffer);

        if (readn > (header.payloadLen - offset))
        {
            readn = header.payloadLen - offset;
        }

        if (DOIP_OK != TesterDiagReadn(fd, buffer, readn))
        {
            LOG_Error("%s read payload fail.", __FUNCTION__);
            int32_t status = E_DOIP_TESTER_CONNECT_FAIL;
            if (priv->isRouteActive)
            {
                status = E_DOIP_TESTER_DISCONECTED;
            }
            priv->isRouteActive = 0;
            int32_t fd = event_get_fd(priv->ev);
            event_del(priv->ev);
            event_free(priv->ev);
            priv->ev = NULL;
            shutdown(fd, SHUT_RDWR);
            close(fd);        
            priv->funs.conFunc(priv->node.targetLa, status);
            return;
        }

        RingBufferEnqueue(priv->rBuf, buffer, readn);
        offset += readn;
    }

    TesterDiagRequestProc(priv);
}

/**
 * @name TesterDiagRouteActivationRequest
 * @brief 发起诊断路由激活请求
 *        此函数用于生成并发送一个诊断路由激活的请求。它首先会检查输入的私有结构指针是否有效，
 *        然后基于该结构中的信息填充请求数据，并通过调用TesterDiagWrite函数来发送这个请求。
 * @param priv 指向TesterDiagPriv_t私有结构的指针，包含请求所需的所有信息。
 * @return 返回函数执行的状态码。成功时返回RET_OK，错误时返回RET_ERR。
 */
static int32_t TesterDiagRouteActivationRequest(TesterDiagPriv_t *priv)
{
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t package[1024];
    uint32_t packageLen = sizeof(package);
    RoutineActivationReq_t reqPL;
    memset(&reqPL, 0x0, sizeof(RoutineActivationReq_t));
    reqPL.sourceAddr[0] = (priv->node.sourceLa >> 8) & 0xff;
    reqPL.sourceAddr[1] = priv->node.sourceLa & 0xff;
    reqPL.activeType = priv->node.activeType;

    DoipGenMessage0005(&reqPL, package, &packageLen);
    return TesterDiagWrite(priv, package, packageLen);
}

/**
 * @name TesterDiagConnect
 * @brief 建立诊断连接
 * @param hdl 指向TesterDiagHandle_t的指针，用于返回诊断会话的句柄
 * @param base 指向event_base的指针，用于事件基础结构
 * @param node 指向DoipTesterNode_t的指针，包含服务器的节点信息
 * @param funs 指向DoipTesterCbs_t的指针，包含回调函数
 * @return  返回函数执行的状态码。成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t TesterDiagConnect(TesterDiagHandle_t *hdl, 
                        struct event_base *base, 
                        const DoipTesterNode_t *node, 
                        const DoipTesterCbs_t *funs)
{
    TesterDiagPriv_t *priv;
    int32_t ret = RET_ERR;

    if (!hdl || !base || !node || !funs)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    priv = (TesterDiagPriv_t *)calloc(1, sizeof(TesterDiagPriv_t));
    if (!priv)
    {
        LOG_Error("%s priv calloc fail.", __FUNCTION__);
        return ret;
    }
    
    if (0 != pthread_mutex_init(&priv->mutex, NULL))
    {
        LOG_Error("%s pthread_mutex_init fail.", __FUNCTION__);
        free(priv);
        return ret;
    }

    memcpy(&priv->node, node, sizeof(DoipTesterNode_t));
    memcpy(&priv->funs, funs, sizeof(DoipTesterCbs_t));

    if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, priv->fds)) 
    {
        LOG_Error("%s socketpair fail.", __FUNCTION__);
        pthread_mutex_destroy(&priv->mutex);
        free(priv);
        return ret;
    }

    priv->local = event_new((struct event_base *)base, priv->fds[1], EV_READ | EV_PERSIST, TesterDiagLocalReadCb, (void *)priv);
    if (!priv->local || (event_add(priv->local, NULL) < 0))
    {
        if (priv->local)
        {
            event_del(priv->local);
            event_free(priv->local);
            priv->local = NULL;
        }
        close(priv->fds[0]);
        close(priv->fds[1]);
        pthread_mutex_destroy(&priv->mutex);
        free(priv);
        return ret;
    }

    int32_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        LOG_Error("%s socket() fail.", __FUNCTION__);
        event_del(priv->local);
        event_free(priv->local);
        priv->local = NULL;
        close(priv->fds[0]);
        close(priv->fds[1]);
        pthread_mutex_destroy(&priv->mutex);
        free(priv);
        return ret;
    }

    // 设置TCP_NODELAY选项
    int32_t flag = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&flag, sizeof(int32_t)) == -1)
    {
        LOG_Error("%s setsockopt() fail.", __FUNCTION__);
        event_del(priv->local);
        event_free(priv->local);
        priv->local = NULL;
        close(priv->fds[0]);
        close(priv->fds[1]);
        close(fd);
        pthread_mutex_destroy(&priv->mutex);
        free(priv);
        return ret;
    }
   // 设置服务器地址
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DOIP_PORT);
    inet_pton(AF_INET, (void*)node->ipAddr, &serverAddr.sin_addr);

    // 阻塞模式下连接到服务器
    if (connect(fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        LOG_Error("%s connect() fail.", __FUNCTION__);
        event_del(priv->local);
        event_free(priv->local);
        priv->local = NULL;
        close(priv->fds[0]);
        close(priv->fds[1]);
        close(fd);
        pthread_mutex_destroy(&priv->mutex);
        free(priv);
        return ret;
    }

    // 创建一个事件用于读取数据
    priv->ev = event_new(base, fd, EV_READ | EV_PERSIST, TesterDiagReadCb, (void *)priv);
    if (!priv->ev || (event_add(priv->ev, NULL) < 0))
    {
        if (priv->ev)
        {
            event_del(priv->ev);
            event_free(priv->ev);
            priv->ev = NULL;
        }
        event_del(priv->local);
        event_free(priv->local);
        priv->local = NULL;
        close(priv->fds[0]);
        close(priv->fds[1]);
        close(fd);
        pthread_mutex_destroy(&priv->mutex);
        free(priv);
        return ret;
    }
    
    uint32_t bufSize = node->maxPL + sizeof(DoipPacket_t);
    bufSize = ((bufSize / 1024) + 1) * 1024;//k bytes align.
    if (0 != RingBufferCreate(&priv->rBuf, bufSize))
    {
        LOG_Error("%s RingBufferCreate() fail.", __FUNCTION__);
        event_del(priv->ev);
        event_free(priv->ev);
        priv->ev = NULL;
        event_del(priv->local);
        event_free(priv->local);
        priv->local = NULL;
        close(priv->fds[0]);
        close(priv->fds[1]);
        close(fd);
        pthread_mutex_destroy(&priv->mutex);
        free(priv);
        return ret;
    }

    if (RET_ERR == TesterDiagRouteActivationRequest(priv))
    {
        LOG_Error("%s route activation request fail.", __FUNCTION__);
        RingBufferDestroy(priv->rBuf);
        event_del(priv->ev);
        event_free(priv->ev);
        priv->ev = NULL;
        event_del(priv->local);
        event_free(priv->local);
        priv->local = NULL;
        close(priv->fds[0]);
        close(priv->fds[1]);
        close(fd);
        pthread_mutex_destroy(&priv->mutex);
        free(priv);
        return ret;        
    }

    *hdl = (TesterDiagHandle_t)priv;
    ret = RET_OK;
    return ret;
}

/**
 * @name TesterDiagUdsRequest
 * @brief 测试设备发送UDS请求。
 * @param hdl 诊断句柄，用于标识当前的诊断会话。
 * @param targetLa 目标逻辑地址，指定要发送UDS请求的网络节点。
 * @param udsData UDS请求数据，包含具体的诊断请求信息。
 * @param udsDataLen UDS请求数据长度，指定udsData参数中数据的字节长度。
 * @return 返回操作结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t TesterDiagUdsRequest(TesterDiagHandle_t hdl, uint32_t targetLa, const uint8_t *udsData, uint32_t udsDataLen)
{
    TesterDiagPriv_t *priv = (TesterDiagPriv_t *) hdl;
    int32_t ret = RET_ERR;

    if (!priv || !udsData || (udsDataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (0 == priv->isRouteActive)
    {
        LOG_Error("%s have not actived, can not request.", __FUNCTION__);
        return ret;
    }

    uint32_t packageLen = sizeof(DoipPacket_t) + sizeof(DiagnosticMessage_t) + udsDataLen;
    uint8_t *package = calloc(1, packageLen);

    if (!package)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return ret;
    }
    int32_t retVal;

    retVal = DoipGenMessage8001(priv->node.sourceLa, 
                                targetLa, 
                                udsData, 
                                udsDataLen, 
                                package, 
                                &packageLen);

    if (RET_ERR == retVal)
    {
        LOG_Error("%s DoipGenMessage8001 fail.", __FUNCTION__);
        free(package);
        return ret;
    }

    pthread_mutex_unlock(&priv->mutex);
    retVal = write(priv->fds[0], package, packageLen);
    //LOG_Debug("%s retVal:%d.", __FUNCTION__, retVal);    
    pthread_mutex_unlock(&priv->mutex);
    free(package);
    ret = RET_OK;
    return ret;
}

/**
 * @name TesterDiagDisconnect
 * @brief 断开测试设备诊断连接
 *        该函数用于断开与测试设备的诊断连接，释放相关资源。
 * @param hdl 诊断句柄，指向TesterDiagPriv_t结构体的指针。
 * @return 返回操作结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t TesterDiagDisconnect(TesterDiagHandle_t hdl)
{
    TesterDiagPriv_t *priv = (TesterDiagPriv_t *) hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    RingBufferDestroy(priv->rBuf);
    if (priv->ev)
    {
        int32_t fd = event_get_fd(priv->ev);
        event_del(priv->ev);
        event_free(priv->ev);
        priv->ev = NULL;
        shutdown(fd, SHUT_RDWR);
        close(fd);       
    }
    event_del(priv->local);
    event_free(priv->local);
    priv->local = NULL;
    close(priv->fds[0]);
    close(priv->fds[1]);
    pthread_mutex_destroy(&priv->mutex);
    free(priv);
    ret = RET_OK;
    return ret;
}