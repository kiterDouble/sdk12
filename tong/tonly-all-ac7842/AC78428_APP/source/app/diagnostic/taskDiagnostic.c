/**************************************************
author: xiewenming

finish date:2018.7.31



*****************************************************/

#include "taskDiagnostic.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#include "logHal.h"
#include "flashHal.h"
#include "peripheralHal.h"
#include "mpuHal.h"
#include "timerHal.h"
#include "flashHal.h"
#include "crc8_16_32.h"

#include "canTpSdk.h"
#include "nmConfig.h"
#if (NM_PROTOCOL_TYPE == NM_PROTOCOL_OSEK)
#include "osekNmSdk.h"
#else
#include "autoSarNmSdk.h"
#endif
#include "timeSyncSdk.h"
#include "remoteDiagnosticSdk.h"
#include "powerManageSdk.h"
#include "mpuPowerSyncSdk.h"
#include "virtualTpSdk.h"

#include "taskDtcProcess.h"
#include "udsDidFunction.h"
#include "udsSecurityCalculate.h"
#include "udsEolTest.h"
#include "ParameterStoreManageApp.h"
#include "canPeriodTask.h"
#include "eolTestSyncWithCpu.h"
#include "mcuMpuSyncTask.h"
#include "parameterSyncSdk.h"
#include "vehicleSignalApp.h"
// #include "cy_mw_flash.h"

#define UDS_SECURITY_ERROR_COUNT_IN_NONVOLATILE 1

#define TEST_CODE 0

typedef int16_t (*pServiceFunction)(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
typedef int16_t (*pService22_DIDFunction)(uint8_t *pReadData, uint16_t *pDataLength);
typedef int16_t (*pService2E_DIDFunction)(uint8_t *pWriteData, uint16_t dataLength);
typedef int16_t (*pService31_DIDFunction)(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);

typedef enum
{
  E_SUBFUN_GET_OK,
  E_SUBFUN_GET_INVALID_DID,
  E_SUBFUN_GET_INVALID_SECURITY,
  E_SUBFUN_GET_INVALID_LENGTH,
  E_SUBFUN_GET_CONDITIONS_NOT_CORRECT,
  E_SUBFUN_GET_GENERAL_PROGRAMMING_ERROR,
} SeriveGetSubFunState_e;

typedef enum
{
  E_SECURITYTIMER_NONE,
  E_SECURITYTIMER_RESETDELAY,
  E_SECURITYTIMER_ACCESSERRORDELAY,
  E_SECURITYTIMER_KEYDELAY,
} SecurityTimerDelayType_e;

typedef struct
{
  uint16_t u16DID;
  uint16_t u16Len;
  pService22_DIDFunction pFun;
} RdidInfor_t;

typedef struct _WdidInfor_t
{
  uint16_t u16DID;
  uint16_t u16Len;
  pService2E_DIDFunction pFun;
  UdsSecurityLevel_e u8SecurityLevel;
} WdidInfor_t;

#if 1 //sq
typedef struct
{
  uint16_t u16DID;
  pService31_DIDFunction pFun;
  uint8_t sessionSupport;
  UdsSecurityLevel_e u8SecurityLevel;
} S31DidInfor_t;
#else
typedef struct
{
  uint16_t u16DID;
  pService31_DIDFunction pFun;
  UdsSecurityLevel_e u8SecurityLevel;
} S31DidInfor_t;
#endif

#define UDS_OFFSET_SID 0
#define UDS_OFFSET_SUB_FUNC 1

// sessionSupport bit definition
#define SESSION_BIT_DEFAULT (0x01 << 0)
#define SESSION_BIT_EXTENDED (0x01 << 1)
#define SESSION_BIT_PROGRAMMING (0x01 << 2)

#define ADDRESSING_BIT_PHYSICAL (0x01 << 0)
#define ADDRESSING_BIT_FUNCTION (0x01 << 1)

typedef struct
{
  uint8_t SID;
  uint8_t sessionSupport;
  uint8_t addressingSupport;
  pServiceFunction pFun;
} Diagnostic_t;

static int16_t Service0x10Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
static int16_t Service0x11Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
static int16_t Service0x14Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
static int16_t Service0x19Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
static int16_t Service0x28Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
static int16_t Service0x85Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
static int16_t Service0x31Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
static int16_t Service0x3EProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
// static int16_t Service0x2FProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
static int16_t Service0x27Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
static int16_t Service0x22Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
static int16_t Service0x2EProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
static int16_t Service0xBBProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);
// static int16_t Service0xBAProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag);

// static int16_t Service22ReadSeesionStatus(uint8_t* buf, uint16_t *len);

static uint8_t CommunicationControl(uint8_t ContolType, uint8_t SubService);
static void ResetTboxStatusUpdate(void);
static uint8_t g_isTesterPresent = 0; // 0未连接 1已连接
// static uint8_t g_disableSelfDiagnostics = 0; // 0: 允许自诊断 1: 禁止自诊断
#if 1 //sq
static const uint32_t g_physicalReceiveCanId = 0x18DAEBFA;
static const uint32_t g_functionalReceiveCanId = 0x18DBFFFA;
static const uint32_t g_physicalTransmitCanId = 0x18DAFAEB;
#else
static const uint32_t g_physicalReceiveCanId = 0x72D;
static const uint32_t g_functionalReceiveCanId = 0x7DF;
static const uint32_t g_physicalTransmitCanId = 0x7AD;
#endif
// static const uint8_t CanIdExtendedFlag = 0;
// static Crc32Objec_t g_crc32Object;

static const Diagnostic_t g_serviceECUFunGroup[] =
    {
        /* SID                              SEEION support                                  addressing support                      deal_function    */
        /*********Application Software*********/
        {0x10, {SESSION_BIT_DEFAULT | SESSION_BIT_EXTENDED | SESSION_BIT_PROGRAMMING}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_FUNCTION}, Service0x10Process},
        {0x11, {SESSION_BIT_DEFAULT | SESSION_BIT_EXTENDED | SESSION_BIT_PROGRAMMING}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_FUNCTION}, Service0x11Process},
        {0x14, {SESSION_BIT_DEFAULT | SESSION_BIT_EXTENDED | SESSION_BIT_EXTENDED}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_FUNCTION}, Service0x14Process},
        {0x19, {SESSION_BIT_DEFAULT | SESSION_BIT_EXTENDED | SESSION_BIT_EXTENDED}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_FUNCTION}, Service0x19Process},
        {0x28, {SESSION_BIT_EXTENDED | SESSION_BIT_EXTENDED}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_FUNCTION}, Service0x28Process},
        {0x85, {SESSION_BIT_EXTENDED | SESSION_BIT_EXTENDED}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_FUNCTION}, Service0x85Process},
        {0x31, {SESSION_BIT_EXTENDED | SESSION_BIT_PROGRAMMING}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_PHYSICAL}, Service0x31Process},
        {0x3E, {SESSION_BIT_DEFAULT | SESSION_BIT_EXTENDED | SESSION_BIT_PROGRAMMING}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_FUNCTION}, Service0x3EProcess},
        {0xBB, {SESSION_BIT_DEFAULT | SESSION_BIT_EXTENDED | SESSION_BIT_EXTENDED}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_FUNCTION}, Service0xBBProcess},
       // {0xBA, {SESSION_BIT_DEFAULT | SESSION_BIT_EXTENDED | SESSION_BIT_EXTENDED}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_FUNCTION}, Service0xBAProcess},
      //  {0x2F, {SESSION_BIT_EXTENDED | SESSION_BIT_EXTENDED}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_PHYSICAL}, Service0x2FProcess},
        {0x27, {SESSION_BIT_EXTENDED | SESSION_BIT_PROGRAMMING}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_PHYSICAL}, Service0x27Process},
        {0x22, {SESSION_BIT_DEFAULT | SESSION_BIT_EXTENDED | SESSION_BIT_PROGRAMMING}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_FUNCTION}, Service0x22Process},
        {0x2E, {SESSION_BIT_EXTENDED | SESSION_BIT_PROGRAMMING}, {ADDRESSING_BIT_PHYSICAL | ADDRESSING_BIT_PHYSICAL}, Service0x2EProcess},
};

#if 1 //sq
static const WdidInfor_t g_Service2EFunMapList[] =
    {
        /*WDID  Lenth   point_store*/   
        //静态DID     
        {0xF10A, 10, Service2EWriteFingerprintEthernetF10A, E_UDS_SECURITY_LEVEL_REPROG},        
        {0xF190, 17, Service2EWriteVINF190, E_UDS_SECURITY_LEVEL1},
        {0xF1B7, 7,  Service2EWriteManufactureDateF1B7,  E_UDS_SECURITY_LEVEL1},  //Vehicle Manufacturing Date
        {0xF1B1, 16, Service2EWritePINF1B1, E_UDS_SECURITY_LEVEL1},

        //EOL DID
        {0xF201, 1, Service2EWriteLowVoltBatteryTypeF201, E_UDS_SECURITY_LEVEL1},
        {0xF202, 1, Service2EWriteVoltPlatformF202, E_UDS_SECURITY_LEVEL1},
        {0xF203, 1, Service2EWriteEthRoleF203, E_UDS_SECURITY_LEVEL1},
    };

static const RdidInfor_t g_Service22FunMapList[] =
    {
        //静态DID
        {0xF10A, 10, Service22ReadFingerprintEthernetF10A},                                      // 指纹数据          
        {0xF187, 13, Service22ReadEcuPartNumberF187},                                              // 零件号
        {0xF18A, 10, Service22ReadSupplierIdF18A},                                                     // 供应商代码
        {0xF18B, 4, Service22ReadManufactureDateF18B},                                                // 控制器制造日期
        {0xF18C, 14, Service22ReadSerialNumberF18C},                                                  // 控制器生产序列号
        {0xF190, 17, Service22ReadVINF190},                                                             // VIN m_VIN
        {0xF193, 6, Service22ReadSupplierECUHardwareVersionF193},                                     // 控制器硬件版本
        {0xF195, 12, Service22ReadSupplierECUSoftwareVersionF195},                                    // 控制器软件版本
        {0xF1B7, 7, Service22ReadVehicleManufactureDateF1B7},                                     //车辆制造日期
        {0xF200, 1, Service22ReadSWWrittenStatusF200},                                     //该网段软件刷写支持情况
        {0xF1B0, 1, Service22ReadConfigurationDataVersionF1B0},                            //配置字版本
        {0xF1B1, 16, Service22ReadPINF1B1},                                                //PIN码     
        {0xF1B2, 16, Service22ReadChipIdF1B2},                                                //芯片ID     
        {0xF1B3, 64, Service22ReadPublicKeyF1B3},                                                //公钥

        //动态DID
        {0xF300, 1, Service22ReadTerminalActiveStateF300},                               //终端激活状态
      
        //EOL DID
        {0xF201, 1, Service22ReadLowVoltBatteryTypeF201},                           //低压电池类型
        {0xF202, 1, Service22ReadVoltPlatformF202},                                //电压平台
        {0xF203, 1, Service22ReadEthRoleF203},                                //以太网主从节点
    };

static const uint16_t g_passthroughDidList_22[] = {
     //sq - EOL检测项DID透传到MPU
    0x1203, // ICCID
    0x1204, // IMSI
    0x1205, // IMEI
    0x1206, // APN1
    0x1207, // APN2
    0x1208, // APN3
    0x1211, // EMMC状态    
    0x1218, // Tbox SN
  };
