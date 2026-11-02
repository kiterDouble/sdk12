/**
 * @file fty.c
 * @brief 周期同步信息到MCU
 * @author mia.zhong
 * @date 2025-09-22
 *
 * @copyright Copyright (c) 2025 SiRun (HeFei).
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <linux/mii.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/types.h>
#include <netinet/in.h>
#include <linux/ethtool.h>
#include <linux/if.h>

#include "err_code.h"
#include "hal_modem_sim.h"
#include "hal_modem_nw.h"
#include "hal_modem_ant.h"
#include "sr_hal_gnss.h"
#include "net_service_api.h"
#include "pm_server_api.h"
#include "modem_ipc_msg.h"
#include "gnss_ipc_msg.h"
#include "queue_msg.h"
#include "mcu_api.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "sr_param.h"
#include "modem_api.h"
#include "utils.h"
// #include "tspRpcApi.h"
#include "avntRpcApi.h"
#include "appRpcType.h"
#include "fty.h"
#include "mdio.h"
#include "udsCallbackEntry.h"
#include "didRegistry.h"

#define OP_CMCC 0u // 中国移动
#define OP_CUCC 1u // 中国联通
#define OP_CTCC 2u // 中国电信
#define OP_UNKNOWN 3u
#define MAX_FIELDS 32

static pthread_mutex_t g_ftyDataMutex = PTHREAD_MUTEX_INITIALIZER;
static ftyCircleDataToMcu_t g_ftyCircleData = {0};
static MPUPmStateChange_t g_pmData = {0, PM_STATE_NORMAL, 0, 0};
static ModemSrvNotify_t g_modemData = {0};
static GnssLocation_t g_gnssData = {0};
static GnssNmea_t g_RMCNmeaData = {0};
static GnssNmea_t g_GGANmeaData = {0};
static GnssNmea_t g_GSANmeaData = {0}; // 可获取PDOP/HDOP/VDOP
static GnssNmea_t g_GSVNmeaData = {0};
static GnssNmea_t g_GLLNmeaData = {0};
static GnssNmea_t g_VTGNmeaData = {0};
static int sockfd = 0;
static uint8_t g_ddrResult = 0; /* 0正常 1故障 */

/* 以太网版本标志：TRUE=以太网版本，FALSE=非以太网版本 */
static gboolean g_isEthernetVersion = TRUE;
static pthread_mutex_t g_ethernetVersionMutex = PTHREAD_MUTEX_INITIALIZER;

// EOL数据同步DID列表和状态管理
static const uint16_t g_eolDidList[] = {
    0xF190 
};
#define EOL_DID_COUNT (sizeof(g_eolDidList) / sizeof(g_eolDidList[0]))
static bool g_eolDidSyncStatus[EOL_DID_COUNT+1] = {false};  // 对应每个DID的同步状态

// EOL数据同步的条件变量和互斥锁
static pthread_mutex_t g_eolSyncMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_eolSyncCond = PTHREAD_COND_INITIALIZER;
static bool g_eolSyncResponseReceived = false;

#define reteck(ret)                                               \
    if (ret < 0)                                                  \
    {                                                             \
        LOG_Debug("%m! \"%s\" : line: %d\n", __func__, __LINE__); \
        goto lab;                                                 \
    }

static inline uint16_t clamp_u16(uint32_t v, uint16_t hi) { return (v > hi) ? hi : (uint16_t)v; }

static inline uint8_t clamp_u8(uint32_t v, uint8_t hi) { return (v > hi) ? hi : (uint8_t)v; }

static inline uint8_t infer_fix_dim(uint8_t used_sats, uint8_t gnss_state)
{
    // gnss_state 的 bit0-5 为定位质量（0未定位/1单点/2浮点/3RTK…）
    uint8_t quality = (uint8_t)(gnss_state & 0x3F);
    if (quality == 0 || used_sats < 3)
        return 0; // 搜索中/未定位
    if (used_sats >= 4)
        return 2; // 三维
    return 1;     // 二维
}

static int InList2Digit(const char *mnc2, const char *const list[], size_t n)
{
    size_t i = 0;

    for (i = 0; i < n; ++i)
    {
        if (strncmp(mnc2, list[i], 2) == 0)
        {
            return 1;
        }
    }

    return 0;
}

static void StoreU24Be(uint8_t out[3], uint32_t v)
{
    out[0] = (uint8_t)((v >> 16) & 0xFF);
    out[1] = (uint8_t)((v >> 8)  & 0xFF);
    out[2] = (uint8_t)(v & 0xFF);
}

/**
 * @brief HexCharToNibble
 * @purpose 将 1 个十六进制字符转换为 0~15；失败返回 false
 * @param c 输入字符
 * @param outNibble 输出 nibble
 * @return true=成功，false=失败
 */
static bool HexCharToNibble(char c, uint8_t *outNibble)
{
    bool ok = false;
    uint8_t val = 0U;

    if (outNibble != NULL)
    {
        if ((c >= '0') && (c <= '9'))
        {
            val = (uint8_t)(c - '0');
            ok = true;
        }
        else if ((c >= 'A') && (c <= 'F'))
        {
            val = (uint8_t)(10U + (uint8_t)(c - 'A'));
            ok = true;
        }
        else if ((c >= 'a') && (c <= 'f'))
        {
            val = (uint8_t)(10U + (uint8_t)(c - 'a'));
            ok = true;
        }
        else
        {
            ok = false;
        }

        if (ok)
        {
            *outNibble = val;
        }
    }

    return ok;
}

/**
 * @brief ParseHexByte
 * @purpose 解析两位十六进制为 1 字节；失败返回 false
 * @param hi 高位字符
 * @param lo 低位字符
 * @param outByte 输出字节
 * @return true=成功，false=失败
 */
static bool ParseHexByte(char hi, char lo, uint8_t *outByte)
{
    bool ok = false;
    uint8_t n1 = 0U;
    uint8_t n2 = 0U;

    if (outByte != NULL)
    {
        if ((HexCharToNibble(hi, &n1) == true) && (HexCharToNibble(lo, &n2) == true))
        {
            *outByte = (uint8_t)((uint8_t)(n1 << 4U) | n2);
            ok = true;
        }
    }

    return ok;
}

/**
 * @brief FindAsteriskIndex
 * @purpose 在给定长度内寻找 '*' 位置
 * @param s NMEA 字符串
 * @param len 长度
 * @param outIdx 输出索引
 * @return true=找到，false=未找到
 */
static bool FindAsteriskIndex(const char *s, uint32_t len, uint32_t *outIdx)
{
    bool found = false;
    uint32_t i = 0U;

    if ((s != NULL) && (outIdx != NULL))
    {
        for (i = 0U; i < len; i++)
        {
            if (s[i] == '*')
            {
                *outIdx = i;
                found = true;
                break;
            }
        }
    }

    return found;
}

/**
 * @brief NmeaChecksumOk
 * @purpose 校验 NMEA XOR checksum 是否正确（要求包含 *HH）
 * @param s NMEA 字符串（允许带 \r\n）
 * @param len 输入长度（建议使用 call->length）
 * @return true=checksum 通过，false=失败
 */
bool NmeaChecksumOk(const char *s, uint32_t len)
{
    bool ok = false;
    uint32_t starIdx = 0U;
    uint32_t i = 0U;
    uint8_t x = 0U;
    uint8_t expected = 0U;

    if ((s == NULL) || (len < 6U))
    {
        return false;
    }

    /* 必须以 '$' 开头 */
    if (s[0] != '$')
    {
        return false;
    }

    /* 找 '*' */
    if (FindAsteriskIndex(s, len, &starIdx) == false)
    {
        return false;
    }

    /* '*' 后面至少要有 2 个 hex */
    if ((starIdx + 2U) >= len)
    {
        return false;
    }

    /* 计算 XOR：从 '$' 后第一个字符开始，到 '*' 前 */
    x = 0U;
    for (i = 1U; i < starIdx; i++)
    {
        x ^= (uint8_t)s[i];
    }

    if (ParseHexByte(s[starIdx + 1U], s[starIdx + 2U], &expected) == false)
    {
        return false;
    }

    ok = (x == expected);
    return ok;
}

/**
 * @brief GetFieldByIndex
 * @purpose 获取第 idx 个字段的起始指针和长度（idx 从 0 开始；0 为 "$xxYYY" 头）
 * @param s NMEA 字符串
 * @param len 字符串长度
 * @param idx 字段索引
 * @param outPtr 输出字段起始指针
 * @param outLen 输出字段长度
 * @return true=找到字段，false=失败
 */
static bool GetFieldByIndex(const char *s, uint32_t len, uint32_t idx, const char **outPtr, uint32_t *outLen)
{
    bool ok = false;
    uint32_t i = 0U;
    uint32_t curIdx = 0U;
    uint32_t start = 0U;
    uint32_t end = 0U;
    uint32_t starIdx = len;

    if ((s == NULL) || (outPtr == NULL) || (outLen == NULL))
    {
        return false;
    }

    /* 找到 '*'，字段解析只到 '*' 前 */
    (void)FindAsteriskIndex(s, len, &starIdx);

    /* 字段0从开头开始 */
    start = 0U;
    end = 0U;
    curIdx = 0U;

    for (i = 0U; i <= starIdx; i++)
    {
        const char ch = (i < starIdx) ? s[i] : ','; /* i==starIdx 时模拟分隔符，便于收尾 */

        if ((ch == ',') || (i == starIdx))
        {
            end = i;

            if (curIdx == idx)
            {
                *outPtr = &s[start];
                *outLen = (end >= start) ? (end - start) : 0U;
                ok = true;
                break;
            }

            /* 下一个字段 */
            curIdx++;
            start = i + 1U;
        }
        else
        {
            /* continue */
        }
    }

    return ok;
}

/**
 * @brief ParseUint32
 * @purpose 解析十进制无符号整数（空字段失败）
 * @param s 字段指针
 * @param len 字段长度
 * @param outVal 输出值
 * @return true=成功，false=失败
 */
static bool ParseUint32(const char *s, uint32_t len, uint32_t *outVal)
{
    bool ok = false;
    uint32_t v = 0U;
    uint32_t i = 0U;

    if ((s == NULL) || (outVal == NULL) || (len == 0U))
    {
        return false;
    }

    for (i = 0U; i < len; i++)
    {
        if ((s[i] < '0') || (s[i] > '9'))
        {
            return false;
        }

        v = (v * 10U) + (uint32_t)(s[i] - '0');
    }

    *outVal = v;
    ok = true;
    return ok;
}

