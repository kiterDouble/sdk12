#include "tsp_upgrade_tbox.h"
#include "tsp_tbox_sdk.h"
#include "tsp_log.h"
#include "ota_api.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>


static volatile int g_upgrade_active = 0;

typedef struct {
    char  downloaded_file[512];
    char  extract_dir[512];
    int   upgrade_type;
    tly_upgrade_progress_cb progress_cb;
    tly_upgrade_result_cb   result_cb;
    void *user_data;
} upgrade_ctx_t;

static int file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static void normalize_dir_trailing_slash(char *dir, size_t cap)
{
    size_t n = strlen(dir);
    if (n == 0) return;
    if (dir[n - 1] != '/') {
        if (n + 1 < cap) {
            dir[n] = '/';
            dir[n + 1] = '\0';
        }
    }
}

static int ensure_dir_exists(const char *dir)
{
    struct stat st;
    if (stat(dir, &st) == 0 && S_ISDIR(st.st_mode)) return 0;

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "mkdir -p '%s'", dir);
    if (system(cmd) != 0) {
        TLY_LOG_E("UPG: mkdir %s failed", dir);
        return -1;
    }
    return 0;
}

enum pkg_format {
    PKG_ZIP    = 0,
    PKG_TARGZ  = 1,
};

static int detect_pkg_format(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;
    uint8_t hdr[4];
    size_t n = fread(hdr, 1, sizeof(hdr), fp);
    fclose(fp);
    if (n < 4) return -1;

    if (hdr[0] == 0x50 && hdr[1] == 0x4B) return PKG_ZIP;       /* "PK" */
    if (hdr[0] == 0x1F && hdr[1] == 0x8B) return PKG_TARGZ;     /* gzip */
    return -1;
}

static void flatten_fota_subdir(const char *extract_dir)
{
    char sub_path[600];
    snprintf(sub_path, sizeof(sub_path), "%sFOTA", extract_dir);

    struct stat st;
    if (stat(sub_path, &st) != 0 || !S_ISDIR(st.st_mode)) return;

    char cmd[2048];
    TLY_LOG_I("UPG: flattening FOTA/ subdir into %s", extract_dir);
    snprintf(cmd, sizeof(cmd),
             "mv %sFOTA/* %s 2>/dev/null; "
             "mv %sFOTA/.[!.]* %s 2>/dev/null; "
             "rmdir %sFOTA 2>/dev/null; true",
             extract_dir, extract_dir,
             extract_dir, extract_dir,
             extract_dir);
    system(cmd);

    if (stat(sub_path, &st) == 0) {
        TLY_LOG_W("UPG: FOTA/ still exists after flatten (not empty?)");
    }
}

static int prepare_package(upgrade_ctx_t *ctx)
{
    char cmd[2048];

    if (ensure_dir_exists(ctx->extract_dir) != 0) {
        return -1;
    }

    int fmt = detect_pkg_format(ctx->downloaded_file);
    if (fmt < 0) {
        TLY_LOG_E("UPG: unknown package format (not zip/tar.gz): %s", ctx->downloaded_file);
        return -1;
    }

    if (fmt == PKG_ZIP) {
        TLY_LOG_I("UPG: unzip '%s' -> '%s'", ctx->downloaded_file, ctx->extract_dir);
        snprintf(cmd, sizeof(cmd), "unzip -o -q '%s' -d '%s' 2>&1",
                 ctx->downloaded_file, ctx->extract_dir);
    } else {
        TLY_LOG_I("UPG: tar -zxf '%s' -> '%s'", ctx->downloaded_file, ctx->extract_dir);
        snprintf(cmd, sizeof(cmd), "tar -zxf '%s' -C '%s'",
                 ctx->downloaded_file, ctx->extract_dir);
    }
    if (system(cmd) != 0) {
        TLY_LOG_E("UPG: extract failed (%s)", fmt == PKG_ZIP ? "unzip" : "tar");
        return -1;
    }

    flatten_fota_subdir(ctx->extract_dir);

    unlink(ctx->downloaded_file);

    return 0;
}

