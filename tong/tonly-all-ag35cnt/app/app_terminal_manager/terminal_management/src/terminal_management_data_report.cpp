#include <pthread.h>
#include <cstdint>
#include <cstdio>
#include <atomic>
#include <string>
#include <vector>
#include <json/json.h>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sys/prctl.h>
#include <curl/curl.h>

#include "terminal_management_api.h"
#include "sr_log.h"
#include "srTspFWInterface.h"
#include "srTspCallbackRegister.h"
#include "srTspFWProtocol.h"
#include "srTspFWService.h"
#include "srTspRemoteProcess.h"
#include "srTspEncryptAndDecrypt.h"
#include "terminal_management_data_monitor.h"
#include "terminal_management_data_packag.h"
#include "terminal_management_data_report.h"
#include "terminal_management_data_response.h"

#include "net/sirun/tboxs3/connect/report.pb.h"
#include "net/sirun/tboxs3/connect/event_report.pb.h"
#include "net/sirun/tboxs3/connect/command.pb.h"

struct tast_t
{
    void (*funcCallback)(void *); // 下行任务回调
    void *args;                   // 下行任务参数
};

struct threadpool_t
{
    pthread_t *threads;
    int threadCount;
    bool destroyFlag;

    tast_t *taskQueue;
    int queueCapacity;
    int readTaskIdx;
    int writeTaskIdx;
    int taskCount;

    pthread_mutex_t lock;
    pthread_cond_t haveIdle;
    pthread_cond_t haveTask;
};

pthread_t g_uploadData = -1;
pthread_t g_cmdExecution = -1;

extern std::atomic<SRTspState_e> g_mqttState; // Mqtt状态机

static std::queue<std::string> g_commandQueue;   // 下行数据命令队列
static std::condition_variable g_commandQueueCV; // 命令队列非空的条件变量
static std::mutex g_commandQueueMutex;

static std::unordered_set<std::string> g_canceledTaskIds; // 取消任务cmdid列表
static std::mutex g_cancelMutex;

// 辅助函数
// ******************************************************************************************************************************************************//

/**
 * @brief 根据数据优先级构造对应的 MQTT 上报消息
 * @param msg      [out] 构造好的 MQTT 消息
 * @param loadData [in]  待发送的数据（包含二进制 payload 和优先级）
 * @return RET_OK 成功，RET_ERR 失败
 */
static int32_t BuildReportMessage(SRMqttMessage_t &msg, const LoadData_t &loadData)
{
    // 检查 payload 大小是否超出消息缓冲区
    if (loadData.len > sizeof(msg.payload))
    {
        SR_TSP_LOG_E("Report payload size too large: %zu bytes (max %zu)",
                     loadData.len, sizeof(msg.payload));
        return RET_ERR;
    }

    char snBuffer[64] = {0};
    TspBaseInfoGetSn(snBuffer, sizeof(snBuffer));

    // 根据优先级选择主题前缀
    const char *topicPrefix = NULL;
    if (loadData.priority == 3)
    {
        // P3 周期数据
        topicPrefix = SR_TOPIC_PREFIX_T2S_REPORT;
    }
    else if (loadData.priority >= 0 && loadData.priority <= 2)
    {
        // P0~P2 事件数据
        topicPrefix = SR_TOPIC_PREFIX_T2S_EVENT;
    }
    else if (loadData.priority == 4)
    {
        // P4 补发数据
        topicPrefix = SR_TOPIC_PREFIX_T2S_REISSUE;
    }
    else
    {
        SR_TSP_LOG_E("Unknown priority %d, cannot determine topic", loadData.priority);
        return RET_ERR;
    }

    // 拷贝二进制 payload（若加密会话已激活则加密后写入）
    if (EncryptionIsActive())
    {
        uint8_t *encrypted = NULL;
        int encLen = EncryptPayload(
            reinterpret_cast<const uint8_t *>(loadData.data), loadData.len, &encrypted);
        if (encLen <= 0 || encLen > (int)sizeof(msg.payload))
        {
            SR_TSP_LOG_E("BuildReportMessage: encrypt failed (encLen=%d, max=%zu)",
                         encLen, sizeof(msg.payload));
            if (encrypted)
                free(encrypted);
            return RET_ERR;
        }
        memcpy(msg.payload, encrypted, (size_t)encLen);
        msg.payloadLen = (size_t)encLen;
        free(encrypted);
    }
    else
    {
        memcpy(msg.payload, loadData.data, loadData.len);
        msg.payloadLen = loadData.len;
    }

    // 拼接完整主题：前缀 + VIN
    snprintf(msg.topicName, sizeof(msg.topicName), "%s%s", topicPrefix, snBuffer);
    msg.topicLen = strlen(msg.topicName);
#ifdef TEST_NOACK
    msg.qos = MQTT_QOS_0;
#else
    msg.qos = MQTT_QOS_1;
#endif

    return RET_OK;
}

