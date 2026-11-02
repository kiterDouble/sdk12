#include "udsDidFunction.h"
#include <string.h>
#include <math.h>
#include "ParameterStoreManageApp.h"

#include "parameterSyncSdk.h"
#include "powerManageSdk.h"
#include "taskDiagnostic.h"
#include "batterySdk.h"
#include "taskDtcProcess.h"
#include "timeSyncSdk.h"
// #include "mcu_cpu_sync/mcu_control_cpu_interface.h
#include "peripheralHal.h"
#include "flashHal.h"
#include "projectConfigure.h"
#include "stateSyncSdk.h"
#include "mpuPowerSyncSdk.h"
#include "secocSdk.h"
#include "remoteDiagnosticSdk.h"
#include "eolTestSyncWithCpu.h"
#include "mcuMpuSyncTask.h"
#include "canParseSdk.h"
#include "vehicleSignalApp.h"
#include "vehicleSignalApp.h"
#include "taskPowerManage.h"
#include "powerManageHal.h"
#include "logHal.h"
#include "ecallHal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mpuHal.h"
#include "remoteControl.h"
#include "crc8_16_32.h"

#if 0
static uint8_t g_didF1A2[19] = {"P01       -V6.02.00"}; // CAN矩阵
static uint8_t g_didF1B3[9] = {"CDONO-77E"};                        // 节点地址
static uint8_t g_didF1B4[31] = {"RB_AUTOCDONO-77ES026A09XKM14-FW"}; // CompID
static uint8_t g_didF1B6[8] = {"00000000"};                         // 工具版本
static uint8_t g_didF1BC[10] = {"P01-230412"};                      // 软件信息
static uint8_t g_didF1BD[10] = {"P01-230301"};                      // 硬件信息
static uint8_t g_didF1C8[5] = {"V0001"};                            // SecOC版本
static uint8_t g_didF1C0[15] = {"FFFFFFFFFFFFFFF"}; // 校准软件版本
// static uint8_t g_didF1C1[15] = {"FFFFFFFFFFFFFFF"};//
static uint8_t g_didF1B5[10] = {"          "};                      //
static uint8_t g_didF1B9[31] = {"RB_AUTOCDONO-77ES026C09XKM14-MW"}; //
#endif

// static uint8_t g_didF18C[14] = {"12345678900000"};                      // 0xF18C_sq  控制器生产序列号,零部件ECU的生产序列信息
static uint8_t g_swWrittenSupported = 1;
static uint8_t g_conDataVersion = 0; //配置字版本, 该内容由主机厂EOL刷写

static uint8_t g_vehicleTypeConfigure[12];
// static uint8_t m_ApplicationStop = 0x00;
static uint8_t g_31ReportToDcanFlag = 0;
static uint8_t g_31ReportToDcanValue = 0;
static uint8_t g_2EReportToDcanVaule = 0;
const char g_ucuType[] = "SGMW-UCU-05";
static uint8_t g_vin[17];
static uint8_t g_eskKeyValid = 0;

static void CheckVinWriteStatus(void);

#if 0
static uint8_t HexToChar(uint8_t hexValue)
{
  uint8_t value;
  uint8_t result;
  value = hexValue & 0x0F;
  if (value < 10)
  {
    result = value + '0';
  }
  else
  {
    result = (value - 10) + 'a';
  }
  return result;
}
#endif
#if 1 //sq
// 编程允许的电压范围（示例：9.0V ~ 16.0V）
#define MIN_PROG_VOLTAGE 9000    // 单位：mV
#define MAX_PROG_VOLTAGE 16000   // 单位：mV

#define CRC32_POLYNOMIAL        0x04C11DB7  // OEM指定的多项式
// #define FLASH_APP_BASE_ADDRESS  0x08000000  // Flash应用程序起始地址（根据实际硬件修改）
// #define FLASH_APP_SIZE          0x10000     // 应用程序大小（16KB，根据实际修改）
// #define CRC32_INIT_VALUE        0xFFFFFFFF  // CRC32初始值

#endif

/*******************************************************************************
return
0:valid
*******************************************************************************/
static int16_t CheckEskValid(uint8_t *pEsk)
{
  uint16_t i;
  uint8_t validFlag;

  validFlag = 0;
  if (0xFF == pEsk[0])
  {
    for (i = 0; i < 16; i++)
    {
      if (0xFF != pEsk[i])
      {
        validFlag = 1;
        break;
      }
    }
    if (0 == validFlag)
    {
      return 1;
    }
  }

  validFlag = 0;
  if (0x00 == pEsk[0])
  {
    for (i = 0; i < 16; i++)
    {
      if (0x00 != pEsk[i])
      {
        validFlag = 1;
        break;
      }
    }
    if (0 == validFlag)
    {
      return 1;
    }
  }
  return 0;
}
#if 0
static int16_t HexArrayToCharArray(uint8_t *pHexInput, uint16_t hexLenthIn, uint8_t *pCharOut, uint16_t *pCharLengthOut)
{
  int16_t ret;
  uint16_t i;

  ret = 0;
  if (pHexInput == NULL)
  {
    return -1;
  }
  if (hexLenthIn == 0)
  {
    return -1;
  }
  if (pCharOut == NULL)
  {
    return -1;
  }
  if (pCharLengthOut == NULL)
  {
    return -1;
  }

  for (i = 0; i < hexLenthIn; i++)
  {
    pCharOut[i * 2] = HexToChar(pHexInput[i] >> 4); // H
    pCharOut[i * 2 + 1] = HexToChar(pHexInput[i]);  // L
  }
  *pCharLengthOut = hexLenthIn * 2;
  return ret;
}

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

static int16_t CharArrayToHexArray(uint8_t *pCharIn, uint16_t charLengthIn, uint8_t *pHexOut, uint16_t *pHexLenthOut)
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
#endif

#if 1 //sq
//22 read
static int16_t CheckVinIsAsciiZero(uint8_t *pVin)
{
  uint8_t i = 0;
  int16_t ret = 0;
  for (i = 0; i < 17; i++)
  {
    if (pVin[i] != 0x30)
    {
      ret = -1;
      break;
    }
  }
  return ret;
}

int16_t Service22ReadFingerprintEthernetF10A(uint8_t *pData, uint16_t *pLength) // 指纹数据
{
  uint32_t length = 0;
  int16_t ret = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_FINGERPRINT_ETHERNET_F10A, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (length > 10)
  {
    length = 10;
  }
  *pLength = (uint16_t)length;
  return 0;
}

int16_t Service22ReadEcuPartNumberF187(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetGacEcuPartNumber_F187(pData, pLength);
  return 0;
}

int16_t Service22ReadSupplierIdF18A(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetSupplierId_F18A(pData, pLength);
  return 0;  
}

int16_t Service22ReadManufactureDateF18B(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 0;
  int16_t ret = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_MANUFACTURE_DATE, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (length > 4)
  {
    length = 4;
  }
  *pLength = (uint16_t)length;
  return 0;

}

int16_t Service22ReadSerialNumberF18C(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 0;
  int16_t ret = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_SERIAL_NUMBER, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (length > 14)
  {
    length = 14;
  }
  *pLength = (uint16_t)length;
  return 0;
}

int16_t Service22ReadVINF190(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 0;
  int16_t ret = 0;
  uint8_t vinTem[40] = {0};
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_VIN, vinTem, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (length > 17)
  {
    length = 17;
  }
  if (CheckVinIsAsciiZero(vinTem) != 0)
  {
    memcpy(pData, vinTem, length);
    *pLength = (uint16_t)length;
    return 0;
  }
#if (0)
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_VIN_hex, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
#endif
  *pLength = (uint16_t)length;

  return 0;
}

int16_t Service22ReadSupplierECUHardwareVersionF193(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetSupplierHdVersion_F193(pData, pLength);
  return 0;
}

int16_t Service22ReadSupplierECUSoftwareVersionF195(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetInternalSwVersion_1214(pData, pLength);
  return 0;
}

int16_t Service22ReadVehicleManufactureDateF1B7(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 0;
  int16_t ret = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_MANUFACTURE_DATE_F1B7, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (length > 7)
  {
    length = 7;
  }
  *pLength = (uint16_t)length;
  return 0;
}

int16_t Service22ReadSWWrittenStatusF200(uint8_t *pData, uint16_t *pLength)
{
  if (g_swWrittenSupported == 1)
  {
    pData[0] = 0x01;
  }
  else
  {
    pData[0] = 0x00;
  }

  *pLength = 1;

  return 0;
}

int16_t Service22ReadConfigurationDataVersionF1B0(uint8_t *pData, uint16_t *pLength)
{
  if (g_conDataVersion == 1)
  {
    pData[0] = 0x01;
  }
  else
  {
    pData[0] = 0x00;
  }

  *pLength = 1;

  return 0;
}

int16_t Service22ReadPINF1B1(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 0;
  int16_t ret = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_PIN_F1B1, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (length > 16)
  {
    length = 16;
  }
  *pLength = (uint16_t)length;
  return 0;
}

int16_t Service22ReadChipIdF1B2(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetChipID_F1B2(pData, pLength);
  return 0;
}

int16_t Service22ReadPublicKeyF1B3(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetPublicKey_F1B3(pData, pLength);
  return 0;
}

int16_t Service22ReadTerminalActiveStateF300(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetTerminalActiveState_F300(pData, pLength);
  return 0;
}

int16_t ServiceReadManufactoryMode(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 0;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_MANUFACTORY_MODE, pData, &length);
  if (ret != 0 || length == 0)
  {
    pData[0] = 0xFF;
  }

  *pLength = 1;
  return 0;
}

//EOL DID
int16_t Service22ReadLowVoltBatteryTypeF201(uint8_t *pData, uint16_t *pLength) // 低压电池类型
{
  uint32_t length = 0;
  int16_t ret = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOWVOLTBATTERY_TYPE_F201, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (length > 1)
  {
    length = 1;
  }
  *pLength = (uint16_t)length;
  return 0;
}

int16_t Service22ReadVoltPlatformF202(uint8_t *pData, uint16_t *pLength) // 电压平台
{
  uint32_t length = 0;
  int16_t ret = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_VOLT_PLATFORM_F202, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (length > 1)
  {
    length = 1;
  }
  *pLength = (uint16_t)length;
  return 0;
}

int16_t Service22ReadEthRoleF203(uint8_t *pData, uint16_t *pLength) // 以太网主从节点
{
  uint32_t length = 0;
  int16_t ret = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ETH_ROLE_F203, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (length > 1)
  {
    length = 1;
  }
  *pLength = (uint16_t)length;
  return 0;
}

//2E write

int16_t Service2EWriteVINF190(uint8_t *pData, uint16_t dataLength)
{
  int ret = 0;
  if (dataLength != 17)
  {
    return 0x13;
  } 
 
  ret = ParameterSyncSdkGetFromCpuIsFinished();
  if (ret < 0)      
  {
    return -1;
  }

  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_VIN, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }
  if (storeResult == 0)
  {
    ParameterSyncSdkSetToCpu(E_ParamId_VIN, pData, dataLength);
  }

  return 0;
}

int16_t Service2EWriteFingerprintEthernetF10A(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret = 0;

  // if(GetSyncParameterFromCpuIsFinished()!=0)
  // {
  //   return -1;
  // }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_FINGERPRINT_ETHERNET_F10A, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteManufactureDateF1B7(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret = 0;
  uint8_t power_type;     // Byte0:车辆动力系统类别
  uint8_t product_attr;   // Byte1:生产制造属性
  uint8_t car_model;      // Byte2:车型类型代码
  uint8_t manu_year_high;  // Byte3 年份高位 BCD
  uint8_t manu_year_low;   // Byte4 年份低位 BCD
  uint8_t manu_month;      // Byte5 月份 BCD
  uint8_t manu_day;        // Byte6 日期 BCD
  
  if (pData == NULL || dataLength != 7)
  {
    return 0x13;
  }  

  power_type    = pData[0];
  product_attr  = pData[1];
  car_model     = pData[2];
  manu_year_high = pData[3];
  manu_year_low  = pData[4];
  manu_month     = pData[5];
  manu_day       = pData[6];
  
  if( (power_type > 0x02) || (product_attr > 0x02) || (car_model > 0x02) )  //有效值0x00/0x01/0x02
  {
    return 0x31;
  }

  if((manu_year_high > 0x99) || (manu_year_low > 0x99))
  {
    return 0x31; // 年份非法，返回0x31
  }
 
  if((manu_month < 0x01) || (manu_month > 0x12))
  {
    return 0x31; // 月份非法，返回0x31
  }

  if((manu_day < 0x01) || (manu_day > 0x31))
  {
    return 0x31; // 日期非法，返回0x31
  }

  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_MANUFACTURE_DATE_F1B7, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  
  return 0;  
}

int16_t Service2EWritePINF1B1(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret = 0;

  if (pData == NULL || dataLength != 16)
    {
        return 0x13; 
  }

  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PIN_F1B1, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteLowVoltBatteryTypeF201(uint8_t *pData, uint16_t dataLength)
{
    int16_t ret = 0;
    if (pData == NULL || dataLength != 1)
    {
        return 0x13; 
    }
    // 仅允许0x00/0x01/0x02
    if (*pData > 0x02)
    {
        return 0x31;
    }
   
    ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_LOWVOLTBATTERY_TYPE_F201, pData, dataLength);
    return (ret == 0) ? 0 : -1;
}

int16_t Service2EWriteVoltPlatformF202(uint8_t *pData, uint16_t dataLength)
{
    int16_t ret = 0;
    if (pData == NULL || dataLength != 1)
    {
        return 0x13; 
    }
    // 仅允许0x00/0x01
    if (*pData > 0x01)
    {
        return 0x31;
    }
   
    ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_VOLT_PLATFORM_F202, pData, dataLength);
    return (ret == 0) ? 0 : -1;
}

int16_t Service2EWriteEthRoleF203(uint8_t *pData, uint16_t dataLength)
{
    int16_t ret = 0;
    if (pData == NULL || dataLength != 1)
    {
        return 0x13; 
    }
    // 仅允许0x00/0x01
    if (*pData > 0x01)
    {
        return 0x31;
    }
   
    ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ETH_ROLE_F203, pData, dataLength);
    return (ret == 0) ? 0 : -1;
}

int16_t Service2EWriteSK(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret = 0;

  /*if(GetSyncParameterFromCpuIsFinished()!=0)
  {
    return -1;
  }  */
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_SK, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

//写入工厂模式状态值(0x01：Factory mode; 0x02：Normal mode) 
int16_t ServiceWriteManufactoryMode(uint8_t *pData, uint16_t dataLength)
{ 
  if (dataLength != 1)
  {
    return 0x13;
  }

  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_MANUFACTORY_MODE, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }

  uint8_t newMode = pData[0];
  if (newMode >= 0x10) // 工厂模式激活
  {
    // 进入工厂模式，禁用远程诊断
    RemoteDiagnosticSdkDisable();
  }
  else // 工厂模式未激活 (0x00 ~ 0x0F)
  {
    // 退出工厂模式，恢复远程诊断
    RemoteDiagnosticSdkRecover();
  }

  return 0; // 返回0表示成功
}

//获取当前工厂模式状态值 用于27服务
uint8_t UdsDidGetManufactoryMode(void)
{
  uint8_t mode = 0xFF;
  uint32_t length = 0;

  if (WorkFlashVehicleInforRead(E_PARAMETER_INFO_MANUFACTORY_MODE, &mode, &length) != 0 || length == 0)
  {
    mode = 0xFF;
  }
  return mode;
}

bool UdsCheckSecurityAccessValid(uint8_t requiredLevel)
{
  uint8_t currentLevel = 0;
  uint8_t factoryMode = UdsDidGetManufactoryMode();

  // 工厂模式自动放行
  if (factoryMode >= 0x10)
  {
    return true;
  }

  // 普通安全等级判断
  switch (requiredLevel)
  {
  case E_UDS_NONE_SECURITY_LEVEL:
    return true; // 不需要安全验证

  case E_UDS_SECURITY_LEVEL1:
    return (currentLevel == E_UDS_SECURITY_LEVEL1);

  // case E_UDS_SECURITY_LEVEL2_STD:
  //   return (currentLevel == E_UDS_SECURITY_LEVEL2_STD);

  case E_UDS_SECURITY_LEVEL_REPROG:
    return (currentLevel == E_UDS_SECURITY_LEVEL_REPROG);

  default:
    return false;
  }
}

//EOL Write
int16_t ServiceWriteManufactureDateF18B(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;  
  uint8_t manu_year_high;  // Byte0 年份高位 BCD
  uint8_t manu_year_low;   // Byte1 年份低位 BCD
  uint8_t manu_month;      // Byte2 月份 BCD
  uint8_t manu_day;        // Byte3 日期 BCD
  
  if (pData == NULL || dataLength != 4)
  {
    return 0x13;
  }

  manu_year_high = pData[0];
  manu_year_low  = pData[1];
  manu_month     = pData[2];
  manu_day       = pData[3];
  
  if((manu_year_high > 0x99) || (manu_year_low > 0x99))
  {
    return 0x31; // 年份非法，返回0x31
  }
 
  if((manu_month < 0x01) || (manu_month > 0x12))
  {
    return 0x31; // 月份非法，返回0x31
  }

  if((manu_day < 0x01) || (manu_day > 0x31))
  {
    return 0x31; // 日期非法，返回0x31
  }

  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_MANUFACTURE_DATE, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  
  return 0;  
}

int16_t ServiceWriteSerialNumberF18C(uint8_t *pData, uint16_t dataLength)
{
  int16_t storeResult;  
  if (pData == NULL || dataLength != 14)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_SERIAL_NUMBER, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }
  if (storeResult == 0)
  {
    ParameterSyncSdkSetToCpu(E_ParamId_SN, pData, dataLength);
  }

  return 0;
}

#else
int16_t Service22ReadPartNumber(uint8_t *pData, uint16_t *pLength)
{
  // int16_t ret = 0;
  uint32_t u32Length;
  // uint32_t u16Length;
  u32Length = 0;
  // u16Length = 0;
#if (0)
  uint8_t asc[64];
  uint32_t ascLength;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_PART_NUMBER, asc, &ascLength);
  if (ret != 0)
  {
    return -1;
  }
  ret = CharArrayToHexArray(asc, ascLength, pData, &u32Length);
  *pLength = length;
  if (ret != 0)
  {
    return -1;
  }
#else
  ProjectConfigGetPartNumber(pData, &u32Length);
  *pLength = u32Length;
#endif
  return 0;
}

// int16_t Service22ReadSupplierId(uint8_t *pData, uint16_t *pLength)
// {
//   memcpy(pData, g_didF18A, sizeof(g_didF18A));
//   *pLength = sizeof(g_didF18A);

//   return 0;
//   //   uint32_t length;
//   //   int16_t ret;

//   //   length = 0;
//   // #if(1)
//   //   ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_SYSTEM_SUPPLIER_ID,pData,&length);
//   //   length = 7;
//   // #else
//   //   ret = GlobalGetSupplierIdentifier(pData,&length);
//   // #endif
//   //   if(ret != 0)
//   //   {
//   //     return -1;
//   //   }
//   //   *pLength = length;
//   //   return 0;
// }

// 获取当前工厂模式状态值 用于27服务
uint8_t UdsDidGetManufactoryMode(void)
{
  uint8_t mode = 0xFF;
  uint32_t length = 0;

  if (WorkFlashVehicleInforRead(E_PARAMETER_INFO_MANUFACTORY_MODE, &mode, &length) != 0 || length == 0)
  {
    mode = 0xFF;
  }
  return mode;
}

bool UdsCheckSecurityAccessValid(uint8_t requiredLevel)
{
  uint8_t currentLevel = 0;
  uint8_t factoryMode = UdsDidGetManufactoryMode();

  // 工厂模式自动放行
  if (factoryMode >= 0x10)
  {
    return true;
  }

  // 普通安全等级判断
  switch (requiredLevel)
  {
  case E_UDS_NONE_SECURITY_LEVEL:
    return true; // 不需要安全验证

  case E_UDS_SECURITY_LEVEL1:
    return (currentLevel == E_UDS_SECURITY_LEVEL1);

  case E_UDS_SECURITY_LEVEL2_STD:
    return (currentLevel == E_UDS_SECURITY_LEVEL2_STD);

  case E_UDS_SECURITY_LEVEL_REPROG:
    return (currentLevel == E_UDS_SECURITY_LEVEL_REPROG);

  default:
    return false;
  }
}

int16_t Service22ReadHardwareVersion(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_HARDWARE_VERSION, pData, &length);
  // ret = GlobalGetHardwareVersion(pData,&length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = 4;
  return 0;
}

int16_t Service22ReadSupplierSoftwareNumber(uint8_t *pData, uint16_t *pLength)
{
  uint16_t length;

  length = 0;
  ProjectConfigGetSoftwareNumber(pData, &length);
  *pLength = 10;
  return 0;
}

int16_t Service22ReadSoftwareVersion(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  // int16_t ret;
  // ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_SOFTWARE_VERSION,pData,&length);
  ProjectConfigGetSoftwareVersion(pData, &length);

  *pLength = length;
  return 0;
}

int16_t Service22ReadBootSoftwareVersion(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 2;
  int16_t ret;
  // ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_SOFTWARE_VERSION,pData,&length);
#if (1)
  ret = ProjectConfigGetPartNameBootSoftwareVersion(pData, &length);
  if (ret != 0)
  {
    return -1;
  }
#endif
  *pLength = length;
  return 0;
}

int16_t Service22ReadAppSoftwareVersion(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;

  ret = ProjectConfigGetCustomVersion(pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadEcuMask(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;

  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_SAP_ECU_MASK, pData, &length);
  if (ret != 0)
  {
    return -1;
  }

  *pLength = length;
  return 0;
}

int16_t Service22ReadManufactureDate(uint8_t *pData, uint16_t *pLength)
{
  uint16_t length;
  int16_t ret;
  uint32_t ascLength;
  uint8_t asc[64];
  // ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_MANUFACTURE_DATE,pData,&length);
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_MANUFACTURE_DATE, asc, &ascLength);
  if (ret != 0)
  {
    return -1;
  }

  ret = CharArrayToHexArray(asc, ascLength, pData, &length);
  *pLength = 4;
  return 0;
}

