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
#include "mqtt_can_upload.h"
#include "udp_client.h"
#include "udp_server.h"


int RemoteConfigurationHandle(mqtt_data *data)
{
    RemoteConfigResponse  configResponse  = {0};
    McuMesg_t msg ;
    VehicleMessage_t *RemoteConfigurationData = NULL;
    uint8_t mcudata[8] = {0};
    mqtt_data sneddata = {0};
    char *seqid = "xxxxxx";
    int ret = 0;

    RemoteConfigurationData =  ParseVehicleRemoteConfigurationMessage(data->payload);
    freemqtt(data);
    PrintRemoteConfigurationVehicleMessage(RemoteConfigurationData);
    if(!RemoteConfigurationData)
    {
        LOG_Error("RemoteConfigurationData null");
        configResponse.result = 1;
        configResponse.error_code = 7;
        configResponse.configStatus.escFunction = 1;
        configResponse.configStatus.externalDischargeFunc = 1;
        configResponse.configStatus.mileageSync = 1;
        sneddata.payload =  ResponseRemoteConfigurationCjaon(configResponse,seqid);
        sneddata.payloadlen = strlen(sneddata.payload);
        sneddata.topic = MqttHandle::getInstance().topic_handle.tbox_config_reply_topic;
        ret = mqtt_publish(sneddata,1);
        if(ret < 0)
        {
            LOG_Error("mqtt publish fail");
        }
        if(sneddata.payload)
        {
            free(sneddata.payload);
            sneddata.payload = NULL;
        }
            return -1;
    }

    if(RemoteConfigurationData->payload.esc_function)
    {
        if(strcmp(RemoteConfigurationData->payload.esc_function,"开启") == 0)
        {
            LOG_Debug("strcmp esc start");

            configResponse.configStatus.escFunction = 0;
            configResponse.result = 0;
            configResponse.error_code |= (0<< 0);

            msg.type = (SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_ESC;
            msg.payloadLen = 1;
            mcudata[0] = 0x01;
            msg.payload = (uint8_t *)mcudata;
            ret = McuSrvSendMesg(&msg);
            if(ret < 0)
            {
                configResponse.configStatus.escFunction = 1;
                configResponse.result = 1;
                configResponse.error_code |= (1<< 0);
                LOG_Error("Mcu Srv Send ESC Msg Open Fail");
            }
           
        }else if(strcmp(RemoteConfigurationData->payload.esc_function,"关闭") == 0)
        {
            LOG_Debug("strcmp esc close");
            configResponse.configStatus.escFunction = 0;
            configResponse.result = 0;
            configResponse.error_code |= (0<< 0);
            msg.type = (SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_ESC;
            msg.payloadLen = 1;
            mcudata[0] = 0x02;
            msg.payload = (uint8_t *)mcudata;
            ret = McuSrvSendMesg(&msg);
            if(ret < 0)
            {
                configResponse.configStatus.escFunction = 1;
                configResponse.result = 1;
                configResponse.error_code |= (1<< 0);
                LOG_Error("Mcu Srv Send ESC Msg Close Fail");
            }
            
        }else{

            LOG_Error("RemoteConfigurationData->payload.esc_function = %s",RemoteConfigurationData->payload.esc_function);
        }

    }else{
        configResponse.configStatus.escFunction = -1;
        LOG_Debug("RemoteConfigurationData payload esc_function null");
    }

    memset(mcudata,0,sizeof(mcudata));
    if(RemoteConfigurationData->payload.mileage_sync)
    {
        double value1 = atof(RemoteConfigurationData->payload.mileage_sync);
        unsigned long long mileage = (value1 * 100);
        memcpy(mcudata,&mileage,8);
        LOG_Debug("RemoteConfigurationData->payload.mileage_sync:%s  value:%.2f mileage:%x",RemoteConfigurationData->payload.mileage_sync,value1,mileage);
        configResponse.configStatus.mileageSync = 0;
        configResponse.result = 0;
        configResponse.error_code |= (0<< 4);
        msg.type = (SYSQueMsgType_e)SYS_QUE_TYPE_MPU_SYNC_MILEAGE_SYNC;
        msg.payloadLen = 8;
        msg.payload = (uint8_t *)mcudata;
        ret = McuSrvSendMesg(&msg);
        if(ret < 0)
        {
            configResponse.configStatus.mileageSync = 1;
            configResponse.result = 1;
            configResponse.error_code |= (1<< 4);
            LOG_Error("Mcu Srv Send mileageSync Fail");
        }

    }else{
        configResponse.configStatus.mileageSync = -1;
        LOG_Debug("RemoteConfigurationData payload mileageSync null");
    }

    // if(RemoteConfigurationData->payload.low_speed_prompt_volume != -1)
    // {
    //     configResponse.configStatus.lowSpeedPromptVolume = 0;
    //     if(configResponse.result == 0)
    //     {
    //         configResponse.result = 0;
    //     }
        
    //     configResponse.error_code |= (0<< 2);
    //     ret = SetVolumeLevel(RemoteConfigurationData->payload.low_speed_prompt_volume/10,1); 
    //     if(ret < 0)
    //     {
    //         LOG_Error("shout set fail");
    //         configResponse.result = 1;
    //         configResponse.error_code |= (1<< 2);
    //         configResponse.configStatus.lowSpeedPromptVolume = 1;
    //     }
    //     ret = SetVolumeLevel(RemoteConfigurationData->payload.low_speed_prompt_volume/10,2); 
    //     if(ret < 0)
    //     {
    //         LOG_Error("tts set fail");
    //         configResponse.result = 1;
    //         configResponse.error_code |= (1<< 2);
    //         configResponse.configStatus.lowSpeedPromptVolume = 1;
    //     }
        
    // }else{
    //     LOG_Debug("RemoteConfigurationData payload low_speed_prompt_volume null");
    //     configResponse.configStatus.lowSpeedPromptVolume = -1;
    // }





    //音量配置
    //RemoteConfigurationData->payload.low_speed_prompt_volume;
    //configResponse.configStatus.low_speed_prompt_volume =1;
    memset(mcudata,0,sizeof(mcudata));
    if(RemoteConfigurationData->payload.external_discharge_func)
    {

        if(strcmp(RemoteConfigurationData->payload.external_discharge_func,"开启") == 0)
        {
            LOG_Debug("strcmp external start");
            configResponse.configStatus.externalDischargeFunc = 0;
            configResponse.error_code |= (0<< 1);

            if(configResponse.result == 0)
            {
                configResponse.result = 0;
            }
            msg.type = (SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_EXTERNAL_DISCHARGE;
            msg.payloadLen = 1;
            mcudata[0] = 0x01;
            msg.payload = (uint8_t *)mcudata;
            ret = McuSrvSendMesg(&msg);
            if(ret < 0)
            {
                configResponse.result = 1;
                configResponse.configStatus.externalDischargeFunc = 1;
                configResponse.error_code |= (1<< 1);
                LOG_Error("Mcu Srv Send Discharged Msg Open Fail");
            }
            
            
        }else if(strcmp(RemoteConfigurationData->payload.external_discharge_func,"关闭") == 0)
        {
            LOG_Debug("strcmp external close");
            configResponse.configStatus.externalDischargeFunc = 0;
            configResponse.error_code |= (0<< 1);
            if(configResponse.result == 0)
            {
                configResponse.result = 0;
            }
            msg.type = (SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_EXTERNAL_DISCHARGE;
            msg.payloadLen = 1;
            mcudata[0] = 0x02;
            msg.payload = (uint8_t *)mcudata;
            ret = McuSrvSendMesg(&msg);
            if(ret < 0)
            {
                configResponse.result = 1;
                configResponse.configStatus.externalDischargeFunc = 1;
                configResponse.error_code |= (1<< 1);
                LOG_Error("Mcu Srv Send Discharged Msg Close Fail");
            }

            
        }else{

            LOG_Error("RemoteConfigurationData->payload.external_discharge_func = %s",RemoteConfigurationData->payload.external_discharge_func);
        }
    }else{
        configResponse.configStatus.externalDischargeFunc = -1;
        LOG_Debug("RemoteConfigurationData payload external_discharge_func null");
    }
    
    sneddata.payload =  ResponseRemoteConfigurationCjaon(configResponse,RemoteConfigurationData->seqid);
    FreeRemoteConfigurationVehicleMessage(RemoteConfigurationData);
    sneddata.payloadlen = strlen(sneddata.payload);
    sneddata.topic = MqttHandle::getInstance().topic_handle.tbox_config_reply_topic;
    ret = mqtt_publish(sneddata,1);
    if(ret < 0)
    {
        LOG_Error("mqtt publish fail");
    }
    if(sneddata.payload)
    {
        free(sneddata.payload);
        sneddata.payload = NULL;
    }

    return 0;
}



// 处理接触器开关状态
int ContactSwitchHandle(mqtt_data *data) 
{
    McuMesg_t msg;
    uint8_t mcudata[8] = {0};
    int ret = 0;
    mqtt_data sneddata = {0};
    char *seqid = "xxxxxx";
    ContactConfigResultStatusResponse  ResultStatusResponse = {0};
    mcudata[0] = 0x22;
    mcudata[1] = 0x22;
    mcudata[2] = 0x22;
    mcudata[3] = 0x22;
    VehicleContactMessage_t* ContactMessage = ParseVehicleContactMessage(data->payload);
    freemqtt(data);
    if (!ContactMessage) 
    {
        LOG_Error("Parse Vehicle Contact Message FAIL");
        //需要修改接口
        ResultStatusResponse.result = 1;
        ResultStatusResponse.error_code = 0xf;
        ResultStatusResponse.contact_ctatus.firstLevelContactSwitch = 1;
        ResultStatusResponse.contact_ctatus.firstLevelContactSwitchRelease = 1;
        ResultStatusResponse.contact_ctatus.secondLevelContactSwitchRelease = 1;
        ResultStatusResponse.contact_ctatus.thirdLevelContactSwitchRelease = 1;
        sneddata.payload =  ResponseVehicleContactCjaon(ResultStatusResponse,seqid);
        sneddata.payloadlen = strlen(sneddata.payload);
        sneddata.topic = MqttHandle::getInstance().topic_handle.tbox_config_reply_topic;
        ret = mqtt_publish(sneddata,1);
        if(ret < 0)
        {
            LOG_Error("mqtt publish fail");
        }

        if(sneddata.payload)
        {
            free(sneddata.payload);
            sneddata.payload = NULL;
        }
        return -1;
    }
    //SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CONTACT
    LOG_Debug("Processing contact switch states for vehicle %s:\n", ContactMessage->vehicle_no);
    // 处理一级接触器开关
    if (ContactMessage->payload.first_level_contact_switch) {
        ResultStatusResponse.contact_ctatus.firstLevelContactSwitch = 0;
        if (strcmp(ContactMessage->payload.first_level_contact_switch, "禁用") == 0) 
        {
            mcudata[1] = 0x01;
            LOG_Debug("  - First level contact switch: DISABLED\n");
            // 执行禁用一级接触器的操作
        } else if (strcmp(ContactMessage->payload.first_level_contact_switch, "无效") == 0) 
        {
            mcudata[1] = 0x00;
            LOG_Debug("  - First level contact switch: ENABLED\n");
            // 执行启用一级接触器的操作
        }else{
            
            LOG_Error("ERROR: first_level_contact_switch : %s",ContactMessage->payload.first_level_contact_switch);
            ResultStatusResponse.contact_ctatus.firstLevelContactSwitch = -1;
        }
    }else{
        LOG_Debug("  - First level contact switch: NULL\n");
        ResultStatusResponse.contact_ctatus.firstLevelContactSwitch = -1;
        mcudata[1] = 0x22;
    }

    // 处理接触器释放状态
    if(ContactMessage->payload.first_level_contact_switch_release)
    {
        ResultStatusResponse.contact_ctatus.firstLevelContactSwitchRelease = 0;
        if (strcmp(ContactMessage->payload.first_level_contact_switch_release, "解除") == 0) 
        {
            LOG_Debug("  - First level contact switch release: RELEASED\n");
            mcudata[0] = 0x01;
            // 执行一级接触器解除操作
        }else if (strcmp(ContactMessage->payload.first_level_contact_switch_release, "无效") == 0){
            LOG_Debug("  - First level contact switch release: invalid\n");
            mcudata[0] = 0x00;

        }else{
            LOG_Error("ERROR: first_level_contact_switch_release: %s",ContactMessage->payload.first_level_contact_switch_release);
            ResultStatusResponse.contact_ctatus.firstLevelContactSwitchRelease = -1;
        }
    }else{
        LOG_Debug("  - First level contact switch release: null\n");
        ResultStatusResponse.contact_ctatus.firstLevelContactSwitchRelease = -1;
        mcudata[0] = 0x22;
    }

    if(ContactMessage->payload.second_level_contact_switch_release)
    {
        ResultStatusResponse.contact_ctatus.secondLevelContactSwitchRelease = 0;
        if (strcmp(ContactMessage->payload.second_level_contact_switch_release, "解除") == 0) 
        {
            LOG_Debug("  - Second level contact switch release: RELEASED");
            mcudata[2] = 0x01;
            // 执行二级接触器解除操作
        }else if (strcmp(ContactMessage->payload.second_level_contact_switch_release, "无效") == 0)
        {
            LOG_Debug("  - Second level contact switch release: invalid");
            mcudata[2] = 0x00;
        }else{
            LOG_Error("ERROR: second_level_contact_switch_release: %s",ContactMessage->payload.second_level_contact_switch_release);
            ResultStatusResponse.contact_ctatus.secondLevelContactSwitchRelease = -1;
        }
    }else{
        LOG_Debug("  - Second level contact switch release: null");
        mcudata[2] = 0x22;
        ResultStatusResponse.contact_ctatus.secondLevelContactSwitchRelease = -1;
    }
    if(ContactMessage->payload.third_level_contact_switch_release)
    {
        ResultStatusResponse.contact_ctatus.thirdLevelContactSwitchRelease = 0;
        if (strcmp(ContactMessage->payload.third_level_contact_switch_release, "解除") == 0) 
        {
            LOG_Debug("  - Third level contact switch release: RELEASED\n");
            mcudata[3] = 0x01;
            // 执行三级接触器解除操作
        }else if (strcmp(ContactMessage->payload.third_level_contact_switch_release, "无效") == 0)
        {
            LOG_Debug("  - Third level contact switch release: invalid\n");
            mcudata[3] = 0x00;
        }else{
            LOG_Error("ERROR: third_level_contact_switch_release: %s",ContactMessage->payload.third_level_contact_switch_release);
            ResultStatusResponse.contact_ctatus.thirdLevelContactSwitchRelease = -1;
        }
    }else{
        LOG_Debug("  - Third level contact switch release: null\n");
        ResultStatusResponse.contact_ctatus.thirdLevelContactSwitchRelease = -1;
        mcudata[3] = 0x22;
    }

    msg.type = (SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CONTACT;
    msg.payloadLen = 4;
    msg.payload = (uint8_t *)mcudata;
    ret = McuSrvSendMesg(&msg);
    if(ret < 0)
    {
        LOG_Error("Mcu Srv Send Mesg Close Fail");
        ResultStatusResponse.result = 1;
        if(ResultStatusResponse.contact_ctatus.firstLevelContactSwitch == 0)
        {
            ResultStatusResponse.contact_ctatus.firstLevelContactSwitch = 1;
            ResultStatusResponse.error_code |=  (1<< 0);
        }

        if(ResultStatusResponse.contact_ctatus.firstLevelContactSwitchRelease == 0)
        {
            ResultStatusResponse.contact_ctatus.firstLevelContactSwitchRelease = 1;
            ResultStatusResponse.error_code |=  (1<< 1);
        }

        if(ResultStatusResponse.contact_ctatus.secondLevelContactSwitchRelease == 0)
        {
            ResultStatusResponse.contact_ctatus.secondLevelContactSwitchRelease = 1;
            ResultStatusResponse.error_code |=  (1<< 2);
        }

        if(ResultStatusResponse.contact_ctatus.thirdLevelContactSwitchRelease == 0)
        {
            ResultStatusResponse.contact_ctatus.thirdLevelContactSwitchRelease = 1;
            ResultStatusResponse.error_code |=  (1<< 3);
        }

    }

    
    sneddata.payload =  ResponseVehicleContactCjaon(ResultStatusResponse,ContactMessage->seqid);
    FreeVehicleContactMessage(ContactMessage);
    sneddata.payloadlen = strlen(sneddata.payload);
    sneddata.topic = MqttHandle::getInstance().topic_handle.tbox_config_reply_topic;
    ret = mqtt_publish(sneddata,1);
    if(ret < 0)
    {
        LOG_Error("mqtt publish fail");
    }

    if(sneddata.payload)
    {
        free(sneddata.payload);
        sneddata.payload = NULL;
    }
    return 0;
}


int CanLogHandle(mqtt_data *data)
{
    int ret = 0;
    LogData logData = {0};
    char buf[64] = {0};
    LOG_Debug("CanLogHandle statrt \n");
    if(data == NULL)
    {
        LOG_Error("mqtt data null");
        return -1;
    }
    ret = CanLogParseJson(data->payload,&logData);
    freemqtt(data);
    if(ret < 0)
    {
        LOG_Error("Can Log Parse Json Fail");
        return -1;
    }
    LOG_Debug("Can Log Handle logData.logStart = %lld  logData.logEnd = %lld\n",logData.logStart,logData.logEnd);
    ret = SetUpLoadCanFileInfo(logData.logStart/1000, logData.logEnd/1000, logData.batchNumber);
    if(ret < 0 )
    {
        LOG_Error("Set UpLoad Can File Info Fail");
        FreeCanLogParseJson(&logData);
        return -1;
    }

    FreeCanLogParseJson(&logData);
    return 0;
}


int DotaWakeHandle(mqtt_data *data)
{
    char *seqid = NULL;
    mqtt_data senddata = {0};
    int ret = 0;
    char result = 0; 
    LOG_Debug("DotaWakeHandle start...");
    
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
    senddata.topic = MqttHandle::getInstance().topic_handle.tbox_dota_reply_topic;
    ret = mqtt_publish(senddata,1);
    if(ret < 0)
    {
        LOG_Error("mqtt publish fail");
        return -1;
    }

    if(senddata.payload)
    {
        free(senddata.payload);
        senddata.payload = NULL;
    }
    
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

int DomainNameSwitching(mqtt_data *data)
{

    LOG_Debug("DomainNameSwitching statrt ");
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
        params.url = "http://127.0.0.1:20006/DOTAChangeDomain";
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

    senddata.topic = MqttHandle::getInstance().topic_handle.tbox_dota_reply_topic;
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
                ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_DOTA_SEND, (void *)domiandata, 1);
                if(ret < 0)
                {
                    LOG_Error("Send  Mqtt Uat Domain Switch Fail");
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
                            ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_DOTA_SEND, (void *)domiandata, 1);
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
                    ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_DOTA_SEND, (void *)domiandata, 1);
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
                ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_DOTA_SEND, (void *)domiandata, 1);
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
                            ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_DOTA_SEND, (void *)domiandata, 1);
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
                    ret = SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_DOMAIN_DOTA_SEND, (void *)domiandata, 1);
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

