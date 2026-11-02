#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <stdint.h>

#include "err_code.h"
#include "sr_log.h"
#include "net_config.h"

// 定义配置文件中的各个部分和键
#define BASE_SECTION "base"
#define LED_PIN_NUM_KEY "ledPinNum"
#define APN_NUM_KEY "apnNum"
#define AUTO_DIAL_KEY "netAutoDail"
#define AUTO_DNS_PATH_NAME "dnsPathName"
#define HIGH_SPEED_MODE_ENABLE "highSpeedModeEnable"
#define BEWINNER_SIM_CARD_ENABLE "beWinnerSimCardEnable"

#define APN_DIAL_PRI_INTERVAL_KEY "apnDialPriInterval"
#define APN_DIAL_SEC_INTERVAL_KEY "apnDialSecInterval"
#define APN_DIAL_SUCC_ONE_HOUR_THRESHOLD_COUNT_KEY "apnDialSuccOneHourThresHoldCount"

#define APN_DIAL_PRI_INTERVAL_DEFAULT (20)
#define APN_DIAL_SEC_INTERVAL_DEFAULT (120)
#define APN_DIAL_SUCC_ONE_HOUR_THRESHOLD_COUNT_DEFAULT (15)

#define PROFILE_INDEXDX "pid"
#define APN_SECTION_PREFIX "apn"
#define PDP_TYPE_KEY "pdpType"
#define AUTH_PROTO_KEY "authProto"
#define APN_NAME_KEY "apnName"
#define USER_NAME_KEY "apnUserName"
#define PASSWORD_KEY "apnPassWd"
#define ROUTE_IP_KEY "routeIp"
#define DOMAIN_KEY "domain"
#define NAMESAVER_PRI "nameserverPri"
#define DNS_ROUTE_SET "dnsRouteSet"
#define PRIVATE_NET "privateNet"

/* 网络异常检查配置 section/key 定义 */
#define CHECK_REG_SECTION "check_reg"
#define CHECK_REG_FAIL_TRIGGER_RESET_NET_PERIOD "regFailTriggerResetNetPeriod"
#define CHECK_REG_FAIL_RESET_NET_TRIGGER_RESET_TBOX_COUNT "regFailResetNetTriggerResetTboxCount"

#define CHECK_APN_DIAL_SECTION "check_apn_dial"
#define CHECK_APN_DIAL_FAIL_TRIGGER_RESET_NET_PERIOD "apnDialFailTriggerResetNetPeriod"
#define CHECK_APN_DIAL_FAIL_RESET_NET_TRIGGER_RESET_TBOX_COUNT "apnDialFailResetNetTriggerResetTboxCount"

#define CHECK_NET_TYPE_SECTION "check_net_type"
#define CHECK_NET_TYPE_RESET_NET_PERIOD "resetNetPeriod"
#define CHECK_NET_TYPE_RESET_NET_MAX_COUNT "resetNetMaxCount"

#define CHECK_NET_CONNECTION_SECTION "check_net_connection"
#define CHECK_NET_CONNECTION_PING_CYCLE_DURATION_SECOND "pingCycleDurationSecond"
#define CHECK_NET_CONNECTION_PING_FAILED_COUNT_TRIGGER_RESET_NET "pingFailedCountTriggerResetNet"
#define CHECK_NET_CONNECTION_PING_FAILED_RESET_NET_COUNT_TRIGGER_RESET_TBOX "pingFailedResetNetCountTriggerResetTbox"

/**
 * @brief  将字符串解析为 uint32_t
 * @param  str        输入字符串
 * @param  defaultVal 解析失败时返回的默认值
 * @return 解析后的 uint32_t 值
 * @details 自动跳过首尾空白和尾部 ';'，兼容 ini 中 value 带分号的写法。
 */
