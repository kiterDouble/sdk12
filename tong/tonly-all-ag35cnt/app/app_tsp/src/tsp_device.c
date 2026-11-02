#include "tsp_device.h"
#include "tsp_log.h"
#include "tsp_vin.h"

#include "sr_param.h"
#include "err_code.h"
#include "can_signal_header_tonly.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

/* CAN数据导出: 源目录 */
#define CAN_LOG_DIR  "/mnt/sdcard/can_log"
/* CAN数据导出: 临时存放tgz的目录 */
#define EXPORT_TMP_DIR  "/mnt/sdcard/tly_export"

/* 并发保护: 平台连续下发多条上传指令时避免并行执行 */
static volatile int g_sd_upload_busy = 0;

extern int GetCanSignalData(TonlyAllCanSignal_canVal_t *outData);

int tly_device_get_vin(char *buf, int buf_len)
{
#if 0
    const char *vin = "LSNZLFYA1P1000001";
    int len = 17 < buf_len ? 17 : buf_len - 1;
    memcpy(buf, vin, len);
    buf[len] = '\0';
    TLY_LOG_D("[STUB] tly_device_get_vin: %s", buf);
#else
    (void)buf_len;

    int ret = RET_ERR;
    ParamInfo_t param = {0};
    char vin_buf[VIN_CAN1_VIN_LENGTH + 1] = {0};

    TLY_LOG_D("%s enter", __func__);

    param.id = E_ParamId_VIN;
    ret = GetParam(&param);
    if (ret != RET_OK) {
        TLY_LOG_E("get param[VIN] fail");
        return ret;
    }

    /* 检查VIN是否全为0 */
    if (vin_is_all_zero((const char *)param.value, VIN_CAN1_VIN_LENGTH)) {
        /* VIN为全0，使用SN的后17位替代 */
        ParamInfo_t sn_param = {0};
        sn_param.id = E_ParamId_SN;
        if (GetParam(&sn_param) == RET_OK) {
            int sn_len = strlen((const char *)sn_param.value);
            int copy_len = (sn_len >= VIN_CAN1_VIN_LENGTH) ? VIN_CAN1_VIN_LENGTH : sn_len;
            int offset = (sn_len >= VIN_CAN1_VIN_LENGTH) ? (sn_len - VIN_CAN1_VIN_LENGTH) : 0;
            memcpy(vin_buf, (const char *)sn_param.value + offset, copy_len);
            vin_buf[copy_len] = '\0';
            TLY_LOG_I("tly_device_get_vin: VIN is all zero, use SN last 17 chars: %s", vin_buf);
        } else {
            TLY_LOG_W("tly_device_get_vin: VIN is zero and SN get failed");
            memcpy(vin_buf, param.value, VIN_CAN1_VIN_LENGTH);
            vin_buf[VIN_CAN1_VIN_LENGTH] = '\0';
        }
    } else {
        memcpy(vin_buf, param.value, VIN_CAN1_VIN_LENGTH);
        vin_buf[VIN_CAN1_VIN_LENGTH] = '\0';
    }

    strcpy(buf, vin_buf);

    TLY_LOG_D("tly_device_get_vin: %s", buf);

    TLY_LOG_D("%s exit", __func__);
#endif
    return 0;
}

int tly_device_get_iccid(char *buf, int buf_len)
{
#if 0
    const char *iccid = "898600MFSSYYG123456P";
    int len = 20 < buf_len ? 20 : buf_len - 1;
    memcpy(buf, iccid, len);
    buf[len] = '\0';
    TLY_LOG_D("[STUB] tly_device_get_iccid: %s", buf);
#else
    (void)buf_len;


    int ret = RET_ERR;
    ParamInfo_t param = {0};

    TLY_LOG_D("%s enter", __func__);

    param.id = E_ParamId_ICCID;
    ret = GetParam(&param);
    if (ret != RET_OK) {
        TLY_LOG_E("get param[ICCID] fail");
        return ret;
    }

    strcpy(buf, (const char *)param.value);

    TLY_LOG_D("tly_device_get_iccid: %s", buf);

    TLY_LOG_D("%s exit", __func__);

#endif
    return 0;
}

