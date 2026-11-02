#include <string.h>
#include "stdlib.h"
#include "flashHal.h"
#include "ParameterStoreManageApp.h"

// ==================== 宏定义 (完全保留，一个不少) ====================
#define WORKFLASH_SYSTEM_SUPPLIER_ID_LEN 16 + 2
#define WORKFLASH_ECU_HARDWARE_VER_LEN 16 + 2
#define WORKFLASH_ECU_SOFTWARE_VER_LEN 16 + 2
#define WORKFLASH_SAP_ECU_MASK_LEN 16 + 2
#define WORKFLASH_ECU_MANUFACTURE_DATE_LEN 4 + 2
#define WORKFLASH_PART_NAME_LEN 14 + 2
#define WORKFLASH_PUBLIC_ASE_KEY 16 + 1
#define WORKFLASH_WORK_ADDR_LEN 32 + 2
#define WORKFLASH_WORK_PORT_LEN 16 + 2
#define WORKFLASH_WORK_PUBLIC_KEY_LEN 16 + 2
#define WORKFLASH_TBOX_CALL_NUM_LEN 20 + 2
#define WORKFLASH_VIN_LEN 17 + 2
#define WORKFLASH_BID_LEN 32 + 2
#define WORKFLASH_ECU_SERIAL_NUM_LEN 14 + 2
#define WORKFLASH_ICCID_LEN 32 + 2
#define WORKFLASH_IMEI_LEN 16 + 2
#define WORKFLASH_IMSI_LEN 16 + 2
#define WORKFLASH_REGISTER_FLAG_LEN 4 + 2
#define WORKFLASH_REGISTER_KEY_TYPE_LEN 4 + 2
#define WORKFLASH_CAN_LOAD_CYCLE_LEN 16 + 2
#define WORKFLASH_TBOX_MODEL_LEN 12 + 2
#define WORKFLASH_CAR_MODE_TYPE_LEN 16 + 2
#define WORKFLASH_SECURITY_VERSION_LEN 16 + 2
#define WORKFLASH_HARDWARE_NUM_LEN 16 + 5
#define WORKFLASH_SOFTWARE_NUM_LEN 16 + 5
#define WORKFLASH_DIAG_SN_LEN 16 + 5
#define WORKFLASH_ECU_VARIANTCODE_LEN 10 + 5
#define WORKFLASH_ECU_EOL_LEN 10 + 5
#define WORKFLASH_APP_SOFT_PRINT_LEN 10 + 5
#define WORKFLASH_SUBNET_CONFIG_LIST_SPEED_CAN_LEN 5 + 5
#define WORKFLASH_MCU_APP_SOFTWARE_VERSION_LEN 8 + 5

#define WORKFLASH_VEHICLE_NETWORK_CONFIGURATION_F1A1_LEN (17 + 2)
#define WORKFLASH_VEHICLE_NETWORK_CONFIGURATION_F1B0_LEN (31 + 2)
#define WORKFLASH_ECU_LEVEL_NETWORK_CONFIGURATION_DATA_IDENTIFIER_F1B1_LEN (31 + 2)
#define WORKFLASH_UIN_LEN (20 + 2)
#define WORKFLASH_VEHICLE_MODEL_F112_LEN (20 + 2)
#define WORKFLASH_FINGERPRINT_F1F0_LEN (26 + 2)
#define WORKFLASH_FINGERPRINT_ETHERNET_F0FF_LEN (48 + 2)
#define WORKFLASH_VEHICLE_SOFEWARE_VERSION_F1AB_LEN (10 + 2)
#define WORKFLASH_WORKING_MODE_F1C2_LEN (1 + 2)
#define WORKFLASH_SECOC_KEY_F1C7_LEN (32 + 2)
#define WORKFLASH_SK_LEN (16 + 2)
#define WORKFLASH_PIN_LEN (4 + 2)
#define WORKFLASH_CSR_HASH_F1CD_LEN (20 + 1)
#define WORKFLASH_MAINTENANCE_MODE_FD00_LEN (1 + 2)
#define UDS_SECURITY_ERROR_COUNT_LEN (3 + 2)
#define WORKING_MODE_LEN (2 + 2)
#define SECOC_TRIP_COUNT_LEN (4 + 2)
#define WORKFLASH_MANUFACTORY_MODE_LEN (1 + 2)
#define WORKFLASH_MANUFACTORY_REMOTE_WAKE_MAINTAIN_LEN (1 + 2)
#if 1 //SQ