int16_t Service22ReadPartName(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_PART_NAME, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

static int16_t CheckVinIsAsciiZero(uint8_t *pVin)
{
  uint8_t i;
  int16_t ret = 0;
  for (i = 0; i < 17; i++)
  {
    if (pVin[i] != 0x30)
    {
      ret = -1;
      break;
    }
  }
  return ret;
}

int16_t Service22ReadVIN(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  uint8_t vinTem[40];
  // uint8_t i;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_VIN, vinTem, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (CheckVinIsAsciiZero(vinTem) != 0)
  {
    memcpy(pData, vinTem, 17);
    *pLength = 17;
    return 0;
  }
#if (0)
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_VIN_hex, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
#endif
  *pLength = 17;

  return 0;
}

// int16_t Service22ReadICCID(uint8_t *pData, uint16_t *pLength)
// {
//   uint32_t length;
//   int16_t ret;
//   if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
//   {
//     return -1;
//   }
//   ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ICCID, pData, &length);
//   if (ret != 0)
//   {
//     return -1;
//   }
//   *pLength = length;
//   return 0;
// }

// int16_t Service22ReadIMEI(uint8_t *pData, uint16_t *pLength)
// {
//   uint32_t length;
//   int16_t ret;
//   ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_IMEI, pData, &length);
//   if (ret != 0)
//   {
//     return -1;
//   }
//   *pLength = length;
//   return 0;
// }

// int16_t Service22ReadIMSI(uint8_t *pData, uint16_t *pLength)
// {
//   uint32_t length;
//   int16_t ret;
//   ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_IMSI, pData, &length);
//   if (ret != 0)
//   {
//     return -1;
//   }
//   *pLength = length;
//   return 0;
// }

int16_t Service22ReadTboxCallNumber(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;

  memset(pData, 0x00, 20);
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_TBOX_CALL_NUMBER, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = 20;
  return 0;
}

int16_t Service22ReadRegisterFlag(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  uint8_t data[10];
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_TBOX_REGISTER_FLAG, data, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (length < 1)
  {
    return -1;
  }
  pData[0] = data[length - 1];
  *pLength = 1;
  return 0;
}

int16_t Service22ReadSecurityVersion(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_SECURITY_VERSION, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadFingerPrint(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret;
#if (0)
  ret = WorkFlashReadAllFingerPrint(pData, pLength);
#else
  ret = 0;
#endif
  return ret;
}

int16_t Service22ReadKL30Voltage(uint8_t *pData, uint16_t *pLength)
{
  uint32_t voltage;
  // ReadPowerVoltage(&voltage);
  PeripheralHalAdGet(AD_CHANNEL_KL30, &voltage);
  pData[0] = voltage / 100;

  *pLength = 1;
  return 0;
}

int16_t Service22ReadKL30ConnectStatus(uint8_t *pData, uint16_t *pLength)
{
  uint32_t voltage;
#if (1)
  // ReadPowerVoltage(&voltage);
  PeripheralHalAdGet(AD_CHANNEL_KL30, &voltage);
  if (voltage < 6000)
  {
    pData[0] = 0x01;
  }
  else
  {
    pData[0] = 0x00;
  }
#else
  pData[0] = 0x00;
#endif
  *pLength = 1;
  return 0;
}

int16_t Service22ReadBatteryStatus(uint8_t *pData, uint16_t *pLength)
{
  uint32_t voltage;
  int32_t temperature;
  BatterySdkGetVoltage(&voltage);
  BatterySdkGetTemperature(&temperature);

  pData[0] = 0x00;
  pData[1] = 0x00;
  pData[2] = (voltage >> 8) & 0xFF;
  pData[3] = (voltage) & 0xFF;
  // temperature = -1000;
  temperature /= 10; // resolution changed from 0.01 to 0.1
  // temperature += 1000;//offset  1000(100??)
  pData[4] = (temperature >> 24) & 0xFF;
  pData[5] = (temperature >> 16) & 0xFF;
  pData[6] = (temperature >> 8) & 0xFF;
  pData[7] = (temperature) & 0xFF;
  *pLength = 8;
  return 0;
}

int16_t Service22ReadGNSS(uint8_t *pData, uint16_t *pLength)
{
  *pLength = 10;

  int16_t ret;

  ret = 0; // test
#if (0)
  ret = McuControlGetCpuInfo(SYNC_CPU_GET_INFO_ITEM_GNSS, pData, pLength);
#endif

  return ret;
}

int16_t Service22ReadGNSSSLocationStatus(uint8_t *pData, uint16_t *pLength)
{
  uint8_t data[20] = {0};
  // uint16_t length;

  *pLength = 1;

  int16_t ret;

  ret = 0; // test
  // ret = McuControlGetCpuInfo(SYNC_CPU_GET_INFO_ITEM_GNSS,data,&length);
  LocationInfoSync_t localtionInfo;
  if (StateSyncGetLocationInfo(&localtionInfo) == 0)
  {
    // Bit0-bit5: GNSS定位状�?0: 未定�?1: 单点定位 2: 浮点定位 3: RTK定位
    data[0] = localtionInfo.locationState & 0x1F;
  }
  else
  {
    ret = -1;
  }

  // 00:not used,01:fix not available,02:fix in 2D mode,03:fix in 3d mode
  *pData = data[0];
  return ret;
}

// #include<stdio.h>
int ChangetoSexagesimal(float Num, int *a, int *b, float *c)
{

  // float e;
  int Hour, Minute;
  float Second;

  Hour = (int)(Num);                                   // �?
  Minute = (int)((Num - Hour) * 60);                   // �?
  Second = (float)(((Num - Hour) * 60 - Minute) * 60); // �?
  /*
      e = ((Num-Hour)*60 - Minute) *60 - Second;      //四舍五入
      if(5 <=(int)(e *10) )
      {
          //Second = Second + 1;
          Second = Second + e;
      }
      if(60 == Second)
      {
          Second = 0;
          Minute = Minute + 1;
      }
      if(60 == Minute)
      {
          Minute = 0;
          Hour = Hour + 1;
      }
  */
  *a = Hour;
  *b = Minute;
  *c = Second;

  return 1;
}

int16_t Service22ReadGNSS_LatitudeLongitude(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret;

  *pLength = 10;
  LocationInfoSync_t localtionInfo;
  float latitude = 0;
  float longitude = 0;

  int32_t latitudeHour = 0;
  int32_t latitudeMinute = 0;
  float latitudeSecond = 0;

  int32_t longitudeHour = 0;
  int32_t longitudeMinute = 0;
  float longitudeSecond = 0;

  ret = 0; // test
#if (1)
  // GetGnssLatitudeLongitude(pData);
  if (StateSyncGetLocationInfo(&localtionInfo) == 0)
  {
    latitude = (localtionInfo.latitude * pow(10, -7));
    longitude = (localtionInfo.longitude * pow(10, -7));
    ChangetoSexagesimal(latitude, &latitudeHour, &latitudeMinute, &latitudeSecond);
    ChangetoSexagesimal(longitude, &longitudeHour, &longitudeMinute, &longitudeSecond);
    pData[0] = latitudeHour;
    pData[1] = latitudeMinute;
    pData[2] = (((uint16_t)(latitudeSecond * 100)) >> 8) & 0xFF;
    pData[3] = ((uint16_t)(latitudeSecond * 100)) & 0xFF;
    if (localtionInfo.locationState & 0x80 == 0)
    {
      pData[4] = 'N';
    }
    else
    {
      pData[4] = 'S';
    }
    pData[5] = longitudeHour;
    pData[6] = longitudeMinute;
    pData[7] = (((uint16_t)(longitudeSecond * 100)) >> 8) & 0xFF;
    pData[8] = ((uint16_t)(longitudeSecond * 100)) & 0xFF;
    if (localtionInfo.locationState & 0x40 == 0)
    {
      pData[9] = 'E';
    }
    else
    {
      pData[9] = 'W';
    }
  }
  else
  {
    ret = -1;
  }
#endif
  //*pData = data[0];

  return ret;
}

int16_t Service22ReadGNSSStatus(uint8_t *pData, uint16_t *pLength)
{
  uint8_t data[20] = {0};
  // uint16_t length;

  *pLength = 1;
  LocationInfoSync_t localtionInfo;
  int16_t ret;

  ret = 0; // test
  /// ret = McuControlGetCpuInfo(SYNC_CPU_GET_INFO_ITEM_GNSS,data,&length);
#if (1)
  if (StateSyncGetLocationInfo(&localtionInfo) == 0)
  {
    data[0] = localtionInfo.moduleState;
  }
  else
  {
    ret = -1;
  }
  // GetGnssStatus();
#else
  data[0] = 0;
#endif
  // 00:not used,01:fix not available,02:fix in 2D mode,03:fix in 3d mode
  if (data[0] == 0)
  {
    *pData = 0x00;
  }
  else if (data[0] == 1)
  {
    *pData = 0x01;
  }
  else
  {
    *pData = 0x01;
  }
  *pLength = 1;
  //*pData = 0x00;//normal;01:fault

  return ret;
}

int16_t Service22ReadGNSSSAntenaStatus(uint8_t *pData, uint16_t *pLength)
{
  uint8_t status;
  // uint16_t length;
  LocationInfoSync_t localtionInfo;
  *pLength = 1;

  int16_t ret = 0;
  status = 0; // 0x00: Connected/正常连接 0x01: Disconnected/未连�?0x02: Short to GND/对地短路 0x03: Short to KL30/对电源短�?
#if (1)
  // GetGnssAntennaStatus(&status);
  if (StateSyncGetLocationInfo(&localtionInfo) == 0)
  {
    // 0: 正常 1：电线断�?2：电线地短路 3：电线电源短�?
    if (localtionInfo.wireState == 0)
    {
      status = 0x00;
    }
    else if (localtionInfo.wireState == 1)
    {
      status = 0x01;
    }
    else if (localtionInfo.wireState == 2)
    {
      status = 0x02;
    }
    else if (localtionInfo.wireState == 3)
    {
      status = 0x03;
    }
  }
  else
  {
    ret = -1;
  }
#endif
  *pData = status; // 00:connect,01:disconnected,02:short to gnc,03:short to kl30
  *pLength = 1;
  return ret;
}

int16_t Service22ReadGPRSModeStatus(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret = 0;
  *pLength = 1;

  *pData = MpuPowerSyncSdkGetNadModuleStatus(); // 0x00;//normal;01:fault

  return ret;
}

int16_t Service22ReadNADPrimaryAntenaStatus(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret;
  // uint8_t status;
  *pLength = 1;

  ret = 0; // test
  // status = 0;
#if (0)
  GetModemAntennaStatus(&status);
#endif
  *pData = 0x00; // 00:connnected,01:disconnected,02:short to gnd,03:short to kl30;

  return ret;
}
/*
static void registerFlagToU32(uint8_t *data,uint8_t length,uint32_t *pDataOut)
{
  if(length==0)
  {
    *pDataOut = 0;
  }
  else if(length==1)
  {
    *pDataOut = data[0];
  }
  else if(length==2)
  {
    *pDataOut = ((uint32_t)data[0]<<8)+((uint32_t)data[1]<<0);
  }
  else if(length==3)
  {
    *pDataOut = ((uint32_t)data[0]<<16)+((uint32_t)data[1]<<8)+((uint32_t)data[2]<<0);
  }
  else if(length==4)
  {
    *pDataOut = ((uint32_t)data[0]<<24)+((uint32_t)data[1]<<16)+((uint32_t)data[2]<<8)+((uint32_t)data[3]<<0);
  }
  else
  {
    *pDataOut = 0;
  }
}*/

int16_t Service22ReadGPRSNetWorkStatus(uint8_t *pData, uint16_t *pLength)
{
  *pLength = 1;
  TspStateSync_t tspState;
  if (StateSyncGetTspState(&tspState) == 0)
  {
    *pData = tspState.tspStatus; // 0x00= login（已登录平台�?x01= not login（未登录平台�?
  }
  else
  {
    return -1;
  }
  return 0;
}

int16_t Service22ReadCANBUSCommunicationStatus(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret;

  *pLength = 1;

  ret = 0; // test

  *pData = 0x00; // normal;01:fault

  return ret;
}

int16_t Service22ReadUploadFrequency(uint8_t *pData, uint16_t *pLength)
{
  uint32_t cycle;
  int16_t ret;

  cycle = 2000;
  ret = 0; // test

  pData[0] = (cycle >> 24) & 0xFF;
  pData[1] = (cycle >> 16) & 0xFF;
  pData[2] = (cycle >> 8) & 0xFF;
  pData[3] = (cycle >> 0) & 0xFF;

  *pLength = 4;
  return ret;
}

int16_t Service22ReadStorageMemoryUsages(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret;
  static uint8_t RemainValue = 0;

  ret = 0; // test

  RemainValue = (RemainValue + 1) / 2;

  if (RemainValue > 9)
  {
    RemainValue = 1;
  }

  pData[0] = 50 - RemainValue; // test usages

  *pLength = 1;
  return ret;
}

int16_t Service22ReadStorageMemoryStatus(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret;

  ret = 0; // test

  pData[0] = 00; // normal,01:fault

  *pLength = 1;
  return ret;
}

int16_t Service22ReadBackupBatterySoc(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret;

  static uint8_t RandValue = 0;

  ret = 0; // test

  RandValue = (RandValue + 1) / 2;

  if (RandValue >= 9)
  {
    RandValue = 1;
  }
  // pData[0] = 92;//test 92%

  pData[0] = 90 + RandValue;

  *pLength = 1;
  return ret;
}

int16_t Service22ReadBackupBatteryStatus(uint8_t *pData, uint16_t *pLength)
{
  pData[0] = BatterySdkGetBatteyStatus();
  *pLength = 1;

  return 0;
}

int16_t Service22ReadPhoneSignal(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret;

  *pLength = 1;
  ret = 0; // test
#if (1)
  ret = EolTestSyncWithCpuRecv(SYNC_CPU_GET_INFO_ITEM_4G_SIGNAL_VALUE, pData, pLength);
#endif

  return ret;
}

int16_t Service22ReadNetWorkProvider(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret;

  ret = 0; // test
#if (1)
  ret = EolTestSyncWithCpuRecv(SYNC_CPU_GET_INFO_ITEM_NET_PROVIDER, pData, pLength);
#endif

  return ret;
}

int16_t Service22ReadNetWorkType(uint8_t *pData, uint16_t *pLength)
{

  *pLength = 1;

  int16_t ret;

  ret = 0; // test
#if (1)
  ret = EolTestSyncWithCpuRecv(SYNC_CPU_GET_INFO_ITEM_NET_TYPE, pData, pLength);
#endif

  return ret;
}

int16_t Service22ReadApnNumber(uint8_t *pData, uint16_t *pLength)
{
  *pLength = 1;

  int16_t ret;

  ret = 0; // test
#if (1)
  ret = EolTestSyncWithCpuRecv(SYNC_CPU_GET_INFO_ITEM_APN_NUMBER, pData, pLength);
#endif
  return ret;
}

int16_t Service22ReadCpuFlashCapacity(uint8_t *pData, uint16_t *pLength)
{
  *pLength = 4;

  int16_t ret;

  ret = 0; // test
#if (1)
  ret = EolTestSyncWithCpuRecv(SYNC_CPU_GET_INFO_ITEM_FLASH_CAPACITY, pData, pLength);
#endif
  return ret;
}

int16_t Service22ReadHardwareNumber(uint8_t *pData, uint16_t *pLength)
{
#if 1
  // GlobalGetSGMWSoftwareNumer(pData,pLength);
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_HARDWARE_NUM, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = 4;
#else
  GlobalGetSGMWSoftwareNumer(pData, pLength);
#endif
  return 0;
}

int16_t Service22ReadSupplierHardwareNumber(uint8_t *pData, uint16_t *pLength)
{
  uint16_t length;

  ProjectConfigGetHardNumber(pData, &length);

  *pLength = length;
  return 0;
}

int16_t Service22ReadSoftwareNumber(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret;
#if (0)
  uint32_t length;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_SOFTWARE_NUM, pData, &length);
#else
  uint16_t length;
  ret = ProjectConfigGetSoftwareNumber(pData, &length);
#endif
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}
/*******************add 22 DID*************************************************/
int16_t Service22ReadSubnetConfigListOnHighSpeedCan(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_SUBNET_CONFIG_LIST_SPEED_CAN, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = 4;
  return 0;
}

int16_t Service22ReadVehicleManufacturingDate(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_INSTALL_DATE, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = 4;
  return 0;
}

int16_t Service22ReadVariantcode(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_CODE, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadEOLconfig(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_EOL_CONFIG, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = 12;
  return 0;
}

int16_t Service22ReadWorkingMode(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  uint8_t data[2] = {0};

  // ret = WorkFlashUserInfoRead(E_WORKING_MODE,data,&length);
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_WORKING_MODE, data, &length);
  if (ret != 0)
  {
    return -1;
  }
  if (data[0] != 0x5A)
  {
    *pData = 0; // nomal
  }
  else
  {
    *pData = data[1];
  }
  *pLength = 1;
  return 0;
}

int16_t Service22ReadPhoneKeyOffLineUseTimes(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret;
  // int16_t i;
  int16_t flag = 1;
#if (0)
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_PHONEKEY_OFFLINE_USETIMES, pData, &length);

  if (ret != 0)
  {
    return -1;
  }
#endif
  // for(i = 0;i< 12;i++)
  {
    if (pData[0] != 0xFF)
    {
      flag = 0;
    }
  }
#if (1)
  if (flag == 1)
  {
    // memset(pData,0,12);
    pData[0] = 0;
    // WorkFlashVehicleInforStore(E_PARAMETER_INFO_PHONEKEY_OFFLINE_USETIMES, pData, 1);
  }
#endif
  *pLength = 1;
  return 0;
}

int16_t Service22ReadSEID(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret;
  // int16_t i;
  // int16_t flag = 1;
#if (0)
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_SEID, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  for (i = 0; i < 16; i++)
  {
    if (pData[i] != 0xFF)
    {
      flag = 0;
    }
  }
  if (flag == 1)
  {
    memset(pData, 0, 16);
    WorkFlashVehicleInforStore(E_PARAMETER_INFO_SEID, pData, 16);
  }
#endif
  *pLength = 16;
  return 0;
}
int16_t Service22ReadProlinesChannelKey3(uint8_t *pData, uint16_t *pLength)
{
#if (0)
  uint32_t length;
  int16_t ret;
  int16_t i;
  int16_t flag = 1;

  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_PROLINES_CHANNEL_KEY3, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  for (i = 0; i < 16; i++)
  {
    if (pData[i] != 0xFF)
    {
      flag = 0;
    }
  }
  if (flag == 1)
  {
    memset(pData, 0, 16);
    WorkFlashVehicleInforStore(E_PARAMETER_INFO_PROLINES_CHANNEL_KEY3, pData, 16);
  }
#endif
  *pLength = 16;
  return 0;
}

int16_t Service22ReadBleMac(uint8_t *pData, uint16_t *pLength)
{
#if (0)
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_BLE_MAC, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
#endif
  *pLength = 12;
  return 0;
}

int16_t Service22ReadNationIpAddress_DIG(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;

  uint32_t lentemp = 0;

  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_NATION_WORK_ADDRESS, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  lentemp = length;

  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_NATION_WORK_PORT, pData + 16, &length);
  if (ret != 0)
  {
    return -1;
  }
  lentemp += length;

  *pLength = lentemp;

  return 0;
}

int16_t Service22ReadNationIpAddress(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_NATION_WORK_ADDRESS, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadNationIpPort(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_NATION_WORK_PORT, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadGovernmentIpAddress_DIG(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;

  uint32_t lentemp = 0;

  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_GOVERNMENT_WORK_ADDRESS, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  lentemp = length;

  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_GOVERNMENT_WORK_PORT, pData + 16, &length);
  if (ret != 0)
  {
    return -1;
  }
  lentemp += length;
  *pLength = lentemp;
  return 0;
}

int16_t Service22ReadGovernmentIpAddress(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_GOVERNMENT_WORK_ADDRESS, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadGovernmentIpPort(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_GOVERNMENT_WORK_PORT, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadInternalBatteryStatus(uint8_t *buf, uint16_t *len)
{
  uint16_t ret = 0;
  *buf = 0;
  *len = 1;

  return ret;
}

int16_t Service22ReadGNSS_test(uint8_t *buf, uint16_t *len)
{
  static uint8_t temp = 10;
  uint16_t ret = 0;

  *buf = ((temp / 10) % 2);
  *len = 1;
  temp++;
  return ret;
}

int16_t Service22ReadNetWorkProvider_test(uint8_t *buf, uint16_t *len)
{

  uint16_t ret = 0;
  *buf = 0x1;
  *len = 1;

  return ret;
}

int16_t Service22ReadPhoneSignalTest(uint8_t *pData, uint16_t *pLength)
{
  int16_t ret = 0;
  NetInfoSync_t netInfo;

  *pLength = 1;

  if (StateSyncGetNetInfo(&netInfo) == 0)
  {
    pData[0] = netInfo.csq;
    if (pData[0] > 31)
    {
      pData[0] = 0;
    }
    *pLength = 0x01;
  }
  else
  {
    ret = -1;
  }
  return ret;
}

int16_t Service22ReadAppSoftwareFingerprint(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_APP_SOFT_FINGER_PRINT, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = 9;
  return 0;
}

/***********************************************
 * scaling:1
 * physical limits:-127~128
 *
 *
 * ********************************************/
int16_t Service22ReadTboxTemperature(uint8_t *pData, uint16_t *pLength)
{
  HalStateSync_t halState;
  if (StateSyncGetHalstate(&halState) == 0)
  {
    pData[0] = halState.tmpStatus + 127;
  }
  else
  {
    return -1;
  }

  *pLength = 1;
  return 0;
}

// int16_t Service22ReadSystemTime(uint8_t *pData, uint16_t *pLength)
// {
//   // uint32_t length;
//   // int16_t ret;
//   uint32_t utcTime;

//   utcTime = 0;
//   TimeSyncSdkGetUtcTime(&utcTime);
//   // utcTime += (3600*8);//beijing time
//   pData[0] = (utcTime >> 24) & 0xFF;
//   pData[1] = (utcTime >> 16) & 0xFF;
//   pData[2] = (utcTime >> 8) & 0xFF;
//   pData[3] = (utcTime >> 0) & 0xFF;
//   *pLength = 4;
//   return 0;
// }

int16_t Service22ReadNetworkMCC(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  uint8_t buffer[64];

  length = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_IMSI, buffer, &length); // actually IMSI
  if (ret != 0)
  {
    return -1;
  }
  if (length != 15)
  {
    return -1;
  }
  memcpy(pData, buffer, 3);

  *pLength = 3;
  return 0;
}

int16_t Service22ReadNetworkMNC(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  uint8_t buffer[64];

  length = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_IMSI, buffer, &length); // actually IMSI
  if (ret != 0)
  {
    return -1;
  }
  if (length != 15)
  {
    return -1;
  }
  memcpy(pData, buffer + 3, 2);
  pData[2] = 0x00;
  *pLength = 3;
  return 0;
}

/**********************************************************
 * 0:unknown
 * 1:CDMA/����2G����
 * 2��GSM/�ƶ�����??G����
 * 3��WCDMA/��??G����
 * 4��TDSCDMA/�ƶ�3G����
 * 5��EVDO/����3G����
 * 6��LTE/4G����
 **********************************************************/
int16_t Service22ReadConnectionType(uint8_t *pData, uint16_t *pLength)
{
  uint8_t type = 0;
  NetInfoSync_t netInfo;
  if (StateSyncGetNetInfo(&netInfo) == 0)
  {
    if (netInfo.netType == 0) // 0：GSM 1：LTE
    {
      type = 2;
    }
    else if (netInfo.netType == 1)
    {
      type = 6;
    }
  }

  pData[0] = type; // for debug
  *pLength = 1;
  return 0;
}

int16_t Service22ReadModemTemperature(uint8_t *pData, uint16_t *pLength)
{
  HalStateSync_t halState;
  if (StateSyncGetHalstate(&halState) == 0)
  {
    pData[0] = halState.tmpStatus + 127;
  }
  else
  {
    return -1;
  }
  *pLength = 1;
  return 0;
}

int16_t Service22ReadGNSSLiveTime(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret;
  uint32_t utcTime;
  utcTime = 0;
  TimeSyncSdkGetUtcTime(&utcTime);
  pData[0] = (utcTime >> 24) & 0xFF;
  pData[1] = (utcTime >> 16) & 0xFF;
  pData[2] = (utcTime >> 8) & 0xFF;
  pData[3] = (utcTime >> 0) & 0xFF;
  *pLength = 4;
  return 0;
}

int16_t Service22ReadGNSSLiveHeigth(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret;
  uint32_t heigth = 0;
  LocationInfoSync_t localtionInfo;

  // heigth = 500;//debug 500*0.1=50.0m
  if (StateSyncGetLocationInfo(&localtionInfo) == 0)
  {
    heigth = localtionInfo.altitude;
  }
  else
  {
    return -1;
  }

  pData[0] = (heigth >> 24) & 0xFF;
  pData[1] = (heigth >> 16) & 0xFF;
  pData[2] = (heigth >> 8) & 0xFF;
  pData[3] = (heigth >> 0) & 0xFF;
  *pLength = 4;
  return 0;
}

int16_t Service22ReadGNSSLiveHDOP(uint8_t *pData, uint16_t *pLength)
{
  uint16_t hdop = 0;
  LocationInfoSync_t localtionInfo;

  if (StateSyncGetLocationInfo(&localtionInfo) == 0)
  {
    hdop = localtionInfo.accuracy;
  }
  else
  {
    return -1;
  }

  pData[0] = (hdop >> 8) & 0xFF;
  pData[1] = (hdop >> 0) & 0xFF;
  *pLength = 2;
  return 0;
}

int16_t Service22ReadGNSSLiveSpeed(uint8_t *pData, uint16_t *pLength)
{
  uint16_t speed = 0;
  LocationInfoSync_t localtionInfo;

  if (StateSyncGetLocationInfo(&localtionInfo) == 0)
  {
    speed = localtionInfo.speed * 10;
  }
  else
  {
    return -1;
  }
  pData[0] = (speed >> 8) & 0xFF;
  pData[1] = (speed >> 0) & 0xFF;

  *pLength = 2;
  return 0;
}

int16_t Service22ReadGNSSLiveHeading(uint8_t *pData, uint16_t *pLength)
{
  uint16_t heading = 0;
  LocationInfoSync_t localtionInfo;

  if (StateSyncGetLocationInfo(&localtionInfo) == 0)
  {
    heading = localtionInfo.heading / 10;
  }
  else
  {
    return -1;
  }
  pData[0] = (heading >> 8) & 0xFF;
  pData[1] = (heading >> 0) & 0xFF;

  *pLength = 2;
  return 0;
}

int16_t Service22ReadGNSSCurrentNumOfSatallitesUsed(uint8_t *pData, uint16_t *pLength)
{
  uint8_t num = 0;
  LocationInfoSync_t localtionInfo;
  if (StateSyncGetLocationInfo(&localtionInfo) == 0)
  {
    num = localtionInfo.useSvsnum;
  }
  else
  {
    return -1;
  }
  pData[0] = num; // debug,0~12
  *pLength = 1;
  return 0;
}

int16_t Service22ReadGNSSCurrentNumOfSatallitesInView(uint8_t *pData, uint16_t *pLength)
{
  uint8_t num = 0;
  LocationInfoSync_t localtionInfo;
  if (StateSyncGetLocationInfo(&localtionInfo) == 0)
  {
    num = localtionInfo.svsNum;
  }
  else
  {
    return -1;
  }
  pData[0] = num; // debug,0~12
  *pLength = 1;
  return 0;
}

int16_t Service22ReadBackupBatteryTemperature(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret;
  int32_t tem;

  BatterySdkGetTemperature(&tem);
  tem /= 100;
  pData[0] = tem + 127; //
  *pLength = 1;
  return 0;
}

int16_t Service22ReadBackupBatteryAgeCoefficient(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret;

  pData[0] = 10; // debug 10*0.04=0.4
  *pLength = 1;
  return 0;
}

int16_t Service22ReadBackupBatteryVoltage(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret;
  uint32_t voltage = 0;

  BatterySdkGetVoltage(&voltage);
  voltage /= 20;
  pData[0] = voltage; // debug
  *pLength = 1;
  return 0;
}

int16_t Service22ReadIMMOStatus(uint8_t *pData, uint16_t *pLength)
{
  uint8_t key[32];
  uint32_t dataLength;
  uint8_t immoStatus;
  // uint32_t i;
  int16_t ret;

  immoStatus = 0;
  dataLength = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ESK_KEY, key, &dataLength);
  if (ret == 0)
  {
    if (dataLength == 16)
    {
      if (CheckEskValid(key) == 0)
      {
        immoStatus = 1;
      }
    }
  }

  pData[0] = immoStatus; // 00:UUC is virgin;0x01:UCU has ESK;
  *pLength = 1;
  return 0;
}

