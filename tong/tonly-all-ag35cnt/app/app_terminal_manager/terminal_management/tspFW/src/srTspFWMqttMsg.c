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
#include "srTspFWMqttMsg.h"
#include "string.h"
#include "srTspFWMqttClient.h"
#include "srTspFWLog.h"

int SRMqttMessageInit(SRMqttMessage_t *msg)
{
    if (msg == NULL)
    {
        SR_TSP_LOG_E("SRMqttMessageInit called with NULL pointer");
        return RET_ERR;
    }

    memset(msg->topicName, 0, sizeof(msg->topicName));
    msg->topicLen = 0;
    memset(msg->payload, 0, sizeof(msg->payload));
    msg->payloadLen = 0;
    msg->qos = MQTT_QOS_1;         // 默认QoS 1
    msg->retain = MQTT_RETAIN_OFF; // 默认不保留

    return RET_OK;
}

int SRMqttMessageSetTopic(SRMqttMessage_t *msg, const char *topic, const size_t len)
{
    if (!msg || !topic)
        return RET_ERR;

    if (strlen(topic) >= sizeof(msg->topicName))
    {
        SR_TSP_LOG_E("Topic too long, size=%zu, max=%zu", strlen(topic), sizeof(msg->topicName) - 1);
        return RET_ERR;
    }
    strcpy(msg->topicName, topic);
    msg->topicLen = strlen(msg->topicName);

    return RET_OK;
}

int SRMqttMessageSetPayload(SRMqttMessage_t *msg, const char *data, size_t len)
{
    if (!msg || !data)
        return RET_ERR;
    memcpy(msg->payload, data, len);
    msg->payloadLen = len;
    return RET_OK;
}

int SRMqttMessageSetQos(SRMqttMessage_t *msg, const SRMqttQos_t qos)
{
    if (!msg)
        return RET_ERR;
    msg->qos = qos;
    return RET_OK;
}

int SRMqttMessageSetRetain(SRMqttMessage_t *msg, const SRMqttRetain_t retain)
{
    if (!msg)
        return RET_ERR;
    msg->retain = retain;
    return RET_OK;
}
