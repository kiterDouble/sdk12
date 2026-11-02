#ifndef _NET_RPC_H_
#define _NET_RPC_H_

#include <stdint.h>

/*
* @brief include files here
*
*/
#ifdef __cplusplus
extern "C" {
#endif

void InitIpcRpcModules(void* arg);
void NetIpcSendNetState(void* arg);

#ifdef __cplusplus
}
#endif

#endif //_NET_RPC_H_