#define WORKFLASH_FINGERPRINT_F10A_LEN (10+2)
#define WORKFLASH_MANUFACTUREDATE_F1B7_LEN (7+2)
#define WORKFLASH_PIN_F1B1_LEN    (16+2)
#define WORKFLASH_BATTERY_TYPE_F201_LEN  (1+2)
#define WORKFLASH_VOLT_PLATFORM_F202_LEN  (1+2)
#define WORKFLASH_EOL_ROLE_F203_LEN  (1+2)
#define WORKFLASH_TBOX_RESET_FLAG_LEN (1+2)
#define WORKFLASH_APN_LEN (32+2)
#else
// 带 _cxl 注释的参数 (完全保留)
#define WORKFLASH_APN1_LEN (32 + 2)                   // 0x011B_cxl
#define WORKFLASH_IP_ADDR_LEN (16 + 2)                // 0x011C_cxl
#define WORKFLASH_TSP_PORT_LEN (8 + 2)                // 0x011D_cxl
#define WORKFLASH_TSP_TIMEOUT_LEN (1 + 2)             // 0xB209_cxl
#define WORKFLASH_ECALL_NUM1_LEN (14 + 2)             // 0x011F_cxl
#define WORKFLASH_BCALL_NUM1_LEN (14 + 2)             // 0x0124_cxl
#define WORKFLASH_OFFICIAL_SERVICE_NUM_LEN (14 + 2)   // 0xB20A_cxl
#define WORKFLASH_EMERGENCY_AS_NUM_LEN (14 + 2)       // 0xB20B_cxl
#define WORKFLASH_TBOX_PHONE_NUM_LEN (14 + 2)         // 0x0129_cxl
#define WORKFLASH_TSP_FLOW_TIMEOUT_LEN (1 + 2)        // 0xB20C_cxl
#define WORKFLASH_TSP_ACK_TIME_LEN (1 + 2)            // 0xB20D_cxl
#define WORKFLASH_APN2_LEN (32 + 2)                   // 0x013C_cxl
#define WORKFLASH_IP2_ADDR_LEN (16 + 2)               // 0xB20E_cxl
#define WORKFLASH_PORT2_LEN (8 + 2)                   // 0xB20F_cxl
#define WORKFLASH_IP3_ADDR_LEN (16 + 2)               // 0x105F_cxl
#define WORKFLASH_LONG_CONN_CYCLE_LEN (2 + 2)         // 0xB211_cxl
#define WORKFLASH_NORMAL_UPLOAD_PERIOD_LEN (2 + 2)    // 0xB229_cxl
#define WORKFLASH_ALARM_UPLOAD_PERIOD_LEN (2 + 2)     // 0xB22A_cxl
#define WORKFLASH_TSP_DOMAIN_LEN_LEN (1 + 2)          // 0xB22B_cxl
#define WORKFLASH_TSP_DOMAIN_LEN (50 + 2)             // 0xB22C_cxl
#define WORKFLASH_TSP_PORT_NUMERIC_LEN (2 + 2)        // 0xB22D_cxl
#define WORKFLASH_PUBLIC_TSP_DOMAIN_LEN (50 + 2)      // 0x1014_cxl
#define WORKFLASH_APN_TYPE_LEN (1 + 2)                // 0xB247_cxl 0xB248_cxl
#define WORKFLASH_TSP_CONNECT_SETTING_LEN (1 + 2)     // 0xB258_cxl
#define WORKFLASH_PKI_APPLY_REQUEST_LEN (1 + 2)       // 0xB261_cxl
#define WORKFLASH_TSP_DOMAIN_1_LEN (50 + 2)           // 0x031C_cxl
#define WORKFLASH_APN3_LEN (32 + 2)                   // 0x1061_cxl
#define WORKFLASH_OTA_DOMAIN_NAME_LEN (50 + 2)        // 0xB2CB_cxl
#define WORKFLASH_OTA_PORT_LEN (8 + 2)                // 0xB2CC_cxl
#define WORKFLASH_PKI_DOMAIN_NAME_LEN (50 + 2)        // 0x1052_cxl
#define WORKFLASH_PKI_PORT_LEN (8 + 2)                // 0x1053_cxl
#define WORKFLASH_TSP_PORT_3_LEN (8 + 2)              // 0x105E_cxl
#define WORKFLASH_DTC_SETTING_CONTROL_LEN (12 + 2)    // 0x1020_cxl
#define WORKFLASH_LOG_DOMAIN_NAME_LEN (100 + 2)       // 0xB275_cxl
#define WORKFLASH_LOG_PORT_LEN (8 + 2)                // 0xB276_cxl
#define WORKFLASH_LOG_CONNECTION_COMMAND_LEN (16 + 2) // 0xB277_cxl
#define WORKFLASH_ETHERNET_CONFIG_LEN (4 + 2)         // 0xB212_cxl
#define WORKFLASH_PKI_CYCLE_LEN (1 + 2)               // 0xB278_cxl
#define WORKFLASH_LOG_CREDENTIAL_LEN (50 + 2)         // 0xB27C_cxl 0xB27D_cxl
#define WORKFLASH_LOG_CHANNEL_LEN (1 + 2)             // 0xB27F_cxl
#define WORKFLASH_LOG_PATH_LEN (256 + 2)              // 0xB282_cxl
#define WORKFLASH_APN3_TYPE_LEN (1 + 2)               // 0xB28B_cxl
#define WORKFLASH_SENSITIVE_DATA_SET_LEN (16 + 2)     // 0xB2C5_cxl
#define WORKFLASH_OTA_RTC_WAKEUP_LEN (8 + 2)          // 0xB2C6_cxl
#define WORKFLASH_BURYING_POINT_SWITCH_LEN (4 + 2)    // 0xB2C7_cxl
#define WORKFLASH_ADB_STATUS_LEN (1 + 2)              // 0xB296_cxl
#define WORKFLASH_CONN_TEST_ADDR_LEN (50 + 2)         // 0xB297_cxl
#define WORKFLASH_BURIED_POINT_DOMAIN_LEN (128 + 2)   // 0xB289_cxl
#define WORKFLASH_GNSS_GALAXY_LEN (1 + 2)             // 0xB2C9_cxl
#define WORKFLASH_OTA_BOOKING_TIME_LEN (6 + 2)        // 0xB2CA_cxl
#define WORKFLASH_OTA_SW_ACQ_TIME_LEN (1 + 2)         // 0xB2CD_cxl
#define WORKFLASH_JTAG_STATUS_LEN (1 + 2)             // 0xB2E5_cxl
#define WORKFLASH_FUNCTION_CONFIG_LEN (64 + 2)        // 0xB2E6_cxl
#define WORKFLASH_TRANSPORT_MODE_LEN (1 + 2)          // 0xB2B4_cxl
#define WORKFLASH_MANUFACTORY_MODE_LEN (1 + 2)        // 0x0110_cxl
#define WORKFLASH_4G_RESET_COUNT_LEN (1 + 2)          // 0xB260_cxl
#endif

