/*************************************************
Copyright ? 2024 SiRun (Beijing) . All rights reserved.
  File Name:       FlashParameterManageApp
  Author:          lei.wang
  Created Time:    2024
  Description:     Parameter storage flash operation
   Others:      // 其它说明
*************************************************/
#ifndef _PARAMETER_STORE_MANAGEAPP_H_
#define _PARAMETER_STORE_MANAGEAPP_H_

#include "stdint.h"

/*
 * WorkFlash 数据块布局 (紧凑分配, 每块 0x1000 = 2 × 2KB 扇区, 支持 Ping-Pong 备份)
 *
 *  偏移     块名                    物理范围        逻辑容量   说明
 *  ─────── ─────────────────────── ─────────────── ───────── ────────────────────────────
 *  0x0000  IMPORTANT               [0x0000-0x0FFF]  2048 B   重要设备信息（不经常更改）
 *  0x1000  VARIABLE                [0x1000-0x1FFF]  2048 B   可变设备信息
 *  0x2000  DTC1                    [0x2000-0x2FFF]  2048 B   故障码 Block 1
 *  0x3000  DTC2                    [0x3000-0x3FFF]  2048 B   故障码 Block 2
 *  0x4000  DTC3                    [0x4000-0x4FFF]  2048 B   故障码 Block 3 (新增)
 *  0x5000  DTC4                    [0x5000-0x5FFF]  2048 B   故障码 Block 4 (新增)
 *  0x6000  REMOTE_CONTROL_VALUE    [0x6000-0x6FFF]  2048 B   远控参数
 *  0x7000  FINGER_PRINT            [0x7000-0x7FFF]  2048 B   诊断指纹信息
 *  0x8000  BLUETOOTH_KEY           [0x8000-0x8FFF]  2048 B   蓝牙钥匙信息
 *  0x9000  RESERVED4               [0x9000-0x9FFF]  2048 B   预留 (CrashDump 等)
 *  0xA000  RESERVED5               [0xA000-0xAFFF]  2048 B   预留 - 新增块从此处开始追加
 *  0xB000  RESERVED6               [0xB000-0xBFFF]  2048 B   预留
 *  0xC000  RESERVED7               [0xC000-0xCFFF]  2048 B   预留
 *  0xD000  RESERVED8               [0xD000-0xDFFF]  2048 B   预留
 *  0xE000  RESERVED9               [0xE000-0xEFFF]  2048 B   预留
 *  0xF000  RESERVED10              [0xF000-0xFFFF]  2048 B   预留
 *
 * 注意事项:
 *    - 每块逻辑容量 = WORKFLASH_SECTOR_SIZE (2048 字节), 写入数据不能超过此值
 *    - Ping-Pong 备份: 底层自动使用 2 个物理扇区轮流写入, 无需额外预留空间
 *    - 新增数据块时, 从 RESERVED5 (0xA000) 开始依次分配, 步进 0x1000
 *    - 每个块地址必须对齐到 0x1000 边界 (低 12 位为 0)
 */
#define WORKFLASH_ADDRESS_VEHICLE_INFO_IMPORTANT 0x00000000 /* [0x0000] 重要设备信息, ≤2048B */
#define WORKFLASH_ADDRESS_VEHICLE_INFO_VARIABLE  0x00001000 /* [0x1000] 可变设备信息, ≤2048B */
#define WORKFLASH_ADDRESS_VEHICLE_DTC1           0x00002000 /* [0x2000] 故障码 Block 1, ≤2048B */
#define WORKFLASH_ADDRESS_VEHICLE_DTC2           0x00003000 /* [0x3000] 故障码 Block 2, ≤2048B */
#define WORKFLASH_ADDRESS_VEHICLE_DTC3           0x00004000 /* [0x4000] 故障码 Block 3, ≤2048B */
#define WORKFLASH_ADDRESS_VEHICLE_DTC4           0x00005000 /* [0x5000] 故障码 Block 4, ≤2048B */
#define WORKFLASH_ADDRESS_REMOTE_CONTROL_VALUE   0x00006000 /* [0x6000] 远控参数, ≤2048B */
#define WORKFLASH_ADDRESS_FINGER_PRINT           0x00007000 /* [0x7000] 诊断指纹信息, ≤2048B */
#define WORKFLASH_ADDRESS_BLUETOOTH_KEY          0x00008000 /* [0x8000] 蓝牙钥匙信息, ≤2048B */
#define WORKFLASH_ADDRESS_RESERVED4              0x00009000 /* [0x9000] 预留, ≤2048B */
#define WORKFLASH_ADDRESS_RESERVED5              0x0000A000 /* [0xA000] 预留, ≤2048B */
#define WORKFLASH_ADDRESS_RESERVED6              0x0000B000 /* [0xB000] 预留, ≤2048B */
#define WORKFLASH_ADDRESS_RESERVED7              0x0000C000 /* [0xC000] 预留, ≤2048B */
#define WORKFLASH_ADDRESS_RESERVED8              0x0000D000 /* [0xD000] 预留, ≤2048B */
#define WORKFLASH_ADDRESS_RESERVED9              0x0000E000 /* [0xE000] 预留, ≤2048B */
#define WORKFLASH_ADDRESS_RESERVED10             0x0000F000 /* [0xF000] 预留, ≤2048B */