int16_t Service22ReadPowerIn(uint8_t *pData, uint16_t *pLength)
{
  uint32_t voltage;
  uint32_t tem;

  PeripheralHalAdGet(AD_CHANNEL_KL30, &voltage);
  voltage += 420;                       // offset:0.42
  tem = ((uint32_t)voltage) * 10 / 276; // scaling:0.0276
  pData[0] = (tem >> 8) & 0xFF;
  pData[1] = tem & 0xFF;
  *pLength = 2;
  return 0;
}

int16_t Service22ReadEepromCheck(uint8_t *pData, uint16_t *pLength)
{
  pData[0] = 0x01; // 0x00:Software is not to match the eprom;0x01:Software is to match the eprom

  *pLength = 1;
  return 0;
}

int16_t Service22ReadDiagnosticCanReport(uint8_t *pData, uint16_t *pLength)
{

  /**********************************************************
    0x00:not report,quite CAN Report
    0x01:reserved;
    0x02:Report B CAN messages to Diagnostic CAN;
    0x03:Report E CAN messages to Diagnostic CAN;
    0x04:Report C CAN messages to Diagnostic CAN;
    0x05:Report I CAN messages to Diagnostic CAN;
    0x06:Report A CAN messages to Diagnostic CAN;
  **************************************************************/
  uint32_t dataLength;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_DIAG_CAN_REPORT, pData, &dataLength);
  if (ret != 0)
  {
    return -1;
  }

  if (g_2EReportToDcanVaule != pData[0])
  {
    g_2EReportToDcanVaule = pData[0];
  }
  *pLength = 1;
  return 0;
}

int16_t Service22ReadUCUType(uint8_t *pData, uint16_t *pLength)
{
  memcpy(pData, g_ucuType, 11);
  *pLength = 11;
  return 0;
}

/*****************2E write by identufy******************************************************/
int16_t Service2EWriteGovernmentIpAddress(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_GOVERNMENT_WORK_ADDRESS, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }

  // SetParameterSyncToCpu(pData,dataLength);
  return 0;
}

int16_t Service2EWriteNationIpAddress(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_NATION_WORK_ADDRESS, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }

  // SetParameterSyncToCpu(pData,dataLength);
  return 0;
}

int16_t Service2EWriteRepairShopCodeOrTestSN(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_DIAG_SN, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  // SetParameterSyncToCpu(pData,dataLength);
  return 0;
}

int16_t Service2EWriteInstallationDate(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  /*if(ParameterSyncSdkGetFromCpuIsFinished()!=0)
  {
    return -1;
  } */
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_INSTALL_DATE, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  // SetParameterSyncToCpu(pData,dataLength);
  return 0;
}

/*static int16_t CheckByteArrayIsSame(const uint8_t *pData1,const uint8_t *pData2,uint32_t len)
{
  uint32_t i;
  int16_t valid;

  valid = 0;
  for(i=0;i<len;i++)
  {
    if(pData1[i]!=pData2[i])
    {
      valid = 1;
      break;
    }
  }
  return valid;
}*/

/*static int16_t CheckVehicleTypeVaild(uint8_t *pConfig,uint8_t length)
{
  uint8_t i;
  uint8_t arraySize;
  int16_t valid;

  valid = 1;
  arraySize = sizeof(m_EolConfig)/12;

  for(i=0;i<arraySize;i++)
  {
    if(CheckByteArrayIsSame(m_EolConfig[i],pConfig,12)==0)
    {
      valid = 0;//valid ok
      break;
    }
  }
  return valid;
}*/

int16_t Service2EWriteEOLconfig(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;
  // uint8_t i = 0;

  /*if(CheckVehicleTypeVaild(pData,dataLength)!=0)
  {
    return -1;
  }*/
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_EOL_CONFIG, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
#if (0)
  // g_vehicleTypeConfigure
  memcpy(g_vehicleTypeConfigure, pData, sizeof(g_vehicleTypeConfigure));

  for (i = 0; i < sizeof(g_vehicleTypeConfigure); i++)
  {
    if (g_vehicleTypeConfigure[i] & 0xFF) // configured
    {
      ClearDtcFaultState(DTC_ITEM_CONFIGURE_BYTE_NOT_COMPLETE);
      return 0;
    }
    else
    {
      if (i == 11)
      {
        SetDtcFaultState(DTC_ITEM_CONFIGURE_BYTE_NOT_COMPLETE);
      }
    }
  }
#endif
  // ReCheckConfigRelatedDtc();
  return 0;
}

int16_t Service2EWriteEOLconfigByEol(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;
  uint8_t i = 0;

  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_EOL_CONFIG, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  // g_vehicleTypeConfigure
  memcpy(g_vehicleTypeConfigure, pData, sizeof(g_vehicleTypeConfigure));
  for (i = 0; i < sizeof(g_vehicleTypeConfigure); i++)
  {
    if (g_vehicleTypeConfigure[i] & 0xFF) // configured
    {
      // ClearDtcFaultState(DTC_ITEM_CONFIGURE_BYTE_NOT_COMPLETE);
      return 0;
    }
    else
    {
      if (i == 11)
      {
        // SetDtcFaultState(DTC_ITEM_CONFIGURE_BYTE_NOT_COMPLETE);
      }
    }
  }
  return 0;
}

int16_t Service2EWriteVariantcode(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_CODE, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  // SetParameterSyncToCpu(pData,dataLength);
  return 0;
}

int16_t Service2EWriteHardwareNumber(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_HARDWARE_NUM, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteSoftwareNumber(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_SOFTWARE_NUM, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

/************************** ADD 2E DID *****************************************************/

int16_t Service2EWriteSupplierId(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_SYSTEM_SUPPLIER_ID, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  /*ret = SetParameterSyncToCpu(SYNC_PARAMETER_ITEM_SUPPLIER_IDENTIFIER,pData,dataLength);
  if(ret!=0)
  {
    return -1;
  } */
  return 0;
}

int16_t Service2EWriteHardwareVersion(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_HARDWARE_VERSION, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  /*ret = SetParameterSyncToCpu(SYNC_PARAMETER_ITEM_ECU_HARDWARE_VERSION,pData,dataLength);
  if(ret!=0)
  {
    return -1;
  } */
  return 0;
}

int16_t Service2EWriteSoftwareVersion(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_SOFTWARE_VERSION, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  /*ret = SetParameterSyncToCpu(SYNC_PARAMETER_ITEM_ECU_SOFTWARE_VERSION,pData,dataLength);
  if(ret!=0)
  {
    return -1;
  } */
  return 0;
}

int16_t Service2EWriteEcuMask(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_SAP_ECU_MASK, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteManufactureDate(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;
  uint16_t ascLength;
  uint8_t asc[64];

  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = HexArrayToCharArray(pData, dataLength, asc, &ascLength);
  if (ret != 0) // convert failed
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_MANUFACTURE_DATE, asc, ascLength);
  if (ret != 0)
  {
    return -1;
  }

  /*ret = SetParameterSyncToCpu(SYNC_PARAMETER_ITEM_MANUFACTURE_DATE,asc,ascLength);
  if(ret!=0)
  {
    return -1;
  } */
  return 0;
}

int16_t Service2EWritePartName(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PART_NAME, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }

  return 0;
}

int16_t Service2EWriteESKeyByEol(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;
  if (dataLength != 16)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ESK_KEY, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  if (CheckEskValid(pData) == 0)
  {
    g_eskKeyValid = 1;
  }
  else
  {
    g_eskKeyValid = 0;
  }
  return 0;
}

static int16_t SetWorkingMode(uint8_t oldMode, uint8_t newMode)
{
  if (oldMode == 0) // nomal
  {
    if (newMode == 1) // factory mode
    {
      // 关闭远程诊断功能
      RemoteDiagnosticSdkDisable();
    }
    else if (newMode == 2) // 运输模式
    {
      // 进入深度休眠，关闭MPU
      PowerManageSdkDeepSleep(1);
    }
  }
  else if (oldMode == 1) // factory mode
  {
    if (newMode == 0)
    {
      // 恢复远程诊断功能
      RemoteDiagnosticSdkRecover();
    }
    else if (newMode == 2)
    {
      RemoteDiagnosticSdkRecover();
      // 进入深度休眠，关闭MPU
      PowerManageSdkDeepSleep(1);
    }
  }
  else if (oldMode == 2) // transport mode
  {
    if (newMode == 0)
    {
      // 退出深度休眠
      PowerManageSdkDeepSleep(0);
    }
    else if (newMode == 1)
    {
      // 退出深度休眠
      PowerManageSdkDeepSleep(0);
      // 关闭远程诊断功能
      RemoteDiagnosticSdkDisable();
    }
  }
  else
  {
    return -1;
  }
  return 0;
}
int16_t Service2EWriteWorkingMode(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret = 0;
  uint8_t data[2] = {0}; // nomal mode
  uint32_t lenth = 0;
  if (dataLength != 1)
  {
    return -1;
  }
  // ret = WorkFlashUserInfoRead(E_WORKING_MODE,data,&lenth);
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_WORKING_MODE, data, &lenth);
  if (ret != 0)
  {
    return -1;
  }
  if ((data[0] == 0x5A) && (data[1] == pData[0]))
  {
    if (pData[0] == 0)
    {
      MpuPowerSyncSdkSetRkMode(1);
    }
    else if (pData[0] == 1)
    {
      MpuPowerSyncSdkSetRkMode(3);
    }
    else if (pData[0] == 2)
    {
      MpuPowerSyncSdkSetRkMode(2);
    }
    return 0;
  }
  else
  {
    if (data[0] == 0x5A)
    {
      SetWorkingMode(data[1], pData[0]);
    }
    else
    {
      SetWorkingMode(0, pData[0]);
    }
    data[0] = 0x5A;
    data[1] = pData[0];
    // ret = WorkFlashUserInfoStore(E_WORKING_MODE,data,2);
    ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_WORKING_MODE, data, 2);
    if (ret == 0)
    {
      if (pData[0] == 0)
      {
        MpuPowerSyncSdkSetRkMode(1);
      }
      else if (pData[0] == 1)
      {
        MpuPowerSyncSdkSetRkMode(3);
      }
      else if (pData[0] == 2)
      {
        MpuPowerSyncSdkSetRkMode(2);
      }
    }
  }

  return ret;
}
#if 0
static int16_t CheckVINIsValid(uint8_t *vinData, uint16_t vinLength)
{
  int16_t ret = 0;
  uint16_t i;
  for (i = 0; i < vinLength; i++)
  {
    if ((vinData[i] < 0x20) || (vinData[i] > 0x7E))
    {
      ret = -1;
      break;
    }
  }
  return ret;
}
#endif
// int16_t Service2EWriteVIN(uint8_t *pData, uint16_t dataLength)
// {
//   int16_t ret;
//   uint8_t vin[17];
// #if (0)
//   if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
//   {
//     return -1;
//   }
// #endif
//   ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_VIN_hex, pData, dataLength);
//   if (ret != 0)
//   {
//     return -1;
//   }
//   // if (CheckVINIsValid(pData, dataLength) != 0)
//   // {
//   //   memset(vin, 0x30, 17);
//   // }
//   // else
//   // {
//   //   memcpy(vin, pData, 17);
//   // }
//   // ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_VIN, vin, 17);
//   // if (ret != 0)
//   // {
//   //   return -1;
//   // }
// #if (0)
//   ret = ParameterSyncSdkSetToCpu(E_ParamId_VIN, pData, dataLength);
//   if (ret != 0)
//   {
//     return -1;
//   }
//   memcpy(g_vin, pData, 17);
//   CheckVinWriteStatus();
// #endif
// #if (SECOC_ENABLE == 1)
//   SecocSdkReInit();
// #endif
//   return 0;
// }

// 0xF190_cxl
int16_t Service2EWriteVIN(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 17)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }

  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_VIN, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }
  if (storeResult == 0)
  {
    ParameterSyncSdkSetToCpu(E_ParamId_VIN, pData, dataLength);
  }

  return 0;
}
// F18C
int16_t Service2EWriteSerialNumber(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 45)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_SERIAL_NUMBER, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }
  if (storeResult == 0)
  {
    ParameterSyncSdkSetToCpu(E_ParamId_SN, pData, dataLength);
  }
  return 0;
}

// int16_t Service2EWriteICCID(uint8_t *pData, uint16_t dataLength)
// {
//   int16_t ret;
//   if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
//   {
//     return -1;
//   }
//   ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ICCID, pData, dataLength);
//   if (ret != 0)
//   {
//     return -1;
//   }
//   // SetParameterSyncToCpu(pData,dataLength);
//   return 0;
// }

int16_t Service2EWriteTboxRegisterFlag(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_TBOX_REGISTER_FLAG, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
#if (0)
  ret = SetParameterSyncToCpu(SYNC_PARAMETER_ITEM_REGISTER_FLAG, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
#endif
  return 0;
}

int16_t Service2EWriteFingerPrint(uint8_t *pData, uint16_t dataLength)
{
#if (0)
  int16_t ret;
  ret = WorkFlashWriteFingerPrint(0, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
#endif
  return 0;
}

int16_t Service2EWriteAppSoftwareFingerprint(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_APP_SOFT_FINGER_PRINT, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteSubnetConfigListOnHighSpeedCan(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_SUBNET_CONFIG_LIST_SPEED_CAN, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

static void CheckCanReportDtc(uint8_t reportValue)
{
  if (reportValue)
  {
    // SetDtcFaultState(DTC_ITEM_GATEWAY_MAPING);
  }
  else
  {
    // ClearDtcFaultState(DTC_ITEM_GATEWAY_MAPING);
  }
}

int16_t Service2EWriteDiagnosticCanReport(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  if (g_31ReportToDcanFlag)
  {
    return 0x22;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_DIAG_CAN_REPORT, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  // GateWaySetCanReportControl(pData[0]);
  g_2EReportToDcanVaule = pData[0];
  CheckCanReportDtc(g_2EReportToDcanVaule);
  return 0;
}

int16_t Service2EWriteProlinesChannelKey(uint8_t *pData, uint16_t dataLength)
{
  // int16_t ret;
  if (dataLength != 16)
  {
    return -1;
  }
#if (0)
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PROLINES_CHANNEL_KEY, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
#endif
  return 0;
}
int16_t Service2EWritePhoneKeyOffLineUseTimes(uint8_t *pData, uint16_t dataLength)
{
#if (0)
  int16_t ret;

  if (dataLength != 1)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PHONEKEY_OFFLINE_USETIMES, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
#endif
  return 0;
}
int16_t Service2EWriteNFCCardID(uint8_t *pData, uint16_t dataLength)
{
#if (0)
  int16_t ret;
  if (dataLength != 12)
  {
    return -1;
  }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_NFC_CARD_ID, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
#endif
  return 0;
}

#endif
/********************31 start routine ****************************************/
int16_t Service31CheckPreFlashConditon(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  pDataOut[0] = 0;
  *pLengthOut = 1;
  return 0;
}

// int16_t CanDriverHalAppMsgDisable(typeCanHandLe canHandle);
// int16_t CanDriverHalAppMsgEnable(typeCanHandLe canHandle);

int16_t Service31StopApplication(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
#if (0)
  if ((GetVehicleSpeed() > 10) && (GetVehicleSpeed() != 0xFFFF))
  {
    // negativeNum = 0x22;
    return 0x22;
  }
  if (lengthIn != 1)
  {
    return 0x13; // lenth negtive
  }
  // BCAN
  if (pDataIn[0] & (0x01 << 6))
  {
    if (!(m_ApplicationStop & (0x01 << 6)))
    {
      CanDriverHalAppMsgDisable(TBOX_CAN_CHANNEL_B << 8);
    }
  }
  else
  {
    if ((m_ApplicationStop & (0x01 << 6)))
    {
      CanDriverHalAppMsgEnable(TBOX_CAN_CHANNEL_B << 8);
    }
  }
  // ECAN
  if (pDataIn[0] & (0x01 << 5))
  {
    if (!(m_ApplicationStop & (0x01 << 5)))
    {
      CanDriverHalAppMsgDisable(TBOX_CAN_CHANNEL_E << 8);
    }
  }
  else
  {
    if ((m_ApplicationStop & (0x01 << 5)))
    {
      CanDriverHalAppMsgEnable(TBOX_CAN_CHANNEL_E << 8);
    }
  }
  // CCAN
  if (pDataIn[0] & (0x01 << 4))
  {
    if (!(m_ApplicationStop & (0x01 << 4)))
    {
      CanDriverHalAppMsgDisable(TBOX_CAN_CHANNEL_C << 8);
    }
  }
  else
  {
    if ((m_ApplicationStop & (0x01 << 4)))
    {
      CanDriverHalAppMsgEnable(TBOX_CAN_CHANNEL_C << 8);
    }
  }
  // ICAN
  if (pDataIn[0] & (0x01 << 3))
  {
    if (!(m_ApplicationStop & (0x01 << 3)))
    {
      CanDriverHalAppMsgDisable(TBOX_CAN_CHANNEL_I << 8);
    }
  }
  else
  {
    if ((m_ApplicationStop & (0x01 << 3)))
    {
      CanDriverHalAppMsgEnable(TBOX_CAN_CHANNEL_I << 8);
    }
  }
  // ACAN
  if (pDataIn[0] & (0x01 << 1))
  {
    if (!(m_ApplicationStop & (0x01 << 1)))
    {
      CanDriverHalAppMsgDisable(TBOX_CAN_CHANNEL_A << 8);
    }
  }
  else
  {
    if ((m_ApplicationStop & (0x01 << 1)))
    {
      CanDriverHalAppMsgEnable(TBOX_CAN_CHANNEL_A << 8);
    }
  }
#endif
  // m_ApplicationStop = pDataIn[0];
  *pLengthOut = 0;
  return 0;
}

void Service31StopApplicationSetDefault(void)
{
#if (0)
  // BCAN
  if ((m_ApplicationStop & (0x01 << 6)))
  {
    // CanDriverHalAllTxMsgEnable(TBOX_CAN_CHANNEL_B);
    CanDriverHalAppMsgEnable(TBOX_CAN_CHANNEL_B << 8);
  }
  // ECAN
  if ((m_ApplicationStop & (0x01 << 5)))
  {
    // CanDriverHalAllTxMsgEnable(TBOX_CAN_CHANNEL_E);
    CanDriverHalAppMsgEnable(TBOX_CAN_CHANNEL_E << 8);
  }
  // CCAN
  if ((m_ApplicationStop & (0x01 << 4)))
  {
    // CanDriverHalAllTxMsgEnable(TBOX_CAN_CHANNEL_C);
    CanDriverHalAppMsgEnable(TBOX_CAN_CHANNEL_C << 8);
  }
  // ICAN
  if ((m_ApplicationStop & (0x01 << 3)))
  {
    // CanDriverHalAllTxMsgEnable(TBOX_CAN_CHANNEL_C);
    CanDriverHalAppMsgEnable(TBOX_CAN_CHANNEL_I << 8);
  }
  // ACAN
  if ((m_ApplicationStop & (0x01 << 1)))
  {
    // CanDriverHalAllTxMsgEnable(TBOX_CAN_CHANNEL_A);
    CanDriverHalAppMsgEnable(TBOX_CAN_CHANNEL_A << 8);
  }
#endif
  // m_ApplicationStop = 0x00;
}

int16_t Service31WriteDiagnosticCanReport01(uint8_t *pData, uint16_t dataLength, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  // int16_t ret;

  if (dataLength != 1)
  {
    return 0x13; // lenth negtive
  }
  /*ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_DIAG_CAN_REPORT,pData,dataLength);
  if(ret!=0)
  {
    return -1;
  }*/

  // GateWaySetCanReportControl(pData[0]);
  g_31ReportToDcanFlag = 0x01;
  return 0;
}

int16_t Service31WriteDiagnosticCanReport02(uint8_t *pData, uint16_t dataLength, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  uint8_t tem[32];
  uint32_t len;
  if (g_31ReportToDcanFlag)
  {
    g_31ReportToDcanFlag = 0;
    WorkFlashVehicleInforRead(E_PARAMETER_INFO_DIAG_CAN_REPORT, tem, &len);
    // GateWaySetCanReportControl(tem[0]);
  }
  return 0;
}

int16_t Service31WriteDiagnosticCanReport03(uint8_t *pData, uint16_t dataLength, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  pDataOut[0] = g_31ReportToDcanValue;
  *pLengthOut = 0x01;
  return 0;
}

void Service31WriteDiagnosticCanReportSetDefault(void)
{
  uint8_t tem[32];
  uint32_t len;
  if (g_31ReportToDcanFlag)
  {
    g_31ReportToDcanFlag = 0;
    WorkFlashVehicleInforRead(E_PARAMETER_INFO_DIAG_CAN_REPORT, tem, &len);
    // GateWaySetCanReportControl(tem[0]);
  }
  g_31ReportToDcanValue = 0x00;
}

/*******************************************************************************/

/*******************************************************************************/
void CheckEolConfigureState(void)
{
  // uint8_t tem[20];
  // uint32_t length;
  static uint32_t count = 150;
  uint8_t i = 0;
  count++;
  if (count < 200)
  {
    return;
  }
  count = 0;
  // WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_EOL_CONFIG,tem,&length);

  for (i = 0; i < sizeof(g_vehicleTypeConfigure); i++)
  {
    if (g_vehicleTypeConfigure[i] & 0xFF) // configured
    {
      // ClearDtcFaultState(DTC_ITEM_CONFIGURE_BYTE_NOT_COMPLETE);
      return;
    }
    else
    {
      if (i == 11)
      {
        // SetDtcFaultState(DTC_ITEM_CONFIGURE_BYTE_NOT_COMPLETE);
      }
    }
  }
}

static void CheckCanReportDtc(uint8_t reportValue)
{
  if (reportValue)
  {
    // SetDtcFaultState(DTC_ITEM_GATEWAY_MAPING);
  }
  else
  {
    // ClearDtcFaultState(DTC_ITEM_GATEWAY_MAPING);
  }
}

void CheckCanReportState(void)
{
  static uint32_t count = 150;
  count++;
  if (count < 200)
  {
    return;
  }
  count = 0;
  CheckCanReportDtc(g_2EReportToDcanVaule);
}
void EolConfigureInit(void)
{
  // g_vehicleTypeConfigure;
  uint8_t tem[32];
  uint32_t len;
  len = 0;
  WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_EOL_CONFIG, tem, &len);

  if (len != 12)
  {
    memset(tem, 0x00, sizeof(tem));
    WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_EOL_CONFIG, tem, 12);
  }
  memcpy(g_vehicleTypeConfigure, tem, sizeof(g_vehicleTypeConfigure));
}

const uint8_t *GetVehicleTypeConfigure(void)
{
  return g_vehicleTypeConfigure;
}

void DiagCanReportValueInit(void)
{
  uint8_t tem[32];
  uint32_t len;

  WorkFlashVehicleInforRead(E_PARAMETER_INFO_DIAG_CAN_REPORT, tem, &len);
  if (tem[0] >= 0x07)
  {
    tem[0] = 0x00;
    WorkFlashVehicleInforStore(E_PARAMETER_INFO_DIAG_CAN_REPORT, tem, 1);
  }
  g_2EReportToDcanVaule = tem[0];
  // GateWaySetCanReportControl(tem[0]);
}

void VinInit(void)
{
  int16_t ret;
  uint8_t vinTem[40];
  uint32_t length;
  memset(g_vin, 0x00, 17);
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_VIN, vinTem, &length);
  if (ret != 0)
  {
    return;
  }
  if (CheckVinIsAsciiZero(vinTem) != 0)
  {
    memcpy(g_vin, vinTem, 17);
    return;
  }
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_VIN_hex, vinTem, &length);
  if (ret != 0)
  {
    return;
  }
  memcpy(g_vin, vinTem, 17);
}

static void CheckVinWriteStatus(void)
{
#if (0)
  uint8_t i;
  uint8_t vinValidFlag;
  vinValidFlag = 0;
  for (i = 0; i < 17; i++)
  {
    if (g_vin[i] != 0x00)
    {
      vinValidFlag = 1;
      break;
    }
  }
#endif
  // TBOX_PRINT("CheckVinWriteStatus--%d\r\n",vinValidFlag);
  // TBOX_PRINT("vin %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n",g_vin[0],g_vin[1],g_vin[2],g_vin[3],g_vin[4],g_vin[5],g_vin[6],g_vin[7],g_vin[8],g_vin[9],g_vin[10],g_vin[11],g_vin[12],g_vin[13],g_vin[14],g_vin[15],g_vin[16]);
  /*if(vinValidFlag)
  {
    ClearDtcFaultState(DTC_ITEM_VIN_NOT_WRITEN);
  }
  else
  {
    SetDtcFaultState(DTC_ITEM_VIN_NOT_WRITEN);
  }*/
}

void CheckVinCyleProcess(void)
{
  static uint32_t count = 150;
  count++;
  if (count < 200)
  {
    return;
  }
  count = 0;
  CheckVinWriteStatus();
}

void EskkeyCheckInit(void)
{
  uint32_t dataLength;
  uint8_t key[64];
  uint8_t keyValid;
  int16_t ret;
  // uint16_t i;

  dataLength = 0;
  keyValid = 0;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ESK_KEY, key, &dataLength);
  if (ret == 0)
  {
    if (dataLength == 16)
    {
      if (CheckEskValid(key) == 0)
      {
        keyValid = 1;
      }
    }
  }
  g_eskKeyValid = keyValid;
}

