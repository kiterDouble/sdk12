#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <openssl/md5.h>
#include <signal.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <math.h>
#include "ota_api.h"
#include "sr_ipc.h"
#include "otamodem2.h"
#include "sr_log.h"
#include "file_interface.h"
#include "mtd_interface.h"
#include "upgrade_interface.h"
#include "system_interface.h"
#include "mcuinte.h"
#include "md5_interface.h"

#define UPGRADE_STATE_IDLE      0
#define UPGRADE_STATE_RUNNING   1
#define UPGRADE_STATE_SUCCESS   2
#define UPGRADE_STATE_FAILED    3
#define UPGRADE_STATE_BACKUP    4

static int g_currentUpgradeState = UPGRADE_STATE_IDLE;
static long g_expectedFileSize = 0;  // 期望接收的文件大小（字节）

static void SetUpgradeStatus(int status)
{
    if (status < 0 || status > 4) {
        LOG_Error("Invalid status value %d, must be 0-4", status);
        return;
    }
    
    if (g_currentUpgradeState == status) {
        return;
    }
    
    SetsloveModemUpgradeStatus(status);
    g_currentUpgradeState = status;
}

static void CloseSocketSafe(int *sockfd)
{
    if (sockfd == NULL || *sockfd < 0) {
        return;
    }
    
    close(*sockfd);
    *sockfd = -1;
}

static int WaitForSocketConnection(int server_fd, int timeout_sec)
{
    int flags;
    fd_set readfds;
    struct timeval timeout;
    int activity;
    
    flags = fcntl(server_fd, F_GETFL, 0);
    if (flags < 0) {
        LOG_Error("fcntl F_GETFL error");
        return -1;
    }
    
    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        LOG_Error("fcntl F_SETFL error");
        return -1;
    }
    
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
    
    activity = select(server_fd + 1, &readfds, NULL, NULL, &timeout);
    
    if (activity < 0) {
        LOG_Error("select error\n");
        return -1;
    }
    
    if (activity == 0) {
        LOG_Debug("Accept timeout, continuing...\n");
        return 0;
    }
    
    return FD_ISSET(server_fd, &readfds) ? 1 : 0;
}

