#ifndef _TERMINAL_MANAGEMENT_DATA_RESPONSE_H
#define _TERMINAL_MANAGEMENT_DATA_RESPONSE_H

#include <string>
#include <cstdint>
#include "net/sirun/tboxs3/connect/command.pb.h"
typedef struct
{
    std::string cmdId;
    net::sirun::tboxs3::connect::command::LogCollectParams params;
    std::string sn;
    // LogCollectParams params;
    // ... 其他参数
} TaskArgs; // 任务线程的参数结构

// #define REBOOT_MCU_CARRY_CPU // 单独重启MCU会复位CPU的问题存在

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 向平台发送命令执行结果
     */
    void SendCommandResponse(const std::string &cmdId,
                             net::sirun::tboxs3::connect::command::ControlResult result,
                             int32_t errCode,
                             const std::string &errLog);

    /**
     * @brief 重启类命令在完成重启后重新上点时发送的ACK回执
     *
     * @param cmdId
     */
    void HandlerRebootCmdAck(const std::string &cmdId);

    /**
     * @brief MCU重启线程入口
     *
     * @param arg
     * @return void*
     */
    void McuRestartTaskThread(void *arg);

    /**
     * @brief CPU重启线程入口
     *
     * @param arg
     * @return void*
     */
    void CpuRestartTaskThread(void *arg);

    /**
     * @brief TBOX重启线程入口
     *
     * @param arg
     * @return void*
     */
    void TboxRestartTaskThread(void *arg);

    /**
     * @brief 日志上传线程入口
     *
     * @param arg
     */
    void LogUploadTaskThread(void *arg);

#ifdef __cplusplus
}
#endif

#endif // _TERMINAL_MANAGEMENT_DATA_RESPONSE_H