uint8_t tly_device_get_sw_version(void)
{



    TLY_LOG_D("[STUB] tly_device_get_sw_version: 1");
    return 1;
}

uint16_t tly_device_get_hw_version(void)
{



    TLY_LOG_D("[STUB] tly_device_get_hw_version: 1");
    return 1;
}

uint32_t tly_device_get_main_mcu_version(void)
{
    uint32_t main_mcu_version = 0;
    TLY_LOG_D("%s enter", __func__);

    /* 从文件读取版本号: /oemapp/version.txt    
     * 取前9位MPU版本的后3位作为main_mcu_version     
     */
    FILE *file = fopen("/oemapp/version.txt", "r");
    if (file == NULL) {
        TLY_LOG_E("open /oemapp/version.txt failed");
        return 1;
    }

    char version[16] = {0};
    if (fgets(version, sizeof(version), file) == NULL) {
        TLY_LOG_E("read version.txt failed");
        fclose(file);
        return 1;
    }
    fclose(file);

    int version_len = strlen(version);
    if (version_len >= 9) {
        char main_version_str[4] = {0};
        memcpy(main_version_str, version + 6, 3);  // 取前9位的后3位
        main_mcu_version = (uint32_t)strtoul(main_version_str, NULL, 10);
        TLY_LOG_D("main_mcu_version: %u", main_mcu_version);
    } else {
        TLY_LOG_E("version string too short: %s (len=%d)", version, version_len);
        return 1;
    }

    TLY_LOG_D("%s exit, main_mcu_version=0x%08X", __func__, main_mcu_version);
    return main_mcu_version;
}

uint16_t tly_device_get_sub_mcu_version(void)
{
    uint16_t sub_mcu_version = 0;
    TLY_LOG_D("%s enter", __func__);

    /* 直接从文件读取版本号: /oemapp/version.txt
     * 格式: 235000001001 (12位)
     * 取后3位作为sub_mcu_version    
     */
    FILE *file = fopen("/oemapp/version.txt", "r");
    if (file == NULL) {
        TLY_LOG_E("open /oemapp/version.txt failed");
        return 1;
    }

    char version[16] = {0};
    if (fgets(version, sizeof(version), file) == NULL) {
        TLY_LOG_E("read version.txt failed");
        fclose(file);
        return 1;
    }
    fclose(file);

    int version_len = strlen(version);
    if (version_len >= 12) {
        char sub_version_str[4] = {0};
        memcpy(sub_version_str, version + 9, 3);  // 取后3位
        sub_mcu_version = (uint16_t)strtoul(sub_version_str, NULL, 10);
        TLY_LOG_D("sub_mcu_version: %u", sub_mcu_version);
    } else {
        TLY_LOG_E("version string too short: %s (len=%d)", version, version_len);
        return 1;
    }

    TLY_LOG_D("%s exit, sub_mcu_version=0x%04X", __func__, sub_mcu_version);
    return sub_mcu_version;
}

uint16_t tly_device_get_non_stage4_version(void)
{
    TLY_LOG_D("[STUB] tly_device_get_non_stage4_version: 1");
    return 1;
}

int tly_device_battery_heat(uint32_t time_second)
{
    TLY_LOG_I("[STUB] battery_heat: time_second=%u", time_second);
    return 0;
}

/*
 * 带超时的 waitpid: WNOHANG 轮询(100ms粒度), 超时后 SIGKILL
 * 返回值: 0=正常退出, -1=超时被kill, -2=waitpid异常
 */