static int detect_upgrade_flag(const char *extract_dir)
{
    char path[600];
    int  flag = 0;

    snprintf(path, sizeof(path), "%s%s", extract_dir, TBOX_MCU1_APP_NAME);
    int mcu1 = file_exists(path);
    snprintf(path, sizeof(path), "%s%s", extract_dir, TBOX_MCU2_APP_NAME);
    int mcu2 = file_exists(path);
    snprintf(path, sizeof(path), "%s%s", extract_dir, TBOX_MPU_APP_NAME);
    int mpu = file_exists(path);
    snprintf(path, sizeof(path), "%s%s", extract_dir, TBOX_MD5SUM_NAME);
    int md5 = file_exists(path);
    snprintf(path, sizeof(path), "%s%s", extract_dir, TBOX_VERSION_NAME);
    int ver = file_exists(path);

    TLY_LOG_I("UPG: package contents: %s=%d %s=%d %s=%d %s=%d %s=%d",
              TBOX_MCU1_APP_NAME, mcu1,
              TBOX_MCU2_APP_NAME, mcu2,
              TBOX_MPU_APP_NAME, mpu,
              TBOX_MD5SUM_NAME, md5,
              TBOX_VERSION_NAME, ver);

    if (mcu1 && mcu2) {
        flag |= TLY_UPG_FLAG_MCU;
    } else if (mcu1 || mcu2) {
        TLY_LOG_W("UPG: only one of %s/%s found, MCU upgrade skipped",
                  TBOX_MCU1_APP_NAME, TBOX_MCU2_APP_NAME);
    }
    if (mpu) {
        flag |= TLY_UPG_FLAG_MPU;
    }

    return flag;
}

static uint8_t compute_progress(int flag, int status, uint8_t current_pct)
{
    (void)flag;
    switch (status) {
        case UPGRADE_START:
            return 20;
        case UPGRADE_MPU_FLASHING:
            return 30;
        // case UPGRADE_MPU_SUCCESS:
        //     return 100;
        case UPGRADE_MCU_FLASH_COMPLETED:        
            return 100;
        default:            
            if (current_pct < 40) {
                return 40;
            }
            if (current_pct < 99) {
                return current_pct + 1;
            }
            return current_pct;
    }
}

static int evaluate_outcome(int flag, int status)
{
    if (status == UPGRADE_MPU_FAILED) return 0;
    if (status == UPGRADE_MCU_FAILED) return 0;

    if (status == UPGRADE_MPU_NEED_BACKUP && flag == TLY_UPG_FLAG_MPU) return 1;
    if (status == UPGRADE_MPU_NEED_BACKUP && flag == TLY_UPG_FLAG_ALL) return 1;
    if (status == UPGRADE_MCU_SUCCESS     && flag == TLY_UPG_FLAG_MCU) return 1;
    if (status == UPGRADE_MCU_SUCCESS     && flag == TLY_UPG_FLAG_ALL) return 1;
    if (status == UPGRADE_MCU_NEED_RESET  && flag == TLY_UPG_FLAG_ALL) return 1;
    if (status == UPGRADE_MCU_FLASH_COMPLETED && (flag & TLY_UPG_FLAG_MCU || flag & TLY_UPG_FLAG_ALL)) return 1;

    TLY_LOG_W("UPG: unexpected terminal status %d (flag=0x%X)", status, flag);
    return 0;
}

