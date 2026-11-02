#include <time.h>
#include <glib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "mcu_api.h"
#include "sr_log.h"
#include "queue.h"
#include "modem_api.h"
#include "cjson_interface.h"
#include "timer_interface.h"
#include "string_hex_interface.h"
#include "common_functions.h"
#include "mqtt_data_handler.h"
#include "cjson_interface.h"
#include "modem_function.h"
#include "system_interface.h"
#include "sr_sdk.h"
#include "pm_server_api.h"
#include "mqtt_data_handler.h"
#include "http.h"
#include "udp_client.h"
#include "udp_server.h"


static int otahttpscallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    int ret = 0;
    HTTPSClientParams *params = (HTTPSClientParams *)userdata;
    LOG_Debug("http ota  str = %s\n",(char *)ptr);
    //printf("http ota  str = %s\n",(char *)ptr);
    ret = GetHttpErrValue((char *)ptr);
    if(ret < 0 )
    {
        return -1;
    }

    params->response = ret;
    return 0;
}


static int domainhttpscallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    int ret = 0;
    HTTPSClientParams *params = (HTTPSClientParams *)userdata;
    LOG_Debug("http ota  str = %s\n",(char *)ptr);
    //printf("http ota  str = %s\n",(char *)ptr);
    ret = GetDomainHttpReValue((char *)ptr);
    if(ret < 0 )
    {
        LOG_Error("GetDomainHttpReValue Fail");
        return -1;
    }

    params->response = ret;
    return 0;
}

int OtaConfigHandle(mqtt_data *data) 
{
    int ret = 0;
    char* str_json = NULL;
    char *seqid = NULL;
    unsigned char result = 0;
    mqtt_data senddata = {0};
    HTTPSClientParams params = {0};


    str_json = OtaExtractPlayload(data->payload,&seqid);
    freemqtt(data);
    LOG_Debug("str_json = %s ",str_json);
    LOG_Debug("seqid = %s ",seqid);
    if(str_json)
    {
        //调用UDS接口
        params.header1 = "Content-Type: application/json";
        params.post_data = str_json;
        params.url = "http://127.0.0.1:20005/setotafield";
        params.httpscallback = otahttpscallback;
        params.response = -1;
        ret = HttpPostClient(&params);
        if(ret < 0)
        {
            LOG_Error("Http Post Client Fail");
        }
        free(str_json);
        str_json = NULL;
    }

    result = params.response;
    senddata.payload =  OtaCreateJsonInterface(seqid,result);
    if(seqid)
    {
        free(seqid);
        seqid = NULL;
    }
    
    senddata.payloadlen = strlen(senddata.payload);
    senddata.topic = MqttHandle::getInstance().topic_handle.tbox_ota_reply_topic;
    ret = mqtt_publish(senddata,1);
    if(ret < 0)
    {
        LOG_Error("mqtt publish fail");
    }
    if(senddata.payload)
    {
        free(senddata.payload);
        senddata.payload = NULL;
    }

}


int OtaWakeHandle(mqtt_data *data)
{
    char *seqid = NULL;
    mqtt_data senddata = {0};
    int ret = 0;
    char result = 0; 
    LOG_Debug("OtaWakeHandle start...");
    
    PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
    ret = PmWakeupSetDevices("mcu",1);
    
    // if(GetKl15Status() == KL15_IS_OFF)
    // {
    //     //给整车上电
    //     otakl15status = KL15_IS_OFF;
    // }

    // HTTPSClientParams params = {0};


    // //调用UDS接口
    // //params.header1 = "Content-Type: application/json";
    // params.post_data = "start";
    // params.url = "http://127.0.0.1:20003/tspmessage?dev=tbox";
    // params.httpscallback = Otawakehttpscallback;
    // params.response = -5;
    // ret = HttpPostClient(&params);
    // if(ret < 0)
    // {
    //     LOG_Error("Http Post Client Fail");
    // }

    //杨昊 HTTP通信
    //result = params.response;
    if(ret < 0 )
    {
        LOG_Error("Pm Wakeup Set Devices fail ret:%d",ret);
        result = 1;
    }else{

        result = 0;
    }
    seqid =  OtaGetSeqId(data->payload);
    freemqtt(data);
    senddata.payload = OtaWakenVehicleCreateJSONInterface(seqid,result);
    if(seqid)
    {
        free(seqid);
        seqid = NULL;
    }
    
    senddata.payloadlen = strlen(senddata.payload);
    senddata.topic = MqttHandle::getInstance().topic_handle.tbox_ota_reply_topic;
    ret = mqtt_publish(senddata,1);
    if(ret < 0)
    {
        LOG_Error("mqtt publish fail");
    }

    if(senddata.payload)
    {
        free(senddata.payload);
        senddata.payload = NULL;
    }
    return 0;
}

