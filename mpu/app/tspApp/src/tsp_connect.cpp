#include <iostream>
#include <fstream>
#include "tsp_parser_diagnosis.h"
#include "tsp_common.h"
#include "json/inc/json.h"
#include "sr_log.h"
#include "err_code.h"
#include "tbox_uds_service.h"
#include "tsp_connect.h"
#include "tsp_log_report.h"
#include "tsp_gettoken.h"
#include "tsp_dbus_server.h"	
#include <unistd.h>
#include <fcntl.h>
#include "tsp_cache_queue.h"
#include "tsp_dispatch.h"
#include "net_service_api.h"


using namespace std;

#define  NET_OFF_TO_ON_FILE_NAME    "/oemdata/tmp/tsp_heartbeat"

#define MQTT_CONNECT_ZELOS_IP_ADDR  "mqtt.zelostech.com.cn"
#define MQTT_PROD_ZELOS_TOPIC       "TS00010/VehicleState/set" 
#define MQTT_CONNECT_ZELOS_PORT     (1993)

#define oelinux123
#define SUPPORT_MQTT
typedef enum OtaConnectStat
{
    E_OTA_CONNECT_REQUEST_TOKEN,
    E_OTA_CONNECT_SERVER,
    E_OTA_CONNECT_LOGIN,
    E_OTA_CONNECT_RECV,
    E_OTA_CONNECT_LOGOUT,
    E_OTA_CONNECT_NONE,
}OtaConnectStat;

typedef enum OtaConnect90Stat
{
    E_OTA_CONNECT_90_SERVER,
    E_OTA_CONNECT_90_LOGIN,
    E_OTA_CONNECT_90_RECV,
    E_OTA_CONNECT_90_LOGOUT,
    E_OTA_CONNECT_90_NONE,
} OtaConnect90Stat;

    
typedef struct{  
    GAsyncQueue *g_v3Tsp;
    #ifdef SUPPORT_MQTT
    MQTTClient client;
    #else
    gint socketFd;
    #endif
    guchar u8CurrStat;
    pthread_mutex_t lock;
}Ota_Connect_Param_t;

static Ota_Connect_Param_t g_connectParam;

typedef enum
{
    E_TSP_LOGOUT_NOEN,
    E_TSP_LOGOUT_SLEEP,
    E_TSP_LOGOUT_RECV,
    E_TSP_LOGOUT_LOGIN,
}TspLogoutTpye;



    
static TspLogoutTpye g_tspLogoutType = E_TSP_LOGOUT_NOEN;
static Json::Reader g_reader;
static Json::Value g_root;

static gint g_sendHeartBeat = 0;
pthread_mutex_t g_sendHeartBeatLock;

static char g_ipAddr[32] = {0};
#define TBOXTSPHOST			"/oemdata/tmp/tsp_ip.txt"

//周期性发布车辆硬线状态结构体，SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW订阅信息
typedef struct
{
	uint8_t kl15;
	int16_t kl30;
	//uint16_t fastCharging;
    uint8_t canNet;
} MCUSYNCKINE_t;
static MCUSYNCKINE_t g_mcuSyncKine = {
    .kl15 = 0,      
    .kl30 = -1,     
    .canNet = 0    
};

//当前日志等级
//日志级别从低到高依次是"DEBUG", "INFO", "NOTICE", "WARN", "ERROR", "FATAL"
static gchar g_logLevel[8] = "DEBUG";
// 检查并修改日志等级
static void checkAndSetLogLevel(const char *level)
{
    if (level == NULL) {
        return;
    }

    // 确保新日志级别不会超出缓冲区大小
    size_t len = strlen(level);
    if (len >= sizeof(g_logLevel)) {
        LOG_Fatal("New log level too long: %s", level);
        return;
    }

    if (0 != strcmp(level, g_logLevel))
    {
        LOG_Fatal("set log level: %s", level);
        LOG_SET_LEVEL(level);
        strncpy(g_logLevel, level, sizeof(g_logLevel) - 1);
        g_logLevel[sizeof(g_logLevel) - 1] = '\0';  // 确保以null结尾
    }
}

/*
* @brief get wakeup information when in sleep mode 
*
* @param p
*
* @return 
*/
gpointer GetTspWakeupInfo(gpointer p)
{
    MQTTClient_message* m = NULL;
    gchar* topicName = NULL;
    gint topicLen;
    while(1)
    {
        if( (GetPmState() == SGMW_PM_STATE_SLEEP) && ( NULL != g_connectParam.client ) )
        {
            if (MQTTCLIENT_SUCCESS != MQTTClient_receive(g_connectParam.client, &topicName, &topicLen, &m, 1000))
            {
                LOG_Error("get tsp info error in sleepmode ");
            }
            
            //Wakeup(PM_WAKEUP_SOURCE_TCP );
            PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
            PmWakeupSetDevices("mcu", POWER_MODE);
            PmWakeupSetDevices("GNSS", POWER_MODE);
            PmWakeupSetDevices("HSE", POWER_MODE);
            PmWakeupSetDevices("codec", POWER_MODE);
            PmWakeupSetDevices("PHY", POWER_MODE);

            if (topicName)
            {
                LOG_Debug("topic:%s,payload:%s,payloadlen :%d", topicName, m->payload,m->payloadlen);
                CacheDispatchData(topicName, (guchar *)m->payload, m->payloadlen);
                MQTTClient_free(topicName);
                MQTTClient_freeMessage(&m);
            }
        }
        else
        {
            sleep(2);
        }
    }
 
    return NULL;
}

gint GetDeviceID(gchar *deviceID)
{
    if(NULL == deviceID)
    {
        LOG_Error("deviceID is NULL ");
    }
    memcpy(deviceID,g_root["content"]["deviceId"].asString().c_str(),g_root["content"]["deviceId"].asString().length());
    return RET_OK;
}


