#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "sr_wifi_api.h"

int main(int argc, char *argv[])
{
    int32_t retValue = 0;
    SrWifiInfo_t wifiInfo;

    LOG_INIT("demo");

    // WifiDisable();

    retValue = WifiSetRunMode(SR_WIFI_AP_MODE);
    if(retValue < 0)
    {
        LOG_Error("WifiSetRunMode errror");
        return RET_ERR;
    }

    wifiInfo.index = SR_WIFI_CHANNEL_ONE;
    wifiInfo.mode = SR_WIFI_CHANNEL_MODE_2_4G;
    wifiInfo.type = SR_WIFI_PASSWD_TYPE_WPA2;
    wifiInfo.hide = SR_WIFI_AP_HIDE_OFF;
    strcpy(wifiInfo.ssid, "SR_WIFI_AP_MODE");
    strcpy(wifiInfo.passwd, "12345678");

    WifiSetApModeInfo(wifiInfo);

    WifiEnable();

    return RET_OK;
}