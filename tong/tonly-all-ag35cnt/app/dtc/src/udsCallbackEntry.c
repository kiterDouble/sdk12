/**
 * @file udsCallBackEntry.c
 * @brief 处理uds需要通过回调处理的相关did
 * @author mia.zhong
 * @date 2025/09/11
 *
 * @copyright Copyright (c) 2025 SiRun (BeiJing).
 */
#include <glib.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <net/if.h>
#include "queue_msg.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "sr_param.h"
#include "pm_server_api.h"

// #include "tspRpcApi.h"
#include "canSignal.h"
#include "didRegistry.h"
#include "udsCallbackEntry.h"
#include "appRpcType.h"
#include "avntRpcApi.h"
#include "udsTypes.h"
#include "dtcRpcApi.h"
#include "dbDriver.h"
#include "fty.h"
// #include "gacPki.h"
#include "mdio.h"
// #include "logRecall.h"
// #include "logRecallGmClient.h"
#include "modem_ipc_msg.h"
#include "gnss_ipc_msg.h"

typedef struct
{
    uint8_t kl15status;
    uint32_t kl30voltage;
    uint16_t fastCharge;
    uint8_t OBD;   // 0:诊断仪未接入  1:诊断仪已接入
 }HardLineSyncInfo_t;

typedef struct
{
    MPUPmStateChange_t pmInfo;
    HardLineSyncInfo_t hardLineData;
    GacT08y1UdsSignal_canVal_t diagSignalData;
} UdsSubMsgInfo_t;

static pthread_mutex_t g_canSignalMutex = PTHREAD_MUTEX_INITIALIZER; // CAN数据保护互斥锁
static UdsSubMsgInfo_t g_subData = {0};
static uint8_t g_otaStatus = 0;

static UdsDidToParamIdMapping_t g_udsParamMapping[] = {
#if 1 //SQ 按陕汽需求添加DID
    {DID_VIN, E_ParamId_VIN},
    {DID_ICCID, E_ParamId_ICCID},
    {DID_IMSI, E_ParamId_IMSI},
    {DID_IMEI, E_ParamId_IMEI},
    {DID_APN1, E_ParamId_APN1},
    {DID_APN2, E_ParamId_APN2},
    {DID_APN3, E_ParamId_APN3},
    {DID_FAST_SLEEP_MODE, E_ParamId_Fast_Sleep_Mode},
    {DID_SW_VERSION_INTERNAL, E_ParamId_SW_Version},
    {DID_SW_VERSION_EXTERNAL, E_ParamId_SW_Version_External},
    {DID_HW_VERSION, E_ParamId_HW_Version},
    {DID_TBOX_SN, E_ParamId_SN},
    {DID_ICCID0, E_ParamId_enumTonlyBeWinnerICCID0},
    {DID_ICCID1, E_ParamId_enumTonlyBeWinnerICCID1},
    {DID_ICCID2, E_ParamId_enumTonlyBeWinnerICCID2},
#else
    {DID_TRANSPORT_MODE, E_ParamId_Transport_mode},
    {DID_MANUFACTORY_MODE, E_ParamId_Manufactory_mode},
    {DID_VIN, E_ParamId_VIN},
    {DID_SUPPLIER_HW_VER, E_ParamId_HW_Version},
    {DID_SUPPLIER_SW_VER, E_ParamId_SW_Version},
    {DID_TSP_DOMAIN_NAME, E_ParamId_UserPrivateTspDomain},
    {DID_IMEI, E_ParamId_IMEI},
    {DID_IMSI, E_ParamId_IMSI},
    {DID_ICCID, E_ParamId_ICCID},
    {DID_RTC_WAKEUP_INTERVAL, E_ParamId_RTC_Wakeup_Interval},
    {DID_PORT1, E_ParamId_UserPrivateTspPort},
    {DID_ECALL_DIAL1, E_ParamId_ECallNumber},
    {DID_BCALL_DIAL1, E_ParamId_BCallNumber},
    {DID_MQTT_PORT_PUBLIC, E_ParamId_UserPublicTspPort},
    {DID_TSP_CONN_TIMEOUT, E_ParamId_TSP_Conn_Timeout},
    {DID_TSP_TIMEOUT_FOR_FLOWCHART, E_ParamId_TSP_Timeout_For_Flowchart},
    {DID_TSP_ACK_TIME, E_ParamId_TSP_Ack_Time},
    // {DID_LONG_CONNECTION_CYCLE, E_ParamId_Long_Connection_Cycle},
    {DID_ETHERNET_CONFIG_INFO, E_ParamId_Ethernet_Config_Info},
    {DID_PUBLIC_TSP_DOMAIN_NAME, E_ParamId_UserPublicTspDomain},
    {DID_TSP_PLATFORM_CONN_SETTING, E_ParamId_TSP_Platform_Conn_Setting},
    {DID_TBOX_RUNNING_MODE, E_ParamId_Tbox_Running_Mode},
    {DID_4G_HW_RESET_TIME, E_ParamId_4G_HW_Reset_Time},
    {DID_PKI_DOMAIN_NAME, E_ParamId_UserPkiDomain},
    {DID_PKI_PORT, E_ParamId_UserPkiPort},
    {DID_PKI_MAINTENANCE_CYCLE, E_ParamId_UserPkiCycle},
    {DID_LOG_DOMAIN_NAME, E_ParamId_UserLogUploadDomain},
    {DID_LOG_PORT, E_ParamId_UserLogUploadPort},
    {DID_LOG_ACCOUNT, E_ParamId_UserLogUploadAccount},
    {DID_LOG_PASSWORD, E_ParamId_UserLogUploadPassword},
    {DID_LOG_INTERFACE, E_ParamId_UserLogUploadInterface},
    {DID_LOG_PATH, E_ParamId_UserLogUploadPath},
    {DID_SENSITIVE_DATA_SET, E_ParamId_Sensitive_Data_Set},
    {DID_OTA_RTC_WAKEUP_PARAM, E_ParamId_OTA_RTC_Wakeup_Param},
    {DID_BURYING_POINT_SWITCH, E_ParamId_Burying_Point_Switch},
    {DID_CONN_TEST_ADDR, E_ParamId_Conn_Test_Addr},
    {DID_BURIED_POINT_DOMAIN, E_ParamId_UserTelemetryDataDomain},
    {DID_GAC_ECU_SERIAL_NUM, E_ParamId_SN},
    {DID_GAC_SPARE_PART_NUM, E_ParamId_ParatNumber},
    {DID_GAC_KIT_ASSEMBLY_PART_NUM, E_ParamId_ParatNumber},
    {DID_GAC_ECU_HW_VER, E_ParamId_HW_Version},
    {DID_GAC_ECU_SW_VER, E_ParamId_CustomSW_Version},
    {DID_SYSTEM_SUPPLIER_ID, E_ParamId_SupIdentifier},
    {DID_4G_HW_RESET_COUNT, E_ParamId_4G_HW_Reset_Count}
#endif
};
#define UDS_DID_MAPPING_TABLE_COUNT ((uint32_t)(sizeof(g_udsParamMapping) / sizeof(g_udsParamMapping[0])))

typedef UdsNrc_e (*udsWriteToFileCb_t)(UDS_DID_IDS_e did, uint8_t *buf, uint16_t len);
typedef UdsNrc_e (*udsReadFromFileCb_t)(UDS_DID_IDS_e did, uint8_t *buf, uint16_t *len);
typedef struct
{
    UDS_DID_IDS_e did;
    udsWriteToFileCb_t writeCb;
    udsReadFromFileCb_t readCb;
} UdsFileHandle_t;

static UdsNrc_e UdsNetworkFileWriteCb(UDS_DID_IDS_e did, uint8_t *buf, uint16_t len);
static UdsNrc_e UdsNetworkFileReadCb(UDS_DID_IDS_e did, uint8_t *buf, uint16_t *len);

static UdsFileHandle_t g_udsFileMapping[] = {
    {DID_APN1, UdsNetworkFileWriteCb, UdsNetworkFileReadCb},
    {DID_APN2, UdsNetworkFileWriteCb, UdsNetworkFileReadCb},
    {DID_APN3, UdsNetworkFileWriteCb, UdsNetworkFileReadCb}
};
#define UDS_FILE_HANDLE_TABLE_COUNT ((uint32_t)(sizeof(g_udsFileMapping) / sizeof(g_udsFileMapping[0])))

/* 选择第 4 个参数（如果存在） */
#define WriteToParam_Select(_1, _2, _3, _4, NAME, ...)  NAME

/* 对外同名 API：3参 / 4参都叫 WriteToParam */
#define WriteToParam(...) \
    WriteToParam_Select(__VA_ARGS__, WriteToParam4, WriteToParam3)(__VA_ARGS__)

static void HardLineSyncCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
    (void)pthread_mutex_lock(&g_canSignalMutex);
    g_subData.hardLineData.kl15status = data[0];
    g_subData.hardLineData.kl30voltage = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
    g_subData.hardLineData.fastCharge = (data[5] << 8) | data[6];
    g_subData.hardLineData.OBD = data[7];
    (void)pthread_mutex_unlock(&g_canSignalMutex);
    LOG_Debug("%s called, kl15status:%d, kl30voltage:%u mv.", __FUNCTION__, g_subData.hardLineData.kl15status, g_subData.hardLineData.kl30voltage);
}
#if 0
static void DiagDataCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    if (recvData == NULL)
    {
        LOG_Error("recvData is nullptr");
        return;
    }

    if (dataLen != sizeof(GacT08y1UdsSignal_canVal_t))
    {
        LOG_Error("Received REAL data length[%d] is not equal with %d", dataLen,
                  sizeof(GacT08y1UdsSignal_canVal_t));
        return;
    }

    (void)pthread_mutex_lock(&g_canSignalMutex);
    memcpy(&g_subData.diagSignalData, recvData, sizeof(GacT08y1UdsSignal_canVal_t));
    (void)pthread_mutex_unlock(&g_canSignalMutex);
}

static void OtaStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    // 基础参数检查
    if (!recvData) {
        LOG_Error("Invalid input! recvData:%p", recvData);
        return;
    }  
    if (dataLen != sizeof(uint8_t))
    {
        LOG_Error("Received tbox ota mode data length[%d] is not equal with %d", dataLen,sizeof(uint8_t));
        return;
    }

    g_otaStatus = *recvData;
}

/* 去掉首尾空白（空格/制表符/回车换行） */
static void strtrim(char *s)
{
    char *p = s, *q;
    while (*p && isspace((unsigned char)*p))
        p++; // 去前缀空白
    if (p != s)
        memmove(s, p, strlen(p) + 1);
    size_t n = strlen(s);
    while (n && isspace((unsigned char)s[n - 1]))
        s[--n] = '\0'; // 去后缀空白
}
#endif
static inline void PutU16BE(uint8_t *out, uint16_t v)
{
    out[0] = (uint8_t)(v >> 8);
    out[1] = (uint8_t)(v);
}

