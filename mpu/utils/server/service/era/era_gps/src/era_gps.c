#include <string.h>
#include <stdlib.h>
#include "sr_ipc.h"
#include "era_gps.h"
#include "gnss_ipc_msg.h"
#include "sr_log.h"
#include "list.h"

#if 0
static GnssLocation_t g_GnssLocation;
static StrGPSInfo g_GPSInfo = {0};

/**
* @brief 获取GPS信息
*
* @return GPSInfo_t 返回GPS信息结构体
*/
StrGPSInfo *GetGpsInfo(void)
{
    g_GPSInfo.status = g_GnssLocation.gpsState;
    g_GPSInfo.direction = g_GnssLocation.direction;
//    g_GPSInfo.timestamp = g_GnssLocation.timestamp;
    g_GPSInfo.timestamp = time(NULL);
    g_GPSInfo.latitude_value_N2 = g_GPSInfo.latitude_value_N1;
    g_GPSInfo.longtitude_value_N2 = g_GPSInfo.longtitude_value_N1;
    g_GPSInfo.latitude_value_N1 = g_GPSInfo.latitude_value;
    g_GPSInfo.longtitude_value_N1 = g_GPSInfo.longtitude_value;
    g_GPSInfo.latitude_value = g_GnssLocation.flatitude;
    g_GPSInfo.longtitude_value = g_GnssLocation.flongitude;
    g_GPSInfo.latitude_degree = g_GnssLocation.latitudeValue;
    g_GPSInfo.longtitude_degree = g_GnssLocation.longtitudeValue;

    return &g_GPSInfo;
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
    memset(&g_GnssLocation, 0x00, sizeof(GnssLocation_t));
    if(sizeof(GnssLocation_t) == len)
    {
        memmove(&g_GnssLocation, data, len);
    }
    else
    {
        LOG_Debug("GnssMsgSubCallBack len error!");
    }
    LOG_Debug("Gps : %s - latitude_value:%f , longtitude_value:%f ,latitude_degree:%d ,longtitude_degree:%d ",
    ((g_GnssLocation.gpsState & 0x1F) == 0) ? "no location" : "location",
    g_GnssLocation.flatitude,g_GnssLocation.flongitude,g_GnssLocation.latitudeValue,g_GnssLocation.longtitudeValue);
}

void EcallGnssInit(void)
{
    SubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO, GnssMsgSubCallBack, NULL);
}
#else

struct EcallGnssNode {
    GnssLocation_t data;
    struct list_head node;
};

#define ECALL_GNSS_MAX_CACHE    3
#define ECALL_GNSS_LOC_UNKNOWN  0x7FFFFFFF

static GnssLocation_t g_GnssActiveLocation;     /* 最新有效定位值 */
static pthread_mutex_t g_GnssCacheMutex= PTHREAD_MUTEX_INITIALIZER;
static struct list_head g_GnssHead = LIST_HEAD_INIT(g_GnssHead);
static StrGPSInfo g_GPSInfo = {0};

static void EcallGnssCacheInit();
static void EcallGnssLocationInit(GnssLocation_t *data);
static void GnssLocationPrint(GnssLocation_t *data);
static int32_t EcallGnssLocationEncode(double value);
static int32_t EcallGnnsLocationDelta(double prev, double cur);
static int32_t EcallGnnsActive(GnssLocation_t *gnss);

