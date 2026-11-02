#ifndef _TERMINAL_MANAGEMENT_DATA_ENCODE_H
#define _TERMINAL_MANAGEMENT_DATA_ENCODE_H

#include "net/sirun/tboxs3/connect/report.pb.h"
#include "net/sirun/tboxs3/connect/common.pb.h"
#include "net/sirun/tboxs3/connect/event_report.pb.h"
// #include "connect.pb.h"
#include "net/sirun/tboxs3/connect/device_status.pb.h"

// #include "sr_param.h"
// #include "pm_server_api.h"
#include "terminal_management_data_monitor.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 编码S3Connect
     * @param connectMsg
     * @param tboxDbInfo
     */
    void EncodeS3Connect(net::sirun::tboxs3::connect::S3Connect *connectMsg, const MonitorTboxDbInfo_t &tboxDbInfo);

    /**
     * @brief 编码StaticInfo
     *
     * @param staticInfo
     * @param tboxDbInfo
     */
    void EncodeStaticInfo(net::sirun::tboxs3::report::StaticInfo *staticInfo, const MonitorTboxDbInfo_t tboxDbInfo);

    /**
     * @brief 编码WorkStatus
     *
     * @param work
     * @param mcuStatusData
     */
    void EncodeWorkStatus(net::sirun::tboxs3::report::WorkStatus *work, const MonitorMcuStatusData_t mcuStatusData);

    /**
     * @brief 编码ExternalConnect
     *
     * @param ext
     * @param rawData
     */
    void EncodeExternalConnect(net::sirun::tboxs3::report::ExternalConnect *ext, const MonitoringData_t rawData);

    /**
     * @brief 编码InnerHealth
     *
     * @param health
     * @param rawData
     */
    void EncodeInnerHealth(net::sirun::tboxs3::report::InnerHealth *health, const MonitoringData_t rawData);

    /**
     * @brief 编码LogReporter
     *
     * @param logReporter
     * @param rawData
     */
    void EncodeLogReporter(net::sirun::tboxs3::common::LogReporter *logReporter, const MonitoringData_t rawData);

#ifdef __cplusplus
}
#endif

#endif // _TERMINAL_MANAGEMENT_DATA_ENCODE_H
