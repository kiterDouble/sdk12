#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include "mqtt.h"
#include "queue.h"
#include "sr_param.h"
#include "err_code.h"
#include "sr_log.h"
#include "mqtt_interface.h"
#include "system_interface.h"
#include "mqtt_thread.h"
#include <unistd.h>
#include "modem_function.h"
#include "mcu_interface.h"
#include "completion_timeout.h"
#include "cjson_interface.h"
#include "pki_sdk.h"

// 主副模组判断工具函数
/**
 * @brief 检查是否为主模组
 * @return 1: 主模组, 0: 副模组
 */
static inline int IsMasterModule(void)
{
    return (0 == access("/dev/mmcblk0", F_OK));
}
static MQTT_STATUS mqttstatus = MQTT_INIT;
MqttHandle::MqttHandle()
{
    
}

MqttHandle::~MqttHandle()
{
    
}


// 连接成功的回调函数

void on_connect(void* context, MQTTAsync_successData* response) 
{
    int ret;
    LOG_Debug("Connected to the MQTT broker successfully!");
    MqttHandle::getInstance().client_handle.is_connected = true;
    Complete(GetComp());
    //SiRunPKIEngineDeInit();

}



// 连接失败的回调函数

void on_connect_failure(void* context, MQTTAsync_failureData* response) {

    LOG_Error("Failed to connect to the MQTT broker Reason token: %d  Reason code: %d", response->token,response->code );
    if (response && response->message) {
        LOG_Error("Reason message: %d",response->message);
    }
    MqttHandle::getInstance().client_handle.is_connected = false;
    SetMqttStatus(MQTT_CONNECT);
    Complete(GetComp());
    return ;
}



// 回调函数：连接丢失
void connlost(void *context, char *cause) 
{
    LOG_Error("Connection lost, cause: %s\n", cause);
    SetMqttStatus(MQTT_CONNECT);
}

void freemqtt(mqtt_data *data)
{
    if(!data)
    {
        return;
    }

    if(data->topic)
    {
        free(data->topic);
        data->topic = NULL;
    }

    if(data->payload)
    {
        free(data->payload);
        data->payload = NULL;
    }

    free(data);
    data = NULL;
}

