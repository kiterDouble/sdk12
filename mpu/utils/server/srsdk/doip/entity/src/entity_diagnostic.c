#include <errno.h>
#include "entity_diagnostic.h"
#include "ring_buffer.h"

typedef struct
{
    uint32_t isActived;
    uint32_t testerAddr;
    RingBuffer_t rBuf;
    struct event *testerEv;
    struct event *initTimer;
    struct event *aliveTimer;
    struct event *geneTimer; 
}EntityDiagClient_t;

typedef struct
{
    struct event_base *base;
    DoipEntityCfg_t cfg;
    uint8_t *establisted;
    UdsRequestCallBack reqCb;
    struct evconnlistener *listener;
    int32_t fds[2];
    struct event *local;
    EntityDiagClient_t client;
}EntityDiagPriv_t;

/**
 * @name EntityDiagReqProcCb
 * @brief 实体诊断请求处理回调
 * @param priv 指向实体诊断私有数据的指针。
 * @param data 指向请求数据的指针。
 * @param dataLen 请求数据的长度。
 * @return 成功返回RET_OK，否则返回RET_ERR。
 */
typedef int32_t (*EntityDiagReqProcCb)(EntityDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen);

typedef struct
{
    uint32_t payloadType;
    EntityDiagReqProcCb procCb;
}EntityAnnoProc_t;

static int32_t EntityDiagClientDeinit(EntityDiagPriv_t *priv);

/**
 * @name EntityDiagSendError
 * @brief 实体错误消息发送函数
 *        该函数用于生成一个包含错误信息的协议包，并通过给定的socket发送给实体。
 *        错误信息是通过一个32位的错误码来表示的。
 * @param fd 文件描述符，用于标识接收错误信息的实体的socket。
 * @param err 错误码，表示具体的错误信息。
 * @return 函数无返回值
 */
static void EntityDiagSendError(evutil_socket_t fd, uint32_t err)
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
 * @name RoutingActivationReqProc
 * @brief 路由激活请求处理函数。
 * @param priv 指向实体诊断私有数据的指针。
 * @param data 指向请求数据的指针。
 * @param dataLen 请求数据的长度。
 * @return 成功返回RET_OK，否则返回RET_ERR。
 */
static int32_t RoutingActivationReqProc(EntityDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    RoutineActivationReq_t reqPL;

    if (RET_ERR == DoipParserMessage0005(data, dataLen, &reqPL))
    {
        LOG_Error("%sDoipParserMessage0005 fail.", __FUNCTION__);
        return ret;
    }

    RoutineActivationRes_t resPL;
    uint8_t package[128] = {0x0,};
    uint32_t pLen = sizeof(package);

    if (reqPL.activeType != priv->cfg.activeType)
    {
        resPL.code = E_RARC_UNSUPPORTED_ROUTING_ACTIVATION_TYPE;
    }
    else
    {
        resPL.code = E_RARC_SUCCESSFULLY_ACTIVATED;
    }

    priv->client.testerAddr = BYTES_TO_UINT16(reqPL.sourceAddr[0], reqPL.sourceAddr[1]);
    priv->client.isActived = 1;
    memset(&resPL, 0x0, sizeof(RoutineActivationRes_t));
    resPL.testEquipment[0] = reqPL.sourceAddr[0];
    resPL.testEquipment[1] = reqPL.sourceAddr[1];
    resPL.entityEquipment[0] = (priv->cfg.entityLa >> 8) & 0xff;
    resPL.entityEquipment[1] = priv->cfg.entityLa & 0xff;
    resPL.code = E_RARC_SUCCESSFULLY_ACTIVATED;

    DoipGenMessage0006(&resPL, package, &pLen);
    int32_t retVal = write(event_get_fd(priv->client.testerEv), package, pLen);
    LOG_Debug("%s retVal:%d.", __FUNCTION__, retVal);
    if (resPL.code != E_RARC_SUCCESSFULLY_ACTIVATED)
    {
        EntityDiagClientDeinit(priv);
    }
    ret = RET_OK;
    return ret;
    
}

/**
 * @name AliveCheckResProc
 * @brief 检查实体状态响应处理
 *        本函数用于处理实体状态检查的响应。它首先验证传入的参数是否有效，然后如果存在存活定时器，
 *        则删除并释放该定时器。
 * @param priv 指向实体诊断私有数据的指针。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据长度。
 * @return 函数返回RET_OK表示成功，RET_ERR表示失败。
 */
