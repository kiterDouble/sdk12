#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include "err_code.h"
#include "sr_log.h"
#include "msg_queue.h"

typedef struct
{
    uint8_t *value;
    uint32_t len;
} QueueData;

typedef struct DNode
{
    QueueData data;
    struct DNode *prev;
    struct DNode *next;
} TDNode, TDList;

typedef struct
{
    TDList *mList;
    pthread_mutex_t mLock;
    pthread_cond_t mNotEmpty;
}MessageQueuePriv_t;

static TDList* CreateList(void)
{
    TDList *L = (TDList *) calloc(1, sizeof(TDNode));
    
    if (!L)
        return NULL;
        
    L->next = L;
    L->prev = L;
    return L;
}

static int32_t IsEmptyList(TDList *L)
{
    int32_t ret = 0;
    
    ret = (L->next == L) && (L->prev == L);
    return  ret;
}

static int32_t InsertNode(TDList *L, uint8_t *data, uint32_t len)
{
    if (!L || !data || !len)
    {
        return RET_ERR;
    }
    
    uint8_t *value = calloc(1, len);
    if (!value)
        return RET_ERR;
        
    memcpy(value, data, len);
    TDNode *node = (TDNode *) calloc(1, sizeof(TDNode));
    if (!node)
    {
        free(value);
        return RET_ERR;
    }
    
    node->data.value = value;
    node->data.len = len;
    
    if (IsEmptyList(L))
    {
        L->next = node;
        L->prev = node;
        node->prev = L;
        node->next = L;
        
    }
    else
    {       
        L->next->prev = node;
        node->next = L->next;
        node->prev = L;
        L->next = node;
    }
    return RET_OK;
}

static int32_t DelNode(TDList *L, uint8_t *data, uint32_t *plen)
{
    if (!L || IsEmptyList(L))
    {
        return RET_ERR;
    }
    
    TDNode *node = L->prev;
    
    L->prev = node->prev;
    node->prev->next = L;
    
    if (data && plen)
    {
        memcpy(data, node->data.value, node->data.len);
        *plen = node->data.len;
    }
    
    free(node->data.value);
    free(node);
    return RET_OK;
}

static int32_t DestroyList(TDList *L)
{
    if (!L)
    {
        return RET_ERR;
    }
    
    while (!IsEmptyList(L))
    {
        DelNode(L, NULL, NULL);
    }
    
    free(L);
    return RET_OK;
}

int32_t CreateMsgQueue(MsgQueueHandle_t *qHdl)
{
    MessageQueuePriv_t *priv;
    int32_t ret = RET_ERR;

    if (!qHdl)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }

    priv = (MessageQueuePriv_t*) calloc(1, sizeof(MessageQueuePriv_t));

    if (!priv)
    {
        LOG_Error("%s malloc fail.", __FUNCTION__);
        return ret;
    }

    priv->mList = CreateList();
    
    if (!priv->mList)
    {
        LOG_Error("%s CreateList fail.", __FUNCTION__);
        free(priv);
        return ret;
    }
    
    pthread_condattr_t cattr;
    pthread_mutex_init(&priv->mLock, NULL);
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    pthread_cond_init(&priv->mNotEmpty, &cattr);
    *qHdl = (MsgQueueHandle_t) priv;
    return RET_OK;
}

int32_t SendMsgQueue(MsgQueueHandle_t qHdl, uint8_t *data, uint32_t len)
{
    MessageQueuePriv_t *priv = (MessageQueuePriv_t*) qHdl;
    int32_t ret = RET_ERR;

    if (!priv || !data || !len)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }
    
    pthread_mutex_lock(&priv->mLock);
    if (InsertNode(priv->mList, data, len) < 0)
    {
        LOG_Error("SendMsgQueue Send error");
        pthread_mutex_unlock(&priv->mLock);
        return ret;
    }

    pthread_cond_signal(&priv->mNotEmpty);
    pthread_mutex_unlock(&priv->mLock);
    return RET_OK;
}

static int32_t RecvMsgQueueInternal(MessageQueuePriv_t *priv, uint8_t *data, uint32_t *plen, struct timespec *outTime)
{
    int32_t res = 0;
    int32_t ret = RET_ERR;
    
    if (!priv || !data || !plen)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }
    
    pthread_mutex_lock(&priv->mLock);
    if (IsEmptyList(priv->mList))
    {
        res = (outTime != NULL)
            ? pthread_cond_timedwait(&priv->mNotEmpty,
                    &priv->mLock, outTime)
            : pthread_cond_wait(&priv->mNotEmpty,
                    &priv->mLock);
    } 
    
    if (res == 0)
    {
        if (RET_OK == DelNode(priv->mList, data, plen))
        {
            ret = RET_OK;
        }     
    }
    else
    {
        //LOG_Error("%s recv error.res:%d", __FUNCTION__, res);
    }
    
    pthread_mutex_unlock(&priv->mLock); 
    return ret;
}

static inline int timespec_compare(const struct timespec *lhs, const struct timespec *rhs)
{
    if (lhs->tv_sec < rhs->tv_sec)
        return RET_ERR;
    if (lhs->tv_sec > rhs->tv_sec)
        return 1;
    return lhs->tv_nsec - rhs->tv_nsec;
}

static int32_t RecvMsgQueueForSpuriousWakeup(MessageQueuePriv_t *priv, uint8_t *data, uint32_t *plen, struct timespec *outTime)
{
    int32_t ret = RET_ERR;
    struct timespec now;

    ret = RecvMsgQueueInternal(priv, data, plen, outTime);

    if (RET_OK != ret)//interupt break. should retry.
    {
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (timespec_compare(&now, outTime) < 0)//interupt break?
        {
            LOG_Debug("--{now}millisecond: %ld,{outTime}millisecond: %ld--",
                    (now.tv_sec * 1000 + now.tv_nsec / 1000000),
                    (outTime->tv_sec * 1000 + outTime->tv_nsec / 1000000));
            return RecvMsgQueueForSpuriousWakeup(priv, data, plen, outTime);
        }
    }

    return ret;
}

int32_t RecvMsgQueue(MsgQueueHandle_t qHdl, uint8_t *data, uint32_t *plen, long ms)
{
    MessageQueuePriv_t *priv = (MessageQueuePriv_t*) qHdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }
    
    if (0 == ms)
    {
        ret = RecvMsgQueueInternal(priv, data, plen, NULL);
    }
    else
    {
        struct timespec outtime;
        long nsec = 0;

        clock_gettime(CLOCK_MONOTONIC, &outtime);
        nsec = outtime.tv_nsec + (ms % 1000) * 1000000;
        outtime.tv_sec = outtime.tv_sec + ms / 1000 + nsec / 1000000000;
        outtime.tv_nsec = nsec % 1000000000;
        ret = RecvMsgQueueForSpuriousWakeup(priv, data, plen, &outtime);
    }
    return ret;
}

int32_t DestroyMsgQueue(MsgQueueHandle_t qHdl)
{
    MessageQueuePriv_t *priv = (MessageQueuePriv_t*) qHdl;
    int32_t ret = RET_ERR;

    if (!priv)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return ret;
    }
    
    DestroyList(priv->mList);
    pthread_mutex_destroy(&priv->mLock);
    pthread_cond_destroy(&priv->mNotEmpty);
    free(priv);
    return RET_OK;
}