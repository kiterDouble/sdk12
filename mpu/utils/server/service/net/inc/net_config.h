#ifndef _NET_CONFIG_H_
#define _NET_CONFIG_H_

#include <stdint.h>
#include "net_service.h"
/*
* @brief include files here
*
*/
#ifdef __cplusplus
extern "C" {
#endif

#define NET_CONFIG_FILE_NAME ("/oemapp/config/net.ini")
#define NET_FIRST_FILE_NAME ("/oemdata/config/tboxNet.ini")

int8_t LoadNetConfig(StrNetInfo_t *strNetInfo, uint8_t *apnNum, uint8_t *autoDail, uint8_t *dnsPathName, uint8_t pathNamLen) ;


#ifdef __cplusplus
}
#endif

#endif //_NET_CONFIG_H_





