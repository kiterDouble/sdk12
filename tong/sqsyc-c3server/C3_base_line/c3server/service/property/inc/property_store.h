
#ifndef __PROPERTY_FLASH_STORE_H__
#define __PROPERTY_FLASH_STORE_H__

#include <stdint.h>
#include <glib.h>

#include "property_conf.h"

#define PROPERTY_BUILD_PROP_OEMDATA "/oemdata/build.prop"
#define PROPERTY_BUILD_PROP_OEMAPP  "/oemapp/build.prop"
#define PROPERTY_SQLITE3_PATH "/oemdata/parameters/tbox_property.db"

#define PROPERTY_TABLE_PERSIST "persist"
#define PROPERTY_TABLE_RO "ro"
#define PROPERTY_TABLE_RW "sys"

#define PROPERTY_TABLE_MAX_COUNT (1024)
#define PROPERTY_BUILD_PROP_LINE_MAX (2*PROPERTY_TABLE_MAX_COUNT)


typedef void (*PropStoreQueryCallBack_t)(char* key, char* value, void* arg);

typedef int (*PropStoreInterfaceInit_t)(void* dev, char* storePath);

typedef int (*PropStoreInterfaceClear_t)(void* dev, char* table);

typedef int (*PropStoreInterfaceSet_t)(void* dev, char* table,char* key, char* value);

typedef int (*PropStoreInterfaceGet_t)(void* dev, char* table,char* key, char* value);

typedef int (*PropStoreInterfaceQuery_t)(void* dev, char* table, PropStoreQueryCallBack_t cbFunc, void* cbArg);

typedef int (*PropStoreInterfaceDeInit_t)(void** dev);

typedef struct{
    PropStoreInterfaceInit_t init;
    PropStoreInterfaceClear_t clear;
    PropStoreInterfaceSet_t set;
    PropStoreInterfaceQuery_t query;
    PropStoreInterfaceDeInit_t deinit;
}PropStoreInterface_t;

int PropertyStoreInIt(PropStoreInterface_t* interface, void* dev, char* storePath);

int PropertyStoreSet(GHashTable* propertyMap, GList** propertyKeyList, PropStoreInterface_t* interface, void* dev, char* key, char* value);

int PropertyStoreGet(GHashTable* propertyMap, char* key, char* value, uint32_t valueSize, char* defaultValue);

int PropertyStoreClearTable(PropStoreInterface_t* interface, void* dev,char* table);

int PropertyStoreQuery(PropStoreInterface_t* interface, void* dev, char* table, PropStoreQueryCallBack_t cbFunc, void* cbArg);

int PropertyStoreDeInit(PropStoreInterface_t* interface, void** dev);

extern const PropStoreInterface_t g_sqlite3_property_store_interface;

#endif
