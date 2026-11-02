
#ifndef _MCU_MPU_SYNC_TASK_H_
#define _MCU_MPU_SYNC_TASK_H_

#include "stdint.h"
#include "parameterStoreManageApp.h"

typedef enum
{
    E_ParamId_ICCID = 0,                    // ICCID - 0x1203
    E_ParamId_IMEI = 1,                     // IMEI - 0x1205
    E_ParamId_IMSI = 2,                     // IMSI - 0x1204
    E_ParamId_SN = 3,                       // Tbox SN - 0x1218
    E_ParamId_VIN = 4,                      // VIN码
    E_ParamId_APN1 = 5,                     // APN1 - 0x1206
    E_ParamId_APN2 = 6,                     // APN2 - 0x1207
    E_ParamId_APN3 = 7,                     // APN3 - 0x1208
    E_ParamId_Fast_Sleep_Mode = 8,          // 快速休眠模式 - 0x1213
    E_ParamId_SW_Version = 9,               // 内部版本号 - 0x1214
    E_ParamId_SW_Version_External = 10,     // 外部版本号 - 0x1215
    E_ParamId_HW_Version = 11,              // 硬件版本号 - 0x1221
    E_ParamId_paramEncryptStatus = 12,      // 加密状态
    E_ParamId_GB32960Activated = 13,        // 国标激活状态 - 与MPU保持一致
    E_ParamId_BaseEnd = 99,                 /* 始末标志请勿删除 - 与MPU保持一致 */
    E_ParamId_UserStart = 100,              /* 始末标志请勿删除 - 与MPU保持一致 */
    E_ParamId_UserEnd = 255,                /* 始末标志请勿删除 */
    E_ParamId_InvalidId = 300,              /* 始末标志请勿删除 */
    // E_ParamId_TSPAddr = 5,                  //
    // E_ParamId_TSPPort = 6,                  //
    // E_ParamId_GB32960Addr = 7,              //
    // E_ParamId_GB329060Port = 8,             //
    // E_ParamId_HJ1239Addr = 9,               // 重型车排放远程监控计数规范
    // E_ParamId_HJ1239Port = 10,              //
    // E_ParamId_ECallNumber = 11,             //
    // E_ParamId_BCallNumber = 12,             //
    // E_ParamId_ICallNumber = 13,             //
    // E_ParamId_SW_Version = 14,              // 不存flash F195_cxl
    // E_ParamId_HW_Version = 15,              // 不存flash F193_cxl
    // E_ParamId_CustomSW_Version = 16,        // 客户版本号
    // E_ParamId_CarVersion = 17,              // 整车版本号
    // E_ParamId_ManufactureData = 18,         // 生产日期
    // E_ParamId_ParatNumber = 19,             // 零部件号
    // E_ParamId_SupIdentifier = 20,           // 供应商ID
    // E_ParamId_ManufactoryMode = 21,
    // E_ParamId_Fast_Sleep_Mode = 22,
    // E_ParamId_APN1 = 21,                    // APN1接入点 0x011B_cxl
    // E_ParamId_IP1_ADDR = 22,                // IP1地址 0x011C_cxl
    // E_ParamId_TSPPort1 = 23,                // TSP1端口 0x011D_cxl
    // E_ParamId_TspTimeout = 24,              // TSP超时时间 0xB209_cxl
    // E_ParamId_OfficialServiceNum = 27,      // 官方服务热线 0xB20A_cxl
    // E_ParamId_EmergencyAsNum = 28,          // 紧急售后热线 0xB20B_cxl
    // E_ParamId_TboxPhoneNum = 29,            // TBOX电话号码 0x0129_cxl
    // E_ParamId_TspFlowTimeout = 30,          // TSP流程超时时间 0xB20C_cxl
    // E_ParamId_TspAckTime = 31,              // 等待TSP应答时间 0xB20D_cxl
    // E_ParamId_Apn2 = 32,                    // APN2 0x013C_cxl
    // E_ParamId_Ip2Addr = 33,                 // IP2 地址 国标 0xB20E_cxl
    // E_ParamId_Port2 = 34,                   // 端口2 国标 0xB20F_cxl
    // E_ParamId_Ip3Addr = 35,                 // IP3 地址 0x105F_cxl
    // E_ParamId_LongConnCycle = 36,           // 长连接周期 0xB211_cxl
    // E_ParamId_NormalUploadPeriod = 38,      // 0xB229_cxl
    // E_ParamId_AlarmUploadPeriod = 39,       // 0xB22A_cxl
    // E_ParamId_TspDomainLen = 40,            // 0xB22B_cxl
    // E_ParamId_TspDomain = 41,               // 0xB22C_cxl
    // E_ParamId_TspPortNumeric = 42,          // 0xB22D_cxl
    // E_ParamId_PublicTspDomain = 43,         // 公网TSP域名 0x1014_cxl
    // E_ParamId_Apn1Type = 44,                // APN1类型 0xB247_cxl
    // E_ParamId_Apn2Type = 45,                // APN2类型 0xB248_cxl
    // E_ParamId_TspConnectSetting = 46,       // TSP连接设置 0xB258_cxl
    // E_ParamId_PkiApplyRequest = 47,         // 0xB261_cxl
    // E_ParamId_TspDomain1 = 48,              // 0x031C_cxl
    // E_ParamId_Apn3 = 49,                    // 0x1061_cxl
    // E_ParamId_OtaDomainName = 50,           // 0xB2CB_cxl
    // E_ParamId_OtaPort = 51,                 // 0xB2CC_cxl
    // E_ParamId_PkiDomainName = 52,           // 0x1052_cxl
    // E_ParamId_PkiPort = 53,                 // 0x1053_cxl
    // E_ParamId_TspPort3 = 54,                // 0x105E_cxl
    // E_ParamId_DtcSettingControl = 55,       // 0x0120_cxl
    // E_ParamId_LogDomainName = 56,           // 日志域名        0xB275_cxl
    // E_ParamId_LogPort = 57,                 // 日志端口    0xB276_cxl
    // E_ParamId_LogConnectionCommand = 58,    // 日志连接指令    0xB277_cxl
    // E_ParamId_EthernetConfig = 59,          // 以太网配置 0xB212_cxl
    // E_ParamId_PkiCertMaintenanceCycle = 60, // 0xB278_cxl
    // E_ParamId_LogAccount = 61,              // 日志账号 0xB27C_cxl
    // E_ParamId_LogPassword = 62,             // 日志密码 0xB27D_cxl
    // E_ParamId_LogUploadChannel = 63,        // 日志上传通道 0xB27F_cxl
    // E_ParamId_LogPath = 64,                 // 日志上传路径 0xB282_cxl
    // E_ParamId_Apn3Type = 65,                // 0xB28B_cxl
    // E_ParamId_SensitiveDataSet = 66,        // 0xB2C5_cxl 敏感数据集
    // E_ParamId_OtaRtcWakeupTime = 67,        // 0xB2C6_cxl
    // E_ParamId_BuryingPointSwitch = 68,      // 0xB2C7_cxl
    // E_ParamId_AdbStatus = 69,               // 0xB296_cxl
    // E_ParamId_ConnectionTestAddr = 70,      // 0xB297_cxl
    // E_ParamId_DataBuriedPointDomain = 71,   // 0xB289_cxl
    // E_ParamId_GnssGalaxy = 72,              // 0xB2C9_cxl
    // E_ParamId_OtaRtcWakeupTime2,            // 0xB2CA_cxl
    // E_ParamId_OtaSwAcqTime,                 // 0xB2CD_cxl
    // E_ParamId_JtagStatus = 78,              // 0xB2E5_cxl
    // E_ParamId_FunctionConfig = 80,          // 0xB2E6_cxl
    // E_ParamId_TransportMode,                // 0xB2B4_cxl
    // E_ParamId_ManufactoryMode,              // 0x0110_cxl
    // E_ParamId_4gResetCount,                 // 0xB260_cxl
    

    // E_ParamId_ecallConfigEnable                       =     21	 ,
    // E_ParamId_ecallConfigVoiceConf                    =     22	 ,
    // E_ParamId_ecallConfigMode                         =     23	 ,
    // E_ParamId_ecallConfigProcessinfo                  =     24	 ,
    // E_ParamId_ecallConfigStartTimer                   =     25	 ,
    // E_ParamId_ecallConfigHackTimer                    =     26	 ,
    // E_ParamId_ecallConfigMsdTransmission              =     27	 ,
    // E_ParamId_ecallConfigMoFailRedial                 =     28	 ,
    // E_ParamId_ecallConfigDropRedial                   =     29	 ,
    // E_ParamId_ecallConfigCleardownTimer               =     30	 ,
    // E_ParamId_ecallConfigInitiationTimer              =     31	 ,
    // E_ParamId_ecallConfigNadRegistrationTimer         =     32	 ,
    // E_ParamId_ecallConfigNadUnregistrationTimer       =     33	 ,
    // E_ParamId_ecallConfigSystem                       =     34	 ,
    // E_ParamId_ecallConfigEraGlonassRedialTimer        =     35	 ,
    // E_ParamId_ecallConfigEraGlonassAutoAnswerTimer    =     36	 ,
    // E_ParamId_ecallConfigRedialDurationTimer          =     37	 ,
    // E_ParamId_ecallConfigTestMode                     =     38	 ,
    // E_ParamId_ecallConfigTestNum                      =     39	 ,
    // E_ParamId_BaseEnd                                 =     63	 ,
    // E_ParamId_UserEnd                                 =     255  ,

    E_PARAMETER_ITEM_NUM_MAX,
} ParameterItemId_e;