static uint32_t parse_uint32_from_string(const gchar *str, uint32_t defaultVal, gboolean *ok)
{
    if (ok != NULL)
    {
        *ok = FALSE;
    }

    if (str == NULL || *str == '\0')
    {
        return defaultVal;
    }

    gchar *endptr = NULL;
    unsigned long val = strtoul(str, &endptr, 10);

    /* 跳过数字后的空白 */
    while (endptr != NULL && isspace((unsigned char)*endptr))
    {
        endptr++;
    }

    /* 剩余字符只能是结束、; 注释或 # 注释 */
    if (endptr != NULL && *endptr != '\0' && *endptr != ';' && *endptr != '#')
    {
        return defaultVal;
    }

    if (val > UINT32_MAX)
    {
        return defaultVal;
    }

    if (ok != NULL)
    {
        *ok = TRUE;
    }
    return (uint32_t)val;
}

/**
 * @brief  从 keyfile 读取 uint32_t 类型配置项
 * @param  keyfile    GKeyFile 句柄
 * @param  group      section 名
 * @param  key        键名
 * @param  defaultVal 键不存在或解析失败时的默认值
 * @return 配置值
 */
static uint32_t key_file_get_uint32(GKeyFile *keyfile, const gchar *group, const gchar *key, uint32_t defaultVal)
{
    gchar *val = g_key_file_get_string(keyfile, group, key, NULL);
    uint32_t ret = parse_uint32_from_string(val, defaultVal, NULL);
    g_free(val);
    return ret;
}

/**
 * @brief  解析逗号分隔的 resetNetPeriod 数组
 * @param  keyfile     GKeyFile 句柄
 * @param  group       section 名
 * @param  key         键名
 * @param  periodArray 输出数组
 * @param  periodCount 输出实际解析到的元素个数
 * @param  maxCount    数组最大容量
 */
static void parse_reset_net_period(GKeyFile *keyfile, const gchar *group, const gchar *key,
                                   uint32_t *periodArray, uint32_t *periodCount, uint32_t maxCount)
{
    gchar *val = g_key_file_get_string(keyfile, group, key, NULL);
    *periodCount = 0;

    if (val == NULL || *val == '\0')
    {
        g_free(val);
        return;
    }

    /* 截断 inline 注释（# 或 ; 之后的内容），避免注释中的逗号干扰解析 */
    for (gchar *p = val; *p != '\0'; p++)
    {
        if (*p == '#' || *p == ';')
        {
            *p = '\0';
            break;
        }
    }

    gchar *token = NULL;
    gchar *saveptr = NULL;
    gchar *valCopy = g_strdup(val);

    token = strtok_r(valCopy, ",", &saveptr);
    while (token != NULL && *periodCount < maxCount)
    {
        gboolean ok = FALSE;
        uint32_t parsed = parse_uint32_from_string(token, 0, &ok);
        if (ok)
        {
            periodArray[*periodCount] = parsed;
            (*periodCount)++;
        }
        token = strtok_r(NULL, ",", &saveptr);
    }

    g_free(valCopy);
    g_free(val);
}

/**
 * @brief  从配置文件加载网络异常检查参数
 * @param  keyfile GKeyFile 句柄
 * @param  param   输出结构体指针
 * @details 按配置文件读取：
 *          - section 存在：enable=1，键缺失时使用 net_config.h 中定义的宏默认值；
 *          - section 不存在：enable=0（该检查项关闭）。
 */
