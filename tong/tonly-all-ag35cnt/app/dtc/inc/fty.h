#ifndef FTY_H
#define FTY_H

#include "stdint.h"
#include "stdio.h"
#include <glib.h>

#define DDR_DIR_PATH       "/sdcard/sr_config"
#define DDR_FILE_PATH      "/sdcard/sr_config/ddr.txt"
#define CMD_DDR_SET        "serial_atcmd at*devtest=1,0"
#define CMD_DDR_READ       "serial_atcmd at*devtest=1,1"
#define CMD_REBOOT         "serial_atcmd at*reboot"

/* ---- DTC 相关 ---- */
typedef enum {
    DTC_STORE_OK        = 0,
    DTC_STORE_EMMC_ERR  = 1,
    DTC_STORE_SPI_ERR   = 2,
    DTC_STORE_RSVD      = 3,
} dtc_store_state_e;

typedef enum {
    DTC_ANT_OK       = 0,
    DTC_ANT_OPEN     = 1,
    DTC_ANT_SHORT_V  = 2,  // 短到电源
    DTC_ANT_SHORT_G  = 3,  // 短到地
} dtc_ant_state_e;

typedef enum {
    DTC_ETH_STATE_OK          = 0, /* 正常 */
    DTC_ETH_STATE_INIT_FAIL   = 1, /* 系统/芯片初始化失败 */
    DTC_ETH_STATE_LINK_FAIL   = 2, /* 链路失败（Link=Down 时有效） */
    DTC_ETH_STATE_SMI_FAIL    = 3, /* SMI 访问/控制失败 */
    DTC_ETH_STATE_COMM_ERR    = 4, /* 以太网通讯错误（业务层/协议层） */
} dtc_eth_state_e;

typedef enum {
    DTC_DDR_OK   = 0,
    DTC_DDR_ERR  = 1,
} dtc_ddr_state_e;

/* ---- 0xB201 电源工作状态 ---- */
typedef enum {
    LTE_PWR_UNDEF        = 0,
    LTE_PWR_OFF_MAINOFF  = 1, // 未供电(主电源未开)
    LTE_PWR_OFF_MDONLY   = 2, // 未供电(模块侧供电)
    LTE_PWR_3G_SLEEP     = 3,
    LTE_PWR_NORMAL       = 4,
    LTE_PWR_FULL         = 5,
    LTE_PWR_RSVD6        = 6,
    LTE_PWR_RSVD7        = 7,
    /* 8..15 预留 */
} lte_power_mode_e;

/* ---- 0xB210 网络状态 ---- */
typedef enum {
    OP_CHINA_MOBILE = 0,
    OP_CHINA_UNICOM = 1,
    OP_CHINA_TELEC  = 2,
} lte_operator_e;

