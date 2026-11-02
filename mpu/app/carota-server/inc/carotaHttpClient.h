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
#define SET_OTA_FIELD_URL       "http://127.0.0.1:20005/setotafield?dev=tbox"
#define NOTIFY_FOTA_SLEEP_URL   "http://127.0.0.1:20005/notifyFOTASleep"
#define DEFAULT_TIMEOUT         15L

#define NOTIFY_DOTA_SLEEP_URL   "http://127.0.0.1:20006/notifyDOTASleep"
#define DEFAULT_DOTA_TIMEOUT         35L

// 新增域名切换转发地址
#define MAINDOMAIN_SWITCH_FORWARD_URL   "http://192.168.1.202:20003/TBOXSetDomain"
#define SLAVEDOMAIN_SWITCH_FORWARD_URL   "http://192.168.1.202:20003/TBOXSetDomain"
// 转发标记：带此 query 参数的是对端转发来的请求，接收方不再二次转发，避免主副模组循环调用
#define DOMAIN_SWITCH_FORWARD_MARK      "forwarded=1"
#define DOMAIN_SWITCH_TIMEOUT       10L

#define AUTH_TOKEN_URL          "https://auth.zelostech.com.cn/vehicle/accessToken"
#define VEHICLE_ALLOW_UPGRADE_URL_BASE  "https://gateway.zelostech.com.cn/admin-server/vehicle/allowUpgrade/"

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

gint32 NotifyDOTASleepMode(int mode);

gint32 SendDOTAChangeDomainRequest();

/**
 * @brief 转发域名切换请求到 192.168.1.202:20003
 * @param original_body 原始请求体 JSON 字符串，格式: {"target": "prod/uat", "platform": "fota/dota"}
 * @return RET_OK 成功, RET_ERR 失败
 */
gint32 ForwardDomainSwitchRequest(const char *original_body);

#ifdef __cplusplus
}
#endif

#endif