static gint ParserTspInfo(gchar *tspInfo)
{
    if(NULL == tspInfo)
    {
        LOG_Error("tspInfo is NULL");
        return RET_ERR;
    }

    if(g_reader.parse(tspInfo,g_root))
    {
        LOG_Debug("g_root[\"msgCode\"]:%s",g_root["msgCode"].asString().c_str());
        LOG_Debug("g_root[\"msg\"]:%s",g_root["msg"].asString().c_str());
        LOG_Debug("g_root[\"content\"][\"deviceId\"]:%s",g_root["content"]["deviceId"].asString().c_str());
        LOG_Debug("g_root[\"content\"][\"deviceSecret\"]:%s",g_root["content"]["deviceSecret"].asString().c_str());
        LOG_Debug("g_root[\"content\"][\"clientId\"]:%s",g_root["content"]["clientId"].asString().c_str());
        LOG_Debug("g_root[\"content\"][\"host\"]:%s",g_root["content"]["host"].asString().c_str());
        LOG_Debug("g_root[\"content\"][\"port\"]:%s",g_root["content"]["port"].asString().c_str());
        LOG_Debug("g_root[\"status\"]:%d",g_root["status"].asInt());
        LOG_Debug("g_root[\"total\"]:%d",g_root["total"].asInt());
        if(strcmp(g_root["msgCode"].asString().c_str(), "10000") && strcmp(g_root["msg"].asString().c_str(), "OK"))
        {
            LOG_Debug("g_root[\"msgCode\"] or g_root[\"msg\"] is error");
            return RET_ERR;
        }

    }
    else
    {
        return RET_ERR;
    }

    return RET_OK;
}

static gint GetMsgId(void)
{
    static gint msgid = 0x0;

    return ++msgid;
}

gint GetTspPackageData(gpointer *pData)
{
    return GetDataWithTimeOutFromQueueWithoutFree(g_connectParam.g_v3Tsp, pData, 300);
}

gint CachePackageData(gpointer pData)
{
    CacheDataToQueueWithoutMalloc(g_connectParam.g_v3Tsp, pData);
    return RET_OK;
}


gboolean IsConnected(void)
{
    gboolean bIsconnect = FALSE;

    pthread_mutex_lock(&g_connectParam.lock);
    bIsconnect = (NULL != g_connectParam.client);
    pthread_mutex_unlock(&g_connectParam.lock);
    return bIsconnect;
}

gint SendPackageData(const gchar *topic, gint qos, gpointer u8Data, guint u32length)
{
    gint s32Ret = RET_ERR;

    pthread_mutex_lock(&g_connectParam.lock);
    if (NULL != g_connectParam.client)
    { 
        MQTTClient_deliveryToken dt;
		MQTTClient_message pubmsg = MQTTClient_message_initializer;
		int rc = 0;
		char payload[128];
		
		
		pubmsg.payload = u8Data;
		pubmsg.payloadlen = u32length;
		pubmsg.qos = qos;
		pubmsg.retained = 0;
        pubmsg.msgid = GetMsgId();
        LOG_Debug("send topic[%d]:%s",qos,topic);
		//printf("msgid:%x\n", pubmsg.msgid);
		rc = MQTTClient_publishMessage(g_connectParam.client, topic, &pubmsg, &dt);
        if (rc != MQTTCLIENT_SUCCESS)
        {
            MQTTClient_destroy(&g_connectParam.client);
            //checkAndSetLogLevel("DEBUG");
            g_connectParam.client = NULL;
            pthread_mutex_unlock(&g_connectParam.lock);
            LOG_Error("send topic[%d]:%s error",qos,topic);
            return s32Ret;
        }
		rc = MQTTClient_waitForCompletion(g_connectParam.client, dt, 2000);
		if (rc != MQTTCLIENT_SUCCESS)
        {
            MQTTClient_destroy(&g_connectParam.client);
            //checkAndSetLogLevel("DEBUG");
            g_connectParam.client = NULL;
            pthread_mutex_unlock(&g_connectParam.lock);
            LOG_Error("send topic[%d]:%s error",qos,topic);
            return s32Ret;
        }
        s32Ret = RET_OK;
        //checkAndSetLogLevel("ERROR");
        LOG_Debug("send topic[%d]:%s",qos,topic);
    }
    else
    {
        //checkAndSetLogLevel("DEBUG");
    }
    pthread_mutex_unlock(&g_connectParam.lock);
    LOG_Debug("send topic[%d]:%s",qos,topic);
    return s32Ret;
}


static gint OtaConnectRequestToken(void)
{
    static time_t oldTime = 0;
    gint  retValue = RET_ERR;
	static gint needTokenFlag = 1;   
    // 判断是否到达5s
    if(time(NULL) < oldTime)
    {
        LOG_Debug("wait [%d] S ",oldTime - time(NULL));
        sleep(1);
        return RET_ERR;
    }

    if(0 == needTokenFlag)
    {
        //向平台通过https协议请求token
        retValue = TaskRequestToken();
        needTokenFlag = 1;
    }
    else
    {
        //检查token文件是否存在
        retValue = GetFileFromTokenCheck();
        needTokenFlag = 0;
    }

	if(retValue != RET_OK)  //表示有这个文件
    {
        oldTime = time(NULL) + 5;   //如果不成功，必须5s后再重新发送
        return RET_ERR;
    }
    oldTime = time(NULL) + 5;   //如果成功，5s后才允许重发
    return RET_OK;
}

int SslErrorFunc(const char *str, size_t len, void *u)
{
    LOG_Debug("error:%s",str);
    LOG_Debug("len:%d",len);
    return 0;
}

static int NetOffToOnAndSendHeartBeat()
{
    int retValue = -1;

    if(0 != access(NET_OFF_TO_ON_FILE_NAME, F_OK))
    {
        return RET_OK;
    }

    DiagnosisHeartbeat();

    system("rm /oemdata/tmp/tsp_heartbeat && sync ");
    LOG_Debug("tsp is offline and send heartbeata error");
    
    return RET_OK;
}

