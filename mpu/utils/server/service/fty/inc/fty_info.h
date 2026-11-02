#ifndef _FTY_INFO_H_
#define _FTY_INFO_H_

#include <stdint.h>
#include "glib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    E_TEST_MODE = 1,
    E_APN_PING_START,
    E_APN_IP_PING_RESULT,
    E_WIFI_CONNECT_SPECIFIC_NETWORK,
    E_WIFI_CONNECT_TEST_RESULT,
    E_BLUETOOTH_CONNECT_COMMAND,
    E_BLUETOOTH_CONNECT_TEST_QUERY,
    E_DIAL_TEST_COMMAND,
    E_DIAL_TEST_RESULT_QUERY,
    E_SET_BLUETOOTH_NAME,
    E_GET_BLUETOOTH_NAME,
    E_SPEAKER_TEST,
    E_RESERVED_START = 13,
    E_RESERVED_END = 39,
    E_GET_4G_MODULE_SIGNAL_STRENGTH = 40,
    E_GET_NETWORK_TYPE,
    E_NETWORK_APN_COUNT,
    E_NETWORK_PROVIDER,
    E_GET_GNSS_INFO,
    E_GET_FLASH_STORAGE_SPACE
} CommandId_e;

extern void GetInfoProcess(const uint8_t* data, const uint16_t len, void* userdata);
extern void SetCsq(guint value);
extern void SetEcalStatus(guint value);
extern void SetNetType(guint value);
extern void SetProviderName(gchar value[]);
extern void SetApnTestResult(guint value);
extern void SetWifiTestResult(guint value);
extern void SetNetRegStatus(uint8_t value);
extern void SetApnNumber(uint8_t value);
extern void SetCallTestResult(guint value);
extern void FtyGnssMsgSubCallBack(const uint8_t* data, const uint16_t len, void* userdata);
extern gpointer SendInfoToMCU(gpointer pArg);
extern void ProcessRequest(const uint8_t* data, const uint16_t len,void* userdata);
extern int32_t SendFtyInfoAck(uint8_t status, uint8_t commandId, uint8_t comResultLen, uint8_t *comResultValue);


#ifdef __cplusplus
}
#endif

#endif //_FTY_INFO_H_




