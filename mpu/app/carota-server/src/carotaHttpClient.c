#include <asm-generic/errno.h>
#include <curl/curl.h>
#include <unistd.h>
#include <stdio.h>
#include "carotaHttpClient.h"

#include "err_code.h"
#include "sr_log.h"
#include "cJSON.h"
#include <time.h>
#include <string.h>
#include "sdk/sr_param.h"

 size_t curl_write_callback1(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    CurlResponse *resp = (CurlResponse *)userp;

    // 动态扩容缓冲区
    char *ptr = realloc(resp->data, resp->len + realsize + 1);
    if (ptr == NULL) {
        LOG_Error("curl response buffer realloc failed");
        return 0;
    }

    resp->data = ptr;
    memcpy(&(resp->data[resp->len]), contents, realsize);
    resp->len += realsize;
    resp->data[resp->len] = '\0';
    return realsize;
}

gint32 ForwardRequestToPort20005(CurlResponse *curl_resp, const char *original_body)
{
    CURL *curl = NULL;
    struct curl_slist *headers = NULL;
    CURLcode res;
    gint32 ret = RET_ERR;
    
    // 参数检查
    if (!curl_resp || !original_body) {
        LOG_Error("Invalid parameters in ForwardRequestToPort20005");
        return RET_ERR;
    }
    
    // 初始化响应结构
    curl_resp->data = NULL;
    curl_resp->len = 0;
    
    curl = curl_easy_init();
    if (curl == NULL) {
        LOG_Error("curl init error");
        return RET_ERR;
    }
    
    // 设置curl请求参数
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_URL, SET_OTA_FIELD_URL);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, original_body);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, DEFAULT_TIMEOUT);

    // 设置回调函数捕获响应
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback1);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)curl_resp);

    // 执行curl请求
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("perform curl error = %s", curl_easy_strerror(res));
        ret = RET_ERR;
    } else {
        LOG_Debug("perform curl success, response: %s", curl_resp->data);
        ret = RET_OK;
    }
    
    // 清理资源
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return ret;
}


gint32 NotifyFOTASleepMode(int mode)
{
    CURL *curl = NULL;
    struct curl_slist *headers = NULL;
    CURLcode res;
    gint32 ret = RET_ERR;
    char post_data[64] = {0};
    
    // 参数验证
    if (mode != 0 && mode != 1) {
        LOG_Error("Invalid mode parameter: %d, should be 0(wake) or 1(sleep)", mode);
        return RET_ERR;
    }
    
    // 构造POST数据 {"mode": 1} 或 {"mode": 0}
    snprintf(post_data, sizeof(post_data), "{\"mode\": %d}", mode);
    LOG_Debug("NotifyFOTASleepMode: mode=%d, post_data=%s", mode, post_data);
    
    curl = curl_easy_init();
    if (curl == NULL) {
        LOG_Error("curl init error in NotifyFOTASleepMode");
        return RET_ERR;
    }
    
    // 设置curl请求参数
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_URL, NOTIFY_FOTA_SLEEP_URL);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, DEFAULT_TIMEOUT);

    // 执行curl请求
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("perform curl error in NotifyFOTASleepMode: %s", curl_easy_strerror(res));
        ret = RET_ERR;
    } else {
        LOG_Debug("NotifyFOTASleepMode success: mode=%d", mode);
        ret = RET_OK;
    }
    

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return ret;
}
gint32 NotifyDOTASleepMode(int mode)
{
    CURL *curl = NULL;
    struct curl_slist *headers = NULL;
    CURLcode res;
    gint32 ret = RET_ERR;
    char post_data[64] = {0};
    
    if (mode != 0 && mode != 1) {
        LOG_Error("Invalid mode parameter: %d, should be 0(wake) or 1(sleep)", mode);
        return RET_ERR;
    }
    
    // 构造POST数据 {"mode": 1} 或 {"mode": 0}
    snprintf(post_data, sizeof(post_data), "{\"mode\": %d}", mode);
    LOG_Debug("NotifyFOTASleepMode: mode=%d, post_data=%s", mode, post_data);
    
    curl = curl_easy_init();
    if (curl == NULL) {
        LOG_Error("curl init error in NotifyFOTASleepMode");
        return RET_ERR;
    }
    
    // 设置curl请求参数
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_URL, NOTIFY_DOTA_SLEEP_URL);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, DEFAULT_DOTA_TIMEOUT);

    // 执行curl请求
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("perform curl error in NotifyFOTASleepMode: %s", curl_easy_strerror(res));
        ret = RET_ERR;
    } else {
        LOG_Debug("NotifyFOTASleepMode success: mode=%d", mode);
        ret = RET_OK;
    }
    

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return ret;
}

