#ifndef __MQTT_CAN_UPLOAD_H__
#define __MQTT_CAN_UPLOAD_H__
#include <stdint.h>

int32_t SetUpLoadCanFileInfo(uint64_t startTime, uint64_t endTime, const char* batchNumber);

void *UploadCanFileThread(void *pArg);

#endif