static int getlocalhost(char *localHost)
{
    if (localHost == NULL)
    {
        LOG_Debug("localHost is NULL");
        return RET_ERR;
    }

    char buf[256] = {0};
    FILE *fp = fopen(TBOXTSPHOST, "r");
    if (fp == NULL)
    {
        LOG_Debug("Cannot open file: %s", TBOXTSPHOST);
        return RET_ERR;
    }

    if (fgets(buf, sizeof(buf), fp) != NULL)
    {
        LOG_Debug("Read tsp host from File[%s] - [%s]", TBOXTSPHOST, buf);
        strncpy(localHost, buf, strlen(buf));
    }
    else
    {
        LOG_Debug("Read tsp host error: %s", strerror(errno));
        fclose(fp);
        return RET_ERR;
    }

    fclose(fp);
    return RET_OK;
}

static int setlocalhost(const char *localHost)
{
    if (localHost == NULL)
    {
        LOG_Debug("localHost is NULL");
        return RET_ERR;
    }

    FILE *fp = fopen(TBOXTSPHOST, "w");
    if (fp == NULL)
    {
        LOG_Debug("Cannot open file: %s", TBOXTSPHOST);
        return RET_ERR;
    }

    if (fputs(localHost, fp) != EOF)
    {
        LOG_Debug("Write tsp host to File[%s] - [%s]", TBOXTSPHOST, localHost);
    }
    else
    {
        LOG_Debug("Write tsp host error: %s", strerror(errno));
        fclose(fp);
        return RET_ERR;
    }

    fclose(fp);
    return RET_OK;
}

#ifdef SUPPORT_MQTT
static gint MQTTConnectToServer(void)
{
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    MQTTClient_SSLOptions mqttSsl = MQTTClient_SSLOptions_initializer;
    gchar username[256] = {0};
    gchar password[256] = {0};
    gchar url[256] = {0};
    gchar topicStr[128] = {0};
    uint8_t c8Iccid[32] = {0};
    char ipAddr[32] = {0};
    char persistenceStore[1024] = {0};
    gchar tspInfo[MAX_BUFFER_SIZE] = {0};
	gint rc = 0;
    gint retValue = RET_ERR;
    struct hostent *host = NULL;
    struct in_addr ipaddrTmp;

    pthread_mutex_lock(&g_connectParam.lock);
    // 从tocken文件中获取连接TSP平台的信息
    GetTokenInfo(tspInfo);
    LOG_Debug("tspInfo :%s", tspInfo);
    if(RET_OK != ParserTspInfo(tspInfo))
    {
        LOG_Error("ParserTspInfo Error");
        pthread_mutex_unlock(&g_connectParam.lock);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("ParserTspInfo succes");
    }
    
    if (NULL != (host = gethostbyname(g_root["content"]["host"].asString().c_str())))
    {
        inet_ntop(AF_INET, host->h_addr_list[0], ipAddr, sizeof(ipAddr));

        if (0 != strcmp(g_ipAddr, ipAddr))
        {
            strncpy(g_ipAddr, ipAddr, sizeof(g_ipAddr) - 1);
            LOG_Debug("setlocalhost :%d", setlocalhost(g_ipAddr));
        }
    }
    else
    {
        LOG_Debug("Gethostbyname Error:%s", strerror(errno));
        // 获取本地缓存的TSP地址
        if (getlocalhost(g_ipAddr) != RET_OK)
        {
            LOG_Debug("Cannot get localhost");
            pthread_mutex_unlock(&g_connectParam.lock);
            return RET_ERR;  // 获取本地缓存失败，退出程序
        }
        strncpy(ipAddr, g_ipAddr, sizeof(ipAddr) - 1);
    }

    LOG_Debug("ipAddr :%s", ipAddr);

    //snprintf(url, sizeof(url), "tcp://%s:%d", c8server, u32Port);111.198.98.57
    //snprintf(url, sizeof(url), "tcp://%s:%d", "192.168.150.196", 7884);
    // retValue = GetHostIpAddr(g_root["content"]["host"].asString().c_str(), 
    //                          g_root["content"]["host"].asString().length(),
    //                          ipAddr, 32);
    // if(RET_OK != retValue)
    // {
    //     LOG_Error("GetHostIpAddr Error\n");
    //     pthread_mutex_unlock(&g_connectParam.lock);
    //     return RET_ERR;
    // }
    //inet_ntop(AF_INET, host->h_addr, ipAddr, 32);
    inet_aton(ipAddr,&ipaddrTmp);
    if (1 != checkIPonInterface("ccinet1", ipaddrTmp))
    {
        int retValue = -1;
        char cmdBuff[1024] = {0};
        memset(cmdBuff, 0, 1024);
        snprintf(cmdBuff, 1024,"ip route add %s dev %s", ipAddr, "ccinet1");
        retValue = GetSystemRetValue(cmdBuff);
        LOG_Debug("cmdBuff exe ret [%d]:%s", retValue, cmdBuff);
        if (0 != retValue)
        {
            pthread_mutex_unlock(&g_connectParam.lock);
            return RET_ERR;
        }
    }
    else
    {
        LOG_Debug("IP %s has bound on ccinet1", ipAddr);
    }

    snprintf(url, sizeof(url), "ssl://%s:%s", ipAddr, g_root["content"]["port"].asString().c_str());
    memcpy(username,g_root["content"]["deviceId"].asString().c_str(),g_root["content"]["deviceId"].asString().length());
    memcpy(password,g_root["content"]["deviceSecret"].asString().c_str(),g_root["content"]["deviceSecret"].asString().length());
    LOG_Debug("url:%s",url);
    LOG_Debug("username:%s",username);
    LOG_Debug("password:%s",password);
    
    if(strlen(ipAddr) <= 0 || strlen(username) <= 0 || strlen(password) <= 0)
    {
        LOG_Error("MQTT param is NULL");
        pthread_mutex_unlock(&g_connectParam.lock);
        return RET_ERR;
    }
    
    if(NULL != g_connectParam.client)
    {
        MQTTClient_disconnect(g_connectParam.client, 0);
        MQTTClient_destroy(&g_connectParam.client);
        //checkAndSetLogLevel("DEBUG");
        g_connectParam.client = NULL;
    }

	rc = MQTTClient_create(&g_connectParam.client, url, g_root["content"]["clientId"].asString().c_str(), MQTTCLIENT_PERSISTENCE_NONE, persistenceStore);
    if(rc != MQTTCLIENT_SUCCESS || NULL == g_connectParam.client)
    {
        LOG_Error("MQTT create fail :%d!",rc);
        pthread_mutex_unlock(&g_connectParam.lock);
		return RET_ERR;
    }
    else
    {
        LOG_Error("MQTT create success");
    }
    

	opts.keepAliveInterval = 600;
    opts.cleansession = 1;
	opts.username = username;
	opts.password = password;
    //mqttSsl.keyStore = "/usrdata/status/ca/1671639__sgmwcloud.com.cn.pem";
    mqttSsl.trustStore = "/oemapp/status/ca/cacert.crt";
    //mqttSsl.privateKey = "/usrdata/status/ca/1671639__sgmwcloud.com.cn.pem";
    //mqttSsl.privateKeyPassword = "123456";
    //mqttSsl.enabledCipherSuites = "aNULL";
    mqttSsl.enableServerCertAuth = false;
    mqttSsl.sslVersion = MQTT_SSL_VERSION_TLS_1_2;
    //mqttSsl.CApath = "/usrdata/status/ca/";
    mqttSsl.ssl_error_cb = SslErrorFunc;
    opts.ssl = &mqttSsl;
    opts.connectTimeout = 3;

    LOG_Debug("MQTTClient_connect start");
	rc = MQTTClient_connect(g_connectParam.client, &opts);
	if(rc != MQTTCLIENT_SUCCESS)
	{
        LOG_Error("MQTT connect fail :%d!",rc);
        MQTTClient_destroy(&g_connectParam.client);
        g_connectParam.client = NULL;
        pthread_mutex_unlock(&g_connectParam.lock);
		return RET_ERR;
	}
    LOG_Debug("MQTTClient_connect success");
    // 连接成功后，更改解析名单中的主题和对应的校验码
    DispatchCalTopicCrc();
    //subscribe topic
    sprintf(topicStr,"%s%s/get",E50_DOWNLINK_TOPIC,g_root["content"]["deviceId"].asString().c_str());
    LOG_Debug("subscribe topic: %s",topicStr);
    MQTTClient_subscribe(g_connectParam.client, topicStr, E50_QOS1);
    GetIccid(c8Iccid, sizeof(c8Iccid) - 1);
    sprintf(topicStr,"%s%s/cmd/get",E50_FOTA_TOPIC,c8Iccid);
    LOG_Debug("subscribe topic: %s",topicStr);
    MQTTClient_subscribe(g_connectParam.client, topicStr, E50_QOS1);
    pthread_mutex_unlock(&g_connectParam.lock);
    
    return RET_OK;
}