static int32_t AliveCheckResProc(EntityDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    if (priv->client.aliveTimer)
    {
        event_del(priv->client.aliveTimer);
        event_free(priv->client.aliveTimer);
        priv->client.aliveTimer = NULL;
    }
    ret = RET_OK;
    return ret;
}

/**
 * @name DiagnosticReqProc
 * @brief 诊断请求处理函数
 * @param priv 指向实体诊断私有数据的指针
 * @param data 指向诊断数据的指针
 * @param dataLen 诊断数据的长度
 * @return 返回处理结果，RET_OK表示成功，其他值表示失败
 */
static int32_t DiagnosticReqProc(EntityDiagPriv_t *priv, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!priv || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (0 == priv->client.isActived)
    {
        LOG_Error("%s have not actived.", __FUNCTION__);
        EntityDiagClientDeinit(priv);
        return ret;        
    }

    uint32_t retVal = DoipParserMessage8001(data, dataLen);
    if (RET_OK != retVal)
    {
        LOG_Error("%s DoipParserMessage8001 fail.", __FUNCTION__);
        return ret;
    }


    DoipMsgHeader_t header;

    DoipParserHeader(data, dataLen, &header);

    if (DOIP_ENTITY_SUPPORT_PAYLOAD_lENGTH_MAX < header.payloadLen)
    {
        EntityDiagSendError(event_get_fd(priv->client.testerEv), E_GDHNAE_MESSAGE_TOO_LARGE);
        return ret;
    }

    DoipPacket_t *packet = (DoipPacket_t *) data;
    DiagnosticMessage_t *diagPL = (DiagnosticMessage_t*)packet->payload;
    uint32_t entityAddr = BYTES_TO_UINT16(diagPL->targetAddr[0], diagPL->targetAddr[1]);
    uint32_t testerAddr = BYTES_TO_UINT16(diagPL->sourceAddr[0], diagPL->sourceAddr[1]);    
    uint8_t bytes[1024] = {0x0,};
    uint32_t bytesLen = sizeof(bytes);

    //check source addr.
    if (1)
    {
        int32_t i = 0;

        for (; i < priv->cfg.testerNum; i++)
        {
            if (testerAddr == priv->cfg.testerLas[i])
            {
                break;
            }
        }

        if (i == priv->cfg.testerNum)
        {
            DoipGenMessage8003(priv->cfg.entityLa, testerAddr, 
                            E_DMNAC_INVALID_SOURCE_ADDRESS, 
                            NULL, 0, bytes, &bytesLen);
            retVal = write(event_get_fd(priv->client.testerEv), bytes, bytesLen);
            LOG_Debug("%s testerAddr:%04x, retVal:%d.", __FUNCTION__, testerAddr,retVal);
            return ret;
        }
    }
    //check target addr.
    if (1)
    {
        if ((entityAddr != priv->cfg.entityLa) && (entityAddr != priv->cfg.functionLa))
        {
            DoipGenMessage8003(priv->cfg.entityLa, testerAddr, 
                            E_DMNAC_UNKNOWN_TARGET_ADDRESS, 
                            NULL, 0, bytes, &bytesLen);
            retVal = write(event_get_fd(priv->client.testerEv), bytes, bytesLen);
            LOG_Debug("%s retVal:%d.", __FUNCTION__, retVal);
            return ret;
        }
    }

    DoipGenMessage8002(priv->cfg.entityLa, testerAddr, 
                            E_DMPAC_SUCCESS, 
                            NULL, 0, bytes, &bytesLen);
    retVal = write(event_get_fd(priv->client.testerEv), bytes, bytesLen);
    LOG_Debug("%s retVal:%d.", __FUNCTION__, retVal);
    priv->reqCb(testerAddr, diagPL->userdata, (header.payloadLen - sizeof(DiagnosticMessage_t)));
    ret = RET_OK;
    return ret;
}

/**
 * @name EntityDiagRequestProc
 * @brief 实体诊断请求处理函数。
 * @param priv 指向实体诊断私有数据结构的指针，用于存储和访问与实体诊断相关的数据和状态。
 * @return 函数无返回值
 */
