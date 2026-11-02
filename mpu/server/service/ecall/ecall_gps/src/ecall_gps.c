#include <string.h>

#include "ecall_gps.h"
#include "gnss_ipc_msg.h"
#include "sr_log.h"

static GnssLocation_t g_GnssLocation;
static GnssLocation_t g_GnssLocation1 = {0};
static GnssLocation_t g_GnssLocation2 = {0};
static StrGPSInfo g_GPSInfo = {0};
static int getCount = 1;

/**
* @brief 获取GPS信息
*
* @return GPSInfo_t 返回GPS信息结构体
*/
StrGPSInfo GetGpsInfo(void)
{
    g_GPSInfo.status = g_GnssLocation.gpsState;
    g_GPSInfo.direction = g_GnssLocation.direction;
    g_GPSInfo.timestamp = g_GnssLocation.timestamp;
    g_GPSInfo.latitude_value_N2 = g_GnssLocation2.flatitude;//g_GPSInfo.latitude_value_N1;
    g_GPSInfo.longtitude_value_N2 = g_GnssLocation2.flongitude;//g_GPSInfo.longtitude_value_N1;
    g_GPSInfo.latitude_value_N1 = g_GnssLocation1.flatitude;//g_GPSInfo.latitude_value;
    g_GPSInfo.longtitude_value_N1 = g_GnssLocation1.flongitude;//g_GPSInfo.longtitude_value;
    g_GPSInfo.latitude_value = g_GnssLocation.flatitude;
    g_GPSInfo.longtitude_value = g_GnssLocation.flongitude;
    g_GPSInfo.latitude_degree = g_GnssLocation.latitudeValue;
    g_GPSInfo.longtitude_degree = g_GnssLocation.longtitudeValue;

    return g_GPSInfo;
}
/**
* @brief 当接收到GPS消息时，此函数将被调用。
* @param data 接收到的GPS消息数据。
* @param len GPS消息数据的长度。
* @param userdata 回调函数传递的用户数据。
* @return void
*/
void GnssMsgSubCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
    static GnssLocation_t saveLocation = {0};

    memset(&g_GnssLocation, 0x00, sizeof(GnssLocation_t));
    if(sizeof(GnssLocation_t) == len)
    {
        memmove(&g_GnssLocation, data, len);
    }
    else
    {
        LOG_Debug("GnssMsgSubCallBack len error!");
    }
    LOG_Debug("%s - Gps : %s - latitude_value:%f , longtitude_value:%f ,latitude_degree:%d ,longtitude_degree:%d ",
    __FUNCTION__,((g_GnssLocation.gpsState & 0x1F) == 0) ? "no location" : "location",
    g_GnssLocation.flatitude,g_GnssLocation.flongitude,g_GnssLocation.latitudeValue,g_GnssLocation.longtitudeValue);

    if(getCount == 1)
    {
        getCount = 2;
        memcpy(&saveLocation, &g_GnssLocation, sizeof(GnssLocation_t));
    }
    else if(getCount == 2)
    {
        getCount = 3;
        memcpy(&g_GnssLocation1, &saveLocation, sizeof(GnssLocation_t));
        memcpy(&saveLocation, &g_GnssLocation, sizeof(GnssLocation_t));
    }
    else
    {
        memcpy(&g_GnssLocation2, &g_GnssLocation1, sizeof(GnssLocation_t));
        memcpy(&g_GnssLocation1, &saveLocation, sizeof(GnssLocation_t));
        memcpy(&saveLocation, &g_GnssLocation, sizeof(GnssLocation_t));
    }
}

