#include "mqtt/MQTTClient.h"
#include "mqtt_connect.h"
#include "mqtt_gettoken.h"
#include "mqtt_dispatch.h"
#include <unistd.h>
#include <fcntl.h>
#include "mqtt_proto_package.h"
#include "mqtt_evet_handle.h"
#include "mcu_api.h"
#include "json/inc/json.h"
#include "mqtt_get_modem_info.h"
#include "mqtt_gateway.h"
#include "common_functions.h"
#include "mqtt_srlog_upload.h"
#include "hal_modem_nw.h"
#include "mqtt_qos.h"
#include "pki_sdk.h"
#include "mqtt_log_coord.h"

#include "net_service_api.h"

using namespace std;


#define MQTT_PROD_ZELOS_TOPIC       "ZL02813/VehicleState/set" 

#define SUPPORT_MQTT
typedef enum OtaConnectStat
{
    E_OTA_CONNECT_REQUEST_TOKEN,
    E_OTA_CONNECT_SERVER,
    E_OTA_CONNECT_LOGIN,
    E_OTA_CONNECT_RECV,
    E_OTA_CONNECT_LOGOUT,
    E_OTA_CONNECT_NONE,
} OtaConnectStat;

typedef struct
{
    GAsyncQueue *g_v3Tsp;
#ifdef SUPPORT_MQTT
    MQTTClient client;
#else
    gint socketFd;
#endif
    guchar u8CurrStat;
    pthread_mutex_t lock;
} Ota_Connect_Param_t;

static Ota_Connect_Param_t g_connectParam;

typedef enum
{
    E_TSP_LOGOUT_NOEN,
    E_TSP_LOGOUT_SLEEP,
    E_TSP_LOGOUT_RECV,
    E_TSP_LOGOUT_LOGIN,
} TspLogoutTpye;

//static TspLogoutTpye g_tspLogoutType = E_TSP_LOGOUT_NOEN;
// static Json::Reader g_reader;
// static Json::Value g_root;

static gint g_sendHeartBeat = 0;
pthread_mutex_t g_KLINEStatusLock; 
static gint g_canNet = 0;
//static gint g_sleepMode = 0;
static gint g_needTokenFlag = 1;
static int32_t  g_SendStatus = 0;
static uint8_t g_qosSwitch = 0;

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

int32_t GetMqttSendStatus(void)
{
    int32_t retValue = RET_ERR;

    if(0 == pthread_mutex_trylock(&g_connectParam.lock))
    {
        retValue = g_SendStatus;
        pthread_mutex_unlock(&g_connectParam.lock);
    }
    
    return retValue;
}

//gint GetSleepMode()
//{
//    return g_sleepMode;
//}

gboolean IsConnected(void)
{
    gboolean bIsconnect = FALSE;

    pthread_mutex_lock(&g_connectParam.lock);
    bIsconnect = (NULL != g_connectParam.client);
    pthread_mutex_unlock(&g_connectParam.lock);
    return bIsconnect;
}

static gint GetMsgId(void)
{
    static gint msgid = 0x0;

    return ++msgid;
}

gint SendPackageData(const gchar *topic, gint qos, gpointer u8Data, guint u32length)
{
    gint s32Ret = RET_ERR;
    static gint sendErrorCount = 0;
    pthread_mutex_lock(&g_connectParam.lock);
    if (NULL != g_connectParam.client)
    {
        MQTTClient_deliveryToken dt;
        MQTTClient_message pubmsg = MQTTClient_message_initializer;
        int rc = 0;

        pubmsg.payload = u8Data;
        pubmsg.payloadlen = u32length;
        pubmsg.qos = qos;
        pubmsg.retained = 0;
        pubmsg.msgid = GetMsgId();
        
        LOG_Debug("mqtt publish start\n");
        rc = MQTTClient_publishMessage(g_connectParam.client, topic, &pubmsg, &dt);
        if (NULL == g_connectParam.client)
        {
            LOG_Error("g_connectParam.client Is NULL\n");
            // 解锁
            g_SendStatus = 2;
            pthread_mutex_unlock(&g_connectParam.lock);
            return RET_ERR;
        }
        LOG_Error("MQTTClient_publishMessage rc = %d\n", rc);
        if (rc != MQTTCLIENT_SUCCESS)
        {
            MQTTClient_disconnect(g_connectParam.client, 0);
            // MQTTClient_destroy(&g_connectParam.client);
            // //checkAndSetLogLevel("DEBUG");
            // g_connectParam.client = NULL;
            // LOG_Error("close mqtt client");
            g_SendStatus = 2;
            pthread_mutex_unlock(&g_connectParam.lock);        
            LOG_Error("send topic[%d]:%s error", qos, topic);
            return s32Ret;
        }
        
        rc = MQTTClient_waitForCompletion(g_connectParam.client, dt, 10000);
        if (rc != MQTTCLIENT_SUCCESS)
        {
            LOG_Error("waitForCompletion time out!\n");
            MQTTClient_disconnect(g_connectParam.client, 0);
            // MQTTClient_destroy(&g_connectParam.client);
            // g_connectParam.client = NULL;
            // g_connectParam.u8CurrStat = E_OTA_CONNECT_SERVER; //重新连接
            g_SendStatus = 2;
            pthread_mutex_unlock(&g_connectParam.lock);
            LOG_Error("send topic[%d]:%s error", qos, topic);
            return s32Ret;
        }
        s32Ret = RET_OK;
        g_SendStatus = 1;
        //checkAndSetLogLevel("ERROR");
    }
    else
    {
        // 发送时客户端为空返回-2
        s32Ret = -2;
        //checkAndSetLogLevel("DEBUG");
    }
    pthread_mutex_unlock(&g_connectParam.lock);
    LOG_Debug("send topic[%d]:%s: %d,ret:%d\n", qos, topic, u32length,s32Ret);
    return s32Ret;
}

