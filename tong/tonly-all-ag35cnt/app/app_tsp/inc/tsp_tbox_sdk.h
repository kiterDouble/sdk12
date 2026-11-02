#ifndef __TSP_TBOX_SDK_H__
#define __TSP_TBOX_SDK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TLY_UPG_FLAG_MCU  0x01
#define TLY_UPG_FLAG_MPU  0x02
#define TLY_UPG_FLAG_ALL  (TLY_UPG_FLAG_MCU | TLY_UPG_FLAG_MPU)

// enum tly_upgrade_status {
//     TLY_UPG_INIT            = 0,
//     TLY_UPG_MPU_SUCCESS     = 1,
//     TLY_UPG_MPU_FAILED      = 2,
//     TLY_UPG_MPU_NEED_BACKUP = 3,
//     TLY_UPG_MCU_SUCCESS     = 4,
//     TLY_UPG_MCU_FAILED      = 5,
// };

int tly_tbox_sdk_start_upgrade(const char *extract_path, int upgrade_flag);

int tly_tbox_sdk_check_upgrade_status(const char *extract_path);

int tly_tbox_sdk_get_upgrade_status(const char *extract_path, int *out_status);

int tly_tbox_sdk_switch_partition(void);

int tly_tbox_sdk_restore_partition(void);

int tly_tbox_sdk_fota_mode_on(void);

int tly_tbox_sdk_sync_upgrade_processing(void);

int tly_tbox_sdk_system_reboot(void);

#ifdef __cplusplus
}
#endif

#endif
