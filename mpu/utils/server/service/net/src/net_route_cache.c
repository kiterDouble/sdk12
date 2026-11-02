#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "err_code.h"
#include "net_route_cache.h"
#include "sr_log.h"

typedef struct
{
    char *ip;
    char *dev;
} RouteBinding_t;

typedef struct 
{
    RouteBinding_t *bindings;   // 动态分配的绑定数组
    size_t size;                // 当前绑定数量
    size_t capacity;            // 缓存容量
    pthread_rwlock_t lock;      // 读写锁
} RouteCache_t;

static RouteCache_t routeCache = {NULL, 0, 0, PTHREAD_RWLOCK_INITIALIZER};
static const size_t INITIAL_CAPACITY = 128; // 初始容量
static const size_t MAX_CAPACITY = 1024;    // 最大容量

void InitRouteCache(void)
{
    routeCache.bindings = (RouteBinding_t *)malloc(INITIAL_CAPACITY * sizeof(RouteBinding_t));
    if (!routeCache.bindings) 
    {
        LOG_Error("%s : Failed to initialize route cache",__FUNCTION__);
        return;
    }
    routeCache.size = 0;
    routeCache.capacity = INITIAL_CAPACITY;
}

void FreeRouteCache(void)
{
    pthread_rwlock_wrlock(&routeCache.lock);
    for (size_t i = 0; i < routeCache.size; i++)
    {
        free(routeCache.bindings[i].ip);
        free(routeCache.bindings[i].dev);
    }
    free(routeCache.bindings);
    routeCache.bindings = NULL;
    routeCache.size = 0;
    routeCache.capacity = 0;
    pthread_rwlock_unlock(&routeCache.lock);
    pthread_rwlock_destroy(&routeCache.lock);
}

int CacheRoute(const char *ip, const char *dev)
{
    pthread_rwlock_wrlock(&routeCache.lock);

    for (size_t i = 0; i < routeCache.size; i++)
    {
        if (strcmp(routeCache.bindings[i].ip, ip) == 0 && strcmp(routeCache.bindings[i].dev, dev) == 0) 
        {
            pthread_rwlock_unlock(&routeCache.lock);
            return RET_OK; 
        }
    }

    if (routeCache.size == routeCache.capacity)
    {
        if (routeCache.capacity >= MAX_CAPACITY) 
        {
            LOG_Error("%s : Route cache capacity reached maximum limit",__FUNCTION__);
            pthread_rwlock_unlock(&routeCache.lock);
            return RET_ERR;
        }
        size_t newCapacity = routeCache.capacity * 2;
        RouteBinding_t *newBindings = (RouteBinding_t *)realloc(routeCache.bindings, newCapacity * sizeof(RouteBinding_t));
        if (!newBindings)
        {
            LOG_Error("%s : Failed to expand route cache",__FUNCTION__);
            pthread_rwlock_unlock(&routeCache.lock);
            return RET_ERR;
        }
        routeCache.bindings = newBindings;
        routeCache.capacity = newCapacity;
    }

    routeCache.bindings[routeCache.size].ip = strdup(ip);
    routeCache.bindings[routeCache.size].dev = strdup(dev);
    routeCache.size++;
    pthread_rwlock_unlock(&routeCache.lock);

    LOG_Debug("%s : add route cache success.",__FUNCTION__);
    return RET_OK;
}

int8_t QueryRoute(const char *ip, char devs[][MAX_ETH_NAME_LEN], int *count, int maxDevs)
{
    if (!ip || !devs || !count || maxDevs <= 0)
    {
        LOG_Error("%s : the parameter is wrong format.",__FUNCTION__);
        return RET_ERR;
    }

    pthread_rwlock_rdlock(&routeCache.lock);

    int devCount = 0;
    for (size_t i = 0; i < routeCache.size; i++)
    {
        if (strcmp(routeCache.bindings[i].ip, ip) == 0)
        {
            if (devCount >= maxDevs) 
            {
                break;
            }
            strncpy(devs[devCount], routeCache.bindings[i].dev, MAX_ETH_NAME_LEN - 1);
            devs[devCount][MAX_ETH_NAME_LEN - 1] = '\0'; 
            devCount++;
        }
    }

    pthread_rwlock_unlock(&routeCache.lock);

    *count = devCount;
    LOG_Debug("%s : query finish.",__FUNCTION__);
    return devCount > 0 ? RET_OK : RET_ERR;
}

