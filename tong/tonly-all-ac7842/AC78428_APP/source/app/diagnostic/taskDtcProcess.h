#ifndef _TASK_APP_DTC_PROCESS_H
#define _TASK_APP_DTC_PROCESS_H

#include <stdint.h>

typedef enum
{
  E_DTC_GROUP_WAKE_DEDECT = 0,
  E_DTC_GROUP_KL15_DEDECT,
  E_DTC_GROUP_UNUSED,
} DtcGroup_e;

typedef enum
{
  // SQ
  //E_DTC_ITEM_CAN_BUSOFF,               // U007388: CAN总线离线
  E_DTC_ITEM_KL30_VOLTAGE_LOW,          //U300316   ECU供电电压过低
  E_DTC_ITEM_KL30_VOLTAGE_HIGH,         //U300317   ECU供电电压过高
  E_DTC_ITEM_PCAN_BUSOFF,               // U004688: PCAN 总线离线
  E_DTC_ITEM_ECAN_BUSOFF,               // U002888: ECAN 总线离线
  // E_DTC_ITEM_CAN_BUSCLOSE,               // U119088: 总线关闭
  // E_DTC_ITEM_PCAN_LIMPHOME,               // U005688: PCAN LIMPHOME
  // E_DTC_ITEM_ECAN_LIMPHOME,               // U006688: ECAN LIMPHOME
  // E_DTC_ITEM_BLE_MODULE_COMM_LOST,      // U119102: 内部蓝牙模块通讯丢失
  // E_DTC_ITEM_BLE_KEY_MODULE1_COMM_LOST, // U119202: 蓝牙钥匙模块1通讯丢失
  // E_DTC_ITEM_BLE_KEY_MODULE2_COMM_LOST, // U119302: 蓝牙钥匙模块2通讯丢失
  E_DTC_ITEM_MPU_4G_INTERNAL_COMM_FAULT, // U119402: 4G 模块内部通信故障 (MPU)
  E_DTC_ITEM_MPU_ETH_COMM_FAULT,         // U119502: 以太网通信错误 (MPU)
  // E_DTC_ITEM_CONTROLLER_VOLTAGE_LOW,    // P190016: 控制器电压低 (已删除，使用U300316替代)
  // E_DTC_ITEM_CONTROLLER_VOLTAGE_HIGH,   // P190017: 控制器电压高 (已屏蔽)
  // E_DTC_ITEM_CONSTANT_POWER_DISCONNECT, // P190013: 常电断开
  // E_DTC_ITEM_TBOX_BATTERY_FAULT,       // P190007: T-BOX内置电池故障
  // E_DTC_ITEM_TSP_ACTIVATE_FAILED,      // P190104: TSP激活失败
  E_DTC_ITEM_BACKUP_BATTERY_LOW,       // P190218: 后备电池电量低  
  E_DTC_ITEM_BACKUP_BATTERY_LIFE_ALARM,  // P19021A: 后备电池寿命告警
  E_DTC_ITEM_BACKUP_BATTERY_TEMP_ALARM,   // P19021B: 后备电池温度告警
  E_DTC_ITEM_4G_MODULE_VOLTAGE,         // P190217: 4G 模块电压异常 (MPU)
  E_DTC_ITEM_CAN_POWER_ABNORMAL,        // P190219: CAN 电源电压超限
  E_DTC_ITEM_MEMORY_FAILURE,            // P190304: 内部数据存储器故障（包括FLASH+EMMC）
  E_DTC_ITEM_MPU_GPS_ANTENNA_OPEN_SHORT, // P190315: GPS 天线开路或短路到电源 (MPU)
  E_DTC_ITEM_MPU_GPS_ANTENNA_SHORT_GND,  // P190317: GPS 天线短路到地 (MPU)
  E_DTC_ITEM_MPU_4G_MAIN_ANT_OPEN,      // P190415: 4G 主天线开路 (MPU)
  E_DTC_ITEM_MPU_4G_MAIN_ANT_SHORT_BAT, // P190417: 4G 主天线短路到电源 (MPU)
  E_DTC_ITEM_MPU_4G_MAIN_ANT_SHORT_GND, // P190414: 4G 主天线短路到地 (MPU)
  E_DTC_ITEM_MPU_4G_AUX_ANT_OPEN,      // P190515: 4G 副天线开路 (MPU)
  E_DTC_ITEM_MPU_4G_AUX_ANT_SHORT_BAT, // P190517: 4G 副天线短路到电源 (MPU)
  E_DTC_ITEM_MPU_4G_AUX_ANT_SHORT_GND, // P190514: 4G 副天线短路到地 (MPU)
  E_DTC_ITEM_MPU_SIM_FAULT,              // P190507: SIM 卡故障 (MPU)
  E_DTC_ITEM_MPU_ETH_LINE_FAULT,         // P190509: 以太网物理线路故障 (MPU)
  // E_DTC_ITEM_ECALL_LIGHT_SHORT_GND,   // P190614: E-Call 指示灯短路到地
  // E_DTC_ITEM_BCALL_LIGHT_SHORT_GND,   // P190714: B-Call 指示灯短路到地
#if 0
  //E_DTC_ITEM_BACKUP_BATTERY_LOW,        // B320E16: 后备电池电量低
  E_DTC_ITEM_MAIN_POWER_OVER_RANGE,     // B11001C: 主电源电压超限
  E_DTC_ITEM_MAIN_POWER_OVER_MAX_RANGE, // B110091: 主电源电压超极限
  E_DTC_ITEM_MEMORY_FAILURE,            // B320244: 内部数据存储器故障

  E_DTC_ITEM_MPU_GPS_ANTENNA_OPEN_SHORT, // B320415: GPS 天线开路或短路到电源 (MPU)
  E_DTC_ITEM_MPU_GPS_ANTENNA_SHORT_GND,  // B320411: GPS 天线短路到地 (MPU)

  E_DTC_ITEM_MPU_4G_MAIN_ANT_OPEN,      // B320513: 4G 主天线开路 (MPU)
  E_DTC_ITEM_MPU_4G_MAIN_ANT_SHORT_BAT, // B320512: 4G 主天线短路到电源 (MPU)
  E_DTC_ITEM_MPU_4G_MAIN_ANT_SHORT_GND, // B320511: 4G 主天线短路到地 (MPU)

  E_DTC_ITEM_MPU_4G_AUX_ANT_OPEN,      // B320613: 4G 副天线开路 (MPU)
  E_DTC_ITEM_MPU_4G_AUX_ANT_SHORT_BAT, // B320612: 4G 副天线短路到电源 (MPU)
  E_DTC_ITEM_MPU_4G_AUX_ANT_SHORT_GND, // B320611: 4G 副天线短路到地 (MPU)

  E_DTC_ITEM_MPU_SIM_FAULT,              // B320909: SIM 卡故障 (MPU)
  E_DTC_ITEM_MPU_4G_INTERNAL_COMM_FAULT, // B320104: 4G 模块内部通信故障 (MPU)
  E_DTC_ITEM_MPU_ETH_LINE_FAULT,         // B320B79: 以太网物理线路故障 (MPU)
  E_DTC_ITEM_MPU_ETH_COMM_FAULT,         // B320B08: 以太网通信错误 (MPU)
  E_DTC_ITEM_MPU_DDR_FAULT,              // B321296: 模组 DDR 故障 (MPU)

  E_DTC_ITEM_MPU_BCALL_KEY_STUCK, // B320707: B-Call 按键卡滞 (MPU)
  E_DTC_ITEM_MPU_ECALL_KEY_STUCK, // B320807: E-Call 按键卡滞 (MPU)

  E_DTC_ITEM_ECALL_LIGHT_SHORT_GND, // B320C11: E-Call 指示灯短路到地
  E_DTC_ITEM_BCALL_LIGHT_SHORT_GND, // B320D11: B-Call 指示灯短路到地

  E_DTC_ITEM_4G_MODULE_VOLTAGE,    // B32011C: 4G 模块电压异常 (MPU)
  E_DTC_ITEM_CAN_POWER_ABNORMAL,   // B33001C: CAN 电源电压超限
  E_DTC_ITEM_MPU_MIC_POWER_FAULT,  // B32031C: 麦克风电源故障 (MPU)
  E_DTC_ITEM_MPU_MIC_SIGNAL_FAULT, // B32031F: 麦克风信号输入故障 (MPU)

  E_DTC_ITEM_MPU_OPENCPU_OVER_TEMP, // B320198: OpenCPU 过温 (MPU)

  E_DTC_ITEM_DCAN_BUSOFF,
  E_DTC_ITEM_KL30_VOLTAGE_HIGH,
  E_DTC_ITEM_KL30_VOLTAGE_LOW,

  E_DTC_ITEM_NODE_MISSING_310,

  E_DTC_ITEM_NODE_MISSING_BCM1_319,
  E_DTC_ITEM_NODE_MISSING_AC1_29D,
  E_DTC_ITEM_NODE_MISSING_PEPS2_295,
  E_DTC_ITEM_NODE_MISSING_GWFD1_2BB,
  E_DTC_ITEM_NODE_MISSING_IP2_27F,
  E_DTC_ITEM_NODE_MISSING_TPMS1_341,
  E_DTC_ITEM_NODE_MISSING_EEM1_2A8,
#endif
  E_DTC_ITEM_MAX_NUMBER,
} DtcItem_e;