/*
 * 函数名称: IsEthernetVersion
 * 函数作用: 获取当前是否为以太网版本
 * 参数说明: 无
 * 返回值:
 *   TRUE  - 以太网版本
 *   FALSE - 非以太网版本
 */
gboolean IsEthernetVersion(void)
{
    gboolean result;
    pthread_mutex_lock(&g_ethernetVersionMutex);
    result = g_isEthernetVersion;
    pthread_mutex_unlock(&g_ethernetVersionMutex);
    return result;
}

/*
 * 函数名称: SetEthernetVersion
 * 函数作用: 设置以太网版本标志（由 main.c 调用）
 * 参数说明:
 *   isEthernet - TRUE=以太网版本，FALSE=非以太网版本
 * 返回值: 无
 */
void SetEthernetVersion(gboolean isEthernet)
{
    pthread_mutex_lock(&g_ethernetVersionMutex);
    g_isEthernetVersion = isEthernet;
    pthread_mutex_unlock(&g_ethernetVersionMutex);
    LOG_Info("SetEthernetVersion: g_isEthernetVersion=%d", isEthernet);
}

/*
 * 函数名称: GetEthLinkUp
 * 函数作用: 读取指定网口当前链路是否 Up（基于 ETHTOOL_GLINK）
 * 参数说明:
 *   ifname  - 网口名称(如 "eth0")
 *   linkUp  - 输出链路状态(TRUE=Up, FALSE=Down)
 * 返回值:
 *   TRUE  - 获取成功
 *   FALSE - 获取失败
 */
static gboolean GetEthLinkUp(const char *ifname, gboolean *linkUp)
{
    int fd;
    struct ifreq ifr;
    struct ethtool_value edata;

    if (ifname == NULL || linkUp == NULL)
    {
        return FALSE;
    }

    /* 非以太网版本，直接返回失败 */
    if (IsEthernetVersion() == FALSE)
    {
        return FALSE;
    }

    *linkUp = FALSE;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        return FALSE;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, (size_t)(IFNAMSIZ - 1));
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    memset(&edata, 0, sizeof(edata));
    edata.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = (caddr_t)&edata;

    if (ioctl(fd, SIOCETHTOOL, &ifr) == 0)
    {
        *linkUp = (edata.data != 0U) ? TRUE : FALSE;
        (void)close(fd);
        return TRUE;
    }

    (void)close(fd);
    return FALSE;
}

/*
 * 函数名称: IsEthTrafficActive
 * 函数作用: 判断网口是否存在实际收发流量（用于过滤“误报通讯错误”）
 * 参数说明:
 *   ifname - 网口名称(如 "eth0")
 * 返回值:
 *   TRUE  - 有流量(自上次检查以来 rx/tx 包计数有变化)
 *   FALSE - 无流量或读取失败
 */
static gboolean IsEthTrafficActive(const char *ifname)
{
    static uint64_t s_LastRx = 0U;
    static uint64_t s_LastTx = 0U;
    static gboolean s_Inited = FALSE;

    char path[128];
    FILE *fp;
    uint64_t rx = 0U;
    uint64_t tx = 0U;

    if (ifname == NULL)
    {
        return FALSE;
    }

    /* 非以太网版本，直接返回失败 */
    if (IsEthernetVersion() == FALSE)
    {
        return FALSE;
    }

    (void)snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/rx_packets", ifname);
    fp = fopen(path, "r");
    if (fp == NULL)
    {
        return FALSE;
    }
    if (fscanf(fp, "%llu", (unsigned long long *)&rx) != 1)
    {
        (void)fclose(fp);
        return FALSE;
    }
    (void)fclose(fp);

    (void)snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/tx_packets", ifname);
    fp = fopen(path, "r");
    if (fp == NULL)
    {
        return FALSE;
    }
    if (fscanf(fp, "%llu", (unsigned long long *)&tx) != 1)
    {
        (void)fclose(fp);
        return FALSE;
    }
    (void)fclose(fp);

    if (s_Inited == FALSE)
    {
        s_LastRx = rx;
        s_LastTx = tx;
        s_Inited = TRUE;
        return FALSE;
    }

    if ((rx != s_LastRx) || (tx != s_LastTx))
    {
        s_LastRx = rx;
        s_LastTx = tx;
        return TRUE;
    }

    s_LastRx = rx;
    s_LastTx = tx;
    return FALSE;
}

/* --- 复制一行并按逗号分隔，遇到 * 校验前终止 --- */
static uint8_t SplitNmeaLine(const char *src, char *buf, size_t bufsize, char *fields[], uint8_t max_fields)
{
    size_t n = 0;
    uint8_t count = 0;
    char *p = NULL;

    if (src == NULL || buf == NULL || fields == NULL)
    {
        return 0;
    }

    /* 拷贝并保证结尾 0 */
    n = strnlen(src, bufsize - 1);
    memcpy(buf, src, n);
    buf[n] = '\0';

    /* 首字段 */
    fields[count] = buf;
    count++;

    p = buf;
    while (*p != '\0' && count < max_fields)
    {
        if (*p == ',')
        {
            *p = '\0';
            if (*(p + 1) != '\0')
            {
                fields[count] = p + 1;
                count++;
            }
        }
        else if (*p == '*')
        {
            *p = '\0';
            break;
        }
        p++;
    }

    return count;
}

/* --- 解析无符号整数字段，允许空字段 --- */
static int ParseU8(const char *s, uint8_t *out)
{
    char *endp = NULL;
    long v = 0;

    if (out == NULL)
    {
        return -1;
    }

    if (s == NULL || s[0] == '\0')
    {
        *out = 0;
        return 0;
    }

    v = strtol(s, &endp, 10);
    if (endp == s)
    {
        return -1;
    }

    if (v < 0)
    {
        v = 0;
    }
    if (v > 255)
    {
        v = 255;
    }

    *out = (uint8_t)v;
    return 0;
}

static int ParseU16(const char *s, uint16_t *out)
{
    char *endp = NULL;
    long v = 0;

    if (out == NULL)
    {
        return -1;
    }

    if (s == NULL || s[0] == '\0')
    {
        *out = 0;
        return 0;
    }

    v = strtol(s, &endp, 10);
    if (endp == s)
    {
        return -1;
    }

    if (v < 0)
    {
        v = 0;
    }
    if (v > 65535)
    {
        v = 65535;
    }

    *out = (uint16_t)v;
    return 0;
}

static int ParseMilli0p001(const char *s, uint32_t *out_milli)
{
    char *endp = NULL;
    double f = 0.0;

    if (out_milli == NULL)
    {
        return -1;
    }

    if (s == NULL || s[0] == '\0')
    {
        *out_milli = 0;
        return 0;
    }

    f = strtod(s, &endp);
    if (endp == s)
    {
        return -1;
    }

    if (f < 0.0)
    {
        f = 0.0;
    }
    /* 规范上 DOP 合理范围 < 128，这里做个上限钳位 */
    if (f > 127.0)
    {
        f = 127.0;
    }

    *out_milli = (uint32_t)(f * 1000.0 + 0.5);
    return 0;
}

static uint8_t DetectChinaOperatorFromMccMnc(const char *mccmnc_in)
{
    if (mccmnc_in == NULL)
    {
        return OP_UNKNOWN;
    }

    char buf[16] = {0};
    size_t len = 0;

    while (isspace((unsigned char)*mccmnc_in))
    {
        ++mccmnc_in;
    }

    /* 拷贝最多 15 个非空白字符 */
    while (*mccmnc_in && len < sizeof(buf) - 1)
    {
        if (isspace((unsigned char)*mccmnc_in))
        {
            break;
        }

        buf[len] = *mccmnc_in;
        ++len;
        ++mccmnc_in;
    }

    buf[len] = '\0';

    if (len < 5)
    {
        return OP_UNKNOWN; /* 至少 "460" + 2 位 MNC */
    }

    if (strncmp(buf, "460", 3) != 0)
    {
        return OP_UNKNOWN; /* 非中国大陆 MCC */
    }

    const char *mnc = buf + 3;
    char mnc2[3] = {0};

    mnc2[0] = mnc[0];
    mnc2[1] = mnc[1];

    static const char *const CMCC_2D[] = { "00", "02", "07", "08", "20" };
    static const char *const CUCC_2D[] = { "01", "06", "09" };
    static const char *const CTCC_2D[] = { "03", "05", "11" };

    if (InList2Digit(mnc2, CMCC_2D, sizeof(CMCC_2D) / sizeof(CMCC_2D[0])))
    {
        return OP_CMCC;
    }

    if (InList2Digit(mnc2, CUCC_2D, sizeof(CUCC_2D) / sizeof(CUCC_2D[0])))
    {
        return OP_CUCC;
    }

    if (InList2Digit(mnc2, CTCC_2D, sizeof(CTCC_2D) / sizeof(CTCC_2D[0])))
    {
        return OP_CTCC;
    }

    return OP_UNKNOWN;
}

/* --- 从一条 GSV 里取可见数与最多 4 颗卫星 --- */
static void ParseGsvNmeaInfo(const char *gsv, GpsSatInfo_1to4_t *out)
{
    char buf[128] = {0};
    char *fld[MAX_FIELDS] = {0};
    uint8_t n = 0;
    uint8_t i = 0;

    if (gsv == NULL || out == NULL)
    {
        return;
    }

    n = SplitNmeaLine(gsv, buf, sizeof(buf), fld, MAX_FIELDS);
    if (n < 4)
    {
        return;
    }

    /* $GPGSV,total_msgs,msg_num,sats_in_view,... */
    /* 可见卫星数 */
    {
        uint8_t sv = 0;
        if (ParseU8(fld[3], &sv) == 0)
        {
            out->visibleCount = sv;
        }
    }

    /* 填充最多 4 颗卫星 */
    for (i = 0; i < 4; i++)
    {
        uint8_t base = (uint8_t)(4 + i * 4);
        uint8_t id = 0;
        uint8_t elev = 0;
        uint16_t az = 0;
        uint8_t snr = 0;

        if (base + 3 >= n)
        {
            break;
        }

        if (ParseU8(fld[base + 0], &id) != 0)
        {
            id = 0;
        }
        if (ParseU8(fld[base + 1], &elev) != 0)
        {
            elev = 0;
        }
        if (ParseU16(fld[base + 2], &az) != 0)
        {
            az = 0;
        }
        if (ParseU8(fld[base + 3], &snr) != 0)
        {
            /* 末字段可能为空或带 * 校验，split 已去掉 *，这里只保底为 0 */
            snr = 0;
        }

        out->sat[i].id = id;
        out->sat[i].elevation = elev;
        out->sat[i].azimuth = az;
        out->sat[i].cnr = snr;
    }
}

