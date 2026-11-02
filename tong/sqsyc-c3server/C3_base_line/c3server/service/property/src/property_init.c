#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "sr_log.h"
#include "property_store.h"
#include "property_server.h"

/* 定义宏，用于循环调用函数直到返回值为 0 */
#define LOOP_UNTIL_FUNC_SUCCESS(func, ...) \
    do { \
        while (1) { \
            if (func(__VA_ARGS__) == 0) { \
                break; \
            } \
            sleep(1); \
        } \
    } while (0)

static void DumpPropertyMap(GHashTable* map, GList* keyList)
{
    uint32_t listSize = g_list_length(keyList);
    GList *node = NULL;
    char* propertyValue = NULL;

    for (uint32_t i = 0; i < listSize; i++)
    {
        node = g_list_nth(keyList, i);
        if (node != NULL)
        {
            propertyValue = g_hash_table_lookup(map, node->data);
            if (propertyValue != NULL)
            {
                LOG_Debug("[%s]: %s\n", (char*) node->data, propertyValue);
            }
        }
    }
}


static const char* GetBuildPropPath(void)
{
    if (access(PROPERTY_BUILD_PROP_OEMDATA, F_OK) == 0)
    {
        return PROPERTY_BUILD_PROP_OEMDATA;
    }

    return PROPERTY_BUILD_PROP_OEMAPP;
}