typedef struct _TimeService_t
{
  uint32_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

} TimeService_t;

#if 1 //SQ
typedef struct
{
    uint16_t DF20_ECUVoltage;      // DF20: ECU供电电压 (2字节, 0-40V, phy=XX*0.001, 无效值0xFFFF)     
    uint16_t DF21_Speed;            // DF21: 车速 (2字节, 0-255Km/h, phy=XX*0.00390625, 无效值0xFFFF)
    uint16_t DF22_EngineRPM;       // DF22: 发动机转速 (2字节, 0-1734.96875rpm, phy=XX *0.03125-273, 无效值0xFFFF)    
    int16_t  DF23_MotorRPM;        // DF23: 电机转速 (2字节, -15000~15000rpm, phy= XX *1-15000, 无效值0xFFFF)    
    uint8_t  DF24_IgnitionState;   // DF24: 点火状态 (1字节, 0-7, 无效值0xFF)    
    uint16_t DF25_HVLineVoltage;   // DF25: 高压母线电压 (2字节, 0-6553.5V, phy=XX*0.1, 无效值0xFFFF)    
    struct {                       
        uint8_t year_high;         // 年-高字节 (1-99)
        uint8_t year_low;          // 年-低字节 (1-99)
        uint8_t month;             // 月 (1-12)
        uint8_t day;               // 日 (1-31)
        uint8_t hour;              // 时 (0-24)
        uint8_t minute;            // 分 (0-60)
        uint8_t second;            // 秒 (0-60)
    } DF26_SystemTime;            // DF26: 系统时钟 (7字节, BCD格式)    
    uint8_t  DF27_FaultCount;    // DF27: 故障发生次数 (1字节, 0-255, 无效值0xFF)      
    uint32_t DF28_Mileage;         // DF28: 里程 (4字节, 0-536870911.9Km, phy=XX*0.125, 无效值0xFFFFFFFF)    
    uint8_t  reserved[10];          
} DtcSnapshotRecordGlobal_t;