static gint OtaConnectRequestToken(void)
{
    static time_t oldTime = 0;
    gint retValue = RET_ERR;
    gchar vinData[18] = {0};

    if (time(NULL) < oldTime)
    {
        LOG_Debug("wait [%d] S ", oldTime - time(NULL));
        sleep(1);
        return RET_ERR;
    }

    // 先检测VIN是否存在,不存在5分钟后重试
    if (CheckTboxVINIsEmpty())
    {
        LOG_Error("Get VIN Error or VIN is all zero");
        oldTime = time(NULL) + (60 * 5);
        return RET_ERR;
    }
    
    retValue = GetFileFromMqttTokenCheck();
    if(retValue != RET_OK)
    {
        // 如果文件不存在,从未获取到TOCKEN过,5分钟后重新获取(适当减少等待时间)
        retValue = TaskRequestToken();
        if(retValue == E_RES_VIN_ERROR)
        {
            oldTime = time(NULL) + (24 * 60 * 60);
            LOG_Error("response VIN ERROR, Try again in 24 hours");
            return RET_ERR;
        }        
        else if(retValue != E_RES_OK)
        {
            oldTime = time(NULL) + (5 * 60);
            return RET_ERR; 
        } 
    }
    else if((RET_OK == retValue) && (g_needTokenFlag == 0))
    {
        // 文件存在,并且10分钟登陆失败,重新获取一次tocken
        // 成功使用新的TOCKEN,失败使用上一次TOCKEN
        retValue = TaskRequestToken();

        // 回复VIN错误，24小时后重试
        if(retValue == E_RES_VIN_ERROR)
        {
            oldTime = time(NULL) + (24 * 60 * 60);
            LOG_Error("response VIN ERROR, Try again in 24 hours");
            return RET_ERR;
        }else
        {
            g_needTokenFlag = 1;
        }        
    }

    oldTime = time(NULL) + 5; // 如果成功，5s 后才允许重发
    return RET_OK;
}

int SslErrorFunc(const char *str, size_t len, void *u)
{
    LOG_Debug("error:%s", str);
    LOG_Debug("len:%d", len);
}

#ifdef SUPPORT_MQTT
static gint MQTTConnectToServer(void)
{
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    MQTTClient_SSLOptions mqttSsl = MQTTClient_SSLOptions_initializer;
    StrNetStatus_t netStatus = {0};
    gchar username[256] = {0};
    gchar password[256] = {0};
    gchar mqttUrl[256] = {0};
    gchar tmpBuff[1024] = {0};
    gchar clientid[32] = {0};
    char vindata[8] = {0};
    char ipAddr[32] = {0};
    char topicStr[256] = {0};
    char mqttAddrBuff[128] = {0};
    uint8_t carNum[8] = {0};
    char tmpTopic[128] = {0};
    char persistenceStore[1024] = {0};
    gchar tokenInfo[TOKEN_BUFF_MAX_SIZE] = {0};
    gint rc = 0;
    gint retValue = RET_ERR;
    int ret = 0;

    retValue = GetNetStatusInfo(&netStatus, 1000);
    if( retValue != RET_OK || netStatus.netStatusInfo[0].dialStatus != 1)
    {
        LOG_Error("MQTTConnectToZelosServer, app dial failed... ");
        sleep(1);
        return RET_ERR;
    }

    if(FALSE == GetCarNum(carNum, 8))
    {
        LOG_Error("%s ,GetCarNum error",__FUNCTION__);
        return RET_ERR;
    }

    if(FALSE == GetTspTokenUrlUrl((uint8_t *)mqttAddrBuff, 128))
    {
        LOG_Error("%s ,GetTspTokenUrlUrl error",__FUNCTION__);
        return RET_ERR;
    }

    pthread_mutex_lock(&g_connectParam.lock);

    //设置Clientid 构成形式  车号_mqtt_1
    memcpy(vindata, carNum, 7);
    if(0 != access("/dev/mmcblk0", F_OK))
    {
        snprintf(clientid, sizeof(clientid), "%s_mqtt_2", vindata);
    }
    else
    {
        snprintf(clientid, sizeof(clientid), "%s_mqtt_1", vindata);
    }
    
    LOG_Debug("clientid is: %s\n", clientid);

    // struct hostent *host = NULL;
    // host = gethostbyname(MQTT_CONNECT_IP_ADDR);
	// if(NULL != host)
	// {
	//     char cmdBuff[1024] = {0};
	// 	snprintf(cmdBuff, 1024,"route add %s %s", inet_ntoa(*((struct in_addr *)host->h_addr)), APN1_INTERFACE);
    //     int ret = GetSystemRetValue(cmdBuff);
	//     LOG_Debug("index [0] cmdBuff exe ret [%d]:%s\n", ret, cmdBuff);
	// }
    // else if (NULL == host)
    // {
    //     LOG_Error(" %s ,gethostbyname error",__FUNCTION__);
    //     //这里加sleep的原因 防止gethostbyname error后 多次频繁解析 
    //     sleep(1);
    //     pthread_mutex_unlock(&g_connectParam.lock);
    //     return RET_ERR;
    // }

    retValue = snprintf(mqttUrl, sizeof(mqttUrl), "ssl://%s:%d", mqttAddrBuff, MQTT_CONNECT_PORT);
    if (retValue <= 0 || retValue > sizeof(mqttUrl))
    {
        LOG_Error("snprintf mqttUrl Error\n");
        pthread_mutex_unlock(&g_connectParam.lock);
        return RET_ERR;
    }

    LOG_Debug("url:%s", mqttUrl); 
    // 创建 MQTT 客户端
    rc = MQTTClient_create(&g_connectParam.client, mqttUrl, clientid, MQTTCLIENT_PERSISTENCE_NONE, persistenceStore);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        LOG_Error("MQTT create fail!");
        pthread_mutex_unlock(&g_connectParam.lock);
        return RET_ERR;
    }

     /*TLS1.2 证书相关*/
    ret = CertificateInspection();
    if(ret < 0 )
    {
        LOG_Error("CertificateInspection error ");
        MQTTClient_destroy(&g_connectParam.client);
        g_connectParam.client = NULL;
        pthread_mutex_unlock(&g_connectParam.lock);
        return -1;
    }

    SiRunPKIEngineInit();
    mqttSsl.keyStore   = DEVICE_CERT;  //客户端证书路径
    mqttSsl.trustStore = CERT_CHAIN ;   //ca证书路径
    mqttSsl.privateKey = "engine-sirun";   //客户端密钥路
    mqttSsl.struct_version = 1;
    mqttSsl.sslVersion =  MQTT_SSL_VERSION_TLS_1_2;
    mqttSsl.enableServerCertAuth = true;
    mqttSsl.verify = 1;
    opts.ssl = &(mqttSsl);
    //配置客户端连接参数结构体
    opts.keepAliveInterval = 600;            //心跳间隔（秒）
    opts.cleansession = 0;                  //清除临时会话
    // opts.username = "rds_user";
    // opts.password = "Pass123456";
    opts.username = "zelos";                //用户名
    opts.password = "mqttZelos@20220802";   //密码
    // opts.username = "TSFXL01";                //用户名
    // opts.password = "zH44znujy0vzu9zv4b0D5m5Pvz0j484Cuq8HyyrfumrjnWzyCP11CS9Krj599G9H";   //密码
    opts.connectTimeout = 20;               //连接超时
    //连接
    LOG_Debug("MQTTClient connect start!");
    rc = MQTTClient_connect(g_connectParam.client, &opts);
    //如果连接失败
    if (rc != MQTTCLIENT_SUCCESS)
    {
        LOG_Error("MQTT connect fail :%d!", rc);
        MQTTClient_destroy(&g_connectParam.client);
        g_connectParam.client = NULL;
        pthread_mutex_unlock(&g_connectParam.lock);
        return RET_ERR;
    }
    //打印
    snprintf(tmpTopic, 128, "%s/VehicleState/set", carNum);

    LOG_Debug("tmpTopic :%s ", tmpTopic);
    rc = MQTTClient_subscribe(g_connectParam.client, tmpTopic, 1);
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

    //打印
    snprintf(tmpTopic, 128, "%s/FunctionControl/set", carNum);
    LOG_Debug("tmpTopic :%s ", tmpTopic);
    rc = MQTTClient_subscribe(g_connectParam.client, tmpTopic, 1);
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

    pthread_mutex_unlock(&g_connectParam.lock);
    LOG_Debug("MQTTClient_connect success\n");
    return RET_OK;
}

