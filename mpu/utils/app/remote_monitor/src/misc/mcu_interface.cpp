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

    LOG_Debug("pm status pmstate:%d  PmState.PmState:%d",pmstate,PmState.PmState);
    if((pmstate != PmState.PmState) || (PmState.WakeupSrc == PM_WAKEUP_SOURCE_TIMER_HEARTBEAT))
    {
        LOG_Debug("pm status pmstate:%d  PmState.PmState:%d WakeupSrc:%d",pmstate,PmState.PmState,PmState.WakeupSrc);
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