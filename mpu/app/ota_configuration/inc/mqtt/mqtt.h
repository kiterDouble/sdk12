

#ifndef _MQTT_H_
#define _MQTT_H_

//#if defined(__cplusplus)
//extern "C" {
//#endif



#include <iostream>
#include <string>
#include <cstring>
#include "MQTTAsync.h"


// MQTT Broker 配置 121.37.233.9  118.196.12.126:1883
//#define UAT_ADDRESS     "tls://ota-broker-uat.zelosapi.com:1883"  //测试 MQTT Broker 地址
#define UAT_ADDRESS     "ssl://ota-broker-uat.zelosapi.com:1883" 
#define PROD_ADDRESS     "ssl://ota-broker.zelosapi.com:1883"  //正式 MQTT Broker 地址
//#define ADDRESS     "192.168.225.163:8883"  // MQTT Broker 地址
//#define ADDRESS     "wlappemq-sits.tsp.sgmwcloud.com.cn:8883"  // MQTT Broker 地址
//#define ADDRESS     "ssl://wlappemq-sits.tsp.sgmwcloud.com.cn:8883"  // MQTT Broker 地址
#define MQTT_QOS    1
#define DEVICE_CERT     "/oemdata/cert/device.pem"
#define CERT_CHAIN        "/oemapp/cert/certificate_chain_client.pem" 

#define QOS         1                               // QoS 等级
#define TIMEOUT     10000L                          // 操作超时时间（毫秒）
#define TOPIC_LEN     64 
#define TSP_APP   "OtaApp"

#define     EMMCDEVICENODE                      "/dev/mmcblk0"
#define   COLLISIONCONFIGURATIONTOPIC  "vehicleNo/vehStatusEvent/cloud"
#define   CONFIGURATIONTOPIC  "vehicleNo/vehStatusEvent/cloud"
#define   PERIODICITYTOPIC  "vehicleNo/prdVehicleStatus/dev"

#define CAROTACONFIGMODE        (0x191046046)



enum MQTT_STATUS{
    MQTT_NONE = -1,
    MQTT_INIT = 0,
    MQTT_CONNECT,
    MQTT_SUBSCRIBE,
    MQTT_UNSUBSCRIBE,
    MQTT_DISCONNECT,

};

//tbox 代表上行  platform代表下行
typedef struct mqtt_topic{

    char tbox_device_topic[TOPIC_LEN]; 
    char tbox_will_topic[TOPIC_LEN]; 
    char tbox_ota_reply_topic[TOPIC_LEN]; 
    char tbox_ota_topic[TOPIC_LEN]; 
    char tbox_config_topic[TOPIC_LEN]; 
    char tbox_config_reply_topic[TOPIC_LEN]; 
    
    char tbox_ota_wake_topic[TOPIC_LEN]; 
    char tbox_ota_wake_reply_topic[TOPIC_LEN]; 
    char tbox_cyc_topic[TOPIC_LEN]; 
    char tbox_can_topic[TOPIC_LEN]; 
    char tbox_can_reply_topic[TOPIC_LEN];
    
}mqtt_topic;

typedef struct {
    
    int payloadlen;
    int topic_len;
    char *topic;
    char *payload;

}mqtt_data;

class mqtt_client
{
public:
    volatile bool is_connected = false;                         // 全局标志：连接状态
    int tsp_status;                                            //tsp平台连接状态 
    MQTTAsync_message pubmsg;
    MQTTAsync s3_mqtt_client;
    MQTTAsync_connectOptions conn_opts;
};

class MqttHandle
{
public:
    MqttHandle();
    
    MqttHandle(const MqttHandle&)=delete;
    ~MqttHandle();
    MqttHandle& operator = (const MqttHandle&)=delete;
    static MqttHandle& getInstance(){
        static MqttHandle instance;
        return instance;
    }
public:
    mqtt_client client_handle;
    mqtt_topic  topic_handle;
    pthread_t mqttreceive_threadId;
    pthread_t mqttcycsend_threadId;  
    pthread_t httpcanlog_threadid;
    pthread_t udpotasync_threadid;
};





int mqtt_init();
int mqtt_connect();
int mqtt_publish(mqtt_data data,int qos);
int mqtt_subscribe();
int mqtt_unsubscribe();
int mqtt_disconnect();
void mqtt_cleanup();
int TopicIinit();
MQTT_STATUS GetMqttStatus();
void SetMqttStatus(MQTT_STATUS status);

void * MQTTThread(void *arg);
void freemqtt(mqtt_data *data);
void MqttDomainSwitchCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);
//#if defined(__cplusplus)
//     }
//#endif

#endif