/* 解析一条 GSA，把 PDOP/HDOP/VDOP 写入 24bit */
static void ParseGsaNmeaInfo(const char *gsa, GpsSatInfo_1to4_t *out)
{
    char buf[GNSS_NMEA_MAX_LEN] = {0};
    char *f[MAX_FIELDS] = {0};
    uint8_t n = 0;
    uint32_t v = 0;

    if (gsa == NULL || out == NULL)
    {
        return;
    }

    n = SplitNmeaLine(gsa, buf, sizeof(buf), f, MAX_FIELDS);
    if (n < 18)  /* 最少要到 VDOP 字段 */
    {
        return;
    }

    if (ParseMilli0p001(f[15], &v) == 0)
    {
        StoreU24Be(out->pdop, v);
    }

    if (ParseMilli0p001(f[16], &v) == 0)
    {
        StoreU24Be(out->hdop, v);
    }

    if (ParseMilli0p001(f[17], &v) == 0)
    {
        StoreU24Be(out->vdop, v);
    }
}

static int GetMpuOverTemperatureStatus(uint8_t *status)
{
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (fp == NULL) {
        perror("fopen failed");
        return -1;
    }

    char buf[64] = {0};
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        perror("fgets failed");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    long temp_mC = atol(buf);  /* 温度值通常是毫摄氏度，例如 45000 表示 45.0°C */
    double temp_C = temp_mC / 1000.0;

    LOG_Debug("Current temperature: %.1f°C\n", temp_C);
    if (temp_C > 120.0)
    {
        *status = 1;
    }
    else
    {
        *status = 0;
    }

    return 0;
}

static int EnsureDirectoryExists(const char *dirPath)
{
    if (dirPath == NULL)
    {
        return -1;
    }

    /* 逐级创建 /sdcard 与 sr_config；/sdcard 题述已存在，这里仍做健壮处理 */
    char pathBuf[256];
    size_t len = strlen(dirPath);
    if (len == 0U || len >= sizeof(pathBuf))
    {
        LOG_Debug("EnsureDirectoryExists: invalid path length");
        return -1;
    }

    memcpy(pathBuf, dirPath, len);
    pathBuf[len] = '\0';

    for (char *p = pathBuf + 1; *p != '\0'; ++p)
    {
        if (*p == '/')
        {
            *p = '\0';
            if (access(pathBuf, F_OK) != 0)
            {
                if (mkdir(pathBuf, 0755) != 0 && errno != EEXIST)
                {
                    LOG_Debug("mkdir '%s' failed: %s", pathBuf, strerror(errno));
                    *p = '/';
                    return -1;
                }
            }
            *p = '/';
        }
    }

    if (access(pathBuf, F_OK) != 0)
    {
        if (mkdir(pathBuf, 0755) != 0 && errno != EEXIST)
        {
            LOG_Debug("mkdir '%s' failed: %s", pathBuf, strerror(errno));
            return -1;
        }
    }

    return 0;
}

static int SaveDdrInfoToFile(const char *filePath, const DdrInfo_t *info)
{
    if (filePath == NULL || info == NULL)
    {
        return -1;
    }

    FILE *fp = fopen(filePath, "wb");
    if (fp == NULL)
    {
        LOG_Debug("Open file for write failed: %s", strerror(errno));
        return -1;
    }

    size_t n = fwrite(info, 1U, sizeof(DdrInfo_t), fp);
    if (n != sizeof(DdrInfo_t))
    {
        LOG_Debug("Write size mismatch: %zu", n);
        fclose(fp);
        return -1;
    }

    if (fflush(fp) != 0)
    {
        LOG_Debug("fflush failed: %s", strerror(errno));
    }

    fclose(fp);
    LOG_Debug("Persisted ddr info: set=%u read=%u result=%u",
         (unsigned)info->ddrSet,
        (unsigned)info->ddrRead,
        (unsigned)info->ddrResult);
    return 0;
}

static int LoadDdrInfoFromFile(const char *filePath, DdrInfo_t *info)
{
    if (filePath == NULL || info == NULL)
    {
        return -1;
    }

    FILE *fp = fopen(filePath, "rb");
    if (fp == NULL)
    {
        return -2; /* 文件不存在或无法打开 */
    }

    size_t n = fread(info, 1U, sizeof(DdrInfo_t), fp);
    fclose(fp);

    if (n != sizeof(DdrInfo_t))
    {
        LOG_Debug("Read size mismatch: %zu, reset defaults", n);
        info->ddrSet = 0U;
        info->ddrRead = 0U;
        info->ddrResult = 1U; /* 保守置为故障 */
        return -3;
    }

    LOG_Debug("Loaded ddr info: set=%u read=%u result=%u",
         (unsigned)info->ddrSet,
        (unsigned)info->ddrRead,
        (unsigned)info->ddrResult);
    return 0;
}

static int ParseDdrResultFromOutput(const char *out, uint8_t *result)
{
    if (out == NULL || result == NULL) {
        return -1;
    }

    /* 1) 明确 PASS */
    if (strstr(out, "DDR PASS") != NULL) {
        *result = 0U; /* 成功 */
        return 0;
    }

    /* 2) FAIL 的特殊放行：出现 FAIL 但 fail=0 也算成功 */
    const char *pFail = strstr(out, "DDR FAIL");
    if (pFail != NULL) {
        if (strstr(out, "fail 0") != NULL) {
            *result = 0U; /* 成功 */
            return 0;
        }
        *result = 1U; /* 其它 FAIL 视为失败 */
        return 0;
    }

    /* 3) 没有出现 PASS/FAIL 关键词，交由上层按保守策略处理 */
    return -2;
}

void UpdateDdrResultRam(uint8_t value)
{
    g_ddrResult = (value != 0U) ? 1U : 0U;
    LOG_Debug("RAM g_ddrResult updated to: %u", (unsigned)g_ddrResult);
}

/**
 * @brief IsRmcValid
 * @purpose 判定 RMC 是否有效（Status='A' 且经纬度字段非空）
 * @param s NMEA 字符串
 * @param len 长度
 * @return true=有效，false=无效
 */
bool IsRmcValid(const char *s, uint32_t len)
{
    const char *statusPtr = NULL;
    uint32_t statusLen = 0U;
    const char *latPtr = NULL;
    uint32_t latLen = 0U;
    const char *nsPtr = NULL;
    uint32_t nsLen = 0U;
    const char *lonPtr = NULL;
    uint32_t lonLen = 0U;
    const char *ewPtr = NULL;
    uint32_t ewLen = 0U;

    /* RMC 字段：
       0:$xxRMC
       1:UTC
       2:Status A/V
       3:Lat
       4:N/S
       5:Lon
       6:E/W
    */
    if (NmeaChecksumOk(s, len) == false)
    {
        return false;
    }

    if (GetFieldByIndex(s, len, 2U, &statusPtr, &statusLen) == false)
    {
        return false;
    }
    if ((statusLen != 1U) || (statusPtr[0] != 'A'))
    {
        return false;
    }

    (void)GetFieldByIndex(s, len, 3U, &latPtr, &latLen);
    (void)GetFieldByIndex(s, len, 4U, &nsPtr, &nsLen);
    (void)GetFieldByIndex(s, len, 5U, &lonPtr, &lonLen);
    (void)GetFieldByIndex(s, len, 6U, &ewPtr, &ewLen);

    if ((latLen == 0U) || (nsLen != 1U) || (lonLen == 0U) || (ewLen != 1U))
    {
        return false;
    }

    return true;
}

/**
 * @brief IsGgaValid
 * @purpose 判定 GGA 是否有效（FixQuality > 0）
 * @param s NMEA 字符串
 * @param len 长度
 * @return true=有效，false=无效
 */
bool IsGgaValid(const char *s, uint32_t len)
{
    const char *qPtr = NULL;
    uint32_t qLen = 0U;
    uint32_t qVal = 0U;

    /* GGA 字段（常见）：
       0:$xxGGA
       6:FixQuality
    */
    if (NmeaChecksumOk(s, len) == false)
    {
        return false;
    }

    if (GetFieldByIndex(s, len, 6U, &qPtr, &qLen) == false)
    {
        return false;
    }

    if (ParseUint32(qPtr, qLen, &qVal) == false)
    {
        return false;
    }

    return (qVal > 0U);
}

/**
 * @brief IsGsaValid
 * @purpose 判定 GSA 是否有效（FixType >= 2）
 * @param s NMEA 字符串
 * @param len 长度
 * @return true=有效，false=无效
 */
bool IsGsaValid(const char *s, uint32_t len)
{
    const char *fixPtr = NULL;
    uint32_t fixLen = 0U;
    uint32_t fixVal = 0U;

    /* GSA 字段：
       0:$xxGSA
       2:Mode(M/A)
       3:FixType(1/2/3)  <-- 注意：部分资料写第 2/3 字段不同，这里以常见格式：
            $GxGSA,<Mode1>,<FixType>,...
       所以 FixType 通常是字段2 (idx=2) 或 字段3 (idx=3)？
       实测多数模块：idx=2 为 FixType（因为 idx=1 是模式 M/A，idx=2 是 FixType）
       你如果不确定，我这里做“双位置兼容”：优先 idx=2，不行再试 idx=3。
    */
    if (NmeaChecksumOk(s, len) == false)
    {
        return false;
    }

    if (GetFieldByIndex(s, len, 2U, &fixPtr, &fixLen) == true)
    {
        if (ParseUint32(fixPtr, fixLen, &fixVal) == true)
        {
            return (fixVal >= 2U);
        }
    }

    if (GetFieldByIndex(s, len, 3U, &fixPtr, &fixLen) == true)
    {
        if (ParseUint32(fixPtr, fixLen, &fixVal) == true)
        {
            return (fixVal >= 2U);
        }
    }

    return false;
}

/**
 * @brief IsGllValid
 * @purpose 判定 GLL 是否有效（Status='A'；若存在 Mode 且为 'N' 则无效）
 * @param s NMEA 字符串
 * @param len 长度
 * @return true=有效，false=无效
 */
