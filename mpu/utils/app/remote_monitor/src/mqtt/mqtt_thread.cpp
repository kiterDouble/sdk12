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
#include "mqtt_can_upload.h"
#include "udp_server.h"
#include "cycle_misc_json_interface.h"
#include "cycle_get_modem_info.h"
#include "cycle_common.h"
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
        //printf("data.payload  = %s \n",data->payload);
        //LOG_Debug("data.topic = %s ",data->topic);
        //printf("data.topic = %s \n",data->topic);
        if(!strncmp(data->topic,MqttHandle::getInstance().topic_handle.tbox_dota_topic,strlen(data->topic)))
        {
            ret = GetServiceIdFromJson(data->payload);
            if( ret < 0)
            {
                LOG_Error("Get ServiceId Fail");

            }else if(ret == 0x501)
            {
                ret = DotaWakeHandle(data);
                if(ret < 0 )
                {
                    LOG_Error("Dota Wake Handle Error");
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
            
        }
        else if(strncmp(data->topic,MqttHandle::getInstance().topic_handle.tbox_config_topic,strlen(data->topic)) == 0)
        {
            ret = GetServiceIdFromJson(data->payload);
            if( ret < 0)
            {
                LOG_Error("Get ServiceId Fail");
            }else if(ret == 3)
            {
                ret = RemoteConfigurationHandle(data);
                if(ret < 0)
                {
                    LOG_Error("Handle Remote Configuration Fail");
                }
            }else if(ret == 4)
            {
                ret = ContactSwitchHandle(data);
                if(ret < 0)
                {
                    LOG_Error("Contact Switch Handle Fail");
                }
            }

        }else if(!strncmp(data->topic,MqttHandle::getInstance().topic_handle.tbox_can_topic,strlen(data->topic)))
        {
            ret = CanLogHandle(data);
            if(ret < 0 )
            {
                LOG_Error("Can Log Handle Error");
            }
        }
        else{

            LOG_Error("unknown topic ...\n");
        }
        
       
    }
    
    return NULL;
}



void * CycMiscSendFunction(void *arg)
{
    
    mqtt_data data;
    int qos = 0;
    int len = 0;
    char buf[1024] = {0};
    while (1)
    {
        if(GetPmState() == PM_STATE_SLEEP)
        {
            LOG_Debug("PM Sleep State");
            sleep(1);
            continue;
        }
        sleep(1);
        if(MqttHandle::getInstance().client_handle.is_connected )
        {
            memset(buf,0,1024);
            len = MiscCycleData(buf, 1024);
            if(len < 0)
            {
                LOG_Error("Misc Cycle Data Json string len error");
            }
            memset(&data,0,sizeof(mqtt_data));
            data.payload  = buf;
            if(!data.payload)
            {
                LOG_Error("Misc Cycle Data Json string NULL");
            }

            data.topic = MqttHandle::getInstance().topic_handle.tbox_cyc_topic;
            data.payloadlen = strlen(data.payload);
            mqtt_publish(data,qos);

        }

    }
    
    return NULL;
}


void * CycCanSendFunction(void *arg)
{
    
    mqtt_data data;
    int qos = 0;
    int len = 0;
    char buf[1024] = {0};
    while (1)
    {
        if(GetPmState() == PM_STATE_SLEEP)
        {
            LOG_Debug("PM Sleep State");
            sleep(1);
            continue;
        }
        
        sleep(1);
        if(MqttHandle::getInstance().client_handle.is_connected )
        {
            memset(&data,0,sizeof(mqtt_data));
            data.payload  = CanVehicleJsonInterface();
            if(!data.payload)
            {
                LOG_Error("CAN Json string NULL");
                //sleep(1);
                continue;
            }
            data.topic = MqttHandle::getInstance().topic_handle.tbox_cyc_topic;
            data.payloadlen = strlen(data.payload);
            mqtt_publish(data,qos);
            if(data.payload)
            {
                free(data.payload);
                data.payload = NULL;
            }

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

    ret = CreateThread(&MqttHandle::getInstance().mqttmodeminfo_threadId, GetModemInfoThread, NULL);
    if(ret < 0)
    {
        LOG_Error("Create GetModem Info Thread  Fail \n");
    }

    ret = CreateThread(&MqttHandle::getInstance().mqttcycmiscsend_threadId, CycMiscSendFunction, NULL);
    if(ret < 0)
    {
        LOG_Error("Create Cyc Misc Send Function Thread Fail \n");
    }

    ret = CreateThread(&MqttHandle::getInstance().mqttcyccansend_threadId, CycCanSendFunction, NULL);
    if(ret < 0)
    {
        LOG_Error("Create Cyc Can Send Function Fail \n");
    }

    ret = CreateThread(&MqttHandle::getInstance().httpcanlog_threadid,UploadCanFileThread, NULL);
    if (ret < 0)
    {
        LOG_Error("Create Upload Can File Thread Fail \n");
    }

}

void DeInit()
{
    pthread_join(MqttHandle::getInstance().mqttreceive_threadId, NULL);
    pthread_join(MqttHandle::getInstance().mqttcycmiscsend_threadId, NULL);
    pthread_join(MqttHandle::getInstance().mqttcyccansend_threadId, NULL);
    pthread_join(MqttHandle::getInstance().httpcanlog_threadid, NULL);
    pthread_join(MqttHandle::getInstance().mqttmodeminfo_threadId, NULL);
    
    
}


