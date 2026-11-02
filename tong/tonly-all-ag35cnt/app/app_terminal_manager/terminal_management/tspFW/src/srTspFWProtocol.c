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
#include <MQTTClient.h>
#include <stdbool.h>
#include "srTspFWMqttClient.h"
#include "srTspFWProtocol.h"
#include "srTspFWOs.h"
#include "srTspFWService.h"
#include "string.h"
#include "stdlib.h"
#include "srTspFWLog.h"
#include "srTspCallbackRegister.h"
#include "srTspEncryptAndDecrypt.h"

// 默认最大topic是32位
#define SR_PROTOCOL_MAX_ENTRIES 32

typedef int32_t (*SRTspServiceCommandIdentificationCb)(const char *topicName, int topicLen,
                                                       const MQTTClient_message *message);
static int32_t SRMqttProtocolAsyncRecvResponseHandle(const char *topicName, const int topicLen,
                                                     const MQTTClient_message *message);
static int32_t SRMqttProtocolSyncRecvResponseHandle(const char *topicName, const int topicLen,
                                                    const MQTTClient_message *message);

typedef struct
{
    char *topic;
    SRTspServiceCommandIdentificationCb handler;
} SRProtocolHandlerEntry;

static SRProtocolHandlerEntry g_protocolTable[SR_PROTOCOL_MAX_ENTRIES];
static uint32_t g_protocolTableCount = 0;
static bool g_protocolInitFlag = false;

static void SRProtocolRegisterHandler(const char *topic, SRTspServiceCommandIdentificationCb handler)
{
    if (!topic || !handler)
        return;
    if (g_protocolTableCount >= SR_PROTOCOL_MAX_ENTRIES)
    {
        return;
    }
    // 复制 topic 字符串（内部持有）x
    g_protocolTable[g_protocolTableCount].topic = strdup(topic);
    if (g_protocolTable[g_protocolTableCount].topic == NULL)
    {
        // 内存申请失败
        return;
    }
    g_protocolTable[g_protocolTableCount].handler = handler;
    g_protocolTableCount++;
}

int32_t SRTspServiceInitProtocol()
{
    if (g_protocolInitFlag)
    {
        SR_TSP_LOG_I("protocol has been Init...");
        return RET_OK;
    }
    g_protocolInitFlag = true;

    char snBuffer[64] = {0};
    TspBaseInfoGetSn(snBuffer, sizeof(snBuffer));

    char topic[128];
    SR_BUILD_TOPIC(topic, SR_TOPIC_PREFIX_S2T_CONNECT_PLATFORM, snBuffer);
    SRProtocolRegisterHandler(topic, SRMqttProtocolSyncRecvResponseHandle);
    SR_BUILD_TOPIC(topic, SR_TOPIC_PREFIX_S2T_REPORT_PLATFORM, snBuffer);
    SRProtocolRegisterHandler(topic, SRMqttProtocolSyncRecvResponseHandle);
    SR_BUILD_TOPIC(topic, SR_TOPIC_PREFIX_S2T_REISSUE_PLATFORM, snBuffer);
    SRProtocolRegisterHandler(topic, SRMqttProtocolSyncRecvResponseHandle);
    SR_BUILD_TOPIC(topic, SR_TOPIC_PREFIX_S2T_COMMAND_PLATFORM, snBuffer);
    SRProtocolRegisterHandler(topic, SRMqttProtocolAsyncRecvResponseHandle);

    return RET_OK;
}

/**
 * @brief 在 payload 入队前原地解密（若加密会话已激活）
 * @return RET_OK 成功（或无需解密），RET_ERR 解密失败
 */
static int32_t SRDecryptPayloadIfActive(SRMqttMessage_t *msg)
{
    if (!msg || !EncryptionIsActive())
    {
        return RET_OK;
    }

    uint8_t *plaintext = NULL;
    int plainLen = DecryptPayload(
        (const uint8_t *)msg->payload, msg->payloadLen, &plaintext);

    if (plainLen <= 0 || plainLen > (int)sizeof(msg->payload))
    {
        SR_TSP_LOG_E("Decrypt inbound payload failed: plainLen=%d", plainLen);
        if (plaintext)
            free(plaintext);
        return RET_ERR;
    }

    (void)memcpy(msg->payload, plaintext, (size_t)plainLen);
    msg->payloadLen = (size_t)plainLen;
    free(plaintext);
    return RET_OK;
}