/**
 * @brief 解析上报消息的 ACK 响应（暂留空，待后续实现）
 * @param rspMsg 接收到的响应消息
 * @return RET_OK 解析成功，RET_ERR 失败（当前固定返回成功）
 */
static int32_t ParseReportAckWithHandler(const SRMqttMessage_t &rspMsg)
{
    int32_t ret = RET_OK;

    // 接收到ACK说明报文异常
    LOG_Debug("%s() recv topicName:%s", __func__, rspMsg.topicName);

    // 校验响应 topic 前缀（report 或 reissue）
    {
        bool isReport = (strncmp(rspMsg.topicName, SR_TOPIC_PREFIX_S2T_REPORT_PLATFORM,
                                 strlen(SR_TOPIC_PREFIX_S2T_REPORT_PLATFORM)) == 0);
        bool isReissue = (strncmp(rspMsg.topicName, SR_TOPIC_PREFIX_S2T_REISSUE_PLATFORM,
                                  strlen(SR_TOPIC_PREFIX_S2T_REISSUE_PLATFORM)) == 0);
        if (!isReport && !isReissue)
        {
            LOG_Error("ParseReportAck unexpected topicName:%s", rspMsg.topicName);
            return RET_OK;
        }
    }

    do
    {
        // 解析并打印异常原因
        net::sirun::tboxs3::report::ReportReply ackMsg;
        if (!ackMsg.ParseFromArray(rspMsg.payload, rspMsg.payloadLen))
        {
            LOG_Error("ParseReportAck ParseFromArray failed, payloadLen=%zu", rspMsg.payloadLen);
            ret = RET_ERR;
            break;
        }

        const char *reasonStr = net::sirun::tboxs3::report::ReportNackReason_Name(ackMsg.reason()).c_str();
        const char *msgStr = ackMsg.has_message() ? ackMsg.message().c_str() : "(none)";
        LOG_Error("ParseReportAck reportCode:%u reason:%s message:%s", ackMsg.reportcode(), msgStr,
                  reasonStr, static_cast<int>(ackMsg.reason()));
    } while (0);

    // 报文异常，重连平台
    atomic_store(&g_mqttState, MQTT_STATE_ERROR);
    return ret;
}

/**
 * @brief 线程池通用线程处理模板
 *
 * @param arg
 */
static void *WorkPthread(void *arg)
{
    threadpool_t *pthreadPool = (threadpool_t *)arg;
    while (1)
    {
        pthread_mutex_lock(&pthreadPool->lock);

        // 从任务队列取出任务
        pthread_cond_wait(&pthreadPool->haveTask, &pthreadPool->lock);
        if (pthreadPool->destroyFlag)
        {
            pthread_mutex_unlock(&pthreadPool->lock);
            break;
        }
        tast_t task = pthreadPool->taskQueue[pthreadPool->readTaskIdx];
        pthreadPool->readTaskIdx = (pthreadPool->readTaskIdx + 1) % pthreadPool->queueCapacity;
        pthreadPool->taskCount--;
        pthread_cond_signal(&pthreadPool->haveIdle);

        pthread_mutex_unlock(&pthreadPool->lock);

        // 执行任务
        task.funcCallback(task.args);
    }

    return NULL;
}

/**
 * @brief 下行任务线程池创建
 *
 * @param threadCount 线程个数
 * @param taskCapacity 任务队列容量
 * @return threadpool_t*
 */
