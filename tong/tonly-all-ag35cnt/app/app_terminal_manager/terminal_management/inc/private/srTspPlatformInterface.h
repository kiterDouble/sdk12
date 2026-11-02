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

#ifndef SR_TSP_STATE_MACHINE_H
#define SR_TSP_STATE_MACHINE_H
#include <stdint.h>
#include "srTspFWInterface.h"
#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief 启动 TSP 状态机
     *
     * 初始化并启动 TSP（Telematics Service Platform）状态机，
     * 状态机将开始管理MQTT连接、数据处理和业务流程。
     *
     * @return
     *      0：启动成功
     *     -1：启动失败（如资源错误、已启动等）
     *
     * @note
     *      本接口是非阻塞的，启动后状态机在后台独立运行。
     */
    int32_t StartTspStateMachine();

    /**
     * @brief 停止 TSP 状态机
     *
     * 停止正在运行的 TSP 状态机，释放相关资源并退出所有任务流程。
     *
     * @return
     *      0：停止成功
     *     -1：停止失败（如未启动、停止过程中出错）
     *
     * @note
     *      本接口会阻塞至状态机完全退出，执行时间依赖于内部状态清理。
     */
    int32_t StopTspStateMachine();

    /**
     * @brief 同步执行 TSP 服务请求
     *
     * 向 TSP 平台发送一条服务请求消息，并同步等待应答消息或超时。
     *
     * @param[in]  pTxData   待发送的MQTT消息内容
     * @param[out] pRxData   接收应答的MQTT消息缓存指针
     * @param[in]  timeout   等待应答超时时间（毫秒）
     *
     * @return
     *      0：服务调用成功，已收到应答
     *     -1：服务调用失败（如超时、连接异常、参数错误）
     *
     * @note
     *      本接口为阻塞调用，直到接收到应答或超时退出。
     */
    int32_t SRTspServiceExecute(SRMqttMessage_t *pTxData, SRMqttMessage_t *pRxData, uint32_t timeout);

    /**
     * @brief 异步发送 TSP 服务请求（无应答）
     *
     * 向 TSP 平台发送一条服务请求消息，不等待应答，立即返回。
     *
     * @param[in] pTxData    待发送的MQTT消息内容
     *
     * @return
     *      0：发送成功
     *     -1：发送失败（如连接异常、参数错误）
     *
     * @note
     *      本接口为非阻塞调用，仅发送数据，不等待平台返回。
     */
    int32_t SRTspServiceExecuteNoRes(SRMqttMessage_t *pTxData);

    /**
     * @brief 设置 CA 根证书的保存路径
     *
     * 此函数用于设置 TSP 模块中根证书（CA Certificate）保存的目录路径。
     * 后续根证书写入与读取均将使用此目录进行文件操作。
     * 该路径必须是一个有效的、具备读写权限的目录。
     *
     * @param[in] dirPath  根证书保存目录路径，需为以 '\0' 结尾的字符串
     *
     * @return
     *      RET_OK (0)：设置成功
     *      RET_ERR (-1)：参数为空或路径无效
     *
     * @note
     *      - 此函数应在证书操作前调用，确保路径已初始化。
     *      - 若未设置路径，相关证书保存操作将失败。
     */
    int32_t SRTspServiceSetCaSavePath(const char *dirPath);

#ifdef __cplusplus
}
#endif

#endif // SRTSPSTATEMACHINE_H