static int waitpid_timeout(pid_t pid, int timeout_sec, int *out_status)
{
    int elapsed_ms = 0;
    int timeout_ms = timeout_sec * 1000;
    int status = 0;
    pid_t rc;

    while (elapsed_ms < timeout_ms) {
        rc = waitpid(pid, &status, WNOHANG);

        if (rc == pid) {
            /* 子进程已退出 */
            if (out_status) *out_status = status;
            return 0;
        }

        if (rc < 0) {
            /* ECHILD: 子进程已不存在; 其他错误 */
            if (errno == ECHILD) return 0;
            TLY_LOG_E("waitpid_timeout: waitpid error, errno=%d", errno);
            return -2;
        }

        /* 子进程仍在运行, 等待 100ms */
        usleep(100000);  /* 100ms */
        elapsed_ms += 100;
    }

    /* 超时: 强制杀死子进程 */
    TLY_LOG_E("waitpid_timeout: pid %d timeout after %ds, sending SIGKILL",
              (int)pid, timeout_sec);
    if (kill(pid, SIGKILL) < 0) {
        if (errno == ESRCH) {
            TLY_LOG_W("waitpid_timeout: pid %d already exited before SIGKILL", (int)pid);
        } else {
            TLY_LOG_E("waitpid_timeout: kill(SIGKILL) failed, errno=%d", errno);
        }
    }

    /* 再等 5 秒确认 kill 生效 */
    int kill_wait_ms = 0;
    while (kill_wait_ms < 5000) {
        rc = waitpid(pid, &status, WNOHANG);
        if (rc == pid) {
            /* 成功回收子进程 */
            if (out_status) *out_status = status;
            return -1;
        }
        if (rc < 0 && errno == ECHILD) {
            /* 子进程已被其他方式回收 */
            return -1;
        }
        usleep(100000);
        kill_wait_ms += 100;
    }

    /* 5秒后仍未回收, 记录严重错误 */
    TLY_LOG_E("waitpid_timeout: pid %d may become zombie (uninterruptible sleep?)",
              (int)pid);
    return -1;
}

/*
 * 执行外部命令（fork/exec/wait 封装）
 * 参数:
 *   cmd: 命令路径（如 "/bin/tar"）
 *   args: 命令参数数组（以 NULL 结尾）
 *   timeout_sec: 超时时间（秒）
 * 返回值: 0=成功, -1=超时, -2=其他错误
 */
static int run_command(const char *cmd, char *const args[], int timeout_sec)
{
    pid_t pid = fork();
    if (pid < 0) {
        TLY_LOG_E("run_command: fork failed: %s", strerror(errno));
        return -2;
    }

    if (pid == 0) {
        /* 子进程：关闭非标准文件描述符 */
        int max_fd = (int)sysconf(_SC_OPEN_MAX);
        if (max_fd <= 0) max_fd = 1024;
        for (int fd = 3; fd < max_fd; fd++) {
            close(fd);
        }
        /* 执行命令 */
        execv(cmd, args);
        _exit(127);
    }

    /* 父进程：等待子进程完成 */
    int status;
    int rc = waitpid_timeout(pid, timeout_sec, &status);

    if (rc != 0) {
        TLY_LOG_E("run_command: %s timeout or error, rc=%d", cmd, rc);
        return rc;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        TLY_LOG_E("run_command: %s failed, exit=%d, sig=%d",
                  cmd,
                  WIFEXITED(status) ? WEXITSTATUS(status) : -1,
                  WIFSIGNALED(status) ? WTERMSIG(status) : 0);
        return -2;
    }

    return 0;
}

/*
 * 将年月日时分秒转换为 time_t
 * 参数:
 *   year: 年份（如 2024）
 *   mon: 月份（1-12）
 *   mday: 日期（1-31）
 *   hour: 小时（0-23）
 *   min: 分钟（0-59）
 *   sec: 秒（0-59）
 * 返回值: 成功返回 time_t，失败返回 (time_t)-1
 */
static time_t datetime_to_time_t(int year, int mon, int mday,
                                  int hour, int min, int sec)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = year - 1900;
    tm.tm_mon  = mon - 1;
    tm.tm_mday = mday;
    tm.tm_hour = hour;
    tm.tm_min  = min;
    tm.tm_sec  = sec;
    tm.tm_isdst = 0;
    return mktime(&tm);
}

/*
 * 清理目录中匹配指定模式的文件
 * 参数:
 *   dir_path: 目录路径
 *   prefix: 文件名前缀
 *   suffix: 文件名后缀
 *   desc: 日志描述（如 "tgz", "filelist"）
 */