static int ReceiveFileSize(int client_fd, double *out_size)
{
    unsigned char buffer[256];  // 足够大的缓冲区
    int ret;
    int total_received = 0;
    
    if (client_fd < 0 || out_size == NULL) {
        LOG_Error("Invalid parameters");
        return -1;
    }
    
    memset(buffer, 0, sizeof(buffer));
    
    // 第一步：精确读取8字节（旧协议的MB值）
    while (total_received < 8) {
        ret = read(client_fd, buffer + total_received, 8 - total_received);
        if (ret <= 0) {
            LOG_Error("Failed to receive MB bytes, ret=%d\n", ret);
            return -1;
        }
        total_received += ret;
    }
    
    // 第二步：使用select检查是否还有更多数据（新协议的type+bytes）
    fd_set fds;
    struct timeval tv;
    FD_ZERO(&fds);
    FD_SET(client_fd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 50000;  // 50ms超时
    
    ret = select(client_fd + 1, &fds, NULL, NULL, &tv);
    
    if (ret > 0 && FD_ISSET(client_fd, &fds)) {
        // 有新数据，可能是新协议，继续读取9字节（type+bytes）
        int extra_received = 0;
        while (extra_received < 9) {
            ret = read(client_fd, buffer + 8 + extra_received, 9 - extra_received);
            if (ret <= 0) {
                break;
            }
            extra_received += ret;
        }
        
        if (extra_received >= 9) {
            // 新协议：MB(8) + type(1) + bytes(8)
            double size_bytes;
            memcpy(&size_bytes, &buffer[9], sizeof(double));
            *out_size = size_bytes;
            g_expectedFileSize = (long)size_bytes;
            LOG_Debug("New protocol (type=0x%02X): file size %.2f bytes (%.2f MB)",buffer[8], size_bytes, size_bytes / (1024.0 * 1024.0));
        } else {
            // 数据不完整，当作旧协议处理
            double size_mb;
            memcpy(&size_mb, buffer, sizeof(double));
            *out_size = size_mb;
            g_expectedFileSize = (long)(size_mb * 1024 * 1024);
            LOG_Debug("Partial new protocol (extra=%d), using legacy: file size %.2f MB (%ld bytes)",extra_received, size_mb, g_expectedFileSize);
        }
    } else {
        // 没有更多数据，旧协议：只有MB(8字节)
        double size_mb;
        memcpy(&size_mb, buffer, sizeof(double));
        *out_size = size_mb;
        g_expectedFileSize = (long)(size_mb * 1024 * 1024);
        LOG_Debug("Legacy protocol: file size %.2f MB (%ld bytes)",size_mb, g_expectedFileSize);
    }
    
    return 0;
}

static int CheckDiskSpace(double required_size_mb)
{
    int ret;
    
    RemoveUpgradeFile();
    ret = SpatialProcessingFunction(((long)required_size_mb + 2 + 5 + 100));
    if (ret < 0) {
        LOG_Error("Spatial Processing Function fail ");
        return -1;
    }
    
    return 0;
}

static int ReceiveAndWriteFile(int client_fd, const char *filepath, long *total_bytes_out, long *actual_size_out)
{
    FILE *file;
    unsigned char buffer[BUFFER_SIZE];
    long total_bytes_received = 0;
    long actual_file_size = 0;
    int upgrade_started = 0;
    
    if (client_fd < 0 || filepath == NULL) {
        LOG_Error("Invalid parameters");
        return -1;
    }
    
    file = fopen(filepath, "wb");
    if (file == NULL) {
        LOG_Error("Error opening or creating file ,ReceiveAndWriteFile\n");
        return -1;
    }
    
    LOG_Debug("Begin write file \n");
    
    while (1) {
        int bytes_received;
        int ret;
        
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = read(client_fd, buffer, BUFFER_SIZE);
        
        if (bytes_received <= 0) {
            LOG_Error("Connection closed or error occurred\n");
            fclose(file);
            return -1;
        }
        
        total_bytes_received += bytes_received;
        
        if (strncmp((char *)buffer, "end", 3) == 0) {
            LOG_Debug("End marker detected, checking file total_bytes_received:%d...\n",total_bytes_received);
            LOG_Debug("Expected: %ld bytes, Received: %ld bytes\n",g_expectedFileSize, actual_file_size);
            
            // 关键验证：接收大小必须大于等于期望大小
            if (actual_file_size >= g_expectedFileSize && g_expectedFileSize > 0) {
                LOG_Debug("File size validation passed\n");
                system("echo 20 > /oemdata/parameters/SlaveModemUpProgress");
                break;
            } else {
                // 大小不够，说明是误判（数据里有"end"）
                LOG_Warn("False end detection! Data contains 'end' string. Continue receiving...\n");
                // 把"end"当作正常数据写入
                ret = WriteFile(file, buffer, bytes_received);
                if (ret < 0) {
                    LOG_Error("Write File Fail \n");
                    fclose(file);
                    return -1;
                }
                actual_file_size += bytes_received;
                upgrade_started = 1;
                continue;  // 继续接收，不break
            }
        }
        
        ret = WriteFile(file, buffer, bytes_received);
        if (ret < 0) {
            LOG_Error("Write File Fail \n");
            fclose(file);
            return -1;
        }
        
        actual_file_size += bytes_received;
        upgrade_started = 1;
    }
    
    fclose(file);
    
    if (total_bytes_out != NULL) {
        *total_bytes_out = total_bytes_received;
    }
    
    if (actual_size_out != NULL) {
        *actual_size_out = actual_file_size;
    }
    
    LOG_Debug("modem_b total_bytes_received: %ld, actual_file_size: %ld\n", 
              total_bytes_received, actual_file_size);
    
    return upgrade_started ? 0 : -1;
}

static void SendUpgradeResult(int client_fd, int success)
{
    const char *msg;
    int ret;
    
    msg = success ? UPGRADESUCCESS : UPGRADEFAIL;
    ret = send(client_fd, msg, strlen(msg), 0);
    
    if (ret == -1) {
        LOG_Error("send failed");
        if (errno == EPIPE) {
            LOG_Error("Connection was closed by peer\n");
        }
    }
    sleep(1);
}

static int ProcessRebootCommand(int client_fd)
{
    unsigned char buffer[BUFFER_SIZE];
    int reboot_cmd;
    
    memset(buffer, 0, BUFFER_SIZE);
    
    reboot_cmd = read(client_fd, buffer, BUFFER_SIZE);
    LOG_Debug("buffer: %s\n", buffer);
    printf("buffer: %s\n", buffer);
    
    if (strstr((char*)buffer, "reboot_begin")) {
        system("echo 100 > /oemdata/parameters/SlaveModemUpProgress");
        Set_upgrade_mode(2);
        LOG_Debug("Upgrade MPU need switch partition");

        int ret = OtaSwitchAOrBPartition();
        if (ret < 0) {
            LOG_Error("OtaSwitchAOrBPartition fail ret = %d", ret);
        }
        SetsloveModemUpgradeStatus(2);
        sleep(10);
        return 1;
    }
    else if (strstr((char*)buffer, "update_failed")) {
        Set_upgrade_mode(2);
        LOG_Debug("Upgrade MPU update_failed");
        int ret = SrTboxSystemReboot("modem_b", REMOTE_CONTROL_REBOOT);
        if (ret < 0) {
            LOG_Error("SrTboxSystemReboot fail ret = %d", ret);
        }
        SetsloveModemUpgradeStatus(3);
        sleep(10);
        return 0;
    }
    
    return -1;
}

int BlockTheProcess()
{
    sigset_t mask;
    
    if (sigfillset(&mask) == -1) {
        LOG_Error("sigfillset error");
        return -1;
    }

    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        LOG_Error("sigprocmask error");
        return -1;
    }

    LOG_Debug("PID: %d\n", getpid());
    LOG_Debug("Blocking all signals (except SIGKILL/SIGSTOP)...\n");
    pause();

    LOG_Error("This line will not be printed!\n");
    return 0;
}