static gint  MQTTLogin(void)
{
    return DiagnosisLogin();
}

static gint  MQTTRecv(void)
{
    // MQTTClient_message* m = NULL;
    // gchar* topicName = NULL;
    // gint topicLen;
    // //pthread_mutex_lock(&g_connectParam.lock);
    // if(NULL != g_connectParam.client)
    // {
    //     if (MQTTCLIENT_SUCCESS != MQTTClient_receive(g_connectParam.client, &topicName, &topicLen, &m, 1000))
    //     {
    //         pthread_mutex_lock(&g_connectParam.lock);
    //         MQTTClient_disconnect(g_connectParam.client, 0);
    //         MQTTClient_destroy(&g_connectParam.client);
    //         checkAndSetLogLevel("DEBUG");
    //         g_connectParam.client = NULL;
    //         pthread_mutex_unlock(&g_connectParam.lock);
    //         return RET_ERR;
    //     }
       
    //     if (topicName)
    //     {
    //         if(SGMW_PM_STATE_SLEEP == GetPmState())
    //         {
    //             // F510C新加，如果改为加密数据则不可用
    //             // fota唤醒和OTA上电（存疑）
    //             if(m->payloadlen > 24)
    //             {
    //                 if((!strncmp("41BE",(const char *)m->payload+18,4) || !strncmp("021B",(const char *)m->payload+30,4)))
    //                 {
    //                     PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_OTA);
    //                     PmSetSleepEvent(PM_SLEEP_EVENT_BUSYING);
    //                     LOG_Debug("OTA wake up");
    //                 }
    //                 else if(!strncmp("51BE",(const char *)m->payload+18,4) || !strncmp("24DB",(const char *)m->payload+18,4))
    //                 {
    //                     PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DTC);
    //                     LOG_Debug("DTC wake up");
    //                 }
    //                 else if ((!strncmp("23DC",(const char *)m->payload+18,4)) || (!strncmp("26D9",(const char *)m->payload+18,4)))//远程参数查询及配置及参数配置
    //                 {
    //                     PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
    //                     LOG_Debug("get information wake up");
    //                 }
    //                 else if(!strncmp("22DD",(const char *)m->payload+18,4))
    //                 {
    //                     if(  (!strncmp("0004",(const char *)m->payload+22,4))||(!strncmp("0007",(const char *)m->payload+22,4))  )//修改VIN及重启
    //                     {
    //                         PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
    //                     }
    //                     else
    //                     {
    //                         PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
    //                     }
    //                 }
    //                 else
    //                 {
    //                     PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
    //                 }
    //             }
    //         }
    //         //printf("topic:%s,payload:!%s", topicName, m->payload);
    //         //call set cache
    //         LOG_Debug("topic:%s,payload:%s,payloadlen :%d", topicName, m->payload,m->payloadlen);
    //         //将接收到的mqtt消息放入消息队列，等待解析
    //         CacheDispatchData(topicName, (guchar *)m->payload, m->payloadlen);
    //         MQTTClient_free(topicName);
    //         MQTTClient_freeMessage(&m);
    //     }
    // }
    // else 
    // {
    //     LOG_Error("g_connectParam.client Is NULL");
    //     return RET_ERR;
    // }
    // //pthread_mutex_unlock(&g_connectParam.lock);
    // return RET_OK;
}

