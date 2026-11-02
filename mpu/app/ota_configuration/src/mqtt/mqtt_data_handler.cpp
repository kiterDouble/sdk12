#include <time.h>
#include <glib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
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


// ==================== 主副模组判断工具函数 ====================
/**
 * @brief 检查是否为主模组
 * @return 1: 主模组, 0: 副模组
 */
static inline int IsMasterModule(void)
{
    return (0 == access("/dev/mmcblk0", F_OK));
}

/**
 * @brief 检查是否为副模组
 * @return 1: 副模组, 0: 主模组
 */
static inline int IsSlaveModule(void)
{
    return (0 != access("/dev/mmcblk0", F_OK));
}


// ==================== HTTP回调函数 ====================
static int otahttpscallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    int ret = 0;
    HTTPSClientParams *params = (HTTPSClientParams *)userdata;
    LOG_Debug("http ota  str = %s\n",(char *)ptr);
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
    ret = GetDomainHttpReValue((char *)ptr);
    if(ret < 0 )
    {
        LOG_Error("GetDomainHttpReValue Fail");
        return -1;
    }

    params->response = ret;
    return 0;
}


// ==================== 副模组专用CURL HTTP POST函数 ====================
static size_t OtaConfigCurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalSize = size * nmemb;
    int *response = (int *)userp;

    if (contents && totalSize > 0)
    {
        int err = GetHttpErrValue((char *)contents);
        if (err >= 0)
        {
            *response = err;
        }
    }
    return totalSize;
}

static int OtaConfigHttpPost(const char *url, const char *post_data, int *response)
{
    CURL *curl = NULL;
    CURLcode res;
    struct curl_slist *headers = NULL;
    int ret = -1;
    int retry_count = 0;
    const int max_retries = 10;
    const int retry_delay_sec = 2;

    curl = curl_easy_init();
    if (!curl)
    {
        LOG_Error("[OtaConfigHttpPost] Failed to initialize CURL.");
        return -1;
    }

    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OtaConfigCurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    for (retry_count = 0; retry_count < max_retries; retry_count++)
    {
        *response = -1;
        res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            long response_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code == 200 && *response >= 0)
            {
                LOG_Debug("[OtaConfigHttpPost] Request success on attempt %d, err=%d", retry_count + 1, *response);
                ret = 0;
                break;
            }
            else
            {
                LOG_Warn("[OtaConfigHttpPost] HTTP %ld, err=%d on attempt %d", response_code, *response, retry_count + 1);
            }
        }
        else
        {
            LOG_Error("[OtaConfigHttpPost] curl_easy_perform failed on attempt %d: %s", retry_count + 1, curl_easy_strerror(res));
        }

        if (retry_count < max_retries - 1)
        {
            LOG_Debug("[OtaConfigHttpPost] Retry %d/%d, waiting %ds...", retry_count + 1, max_retries, retry_delay_sec);
            sleep(retry_delay_sec);
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (ret != 0)
    {
        LOG_Error("[OtaConfigHttpPost] All %d attempts failed.", max_retries);
    }

    return ret;
}


// ==================== OTA配置处理函数（整合主副模组逻辑） ====================
int OtaConfigHandle(mqtt_data *data) 
{
    int ret = 0;
    char* str_json = NULL;
    char *seqid = NULL;
    unsigned char result = 0;
    mqtt_data senddata = {0};
    HTTPSClientParams params = {0};
    int http_response = -1;


    str_json = OtaExtractPlayload(data->payload,&seqid);
    freemqtt(data);
    LOG_Debug("str_json = %s ",str_json);
    LOG_Debug("seqid = %s ",seqid);
    if(str_json)
    {
        if (IsMasterModule())
        {
            // 主模组：使用HttpPostClient发送到本地UDS服务
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
            http_response = params.response;
        }
        else
        {
            // 副模组：使用CURL发送到主模组IP
            ret = OtaConfigHttpPost("http://192.168.1.201:20003/setotafield", str_json, &http_response);
            if(ret < 0)
            {
                LOG_Error("OtaConfigHttpPost Fail");
            }
        }
        free(str_json);
        str_json = NULL;
    }

    result = (http_response >= 0) ? http_response : 0;
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

    return 0;
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
        // 主副模组使用相同的URL（本地服务）
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
                    if(IsSlaveModule())
                    {
                        // 副模组（网关在副模组）：收到切换指令只保存数据库，返回成功，不重连平台
                        LOG_Debug("Slave module: uat config saved, no reconnect");
                        domiandata[0] = 0;
                        ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_FOTA_SEND, (void *)domiandata, 1);
                        if(ret < 0)
                        {
                            LOG_Error("Send  Mqtt Uat Domain Switch Fail");
                        }else{
                            LOG_Debug("Send  Mqtt Uat Domain Switch ok");
                        }
                        return;
                    }
                    mqtt_disconnect();
                    mqtt_cleanup();
                    SetMqttStatus(MQTT_INIT);
                    for(int j = 0; j< 60;j++)
                    {
                        if(MqttHandle::getInstance().client_handle.is_connected)
                        {
                            LOG_Debug("Mqtt Uat Domain Switch is connected %d",recvData[0]);
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
                        if(!access("/dev/mmcblk0", F_OK))
	                    {
                            mqtt_disconnect();
                            mqtt_cleanup();
                        }
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
                    if(IsSlaveModule())
                    {
                        // 副模组（网关在副模组）：收到切换指令只保存数据库，返回成功，不重连平台
                        LOG_Debug("Slave module: prod config saved, no reconnect");
                        domiandata[0] = 0;
                        ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_FOTA_SEND, (void *)domiandata, 1);
                        if(ret < 0)
                        {
                            LOG_Error("Send  Mqtt Prod Domain Switch Fail");
                        }else{
                            LOG_Debug("Send  Mqtt Prod Domain Switch ok");
                        }
                        return;
                    }
                    mqtt_disconnect();
                    mqtt_cleanup();
                    SetMqttStatus(MQTT_INIT);
                    for(int j = 0; j< 58;j++)
                    {
                        if(MqttHandle::getInstance().client_handle.is_connected)
                        {
                            
                            LOG_Debug("Mqtt Domain Switch is connected %d",recvData[0]);
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
                        if(!access("/dev/mmcblk0", F_OK))
	                    {
                            mqtt_disconnect();
                            mqtt_cleanup();
                            SetMqttStatus(MQTT_INIT);
                        }
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