typedef struct
{
    uint8_t paramId;
    uint16_t length;
} ParamLengthEntry_t;

#define TBOX_PARAMTER_SYNC_MAP_WRITE_BEGIN()                                                              \
    static int16_t McuParameterWrite(uint8_t parameterItem, uint8_t *parameterData, uint16_t pDataLength) \
    {                                                                                                     \
        switch (parameterItem)                                                                            \
        {

#define TBOX_PARAMETER_SYNC_WRITE_MAP(syncItem, parameterItem)                 \
    case syncItem:                                                             \
    {                                                                          \
        WorkFlashVehicleInforStore(parameterItem, parameterData, pDataLength); \
    }                                                                          \
    break;

#define TBOX_PARAMTER_SYNC_MAP_WRITE_END() \
    default:                               \
        break;                             \
        }                                  \
        return 0;                          \
        }

#define TBOX_PARAMTER_SYNC_MAP_READ_BEGIN()                                                               \
    static int16_t McuParameterRead(uint8_t parameterItem, uint8_t *parameterData, uint16_t *pDataLength) \
    {                                                                                                     \
        uint32_t itemSize;                                                                                \
        itemSize = 0;                                                                                     \
        switch (parameterItem)                                                                            \
        {

#define TBOX_PARAMETER_SYNC_READ_MAP(syncItem, parameterItem)               \
    case syncItem:                                                          \
    {                                                                       \
        WorkFlashVehicleInforRead(parameterItem, parameterData, &itemSize); \
    }                                                                       \
    break;

#define TBOX_PARAMTER_SYNC_MAP_READ_END() \
    default:                              \
        break;                            \
        }                                 \
        *pDataLength = itemSize;          \
        return 0;                         \
        }