// #define WORKFLASH_ADDRESS_SECURITY	                0x14017500	//安全访问失败次数存储地址

#define WORKFLASH_ADDRESS_SMALL_BLOCK_1 0x14017500 // 小块存储地址，一小块=128字节
#define WORKFLASH_ADDRESS_SMALL_BLOCK_2 0x14017580 // 小块存储地址，一小块=128字节

typedef enum
{   
    E_PARAMETER_INFO_SYSTEM_SUPPLIER_ID, // 系统供应商标识
    E_PARAMETER_INFO_ECU_HARDWARE_VERSION,
    E_PARAMETER_INFO_ECU_SOFTWARE_VERSION,
    E_PARAMETER_INFO_SAP_ECU_MASK,
    E_PARAMETER_INFO_ECU_MANUFACTURE_DATE, ////TBOX生产日期
    E_PARAMETER_INFO_PART_NAME,
    E_PARAMETER_INFO_VIN,               //
    E_PARAMETER_INFO_ECU_SERIAL_NUMBER, //
    E_PARAMETER_INFO_ESK_KEY,
    E_PARAMETER_INFO_CAR_MODE_TYPE,
    E_PARAMETER_INFO_SECURITY_VERSION,
    E_PARAMETER_INFO_ECU_HARDWARE_NUM,
    E_PARAMETER_INFO_ECU_SOFTWARE_NUM, //
    E_PARAMETER_INFO_ECU_DIAG_SN,
    E_PARAMETER_INFO_ECU_INSTALL_DATE,
    E_PARAMETER_INFO_ECU_CODE,
    E_PARAMETER_INFO_NATION_WORK_ADDRESS,
    E_PARAMETER_INFO_NATION_WORK_PORT,
    E_PARAMETER_INFO_GOVERNMENT_WORK_ADDRESS,
    E_PARAMETER_INFO_GOVERNMENT_WORK_PORT,
    E_PARAMETER_INFO_ECU_EOL_CONFIG,
    E_PARAMETER_INFO_TBOX_REGISTER_FLAG, //

    E_PARAMETER_INFO_ICCID, //
    E_PARAMETER_INFO_IMEI,  //
    E_PARAMETER_INFO_IMSI,  //
    E_PARAMETER_INFO_KEY_TYPE,
    E_PARAMETER_INFO_MCU_RESET_COUNT, //
    E_PARAMETER_INFO_APP_SOFT_FINGER_PRINT,
    E_PARAMETER_INFO_SUBNET_CONFIG_LIST_SPEED_CAN,
    E_PARAMETER_INFO_MCU_APP_SOFTWARE_VERSION, //
    E_PARAMETER_INFO_TBOX_CALL_NUMBER,
    E_PARAMETER_INFO_MCU_APP_SOFTWARE_VERSION_F1C1,
    E_PARAMETER_INFO_VIN_hex,
    E_PARAMETER_INFO_DIAG_CAN_REPORT,
    E_PARAMETER_INFO_E_CALL_NUMBER,
    E_PARAMETER_INFO_B_CALL_NUMBER,
    E_PARAMETER_INFO_I_CALL_NUMBER,

    E_PARAMETER_INFO_VEHICLE_NETWORK_CONFIGURATION_F1A1, // 整车网络配置
    E_PARAMETER_INFO_VEHICLE_NETWORK_CONFIGURATION_F1B0, // 整车网络配置
    E_PARAMETER_INFO_ECU_LEVEL_NETWORK_CONFIGURATION_DATA_IDENTIFIER_F1B1,
    E_PARAMETER_INFO_UIN,                           // 终端安全序列号	ASCII	20
    E_PARAMETER_INFO_VEHICLE_MODEL_F112,            // 车辆
    E_PARAMETER_INFO_FINGERPRINT_F1F0,              // 指纹    
    E_PARAMETER_INFO_FINGERPRINT_ETHERNET_F0FF,     // 以太网指纹
    E_PARAMETER_INFO_VEHICLE_SOFEWARE_VERSION_F1AB, // 整车软件版本
    E_PARAMETER_INFO_WORKING_MODE_F1C2,             // 工作模式
    E_PARAMETER_INFO_SECOC_KEY_F1C7,                // SecOC密钥
    E_PARAMETER_INFO_SK,                            // SK码
    E_PARAMETER_INFO_PIN,                           // PIN码
    E_PARAMETER_INFO_CSR_HASH_F1CD,
    // 生产工装需要
    E_PARAMETER_INFO_MAINTENANCE_MODE_FD00, // 维修模式

    E_PARAMETER_INFO_UDS_SECURITY_ERROR_COUNT,
    E_PARAMETER_INFO_WORKING_MODE,
    E_PARAMETER_INFO_SECOC_TRIP_COUNT,
    E_PARAMETER_INFO_MANUFACTORY_MODE,
    E_PARAMETER_INFO_DTC_SETTING_CONTROL,
    E_PARAMETER_INFO_REMOTE_WAKE_MAINTAIN,
#if 1
    E_PARAMETER_INFO_FINGERPRINT_ETHERNET_F10A,     // 0xF10A_SQ
    E_PARAMETER_INFO_MANUFACTURE_DATE_F1B7,     // 0xF1B7_SQ
    E_PARAMETER_INFO_PIN_F1B1,                   // 0xF1B1_SQ
    E_PARAMETER_INFO_LOWVOLTBATTERY_TYPE_F201,      // 0xF201_SQ
    E_PARAMETER_INFO_VOLT_PLATFORM_F202,      		// 0xF202_SQ
    E_PARAMETER_INFO_ETH_ROLE_F203,      			// 0xF203_SQ
    E_PARAMETER_INFO_TBOX_RESET_FLAG,            // Tbox Reset Flag
    E_PARAMETER_INFO_APN,                        // APN
#else
    E_PARAMETER_INFO_APN1,                       // 0x011B_cxl
    E_PARAMETER_INFO_IP1_ADDR,                   // 0x011C_cxl
    E_PARAMETER_INFO_PORT1,                      // 0x011D_cxl
    E_PARAMETER_INFO_TSP_TIMEOUT,                // 0xB209_cxl
    E_PARAMETER_INFO_ECALL_NUM1,                 // 0x011F_cxl
    E_PARAMETER_INFO_BCALL_NUM1,                 // 0x0124_cxl
    E_PARAMETER_INFO_OFFICIAL_SERVICE_NUM,       // 0xB20A_cxl
    E_PARAMETER_INFO_EMERGENCY_AS_NUM,           // 0xB20B_cxl
    E_PARAMETER_INFO_TBOX_PHONE_NUM,             // 0x0129_cxl
    E_PARAMETER_INFO_TSP_FLOW_TIMEOUT,           // 0xB20C_cxl
    E_PARAMETER_INFO_TSP_ACK_TIME,               // 0xB20D_cxl
    E_PARAMETER_INFO_APN2,                       // 0x013C_cxl
    E_PARAMETER_INFO_IP2_ADDR,                   // 0xB20E_cxl
    E_PARAMETER_INFO_PORT2,                      // 0xB20F_cxl
    E_PARAMETER_INFO_IP3_ADDR,                   // 0x105F_cxl
    E_PARAMETER_INFO_LONG_CONN_CYCLE,            // 0xB211_cxl
    E_PARAMETER_INFO_NORMAL_UPLOAD_PERIOD,       // 0xB229_cxl
    E_PARAMETER_INFO_ALARM_UPLOAD_PERIOD,        // 0xB22A_cxl
    E_PARAMETER_INFO_TSP_DOMAIN_LEN,             // 0xB22B_cxl
    E_PARAMETER_INFO_TSP_DOMAIN,                 // 0xB22C_cxl
    E_PARAMETER_INFO_TSP_PORT_NUMERIC,           // 0xB22D_cxl
    E_PARAMETER_INFO_PUBLIC_TSP_DOMAIN,          // 0x1014_cxl
    E_PARAMETER_INFO_APN1_TYPE,                  // 0xB247_cxl
    E_PARAMETER_INFO_APN2_TYPE,                  // 0xB248_cxl
    E_PARAMETER_INFO_TSP_CONNECT_SETTING,        // 0xB258_cxl
    E_PARAMETER_INFO_PKI_APPLY_REQUEST,          // 0xB261_cxl
    E_PARAMETER_INFO_TSP_DOMAIN_1,               // 0x031C_cxl
    E_PARAMETER_INFO_APN3,                       // 0x1061_cxl
    E_PARAMETER_INFO_OTA_DOMAIN_NAME,            // 0xB2CB_cxl
    E_PARAMETER_INFO_OTA_PORT,                   // 0xB2CC_cxl
    E_PARAMETER_INFO_PKI_DOMAIN_NAME,            // 0x1052_cxl
    E_PARAMETER_INFO_PKI_PORT,                   // 0x1053_cxl
    E_PARAMETER_INFO_TSP_PORT_3,                 // 0x105E_cxl
    E_PARAMETER_INFO_DTC_SETTING_CONTROL,        // 0x0120_cxl
    E_PARAMETER_INFO_LOG_DOMAIN_NAME,            // 0xB275_cxl
    E_PARAMETER_INFO_LOG_PORT,                   // 0xB276_cxl
    E_PARAMETER_INFO_LOG_CONNECTION_COMMAND,     // 0xB277_cxl
    E_PARAMETER_INFO_ETHERNET_CONFIG,            // 0xB212_cxl
    E_PARAMETER_INFO_PKI_CERT_MAINTENANCE_CYCLE, // 0xB278_cxl
    E_PARAMETER_INFO_LOG_ACCOUNT,                // 0xB27C_cxl
    E_PARAMETER_INFO_LOG_PASSWORD,               // 0xB27D_cxl
    E_PARAMETER_INFO_LOG_UPLOAD_CHANNEL,         // 0xB27F_cxl
    E_PARAMETER_INFO_LOG_PATH,                   // 0xB282_cxl
    E_PARAMETER_INFO_APN3_TYPE,                  // 0xB28B_cxl
    E_PARAMETER_INFO_SENSITIVE_DATA_SET,         // 0xB2C5_cxl
    E_PARAMETER_INFO_OTA_RTC_WAKEUP_TIME,        // 0xB2C6_cxl
    E_PARAMETER_INFO_BURYING_POINT_SWITCH,       // 0xB2C7_cxl
    E_PARAMETER_INFO_ADB_STATUS,                 // 0xB296_cxl
    E_PARAMETER_INFO_CONNECTION_TEST_ADDR,       // 0xB297_cxl
    E_PARAMETER_INFO_DATA_BURIED_POINT_DOMAIN,   // 0xB289_cxl, 0xB29C, 0xB29D
    E_PARAMETER_INFO_GNSS_GALAXY,                // 0xB2C9_cxl
    E_PARAMETER_INFO_OTA_BOOKING_TIME,           // 0xB2CA_cxl
    E_PARAMETER_INFO_OTA_SW_ACQ_TIME,            // 0xB2CD_cxl
    E_PARAMETER_INFO_JTAG_STATUS,                // 0xB2E5_cxl
    E_PARAMETER_INFO_FUNCTION_CONFIG,            // 0xB2E6_cxl
    E_PARAMETER_INFO_TRANSPORT_MODE,             // 0xB2B4_cxl
    E_PARAMETER_INFO_MANUFACTORY_MODE,           // 0x0110_cxl
    E_PARAMETER_INFO_4G_RESET_COUNT,             // 0xB260_cxl
#endif
} FlashParaId_e;

typedef enum
{
    E_SECURITY_INFO,
    E_WORKING_MODE,
    E_SECOC_TRIP_COUNT,
} SmallBlockDataParamId_e;

int16_t WorkFlashVehicleInforRead(FlashParaId_e parameterId, uint8_t *data, uint32_t *dataLength);
int16_t WorkFlashVehicleInforStore(FlashParaId_e parameterId, uint8_t *data, uint32_t dataLength);

#if 0
int16_t WorkFlashUserInfoStore(SmallBlockDataParamId_e parameterId,uint8_t *dataIn,uint32_t dataLenth);
int16_t WorkFlashUserInfoRead(SmallBlockDataParamId_e parameterId,uint8_t* pDataOut,uint32_t *dataLenth);
#endif

int16_t FlashDtcStore(const uint8_t *pDataIn, uint32_t lengthIn);
int16_t FlashDtcRead(uint8_t *pBufferIn, uint32_t readLength);

#endif