#else
static const WdidInfor_t g_Service2EFunMapList[] =
    {
        /*WDID  Lenth   point_store*/        
        {0xF190, 17, Service2EWriteVIN, E_UDS_SECURITY_LEVEL1},
        //{0xF1A1, 17, Service2EWriteVehicleNetworkConfigurationF1A1, E_UDS_NONE_SECURITY_LEVEL},
        {0xF1B0, 31, Service2EWriteVehicleNetworkConfigurationF1B0, E_UDS_SECURITY_LEVEL1},
        {0xF1B1, 31, Service2EWriteECUlevelNetworkConfigurationDataIdentifierF1B1, E_UDS_SECURITY_LEVEL1},
        {0xF112, 20, Service2EWriteVehicleModelF112, E_UDS_SECURITY_LEVEL1},
        {0xF1F0, 26, Service2EWriteFingerprintF1F0, E_UDS_SECURITY_LEVEL1},
        {0xF0FF, 48, Service2EWriteFingerprintEthernetF0FF, E_UDS_SECURITY_LEVEL1},
        {0xF1AB, 10, Service2EWriteVehicleSoftwareVersionF1AB, E_UDS_SECURITY_LEVEL1},
        {0xF1C2, 1, Service2EWriteWorkingMode, E_UDS_SECURITY_LEVEL1},
        {0xFD00, 1, Service2EWriteMaintenanceModeFD00, E_UDS_SECURITY_LEVEL1},
        {0xF1C7, 16, Service2EWriteSecOCKeyF1C7, E_UDS_SECURITY_LEVEL1},
        {0xF1CB, 1000, Service2EWriteCertificateF1CB, E_UDS_SECURITY_LEVEL1},

        {0xDF7B, 16, Service2EWriteSK, E_UDS_SECURITY_LEVEL1},
        {0x88BB, 4, Service2EWriteManufactureDateF18B, E_UDS_SECURITY_LEVEL1},
        {0xAA11, 2, Service2EWriteTripRestAA11, E_UDS_SECURITY_LEVEL1},

        // {0xF1B2,     4,       Service2EWriteInstallationDate,               E_UDS_NONE_SECURITY_LEVEL},  //Vehicle Manufacturing Date
        // {0xF184,     9,       Service2EWriteAppSoftwareFingerprint,         E_UDS_NONE_SECURITY_LEVEL},
        // {0xF1B9,     4,       Service2EWriteSubnetConfigListOnHighSpeedCan, E_UDS_NONE_SECURITY_LEVEL},

        {0x010D, 16, Service2EWriteESKey, E_UDS_SECURITY_LEVEL2_STD}, // ESK
        // {0xC110,     1,       Service2EWriteDiagnosticCanReport,    	E_UDS_NONE_SECURITY_LEVEL},  //Diagnostic CAN report
        // {0xC101,     12,      Service2EWriteEOLconfig, 			E_UDS_NONE_SECURITY_LEVEL},

        //{0xCF1E,     16,      Service2EWriteProlinesChannelKey,             E_UDS_NONE_SECURITY_LEVEL},
        //{0xCF1F,     1,       Service2EWritePhoneKeyOffLineUseTimes,        E_UDS_NONE_SECURITY_LEVEL},
        //{0xCF20,     12,      Service2EWriteNFCCardID,                      E_UDS_NONE_SECURITY_LEVEL},
        //{0xF18C, 45, Service2EWriteSerialNumber, E_UDS_SECURITY_LEVEL1},       // 0xF18C_cxl
        {0x011B, 32, Service2EWriteApn1, E_UDS_SECURITY_LEVEL1},               // 0x011B_cxl
        {0x011C, 16, Service2EWriteIp1Addr, E_UDS_SECURITY_LEVEL1},            // 0x011C_cxl
        {0x011D, 8, Service2EWriteTspPort, E_UDS_SECURITY_LEVEL1},             // 0x011D_cxl
        {0xB209, 1, Service2EWriteTspTimeout, E_UDS_SECURITY_LEVEL1},          // 0xB209_cxl
        {0x011F, 14, Service2EWriteECallNum1, E_UDS_SECURITY_LEVEL1},          // 0x011F_cxl
        {0x0124, 14, Service2EWriteBCallNum1, E_UDS_SECURITY_LEVEL1},          // 0x0124_cxl
        {0xB20A, 14, Service2EWriteOfficialServiceNum, E_UDS_SECURITY_LEVEL1}, // 0xB20A_cxl
        {0xB20B, 14, Service2EWriteEmergencyAsNum, E_UDS_SECURITY_LEVEL1},     // 0xB20B_cxl
        {0x0129, 14, Service2EWriteTboxPhoneNum, E_UDS_SECURITY_LEVEL1},       // 0x0129_cxl
        {0xB20C, 1, Service2EWriteTspFlowTimeout, E_UDS_SECURITY_LEVEL1},      // 0xB20C_cxl
        {0xB20D, 1, Service2EWriteTspAckTime, E_UDS_SECURITY_LEVEL1},          // 0xB20D_cxl
        {0x013C, 32, Service2EWriteApn2, E_UDS_SECURITY_LEVEL1},               // 0x013C_cxl
        {0xB20E, 16, Service2EWriteIp2Addr, E_UDS_SECURITY_LEVEL1},            // 0xB20E_cxl
        {0xB20F, 8, Service2EWritePort2, E_UDS_SECURITY_LEVEL1},               // 0xB20F_cxl
        {0x105F, 16, Service2EWriteIp3Addr, E_UDS_SECURITY_LEVEL1},            // 0x105F_cxl
        {0xB211, 8, Service2EWriteLongConnCycle, E_UDS_SECURITY_LEVEL1},       // 0xB211_cxl
        {0xB229, 2, Service2EWriteNormalUploadPeriod, E_UDS_SECURITY_LEVEL1},  // 0xB229_cxl
        {0xB22A, 2, Service2EWriteAlarmUploadPeriod, E_UDS_SECURITY_LEVEL1},   // 0xB22A_cxl
        {0xB22B, 1, Service2EWriteTspDomainLen, E_UDS_SECURITY_LEVEL1},        // 0xB22B_cxl
        {0xB22C, 50, Service2EWriteTspDomain, E_UDS_SECURITY_LEVEL1},          // 0xB22C_cxl
        {0xB22D, 2, Service2EWriteTspPortNumeric, E_UDS_SECURITY_LEVEL1},      // 0xB22D_cxl
        {0x1014, 50, Service2EWritePublicTspDomain, E_UDS_SECURITY_LEVEL1},    // 0x1014_cxl
        {0xB247, 1, Service2EWriteApn1Type, E_UDS_SECURITY_LEVEL1},            // 0xB247_cxl
        {0xB248, 1, Service2EWriteApn2Type, E_UDS_SECURITY_LEVEL1},            // 0xB248_cxl
        {0xB258, 1, Service2EWriteTspConnectSetting, E_UDS_SECURITY_LEVEL1},   // 0xB258_cxl
        {0xB259, 1, Service2EWriteEmmcDeleteControl, E_UDS_SECURITY_LEVEL1},   // 0xB259_cxl
        {0x031C, 50, Service2EWriteTspDomain1, E_UDS_SECURITY_LEVEL1},         // 0x031C_cxl
        {0x1061, 32, Service2EWriteApn3, E_UDS_SECURITY_LEVEL1},               // 0x1061_cxl
        {0xB2CB, 50, Service2EWriteOtaDomainName, E_UDS_SECURITY_LEVEL1},      // 0xB2CB_cxl //104F_cxl
        //{0x104F, 50, Service2EWriteOtaDomainName, E_UDS_SECURITY_LEVEL1},      // 0xB2CB_cxl //104F_cxl

        {0xB2CC, 8, Service2EWriteOtaPort, E_UDS_SECURITY_LEVEL1},               // 0xB2CC_cxl
        //{0x1050, 8, Service2EWriteOtaPort, E_UDS_SECURITY_LEVEL1},               // 0x1050_cxl B2CC_cxl
        {0x1052, 50, Service2EWritePkiDomainName, E_UDS_SECURITY_LEVEL1},        // 0x1052_cxl
        {0x1053, 8, Service2EWritePkiPort, E_UDS_SECURITY_LEVEL1},               // 0x1053_cxl
        {0x105E, 8, Service2EWriteTspPort3, E_UDS_SECURITY_LEVEL1},              // 0x105E_cxl
        {0xB275, 100, Service2EWriteLogDomainName, E_UDS_SECURITY_LEVEL1},       // 0xB275_cxl
        {0xB276, 8, Service2EWriteLogPort, E_UDS_SECURITY_LEVEL1},               // 0xB276_cxl
        {0xB277, 16, Service2EWriteLogConnectionCommand, E_UDS_SECURITY_LEVEL1}, // 0xB277_cxl
        {0xB212, 4, Service2EWriteEthernetConfig, E_UDS_SECURITY_LEVEL1},        // 0xB212_cxl
        {0xB278, 1, Service2EWritePkiCertCycle, E_UDS_SECURITY_LEVEL1},          // 0xB278_cxl
        {0xB27C, 50, Service2EWriteLogAccount, E_UDS_SECURITY_LEVEL1},           // 0xB27C_cxl
        {0xB27D, 50, Service2EWriteLogPassword, E_UDS_SECURITY_LEVEL1},          // 0xB27D_cxl
        {0xB27F, 1, Service2EWriteLogUploadChannel, E_UDS_SECURITY_LEVEL1},      // 0xB27F_cxl
        {0xB280, 50, Service2EWriteLogDomainFront, E_UDS_SECURITY_LEVEL1},       // 0xB280_cxl
        {0xB281, 50, Service2EWriteLogDomainLater, E_UDS_SECURITY_LEVEL1},       // 0xB281_cxl
        {0xB282, 256, Service2EWriteLogPath, E_UDS_SECURITY_LEVEL1},             // 0xB282_cxl
        {0xB283, 64, Service2EWriteLogPath_P1, E_UDS_SECURITY_LEVEL1},           // 0xB283_cxl
        {0xB284, 64, Service2EWriteLogPath_P2, E_UDS_SECURITY_LEVEL1},           // 0xB284_cxl
        {0xB285, 64, Service2EWriteLogPath_P3, E_UDS_SECURITY_LEVEL1},           // 0xB285_cxl
        {0xB286, 64, Service2EWriteLogPath_P4, E_UDS_SECURITY_LEVEL1},           // 0xB286_cxl
        {0xB28B, 1, Service2EWriteApn3Type, E_UDS_SECURITY_LEVEL1},              // 0xB28B_cxl
        {0xB2C5, 16, Service2EWriteSensitiveDataSet, E_UDS_SECURITY_LEVEL1},     // 0xB2C5_cxl
        {0xB2C6, 8, Service2EWriteOtaRtcWakeupTime, E_UDS_SECURITY_LEVEL1},      // 0xB2C6_cxl
        {0xB2C7, 4, Service2EWriteBuryingPointSwitch, E_UDS_SECURITY_LEVEL1},    // 0xB2C7_cxl

        {0xB296, 1, Service2EWriteAdbStatus, E_UDS_SECURITY_LEVEL1},                 // 0xB296_cxl
        {0xB297, 50, Service2EWriteConnectionTestAddr, E_UDS_SECURITY_LEVEL1},       // 0xB297_cxl
        {0xB289, 128, Service2EWriteDataBuriedPointDomain, E_UDS_SECURITY_LEVEL1},   // 0xB289_cxl
        {0xB29C, 64, Service2EWriteDataBuriedPointDomain_P1, E_UDS_SECURITY_LEVEL1}, // 0xB29C_cxl
        {0xB29D, 64, Service2EWriteDataBuriedPointDomain_P2, E_UDS_SECURITY_LEVEL1}, // 0xB29D_cxl
        {0xB2C9, 1, Service2EWriteGnssGalaxy, E_UDS_SECURITY_LEVEL1},                // 0xB2C9_cxl
        {0xB2CA, 6, Service2EWriteOtaBookingTime, E_UDS_SECURITY_LEVEL1},            // 0xB2CA_cxl
        {0xB2CD, 1, Service2EWriteOtaSwAcqTime, E_UDS_SECURITY_LEVEL1},              // 0xB2CD_cxl

        {0xB2E6, 64, Service2EWriteFunctionConfig, E_UDS_SECURITY_LEVEL1}, // 0xB2E6_cxl
        {0x0110, 1, Service2EWriteManufactoryMode, E_UDS_SECURITY_LEVEL1}, // 0x0110_cxl
        {0xB2B4, 1, Service2EWriteTransportMode, E_UDS_SECURITY_LEVEL1},   // 0xB2B4_cxl
        {0x0120, 12, Service2EWriteDtcSettingControl, E_UDS_SECURITY_LEVEL1}, // 0x0120_cxl
        {0x0111, 20, Service2EWriteICCID, E_UDS_SECURITY_LEVEL1}, // 0x0111_cxl
        {0X010E, 15, Service2EWriteIMEI, E_UDS_SECURITY_LEVEL1},  // 0x010E_cxl
        {0X010F, 15, Service2EWriteIMSI, E_UDS_SECURITY_LEVEL1},  // 0x010F_cxl
        //{0xF187,14,Service2EWriteGacEcuPartNumber,E_UDS_SECURITY_LEVEL1}, // 0xF187_cxl
        {0xB208, 2, Service2EWriteRtcWakeupSet, E_UDS_SECURITY_LEVEL1},
        {0xB25F, 1, Service2EWrite4GModuleRsetTime, E_UDS_SECURITY_LEVEL1},
        {0x0200, 2, Service2EWriteReprogramCounter, E_UDS_SECURITY_LEVEL1},
        {0x0201, 2, Service2EWriteReprogramAttemptCounter, E_UDS_SECURITY_LEVEL1},
        {0xB2C8, 3, Service2EWriteTcpdumpSwitch, E_UDS_SECURITY_LEVEL1},
        {0xB28E, 1, Service2EWriteTspPlatformConnectionStatus, E_UDS_SECURITY_LEVEL1},
        {0xB261, 1, Service2EWritePKIapply, E_UDS_SECURITY_LEVEL1},
        {0xB25C, 1, Service2ETboxCurrentRunningMode, E_UDS_SECURITY_LEVEL1},
        {0xB2E5, 1, Service2EWriteJtagStatus, E_UDS_SECURITY_LEVEL1},
};
// 读需要透传
static const uint16_t g_passthroughDidList_22[] = {
    // 0xB2B4,
    // 0xB2B5,
    // 0x0110,
    // 0xF193,
    // 0xF195,
    0x031C,
    // 0xB205,
    // 0xB206,
    // 0x010E,
    // 0x010F,
    // 0x0111,
    // 0x0120,
    0x0200,
    0x0201,
    0xB208,
    0x011B,
    0x011C,
    0x011D,
    0xB209,
    0x011F,
    0x0124,
    0xB20A,
    0xB20B,
    0xB20C,
    0XB20D,
    0x0129,
    0x013C,
    0xB20E,
    0xB20F,
    0x105F,
    0xB211,
    0xB212,
    0x1014,
    0xB247,
    0xB248,
    0xB24A, // EMMC State
    0xB24C, // 4G Firmware Version
    0xB24D, // 4G Hardware Version
    0xB256, // Ethernet ID
    0xB258,
    0XB259,
    0xB25B,
    0xB25C,
    0xB25F,
    0xB260, // 4G Reset Count
    0XB261,
    0xB262, // PKI Certificate Status
    0x1052,
    0x1053,
    // 0XB271,
    0xB275,
    0xB276,
    0XB277,
    0XB278,
    0XB279,
    0xB27C,
    0xB27D,
    0xB27F,
    0xB280,
    0xB281,
    0xB282,
    0xB283,
    0xB284,
    0xB285,
    0xB286,
    0x105E,
    0x1061,
    0xB28B,
    0xB2C5,
    0xB2C6,
    0xB2C7,
    0xB28E, // TSP Connection Status
    // 0x0120,
    0x0200,
    0x0201,
    0xB209,
    0xB259,
    // 0xB25B,
    0xB261,
    0xB2C8,
    0xB296,
    0xB297,
    0xB289,
    0xB29C,
    0xB29D,
    0xB2C9,
    0xB2CA,
    0xB2CB,
    0xB2CC,
    0xB2CD,
    0x5005,
    0x2110,
    0x2111,
    0xB2E6,
    0x1014,
    0x011F,
    0x0124,   
};
static const RdidInfor_t g_Service22FunMapList[] =
    {
        {0xF0FF, 48, Service22ReadFingerprintEthernetF0FF},                                       // ethernet fingerprint	ASCII	48
        {0xF10B, 4, Service22ReadGacDiagParamVersion},                                            // 0xF10B_cxl
        {0xF17F, 17, Service22ReadGacSparePartNumber},                                            // 0xF17F_cxl
        {0xF180, 17, Service22ReadBootSoftwarePartNumber},                                        // 0xF180_cxl
        {0xF187, 14, Service22ReadGacEcuPartNumber},                                              // 0xF187_cxl
        {0xF189, 17, Service22ReadVehicleManufacturerECUSoftwareVersionNumberDataIdentifierF189}, // 0xF189_cxl
        {0xF18E, 14, Service22ReadGacVisteonInHousePartNumber},                                   // 0xF18E_cxl
        {0xF193, 6, Service22ReadSupplierECUHardwareVersion},                                     // 0xF193_cxl
        {0xF195, 6, Service22ReadSupplierECUSoftwareVersion},                                     // 0xF195_cxl
        {0x1000, 1, Service22ReadPowerVoltage},                                                   // 0x1000_cxl
        {0xF18A, 9, Service22ReadSupplierId},                                                     // 0xF18A_cxl
        {0xB201, 2, Service22Read4G3GStatus},                                                     // 0xB201_cxl
        {0xB202, 2, Service22ReadGpsStatus},                                                      // 0xB202_cxl
        {0xB203, 7, Service22ReadSystemTime},                                                     // 0xB203_cxl
        {0xB204, 1, Service22ReadBcallEcallKeyStatus},                                            // 0xB204_cxl
        {0xB205, 2, Service22ReadVehicleSpeed},                                                   // 0xB205_cxl
        {0xB206, 2, Service22ReadEngineMotorSpeed},                                               // 0xB206_cxl
        {0x010D, 16, Service22ReadESKey},                                                         // 0x010D_cxl
        {0x0204, 1, Service22ReadESKWrittenStatus},                                               // 0x0204_cxl
        {0x010E, 15, Service22ReadIMEI},                                                          // 0x010E_cxl
        {0x010F, 15, Service22ReadIMSI},                                                          // 0x010F_cxl
        {0x0111, 20, Service22ReadICCID},                                                         // 0x0111_cxl
        {0x011B, 32, Service22ReadApn1},                                                          // 0x011B_cxl
        {0x011C, 16, Service22ReadIp1Addr},                                                       // 0x011C_cxl
        {0x011D, 2, Service22ReadTspPort},                                                        // 0x011D_cxl
        {0xB209, 1, Service22ReadTspTimeout},                                                     // 0xB209_cxl
        {0x011F, 14, Service22ReadECallNum1},                                                     // 0x011F_cxl
        {0x0124, 1, Service22ReadBCallNum1},                                                      // 0x0124_cxl
        {0xB20A, 14, Service22ReadOfficialServiceNum},                                            // 0xB20A_cxl
        {0xB20B, 14, Service22ReadEmergencyAsNum},                                                // 0xB20B_cxl
        {0x0129, 14, Service22ReadTboxPhoneNum},                                                  // 0xB129_cxl
        {0xB20C, 1, Service22ReadTspFlowTimeout},                                                 // 0xB20C_cxl
        {0xB20D, 1, Service22ReadTspAckTime},                                                     // 0XB20D_cxl
        {0x013C, 32, Service22ReadApn2},                                                          // 0x013C_cxl
        {0xB20E, 16, Service22ReadIp2Addr},                                                       // 0xB20E_cxl
        {0xB20F, 8, Service22ReadPort2},                                                          // 0xB20F_cxl
        {0xB210, 4, Service22Read4GNetworkStatus},                                                // 0xB210_cxl
        {0x105F, 16, Service22ReadIp3Addr},                                                       // 0x105F_cxl
        {0xB211, 8, Service22ReadLongConnCycle},                                                  // 0xB211_cxl
        {0xB212, 4, Service22ReadEthernetConfig},                                                 // 0xB212_cxl
        {0xB229, 2, Service22ReadNormalUploadPeriod},                                             // 0xB229_cxl
        {0xB22A, 2, Service22ReadAlarmUploadPeriod},                                              // 0xB22A_cxl
        {0xB22B, 1, Service22ReadTspDomainLen},                                                   // 0xB22B_cxl
        {0xB22C, 50, Service22ReadTspDomain},                                                     // 0xB22C_cxl
        {0xB22D, 2, Service22ReadTspPortNumeric},                                                 // 0xB22D_cxl
        {0x1014, 50, Service22ReadPublicTspDomain},                                               // 0x1014_cxl
        {0xB245, 2, Service22ReadTboxWakeupSource},                                               // 0xB245_cxl
        {0xB247, 1, Service22ReadApn1Type},                                                       // 0xB247_cxl
        {0xB248, 1, Service22ReadApn2Type},                                                       // 0xB248_cxl
        {0xB249, 30, Service22ReadGpsSatelliteInfo},                                              // 0xB249_cxl
        {0xB24A, 1, Service22ReadEmmcState},                                                      // 0xB24A_cxl
        {0xB24C, 20, Service22Read4gFirmwareVersion},                                             // 0xB24C_cxl
        {0xB24D, 20, Service22Read4gHardwareVersion},                                             // 0xB24D_cxl
        {0xB24E, 2, Service22Read4gFunctionState},                                                // 0xB24E_cxl
        {0xB256, 5, Service22ReadEthernetId},                                                     // 0xB256_cxl
        {0xB258, 1, Service22ReadTspConnectSetting},                                              // 0xB258_cxl
        {0xB259, 1, Service22ReadEmmcDeleteStatus},                                               // 0xB259_cxl
        {0xB25B, 17, Service22Read4gAppVersion},                                                  // 0xB25B_cxl
        {0xB261, 1, Service22ReadPkiApply},                                                       // 0xB261_cxl
        {0xB262, 1, Service22ReadPkiCertStatus},                                                  // 0xB262_cxl
        {0xF18C, 45, Service22ReadSerialNumber},                                                  // 0xF18F_cxl
        {0xB266, 2, Service22ReadGnssAntennaVoltage},                                             // 0xB266_cxl
        {0xB275, 100, Service22ReadLogDomainName},                                                // 0xB275_cxl
        {0xB276, 8, Service22ReadLogPort},                                                        // 0xB276_cxl
        {0xB277, 16, Service22ReadLogConnectionCommand},                                          // 0xB277_cxl
        {0xB278, 1, Service22ReadPkiCertCycle},                                                   // 0xB278_cxl
        {0xB279, 5, Service22ReadPkiSdkVersion},                                                  // 0xB279_cxl
        {0xB27C, 50, Service22ReadLogAccount},                                                    // 0xB27C_cxl
        {0xB27D, 50, Service22ReadLogPassword},                                                   // 0xB27D_cxl
        {0xB27F, 1, Service22ReadLogUploadChannel},                                               // 0xB27F_cxl
        {0xB280, 50, Service22ReadLogDomainFront},                                                // 0xB280_cxl
        {0xB281, 50, Service22ReadLogDomainLater},                                                // 0xB281_cxl
        {0xB282, 256, Service22ReadLogPath},                                                      // 0xB282_cxl
        {0xB283, 64, Service22ReadLogPath_P1},                                                    // 0xB283_cxl
        {0xB284, 64, Service22ReadLogPath_P2},                                                    // 0xB284_cxl
        {0xB285, 64, Service22ReadLogPath_P3},                                                    // 0xB285_cxl
        {0xB286, 64, Service22ReadLogPath_P4},                                                    // 0xB286_cxl
        {0xB287, 2, Service22ReadPwmSignal},                                                      // 0xB287_cxl
        {0x105E, 8, Service22ReadTspPort3},                                                       // 0x105E_cxl
        {0x1061, 32, Service22ReadApn3},                                                          // 0x1061_cxl
        {0xB28B, 1, Service22ReadApn3Type},                                                       // 0xB28B_cxl
        {0xB2C5, 16, Service22ReadSensitiveDataSet},                                              // 0xB2C5_cxl
        {0xB2C6, 8, Service22ReadOtaRtcWakeupTime},                                               // 0xB2C6_cxl
        {0xB2C7, 4, Service22ReadBuryingPointSwitch},                                             // 0xB2C7_cxl
        {0xB28E, 1, Service22ReadTspConnectionStatus},                                            // 0xB28E_cxl
        {0xB290, 16, Service22ReadGpsDiagInfo},                                                   // 0xB290_cxl
        {0xB296, 1, Service22ReadAdbStatus},                                                      // 0xB296_cxl
        {0xB297, 50, Service22ReadConnectionTestAddr},                                            // 0xB297_cxl
        {0xB289, 128, Service22ReadDataBuriedPointDomain},                                        // 0xB289_cxl
        {0xB29C, 64, Service22ReadDataBuriedPointDomain_P1},                                      // 0xB29C_cxl
        {0xB29D, 64, Service22ReadDataBuriedPointDomain_P2},                                      // 0xB29D_cxl
        {0xB2C9, 1, Service22ReadGnssGalaxy},                                                     // 0xB2C9_cxl
        {0xB2CA, 6, Service22ReadOtaBookingTime},                                                 // 0xB2CA_cxl
        {0xB2CD, 1, Service22ReadOtaSwAcqTime},                                                   // 0xB2CD_cxl
        {0xB2E5, 1, Service22ReadJtagStatus},                                                     // 0xB2E5_cxl
        {0xB2E6, 64, Service22ReadFunctionConfig},                                                // 0xB2E6_cxl
        {0xB257, 2, Service22ReadEthLineFaultInfo},                                               // 0xB257_cxl
        {0xB2CB, 50, Service22ReadOtaDomainName},                                                 //   0xB2CB_cxl //104F_cxl
        //{0x104F, 50, Service22ReadOtaDomainName},                                                 // 0xB2CB_cxl //104F_cxl
        {0xB2CC, 8, Service22ReadOtaPort},                                                        //   0xB2CC_cxl
        //{0x1050, 8, Service22ReadOtaPort},                                                        // 0x1050_cxl B2CC_cxl
        {0x1052, 50, Service22ReadPkiDomainName},                                                 //   0x1052_cxl
        {0x1053, 8, Service22ReadPkiPort},                                                        //    0x1053_cxl
        {0x0110, 1, Service22ReadManufactoryMode},                                                // 工厂模式 0x0110_cxl
        {0xB2B4, 1, Service22ReadTransportMode},                                                  // 运输模式   0xB2B4_cxl
        {0xB2B5, 1, Service22ReadKeySt},                                                          // 电子钥匙状态   0xB2B5_cxl
        {0x0120, 12, Service22ReadDtcSettingControl},                                             // DTC使能控制 0x0120_cxl
        {0xF186, 1, Service22ReadActiveDiagnosticSession},    // 0xF186_cxl
        {0x031C, 50, Service22ReadTspDomain1},                // 0x031C_cxl
        //{0x5001, 1, Service22ReadVehicleMode},                // 车辆模式 0x5001_cxl
        {0xB26B, 1, Service22ReadSpiCommunicationBetweenMCU}, // MCU与SPI通信状态 0xB26B_cxl
        {0xB271, 1, Service22ReadOpenCpuTemperature},
        {0xF0FF, 48, Service22ReadFingerprintEthernetF0FF}, // ethernet fingerprint	ASCII	48
        {0xF112, 20, Service22ReadVehicleModelF112},        // vehiclemodel 	ASCII	20
        //     {0xF180, 4,  Service22ReadBootSoftwareVersion       },      //
        //     {0xF184, 9,  Service22ReadFingerPrint               },      //applicationSoftwareFingerprint
        //     {0xF184, 9,  Service22ReadAppSoftwareFingerprint    },      //applicationSoftwareFingerprint

        //    {0xF186,  1,  Service22ReadSeesionStatus            },       //诊断活动会话
        //    {0xF188,  4,  Service22ReadSoftwareNumber           },      //供应商 ECU 软件号
        //{0xF189, 15, Service22ReadVehicleManufacturerECUSoftwareVersionNumberDataIdentifierF189}, // 长城软件版本号	ASCII	15
        //{0xF18A, 7, Service22ReadSupplierId},                                                     // 系统供应商标识号
        {0xF18B, 4, Service22ReadManufactureDate}, // ECU制造日期
        {0xF190, 17, Service22ReadVIN},            // VIN m_VIN
        //{0xF191, 4,  Service22ReadHardwareNumber            },        //供应商 ECU 硬件号
        //{0xF192, 10,  Service22ReadSupplierHardwareNumber   },        //供应商 ECU 硬件号
        //{0xF193, 4, Service22ReadHardwareVersion}, // 供应商ECU硬件版本号
        //{0xF194, 10,  Service22ReadSupplierSoftwareNumber   },        //供应商 ECU 软件号
        {0xF195, 8, Service22ReadSoftwareVersion}, // 供应商ECU软件版本号

        //{0xF1A1, 17, Service22ReadVehicleNetworkConfigurationF1A1},
        //{0xF1A2, 9, Service22ReadCANmatrixVersionF1A2},
        {0xF1AB, 10, Service22ReadVehicleSoftwareVersionF1AB}, // vehicle software version
        {0xF1B0, 31, Service22ReadVehicleNetworkConfigurationF1B0},
        {0xF1B1, 31, Service22ReadECUlevelNetworkConfigurationDataIdentifierF1B1}, // 单ECU网络配置 HEX	31
        {0xF1B2, 4, Service22ReadSupplierId},                                      // hardware SupplierId
        {0xF1B3, 9, Service22ReadNodeAddressF1B3},
        {0xF1B4, 31, Service22ReadCompID1F1B4},
        {0xF1B5, 10, Service22ReadNameF1B5},
        {0xF1B6, 8, Service22ReadToolVersionF1B6},          // tool version ASCII	8
        {0xF1B9, 4, Service22ReadCompID2F1B9},              // not completed
        {0xF1BC, 10, Service22ReadSoftwareInformationF1BC}, // Software Information 10
        {0xF1BD, 10, Service22ReadHardwareInformationF1BD}, // Hardware Information 10
        {0xF1C2, 1, Service22ReadWorkingMode},
        {0xF1C5, 20, Service22ReadUINF1C5},               // UIN  ASCII	20
        {0xF1C8, 5, Service22ReadSecOCVersionNumberF1C8}, // 终端安全序列号	ASCII	20
        {0xF1CA, 1, Service22ReadPkiStateF1CA},
        {0xF1CD, 20, Service22ReadCsrHashF1CD},
        {0xF1D9, 1, Service22ReadECUTypeF1D9},           // ECU type 1
        {0xF1F0, 26, Service22ReadFingerprintF1F0},      //?fingerprint	ASCII	26
        {0xFD00, 1, Service22ReadMaintenanceModeFD00},   // 维修模式
        {0xFD03, 15, Service22ReadUploadFrequency},      // 国际移动用户识别码	ASCII	15  IMEI
        {0x7406, 15, Service22ReadPhoneSignalTest},      // 国际移动设备辨识码	ASCII	15  IMSI
        {0x740C, 20, Service22ReadICCID},                // 集成电路卡识别码	ASCII	20
        {0x600D, 1, Service22ReadGPSRNetworkStatus600D}, //
        {0x600E, 1, Service22ReadGPSRNetworkSignalStrength600E},
        {0xF1C0, 15, Service22ReadCalibrationSoftwareVersionF1C0},
        {0xF1C1, 18, Service22ReadSoftwareVersion},
        {0xF1C9, 840, Service22ReadCsrF1C9},
        {0xF1CC, 16, Service22ReadCertificateSerialNumberF1CC},
        {0xDF7B, 16, Service22ReadSK},
        {0xCC77, 16, Service22ReadSecocKeyCC77}, // 终端安全序列号	ASCII	20
        {0xAA11, 8, Service22ReadTripRestAA11},
        {0xAA01, 1, Service22ReadEmmcStateAA01},
};
#endif
#if 1 //SQ
static const S31DidInfor_t g_routineStartDidMap[] =
  {
        {0xDF01, Service31StartRoutineCheckProgramPreConditions, SESSION_BIT_EXTENDED, E_UDS_NONE_SECURITY_LEVEL},  // 检查编程预条件
        {0xDF02, Service31StartRoutineCheckProgramIntegrity, SESSION_BIT_PROGRAMMING, E_UDS_SECURITY_LEVEL_REPROG},  // 编程完整性检查
      	{0xFF00, Service31StartRoutineEraseMemory, SESSION_BIT_PROGRAMMING, E_UDS_SECURITY_LEVEL_REPROG},  // 启动擦除内存
      	{0xFF01, Service31StartRoutineCheckProgrammingDependencies, SESSION_BIT_PROGRAMMING, E_UDS_SECURITY_LEVEL_REPROG}  // 编程依赖性检查

  };