static void EntityDiagRequestProc(EntityDiagPriv_t *priv)
{
    static const EntityAnnoProc_t procCbs[] = 
    {
        {E_PL_TYPE_ROUTING_ACTIVATION_REQ, RoutingActivationReqProc},
        {E_PL_TYPE_ALIVE_CHECK_RES, AliveCheckResProc},
        {E_PL_TYPE_DIAGNOSTIC_REQ, DiagnosticReqProc}
    };

    if(!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    if (priv->client.geneTimer)
    {
        event_del(priv->client.geneTimer);
        event_free(priv->client.geneTimer);
        priv->client.geneTimer = NULL;
    }
    uint32_t avaiSize = 0;
    DoipMsgHeader_t header;
    uint8_t bytes[sizeof(DoipPacket_t)];

    RingBufferGetAvailableSize(priv->client.rBuf, &avaiSize);
    RingBufferPeek(priv->client.rBuf, bytes, sizeof(bytes));
    if (RET_ERR == DoipParserHeader(bytes, sizeof(bytes), &header))
    {
        LOG_Error("%s DoipParserHeader fail.", __FUNCTION__);
        EntityDiagClientDeinit(priv);
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

    RingBufferDequeue(priv->client.rBuf, package, packageLen);
    uint32_t i = 0;   
    uint32_t err;

    if (RET_ERR == DoipCheckHeader(package, packageLen, &err))
    {
        EntityDiagSendError(event_get_fd(priv->client.testerEv), err);

        if (E_GDHNAE_INCORRECT_PATTERN_FORMAT ==  err)
        {
            EntityDiagClientDeinit(priv);
        }
        return;
    }

    for (; i < sizeof(procCbs) / sizeof(EntityAnnoProc_t); i++)
    {
        if (header.payloadType == procCbs[i].payloadType)
        {
            procCbs[i].procCb(priv, package, packageLen);
            free(package);
            return;
        }
    }

    EntityDiagSendError(event_get_fd(priv->client.testerEv), E_GDHNAE_UNKNOWN_PAYLOAD_TYPE);
    free(package);
}


/**
 * @name EntityDiagLocalReadCb
 * @brief 实体诊断local socket读取事件处理函数
 *        此函数被用于处理从实体诊断连接读取到的数据。它会尝试读取数据，并将读取到的数据写入到与客户端的事件基础结构相关联的文件描述符中。
 * @param fd 文件描述符，用于读取数据。
 * @param events 事件类型，指示读取事件是否发生。
 * @param arg 用户自定义参数，通常为指向实体诊断私有数据结构的指针。
 */
static void EntityDiagLocalReadCb(evutil_socket_t fd, short events, void *arg)
{
    EntityDiagPriv_t *priv = (EntityDiagPriv_t *) arg;

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
        if (!priv->client.testerEv)
        {
            LOG_Error("%s testerEv is NULL.", __FUNCTION__);
            return;
        }
        int32_t retVal = write(event_get_fd(priv->client.testerEv), bytes, readn);
        LOG_Debug("%s retVal:%d.", __FUNCTION__, retVal);
    }

}


/**
 * @name EntityDiagInitTimerCb
 * @brief 实体诊断的doip initial定时器事件处理函数
 *        该函数被作为定时器的回调使用，用于在特定事件触发时，对实体诊断客户端进行反初始化操作。
 * @param fd 文件描述符，此处未使用。
 * @param event 事件类型，此处未使用。
 * @param arg 用户自定义参数，指向EntityDiagPriv_t类型的结构体实例。
 * @return 函数无返回值
 */
static void EntityDiagInitTimerCb(evutil_socket_t fd, short event, void *arg)
{
    EntityDiagPriv_t *priv = (EntityDiagPriv_t*) arg;

    if(!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    EntityDiagClientDeinit(priv);
}
/**
 * @name EntityDiagReadn
 * @brief 从指定的文件描述符中读取数据到缓冲区
 * @param fd 文件描述符
 * @param data 用于存储读取数据的缓冲区指针
 * @param datalen 指定要读取的数据长度
 * @return 若成功读取数据，返回RET_OK；否则返回RET_ERR
 */
static int32_t EntityDiagReadn(int32_t fd, uint8_t *data, uint32_t datalen)
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
            if((readn < 0) && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
            {
                usleep(1);
                continue;
            }

            LOG_Error("%s read error.readn:%d, errno:%d", __FUNCTION__, readn, errno);
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
 * @name EntityDiagReadCb
 * @brief 处理实体诊断读取回调
 *        此函数用于处理实体诊断时的数据读取回调事件。它首先尝试读取并解析DOIP包头，随后读取并处理包体。
 *        在读取过程中，如果发生错误，则进行相应的资源清理并返回。成功读取后，会重新设置生成定时器。
 * @param fd 文件描述符，用于数据读取
 * @param events 事件类型，表明读取事件的发生
 * @param arg 用户自定义参数，此处为EntityDiagPriv_t的指针
 * @return 函数无返回值
 */
static void EntityDiagReadCb(evutil_socket_t fd, short events, void *arg)
{
    EntityDiagPriv_t *priv = (EntityDiagPriv_t*) arg;

    if(!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    uint8_t bytes[sizeof(DoipPacket_t)] = {0x0,};
    if (DOIP_OK != EntityDiagReadn(fd, bytes, sizeof(bytes)))
    {
        LOG_Error("%s read header fail.", __FUNCTION__);
        EntityDiagClientDeinit(priv);
        return;
    }
    RingBufferEnqueue(priv->client.rBuf, bytes, sizeof(bytes));
    DoipMsgHeader_t header;
    uint32_t offset = 0;

    DoipParserHeader(bytes, sizeof(bytes), &header);
    while(offset < header.payloadLen)
    {
        uint8_t buffer[1024] = {0x0};
        uint32_t readn = sizeof(buffer);
        if (readn > (header.payloadLen - offset))
        {
            readn = header.payloadLen - offset;
        }
        if (DOIP_OK != EntityDiagReadn(fd, buffer, readn))
        {
            LOG_Error("%s read payload fail.", __FUNCTION__);
            EntityDiagClientDeinit(priv);
            return;
        }
        RingBufferEnqueue(priv->client.rBuf, buffer, readn);
        offset += readn;
    }

    if (priv->client.initTimer)
    {
        event_del(priv->client.initTimer);
        event_free(priv->client.initTimer);
        priv->client.initTimer = NULL;
    }        
    if (priv->client.geneTimer)
    {
        event_del(priv->client.geneTimer);
        event_free(priv->client.geneTimer);
        priv->client.geneTimer = NULL;
    }

    EntityDiagRequestProc(priv);

    struct timeval tv = {5*60, 0};
    priv->client.geneTimer = event_new(priv->base, -1, EV_TIMEOUT, EntityDiagInitTimerCb, (void*)priv);
    if (!priv->client.geneTimer || (event_add(priv->client.geneTimer, &tv) < 0))
    {
        LOG_Error("%s Could not event_new/event_add.", __FUNCTION__);
        if (priv->client.geneTimer)
        {
            event_del(priv->client.geneTimer);
            event_free(priv->client.geneTimer);
            priv->client.geneTimer = NULL;
        }
    }
}

/**
 * @name EntityDiagClientInit
 * @brief 初始化实体诊断客户端
 *        本函数用于初始化实体诊断客户端的相关配置和事件处理机制。
 *        它会设置TCP_NODELAY选项，以减少延迟；创建一个环形缓冲区用于数据收发；
 *        并设置读取事件及初始化超时事件。
 * @param priv 指向实体诊断私有数据结构的指针。
 * @param fd 文件描述符，用于网络通信。
 * @return 成功返回RET_OK，失败返回RET_ERR。
 */
static int32_t EntityDiagClientInit(EntityDiagPriv_t *priv, evutil_socket_t fd)
{
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    int32_t flag = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&flag, sizeof(int32_t)) == -1)
    {
        LOG_Error("%s setsockopt() fail.", __FUNCTION__);
        close(fd);
        return ret;
    }

    uint32_t bufSize = DOIP_ENTITY_SUPPORT_PAYLOAD_lENGTH_MAX + sizeof(DoipPacket_t);
    bufSize = ((bufSize / 1024) + 1) * 1024;//k bytes align.
    if (0 != RingBufferCreate(&priv->client.rBuf, bufSize))
    {
        LOG_Error("%s RingBufferCreate() fail.", __FUNCTION__);
        close(fd);
        return ret;        
    }

    priv->client.testerEv = event_new((struct event_base *)priv->base, fd, EV_READ | EV_PERSIST, EntityDiagReadCb, (void *)priv);
    if (!priv->client.testerEv || (event_add(priv->client.testerEv, NULL) < 0))
    {
        if (priv->client.testerEv)
        {
            event_del(priv->client.testerEv);
            event_free(priv->client.testerEv);
            priv->client.testerEv = NULL;
            
        }
        RingBufferDestroy(priv->client.rBuf);
        close(fd);
        return ret;
    }

    struct timeval tv = {2, 0};
    priv->client.initTimer = event_new(priv->base, -1, EV_TIMEOUT, EntityDiagInitTimerCb, (void*)priv);
    if (!priv->client.initTimer || (event_add(priv->client.initTimer, &tv) < 0))
    {
        LOG_Error("%s Could not event_new/event_add.", __FUNCTION__);
        if (priv->client.initTimer)
        {
            event_del(priv->client.initTimer);
            event_free(priv->client.initTimer);
            priv->client.initTimer = NULL;
        }
        event_del(priv->client.testerEv);
        event_free(priv->client.testerEv);
        priv->client.testerEv = NULL;  
        RingBufferDestroy(priv->client.rBuf);
        close(fd);
        memset(&priv->client, 0x0, sizeof(EntityDiagClient_t));
        return ret;
    }
    *priv->establisted = 1;
    ret = RET_OK;
    return ret;
}

/**
 * @name EntityDiagClientDeinit
 * @brief 实体诊断客户端反初始化
 *        该函数用于释放和清理实体诊断客户端在初始化时分配的资源。
 * @param priv 指向实体诊断私有数据结构的指针。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
static int32_t EntityDiagClientDeinit(EntityDiagPriv_t *priv)
{
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (priv->client.initTimer)
    {
        event_del(priv->client.initTimer);
        event_free(priv->client.initTimer);
        priv->client.initTimer = NULL;
    }

    if (priv->client.geneTimer)
    {
        event_del(priv->client.geneTimer);
        event_free(priv->client.geneTimer);
        priv->client.geneTimer = NULL;
    }

    if (priv->client.aliveTimer)
    {
        event_del(priv->client.aliveTimer);
        event_free(priv->client.aliveTimer);
        priv->client.aliveTimer = NULL;
    }

    if (priv->client.testerEv)
    {
        int32_t fd = event_get_fd(priv->client.testerEv);
        event_del(priv->client.testerEv);
        event_free(priv->client.testerEv);
        priv->client.testerEv = NULL;
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }
    if (priv->client.rBuf)
    {
        RingBufferDestroy(priv->client.rBuf);
    }
    
    memset(&priv->client, 0x0, sizeof(EntityDiagClient_t));
    *priv->establisted = 0;
    ret = RET_OK;
    return ret;
}
/**
 * @name EntityDiagAcceptCb
 * @brief 当实体诊断连接被接受时的回调函数。
 *        这个函数被用于处理新的实体诊断连接请求。它会初始化诊断客户端的相关私有数据结构。
 * @param listener 指向接收连接的监听器的指针。
 * @param fd 新连接的文件描述符。
 * @param addr 新连接的远程地址信息。
 * @param socklen 地址结构的长度。
 * @param ctx 用户提供的上下文信息，用于访问函数外部的数据。
 * @return 函数无返回值
 */
static void EntityDiagAcceptCb(struct evconnlistener *listener, 
                    evutil_socket_t fd, 
                    struct sockaddr *addr, 
                    int socklen, 
                    void *ctx)
{
    EntityDiagPriv_t *priv = (EntityDiagPriv_t *) ctx;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    EntityDiagClientInit(priv, fd);
}

/**
 * @name EntityDiagInit
 * @brief 实体诊断初始化。
 * @param hdl 指向实体诊断句柄的指针，初始化完成后将包含模块的私有数据。
 * @param cfg 指向DOIP实体配置的指针，提供必要的配置信息。
 * @param reqCb 请求回调函数，用于处理来自UDS请求的回调。
 * @param base 指向事件基础结构的指针，用于事件驱动的网络通信。
 * @param establisted 指向建立标志的指针，初始化为NULL，成功建立连接后将被设置为非NULL。
 * @return 成功返回RET_OK，否则返回RET_ERR。
 */
int32_t EntityDiagInit(EntityDiagHandle_t *hdl, 
                    const DoipEntityCfg_t *cfg, 
                    UdsRequestCallBack reqCb, 
                    struct event_base *base, 
                    uint8_t *establisted)
{
    EntityDiagPriv_t *priv;
    int32_t ret = RET_ERR;

    if (!hdl || !cfg || !reqCb || !base || !establisted)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    priv = (EntityDiagPriv_t *) calloc(1, sizeof(EntityDiagPriv_t));
    if (!priv)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return ret;
    }

    if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, priv->fds)) 
    {
        LOG_Error("%s socketpair fail.", __FUNCTION__);
        free(priv);
        return ret;
    }

    priv->local = event_new((struct event_base *)base, priv->fds[1], EV_READ | EV_PERSIST, EntityDiagLocalReadCb, (void *)priv);
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
        free(priv);
        return ret;
    }

    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(DOIP_PORT);
    priv->listener = evconnlistener_new_bind(base, 
                                            EntityDiagAcceptCb, 
                                            (void *) priv, 
                                            LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
                                            DOIP_ENTITY_SUPPORT_TESTER_MAX,
                                            (struct sockaddr *)&sin, 
                                            sizeof(sin));

    if (!priv->listener)
    {
        LOG_Error("%s evconnlistener_new_bind fail.", __FUNCTION__);
        event_del(priv->local);
        event_free(priv->local);
        priv->local = NULL;
        close(priv->fds[0]);
        close(priv->fds[1]);
        free(priv);
        return ret;
    }

    memcpy(&priv->cfg, cfg, sizeof(DoipEntityCfg_t));
    priv->base = base;
    priv->reqCb = reqCb;
    priv->establisted = establisted;
    *hdl = (EntityDiagHandle_t)priv;
    ret = RET_OK;
    return ret;
}

