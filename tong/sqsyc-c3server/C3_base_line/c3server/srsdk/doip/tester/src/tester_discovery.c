#include "tester_discovery.h"

typedef struct 
{
    evutil_socket_t fd;    
    struct event *event;
    DoipTesterOnlineNotifyCb notifyCb;
}TesterDiscoveyPriv_t;

/**
 * @name TesterDiscoveryReadCb
 * @brief 测试器发现应答读取回调函数
 *        此函数被用于处理从设备接收的发现应答消息。它首先尝试解析接收到的消息，如果消息解析成功，
 *        则调用提供的回调函数通知用户逻辑地址。
 * @param fd 文件描述符，用于标识接收数据的套接字。
 * @param event 事件类型，表明接收操作完成或发生错误等。
 * @param arg 用户提供的自定义参数，通常用于指向回调函数所需的上下文信息。
 */
static void TesterDiscoveryReadCb(evutil_socket_t fd, short event, void *arg)
{
    TesterDiscoveyPriv_t *priv = (TesterDiscoveyPriv_t *) arg;
    uint8_t buf[1024] = {0x0,};

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    ssize_t len = recvfrom(fd, (void*)buf, sizeof(buf), 0, (struct sockaddr *)((void*)&client_addr), &client_len);

    if (len > 0)
    {
        VehicleAnnoRes_t res;
        if (RET_OK == DoipParserMessage0004(buf, (uint32_t)len, &res))
        {
            uint32_t targetLa = BYTES_TO_UINT16((uint32_t)res.logicAddr[0], res.logicAddr[1]);
            priv->notifyCb(targetLa);
        }
    }
}

/**
 * @name TesterDisCoveryInit
 * @brief 测试设备发现初始化。
 * @param hdl 指向TesterDiscoveryHandle_t的指针，用于储存内部私有数据。
 * @param base 指向事件基础结构的指针，用于事件驱动处理。 
 * @param notifyCb 当在线测试器被发现时的回调函数。
 * @return 成功返回RET_OK，否则返回RET_ERR。
 */
int32_t TesterDisCoveryInit(TesterDiscoveryHandle_t *hdl, 
                        struct event_base *base, 
                        DoipTesterOnlineNotifyCb notifyCb)
{
    TesterDiscoveyPriv_t *priv;
    int32_t ret = RET_ERR;

    if (!hdl || !base || !notifyCb)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    evutil_socket_t fd = socket(AF_INET, (int)SOCK_DGRAM, 0);
    if (fd < 0)
    {
        LOG_Error("%s Couldn't create UDP socket.", __FUNCTION__);
        return ret;
    }
	
    int32_t reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
	{
        LOG_Error("%s setsockopt fail.", __FUNCTION__);
        (void)close(fd);
        return ret;
    }

    struct sockaddr_in scoketIn;
    (void)memset(&scoketIn, 0, sizeof(scoketIn));
    scoketIn.sin_family = AF_INET;
    scoketIn.sin_port = htons(DOIP_PORT);
    scoketIn.sin_addr.s_addr = INADDR_ANY;
    if (bind(fd, (struct sockaddr *)((void*)&scoketIn), sizeof(scoketIn)) < 0)
    {
        LOG_Error("%s Couldn't bind UDP socket.", __FUNCTION__);
        (void)close(fd);
        return ret;
    }
    priv = (TesterDiscoveyPriv_t *) calloc(1, sizeof(TesterDiscoveyPriv_t));

    if (!priv)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        (void)close(fd);
        return ret;
    }
    priv->fd = fd;
    priv->notifyCb = notifyCb;
    priv->event = event_new(base, fd, EV_READ | EV_PERSIST, TesterDiscoveryReadCb, (void*)priv);
    if (!priv->event || (event_add(priv->event, NULL) < 0))
    {
        LOG_Error("%s Could not event_new/event_add.", __FUNCTION__);
        if (NULL != priv->event)
        {
            (void)event_del(priv->event);
            event_free(priv->event);
        }
        (void)close(fd);
        free(priv);
    }

    *hdl = (TesterDiscoveryHandle_t)priv;
    ret = RET_OK;
    return ret;
}

/**
 * @name TesterDisCoveryDeinit
 * @brief 测试设备发现反初始化
 * @param hdl 测试设备发现句柄
 * @return 成功返回RET_OK，否则返回RET_ERR。 
 */
int32_t TesterDisCoveryDeinit(TesterDiscoveryHandle_t hdl)
{
    TesterDiscoveyPriv_t *priv = (TesterDiscoveyPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if(!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    (void)close(priv->fd);
    (void)event_del(priv->event);
    event_free(priv->event);
    free(priv);
    ret = RET_OK;
    return ret;
}