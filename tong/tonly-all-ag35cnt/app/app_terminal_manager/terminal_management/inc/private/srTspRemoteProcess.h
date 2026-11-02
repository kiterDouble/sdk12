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

#ifndef SR_TSP_REMOTE_PROCESS_H
#define SR_TSP_REMOTE_PROCESS_H

#include "srTspFWMqttClient.h"
#include "srTspFWInterface.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 通过命令ID查询任务是否正在执行
     *
     * @param cmdId
     * @return true
     * @return false
     */
    bool TaskIsRunning(const char *cmdId);

    /**
     * @brief 从已接收命令集合中移除指定命令ID
     * @param cmdId 命令ID
     */
    void RemoveTaskCmdId(const char *cmdId);

    /**
     * @brief 处理TSP平台下发的远程指令消息
     *
     * 解析并处理TSP平台下发的远程控制类MQTT消息，
     * 根据消息内容触发对应的本地业务逻辑或设备操作。
     *
     * @param[in] msg   收到的MQTT远程指令消息指针
     *
     * @return
     *      0：处理成功
     *     -1：处理失败（如消息格式错误、解析失败、内部异常）
     *
     * @note
     *      本接口通常在消息接收回调中调用，用于统一处理远程控制类业务。
     *      要求msg指针有效，且指向完整接收到的MQTT消息数据。
     */
    int TspRemoteProcess(SRMqttMessage_t *msg);

#ifdef __cplusplus
}
#endif
#endif
