#ifndef _PM_MCU_CONTROL_H_
#define _PM_MCU_CONTROL_H_

#include <stdint.h>
#include <glib.h>

/*
* @brief include files here
*
*/
#ifdef __cplusplus
extern "C" {
#endif

void PmSchedWakeUpMcu(void);

void PmMcuReqMpuSleepCallBack(const uint8_t *data, const uint16_t len, void* userdata);

void PmMcuReqSyncWakeupSourceCallBack(const uint8_t *data, const uint16_t len, void* userdata);

void PmMcuSyncPowerMode(const uint8_t *data, const uint16_t len, void* userdata);

gpointer PmSyncPowerBlockEventToMcu(gpointer pArg);

gpointer CheckWakeUpMcuTimeoutThread(gpointer arg);

gboolean PmInitMcuHeartbeatMonitor(void);

gboolean PmMcuHeartbeatIsAlive(void);

#ifdef __cplusplus
}
#endif

#endif //_PM_MCU_CONTROL_H_