static void cleanup_files_by_pattern(const char *dir_path, const char *prefix,
                                      const char *suffix, const char *desc)
{
    DIR *dir = opendir(dir_path);
    if (!dir) return;

    struct dirent *entry;
    size_t prefix_len = strlen(prefix);

    while ((entry = readdir(dir)) != NULL) {
        /* 跳过隐藏文件 */
        if (entry->d_name[0] == '.') continue;

        /* 检查文件名前缀和后缀匹配 */
        if (strncmp(entry->d_name, prefix, prefix_len) != 0) continue;
        if (strstr(entry->d_name + prefix_len, suffix) == NULL) continue;

        /* 使用 stat 确认是普通文件 (某些文件系统 d_type 不可靠) */
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        struct stat st;
        if (stat(full_path, &st) != 0) continue;
        if (!S_ISREG(st.st_mode)) continue;

        unlink(full_path);
        TLY_LOG_D("sd_card_upload: cleanup residual %s: %s", desc, full_path);
    }
    closedir(dir);
}

int tly_device_sd_card_upload(const char *username, const char *password,
                               const char *ftp_url, uint16_t port,
                               const uint8_t begin_time[6], uint16_t time_count)
{
    int ret = -1;
    FILE *fl = NULL;
    DIR *dir = NULL;
    char tgz_path[512] = {0};
    char filelist_path[512] = {0};

    /* 并发保护: 平台连续下发多条上传指令时避免并行执行 */
    if (g_sd_upload_busy) {
        TLY_LOG_W("sd_card_upload: already busy, reject duplicate request");
        return -EBUSY;
    }
    g_sd_upload_busy = 1;

    TLY_LOG_I("sd_card_upload: user=%s, url=%s, port=%u, "
              "begin=20%02u-%02u-%02u %02u:%02u:%02u, count=%u",
              username ? username : "", ftp_url ? ftp_url : "", port,
              begin_time[0], begin_time[1], begin_time[2],
              begin_time[3], begin_time[4], begin_time[5], time_count);

    if (!ftp_url || !ftp_url[0]) {
        TLY_LOG_E("sd_card_upload: ftp_url is empty");
        goto cleanup;
    }

    /* 1. 解析 begin_time 为 time_t, 计算结束时间 */
    time_t t_begin = datetime_to_time_t(2000 + begin_time[0], begin_time[1], begin_time[2],
                                         begin_time[3], begin_time[4], begin_time[5]);
    if (t_begin == (time_t)-1) {
        TLY_LOG_E("sd_card_upload: mktime failed for begin_time");
        goto cleanup;
    }

    /* 时间范围: 以 begin_time 为中心, 前后各 time_count 秒, 最大600秒 */
    uint16_t time_range_sec = time_count;
    if (time_range_sec > 600) time_range_sec = 600;
    time_t t_start = t_begin - (time_t)time_range_sec;
    time_t t_end   = t_begin + (time_t)time_range_sec;

    TLY_LOG_I("sd_card_upload: time range [%ld, %ld] (±%us)",
              (long)t_start, (long)t_end, time_range_sec);

    /* 2. 生成 tgz 文件名: export+打包压缩日期和时间.tgz */
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    char tgz_name[128];
    snprintf(tgz_name, sizeof(tgz_name), "export%04d%02d%02d%02d%02d%02d.tgz",
             tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
             tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);

    /* 3. 确保临时目录存在 */
    struct stat st_dir;
    if (stat(EXPORT_TMP_DIR, &st_dir) != 0 || !S_ISDIR(st_dir.st_mode)) {
        char *mkdir_args[] = {"mkdir", "-p", EXPORT_TMP_DIR, NULL};
        if (run_command("/bin/mkdir", mkdir_args, 10) != 0) {
            TLY_LOG_E("sd_card_upload: mkdir -p %s failed", EXPORT_TMP_DIR);
            goto cleanup;
        }
    }

    int n = snprintf(tgz_path, sizeof(tgz_path), "%s/%s", EXPORT_TMP_DIR, tgz_name);
    if (n < 0 || n >= (int)sizeof(tgz_path)) {
        TLY_LOG_E("sd_card_upload: tgz_path truncated, n=%d", n);
        goto cleanup;
    }

    /* 4. 扫描 /mnt/sdcard/can_log 目录, 筛选时间范围内的文件 */
    n = snprintf(filelist_path, sizeof(filelist_path), "%s/filelist_%s.txt",
                 EXPORT_TMP_DIR, tgz_name);
    if (n < 0 || n >= (int)sizeof(filelist_path)) {
        TLY_LOG_E("sd_card_upload: filelist_path truncated, n=%d", n);
        goto cleanup;
    }

    /* 清理上次异常中断可能残留的文件 (掉电等场景) */
    cleanup_files_by_pattern(EXPORT_TMP_DIR, "export", ".tgz", "tgz");
    cleanup_files_by_pattern(EXPORT_TMP_DIR, "filelist_", ".txt", "filelist");

    fl = fopen(filelist_path, "w");
    if (!fl) {
        TLY_LOG_E("sd_card_upload: cannot create filelist %s", filelist_path);
        goto cleanup;
    }

    dir = opendir(CAN_LOG_DIR);
    int file_count = 0;
    if (dir) {
        struct dirent *entry;
        char fullpath[512];
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;

            int year, mon, mday, hour, min, sec;
            unsigned seq;
            /* 解析文件名时间戳: can_YYYYMMDD_HHMMSS_NNNN.blf (canLogger 格式) */
            if (sscanf(entry->d_name, "can_%04d%02d%02d_%02d%02d%02d_%04u.blf",
                       &year, &mon, &mday, &hour, &min, &sec, &seq) != 7) {
                continue;  /* 非 canLogger 数据文件，跳过 */
            }

            /* 年份范围校验，过滤垃圾文件 */
            if (year < 2020 || year > 2099) continue;

            /* stat 仅用于确认是普通文件 */
            snprintf(fullpath, sizeof(fullpath), "%s/%s", CAN_LOG_DIR, entry->d_name);
            struct stat st;
            if (stat(fullpath, &st) != 0) continue;
            if (!S_ISREG(st.st_mode)) continue;

            /* 用文件名时间做匹配, 而非 st_mtime (后者可能因 cp/touch 等偏移) */
            time_t t_file = datetime_to_time_t(year, mon, mday, hour, min, sec);
            if (t_file == (time_t)-1) {
                TLY_LOG_D("sd_card_upload: skip invalid timestamp: %s", entry->d_name);
                continue;
            }

            if (t_file >= t_start && t_file <= t_end) {
                /* 仅写文件名，配合 tar -C 切换目录，避免归档内携带绝对路径 */
                fprintf(fl, "%s\n", entry->d_name);
                file_count++;
                TLY_LOG_D("sd_card_upload: include [%s] file_time=%ld",
                          entry->d_name, (long)t_file);
            }
        }
        closedir(dir);
        dir = NULL;
    } else {
        TLY_LOG_W("sd_card_upload: cannot open dir %s: %s", CAN_LOG_DIR, strerror(errno));
        goto cleanup;
    }
    fclose(fl);
    fl = NULL;

    if (file_count == 0) {
        TLY_LOG_W("sd_card_upload: no files in range, nothing to upload");
        ret = 0;
        goto cleanup;
    }

    TLY_LOG_I("sd_card_upload: found %d file(s) to package", file_count);

    /* 5. 使用 tar 打包为 tgz (超时 120s) */
    char *tar_args[] = {"tar", "czf", tgz_path, "-C", CAN_LOG_DIR, "-T", filelist_path, NULL};
    int tar_rc = run_command("/bin/tar", tar_args, 120);
    /* filelist_path 会在 cleanup 中统一清理 */

    if (tar_rc != 0) {
        TLY_LOG_E("sd_card_upload: tar failed, rc=%d", tar_rc);
        unlink(tgz_path);
        goto cleanup;
    }

    /* 验证 tgz */
    struct stat st_tgz;
    if (stat(tgz_path, &st_tgz) != 0 || st_tgz.st_size == 0) {
        TLY_LOG_E("sd_card_upload: tgz empty or missing: %s", tgz_path);
        unlink(tgz_path);
        goto cleanup;
    }
    TLY_LOG_I("sd_card_upload: tgz created: %s (%ld bytes)",
              tgz_path, (long)st_tgz.st_size);

    /* 6. fork+execl 使用 curl 上传 tgz 到 FTP 服务器 (超时 600s) */
    char curl_url[1024];
    n = snprintf(curl_url, sizeof(curl_url), "%s/%s", ftp_url, tgz_name);
    if (n < 0 || n >= (int)sizeof(curl_url)) {
        TLY_LOG_E("sd_card_upload: curl_url truncated, n=%d", n);
        unlink(tgz_path);
        goto cleanup;
    }

    char userpwd[256] = {0};
    if (username && username[0]) {
        /* 统一处理空密码情况 */
        const char *pwd = (password && password[0]) ? password : "";
        n = snprintf(userpwd, sizeof(userpwd), "%s:%s", username, pwd);
        if (n < 0 || n >= (int)sizeof(userpwd)) {
            TLY_LOG_E("sd_card_upload: userpwd truncated, n=%d", n);
            unlink(tgz_path);
            goto cleanup;
        }
    }

    TLY_LOG_I("sd_card_upload: uploading to %s", curl_url);

    /* 构建 curl 参数数组 */
    char *curl_args[32];
    int curl_argc = 0;
    curl_args[curl_argc++] = "curl";
    curl_args[curl_argc++] = "--silent";
    curl_args[curl_argc++] = "--show-error";
    curl_args[curl_argc++] = "--fail";
    curl_args[curl_argc++] = "--ftp-pasv";
    curl_args[curl_argc++] = "--disable-epsv";
    curl_args[curl_argc++] = "--ftp-skip-pasv-ip";
    curl_args[curl_argc++] = "-T";
    curl_args[curl_argc++] = tgz_path;
    curl_args[curl_argc++] = "--connect-timeout";
    curl_args[curl_argc++] = "30";
    curl_args[curl_argc++] = "--max-time";
    curl_args[curl_argc++] = "600";
    if (userpwd[0]) {
        curl_args[curl_argc++] = "-u";
        curl_args[curl_argc++] = userpwd;
    }
    curl_args[curl_argc++] = curl_url;
    curl_args[curl_argc++] = NULL;
    /* 安全检查: 确保没有越界 */
    if (curl_argc > (int)(sizeof(curl_args)/sizeof(curl_args[0]))) {
        TLY_LOG_E("sd_card_upload: curl args buffer overflow, argc=%d", curl_argc);
        unlink(tgz_path);
        goto cleanup;
    }

    int curl_rc = run_command("/usr/bin/curl", curl_args, 630);
    if (curl_rc != 0) {
        TLY_LOG_E("sd_card_upload: curl upload failed, rc=%d", curl_rc);
        /* 上传失败删除 tgz 避免占用空间 */
        unlink(tgz_path);
        goto cleanup;
    }

    TLY_LOG_I("sd_card_upload: FTP upload successful");

    /* 7. 删除本地 tgz 压缩包 (源文件不动) */
    if (unlink(tgz_path) == 0) {
        TLY_LOG_D("sd_card_upload: deleted local tgz: %s", tgz_path);
    } else {
        TLY_LOG_W("sd_card_upload: failed to delete local tgz: %s: %s",
                  tgz_path, strerror(errno));
    }

    ret = 0;