gint32 SendDOTAChangeDomainRequest()
{
    CURL *curl = NULL;
    CURLcode res;
    long response_code = 0;
    gint32 ret = RET_ERR;

    cJSON *response_json = NULL;
    CurlResponse response;
    response.data = NULL;
    response.len = 0;

    curl = curl_easy_init();
    if (!curl) {
        LOG_Error("Failed to initialize curl");
        goto cleanup;
    }

    /* 初始化响应 buffer */
    response.data = malloc(1);
    if (!response.data) {
        LOG_Error("Failed to allocate response buffer");
        goto cleanup;
    }
    response.data[0] = '\0';

    /* curl 配置 */
    curl_easy_setopt(curl, CURLOPT_URL, DOTA_CHANGE_DOMAIN_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback1);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, DOTA_CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, DOTA_REQUEST_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    /* 执行请求 */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("curl_easy_perform failed: %s", curl_easy_strerror(res));
        goto cleanup;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    LOG_Debug("DOTAChangeDomain HTTP response code: %ld", response_code);
    LOG_Debug("Response body: %s", response.data);

    if (response_code != 200) {
        LOG_Error("Request failed, HTTP code: %ld", response_code);
        goto cleanup;
    }

    /* 解析响应 JSON */
    response_json = cJSON_Parse(response.data);
    if (!response_json) {
        LOG_Error("Failed to parse response JSON");
        goto cleanup;
    }

    cJSON *r_value = cJSON_GetObjectItem(response_json, "r");
    if (!r_value || !cJSON_IsNumber(r_value)) {
        LOG_Error("Invalid response format: missing 'r'");
        goto cleanup;
    }

    LOG_Debug("Received response r = %d", r_value->valueint);

    if (r_value->valueint == 0) {
        LOG_Debug("DOTAChangeDomain operation succeeded");
        ret = RET_OK;
    } else {
        LOG_Error("DOTAChangeDomain failed, r = %d", r_value->valueint);
        ret = RET_ERR;
    }

cleanup:
    if (response_json) {
        cJSON_Delete(response_json);
    }
    if (response.data) {
        free(response.data);
    }
    if (curl) {
        curl_easy_cleanup(curl);
    }

    return ret;
}