typedef struct
{
  uint8_t Bit1CycleFlag;
  uint8_t Bit2CycleFlag;
  uint8_t FaultOccurrenceCounter;
  uint8_t FaultPendingCounter;
  uint8_t DtcAgingCounter;
  uint8_t DtcAgedCounter;  
} DtcExtendedDataRecordGlobal_t;

// 无效值定义
#define INVALID_DF20    0xFFFF
#define INVALID_DF21    0xFFFF
#define INVALID_DF22    0xFFFF
#define INVALID_DF23    0xFFFF
#define INVALID_DF24    0xFF
#define INVALID_DF25    0xFFFF
#define INVALID_DF27    0xFF
#define INVALID_DF28    0xFFFFFFFF

// 快照数据DID定义
#define DID_DF20_ECU_VOLTAGE    0xDF20
#define DID_DF21_VEHICLE_SPEED  0xDF21
#define DID_DF22_ENGINE_SPEED   0xDF22
#define DID_DF23_MOTOR_SPEED    0xDF23
#define DID_DF24_IGNITION_STATE 0xDF24
#define DID_DF25_HV_VOLTAGE     0xDF25
#define DID_DF26_SYSTEM_TIME    0xDF26
#define DID_DF27_FAULT_COUNT    0xDF27
#define DID_DF28_MILEAGE        0xDF28

