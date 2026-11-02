#ifndef __MQTT_GETTOKEN_H__
#define __MQTT_GETTOKEN_H__
#include "mqtt_common.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define TOKEN_BUFF_MAX_SIZE (128)

gint GetTokenInfo(gchar *buf, gint buffSize);
gint TaskRequestToken(void);

gint GetFileFromMqttTokenCheck();
gint GetHostIpAddr(const char *domainName, gint domainNameLen,
                   char       *ipAddrBuff, gint ipAddrBuffLen);

typedef enum VehicleGetTokenResponse
{
    E_RES_OK = 0,
    E_RES_ERR = -1,
    E_RES_SIGNATURE_ERROR = -2,
    E_RES_VIN_ERROR = -3,
    E_RES_OTHER = -0xFF
}VehicleGetTokenResponse;

#ifdef __cplusplus
}
#endif
#endif