bool IsGllValid(const char *s, uint32_t len)
{
    const char *statusPtr = NULL;
    uint32_t statusLen = 0U;
    const char *modePtr = NULL;
    uint32_t modeLen = 0U;
    const char *latPtr = NULL;
    uint32_t latLen = 0U;
    const char *nsPtr = NULL;
    uint32_t nsLen = 0U;
    const char *lonPtr = NULL;
    uint32_t lonLen = 0U;
    const char *ewPtr = NULL;
    uint32_t ewLen = 0U;

    /* GLL 常见：
       0:$xxGLL
       1:Lat
       2:N/S
       3:Lon
       4:E/W
       6:Status(A/V)
       7:Mode(可选，A/D/E/N)
    */
    if (NmeaChecksumOk(s, len) == false)
    {
        return false;
    }

    (void)GetFieldByIndex(s, len, 1U, &latPtr, &latLen);
    (void)GetFieldByIndex(s, len, 2U, &nsPtr, &nsLen);
    (void)GetFieldByIndex(s, len, 3U, &lonPtr, &lonLen);
    (void)GetFieldByIndex(s, len, 4U, &ewPtr, &ewLen);

    if ((latLen == 0U) || (nsLen != 1U) || (lonLen == 0U) || (ewLen != 1U))
    {
        return false;
    }

    if (GetFieldByIndex(s, len, 6U, &statusPtr, &statusLen) == false)
    {
        return false;
    }
    if ((statusLen != 1U) || (statusPtr[0] != 'A'))
    {
        return false;
    }

    /* Mode 可选：如果存在且为 N，则判无效 */
    if (GetFieldByIndex(s, len, 7U, &modePtr, &modeLen) == true)
    {
        if ((modeLen == 1U) && (modePtr[0] == 'N'))
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief IsVtgValid
 * @purpose 判定 VTG 是否“速度/航向数据可用”（不作为定位 fix 判据）
 * @param s NMEA 字符串
 * @param len 长度
 * @return true=可用，false=不可用
 */
bool IsVtgValid(const char *s, uint32_t len)
{
    const char *kmhPtr = NULL;
    uint32_t kmhLen = 0U;
    const char *knPtr = NULL;
    uint32_t knLen = 0U;
    const char *modePtr = NULL;
    uint32_t modeLen = 0U;

    /* VTG 常见：
       0:$xxVTG
       5:SpeedKnots
       7:SpeedKmh
       9:Mode(可选)
       这里做“字段存在即可”的工程判定：knots 或 kmh 非空，并且 Mode!=N(若有)
    */
    if (NmeaChecksumOk(s, len) == false)
    {
        return false;
    }

    (void)GetFieldByIndex(s, len, 5U, &knPtr, &knLen);
    (void)GetFieldByIndex(s, len, 7U, &kmhPtr, &kmhLen);

    if ((knLen == 0U) && (kmhLen == 0U))
    {
        return false;
    }

    if (GetFieldByIndex(s, len, 9U, &modePtr, &modeLen) == true)
    {
        if ((modeLen == 1U) && (modePtr[0] == 'N'))
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief IsGsvValid
 * @purpose 判定 GSV 是否“结构有效”（不作为定位 fix 判据）
 * @param s NMEA 字符串
 * @param len 长度
 * @return true=结构看起来有效，false=无效
 */
bool IsGsvValid(const char *s, uint32_t len)
{
    const char *totalMsgPtr = NULL;
    uint32_t totalMsgLen = 0U;
    const char *msgNumPtr = NULL;
    uint32_t msgNumLen = 0U;
    const char *totalSvPtr = NULL;
    uint32_t totalSvLen = 0U;
    uint32_t totalMsg = 0U;
    uint32_t msgNum = 0U;
    uint32_t totalSv = 0U;

    /* GSV 常见：
       0:$xxGSV
       1:TotalMsgs
       2:MsgNum
       3:TotalSV
    */
    if (NmeaChecksumOk(s, len) == false)
    {
        return false;
    }

    if ((GetFieldByIndex(s, len, 1U, &totalMsgPtr, &totalMsgLen) == false) ||
        (GetFieldByIndex(s, len, 2U, &msgNumPtr, &msgNumLen) == false) ||
        (GetFieldByIndex(s, len, 3U, &totalSvPtr, &totalSvLen) == false))
    {
        return false;
    }

    if ((ParseUint32(totalMsgPtr, totalMsgLen, &totalMsg) == false) ||
        (ParseUint32(msgNumPtr, msgNumLen, &msgNum) == false) ||
        (ParseUint32(totalSvPtr, totalSvLen, &totalSv) == false))
    {
        return false;
    }

    if ((totalMsg == 0U) || (msgNum == 0U) || (msgNum > totalMsg))
    {
        return false;
    }

    /* TotalSV 允许为 0（比如搜星中），但结构依然算有效 */
    (void)totalSv;

    return true;
}

// DTC 状态表
static void UpdateDtcTableData(DtcTable_t *dtc)
{
    int rc = system("mount | grep -qE ' on (/mnt/sdcard)(/| )'");
    if (rc == -1)
    {
        LOG_Debug("system call failed");
        dtc->storeState = 1;
    }
    else
    {
        if (WIFEXITED(rc))
        {
            int code = WEXITSTATUS(rc);
            if (code != 0) // 找到了挂载项 => 已挂载
            {
                dtc->storeState = 1;
            }
            else
            {
                dtc->storeState = 0;
            }
        }
        else
        {
            dtc->storeState = 0;
        }
    }
    if (g_modemData.simCardStatus != 0x00 || g_modemData.simAppStatus != 0x00)
    {
        dtc->simFault = 1;
    }
    else
    {
        dtc->simFault = 0;
    }

    // 0正常 1开路 2短到电源 3短到地
    if (g_modemData.antennaStatus == E_MODEM_SRV_ANT_STATUS_CIRCUIT_NORMAL)
    {
        dtc->antMain = 0;
    }
    else if (g_modemData.antennaStatus == E_MODEM_SRV_ANT_STATUS_CIRCUIT_OPEN)
    {
        dtc->antMain = 1;
    }
    else if (g_modemData.antennaStatus == E_MODEM_SRV_ANT_STATUS_CIRCUIT_SHORT)
    {
        dtc->antMain = 3;
    }
    else
    {
        dtc->antMain = 2;
    }

    if (g_modemData.secondaryAntennaStatus == E_MODEM_SRV_ANT_STATUS_CIRCUIT_NORMAL)
    {
        dtc->antSec = 0;
    }
    else if (g_modemData.secondaryAntennaStatus == E_MODEM_SRV_ANT_STATUS_CIRCUIT_OPEN)
    {
        dtc->antSec = 1;
    }
    else if (g_modemData.secondaryAntennaStatus == E_MODEM_SRV_ANT_STATUS_CIRCUIT_SHORT)
    {
        dtc->antSec = 3;
    }
    else
    {
        dtc->antSec = 2;
    }

    dtc->ethState = 0;

    /* 以太网版本才执行以太网状态检测 */
    if (IsEthernetVersion() == TRUE)
    {
        /* 先读取当前链路状态：能够互 ping 的前提通常是 Link=Up */
        gboolean linkUp = FALSE;
        gboolean linkUpValid = GetEthLinkUp(PHY_IFNAME, &linkUp);
        gboolean trafficActive = FALSE;
        if ((linkUpValid == TRUE) && (linkUp == TRUE))
        {
            trafficActive = IsEthTrafficActive(PHY_IFNAME);
        }

        /* 0x79: 以太网线路/PHY 故障；detailBits:
         * bit0=系统初始化失败, bit1=线路开短/中断, bit2=SMI访问失败
         *
         * 注意：线路诊断位可能为瞬态或锁存(latch)。为避免”Link 已 Up 仍被判链路失败”，
         * 对 bit1 做 Link 门控：仅在 Link=Down 时才上升为 ethState=2。
         */
        uint8_t detailBits = 0;
        int code79 = GetEthernetLineFailure(PHY_IFNAME, &detailBits);
        if (code79 == 0x79)
        {
            /*
             * 线路诊断/PHY 告警位可能为锁存或瞬态。
             * 当 Link=Up 且存在实际收发流量时，认为”对外供网可用”，不使用 0x79/detailBits 上升 DTC 状态。
             */
            if (trafficActive == FALSE)
            {
                if ((detailBits & 0x01U) != 0U)
                {
                    dtc->ethState = 1;
                }
                else if ((detailBits & 0x04U) != 0U)
                {
                    dtc->ethState = 3;
                }
                else if ((detailBits & 0x02U) != 0U)
                {
                    if ((linkUpValid == TRUE) && (linkUp == TRUE))
                    {
                        /* Link 已 Up：不将线路诊断位直接判为”链路失败”，避免误报 */
                        dtc->ethState = 0;
                    }
                    else
                    {
                        dtc->ethState = 2;
                    }
                }
                else
                {
                    /* do nothing */
                }
            }
        }

        int code08 = 0;
#if 1
        /* 0x08: 以太网通讯错误（业务层/协议层）。
         * 说明：通讯错误与”线路/PHY/SMI”类故障并不严格互斥。
         *       为了让外部能”知道通讯错误”，这里将 0x08 作为独立检测项。
         *       但若已判定为更底层的硬故障（INIT_FAIL/SMI_FAIL），则保持硬故障优先级。
         */
        {
            code08 = GetEthernetCommFailure(PHY_IFNAME);  /* 0x08=通讯错误，0=正常 */
            if (code08 == 0x08)
            {
                /*
                 * 现场表现：Link=Up 且外部设备可互 ping，但仍返回 0x08。
                 * 为避免”业务侧探测条件不满足/瞬态/锁存”导致误报，这里增加流量门控：
                 * - Link=Up 且网口存在收发流量：认为通讯正常，不上报 COMM_ERR
                 * - 其余情况：按通讯错误处理
                 */
                if ((dtc->ethState != 1U) && (dtc->ethState != 2U) && (dtc->ethState != 3U))
                {
                    if ((linkUpValid == TRUE) && (linkUp == TRUE) && (trafficActive == TRUE))
                    {
                        /* 保持当前 ethState（优先反映线路/SMI 类错误，若无则保持 0） */
                    }
                    else
                    {
                        dtc->ethState = 4;
                    }
                }
            }
        }
#endif
        LOG_Debug("dtc->ethState=%d, PHY_IFNAME=%s, linkUpValid=%d, linkUp=%d, code79=0x%02X, detailBits=0x%02X, code08=0x%02X",
            dtc->ethState, PHY_IFNAME, linkUpValid, linkUp, (uint8_t)code79, detailBits, (uint8_t)code08);
    }

    uint8_t overTempFlag = 0;
    (void)GetMpuOverTemperatureStatus(&overTempFlag);
    if (overTempFlag != 0)
    {
        dtc->overTemperature = 1;
    }
    else
    {
        dtc->overTemperature = 0;
    }
    
    dtc->ddrState = g_ddrResult & 0x03;
    
    LOG_Debug("UpdateDtcTableData: storeState=%d, antMain=%d, antSec=%d, simFault=%d, lteFault=%d, ethState=%d, ddrState=%d, overTemperature=%d",
        dtc->storeState, dtc->antMain, dtc->antSec, dtc->simFault, dtc->lteFault, dtc->ethState, dtc->ddrState, dtc->overTemperature);
}

// 0xB201 模块工作状态（长度2B）
static void UpdateLteModuleStatusData(LteModuleStatus_t *lteModuleStatus)
{
    
    if (g_pmData.PmState == PM_STATE_NORMAL)
    {
        lteModuleStatus->powerMode = 5;
    }
    else if (g_pmData.PmState == PM_STATE_SLEEP)
    {
        lteModuleStatus->powerMode = 3;
    }
    else
    {
        lteModuleStatus->powerMode = 0;
    }
#if 0
    RpcAppAvntSessionStatus_e outStatus;
    if (GetAVNTSessionStatus(&outStatus, 2 * 1000) != 0 || outStatus != RPC_APP_AVNT_CONN_STATE_DISCONNECTED_E)
    {
        lteModuleStatus->dtrReady = 0;
        lteModuleStatus->rtsReady = 0;
    }
    else
    {
        lteModuleStatus->dtrReady = 1;
        lteModuleStatus->rtsReady = 1;
    }
    lteModuleStatus->dsrReady = 1;
    lteModuleStatus->ctsReady = 1;
    
#else
    
    lteModuleStatus->dtrReady = 0;
    lteModuleStatus->rtsReady = 0;
    lteModuleStatus->dsrReady = 1;
    lteModuleStatus->ctsReady = 1;
#endif

    LOG_Debug("UpdateLteModuleStatusData: powerMode=%d, dtrReady=%d, dsrReady=%d, rtsReady=%d, ctsReady=%d",
        lteModuleStatus->powerMode, lteModuleStatus->dtrReady, lteModuleStatus->dsrReady, lteModuleStatus->rtsReady, lteModuleStatus->ctsReady);
}

// 0xB202 GNSS 模块工作状态（长度2B）
static void GnssModuleStatusData(GnssModuleStatus_t *gnssModuleStatus)
{
    gnssModuleStatus->gnssEnableState = 0; // 独立定位使能
    gnssModuleStatus->modeMixed = 0;       // 定位模式-0：单模
    gnssModuleStatus->beidouEnable = 1;    // 北斗使能
    gnssModuleStatus->gpsEnable = 1;       // GPS使能
    gnssModuleStatus->glonassEnable = 0;   // GLONASS使能
    gnssModuleStatus->fusionFront = 0;     // 融合前置
    gnssModuleStatus->agpsEnable = 0;      // A-GPS使能
    gnssModuleStatus->protoCustom = 0;     // 输出协议-0：NMEA

    gnssModuleStatus->outComposite = 1;    // 定位信息输出-复合卫星信息
    gnssModuleStatus->rsv9 = 0;
    gnssModuleStatus->outGGA = (IsGgaValid(g_GGANmeaData.nmea, g_GGANmeaData.length) == TRUE) ? 1 : 0;
    gnssModuleStatus->outVTG = (IsVtgValid(g_VTGNmeaData.nmea, g_VTGNmeaData.length) == TRUE) ? 1 : 0;
    gnssModuleStatus->outGSA = (IsGsaValid(g_GSANmeaData.nmea, g_GSANmeaData.length) == TRUE) ? 1 : 0;
    gnssModuleStatus->outGSV = (IsGsvValid(g_GSVNmeaData.nmea, g_GSVNmeaData.length) == TRUE) ? 1 : 0;
    gnssModuleStatus->outRMC = (IsRmcValid(g_RMCNmeaData.nmea, g_RMCNmeaData.length) == TRUE) ? 1 : 0;
    gnssModuleStatus->outGLL = (IsGllValid(g_GLLNmeaData.nmea, g_GLLNmeaData.length) == TRUE) ? 1 : 0;
    
    LOG_Debug("GnssModuleStatusData: gnssEnable=%d, beidou=%d, gps=%d, glonass=%d, outGGA=%d, outVTG=%d, outGSA=%d, outGSV=%d, outRMC=%d, outGLL=%d",
        gnssModuleStatus->gnssEnableState, gnssModuleStatus->beidouEnable, gnssModuleStatus->gpsEnable, gnssModuleStatus->glonassEnable,
        gnssModuleStatus->outGGA, gnssModuleStatus->outVTG, gnssModuleStatus->outGSA, gnssModuleStatus->outGSV, gnssModuleStatus->outRMC, gnssModuleStatus->outGLL);
}

// 0xB203 当前时间（长度7B）
static void UpdateTimeData(TimeData_t *timeData)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts); // 秒+纳秒（Linux）
    time_t sec = ts.tv_sec;

    struct tm t;
    localtime_r(&sec, &t); // 本地时区

    timeData->year = (uint16_t)(t.tm_year + 1900);
    timeData->month = (uint8_t)(t.tm_mon + 1);
    timeData->day = (uint8_t)(t.tm_mday);
    timeData->hour = (uint8_t)(t.tm_hour);
    timeData->minute = (uint8_t)(t.tm_min);
    timeData->second = (uint8_t)(t.tm_sec);
    
    LOG_Debug("UpdateTimeData: year=%d, month=%d, day=%d, hour=%d, min=%d, sec=%d",
        timeData->year, timeData->month, timeData->day, timeData->hour, timeData->minute, timeData->second);
}

// 0xB210 4G模块网络状态（长度4B）
static void UpdateLteNetworkStatusData(LteNetworkStatus_t *lteNetworkStatus)
{
    int ratType = 0;

    lteNetworkStatus->csq = g_modemData.csq;
    lteNetworkStatus->ber = 0; // 误码率暂时拿不到
    uint8_t mccMnc[64+1] = {0};
    (void)ModemSrvGetOperator(mccMnc);
    mccMnc[64] = '\0';
    uint8_t op = DetectChinaOperatorFromMccMnc((const char *)mccMnc);
    lteNetworkStatus->b2_3.operatorType = op & 0x03;
    if (HalModemNwGetRadioTechType(&ratType) != 0)
    {
        lteNetworkStatus->b2_3.rat = 0;
    }
    else
    {
        switch (ratType)
        {
            case 2: /* GSM */
            {
                lteNetworkStatus->b2_3.rat = 0;
                break;
            }

            case 16: /* EDGE */
            {
                lteNetworkStatus->b2_3.rat = 1;
                break;
            }

            case 1: /* TD-SCDMA */
            {
                lteNetworkStatus->b2_3.rat = 4;
                break;
            }

            case 4: /* LTE */
            {
                lteNetworkStatus->b2_3.rat = 5;
                break;
            }

            default:
            {
                lteNetworkStatus->b2_3.rat = 0;
                break;
            }
        }
    }
    HalModemNwRegStatus_e status = 0;
    (void)HalModemNwGetRegStatus(&status);
    if (status == E_HAL_MODEM_NW_REG_STATUS_UNREGISTERED)
    {
        lteNetworkStatus->b2_3.netState = 2;
    }
    else
    {
        lteNetworkStatus->b2_3.netState = 1;
    }
    lteNetworkStatus->b2_3.faultCode = 0;
    
    LOG_Debug("UpdateLteNetworkStatusData: csq=%d, ber=%d, operator=%d, rat=%d, netState=%d, faultCode=%d",
        lteNetworkStatus->csq, lteNetworkStatus->ber, lteNetworkStatus->b2_3.operatorType,
        lteNetworkStatus->b2_3.rat, lteNetworkStatus->b2_3.netState, lteNetworkStatus->b2_3.faultCode);
}

// 0xB249 GPS卫星（1-4）信息（长度30B）
static void UpdateGpsSatInfoData(GpsSatInfo_1to4_t *gpsSatInfo)
{
    if (gpsSatInfo == NULL)
    {
        return;
    }

    (void)pthread_mutex_lock(&g_ftyDataMutex);
    // 解析GSV
    ParseGsvNmeaInfo(g_GSVNmeaData.nmea, gpsSatInfo);
    // 解析GSA
    ParseGsaNmeaInfo(g_GSANmeaData.nmea, gpsSatInfo);
    (void)pthread_mutex_unlock(&g_ftyDataMutex);
    
    LOG_Debug("UpdateGpsSatInfoData: visibleCount=%d, pdop=%02X%02X%02X, hdop=%02X%02X%02X, vdop=%02X%02X%02X",
        gpsSatInfo->visibleCount,
        gpsSatInfo->pdop[0], gpsSatInfo->pdop[1], gpsSatInfo->pdop[2],
        gpsSatInfo->hdop[0], gpsSatInfo->hdop[1], gpsSatInfo->hdop[2],
        gpsSatInfo->vdop[0], gpsSatInfo->vdop[1], gpsSatInfo->vdop[2]);
}

// 0xB24E 4G模块功能状态（长度2B）
static void UpdateLteFunctionStatusData(LteFunctionStatus_t *lteFunctionStatus)
{
    lteFunctionStatus->uartMainEn = 1;
    lteFunctionStatus->spiEn = 1;
    lteFunctionStatus->tcpEn = 1;
    char outBuf[32];
    int sc = RunCommandAndGetOutput("cat /data/usb/usb_adb_enable", outBuf, sizeof(outBuf));
    if (sc == 0 && strcmp(outBuf, "on") == 0)
    {
        lteFunctionStatus->usbModeEn = 1;
    }
    else
    {
        lteFunctionStatus->usbModeEn = 0;
    }
    lteFunctionStatus->netHeartbeatEn = 1;
    lteFunctionStatus->lteGpsEn = 1;
    lteFunctionStatus->i2sDigitalAuEn = 1;
    lteFunctionStatus->agpsEn = 1;

    lteFunctionStatus->cellLocateEn = 0;
    lteFunctionStatus->auxUartConsole = 0;
    lteFunctionStatus->usbModemEn = 0;
    lteFunctionStatus->usbDiagEn = 0;
    lteFunctionStatus->usbAuxEn = 0;
    lteFunctionStatus->uartDataEn = 1;
    lteFunctionStatus->uartTraceEn = 1;
    lteFunctionStatus->wlanEn = 0;
    
    LOG_Debug("UpdateLteFunctionStatusData: uartMain=%d, spi=%d, tcp=%d, usbMode=%d, netHeartbeat=%d, lteGps=%d, i2sAu=%d, agps=%d",
        lteFunctionStatus->uartMainEn, lteFunctionStatus->spiEn, lteFunctionStatus->tcpEn, lteFunctionStatus->usbModeEn,
        lteFunctionStatus->netHeartbeatEn, lteFunctionStatus->lteGpsEn, lteFunctionStatus->i2sDigitalAuEn, lteFunctionStatus->agpsEn);
}

// 0xB257 以太网线路故障信息（长度2B）
static void UpdateEthLineFaultInfoData(EthLineFaultInfo_t *ethLineFaultInfo)
{
    memset(ethLineFaultInfo, 0, sizeof(EthLineFaultInfo_t));

    /* 非以太网版本，直接返回全0 */
    if (IsEthernetVersion() == FALSE)
    {
        return;
    }

    Yt8010DiagStatus_t s = {0};
    uint8_t detailBits = 0;
    int code79 = 0;

    gboolean linkUp = FALSE;
    gboolean linkUpValid = GetEthLinkUp(PHY_IFNAME, &linkUp);
    static gboolean s_LastLinkValid = FALSE;
    static gboolean s_LastLinkUp = FALSE;
    gboolean trafficActive = FALSE;
    if ((linkUpValid == TRUE) && (linkUp == TRUE))
    {
        trafficActive = IsEthTrafficActive(PHY_IFNAME);
    }

    /*
     * 兼容处理：0x79 的 detailBits 中 bit1 表示线路开短/中断（与 B257 的 bit0 含义一致），
     * 同时 bit0/bit2 可用于补充初始化失败/SMI 失败信息。
     * 说明：detailBits 可能为瞬态或锁存位，仅作为“线路故障信息”展示，不直接等价于链路 Up/Down。
     */
    code79 = GetEthernetLineFailure(PHY_IFNAME, &detailBits);
    if (code79 == 0x79)
    {
        if ((detailBits & 0x02U) != 0U)
        {
            ethLineFaultInfo->lanShortOrBreak = 1U;
        }
        if ((detailBits & 0x01U) != 0U)
        {
            ethLineFaultInfo->pwonInitFail = 1U;
        }
        if ((detailBits & 0x04U) != 0U)
        {
            ethLineFaultInfo->smiCtrlFail = 1U;
        }
    }

    /*
     * 优先读取 YT8010 的诊断状态（更细颗粒度）。若读取成功，则以该结果覆盖/补全字段。
     */
    if (GetYt8010anDiagStatus(PHY_IFNAME, &s) == 0)
    {
        ethLineFaultInfo->snrWarn = s.snrWarn;
        ethLineFaultInfo->symErrInitFail = (s.symErrCnt != 0U) ? 1U : 0U;
        ethLineFaultInfo->pwonInitFail = s.initFail;
        /* linkUp2Down/linkDown2Up 可能为事件/锁存位，B257 按“边沿事件”由软件统一生成 */
        ethLineFaultInfo->smiCtrlFail = s.smiFail;
        ethLineFaultInfo->txEnableClamped = s.sendClamp;
        ethLineFaultInfo->pmCloseEth = s.ethPowerOff;
    }

    /* 由软件根据链路状态生成边沿事件，避免诊断/事件位乱跳 */
    if (linkUpValid == TRUE)
    {
        if ((s_LastLinkValid == TRUE) && (s_LastLinkUp == TRUE) && (linkUp == FALSE))
        {
            ethLineFaultInfo->linkOkToFail = 1U;
        }
        else
        {
            ethLineFaultInfo->linkOkToFail = 0U;
        }

        if ((s_LastLinkValid == TRUE) && (s_LastLinkUp == FALSE) && (linkUp == TRUE))
        {
            ethLineFaultInfo->linkFailToOk = 1U;
        }
        else
        {
            ethLineFaultInfo->linkFailToOk = 0U;
        }

        s_LastLinkValid = TRUE;
        s_LastLinkUp = linkUp;
    }
    else
    {
        ethLineFaultInfo->linkOkToFail = 0U;
        ethLineFaultInfo->linkFailToOk = 0U;
    }

    /*
     * Link=Up 且存在实际收发流量：认为“对外供网可用”，对 B257 中容易误导的锁存告警进行抑制，
     * 避免在供网成功时仍显示 init/smi/symErr 等故障。
     */
    if (trafficActive == TRUE)
    {
        ethLineFaultInfo->lanShortOrBreak = 0U;
        ethLineFaultInfo->pwonInitFail = 0U;
        ethLineFaultInfo->smiCtrlFail = 0U;
        ethLineFaultInfo->symErrInitFail = 0U;
    }

    /*
     * 与 DTC ethState 一致的口径：当 Link 已 Up 时，B257 的“线路开短/中断”不应长期保持 1。
     * 现场显示外部设备可正常通信时，该位多为诊断锁存/瞬态导致的误导。
     */
    if ((linkUpValid == TRUE) && (linkUp == TRUE))
    {
        ethLineFaultInfo->lanShortOrBreak = 0U;
    }

    LOG_Debug("EthLineFaultInfo(B257): lanShortOrBreak:%d, snrWarn:%d, symErrInitFail:%d, pwonInitFail:%d, linkOkToFail:%d, linkFailToOk:%d, smiCtrlFail:%d, txEnableClamped:%d, pmCloseEth:%d, linkUpValid=%d, linkUp=%d, code79=0x%02X, detailBits=0x%02X",
        ethLineFaultInfo->lanShortOrBreak,
        ethLineFaultInfo->snrWarn,
        ethLineFaultInfo->symErrInitFail,
        ethLineFaultInfo->pwonInitFail,
        ethLineFaultInfo->linkOkToFail,
        ethLineFaultInfo->linkFailToOk,
        ethLineFaultInfo->smiCtrlFail,
        ethLineFaultInfo->txEnableClamped,
        ethLineFaultInfo->pmCloseEth,
        linkUpValid,
        linkUp,
        (uint8_t)code79,
        detailBits);
}
#if 0
// 0xB2C5 敏感数据设置（长度16B）
static void UpdateSensitiveData(uint8_t *sensitivaData)
{
    if (!sensitivaData) return;

    RpcAppDtcDidReadReq_t didReq = {0};
    RpcAppDtcDidReadResp_t didResp = {0};
    didReq.did = DID_SENSITIVE_DATA_SET;
    if (RpcAppDtcReadDid((const RpcAppDtcDidReadReq_t *)&didReq, &didResp, 3000) != 0) {
        LOG_Error("RpcAppDtcReadDid failed");
        return;
    }
    if (didResp.nrc != 0) {
        LOG_Error("didResp.nrc not equal to 0,nrc:0x%x",didResp.nrc);
        return;
    }
    uint8_t value = 0U;
    if (didResp.value.len !=16U) {
        LOG_Error("RpcAppDtcReadDid success DID:0x%x, but len error:%d",DID_SENSITIVE_DATA_SET, didResp.value.len);
        return;
    }

    memcpy(sensitivaData, didResp.value.data, 16);
}
#endif
static void UpdateGpsPositionData(GpsPosition_t *gpsPosition)
{
    if (!gpsPosition) return;
    const GnssLocation_t *src = &g_gnssData;

    // 1) 方向标志：按你注释的 gpsState bit6/bit7
    uint8_t eastWest   = (src->gpsState >> 6) & 0x01; // 0东/1西
    uint8_t northSouth = (src->gpsState >> 7) & 0x01; // 0北/1南

    // 2) 经纬度：你的 GpsPosition_t 用同样的 1e-7 度无符号表示，直接拷贝即可
    gpsPosition->longitude  = src->longtitudeValue; // 注意原字段拼写 longtitude
    gpsPosition->latitude   = src->latitudeValue;
    gpsPosition->eastWest   = eastWest;
    gpsPosition->northSouth = northSouth;

    // 3) 海拔：0.1 m -> m 四舍五入
    //    最大65535 m，超出则截断
    uint32_t alt_m = (uint32_t)((src->seaHeight + 5) / 10); // 整数四舍五入
    gpsPosition->altitude  = clamp_u16(alt_m, 0xFFFF);

    // 4) 速度：0.1 km/h -> km/h（0~255）
    uint32_t spd_kmh = (uint32_t)((src->speed + 5) / 10);
    gpsPosition->speed       = clamp_u8(spd_kmh, 0xFF);

    // 5) 航向：0.1 度 -> 度，范围 0~359
    uint32_t hdg_deg = (uint32_t)((src->direction + 5) / 10);
    if (hdg_deg >= 360) hdg_deg %= 360;  // 有些设备可能给到3600(=360.0)
    gpsPosition->heading = (uint16_t)hdg_deg;

    // 6) 卫星与定位维度
    uint8_t used = src->moonLinkNum;
    if (used > 63) used = 63;      // 6bit 上限
    gpsPosition->satUsed  = used;
    gpsPosition->fixState = infer_fix_dim(used, src->gpsState);
    
    LOG_Debug("UpdateGpsPositionData: lon=%u, lat=%u, EW=%d, NS=%d, alt=%d, speed=%d, heading=%d, satUsed=%d, fixState=%d",
        gpsPosition->longitude, gpsPosition->latitude, gpsPosition->eastWest, gpsPosition->northSouth,
        gpsPosition->altitude, gpsPosition->speed, gpsPosition->heading, gpsPosition->satUsed, gpsPosition->fixState);
}

static int32_t SendFtyPeriodStatusInfo(char *status, size_t statusLen)
{
    McuMesg_t McuMsg = {0};

    if (NULL == status || 0 == statusLen)
    {
        return RET_OK;
    }

    McuMsg.payload = (uint8_t *)calloc(1, statusLen);
    if (NULL == McuMsg.payload)
    {
        return RET_ERR;
    }

    McuMsg.type = APP_RPC_QUE_TYPE_SYNC_FTY_PERIOD_STATUS_INFO_TO_MCU;
    McuMsg.payloadLen = statusLen;
    memcpy(McuMsg.payload, status, statusLen);
    if (RET_ERR == McuSrvSendMesg(&McuMsg))
    {
        LOG_Error(" %s:SendFtyInfoAck failed!", __func__);
        free(McuMsg.payload);
        return RET_ERR;
    }

    LOG_Debug(" %s:SendFtyInfoAck success!", __func__);
    free(McuMsg.payload);
    return RET_OK;
}

void FtyPmCallback(const uint8_t *data, const uint16_t len, void *userdata)
{
    do
    {
        if (sizeof(MPUPmStateChange_t) != len || NULL == data)
        {
            LOG_Debug("%s len error!", __FUNCTION__);
            break;
        }

        MPUPmStateChange_t *pmStatus = (MPUPmStateChange_t *)data;
        if (g_pmData.PmState == pmStatus->PmState)
        {
            LOG_Debug("%s:pm status not change!", __FUNCTION__);
            break;
        }

        LOG_Debug(" %s:pm status change!,from %d to %d", __FUNCTION__, g_pmData.PmState, pmStatus->PmState);
        (void)pthread_mutex_lock(&g_ftyDataMutex);
        memcpy(&g_pmData, data, len);
        (void)pthread_mutex_unlock(&g_ftyDataMutex);

        if (pmStatus->PmState == PM_STATE_NORMAL)
        {
            // 开内置声卡
            system("echo enable > /sys/kernel/debug/codec_stats");

            // 删除定时器
            // (void)PmDestroySleepAlarm(CPU_APP_TIMER_TBOX_RTC);
        }
        else if (pmStatus->PmState == PM_STATE_SLEEP)
        {
            // 关闭内置声卡
            system("echo disable > /sys/kernel/debug/codec_stats");

            // 创建定时唤醒，(唤醒整车待定，暂时只定时唤醒MPU)
            // ParamInfo_t param= {0};
            // param.id = (DeviceParamEnum)E_ParamId_RTC_Wakeup_Interval;
            // if (GetParam(&param) != 0)
            // {
            //     LOG_Error("%s:GetParam failed!", __FUNCTION__);
            //     break;
            // }

            // uint32_t sleepAlarmSecond = (uint32_t)atoi((char *)param.value);
            // LOG_Debug(" %s:SleepAlarmSecond:%d", __FUNCTION__, sleepAlarmSecond);
            // if (sleepAlarmSecond > 0)
            // {
            //     sleepAlarmSecond = (sleepAlarmSecond * 4.1);
            //     LOG_Debug(" %s:SleepAlarmSecond:%d", __FUNCTION__, sleepAlarmSecond);
            //     (void)PmDestroySleepAlarm(CPU_APP_TIMER_TBOX_RTC);
            //     (void)PmCreateSleepAlarm(CPU_APP_TIMER_TBOX_RTC, sleepAlarmSecond, CPU_APP_TIMER_TBOX_RTC);
            // }
        }
    } while (0);
}

void FtyModemCallback(const uint8_t *data, const uint16_t len, void *userdata)
{
    if (sizeof(ModemSrvNotify_t) == len && NULL != data)
    {
        memcpy(&g_modemData, data, len);
    }
    else
    {
        LOG_Error("%s recv modem data leng is wrong: %d .", __FUNCTION__, len);
    }
}

void FtyGnssMsgCallback(const uint8_t *data, const uint16_t len, void *userdata)
{
    if(sizeof(GnssLocation_t) == len && NULL != data)
    {
        memcpy(&g_gnssData,data,len);
    }
    else
    {
        LOG_Error("%s recv gnss data leng is wrong: %d .",__FUNCTION__,len);
    }
}

void FtyGnssNmeaCallback(const uint8_t *data, const uint16_t len, void *userdata)
{
    GnssNmea_t *gnssNmea = (GnssNmea_t *)data;

    if(sizeof(GnssNmea_t) == len && NULL != data)
    {
        (void)pthread_mutex_lock(&g_ftyDataMutex);
        if (gnssNmea->type == GNSS_NMEA_TYPE_GSA)
        {
            memcpy(&g_GSANmeaData, data, len);
        }
        if (gnssNmea->type == GNSS_NMEA_TYPE_GSV)
        {
            memcpy(&g_GSVNmeaData, data, len);
        }
        if (gnssNmea->type == GNSS_NMEA_TYPE_GPRMC)
        {
            memcpy(&g_RMCNmeaData, data, len);
        }
        if (gnssNmea->type == GNSS_NMEA_TYPE_GPGGA)
        {
            memcpy(&g_GGANmeaData, data, len);
        }
        if (gnssNmea->type == GNSS_NMEA_TYPE_GLL)
        {
            memcpy(&g_GLLNmeaData, data, len);
        }
        if (gnssNmea->type == GNSS_NMEA_TYPE_VTG)
        {
            memcpy(&g_VTGNmeaData, data, len);
        }
        (void)pthread_mutex_unlock(&g_ftyDataMutex);
    }
    else
    {
        LOG_Error("%s recv gnss data leng is wrong: %d .",__FUNCTION__,len);
    }
}

/**
 * num(1byte) +
 * did1(2byte) + len1(2byte) + data1(nbyte) +
 * did2(2byte) + len2(2byte) + data2(nbyte) +
 * ...
 * didn(2byte) + lenn(2byte) + datan(nbyte)
 * */
void FtySyncVehicleEolDataAckCallback(const uint8_t *data, const uint16_t len, void *userdata)
{
    if (data == NULL || len < 1)
    {
        LOG_Error("%s: Invalid data", __FUNCTION__);
        return;
    }

    uint16_t offset = 0;
    uint8_t num = data[offset++];  // 读取DID数量

    LOG_Info("%s: num=%d, len=%d", __FUNCTION__, num, len);

    for (uint8_t i = 0; i < num; i++)
    {
        // 检查是否有足够数据读取DID和长度
        if (offset + 4 > len)
        {
            LOG_Error("%s: Data too short at DID%d", __FUNCTION__, i + 1);
            return;
        }

        // 读取DID (2字节，大端序)
        uint16_t did = (data[offset] << 8) | data[offset + 1];
        offset += 2;

        // 读取数据长度 (2字节，大端序)
        uint16_t dataLen = (data[offset] << 8) | data[offset + 1];
        offset += 2;

        // 检查是否有足够数据读取payload
        if (offset + dataLen > len)
        {
            LOG_Error("%s: Data too short for DID 0x%04X", __FUNCTION__, did);
            return;
        }

        // 拷贝数据到本地缓冲区
        uint8_t payload[512] = {0};
        if (dataLen > sizeof(payload))
        {
            LOG_Error("%s: Data length %d exceeds buffer size", __FUNCTION__, dataLen);
            return;
        }
        memcpy(payload, &data[offset], dataLen);
        offset += dataLen;

        LOG_Info("%s: DID=0x%04X, len=%d", __FUNCTION__, did, dataLen);

        // 更新DID数据到SOC
        if (UdsDidWrite(did, payload, dataLen) != 0)
        {
            LOG_Error("%s: Failed to write DID 0x%04X", __FUNCTION__, did);
            continue;
        }

        // 标记该DID已同步
        for (uint8_t j = 0; j < EOL_DID_COUNT; j++)
        {
            if (g_eolDidList[j] == did)
            {
                g_eolDidSyncStatus[j] = true;

                // 统计已同步的DID数量
                uint8_t syncedCount = 0;
                for (uint8_t k = 0; k < EOL_DID_COUNT; k++) {
                    if (g_eolDidSyncStatus[k]) {
                        syncedCount++;
                    }
                }
                LOG_Info("%s: DID[%d] 0x%04X marked as synced [%d/%d]",
                         __FUNCTION__, j + 1, did, syncedCount, EOL_DID_COUNT);
                break;
            }
        }
    }

    LOG_Info("%s: Parsed %d DIDs successfully", __FUNCTION__, num);

    // 通知SyncVehicleEolDataFromMcu继续发送请求
    pthread_mutex_lock(&g_eolSyncMutex);
    g_eolSyncResponseReceived = true;
    pthread_cond_signal(&g_eolSyncCond);
    pthread_mutex_unlock(&g_eolSyncMutex);
    LOG_Debug("%s: Response processed, notified sync thread", __FUNCTION__);
}

/*num(1byte) + did(2byte*num)*/
void FtyMcuReqVehicleEolDataCallback(const uint8_t *data, const uint16_t len, void *userdata)
{
    // 参数检查
    if (!data || len < 1)
    {
        LOG_Error("%s: Invalid parameters, data=%p, len=%u", __FUNCTION__, data, len);
        return;
    }

    // 解析请求数据：num(1byte) + did(2byte*num)
    uint8_t didCount = data[0];
    uint16_t expectedLen = 1 + (didCount * 2);

    if (len < expectedLen)
    {
        LOG_Error("%s: Data length mismatch, expected=%u, actual=%u", __FUNCTION__, expectedLen, len);
        return;
    }

    LOG_Info("%s: Received request for %u DIDs", __FUNCTION__, didCount);

    // 分配响应缓冲区（最大：1字节num + N个DID*(2字节DID + 2字节len + 最大数据长度)）
    // 假设每个DID最大数据长度为256字节
    uint8_t *respBuf = (uint8_t *)malloc(1 + didCount * (2 + 2 + 256));
    if (!respBuf)
    {
        LOG_Error("%s: Failed to allocate response buffer", __FUNCTION__);
        return;
    }

    uint16_t respOffset = 0;
    uint8_t successCount = 0;

    // 第一个字节预留给成功读取的DID数量
    respOffset = 1;

    // 遍历每个DID，读取数据并组包
    for (uint8_t i = 0; i < didCount; i++)
    {
        // 提取DID（大端序）
        uint16_t did = (data[1 + i * 2] << 8) | data[1 + i * 2 + 1];

        // 准备读取缓冲区
        uint8_t didData[256] = {0};
        uint16_t didDataLen = sizeof(didData);

        // 调用UdsDidRead读取DID数据
        UdsNrc_e nrc = UdsDidRead((UDS_DID_IDS_e)did, didData, &didDataLen);
        if (nrc == NRC_NONE_E && didDataLen > 0)
        {
            // 读取成功，组包：did(2byte) + len(2byte) + data(nbyte)
            respBuf[respOffset++] = (uint8_t)(did >> 8);    // DID高字节
            respBuf[respOffset++] = (uint8_t)(did & 0xFF);  // DID低字节
            respBuf[respOffset++] = (uint8_t)(didDataLen >> 8);    // 长度高字节
            respBuf[respOffset++] = (uint8_t)(didDataLen & 0xFF);  // 长度低字节

            // 拷贝数据
            memcpy(&respBuf[respOffset], didData, didDataLen);
            respOffset += didDataLen;

            successCount++;
            LOG_Info("%s: DID 0x%04X read success, len=%u", __FUNCTION__, did, didDataLen);
        }
        else
        {
            LOG_Warn("%s: DID 0x%04X read failed, nrc=%u", __FUNCTION__, did, nrc);
        }
    }

    // 设置第一个字节为成功读取的DID数量
    respBuf[0] = successCount;

    LOG_Info("%s: Total %u/%u DIDs read successfully, response length=%u",
             __FUNCTION__, successCount, didCount, respOffset);

    // 发送响应到MCU
    McuMesg_t sendToMcu = {0};
    sendToMcu.type = APP_RPC_QUE_TYPE_SYNC_DTC_MCU_FROM_MPU_ACK;
    sendToMcu.payloadLen = respOffset;
    sendToMcu.payload = respBuf;

    if (RET_OK != McuSrvSendMesg(&sendToMcu))
    {
        LOG_Error("%s: Failed to send response to MCU", __FUNCTION__);
    }
    else
    {
        LOG_Info("%s: Response sent to MCU successfully", __FUNCTION__);
    }

    // 释放缓冲区
    free(respBuf);
}

int DdrCheckAndUpdate(void)
{
    /* 1) 确保目录与文件存在；文件以结构体二进制保存 */
    if (EnsureDirectoryExists(DDR_DIR_PATH) != 0)
    {
        LOG_Debug("EnsureDirectoryExists failed");
        return -1;
    }

    DdrInfo_t info;
    int lr = LoadDdrInfoFromFile(DDR_FILE_PATH, &info);
    if (lr != 0)
    {
        /* 文件不存在或损坏：按默认初始值创建 */
        info.ddrSet = 0U;
        info.ddrRead = 0U;
        info.ddrResult = 0U; /* 默认先认为正常，也可改为 1 保守 */
        if (SaveDdrInfoToFile(DDR_FILE_PATH, &info) != 0)
        {
            LOG_Debug("Initialize file failed");
            return -2;
        }
        LOG_Debug("DDR info file initialized.");
    }

    /* 2) 未设置则执行设置命令并置位，仅置位不读取 */
    if (info.ddrSet == 0U)
    {
        LOG_Debug("DDR not set yet. Running: %s", CMD_DDR_SET);
        int sc = system(CMD_DDR_SET);
        if (sc == -1)
        {
            LOG_Debug("system() failed for set cmd: %s", strerror(errno));
            return -3;
        }
        else
        {
            /* 仅记录日志，不强依赖退出码（串口工具可能不规范返回值） */
            LOG_Debug("Set command invoked, exit code=%d", sc);
        }

        info.ddrSet = 1U;
        if (SaveDdrInfoToFile(DDR_FILE_PATH, &info) != 0)
        {
            LOG_Debug("Save after set flag failed");
            return -4;
        }

        /* 按需求第2条，设置完成即返回（不继续读取） */
        return 0;
    }

    /* 3) 已设置但未读取，则执行读取命令，解析结果，保存并同步 RAM */
    if (info.ddrRead == 0U)
    {
        LOG_Debug("DDR set already, not read yet. Running: %s", CMD_DDR_READ);

        char outBuf[1024];
        int rc = RunCommandAndGetOutput(CMD_DDR_READ, outBuf, sizeof(outBuf));
        if (rc != 0)
        {
            LOG_Debug("Run read cmd failed");
            /* 失败时不修改文件，仅返回错误码 */
            return -5;
        }

        uint8_t parsed = 1U; /* 保守先置故障 */
        int pr = ParseDdrResultFromOutput(outBuf, &parsed);
        if (pr != 0)
        {
            LOG_Debug("Parse read output failed, fallback to fault=1");
            parsed = 1U;
        }

        info.ddrResult = parsed;
        info.ddrRead = 1U;

        if (SaveDdrInfoToFile(DDR_FILE_PATH, &info) != 0)
        {
            LOG_Debug("Save after read failed");
            return -6;
        }

        UpdateDdrResultRam(info.ddrResult);
        return 0;
    }

    /* 4) 已读取过：直接把文件里的结果同步到 RAM */
    LOG_Debug("DDR already read before. Sync persisted result to RAM.");
    UpdateDdrResultRam(info.ddrResult);

    return 0;
}

int RemoteDdrTestFile(void)
{
    if (g_remove(DDR_FILE_PATH) != 0)
    {
        if (errno == ENOENT)
        {
            LOG_Debug("DDR file already not exists: %s", DDR_FILE_PATH);
            return 0;  // 文件不存在视为成功
        }
        LOG_Warn("Failed to remove DDR file: %s, errno=%d", DDR_FILE_PATH, errno);
        return -1;
    }

    LOG_Debug("DDR file removed: %s", DDR_FILE_PATH);
    return 0;
}

void GetPmPowerState(MPUPmStateChange_t *pmData)
{
    (void)pthread_mutex_lock(&g_ftyDataMutex);
    memcpy(pmData, &g_pmData, sizeof(MPUPmStateChange_t));
    (void)pthread_mutex_unlock(&g_ftyDataMutex);
}

gpointer SendInfoToMCU(gpointer pArg)
{
    static uint8_t count = 0;

    (void)HalModemNwInit();

    while (1)
    {
        sleep(1);

        // 1、dtc table（长度2B）
        // UpdateDtcTableData(&(g_ftyCircleData.dtc));

        // 2、0xB201 模块工作状态（长度2B）
        UpdateLteModuleStatusData(&(g_ftyCircleData.lteModuleStatus));

        // 3、0xB202 GNSS 模块工作状态（长度2B）
        GnssModuleStatusData(&(g_ftyCircleData.gnssModuleStatus));

        // 4、0xB203 时间数据（长度7B）
        UpdateTimeData(&(g_ftyCircleData.timeData));

        // 5、0xB210 4G模块网络状态（长度4B）
        UpdateLteNetworkStatusData(&(g_ftyCircleData.lteNetworkStatus));

        // 6、0xB249 GPS卫星（1-4）信息（长度30B）
        UpdateGpsSatInfoData(&(g_ftyCircleData.gpsSatInfo1to4));

        // 7、0xB24E 4G模块功能状态（长度2B）
        UpdateLteFunctionStatusData(&(g_ftyCircleData.lteFunctionStatus));

        if (count == 0)
        {
            // 1、dtc table（长度2B）
            UpdateDtcTableData(&(g_ftyCircleData.dtc));
            // 8、0xB257 以太网线路故障信息（长度2B）
            UpdateEthLineFaultInfoData(&(g_ftyCircleData.ethLineFaultInfo));
        }

        // 9、0xB290 GPS经纬度/搜星数（长度16B）
        UpdateGpsPositionData(&(g_ftyCircleData.gpsPosition));

        // 10、0xB2C5 敏感数据（长度16B）
        // UpdateSensitiveData(g_ftyCircleData.sensitiveData);

        // 11、发送数据到MCU
        SendFtyPeriodStatusInfo((char *)&g_ftyCircleData, sizeof(ftyCircleDataToMcu_t));

        count++;
        if (count >=5)
        {
            count = 0;
        }

    }
}

/*num(1byte) + did(2byte*num)*/
gpointer SyncVehicleEolDataFromMcu(gpointer pArg)
{
    //统计来自MCU的参数数量
    uint16_t mcuParaNum = 0;
    uint8_t reqData[(EOL_DID_COUNT * 2) + 8] = {0};
    
    LOG_Debug("SyncVehicleEolDataFromMcu");

    while (1)
    {
        //当前已经从MCU同步的参数数量
        uint16_t curSyncNum = 0;
        uint16_t reqParaLen = 1;
        memset(reqData, 0x00, sizeof(reqData));
        // 组包，将未完成同步的did发送到MCU
        // 格式 num(1byte) + did1(2byte) + did2(2byte) + ... + didn(2byte)

        // 遍历所有DID，找出未同步的DID并打包
        uint8_t unsyncedCount = 0;
        for (uint8_t i = 0; i < EOL_DID_COUNT; i++)
        {
            if (!g_eolDidSyncStatus[i])
            {
                // 将DID以大端序写入缓冲区
                uint16_t did = g_eolDidList[i];
                reqData[reqParaLen++] = (uint8_t)(did >> 8);    // 高字节
                reqData[reqParaLen++] = (uint8_t)(did & 0xFF);  // 低字节
                unsyncedCount++;
                LOG_Debug("%s: Adding unsynced DID 0x%04X to request", __FUNCTION__, did);
            }
            else
            {
                curSyncNum++;
            }
        }

        // 设置第一个字节为未同步DID的数量
        reqData[0] = unsyncedCount;

        // 如果所有DID都已同步，退出循环
        if (unsyncedCount == 0)
        {
            LOG_Info("%s: All EOL DIDs synchronized [%d/%d], exiting sync thread",
                     __FUNCTION__, curSyncNum, EOL_DID_COUNT);
            break;
        }

        LOG_Info("%s: Requesting %d unsynced DIDs from MCU [synced: %d/%d]",
                 __FUNCTION__, unsyncedCount, curSyncNum, EOL_DID_COUNT);

        {
            // 重置响应标志
            pthread_mutex_lock(&g_eolSyncMutex);
            g_eolSyncResponseReceived = false;
            pthread_mutex_unlock(&g_eolSyncMutex);

            // 向MCU获取参数
            McuMesg_t sendToMcu;
            sendToMcu.type = APP_RPC_QUE_TYPE_SYNC_DTC_MPU_FROM_MCU;
            sendToMcu.payloadLen = reqParaLen;
            sendToMcu.payload = reqData;
            McuSrvSendMesg(&sendToMcu);

            // 等待响应，超时3秒
            struct timespec timeout;
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += 3;  // 3秒超时

            pthread_mutex_lock(&g_eolSyncMutex);
            int ret = 0;
            while (!g_eolSyncResponseReceived && ret == 0) {
                ret = pthread_cond_timedwait(&g_eolSyncCond, &g_eolSyncMutex, &timeout);
            }

            if (ret == ETIMEDOUT) {
                LOG_Warn("%s: Response timeout (3s), will retry", __FUNCTION__);
            } else if (g_eolSyncResponseReceived) {
                LOG_Debug("%s: Response received, continue next request", __FUNCTION__);
            }
            pthread_mutex_unlock(&g_eolSyncMutex);
        }

        sleep(1);
    }

    return NULL;
}