static void LoadNetAnomalyCheckParam(GKeyFile *keyfile, NetAnomalyCheckParam_t *param)
{
    /* 驻网异常 */
    if (g_key_file_has_group(keyfile, CHECK_REG_SECTION))
    {
        param->netRegCheckEnable = 1;
        param->netRegCheckParam.regFailTriggerResetNetPeriod =
            key_file_get_uint32(keyfile, CHECK_REG_SECTION, CHECK_REG_FAIL_TRIGGER_RESET_NET_PERIOD,
                                NET_REG_FAIL_TRIGGER_RESET_NET_PERIOD_DEFAULT);
        param->netRegCheckParam.regFailResetNetTriggerResetTboxCount =
            key_file_get_uint32(keyfile, CHECK_REG_SECTION, CHECK_REG_FAIL_RESET_NET_TRIGGER_RESET_TBOX_COUNT,
                                NET_REG_FAIL_RESET_NET_TRIGGER_RESET_TBOX_COUNT_DEFAULT);
    }
    else
    {
        param->netRegCheckEnable = 0;
    }

    /* APN 拨号异常 */
    if (g_key_file_has_group(keyfile, CHECK_APN_DIAL_SECTION))
    {
        param->apnDialCheckEnable = 1;
        param->apnDialCheckParam.apnDialFailTriggerResetNetPeriod =
            key_file_get_uint32(keyfile, CHECK_APN_DIAL_SECTION, CHECK_APN_DIAL_FAIL_TRIGGER_RESET_NET_PERIOD,
                                NET_APN_DIAL_FAIL_TRIGGER_RESET_NET_PERIOD_DEFAULT);
        param->apnDialCheckParam.apnDialFailResetNetTriggerResetTboxCount =
            key_file_get_uint32(keyfile, CHECK_APN_DIAL_SECTION, CHECK_APN_DIAL_FAIL_RESET_NET_TRIGGER_RESET_TBOX_COUNT,
                                NET_APN_DIAL_FAIL_RESET_NET_TRIGGER_RESET_TBOX_COUNT_DEFAULT);
    }
    else
    {
        param->apnDialCheckEnable = 0;
    }

    /* 网络制式异常 */
    if (g_key_file_has_group(keyfile, CHECK_NET_TYPE_SECTION))
    {
        param->netTypeCheckEnable = 1;
        parse_reset_net_period(keyfile, CHECK_NET_TYPE_SECTION, CHECK_NET_TYPE_RESET_NET_PERIOD,
                               param->netTypeCheckParam.resetNetPeriod,
                               &param->netTypeCheckParam.periodCount,
                               sizeof(param->netTypeCheckParam.resetNetPeriod) / sizeof(uint32_t));
        param->netTypeCheckParam.resetNetMaxCount =
            key_file_get_uint32(keyfile, CHECK_NET_TYPE_SECTION, CHECK_NET_TYPE_RESET_NET_MAX_COUNT,
                                NET_TYPE_CHECK_RESET_NET_MAX_COUNT_DEFAULT);

        /* 若未解析到任何周期，使用默认周期 */
        if (param->netTypeCheckParam.periodCount == 0)
        {
            param->netTypeCheckParam.periodCount = 1;
            param->netTypeCheckParam.resetNetPeriod[0] = NET_TYPE_CHECK_RESET_NET_PERIOD_DEFAULT;
        }
    }
    else
    {
        param->netTypeCheckEnable = 0;
    }

    /* 网络连接异常（ping） */
    if (g_key_file_has_group(keyfile, CHECK_NET_CONNECTION_SECTION))
    {
        param->netConnectionCheckEnable = 1;
        param->netConnectionCheckParam.pingCycleDurationSecond =
            key_file_get_uint32(keyfile, CHECK_NET_CONNECTION_SECTION, CHECK_NET_CONNECTION_PING_CYCLE_DURATION_SECOND,
                                NET_CONNECTION_PING_CYCLE_DURATION_SECOND_DEFAULT);
        param->netConnectionCheckParam.pingFailedCountTriggerResetNet =
            key_file_get_uint32(keyfile, CHECK_NET_CONNECTION_SECTION, CHECK_NET_CONNECTION_PING_FAILED_COUNT_TRIGGER_RESET_NET,
                                NET_CONNECTION_PING_FAILED_COUNT_TRIGGER_RESET_NET_DEFAULT);
        param->netConnectionCheckParam.pingFailedResetNetCountTriggerResetTbox =
            key_file_get_uint32(keyfile, CHECK_NET_CONNECTION_SECTION, CHECK_NET_CONNECTION_PING_FAILED_RESET_NET_COUNT_TRIGGER_RESET_TBOX,
                                NET_CONNECTION_PING_FAILED_RESET_NET_COUNT_TRIGGER_RESET_TBOX_DEFAULT);
    }
    else
    {
        param->netConnectionCheckEnable = 0;
    }

    /* 打印最终生效的网络异常检查参数 */
    {
        char periodStr[256] = {0};
        int offset = 0;
        uint32_t maxPeriodCount = sizeof(param->netTypeCheckParam.resetNetPeriod) / sizeof(uint32_t);
        for (uint32_t i = 0; i < param->netTypeCheckParam.periodCount && i < maxPeriodCount; ++i)
        {
            offset += snprintf(periodStr + offset, sizeof(periodStr) - offset, "%s%u",
                               (i == 0) ? "" : ",", (unsigned)param->netTypeCheckParam.resetNetPeriod[i]);
            if (offset >= (int)sizeof(periodStr))
            {
                break;
            }
        }

        LOG_Warn("NetAnomalyCheckParam: ");
        LOG_Warn("netRegCheckEnable=%u, regFailTriggerResetNetPeriod=%u, regFailResetNetTriggerResetTboxCount=%u",
                  (unsigned)param->netRegCheckEnable,
                  (unsigned)param->netRegCheckParam.regFailTriggerResetNetPeriod,
                  (unsigned)param->netRegCheckParam.regFailResetNetTriggerResetTboxCount);
        LOG_Warn("apnDialCheckEnable=%u, apnDialFailTriggerResetNetPeriod=%u, apnDialFailResetNetTriggerResetTboxCount=%u",
                  (unsigned)param->apnDialCheckEnable,
                  (unsigned)param->apnDialCheckParam.apnDialFailTriggerResetNetPeriod,
                  (unsigned)param->apnDialCheckParam.apnDialFailResetNetTriggerResetTboxCount);
        LOG_Warn("netTypeCheckEnable=%u, periodCount=%u, resetNetPeriod=[%s], resetNetMaxCount=%u",
                  (unsigned)param->netTypeCheckEnable,
                  (unsigned)param->netTypeCheckParam.periodCount,
                  periodStr,
                  (unsigned)param->netTypeCheckParam.resetNetMaxCount);
        LOG_Warn("netConnectionCheckEnable=%u, pingCycleDurationSecond=%u, pingFailedCountTriggerResetNet=%u, pingFailedResetNetCountTriggerResetTbox=%u",
                  (unsigned)param->netConnectionCheckEnable,
                  (unsigned)param->netConnectionCheckParam.pingCycleDurationSecond,
                  (unsigned)param->netConnectionCheckParam.pingFailedCountTriggerResetNet,
                  (unsigned)param->netConnectionCheckParam.pingFailedResetNetCountTriggerResetTbox);
    }
}

