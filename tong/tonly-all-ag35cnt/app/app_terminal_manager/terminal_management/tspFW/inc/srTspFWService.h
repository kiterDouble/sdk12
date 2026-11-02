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
#ifndef SR_MQTT_SERVICE_H
#define SR_MQTT_SERVICE_H

#include "srTspFWOs.h"
#include "srTspFwPlatform.h"
#include "srTspFWInterface.h"

// #define TEST_NOACK // 平台无ACK回执，无接收和解析功能

#ifdef TEST_NOACK
#define SR_TSP_SEND_RETRY_COUNT (1)
#else
#define SR_TSP_SEND_RETRY_COUNT (3)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TspPlatformInfo
    {
        SRQueue_t *ACKQueue;     // 交互响应接收队列
        SRQueue_t *ReplyQueue;   // 下行命令接收队列
        SRPlatformInfo platform; // 如果需要，这里也要确保 P_SRPlatformInfo 被定义
    } TspPlatformInfo, *P_SRTspPlatformInfo;

    int32_t SRTspServiceSendMessage(SRMqttMessage_t *msg);

    /* 提供给协议层的借口 service 实现 */
    int32_t SRTspServiceActivateAck(uint32_t size, uint8_t *msgPtr, uint32_t timeout);
    int32_t SRTspServiceActivateReply(uint32_t size, uint8_t *msgPtr, uint32_t timeout);

    /* 协议层必须实现此接口协议层需按丝线 [TCP协议 基于TLV] */
    int32_t SRTspServiceInitProtocol();
    int32_t SRTspServiceProtocolCbProcess(char *topicName, int topicLen, MQTTClient_message *message);
#ifdef __cplusplus
}
#endif
#endif // SRMQTTSERVICE_H
