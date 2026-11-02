
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "mqtt_evet_handle.h"
#include "err_code.h"
#include "sr_log.h"
#include "mqtt_connect.h"
#include "mqtt_proto_package.h"

#define  TBOX_VERSION_FILE      "/oemapp/upgrade/version.txt"
#define  MQTT_CYCLE_PORT_TOPIC  "ZL02813/VehicleState/post"

static VehicleStatus g_vehicleStatus;
static VehicleStatusMCu g_vehicleStatusMcu;

static ResetRsonCycleData g_resetRsonCycleData = {0};
static VehicleCycleData g_vehicleCycleData = {
    .VehicleSOC = 0xff,
    .ChargeStatus = 0xff,
    .VehicleStatus = 0xff,  
    .VCU_VehODO_km = {0xff, 0xff, 0xff, 0xff}, 
    .VehicleMeterSOC = 0xff,
};  //九识车身信息数据
static char g_topicBuff[128] = {0};
static char g_topicParkBuff[128] = {0};
static char g_topicCycleDataBuff[128] = {0};

pthread_mutex_t g_vehicleCycleDataMutex;



int GetVehicleCycleData(VehicleCycleData *vehicleCycleData)
{
    if(NULL == vehicleCycleData)
    {
        return RET_ERR;
    }
    size_t copyLen = sizeof(VehicleCycleData); // 获取长度
    memcpy(vehicleCycleData, &g_vehicleCycleData, sizeof(VehicleCycleData));
    LOG_Debug("vehicleCycleData length = %zu", copyLen);
    return RET_OK;
}

int GetRsonCycleData(ResetRsonCycleData *rsonCycleData)
{
    if(NULL == rsonCycleData)
    {
        return RET_ERR;
    }
    size_t copyLen = sizeof(ResetRsonCycleData); // 获取长度
    LOG_Debug("ResetRsonCycleData copy start");
    memcpy(rsonCycleData, &g_resetRsonCycleData, sizeof(ResetRsonCycleData));
    LOG_Debug("rsonCycleData copy over");
    LOG_Debug("rsonCycleData length = %zu", copyLen);
    return RET_OK;
}
void RecvMcuEvetDataHandleCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    if ((NULL == recvData) && (0 == dataLen) && (NULL == userdata))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    
    LOG_DebugHex(recvData, dataLen, "MCU Data");

    uint32_t aidMid = (uint32_t)((long long)userdata & 0x00ffffff);
    LOG_Debug("RecvMcuEvetDataHandleCallback aidMid:%08x", (unsigned int)aidMid);

    switch (aidMid)
    {
        case SYS_QUE_TYPE_MCU_VEHICLE_EVENT_DATA:
            // 暂不处理事件类数据
            break;

        case SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA:
            if (dataLen > sizeof(VehicleCycleData))
            {
                LOG_Error("dataLen:%d > sizeof(VehicleCycleData)", dataLen);
                break;
            }
            memset(&g_vehicleCycleData, 0xFF, sizeof(VehicleCycleData));
            memcpy(&g_vehicleCycleData, recvData, dataLen);
            break;
         case SYS_QUE_TYPE_MCU_RESET_RSON_DATA:
             if (dataLen >= sizeof(ResetRsonCycleData))
            {
                LOG_Debug("dataLen:%d >= sizeof(ResetRsonCycleData)", dataLen);
            }else{
                LOG_Error("dataLen:%d < sizeof(ResetRsonCycleData)", dataLen);
                break;
            }
            memset(&g_resetRsonCycleData, 0, sizeof(ResetRsonCycleData));
            memcpy(&g_resetRsonCycleData, recvData, sizeof(ResetRsonCycleData));
            
            LOG_Debug("%s resetRson M:S %u:%u", __FUNCTION__,g_resetRsonCycleData.Main_Reset_Reason,g_resetRsonCycleData.Slave_Reset_Reason);
            //LOG_DebugHex(recvData,dataLen, "ResetRson");
            break;
        default:
            break;
    }
}


int GetVehicleStatus(VehicleStatus *vehicleData)
{
    if(NULL == vehicleData)
    {
        return RET_ERR;
    }

    memcpy(vehicleData, (char *)&g_vehicleStatus, sizeof(VehicleStatus));

    return RET_OK;
}

int GetMcuVehicleStatus(VehicleStatusMCu *mcuVehicleData)
{
    if(NULL == mcuVehicleData)
    {
        return RET_ERR;
    }

    memcpy(mcuVehicleData, (char *)&g_vehicleStatusMcu, sizeof(VehicleStatusMCu));

    return RET_OK;
}

