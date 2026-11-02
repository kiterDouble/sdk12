
#ifndef __MQTT_PROTO_PACKAGE_H_
#define __MQTT_PROTO_PACKAGE_H_

#include <stdint.h>

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
#ifdef __cplusplus
}
#endif

#endif
/**********************************end*****************************************/
