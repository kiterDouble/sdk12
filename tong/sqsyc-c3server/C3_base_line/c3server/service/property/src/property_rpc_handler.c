#include "property_rpc_handler.h"
#include "property_server.h"
#include "property_store.h"
#include "sr_log.h"

#include <stdio.h>
#include <string.h>

static void rpcSetProperty(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    PropertySetReq_t *req = NULL;
    PropertySetResp_t msgResp;
    PropertyServerContext_t* context = (PropertyServerContext_t*)userdata;
    PropertySetReq_t reqLocal = {0};

    memset(&msgResp, 0x00, sizeof(PropertySetResp_t));

    if (context == NULL) {
        LOG_Error("[%s] context is NULL", __func__);
        msgResp.result = -1;
        *respLen = sizeof(PropertySetResp_t);
        memcpy(respData, &msgResp, sizeof(PropertySetResp_t));
        return;
    }

    if (reqLen < sizeof(PropertySetReq_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(PropertySetReq_t)=%lu", __func__, reqLen, (unsigned long)sizeof(PropertySetReq_t));
        msgResp.result = -1;
        *respLen = sizeof(PropertySetResp_t);
        memcpy(respData, &msgResp, sizeof(PropertySetResp_t));
        return;
    }

    req = (PropertySetReq_t *)reqData;
    {
        memcpy(&reqLocal, req, sizeof(reqLocal));
        reqLocal.key[PROPERTY_RPC_KEY_MAX_LEN - 1] = '\0';
        reqLocal.value[PROPERTY_RPC_VALUE_MAX_LEN - 1] = '\0';

        LOG_Debug("[%s] key=%s", __func__, reqLocal.key);

        msgResp.result = PropertyStoreSet(context->propertys.map, &context->propertys.keyList,
                                          context->propStoreApi, (void *)context->propStoreDev,
                                          reqLocal.key, reqLocal.value);
    }

    *respLen = sizeof(PropertySetResp_t);
    memcpy(respData, &msgResp, sizeof(PropertySetResp_t));
}

static void rpcGetProperty(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    PropertyGetReq_t *req = NULL;
    PropertyGetResp_t msgResp;
    PropertyServerContext_t* context = (PropertyServerContext_t*)userdata;
    PropertyGetReq_t reqLocal = {0};

    memset(&msgResp, 0x00, sizeof(PropertyGetResp_t));

    if (context == NULL) {
        LOG_Error("[%s] context is NULL", __func__);
        msgResp.result = -1;
        *respLen = sizeof(PropertyGetResp_t);
        memcpy(respData, &msgResp, sizeof(PropertyGetResp_t));
        return;
    }

    if (reqLen < sizeof(PropertyGetReq_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(PropertyGetReq_t)=%lu", __func__, reqLen, (unsigned long)sizeof(PropertyGetReq_t));
        msgResp.result = -1;
        *respLen = sizeof(PropertyGetResp_t);
        memcpy(respData, &msgResp, sizeof(PropertyGetResp_t));
        return;
    }

    req = (PropertyGetReq_t *)reqData;
    {
        memcpy(&reqLocal, req, sizeof(reqLocal));
        reqLocal.key[PROPERTY_RPC_KEY_MAX_LEN - 1] = '\0';
        reqLocal.defaultValue[PROPERTY_RPC_VALUE_MAX_LEN - 1] = '\0';

        LOG_Debug("[%s] key=%s", __func__, reqLocal.key);

        msgResp.result = PropertyStoreGet(context->propertys.map, reqLocal.key, msgResp.value, sizeof(msgResp.value), reqLocal.defaultValue);
    }

    *respLen = sizeof(PropertyGetResp_t);
    memcpy(respData, &msgResp, sizeof(PropertyGetResp_t));
}

static void rpcQueryProperty(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    PropertyQueryReq_t *req = NULL;
    PropertyQueryResp_t msgResp;
    PropertyServerContext_t* context = (PropertyServerContext_t*)userdata;
    int size = 0;
    int number = 0;
    GList *node = NULL;
    char* propertyValue = NULL;

    memset(&msgResp, 0x00, sizeof(PropertyQueryResp_t));

    if (context == NULL) {
        LOG_Error("[%s] context is NULL", __func__);
        msgResp.result = -1;
        *respLen = sizeof(PropertyQueryResp_t);
        memcpy(respData, &msgResp, sizeof(PropertyQueryResp_t));
        return;
    }

    if (reqLen < sizeof(PropertyQueryReq_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(PropertyQueryReq_t)=%lu", __func__, reqLen, (unsigned long)sizeof(PropertyQueryReq_t));
        msgResp.result = -1;
        *respLen = sizeof(PropertyQueryResp_t);
        memcpy(respData, &msgResp, sizeof(PropertyQueryResp_t));
        return;
    }

    req = (PropertyQueryReq_t *)reqData;
    LOG_Debug("[%s] offset=%d, length=%d", __func__, req->offset, req->length);

    size = g_list_length(context->propertys.keyList);
    LOG_Warn("[%s] size: %d", __func__, size);

    if (req->offset < 0 || req->length <= 0 || req->offset >= size)
    {
        LOG_Error("[%s] invalid offset=%d, length=%d, size=%d", __func__, req->offset, req->length, size);
        msgResp.result = -1;
        *respLen = sizeof(PropertyQueryResp_t);
        memcpy(respData, &msgResp, sizeof(PropertyQueryResp_t));
        return;
    }

    int32_t queryLength = req->length;
    if (queryLength > PROPERTY_RPC_QUERY_MAX_NUM)
    {
        LOG_Warn("[%s] length=%d exceeds max=%d, limit to max", __func__, req->length, PROPERTY_RPC_QUERY_MAX_NUM);
        queryLength = PROPERTY_RPC_QUERY_MAX_NUM;
    }

    for (int i = req->offset; i < size && i < (req->offset + queryLength) && number < PROPERTY_RPC_QUERY_MAX_NUM; i++)
    {
        node = g_list_nth(context->propertys.keyList, i);
        if (node != NULL)
        {
            propertyValue = g_hash_table_lookup(context->propertys.map, node->data);
            if (propertyValue != NULL)
            {
                strncpy(msgResp.items[number].key, (char*)node->data, sizeof(msgResp.items[number].key) - 1);
                strncpy(msgResp.items[number].value, propertyValue, sizeof(msgResp.items[number].value) - 1);
                number++;
            }
        }
    }

    msgResp.number = number;
    msgResp.size = size;
    msgResp.result = 0;

    *respLen = sizeof(PropertyQueryResp_t);
    memcpy(respData, &msgResp, sizeof(PropertyQueryResp_t));
}

static const PropertySubProc_t g_propertySubProcs[] = {
    { E_PROPERTY_MSGTYPE_SET,   rpcSetProperty },   // 设置属性
    { E_PROPERTY_MSGTYPE_GET,   rpcGetProperty },   // 获取属性
    { E_PROPERTY_MSGTYPE_QUERY, rpcQueryProperty }, // 查询属性列表
};

void propertyRpcCallback(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    LOG_Debug("[%s] subcmd[%u] reqLen[%u]", __func__, (unsigned)subcmd, (unsigned)reqLen);

    for (uint32_t i = 0; i < sizeof(g_propertySubProcs) / sizeof(g_propertySubProcs[0]); ++i)
    {
        if (g_propertySubProcs[i].sub == subcmd)
        {
            if (g_propertySubProcs[i].procCb == NULL)
            {
                LOG_Error("[%s] handler for subcmd=%u is NULL", __func__, (unsigned)subcmd);
                return;
            }

            g_propertySubProcs[i].procCb(reqData, reqLen, respData, respLen, userdata);
            return;
        }
    }

    LOG_Error("[%s] subcmd invalid: %u", __func__, (unsigned)subcmd);
}