// 回调函数，用于处理响应数据
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) 
{
    size_t totalSize = size * nmemb;
    LOG_Debug("Write Callback start ...");
    if(contents && totalSize > 0)
    {
        LOG_Debug("contents: %s",(char *)contents);
    }else{

        LOG_Error("WriteCallback Fail");
    }
    return totalSize;
}


int SyncOtaFieldHandle(mqtt_data *data)
{

    LOG_Debug("Sync Ota Field Handle statrt \n");
    unsigned int result  = 0;
    int ret = -1;
    char *str = NULL;
    if(data == NULL)
    {
        LOG_Error("mqtt data null");
        return -1;
    }

    ret =  SyncOtaFieldVehicleInfoParseResult(data->payload, &result);
    if(ret < 0 )
    {
        LOG_Error("Sync Ota Field Vehicle Info Parse Result Fail");
    }


    if(result == 0)
    {
        str = "200";
        
    }else{

        str = "500";
    }
    
    ret =  UdpServerSend(str);
    if(ret < 0 )
    {
        LOG_Error("Send Post Request Error");
    }

    return 0;
}



int DomainNameSwitching(mqtt_data *data)
{

    LOG_Debug("DomainNameSwitching statrt \n");
    int result  = 0;
    int ret = -1;
    DomainVehicleData domaindata = {0};
    HTTPSClientParams params = {0};
    char domiandata[5] = {0};
    mqtt_data senddata = {0};
    if(data == NULL)
    {
        LOG_Error("mqtt data null");
        return -1;
    }

    ret = DomainParseVehicleData(data->payload, &domaindata);
    if(ret < 0 )
    {
        LOG_Error("Domain Parse Vehicle Data Fail");
        FreeDomainVehicleData(&domaindata);
        return -1;
    }

    if(domaindata.PayloadTarget)
    {
        LOG_Debug("domaindata.PayloadTarget:%s",domaindata.PayloadTarget);
        params.header1 = "Content-Type: application/json";
        params.post_data = domaindata.PayloadTarget;
        params.url = "http://127.0.0.1:20005/FOTAChangeDomain";
        params.httpscallback = domainhttpscallback;
        params.response = -5;
        ret = HttpPostClient(&params);
        if(ret < 0)
        {
            LOG_Error("Http Post Client Fail");
        }
        result = params.response;
    }else{
        LOG_Error("domaindata PayloadTarget NULL");
    }
    
    LOG_Debug("result :%d SeqId:%s",result,domaindata.SeqId);
    senddata.payload  = DomainCreateJsonInterface(domaindata.SeqId,result);
    FreeDomainVehicleData(&domaindata);
    if(!senddata.payload)
    {
        LOG_Error("CAN Json string NULL");
    }
    senddata.topic = MqttHandle::getInstance().topic_handle.tbox_ota_reply_topic;
    senddata.payloadlen = strlen(senddata.payload);
    mqtt_publish(senddata,1);
    if(senddata.payload)
    {
        free(senddata.payload);
        senddata.payload = NULL;
    }

    return 0;
}



void MqttDomainSwitchRevCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    mqtt_data data = {0};
    char domiandata[128] = {0};
    int ret = 0;
    char oldmode[128] = {0};
    LOG_Debug("MqttDomainSwitchRevCallback start .....");
    for(int i=0;i<3;i++)
    {
        ret = GetCarotaConfigMode(oldmode);
        if(ret < 0)
        {
            LOG_Error("Get Config Mode Fail");

        }else{
            break;
        }
    }
    
    
    if(recvData && dataLen > 0)
    {
        if(recvData[0] == 1)
        {
            if(strncmp(oldmode,"uat",strlen("uat")) == 0 )
            {
                // ret = CreateFotaHttpJson(1, "uat",domiandata,128);
                // if(ret< 0)
                // {
                //     LOG_Error("CreateFotaHttpJson Fail");
                //     return;
                // }
                domiandata[0] = 0;
                ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_FOTA_SEND, (void *)domiandata, 1);
                if(ret < 0)
                {
                    LOG_Error("Send  Mqtt Uat Domain Switch Fail");
                    return;
                }else{
                    LOG_Debug("Send  Mqtt Uat Domain Switch ok");
                    return ;
                }
            }

            for(int i=0;i<3;i++)
            {
                if(SetCarotaConfigMode("uat", strlen("uat")))
                {
                    LOG_Debug("Set Config Uat Mode Ok");
                    mqtt_disconnect();
                    mqtt_cleanup();
                    
                    SetMqttStatus(MQTT_INIT);
                    for(int j = 0; j< 60;j++)
                    {
                        if(MqttHandle::getInstance().client_handle.is_connected)
                        {
                            LOG_Debug("Mqtt Uat Domain Switch is connected %d",recvData[0]);
                            // ret = CreateFotaHttpJson(1, "uat",domiandata,128);
                            // if(ret< 0)
                            // {
                            //     LOG_Error("CreateFotaHttpJson Fail");
                            //     return;
                            // }
                            domiandata[0] = 0;
                            ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_FOTA_SEND, (void *)domiandata, 1);
                            if(ret < 0)
                            {
                                LOG_Error("Send  Mqtt Uat Domain Switch Fail");
                            }else{
                                LOG_Debug("Send  Mqtt Uat Domain Switch ok");
                                return ;
                            }
                            
                        }else{
                            LOG_Error("Mqtt Uat Domain Switch is connected fail %d j:%d",recvData[0],j);
                        }
                        sleep(1);
                    }

                    // ret = CreateFotaHttpJson(0, "uat",domiandata,128);
                    // if(ret< 0)
                    // {
                    //     LOG_Error("CreateFotaHttpJson Fail");
                    //     return;
                    // }
                    domiandata[0] = 1;
                    ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_FOTA_SEND, (void *)domiandata, 1);
                    if(ret < 0)
                    {
                        LOG_Error("Send  Mqtt Uat Domain Switch Fail");

                    }else{
                        LOG_Debug("Send  Mqtt Uat Domain Switch ok");

                    }

                    if(SetCarotaConfigMode(oldmode,(strlen(oldmode))))
                    {
                        LOG_Debug("Set Config %s Mode Ok",oldmode);
                        mqtt_disconnect();
                        mqtt_cleanup();
                        
                        SetMqttStatus(MQTT_INIT);
                    }else{
                        LOG_Error("Set Config %s Mode Fail",oldmode);
                    }
                    return;
                }else{
                    LOG_Error("Set Uat Mode fail");
                }
            }

        }else if(recvData[0] == 2)
        {
            if(strncmp(oldmode,"prod",strlen("prod")) == 0 )
            {
                // ret = CreateFotaHttpJson(1, "prod",domiandata,128);
                // if(ret< 0)
                // {
                //     LOG_Error("CreateFotaHttpJson Fail");
                //     return;
                // }
                domiandata[0] = 0;
                ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_FOTA_SEND, (void *)domiandata, 1);
                if(ret < 0)
                {
                    LOG_Error("Send  Mqtt prod Domain Switch Fail");
                }else{
                    LOG_Debug("Send  Mqtt prod Domain Switch ok");
                    return ;
                }
            }

            for(int i=0;i<3;i++)
            {
                if(SetCarotaConfigMode("prod",strlen("prod")))
                {
                    LOG_Debug("Set Config Prod Mode Ok");
                    
                    mqtt_disconnect();
                    mqtt_cleanup();
                    SetMqttStatus(MQTT_INIT);
                    for(int j = 0; j< 58;j++)
                    {
                        if(MqttHandle::getInstance().client_handle.is_connected)
                        {
                            
                            LOG_Debug("Mqtt Domain Switch is connected %d",recvData[0]);
                            // ret = CreateFotaHttpJson(1, "prod",domiandata,128);
                            // if(ret< 0)
                            // {
                            //     LOG_Error("CreateFotaHttpJson Fail");
                            //     return;
                            // }
                            domiandata[0] = 0;
                            ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_FOTA_SEND, (void *)domiandata, 1);
                            if(ret < 0)
                            {
                                LOG_Error("Send  Mqtt Prod Domain Switch Fail");
                                return;
                            }else{
                                LOG_Debug("Send  Mqtt Prod Domain Switch ok");
                                return ;
                            }
                            
                        }else{
                            LOG_Error("Mqtt Prod Domain Switch is connected fail %d j:%d",recvData[0],j);
                        }
                        sleep(1);
                    }

                    // ret = CreateFotaHttpJson(0, "prod",domiandata,128);
                    // if(ret< 0)
                    // {
                    //     LOG_Error("CreateFotaHttpJson Fail");
                    //     return;
                    // }
                    domiandata[0] = 1;
                    ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_FOTA_SEND, (void *)domiandata, strlen(domiandata));
                    if(ret < 0)
                    {
                        LOG_Error("Send  Mqtt Prod Domain Switch Fail");
                        
                    }else{
                        LOG_Debug("Send  Mqtt Prod Domain Switch ok");
                    }

                    if(SetCarotaConfigMode(oldmode, strlen(oldmode)))
                    {
                        LOG_Debug("Set Config %s Mode Ok",oldmode);
                        mqtt_disconnect();
                        mqtt_cleanup();
                        
                        SetMqttStatus(MQTT_INIT);
                    }else{
                        LOG_Error("et Config %s Mode Fail",oldmode);
                    }
                    return;

                }else{
                    LOG_Error("Set Config Prod Mode fail");
                }
            }
        }
    }else{
        LOG_Error("MqttCallback recvData null/dataLen 0");
    }

}
