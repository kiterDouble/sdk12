#include "rpc_property_api.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "queue_msg.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <limits.h>

static int32_t RpcPropertyCall(uint16_t subcmd, const uint8_t *reqData, uint16_t reqLen,
                               uint8_t *respData, uint16_t *respLen)
{
    int32_t ret = -1;

    do {
        ret = RpcGetMsgSync(RPCSERVER_TYPE_PROPERTYSERVER, subcmd, 15000,
                            reqData, reqLen, respData, respLen);
    } while(0);
    return ret;
}

int32_t RpcPropertySet(const char *key, const char *value)
{
    int32_t ret = -1;
    PropertySetReq_t req;
    PropertySetResp_t resp;
    uint16_t respLen = sizeof(resp);

    LOG_Warn("[%s] enter", __func__);
    do {
        if (key == NULL || value == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        memset(&req, 0, sizeof(req));
        strncpy(req.key, key, sizeof(req.key) - 1);
        strncpy(req.value, value, sizeof(req.value) - 1);

        LOG_Debug("[%s] key=%s", __func__, key);

        ret = RpcPropertyCall(E_PROPERTY_MSGTYPE_SET, (uint8_t*)&req, sizeof(req), (uint8_t*)&resp, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            break;
        }

        if (respLen < sizeof(resp)) {
            LOG_Error("[%s] respLen too small: %u < %zu", __func__, respLen, sizeof(resp));
            ret = -1;
            break;
        }

        if (resp.result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp.result);
            ret = resp.result;
            break;
        }

        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcPropertyGet(const char *key, const char *defaultValue, char *value, uint32_t valueBufSize)
{
    int32_t ret = -1;
    PropertyGetReq_t req;
    PropertyGetResp_t resp;
    uint16_t respLen = sizeof(resp);

    LOG_Warn("[%s] enter", __func__);
    do {
        if (key == NULL || value == NULL || valueBufSize == 0) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        memset(&req, 0, sizeof(req));
        strncpy(req.key, key, sizeof(req.key) - 1);
        if (defaultValue != NULL) {
            strncpy(req.defaultValue, defaultValue, sizeof(req.defaultValue) - 1);
        }

        LOG_Debug("[%s] key=%s", __func__, key);

        ret = RpcPropertyCall(E_PROPERTY_MSGTYPE_GET, (uint8_t*)&req, sizeof(req), (uint8_t*)&resp, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            break;
        }

        if (respLen < sizeof(resp)) {
            LOG_Error("[%s] respLen too small: %u < %zu", __func__, respLen, sizeof(resp));
            ret = -1;
            break;
        }

        if (resp.result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp.result);
            ret = resp.result;
            break;
        }

        strncpy(value, resp.value, valueBufSize - 1);
        value[valueBufSize - 1] = '\0';

        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcPropertySetUint64(const char *key, uint64_t value)
{
    char valueStr[24] = {0};
    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return -1;
    }
    snprintf(valueStr, sizeof(valueStr), "%" PRIu64, value);
    return RpcPropertySet(key, valueStr);
}

int32_t RpcPropertySetInt64(const char *key, int64_t value)
{
    char valueStr[24] = {0};
    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return -1;
    }
    snprintf(valueStr, sizeof(valueStr), "%" PRId64, value);
    return RpcPropertySet(key, valueStr);
}

int32_t RpcPropertySetUint32(const char *key, uint32_t value)
{
    char valueStr[24] = {0};
    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return -1;
    }
    snprintf(valueStr, sizeof(valueStr), "%u", value);
    return RpcPropertySet(key, valueStr);
}

int32_t RpcPropertySetInt32(const char *key, int32_t value)
{
    char valueStr[24] = {0};
    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return -1;
    }
    snprintf(valueStr, sizeof(valueStr), "%d", value);
    return RpcPropertySet(key, valueStr);
}

int32_t RpcPropertySetUint16(const char *key, uint16_t value)
{
    char valueStr[24] = {0};
    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return -1;
    }
    snprintf(valueStr, sizeof(valueStr), "%u", value);
    return RpcPropertySet(key, valueStr);
}

int32_t RpcPropertySetInt16(const char *key, int16_t value)
{
    char valueStr[24] = {0};
    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return -1;
    }
    snprintf(valueStr, sizeof(valueStr), "%d", value);
    return RpcPropertySet(key, valueStr);
}

int32_t RpcPropertySetUint8(const char *key, uint8_t value)
{
    char valueStr[24] = {0};
    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return -1;
    }
    snprintf(valueStr, sizeof(valueStr), "%u", value);
    return RpcPropertySet(key, valueStr);
}

int32_t RpcPropertySetInt8(const char *key, int8_t value)
{
    char valueStr[24] = {0};
    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return -1;
    }
    snprintf(valueStr, sizeof(valueStr), "%d", value);
    return RpcPropertySet(key, valueStr);
}