cleanup:
    /* 统一资源清理 */
    if (fl) {
        fclose(fl);
        fl = NULL;
    }
    if (dir) {
        closedir(dir);
        dir = NULL;
    }
    /* 清理可能残留的文件列表文件 */
    if (filelist_path[0] != '\0') {
        unlink(filelist_path);
        filelist_path[0] = '\0';
    }
    if (g_sd_upload_busy) {
        g_sd_upload_busy = 0;
    }
    TLY_LOG_I("sd_card_upload: finished, ret=%d", ret);
    return ret;
}

int tly_device_vin_unbind(const char *target_vin)
{
    TLY_LOG_I("[STUB] vin_unbind: target_vin=%s", target_vin);
    return 0;
}

int tly_device_vin_bind(const char *target_vin)
{
    int ret;
    
    if (!target_vin) {
        TLY_LOG_E("[STUB] vin_bind: target_vin is NULL");
        return -1;
    }
    
    TLY_LOG_I("[STUB] vin_bind: target_vin=%s", target_vin);
    
    ret = UpdateParam(E_ParamId_VIN, (uint8_t *)target_vin, VIN_CAN1_VIN_LENGTH);
    if (ret != RET_OK) {
        TLY_LOG_E("[STUB] vin_bind: UpdateParam failed, ret=%d", ret);
        return -1;
    }
    
    return 0;
}

