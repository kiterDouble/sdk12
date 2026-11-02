#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>
#include "ota_api.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "upgrade_interface.h"
#include "md5_interface.h"
#include "file_interface.h"
#include "system_interface.h"
#include "mcuinte.h"

// 全局变量
static UpgradeTiming g_timing = {0};
static UpgradeCoordinator g_coordinator = {0};

// 辅助函数：计算时间差（秒）
static inline double calc_duration(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

/**
 * @brief 清理旧的升级文件
 */
void ota_delete_file(void)
{
    int ret = 0;
    char cmd[512] = {0};
    
    LOG_Debug("Cleaning up previous upgrade files");
    
    // 删除BT文件
    if((access(BT1BINFILEPATH,F_OK) == 0)) {
        sprintf(cmd, "rm -rf %s*; sync ;", BT1BINFILEPATH);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret) {
            LOG_Error("Failed to delete BT files, ret %d, cmd : %s", ret, cmd);
        }
    }
    
    // 删除所有升级相关文件
    sprintf(cmd, "rm -rf %s*; rm -rf %s*; rm -rf %s*; rm -rf %s*; sync ;", 
            MD5DATA, MCU1BINFILEPATH, MCU2BINFILEPATH, MPUBINFILEPATHMPUBIN);
    ret = GetSystemRetValue(cmd);
    if(-1 == ret) {
        LOG_Error("Failed to clean upgrade files, ret %d, cmd : %s", ret, cmd);
    }
}

/**
 * @brief 解压升级包并校验文件完整性
 * @param upgradefilepath 升级包路径
 * @return 成功返回0，失败返回-1
 */
int ota_unzip_file(char *upgradefilepath)
{
    int ret = 0;
    char cmd[256] = {0};
    int retryCount = 0;
    const int MAX_RETRY = 3;
    int md5Exists = 0;
    int mcu1Exists = 0;
    int mcu2Exists = 0;
    int mpuExists = 0;
    
    gettimeofday(&g_timing.unzipStart, NULL);
    LOG_Debug("=== Starting file unpacking ===");
    
    // 清理旧文件
    ota_delete_file();
    
    // 最多重试3次解压
    while (retryCount < MAX_RETRY) {
        retryCount++;
        LOG_Debug("Unzip attempt %d/%d", retryCount, MAX_RETRY);
        
        // 执行解压命令
        sprintf(cmd, "tar -zxvf %s -C %s; sync ;", upgradefilepath, PATHUPGRADE);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret) {
            LOG_Error("unzip failed, ret %d, cmd : %s", ret, cmd);
            if (retryCount < MAX_RETRY) {
                LOG_Debug("Cleaning up and retrying...");
                ota_delete_file();
                sleep(1);
                continue;
            } else {
                LOG_Error("unzip failed after %d attempts", MAX_RETRY);
                return ret;
            }
        }
        
        // 校验四个关键文件是否存在 (md5.txt, mcu_1.bin, mcu_2.bin, update.zip)
        md5Exists = (access(MD5DATA, F_OK) == 0) ? 1 : 0;
        mcu1Exists = (access(MCU1BINFILEPATH, F_OK) == 0) ? 1 : 0;
        mcu2Exists = (access(MCU2BINFILEPATH, F_OK) == 0) ? 1 : 0;
        mpuExists = (access(MPUBINFILEPATHMPUBIN, F_OK) == 0) ? 1 : 0;
        
        LOG_Debug("File existence check - MD5: %d, MCU1: %d, MCU2: %d, MPU: %d", 
                  md5Exists, mcu1Exists, mcu2Exists, mpuExists);
        
        if (!md5Exists || !mcu1Exists || !mcu2Exists || !mpuExists) {
            LOG_Error("Required files missing after unzip (attempt %d/%d)", 
                      retryCount, MAX_RETRY);
            LOG_Error("Missing files - MD5: %s, MCU1: %s, MCU2: %s, MPU: %s",
                      md5Exists ? "exists" : "missing",
                      mcu1Exists ? "exists" : "missing",
                      mcu2Exists ? "exists" : "missing",
                      mpuExists ? "exists" : "missing");
            
            LOG_Debug("Cleaning up incomplete extracted files...");
            ota_delete_file();
            
            if (retryCount < MAX_RETRY) {
                memset(cmd, 0, sizeof(cmd));
                sprintf(cmd, "rm -rf %s", upgradefilepath);
                GetSystemRetValue(cmd);
                
                LOG_Debug("Waiting before retry...");
                sleep(1);
                continue;
            } else {
                LOG_Error("All %d unzip attempts failed - required files missing", MAX_RETRY);
                return -1;
            }
        }
        
        LOG_Debug("All required files found after unzip");
        break;
    }
    
    if (retryCount >= MAX_RETRY && (!md5Exists || !mcu1Exists || !mcu2Exists || !mpuExists)) {
        LOG_Error("Failed to extract all required files after %d attempts", MAX_RETRY);
        return -1;
    }
    
    gettimeofday(&g_timing.unzipEnd, NULL);
    g_timing.unzipDuration = calc_duration(g_timing.unzipStart, g_timing.unzipEnd);
    LOG_Debug("=== File unpacking completed: %.3f seconds ===", g_timing.unzipDuration);
    
    // 删除压缩包
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "rm -rf %s", upgradefilepath);
    ret = GetSystemRetValue(cmd);
    if(-1 == ret) {
        LOG_Error("delete zip file failed, ret %d, cmd : %s", ret, cmd);
    }
    
    // MD5校验 MCU1
    ret = VerifyFileMd5(MCU1BINFILEPATH, "MCU1BIN", 1);
    if(ret < 0) {
        LOG_Error("MCU1BIN MD5 verification failed");
        return ret;
    }
    
    // MD5校验 MCU2
    ret = VerifyFileMd5(MCU2BINFILEPATH, "MCU2BIN", 2);
    if(ret < 0) {
        LOG_Error("MCU2BIN MD5 verification failed");
        return ret;
    }
    
    // MD5校验 Tbox (update.zip)
    ret = VerifyFileMd5(MPUBINFILEPATHMPUBIN, "Tbox", 3);
    if(ret < 0) {
        LOG_Error("Tbox MD5 verification failed");
        return ret;
    }
    
    LOG_Debug("All files verified successfully");
    return 0;
}