static gint  MQTTLogout(void)
{
    //unsubscribe topic
    DiagnosisLogout();
    pthread_mutex_lock(&g_connectParam.lock);
    if(NULL != g_connectParam.client)
    {
        if(g_tspLogoutType == E_TSP_LOGOUT_RECV)
        {
            //gchar topic[128] = {0x0,};
            //gchar c8Sn[128] = {0x0,};
            //gchar ip1[16] = {0x0,};
            //gchar ip2[16] = {0x0,};
            //gchar buffer[2048] = {0x0,};
    
            if (!CheckTboxSNIsEmpty())
            {
                MQTTClient_disconnect(g_connectParam.client, 0);
                MQTTClient_destroy(&g_connectParam.client);
                g_connectParam.client = NULL;
                pthread_mutex_unlock(&g_connectParam.lock);
                return RET_OK;
            }
            MQTTClient_unsubscribe(g_connectParam.client, E50_DOWNLINK_TOPIC);
        }
        MQTTClient_disconnect(g_connectParam.client, 0);
        MQTTClient_destroy(&g_connectParam.client);
        g_connectParam.client = NULL;
    }
    pthread_mutex_unlock(&g_connectParam.lock);
    return RET_OK;
}


#define OtaConnectToServer MQTTConnectToServer
#define OtaConnectLogin MQTTLogin
#define OtaConnectRecv MQTTRecv
#define OtaConnectLogout MQTTLogout
#else //tcp
#endif

static gint  OtaConnectStatMachine(void)
{
    static gint creatAlarmFlag = 0;
    gint kl30 = -1;
    //if(1 == creatAlarmFlag)
    {
        PmDestroySleepAlarm(CPU_SEND_HEARTBEAT_SLEEPALARM_EVENT_ID);
        creatAlarmFlag = 0;
    }
    
    //OtaConnectRequestToken();
    while(1)
    {


        LOG_Info("g_connectParam.u8CurrStat = %d",g_connectParam.u8CurrStat);
        switch(g_connectParam.u8CurrStat)
        {
        case E_OTA_CONNECT_REQUEST_TOKEN:
            GetSystemRetValue("echo 1 > /oemdata/tmp/.tsp_connectStatus");
            // 与TSP平台交互，获得TOKEN，用于拼接成用户名和密码
            if(RET_OK == OtaConnectRequestToken())
            {
                g_connectParam.u8CurrStat = E_OTA_CONNECT_SERVER;
            }
            break;
        case E_OTA_CONNECT_SERVER:
            //拿到TOKEN值后，连接SERVER
            // 创建MQTT的客户端
            if(RET_OK == OtaConnectToServer())
            {
                g_connectParam.u8CurrStat = E_OTA_CONNECT_LOGIN;
            }
            else
            {
                g_connectParam.u8CurrStat = E_OTA_CONNECT_REQUEST_TOKEN;
            }
            break;
        case E_OTA_CONNECT_LOGIN:
            // 发送一个登陆包，并主动上报版本
            if(RET_OK == OtaConnectLogin())
            {
                g_connectParam.u8CurrStat = E_OTA_CONNECT_RECV;
                sleep(3);//等待读取MAC
                DiagnosisUploadeVersion();
                //RequestConfigFromTsp();
            }
            else
            {
                g_tspLogoutType = E_TSP_LOGOUT_LOGIN;
                g_connectParam.u8CurrStat = E_OTA_CONNECT_LOGOUT;
            }
            break;
        case E_OTA_CONNECT_RECV:
            //登录成功后，超时接受平台数据
            GetSystemRetValue("echo 0 > /oemdata/tmp/.tsp_connectStatus");
            if(RET_OK == OtaConnectRecv())
            {
                g_connectParam.u8CurrStat = E_OTA_CONNECT_RECV;
                NetOffToOnAndSendHeartBeat();
            }
            else
            {
                //g_tspLogoutType = E_TSP_LOGOUT_RECV;
                g_connectParam.u8CurrStat = E_OTA_CONNECT_SERVER;
            }
            break;
        case E_OTA_CONNECT_LOGOUT:
            //if(RET_OK == OtaConnectLogout())
            GetSystemRetValue("echo 1 > /oemdata/tmp/.tsp_connectStatus");
            {
                g_connectParam.u8CurrStat = E_OTA_CONNECT_NONE;
                //if(0 == creatAlarmFlag)
                {
                    PmCreateSleepAlarm(CPU_SEND_HEARTBEAT_SLEEPALARM_EVENT_ID, 560, SGMW_PM_WAKEUP_SOURCE_TIMER_560);
                    creatAlarmFlag = 1;
                }
                return RET_OK;
            }
            break;
        default:
            g_connectParam.u8CurrStat = E_OTA_CONNECT_RECV;
            break;
        }

        //kl30 = GetKl30Voltage();
        kl30 = g_mcuSyncKine.kl30;
        if(kl30 >= 0 && (kl30 < 9000 || kl30 > 16000))
        {
            LOG_Debug("kl30:%d,more then 16v of less 9v,logout",kl30);
            g_tspLogoutType = E_TSP_LOGOUT_RECV;
            g_connectParam.u8CurrStat = E_OTA_CONNECT_LOGOUT;
            pthread_mutex_lock(&g_sendHeartBeatLock);
            // 登出平台
            OtaConnectLogout();
            pthread_mutex_unlock(&g_sendHeartBeatLock);
        }

        if (SGMW_PM_STATE_SLEEP == GetPmState())
        {
            LOG_Debug("Sleeped");
            if (0 >= GetLengthFromQueue(g_connectParam.g_v3Tsp))
            {
                if(GetLogThreadStat() > 0)
                {
                    LOG_Debug("tar pthread is existed,refuse sleep");
                    continue;
                }
                //wait queue empty
                g_tspLogoutType = E_TSP_LOGOUT_SLEEP;
                //在休眠的状态下，将状态机状态改为休眠
                g_connectParam.u8CurrStat = E_OTA_CONNECT_LOGOUT;

                // 休眠状态下
                pthread_mutex_lock(&g_sendHeartBeatLock);
                // 告诉平台进入休眠
                g_sendHeartBeat = DiagnosisSleep();
                pthread_mutex_unlock(&g_sendHeartBeatLock);

                continue;
            }
        }

        usleep(100000);
    }
    
    return RET_OK;
}


