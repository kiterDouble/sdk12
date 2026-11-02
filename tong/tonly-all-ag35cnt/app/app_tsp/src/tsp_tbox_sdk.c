#include "tsp_tbox_sdk.h"
#include "tsp_log.h"

#include "err_code.h"
#include "ota_api.h"

int tly_tbox_sdk_start_upgrade(const char *extract_path, int upgrade_flag)
{
    TLY_LOG_I("[STUB] start_upgrade: path=%s flag=0x%X", extract_path, upgrade_flag);

    if (OtaStartUpgrade(extract_path, upgrade_flag) != RET_OK) {
        TLY_LOG_E("OtaStartUpgrade fail");
        return -1;
    } 

    return 0;
}


int tly_tbox_sdk_check_upgrade_status(const char *extract_path)
{
    int32_t ret;
    UpgradeStatus_E status = UPGRADE_INIT;

    // (void)extract_path;
    // *out_status = TLY_UPG_MCU_SUCCESS;
    
    OtaGetUpgradeStatus(extract_path, &status);
    
    TLY_LOG_E("OtaGetUpgradeStatus fail, status: %d", status);
    if (status != UPGRADE_MPU_SUCCESS && status != UPGRADE_MPU_FAILED && status != UPGRADE_INIT) {
        TLY_LOG_I("============= OtaRestoreAOrBPartition ===============");
        OtaRestoreAOrBPartition();
    }
    
    return 0;
}

int tly_tbox_sdk_get_upgrade_status(const char *extract_path, int *out_status)
{
    int32_t ret;
    UpgradeStatus_E status = UPGRADE_INIT;

    // (void)extract_path;
    // *out_status = TLY_UPG_MCU_SUCCESS;
    
    ret = OtaGetUpgradeStatus(extract_path, &status);
    
    TLY_LOG_D("[STUB] get_upgrade_status: %d, ret: %d", status, ret);
    *out_status = status;

    return ret;
}

int tly_tbox_sdk_switch_partition(void)
{
    TLY_LOG_I("[STUB] switch_partition");

    OtaSwitchAOrBPartition();

    return 0;
}

int tly_tbox_sdk_restore_partition(void)
{
    TLY_LOG_I("[STUB] restore_partition");

    OtaRestoreAOrBPartition();

    return 0;
}

int tly_tbox_sdk_fota_mode_on(void)
{
    TLY_LOG_I("[STUB] fota_mode_on");



    return 0;
}

int tly_tbox_sdk_sync_upgrade_processing(void)
{
    TLY_LOG_I("[STUB] sync_upgrade_processing");



    return 0;
}

int tly_tbox_sdk_system_reboot(void)
{
    TLY_LOG_I("[STUB] system_reboot");

    SrTboxSystemReboot("tlyApp", OTA_MASTER_REBOOT);

    return 0;
}
