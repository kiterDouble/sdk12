#include <unistd.h>
#include <glib.h>

#include "err_code.h"
#include "sr_log.h"
#include "pm_modem_reboot.h"

/* ini 配置分组名 */
#define CFG_GROUP_UP_TIME_AUTO_REBOOT   "UpTimeAutoReboot"
#define CFG_GROUP_WORK_TIME_AUTO_REBOOT "WorkTimeAutoReboot"

/* ini 配置键名 */
#define CFG_KEY_DURATION_MIN            "durationMin"
#define CFG_KEY_WAKE_UP_REASON          "wakeUpReason"
#define CFG_KEY_RTC_WAKE_UP_TRIGGER     "rtcWakeUpTrigger"

static PmUpTimeAutoRebootCfg_t g_upTimeAutoRebootCfg = {
    DEFAULT_UP_TIME_DURATION_MIN,
    PM_AUTO_REBOOT_WAKE_REASON_NO_LIMIT,
    DEFAULT_RTC_WAKE_UP_TRIGGER
};

static PmWorkTimeAutoRebootCfg_t g_workTimeAutoRebootCfg = {
    DEFAULT_WORK_TIME_DURATION_MIN,
    PM_AUTO_REBOOT_WAKE_REASON_NO_LIMIT
};

/**
 * @brief 读取 ini 无符号整数配置项, 缺失或非法时使用缺省值
 *
 * @param keyFile  GKeyFile 对象
 * @param group    分组名
 * @param key      键名
 * @param defVal   缺省值
 * @return uint32_t 配置值
 */
static uint32_t PmAutoRebootCfgGetUint(GKeyFile *keyFile, const char *group, const char *key, uint32_t defVal)
{
    GError *error = NULL;
    gint value = g_key_file_get_integer(keyFile, group, key, &error);

    if (error != NULL)
    {
        LOG_Warn("cfg [%s] %s invalid: %s, use default %u", group, key, error ? error->message : "unknown", defVal);
        g_clear_error(&error);
        return defVal;
    }
    if (value < 0)
    {
        LOG_Warn("cfg [%s] %s negative(%d), use default %u", group, key, (int)value, defVal);
        return defVal;
    }

    return (uint32_t)value;
}

/**
 * @brief 读取 ini 布尔类(0/1)配置项, 非0按1处理
 *
 * @param keyFile  GKeyFile 对象
 * @param group    分组名
 * @param key      键名
 * @param defVal   缺省值
 * @return uint8_t 0或1
 */
static uint8_t PmAutoRebootCfgGetBool(GKeyFile *keyFile, const char *group, const char *key, uint8_t defVal)
{
    uint32_t value = PmAutoRebootCfgGetUint(keyFile, group, key, defVal);
    return (value != 0U) ? 1U : 0U;
}

/**
 * @brief  加载自动重启配置文件(pm_auto_reboot.ini)
 *         配置文件不存在或解析失败时保持缺省配置
 * @return int8_t  RET_ERR表示失败，RET_OK表示成功
 */
int8_t LoadPmAutoRebootConfig(void)
{
    GKeyFile *keyFile = NULL;
    GError *error = NULL;
    int8_t ret = RET_ERR;

    const char *path = PM_AUTO_REBOOT_CFG_PATH;
    if (0 == access(PM_AUTO_REBOOT_CFG_PATH_PRIMARY, F_OK)) {
        path = PM_AUTO_REBOOT_CFG_PATH_PRIMARY;
        LOG_Debug("Using primary config: %s", path);
    }

    keyFile = g_key_file_new();
    if (!keyFile)
    {
        LOG_Error("Error new key file");
        return RET_ERR;
    }

    if (0 == g_key_file_load_from_file(keyFile, path, G_KEY_FILE_NONE, &error))
    {
        LOG_Error("Error loading key file %s: %s, use default config", path, error ? error->message : "unknown");
        if (error != NULL)
        {
            g_error_free(error);
        }
        g_key_file_free(keyFile);
        return ret;
    }

    /* 启动时长自动重启配置 */
    g_upTimeAutoRebootCfg.durationMin =
        PmAutoRebootCfgGetUint(keyFile, CFG_GROUP_UP_TIME_AUTO_REBOOT, CFG_KEY_DURATION_MIN, DEFAULT_UP_TIME_DURATION_MIN);
    g_upTimeAutoRebootCfg.wakeUpReason =
        (uint8_t)PmAutoRebootCfgGetUint(keyFile, CFG_GROUP_UP_TIME_AUTO_REBOOT, CFG_KEY_WAKE_UP_REASON, PM_AUTO_REBOOT_WAKE_REASON_NO_LIMIT);
    g_upTimeAutoRebootCfg.rtcWakeUpTrigger =
        PmAutoRebootCfgGetBool(keyFile, CFG_GROUP_UP_TIME_AUTO_REBOOT, CFG_KEY_RTC_WAKE_UP_TRIGGER, DEFAULT_RTC_WAKE_UP_TRIGGER);

    /* 连续工作时长自动重启配置 */
    g_workTimeAutoRebootCfg.durationMin =
        PmAutoRebootCfgGetUint(keyFile, CFG_GROUP_WORK_TIME_AUTO_REBOOT, CFG_KEY_DURATION_MIN, DEFAULT_WORK_TIME_DURATION_MIN);
    g_workTimeAutoRebootCfg.wakeUpReason =
        (uint8_t)PmAutoRebootCfgGetUint(keyFile, CFG_GROUP_WORK_TIME_AUTO_REBOOT, CFG_KEY_WAKE_UP_REASON, PM_AUTO_REBOOT_WAKE_REASON_NO_LIMIT);

    LOG_Info("UpTimeAutoReboot: durationMin=%u, wakeUpReason=%u, rtcWakeUpTrigger=%u",
             g_upTimeAutoRebootCfg.durationMin, g_upTimeAutoRebootCfg.wakeUpReason, g_upTimeAutoRebootCfg.rtcWakeUpTrigger);
    LOG_Info("WorkTimeAutoReboot: durationMin=%u, wakeUpReason=%u",
             g_workTimeAutoRebootCfg.durationMin, g_workTimeAutoRebootCfg.wakeUpReason);

    g_key_file_free(keyFile);
    ret = RET_OK;
    return ret;
}

/**
 * @brief  获取启动时长自动重启配置
 * @param  cfg  输出配置结构体
 * @return void
 */
void GetPmUpTimeAutoRebootConfig(PmUpTimeAutoRebootCfg_t *cfg)
{
    if (cfg == NULL)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }
    *cfg = g_upTimeAutoRebootCfg;
}

/**
 * @brief  获取连续工作时长自动重启配置
 * @param  cfg  输出配置结构体
 * @return void
 */
void GetPmWorkTimeAutoRebootConfig(PmWorkTimeAutoRebootCfg_t *cfg)
{
    if (cfg == NULL)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }
    *cfg = g_workTimeAutoRebootCfg;
}
