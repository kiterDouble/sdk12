/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-09 10:31:08
 * @LastEditTime: 2019-09-18 14:00:20
 * @LastEditors: Please set LastEditors
 */
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h> 
#include <sys/types.h>
#include <stdlib.h>
#include "err_code.h"
#include "gnss_process.h"
#include "sr_hal_gnss.h"
#include "queue_msg.h"
#include "sr_gnss.h"
#include <pthread.h>

static pthread_mutex_t gpsLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gpsInitLock = PTHREAD_MUTEX_INITIALIZER;
static HalGnssLocationInfo_t g_LocationData;
static GnssNmea_t g_GPGGA;
static GnssNmea_t g_GPRMC;
static GnssLocation_t g_gpsData;
static int32_t g_gnssErrorTimes = 0;
static int32_t pmStatus = PM_STATE_NORMAL;
static int32_t checkGnssCBCount = 0;

/**
 * @brief check gnss error
 *
 * @param gnssData
 */
void gnssErrorProcess()
{
    if(pmStatus == PM_STATE_NORMAL)
    {
        g_gnssErrorTimes++;
    }
    if( g_gnssErrorTimes > GNSS_ERROR_RESET_TIME)
    {
        LOG_Error("gnss error times > %d", GNSS_ERROR_RESET_TIME);
        g_thread_new("GpsProcessReset", GpsProcessReset, NULL);
        g_gnssErrorTimes = 0;
    }
}

/*
  parse gps data
*/
static void UpdateGpsData(GnssLocation_t *gps_time_data)
{
    glong tmp_latitude = 0;
    glong tmp_longitude = 0;
    gfloat f_TmpLatitude = 0.0;
    gfloat f_TmpLongitude = 0.0;
    static uint8_t oldgpsState = 0;
    memset(gps_time_data, 0, sizeof(GnssLocation_t));
    //模块状态 0：工作正常 1：工作异常
    gps_time_data->status = 0;

    //天线状态
    HalGnssAntStatus_e antstat;
    HalGnssGetAntStatus(&antstat);
    gps_time_data->antStatus = antstat;

    //定位状态 Bit0-bit5: GNSS定位状态 0: 未定位 1: 单点定位 2: 浮点定位 3: RTK定位 Bit6: 0-东经，1-西经 Bit7: 0-北纬，1-南纬
    uint8_t gpsstat = 0;
    if(g_LocationData.gpsState == RMC_GPS_STATUS_INITIALIZING || g_LocationData.gpsState == RMC_GPS_STATUS_SINGLE_POINT)
    {
        gpsstat=g_LocationData.gpsState;
        oldgpsState = gpsstat;
    }
    else if(g_LocationData.gpsState == RMC_GPS_STATUS_CODE_DIFF || g_LocationData.gpsState == RMC_GPS_STATUS_WAAS_DIFF)
    {
        gpsstat = GPS_STATUS_RTK_POINT;
        oldgpsState = gpsstat;
    }
    else
    {
        gpsstat = oldgpsState;
    }

    //经纬度
    tmp_latitude = (g_LocationData.latitude / 100);
    f_TmpLatitude = g_LocationData.latitude - tmp_latitude * 100;
    gps_time_data->flatitude = tmp_latitude + f_TmpLatitude / 60.0;
    gps_time_data->latitudeValue = gps_time_data->flatitude * 10000000;
    
    if (g_LocationData.latitudeFlag == 'S')
    {
        /*南纬*/
        gpsstat = gpsstat + LATITUDE_FLAG_SOUTH;
        gps_time_data->flatitude = -gps_time_data->flatitude;
    }
    tmp_longitude = (g_LocationData.longitude / 100);
    f_TmpLongitude = g_LocationData.longitude - tmp_longitude * 100;
    gps_time_data->flongitude = tmp_longitude + f_TmpLongitude / 60.0;
    gps_time_data->longtitudeValue = gps_time_data->flongitude * 10000000;
    
    if (g_LocationData.longitudeFlag == 'W')
    {
        /*西经*/
        gpsstat = gpsstat + LONGITUDE_FLAG_WEST;
        gps_time_data->flongitude = -gps_time_data->flongitude;
    }

    //有RMC语句无GGA语句，这种情况会判断为未定位，但是实际上有定位数据与gps时间
    //这种情况修改为单点定位，避免timeServer不拿gnss时间
    if (0 == (gpsstat & 0x3F))
    {
        if ((gps_time_data->latitudeValue != 0) && (gps_time_data->longtitudeValue != 0))
        {
            gpsstat += 1;
        }
    }

    gps_time_data->gpsState = gpsstat;

    //海拔高度
    gps_time_data->seaHeight = g_LocationData.height * 10;

    //速度
    gps_time_data->speed = g_LocationData.speed * 1.852 * 10;

    //方向
    gps_time_data->direction = g_LocationData.direction * 10;

    //水平精度
    gps_time_data->accuracy = g_LocationData.accuracy * 100;

    //可见卫星数量
    gps_time_data->num_svs = g_LocationData.num_svs;
    
    //使用卫星数量
    gps_time_data->moonLinkNum = g_LocationData.sumNosv;

    /*时间戳*/
    struct tm utcTm;
    memset(&utcTm, 0x00, sizeof(struct tm));
    utcTm.tm_hour = (g_LocationData.time / 10000) % 24;
    utcTm.tm_min = (g_LocationData.time % 10000) / 100;
    utcTm.tm_sec = (g_LocationData.time % 100);

    //32位系统 支持年份范围1970~2038
    //date后两位：70~99，表示1970-1999年；00~38，表示2000~2038年；
    utcTm.tm_year = ((g_LocationData.date % 100 + 2000) - 1900);
    if (utcTm.tm_year >= 170 && utcTm.tm_year <= 199)
    {
        utcTm.tm_year -= 100;
    }
    utcTm.tm_mon = (((g_LocationData.date % 10000) / 100) - 1);
    utcTm.tm_mday = ((g_LocationData.date / 10000));
    utcTm.tm_isdst = 0;
    gps_time_data->timestamp = mktime(&utcTm);
    return;
}

