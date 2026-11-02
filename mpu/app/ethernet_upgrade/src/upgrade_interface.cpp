#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "sr_log.h"
#include "ota_api.h"
#include "mcuinte.h"
#include "upgrade_interface.h"
#include "err_code.h"

int GetSystemRetValue(const char *cmd)
{
    pid_t status;
    if(NULL == cmd)
    {
        return RET_ERR;
    }
    LOG_Debug("%s exe start", cmd);
    status = system(cmd);
    if(-1 == status)
    {
        return RET_ERR;
    }
    if(!WIFEXITED(status))
    {
        return RET_ERR;
    }
    return WEXITSTATUS(status);
}
int LordMcuUpgrade() {
    UpgradeStatus_E upgradeSta = UPGRADE_INIT;
    time_t start_time, current_time;
    
    LOG_Debug("Starting MCU upgrade process...");
    printf("Starting MCU upgrade process...\n");
    
    McuSrvSetSendLock(1);
    StartQueueMsgRcvThread("upgrade_mcu");
    OtaStartUpgrade("/oemdata/upgrade/",UPGRADE_MCU_FLAG);
    time(&start_time);
    
    // 监控升级过程
    while(1) {
        OtaGetUpgradeStatus("/oemdata/upgrade/", &upgradeSta);
        printf("====UPGRADE_MCU_FLAG ,upgradeSta :%d \n",upgradeSta);
        if(UPGRADE_MCU_SUCCESS == upgradeSta) {
            LOG_Debug("MCU upgrade completed successfully");
            printf("MCU upgrade completed successfully\n");
            McuSrvSetSendLock(0);
            return 0;
        }
        
        if(UPGRADE_MCU_FAILED == upgradeSta) {
            LOG_Error("MCU upgrade failed with status: %d", upgradeSta);
            printf("MCU upgrade failed with status: %d\n", upgradeSta);
            McuSrvSetSendLock(0);
            return -1;
        }
        
        time(&current_time);
        if(difftime(current_time, start_time) >= 300) {
            LOG_Error("MCU upgrade timeout (5 minutes), status: %d", upgradeSta);
            McuSrvSetSendLock(0);
            return -1;
        }
        
        sleep(3);
    }
}

// MPU主模组升级函数（使用并发接口）
int LordMpuMainUpgrade() {
    char cmd[128] = {0};
    int ret = 0;
    UpgradeStatus_E upgradeSta = UPGRADE_INIT;
    time_t start_time, current_time;
    
    LOG_Debug("Starting MPU main upgrade process...");
    printf("Starting MPU main upgrade process...\n");
    
    StartQueueMsgRcvThread("upgrade_mpu");
    // 使用新的并发升级接口
    ret = ConcurrentOtaStartUpgrade("/oemdata/upgrade/", UPGRADE_MPU_FLAG);
    if(ret < 0) {
        LOG_Error("ConcurrentOtaStartUpgrade MPU main failed: %d", ret);
        printf("ConcurrentOtaStartUpgrade MPU main failed: %d\n", ret);
        return -1;
    }
    Set_upgrade_mode(1);
    time(&start_time);
    system("echo 65 > /oemdata/parameters/MainModemUpProgress");
    while(1) {
        printf("---------UPGRADE_MPU_FLAG ,upgradeSta :%d \n",upgradeSta);
        
        // 使用新的原子获取接口
        UpgradeStatus_E mcuStatus, mpuStatus;
        ret = ConcurrentOtaGetAllUpgradeStatus("/oemdata/upgrade/", &mcuStatus, &mpuStatus);
        if(ret == RET_OK) {
            upgradeSta = mpuStatus; // 使用MPU状态
            // printf("Atomic status check - MCU: %d, MPU: %d\n", mcuStatus, mpuStatus);
        } else {
            LOG_Error("Failed to get atomic upgrade status, fallback to individual calls");
            // 降级到单独获取
            ret = ConcurrentOtaGetUpgradeStatus("/oemdata/upgrade/", &upgradeSta, UPGRADE_MPU_FLAG);
        }
        
        if(ret < 0) {
            LOG_Error("Failed to get MPU upgrade status");
            return -1;
        }
        
        if(UPGRADE_MPU_NEED_BACKUP == upgradeSta) {
            LOG_Debug("MPU main upgrade completed successfully");
            printf("MPU main upgrade completed successfully\n");
            system("echo \"2\" > /oemdata/tmp/tbox_upgrade_status.txt");
            return 0;
        }
        
        if(UPGRADE_MPU_FAILED == upgradeSta) {
            LOG_Error("MPU main upgrade failed with status: %d", upgradeSta);
            printf("MPU main upgrade failed with status: %d\n", upgradeSta);

            memset(cmd, 0, sizeof(cmd));
            sprintf(cmd, "ql_cmd sysinfo /oemdata/log;");
            int ret = GetSystemRetValue(cmd);
            if(-1 == ret)
            {
                LOG_Error("ql_cmd sysinfo %d, cmd : %s", ret, cmd);

            }
            
            return -1;
        }
        
        time(&current_time);
        if(difftime(current_time, start_time) >= 600) {
            LOG_Error("MPU main upgrade timeout (15 minutes), status: %d", upgradeSta);
            memset(cmd, 0, sizeof(cmd));
            sprintf(cmd, "ql_cmd sysinfo /oemdata/log;");
            int ret = GetSystemRetValue(cmd);
            if(-1 == ret)
            {
                LOG_Error("ql_cmd sysinfo %d, cmd : %s", ret, cmd);

            }
        
            return -1;
        }
        
        sleep(3);
    }
    
    return -1;
}