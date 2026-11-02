#include "tsp_ota.h"
#include "tsp_log.h"
#include "tsp_tcp.h"
#include "tsp_upgrade_tbox.h"
#include "tsp_common.h"
#include "tsp_device.h"

/* OTA Master SDK */
#include "ota_master_api.h"
#include "ota_master_data.h"
#include "ota_master_pm_helper.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libgen.h>
#include <pthread.h>
#include <event2/event.h>

#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#endif

#define OTA_PROGRESS_INTERVAL_SEC  2

/* 判断是否为 TBox 自升级命令 */
static int is_tbox_self_upgrade(uint8_t ota_cmd)
{
    return (ota_cmd == TLY_OTA_TBOX || ota_cmd == TLY_OTA_TBOX_EXT);
}

/* 根据 ota_cmd获取ECU ID */
static uint16_t get_ecu_id_from_ota_cmd(uint8_t ota_cmd)
{
    switch (ota_cmd) {
        case TLY_OTA_VCU_FIRST:
        case TLY_OTA_VCU:
        case TLY_OTA_VCU_NORMAL:
        case TLY_OTA_VCU_YIKONG:
        case TLY_OTA_VCU_YK_NORMAL:
        case TLY_OTA_VCU_BOLEIKE:
        case TLY_OTA_VCU_BLK_NORMAL:
            return ECU_ID_VCU;
        case TLY_OTA_TCU:
        case TLY_OTA_TCU_NORMAL:
            return ECU_ID_TCU;
        case TLY_OTA_TMS:
        case TLY_OTA_TMS_NORMAL:
            return ECU_ID_TMS;
        case TLY_OTA_MCU1_TM1:
            return ECU_ID_MCU1_TM1;
        case TLY_OTA_MCU1_TM2:
            return ECU_ID_MCU1_TM2;
        case TLY_OTA_MCU1_PDU:
            return ECU_ID_MCU1_PDU;
        case TLY_OTA_MCU1_EPS:
            return ECU_ID_MCU1_EPS;
        case TLY_OTA_MCU1_ACM:
            return ECU_ID_MCU1_ACM;
        case TLY_OTA_MCU1_DCDC:
            return ECU_ID_MCU1_DCDC;
        case TLY_OTA_MCU2_TM1:
        case TLY_OTA_MCU2_TM2:
        case TLY_OTA_MCU2_PDU:
        case TLY_OTA_MCU2_EPS:
        case TLY_OTA_MCU2_ACM:
        case TLY_OTA_MCU2_DCDC:
            return ECU_ID_MCU2;
        default:
            return 0;
    }
}

static EcuType_e get_ecu_type_from_id(uint16_t ecu_id, uint8_t ota_cmd)
{
    switch (ecu_id) {
        case 0: 
            if (is_tbox_self_upgrade(ota_cmd)) {
                return ECU_TYPE_TBOX;
            }
            return ECU_TYPE_OTHER;
        case ECU_ID_VCU:
            return ECU_TYPE_VCU;
        case ECU_ID_TMS:
            return ECU_TYPE_TMS;
        case ECU_ID_TCU:
            return ECU_TYPE_OTHER;
        case ECU_ID_MCU1_TM1:
        case ECU_ID_MCU1_TM2:
        case ECU_ID_MCU1_PDU:
        case ECU_ID_MCU1_EPS:
        case ECU_ID_MCU1_ACM:
        case ECU_ID_MCU1_DCDC:
            return ECU_TYPE_MCU;
        case ECU_ID_MCU2_TM1:
        case ECU_ID_MCU2_TM2:
        case ECU_ID_MCU2_PDU:
        case ECU_ID_MCU2_EPS:
        case ECU_ID_MCU2_ACM:
        case ECU_ID_MCU2_DCDC:
            return ECU_TYPE_MCU;
        default:
            return ECU_TYPE_OTHER;
    }
}

static EcuType_e get_ecu_type_from_ota_cmd(uint8_t ota_cmd)
{
    uint16_t ecu_id = get_ecu_id_from_ota_cmd(ota_cmd);
    return get_ecu_type_from_id(ecu_id, ota_cmd);
}