/**
* @brief 获取GPS信息
*
* @return GPSInfo_t 返回GPS信息结构体
*/
StrGPSInfo *GetGpsInfo(void)
{
    StrGPSInfo *info = &g_GPSInfo;
    struct list_head *head = &g_GnssHead;
    struct EcallGnssNode *node;
    GnssLocation_t dataTable[ECALL_GNSS_MAX_CACHE] = {0};
    GnssLocation_t *ndata;
    int i = 0;

    /* 获取所有当前缓存GNSS信息 */
    pthread_mutex_lock(&g_GnssCacheMutex);
    if (list_empty(head))
    {
        LOG_Warn("GNSS cache list is empty");
        pthread_mutex_unlock(&g_GnssCacheMutex);
        memset(info, 0, sizeof(StrGPSInfo));
        info->trusted = 0;
        info->direction = 0xFF;
        info->latitude = ECALL_GNSS_LOC_UNKNOWN;
        info->longitude = ECALL_GNSS_LOC_UNKNOWN;
        info->timestamp = time(NULL);
        return info;
    }
    list_for_each_entry(node, head, node)
    {
        if (i >= ECALL_GNSS_MAX_CACHE)
        {
            LOG_Warn("GNSS cache exceeds max size %d", ECALL_GNSS_MAX_CACHE);
            break;
        }
        memcpy(&dataTable[i++], &node->data, sizeof(GnssLocation_t));
    }
    pthread_mutex_unlock(&g_GnssCacheMutex);
    
    memset(info, 0, sizeof(StrGPSInfo));
    if(EcallGnnsActive(&dataTable[0]))      /* 当前有效 */
    {
        ndata = &dataTable[0];
        info->trusted = 1;
        info->direction = (uint8_t )(ndata->direction/2);           /* 当前方向 */
        info->latitude = EcallGnssLocationEncode(ndata->flatitude); /* 位置信息 */
        info->longitude = EcallGnssLocationEncode(ndata->flongitude);

        info->timestamp = ndata->timestamp;
    }
    else if(EcallGnnsActive(&g_GnssActiveLocation))    /* 之前存在有效 */
    {
        ndata = &g_GnssActiveLocation;
        info->trusted = 0;
        info->direction = (uint8_t)(ndata->direction/2);       /* 当前方向 */
        info->latitude = EcallGnssLocationEncode(ndata->flatitude);
        info->longitude = EcallGnssLocationEncode(ndata->flongitude);
        info->timestamp = ndata->timestamp;
    }
    else                                /* 当前无效 && 之前不存在无效 */
    {
        info->trusted = 0;
        info->direction = 0xFF;
        info->latitude = ECALL_GNSS_LOC_UNKNOWN;
        info->longitude = ECALL_GNSS_LOC_UNKNOWN;
        info->timestamp = time(NULL);   /* 获取系统时间 */
    }

    /* deltaN1 与 deltaN2值计算 */
    if(EcallGnnsActive(&dataTable[0]) && EcallGnnsActive(&dataTable[1]))    /* 上一次定位有效 */
    {
        info->latitudeDeltaN1 = EcallGnnsLocationDelta(dataTable[1].flatitude, ndata->flatitude);
        info->longitudeDeltaN1 = EcallGnnsLocationDelta(dataTable[1].flongitude, ndata->flongitude);
    }
    else
    {
        info->latitudeDeltaN1 = 0;
        info->longitudeDeltaN1 = 0;
    }
        
    if(EcallGnnsActive(&dataTable[1])&& EcallGnnsActive(&dataTable[2]))     /* 上次 并且 上上次定位有效 */
    {
        info->latitudeDeltaN2 = EcallGnnsLocationDelta(dataTable[2].flatitude, dataTable[1].flatitude);
        info->longitudeDeltaN2 = EcallGnnsLocationDelta(dataTable[2].flongitude, dataTable[1].flongitude);
    }
    else
    {
        info->latitudeDeltaN2 = 0;
        info->longitudeDeltaN2 = 0;
    }

    return info;
}

void GnssMsgSubCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
    struct list_head *head = &g_GnssHead;
    struct EcallGnssNode *node;
    GnssLocation_t *ndata;

    if(sizeof(GnssLocation_t) == len)
    {
        pthread_mutex_lock(&g_GnssCacheMutex);
        node = list_last_entry(head, struct EcallGnssNode, node);
        list_del(&node->node);      /* remove node from list's tail */

        ndata = &node->data;
        memset(ndata, 0x00, sizeof(GnssLocation_t));
        memcpy(ndata, data, len);
        if(EcallGnnsActive(ndata))     /* 记录最新有效定位 */
            memcpy(&g_GnssActiveLocation, ndata, sizeof(GnssLocation_t));
        list_add(&node->node, head);/* insert node into list's head */
        pthread_mutex_unlock(&g_GnssCacheMutex);

//        GnssLocationPrint(ndata);   /* 打印定位信息 */
    }
    else
    {
        LOG_Debug("GnssMsgSubCallBack len error!");
    }
}