/** 将系统内置 property 迁移到Server中 **/
static void LoadPropertysFromSystem(PropertyMap_t* propertyMap, PropStoreInterface_t* interface, void* dev)
{
    const char* filePath = NULL;
    FILE* fp = NULL;
    char line[PROPERTY_BUILD_PROP_LINE_MAX] = {0};
    char key[PROPERTY_KEY_MAX_SIZE] = {0};
    char value[PROPERTY_VALUE_MAX_SIZE] = {0};

    if (propertyMap == NULL || interface == NULL || dev == NULL)
    {
        LOG_Error("[%s] invalid argument", __func__);
        return;
    }

    filePath = GetBuildPropPath();
    if (access(filePath, F_OK) != 0)
    {
        LOG_Warn("[%s] build.prop not found: %s", __func__, filePath);
        return;
    }

    fp = fopen(filePath, "r");
    if (fp == NULL)
    {
        LOG_Error("[%s] failed to open %s: %s", __func__, filePath, strerror(errno));
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        char* p = line;
        char* keyStart = NULL;
        char* keyEnd = NULL;
        char* valueStart = NULL;
        size_t keyLen = 0;
        size_t valueLen = 0;
        size_t lineLen = 0;

        /* 跳过行首空白 */
        while (*p == ' ' || *p == '\t')
        {
            p++;
        }

        /* 跳过空行和注释行 */
        if (*p == '\0' || *p == '\n' || *p == '\r' || *p == '#')
        {
            continue;
        }

        /* 解析 [key]:value 格式 */
        if (*p != '[')
        {
            LOG_Warn("[%s] invalid line format: %s", __func__, line);
            continue;
        }

        keyStart = p + 1;
        keyEnd = strchr(keyStart, ']');
        if (keyEnd == NULL)
        {
            LOG_Warn("[%s] invalid line format (no closing bracket): %s", __func__, line);
            continue;
        }

        if (*(keyEnd + 1) != ':')
        {
            LOG_Warn("[%s] invalid line format (no colon): %s", __func__, line);
            continue;
        }

        valueStart = keyEnd + 2;

        /* 去掉行尾换行符 */
        lineLen = strlen(line);
        while (lineLen > 0 && (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r'))
        {
            line[lineLen - 1] = '\0';
            lineLen--;
        }

        /* 提取 key */
        keyLen = (size_t)(keyEnd - keyStart);
        if (keyLen == 0 || keyLen >= sizeof(key))
        {
            LOG_Warn("[%s] key too long or empty: %.*s", __func__, (int)keyLen, keyStart);
            continue;
        }
        memcpy(key, keyStart, keyLen);
        key[keyLen] = '\0';

        /* 如果 key 以 persist. 开头，丢弃不处理 */
        if ((keyLen >= strlen(PROPERTY_TABLE_PERSIST) + 1) &&
            (strncmp(key, PROPERTY_TABLE_PERSIST, strlen(PROPERTY_TABLE_PERSIST)) == 0) &&
            (key[strlen(PROPERTY_TABLE_PERSIST)] == '.'))
        {
            LOG_Debug("[%s] skip persist property: %s", __func__, key);
            continue;
        }

        /* 提取 value */
        valueLen = strlen(valueStart);
        if (valueLen >= sizeof(value))
        {
            LOG_Warn("[%s] value too long for key %s", __func__, key);
            continue;
        }
        memcpy(value, valueStart, valueLen);
        value[valueLen] = '\0';

        /* 写入数据库和内存缓存 */
        if (PropertyStoreSet(propertyMap->map, &propertyMap->keyList, interface, dev, key, value) != 0)
        {
            LOG_Warn("[%s] failed to set property: %s=%s", __func__, key, value);
        }
    }

    (void)fclose(fp);
}

static void PropStoreQueryCallBackFunc(char* key, char* value, void* arg)
{
    PropertyServerContext_t* context = (PropertyServerContext_t*)arg;
    char* newKey = NULL;
    char* newValue = NULL;

    if (context == NULL || context->propertys.map == NULL || key == NULL || value == NULL)
    {
        return;
    }

    newKey = g_strdup(key);
    newValue = g_strdup(value);
    if (newKey == NULL || newValue == NULL)
    {
        g_free(newKey);
        g_free(newValue);
        LOG_Error("[%s] g_strdup failed for key or value\n", __func__);
        return;
    }

    /* 如果 key 已存在，先从 list 中移除对应节点（data 由 hash table 释放） */
    if (g_hash_table_lookup(context->propertys.map, key) != NULL)
    {
        GList* found = g_list_find_custom(context->propertys.keyList, key, (GCompareFunc)g_strcmp0);
        if (found != NULL)
        {
            context->propertys.keyList = g_list_delete_link(context->propertys.keyList, found);
        }
        g_hash_table_remove(context->propertys.map, key);
    }

    /* 同时维护 keyList 顺序 与 hash table 缓存 */
    context->propertys.keyList = g_list_append(context->propertys.keyList, newKey);
    g_hash_table_insert(context->propertys.map, newKey, newValue);
}

/* 初始化Property线程 */
int PropertyServerInit(void* arg)
{
    PropertyServerContext_t* context = (PropertyServerContext_t*)arg;

    if (context == NULL)
    {
        LOG_Error("[%s] context is NULL", __func__);
        return -1;
    }

    if (context->propStoreApi == NULL ||
        context->propStoreApi->init == NULL ||
        context->propStoreApi->clear == NULL ||
        context->propStoreApi->set == NULL ||
        context->propStoreApi->query == NULL ||
        context->propStoreApi->deinit == NULL)
    {
        LOG_Error("[%s] invalid propStoreApi", __func__);
        return -1;
    }

    /**** 初始化 Property Map  ****/
    context->propertys.map = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    if (context->propertys.map == NULL)
    {
        LOG_Error("[%s] Failed to create property map", __func__);
        return -1;
    }

    /**** 初始化 Property 数据库 句柄  ****/
    LOOP_UNTIL_FUNC_SUCCESS(PropertyStoreInIt, context->propStoreApi, &context->propStoreDev, PROPERTY_SQLITE3_PATH);

    /**** 开机第一次启动，清除 table(ro,rw) ****/
    if (context->isFirstRunning != 0)
    {
        LOOP_UNTIL_FUNC_SUCCESS(PropertyStoreClearTable, context->propStoreApi, (void*) context->propStoreDev, PROPERTY_TABLE_RO);
        LOOP_UNTIL_FUNC_SUCCESS(PropertyStoreClearTable, context->propStoreApi, (void*) context->propStoreDev, PROPERTY_TABLE_RW);
        /** 将系统内置 property 迁移到Server中 **/
        LoadPropertysFromSystem(&context->propertys, context->propStoreApi, (void*) context->propStoreDev);        
    }

    /**** 初始化 加载 table(ro,rw，persist) property到缓存中 ****/
    LOOP_UNTIL_FUNC_SUCCESS(PropertyStoreQuery, context->propStoreApi, (void*) context->propStoreDev, PROPERTY_TABLE_PERSIST, PropStoreQueryCallBackFunc, arg);
    LOOP_UNTIL_FUNC_SUCCESS(PropertyStoreQuery, context->propStoreApi, (void*) context->propStoreDev, PROPERTY_TABLE_RO, PropStoreQueryCallBackFunc, arg);
    LOOP_UNTIL_FUNC_SUCCESS(PropertyStoreQuery, context->propStoreApi, (void*) context->propStoreDev, PROPERTY_TABLE_RW, PropStoreQueryCallBackFunc, arg);

    DumpPropertyMap(context->propertys.map, context->propertys.keyList);

    return 0;
}


int PropertyServerDeInit(void* arg)
{
    PropertyServerContext_t* context = (PropertyServerContext_t*)arg;

    if (context == NULL)
    {
        return -1;
    }

    if (context->propertys.keyList != NULL)
    {
        g_list_free_full(context->propertys.keyList, g_free);
        context->propertys.keyList = NULL;
    }

    if (context->propertys.map != NULL)
    {
        g_hash_table_destroy(context->propertys.map);
        context->propertys.map = NULL;
    }

    if (context->propStoreApi != NULL && context->propStoreDev != NULL)
    {
        PropertyStoreDeInit(context->propStoreApi, &context->propStoreDev);
    }

    return 0;
}