void CheckEskValidtateDtc(void)
{
  // uint8_t tem[20];
  // uint32_t length;
  static uint32_t count = 150;
  count++;
  if (count < 200)
  {
    return;
  }
  count = 0;

  if (g_eskKeyValid) // configured
  {
    // ClearDtcFaultState(DTC_ITEM_ESK_NOT_WRITEN);
  } //
  else
  {
    // SetDtcFaultState(DTC_ITEM_ESK_NOT_WRITEN);
  }
}

int16_t Service2EWritePIN(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  /*if(GetSyncParameterFromCpuIsFinished()!=0)
  {
    return -1;
  }  */
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PIN, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service22ReadPIN(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_PIN, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

#if 0
int16_t Service2EWriteVehicleNetworkConfigurationF1A1(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  // if(GetSyncParameterFromCpuIsFinished()!=0)
  // {
  //   return -1;
  // }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_VEHICLE_NETWORK_CONFIGURATION_F1A1, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteVehicleNetworkConfigurationF1B0(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  // if(GetSyncParameterFromCpuIsFinished()!=0)
  // {
  //   return -1;
  // }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_VEHICLE_NETWORK_CONFIGURATION_F1B0, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteECUlevelNetworkConfigurationDataIdentifierF1B1(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  // if(GetSyncParameterFromCpuIsFinished()!=0)
  // {
  //   return -1;
  // }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_LEVEL_NETWORK_CONFIGURATION_DATA_IDENTIFIER_F1B1, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteVehicleModelF112(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  // if(GetSyncParameterFromCpuIsFinished()!=0)
  // {
  //   return -1;
  // }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_VEHICLE_MODEL_F112, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteFingerprintF1F0(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  // if(GetSyncParameterFromCpuIsFinished()!=0)
  // {
  //   return -1;
  // }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_FINGERPRINT_F1F0, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteFingerprintEthernetF0FF(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  // if(GetSyncParameterFromCpuIsFinished()!=0)
  // {
  //   return -1;
  // }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_FINGERPRINT_ETHERNET_F0FF, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteVehicleSoftwareVersionF1AB(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  // if(GetSyncParameterFromCpuIsFinished()!=0)
  // {
  //   return -1;
  // }
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_VEHICLE_SOFEWARE_VERSION_F1AB, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}
#if 0
//生产模式
int16_t Service2EWriteWorkingModeF1C2(uint8_t *pData,uint16_t dataLength)
{
    int16_t ret;

    /*  if(GetSyncParameterFromCpuIsFinished()!=0)
    {
    return -1;
    }*/ 
    ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_WORKING_MODE_F1C2,pData,dataLength);
    if(ret!=0)
    {
        return -1;
    }  
   if(pData[0] == 1)//生产模式，禁止DTC
   {
       DtcProcessDisable();
   }
   else
   {
       DtcProcessEnable();
   }
    return 0;  
}
#endif
// 维修模式
int16_t Service2EWriteMaintenanceModeFD00(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  /*  if(GetSyncParameterFromCpuIsFinished()!=0)
    {
      return -1;
    } */
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_MAINTENANCE_MODE_FD00, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  // SetWorkingMode(pData[0]);
  return 0;
}

int16_t Service2EWriteSecOCKeyF1C7(uint8_t *pData, uint16_t dataLength) // SecOC密钥 HEX
{
  int16_t ret;
  int16_t i;
  uint8_t keyValid;
  keyValid = 0;
  for (i = 0; i < 16; i++)
  {
    if (pData[i] != 0)
    {
      keyValid = 1;
      break;
    }
  }
  if (0 == keyValid)
  {
    return -1;
  }
#if (SECOC_ENABLE == 1)
  SecocSdkSetTripCount(0);
  SecocSdkSetRestCount(0);
#endif
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_SECOC_KEY_F1C7, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
#if (SECOC_ENABLE == 1)
  SecocSdkReInit();
  SecocSdkStartSyncMsgRequest();
#endif
  return 0;
}

int16_t Service22ReadPkiStateF1CA(uint8_t *pData, uint16_t *pLength)
{
  uint8_t ret = 0;
  *pLength = 1;
  // ret = GetPKIState();
  pData[0] = ret;
  return 0;
}

int16_t Service22ReadCsrHashF1CD(uint8_t *pData, uint16_t *pLength) // CSR_HASH 	ASCII	20
{
  // int16_t ret;
  uint32_t dataLength;

  uint8_t i, value;

  //    if(GetSyncParameterFromCpuIsFinished()!=0)//判断是否
  //    {
  //        return -1;
  //    }
  WorkFlashVehicleInforRead(E_PARAMETER_INFO_CSR_HASH_F1CD, pData, &dataLength);
  //    GetCsrHash(pData,&length);
  if (dataLength > 20)
  {
    return -1;
  }
  value = 0;
  for (i = 0; i < dataLength; i++)
  {
    if ((pData[i] != 0) && (pData[i] != 0xFF))
    {
      value = 1;
      break;
    }
  }
  if (0 == value)
  {
    return -1;
  }
  *pLength = dataLength;
  return 0;
}

int16_t Service2EWriteCertificateF1CB(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret = 0;

  // if(GetSyncParameterFromCpuIsFinished()!=0)
  // {
  //   return -1;
  // }
  // ret = McuSendCertificateDataToCpu(pData,&dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteSK(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  /*if(GetSyncParameterFromCpuIsFinished()!=0)
  {
    return -1;
  }  */
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_SK, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteManufactureDateF18B(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;
  // uint16_t ascLength;
  // uint8_t asc[64];

  /*if (GetSyncParameterFromCpuIsFinished() != 0)
  {
      return -1;
  }*/
  //    ret = HexArrayToCharArray(pData, dataLength, asc, &ascLength);
  //    if (ret != 0) //convert failed
  //    {
  //        return -1;
  //    }
  //    ret = WorkFlashVehicleInforStore(NONVOLATILE_ECU_MANUFACTURE_DATE_F18B, asc, ascLength);
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_MANUFACTURE_DATE, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }

  /*ret = SetParameterSyncToCpu(SYNC_PARAMETER_ITEM_MANUFACTURE_DATE,asc,ascLength);
  if(ret!=0)
  {
    return -1;
  } */
  return 0;
}

int16_t Service2EWriteTripRestAA11(uint8_t *pData, uint16_t dataLength)
{
  // SetTripCount(pData[0]);
  // SetRestCount(pData[1]);
  return 0;
}

int16_t Service2EWritePIN(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;

  /*if(GetSyncParameterFromCpuIsFinished()!=0)
  {
    return -1;
  }  */
  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PIN, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  return 0;
}
// 0x010D_cxl
int16_t Service2EWriteESKey(uint8_t *pData, uint16_t dataLength)
{
  int16_t ret;
  // int16_t i;
  // uint8_t dataValid;
  // uint8_t tem[32];
  // uint32_t len;

  ret = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ESK_KEY, pData, dataLength);
  if (ret != 0)
  {
    return -1;
  }
  if (CheckEskValid(pData) == 0)
  {
    g_eskKeyValid = 0x01;
  }
  else
  {
    g_eskKeyValid = 0x00;
  }
  return 0;
}

// 0x011B_cxl
int16_t Service2EWriteApn1(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 32)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }  
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_APN1, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  return 0;
}

// 0x011C_cxl
int16_t Service2EWriteIp1Addr(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 16)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_IP1_ADDR, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0x011D_cxl
int16_t Service2EWriteTspPort(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 8)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PORT1, pData, dataLength);
  //  TBOX_PRINT("Service2EWriteTspPort storeResult=%d\r\n", storeResult);
  return 0;
}

// 0xB209_cxl
int16_t Service2EWriteTspTimeout(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0x011F_cxl
int16_t Service2EWriteECallNum1(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 14)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECALL_NUM1, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }
  if (storeResult == 0)
  {
    ParameterSyncSdkSetToCpu(E_ParamId_ECallNumber, pData, dataLength);
  }

  return 0;
}

// 0x0124_cxl
int16_t Service2EWriteBCallNum1(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 14)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_BCALL_NUM1, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }
  if (storeResult == 0)
  {
    ParameterSyncSdkSetToCpu(E_ParamId_BCallNumber, pData, dataLength);
  }

  return 0;
}

// 0xB20A_cxl
int16_t Service2EWriteOfficialServiceNum(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 14)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_OFFICIAL_SERVICE_NUM, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB20B_cxl
int16_t Service2EWriteEmergencyAsNum(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 14)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_EMERGENCY_AS_NUM, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0x0129_cxl
int16_t Service2EWriteTboxPhoneNum(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 14)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_TBOX_PHONE_NUM, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB20C_cxl
int16_t Service2EWriteTspFlowTimeout(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_TSP_FLOW_TIMEOUT, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB20D_cxl
int16_t Service2EWriteTspAckTime(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_TSP_ACK_TIME, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0x013C_cxl
int16_t Service2EWriteApn2(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 32)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_APN2, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB20E_cxl
int16_t Service2EWriteIp2Addr(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 16)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_IP2_ADDR, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB20F_cxl
int16_t Service2EWritePort2(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 8)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PORT2, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0x105F_cxl
int16_t Service2EWriteIp3Addr(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 16)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_IP3_ADDR, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB211_cxl
int16_t Service2EWriteLongConnCycle(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 8)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_LONG_CONN_CYCLE, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB229_cxl
int16_t Service2EWriteNormalUploadPeriod(uint8_t *pData, uint16_t len)
{
  if (len != 2)
    return 0x13;
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_NORMAL_UPLOAD_PERIOD, pData, len);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB22A_cxl
int16_t Service2EWriteAlarmUploadPeriod(uint8_t *pData, uint16_t len)
{
  if (len != 2)
    return 0x13;
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ALARM_UPLOAD_PERIOD, pData, len);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB22B_cxl
int16_t Service2EWriteTspDomainLen(uint8_t *pData, uint16_t len)
{
  if (len != 1)
    return 0x13;
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_TSP_DOMAIN_LEN, pData, len);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB22C_cxl
int16_t Service2EWriteTspDomain(uint8_t *pData, uint16_t len)
{
  if (len != 50)
    return 0x13;
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_TSP_DOMAIN, pData, len);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB22D_cxl
int16_t Service2EWriteTspPortNumeric(uint8_t *pData, uint16_t len)
{
  if (len != 2)
    return 0x13;
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_TSP_PORT_NUMERIC, pData, len);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0x1014_cxl
int16_t Service2EWritePublicTspDomain(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 50)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PUBLIC_TSP_DOMAIN, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB247_cxl
int16_t Service2EWriteApn1Type(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }

  if (pData[0] > 0x02)
  {
    return 0x31;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_APN1_TYPE, pData, 1);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0; // 成功
}
// 0xB248_cxl
int16_t Service2EWriteApn2Type(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }

  if (pData[0] > 0x02)
  {
    return 0x31;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_APN2_TYPE, pData, 1);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB258_cxl
int16_t Service2EWriteTspConnectSetting(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }

  if ((pData[0] & 0xFC) != 0)
  {
    return 0x31;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_TSP_CONNECT_SETTING, pData, 1);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB259_cxl
int16_t Service2EWriteEmmcDeleteControl(uint8_t *pData, uint16_t dataLength)
{
  // uint8_t command = pData[0];
  if (dataLength != 1)
  {
    return 0x13;
  }

  // if (command != 0x11 && command != 0x12)
  // {
  //   return 0x31;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB261_cxl
int16_t Service2EWritePkiApply(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }

  if (pData[0] > 0x01)
  {
    return 0x31;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PKI_APPLY_REQUEST, pData, 1);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  // if (storeResult == 0)
  // {
  //   ParameterSyncSdkSetToCpu(E_ParamId_PkiApplyRequest, pData, dataLength);
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0x031C_cxl
int16_t Service2EWriteTspDomain1(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 50)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_TSP_DOMAIN_1, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0cx1061_cxl
int16_t Service2EWriteApn3(uint8_t *pData, uint16_t dataLength)
{

  if (dataLength != 32)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_APN3, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB2CB_cxl   104F_cxl
int16_t Service2EWriteOtaDomainName(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 50)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_OTA_DOMAIN_NAME, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB2CC_cxl   1050_cxl
int16_t Service2EWriteOtaPort(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 8)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_OTA_PORT, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0x1052_cxl
int16_t Service2EWritePkiDomainName(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 50)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PKI_DOMAIN_NAME, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0x1053_cxl
int16_t Service2EWritePkiPort(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 8)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PKI_PORT, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
int16_t Service22ReadFingerprintEthernetF0FF(uint8_t *pData, uint16_t *pLength) // 指纹	ASCII	48
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_FINGERPRINT_ETHERNET_F0FF, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}
// 0x105E_cxl
int16_t Service2EWriteTspPort3(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 8)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_TSP_PORT_3, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB275
int16_t Service2EWriteLogDomainName(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 100)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_LOG_DOMAIN_NAME, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB276_cxl
int16_t Service2EWriteLogPort(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 8)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_LOG_PORT, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB277_cxl
int16_t Service2EWriteLogConnectionCommand(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 16)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_LOG_CONNECTION_COMMAND, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB212_cxl
int16_t Service2EWriteEthernetConfig(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 4)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ETHERNET_CONFIG, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB278_cxl
int16_t Service2EWritePkiCertCycle(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }
  if (pData[0] < 1 || pData[0] > 254)
  {
    return 0x31;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PKI_CERT_MAINTENANCE_CYCLE, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB27C_cxl
int16_t Service2EWriteLogAccount(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 50)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_LOG_ACCOUNT, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB27D_cxl
int16_t Service2EWriteLogPassword(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 50)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_LOG_PASSWORD, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB27F_cxl
int16_t Service2EWriteLogUploadChannel(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }

  if (pData[0] > 0x02)
  {
    return 0x31;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_LOG_UPLOAD_CHANNEL, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB280_cxl
// static uint8_t g_log_domain_buffer[100];
int16_t Service2EWriteLogDomainFront(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 50)
  {
    return 0x13;
  }

  // uint32_t length;
  // WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOG_DOMAIN_NAME, g_log_domain_buffer, &length);

  // memcpy(g_log_domain_buffer, pData, dataLength);
  // if (dataLength < 50)
  // {
  //   memset(g_log_domain_buffer + dataLength, 0x00, 50 - dataLength);
  // }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_LOG_DOMAIN_NAME, g_log_domain_buffer, 100);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB281_cxl
int16_t Service2EWriteLogDomainLater(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 50)
  {
    return 0x13;
  }
  // uint32_t length;
  // WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOG_DOMAIN_NAME, g_log_domain_buffer, &length);

  // memcpy(g_log_domain_buffer + 50, pData, dataLength);
  // if (dataLength < 50)
  // {
  //   memset(g_log_domain_buffer + 50 + dataLength, 0x00, 50 - dataLength);
  // }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_LOG_DOMAIN_NAME, g_log_domain_buffer, 100);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB282_cxl
// static uint8_t g_log_path_buffer[256];

// static void LoadLogPathBuffer(void)
//{
//   uint32_t length;
//   if (WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOG_PATH, g_log_path_buffer, &length) != 0 || length == 0)
//   {
//     memset(g_log_path_buffer, 0x00, 256);
//   }
// }

// static int16_t SaveLogPathBuffer(void)
//{
//   int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_LOG_PATH, g_log_path_buffer, 256);
//   if (storeResult < 0)
//   {
//     return 0x72;
//   }
//   if (storeResult == 0)
//   {
//     ParameterSyncSdkSetToCpu(E_ParamId_LogPath, g_log_path_buffer, 256);
//   }
//   return 0;
// }
int16_t Service22ReadLogPath(uint8_t *pData, uint16_t *pLength)
{
  // LoadLogPathBuffer();
  // memcpy(pData, g_log_path_buffer, 256);
  // *pLength = 256;
  return 0;
}

int16_t Service2EWriteLogPath(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 256)
  {
    return 0x13;
  }
  // memset(g_log_path_buffer, 0x00, 256);
  // memcpy(g_log_path_buffer, pData, dataLength);
  // return SaveLogPathBuffer();
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB283_cxl
int16_t Service22ReadLogPath_P1(uint8_t *pData, uint16_t *pLength)
{
  // LoadLogPathBuffer();
  // memcpy(pData, &g_log_path_buffer[0], 64);
  // *pLength = 64;
  return 0;
}

int16_t Service2EWriteLogPath_P1(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 64)
  {
    return 0x13;
  }
  // LoadLogPathBuffer();
  // memcpy(&g_log_path_buffer[0], pData, dataLength);
  // if (dataLength < 64)
  // {
  //   memset(&g_log_path_buffer[dataLength], 0x00, 64 - dataLength);
  // }
  // return SaveLogPathBuffer();
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB284_cxl
int16_t Service22ReadLogPath_P2(uint8_t *pData, uint16_t *pLength)
{
  // LoadLogPathBuffer();
  // memcpy(pData, &g_log_path_buffer[64], 64);
  // *pLength = 64;
  // return 0;
  // if (dataLength > 64)
  // {
  //   return 0x13;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

int16_t Service2EWriteLogPath_P2(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 64)
  {
    return 0x13;
  }
  // LoadLogPathBuffer();
  // memcpy(&g_log_path_buffer[64], pData, dataLength);
  // if (dataLength < 64)
  // {
  //   memset(&g_log_path_buffer[64 + dataLength], 0x00, 64 - dataLength);
  // }
  // return SaveLogPathBuffer();
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB285_cxl
int16_t Service22ReadLogPath_P3(uint8_t *pData, uint16_t *pLength)
{
  // LoadLogPathBuffer();
  // memcpy(pData, &g_log_path_buffer[128], 64);
  // *pLength = 64;
  return 0;
}

int16_t Service2EWriteLogPath_P3(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 64)
  {
    return 0x13;
  }
  // LoadLogPathBuffer();
  // memcpy(&g_log_path_buffer[128], pData, dataLength);
  // if (dataLength < 64)
  // {
  //   memset(&g_log_path_buffer[128 + dataLength], 0x00, 64 - dataLength);
  // }
  // return SaveLogPathBuffer();
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB286_cxl
int16_t Service22ReadLogPath_P4(uint8_t *pData, uint16_t *pLength)
{
  // LoadLogPathBuffer();
  // memcpy(pData, &g_log_path_buffer[192], 64);
  // *pLength = 64;
  return 0;
}

int16_t Service2EWriteLogPath_P4(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 64)
  {
    return 0x13;
  }
  // LoadLogPathBuffer();
  // memcpy(&g_log_path_buffer[192], pData, dataLength);
  // if (dataLength < 64)
  // {
  //   memset(&g_log_path_buffer[192 + dataLength], 0x00, 64 - dataLength);
  // }
  // return SaveLogPathBuffer();
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB28B_cxl
int16_t Service2EWriteApn3Type(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }

  if (pData[0] > 0x02)
  {
    return 0x31;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_APN3_TYPE, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }

  return 0;
}
// 0xB2C5_cxl
int16_t Service2EWriteSensitiveDataSet(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 16)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_SENSITIVE_DATA_SET, pData, dataLength);

  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB2C6_cxl
int16_t Service2EWriteOtaRtcWakeupTime(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 8)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_OTA_RTC_WAKEUP_TIME, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  // 解析BCD时间并设置硬件RTC唤醒定时器
  //  PowerManageSdkSetRtcWakeup(pData);

  return 0; // 返回成功
}
// 0xB2C7_cxl
int16_t Service2EWriteBuryingPointSwitch(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 4)
  {
    return 0x13;
  }

  if (pData[0] > 1)
  {
    return 0x31;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_BURYING_POINT_SWITCH, pData, dataLength);
  return 0;
}
// 0xB296_cxl
int16_t Service2EWriteAdbStatus(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }
//   if (pData[0] > 0x01)
//   {
//     return 0x31;
//   }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ADB_STATUS, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB297_cxl
int16_t Service2EWriteConnectionTestAddr(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 50)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_CONNECTION_TEST_ADDR, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB289_cxl
int16_t Service2EWriteDataBuriedPointDomain(uint8_t *pData, uint16_t dataLength)
{
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB29C_cxl
int16_t Service2EWriteDataBuriedPointDomain_P1(uint8_t *pData, uint16_t dataLength)
{
  // uint8_t full_buffer[128];
  // uint32_t length;
  // if (dataLength > 64)
  // {
  //   return 0x13;
  // }

  // WorkFlashVehicleInforRead(E_PARAMETER_INFO_DATA_BURIED_POINT_DOMAIN, full_buffer, &length);
  // if (length == 0)
  // {
  //   memset(full_buffer, 0x00, 128);
  // }

  // memcpy(&full_buffer[0], pData, dataLength);
  // if (dataLength < 64)
  // {
  //   memset(&full_buffer[dataLength], 0x00, 64 - dataLength);
  // }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_DATA_BURIED_POINT_DOMAIN, full_buffer, 128);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB29D_cxl
int16_t Service2EWriteDataBuriedPointDomain_P2(uint8_t *pData, uint16_t dataLength)
{
  // uint8_t full_buffer[128];
  // uint32_t length;
  // if (dataLength > 64)
  // {
  //   return 0x13;
  // }

  // WorkFlashVehicleInforRead(E_PARAMETER_INFO_DATA_BURIED_POINT_DOMAIN, full_buffer, &length);
  // if (length == 0)
  // {
  //   memset(full_buffer, 0x00, 128);
  // }

  // memcpy(&full_buffer[64], pData, dataLength);
  // if (dataLength < 64)
  // {
  //   memset(&full_buffer[64 + dataLength], 0x00, 64 - dataLength);
  // }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_DATA_BURIED_POINT_DOMAIN, full_buffer, 128);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB2C9_cxl
int16_t Service2EWriteGnssGalaxy(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_GNSS_GALAXY, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB2CA_cxl
int16_t Service2EWriteOtaBookingTime(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 6)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_OTA_BOOKING_TIME, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  // 调用硬件接口设置RTC唤醒
  //  PowerManageSdkSetRtcWakeup(pData);
  //  TriggerHardwareRtcWakeup(pData);
  return 0;
}
// 0xB2CD_cxl
int16_t Service2EWriteOtaSwAcqTime(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_OTA_SW_ACQ_TIME, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB2E5_cxl
int16_t Service2EWriteJtagStatus(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }
  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_JTAG_STATUS, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }

  // PeripheralHalSetJtag(pData[0]);

  return 0;
}
// 0xB2E6_cxl
int16_t Service2EWriteFunctionConfig(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 64)
  {
    return 0x13;
  }

  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_FUNCTION_CONFIG, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB2B4_cxl
int16_t Service2EWriteTransportMode(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }
  if (pData[0] > 0x01)
  {
    return 0x31;
  }

  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_TRANSPORT_MODE, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }
  if (pData[0] == 0x01) //使能运输模式
  {
    MpuHalPowerOff();; //运输模式下不使能模组
  }
  else
  {
    MpuHalStart();
  }

  return 0;
}
// 0xB25F_cxl
int16_t Service2EWrite4GModuleRsetTime(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }

  return 0;
}
// 0x0200_cxl
int16_t Service2EWriteReprogramCounter(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 2)
  {
    return 0x13;
  }

  return 0;
}
// 0x0201_cxl
int16_t Service2EWriteReprogramAttemptCounter(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 2)
  {
    return 0x13;
  }

  return 0;
}
// B2C8_cxl
int16_t Service2EWriteTcpdumpSwitch(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 3)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// B28E_cxl
int16_t Service2EWriteTspPlatformConnectionStatus(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB261_cxl
int16_t Service2EWritePKIapply(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0xB25C_cxl
int16_t Service2ETboxCurrentRunningMode(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 1)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}
// 0x0110_cxl
int16_t Service2EWriteManufactoryMode(uint8_t *pData, uint16_t dataLength)
{
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  if (dataLength != 1)
  {
    return 0x13;
  }

  uint8_t currentMode;
  uint32_t currentLength = 0;

  WorkFlashVehicleInforRead(E_PARAMETER_INFO_MANUFACTORY_MODE, &currentMode, &currentLength);

  if (currentLength > 0 && currentMode == 0x00)
  {
    return 0x22; // NRC 22: ConditionsNotCorrect
  }

  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_MANUFACTORY_MODE, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }

  uint8_t newMode = pData[0];
  if (newMode >= 0x10) // 工厂模式激活
  {
    // 进入工厂模式，禁用远程诊断
    RemoteDiagnosticSdkDisable();
  }
  else // 工厂模式未激活 (0x00 ~ 0x0F)
  {
    // 退出工厂模式，恢复远程诊断
    RemoteDiagnosticSdkRecover();
  }

  return 0; // 返回0表示成功
}
// 0x0120_cxl
int16_t Service2EWriteDtcSettingControl(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 12)
  {
    return 0x13;
  }

  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_DTC_SETTING_CONTROL, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }

  DtcReloadSettingControl(); 

  return 0; 
}

