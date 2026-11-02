#include <time.h>
#include <glib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "sr_log.h"
#include <stdint.h>
#include "queue.h"
#include "mqtt.h"
#include "cjson_interface.h"

static int g_PmState = PM_STATE_NORMAL;
static unsigned char Kl15Status;
void HandlePmSleepCmd(const uint8_t *data, const uint16_t len, void *userdata)
{
    static int pmstate = 0;
    mqtt_data mqttdata;
    MPUPmStateChange_t PmState = {0};
    //LOG_DebugHex(data,len,"HandlePmSleepCmd");
    if(data ==NULL)
    {
        LOG_Error("data NULL \n");
        return;
    }
    memset(&PmState, 0x00, sizeof(MPUPmStateChange_t));
    if (sizeof(MPUPmStateChange_t) == len)
    {
        memmove(&PmState, data, len);
    }
    g_PmState = PmState.PmState;
    LOG_Debug("pm status pmstate:%d  PmState.PmState:%d",pmstate,PmState.PmState);

    if((pmstate != PmState.PmState) || (PmState.WakeupSrc == PM_WAKEUP_SOURCE_TIMER_HEARTBEAT))
    {
        LOG_Debug("pm status pmstate:%d  PmState.PmState:%d PmState.WakeupSrc:%d",pmstate,PmState.PmState,PmState.WakeupSrc);
        pmstate = PmState.PmState;
        if(!MqttHandle::getInstance().client_handle.is_connected)
        {
            LOG_Error("CycSendFunction is connected fail");
        }
        
        memset(&mqttdata,0,sizeof(mqtt_data));
        mqttdata.payload  = KeepAliveVehicleJsonInterface();
        if(!mqttdata.payload)
        {
            LOG_Error("KeepAliveVehicleJsonInterface string NULL");
    
        }
        mqttdata.topic = MqttHandle::getInstance().topic_handle.tbox_cyc_topic;
        mqttdata.payloadlen = strlen(mqttdata.payload);
        
        mqtt_publish(mqttdata,0);
        if(mqttdata.payload)
        {
            free(mqttdata.payload);
        }
    }

   
}


int GetPmState()
{
    LOG_Debug("PmState = %d",g_PmState);
    return g_PmState;
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