int32_t InitSignalIpcCallback()
{
    //EVENT订阅，0x00020400
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_EVENT_DATA, RecvMcuEvetDataHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA));
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA, RecvMcuEvetDataHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA));
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_RESET_RSON_DATA, RecvMcuEvetDataHandleCallback, (void *)(SYS_QUE_TYPE_MCU_RESET_RSON_DATA));
    
    //AccessMonitor结果订阅
    SubscriptionQueueMsg(SYS_QUE_TYPE_PING_NET,GetNetMonitorstatuscallback, NULL);
    return RET_OK;
}

//重试最多三次 以发送成功
static int SendPackageWithRetry(const gchar *tmpTopic,gpointer mqttBuff, guint mqttBuffLen)
{
    int retValue = -1;
    retValue = SendPackageData(tmpTopic, MQTT_QOS1, mqttBuff, mqttBuffLen);
    for(int i = 0;retValue != RET_OK && i < 3;i++){
        LOG_Warn("SendPackageData failed, ret=%d retry:%d", retValue,i);
        retValue = SendPackageData(tmpTopic, MQTT_QOS1, mqttBuff, mqttBuffLen);
    }
    if (retValue != RET_OK)
    {
        LOG_Warn("SendPackageData failed end, ret=%d", retValue);
    }
    return retValue;
}

void *SendPackageDataTaskEntry(void *pArg)
{
    LOG_Debug("SendPackageDataTaskEntry is Start");

    char mqttBuff[1024] = {0};
    gint mqttBuffLen = 0;
    gint retValue = -1;
    uint8_t carNum[8] = {0};
    char tmpTopic[128] = {0};

    
    //锁初始化
    pthread_mutex_init(&g_vehicleCycleDataMutex, NULL);
    while (1)
    {
        if(FALSE == GetCarNum(carNum, 8))
        {
            LOG_Error("%s ,GetCarNum error",__FUNCTION__);
            sleep(1);
            continue;
        }
    
        snprintf(tmpTopic, 128, "%s/VehicleState/post", carNum);

        memset(mqttBuff, 0, sizeof(mqttBuff));
        // 设置主题
        // memcpy(g_topicCycleDataBuff, MQTT_CYCLE_PORT_TOPIC, sizeof(MQTT_CYCLE_PORT_TOPIC));
        pthread_mutex_lock(&g_vehicleCycleDataMutex);
        // 组包
        mqttBuffLen = PackageCycleData(mqttBuff, sizeof(mqttBuff));
        pthread_mutex_unlock(&g_vehicleCycleDataMutex);
        PmSetSleepEvent("mqtt_clear_rtc");
        if(GetPmState() != PM_STATE_SLEEP){//正常模式下清除RTC唤醒标志位
            ClearOnceRtcflag();
            SetOneMqttFlag(0);
        }
        PmCleanSleepEvent("mqtt_clear_rtc");
        LOG_Debug("mqttBuffLen is:%d PmState:%s OnceRtcflag:%u OneMqttFlag:%u", mqttBuffLen,GetPmState()== PM_STATE_SLEEP?"SLEEPD":"NORMAL",GetOnceRtcflag(),GetOneMqttFlag());

        if (mqttBuffLen > 0)
        {
            if(GetOnceRtcflag() == 1 && GetOneMqttFlag() == 0 )////休眠/定时唤醒 模式下未上报则最多只发送一条有效数据
            {
                LOG_Debug("%s, once rtc flag is set,sleep_mode:%d", __FUNCTION__,GetPmState());
                retValue = SendPackageWithRetry(tmpTopic,mqttBuff,mqttBuffLen);
                SetOneMqttFlag(1);
                SetOrClearRtcSleepEvent(0);//清除休眠事件
            }
            else if(GetPmState() == PM_STATE_SLEEP) //休眠/唤醒后不再继续上报
            {
                LOG_Debug("%s, PM_STATE_SLEEP OneMqttFlag:%u, do not send data", __FUNCTION__,GetOneMqttFlag());
            }else //正常或其他状态正常上报MQTT数据 并清除标志位
            {
                LOG_Debug("%s,MQTT NORMAL send OnceRtcflag:%u OneMqttFlag:%u", __FUNCTION__,GetOnceRtcflag(),GetOneMqttFlag());
                retValue = SendPackageWithRetry(tmpTopic,mqttBuff,mqttBuffLen);//SendPackageData(tmpTopic, MQTT_QOS1, mqttBuff, mqttBuffLen);
                if (retValue != RET_OK)
                {
                    LOG_Warn("SendPackageData failed, ret=%d", retValue);
                }
                SetOneMqttFlag(0);
            }    
        }
        else
        {
               LOG_Warn("PackageCycleData failed");
               SetOrClearRtcSleepEvent(0);//清除休眠事件
        }
        uint8_t wakeupSrc = PmGetWakeupSrc();
        LOG_Debug("%s, PmState:%d wakeupSrc:%u", __FUNCTION__,GetPmState(),wakeupSrc);
        sleep(1);
    }

    return NULL;
}