#else
typedef struct
{
  uint16_t vehicleSpeed;     // 0-1
  int16_t motorSpeed;        // 2-3
  uint16_t ecuVoltage;       // 4-5
  uint8_t keyStatus;         // 6
  uint8_t reserved;          // 7
  uint8_t globalRealTime[6]; // 8-13
} DtcSnapshotRecordGlobal_t;

typedef struct
{
  uint8_t Bit1CycleFlag;
  uint8_t Bit2CycleFlag;
  uint8_t FaultOccurrenceCounter;
  uint8_t FaultPendingCounter;
  uint8_t DtcAgingCounter;
  uint8_t DtcAgedCounter;
} DtcExtendedDataRecordGlobal_t;
#endif

/********storage struct************************/
typedef struct
{
  uint8_t testFailed : 1;
  uint8_t testFailedThisOperationCycle : 1;   //
  uint8_t pendingDTC : 1;                     //
  uint8_t confirmedDTC : 1;                   //
  uint8_t testNotCompletedSinceLastClear : 1; //
  uint8_t testFailedSinceLastClear : 1;       //
  uint8_t testNotCompletedThisOperationCycle : 1;
  uint8_t warningIndicatorRequested : 1;
} DtcStatusStoreBit_t;

typedef union
{
  DtcStatusStoreBit_t bitField;
  uint8_t u8Field;
} DtcStatusStore_t;

typedef struct
{
  DtcStatusStore_t dtcStatus;
  uint8_t confirmStage;
  uint32_t dtcCode;
  uint32_t TripCounter;
  uint32_t AgingCounter;
  uint32_t agedCounter;
  uint32_t occurenceCounter;
  uint32_t currentOperationCycle;
  uint32_t faildOperationCycle;

  // snapshot data
  DtcSnapshotRecordGlobal_t snapshotData;
  // Extended Data
  DtcExtendedDataRecordGlobal_t extendData;

} DtcStore_t;

int16_t DtcUpdateStatusFromMpu(DtcItem_e item, uint8_t statusByte); //_cxl

//
int16_t SetDtcFaultState(DtcItem_e item);
int16_t ClearDtcFaultState(DtcItem_e item);
/*
int16_t CanDtcDetectEnable(uint8_t canChannel);
int16_t CanDtcDetectDisable(uint8_t canChannel);
*/
int16_t DtcProcessEnable(void);
int16_t DtcProcessDisable(void);
uint8_t GetDtcProcessSetStatus(void);
void DtcProcessEnableByGroup(DtcGroup_e groupItem);
void DtcProcessDisableByGroup(DtcGroup_e groupItem);

int16_t DtcProcessGetListByMask(uint8_t statusMaskIn, uint8_t *dtcData, uint32_t *dtcNumOut, uint8_t *statusMaskOut);
int16_t DtcProcessDtcGetCountByMask(uint8_t statusMaskIn, uint32_t *dtcNumOut, uint8_t *statusMaskOut);
int16_t DtcProcessGetExtendedData(uint32_t dtcCode, uint8_t recordNum, uint8_t *extendedData, uint32_t *pExtendedDataLength, uint8_t *statusMaskOut);
int16_t DtcProcessGetReportSnapshotRecordByDtcNumber(uint32_t dtcCode, uint8_t snapshotRecordNum, uint8_t *pSnapshotRecord, uint32_t *pRecordLength, uint8_t *statusMaskOut);
int16_t DtcProcessGetSupportedDtc(uint8_t *pDtcData, uint32_t *pDtcNum, uint8_t *pStutasMaskOut);
uint16_t DtcProcessClearAll(void);

/******only used by EOL test*****************/
int16_t DtcProcessTestGetListByMask(uint8_t statusMaskIn, uint8_t *dtcData, uint32_t *dtcNumOut, uint8_t *statusMaskOut);
int16_t DtcProcessDtcTestGetCountByMask(uint8_t statusMaskIn, uint32_t *dtcNumOut, uint8_t *statusMaskOut);

void SetVolFault(void);

int16_t TaskAppDtcProcessInit(void);
void TaskDtcProcess(void *pvParameters);

void DtcReloadSettingControl(void);

int16_t DtcProcessClearBySpecificDTC(uint32_t specificDtc); //SQ
#endif //_TASK_APP_MCU_CPU_SYSNC_H
