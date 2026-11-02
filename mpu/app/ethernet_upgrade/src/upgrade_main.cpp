#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "err_code.h"
#include "sr_log.h"
#include "ethernet_upgrade.h"
#include "upgrade_interface.h"
#include "md5_interface.h"
#include "upgrade_socket.h"  
#include "system_interface.h"
#include "mcuinte.h"

void ota_delete_file(void);

// 升级时间统计结构
typedef struct {
    // 解压时间
    struct timeval unzipStart;
    struct timeval unzipEnd;
    double unzipDuration;
    
    // 传输时间
    struct timeval transferStart;
    struct timeval transferEnd;
    double transferDuration;
    
    // MCU 升级时间
    struct timeval mcuStart;
    struct timeval mcuEnd;
    double mcuDuration;
    
    // MPU 升级时间
    struct timeval mpuStart;
    struct timeval mpuEnd;
    double mpuDuration;
} UpgradeTiming;

static UpgradeTiming g_timing = {0};

// 升级状态结构
typedef struct {
    int mcuCompleted;
    int mpuMainCompleted;
    int mpuSlaveCompleted;
    int mcuResult;
    int mpuMainResult;
    int mpuSlaveResult;
    pthread_mutex_t statusMutex;
    pthread_cond_t allCompleteCond;
} UpgradeCoordinator;

static UpgradeCoordinator g_coordinator = {0};

// Socket通讯相关的全局变量（从ethernet_upgrade.cpp移动过来）
static unsigned char buffer[BUFFER_SIZE];
static FILE *file = NULL;
static long total_bytes_received = 0;
static size_t bytes_to_send = 0;

// 异步socket回调函数
void on_data_received(int sockfd, const char* data, size_t length) {
    LOG_Debug("Received data: %.*s", (int)length, data);
    // 处理接收到的数据
}

void on_connected(int sockfd) {
    LOG_Debug("Socket connected successfully, fd: %d", sockfd);
}

void on_error(int sockfd, int error) {
    LOG_Error("Socket error: %d, fd: %d", error, sockfd);
}

