
#include "cJSON.h"
#include "system_interface.h"
#include "cjson_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sr_log.h"



// 解析 JSON 的函数
int ParsePlatformJson(const char *json_string, ComponentData **components, int *component_count) 
{
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) 
    {
        LOG_Error("Error parsing JSON\n");
        return -1; // 解析失败
    }

    // 获取 success 字段
    cJSON *success = cJSON_GetObjectItemCaseSensitive(json, "success");
    if (cJSON_IsTrue(success)) {
        // 获取 data 数组
        cJSON *data_array = cJSON_GetObjectItemCaseSensitive(json, "data");
        if (cJSON_IsArray(data_array)) {
            *component_count = cJSON_GetArraySize(data_array);
            *components = (ComponentData *)malloc(*component_count * sizeof(ComponentData));

            for (int i = 0; i < *component_count; i++) {
                cJSON *item = cJSON_GetArrayItem(data_array, i);
                
                // 初始化组件数据
                ComponentData component = {0};
                
                // 获取 component 字段
                cJSON *component_item = cJSON_GetObjectItemCaseSensitive(item, "component");
                component.component = cJSON_GetStringValue(component_item) ? strdup(cJSON_GetStringValue(component_item)) : NULL;
                
                // 获取 versionId 字段
                cJSON *version_id_item = cJSON_GetObjectItemCaseSensitive(item, "versionId");
                component.versionId = version_id_item->valueint;

                // 获取 versionName 字段
                cJSON *version_name_item = cJSON_GetObjectItemCaseSensitive(item, "versionName");
                component.versionName = cJSON_GetStringValue(version_name_item) ? strdup(cJSON_GetStringValue(version_name_item)) : NULL;

                // 获取 url 字段
                cJSON *url_item = cJSON_GetObjectItemCaseSensitive(item, "url");
                component.url = cJSON_GetStringValue(url_item) ? strdup(cJSON_GetStringValue(url_item)) : NULL;

                (*components)[i] = component; // 保存组件信息
            }
        }
    }else{

        cJSON_Delete(json);
        LOG_Error("success-> false ...\n");
        return -1;
    }

    // 释放 JSON 对象
    cJSON_Delete(json);
    return 0; // 解析成功
}

// 清理分配的内存
void PlatformJsonFree(ComponentData *components, int component_count) 
{
    for (int i = 0; i < component_count; i++) 
    {
        if(components[i].component)
        {
            free(components[i].component);
            components[i].component = NULL;
        }

        if(components[i].versionName)
        {
            free(components[i].versionName);
            components[i].versionName = NULL;
        }

        if(components[i].url)    
        {
            free(components[i].url);
            components[i].url = NULL;
        }
    }

    if(components)
    {
        free(components);
        components = NULL;
    }
    
}


// 创建 JSON 数据的函数
char* CreateVehicleInformationJsonData(const UpgradeData *data) 
{
    // 创建根对象
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        return NULL; 
    }

    // 创建 currVersions 对象
    cJSON *currVersions = cJSON_CreateObject();
    if (currVersions == NULL) {
        cJSON_Delete(root);
        return NULL; 
    }
    
    // 填充 currVersions 对象
    cJSON_AddStringToObject(currVersions, "Tbox", data->currVersions.Tbox);
    cJSON_AddStringToObject(currVersions, "status", data->currVersions.status);
    // 将 currVersions 对象添加到根对象
    cJSON_AddItemToObject(root, "currVersions", currVersions);

    // 添加其他字段
    cJSON_AddStringToObject(root, "upgradeComponent", data->upgradeComponent);
    cJSON_AddNumberToObject(root, "upgradeVersionId", data->upgradeVersionId);
    cJSON_AddStringToObject(root, "upgradeVersionName", data->upgradeVersionName);
    cJSON_AddNumberToObject(root, "startTime", data->startTime);
    cJSON_AddNumberToObject(root, "endTime", data->endTime);

    // 将 JSON 对象转为字符串
    char *jsonString = cJSON_Print(root);
    if (jsonString == NULL) {
        LOG_Error("jsonString -> null");
        cJSON_Delete(root);
        return NULL; 
    }

    // 清理根对象
    cJSON_Delete(root);

    return jsonString; 
}


// 创建 JSON 数据的函数  free(json_data);
char* CreateJsonData(const char* appId, const char* appKey) 
{
    // 创建 JSON 对象
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        LOG_Error("Failed to create JSON object\n");
        return NULL;
    }

    // 添加字段
    cJSON_AddItemToObject(json, "appId", cJSON_CreateString(appId));
    cJSON_AddItemToObject(json, "appKey", cJSON_CreateString(appKey));

    // 将 JSON 对象转换为字符串
    char *json_string = cJSON_Print(json);
    if(!json_string)
    {
        LOG_Error("json_string null");
    }
    cJSON_Delete(json);  // 释放 JSON 对象
    return json_string;  // 返回 JSON 字符串
}




// 解析 JSON 数据的函数
ResponseData TokenParseJson(const char *json_string) 
{
    ResponseData responseData = {0};

    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        LOG_Error("Error parsing JSON\n");
        return responseData; // 返回默认的 ResponseData
    }

    // 解析 top-level 数据
    cJSON *success = cJSON_GetObjectItem(json, "success");
    cJSON *errorCode = cJSON_GetObjectItem(json, "errorCode");
    cJSON *message = cJSON_GetObjectItem(json, "message");
    cJSON *data = cJSON_GetObjectItem(json, "data");

    // 设置 success、errorCode 和 message
    if (success && cJSON_IsBool(success)) {
        responseData.success = cJSON_IsTrue(success);
    }

    responseData.errorCode = errorCode && !cJSON_IsNull(errorCode) ? strdup(errorCode->valuestring) : NULL;
    responseData.message = message && !cJSON_IsNull(message) ? strdup(message->valuestring) : NULL;

    // 解析 data
    if (data) 
    {
        cJSON *token = cJSON_GetObjectItem(data, "token");
        if(token == NULL)
        {
            LOG_Error("cJSON_GetObjectItem token fail...\n");
        }
        cJSON *expiresAfter = cJSON_GetObjectItem(data, "expiresAfter");
        if(expiresAfter == NULL)
        {
            LOG_Error("cJSON_GetObjectItem expiresAfter fail...\n");
        }
        cJSON *id = cJSON_GetObjectItem(data, "id");
        if(id == NULL)
        {
            LOG_Error("cJSON_GetObjectItem id fail...\n");
        }

        responseData.token = token ? strdup(token->valuestring) : NULL;
        responseData.expiresAfter = expiresAfter ? expiresAfter->valueint : 0;
        responseData.id = id ? id->valueint : 0;
    }

    cJSON_Delete(json); // 释放 JSON 对象
    return responseData; // 返回解析后的数据
}

void TokenFreeJsonData(ResponseData *data)
{
    if(!data)
    {
        return;
    }

    if(data->errorCode)
    {
        free(data->errorCode);
        data->errorCode = NULL;
    }

    if(data->message)
    {
        free(data->message);
        data->message = NULL;
    }

    if(data->token)
    {
        free(data->token);
        data->token = NULL;
    }
}
