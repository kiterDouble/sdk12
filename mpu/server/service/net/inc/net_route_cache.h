#ifndef _NET_ROUTE_CACHE_H_
#define _NET_ROUTE_CACHE_H_

#include <stdint.h>

/*
* @brief include files here
*
*/


#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ETH_NAME_LEN (64)

void InitRouteCache(void);
void FreeRouteCache(void);
int CacheRoute(const char * ip, const char * dev);
int8_t QueryRoute(const char *ip, char devs[][MAX_ETH_NAME_LEN], int *count, int maxDevs);





#ifdef __cplusplus
}
#endif

#endif //_NET_ROUTE_CACHE_H_





