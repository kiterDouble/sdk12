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
#include <stdio.h>
#include <string.h>
#include "srTspFWMqttClient.h"
#include "srTspFWOs.h"
#include "srTspFWLog.h"
#include "srTspFWCaCertHandler.h"

typedef void (*SRMqttConnectionLostCallback)(const char *cause);
static int SRMqttConnect(SRMQTTClient client, const char *username, const char *password);

int SRMqttInit(SRMQTTClient *client, const char *clientId, const char *brokerUri, MQTTClient_connectionLost *cl, MQTTClient_messageArrived *ma)
{
    SR_TSP_LOG_I("Initializing client ID: %s, broker: %s", clientId, brokerUri);
    if (MQTTClient_create(client, brokerUri, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL) != MQTTCLIENT_SUCCESS)
    {
        SR_TSP_LOG_E("Failed to create MQTT client");
        return RET_ERR;
    }
    MQTTClient_setCallbacks(*client, NULL, cl, ma, NULL);
    return RET_OK;
}

static int SRMqttConnect(SRMQTTClient client, const char *username, const char *password)
{
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession = 1;
    conn_opts.username = username;
    conn_opts.password = password;

    char CaCrtPath[512] = {0};
    char ClientCrtPath[512] = {0};
    char ClientKeyPath[512] = {0};
    // 3. 获取 full path
    int certRet = RET_OK;
    if (SRTspGetCaPath(CaCrtPath, sizeof(CaCrtPath)) != RET_OK)
        certRet = RET_ERR;
    if (SRTspGetClientRetPath(ClientCrtPath, sizeof(ClientCrtPath)) != RET_OK)
        certRet = RET_ERR;
    if (SRTspGetClientKeyPath(ClientKeyPath, sizeof(ClientKeyPath)) != RET_OK)
        certRet = RET_ERR;
    if (certRet == RET_OK)
    {
        MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
        ssl_opts.trustStore = CaCrtPath;     // 必须是 CA 证书
        ssl_opts.keyStore = ClientCrtPath;   // 客户端证书
        ssl_opts.privateKey = ClientKeyPath; // 客户端私钥
        ssl_opts.enableServerCertAuth = 1;
        ssl_opts.verify = 1;
        ssl_opts.sslVersion = MQTT_SSL_VERSION_TLS_1_2;
        conn_opts.ssl = &ssl_opts;
        SR_TSP_LOG_I("[CA] Full ca.crt path: %s\n", CaCrtPath);
        SR_TSP_LOG_I("[CA] Full client.crt path: %s\n", ClientCrtPath);
        SR_TSP_LOG_I("[CA] Full client.key path: %s\n", ClientKeyPath);
    }

    SR_TSP_LOG_I("Attempting to connect with username: %s", username ? username : "NULL");

    int rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        SR_TSP_LOG_E("Connect failed, code %d", rc);
        return RET_ERR;
    }

    SR_TSP_LOG_I("Connected successfully");
    return RET_OK;
}

int SRMqttConnectNode(SRMQTTClient client, const char *username, const char *password)
{
    if (client == NULL)
    {
        return RET_ERR;
    }
    return SRMqttConnect(client, username, password);
}

int SRMqttDisconnect(SRMQTTClient client)
{
    if (client == NULL)
    {
        return RET_ERR;
    }
    if (MQTTClient_isConnected(client))
    {
        SR_TSP_LOG_I("Disconnecting MQTT client");
        MQTTClient_disconnect(client, 1000);
    }
    else
    {
        SR_TSP_LOG_I("MQTT already disconnected");
    }

    return RET_OK;
}

int SRMqttDeInit(SRMQTTClient client)
{
    if (client == NULL)
    {
        return RET_ERR;
    }
    SR_TSP_LOG_I("Destroying MQTT client");
    MQTTClient_destroy(&client);
    return RET_OK;
}

int SRMqttGetStatus(SRMQTTClient client)
{
    if (client == NULL)
    {
        return SR_MQTT_PLATFORM_CONNECT_FAIL;
    }
    int status = MQTTClient_isConnected(client);
    return status ? SR_MQTT_PLATFORM_CONNECT_OK : SR_MQTT_PLATFORM_CONNECT_FAIL;
}

int SRMqttPublish(SRMQTTClient client, const char *topic, const char *payload, int payloadLen, SRMqttQos_t qos,
                  SRMqttRetain_t retain)
{
    if (client == NULL)
    {
        return RET_ERR;
    }
    SR_TSP_LOG_I("Publishing to topic: %s", topic);

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = (void *)payload;
    pubmsg.payloadlen = payloadLen;
    pubmsg.qos = qos;
    pubmsg.retained = retain;

    MQTTClient_deliveryToken token;
    int rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        SR_TSP_LOG_E("Failed to publish message, code %d", rc);
        return RET_ERR;
    }

    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        SR_TSP_LOG_E("Failed to complete publish, code %d", rc);
        return RET_ERR;
    }
    SR_TSP_LOG_I("Publish complete");
    return RET_OK;
}

int SRMqttSubscribe(SRMQTTClient client, const char *topic, int qos)
{
    if (client == NULL)
    {
        return RET_ERR;
    }
    SR_TSP_LOG_I("Subscribing to topic: %s", topic);
    return MQTTClient_subscribe(client, topic, qos);
}