// ==================== 结构体定义 ====================
/*存储于 WORKFLASH_ADDRESS_VEHICLE_INFO_IMPORTANT (0x0000), struct 大小约 263/2048 Byte, 剩余约 1785 Byte*/
typedef struct
{
  uint8_t u8SystemSupplierId[WORKFLASH_SYSTEM_SUPPLIER_ID_LEN];                   // 系统供应商标识
  uint8_t u8ECU_RepairShopCodeOrTestSN[WORKFLASH_DIAG_SN_LEN];                    // 维修店代码或测试序列号
  uint8_t u8ECU_InstallationDate[WORKFLASH_ECU_MANUFACTURE_DATE_LEN];             // ECU 安装日期
  uint8_t u8ECU_Variantcode[WORKFLASH_ECU_VARIANTCODE_LEN];                       // ECU变种代码
  uint8_t u8SAP_ECU_MASK[WORKFLASH_SAP_ECU_MASK_LEN];                             // 安全ECU掩码
  uint8_t u8ECU_ManuFactureDate[WORKFLASH_ECU_MANUFACTURE_DATE_LEN];              // ECU生产日期
  uint8_t u8PartName[WORKFLASH_PART_NAME_LEN];                                    // 零件名称
  uint8_t u8PublicAESKey[WORKFLASH_PUBLIC_ASE_KEY];                               // 【核心】ESK (必须在0x0000)
  uint8_t u8VIN[WORKFLASH_VIN_LEN];                                               // 【核心】VIN (必须在0x0000)
  uint8_t u8BID[WORKFLASH_BID_LEN];                                               // BID
  uint8_t u8ECU_SerialNumber[WORKFLASH_ECU_SERIAL_NUM_LEN];                       // ECU序列号
  uint8_t u8SecurityVersion[WORKFLASH_SECURITY_VERSION_LEN];                      // 安全版本
  uint8_t u8EOLconfig[WORKFLASH_ECU_EOL_LEN];                                     // EOL配置
  uint8_t u8AppSoftFingerPrint[WORKFLASH_APP_SOFT_PRINT_LEN];                     // 应用指纹信息
  uint8_t u8SubnetConfigListSpeedCan[WORKFLASH_SUBNET_CONFIG_LIST_SPEED_CAN_LEN]; // 子网配置列表
  uint8_t u8VIN_hex[WORKFLASH_VIN_LEN];
} vehicleInforParamImportant_t;

