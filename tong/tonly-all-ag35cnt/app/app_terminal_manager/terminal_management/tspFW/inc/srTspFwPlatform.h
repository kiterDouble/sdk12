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
#ifndef SR_TSP_PLATFORM_H
#define SR_TSP_PLATFORM_H
#include "srTspFWMqttClient.h"
#include "srTspFWOs.h"

#define SR_MQTT_MAX_NAME_LENGTH (256) /**< TSP连接最大名称长度。 */

#define SR_TSP_PLATFORM_CONNECTED (0)
#define SR_TSP_PLATFORM_NOT_CONNECTED (-1)

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int32_t (*P_SRMqttMessageCallback)(char *topicName, int topicLen, MQTTClient_message *message);

    typedef struct PlatformInfo
    {
        char name[SR_MQTT_MAX_NAME_LENGTH];
        SRMQTTClient client;
        int32_t isInitialized;
        SRMutex_t mutex; // 用于数据发送的互斥锁
    } SRPlatformInfo, *P_SRPlatformInfo;

    int32_t SRTspPlatformInit(P_SRPlatformInfo platform, const char *clientId, const char *brokerUri, P_SRMqttMessageCallback msgCallback);

    int32_t SRTspPlatformConnectNode(P_SRPlatformInfo platform, const char *username, const char *password);

    int32_t SRTspPlatformDisconnectNode(P_SRPlatformInfo platform);

    int32_t SRTspPlatformGetStatus(P_SRPlatformInfo platform);

    int32_t SRTspPlatformSendData(P_SRPlatformInfo platform, SRMqttMessage_t *pTxData);

    int32_t SRTspPlatformDeInit(P_SRPlatformInfo platform);

    int32_t SRTspPlatformSubscribe(P_SRPlatformInfo platform, const char *topic, int qos);

#ifdef __cplusplus
}
#endif
#endif // SRMQTTPLATFORM_H