static int copy_file_safe(const char *src, const char *dst)
{
    int ret = -1;
    int fd_src = -1;
    int fd_tmp = -1;
    char tmp_path[PATH_MAX];

    fd_src = open(src, O_RDONLY);
    if (fd_src < 0)
    {
        LOG_Error("copy_file_safe: open src failed: %s", src);
        return -1;
    }

    /* 临时文件必须与目标文件在同一目录，保证 rename() 在同一文件系统内原子完成，
     * 从而消除「目标文件被 O_TRUNC 截断为空/半截」的窗口（race 根因）。 */
    (void)snprintf(tmp_path, sizeof(tmp_path), "%s.tmp.%d", dst, (int)getpid());

    fd_tmp = open(tmp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_tmp < 0)
    {
        LOG_Error("copy_file_safe: open tmp failed: %s", tmp_path);
        close(fd_src);
        return -1;
    }

    char buf[4096];
    ssize_t nr = 0;
    int write_err = 0;

    while (!write_err && (nr = read(fd_src, buf, sizeof(buf))) > 0)
    {
        ssize_t written = 0;
        while (written < nr)
        {
            ssize_t nw = write(fd_tmp, buf + written, (size_t)(nr - written));
            if (nw < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                LOG_Error("copy_file_safe: write tmp failed");
                write_err = 1;
                break;
            }
            written += nw;
        }
    }

    if (write_err || nr < 0)
    {
        if (nr < 0)
        {
            LOG_Error("copy_file_safe: read src failed");
        }
        goto cleanup;
    }

    /* 数据落盘后再 rename，确保目标文件始终是完整内容 */
    if (fsync(fd_tmp) < 0)
    {
        LOG_Error("copy_file_safe: fsync tmp failed");
        goto cleanup;
    }

    if (close(fd_tmp) < 0)
    {
        fd_tmp = -1;
        LOG_Error("copy_file_safe: close tmp failed");
        goto cleanup;
    }
    fd_tmp = -1;

    if (rename(tmp_path, dst) < 0)
    {
        LOG_Error("copy_file_safe: rename failed: %s -> %s", tmp_path, dst);
        goto cleanup;
    }

    ret = 0;

cleanup:
    if (fd_tmp >= 0)
    {
        close(fd_tmp);
    }
    if (fd_src >= 0)
    {
        close(fd_src);
    }
    if (ret != 0)
    {
        (void)unlink(tmp_path);
    }
    return ret;
}

