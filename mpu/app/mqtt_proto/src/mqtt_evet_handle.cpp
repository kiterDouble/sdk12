
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
#include "mqtt_queue.h"

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
pthread_mutex_t g_gnssInfoMutex;



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
         case SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_SIMCARD_TO_MPU:
             // 将数据入队列，供 SendPackageDataTaskEntry 线程使用
             // 检查队列是否已初始化，防止回调在队列初始化前被调用
             if (!g_send_queue_initialized) {
                 LOG_Error("MqttQueuePack: g_send_queue not initialized yet, drop data");
                 break;
             }
             // 收到3110消息
             if (DataQueueEnqueue(&g_send_queue, recvData, dataLen) != 0) {
                 LOG_Error("MqttQueuePack: DataQueueEnqueue failed, len=%d", dataLen);
             } else {
                 LOG_Debug("MqttQueuePack: DataQueueEnqueue success, len=%d", dataLen);
             }
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
    // 订阅311001
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_SIMCARD_TO_MPU, RecvMcuEvetDataHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_SIMCARD_TO_MPU));

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
    pthread_mutex_init(&g_gnssInfoMutex, NULL);
    
    // 初始化发送队列
    if (DataQueueInit(&g_send_queue) != 0) {
        LOG_Error("MqttQueuePack: %s, DataQueueInit failed", __FUNCTION__);
        return NULL;
    }
    g_send_queue_initialized = 1;  // 设置初始化标志
    LOG_Debug("MqttQueuePack: DataQueueInit success");
    
    while (1)
    {
        if(FALSE == GetCarNum(carNum, 8))
        {
            LOG_Error("%s ,GetCarNum error",__FUNCTION__);
            sleep(1);
            continue;
        }
    
        snprintf(tmpTopic, 128, "%s/VehicleState/post", carNum);

        // 阻塞等待队列中的数据
        DataBuffer_t queue_data;
        if (DataQueueDequeueLastest(&g_send_queue, &queue_data) != 0) {
            LOG_Debug("MqttQueuePack: %s, DataQueueDequeue exit", __FUNCTION__);
            break;  // 退出标志被设置
        }
        LOG_Debug("MqttQueuePack: DataQueueDequeueLastest success, len=%zu", queue_data.len);
        
        // 将队列中的数据解析成多个独立的全局变量
        pthread_mutex_lock(&g_vehicleCycleDataMutex);
        
        int offset = 0;
        
        // 解析 g_vehicleCycleData
        if (queue_data.len >= offset + sizeof(VehicleCycleData)) {
            memcpy(&g_vehicleCycleData, queue_data.data + offset, sizeof(VehicleCycleData));
            offset += sizeof(VehicleCycleData);
            LOG_Debug("MqttQueuePack: memcpy VehicleCycleData success");
        } else {
            LOG_Error("MqttQueuePack: %s, queue data len too small for VehicleCycleData: %zu < %zu", __FUNCTION__, queue_data.len, offset + sizeof(VehicleCycleData));
        }

        // 解析 g_simCardInfo_main_unify
        if (queue_data.len >= offset + sizeof(SimCardInfoToMpu_t)) {
            memcpy(&g_simCardInfo_main_unify, queue_data.data + offset, sizeof(SimCardInfoToMpu_t));
            offset += sizeof(SimCardInfoToMpu_t);
            LOG_Debug("MqttQueuePack: memcpy SimCardInfoToMpu_t main success");
        } else {
            LOG_Error("MqttQueuePack: %s, queue data len too small for SimCardInfoToMpu_t main: %zu < %zu", __FUNCTION__, queue_data.len, offset + sizeof(SimCardInfoToMpu_t));
        }

        // 解析 g_simCardInfo_sub_unify
        if (queue_data.len >= offset + sizeof(SimCardInfoToMpu_t)) {
            memcpy(&g_simCardInfo_sub_unify, queue_data.data + offset, sizeof(SimCardInfoToMpu_t));
            offset += sizeof(SimCardInfoToMpu_t);
            LOG_Debug("MqttQueuePack: memcpy SimCardInfoToMpu_t sub success");
        } else {
            LOG_Error("MqttQueuePack: %s, queue data len too small for SimCardInfoToMpu_t sub: %zu < %zu", __FUNCTION__, queue_data.len, offset + sizeof(SimCardInfoToMpu_t));
        }
        
        // 解析 g_gnssInfo_unify
        if (queue_data.len >= offset + sizeof(GnssLocation_t)) {
            memcpy(&g_gnssInfo_unify, queue_data.data + offset, sizeof(GnssLocation_t));
            offset += sizeof(GnssLocation_t);
            LOG_Debug("MqttQueuePack: memcpy GnssLocation_t success");
        } else {
            LOG_Error("MqttQueuePack: %s, queue data len too small for GnssLocation_t: %zu < %zu", __FUNCTION__, queue_data.len, offset + sizeof(GnssLocation_t));
        }
        
        // 解析 g_otaState_unify
        if (queue_data.len >= offset + sizeof(uint8_t)) {
            memcpy(&g_otaState_unify, queue_data.data + offset, sizeof(uint8_t));
            offset += sizeof(uint8_t);
            LOG_Debug("MqttQueuePack: memcpy OTA state success");
        } else {
            LOG_Error("MqttQueuePack: %s, queue data len too small for OTA state: %zu < %zu", __FUNCTION__, queue_data.len, offset + sizeof(uint8_t));
        }
        
        // 解析 g_otaProgress_unify
        if (queue_data.len >= offset + sizeof(uint8_t)) {
            memcpy(&g_otaProgress_unify, queue_data.data + offset, sizeof(uint8_t));
            offset += sizeof(uint8_t);
            LOG_Debug("MqttQueuePack: memcpy OTA progress success");
        } else {
            LOG_Error("MqttQueuePack: %s, queue data len too small for OTA progress: %zu < %zu", __FUNCTION__, queue_data.len, offset + sizeof(uint8_t));
        }
        
        LOG_Debug("MqttQueuePack: parsed total offset=%d, queue_data.len=%zu", offset, queue_data.len);

        
        pthread_mutex_unlock(&g_vehicleCycleDataMutex);
        free(queue_data.data);  // 释放队列分配的数据缓冲区
        
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
            if(GetPmState() == PM_STATE_SLEEP && GetOneMqttFlag() == 0 )////休眠/定时唤醒 模式下未上报则最多只发送一条有效数据
            {
                LOG_Debug("%s, sleep_mode && onMqttFlag == 0", __FUNCTION__);
                SetOrClearRtcSleepEvent(1);//设置休眠事件
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

    // 清理队列资源
    LOG_Debug("MqttQueuePack: DataQueueDestroy start");
    g_send_queue_initialized = 0;
    DataQueueDestroy(&g_send_queue);

    return NULL;
}