/**
 * @brief MCU升级线程函数
 */
void* mcu_upgrade_thread(void* arg) {
    LOG_Debug("MCU upgrade thread started");
    printf("MCU upgrade thread started\n");
    
    // 记录MCU升级开始时间
    gettimeofday(&g_timing.mcuStart, NULL);
    LOG_Debug("=== Starting MCU upgrade ===");
    
    int result = LordMcuUpgrade();
    
    // 记录MCU升级结束时间
    gettimeofday(&g_timing.mcuEnd, NULL);
    g_timing.mcuDuration = calc_duration(g_timing.mcuStart, g_timing.mcuEnd);
    LOG_Debug("=== MCU upgrade completed: %.3f seconds, result: %d ===", 
              g_timing.mcuDuration, result);
    
    pthread_mutex_lock(&g_coordinator.statusMutex);
    g_coordinator.mcuCompleted = 1;
    g_coordinator.mcuResult = result;
    LOG_Debug("MCU upgrade completed with result: %d", result);
    printf("MCU upgrade completed with result: %d\n", result);
    pthread_cond_broadcast(&g_coordinator.allCompleteCond);
    pthread_mutex_unlock(&g_coordinator.statusMutex);
    
    return NULL;
}

/**
 * @brief MPU升级线程函数
 */
void* mpu_upgrade_thread(void* arg) {
    LOG_Debug("MPU upgrade thread started");
    printf("MPU upgrade thread started\n");
    
    // 记录MPU升级开始时间
    gettimeofday(&g_timing.mpuStart, NULL);
    LOG_Debug("=== Starting MPU upgrade ===");
    
    int result = LordMpuUpgrade();
    
    // 记录MPU升级结束时间
    gettimeofday(&g_timing.mpuEnd, NULL);
    g_timing.mpuDuration = calc_duration(g_timing.mpuStart, g_timing.mpuEnd);
    LOG_Debug("=== MPU upgrade completed: %.3f seconds, result: %d ===", 
              g_timing.mpuDuration, result);
    
    pthread_mutex_lock(&g_coordinator.statusMutex);
    g_coordinator.mpuCompleted = 1;
    g_coordinator.mpuResult = result;
    LOG_Debug("MPU upgrade completed with result: %d", result);
    printf("MPU upgrade completed with result: %d\n", result);
    pthread_cond_broadcast(&g_coordinator.allCompleteCond);
    pthread_mutex_unlock(&g_coordinator.statusMutex);
    
    return NULL;
}

