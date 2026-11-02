#ifndef DTC_RPC_API_H
#define DTC_RPC_API_H
#include "appRpcType.h"
#include <stdint.h>

#define DTC_DID_PAYLOAD_MAX (1028U)


typedef enum
{
    RPC_APP_DTC_SUBCMD_READ_DID_E,                 /**< 读取 DID (0x22) */
    RPC_APP_DTC_SUBCMD_WRITE_DID_E,                /**< 写入 DID (0x2E) */
} RpcAppDtcSubCmd_e;

/* ============================== DID 枚举表 ================================ */
/* 说明：覆盖数据库注册表 / 默认值表 / 读写接口的所有 DID。 */
typedef enum
{
    // 0x22/0x2E
#if 1   //SQ
    // EOL检测项DID定义
    DID_KL15_STATUS               = 0x1201,  // KL15状态：1=ON
    DID_KL30_VOLTAGE              = 0x1202,  // KL30电压：11000-13000mV
    DID_ICCID                     = 0x1203,  // ICCID：20字节ASCII
    DID_IMSI                      = 0x1204,  // IMSI：15字节ASCII
    DID_IMEI                      = 0x1205,  // IMEI：15字节ASCII
    DID_APN1                      = 0x1206,  // APN1
    DID_APN2                      = 0x1207,  // APN2
    DID_APN3                      = 0x1208,  // APN3
    DID_CSQ                       = 0x1209,  // CSQ信号强度：>=15且不等于99
    DID_GNSS_STATUS               = 0x1210,  // GNSS状态：1=就绪
    DID_EMMC_STATE                = 0x1211,  // EMMC挂载状态：1=成功
    DID_CAN_CHANNEL_STATUS        = 0x1212,  // CAN通道状态：1=正常
    DID_FAST_SLEEP_MODE           = 0x1213,  // 快速休眠模式：1=快速休眠，0=正常休眠
    DID_SW_VERSION_INTERNAL       = 0x1214,  // 内部版本号：12字节
    DID_SW_VERSION_EXTERNAL       = 0x1215,  // 外部版本号：17字节
    DID_4G_ANTENNA_STATUS         = 0x1216,  // 4G天线状态：1=正常
    DID_GNSS_ANTENNA_STATUS       = 0x1217,  // GNSS天线状态：1=正常
    DID_HW_VERSION                = 0x1221,  // 硬件版本号：17字节
    DID_TBOX_SN                   = 0x1218,  // Tbox SN：45字节（读写） 
    // 其他DID定义
    DID_VIN                       = 0xf190,    
    // 0x31     
    DID_DDR_TEST                     = 0x1301,    // 模组DDR测试
    DID_ICCID0                       = 0x1302,    // ICCID0：20字节ASCII
    DID_ICCID1                       = 0x1303,    // ICCID1：20字节ASCII
    DID_ICCID2                       = 0x1304,    // ICCID2：20字节ASCII
    
#else
	DID_VEHICLE_MODE              = 0x5001u,
    DID_TRANSPORT_MODE            = 0xb2b4u,
    DID_USAGE_MODE                = 0x3400u,
    DID_KEY_STATUS                = 0xb2b5u,    
    DID_MANUFACTORY_MODE          = 0x0110u,
    DID_DTC_CONTROL_STATE         = 0x0120u,
    DID_REPROGRAMMING_COUNTER     = 0x0200u,
    DID_REPROGRAMMING_ATTEMPT_COUNTER = 0x0201u,
    DID_GAC_DIAG_PARAM_VERSION    = 0xf10bu,
    DID_GAC_ECU_HW_VER            = 0xf17f,
    DID_BOOT_SW_ID                = 0xf180,
    DID_ACTIVE_SESSION            = 0xf186,
    DID_GAC_SPARE_PART_NUM        = 0xf187,
    DID_GAC_ECU_SW_VER            = 0xf189,
    DID_GAC_ECU_SERIAL_NUM        = 0xf18c,
    DID_GAC_KIT_ASSEMBLY_PART_NUM = 0xf18e,
    DID_VIN                       = 0xf190,
    DID_SUPPLIER_HW_VER           = 0xf193,
    DID_SUPPLIER_SW_VER           = 0xf195,
    DID_POWER_VOLTAGE             = 0x1000,
    DID_SYSTEM_SUPPLIER_ID        = 0xf18a,
    DID_CELL_STATUS_4G_3G_2G      = 0xb201,
    DID_GPS_STATUS                = 0xb202,
    DID_CURRENT_TIME              = 0xb203,
    DID_BCALL_ECALL_KEY_STATUS    = 0xb204,
    DID_TSP_DOMAIN_NAME           = 0x031c,
    DID_VEHICLE_SPEED             = 0xb205,
    DID_MOTOR_SPEED               = 0xb206,
    DID_IMEI                      = 0x010e,
    DID_IMSI                      = 0x010f,
    DID_ICCID                     = 0x0111,
    DID_RTC_WAKEUP_INTERVAL       = 0xb208,
    DID_APN1                      = 0x011b,
    DID_IP1                       = 0x011c,
    DID_PORT1                     = 0x011d,
    DID_TSP_CONN_TIMEOUT          = 0xb209,
    DID_ECALL_DIAL1               = 0x011f,
    DID_BCALL_DIAL1               = 0x0124,
    DID_OFFICIAL_SERVICE          = 0xb20a,
    DID_EMERGENCY_AFTER_SALES     = 0xb20b,
    DID_TBOX_PHONE_NUMBER         = 0x0129,
    DID_TSP_TIMEOUT_FOR_FLOWCHART = 0xb20c, //todo tsp流程超时时间
    DID_TSP_ACK_TIME              = 0xb20d, //todo 等待tsp应答时间
    DID_APN2                      = 0x013c,
    DID_IP2                       = 0xb20e,
    DID_PORT2                     = 0xb20f,
    DID_4G_NETWORK_STATUS         = 0xb210,
    DID_IP3                       = 0x105f,
    DID_LONG_CONNECTION_CYCLE     = 0xb211,
    DID_ETHERNET_CONFIG_INFO      = 0xb212,
    DID_TBOX_WAKEUP_SOURCE        = 0xb245,
    DID_PUBLIC_TSP_DOMAIN_NAME    = 0x1014,
    DID_APN1_TYPE                 = 0xb247,
    DID_APN2_TYPE                 = 0xb248,
    DID_GPS_SATELLITE_INFO_1_4    = 0xb249,
    DID_EMMC_STATE                = 0xb24a,
    DID_4G_MODULE_FW_VER          = 0xb24c,
    DID_4G_MODULE_HW_VER          = 0xb24d,
    DID_4G_MODULE_FUNC_STATE      = 0xb24e,
    DID_ETHERNET_ID               = 0xb256,
    DID_ETHERNET_FAULT_INFO       = 0xb257,
    DID_TSP_PLATFORM_CONN_SETTING = 0xb258, // todo tsp连接超时间间
    DID_DELETE_STORED_FILES       = 0xb259,
    DID_4G_APP_VER                = 0xb25b,
    DID_TBOX_RUNNING_MODE         = 0xb25c,
    DID_4G_HW_RESET_TIME          = 0xb25f, // 不依赖广汽的定时重启方案，该DID无用
    DID_4G_HW_RESET_COUNT         = 0xb260,
    DID_PKI_APPLY                 = 0xb261,
    DID_PKI_CERT_STATUS           = 0xb262,
    DID_PKI_DOMAIN_NAME           = 0x1052,
    DID_PKI_PORT                  = 0x1053,
    DID_GNSS_ANTENNA_VOLTAGE      = 0xb266,
    DID_4G_MODULE_STATE           = 0xb268,
    DID_4G_SPI_COMM_MCU           = 0xb26b,
    DID_OPENCPU_OVERTEMP          = 0xb271, // 周期给到MCU
    DID_LOG_DOMAIN_NAME           = 0xb275,
    DID_LOG_PORT                  = 0xb276,
    DID_LOG_CONN                  = 0xb277,
    DID_PKI_MAINTENANCE_CYCLE     = 0xb278,
    DID_PKI_SDK_VER               = 0xb279,
    DID_LOG_ACCOUNT               = 0xb27c,
    DID_LOG_PASSWORD              = 0xb27d,
    DID_LOG_INTERFACE             = 0xb27f,
    DID_LOG_DOMAIN_FRONT          = 0xb280,
    DID_LOG_DOMAIN_LATER          = 0xb281,
    DID_LOG_PATH                  = 0xb282,
    DID_LOG_PATH_P1               = 0xb283,
    DID_LOG_PATH_P2               = 0xb284,
    DID_LOG_PATH_P3               = 0xb285,
    DID_LOG_PATH_P4               = 0xb286,
    DID_PWM_SIGNAL                = 0xb287,
    DID_MQTT_PORT_PUBLIC          = 0x105e,
    DID_APN3                      = 0x1061,
    DID_APN3_TYPE                 = 0xb28b,
    DID_SENSITIVE_DATA_SET        = 0xb2c5,
    DID_OTA_RTC_WAKEUP_PARAM      = 0xb2c6,
    DID_BURYING_POINT_SWITCH      = 0xb2c7,
    DID_TCPDUMP_SWITCH            = 0xb2c8,
    DID_TSP_PLATFORM_CONN_STATUS  = 0xb28e,
    DID_GPS_LONG_LATITUDE         = 0xb290,
    DID_ADB_STATUS                = 0xb296,
    DID_CONN_TEST_ADDR            = 0xb297,
    DID_BURIED_POINT_DOMAIN       = 0xb289,
    DID_BURIED_POINT_DOMAIN_1     = 0xb29c,
    DID_BURIED_POINT_DOMAIN_2     = 0xb29d,
    DID_GNSS_GALAXY               = 0xb2c9,
    DID_OTA_UPDATE_TIME           = 0xb2ca,
    DID_OTA_DOMAIN_NAME           = 0xb2cb,
    DID_OTA_PORT                  = 0xb2cc,
    DID_OTA_SW_VER_ACQUIRE_TIME   = 0xb2cd,
    DID_OTA_MODE                  = 0x5005,
    DID_OTA_RECOVERY_STATUS       = 0x2110,
    DID_OTA_PART_SYNC_STATUS      = 0x2111,
    DID_JTAG_STATUS               = 0xb2e5,
    DID_FUNCTION_CONFIG           = 0xb2e6,
    DID_LOCAL_SLEEP_STATUS        = 0xf1a1,
    DID_LOCAL_WAKEUP_STATUS       = 0xf1a2,
    DID_RESET_PARAMS              = 0xFFFE,         // sirun did,用来删除数据库文件

    // 0x31
    DID_31_READ_ETH_TRANS_STATUS  = 0x1101,          // 以太网收发器通信状态读取
    DID_31_SET_AND_READ_ETH_MODE  = 0x1104,          // 设置&读取ETH通信模式
    DID_31_SET_AND_READ_ETH_TEST_MODE  = 0x1107,     // 设置&读取ETH测试模式
    DID_31_RUN_AND_READ_CABLE_DIAGNOSIS = 0x110B,    // 读取线缆诊断结果
    DID_31_DDR_TEST                     = 0xB2E6,    // 模组DDR测试
#endif
} UDS_DID_IDS_e;