static const S31DidInfor_t g_routineStopDidMap[] =
  {
  
  };

static const S31DidInfor_t g_routineResultDidMap[] =
  {
   
  };

#else
static const S31DidInfor_t g_routineStartDidMap[] =
    {
        {0xDF7A, Service31StartRoutineWritePIN, E_UDS_SECURITY_LEVEL1},  // 写入PIN
        {0xDF7B, Service31StartRoutineWriteSK, E_UDS_SECURITY_LEVEL1},   // 写入SK
        {0xDF7C, Service31StartRoutineVerifyPIN, E_UDS_SECURITY_LEVEL1}, // 验证PIN码，EOL发送给TBOX这条消息，让TBOX与KBCM验证PIN码
        {0xDF7D, Service31StartRoutineVerifySK, E_UDS_SECURITY_LEVEL1},
        {0xDFD6, Service31StartRoutineGetLearn, E_UDS_SECURITY_LEVEL1},
        {0xDF75, Service31StartRoutineVerifyEolPIN, E_UDS_SECURITY_LEVEL1},
        {0xDF78, Service31StartRoutineClearPINSK, E_UDS_SECURITY_LEVEL1},
        {0xCE01, Service31StartRoutinePKI, E_UDS_SECURITY_LEVEL1},               // PKI触发流程
                                                                                 //  {0xF002, Service31StopApplication             , E_UDS_SECURITY_LEVEL1 },
                                                                                 //  {0xF005, Service31WriteDiagnosticCanReport01  ,     E_UDS_SECURITY_LEVEL1 },
                                                                                 //{0xB2D0, Service31StartRoutineDiagDefault, E_UDS_SECURITY_LEVEL1}, // DID,DTC恢复默认值
        {0xB2D3, Service31StartRoutineDTC, E_UDS_SECURITY_LEVEL1},               // DTC计数器
                                                                                 //{0xB2E5, Service31StartRoutineJTAG, E_UDS_SECURITY_LEVEL1},        // JTAG调试口状态控制
        {0x1101, Service31StartRoutineReadEthStatus, E_UDS_SECURITY_LEVEL1}, // 读取ETH收发器状态
        //{0x1104, Service31StartRoutineEthMode, E_UDS_SECURITY_LEVEL1},           // 设置ETH通信模式
        //{0x1107, Service31StartRoutineEthTestMode, E_UDS_SECURITY_LEVEL1},       // 设置ETH测试模式
        {0x110B, Service31StartRoutineCableDiag, E_UDS_SECURITY_LEVEL1}, // 运行并读取线缆诊断结果
        {0x220B, Service31StartRoutinePkiTest, E_UDS_SECURITY_LEVEL1},   // PKI自测试
        {0xB2E6, Service31StartRoutineDdrTest, E_UDS_SECURITY_LEVEL1}    // 模组DDR测试

};

// static const S31DidInfor_t g_routineStopDidMap[] =
// {
//   {0xF005, Service31WriteDiagnosticCanReport01  ,     E_UDS_SECURITY_LEVEL1 },
// };

static const S31DidInfor_t g_routineResultDidMap[] =
    {
        {0xDF7A, Service31RequestRoutineResultsReadPIN, E_UDS_SECURITY_LEVEL1},   // 读PIN
        {0xDF7B, Service31RequestRoutineResultsReadSK, E_UDS_SECURITY_LEVEL1},    // 读SK
        {0xDF7C, Service31RequestRoutineResultsVerifyPIN, E_UDS_SECURITY_LEVEL1}, // 获取验证结果
        {0xDF7D, Service31RequestRoutineResultsVerifySK, E_UDS_SECURITY_LEVEL1},
        {0xDFD6, Service31RequestRoutineResultsGetLearn, E_UDS_SECURITY_LEVEL1},
        {0xDF75, Service31RequestRoutineResultsVerifyEolPIN, E_UDS_SECURITY_LEVEL1},
        {0xDF78, Service31RequestRoutineResultsClearPINSK, E_UDS_SECURITY_LEVEL1},
        {0xCE01, Service31RequestRoutineResultsPKI, E_UDS_SECURITY_LEVEL1}, // 获取PKI状态
                                                                            //  {0xF005, Service31WriteDiagnosticCanReport03  ,     E_UDS_SECURITY_LEVEL1 },
                                                                            //{0xB2D0, Service31RequestRoutineResultsDiagDefault, E_UDS_SECURITY_LEVEL1}, // DID,DTC恢复默认值
                                                                            //{0xB2E5, Service31RequestRoutineResultsJTAG, E_UDS_SECURITY_LEVEL1},        // JTAG调试口状态控制
        {0x1104, Service31ResultRoutineEthMode, E_UDS_SECURITY_LEVEL1},     // 读取ETH通信模式
        {0x1107, Service31ResultRoutineEthTestMode, E_UDS_SECURITY_LEVEL1}, // 读取ETH测试模式
        {0x220B, Service31ResultRoutinePkiTest, E_UDS_SECURITY_LEVEL1},     // 获取PKI自测试结果
        {0xB2E6, Service31ResultRoutineDdrTest, E_UDS_SECURITY_LEVEL1}      // 获取模组DDR测试结果
};
#endif
static uint8_t g_udsRxData[1100] = {0};;
static uint8_t g_securitySeed[20] = {0};;
static uint8_t g_securityKey[20] = {0};;
static uint8_t g_udsTxBuffer[512] = {0};;

static uint16_t g_udsRxDataLen = 0;

/**********session control related variable*******************/
static SessionState_e g_currentSession = E_DEFAULT_SESSION;
static uint8_t g_ecuOnlineFlag = 0;

/**********security accessrelated variable*******************/
static UdsSecurityLevel_e g_currentSecurityLevel = E_UDS_NONE_SECURITY_LEVEL;
static UdsSecurityLevel_e g_securitySendSeedLevel = E_UDS_NONE_SECURITY_LEVEL;

static uint8_t g_ecuResetFlag[3];
static uint8_t g_u8ConsecutiveSeedCount[3] = {0, 0, 0};
static SecurityTimerDelayType_e g_securityTimerDelayType;
static SecurityTimerDelayType_e g_securityTimerDelayType_2;
static SecurityTimerDelayType_e g_securityTimerDelayType_3;

static SecurityTimerDelayType_e g_securityTimerDelayType_Reprog;
// static SecurityTimerDelayType_e g_securityTimerDelayType_L3;

static int16_t g_ecuSecurityTimerHandle = -1;
static int16_t g_ecuSecurityTimerHandle_2 = -1;
static int16_t g_ecuSecurityTimerHandle_3 = -1;

static int16_t g_ecuSecurityTimerHandle_Reprog = -1;
// static int16_t g_ecuSecurityTimerHandle_L3 = -1;

static uint8_t g_securityKeyLength = 0;
static uint8_t g_securitySeedLength = 0;

/*************************************************************/
static uint8_t g_bDtcControlFlag = 0;
static uint8_t g_bCommunicationControlFlag = 0;

static uint8_t g_sleepFlag = 0;
static uint8_t g_currentServiceNum = 0;
static int16_t g_tpHandle = -1;

static uint8_t g_virtualTpFlag = 0;
static uint8_t g_virtualTpChanalId = 0;
// static int16_t ProcessWriteDidESKey(uint8_t *udsData, uint16_t udsLen);
/*************************************************
  Function:       GetCurrentSession
  Description:    获取当前的诊断会话状态
  Input:          无
  Output:         无
  Return:         当前的会话状态
*************************************************/
SessionState_e GetCurrentSession(void) // 0xF186_cxl
{
  return g_currentSession;
}

UdsSecurityLevel_e GetCurrentSecurityLevel(void)
{
  return g_currentSecurityLevel;
}

static bool IsDidPassthrough_22(uint16_t did)
{
  for (int i = 0; i < (sizeof(g_passthroughDidList_22) / sizeof(g_passthroughDidList_22[0])); ++i)
  {
    if (g_passthroughDidList_22[i] == did)
    {
      return true;
    }
  }
  return false;
}
// 诊断仪是否连接
uint8_t GetTesterPresenceStatus(void)
{
  return g_isTesterPresent;
}
// static SeriveGetSubFunState_e Service2EGetDidFunction(uint16_t WDID, pService2E_DIDFunction *pfun, uint16_t u16Validlen)
// {
//   SeriveGetSubFunState_e ret;

//   uint16_t i = 0;
//   uint16_t listSize;

//   listSize = sizeof(g_Service2EFunMapList) / sizeof(g_Service2EFunMapList[0]);
//   ret = E_SUBFUN_GET_INVALID_DID; // invalid did
//   *pfun = NULL;

//   for (i = 0; i < listSize; i++)
//   {
//     if (g_Service2EFunMapList[i].u16DID == WDID)
//     {
//       if (g_currentSecurityLevel == g_Service2EFunMapList[i].u8SecurityLevel)
//       {
//         if (g_Service2EFunMapList[i].u16Len == (u16Validlen - 3))
//         {
//           if (g_Service2EFunMapList[i].pFun != NULL)
//           {
//             *pfun = g_Service2EFunMapList[i].pFun;
//             ret = E_SUBFUN_GET_OK;
//           }
//         }
//         else
//         {
//           ret = E_SUBFUN_GET_INVALID_LENGTH; // length error
//         }
//       }
//       else
//       {
//         return E_SUBFUN_GET_INVALID_SECURITY;
//       }
//       break;
//     }
//   }

//   return ret;
// }
static SeriveGetSubFunState_e Service2EGetDidFunction(uint16_t WDID, pService2E_DIDFunction *pfun, uint16_t u16Validlen)
{
  SeriveGetSubFunState_e ret;
  uint16_t i = 0;
  uint16_t listSize;
  UdsSecurityLevel_e requiredLevel;
  // uint8_t factoryMode;
  bool securityBypassed = false;

  listSize = sizeof(g_Service2EFunMapList) / sizeof(g_Service2EFunMapList[0]);
  ret = E_SUBFUN_GET_INVALID_DID;
  *pfun = NULL;

  for (i = 0; i < listSize; i++)
  {
    if (g_Service2EFunMapList[i].u16DID == WDID)
    {
      requiredLevel = g_Service2EFunMapList[i].u8SecurityLevel;
      #if 0
      factoryMode = UdsDidGetManufactoryMode();

      // 工厂模式下，安全级别1和2的DID可以直接访问
      if (factoryMode >= 0x10 && (requiredLevel == E_UDS_SECURITY_LEVEL1 || requiredLevel == E_UDS_SECURITY_LEVEL2_STD))
      {
        securityBypassed = true;
      }
      #endif
      if (securityBypassed || (g_currentSecurityLevel == requiredLevel))
      {
        if (g_Service2EFunMapList[i].u16Len == (u16Validlen - 3))
        {
          if (g_Service2EFunMapList[i].pFun != NULL)
          {
            *pfun = g_Service2EFunMapList[i].pFun;
            ret = E_SUBFUN_GET_OK;
          }
        }
        else
        {
          ret = E_SUBFUN_GET_INVALID_LENGTH;
        }
      }
      else
      {
        ret = E_SUBFUN_GET_INVALID_SECURITY;
      }
      break;
    }
  }

  return ret;
}

static SeriveGetSubFunState_e Service22GetDidFunction(uint16_t RDID, pService22_DIDFunction *pfun)
{
  uint16_t i;
  uint16_t listSize;
  SeriveGetSubFunState_e ret;

  *pfun = NULL;
  ret = E_SUBFUN_GET_INVALID_DID;
  listSize = sizeof(g_Service22FunMapList) / sizeof(g_Service22FunMapList[0]);

  for (i = 0; i < listSize; i++)
  {
    if (g_Service22FunMapList[i].u16DID == RDID)
    {
      if (g_Service22FunMapList[i].pFun != NULL)
      {
        *pfun = g_Service22FunMapList[i].pFun;
        ret = E_SUBFUN_GET_OK;
      }
      break;
    }
  }

  return ret;
}
// 31服务-01-请求例程开始
static int16_t ServiceGetRoutineStartDidFunction(uint16_t DID, uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  uint16_t i = 0;
  uint16_t listSize;
  int16_t ret = 0x31; // 默认返回 DID 不支持

  listSize = sizeof(g_routineStartDidMap) / sizeof(g_routineStartDidMap[0]);

  for (i = 0; i < listSize; i++)
  {
    if (g_routineStartDidMap[i].u16DID == DID)
    {     
      if (g_currentSecurityLevel != g_routineStartDidMap[i].u8SecurityLevel &&
          g_routineStartDidMap[i].u8SecurityLevel != E_UDS_NONE_SECURITY_LEVEL)
      {
        return 0x33; // 安全访问被拒绝
      }

      // 检查会话是否有效
      if ((g_currentSession == E_DEFAULT_SESSION && !(g_routineStartDidMap[i].sessionSupport & SESSION_BIT_DEFAULT)) ||
          (g_currentSession == E_EXTEND_SESSION && !(g_routineStartDidMap[i].sessionSupport & SESSION_BIT_EXTENDED)) ||
          (g_currentSession == E_PROGROM_SESSION && !(g_routineStartDidMap[i].sessionSupport & SESSION_BIT_PROGRAMMING)))
      {
        return 0x7F; // 会话不支持
      }
      
      if (g_routineStartDidMap[i].pFun != NULL)
      {
        *pLengthOut = 0;
        ret = g_routineStartDidMap[i].pFun(pDataIn, lengthIn, pDataOut, pLengthOut);
      }
      break;
    }
  }

  return ret;
}


// 31服务-02-请求例程停止
static  SeriveGetSubFunState_e ServiceGetRoutineStopDidFunction(uint16_t DID,pService31_DIDFunction *pfun)
{
    SeriveGetSubFunState_e ret;

    uint16_t i = 0;
    uint16_t listSize;

    listSize = sizeof(g_routineStopDidMap) / sizeof(g_routineStopDidMap[0]);
    ret = E_SUBFUN_GET_INVALID_DID;//invalid did
    *pfun = NULL;

    for(i = 0; i < listSize; i++)
    {
        if(g_routineStartDidMap[i].u16DID == DID)
        {
            if(g_routineStartDidMap[i].pFun!=NULL)
            {
              *pfun = g_routineStartDidMap[i].pFun;
              ret = E_SUBFUN_GET_OK;
            }
            break;
        }
    }

    return  ret;
}

// 31服务-03-请求例程结果
static SeriveGetSubFunState_e ServiceGetRoutineResultDidFunction(uint16_t DID, pService31_DIDFunction *pfun)
{
  SeriveGetSubFunState_e ret;

  uint16_t i = 0;
  uint16_t listSize;

  listSize = sizeof(g_routineResultDidMap) / sizeof(g_routineResultDidMap[0]);
  ret = E_SUBFUN_GET_INVALID_DID; // invalid did
  *pfun = NULL;

  for (i = 0; i < listSize; i++)
  {
    if (g_routineResultDidMap[i].u16DID == DID)
    {
        if (g_routineResultDidMap[i].pFun != NULL)
        {
          *pfun = g_routineResultDidMap[i].pFun;
          ret = E_SUBFUN_GET_OK;
        }
      else
      {
        return E_SUBFUN_GET_INVALID_SECURITY;
      }
      break;
    }
  }

  return ret;
}

/*************************************
return:
0: Vaild
**************************************/
static int16_t CheckSessionIsValid(SessionState_e currentSession, uint8_t supportedSession)
{
  int16_t ret;
  uint8_t sessionBit;
  ret = 1;
  sessionBit = 0;
  if (E_DEFAULT_SESSION == currentSession)
  {
    sessionBit = SESSION_BIT_DEFAULT;
  }
  else if (E_PROGROM_SESSION == currentSession)
  {
    sessionBit = SESSION_BIT_PROGRAMMING;
  }
  else if (E_EXTEND_SESSION == currentSession)
  {
    sessionBit = SESSION_BIT_EXTENDED;
  }

  if (sessionBit & supportedSession)
  {
    ret = 0;
  }
  return ret;
}

// int16_t Service22ReadSeesionStatus(uint8_t* buf, uint16_t *len)
//{
//     uint8_t seeion = 0;
//     uint16_t ret = 0;
//     seeion = g_currentSession;
//     *buf  = seeion;
//     *len = 1;
//
//     return ret;
// }

static void SaveSeedAccessCountToNonVolatile(uint8_t *data)
{
  uint8_t *dataTem;
  dataTem = data;
  // WorkFlashUserInfoStore(E_SECURITY_INFO,dataTem,3);
  WorkFlashVehicleInforStore(E_PARAMETER_INFO_UDS_SECURITY_ERROR_COUNT, dataTem, 3);
}
#if 0
static uint8_t GetSeedAccessCountFromVolatile(uint8_t *data)
{
  uint32_t lenth = 0;
  // WorkFlashUserInfoRead(E_SECURITY_INFO,data,&lenth);
  WorkFlashVehicleInforRead(E_PARAMETER_INFO_UDS_SECURITY_ERROR_COUNT, data, &lenth);
  return 0;
}
#endif
static void SessionChangeToDefaultProcess(void)
{
  if (g_bDtcControlFlag)
  {
    g_bDtcControlFlag = 0;
    // reset DTC control setting
    DtcProcessEnable();
  }
  if (g_bCommunicationControlFlag)
  {
    g_bCommunicationControlFlag = 0;
    /*SgmwGWNm_AllEnableCommunication();
    CycleSendEnableAll();*/
    CanPeriodSendEnableAll();
#if (NM_PROTOCOL_TYPE == NM_PROTOCOL_OSEK)
    Nm_AllEnableCommunication();
#else
    AutosarNmSdkEnableCommunication(0);
#endif
}
  Service31StopApplicationSetDefault();
  Service31WriteDiagnosticCanReportSetDefault();
}

static void SessionChangeToExtendedProcess(void)
{
}

static int16_t DiagnosticDataTransmit(int16_t tpHandle, uint32_t canId, uint8_t *txData, uint16_t txLength, uint8_t functionAddFlag)
{
  if (g_virtualTpFlag == 0)
  {

    CanTpSdkDataTransmit(tpHandle, canId, txData, txLength, functionAddFlag);
  }
  else
  {
    VirtualTpSdkServerTransmit(g_virtualTpChanalId, (const uint8_t *)txData, txLength);
  }
  return 0;
}
/*********************************************************************
session control

********************************************************************/
static int16_t Service0x10Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint8_t responseData[32];
  uint8_t subFun;
  uint8_t suppressPosRsp;
  responseData[0] = 0x50;
  responseData[1] = udsData[1];
  responseData[2] = 0x00; // P2 Server
  responseData[3] = 0x32; // P2 Server
  responseData[4] = 0x01; // P2*Server
  responseData[5] = 0xF4; // P2*Server

  subFun = udsData[1] & 0x7F;
  suppressPosRsp = udsData[1] >> 7;
  if (0X02 == udsLen)
  {
    if (0x01 == subFun) // default session
    {
      g_currentSession = E_DEFAULT_SESSION;
      g_currentSecurityLevel = E_UDS_NONE_SECURITY_LEVEL; // lock security
      SessionChangeToDefaultProcess();
    }
    else if (0x02 == subFun) // programing session
    {
      if ((g_currentSession == E_EXTEND_SESSION) || (g_currentSession == E_PROGROM_SESSION))
      {
        g_currentSession = E_PROGROM_SESSION;
        g_currentSecurityLevel = E_UDS_NONE_SECURITY_LEVEL; // lock security
        g_ecuOnlineFlag = 1;
        CanTpSdkSetBootMode(g_tpHandle, 1);
      }
      else
      {
        negativeNum = 0x7E;
      }
    }
    else if (0x03 == subFun) // extended session
    {
      if ((g_currentSession == E_DEFAULT_SESSION) || (g_currentSession == E_EXTEND_SESSION))
      {
        g_currentSession = E_EXTEND_SESSION;
        // g_currentSecurityLevel = E_UDS_NONE_SECURITY_LEVEL; // lock security
        g_ecuOnlineFlag = 1;
        uint8_t factoryMode = UdsDidGetManufactoryMode();
        if (factoryMode < 0x10)
        {
          g_currentSecurityLevel = E_UDS_NONE_SECURITY_LEVEL; // lock security
        }
        SessionChangeToExtendedProcess();
      }
      else
      {
        negativeNum = 0x7E;
      }
    }
    else
    {
      negativeNum = 0x12; // sub function not supported
    }
    if (negativeNum == 0)
    {
      if (suppressPosRsp == 0)
      {
        DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 6, 0);
      }
      /*if(g_currentSession==E_PROGROM_SESSION)
      {
        PeripheralDriverHal_WriteBootFlag(0x00002211);
        PeripheralHalMcuHardReset();
      }      */
    }
  }
  else
  {
    negativeNum = 0x13;
  }

  /*if(functionAddressFlag)
  {
    if(0x12==negativeNum)//not
    }
    {
      negativeNum = 0;
  }*/
  return negativeNum;
}

