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
#ifndef SR_TSP_FW_INTERFACE_H
#define SR_TSP_FW_INTERFACE_H
#include "srTspFWOs.h"
#include "srTspFWMqttMsg.h"
#include "srTspFWProtocol.h"
#include "srTspFWLog.h"
#include "srTspFWCaCertHandler.h"

#define SR_TSP_SERVICE_CONNECT 0
#define SR_TSP_SERVICE_NOT_CONNECT (-1)

/* TSP状态机状态定义 */
typedef enum
{
    MQTT_STATE_INIT = 0,       /**< 初始化阶段 */
    MQTT_STATE_CONNECTING,     /**< 正在连接Broker */
    MQTT_STATE_CONNECTED,      /**< 已连接Broker */
    MQTT_STATE_SUBSCRIBED,     /**< 已订阅Topic */
    MQTT_STATE_LOGIN,          /**< 登陆         */
    MQTT_STATE_LOGOUT,         /**< 登陆失败        */
    MQTT_STATE_RECV,           /**< 正常接收数据中 */
    MQTT_STATE_ERROR,          /**< 错误发生，等待重连 */
    MQTT_STATE_RECONNECT_WAIT, /**< 重连等待阶段 */
    MQTT_STATE_DEINIT,         /**< 状态机退出阶段 */
} SRTspState_e;

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t SRTspServiceInit(const char *clientId, const char *brokerUri);
    int32_t SRTspServiceConnect(const char *username, const char *password);
    int32_t SRTspServiceSubscribe(const char *topic, int qos);
    int32_t SRTspServiceGetStatus();
    int32_t SRTspServiceDisconnect();
    int32_t SRTspServiceRecData(SRMqttMessage_t *pRxData, uint32_t timeout);
    int32_t SRTspServiceDeInit();

    int32_t SRTspServiceExecute(SRMqttMessage_t *pTxData, SRMqttMessage_t *pRxData, uint32_t timeout);
    int32_t SRTspServiceExecuteNoRes(SRMqttMessage_t *pTxData);

#ifdef __cplusplus
}
#endif
#endif
