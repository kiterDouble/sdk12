#include "entity_announce.h"

typedef struct
{
    evutil_socket_t fd;
    DoipEntityCfg_t cfg;
    uint8_t *establisted;
    struct event *event;
    struct event *timerEvent;
}EntityAnnoPriv_t;

/**
 * @name EntityAnnoReqProcCb
 * @brief 实体UDP接收处理回调
 * @param priv 指向 EntityAnnoPriv_t 结构体的指针，包含配置和状态信息。
 * @param clientAddr 指向接收到的数据包的来源地址的指针。
 * @param data 指向接收到的数据包的数据部分的指针。
 * @param dataLen 接收到的数据包的数据部分的长度。
 * @return 0 表示成功，其他值表示失败。
 */
typedef int32_t (*EntityAnnoReqProcCb)(EntityAnnoPriv_t *priv, const struct sockaddr_in *clientAddr, const uint8_t *data, uint32_t dataLen);

typedef struct
{
    uint32_t payloadType;
    EntityAnnoReqProcCb procCb;
}EntityAnnoProc_t;

/**
 * @name EntityAnnoSendTo
 * @brief 将实体announce信息发送给指定客户端。
 * @param priv 指向 EntityAnnoPriv_t 结构体的指针，包含配置和状态信息。
 * @param clientAddr 指向客户端地址的指针，用于指定消息发送的目的地。
 * @return 函数无返回值
 * 
 * 该函数首先检查输入参数的有效性，然后根据配置信息组装一个车辆注解响应，
 * 并使用 DoipGenMessage0004 函数生成相应的消息，最后通过 sendto 函数将消息发送给客户端。
 */
static void EntityAnnoSendTo(EntityAnnoPriv_t *priv, const struct sockaddr_in *clientAddr)
{
    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    uint8_t package[1024] = {0x0,};
    uint32_t packageLen = sizeof(package);
    VehicleAnnoRes_t annoResp;

    memcpy(annoResp.vin, priv->cfg.vin, sizeof(annoResp.vin));
    annoResp.logicAddr[0] = (priv->cfg.entityLa >> 8) & 0xff;
    annoResp.logicAddr[1] = priv->cfg.entityLa & 0xff;
    memcpy(annoResp.eid, priv->cfg.eid, sizeof(annoResp.eid));
    memcpy(annoResp.gid, priv->cfg.gid, sizeof(annoResp.gid));
    annoResp.actionReq = 0x00;
    DoipGenMessage0004(&annoResp, package, &packageLen);
    sendto(priv->fd, package, packageLen, 0, (struct sockaddr*)clientAddr, sizeof(struct sockaddr_in));
}


/**
 * @name EntityAnnoIDReqCb
 * @brief 处理0x0001请求的回调函数。
 * @param priv 指向实体注释私有数据的指针。
 * @param clientAddr 指向客户端地址的指针，用于标识请求来源。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据长度。
 * @return 函数执行成功返回RET_OK，否则返回RET_ERR。
 */
int32_t EntityAnnoIDReqCb(EntityAnnoPriv_t *priv, const struct sockaddr_in *clientAddr, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!priv || !clientAddr || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (RET_ERR == DoipParserMessage0001(data, dataLen))
    {
        LOG_Error("%s DoipParserMessage0001 fail.", __FUNCTION__);
        return ret;
    }

    EntityAnnoSendTo(priv, clientAddr);
    ret = RET_OK;
    return ret;
}


/**
 * @name EntityAnnoIDWithEidReqCb
 * @brief 处理0x0002请求的回调函数。
 * @param priv 指向实体注释私有数据的指针。
 * @param clientAddr 指向客户端地址的指针，用于标识请求来源。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据长度。
 * @return 函数执行成功返回RET_OK，否则返回RET_ERR。
 */