// 回调函数：消息到达
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message) 
{
    int ret;
    mqtt_data *data = NULL;

    LOG_Debug("Message payload len: %d", message->payloadlen);
    LOG_Debug("Message payload len: %s", message->payload);

    data = (mqtt_data *)malloc(sizeof(mqtt_data));
    data->payloadlen = message->payloadlen;
    data->topic_len = topicLen;
    data->topic  = (char *)malloc(topicLen +1);
    memset(data->topic,0,topicLen+1);
    memcpy(data->topic,topicName,topicLen);
    data->payload  = (char *)malloc(message->payloadlen +1);
    memset(data->payload,0,message->payloadlen+1 );
    memcpy(data->payload,(char *)message->payload,message->payloadlen);

    ret = PushMqttDataToQue(data);
    if(ret < 0)
    {
        LOG_Error("PushMqttDataToQue fail\n");
    }

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

// 回调函数：消息传递完成
void deliveryComplete(void *context, MQTTAsync_token token) {
    LOG_Debug("Delivery complete for token: %d\n", token);
}

// 发送成功回调
void onSendSuccess(void *context, MQTTAsync_successData *response) {
    
    LOG_Debug("Message sent successfully! Token: %d\n", response->token);
}

// 发送失败回调
void onSendFailure(void *context, MQTTAsync_failureData *response) {
    LOG_Error("Failed to send message.\n");
    if (response) {
        LOG_Error("Reason code: %d\n", response->code);
        if(response->code == MQTTASYNC_DISCONNECTED)
        {
            LOG_Error("SetMqttStatus(MQTT_CONNECT) \n");
            SetMqttStatus(MQTT_CONNECT);
        }
    }
}


// 初始化 MQTT 客户端
int mqtt_init() 
{
    int rc;
    char carid[30] = {0};
    int ret = 0;
    char clientid[30] = {0};
    char carotaConfigMode[30] = {0};
    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        return -1;
    }

    if(access(EMMCDEVICENODE, F_OK) != 0)
    {
        LOG_Debug("nad2");
        sprintf(clientid,"ota_tbox_nad2_%s",carid);
    }else{
        LOG_Debug("nad1");
        sprintf(clientid,"ota_tbox_nad1_%s",carid);
    }

    if(GetCarotaConfigMode(carotaConfigMode) == -1)
    {
        LOG_Error("GetCarotaConfigMode fail");
        return -1;
    }

    if(strncmp(carotaConfigMode,"prod",strlen("prod")) == 0)
    {
        rc = MQTTAsync_create(&MqttHandle::getInstance().client_handle.s3_mqtt_client, PROD_ADDRESS, clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
        if (rc != MQTTASYNC_SUCCESS) {
            LOG_Error("Failed to create MQTT client PROD_ADDRESS, return code %d\n", rc);
            return rc;
        }
        LOG_Debug("PROD_ADDRESS address = %s  clientid =%s",PROD_ADDRESS,clientid);
    }else if(strncmp(carotaConfigMode,"uat",strlen("uat")) == 0)
    {
        rc = MQTTAsync_create(&MqttHandle::getInstance().client_handle.s3_mqtt_client, UAT_ADDRESS, clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
        if (rc != MQTTASYNC_SUCCESS) {
            LOG_Error("Failed to create MQTT client UAT_ADDRESS, return code %d\n", rc);
            return rc;
        }
        LOG_Debug("UAT_ADDRESS address = %s  clientid =%s",UAT_ADDRESS,clientid);
    }else{
        LOG_Error("CarotaConfigMode Error");
    }
    
    LOG_Debug ("MqttHandle::getInstance().client_handle.s3_mqtt_client = %x\n",MqttHandle::getInstance().client_handle.s3_mqtt_client);
    if( MqttHandle::getInstance().client_handle.s3_mqtt_client  == NULL)
    {
        LOG_Error("MqttHandle::getInstance().client_handle.s3_mqtt_client  == NULL");
        return -1;
    }
    // 设置回调函数
    rc = MQTTAsync_setCallbacks(MqttHandle::getInstance().client_handle.s3_mqtt_client, NULL, connlost, msgarrvd, deliveryComplete);
    if (rc != MQTTASYNC_SUCCESS) {
        LOG_Error("Failed to set callbacks, return code %d\n", rc);
        return rc;
    }

    return MQTTASYNC_SUCCESS;
}


// 连接到 MQTT Broker
int mqtt_connect() 
{
    int rc;
    int ret = 0;
    char tls[10];
    char keepalive[10];
    char *json = NULL;
    char carotaConfigMode[30] = {0};
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_willOptions will_opts = MQTTAsync_willOptions_initializer;
    MQTTAsync_SSLOptions mqttSsl = MQTTAsync_SSLOptions_initializer;

    if(GetCarotaConfigMode(carotaConfigMode) == -1)
    {
        LOG_Error("GetCarotaConfigMode fail");
        return -1;
    }

    ret = CertificateInspection();
    if(ret < 0 )
    {
        LOG_Error("CertificateInspection error ");
    }
    SiRunPKIEngineInit();
    /*SSL 证书相关*/
    mqttSsl.keyStore   = DEVICE_CERT;  //客户端证书路径
    mqttSsl.trustStore = CERT_CHAIN ;   //ca证书路径
    mqttSsl.privateKey = "engine-sirun";   //客户端密钥路径
    //mqttSsl.privateKeyPassword = "123456"; 
    mqttSsl.struct_version = 1;
    mqttSsl.sslVersion =  MQTT_SSL_VERSION_TLS_1_2;
    mqttSsl.enableServerCertAuth = true;
    mqttSsl.verify = 1;
    conn_opts.ssl = &(mqttSsl);

    json = CreateWillJsonInterface();

    will_opts.topicName = MqttHandle::getInstance().topic_handle.tbox_will_topic;
    will_opts.message = json;
    will_opts.qos = 1;
    will_opts.retained = 0;
    if(will_opts.message)
    {
        LOG_Debug("Set will message");
        conn_opts.will = &will_opts;
    }
    
    if(strncmp(carotaConfigMode,"uat",strlen("uat")) == 0)
    {
        conn_opts.username = "rds_user";
        conn_opts.password = "Pass123456";
        
    }else if(strncmp(carotaConfigMode,"prod",strlen("prod")) == 0)
    {
        conn_opts.username = "rds_user";
        conn_opts.password = "Mj2QkeUw(4ERt(=PKA";
    }else{
        LOG_Error("CarotaConfigMode Error");

    }
    conn_opts.keepAliveInterval = 600;
    conn_opts.cleansession = 1;
    conn_opts.connectTimeout = 20; 
    conn_opts.onSuccess = on_connect;
    conn_opts.onFailure = on_connect_failure;
    conn_opts.MQTTVersion = MQTTVERSION_3_1_1;
    conn_opts.context = MqttHandle::getInstance().client_handle.s3_mqtt_client;
    LOG_Debug("MQTTAsync connect start");
    if(!(MqttHandle::getInstance().client_handle.s3_mqtt_client))
    {
        LOG_Error("MqttHandle::getInstance().client_handle.s3_mqtt_client NULL");
        if(json)
        {
            LOG_Debug("free will message");
            free(json);
            json = NULL;
        }
        return -1;
    }
    rc = MQTTAsync_connect(MqttHandle::getInstance().client_handle.s3_mqtt_client, &conn_opts);
    if (rc != MQTTASYNC_SUCCESS) {
        LOG_Error("Failed to connect to broker, return code %d\n", rc);
        if(json)
        {
            LOG_Debug("free will message");
            free(json);
        }
        return rc;
    }
    //printf ("MqttHandle::getInstance().client_handle.s3_mqtt_client = %x\n",MqttHandle::getInstance().client_handle.s3_mqtt_client);
    if(json)
    {
        LOG_Debug("free will message");
        free(json);
    }
    return MQTTASYNC_SUCCESS;
}

// 发布消息
int mqtt_publish(mqtt_data data,int qos) 
{
    int rc;

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    
    opts.onSuccess = onSendSuccess;
    opts.onFailure = onSendFailure;

    pubmsg.payload = (void *)data.payload;
    pubmsg.payloadlen = (int)data.payloadlen;
    pubmsg.qos = qos;
 
    if( pubmsg.payload == NULL)
    {
        LOG_Error("pubmsg.payload null");
        return -1;
    }

    LOG_Debug("data.topic = %s data.payload = %s ",data.topic,data.payload);
    //LOG_Debug ("MqttHandle::getInstance().client_handle.s3_mqtt_client = %x\n",MqttHandle::getInstance().client_handle.s3_mqtt_client);
    if(!(MqttHandle::getInstance().client_handle.s3_mqtt_client))
    {
        LOG_Error("MqttHandle::getInstance().client_handle.s3_mqtt_client NULL");
        return -1;
    }

    if(!MqttHandle::getInstance().client_handle.is_connected)
    {
        LOG_Error("MqttHandle::getInstance().client_handle.is_connected fail");
        return -1;
    }

    rc = MQTTAsync_sendMessage(MqttHandle::getInstance().client_handle.s3_mqtt_client, data.topic, &pubmsg, &opts);
    if (rc != MQTTASYNC_SUCCESS) {
        LOG_Error("Failed to send message, return code %d\n", rc);
        return rc;
    }

    return MQTTASYNC_SUCCESS;
}

// 订阅主题
int mqtt_subscribe() 
{
    int rc = 0;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    LOG_Debug("mqtt subscribe start");
    if(!(MqttHandle::getInstance().client_handle.s3_mqtt_client))
    {
        LOG_Error("MqttHandle::getInstance().client_handle.s3_mqtt_client NULL");
        return -1;
    }

    if(!MqttHandle::getInstance().client_handle.is_connected)
    {
        LOG_Error("MqttHandle::getInstance().client_handle.is_connected fail");
        return -1;
    }

    // rc = MQTTAsync_subscribe(MqttHandle::getInstance().client_handle.s3_mqtt_client, MqttHandle::getInstance().topic_handle.tbox_config_topic, MQTT_QOS, &opts);
    // if (rc != MQTTASYNC_SUCCESS) {
    //     LOG_Error("Failed to subscribe to topic %s, return code %d\n", MqttHandle::getInstance().topic_handle.tbox_config_topic, rc);
    // }
    // rc = rc | rc;
    rc = MQTTAsync_subscribe(MqttHandle::getInstance().client_handle.s3_mqtt_client, MqttHandle::getInstance().topic_handle.tbox_ota_topic, MQTT_QOS, &opts);
    if (rc != MQTTASYNC_SUCCESS) {
        LOG_Error("Failed to subscribe to topic %s, return code %d\n", MqttHandle::getInstance().topic_handle.tbox_ota_topic, rc);
    }
    
    rc = rc | rc;
    // rc = MQTTAsync_subscribe(MqttHandle::getInstance().client_handle.s3_mqtt_client, MqttHandle::getInstance().topic_handle.tbox_can_topic, MQTT_QOS, &opts);
    // if (rc != MQTTASYNC_SUCCESS) {
    //     LOG_Error("Failed to subscribe to topic %s, return code %d\n", MqttHandle::getInstance().topic_handle.tbox_can_topic, rc);
    // }

    // rc = rc | rc;
    
    return rc;
}


int mqtt_unsubscribe()
{

    int rc;
    LOG_Debug("mqtt unsubscribe start");
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    if(!(MqttHandle::getInstance().client_handle.s3_mqtt_client))
    {
        LOG_Error("MqttHandle::getInstance().client_handle.s3_mqtt_client NULL");
        return -1;
    }

    if(!MqttHandle::getInstance().client_handle.is_connected)
    {
        LOG_Error("MqttHandle::getInstance().client_handle.is_connected fail");
        return -1;
    }
    rc = MQTTAsync_unsubscribe(MqttHandle::getInstance().client_handle.s3_mqtt_client, MqttHandle::getInstance().topic_handle.tbox_config_topic,&opts);
    if (rc != MQTTASYNC_SUCCESS) {
        LOG_Error("Failed to unsubscribe to topic %s, return code %d\n", MqttHandle::getInstance().topic_handle.tbox_config_topic, rc);
        return -1;
    }
    rc = rc | rc;
    rc = MQTTAsync_unsubscribe(MqttHandle::getInstance().client_handle.s3_mqtt_client, MqttHandle::getInstance().topic_handle.tbox_ota_topic,&opts);
    if (rc != MQTTASYNC_SUCCESS) {
        LOG_Error("Failed to unsubscribe to topic %s, return code %d\n", MqttHandle::getInstance().topic_handle.tbox_ota_topic, rc);
        return -1;
    }
    rc = rc | rc;    
    return rc;
}

// 断开连接
int mqtt_disconnect() {
    int rc;
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
    if(!MqttHandle::getInstance().client_handle.is_connected)
    {
        
        LOG_Error("is_connected fail");
        return -1;
    }
    rc = MQTTAsync_disconnect(MqttHandle::getInstance().client_handle.s3_mqtt_client, &opts);
    if (rc != MQTTASYNC_SUCCESS) {
        LOG_Error("Failed to disconnect, return code %d\n", rc);
        return rc;
    }

    MqttHandle::getInstance().client_handle.is_connected = false;
    LOG_Error("Disconnected from broker\n");
    return MQTTASYNC_SUCCESS;
}

// 清理资源
void mqtt_cleanup() 
{
    if(!(MqttHandle::getInstance().client_handle.s3_mqtt_client))
    {
        LOG_Debug("MqttHandle::getInstance().client_handle.s3_mqtt_client NULL");
        return ;
    }
    MQTTAsync_destroy(&MqttHandle::getInstance().client_handle.s3_mqtt_client);
    LOG_Debug("Cleaned up MQTT resources\n");
}


int TopicIinit()
{
    char carid[30] = {0};
    int ret = 0;

    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
    }

    LOG_Debug("vehicleNo = %s",carid);
    // sprintf(MqttHandle::getInstance().topic_handle.tbox_config_topic,"vehStatusEvent/cloud/%s",carid);
    // sprintf(MqttHandle::getInstance().topic_handle.tbox_config_reply_topic,"vehStatusEvent/dev/%s",carid);
    
    memset(MqttHandle::getInstance().topic_handle.tbox_will_topic,0,TOPIC_LEN);
    memset(MqttHandle::getInstance().topic_handle.tbox_ota_topic,0,TOPIC_LEN);
    memset(MqttHandle::getInstance().topic_handle.tbox_ota_reply_topic,0,TOPIC_LEN);
    memset(MqttHandle::getInstance().topic_handle.tbox_cyc_topic,0,TOPIC_LEN);




    sprintf(MqttHandle::getInstance().topic_handle.tbox_will_topic,"ota/clients/will/%s",carid);
    sprintf(MqttHandle::getInstance().topic_handle.tbox_ota_topic,"Ota/cloud/%s",carid);
    sprintf(MqttHandle::getInstance().topic_handle.tbox_ota_reply_topic,"Ota/dev/%s",carid);
    sprintf(MqttHandle::getInstance().topic_handle.tbox_cyc_topic,"Ota/prdVehicleStatus/dev/%s",carid);
    // sprintf(MqttHandle::getInstance().topic_handle.tbox_can_topic,"VehicleState/set/%s",carid);
    // sprintf(MqttHandle::getInstance().topic_handle.tbox_can_reply_topic,"vehicle/asc/upload/%s",carid);
    return 0;
}


static int CarVehicleCheck()
{
    char carid[30] = {0};
    int ret = 0;

    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
    }

    char *result = strstr(MqttHandle::getInstance().topic_handle.tbox_ota_topic,carid);
     if (result != NULL) {
        return 0;
    } else {
        return 1;
    }

}