gint32 SendAuthTokenRequest(const char *serialNo, const char *vehicleName, char *tokenBuf,size_t tokenBufLen, int *vehicleId)
{
    CURL *curl = NULL;
    CURLcode res;
    long response_code = 0;
    gint32 ret = RET_ERR;

    struct curl_slist *headers = NULL;

    CurlResponse response;
    response.data = NULL;
    response.len = 0;

    cJSON *request_json = NULL;
    cJSON *response_json = NULL;

    char *post_data = NULL;

    curl = curl_easy_init();
    if (!curl) {
        LOG_Error("Failed to initialize curl");
        goto cleanup;
    }

    response.data = malloc(1);
    if (!response.data) {
        LOG_Error("Failed to allocate response buffer");
        goto cleanup;
    }
    response.data[0] = '\0';

    /* 构造 JSON 请求 */
    request_json = cJSON_CreateObject();
    cJSON_AddStringToObject(request_json, "serialNo", serialNo);
    cJSON_AddStringToObject(request_json, "vehicleName", vehicleName);

    post_data = cJSON_PrintUnformatted(request_json);

    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, AUTH_TOKEN_URL);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback1);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, AUTH_CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, AUTH_REQUEST_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("curl_easy_perform failed: %s", curl_easy_strerror(res));
        goto cleanup;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    LOG_Debug("kiter Response body: %s", response.data);

    if (response_code != 200) {
        LOG_Error("Request failed, HTTP code: %ld", response_code);
        goto cleanup;
    }

    response_json = cJSON_Parse(response.data);
    if (!response_json) {
        LOG_Error("Failed to parse response JSON");
        goto cleanup;
    }

    cJSON *success = cJSON_GetObjectItem(response_json, "success");
    if (!success || !cJSON_IsTrue(success)) {
        LOG_Error("Auth request failed: success != true");
        goto cleanup;
    }

    cJSON *data = cJSON_GetObjectItem(response_json, "data");
    if (!data) {
        LOG_Error("Missing data field");
        goto cleanup;
    }

    cJSON *token = cJSON_GetObjectItem(data, "token");
    cJSON *id = cJSON_GetObjectItem(data, "id");

    if (!token || !cJSON_IsString(token)) {
        LOG_Error("Missing token");
        goto cleanup;
    }

    if (!id || !cJSON_IsNumber(id)) {
        LOG_Error("Missing vehicle id");
        goto cleanup;
    }

    snprintf(tokenBuf, tokenBufLen, "%s", token->valuestring);
    LOG_Debug("%s Received token: %s", __func__, tokenBuf);
    *vehicleId = id->valueint;
    LOG_Debug("%s kiter Vehicle ID = %d", __func__, *vehicleId);

    ret = RET_OK;

cleanup:

    if (request_json)
        cJSON_Delete(request_json);

    if (response_json)
        cJSON_Delete(response_json);

    if (post_data)
        free(post_data);

    if (headers)
        curl_slist_free_all(headers);

    if (response.data)
        free(response.data);

    if (curl)
        curl_easy_cleanup(curl);

    return ret;
}

gint32 SendAllowUpgradeRequest(const char * vehicleId, const char *token)
{
    CURL *curl = NULL;
    CURLcode res;
    long response_code = 0;
    gint32 ret = 1;
    LOG_Debug("%s kiter vehicleId = %s,Token = %s", __func__, vehicleId,token);
    struct curl_slist *headers = NULL;
    CurlResponse response;
    response.data = NULL;
    response.len = 0;

    cJSON *response_json = NULL;

    char url[512];
    char tokenHeader[1024];

    curl = curl_easy_init();
    if (!curl) {
        LOG_Error("Failed to initialize curl");
        goto cleanup;
    }

    response.data = malloc(1);
    if (!response.data) {
        LOG_Error("Failed to allocate response buffer");
        goto cleanup;
    }
    response.data[0] = '\0';

    snprintf(url, sizeof(url), "%s%s", VEHICLE_ALLOW_UPGRADE_URL_BASE, vehicleId);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    snprintf(tokenHeader, sizeof(tokenHeader), "token: %s", token);
    headers = curl_slist_append(headers, tokenHeader);
    LOG_Debug("%s kiter Token header: %s", __func__, tokenHeader);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback1);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, AUTH_CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, AUTH_REQUEST_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("curl_easy_perform failed: %s", curl_easy_strerror(res));
        goto cleanup;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    LOG_Debug("kiter AllowUpgrade HTTP response code: %ld", response_code);
    LOG_Debug("kiter Response body: %s", response.data);

    if (response_code != 200) {
        LOG_Error("Request failed, HTTP code: %ld", response_code);
        goto cleanup;
    }

    response_json = cJSON_Parse(response.data);
    if (!response_json) {
        LOG_Error("Failed to parse response JSON");
        goto cleanup;
    }
    LOG_Debug("%s AllowUpgrade response: %s", __func__, response.data);

    cJSON *success = cJSON_GetObjectItem(response_json, "success");
    if (!success || !cJSON_IsTrue(success)) {
        LOG_Error("AllowUpgrade failed: success != true");
        goto cleanup;
    }

    cJSON *data = cJSON_GetObjectItem(response_json, "data");
    if (!data || !cJSON_IsBool(data)) {
        LOG_Error("Invalid data field");
        goto cleanup;
    }

    if (cJSON_IsTrue(data)) {
        LOG_Debug("Vehicle allow upgrade = true");
        ret = 0;
    } else {
        LOG_Error("Vehicle allow upgrade = false");
        ret = 1;
    }

