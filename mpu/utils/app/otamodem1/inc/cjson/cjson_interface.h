#ifndef _CJSON_INTERFACE_H_
#define _CJSON_INTERFACE_H_

#include <iostream>
#include <queue>
#include <string>
#include <cstring>


#define OTASTATUSSUCCESS                    "升级成功"
#define OTASTATUSFAIL                       "升级失败"
#define OTADOWNLOADSTATUSSUCCESS            "文件下载成功"
#define OTADOWNLOADSTATUSFAIL               "文件下载失败"
#define OTADOWNLOADSTATUSSTART              "开始文件下载"

// 定义数据结构以存储解析后的数据
typedef struct {
    char *component;
    int versionId;
    char *versionName;
    char *url;
} ComponentData;

// 定义结构体来表示版本信息
typedef struct {
    char *Tbox; // 只保留 Tbox 字段
    char *status;
    char *VCU;
} CurrVersions;

// 定义参数结构体
typedef struct {
    CurrVersions currVersions;
    char *upgradeComponent;
    int upgradeVersionId;
    char *upgradeVersionName;
    long long startTime;
    long long endTime;
} UpgradeData;

typedef struct {
    int success;
    char *errorCode;
    char *message;
    char *token;
    int expiresAfter;
    int id;
} ResponseData;

int ParsePlatformJson(const char *json_string, ComponentData **components, int *component_count);
void PlatformJsonFree(ComponentData *components, int component_count) ;
char* CreateJsonData(const char* appId, const char* appKey) ;
ResponseData TokenParseJson(const char *json_string) ;
void TokenFreeJsonData(ResponseData *data);
char* CreateVehicleInformationJsonData(const UpgradeData *data);
#endif