int32_t RpcPropertySetDouble(const char *key, double value)
{
    char valueStr[24] = {0};
    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return -1;
    }
    if (snprintf(valueStr, sizeof(valueStr), "%.6g", value) >= (int)sizeof(valueStr)) {
        LOG_Error("[%s] double value truncated", __func__);
        return -1;
    }
    return RpcPropertySet(key, valueStr);
}

uint64_t RpcPropertyGetUint64(const char *key, uint64_t defaultValue)
{
    int32_t ret = 0;
    char value[PROPERTY_RPC_VALUE_MAX_LEN] = {0};
    char defaultStr[24] = {0};
    char *endptr = NULL;
    uint64_t result = 0;

    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return defaultValue;
    }

    snprintf(defaultStr, sizeof(defaultStr), "%" PRIu64, defaultValue);
    ret = RpcPropertyGet(key, defaultStr, value, sizeof(value));
    if (ret == 0)
    {
        errno = 0;
        result = strtoull(value, &endptr, 10);
        if (endptr == value || *endptr != '\0' || errno == ERANGE)
        {
            return defaultValue;
        }
        return result;
    }
    else
    {
        return defaultValue;
    }
}

int64_t RpcPropertyGetInt64(const char *key, int64_t defaultValue)
{
    int32_t ret = 0;
    char value[PROPERTY_RPC_VALUE_MAX_LEN] = {0};
    char defaultStr[24] = {0};
    char *endptr = NULL;
    int64_t result = 0;

    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return defaultValue;
    }

    snprintf(defaultStr, sizeof(defaultStr), "%" PRId64, defaultValue);
    ret = RpcPropertyGet(key, defaultStr, value, sizeof(value));
    if (ret == 0)
    {
        errno = 0;
        result = strtoll(value, &endptr, 10);
        if (endptr == value || *endptr != '\0' || errno == ERANGE)
        {
            return defaultValue;
        }
        return result;
    }
    else
    {
        return defaultValue;
    }
}

uint32_t RpcPropertyGetUint32(const char *key, uint32_t defaultValue)
{
    int32_t ret = 0;
    char value[PROPERTY_RPC_VALUE_MAX_LEN] = {0};
    char defaultStr[24] = {0};
    char *endptr = NULL;
    unsigned long result = 0;

    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return defaultValue;
    }

    snprintf(defaultStr, sizeof(defaultStr), "%u", defaultValue);
    ret = RpcPropertyGet(key, defaultStr, value, sizeof(value));
    if (ret == 0)
    {
        errno = 0;
        result = strtoul(value, &endptr, 10);
        if (endptr == value || *endptr != '\0' || errno == ERANGE || result > UINT32_MAX)
        {
            return defaultValue;
        }
        return (uint32_t)result;
    }
    else
    {
        return defaultValue;
    }
}

int32_t RpcPropertyGetInt32(const char *key, int32_t defaultValue)
{
    int32_t ret = 0;
    char value[PROPERTY_RPC_VALUE_MAX_LEN] = {0};
    char defaultStr[24] = {0};
    char *endptr = NULL;
    long result = 0;

    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return defaultValue;
    }

    snprintf(defaultStr, sizeof(defaultStr), "%d", defaultValue);
    ret = RpcPropertyGet(key, defaultStr, value, sizeof(value));
    if (ret == 0)
    {
        errno = 0;
        result = strtol(value, &endptr, 10);
        if (endptr == value || *endptr != '\0' || errno == ERANGE || result < INT32_MIN || result > INT32_MAX)
        {
            return defaultValue;
        }
        return (int32_t)result;
    }
    else
    {
        return defaultValue;
    }
}

uint16_t RpcPropertyGetUint16(const char *key, uint16_t defaultValue)
{
    int32_t ret = 0;
    char value[PROPERTY_RPC_VALUE_MAX_LEN] = {0};
    char defaultStr[24] = {0};
    char *endptr = NULL;
    unsigned long result = 0;

    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return defaultValue;
    }

    snprintf(defaultStr, sizeof(defaultStr), "%u", defaultValue);
    ret = RpcPropertyGet(key, defaultStr, value, sizeof(value));
    if (ret == 0)
    {
        errno = 0;
        result = strtoul(value, &endptr, 10);
        if (endptr == value || *endptr != '\0' || errno == ERANGE || result > UINT16_MAX)
        {
            return defaultValue;
        }
        return (uint16_t)result;
    }
    else
    {
        return defaultValue;
    }
}

int16_t RpcPropertyGetInt16(const char *key, int16_t defaultValue)
{
    int32_t ret = 0;
    char value[PROPERTY_RPC_VALUE_MAX_LEN] = {0};
    char defaultStr[24] = {0};
    char *endptr = NULL;
    long result = 0;

    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return defaultValue;
    }

    snprintf(defaultStr, sizeof(defaultStr), "%d", defaultValue);
    ret = RpcPropertyGet(key, defaultStr, value, sizeof(value));
    if (ret == 0)
    {
        errno = 0;
        result = strtol(value, &endptr, 10);
        if (endptr == value || *endptr != '\0' || errno == ERANGE || result < INT16_MIN || result > INT16_MAX)
        {
            return defaultValue;
        }
        return (int16_t)result;
    }
    else
    {
        return defaultValue;
    }
}

