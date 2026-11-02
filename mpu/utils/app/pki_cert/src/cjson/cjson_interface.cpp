#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sr_log.h"

#include "cJSON.h"
#include "cjson_interface.h"
#include "system_interface.h"


// 函数接口，封装 JSON 数据
char* CreateJson(Pki_ComponentData data) 
{
    
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        LOG_Error("Failed to create JSON object.");
        return NULL;
    }

    // 添加 car_id 键值对
    if (cJSON_AddStringToObject(json, "car_id", data.car_id) == NULL) {
        LOG_Error("Failed to add car_id.");
        cJSON_Delete(json); // 释放 JSON 对象
        return NULL;
    }

    // 添加 part_type 键值对
    if (cJSON_AddStringToObject(json, "part_type", data.part_type) == NULL) {
        LOG_Error("Failed to add part_type.");
        cJSON_Delete(json); // 释放 JSON 对象
        return NULL;
    }

    // 转换为字符串
    char *json_string = cJSON_Print(json);
    LOG_Debug("Generated JSON: %s\n", json_string);
    cJSON_Delete(json); // 删除 JSON 对象以释放内存

    return json_string; // 返回 JSON 字符串
}




// 解析 JSON 字符串并将结果存入 ApiResponse 结构体
static int ParseJson(const char *json_string, Pki_Response **response) 
{
    // 解析 JSON 字符串
    cJSON *root_item = cJSON_Parse((json_string));
    if (root_item == NULL) {
        LOG_Error("Error parsing JSON");
        return -1; // 返回错误标志
    }
    
    *response = (Pki_Response *)malloc(sizeof(Pki_Response));
    if (*response == NULL) {
        LOG_Error("Memory allocation failed");
        cJSON_Delete(root_item);
        return -1; // 返回错误标志
    }
    // 获取 status 字段
    cJSON *status_item = cJSON_GetObjectItemCaseSensitive(root_item, "status");

    if ((status_item == NULL) || (!(status_item->valuestring))) {
        LOG_Error("Status field not found or not a string");
        cJSON_Delete(root_item);
        free((*response));
        (*response) = NULL;
        return -1; // 返回错误标志
    }

    (*response)->status = strdup(cJSON_GetStringValue(status_item));
    if (((*response)->status) == NULL) {
        LOG_Error("Memory allocation failed for status");
        cJSON_Delete(root_item);
        free((*response));
        (*response) = NULL;
        return -1; // 返回错误标志
    }

    // 检查 status 是否为 "success"
    if (strcmp((*response)->status, "success") == 0) {
        // 获取 private_key 字段
        //cJSON *private_key_item = cJSON_GetObjectItem(root_item, "private_key");
        cJSON *private_key_item = cJSON_GetObjectItemCaseSensitive(root_item, "private_key");
        if ((private_key_item == NULL) || (!(private_key_item->valuestring))) {
            LOG_Error("Private key field not found or not a string");
            cJSON_Delete(root_item);
            free(((*response)->status));
            free((*response));
            ((*response)->status) = NULL;
            (*response) = NULL;
            return -1; // 返回错误标志
        }

        // 分配内存并复制 private_key 值
        (*response)->private_key = strdup(cJSON_GetStringValue(private_key_item));
        if (((*response)->private_key) == NULL) {
            LOG_Error("Memory allocation failed for private key");
            cJSON_Delete(root_item);
            free((*response)->status);
            free(*response);
            ((*response)->status) = NULL;
            (*response) = NULL;
            return -1; // 返回错误标志
        }

        // 获取 certificate 字段
        cJSON *certificate_item = cJSON_GetObjectItem(root_item, "certificate");
        if ((private_key_item == NULL) ||  (!(certificate_item->valuestring))) {
            LOG_Error("Certificate field not found or not a string");
            cJSON_Delete(root_item);
            free(((*response)->private_key));
            free(((*response)->status));
            free((*response));
            ((*response)->status) = NULL;
            (*response) = NULL;
            ((*response)->private_key) = NULL;
            return -1; // 返回错误标志
        }

        // 分配内存并复制 certificate 值
        (*response)->certificate = strdup(cJSON_GetStringValue(certificate_item));
        if (((*response)->certificate) == NULL) {
            LOG_Error("Memory allocation failed for certificate");
            cJSON_Delete(root_item);
            free(((*response)->private_key));
            free(((*response)->status));
            free((*response));
            ((*response)->status) = NULL;
            ((*response)->private_key) = NULL;
            (*response) = NULL;
            return -1; // 返回错误标志
        }
    } else {
        LOG_Debug("Status is not success, received status: %s\n", (*response)->status);
        free(((*response)->status));
        free((*response));
        ((*response)->status) = NULL;
        (*response) = NULL;
        cJSON_Delete(root_item);

        return -1; // 返回错误标志
    }

    // 释放 cJSON 对象
    cJSON_Delete(root_item);

   
    return 0; // 返回成功标志
}


char* CreateJsonInterface()
{
    Pki_ComponentData data;
    unsigned char carid[30] = {0};

    if(GetCarNum(carid, sizeof(carid)))
    {
        data.car_id = (char *)carid;
    }else{
        LOG_Error("GetCarNum fail");
        return NULL;
    }

    data.part_type = "tbox";
    if(!CreateJson(data))
    {
        LOG_Error("CreateJson fail");
    }
    return CreateJson(data);
}

void FreeParseJson(Pki_Response **response)
{
    #if 0
    LOG_Debug("FreeParseJson start");
    if(response)
    {
        if(response->status)
        {
            free(response->status);
            response->status = NULL;
        }

        if(response->private_key)
        {
            free(response->private_key);
            response->private_key = NULL;
        }

        if(response->certificate)
        {
            free(response->certificate);
            response->certificate = NULL;
        }

        if(response)
        {
            free(response);
            response = NULL;
        }
        
    }else{
        LOG_Error("FreeParseJson response NULL");
    }
    #endif
      LOG_Debug("FreeParseJson start");
    if ((response) && (*response)) {  
        Pki_Response *res = (*response);  
        if (res->status) {
            free(res->status);
            res->status = NULL;
        }

        if (res->private_key) {
            free(res->private_key);
            res->private_key = NULL;
        }

        if (res->certificate) {
            free(res->certificate);
            res->certificate = NULL;
        }

        free(res);  // 释放结构体本身
        *response = NULL;  // 指向指针的指针设置为 NULL
    } else {
        LOG_Error("FreeParseJson response NULL");
    }
}
 
//释放分配的内存   FreeParseJson(response);
int ParseJsonInterface(const char *string,Pki_Response **response)
{
    LOG_Error("len: %d\n", strlen(string));
    if (ParseJson(string, response) == 0) {
        // 输出结果
        //printf("Private Key: %s\n", (*response)->private_key);
        //printf("Certificate: %s\n", (*response)->certificate);
        LOG_Error("Status: %s\n", (*response)->status);
       
        return 0;

    } else {
        LOG_Error("Failed to parse JSON or invalid status.");
        LOG_Error("json string %s",string);
       
        return -1;
    }
}



