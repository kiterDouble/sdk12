#include <event2/thread.h>
#include "tester_discovery.h"
#include "tester_diagnostic.h"

#define SUPPROT_TESTER_DISCOVERY
typedef struct
{
    struct event_base *base;
    struct event *signalEvent;
    pthread_t threadId;
    DoipTesterCbs_t funs;
#ifdef SUPPROT_TESTER_DISCOVERY
    TesterDiscoveryHandle_t discoveryHdl;
#endif
    TesterDiagHandle_t diagnosticHdl;
}DoipTesterPriv_t;


/**
 * @name TesterSignalHandle
 * @brief 信号处理函数
 *        该函数用于处理特定的事件信号。当对应的事件发生时，此函数将被调用以处理该事件。
 * @param fd 文件描述符，用于标识发生事件的socket。
 * @param event 事件类型，表示发生了哪种类型的事件。
 * @param arg 传递给函数的用户数据，可用于处理特定的业务逻辑。
 * @return 函数无返回值
 */
static void TesterSignalHandle(evutil_socket_t fd, short event, void *arg)
{
    LOG_Error("%s event %d.", __FUNCTION__, event);
}

/**
 * @name DoipTesterHandler
 * @brief 执行Doip测试设备的处理函数
 *        该函数是一个线程入口函数，用于启动Doip测试设备的事件处理。
 * @param args 函数的输入参数，期望是一个DoipTesterPriv_t类型的指针，用于访问测试设备的私有数据。
 * @return 函数无返回值。
 */
static void* DoipTesterHandler(void* args)
{
    DoipTesterPriv_t *priv = (DoipTesterPriv_t *)args;
    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return NULL;
    }
    event_base_dispatch(priv->base);
    return NULL;
}

/**
 * @name DoipTesterInit
 * @brief 初始化DOIP测试设备。
 * @param hdl 指向DoipTesterHandle_t的指针，用于储存初始化后的测试设备句柄。
 * @param funs 指向DoipTesterCbs_t的指针，包含了测试设备需要的回调函数。
 * @return 成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipTesterInit(DoipTesterHandle_t *hdl, const DoipTesterCbs_t *funs)
{
    DoipTesterPriv_t *priv;
    int32_t ret = RET_ERR;

    if (!hdl || !funs)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    evthread_use_pthreads();
    priv = (DoipTesterPriv_t *) calloc(1, sizeof(DoipTesterPriv_t));

    if (!priv)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return ret;
    }

    memcpy(&priv->funs, funs, sizeof(DoipTesterCbs_t));
    priv->base = event_base_new();
    if (!priv->base)
    {
        LOG_Error("%s event_base_new fail.", __FUNCTION__);
        free(priv);
        return ret;
    }

    priv->signalEvent = evsignal_new(priv->base, SIGPIPE, TesterSignalHandle, NULL);
    if (!priv->signalEvent || event_add(priv->signalEvent, NULL) < 0)
    {
        LOG_Error("%s Could not create/add signal event.", __FUNCTION__);
        event_base_free(priv->base);
        free(priv);
        return ret;
    }
#ifdef SUPPROT_TESTER_DISCOVERY
    ret = TesterDisCoveryInit(&priv->discoveryHdl, priv->base, funs->onlineFunc);
    
    if (RET_ERR == ret)
    {
        LOG_Error("%s Could not TesterDisCoveryInit.", __FUNCTION__);
        event_del(priv->signalEvent);
        event_free(priv->signalEvent);
        event_base_free(priv->base);
        free(priv);
        return ret;
    }
#endif
    if(0 != pthread_create(&priv->threadId, NULL, DoipTesterHandler, (void*)priv))
    {
        LOG_Error("%s Could not pthread_create.", __FUNCTION__);
    #ifdef SUPPROT_TESTER_DISCOVERY    
        TesterDisCoveryDeinit(priv->discoveryHdl);
        priv->discoveryHdl = NULL;
    #endif
        event_del(priv->signalEvent);
        event_free(priv->signalEvent);
        event_base_free(priv->base);
        free(priv);
        return ret;
    }

    *hdl = (DoipTesterHandle_t)priv;
    ret = RET_OK;
    return ret;
}

/**
 * @name DoipTesterDeinit
 * @brief 对DOIP测试设备进行反初始化
 *        该函数用于释放DOIP测试设备在初始化时分配的资源，包括网络连接、事件基等。
 * @param hdl DOIP测试设备的句柄，用于标识和访问测试设备的私有数据。
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t DoipTesterDeinit(DoipTesterHandle_t hdl)
{
    DoipTesterPriv_t *priv = (DoipTesterPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (priv->diagnosticHdl)
    {
        DoipTesterDisconnect(priv->diagnosticHdl);
        priv->diagnosticHdl = NULL;
    }
#ifdef SUPPROT_TESTER_DISCOVERY
    TesterDisCoveryDeinit(priv->discoveryHdl);
    priv->discoveryHdl = NULL;
#endif
    event_del(priv->signalEvent);
    event_free(priv->signalEvent);
    event_base_loopbreak(priv->base);
    event_base_free(priv->base);
    free(priv);
    ret = RET_OK;
    return ret;
}

/**
 * @name DoipTesterConnect
 * @brief 连接到DOIP测试设备
 * 本函数用于通过提供的节点信息，建立到DOIP测试设备的连接。
 * @param hdl DOIP测试设备的句柄，用于标识和访问测试设备的私有数据。
 * @param node 指向包含要连接的节点信息的结构体的指针。
 * @return 返回连接操作的结果状态码。成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t DoipTesterConnect(DoipTesterHandle_t hdl, const DoipTesterNode_t *node)
{
    DoipTesterPriv_t *priv = (DoipTesterPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    return TesterDiagConnect(&priv->diagnosticHdl, priv->base, node, &priv->funs);
}

/**
 * @name DoipTesterUdsRequest
 * @brief 执行UDS请求的函数
 *        该函数用于通过DOIP（Diagnostic Over IP）协议向目标设备发送一个UDS（Unified Diagnostic Services）请求。
 * @param hdl DoipTester的句柄，用于标识和访问测试设备的私有数据。
 * @param targetLa 目标逻辑地址，标识要向哪个网络节点发送请求。
 * @param udsData 指向包含UDS请求数据的缓冲区的指针。
 * @param udsDataLen UDS请求数据的长度。
 * @return 返回函数执行状态码。成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t DoipTesterUdsRequest(DoipTesterHandle_t hdl, uint32_t targetLa, const uint8_t *udsData, uint32_t udsDataLen)
{
    DoipTesterPriv_t *priv = (DoipTesterPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    return TesterDiagUdsRequest(priv->diagnosticHdl, targetLa, udsData, udsDataLen);
}

/**
 * @name DoipTesterDisconnect
 * @brief 断开DOIP测试设备的连接。
 * @param hdl DoipTesterHandle_t 类型，表示DOIP测试设备的句柄。
 * @return 返回操作的结果状态码。成功时返回RET_OK，失败时返回RET_ERR。
 */
int32_t DoipTesterDisconnect(DoipTesterHandle_t hdl)
{
    DoipTesterPriv_t *priv = (DoipTesterPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    
    ret = TesterDiagDisconnect(priv->diagnosticHdl);
    priv->diagnosticHdl = NULL;
    return ret;
}