static char *url_filename(const char *url, char *buf, uint32_t buf_len)
{
    const char *p = strrchr(url, '/');
    if (!p || !p[1]) {
        snprintf(buf, buf_len, "firmware.bin");
    } else {
        snprintf(buf, buf_len, "%s", p + 1);
    }
    return buf;
}

static int ensure_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return 0;
        return -1;
    }
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "mkdir -p %s", path);
    return system(tmp);
}

/* ======================== 下载/升级状态 ======================== */

typedef enum {
    OTA_PHASE_IDLE = 0,
    OTA_PHASE_DOWNLOADING,
    OTA_PHASE_UPGRADING
} ota_phase_t;

typedef struct {
    tly_app_t       *app;
    tly_ota_info_t   ota;
    char             save_path[512];
    char             progress_path[512];
    struct event    *ev_timer;
    long             total_size;
    int              active;
    uint8_t          last_pct;
    pid_t            curl_pid;
    ota_phase_t      phase;
    uint16_t         ecu_id;         /* ECU ID */
    volatile uint8_t upgrade_pct;
    volatile int     upgrade_done;
    volatile int     upgrade_result;
    pthread_t        brush_thread;   /* ECU刷写线程 */
} ota_dl_ctx_t;

static ota_dl_ctx_t g_dl;
static void ota_download_finish(int success);
static void ota_start_progress_timer(void);
static void ota_upgrade_finish(void);
static void on_ecu_brush_progress(uint8_t percent, void *userdata);

/* ECU刷写线程参数结构 */
typedef struct {
    uint16_t ecu_id;
    char save_path[512];
} brush_thread_args_t;

static brush_thread_args_t g_brush_args;

/* ECU刷写线程函数 */
static void *ecu_brush_thread_func(void *arg)
{
    brush_thread_args_t *args = (brush_thread_args_t *)arg;
    TLY_LOG_I("ECU brush thread started: ecuId=0x%04X", args->ecu_id);
    
    int ret = OtaMasterExecEcuBrush(args->ecu_id, NULL, args->save_path, NULL,
                                    on_ecu_brush_progress, NULL);
    if (ret == OTA_RET_OK) {
        TLY_LOG_I("ECU brush thread success: ecuId=0x%04X", args->ecu_id);
        g_dl.upgrade_result = (int)TLY_UPG_RESULT_OK_NO_REBOOT;
    } else {
        TLY_LOG_E("ECU brush thread failed: ecuId=0x%04X, ret=%d", args->ecu_id, ret);
        g_dl.upgrade_result = (int)TLY_UPG_RESULT_FAIL_NO_REBOOT;
        /* ECU刷写失败后立即清除OTA任务，避免状态机长时间等待 */
        OtaMasterClearOtaTask();
    }
    g_dl.upgrade_done = 1;
    g_dl.upgrade_pct = 100;  /* 标记完成 */
    TLY_LOG_I("ECU brush thread finished, deleting file: %s", args->save_path);
    unlink(args->save_path);
    return NULL;
}

static void on_upgrade_progress(uint8_t pct, void *user_data)
{
    (void)user_data;
    g_dl.upgrade_pct = pct;
}

/* ECU 刷写进度回调函数 */
static void on_ecu_brush_progress(uint8_t percent, void *userdata)
{
    (void)userdata;
    if (g_dl.active && g_dl.phase == OTA_PHASE_UPGRADING) {
        g_dl.upgrade_pct = percent;
        if (percent != g_dl.last_pct) {
            TLY_LOG_I("ECU brush progress: %u%%", percent);
            g_dl.last_pct = percent;
        }
                     
        if (percent != 100) {
            tly_ota_send_upgrade_status(g_dl.app, g_dl.ota.ota_cmd,
                4, 0, 10000, (uint16_t)percent * 100);
        }
    }
}

