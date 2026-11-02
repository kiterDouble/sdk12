#ifndef __MQTT_GATEWAY_H__
#define __MQTT_GATEWAY_H__

#include <stdint.h>

void *ControlGatewayThread(void *pArg);

int32_t InitMcuSwitchEventHandler(void);

void McuControlMpuGatewayCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

void McuSyncSwitchPowerUpCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

void McuSyncSwitchPowerDownCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

void McuSyncSwitchResetCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

int32_t ClearSwitchUpStatus();

uint8_t GetGatewayLocation();

#endif
