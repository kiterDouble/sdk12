
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "cycle_evet_handle.h"
#include "err_code.h"
#include "sr_log.h"
//#include "mqtt_connect.h"
#include "cycle_misc_json_interface.h"
#include "common_functions.h"
#include "sr_sdk.h"
#include "sr_ipc.h"
#include "cycle_get_modem_info.h"
#include "mqtt_queue.h"

// #define  TBOX_VERSION_FILE      "/oemapp/upgrade/version.txt"
// #define  MQTT_CYCLE_PORT_TOPIC  "ZL02813/VehicleState/post"

static VehicleStatus g_vehicleStatus;
static VehicleStatusMCu g_vehicleStatusMcu;

VehicleCycleData g_vehicleCycleData;  //九识车身信息数据

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
    LOG_Debug("vehicleCycleData copy start");
    memcpy(vehicleCycleData, &g_vehicleCycleData, sizeof(VehicleCycleData));
    LOG_Debug("vehicleCycleData copy over");
    LOG_Debug("vehicleCycleData length = %zu", copyLen);
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
        // 新增数据
        case SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_SIMCARD_TO_MPU:
                         // 检查队列是否已初始化，防止回调在队列初始化前被调用
             if (!g_send_misc_queue_initialized) {
                 LOG_Error("RemoteMonitorMqttQueuePack: g_send_misc_queue not initialized yet, drop data");
                 break;
             }
             // 收到3110消息
             if (DataQueueEnqueue(&g_send_misc_queue, recvData, dataLen) != 0) {
                 LOG_Error("RemoteMonitorMqttQueuePack: DataQueueEnqueue failed, len=%d", dataLen);
             } else {
                 LOG_Debug("RemoteMonitorMqttQueuePack: DataQueueEnqueue success, len=%d", dataLen);
             }
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
    // 初始化发送队列（必须在订阅消息之前初始化，防止回调在队列初始化前被调用）
    if (DataQueueInit(&g_send_misc_queue) != 0) {
        LOG_Error("RemoteMonitorMqttQueuePack: DataQueueInit for g_send_misc_queue failed");
        return RET_ERR;
    }
    g_send_misc_queue_initialized = 1;
    LOG_Debug("RemoteMonitorMqttQueuePack: g_send_misc_queue initialized");

    //EVENT订阅，0x00020400
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_EVENT_DATA, RecvMcuEvetDataHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA));
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA, RecvMcuEvetDataHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA));
    //AccessMonitor结果订阅
    SubscriptionQueueMsg(SYS_QUE_TYPE_PING_NET,GetNetMonitorstatuscallback, NULL);
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CTROL_SIMCARD_INFO_TO_MPU, SimCardInfoToMpuCallback, NULL);
    // 新增3110
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_SIMCARD_TO_MPU, RecvMcuEvetDataHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_SIMCARD_TO_MPU));
    return RET_OK;
}


// void *SendPackageDataTaskEntry(void *pArg)
// {
//     LOG_Debug("SendPackageDataTaskEntry is Start");

//     char mqttBuff[1024] = {0};
//     gint mqttBuffLen = 0;
//     gint retValue = -1;
//     uint8_t carNum[8] = {0};
//     char tmpTopic[128] = {0};

    
//     //锁初始化
//     pthread_mutex_init(&g_vehicleCycleDataMutex, NULL);
//     while (1)
//     {
//         // if(FALSE == GetCarNum(carNum, 8))
//         // {
//         //     LOG_Error("%s ,GetCarNum error",__FUNCTION__);
//         //     sleep(1);
//         //     continue;
//         // }
    
//         // snprintf(tmpTopic, 128, "%s/VehicleState/post", carNum);

//         memset(mqttBuff, 0, sizeof(mqttBuff));
//         // 设置主题
//         // memcpy(g_topicCycleDataBuff, MQTT_CYCLE_PORT_TOPIC, sizeof(MQTT_CYCLE_PORT_TOPIC));
//         pthread_mutex_lock(&g_vehicleCycleDataMutex);
//         // 组包
//         mqttBuffLen = PackageCycleData(mqttBuff, sizeof(mqttBuff));
//         pthread_mutex_unlock(&g_vehicleCycleDataMutex);

//         LOG_Debug("mqttBuffLen is:%d", mqttBuffLen);

//         if (mqttBuffLen > 0)
//         {
//             //retValue = SendPackageData(tmpTopic, MQTT_QOS1, mqttBuff, mqttBuffLen);
//             if (retValue != RET_OK)
//             {
//                 LOG_Warn("SendPackageData failed, ret=%d", retValue);
//             }
//         }
//         else
//         {
//                LOG_Warn("PackageCycleData failed");
//         }
//         sleep(1);
//     }

//     return NULL;
// }