/* ========== 统一的数据块 ========== */
typedef struct
{
    uint16_t len;                              /**< 数据长度 */
    uint8_t  data[DTC_DID_PAYLOAD_MAX];         /**< 数据内容 */
} RpcAppDtcDidData_t;

typedef struct
{
    uint16_t did;                /**< 目标 DID */
} RpcAppDtcDidReadReq_t;
typedef struct
{
    uint8_t             nrc;       /**< 执行结果：0x00 正常，其它为 NRC */
    RpcAppDtcDidData_t value;     /**< DID 值（当 exec.status==OK 时有效） */
} RpcAppDtcDidReadResp_t;

/* ========== 写 DID ========== */
typedef struct
{
    uint16_t             did;    /**< 目标 DID */
    RpcAppDtcDidData_t  value;  /**< 待写入的数据 */
} RpcAppDtcDidWriteReq_t;

typedef struct
{
    uint8_t          nrc;                /**< 执行结果：0x00 正常，其它为 NRC */
} RpcAppDtcDidWriteResp_t;


int32_t RpcAppDtcReadDid   (const RpcAppDtcDidReadReq_t  *req,
                             RpcAppDtcDidReadResp_t      *resp,
                             uint32_t timeoutMs);

int32_t RpcAppDtcWriteDid  (const RpcAppDtcDidWriteReq_t *req,
                             RpcAppDtcDidWriteResp_t     *resp,
                             uint32_t timeoutMs);

#endif