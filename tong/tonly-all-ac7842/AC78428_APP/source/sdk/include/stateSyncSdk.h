/*************************************************
Copyright ? 2024 SiRun (Beijing) . All rights reserved.
  File Name:       stateSyncSdk.h
  Author:          lei.wang
  Created Time:    2024
  Description:     Synchronize status with CPU
   Others:      // 其它说明
*************************************************/
#ifndef _STATE_SYNC_SDK_H_
#define _STATE_SYNC_SDK_H_

#include <stdint.h>

#include "mpuHal.h"
#include "stdint.h"
#include "timerHal.h"

typedef enum
{
  E_STATE_SYNC_LOCALTION_INFO_MID = 0x10,
  E_STATE_SYNC_NET_INFO_MID = 0x11,
  // E_STATE_SYNC_G_SENSOR_MID              = 0x14,
  E_STATE_SYNC_HAL_STATE_MID = 0x16,
  E_STATE_SYNC_TSP_CONNECT_STATE_MID = 0x17,
  E_STATE_SYNC_ALL_FTY_DATA_MID = 0x08 // 0xDTC_table_cxl

} StateSyncMid_e;
//

// 0xB28E_cxl
typedef struct
{
  uint8_t status;
} TspConnectionStatusPayload_t;
typedef struct
{
  TspConnectionStatusPayload_t pkiStatus; // 核心数据负载 (1字节)
  uint16_t timeCount;                     // 更新时长计数
  uint16_t failureTime;                   // 失效时间 (ms)
  uint8_t validity;                       // 数据是否有效 1：有效 0：无效
} CpuTspConnectionStatusSync_t;

typedef struct
{
  uint8_t moduleState;   // 模块状态 0：工作正常 1：工作异常
  uint8_t wireState;     // 电线状态 0: 正常 1：电线断路 2：电线地短路 3：电线电源短路
  uint8_t locationState; // 定位状态 Bit0-bit5: GNSS定位状态 0: 未定位 1: 单点定位 2: 浮点定位 3: RTK定位
                         // Bit6: 0-东经，1-西经
                         // Bit7: 0-北纬，1-南纬
  uint32_t longitude;    // 经度，单位：度，精度：10-7
  uint32_t latitude;     // 纬度，单位：度，精度：10-7
  uint8_t flongitude[8];
  uint8_t flatitude[8];
  uint16_t altitude;  // 高程，单位：米，精度：0.1米
  uint16_t speed;     // 速度，单位：km/h，精度：0.1km/h
  uint16_t heading;   // 0...360	航向，单位：度，精度：0.1度
  uint16_t accuracy;  // 定位水平精度,单位：厘米，精度：1厘米
  uint8_t svsNum;     // 可见卫星数量
  uint8_t useSvsnum;  // 使用卫星数量
  uint32_t timeStamp; // 时间戳
} LocationInfoSync_t;

typedef struct
{
  uint8_t creg;       // 驻网状态，驻网失败时，其它字段无意义：0：成功 1：失败
  uint8_t csq;        // 信号质量：0-31有效
  uint8_t netType;    // 网络类型：0：GSM1：LTE
  uint8_t apnCounter; // APN数量。最大6路APN
  uint8_t apnStatus;  // 0: 未连接，1：已连接 Bit0：第1路APN bit5：第6路APN
  uint8_t mainAntennaStatus;
  uint8_t slaveAntennaStatus;
} NetInfoSync_t;

typedef struct
{
  uint8_t MemUsageStatus; // cpu内存使用率 ，百分比
  uint8_t cpuUsage;       // CPU使用率，百分比
  uint16_t tmpStatus;     // CPU温度；
} HalStateSync_t;

typedef struct
{
  uint8_t tspStatus; // tsp采集平台连接状态 0：未登录，1：登录
} TspStateSync_t;

// 0xB24C_cxl 0xB24D_cxl
typedef struct
{
  uint8_t fw_version[20]; // 固件版本
  uint8_t hw_version[20]; // 硬件版本
} ModuleInfo4GPayload_t;

// 0xB256_cxl
typedef struct
{
  uint8_t id[5];
} EthernetIdPayload_t;

// 0xB259_cxl
typedef struct
{
  uint8_t status; // 0x00=Idle/Default, 0x11=GB_Deleting, 0x12=EMMC_Deleting, 0x21=GB_Success, 0x22=EMMC_Success, 0x31=GB_Fail, 0x32=EMMC_Fail
} EmmcDeleteStatusPayload_t;
// 0xB25B_cxl
typedef struct
{
  uint8_t version[17];
} AppVersion4GPayload_t;
// 0xB262_cxl
typedef struct
{
  uint8_t status; // 0=无, 1=公有, 2=专属, 3=都有
} PkiCertStatusPayload_t;
// 0xB262_cxl
typedef struct
{
  PkiCertStatusPayload_t pkiStatus;
  uint16_t timeCount;
  uint16_t failureTime;
  uint8_t validity;
} CpuPkiCertStatusSync_t;