uint8_t RpcPropertyGetUint8(const char *key, uint8_t defaultValue)
{
    int32_t ret = 0;
    char value[PROPERTY_RPC_VALUE_MAX_LEN] = {0};
    char defaultStr[24] = {0};
    char *endptr = NULL;
    unsigned long result = 0;

    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return defaultValue;
    }

    snprintf(defaultStr, sizeof(defaultStr), "%u", defaultValue);
    ret = RpcPropertyGet(key, defaultStr, value, sizeof(value));
    if (ret == 0)
    {
        errno = 0;
        result = strtoul(value, &endptr, 10);
        if (endptr == value || *endptr != '\0' || errno == ERANGE || result > UINT8_MAX)
        {
            return defaultValue;
        }
        return (uint8_t)result;
    }
    else
    {
        return defaultValue;
    }
}

int8_t RpcPropertyGetInt8(const char *key, int8_t defaultValue)
{
    int32_t ret = 0;
    char value[PROPERTY_RPC_VALUE_MAX_LEN] = {0};
    char defaultStr[24] = {0};
    char *endptr = NULL;
    long result = 0;

    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return defaultValue;
    }

    snprintf(defaultStr, sizeof(defaultStr), "%d", defaultValue);
    ret = RpcPropertyGet(key, defaultStr, value, sizeof(value));
    if (ret == 0)
    {
        errno = 0;
        result = strtol(value, &endptr, 10);
        if (endptr == value || *endptr != '\0' || errno == ERANGE || result < INT8_MIN || result > INT8_MAX)
        {
            return defaultValue;
        }
        return (int8_t)result;
    }
    else
    {
        return defaultValue;
    }
}

double RpcPropertyGetDouble(const char *key, double defaultValue)
{
    int32_t ret = 0;
    char value[PROPERTY_RPC_VALUE_MAX_LEN] = {0};
    char defaultStr[24] = {0};
    char *endptr = NULL;
    double result = 0.0;

    if (key == NULL) {
        LOG_Error("[%s] key is NULL", __func__);
        return defaultValue;
    }

    if (snprintf(defaultStr, sizeof(defaultStr), "%.6g", defaultValue) >= (int)sizeof(defaultStr)) {
        LOG_Error("[%s] default value truncated", __func__);
        return defaultValue;
    }
    ret = RpcPropertyGet(key, defaultStr, value, sizeof(value));
    if (ret == 0)
    {
        errno = 0;
        result = strtod(value, &endptr);
        if (endptr == value || *endptr != '\0' || errno == ERANGE)
        {
            return defaultValue;
        }
        return result;
    }
    else
    {
        return defaultValue;
    }
}

int32_t RpcPropertyQuery(int32_t offset, int32_t length, PropertyItem_t *items, uint32_t itemBufNum, int32_t *number, int32_t *size)
{
    int32_t ret = -1;
    PropertyQueryReq_t req;
    PropertyQueryResp_t resp;
    uint16_t respLen = sizeof(resp);
    int32_t copyNum = 0;

    LOG_Warn("[%s] enter", __func__);
    do {
        if (items == NULL || itemBufNum == 0 || number == NULL || size == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        memset(&req, 0, sizeof(req));
        req.offset = offset;
        req.length = length;

        LOG_Debug("[%s] offset=%d, length=%d", __func__, offset, length);

        ret = RpcPropertyCall(E_PROPERTY_MSGTYPE_QUERY, (uint8_t*)&req, sizeof(req), (uint8_t*)&resp, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            break;
        }

        if (respLen < sizeof(resp)) {
            LOG_Error("[%s] respLen too small: %u < %zu", __func__, respLen, sizeof(resp));
            ret = -1;
            break;
        }

        if (resp.result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp.result);
            ret = resp.result;
            break;
        }

        if (resp.number < 0 || resp.size < 0) {
            LOG_Error("[%s] Invalid resp number=%d or size=%d", __func__, resp.number, resp.size);
            ret = -1;
            break;
        }

        copyNum = (resp.number < (int32_t)itemBufNum) ? resp.number : (int32_t)itemBufNum;
        if (copyNum > PROPERTY_RPC_QUERY_MAX_NUM) {
            copyNum = PROPERTY_RPC_QUERY_MAX_NUM;
        }
        if (copyNum > 0) {
            memcpy(items, resp.items, (size_t)copyNum * sizeof(PropertyItem_t));
        }
        *number = copyNum;
        *size = resp.size;

        LOG_Debug("[%s] number=%d, size=%d", __func__, *number, *size);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}
