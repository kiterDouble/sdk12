/****************************************************************************
 * Copyright (c) 2025 <SRTL>. All rights reserved.
 *
 * This file is part of <TSP>.
 *
 * Licensed under the  Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: RunJie Xu
 ****************************************************************************/

#ifndef S3MQTTMSG_H
#define S3MQTTMSG_H
#include "stddef.h"

#ifdef __cplusplus
extern "C"
{
#endif
    typedef enum
    {
        MQTT_QOS_0 = 0, // 最多一次 (at most once)
        MQTT_QOS_1 = 1, // 至少一次 (at least once)
        MQTT_QOS_2 = 2  // 只有一次 (exactly once)
    } SRMqttQos_t;

    typedef enum
    {
        MQTT_RETAIN_OFF = 0, // 不保留消息
        MQTT_RETAIN_ON = 1   // 保留消息
    } SRMqttRetain_t;

#define SRMqttMessage_initializer {{0}, 0, {0}, 0, MQTT_QOS_1, MQTT_RETAIN_OFF}

    typedef struct
    {
        char topicName[128]; // 固定数组，手动拷贝进去
        size_t topicLen;     // topic的有效长度
        char payload[4096];  // 动态payload
        size_t payloadLen;   // payload的有效长度
        SRMqttQos_t qos;
        SRMqttRetain_t retain;
    } SRMqttMessage_t;

    int SRMqttMessageInit(SRMqttMessage_t *msg);
    int SRMqttMessageSetTopic(SRMqttMessage_t *msg, const char *topic, size_t len);
    int SRMqttMessageSetPayload(SRMqttMessage_t *msg, const char *data, size_t len);
    int SRMqttMessageSetQos(SRMqttMessage_t *msg, const SRMqttQos_t qos);
    int SRMqttMessageSetRetain(SRMqttMessage_t *msg, const SRMqttRetain_t retain);
#ifdef __cplusplus
}
#endif
#endif // S3MQTTMSG_H