static void on_upgrade_result(tly_upgrade_result_t result, void *user_data)
{
    (void)user_data;
    g_dl.upgrade_result = (int)result;
    g_dl.upgrade_done = 1;
}

static void ota_progress_cb(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd;
    (void)events;
    (void)ctx;

    if (!g_dl.active) return;

    if (g_dl.phase == OTA_PHASE_UPGRADING) {
        uint8_t pct = g_dl.upgrade_pct;
        if (pct != g_dl.last_pct) {
            TLY_LOG_I("OTA upgrading: %u%%", pct);
            g_dl.last_pct = pct;
        }
        if (pct != 100) {
            tly_ota_send_upgrade_status(g_dl.app, g_dl.ota.ota_cmd,
                4, 0, 10000, (uint16_t)pct * 100);
        } else {
            tly_ota_send_upgrade_status(g_dl.app, g_dl.ota.ota_cmd,
                4, 0, 10000, 10000);
        }

        if (g_dl.upgrade_done) {
            ota_upgrade_finish();
        }
        return;
    }

    int status;
    pid_t ret = waitpid(g_dl.curl_pid, &status, WNOHANG);

    if (ret == 0) {
        uint8_t pct = 0;
        if (g_dl.total_size > 0) {
            struct stat st;
            if (stat(g_dl.save_path, &st) == 0 && st.st_size > 0) {
                pct = (uint8_t)((uint64_t)st.st_size * 100 / (uint64_t)g_dl.total_size);
                if (pct > 100) pct = 100;
            }
        }
        if (pct != g_dl.last_pct) {
            TLY_LOG_I("OTA downloading: %u%%", pct);
            g_dl.last_pct = pct;
        }
        tly_ota_send_upgrade_status(g_dl.app, g_dl.ota.ota_cmd, 2, 0, (uint16_t)pct * 100, 0);
        return;
    }

    if (ret < 0) {
        TLY_LOG_E("OTA download: waitpid error");
        ota_download_finish(0);
        return;
    }   
    if (WIFEXITED(status)) {
        TLY_LOG_I("OTA download: curl exited with status %d", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        TLY_LOG_I("OTA download: curl killed by signal %d", WTERMSIG(status));
    }
    
    /* 检查下载文件大小 */
    struct stat final_st;
    if (stat(g_dl.save_path, &final_st) == 0) {
        TLY_LOG_I("OTA download: file size after download = %ld bytes", (long)final_st.st_size);
    } else {
        TLY_LOG_E("OTA download: file not found after download: %s", g_dl.save_path);
    }

    ota_download_finish(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static void ota_upgrade_finish(void)
{
    if (g_dl.ev_timer) {
        event_del(g_dl.ev_timer);
        event_free(g_dl.ev_timer);
        g_dl.ev_timer = NULL;
    }

    /* 等待ECU刷写线程完成 */
    if (g_dl.brush_thread != 0) {
        pthread_join(g_dl.brush_thread, NULL);
        g_dl.brush_thread = 0;
    }

    int ok = (g_dl.upgrade_result == (int)TLY_UPG_RESULT_OK_NEED_REBOOT ||
              g_dl.upgrade_result == (int)TLY_UPG_RESULT_OK_NO_REBOOT);


    EcuType_e ecuType = get_ecu_type_from_id(get_ecu_id_from_ota_cmd(g_dl.ota.ota_cmd), g_dl.ota.ota_cmd);
    if (OtaMasterPmCleanSleepEventByType(ecuType) != 0) {
        TLY_LOG_E("OTA upgrade end: failed to clean sleep event for ECU type %d", ecuType);
    } else {
        TLY_LOG_I("OTA upgrade end: clean sleep event for ECU type %d success", ecuType);
    }

    if (ok) {
        TLY_LOG_I("OTA upgrade complete");
        tly_ota_send_upgrade_status(g_dl.app, g_dl.ota.ota_cmd, 5, 0, 10000, 10000);
        g_dl.active = 0;
        if (g_dl.upgrade_result == (int)TLY_UPG_RESULT_OK_NEED_REBOOT) {
            sleep(2);
            tly_upgrade_tbox_reboot();
        }
    } else {
        TLY_LOG_E("OTA upgrade failed");
        tly_ota_send_upgrade_status(g_dl.app, g_dl.ota.ota_cmd, 4, 1, 10000, 0);
        g_dl.active = 0;
    }
}

static void ota_download_finish(int success)
{
    if (!success) {
        if (g_dl.ev_timer) {
            event_del(g_dl.ev_timer);
            event_free(g_dl.ev_timer);
            g_dl.ev_timer = NULL;
        }
        g_dl.active = 0;
        TLY_LOG_E("OTA download failed: url=%s", g_dl.ota.url);

        EcuType_e ecuType = get_ecu_type_from_id(get_ecu_id_from_ota_cmd(g_dl.ota.ota_cmd), g_dl.ota.ota_cmd);
        if (OtaMasterPmCleanSleepEventByType(ecuType) != 0) {
            TLY_LOG_E("OTA download failed: failed to clean sleep event for ECU type %d", ecuType);
        } else {
            TLY_LOG_I("OTA download failed: clean sleep event for ECU type %d success", ecuType);
        }
        unlink(g_dl.save_path);
        tly_ota_send_upgrade_status(g_dl.app, g_dl.ota.ota_cmd, 2, 1, 0, 0);
        return;
    }

    TLY_LOG_I("OTA download complete: %s", g_dl.save_path);
    tly_ota_send_upgrade_status(g_dl.app, g_dl.ota.ota_cmd, 2, 0, 10000, 0);

    g_dl.phase = OTA_PHASE_UPGRADING;
    g_dl.upgrade_done = 0;
    g_dl.upgrade_pct = 0;
    g_dl.last_pct = 0;

    /* 根据升级类型选择不同的升级方式 */
    if (g_dl.ecu_id != 0) {        
        TLY_LOG_I("OTA starting ECU brush thread: ecuId=0x%04X, file=%s",
                  g_dl.ecu_id, g_dl.save_path);
        
        /* 使用全局变量传递参数 */
        g_brush_args.ecu_id = g_dl.ecu_id;
        snprintf(g_brush_args.save_path, sizeof(g_brush_args.save_path), "%s", g_dl.save_path);
        
        /* 启动刷写线程 */
        if (pthread_create(&g_dl.brush_thread, NULL, ecu_brush_thread_func, &g_brush_args) != 0) {
            TLY_LOG_E("Failed to create ECU brush thread");
            /* 刷写线程创建失败，清除休眠事件 */
            EcuType_e ecuType = get_ecu_type_from_ota_cmd(g_dl.ota.ota_cmd);
            if (OtaMasterPmCleanSleepEventByType(ecuType) != 0) {
                TLY_LOG_E("ECU brush thread failed: failed to clean sleep event for ECU type %d", ecuType);
            } else {
                TLY_LOG_I("ECU brush thread failed: clean sleep event for ECU type %d success", ecuType);
            }
            g_dl.upgrade_result = (int)TLY_UPG_RESULT_FAIL_NO_REBOOT;
            tly_ota_send_upgrade_status(g_dl.app, g_dl.ota.ota_cmd, 4, 1, 10000, 0);
            OtaMasterClearOtaTask();
            g_dl.upgrade_done = 1;
            g_dl.active = 0;
            unlink(g_dl.save_path);
        }        
    } else {
        /* TBox 自升级 */
        char extract_dir[512];
        snprintf(extract_dir, sizeof(extract_dir), "%s/selfUpgrade",
                 g_dl.app->config.ota_save_dir);

        TLY_LOG_I("OTA starting TBOX self-upgrade: %s -> %s",
                  g_dl.save_path, extract_dir);

        tly_upgrade_tbox_start(g_dl.save_path,
                               extract_dir,
                               TLY_UPGRADE_NORMAL,
                               on_upgrade_progress,
                               on_upgrade_result,
                               g_dl.app);
    }
}

static void ota_start_progress_timer(void)
{
    struct timeval tv = {OTA_PROGRESS_INTERVAL_SEC, 0};
    g_dl.ev_timer = event_new(g_dl.app->evbase, -1,
                              EV_TIMEOUT | EV_PERSIST,
                              ota_progress_cb, NULL);
    if (g_dl.ev_timer) {
        evtimer_add(g_dl.ev_timer, &tv);
    }
}

/* ======================== 命令方式: fork+exec curl ======================== */

static long curl_cmd_get_size(const tly_ota_info_t *ota)
{
    char cmd[1024];
    char userpwd[256] = "";

    if (ota->username[0] || ota->password[0]) {
        snprintf(userpwd, sizeof(userpwd), "-u '%s:%s'", ota->username, ota->password);
    }

    snprintf(cmd, sizeof(cmd),
             "curl -sI %s --connect-timeout 10 --max-time 15 '%s' 2>/dev/null"
             " | grep -i content-length | awk '{print $2}' | tr -d '\\r\\n'",
             userpwd, ota->url);

    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;

    char val[32];
    long size = -1;
    if (fgets(val, sizeof(val), fp)) {
        size = atol(val);
    }
    pclose(fp);
    return size > 0 ? size : -1;
}

static int ota_download_curl_cmd(tly_app_t *app, const tly_ota_info_t *ota)
{
    (void)app;
    long remote_size = curl_cmd_get_size(ota);
    TLY_LOG_I("OTA remote file size: %ld bytes", remote_size);

    g_dl.total_size = remote_size;

    pid_t pid = fork();
    if (pid < 0) {
        TLY_LOG_E("OTA download: fork failed");
        return -1;
    }

    if (pid == 0) {
        char userpwd[256];
        if (ota->username[0] || ota->password[0]) {
            snprintf(userpwd, sizeof(userpwd), "%s:%s", ota->username, ota->password);
        } else {
            userpwd[0] = '\0';
        }

        const char *url = ota->url;
        char url_buf[512];
        if (ota->port != 0 && ota->port != 21 && strncmp(ota->url, "ftp://", 6) == 0) {
            const char *path = ota->url + 6;
            snprintf(url_buf, sizeof(url_buf), "ftp://%u.%u.%u.%u.%u.%u:%u/%s",
                     ota->ip[0], ota->ip[1], ota->ip[2],
                     ota->ip[3], ota->ip[4], ota->ip[5],
                     ota->port, path);
            url = url_buf;
        }
		TLY_LOG_I("curl exec: url=%s, output=%s, userpwd=%s", url, g_dl.save_path, userpwd);
        execl("/usr/bin/curl", "curl",
              "--silent", "--show-error", "--fail",
              "-o", g_dl.save_path,
              "--connect-timeout", "30",
              "--max-time", "600",
              userpwd[0] ? "-u" : NULL,
              userpwd[0] ? userpwd : NULL,
              url,
              (char *)NULL);
        TLY_LOG_E("curl execl failed");
        _exit(127);
    }

    g_dl.curl_pid = pid;
    return 0;
}

/* ======================== 代码方式: libcurl API ======================== */

#ifdef HAVE_LIBCURL

static size_t libcurl_write_cb(void *ptr, size_t size, size_t nmemb, void *ctx)
{
    FILE *fp = (FILE *)ctx;
    return fwrite(ptr, size, nmemb, fp);
}

static int ota_download_libcurl(tly_app_t *app, const tly_ota_info_t *ota)
{
    (void)app;

    pid_t pid = fork();
    if (pid < 0) {
        TLY_LOG_E("OTA libcurl: fork failed");
        return -1;
    }

    if (pid == 0) {
        FILE *fp = fopen(g_dl.save_path, "wb");
        if (!fp) _exit(2);

        CURL *curl = curl_easy_init();
        if (!curl) { fclose(fp); _exit(3); }

        curl_easy_setopt(curl, CURLOPT_URL, ota->url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, libcurl_write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_MAXFILESIZE_LARGE, (curl_off_t)0);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

        if (ota->username[0] || ota->password[0]) {
            char userpwd[256];
            snprintf(userpwd, sizeof(userpwd), "%s:%s", ota->username, ota->password);
            curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
        }

        if (ota->port != 0 && ota->port != 21) {
            curl_easy_setopt(curl, CURLOPT_PORT, (long)ota->port);
        }

        CURLcode res = curl_easy_perform(curl);

#if LIBCURL_VERSION_NUM >= 0x073700
        curl_off_t cl = 0;
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &cl);
        g_dl.total_size = (long)cl;
#else
        double total = 0;
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &total);
        g_dl.total_size = (long)total;
#endif

        curl_easy_cleanup(curl);
        fclose(fp);

        _exit(res == CURLE_OK ? 0 : 1);
    }

    g_dl.curl_pid = pid;
    return 0;
}

#endif /* HAVE_LIBCURL */

/* ======================== 公共接口 ======================== */

void tly_ota_dispatch(tly_app_t *app, const tly_ota_info_t *ota)
{
    if (!app || !ota) return;

    if (g_dl.active) {
        TLY_LOG_W("OTA dispatch: download already in progress");
        return;
    }

    if (!ota->url[0]) {
        TLY_LOG_E("OTA dispatch: URL is empty");
        return;
    }

    #if  0   // 车辆状态检查功能暂时关闭，后续can解析完善后放开
    /* TBOX自升级前检查车辆状态条件 */
    if (is_tbox_self_upgrade(ota->ota_cmd)) {
        tly_vehicle_state_t vehicle_state;
        if (tly_vehicle_state_get(&vehicle_state) == 0) {
            if (!tly_vehicle_state_check_upgrade_ready(&vehicle_state)) {
                char reason_buf[256] = {0};
                tly_vehicle_state_get_unready_reason(&vehicle_state, reason_buf, sizeof(reason_buf));
                TLY_LOG_E("OTA dispatch: vehicle state not ready for TBOX self-upgrade - %s",
                          reason_buf[0] ? reason_buf : "unknown");
                /* 上报升级失败，使用 TLY_CMD_UPGRADE_FAIL (0x83)，错误码1 */
                tly_ota_send_upgrade_fail(app, 1, 1);
                return;
            }
            TLY_LOG_I("OTA dispatch: vehicle state ready for TBOX self-upgrade");
        } else {
            TLY_LOG_E("OTA dispatch: failed to get vehicle state");
            /* 获取车辆状态失败，上报升级失败，使用 TLY_CMD_UPGRADE_FAIL (0x83)，错误码1 */
            tly_ota_send_upgrade_fail(app, 1, 1);
            return;
        }
    }
    #endif
    TLY_LOG_I("OTA dispatch: cmd=0x%02X, target=%s, url=%s, mode=%s",
              ota->ota_cmd, ota->mid, ota->url,
              app->config.ota_use_libcurl ? "libcurl" : "curl_cmd");

    EcuType_e ecuType = get_ecu_type_from_ota_cmd(ota->ota_cmd);
    if (OtaMasterPmSetSleepEvent(ecuType) != 0) {
        TLY_LOG_E("OTA dispatch: failed to set sleep event for ECU type %d", ecuType);
    } else {
        TLY_LOG_I("OTA dispatch: set sleep event for ECU type %d success", ecuType);
    }
   
    if (ensure_dir(app->config.ota_save_dir) != 0) {
        TLY_LOG_E("OTA dispatch: cannot create dir %s", app->config.ota_save_dir);
        return;
    }

    char fname[128];
    url_filename(ota->url, fname, sizeof(fname));

    memset(&g_dl, 0, sizeof(g_dl));
    g_dl.app = app;
    /* 根据 ota_cmd设置ECU ID：自升级为 0, ECU 升级为对应 ECU ID */
    g_dl.ecu_id = is_tbox_self_upgrade(ota->ota_cmd) ? 0 : get_ecu_id_from_ota_cmd(ota->ota_cmd);
    memcpy(&g_dl.ota, ota, sizeof(g_dl.ota));
    snprintf(g_dl.save_path, sizeof(g_dl.save_path), "%s/%s",
             app->config.ota_save_dir, fname);
    unlink(g_dl.save_path);

    int rc;

#ifdef HAVE_LIBCURL
    if (app->config.ota_use_libcurl) {
        CURL *curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, ota->url);
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
            if (ota->username[0] || ota->password[0]) {
                char userpwd[256];
                snprintf(userpwd, sizeof(userpwd), "%s:%s", ota->username, ota->password);
                curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
            }
            if (ota->port != 0 && ota->port != 21) {
                curl_easy_setopt(curl, CURLOPT_PORT, (long)ota->port);
            }
            CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
#if LIBCURL_VERSION_NUM >= 0x073700
                curl_off_t cl = 0;
                curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &cl);
#else
                double cl = 0;
                curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &cl);