static int16_t Service0x11Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  // uint8_t i = 0;
  uint8_t negativeNum = 0;
  uint8_t subFun;
  uint8_t suppressPosRsp;
  const Diagnostic_t *psDignostic = NULL;
  uint8_t responseData[32];

  responseData[0] = 0x51;
  responseData[1] = udsData[1];

  subFun = udsData[1] & 0x7F;
  suppressPosRsp = udsData[1] >> 7;

  psDignostic = (g_serviceECUFunGroup + g_currentServiceNum);

  if (CheckSessionIsValid(g_currentSession, psDignostic->sessionSupport) == 0)
  {
    if (0x02 == udsLen)
    {
      if ((0x01 == subFun) || (0x03 == subFun))
      {
        
        if (suppressPosRsp == 0)
        {
          DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, 0);
        }

        vTaskDelay(100);

        if (0x01 == subFun) // hardware reset（复位MCU and MPU）
        {
          MpuHalPowerOff();
          vTaskDelay(100);
          PeripheralHalMcuHardReset();
        }
        else // 0x03 software reset(仅复位MCU)
        {
          PeripheralHalMcuHardReset();
        }
      }
      else
      {
        negativeNum = 0x12;
      }
    }
    else
    {
      negativeNum = 0x13;
    }
  }
  else
  {
    negativeNum = 0x7F;
  }

  return negativeNum;
}

static int16_t Service0x14Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint32_t dtcValue;
  const Diagnostic_t *psDignostic = NULL;
  uint8_t responseData[32];
  uint8_t ret = 0;

  psDignostic = (g_serviceECUFunGroup + g_currentServiceNum);

  responseData[0] = 0x54;
  dtcValue = ((uint32_t)udsData[1] << 16) | ((uint32_t)udsData[2] << 8) | (uint32_t)udsData[3];
  if (CheckSessionIsValid(g_currentSession, psDignostic->sessionSupport) == 0)
  {
    if (0x04 == udsLen)
    {
      if (0x100000 == dtcValue) // Powertrain group
      {
        DtcProcessClearAll();
      }
      else if (0x400000 == dtcValue) // Chassis group
      {
        DtcProcessClearAll();
      }
      else if (0x800000 == dtcValue) // Body group
      {
        DtcProcessClearAll();
      }
      else if (0xC00000 == dtcValue) // Network communication group
      {
        DtcProcessClearAll();
      }
      else if (0xFFFFFF == dtcValue) // all dtc
      {
        DtcProcessClearAll();
      }
      else
      {
        if (dtcValue != 0x000000) { 
          ret = DtcProcessClearBySpecificDTC(dtcValue); //清除指定单个DTC
          if (ret != 0)
            negativeNum = 0x31;
        }
        else {
          negativeNum = 0x31;
        }       
      }
      if (negativeNum == 0)
      {
        DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 1, 0);
      }
    }
    else
    {
      negativeNum = 0x13;
    }
  }
  else
  {
    negativeNum = 0x7F;
  }

  return negativeNum;
}
static uint8_t Service0x19Process0x0A(uint8_t *udsData, uint16_t udsLen)
{
  uint8_t negativeNum = 0;
  uint32_t dtcNum = 0;
  if (udsLen == 2)
  {
    DtcProcessGetSupportedDtc(g_udsTxBuffer + 3, &dtcNum, &g_udsTxBuffer[2]);

    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, g_udsTxBuffer, 3 + (dtcNum * 4), 0);
  }
  else
  {
    negativeNum = 0x13; // incorrect length
  }
  return negativeNum;
}
static uint8_t Service0x19Process0x06(uint8_t *udsData, uint16_t udsLen)
{
  uint32_t length = 0;
  int16_t ret;
  uint8_t negativeNum = 0;
  if (udsLen == 6)
  {
    // g_udsTxBuffer[5] = udsData[5];    
    ret = DtcProcessGetExtendedData((udsData[2] << 16)|(udsData[3] << 8) | (udsData[4] << 0), udsData[5], g_udsTxBuffer + 6, &length, &g_udsTxBuffer[5]);
    
    if (ret == 0)
    {
      // dtc
      g_udsTxBuffer[2] = udsData[2];
      g_udsTxBuffer[3] = udsData[3];
      g_udsTxBuffer[4] = udsData[4];

      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, g_udsTxBuffer, length + 6, 0);
    }
    else
    {
      negativeNum = 0x31; // request out of range
    }
  }
  else
  {
    negativeNum = 0x13; // incorrect length
  }
  return negativeNum;
}
static uint8_t Service0x19Process0x04(uint8_t *udsData, uint16_t udsLen)
{
  uint32_t length = 0;
  int16_t ret;
  uint8_t negativeNum = 0;
  if (udsLen == 6)
  {
    if ((udsData[5] == 0x01) || (udsData[5] == 0x05) || (udsData[5] == 0x02) || (udsData[5] == 0xFF)) // dtc snapshot record number
    {
      // 获取快照数据，读取出现故障后的快照数据，
      //(udsData[2]<<16)+(udsData[3]<<8)+(udsData[4]<<0)：故障编码
      // udsData[5]：快照记录号
      // m_udsTxBuffer+7：从第七位开始存放快照数据
      // length：快照数据长度
      ret = DtcProcessGetReportSnapshotRecordByDtcNumber((udsData[2] << 16) + (udsData[3] << 8) + (udsData[4] << 0), udsData[5], g_udsTxBuffer + 6, &length, &g_udsTxBuffer[5]);
      if (ret == 0)
      {
        // dtc
        g_udsTxBuffer[2] = udsData[2];
        g_udsTxBuffer[3] = udsData[3];
        g_udsTxBuffer[4] = udsData[4];
        // DTCSnapshotRecordNumber
        // g_udsTxBuffer[6] = 0x01;
        //
        // g_udsTxBuffer[7] = 0x05;//dtc snapshot record number of identifiers

        DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, g_udsTxBuffer, length + 6, 0);
      }
      else
      {
        negativeNum = 0x31; // request out of range
      }
    }
    else
    {
      negativeNum = 0x31; // request out of range
    }
  }
  else
  {
    negativeNum = 0x13; // incorrect length
  }
  return negativeNum;
}
static uint8_t Service0x19Process0x02(uint8_t *udsData, uint16_t udsLen)
{
  uint8_t negativeNum = 0;
  uint32_t dtcNum = 0;
  if (udsLen == 0x03)
  {
    dtcNum = 0;
    DtcProcessGetListByMask(udsData[2], &g_udsTxBuffer[3], &dtcNum, &(g_udsTxBuffer[2]));
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, g_udsTxBuffer, 3 + (dtcNum * 4), 0);
  }
  else
  {
    negativeNum = 0x13; // invlid length
  }
  return negativeNum;
}
static uint8_t Service0x19Process0x01(uint8_t *udsData, uint16_t udsLen)
{
  uint8_t negativeNum = 0;
  uint32_t dtcNum = 0;
  uint8_t MaskValue = 0;

  if (udsLen == 0x03)
  {
    g_udsTxBuffer[0] = 0x59; 
    g_udsTxBuffer[1] = 0x01;

    DtcProcessDtcGetCountByMask(udsData[2], &dtcNum, &MaskValue);
    g_udsTxBuffer[2] = MaskValue;
    g_udsTxBuffer[3] = 0x01; 
    g_udsTxBuffer[4] = (uint8_t)((dtcNum >> 8) & 0xFF);
    g_udsTxBuffer[5] = (uint8_t)(dtcNum & 0xFF);

    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, g_udsTxBuffer, 6, 0);
  }
  else
  {
    negativeNum = 0x13;
  }

  return negativeNum;
}

static int16_t Service0x19Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint8_t subFun;
  const Diagnostic_t *psDignostic = NULL;
  // dtc_snapshot_record_global_t snapRecord;
  // dtc_extended_data_t extendedData;
  // uint32_t dtcNum = 0;
  // uint8_t MaskValue = 0;
  // uint32_t length = 0;
  // int16_t ret;
  psDignostic = (g_serviceECUFunGroup + g_currentServiceNum);
  g_udsTxBuffer[0] = 0x59;
  g_udsTxBuffer[1] = udsData[1];

  subFun = udsData[1];
  if (CheckSessionIsValid(g_currentSession, psDignostic->sessionSupport) == 0)
  {
    if (0x02 <= udsLen)
    {
      switch (subFun)
      {
      case 0x01:
      {
        // if(udsLen==0x03)
        //{
        //   if (udsData [2] & 0x09)
        //   {
        //       //read dtc num
        //       DtcProcessDtcGetCountByMask(udsData [2], &dtcNum, &MaskValue);
        //       g_udsTxBuffer[2] = MaskValue;
        //       g_udsTxBuffer[3] = 0x00;
        //       g_udsTxBuffer[4] = (uint8_t)((dtcNum >> 8) & 0XFF);
        //       g_udsTxBuffer[5] = (uint8_t)(dtcNum & 0XFF);;
        //   }
        //   else
        //   {
        //       g_udsTxBuffer[2] = 0x00;
        //       g_udsTxBuffer[3] = 0x00;
        //       g_udsTxBuffer[4] = 0;
        //       g_udsTxBuffer[5] = 0;
        //   }
        //   DiagnosticDataTransmit(g_tpHandle,g_physicalTransmitCanId,g_udsTxBuffer,6,0);
        // }
        // else
        //{
        //   negativeNum = 0x13;//invlid length
        // }
        negativeNum = Service0x19Process0x01(udsData, udsLen);
      }
      break;
      case 0x02:
      {
        // if(udsLen==0x03)
        //{
        //   dtcNum = 0;
        //   DtcProcessGetListByMask(udsData[2],&g_udsTxBuffer[3],&dtcNum,&(g_udsTxBuffer[2]));
        //   DiagnosticDataTransmit(g_tpHandle,g_physicalTransmitCanId,g_udsTxBuffer,3+(dtcNum*4),0);
        // }
        // else
        //{
        //   negativeNum = 0x13;//invlid length
        // }
        negativeNum = Service0x19Process0x02(udsData, udsLen);
      }
      break;
      case 0x04: // reportDTCSnapshotRecordByDTCNumber
      {
        // if(udsLen==6)
        //{
        //   if((udsData[5]==0x01) ||(udsData[5]==0x05) || (udsData[5]==0xFF))//dtc snapshot record number
        //   {
        //     //获取快照数据，读取出现故障后的快照数据，
        //     //(udsData[2]<<16)+(udsData[3]<<8)+(udsData[4]<<0)：故障编码
        //     //udsData[5]：快照记录号
        //     //m_udsTxBuffer+7：从第七位开始存放快照数据
        //     //length：快照数据长度
        //     ret = DtcProcessGetReportSnapshotRecordByDtcNumber((udsData[2]<<16)+(udsData[3]<<8)+(udsData[4]<<0),udsData[5],g_udsTxBuffer+6,&length,&g_udsTxBuffer[5]);
        //     if(ret==0)
        //     {
        //       //dtc
        //       g_udsTxBuffer[2] = udsData[2];
        //       g_udsTxBuffer[3] = udsData[3];
        //       g_udsTxBuffer[4] = udsData[4];
        //       //DTCSnapshotRecordNumber
        //       //g_udsTxBuffer[6] = 0x01;
        //       //
        //       //g_udsTxBuffer[7] = 0x05;//dtc snapshot record number of identifiers
        //
        //       DiagnosticDataTransmit(g_tpHandle,g_physicalTransmitCanId,g_udsTxBuffer,length+6,0);
        //     }
        //     else
        //     {
        //       negativeNum = 0x31;//request out of range
        //     }
        //   }
        //   else
        //   {
        //     negativeNum = 0x31;//request out of range
        //   }
        //
        // }
        // else
        //{
        //   negativeNum = 0x13;//incorrect length
        // }
        negativeNum = Service0x19Process0x04(udsData, udsLen);
      }
      break;
      case 0x06: // report dtc extended data record by dtc number 19 06 xx xx xx FF    (XX XX XX为DTC编号)
      {
        // if(udsLen==6)
        //{
        //     ret = DtcProcessGetExtendedData((udsData[2]<<16)+(udsData[3]<<8)+(udsData[4]<<0),udsData[5],g_udsTxBuffer+6,&length,&g_udsTxBuffer[5]);
        //     if(ret==0)
        //     {
        //       //dtc
        //       g_udsTxBuffer[2] = udsData[2];
        //       g_udsTxBuffer[3] = udsData[3];
        //       g_udsTxBuffer[4] = udsData[4];
        //
        //       DiagnosticDataTransmit(g_tpHandle,g_physicalTransmitCanId,g_udsTxBuffer,length+6,0);
        //     }
        //     else
        //     {
        //       negativeNum = 0x31;//request out of range
        //     }
        // }
        // else
        //{
        //   negativeNum = 0x13;//incorrect length
        // }
        negativeNum = Service0x19Process0x06(udsData, udsLen);
      }
      break;
      case 0x0A: // 读取支持的DTC
      {
        // if(udsLen==2)
        //{
        //	ret = DtcProcessGetSupportedDtc(g_udsTxBuffer+3,&dtcNum,&g_udsTxBuffer[2]);
        //	DiagnosticDataTransmit(g_tpHandle,g_physicalTransmitCanId,g_udsTxBuffer,3+(dtcNum*4),0);
        // }
        // else
        //{
        //	negativeNum = 0x13;//incorrect length
        // }
        negativeNum = Service0x19Process0x0A(udsData, udsLen);
      }
      break;
      default:
        if (functionAddressFlag)
        {
          return negativeNum;
        }
        negativeNum = 0x12;
        break;
      }
    }
    else
    {
      negativeNum = 0x13;
    }
  }
  else
  {
    if (functionAddressFlag)
    {
      return negativeNum;
    }
    negativeNum = 0x7F;
  }

  return negativeNum;
}

/*启用和关闭TX和RX*/
static int16_t Service0x28Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  const Diagnostic_t *psDignostic = NULL;
  uint8_t responseData[32];

  psDignostic = (g_serviceECUFunGroup + g_currentServiceNum);

  responseData[0] = 0x68;
  responseData[1] = udsData[1];
  if (udsLen != 3)
  {
    return 0x13;
  }
  if (CheckSessionIsValid(g_currentSession, psDignostic->sessionSupport) == 0)
  {
    if (0x03 == udsLen)
    {
      switch (udsData[UDS_OFFSET_SUB_FUNC])
      {
      case 0x83:
      case 0x00:
      case 0x80:
      case 0x03:
      {
        #if 1  //SQ 28服务仅使用00/03子功能，且子功能下通信控制类型仅支持03
        if (udsData[UDS_OFFSET_SUB_FUNC + 1] == 0x03)
        #else
        if ((udsData[UDS_OFFSET_SUB_FUNC + 1] == 0x01) || (udsData[UDS_OFFSET_SUB_FUNC + 1] == 0x02) || (udsData[UDS_OFFSET_SUB_FUNC + 1] == 0x03))
        #endif
        {
          negativeNum = CommunicationControl((udsData[UDS_OFFSET_SUB_FUNC] & 0x7F), udsData[UDS_OFFSET_SUB_FUNC + 1]);

          if (negativeNum != 0)
          {
            return negativeNum;
          }
          if ((udsData[UDS_OFFSET_SUB_FUNC] >> 7) == 0)
          {
            DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, 0);
          }
        }
        else
        {
          if (functionAddressFlag)
          {
            return negativeNum;
          }
          negativeNum = 0x31;
        }
      }
      break;
      default:
        if (functionAddressFlag)
        {
          return negativeNum;
        }
        negativeNum = 0x12;
        break;
      }
    }
    else
    {
      negativeNum = 0x13;
    }
  }
  else
  {
    /*if(functionAddressFlag)
    {
            return negativeNum;
    }*/
    negativeNum = 0x7F;
  }
  return negativeNum;
}

/*******************************************************
ControlDTCSetting (85 hex) service




***************************************************/
static int16_t Service0x85Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint8_t responseData[32];

  const Diagnostic_t *psDignostic = NULL;

  psDignostic = (g_serviceECUFunGroup + g_currentServiceNum);

  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  if (CheckSessionIsValid(g_currentSession, psDignostic->sessionSupport) == 0)
  {
    if (0x02 == udsLen)
    {
      switch (udsData[UDS_OFFSET_SUB_FUNC])
      {
      case 0x81:
      case 0x01:
      case 0x82:
      case 0x02:
      {
        if ((0x01 == udsData[UDS_OFFSET_SUB_FUNC]) || (0x81 == udsData[UDS_OFFSET_SUB_FUNC]))
        {
          g_bDtcControlFlag = 0;
          DtcProcessEnable();
        }
        else
        {
          g_bDtcControlFlag = 1;
          DtcProcessDisable();
        }
        if (0 == (udsData[UDS_OFFSET_SUB_FUNC] >> 7))
        {
          DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, 0);
        }
      }
      break;
      default:
      {
        if (functionAddressFlag)
        {
          return negativeNum;
        }
        negativeNum = 0x12;
      }
      break;
      }
    }
    else
    {
      negativeNum = 0x13;
    }
  }
  else
  {
    /*if(functionAddressFlag)
    {
            return negativeNum;
    }*/
    negativeNum = 0x7F;
  }
  return negativeNum;
}

static int16_t Service0x31Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum;
  uint8_t subFunction;
  uint8_t responseData[32];
  uint16_t did;
  pService31_DIDFunction pFun;
  SeriveGetSubFunState_e ret;
  uint16_t lengOut;
  const Diagnostic_t *psDignostic = NULL;

  psDignostic = (g_serviceECUFunGroup + g_currentServiceNum);
  negativeNum = 0;
  subFunction = udsData[1];

  did = ((uint16_t)udsData[2] << 8) + (uint16_t)udsData[3];
  responseData[0] = 0x71;
  responseData[1] = udsData[1];
  responseData[2] = udsData[2];
  responseData[3] = udsData[3];
  // TBOX_PRINT("Service0x31Process did=%04X subFunction=%02X\r\n", did, subFunction);
  if (udsLen < 4)
  {
    return 0x13; // 消息长度错误
  }
  // uint8_t factoryMode = UdsDidGetManufactoryMode();
  
  // if ( (factoryMode < 0x10) && (g_currentSecurityLevel != E_UDS_SECURITY_LEVEL1) )
  // {
  //   negativeNum = 0x33;
  //   return negativeNum;
  // }

  if (CheckSessionIsValid(g_currentSession, psDignostic->sessionSupport) != 0)
  {
    negativeNum = 0x7F;
    return negativeNum;
  }
  if (0x01 == subFunction)
  {
    lengOut = 0;
    negativeNum = ServiceGetRoutineStartDidFunction(did, &udsData[4], udsLen - 4, &responseData[4], &lengOut);
    if (negativeNum == 0)
    {
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, lengOut + 4, 0);
    }
  }
  else if (0x02 == subFunction)
  {
    ret = ServiceGetRoutineStopDidFunction(did, &pFun);
    if (ret == E_SUBFUN_GET_OK)
    {
      lengOut = 0;
      negativeNum = pFun(&udsData[4], udsLen - 4, &responseData[4], &lengOut);
      if (negativeNum == 0)
      {
        DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, lengOut + 4, 0);
      }
    }
    else if (ret == E_SUBFUN_GET_INVALID_LENGTH)
    {
      negativeNum = 0x13;
    }    
    else
    {
      negativeNum = 0x31; // did not supported
    }
  }
  else if (0x03 == subFunction)
  {
    ret = ServiceGetRoutineResultDidFunction(did, &pFun);
    if (ret == E_SUBFUN_GET_OK)
    {
      lengOut = 0;
      negativeNum = pFun(&udsData[4], udsLen - 4, &responseData[4], &lengOut);
      if (negativeNum == 0)
      {
        DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, lengOut + 4, 0);
      }
    }
    else if (ret == E_SUBFUN_GET_INVALID_LENGTH)
    {
      negativeNum = 0x13;
    }    
    else
    {
      negativeNum = 0x31; // did not supported
    }
  }
  else
  {
    negativeNum = 0x12; // sub function not supported
  }
  return negativeNum;
}

static int16_t Service0x3EProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint8_t responseData[32];

  // const Diagnostic_t *psDignostic = NULL;
  // psDignostic = ( g_serviceECUFunGroup + g_currentServiceNum );

  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  // if (CheckSessionIsValid(g_currentSession,psDignostic->sessionSupport)==0)
  {
    if (0x02 == udsLen)
    {
      switch (udsData[UDS_OFFSET_SUB_FUNC])
      {
      case 0x00:
      case 0x80:
      {
        g_ecuOnlineFlag = 1;

        if ((udsData[UDS_OFFSET_SUB_FUNC] >> 7) == 0)
        {
          DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, 0);
        }
      }
      break;
      default:
        if (functionAddressFlag)
        {
          return negativeNum;
        }
        negativeNum = 0x12;
        break;
      }
    }
    else
    {
      negativeNum = 0x13;
    }
  }
  //    else
  //    {
  //        /*if(functionAddressFlag)
  //        {
  //                return negativeNum;
  //        }*/
  //        negativeNum = 0x7F;
  //    }
  return negativeNum;
}

/************************************************
InputOutputControlByIdentifier (2F hex) service



***********************************************/
// static int16_t Service0x2FProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
// {
//   uint8_t negativeNum = 0;
//   // uint16_t did;
//   // uint8_t controlParameter;
//   // uint8_t responseData[32];
//   const Diagnostic_t *psDignostic = NULL;

//   /*psDignostic = ( g_serviceECUFunGroup + g_currentServiceNum );

//   did = ((uint16_t)udsData[1]<<8)+(uint16_t)udsData[2];
//   controlParameter = udsData[3];

//   responseData[0] = 0x6F;
//   responseData[1] = udsData[1];
//   responseData[2] = udsData[2];
//   responseData[3] = udsData[3];*/

//   if (CheckSessionIsValid(g_currentSession, psDignostic->sessionSupport) != 0)
//   {
//     negativeNum = 0x7E;
//     return negativeNum;
//   }