typedef enum {
    RAT_GSM     = 0,
    RAT_EDGE    = 1,
    RAT_EGSM    = 2,
    RAT_WCDMA   = 3,
    RAT_TDSCDMA = 4,
    RAT_LTE_FDD = 5,
    RAT_LTE_TDD = 6,
    RAT_RSVD7   = 7,
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

/* ---- 0xB290：方位/定位状态 ---- */
typedef enum
{
    EW_EAST = 0,
    EW_WEST = 1
} ew_dir_e;

typedef enum
{
    NS_NORTH = 0,
    NS_SOUTH = 1
} ns_dir_e;

typedef enum
{
    FIX_SEARCHING = 0,
    FIX_2D = 1,
    FIX_3D = 2
} gps_fix_state_e;

typedef struct
{
    uint8_t ddrSet;
    uint8_t ddrRead;
    uint8_t ddrResult;
}DdrInfo_t;

/* ---------------- DTC 状态表 ---------------- */
typedef struct
{
    uint16_t storeState     : 2;        // [1:0]  内部存储器：0正常 1 eMMC错 2 SPI错 3保留
    uint16_t antMain        : 2;        // [3:2]  主天线：0正常 1开路 2短到电源 3短到地
    uint16_t antSec         : 2;        // [5:4]  副天线：0正常 1开路 2短到电源 3短到地
    uint16_t simFault       : 1;        // [6]    SIM：0 ready 1 fault
    uint16_t lteFault       : 1;        // [7]    4G内部通讯：0正常 1故障
    uint16_t ethState       : 3;        // [10:8]  以太网：0正常 1系统初始化失败 2链路失败 3SMI控制失败 4以太网通讯错误
    uint16_t ddrState       : 2;        // [12:11]  DDR：0正常 1故障 其他保留
    uint16_t overTemperature : 1;       // [13]  温度过高：0正常 1故障(超过120摄氏度))
    uint16_t rsv             : 2;       // [15:14]保留，写0
} DtcTable_t;

/* ---------------- 0xB201 模块工作状态（长度2B） ----------------
    Byte0:
        bit0-3 powerMode（电源工作状态）：
            0未定义 / 1未供电(主电源未开) / 2未供电(模块侧供电) / 3=3G休眠
            4=一般模式 / 5=全功能(数据/语音进行中) / 6~15保留
        bit4 (数据终端就绪) 主机可通讯：0不使能 1使能
        bit5 (数据设备就绪) 模块可通讯：0不使能 1使能
        bit6 (请求发送)     主机可接收数据：0不使能 1使能
        bit7 (清除发送)     模块不能接收数据：0不使能 1使能
    Byte1: 全保留 */
typedef struct
{
    /* Byte0 */
    uint16_t powerMode : 4;  // [3:0]
    uint16_t dtrReady  : 1;  // [4]
    uint16_t dsrReady  : 1;  // [5]
    uint16_t rtsReady  : 1;  // [6]
    uint16_t ctsReady  : 1;  // [7]
    /* Byte1 */
    uint16_t rsv8_15   : 8;  // [15:8] 保留
} LteModuleStatus_t;

/* ---------------- 0xB202 GNSS 模块工作状态（长度2B） ----------------
    Byte0: bit0 独立定位使能；bit1 定位模式(0单模/1混合GPS)；bit2 北斗使能；bit3 GPS使能；
            bit4 Glonass使能；bit5 前端融合(0不融合/1融合)；bit6 AGPS使能；bit7 协议(0 NMEA/1定制)
    Byte1: bit0 定位信息输出(0仅定位/1复合卫星)；bit1 Reserved；
            bit2 GPS定位信息；bit3 地面速度信息；bit4 当前卫星信息；
            bit5 可见卫星信息；bit6 推荐定位信息；bit7 地理定位信息 */
typedef struct
{
    /* Byte0 */
    uint16_t gnssEnableState : 1; // [0]
    uint16_t modeMixed       : 1; // [1]
    uint16_t beidouEnable    : 1; // [2]
    uint16_t gpsEnable       : 1; // [3]
    uint16_t glonassEnable   : 1; // [4]
    uint16_t fusionFront     : 1; // [5]
    uint16_t agpsEnable      : 1; // [6]
    uint16_t protoCustom     : 1; // [7]
    /* Byte1 */
    uint16_t outComposite    : 1; // [8]
    uint16_t rsv9            : 1; // [9]
    uint16_t outGGA          : 1; // [10]
    uint16_t outVTG          : 1; // [11]
    uint16_t outGSA          : 1; // [12]
    uint16_t outGSV          : 1; // [13]
    uint16_t outRMC          : 1; // [14]
    uint16_t outGLL          : 1; // [15]
} GnssModuleStatus_t;

/* ---------------- 0xB203 当前时间（长度7B） ---------------- */
typedef struct
{
    uint16_t year;    // Byte0:年(高) Byte1:年(低) 例如 0x20 0x19 => 2019
    uint8_t  month;   // Byte2:月(1..12)
    uint8_t  day;     // Byte3:日(1..31)
    uint8_t  hour;    // Byte4:时(0..23)
    uint8_t  minute;  // Byte5:分(0..59)
    uint8_t  second;  // Byte6:秒(0..59)
} TimeData_t;

/* ---------------- 0xB210 4G模块网络状态（长度4B） ----------------
    Byte0: 信号强度RSSI（0=≤-113dBm；1=-111dBm；2~30 每级+2dB到-53dBm；31=≥-53dBm；99=unknown）
    Byte1: 误码率BER（分辨率=1，偏移=0）
    Byte2: bit0-1 当前运营商(0移动/1联通/2电信)
            bit2-4 当前制式(0 GSM/1 EDGE/2 EGSM/3 WCDMA/4 TD-SCDMA/5 LTE_FDD/6 LTE_TDD 等)
            bit5-7 当前网络状态(0未注册未搜索/1已注册/2未注册搜索中/3注册被拒绝/4未知/5已注册漫游)
    Byte3: 当前内部故障代码 */
typedef struct
{
    uint8_t  csq;     // Byte0
    uint8_t  ber;      // Byte1
    struct {           // Byte2-3 合并成 16bit 位段，便于对齐
        uint16_t operatorType : 2; // [1:0]  0移动/1联通/2电信
        uint16_t rat          : 3; // [4:2]  制式
        uint16_t netState     : 3; // [7:5]  网络状态
        uint16_t faultCode    : 8; // [15:8] 故障代码(原Byte3)
    } b2_3;
} LteNetworkStatus_t;

/* ---------------- 0xB249 GPS卫星（1-4）信息（长度30B） ----------------
    Byte0: N（当前可见卫星数）
    卫星1：Byte1 ID, Byte2 海拔, Byte3-4 方位角, Byte5 C/N
    卫星2：Byte6 ID, Byte7 海拔, Byte8-9 方位角, Byte10 C/N
    卫星3：Byte11 ID, Byte12 海拔, Byte13-14 方位角, Byte15 C/N
    卫星4：Byte16 ID, Byte17 海拔, Byte18-19 方位角, Byte20 C/N
    DOP： 位置精度因子PDOP Byte21-23（24bit, 分辨率0.001）
            水平精度因子HDOP Byte24-26（24bit, 分辨率0.001）
            垂直精度因子VDOP Byte27-29（24bit, 分辨率0.001） */
typedef struct
{
    uint8_t visibleCount; // N
    struct {
        uint8_t  id;
        uint8_t  elevation; // 海拔(仰角)
        uint16_t azimuth;   // 方向角
        uint8_t  cnr;       // 信噪比 C/N
    } sat[4];
    uint8_t pdop[3]; // 24bit，分辨率0.001
    uint8_t hdop[3]; // 24bit，分辨率0.001
    uint8_t vdop[3]; // 24bit，分辨率0.001
} GpsSatInfo_1to4_t;

/* ---------------- 0xB24E 4G模块功能状态（长度2B） ----------------
    Byte0(bit0..7)：主UART口/SP I/TCP/USB模式/网络心跳/4G内置GPS/数字音频/AGPS 使能
    Byte1(bit0..7)：基站定位/AUX-UART 控制台/USB Modem/USB Diagnostic/USB Auxiliary/
                    UART Data/UART Trace/WLAN 使能 */
typedef struct
{
    uint16_t uartMainEn     : 1; // [0]
    uint16_t spiEn          : 1; // [1]
    uint16_t tcpEn          : 1; // [2]
    uint16_t usbModeEn      : 1; // [3]
    uint16_t netHeartbeatEn : 1; // [4]
    uint16_t lteGpsEn       : 1; // [5]
    uint16_t i2sDigitalAuEn : 1; // [6] 数字音频
    uint16_t agpsEn         : 1; // [7]
    uint16_t cellLocateEn   : 1; // [8] 基站定位
    uint16_t auxUartConsole : 1; // [9] AUX-UART 控制台
    uint16_t usbModemEn     : 1; // [10]
    uint16_t usbDiagEn      : 1; // [11]
    uint16_t usbAuxEn       : 1; // [12]
    uint16_t uartDataEn     : 1; // [13]
    uint16_t uartTraceEn    : 1; // [14]
    uint16_t wlanEn         : 1; // [15]
} LteFunctionStatus_t;

/* ---------------- 0xB257 以太网线路故障信息（长度2B） ----------------
    Byte0(bit0..7)：LAN_M/P短路/中断；信噪比警告；SYM_ERR_CNT；PWON失败；链路失败↔成功；
                    SMI控制失败；发送使能被钳位
    Byte1(bit0..3)：电源超限；芯片温度超限；唤醒后发送超时；电源策略关闭以太网
        (bit4..7 Reserved) */
typedef struct
{
    uint16_t lanShortOrBreak : 1; // [0] M/P 线路中断或相互短路
    uint16_t snrWarn         : 1; // [1] 线路信噪比警告
    uint16_t symErrInitFail  : 1; // [2] 初始化失败 SYM_ERR_CNT
    uint16_t pwonInitFail    : 1; // [3] 芯片启动/初始化失败
    uint16_t linkOkToFail    : 1; // [4] 链路成功→失败
    uint16_t linkFailToOk    : 1; // [5] 链路失败→成功
    uint16_t smiCtrlFail     : 1; // [6] SMI 控制失败
    uint16_t txEnableClamped : 1; // [7] 发送使能被钳位
    uint16_t powerOverLimit  : 1; // [8] 电源超限
    uint16_t chipOverTemp    : 1; // [9] 芯片温度超限
    uint16_t wakeTxTimeout   : 1; // [10] 唤醒后发送数据超时
    uint16_t pmCloseEth      : 1; // [11] 电源策略关闭以太网
    uint16_t rsv12_15        : 4; // [15:12] Reserved
} EthLineFaultInfo_t;

/* ---------------- 0xB290 GPS经纬度/搜星数（长度16B） ----------------
    Byte0-3 经度(longitude)；Byte4-7 纬度(latitude)
        度 = 值/10000000；分 = (值%10000000)/100000；秒 = (值%100000)*6/10000
    Byte8 东经/西经；Byte9 北纬/南纬；Byte10-11 高程；Byte12 速度；
    Byte13-14 方向角(相对正北)；Byte15 bit0-5 可使用卫星数，bit6-7 定位状态(0搜索中/1二维/2三维) */
typedef struct
{
    uint32_t longitude;   // Byte0-3
    uint32_t latitude;    // Byte4-7
    uint8_t  eastWest;    // Byte8  东/西（按平台定义取值）
    uint8_t  northSouth;  // Byte9  北/南（按平台定义取值）
    uint16_t altitude;    // Byte10-11
    uint8_t  speed;       // Byte12
    uint16_t heading;     // Byte13-14
    /* Byte15 */
    uint8_t  satUsed  : 6; // [5:0] 可使用卫星数
    uint8_t  fixState : 2; // [7:6] 0搜索中/1二维/2三维
} GpsPosition_t;

typedef struct
{
    DtcTable_t           dtc;                 // DTC table
    LteModuleStatus_t    lteModuleStatus;     // 0xB201
    GnssModuleStatus_t   gnssModuleStatus;    // 0xB202
    TimeData_t           timeData;            // 0xB203
    LteNetworkStatus_t   lteNetworkStatus;    // 0xB210
    GpsSatInfo_1to4_t    gpsSatInfo1to4;      // 0xB249
    LteFunctionStatus_t  lteFunctionStatus;   // 0xB24e
    EthLineFaultInfo_t   ethLineFaultInfo;    // 0xB257
    GpsPosition_t        gpsPosition;         // 0xB290
    // uint8_t              sensitiveData[16];   // 0xB2C5 //和MCU端保持一致
} ftyCircleDataToMcu_t;

void FtyPmCallback(const uint8_t* data, const uint16_t len, void* userdata);

void GetPmPowerState(MPUPmStateChange_t *pmData);

void FtyModemCallback(const uint8_t *data, const uint16_t len, void* userdata);

void FtyGnssMsgCallback(const uint8_t* data, const uint16_t len, void* userdata);

void FtyGnssNmeaCallback(const uint8_t *data, const uint16_t len, void *userdata);

void FtySyncVehicleEolDataAckCallback(const uint8_t *data, const uint16_t len, void *userdata);

void FtyMcuReqVehicleEolDataCallback(const uint8_t *data, const uint16_t len, void *userdata);

gboolean IsEthernetVersion(void);

void SetEthernetVersion(gboolean isEthernet);

int DdrCheckAndUpdate(void);

gpointer SendInfoToMCU(gpointer pArg);

gpointer SyncVehicleEolDataFromMcu(gpointer pArg);

void UpdateDdrResultRam(uint8_t value);

int RemoteDdrTestFile(void);

#endif