#endif
#if 0
int16_t Service22ReadVehicleModelF112(uint8_t *pData, uint16_t *pLength) // 车辆 	ASCII	20
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_VEHICLE_MODEL_F112, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadVehicleNetworkConfigurationF1A1(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_VEHICLE_NETWORK_CONFIGURATION_F1A1, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadCANmatrixVersionF1A2(uint8_t *pData, uint16_t *pLength)
{
  memcpy(pData, g_didF1A2, sizeof(g_didF1A2));
  *pLength = sizeof(g_didF1A2);

  return 0;
}

int16_t Service22ReadVehicleSoftwareVersionF1AB(uint8_t *pData, uint16_t *pLength) // 整车软件版本 10
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_VEHICLE_SOFEWARE_VERSION_F1AB, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadVehicleNetworkConfigurationF1B0(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_VEHICLE_NETWORK_CONFIGURATION_F1B0, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadECUlevelNetworkConfigurationDataIdentifierF1B1(uint8_t *pData, uint16_t *pLength) // 整车网络配置	HEX	31
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_LEVEL_NETWORK_CONFIGURATION_DATA_IDENTIFIER_F1B1, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadIdentifierF1B2(uint8_t *pData, uint16_t *pLength)
{
  memcpy(pData, g_didF1B3, sizeof(g_didF1B3));
  *pLength = sizeof(g_didF1B3);

  return 0;
}

int16_t Service22ReadNodeAddressF1B3(uint8_t *pData, uint16_t *pLength)
{
  memcpy(pData, g_didF1B3, sizeof(g_didF1B3));
  *pLength = sizeof(g_didF1B3);

  return 0;
}

int16_t Service22ReadCompID1F1B4(uint8_t *pData, uint16_t *pLength)
{
  memcpy(pData, g_didF1B4, sizeof(g_didF1B4));
  *pLength = sizeof(g_didF1B4);

  return 0;
}

int16_t Service22ReadNameF1B5(uint8_t *pData, uint16_t *pLength)
{
  memcpy(pData, g_didF1B5, sizeof(g_didF1B5));
  *pLength = sizeof(g_didF1B5);

  return 0;
}

int16_t Service22ReadToolVersionF1B6(uint8_t *pData, uint16_t *pLength) // 工具版本	ASCII	8
{
  memcpy(pData, g_didF1B6, sizeof(g_didF1B6));
  *pLength = sizeof(g_didF1B6);
  return 0;
}

int16_t Service22ReadCompID2F1B9(uint8_t *pData, uint16_t *pLength)
{
  memcpy(pData, g_didF1B9, sizeof(g_didF1B9));
  *pLength = sizeof(g_didF1B9);

  return 0;
}

int16_t Service22ReadSoftwareInformationF1BC(uint8_t *pData, uint16_t *pLength) // 软件信息 10
{
  memcpy(pData, g_didF1BC, sizeof(g_didF1BC));
  *pLength = sizeof(g_didF1BC);
  return 0;
}

int16_t Service22ReadHardwareInformationF1BD(uint8_t *pData, uint16_t *pLength) // Hardware Information 10
{
  memcpy(pData, g_didF1BD, sizeof(g_didF1BD));
  *pLength = sizeof(g_didF1BD);
  return 0;
}

int16_t Service22ReadWorkingModeF1C2(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_WORKING_MODE_F1C2, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadUINF1C5(uint8_t *pData, uint16_t *pLength) // 终端安全序列号	ASCII	20
{
  // int16_t ret;
  uint32_t dataLength;

  uint8_t i, value;
  //    if(GetSyncParameterFromCpuIsFinished()!=0)//判断是否
  //    {
  //        return -1;
  //    }

  WorkFlashVehicleInforRead(E_PARAMETER_INFO_UIN, pData, &dataLength);
  // GetUIN(pData,&length);
  if (dataLength > 20)
  {
    return -1;
  }
  value = 0;
  // TBOX_PRINT("%x\n",pData[0]);
  for (i = 0; i < dataLength; i++)
  {
    if ((pData[i] != 0) && (pData[i] != 0xFF))
    {
      value = 1;
      break;
    }
  }
  if (0 == value)
  {
    return -1;
  }
  *pLength = dataLength;
  return 0;
}

int16_t Service22ReadSecocVersion(uint8_t *pData, uint16_t *pLength)
{
  // uint8_t len;
  // SecOcGetVersion(pData,&len);
  *pLength = 20; // len;
  return 0;
}

int16_t Service22ReadCalibrationSoftwareVersionF1C0(uint8_t *pData, uint16_t *pLength)
{
  memcpy(pData, g_didF1C0, sizeof(g_didF1C0));
  *pLength = sizeof(g_didF1C0);

  return 0;
}

int16_t Service22ReadSecOCVersionNumberF1C8(uint8_t *pData, uint16_t *pLength) // SecOC版本号 1
{
  memcpy(pData, g_didF1C8, sizeof(g_didF1C8));
  *pLength = sizeof(g_didF1C8);
  return 0;
}

int16_t Service22ReadCsrF1C9(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 840;
  int16_t ret;
  // ret = WorkFlashVehicleInforRead(WORKFLASH_ADDRESS_RESERVED4,pData,&length);
  ret = FlashHalDataBlockRead(WORKFLASH_ADDRESS_RESERVED4, 0, pData, length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadCertificateSerialNumberF1CC(uint8_t *pData, uint16_t *pLength)
{
  // int16_t ret;

  // if(GetSyncParameterFromCpuIsFinished()!=0)
  // {
  //   return -1;
  // }
  // ret = McuGetDataFromCpu(1,pData,pLength);
  // if(ret!=0)
  // {
  //   return -1;
  // }
  return 0;
}

int16_t Service22ReadECUTypeF1D9(uint8_t *pData, uint16_t *pLength) // ECUType 1
{
  uint8_t arry[1] = {0};
  arry[0] = 0x01;
  memcpy(pData, arry, sizeof(arry));
  *pLength = sizeof(arry);
  return 0;
}

int16_t Service22ReadFingerprintF1F0(uint8_t *pData, uint16_t *pLength) // 指纹	ASCII	26
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_FINGERPRINT_F1F0, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadMaintenanceModeFD00(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_MAINTENANCE_MODE_FD00, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadGPSRNetworkStatus600D(uint8_t *pData, uint16_t *pLength) // 4G网络联网状态
{
  NetInfoSync_t netInfo;
  if (StateSyncGetNetInfo(&netInfo) != 0) // 判断是否
  {
    return -1;
  }
  *pLength = 1;
  pData[0] = netInfo.creg;
  return 0;
}

int16_t Service22ReadGPSRNetworkSignalStrength600E(uint8_t *pData, uint16_t *pLength) // 4G网络信号强度
{
  NetInfoSync_t netInfo;
  if (StateSyncGetNetInfo(&netInfo) != 0) // 判断是否
  {
    return -1;
  }
  *pLength = 1;
  pData[0] = netInfo.csq;
  return 0;
}

int16_t Service22ReadSK(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_SK, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadSecocKeyCC77(uint8_t *pData, uint16_t *pLength)
{
  uint8_t tem[32];
  uint32_t len;
  uint8_t i;
  WorkFlashVehicleInforRead(E_PARAMETER_INFO_SECOC_KEY_F1C7, tem, &len);
  i = 0;
  for (; i < 7; i++)
  {
    pData[i] = tem[5 + i];
  }
  for (; i < 16; i++)
  {
    pData[i] = tem[15 + i];
  }
  *pLength = 16;
  return 0;
}

int16_t Service22ReadPIN(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_PIN, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service22ReadTripRestAA11(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t TripCount,RestCount;
  // *pLength = 8;
  // TripCount = GetTripCount();
  // RestCount = GetRestCount();
  // memcpy(&pData[0],(uint8_t *)&TripCount,4);
  // memcpy(&pData[4],(uint8_t *)&RestCount,4);
  return 0;
}

int16_t Service22ReadEmmcStateAA01(uint8_t *pData, uint16_t *pLength) // 获取EMMC状态
{
  // int16_t ret = 0;
  // ret = GetEmmcState();
  //  if(ret == 0)
  //  {
  //      return -1;
  //  }
  // pData[0] = ret;
  pData[0] = 1;
  *pLength = 1;
  return 0;
}

// 0xF10B_cxl
int16_t Service22ReadGacDiagParamVersion(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetGacDiagParamVersion_F10B(pData, pLength);
  return 0;
}

// 0xF17F_cxl
int16_t Service22ReadGacSparePartNumber(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetGacSparePartNumber_F17F(pData, pLength);
  return 0;
}

// 0xF180_cxl
int16_t Service22ReadBootSoftwarePartNumber(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetBootSwPartNumber_F180(pData, pLength);
  return 0;
}

// 0xF187_cxl
int16_t Service22ReadGacEcuPartNumber(uint8_t *pData, uint16_t *pLength)
{
  //ProjectConfigGetGacEcuPartNumber_F187(pData, pLength);
  uint32_t len;
  WorkFlashVehicleInforRead(E_PARAMETER_INFO_PART_NAME, pData, &len);
  *pLength = len;
  return 0;
}
// // 0xF187_cxl
// int16_t Service22ReadGacEcuPartNumber(uint8_t *pData, uint16_t *pLength)
// {
//     // ProjectConfigGetGacEcuPartNumber_F187(pData, pLength);
//     ProjectConfigGetGacEcuPartNumber_F187(pData, pLength);
//     return 0;
// }
// 0xF187_cxl
int16_t Service2EWriteGacEcuPartNumber(uint8_t *pData, uint16_t dataLength)
{
    if (dataLength != 14)
    {
        return 0x13;
    }

    int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_PART_NAME, pData, dataLength);
    if (storeResult < 0)
    {
        return 0x72;
    }
    if (storeResult == 0)
    {
      ParameterSyncSdkSetToCpu(E_ParamId_ParatNumber, pData, dataLength);
    }
    return 0;
}
// 0xF189_cxl
int16_t Service22ReadVehicleManufacturerECUSoftwareVersionNumberDataIdentifierF189(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetEcuSwVersion_F189(pData, pLength);
  return 0;
}

// 0xF18E_cxl
int16_t Service22ReadGacVisteonInHousePartNumber(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetVisteonPartNumber_F18E(pData, pLength);
  return 0;
}

// 0xF193_cxl
int16_t Service22ReadSupplierECUHardwareVersion(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetSupplierHdVersion_F193(pData, pLength);
  return 0;
}
int16_t Service22WriteSupplierECUHardwareVersion(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 20)
  {
    return 0x13;
  }

  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ECU_HARDWARE_VERSION, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }
  // if (storeResult == 0)
  // {
  //   ParameterSyncSdkSetToCpu(E_ParamId_EcuHardwareVersion, pData, dataLength);
  //  }
  return 0;
}

// 0xF195_cxl
int16_t Service22ReadSupplierECUSoftwareVersion(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetSupplierSwVersion_F195(pData, pLength);
  return 0;
}

// 0xF18A_cxl
int16_t Service22ReadSupplierId(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetSupplierId_F18A(pData, pLength);
  return 0;
}

// 0x1000_cxl
int16_t Service22ReadPowerVoltage(uint8_t *pData, uint16_t *pLength)
{
  uint32_t voltage_mv = 0;
  float physical_voltage = 0.0;
  uint8_t raw_value = 0;

  // KL30的电压 毫伏
  PeripheralHalAdGet(AD_CHANNEL_KL30, &voltage_mv);

  physical_voltage = (float)voltage_mv / 1000.0f; // 12300mV -> 12.3V

  raw_value = (uint8_t)(physical_voltage / 0.1f); // 12.3V / 0.1 -> 123

  pData[0] = raw_value;
  *pLength = 1;

  return 0;
}
// 0xB201_cxl
int16_t Service22Read4G3GStatus(uint8_t *pData, uint16_t *pLength)
{
    const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();
    LteModuleStatus_t status;

    if (ftyData != NULL)
    {
        status = ftyData->lteModuleStatus;
    }
    else
    {
        return 0x22;
    }

    uint16_t value = 0;

    value |= (status.powerMode & 0x0F) << 0;
    value |= (status.dtrReady  & 0x01) << 4;
    value |= (status.dsrReady  & 0x01) << 5;
    value |= (status.reqSend   & 0x01) << 6;
    value |= (status.clrSend   & 0x01) << 7;

    value |= (status.rsv8_15 & 0xFF) << 8;

    pData[0] = value & 0xFF;
    pData[1] = (value >> 8) & 0xFF;
    *pLength = 2;
    return 0;
}


// 0xB202_cxl
int16_t Service22ReadGpsStatus(uint8_t *pData, uint16_t *pLength)
{
    const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();
    GnssModuleStatus_t status;

    if (ftyData != NULL)
    {
        status = ftyData->gnssModuleStatus;
    }
    else
    {
        memset(&status, 0xFF, sizeof(status));
    }

    uint16_t value = 0;
    value |= (status.gnssEnableState   & 0x1) << 0;
    value |= (status.modeMixed         & 0x1) << 1;
    value |= (status.beidouEnable      & 0x1) << 2;
    value |= (status.gpsEnable         & 0x1) << 3;
    value |= (status.glonassEnable     & 0x1) << 4;
    value |= (status.fusionFront       & 0x1) << 5;
    value |= (status.agpsEnable        & 0x1) << 6;
    value |= (status.protoCustom       & 0x1) << 7;
    value |= (status.outComposite      & 0x1) << 8;
    value |= (status.rsv9              & 0x1) << 9;
    value |= (status.outGpsInfo        & 0x1) << 10;
    value |= (status.outGroundSpeed    & 0x1) << 11;
    value |= (status.outCurSatInfo     & 0x1) << 12;
    value |= (status.outVisibleSat     & 0x1) << 13;
    value |= (status.outRecommended    & 0x1) << 14;
    value |= (status.outGeoPos         & 0x1) << 15;

    pData[0] = (value & 0xFF);
    pData[1] = (value >> 8) & 0xFF;
    

    *pLength = 2;

    return 0;
}


// 0xB203_cxl
int16_t Service22ReadSystemTime(uint8_t *pData, uint16_t *pLength)
{
  const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();

  if (ftyData != NULL)
  {
    const TimeData_t *sourceTime = &ftyData->timeData;
    uint8_t *p_buf = pData;


    *p_buf++ = (uint8_t)((sourceTime->year >> 8) & 0xFF);

    *p_buf++ = (uint8_t)(sourceTime->year & 0xFF);

    *p_buf++ = sourceTime->month;
    *p_buf++ = sourceTime->day;
    *p_buf++ = sourceTime->hour;
    *p_buf++ = sourceTime->minute;
    *p_buf++ = sourceTime->second;
  }
  else
  {
    return 0x22;
  }

  *pLength = 7;

  return 0;
}

// 0xB204_cxl
int16_t Service22ReadBcallEcallKeyStatus(uint8_t *pData, uint16_t *pLength)
{
  uint8_t byte0 = 0;

  // E-Call
  if (EcallHalGetSosButtonStatus() == 1) // 1按下
  {
    byte0 |= (1 << 0);
  }

  // B-Call
  // if (EcallHalGetBcallKeyStatus() == 1) // 1按下 没有HAL层函数 暂时代替
  // {
  //   byte0 |= (1 << 1);
  // }

  // Bits 2-7 保留位

  pData[0] = byte0;
  *pLength = 1;

  return 0;
}

// 0xB205_cxl
int16_t Service22ReadVehicleSpeed(uint8_t *pData, uint16_t *pLength)
{
  double physical_speed_kph = 0.0;
  uint16_t raw_value = 0;
  const can0_signal_configure_t *pCan0SignalConfigure = GetCan0SignalConfigure();

  // 获取车速物理值km/h
  CanParseSdkReadSignal(VEHICLE_CAN_UNPACK_FORMAT_INTEL, &pCan0SignalConfigure->BCS_VehSpd, &physical_speed_kph);

  // Raw Value
  raw_value = (uint16_t)(physical_speed_kph / 0.05625f);

  // 大端模式
  pData[0] = (raw_value >> 8) & 0xFF;
  pData[1] = raw_value & 0xFF;

  *pLength = 2;

  return 0; // 成功
}
// 0xB206_cxl
int16_t Service22ReadEngineMotorSpeed(uint8_t *pData, uint16_t *pLength)
{
  double physical_speed_rpm = 0.0;
  uint16_t raw_value = 0;
  const can0_signal_configure_t *pCan0SignalConfigure = GetCan0SignalConfigure();

  CanParseSdkReadSignal(VEHICLE_CAN_UNPACK_FORMAT_INTEL, &pCan0SignalConfigure->EMS_EngSpd, &physical_speed_rpm);

  raw_value = (uint16_t)physical_speed_rpm;

  pData[0] = (raw_value >> 8) & 0xFF;
  pData[1] = raw_value & 0xFF;

  *pLength = 2;

  return 0;
}
// 0x010D_cxl
int16_t Service22ReadESKey(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ESK_KEY, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

// 0x0204_cxl
int16_t Service22ReadESKWrittenStatus(uint8_t *pData, uint16_t *pLength)
{
  if (g_eskKeyValid == 1)
  {
    pData[0] = 0x01;
  }
  else
  {
    pData[0] = 0x00;
  }

  *pLength = 1;

  return 0;
}

// 0x010E_cxl
int16_t Service22ReadIMEI(uint8_t *pData, uint16_t *pLength)
{ 
  uint32_t len32 = 0;
  WorkFlashVehicleInforRead(E_PARAMETER_INFO_IMEI, pData, &len32);

  *pLength = (uint16_t)len32;
  return 0;
}
int16_t Service2EWriteIMEI(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 20)
  {
    return 0x13;
  }
  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_IMEI, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }
  if (storeResult == 0)
  {
    ParameterSyncSdkSetToCpu(E_ParamId_IMEI, pData, dataLength);
  }
  return 0;
}
// 0x010F_cxl
int16_t Service22ReadIMSI(uint8_t *pData, uint16_t *pLength)
{
  uint32_t len32 = 0;

  WorkFlashVehicleInforRead(E_PARAMETER_INFO_IMSI, pData, &len32);

  *pLength = (uint16_t)len32;
  return 0;
}

int16_t Service2EWriteIMSI(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 20)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_IMSI, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }
  if (storeResult == 0)
  {
    ParameterSyncSdkSetToCpu(E_ParamId_IMSI, pData, dataLength);
  }
  return 0;
}
// 0xB208_cxl
int16_t Service2EWriteRtcWakeupSet(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength != 2)
  {
    return 0x13;
  }
  // int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_RTC_WAKEUP, pData, dataLength);
  // if (storeResult < 0)
  // {
  //   return 0x72;
  // }
  // 调用硬件接口设置RTC唤醒
  // PowerManageSdkSetRtcWakeup(pData);
  // TriggerHardwareRtcWakeup(pData);
  return 0;
}
// 0x0111_cxl
int16_t Service22ReadICCID(uint8_t *pData, uint16_t *pLength)
{
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  uint32_t len32 = 0;
  WorkFlashVehicleInforRead(E_PARAMETER_INFO_ICCID, pData, &len32);
  *pLength = (uint16_t)len32;
  return 0;
}
// 0x0111_cxl
int16_t Service2EWriteICCID(uint8_t *pData, uint16_t dataLength)
{
  if (dataLength > 20)
  {
    return 0x13;
  }
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  int16_t storeResult = WorkFlashVehicleInforStore(E_PARAMETER_INFO_ICCID, pData, dataLength);
  if (storeResult < 0)
  {
    return 0x72;
  }
  if (storeResult == 0)
  {
    ParameterSyncSdkSetToCpu(E_ParamId_ICCID, pData, dataLength);
  }
  return 0;
}

// 0x011B_cxl
int16_t Service22ReadApn1(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret;

  // ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_APN1, pData, &length);

  // if (ret != 0 || length == 0)
  // {
  //   memset(pData, 0x00, 32);
  // }

  // *pLength = 32;
  return 0;
}

// 0x011C_cxl
int16_t Service22ReadIp1Addr(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;

  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_IP1_ADDR, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 16);
  }

  *pLength = 16;
  return 0;
}

// 0x011D_cxl
int16_t Service22ReadTspPort(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;

  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_PORT1, pData, &length);

  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 8);
    pData[6] = 0x22;
    pData[7] = 0xB8;
  }

  *pLength = 8;
  return 0;
}

// 0xB209_cxl
int16_t Service22ReadTspTimeout(uint8_t *pData, uint16_t *pLength)
{
   uint32_t length;
   int16_t ret;

   ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_TSP_TIMEOUT, pData, &length);

   if (ret != 0 || length == 0)
   {
     pData[0] = 0x05;
   }

   *pLength = 1;
   return 0;
}

// 0x011F_cxl
int16_t Service22ReadECallNum1(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;

  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECALL_NUM1, pData, &length);

  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 14);
  }

  *pLength = 14;
  return 0;
}

// 0x0124_cxl
int16_t Service22ReadBCallNum1(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;

  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_BCALL_NUM1, pData, &length);

  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 14);
  }

  *pLength = 14;
  return 0;
}

// 0xB20A_cxl
int16_t Service22ReadOfficialServiceNum(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_OFFICIAL_SERVICE_NUM, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 14);
  }
  *pLength = 14;
  return 0;
}

