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
// MCU升级函数（使用并发接口）
int LordMcuUpgrade() {
    int ret = 0;
    UpgradeStatus_E upgradeSta = UPGRADE_INIT;
    time_t start_time, current_time;
    int retryCount = 0;
    const int MAX_RETRY = 3;
    const int RETRY_INTERVAL = 5; // 5秒间隔
    
    LOG_Debug("Starting MCU upgrade process with retry mechanism...");
    printf("Starting MCU upgrade process with retry mechanism...\n");
    
    // 最多重试3次
    while (retryCount < MAX_RETRY) {
        retryCount++;
        LOG_Debug("MCU upgrade attempt %d/%d", retryCount, MAX_RETRY);
        printf("MCU upgrade attempt %d/%d\n", retryCount, MAX_RETRY);
        
        McuSrvSetSendLock(1);
        StartQueueMsgRcvThread("upgrade_mcu");
        OtaStartUpgrade("/oemdata/upgrade/",UPGRADE_MCU_FLAG);
        // 使用新的并发升级接口
        // ret = ConcurrentOtaStartUpgrade("/oemdata/upgrade/", UPGRADE_MCU_FLAG);
        // if(ret < 0) {
        //     McuSrvSetSendLock(0);
        //     LOG_Error("ConcurrentOtaStartUpgrade MCU failed: %d, attempt %d", ret, retryCount);
        //     printf("ConcurrentOtaStartUpgrade MCU failed: %d, attempt %d", ret, retryCount);
        //     // 如果不是最后一次尝试，等待后重试
        //     if (retryCount < MAX_RETRY) {
        //         LOG_Debug("Waiting %d seconds before retry...", RETRY_INTERVAL);
        //         printf("Waiting %d seconds before retry...\n", RETRY_INTERVAL);
        //         sleep(RETRY_INTERVAL);
        //         continue;
        //     } else {
        //         LOG_Error("MCU upgrade failed after %d attempts", MAX_RETRY);
        //         printf("MCU upgrade failed after %d attempts\n", MAX_RETRY);
        //         return -1;
        //     }
        // }
        
        time(&start_time);
        int upgradeTimeout = 0;
        
        // 监控单次升级过程
        while(1) {
            OtaGetUpgradeStatus("/oemdata/upgrade/", &upgradeSta);
            printf("====UPGRADE_MCU_FLAG ,upgradeSta :%d \n",upgradeSta);
            
            // 使用新的原子获取接口
            // UpgradeStatus_E mcuStatus, mpuStatus;
            // ret = ConcurrentOtaGetAllUpgradeStatus("/oemdata/upgrade/", &mcuStatus, &mpuStatus);
            // if(ret == RET_OK) {
            //     upgradeSta = mcuStatus; // 使用MCU状态
            //     // printf("Atomic status check - MCU: %d, MPU: %d\n", mcuStatus, mpuStatus);
            // } else {
            //     LOG_Error("Failed to get atomic upgrade status, fallback to individual calls");
            //     // 降级到单独获取
            //     ret = ConcurrentOtaGetUpgradeStatus("/oemdata/upgrade/", &upgradeSta, UPGRADE_MCU_FLAG);
            // }
            
            // if(ret < 0) {
            //     LOG_Error("Failed to get MCU upgrade status, attempt %d", retryCount);
            //     printf("Failed to get MCU upgrade status, attempt %d\n", retryCount);
            //     McuSrvSetSendLock(0);
                
            //     // 如果不是最后一次尝试，等待后重试
            //     if (retryCount < MAX_RETRY) {
            //         sleep(RETRY_INTERVAL);
            //         break; // 退出内层循环，进入外层重试循环
            //     } else {
            //         return -1;
            //     }
            // }
            
            if(UPGRADE_MCU_SUCCESS == upgradeSta) {
                LOG_Debug("MCU upgrade completed successfully on attempt %d", retryCount);
                printf("MCU upgrade completed successfully on attempt %d\n", retryCount);
                McuSrvSetSendLock(0);
                return 0;
            }
            
            if(UPGRADE_MCU_FAILED == upgradeSta) {
                LOG_Error("MCU upgrade failed with status: %d, attempt %d", upgradeSta, retryCount);
                McuSrvSetSendLock(0);
                
                // 如果不是最后一次尝试，等待后重试
                if (retryCount < MAX_RETRY) {
                    LOG_Debug("Waiting %d seconds before retry...", RETRY_INTERVAL);
                    sleep(RETRY_INTERVAL);
                    break; // 退出内层循环，进入外层重试循环
                } else {
                    LOG_Error("MCU upgrade failed after %d attempts", MAX_RETRY);
                    return -1;
                }
            }
            
            time(&current_time);
            if(difftime(current_time, start_time) >= 300) { 
                LOG_Error("MCU upgrade timeout (15 minutes), attempt %d", retryCount);
                McuSrvSetSendLock(0);
                
                // 如果不是最后一次尝试，等待后重试
                if (retryCount < MAX_RETRY) {
                    sleep(RETRY_INTERVAL);
                    break; // 退出内层循环，进入外层重试循环
                } else {
                    return -1;
                }
            }
            
            sleep(3);
        }
    }
    
    LOG_Error("MCU upgrade failed after maximum %d retries", MAX_RETRY);
    return -1;
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
    printf("===============1==============\n");
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