/*
 * 校验目标 ini 是否满足参考 ini 的结构要求：
 * 1. 参考文件中的每个 section，目标文件必须存在
 * 2. 参考文件中每个 section 下的每个 key，目标文件必须存在
 * 3. 仅校验“是否存在”，不比较 value 是否一致
 *
 * 说明：
 * - 允许目标 ini 比参考 ini 多 section / key
 * - 只要参考 ini 中要求的结构缺失，就认为不合法
 */
static gboolean IniStructureMatch(GKeyFile *ref_kf, GKeyFile *tgt_kf)
{
    gsize ref_group_num = 0;
    gchar **ref_groups = g_key_file_get_groups(ref_kf, &ref_group_num);
    gboolean match = TRUE;

    if (ref_groups == NULL)
    {
        LOG_Warn("IniStructureMatch: ref_groups is NULL");
        return FALSE;
    }

    for (gsize i = 0; i < ref_group_num; ++i)
    {
        gsize ref_key_num = 0;
        gchar **ref_keys = NULL;
        const gchar *group = ref_groups[i];

        if (!g_key_file_has_group(tgt_kf, group))
        {
            LOG_Warn("IniStructureMatch: missing group [%s]", group);
            match = FALSE;
            break;
        }

        ref_keys = g_key_file_get_keys(ref_kf, group, &ref_key_num, NULL);
        if (ref_keys == NULL)
        {
            /*
             * 若参考文件该 section 下没有 key，则按空 section 处理
             * 不作为失败条件
             */
            continue;
        }

        for (gsize j = 0; j < ref_key_num; ++j)
        {
            if (!g_key_file_has_key(tgt_kf, group, ref_keys[j], NULL))
            {
                LOG_Warn("IniStructureMatch: missing key [%s] in group [%s]", ref_keys[j], group);
                match = FALSE;
                break;
            }
        }

        g_strfreev(ref_keys);

        if (!match)
        {
            break;
        }
    }

    g_strfreev(ref_groups);
    return match;
}

/**
 * 启动时保证 tboxNet.ini 存在且格式正确：
 * - 不存在：从 oemapp net.ini 拷贝
 * - 存在但格式不合法：重新从 oemapp 拷贝
 *
 * “格式合法”的定义：
 * - oemapp net.ini 中存在的每个 section，tboxNet.ini 中也必须存在
 * - oemapp net.ini 中每个 section 下存在的每个 key，tboxNet.ini 中也必须存在
 * - key 对应的 value 可以不同，不做比较
 */
