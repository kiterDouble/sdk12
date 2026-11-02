
#ifndef __CYCLE_MISC_JSON_INTETFACER_H_
#define __CYCLE_MISC_JSON_INTETFACER_H_

#include <stdint.h>
#include "mqtt_queue.h"

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

int MiscCycleData(char *vehicleData, int dataLen);

char SetSendMqttFlag(char sendFlag);

char SetsendErrorFlag(char sendErrorFlag);

// 发送数据队列 extern 声明（定义在 cycle_misc_json_interface.cpp 中）
extern DataQueue_t g_send_misc_queue;
extern volatile int g_send_misc_queue_initialized;

// 队列处理线程函数声明
void *MiscQueueProcessThread(void *arg);

#ifdef __cplusplus
}
#endif

#endif
/**********************************end*****************************************/
