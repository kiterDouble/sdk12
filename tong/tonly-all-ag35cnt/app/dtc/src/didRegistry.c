/* ========================================================================== */
/*  文件名称  : didRegistry.c                                                 */
/*  模块名称  : UDS DID 注册表与持久化访问                                     */
/*  版本      : v1.1.0                                                         */
/*  作者      : <SIRUN HF / Jaxsen>                                           */
/*  创建日期  : 2025-09-01                                                     */
/* -------------------------------------------------------------------------- */
/*  摘要说明  :                                                                */
/*    本模块维护“DID→元数据（权限/长度）→固定 db_id”的注册表，并提供基于 DID 的     */
/*    读写接口。底层持久化以固定 db_id 作为唯一索引，确保数据库中主键稳定、可进化。   */
/*                                                                            */
/*  设计要点  :                                                                */
/*    - DID 与 db_id 一一绑定：db_id 从 1 起分配，今后不得更改，以保证历史数据可复用。  */
/*    - 冷启动流程：打开 DB → 校验建表 → 绑定 did（约束） → 默认值下沉（缺失项） → 回灌。*/
/*    - RAM 缓存：读写走 RAM 槽；写成功后落库并更新 RAM，读从 RAM 返回以减少 IO。       */
/*    - 默认值：集中在 didDefaults.c 中，通过 DefaultGetByDid() 按 DID 提供。            */
/*    - 回调优先：若表项存在 read_cb / write_cb，则优先走回调；否则走 RAM/DB 默认路径。  */
/*                                                                            */
/*  MISRA 注意 :                                                               */
/*    - 所有外部可见 API 做空指针检查；                                         */
/*    - 静态对象/常量使用 file-scope static，并避免可变全局状态逃逸；                */
/*    - switch 语句使用完整的 default 分支；                                    */
/*    - 辅助函数声明为 static，内部链接可见性；                                  */
/*    - 返回码使用 uds_types.h 中的 NRC_* 子集，不与布尔返回混用。                  */
/*                                                                            */
/*  变更记录  :                                                                */
/*    v1.0.0  初版                                                             */
/*    v1.1.0  新增回调优先逻辑与 ENTRY/ENTRY_CB 宏                              */
/* ========================================================================== */

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <glib.h>
#include "sr_log.h"
#include "pm_server_api.h"
#include "didRegistry.h"
#include "udsCfg.h"
#include "udsTypes.h"
#include "dtcRpcApi.h"
#include "dbDriver.h"
#include "didDefaults.h"
#include "udsCallbackEntry.h"
// #include "gacPki.h"
#include "net_service_api.h"
#include "dbBackupService.h"

/* ========================================================================== */
/*                              内部类型与变量                                */
/* ========================================================================== */

/* ------------------------------- RAM 槽 ------------------------------------ */
/* 说明：为每个表项保留一段最大 UDS_MAX_DID_PAYLOAD 的缓存及其当前长度。 */
typedef struct
{
    uint16_t len;
    uint8_t  data[UDS_MAX_DID_PAYLOAD];
} uds_did_slot_t;

/* ------------------------------ ENTRY 宏 ----------------------------------- */
/* 你的 uds_did_entry_t 含 read_cb / write_cb 字段，因此将默认回调置为 NULL。 */
#ifndef ENTRY
#define ENTRY(_did, _name, _acc, _min, _max) \
    {.did = (_did), .name = (_name), .access = (_acc), \
      .min_len = (_min), .max_len = (_max), .read_cb = NULL, .write_cb = NULL, .control_cb = NULL }
#endif

/* 可选：如需在表里直接挂回调，用这个 8 参数宏 */
#ifndef ENTRY_CB
#define ENTRY_CB(_did, _name, _acc, _min, _max, _r_cb, _w_cb,  _c_cb) \
    {.did = (_did), .name = (_name), .access = (_acc), \
      .min_len = (_min), .max_len = (_max), .read_cb = (_r_cb), .write_cb = (_w_cb), .control_cb = (_c_cb) }
#endif