int tly_device_time_sync(uint32_t time_second)
{
    TLY_LOG_I("[STUB] time_sync: time_second=%u", time_second);
    return 0;
}


int tly_vehicle_state_get(tly_vehicle_state_t *state)
{
    if (!state) {
        TLY_LOG_E("VEHICLE_STATE: invalid param");
        return -1;
    }

    memset(state, 0, sizeof(tly_vehicle_state_t));

    TonlyAllCanSignal_canVal_t canSignal = {0};
    if (GetCanSignalData(&canSignal) == 0) {
        /* PKB_status: 1=手刹已拉起, 0=未拉起 */
        state->handbrake_valid = (canSignal.PKB_status == 1) ? 1 : 0;

        /* vehicle_speed: 0=车速为0, 非0=车速不为0 */
        state->zero_speed = (canSignal.vehicle_speed == 0.0f) ? 1 : 0;

        /* BMS_SOC: >=50%=满足条件, <50%=不满足 */
        state->soc_50_percent = (canSignal.BMS_SOC >= 50.0f) ? 1 : 0;

        /* key_position: 1=ON档, 0=非ON档 */
        state->key_on_position = (canSignal.key_position == 0) ? 1 : 0;

        TLY_LOG_D("VEHICLE_STATE: CAN signal - PKB=%u, speed=%.1f, SOC=%.1f, key=%u",
                  canSignal.PKB_status, canSignal.vehicle_speed,
                  canSignal.BMS_SOC, canSignal.key_position);
    } else {
        TLY_LOG_W("VEHICLE_STATE: failed to get CAN signal data, using default values");
    }

    TLY_LOG_I("VEHICLE_STATE: get state - handbrake=%u, zero_speed=%u, soc_50=%u, key_on=%u",
              state->handbrake_valid, state->zero_speed,
              state->soc_50_percent, state->key_on_position);
    printf("VEHICLE_STATE: CAN signal - PKB=%u, speed=%.1f, SOC=%.1f, key=%u",
                  canSignal.PKB_status, canSignal.vehicle_speed,
                  canSignal.BMS_SOC, canSignal.key_position);
    return 0;
}

