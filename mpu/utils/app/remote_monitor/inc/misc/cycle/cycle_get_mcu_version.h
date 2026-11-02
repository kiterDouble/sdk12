#ifndef __CYCLE_GET_MCU_VERSION_H__
#define __CYCLE_GET_MCU_VERSION_H__
#include <stdint.h>
void McuSyncVersionToMpuCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);
#endif