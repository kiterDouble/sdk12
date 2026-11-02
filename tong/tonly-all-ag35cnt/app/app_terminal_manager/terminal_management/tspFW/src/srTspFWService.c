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
#include "srTspFWProtocol.h"
#include "srTspFWService.h"
#include "stdlib.h"
#include "string.h"

static P_SRTspPlatformInfo g_tsp_ser = NULL;

int32_t SRTspServiceInit(const char *clientId, const char *brokerUri)
{
    if (clientId == NULL || brokerUri == NULL)
    {
        SR_TSP_LOG_E("SRTspServiceInit Invalid input: clientId or brokerUri is NULL");
        return RET_ERR;
    }

    if (g_tsp_ser != NULL)
    {
        return RET_ERR;
    }
    g_tsp_ser = (P_SRTspPlatformInfo)calloc(1, sizeof(TspPlatformInfo));
    if (g_tsp_ser == NULL)
    {
        SR_TSP_LOG_E("SRTspServiceInit Failed to allocate memory for TspPlatformInfo");
        return RET_ERR;
    }
    SRTspServiceInitProtocol();
    return SRTspPlatformInit(&(g_tsp_ser->platform), clientId, brokerUri, SRTspServiceProtocolCbProcess);
}

int32_t SRTspServiceConnect(const char *username, const char *password)
{
    int32_t ret = RET_ERR;

    ret = SRThreadQueueCreate(&(g_tsp_ser->ACKQueue), sizeof(SRMsgQueueInternal),
                              SR_QUEUE_CALLBACK_QUEUE_DEFAULT_MAX_CAPACITY);
    if (RET_OK != ret)
    {
        SR_TSP_LOG_E("ACKQueue create failed %d\n", ret);
        return RET_ERR;
    }

    ret = SRThreadQueueCreate(&(g_tsp_ser->ReplyQueue), sizeof(SRMsgQueueInternal),
                              SR_QUEUE_CALLBACK_QUEUE_DEFAULT_MAX_CAPACITY);
    if (RET_OK != ret)
    {
        SR_TSP_LOG_E("ACKQueue create failed %d\n", ret);
        return RET_ERR;
    }

    ret = SRTspPlatformConnectNode(&(g_tsp_ser->platform), username, password);
    if (RET_OK != ret)
    {
        SR_TSP_LOG_E("[SRTspServiceConnect] TSP Server Connect Failed\n");
        return RET_ERR;
    }

    SR_TSP_LOG_I("[SRTspServiceConnect] TSP Server Connect SUCCESS");
    return RET_OK; // 创建成功
}

int32_t SRTspServiceDisconnect()
{
    if (g_tsp_ser == NULL)
    {
        SR_TSP_LOG_E("[SRTspServiceDisconnect] error tsp server not exist.\n");
        return RET_ERR;
    }
    SRTspPlatformDisconnectNode(&(g_tsp_ser->platform));
    if (g_tsp_ser->ReplyQueue != NULL)
    {
        SR_TSP_LOG_I("[SRTspServiceDisconnect] delete service reply queue\n");
        SROSStatus status = 0;
        do
        {
            SRMsgQueueInternal msg = {0};
            status = SRThreadQueueWait(g_tsp_ser->ReplyQueue, (uint8_t *)&msg, sizeof(SRMsgQueueInternal),
                                       E_SR_NO_WAIT);
            if (SR_SUCCESS == status && msg.q != NULL)
            {
                free(msg.q);
                msg.q = NULL;
            }
        } while (SR_SUCCESS == status);
        SRThreadQueueDelete(g_tsp_ser->ReplyQueue);
        g_tsp_ser->ReplyQueue = NULL;
    }
    if (g_tsp_ser->ACKQueue != NULL)
    {
        SR_TSP_LOG_I("[SRTspServiceDisconnect] delete service ack queue\n");
        SROSStatus status = 0;
        do
        {
            SRMsgQueueInternal msg = {0};
            status = SRThreadQueueWait(g_tsp_ser->ACKQueue, (uint8_t *)&msg, sizeof(SRMsgQueueInternal), E_SR_NO_WAIT);
            if (SR_SUCCESS == status && msg.q != NULL)
            {
                free(msg.q);
                msg.q = NULL;
            }
        } while (SR_SUCCESS == status);
        SRThreadQueueDelete(g_tsp_ser->ACKQueue);
        g_tsp_ser->ACKQueue = NULL;
    }
    return RET_OK;
}

int32_t SRTspServiceGetStatus()
{
    if (g_tsp_ser == NULL)
    {
        SR_TSP_LOG_I("[SRTspServiceGetStatus] error tsp server not exist.\n");
        return RET_ERR;
    }
    return SRTspPlatformGetStatus(&(g_tsp_ser->platform));
}

int32_t SRTspServiceSubscribe(const char *topic, int qos)
{
    if (g_tsp_ser == NULL)
    {
        SR_TSP_LOG_I("[SRTspServiceGetStatus] error tsp server not exist.\n");
        return RET_ERR;
    }
    return SRTspPlatformSubscribe(&(g_tsp_ser->platform), topic, qos);
}