/*存储于 WORKFLASH_ADDRESS_VEHICLE_INFO_VARIABLE (0x1000), struct 大小约 658/2048 Byte, 剩余约 1390 Byte*/
typedef struct
{
  // --- 原有的 Variable 参数 ---
  uint8_t u8ICCID[32];
  uint8_t u8IMEI[16];
  uint8_t u8IMSI[16];
  uint8_t u8RegisterFlag[8];
  uint8_t u8KeyType[8];
  uint8_t u8McuResetCount[8];
  uint8_t u8ECU_HardwareNumber[WORKFLASH_HARDWARE_NUM_LEN];
  uint8_t u8ECU_SoftwareNumber[WORKFLASH_SOFTWARE_NUM_LEN];
  uint8_t u8ECU_HardwareVersion[WORKFLASH_ECU_HARDWARE_VER_LEN];
  uint8_t u8ECU_SoftwareVersion[WORKFLASH_ECU_SOFTWARE_VER_LEN];
  uint8_t u8MCU_AppSoftWareVersion[WORKFLASH_MCU_APP_SOFTWARE_VERSION_LEN];
  uint8_t u8TboxCallNumber[32];
  uint8_t u8MCU_AppSoftWareVersionF1C1[32];
  uint8_t u8DiagCanReport[8];

  uint8_t VehicleNetworkConfigurationF1A1[WORKFLASH_VEHICLE_NETWORK_CONFIGURATION_F1A1_LEN];
  uint8_t VehicleNetworkConfigurationF1B0[WORKFLASH_VEHICLE_NETWORK_CONFIGURATION_F1B0_LEN];
  uint8_t ECUlevelNetworkConfigurationDataIdentifierF1B1[WORKFLASH_ECU_LEVEL_NETWORK_CONFIGURATION_DATA_IDENTIFIER_F1B1_LEN];
  uint8_t UIN[WORKFLASH_UIN_LEN];
  uint8_t VehicleModelF112[WORKFLASH_VEHICLE_MODEL_F112_LEN];
  uint8_t FingerprintF1F0[WORKFLASH_FINGERPRINT_F1F0_LEN];
  uint8_t FingerprintEthernetF0FF[WORKFLASH_FINGERPRINT_ETHERNET_F0FF_LEN];
  uint8_t VehicleSoftwareVersionF1AB[WORKFLASH_VEHICLE_SOFEWARE_VERSION_F1AB_LEN];
  uint8_t WorkingModeF1C2[WORKFLASH_WORKING_MODE_F1C2_LEN];
  uint8_t SecOCKeyF1C7[WORKFLASH_SECOC_KEY_F1C7_LEN];
  uint8_t SK[WORKFLASH_SK_LEN];
  uint8_t PIN[WORKFLASH_PIN_LEN];
  uint8_t CSR_HASH[WORKFLASH_CSR_HASH_F1CD_LEN];
  uint8_t Maintenance[WORKFLASH_MAINTENANCE_MODE_FD00_LEN];
  uint8_t UdsSEecurityErrorCount[UDS_SECURITY_ERROR_COUNT_LEN];
  uint8_t WorkingMode[WORKING_MODE_LEN];
  uint8_t SecocTrioCount[SECOC_TRIP_COUNT_LEN];  
  uint8_t u8ManufactoryMode[WORKFLASH_MANUFACTORY_MODE_LEN];
  uint8_t RemoteWakeMaintainFlag[WORKFLASH_MANUFACTORY_REMOTE_WAKE_MAINTAIN_LEN];
  #if 1 //SQ
  uint8_t u8FingerprintF10A[WORKFLASH_FINGERPRINT_F10A_LEN];
  uint8_t u8ManufactureDateF1B7[WORKFLASH_MANUFACTUREDATE_F1B7_LEN];
  uint8_t u8PINF1B1[WORKFLASH_PIN_F1B1_LEN];
  uint8_t u8LowVoltBatteryTypeF201[WORKFLASH_BATTERY_TYPE_F201_LEN];
  uint8_t u8VoltPlatformF202[WORKFLASH_VOLT_PLATFORM_F202_LEN];
  uint8_t u8EolRoleF203[WORKFLASH_EOL_ROLE_F203_LEN];
  uint8_t u8TboxResetFlag[WORKFLASH_TBOX_RESET_FLAG_LEN];
  uint8_t u8Apn[WORKFLASH_APN_LEN];
  #else
  // --- ！！！关键移动！！！ 所有带 _cxl 的参数全部移到这里 (0x4000) ---
  // 保留所有参数定义，不做删除，确保代码完整性
  uint8_t u8Apn1[WORKFLASH_APN1_LEN];                                   // 0x011B_cxl
  uint8_t u8Ip1Addr[WORKFLASH_IP_ADDR_LEN];                             // 0x011C_cxl
  uint8_t u8TspPort[WORKFLASH_TSP_PORT_LEN];                            // 0x011D_cxl
  uint8_t u8TspTimeout[WORKFLASH_TSP_TIMEOUT_LEN];                      // 0xB209_cxl
  uint8_t u8EcallNum1[WORKFLASH_ECALL_NUM1_LEN];                        // 0x011F_cxl
  uint8_t u8BcallNum1[WORKFLASH_BCALL_NUM1_LEN];                        // 0x0124_cxl
  uint8_t u8OfficialServiceNum[WORKFLASH_OFFICIAL_SERVICE_NUM_LEN];     // 0xB20A_cxl
  uint8_t u8EmergencyAsNum[WORKFLASH_EMERGENCY_AS_NUM_LEN];             // 0xB20B_cxl
  uint8_t u8TboxPhoneNum[WORKFLASH_TBOX_PHONE_NUM_LEN];                 // 0x0129_cxl
  uint8_t u8TspFlowTimeout[WORKFLASH_TSP_FLOW_TIMEOUT_LEN];             // 0xB20C_cxl
  uint8_t u8TspAckTime[WORKFLASH_TSP_ACK_TIME_LEN];                     // 0xB20D_cxl
  uint8_t u8Apn2[WORKFLASH_APN2_LEN];                                   // 0x013C_cxl
  uint8_t u8Ip2Addr[WORKFLASH_IP2_ADDR_LEN];                            // 0xB20E_cxl
  uint8_t u8Port2[WORKFLASH_PORT2_LEN];                                 // 0xB20F_cxl
  uint8_t u8Ip3Addr[WORKFLASH_IP3_ADDR_LEN];                            // 0x105F_cxl
  uint8_t u8LongConnCycle[WORKFLASH_LONG_CONN_CYCLE_LEN];               // 0xB211_cxl
  uint8_t u8NormalUploadPeriod[WORKFLASH_NORMAL_UPLOAD_PERIOD_LEN];     // 0xB229_cxl
  uint8_t u8AlarmUploadPeriod[WORKFLASH_ALARM_UPLOAD_PERIOD_LEN];       // 0xB22A_cxl
  uint8_t u8TspDomainLen[WORKFLASH_TSP_DOMAIN_LEN_LEN];                 // 0xB22B_cxl
  uint8_t u8TspDomain[WORKFLASH_TSP_DOMAIN_LEN];                        // 0xB22C_cxl
  uint8_t u8TspPortNumeric[WORKFLASH_TSP_PORT_NUMERIC_LEN];             // 0xB22D_cxl
  uint8_t u8PublicTspDomain[WORKFLASH_PUBLIC_TSP_DOMAIN_LEN];           // 0x1014_cxl
  uint8_t u8Apn1Type[WORKFLASH_APN_TYPE_LEN];                           // 0xB247_cxl
  uint8_t u8Apn2Type[WORKFLASH_APN_TYPE_LEN];                           // 0xB248_cxl
  uint8_t u8TspConnectSetting[WORKFLASH_TSP_CONNECT_SETTING_LEN];       // 0xB258_cxl
  uint8_t u8PkiApplyRequest[WORKFLASH_PKI_APPLY_REQUEST_LEN];           // 0xB261_cxl
  uint8_t u8TspDomain1[WORKFLASH_TSP_DOMAIN_1_LEN];                     // 0x031C_cxl
  uint8_t u8Apn3[WORKFLASH_APN3_LEN];                                   // 0x1061_cxl
  uint8_t u8OtaDomainName[WORKFLASH_OTA_DOMAIN_NAME_LEN];               // 0xB2CB_cxl
  uint8_t u8OtaPort[WORKFLASH_OTA_PORT_LEN];                            // 0xB2CC_cxl
  uint8_t u8PkiDomainName[WORKFLASH_PKI_DOMAIN_NAME_LEN];               // 0x1052_cxl
  uint8_t u8PkiPort[WORKFLASH_PKI_PORT_LEN];                            // 0x1053_cxl
  uint8_t u8TspPort3[WORKFLASH_TSP_PORT_3_LEN];                         // 0x105E_cxl
  uint8_t u8DtcSettingControl[WORKFLASH_DTC_SETTING_CONTROL_LEN];       // 0x0120_cxl
  uint8_t u8LogDomainName[WORKFLASH_LOG_DOMAIN_NAME_LEN];               // 0xB275_cxl
  uint8_t u8LogPort[WORKFLASH_LOG_PORT_LEN];                            // 0xB276_cxl
  uint8_t u8LogConnectionCommand[WORKFLASH_LOG_CONNECTION_COMMAND_LEN]; // 0xB277_cxl
  uint8_t u8EthernetConfig[WORKFLASH_ETHERNET_CONFIG_LEN];              // 0xB212_cxl
  uint8_t u8PkiCertMaintenanceCycle[WORKFLASH_PKI_CYCLE_LEN];           // 0xB278_cxl
  uint8_t u8LogAccount[WORKFLASH_LOG_CREDENTIAL_LEN];                   // 0xB27C_cxl
  uint8_t u8LogPassword[WORKFLASH_LOG_CREDENTIAL_LEN];                  // 0xB27D_cxl
  uint8_t u8LogUploadChannel[WORKFLASH_LOG_CHANNEL_LEN];                // 0xB27F_cxl
  uint8_t u8LogPath[WORKFLASH_LOG_PATH_LEN];                            // 0xB282_cxl
  uint8_t u8Apn3Type[WORKFLASH_APN3_TYPE_LEN];                          // 0xB28B_cxl
  uint8_t u8SensitiveDataSet[WORKFLASH_SENSITIVE_DATA_SET_LEN];         // 0xB2C5_cxl
  uint8_t u8OtaRtcWakeupTime[WORKFLASH_OTA_RTC_WAKEUP_LEN];             // 0xB2C6_cxl
  uint8_t u8BuryingPointSwitch[WORKFLASH_BURYING_POINT_SWITCH_LEN];     // 0xB2C7_cxl
  uint8_t u8AdbStatus[WORKFLASH_ADB_STATUS_LEN];                        // 0xB296_cxl
  uint8_t u8ConnectionTestAddr[WORKFLASH_CONN_TEST_ADDR_LEN];           // 0xB297_cxl
  uint8_t u8DataBuriedPointDomain[WORKFLASH_BURIED_POINT_DOMAIN_LEN];   // 0xB289_cxl
  uint8_t u8GnssGalaxy[WORKFLASH_GNSS_GALAXY_LEN];                      // 0xB2C9_cxl
  uint8_t u8OtaBookingTime[WORKFLASH_OTA_BOOKING_TIME_LEN];             // 0xB2CA_cxl
  uint8_t u8OtaSwAcqTime[WORKFLASH_OTA_SW_ACQ_TIME_LEN];                // 0xB2CD_cxl
  uint8_t u8JtagStatus[WORKFLASH_JTAG_STATUS_LEN];                      // 0xB2E5_cxl
  uint8_t u8FunctionConfig[WORKFLASH_FUNCTION_CONFIG_LEN];              // 0xB2E6_cxl
  uint8_t u8TransportMode[WORKFLASH_TRANSPORT_MODE_LEN];                // 0xB2B4_cxl
  uint8_t u8ManufactoryMode[WORKFLASH_MANUFACTORY_MODE_LEN];            // 0x0110_cxl
  uint8_t u8_4gResetCount[WORKFLASH_4G_RESET_COUNT_LEN];                // 0xB260_cxl  
  
  // 保留一些之前定义的但没用的参数位，防止偏移错乱
  uint8_t u8WorkAddr[WORKFLASH_WORK_ADDR_LEN];
  uint8_t u8WorkPort[WORKFLASH_WORK_PORT_LEN];
  uint8_t u8PublicKey[WORKFLASH_WORK_PUBLIC_KEY_LEN];
  uint8_t u8TboxICallNumber[WORKFLASH_TBOX_CALL_NUM_LEN];
  uint8_t u8TSPCallNumber1[WORKFLASH_TBOX_CALL_NUM_LEN];
  uint8_t u8TSPCallNumber2[WORKFLASH_TBOX_CALL_NUM_LEN];
  uint8_t u8TSPCallNumber3[WORKFLASH_TBOX_CALL_NUM_LEN];
  uint8_t u8TboxPreKey[WORKFLASH_IMSI_LEN];
  uint8_t u8TSPMsgNumber1[WORKFLASH_TBOX_CALL_NUM_LEN];
  uint8_t u8TSPMsgNumber2[WORKFLASH_TBOX_CALL_NUM_LEN];
  uint8_t u8CanLoadCycle[WORKFLASH_CAN_LOAD_CYCLE_LEN];
  uint8_t u8TboxModel[WORKFLASH_TBOX_MODEL_LEN];
  uint8_t u8CarModeType[WORKFLASH_CAR_MODE_TYPE_LEN];
  uint8_t u8NationWorkAddr[WORKFLASH_WORK_ADDR_LEN];
  uint8_t u8NationWorkPort[WORKFLASH_WORK_PORT_LEN];
  uint8_t u8GovernmentWorkAddr[WORKFLASH_WORK_ADDR_LEN];
  uint8_t u8GovernmentWorkPort[WORKFLASH_WORK_PORT_LEN];  
#endif
} vehicleInforParamVariable_t;