void gnssLocationSyncMcu(GnssLocation_t *dataToPostmcu)
{
    union
    {
        double d;
        uint8_t u[8];
    } un;
    McuMesg_t sendToMcu;
    sendToMcu.type = SYS_QUE_TYPE_MCU_SYNC_POSITON;
    uint8_t reqData[128] = {0};
    uint16_t index = 0;
    // 模块状态
    reqData[index++] = dataToPostmcu->status;
    // 电线状态
    reqData[index++] = dataToPostmcu->antStatus;
    // 定位状态
    reqData[index++] = dataToPostmcu->gpsState;
    // 经度
    reqData[index++] = dataToPostmcu->longtitudeValue >> 24;
    reqData[index++] = dataToPostmcu->longtitudeValue >> 16;
    reqData[index++] = dataToPostmcu->longtitudeValue >> 8;
    reqData[index++] = dataToPostmcu->longtitudeValue;
    // 纬度
    reqData[index++] = dataToPostmcu->latitudeValue >> 24;
    reqData[index++] = dataToPostmcu->latitudeValue >> 16;
    reqData[index++] = dataToPostmcu->latitudeValue >> 8;
    reqData[index++] = dataToPostmcu->latitudeValue;
    // 经度的原始浮点形式
    un.d = dataToPostmcu->flongitude;
    for (int i = 0; i < 8; i++)
    {
        reqData[index++] = un.u[7 - i];
    }
    // 纬度的原始浮点形式
    un.d = dataToPostmcu->flatitude;
    for (int i = 0; i < 8; i++)
    {
        reqData[index++] = un.u[7 - i];
    }
    // 高程
    reqData[index++] = dataToPostmcu->seaHeight >> 8;
    reqData[index++] = dataToPostmcu->seaHeight;
    // 速度
    reqData[index++] = dataToPostmcu->speed >> 8;
    reqData[index++] = dataToPostmcu->speed;

    // 方向
    reqData[index++] = dataToPostmcu->direction >> 8;
    reqData[index++] = dataToPostmcu->direction;
    // 水平精度
    reqData[index++] = dataToPostmcu->accuracy >> 8;
    reqData[index++] = dataToPostmcu->accuracy;
    // 可见卫星数量
    reqData[index++] = dataToPostmcu->num_svs;
    // 使用卫星数量
    reqData[index++] = dataToPostmcu->moonLinkNum;
    // 时间戳
    reqData[index++] = dataToPostmcu->timestamp >> 24;
    reqData[index++] = dataToPostmcu->timestamp >> 16;
    reqData[index++] = dataToPostmcu->timestamp >> 8;
    reqData[index++] = dataToPostmcu->timestamp;
    sendToMcu.payload = reqData;
    sendToMcu.payloadLen = index;
    LOG_DebugHex(reqData,index,"McuSrvSendMesg");
    LOG_Debug("start gnss McuSrvSendMesg,len:%d",index);
    McuSrvSendMesg(&sendToMcu);
}