static gint MQTTLogin(void)
{
    return 0;
    // return DiagnosisLogin();
}

#define MSG_CONTROL (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_DOOR & 0xffffff00)
#define MSG_SUBCMD_CONTROL_START_ENGINE (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_START_ENGINE & 0xff)
#define MSG_SUBCMD_CONTROL_TURNOFF_ENGINE (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_TURNOFF_ENGINE & 0xff)
#define MSG_SUBCMD_CONTROL_GATEWAY_SIM1 (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_GATEWAY_SIM1 & 0xff)
#define MSG_SUBCMD_CONTROL_GATEWAY_SIM2 (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_GATEWAY_SIM2 & 0xff)
#define MSG_SUBCMD_CONTROL_REMOTE_RESET (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_RESET & 0xff)
#define MSG_SUBCMD_CONTROL_FUNCTION_SWITCH (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_FUNCTION_SWITCH & 0xff)

#define MSG_SUBCMD_CONTROL_DISP_FRCE (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_DISP_FRCE & 0xff)

gint SendRemoteControlDataToMcu(guchar controlCmd ,guchar *controlDataBuff,gshort dataLen)
{
    gint count = 0;
    if(NULL == controlDataBuff)
    {
        return RET_ERR;
    }
    LOG_Debug("Send Control cmd: %x", controlCmd);
    for(count = 0; count < dataLen; count++)
    {
        LOG_Debug("Control Service Data[%d]: %x", count, controlDataBuff[count]);
    }
    
    LOG_Debug("MSG_CONTROL : %8x", MSG_CONTROL);
    McuMesg_t sendToMcu;
    sendToMcu.type = (SYSQueMsgType_e)(MSG_CONTROL | controlCmd);
    sendToMcu.payloadLen = dataLen;
    sendToMcu.payload = controlDataBuff;

    LOG_Debug("sendToMcu.type : %8x", sendToMcu.type);
    return McuSrvSendMesg(&sendToMcu);
}

gint ZelosControlRemotePowerup(int mode)
{
    gint retValue = RET_ERR;
    guchar controlBuff[4] = {0};
    controlBuff[0] = (guchar)mode;
    controlBuff[1] = 0x00;
    controlBuff[2] = 0x00;
    controlBuff[3] = 0x00;
    LOG_Debug("Zelos Remote Powerup!");
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_START_ENGINE, controlBuff, 4);
    return retValue;
}

gint ZelosControlGatewaySim1()
{
    gint retValue = RET_ERR;
    guchar controlBuff[4] = {0};
    LOG_Debug("Zelos Remote SIM1 !");
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_GATEWAY_SIM1, controlBuff, 4);
    return retValue;
}

gint ZelosControlGatewaySim2()
{
    gint retValue = RET_ERR;
    guchar controlBuff[4] = {0};

    LOG_Debug("Zelos Remote SIM2!");
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_GATEWAY_SIM2, controlBuff, 4);
    return retValue;
}

gint ZelosControlTboxReset()
{
    gint retValue = RET_ERR;
    guchar controlBuff[4] = {0};

    LOG_Debug("Zelos Remote SIM2!");
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_REMOTE_RESET, controlBuff, 4);
    return retValue;
}