static void SanitizeTboxNetIni(void)
{
    if (access(NET_FIRST_FILE_NAME, F_OK) != 0)
    {
        LOG_Info("SanitizeTboxNetIni: tboxNet.ini not found, copy from oemapp");
        if (copy_file_safe(NET_CONFIG_FILE_NAME, NET_FIRST_FILE_NAME) != 0)
        {
            LOG_Error("SanitizeTboxNetIni: initial copy failed");
        }
        else
        {
            LOG_Info("SanitizeTboxNetIni: tboxNet.ini created from oemapp net.ini");
        }
        return;
    }

    gboolean need_recopy = FALSE;
    GKeyFile *ref_kf = g_key_file_new();
    GKeyFile *tgt_kf = g_key_file_new();

    if (!g_key_file_load_from_file(ref_kf, NET_CONFIG_FILE_NAME, G_KEY_FILE_NONE, NULL))
    {
        LOG_Warn("SanitizeTboxNetIni: cannot load oemapp net.ini, skip format check");
    }
    else if (!g_key_file_load_from_file(tgt_kf, NET_FIRST_FILE_NAME, G_KEY_FILE_NONE, NULL))
    {
        LOG_Warn("SanitizeTboxNetIni: tboxNet.ini parse failed, re-copy from oemapp");
        need_recopy = TRUE;
    }
    else if (!IniStructureMatch(ref_kf, tgt_kf))
    {
        LOG_Warn("SanitizeTboxNetIni: tboxNet.ini structure mismatch, re-copy from oemapp");
        need_recopy = TRUE;
    }

    g_key_file_unref(ref_kf);
    g_key_file_unref(tgt_kf);

    if (need_recopy)
    {
        if (copy_file_safe(NET_CONFIG_FILE_NAME, NET_FIRST_FILE_NAME) != 0)
        {
            LOG_Error("SanitizeTboxNetIni: re-copy failed");
        }
        else
        {
            LOG_Info("SanitizeTboxNetIni: tboxNet.ini restored from oemapp net.ini");
        }
    }
}