/* ------------------------------ 注册表 ------------------------------------ */
/* 重要：db_id 为数据库固定主键，务必保证线上版本后不再变动对应关系。 */
static const UdsDidEntry_t g_uds_did_table[] =
    {
    #if 1 //SQ - 陕汽项目参数定义
        ENTRY_CB(DID_VIN, "VIN", UDS_ACCESS_RW, 17u, 17u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),        
        // EOL检测项DID注册表
        ENTRY_CB(DID_KL15_STATUS, "KL15 Status", UDS_ACCESS_R, 1u, 1u, UdsReadCommFunc, NULL, NULL),
        ENTRY_CB(DID_KL30_VOLTAGE, "KL30 Voltage", UDS_ACCESS_R, 2u, 2u, UdsReadCommFunc, NULL, NULL),
        ENTRY_CB(DID_ICCID, "ICCID", UDS_ACCESS_R, 20u, 20u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_IMSI, "IMSI", UDS_ACCESS_R, 15u, 15u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_IMEI, "IMEI", UDS_ACCESS_R, 15u, 15u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_APN1, "APN1", UDS_ACCESS_RW, 32u, 32u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_APN2, "APN2", UDS_ACCESS_RW, 32u, 32u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_APN3, "APN3", UDS_ACCESS_RW, 32u, 32u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_CSQ, "CSQ Signal Strength", UDS_ACCESS_R, 1u, 1u, UdsReadCommFunc, NULL, NULL),
        ENTRY_CB(DID_GNSS_STATUS, "GNSS Status", UDS_ACCESS_R, 1u, 1u, UdsReadCommFunc, NULL, NULL),
        ENTRY_CB(DID_EMMC_STATE, "EMMC State", UDS_ACCESS_R, 1u, 1u, UdsReadCommFunc, NULL, NULL),        
        ENTRY_CB(DID_FAST_SLEEP_MODE, "Fast Sleep Mode", UDS_ACCESS_RW, 1u, 1u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_SW_VERSION_EXTERNAL, "SW Version External", UDS_ACCESS_R, 13u, 13u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_4G_ANTENNA_STATUS, "4G Antenna Status", UDS_ACCESS_R, 1u, 1u, UdsReadCommFunc, NULL, NULL),
        ENTRY_CB(DID_GNSS_ANTENNA_STATUS, "GNSS Antenna Status", UDS_ACCESS_R, 1u, 1u, UdsReadCommFunc, NULL, NULL),
        ENTRY_CB(DID_HW_VERSION, "HW Version", UDS_ACCESS_R, 17u, 17u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_TBOX_SN, "Tbox SN", UDS_ACCESS_RW, 45u, 45u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        
         // 0x31 server        
        // ENTRY_CB(DID_31_RUN_AND_READ_CABLE_DIAGNOSIS, "run and read cable diagnosis", UDS_ACCESS_C, 2, 2, NULL, NULL, Uds31ServerCommFun)
        ENTRY_CB(DID_DDR_TEST, "ddr test", UDS_ACCESS_C, 1, 1, NULL, NULL, Uds31ServerCommFun),
        // EOL ICCID0/1/2
        ENTRY_CB(DID_ICCID0, "ICCID0", UDS_ACCESS_R, 20u, 20u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_ICCID1, "ICCID1", UDS_ACCESS_R, 20u, 20u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_ICCID2, "ICCID2", UDS_ACCESS_R, 20u, 20u, UdsReadFromParamsDb, NULL, NULL)
    #else
        ENTRY(DID_VEHICLE_MODE, "Vehicle mode", UDS_ACCESS_R, 1u, 1u),
        ENTRY_CB(DID_TRANSPORT_MODE, "Transport mode", UDS_ACCESS_RW, 1u, 1u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY(DID_USAGE_MODE, "Usage mode", UDS_ACCESS_R, 1u, 1u),
        ENTRY_CB(DID_KEY_STATUS, "Key status", UDS_ACCESS_R, 1u, 1u, UdsReadFromCanSignal, NULL, NULL),
        ENTRY_CB(DID_MANUFACTORY_MODE, "Manufactory mode", UDS_ACCESS_RW, 1u, 1u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        // ENTRY_CB(DID_DTC_CONTROL_STATE, "DTC Setting control state", UDS_ACCESS_RW, 1u, 12u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY(DID_REPROGRAMMING_COUNTER, "Reprogramming Counter", UDS_ACCESS_RW, 2u, 2u),
        ENTRY(DID_REPROGRAMMING_ATTEMPT_COUNTER, "Reprogramming Attempt Counter", UDS_ACCESS_RW, 2u, 2u),
        ENTRY(DID_GAC_DIAG_PARAM_VERSION, "GAC Diagnostic Parameter Table Version", UDS_ACCESS_R, 4u, 4u),
        ENTRY_CB(DID_GAC_ECU_HW_VER, "GAC ECU Hardware Version", UDS_ACCESS_R, 17u, 17u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY(DID_BOOT_SW_ID, "Boot Software Identification", UDS_ACCESS_R, 17u, 17u),
        ENTRY(DID_ACTIVE_SESSION, "activeDiagnosticSessionDataIdentifier", UDS_ACCESS_R, 1u, 1u),
        ENTRY_CB(DID_GAC_SPARE_PART_NUM, "GAC Spare Part Number", UDS_ACCESS_R, 14u, 14u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_GAC_ECU_SW_VER, "GAC ECU Software Version", UDS_ACCESS_R, 17u, 17u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_GAC_ECU_SERIAL_NUM, "GAC ECUSerialNumberDataIdentifier", UDS_ACCESS_R, 45u, 45u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_GAC_KIT_ASSEMBLY_PART_NUM, "GAC Kit Assembly Part Number", UDS_ACCESS_R, 14u, 14u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_VIN, "VIN", UDS_ACCESS_RW, 17u, 17u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_SUPPLIER_HW_VER, "Supplier ECU Hardware Version", UDS_ACCESS_R, 6u, 6u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_SUPPLIER_SW_VER, "Supplier ECU Software Version", UDS_ACCESS_R, 6u, 6u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY(DID_POWER_VOLTAGE, "Power Voltage", UDS_ACCESS_R, 1u, 1u),
        ENTRY_CB(DID_SYSTEM_SUPPLIER_ID, "System Supplier Identifier", UDS_ACCESS_R, 9u, 9u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_CELL_STATUS_4G_3G_2G, "4G/3G/2G Status", UDS_ACCESS_R, 2u, 2u, UdsReadCommFunc, NULL, NULL),
        ENTRY(DID_GPS_STATUS, "GPS Status", UDS_ACCESS_R, 2u, 2u),
        ENTRY(DID_CURRENT_TIME, "Current Time (BCD)", UDS_ACCESS_R, 7u, 7u),
        ENTRY(DID_BCALL_ECALL_KEY_STATUS, "B/E-call Key Status", UDS_ACCESS_R, 1u, 1u),
        ENTRY_CB(DID_TSP_DOMAIN_NAME, "tsp domain name", UDS_ACCESS_RW, 50u, 50u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_VEHICLE_SPEED, "Vehicle Speed", UDS_ACCESS_R, 2u, 2u, UdsReadFromCanSignal, NULL, NULL),
        ENTRY_CB(DID_MOTOR_SPEED, "Motor Speed", UDS_ACCESS_R, 2u, 2u, UdsReadFromCanSignal, NULL, NULL),
        ENTRY_CB(DID_IMEI, "IMEI", UDS_ACCESS_R, 15u, 15u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_IMSI, "IMSI", UDS_ACCESS_R, 15u, 15u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_ICCID, "ICCID", UDS_ACCESS_R, 20u, 20u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_RTC_WAKEUP_INTERVAL, "RTC Wakeup Interval", UDS_ACCESS_RW, 2u, 2u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_APN1, "APN1", UDS_ACCESS_RW, 32u, 32u, UdsReadFromFile, UdsWriteToFile, NULL),
        ENTRY(DID_IP1, "IP1 Address", UDS_ACCESS_RW, 16u, 16u),
        ENTRY_CB(DID_PORT1, "port1", UDS_ACCESS_RW, 8u, 8u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_ECALL_DIAL1, "E-Call Dial Number 1", UDS_ACCESS_RW, 14u, 14u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_BCALL_DIAL1, "B-Call Dial Number 1", UDS_ACCESS_RW, 14u, 14u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY(DID_OFFICIAL_SERVICE, "Official Service Number", UDS_ACCESS_RW, 14u, 14u),
        ENTRY(DID_EMERGENCY_AFTER_SALES, "Emergency After-Sales Number", UDS_ACCESS_RW, 14u, 14u),
        ENTRY(DID_TBOX_PHONE_NUMBER, "TBox Phone Number", UDS_ACCESS_RW, 14u, 14u),
        ENTRY_CB(DID_APN2, "APN2", UDS_ACCESS_RW, 32u, 32u, UdsReadFromFile, UdsWriteToFile, NULL),
        ENTRY(DID_4G_NETWORK_STATUS, "4G Network Status", UDS_ACCESS_R, 4u, 4u),
        ENTRY_CB(DID_TSP_CONN_TIMEOUT, "TSP connect Timeout", UDS_ACCESS_RW, 1u, 1u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_TSP_TIMEOUT_FOR_FLOWCHART, "TSP Flowchart Timeout", UDS_ACCESS_RW, 1u, 1u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_TSP_ACK_TIME, "TSP Ack Time", UDS_ACCESS_RW, 1u, 1u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY(DID_IP3, "IP3 Address", UDS_ACCESS_RW, 16u, 16u),
        ENTRY(DID_LONG_CONNECTION_CYCLE, "TSP Long connect cycle", UDS_ACCESS_RW, 8u, 8u),
        ENTRY_CB(DID_ETHERNET_CONFIG_INFO, "Ethernet Config Info", UDS_ACCESS_RW, 4u, 4u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_PUBLIC_TSP_DOMAIN_NAME, "public tsp domain name", UDS_ACCESS_RW, 50u, 50u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY(DID_APN1_TYPE, "APN1 type", UDS_ACCESS_RW, 1u, 1u),
        ENTRY(DID_APN2_TYPE, "APN2 type", UDS_ACCESS_RW, 1u, 1u),
        ENTRY_CB(DID_EMMC_STATE, "EMMC State", UDS_ACCESS_R, 1u, 1u, UdsReadCommFunc, NULL, NULL),
        ENTRY_CB(DID_4G_MODULE_FW_VER, "4G module firmware version", UDS_ACCESS_R, 20u, 20u, UdsReadCommFunc, NULL, NULL),
        ENTRY(DID_4G_MODULE_HW_VER, "4G module hardware version", UDS_ACCESS_R, 20u, 20u),
        ENTRY(DID_ETHERNET_ID, "Ethernet ID", UDS_ACCESS_R, 5u, 5u),
        ENTRY_CB(DID_TSP_PLATFORM_CONN_SETTING, "TSP Platform Connection Setting", UDS_ACCESS_RW, 1u, 1u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_TBOX_RUNNING_MODE, "TBox Running Mode", UDS_ACCESS_RW, 1u, 1u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_4G_HW_RESET_TIME, "4G HW Reset Time", UDS_ACCESS_RW, 1u, 1u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_4G_HW_RESET_COUNT, "4G HW Reset Count", UDS_ACCESS_R, 1u, 1u, UdsReadFromParamsDb, NULL, NULL),
        ENTRY_CB(DID_PKI_CERT_STATUS, "PKI Cert Status", UDS_ACCESS_R, 1u, 1u, UdsReadCommFunc, NULL, NULL),
        ENTRY_CB(DID_PKI_DOMAIN_NAME, "PKI Domain Name", UDS_ACCESS_RW, 50u, 50u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_PKI_PORT, "PKI Port", UDS_ACCESS_RW, 8u, 8u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_PKI_MAINTENANCE_CYCLE, "PKI CYCLE", UDS_ACCESS_RW, 1u, 1u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_LOG_DOMAIN_NAME, "Log Domain Name", UDS_ACCESS_RW, 100u, 100u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_LOG_CONN, "Log Conn", UDS_ACCESS_RW, 16u, 16u, NULL, UdsWriteCommFunc, NULL),
        ENTRY_CB(DID_LOG_ACCOUNT, "Log Account", UDS_ACCESS_RW, 50u, 50u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_LOG_PASSWORD, "Log Password", UDS_ACCESS_RW, 50u, 50u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_LOG_INTERFACE, "Log Interface", UDS_ACCESS_RW, 1u, 1u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_LOG_DOMAIN_FRONT, "Log Domain Front", UDS_ACCESS_RW, 50u, 50u, NULL, UdsWriteCommFunc, NULL),
        ENTRY_CB(DID_LOG_DOMAIN_LATER, "Log Domain Later", UDS_ACCESS_RW, 50u, 50u, NULL, UdsWriteCommFunc, NULL),
        ENTRY_CB(DID_LOG_PATH, "Log Path", UDS_ACCESS_RW, 256u, 256u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_LOG_PATH_P1, "Log path 1", UDS_ACCESS_RW, 64u, 64u, NULL, UdsWriteCommFunc, NULL),
        ENTRY_CB(DID_LOG_PATH_P2, "Log path 2", UDS_ACCESS_RW, 64u, 64u, NULL, UdsWriteCommFunc, NULL),
        ENTRY_CB(DID_LOG_PATH_P3, "Log path 3", UDS_ACCESS_RW, 64u, 64u, NULL, UdsWriteCommFunc, NULL),
        ENTRY_CB(DID_LOG_PATH_P4, "Log path 4", UDS_ACCESS_RW, 64u, 64u, NULL, UdsWriteCommFunc, NULL),
        ENTRY_CB(DID_MQTT_PORT_PUBLIC, "MQTT public port", UDS_ACCESS_RW, 8u, 8u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_APN3, "APN3", UDS_ACCESS_RW, 32u, 32u, UdsReadFromFile, UdsWriteToFile, NULL),
        ENTRY(DID_APN3_TYPE, "APN3 type", UDS_ACCESS_RW, 1u, 1u),
        ENTRY_CB(DID_SENSITIVE_DATA_SET, "Sensitive Data Set", UDS_ACCESS_RW, 16u, 16u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY(DID_OTA_RTC_WAKEUP_PARAM, "OTA RTC Wakeup Param", UDS_ACCESS_RW, 8u, 8u),
        ENTRY_CB(DID_BURYING_POINT_SWITCH, "Burying Point Switch", UDS_ACCESS_RW, 4u, 4u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_TSP_PLATFORM_CONN_STATUS, "tsp conn status", UDS_ACCESS_R, 1u, 1u, UdsReadCommFunc, NULL, NULL),
        ENTRY_CB(DID_CONN_TEST_ADDR, "conn test addr", UDS_ACCESS_RW, 50u, 50u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_BURIED_POINT_DOMAIN, "buried point domain", UDS_ACCESS_RW, 128u, 128u, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY_CB(DID_BURIED_POINT_DOMAIN_1, "buried point domain front", UDS_ACCESS_RW, 64u, 64u, NULL, UdsWriteCommFunc, NULL),
        ENTRY_CB(DID_BURIED_POINT_DOMAIN_2, "buried point domain later", UDS_ACCESS_RW, 64u, 64u, NULL, UdsWriteCommFunc, NULL),
        ENTRY(DID_GNSS_GALAXY, "gnss galaxy", UDS_ACCESS_RW, 1u, 1u),
        ENTRY(DID_OTA_UPDATE_TIME, "ota update time", UDS_ACCESS_RW, 6u, 6u),
        ENTRY(DID_OTA_DOMAIN_NAME, "ota domain name", UDS_ACCESS_RW, 50u, 50u),
        ENTRY(DID_OTA_PORT, "ota port", UDS_ACCESS_RW, 8u, 8u),
        ENTRY(DID_OTA_SW_VER_ACQUIRE_TIME, "ota sw ver acquire time", UDS_ACCESS_RW, 1u, 1u),
        ENTRY(DID_OTA_MODE, "ota mode", UDS_ACCESS_RW, 1u, 1u),
        ENTRY(DID_OTA_RECOVERY_STATUS, "ota recovery status", UDS_ACCESS_RW, 1u, 1u),
        ENTRY(DID_OTA_PART_SYNC_STATUS, "ota part sync status", UDS_ACCESS_RW, 1u, 1u),
        ENTRY_CB(DID_FUNCTION_CONFIG, "function config", UDS_ACCESS_RWC, 64u, 64u, NULL, NULL, Uds31ServerCommFun), // DDR 测试
        ENTRY_CB(DID_LOG_PORT, "log port", UDS_ACCESS_RW, 8, 8, UdsReadFromParamsDb, UdsWriteToParamsDb, NULL),
        ENTRY(DID_DELETE_STORED_FILES, "delete stored files", UDS_ACCESS_RW, 1, 1),
        ENTRY(DID_PKI_APPLY, "pki apply", UDS_ACCESS_RW, 1, 1),
        ENTRY_CB(DID_ADB_STATUS, "adb status", UDS_ACCESS_RW, 1, 1, NULL, UdsWriteCommFunc, NULL),
        ENTRY(DID_4G_APP_VER, "4g app ver", UDS_ACCESS_RW, 17, 17),
        ENTRY_CB(DID_PKI_SDK_VER, "pki sdk ver", UDS_ACCESS_R, 5, 5, UdsReadCommFunc, NULL, NULL),
        ENTRY_CB(DID_RESET_PARAMS, "reset status", UDS_ACCESS_RW, 1, 1, NULL, UdsWriteCommFunc, NULL),
        ENTRY_CB(DID_LOCAL_SLEEP_STATUS, "The local is not sleep", UDS_ACCESS_RW, 7, 7, NULL, UdsWriteCommFunc, NULL),
        ENTRY_CB(DID_LOCAL_WAKEUP_STATUS, "The local wake up is frequent", UDS_ACCESS_RW, 7, 7, NULL, UdsWriteCommFunc, NULL),
        // 0x31 server
        ENTRY_CB(DID_31_READ_ETH_TRANS_STATUS, "read eth trans status", UDS_ACCESS_C, 4, 4, NULL, NULL, Uds31ServerCommFun),
        ENTRY_CB(DID_31_RUN_AND_READ_CABLE_DIAGNOSIS, "run and read cable diagnosis", UDS_ACCESS_C, 2, 2, NULL, NULL, Uds31ServerCommFun)
        // ENTRY_CB(DID_31_DDR_TEST, "ddr test", UDS_ACCESS_C, 1, 1, NULL, NULL, Uds31ServerCommFun)
    #endif
};

#define UDS_DID_TABLE_COUNT   ((uint32_t)(sizeof(g_uds_did_table) / sizeof(g_uds_did_table[0])))

/* RAM 槽：与注册表一一对应 */
static uds_did_slot_t g_uds_store[UDS_DID_TABLE_COUNT];

/* ========================================================================== */
/*                              内部函数声明                                  */
/* ========================================================================== */
static int32_t UdsFindDidIndex(UDS_DID_IDS_e did);
static uint8_t CheckLenRange(const UdsDidEntry_t *e, uint16_t len);
static void    UdsDidInit(void);

int IsMountPointMounted(const char *mountPoint)
{
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) return 0;

    char line[1024];
    int mounted = 0;
    while (fgets(line, sizeof(line), fp)) {
        // /proc/mounts 格式: <src> <target> <fs> <opts> ...
        // 我们只要第二列 target
        char src[256], target[256], fstype[64], opts[256];
        if (sscanf(line, "%255s %255s %63s %255s", src, target, fstype, opts) == 4) {
            if (strcmp(target, mountPoint) == 0) {
                mounted = 1;
                break;
            }
        }
    }
    fclose(fp);
    return mounted;
}

int RemoveAllFilesInDir(const char *path)
{
    DIR *dir = opendir(path);
    if (!dir) return -1;

    struct dirent *entry;
    char fullPath[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (lstat(fullPath, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            if (RemoveAllFilesInDir(fullPath) == 0) rmdir(fullPath);
        } else {
            remove(fullPath);
        }
    }
    closedir(dir);
    return 0;
}
#if 0
static int PkiSync(void)
{
    return GacPkiDefaultSync();
}

static int bcd_to_decimal(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

/* 计算 RTC 唤醒时间与当前时间的差值（单位：秒） */
static int32_t CalcRtcWakeupDiffSeconds(const uint8_t *data)
{
    if (data == NULL) {
        return 0;
    }

    /* ---- 解析字节结构 (BCD 转换) ---- */
    int year  = bcd_to_decimal(data[0]) * 100 + bcd_to_decimal(data[1]);
    int month = bcd_to_decimal(data[2]);
    int day   = bcd_to_decimal(data[3]);
    int hour  = bcd_to_decimal(data[4]);
    int min   = bcd_to_decimal(data[5]);
    int sec   = bcd_to_decimal(data[6]);

    /* ---- 构造 time_t ---- */
    struct tm rtc_tm = {0};
    rtc_tm.tm_year = year - 1900;  // struct tm 从1900开始计
    rtc_tm.tm_mon  = month - 1;    // struct tm 的月从0开始
    rtc_tm.tm_mday = day;
    rtc_tm.tm_hour = hour;
    rtc_tm.tm_min  = min;
    rtc_tm.tm_sec  = sec;

    time_t rtc_time = mktime(&rtc_tm);
    if (rtc_time == (time_t)-1) {
        return 0;
    }

    /* ---- 获取当前系统时间 ---- */
    time_t now = time(NULL);
    if (now == (time_t)-1) {
        return 0;
    }

    /* ---- 差值（单位：秒）---- */
    int32_t diff = (int32_t)difftime(rtc_time, now);
    return diff;
}
#endif
/* ========================================================================== */
/*                              工具/查询函数                                  */
/* ========================================================================== */

static int32_t UdsFindDidIndex(UDS_DID_IDS_e did)
{
    uint32_t i;

    for (i = 0u; i < UDS_DID_TABLE_COUNT; i++)
    {
        if (g_uds_did_table[i].did == did)
        {
            return (int32_t)i;
        }
    }
    return -1;
}

const char * UdsDidName(UDS_DID_IDS_e did)
{
    int32_t idx = UdsFindDidIndex(did);
    return (idx >= 0) ? g_uds_did_table[(uint32_t)idx].name : "UNKNOWN_DID";
}

const UdsDidEntry_t * UdsGetDidRegistry(uint32_t *count)
{
    if (count != NULL)
    {
        *count = UDS_DID_TABLE_COUNT;
    }
    return g_uds_did_table;
}

static uint8_t CheckLenRange(const UdsDidEntry_t *e, uint16_t len)
{
    if ((len < e->min_len) || (len > e->max_len))
    {
        return (uint8_t)NRC_INCORRECT_MESSAGE_LENGTH_E;
    }
    return (uint8_t)NRC_NONE_E;
}

static void UdsDidInit(void)
{
    uint32_t i;

    for (i = 0u; i < UDS_DID_TABLE_COUNT; i++)
    {
        (void)memset(g_uds_store[i].data, 0, (size_t)UDS_MAX_DID_PAYLOAD);
        g_uds_store[i].len = (g_uds_did_table[i].min_len == g_uds_did_table[i].max_len)
                             ? g_uds_did_table[i].max_len
                             : g_uds_did_table[i].min_len;
    }
}

/* ========================================================================== */
/*                              初始化/关闭                                    */
/* ========================================================================== */

int UdsStorageInit(const char *path)
{
    uint32_t i;
    uint8_t needWriteFlag = 0;

    /* STEP 1: 打开 DB 与建表 */
    if (DbOpen(path) != 0)
    {
        return -1;
    }
    if (DbEnsureSchema() != (int)NRC_NONE_E)
    {
        DbClose();
        return -1;
    }

    /* STEP 2: 初始化 RAM 槽 */
    UdsDidInit();

    /* STEP 4: 冷启动默认值下沉（仅对 DB 中不存在的 id） */
    for (i = 0u; i < UDS_DID_TABLE_COUNT; i++)
    {
        const UdsDidEntry_t *e = &g_uds_did_table[i];
        uint8_t  tmp[UDS_MAX_DID_PAYLOAD];
        uint16_t cap = (uint16_t)sizeof(tmp);
        int      rc;

        rc = DbReadValueByDid(e->did, tmp, &cap);
        if (rc == (int)NRC_REQUEST_OUT_OF_RANGE_E)
        {
            const uint8_t *p = NULL;
            uint16_t       l = 0u;

            if (DefaultGetByDid(e->did, &p, &l) == 0)
            {
                /* 无显式默认 → 使用 min_len 个 0 */
                static uint8_t zeros[UDS_MAX_DID_PAYLOAD];
                (void)memset(zeros, 0, sizeof(zeros));
                p = zeros;
                l = e->min_len;
            }

            /* 限幅到 [min, max] */
            if (l < e->min_len) { l = e->min_len; }
            if (l > e->max_len) { l = e->max_len; }

            (void)DbUpsertValueByDidEx(e->did, p, l, E_DB_BAK_UPDATE_DISABLE);
        }
        else
        {
            /* 已存在或出错：回灌阶段处理 */
        }
    }

    LOG_Debug("Start forced backup");
    DbBackupNotifyBackup();
    /* STEP 5: 从 DB 回灌 RAM 槽 */
    for (i = 0u; i < UDS_DID_TABLE_COUNT; i++)
    {
        const UdsDidEntry_t *e = &g_uds_did_table[i];
        uint8_t  tmp[UDS_MAX_DID_PAYLOAD];
        uint16_t cap = (uint16_t)sizeof(tmp);

        if (DbReadValueByDid(e->did, tmp, &cap) == (int)NRC_NONE_E)
        {
            if (cap < e->min_len) { cap = e->min_len; }
            if (cap > e->max_len) { cap = e->max_len; }

            (void)memcpy(g_uds_store[i].data, tmp, cap);
            g_uds_store[i].len = cap;
        }
        else
        {
            /* 读取失败：保持初始化值 */
        }
    }

    return 0;
}

void UdsStorageClose(void)
{
    DbClose();
}

/* ========================================================================== */
/*                                  读/写                                     */
/* ========================================================================== */

UdsNrc_e UdsDidRead(UDS_DID_IDS_e did, uint8_t *buf, uint16_t *io_len)
{
    int32_t idx;
    uint16_t need;

    /* 参数检查 */
    if ((buf == NULL) || (io_len == NULL))
    {
        return (uint8_t)NRC_INCORRECT_MESSAGE_LENGTH_E;
    }

    idx = UdsFindDidIndex(did);
    if (idx < 0)
    {
        return (uint8_t)NRC_REQUEST_OUT_OF_RANGE_E;
    }

    const UdsDidEntry_t *e = &g_uds_did_table[(uint32_t)idx];

    /* 访问权限检查（可读） */
    if ((e->access & UDS_ACCESS_R) == 0u)
    {
        return (uint8_t)NRC_REQUEST_OUT_OF_RANGE_E;
    }

    /* -------------------- 优先走回调（若已注册） -------------------- */
    if (e->read_cb != NULL)
    {
        UdsNrc_e rc = e->read_cb(did, buf, io_len);
        // 补充长度符合诊断调查表要求
        if (rc == NRC_NONE_E)
        {
            if (*io_len < e->min_len)
            {
                size_t pad = e->min_len - *io_len;
                memset(buf + *io_len, 0x00, pad);
                *io_len = e->min_len;
            }
            else if (*io_len > e->max_len)
            {
                *io_len = e->max_len;
            }
        }

        return rc;
    }

    /* -------------------- 无回调：走 RAM 缓存 -------------------- */
    need = g_uds_store[(uint32_t)idx].len;
    if (*io_len < need)
    {
        return (uint8_t)NRC_RESPONSE_TOO_LONG_E;
    }

    (void)memcpy(buf, g_uds_store[(uint32_t)idx].data, (size_t)need);
    *io_len = need;

    return (uint8_t)NRC_NONE_E;
}

UdsNrc_e UdsDidWrite(UDS_DID_IDS_e did, const uint8_t *buf, uint16_t len)
{
    int32_t idx;
    const UdsDidEntry_t *e;
    uint8_t nrc;

    LOG_Debug("UdsDidWrite: %04X", did);
    LOG_DebugHex(buf, len, "UdsDidWrite: ");

    /* 参数检查 */
    if (buf == NULL)
    {
        return (uint8_t)NRC_INCORRECT_MESSAGE_LENGTH_E;
    }

    idx = UdsFindDidIndex(did);
    if (idx < 0)
    {
        return (uint8_t)NRC_REQUEST_OUT_OF_RANGE_E;
    }

    e = &g_uds_did_table[(uint32_t)idx];

    /* 访问权限检查（可写） */
    if ((e->access & UDS_ACCESS_W) == 0u)
    {
        return (uint8_t)NRC_REQUEST_OUT_OF_RANGE_E;
    }

    /* 协议长度兜底（即便有回调也做边界校验；若你希望完全由回调自校验，可移除） */
    nrc = CheckLenRange(e, len);
    if (nrc != (uint8_t)NRC_NONE_E)
    {
        return nrc;
    }

    /* -------------------- 优先走回调（若已注册） -------------------- */
    if (e->write_cb != NULL)
    {
        /* 你的要求：有回调则直接执行并返回，不再触碰 DB/RAM */
        return e->write_cb(did, buf, len);
    }

    /* -------------------- 无回调：默认先落库，再更新 RAM -------------------- */
    if (DbUpsertValueByDid(e->did, buf, len) != (int)NRC_NONE_E)
    {
        return (uint8_t)NRC_GENERAL_REJECT_E;
    }

    (void)memcpy(g_uds_store[(uint32_t)idx].data, buf, (size_t)len);
    g_uds_store[(uint32_t)idx].len = len;
#if 0
    if (did == DID_DELETE_STORED_FILES)
    {
        // "0x11=Delete GB data 0x12=Delete EMMC data"
        if (buf[0] == 0x12)
        {
            const char *mountPoint = "/mnt/sdcard";
            if (!IsMountPointMounted(mountPoint))
            {
                LOG_Debug("SD card is not mounted at %s\n", mountPoint);
                return (uint8_t)NRC_NONE_E;
            }
            LOG_Debug("SD card mounted, clearing directory: %s\n", mountPoint);

            if (RemoveAllFilesInDir(mountPoint) == 0)
            {
                LOG_Debug("All files under %s removed successfully.\n", mountPoint);
            }
            else
            {
                LOG_Debug("Failed to clear %s\n", mountPoint);
            }
        }
        else
        {
            LOG_Debug("invalid cmd : %02x", buf[0]);
        }
    }

    else if (did == DID_PKI_APPLY)
    {
        // pki apply
        if (GacPkiGetSyncState() != 0) // 获取sync状态，如果未同步，则进行sync
        {
            if (PkiSync() != 0) // 同步
            {
                LOG_Debug("gac pki sync failed!");
                return (uint8_t)NRC_NONE_E;
            }
        }

        /* 初始化 GMTLS SSL_CTX（轮询等待 PKI 就绪，最多 3 次） */
        {
            uint8_t count = 5;
            while (count--)
            {
                int rc = GacPkiSslTryInit(3U);
                if (rc != 0)
                {
                    LOG_Error("GacPkiSslTryInit failed, retry");
                    continue;
                }
                else
                {
                    LOG_Debug("GacPkiSslTryInit success!");
                    break;
                }
            }
        }
    }

    else if (did == DID_OTA_RTC_WAKEUP_PARAM)
    {
        uint8_t pattern1[] = { 0x00, 0x00, 0xEE, 0xEE, 0xDD, 0xDD };
        uint8_t pattern2[] = { 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF };
        if (memcmp(buf, pattern1, sizeof(pattern1)) == 0 || memcmp(buf, pattern2, sizeof(pattern2)) == 0)
        {
            (void)PmDestroySleepAlarm(CPU_APP_TIMER_OTA_RTC);
        }
        else
        {
            (void)PmDestroySleepAlarm(CPU_APP_TIMER_OTA_RTC);
            int32_t wakeUpTimeInSeconds = CalcRtcWakeupDiffSeconds(buf);
            if (wakeUpTimeInSeconds  < 0)
            {
                LOG_Error("invalid wakeup time: %d", wakeUpTimeInSeconds);
            }
            else
            {
                LOG_Debug("wakeup time: %d", wakeUpTimeInSeconds);}
                (void)PmCreateSleepAlarm(CPU_APP_TIMER_OTA_RTC, wakeUpTimeInSeconds, 0);
            }
    }
#endif
    return (uint8_t)NRC_NONE_E;
}

UdsNrc_e UdsDidWriteForOwner(UDS_DID_IDS_e did, const uint8_t *buf, uint16_t len)
{
    int32_t idx;
    const UdsDidEntry_t *e;
    uint8_t nrc;

    /* 参数检查 */
    if (buf == NULL)
    {
        return (uint8_t)NRC_INCORRECT_MESSAGE_LENGTH_E;
    }

    idx = UdsFindDidIndex(did);
    if (idx < 0)
    {
        return (uint8_t)NRC_REQUEST_OUT_OF_RANGE_E;
    }

    e = &g_uds_did_table[(uint32_t)idx];

    /* 访问权限检查（可写） */
    if ((e->access & UDS_ACCESS_W) == 0u)
    {
        return (uint8_t)NRC_REQUEST_OUT_OF_RANGE_E;
    }

    if (DbUpsertValueByDid(e->did, buf, len) != (int)NRC_NONE_E)
    {
        return (uint8_t)NRC_GENERAL_REJECT_E;
    }

    (void)memcpy(g_uds_store[(uint32_t)idx].data, buf, (size_t)len);
    g_uds_store[(uint32_t)idx].len = len;

    return (uint8_t)NRC_NONE_E;
}
/* ========================================================================== */
/*                                  控  制                                     */
/* ========================================================================== */
/**
 * @brief DID 控制入口：仅走 control_cb（不触库、不改 RAM）
 * @param did            目标 DID
 * @param subFunc        子功能码（若无子功能，可传 0）
 * @param in/inLen       控制输入数据
 * @param out/io_len     可选输出状态记录（当 *pReturnStatus=TRUE 时有效）
 * @param pReturnStatus  TRUE=正响应应携带 out；FALSE=仅回 6x DID（由上层封装）
 */
UdsNrc_e UdsDidControl(UDS_DID_IDS_e did, uint8_t subFunc,
                       const uint8_t *in, uint16_t inLen,
                       uint8_t *out, uint16_t *io_len,
                       gboolean *pReturnStatus)
{
    int32_t idx;

    /* 参数检查 */
    if (pReturnStatus == NULL)
    {
        return (uint8_t)NRC_INCORRECT_MESSAGE_LENGTH_E;
    }

    idx = UdsFindDidIndex(did);
    if (idx < 0)
    {
        LOG_Debug("DID %04x not found", did);
        return (uint8_t)NRC_REQUEST_OUT_OF_RANGE_E;
    }

    const UdsDidEntry_t *e = &g_uds_did_table[(uint32_t)idx];

    /* 权限检查（需要 Control 位） */
    if ((e->access & UDS_ACCESS_C) == 0u)
    {
        LOG_Debug("DID %04x no control access", did);
        return (uint8_t)NRC_REQUEST_OUT_OF_RANGE_E;
    }

    /* 仅回调：若未注册，视为不支持 */
    if (e->control_cb == NULL)
    {
        LOG_Debug("DID %04x no control callback", did);
        return (uint8_t)NRC_REQUEST_OUT_OF_RANGE_E;
    }

    /* 调用回调；out/io_len/pReturnStatus 由回调决定 */
    return e->control_cb((uint16_t)did, subFunc, in, inLen, out, io_len, pReturnStatus);
}