//   return negativeNum;
// }
#if 0
static int16_t Service0x2FProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
    //uint8_t nrc = 0;
    uint8_t responseData[32];
    uint16_t did;
    uint8_t controlParameter;

    responseData[0] = 0x6F;         // Positive Response SID
    responseData[1] = udsData[1];   // DID MSB
    responseData[2] = udsData[2];   // DID LSB

    if (udsLen < 4)
        return 0x13; // Incorrect message length

    did = ((uint16_t)udsData[1] << 8) | udsData[2];
    controlParameter = udsData[3];
    responseData[3] = controlParameter;

    switch (did)
    {
        case 0xB2A0: // Call control
        {
            if (controlParameter == 0x03)
            {
                if (udsLen != 5) return 0x13;
                uint8_t callType = udsData[4];

                if (callType == 0x00)
                {
                    // EcallHalTriggerBcall_placeholder();
                }
                else if (callType == 0x01)
                {
                    // EcallHalTriggerEcall_placeholder();
                }
                else return 0x31;

                responseData[4] = callType;
                DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 5, 0);
            }
            else if (controlParameter == 0x00)
            {
                if (udsLen != 4)
                {
                    return 0x13;
                }
                DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 4, 0);
            }
            else return 0x31;

            break;
        }
      #if 0
        case 0xB2A1: // EMMC detection
        {
            if (controlParameter == 0x03)
            {
                if (udsLen != 5) return 0x13;
                uint8_t emmcState = 0;
                uint16_t len = 0;
                if (Service22ReadEmmcState(&emmcState, &len) == 0)
                {
                    responseData[4] = emmcState;
                    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 5, 0);
                }
                else return 0x22;
            }
            else if (controlParameter == 0x00)
            {
                if (udsLen != 4)
                {
                    return 0x13;
                }
                DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 4, 0);
            }
            else return 0x31;
            break;
        }
        #endif
        default:
            return 0x31; // RequestOutOfRange
    }

    return 0x00; // success
}
#endif

/**********************************************************
description: compare two byte arrays is the same
parameter:
  pArray1: first array
  pArray2: second array
  arrayLength:array length
return:
  0:arrays are the same
**********************************************************/
static int16_t ByteArrayCheckIsSame(uint8_t *pArray1, uint8_t *pArray2, uint16_t arrayLength)
{
  int16_t ret;
  uint16_t i;

  ret = 0;
  for (i = 0; i < arrayLength; i++)
  {
    if (pArray1[i] != pArray2[i])
    {
      ret = 1;
      break;
    }
  }
  return ret;
}
/*********************************************************
Security Access /send key




********************************************************/
#if 1 //SQ
// --- Level1 (0x01/0x02) ---
static uint8_t Service0x27Process0x01(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t responseData[32];
  uint8_t negativeNum = 0;
  const int level_index = 0;

  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  if (udsLen != 2)
    return 0x13;

  // uint8_t factoryMode = UdsDidGetManufactoryMode();
  // if (factoryMode >= 0x10)
  // {
  //   // 0x10-0xFF 返回全0的种子
  //   memset(g_securitySeed, 0, sizeof(g_securitySeed));
  //   g_securitySeedLength = 4;

  //   memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
  //   DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, functionAddressFlag);

  //   g_currentSecurityLevel = E_UDS_SECURITY_LEVEL1; // 自动解锁
  //   g_securitySendSeedLevel = E_UDS_NONE_SECURITY_LEVEL;

  //   return 0;
  // }

  if (g_ecuResetFlag[level_index])
  {
    negativeNum = 0x37;
  }
  else if (g_currentSecurityLevel == E_UDS_SECURITY_LEVEL1)
  {
    UdsGetSeed(g_securitySeed, &g_securitySeedLength);
    memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, 0);
  }
  else
  {
    if (g_u8ConsecutiveSeedCount[level_index] < 3)
    {
      UdsGetSeed(g_securitySeed, &g_securitySeedLength);
      unsigned int key_len_uint = 0;

      int result = Seed2Key(g_securitySeed, g_securitySeedLength, 0x01, NULL, g_securityKey, sizeof(g_securityKey), &key_len_uint);

      if (result == 1)
      {
        g_securityKeyLength = (uint8_t)key_len_uint;
        memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
        DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, functionAddressFlag);
        g_securitySendSeedLevel = E_UDS_SECURITY_LEVEL1;
      }
      else
      {
        negativeNum = 0x24;
      }
    }
    else
    {
      if (g_securityTimerDelayType != E_SECURITYTIMER_ACCESSERRORDELAY)
      {
        negativeNum = 0x36;
        TimerHalStartTime(g_ecuSecurityTimerHandle, 180000);
        g_securityTimerDelayType = E_SECURITYTIMER_ACCESSERRORDELAY;
      }
      else
      {
        negativeNum = 0x37;
      }
    }
  }
  return negativeNum;
}

static uint8_t Service0x27Process0x02(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint8_t responseData[32];
  const int level_index = 0;

  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  if (udsLen != 18)
    return 0x13;

  if (E_UDS_SECURITY_LEVEL1 == g_securitySendSeedLevel)
  {
    if (ByteArrayCheckIsSame(&udsData[2], g_securityKey, g_securityKeyLength) == 0)
    {
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, 0);
      g_currentSecurityLevel = E_UDS_SECURITY_LEVEL1;
      g_u8ConsecutiveSeedCount[level_index] = 0;
      SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
    }
    else
    {
      if (g_u8ConsecutiveSeedCount[level_index] < 3)
      {
        g_u8ConsecutiveSeedCount[level_index] += 1;
        SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
      }
      if (g_u8ConsecutiveSeedCount[level_index] < 3)
      {
        negativeNum = 0x35;
      }
      else
      {
        negativeNum = 0x36;
        TimerHalStartTime(g_ecuSecurityTimerHandle, 180000);
        g_securityTimerDelayType = E_SECURITYTIMER_ACCESSERRORDELAY;
      }
    }
    g_securitySendSeedLevel = E_UDS_NONE_SECURITY_LEVEL;
  }
  else
  {
    negativeNum = 0x24;
  }
  return negativeNum;
}

// --- Level3 (0x09/0x0A) ---
static uint8_t Service0x27Process0x09(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t responseData[32];
  uint8_t negativeNum = 0;
  const int level_index = 3;
if (g_currentSession != E_PROGROM_SESSION)
    {
        return 0x7F;
    }
  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  if (udsLen != 2)
    return 0x13;

  if (g_ecuResetFlag[level_index])
  {
    negativeNum = 0x37;
  }
  else if (g_currentSecurityLevel == E_UDS_SECURITY_LEVEL_REPROG)
  {
    UdsGetSeed(g_securitySeed, &g_securitySeedLength);
    memset((responseData + 2), 0, g_securitySeedLength);
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, 0);
  }
  else
  {
    if (g_u8ConsecutiveSeedCount[level_index] < 3)
    {

      UdsGetSeed(g_securitySeed, &g_securitySeedLength);

      unsigned int key_len_uint = 0;

      int result = Seed2Key(g_securitySeed, g_securitySeedLength, 0x09, NULL, g_securityKey, sizeof(g_securityKey), &key_len_uint);
      TBOX_PRINT("g_securitySeed: %16X, g_securityKey: %16X\n", g_securitySeed, g_securityKey);
      if (result == 1)
      {
        g_securityKeyLength = (uint8_t)key_len_uint;
        memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
        DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, functionAddressFlag);
        g_securitySendSeedLevel = E_UDS_SECURITY_LEVEL_REPROG;
      }
      else
      {
        negativeNum = 0x24;
      }
    }
    else
    {
      if (g_securityTimerDelayType_Reprog != E_SECURITYTIMER_ACCESSERRORDELAY)
      {
        negativeNum = 0x36;
        TimerHalStartTime(g_ecuSecurityTimerHandle_Reprog, 180000);
        g_securityTimerDelayType_Reprog = E_SECURITYTIMER_ACCESSERRORDELAY;
      }
      else
      {
        negativeNum = 0x37;
      }
    }
  }
  return negativeNum;
}

static uint8_t Service0x27Process0x0A(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint8_t responseData[32];
  const int level_index = 3;
  if (g_currentSession != E_PROGROM_SESSION)
    {
        return 0x7F;
    }
  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  if (udsLen != 18)
    return 0x13;

  if (E_UDS_SECURITY_LEVEL_REPROG == g_securitySendSeedLevel)
  {
    if (ByteArrayCheckIsSame(&udsData[2], g_securityKey, g_securityKeyLength) == 0)
    {
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, 0);
      g_currentSecurityLevel = E_UDS_SECURITY_LEVEL_REPROG;
      g_u8ConsecutiveSeedCount[level_index] = 0;
      SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
    }
    else
    {
      if (g_u8ConsecutiveSeedCount[level_index] < 3)
      {
        g_u8ConsecutiveSeedCount[level_index] += 1;
        SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
      }
      if (g_u8ConsecutiveSeedCount[level_index] < 3)
      {
        negativeNum = 0x35;
      }
      else
      {
        negativeNum = 0x36;
        TimerHalStartTime(g_ecuSecurityTimerHandle_Reprog, 180000);
        g_securityTimerDelayType_Reprog = E_SECURITYTIMER_ACCESSERRORDELAY;
      }
    }
    g_securitySendSeedLevel = E_UDS_NONE_SECURITY_LEVEL;
  }
  else
  {
    negativeNum = 0x24;
  }
  return negativeNum;
}
#else
// static uint8_t Service0x27Process0x01(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
// {
//   uint8_t responseData[32];
//   uint8_t negativeNum = 0;

//   responseData[0] = udsData[0] + 0x40;
//   responseData[1] = udsData[1];

//   if (udsLen != 2)
//   {
//     return 0x13;
//   }
//   if (g_ecuResetFlag[0])
//   {
//     negativeNum = 0x37;
//   }
//   else if (g_currentSecurityLevel == E_UDS_SECURITY_LEVEL1)
//   {
//     // uint8_t seedLength;
//     UdsGetSeed(g_securitySeed, &g_securitySeedLength);
//     memset((responseData + 2), 0, g_securitySeedLength);
//     DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, 0);
//     // return negativeNum;
//   }
//   else
//   {
//     if (g_u8ConsecutiveSeedCount[0] < 3)
//     {
//       // if(g_u8ConsecutiveSeedCount==0)
//       {
//         UdsGetSeed(g_securitySeed, &g_securitySeedLength);
//       }
//       UdsSeedtoKey(g_securitySeed, g_securitySeedLength, g_securityKey, &g_securityKeyLength, E_SECURITY_CALCULATE_LEVEL1);
//       memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
//       DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, functionAddressFlag);
//       g_securitySendSeedLevel = E_UDS_SECURITY_LEVEL1;
//     }
//     else
//     {
//       if (g_securityTimerDelayType != E_SECURITYTIMER_ACCESSERRORDELAY)
//       {
//         negativeNum = 0x36;
//         // start delay access timer 180S
//         TimerHalStartTime(g_ecuSecurityTimerHandle, 180000);
//         g_securityTimerDelayType = E_SECURITYTIMER_ACCESSERRORDELAY;
//       }
//       else
//       {
//         negativeNum = 0x37;
//       }
//     }
//   }
//   return negativeNum;
// }
static uint8_t Service0x27Process0x01(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t responseData[32];
  uint8_t negativeNum = 0;
  const int level_index = 0;

  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  if (udsLen != 2)
    return 0x13;

  uint8_t factoryMode = UdsDidGetManufactoryMode();
  if (factoryMode >= 0x10)
  {
    // 0x10-0xFF 返回全0的种子
    memset(g_securitySeed, 0, sizeof(g_securitySeed));
    g_securitySeedLength = 4;

    memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, functionAddressFlag);

    g_currentSecurityLevel = E_UDS_SECURITY_LEVEL1; // 自动解锁
    g_securitySendSeedLevel = E_UDS_NONE_SECURITY_LEVEL;

    return 0;
  }

  if (g_ecuResetFlag[level_index])
  {
    negativeNum = 0x37;
  }
  else if (g_currentSecurityLevel == E_UDS_SECURITY_LEVEL1)
  {
    UdsGetSeed(g_securitySeed, &g_securitySeedLength);
    memcpy((responseData + 2), g_securitySeed, g_securitySeedLength); // 修正2701 0x01-0xFF 返回全0种子
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, 0);
  }
  else
  {
    if (g_u8ConsecutiveSeedCount[level_index] < 3)
    {
      UdsGetSeed(g_securitySeed, &g_securitySeedLength);
      unsigned int key_len_uint = 0;

      int result = Seed2Key(g_securitySeed, g_securitySeedLength, 0x01, NULL, g_securityKey, sizeof(g_securityKey), &key_len_uint);

      if (result == 1)
      {
        g_securityKeyLength = (uint8_t)key_len_uint;
        memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
        DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, functionAddressFlag);
        g_securitySendSeedLevel = E_UDS_SECURITY_LEVEL1;
      }
      else
      {
        negativeNum = 0x24;
      }
    }
    else
    {
      if (g_securityTimerDelayType != E_SECURITYTIMER_ACCESSERRORDELAY)
      {
        negativeNum = 0x36;
        TimerHalStartTime(g_ecuSecurityTimerHandle, 180000);
        g_securityTimerDelayType = E_SECURITYTIMER_ACCESSERRORDELAY;
      }
      else
      {
        negativeNum = 0x37;
      }
    }
  }
  return negativeNum;
}

static uint8_t Service0x27Process0x02(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint8_t responseData[32];
  const int level_index = 0;

  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  if (udsLen != 6)
    return 0x13;

  if (E_UDS_SECURITY_LEVEL1 == g_securitySendSeedLevel)
  {
    if (ByteArrayCheckIsSame(&udsData[2], g_securityKey, g_securityKeyLength) == 0)
    {
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, 0);
      g_currentSecurityLevel = E_UDS_SECURITY_LEVEL1;
      g_u8ConsecutiveSeedCount[level_index] = 0;
      SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
    }
    else
    {
      if (g_u8ConsecutiveSeedCount[level_index] < 3)
      {
        g_u8ConsecutiveSeedCount[level_index] += 1;
        SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
      }
      if (g_u8ConsecutiveSeedCount[level_index] < 3)
      {
        negativeNum = 0x35;
      }
      else
      {
        negativeNum = 0x36;
        TimerHalStartTime(g_ecuSecurityTimerHandle, 180000);
        g_securityTimerDelayType = E_SECURITYTIMER_ACCESSERRORDELAY;
      }
    }
    g_securitySendSeedLevel = E_UDS_NONE_SECURITY_LEVEL;
  }
  else
  {
    negativeNum = 0x24;
  }
  return negativeNum;
}

// --- Level 2 (Standard: 0x03/0x04) ---
static uint8_t Service0x27Process0x03(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t responseData[32];
  uint8_t negativeNum = 0;
  const int level_index = 2;

  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  if (udsLen != 2)
    return 0x13;
  uint8_t factoryMode = UdsDidGetManufactoryMode();

  if (factoryMode >= 0x10)
  {
    // 0x10-0xFF 返回全0的种子
    memset(g_securitySeed, 0, sizeof(g_securitySeed));
    g_securitySeedLength = 4;

    memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, functionAddressFlag);

    g_currentSecurityLevel = E_UDS_SECURITY_LEVEL2_STD; // 解锁到Level 2
    g_securitySendSeedLevel = E_UDS_NONE_SECURITY_LEVEL;

    return 0;
  }
  if (g_ecuResetFlag[level_index])
  {
    negativeNum = 0x37;
  }
  else if (g_currentSecurityLevel == E_UDS_SECURITY_LEVEL2_STD)
  {
    UdsGetSeed(g_securitySeed, &g_securitySeedLength);

    memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, 0);
  }
  else
  {
    if (g_u8ConsecutiveSeedCount[level_index] < 3)
    {
      UdsGetSeed(g_securitySeed, &g_securitySeedLength);

      unsigned int key_len_uint = 0;

      int result = Seed2Key(g_securitySeed, g_securitySeedLength, 0x03, NULL, g_securityKey, sizeof(g_securityKey), &key_len_uint);

      if (result == 1)
      {
        g_securityKeyLength = (uint8_t)key_len_uint;
        memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
        DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, functionAddressFlag);
        g_securitySendSeedLevel = E_UDS_SECURITY_LEVEL2_STD;
      }
      else
      {
        negativeNum = 0x24;
      }
    }
    else
    {
      if (g_securityTimerDelayType_3 != E_SECURITYTIMER_ACCESSERRORDELAY)
      {
        negativeNum = 0x36;
        TimerHalStartTime(g_ecuSecurityTimerHandle_3, 180000);
        g_securityTimerDelayType_3 = E_SECURITYTIMER_ACCESSERRORDELAY;
      }
      else
      {
        negativeNum = 0x37;
      }
    }
  }
  return negativeNum;
}

static uint8_t Service0x27Process0x04(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint8_t responseData[32];
  const int level_index = 2;

  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  if (udsLen != 6)
    return 0x13;

  if (E_UDS_SECURITY_LEVEL2_STD == g_securitySendSeedLevel)
  {
    if (ByteArrayCheckIsSame(&udsData[2], g_securityKey, g_securityKeyLength) == 0)
    {
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, 0);
      g_currentSecurityLevel = E_UDS_SECURITY_LEVEL2_STD;
      g_u8ConsecutiveSeedCount[level_index] = 0;
      SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
    }
    else
    {
      if (g_u8ConsecutiveSeedCount[level_index] < 3)
      {
        g_u8ConsecutiveSeedCount[level_index] += 1;
        SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
      }
      if (g_u8ConsecutiveSeedCount[level_index] < 3)
      {
        negativeNum = 0x35;
      }
      else
      {
        negativeNum = 0x36;
        TimerHalStartTime(g_ecuSecurityTimerHandle_3, 180000);
        g_securityTimerDelayType_3 = E_SECURITYTIMER_ACCESSERRORDELAY;
      }
    }
    g_securitySendSeedLevel = E_UDS_NONE_SECURITY_LEVEL;
  }
  else
  {
    negativeNum = 0x24;
  }
  return negativeNum;
}

// --- Reprogramming (0x11/0x12) ---
static uint8_t Service0x27Process0x11(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t responseData[32];
  uint8_t negativeNum = 0;
  const int level_index = 3;
if (g_currentSession != E_PROGROM_SESSION)
    {
        return 0x7E;
    }
  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  if (udsLen != 2)
    return 0x13;

  if (g_ecuResetFlag[level_index])
  {
    negativeNum = 0x37;
  }
  else if (g_currentSecurityLevel == E_UDS_SECURITY_LEVEL_REPROG)
  {
    UdsGetSeed(g_securitySeed, &g_securitySeedLength);
    memset((responseData + 2), 0, g_securitySeedLength);
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, 0);
  }
  else
  {
    if (g_u8ConsecutiveSeedCount[level_index] < 3)
    {

      UdsGetSeed(g_securitySeed, &g_securitySeedLength);

      unsigned int key_len_uint = 0;

      int result = Seed2Key(g_securitySeed, g_securitySeedLength, 0x11, NULL, g_securityKey, sizeof(g_securityKey), &key_len_uint);

      if (result == 1)
      {
        g_securityKeyLength = (uint8_t)key_len_uint;
        memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
        DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, functionAddressFlag);
        g_securitySendSeedLevel = E_UDS_SECURITY_LEVEL_REPROG;
      }
      else
      {
        negativeNum = 0x24;
      }
    }
    else
    {
      if (g_securityTimerDelayType_Reprog != E_SECURITYTIMER_ACCESSERRORDELAY)
      {
        negativeNum = 0x36;
        TimerHalStartTime(g_ecuSecurityTimerHandle_Reprog, 180000);
        g_securityTimerDelayType_Reprog = E_SECURITYTIMER_ACCESSERRORDELAY;
      }
      else
      {
        negativeNum = 0x37;
      }
    }
  }
  return negativeNum;
}

static uint8_t Service0x27Process0x12(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint8_t responseData[32];
  const int level_index = 3;
  if (g_currentSession != E_PROGROM_SESSION)
    {
        return 0x7E;
    }
  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];

  if (udsLen != 6)
    return 0x13;

  if (E_UDS_SECURITY_LEVEL_REPROG == g_securitySendSeedLevel)
  {
    if (ByteArrayCheckIsSame(&udsData[2], g_securityKey, g_securityKeyLength) == 0)
    {
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, 0);
      g_currentSecurityLevel = E_UDS_SECURITY_LEVEL_REPROG;
      g_u8ConsecutiveSeedCount[level_index] = 0;
      SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
    }
    else
    {
      if (g_u8ConsecutiveSeedCount[level_index] < 3)
      {
        g_u8ConsecutiveSeedCount[level_index] += 1;
        SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
      }
      if (g_u8ConsecutiveSeedCount[level_index] < 3)
      {
        negativeNum = 0x35;
      }
      else
      {
        negativeNum = 0x36;
        TimerHalStartTime(g_ecuSecurityTimerHandle_Reprog, 180000);
        g_securityTimerDelayType_Reprog = E_SECURITYTIMER_ACCESSERRORDELAY;
      }
    }
    g_securitySendSeedLevel = E_UDS_NONE_SECURITY_LEVEL;
  }
  else
  {
    negativeNum = 0x24;
  }
  return negativeNum;
}