/**
 * @name EntityDiagnUdsResponse
 * @brief 实体诊断UDS响应
 * @param hdl 实体诊断句柄
 * @param testerLa 测试者地址，用于标识发出诊断请求的设备
 * @param udsData UDS诊断数据，包含诊断请求的具体内容
 * @param dataLen UDS数据长度，指定udsData参数中数据的字节长度
 * @return 返回执行结果，RET_OK表示成功，RET_ERR表示失败
 */
int32_t EntityDiagnUdsResponse(EntityDiagHandle_t hdl, 
                            uint32_t testerLa, 
                            const uint8_t *udsData, 
                            uint32_t dataLen)
{

    EntityDiagPriv_t *priv = (EntityDiagPriv_t *) hdl;
    int32_t ret = RET_ERR;

    if (!priv || !udsData || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint32_t packageLen = sizeof(DoipPacket_t) + sizeof(DiagnosticMessage_t) + dataLen;
    uint8_t *package = calloc(1, packageLen);

    if (!package)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return ret;
    }
    int32_t retVal;

    retVal = DoipGenMessage8001(priv->cfg.entityLa, 
                                testerLa, 
                                udsData, 
                                dataLen, 
                                package, 
                                &packageLen);

    if (RET_ERR == retVal)
    {
        LOG_Error("%s DoipGenMessage8001 fail.", __FUNCTION__);
        return ret;
    }

    retVal = write(priv->fds[0], package, packageLen);
    LOG_Debug("%s retVal:%d.", __FUNCTION__, retVal);
    free(package);
    ret = RET_OK;
    return ret;
}

/**
 * @name EntityDiagDeinit
 * @brief 实体诊断反初始化函数。
 * @param hdl 实体诊断句柄
 * @return 返回执行结果，RET_OK表示成功，RET_ERR表示失败 
 */
int32_t EntityDiagDeinit(EntityDiagHandle_t hdl)
{
    EntityDiagPriv_t *priv = (EntityDiagPriv_t *) hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    EntityDiagClientDeinit(priv);
    evconnlistener_free(priv->listener);
    close(priv->fds[0]);
    close(priv->fds[1]);
    free(priv);
    ret = RET_OK;
    return ret;
}