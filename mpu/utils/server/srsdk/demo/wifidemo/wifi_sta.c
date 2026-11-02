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
    int32_t status;

    LOG_INIT("demo");

    // WifiDisable();

    retValue = WifiSetRunMode(SR_WIFI_STA_MODE);
    if(retValue < 0)
    {
        printf("WifiSetRunMode errror\n");
        return RET_ERR;
    }

    wifiInfo.type = SR_WIFI_PASSWD_TYPE_WPA_WPA2;
    strcpy(wifiInfo.ssid, "Xiaomi14Pro");
    strcpy(wifiInfo.passwd, "123456789");

    retValue = WifiStaModeSetInfo(wifiInfo);
    if(retValue < 0)
    {
        printf("WifiSetRunMode errror\n");
        return RET_ERR;
    }
    WifiEnable();

    while(1)
    {
        WifiStaGetSta(&status);
        printf("status: %d\n", status);
        sleep(1);
    }

    return RET_OK;
}