// 辅助函数：获取当前时间戳（毫秒）
static inline long long get_timestamp_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// 辅助函数：计算时间差（秒）
static inline double calc_duration(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

// MCU升级线程
void* mcu_upgrade_thread(void* arg) {
    LOG_Debug("MCU upgrade thread started");
    printf("MCU upgrade thread started\n");
    
    // 记录 MCU 升级开始时间
    gettimeofday(&g_timing.mcuStart, NULL);
    LOG_Debug("=== Starting MCU upgrade ===");
    
    int result = LordMcuUpgrade();
    
    // 记录 MCU 升级结束时间
    gettimeofday(&g_timing.mcuEnd, NULL);
    g_timing.mcuDuration = calc_duration(g_timing.mcuStart, g_timing.mcuEnd);
    LOG_Debug("=== MCU upgrade completed: %.3f seconds, result: %d ===", g_timing.mcuDuration, result);
    
    pthread_mutex_lock(&g_coordinator.statusMutex);
    g_coordinator.mcuCompleted = 1;
    g_coordinator.mcuResult = result;
    LOG_Debug("MCU upgrade completed with result: %d", result);
    printf("MCU upgrade completed with result: %d\n", result);
    pthread_cond_broadcast(&g_coordinator.allCompleteCond);
    pthread_mutex_unlock(&g_coordinator.statusMutex);
    
    return NULL;
}


void* mpu_main_upgrade_thread(void* arg) {
    LOG_Debug("MPU main upgrade thread started");
    printf("MPU main upgrade thread started\n");
    
    // 记录 MPU 升级开始时间
    gettimeofday(&g_timing.mpuStart, NULL);
    LOG_Debug("=== Starting MPU main upgrade ===");
    
    int result = LordMpuMainUpgrade();
    
    // 记录 MPU 升级结束时间
    gettimeofday(&g_timing.mpuEnd, NULL);
    g_timing.mpuDuration = calc_duration(g_timing.mpuStart, g_timing.mpuEnd);
    LOG_Debug("=== MPU main upgrade completed: %.3f seconds, result: %d ===", g_timing.mpuDuration, result);
    
    pthread_mutex_lock(&g_coordinator.statusMutex);
    g_coordinator.mpuMainCompleted = 1;
    g_coordinator.mpuMainResult = result;
    LOG_Debug("MPU main upgrade completed with result: %d", result);
    printf("MPU main upgrade completed with result: %d\n", result);
    pthread_cond_broadcast(&g_coordinator.allCompleteCond);
    pthread_mutex_unlock(&g_coordinator.statusMutex);
    
    return NULL;
}

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
    
    ota_delete_file();
    
    while (retryCount < MAX_RETRY) {
        retryCount++;
        LOG_Debug("Unzip attempt %d/%d", retryCount, MAX_RETRY);
        
        sprintf(cmd, "tar -zxvf  %s -C %s; sync ;", upgradefilepath, PATHUPGRADE);
        ret = GetSysRetValue(cmd);
        if(-1 == ret)
        {
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
                GetSysRetValue(cmd);
                
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
    
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "rm -rf %s", upgradefilepath);
    ret = GetSysRetValue(cmd);
    if(-1 == ret)
    {
        LOG_Error("delete zip file failed, ret %d, cmd : %s", ret, cmd);
    }
    
    ret = VerifyFileMd5(MCU1BINFILEPATH, "MCU1BIN", 1);
    if(ret < 0) {
        LOG_Error("MCU1BIN MD5 verification failed");
        return ret;
    }
    
    ret = VerifyFileMd5(MCU2BINFILEPATH, "MCU2BIN", 2);
    if(ret < 0) {
        LOG_Error("MCU2BIN MD5 verification failed");
        return ret;
    }
    
    ret = VerifyFileMd5(MPUBINFILEPATHMPUBIN, "Tbox", 3);
    if(ret < 0) {
        LOG_Error("Tbox MD5 verification failed");
        return ret;
    }
    
    LOG_Debug("All files verified successfully");
    return 0;
}


void ota_delete_file(void)
{
    int ret = 0;
    char cmd[512] = {0};
    
    LOG_Debug("Cleaning up previous upgrade files");
    
    // 删除BT文件
    if((access(BT1BINFILEPATH,F_OK) == 0)) {
        sprintf(cmd, "rm -rf %s*; sync ;", BT1BINFILEPATH);
        ret = GetSysRetValue(cmd);
        if(-1 == ret) {
            LOG_Error("Failed to delete BT files, ret %d, cmd : %s", ret, cmd);
        }
    }
    
    // 删除所有升级相关文件
    sprintf(cmd, "rm -rf %s*; rm -rf %s*; rm -rf %s*; rm -rf %s*; sync ;", 
            MD5DATA, MCU1BINFILEPATH, MCU2BINFILEPATH, MPUBINFILEPATHMPUBIN);
    ret = GetSysRetValue(cmd);
    if(-1 == ret) {
        LOG_Error("Failed to clean upgrade files, ret %d, cmd : %s", ret, cmd);
    }
}


extern void handle_signal(int sig);

static void handle_server_command(const char* command, size_t length) {
    LOG_Debug("Processing server command: %.*s", (int)length, command);
    printf("Processing server command: %.*s \n", (int)length, command);
    // 根据具体需求处理不同的命令
    if (strstr(command, "Upgrade Success") || strstr(command, "Success")) {
        g_coordinator.mpuSlaveCompleted = 1;
        // 可以设置全局标志或触发相应事件
    } else if (strstr(command, "Upgrade Fail") || strstr(command, "Upgrade Fail")) {
        g_coordinator.mpuSlaveCompleted = 3; 
        // 处理失败情况
    } else if (strstr(command, "status")) {
        
        // 处理状态更新
    } else {
        LOG_Debug("Unknown server command received");
    }
}
int SetMainModemUpgradeStatus(int status)
{
    char cmd[512] = {0};
    int ret = 0;
    
    // 验证状态值范围 (只允许 0,1,2,3,4)
    if (status < 0 || status > 4) {
        LOG_Error("WriteSMainModemUpgradeStatus: Invalid status value %d, must be 0-4", status);
        return RET_ERR;
    }
    
    // 使用 echo 写入文件
    snprintf(cmd, sizeof(cmd), "echo %d > /oemdata/parameters/MainModemUpgradeStatus", status);
    ret = system(cmd);
    if (ret != 0) {
        LOG_Error("WriteSMainModemUpgradeStatus: Failed to write status %d, ret=%d", status, ret);
        return RET_ERR;
    }
    
    LOG_Debug("WriteSMainModemUpgradeStatus: Successfully wrote status %d", status);
    return RET_OK;
}
int main(int argc, char *argv[]) {
    LOG_INIT("ethernet_upgrade");
    int ret = 0;
    double filesize = 0;
    char filepatch[256] = {0};
    char bufferdouble[10] = {0};
    char cmd[256] = {0};
    // 初始化协调器
    memset(&g_coordinator, 0, sizeof(UpgradeCoordinator));
    pthread_mutex_init(&g_coordinator.statusMutex, NULL);
    pthread_cond_init(&g_coordinator.allCompleteCond, NULL);
    system("echo 0 > /oemdata/parameters/MainModemUpProgress");
    // 初始化异步socket上下文
    async_socket_init(&g_async_socket_ctx);
    
    ret = sprintf(filepatch,"%s%s",DOWNLOADUPGRADEPACKAGEFILEPATH,argv[1]);
    if(ret < 0 ) {
        LOG_Debug("sprintf error return\n");       
        return -1; 
    }
    LOG_Debug("File argument: %s\n", filepatch);
    signal(SIGINT, handle_signal);
    signal(SIGTSTP, handle_signal);
    filesize = GetFileSize(filepatch);
    if(filesize < 0)
    {
        SwtMainModemUpgradeStatus(3);
        printf("Error in obtaining file size %s\n",filepatch);
        LOG_Error("Error in obtaining file size %s\n",filepatch);
        return -1;
    }
    ret = ota_unzip_file(filepatch);
    if(ret == -1) {
        LOG_Error("unzip fail ret:%d",ret);
        SwtMainModemUpgradeStatus(3);
        return -1;
    }
    SwtMainModemUpgradeStatus(1);
    #if 1
    async_socket_set_command_handler(&g_async_socket_ctx, handle_server_command);
    // 使用异步socket连接服务器（替换原来的ConnectToServer）
    ret = async_socket_connect(&g_async_socket_ctx, SERVER_IP, PORT);
    if(ret < 0)
    {
        printf("Failed to connect socket ret = %d \n",ret);
        LOG_Error("Failed to connect socket ret = %d \n",ret);
        if(ret == -1)
        {
            printf("%s fail \n",RMFILEETHERNET);
            LOG_Error("%s fail \n",RMFILEETHERNET);
        }
        return -1;
    }

    // 同时获取MB（兼容旧版）和字节数（新版）
    filesize = GetFileSize(MPUBINFILEPATHMPUBIN);
    struct stat st;
    if(filesize < 0 || stat(MPUBINFILEPATHMPUBIN, &st) != 0)
    {
        printf("Get File Size Error \n");
        LOG_Error("Get File Size Error \n");
        printf("%s fail\n",RMFILEETHERNET);
        LOG_Error("%s fail\n",RMFILEETHERNET);
        async_socket_close(&g_async_socket_ctx);
        printf("Error in obtaining file size %s\n",filepatch);
        LOG_Error("Error in obtaining file size %s\n",filepatch);
        return -1;
    }else{
        // 新协议：MB(8字节) + type(1字节) + bytes(8字节) = 17字节
        // 旧协议：MB(8字节)
        unsigned char size_buffer[17];
        double filesize_bytes = (double)st.st_size; // 精确字节数
        
        // 前8字节：MB（兼容旧版）
        memcpy(&size_buffer[0], &filesize, sizeof(double));
        // 第9字节：type标识
        size_buffer[8] = 0xFF; // type=0xFF表示后面是字节数（使用特殊值避免与数据冲突）
        // 后8字节：精确字节数
        memcpy(&size_buffer[9], &filesize_bytes, sizeof(double));
        
        LOG_Debug("Sending: MB=%.2f, bytes=%.0f\n", filesize, filesize_bytes);
        
        ret = async_socket_send(&g_async_socket_ctx, size_buffer, sizeof(size_buffer));
        if(ret < 0)
        {
            if(ret == -1)
            {
                printf("%s fail",RMFILEETHERNET);
            }
            printf("socket send fail ret = %d\n",ret);
            LOG_Error("socket send fail ret = %d\n",ret);
            async_socket_close(&g_async_socket_ctx);
            return -1;
        }
    }
    
    memset(bufferdouble,0,sizeof(bufferdouble));
    file = fopen(MPUBINFILEPATHMPUBIN, "rb");
    if (file == NULL) {
        printf("Failed to open file %s\n",filepatch);
        LOG_Error("Failed to open file %s\n",filepatch);
        if(ret == -1)
        {
            printf("%s fail \n",RMFILEETHERNET);
            LOG_Error("%s fail \n",RMFILEETHERNET);
        }
        async_socket_close(&g_async_socket_ctx);
        return -1;
    }
    sleep(2);
    
    printf("Sending upgrade package...\n");
    LOG_Debug("Sending upgrade package...\n");
    
    // 记录传输开始时间
    gettimeofday(&g_timing.transferStart, NULL);
    LOG_Debug("=== Starting upgrade package transfer ===");
    Set_upgrade_mode(1);
    while ((bytes_to_send = fread(buffer, 1, BUFFER_SIZE, file)) > 0) 
    {
        // usleep(10*1000);
        ret = async_socket_send(&g_async_socket_ctx, buffer, bytes_to_send);
        if(ret < 0)
        {
            if(ret == -1)
            {
                printf("%s fail \n",RMFILEETHERNET);
                LOG_Error("%s fail \n",RMFILEETHERNET);
            }
            printf("Bytes sent: %ld,ret = %d\n",ret, total_bytes_received);
            LOG_Debug("Bytes sent: %ld,ret = %d\n",ret, total_bytes_received);
            async_socket_close(&g_async_socket_ctx);
            fclose(file);
            return -1;
        }
        total_bytes_received = bytes_to_send + total_bytes_received;
        
    }
    
    // 记录传输结束时间
    gettimeofday(&g_timing.transferEnd, NULL);
    g_timing.transferDuration = calc_duration(g_timing.transferStart, g_timing.transferEnd);
    LOG_Debug("=== Upgrade package transfer completed: %.3f seconds, total bytes: %ld ===", g_timing.transferDuration, total_bytes_received);
    
    printf("Bytes sent: %ld\n", total_bytes_received);
    LOG_Debug("Bytes sent: %ld\n", total_bytes_received);
    sleep(2);
    fclose(file);

    const char* end_signal = "end";
    ret = async_socket_send_string(&g_async_socket_ctx, end_signal);
    if(ret < 0)
    {
        if(ret == -1)
        {
            printf("%s fail",RMFILEETHERNET);
            LOG_Error("%s fail",RMFILEETHERNET);
        }
        printf("socket send end fail ret = %d\n",ret);
        LOG_Error("socket send end fail ret = %d\n",ret);
        async_socket_close(&g_async_socket_ctx);
        return -1;
    }
    printf("send end \n");
    LOG_Debug("send end \n");
    system("echo 20 > /oemdata/parameters/MainModemUpProgress");
   #endif

    pthread_t mcuThread, mpuMainThread;
    
    // MCU升级线程
    ret = pthread_create(&mcuThread, NULL, mcu_upgrade_thread, NULL);
    if (ret != 0) {
        LOG_Error("Failed to create MCU thread: %d", ret);
        return -1;
    }
    // MPU主模组升级线程
    ret = pthread_create(&mpuMainThread, NULL, mpu_main_upgrade_thread, NULL);
    if (ret != 0) {
        LOG_Error("Failed to create MPU main thread: %d", ret);
        return -1;
    }

    // 移除阻塞的 pthread_join，改为非阻塞监控
    /*
pthread_join(mcuThread, NULL);
pthread_join(mpuMainThread, NULL);
*/

    // 主控循环：监控升级状态并统一处理重启指令
    int rebootCommandSent = 0;
    int upgradeTimeout = 600;
    time_t startTime = time(NULL);
    int mcuDone = g_coordinator.mcuCompleted;
    int mpuMainDone = g_coordinator.mpuMainCompleted;
    int slaveModemStatus = g_coordinator.mpuSlaveCompleted;
    int mcuResult = g_coordinator.mcuResult;
    int mpuMainResult = g_coordinator.mpuMainResult;
    while (1)
    {
        // printf("Monitoring upgrade status...\n");
        // 检查超时
        if (difftime(time(NULL), startTime) >= upgradeTimeout) {
            LOG_Error("Upgrade monitoring timeout after %d seconds", upgradeTimeout);
            SetMainModemUpgradeStatus(3);
            system("echo 0 > /oemdata/parameters/MainModemUpProgress");
            break;
        }
        
        // 检查MCU和主模组升级状态
        pthread_mutex_lock(&g_coordinator.statusMutex);
        mcuDone = g_coordinator.mcuCompleted;
        mpuMainDone = g_coordinator.mpuMainCompleted;
        slaveModemStatus = g_coordinator.mpuSlaveCompleted;
        mcuResult = g_coordinator.mcuResult;
        mpuMainResult = g_coordinator.mpuMainResult;
        // printf("MCU upgrade status: %d, MPU main upgrade status: %d, Slave modem upgrade status: %d\n", mcuDone, mpuMainDone, slaveModemStatus);
        pthread_mutex_unlock(&g_coordinator.statusMutex);
        Set_upgrade_mode(1);
        // 如果所有模块都完成升级
        if (mcuDone && mpuMainDone && slaveModemStatus != 0) {
            LOG_Debug("All modules upgrade completed");
            printf("All modules upgrade completed\n");
            
            // 检查是否所有模块都升级成功
            int allSuccess = 1;
            
            // 检查MCU升级结果
            if (mcuResult != 0) {
                LOG_Error("MCU upgrade failed with result: %d", mcuResult);
                allSuccess = 0;
            }
            
            // 检查主模组升级结果
            if (mpuMainResult != 0) {
                LOG_Error("MPU main upgrade failed with result: %d", mpuMainResult);
                allSuccess = 0;
            }
            
            // 检查副模组升级结果
            if (slaveModemStatus != 1) {
                LOG_Error("Slave modem upgrade failed with status: %d", slaveModemStatus);
                allSuccess = 0;
            }
            
            if (allSuccess) {
                system("echo 100 > /oemdata/parameters/MainModemUpProgress");
                SetMainModemUpgradeStatus(2);
                sleep(2);
                LOG_Info("All modules upgraded successfully!");
                printf("All modules upgraded successfully!\n");
                
                // 输出完整的计时报告
                LOG_Info("========== Upgrade Timing Report ==========");
                LOG_Info("1. File unpacking time: %.3f seconds", g_timing.unzipDuration);
                LOG_Info("2. Package transfer time: %.3f seconds", g_timing.transferDuration);
                LOG_Info("3. MCU upgrade time: %.3f seconds", g_timing.mcuDuration);
                LOG_Info("4. MPU main upgrade time: %.3f seconds", g_timing.mpuDuration);
                LOG_Info("===========================================");
                
                printf("\n========== Upgrade Timing Report ==========\n");
                printf("1. File unpacking time: %.3f seconds\n", g_timing.unzipDuration);
                printf("2. Package transfer time: %.3f seconds\n", g_timing.transferDuration);
                printf("3. MCU upgrade time: %.3f seconds\n", g_timing.mcuDuration);
                printf("4. MPU main upgrade time: %.3f seconds\n", g_timing.mpuDuration);
                printf("===========================================\n");
                
                // 发送重启指令给所有模块
                if (!rebootCommandSent) {
                    LOG_Info("Sending reboot command to all modules...");
                    printf("Sending reboot command to all modules...\n");
                    Set_upgrade_mode(1);
                    SetMainModemUpgradeStatus(2);
                    // 发送重启指令到副模组
                    char reboot_cmd[] = "reboot_begin";
                    ret = async_socket_send_string(&g_async_socket_ctx, reboot_cmd);
                    if (ret < 0) {
                        LOG_Error("Failed to send reboot command to slave modem");
                    } else {
                        rebootCommandSent = 1;
                        LOG_Debug("Reboot command sent to slave modem");
                    }
                    ret = OtaSwitchAOrBPartition2();
                    if(ret < 0)
                    {
                        LOG_Error("OtaSwitchAOrBPartition fail ret = %d",ret);
                    }
                    SwtMainModemUpgradeStatus(2);
                    sleep(10);
                }
            } else {
                LOG_Error("Some modules failed to upgrade");
                printf("Some modules failed to upgrade\n");
                SetMainModemUpgradeStatus(3);
                Set_upgrade_mode(1);
                // 输出失败的计时报告
                LOG_Error("========== Upgrade Timing Report (Failed) ==========");
                LOG_Error("1. File unpacking time: %.3f seconds", g_timing.unzipDuration);
                LOG_Error("2. Package transfer time: %.3f seconds", g_timing.transferDuration);
                LOG_Error("3. MCU upgrade time: %.3f seconds", g_timing.mcuDuration);
                LOG_Error("4. MPU main upgrade time: %.3f seconds", g_timing.mpuDuration);
                LOG_Error("====================================================");
                
                printf("\n========== Upgrade Timing Report (Failed) ==========\n");
                printf("1. File unpacking time: %.3f seconds\n", g_timing.unzipDuration);
                printf("2. Package transfer time: %.3f seconds\n", g_timing.transferDuration);
                printf("3. MCU upgrade time: %.3f seconds\n", g_timing.mcuDuration);
                printf("4. MPU main upgrade time: %.3f seconds\n", g_timing.mpuDuration);
                printf("====================================================\n");
                
                // 发送升级失败指令
                char fail_cmd[] = "update_failed";
                ret = async_socket_send_string(&g_async_socket_ctx, fail_cmd);
                if (ret < 0) {
                    LOG_Error("Failed to send update failed command");
                }
                async_socket_close(&g_async_socket_ctx);
                ret = SrTboxSystemReboot("ethernet_upgrade",REMOTE_CONTROL_REBOOT);
                if(ret < 0)
                {
                    LOG_Error("OtaSwitchAOrBPartition fail ret = %d",ret);
                }
                sleep(30);
                
            }
            
            break; // 退出监控循环
        }
        
        // 短暂休眠避免过度占用CPU
        sleep(3);
    }

    // 清理资源
    async_socket_close(&g_async_socket_ctx);
    LOG_Info("Ethernet upgrade process completed");
    printf("Ethernet upgrade process completed\n");
    return 0;
}