typedef struct
{
  uint8_t securityInfo[3];
  uint8_t workingMode[2];
} SmallBlock1DataTable_t;

typedef struct
{
  uint8_t secocTripCount[8];
} SmallBlock2DataTable_t;

// ==================== 参数映射表 (关键修改) ====================
#define TBOX_PARAMTER_MAP_BEGIN() \
  static int16_t VehicleInforGetDataOffsetAddressAndLength(FlashParaId_e parameterId, uint8_t *pTypeFlag, uint32_t *offsetAddress, uint32_t *length) \
  { \
    vehicleInforParamVariable_t *pData0; \
    vehicleInforParamImportant_t *pData1; \
    uint8_t *pDataElement; \
    int16_t ret = 0; \
    uint32_t size; \
    pData0 = NULL; \
    pData1 = NULL; \
    switch (parameterId) {

// Type 0 = Variable (0x4000), Type 1 = Important (0x0000)
#define TBOX_PARAMETER_MAP(item, parameter, TypeIdx) \
  case item: \
  { \
    pDataElement = pData##TypeIdx->parameter; \
    size = sizeof(pData##TypeIdx->parameter); \
    *pTypeFlag = TypeIdx; \
  } \
  break;

#define TBOX_PARAMTER_MAP_END() \
  default: ret = -1; break; \
  } \
  if (ret != 0) return ret; \
  *offsetAddress = (uint32_t)pDataElement; \
  *length = size; \
  return ret; \
  }

// Small Block Map 定义 (保持不变)
#define TBOX_SMALL_BLOCK_MAP_BEGIN() \
  static int16_t VehicleInforSmallBlockGetDataOffsetAddressAndLength(SmallBlockDataParamId_e parameterId, uint8_t *pTypeFlag, uint32_t *offsetAddress, uint32_t *length) \
  { \
    SmallBlock1DataTable_t *pData1; \
    SmallBlock2DataTable_t *pData2; \
    uint8_t *pDataElement; \
    int16_t ret = 0; \
    uint32_t size; \
    pData1 = NULL; \
    pData2 = NULL; \
    switch (parameterId) {

#define TBOX_SMALL_BLOCK__MAP(item, parameter, blockNum) \
  case item: \
  { \
    pDataElement = pData##blockNum->parameter; \
    size = sizeof(pData##blockNum->parameter); \
    *pTypeFlag = blockNum; \
  } \
  break;

#define TBOX_SMALL_BLOCK__MAP_END() \
  default: ret = -1; break; \
  } \
  if (ret != 0) return ret; \
  *offsetAddress = (uint32_t)pDataElement; \
  *length = size; \
  return ret; \
  }

TBOX_PARAMTER_MAP_BEGIN()
// === Type 1: Important (0x0000) - 仅核心身份安全数据 ===
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_SYSTEM_SUPPLIER_ID, u8SystemSupplierId, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECU_DIAG_SN, u8ECU_RepairShopCodeOrTestSN, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECU_INSTALL_DATE, u8ECU_InstallationDate, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECU_CODE, u8ECU_Variantcode, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_SAP_ECU_MASK, u8SAP_ECU_MASK, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECU_MANUFACTURE_DATE, u8ECU_ManuFactureDate, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_PART_NAME, u8PartName, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ESK_KEY, u8PublicAESKey, 1) // 核心：ESK
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_VIN, u8VIN, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_VIN_hex, u8VIN_hex, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECU_SERIAL_NUMBER, u8ECU_SerialNumber, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_SECURITY_VERSION, u8SecurityVersion, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECU_EOL_CONFIG, u8EOLconfig, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_APP_SOFT_FINGER_PRINT, u8AppSoftFingerPrint, 1)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_SUBNET_CONFIG_LIST_SPEED_CAN, u8SubnetConfigListSpeedCan, 1)