static threadpool_t *ThreadpoolCreate(int threadCount, int taskCapacity)
{
    threadpool_t *pthreadPool = (threadpool_t *)malloc(sizeof(threadpool_t));

    pthreadPool->threadCount = threadCount;
    pthreadPool->threads = (pthread_t *)malloc(sizeof(pthread_t) * threadCount);
    pthreadPool->destroyFlag = false;

    pthreadPool->queueCapacity = taskCapacity;
    pthreadPool->taskQueue = (tast_t *)malloc(sizeof(tast_t) * taskCapacity);
    pthreadPool->readTaskIdx = 0;
    pthreadPool->writeTaskIdx = 0;
    pthreadPool->taskCount = 0;

    for (size_t i = 0; i < threadCount; i++)
    {
        pthread_create(pthreadPool->threads, NULL, WorkPthread, pthreadPool);
    }

    pthread_mutex_init(&pthreadPool->lock, NULL);
    pthread_cond_init(&pthreadPool->haveIdle, NULL);
    pthread_cond_init(&pthreadPool->haveTask, NULL);
    return pthreadPool;
}

/**
 * @brief 下行任务销毁线程池
 *
 * @param pthreadPool
 */
static void ThreadpoolDestroy(threadpool_t *pthreadPool)
{
    if (!pthreadPool)
        return;

    pthread_mutex_lock(&pthreadPool->lock);
    pthreadPool->destroyFlag = true; // 标记销毁
    pthread_mutex_unlock(&pthreadPool->lock);

    // 广播给所有等待的线程
    pthread_cond_broadcast(&pthreadPool->haveTask);
    pthread_cond_broadcast(&pthreadPool->haveIdle);

    // 等待所有工作线程结束
    for (int i = 0; i < pthreadPool->threadCount; i++)
    {
        pthread_join(pthreadPool->threads[i], NULL);
    }

    // 释放资源
    free(pthreadPool->threads);
    free(pthreadPool->taskQueue);
    pthread_mutex_destroy(&pthreadPool->lock);
    pthread_cond_destroy(&pthreadPool->haveTask);
    pthread_cond_destroy(&pthreadPool->haveIdle);
    free(pthreadPool);
}

/**
 * @brief 向任务队列插入一条下行任务
 *
 * @param pthreadPool
 * @param funcCallback
 * @param taskArgs
 */
static int InsertTask(threadpool_t *pthreadPool, void (*funcCallback)(void *), TaskArgs *taskArgs)
{
    if (!pthreadPool || !funcCallback)
    {
        return RET_ERR;
    }

    pthread_mutex_lock(&pthreadPool->lock);

    if (pthreadPool->taskCount > pthreadPool->queueCapacity)
    {
        pthread_cond_wait(&pthreadPool->haveIdle, &pthreadPool->lock);
    }
    if (pthreadPool->destroyFlag)
    {
        pthread_mutex_unlock(&pthreadPool->lock);
        return RET_ERR;
    }

    pthreadPool->taskQueue[pthreadPool->writeTaskIdx].funcCallback = funcCallback;
    pthreadPool->taskQueue[pthreadPool->writeTaskIdx].args = taskArgs;
    pthreadPool->writeTaskIdx = (pthreadPool->writeTaskIdx + 1) % pthreadPool->queueCapacity;
    pthreadPool->taskCount++;
    pthread_cond_signal(&pthreadPool->haveTask);

    pthread_mutex_unlock(&pthreadPool->lock);

    return RET_OK;
}

/**
 * @brief 取消正在执行的任务
 *
 */
static void CancelTask(const std::string &cmdId)
{
    if (TaskIsRunning(cmdId.c_str()))
    {
        std::lock_guard<std::mutex> lock(g_cancelMutex);
        g_canceledTaskIds.insert(cmdId);
    }
    return;
}

/**
 * @brief 判断任务ID是否被取消
 * @param cmdId
 * @return
 */
bool IsCanceledTask(const char *cmdId)
{
    if (!cmdId)
        return false;
    std::lock_guard<std::mutex> lock(g_cancelMutex);
    return g_canceledTaskIds.count(cmdId) > 0;
}

/**
 * @brief 移除已取消的任务ID
 * @param cmdId
 */