// 0xB25B_cxl
typedef struct
{
  AppVersion4GPayload_t appVersion;
  uint16_t timeCount;
  uint16_t failureTime;
  uint8_t validity;
} CpuAppVersion4GSync_t;

// 0xB259_cxl
typedef struct
{
  EmmcDeleteStatusPayload_t deleteStatus;
  uint16_t timeCount;
  uint16_t failureTime;
  uint8_t validity;
} CpuEmmcDeleteStatusSync_t;

// 0xB256_cxl
typedef struct
{
  EthernetIdPayload_t ethId;
  uint16_t timeCount;
  uint16_t failureTime;
  uint8_t validity;
} CpuEthernetIdSync_t;

// 0xB24C_cxl 0xB24D_cxl
typedef struct
{
  ModuleInfo4GPayload_t moduleInfo;
  uint16_t timeCount;
  uint16_t failureTime;
  uint8_t validity;
} CpuModuleInfo4GSync_t;

typedef struct
{
  LocationInfoSync_t locationInfo;
  uint16_t timeCount;   // 更新时长计数，用来确认
  uint16_t failureTime; // 失效时间
  uint8_t validity;     // 数据是否有效 1：有效 0：无效
} CpuLocationInfo_t;

typedef struct
{
  NetInfoSync_t netInfo;
  uint16_t timeCount;   // 更新时长计数，用来确认
  uint16_t failureTime; // 失效时间
  uint8_t validity;     // 数据是否有效 1：有效 0：无效
} CpuNetInfoSync_t;

typedef struct
{
  HalStateSync_t halState;
  uint16_t timeCount;   // 更新时长计数，用来确认
  uint16_t failureTime; // 失效时间
  uint8_t validity;     // 数据是否有效 1：有效 0：无效
} CpuHalStateSync_t;

typedef struct
{
  TspStateSync_t tspState;
  uint16_t timeCount;   // 更新时长计数，用来确认
  uint16_t failureTime; // 失效时间 单位ms
  uint8_t validity;     // 数据是否有效 1：有效 0：无效
} CpuTspStateSync_t;

typedef struct
{
  uint8_t state;        //
  uint16_t timeCount;   // 更新时长计数，用来确认
  uint16_t failureTime; // 失效时间 单位ms
  uint8_t validity;     // 数据是否有效 1：有效 0：无效
} GsensorStateSync_t;

/* ---- DTC 相关枚举 ---- */
typedef enum
{
  DTC_STORE_OK = 0,
  DTC_STORE_EMMC_ERR = 1,
  DTC_STORE_SPI_ERR = 2,
  DTC_STORE_RSVD = 3,
} dtc_store_state_e;

typedef enum
{
  DTC_ANT_OK = 0,
  DTC_ANT_OPEN = 1,
  DTC_ANT_SHORT_V = 2, // 短到电源
  DTC_ANT_SHORT_G = 3, // 短到地
} dtc_ant_state_e;

typedef enum
{
  DTC_ETH_OK = 0,
  DTC_ETH_INIT_ERR = 1,
  DTC_ETH_LINK_ERR = 2,
  DTC_ETH_SMI_ERR = 3,
  DTC_ETH_COMM_ERR = 4,
} dtc_eth_state_e;

typedef enum
{
  DTC_DDR_OK = 0,
  DTC_DDR_ERR = 1,
} dtc_ddr_state_e;

/* ---- 0xB210 网络状态枚举 ---- */
typedef enum
{
  OP_CHINA_MOBILE = 0,
  OP_CHINA_UNICOM = 1,
  OP_CHINA_TELEC = 2,
} lte_operator_e;

typedef enum
{
  RAT_GSM = 0,
  RAT_EDGE = 1,
  RAT_EGSM = 2,
  RAT_WCDMA = 3,
  RAT_TDSCDMA = 4,
  RAT_LTE_FDD = 5,
  RAT_LTE_TDD = 6,
  RAT_RSVD7 = 7,
} lte_rat_e;

typedef enum
{
  NET_NOT_REG_NOT_SEARCH = 0,
  NET_REGISTERED = 1,
  NET_NOT_REG_SEARCHING = 2,
  NET_REG_REJECTED = 3,
  NET_UNKNOWN = 4,
  NET_REGISTERED_ROAM = 5,
} lte_net_state_e;