static void *upgrade_thread(void *arg)
{
    upgrade_ctx_t *ctx = (upgrade_ctx_t *)arg;

    normalize_dir_trailing_slash(ctx->extract_dir, sizeof(ctx->extract_dir));
    TLY_LOG_I("UPG: thread start, extract_dir=%s, type=0x%02X",
              ctx->extract_dir, ctx->upgrade_type);

    if (ctx->progress_cb) ctx->progress_cb(5, ctx->user_data);

    if (prepare_package(ctx) != 0) {
        TLY_LOG_E("UPG: package prepare failed");
        if (ctx->result_cb) ctx->result_cb(TLY_UPG_RESULT_FAIL_NO_REBOOT, ctx->user_data);
        goto out;
    }

    int flag = detect_upgrade_flag(ctx->extract_dir);
    if (flag == 0) {
        TLY_LOG_E("UPG: no valid upgrade package (mcu_1.bin/mcu_2.bin/update.zip) in %s", ctx->extract_dir);
        if (ctx->result_cb) ctx->result_cb(TLY_UPG_RESULT_FAIL_NO_REBOOT, ctx->user_data);
        goto out;
    }
    TLY_LOG_I("UPG: upgrade flag = 0x%X (MCU=%d, MPU=%d)",
              flag, !!(flag & TLY_UPG_FLAG_MCU), !!(flag & TLY_UPG_FLAG_MPU));

    int status = UPGRADE_INIT;
    tly_tbox_sdk_check_upgrade_status(ctx->extract_dir);

    if (ctx->progress_cb) ctx->progress_cb(15, ctx->user_data);

    if (tly_tbox_sdk_start_upgrade(ctx->extract_dir, flag) != 0) {
        TLY_LOG_E("UPG: start_upgrade failed");
        if (ctx->result_cb) ctx->result_cb(TLY_UPG_RESULT_FAIL_NO_REBOOT, ctx->user_data);
        goto out;
    }
    TLY_LOG_I("UPG: upgrade started");

    if (ctx->progress_cb) ctx->progress_cb(20, ctx->user_data);

    int prev = status;
    uint8_t current_pct = 20;
    while (1) {
        sleep(1);
        tly_tbox_sdk_get_upgrade_status(ctx->extract_dir, &status);
        if (status != prev) {
            TLY_LOG_I("UPG: status %d -> %d", prev, status);
            prev = status;
            if (status == UPGRADE_MCU_FLASH_COMPLETED)
                break;
        }
        if (ctx->progress_cb) {
            current_pct = compute_progress(flag, status, current_pct);
            ctx->progress_cb(current_pct, ctx->user_data);
        }

        if ((flag & TLY_UPG_FLAG_MPU) &&
            (status == UPGRADE_MPU_FAILED || status == UPGRADE_MPU_NEED_BACKUP)) {
            break;
        }
        if ((flag & TLY_UPG_FLAG_MCU) &&
            (status == UPGRADE_MCU_SUCCESS || status == UPGRADE_MCU_FAILED)) {
            break;
        }
    }

    int ok = evaluate_outcome(flag, status);
    TLY_LOG_I("UPG: terminal status=%d, outcome=%s", status, ok ? "SUCCESS" : "FAIL");

    if (ok) {
        //tly_tbox_sdk_switch_partition();
        if (ctx->progress_cb) ctx->progress_cb(100, ctx->user_data);
        if (ctx->result_cb) ctx->result_cb(TLY_UPG_RESULT_OK_NEED_REBOOT, ctx->user_data);
    } else {
        tly_tbox_sdk_restore_partition();
        if (ctx->progress_cb) ctx->progress_cb(0, ctx->user_data);
        if (ctx->result_cb) ctx->result_cb(TLY_UPG_RESULT_FAIL_NO_REBOOT, ctx->user_data);
    }

    if (flag & TLY_UPG_FLAG_MCU) {
        TLY_LOG_I("UPG: MCU post-processing (FotaMode ON + sync status)");
        sleep(1);
        tly_tbox_sdk_fota_mode_on();
        sleep(1);
        tly_tbox_sdk_sync_upgrade_processing();
    }

out:
    g_upgrade_active = 0;
    free(ctx);
    return NULL;
}

void tly_upgrade_tbox_start(const char *downloaded_file,
                            const char *extract_dir,
                            tly_upgrade_type_t type,
                            tly_upgrade_progress_cb progress_cb,
                            tly_upgrade_result_cb result_cb,
                            void *user_data)
{
    if (!downloaded_file || !extract_dir) {
        TLY_LOG_E("UPG: start param null");
        return;
    }

    if (g_upgrade_active) {
        TLY_LOG_W("UPG: already in progress, ignore new request");
        if (result_cb) result_cb(TLY_UPG_RESULT_FAIL_NO_REBOOT, user_data);
        return;
    }

    upgrade_ctx_t *ctx = (upgrade_ctx_t *)calloc(1, sizeof(*ctx));
    if (!ctx) {
        TLY_LOG_E("UPG: ctx alloc failed");
        if (result_cb) result_cb(TLY_UPG_RESULT_FAIL_NO_REBOOT, user_data);
        return;
    }
    snprintf(ctx->downloaded_file, sizeof(ctx->downloaded_file), "%s", downloaded_file);
    snprintf(ctx->extract_dir, sizeof(ctx->extract_dir), "%s", extract_dir);
    ctx->upgrade_type = (int)type;
    ctx->progress_cb  = progress_cb;
    ctx->result_cb    = result_cb;
    ctx->user_data    = user_data;

    pthread_t tid;
    int rc = pthread_create(&tid, NULL, upgrade_thread, ctx);
    if (rc != 0) {
        TLY_LOG_E("UPG: pthread_create failed: %d", rc);
        free(ctx);
        if (result_cb) result_cb(TLY_UPG_RESULT_FAIL_NO_REBOOT, user_data);
        return;
    }
    pthread_detach(tid);
    g_upgrade_active = 1;
    TLY_LOG_I("UPG: upgrade thread launched");
}

int tly_upgrade_tbox_reboot(void)
{
    TLY_LOG_I("UPG: triggering system reboot");
    return tly_tbox_sdk_system_reboot();
}

int tly_upgrade_tbox_is_active(void)
{
    return g_upgrade_active;
}
