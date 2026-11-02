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

#include "srTspFwPlatform.h"
#include "srTspFWMqttClient.h"
#include "srTspFWLog.h"

static int SRMessageArrivedCallback(void *context, char *topicName, int topicLen, MQTTClient_message *message);
static void SRConnectionLostCallback(void *context, char *cause);
static P_SRMqttMessageCallback g_msg_callback = NULL;

static int SRMessageArrivedCallback(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    SR_TSP_LOG_I("Received message on topic [%s] %d", topicName, message->payloadlen);
    if (g_msg_callback)
    {
        g_msg_callback(topicName, topicLen, message);
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

static void SRConnectionLostCallback(void *context, char *cause)
{
    SR_TSP_LOG_W("Connection lost: %s", cause);
    return;
}

int32_t SRTspPlatformInit(P_SRPlatformInfo platform, const char *clientId, const char *brokerUri, P_SRMqttMessageCallback msgCallback)
{
    if (platform == NULL)
    {
        return RET_ERR;
    }
    if (platform->isInitialized)
    {
        return RET_ERR;
    }
    g_msg_callback = msgCallback;
    const int32_t ret = SRMqttInit(&(platform->client), clientId, brokerUri, SRConnectionLostCallback, SRMessageArrivedCallback);
    if (ret != RET_OK)
    {
        return ret;
    }
    if (SRThreadMutexCreate(&(platform->mutex)) != RET_OK) // 初始化互斥
    {
        SR_TSP_LOG_E("SRThreadMutexCreate fail!\n");
        return RET_ERR;
    }
    platform->isInitialized = 1;
    return ret;
}

int32_t SRTspPlatformSubscribe(P_SRPlatformInfo platform, const char *topic, int qos)
{
    if (platform == NULL)
    {
        return RET_ERR;
    }
    return SRMqttSubscribe(platform->client, topic, qos);
}

int32_t SRTspPlatformConnectNode(P_SRPlatformInfo platform, const char *username, const char *password)
{
    if (NULL == platform)
    {
        SR_TSP_LOG_E("PlatformInfo not create\n");
        return RET_ERR;
    }
    if (!platform->isInitialized)
        return RET_ERR;
    return SRMqttConnectNode(platform->client, username, password);
}

int32_t SRTspPlatformDisconnectNode(P_SRPlatformInfo platform)
{
    if (NULL == platform)
    {
        SR_TSP_LOG_E("PlatformInfo not create\n");
        return RET_ERR;
    }
    SRMqttDisconnect(platform->client);
    return RET_OK;
}

int32_t SRTspPlatformGetStatus(P_SRPlatformInfo platform)
{
    if (!platform->isInitialized)
        return RET_ERR;
    return SRMqttGetStatus(platform->client);
}

int32_t SRTspPlatformSendData(P_SRPlatformInfo platform, SRMqttMessage_t *pTxData)
{
    int32_t ret = RET_ERR;
    if (NULL == platform)
    {
        SR_TSP_LOG_E("PlatformInfo not create\n");
        return ret;
    }
    if (NULL == pTxData)
    {
        SR_TSP_LOG_E("pTxData is NULL\n");
        return ret;
    }
    if (SRTspPlatformGetStatus(platform) == SR_TSP_PLATFORM_CONNECTED)
    {
        SRThreadMutexLock(&(platform->mutex));
        ret = SRMqttPublish(platform->client, pTxData->topicName, pTxData->payload, pTxData->payloadLen, pTxData->qos, pTxData->retain);
        SRThreadMutexUnlock(&(platform->mutex));
        return ret;
    }
    SR_TSP_LOG_E("send data failed, platform not connect ...\n");
    return ret;
}

int32_t SRTspPlatformDeInit(P_SRPlatformInfo platform)
{
    if (!platform->isInitialized)
        return RET_ERR;
    SRMqttDeInit(platform->client);
    platform->isInitialized = 0;
    SRThreadMutexDelete(&(platform->mutex));
    return RET_OK;
}