gint ZelosControlRemotePowerdown(int mode)
{
    gint retValue = RET_ERR;
    guchar controlBuff[4] = {0};
    controlBuff[0] = (guchar)mode;
    controlBuff[1] = 0x00;
    controlBuff[2] = 0x00;
    controlBuff[3] = 0x00;

    LOG_Debug("Zelos Remote Powerdown!");
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_TURNOFF_ENGINE, controlBuff, 4);
    return retValue;
}


gint ZelosControlSwitchFunction(uint8_t simAuto, uint8_t lowPower, uint8_t functionSwitch)
{
    gint retValue = RET_ERR;
    guchar controlBuff[4] = {0};

    functionSwitch = functionSwitch | 0x42;
    controlBuff[0] = simAuto;      /*切卡开关 1：打开自动切卡,0 关闭自动切卡 */
    controlBuff[1] = lowPower;    /*低功耗开关： 1： 打开低功耗，0： 关闭低功耗*/
    controlBuff[2] = functionSwitch;    /*整体功能开关，需由MCU开机上电时报给CPU,CPU 选择是否开启QOS和锁4G功能*/

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_FUNCTION_SWITCH, controlBuff, 4);
    return retValue;
}

gint ZelosControlCanFileUpload(uint64_t startTime, uint64_t endTime, const char *batchNumber)
{
    LOG_Debug("startTime %ld endTime %ld batchNumber %s", startTime, endTime, batchNumber);
    return SetUpLoadCanFileInfo(startTime, endTime, batchNumber);
}

static gint ZelosControlDispOn()
{
  gint retValue = RET_ERR;
  guchar controlBuff[4] = {0};

  controlBuff[0] = 0x02;

  LOG_Debug("%s control disp frce on", __FUNCTION__);
  retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_DISP_FRCE, controlBuff, 4);
  return retValue;
}

static gint ZelosControlDispOff()
{
  gint retValue = RET_ERR;
  guchar controlBuff[4] = {0};

  controlBuff[0] = 0x01;

  LOG_Debug("%s control disp frce off", __FUNCTION__);
  retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_DISP_FRCE, controlBuff, 4);
  return retValue;
}

int32_t FunctionControlHandle(uint8_t keyValue, uint32_t functionSwitch)
{
    uint8_t simAuto = 0;
    uint8_t lowPower = 0;
    uint8_t qosSwitch = 0;
    uint8_t modeAuto1 = 0;
    uint8_t modeAuto2 = 0;
    char cmdBuff[128] = {0};
    char ipaddrBuff[16] = {0};
    
    // 从MCU获取当前功能开关状态
    SimCardInfoToMpu_t simCardInfoFromMcu;
    int32_t getInfoResult = GetSimCardInfoFromMcu(&simCardInfoFromMcu);
    if (getInfoResult != RET_OK) {
        LOG_Error("Failed to get sim card info from MCU, proceeding with control anyway");
        // 如果无法获取MCU状态，仍然执行控制命令
        return ZelosControlSwitchFunction(simAuto, lowPower, functionSwitch);
    }

    // 从MCU获取的功能开关状态
    uint8_t currentSimAuto = (simCardInfoFromMcu.funcSwitch & 0x01);      // bit0
    uint8_t currentLowPower = (simCardInfoFromMcu.funcSwitch & 0x04) >> 2; // bit2
    uint8_t currentQosSwitch = (simCardInfoFromMcu.funcSwitch & 0x08) >> 3; // bit3
    uint8_t currentModeAuto1 = (simCardInfoFromMcu.funcSwitch & 0x10) >> 4; // bit4
    uint8_t currentModeAuto2 = (simCardInfoFromMcu.funcSwitch & 0x20) >> 5; // bit5

    if(0xA5 != keyValue)
    {
        functionSwitch = 0xFF;
    }

    simAuto = (functionSwitch & 0x01);
    lowPower = (functionSwitch & 0x04) >> 2;
    qosSwitch = (functionSwitch & 0x08) >> 3;
    modeAuto1 = (functionSwitch & 0x10) >> 4;
    modeAuto2 = (functionSwitch & 0x20) >> 5;

    LOG_Debug("Received control values - simAuto: %u, lowPower: %u, qosSwitch: %u, modeAuto1: %u, modeAuto2: %u", 
              simAuto, lowPower, qosSwitch, modeAuto1, modeAuto2);

    LOG_Debug("Current MCU funcSwitch values - simAuto: %u, lowPower: %u, qosSwitch: %u, modeAuto1: %u, modeAuto2: %u", 
              currentSimAuto, currentLowPower, currentQosSwitch, currentModeAuto1, currentModeAuto2);

    // 检查QoS开关状态
    if(currentQosSwitch != qosSwitch) {
        LOG_Debug("QoS switch changing from %u to %u", currentQosSwitch, qosSwitch);
        // 根据平台下发的QoS开关状态更新全局变量
        g_qosSwitch = qosSwitch;
        if(0 != qosSwitch)
        {
            // SrGetInterfaceIp("rmnet_data0", ipaddrBuff, 16);
            // snprintf(cmdBuff, 128, "/oemapp/scripts/iptables.sh rmnet_data0 %s", ipaddrBuff);
            // SrGetSystemRetValue(cmdBuff);
            // SrGetSystemRetValue("touch /oemdata/qoos; sync");
            LOG_Debug("****** Enable QoS ******");
            QoS_Enable();
        }
        else
        {
            // SrGetSystemRetValue("ip rule del fwmark 100 2>/dev/null");
            // SrGetSystemRetValue("ip rule del fwmark 200 2>/dev/null");
            // SrGetSystemRetValue("ip rule del fwmark 300 2>/dev/null");
            // SrGetSystemRetValue("ip rule del fwmark 400 2>/dev/null");
            // SrGetSystemRetValue("ip rule del fwmark 500 2>/dev/null");
            // SrGetSystemRetValue("ip rule del fwmark 600 2>/dev/null");
            // SrGetSystemRetValue("tc qdisc del dev rmnet_data0 root 2>/dev/null");
            // SrGetSystemRetValue("rm /oemdata/qoos; sync");
            LOG_Debug("****** Disable QoS ******");
            QoS_Disable();
        }
    } else {
        LOG_Debug("QoS switch state unchanged (%u), skipping network operation", qosSwitch);
    }

    // 检查modeAuto1状态
    if(currentModeAuto1 != modeAuto1) {
        LOG_Debug("modeAuto1 changing from %u to %u", currentModeAuto1, modeAuto1);
        if(0 != modeAuto1 && (0 == access("/dev/mmcblk0", F_OK)))
        {
            SrGetSystemRetValue("ql_cmd at at+qnwprefcfg=\"mode_pref\",LTE");
            SrGetSystemRetValue("ql_cmd at at+cfun=0");
            SrGetSystemRetValue("ql_cmd at at+cfun=1");
        }
        else
        {
            SrGetSystemRetValue("ql_cmd at at+qnwprefcfg=\\\"mode_pref\\\",AUTO");
            SrGetSystemRetValue("ql_cmd at at+cfun=0");
            SrGetSystemRetValue("ql_cmd at at+cfun=1");
        }
    } else {
        LOG_Debug("modeAuto1 state unchanged (%u), skipping network operation", modeAuto1);
    }

    // 检查modeAuto2状态
    if(currentModeAuto2 != modeAuto2) {
        LOG_Debug("modeAuto2 changing from %u to %u", currentModeAuto2, modeAuto2);
        if(0 != modeAuto2 && (0 != access("/dev/mmcblk0", F_OK)))
        {
            SrGetSystemRetValue("ql_cmd at at+qnwprefcfg=\"mode_pref\",LTE");
            SrGetSystemRetValue("ql_cmd at at+cfun=0");
            SrGetSystemRetValue("ql_cmd at at+cfun=1");
        }
        else
        {
            SrGetSystemRetValue("ql_cmd at at+qnwprefcfg=\\\"mode_pref\\\",AUTO");
            SrGetSystemRetValue("ql_cmd at at+cfun=0");
            SrGetSystemRetValue("ql_cmd at at+cfun=1");
        }
    } else {
        LOG_Debug("modeAuto2 state unchanged (%u), skipping network operation", modeAuto2);
    }

    return ZelosControlSwitchFunction(simAuto, lowPower, functionSwitch);
}