//// --- Level 3 (Standard: 0x21/0x22) ---
// static uint8_t Service0x27Process0x21(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
//{
//   uint8_t responseData[32];
//   uint8_t negativeNum = 0;
//   const int level_index = 4;
//
//   responseData[0] = udsData[0] + 0x40;
//   responseData[1] = udsData[1];
//
//   if (udsLen != 2)
//     return 0x13;
//
//   if (g_ecuResetFlag[level_index])
//   {
//     negativeNum = 0x37;
//   }
//   else if (g_currentSecurityLevel == E_UDS_SECURITY_LEVEL3_STD)
//   {
//     UdsGetSeed(g_securitySeed, &g_securitySeedLength);
//     memset((responseData + 2), 0, g_securitySeedLength);
//     DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, 0);
//   }
//   else
//   {
//     if (g_u8ConsecutiveSeedCount[level_index] < 3)
//     {
//       UdsGetSeed(g_securitySeed, &g_securitySeedLength);
//
//       unsigned int key_len_uint = 0;
//
//       int result = Seed2Key(g_securitySeed, g_securitySeedLength, 0x21, NULL, g_securityKey, sizeof(g_securityKey), &key_len_uint);
//
//       if (result == 1)
//       {
//         g_securityKeyLength = (uint8_t)key_len_uint;
//         memcpy((responseData + 2), g_securitySeed, g_securitySeedLength);
//         DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2 + g_securitySeedLength, functionAddressFlag);
//         g_securitySendSeedLevel = E_UDS_SECURITY_LEVEL3_STD;
//       }
//       else
//       {
//         negativeNum = 0x24;
//       }
//     }
//     else
//     {
//       if (g_securityTimerDelayType_L3 != E_SECURITYTIMER_ACCESSERRORDELAY)
//       {
//         negativeNum = 0x36;
//         TimerHalStartTime(g_ecuSecurityTimerHandle_L3, 180000);
//         g_securityTimerDelayType_L3 = E_SECURITYTIMER_ACCESSERRORDELAY;
//       }
//       else
//       {
//         negativeNum = 0x37;
//       }
//     }
//   }
//   return negativeNum;
// }
//
// static uint8_t Service0x27Process0x22(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
//{
//   uint8_t negativeNum = 0;
//   uint8_t responseData[32];
//   const int level_index = 4;
//
//   responseData[0] = udsData[0] + 0x40;
//   responseData[1] = udsData[1];
//
//   if (udsLen != 6)
//     return 0x13;
//
//   if (E_UDS_SECURITY_LEVEL3_STD == g_securitySendSeedLevel)
//   {
//     if (ByteArrayCheckIsSame(&udsData[2], g_securityKey, g_securityKeyLength) == 0)
//     {
//       DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, 0);
//       g_currentSecurityLevel = E_UDS_SECURITY_LEVEL3_STD;
//       g_u8ConsecutiveSeedCount[level_index] = 0;
//       SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
//     }
//     else
//     {
//       if (g_u8ConsecutiveSeedCount[level_index] < 3)
//       {
//         g_u8ConsecutiveSeedCount[level_index] += 1;
//         SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
//       }
//       if (g_u8ConsecutiveSeedCount[level_index] < 3)
//       {
//         negativeNum = 0x35;
//       }
//       else
//       {
//         negativeNum = 0x36;
//         TimerHalStartTime(g_ecuSecurityTimerHandle_L3, 180000);
//         g_securityTimerDelayType_L3 = E_SECURITYTIMER_ACCESSERRORDELAY;
//       }
//     }
//     g_securitySendSeedLevel = E_UDS_NONE_SECURITY_LEVEL;
//   }
//   else
//   {
//     negativeNum = 0x24;
//   }
//   return negativeNum;
// }
#endif
static int16_t Service0x27Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  const Diagnostic_t *psDignostic = (g_serviceECUFunGroup + g_currentServiceNum);

  if (CheckSessionIsValid(g_currentSession, psDignostic->sessionSupport) == 0)
  {
    if (udsLen < 2)
    {
      return 0x13;
    }
    switch (udsData[UDS_OFFSET_SUB_FUNC])
    {
    // Level1
    case 0x01:
      negativeNum = Service0x27Process0x01(udsData, udsLen, functionAddressFlag);
      break;
    case 0x02:
      negativeNum = Service0x27Process0x02(udsData, udsLen, functionAddressFlag);
      break;

    // Level3
    case 0x09:
      negativeNum = Service0x27Process0x09(udsData, udsLen, functionAddressFlag);
      break;
    case 0x0A:
      negativeNum = Service0x27Process0x0A(udsData, udsLen, functionAddressFlag);
      break;
    #if 0
    case 0x03:
      negativeNum = Service0x27Process0x03(udsData, udsLen, functionAddressFlag);
      break;
    case 0x04:
      negativeNum = Service0x27Process0x04(udsData, udsLen, functionAddressFlag);
      break;

    case 0x11:
      negativeNum = Service0x27Process0x11(udsData, udsLen, functionAddressFlag);
      break;
    case 0x12:
      negativeNum = Service0x27Process0x12(udsData, udsLen, functionAddressFlag);
      break;
    
      // case 0x21:
      //   negativeNum = Service0x27Process0x21(udsData, udsLen, functionAddressFlag);
      //   break;
      // case 0x22:
      //   negativeNum = Service0x27Process0x22(udsData, udsLen, functionAddressFlag);
      //   break;
      // case 0x09:
      //   negativeNum = Service0x27Process0x09(udsData, udsLen, functionAddressFlag);
      //   break;
      // case 0x0A:
      //   negativeNum = Service0x27Process0x0A(udsData, udsLen, functionAddressFlag);
      //   break;
      #endif
     
    default:
      negativeNum = 0x12;
      break;
    }
  }
  else
  {
    negativeNum = 0x7F;
  }
  return negativeNum;
}

/*********************************************
read data by identifier


*************************************/
// static int16_t Service0x22Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
// {
//   uint16_t Rdid_len = 0;
//   uint16_t Rdid = 0;
//   uint8_t negativeNum = 0;
//   uint16_t didNum = 0;
//   uint16_t i;
//   uint16_t txLength;
//   uint8_t resultSum = E_SUBFUN_GET_INVALID_DID;
//   // uint8_t responseData[64];
//   int16_t ret;
//   const Diagnostic_t *psDignostic = NULL;
//   SeriveGetSubFunState_e result;
//   pService22_DIDFunction pReadFun;

//   psDignostic = (g_serviceECUFunGroup + g_currentServiceNum);

//   /*responseData[0] = udsData[0]+0x40;
//   responseData[1] = udsData[1];
//   responseData[2] = udsData[2];*/
//   g_udsTxBuffer[0] = udsData[0] + 0x40;

//   if (CheckSessionIsValid(g_currentSession, psDignostic->sessionSupport) == 0)
//   {
//     txLength = 1;
//     if ((udsLen >= 0x03) && (udsLen <= (0x03 + 5 * 2)) && (((udsLen - 1) % 2) == 0))
//     {
//       didNum = (udsLen - 1) / 2;
//       for (i = 0; i < didNum; i++)
//       {
//         pReadFun = NULL;
//         Rdid = ((udsData[i * 2 + 1] << 8) | udsData[i * 2 + 2]);
//         result = Service22GetDidFunction(Rdid, &pReadFun);

//         if (result != E_SUBFUN_GET_OK)
//         {
//           continue;
//         }
//         resultSum = E_SUBFUN_GET_OK;
//         if (result == E_SUBFUN_GET_OK)
//         {
//           ret = pReadFun(&g_udsTxBuffer[txLength + 2], &Rdid_len);
//           if (ret == 0)
//           {
//             g_udsTxBuffer[txLength] = udsData[i * 2 + 1];
//             g_udsTxBuffer[txLength + 1] = udsData[i * 2 + 2];
//             txLength += (Rdid_len + 2);
//           }
//           else
//           {
//             negativeNum = 0x22;
//             break;
//           }
//         }
//         else if (result == E_SUBFUN_GET_INVALID_DID)
//         {
//           negativeNum = 0x31;
//           break;
//         }
//         else
//         {
//           if (functionAddressFlag)
//           {
//             return negativeNum;
//           }
//           /*������-31*/
//           negativeNum = 0x31;
//           break;
//         }
//       }
//       if (resultSum != E_SUBFUN_GET_OK)
//       {
//         negativeNum = 0x31;
//       }

//       if (0 == negativeNum)
//       {
//         DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, g_udsTxBuffer, txLength, 0);
//       }
//     }
//     else
//     {
//       /*�����ĵ����ݸ�ʽ�򳤶Ȳ����ϱ�׼-13*/
//       negativeNum = 0x13;
//     }
//   }
//   else
//   {
//     /*if(functionAddressFlag)
//     {
//         return negativeNum;
//     }*/
//     /*�ӹ��ܲ�֧�ֵ���ϻỰ7F*/
//     negativeNum = 0x7F;
//   }
//   return negativeNum;
// }

//
static int16_t Service0x22Process(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint16_t first_did;

  if (udsLen < 3 || ((udsLen - 1) % 2) != 0)
  {
    return 0x13;
  }

  first_did = (udsData[1] << 8) | udsData[2];

  if (IsDidPassthrough_22(first_did))
  {
    uint8_t mpu_response_buffer[512];
    uint16_t mpu_response_length = 0;
    int16_t ret;
    // TBOX_PRINT("22 passthrough DID: %04X\n", first_did);
    ret = CanPassthrough_RequestAndGetResponse(udsData, udsLen, mpu_response_buffer, &mpu_response_length);
    if (ret == 0 && mpu_response_length > 0)
    {
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, mpu_response_buffer, mpu_response_length, functionAddressFlag);
      return 0;
    }
    else
    {
      negativeNum = 0x31;
    }
  }
  else
  {

    uint16_t Rdid_len = 0;
    uint16_t Rdid = 0;
    uint16_t didNum = 0;
    uint16_t i;
    uint16_t txLength;
    pService22_DIDFunction pReadFun;

    g_udsTxBuffer[0] = udsData[0] + 0x40;
    txLength = 1;
    didNum = (udsLen - 1) / 2;

    for (i = 0; i < didNum; i++)
    {
      Rdid = ((udsData[i * 2 + 1] << 8) | udsData[i * 2 + 2]);

      if (Service22GetDidFunction(Rdid, &pReadFun) == E_SUBFUN_GET_OK)
      {
        if (pReadFun(&g_udsTxBuffer[txLength + 2], &Rdid_len) == 0)
        {
          g_udsTxBuffer[txLength] = (uint8_t)(Rdid >> 8);
          g_udsTxBuffer[txLength + 1] = (uint8_t)(Rdid & 0xFF);
          txLength += (Rdid_len + 2);
        }
        else
        {
          negativeNum = 0x22; // ConditionsNotCorrect
          break;
        }
      }
      else
      {
        // 如果DID不在透传列表，也不在本地总列表，则为无效DID
        negativeNum = 0x31; // RequestOutOfRange
        break;
      }
    }

    if (negativeNum == 0)
    {
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, g_udsTxBuffer, txLength, functionAddressFlag);
    }
  }

  return negativeNum;
}
/**********************************
write data by identifier


********************************/
#if 0
// 针对ES Key的写入做特殊处理
static int16_t ProcessWriteDidESKey(uint8_t *udsData, uint16_t udsLen)
{
  uint8_t factoryMode = UdsDidGetManufactoryMode();
  bool securityCheckPassed = false;

  if (factoryMode >= 0x10)
  {
    // 工厂模式下，不需要任何安全等级即可写入
    securityCheckPassed = true;
    // TBOX_PRINT("Factory mode, no security check for ES Key write\n");
  }
  else
  {
    if (g_currentSecurityLevel == E_UDS_SECURITY_LEVEL2_STD)
    {
      securityCheckPassed = true;
    }
  }

  if (!securityCheckPassed)
  {
    return 0x33;
  }
  if (udsLen - 3 != 16)
  {
    return 0x13;
  }
  if (Service2EWriteESKey(&udsData[3], 16) == 0)
  {
    uint8_t responseData[3];
    responseData[0] = udsData[0] + 0x40;
    responseData[1] = udsData[1];
    responseData[2] = udsData[2];
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 3, 0);

    return 0;
  }
  else
  {
    return 0x72;
  }
}
  #endif

static int16_t Service0x2EProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  const Diagnostic_t *pDiagnostic;
  uint16_t did;
  uint8_t negativeNum = 0;
  pService2E_DIDFunction pWriteFun;

  pDiagnostic = g_serviceECUFunGroup + g_currentServiceNum;

  if (udsLen < 3)
  {
    return 0x13;
  }
  did = (udsData[UDS_OFFSET_SUB_FUNC] << 8) | udsData[UDS_OFFSET_SUB_FUNC + 1];
  if (CheckSessionIsValid(g_currentSession, pDiagnostic->sessionSupport) != 0)
  {
    return 0x7F;
  }
  #if 0
  if (did == 0x010D)
  {
    return ProcessWriteDidESKey(udsData, udsLen);
  }
  #endif
  switch (Service2EGetDidFunction(did, &pWriteFun, udsLen))
  {
  case E_SUBFUN_GET_OK:
    break;
  case E_SUBFUN_GET_INVALID_LENGTH:
    return 0x13;
  case E_SUBFUN_GET_INVALID_DID:
    return 0x31;
  case E_SUBFUN_GET_INVALID_SECURITY:
    return 0x33;
  default:
    return 0x11;
  }

  // uint8_t u8Busy7f78CMD[8] = {0x03, 0x7f, 0x2e, 0x78, 0x00, 0x00, 0x00, 0x00};
  // DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, &u8Busy7f78CMD[1], 3, 0);

  int16_t localWriteRet = pWriteFun(&udsData[3], udsLen - 3);

  if (localWriteRet != 0)
  {
    return (localWriteRet > 0) ? localWriteRet : 0x72;
  }

  uint8_t responseData[32];
  responseData[0] = udsData[0] + 0x40;
  responseData[1] = udsData[1];
  responseData[2] = udsData[2];

  if (1)//(did == 0xB2E5 || did == 0xF18C || did == 0xF187|| did == 0xB2B4 || did == 0x0120)
  {
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 3, 0);
  }
  else
  {
    uint8_t mpu_response_buffer[64]; // mpu会给我回什么
    uint16_t mpu_response_length = 0;
    int8_t passthroughRet;
    if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
    {
      return 0x72;
    }
    passthroughRet = CanPassthrough_RequestAndGetResponse(udsData, udsLen, mpu_response_buffer, &mpu_response_length);

    if (passthroughRet == 0 && mpu_response_length > 0)
    {
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, mpu_response_buffer, mpu_response_length, 0);
    }
    else
    {
      negativeNum = 0x72;
    }
  }
  return negativeNum;
}
// static int16_t Service0x2EProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
// {
//   const Diagnostic_t *pDiagnostic;
//   uint16_t did;
//   uint8_t negativeNum = 0;
//   pService2E_DIDFunction pWriteFun;

//   pDiagnostic = g_serviceECUFunGroup + g_currentServiceNum;

//   if (udsLen < 3)
//   {
//     return 0x13;
//   }
//   did = (udsData[UDS_OFFSET_SUB_FUNC] << 8) | udsData[UDS_OFFSET_SUB_FUNC + 1];
//   if (CheckSessionIsValid(g_currentSession, pDiagnostic->sessionSupport) != 0)
//   {
//     return 0x7F;
//   }
//   switch (Service2EGetDidFunction(did, &pWriteFun, udsLen))
//   {
//   case E_SUBFUN_GET_OK:
//     break;
//   case E_SUBFUN_GET_INVALID_LENGTH:
//     return 0x13;
//   case E_SUBFUN_GET_INVALID_DID:
//     return 0x31;
//   case E_SUBFUN_GET_INVALID_SECURITY:
//     return 0x33;
//   default:
//     return 0x11;
//   }

//   if (did == 0x010D || did == 0xB2E5)
//   {

//     if (pWriteFun == NULL)
//     {
//       return 0x11; // ServiceNotSupported
//     }

//     uint8_t u8Busy7f78CMD[8] = {0x03, 0x7f, 0x2e, 0x78, 0x00, 0x00, 0x00, 0x00};
//     DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, &u8Busy7f78CMD[1], 3, 0);

//     int16_t localWriteRet = pWriteFun(&udsData[3], udsLen - 3);

//     if (localWriteRet == 0)
//     {
//       uint8_t responseData[3];
//       responseData[0] = udsData[0] + 0x40;
//       responseData[1] = udsData[1];
//       responseData[2] = udsData[2];
//       DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 3, 0);
//     }
//     else
//     {
//       negativeNum = (localWriteRet > 0) ? localWriteRet : 0x72;
//     }
//   }
//   else
//   {
//     uint8_t mpu_response_buffer[512];
//     uint16_t mpu_response_length = 0;
//     int16_t passthroughRet;

//     passthroughRet = CanPassthrough_RequestAndGetResponse(udsData, udsLen, mpu_response_buffer, &mpu_response_length);

//     if (passthroughRet == 0 && mpu_response_length > 0)
//     {
//       DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, mpu_response_buffer, mpu_response_length, 0);
//       return 0;
//     }
//     else
//     {
//       negativeNum = 0x72;
//     }
//   }

//   return negativeNum;
// }
/**************************************************
self defined uds service used for EOL test


// ******************************************************/
// static int16_t Service0xBAProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
// {
//   uint8_t negativeNum = 0;
//   uint8_t responseData[32];
//   uint16_t responseDataLength;

//   negativeNum = ServiceTestSecurityProcess(udsData, udsLen, responseData, &responseDataLength);
//   if (negativeNum == 0)
//   {
//     DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, responseDataLength, 0);
//   }
//   return negativeNum;
// }

/**************************************************
self defined uds service used for EOL test


******************************************************/
int16_t Service0xBBProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum = 0;
  uint8_t responseData[128];
  uint16_t responseDataLength;

  negativeNum = ServiceTestProcess(udsData, udsLen, responseData, &responseDataLength);
  if (negativeNum == 0)
  {
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, responseDataLength, 0);
  }
  return negativeNum;
}

static int16_t ServiceNoDefinitionProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  int16_t negativeNum;
  negativeNum = 1;

  if (!functionAddressFlag)
  {
    negativeNum = 0x11;
  }
  return negativeNum;
}

static int16_t NegativeResponseProcess(uint8_t negativeNum, uint8_t ServiceId, uint8_t functionAddressFlag)
{
  uint8_t responseData[8];
  responseData[0] = 0x7F;
  responseData[1] = ServiceId;
  responseData[2] = negativeNum;

  if (functionAddressFlag && ((negativeNum == 0x11) || (negativeNum == 0x7F) ||
                              (negativeNum == 0x12) || (negativeNum == 0x7E) || (negativeNum == 0x31)))
  {
    return 0;
  }

  DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 3, 0);

  return 0;
}

static pServiceFunction GetEcuServiceFunction(uint8_t ServiceId)
{
  uint16_t i;

  for (i = 0; i < (sizeof(g_serviceECUFunGroup) / sizeof(g_serviceECUFunGroup[0])); i++)
  {
    if (ServiceId == g_serviceECUFunGroup[i].SID)
    {
      g_currentServiceNum = i;
      return g_serviceECUFunGroup[i].pFun;
    }
  }
  return ServiceNoDefinitionProcess;
}
uint16_t GetVehicleSpeed(void)
{
  double rawValue = 0.0f;
  uint16_t speed = 0;
  const can0_signal_configure_t *pCanCfg = GetCan0SignalConfigure();
  CanParseSdkReadSignal(VEHICLE_CAN_UNPACK_FORMAT_INTEL, &pCanCfg->Tx_DB_VehicleSpeed, &rawValue);
  speed = (uint16_t)(rawValue / 0.00390625); // 原始值→0.00390625单位
  if (speed <= 65280) {  // 255/0.00390625=65280
    return speed;
  }
  return 0;
}
static int16_t DiagnosticResponseProcess(uint8_t *udsData, uint16_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t funNum = 0;
  uint8_t negativeNum = 0;
  pServiceFunction pFun;  

  funNum = udsData[UDS_OFFSET_SID];
  if ((funNum != 0x19) && (funNum != 0x3E))
  {
#if TEST_CODE
    if (0)
#else
    uint16_t vehicleSpeed = GetVehicleSpeed();
    if ((vehicleSpeed >= 1280) && (vehicleSpeed != 0xFFFF)) // 5/0.00390625=1280
#endif
    {
      negativeNum = 0x22;
      NegativeResponseProcess(negativeNum, funNum, functionAddressFlag);
      return 0;
    }
  }

  pFun = GetEcuServiceFunction(funNum);
  negativeNum = pFun(udsData, udsLen, functionAddressFlag);

  if (0 != negativeNum)
  {
    if (negativeNum == 1)
    {
      return 0;
    }
    NegativeResponseProcess(negativeNum, funNum, functionAddressFlag);
  }
#if 0  
  else
  {    
    if(g_currentSession != E_DEFAULT_SESSION)
    {
      g_ecuOnlineFlag = 1;      
    }      
  }
#endif

  if (g_currentSession != E_DEFAULT_SESSION)
  {
    g_ecuOnlineFlag = 1;
  }

  return 0;
}

int16_t TaskEcuDiagnosticInit(void)
{
  return 0;
}

// Place following data in section SYS_RAM

static CanTpSdkBuffer_t g_tpBuffer;

const CanTpSdkParameter_t g_tpParameter =
    {
        .blockSize_APP = 0,  // application mode 连续接收多少帧后，再次发送流控帧
        .STmin_APP = 0,      // application mode
        .blockSize_boot = 0, // boot mode
        .STmin_boot = 0,     // boot mode
        .fillByte = 0xCC,
        .N_As_enforced = 25,  // 70ms
        .N_Ar_enforced = 25,  // 70ms
        .N_Bs_enforced = 75,  // 70ms
        .N_Br_enforced = 75,  // 70ms
        .N_Cs_enforced = 70,  // 70ms
        .N_Cr_enforced = 150, // 70ms
        .N_As_obd = 25,       // 25ms
        .N_Ar_obd = 25,       // 25ms
        .N_Bs_obd = 75,       // 70m
        .N_Br_obd = 0,        ////(N_Br+N_Ar)＜25ms
        .N_Cs_obd = 0,        //(N_Cs+N_As) ＜50ms
        .N_Cr_obd = 150,      //(N_Br+N_Ar)＜25ms
};

/*static void PrintTaskInfo(void)
{
    static uint32_t cycleCount = 0;
    cycleCount++;
    if(cycleCount<100)
    {
      return;
    }
    cycleCount = 0;
    unsigned long uxHighWaterMark_DataToCPU;
    uxHighWaterMark_DataToCPU = uxTaskGetStackHighWaterMark(NULL);
    TBOX_PRINT("diagnostic thread remain stack space:%d.\r\n",uxHighWaterMark_DataToCPU);
}*/