static inline void PutU32BE(uint8_t *out, uint32_t v)
{
    out[0] = (uint8_t)(v >> 24);
    out[1] = (uint8_t)(v >> 16);
    out[2] = (uint8_t)(v >> 8);
    out[3] = (uint8_t)(v);
}

/* ====================== F1A1/F1A2 Stored Data 工具函数 ====================== */

/* 校验 7 字节时间戳与来源 bitfield 的基本合法性（不校验日期大小月/闰年，仅做范围兜底） */
#if 0
static bool CheckLocalDiagStoredDataFormat(const uint8_t *buf, uint16_t len)
{
    if ((buf == NULL) || (len != 7U))
    {
        return false;
    }

    /* year = 2000 + buf[0]，buf[0] 天然 0..255 => 2000..2255，无需额外校验 */
    uint8_t mon    = buf[1];
    uint8_t day    = buf[2];
    uint8_t hour   = buf[3];
    uint8_t minute = buf[4];
    uint8_t sec    = buf[5];

    if ((mon < 1U) || (mon > 12U) ||
        (day < 1U) || (day > 31U) ||
        (hour > 23U) ||
        (minute > 59U) ||
        (sec > 59U))
    {
        return false;
    }

    /* buf[6] 为 bitfield：bit0..2 有定义，bit3..7 reserved；默认不强制清 0，兼容工具 */
    return true;
}

/* 从参数服务读取一个“十进制整形”的子参数，解析后以大端写入 buf（len=1/2/4 之一） */
static bool ReadOneIntParamToBE(DeviceParamEnum id, uint8_t *buf, uint16_t bytes)
{
    ParamInfo_t p = {0};
    p.id = id;
    if (GetParam(&p) != 0) return false;

    uint32_t v = atoi((char *)p.value);
    // if (!ParseUint32FromAscii(p.value, sizeof(p.value), 0U, 0xFFFFFFFFU, &v)) return false;
    if (bytes == 1) {
        buf[0] = (uint8_t)(v & 0xFFu);
    } else if (bytes == 2) {
        PutU16BE(buf, (uint16_t)(v & 0xFFFFu));
    } else if (bytes == 3)
    {
        // 暂不支持
    } else if (bytes == 4) {
        PutU32BE(buf, v);
    } else {
        return false;
    }
    return true;
}
#endif
static bool ParseUintFromBigEndian(const uint8_t *buf, uint16_t len, uint32_t *outVal)
{
    if (buf == NULL || outVal == NULL) return false;
    if (len == 1)
    {
        *outVal = (uint32_t)buf[0];
        return true;
    }
    else if (len == 2)
    {
        *outVal = ((uint32_t)buf[0] << 8) | (uint32_t)buf[1];
        return true;
    }
    else if (len == 4)
    {
        *outVal = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
        return true;
    }
    else
    {
        return false; /* 不支持的字节数 */
    }
}

/* 工具：把字符串写入 param.value（截断到容量，且以 '\0' 结尾） */
static void CopyStringIntoParamValue(uint8_t dst[], size_t dstSize, const uint8_t *src, uint16_t srcLen)
{
    if (dst == NULL || dstSize == 0) return;
    size_t n = (srcLen < dstSize - 1) ? srcLen : (dstSize - 1);
    memset(dst, 0, dstSize);
    if (src != NULL && n > 0)
    {
        memcpy(dst, src, n);
        dst[n] = '\0';
    }
}

/* 通用：按 DID 写入参数服务（整形按十进制字符串写，字符型按原样写） */
static int WriteToParam3(uint32_t id, const uint8_t *buf, uint16_t len)
{
    int ret = 0;
    ParamInfo_t param = {0};
    
    LOG_DebugHex(buf, len, "WriteToParam");

    do
    {
        if (buf == NULL)
        {
            ret = -1;
            break;
        }

        param.id = id;

        /* 调 GetParam 获取类型、范围等元信息（注意：param.value 里是“当前值”，下面会覆盖） */
        if (GetParam(&param) != 0)
        {
            LOG_Debug("GetParam failed for id=%d", param.id);
            ret = -1;
            break;
        }

        /* 分类型写入 */
        if (param.type == 1)
        {
            /* 整形：buf 为大端编码的数值（1/2/4 字节），写入 param.value 的十进制字符串 */
            uint32_t v = 0;
            if (!ParseUintFromBigEndian(buf, len, &v))
            {
                LOG_Debug("invalid integer length=%u (only 1/2/4 supported)", (unsigned)len);
                ret = -1;
                break;
            }

            /* 范围校验（min/max 来自 ParamInfo_t） */
            if (v < (uint32_t)param.min || v > (uint32_t)param.max)
            {
                LOG_Debug("value %u out of range [%d,%d]", (unsigned)v, param.min, param.max);
                ret = -1;
                break;
            }

            /* 转字符串写入 param.value */
            UintToAsciiIntoParamValue(param.value, sizeof(param.value), v);

            /* 更新参数（长度传实际写入的字符串长度；也可传 sizeof(param.value)，两者皆可） */
            uint32_t wlen = (uint32_t)strnlen((const char*)param.value, sizeof(param.value));
            if (UpdateParam(param.id, param.value, wlen) != 0)
            {
                LOG_Debug("UpdateParam int failed id=%d", param.id);
                ret = -1;
                break;
            }
            LOG_Debug("write int ok id=%d val=%s", param.id, (const char*)param.value);
        }
        else if (param.type == 0)
        {
            /* 字符串：原样写入（截断到容量并 '\0' 终止） */
            CopyStringIntoParamValue(param.value, sizeof(param.value), buf, len);

            uint32_t wlen = (uint32_t)strnlen((const char*)param.value, sizeof(param.value));
            if (UpdateParam(param.id, param.value, wlen) != 0)
            {
                LOG_Debug("UpdateParam str failed id=%d", param.id);
                ret = -1;
                break;
            }
            LOG_Debug("write str ok id=%d len=%u", param.id, (unsigned)wlen);
        }
        else
        {
            /* 未支持的类型 */
            LOG_Debug("unsupported param.type=%d for id=%d", param.type, param.id);
            ret = -1;
            break;
        }
    }
    while (0);

    LOG_Debug("UdsWriteToParamsDb done ret=%d", ret);
    return ret;
}
#if 0
// 适配客户以ascii格式写入整形数据
static int WriteToParam4(uint32_t id, const uint8_t *buf, uint16_t len, uint8_t type)
{
    int ret = 0;
    ParamInfo_t param = {0};
    
    LOG_DebugHex(buf, len, "WriteToParamEx");

    do
    {
        if (buf == NULL)
        {
            ret = -1;
            break;
        }

        param.id = id;

        /* 调 GetParam 获取类型、范围等元信息（注意：param.value 里是“当前值”，下面会覆盖） */
        if (GetParam(&param) != 0)
        {
            LOG_Debug("GetParam failed for id=%d", param.id);
            ret = -1;
            break;
        }

        /* 分类型写入 */
        if (param.type == 1)
        {
            uint32_t v = 0;
            if (type == UDS_DATA_TYPE_UINT)
            {
                /* 整形：buf 为大端编码的数值（1/2/4 字节），写入 param.value 的十进制字符串 */
                if (!ParseUintFromBigEndian(buf, len, &v))
                {
                    LOG_Debug("invalid integer length=%u (only 1/2/4 supported)", (unsigned)len);
                    ret = -1;
                    break;
                }

                /* 范围校验（min/max 来自 ParamInfo_t） */
                if (v < (uint32_t)param.min || v > (uint32_t)param.max)
                {
                    LOG_Debug("value %u out of range [%d,%d]", (unsigned)v, param.min, param.max);
                    ret = -1;
                    break;
                }
            }
            else if (type == UDS_DATA_TYPE_ASCII_HEX)
            {
                if (AsciiHexBytesToU32(buf, strlen((char *)buf), &v) != ConvRetOk)
                {
                    LOG_Debug("AsciiHexBytesToU32 failed");
                    ret = -1;
                    break;
                }
            }
            else
            {
                LOG_Debug("unsupported type=%d", type);
                ret = -1;
                break;
            }

            /* 转字符串写入 param.value */
            UintToAsciiIntoParamValue(param.value, sizeof(param.value), v);

            /* 更新参数（长度传实际写入的字符串长度；也可传 sizeof(param.value)，两者皆可） */
            uint32_t wlen = (uint32_t)strnlen((const char*)param.value, sizeof(param.value));
            if (UpdateParam(param.id, param.value, wlen) != 0)
            {
                LOG_Debug("UpdateParam int failed id=%d", param.id);
                ret = -1;
                break;
            }
            LOG_Debug("write int ok id=%d val=%s", param.id, (const char*)param.value);
        }
        else if (param.type == 0)
        {
            /* 字符串：原样写入（截断到容量并 '\0' 终止） */
            CopyStringIntoParamValue(param.value, sizeof(param.value), buf, len);

            uint32_t wlen = (uint32_t)strnlen((const char*)param.value, sizeof(param.value));
            if (UpdateParam(param.id, param.value, wlen) != 0)
            {
                LOG_Debug("UpdateParam str failed id=%d", param.id);
                ret = -1;
                break;
            }
            LOG_Debug("write str ok id=%d len=%u", param.id, (unsigned)wlen);
        }
        else
        {
            /* 未支持的类型 */
            LOG_Debug("unsupported param.type=%d for id=%d", param.type, param.id);
            ret = -1;
            break;
        }
    }
    while (0);

    LOG_Debug("UdsWriteToParamsDb done ret=%d", ret);
    return ret;
}


/* 提取 Project Rev 的值
 * out: 存放版本号的缓冲区
 * out_sz: 缓冲区大小（包含结尾 '\0'）
 * 返回 0 表示成功；<0 表示失败
 */
static int getProjectVersion(char *out, size_t out_sz)
{
    if (!out || out_sz == 0)
        return -1;

    const char *path = "/etc/quectel-project-version";
    const char *needle = "Project Rev";
    char line[512];

    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        LOG_Debug("open %s failed: %s", path, strerror(errno));
        return -2; // 打不开文件
    }

    int found = 0;
    while (fgets(line, sizeof(line), fp))
    {
        LOG_Debug("line: %s", line);
        // 粗略过滤：是否包含 "Project Rev"
        if (strstr(line, needle) == NULL)
            continue;

        // 找冒号':'
        char *colon = strchr(line, ':');
        if (!colon)
            continue;

        // 取冒号后的部分并去空白
        char *val = colon + 1;
        strtrim(val);

        // 若为空就继续找下一行
        if (*val == '\0')
            continue;

        // 拷贝到 out，截断保护
        if (strlen(val) >= out_sz)
        {
            LOG_Debug("%s: line value too long", __func__);
            fclose(fp);
            return -3; // 缓冲区不够
        }
        strcpy(out, val);
        found = 1;
        break;
    }

    fclose(fp);
    return found ? 0 : -4; // 没找到
}

