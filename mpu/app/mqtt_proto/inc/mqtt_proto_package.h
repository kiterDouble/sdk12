
#ifndef __MQTT_PROTO_PACKAGE_H_
#define __MQTT_PROTO_PACKAGE_H_

#include <stdint.h>
#include "gnss_ipc_msg.h"
#include "modem_api.h"
#include "mqtt_queue.h"
#include "mqtt_evet_handle.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OP_UNKNOWN = 0,
    OP_CMCC,   // 中国移动
    OP_CUCC,   // 中国联通
    OP_CTCC    // 中国电信
} OperatorType;


int PackageVehicleStatus(char *vehicleData, int dataLen);

OperatorType GetOperatorFromIccid(const uint8_t *iccid);

void GetNetMonitorstatuscallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

int PackageMcuVehicleStatus(char *vehicleData, int dataLen);

//int PackageMcuVehicleParkStatus(char *vehicleData, int dataLen);

int PackageCycleData(char *vehicleData, int dataLen);

char SetSendMqttFlag(char sendFlag);

char SetsendErrorFlag(char sendErrorFlag);

// 获取 OTA 状态和进度
uint8_t GetOtaState(void);
uint8_t GetUpProgress(void);

// 全局变量 extern 声明（定义在 mqtt_proto_package.cpp 中）
extern GnssLocation_t g_gnssInfo_unify;
extern uint8_t g_otaState_unify;
extern uint8_t g_otaProgress_unify;
extern SimCardInfoToMpu_t g_simCardInfo_main_unify;
extern SimCardInfoToMpu_t g_simCardInfo_sub_unify;
extern DataQueue_t g_send_queue;
extern volatile int g_send_queue_initialized;

#ifdef __cplusplus
}
#endif

#endif
/**********************************end*****************************************/