/* ---- MPU上报的DTC状态表结构体 ---- */
typedef struct
{
  uint16_t storeState : 2;      // [1:0]  内部存储器：0正常 1 eMMC错 2 SPI错 3保留
  uint16_t antMain : 2;         // [3:2]  主天线：0正常 1开路 2短到电源 3短到地
  uint16_t antSec : 2;          // [5:4]  副天线：0正常 1开路 2短到电源 3短到地
  uint16_t simFault : 1;        // [6]    SIM：0 ready 1 fault
  uint16_t lteFault : 1;        // [7]    4G内部通讯：0正常 1故障
  uint16_t ethState : 3;        // [10:8]  以太网：0正常 1系统初始化失败 2链路失败 3SMI控制失败 4以太网通讯错误
  uint16_t ddrState : 2;        // [12:11]  DDR：0正常 1故障 其他保留
  uint16_t overTemperature : 1; // [13]  温度过高：0正常 1故障(超过120摄氏度))
  uint16_t rsv : 2;             // [15:14]保留，写0
} DtcTable_t;

/* ---- 0xB201 模块工作状态结构体 ---- */
typedef struct
{
  uint16_t powerMode : 4;
  uint16_t dtrReady : 1;
  uint16_t dsrReady : 1;
  uint16_t reqSend : 1;
  uint16_t clrSend : 1;
  uint16_t rsv8_15 : 8;
} LteModuleStatus_t;

/* ---- 0xB202 GNSS 模块工作状态结构体 ---- */
typedef struct
{
  uint16_t gnssEnableState : 1;
  uint16_t modeMixed : 1;
  uint16_t beidouEnable : 1;
  uint16_t gpsEnable : 1;
  uint16_t glonassEnable : 1;
  uint16_t fusionFront : 1;
  uint16_t agpsEnable : 1;
  uint16_t protoCustom : 1;
  uint16_t outComposite : 1;
  uint16_t rsv9 : 1;
  uint16_t outGpsInfo : 1;
  uint16_t outGroundSpeed : 1;
  uint16_t outCurSatInfo : 1;
  uint16_t outVisibleSat : 1;
  uint16_t outRecommended : 1;
  uint16_t outGeoPos : 1;
} GnssModuleStatus_t;

/* ---- 0xB210 4G模块网络状态结构体 ---- */
typedef struct
{
  uint8_t rssi;
  uint8_t ber;
  struct
  {
    uint16_t operatorType : 2;
    uint16_t rat : 3;
    uint16_t netState : 3;
    uint16_t faultCode : 8;
  } b2_3;
} LteNetworkStatus_t;

/* ---- 0xB249 GPS卫星信息结构体 ---- */

typedef struct
{
  uint8_t visibleCount;
  struct
  {
    uint8_t id;
    uint8_t elevation;
    uint16_t azimuth;
    uint8_t cnr;
  } sat[4];
  uint8_t pdop[3];
  uint8_t hdop[3];
  uint8_t vdop[3];
} GpsSatInfo_1to4_t;

/* ---- 0xB24E 4G模块功能状态结构体 ---- */
typedef struct
{
  uint16_t uartMainEn : 1;
  uint16_t spiEn : 1;
  uint16_t tcpEn : 1;
  uint16_t usbModeEn : 1;
  uint16_t netHeartbeatEn : 1;
  uint16_t lteGpsEn : 1;
  uint16_t i2sDigitalAuEn : 1;
  uint16_t agpsEn : 1;
  uint16_t cellLocateEn : 1;
  uint16_t auxUartConsole : 1;
  uint16_t usbModemEn : 1;
  uint16_t usbDiagEn : 1;
  uint16_t usbAuxEn : 1;
  uint16_t uartDataEn : 1;
  uint16_t uartTraceEn : 1;
  uint16_t wlanEn : 1;
} LteFunctionStatus_t;

/* ---- 0xB257 以太网线路故障信息结构体 ---- */
typedef struct
{
  uint16_t lanShortOrBreak : 1;
  uint16_t snrWarn : 1;
  uint16_t symErrInitFail : 1;
  uint16_t pwonInitFail : 1;
  uint16_t linkOkToFail : 1;
  uint16_t linkFailToOk : 1;
  uint16_t smiCtrlFail : 1;
  uint16_t txEnableClamped : 1;
  uint16_t powerOverLimit : 1;
  uint16_t chipOverTemp : 1;
  uint16_t wakeTxTimeout : 1;
  uint16_t pmCloseEth : 1;
  uint16_t rsv12_15 : 4;
} EthLineFaultInfo_t;

/* ---- 0xB290 GPS经纬度/搜星数结构体 ---- */