static gint MQTTConnectToZelosServer(void)
{
    StrNetStatus_t netStatus = {0};
    gchar username[256] = {0};             //存放 90 MQTT客服端 登录用户名
    gchar password[256] = {0};             //存放 90 MQTT客服端 登录密码
    gchar mqttUrl[256] = {0};              //存放 90 MQTT客服端 地址url
    char cmdBuff[1024] = {0};              //存放命令缓冲区
    char persistenceStore[1024] = {0};  
    char vindata[8] = {0};   
    char clientid[20] = {0};
    gint rc = 0;
    gint retValue = RET_ERR; 
    //加锁
    pthread_mutex_lock(&g_connectParam.lock);              
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;  //MQTT客户端结构体初始化
    struct hostent *host = NULL;                                             //主机信息结构体初始化

    retValue = GetNetStatusInfo(&netStatus, 1000);
    if( retValue != RET_OK || netStatus.netStatusInfo[0].dialStatus != 1)
    {
        LOG_Error("MQTTConnectToZelosServer, app dial failed... ");
        sleep(1);
        return RET_ERR;
    }

    //解析90 MQTT端域名成IP地址
    host = gethostbyname(MQTT_CONNECT_ZELOS_IP_ADDR);
    //检查
    if (NULL == host)
    {
        LOG_Debug(" %s ,gethostbyname error 530", __FUNCTION__);
        printf(" %s ,gethostbyname error 530\n", __FUNCTION__);
        //解锁 这里加sleep的原因 防止gethostbyname error后 多次频繁解析 
        sleep(1);
        pthread_mutex_unlock(&g_connectParam.lock); 
        return RET_ERR;
    }

    // //拼接 路由添加命令 示例：route add IP地址 ppp0
    // snprintf(cmdBuff, 1024,"route add %s %s", inet_ntoa(*((struct in_addr *)host->h_addr)), APN1_INTERFACE);
    // //获取执行命令返回结果
    // int ret = GetSystemRetValue(cmdBuff);
    // //打印路由添加命令执行结果
    // LOG_Debug(" 538 index [0] cmdBuff exe ret [%d]:%s", ret, cmdBuff);
    
    //拼接 90 mqtt的url
    retValue=snprintf(mqttUrl, sizeof(mqttUrl), "tcp://%s:%d", MQTT_CONNECT_ZELOS_IP_ADDR, MQTT_CONNECT_ZELOS_PORT);
    //检查拼接的结果
    if (retValue <= 0 || retValue >= sizeof(mqttUrl))
    {
        LOG_Error("545 snprintf mqttUrl Error");
        //解锁
        pthread_mutex_unlock(&g_connectParam.lock);
        return RET_ERR;
    }
    LOG_Debug("url:%s", mqttUrl);

    //设置Clientid 构成形式  车号_mqtt_1
    memcpy(vindata, "TS00010", 7);
    vindata[7] = '\0'; // 补结束符，确保是字符串
    snprintf(clientid, sizeof(clientid), "%s_mqtt_1", vindata);
    LOG_Debug("clientid is: %s", clientid);

    // 创建 MQTT 客户端
    rc = MQTTClient_create(&g_connectParam.client, mqttUrl, clientid, MQTTCLIENT_PERSISTENCE_NONE, persistenceStore);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        LOG_Error(" MQTT create fail :%d!", rc);
        pthread_mutex_unlock(&g_connectParam.lock);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("MQTT create success, client = %p", (void *)g_connectParam.client);
    }
    //配置客户端连接参数结构体
    opts.keepAliveInterval = 20;            //心跳间隔（秒）
    opts.cleansession = 1;                  //清除临时会话
    opts.username = "zelos";                //用户名
    opts.password = "mqttZelos@20220802";   //密码
    opts.connectTimeout = 20;               //连接超时

    LOG_Debug("581 MQTTClient_connect start");
    //连接
    rc = MQTTClient_connect(g_connectParam.client, &opts);
    //如果连接失败
    if (rc != MQTTCLIENT_SUCCESS)
    {
        LOG_Error("587 MQTT connect fail :%d!", rc);
        MQTTClient_disconnect(g_connectParam.client, 0);
        MQTTClient_destroy(&g_connectParam.client);
        g_connectParam.client = NULL;
        //解锁
        pthread_mutex_unlock(&g_connectParam.lock);
        return RET_ERR;
    }

    //打印
    LOG_Debug("MQTTClient_connect success, client = %p", (void *)g_connectParam.client);
    printf("MQTTClient_connect success, client = %p\n", (void *)g_connectParam.client);

    //订阅主题(占时这样写)
    rc = MQTTClient_subscribe(g_connectParam.client, MQTT_PROD_ZELOS_TOPIC, 1);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        LOG_Error("MQTT subscribe fail :%d!", rc);
        MQTTClient_disconnect(g_connectParam.client, 0);
        MQTTClient_destroy(&g_connectParam.client);
        g_connectParam.client = NULL;
        //解锁
        pthread_mutex_unlock(&g_connectParam.lock);
        return RET_ERR;
    }
    LOG_Debug("MQTT subscribe success");
    LOG_Debug("MQTTClient_subscribe success, client = %p", (void *)g_connectParam.client);
    //解锁
    pthread_mutex_unlock(&g_connectParam.lock);
    return RET_OK;
}