void GnssLocationInfoCB(HalGnssLocationInfo_t *call)
{
    g_atomic_int_set(&checkGnssCBCount, 0);
    g_gnssErrorTimes = 0;
    g_LocationData = *call;
    GnssLocation_t dataToPost;
    pthread_mutex_lock(&gpsLock);
    //转换为要发送的格式
    UpdateGpsData(&g_gpsData);
    dataToPost = g_gpsData;
    pthread_mutex_unlock(&gpsLock);

    //ipc发送 send g_gpsData
    LOG_Debug("[ipc][send][gnsslocation]:[status:%d][antStatus:%d][gpsState:%d][longtitudeValue:%d][latitudeValue:%d]",
                dataToPost.status,dataToPost.antStatus,dataToPost.gpsState,dataToPost.longtitudeValue,dataToPost.latitudeValue);
    LOG_Debug("[ipc][send][gnsslocation]:[flongitude:%f][flatitude:%f][seaHeight:%d][speed:%d][direction:%d]",
                dataToPost.flongitude,dataToPost.flatitude,dataToPost.seaHeight,dataToPost.speed,dataToPost.direction);
    LOG_Debug("[ipc][send][gnsslocation]:[accuracy:%d][num_svs:%d][moonLinkNum:%d][timestamp:%d]",
                dataToPost.accuracy,dataToPost.num_svs,dataToPost.moonLinkNum,dataToPost.timestamp);
    SendMsgToAllQueue(SYS_QUE_TYPE_GNSS_LOCATIONINFO, (void *)&dataToPost, sizeof(GnssLocation_t));
    //同步mcu

    gnssLocationSyncMcu(&dataToPost);
}

void GnssNmeaCB(HalGnssNmea_t *call)
{
    //保存NMEA数据并发送
    if ((strncmp(call->nmea, "$GPGGA", strlen("$GPGGA")) == 0) || (strncmp(call->nmea, "$GNGGA", strlen("$GNGGA")) == 0))
    {
        g_GPGGA.timestamp = call->timestamp;
        g_GPGGA.type = GNSS_NMEA_TYPE_GPGGA;
        g_GPGGA.length = call->length;
        strncpy(g_GPGGA.nmea, call->nmea, NMEA_MAX_LENGTH + 1);
        LOG_Debug("[ipc][send][NMEA][GP/GN][GGA]:[timestamp:%llu][type:%d][length:%d][nmea:%s]",
                  g_GPGGA.timestamp, g_GPGGA.type, g_GPGGA.length, g_GPGGA.nmea);
        SendMsgToAllQueue(SYS_QUE_TYPE_GNSS_NMEA, (void *)&g_GPGGA, sizeof(GnssNmea_t));
    }
    else if ((strncmp(call->nmea, "$GPRMC", strlen("$GPRMC")) == 0) || (strncmp(call->nmea, "$GNRMC", strlen("$GNRMC")) == 0))
    {
        g_GPRMC.timestamp = call->timestamp;
        g_GPRMC.type = GNSS_NMEA_TYPE_GPRMC;
        g_GPRMC.length = call->length;
        strncpy(g_GPRMC.nmea, call->nmea, NMEA_MAX_LENGTH + 1);
        LOG_Debug("[ipc][send][NMEA][GP/GN][RMC]:[timestamp:%llu][type:%d][length:%d][nmea:%s]",
                  g_GPRMC.timestamp, g_GPRMC.type, g_GPRMC.length, g_GPRMC.nmea);
        SendMsgToAllQueue(SYS_QUE_TYPE_GNSS_NMEA, (void *)&g_GPRMC, sizeof(GnssNmea_t));
    }
    else
    {

    }
}

