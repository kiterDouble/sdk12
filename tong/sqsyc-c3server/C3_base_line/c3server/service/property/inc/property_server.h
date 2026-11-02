#ifndef __PROPERTY_SERVER_H__
#define __PROPERTY_SERVER_H__

#include "property_store.h"

/**
 * GHashTable 是 GLib 库中用于实现哈希表的一个数据结构，它本质上是无序的
 * 使用 GList 维护 GHashTable 所有键值 实现 顺序维护
 * **/
typedef struct
{
    GHashTable* map;
    GList* keyList;
}PropertyMap_t;

typedef struct property_server
{
    PropStoreInterface_t *propStoreApi;
    void* propStoreDev;
    uint8_t isFirstRunning;
    PropertyMap_t propertys;
}PropertyServerContext_t;

int PropertyServerInit(void* arg);

int PropertyServerDeInit(void* arg);

#endif