static gint MQTT_ZelosRecv(void)
{
    MQTTClient_message *m = NULL;
    gchar *topicName = NULL;
    gint topicLen;
    gint powerstate;

    Json::Value  mqtt_rootvalue;                
    Json::Reader mqtt_reader; 
    
    //这边尝试加锁试试  目的：为了 g_connectParam.client 不被意外设置
    pthread_mutex_lock(&g_connectParam.lock); 
    //如果接收错误，断开连接              
    if (NULL == g_connectParam.client)
    {
        LOG_Error("g_connectParam.client Is NULL");
        printf("g_connectParam.client Is NULL\n");
        //解锁
        pthread_mutex_unlock(&g_connectParam.lock);
        return RET_ERR;
    }
    printf("g_connectParam.client");
    //这边解锁 已保证g_connectParam.client 不被意外设置
    pthread_mutex_unlock(&g_connectParam.lock);
    int rc = MQTTClient_receive(g_connectParam.client, &topicName, &topicLen, &m, 1000);
    printf("Recv MQTTCLIENT_SUCCESS\n");
    if (rc == MQTTCLIENT_SUCCESS)
    {
        LOG_Debug("Recv MQTTCLIENT_SUCCESS!");
    }
    else if (rc == -1) // 如果超时
    {
        LOG_Debug("MQTT receive timeout, no message");
        // 保持连接，不断开
        return RET_OK; 
    }
    else
    {
        LOG_Error("MQTT receive error: %d", rc);
        pthread_mutex_lock(&g_connectParam.lock);
        MQTTClient_disconnect(g_connectParam.client, 0);
        MQTTClient_destroy(&g_connectParam.client);
        g_connectParam.client = NULL;
        pthread_mutex_unlock(&g_connectParam.lock);
        return RET_ERR;
    }

    if (m != NULL && topicName != NULL)
    {
        LOG_Debug("974 Received MQTT message: topic=[%s], len=[%d]", topicName, m->payloadlen);
        printf("Received MQTT message: topic=[%s], len=[%d]\n", topicName, m->payloadlen);
         //转化为C++风格的字符串，并于解析
        std::string payloadStr((char*)m->payload, m->payloadlen);
        LOG_Debug(" payloadStr = %s",payloadStr.c_str());
        printf(" payloadStr = %s\n",payloadStr.c_str());
         //解析字段
         if (mqtt_reader.parse(payloadStr, mqtt_rootvalue))
         {
             int zelos_vehicleState = mqtt_rootvalue["vehicleState"].asInt();
             printf("zelos_vehicleState = %d\n",zelos_vehicleState);
             if(zelos_vehicleState == 0)
             {
                 //执行远程上电
                 powerstate = ZelosControlRemotePowerup();
                 if(powerstate == -1)
                 {
                    LOG_Error("Zelos Remote Powerup is fail");
                    return RET_ERR;
                 }     
             }
             else if (zelos_vehicleState == 3)
             {
                  //执行远程下电
                  powerstate = ZelosControlRemotePowerdown();
                  if (powerstate == -1)
                  {
                      LOG_Error("Zelos Remote Powerdown is fail");
                      return RET_ERR;
                  }
             }
         }
    }
    else
    {
        LOG_Debug("979 MQTTClient_receive succeeded, but m=%p, topicName=%p", m, topicName);     
    }
    return RET_OK;
}


static gint MQTT_ZelosLogout(void)
{
    //加锁
    pthread_mutex_lock(&g_connectParam.lock);
    if (NULL != g_connectParam.client)
    { 
        MQTTClient_disconnect(g_connectParam.client, 0);
        MQTTClient_destroy(&g_connectParam.client);
        g_connectParam.client = NULL;
        //解锁
         pthread_mutex_unlock(&g_connectParam.lock);
    }
    //解锁
    pthread_mutex_unlock(&g_connectParam.lock);
    return RET_OK;
}



#define OtaConnectToZelosServer MQTTConnectToZelosServer
#define OtaConnectZelosRecv     MQTT_ZelosRecv
#define OtaConnectZelosLogout   MQTT_ZelosLogout

static gint OtaConnectZelosStatMachine(void)
{
    gint retValue = 0;
    gint connectEorrorCount = 0;
    while (1)
    {
        LOG_Debug("1151 g_connectParam.u8CurrStat = %d", g_connectParam.u8CurrStat);
        switch (g_connectParam.u8CurrStat)
        {
        case E_OTA_CONNECT_90_SERVER:
            if (RET_OK == OtaConnectToZelosServer())
            {
                LOG_Debug("1176 Zelos RECV Success");
                g_connectParam.u8CurrStat = E_OTA_CONNECT_90_RECV;
                // sleep(1); 
            }
            else
            {
                LOG_Error("Zelos RECV failed, Reconnecting");
                g_connectParam.u8CurrStat = E_OTA_CONNECT_90_SERVER;
                sleep(5); 
            }
            break;
        // case E_OTA_CONNECT_90_LOGIN:
        //      LOG_Debug(" 1158 will enter Zelos RECV");
        //      g_connectParam.u8CurrStat = E_OTA_CONNECT_90_RECV;
        //     break;
        case E_OTA_CONNECT_90_RECV:
            if (RET_OK == OtaConnectZelosRecv())
            { 
                LOG_Debug("1176 Zelos RECV Success");
                g_connectParam.u8CurrStat = E_OTA_CONNECT_90_RECV;
            }
            else
            {
                // g_tspLogoutType = E_TSP_LOGOUT_RECV;
                g_connectParam.u8CurrStat = E_OTA_CONNECT_90_SERVER;
            }
            break;
        case E_OTA_CONNECT_90_LOGOUT:
            if (RET_OK == OtaConnectZelosLogout())
            {
                g_connectParam.u8CurrStat = E_OTA_CONNECT_90_NONE;
                return RET_OK;
            }
            break;
        default:
            break;
        }
        
        //获取电源状态 进行登出
        // if (SGMW_PM_STATE_NORMAL != GetPmState())
        // {
        //     LOG_Debug("Sleeped");
        //     // g_sleepMode = PM_STATE_SLEEP;

        //     // g_tspLogoutType = E_TSP_LOGOUT_SLEEP;
        //     g_connectParam.u8CurrStat = E_OTA_CONNECT_LOGOUT;
        //     continue;
        // }
        // sleep(1);
    }
    return RET_OK;
}