// === Type 0: Variable (0x4000) - 所有其他数据（含远控、_cxl参数） ===
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ICCID, u8ICCID, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_IMEI, u8IMEI, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_IMSI, u8IMSI, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TBOX_REGISTER_FLAG, u8RegisterFlag, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_KEY_TYPE, u8KeyType, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_MCU_RESET_COUNT, u8McuResetCount, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECU_HARDWARE_NUM, u8ECU_HardwareNumber, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECU_SOFTWARE_NUM, u8ECU_SoftwareNumber, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECU_HARDWARE_VERSION, u8ECU_HardwareVersion, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECU_SOFTWARE_VERSION, u8ECU_SoftwareVersion, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_MCU_APP_SOFTWARE_VERSION, u8MCU_AppSoftWareVersion, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TBOX_CALL_NUMBER, u8TboxCallNumber, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_MCU_APP_SOFTWARE_VERSION_F1C1, u8MCU_AppSoftWareVersionF1C1, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_DIAG_CAN_REPORT, u8DiagCanReport, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_VEHICLE_NETWORK_CONFIGURATION_F1A1, VehicleNetworkConfigurationF1A1, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_VEHICLE_NETWORK_CONFIGURATION_F1B0, VehicleNetworkConfigurationF1B0, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECU_LEVEL_NETWORK_CONFIGURATION_DATA_IDENTIFIER_F1B1, ECUlevelNetworkConfigurationDataIdentifierF1B1, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_UIN, UIN, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_VEHICLE_MODEL_F112, VehicleModelF112, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_FINGERPRINT_F1F0, FingerprintF1F0, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_FINGERPRINT_ETHERNET_F0FF, FingerprintEthernetF0FF, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_VEHICLE_SOFEWARE_VERSION_F1AB, VehicleSoftwareVersionF1AB, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_WORKING_MODE_F1C2, WorkingModeF1C2, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_SECOC_KEY_F1C7, SecOCKeyF1C7, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_SK, SK, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_PIN, PIN, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_CSR_HASH_F1CD, CSR_HASH, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_MAINTENANCE_MODE_FD00, Maintenance, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_UDS_SECURITY_ERROR_COUNT, UdsSEecurityErrorCount, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_WORKING_MODE, WorkingMode, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_SECOC_TRIP_COUNT, SecocTrioCount, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_MANUFACTORY_MODE, u8ManufactoryMode, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_REMOTE_WAKE_MAINTAIN, RemoteWakeMaintainFlag, 0)
#if 1 //SQ
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_FINGERPRINT_ETHERNET_F10A, u8FingerprintF10A, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_MANUFACTURE_DATE_F1B7, u8FingerprintF10A, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_PIN_F1B1, u8PINF1B1, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_LOWVOLTBATTERY_TYPE_F201,u8LowVoltBatteryTypeF201, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_VOLT_PLATFORM_F202,u8VoltPlatformF202, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ETH_ROLE_F203,u8EolRoleF203, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TBOX_RESET_FLAG, u8TboxResetFlag, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_APN, u8Apn, 0)
#else
// --- 全部 _cxl 参数都映射到 Type 0 ---
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_APN1, u8Apn1, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_IP1_ADDR, u8Ip1Addr, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_PORT1, u8TspPort, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TSP_TIMEOUT, u8TspTimeout, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ECALL_NUM1, u8EcallNum1, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_BCALL_NUM1, u8BcallNum1, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_OFFICIAL_SERVICE_NUM, u8OfficialServiceNum, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_EMERGENCY_AS_NUM, u8EmergencyAsNum, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TBOX_PHONE_NUM, u8TboxPhoneNum, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TSP_FLOW_TIMEOUT, u8TspFlowTimeout, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TSP_ACK_TIME, u8TspAckTime, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_APN2, u8Apn2, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_IP2_ADDR, u8Ip2Addr, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_PORT2, u8Port2, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_IP3_ADDR, u8Ip3Addr, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_LONG_CONN_CYCLE, u8LongConnCycle, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_NORMAL_UPLOAD_PERIOD, u8NormalUploadPeriod, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ALARM_UPLOAD_PERIOD, u8AlarmUploadPeriod, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TSP_DOMAIN_LEN, u8TspDomainLen, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TSP_DOMAIN, u8TspDomain, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TSP_PORT_NUMERIC, u8TspPortNumeric, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_PUBLIC_TSP_DOMAIN, u8PublicTspDomain, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_APN1_TYPE, u8Apn1Type, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_APN2_TYPE, u8Apn2Type, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TSP_CONNECT_SETTING, u8TspConnectSetting, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_PKI_APPLY_REQUEST, u8PkiApplyRequest, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TSP_DOMAIN_1, u8TspDomain1, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_APN3, u8Apn3, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_OTA_DOMAIN_NAME, u8OtaDomainName, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_OTA_PORT, u8OtaPort, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_PKI_DOMAIN_NAME, u8PkiDomainName, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_PKI_PORT, u8PkiPort, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TSP_PORT_3, u8TspPort3, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_DTC_SETTING_CONTROL, u8DtcSettingControl, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_LOG_DOMAIN_NAME, u8LogDomainName, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_LOG_PORT, u8LogPort, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_LOG_CONNECTION_COMMAND, u8LogConnectionCommand, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ETHERNET_CONFIG, u8EthernetConfig, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_PKI_CERT_MAINTENANCE_CYCLE, u8PkiCertMaintenanceCycle, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_LOG_ACCOUNT, u8LogAccount, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_LOG_PASSWORD, u8LogPassword, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_LOG_UPLOAD_CHANNEL, u8LogUploadChannel, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_LOG_PATH, u8LogPath, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_APN3_TYPE, u8Apn3Type, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_SENSITIVE_DATA_SET, u8SensitiveDataSet, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_OTA_RTC_WAKEUP_TIME, u8OtaRtcWakeupTime, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_BURYING_POINT_SWITCH, u8BuryingPointSwitch, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_ADB_STATUS, u8AdbStatus, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_CONNECTION_TEST_ADDR, u8ConnectionTestAddr, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_DATA_BURIED_POINT_DOMAIN, u8DataBuriedPointDomain, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_GNSS_GALAXY, u8GnssGalaxy, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_OTA_BOOKING_TIME, u8OtaBookingTime, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_OTA_SW_ACQ_TIME, u8OtaSwAcqTime, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_JTAG_STATUS, u8JtagStatus, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_FUNCTION_CONFIG, u8FunctionConfig, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_TRANSPORT_MODE, u8TransportMode, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_MANUFACTORY_MODE, u8ManufactoryMode, 0)
TBOX_PARAMETER_MAP(E_PARAMETER_INFO_4G_RESET_COUNT, u8_4gResetCount, 0)
#endif