TBOX_PARAMTER_SYNC_MAP_WRITE_BEGIN()
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_ICCID, E_PARAMETER_INFO_ICCID)
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_IMEI, E_PARAMETER_INFO_IMEI)
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_IMSI, E_PARAMETER_INFO_IMSI)
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_SN, E_PARAMETER_INFO_ECU_SERIAL_NUMBER)
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_VIN, E_PARAMETER_INFO_VIN)
#if 0
// TBOX_PARAMETER_SYNWRITE_C_MAP(E_ParamId_TSPAddr         , )
// TBOX_PARAMETER_SYNWRITE_C_MAP(E_ParamId_TSPPort         , )
// TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_GB32960Addr, E_PARAMETER_INFO_NATION_WORK_ADDRESS)
// TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_GB329060Port, E_PARAMETER_INFO_NATION_WORK_PORT)
// TBOX_PARAMETER_SYNWRITE_C_MAP(E_ParamId_HJ1239Addr      , )
// TBOX_PARAMETER_SYNWRITE_C_MAP(E_ParamId_HJ1239Port      , )
// TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_ECallNumber, E_PARAMETER_INFO_E_CALL_NUMBER)
// TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_BCallNumber, E_PARAMETER_INFO_B_CALL_NUMBER)
// TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_ICallNumber, E_PARAMETER_INFO_I_CALL_NUMBER)
// TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_SW_Version, E_PARAMETER_INFO_ECU_SOFTWARE_NUM)
// TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_HW_Version, E_PARAMETER_INFO_ECU_HARDWARE_NUM)
// TBOX_PARAMETER_SYNWRITE_C_MAP(E_ParamId_CustomSW_Version, )
// TBOX_PARAMETER_SYNWRITE_C_MAP(E_ParamId_CarVersion      , )
// TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_ManufactureData, E_PARAMETER_INFO_ECU_MANUFACTURE_DATE)
// TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_ParatNumber, E_PARAMETER_INFO_PART_NAME)
// TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_SupIdentifier, E_PARAMETER_INFO_SYSTEM_SUPPLIER_ID)
// TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_ManufactoryMode, E_PARAMETER_INFO_MANUFACTORY_MODE) 

TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_APN1, E_PARAMETER_INFO_APN1)                                          // 0x011B_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_IP1_ADDR, E_PARAMETER_INFO_IP1_ADDR)                                  // 0x011C_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TSPPort1, E_PARAMETER_INFO_PORT1)                                     // 0x011D_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TspTimeout, E_PARAMETER_INFO_TSP_TIMEOUT)                             // 0xB209_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_ECallNumber, E_PARAMETER_INFO_ECALL_NUM1)                            // 0x011F_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_BCallNumber, E_PARAMETER_INFO_BCALL_NUM1)                            // 0x0124_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_OfficialServiceNum, E_PARAMETER_INFO_OFFICIAL_SERVICE_NUM)            // 0xB20A_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_EmergencyAsNum, E_PARAMETER_INFO_EMERGENCY_AS_NUM)                    // 0xB20B_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TboxPhoneNum, E_PARAMETER_INFO_TBOX_PHONE_NUM)                        // 0x0129_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TspFlowTimeout, E_PARAMETER_INFO_TSP_FLOW_TIMEOUT)                    // 0xB20C_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TspAckTime, E_PARAMETER_INFO_TSP_ACK_TIME)                            // 0xB20D_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_Apn2, E_PARAMETER_INFO_APN2)                                          // 0x013C_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_Ip2Addr, E_PARAMETER_INFO_IP2_ADDR)                                   // 0xB20E_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_Port2, E_PARAMETER_INFO_PORT2)                                        // 0xB20F_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_Ip3Addr, E_PARAMETER_INFO_IP3_ADDR)                                   // 0x105F_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_LongConnCycle, E_PARAMETER_INFO_LONG_CONN_CYCLE)                      // 0xB211_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_NormalUploadPeriod, E_PARAMETER_INFO_NORMAL_UPLOAD_PERIOD)            // 0xB229_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_AlarmUploadPeriod, E_PARAMETER_INFO_ALARM_UPLOAD_PERIOD)              // 0xB22A_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TspDomainLen, E_PARAMETER_INFO_TSP_DOMAIN_LEN)                        // 0xB22B_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TspDomain, E_PARAMETER_INFO_TSP_DOMAIN)                               // 0xB22C_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TspPortNumeric, E_PARAMETER_INFO_TSP_PORT_NUMERIC)                    // 0xB22D_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_PublicTspDomain, E_PARAMETER_INFO_PUBLIC_TSP_DOMAIN)                  // 0x1014_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_Apn1Type, E_PARAMETER_INFO_APN1_TYPE)                                 // 0xB247_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_Apn2Type, E_PARAMETER_INFO_APN2_TYPE)                                 // 0xB248_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TspConnectSetting, E_PARAMETER_INFO_TSP_CONNECT_SETTING)              // 0xB258_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_PkiApplyRequest, E_PARAMETER_INFO_PKI_APPLY_REQUEST)                  // 0xB261_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TspDomain1, E_PARAMETER_INFO_TSP_DOMAIN_1)                            // 0x031C_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_Apn3, E_PARAMETER_INFO_APN3)                                          // 0x1061_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_OtaDomainName, E_PARAMETER_INFO_OTA_DOMAIN_NAME)                      // 0xB2CB_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_OtaPort, E_PARAMETER_INFO_OTA_PORT)                                   // 0xB2CC_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_PkiDomainName, E_PARAMETER_INFO_PKI_DOMAIN_NAME)                      // 0x1052_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_PkiPort, E_PARAMETER_INFO_PKI_PORT)                                   // 0x1053_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TspPort3, E_PARAMETER_INFO_TSP_PORT_3)                                // 0x105E_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_DtcSettingControl, E_PARAMETER_INFO_DTC_SETTING_CONTROL)              // 0x0120_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_LogDomainName, E_PARAMETER_INFO_LOG_DOMAIN_NAME)                      // 日志域名 0xB275_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_LogPort, E_PARAMETER_INFO_LOG_PORT)                                   // 日志端口 0xB276_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_LogConnectionCommand, E_PARAMETER_INFO_LOG_CONNECTION_COMMAND)        // 日志连接指令 0xB277_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_EthernetConfig, E_PARAMETER_INFO_ETHERNET_CONFIG)                     // 0xB212_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_PkiCertMaintenanceCycle, E_PARAMETER_INFO_PKI_CERT_MAINTENANCE_CYCLE) // 0xB278_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_LogAccount, E_PARAMETER_INFO_LOG_ACCOUNT)                             // 日志账号 0xB27C_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_LogPassword, E_PARAMETER_INFO_LOG_PASSWORD)                           // 日志密码 0xB27D_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_LogUploadChannel, E_PARAMETER_INFO_LOG_UPLOAD_CHANNEL)                // 日志上传通道 0xB27F_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_LogPath, E_PARAMETER_INFO_LOG_PATH)                                   // 日志上传路径 0xB282_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_Apn3Type, E_PARAMETER_INFO_APN3_TYPE)                                 // 0xB28B_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_SensitiveDataSet, E_PARAMETER_INFO_SENSITIVE_DATA_SET)                // 0xB2C5_cxl 敏感数据集
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_OtaRtcWakeupTime, E_PARAMETER_INFO_OTA_RTC_WAKEUP_TIME)               // 0xB2C6_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_BuryingPointSwitch, E_PARAMETER_INFO_BURYING_POINT_SWITCH)            // 0xB2C7_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_AdbStatus, E_PARAMETER_INFO_ADB_STATUS)                               // 0xB296_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_ConnectionTestAddr, E_PARAMETER_INFO_CONNECTION_TEST_ADDR)            // 0xB297_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_DataBuriedPointDomain, E_PARAMETER_INFO_DATA_BURIED_POINT_DOMAIN)     // 0xB289_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_GnssGalaxy, E_PARAMETER_INFO_GNSS_GALAXY)                             // 0xB2C9_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_OtaRtcWakeupTime2, E_PARAMETER_INFO_OTA_BOOKING_TIME)                 // 0xB2CA_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_OtaSwAcqTime, E_PARAMETER_INFO_OTA_SW_ACQ_TIME)                       // 0xB2CD_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_JtagStatus, E_PARAMETER_INFO_JTAG_STATUS)                             // 0xB2E5_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_FunctionConfig, E_PARAMETER_INFO_FUNCTION_CONFIG)                     // 0xB2E6_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_TransportMode, E_PARAMETER_INFO_TRANSPORT_MODE)                       // 0xB2B4_cxl
TBOX_PARAMETER_SYNC_WRITE_MAP(E_ParamId_ManufactoryMode, E_PARAMETER_INFO_MANUFACTORY_MODE)                   // 0x0110_cxl
#endif
TBOX_PARAMTER_SYNC_MAP_WRITE_END()