int tly_vehicle_state_check_upgrade_ready(const tly_vehicle_state_t *state)
{
    if (!state) {
        TLY_LOG_E("VEHICLE_STATE: invalid param");
        return 0;
    }

    int ready = (state->handbrake_valid &&
                 state->zero_speed &&
                 state->soc_50_percent &&
                 state->key_on_position);

    if (ready) {
        TLY_LOG_I("VEHICLE_STATE: upgrade conditions satisfied");
    } else {
        TLY_LOG_W("VEHICLE_STATE: upgrade conditions not satisfied - "
                  "handbrake=%u, zero_speed=%u, soc_50=%u, key_on=%u",
                  state->handbrake_valid, state->zero_speed,
                  state->soc_50_percent, state->key_on_position);
    }

    return ready;
}

int tly_vehicle_state_get_unready_reason(const tly_vehicle_state_t *state,
                                          char *buf, uint32_t buf_len)
{
    if (!state || !buf || buf_len == 0) {
        return -1;
    }

    buf[0] = '\0';
    int first = 1;

    if (!state->handbrake_valid) {
        snprintf(buf + strlen(buf), buf_len - strlen(buf),
                 "%s手刹未拉起", first ? "" : ", ");
        first = 0;
    }
    if (!state->zero_speed) {
        snprintf(buf + strlen(buf), buf_len - strlen(buf),
                 "%s车速不为0", first ? "" : ", ");
        first = 0;
    }
    if (!state->soc_50_percent) {
        snprintf(buf + strlen(buf), buf_len - strlen(buf),
                 "%sSOC<50%%", first ? "" : ", ");
        first = 0;
    }
    if (!state->key_on_position) {
        snprintf(buf + strlen(buf), buf_len - strlen(buf),
                 "%s钥匙非ON档", first ? "" : ", ");
        first = 0;
    }

    return 0;
}

