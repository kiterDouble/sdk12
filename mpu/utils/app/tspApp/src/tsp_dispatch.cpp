#include "tsp_common.h"
#include "tsp_connect.h"
#include "tsp_dispatch.h"
#include "tsp_crc32.h"
#include "tsp_parser_diagnosis.h"

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
    {0x0, E50_DOWNLINK_TOPIC, ParserDiagnosisData},
    {0x0, E50_DOWNLINK_TOPIC, ParserDiagnosisData},
};

static pthread_mutex_t g_dispatchLock = PTHREAD_MUTEX_INITIALIZER;

static gint GetDispatchData(OUT PDispatch_Cache_Data_t *pData)
{
    if (!pData)
    {
        //LOG_Error("param invalid!");
        return RET_ERR;
    }
    if (RET_ERR == GetTspPackageData((gpointer *) pData))
    {
        //LOG_Error("GetPackageData invalid!");
        return RET_ERR;
    }
    
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
    // 计算循环冗余校验码，用于在解析时启动对应的解析程序
    data->u32crc = StrCalCrc(pTopic);
    data->plen = dataLen;
    memcpy(data->payload, pData, dataLen);
    CachePackageData((gpointer) data);
    //g_free(data);
    return RET_OK;
}

gint DispatchCalTopicCrc(void)
{
    uint8_t c8Iccid[32] = {0x0};
    gchar topic[128] = {0x0};
    gchar deviceId[128] = {0};
    guint len = 0;
    gint s32Ret;
    gint i = 0;

    len = sizeof(g_dispatcher) / sizeof(DispatcherProcess_t);
    pthread_mutex_lock(&g_dispatchLock);
    // 获取设备ID，用于订阅远控下行主题？？？
    GetDeviceID(deviceId);
    sprintf(g_dispatcher[0].topic,"%s%s/get",E50_UPLINK_TOPIC,deviceId);
    g_dispatcher[0].u32Crc = StrCalCrc(g_dispatcher[0].topic);
    // 获取ICCID，用于订阅远程升级下行主题？？？
    GetIccid(c8Iccid, sizeof(c8Iccid) - 1);
    sprintf(g_dispatcher[1].topic,"%s%s/cmd/get",E50_FOTA_TOPIC,c8Iccid);
    g_dispatcher[1].u32Crc = StrCalCrc(g_dispatcher[1].topic);
    pthread_mutex_unlock(&g_dispatchLock);
    return RET_OK;
}

gpointer DispatchTaskEntry(IN gpointer pArg)
{
    GMainLoop *loop = (GMainLoop *) pArg;
    guint len = sizeof(g_dispatcher) / sizeof(DispatcherProcess_t);
    guint i = 0;
    gint tmpCrc = 0;
    DispatchCalTopicCrc();
    while (1)
    {

        PDispatch_Cache_Data_t pData = NULL;
        //从消息队列中获取消息
        if ((RET_OK == GetDispatchData(&pData)) && pData)
        {
            if(NULL == pData)
            {
                continue;
            }            
            for (i = 0; i < len; i++)
            {
                if(NULL == g_dispatcher[i].pCallback)
                {
                    continue;
                }

                pthread_mutex_lock(&g_dispatchLock);
                LOG_Debug("g_dispatcher : %s: %X", g_dispatcher[i].topic, g_dispatcher[i].u32Crc);
                LOG_Debug("pData->u32crc: %X", pData->u32crc);
                tmpCrc = g_dispatcher[i].u32Crc;
                pthread_mutex_unlock(&g_dispatchLock);
                if ((pData->u32crc == tmpCrc))
                {
                    LOG_Debug("%s Callback executed", g_dispatcher[i].topic);
                    g_dispatcher[i].pCallback(pData->payload, pData->plen);
                    break;
                }
            }
            g_free(pData);
        }
       // else
       // {
       //     LOG_Debug("dispatch recv timeout!");
       // }
    }
}