TBOX_PARAMTER_SYNC_MAP_READ_BEGIN()
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_ICCID, E_PARAMETER_INFO_ICCID)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_IMEI, E_PARAMETER_INFO_IMEI)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_IMSI, E_PARAMETER_INFO_IMSI)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_SN, E_PARAMETER_INFO_ECU_SERIAL_NUMBER)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_VIN, E_PARAMETER_INFO_VIN)
#if 0
// TBOX_PARAMETER_SYNREAD_C_MAP(E_ParamId_TSPAddr         , )
// TBOX_PARAMETER_SYNREAD_C_MAP(E_ParamId_TSPPort         , )
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_GB32960Addr, E_PARAMETER_INFO_NATION_WORK_ADDRESS)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_GB329060Port, E_PARAMETER_INFO_NATION_WORK_PORT)
// TBOX_PARAMETER_SYNREAD_C_MAP(E_ParamId_HJ1239Addr      , )
// TBOX_PARAMETER_SYNREAD_C_MAP(E_ParamId_HJ1239Port      , )
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_ECallNumber, E_PARAMETER_INFO_E_CALL_NUMBER)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_BCallNumber, E_PARAMETER_INFO_B_CALL_NUMBER)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_ICallNumber, E_PARAMETER_INFO_I_CALL_NUMBER)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_SW_Version, E_PARAMETER_INFO_ECU_SOFTWARE_NUM)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_HW_Version, E_PARAMETER_INFO_ECU_HARDWARE_NUM)
// TBOX_PARAMETER_SYNREAD_C_MAP(E_ParamId_CustomSW_Version, )
// TBOX_PARAMETER_SYNREAD_C_MAP(E_ParamId_CarVersion      , )
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_ManufactureData, E_PARAMETER_INFO_ECU_MANUFACTURE_DATE)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_ParatNumber, E_PARAMETER_INFO_PART_NAME)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_SupIdentifier, E_PARAMETER_INFO_SYSTEM_SUPPLIER_ID)
TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_ManufactoryMode, E_PARAMETER_INFO_MANUFACTORY_MODE)