void EcallGnssInit(void)
{
    EcallGnssLocationInit(&g_GnssActiveLocation);
    EcallGnssCacheInit();

    SubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO, GnssMsgSubCallBack, NULL);
}

void EcallGnssDump(void)
{
    struct list_head *head = &g_GnssHead;
    struct EcallGnssNode *node;
    GnssLocation_t *ndata;
    int i = 0;

    LOG_Debug("%-2s %-6s %-8s %-8s %-20s %-20s %-6s %s\n", "no", "gs(h)", "uo(h)", "ua(h)", "o", "a", "d(h)", "t");
    list_for_each_entry(node, head, node)
    {
        ndata = &node->data;
        LOG_Debug("%02d %-6X %-8X %-8X %-20lf %-20lf %-6X %u\n", \
                i++, ndata->status, ndata->longtitudeValue, ndata->latitudeValue, \
                ndata->flongitude, ndata->flatitude, ndata->direction, ndata->timestamp);
    }
}

void EcallGPSInfoDump(StrGPSInfo *data)
{
    LOG_Debug("s:%u, d:%X(h), a:%d, o:%d, aN1:%d, oN1:%d, aN2:%d, oN2:%d, t:%u\n",   \
            data->trusted, data->direction, data->latitude, data->longitude,\
            data->latitudeDeltaN1, data->longitudeDeltaN1, data->latitudeDeltaN2, \
            data->longitudeDeltaN2, data->timestamp);
}


int32_t EcallGnssLocationEncode(double value)
{
    int hour, min, sec;

    hour = (int)value;
    value = (value - hour)*60;
    min  = (int)value;
    value = (value - min)*60;
    sec = (int)value;

    return (hour*60*60 + min*60 + sec)*1000;
}

double EcallGnssLocation2Second(double value)
{
    int hour, min;

    hour = (int)value;
    value = (value - hour)*60;
    min  = (int)value;
    value = (value - min)*60;

    return hour*60*60 + min*60 + value;     /* 经纬度转换为秒*/
}

int32_t EcallGnnsLocationDelta(double prev, double cur)
{
    int32_t delta = 0; 

    delta = ((int32_t )((EcallGnssLocation2Second(cur) - EcallGnssLocation2Second(prev))/0.1))*3;
    
    return ((delta>=-512)&&(delta<=511))? delta: 0;
}

int32_t EcallGnnsActive(GnssLocation_t *gnss)
{
    return 0 != (gnss->gpsState & 0x1F);
}

static void GnssLocationPrint(GnssLocation_t *data)
{
    if(data)
    {
        LOG_Debug("Gps Ant: %u, Gps : %s - latitude_value:%f , longtitude_value:%f ,latitude_degree:%d ,longtitude_degree:%d ",
        data->antStatus, (EcallGnnsActive(data) ? "ok" : "no"),
        data->flatitude, data->flongitude, data->latitudeValue, data->longtitudeValue);
    }
}

static void EcallGnssLocationInit(GnssLocation_t *data)
{
    if(data)
    {
        data->status = 1;
        data->antStatus = 99;
        data->gpsState = 0;                 /* 未定位 */
        data->longtitudeValue = ECALL_GNSS_LOC_UNKNOWN;
        data->latitudeValue = ECALL_GNSS_LOC_UNKNOWN;
        data->direction = 0xFF;
        data->timestamp = 0;
    }
}
static void EcallGnssCacheInit()
{
    int i = 0;
    struct list_head *head = &g_GnssHead;
    struct EcallGnssNode *node = NULL;

    for(i=0; i < ECALL_GNSS_MAX_CACHE; i++)
    {
        node = (struct EcallGnssNode *)calloc(1, sizeof(struct EcallGnssNode));
        if(node)
        {
            INIT_LIST_HEAD(&node->node);
            EcallGnssLocationInit(&node->data);     /* 初始化gnss node */
            list_add(&node->node, head);            /* 添加到链表 */
        }
    }
}
#endif