int8_t LoadNetConfig(NetUserCfg_t *cfg, uint8_t pathNamLen)
{
    GKeyFile *keyfile;
    GError *error = NULL;
    gboolean success = FALSE;

    if(NULL == cfg || pathNamLen <= 0)
    {
        LOG_Error("%s : parameter is NULL.",__FUNCTION__);
        return RET_ERR;
    }

    // SanitizeTboxNetIni();

    // 初始化
    memset(cfg, 0, sizeof(NetUserCfg_t));

    keyfile = g_key_file_new();

    // 先从首选配置文件中加载配置
    if (!g_key_file_load_from_file(keyfile, NET_FIRST_FILE_NAME, G_KEY_FILE_NONE, &error))
    {
        LOG_Warn("Error loading first file: %s", error->message);
        g_error_free(error);
        error = NULL;

        // 如果首选配置文件加载失败，再尝试从第二个默认配置文件中加载配置
        if (!g_key_file_load_from_file(keyfile, NET_CONFIG_FILE_NAME, G_KEY_FILE_NONE, &error))
        {
            LOG_Error("Error loading second file: %s", error->message);
            g_error_free(error);
            g_key_file_free(keyfile);
            return RET_ERR;
        }
        else
        {
            success = TRUE;
            LOG_Debug("load file %s", NET_CONFIG_FILE_NAME);
        }
    }
    else
    {
        success = TRUE;
        LOG_Debug("load file %s", NET_FIRST_FILE_NAME);
    }

    if (success)
    {
        // 读取基本配置
        cfg->ledPinNum = g_key_file_get_integer(keyfile, BASE_SECTION, LED_PIN_NUM_KEY, NULL);
        cfg->apnNum = g_key_file_get_integer(keyfile, BASE_SECTION, APN_NUM_KEY, NULL);
        if(cfg->apnNum > NETWORK_APN_NUM)
        {
            cfg->apnNum = NETWORK_APN_NUM;
        }
        cfg->autoDail = g_key_file_get_integer(keyfile, BASE_SECTION, AUTO_DIAL_KEY, NULL);

        g_autofree gchar *val = g_key_file_get_string(keyfile, BASE_SECTION, AUTO_DNS_PATH_NAME, NULL);
        g_strlcpy((gchar *)cfg->dnsPathName, val ? val : "NULL", pathNamLen);

        GError *error = NULL;
        cfg->highSpeedModeEnable = g_key_file_get_integer(keyfile, BASE_SECTION, HIGH_SPEED_MODE_ENABLE, &error);
        if (error)
        {
            // 如果读取出错，则赋予默认值
            g_error_free(error);
            cfg->highSpeedModeEnable = 0;  // 默认不开启高速模式
            error = NULL;
        }
        cfg->supportBeWinnerSimCard = g_key_file_get_integer(keyfile, BASE_SECTION, BEWINNER_SIM_CARD_ENABLE, &error);
        if (error)
        {
            // 如果读取出错，则赋予默认值
            g_error_free(error);
            cfg->supportBeWinnerSimCard = 0;  // 默认不支持北伟多卡切换
            error = NULL;
        }

        // 读取 APN 拨号间隔相关配置
        cfg->apnDialPriInterval = g_key_file_get_integer(keyfile, BASE_SECTION, APN_DIAL_PRI_INTERVAL_KEY, &error);
        if (error || cfg->apnDialPriInterval <= 0)
        {
            if (error)
            {
                g_error_free(error);
                error = NULL;
            }
            cfg->apnDialPriInterval = APN_DIAL_PRI_INTERVAL_DEFAULT;
        }

        cfg->apnDialSecInterval = g_key_file_get_integer(keyfile, BASE_SECTION, APN_DIAL_SEC_INTERVAL_KEY, &error);
        if (error || cfg->apnDialSecInterval <= 0)
        {
            if (error)
            {
                g_error_free(error);
                error = NULL;
            }
            cfg->apnDialSecInterval = APN_DIAL_SEC_INTERVAL_DEFAULT;
        }

        cfg->apnDialSuccOneHourThreadHoldCount = g_key_file_get_integer(keyfile, BASE_SECTION, APN_DIAL_SUCC_ONE_HOUR_THRESHOLD_COUNT_KEY, &error);
        if (error || cfg->apnDialSuccOneHourThreadHoldCount <= 0)
        {
            if (error)
            {
                g_error_free(error);
                error = NULL;
            }
            cfg->apnDialSuccOneHourThreadHoldCount = APN_DIAL_SUCC_ONE_HOUR_THRESHOLD_COUNT_DEFAULT;
        }

        LOG_Debug("ledPinNum:%d, apnNum:%d, autoDail:%d, dnsPathName:%s, highSpeedModeEnable:%d, supportBeWinnerSimCard:%d",
                cfg->ledPinNum, cfg->apnNum, cfg->autoDail, (char *)cfg->dnsPathName, cfg->highSpeedModeEnable, cfg->supportBeWinnerSimCard);
        LOG_Debug("apnDialPriInterval:%d, apnDialSecInterval:%d, apnDialSuccOneHourThreadHoldCount:%d",
                cfg->apnDialPriInterval, cfg->apnDialSecInterval, cfg->apnDialSuccOneHourThreadHoldCount);

        // 读取每个 APN 的配置
        for (uint8_t i = 0; i < cfg->apnNum; ++i)
        {
            gchar *section = g_strdup_printf("%s%d", APN_SECTION_PREFIX, i);

            cfg->apnInfoCfg[i].profileIndex = g_key_file_get_integer(keyfile, section, PROFILE_INDEXDX, NULL);
            cfg->apnInfoCfg[i].pdpType = g_key_file_get_integer(keyfile, section, PDP_TYPE_KEY, NULL);
            cfg->apnInfoCfg[i].authProto = g_key_file_get_integer(keyfile, section, AUTH_PROTO_KEY, NULL);
            cfg->apnInfoCfg[i].nameserverPri = g_key_file_get_integer(keyfile, section, NAMESAVER_PRI, NULL);
            cfg->apnInfoCfg[i].dnsRouteSet = g_key_file_get_integer(keyfile, section, DNS_ROUTE_SET, NULL);
            cfg->apnInfoCfg[i].privateNet = g_key_file_get_integer(keyfile, section, PRIVATE_NET, 0);
            g_strlcpy(cfg->apnInfoCfg[i].apnName, g_key_file_get_string(keyfile, section, APN_NAME_KEY, NULL), MODEM_TEXT_LEN);
            g_strlcpy(cfg->apnInfoCfg[i].usrName, g_key_file_get_string(keyfile, section, USER_NAME_KEY, NULL), MODEM_TEXT_LEN);
            g_strlcpy(cfg->apnInfoCfg[i].password, g_key_file_get_string(keyfile, section, PASSWORD_KEY, NULL), MODEM_TEXT_LEN);
            g_strlcpy(cfg->apnInfoCfg[i].privateRoute, g_key_file_get_string(keyfile, section, ROUTE_IP_KEY, NULL), CONFIG_DOMAIN_LEN);
            g_strlcpy(cfg->apnInfoCfg[i].domain, g_key_file_get_string(keyfile, section, DOMAIN_KEY, NULL), CONFIG_DOMAIN_LEN);

            LOG_Debug("profileIndex : %d pdpType : %d authProto : %d private: %d apnName : %s usrName : %s password : %s privateRoute : %s domain : %s",
                cfg->apnInfoCfg[i].profileIndex, cfg->apnInfoCfg[i].pdpType, cfg->apnInfoCfg[i].authProto, cfg->apnInfoCfg[i].privateNet, cfg->apnInfoCfg[i].apnName,
                cfg->apnInfoCfg[i].usrName, cfg->apnInfoCfg[i].password, cfg->apnInfoCfg[i].privateRoute, cfg->apnInfoCfg[i].domain);

            g_free(section);
        }

        // 读取网络异常检查参数
        LoadNetAnomalyCheckParam(keyfile, &cfg->netAnomalyCheckParam);
    }

    // 释放 GKeyFile 结构
    g_key_file_free(keyfile);

    return RET_OK;
}