/**
 * @brief 异步接收响应数据的回调处理
 *
 * @param topicName
 * @param topicLen
 * @param message
 * @return int32_t
 */
static int32_t SRMqttProtocolAsyncRecvResponseHandle(const char *topicName, const int topicLen,
                                                     const MQTTClient_message *message)
{
    int32_t ret = RET_ERR;
    SRMqttMessage_t *msg = (SRMqttMessage_t *)calloc(1, sizeof(SRMqttMessage_t));
    if (msg == NULL)
    {
        SR_TSP_LOG_E("%s: Failed to allocate memory", __func__);
        return RET_ERR;
    }
    SRMqttMessageInit(msg);
    SRMqttMessageSetTopic(msg, topicName, topicLen);
    SRMqttMessageSetPayload(msg, message->payload, message->payloadlen);
    if (SRDecryptPayloadIfActive(msg) != RET_OK)
    {
        SR_TSP_LOG_E("%s: decrypt failed, drop msg", __func__);
        free(msg);
        return RET_ERR;
    }
    SRMqttMessageSetQos(msg, message->qos);
    SRMqttMessageSetRetain(msg, message->retained);

    SRMsgQueueInternal msgInternal = {0};
    msgInternal.q = msg;

    ret = SRTspServiceActivateReply(sizeof(msgInternal), (void *)&msgInternal, E_SR_NO_WAIT);
    if (ret != RET_OK)
    {
        SR_TSP_LOG_E("%s: Send to ReplyQueue failed, ret = %d\n", __func__, ret);
        free(msg);
        msg = NULL;
    }
    return ret;
}

/**
 * @brief 同步接收响应数据的回调处理
 * 发送请求数据后同步等待接收平台响应，将所有回调的数据以FIFO的逻辑入队
 *
 * @param topicName
 * @param topicLen
 * @param message
 * @return int32_t
 */
static int32_t SRMqttProtocolSyncRecvResponseHandle(const char *topicName, const int topicLen,
                                                    const MQTTClient_message *message)
{
    int32_t ret = RET_ERR;
    SRMqttMessage_t *msg = (SRMqttMessage_t *)calloc(1, sizeof(SRMqttMessage_t));
    if (msg == NULL)
    {
        SR_TSP_LOG_E("%s Failed to allocate memory for SRMqttMessage_t", __func__);
        return RET_ERR;
    }
    SRMqttMessageInit(msg);
    SRMqttMessageSetTopic(msg, topicName, topicLen);
    SRMqttMessageSetPayload(msg, message->payload, message->payloadlen);
    SRMqttMessageSetQos(msg, message->qos);
    SRMqttMessageSetRetain(msg, message->retained);
    SRMsgQueueInternal msgInternal = {0};
    msgInternal.q = msg;
    ret = SRTspServiceActivateAck(sizeof(msgInternal), (void *)&msgInternal, E_SR_NO_WAIT);
    if (ret != RET_OK)
    {
        SR_TSP_LOG_E("MqttProtocolRecvRemoteControlHandleRes platform.ReplyQueue send queue failed ret = [%d]\n", ret);
        free(msg);
        msg = NULL;
    }
    return ret;
}

static int32_t TspProtocolRecvProcess(char *topicName, int topicLen, MQTTClient_message *message)
{
    if (topicName == NULL || message == NULL)
    {
        SR_TSP_LOG_E("Invalid input: topicName or message is NULL");
        return RET_ERR;
    }

    for (uint32_t i = 0; i < g_protocolTableCount; ++i)
    {
        const SRProtocolHandlerEntry *entry = &g_protocolTable[i];

        if (entry->topic == NULL)
        {
            continue; // 跳过空 entry
        }

        if (strcmp(entry->topic, topicName) != 0)
        {
            continue; // topic 不匹配，继续找下一个
        }

        if (entry->handler == NULL)
        {
            SR_TSP_LOG_W("Handler for topic %s is NULL", topicName);
            return RET_ERR; // 找到了 topic，但没 handler
        }

        entry->handler(topicName, topicLen, message);
        return RET_OK; // 找到并成功处理
    }
    SR_TSP_LOG_I("payloadType %s not support\n", topicName);
    return RET_ERR;
}

int32_t SRTspServiceProtocolCbProcess(char *topicName, int topicLen, MQTTClient_message *message)
{
    return TspProtocolRecvProcess(topicName, topicLen, message);
}