int32_t SRTspServiceExecuteNoRes(SRMqttMessage_t *pTxData)
{
    if (g_tsp_ser == NULL)
    {
        SR_TSP_LOG_E("[SRTspServiceExecuteNoRes] error tsp server not exist.\n");
        return RET_ERR;
    }
    int32_t ret = RET_ERR;
    if (!pTxData)
    {
        SR_TSP_LOG_E("[SRTspServiceExecuteNoRes] args error.\n");
        return -1;
    }

    ret = SRTspPlatformSendData(&(g_tsp_ser->platform), pTxData);

    return ret;
}

int32_t SRTspServiceExecute(SRMqttMessage_t *pTxData, SRMqttMessage_t *pRxData, uint32_t timeout)
{
    if (NULL == g_tsp_ser)
    {
        SR_TSP_LOG_D("[SRTspServiceExecute] error tsp server not exist.\n");
        return RET_ERR;
    }
    uint8_t times = SR_TSP_SEND_RETRY_COUNT;
    if (!pTxData || !pRxData)
    {
        SR_TSP_LOG_D("[SRTspServiceExecute] args error.\n");
        return RET_ERR;
    }
    int32_t ret = RET_ERR;
    SROSStatus status = 0;
    do
    {
        SRMsgQueueInternal msg = {0};
        status = SRThreadQueueWait(g_tsp_ser->ACKQueue, (uint8_t *)&msg, sizeof(SRMsgQueueInternal), E_SR_NO_WAIT);
        if (0 == status && msg.q != NULL)
        {
            free(msg.q);
            msg.q = NULL;
        }
    } while (0 == status);

    SRMsgQueueInternal msg = {0};
    SRMqttMessage_t *rec_data = NULL;
    while (times-- > 0 && (g_tsp_ser->ACKQueue != NULL))
    {
        ret = SRTspPlatformSendData(&(g_tsp_ser->platform), pTxData);
        if (ret == RET_ERR)
        {
            SR_TSP_LOG_D("[SRTspServiceExecute] send fail.\n");
            return RET_ERR;
        }
        ret = SRThreadQueueWait(g_tsp_ser->ACKQueue, (void *)&msg, sizeof(SRMsgQueueInternal), timeout);
        if (RET_OK != ret)
        {
            return ret == RET_ERR ? RET_ERR : RET_TIMEOUT;
        }
        if (msg.q == NULL)
        {
            continue;
        }
        rec_data = (SRMqttMessage_t *)msg.q;
        memcpy(pRxData, rec_data, sizeof(SRMqttMessage_t));
        free(rec_data);
        rec_data = NULL;
        return RET_OK;
    }
    return RET_ERR;
}

int32_t SRTspServiceRecData(SRMqttMessage_t *pRxData, uint32_t timeout)
{
    if (NULL == g_tsp_ser)
    {
        return RET_ERR;
    }
    SRMsgQueueInternal msg = {0};
    SRMqttMessage_t *recData = NULL;
    int32_t ret = RET_ERR;
    if (0 != SRThreadQueueWait(g_tsp_ser->ReplyQueue, (void *)&msg, sizeof(SRMsgQueueInternal), timeout))
    {
        goto error_exit;
    }
    recData = (SRMqttMessage_t *)msg.q;
    // 将recData的数据拷贝到pDxData中
    memcpy(pRxData, recData, sizeof(SRMqttMessage_t));
    ret = RET_OK; // 成功

error_exit:
    if (recData)
    {
        free(recData);
        recData = NULL;
        msg.q = NULL;
    }
    return ret;
}

int32_t SRTspServiceDeInit()
{
    if (NULL == g_tsp_ser)
    {
        return RET_ERR;
    }
    if (SRTspServiceGetStatus() == SR_TSP_PLATFORM_CONNECTED)
    {
        SRTspServiceDisconnect();
    }
    SRTspPlatformDeInit(&(g_tsp_ser->platform));
    free(g_tsp_ser);
    g_tsp_ser = NULL;
    return RET_OK;
}

int32_t SRTspServiceSendMessage(SRMqttMessage_t *msg)
{
    if (g_tsp_ser == NULL)
    {
        SR_TSP_LOG_E("[SRTspServiceSendMessage] tsp server not exist.\n");
        return RET_ERR;
    }
    return SRTspPlatformSendData(&(g_tsp_ser->platform), msg);
}

/* 触发应答指令，应答指令 */
int32_t SRTspServiceActivateReply(uint32_t size, uint8_t *msgPtr, uint32_t timeout)
{
    if (g_tsp_ser == NULL)
    {
        SR_TSP_LOG_D("[SRTspServiceActivateReply] error tsp server not exist.\n");
        return RET_ERR;
    }
    return SRThreadQueueRelease(g_tsp_ser->ReplyQueue, size, msgPtr, timeout);
}

/* TSP响应包主要响应内容，云端发送的内容 */
int32_t SRTspServiceActivateAck(uint32_t size, uint8_t *msgPtr, uint32_t timeout)
{
    if (g_tsp_ser == NULL)
    {
        SR_TSP_LOG_D("[SRTspServiceActivateAck] error tsp server not exist.\n");
        return RET_ERR;
    }
    return SRThreadQueueRelease(g_tsp_ser->ACKQueue, size, msgPtr, timeout);
}