int Ql_GPIO_Init(uint8_t pinNumber, uint8_t pinDirection)
{
    char command[512] = {0};

    sprintf(command, "echo %d > /sys/class/gpio/export", pinNumber);
    int status = system(command);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.",__FUNCTION__,command);
    } else {
        LOG_Error("%s : exec cmd[ %s ] failed.",__FUNCTION__,command);
    }
    
    memset(command,0,sizeof(command));
    
    sprintf(command, "echo %s > /sys/class/gpio/gpio%d/direction",(pinDirection == 0) ? "in" : "out", pinNumber);
    status = system(command);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.",__FUNCTION__,command);
        return RET_OK;
    } else {
        LOG_Error("%s : exec cmd[ %s ] failed.",__FUNCTION__,command);
        return RET_ERR;
    }
    
}

int Ql_GPIO_SetLevel(uint8_t pinNumber, uint8_t pinLevel)
{
    char command[512] = {0};
    
    sprintf(command, "echo %d > /sys/class/gpio/gpio%d/value",pinLevel, pinNumber);
    int status = system(command);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.",__FUNCTION__,command);
        return RET_OK;
    } else {
        LOG_Debug("%s : exec cmd[ %s ] failed.",__FUNCTION__,command);
        return RET_ERR;
    }
    
}