#endif


/**
 * 将 src 文件内容完整拷贝到 dst，替代 system("cp src dst")。
 * 成功返回 0，失败返回 -1 并删除不完整的 dst。
 */
static int copy_file_safe(const char *src, const char *dst)
{
    int ret = -1;
    int fd_src = -1;
    int fd_tmp = -1;
    char tmp_path[PATH_MAX];

    fd_src = open(src, O_RDONLY);
    if (fd_src < 0)
    {
        LOG_Error("open src failed: %s", src);
        return -1;
    }

    /* 临时文件必须与目标文件在同一目录，保证 rename() 在同一文件系统内原子完成，
     * 从而消除「目标文件被 O_TRUNC 截断为空/半截」的窗口（race 根因）。 */
    (void)snprintf(tmp_path, sizeof(tmp_path), "%s.tmp.%d", dst, (int)getpid());

    fd_tmp = open(tmp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_tmp < 0)
    {
        LOG_Error("open tmp failed: %s", tmp_path);
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
                LOG_Error("write tmp failed");
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
            LOG_Error("read src failed");
        }
        goto cleanup;
    }

    /* 数据落盘后再 rename，确保目标文件始终是完整内容 */
    if (fsync(fd_tmp) < 0)
    {
        LOG_Error("fsync tmp failed");
        goto cleanup;
    }

    if (close(fd_tmp) < 0)
    {
        fd_tmp = -1;
        LOG_Error("close tmp failed");
        goto cleanup;
    }
    fd_tmp = -1;

    if (rename(tmp_path, dst) < 0)
    {
        LOG_Error("rename failed: %s -> %s", tmp_path, dst);
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

/**
 * 校验目标 ini 是否满足参考 ini 的结构要求：
 * 1. 参考文件中的每个 section，目标文件必须存在
 * 2. 参考文件中每个 section 下的每个 key，目标文件必须存在
 * 3. 仅校验“是否存在”，不比较 value 是否一致
 *
 * 说明：
 * - 允许目标 ini 比参考 ini 多 section / key
 * - 只要参考 ini 中要求的结构缺失，就认为不合法
 */
static gboolean IniStructureMatch(const char *ref_path, const char *target_path)
{
    GKeyFile *ref_kf = g_key_file_new();
    GKeyFile *tgt_kf = g_key_file_new();
    gboolean match = TRUE;

    GError *ref_err = NULL;
    if (!g_key_file_load_from_file(ref_kf, ref_path, G_KEY_FILE_NONE, &ref_err))
    {
        LOG_Warn("IniStructureMatch: load ref file failed: %s, domain=%d, code=%d, msg=%s",
                 ref_path,
                 ref_err ? ref_err->domain : 0,
                 ref_err ? ref_err->code : 0,
                 ref_err ? ref_err->message : "(null)");
        g_clear_error(&ref_err);
        match = FALSE;
        goto done;
    }

    GError *tgt_err = NULL;
    if (!g_key_file_load_from_file(tgt_kf, target_path, G_KEY_FILE_NONE, &tgt_err))
    {
        LOG_Warn("IniStructureMatch: load target file failed: %s, domain=%d, code=%d, msg=%s",
                 target_path,
                 tgt_err ? tgt_err->domain : 0,
                 tgt_err ? tgt_err->code : 0,
                 tgt_err ? tgt_err->message : "(null)");
        g_clear_error(&tgt_err);
        match = FALSE;
        goto done;
    }

    gsize ref_group_num = 0;
    gchar **ref_groups = g_key_file_get_groups(ref_kf, &ref_group_num);
    if (ref_groups == NULL)
    {
        LOG_Warn("IniStructureMatch: ref_groups is NULL");
        match = FALSE;
        goto done;
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

done:
    g_key_file_unref(ref_kf);
    g_key_file_unref(tgt_kf);
    return match;
}

/**
 * /oemdata/config/tboxNet.ini 文件自修复规则：
 * 1、不存在：从 oemapp net.ini 拷贝
 * 2、存在但解析失败或结构不合法：从 oemapp net.ini 回拷恢复
 * 3、仅校验结构，不比较具体 value
 *
 * 成功返回 TRUE，失败返回 FALSE
 */
static gboolean SanitizeCustomerNetIni(void)
{
    if (access(CUSTOMER_NET_CONFIG_FILE_NAME, F_OK) != 0)
    {
        LOG_Debug("SanitizeCustomerNetIni: tboxNet.ini not found, copy from oemapp");
        if (copy_file_safe(OEPAPP_NET_CONFIG_FILE_NAME, CUSTOMER_NET_CONFIG_FILE_NAME) != 0)
        {
            LOG_Error("SanitizeCustomerNetIni: initial copy failed");
            return FALSE;
        }

        LOG_Debug("SanitizeCustomerNetIni: tboxNet.ini created from oemapp net.ini");
        return TRUE;
    }

    if (!IniStructureMatch(OEPAPP_NET_CONFIG_FILE_NAME, CUSTOMER_NET_CONFIG_FILE_NAME))
    {
        LOG_Warn("SanitizeCustomerNetIni: tboxNet.ini structure mismatch, re-copy from oemapp");
        if (copy_file_safe(OEPAPP_NET_CONFIG_FILE_NAME, CUSTOMER_NET_CONFIG_FILE_NAME) != 0)
        {
            LOG_Error("SanitizeCustomerNetIni: re-copy failed");
            return FALSE;
        }

        LOG_Debug("SanitizeCustomerNetIni: tboxNet.ini restored from oemapp net.ini");
    }

    return TRUE;
}

/**
 * 诊断写 ini 时使用 GKeyFile 加载->修改->重写。
 * 注意：调试中已发现目标设备上的 GLib 版本对 KEEP_COMMENTS 模式下的
 * 某些行内注释可能解析失败，因此此处保留了详细的 GError 日志输出，
 * 以便在目标设备上复现并定位具体的解析失败原因。
 */
static gboolean IniSetString(const char *ini_path,
                               const char *section,
                               const char *key,
                               const char *value,
                               GError **err)
{
    GKeyFile *kf = g_key_file_new();
    GError *load_err = NULL;

    /* 尝试加载现有文件，不再忽略错误 */
    if (!g_key_file_load_from_file(kf, ini_path,
        G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &load_err))
    {
        LOG_Error("IniSetString: g_key_file_load_from_file failed, file=%s, domain=%d, code=%d, msg=%s",
                  ini_path,
                  load_err ? load_err->domain : 0,
                  load_err ? load_err->code : 0,
                  load_err ? load_err->message : "(null)");
        if (err)
        {
            *err = load_err;
        }
        else
        {
            g_clear_error(&load_err);
        }
        g_key_file_unref(kf);
        return FALSE;
    }
    g_clear_error(&load_err);

    // 设置键值（自动建节/键）
    g_key_file_set_string(kf, section, key, value);

    // 生成文本并原子写回
    gsize out_len = 0;
    gchar *out = g_key_file_to_data(kf, &out_len, err);
    if (!out) { g_key_file_unref(kf); return FALSE; }

    // g_file_set_contents 会写临时文件后 rename，原子落盘
    gboolean ok = g_file_set_contents(ini_path, out, out_len, err);
    g_free(out);
    g_key_file_unref(kf);
    return ok;
}

/**
 * 从 ini 读取字符串（UTF-8），写入调用方缓冲区。
 * 成功返回 TRUE；失败则把 def（若提供）写入缓冲区并返回 FALSE。
 * - out/out_sz 必须有效，函数保证以 '\0' 结尾。
 */
static gboolean IniGetStringBuf(const char *ini_path,
                            const char *section,
                            const char *key,
                            char *out, gsize out_sz,
                            const char *def,
                            GError **err_out)  // 可为 NULL
{
    if (!out || out_sz == 0) {
        if (err_out) *err_out = g_error_new_literal(G_FILE_ERROR, G_FILE_ERROR_INVAL, "out buffer invalid");
        return FALSE;
    }

    GError *err = NULL;
    GKeyFile *kf = g_key_file_new();

    gboolean ok = g_key_file_load_from_file(kf, ini_path,
                    G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &err);
    if (!ok) {
        if (err_out) *err_out = err; else g_clear_error(&err);
        g_strlcpy(out, def ? def : "", out_sz);
        g_key_file_unref(kf);
        return FALSE;
    }

    gchar *val = g_key_file_get_string(kf, section, key, &err);
    if (!val) {
        if (err_out) *err_out = err; else g_clear_error(&err);
        g_strlcpy(out, def ? def : "", out_sz);
        g_key_file_unref(kf);
        return FALSE;
    }

    g_strlcpy(out, val, out_sz); // 自动截断并补 '\0'
    g_free(val);
    g_key_file_unref(kf);
    return TRUE;
}

/**
 * /oemdata/config/tboxNet.ini文件更新规则：
 * 1、收到诊断读写请求时，判断配置文件是否存在，不存在则创建配置文件（从oemapp中拷贝），并更新诊断参数到配置文件中
 * 2、如果存在，则读写配置文件
 */
static UdsNrc_e UdsNetworkFileWriteCb(UDS_DID_IDS_e did, uint8_t *buf, uint16_t len)
{
    UdsNrc_e ret = 0;
    const char *section = NULL;

    (void)len; /* APN 名称在 32 字节定长字段内以 '\0' 结尾，比较/写入均按 C 字符串处理 */

    do
    {
        if (buf == NULL)
        {
            LOG_Error("UdsNetworkFileWriteCb: buf is NULL");
            ret = NRC_GENERAL_REJECT_E;
            break;
        }

        switch (did)
        {
            case DID_APN1:
                section = "apn1";
                break;
            case DID_APN2:
                section = "apn0";
                break;
            case DID_APN3:
                section = "apn2";
                break;
            default:
                LOG_Debug("did[%d] is invalid", did);
                ret = NRC_GENERAL_REJECT_E;
                break;
        }
        if (ret != 0 || section == NULL)
        {
            break;
        }

        if (!SanitizeCustomerNetIni())
        {
            LOG_Error("UdsNetworkFileWriteCb: sanitize tboxNet.ini failed");
            ret = NRC_GENERAL_REJECT_E;
            break;
        }

        /* 判同：先读出当前值，若与待写内容一致则跳过写入，避免无谓的文件改写
         * （减少 tboxNet.ini 的写窗口，降低与其它访问者并发踩空的概率） */
        char curValue[64] = {0};
        if (IniGetStringBuf(CUSTOMER_NET_CONFIG_FILE_NAME, section, "apnName",
                            curValue, sizeof(curValue), NULL, NULL) == true)
        {
            if (strcmp(curValue, (const char *)buf) == 0)
            {
                LOG_Debug("UdsNetworkFileWriteCb: apnName unchanged(%s), skip write", curValue);
                break;
            }
        }
        else
        {
            LOG_Warn("UdsNetworkFileWriteCb: read current apnName failed, proceed to write");
        }

        if (IniSetString(CUSTOMER_NET_CONFIG_FILE_NAME, section, "apnName", (char *)buf, NULL) != true)
        {
            LOG_Error("IniSetString failed");
            ret = NRC_GENERAL_REJECT_E;
            break;
        }
    } while (0);

    return ret;
}

static UdsNrc_e UdsNetworkFileReadCb(UDS_DID_IDS_e did, uint8_t *buf, uint16_t *len)
{
    UdsNrc_e ret = 0;

    do
    {
        if (buf == NULL || len == NULL)
        {
            LOG_Error("UdsNetworkFileReadCb: invalid input");
            ret = NRC_GENERAL_REJECT_E;
            break;
        }

        if (!SanitizeCustomerNetIni())
        {
            LOG_Error("UdsNetworkFileReadCb: sanitize tboxNet.ini failed");
            ret = NRC_GENERAL_REJECT_E;
            break;
        }

        switch (did)
        {
            case DID_APN1:
            {
                gsize out_len = *len;
                if (IniGetStringBuf(CUSTOMER_NET_CONFIG_FILE_NAME, "apn1", "apnName", (char *)buf, out_len, NULL, NULL) != true)
                {
                    LOG_Error("IniGetStringBuf failed");
                    ret = NRC_GENERAL_REJECT_E;
                    break;
                }
                *len = strlen((char *)buf);
                break;
            }
            case DID_APN2:
            {
                gsize out_len = *len;
                if (IniGetStringBuf(CUSTOMER_NET_CONFIG_FILE_NAME, "apn0", "apnName", (char *)buf, out_len, NULL, NULL) != true)
                {
                    LOG_Error("IniGetStringBuf failed");
                    ret = NRC_GENERAL_REJECT_E;
                    break;
                }
                *len = strlen((char *)buf);
                break;
            }
            case DID_APN3:
            {
                gsize out_len = *len;
                if (IniGetStringBuf(CUSTOMER_NET_CONFIG_FILE_NAME, "apn2", "apnName", (char *)buf, out_len, NULL, NULL) != true)
                {
                    LOG_Error("IniGetStringBuf failed");
                    ret = NRC_GENERAL_REJECT_E;
                    break;
                }
                *len = strlen((char *)buf);
                break;
            }
            default:
            {
                LOG_Debug("did[%d] is invalid", did);
                ret = NRC_GENERAL_REJECT_E;
                break;
            }
        }

    } while (0);

    return ret;
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
#if 0
// 返回：0 成功；>0 表示被截断且返回需要的总长度；<0 表示错误
static int copy_string_to_buf(void *dst, size_t cap, uint8_t *src)
{
    if (!dst || !src)
        return -1;
    if (cap == 0)
        return (int)strlen((char *)src); // 没空间，告知需要长度

    int n = snprintf((char *)dst, cap, "%.*s", (int)(cap - 1), src);
    if (n < 0)
        return -2; // 写失败
    if ((size_t)n >= cap)
        return n; // 截断，n 为完整长度
    return 0;     // 完全写入
}

static int WriteStringToSysfsWithFopen(const char *path, const char *s)
{
    if (path == NULL || s == NULL)
    {
        LOG_Debug("[WriteStringToSysfsWithFopen] invalid arg\n");
        return -1;
    }

    FILE *fp = fopen(path, "w");
    if (fp == NULL)
    {
        LOG_Debug("[WriteStringToSysfsWithFopen] fopen('%s','w') failed: %s\n", path, strerror(errno));
        return -2;
    }

    /* 模拟 echo 行为：写入字符串并附加换行符 */
    if (fputs(s, fp) == EOF || fputc('\n', fp) == EOF)
    {
        LOG_Debug("[WriteStringToSysfsWithFopen] write to '%s' failed: %s\n", path, strerror(errno));
        fclose(fp);
        return -3;
    }

    /* 刷新缓冲区，确保尽快提交到内核 */
    if (fflush(fp) != 0)
    {
        /* sysfs 通常是立即生效；fflush 失败也提示一下 */
        LOG_Debug("[WriteStringToSysfsWithFopen] fflush('%s') warn: %s\n", path, strerror(errno));
        /* 不中止，继续关闭 */
    }

    if (fclose(fp) != 0)
    {
        LOG_Debug("[WriteStringToSysfsWithFopen] fclose('%s') failed: %s\n", path, strerror(errno));
        return -4;
    }

    return 0;
}

static int EnableAndroidUsb(int enable)
{
    const char *enable_path = "/sys/devices/virtual/android_usb/android0/enable";
    return WriteStringToSysfsWithFopen(enable_path, enable ? "1" : "0");
}

static int ConfigureAndroidUsbFunctions(const char *functions_value)
{
    const char *functions_path = "/sys/devices/virtual/android_usb/android0/functions";
    return WriteStringToSysfsWithFopen(functions_path, functions_value);
}
#endif

int AdbClose(void)
{
    LOG_Debug("close adb\n");
    system("echo off > /data/usb/usb_adb_enable");

    return 0;
}

int AdbOpen(void)
{
    LOG_Debug("open adb\n");
    system("echo on > /data/usb/usb_adb_enable");

    return 0;
}

void UdsCallbackEntryInit(void)
{
    (void)pthread_mutex_init(&g_canSignalMutex, NULL);

    // 订阅电源状态
    // (void)SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, PmStateChangeCallback, NULL);
    // 订阅硬线状态
    (void)SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_KLINE, HardLineSyncCallBack, NULL);
    // 订阅诊断需要用到的can signal
    //(void)SubscriptionQueueMsg((SYSQueMsgType_e)GacT08y1UdsSignal_canVal_t_msgid, DiagDataCallback, NULL);
    // 订阅OTA状态
    // (void)SubscriptionQueueMsg(APP_RPC_QUE_TYPE_TBOX_OTA_MODE_MPU_TO_MPU, OtaStatusCallback, NULL);
}

/*
 * Function: RunCommandAndGetOutput
 * Purpose : 执行指定的 shell 命令，并获取标准输出内容（结尾去掉回车换行符）
 * Parameters:
 *     cmd     - [IN]  要执行的命令字符串
 *     outBuf  - [OUT] 命令输出缓冲区
 *     outSize - [IN]  输出缓冲区大小（字节数）
 * Return:
 *     0   - 执行成功
 *    -1   - 输入参数错误
 *    -2   - popen 失败
 */
int RunCommandAndGetOutput(const char *cmd, char *outBuf, size_t outSize)
{
    if (cmd == NULL || outBuf == NULL || outSize == 0U)
    {
        return -1;
    }

    FILE *pp = popen(cmd, "r");
    if (pp == NULL)
    {
        LOG_Debug("popen('%s') failed: %s", cmd, strerror(errno));
        return -2;
    }

    size_t total = 0U;
    while (!feof(pp))
    {
        if (total + 1U >= outSize)
        {
            break;
        }

        size_t r = fread(outBuf + total, 1U, outSize - 1U - total, pp);
        if (r == 0U)
        {
            break;
        }

        total += r;
    }

    /* 先补 '\0'，保证是字符串 */
    outBuf[total] = '\0';

    /* 去掉字符串末尾的 '\n' 和 '\r'（包括 "\r\n"、多余的回车） */
    if (total > 0U)
    {
        size_t i = total;
        while (i > 0U && (outBuf[i - 1U] == '\n' || outBuf[i - 1U] == '\r'))
        {
            i--;
        }
        outBuf[i] = '\0';
    }

    int rc = pclose(pp);
    if (rc == -1)
    {
        LOG_Debug("pclose failed: %s", strerror(errno));
    }

    LOG_Debug("CMD out (%s): %s", cmd, outBuf);
    return 0;
}

int UdsGetParamIdFromMappingTable(uint16_t did, DeviceParamEnum *paramId)
{
    int ret = 0;
    uint8_t index = 0;

    for (index = 0; index < UDS_DID_MAPPING_TABLE_COUNT; index++)
    {
        if (did == g_udsParamMapping[index].did)
        {
            *paramId = g_udsParamMapping[index].paramId;
            break;
        }
    }

    if (index >= UDS_DID_MAPPING_TABLE_COUNT)
    {
        ret = -1;
    }

    return ret;
}

UdsNrc_e UdsWriteToParamsDb(uint16_t did, const uint8_t *buf, uint16_t len)
{
    UdsNrc_e rc = NRC_NONE_E;
    DeviceParamEnum paramId = 0;

    LOG_Debug("write did: 0x%04x", did);
    do
    {
         if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
        {
            LOG_Debug("did: 0x%04x not found", did);
            rc = NRC_GENERAL_REJECT_E;
            break;
        }
       
        if (WriteToParam(paramId, (uint8_t *)buf, len) != 0)
        {
            LOG_Debug("UpdateParam failed");
            rc = NRC_GENERAL_REJECT_E;
            break;
        }
       
    #if 0
        if (did == DID_LOG_DOMAIN_NAME)
        {
            // 需要同步更新B280,B281
            if (len <= 50)
            {
                (void)UdsDidWriteForOwner(DID_LOG_DOMAIN_FRONT, buf, len);
            }
            else
            {
                (void)UdsDidWriteForOwner(DID_LOG_DOMAIN_FRONT, buf, 50);
                (void)UdsDidWriteForOwner(DID_LOG_DOMAIN_LATER, buf+50, len-50);
            }
        }
        else if (did == DID_LOG_PATH)
        {
            if (len > 256)
            {
                len = 256;
            }

            if (len > 192)
            {
                (void)UdsDidWriteForOwner(DID_LOG_PATH_P4, buf+192, len-192);
            }
            if (len > 128)
            {
                (void)UdsDidWriteForOwner(DID_LOG_PATH_P3, buf+128, 64);
            }
            if (len > 64)
            {
                (void)UdsDidWriteForOwner(DID_LOG_PATH_P2, buf+64, 64);
            }
            (void)UdsDidWriteForOwner(DID_LOG_PATH_P1, buf, 64);

        }
        else if (did == DID_BURIED_POINT_DOMAIN)
        {
            if (len > 64)
            {
                (void)UdsDidWriteForOwner(DID_BURIED_POINT_DOMAIN_2, buf+64, len-64);
            }
            (void)UdsDidWriteForOwner(DID_BURIED_POINT_DOMAIN_1, buf, 64);
        }
        else
        {
            // do nothing
        }
    
        if (paramId == (DeviceParamEnum)E_ParamId_Sensitive_Data_Set)
        {
            // buf 0-3 E_ParamId_Sensitive_Data_Set_1
            if (WriteToParam(E_ParamId_Sensitive_Data_Set_1, (uint8_t *)buf, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
            // buf 4-7 E_ParamId_Sensitive_Data_Set_2
            if (WriteToParam(E_ParamId_Sensitive_Data_Set_2, (uint8_t *)buf + 4, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
            // buf 8-11 E_ParamId_Sensitive_Data_Set_3
            if (WriteToParam(E_ParamId_Sensitive_Data_Set_3, (uint8_t *)buf + 8, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
            // buf 12-15 E_ParamId_Sensitive_Data_Set_4
            if (WriteToParam(E_ParamId_Sensitive_Data_Set_4, (uint8_t *)buf + 12, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
        }
        else if (paramId == (DeviceParamEnum)E_ParamId_OTA_RTC_Wakeup_Param)
        {
            // buf 0-3 E_ParamId_OTA_RTC_Wakeup_Param_1
            if (WriteToParam(E_ParamId_OTA_RTC_Wakeup_Param_1, (uint8_t *)buf, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
            // buf 4-7 E_ParamId_OTA_RTC_Wakeup_Param_2
            if (WriteToParam(E_ParamId_OTA_RTC_Wakeup_Param_2, (uint8_t *)buf + 4, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
        }
        else if (paramId == (DeviceParamEnum)E_ParamId_UserPrivateTspPort)
        {
            // buf 0-3 E_ParamId_UserPrivateTspPort_1
            if (WriteToParam(E_ParamId_UserPrivateTspPort_1, (uint8_t *)buf, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
            // buf 4-7 E_ParamId_UserPrivateTspPort_2
            if (WriteToParam(E_ParamId_UserPrivateTspPort_2, (uint8_t *)buf + 4, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }

            if (WriteToParam(paramId, (uint8_t *)buf, len, UDS_DATA_TYPE_ASCII_HEX) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
        }
        else if (paramId == (DeviceParamEnum)E_ParamId_UserPublicTspPort)
        {
            if (WriteToParam(E_ParamId_UserPublicTspPort_1, (uint8_t *)buf, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
            if (WriteToParam(E_ParamId_UserPublicTspPort_2, (uint8_t *)buf + 4, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }

            if (WriteToParam(paramId, (uint8_t *)buf, len, UDS_DATA_TYPE_ASCII_HEX) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
        }
        else if (paramId == (DeviceParamEnum)E_ParamId_UserPkiPort)
        {
            if (WriteToParam(E_ParamId_UserPkiPort_1, (uint8_t *)buf, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
            if (WriteToParam(E_ParamId_UserPkiPort_2, (uint8_t *)buf + 4, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }

            if (WriteToParam(paramId, (uint8_t *)buf, len, UDS_DATA_TYPE_ASCII_HEX) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
        }
        else if (paramId == (DeviceParamEnum)E_ParamId_UserLogUploadPort)
        {
            if (WriteToParam(E_ParamId_UserLogUploadPort_1, (uint8_t *)buf, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
            if (WriteToParam(E_ParamId_UserLogUploadPort_2, (uint8_t *)buf + 4, 4) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }

            if (WriteToParam(paramId, (uint8_t *)buf, len, UDS_DATA_TYPE_ASCII_HEX) != 0)
            {
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
        }
        else if (paramId == (DeviceParamEnum)E_ParamId_UserPkiCycle)
        {
            if (WriteToParam(paramId, (uint8_t *)buf, len) != 0){
                LOG_Debug("UpdateParam failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
            // TODO 执行PKI同步操作
            // GacPkiDefaultSync();
        }
    #endif       
    } while (0);

    return rc;
}

UdsNrc_e UdsReadFromParamsDb(uint16_t did, uint8_t *buf, uint16_t *io_len)
{
    UdsNrc_e rc = NRC_NONE_E;
    ParamInfo_t param = {0};

    if (!buf || !io_len) return NRC_GENERAL_REJECT_E;

    LOG_Debug("read did: 0x%04x", did);
    do
    {
        /* 先根据映射表拿到“主参数 id” */
        if (UdsGetParamIdFromMappingTable(did, &(param.id)) != 0)
        {
            LOG_Debug("did: 0x%04x not found", did);
            rc = NRC_GENERAL_REJECT_E;
            break;
        }
        LOG_Debug("param id: %d", param.id);
    #if 0
        /* -------- 分段类（每段都是十进制整形字符串） -------- */
        if (param.id == E_ParamId_Sensitive_Data_Set)
        {
            /* 4 段，每段 4 字节大端，总 16 字节 */
            if (!ReadOneIntParamToBE(E_ParamId_Sensitive_Data_Set_1, buf +  0, 4) ||
                !ReadOneIntParamToBE(E_ParamId_Sensitive_Data_Set_2, buf +  4, 4) ||
                !ReadOneIntParamToBE(E_ParamId_Sensitive_Data_Set_3, buf +  8, 4) ||
                !ReadOneIntParamToBE(E_ParamId_Sensitive_Data_Set_4, buf + 12, 4))
            {
                rc = NRC_GENERAL_REJECT_E; break;
            }
            *io_len = 16;
            LOG_DebugHex(buf, *io_len, "SENSITIVE");
            break;
        }
        else if (param.id == E_ParamId_OTA_RTC_Wakeup_Param)
        {
            /* 2 段，每段 4 字节大端，总 8 字节 */
            if (!ReadOneIntParamToBE(E_ParamId_OTA_RTC_Wakeup_Param_1, buf + 0, 4) ||
                !ReadOneIntParamToBE(E_ParamId_OTA_RTC_Wakeup_Param_2, buf + 4, 4))
            {
                rc = NRC_GENERAL_REJECT_E; break;
            }
            *io_len = 8;
            LOG_DebugHex(buf, *io_len, "OTA_RTC_Wakeup_Param");
            break;
        }
        else if (param.id == E_ParamId_UserPrivateTspPort)
        {
            /* 私网端口：分两段（_1/_2），各 4 字节大端 → 总 8 字节 */
            if (!ReadOneIntParamToBE(E_ParamId_UserPrivateTspPort_1, buf + 0, 4) ||
                !ReadOneIntParamToBE(E_ParamId_UserPrivateTspPort_2, buf + 4, 4))
            {
                rc = NRC_GENERAL_REJECT_E; break;
            }
            *io_len = 8;
            LOG_DebugHex(buf, *io_len, "UDS_GetParam: E_ParamId_UserPrivateTspPort");
            break;
        }
        else if (param.id == E_ParamId_UserPublicTspPort)
        {
            /* 私网端口：分两段（_1/_2），各 4 字节大端 → 总 8 字节 */
            if (!ReadOneIntParamToBE(E_ParamId_UserPublicTspPort_1, buf + 0, 4) ||
                !ReadOneIntParamToBE(E_ParamId_UserPublicTspPort_2, buf + 4, 4))
            {
                rc = NRC_GENERAL_REJECT_E; break;
            }
            *io_len = 8;
            LOG_DebugHex(buf, *io_len, "UDS_GetParam: E_ParamId_UserPublicTspPort");
            break;
        }
        else if (param.id == E_ParamId_UserPkiPort)
        {
            /* PKI 端口：同上，总 8 字节 */
            if (!ReadOneIntParamToBE(E_ParamId_UserPkiPort_1, buf + 0, 4) ||
                !ReadOneIntParamToBE(E_ParamId_UserPkiPort_2, buf + 4, 4))
            {
                rc = NRC_GENERAL_REJECT_E; break;
            }
            *io_len = 8;
            LOG_DebugHex(buf, *io_len, "PKI_PORT");
            break;
        }
        else if (param.id == E_ParamId_UserLogUploadPort)
        {
            /* PKI 端口：同上，总 8 字节 */
            if (!ReadOneIntParamToBE(E_ParamId_UserLogUploadPort_1, buf + 0, 4) ||
                !ReadOneIntParamToBE(E_ParamId_UserLogUploadPort_2, buf + 4, 4))
            {
                rc = NRC_GENERAL_REJECT_E; break;
            }
            *io_len = 8;
            LOG_DebugHex(buf, *io_len, "LOG_PORT");
            break;
        }
    #endif
        /* -------- 普通参数：按 type 处理 -------- */
        if (GetParam(&param) != 0)
        {
            LOG_Debug("GetParam failed");
            rc = NRC_GENERAL_REJECT_E;
            break;
        }

        if (param.type == 1) /* 整形：value 是十进制字符串 → 解析后按大端写 4 字节 */
        {
            uint32_t v = 0;
            if (!ParseUint32FromAscii(param.value, sizeof(param.value), 0U, 0xFFFFFFFFU, &v))
            {
                LOG_Debug("parse int failed");
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
            uint8_t tmp[4] = {0};
            PutU32BE(tmp, v);

            uint32_t count = 0u;
            uint16_t len = 0u;
            const UdsDidEntry_t * didEntry = UdsGetDidRegistry(&count);
            for (uint32_t i = 0; i < count; i++)
            {
                if (didEntry[i].did == did)
                {
                    if (didEntry[i].min_len == didEntry[i].max_len)
                    {
                        len = didEntry[i].min_len;
                    }
                }
            }
            uint16_t offset = 4-len;
            memcpy(buf, tmp + offset, 4-offset);
            *io_len = len;
            // int start = 0;
            // while (start < 3 && tmp[start] == 0x00)
            // {
            //     start++;
            // }
            // uint16_t outlen = 4 - start;
            // memcpy(buf, tmp + start, outlen);
            // *io_len = outlen;
            LOG_DebugHex(buf, *io_len, "put int");
        }
        else if (param.type == 0) /* 字符串：原样拷贝（不含多余尾部 0） */
        {
            size_t sl = strnlen((const char*)param.value, sizeof(param.value));
            memcpy(buf, param.value, sl);
            *io_len = (uint16_t)sl;
            LOG_Debug("STR did=0x%04X, len=%u, str=\"%.*s\"",
                      did, (unsigned)*io_len, (int)*io_len, (const char*)buf);
        }
        else
        {
            LOG_Debug("unknown param.type=%d", param.type);
            rc = NRC_GENERAL_REJECT_E;
            break;
        }
    } while (0);

    LOG_Debug("%s, rc=%d", __FUNCTION__, rc);
    return rc;
}

UdsNrc_e UdsReadFromCanSignal(uint16_t did, uint8_t *buf, uint16_t *io_len)
{
    LOG_Debug("recv did: 0x%04x", did);
    UdsNrc_e rc = NRC_NONE_E;

    do
    {
        if (buf == NULL || io_len == NULL)
        {
            rc = NRC_GENERAL_REJECT_E;
            break;
        }

        (void)pthread_mutex_lock(&g_canSignalMutex);
        switch (did)
        {
        #if 0
            case DID_KEY_STATUS:
            {
                const uint16_t data_len = (uint16_t)sizeof(g_subData.diagSignalData.BCM_KeySt);
                if (*io_len < data_len)
                {
                    *io_len = data_len; // 告知上层需要的长度
                    rc = NRC_INCORRECT_MESSAGE_LENGTH_E;
                    break;
                }
                buf[0] = g_subData.diagSignalData.BCM_KeySt;
                *io_len = data_len; // 实际返回长度
            }
            case DID_VEHICLE_SPEED:
            {
                const uint16_t data_len = (uint16_t)sizeof(g_subData.diagSignalData.BCS_VehSpd);
                if (*io_len < data_len)
                {
                    *io_len = data_len; // 告知上层需要的长度
                    rc = NRC_INCORRECT_MESSAGE_LENGTH_E;
                    break;
                }
                memcpy((void *)buf, (void *)&g_subData.diagSignalData.BCS_VehSpd, sizeof(g_subData.diagSignalData.BCS_VehSpd));
                *io_len = data_len; // 实际返回长度
            }
            case DID_MOTOR_SPEED:
            {
                const uint16_t data_len = (uint16_t)sizeof(g_subData.diagSignalData.EMS_EngSpd);
                if (*io_len < data_len)
                {
                    *io_len = data_len; // 告知上层需要的长度
                    rc = NRC_INCORRECT_MESSAGE_LENGTH_E;
                    break;
                }
                memcpy((void *)buf, (void *)&g_subData.diagSignalData.EMS_EngSpd, sizeof(g_subData.diagSignalData.EMS_EngSpd));
                *io_len = data_len; // 实际返回长度
            }
        #endif
            default:
            {
                rc = NRC_REQUEST_OUT_OF_RANGE_E;
                break;
            }        
        }
        (void)pthread_mutex_unlock(&g_canSignalMutex);
    } while (0);

    return rc;
}

UdsNrc_e UdsWriteToFile(uint16_t did, const uint8_t *buf, uint16_t len)
{
    LOG_Debug("recv did: 0x%04x", did);
    UdsNrc_e rc = NRC_NONE_E;

    do
    {
        if (buf == NULL)
        {
            rc = NRC_GENERAL_REJECT_E;
            break;
        }

        for (int i = 0; i < UDS_FILE_HANDLE_TABLE_COUNT; i++)
        {
            if (did == g_udsFileMapping[i].did)
            {
                if (g_udsFileMapping[i].writeCb != NULL)
                {
                    rc = g_udsFileMapping[i].writeCb(did, (uint8_t *)buf, len);
                }
                break;
            }
        }
    } while (0);

    return rc;
}

UdsNrc_e UdsReadFromFile(uint16_t did, uint8_t *buf, uint16_t *io_len)
{
    LOG_Debug("recv did: 0x%04x", did);
    UdsNrc_e rc = NRC_NONE_E;

    do
    {
        // 1) 基本参数校验
        if (buf == NULL || io_len == NULL)
        {
            rc = NRC_GENERAL_REJECT_E;
            break;
        }

        for (int i = 0; i < UDS_FILE_HANDLE_TABLE_COUNT; i++)
        {
            if (did == g_udsFileMapping[i].did)
            {
                if (g_udsFileMapping[i].readCb != NULL)
                {
                    rc = g_udsFileMapping[i].readCb(did, buf, io_len);
                }
                break;
            }
        }
    } while(0);

    return rc;
}

UdsNrc_e UdsReadCommFunc(uint16_t did, uint8_t *buf, uint16_t *io_len)
{
    LOG_Debug("recv did: 0x%04x", did);
    UdsNrc_e rc = NRC_NONE_E;
    ModemSrvNotify_t g_modemData;
    GnssLocation_t g_gnssData;
    do
    {
        // 1) 基本参数校验
        if (buf == NULL || io_len == NULL)
        {
            rc = NRC_GENERAL_REJECT_E;
            break;
        }
        uint32_t count = 0u;
        const UdsDidEntry_t * didEntry = UdsGetDidRegistry(&count);
        for (uint32_t i = 0; i < count; i++)
        {
            if (didEntry[i].did == did)
            {
                if (didEntry[i].min_len == didEntry[i].max_len)
                {
                    *io_len = didEntry[i].min_len;
                }
            }
        }

        switch (did)
        {
    #if 1   //SQ EOL检测项DID读取处理 
        // MCU端直接读取
            // case DID_KL15_STATUS:
            // {
            //     // KL15状态：1=ON, 0=OFF
            //     *io_len = 1u;
            //     (void)pthread_mutex_lock(&g_canSignalMutex);
            //     buf[0] = g_subData.hardLineData.kl15status;
            //     (void)pthread_mutex_unlock(&g_canSignalMutex);
            //     LOG_Debug("DID_KL15_STATUS: %d", buf[0]);
            //     break;
            // }
            // case DID_KL30_VOLTAGE:
            // {
            //     // KL30电压：单位mV，范围11000-13000
            //     *io_len = 2u;
            //     (void)pthread_mutex_lock(&g_canSignalMutex);
            //     uint32_t voltage = g_subData.hardLineData.kl30voltage;
            //     (void)pthread_mutex_unlock(&g_canSignalMutex);
            //     buf[0] = (uint8_t)((voltage >> 8) & 0xFF);
            //     buf[1] = (uint8_t)(voltage & 0xFF);
            //     LOG_Debug("DID_KL30_VOLTAGE: %u mV", voltage);
            //     break;
            // }        
            case DID_CSQ:
            {
                // CSQ信号强度：>=15且不等于99为合格
                *io_len = 1u;            
                buf[0] = g_modemData.csq;
                LOG_Debug("DID_CSQ: %d", buf[0]);
                break;
            }
            case DID_GNSS_STATUS:
            {
                // GNSS状态：1=就绪
                *io_len = 1u;
                
                uint8_t gnssReady = (g_gnssData.gpsState & 0x3F) > 0 ? 1 : 0;
                buf[0] = gnssReady;
                LOG_Debug("DID_GNSS_STATUS: %d", buf[0]);
                break;
            }
            case DID_EMMC_STATE:
            {
                // EMMC挂载状态：1=成功
                *io_len = 1u;
                int ret = system("mount | grep -qE ' on (/mnt/sdcard)(/| )'");
                if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
                {
                    buf[0] = 1;  // 已挂载
                }
                else
                {
                    buf[0] = 0;  // 未挂载
                }
                LOG_Debug("DID_EMMC_STATE: %d", buf[0]);
                break;
            }    
            case DID_4G_ANTENNA_STATUS:
            {
                // 4G天线状态：1=正常
                *io_len = 1u;            
                buf[0] = (g_modemData.antennaStatus == E_MODEM_SRV_ANT_STATUS_CIRCUIT_NORMAL) ? 1 : 0;
                LOG_Debug("DID_4G_ANTENNA_STATUS: %d", buf[0]);
                break;
            }
            case DID_GNSS_ANTENNA_STATUS:
            {
                // GNSS天线状态：1=正常
                *io_len = 1u;            
                buf[0] = (g_modemData.secondaryAntennaStatus == E_MODEM_SRV_ANT_STATUS_CIRCUIT_NORMAL) ? 1 : 0;
                LOG_Debug("DID_GNSS_ANTENNA_STATUS: %d", buf[0]);
                break;
            }      
    #else
            case DID_CELL_STATUS_4G_3G_2G:
            {
                MPUPmStateChange_t pmState = {0};
                RpcAppAvntSessionStatus_e status = 0;
                GetPmPowerState(&pmState);

                *io_len = 2u;
                uint8_t powerMode = ((pmState.PmState == PM_STATE_SLEEP) ? 3 : 5); // 电源状态0-3bit
                uint8_t dtr = 0x00; // 4bit
                uint8_t dsr = 0x00; // 5bit
                uint8_t rts = 0x00; // 6bit
                uint8_t cts = 0x01; // 7bit

                (void)GetAVNTSessionStatus(&status, 2*1000);
                if (status == RPC_APP_AVNT_CONN_STATE_DISCONNECTED_E)
                {
                    dtr = 0;
                    dsr = 0;
                    rts = 0;
                    cts = 1;
                }
                else if (status == RPC_APP_AVNT_CONN_STATE_CONNECTED_E)
                {
                    dtr = 1;
                    dsr = 1;
                    rts = 0;
                    cts = 1;
                }
                else if (status == RPC_APP_AVNT_CONN_STATE_AUTHENTICATED_E)
                {
                    dtr = 1;
                    dsr = 1;
                    rts = 1;
                    cts = 0;
                }
                buf[0] = (powerMode & 0x0F) | ((dtr & 0x01) << 4) | ((dsr & 0x01) << 5) | ((rts & 0x01) << 6) | ((cts & 0x01) << 7);
                buf[1] = 0x00;
                break;
            }
            case DID_EMMC_STATE:
            {
                *io_len = 1u;
                // checkout sdcard是否加载成功
                int rc = system("mount | grep -qE ' on (/mnt/sdcard)(/| )'");
                if (rc == -1)
                {
                    LOG_Debug("system call failed");
                    rc = NRC_GENERAL_REJECT_E; // system 调用失败
                    break;
                }
                if (WIFEXITED(rc))
                {
                    int code = WEXITSTATUS(rc);
                    if (code == 0) // 找到了挂载项 => 已挂载
                    {
                        LOG_Debug("emmc is mounted");
                        buf[0] = 1;
                    }
                    else
                    {
                        LOG_Debug("emmc is not mounted");
                        buf[0] = 0;
                    }
                }
                break;
            }
            case DID_4G_MODULE_FW_VER:
            {
                char version[64] = {0};
                size_t out_sz = sizeof(version);
                int ret = getProjectVersion(version, out_sz);
                if (ret != 0)
                {
                    LOG_Error("getProjectVersion failed, ret:%d", ret);
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }
                memcpy((char *)buf, version, out_sz);
                *io_len = strlen(version);
                break;
            }
        
            case DID_PKI_CERT_STATUS:
            {
                // 获取证书状态
                /**
                 * 0=无可用证书
                 * 1=公有证书可用
                 * 2=专属证书可用
                 * 3=公有证书及专属证书均可用
                 */
                *io_len = 1u;
                if (GacPkiGetSyncState() == 0)
                {
                    buf[0] = 2;
                }
                else
                {
                    buf[0] = 0;
                }
                break;
            }
            case DID_PKI_SDK_VER:
            {
                char *ver = NULL;
                int verLen = 0;
                if (GacPkiGetVersion(&ver, &verLen) == 0)
                {
                    LOG_Debug("get pki sdk version success, ver len = %d, ver = %s", verLen, ver);
                    memcpy(buf, ver, verLen);
                    buf[verLen] = '\0';
                    *io_len = verLen;
                }
                else
                {
                    LOG_Error("get pki sdk version failed");
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }
                break;
            }
            case DID_TSP_PLATFORM_CONN_STATUS:
            {
                LOG_Debug("recv did: 0x%04x", did);
                uint8_t status = 0;
                if (GetTspConnectStatus(&status, 0, 2*1000) != 0) // 私网平台
                {
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }
                buf[0] = status & 0x03;

                if (GetTspConnectStatus(&status, 1, 2*1000) != 0) // 公网平台
                {
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }
                LOG_Debug("status: %d", status);
                buf[0] = buf[0] | ((status & 0x03) << 2);
                *io_len = 1u;
                break;
            }
        
            case DID_ADB_STATUS:
            {
                LOG_Debug("Running: cat /data/usb/usb_adb_enable");
                char outBuf[32];
                int sc = RunCommandAndGetOutput("cat /data/usb/usb_adb_enable", outBuf, sizeof(outBuf));
                if (sc != 0)
                {
                    LOG_Debug("Run read cmd failed");
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }
                LOG_Debug("outbuf: %s", outBuf);
                if (strcmp(outBuf, "on") == 0)
                {
                    LOG_Debug("adb on");
                    buf[0] = 0x01;
                }
                else
                {
                    LOG_Debug("adb off");
                    buf[0] = 0x00;
                }
                *io_len = 1u;
                break;
            }
            case DID_CAN_CHANNEL_STATUS:
            {
                // CAN通道状态：1=正常
                *io_len = 1u;
                (void)pthread_mutex_lock(&g_canSignalMutex);
                // 暂时返回1，实际需要从CAN信号获取
                buf[0] = 1;
                (void)pthread_mutex_unlock(&g_canSignalMutex);
                LOG_Debug("DID_CAN_CHANNEL_STATUS: %d", buf[0]);
                break;
            }
            case DID_SW_VERSION_INTERNAL:
            {
                // 内部版本号：12字节
                *io_len = 12u;
                char version[64] = {0};
                int ret = getProjectVersion(version, sizeof(version));
                if (ret != 0)
                {
                    LOG_Error("getProjectVersion failed");
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }
                size_t verLen = strlen(version);
                if (verLen > 12) verLen = 12;
                memcpy(buf, version, verLen);
                if (verLen < 12) memset(buf + verLen, 0, 12 - verLen);
                LOG_Debug("DID_SW_VERSION_INTERNAL: %s", version);
                break;
            }
            case DID_SW_VERSION_EXTERNAL:
            {
                // 外部版本号：17字节
                *io_len = 17u;
                char version[64] = {0};
                int ret = getProjectVersion(version, sizeof(version));
                if (ret != 0)
                {
                    LOG_Error("getProjectVersion failed");
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }
                size_t verLen = strlen(version);
                if (verLen > 17) verLen = 17;
                memcpy(buf, version, verLen);
                if (verLen < 17) memset(buf + verLen, ' ', 17 - verLen);
                LOG_Debug("DID_SW_VERSION_EXTERNAL: %s", version);
                break;
            }

            case DID_HW_VERSION:
            {
                // 硬件版本号：17字节
                *io_len = 17u;
                ParamInfo_t param = {0};
                param.id = E_ParamId_HW_Version;
                if (GetParam(&param) == 0)
                {
                    size_t hwLen = strlen(param.value);
                    if (hwLen > 17) hwLen = 17;
                    memcpy(buf, param.value, hwLen);
                    if (hwLen < 17) memset(buf + hwLen, ' ', 17 - hwLen);
                }
                else
                {
                    memset(buf, ' ', 17);
                }
                LOG_Debug("DID_HW_VERSION read");
                break;
            }
    #endif   
    
            default:
            {
                rc = NRC_REQUEST_OUT_OF_RANGE_E;
                break;
            }        
        }

    } while (0);

    return rc;
}

UdsNrc_e UdsWriteCommFunc(uint16_t did, const uint8_t *buf, uint16_t len)
{
    UdsNrc_e rc = NRC_NONE_E;
    DeviceParamEnum paramId = 0;

    LOG_Debug("write did: 0x%04x", did);
    do
    {
        if (buf == NULL)
        {
            rc = NRC_GENERAL_REJECT_E;
            break;
        }

        switch(did)
        {
        #if 0
            case DID_LOG_DOMAIN_FRONT:
            {
                // 1、写到dtc数据库中
                UdsDidWriteForOwner(did, buf, len);

                // 2、更新0xB275-获取B275 value
                ParamInfo_t param = {0};
                if (UdsGetParamIdFromMappingTable(DID_LOG_DOMAIN_NAME, &(param.id)) != 0)
                {
                    LOG_Debug("did: 0x%04x not found", did);
                    LOG_Debug("update DID_LOG_DOMAIN_NAME failed");
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }

                if (GetParam(&param) != 0)
                {
                    LOG_Debug("GetParam failed");
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }

                // 2、更新0xB275-更新B275前50个字节
                uint8_t data[100+1] = {0};
                memcpy(data, buf, len);
                snprintf((char *)data+50, 50, "%s", param.value+50);
                data[100] = '\0';
                memcpy(param.value, data, 100);
                WriteToParam(param.id, param.value, 100);
                break;
            }
            case DID_LOG_DOMAIN_LATER:
            {
                // 1、写到dtc数据库中
                UdsDidWriteForOwner(did, buf, len);

                // 2、更新0xB275-获取B275 value
                ParamInfo_t param = {0};
                if (UdsGetParamIdFromMappingTable(DID_LOG_DOMAIN_NAME, &(param.id)) != 0)
                {
                    LOG_Debug("did: 0x%04x not found", did);
                    LOG_Debug("update DID_LOG_DOMAIN_NAME failed");
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }

                if (GetParam(&param) != 0)
                {
                    LOG_Debug("GetParam failed");
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }

                // 2、更新0xB275-更新B275后50个字节
                snprintf((char *)param.value+50, len, "%s", buf);
                WriteToParam(param.id, param.value, 100);
                break;
            }
        
            case DID_LOG_CONN:
            {
                // LogMgrTriggerRet_e ret = LogMgrTriggerProcess(buf, len);
                LogRecall_TaskRet_e ret = LogRecall_ProcessTaskTrigger(buf, len, NULL, 1, NULL);
                if (ret != E_LOGRECALL_TASK_OK)
                {
                    LOG_Debug("LogMgrTriggerProcess failed %d", ret);
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }
                break;
            }
            case DID_LOG_PATH_P1:
            case DID_LOG_PATH_P2:
            case DID_LOG_PATH_P3:
            case DID_LOG_PATH_P4:
            {
                // 1、写到dtc数据库中
                UdsDidWriteForOwner(did, buf, len);

                // 2、更新0xB282
                ParamInfo_t param = {0};
                if (UdsGetParamIdFromMappingTable(DID_LOG_PATH, &(param.id)) != 0)
                {
                    LOG_Debug("did: 0x%04x not found", did);
                    LOG_Debug("update DID_LOG_PATH failed");
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }

                if (GetParam(&param) != 0)
                {
                    LOG_Debug("GetParam failed");
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }
                uint8_t data[256+1] = {0};
                if (did == DID_LOG_PATH_P1)
                {
                    if (len < 64)
                    {
                        memcpy((char *)param.value, buf, len);
                    }
                    else
                    {
                        memcpy(data, buf, len);
                        (void)copy_string_to_buf(data + 64, strlen((char *)param.value) - 64, param.value + 64);
                        data[256] = '\0';
                        memcpy((char *)param.value, (char *)data, strlen((char *)data));
                    }
                }
                else if (did == DID_LOG_PATH_P2)
                {
                    if (len < 64)
                    {
                        (void)copy_string_to_buf(data, 64, param.value);
                        (void)copy_string_to_buf(data + 64, len, (uint8_t *)buf);
                        data[strlen((char *)data)] = '\0';

                    }
                    else
                    {
                        (void)copy_string_to_buf(data, 64, param.value);
                        (void)copy_string_to_buf(data + 64, 64, (uint8_t *)buf);
                        (void)copy_string_to_buf(data + 128, strlen((char *)param.value) - 128, param.value + 128);
                        data[256] = '\0';
                    }
                    memcpy((char *)param.value, (char *)data, strlen((char *)data));
                }
                else if (did == DID_LOG_PATH_P3)
                {
                    if (len < 64)
                    {
                        (void)copy_string_to_buf(data, 128, param.value);
                        (void)copy_string_to_buf(data + 128, len, (uint8_t *)buf);
                        data[strlen((char *)data)] = '\0';
                    }
                    else
                    {
                        (void)copy_string_to_buf(data, 128, param.value);
                        (void)copy_string_to_buf(data + 128, 64, (uint8_t *)buf);
                        (void)copy_string_to_buf(data + 192, strlen((char *)param.value) - 192, param.value + 192);
                        data[256] = '\0';
                    }
                    memcpy((char *)param.value, (char *)data, strlen((char *)data));
                }
                else if (did == DID_LOG_PATH_P4)
                {
                    (void)copy_string_to_buf(data, 192, param.value);
                    (void)copy_string_to_buf(data + 192, len, (uint8_t *)buf);
                    data[256] = '\0';
                    memcpy((char *)param.value, (char *)data, strlen((char *)data));
                }
                else
                {
                    // do nothing
                }

                WriteToParam(param.id, param.value, 256);
                break;
            }
       
            case DID_BURIED_POINT_DOMAIN_1:
            case DID_BURIED_POINT_DOMAIN_2:
            {
                // 1、写到dtc数据库中
                UdsDidWriteForOwner(did, buf, len);

                // 2、更新0xB289
                ParamInfo_t param = {0};
                if (UdsGetParamIdFromMappingTable(DID_BURIED_POINT_DOMAIN, &(param.id)) != 0)
                {
                    LOG_Debug("did: 0x%04x not found", did);
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }

                if (GetParam(&param) != 0)
                {
                    LOG_Debug("GetParam failed");
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }
                uint8_t data[128+1] = {0};
                if (did == DID_BURIED_POINT_DOMAIN_1)
                {
                    if (len < 64)
                    {
                        memcpy((char *)param.value, buf, len);
                    }
                    else
                    {
                        memcpy(data, buf, len);
                        (void)copy_string_to_buf(data + 64, strlen((char *)param.value) - 64, param.value + 64);
                        data[128] = '\0';
                        memcpy((char *)param.value, (char *)data, strlen((char *)data));
                    }
                }
                else if (did == DID_BURIED_POINT_DOMAIN_2)
                {
                    (void)copy_string_to_buf(data, 64, param.value);
                    (void)copy_string_to_buf(data + 64, len, (uint8_t *)buf);
                    data[128] = '\0';
                    memcpy((char *)param.value, (char *)data, strlen((char *)data));
                }
                else
                {
                    // do nothing
                }
                WriteToParam(param.id, param.value, 128);
                break;
            }
            case DID_ADB_STATUS:
            {
                if (buf[0] == 0x00) // 关闭ADB
                {
                    AdbClose();
                }
                else if (buf[0] == 0x01) // 打开ADB
                {
                    AdbOpen();
                }
                else
                {
                    // do nothing
                }
                break;
            }
            case DID_RESET_PARAMS:
            {
                system("rm -f /sdcard/parameters/paramsDb* /oemdata/parameters/paramsDb* /oemdata/uds_did_store.db* /mnt/sdcard/dtcParameters/uds_did_store.db* && sync");
                break;
            }
            case DID_LOCAL_SLEEP_STATUS:
            case DID_LOCAL_WAKEUP_STATUS:
            {
                /* len 上层 didRegistry.c 已按 min=max=7 校验过，这里再兜底一次并做内容合法性校验 */
                if (!CheckLocalDiagStoredDataFormat(buf, len))
                {
                    rc = NRC_REQUEST_OUT_OF_RANGE_E;
                }
                break;
            }
        #endif
            default:
            {
                // rc = NRC_REQUEST_OUT_OF_RANGE_E;
                break;
            }
        }
    #if 0
        if (did == DID_RESET_PARAMS)
        {
            break;
        }
    #endif
        (void)UdsDidWriteForOwner(did, buf, len);
    } while (0);

    return rc;
}
#if 1
UdsNrc_e Uds31ServerCommFun(uint16_t did, uint8_t subFunc, const uint8_t *in, uint16_t inLen, uint8_t *out, uint16_t *io_len, gboolean *pReturnStatus)
{
    LOG_Debug("func:%s recv did: 0x%04x, subFunc: 0x%02x", __func__, did, subFunc);
    UdsNrc_e rc = NRC_NONE_E;

    do
    {
        // DDR检测 (0x1301)
        if (did == DID_DDR_TEST)
        {
            if (subFunc == 0x01) // 启动DDR检测
            {
                LOG_Debug("DDR Test: Start detection");
                int sc = system(CMD_DDR_SET);
                if (sc == -1)
                {
                    LOG_Debug("system() failed for DDR set cmd");
                    *pReturnStatus = FALSE;
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }

                // reboot 应该放在返回结果之后重启
                // 重启动作由MCU来做
                // LOG_Debug("Running: %s", CMD_REBOOT);
                // system(CMD_REBOOT);
                *pReturnStatus = TRUE;
                *io_len = 0;
                rc = NRC_NONE_E;
                LOG_Debug("DDR Test: Detection started successfully");
                break;

            }
            else if (subFunc == 0x03) // 读取结果
            {
                // read ddr test result
                LOG_Debug("Running: %s", CMD_DDR_READ);
                char outBuf[1024];
                int sc = RunCommandAndGetOutput(CMD_DDR_READ, outBuf, sizeof(outBuf));
                if (sc != 0)
                {
                    LOG_Debug("Run DDR read cmd failed");
                    *pReturnStatus = FALSE;
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }

                uint8_t parsed = 1U; /* 保守先置故障 */
                int pr = ParseDdrResultFromOutput(outBuf, &parsed);
                if (pr != 0)
                {
                    LOG_Debug("Parse read output failed, fallback to fault=1");
                    *pReturnStatus = FALSE;
                    rc = NRC_GENERAL_REJECT_E;
                    break;
                }

                // 更新DDR状态
                UpdateDdrResultRam(parsed);

                // 返回结果：0=正常，1=故障
                out[0] = parsed;
                *io_len = 1;
                *pReturnStatus = TRUE;
                rc = NRC_NONE_E;
                LOG_Debug("DDR Test: Result = %d", parsed);
                break;
            }
            else
            {
                LOG_Debug("DDR Test: Invalid subFunc 0x%02x", subFunc);
                *pReturnStatus = FALSE;
                *io_len = 0;
                rc = NRC_REQUEST_OUT_OF_RANGE_E;
                break;
            }
        }        
        
    #if 0
        else if (did == DID_31_RUN_AND_READ_CABLE_DIAGNOSIS) // 读取线缆诊断结果
        {
            /* 非以太网版本，返回不支持 */
            if (IsEthernetVersion() == FALSE) {
                LOG_Debug("Non-ethernet version, cable diagnosis not supported");
                *pReturnStatus = FALSE;
                rc = NRC_REQUEST_OUT_OF_RANGE_E;
                break;
            }

            unsigned int ifindex = if_nametoindex(PHY_IFNAME);
            if (ifindex == 0) {
                LOG_Debug("if_nametoindex failed");
                *pReturnStatus = FALSE;
                rc = NRC_GENERAL_REJECT_E;
                break;
            }
            LOG_Debug("Interface %s index = %u\n", PHY_IFNAME, ifindex);
            out[0] = (uint8_t)ifindex;

            cable_diag_result_t cable = yt8010an_get_cable_status(PHY_IFNAME);
            if (cable.status == CABLE_NORMAL) // 正常
            {
                out[1] = 0x00;
            }
            else if (cable.status == CABLE_OPEN) // 线缆开路
            {
                out[1] = 0x01;
            }
            else if (cable.status == CABLE_SHORT) // 线缆短路
            {
                out[1] = 0x02;
            }
            else // 线缆未知 无效值
            {
                out[1] = 0x03;
            }
            *io_len = 2;
            *pReturnStatus = TRUE;
            rc = NRC_NONE_E;
            break;
        }
        else if (did == DID_31_SET_AND_READ_ETH_TEST_MODE)   // 设置&读取ETH测试模式
        {
            // todo
        }
        else if (did == DID_31_SET_AND_READ_ETH_MODE)  // 设置&读取ETH通信模式
        {
            // todo
        }
        else if (did == DID_31_READ_ETH_TRANS_STATUS)  // 以太网收发器通信状态读取
        {
            /* 非以太网版本，返回不支持 */
            if (IsEthernetVersion() == FALSE) {
                LOG_Debug("Non-ethernet version, eth transceiver status not supported");
                *pReturnStatus = FALSE;
                rc = NRC_REQUEST_OUT_OF_RANGE_E;
                break;
            }

            EthStatusInfo_t info = {0};
            (void)GetEthTransceiverStatus(PHY_IFNAME, &info);
            LOG_Debug("Ethernet Index       : %u\n", info.ethIndex);
            LOG_Debug("Link State           : 0x%02X (%s)\n", info.linkState, info.linkState == 0x00 ? "Link Up" : "Link Down");
            LOG_Debug("Current SQI Level    : 0x%02X\n", info.currentSQI);
            LOG_Debug("Worst-case SQI Level : 0x%02X\n", info.worstSQI);
            out[0] = info.ethIndex;
            out[1] = info.linkState;
            out[2] = info.currentSQI;
            out[3] = info.worstSQI;
            *io_len = 4;
            *pReturnStatus = TRUE;
            rc = NRC_NONE_E;
            break;
        }
    #endif
    } while (0);

    LOG_Debug("func:%s return rc: 0x%02x", __func__, rc);
    return rc;
}
#endif
bool ParseUint32FromAscii(const uint8_t *val_buf, size_t val_len, uint32_t minv, uint32_t maxv, uint32_t *out)
{
    if (!val_buf || !out) return false;

    /* 拷到小缓冲，确保以 '\0' 结尾再解析 */
    char tmp[64] = {0};
    size_t n = (val_len < sizeof(tmp) - 1) ? val_len : (sizeof(tmp) - 1);
    memcpy(tmp, val_buf, n);
    tmp[sizeof(tmp) - 1] = '\0';

    /* 去掉前后空白 */
    char *s = tmp;
    while (*s && isspace((unsigned char)*s)) ++s;
    if (*s == '\0') return false;

    errno = 0;
    char *endp = NULL;
    unsigned long v = strtoul(s, &endp, 10);

    /* 允许尾随空白，但不能有其它字符 */
    while (endp && *endp && isspace((unsigned char)*endp)) ++endp;
    if (errno == ERANGE || endp == NULL || *endp != '\0') return false;
    if (v < minv || v > maxv) return false;

    *out = (uint32_t)v;
    return true;
}

/* 工具：把无符号整形写成十进制 ASCII 到 param.value，自动补 '\0' */
void UintToAsciiIntoParamValue(uint8_t dst[], size_t dstSize, uint32_t v)
{
    /* ParamInfo_t.value 是定长缓冲，这里安全格式化并截断 */
    if (dst == NULL || dstSize == 0) return;
    char tmp[32] = {0};
    (void)snprintf(tmp, sizeof(tmp), "%u", (unsigned)v);
    size_t n = strlen(tmp);
    if (n >= dstSize) n = dstSize - 1;
    memset(dst, 0, dstSize);
    memcpy(dst, tmp, n);
}

/**
 * @brief AsciiHexBytesToU32
 *
 * @purpose
 *   将 ASCII 数字字节数组（每个元素应在 0x30~0x39）转换为十进制数值。
 *   例如: [0x31,0x32,0x34,0x34,0x33] => 12443
 *
 * @param inBytes   输入字节数组指针（ASCII 数字）
 * @param inLen     输入长度
 * @param outValue  输出的数值
 *
 * @return ConvRet_t
 *   ConvRetOk        - 成功
 *   ConvRetParamErr  - 入参为空或长度为 0
 *   ConvRetFormatErr - 存在非 '0'~'9' 字符
 *   ConvRetOverflow  - 结果超出 uint32_t
 */
ConvRet_t AsciiHexBytesToU32(const uint8_t *inBytes, uint32_t inLen, uint32_t *outValue)
{
    ConvRet_t ret = ConvRetOk;
    uint32_t value = 0U;
    uint32_t i = 0U;

    if ((inBytes == NULL) || (outValue == NULL) || (inLen == 0U))
    {
        ret = ConvRetParamErr;
    }
    else
    {
        for (i = 0U; i < inLen; i++)
        {
            uint8_t ch = inBytes[i];
            uint32_t digit = 0U;

            if ((ch < (uint8_t)'0') || (ch > (uint8_t)'9'))
            {
                ret = ConvRetFormatErr;
                break;
            }

            digit = (uint32_t)(ch - (uint8_t)'0');

            /* 防溢出：value*10 + digit <= UINT32_MAX */
            if (value > (UINT32_MAX / 10U))
            {
                ret = ConvRetOverflow;
                break;
            }
            if ((value == (UINT32_MAX / 10U)) && (digit > (UINT32_MAX % 10U)))
            {
                ret = ConvRetOverflow;
                break;
            }

            value = (value * 10U) + digit;
        }
    }

    if (ret == ConvRetOk)
    {
        *outValue = value;
    }

    LOG_Debug("func:%s return ret: %d", __func__, ret);
    return ret;
}