static gint MQTTRecv(void)
{
    MQTTClient_message *m = NULL;
    gchar *topicName = NULL;
    gint topicLen;
    int rc = -1;
    Json::Value  mqtt_rootvalue; 
    Json::Reader mqtt_reader;
    gint powerstate;
    int32_t retValue = -1;

    if (NULL != g_connectParam.client)
    {
        rc = MQTTClient_receive(g_connectParam.client, &topicName, &topicLen, &m, 1000);
        if (MQTTCLIENT_SUCCESS != rc)
        {
            LOG_Debug("MQTTClient_receive  recv error %d\n", rc);
            pthread_mutex_lock(&g_connectParam.lock);
            if(NULL != g_connectParam.client)
            {
                if (rc != MQTTCLIENT_DISCONNECTED) 
                {
                    MQTTClient_disconnect(g_connectParam.client, 0);
                }
                MQTTClient_destroy(&g_connectParam.client);
                g_connectParam.client = NULL;
            }
            pthread_mutex_unlock(&g_connectParam.lock);
            return RET_ERR;
        }
        if (topicName)
        {
            if (SGMW_PM_STATE_NORMAL != GetPmState())
            {
                LOG_Debug("wakeup system and mcu");
                PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
                PmWakeupSetDevices("mcu", 1);
                sleep(1);
            }
            LOG_Debug("974 Received MQTT message: topic=[%s], len=[%d]", topicName, m->payloadlen);
            //转化为C++风格的字符串，并于解析
            std::string payloadStr((char*)m->payload, m->payloadlen);
            LOG_Debug(" payloadStr = %s",payloadStr.c_str());
            //解析字段
            if (mqtt_reader.parse(payloadStr, mqtt_rootvalue))
            {
                if(mqtt_rootvalue["vehicleState"].isNull())
                {
                    LOG_Debug("vehicleState is NULL");
                }
                else
                {
                    int zelos_vehicleState = mqtt_rootvalue["vehicleState"].asInt();
                    //远程上下电模式
                    //0： 正常模式 1： 低功耗模式
                    int zelos_ControlMode = 0;
                    LOG_Debug("zelos_vehicleState = %d\n",zelos_vehicleState);
                    switch(zelos_vehicleState)
                    {
                        case 0:             //执行远程上电
                            if(mqtt_rootvalue["ControlMode"].isNull())
                            {
                                LOG_Debug("ControlMode is NULL, set normal 0");
                            }
                            else {
                                zelos_ControlMode = mqtt_rootvalue["ControlMode"].asInt();
                                LOG_Debug("%s powerup mode %d",__FUNCTION__, zelos_vehicleState);
                                if(0 != zelos_ControlMode && 1 != zelos_ControlMode)
                                {
                                  zelos_ControlMode = 0;
                                }
                            }

                            retValue = ZelosControlRemotePowerup(zelos_ControlMode);
                            if(retValue < 0)
                            {
                                LOG_Error("Zelos Remote Powerup is fail");
                            }
                            break;
                        case 1:
                            retValue = ZelosControlGatewaySim1();
                            if(retValue < 0)
                            {
                                LOG_Error("Zelos Remote SIM1 is fail");
                            }
                            break;
                        case 2:
                            retValue = ZelosControlGatewaySim2();
                            if(retValue < 0)
                            {
                                LOG_Error("Zelos Remote SIM2 is fail");
                            }
                            break;
                        case 3:             //执行远程下电
                            if(mqtt_rootvalue["ControlMode"].isNull())
                            {
                                LOG_Debug("ControlMode is NULL, set normal 0");
                            }
                            else {
                                zelos_ControlMode = mqtt_rootvalue["ControlMode"].asInt();
                                LOG_Debug("%s powerdown mode %d",__FUNCTION__, zelos_vehicleState);
                                if(0 != zelos_ControlMode && 1 != zelos_ControlMode)
                                {
                                  zelos_ControlMode = 0;
                                }
                            }
                            retValue = ZelosControlRemotePowerdown(zelos_ControlMode);
                            if (retValue < 0)
                            {
                                LOG_Error("Zelos Remote Powerdown is fail");
                            }
                            break;
                        case 4:
                            break;
                        case 5:
                            retValue = ZelosControlTboxReset();
                            if (retValue < 0)
                            {
                                LOG_Error("Zelos Remote reset is fail");
                            }
                            break;
                        case 9:
                            retValue = ZelosControlDispOff();
                            if (retValue < 0)
                            {
                                LOG_Error("Zelos Remote Disp Off is fail");
                            }
                            break;
                        case 10:
                            retValue = ZelosControlDispOn();
                            if (retValue < 0)
                            {
                                LOG_Error("Zelos Remote Disp On is fail");
                            }
                            break;
                    }
                }
                if(mqtt_rootvalue["key"].isNull())
                {
                    LOG_Debug("vehicleState is NULL");
                }
                else
                {
                    FunctionControlHandle(mqtt_rootvalue["key"].asInt() & 0xFF, mqtt_rootvalue["control1"].asUInt());
                }
                if(true != mqtt_rootvalue["cmdType"].isNull() && true != mqtt_rootvalue["logStart"].isNull() && \
                true != mqtt_rootvalue["logEnd"].isNull() && true != mqtt_rootvalue["batchNumber"].isNull())
                {
                    if(0 == strncmp("LOG", mqtt_rootvalue["cmdType"].asString().c_str(), 3))
                    {
                        uint64_t logStart = (uint64_t)atol(mqtt_rootvalue["logStart"].asString().c_str());
                        uint64_t logEnd = (uint64_t)atol(mqtt_rootvalue["logEnd"].asString().c_str());
                        char batchNumber[64] = {0};
                        strncpy(batchNumber, mqtt_rootvalue["batchNumber"].asString().c_str(), sizeof(batchNumber) - 1);
                        
                        // 判断是否为主模组
                        if (0 != access("/dev/mmcblk0", F_OK)) {
                            // 副模组：使用协同上传机制（异步执行，不阻塞MQTTRecv线程）
                            LOG_Info("Slave module received LOG command, using coordination mode (async)");
                            int ret = SlaveHandleLogUploadAsync(batchNumber, logStart, logEnd);
                            if (ret != RET_OK) {
                                LOG_Error("SlaveHandleLogUploadAsync failed to create thread, ret=%d", ret);
                            }
                        } else {
                            // 主模组：使用协同上传机制（异步执行，不阻塞MQTTRecv线程）
                            LOG_Info("Master module received LOG command, using coordination mode (async)");
                            int ret = MasterHandleLogUploadAsync(batchNumber, logStart, logEnd);
                            if (ret != RET_OK) {
                                LOG_Error("MasterHandleLogUploadAsync failed to create thread, ret=%d", ret);
                            }
                        }
                    }
                    else
                    {
                        LOG_Error("can upload cmdType is error");
                    }            
                            //   if(0 == strncmp("LOG", mqtt_rootvalue["cmdType"].asString().c_str(), 3))
        //   {
        //     ZelosControlCanFileUpload((uint64_t)atol(mqtt_rootvalue["logStart"].asString().c_str()), \
        //         (uint64_t)atol(mqtt_rootvalue["logEnd"].asString().c_str()), \
        //         mqtt_rootvalue["batchNumber"].asString().c_str());
        //   }
        //   else
        //   {
        //     LOG_Error("can upload cmdType is error");
        //   }  
                }
                else
                {
                    LOG_Error("can upload key is error");
                }
                
            }
            MQTTClient_free(topicName);
            MQTTClient_freeMessage(&m);
        }
    }
    else
    {
        LOG_Error("g_connectParam.client Is NULL");
        return RET_ERR;
    }
    return RET_OK;
}

