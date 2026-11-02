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
#ifndef _SR_MQTT_CLIENT_H_
#define _SR_MQTT_CLIENT_H_

#include <MQTTClient.h>
#include "srTspFWMqttMsg.h"

#define RET_OK 0
#define RET_ERR (-1)
#define RET_TIMEOUT (-99)

#define TIMEOUT 10000L

#define SR_MQTT_PLATFORM_CONNECT_OK 0
#define SR_MQTT_PLATFORM_CONNECT_FAIL (-1)

typedef MQTTClient SRMQTTClient;

#ifdef __cplusplus
extern "C"
{
#endif

    int SRMqttInit(SRMQTTClient *client, const char *clientId, const char *brokerUri, MQTTClient_connectionLost *cl, MQTTClient_messageArrived *ma);
    int SRMqttConnectNode(SRMQTTClient client, const char *username, const char *password);
    int SRMqttDisconnect(SRMQTTClient client);
    int SRMqttGetStatus(SRMQTTClient client);
    int SRMqttPublish(SRMQTTClient client, const char *topic, const char *payload, int payloadLen, SRMqttQos_t qos, SRMqttRetain_t retain);
    int SRMqttSubscribe(SRMQTTClient client, const char *topic, int qos);
    int SRMqttDeInit(SRMQTTClient client);
#ifdef __cplusplus
}
#endif
#endif