/**
 * @brief MCU升级执行函数
 * @return 成功返回0，失败返回-1
 */
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
        
        // 启动MCU升级
        ret = OtaStartUpgrade("/oemdata/upgrade/", UPGRADE_MCU_FLAG);
        if(ret < 0) {
            McuSrvSetSendLock(0);
            LOG_Error("OtaStartUpgrade MCU failed: %d, attempt %d", ret, retryCount);
            printf("OtaStartUpgrade MCU failed: %d, attempt %d\n", ret, retryCount);
            
            if (retryCount < MAX_RETRY) {
                LOG_Debug("Waiting %d seconds before retry...", RETRY_INTERVAL);
                printf("Waiting %d seconds before retry...\n", RETRY_INTERVAL);
                sleep(RETRY_INTERVAL);
                continue;
            } else {
                LOG_Error("MCU upgrade failed after %d attempts", MAX_RETRY);
                printf("MCU upgrade failed after %d attempts\n", MAX_RETRY);
                return -1;
            }
        }
        
        time(&start_time);
        
        // 监控升级过程
        while(1) {
            ret = OtaGetUpgradeStatus("/oemdata/upgrade/", &upgradeSta);
            if(ret < 0) {
                LOG_Error("Failed to get MCU upgrade status, attempt %d", retryCount);
                printf("Failed to get MCU upgrade status, attempt %d\n", retryCount);
                McuSrvSetSendLock(0);
                
                if (retryCount < MAX_RETRY) {
                    sleep(RETRY_INTERVAL);
                    break;
                } else {
                    return -1;
                }
            }
            
            printf("MCU upgrade status: %d\n", upgradeSta);
            
            if(UPGRADE_MCU_SUCCESS == upgradeSta) {
                LOG_Debug("MCU upgrade completed successfully on attempt %d", retryCount);
                printf("MCU upgrade completed successfully on attempt %d\n", retryCount);
                McuSrvSetSendLock(0);
                return 0;
            }
            
            if(UPGRADE_MCU_FAILED == upgradeSta) {
                LOG_Error("MCU upgrade failed with status: %d, attempt %d", upgradeSta, retryCount);
                McuSrvSetSendLock(0);
                
                if (retryCount < MAX_RETRY) {
                    LOG_Debug("Waiting %d seconds before retry...", RETRY_INTERVAL);
                    sleep(RETRY_INTERVAL);
                    break;
                } else {
                    LOG_Error("MCU upgrade failed after %d attempts", MAX_RETRY);
                    return -1;
                }
            }
            
            time(&current_time);
            if(difftime(current_time, start_time) >= 300) { // 5分钟超时
                LOG_Error("MCU upgrade timeout (5 minutes), attempt %d", retryCount);
                McuSrvSetSendLock(0);
                
                if (retryCount < MAX_RETRY) {
                    sleep(RETRY_INTERVAL);
                    break;
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

/**
 * @brief MPU升级执行函数
 * @return 成功返回0，失败返回-1
 */
int LordMpuUpgrade() {
    char cmd[128] = {0};
    int ret = 0;
    UpgradeStatus_E upgradeSta = UPGRADE_INIT;
    time_t start_time, current_time;
    
    LOG_Debug("Starting MPU upgrade process...");
    printf("Starting MPU upgrade process...\n");
    
    StartQueueMsgRcvThread("upgrade_mpu");
    
    // 启动MPU升级
    ret = OtaStartUpgrade("/oemdata/upgrade/", UPGRADE_MPU_FLAG);
    if(ret < 0) {
        LOG_Error("OtaStartUpgrade MPU failed: %d", ret);
        printf("OtaStartUpgrade MPU failed: %d\n", ret);
        return -1;
    }
    
    Set_upgrade_mode(1);
    time(&start_time);
    
    while(1) {
        ret = ConcurrentOtaStartUpgrade("/oemdata/upgrade/", UPGRADE_MPU_FLAG);
        if(ret < 0) {
            LOG_Error("Failed to get MPU upgrade status");
            return -1;
        }
        
        printf("MPU upgrade status: %d\n", upgradeSta);
        
        if(UPGRADE_MPU_NEED_BACKUP == upgradeSta) {
            LOG_Debug("MPU upgrade completed successfully");
            printf("MPU upgrade completed successfully\n");
            system("echo 100 > /oemdata/parameters/SlaveModemUpProgress");
            return 0;
        }
        
        if(UPGRADE_MPU_FAILED == upgradeSta) {
            LOG_Error("MPU upgrade failed with status: %d", upgradeSta);
            printf("MPU upgrade failed with status: %d\n", upgradeSta);
            
            memset(cmd, 0, sizeof(cmd));
            sprintf(cmd, "ql_cmd sysinfo /oemdata/log;");
            int sys_ret = GetSystemRetValue(cmd);
            if(-1 == sys_ret) {
                LOG_Error("ql_cmd sysinfo %d, cmd : %s", sys_ret, cmd);
            }
            
            return -1;
        }
        
        time(&current_time);
        if(difftime(current_time, start_time) >= 600) { // 10分钟超时
            LOG_Error("MPU upgrade timeout (10 minutes), status: %d", upgradeSta);
            memset(cmd, 0, sizeof(cmd));
            sprintf(cmd, "ql_cmd sysinfo /oemdata/log;");
            int sys_ret = GetSystemRetValue(cmd);
            if(-1 == sys_ret) {
                LOG_Error("ql_cmd sysinfo %d, cmd : %s", sys_ret, cmd);
            }
            
            return -1;
        }
        
        sleep(3);
    }
    
    return -1;
}

/**
 * @brief 执行并行升级（MCU + MPU）
 * @return 成功返回0，失败返回-1
 */
int DoUpgrade() {
    int ret = 0;
    pthread_t mcuThread, mpuThread;
    
    LOG_Debug("Starting parallel upgrade process (MCU + MPU)");
    printf("Starting parallel upgrade process (MCU + MPU)\n");
    
    // 初始化协调器
    memset(&g_coordinator, 0, sizeof(UpgradeCoordinator));
    pthread_mutex_init(&g_coordinator.statusMutex, NULL);
    pthread_cond_init(&g_coordinator.allCompleteCond, NULL);
    
    // 创建MCU升级线程
    ret = pthread_create(&mcuThread, NULL, mcu_upgrade_thread, NULL);
    if (ret != 0) {
        LOG_Error("Failed to create MCU thread: %d", ret);
        return -1;
    }
    
    // 创建MPU升级线程
    ret = pthread_create(&mpuThread, NULL, mpu_upgrade_thread, NULL);
    if (ret != 0) {
        LOG_Error("Failed to create MPU thread: %d", ret);
        return -1;
    }
    
    // 等待两个线程完成（带超时）
    int upgradeTimeout = 900; // 15分钟超时
    time_t startTime = time(NULL);
    int mcuDone = 0, mpuDone = 0;
    int mcuResult = 0, mpuResult = 0;
    
    while (1) {
        // 检查超时
        if (difftime(time(NULL), startTime) >= upgradeTimeout) {
            LOG_Error("Upgrade monitoring timeout after %d seconds", upgradeTimeout);
            break;
        }
        
        // 检查升级状态
        pthread_mutex_lock(&g_coordinator.statusMutex);
        mcuDone = g_coordinator.mcuCompleted;
        mpuDone = g_coordinator.mpuCompleted;
        mcuResult = g_coordinator.mcuResult;
        mpuResult = g_coordinator.mpuResult;
        pthread_mutex_unlock(&g_coordinator.statusMutex);
        
        // 如果两个都完成，退出循环
        if (mcuDone && mpuDone) {
            break;
        }
        
        sleep(1);
    }
    
    // 等待线程结束
    pthread_join(mcuThread, NULL);
    pthread_join(mpuThread, NULL);
    
    // 输出计时报告
    LOG_Debug("========== Upgrade Timing Report ==========");
    LOG_Debug("1. File unpacking time: %.3f seconds", g_timing.unzipDuration);
    LOG_Debug("2. MCU upgrade time: %.3f seconds", g_timing.mcuDuration);
    LOG_Debug("3. MPU upgrade time: %.3f seconds", g_timing.mpuDuration);
    LOG_Debug("===========================================");
    
    printf("\n========== Upgrade Timing Report ==========\n");
    printf("1. File unpacking time: %.3f seconds\n", g_timing.unzipDuration);
    printf("2. MCU upgrade time: %.3f seconds\n", g_timing.mcuDuration);
    printf("3. MPU upgrade time: %.3f seconds\n", g_timing.mpuDuration);
    printf("===========================================\n");
    
    // 检查升级结果
    int allSuccess = 1;
    
    if (mcuResult != 0) {
        LOG_Error("MCU upgrade failed with result: %d", mcuResult);
        allSuccess = 0;
    }
    
    if (mpuResult != 0) {
        LOG_Error("MPU upgrade failed with result: %d", mpuResult);
        allSuccess = 0;
    }
    
    if (allSuccess) {
        LOG_Debug("All modules upgraded successfully!");
        printf("All modules upgraded successfully!\n");
        system("echo 100 > /oemdata/parameters/SlaveModemUpProgress");
        SetsloveModemUpgradeStatus(2); // SUCCESS
        return 0;
    } else {
        LOG_Error("Some modules failed to upgrade");
        printf("Some modules failed to upgrade\n");
        SetsloveModemUpgradeStatus(3); // FAILED
        return -1;
    }
}

/**
 * @brief 等待下一步指令（重启或切换分区）
 * @param socket_fd socket文件描述符
 * @param timeout_seconds 超时时间（秒）
 * @return 0-成功接收指令，-1-超时或错误
 */
int WaitForNextCommand(int socket_fd, int timeout_seconds) {
    char buffer[256] = {0};
    int ret = 0;
    time_t start_time, current_time;
    
    LOG_Debug("Waiting for next command (timeout: %d seconds)...", timeout_seconds);
    printf("Waiting for next command (timeout: %d seconds)...\n", timeout_seconds);
    
    time(&start_time);
    
    while (1) {
        // 检查超时
        time(&current_time);
        if (difftime(current_time, start_time) >= timeout_seconds) {
            LOG_Error("Wait for next command timeout after %d seconds", timeout_seconds);
            printf("Wait for next command timeout after %d seconds\n", timeout_seconds);
            return -1;
        }
        
        // 使用select等待数据，设置1秒超时
        fd_set read_fds;
        struct timeval tv;
        FD_ZERO(&read_fds);
        FD_SET(socket_fd, &read_fds);
        
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        ret = select(socket_fd + 1, &read_fds, NULL, NULL, &tv);
        if (ret < 0) {
            LOG_Error("select error: %s", strerror(errno));
            return -1;
        } else if (ret == 0) {
            // 超时，继续循环
            continue;
        }
        
        // 有数据可读
        if (FD_ISSET(socket_fd, &read_fds)) {
            ret = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
            if (ret <= 0) {
                LOG_Error("recv error or connection closed: %s", 
                         ret == 0 ? "Connection closed" : strerror(errno));
                return -1;
            }
            
            buffer[ret] = '\0';
            LOG_Debug("Received command: %s", buffer);
            printf("Received command: %s\n", buffer);
            
            // 处理指令
            if (strcmp(buffer, "reboot") == 0 || strcmp(buffer, "reboot_begin") == 0) {
                LOG_Debug("Received reboot command");
                printf("Received reboot command\n");
                
                // 执行重启
                ret = SrTboxSystemReboot("otamodem1", REMOTE_CONTROL_REBOOT);
                if (ret < 0) {
                    LOG_Error("Failed to execute reboot command");
                    return -1;
                }
                
                // 如果重启命令发送成功，程序应该会被重启
                // 这里sleep一段时间确保重启命令生效
                sleep(5);
                return 0;
                
            } else if (strcmp(buffer, "switch_partition") == 0 || 
                       strcmp(buffer, "switch_partition_begin") == 0) {
                LOG_Debug("Received switch partition command");
                printf("Received switch partition command\n");
                
                // 执行分区切换
                ret = OtaSwitchAOrBPartition2();
                if (ret < 0) {
                    LOG_Error("Failed to switch partition, ret = %d", ret);
                    return -1;
                }
                
                LOG_Debug("Partition switched successfully");
                printf("Partition switched successfully\n");
                
                // 分区切换后可能需要重启
                sleep(2);
                return 0;
                
            } else {
                LOG_Debug("Unknown command: %s", buffer);
                printf("Unknown command: %s\n", buffer);
                // 继续等待其他指令
            }
        }
    }
    
    return -1;
}