int main() {
    int server_fd = -1;
    int new_socket = -1;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int ret = 0;
    UpgradeStatus_E mcuStatus = UPGRADE_INIT;
    UpgradeStatus_E upgradeSta = UPGRADE_INIT;
    int timeout_counter = 0;
    const int MAX_TIMEOUT = 300; // 5分钟超时(300秒)
    printf("modem_b start\n");
    LOG_INIT("modem_b");
    system("rm /oemdata/upgrade -rf;");
    system("mkdir -p /oemdata/upgrade;");
    if (access(EMMCDEVICENODE, F_OK) == 0) {
        LOG_Debug("Secondary module code..... \n");
        printf("Secondary module code..... \n");
        BlockTheProcess();
        return 0;
    }
    printf("modem_b start 2 \n");
    // 检测异常断电后的升级状态
    // 如果上次升级过程中断电，重启后需要检测升级状态
    // 先检查升级目录是否存在，避免OtaGetUpgradeStatus访问不存在的路径
    if (access("/oemdata/upgrade", F_OK) != 0) {
        LOG_Debug("main: Upgrade directory does not exist, skip power-off upgrade check");
    } else {
        ret = ConcurrentOtaGetAllUpgradeStatus("/oemdata/upgrade/", &mcuStatus, &upgradeSta);
        if (ret == 0) {
            LOG_Debug("main: Initial upgrade status check = %d", upgradeSta);
            
            if (upgradeSta == UPGRADE_MPU_FLASHING) {
                LOG_Debug("main: Detected upgrading state after power off, waiting for completion...");
                
                while (timeout_counter < MAX_TIMEOUT) {
                    sleep(1);
                    timeout_counter++;
                    
                    // 检查目录是否仍然存在
                    if (access("/oemdata/upgrade", F_OK) != 0) {
                        LOG_Error("main: Upgrade directory disappeared during wait");
                        break;
                    }
                    
                    ret = ConcurrentOtaGetAllUpgradeStatus("/oemdata/upgrade/", &mcuStatus, &upgradeSta);
                    if (ret != 0) {
                        LOG_Error("main: Failed to get upgrade status in loop, ret = %d", ret);
                        continue;
                    }
                    
                    LOG_Debug("main: Waiting for upgrade completion... status = %d", upgradeSta);
                    if (upgradeSta == UPGRADE_MPU_NEED_BACKUP) {
                        LOG_Debug("main: Upgrade completed successfully, preparing to reboot");
                        system("echo 100 > /oemdata/parameters/SlaveModemUpProgress");
                        sleep(1);
                        SrTboxSystemReboot("otamodem2", REMOTE_CONTROL_REBOOT);
                        return 0;
                    }
                    
                    if (upgradeSta == UPGRADE_MPU_FAILED) {
                        LOG_Debug("main: Upgrade failed, preparing to reboot");
                        system("echo 0 > /oemdata/parameters/SlaveModemUpProgress");
                        sleep(1);
                        SrTboxSystemReboot("otamodem2", REMOTE_CONTROL_REBOOT);
                        return 0;
                    }
                }
                
                // 超时处理
                LOG_Error("main: Timeout waiting for upgrade completion");
                sleep(1);
                SrTboxSystemReboot("otamodem2", REMOTE_CONTROL_REBOOT);
                return 0;
            }
        } else {
            LOG_Debug("main: Failed to get upgrade status, ret = %d, continue normal startup", ret);
        }
    }
    
    // 正常启动流程
    system("echo 0 > /oemdata/parameters/SlaveModemUpProgress");
    LOG_Debug("Secondary module code starts running..... \n");
    sleep(10);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        LOG_Error("socket failed ");
        return -1;
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        LOG_Error("setsockopt fail ");
        CloseSocketSafe(&server_fd);
        return -1;
    }
    
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        LOG_Error("bind failed");
        CloseSocketSafe(&server_fd);
        return -1;
    }
    
    if (listen(server_fd, 3) < 0) {
        LOG_Error("listen fail");
        CloseSocketSafe(&server_fd);
        return -1;
    }
    
    LOG_Debug("Device 2 (Server) listening on port %d...\n", PORT);
    
    while (1) {
        int connection_ready;
        double received_value = 0;
        long total_bytes_received = 0;
        long actual_file_size = 0;
        int upgradeSta;
        int should_reboot;
        
        LOG_Debug("Waiting for device connection.....\n");
        
        connection_ready = WaitForSocketConnection(server_fd, 10);
        if (connection_ready <= 0) {
            continue;
        }
        
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            LOG_Error("accept error \n");
            continue;
        }
        
        SetUpgradeStatus(UPGRADE_STATE_RUNNING);
        LOG_Debug("New connection accepted\n");
        
        if (ReceiveFileSize(new_socket, &received_value) < 0) {
            SetUpgradeStatus(UPGRADE_STATE_FAILED);
            CloseSocketSafe(&new_socket);
            continue;
        }
        
        if (CheckDiskSpace(received_value) < 0) {
            SetUpgradeStatus(UPGRADE_STATE_FAILED);
            CloseSocketSafe(&new_socket);
            continue;
        }
        
        if (ReceiveAndWriteFile(new_socket, MPUBINFILEPATHMPUBIN, &total_bytes_received, &actual_file_size) < 0) {
            SetUpgradeStatus(UPGRADE_STATE_FAILED);
            CloseSocketSafe(&new_socket);
            continue;
        }
        
        LOG_Debug("modem_b total_bytes_received: %ld, received_value: %.2f, actual_file_size: %ld\n", 
                  total_bytes_received, received_value, actual_file_size);
        printf("modem_b total_bytes_received: %ld, received_value: %.2f, actual_file_size: %ld\n", 
               total_bytes_received, received_value, actual_file_size);
        
        Set_upgrade_mode(2);
        upgradeSta = DoUpgrade();
        LOG_Debug("---Upgrade status: %d\n", upgradeSta);
        
        if (UPGRADE_MPU_NEED_BACKUP == upgradeSta) {
            SendUpgradeResult(new_socket, 1);
            Set_upgrade_mode(2);
            
            should_reboot = ProcessRebootCommand(new_socket);
            SetUpgradeStatus(should_reboot >= 0 ? UPGRADE_STATE_SUCCESS : UPGRADE_STATE_FAILED);
        } else {
            SendUpgradeResult(new_socket, 0);
            SetUpgradeStatus(UPGRADE_STATE_FAILED);
        }
        
        CloseSocketSafe(&new_socket);
    }
    
    CloseSocketSafe(&server_fd);
    return 0;
}