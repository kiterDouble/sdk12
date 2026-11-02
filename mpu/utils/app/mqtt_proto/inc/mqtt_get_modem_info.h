#ifndef __MQTT_GET_MODEM_INFO_H__
#define __MQTT_GET_MODEM_INFO_H__
#include <stdint.h>
#include "gnss_ipc_msg.h"
#include "mqtt_evet_handle.h"

int32_t GetCurrentGnssInfo(GnssLocation_t *currentGnssInfo);
extern void *GetModemInfoThread(void *pArg);
extern void *UartSendDataThread(void *pArg);

void SimCardInfoToMpuCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

int32_t GetCurrentSimCardInfo(SimCardInfoToMcu_t *currentSimCardInfo);

int32_t GetSimCardInfoFromMcu(SimCardInfoToMpu_t *simCardInfoFromMcu);

int32_t SrGetSystemRetValue(const char *cmd);

int SrGetInterfaceIp(const char *ifName, char *ipBuf, size_t bufLen);
#endif