void RemoveCanceledTask(const char *cmdId)
{
    if (!cmdId)
        return;
    std::lock_guard<std::mutex> lock(g_cancelMutex);
    g_canceledTaskIds.erase(cmdId);
}

// 待接收队列处理
// ******************************************************************************************************************************************************//

/**
 * @brief 向命令队列推送序列化后的命令数据（拷贝）
 * @param serialized_data 序列化后的 S3Command 二进制数据
 * @param size 数据长度
 */
void PushCommandToQueue(const void *serialized_data, size_t size)
{
    std::string data((const char *)serialized_data, size);
    {
        std::lock_guard<std::mutex> lock(g_commandQueueMutex);
        g_commandQueue.push(std::move(data));
    }
    g_commandQueueCV.notify_one();
}

/**
 * @brief 从命令队列阻塞取出一个命令
 * @param outSerialized 输出参数，存放取出的序列化数据
 * @param timeoutMs 超时毫秒（0表示阻塞直到有数据）
 * @return 成功为 0，超时或失败返回 -1
 */
static int PopCommandFromQueue(std::string &outSerialized, int timeoutMs)
{
    std::unique_lock<std::mutex> lock(g_commandQueueMutex);
    if (timeoutMs == 0)
    {
        g_commandQueueCV.wait(lock, []
                              { return !g_commandQueue.empty(); });
    }
    else
    {
        bool got = g_commandQueueCV.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                                             []
                                             { return !g_commandQueue.empty(); });
        if (!got)
            return -1;
    }
    outSerialized = std::move(g_commandQueue.front());
    g_commandQueue.pop();
    return 0;
}

// 线程实现
// ******************************************************************************************************************************************************//

/**
 * @brief 将待发队列 P0~P4 中的数据按优先级上报，等待并解析服务器 ACK 响应
 * @return RET_OK 所有数据上报成功，RET_ERR 发生不可恢复错误
 */
int DataReporting()
{
    LoadData_t loadData = {0};

    for (int idx = 0; idx <= 4; idx++)
    {
        if (GetQueueDataCnt(idx) > 0)
        {
            // 1. 从队列取出数据
            if (GetQueueData(&loadData, sizeof(LoadData_t), idx) != RET_OK)
            {
                LOG_Error("%s() GetQueueData err, break queue %d", __func__, idx);
                return RET_ERR;
            }

            if (loadData.len == 0)
            {
                LOG_Warn("%s() empty data in queue %d, skip", __func__, idx);
                continue;
            }
            LOG_DebugHex(loadData.data, loadData.len, "CLEARTEXT:");

            // 2. 构造 MQTT 负载数据
            SRMqttMessage_t reqMsg = SRMqttMessage_initializer;
            if (BuildReportMessage(reqMsg, loadData) != RET_OK)
            {
                LOG_Error("BuildReportMessage failed for priority %d", loadData.priority);
                return RET_ERR;
            }
            // LOG_DebugHex(reqMsg.payload, reqMsg.payloadLen, "CIPHERTEXT:");

#ifndef TEST_NOACK
            // 3. 发送 event/report/reissue 报文，并同步等待响应
            // 平台仅在数据异常时发送响应!!!
            SRMqttMessage_t rspMsg = SRMqttMessage_initializer;
            int ret = SRTspServiceExecute(&reqMsg, &rspMsg, 500);
            if (ret == RET_ERR)
            {
                // 发送失败，计入补发
                LOG_Error("SRTspServiceExecute failed for topic %s", reqMsg.topicName);
                CacheAdd(loadData, idx, 0);
                printf("$$$$$$$$$$ cached data from queue[%d] to P%d DB. data_len:%zu\n",
                       idx, idx, loadData.len);
                return RET_ERR;
            }
            else if (ret == RET_TIMEOUT)
            {
                // 发送的数据正常
                LOG_Debug("Report successfully sent, priority=%d, topic=%s, size=%zu",
                          loadData.priority, reqMsg.topicName, reqMsg.payloadLen);
                return RET_TIMEOUT;
            }

            // 4. 发送的数据异常——解析 ACK 响应
            CacheAdd(loadData, idx, 0); // 数据异常，计入补发
            if (ParseReportAckWithHandler(rspMsg) != RET_OK)
            {
                LOG_Error("ParseReportAckWithHandler failed for reqTopic:%s", reqMsg.topicName);
                return RET_ERR;
            }
#else
            SRTspServiceExecuteNoRes(&reqMsg);
            LOG_Debug("Report successfully sent, priority=%d, topic=%s, size=%zu",
                      loadData.priority, reqMsg.topicName, reqMsg.payloadLen);
            return RET_OK;
#endif
        }
    }
    return RET_OK;
}

