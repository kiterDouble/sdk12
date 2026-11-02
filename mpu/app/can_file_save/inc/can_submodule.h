#ifndef __CAN_SUBMODULE_H__
#define __CAN_SUBMODULE_H__

#include <stdint.h>
#include "ethernet_interface.h"

void SubmoduleHandleQueue(SafeQueue& queue);
void *TcpServerThread(void *arg);
#endif