cleanup:

    if (response_json)
        cJSON_Delete(response_json);

    if (headers)
        curl_slist_free_all(headers);

    if (response.data)
        free(response.data);

    if (curl)
        curl_easy_cleanup(curl);

    return ret;
}

gint32 CheckVehicleAllowUpgrade()
{
    char token[1024] = {0};
    gchar vehicleName[18] = "0";
    gchar serialNo[18] = "0";
    int vehicleId = 0;
    ParamInfo_t param;
    param.id = 102;//E_ParamId_CarNum
	if (RET_OK != GetParam(&param))
	{
		LOG_Error("get tbox vn fail.");
	}
    memcpy(vehicleName, param.value, sizeof(vehicleName));
    param.id = 4;//E_ParamId_CarNum
	if (RET_OK != GetParam(&param))
	{
		LOG_Error("get tbox vn fail.");
	}
    memcpy(serialNo, param.value, sizeof(serialNo));

    LOG_Debug("%s Vehicle Name = %s,serialNo = %s", __func__, vehicleName, serialNo);
    if (SendAuthTokenRequest(serialNo,
                             vehicleName,
                             token,
                             sizeof(token),
                             &vehicleId) != RET_OK) {
        LOG_Error("Get token failed");
        return 1;
    }

    int ret = SendAllowUpgradeRequest(vehicleName, token);
    if(ret == 1) 
    {
        LOG_Error("Vehicle not allowed to upgrade");
        return 1;
    }   

    return 0;
}

gint32 ForwardDomainSwitchRequest(const char *original_body)
{
    CURL *curl = NULL;
    struct curl_slist *headers = NULL;
    CURLcode res;
    gint32 ret = RET_ERR;
    CurlResponse response;
    
    if (!original_body) {
        LOG_Error("Invalid parameters in ForwardDomainSwitchRequest");
        return RET_ERR;
    }
    
    LOG_Debug("ForwardDomainSwitchRequest: body=%s", original_body);
    
    response.data = NULL;
    response.len = 0;
    
    curl = curl_easy_init();
    if (curl == NULL) {
        LOG_Error("curl init error in ForwardDomainSwitchRequest");
        return RET_ERR;
    }
    
    // 主模组使用 MAINDOMAIN_SWITCH_FORWARD_URL，副模组使用 SLAVEDOMAIN_SWITCH_FORWARD_URL
    // 追加标记，让对端识别这是转发请求，避免主副模组互相转发形成循环
    const char *baseForwardUrl = (0 == access("/dev/mmcblk0", F_OK))
                                     ? MAINDOMAIN_SWITCH_FORWARD_URL
                                     : SLAVEDOMAIN_SWITCH_FORWARD_URL;
    char forwardUrl[256] = {0};
    snprintf(forwardUrl, sizeof(forwardUrl), "%s?%s", baseForwardUrl, DOMAIN_SWITCH_FORWARD_MARK);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, forwardUrl);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, original_body);    
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);    
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, DOTA_CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, DOMAIN_SWITCH_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback1);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("perform curl error in ForwardDomainSwitchRequest: %s", curl_easy_strerror(res));
        ret = RET_ERR;
        goto cleanup;
    }
    
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    LOG_Debug("ForwardDomainSwitchRequest HTTP response code: %ld", response_code);
    
    if (response.data) {
        LOG_Debug("ForwardDomainSwitchRequest response body: %s", response.data);
    }
    
    if (response_code == 200) {
        LOG_Debug("ForwardDomainSwitchRequest success");
        ret = RET_OK;
    } else {
        LOG_Error("ForwardDomainSwitchRequest failed, HTTP code: %ld", response_code);
        ret = RET_ERR;
    }
    
cleanup:
    // 清理资源
    if (response.data) {
        free(response.data);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return ret;
}
