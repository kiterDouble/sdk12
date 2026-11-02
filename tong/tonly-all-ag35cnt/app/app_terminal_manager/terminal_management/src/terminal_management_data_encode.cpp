#include <cmath>
#include <stdio.h>
#include "terminal_management_data_encode.h"

/**
 * @brief 编码S3ConnectInfo
 * @param connectMsg
 * @param tboxDbInfo
 */
void EncodeS3Connect(net::sirun::tboxs3::connect::S3Connect *connectMsg, const MonitorTboxDbInfo_t &tboxDbInfo)
{
    if (!connectMsg)
        return;

    // TBoxInfo
    auto *info = connectMsg->mutable_info();
    info->set_sn(tboxDbInfo.sn);
    info->set_vin(tboxDbInfo.vin);
    info->set_iccid(tboxDbInfo.iccid);
    info->set_imei(tboxDbInfo.imei);
    info->set_sim_no(tboxDbInfo.imsi);
    info->set_project_name(tboxDbInfo.projectName);
    // info->set_activation(net::sirun::tboxs3::common::TBoxActivationStaTBoxs::unknowTBoxActivation);

    // TBoxVersion
    auto *version = connectMsg->mutable_version();
    version->set_hwversion(tboxDbInfo.HwVersion);
    version->set_swversionmain(tboxDbInfo.SwVersion);
    version->set_swversionsecondary(tboxDbInfo.srSwVersion);
    version->set_soft_update_time(0); // 可根据需要改为从 BUILD_INFO 读取
    // version->set_swversionconfiguration("1.0.1"); // 固定配置版本

    connectMsg->set_configversion(1);
}

/**
 * @brief 编码StaticInfo
 *
 * @param staticInfo
 * @param tboxDbInfo
 */
void EncodeStaticInfo(net::sirun::tboxs3::report::StaticInfo *staticInfo, const MonitorTboxDbInfo_t tboxDbInfo)
{
    if (!staticInfo)
        return;

    // TBoxInfo
    auto *info = staticInfo->mutable_info();
    info->set_sn(tboxDbInfo.sn);                    // 设备序列号(SN)
    info->set_vin(tboxDbInfo.vin);                  // 车辆识别码(VIN)
    info->set_iccid(tboxDbInfo.iccid);              // SIM卡唯一标识(ICCID)
    info->set_imei(tboxDbInfo.imei);                // 国际移动设备识别码(IMEI)
    info->set_sim_no(tboxDbInfo.imsi);              // SIM卡号(此处使用IMSI，如有问题后续修改)
    info->set_project_name(tboxDbInfo.projectName); // 项目名称
    // info->set_activation(net::sirun::tboxs3::common::TBoxActivationStaTBoxs::unknowTBoxActivation);

    // TBoxVersion
    auto *version = staticInfo->mutable_version();
    version->set_hwversion(tboxDbInfo.HwVersion);            // 硬件版本号
    version->set_swversionmain(tboxDbInfo.SwVersion);        // 客户软件主版本
    version->set_swversionsecondary(tboxDbInfo.srSwVersion); // 内部软件版本
    version->set_soft_update_time(0);                        // 软件升级时间戳(无升级时填0)
    // version->set_swversionconfiguration("1.0.1");

    staticInfo->set_configversion(1); // 配置版本号
}

/**
 * @brief 填充工作状态 (WorkStatus)，包括工作模式和模式改变时间
 *
 * @param work 指向待填充的 WorkStatus 消息的指针
 */

/**
 * @brief 编码WorkStatus
 *
 * @param work
 * @param mcuStatusData
 */
void EncodeWorkStatus(net::sirun::tboxs3::report::WorkStatus *work, const MonitorMcuStatusData_t mcuStatusData)
{
    if (!work)
        return;

    work->set_mode(static_cast<net::sirun::tboxs3::report::WorkMode>(mcuStatusData.workMode)); // 工作模式(唤醒源)
    work->set_changingtime(mcuStatusData.powerModeChangeTime);                                 // 工作模式改变时刻(Unix时间戳)
}

/**
 * @brief 编码ExternalConnect
 *
 * @param ext
 * @param rawData
 */