/**
 * @brief 将待发队列P0~P3中的数据直接存入对应优先级的补发数据库
 */
void CatchDataToReissueDB()
{
    LoadData_t loadData = {0};
    for (int idx = 0; idx < 4; idx++)
    {
        while (GetQueueDataCnt(idx) > 0)
        {
            if (GetQueueData(&loadData, sizeof(LoadData_t), idx) == RET_OK)
            {
                if (loadData.len > 0)
                {
                    CacheAdd(loadData, idx, 0);
                    printf("$$$$$$$$$$ cached data from queue[%d] to P%d DB. data_len:%zu\n",
                           idx, idx, loadData.len);
                }
            }
            else
            {
                LOG_Error("%s() GetQueueData err", __func__);
                break;
            }
        }
    }
}

/**
 * @brief (重启类)命令ACK发送初始化
 *
 */
static void InitCmdAckSend()
{
    char commandId[3][128] = {""};
    bool restartAckFlag[3] = {false};
    restartAckFlag[0] = (InitStateFlagFromDisk(20, commandId[0], sizeof(commandId[0])) == EventStatus::EVENT_STATUS_TRIGGER);
#ifdef REBOOT_MCU_CARRY_CPU
    restartAckFlag[1] = (InitStateFlagFromDisk(21, commandId[1], sizeof(commandId[1])) == EventStatus::EVENT_STATUS_TRIGGER);
#endif
    restartAckFlag[2] = (InitStateFlagFromDisk(22, commandId[2], sizeof(commandId[2])) == EventStatus::EVENT_STATUS_TRIGGER);

    while (atomic_load(&g_mqttState) != MQTT_STATE_RECV)
    {
        // 等待MQTT处于连接状态后再上报重启类命令的回执
        usleep(100 * 1000);
    }

    for (int i = 0; i < 3; i++)
    {
        if (restartAckFlag[i])
        {
            std::string cmdId = commandId[i];
            HandlerRebootCmdAck(cmdId);
            ResetStatusFlagToDisk(20 + i);
        }
    }

    return;
}

// 线程
// ******************************************************************************************************************************************************//

/**
 * @brief 上行数据处理线程
 *
 * @param args
 * @return void*
 */
static void *UploadDataProcessing(void *args)
{
    prctl(PR_SET_NAME, "ReportData");
    long long lastTime, nowTime;
    lastTime = nowTime = (long long)time(NULL);

    while (1)
    {
        nowTime = (long long)time(NULL);
        if (nowTime - lastTime >= 1)
        {
            printf("============================== MqttStatus:%d\n", atomic_load(&g_mqttState));
            LOG_Debug("============================== MqttStatus:%d", atomic_load(&g_mqttState));
            lastTime = nowTime;
        }

        switch (atomic_load(&g_mqttState))
        {
        case MQTT_STATE_INIT:
        case MQTT_STATE_CONNECTING:
        case MQTT_STATE_CONNECTED:
        case MQTT_STATE_SUBSCRIBED:
        case MQTT_STATE_LOGIN:
        case MQTT_STATE_LOGOUT:
        case MQTT_STATE_ERROR:
        case MQTT_STATE_RECONNECT_WAIT:
        case MQTT_STATE_DEINIT:
            CatchDataToReissueDB();
            break;
        case MQTT_STATE_RECV:
            DataReporting();
            break;
        default:
            break;
        }

        usleep(100 * 1000);
    }
    return NULL;
}

/**
 * @brief 下行数据处理线程（命令下发处理）
 */
