#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "err_code.h"
#include "sr_log.h"
#include "sr_wifi_api.h"
#include "sr_param.h"

int main(int argc, char *argv[])
{
    int32_t retValue = 0;
    SrWifiInfo_t wifiInfo;
    ParamInfo_t param;

    LOG_INIT("demo");


    // param.id = E_ParamId_UserStart + 1;
    // param.type = 1;
    // param.min = 0;
    // param.max = 128;
    // strcpy((char *)param.name, "SR_WIFI_MODE");
    // strcpy((char *)param.value, "4");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);

    // param.id = E_ParamId_UserStart + 2;
    // param.type = 0;
    // param.min = 1;
    // param.max = 128;
    // strcpy((char *)param.name, "WIFI_AP_1_SSID");
    // strcpy((char *)param.value, "SR_WIFI_AP_AP_MODE_1");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);
    // param.id = E_ParamId_UserStart + 3;
    // param.type = 0;
    // param.min = 1;
    // param.max = 128;
    // strcpy((char *)param.name, "WIFI_AP_1_PASSWD");
    // strcpy((char *)param.value, "123456789");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);

    // param.id = E_ParamId_UserStart + 4;
    // param.type = 1;
    // param.min = 0;
    // param.max = 128;
    // strcpy((char *)param.name, "SR_WIFI_CHANNEL_1_MODE");
    // strcpy((char *)param.value, "1");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);

    // param.id = E_ParamId_UserStart + 5;
    // param.type = 1;
    // param.min = 0;
    // param.max = 128;
    // strcpy((char *)param.name, "SR_WIFI_PASSWD_TYPE_1");
    // strcpy((char *)param.value, "3");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);

    // param.id = E_ParamId_UserStart + 6;
    // param.type = 1;
    // param.min = 0;
    // param.max = 128;
    // strcpy((char *)param.name, "SR_WIFI_AP_HIDE_2");
    // strcpy((char *)param.value, "0");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);


    // param.id = E_ParamId_UserStart + 7;
    // param.type = 0;
    // param.min = 1;
    // param.max = 128;
    // strcpy((char *)param.name, "WIFI_AP_2_SSID");
    // strcpy((char *)param.value, "SR_WIFI_AP_AP_MODE_2");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);
    // param.id = E_ParamId_UserStart + 8;
    // param.type = 0;
    // param.min = 1;
    // param.max = 128;
    // strcpy((char *)param.name, "WIFI_AP_2_PASSWD");
    // strcpy((char *)param.value, "123456789");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);


    // param.id = E_ParamId_UserStart + 9;
    // param.type = 1;
    // param.min = 0;
    // param.max = 128;
    // strcpy((char *)param.name, "SR_WIFI_CHANNEL_2_MODE");
    // strcpy((char *)param.value, "1");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);

    // param.id = E_ParamId_UserStart + 10;
    // param.type = 1;
    // param.min = 0;
    // param.max = 128;
    // strcpy((char *)param.name, "SR_WIFI_PASSWD_TYPE_2");
    // strcpy((char *)param.value, "3");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);

    // param.id = E_ParamId_UserStart + 11;
    // param.type = 1;
    // param.min = 0;
    // param.max = 128;
    // strcpy((char *)param.name, "SR_WIFI_AP_HIDE_2");
    // strcpy((char *)param.value, "0");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);


    // param.id = E_ParamId_UserStart + 12;
    // param.type = 1;
    // param.min = 0;
    // param.max = 128;
    // strcpy((char *)param.name, "SR_WIFI_STA_PASSWD_TYPE");
    // strcpy((char *)param.value, "3");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);
 
    // param.id = E_ParamId_UserStart + 13;
    // param.type = 0;
    // param.min = 1;
    // param.max = 128;
    // strcpy((char *)param.name, "WIFI_STA_SSID");
    // strcpy((char *)param.value, "SR_WIFI_AP_AP_MODE_1");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);
    // param.id = E_ParamId_UserStart + 14;
    // param.type = 0;
    // param.min = 1;
    // param.max = 128;
    // strcpy((char *)param.name, "WIFI_STA_PASSWD");
    // strcpy((char *)param.value, "123456789");
    // param.paramSources = PARAM_SOURCES_USER;
    // param.rpcWriteEnable = true;
    // AddUserParam(param);

    WifiDisable();

    param.id = E_ParamId_UserStart + 1;
    GetParam(&param);

    retValue = WifiSetRunMode(atoi((char *)param.value));
    if(retValue < 0)
    {
        LOG_Error("WifiSetRunMode errror");
        return RET_ERR;
    }

    wifiInfo.index = SR_WIFI_CHANNEL_ONE;
    
    // wifiInfo.mode = SR_WIFI_CHANNEL_MODE_2_4G;
    // wifiInfo.type = SR_WIFI_PASSWD_TYPE_WPA2;
    // wifiInfo.hide = SR_WIFI_AP_HIDE_OFF;
    // strcpy(wifiInfo.ssid, "SR_WIFI_AP_AP_MODE_1");
    // strcpy(wifiInfo.passwd, "123456789");

    // WifiSetApModeInfo(wifiInfo);

    // wifiInfo.index = SR_WIFI_CHANNEL_TWO;
    // wifiInfo.mode = SR_WIFI_CHANNEL_MODE_2_4G;
    // wifiInfo.type = SR_WIFI_PASSWD_TYPE_WPA2;
    // wifiInfo.hide = SR_WIFI_AP_HIDE_OFF;
    // strcpy(wifiInfo.ssid, "SR_WIFI_AP_AP_MODE_2");
    // strcpy(wifiInfo.passwd, "012345678");

    param.id = E_ParamId_UserStart + 2;
    GetParam(&param);
    strcpy(wifiInfo.ssid, (char *)param.value);
    param.id = E_ParamId_UserStart + 3;
    GetParam(&param);
    strcpy(wifiInfo.passwd, (char *)param.value);

    param.id = E_ParamId_UserStart + 4;
    GetParam(&param);    
    wifiInfo.mode = atoi((char *)param.value);
    param.id = E_ParamId_UserStart + 5;
    GetParam(&param);   
    wifiInfo.type = atoi((char *)param.value);
     param.id = E_ParamId_UserStart + 6;
    GetParam(&param); 
    wifiInfo.hide = atoi((char *)param.value);
    // strcpy(wifiInfo.ssid, "SR_WIFI_AP_AP_MODE_1");
    // strcpy(wifiInfo.passwd, "123456789");

    WifiSetApModeInfo(wifiInfo);

    wifiInfo.index = SR_WIFI_CHANNEL_TWO;
    // wifiInfo.mode = SR_WIFI_CHANNEL_MODE_2_4G;
    // wifiInfo.type = SR_WIFI_PASSWD_TYPE_WPA2;
    // wifiInfo.hide = SR_WIFI_AP_HIDE_OFF;
    // strcpy(wifiInfo.ssid, "SR_WIFI_AP_AP_MODE_2");
    // strcpy(wifiInfo.passwd, "012345678");

    param.id = E_ParamId_UserStart + 7;
    GetParam(&param);
    strcpy(wifiInfo.ssid, (char *)param.value);
    param.id = E_ParamId_UserStart + 8;
    GetParam(&param);
    strcpy(wifiInfo.passwd, (char *)param.value);

    param.id = E_ParamId_UserStart + 9;
    GetParam(&param);    
    wifiInfo.mode = atoi((char *)param.value);
    param.id = E_ParamId_UserStart + 10;
    GetParam(&param);   
    wifiInfo.type = atoi((char *)param.value);
     param.id = E_ParamId_UserStart + 11;
    GetParam(&param); 
    wifiInfo.hide = atoi((char *)param.value);

    WifiSetApModeInfo(wifiInfo);

    WifiEnable();

    return RET_OK;
}