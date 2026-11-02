#include <time.h>
#include <glib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "sr_log.h"
#include <stdint.h>
#include "queue.h"
#include "mqtt.h"
#include "cycle_common.h"
#include "cycle_misc_json_interface.h"

static MPUPmStateChange_t PmState;
static unsigned char Kl15Status;
static int g_mqtt_control_enable = 0;

// RTC唤醒一次性上报标志位
static uint8_t g_onceRtcFlag = 0;   // RTC唤醒标志
static uint8_t g_oneMqttFlag = 0;   // 已发送标志
static time_t g_lastRtcWakeupTime = 0;  // 上次RTC唤醒时间
MCUCMDSTATUS mcu_cmd_status;

void CANIdCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    LOG_Debug("CANIdCallback start...");
    if((NULL == recvData))
    {
        LOG_Error("%s argv is invalid, dataLen = %d", __FUNCTION__, dataLen);
        printf("%s argv is invalid, dataLen = %d", __FUNCTION__, dataLen);
        return;
    }

    if(!MqttHandle::getInstance().client_handle.is_connected)
    {
        LOG_Error("mqtt No connection");
        return;
    }

    can_queue *data = (can_queue *)malloc(sizeof(can_queue));
    if(data == NULL)
    {
        LOG_Error("CAN Id Callback malloc can_queue fail");
        return ;
    }

    data->candata = (unsigned char *)malloc(dataLen);
    if(data->candata == NULL)
    {
        LOG_Error("CAN Id Callback malloc data.can fail");

        free(data);
        return ;
    }
    data->datalen = dataLen;
    memcpy(data->candata,recvData,dataLen);
    PushCanDataToQue(data);
   // LOG_Debug("%s, recv DataLen:%d, type:%08x, userdata:%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW, (uint32_t)userdata);
    //LOG_DebugHex(recvData, dataLen, "mcu_debug");
}

void CANIdDataFree(can_queue *data)
{

    if(!data)
    {
        return;
    }

    if(data->candata)
    {
        free(data->candata);
        data->candata = NULL;
    }
    free(data);
    data = NULL;
}



void PmStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    //MPUPmStateChange_t *pmStatus = NULL;
    static int pmstate = 0;
    mqtt_data mqttdata = {0};
    MPUPmStateChange_t PmState = {0};
    char buf[1024] = {0};
    int len = 0;

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

    //LOG_DebugHex(recvData, dataLen, "PmStatus");
    memmove(&PmState, recvData, dataLen);
    if(PmState.PmState != GetPmState())
    {
        SetPmState(PmState.PmState);
    }

    LOG_Debug("pm status pmstate:%d  PmState.PmState:%d OnceRtcFlag:%u OneMqttFlag:%u",pmstate,PmState.PmState,g_onceRtcFlag,g_oneMqttFlag);
    
    // RTC心跳唤醒时设置一次性标志位（时间窗口过滤，防止同一唤醒周期内重复触发）
    if (PmState.WakeupSrc == PM_WAKEUP_SOURCE_TIMER_HEARTBEAT) {
        time_t currentTime = time(NULL);
        // 只有当次唤醒源是TIMER_HEARTBEAT且比上次唤醒超过15秒才处理
        if (g_lastRtcWakeupTime == 0 || (currentTime - g_lastRtcWakeupTime) > 15) {
            g_onceRtcFlag = 1;
            g_oneMqttFlag = 0;
            g_lastRtcWakeupTime = currentTime;
            LOG_Debug("RTC wakeup detected, interval=%ld seconds, set OnceRtcFlag=1, OneMqttFlag=0",
                      (long)(currentTime - g_lastRtcWakeupTime));
        } else {
            LOG_Debug("RTC wakeup ignored, interval=%ld seconds < 15s",
                      (long)(currentTime - g_lastRtcWakeupTime));
        }
    }
    
    // 正常模式时清除标志位
    if (PmState.PmState == PM_STATE_NORMAL) {
        g_onceRtcFlag = 0;
        g_oneMqttFlag = 0;
        LOG_Debug("Normal mode, clear RTC flags");
    }
    
    if((pmstate != PmState.PmState) || (g_onceRtcFlag == 1 && g_oneMqttFlag == 0))
    {
        LOG_Debug("pm status pmstate:%d  PmState.PmState:%d WakeupSrc:%d OnceRtcFlag:%u OneMqttFlag:%u",pmstate,PmState.PmState,PmState.WakeupSrc,g_onceRtcFlag,g_oneMqttFlag);
        pmstate = PmState.PmState;
        if(!MqttHandle::getInstance().client_handle.is_connected)
        {
            LOG_Error("CycSendFunction is connected fail");
        }
        
        memset(buf,0,1024);
        len = MiscCycleData(buf, 1024);
        if(len < 0)
        {
            LOG_Error("Misc Cycle Data Json string len error");
        }
        memset(&mqttdata,0,sizeof(mqtt_data));
        mqttdata.payload  = buf;
        if(!mqttdata.payload)
        {
            LOG_Error("Misc Cycle Data Json string NULL");
        }

        mqttdata.topic = MqttHandle::getInstance().topic_handle.tbox_cyc_topic;
        mqttdata.payloadlen = strlen(mqttdata.payload);
        mqtt_publish(mqttdata,0);
        
        // RTC唤醒后标记已发送，确保只发一次
        if (g_onceRtcFlag == 1) {
            g_oneMqttFlag = 1;
            LOG_Debug("RTC wakeup MQTT sent once, set OneMqttFlag=1");
        }
    }

}

void HardLineSyncCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{

    Kl15Status = data[0];
    LOG_Debug(" Kl15Status = %d\n",Kl15Status);

}

int GetKl15Status()
{

    return Kl15Status;
}

void get_cmd_num(const uint8_t* data, const uint16_t len, void* userdata)
{
    LOG_DebugHex(data,len,"get_cmd_num is:");
    if((len > 0 ) && (NULL != data))
    {
        g_mqtt_control_enable = data[1];
        if(memcmp(&mcu_cmd_status, data, len) != 0)
        {
            memcpy(&mcu_cmd_status, data, len);
        }
    }
    else
    {
         LOG_Error("%s recv modem change gateway data leng is wrong.",__FUNCTION__);
    }

    return;
}

int GetMqttControlEnable(void)
{
    return g_mqtt_control_enable;
}