static void ResetTboxStatusUpdate(void)
{
  g_currentSession = E_DEFAULT_SESSION;
  g_currentSecurityLevel = E_UDS_NONE_SECURITY_LEVEL;
  g_securitySendSeedLevel = E_UDS_NONE_SECURITY_LEVEL;
  CanTpSdkSetBootMode(g_tpHandle, 0);
  /*if(g_bDtcControlFlag)
  {
    g_bDtcControlFlag = 0;
    //reset DTC control setting
    DtcProcessEnable();
  }
  if(g_bCommunicationControlFlag)
  {
    g_bCommunicationControlFlag = 0;
    //reset Communication control setting
  }*/
  SessionChangeToDefaultProcess();
}
#if 0
static int16_t DiagnosticWorkingModeInit(void)
{
  int16_t ret = 0;
  uint8_t data[2] = {0}; // nomal mode
  uint32_t lenth = 0;

  // ret = WorkFlashUserInfoRead(E_WORKING_MODE,data,&lenth);
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_WORKING_MODE, data, &lenth);
  if (ret != 0)
  {
    return -1;
  }
  if ((lenth == 2) && (data[0] == 0x5A) && (data[1] < 3))
  {
    if (data[1] == 0) // nomal mode
    {
      MpuPowerSyncSdkSetRkMode(1);
    }
    else if (data[1] == 1) // factory mode
    {
      MpuPowerSyncSdkSetRkMode(3);
      // 关闭远程诊断功能
      RemoteDiagnosticSdkDisable();
    }
    else if (data[1] == 2) // 运输模式
    {
      MpuPowerSyncSdkSetRkMode(2);
      // 进入深度休眠，关闭MPU
      PowerManageSdkDeepSleep(1);
    }
    return 0;
  }
  else
  {
    data[0] = 0x5A;
    data[1] = 0; // 初始化未nomal mode
    // ret = WorkFlashUserInfoStore(E_WORKING_MODE,data,2);
    ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_WORKING_MODE, data, 2);
  }

  return ret;
}
#endif
#if 0
static void DiagnosticSeedAccessInit(void)
{
  GetSeedAccessCountFromVolatile(g_u8ConsecutiveSeedCount);
  if ((g_u8ConsecutiveSeedCount[0] == 3))
  {
    TimerHalStartTime(g_ecuSecurityTimerHandle, 180000); // ecu reset security access delay
    g_securityTimerDelayType = E_SECURITYTIMER_RESETDELAY;
    g_ecuResetFlag[0] = 1;
    TBOX_PRINT("reset TimerStart\r\n");
  }
  else if (g_u8ConsecutiveSeedCount[0] > 4)
  {
    g_u8ConsecutiveSeedCount[0] = 0;
    g_ecuResetFlag[0] = 0;
  }
  else
  {
    g_ecuResetFlag[0] = 0;
  }
  if ((g_u8ConsecutiveSeedCount[1] == 3))
  {
    TimerHalStartTime(g_ecuSecurityTimerHandle_2, 180000); // ecu reset security access delay
    g_securityTimerDelayType_2 = E_SECURITYTIMER_RESETDELAY;
    g_ecuResetFlag[1] = 1;
    TBOX_PRINT("reset TimerStart\r\n");
  }
  else if (g_u8ConsecutiveSeedCount[1] > 4)
  {
    g_u8ConsecutiveSeedCount[1] = 0;
    g_ecuResetFlag[1] = 0;
  }
  else
  {
    g_ecuResetFlag[1] = 0;
  }
  if ((g_u8ConsecutiveSeedCount[2] == 3))
  {
    TimerHalStartTime(g_ecuSecurityTimerHandle_3, 180000); // ecu reset security access delay
    g_securityTimerDelayType_3 = E_SECURITYTIMER_RESETDELAY;
    g_ecuResetFlag[2] = 1;
    TBOX_PRINT("reset TimerStart\r\n");
  }
  else if (g_u8ConsecutiveSeedCount[2] > 4)
  {
    g_u8ConsecutiveSeedCount[2] = 0;
    g_ecuResetFlag[2] = 0;
  }
  else
  {
    g_ecuResetFlag[2] = 0;
  }
}
#endif
// static void UdsServiceProcess(uint8_t *pTpData, uint32_t tpDataLength, uint8_t FunctionalAddress);

void TaskEcuDiagnostic(void *pvParameters)
{

  int16_t ecuOnlineTimerHandle;
  int16_t ret;
  uint8_t functionalFlag = 0;
  // int16_t tpHandle;
  uint8_t isS3ServerTimerActive = 0;

  g_tpHandle = CanTpSdkInitialize(TBOX_CAN_CHANNEL_1, &g_tpParameter, &g_tpBuffer);

  if (g_tpHandle < 0)
  {
    return;
  }
  //ServiceTestSetDiagnosticCan(TBOX_CAN_CHANNEL_2);

  CanTpSdkSetCanId(g_tpHandle, g_physicalReceiveCanId, g_functionalReceiveCanId, g_physicalTransmitCanId);

  g_ecuSecurityTimerHandle = TimerHalOpen();
  g_ecuSecurityTimerHandle_2 = TimerHalOpen();
  g_ecuSecurityTimerHandle_3 = TimerHalOpen();
  ecuOnlineTimerHandle = TimerHalOpen();

  g_ecuSecurityTimerHandle_Reprog = TimerHalOpen();
  // g_ecuSecurityTimerHandle_L3 = TimerHalOpen();
  EolTestSyncWithCpuInit();
  // RemoteSyncWithCpuInit();
  //  EolConfigureInit();
  //  //DiagCanReportValueInit();
  //  VinInit();
  EskkeyCheckInit();

  // CopyMcuAppSoftWareVersionToFlash();// save app software version
  // CopyMcuAppSoftWareNumberToFlash(); // save app software Number
  // CopyMcuAppPartNumberToFlash(); // save app PartNumber
  //  DiagnosticSeedAccessInit();

  // //工作模式初始化
  // DiagnosticWorkingModeInit();

  while (1)
  {
    ret = CanTpSdkDataReceive(g_tpHandle, g_udsRxData, &g_udsRxDataLen, &functionalFlag);
    if (0 == ret)
    {
      if (g_sleepFlag == 0)
      {
        g_virtualTpFlag = 0;
        {
          DiagnosticResponseProcess(g_udsRxData, g_udsRxDataLen, functionalFlag);
        }
      }
    }
    uint8_t currentTesterPresent = ((g_currentSession != E_DEFAULT_SESSION) || (isS3ServerTimerActive == 1)) ? 1 : 0;

    if (currentTesterPresent != g_isTesterPresent)
    {
      g_isTesterPresent = currentTesterPresent;

      if (currentTesterPresent)
      {
        RemoteDiagnosticSdkShortDisable(); // 暂时禁止远程诊断
      }
      else
      {
        RemoteDiagnosticSdkRecover(); // 恢复远程诊断
      }
    }

    ret = -1;
    ret = VirtualTpSdkServerReceive(&g_virtualTpChanalId,g_udsRxData,sizeof(g_udsRxData),&g_udsRxDataLen);
    if(0==ret)
    {
      if(g_sleepFlag==0)
      {
        g_virtualTpFlag = 1;
        {
            DiagnosticResponseProcess(g_udsRxData, g_udsRxDataLen, 0);
        }
      }
    }

    if (g_ecuOnlineFlag)
    {
      g_ecuOnlineFlag = 0;
      TimerHalStartTime(ecuOnlineTimerHandle, 5000);
      isS3ServerTimerActive = 1;
    }
    if (TimerHalIsTimeout(ecuOnlineTimerHandle) == 0)
    {
      TimerHalStopTime(ecuOnlineTimerHandle);
      ResetTboxStatusUpdate();
      isS3ServerTimerActive = 0;
    }
    if (TimerHalIsTimeout(g_ecuSecurityTimerHandle) == 0)
    {
      TBOX_PRINT("Timer out!!!\r\n");
      TimerHalStopTime(g_ecuSecurityTimerHandle);
      if (E_SECURITYTIMER_RESETDELAY == g_securityTimerDelayType)
      {
        g_ecuResetFlag[0] = 0;
        g_u8ConsecutiveSeedCount[0] = 0;
        SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
      }
      if (E_SECURITYTIMER_ACCESSERRORDELAY == g_securityTimerDelayType)
      {
        // g_u8ConsecutiveSeedCount--;
        g_u8ConsecutiveSeedCount[0] = 0;
        SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
      }
      g_securityTimerDelayType = E_SECURITYTIMER_NONE;
    }
    if (TimerHalIsTimeout(g_ecuSecurityTimerHandle_2) == 0)
    {
      TBOX_PRINT("Timer out!!!\r\n");
      TimerHalStopTime(g_ecuSecurityTimerHandle_2);
      if (E_SECURITYTIMER_RESETDELAY == g_securityTimerDelayType_2)
      {
        g_ecuResetFlag[1] = 0;
        g_u8ConsecutiveSeedCount[1] = 0;
        SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
      }
      if (E_SECURITYTIMER_ACCESSERRORDELAY == g_securityTimerDelayType_2)
      {
        // g_u8ConsecutiveSeedCount--;
        g_u8ConsecutiveSeedCount[1] = 0;
        SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
      }
      g_securityTimerDelayType_2 = E_SECURITYTIMER_NONE;
    }
    if (TimerHalIsTimeout(g_ecuSecurityTimerHandle_3) == 0)
    {
      TBOX_PRINT("Timer out!!!\r\n");
      TimerHalStopTime(g_ecuSecurityTimerHandle_3);
      if (E_SECURITYTIMER_RESETDELAY == g_securityTimerDelayType_3)
      {
        g_ecuResetFlag[2] = 0;
        g_u8ConsecutiveSeedCount[2] = 0;
        SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
      }
      if (E_SECURITYTIMER_ACCESSERRORDELAY == g_securityTimerDelayType_3)
      {
        // g_u8ConsecutiveSeedCount--;
        g_u8ConsecutiveSeedCount[2] = 0;
        SaveSeedAccessCountToNonVolatile(g_u8ConsecutiveSeedCount);
      }
      g_securityTimerDelayType_3 = E_SECURITYTIMER_NONE;
    }

    vTaskDelay(5);
    ServiceTestCycleProcess(5); // 5ms
    // PrintTaskInfo();
    // CheckEolConfigureState();
    // CheckCanReportState();
    // CheckVinCyleProcess();
    // CheckEskValidtateDtc();
  }
}

/**************************************************************
Description:	Communication Control process(0x28)
Input:
bCanRxMsg	receive can data
Output:	        service ID
Return:	        NRC
Others:
***************************************************************/
static uint8_t CommunicationControl(uint8_t ContolType, uint8_t SubService)
{
  uint8_t ret = 0;

  if (ContolType == 0x00) //0x00 EnableRxAndTx
  {
    switch (SubService)
    {
  #if 0
    case 01:
      CanPeriodSendEnableAll(); // 启用周期性应用报文发送
      break;
    case 02:
    #if (NM_PROTOCOL_TYPE == NM_PROTOCOL_OSEK)
      Nm_AllEnableCommunication(); // 启用网络管理报文发送/接收
    #else
      AutosarNmSdkEnableCommunication(0);
    #endif
      break;
  #endif
    case 03:
      // CycleSendEnable(TBOX_CAN_CHANNEL_E);
      if (g_bCommunicationControlFlag == 1)
      {
        /*CycleSendEnableAll();
        SgmwGWNm_AllEnableCommunication();   */
        CanPeriodSendEnableAll();
      #if (NM_PROTOCOL_TYPE == NM_PROTOCOL_OSEK)
        Nm_AllEnableCommunication();
      #else
        AutosarNmSdkEnableCommunication(0);
      #endif
        g_bCommunicationControlFlag = 0;
      }
      break;

    default:
      ret = 0x31;
      break;
    }
  }
  else if (ContolType == 0x03) //0x03 DisableRxAndTx
  {
    switch (SubService)
    {
  #if 0
    case 01:
      CanPeriodSendDisableAll(); // 停止周期性应用报文发送
      g_bCommunicationControlFlag = 1;
      break;
    case 02:
    #if 0 //osek
      OsekNmSdkDisableTx(0); // 停止网络管理报文发送/接收
    #else
      AutosarNmSdkDisableCommunication(0); // 网管报文
    #endif
      g_bCommunicationControlFlag = 1;
      break;
    #endif
    case 03:
      // CycleSendDisable(TBOX_CAN_CHANNEL_E);
      if (g_bCommunicationControlFlag == 0)
      {
        /*CycleSendDisableAll();
        SgmwGWNm_AllDisableCommunication();*/
        CanPeriodSendDisableAll();           // 应用报文
      #if (NM_PROTOCOL_TYPE == NM_PROTOCOL_OSEK)
        Nm_AllDisableCommunication(); // 网管报文
      #else
        AutosarNmSdkDisableCommunication(0); // 网管报文
      #endif
        g_bCommunicationControlFlag = 1;
      }
      break;

    default:
      ret = 0x31;
      break;
    }
  }
  else
  {
    ret = 0x12;
  }

  return ret;
}

/**************************************************************
 *
 *
 *                   Boot Funcation
 *
 *
 ***************************************************************/
// #define _FLASH_DRIVER_RAM  1
// static uint32_t g_flashAddress = 0;
// static uint32_t g_flashSize = 0;
// static uint32_t g_flashCrc32Value;
// static uint32_t g_flashDriverDownLoadFlag = 0;
// static uint8_t g_blockSequenceCounter = 0;
// static uint8_t g_requestDownLoadFlag = 0;

/******************************************************
return
   0: success
*******************************************************/
int16_t writeToRam(uint32_t address, uint8_t *pData, uint32_t dataLength)
{
  // uint8_t *pRam;
  // uint32_t i;
  // pRam = (uint8_t *)address;
  // for(i=0;i<dataLength;i++)
  //{
  //   pRam[i] = pData[i];
  // }
  return 0;
}

// static int8_t FlashErase(uint32_t address, uint32_t size)
//{
//   int8_t ret;
// #ifdef _FLASH_DRIVER_RAM
//   if (g_flashDriverDownLoadFlag == 0)
//   {
//     return -2;
//   }
// #endif
//   //  //עҢ,Ҫ??dress?ĵؖ???Χ׶??-
//   //  if (address >= 0x10018000 && address < 0x10078000)
//   //  {
//   //      FOTA_EraseSectors(1);
//   //  }
//   //  else if(address >= 0x12018000 && address < 0x12078000)
//   //  {
//   //      FOTA_EraseSectors(2);
//   //  }
//
//   //     if (Cy_Flashc_GetMainMap() == CY_FLASH_MAPPING_A)
//   //     {
//   //         TBOX_PRINT("Erase APP2!----------------------------------->\r\n");
//   // //      FOTA_EraseSectors(2);
//   //         FlashHalOtaFlashErase(1);
//   //     }
//   //     else
//   //     {
//   //         TBOX_PRINT("Erase APP1!----------------------------------->\r\n");
//   // //      FOTA_EraseSectors(1);
//   //         FlashHalOtaFlashErase(0);
//   //     }
//
//   /// FOTA_EraseSectors(2);
//   /*
//   uint32_t* p_TestFlsTop = (uint32_t*)0x10010000;
//
//   for (uint8_t i = 0; i < 30; i++) // 30 * 32k byte =960k byte
//   {
//      for(uint32_t i_wordId = 0; i_wordId < 0x08000; i_wordId++)
//      {
//          CY_ASSERT(p_TestFlsTop[i_wordId] == 0xFFFFFFFF);
//      }
//   }
//   */
//   // Term_Printf("Erase completed\r\n");
//   ret = 0;
//
//   return ret;
// }

static uint8_t CharToHex(uint8_t charData)
{
  uint8_t value;
  if ((charData >= '0') && (charData <= '9'))
  {
    value = charData - '0';
  }
  else if ((charData >= 'a') && (charData <= 'f'))
  {
    value = charData - 'a' + 10;
  }
  else if ((charData >= 'A') && (charData <= 'F'))
  {
    value = charData - 'A' + 10;
  }
  else
  {
    value = 0;
  }
  return value;
}

int16_t CharArrayToHexArray(uint8_t *pCharIn, uint16_t charLengthIn, uint8_t *pHexOut, uint32_t *pHexLenthOut)
{
  uint16_t hexLength, i;

  if (charLengthIn == 0)
  {
    return -1;
  }
  hexLength = charLengthIn / 2;
  for (i = 0; i < hexLength; i++)
  {
    pHexOut[i] = (CharToHex(pCharIn[2 * i]) << 4) + CharToHex(pCharIn[2 * i + 1]);
  }
  *pHexLenthOut = hexLength;
  return 0;
}

///***************************************
// return :
//   0: checksum is ok,ϖԚֱ?ӷ??؏K
//*****************************************/
// static int8_t FlashCheckSumIsOk(uint32_t checkSum) // 0x31?????hecksum
//{
//   int8_t ret;
//   ret = 0;
//   g_flashCrc32Value = 0xFFFFFFFF;
//   g_flashCrc32Value = Crc32(&g_crc32Object, g_flashCrc32Value, (uint8_t *)FLASH_APP_BANKB_CM4_BASE_ADDRESS, g_flashSize);
//   g_flashCrc32Value ^= 0xFFFFFFFF;
//
//   // alter,2023-1-3,
//   // g_flashCrc32Value = GetFlashCrcCheckSum();
//   //  g_flashCrc32Value = checkSum;
//   if (g_flashCrc32Value != checkSum)
//   {
//     ret = 1;
//   }
//   return ret;
// }

// static void TransmitNegative78ResponseData(uint8_t serviceId)
//{
//   uint8_t responseData[8];
//   responseData[0] = 0x7F;
//   responseData[1] = serviceId;
//   responseData[2] = 0x78;
//   DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 3, 0);
// }

// static uint8_t BootSerivice0x10Process(uint8_t *pTpData, uint32_t dataLength, uint8_t functionalAddress)
//{
//   uint8_t negativeNum;
//   uint8_t subFunction;
//   uint8_t posRspMsgIndication;
//   uint8_t responseData[20];
//
//   negativeNum = 0;
//   subFunction = pTpData[1] & 0x7F;
//   posRspMsgIndication = (pTpData[1] >> 7) & 0x01;
//
//   if (0x01 == subFunction) // default
//   {
//     if (posRspMsgIndication == 0)
//     {
//       responseData[0] = 0x50;
//       responseData[1] = 0x01;
//       responseData[2] = 0x00;
//       responseData[3] = 0x32;
//       responseData[4] = 0x01;
//       responseData[5] = 0xf4;
//       // alter,??,bootloader?????CAN
//       DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 6, functionalAddress);
//       vTaskDelay(50);
//       // alter,Եô?ԓ?CYT2B7
//       PeripheralHalMcuHardReset();
//       CanTpSdkSetBootMode(g_tpHandle, 0);
//     }
//   }
//   else if (0x02 == subFunction) // programing
//   {
//     if ((g_currentSession == E_EXTEND_SESSION) || (g_currentSession == E_DEFAULT_SESSION) || (g_currentSession == E_PROGROM_SESSION))
//     {
//       g_ecuOnlineFlag = 1;
//       g_currentSession = E_PROGROM_SESSION;
//       if (posRspMsgIndication == 0)
//       {
//         responseData[0] = 0x50;
//         responseData[1] = 0x02;
//         responseData[2] = 0x00;
//         responseData[3] = 0x32;
//         responseData[4] = 0x01;
//         responseData[5] = 0xf4;
//         DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 6, functionalAddress);
//       }
//       CanTpSdkSetBootMode(g_tpHandle, 1);
//     }
//     else
//     {
//       negativeNum = 0x22;
//     }
//   }
//   else if (0x03 == subFunction) // extended
//   {
//     g_ecuOnlineFlag = 1;
// #if 0
//         if(m_udsCurrentSession==udsSession_Defalut)
//         {
//             g_ecuOnlineFlag = 1;
//             m_udsCurrentSession = udsSession_Extended;
//             if(posRspMsgIndication==0)
//             {
//                 responseData[0] = 0x50;
//                 responseData[1] = 0x03;
//                 responseData[2] = 0x00;
//                 responseData[3] = 0x32;
//                 responseData[4] = 0x01;
//                 responseData[5] = 0xf4;
//                 DiagnosticDataTransmit(m_udsParameter.physicalResponseId,responseData,6,functionalAddress);
//             }
//         }
//         else
//         {
//             negativeNum = 0x22;
//         }
// #endif
//     negativeNum = 0x7E;
//   }
//   else
//   {
//     negativeNum = 0x12; // sub function not supported
//   }
//   // ResetSessionTimer();
//   return negativeNum;
// }
#if (0)
static uint8_t BootService0x11Process(uint8_t *pTpData, uint32_t dataLength, uint8_t functionalAddress)
{
  uint8_t negativeNum;
  uint8_t subFunction;
  uint8_t posRspMsgIndication;
  uint8_t responseData[20];

  negativeNum = 0;
  subFunction = pTpData[1] & 0x7F;
  posRspMsgIndication = (pTpData[1] >> 7) & 0x01;

  if (0x01 == subFunction)
  {
    if (posRspMsgIndication == 0)
    {
      responseData[0] = 0x51;
      responseData[1] = 0x01;
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, functionalAddress);
      // alter,2023-1-3
      vTaskDelay(100);
      PeripheralHalMcuHardReset();
    }
  }
  else if (0x02 == subFunction)
  {
    if (posRspMsgIndication == 0)
    {
      responseData[0] = 0x51;
      responseData[1] = 0x02;
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, functionalAddress);
    }
  }
  else if (0x03 == subFunction)
  {
    if (posRspMsgIndication == 0)
    {
      responseData[0] = 0x51;
      responseData[1] = 0x03;
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, functionalAddress);
      vTaskDelay(100);
      PeripheralHalMcuHardReset();
    }
  }
  else
  {
    negativeNum = 0x12; // sub function not supported
  }

  return negativeNum;
}