static gint MQTTLogout(void)
{
    // unsubscribe topic

    pthread_mutex_lock(&g_connectParam.lock);
    if (NULL != g_connectParam.client)
    {
        // if(g_tspLogoutType == E_TSP_LOGOUT_RECV)
        // {
        //     gchar topic[128] = {0x0,};
        //     gchar c8Sn[128] = {0x0,};
        //     gchar ip1[16] = {0x0,};
        //     gchar ip2[16] = {0x0,};
        //     gchar buffer[2048] = {0x0,};

        //     if (RET_OK != GetParamsValue(enumTboxSn, (void *)c8Sn))
        //     {
        //         MQTTClient_disconnect(g_connectParam.client, 0);
        //         MQTTClient_destroy(&g_connectParam.client);
        //         g_connectParam.client = NULL;
        //         pthread_mutex_unlock(&g_connectParam.lock);
        //         return RET_OK;
        //     }
        //     // MQTTClient_unsubscribe(g_connectParam.client, E50_DOWNLINK_TOPIC);
        // }
        MQTTClient_disconnect(g_connectParam.client, 0);
        MQTTClient_destroy(&g_connectParam.client);
        g_connectParam.client = NULL;
    }

    //checkAndSetLogLevel("DEBUG");

    pthread_mutex_unlock(&g_connectParam.lock);
    return RET_OK;
}

#define OtaConnectToServer MQTTConnectToServer
#define OtaConnectLogin MQTTLogin
#define OtaConnectRecv MQTTRecv
#define OtaConnectLogout MQTTLogout
#else // tcp
#endif