int32_t EntityAnnoIDWithEidReqCb(EntityAnnoPriv_t *priv, const struct sockaddr_in *clientAddr, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;
    if (!priv || !clientAddr || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    VehicleIDReqWithEid_t reqPL;

    if (RET_ERR == DoipParserMessage0002(data, dataLen, &reqPL))
    {
        LOG_Error("%s DoipParserMessage0002 fail.", __FUNCTION__);
        return ret;
    }

    if (0 != memcmp(reqPL.eid, priv->cfg.eid, sizeof(priv->cfg.eid)))
    {
        return ret;
    }
    EntityAnnoSendTo(priv, clientAddr);
    ret = RET_OK;
    return ret;
}

/**
 * @name EntityAnnoIDWithVinReqCb
 * @brief 处理0x0003请求的回调函数。
 * @param priv 指向实体注释私有数据的指针。
 * @param clientAddr 指向客户端地址的指针，用于标识请求来源。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据长度。
 * @return 函数执行成功返回RET_OK，否则返回RET_ERR。
 */
int32_t EntityAnnoIDWithVinReqCb(EntityAnnoPriv_t *priv, const struct sockaddr_in *clientAddr, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;
    if (!priv || !clientAddr || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    VehicleIDReqWithVin_t reqPL;

    if (RET_ERR == DoipParserMessage0003(data, dataLen, &reqPL))
    {
        LOG_Error("%s DoipParserMessage0002 fail.", __FUNCTION__);
        return ret;
    }

    if (0 != memcmp(reqPL.vin, priv->cfg.vin, sizeof(priv->cfg.vin)))
    {
        return ret;
    }
    EntityAnnoSendTo(priv, clientAddr);
    ret = RET_OK;
    return ret;
}

/**
 * @name EntityAnnoStatusReqCb
 * @brief 处理0x4001请求的回调函数。
 * @param priv 指向实体注释私有数据的指针。
 * @param clientAddr 指向客户端地址的指针，用于标识请求来源。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据长度。
 * @return 函数执行成功返回RET_OK，否则返回RET_ERR。
 */
int32_t EntityAnnoStatusReqCb(EntityAnnoPriv_t *priv, const struct sockaddr_in *clientAddr, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;
    if (!priv || !clientAddr || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (RET_ERR == DoipParserMessage4001(data, dataLen))
    {
        LOG_Error("%s DoipParserMessage4001 fail.", __FUNCTION__);
        return ret;
    }

    VehicleEntityStatusRes_t resPL;
    uint8_t package[1024] = {0x0,};
    uint32_t packageLen = sizeof(package);

    resPL.nodeType = E_ESNT_NODE;
    resPL.mcts = DOIP_ENTITY_SUPPORT_TESTER_MAX;
    resPL.ncts = *(priv->establisted);
    resPL.mds[0] = (DOIP_ENTITY_SUPPORT_PAYLOAD_lENGTH_MAX >> 24) & 0xff;
    resPL.mds[1] = (DOIP_ENTITY_SUPPORT_PAYLOAD_lENGTH_MAX >> 16) & 0xff;
    resPL.mds[2] = (DOIP_ENTITY_SUPPORT_PAYLOAD_lENGTH_MAX >> 8) & 0xff;
    resPL.mds[3] = DOIP_ENTITY_SUPPORT_PAYLOAD_lENGTH_MAX & 0xff;
    if (RET_ERR == DoipGenMessage4002(&resPL, package, &packageLen))
    {
        LOG_Error("%s DoipGenMessage4002 fail.", __FUNCTION__);
        return ret;
    }
    sendto(priv->fd, package, packageLen, 0, (struct sockaddr*)clientAddr, sizeof(struct sockaddr_in));
    ret = RET_OK;
    return ret;
}

/**
 * @name EntityAnnoPowerModeReqCb
 * @brief 处理0x4003请求的回调函数。
 * @param priv 指向实体注释私有数据的指针。
 * @param clientAddr 指向客户端地址的指针，用于标识请求来源。
 * @param data 指向接收到的数据的指针。
 * @param dataLen 接收到的数据长度。
 * @return 函数执行成功返回RET_OK，否则返回RET_ERR。
 */
int32_t EntityAnnoPowerModeReqCb(EntityAnnoPriv_t *priv, const struct sockaddr_in *clientAddr, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;
    if (!priv || !clientAddr || !data || (dataLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (RET_ERR == DoipParserMessage4003(data, dataLen))
    {
        LOG_Error("%s DoipParserMessage4003 fail.", __FUNCTION__);
        return ret;
    }
    PowerModeInfoRes_t resPL;
    uint8_t package[1024] = {0x0,};
    uint32_t packageLen = sizeof(package);

    resPL.powerMode = E_DPM_READY;
    if (RET_ERR == DoipGenMessage4004(&resPL, package, &packageLen))
    {
        LOG_Error("%s DoipGenMessage4004 fail.", __FUNCTION__);
        return ret;
    }
    sendto(priv->fd, package, packageLen, 0, (struct sockaddr*)clientAddr, sizeof(struct sockaddr_in));
    ret = RET_OK;
    return ret;
}

/**
 * @name EntityAnnoReadCb
 * @brief 实体Announce°事件处理函数
 *        此函数用于处理从指定套接字接收的实体注释请求，并根据请求的类型调用相应的处理回调。
 * @param fd 文件描述符，用于标识接收数据的套接字。
 * @param event 事件类型，表明接收操作完成或发生错误。
 * @param arg 用户自定义参数，通常用于传递回调函数所需的上下文信息。
 * @return 函数无返回值
 */
static void EntityAnnoReadCb(evutil_socket_t fd, short event, void *arg)
{
    EntityAnnoPriv_t *priv = (EntityAnnoPriv_t *) arg;
    static const EntityAnnoProc_t procCbs[] = 
    {
        {E_PL_TYPE_VEHICLE_INDENTIFY_REQ, EntityAnnoIDReqCb},
        {E_PL_TYPE_VEHICLE_INDENTIFY_EID_REQ, EntityAnnoIDWithEidReqCb},
        {E_PL_TYPE_VEHICLE_INDENTIFY_VIN_REQ, EntityAnnoIDWithVinReqCb},
        {E_PL_TYPE_ENTITY_STATUS_REQ, EntityAnnoStatusReqCb},
        {E_PL_TYPE_POWER_MODE_REQ, EntityAnnoPowerModeReqCb}
    };

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }
    uint8_t buf[1024] = {0x0,};

    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    ssize_t len = recvfrom(fd, (void*)buf, sizeof(buf), 0, (struct sockaddr *)&clientAddr, &clientLen);

    if (len > 0)
    {
        DoipMsgHeader_t header;
        uint32_t i = 0;
        
        if (RET_ERR == DoipParserHeader(buf, len, &header))
        {
            LOG_Error("%s DoipParserHeader fail.", __FUNCTION__);
            return;
        }
        for (; i < sizeof(procCbs) / sizeof(EntityAnnoProc_t); i++)
        {
            if (header.payloadType == procCbs[i].payloadType)
            {
                procCbs[i].procCb(priv, &clientAddr, buf, len);
            }
        }
    }
}

/**
 * @name EntityAnnoTimerCb
 * @brief 实体Announce定时器事件处理函数
 *        此函数为实体注解操作的定时器回调函数，用于在特定间隔后向指定地址发送注解信息。
 * @param fd 文件描述符，此回调中未使用。
 * @param event 事件类型，此回调中未使用。
 * @param arg 用户自定义参数，指向EntityAnnoPriv_t结构体的指针。
 */
static void EntityAnnoTimerCb(evutil_socket_t fd, short event, void *arg)
{
    EntityAnnoPriv_t *priv = (EntityAnnoPriv_t *)arg;
    static uint8_t count = 0;
    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    const char* address = "255.255.255.255";
    struct sockaddr_in clientAddr;

    clientAddr.sin_port=htons(DOIP_PORT);
    inet_aton(address, &(clientAddr.sin_addr));
    EntityAnnoSendTo(priv, &clientAddr);
    if (++count == 3)
    {
        event_del(priv->timerEvent);
        event_free(priv->timerEvent);
        priv->timerEvent = NULL;
        count = 0;
    }    
}

/**
 * @brief 实体announce初始化函数。
 * @param hdl 指向实体注解句柄的指针，初始化后将包含操作实体注解所需的所有资源。
 * @param cfg 指向DOIP实体配置的指针，用于配置实体注解的行为。
 * @param base 指向事件基础结构的指针，用于管理实体注解的事件处理。
 * @param establisted 指向一个布尔值的指针，初始化后将指示实体注解是否已建立连接。
 * @return 成功返回RET_OK，否则返回RET_ERR。
 */
int32_t EntityAnnoInit(EntityAnnoHandle_t *hdl, 
                    const DoipEntityCfg_t *cfg, 
                    struct event_base *base, 
                    uint8_t *establisted)
{
    EntityAnnoPriv_t *priv;
    int32_t ret = RET_ERR;

    if (!hdl || !cfg || !base)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    evutil_socket_t fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        LOG_Error("%s Couldn't create UDP socket.", __FUNCTION__);
        return ret;
    }

    int32_t reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        LOG_Error("%s setsockopt fail.", __FUNCTION__);
        close(fd);
        return ret;
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(DOIP_PORT);
    sin.sin_addr.s_addr = INADDR_ANY;
    if (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        LOG_Error("%s Couldn't bind UDP socket.", __FUNCTION__);
        close(fd);
        return ret;
    }
    priv = (EntityAnnoPriv_t *) calloc(1, sizeof(EntityAnnoPriv_t));

    if (!priv)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        close(fd);
        return ret;
    }
    priv->fd = fd;
    priv->event = event_new(base, fd, EV_READ | EV_PERSIST, EntityAnnoReadCb, (void*)priv);
    if (!priv->event || (event_add(priv->event, NULL) < 0))
    {
        LOG_Error("%s Could not event_new/event_add.", __FUNCTION__);
        if (priv->event)
        {
            event_del(priv->event);
            event_free(priv->event);            
        }
        close(fd);
        free(priv);
        return ret;
    }
    
    struct timeval tv = {0, 500000};
    priv->timerEvent = event_new(base, -1, EV_PERSIST, EntityAnnoTimerCb, (void*)priv);
    if (!priv->timerEvent || (event_add(priv->timerEvent, &tv) < 0))
    {
        LOG_Error("%s Could not event_new/event_add.", __FUNCTION__);
        if (priv->timerEvent)
        {
            event_del(priv->timerEvent);
            event_free(priv->timerEvent);            
        }
        event_del(priv->event);
        event_free(priv->event);
        close(fd);
        free(priv);
        return ret;
    }

    memcpy(&priv->cfg, cfg, sizeof(DoipEntityCfg_t));
    priv->establisted = establisted;
    *hdl = (EntityAnnoHandle_t)priv;
    ret = RET_OK;
    return ret;
}

/**
 * @name EntityAnnoDeinit
 * @brief 实体announce反初始化函数
 *        该函数用于释放实体注解相关的资源，包括事件和文件描述符等。
 * @param hdl 实体注解句柄，用于标识特定的实体注解实例
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t EntityAnnoDeinit(EntityAnnoHandle_t hdl)
{

    EntityAnnoPriv_t *priv = (EntityAnnoPriv_t *)hdl;
    int32_t ret = RET_ERR;
    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (priv->timerEvent)
    {
        event_del(priv->timerEvent);
        event_free(priv->timerEvent);
    }

    event_del(priv->event);
    event_free(priv->event);
    close(priv->fd);
    free(priv);
    ret = RET_OK;
    return ret;
}