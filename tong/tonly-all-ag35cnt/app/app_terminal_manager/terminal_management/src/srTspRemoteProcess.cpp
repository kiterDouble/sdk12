//
// Created by Jaxsen on 2025/4/28.
//
#include <string.h>
#include <stdlib.h>
#include <unordered_map>
#include "srTspRemoteProcess.h"
#include "sr_log.h"
#include "srTspFWMqttClient.h"
#include "terminal_management_data_report.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include "terminal_management_data_response.h"

// 平台已下发的命令ID
static std::unordered_set<std::string> g_taskCmdIds;
static std::mutex g_cmdIdMutex;

static int TspRemoteCommandHandler(SRMqttMessage_t *msg)
{
    if (!msg)
        return RET_ERR;

    net::sirun::tboxs3::connect::command::S3Command command;
    if (!command.ParseFromArray(msg->payload, msg->payloadLen))
    {
        LOG_Error("Parse S3Command failed");
        return RET_ERR;
    }

    // 检查任务ID是否重复（取消命令除外）
    std::string cmdId = command.command_id();
    LOG_Debug("taskIdString: %s", cmdId.c_str());
    auto cmdType = command.downlink().command_type();
    if (cmdType != net::sirun::tboxs3::connect::command::CommandType::CANCEL_TASK)
    {
        std::lock_guard<std::mutex> lock(g_cmdIdMutex);
        if (g_taskCmdIds.count(cmdId))
        {
            LOG_Error("command ID is repeated: %s", cmdId.c_str());
            return RET_ERR;
        }
        g_taskCmdIds.insert(cmdId);
    }

    // 命令推送给执行线程
    std::string serialized;
    if (!command.SerializeToString(&serialized))
    {
        LOG_Error("Serialize command failed");
        SendCommandResponse(cmdId, net::sirun::tboxs3::connect::command::ControlResult::command_failed,
                            0, "SerializeToString fail");
        if (cmdType != net::sirun::tboxs3::connect::command::CommandType::CANCEL_TASK)
        {
            std::lock_guard<std::mutex> lock(g_cmdIdMutex);
            g_taskCmdIds.erase(cmdId);
        }
        return RET_ERR;
    }
    PushCommandToQueue(serialized.data(), serialized.size());
    LOG_Debug("Command pushed to worker thread: id=%s", cmdId.c_str());

    // 立即回复"已接收命令"
    SendCommandResponse(cmdId, net::sirun::tboxs3::connect::command::ControlResult::command_received,
                        0, "Command accepted");
    return RET_OK;
}

bool TaskIsRunning(const char *cmdId)
{
    if (!cmdId)
        return false;
    std::lock_guard<std::mutex> lock(g_cmdIdMutex);
    if (g_taskCmdIds.count(cmdId))
        return true;
    else
        return false;
}

void RemoveTaskCmdId(const char *cmdId)
{
    if (!cmdId)
        return;
    std::lock_guard<std::mutex> lock(g_cmdIdMutex);
    g_taskCmdIds.erase(cmdId);
}

int TspRemoteProcess(SRMqttMessage_t *msg)
{
    if (nullptr == msg)
    {
        LOG_Error("%s input argv is invalid.", __FUNCTION__);
        return RET_ERR;
    }

    if (strncmp(msg->topicName, SR_TOPIC_PREFIX_S2T_COMMAND_PLATFORM, strlen(SR_TOPIC_PREFIX_S2T_COMMAND_PLATFORM)) == RET_OK)
    {
        return TspRemoteCommandHandler(msg);
    }
    return RET_ERR;
}
