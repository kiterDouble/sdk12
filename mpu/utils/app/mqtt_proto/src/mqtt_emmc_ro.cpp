#define _GNU_SOURCE
#include <stdint.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <time.h>
#include "sr_log.h"
#include "err_code.h"
#include "common_functions.h"
#include "mcu_api.h"

#define EMMC_MOUNT_STATUS_NONE  (0)
#define EMMC_MOUNT_STATUS_RO    (1)
#define EMMC_MOUNT_STATUS_RW    (2)
#define EMMC_FILE_LIMIT_CMD     "tail -1000 /oemdata/log/emmc_detect.log  > /tmp/emmc_detect.log && \
                                    mv /tmp/emmc_detect.log /oemdata/log/emmc_detect.log"


typedef struct 
{
    char device[128];
    char mountpoint[128];
}MMCDevInFo_t;

typedef struct
{
    uint8_t emmcStatus;
}MMCDevStatus_t;

static int32_t AppendFileWithHeader(const char *srcFile, const char *dstFile) 
{
    int32_t fdSrc = -1;
    int32_t fdDst = -1;
    off_t offset = 0;
    char fileConten[2048] = {0};
    char timeBuff[128] = {0};
    char header[512] = {0};
    int32_t readLen = 0;
    time_t now = time(NULL);
    
    struct tm *tmInfo = localtime(&now);

    if (tmInfo == NULL) 
    {
        LOG_Error("%s tmInfo is NULL !!!", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == srcFile || NULL == dstFile)
    {
        LOG_Error("%s srcFile is NULL  or dstFile is NULL !!!", __FUNCTION__ );
        return RET_ERR;
    }

    fdSrc = open(srcFile, O_RDONLY);
    if (fdSrc < 0) 
    {
        LOG_Error("%s open %s error", __FUNCTION__, srcFile);
        return RET_ERR;
    }

    // 打开目标文件：追加模式，不存在则创建
    fdDst = open(dstFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fdDst < 0) 
    {
        close(fdSrc);
        LOG_Error("%s open %s error", __FUNCTION__, dstFile);
        return RET_ERR;
    }

    strftime(timeBuff, sizeof(timeBuff), "[%Y-%m-%d %H:%M:%S] Appending file: ", tmInfo);
    snprintf(header, sizeof(header), "%s%s\n", timeBuff, srcFile);

    if (write(fdDst, header, strlen(header)) != strlen(header)) 
    {
        close(fdSrc);
        close(fdDst);
        LOG_Error("%s write  header [%s]  is error", __FUNCTION__, header);
        return RET_ERR;
    }
    
    readLen = read(fdSrc, fileConten, sizeof(fileConten));
    if (readLen < 0) 
    {
        close(fdSrc);
        close(fdDst);
        LOG_Error("%s read %s error", __FUNCTION__, srcFile);
        return RET_ERR;
    }

    if(write(fdDst, fileConten, readLen) != readLen)
    {
        close(fdSrc);
        close(fdDst);
        LOG_Error("%s write %s fileConten error", __FUNCTION__, srcFile);
        return RET_ERR;
    }

    close(fdSrc);
    close(fdDst);
    return RET_OK;
}


static int32_t DetectRoPartition(const char *device, const char *mountpoint) 
{
    FILE *fileFp = NULL;
    char *fileLine;
    size_t oneLinelen = 0;
    int32_t roFound = EMMC_MOUNT_STATUS_NONE;
    struct stat fileStat;
    int32_t scanfRet = 0;
    char dev[128] = {0};
    char mnt[128] = {0};
    char type[32] = {0};
    char opts[256] = {0};

    if (NULL  == device || NULL == mountpoint) 
    {
        LOG_Error("用法: fix_ro_partition <设备> <挂载点>");
        return RET_ERR;
    }

    if (0 != stat((char *)device, &fileStat)) 
    {
        LOG_Error("错误: 设备 %s 不存在", device);
        return RET_ERR;
    }

    if (!S_ISBLK(fileStat.st_mode)) 
    {
        LOG_Error("错误: 设备 %s 不是块设备", device);
        return RET_ERR;
    }

    // 检查挂载状态（解析 /proc/mounts）
    fileFp = fopen("/proc/mounts", "r");
    if (NULL == fileFp) 
    {
        LOG_Error("无法打开 /proc/mounts");
        return RET_ERR;
    }

    while (getline(&fileLine, &oneLinelen, fileFp) != -1) 
    {
        memset(dev, 0, 128);
        memset(mnt, 0, 128);
        memset(type, 0, 32);
        memset(opts, 0, 256);
        scanfRet = sscanf(fileLine, "%127s %127s %31s %255s", dev, mnt, type, opts);
        if (scanfRet >= 4 && 0 == strcmp(dev, device) && 0 == strcmp(mnt, mountpoint)) 
        {
            if (strstr(opts, "ro") != NULL) 
            {
                roFound = 1;
            }
            else if (strstr(opts, "rw") != NULL) 
            {
                roFound = 2;
            }
            LOG_Debug("mount opts %s", opts);
            break;
        }
    }
    fclose(fileFp);
    free(fileLine);
    fileLine = NULL;

    return roFound;
}

static int32_t ReMountPartition(const char *device, const char *mountpoint)
{
    struct stat fileStat;

    if (NULL  == device || NULL == mountpoint) 
    {
        LOG_Error("用法: fix_ro_partition <设备> <挂载点>");
        return RET_ERR;
    }

    if (0 != stat(device, &fileStat)) 
    {
        LOG_Error("错误: 设备 %s 不存在", device);
        return RET_ERR;
    }

    if (!S_ISBLK(fileStat.st_mode)) 
    {
        LOG_Error("错误: 设备 %s 不是块设备", device);
        return RET_ERR;
    }

    if (umount2(mountpoint, 2) != 0) 
    {
        if (errno == EBUSY)
        {
            LOG_Error("错误: 设备 %s is busy", device);
        }
        LOG_Error("错误:卸载 设备 %s 失败", device);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("卸载 %s 成功", device);
    }

    // 3. 重新挂载为读写
    if (mount(device, mountpoint, "ext4", 0, NULL) == 0) 
    {
        LOG_Debug("修复完成，分区已重新挂载为读写");
        return RET_OK;
    }
    else 
    {
        LOG_Error("错误: 重新挂载失败: %s", strerror(errno));
        return RET_ERR;
    }
}

static int32_t EMMCLogRecords(void)
{
    AppendFileWithHeader("/sys/kernel/debug/mmc0/ios", "/oemdata/log/emmc_detect.log");
    AppendFileWithHeader("/sys/kernel/debug/mmc0/err_stats", "/oemdata/log/emmc_detect.log");
    AppendFileWithHeader("/sys/kernel/debug/mmc0/err_state", "/oemdata/log/emmc_detect.log");
    AppendFileWithHeader("/sys/kernel/debug/clk/gcc_sdcc1_apps_clk/clk_measure", "/oemdata/log/emmc_detect.log");
    AppendFileWithHeader("/sys/kernel/debug/mmc0/mmc0:0001/ext_csd", "/oemdata/log/emmc_detect.log");
    AppendFileWithHeader("/sys/kernel/debug/mmc0/mmc0:0001/state", "/oemdata/log/emmc_detect.log");
    AppendFileWithHeader("/sys/kernel/debug/mmc0/mmc0:0001/status", "/oemdata/log/emmc_detect.log");
    AppendFileWithHeader("/sys/block/mmcblk0/ro", "/oemdata/log/emmc_detect.log");
    AppendFileWithHeader("/sys/block/mmcblk0/mmcblk0p1/ro", "/oemdata/log/emmc_detect.log");
    AppendFileWithHeader("/sys/block/mmcblk0/mmcblk0p2/ro", "/oemdata/log/emmc_detect.log");
    AppendFileWithHeader("/sys/block/mmcblk0/mmcblk0p3/ro", "/oemdata/log/emmc_detect.log");
    AppendFileWithHeader("/sys/block/mmcblk0/mmcblk0p4/ro", "/oemdata/log/emmc_detect.log");
    return RET_OK;
}

void *DetectEMMCRoTHread(void *)
{
    int32_t writeRoLog = 0;
    int32_t mmcInfoIndex = 0;
    int32_t mmcStatus = 0;
    McuMesg_t sendToMcu;
    uint8_t sendBuff[128] = {0};
    MMCDevInFo_t mmcInfo[] = {{.device = "/dev/mmcblk0p1", .mountpoint = "/mnt/emmc"  }, 
                              {.device = "/dev/mmcblk0p2", .mountpoint = "/mnt/srlog" },
                              {.device = "/dev/mmcblk0p3", .mountpoint = "/mnt/canlog"},
                              {.device = "/dev/mmcblk0p4", .mountpoint = "/mnt/carota"}};

    system(EMMC_FILE_LIMIT_CMD);
    EMMCLogRecords();
    while(1)
    {
        mmcStatus = DetectRoPartition(mmcInfo[mmcInfoIndex].device, mmcInfo[mmcInfoIndex].mountpoint);
        if(EMMC_MOUNT_STATUS_RO == mmcStatus && 0x00 == (writeRoLog & (0x01 << mmcInfoIndex)))
        {
            EMMCLogRecords();
            writeRoLog |= (0x01 << mmcInfoIndex);
            ReMountPartition(mmcInfo[mmcInfoIndex].device, mmcInfo[mmcInfoIndex].mountpoint);
        }
        mmcStatus = DetectRoPartition(mmcInfo[mmcInfoIndex].device, mmcInfo[mmcInfoIndex].mountpoint);
        if (EMMC_MOUNT_STATUS_RW == mmcStatus) 
        {
            writeRoLog &= ~(1 << mmcInfoIndex);
        }

        mmcInfoIndex++;
        if(mmcInfoIndex >= (sizeof(mmcInfo) / sizeof(MMCDevInFo_t)))
        {
            mmcInfoIndex = 0;
            sleep(600);
        }
        else
        {
            sendToMcu.type = (SYSQueMsgType_e)(SYS_QUE_TYPE_MCU_VEHICLE_EMMC_RO_STATUS_SYNC_TO_MCU);
            sendToMcu.payloadLen = sizeof(MMCDevStatus_t);
            sendToMcu.payload = sendBuff;
            
            McuSrvSendMesg(&sendToMcu);
            sleep(5);
        }        
    }
}