void GpsProcessInit(void)
{
    pthread_mutex_lock(&gpsInitLock);
    HalGnssInit(GnssLocationInfoCB, GnssNmeaCB);
    usleep(100000);
    HalGnssStart();
    pthread_mutex_unlock(&gpsInitLock);
}

void *GpsProcessReset(void *pArg)
{
    HalGnssEnd();
    usleep(100000);
    HalGnssDeInit();
    usleep(100000);
    GpsProcessInit();
    return NULL;
}


void gnssServerCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    LOG_Debug("gnss RPC recv:%d",subcmd);
    if(GNSSSERVER_MSGTYPE_SUBCMD_LOCATIONINFO == subcmd)
    {
        GnssLocation_t locationData;
        *respLen = sizeof(GnssLocation_t);
        pthread_mutex_lock(&gpsLock);
        // memmove(respData, (uint8_t *)&g_gpsData, sizeof(GnssLocation_t));
        HalGnssGetLocationInfo(&g_LocationData);
        UpdateGpsData(&locationData);
        memmove(respData, (uint8_t *)&locationData, sizeof(GnssLocation_t));
        pthread_mutex_unlock(&gpsLock);
    }
    else
    {
    }
}


void HandlePmSleepCmd(const uint8_t *data, const uint16_t len, void *userdata)
{
    LOG_DebugHex(data,len,"HandlePmSleepCmd");
    MPUPmStateChange_t recvmsg;
    memset(&recvmsg, 0x00, sizeof(MPUPmStateChange_t));
    if (sizeof(MPUPmStateChange_t) == len)
    {
        memmove(&recvmsg, data, len);
    }
    else
    {
        LOG_Error("HandlePmSleepCmd recv len error!");
        return;
    }
    if (recvmsg.PmState == PM_STATE_SLEEP && pmStatus == PM_STATE_NORMAL)
    {
        pmStatus = PM_STATE_SLEEP;
        HalGnssEnd();
        usleep(100000);
        HalGnssDeInit();
    }
    else if(recvmsg.PmState == PM_STATE_NORMAL && pmStatus == PM_STATE_SLEEP)
    {   
        pmStatus = PM_STATE_NORMAL;
        GpsProcessInit();
    }
    else
    {
    }
}

bool CheckGnssCBTimer(void *user_data)
{
    if (g_atomic_int_compare_and_exchange(&checkGnssCBCount, 2, 2))
    {
        GnssLocation_t dataToPost;
        gnssErrorProcess();
        pthread_mutex_lock(&gpsLock);
        dataToPost = g_gpsData;
        pthread_mutex_unlock(&gpsLock);
        dataToPost.status = 1;   // 工作异常
        dataToPost.gpsState = 0; // 未定位

        // ipc发送 send g_gpsData
        LOG_Debug("[ipc][send][gnsslocation]:[status:%d][antStatus:%d][gpsState:%d][longtitudeValue:%d][latitudeValue:%d]",
                  dataToPost.status, dataToPost.antStatus, dataToPost.gpsState, dataToPost.longtitudeValue, dataToPost.latitudeValue);
        LOG_Debug("[ipc][send][gnsslocation]:[flongitude:%f][flatitude:%f][seaHeight:%d][speed:%d][direction:%d]",
                  dataToPost.flongitude, dataToPost.flatitude, dataToPost.seaHeight, dataToPost.speed, dataToPost.direction);
        LOG_Debug("[ipc][send][gnsslocation]:[accuracy:%d][num_svs:%d][moonLinkNum:%d][timestamp:%d]",
                  dataToPost.accuracy, dataToPost.num_svs, dataToPost.moonLinkNum, dataToPost.timestamp);
        SendMsgToAllQueue(SYS_QUE_TYPE_GNSS_LOCATIONINFO, (void *)&dataToPost, sizeof(GnssLocation_t));
        gnssLocationSyncMcu(&dataToPost);
        return true;
    }
    g_atomic_int_add(&checkGnssCBCount, 1);
    return true;
}