TBOX_PARAMTER_MAP_END()

#if 0
TBOX_SMALL_BLOCK_MAP_BEGIN()
TBOX_SMALL_BLOCK__MAP(E_SECURITY_INFO, securityInfo, 1)
TBOX_SMALL_BLOCK__MAP(E_WORKING_MODE, workingMode, 1)
TBOX_SMALL_BLOCK__MAP(E_SECOC_TRIP_COUNT, secocTripCount, 2)
TBOX_SMALL_BLOCK__MAP_END()
#endif
// ==================== 读写接口 (Type 0/1) ====================

// static int16_t ByteArrayCompare(uint8_t *pData1, uint8_t *pData2, uint16_t length)
// {
//   uint16_t i;
//   int16_t ret;
//   if (pData1 == NULL || pData2 == NULL) { return -1; }
//   ret = 0;
//   for (i = 0; i < length; i++) {
//     if (pData1[i] != pData2[i]) { ret = 1; break; }
//   }
//   return ret;
// }

static int16_t WorkFlashWriteVehicleInfo(uint8_t importantFlag, uint32_t address, const uint8_t *data, uint32_t dataLength)
{
  uint32_t workFlashAddress;
  // 只有 0 和 1 两种情况
  if (importantFlag == 0)
  {
    workFlashAddress = WORKFLASH_ADDRESS_VEHICLE_INFO_VARIABLE; // 0x4000
  }
  else if (importantFlag == 1)
  {
    workFlashAddress = WORKFLASH_ADDRESS_VEHICLE_INFO_IMPORTANT; // 0x0000
  }
  else
  {
    return -1; 
  }
  return FlashHalDataBlockWrite(workFlashAddress, address, data, dataLength);
}

