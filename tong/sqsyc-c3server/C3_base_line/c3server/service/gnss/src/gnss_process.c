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
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>
#include "err_code.h"
#include "gnss_process.h"
#include "sr_hal_gnss.h"
#include "queue_msg.h"
#include "sr_gnss.h"


static pthread_mutex_t gpsLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gpsInitLock = PTHREAD_MUTEX_INITIALIZER;
static HalGnssLocationInfo_t g_LocationData;
static GnssNmea_t g_GPGGA;
static GnssNmea_t g_GPRMC;
static GnssNmea_t g_GSA;
static GnssNmea_t g_GSV;
static GnssNmea_t g_VTG;
static GnssNmea_t g_GLL;
static GnssLocation_t g_gpsData;
static int32_t g_gnssErrorTimes = 0;
static int32_t pmStatus = PM_STATE_NORMAL;
static int32_t checkGnssCBCount = 0;
static pthread_t g_injectThread = 0;
static volatile int g_injectThreadExit = 0;   /* 退出标志 */
static uint8_t g_pinNumber = 0; // gnss指示灯pin

static bool NmeaStartsWith(const char *nmea, const char *prefix)
{
    bool ret = false;

    if ((nmea != NULL) && (prefix != NULL))
    {
        const size_t prefixLen = strlen(prefix);
        if (strncmp(nmea, prefix, prefixLen) == 0)
        {
            ret = true;
        }
    }

    return ret;
}

static void CopyNmeaSafe(char *dst, uint32_t dstSize, const char *src, uint32_t srcLen)
{
    uint32_t copyLen = 0U;

    if ((dst != NULL) && (src != NULL) && (dstSize > 0U))
    {
        /* 目标至少留 1 字节给 '\0' */
        copyLen = srcLen;
        if (copyLen >= dstSize)
        {
            copyLen = dstSize - 1U;
        }

        if (copyLen > 0U)
        {
            (void)memcpy(dst, src, (size_t)copyLen);
        }
        dst[copyLen] = '\0';
    }
}

static bool IsGgaSentence(const char *nmea)
{
    /* GPS / Multi-GNSS / BeiDou(新) / BeiDou(旧兼容) */
    return (NmeaStartsWith(nmea, "$GPGGA") ||
            NmeaStartsWith(nmea, "$GNGGA") ||
            NmeaStartsWith(nmea, "$BDGGA") ||
            NmeaStartsWith(nmea, "$GBGGA"));
}

static bool IsRmcSentence(const char *nmea)
{
    return (NmeaStartsWith(nmea, "$GPRMC") ||
            NmeaStartsWith(nmea, "$GNRMC") ||
            NmeaStartsWith(nmea, "$BDRMC") ||
            NmeaStartsWith(nmea, "$GBRMC"));
}

static bool IsGsaSentence(const char *nmea)
{
    return (NmeaStartsWith(nmea, "$GPGSA") ||
            NmeaStartsWith(nmea, "$GNGSA") ||
            NmeaStartsWith(nmea, "$BDGSA") ||
            NmeaStartsWith(nmea, "$GBGSA"));
}

static bool IsGsvSentence(const char *nmea)
{
    return (NmeaStartsWith(nmea, "$GPGSV") ||
            NmeaStartsWith(nmea, "$GNGSV") ||
            NmeaStartsWith(nmea, "$BDGSV") ||
            NmeaStartsWith(nmea, "$GBGSV"));
}

static bool IsVtgSentence(const char *nmea)
{
    return (NmeaStartsWith(nmea, "$GPVTG") ||
            NmeaStartsWith(nmea, "$GNVTG") ||
            NmeaStartsWith(nmea, "$BDVTG") ||
            NmeaStartsWith(nmea, "$GBVTG"));
}