// 0xB20B_cxl
int16_t Service22ReadEmergencyAsNum(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_EMERGENCY_AS_NUM, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 14);
  }
  *pLength = 14;
  return 0;
}

// 0x0129_cxl
int16_t Service22ReadTboxPhoneNum(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_TBOX_PHONE_NUM, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 14);
  }
  *pLength = 14;
  return 0;
}

// 0xB20C_cxl
int16_t Service22ReadTspFlowTimeout(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_TSP_FLOW_TIMEOUT, pData, &length);
  if (ret != 0 || length == 0)
  {
    pData[0] = 0x28;
  }
  *pLength = 1;
  return 0;
}

// 0xB20D_cxl
int16_t Service22ReadTspAckTime(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_TSP_ACK_TIME, pData, &length);
  if (ret != 0 || length == 0)
  {
    pData[0] = 0x28;
  }
  *pLength = 1;
  return 0;
}

// 0x013C_cxl
int16_t Service22ReadApn2(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_APN2, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 32);
  }
  *pLength = 32;
  return 0;
}

// 0xB20E_cxl
int16_t Service22ReadIp2Addr(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_IP2_ADDR, pData, &length);
  if (ret != 0 || length == 0)
  {
    // 默认值: "218.205.176.44"
    const char *defaultIp = "218.205.176.44";
    memset(pData, 0x00, 16);
    memcpy(pData, defaultIp, strlen(defaultIp));
  }
  *pLength = 16;
  return 0;
}

// 0xB20F_cxl
int16_t Service22ReadPort2(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_PORT2, pData, &length);
  if (ret != 0 || length == 0)
  {
    // 默认值: 19006 (0x4A3E)
    memset(pData, 0x00, 8);
    pData[6] = 0x4A;
    pData[7] = 0x3E;
  }
  *pLength = 8;
  return 0;
}

// 0xB210_cxl
int16_t Service22Read4GNetworkStatus(uint8_t *pData, uint16_t *pLength)
{
    const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();
    LteNetworkStatus_t status;

    if (ftyData != NULL)
    {
        status = ftyData->lteNetworkStatus;
    }
    else
    {
        return 0x22;
    }

    pData[0] = status.rssi;
    pData[1] = status.ber;

    pData[2] =
        ((status.b2_3.operatorType & 0x03) << 0) |
        ((status.b2_3.rat          & 0x07) << 2) |
        ((status.b2_3.netState     & 0x07) << 5);
    pData[3] = status.b2_3.faultCode & 0xFF;

    *pLength = 4;
    return 0;
}

// 0x105F_cxl
int16_t Service22ReadIp3Addr(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_IP3_ADDR, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 16);
  }
  *pLength = 16;
  return 0;
}

// 0xB211_cxl
int16_t Service22ReadLongConnCycle(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // uint8_t storedValue[2];

  // int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_LONG_CONN_CYCLE, storedValue, &length);

  // memset(pData, 0x00, 8);

  // if (ret != 0 || length == 0)
  // {

  //   pData[0] = 0x0E;
  //   pData[1] = 0x10;
  // }
  // else
  // {

  //   pData[0] = storedValue[0];
  //   pData[1] = storedValue[1];
  // }

  // *pLength = 8;
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  return 0;
}

// 0xB229_cxl
int16_t Service22ReadNormalUploadPeriod(uint8_t *pData, uint16_t *pLength)
{
  uint32_t len;
  if (WorkFlashVehicleInforRead(E_PARAMETER_INFO_NORMAL_UPLOAD_PERIOD, pData, &len) != 0 || len == 0)
  {
    pData[0] = 0x00;
    pData[1] = 0x0A;
  }
  *pLength = 2;
  return 0;
}

// 0xB22A_cxl
int16_t Service22ReadAlarmUploadPeriod(uint8_t *pData, uint16_t *pLength)
{
  uint32_t len;
  if (WorkFlashVehicleInforRead(E_PARAMETER_INFO_ALARM_UPLOAD_PERIOD, pData, &len) != 0 || len == 0)
  {
    pData[0] = 0x03;
    pData[1] = 0xEB;
  }
  *pLength = 2;
  return 0;
}

// 0xB22B_cxl
int16_t Service22ReadTspDomainLen(uint8_t *pData, uint16_t *pLength)
{
  uint32_t len;
  if (WorkFlashVehicleInforRead(E_PARAMETER_INFO_TSP_DOMAIN_LEN, pData, &len) != 0 || len == 0)
  {
    pData[0] = 0x32;
  }
  *pLength = 1;
  return 0;
}

// 0xB22C_cxl
int16_t Service22ReadTspDomain(uint8_t *pData, uint16_t *pLength)
{
  uint32_t len;
  if (WorkFlashVehicleInforRead(E_PARAMETER_INFO_TSP_DOMAIN, pData, &len) != 0 || len == 0)
  {
    memset(pData, 0x00, 50);
  }
  *pLength = 50;
  return 0;
}

// 0xB22D_cxl
int16_t Service22ReadTspPortNumeric(uint8_t *pData, uint16_t *pLength)
{
  uint32_t len;
  if (WorkFlashVehicleInforRead(E_PARAMETER_INFO_TSP_PORT_NUMERIC, pData, &len) != 0 || len == 0)
  {
    pData[0] = 0x00;
    pData[1] = 0x00;
  }
  *pLength = 2;
  return 0;
}

// 0x1014_cxl
int16_t Service22ReadPublicTspDomain(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_PUBLIC_TSP_DOMAIN, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 50);
  }
  *pLength = 50;
  return 0;
}

int16_t Service22ReadTboxWakeupSource(uint8_t *pData, uint16_t *pLength)
{
  uint8_t wakeupSource = PowerManageGetLastWakeupSource();
  uint16_t wakeupBits = 0;

  switch (wakeupSource)
  {
  case PM_HAL_WAKEUP_SOURCE_RTC:
    wakeupBits |= (1 << 2); // Bit2: RTC唤醒
    break;

  case PM_HAL_WAKEUP_SOURCE_CAN1:
  case PM_HAL_WAKEUP_SOURCE_CAN2:
  case PM_HAL_WAKEUP_SOURCE_CAN3:
  case PM_HAL_WAKEUP_SOURCE_CAN4:
  case PM_HAL_WAKEUP_SOURCE_CAN5:
  case PM_HAL_WAKEUP_SOURCE_CAN6:
    wakeupBits |= (1 << 3); // Bit3: NM唤醒 (CAN唤醒)
    break;

  case PM_HAL_WAKEUP_SOURCE_KL15:
    wakeupBits |= (1 << 4); // Bit4: IG唤醒 (Ignition)
    break;

  case PM_HAL_WAKEUP_SOURCE_KL30:
    wakeupBits |= (1 << 5); // Bit5: 后备电池唤醒 (KL30上电唤醒)
    break;

  case PM_HAL_WAKEUP_SOURCE_MPU:
    // (如远程指令、电话、短信)，
    wakeupBits |= (1 << 0); // Bit0: Ringing awaken
    wakeupBits |= (1 << 1); // Bit1: SMS awaken
    wakeupBits |= (1 << 6); // Bit6: wds远程指令唤醒
    break;

  default:
    break;
  }

  pData[0] = (uint8_t)(wakeupBits & 0xFF);
  pData[1] = (uint8_t)((wakeupBits >> 8) & 0xFF);

  *pLength = 2;

  return 0;
}

// 0xB247_cxl
int16_t Service22ReadApn1Type(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 0;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_APN1_TYPE, pData, &length);

  if (ret != 0 || length == 0)
  {
    pData[0] = 0x01;
  }

  *pLength = 1;
  return 0;
}

// 0xB248_cxl
int16_t Service22ReadApn2Type(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 0;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_APN2_TYPE, pData, &length);

  if (ret != 0 || length == 0)
  {
    pData[0] = 0x00;
  }

  *pLength = 1;
  return 0;
}
// 0xB249_cxl
int16_t Service22ReadGpsSatelliteInfo(uint8_t *pData, uint16_t *pLength)
{
  const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();
  uint8_t *p_buf = pData;
  int i;                  

  if (ftyData != NULL)
  {
    const GpsSatInfo_1to4_t *sourceInfo = &ftyData->gpsSatInfo1to4;

    *p_buf++ = sourceInfo->visibleCount;

    for (i = 0; i < 4; i++)
    {
      *p_buf++ = sourceInfo->sat[i].id;
      *p_buf++ = sourceInfo->sat[i].elevation;

      *p_buf++ = (uint8_t)((sourceInfo->sat[i].azimuth >> 8) & 0xFF);
      *p_buf++ = (uint8_t)(sourceInfo->sat[i].azimuth & 0xFF);

      *p_buf++ = sourceInfo->sat[i].cnr;
    }

    for (i = 0; i < 3; i++)
    {
      *p_buf++ = sourceInfo->pdop[i];
    }

    for (i = 0; i < 3; i++)
    {
      *p_buf++ = sourceInfo->hdop[i];
    }

    for (i = 0; i < 3; i++)
    {
      *p_buf++ = sourceInfo->vdop[i];
    }
  }
  else
  {
    return 0x22;
  }
  *pLength = 30;

  return 0;
}
// 0xB24A_cxl
int16_t Service22ReadEmmcState(uint8_t *pData, uint16_t *pLength)
{
    const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();
    uint8_t emmcState = 0;

    if (ftyData != NULL)
    {
        uint8_t storeState = (uint8_t)(ftyData->dtc.storeState);
        TBOX_PRINT("emmc store state: %d\r\n", storeState);
        if (storeState == 1)
        {
            emmcState = 0;
        }
        else
        {
            emmcState = 1;
        }
    }
    else
    {
        return 0x22;
    }

    pData[0] = emmcState;
    *pLength = 1;

    return 0;
}
// 0xB24C_cxl
int16_t Service22Read4gFirmwareVersion(uint8_t *pData, uint16_t *pLength)
{
  // ModuleInfo4GPayload_t moduleInfo;
  // int16_t ret;

  // *pLength = 20;
  // memset(pData, 0x00, *pLength);
  // ret = StateSyncGet4gModuleInfo(&moduleInfo);
  // if (ret == 0)
  // {
  //   memcpy(pData, moduleInfo.fw_version, *pLength);
  // }

  return 0;
}
// 0xB24D_cxl
int16_t Service22Read4gHardwareVersion(uint8_t *pData, uint16_t *pLength)
{
  ModuleInfo4GPayload_t moduleInfo;
  int16_t ret;

  *pLength = 20;
  memset(pData, 0x00, *pLength);

  ret = StateSyncGet4gModuleInfo(&moduleInfo);
  if (ret == 0)
  {
    memcpy(pData, moduleInfo.hw_version, *pLength);
  }

  return 0;
}
// 0xB24E_cxl
int16_t Service22Read4gFunctionState(uint8_t *pData, uint16_t *pLength)
{
    const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();
    //uint16_t statusValue = 0;
    uint8_t byte0 = 0;
    uint8_t byte1 = 0;

    if (ftyData != NULL)
    {
        const LteFunctionStatus_t *status = &ftyData->lteFunctionStatus;

        byte0  = (status->uartMainEn      & 0x01) << 0;
        byte0 |= (status->spiEn           & 0x01) << 1;
        byte0 |= (status->tcpEn           & 0x01) << 2;
        byte0 |= (status->usbModeEn       & 0x01) << 3;
        byte0 |= (status->netHeartbeatEn  & 0x01) << 4;
        byte0 |= (status->lteGpsEn        & 0x01) << 5;
        byte0 |= (status->i2sDigitalAuEn  & 0x01) << 6;
        byte0 |= (status->agpsEn          & 0x01) << 7;

        byte1  = (status->cellLocateEn    & 0x01) << 0;
        byte1 |= (status->auxUartConsole  & 0x01) << 1;
        byte1 |= (status->usbModemEn      & 0x01) << 2;
        byte1 |= (status->usbDiagEn       & 0x01) << 3;
        byte1 |= (status->usbAuxEn        & 0x01) << 4;
        byte1 |= (status->uartDataEn      & 0x01) << 5;
        byte1 |= (status->uartTraceEn     & 0x01) << 6;
        byte1 |= (status->wlanEn          & 0x01) << 7;
    }
    else
    {
        return 0x22;
    }

    pData[0] = byte0;
    pData[1] = byte1;
    *pLength = 2;

    return 0;
}

// 0xB256_cxl
int16_t Service22ReadEthernetId(uint8_t *pData, uint16_t *pLength)
{
  // EthernetIdPayload_t ethId;
  // int16_t ret;

  // *pLength = 5;
  // memset(pData, 0x00, *pLength);

  // ret = StateSyncGetEthernetId(&ethId);
  // if (ret == 0)
  // {
  //   memcpy(pData, ethId.id, *pLength);
  // }

  return 0;
}
// 0xB258_cxl
int16_t Service22ReadTspConnectSetting(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 0;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_TSP_CONNECT_SETTING, pData, &length);

  if (ret != 0 || length == 0)
  {
    pData[0] = 0x03;
  }

  *pLength = 1;
  return 0;
}
// 0xB259_cxl
int16_t Service22ReadEmmcDeleteStatus(uint8_t *pData, uint16_t *pLength)
{
  EmmcDeleteStatusPayload_t deleteStatus;
  int16_t ret;

  *pLength = 1;

  ret = StateSyncGetEmmcDeleteStatus(&deleteStatus);
  if (ret == 0)
  {
    // 成功获取到MPU同步过来的状态
    pData[0] = deleteStatus.status;
  }
  else
  {
    // 获取失败或数据超时，返回规范定义的默认值 0x00
    pData[0] = 0x00;
  }

  return 0;
}
// 0xB25B_cxl
int16_t Service22Read4gAppVersion(uint8_t *pData, uint16_t *pLength)
{
  AppVersion4GPayload_t appVersion;
  int16_t ret;

  *pLength = 17;
  memset(pData, 0x00, *pLength);

  ret = StateSyncGet4gAppVersion(&appVersion);
  if (ret == 0)
  {
    memcpy(pData, appVersion.version, *pLength);
  }

  return 0;
}
// 0xB261_cxl
int16_t Service22ReadPkiApply(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length = 0;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_PKI_APPLY_REQUEST, pData, &length);
  if (ret != 0 || length == 0)
  {
    pData[0] = 0x00;
  }

  *pLength = 1;
  return 0;
}

// 0xB262_cxl
int16_t Service22ReadPkiCertStatus(uint8_t *pData, uint16_t *pLength)
{
  uint8_t pki_status = 0x00;
  int16_t ret;

  ret = EolTestSyncWithCpuGetPkiStatus(&pki_status);

  if (ret != 0)
  {
    pki_status = 0x00;
  }

  pData[0] = pki_status;
  *pLength = 1;

  return 0;
}
// 0xF18C_cxl
int16_t Service22ReadSerialNumber(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  //  uint8_t asc[64];
  //  uint32_t ascLength;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_SERIAL_NUMBER, pData, &length);
  // ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ECU_SERIAL_NUMBER,asc,&ascLength);
  if (ret != 0)
  {
    return -1;
  }
  // static int16_t CharArrayToHexArray(uint8_t *pCharIn,uint16_t charLengthIn,uint8_t *pHexOut,uint16_t *pHexLenthOut)
  // ret = CharArrayToHexArray(asc,ascLength,pData,&length);
  *pLength = length;
  return 0;
}
// 0xB266_cxl
int16_t Service22ReadGnssAntennaVoltage(uint8_t *pData, uint16_t *pLength)
{
  uint32_t voltage_mv = 0;
  uint16_t final_value = 0;

  PeripheralHalAdGet(AD_CHANNEL_GPS, &voltage_mv);

  final_value = (uint16_t)voltage_mv;

  pData[0] = (final_value >> 8) & 0xFF;
  pData[1] = final_value & 0xFF;

  *pLength = 2;

  return 0;
}
// 0xB271_cxl
int16_t Service22ReadOpenCpuTemperature(uint8_t *pData, uint16_t *pLength)
{
  if (ParameterSyncSdkGetFromCpuIsFinished() != 0)
  {
    return -1;
  }
  const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();

  if (ftyData != NULL)
  {

    pData[0] = (uint8_t)ftyData->dtc.overTemperature;
  }
  // else
  // {
  //   pData[0] = 0x00;
  // }
  *pLength = 1;
  return 0;
}

// 0xB275_cxl
int16_t Service22ReadLogDomainName(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOG_DOMAIN_NAME, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 100);
    *pLength = 100;
  }
  else
  {
    *pLength = length;
  }
  return 0;
}
// 0xB276_cxl
int16_t Service22ReadLogPort(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOG_PORT, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 8);
  }
  *pLength = 8;
  return 0;
}

// 0xB277_cxl
int16_t Service22ReadLogConnectionCommand(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOG_CONNECTION_COMMAND, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 16);
  }
  *pLength = 16;
  return 0;
}
// 0xB212_cxl
int16_t Service22ReadEthernetConfig(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ETHERNET_CONFIG, pData, &length);

  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 4);
  }

  *pLength = 4;
  return 0;
}
// 0xB278_cxl
int16_t Service22ReadPkiCertCycle(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_PKI_CERT_MAINTENANCE_CYCLE, pData, &length);
  if (ret != 0 || length == 0)
  {
    pData[0] = 0x0F;
  }

  *pLength = 1;
  return 0;
}
// 0xB279_cxl
int16_t Service22ReadPkiSdkVersion(uint8_t *pData, uint16_t *pLength)
{
  ProjectConfigGetAppSoftwareVersion_B279(pData, pLength);
  return 0;
}
// 0xB27C_cxl
int16_t Service22ReadLogAccount(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  //  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOG_ACCOUNT, pData, &length);
  //  if (ret != 0 || length == 0)
  //  {
  //    memset(pData, 0x00, 50);
  //    *pLength = 50;
  //  }
  //  else
  //  {
  //    *pLength = length;
  //  }

  return 0;
}
// 0xB27D_cxl
int16_t Service22ReadLogPassword(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOG_PASSWORD, pData, &length);
  // if (ret != 0 || length == 0)
  // {
  //   memset(pData, 0x00, 50);
  //   *pLength = 50;
  // }
  // else
  // {
  //   *pLength = length;
  // }
  return 0;
}
// 0xB27F_cxl
int16_t Service22ReadLogUploadChannel(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOG_UPLOAD_CHANNEL, pData, &length);
  // if (ret != 0 || length == 0)
  // {
  //   pData[0] = 0x00;
  // }

  // *pLength = 1;
  return 0;
}
// 0xB280_cxl
int16_t Service22ReadLogDomainFront(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOG_DOMAIN_NAME, g_log_domain_buffer, &length);
  // if (ret != 0 || length == 0)
  // {
  //   memset(pData, 0x00, 50);
  // }
  // else
  // {
  //   memcpy(pData, g_log_domain_buffer, 50);
  // }
  // *pLength = 50;
  return 0;
}
// 0xB281_cxl
int16_t Service22ReadLogDomainLater(uint8_t *pData, uint16_t *pLength)
{
  // uint32_t length;
  // int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_LOG_DOMAIN_NAME, g_log_domain_buffer, &length);
  // if (ret != 0 || length < 50)
  // {
  //   memset(pData, 0x00, 50);
  // }
  // else
  // {
  //   memcpy(pData, g_log_domain_buffer + 50, 50);
  // }
  // *pLength = 50;
  return 0;
}
// 0xB287_cxl
int16_t Service22ReadPwmSignal(uint8_t *pData, uint16_t *pLength)
{
  uint32_t voltage_mv = 0;
  uint16_t uds_value = 0;

  // PeripheralHalAdGet(AD_CHANNEL_PWM_TEST, &voltage_mv); AD_CHANNEL_PWM_TEST 需要引脚adc模拟值

  uds_value = (uint16_t)(voltage_mv / 100);

  pData[0] = (uds_value >> 8) & 0xFF;
  pData[1] = uds_value & 0xFF;

  *pLength = 2;

  return 0;
}
// 0x105E_cxl
int16_t Service22ReadTspPort3(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_TSP_PORT_3, pData, &length);

  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 8);
  }

  *pLength = 8;
  return 0;
}

// 0x1061_cxl
int16_t Service22ReadApn3(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_APN3, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 32);
    *pLength = 32;
  }
  else
  {
    *pLength = length;
  }
  return 0;
}

// 0xB28B_cxl
int16_t Service22ReadApn3Type(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_APN3_TYPE, pData, &length);
  if (ret != 0 || length == 0)
  {
    pData[0] = 0x02;
  }

  *pLength = 1;
  return 0;
}
// 0xB2C5_cxl
int16_t Service22ReadSensitiveDataSet(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_SENSITIVE_DATA_SET, pData, &length);

  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 16);
  }

  *pLength = 16;
  return 0;
}
// 0xB2C6_cxl
int16_t Service22ReadOtaRtcWakeupTime(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;

  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_OTA_RTC_WAKEUP_TIME, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 8);
  }
  *pLength = 8;
  return 0;
}
// 0xB2CA_cxl
int16_t Service22ReadOtaBookingTime(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_OTA_BOOKING_TIME, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 6);
  }
  *pLength = 6;
  return 0;
}
// 0xB2CD_cxl
int16_t Service22ReadOtaSwAcqTime(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_OTA_SW_ACQ_TIME, pData, &length);
  if (ret != 0 || length == 0)
  {
    pData[0] = 0x00;
  }
  *pLength = 1;
  return 0;
}
// 0xB2C7_cxl
int16_t Service22ReadBuryingPointSwitch(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_BURYING_POINT_SWITCH, pData, &length);

  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 4);
    pData[0] = 0x01;
  }
  *pLength = 4;
  return 0;
}
// 0xB28E_cxl
int16_t Service22ReadTspConnectionStatus(uint8_t *pData, uint16_t *pLength)
{
  TspConnectionStatusPayload_t tspStatusPayload;

  if (StateSyncGetTspConnectionStatus(&tspStatusPayload) == 0)
  {
    pData[0] = tspStatusPayload.status;
  }
  else
  {
    pData[0] = 0x00;
  }

  *pLength = 1;
  return 0;
}
// 0xB257_cxl
int16_t Service22ReadEthLineFaultInfo(uint8_t *pData, uint16_t *pLength)
{
    const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();
    uint8_t byte0 = 0;
    uint8_t byte1 = 0;

    if (ftyData != NULL)
    {
        const EthLineFaultInfo_t *info = &ftyData->ethLineFaultInfo;

        byte0  = (info->lanShortOrBreak    & 0x01) << 0;
        byte0 |= (info->snrWarn            & 0x01) << 1;
        byte0 |= (info->symErrInitFail     & 0x01) << 2;
        byte0 |= (info->pwonInitFail       & 0x01) << 3;
        byte0 |= (info->linkOkToFail       & 0x01) << 4;
        byte0 |= (info->linkFailToOk       & 0x01) << 5;
        byte0 |= (info->smiCtrlFail        & 0x01) << 6;
        byte0 |= (info->txEnableClamped    & 0x01) << 7;

        byte1  = (info->powerOverLimit     & 0x01) << 0;
        byte1 |= (info->chipOverTemp       & 0x01) << 1;
        byte1 |= (info->wakeTxTimeout      & 0x01) << 2;
        byte1 |= (info->pmCloseEth         & 0x01) << 3;
        byte1 |= (info->rsv12_15           & 0x0F) << 4;
    }
    else
    {
        return 0x22;
    }

    pData[0] = byte0;
    pData[1] = byte1;
    *pLength = 2;

    return 0;
}

