#ifndef __UPGRADE_INTERFACE_H__
#define __UPGRADE_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <sys/time.h>

#define     DOWNLOADUPGRADEPACKAGEFILEPATH     "/oemdata/upgrade/download/Tbox.tar.gz"
#define     PATHUPGRADE                        "/oemdata/upgrade/"
#define     TBOXUPGRADEPACKAGEFILEPATH         "/oemdata/upgrade/download/"
#define     MD5DATA                            "/oemdata/upgrade/md5.txt"


#define     MCU1BINFILEPATH                    "/oemdata/upgrade/mcu_1.bin"
#define     MCU2BINFILEPATH                    "/oemdata/upgrade/mcu_2.bin"
#define     MPUBINFILEPATHMPUBIN               "/oemdata/upgrade/update.zip"

#define     BT1BINFILEPATH                     "/oemdata/upgrade/bt_1.bin"
#define     BT2BINFILEPATH                     "/oemdata/upgrade/bt_2.bin"

// 升级时间统计结构
typedef struct {
    struct timeval unzipStart;
    struct timeval unzipEnd;
    double unzipDuration;
    
    struct timeval mcuStart;
    struct timeval mcuEnd;
    double mcuDuration;
    
    struct timeval mpuStart;
    struct timeval mpuEnd;
    double mpuDuration;
} UpgradeTiming;

// 升级协调器结构
typedef struct {
    int mcuCompleted;
    int mpuCompleted;
    int mcuResult;
    int mpuResult;
    pthread_mutex_t statusMutex;
    pthread_cond_t allCompleteCond;
} UpgradeCoordinator;

int DoUpgrade();
int ota_unzip_file(char *upgradefilepath);
void ota_delete_file(void);

// 升级线程函数
void* mcu_upgrade_thread(void* arg);
void* mpu_upgrade_thread(void* arg);

// 升级执行函数
int LordMcuUpgrade();
int LordMpuUpgrade();

// 等待下一步指令（重启或切换分区）
int WaitForNextCommand(int socket_fd, int timeout_seconds);

#ifdef __cplusplus
}
#endif
#endif
