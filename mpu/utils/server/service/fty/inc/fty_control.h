#ifndef _FTY_CONTROL_H_
#define _FTY_CONTROL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OperationResultSuccess (0)
#define OperationResultFailure (1)

extern void FtyGetPmPowerState(const uint8_t* data, const uint16_t len, void* userdata);
extern void FtyGetModemSrvInfo(const uint8_t *data, const uint16_t len, void* userdata);
extern gpointer TestTimeout(gpointer pArg);
extern void HandleTestMode(const uint8_t *data, const uint16_t len);
extern void HandleApnPing(const uint8_t *data, const uint16_t len);
extern void HandleWifi(const uint8_t *data, const uint16_t len);
extern void HandleBTConnect(const uint8_t *data, const uint16_t len);
extern void HandleCallDail(const uint8_t *data, const uint16_t len);

#ifdef __cplusplus
}
#endif

#endif //_FTY_CONTROL_H_