static gint OtaConnectStatMachine(void)
{
    gint retValue = 0;
    gint connectEorrorCount = 0;
    static uint8_t sleepFlag = 0;
    char mqttAddrBuff[128] = {0};
    char oldMqttAddrBuff[128] = {0};
    char cmdBuff[128] = {0};
    
    if(FALSE == GetTspTokenUrlUrl((uint8_t *)mqttAddrBuff, 128))
    {
        LOG_Error("%s ,GetTspTokenUrlUrl error",__FUNCTION__);
        return RET_ERR;
    }

    memcpy(oldMqttAddrBuff, mqttAddrBuff, 128);


    while (1)
    {
        if(FALSE == GetTspTokenUrlUrl((uint8_t *)mqttAddrBuff, 128))
        {
            LOG_Error("%s ,GetTspTokenUrlUrl error",__FUNCTION__);
            return RET_ERR;
        }

        if(0 != strncmp(oldMqttAddrBuff, mqttAddrBuff, 128))
        {
            memcpy(oldMqttAddrBuff, mqttAddrBuff, 128);
            g_connectParam.u8CurrStat = E_OTA_CONNECT_LOGOUT;
            LOG_Debug("g_connectParam.u8CurrStat = %d", g_connectParam.u8CurrStat);
        }

        LOG_Debug("g_connectParam.u8CurrStat = %d", g_connectParam.u8CurrStat);
        switch (g_connectParam.u8CurrStat)
        {

        case E_OTA_CONNECT_SERVER:
            if (RET_OK == OtaConnectToServer())
            {
                g_connectParam.u8CurrStat = E_OTA_CONNECT_LOGIN;
                connectEorrorCount = 0;
            }
            else
            {
                // g_connectParam.u8CurrStat = E_OTA_CONNECT_REQUEST_TOKEN;
                // 5s重试连接一次,累计超过10分钟连接失败,重新获取TOCKEN
                sleep(5);
            }
            break;
        case E_OTA_CONNECT_LOGIN:
            {
                g_connectParam.u8CurrStat = E_OTA_CONNECT_RECV;
                SetSendMqttFlag(1);
            }

        case E_OTA_CONNECT_RECV:
                retValue = OtaConnectRecv();
                if(retValue < 0)
                {
                    g_connectParam.u8CurrStat = E_OTA_CONNECT_SERVER;
                }
                else
                {
                    g_connectParam.u8CurrStat = E_OTA_CONNECT_RECV;
                }
                
            break;
        case E_OTA_CONNECT_LOGOUT:
            SetSendMqttFlag(0);
            if (RET_OK == OtaConnectLogout())
            {
                g_connectParam.u8CurrStat = E_OTA_CONNECT_SERVER;
                return RET_OK;
            }
            break;
        default:
            break;
        }

        if(SGMW_PM_STATE_NORMAL != GetPmState())
        {
            LOG_Debug("%s PmState: %s ", __FUNCTION__,GetPmState()==SGMW_PM_STATE_SLEEP?"Sleep":"Normal");
            ClearSwitchUpStatus();
           
        }

        if (SGMW_PM_STATE_NORMAL != GetPmState() && 2 != sleepFlag)
        {
            LOG_Debug("%s Sleeped set RTC wake", __FUNCTION__);
            PmCleanSleepEvent("mqtt");
            //PmCreateSleepAlarm(CPU_SEND_HEARTBEAT_SLEEPALARM_EVENT_ID, 40, 1);
            sleepFlag = 2;
            
            if (g_qosSwitch != 0) {
                LOG_Debug("****** Sleep Disable QoS ******");
                QoS_Disable();
            }
        }
        else if(2 == sleepFlag && SGMW_PM_STATE_NORMAL == GetPmState())
        {
            LOG_Debug("%s RTC wake NORMAL", __FUNCTION__);
            //PmDestroySleepAlarm(CPU_SEND_HEARTBEAT_SLEEPALARM_EVENT_ID);
            PmSetSleepEvent("mqtt");
            LOG_Debug("WakeUp");
            sleepFlag = 1;

            if (g_qosSwitch != 0) {
                LOG_Debug("****** Wakeup Enable QoS ******");
                QoS_Enable();
            }
        }
        sleep(1);
    }

    return RET_OK;
}

void PmStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
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

    LOG_DebugHex(recvData, dataLen, "PmStatus");
    pmStatus = (MPUPmStateChange_t *)recvData;
    if(pmStatus->PmState != GetPmState())
    {
        SetPmState(pmStatus->PmState);
    }
    if(pmStatus->WakeupSrc != PM_WAKEUP_SOURCE_INVALID ){
        SetWakeupSrc(pmStatus->WakeupSrc);
    }
    LOG_Debug("WakeupSrc:%u PmState:%u",pmStatus->WakeupSrc,pmStatus->PmState);
    
}

void KLINEStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    if((NULL == recvData) || (NULL == userdata) || (8 > dataLen))
    {
        LOG_Error("%s argv is invalid, dataLen = %d", __FUNCTION__, dataLen);
        return;
    }

   // LOG_Debug("%s, recv DataLen:%d, type:%08x, userdata:%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW, (uint32_t)userdata);
    LOG_DebugHex(recvData, dataLen, "mcu_debug");

    uint8_t tmpBuf[dataLen + 1] = {0};
    memcpy(tmpBuf, recvData, dataLen);
    if (g_canNet != tmpBuf[7])
    {
        pthread_mutex_lock(&g_KLINEStatusLock);
        g_canNet = tmpBuf[7]; //can网络状态， 0：代表未唤醒 1：代表唤醒
        LOG_Info("%s, canNet:%02x", __FUNCTION__, g_canNet);
        pthread_mutex_unlock(&g_KLINEStatusLock);
    }
    
}

/**
 * @brief 获取QoS开关标志
 * 
 * @return gint 
 */
gint GetQoSFlag(void)
{
    return g_qosSwitch;
}

/**
 * @brief 设置QoS开关标志
 * 
 * @param flag 
 */
void SetQoSFlag(int flag)
{
    g_qosSwitch = flag;
}