static bool IsGllSentence(const char *nmea)
{
    return (NmeaStartsWith(nmea, "$GPGLL") ||
            NmeaStartsWith(nmea, "$GNGLL") ||
            NmeaStartsWith(nmea, "$BDGLL") ||
            NmeaStartsWith(nmea, "$GBGLL"));
}

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
    /* mktime 按本地时区解释，此处 utcTm 为 GNSS UTC 时间，须用 timegm */
    gps_time_data->timestamp = timegm(&utcTm);
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

    if ((call == NULL))
    {
        return;
    }

    /* 防御：length 过大时，CopyNmeaSafe 会截断 */
    if (IsGgaSentence(call->nmea))
    {
        g_GPGGA.timestamp = call->timestamp;
        g_GPGGA.type = GNSS_NMEA_TYPE_GPGGA;
        g_GPGGA.length = call->length;
        CopyNmeaSafe(g_GPGGA.nmea, (uint32_t)(NMEA_MAX_LENGTH + 1U), call->nmea, call->length);

        LOG_Debug("[ipc][send][NMEA][GGA]:[timestamp:%llu][type:%d][length:%d][nmea:%s]",
                  g_GPGGA.timestamp, g_GPGGA.type, g_GPGGA.length, g_GPGGA.nmea);
        SendMsgToAllQueue(SYS_QUE_TYPE_GNSS_NMEA, (void *)&g_GPGGA, sizeof(GnssNmea_t));
    }
    else if (IsRmcSentence(call->nmea))
    {
        g_GPRMC.timestamp = call->timestamp;
        g_GPRMC.type = GNSS_NMEA_TYPE_GPRMC;
        g_GPRMC.length = call->length;
        CopyNmeaSafe(g_GPRMC.nmea, (uint32_t)(NMEA_MAX_LENGTH + 1U), call->nmea, call->length);

        LOG_Debug("[ipc][send][NMEA][RMC]:[timestamp:%llu][type:%d][length:%d][nmea:%s]",
                  g_GPRMC.timestamp, g_GPRMC.type, g_GPRMC.length, g_GPRMC.nmea);
        SendMsgToAllQueue(SYS_QUE_TYPE_GNSS_NMEA, (void *)&g_GPRMC, sizeof(GnssNmea_t));
    }
    else if (IsGsaSentence(call->nmea))
    {
        g_GSA.timestamp = call->timestamp;
        g_GSA.type = GNSS_NMEA_TYPE_GSA;
        g_GSA.length = call->length;
        CopyNmeaSafe(g_GSA.nmea, (uint32_t)(NMEA_MAX_LENGTH + 1U), call->nmea, call->length);

        LOG_Debug("[ipc][send][NMEA][GSA]:[timestamp:%llu][type:%d][length:%d][nmea:%s]",
                  g_GSA.timestamp, g_GSA.type, g_GSA.length, g_GSA.nmea);
        SendMsgToAllQueue(SYS_QUE_TYPE_GNSS_NMEA, (void *)&g_GSA, sizeof(GnssNmea_t));
    }
    else if (IsGsvSentence(call->nmea))
    {
        g_GSV.timestamp = call->timestamp;
        g_GSV.type = GNSS_NMEA_TYPE_GSV;
        g_GSV.length = call->length;
        CopyNmeaSafe(g_GSV.nmea, (uint32_t)(NMEA_MAX_LENGTH + 1U), call->nmea, call->length);

        LOG_Debug("[ipc][send][NMEA][GSV]:[timestamp:%llu][type:%d][length:%d][nmea:%s]",
                  g_GSV.timestamp, g_GSV.type, g_GSV.length, g_GSV.nmea);
        SendMsgToAllQueue(SYS_QUE_TYPE_GNSS_NMEA, (void *)&g_GSV, sizeof(GnssNmea_t));
    }
    else if (IsVtgSentence(call->nmea))
    {
        g_VTG.timestamp = call->timestamp;
        g_VTG.type = GNSS_NMEA_TYPE_VTG;
        g_VTG.length = call->length;
        CopyNmeaSafe(g_VTG.nmea, (uint32_t)(NMEA_MAX_LENGTH + 1U), call->nmea, call->length);

        LOG_Debug("[ipc][send][NMEA][VTG]:[timestamp:%llu][type:%d][length:%d][nmea:%s]",
                  g_VTG.timestamp, g_VTG.type, g_VTG.length, g_VTG.nmea);
        SendMsgToAllQueue(SYS_QUE_TYPE_GNSS_NMEA, (void *)&g_VTG, sizeof(GnssNmea_t));
    }
    else if (IsGllSentence(call->nmea))
    {
        g_GLL.timestamp = call->timestamp;
        g_GLL.type = GNSS_NMEA_TYPE_GLL;
        g_GLL.length = call->length;
        CopyNmeaSafe(g_GLL.nmea, (uint32_t)(NMEA_MAX_LENGTH + 1U), call->nmea, call->length);

        LOG_Debug("[ipc][send][NMEA][GLL]:[timestamp:%llu][type:%d][length:%d][nmea:%s]",
                  g_GLL.timestamp, g_GLL.type, g_GLL.length, g_GLL.nmea);
        SendMsgToAllQueue(SYS_QUE_TYPE_GNSS_NMEA, (void *)&g_GLL, sizeof(GnssNmea_t));
    }
    else
    {
        /* do nothing */
        LOG_Debug("[ipc][send][NMEA][unknown]:[timestamp:%llu][length:%d][nmea:%s]",
                  call->timestamp, call->length, call->nmea);
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
        StopInjectAgnssDataThread();
    }
    else if(recvmsg.PmState == PM_STATE_NORMAL && pmStatus == PM_STATE_SLEEP)
    {   
        pmStatus = PM_STATE_NORMAL;
        GpsProcessInit();
        StartInjectAgnssDataThread();
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

/**
 * @brief   检查车企付费公网 APN（privateNet == 0）是否拨号成功
 *
 * @return  1：拨号成功
 *          0：拨号失败（已重试最大次数）
 **/
static int CheckPublicApnDialSuccess(void)
{
    StrNetStatus_t netStatus = {0};
    int retry = 0;
    int ret   = -1;
    uint8_t dialStatus = 0U;

    // 初次查询拨号状态
    ret = GetNetStatusInfo(&netStatus, 3000);
    if (ret != RET_OK) {
        LOG_Error("GetNetStatusInfo failed");
    } else {
        // 查找到公网 APN（privateNet == 0）
        for (uint8_t i = 0U; i < netStatus.apnNum; i++) {
            if (netStatus.netStatusInfo[i].privateNet == 0U) {
                dialStatus = netStatus.netStatusInfo[i].dialStatus;
                break;
            }
        }
        // 拨号成功返回
        if (dialStatus == 1U) {
            LOG_Info("Apn public dial success (first check)");
            return 1;
        }
    }
    // 循环重试
    while (retry < DIAL_RETRY_MAX) {
        if (g_injectThreadExit) {
            break;
        }
        retry++;
        (void)sleep(1);

        (void)memset(&netStatus, 0, sizeof(netStatus));
        ret = GetNetStatusInfo(&netStatus, 3000);
        if (ret != RET_OK) {
            LOG_Error("Retry GetNetStatusInfo failed %d/%d", retry, DIAL_RETRY_MAX);
            continue;
        }
        // 查找到公网 APN（privateNet == 0）
        for (uint8_t i = 0U; i < netStatus.apnNum; i++) {
            if (netStatus.netStatusInfo[i].privateNet == 0U) {
                dialStatus = netStatus.netStatusInfo[i].dialStatus;
                break;
            }
        }
        if (dialStatus == 1U) {
            LOG_Info("Apn public dial success");
            return 1;
        }
        LOG_Warn("Apn public dial failed, retry:%d/%d", retry, DIAL_RETRY_MAX);
    }
    LOG_Warn("APN Public Dial failed %d times,g_injectThreadExit:%d", retry,g_injectThreadExit);
    return 0;
}


/**
 * @brief   添加 AGNSS 下载/注入所需的路由配置
 *
 * @return  无
 **/
static void AddAgnssRouteIfNeeded(void)
{
    StrSetNetRoute_t routeConfig = {
        .priority = 100,
        .route = AGPS_HOST_NAME,
        .routeDevice = NET_ROUTE_DEVICE_MODEM,
        .deviceField = {.apnId = 1}
    };
    StrAddNetRouteResp_t routeResp = {0};

    if (AddNetRouteConfig(&routeConfig, &routeResp, 0) != 0) {
        LOG_Error("Add agnss NetRouteConfig failed");
    } else {
        int i;
        LOG_Debug("AddNetRouteConfig success, resolved ipCount: %d", routeResp.ipCount);
        for (i = 0; i < routeResp.ipCount; i++) {
            LOG_Debug("  resolved IP[%d]: %s", i, routeResp.ip[i]);
        }
        LOG_Debug("RouteConfig: Priority=%u, Route=%s, Device=%u, DeviceField={%u|%s}",
                  routeConfig.priority, routeConfig.route, routeConfig.routeDevice,
                  (routeConfig.routeDevice == NET_ROUTE_DEVICE_MODEM) ? routeConfig.deviceField.apnId : 0,
                  (routeConfig.routeDevice == NET_ROUTE_DEVICE_OTHER) ? routeConfig.deviceField.ethName : "");
    }
}

int CopyFileToDir(const char *src_file, const char *dst_dir)
{
    int ret = -1;
    FILE *src = NULL;
    FILE *dst = NULL;
    unsigned char b = 0U;
    size_t n = 0U;
    char dst_file[256] = {0};
    const char *filename = NULL;
    /* 参数检查 */
    if ((src_file == NULL) || (dst_dir == NULL)) {
        return -1;
    }
    /* 提取文件名 */
    filename = strrchr(src_file, '/');
    if (filename == NULL) {
        filename = src_file;
    } else {
        filename++;
    }
    /* 组合目标路径 */
    (void)snprintf(dst_file, sizeof(dst_file), "%s/%s", dst_dir, filename);
    /* 创建目录（如果不存在不会报错） */
    (void)mkdir(dst_dir, 0755);
    /* 打开源文件 */
    src = fopen(src_file, "rb");
    if (src == NULL) {
        return -2;
    }
    /* 打开目标文件 */
    dst = fopen(dst_file, "wb");
    if (dst == NULL) {
        (void)fclose(src);
        return -3;
    }
    /* 单字节拷贝 */
    n = fread(&b, 1U, 1U, src);
    while (n == 1U) {
        if (fwrite(&b, 1U, 1U, dst) != 1U) {
            ret = -4;
            break;
        }
        n = fread(&b, 1U, 1U, src);
    }
    /* 写操作出错时 ret 会是 -4，否则 ret = 0 */
    if (ret != -4) {
        ret = 0;
    }
    (void)fclose(src);
    (void)fclose(dst);
    return ret;
}

/**
 * @brief   从互联网下载 AGNSS 数据并注入 GNSS 引擎
 *
 * @details 调用 HalInjectAgnssData 完成两个动作：
 *          1. 下载星历数据到 /tmp/agps.bin
 *          2. 将星历数据注入 GNSS 引擎
 *
 * @return 无
 **/
static void InjectInternetAgnss(void)
{
    int ret = HalInjectAgnssData();
    if (ret != RET_OK) {
        LOG_Error("Internet AGNSS inject failed, code=%d", ret);
    } else {
        // 将文件/tmp/agps.bin 拷贝到 /sdcard/data/目录
        ret = CopyFileToDir(AGNSS_FILE_PATH,LOCAL_AGNSS_FILE_DIR);
        if (ret != 0) 
        {
            LOG_Error("Copy AGNSS file to local failed, code=%d", ret);
        }
        LOG_Info("Internet AGNSS inject success");
    }
}

/**
 * @brief   无外网时，如果本地 AGNSS 文件存在，则使用本地文件注入
 *
 * @return  无
 **/
static void InjectLocalAgnssIfPossible(void)
{
    if (access(LOCAL_AGNSS_FILE_PATH, F_OK) == 0) {
        LOG_Info("Local AGNSS file exist, inject local data");
        // 将文件/sdcard/data/agps.bin 拷贝到 /tmp 目录
        CopyFileToDir(LOCAL_AGNSS_FILE_PATH,AGNSS_FILE_DIR);
        int ret = HalInjectLocalAgnssData();
        if (ret != RET_OK) {
            LOG_Error("Local AGNSS inject failed, code=%d", ret);
        } else {
            LOG_Info("Local AGNSS inject success");
        }
    } else {
        LOG_Error("AGNSS file expired and no network, AGNSS inject abort");
    }
}

/**
 * @brief   AGNSS 注入线程函数
 *
 * @details 1. 检查公网 APN（privateNet == 0）拨号状态（循环检查）
 *          2. 拨号成功 -> 添加路由 -> 从互联网下载并注入 AGNSS
 *          3. 拨号失败 -> 若本地文件存在则注入本地文件
 *          
 * @return  void*
 **/
static void *InjectAgnssData(void *arg)
{
    (void)arg;
    int dialOk = CheckPublicApnDialSuccess();
    if (g_injectThreadExit) {
        return NULL;
    }
    if (dialOk == 1) {
        // AddAgnssRouteIfNeeded();
        LOG_Info("APN dial success, inject AGNSS from internet");
        InjectInternetAgnss();
    } else {
        LOG_Warn("APN dial failed, fallback to local AGNSS");
        InjectLocalAgnssIfPossible();
    }
    return NULL;
}

int StartInjectAgnssDataThread(void)
{
    int ret;
    g_injectThreadExit = 0;

    ret = pthread_create(&g_injectThread, NULL, InjectAgnssData, NULL);
    if (ret != 0) {
        LOG_Error("StartInjectAgnssDataThread failed: %d\n", ret);
        g_injectThread = 0;
        return -1;
    }
    ret = pthread_detach(g_injectThread);
    if (ret != 0) {
        LOG_Error("pthread_detach failed: %d\n", ret);
        g_injectThread = 0;
        return -2;
    }
    LOG_Info("InjectAgnssData thread started, tid=%lu\n", g_injectThread);
    return 0;
}

int StopInjectAgnssDataThread(void)
{
    g_injectThreadExit = 1;
    return 0; 
}

static int LoadGnssLedPinNumber(uint8_t *pinNum)
{
    int ret = RET_ERR;
    GKeyFile *keyFile = NULL;
    GError *error = NULL;
    keyFile = g_key_file_new();

    if (!keyFile)
    {
        LOG_Error("Error new key file");
        return ret;
    }

    const char *gnssIniPath = GNSS_INI_FILE_PATH;
    if (0 == access(GNSS_INI_FILE_PATH_PRIMARY, F_OK)) {
        gnssIniPath = GNSS_INI_FILE_PATH_PRIMARY;
        LOG_Debug("Using primary config: %s", gnssIniPath);
    }

    if (!g_key_file_load_from_file(keyFile, gnssIniPath, G_KEY_FILE_NONE, &error))
    {
        LOG_Error("Error loading key file: %s", error->message);
        g_error_free(error);
        g_key_file_free(keyFile);
        return ret;
    }
    const char *section = "gnss";
    *pinNum = g_key_file_get_integer(keyFile, section, "pin_number", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'pin_number': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }
    LOG_Debug("Value of integer key 'pin_number' in section 'gnss': %d",  *pinNum);
    g_key_file_free(keyFile);
    ret = RET_OK;
    return ret;
}

static int Ql_GPIO_Init(uint8_t pinNumber, uint8_t pinDirection)
{
    char command[512] = {0};

    sprintf(command, "echo %d > /sys/class/gpio/export", pinNumber);
    int status = system(command);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.",__FUNCTION__,command);
    } else {
        LOG_Error("%s : exec cmd[ %s ] failed.",__FUNCTION__,command);
    }
    
    memset(command,0,sizeof(command));
    
    sprintf(command, "echo %s > /sys/class/gpio/gpio%d/direction",(pinDirection == 0) ? "in" : "out", pinNumber);
    status = system(command);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.",__FUNCTION__,command);
        return RET_OK;
    } else {
        LOG_Error("%s : exec cmd[ %s ] failed.",__FUNCTION__,command);
        return RET_ERR;
    }
    
}