static uint8_t BootService0x27Process(uint8_t *udsData, uint32_t udsLen, uint8_t functionAddressFlag)
{
  uint8_t negativeNum;
  uint8_t subFunction;
  uint8_t responseData[20];
  negativeNum = 0;
  subFunction = udsData[1];
#if (1)
  if (0x35 == subFunction) // FBL seed
  {
    UdsGetSeed(g_securitySeed, &g_securitySeedLength);
    UdsSeedtoKey(g_securitySeed, g_securitySeedLength, g_securityKey, &g_securityKeyLength, E_SECURITY_CALCULATE_LEVEL3);
    responseData[0] = 0x67;
    responseData[1] = 0x35;
    memcpy(&responseData[2], g_securitySeed, 4);
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 6, functionAddressFlag);
  }
  else if (0x36 == subFunction) // FBL key
  {
    if (ByteArrayCheckIsSame(g_securityKey, udsData + 2, 4) == 0) // key is valid
    {
      // m_currentSecurityLevel = udsSecurityLevel_Programing;
      responseData[0] = 0x67;
      responseData[1] = 0x36;
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, functionAddressFlag);
    }
    else // key invalid
    {
      negativeNum = 0x35; // invalid key
    }
  }
#endif
  else
  {
    // negativeNum = 0x12;//sub function not supported
    negativeNum = 0x7F;
  }

  // ResetSessionTimer();
  return negativeNum;
}

/**************************************************************************
routine control

**************************************************************************/
static uint8_t BootSerivice0x31ProcessCheckPrograming(uint8_t *pTpData, uint32_t dataLength, uint8_t functionalAddress)
{
  uint8_t negativeNum = 0;
  uint32_t checkSum;
  uint8_t responseData[20];
  responseData[0] = 0x71;
  responseData[1] = pTpData[1];
  responseData[2] = pTpData[2];
  responseData[3] = pTpData[3];

  if (g_currentSession == E_DEFAULT_SESSION)
  {
    negativeNum = 0x33;
    return negativeNum;
  }
  if (g_currentSecurityLevel == E_UDS_NONE_SECURITY_LEVEL)
  {
    negativeNum = 0x33;
    return negativeNum;
  }
  // alter,2023-1-3??עҢ:ϖԚ?ĳrecʇ?Ķ??󵄬˹Ҕ΄??֐?ģhecksum«ӫ?Ƌ�???ģhecksum«Ξ??ƥŤ
  checkSum = ((uint32_t)pTpData[4] << 24) + ((uint32_t)pTpData[5] << 16) + ((uint32_t)pTpData[6] << 8) + ((uint32_t)pTpData[7] << 0);
  TransmitNegative78ResponseData(pTpData[0]);

  // if (FlashCheckSumIsOk(checkSum) == 0)
  // {
  //     if ((g_flashAddress >= CFLASH_FLASH_DRIVER_START_ADDRESS) && (g_flashAddress <= CFLASH_FLASH_DRIVER_END_ADDRESS))
  //     {
  //         g_flashDriverDownLoadFlag = 1;
  //     }
  //     else if ((g_flashAddress >= CFLASH_BANKB_CM4_START_ADDRESS) && (g_flashAddress < CFLASH_BANKB_CM4_END_ADDRESS))
  //     {
  //         //FOTA_BankAPPIntegrityFlagUpdate(1);
  //         FlashHalOtaIntegrityFlagUpdate(1);

  //         if(FlashHalOtaGetActiveBank() == 1)
  //         {
  //             FlashHalOtaSetActiveBank(0);
  //         }
  //         else
  //         {
  //             FlashHalOtaSetActiveBank(1);
  //         }
  //     }
  //     responseData[4] = 0x00; //correct result
  // }
  // else
  // {
  //     responseData[4] = 0x01; //incorrect result
  // }
  DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 5, functionalAddress);

  return negativeNum;
}

static uint8_t BootSerivice0x31ProcessEraseMemory(uint8_t *pTpData, uint32_t dataLength, uint8_t functionalAddress)
{
  int8_t lengthFormat;
  int8_t addressFormat;
  int8_t i, index;
  uint32_t eraseAddress;
  uint32_t eraseSize;
  lengthFormat = (pTpData[4] >> 4) & 0x0F;
  addressFormat = pTpData[4] & 0x0F;
  eraseAddress = 0;
  eraseSize = 0;
  uint8_t negativeNum = 0;
  uint8_t responseData[20];
  responseData[0] = 0x71;
  responseData[1] = pTpData[1];
  responseData[2] = pTpData[2];
  responseData[3] = pTpData[3];

  if ((lengthFormat == 4) && (addressFormat == 4))
  {
    index = 5;
  }
  else
  {
    lengthFormat = 4;
    addressFormat = 4;
    index = 4;
  }

  if (g_currentSession == E_DEFAULT_SESSION)
  {
    negativeNum = 0x33;
    return negativeNum;
  }
  if (g_currentSecurityLevel == E_UDS_NONE_SECURITY_LEVEL)
  {
    negativeNum = 0x33;
    return negativeNum;
  }

  if ((addressFormat > 0) && (addressFormat <= 4))
  {
    if ((lengthFormat > 0) && (lengthFormat <= 4))
    {
      for (i = addressFormat - 1; i >= 0; i--)
      {
        eraseAddress |= (uint32_t)pTpData[index] << (i * 8);
        index++;
      }
      for (i = lengthFormat - 1; i >= 0; i--)
      {
        eraseSize |= (uint32_t)pTpData[index] << (i * 8);
        index++;
      }
      TransmitNegative78ResponseData(pTpData[0]);
      // erase flash function
      if (eraseAddress == FLASH_APP_BANKA_CM4_BASE_ADDRESS)
      {
        // FOTA_BankAPPIntegrityFlagUpdate(0);
        // FlashHalOtaIntegrityFlagUpdate(0);
      }

      if (g_flashDriverDownLoadFlag == 0)
      {
        // alter
        // MemMpu_Init();
      }
      // alter,2023-1-2

      if (FlashErase(eraseAddress, eraseSize) == 0)
      {
        // responseData[4] = 0x00;//correct result
        DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 4, functionalAddress);
        // OtaCrc32Init(&g_flashCrc32Value);
        Crc32Init(&g_crc32Object, 0x04C11DB7);
        g_flashCrc32Value = 0xFFFFFFFF;
      }
      else
      {
        // responseData[4] = 0x01;//incorrect result
        negativeNum = 0x72; //
      }
    }
    else
    {
      negativeNum = 0x13; // frame length error
    }
  }
  else
  {
    negativeNum = 0x13; // frame length error
  }
  return negativeNum;
}

// alter
static uint8_t BootSerivice0x31Process(uint8_t *pTpData, uint32_t dataLength, uint8_t functionalAddress)
{
  uint8_t negativeNum;
  uint8_t subFunction;
  uint8_t responseData[20];
  uint16_t did;
  negativeNum = 0;
  subFunction = pTpData[1];

  did = ((uint16_t)pTpData[2] << 8) + (uint16_t)pTpData[3];
  responseData[0] = 0x71;
  responseData[1] = pTpData[1];
  responseData[2] = pTpData[2];
  responseData[3] = pTpData[3];

  if (0x01 == subFunction) // startRoutine
  {
    if ((0x0202 == did) || (0xFD02 == did)) // check programing Interity
    {
      // uint32_t checkSum;
      //
      // if (g_currentSession == E_DEFAULT_SESSION)
      //{
      //     negativeNum = 0x33;
      //     return negativeNum;
      // }
      // if (g_currentSecurityLevel == E_UDS_NONE_SECURITY_LEVEL)
      //{
      //     negativeNum = 0x33;
      //     return negativeNum;
      // }
      ////alter,2023-1-3??עҢ:ϖԚ?ĳrecʇ?Ķ??󵄬˹Ҕ΄??֐?ģhecksum«ӫ?Ƌ�???ģhecksum«Ξ??ƥŤ
      // checkSum = ((uint32_t)pTpData[4] << 24) + ((uint32_t)pTpData[5] << 16) + ((uint32_t)pTpData[6] << 8) + ((uint32_t)pTpData[7] << 0);
      // TransmitNegative78ResponseData(pTpData[0]);
      //
      // if (FlashCheckSumIsOk(checkSum) == 0)
      //{
      //     if ((g_flashAddress >= CFLASH_FLASH_DRIVER_START_ADDRESS) && (g_flashAddress <= CFLASH_FLASH_DRIVER_END_ADDRESS))
      //     {
      //         g_flashDriverDownLoadFlag = 1;
      //     }
      //     else if ((g_flashAddress >= CFLASH_BANKB_CM4_START_ADDRESS) && (g_flashAddress < CFLASH_BANKB_CM4_END_ADDRESS))
      //     {
      //         //FOTA_BankAPPIntegrityFlagUpdate(1);
      //         FlashHalOtaIntegrityFlagUpdate(1);
      //
      //         if(FlashHalOtaGetActiveBank() == 1)
      //         {
      //             FlashHalOtaSetActiveBank(0);
      //         }
      //         else
      //         {
      //             FlashHalOtaSetActiveBank(1);
      //         }
      //     }
      //     responseData[4] = 0x00; //correct result
      // }
      // else
      //{
      //     responseData[4] = 0x01; //incorrect result
      // }
      // DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 5, functionalAddress);
      negativeNum = BootSerivice0x31ProcessCheckPrograming(pTpData, dataLength, functionalAddress);
      if (negativeNum != 0)
      {
        return negativeNum;
      }
    }
    else if (0xFD03 == did) // check programing preconditions
    {
      if (g_currentSession != E_EXTEND_SESSION)
      {
        negativeNum = 0x33;
        return negativeNum;
      }
      // g_flashCrc32Value = m_Crc32InitValue;
      // responseData[4] = 0x00;//correct result
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 4, functionalAddress);
    }
    else if (0xFD00 == did) // erase memory()
    {
      negativeNum = BootSerivice0x31ProcessEraseMemory(pTpData, dataLength, functionalAddress);
      if (negativeNum != 0)
      {
        return negativeNum;
      }
    }
    else if (0xFD01 == did) // check programing dependencies
    {
      if (g_currentSession == E_DEFAULT_SESSION)
      {
        negativeNum = 0x33;
        return negativeNum;
      }
      if (g_currentSecurityLevel == E_UDS_NONE_SECURITY_LEVEL)
      {
        negativeNum = 0x33;
        return negativeNum;
      }
      responseData[4] = 0x00; // correct result
      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 5, functionalAddress);
    }
    else
    {
      negativeNum = 0x31; // did not supported
    }
  }
  else
  {
    negativeNum = 0x12; // sub function not supported
  }
  g_ecuOnlineFlag = 1;
  return negativeNum;
}

/***************************************************************************
request download,կ?ϒǍ???ط?α4????ӕﶏ҇ϲECUςԘ?Ċ?�?*************************************************************************/
static uint8_t BootSerivice0x34Process(uint8_t *pTpData, uint32_t dataLength, uint8_t functionalAddress)
{
  uint8_t negativeNum;
  uint8_t responseData[20];
  int8_t lengthFormat;
  int8_t addressFormat;
  uint32_t address;
  uint32_t size;
  negativeNum = 0;

  lengthFormat = (pTpData[2] >> 4) & 0x0F;
  addressFormat = pTpData[2] & 0x0F;
  responseData[0] = 0x74;

  if (g_currentSession == E_DEFAULT_SESSION)
  {
    negativeNum = 0x33;
    return negativeNum;
  }

  if (g_currentSecurityLevel == E_UDS_NONE_SECURITY_LEVEL)
  {
    negativeNum = 0x33;
    return negativeNum;
  }

  if ((addressFormat > 0) && (addressFormat <= 4))
  {
    if ((lengthFormat > 0) && (lengthFormat <= 4))
    {
      int8_t i, index;
      uint16_t maxNumberOfBlockLength;
      address = 0;
      size = 0;
      index = 3;
      for (i = addressFormat - 1; i >= 0; i--)
      {
        address |= (uint32_t)pTpData[index] << (i * 8); // memoryAddress
        index++;
      }
      for (i = lengthFormat - 1; i >= 0; i--)
      {
        size |= (uint32_t)pTpData[index] << (i * 8); // memorySize
        index++;
      }
      g_blockSequenceCounter = 0x01;

      // if (Cy_Flashc_GetMainMap() == CY_FLASH_MAPPING_A)
      // {
      //     TBOX_PRINT("Updata APP2!----------------------------------->\r\n");
      // }
      // else
      // {
      //     TBOX_PRINT("Updata APP1!----------------------------------->\r\n");
      // }
      // 升级包为BANKA的APP地址，升级时需要写到另一个失活Bank区，需要偏移地址
      g_flashAddress = address + 0x2000000;
      g_flashSize = size; // memorySize
      g_requestDownLoadFlag = 1;

      responseData[1] = 0x20;
      maxNumberOfBlockLength = 0x0202; // to be changed
      responseData[2] = (maxNumberOfBlockLength >> 8) & 0xFF;
      responseData[3] = maxNumberOfBlockLength & 0xFF;

      DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 4, functionalAddress); //?????????
    }
    else
    {
      negativeNum = 0x13; // frame length error
    }
  }
  else
  {
    negativeNum = 0x13; // frame length error
  }
  g_ecuOnlineFlag = 1; // m_sessionTimerCount = 5000; ?UdsTimerCallBack?,????????m_sessionTimerCount--,5s?????????????
  return negativeNum;
}

/*****************************************************************************
transfer data
****************************************************************************/
static uint8_t BootSerivice0x36Process(uint8_t *pTpData, uint32_t dataLength, uint8_t functionalAddress) //?ъ????????ECU
{
  uint8_t negativeNum;
  uint8_t responseData[20];
  uint8_t blockSequenceCounter;
  negativeNum = 0;
  blockSequenceCounter = pTpData[1]; //???鼆??
  // alter,2023-1-2
  // uint32_t *pProgramData;

  if (g_currentSession == E_DEFAULT_SESSION)
  {
    negativeNum = 0x33;
    return negativeNum;
  }

  if (g_currentSecurityLevel == E_UDS_NONE_SECURITY_LEVEL)
  {
    negativeNum = 0x33;
    return negativeNum;
  }

  if (!g_requestDownLoadFlag) //?0x34?n?g_requestDownLoadFlag=1
  {
    negativeNum = 0x24; // requestSequenceError
    return negativeNum;
  }
  //     if (g_blockSequenceCounter == blockSequenceCounter) //?0x34?n??_blockSequenceCounter=1,??0x36?n??????,
  //     {
  //         //uint32_t address;
  //         g_blockSequenceCounter++; //?????0x36?n???lockSequenceCounter++,?????lockSequenceCounter??????
  //         //flash data function
  //         /*if(g_blockSequenceCounter==4)
  //         {
  //           address = 100;
  //         }*/
  //         //alter,??,?????????0x10010000~~0x10088000
  //         if ((g_flashAddress >= CFLASH_BANKB_CM4_START_ADDRESS) && (g_flashAddress < CFLASH_BANKB_CM4_END_ADDRESS)) //
  //         {
  //             TransmitNegative78ResponseData(pTpData[0]); //???
  //             //alter,???????资???,??codeflash???,
  //             //pProgramData = (uint32_t*)pTpData;
  //             pTpData = pTpData + 2;
  //             //pProgramData = (uint32_t *)pTpData;

  //             //FOTA_ProgramFlash(g_flashAddress, dataLength - 2, pProgramData); //program 512 byte
  //             FlashHalOtaFlashWrite(g_flashAddress,pTpData,dataLength - 2);
  //             //pProgramData = pProgramData + (dataLength - 2) / 4; //4x128

  //             g_flashAddress += dataLength - 2;
  //             responseData[0] = 0x76;
  //             responseData[1] = blockSequenceCounter;
  //             //responseData[2] = 0x00;//??
  //             DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, functionalAddress);
  //         }
  //         else if ((g_flashAddress >= CFLASH_FLASH_DRIVER_START_ADDRESS) && (g_flashAddress < CFLASH_FLASH_DRIVER_END_ADDRESS))
  //         {
  //             if (writeToRam(g_flashAddress, &pTpData[2], dataLength - 2) == 0)
  //             {
  // //              g_flashCrc32Value = crc32(g_flashCrc32Value,(uint8_t *)g_flashAddress,dataLength-2);
  //                 g_flashAddress += dataLength - 2;
  //                 responseData[0] = 0x76;
  //                 responseData[1] = blockSequenceCounter;

  //                 //responseData[2] = 0x00;//??
  //                 DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 2, functionalAddress);
  //             }
  //             else
  //             {
  //                 negativeNum = 0x22;
  //             }
  //         }
  //         else
  //         {
  //             negativeNum = 0x72; //generalProgrammingFailure
  //         }
  // }
  // else
  // {
  //     negativeNum = 0x73; //wrong Block Sequence Counter
  // }
  g_ecuOnlineFlag = 1;
  return negativeNum;
}

/****************************************************************************
request transfer exit
*****************************************************************************/
static uint8_t BootSerivice0x37Process(uint8_t *pTpData, uint32_t dataLength, uint8_t functionalAddress)
{
  uint8_t negativeNum;
  uint8_t responseData[20];
  negativeNum = 0;

  if (g_currentSession == E_DEFAULT_SESSION)
  {
    negativeNum = 0x33;
    return negativeNum;
  }

  if (g_currentSecurityLevel == E_UDS_NONE_SECURITY_LEVEL)
  {
    negativeNum = 0x33;
    return negativeNum;
  }

  if (!g_requestDownLoadFlag)
  {
    negativeNum = 0x24; // requestSequenceError
    return negativeNum;
  }
  g_requestDownLoadFlag = 0;
  responseData[0] = 0x77;
  DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, 1, functionalAddress);

  g_ecuOnlineFlag = 1;
  return negativeNum;
}

static uint8_t BootSerivice0x2EProcess(uint8_t *pTpData, uint32_t dataLength, uint8_t functionalAddress)
{
  uint8_t negativeNum;
  // uint8_t responseData[50];
  // uint16_t did;
  // uint8_t posRspMsgIndication;

  negativeNum = 0;

  if (g_currentSession != E_PROGROM_SESSION)
  {
    negativeNum = 0x33;
    return negativeNum;
  }

  if (g_currentSecurityLevel != E_UDS_SECURITY_LEVEL3)
  {
    negativeNum = 0x33;
    return negativeNum;
  }
  // did = ((uint16_t)pTpData[1] << 8) + (uint16_t)pTpData[2];
  negativeNum = 0x31;
  g_ecuOnlineFlag = 1;
  return negativeNum;
}

static uint8_t BootSerivice0x22Process(uint8_t *pTpData, uint32_t dataLength, uint8_t functionalAddress)
{
  uint8_t negativeNum;
  uint8_t responseData[64];
  uint16_t did;
  uint32_t length = 0;

  did = (pTpData[1] << 8) + pTpData[2];
  negativeNum = 0;
  if (0xF1C1 == did) // app software version
  {
    Service22ReadAppSoftwareVersion(&responseData[3], (uint16_t *)&length);
  }
  else
  {
    negativeNum = 0x31;
  }
  if (0 == negativeNum)
  {
    responseData[0] = 0x62;
    responseData[1] = pTpData[1];
    responseData[2] = pTpData[2];
    DiagnosticDataTransmit(g_tpHandle, g_physicalTransmitCanId, responseData, length + 3, functionalAddress);
  }

  g_ecuOnlineFlag = 1;
  return negativeNum;
}

void SetEcuOnlineFlag(void)
{
    g_ecuOnlineFlag = 1;
}

static void UdsServiceProcess(uint8_t *pTpData, uint32_t tpDataLength, uint8_t FunctionalAddress)
{
  uint8_t serviceId;
  uint8_t negativeNum;
  serviceId = pTpData[0];
  negativeNum = 0;
  if (0x10 == serviceId) // 0x10,???????(????˙???0km?10 02,??????????А)
  {
    negativeNum = BootSerivice0x10Process(pTpData, tpDataLength, FunctionalAddress); // BootSerivice0x10Process(pTpData, tpDataLength, FunctionalAddress);
  }
  else if (0x11 == serviceId) // ECU RESET;11 01??hardReset: ????????????????1 02??keyOffOnReset?? ?????????????? 11 03??softReset
  {
    negativeNum = BootService0x11Process(pTpData, tpDataLength, FunctionalAddress); //
  }
  else if (0x22 == serviceId) //????
  {
    negativeNum = BootSerivice0x22Process(pTpData, tpDataLength, FunctionalAddress);
  }
  else if (0x27 == serviceId) //??????
  {
    negativeNum = BootService0x27Process(pTpData, tpDataLength, FunctionalAddress);
  }
  else if (0x2E == serviceId) //????
  {
    negativeNum = BootSerivice0x2EProcess(pTpData, tpDataLength, FunctionalAddress);
  }
  else if (0x31 == serviceId) // 31??α???CUě??ē?Щ???е?????lash?????
  {
    negativeNum = BootSerivice0x31Process(pTpData, tpDataLength, FunctionalAddress);
  }
  else if (0x34 == serviceId) // RequestDownload,??g_flashAddress????31??А
  {
    negativeNum = BootSerivice0x34Process(pTpData, tpDataLength, FunctionalAddress);
  }
  else if (0x36 == serviceId) // TransferData
  {
    negativeNum = BootSerivice0x36Process(pTpData, tpDataLength, FunctionalAddress);
  }
  else if (0x37 == serviceId) // RequestTransferExit
  {
    negativeNum = BootSerivice0x37Process(pTpData, tpDataLength, FunctionalAddress);
  }
  else if (0x3E == serviceId) //????α
  {
    negativeNum = Service0x3EProcess(pTpData, tpDataLength, FunctionalAddress);
  }
  else // unsupported service
  {
    negativeNum = ServiceNoDefinitionProcess(pTpData, tpDataLength, FunctionalAddress);
  }
  if (negativeNum != 0)
  {
    // transmit negative response
    NegativeResponseProcess(negativeNum, serviceId, FunctionalAddress);
  }
}
#endif
