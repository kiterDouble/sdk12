
#ifndef _SIRUNQUEUE_H_
#define _SIRUNQUEUE_H_


#include <iostream>
#include <queue>
#include <string>
#include <cstring>
#include "mqtt.h"
#include "mqtt_data_handler.h"

#include "mcu_interface.h"




int32_t PopMqttDataFromQue(mqtt_data *&data);
int32_t PushMqttDataToQue(mqtt_data *data);
int32_t PushMsgDataToList(int data);
int32_t PushCanDataToQue(can_queue *data);

int32_t PopCanDataFromQue(can_queue *&data);

#endif
