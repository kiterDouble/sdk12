#include "mqtt_connect.h"
#include "mqtt_dispatch.h"
#include "mqtt_crc32.h"

typedef gint (*DispatchCb)(gpointer pData, guint u32Len);

typedef struct 
{
    guint u32Crc;
    gchar topic[64];
    DispatchCb pCallback;
}DispatcherProcess_t;

typedef struct 
{
    guint u32crc;
    guint plen;
    guchar payload[0];
}Dispatch_Cache_Data_t, *PDispatch_Cache_Data_t;

static DispatcherProcess_t g_dispatcher[] =
{
    // {0x0, E50_DOWNLINK_TOPIC, ParserDiagnosisData},
    // {0x0, E50_DOWNLINK_TOPIC, ParserDiagnosisData},
};

static pthread_mutex_t g_dispatchLock = PTHREAD_MUTEX_INITIALIZER;

static gint GetDispatchData(OUT PDispatch_Cache_Data_t *pData)
{
    if (!pData)
    {
        LOG_Error("param invalid!");
        return RET_ERR;
    }
    // if (RET_ERR == GetTspPackageData((gpointer *) pData))
    // {
    //     LOG_Error("GetPackageData invalid!");
    //     return RET_ERR;
    // }
    
    return RET_OK;
}

gint CacheDispatchData(IN gchar *pTopic, IN guchar *pData, IN guint dataLen)
{
    if (!pTopic || !pData || (0 == dataLen))
    {
        LOG_Error("param invalid!");
        return RET_ERR;
    }
    
    PDispatch_Cache_Data_t data = (PDispatch_Cache_Data_t)g_try_malloc0(sizeof(Dispatch_Cache_Data_t) + dataLen);
    if (data == NULL)
    {
        LOG_Error("New Cache Data Failed");
        return RET_ERR;
    }
    memset(data,0,sizeof(Dispatch_Cache_Data_t) + dataLen); 
    // data->u32crc = StrCalCrc(pTopic);
    data->plen = dataLen;
    memcpy(data->payload, pData, dataLen);
    // CachePackageData((gpointer) data);
    //g_free(data);
    return RET_OK;
}

gint DispatchCalTopicCrc(void)
{
    gchar c8Iccid[21] = {0x0};
    gchar topic[128] = {0x0,};
    gchar deviceId[128] = {0};
    guint len = 0;
    gint s32Ret;
    gint i = 0;

    // len = sizeof(g_dispatcher) / sizeof(DispatcherProcess_t);
    // pthread_mutex_lock(&g_dispatchLock);
    // GetDeviceID(deviceId);
    // sprintf(g_dispatcher[0].topic,"%s%s/get",E50_UPLINK_TOPIC,deviceId);
    // g_dispatcher[0].u32Crc = StrCalCrc(g_dispatcher[0].topic);

    // GetParamsValue(enumIiccid,c8Iccid);
    // sprintf(g_dispatcher[1].topic,"%s%s/cmd/get",E50_FOTA_TOPIC,c8Iccid);
    // g_dispatcher[1].u32Crc = StrCalCrc(g_dispatcher[1].topic);
    // pthread_mutex_unlock(&g_dispatchLock);
    return RET_OK;
}

gpointer DispatchTaskEntry(IN gpointer pArg)
{
    GMainLoop *loop = (GMainLoop *) pArg;
    guint len = sizeof(g_dispatcher) / sizeof(DispatcherProcess_t);
    guint i = 0;
    DispatchCalTopicCrc();
    while (1)
    {

        PDispatch_Cache_Data_t pData = NULL;

        if ((RET_OK == GetDispatchData(&pData)) && pData)
        {
            if(NULL == pData)
            {
                continue;
            }
            pthread_mutex_lock(&g_dispatchLock);
            for (i = 0; i < len; i++)
            {
                if(NULL == g_dispatcher[i].pCallback)
                {
                    continue;
                }

                LOG_Debug("g_dispatcher : %s: %X", g_dispatcher[i].topic, g_dispatcher[i].u32Crc);
                LOG_Debug("pData->u32crc: %X", pData->u32crc);
                if ((pData->u32crc == g_dispatcher[i].u32Crc))
                {
                    LOG_Debug("%s Callback executed", g_dispatcher[i].topic);
                    g_dispatcher[i].pCallback(pData->payload, pData->plen);
                    break;
                }
            }
            pthread_mutex_unlock(&g_dispatchLock);
            g_free(pData);
        }
       // else
       // {
       //     LOG_Debug("dispatch recv timeout!");
       // }
    }
}