typedef struct
{
  uint32_t longitude;
  uint32_t latitude;
  uint8_t eastWest;
  uint8_t northSouth;
  uint16_t altitude;
  uint8_t speed;
  uint16_t heading;
  uint8_t satUsed : 6;
  uint8_t fixState : 2;
} GpsPosition_t;

/* ---- 统一的、聚合的同步大结构体 ---- */
typedef struct
{
  DtcTable_t dtc;
  LteModuleStatus_t lteModuleStatus;
  GnssModuleStatus_t gnssModuleStatus;
  TimeData_t timeData;
  LteNetworkStatus_t lteNetworkStatus;
  GpsSatInfo_1to4_t gpsSatInfo1to4;
  LteFunctionStatus_t lteFunctionStatus;
  EthLineFaultInfo_t ethLineFaultInfo;
  GpsPosition_t gpsPosition;
} ftyCircleDataToMcu_t;

typedef struct
{
  uint8_t validity;
  uint32_t timeCount;
  uint32_t failureTime;
  ftyCircleDataToMcu_t payload;
} CpuFtyDataSync_t;

const ftyCircleDataToMcu_t *StateSyncGetFtyData(void);

/*************************************************
  Function:       StateSyncSdkInit
  Description:    状态同步模块初始化接口
  Input:          mpuHandle：mpu通信句柄
                  cycleTime：周期调用接口调用周期
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t StateSyncSdkInit(int16_t mpuHandle, uint16_t cycleTime);

/*************************************************
  Function:       StateSyncSdkCycleProcess
  Description:    状态同步模块周期调用接口
  Input:          pRxData：传入接收到的CPU数据
                  rxLength：数据长度
  Output:         无
  Return:         无
  Others:
*************************************************/
void StateSyncSdkCycleProcess(MpuHalDataPack_t *msgData);

/*************************************************
  Function:       StateSyncGetLocationInfo
  Description:    获取CPU定位信息接口
  Input:          无
  Output:         localtioninfo：定位信息
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t StateSyncGetLocationInfo(LocationInfoSync_t *localtionInfo);

/*************************************************
  Function:       StateSyncGetNetInfo
  Description:    获取cpu网络信息接口
  Input:          无
  Output:         netInfo：cpu网络信息
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t StateSyncGetNetInfo(NetInfoSync_t *netInfo);

/*************************************************
  Function:       StateSyncGetHalstate
  Description:    获取cpu硬件状态
  Input:          无
  Output:         halState：cpu硬件状态
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t StateSyncGetHalstate(HalStateSync_t *halState);

/*************************************************
  Function:       StateSyncGetTspState
  Description:    获取远程平台连接状态
  Input:          无
  Output:         tspState：平台连接状态
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t StateSyncGetTspState(TspStateSync_t *tspState);

/*************************************************
  Function:       StateSyncGetGsersorState
  Description:    获取G-Sensor状态
  Input:          无
  Output:         state：状态 0正常，1错误
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t StateSyncGetGsensorState(uint8_t *state);

/*
  Function:       StateSyncGet4gModuleInfo
  Description:    获取从MPU同步过来的4G模块信息
  Input:          pModuleInfo: 指向用于存储4G模块信息的结构体指针
  Output:         pModuleInfo: 填充了最新的4G模块信息
  Return:         0: 成功, -1: 失败或数据无效
*/
int16_t StateSyncGet4gModuleInfo(ModuleInfo4GPayload_t *pModuleInfo); // 0xB24C_cxl 0xB24D_cxl

int16_t StateSyncGetEthernetId(EthernetIdPayload_t *pEthId); // 0xB256_cxl

// 0xB259_cxl
/*************************************************
  Function:       StateSyncSendEmmcDeleteCommand
  Description:    发送EMMC删除命令到MPU
*************************************************/
int16_t StateSyncSendEmmcDeleteCommand(uint8_t command);
// 0xB259_cxl
/*************************************************
  Function:       StateSyncGetEmmcDeleteStatus
  Description:    获取EMMC删除状态
*************************************************/
int16_t StateSyncGetEmmcDeleteStatus(EmmcDeleteStatusPayload_t *pDeleteStatus);

/*************************************************
  Function:       StateSyncGet4gAppVersion
  Description:    获取4G模块应用程序版本

*************************************************/
int16_t StateSyncGet4gAppVersion(AppVersion4GPayload_t *pAppVersion); // 0xB25B_cxl

int16_t StateSyncGetPkiCertStatus(PkiCertStatusPayload_t *pPkiStatus); // 0xB262_cxl

int16_t StateSyncGetTspConnectionStatus(TspConnectionStatusPayload_t *pStatus); // 获取TSP连接状态 0xB28E_cxl

#endif
