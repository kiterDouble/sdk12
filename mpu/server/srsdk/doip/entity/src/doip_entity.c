#include <event2/thread.h>
#include "entity_announce.h"
#include "entity_diagnostic.h"

typedef struct
{
    struct event_base *base;
    struct event *signalEvent;
    uint8_t establisted;
    pthread_t threadId;
    EntityAnnoHandle_t annoHdl;
    EntityDiagHandle_t diagHdl;
}DoipEntityPriv_t;

/**
 * @name EntitySignalHandle
 * @brief 实体的信号处理函数。
 * @param fd 文件描述符，用于标识触发事件的实体。
 * @param event 事件类型，表明了实体所触发的具体事件。
 * @param arg 用户自定义参数，可用于传递额外的信息或上下文。
 * @return 函数无返回值
 */
static void EntitySignalHandle(evutil_socket_t fd, short event, void *arg)
{
    LOG_Error("%s event %d.", __FUNCTION__, event);
}

/**
 * @name DoipEntityrHandler
 * @brief 处理Doip实体事件的函数。
 *        该函数是一个线程函数，用于调度和处理传入的Doip实体的事件。
 * @param args 函数接收的参数，期望是一个DoipEntityPriv_t类型的指针，用于访问实体的私有数据。
 * @return void* 函数总是返回NULL，因为它的主要目的是事件处理，不产生需要返回的结果。
 */
static void* DoipEntityrHandler(void* args)
{
    DoipEntityPriv_t *priv = (DoipEntityPriv_t *)args;
    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return NULL;
    }
    event_base_dispatch(priv->base);
    return NULL;
}

/**
 * @name DoipEntityInit
 * @brief DOIP实体初始化
 *        本函数用于初始化DOIP实体，配置实体的基本属性、事件处理机制，并启动相关的线程。
 * @param hdl 指向DoipEntityHandle_t的指针，用于返回初始化后的实体句柄。
 * @param cfg 指向DoipEntityCfg_t的常量指针，提供实体的配置信息。
 * @param reqCb UdsRequestCallBack类型的回调函数，用于处理UDS请求。
 * @return 返回RET_OK表示成功，其他错误码表示失败。
 */
int32_t DoipEntityInit(DoipEntityHandle_t *hdl, 
                    const DoipEntityCfg_t *cfg, 
                    UdsRequestCallBack reqCb)
{
    DoipEntityPriv_t *priv;
    int32_t ret = RET_ERR;

    if (!hdl || !cfg || !reqCb)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    evthread_use_pthreads();
    priv = (DoipEntityPriv_t *) calloc(1, sizeof(DoipEntityPriv_t));

    if (!priv)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return ret;
    }

    priv->base = event_base_new();
    if (!priv->base)
    {
        LOG_Error("%s event_base_new fail.", __FUNCTION__);
        free(priv);
        return ret;
    }

    priv->signalEvent = evsignal_new(priv->base, SIGPIPE, EntitySignalHandle, NULL);
    if (!priv->signalEvent || event_add(priv->signalEvent, NULL) < 0)
    {
        LOG_Error("%s Could not create/add signal event.", __FUNCTION__);
        event_base_free(priv->base);
        free(priv);
        return ret;
    }

    if (RET_OK != EntityDiagInit(&priv->diagHdl, cfg, reqCb, priv->base, &priv->establisted))
    {
        event_del(priv->signalEvent);
        event_free(priv->signalEvent);
        event_base_free(priv->base);
        free(priv);
        return ret;       
    }

    if (RET_OK != EntityAnnoInit(&priv->annoHdl, cfg, priv->base, &priv->establisted))
    {
        EntityDiagDeinit(priv->diagHdl);
        event_del(priv->signalEvent);
        event_free(priv->signalEvent);
        event_base_free(priv->base);
        free(priv);
        return ret;       
    }


    if(0 != pthread_create(&priv->threadId, NULL, DoipEntityrHandler, (void*)priv))
    {
        LOG_Error("%s Could not pthread_create.", __FUNCTION__);
        EntityAnnoDeinit(priv->annoHdl);
        EntityDiagDeinit(priv->diagHdl);
        event_del(priv->signalEvent);
        event_free(priv->signalEvent);
        event_base_free(priv->base);
        free(priv);
        return ret;
    }
    *hdl = priv;
    ret = RET_OK;
    return ret;
}


/**
 * @name DoipEntityUdsResponse
 * @brief 处理UDS（UDS诊断服务）响应的函数。
 * @param hdl Doip实体的句柄，用于标识特定的DOIP实体。
 * @param testerLa 测试设备地址，用于标识发送UDS请求的测试者。
 * @param udsData 包含UDS响应数据的缓冲区。
 * @param dataLen UDS响应数据的长度。
 * @return 函数执行状态，如果执行成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipEntityUdsResponse(DoipEntityHandle_t hdl, 
                            uint32_t testerLa, 
                            const uint8_t *udsData, 
                            uint32_t dataLen)
{
    DoipEntityPriv_t *priv = (DoipEntityPriv_t *)hdl;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }

    return EntityDiagnUdsResponse(priv->diagHdl, testerLa, udsData, dataLen);
}

/**
 * @name DoipEntityDeinit
 * @brief 对DOIP实体进行反初始化
 *        该函数用于释放DOIP实体在初始化时分配的资源，包括诊断模块、注释模块的资源释放，以及事件系统的清理。
 * @param hdl DoipEntityHandle_t类型，DOIP实体的句柄，用于标识和访问具体的实体。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t DoipEntityDeinit(DoipEntityHandle_t hdl)
{
    DoipEntityPriv_t *priv = (DoipEntityPriv_t *)hdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    EntityDiagDeinit(priv->diagHdl);
    EntityAnnoDeinit(priv->annoHdl);
    event_del(priv->signalEvent);
    event_free(priv->signalEvent);
    event_base_loopbreak(priv->base);
    event_base_free(priv->base);
    free(priv);
    ret = RET_OK;
    return ret;
}