static int Ql_GPIO_SetLevel(uint8_t pinNumber, uint8_t pinLevel)
{
    char command[512] = {0};
    
    sprintf(command, "echo %d > /sys/class/gpio/gpio%d/value",pinLevel, pinNumber);
    int status = system(command);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.",__FUNCTION__,command);
        return RET_OK;
    } else {
        LOG_Debug("%s : exec cmd[ %s ] failed.",__FUNCTION__,command);
        return RET_ERR;
    }
    
}

uint8_t GpsLedIOInit(void)
{
    if (LoadGnssLedPinNumber(&g_pinNumber) != RET_OK)
    {
        LOG_Error("LoadGnssLedPinNumber failed, use default pin number 47");
        g_pinNumber = GPS_LED_DEFAULT_PIN_NUM; // default pin number
    }
    LOG_Info("GPS LED pin number: %d", g_pinNumber);
    if (g_pinNumber != 0)
    {
        if (Ql_GPIO_Init(g_pinNumber, 1) != RET_OK)
        {
            LOG_Error("Ql_GPIO_Init failed for pin %d", g_pinNumber);
            g_pinNumber = 0;
        }
    }
    return g_pinNumber;
}

bool CheckStatusOfGnssLocation(void *user_data)
{
    (void)user_data;
    GnssLocation_t dataToPost;
    uint8_t ledLevel = 0;
    static uint8_t lastLedLevel = 0xFF;

    pthread_mutex_lock(&gpsLock);
    dataToPost = g_gpsData;
    pthread_mutex_unlock(&gpsLock);

    if (dataToPost.status == 1)     // 工作异常
    {
        ledLevel = 0;
    }
    else
    {
        if (dataToPost.gpsState == GPS_STATUS_NOT_POSITIONED)
        {
            ledLevel = 0;
        }
        else
        {
            ledLevel = 1;
        }
    }

    if (lastLedLevel != ledLevel)
    {
        Ql_GPIO_SetLevel(g_pinNumber, ledLevel);
        lastLedLevel = ledLevel;
        LOG_Info("[GPS LED] LED changed to %s(state=%d, status=%d)", ledLevel ? "ON" : "OFF", dataToPost.gpsState, dataToPost.status);
    }

    return true;
}