static void *CommandProcessing(void *args)
{
    prctl(PR_SET_NAME, "CmdExecute");
    curl_global_init(CURL_GLOBAL_DEFAULT);

    threadpool_t *pthreadPool = ThreadpoolCreate(5, 10);
    InitCmdAckSend();

    while (1)
    {
        // 从命令队列中阻塞取出一个序列化的 S3Command
        std::string serialized;
        if (PopCommandFromQueue(serialized, 0) != 0)
            continue; // 理论上不会失败，因为超时设为 0（无限等待）

        // 反序列化获取 cmdId 和 cmdType
        net::sirun::tboxs3::connect::command::S3Command command;
        if (!command.ParseFromString(serialized))
        {
            LOG_Error("CommandProcessing: parse failed");
            continue;
        }
        std::string cmdId = command.command_id();
        auto cmdType = command.downlink().command_type();
        LOG_Debug("CommandProcessing executing: id=%s type=%d", cmdId.c_str(), (int)cmdType);

        // 根据命令类型创建对应任务
        void (*taskThread)(void *) = NULL;
        switch (cmdType)
        {
        case net::sirun::tboxs3::connect::command::CommandType::TBOX_RESTART:
            taskThread = TboxRestartTaskThread;
            break;
        case net::sirun::tboxs3::connect::command::CommandType::MCU_RESTART:
            taskThread = McuRestartTaskThread;
            break;
        case net::sirun::tboxs3::connect::command::CommandType::CPU_RESTART:
            taskThread = CpuRestartTaskThread;
            break;
        case net::sirun::tboxs3::connect::command::CommandType::LOG_UPLOAD:
            taskThread = LogUploadTaskThread;
            break;
        case net::sirun::tboxs3::connect::command::CommandType::CANCEL_TASK:
            CancelTask(cmdId);
            continue;
            break;
        default:
            SendCommandResponse(cmdId, net::sirun::tboxs3::connect::command::ControlResult::refused_unknown, -1, "Unsupported");
            continue;
            break;
        }

        int ret = RET_ERR;
        TaskArgs *taskArgs = new TaskArgs;
        taskArgs->cmdId = cmdId;
        taskArgs->params = command.downlink().log_params();
        char snBuff[17] = "";
        ParamInfo_t paramInfo = {.id = E_ParamId_SN};
        if (GetParam(&paramInfo) == RET_OK)
        {
            memcpy(snBuff, paramInfo.value, strlen((char *)paramInfo.value));
            taskArgs->sn = snBuff;
            ret = RET_OK;
        }

        if (ret == RET_ERR || InsertTask(pthreadPool, taskThread, taskArgs) != RET_OK)
        {
            SendCommandResponse(cmdId, net::sirun::tboxs3::connect::command::ControlResult::command_failed, -1, "Failed to create task");
        }
    }

    // 线程池销毁
    ThreadpoolDestroy(pthreadPool);
    curl_global_cleanup();
    return NULL;
}

/**
 * @brief 开始数据处理
 *
 * @return int
 */
int DataProcessingStart()
{
    int ret = -1;
    MonitorConfigPath cfgPath;
    MonitorConfig_Get(&cfgPath);

    ret = pthread_create(&g_uploadData, NULL, UploadDataProcessing, NULL);
    if (ret < 0)
    {
        LOG_Error("pthread_create err!");
        return RET_ERR;
    }

    ret = pthread_create(&g_cmdExecution, NULL, CommandProcessing, NULL);
    if (ret < 0)
    {
        LOG_Error("pthread_create err!");
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @brief 结束数据处理
 *
 * @return int
 */
int DataProcessingStop()
{
    int ret;
    // if (!g_isInit.load())
    // {
    //     LOG_Error("DataProcessingStop failed: client not initialized\n");
    //     return RET_ERR;
    // }

    if (g_uploadData != 0)
    {
        ret = pthread_join(g_uploadData, NULL);
        if (ret != 0)
        {
            LOG_Error("Failed to join <StartRunStatusMonitoring> thread, ret = %d", ret);
            return RET_ERR;
        }
    }

    if (g_cmdExecution != 0)
    {
        int ret = pthread_join(g_cmdExecution, NULL);
        if (ret != 0)
        {
            LOG_Error("Failed to join <ReissueDataProcessing> thread, ret = %d", ret);
            return RET_ERR;
        }
    }

    return RET_OK;
}
