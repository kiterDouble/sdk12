#include <time.h>
#include "mqtt_thread.h"
#include "mqtt.h"
#include "queue.h"
#include "err_code.h"
#include "sr_log.h" 
#include "mqtt_interface.h"
#include "mqtt_data_handler.h"
#include "thread_interface.h"
#include "cjson_interface.h"
#include  "timer_interface.h"
#include "udp_server.h"
//处理MQTT 接收的数据 
void * ReceiveFunction(void *arg)
{
    int ret;
    mqtt_data* data = NULL;
    
    while(1)
    {
        DelayMS(10);
        ret = PopMqttDataFromQue(data);
        if(ret != 0)
        {
            //LOG_Debug("PopMqttDataFromQue empty\n");
            continue; 
        }
        LOG_Debug("data.topic = %s   data.payload  = %s ",data->topic,data->payload);
        printf("data.topic = %s   data.payload  = %s ",data->topic,data->payload);
        
        //LOG_Debug("data.topic = %s ",data->topic);
        //printf("data.topic = %s \n",data->topic);
        if(!strncmp(data->topic,MqttHandle::getInstance().topic_handle.tbox_ota_topic,strlen(data->topic)))
        {
            ret = GetServiceIdFromJson(data->payload);
            if( ret < 0)
            {
                LOG_Error("Get ServiceId Fail");

            }else if(ret == 0x501)
            {
                ret = OtaWakeHandle(data);
                if(ret < 0)
                {
                    LOG_Error("ota Wake Handle Fail");
                }
            }else if(ret == 0x502)
            {
                ret = OtaConfigHandle(data);
                if(ret < 0)
                {
                    LOG_Error("ota Handle Fail");
                }
            }else if(ret == 0x503)
            {
                ret = SyncOtaFieldHandle(data);
                if(ret < 0)
                {
                    LOG_Error("Sync Ota Field Handle Fail");
                }
            }else if(ret == 0x504)
            {
                ret = DomainNameSwitching(data);
                if(ret < 0)
                {
                    LOG_Error("Domain Name Switching Fail");
                }
            }else{

                LOG_Error("unknow ServiceId :%x",ret);
            }
           
        }else{

            LOG_Error("unknown topic ...\n");
        }
        
       
    }
    
    return NULL;
}


void * CycSendFunction(void *arg)
{
    
    mqtt_data data;
    int qos = 0;
    while (1)
    {
        if(GetPmState() == PM_STATE_SLEEP)
        {
            LOG_Debug("PM Sleep State");
            sleep(1);
            continue;
        }

        if(!MqttHandle::getInstance().client_handle.is_connected)
        {
            LOG_Error("CycSendFunction is connected fail");
            sleep(1);
            continue;
        }
        sleep(10);
        memset(&data,0,sizeof(mqtt_data));
        data.payload  = KeepAliveVehicleJsonInterface();
        if(!data.payload)
        {
            LOG_Error("KeepAliveVehicleJsonInterface string NULL");
    
        }
        data.topic = MqttHandle::getInstance().topic_handle.tbox_cyc_topic;
        data.payloadlen = strlen(data.payload);
        
        mqtt_publish(data,qos);
        if(data.payload)
        {
            free(data.payload);
        }


    }
    
    return NULL;
}


void ThreadInit()
{
    int ret;
    ret = CreateThread(&MqttHandle::getInstance().mqttreceive_threadId, ReceiveFunction, NULL);
    if(ret < 0)
    {
        LOG_Error("Create Receive Thread Fail \n");
    }

    ret = CreateThread(&MqttHandle::getInstance().mqttcycsend_threadId, CycSendFunction, NULL);
    if(ret < 0)
    {
        LOG_Error("Create CycSendFunction Thread Fail \n");
    }


    ret = CreateThread(&MqttHandle::getInstance().udpotasync_threadid,UdphandleServer, NULL);
    if (ret < 0)
    {
        LOG_Error("Create HTTP Thread Fail \n");
    }

}

void DeInit()
{
    pthread_join(MqttHandle::getInstance().mqttreceive_threadId, NULL);
    pthread_join(MqttHandle::getInstance().mqttcycsend_threadId, NULL);
    pthread_join(MqttHandle::getInstance().udpotasync_threadid, NULL);
    
}