void VehicleNumberChangeReconnection()
{
    if(CarVehicleCheck())
    {
        LOG_Debug("Vehicle number change topic: %s ", MqttHandle::getInstance().topic_handle.tbox_ota_topic);
        TopicIinit();
        LOG_Debug("Vehicle number change topic: %s ", MqttHandle::getInstance().topic_handle.tbox_ota_topic);
        mqtt_disconnect();
        mqtt_cleanup();
        SetMqttStatus(MQTT_INIT);
    }
}


MQTT_STATUS GetMqttStatus()
{
    return mqttstatus;
}

void SetMqttStatus(MQTT_STATUS status)
{
    mqttstatus = status;
}


void * MQTTThread(void *arg)
{
    int rc;
    TopicIinit();
    ThreadInit();
    while(1)
    {
        int mqttControlEnable = GetMqttControlEnable();
        bool isMaster = IsMasterModule();
        int pmState = GetPmState();
        
        LOG_Debug("MQTTThread isMaster:%d, MqttControlEnable:%d, PmState:%d", isMaster, mqttControlEnable, pmState);
        bool shouldDisconnect = false;
        
        if(pmState == PM_STATE_SLEEP && mqttControlEnable == 2)
        {
            if(isMaster)
            {
                shouldDisconnect = true;
                LOG_Debug("PM_STATE_SLEEP && Gateway on slave module, master module should disconnect");
            }
        }
        else
        {
            if(!isMaster)
            {
                shouldDisconnect = true;
                LOG_Debug("Not (PM_STATE_SLEEP && Gateway on slave module), slave module should disconnect");
            }
        }
        
        if(shouldDisconnect)
        {
            if(MqttHandle::getInstance().client_handle.is_connected)
            {
                LOG_Debug("MQTT control disabled for this module, disconnecting...");
                mqtt_disconnect();
            }
            mqtt_cleanup();
            if(GetMqttStatus() != MQTT_INIT)
            {
                SetMqttStatus(MQTT_INIT);
            }
            sleep(1);
            continue;
        }

        VehicleNumberChangeReconnection();
        switch(GetMqttStatus())
        {
            case MQTT_INIT:
                PmSetSleepEvent(TSP_APP);
                rc = mqtt_init();
                if (rc != MQTTASYNC_SUCCESS) {
                    SetMqttStatus(MQTT_INIT);
                    break;
                }
                SetMqttStatus(MQTT_CONNECT);
                break;
            case MQTT_CONNECT:
                PmSetSleepEvent(TSP_APP);
                mqtt_connect();

                if(!WaitForCompletionTimeout(GetComp(),25))
                {
                    LOG_Error("connect time out");
                }                

                if (!MqttHandle::getInstance().client_handle.is_connected) {
                   
                    if(MqttHandle::getInstance().client_handle.s3_mqtt_client)
                    {
                        MQTTAsync_destroy(&MqttHandle::getInstance().client_handle.s3_mqtt_client);
                    }
                    SetMqttStatus(MQTT_INIT);
                    break;
                }

                while (!MqttHandle::getInstance().client_handle.is_connected);
                SetMqttStatus(MQTT_SUBSCRIBE);
                break;
            case MQTT_SUBSCRIBE:
                rc = mqtt_subscribe();
                if (rc != MQTTASYNC_SUCCESS) {
                    SetMqttStatus(MQTT_CONNECT);
                    break;
                }
                
                SetMqttStatus(MQTT_NONE);
                PmCleanSleepEvent(TSP_APP);
                break;
            case MQTT_UNSUBSCRIBE:
                rc = mqtt_unsubscribe();
                if (rc != MQTTASYNC_SUCCESS) {
                    SetMqttStatus(MQTT_UNSUBSCRIBE);
                }

                break;
            case MQTT_DISCONNECT:
                rc = mqtt_disconnect();
                if (rc != MQTTASYNC_SUCCESS) {
                    SetMqttStatus(MQTT_DISCONNECT);
                }
                break;
            default:
                break;
        }
        sleep(1);
    }

    // 断开连接并清理资源
    mqtt_disconnect();
    mqtt_cleanup();
    DeInit();
    
    return EXIT_SUCCESS;
}

