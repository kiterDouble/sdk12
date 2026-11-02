#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>

#include <sqlite3.h>

#include "sr_log.h"
#include "property_store.h"

static void GetPropertyTable(char* key,char* table)
{
    size_t keyLen = 0;
    size_t persistLen = strlen(PROPERTY_TABLE_PERSIST);
    size_t roLen = strlen(PROPERTY_TABLE_RO);

    if (table == NULL || key == NULL)
    {
        return;
    }

    keyLen = strlen(key);

    if ((keyLen >= persistLen + 1) &&
        (strncmp(key, PROPERTY_TABLE_PERSIST, persistLen) == 0) &&
        (key[persistLen] == '.'))
    {
        memcpy(table, PROPERTY_TABLE_PERSIST, persistLen);
        table[persistLen] = '\0';
    }
    else if ((keyLen >= roLen + 1) &&
             (strncmp(key, PROPERTY_TABLE_RO, roLen) == 0) &&
             (key[roLen] == '.'))
    {
        memcpy(table, PROPERTY_TABLE_RO, roLen);
        table[roLen] = '\0';
    }
    else
    {
        memcpy(table, PROPERTY_TABLE_RW, strlen(PROPERTY_TABLE_RW));
        table[strlen(PROPERTY_TABLE_RW)] = '\0';
    }
}

int PropertyStoreInIt(PropStoreInterface_t* interface, void* dev, char* storePath)
{
    int ret = 0;

    if (interface == NULL || dev == NULL || storePath == NULL ||
        interface->init == NULL)
    {
        ret = -1;
    }
    else
    {
        ret = interface->init(dev, storePath);
    }
    
    LOG_Debug("[%s] ret = %d \n", __func__, ret);

    return ret;
}

static uint32_t CountPropertyInTable(GList* propertyKeyList, const char* table)
{
    uint32_t count = 0;
    GList* node = NULL;
    char tmpTable[16] = {0};

    if (propertyKeyList == NULL || table == NULL)
    {
        return 0;
    }

    for (node = propertyKeyList; node != NULL; node = node->next)
    {
        char* key = (char*)node->data;
        if (key == NULL)
        {
            continue;
        }

        GetPropertyTable(key, tmpTable);
        if (strcmp(tmpTable, table) == 0)
        {
            count++;
        }
    }

    return count;
}

int PropertyStoreSet(GHashTable* propertyMap, GList** propertyKeyList, PropStoreInterface_t* interface,void* dev, char* key, char* value)
{
    int ret = 0;
    char table[16] = {0};
    char* propertyValue = NULL;

    if (propertyMap == NULL || propertyKeyList == NULL || interface == NULL ||
        dev == NULL || key == NULL || value == NULL || interface->set == NULL)
    {
        ret = -1;
    }
    else
    {
        GetPropertyTable(key, table);

        propertyValue = g_hash_table_lookup(propertyMap, key);

        if ((propertyValue != NULL) && (strcmp(table, PROPERTY_TABLE_RO) == 0))
        {
            /** read only property can not be written again **/
            LOG_Debug("[%s] [%s]: %s readonly property can not be written again\n", __func__, key, propertyValue);
            ret  = -2;
        }
        else if ((propertyValue != NULL) && (strcmp(propertyValue, value) == 0))
        {
            /** property value is already the target value  **/
            ret  = 0;
        }
        else
        {
            /** new key need check table max count **/
            if (propertyValue == NULL)
            {
                uint32_t tableCount = CountPropertyInTable(*propertyKeyList, table);
                if (tableCount >= PROPERTY_TABLE_MAX_COUNT)
                {
                    LOG_Error("[%s] table [%s] reaches max count %u, key [%s] rejected\n",
                              __func__, table, (unsigned)PROPERTY_TABLE_MAX_COUNT, key);
                    ret = -3;
                }
            }

            if (ret != -3)
            {
                char* newKey = g_strdup(key);
                char* newValue = g_strdup(value);
                if (newKey == NULL || newValue == NULL)
                {
                    g_free(newKey);
                    g_free(newValue);
                    LOG_Error("[%s] g_strdup failed for key or value\n", __func__);
                    ret = -1;
                }
                else
                {
                    /** update key-value **/
                    ret = interface->set(dev, table, key, value);
                    if(ret == 0)
                    {
                        if (propertyValue != NULL)
                        {
                            /* key 已存在：先从 keyList 中移除对应节点（data 由 hash table 释放） */
                            GList* found = g_list_find_custom(*propertyKeyList, key, (GCompareFunc)g_strcmp0);
                            if (found != NULL)
                            {
                                *propertyKeyList = g_list_delete_link(*propertyKeyList, found);
                            }
                            /* GHashTable 移除旧 key-value，释放旧 key/value */
                            g_hash_table_remove(propertyMap, key);
                        }

                        /* 同时维护 keyList 顺序 与 hash table 缓存 */
                        *propertyKeyList = g_list_append(*propertyKeyList, newKey);
                        g_hash_table_insert(propertyMap, newKey, newValue);
                    }
                    else
                    {
                        g_free(newKey);
                        g_free(newValue);
                    }
                }
            }
        }
    }

    LOG_Debug("[%s] [%s]: %s ; ret = %d\n", __func__, key, value, ret);

    return ret;
}

int PropertyStoreGet(GHashTable* propertyMap, char* key, char* value, uint32_t valueSize, char* defaultValue)
{
    int ret = 0;
    char* propertyValue = NULL;
    uint32_t copyLen = 0;
    
    if (propertyMap == NULL || key == NULL || value == NULL || valueSize == 0 || defaultValue == NULL)
    {
        ret = -1;
    }
    else
    {
        propertyValue = g_hash_table_lookup(propertyMap, key);
        if (propertyValue != NULL)
        {
            copyLen = MIN(strlen(propertyValue), valueSize - 1);
            memcpy(value, propertyValue, copyLen);
            value[copyLen] = '\0';
        }
        else
        {
            copyLen = MIN(strlen(defaultValue), valueSize - 1);
            memcpy(value, defaultValue, copyLen);
            value[copyLen] = '\0';
        }
    }

    LOG_Debug("[%s] [%s]: %s ; ret = %d\n", __func__, key, value, ret);

    return ret;
}

int PropertyStoreClearTable(PropStoreInterface_t* interface, void* dev,char* table)
{
    int ret = 0;

    if (interface == NULL || dev == NULL || table == NULL || interface->clear == NULL)
    {
        ret = -1;
    }
    else
    {
        ret = interface->clear(dev, table);
    }

    LOG_Debug("[%s] [%s] ret = %d \n", __func__, table, ret);

    return ret;
}

int PropertyStoreQuery(PropStoreInterface_t* interface, void* dev, char* table, PropStoreQueryCallBack_t cbFunc, void* cbArg)
{
    int ret = 0;

    if (interface == NULL || dev == NULL || table == NULL || cbFunc == NULL ||
        interface->query == NULL)
    {
        ret = -1;
    }
    else
    {
        ret = interface->query(dev, table, cbFunc, cbArg);
    }

    LOG_Debug("[%s] [%s] ret = %d \n", __func__, table, ret);

    return ret;
}

int PropertyStoreDeInit(PropStoreInterface_t* interface, void** dev)
{
    int ret = 0;

    if (interface == NULL || dev == NULL || *dev == NULL || interface->deinit == NULL)
    {
        ret = -1;
    }
    else
    {
        ret = interface->deinit(dev);
    }

    LOG_Debug("[%s] ret = %d \n", __func__, ret);

    return ret;
}
