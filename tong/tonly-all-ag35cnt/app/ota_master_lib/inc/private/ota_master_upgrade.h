/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-03-11
 * Description: OTA Master升级&回滚模块
 *********************************************************/
#ifndef _OTA_MASTER_UPGRADE_H
#define _OTA_MASTER_UPGRADE_H

#include <stdint.h>
#include <unordered_map>
#include <functional>
#include "ota_master_config.h"
#include "ota_master_inner_code.h"
#include "ota_master_data.h"

/**
 * @brief  是否OTA升级超时回调函数
 * 
 * @return 是否OTA升级超时
 */
using IsOtaUpgradeTimeoutFunc = std::function<bool(void)>;

/**
 * @brief  升级目标ECU
 *
 * @param[inout] ecuConfig ECU任务配置信息
 * @param[out] error 错误码
 * @param[out] isTboxNeedReboot TBOX是否需要重启
 * @param[in] isOtaUpgradeTimeout 是否OTA升级超时回调函数
 *
 * @return 升级状态
 */
OtaEcuUpgradeStatus UpgradeTargetEcu(OtaTaskEcuConfigInfo &ecuConfig, OtaUpgradeErrorCode &error,
                                     bool &isTboxNeedReboot, IsOtaUpgradeTimeoutFunc isOtaUpgradeTimeout);

/**
 * @brief  优先回滚控制高压电的ECU（BCM、VCU、IPU、BMS）
 *         这些ECU升级失败时无需上下高压直接回滚
 *
 * @param[inout] taskEcuConfig OTA任务配置信息
 *
 * @return 回滚状态
 */
OtaEcuRollbackStatus RollbackPowerControlEcusFirst(OtaTaskConfigInfo &taskEcuConfig);

/**
 * @brief 回滚目标ECU
 * 
 * @param[in] ecuConfig ECU配置信息
 * @param[out] isTboxNeedReboot TBox是否需要重启
 * 
 * @return 回滚状态
 */
OtaEcuRollbackStatus RollbackTargetEcu(OtaTaskEcuConfigInfo &ecuConfig, bool &isTboxNeedReboot);

/**
 * 通知TBox自升级结果
 *
 * @param[in] result    升级结果：OTA_RET_OK：升级成功；OTA_RET_ERR：升级失败
 * @param[in] rebootState 是否需要TBox重启
 *
 * @return 无
 */
void NoticeTboxSelfUpgradeResult(int result, OtaMasterTboxRebootState rebootState);

/**
 * 通知TBox回滚结果
 *
 * @param[in] result    回滚结果：OTA_RET_OK：回滚成功；OTA_RET_ERR：回滚失败
 * @param[in] rebootState 是否需要TBox重启
 *
 * @return 无
 */
void NoticeTboxRollbackResult(int result, OtaMasterTboxRebootState rebootState);
#endif // _OTA_MASTER_UPGRADE_H