void *ConnectTaskEntry(void *pArg)
{
    GMainLoop *loop = (GMainLoop *) pArg;
    gint sendHeartTimes = 0;

#ifdef SUPPORT_MQTT
    g_connectParam.client = NULL;
#else
    g_connectParam.socketFd = -1;
#endif
    pthread_mutex_init(&g_connectParam.lock, NULL);
    pthread_mutex_init(&g_KLINEStatusLock, NULL);

    g_sendHeartBeat = RET_OK;

    g_connectParam.u8CurrStat = E_OTA_CONNECT_SERVER;
    while (1)
    {
#if 0      
        if (CheckTboxVINIsEmpty())
        {
            g_usleep(G_TIME_SPAN_SECOND);
            LOG_Debug("CheckVINIsEmpty");
            continue;
        }

        if ((SGMW_PM_STATE_NORMAL != GetPmState()) || (0 == g_canNet))
        {
            LOG_Debug("%s, pmState:%d, canNet:%d", __FUNCTION__, GetPmState(), g_canNet);
            sleep(1);
            continue;
        }


        sendHeartTimes = 0;
        //g_sleepMode = PM_STATE_NORMAL;


        if (SGMW_PM_STATE_NORMAL != GetPmState())
        {
            OtaConnectRecv();
            sleep(1);
            continue;
        }
#endif
        LOG_Debug("will enter Zelos ota connect machine");
        OtaConnectStatMachine();
        sleep(1);
    }
}

static void wakeupMcuForNetRecovery(const char *reason)
{
  LOG_Warn("%s in sleep mode, waking up MCU", reason);
  PmSetSleepEvent("mqtt_net");
  PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
  PmWakeupSetDevices("mcu", 1);
  sleep(1);
}

static void checkSleepFailure(int *count, int threshold, const char *name, const char *reason)
{
  (*count)++;
  LOG_Debug("%s failure count: %d", name, *count);
  if(*count >= threshold)
  {
    wakeupMcuForNetRecovery(reason);
    *count = 0;
  }
}

static void monitorSleepMode(gboolean reset)
{
  static int regCnt = 0;
  static int dialCnt = 0;
  static int connCnt = 0;

  if(!reset)
  {
    regCnt = 0;
    dialCnt = 0;
    connCnt = 0;
    return;
  }

  SimCardInfoToMcu_t simCardInfo;

  LOG_Debug("%s mqtt STATE_SLEEP", __FUNCTION__);

  if(GetCurrentSimCardInfo(&simCardInfo) != RET_OK)
  {
    LOG_Debug("%s GetCurrentSimCardInfo fail", __FUNCTION__);
    PmCleanSleepEvent("mqtt_net");
    return;
  }

  /* 注网状态: modemSta bit[1] */
  uint8_t regStatus = (simCardInfo.modemSta >> 1) & 0x01;
  if(0 == regStatus)
  {
    checkSleepFailure(&regCnt, 20, "Registration", "Registration failed for 20s");
  }
  else
  {
    regCnt = 0;
  }

  /* 拨号状态 */
  if(0 == simCardInfo.dialStatus)
  {
    checkSleepFailure(&dialCnt, 30, "Dial", "Dial failed for 30s");
  }
  else
  {
    dialCnt = 0;
  }

  /* 连通性 */
  if(0 == simCardInfo.pingStatus)
  {
    checkSleepFailure(&connCnt, 30, "Connectivity", "Connectivity failed for 30s");
  }
  else
  {
    connCnt = 0;
  }

  if(0 == regCnt && 0 == dialCnt && 0 == connCnt)
  {
    LOG_Debug("%s mqtt conn all ok", __FUNCTION__);
    PmCleanSleepEvent("mqtt_net");
  }
  else
  {
    PmSetSleepEvent("mqtt_net");
  }
}

static void monitorNormalMode(gboolean reset)
{
  static int times = 0;

  if(!reset)
  {
    times = 0;
    return ;
  }

  SimCardInfoToMcu_t simCardInfo;

  //如果获取sim卡信息失败，或拨号失败，不继续检测
  if(GetCurrentSimCardInfo(&simCardInfo) != RET_OK 
      || simCardInfo.dialStatus != 1)
  {
    times = 0;
    return;
  }

  //如果mqtt处于上报失败状态持续60s,切飞行
  if(0 == simCardInfo.pingStatus)
  {
    times++;
    if(times > 60)
    {
      LOG_Warn("MQTT disconnected 60s (dial OK), request flight mode");
      RequestNetFlightMode(FLIGHT_MODE_SRC_MQTT_DISCONNECT);
      times = 0;
    }
  }
  else
  {
    times = 0;
  }
}

static int is_gateway_local()
{
  uint8_t gw_location = GetGatewayLocation();

  if(0 == gw_location || gw_location > 2)
  {
    LOG_Error("%s gateway set error!",__FUNCTION__);
    return 0;
  }

  if(0 == access("/dev/mmcblk0", F_OK))
  {
    return 1 == gw_location;
  }
  else
  {
    return 2 == gw_location;
  }

  return 0;
}

// 网络状态监测线程
void *NetworkMonitorThread(void *arg)
{
    SimCardInfoToMcu_t simCardInfo;
    int sleepCounter = 0;
    
    LOG_Info("Network monitor thread started");

  while(1)
  {
    //如果休眠状态下，网关不在当前模组，不进行监测,允许正常休眠
    if((!is_gateway_local()) && (SGMW_PM_STATE_SLEEP == GetPmState()))
    {
      LOG_Debug("%s gateway on %u, not on current modem", __FUNCTION__, GetGatewayLocation());
      sleep(1);
      continue;
    }

    if (SGMW_PM_STATE_SLEEP == GetPmState()) 
    {
      monitorSleepMode(TRUE);
      monitorNormalMode(FALSE);
    }
    else 
    {
      monitorSleepMode(FALSE);
      monitorNormalMode(TRUE);
    }

    sleep(1);
  }

  LOG_Info("Network monitor thread ended");
  return NULL;
}