void EncodeExternalConnect(net::sirun::tboxs3::report::ExternalConnect *ext, const MonitoringData_t rawData)
{
    if (!ext)
        return;

    // 定位信息 Position
    auto *position = ext->mutable_position();
    auto *basic_pos = position->mutable_basicpos();
    const GnssLocation_t *loc = &rawData.positioningData.locationInfo;
    basic_pos->set_latitude(static_cast<int32_t>(loc->flatitude * 1e7));   // 纬度（度 * 1e7，北纬为正，南纬为负）
    basic_pos->set_longitude(static_cast<int32_t>(loc->flongitude * 1e7)); // 经度（度 * 1e7，东经为正，西经为负）

    auto *ext_pos = position->mutable_extpos();
    ext_pos->set_altitudemtrx10(static_cast<int32_t>(loc->seaHeight * 10)); // 海拔高度，单位0.1米
    ext_pos->set_headingdeg(static_cast<int32_t>(loc->direction * 0.1));    // 航向角，单位0.1度
    ext_pos->set_gnssspeedkmph(static_cast<int32_t>(loc->speed * 0.1));     // 地面速度，单位0.1 km/h
    ext_pos->set_fixtime(loc->timestamp);                                   // 定位时刻(Unix时间戳)

    position->set_usesexternalgnss(static_cast<net::sirun::tboxs3::common::BOOLEXT>(rawData.positioningData.usesexternalgnss)); // 是否使用外置GNSS天线

    auto *reliability = position->mutable_reliability();
    reliability->set_fixtype(static_cast<net::sirun::tboxs3::common::GnssFixType>(loc->gpsState & 0x0F));
    reliability->set_noofsatellites(loc->num_svs); // 可见卫星数量

    // 网络信息 Network
    auto *network = ext->mutable_network();
    const MonitorNetStatus_t *net = &rawData.netInfo;
    network->set_carrier(static_cast<net::sirun::tboxs3::common::CarrierType>(net->carrierType)); // 运营商(1:联通,2:电信,3:移动)
    network->set_network(static_cast<net::sirun::tboxs3::common::NetworkType>(net->networkType)); // 网络制式(2:GSM,4:LTE等)
    network->set_dbm(-113 + (net->csq * 2));                                                      // 信号强度(dBm)，由CSQ简单换算

    // 电源信息 Power (TboxBatteryInfo)
    auto *power = ext->mutable_power();
    const MonitorMcuStatusData_t *mcuStatus = &rawData.mcuStatusData;
    power->set_power(static_cast<net::sirun::tboxs3::common::TBoxBatterySource>(mcuStatus->batterySource)); // 供电来源(主电/备电)
    auto *battery = power->mutable_battery();
    battery->set_mainbatteryvoltx10(static_cast<int32_t>(mcuStatus->vehicleBatteryVoltage * 10)); // 整车蓄电池电压，单位0.1V
    battery->set_primaryvoltx10(static_cast<int32_t>(mcuStatus->backupBatteryVoltage * 10));      // 备用电池电压，单位0.1V

    // 车辆状态 (VehicleState)
    // ext->set_vehiclestate(static_cast<net::sirun::tboxs3::report::VehicleState>(rawData.vehicleStatus)); // 车辆综合状态(充电/行驶/升级等)
}

/**
 * @brief 编码InnerHealth
 *
 * @param health
 * @param rawData
 */
void EncodeInnerHealth(net::sirun::tboxs3::report::InnerHealth *health, const MonitoringData_t rawData)
{
    if (!health)
        return;

    auto *diag = health->mutable_diagnostics();
    // GNSS天线状态：0正常为functioning，否则为broken
    diag->set_gnssantenna(rawData.positioningData.locationInfo.antStatus == 0
                              ? net::sirun::tboxs3::common::TBoxDiagnostic::functioning
                              : net::sirun::tboxs3::common::TBoxDiagnostic::broken);

    // 软件运行状态 (SoftRunningStatus)
    auto *soft = health->mutable_softrunningstatus();
    soft->set_cpuusage(std::round(rawData.cpuStatusData.cpuUsage * 10.0f) / 10.0f);                // CPU使用率(%)
    soft->set_memoryusage(static_cast<uint64_t>(std::llround(rawData.cpuStatusData.memoryUsage))); // 内存使用率(%)
    soft->set_diskusage(static_cast<uint32_t>(std::llround(rawData.cpuStatusData.diskUsage)));     // 磁盘使用率(%)
    soft->set_diskfree(rawData.cpuStatusData.diskFreeSpaceKB);                                     // 磁盘剩余空间(KB)
}

/**
 * @brief 编码LogReporter
 *
 * @param logReporter
 * @param rawData
 */
void EncodeLogReporter(net::sirun::tboxs3::common::LogReporter *logReporter, const MonitoringData_t rawData)
{
    if (!logReporter)
        return;

    // 暂无数据，可留空或根据业务填充
    (void)logReporter;
}