#endif
                g_dl.total_size = (long)cl;
            }
            curl_easy_cleanup(curl);
        }
        TLY_LOG_I("OTA remote file size: %ld bytes", g_dl.total_size);
        rc = ota_download_libcurl(app, ota);
    } else
#endif
    {
        rc = ota_download_curl_cmd(app, ota);
    }

    if (rc != 0) return;

    g_dl.active = 1;
    g_dl.last_pct = 0;
    g_dl.phase = OTA_PHASE_DOWNLOADING;

    TLY_LOG_I("OTA download started: %s -> %s (user=%s, port=%u)",
              ota->url, g_dl.save_path, ota->username, ota->port);

    tly_ota_send_upgrade_status(app, ota->ota_cmd, 1, 0, 0, 0);
    ota_start_progress_timer();
}

int tly_ota_send_upgrade_status(tly_app_t *app,
                                uint8_t ota_cmd,
                                uint8_t process_status,
                                uint8_t error_status,
                                uint16_t download_percent,
                                uint16_t upgrade_percent)
{
    if (!app || !tly_tcp_is_connected(app->tcp)) return TLY_ERR_CLOSED;

    // 将ota_cmd转换为ecu_type
    uint8_t ecu_type = tly_ota_cmd_to_ecu_type(ota_cmd);

    tly_upgrade_status_t status;
    memset(&status, 0, sizeof(status));
    tly_time_now(status.time);
    status.ecu_type         = ecu_type;
    status.process_status   = process_status;
    status.error_status     = error_status;
    status.download_percent = download_percent;
    status.upgrade_percent  = upgrade_percent;

    uint8_t data_buf[16];
    uint32_t data_len = 0;
    int rc = tly_encode_upgrade_status(&status, data_buf, &data_len);
    if (rc != 0) {
        TLY_LOG_E("Failed to encode upgrade status: %d", rc);
        return rc;
    }

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_UPGRADE_STATUS;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_len;
    pkt.data         = data_buf;

    rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_I("Upgrade status sent: ecu=0x%02X, process=%u, error=%u, download=%u%%, upgrade=%u%%",
                  ecu_type, process_status, error_status,
                  download_percent, upgrade_percent);
    } else {
        TLY_LOG_E("Upgrade status send failed: %d", rc);
    }

    uint8_t raw_buf[TLY_PKT_MAX_SIZE];
    uint32_t raw_len = 0;
    if (tly_packet_encode(&pkt, raw_buf, &raw_len) == 0) {
        TLY_LOG_Dump("UPGRADE STATUS PKT RAW:", raw_buf, raw_len);
    }

    return rc;
}

void tly_ota_handle_upgrade_status(const uint8_t *data, uint32_t data_len)
{
    if (data_len > 0 && data) {
        tly_upgrade_status_t status;
        if (tly_decode_upgrade_status(data, data_len, &status) == 0) {
            tly_upgrade_status_dump(&status);
        } else {
            TLY_LOG_W("Failed to decode upgrade status");
        }
    } else {
        TLY_LOG_I("Upgrade status: %u bytes", data_len);
    }
}