static void PmStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    MPUPmStateChange_t *pmStatus = NULL;

    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(dataLen != sizeof(MPUPmStateChange_t))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY);
    pmStatus = (MPUPmStateChange_t *)recvData;
    uint8_t oldPmstatus = GetPmState(); 
    if(pmStatus->PmState != oldPmstatus)
    {
        SetPmState(pmStatus->PmState);
    }
    SetPmState(pmStatus->PmState);
    // 如果电源状态改为唤醒,发送唤醒包;
    if ((SGMW_PM_STATE_NORMAL == pmStatus->PmState) && (SGMW_PM_STATE_SLEEP == oldPmstatus))
    {
        g_sendHeartBeat = DiagnosisWakeUp(pmStatus->WakeupSrc);
    }
    //else if ((SGMW_PM_STATE_SLEEP == pmStatus->PmState) && (SGMW_PM_STATE_SLEEP == oldPmstatus))
    //{
        // 切换为休眠状态,或者休眠状态唤醒源为发送休眠包,发送心跳包会被认为在线
    //    g_sendHeartBeat = DiagnosisSleep();
    //}

    if (PM_WAKEUP_SOURCE_TIMER_HEARTBEAT == pmStatus->WakeupSrc)
    {
        // 切换为休眠状态,或者休眠状态唤醒源为发送休眠包,发送心跳包会被认为在线
        PmSetSleepEvent(PM_TSPHEART_APP_NAME);
        g_sendHeartBeat = DiagnosisSleep();
        PmCleanSleepEvent(PM_TSPHEART_APP_NAME);
    }
    

}

static void KLINEStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    if((NULL == recvData) || (NULL == userdata) || (8 > dataLen))
    {
        LOG_Error("%s argv is invalid, dataLen = %d", __FUNCTION__, dataLen);
        return;
    }

    LOG_Debug("%s, recv DataLen:%d, type:%08x, userdata:%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW, (uint64_t)userdata);
    LOG_DebugHex(recvData, dataLen, "mcu_debug");

    pthread_mutex_lock(&g_sendHeartBeatLock);
    uint8_t tmpBuf[dataLen + 1] = {0};
    memcpy(tmpBuf, recvData, dataLen);
    g_mcuSyncKine.kl15 = tmpBuf[0];
    g_mcuSyncKine.kl30 = tmpBuf[1] * 256 + tmpBuf[2];
    //g_mcuSyncKine.fastCharging = tmpBuf[3] * 256 + tmpBuf[4];
    g_mcuSyncKine.canNet = tmpBuf[7];
    //LOG_Debug("%s, KL15 Status:%02x, KL30 Status:%d, Charge Status:%04x", __FUNCTION__, g_mcuSyncKine.kl15, (int32_t)g_mcuSyncKine.kl30, g_mcuSyncKine.fastCharging);
    LOG_Debug("%s, KL15 Status:%02x, KL30 Status:%d, canNet:%02x", __FUNCTION__, g_mcuSyncKine.kl15, (int32_t)g_mcuSyncKine.kl30, g_mcuSyncKine.canNet);
    pthread_mutex_unlock(&g_sendHeartBeatLock);
}

void *ConnectTaskEntry(void *pArg)
{
    GMainLoop *loop = (GMainLoop *) pArg;
    // 创建一个消息队列，用于将接收到的消息传递给消息处理线程
    g_connectParam.g_v3Tsp = CreadNewCacheQueue();
    gint sendHeartTimes = 0;
    gint kl30 = -1;
    char setSleepEventFlag = 0;
    if (!g_connectParam.g_v3Tsp)
    {
        LOG_Error("create queue fail!");
        return NULL;
    }
#ifdef SUPPORT_MQTT
    g_connectParam.client = NULL;
#else
    g_connectParam.socketFd = -1;    
#endif
    pthread_mutex_init(&g_connectParam.lock, NULL);
    pthread_mutex_init(&g_sendHeartBeatLock, NULL);
    g_sendHeartBeat = RET_ERR;
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, PmStatusCallback, NULL);   //订阅电源状态变化
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW, KLINEStatusCallback, (void *)(SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW));   //订阅车辆硬线状态变化
    /*******这里占时取消原来五菱的九分半心跳包策略和电源管理策略，后面根据九识需求增加*********/

    //验证VIN是否为0，如果不为0，设置休眠事件，阻止休眠
    // if (!CheckTboxVINIsEmpty())
    // {
    //     PmSetSleepEvent(PM_TSP_APP_NAME);
    //     setSleepEventFlag = 1;
    // }

    while (1)
    {
    //     if (CheckTboxVINIsEmpty())
    //     {
    //         g_usleep(G_TIME_SPAN_SECOND);
    //         LOG_Debug("CheckVINIsEmpty");
    //         continue;
    //     }

    //    if(SGMW_PM_STATE_SLEEP == GetPmState())
    //     {
    //         sleep(1);
    //         continue;
    //     }

        sendHeartTimes = 0;
        g_connectParam.u8CurrStat = E_OTA_CONNECT_90_SERVER;
        // PmSetSleepEvent(PM_TSP_APP_NAME);
        LOG_Debug("will enter Zelos tsp connect machine");
        OtaConnectZelosStatMachine();
        // PmCleanSleepEvent(PM_TSP_APP_NAME);
        sleep(1);
    }

    // g_thread_unref( getTspWakeupInfo );
}
