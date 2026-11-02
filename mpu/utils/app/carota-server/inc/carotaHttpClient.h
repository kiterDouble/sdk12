#ifndef __CAROTA_HTTP_CLIENT_H__
#define __CAROTA_HTTP_CLIENT_H__ 

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>

#define DOTA_CHANGE_DOMAIN_URL   "http://127.0.0.1:20006/notifyDOTASleep"
#define DOTA_CONNECT_TIMEOUT    5L
#define DOTA_REQUEST_TIMEOUT    35L
#define SET_OTA_FIELD_URL       "http://127.0.0.1:20005/setotafield"
#define NOTIFY_FOTA_SLEEP_URL   "http://127.0.0.1:20005/notifyFOTASleep"
#define DEFAULT_TIMEOUT         120L

// 新增 HTTPS 接口 URL 定义
#if 1
#define AUTH_TOKEN_URL          "https://auth.zelostech.com.cn/vehicle/accessToken"
#define VEHICLE_ALLOW_UPGRADE_URL_BASE  "https://gateway.zelostech.com.cn/admin-server/vehicle/allowUpgrade/"
#else
#define AUTH_TOKEN_URL          "http://auth-uat.zelostech.com.cn/vehicle/accessToken"
#define VEHICLE_ALLOW_UPGRADE_URL_BASE  "https://gateway-uat.zelostech.com.cn/admin-server/vehicle/allowUpgrade/"
#endif
#define AUTH_CONNECT_TIMEOUT    10L
#define AUTH_REQUEST_TIMEOUT    30L

typedef struct {
    char *data;
    size_t len;
} CurlResponse;

gint32 CheckVehicleAllowUpgrade();
gint32 SendAuthTokenRequest(const char *serialNo, const char *vehicleName, char *tokenBuf,size_t tokenBufLen, int *vehicleId);
gint32 SendAllowUpgradeRequest(const char * vehicleId, const char *token);

size_t curl_write_callback1(void* contents, size_t size, size_t nmemb, void* userp);

gint32 ForwardRequestToPort20005(CurlResponse *curl_resp, const char *original_body);

gint32 NotifyFOTASleepMode(int mode);  // mode: 1=休眠模式, 0=唤醒模式

gint32 SendDOTAChangeDomainRequest();

#ifdef __cplusplus
}
#endif

#endif