//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_APN1, E_PARAMETER_INFO_APN1)                                          // 0x011B_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_IP1_ADDR, E_PARAMETER_INFO_IP1_ADDR)                                  // 0x011C_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TSPPort1, E_PARAMETER_INFO_PORT1)                                     // 0x011D_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TspTimeout, E_PARAMETER_INFO_TSP_TIMEOUT)                             // 0xB209_cxl
// TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_ECallNumber, E_PARAMETER_INFO_ECALL_NUM1)                            // 0x011F_cxl
// TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_BCallNumber, E_PARAMETER_INFO_BCALL_NUM1)                            // 0x0124_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_OfficialServiceNum, E_PARAMETER_INFO_OFFICIAL_SERVICE_NUM)            // 0xB20A_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_EmergencyAsNum, E_PARAMETER_INFO_EMERGENCY_AS_NUM)                    // 0xB20B_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TboxPhoneNum, E_PARAMETER_INFO_TBOX_PHONE_NUM)                        // 0x0129_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TspFlowTimeout, E_PARAMETER_INFO_TSP_FLOW_TIMEOUT)                    // 0xB20C_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TspAckTime, E_PARAMETER_INFO_TSP_ACK_TIME)                            // 0xB20D_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_Apn2, E_PARAMETER_INFO_APN2)                                          // 0x013C_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_Ip2Addr, E_PARAMETER_INFO_IP2_ADDR)                                   // 0xB20E_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_Port2, E_PARAMETER_INFO_PORT2)                                        // 0xB20F_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_Ip3Addr, E_PARAMETER_INFO_IP3_ADDR)                                   // 0x105F_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_LongConnCycle, E_PARAMETER_INFO_LONG_CONN_CYCLE)                      // 0xB211_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_NormalUploadPeriod, E_PARAMETER_INFO_NORMAL_UPLOAD_PERIOD)            // 0xB229_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_AlarmUploadPeriod, E_PARAMETER_INFO_ALARM_UPLOAD_PERIOD)              // 0xB22A_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TspDomainLen, E_PARAMETER_INFO_TSP_DOMAIN_LEN)                        // 0xB22B_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TspDomain, E_PARAMETER_INFO_TSP_DOMAIN)                               // 0xB22C_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TspPortNumeric, E_PARAMETER_INFO_TSP_PORT_NUMERIC)                    // 0xB22D_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_PublicTspDomain, E_PARAMETER_INFO_PUBLIC_TSP_DOMAIN)                  // 0x1014_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_Apn1Type, E_PARAMETER_INFO_APN1_TYPE)                                 // 0xB247_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_Apn2Type, E_PARAMETER_INFO_APN2_TYPE)                                 // 0xB248_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TspConnectSetting, E_PARAMETER_INFO_TSP_CONNECT_SETTING)              // 0xB258_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_PkiApplyRequest, E_PARAMETER_INFO_PKI_APPLY_REQUEST)                  // 0xB261_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TspDomain1, E_PARAMETER_INFO_TSP_DOMAIN_1)                            // 0x031C_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_Apn3, E_PARAMETER_INFO_APN3)                                          // 0x1061_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_OtaDomainName, E_PARAMETER_INFO_OTA_DOMAIN_NAME)                      // 0xB2CB_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_OtaPort, E_PARAMETER_INFO_OTA_PORT)                                   // 0xB2CC_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_PkiDomainName, E_PARAMETER_INFO_PKI_DOMAIN_NAME)                      // 0x1052_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_PkiPort, E_PARAMETER_INFO_PKI_PORT)                                   // 0x1053_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TspPort3, E_PARAMETER_INFO_TSP_PORT_3)                                // 0x105E_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_DtcSettingControl, E_PARAMETER_INFO_DTC_SETTING_CONTROL)              // 0x0120_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_LogDomainName, E_PARAMETER_INFO_LOG_DOMAIN_NAME)                      // 日志域名 0xB275_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_LogPort, E_PARAMETER_INFO_LOG_PORT)                                   // 日志端口 0xB276_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_LogConnectionCommand, E_PARAMETER_INFO_LOG_CONNECTION_COMMAND)        // 日志连接指令 0xB277_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_EthernetConfig, E_PARAMETER_INFO_ETHERNET_CONFIG)                     // 0xB212_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_PkiCertMaintenanceCycle, E_PARAMETER_INFO_PKI_CERT_MAINTENANCE_CYCLE) // 0xB278_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_LogAccount, E_PARAMETER_INFO_LOG_ACCOUNT)                             // 日志账号 0xB27C_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_LogPassword, E_PARAMETER_INFO_LOG_PASSWORD)                           // 日志密码 0xB27D_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_LogUploadChannel, E_PARAMETER_INFO_LOG_UPLOAD_CHANNEL)                // 日志上传通道 0xB27F_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_LogPath, E_PARAMETER_INFO_LOG_PATH)                                   // 日志上传路径 0xB282_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_Apn3Type, E_PARAMETER_INFO_APN3_TYPE)                                 // 0xB28B_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_SensitiveDataSet, E_PARAMETER_INFO_SENSITIVE_DATA_SET)                // 0xB2C5_cxl 敏感数据集
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_OtaRtcWakeupTime, E_PARAMETER_INFO_OTA_RTC_WAKEUP_TIME)               // 0xB2C6_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_BuryingPointSwitch, E_PARAMETER_INFO_BURYING_POINT_SWITCH)            // 0xB2C7_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_AdbStatus, E_PARAMETER_INFO_ADB_STATUS)                               // 0xB296_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_ConnectionTestAddr, E_PARAMETER_INFO_CONNECTION_TEST_ADDR)            // 0xB297_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_DataBuriedPointDomain, E_PARAMETER_INFO_DATA_BURIED_POINT_DOMAIN)     // 0xB289_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_GnssGalaxy, E_PARAMETER_INFO_GNSS_GALAXY)                             // 0xB2C9_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_OtaRtcWakeupTime2, E_PARAMETER_INFO_OTA_BOOKING_TIME)                 // 0xB2CA_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_OtaSwAcqTime, E_PARAMETER_INFO_OTA_SW_ACQ_TIME)                       // 0xB2CD_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_JtagStatus, E_PARAMETER_INFO_JTAG_STATUS)                             // 0xB2E5_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_FunctionConfig, E_PARAMETER_INFO_FUNCTION_CONFIG)                     // 0xB2E6_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_TransportMode, E_PARAMETER_INFO_TRANSPORT_MODE)                       // 0xB2B4_cxl
//TBOX_PARAMETER_SYNC_READ_MAP(E_ParamId_ManufactoryMode, E_PARAMETER_INFO_MANUFACTORY_MODE)                   // 0x0110_cxl
#endif
TBOX_PARAMTER_SYNC_MAP_READ_END()

void McuMpuSyncTaskMain(void *pvParameters);

#endif
