#ifndef __UPGRADE_INTERFACE_H__
#define __UPGRADE_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

// 包含必要的头文件
#include <stdint.h>
#include "ota_api.h"  // 包含OTA API头文件以获取UpgradeFlag_E定义

#define   MD5DATA                           "/oemdata/upgrade/md5.txt"
#define MCU1BINFILEPATH                     "/oemdata/upgrade/mcu_1.bin"
#define MCU2BINFILEPATH                     "/oemdata/upgrade/mcu_2.bin"
#define MPUBINFILEPATHMPUBIN                "/oemdata/upgrade/update.zip"
#define BT1BINFILEPATH                     "/oemdata/upgrade/bt_1.bin"
#define BT2BINFILEPATH                     "/oemdata/upgrade/bt_2.bin"

// 并发升级接口声明
int32_t ConcurrentOtaStartUpgrade(const char *filePath, UpgradeFlag_E upgradeFlag);
int32_t ConcurrentOtaGetUpgradeStatus(const char *filePath, UpgradeStatus_E *status, UpgradeFlag_E flag);
int32_t ConcurrentOtaStopUpgrade(UpgradeFlag_E flag);

// MCU升级函数（带重试机制）
int LordMcuUpgrade();

// MPU主模组升级函数
int LordMpuMainUpgrade();

#ifdef __cplusplus
}
#endif
#endif