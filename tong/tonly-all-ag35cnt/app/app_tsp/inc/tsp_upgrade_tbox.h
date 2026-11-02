#ifndef __TSP_UPGRADE_TBOX_H__
#define __TSP_UPGRADE_TBOX_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TBOX_MCU1_APP_NAME  "mcu_1.bin"
#define TBOX_MCU2_APP_NAME  "mcu_2.bin"
#define TBOX_MPU_APP_NAME   "update.zip"
#define TBOX_MD5SUM_NAME    "md5sum.txt"
#define TBOX_VERSION_NAME   "version.txt"

typedef enum {
    TLY_UPGRADE_NORMAL  = 0x01,
    TLY_UPGRADE_ROLLBACK = 0x02,
} tly_upgrade_type_t;

typedef enum {
    TLY_UPG_RESULT_OK_NEED_REBOOT = 0,
    TLY_UPG_RESULT_OK_NO_REBOOT   = 1,
    TLY_UPG_RESULT_FAIL_NO_REBOOT = 2,
} tly_upgrade_result_t;

typedef void (*tly_upgrade_result_cb)(tly_upgrade_result_t result, void *user_data);

typedef void (*tly_upgrade_progress_cb)(uint8_t upgrade_percent, void *user_data);

void tly_upgrade_tbox_start(const char *downloaded_file,
                            const char *extract_dir,
                            tly_upgrade_type_t type,
                            tly_upgrade_progress_cb progress_cb,
                            tly_upgrade_result_cb result_cb,
                            void *user_data);

int  tly_upgrade_tbox_reboot(void);

int  tly_upgrade_tbox_is_active(void);

#ifdef __cplusplus
}
#endif

#endif