// 0xB290_cxl
int16_t Service22ReadGpsDiagInfo(uint8_t *pData, uint16_t *pLength)
{
  const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();
  uint8_t *p_buf = pData;

  if (ftyData != NULL)
  {
    const GpsPosition_t *sourcePos = &ftyData->gpsPosition;
    uint8_t bitFieldByte = 0;

    *p_buf++ = (uint8_t)((sourcePos->longitude >> 24) & 0xFF);
    *p_buf++ = (uint8_t)((sourcePos->longitude >> 16) & 0xFF);
    *p_buf++ = (uint8_t)((sourcePos->longitude >> 8) & 0xFF);
    *p_buf++ = (uint8_t)(sourcePos->longitude & 0xFF);

    *p_buf++ = (uint8_t)((sourcePos->latitude >> 24) & 0xFF);
    *p_buf++ = (uint8_t)((sourcePos->latitude >> 16) & 0xFF);
    *p_buf++ = (uint8_t)((sourcePos->latitude >> 8) & 0xFF);
    *p_buf++ = (uint8_t)(sourcePos->latitude & 0xFF);

    *p_buf++ = sourcePos->eastWest;

    *p_buf++ = sourcePos->northSouth;

    *p_buf++ = (uint8_t)((sourcePos->altitude >> 8) & 0xFF);
    *p_buf++ = (uint8_t)(sourcePos->altitude & 0xFF);

    *p_buf++ = sourcePos->speed;

    *p_buf++ = (uint8_t)((sourcePos->heading >> 8) & 0xFF);
    *p_buf++ = (uint8_t)(sourcePos->heading & 0xFF);

    *p_buf++ = ((sourcePos->satUsed & 0x3F) << 0) | ((sourcePos->fixState & 0x03) << 6);
    *p_buf++ = bitFieldByte;
  }
  else
  {
    return 0x22;
  }

  *pLength = 16;

  return 0;
}
// 0xB296_cxl
int16_t Service22ReadAdbStatus(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_ADB_STATUS, pData, &length);

  if (ret != 0 || length == 0)
  {
    pData[0] = 0x00;
  }

  *pLength = 1;
  return 0;
}
// 0xB297_cxl
int16_t Service22ReadConnectionTestAddr(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_CONNECTION_TEST_ADDR, pData, &length);
  if (ret != 0 || length == 0)
  {
    const char *default_addr = "8.8.8.8";
    memset(pData, 0x00, 50);
    memcpy(pData, default_addr, strlen(default_addr));
    *pLength = 50;
  }
  else
  {
    *pLength = length;
  }
  return 0;
}
// 0xB289_cxl
int16_t Service22ReadDataBuriedPointDomain(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_DATA_BURIED_POINT_DOMAIN, pData, &length);
  if (ret != 0 || length == 0)
  {
    const char *default_domain = "https://bc-v2c-servicedatasync.gacicv.com/data-synchronization/dataSync/v1/buriedPointData";
    memset(pData, 0x00, 128);
    memcpy(pData, default_domain, strlen(default_domain));
    *pLength = 128;
  }
  else
  {
    *pLength = length;
  }
  return 0;
}
// 0xB29C_cxl
int16_t Service22ReadDataBuriedPointDomain_P1(uint8_t *pData, uint16_t *pLength)
{
  uint8_t full_buffer[128];
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_DATA_BURIED_POINT_DOMAIN, full_buffer, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 64);
  }
  else
  {
    memcpy(pData, &full_buffer[0], 64);
  }
  *pLength = 64;
  return 0;
}
// 0xB29D_cxl
int16_t Service22ReadDataBuriedPointDomain_P2(uint8_t *pData, uint16_t *pLength)
{
  uint8_t full_buffer[128];
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_DATA_BURIED_POINT_DOMAIN, full_buffer, &length);
  if (ret != 0 || length < 64)
  {
    memset(pData, 0x00, 64);
  }
  else
  {
    memcpy(pData, &full_buffer[64], 64);
  }
  *pLength = 64;
  return 0;
}
// 0xB2C9_cxl
int16_t Service22ReadGnssGalaxy(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_GNSS_GALAXY, pData, &length);
  if (ret != 0 || length == 0)
  {
    pData[0] = 0xFF;
  }

  *pLength = 1;
  return 0;
}
// 0xB2E5_cxl
int16_t Service22ReadJtagStatus(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_JTAG_STATUS, pData, &length);

  if (ret != 0 || length == 0)
  {
    pData[0] = 0x00;
  }

  *pLength = 1;
  return 0;
}
// 0xB2E6_cxl
int16_t Service22ReadFunctionConfig(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_FUNCTION_CONFIG, pData, &length);
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 64);
  }
  *pLength = 64;
  return 0;
}
// 104F_cxl
int16_t Service22ReadOtaDomainName(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  WorkFlashVehicleInforRead(E_PARAMETER_INFO_OTA_DOMAIN_NAME, pData, &length);
  *pLength = (uint16_t)length;
  return 0;
}
// 1050_cxl
int16_t Service22ReadOtaPort(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  WorkFlashVehicleInforRead(E_PARAMETER_INFO_OTA_PORT, pData, &length);
  *pLength = (uint16_t)length;
  return 0;
}

// 1052_cxl
int16_t Service22ReadPkiDomainName(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  WorkFlashVehicleInforRead(E_PARAMETER_INFO_PKI_DOMAIN_NAME, pData, &length);
  *pLength = (uint16_t)length;
  return 0;
}

// 1053_cxl
int16_t Service22ReadPkiPort(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  WorkFlashVehicleInforRead(E_PARAMETER_INFO_PKI_PORT, pData, &length);
  *pLength = (uint16_t)length;
  return 0;
}
// 0xB2B4_cxl
int16_t Service22ReadTransportMode(uint8_t *pData, uint16_t *pLength)
{
  // double transportModeValue = 0.0;
  // int16_t readResult;
  // const can0_signal_configure_t *pCan0SignalConfigure = GetCan0SignalConfigure();

  // if (pCan0SignalConfigure == NULL)
  // {
  //   pData[0] = 0x00;
  //   *pLength = 1;
  //   return 0;
  // }

  // readResult = CanParseSdkReadSignal(VEHICLE_CAN_UNPACK_FORMAT_INTEL,
  //                                    &pCan0SignalConfigure->BCM_TransportModeSt,
  //                                    &transportModeValue);

  // if (readResult != 0)
  // {
  //   pData[0] = 0x00;
  // }
  // else
  // {
  //   pData[0] = (uint8_t)transportModeValue;
  // }

  // *pLength = 1;
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_TRANSPORT_MODE, pData, &length);
  if (ret != 0 || length == 0|| pData[0] == 0xFF)
  {
    pData[0] = 0x00;
  }
  *pLength = 1;
  return 0;
}
// 0x0110_cxl
int16_t Service22ReadManufactoryMode(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_MANUFACTORY_MODE, pData, &length);
  if (ret != 0 || length == 0)
  {
    pData[0] = 0xFF;
  }

  *pLength = 1;
  return 0;
}

// 0xB2B5_cxl
int16_t Service22ReadKeySt(uint8_t *pData, uint16_t *pLength)
{
  double powerMode = 0.0;
  int16_t ret = 0;
  const can0_signal_configure_t *pCan0SignalConfigure = GetCan0SignalConfigure();

  ret = CanParseSdkReadSignal(VEHICLE_CAN_UNPACK_FORMAT_INTEL, &pCan0SignalConfigure->BCM_KeySt, &powerMode);

  if (ret != 0)
  {
    pData[0] = 0x00;
  }
  else
  {
    pData[0] = (uint8_t)powerMode;
  }

  *pLength = 1;
  return 0;
}
// 0x0120_cxl
int16_t Service22ReadDtcSettingControl(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_DTC_SETTING_CONTROL, pData, &length);

  if (ret != 0 || length == 0)
  {
    memset(pData, 0x01, 12);
    *pLength = 12;
  }
  else
  {
    *pLength = length;
  }

  return 0;
}
// 0xF186_cxl
int16_t Service22ReadActiveDiagnosticSession(uint8_t *pData, uint16_t *pLength)
{
  SessionState_e currentSession = GetCurrentSession();

  switch (currentSession)
  {
  case E_DEFAULT_SESSION:
    pData[0] = 0x01;
    break;
  case E_PROGROM_SESSION:
    pData[0] = 0x02;
    break;
  case E_EXTEND_SESSION:
    pData[0] = 0x03;
    break;
  default:
    pData[0] = 0x01;
    break;
  }

  *pLength = 1;
  return 0;
}

// 0x031C_cxl
int16_t Service22ReadTspDomain1(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_TSP_DOMAIN_1, pData, &length);

  // 如果读取失败或长度为0，则返回默认值（全0）
  if (ret != 0 || length == 0)
  {
    memset(pData, 0x00, 50);
    *pLength = 50;
  }
  else
  {
    *pLength = length;
  }
  return 0;
}
// 0x5001_cxl
int16_t Service22ReadVehicleMode(uint8_t *pData, uint16_t *pLength)
{
  pData[0] = 0x00;
  *pLength = 1;
  return 0;
}
// 0xB26B_cxl
int16_t Service22ReadSpiCommunicationBetweenMCU(uint8_t *pData, uint16_t *pLength)
{
  pData[0] = 0x00;
  *pLength = 1;
  return 0;
}
#endif

#if 1 //sq
/*31 01 service*/

// 检查编程预条件 (0x3101 0xDF01)
// 输入: 无
// 输出: 1字节结果 (0x00: correctResult, 0x01: incorrectResult)
int16_t Service31StartRoutineCheckProgramPreConditions(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut) {
  uint32_t battery_voltage = 0;  
  
  //无输入参数
  if (lengthIn != 0)
    return 0x13;

  // 检查供电电压在允许范围内（9.0V ~ 16.0V）
  BatterySdkGetVoltage(&battery_voltage);  
  if (battery_voltage < MIN_PROG_VOLTAGE || battery_voltage > MAX_PROG_VOLTAGE) {
      pDataOut[0] = 0x01;  // incorrectResult
      *pLengthOut = 1;
      return 0;
  }

  // 检查车速必须为0（车辆静止）
  uint16_t vehicle_speed = GetVehicleSpeed();  
  if (vehicle_speed != 0 && vehicle_speed != 0xFFFF) {
      pDataOut[0] = 0x01;  // incorrectResult
      *pLengthOut = 1;
      return 0;
  }
  
  // 所有预条件检查通过
  pDataOut[0] = 0x00;  // correctResult
  *pLengthOut = 1;
  return 0;
}

// 编程完整性检查例程 (0x3101 0xDF02)
// 输入: 4字节Checksum（诊断工具计算的结果）
// 输出: 1字节结果 (0x00: correctResult, 0x01: incorrectResult)
int16_t Service31StartRoutineCheckProgramIntegrity(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut){
    static Crc32Objec_t g_crc32Object = {0};
    uint32_t tool_checksum = 0;
    uint32_t calculated_checksum = 0;
    // uint32_t flash_crc32 = 0;
    // uint32_t flash_size = 0;
    
    //输入数据必须包含4字节Checksum
    if (lengthIn != 4)
      return 0x13;

    if (pDataIn == NULL || pDataOut == NULL || pLengthOut == NULL) {
        pDataOut[0] = 0x01;  // incorrectResult
        *pLengthOut = 1;
        return 0;
    }
    
    // 解析诊断工具传入的Checksum（小端格式）
    tool_checksum = ((uint32_t)pDataIn[3] << 24) | ((uint32_t)pDataIn[2] << 16) |
                    ((uint32_t)pDataIn[1] << 8)  | (uint32_t)pDataIn[0];
    
    // 初始化CRC32计算（多项式: 0x04C11DB7, 初始值: 0xFFFFFFFF）
    Crc32Init(&g_crc32Object, CRC32_POLYNOMIAL);
    
    // 计算Flash应用程序的CRC32
    // TODO: 根据实际Flash地址和大小进行配置
    // flash_crc32 = 0xFFFFFFFF;
    // flash_crc32 = Crc32(&g_crc32Object, flash_crc32, (uint8_t *)FLASH_APP_BASE_ADDRESS, flash_size);
    // flash_crc32 ^= 0xFFFFFFFF;
    // calculated_checksum = flash_crc32 ^ CRC32_INIT_VALUE;

    // 使用示例值（实际需要根据Flash内容计算）
    calculated_checksum = 0xFFFFFFFF;
    
    if (calculated_checksum == tool_checksum)
    {
        pDataOut[0] = 0x00;  // correctResult
    }
    else
    {
        pDataOut[0] = 0x01;  // incorrectResult
    }
    
    *pLengthOut = 1;
    return 0;
}

// 启动擦除内存例程 (0x3101 0xFF00)
// 输入: 9字节
//   - AddressAndLengthFormatIdentifier：1 Byte（固定值0x44）
//   - MemoryAddress：4 bytes erase address
//   - MemorySize：4 bytes erase size
// 输出: 1字节结果 (0x00: correctResult, 0x01: incorrectResult)
int16_t Service31StartRoutineEraseMemory(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
    uint32_t erase_address = 0;
    uint32_t erase_size = 0;
    uint8_t format_identifier = 0;    
    
    // 参数检查：输入数据必须包含9字节
    if (lengthIn != 9)
      return 0x13;    

    if (pDataIn == NULL || pDataOut == NULL || pLengthOut == NULL)
    {
        pDataOut[0] = 0x01;  // incorrectResult
        *pLengthOut = 1;
        return 0;
    }
    
    // 解析输入参数
    format_identifier = pDataIn[0];
    
    // 检查格式标识符（必须为0x44）
    if (format_identifier != 0x44)
    {
        pDataOut[0] = 0x01;  // incorrectResult
        *pLengthOut = 1;
        return 0;
    }
    
    // 解析擦除地址（小端格式）
    erase_address = ((uint32_t)pDataIn[4] << 24) | ((uint32_t)pDataIn[3] << 16) |
                    ((uint32_t)pDataIn[2] << 8)  | (uint32_t)pDataIn[1];
    
    // 解析擦除大小（小端格式）
    erase_size = ((uint32_t)pDataIn[8] << 24) | ((uint32_t)pDataIn[7] << 16) |
                 ((uint32_t)pDataIn[6] << 8)  | (uint32_t)pDataIn[5];
    
    // 检查地址和大小是否有效
    if (erase_size == 0)
    {
        pDataOut[0] = 0x01;  // incorrectResult
        *pLengthOut = 1;
        return 0;
    }
    
    // 检查地址是否在有效的Flash范围内
    // APP BankA: 0x000C0000 ~ 0x000BFFFF (768KB)
    // APP BankB: 0x00180000 ~ 0x0023FFFF (768KB)
    if ((erase_address < FLASH_APP_BANKA_BASE_ADDRESS ||
         erase_address > FLASH_ALL_END_ADDR) &&
        (erase_address + erase_size > FLASH_APP_BANKA_BASE_ADDRESS &&
         erase_address < FLASH_ALL_END_ADDR))
    {
        pDataOut[0] = 0x01;  // incorrectResult
        *pLengthOut = 1;
        return 0;
    }
    
    // TODO: 实现按扇区擦除逻辑
    // return 0x72;// 如果擦除失败返回0x72
    
    // 临时返回成功
    pDataOut[0] = 0x00;  // correctResult
    *pLengthOut = 1;
    return 0;
}

// 编程依赖性检查例程 (0x3101 0xFF01)
// 输入: 无
// 输出: 1字节结果 (0x00: correctResult, 0x01: incorrectResult)
int16_t Service31StartRoutineCheckProgrammingDependencies(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
    uint8_t active_bank = 0;
    // uint32_t app_crc = 0;
    // uint32_t calculated_crc = 0;
    // static Crc32Objec_t g_crc32Object = {0};

    //无输入参数
    if (lengthIn != 0)
      return 0x13;
    // 参数检查
    if (pDataOut == NULL || pLengthOut == NULL)
    {
        pDataOut[0] = 0x01;  // incorrectResult
        *pLengthOut = 1;
        return 0;
    }
    
    // 获取当前活动的APP分区
    active_bank = FlashHalOtaGetActiveBank();
    
    if (active_bank > 1)
    {
        pDataOut[0] = 0x01;  // incorrectResult
        *pLengthOut = 1;
        return 0;
    }
    
    // TODO: 验证Flash完整性
    
    // 临时返回成功    
    pDataOut[0] = 0x00;  // correctResult
    *pLengthOut = 1;
    return 0;
}

int16_t Service31StartRoutineDdrTest(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
    uint8_t ota_flag = 0;
  // 1. 检查请求长度 (RID 0xB2E6 后面无 OptionRecord)
  if (lengthIn != 0)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }
  // 2. 准备透传给 MPU 的 UDS 请求
  // UDS Payload 格式: 31 01 13 01
  uint8_t mpu_request_buffer[4];
  mpu_request_buffer[0] = 0x31; // SID
  mpu_request_buffer[1] = 0x01; // Sub-function (startRoutine)
  mpu_request_buffer[2] = 0x13; // RID High Byte
  mpu_request_buffer[3] = 0x01; // RID Low Byte

  uint8_t mpu_response_buffer[16]; // 响应缓冲区
  uint16_t mpu_response_length = 0;
  int16_t ret;

  // 3. 调用 MPU 透传函数
  ret = CanPassthrough_RequestAndGetResponse(mpu_request_buffer,
                                             sizeof(mpu_request_buffer),
                                             mpu_response_buffer,
                                             &mpu_response_length);

  if (ret != 0)
  {
    // MPU 通信失败 (超时或无响应)
    return 0x72; // generalProgrammingFailure
  }

  // 4. 处理 MPU 的响应
  if (mpu_response_length > 2 && mpu_response_buffer[0] == 0x7F)
  {
    // MPU 返回否定响应 (7F 31 [NRC])
    // "如果负反馈就不重启" - 满足条件
    return mpu_response_buffer[2];
  }

  if (mpu_response_length == 4 && mpu_response_buffer[0] == 0x71)
  {
    // MPU 返回肯定响应 (71 01 B2 E6)
    // "只要mpu回正响应就可以重启" - 满足条件

    // 5. 执行 MPU 重启
    // TBOX_PRINT("RID 0xB2E6: MPU Positive Response. Rebooting MPU...\r\n");
    //MpuHalReset(); //
    ota_flag = RemoteControlGetOtaFlag();
    if (ota_flag == 1)
    {
        return 0x21;
      // 处于OTA模式，不重启MPU
      // TBOX_PRINT("OTA mode active, skipping MPU reboot.\r\n");
    }
    else
    {
        MpuHalReset();
    }

    // 等待 MPU 开始重启 (提供短暂延时以确保 MPUHalReset 执行)
    vTaskDelay(50);

    // 6. "重启完MPU，MCU需要给诊断正响应"
    *pLengthOut = 0;
    return 0; // 成功
  }
  else
  {
    // MPU 返回了无法识别的数据或意外长度
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }
}

// 获取验证结果
int16_t Service31ResultRoutineDdrTest(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  // 1. 检查请求长度 (RID 0xB2E6 后面无 OptionRecord)
  if (lengthIn != 0)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }

  // 2. 准备透传给 MPU 的 UDS 请求
  // UDS Payload 格式: 31 03 13 01
  uint8_t mpu_request_buffer[4];
  mpu_request_buffer[0] = 0x31; // SID
  mpu_request_buffer[1] = 0x03; // Sub-function (requestRoutineResults)
  mpu_request_buffer[2] = 0x13; // RID High Byte
  mpu_request_buffer[3] = 0x01; // RID Low Byte

  uint8_t mpu_response_buffer[16]; // 响应缓冲区
  uint16_t mpu_response_length = 0;
  int16_t ret;

  // 3. 调用 MPU 透传函数
  ret = CanPassthrough_RequestAndGetResponse(mpu_request_buffer,
                                             sizeof(mpu_request_buffer),
                                             mpu_response_buffer,
                                             &mpu_response_length);

  if (ret != 0)
  {
    // MPU 通信失败 (超时或无响应)
    return 0x72; // generalProgrammingFailure
  }

  // 4. 处理 MPU 的响应
  if (mpu_response_length > 0 && mpu_response_buffer[0] == 0x71)
  {
    // MPU 返回肯定响应 (71 03 B2 E6 [Status])
    // 根据规范，MPU 应返回 1 字节状态
    if (mpu_response_length == 5) // 4 字节UDS头 + 1 字节状态
    {
      pDataOut[0] = mpu_response_buffer[4]; // statusRequest
      *pLengthOut = 1;
      return 0; // 成功
    }
    else
    {
      // MPU 返回的长度不符合预期
      return 0x13; // incorrectMessageLengthOrInvalidFormat
    }
  }
  else if (mpu_response_length > 2 && mpu_response_buffer[0] == 0x7F)
  {
    // MPU 返回否定响应 (7F 31 [NRC])
    return mpu_response_buffer[2];
  }
  else
  {
    // MPU 返回了无法识别的数据
    return 0x72; // generalProgrammingFailure
  }
}
#else
int16_t Service22ReadSK(uint8_t *pData, uint16_t *pLength)
{
  uint32_t length;
  int16_t ret;
  ret = WorkFlashVehicleInforRead(E_PARAMETER_INFO_SK, pData, &length);
  if (ret != 0)
  {
    return -1;
  }
  *pLength = length;
  return 0;
}

int16_t Service31StartRoutineWritePIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  int16_t ret = 0;

  pDataOut[0] = 0;
  *pLengthOut = 0;

  ret = Service2EWritePIN(pDataIn, lengthIn);
  if (ret != 0)
  {
    return 0x72; // 写入Flash出现错误
  }
  return 0;
}

int16_t Service31StartRoutineWriteSK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  int16_t ret = 0;

  pDataOut[0] = 0;
  *pLengthOut = 0;

  ret = Service2EWriteSK(pDataIn, lengthIn);
  if (ret != 0)
  {
    return 0x72; // 写入Flash出现错误
  }
  return 0;
}

typedef struct
{
  int8_t tbox_verify_pin_start;   // TBOX验证PIN开始标志，0结束；1开始
  int8_t tbox_verify_pin_results; // TBOX验证PIN的结果，0失败；1成功
  int8_t tbox_verify_sk_start;    // TBOX验证SK开始标志，0结束；1开始
  int8_t tbox_verify_sk_results;  // TBOX验证SK的结果，0失败；1成功
} str_tboxLearnFlagTypeDef;

static str_tboxLearnFlagTypeDef str_tboxLearnFlag = {0, 0, 0, 0};

int8_t getTboxVerifyPINstart(void)
{
  return str_tboxLearnFlag.tbox_verify_pin_start;
}

void setTboxVerifyPINstart(int8_t flag)
{
  str_tboxLearnFlag.tbox_verify_pin_start = flag;
}

void setTboxVerifyPINresults(int8_t flag)
{
  str_tboxLearnFlag.tbox_verify_pin_results = flag;
}

int8_t getTboxVerifySKstart(void)
{
  return str_tboxLearnFlag.tbox_verify_sk_start;
}

void setTboxVerifySKstart(int8_t flag)
{
  str_tboxLearnFlag.tbox_verify_sk_start = flag;
}

void setTboxVerifySKresults(int8_t flag)
{
  str_tboxLearnFlag.tbox_verify_sk_results = flag;
}

int16_t Service31StartRoutineVerifyPIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  pDataOut[0] = 0;
  *pLengthOut = 0;
  str_tboxLearnFlag.tbox_verify_pin_start = 1;
  return 0;
}

int16_t Service31StartRoutineVerifySK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  pDataOut[0] = 0;
  *pLengthOut = 0;
  str_tboxLearnFlag.tbox_verify_sk_start = 1;
  return 0;
}

int16_t Service31StartRoutineGetLearn(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  pDataOut[0] = 0;
  *pLengthOut = 0;
  return 0;
}

// 触发PKI
int16_t Service31StartRoutinePKI(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  pDataOut[0] = 0;
  *pLengthOut = 0;
  // triggerPKIToCpu();
  return 0;
}

int16_t Service31StartRoutineDiagDefault(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  SetDiagnosticVaultDefault(pDataIn[0]);
  *pLengthOut = 0;
  return 0;
}
void SetDiagnosticVaultDefault(uint8_t mode)
{
  switch (mode)
  {
  case 0:
    /* code */
    break;

  case 1:
    /* code */
    break;

  case 3:
    /* code */
    break;

  default:
    break;
  }
}


// 设置JTAG状态
int16_t Service31StartRoutineJTAG(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  SetJTAGStatus(pDataIn[0]);
  *pLengthOut = 0;
  return 0;
}

