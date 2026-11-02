#ifndef _MQTT_DATA_HANDLER_H_
#define _MQTT_DATA_HANDLER_H_

#include "mqtt.h"

int RemoteConfigurationHandle(mqtt_data* data);
int ContactSwitchHandle(mqtt_data* data);
int OtaConfigHandle(mqtt_data *data);
int OtaWakeHandle(mqtt_data *data);
int CanLogHandle(mqtt_data *data);
int SyncOtaFieldHandle(mqtt_data *data);
void MqttDomainSwitchRevCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);
int DomainNameSwitching(mqtt_data *data);

int DotaWakeHandle(mqtt_data *data);
#endif