static int16_t WorkFlashReadVehicleInfo(uint8_t importantFlag, uint32_t address, uint8_t *data, uint32_t dataLength)
{
  uint32_t baseAddress;
  if (address + dataLength > 4096) { return -1; }
  
  if (importantFlag == 0)
  {
    baseAddress = WORKFLASH_ADDRESS_VEHICLE_INFO_VARIABLE;
  }
  else if (importantFlag == 1)
  {
    baseAddress = WORKFLASH_ADDRESS_VEHICLE_INFO_IMPORTANT;
  }
  else 
  {
    return -1;
  }
  FlashHalDataBlockRead(baseAddress, address, data, dataLength);
  return 0;
}

int16_t WorkFlashVehicleInforStore(FlashParaId_e parameterId, uint8_t *data, uint32_t dataLength)
{
  int16_t ret;
  uint32_t OffsetAddress;
  uint32_t length;
  uint8_t importantFlag;
  uint8_t tem[256];
  uint8_t i;

//   switch (parameterId)
//   {
//     case E_PARAMETER_INFO_ECU_SERIAL_NUMBER: // SN
//       if (dataLength != 45) return -3;
//       break;
       
//     case E_PARAMETER_INFO_VIN:               // VIN
//       if (dataLength != 17) return -3;
//       break;
       
//     case E_PARAMETER_INFO_ECU_PART_NUMBER:   // PN
//       if (dataLength != 14) return -3;        
//       break;
//     default:
//       break; 
//   }
  
  if (data == NULL) { return -1; }

  ret = VehicleInforGetDataOffsetAddressAndLength(parameterId, &importantFlag, &OffsetAddress, &length);
  if (ret != 0) { return ret; }

  if (length < dataLength) { return -2; }
  
  // 截断防止溢出
  if (length > (sizeof(tem) - 1)) { length = (sizeof(tem) - 1); }

  ret = WorkFlashReadVehicleInfo(importantFlag, OffsetAddress, tem, length);
  if (ret != 0) { return ret; }
  #if 0 //不可写入重复数据
  if (tem[0] == dataLength)
  {
    if (ByteArrayCompare(&tem[1], data, dataLength) == 0) 
    {
      return 1;
    }
  }
  #endif
  tem[0] = dataLength;
  for (i = 0; i < dataLength; i++) { tem[i + 1] = data[i]; }
  
  ret = WorkFlashWriteVehicleInfo(importantFlag, OffsetAddress, tem, dataLength + 1);
  if (ret != 0) { return ret; }
  
  return 0;
}

int16_t WorkFlashVehicleInforRead(FlashParaId_e parameterId, uint8_t *data, uint32_t *dataLength)
{
  int16_t ret;
  uint32_t OffsetAddress;
  uint32_t length;
  uint8_t importantFlag;
  uint8_t flashData[64];

  if (data == NULL) { return -1; }

  ret = VehicleInforGetDataOffsetAddressAndLength(parameterId, &importantFlag, &OffsetAddress, &length);
  if (ret != 0) { return ret; }

  ret = WorkFlashReadVehicleInfo(importantFlag, OffsetAddress, flashData, length);
  if (ret != 0) { return ret; }
  
  if (flashData[0] < length)
  {
    length = flashData[0];
  }
  else
  {
    length -= 1;
  }
  memcpy(data, &flashData[1], length);
  *dataLength = length;
  return 0;
}

#define DTC_BLOCK_COUNT 4

static const uint32_t g_dtcBlockAddress[DTC_BLOCK_COUNT] =
{
    WORKFLASH_ADDRESS_VEHICLE_DTC1,  /* 0x2000, ≤2048B */
    WORKFLASH_ADDRESS_VEHICLE_DTC2,  /* 0x3000, ≤2048B */
    WORKFLASH_ADDRESS_VEHICLE_DTC3,  /* 0x4000, ≤2048B */
    WORKFLASH_ADDRESS_VEHICLE_DTC4,  /* 0x5000, ≤2048B */
};

int16_t FlashDtcStore(const uint8_t *pDataIn, uint32_t lengthIn)
{
  int16_t ret = 0;
  uint32_t offset = 0;
  uint32_t remaining;
  uint32_t chunk;
  uint32_t i;

  /* 总容量: 4 × 2KB = 8192 bytes */
  if ((lengthIn > (WORKFLASH_SECTOR_SIZE * DTC_BLOCK_COUNT)) || (pDataIn == NULL) || (lengthIn == 0))
  {
    return -1;
  }

  remaining = lengthIn;
  for (i = 0; i < DTC_BLOCK_COUNT && remaining > 0; i++)
  {
    chunk = (remaining > WORKFLASH_SECTOR_SIZE) ? WORKFLASH_SECTOR_SIZE : remaining;
    ret = FlashHalDataBlockWrite(g_dtcBlockAddress[i], 0, ((uint8_t *)pDataIn) + offset, chunk);
    if (ret != 0)
    {
      return ret;
    }
    offset += chunk;
    remaining -= chunk;
  }

  return ret;
}

int16_t FlashDtcRead(uint8_t *pBufferIn, uint32_t readLength)
{
  int16_t ret = 0;
  uint32_t offset = 0;
  uint32_t remaining;
  uint32_t chunk;
  uint32_t i;

  if (pBufferIn == NULL)
  {
    return -1;
  }

  remaining = readLength;
  for (i = 0; i < DTC_BLOCK_COUNT && remaining > 0; i++)
  {
    chunk = (remaining > WORKFLASH_SECTOR_SIZE) ? WORKFLASH_SECTOR_SIZE : remaining;
    ret = FlashHalDataBlockRead(g_dtcBlockAddress[i], 0, ((uint8_t *)pBufferIn) + offset, chunk);
    if (ret != 0)
    {
      return ret;
    }
    offset += chunk;
    remaining -= chunk;
  }

  return ret;
}