void SetJTAGStatus(uint8_t mode)
{
  switch (mode)
  {
  case 0:
    /* code */
    break;

  case 1:
    /* code */
    break;

  default:
    break;
  }
}
int16_t Service31StartRoutineDTC(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  if (lengthIn != 1)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }

  uint8_t statusMaskIn = pDataIn[0];
  uint32_t dtcNum = 0;
  uint8_t statusMaskOut = 0; // 此处用不到，但 API 需要

  // 2. 调用 taskDtcProcess.c 中的函数获取 DTC 数量
  int16_t ret = DtcProcessDtcGetCountByMask(statusMaskIn, &dtcNum, &statusMaskOut);

  if (ret == 0)
  {
    // 3. 按要求返回 1 字节的计数值
    pDataOut[0] = (uint8_t)dtcNum;
    *pLengthOut = 1;
    return 0; // 成功
  }
  else
  {
    return 0x72; // generalProgrammingFailure
  }
}
int16_t Service31StartRoutineReadEthStatus(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  // 1. 检查请求长度 (RID 0x1101 后面跟 1 个字节的 OptionRecord)
  if (lengthIn != 1)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }

  // 2. 准备透传给 MPU 的 UDS 请求
  // UDS Payload 格式: 31 01 11 01 [OptionRecord]
  uint8_t mpu_request_buffer[5];
  mpu_request_buffer[0] = 0x31;       // SID
  mpu_request_buffer[1] = 0x01;       // Sub-function (startRoutine)
  mpu_request_buffer[2] = 0x11;       // RID High Byte
  mpu_request_buffer[3] = 0x01;       // RID Low Byte
  mpu_request_buffer[4] = pDataIn[0]; // OptionRecord (以太网接口序号)

  uint8_t mpu_response_buffer[16]; // 响应缓冲区 (71 01 11 01 + 4 字节状态 = 8 字节. 16 字节是安全的)
  uint16_t mpu_response_length = 0;
  int16_t ret;

  // 3. 调用 MPU 透传函数 (来自 eolTestSyncWithCpu.h)
  // 该函数实现了 05 10 22 (请求) / 05 10 23 (响应) 的协议
  ret = CanPassthrough_RequestAndGetResponse(mpu_request_buffer,
                                             sizeof(mpu_request_buffer),
                                             mpu_response_buffer,
                                             &mpu_response_length);

  if (ret != 0)
  {
    // MPU 通信失败 (超时或无响应)
    return 0x72; // generalProgrammingFailure
  }

  // 4. 处理 MPU 的响应
  if (mpu_response_length > 0 && mpu_response_buffer[0] == 0x71)
  {
    // MPU 返回肯定响应 (71 01 11 01 [data...])
    // 根据规范，MPU 应返回 4 字节状态
    if (mpu_response_length == 8) // 4 字节UDS头 + 4 字节状态
    {
      memcpy(pDataOut, &mpu_response_buffer[4], 4);
      *pLengthOut = 4;
      return 0; // 成功
    }
    else
    {
      // MPU 返回的长度不符合预期
      return 0x13; // incorrectMessageLengthOrInvalidFormat
    }
  }
  else if (mpu_response_length > 2 && mpu_response_buffer[0] == 0x7F)
  {
    // MPU 返回否定响应 (7F 31 [NRC])
    // 将 MPU 的 NRC 转发给诊断仪
    return mpu_response_buffer[2];
  }
  else
  {
    // MPU 返回了无法识别的数据
    return 0x72; // generalProgrammingFailure
  }
}
int16_t Service31StartRoutineEthMode(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  // 1. 检查请求长度 (RID 0x1104 后面跟 2 个字节的 OptionRecord)
  if (lengthIn != 2)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }

  // 2. 准备透传给 MPU 的 UDS 请求
  // UDS Payload 格式: 31 01 11 04 [OptionRecord_Byte0] [OptionRecord_Byte1]
  uint8_t mpu_request_buffer[6];
  mpu_request_buffer[0] = 0x31;       // SID
  mpu_request_buffer[1] = 0x01;       // Sub-function (startRoutine)
  mpu_request_buffer[2] = 0x11;       // RID High Byte
  mpu_request_buffer[3] = 0x04;       // RID Low Byte
  mpu_request_buffer[4] = pDataIn[0]; // OptionRecord (以太网接口序号)
  mpu_request_buffer[5] = pDataIn[1]; // OptionRecord (通信模式)

  uint8_t mpu_response_buffer[16]; // 响应缓冲区
  uint16_t mpu_response_length = 0;
  int16_t ret;

  // 3. 调用 MPU 透传函数
  ret = CanPassthrough_RequestAndGetResponse(mpu_request_buffer,
                                             sizeof(mpu_request_buffer),
                                             mpu_response_buffer,
                                             &mpu_response_length);

  if (ret != 0)
  {
    // MPU 通信失败 (超时或无响应)
    return 0x72; // generalProgrammingFailure
  }

  // 4. 处理 MPU 的响应
  if (mpu_response_length > 0 && mpu_response_buffer[0] == 0x71)
  {
    // MPU 返回肯定响应 (71 01 11 04 [routineStatus])
    // 根据规范，MPU 应返回 1 字节状态
    if (mpu_response_length == 5) // 4 字节UDS头 + 1 字节状态
    {
      pDataOut[0] = mpu_response_buffer[4]; // routineStatus
      *pLengthOut = 1;
      return 0; // 成功
    }
    else
    {
      // MPU 返回的长度不符合预期
      return 0x13; // incorrectMessageLengthOrInvalidFormat
    }
  }
  else if (mpu_response_length > 2 && mpu_response_buffer[0] == 0x7F)
  {
    // MPU 返回否定响应 (7F 31 [NRC])
    return mpu_response_buffer[2];
  }
  else
  {
    // MPU 返回了无法识别的数据
    return 0x72; // generalProgrammingFailure
  }
}
int16_t Service31StartRoutineEthTestMode(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  // 1. 检查请求长度 (RID 0x1107 后面跟 2 个字节的 OptionRecord)
  if (lengthIn != 2)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }

  // 2. 准备透传给 MPU 的 UDS 请求
  // UDS Payload 格式: 31 01 11 07 [OptionRecord_Byte0] [OptionRecord_Byte1]
  uint8_t mpu_request_buffer[6];
  mpu_request_buffer[0] = 0x31;       // SID
  mpu_request_buffer[1] = 0x01;       // Sub-function (startRoutine)
  mpu_request_buffer[2] = 0x11;       // RID High Byte
  mpu_request_buffer[3] = 0x07;       // RID Low Byte
  mpu_request_buffer[4] = pDataIn[0]; // OptionRecord (以太网接口序号)
  mpu_request_buffer[5] = pDataIn[1]; // OptionRecord (测试模式)

  uint8_t mpu_response_buffer[16]; // 响应缓冲区
  uint16_t mpu_response_length = 0;
  int16_t ret;

  // 3. 调用 MPU 透传函数
  ret = CanPassthrough_RequestAndGetResponse(mpu_request_buffer,
                                             sizeof(mpu_request_buffer),
                                             mpu_response_buffer,
                                             &mpu_response_length);

  if (ret != 0)
  {
    // MPU 通信失败 (超时或无响应)
    return 0x72; // generalProgrammingFailure
  }

  // 4. 处理 MPU 的响应
  if (mpu_response_length > 0 && mpu_response_buffer[0] == 0x71)
  {
    // MPU 返回肯定响应 (71 01 11 07 [routineStatus])
    // 根据规范，MPU 应返回 1 字节状态
    if (mpu_response_length == 5) // 4 字节UDS头 + 1 字节状态
    {
      pDataOut[0] = mpu_response_buffer[4]; // routineStatus
      *pLengthOut = 1;
      return 0; // 成功
    }
    else
    {
      // MPU 返回的长度不符合预期
      return 0x13; // incorrectMessageLengthOrInvalidFormat
    }
  }
  else if (mpu_response_length > 2 && mpu_response_buffer[0] == 0x7F)
  {
    // MPU 返回否定响应 (7F 31 [NRC])
    return mpu_response_buffer[2];
  }
  else
  {
    // MPU 返回了无法识别的数据
    return 0x72; // generalProgrammingFailure
  }
}
int16_t Service31StartRoutineCableDiag(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  // 1. 检查请求长度 (RID 0x110B 后面跟 1 个字节的 OptionRecord)
  if (lengthIn != 1)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }

  // 2. 准备透传给 MPU 的 UDS 请求
  // UDS Payload 格式: 31 01 11 0B [OptionRecord_Byte0]
  uint8_t mpu_request_buffer[5];
  mpu_request_buffer[0] = 0x31;       // SID
  mpu_request_buffer[1] = 0x01;       // Sub-function (startRoutine)
  mpu_request_buffer[2] = 0x11;       // RID High Byte
  mpu_request_buffer[3] = 0x0B;       // RID Low Byte
  mpu_request_buffer[4] = pDataIn[0]; // OptionRecord (以太网接口序号)

  uint8_t mpu_response_buffer[16]; // 响应缓冲区
  uint16_t mpu_response_length = 0;
  int16_t ret;

  // 3. 调用 MPU 透传函数
  ret = CanPassthrough_RequestAndGetResponse(mpu_request_buffer,
                                             sizeof(mpu_request_buffer),
                                             mpu_response_buffer,
                                             &mpu_response_length);

  if (ret != 0)
  {
    // MPU 通信失败 (超时或无响应)
    return 0x72; // generalProgrammingFailure
  }

  // 4. 处理 MPU 的响应
  if (mpu_response_length > 0 && mpu_response_buffer[0] == 0x71)
  {
    // MPU 返回肯定响应 (71 01 11 0B [Interface] [Result])
    // 根据规范，MPU 应返回 2 字节状态
    if (mpu_response_length == 6) // 4 字节UDS头 + 2 字节状态
    {
      memcpy(pDataOut, &mpu_response_buffer[4], 2);
      *pLengthOut = 2;
      return 0; // 成功
    }
    else
    {
      // MPU 返回的长度不符合预期
      return 0x13; // incorrectMessageLengthOrInvalidFormat
    }
  }
  else if (mpu_response_length > 2 && mpu_response_buffer[0] == 0x7F)
  {
    // MPU 返回否定响应 (7F 31 [NRC])
    return mpu_response_buffer[2];
  }
  else
  {
    // MPU 返回了无法识别的数据
    return 0x72; // generalProgrammingFailure
  }
}
int16_t Service31StartRoutinePkiTest(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  // 1. 检查请求长度 (RID 0x220B 后面无 OptionRecord)
  if (lengthIn != 0)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }

  // 2. 准备透传给 MPU 的 UDS 请求
  // UDS Payload 格式: 31 01 22 0B
  uint8_t mpu_request_buffer[4];
  mpu_request_buffer[0] = 0x31; // SID
  mpu_request_buffer[1] = 0x01; // Sub-function (startRoutine)
  mpu_request_buffer[2] = 0x22; // RID High Byte
  mpu_request_buffer[3] = 0x0B; // RID Low Byte

  uint8_t mpu_response_buffer[16]; // 响应缓冲区
  uint16_t mpu_response_length = 0;
  int16_t ret;

  // 3. 调用 MPU 透传函数
  ret = CanPassthrough_RequestAndGetResponse(mpu_request_buffer,
                                             sizeof(mpu_request_buffer),
                                             mpu_response_buffer,
                                             &mpu_response_length);

  if (ret != 0)
  {
    // MPU 通信失败 (超时或无响应)
    return 0x72; // generalProgrammingFailure
  }

  // 4. 处理 MPU 的响应
  if (mpu_response_length == 4 && mpu_response_buffer[0] == 0x71)
  {
    // MPU 返回肯定响应 (71 01 22 0B)
    // 根据规范，startRoutine 无响应数据
    *pLengthOut = 0;
    return 0; // 成功
  }
  else if (mpu_response_length > 2 && mpu_response_buffer[0] == 0x7F)
  {
    // MPU 返回否定响应 (7F 31 [NRC])
    return mpu_response_buffer[2];
  }
  else
  {
    // MPU 返回了无法识别的数据或意外长度
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }
}
int16_t Service31StartRoutineDdrTest(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
    uint8_t ota_flag = 0;
  // 1. 检查请求长度 (RID 0xB2E6 后面无 OptionRecord)
  if (lengthIn != 0)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }

  // 2. 准备透传给 MPU 的 UDS 请求
  // UDS Payload 格式: 31 01 B2 E6
  uint8_t mpu_request_buffer[4];
  mpu_request_buffer[0] = 0x31; // SID
  mpu_request_buffer[1] = 0x01; // Sub-function (startRoutine)
  mpu_request_buffer[2] = 0xB2; // RID High Byte
  mpu_request_buffer[3] = 0xE6; // RID Low Byte

  uint8_t mpu_response_buffer[16]; // 响应缓冲区
  uint16_t mpu_response_length = 0;
  int16_t ret;

  // 3. 调用 MPU 透传函数
  ret = CanPassthrough_RequestAndGetResponse(mpu_request_buffer,
                                             sizeof(mpu_request_buffer),
                                             mpu_response_buffer,
                                             &mpu_response_length);

  if (ret != 0)
  {
    // MPU 通信失败 (超时或无响应)
    return 0x72; // generalProgrammingFailure
  }

  // 4. 处理 MPU 的响应
  if (mpu_response_length > 2 && mpu_response_buffer[0] == 0x7F)
  {
    // MPU 返回否定响应 (7F 31 [NRC])
    // "如果负反馈就不重启" - 满足条件
    return mpu_response_buffer[2];
  }

  if (mpu_response_length == 4 && mpu_response_buffer[0] == 0x71)
  {
    // MPU 返回肯定响应 (71 01 B2 E6)
    // "只要mpu回正响应就可以重启" - 满足条件

    // 5. 执行 MPU 重启
    // TBOX_PRINT("RID 0xB2E6: MPU Positive Response. Rebooting MPU...\r\n");
    //MpuHalReset(); //
    ota_flag = RemoteControlGetOtaFlag();
    if (ota_flag == 1)
    {
        return 0x21;
      // 处于OTA模式，不重启MPU
      // TBOX_PRINT("OTA mode active, skipping MPU reboot.\r\n");
    }
    else
    {
        MpuHalReset();
    }

    // 等待 MPU 开始重启 (提供短暂延时以确保 MPUHalReset 执行)
    vTaskDelay(50);

    // 6. "重启完MPU，MCU需要给诊断正响应"
    *pLengthOut = 0;
    return 0; // 成功
  }
  else
  {
    // MPU 返回了无法识别的数据或意外长度
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }
}
// 验证诊断仪PIN
uint8_t VerifyEolPINResults = 0;
int16_t Service31StartRoutineVerifyEolPIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  uint8_t pin[4] = {0};
  uint16_t pin_len = 0;
  Service22ReadPIN(pin, &pin_len);

  if (0 == memcmp(pDataIn, pin, 4))
  {
    VerifyEolPINResults = 1;
  }
  pDataOut[0] = 0;
  *pLengthOut = 0;
  return 0;
}

int16_t Service31StartRoutineClearPINSK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  uint8_t pin[4] = {0xFF, 0xFF, 0xFF, 0xFF};
  uint8_t sk[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  Service2EWritePIN(pin, 4);
  Service2EWriteSK(sk, 16);

  pDataOut[0] = 0;
  *pLengthOut = 0;
  return 0;
}


/*31 03 service*/
int16_t Service31RequestRoutineResultsVerifyEolPIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  pDataOut[0] = VerifyEolPINResults;
  *pLengthOut = 1;
  VerifyEolPINResults = 0;
  return 0;
}

int16_t Service31RequestRoutineResultsReadPIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  int16_t ret = 0;

  ret = Service22ReadPIN(pDataOut, pLengthOut);
  if (ret != 0)
  {
    return 0x72; // 写入Flash出现错误
  }
  return 0;
}

int16_t Service31RequestRoutineResultsReadSK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  int16_t ret = 0;

  ret = Service22ReadSK(pDataOut, pLengthOut);
  if (ret != 0)
  {
    return 0x72; // 写入Flash出现错误
  }
  return 0;
}

int16_t Service31ResultRoutinePkiTest(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  // 1. 检查请求长度 (RID 0x220B 后面无 OptionRecord)
  if (lengthIn != 0)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }

  // 2. 准备透传给 MPU 的 UDS 请求
  // UDS Payload 格式: 31 03 22 0B
  uint8_t mpu_request_buffer[4];
  mpu_request_buffer[0] = 0x31; // SID
  mpu_request_buffer[1] = 0x03; // Sub-function (requestRoutineResults)
  mpu_request_buffer[2] = 0x22; // RID High Byte
  mpu_request_buffer[3] = 0x0B; // RID Low Byte

  uint8_t mpu_response_buffer[16]; // 响应缓冲区
  uint16_t mpu_response_length = 0;
  int16_t ret;

  // 3. 调用 MPU 透传函数
  ret = CanPassthrough_RequestAndGetResponse(mpu_request_buffer,
                                             sizeof(mpu_request_buffer),
                                             mpu_response_buffer,
                                             &mpu_response_length);

  if (ret != 0)
  {
    // MPU 通信失败 (超时或无响应)
    return 0x72; // generalProgrammingFailure
  }

  // 4. 处理 MPU 的响应
  if (mpu_response_length > 0 && mpu_response_buffer[0] == 0x71)
  {
    // MPU 返回肯定响应 (71 03 22 0B [Status])
    // 根据规范，MPU 应返回 1 字节状态
    if (mpu_response_length == 5) // 4 字节UDS头 + 1 字节状态
    {
      pDataOut[0] = mpu_response_buffer[4]; // statusRequest
      *pLengthOut = 1;
      return 0; // 成功
    }
    else
    {
      // MPU 返回的长度不符合预期
      return 0x13; // incorrectMessageLengthOrInvalidFormat
    }
  }
  else if (mpu_response_length > 2 && mpu_response_buffer[0] == 0x7F)
  {
    // MPU 返回否定响应 (7F 31 [NRC])
    return mpu_response_buffer[2];
  }
  else
  {
    // MPU 返回了无法识别的数据
    return 0x72; // generalProgrammingFailure
  }
}
int16_t Service31ResultRoutineEthTestMode(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  // 1. 检查请求长度 (RID 0x1107 后面跟 1 个字节的 OptionRecord)
  if (lengthIn != 1)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }

  // 2. 准备透传给 MPU 的 UDS 请求
  // UDS Payload 格式: 31 03 11 07 [OptionRecord_Byte0]
  uint8_t mpu_request_buffer[5];
  mpu_request_buffer[0] = 0x31;       // SID
  mpu_request_buffer[1] = 0x03;       // Sub-function (requestRoutineResults)
  mpu_request_buffer[2] = 0x11;       // RID High Byte
  mpu_request_buffer[3] = 0x07;       // RID Low Byte
  mpu_request_buffer[4] = pDataIn[0]; // OptionRecord (以太网接口序号)

  uint8_t mpu_response_buffer[16]; // 响应缓冲区
  uint16_t mpu_response_length = 0;
  int16_t ret;

  // 3. 调用 MPU 透传函数
  ret = CanPassthrough_RequestAndGetResponse(mpu_request_buffer,
                                             sizeof(mpu_request_buffer),
                                             mpu_response_buffer,
                                             &mpu_response_length);

  if (ret != 0)
  {
    // MPU 通信失败 (超时或无响应)
    return 0x72; // generalProgrammingFailure
  }

  // 4. 处理 MPU 的响应
  if (mpu_response_length > 0 && mpu_response_buffer[0] == 0x71)
  {
    // MPU 返回肯定响应 (71 03 11 07 [Interface] [Test Mode])
    // 根据规范，MPU 应返回 2 字节状态
    if (mpu_response_length == 6) // 4 字节UDS头 + 2 字节状态
    {
      memcpy(pDataOut, &mpu_response_buffer[4], 2);
      *pLengthOut = 2;
      return 0; // 成功
    }
    else
    {
      // MPU 返回的长度不符合预期
      return 0x13; // incorrectMessageLengthOrInvalidFormat
    }
  }
  else if (mpu_response_length > 2 && mpu_response_buffer[0] == 0x7F)
  {
    // MPU 返回否定响应 (7F 31 [NRC])
    return mpu_response_buffer[2];
  }
  else
  {
    // MPU 返回了无法识别的数据
    return 0x72; // generalProgrammingFailure
  }
}
int16_t Service31ResultRoutineEthMode(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  // 1. 检查请求长度 (RID 0x1104 后面跟 1 个字节的 OptionRecord)
  if (lengthIn != 1)
  {
    return 0x13; // incorrectMessageLengthOrInvalidFormat
  }

  // 2. 准备透传给 MPU 的 UDS 请求
  // UDS Payload 格式: 31 03 11 04 [OptionRecord_Byte0]
  uint8_t mpu_request_buffer[5];
  mpu_request_buffer[0] = 0x31;       // SID
  mpu_request_buffer[1] = 0x03;       // Sub-function (requestRoutineResults)
  mpu_request_buffer[2] = 0x11;       // RID High Byte
  mpu_request_buffer[3] = 0x04;       // RID Low Byte
  mpu_request_buffer[4] = pDataIn[0]; // OptionRecord (以太网接口序号)

  uint8_t mpu_response_buffer[16]; // 响应缓冲区
  uint16_t mpu_response_length = 0;
  int16_t ret;

  // 3. 调用 MPU 透传函数
  ret = CanPassthrough_RequestAndGetResponse(mpu_request_buffer,
                                             sizeof(mpu_request_buffer),
                                             mpu_response_buffer,
                                             &mpu_response_length);

  if (ret != 0)
  {
    // MPU 通信失败 (超时或无响应)
    return 0x72; // generalProgrammingFailure
  }

  // 4. 处理 MPU 的响应
  if (mpu_response_length > 0 && mpu_response_buffer[0] == 0x71)
  {
    // MPU 返回肯定响应 (71 03 11 04 [Interface] [Comm Mode])
    // 根据规范，MPU 应返回 2 字节状态
    if (mpu_response_length == 6) // 4 字节UDS头 + 2 字节状态
    {
      memcpy(pDataOut, &mpu_response_buffer[4], 2);
      *pLengthOut = 2;
      return 0; // 成功
    }
    else
    {
      // MPU 返回的长度不符合预期
      return 0x13; // incorrectMessageLengthOrInvalidFormat
    }
  }
  else if (mpu_response_length > 2 && mpu_response_buffer[0] == 0x7F)
  {
    // MPU 返回否定响应 (7F 31 [NRC])
    return mpu_response_buffer[2];
  }
  else
  {
    // MPU 返回了无法识别的数据
    return 0x72; // generalProgrammingFailure
  }
}

int16_t Service31RequestRoutineResultsVerifyPIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  int16_t ret = 0;
  // 1、从FLASH中读取PIN码
  ret = Service22ReadPIN(pDataOut, pLengthOut);
  if (ret != 0)
  {
    return 0x72;
  }
  // TBOX_PRINT("len = %d\n",*pLengthOut);
  // 2、读取验证结果
  if (1 == str_tboxLearnFlag.tbox_verify_pin_results)
  {
    pDataOut[*pLengthOut] = 1;
  }
  else
  {
    pDataOut[*pLengthOut] = 0;
  }
  (*pLengthOut) += 1;
  // 3、验证结果清除
  str_tboxLearnFlag.tbox_verify_pin_results = -1;
  return 0;
}

int16_t Service31RequestRoutineResultsVerifySK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  int16_t ret = 0;
  // 1、从FLASH中读取SK码
  ret = Service22ReadSK(pDataOut, pLengthOut);
  if (ret != 0)
  {
    return 0x72;
  }
  // 2、读取验证结果
  if (1 == str_tboxLearnFlag.tbox_verify_sk_results)
  {
    pDataOut[*pLengthOut] = 1;
  }
  else
  {
    pDataOut[*pLengthOut] = 0;
  }
  (*pLengthOut) += 1;
  // 3、验证结果清除
  str_tboxLearnFlag.tbox_verify_sk_results = -1;
  return 0;
}

int16_t Service31RequestRoutineResultsGetLearn(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  pDataOut[0] = 1;
  *pLengthOut = 1;
  return 0;
}

int16_t Service31RequestRoutineResultsClearPINSK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  pDataOut[0] = 1;
  *pLengthOut = 1;
  return 0;
}

int16_t Service31RequestRoutineResultsPKI(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  int16_t ret = 0;

  *pLengthOut = 1;
  // ret = GetPKIState();
  pDataOut[0] = ret;

  return 0;
}

int16_t Service31RequestRoutineResultsDiagDefault(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  pDataOut[0] = GetDefaultStatus(pDataIn[0]);
  *pLengthOut = 1;
  return 0;
}
void GetDefaultStatus(uint8_t mode)
{
  uint8_t ret = 0xFF;
  if ()
  {
    renturn 2;
  }
  switch (mode)
  {
  case 0:
    /* code */
    if ()
    {
      ret = 0;
    }
    else
    {
      ret = 1;
    }
    break;

  case 1:
    /* code */
    if ()
    {
      ret = 0;
    }
    else
    {
      ret = 1;
    }

  default:
    break;
  }
  return ret;
}

int16_t Service31RequestRoutineResultsJTAG(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut)
{
  pDataOut[0] = GetJTAGStatus(pDataIn[0]);
  *pLengthOut = 1;
  return 0;
}

void GetJTAGStatus(uint8_t mode)
{
  uint8_t ret = 0xFF;
  if ()
  {
    renturn 2;
  }
  switch (mode)
  {
  case 0:
    /* code */
    if ()
    {
      ret = 0;
    }
    else
    {
      ret = 1;
    }
    break;

  case 1:
    /* code */
    if ()
    {
      ret = 0;
    }
    else
    {
      ret = 1;
    }

  default:
    break;
  }
  return ret;
}
#endif