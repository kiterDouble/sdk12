#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include "err_code.h"
#include "sr_log.h"
#include "ota_api.h"
#include "sr_ipc.h"
#include "pm_server_api.h"

/* 升级包解压目录 */
#define DECOMPRESS_DIR "/sdcard/selfUpgrade/decompress/"

/**
 * @brief TBOX自升级守护进程
 *
 * @param[in] arg 传递升级标志，升级标志: 1只升级MCU，2只升级MPU，3同时升级MCU和MPU
 * @return 自升级成功返回0，参数有误返回1，自升级启动失败返回2，自升级失败返回3
 */
int main(int argc, char *argv[])
{
    UpgradeStatus_E upgradeSta = -1;
    LOG_INIT("otaDaemon");

    if(argc < 2)
    {
        LOG_Error("uesage: otaDaemon <1 :升级MCU, 2升级CPU, 3 全部升级>");
        return 1;
    }
    if(atoi(argv[1]) < UPGRADE_MCU_FLAG || atoi(argv[1]) > UPGRADE_ALL_FLAG)
    {
        LOG_Error("argv is invalid <1 :升级MCU, 2升级CPU, 3 全部升级>");
        return 1;
    }
    UpgradeFlag_E upgradeFlag = (UpgradeFlag_E)atoi(argv[1]);
    StartQueueMsgRcvThread("upgrade");
    // 阻止PM模块因MCU心跳丢失触发强制休眠
    PmSetSleepEvent(PM_SLEEP_EVENT_UPGRADE);
    // 获取升级状态
    OtaGetUpgradeStatus(DECOMPRESS_DIR, &upgradeSta);
    LOG_Debug("upgrade status : %d, upgrade flag : %d", upgradeSta,upgradeFlag);
    // 执行升级前的操作
    if (UPGRADE_MPU_SUCCESS != upgradeSta && UPGRADE_MPU_FAILED != upgradeSta && UPGRADE_INIT != upgradeSta) {
        OtaRestoreAOrBPartition();
    }
    // 开始自升级
    if(OtaStartUpgrade(DECOMPRESS_DIR, upgradeFlag) != RET_OK) {
        LOG_Error("Tbox self upgrade start failed.");
        PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);
        return 2;
    }
    while(1)
    {
        sleep(1);
        // 更新升级状态
        OtaGetUpgradeStatus(DECOMPRESS_DIR, &upgradeSta);
        LOG_Debug("upgrade status : %d", upgradeSta);
        // MPU失败, 无论是否同时升级MCU，MPU失败都会导致升级线程退出
        if (UPGRADE_MPU_FLAG == (upgradeFlag & UPGRADE_MPU_FLAG) &&
            UPGRADE_MPU_FAILED == upgradeSta) {
            break;
        }
        // 仅升级MPU且成功（同时升级MPU+MCU时需等待MCU状态, 不能退出）
        if (UPGRADE_MPU_FLAG == upgradeFlag &&
            UPGRADE_MPU_NEED_BACKUP == upgradeSta) {
            break;
        }

        // 只升级MCU 或者 同时升级MPU和MCU
        if (UPGRADE_MCU_FLAG == (upgradeFlag & UPGRADE_MCU_FLAG) &&
            (UPGRADE_MCU_NEED_RESET == upgradeSta || UPGRADE_MCU_FAILED == upgradeSta ||
            UPGRADE_MCU_SUCCESS == upgradeSta)) {
            break;
}
    }
    // 只升级MPU且升级失败 或者 MPU与MCU同时升级且MCU升级失败，还原MPU非活跃分区
    if (UPGRADE_MPU_FAILED == upgradeSta ||
        (UPGRADE_MCU_FAILED == upgradeSta && UPGRADE_ALL_FLAG == upgradeFlag)) {
        LOG_Error("upgrade failed, status : %d", upgradeSta);
        PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);
        return 3;
    }
    // 只升级MPU且升级成功 或者 MPU与MCU同时升级且全部升级成功
    // 注意：1、仅升级MPU的情况下：切换MPU分区后，需要主动reboot
    //       2、同时升级MPU和MCU的情况下：切换分区后，需要主动reboot
    //       3、仅升级MCU的情况下：MPU无需reboot
    if (UPGRADE_MCU_NEED_RESET == upgradeSta && UPGRADE_MCU_FLAG == upgradeFlag){
        // 仅升级MCU成功无需切换分区
        LOG_Debug("[OTA_DEBUG] upgrade success, status=%d, flag=%d, MCU will reset MPU, daemon exit", upgradeSta, upgradeFlag);
    }    
    else if ((UPGRADE_MPU_NEED_BACKUP == upgradeSta && UPGRADE_MPU_FLAG == upgradeFlag) ||
        (UPGRADE_MCU_NEED_RESET == upgradeSta && UPGRADE_ALL_FLAG == upgradeFlag) ||
        (UPGRADE_MCU_SUCCESS == upgradeSta && UPGRADE_ALL_FLAG == upgradeFlag)) {
        LOG_Debug("[OTA_DEBUG] upgrade success, switching partition");
        if (OtaSwitchAOrBPartition() != RET_OK) {
            LOG_Error("Switch partition failed!");
            PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);
            return 3;
        }
        // 只要升级MPU，切换分区后都需要主动重启
        LOG_Debug("[OTA_DEBUG] MPU upgrade success, rebooting MPU");
        PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);
        sleep(2);
        system("reboot");
        return 0;
    }
    LOG_Debug("[OTA_DEBUG] otaDaemon exit, status=%d, flag=%d", upgradeSta, upgradeFlag);
    // 升级结束，清除睡眠事件，允许系统正常休眠
    PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);
    return 0;
}
