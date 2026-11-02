#include <iostream>
#include <fstream>
#include "tsp_parser_diagnosis.h"
#include "tsp_common.h"
#include "json/inc/json.h"
#include "tbox_uds_service.h"
#include "tsp_connect.h"
#include "mcu_api.h"
#include "tsp_log_report.h"
#include "tsp_gettoken.h"
#include "tsp_dbus_server.h"
#include "sgmw_ipc.h"

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                				 

using namespace std;

#define F510C                   //用于注释之前部分代码，确定后可将其删除

#define PM_SLEEP_EVENT_BUSYING      "busying"     // 系统启动中或者系统正在重启

#define  CONTROL_CMD_TIME_OUT_S     (15)

#define  OEMAPP_VERSION_PATH_NAME           "/oemapp/upgrade/version.txt"
#define  OEMDATA_VERSION_PATH_NAME          "/oemdata/tmp/version.txt"
#define  SYSTEM_CMD_TMP_FILE                "/oemdata/tmp/system_tmp_file"
#define  BLE_MAC_PATH_NAME                  "/oemdata/tmp/bleMac"
#define  BLE_MAC_FLASH_PATH_NAME            "/oemdata/flash/bleMac"

//MCU日志记录 aid 00 mid 00
#define MSG_MCU_LOG (SYS_QUE_TYPE_MCU_LOG_CAN & 0xffffff00)
#define MSG_SUBCMD_MCU_LOG_CAN (SYS_QUE_TYPE_MCU_LOG_CAN & 0xff) /*MCU的can日志-字符串,a00m00s01*/
#define MSG_SUBCMD_MCU_LOG_TSP (SYS_QUE_TYPE_MCU_LOG_TSP & 0xff) /*MCU的远控日志-字符串,a00m00s02*/
#define MSG_SUBCMD_MCU_LOG_PM (SYS_QUE_TYPE_MCU_LOG_PM & 0xff) /*MCU的电源管理日志-字符串,a00m00s03*/
#define MSG_SUBCMD_MCU_LOG_RDM (SYS_QUE_TYPE_MCU_LOG_RDM & 0xff) /*MCU的远程诊断日志-字符串,a00m00s04*/

//MCU调试记录 aid 00 mid 01
#define MSG_MCU_Debug_LOG (SYS_QUE_TYPE_MCU_DEBUG_LOG & 0xffffff00)
#define MSG_SUBCMD_MCU_Debug_LOG (SYS_QUE_TYPE_MCU_DEBUG_LOG & 0xff) /*MCU的DEBUG日志-字符串,a00m01s00*/

//蓝牙日志记录 aid 00 mid 02
#define MSG_BLE_LOG (SYS_QUE_TYPE_MCU_BLE_LOG_Stream & 0xffffff00)
#define MSG_SUBCMD_BLE_LOG_Stream (SYS_QUE_TYPE_MCU_BLE_LOG_Stream & 0xff) /*蓝牙日志-字节流,a00m02s00*/
#define MSG_SUBCMD_BLE_LOG_String (SYS_QUE_TYPE_MCU_BLE_LOG_String & 0xff) /*蓝牙日志-字符串,a00m02s01*/

//电源状态同步 aid 01 mid 04
#define MSG_MCU_SYNC_POWER (SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_MCU_SYNC_TO_CPU & 0xffffff00)
#define MSG_SUBCMD_RESERVATION_CHARGING_MCU_SYNC_TO_CPU (SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_MCU_SYNC_TO_CPU & 0xff)
#define MSG_SUBCMD_RESERVATION_CHARGING_CPU_RESP_TO_MCU (SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_CPU_RESP_TO_MCU & 0xff)
#define MSG_SUBCMD_RESERVATION_CHARGING_CONFIRM (SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_CONFIRM & 0xff)

/*Control subcmd*/
#define MSG_CONTROL (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_DOOR & 0xffffff00)    //aid 02 mid 40
#define MSG_SUBCMD_CONTROL_MCU_ACK (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SGMW_ACK & 0xff)
#define MSG_SUBCMD_CONTROL_RESULT (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SGMW_RES & 0xff)
#define MSG_SUBCMD_CONTROL_OPEN_DOOR (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_DOOR & 0xff)
#define MSG_SUBCMD_CONTROL_CLOSE_DOOR (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_DOOR & 0xff)
#define MSG_SUBCMD_CONTROL_OPEN_WINDOW (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_WINDOW & 0xff)
#define MSG_SUBCMD_CONTROL_CLOSE_WINDOW (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_WINDOW & 0xff)
#define MSG_SUBCMD_CONTROL_FLASH_WHISTLE (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_FLASH_WHISTLE & 0xff)
#define MSG_SUBCMD_CONTROL_OPEN_AC (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_AC & 0xff)
#define MSG_SUBCMD_CONTROL_CLOSE_AC (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_AC & 0xff)
#define MSG_SUBCMD_CONTROL_START_ENGINE (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_START_ENGINE & 0xff)
#define MSG_SUBCMD_CONTROL_TURNOFF_ENGINE (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_TURNOFF_ENGINE & 0xff)
#define MSG_SUBCMD_CONTROL_OPEN_SKYLIGHT (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_SKYLIGHT & 0xff)
#define MSG_SUBCMD_CONTROL_CLOSE_SKYLIGHT (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_SKYLIGHT & 0xff)
#define MSG_SUBCMD_CONTROL_OPEN_TRUNK (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_TRUNK & 0xff)
#define MSG_SUBCMD_CONTROL_CLOSE_TRUNK (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_TRUNK & 0xff)
#define MSG_SUBCMD_CONTROL_REMOTE_CHARGING (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_CHARGING & 0xff)
#define MSG_SUBCMD_CONTROL_ODER_CHARGING (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_ODER_CHARGING & 0xff)
#define MSG_SUBCMD_CONTROL_REMOTE_SPEED_LIMIT (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_SPEED_LIMIT & 0xff)
#define MSG_SUBCMD_CONTROL_PARKING_HEAT (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_PARKING_HEAT & 0xff)
#define MSG_SUBCMD_CONTROL_CHARGE_HEAT (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CHARGE_HEAT & 0xff)
#define MSG_SUBCMD_CONTROL_INTHE_CONTROL (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_INTHE_CONTROL & 0xff)
#define MSG_SUBCMD_CONTROL_SHARED_CAR_PREBOOK (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SHARED_CAR_PREBOOK & 0xff)
#define MSG_SUBCMD_CONTROL_SHARED_CAR_PREBOOK_CANCEL (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SHARED_CAR_PREBOOK_CANCEL & 0xff)
#define MSG_SUBCMD_CONTROL_SHARED_CAR_RETURN (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SHARED_CAR_RETURN & 0xff)
#define MSG_SUBCMD_CONTROL_OPEN_LIGHTS  (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_LIGHTS & 0xff)
#define MSG_SUBCMD_CONTROL_CLOSE_LIGHTS  (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_LIGHTS & 0xff)
#define MSG_SUBCMD_CONTROL_REMOTE_AUTHORIZED_IGNITION (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_AUTHORIZED_IGNITION & 0xff)
#define MSG_SUBCMD_CONTROL_OPEN_SENTINE_MODE (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_SENTINE_MODE & 0xff)
#define MSG_SUBCMD_CONTROL_CLOSE_SENTINE_MODE (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_SENTINE_MODE & 0xff)
#define MSG_SUBCMD_CONTROL_WINDOWS (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_WINDOWS & 0xff)
#define MSG_SUBCMD_CONTROL_REMOTE_PARKING (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_PARKING & 0xff)
#define MSG_SUBCMD_CONTROL_UNLOCK_THE_CAR (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_UNLOCK_THE_CAR & 0xff)
#define MSG_SUBCMD_CONTROL_REMOTE_PARKING_STA (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_PARKING_STA & 0xff)
#define MSG_SUBCMD_CONTROL_OTA_WAKE_UP (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OTA_WAKE_UP & 0xff)
#define MSG_SUBCMD_CONTROL_CHARGE_LIMIT (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CHARGE_LIMIT & 0xff)
#define MSG_SUBCMD_CONTROL_REMOTE_RESET (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_RESET & 0xff)
#define MSG_SUBCMD_CONTROL_OPEN_SEAT_HEAT (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_SEAT_HEAT & 0xff)
#define MSG_SUBCMD_CONTROL_CLOSE_SEAT_HEAT (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_SEAT_HEAT & 0xff)
#define MSG_SUBCMD_CONTROL_OPEN_SEAT_VENTILATE (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_SEAT_VENTILATE & 0xff)
#define MSG_SUBCMD_CONTROL_CLOSE_SEAT_VENTILATE (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_SEAT_VENTILATE & 0xff)
#define MSG_SUBCMD_CONTROL_OPEN_AC_OLD (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_AC_OLD & 0xff)
#define MSG_SBUCMD_CONTROL_BLE_BUSY  (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_BLE_BUSY & 0xff)
#define MSG_SBUCMD_CONTROL_BLE_NOT_FIND  (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_BLE_NOT_FIND & 0xff)
#define MSG_SBUCMD_CONTROL_BLE_KEY_MAC  (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_BLE_KEY_MAC & 0xff)

/*远控结果周期上报 subcmd*/
#define MSG_CONTROL_CYCLE_ACK (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_POWER_SUPPLY & 0xffffff00)    //aid 02 mid 42
#define MSG_SBUCMD_CONTROL_REMOTE_POWER_SUPPLY (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_POWER_SUPPLY & 0xff)   /*车辆控制结果周期上报-远程补电,a02m05s01*/
#define MSG_SBUCMD_CONTROL_REMOTE_MEMORY_PARKING (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_MEMORY_PARKING & 0xff)   /*车辆控制结果周期上报-远程泊车,a02m05s01*/
#define MSG_SBUCMD_CONTROL_UNLOCK_CAR (SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_UNLOCK_CAR & 0xff)   /*车辆控制结果周期上报-远程下电,a02m05s01*/



// aid 0x06 mid 0x40 or 0x41 子命令          //蓝牙钥匙下发和回馈
#define MSG_BLE_CONTROL_REQ                              (SYS_QUE_TYPE_MCU_CREATE_BLE_KEY & 0xffffff00)         //aid 06 mid 40
#define MSG_BLE_CONTROL_ACK                              (SYS_QUE_TYPE_MCU_CREATE_BLE_KEY_ACK & 0xffffff00)     //aid 06 mid 41
#define MSG_SUBCMD_CONTROL_CREATE_BLE_KEY                (SYS_QUE_TYPE_MCU_CREATE_BLE_KEY & 0xff)               //新建蓝牙钥匙
#define MSG_SUBCMD_CONTROL_UPDATE_BLE_SECRET_KEY         (SYS_QUE_TYPE_MCU_UPDATE_BLE_SECRET_KEY & 0xff)        //更新蓝牙钥匙密钥
#define MSG_SUBCMD_CONTROL_UPDATE_BLE_KEY_PERMISSION     (SYS_QUE_TYPE_MCU_UPDATE_BLE_KEY_PERMISSION & 0xff)    //更新蓝牙钥匙权限
#define MSG_SUBCMD_CONTROL_DELETE_SPECIFIED_BLE_KEY      (SYS_QUE_TYPE_MCU_DELETE_SPECIFIED_BLE_KEY & 0xff)     //删除指定蓝牙钥匙
#define MSG_SUBCMD_CONTROL_SELECT_BLE_KEY                (SYS_QUE_TYPE_MCU_SELECT_BLE_KEY & 0xff)               //查询（指定或者）全部蓝牙钥匙
#define MSG_SUBCMD_CONTROL_DELECT_BLE_KEY                (SYS_QUE_TYPE_MCU_DELECT_BLE_KEY & 0xff)               //删除所有蓝牙钥匙
#define MSG_SUBCMD_CONTROL_BLE_MAC                       (SYS_QUE_TYPE_MCU_BLE_MAC & 0xff)                      //蓝牙MAC


#pragma pack(1)
typedef struct
{
    guchar headerVersion;
    guchar flag;
    guint16 pkgLength;
    guchar eventID;
    guint32 randomData;
    guint16 serviceID;
    guint16 subfunction;
    guint16 payloadLength;
    guchar payloadAndCheck[0];
}IotDataFormat;

typedef enum
{
    RemoteHeatE     = 0x00,
    SpeedRemoteE    = 0x01,
    ParkingHeatE    = 0x02,
    SearchCarE      = 0x03,
    DoorLockE       = 0x04,
    DoorUnLockE     = 0x05,
    ReserverCharE   = 0x06,
    InTheControl    = 0x07,
    CarPreBook      = 0x08,
    CarPreBookCancel= 0x09,
    CarReturn       = 0x0A,
    WindowClose     = 0x0B,
    WindowOpen      = 0x0C,
    LightOpen       = 0x0E,
    LightClose      = 0x0F,
    PowerUp         = 0x10,
    PowerDown       = 0x11,
    OtaPowerUp      = 0x12,
    AcOff           = 0x13,
    AcOn            = 0x14,
    RemoteBLEEnable = 0x15,
    RemoteBLEKeyCrl = 0x16,
    SentineOpen     = 0x17,
    SentineClose    = 0x18,
    ControlWindowsE = 0x19,
    RemoteParkingE  = 0x20,
    UnlockTheCarE   = 0x21,
    RemoteParkStaE  = 0x22,
    OtaWakeupE      = 0x23,
    OpenTrunkE      = 0x24,
    CloseTrunkE     = 0x25,
    ChargeLimitE    = 0x27,
    RemoteReset     = 0x28,
    RemoteSeatHeat  = 0x29,
    RemoteSeatHeatClose = 0x30,         //与开启共用，此处用作占位
    RemoteSeatVentilate = 0x31,
    RemoteSeatVentilateClose = 0x32,    //与开启共用，此处用作占位
    MaxIndexE
}TimeOutIndex;

static guint32 g_DtcNumArray[128] = {0};
static guint32 g_dtcSum = 0;
static guint8 g_versionNum[128] = {0};
static guint32 g_versionLen = 0;
static guint32 g_dtcCode;
static guint8 g_dtcCodeStatus;
static STRUCT_UDS_DTC g_stDtcNumArray[128];
static STRUCT_UDS_DTC_SNAPSHOT g_udsDtcSnapShot;
static STRUCT_UDS_DTC_EXTENDED_DATA g_udsExtended;
static gint g_mcuRecvFd = -1;
static guint g_controlFlag = 0;
IotDataFormat remoteControlData;
IotDataFormat g_reserverChargingData;                       //预约充电
IotDataFormat *g_remoteHeatControlData;                     //远程加热
IotDataFormat *g_speedRemoteControlData;                    //限速
IotDataFormat *g_parkingHeatingRemoteControlData;           //驻车保温
IotDataFormat *g_searchCarControlData;                      //寻车
IotDataFormat *g_doorLockData;                              //闭锁
IotDataFormat *g_doorUnLockData;                            //解锁
IotDataFormat g_shardCarPreBook;                            //预约
IotDataFormat g_shardCarPreBookCancle;                      //取消预约
IotDataFormat g_shardCarReturn;                             //还车
IotDataFormat *g_windowOpenData;                            //开窗
IotDataFormat *g_windowCloseData;                           //关窗
IotDataFormat *g_lightOpenData;                             //开灯
IotDataFormat *g_lightCloseData;                            //关灯
IotDataFormat *g_powerUpData;                               //远程上电
IotDataFormat *g_powerDownData;                             //远程下电
IotDataFormat *g_otaPowerUpData;                            //ota上电
IotDataFormat *g_acOffData;                                 //远程关空调
IotDataFormat *g_acOnData;                                  //远程开空调
IotDataFormat *g_remoteBleEnableData;                       //远程授权点火
IotDataFormat *g_remoteBLEKeyData;                          //控制蓝牙钥匙操作
IotDataFormat *g_sintineOpenData;                           //打开哨兵模式
IotDataFormat *g_sintineCloseData;                          //关闭哨兵模式
IotDataFormat *g_controlWindows;                            //控制车窗
IotDataFormat *g_remoteParking;                             //远控泊车
static gchar g_remoteParkingFlag = 0;
IotDataFormat *g_remoteParkingStatus;                       //查询泊车状态
IotDataFormat *g_unlockTheCar;                              //下电锁车
IotDataFormat *g_openTrunk;                                 //打开电动尾门
IotDataFormat *g_closeTrunk;                                //关闭电动尾门
IotDataFormat *g_chargeLimit;                               //充电限值
IotDataFormat *g_remotereset;                               //远程重启
IotDataFormat *g_remoteSeatHeat;                            //远程座椅加热
IotDataFormat *g_remoteSeatHeatClose;                       //远程座椅加热 与开启共用，此处用作占位
IotDataFormat *g_remoteSeatVentilate;                       //远程座椅通风
IotDataFormat *g_remoteSeatVentilateClose;                  //远程座椅通风 与开启共用，此处用作占位

static gint g_timeOutArray[MaxIndexE];
ReserverCharging g_ReserverChargingTime = {0};
static guchar g_bleMac[7] = {0};
static char g_lightFlag = 0;
static char g_otaPowerOnFlag = 0;
static char g_otaPowerOffFlag = 0;
static guint16 g_controlCmd = 0x0000;

static pthread_mutex_t g_controlLock = PTHREAD_MUTEX_INITIALIZER;

//发送远程控制结果到tsp平台
gint SendRemoteControlResult(IotDataFormat *remoteControlData,guchar result,guint8 status);

//相应TSP平台远程控制
gint SendRemoteControlAck(IotDataFormat remoteControlData);

//周期上报远程控制状态
gint SendRemoteControlStatus(IotDataFormat remoteControlData,guchar status);

//发送远程指令到MCU
gint SendRemoteControlDataToMcu(guchar controlCmd ,guchar *controlDataBuff,gshort dataLen);

gint SendRemoteControlBLEKeyDataToMcu(guchar controlCmd ,guchar *controlDataBuff,gshort dataLen);

//发送复位mpu指令回包
gint SendRemoteControlResetAck(IotDataFormat remoteControlData,guchar status);

//发送远控空调回复
static gint SendRemoteControlAcOnAck(IotDataFormat *remoteControlData);

//发送远控座椅加热回复
static gint SendRemoteControlSeatAck(IotDataFormat *remoteControlData);

static gint GetSwVersion(const char *filePathName, guchar *pSwVersion,  gint swVersionLen);

static gint SendBLEKeyControlResultData(  guint8         cmd,
                                          IotDataFormat *controlData, 
                                          unsigned char *pResult, 
                                          guint16       resultLen, 
                                          gint          status);

static gint SendRemoteParkingAck(IotDataFormat *remoteControlData);

static gint SendUnlockTheCarAck(IotDataFormat *remoteControlData);



gint GetRdmRemoteCtrolRet(char *pwerOn, char *powerOff)
{
    if(NULL == pwerOn || NULL == powerOff)
    {
        return RET_ERR;
    }

    *pwerOn = g_otaPowerOnFlag;
    *powerOff = g_otaPowerOffFlag;

    return RET_OK;
}

gint InitOderChargingTime(const char *pFilePath, gint filePathLen)
{
    Json::Reader reader;
    Json::Value jsonValue;

    if(NULL == pFilePath)
        return RET_ERR;

    ifstream is(pFilePath, ios::binary);
    if (!is.is_open())
    {
        LOG_Debug("InitClientInfo open json file failed.");
        return RET_ERR;
    }

    pthread_mutex_lock(&g_controlLock);
    if(!reader.parse(is, jsonValue))
    {
        LOG_Error("InitOderChargingTime parse Json Error ");
        pthread_mutex_unlock(&g_controlLock);
        return RET_ERR;
    }
    // 读取预约充电开始时间
    g_ReserverChargingTime.flag = 0xFF & jsonValue["enable"].asInt();
    g_ReserverChargingTime.version = 0xFF & jsonValue["version"].asInt();

    {
        Json::Value v1;
        v1 = jsonValue["v1"];
        g_ReserverChargingTime.startTime = atoll(v1["startTime"].asString().c_str());
    }
    
    {
       Json::Value v2; 
       v2 = jsonValue["v2"];
       g_ReserverChargingTime.appointmentType = 0xFF & v2["appointmentType"].asInt();

       {
            Json::Value oderTime;
            oderTime = v2["oderTime"];
            {
                g_ReserverChargingTime.startTimeH = 0xFF & oderTime["startTimeH"].asInt();
                g_ReserverChargingTime.startTimeM = 0xFF & oderTime["startTimeM"].asInt();
                g_ReserverChargingTime.endTimeH = 0xFF & oderTime["endTimeH"].asInt();
                g_ReserverChargingTime.endTimeM = 0xFF & oderTime["endTimeM"].asInt();
                g_ReserverChargingTime.singleStartTime = atoll(oderTime["singleStartTime"].asString().c_str());
            } 
       }
    }

    g_ReserverChargingTime.sendFlag = 3;
    g_ReserverChargingTime.sendFlagTmp = 3;
    g_ReserverChargingTime.mcuSetState = 1;

    pthread_mutex_unlock(&g_controlLock);

    return RET_OK;
}


static gint SaveOderChargingTimeToFile(const char *pFilePath, gint filePathLen)
{
    Json::StyledStreamWriter writer;
    Json::Value jsonValue;
    ofstream os;
    struct timeval newTime;
    char timeStr[32] = {0};
    
    if(NULL == pFilePath)
        return RET_ERR;
    
    os.open(pFilePath);
    if (!os.is_open())
    {
        LOG_Debug("SaveVinToFile open json file failed.");
        return RET_ERR;
    }
                                                                                                                                                                                                                                                                                                                                                                                                                                         
    gettimeofday(&newTime, NULL);
    pthread_mutex_lock(&g_controlLock);
    jsonValue["enable"] = g_ReserverChargingTime.flag;
    jsonValue["version"] = g_ReserverChargingTime.version;
    snprintf(timeStr, 32, "%lld", g_ReserverChargingTime.startTime);

    {
        // v1赋值
        Json::Value v1;
        v1["startTime"] = timeStr;
        jsonValue["v1"] = v1;
        LOG_Debug("%s", timeStr);
    }

    {
        Json::Value v2;
        v2["appointmentType"] = g_ReserverChargingTime.appointmentType;

        {
            // 预约每日参数赋值
            Json::Value oderTime;
            oderTime["startTimeH"] = g_ReserverChargingTime.startTimeH;
            oderTime["startTimeM"] = g_ReserverChargingTime.startTimeM;
            oderTime["endTimeH"] = g_ReserverChargingTime.endTimeH;
            oderTime["endTimeM"] = g_ReserverChargingTime.endTimeM;
            memset(timeStr, 0 ,32);
            snprintf(timeStr, 32, "%lld", g_ReserverChargingTime.singleStartTime);
            oderTime["singleStartTime"] = timeStr;
            v2["oderTime"] = oderTime;
        }

        jsonValue["v2"] = v2;
        LOG_Debug("%s", timeStr);
    }


    memset(timeStr, 0 ,32);
    snprintf(timeStr, 32, "%d", newTime.tv_sec);
    jsonValue["writeTime"] = timeStr;
    writer.write(os, jsonValue);
    pthread_mutex_unlock(&g_controlLock);
    os.close();

    return RET_OK;
}

gint SendDiagnosisData(gpointer diagnosisData, guint u32Len, guchar dataType)
{
    gint errorReturn;
    if(NULL == diagnosisData)
    {
        LOG_Error("diagnosisData is NULL");
        return RET_ERR;
    }
    Json::Value jsonValue;
    Json::FastWriter jsonWriter;
    gchar *iotData = (gchar *)g_try_malloc0(1024);
    if(NULL == iotData)
    {
        LOG_Error("iotData malloc error");
        return RET_ERR;
    }
    gchar deviceId[128] = {0};
    gchar dataStr[15] = {0};
    gchar sendTopic[128] = {0};
    

    time_t stTmpTime;
    struct tm *pstRetTime;
    time(&stTmpTime);
    stTmpTime += 28800;
    pstRetTime = gmtime(&stTmpTime);
    iotData[0] = ((pstRetTime->tm_year + 1900) / 100);
    iotData[1] = ((pstRetTime->tm_year + 1900) % 100);
    iotData[2] = pstRetTime->tm_mon + 1;
    iotData[3] = pstRetTime->tm_mday;
    iotData[4] = pstRetTime->tm_hour;
    iotData[5] = pstRetTime->tm_min;
    iotData[6] = pstRetTime->tm_sec;
    HexToStr((guchar *)dataStr,(guchar *)iotData,7);
    jsonValue["DATE"] = dataStr;

    LOG_Debug("dataStr:%s",dataStr);
    GetDeviceID(deviceId);
 
    if(dataType == 0x03)
    {
        // 03远控反馈上报
        jsonValue["TY"] = "03";
        sprintf(sendTopic,"%s%s/data/03",E50_UPLINK_TOPIC,deviceId);
    }
    else if(dataType == 0x02)
    {
        // 02主动上报
        jsonValue["TY"] = "02";
        sprintf(sendTopic,"%s%s/data/02",E50_UPLINK_TOPIC,deviceId);
    }

    

    jsonValue["VIN"] = deviceId;
    memset(iotData, 0,1024);
    LOG_Debug("u32Len:%d",u32Len);
    if(u32Len >= 1024)
    {
        g_free(iotData);
        return RET_ERR;
    }
    HexToStr((guchar *)iotData,(guchar *)diagnosisData,u32Len);
    jsonValue["IOT"] = iotData;
    LOG_Debug("iotData:%s",iotData);

    LOG_Debug("%s",jsonWriter.write(jsonValue).c_str());
    //printf("%s\n",jsonWriter.write(jsonValue).c_str());
    LOG_Debug("sendTopic :%s", sendTopic);
    errorReturn = SendPackageData(sendTopic,E50_QOS1,(gpointer)(jsonWriter.write(jsonValue).c_str()),jsonWriter.write(jsonValue).length());
    LOG_Debug("sendTopic :%s, errorReturn:%d", sendTopic, errorReturn);
    //return SendPackageData(sendTopic,E50_QOS1,(gpointer)(jsonWriter.write(jsonValue).c_str()),jsonWriter.write(jsonValue).length());
    g_free(iotData);
    return errorReturn;
}


static guint16 CalCrcCal(const guint8 *buf, const guint32 size)
{
	guint32  i,j;
	guint16 curCrc = 0xFFFF, m_Crc=0;    

	for (i = 0; i < size; i ++)
	{
		for(j = 0; j < 8;j++)
		{
			m_Crc = (((guint16)buf[i] << j) & 0x80) ^ ((curCrc & 0x8000) >> 8);
			curCrc = curCrc << 1;
			if(m_Crc != 0)
			{
				curCrc= curCrc ^ 0x1021;
			}
		}
        //printf("%02x ",buf[i]);
	}
    //printf("\n");
    LOG_Debug("curCrc%04x",curCrc);
    return curCrc;
}

static int GetCpuVersionFromTxt(guint8 *value)
{
    int  fileFd = -1;
    char tokenBuffer[32] = {0};
    char version[8]={0};
    char gVersion[9];
	//int value[4];
    guint8 i;
    fileFd = open(TBOXCPUVERSIONNAME,O_RDONLY);
    if(-1 == fileFd)
    {
        LOG_Debug("open version.txt failed\n");
        return -1;
    }
    else
    {
        if(read(fileFd,tokenBuffer,sizeof(tokenBuffer)) > 0)
        {
         	close(fileFd);
          	//printf("tokenBuffer:%s\n",tokenBuffer);
            strncpy(version,tokenBuffer+5,3);
            sprintf(gVersion,".%s",version);
            //printf("gVersion %s\n",gVersion);
            for(i=0;i<strlen(gVersion);i++)
            { 
                value[i] = gVersion[i];
                //printf("version value[%d]=%x\n",i,value[i]);
            }
		    return 0;
        }
        else
        {
            close(fileFd);
            return -2;
        }
    }
}

#define TBOXUPGRADESTATUS			"/oemdata/tmp/upgrade_status"


//-1 找不到文件
//-3 不处于升级状态
//0 处于升级状态
//-2 文件大小不对
static int GetUpgradeStatus()
{
	int fileFd = -1;
    char status[3];
	int st;
    fileFd = open(TBOXUPGRADESTATUS,O_RDONLY);
    if(-1 == fileFd)
    {
        LOG_Debug("open upgrade_status failed");
        return -1;
    }
    else
    {
        if(read(fileFd,status,3) > 0)
        {
            close(fileFd);
	        st = atoi(status);
	        LOG_Debug("get xhf upgrade status %d",st);
          	if(st != 1)
			    return -3;
		    return 0;
        }
        else
        {
            close(fileFd);
            return -2;
        }
    }
}


static guint16 PackrDiagnosisPayloadData(IotDataFormat *formatIotData,IotDataFormat *resultIotDataFormat)
{
    guint16 dataOffset = 0;
    resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x00;
    resultIotDataFormat->payloadAndCheck[dataOffset++] = formatIotData->payloadAndCheck[0];
    resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x07;
    resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x62;
    resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x07;
    resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x6A;
    switch(resultIotDataFormat->payloadAndCheck[0])
    {
        case 0:
        {
            gint dtcLen = g_dtcSum * 4;
            gint i = 0;
            resultIotDataFormat->payloadAndCheck[dataOffset++] =  dtcLen / 256;
            resultIotDataFormat->payloadAndCheck[dataOffset++] =  dtcLen % 256;
            for(i = 0; i < g_dtcSum; i++)
            {
                resultIotDataFormat->payloadAndCheck[dataOffset++] = (g_stDtcNumArray[i].dtcCode >> 16) & 0xFF;
                resultIotDataFormat->payloadAndCheck[dataOffset++] = (g_stDtcNumArray[i].dtcCode >> 8) & 0xFF;
                resultIotDataFormat->payloadAndCheck[dataOffset++] = g_stDtcNumArray[i].dtcCode & 0xFF;
                resultIotDataFormat->payloadAndCheck[dataOffset++] = g_stDtcNumArray[i].status;
            }
        }
        break;
        case 1:
        {
            gint dtcLen = 30;
            resultIotDataFormat->payloadAndCheck[dataOffset++] =  dtcLen / 256;
            resultIotDataFormat->payloadAndCheck[dataOffset++] =  dtcLen % 256;

            //dtc
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_dtcCode & 0xFF;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = (g_dtcCode >> 8) & 0xFF;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = (g_dtcCode >> 16) & 0xFF;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_dtcCodeStatus;
            
            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0xF0;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x11;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.ecuVoltage;

            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0xF0;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x12;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.motorSpeed / 256;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.motorSpeed % 256;

            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0xF0;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x13;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.vehicleSpeed / 256;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.vehicleSpeed % 256;

            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0xF0;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x14;            
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.vehicleOdlmeter & 0xFF;            
            resultIotDataFormat->payloadAndCheck[dataOffset++] = (g_udsDtcSnapShot.vehicleOdlmeter >> 8) & 0xFF;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = (g_udsDtcSnapShot.vehicleOdlmeter >> 16) & 0xFF;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = (g_udsDtcSnapShot.vehicleOdlmeter >> 24) & 0xFF;

            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0xF0;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x10;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.globalRealTime[0];
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.globalRealTime[1];
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.globalRealTime[2];
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.globalRealTime[3];
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.globalRealTime[4];
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.globalRealTime[5];
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsDtcSnapShot.globalRealTime[6];
        }
        break;
        case 2:
        {
            gint dtcLen = 8;
            resultIotDataFormat->payloadAndCheck[dataOffset++] =  dtcLen / 256;
            resultIotDataFormat->payloadAndCheck[dataOffset++] =  dtcLen % 256;

            //dtc        
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_dtcCode & 0xFF;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = (g_dtcCode >> 8) & 0xFF;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = (g_dtcCode >> 16) & 0xFF;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_dtcCodeStatus;

            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x01;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsExtended.faultOccurrenceCounter;

            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x02;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsExtended.faultPendingCounter;

            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x03;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsExtended.dtcAgingCounter;

            resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x04;
            resultIotDataFormat->payloadAndCheck[dataOffset++] = g_udsExtended.dtcAgedCounter;
        }
        break;
    }
    LOG_Debug("dataOffset:%d",dataOffset);
    return dataOffset;
}


static guint16 PackrReadDataPayloadData(IotDataFormat *formatIotData,IotDataFormat *resultIotDataFormat)
{
    guint16 dataOffset = 0;
    guint8 cpuValue[4] = {0x2e,0x30,0x30,0x30};
    resultIotDataFormat->payloadAndCheck[dataOffset++] = formatIotData->payloadAndCheck[0];
    resultIotDataFormat->payloadAndCheck[dataOffset++] = formatIotData->payloadAndCheck[1];
    
    resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x07;
    resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x62;
    resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x07;
    resultIotDataFormat->payloadAndCheck[dataOffset++] = 0x6A;
    if(formatIotData->payloadAndCheck[0] == 0xF1 && formatIotData->payloadAndCheck[1] == 0x95)
    {
   	resultIotDataFormat->payloadAndCheck[dataOffset++] = ((g_versionLen+4) >> 8);
    	resultIotDataFormat->payloadAndCheck[dataOffset++] = g_versionLen+4;
    }else
    {
	resultIotDataFormat->payloadAndCheck[dataOffset++] = (g_versionLen >> 8);
        resultIotDataFormat->payloadAndCheck[dataOffset++] = g_versionLen;
    }
    memcpy(&resultIotDataFormat->payloadAndCheck[dataOffset],g_versionNum,g_versionLen);
    dataOffset += g_versionLen;
    LOG_Debug("g_versionLen:%d:%s",g_versionLen,g_versionNum);
    LOG_Debug("dataOffset:%d",dataOffset);
	if(formatIotData->payloadAndCheck[0] == 0xF1 && formatIotData->payloadAndCheck[1] == 0x95)
	{
		GetCpuVersionFromTxt(cpuValue);
		memcpy(&resultIotDataFormat->payloadAndCheck[dataOffset],cpuValue,4);
		dataOffset += 4;
		LOG_Debug("cpu_version:%s",cpuValue);
	}
    return dataOffset;
}


static gint PackResultIotData(IotDataFormat *formatIotData,gint result)
{
    if(NULL == formatIotData)
    {
        LOG_Error("formatIotData is NULL ");
        return RET_ERR;
    }
    guint16 payloadLen = 0;
    guint16 crcData = 0;
    IotDataFormat *resultIotDataFormat = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 1024);
    if(NULL == resultIotDataFormat)
    {
        LOG_Error("resultIotDataFormat malloc error ");
        return RET_ERR;
    }
    LOG_Debug("formatIotData->pkgLength: %04x",formatIotData->pkgLength);
    resultIotDataFormat->headerVersion = formatIotData->headerVersion;
    resultIotDataFormat->flag = formatIotData->flag;
    resultIotDataFormat->pkgLength = formatIotData->pkgLength;
    resultIotDataFormat->eventID = formatIotData->eventID;
    resultIotDataFormat->randomData = formatIotData->randomData;
    if(RET_OK == result)
    {
        resultIotDataFormat->serviceID = 0x6B94;
        resultIotDataFormat->subfunction = formatIotData->subfunction;
        if(0x0003 == formatIotData->subfunction)
        {
            payloadLen = 0x0002;
        }
        else
        {
            //调用每个服务的payload接口
            switch(formatIotData->subfunction)
            {
                case 0x0500:
                case 0x0400:
                case 0x0100:
                {
                   payloadLen = PackrDiagnosisPayloadData(formatIotData,resultIotDataFormat);
                }
                break;
                case 0x0200:
                {
                    payloadLen = PackrReadDataPayloadData(formatIotData,resultIotDataFormat);
                }
                break;
                default:
                {
                    payloadLen = 0x0002;
                }
            }
            
        }
    }
    else
    {
        resultIotDataFormat->serviceID = 0xFFFF;
        resultIotDataFormat->subfunction =  0xDB24;
        payloadLen = 0x0006;
        //设置errorCode 
        switch(result)
        {
            case -1:
            case -3:
                resultIotDataFormat->payloadAndCheck[0] = 0x0D;
                break;
            case -2:
            case -7:
                resultIotDataFormat->payloadAndCheck[0] = 0x0B;
                break;
            case -4:
                resultIotDataFormat->payloadAndCheck[0] = 0x04;
                break;
            case -5:
                resultIotDataFormat->payloadAndCheck[0] = 0x05;
                break;
            case -6:
                resultIotDataFormat->payloadAndCheck[0] = 0x0E;
                break;
        }
    }
    LOG_Debug("payloadLen:%d",payloadLen);
    payloadLen += 2;
    resultIotDataFormat->payloadLength = (payloadLen % 256) * 256 + (payloadLen / 256);
    payloadLen = payloadLen + 15;
    crcData = CalCrcCal((guint8 *)resultIotDataFormat,payloadLen - 2);
    resultIotDataFormat->payloadAndCheck[payloadLen - 17] = crcData / 256;
    resultIotDataFormat->payloadAndCheck[payloadLen - 16] = crcData % 256;
    
    LOG_Debug("payloadLen :%d",payloadLen);
    SendDiagnosisData((gpointer)resultIotDataFormat,payloadLen,3);
    LOG_Debug("resultIotDataFormat->payloadLength:%04x",resultIotDataFormat->payloadLength);
    LOG_Debug("resultIotDataFormat->pkgLength:%04x",resultIotDataFormat->pkgLength);
    g_free(resultIotDataFormat);
    return RET_OK;
}

static gint SendReserverChargingResultData(IotDataFormat controlData)
{
    gint retValue = -1;
    guint16 craData = 0;
    IotDataFormat *pIotData = NULL;

    pIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 2);
    if(NULL == pIotData)
    {
        LOG_Error("SendControlResultData malloc error");
        return retValue;
    }

    pIotData->headerVersion = controlData.headerVersion;
    pIotData->flag          = controlData.flag;
    pIotData->pkgLength     = controlData.pkgLength;
    pIotData->eventID       = controlData.eventID;
    pIotData->randomData    = controlData.randomData;


    pIotData->serviceID   = 0x6D92;
    pIotData->subfunction = 0x0A00;
  
    pIotData->payloadLength = 0x0200;

    craData = CalCrcCal((guint8 *)pIotData, htons(pIotData->pkgLength));
    pIotData->payloadAndCheck[0] = craData / 256;
    pIotData->payloadAndCheck[1] = craData % 256;
   
    retValue = SendDiagnosisData((gpointer)pIotData,(2 + htons(pIotData->pkgLength)), 3);

    g_free(pIotData);

    return retValue;
}


gint GetReserverChargingTime(ReserverCharging *pReserverChargingData)
{
    gint retValue = RET_ERR;

    if(NULL == pReserverChargingData)
        return retValue;
    
    pthread_mutex_lock(&g_controlLock);
    memcpy(pReserverChargingData, &g_ReserverChargingTime, sizeof(ReserverCharging));
    pthread_mutex_unlock(&g_controlLock);

    return RET_OK;
}
gint SetReserverChargingTime(ReserverCharging *pReserverChargingData)
{
    gint retValue = RET_ERR;

    if(NULL == pReserverChargingData)
        return retValue;
    
    pthread_mutex_lock(&g_controlLock);
    memcpy(&g_ReserverChargingTime, pReserverChargingData, sizeof(ReserverCharging));
    pthread_mutex_unlock(&g_controlLock);

    return RET_OK;
}

static gint ControlCarSpeed(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR;
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    /*控制协议转换 */
    switch(pControlData[2])
    {
        case 0x00:
            controlBuff[0] = 0x00;
            break;
        case 0x40:
            controlBuff[0] = 0x01;
            break;
        case 0x80:
            controlBuff[0] = 0x02;
            break;
        case 0xC0:
            controlBuff[0] = 0x03;
            break;
        case 0x01:
            controlBuff[0] = 0x04;
            break;
        case 0x02:
            controlBuff[0] = 0x05;
            break;
        case 0x03:
            controlBuff[0] = 0x06;
             break;
        case 0x04:
            controlBuff[0] = 0x07;
             break;
        case 0x05:
            controlBuff[0] = 0x08;
             break;
        case 0x06:
            controlBuff[0] = 0x09;
            break;
        default:
            controlBuff[0] = 0x00;
    }

    return SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_REMOTE_SPEED_LIMIT, controlBuff, 4);
}

static gint ControlParkingHeat(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    /*控制协议转换*/
    switch(pControlData[2])
    {
        case 0x01:
            controlBuff[0] = 0x01;
            break;
        case 0x02:
            controlBuff[0] = 0x02;
            break;
        default:
            controlBuff[0] = 0x02;
    }
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_PARKING_HEAT, controlBuff, 4);

    return retValue;
}

static gint ControlRemoteHeat(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    /*控制协议转换*/
    switch(pControlData[2])
    {
        case 0x01:
            controlBuff[0] = 0x01;
            break;
        case 0x02:
            controlBuff[0] = 0x02;
            break;
        default:
            controlBuff[0] = 0x01;
    }

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_CHARGE_HEAT, controlBuff, 4);

    return retValue;
}

static gint ControlDoorUnLock(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};
    guchar controlFlag = 0;

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

	if(pControlData[2] & 0x80)
    {
        controlFlag = 1;
        LOG_Debug("左前门解锁");
    }
	if(pControlData[2] & 0x40)
    {
        controlFlag = 1;
        LOG_Debug("右前门解锁");
    }

    if(pControlData[2] & 0x20)
    {
        controlFlag = 3;
        LOG_Debug("左后门解锁");
    }

    if(pControlData[2] & 0x10)
    {
        controlFlag = 3;
        LOG_Debug("右后门解锁");
    }
	if(pControlData[2] & 0x08)
    {
         
        LOG_Debug("尾门解锁1");
        //controlBuff[0] = pControlData[2];
        controlBuff[0] = 0x00;
        retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_TRUNK, controlBuff, 4);
        return RET_OK;
    }
	if(pControlData[2] & 0x04)
    {
        controlFlag = 3;
        LOG_Debug("引擎盖解锁");
    }
	if(pControlData[2] & 0x02)
    {
        controlFlag = 3;
        LOG_Debug("中控解锁");
    }

	controlBuff[0] = pControlData[2];

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_DOOR, controlBuff, 4);

    return RET_OK;
}

static gint ControlTrunkOpen()
{
    guchar controlBuff[4] = {0};

    controlBuff[0] = 0x01;
    return SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_TRUNK, controlBuff, 4);
}

static gint ControlTrunkClose()
{
    guchar controlBuff[4] = {0};
    //在MCU层：00 关闭尾门 01：关闭电动尾门
    controlBuff[0] = 0x01;
    return SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_CLOSE_TRUNK, controlBuff, 4);
}

static gint ControlDoorLock(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    /*控制协议转换*/
    /*控制协议转换*/
    if(pControlData[2] & 0x80)
        LOG_Debug("左前门闭锁");

    if(pControlData[2] & 0x40)
        LOG_Debug("右前门闭锁");

    if(pControlData[2] & 0x20)
        LOG_Debug("左后门闭锁");

    if(pControlData[2] & 0x10)
        LOG_Debug("右后门闭锁");

    if(pControlData[2] & 0x02)
        LOG_Debug("中控闭锁");

    controlBuff[0] = pControlData[2];

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_CLOSE_DOOR, controlBuff, 4);

    return retValue;
}

static gint ControlWindowClose(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};
    guchar controlFlag = 0;

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    /*控制协议转换*/
    /*控制协议转换*/
    if(pControlData[2] & 0x80)
    {
        LOG_Debug("左前窗升窗");
		
        controlFlag =controlFlag | 0x01;
    }

    if(pControlData[2] & 0x40)
    {
        LOG_Debug("右前窗升窗");
		
        controlFlag =controlFlag | 0x02;
    }

    if(pControlData[2] & 0x20)
    {
        LOG_Debug("左后窗升窗");
		
        controlFlag =controlFlag | 0x04;
    }

    if(pControlData[2] & 0x10)
    {
        LOG_Debug("右后窗升窗");
		
        controlFlag =controlFlag | 0x08;
    }
    // 关闭车窗与其他车窗关闭无法在同一条指令下实现
    if(pControlData[2] & 0x08)
    {
        LOG_Debug("天窗关闭");
        retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_CLOSE_SKYLIGHT, controlBuff, 4);
        return retValue;
    }
	
    controlBuff[0] = controlFlag;//pControlData[2];

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_CLOSE_WINDOW, controlBuff, 4);

    return retValue;
}

static gint ControlWindowOpen(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};
	guchar controlFlag = 0;

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    /*控制协议转换*/
    /*控制协议转换*/
    if(pControlData[2] & 0x80)
    {
        LOG_Debug("左前窗降窗");
		controlFlag =controlFlag | 0x01;
    }

    if(pControlData[2] & 0x40)
    {
        LOG_Debug("右前窗降窗");
		controlFlag =controlFlag | 0x02;
    }

    if(pControlData[2] & 0x20)
    {
        LOG_Debug("左后窗降窗");
		controlFlag =controlFlag | 0x04;
    }

    if(pControlData[2] & 0x10)
    {
        LOG_Debug("右后窗降窗");
		controlFlag =controlFlag | 0x08;
    }
    // 天窗无法与其他窗户同一条指令打开
    if(pControlData[2] & 0x08)
    {
        LOG_Debug("天窗打开");
        retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_SKYLIGHT, controlBuff, 4);
        return retValue;
    }

    controlBuff[0] =controlFlag;// pControlData[2];

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_WINDOW, controlBuff, 4);

    return retValue;
}
static gint ControlSkyLightClose(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    /*控制协议转换*/
    /*控制协议转换*/
  

    if(pControlData[2] & 0x08)
        LOG_Debug("天窗关闭");

    controlBuff[0] = pControlData[2];

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_CLOSE_SKYLIGHT, controlBuff, 4);

    return retValue;
}


static gint ControlSkyLightOpen(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    /*控制协议转换*/
    /*控制协议转换*/
    

    if(pControlData[2] & 0x08)
        LOG_Debug("天窗打开");

    controlBuff[0] = pControlData[2];

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_SKYLIGHT, controlBuff, 4);

    return retValue;
}


static gint ControlLightClose(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    /*控制协议转换*/
    /*控制协议转换*/
    if(pControlData[2] & 0x80)
    {
        LOG_Debug("近光灯关闭1");
		controlBuff[0] = 0x01;
    }

    if(pControlData[2] & 0x40)
    {
        LOG_Debug("远光灯关闭1");
		controlBuff[0] = 0x02;
    }

  //  controlBuff[0] = pControlData[2];

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_CLOSE_LIGHTS, controlBuff, 4);

    return retValue;
}

static gint ControlLightOpen(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    /*控制协议转换*/
    /*控制协议转换*/
    if(pControlData[2] & 0x80)
    {
        LOG_Debug("近光灯开启1");
		controlBuff[0] = 0x01;
        g_lightFlag = 1;
    }

    if(pControlData[2] & 0x40)
    { 
        LOG_Debug("远光灯开启1");
		controlBuff[0] = 0x02;
        g_lightFlag = 2;
    }

    //controlBuff[0] = pControlData[2];

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_LIGHTS, controlBuff, 4);

    return retValue;
}

static gint ControlRemotePowerUp(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};
    controlBuff[0] = 0x03;
    controlBuff[1] = 0x00;
    controlBuff[2] = 0x00;
    controlBuff[3] = 0x00;
    LOG_Debug("远程上电");
    g_otaPowerOnFlag = 0x01;
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_START_ENGINE, controlBuff, 4);
    return retValue;
}

gint ControlRemotePowerDown(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    LOG_Debug("远程下电");

    g_otaPowerOffFlag = 0x01;
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_TURNOFF_ENGINE, controlBuff, 4);

    return retValue;
}


gint ZelosControlRemotePowerup()
{
    gint retValue = RET_ERR;
    guchar controlBuff[4] = {0};
    controlBuff[0] = 0x03;
    controlBuff[1] = 0x00;
    controlBuff[2] = 0x00;
    controlBuff[3] = 0x00;
    LOG_Debug("Zelos Remote Powerup!");
    printf("Zelos Remote Powerup!\n");
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_START_ENGINE, controlBuff, 4);
    return retValue;
}


gint ZelosControlRemotePowerdown()
{
    gint retValue = RET_ERR;
    guchar controlBuff[4] = {0};

    LOG_Debug("Zelos Remote Powerdown!");
    printf("Zelos Remote Powerdown!\n");
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_TURNOFF_ENGINE, controlBuff, 4);
    return retValue;
}


gint ControlOTAPowerUp(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    controlBuff[0] = 2;
    LOG_Debug("OTA上电");

    g_otaPowerOnFlag = 0x01;
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_START_ENGINE, controlBuff, 4);

    return retValue;
}

static gint ControlOTAWakeUp()
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    LOG_Debug("OTAWakeup");

    g_timeOutArray[OtaWakeupE] = 2;
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OTA_WAKE_UP, controlBuff, 4);

    return retValue;
}

static gint ControlRemoteBLEEnable(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    LOG_Debug("远程授权点火");
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_REMOTE_AUTHORIZED_IGNITION, controlBuff, 4);

    return retValue;
}

static gint ControlWindows(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
    {
        LOG_Error("远程控制车窗位置 argv error ");
        return RET_ERR;
    }

    LOG_Debug("远程控制车窗位置");
    controlBuff[0] = pControlData[2];
    controlBuff[1] = pControlData[3];

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_WINDOWS, controlBuff, 4);

    return retValue;
}

static gint ControlRemoteParking(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
    {
        LOG_Error("远控泊车 argv error ");
        return RET_ERR;
    }

    if(0x04 != pControlData[0])
    {
        LOG_Error("远控泊车 argv error :%d", pControlData[0]);
        return RET_ERR;
    }

    if(0x0B == pControlData[1] || 0x0D == pControlData[1] || \
       0x07 == pControlData[1] || 0x03 == pControlData[1])
    {
        g_remoteParkingFlag = 1;
    }
    else
    {
        g_remoteParkingFlag = 2;
    }

    LOG_Debug("远控泊车[%02x][%02x]", pControlData[1], pControlData[2]);
    controlBuff[0] = pControlData[1];
    controlBuff[1] = pControlData[2];

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_REMOTE_PARKING, controlBuff, 4);

    return retValue;
}

static gint ControlRemoteParkingSta(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData)
    {
        LOG_Error("远控泊车 argv error ");
        return RET_ERR;
    }

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_REMOTE_PARKING_STA, controlBuff, 4);

    return retValue;
}

static gint ControlAC(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    controlBuff[0] = pControlData[2];
	controlBuff[1] = pControlData[3];
	controlBuff[2] = pControlData[4];
    if(pControlData[2] & 0x80)
    {   
        
		LOG_Debug("打开空调");
        retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_AC, controlBuff, 4);
    } 
	else
	{
	    LOG_Debug("关闭空调");
	    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_CLOSE_AC, controlBuff, 4);
	}

    return retValue;
}

static gint ControlACOff(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    controlBuff[0] = pControlData[2];
	controlBuff[1] = pControlData[3];
	controlBuff[2] = pControlData[4];

   // if(pControlData[2] & 0x80)
   // {   
        
   //	LOG_Debug("打开空调");
   //     retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_AC, controlBuff, 4);
   // } 
	//else
	//{
	    LOG_Debug("关闭空调");
	    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_CLOSE_AC, controlBuff, 4);
	//}

    return retValue;
}


static gint ControlACOn(IotDataFormat *pIotData) 
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pIotData || NULL == pIotData->payloadAndCheck || pIotData->payloadLength < 6)
    {
        LOG_Debug("pIotData is error");
        return retValue;
    }

    controlBuff[0] = (pIotData->payloadAndCheck[2] >> 7) & 0x01;
    controlBuff[1] = (pIotData->payloadAndCheck[2] >> 3) & 0x0F;
	controlBuff[2] = pIotData->payloadAndCheck[3];
	controlBuff[3] = pIotData->payloadAndCheck[4];
 
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_AC_OLD, controlBuff, 4);
  
    return retValue;
}

static gint ControlACOnFor0118(guchar *pControlData,gint controlDataLen) 
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;
	//controlBuff[3] = pIotData->payloadAndCheck[7];
    //F510C为0x204F控制协议
    LOG_Debug("空调控制指令:%d",pControlData[2]);
    switch (pControlData[2])
    {
    case 0x00:  //无请求
        LOG_Debug("空调无请求控制指令");
        return retValue;
        break;
    case 0x01:  //off/stop ????
        LOG_Debug("空调控制指令:Off/Stop");
        break;
    case 0x02:  //仅吹风
        LOG_Debug("空调控制指令:Blower On");
        break;
    case 0x03:  //自动模式
        break;
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
        LOG_Debug("空调预留控制命令");
        break;    
    default:
        break;
    }

    LOG_Debug("空调温度设定：%d",pControlData[3]);
    LOG_Debug("空调开启时间（预留）：%d",pControlData[4]);

    controlBuff[0] = pControlData[2];
    controlBuff[1] = 0x00;
    controlBuff[2] = pControlData[3];
	controlBuff[3] = pControlData[4];

    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_AC, controlBuff, 4);
  
    return retValue;
}

static gint ControlSeatHeat(guchar *pControlData,gint controlDataLen) 
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};
    gint subcmd = 0;
    if(NULL == pControlData || controlDataLen < 6 )
        return retValue;
	//controlBuff[3] = pIotData->payloadAndCheck[7];
    //F510C为0x204F控制协议
    LOG_Debug("SeatHeat cmd:%d",pControlData[3]);
    switch (pControlData[3])
    {
    case 0x00:  //无请求
        LOG_Debug("SeatHeat Norequest");
        return retValue;
        break;
    case 0x01:  
    case 0x02:
    case 0x03:
        subcmd = MSG_SUBCMD_CONTROL_OPEN_SEAT_HEAT;
        break;
    case 0x07:
        LOG_Debug("SeatHeat off");
        subcmd = MSG_SUBCMD_CONTROL_CLOSE_SEAT_HEAT;
        break;    
    default:
        LOG_Debug("SeatHeat cmd error");
        return retValue;
        break;
    }

    LOG_Debug("Seat position %d",pControlData[2]);
    LOG_Debug("SVOpenTime:%d",pControlData[4]);

    controlBuff[0] = pControlData[2];
    controlBuff[1] = pControlData[3];
    controlBuff[2] = pControlData[4];
	controlBuff[3] = 0x00;

    retValue = SendRemoteControlDataToMcu(subcmd, controlBuff, 4);
  
    return retValue;
}

static gint ControlSeatVentilate(guchar *pControlData,gint controlDataLen) 
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};
    gint subcmd = 0;
    if(NULL == pControlData || controlDataLen < 6 )
        return retValue;
	//controlBuff[3] = pIotData->payloadAndCheck[7];
    //F510C为0x204F控制协议
    LOG_Debug("SeatVentilate cmd:%d",pControlData[3]);
    switch (pControlData[3])
    {
    case 0x00:  //无请求
        LOG_Debug("SeatVentilate Norequest");
        return retValue;
        break;
    case 0x01:  
    case 0x02:
    case 0x03:
        subcmd = MSG_SUBCMD_CONTROL_OPEN_SEAT_VENTILATE;
        break;
    case 0x07:
        LOG_Debug("SeatVentilate off");
        subcmd = MSG_SUBCMD_CONTROL_CLOSE_SEAT_VENTILATE;
        break;    
    default:
        LOG_Debug("SeatVentilate cmd error");
        return retValue;
        break;
    }

    LOG_Debug("Seat position %d",pControlData[2]);
    LOG_Debug("SVOpenTime:%d",pControlData[4]);

    controlBuff[0] = pControlData[2];
    controlBuff[1] = pControlData[3];
    controlBuff[2] = pControlData[4];
	controlBuff[3] = 0x00;

    retValue = SendRemoteControlDataToMcu(subcmd, controlBuff, 4);
  
    return retValue;
}

static gint ControlSearchCar(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};
	guchar controlFlag = 0;

    if(NULL == pControlData || controlDataLen < 6)
        return retValue;

    /*控制协议转换*/
    if(pControlData[2] & 0x80)
    {
        LOG_Debug("鸣笛");
		controlFlag = 1;
    }

     if(pControlData[2] & 0x10)
    {
        LOG_Debug("双闪");
		controlFlag = 2;
    }
     if((pControlData[2] & 0x80)&(pControlData[2] & 0x10))
    {
        LOG_Debug("鸣笛双闪");
		controlFlag = 0;
    }
	

    controlBuff[0] = controlFlag;//pControlData[2];
    
    retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_FLASH_WHISTLE, controlBuff, 4);

    return retValue;
}
gint ControlReserverChargingOnlySend(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 4)
        return retValue;

    /*控制协议转换*/
    switch(pControlData[0])
    {
        case 0x00:
            controlBuff[0] = 0x00;
            break;
        case 0x01:
            controlBuff[0] = 0x01;
            break;
        default:
            controlBuff[0] = 0x00;
    }
    pthread_mutex_lock(&g_controlLock);
    g_timeOutArray[ReserverCharE] = CONTROL_CMD_TIME_OUT_S;
    pthread_mutex_unlock(&g_controlLock);
    SaveOderChargingTimeToFile(ODER_CHARGING_INFO, strlen(ODER_CHARGING_INFO));
    return SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_ODER_CHARGING, controlBuff, 4);    
}

gint ControlReserverCharging(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};

    if(NULL == pControlData || controlDataLen < 4)
        return retValue;

    /*控制协议转换*/
    switch(pControlData[0])
    {
        case 0x00:
            controlBuff[0] = 0x00;
            break;
        case 0x01:
            controlBuff[0] = 0x01;
            break;
        default:
            controlBuff[0] = 0x00;
    }
    pthread_mutex_lock(&g_controlLock);
    g_ReserverChargingTime.flag = controlBuff[0];
    if(g_ReserverChargingTime.flag == 0)
    {
        memset(&g_ReserverChargingTime, 0 , sizeof(ReserverCharging));
    }
    g_timeOutArray[ReserverCharE] = CONTROL_CMD_TIME_OUT_S;
    pthread_mutex_unlock(&g_controlLock);

    SaveOderChargingTimeToFile(ODER_CHARGING_INFO, strlen(ODER_CHARGING_INFO));

    return SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_ODER_CHARGING, controlBuff, 4);
}


gint ControlReserverChargingSendInfoToMCU(guchar *pControlData, gint controlDataLen)
{
    gint retValue = RET_ERR; 
    guchar controlBuff[7] = {0};
    if(NULL == pControlData || controlDataLen != 7)
        return retValue;
    memcpy(controlBuff,pControlData,controlDataLen);

    pthread_mutex_lock(&g_controlLock);
    g_timeOutArray[ReserverCharE] = CONTROL_CMD_TIME_OUT_S;
    pthread_mutex_unlock(&g_controlLock);

    SaveOderChargingTimeToFile(ODER_CHARGING_INFO, strlen(ODER_CHARGING_INFO));
    return SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_ODER_CHARGING, controlBuff, 7);    
}

static gint ParseCE31ControlDataAndHandle(IotDataFormat *pIotData)
{
    int retValue = RET_ERR;
    guchar controlBuff[128] = {0};
    gint controlBuffLen = 0;
    IotDataFormat *pTmpIotData = NULL;
    guchar subCmd = 0;

    if(NULL == pIotData)
        return retValue;
    
    LOG_Debug("formatIotData->serviceID :%04x",pIotData->subfunction);
    switch(pIotData->subfunction)
    {
        case 0x0600:            //预约
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[CarPreBook] = CONTROL_CMD_TIME_OUT_S;
            memcpy(&g_shardCarPreBook, pIotData, sizeof(IotDataFormat));
            pthread_mutex_unlock(&g_controlLock);
            retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_SHARED_CAR_PREBOOK, controlBuff, 4);
            break;
        case 0x0700:            //取消预约
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[CarPreBookCancel] = CONTROL_CMD_TIME_OUT_S;
            memcpy(&g_shardCarPreBookCancle, pIotData, sizeof(IotDataFormat));
            pthread_mutex_unlock(&g_controlLock);
            retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_SHARED_CAR_PREBOOK_CANCEL, controlBuff, 4);
            break;
        case 0x0C00:            //还车
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[CarReturn] = CONTROL_CMD_TIME_OUT_S;
            memcpy(&g_shardCarReturn, pIotData, sizeof(IotDataFormat));
            pthread_mutex_unlock(&g_controlLock);
            retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_SHARED_CAR_RETURN, controlBuff, 4);
            break;
        case 0x1100: //创建蓝牙指令  
            subCmd = MSG_SUBCMD_CONTROL_CREATE_BLE_KEY;
            controlBuffLen = htons(pIotData->payloadLength) - 2;
            memcpy(controlBuff, pIotData->payloadAndCheck, controlBuffLen);
            break;
        case 0x1200:  //更新蓝牙密钥指令
            subCmd = MSG_SUBCMD_CONTROL_UPDATE_BLE_SECRET_KEY;
            controlBuffLen = htons(pIotData->payloadLength) - 2;
            memcpy(controlBuff, pIotData->payloadAndCheck, controlBuffLen);
            break;
        case 0x1300:  //更新蓝牙钥匙权限指令
            subCmd = MSG_SUBCMD_CONTROL_UPDATE_BLE_KEY_PERMISSION;
            controlBuffLen = htons(pIotData->payloadLength) - 2;
            memcpy(controlBuff, pIotData->payloadAndCheck, controlBuffLen);
            break;
        case 0x1400:   //撤销指定得蓝牙钥匙指令
            subCmd = MSG_SUBCMD_CONTROL_DELETE_SPECIFIED_BLE_KEY;
            controlBuffLen = htons(pIotData->payloadLength) - 2;
            memcpy(controlBuff, pIotData->payloadAndCheck, controlBuffLen);
            break;
        case 0x1500:   //删除所有蓝牙钥匙指令
            subCmd = MSG_SUBCMD_CONTROL_DELECT_BLE_KEY;
            controlBuff[0] = 0x0F;
            controlBuffLen = 4;
            break;
        case 0x1600:   //查询蓝牙钥匙指令
            subCmd = MSG_SUBCMD_CONTROL_SELECT_BLE_KEY;
            controlBuff[0] = 0x0F;
            controlBuffLen = 4;
            break;
        default:
            break;
    }

    if(0 == subCmd)
    {
        return retValue;
    }

    if(NULL != g_remoteBLEKeyData)
    {
        SendBLEKeyControlResultData(MSG_SBUCMD_CONTROL_BLE_BUSY, pIotData, controlBuff, 4, 1);
        return retValue;
    }

    pTmpIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat));
    if(NULL == pTmpIotData)
    {
        SendBLEKeyControlResultData(MSG_SBUCMD_CONTROL_BLE_NOT_FIND, pIotData, controlBuff, 4, 1);
        return retValue;
    }

    pthread_mutex_lock(&g_controlLock);
    g_timeOutArray[RemoteBLEKeyCrl] = CONTROL_CMD_TIME_OUT_S;
    g_remoteBLEKeyData = pTmpIotData;
    memcpy(g_remoteBLEKeyData, pIotData, sizeof(IotDataFormat));
    pthread_mutex_unlock(&g_controlLock);
    retValue = SendRemoteControlBLEKeyDataToMcu(subCmd, controlBuff, controlBuffLen);

    return retValue;
}

static gint Parse0D00ControlDataAndHandle(IotDataFormat *pIotData)
{
    gint retValue = RET_ERR;
    guchar controlBuff[4] = {0};
    guint16 controlCmd = 0x0000;
    IotDataFormat *pTmpIotData = NULL;

    if(NULL == pIotData)
        return retValue;

    LOG_Debug("payloadLength :%04x", pIotData->payloadLength);
    /*payloadLength = payload + checksum(2)*/
    if(pIotData->payloadLength < 4)
        return retValue;

    pTmpIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + pIotData->payloadLength);
    if(NULL == pTmpIotData)
    {
        LOG_Error("Parse0100ControlDataAndHandle malloc error");
        return retValue;
    }

    controlCmd = (pIotData->payloadAndCheck[0] << 8) + pIotData->payloadAndCheck[1];

    LOG_Debug("controlCmd :%04x", controlCmd);
    switch(controlCmd)
    {
        case 0x0216:   //开启哨兵模式
            if(g_sintineOpenData != NULL)
            {
                g_free(pTmpIotData);
                LOG_Debug("sintine open ctr is busy !");
                break;
            }
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[SentineOpen] = CONTROL_CMD_TIME_OUT_S;
            g_sintineOpenData = pTmpIotData;
            memcpy(g_sintineOpenData, pIotData, sizeof(IotDataFormat) + pIotData->payloadLength);
            pthread_mutex_unlock(&g_controlLock);
            retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_OPEN_SENTINE_MODE, controlBuff, 4);
            break;
        break;
        case 0x0217:   //关闭哨兵模式
            if(g_sintineCloseData != NULL)
            {
                g_free(pTmpIotData);
                LOG_Debug("sintine close ctr is busy !");
                break;
            }
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[SentineClose] = CONTROL_CMD_TIME_OUT_S;
            g_sintineCloseData = pTmpIotData;
            memcpy(g_sintineCloseData, pIotData, sizeof(IotDataFormat) + pIotData->payloadLength);
            pthread_mutex_unlock(&g_controlLock);
            retValue = SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_CLOSE_SENTINE_MODE, controlBuff, 4);
            break;
        default:
        break;
    }

    return retValue;
}

static gint Parse1E00ControlDataAndHandle(IotDataFormat *pIotData)
{
    gint retValue = RET_ERR;
    guint16 controlCmd = 0x0000;
    guint16 payloadLen = 0;
    IotDataFormat *pTmpIotData = NULL;

    if(NULL == pIotData)
        return retValue;

    payloadLen = htons(pIotData->payloadLength);
    LOG_Debug("payloadLength :%04x", pIotData->payloadLength);
    /*payloadLength = payload + checksum(2)*/
    if(payloadLen < 4)
        return retValue;

    pTmpIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + payloadLen);
    if(NULL == pTmpIotData)
    {
        LOG_Error("Parse1E00ControlDataAndHandle malloc error");
        return retValue;
    }

    controlCmd = (pIotData->payloadAndCheck[0] << 8) + pIotData->payloadAndCheck[1];

    LOG_Debug("controlCmd :%04x", controlCmd);
    switch(controlCmd)
    {
        case 0x0120:        //开启座椅加热
            g_controlCmd = 0x0120;        
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[RemoteSeatHeat] = CONTROL_CMD_TIME_OUT_S;
            g_remoteSeatHeat = pTmpIotData;
            memcpy(g_remoteSeatHeat, pIotData, sizeof(IotDataFormat));
            pthread_mutex_unlock(&g_controlLock);
            SendRemoteControlSeatAck(pIotData);
            retValue = ControlSeatHeat(pIotData->payloadAndCheck, payloadLen);
            break;
        case 0x0121:        //开启座椅通风
            g_controlCmd = 0x0121;        
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[RemoteSeatVentilate] = CONTROL_CMD_TIME_OUT_S;
            g_remoteSeatVentilate = pTmpIotData;
            memcpy(g_remoteSeatVentilate, pIotData, sizeof(IotDataFormat));
            pthread_mutex_unlock(&g_controlLock);
            SendRemoteControlSeatAck(pIotData);
            retValue = ControlSeatVentilate(pIotData->payloadAndCheck, payloadLen);            
            break;
        default:
            break;
    }

    return retValue;
}


static gint Parse0900ControlDataAndHandle(IotDataFormat *pIotData)
{
    gint retValue = RET_ERR;
    guint16 controlCmd = 0x0000;
    guint16 payloadLen = 0;
    IotDataFormat *pTmpIotData = NULL;

    if(NULL == pIotData)
        return retValue;

    payloadLen = htons(pIotData->payloadLength);
    LOG_Debug("payloadLength :%04x", pIotData->payloadLength);
    /*payloadLength = payload + checksum(2)*/
    if(payloadLen < 4)
        return retValue;

    pTmpIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + payloadLen);
    if(NULL == pTmpIotData)
    {
        LOG_Error("Parse0900ControlDataAndHandle malloc error");
        return retValue;
    }

    controlCmd = (pIotData->payloadAndCheck[0] << 8) + pIotData->payloadAndCheck[1];

    LOG_Debug("controlCmd :%04x", controlCmd);
    switch(controlCmd)
    {
        case 0x0118:        //远控开启空调--新
            g_controlCmd = 0x0118;        
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[AcOn] = CONTROL_CMD_TIME_OUT_S;
            g_acOnData = pTmpIotData;
            memcpy(g_acOnData, pIotData, sizeof(IotDataFormat));
            pthread_mutex_unlock(&g_controlLock);
            SendRemoteControlAcOnAck(pIotData);
            retValue = ControlACOnFor0118(pIotData->payloadAndCheck, payloadLen);
            break;
        case 0x0105:        //远控开启空调--老
            g_controlCmd = 0x0105;  
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[AcOn] = CONTROL_CMD_TIME_OUT_S;
            g_acOnData = pTmpIotData;
            memcpy(g_acOnData, pIotData, sizeof(IotDataFormat));
            pthread_mutex_unlock(&g_controlLock);
            SendRemoteControlAcOnAck(pIotData);
            retValue = ControlACOn(pIotData);
            break;
        default:
            break;

    }

    return retValue;

}
static gint Parse1D00ControlDataAndHandle(IotDataFormat *pIotData)
{
    gint retValue = RET_ERR;
    guint16 payloadLen = 0;
    guchar controlData[7] = {0};
    IotDataFormat *pTmpIotData = NULL;

    if(NULL == pIotData)
        return retValue;

    payloadLen = htons(pIotData->payloadLength);
    LOG_Debug("payloadLength :%04x", pIotData->payloadLength);
    /*payloadLength = payload + checksum(2)*/
    if(payloadLen < 2)
        return retValue;

   
    pthread_mutex_lock(&g_controlLock);
    memset(&g_ReserverChargingTime,0,sizeof(ReserverCharging));
    g_ReserverChargingTime.flag = pIotData->payloadAndCheck[0];
    if(0x1B00 == pIotData->subfunction)
    {
        g_ReserverChargingTime.flag = 0;
    }

    g_ReserverChargingTime.version = 2;
    g_ReserverChargingTime.mcuSetState = 0;

    if(1 == g_ReserverChargingTime.flag && 0x08 <= payloadLen)
    {
        //v2新版预约信息单次+每日
        g_ReserverChargingTime.appointmentType = pIotData->payloadAndCheck[1];

        g_ReserverChargingTime.startTimeH = pIotData->payloadAndCheck[2];
        g_ReserverChargingTime.startTimeM = pIotData->payloadAndCheck[3];
        g_ReserverChargingTime.endTimeH = pIotData->payloadAndCheck[4];
        g_ReserverChargingTime.endTimeM = pIotData->payloadAndCheck[5];

        time_t now = time(NULL);
        tm* local_time = localtime(&now);
        int nowMinute =local_time->tm_hour * 60 + local_time->tm_min;
        int reserverChargingTimeStartM = g_ReserverChargingTime.startTimeH * 60 + g_ReserverChargingTime.startTimeM;
        int reserverChargingTimeEndM = g_ReserverChargingTime.endTimeH * 60 + g_ReserverChargingTime.endTimeM;
        LOG_Debug("now:%d start:%d end:%d",nowMinute,reserverChargingTimeStartM,reserverChargingTimeEndM);


        g_ReserverChargingTime.sendFlag = 1;
        g_ReserverChargingTime.sendFlagTmp = 1; 

        local_time->tm_hour = 0;
        local_time->tm_min = 0;
        local_time->tm_sec = 0;
        g_ReserverChargingTime.singleStartTime = mktime(local_time) + reserverChargingTimeStartM * 60;
        
        if(1 == g_ReserverChargingTime.appointmentType && reserverChargingTimeStartM < nowMinute)
        {
            // 如果预约开始时间小于现在时间，单次预约为次日，否则为当日；仅单次预约使用
            g_ReserverChargingTime.singleStartTime += 86400;
        }
        // 预约每日存在跨天情况
        if(2 == g_ReserverChargingTime.appointmentType && (
            (reserverChargingTimeEndM < reserverChargingTimeStartM && !(reserverChargingTimeEndM <= nowMinute && nowMinute < reserverChargingTimeStartM)) || 
            (reserverChargingTimeEndM > reserverChargingTimeStartM && reserverChargingTimeStartM <= nowMinute && nowMinute < reserverChargingTimeEndM)
            ))
        {

            g_ReserverChargingTime.sendFlag = 0;
            g_ReserverChargingTime.sendFlagTmp = 0;         
        }

    }

    controlData[0] = g_ReserverChargingTime.flag;
    controlData[1] = g_ReserverChargingTime.flag;
    controlData[2] = g_ReserverChargingTime.appointmentType;
    controlData[3] = g_ReserverChargingTime.startTimeH;
    controlData[4] = g_ReserverChargingTime.startTimeM;
    controlData[5] = g_ReserverChargingTime.endTimeH;
    controlData[6] = g_ReserverChargingTime.endTimeM;

    memcpy(&g_reserverChargingData, pIotData, sizeof(IotDataFormat));
    pthread_mutex_unlock(&g_controlLock);
    ControlReserverChargingSendInfoToMCU(controlData,7);

    return retValue;
}

static gint Parse1C00ControlDataAndHandle(IotDataFormat *pIotData)
{
    gint retValue = RET_ERR;
    guint16 controlCmd = 0x0000;
    guint16 payloadLen = 0;
    IotDataFormat *pTmpIotData = NULL;

    if(NULL == pIotData)
        return retValue;

    payloadLen = htons(pIotData->payloadLength);
    LOG_Debug("payloadLength :%04x", pIotData->payloadLength);
    /*payloadLength = payload + checksum(2)*/
    if(payloadLen < 4)
        return retValue;

    pTmpIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + payloadLen);
    if(NULL == pTmpIotData)
    {
        LOG_Error("Parse1C00ControlDataAndHandle malloc error");
        return retValue;
    }
    if(pIotData->payloadAndCheck[0] >= 50 && pIotData->payloadAndCheck[0] <= 100)
    {
        guchar controlDataBuff[4] = {0};
        pthread_mutex_lock(&g_controlLock);
        g_timeOutArray[ChargeLimitE] = CONTROL_CMD_TIME_OUT_S;
        controlDataBuff[0] = pIotData->payloadAndCheck[0];
        g_chargeLimit = pTmpIotData;
        memcpy(g_chargeLimit,pIotData,sizeof(IotDataFormat));
        pthread_mutex_unlock(&g_controlLock);
        retValue =SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_CHARGE_LIMIT,controlDataBuff,4);
    }
    else
    {
        LOG_Debug("charge limit error %d",pTmpIotData->payloadAndCheck[0]);
        retValue = RET_ERR;
    }

    return retValue;
}

static gint Parse0400ControlDataAndHandle(IotDataFormat *pIotData)
{
    gint retValue = RET_ERR;
    guint16 controlCmd = 0x0000;
    guint16 payloadLen = 0;
    IotDataFormat *pTmpIotData = NULL;

    if(NULL == pIotData)
        return retValue;

    payloadLen = htons(pIotData->payloadLength);
    LOG_Debug("payloadLength :%04x", pIotData->payloadLength);
    /*payloadLength = payload + checksum(2)*/
    if(payloadLen != 2)
        return retValue;
    pTmpIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + payloadLen);
    if(NULL == pTmpIotData)
    {
        LOG_Error("Parse1C00ControlDataAndHandle malloc error");
        return retValue;
    }
        guchar controlDataBuff[4] = {0};
        pthread_mutex_lock(&g_controlLock);
        g_timeOutArray[RemoteReset] = CONTROL_CMD_TIME_OUT_S;
        //controlDataBuff[0] = pIotData->payloadAndCheck[0];
        controlDataBuff[0] = 0x01;
        g_remotereset = pTmpIotData;
        memcpy(g_remotereset,pIotData,sizeof(IotDataFormat));
        pthread_mutex_unlock(&g_controlLock);
        retValue =SendRemoteControlDataToMcu(MSG_SUBCMD_CONTROL_REMOTE_RESET,controlDataBuff,4);
        return retValue;
}


static gint Parse0100ControlDataAndHandle(IotDataFormat *pIotData)
{
    gint retValue = RET_ERR;
    guint16 controlCmd = 0x0000;
    guint16 payloadLen = 0;
    IotDataFormat *pTmpIotData = NULL;

    if(NULL == pIotData)
        return retValue;

    payloadLen = htons(pIotData->payloadLength);
    LOG_Debug("payloadLength :%04x", pIotData->payloadLength);
    /*payloadLength = payload + checksum(2)*/
    if(payloadLen < 4)
        return retValue;

    pTmpIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + payloadLen);
    if(NULL == pTmpIotData)
    {
        LOG_Error("Parse0100ControlDataAndHandle malloc error");
        return retValue;
    }

    controlCmd = (pIotData->payloadAndCheck[0] << 8) + pIotData->payloadAndCheck[1];

    LOG_Debug("controlCmd :%04x", controlCmd);
    switch(controlCmd)
    {
        case 0x0203:            //车速限制-未要求
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[SpeedRemoteE] = CONTROL_CMD_TIME_OUT_S;
            g_speedRemoteControlData = pTmpIotData;
            memcpy(g_speedRemoteControlData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlCarSpeed(pIotData->payloadAndCheck, payloadLen);
            break;
        case 0x0112:            //驻车保温-F510C
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[ParkingHeatE] = CONTROL_CMD_TIME_OUT_S;
            g_parkingHeatingRemoteControlData = pTmpIotData;
            memcpy(g_parkingHeatingRemoteControlData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlParkingHeat(pIotData->payloadAndCheck, payloadLen);
            break;
        case 0x0113:            //远程加热-未要求
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[RemoteHeatE] = CONTROL_CMD_TIME_OUT_S;
            g_remoteHeatControlData = pTmpIotData;
            memcpy(g_remoteHeatControlData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlRemoteHeat(pIotData->payloadAndCheck, payloadLen);
            break;
        case 0x0101:            //车门解锁-F510C
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[DoorUnLockE] = CONTROL_CMD_TIME_OUT_S;
            g_doorUnLockData = pTmpIotData;
            memcpy(g_doorUnLockData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlDoorUnLock(pIotData->payloadAndCheck, payloadLen);
            break;
        case 0x0102:            //车门闭锁-F510C
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[DoorLockE] = CONTROL_CMD_TIME_OUT_S;
            g_doorLockData = pTmpIotData;
            memcpy(g_doorLockData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlDoorLock(pIotData->payloadAndCheck, payloadLen);
            break;
        case 0x0108:            //鸣笛-寻车-F51C
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[SearchCarE] = CONTROL_CMD_TIME_OUT_S;
            g_searchCarControlData = pTmpIotData;
            memcpy(g_searchCarControlData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlSearchCar(pIotData->payloadAndCheck, payloadLen);
            break;
        case 0x0103:            //关窗-F510C
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[WindowClose] = CONTROL_CMD_TIME_OUT_S;
            g_windowCloseData = pTmpIotData;
            memcpy(g_windowCloseData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlWindowClose(pIotData->payloadAndCheck, payloadLen);
            break;
		case 0x0104:            //开窗-F510C
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[WindowOpen] = CONTROL_CMD_TIME_OUT_S;
            g_windowOpenData = pTmpIotData;
            memcpy(g_windowOpenData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlWindowOpen(pIotData->payloadAndCheck, payloadLen);
            break;
	    case 0x0110:            //开灯-未要求
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[LightOpen] = CONTROL_CMD_TIME_OUT_S;
            g_lightOpenData = pTmpIotData;
            memcpy(g_lightOpenData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlLightOpen(pIotData->payloadAndCheck, payloadLen);
            break;
		case 0x0111:            //关灯-未要求
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[LightClose] = CONTROL_CMD_TIME_OUT_S;
            g_lightCloseData = pTmpIotData;
            memcpy(g_lightCloseData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlLightClose(pIotData->payloadAndCheck, payloadLen);
            break;
	    case 0x0218:            //远程上电-未要求
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[PowerUp] = CONTROL_CMD_TIME_OUT_S;
            g_powerUpData = pTmpIotData;
            memcpy(g_powerUpData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlRemotePowerUp(pIotData->payloadAndCheck, payloadLen);
            break;
		case 0x0219:            //远程下电-F510C
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[PowerDown] = CONTROL_CMD_TIME_OUT_S;
            g_powerDownData= pTmpIotData;
            memcpy(g_powerDownData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlRemotePowerDown(pIotData->payloadAndCheck, payloadLen);
            break;
		case 0x021B:            //OTA上电-F510C
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[PowerDown] = CONTROL_CMD_TIME_OUT_S;
            g_otaPowerUpData = pTmpIotData;
            memcpy(g_otaPowerUpData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlOTAPowerUp(pIotData->payloadAndCheck, payloadLen);
            break;     
		case 0x0105:            //远程关AC-其他项目
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[AcOff] = CONTROL_CMD_TIME_OUT_S;
            g_acOffData= pTmpIotData;
            memcpy(g_acOffData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlACOff(pIotData->payloadAndCheck, payloadLen);
            break;
		case 0x0115:            //远程授权点火-F510C
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[RemoteBLEEnable] = CONTROL_CMD_TIME_OUT_S;
            g_remoteBleEnableData= pTmpIotData;
            memcpy(g_remoteBleEnableData, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlRemoteBLEEnable(pIotData->payloadAndCheck, payloadLen);
            break;
        case 0x0116:            //远程车窗带开度-未要求
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[ControlWindowsE] = CONTROL_CMD_TIME_OUT_S;
            g_controlWindows= pTmpIotData;
            memcpy(g_controlWindows, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlWindows(pIotData->payloadAndCheck, payloadLen);
            break;
        case 0x0319:    //泊车-未要求
            pthread_mutex_lock(&g_controlLock);
            if(NULL != g_remoteParkingStatus)
            {
                g_free(pTmpIotData);
                pthread_mutex_unlock(&g_controlLock);
                break;
            }
            g_timeOutArray[RemoteParkStaE] = CONTROL_CMD_TIME_OUT_S;
            g_remoteParkingStatus= pTmpIotData;
            memcpy(g_remoteParkingStatus, pIotData, sizeof(IotDataFormat) + payloadLen);
            retValue = ControlRemoteParkingSta(pIotData->payloadAndCheck, payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            break;
        case 0x011A:    //F510C-开尾门
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[OpenTrunkE] = CONTROL_CMD_TIME_OUT_S;
            g_openTrunk = pTmpIotData;
            memcpy(g_openTrunk, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlTrunkOpen();
            break;
        case 0x0117:    //F510c-关尾门
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[CloseTrunkE] = CONTROL_CMD_TIME_OUT_S;
            g_closeTrunk = pTmpIotData;
            memcpy(g_closeTrunk, pIotData, sizeof(IotDataFormat) + payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            retValue = ControlTrunkClose();
            break;
        case 0x0119:    //F510C-关空调
            pthread_mutex_lock(&g_controlLock);
            g_controlCmd = 0x0119;
            g_timeOutArray[AcOff] = CONTROL_CMD_TIME_OUT_S;
            g_acOffData= pTmpIotData;
            memcpy(g_acOffData, pIotData, sizeof(IotDataFormat));
            ControlACOff(pIotData->payloadAndCheck, htons(pIotData->payloadLength));
            pthread_mutex_unlock(&g_controlLock);
            break;
        default:
            break;
    }

    return retValue;
}

static gint Parse17001800ControlDataAndHandle(IotDataFormat *pIotData)
{
    gint retValue = RET_ERR;
    guint16 controlCmd = 0x0000;
    guint16 payloadLen = 0;
    IotDataFormat *pTmpIotData = NULL;

    if(NULL == pIotData)
        return retValue;

    payloadLen = htons(pIotData->payloadLength);
    LOG_Debug("payloadLength :%04x", pIotData->payloadLength);
    /*payloadLength = payload + checksum(2)*/
    if(pIotData->payloadLength < 4)
        return retValue;

    pTmpIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + payloadLen);
    if(NULL == pTmpIotData)
    {
        LOG_Error("Parse0100ControlDataAndHandle malloc error");
        return retValue;
    }

    controlCmd = (pIotData->payloadAndCheck[0] << 8) + pIotData->payloadAndCheck[1];

    LOG_Debug("controlCmd :%04x", controlCmd);
    switch(pIotData->subfunction)
    {
         case 0x1700: 
            pthread_mutex_lock(&g_controlLock);
            if(NULL != g_remoteParking && 1 == g_remoteParkingFlag)
            {
                g_free(pTmpIotData);
                pthread_mutex_unlock(&g_controlLock);
                break;
            }

            if(NULL != g_remoteParking)
            {
                g_free(g_remoteParking);
            }

            SendRemoteParkingAck(pIotData);
            g_timeOutArray[RemoteParkingE] = CONTROL_CMD_TIME_OUT_S;
            g_remoteParking= pTmpIotData;
            LOG_Debug("备份数据成功开始");
            memcpy(g_remoteParking, pIotData, sizeof(IotDataFormat) + payloadLen);
            LOG_Debug("备份数据成功");
            retValue = ControlRemoteParking(pIotData->payloadAndCheck, payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            break;
        case 0x1800: 
            pthread_mutex_lock(&g_controlLock);
            if(NULL != g_unlockTheCar)
            {
                g_free(pTmpIotData);
                pthread_mutex_unlock(&g_controlLock);
                break;
            }
            SendUnlockTheCarAck(pIotData);
            g_timeOutArray[UnlockTheCarE] = CONTROL_CMD_TIME_OUT_S;
            g_unlockTheCar= pTmpIotData;
            memcpy(g_unlockTheCar, pIotData, sizeof(IotDataFormat) + payloadLen);
            retValue = ControlRemoteParking(pIotData->payloadAndCheck, payloadLen);
            pthread_mutex_unlock(&g_controlLock);
            break;
        default:
            break;
    }

    return retValue;
}


gint SendRemoteCmdResult(IotDataFormat *remoteControlData, gchar *result, guint16 resultLen, guint16 serviceID)
{
    IotDataFormat *logouttData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    gint errReturn;

    if(NULL == remoteControlData)
    {
        return RET_ERR;
    }

    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + resultLen + 3);
    
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = remoteControlData->headerVersion;
    logouttData->flag = remoteControlData->flag;
    logouttData->pkgLength = remoteControlData->pkgLength;
    logouttData->eventID = remoteControlData->eventID;
    logouttData->randomData = remoteControlData->randomData;
    payloadLength = resultLen + 2;
    logouttData->payloadLength = htons(payloadLength);
    if(serviceID != 0xFFFF)
    {
        logouttData->serviceID = serviceID;
        logouttData->subfunction = remoteControlData->subfunction;
    }
    else 
    {
        logouttData->serviceID = 0xFFFF;
        logouttData->subfunction = remoteControlData->serviceID;
    }

    memcpy(&logouttData->payloadAndCheck[0], result, resultLen);

    logintDataLen = htons(logouttData->pkgLength);
    craData = CalCrcCal((guint8 *)logouttData,logintDataLen + payloadLength - 2);
    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    //LOG_Debug("send data here : %d\n",logouttData->payloadAndCheck[1]);
    //return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),3);
    errReturn = SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),3);
    g_free(logouttData);
    return errReturn;
}

static gint SetCollectionAddr(IotDataFormat *iotData)
{
    uint8_t   ipAddrBuff[128] = {0};
    uint8_t   ipAddrBuffBack[128] = {0};
    gchar   resultData[8] = {0};
    uint8_t   portBuff[16] {0};
    guint32 ipPort = 0; 
    guint8  ipDataLen = 0;
    guint8  portDataLen = 0;
    gint    retValue = RET_ERR;
    guint16 paramServiceID = 0xFFFF;
    
    guint16 dataLen = 0;

    if(NULL == iotData)
    {
        return RET_ERR;
    }

    dataLen = htons(iotData->payloadLength) - 2;

    if(dataLen <= 2)
    {
        LOG_Error("SetCollectionAddr dataLen error");
        SendRemoteCmdResult(iotData, resultData, 4, paramServiceID);
        return retValue;
    }
    
    ipDataLen = iotData->payloadAndCheck[0];
    if(ipDataLen <= 0 || ipDataLen >= 128)
    {
        LOG_Error("SetCollectionAddr ipDataLen error");
        SendRemoteCmdResult(iotData, resultData, 4, paramServiceID);
        return retValue;
    }

    if (!GetGBAddr(ipAddrBuffBack, sizeof(ipAddrBuffBack) - 1))
    {
        LOG_Error("SetCollectionAddr GetParamsValue enumGBParamAddr error");
        SendRemoteCmdResult(iotData, resultData, 4, paramServiceID);
        return RET_ERR;
    }

    memcpy(ipAddrBuff, &iotData->payloadAndCheck[1], ipDataLen);
    retValue = UpdateParam(E_ParamId_GB32960Addr, ipAddrBuff, ipDataLen);
    if(retValue < RET_OK)
    {
        LOG_Error("SetCollectionAddr SetParamsValue enumGBParamAddr error");
        SendRemoteCmdResult(iotData, resultData, 4, paramServiceID);
        return retValue;
    }

    portDataLen = iotData->payloadAndCheck[ipDataLen + 1];
    if(portDataLen <= 0 || portDataLen > 16)
    {
        LOG_Error("SetCollectionAddr portDataLen error: %d", portDataLen);
        SendRemoteCmdResult(iotData, resultData, 4, paramServiceID);
        return retValue;
    }

    memcpy(portBuff, &iotData->payloadAndCheck[ipDataLen + 2], portDataLen);
    //ipPort = atoi(portBuff);
    //retValue = SetParamsValue(enumGBParamPort, &ipPort);
    retValue = UpdateParam(E_ParamId_GB329060Port, portBuff, portDataLen);
    if(retValue < RET_OK)
    {
        LOG_Error("SetCollectionAddr SetParamsValue enumGBParamPort error");
        //SetParamsValue(enumGBParamAddr, ipAddrBuffBack);
        UpdateParam(E_ParamId_GB32960Addr, ipAddrBuffBack, strlen((gchar *)ipAddrBuffBack));
        SendRemoteCmdResult(iotData, resultData, 4, paramServiceID);
        return retValue;
    }

    paramServiceID = 0x6996;
    SendRemoteCmdResult(iotData, resultData, 0, paramServiceID);
    return RET_OK;
}

static gint SetCarControlMqttAddr(IotDataFormat *iotData)
{
    uint8_t   ipAddrBuff[128] = {0};
    gchar   resultData[8] = {0};
    guint8  ipDataLen = 0;
    gint    retValue = RET_ERR;
    guint16 paramServiceID = 0xFFFF;
    
    guint16 dataLen = 0;

    if(NULL == iotData)
    {
        return RET_ERR;
    }

    dataLen = htons(iotData->payloadLength) - 2;

    if(dataLen <= 2)
    {
        LOG_Error("SetCarControlMqttAddr dataLen error");
        SendRemoteCmdResult(iotData, resultData, 4, paramServiceID);
        return retValue;
    }
    
    ipDataLen = iotData->payloadAndCheck[0] * 256 + iotData->payloadAndCheck[1];
    if(ipDataLen <= 0 || ipDataLen >= 128)
    {
        LOG_Error("SetCarControlMqttAddr ipDataLen error");
        SendRemoteCmdResult(iotData, resultData, 4, paramServiceID);
        return retValue;
    }

    memcpy(ipAddrBuff, &iotData->payloadAndCheck[2], ipDataLen);
    //retValue = SetParamsValue(enumMqttUrl, ipAddrBuff);
    retValue = UpdateParam(E_ParamId_TspTokenUrl, ipAddrBuff, ipDataLen);
    if(retValue < RET_OK)
    {
        LOG_Error("SetCarControlTspTokenAddr SetParamsValue enumMqttUrl error");
        SendRemoteCmdResult(iotData, resultData, 4, paramServiceID);
        return retValue;
    }

    retValue = truncate("/oemdata/data/token.txt", 0);
    if(retValue < 0)
    {
        LOG_Debug("truncate /oemdata/data/token.txt  error");
    }

    paramServiceID = 0x6996;
    SendRemoteCmdResult(iotData, resultData, 0, paramServiceID);
    return RET_OK;
}

static gint SetNationalReportRate(IotDataFormat *iotData)
{
    uint8_t   ipAddrBuff[128] = {0};
    gchar   resultData[8] = {0};
    uint8_t  ipData = 0;
    gint    retValue = RET_ERR;
    guint16 paramServiceID = 0xFFFF;
    
    guint16 dataLen = 0;

    if(NULL == iotData)
    {
        return RET_ERR;
    }

    dataLen = htons(iotData->payloadLength) - 2;

    if(dataLen != 1)
    {
        LOG_Error("SetNationalReportRate dataLen error");
        SendRemoteCmdResult(iotData, resultData, 4, paramServiceID);
        return retValue;
    }
    
    ipData = iotData->payloadAndCheck[0];
    if(ipData <= 0 || ipData >= 31)
    {
        LOG_Error("SetNationalReportRate ipData error");
        SendRemoteCmdResult(iotData, resultData, 4, paramServiceID);
        return retValue;
    }
    paramServiceID = 0xD926;
    SendRemoteCmdResult(iotData, resultData, 0, paramServiceID);
    return RET_OK;
}
static gint SendRemoteControlSeatAck(IotDataFormat *remoteControlData)
{
    IotDataFormat *logouttData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = remoteControlData->headerVersion;
    logouttData->flag = remoteControlData->flag;
    logouttData->pkgLength = remoteControlData->pkgLength;
    logouttData->eventID = remoteControlData->eventID;
    logouttData->randomData = remoteControlData->randomData;
    logouttData->serviceID = 0x6D92;
    logouttData->subfunction = 0x1E00;
    logouttData->payloadLength = 0x0800;
    payloadLength = 8;
    memcpy(logouttData->payloadAndCheck, remoteControlData->payloadAndCheck, 6);

    logintDataLen = htons(logouttData->pkgLength) + 6;
    craData = CalCrcCal((guint8 *)logouttData, logintDataLen);

    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),3);    
}

static gint SendRemoteControlAcOnAck(IotDataFormat *remoteControlData)
{
    IotDataFormat *logouttData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = remoteControlData->headerVersion;
    logouttData->flag = remoteControlData->flag;
    logouttData->pkgLength = remoteControlData->pkgLength;
    logouttData->eventID = remoteControlData->eventID;
    logouttData->randomData = remoteControlData->randomData;
    logouttData->serviceID = 0x6D92;
    logouttData->subfunction = 0x0900;
    logouttData->payloadLength = 0x0800;
    payloadLength = 8;
    memcpy(logouttData->payloadAndCheck, remoteControlData->payloadAndCheck, 6);

    logintDataLen = htons(logouttData->pkgLength) + 6;
    craData = CalCrcCal((guint8 *)logouttData, logintDataLen);

    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),3);
}

static gint SendRemoteParkingAck(IotDataFormat *remoteControlData)
{
    IotDataFormat *logouttData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = remoteControlData->headerVersion;
    logouttData->flag = remoteControlData->flag;
    logouttData->pkgLength = remoteControlData->pkgLength;
    logouttData->eventID = remoteControlData->eventID;
    logouttData->randomData = remoteControlData->randomData;
    logouttData->serviceID = 0x6D92;
    logouttData->subfunction = 0x1700;
    logouttData->payloadLength = 0x0800;
    payloadLength = 8;
    memcpy(logouttData->payloadAndCheck, remoteControlData->payloadAndCheck, 6);

    logintDataLen = htons(logouttData->pkgLength) + 6;
    craData = CalCrcCal((guint8 *)logouttData, logintDataLen);

    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),3);
}

static gint SendUnlockTheCarAck(IotDataFormat *remoteControlData)
{
    IotDataFormat *logouttData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = remoteControlData->headerVersion;
    logouttData->flag = remoteControlData->flag;
    logouttData->pkgLength = remoteControlData->pkgLength;
    logouttData->eventID = remoteControlData->eventID;
    logouttData->randomData = remoteControlData->randomData;
    logouttData->serviceID = 0x6D92;
    logouttData->subfunction = 0x1800;
    logouttData->payloadLength = 0x0800;
    payloadLength = 8;
    memcpy(logouttData->payloadAndCheck, remoteControlData->payloadAndCheck, 6);

    logintDataLen = htons(logouttData->pkgLength) + 6;
    craData = CalCrcCal((guint8 *)logouttData, logintDataLen);

    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),3);
}

static gint ParserIotData(guchar *iotData, gint iotDataLen)
{
    LOG_Debug("ParserIotData");
    if(NULL == iotData || iotDataLen <= (sizeof(IotDataFormat) - sizeof(guchar *)))
    {
        LOG_Debug("iotData error");
        return RET_ERR;
    }
    IotDataFormat *formatIotData = (IotDataFormat *)iotData;
    gint ret = -1;
    gchar resultData[8] = {0};
    gchar systemCmdBuff[128] = {0};
    gint dtcNum = 0;
    guint16 didCode = 0;
    gint pmStatus;
    gint upgradeStatus;
    //UDS统一的诊断服务
//liulei    CTboxUdsService udsClass;
//liulei    udsClass.Open(0);
    LOG_Debug("formatIotData->serviceID :%04x",formatIotData->serviceID);
    LOG_Debug("formatIotData->subfunction :%04x",formatIotData->subfunction);

    formatIotData->headerVersion = 0x04;
#if 0
//liulei
    if(formatIotData->serviceID == 0xDB24)//远程诊断
    {
        switch(formatIotData->subfunction)
        {
            case 0x0100://读取TDC
            {
                if(0x00 == formatIotData->payloadAndCheck[1]) //tbox 
                {
                    if(0x00 == formatIotData->payloadAndCheck[0]) //按状态掩码报告 DTC
                    {
                        ret = udsClass.ExcuteReadDtcListByStatusMask(0xFF,&g_dtcCodeStatus,&g_stDtcNumArray[0],&g_dtcSum);
                    }
                    else
                    {
                        LOG_Debug("no service \n");
                        return RET_ERR;
                    }
                }
                LOG_Debug("formatIotData->payloadAndCheck[1] :%02x\n",formatIotData->payloadAndCheck[1]);
                LOG_Debug("formatIotData->payloadAndCheck[0] :%02x\n",formatIotData->payloadAndCheck[0]);
            }
            break;
            case 0x0400://读取TDC
            {
                if(0x00 == formatIotData->payloadAndCheck[1]) //tbox 
                {
                    if(0x01 == formatIotData->payloadAndCheck[0]) //按 DTC 编号报告 DTC 快照记录
                    {
                        g_dtcCode = (formatIotData->payloadAndCheck[6] << 16) + (formatIotData->payloadAndCheck[7] << 8) + formatIotData->payloadAndCheck[8];
                        ret = udsClass.ExcuteReadDtcSnapshotRecord(g_dtcCode,&g_udsDtcSnapShot,&g_dtcCodeStatus);
                    }
                    else
                    {
                        LOG_Debug("No service\n");
                        return RET_ERR;
                    }
                }
                LOG_Debug("formatIotData->payloadAndCheck[1] :%02x\n",formatIotData->payloadAndCheck[1]);
                LOG_Debug("formatIotData->payloadAndCheck[0] :%02x\n",formatIotData->payloadAndCheck[0]);
            }
            break;
            case 0x0500://读取TDC
            {
                if(0x00 == formatIotData->payloadAndCheck[1]) //tbox 
                {
                    if(0x02 == formatIotData->payloadAndCheck[0])//按 DTC编号报告 DTC扩展数据记录
                    {
                        g_dtcCode = (formatIotData->payloadAndCheck[6] << 16) + (formatIotData->payloadAndCheck[7] << 8) + formatIotData->payloadAndCheck[8];
                        ret = udsClass.ExcuteReadDtcExtendedDataRecord(g_dtcCode,&g_udsExtended,&g_dtcCodeStatus);
                    }
                    else
                    {
                        LOG_Debug("no service \n");
                        return RET_ERR;
                    }
                }
                LOG_Debug("formatIotData->payloadAndCheck[1] :%02x\n",formatIotData->payloadAndCheck[1]);
                LOG_Debug("formatIotData->payloadAndCheck[0] :%02x\n",formatIotData->payloadAndCheck[0]);
            }
            break;
            case 0x0200://读取参数
            {
                if(0x00 == formatIotData->payloadAndCheck[2]) //tbox 
                {
                    didCode = formatIotData->payloadAndCheck[0] * 256 + formatIotData->payloadAndCheck[1];
                    ret = udsClass.ExcuteReadDataByIdentifier(didCode,&g_versionNum[0],&g_versionLen);
                }
                LOG_Debug("formatIotData->payloadAndCheck[2] :%02x\n",formatIotData->payloadAndCheck[2]);
            }
            break;
            case 0x0300: //清楚故障
            {
                if(0x00 == formatIotData->payloadAndCheck[1]) //tbox 
                {
                    if(0x00 == formatIotData->payloadAndCheck[0]) //清除所有诊断故障信息
                    {
                        ret = udsClass.ExcuteClearDtc(0xFFFFFF);
                    }
                    else if(0x01 == formatIotData->payloadAndCheck[0]) //清除对应 DTC
                    {
                        for(dtcNum = 0;dtcNum < (formatIotData->payloadAndCheck[6] / 3); dtcNum++)
                        {

                        }
                    }
                }
                LOG_Debug("formatIotData->payloadAndCheck[1] :%02x\n",formatIotData->payloadAndCheck[1]);
                LOG_Debug("formatIotData->payloadAndCheck[0] :%02x\n",formatIotData->payloadAndCheck[0]);
            }
            break;
        }
        LOG_Debug("ret : %d\n",ret);
        PackResultIotData(formatIotData,ret);
        udsClass.Close();
    }
#endif
    
    if( formatIotData->serviceID == 0xDD22) //远控相关
    {
        usleep(200000);
        guchar controlDataBuff[4] = {0};
        switch(formatIotData->subfunction)
        {
            case 0x0800:
            {
                controlDataBuff[0] = 0x01;
                memcpy(&remoteControlData,formatIotData,sizeof(IotDataFormat));
                SendRemoteControlAck(remoteControlData);
                SendRemoteControlDataToMcu(0x0E,controlDataBuff,4);
                g_controlFlag |= 0x01;
            }
            break;
            case 0x0900:
            {
                //空调开启 - F510C
                Parse0900ControlDataAndHandle(formatIotData);
            }
            break;
            case 0x0100:
            {
                Parse0100ControlDataAndHandle(formatIotData);
            }
            break;
            case 0x1700:
            case 0x1800:
            {
                Parse17001800ControlDataAndHandle(formatIotData);
            }
            break;
            case 0x0A00:
            {
                // 预约充电-F510C
                guchar timeBuff[8] = {0};
                guchar controlData[4] = {0};
                gint count = 0;
                gint offset = 1;
                pthread_mutex_lock(&g_controlLock);
                LOG_Debug("formatIotData->payloadLength :%x", formatIotData->payloadLength);
                if(formatIotData->payloadLength == 0x0F00)
                {
                    g_ReserverChargingTime.flag = formatIotData->payloadAndCheck[0];
                    g_ReserverChargingTime.version = 1;
                    g_ReserverChargingTime.startTime = 0;
                    for(count = 0; count < 6; count++)
                    {
                        g_ReserverChargingTime.startTime = g_ReserverChargingTime.startTime << 8;
                        g_ReserverChargingTime.startTime += formatIotData->payloadAndCheck[offset++];
                    }
                    LOG_Debug("g_ReserverChargingTime.startTime :%lld", g_ReserverChargingTime.startTime);

                    g_ReserverChargingTime.endTime = 0;
                    for(count = 0; count < 6; count++)
                    {
                        g_ReserverChargingTime.endTime = g_ReserverChargingTime.endTime << 8;
                        g_ReserverChargingTime.endTime += formatIotData->payloadAndCheck[offset++];
                    }
                    LOG_Debug("g_ReserverChargingTime.endTime :%lld", g_ReserverChargingTime.endTime);

                    memcpy(&g_reserverChargingData, formatIotData, sizeof(IotDataFormat));
                    
                    // SendReserverChargingResultData(g_reserverChargingData);
                    controlData[0] = g_ReserverChargingTime.flag;
                    
                }
                pthread_mutex_unlock(&g_controlLock);
                ControlReserverCharging(controlData, 4);
                // SaveOderChargingTimeToFile(ODER_CHARGING_INFO, strlen(ODER_CHARGING_INFO));
            }
            break;
            case 0x0400:
            {
                //远程重启
                pmStatus = GetPmState();
                upgradeStatus = GetUpgradeStatus();
                LOG_Debug("xhf get reset command pmstatus %d,upgradeStatus %d\n",pmStatus,upgradeStatus);
                //判断是在升级，发送否定回答，再休眠也发送否定
                //if(GetPmState() != 2 || GetUpgradeStatus() == 0 )
                if( upgradeStatus == 0 )
                {
                
                    memcpy(&remoteControlData,formatIotData,sizeof(IotDataFormat));
                    SendRemoteControlResetAck(remoteControlData,0);//否定回答
                    LOG_Debug("xhf recv reset then error");
                }
                else
                {
                    //若MCU返回否定应答，APP清除事件；
                    //若MCU返回肯定应答，直接重启，不需要清除APP事件，若重启失败，在同步阻塞事件心跳中，会超时自动清除；
                    PmSetSleepEvent(PM_SLEEP_EVENT_BUSYING);
                    Parse0400ControlDataAndHandle(formatIotData); 
                }
            }	
            break;
            case 0x0700:
            {
                // 修改VIN
                uint8_t tboxVIN[18] = {0};
                if(formatIotData->payloadLength == 0x1300)
                {
                    memcpy(tboxVIN, &formatIotData->payloadAndCheck[0], 17);
                    ret = UpdateParam(E_ParamId_VIN, tboxVIN, strlen((char*)tboxVIN));
                    if(RET_OK == ret)
                    {
                        SendRemoteCmdResult(formatIotData, resultData, 0, 0x6D92); //肯定回答
                        break;
                    }
                }
                SendRemoteCmdResult(formatIotData, resultData, 4, 0xFFFF);//否定回答
            }
            break;
            case 0x0F00:    //删除备份包
            {
                snprintf(systemCmdBuff, 128, "rm %s", SYSTEM_CMD_TMP_FILE);
                ret = GetSystemRetValue(systemCmdBuff);
                if(ret == 0)
                {
                    SendRemoteCmdResult(formatIotData, resultData, 0, 0x6D92); //肯定回答
                }
                else
                {
                    SendRemoteCmdResult(formatIotData, resultData, 4, 0xFFFF);//否定回答
                }
                LOG_Debug("systemCmd [%d]:%s", ret, systemCmdBuff);
            }
            break;
            case 0x0D00:    //哨兵模式
            {
                Parse0D00ControlDataAndHandle(formatIotData);
            }
            break;
            case 0x1C00:    //充电电量限值
            {
                if(RET_ERR == Parse1C00ControlDataAndHandle(formatIotData))
                {
                    SendRemoteCmdResult(formatIotData, resultData, 4, 0xFFFF);//否定回答
                }
            }
            break;
            case 0x1B00:    //取消预约充电-新（单次+每天）
            case 0x1D00:    //预约充电-新（单次+每天）
            {
                Parse1D00ControlDataAndHandle(formatIotData);
            }
            break;
            case 0x1E00:    //座椅加热+座椅通风
            {
                Parse1E00ControlDataAndHandle(formatIotData);
            }
            break;
            default:
                break;
        }
    }
    else if( formatIotData->serviceID == 0xCE31) //蓝牙近场通信
    {
        // 蓝牙近控相关
        LOG_Debug("formatIotData->serviceID :%04x",formatIotData->serviceID);

        ParseCE31ControlDataAndHandle(formatIotData);
    }
    else if( formatIotData->serviceID == 0xBE41)
    {
        // FOTA唤醒    
        if(formatIotData->subfunction == 0x0100 || formatIotData->subfunction == 0x0800 || formatIotData->subfunction == 0x0900)
        {
            struct hostent *host = NULL;
            host = gethostbyname("carota2-filecdn.sgmwcloud.com.cn");
            if(NULL != host)
            {
                // 计算地址个数
                int address_count = 0;
                while (host->h_addr_list[address_count] != NULL) {
                    address_count++;
                }

                // 将每个结果地址检查后，设置到路由中
                for (int index = 0; index < address_count; index++) {
                    struct in_addr *address = (struct in_addr *)host->h_addr_list[index];
                    if (1 != checkIPonInterface("ccinet1", *address))
                    {
                        int retValue = -1;
                        char cmdBuff[1024] = {0};
                        memset(cmdBuff, 0, 1024);
                        snprintf(cmdBuff, 1024,"route add %s %s", inet_ntoa(*address), "ccinet1");
                        retValue = GetSystemRetValue(cmdBuff);
                        LOG_Debug("index [%d]cmdBuff exe ret [%d]:%s", index, retValue, cmdBuff);
                        if (0 != retValue)
                        {
                            return RET_ERR;
                        }
                    }
                    else
                    {
                        LOG_Debug("IP %s has bound on ccinet1", inet_ntoa(*address));
                    }
                }
            }
            else
            {
                LOG_Error("gethostbyname failed!");
                return RET_ERR;
            }

            sprintf(systemCmdBuff, "%s","sh /oemapp/upgrade/upgrade_env_check.sh &");
            system(systemCmdBuff);
            ControlOTAWakeUp();
            //if( 0 != Fota_ServerSync( (const char *)formatIotData->payloadAndCheck) )
            if( 0 != PostFotaMsg(0, formatIotData->payloadAndCheck, htons(formatIotData->payloadLength) - 2))
            {
                LOG_Error("Fota_ServerSync error");
            }
            else
            {
                LOG_Error("Fota_ServerSync success");
                SendRemoteCmdResult(formatIotData, resultData, 0, 0xBE41);//正确回复
            }
        }
        else
        {
            LOG_Debug("there is no fotati subfunction %d",formatIotData->subfunction);
        }
    }
    else if( formatIotData->serviceID == 0xBE51)
    {
        // 远程诊断唤醒
        if(formatIotData->subfunction == 0x0100)
        {
            //if( 0 != EmitTspEvent((gchar *)formatIotData->payloadAndCheck, htons(formatIotData->payloadLength) - 2))
            if (RET_OK != SendMsgToAllQueue((SYSQueMsgType_e)SYS_QUE_TYPE_SGMW_ABUP_RECV_RESP, (void *)formatIotData->payloadAndCheck, htons(formatIotData->payloadLength) - 2))
            {
                resultData[0] = 0x2E;
                resultData[1] = 0x00;
                resultData[2] = 0x00;
                resultData[3] = 0x00;
                SendRemoteCmdResult(formatIotData, resultData, 4, 0xFFFF);//否定回答
            }
            else
            {
                SendRemoteCmdResult(formatIotData, resultData, 0, 0xBE51);//正确回复
            }
        }
    }
    else if( formatIotData->serviceID == 0xDC23) //远程参数查询及配置
    {
        guint16 logServiceID = 0x6C93;
        /*上传log*/
        if(formatIotData->subfunction == 0x0900)//日志上传
        {
            /*1、打包log; 2、上传log；3、删除log*/
            //if(TspReportLog() < 0)
            PmSetSleepEvent(PM_SLEEP_EVENT_LOG);
            if(TspReportLog(formatIotData->payloadAndCheck[0]) < 0)
            {
                PmCleanSleepEvent(PM_SLEEP_EVENT_LOG);
                SendRemoteCmdResult(formatIotData, resultData, 4, 0xFFFF);//否定回答
            }
            else
            {
                SendRemoteCmdResult(formatIotData, resultData, 4, 0x6C93);//正确回复
            }
            
        }
        else if(formatIotData->subfunction == 0x0F00) //日志删除
        {
            // 删除存储MD5的文件
            snprintf(systemCmdBuff, 128, "rm %s", SYSTEM_CMD_TMP_FILE);
            ret = GetSystemRetValue(systemCmdBuff);
            if(ret == 0)
            {
                SendRemoteCmdResult(formatIotData, resultData, 4, 0xFFFF);//否定回答
            }
            else
            {
                SendRemoteCmdResult(formatIotData, resultData, 4, 0x6C93);//正确回复
            }
            LOG_Debug("systemCmd [%d]:%s", ret, systemCmdBuff);
        }
        else if(formatIotData->subfunction == 0x0200) //设备信息查询
        {
            guchar infoBuff[128] = {0};
            int infoOffset = 0;
            
            //获取设备信息
            GetIMEI(&infoBuff[infoOffset], 16);
            infoOffset += 15;
            GetIccid(&infoBuff[infoOffset], 21);
            infoOffset += 20;
            GetVin(&infoBuff[infoOffset], 21);
            infoOffset += 17;
            /*reserved ，默认填充 0x00  PDID*/
            infoOffset += 16;
            /*swversion*/
            GetSwVersion(OEMDATA_VERSION_PATH_NAME, &infoBuff[infoOffset], 7);
            infoOffset += 7;
            /*ble mac*/
            infoOffset += 6;
            SendRemoteCmdResult(formatIotData, (gchar *)infoBuff, infoOffset, 0x6C93);//正确回复
        }
        else if(formatIotData->subfunction == 0x0D00) //MD5值查询（MD5 签名信息）
        {
            gchar md5Buff[65] = {0};
            //获取APP的MD5值
            snprintf(systemCmdBuff, 128, "md5sum %s > %s", SYSTEM_CMD_TMP_FILE, SYSTEM_CMD_TMP_FILE);
            GetSystemRetValue(systemCmdBuff);
            // 在文件中获取MD5到md5Buff，然后将文件内容擦除（uaes_app_MD5）
            GetFotaTmpFileContentAndEmptyFile(SYSTEM_CMD_TMP_FILE, md5Buff, 65);
            snprintf(systemCmdBuff, 128, "md5sum %s > %s", SYSTEM_CMD_TMP_FILE, SYSTEM_CMD_TMP_FILE);
            GetSystemRetValue(systemCmdBuff);
            // 在文件中获取MD5到md5Buff第32个字节开始，然后将文件内容擦除（nts_ota_main_MD5）
            GetFotaTmpFileContentAndEmptyFile(SYSTEM_CMD_TMP_FILE, &md5Buff[32], 33);

            SendRemoteCmdResult(formatIotData, md5Buff, 64, 0x6C93);//正确回复
        }
    }
    else if( formatIotData->serviceID == 0xD926) //采集平台，mqtt
    {
        /*设置采集平台的地址*/
        if(formatIotData->subfunction == 0x1300)
        {
            SetCollectionAddr(formatIotData);
        }
        else if(formatIotData->subfunction == 0x1000)/*设置MQTT的地址*/
        {
            SetCarControlMqttAddr(formatIotData);
        }
        else if(formatIotData->subfunction == 0x1600) /*设置采集上报频率*/
        {
            SetNationalReportRate(formatIotData);
        }
    }
    return RET_OK;
}

/*
gint32 checkIPonInterface(const char* ifname, struct in_addr ipaddr) 
{
    FILE *fp;
    char line[256];

    fp = fopen("/proc/net/route", "r");
    if (fp == NULL) {
        LOG_Debug("open /proc/net/route failed");
        return -1;
    }

    // 将IPv4地址从网络字节序转换为主机字节序
    uint32_t host_order_addr = ntohl(ipaddr.s_addr);

    // 将主机字节序IPv4地址转换为小端十六进制字符串
    char hex_str[sizeof("0xFFFFFFFF")] = "0"; // 存储十六进制字符串的缓冲区
    snprintf(hex_str, sizeof(hex_str), "%02X%02X%02X%02X", 
             (host_order_addr & 0xFF),
             ((host_order_addr >> 8) & 0xFF),
             ((host_order_addr >> 16) & 0xFF),
             ((host_order_addr >> 24) & 0xFF));
	LOG_Debug("hex_str = %s", hex_str);

    while (fgets(line, sizeof(line), fp)) {
		LOG_Debug("line = %s", line);
        char iface[16], dest[16];
        sscanf(line, "%s %s", iface, dest);
		LOG_Debug("iface = %s, dest = %s", iface, dest);

        if (strcmp(dest, hex_str) == 0 && strcmp(iface, ifname) == 0) 
		{
			LOG_Debug("IP %s has bound on ccinet1", hex_str);
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}*/

gint ParserDiagnosisData(gpointer diagnosisData, guint u32Len)
{
    Json::Reader jsonReader;
    Json::Value jsonValue;
    guchar *iotData = NULL;
    gint iotDataLen = 0;
    if(NULL == diagnosisData)
    {
        LOG_Error("diagnosisData is NULL");
        return RET_ERR;
    }

    // if(jsonReader.parse((gchar *)diagnosisData,jsonValue))
    // {
    //     LOG_Debug("g_root[\"DATE\"]:%s\n",jsonValue["DATE"].asString().c_str());
    //     LOG_Debug("g_root[\"TY\"]:%s\n",jsonValue["TY"].asString().c_str());
    //     LOG_Debug("g_root[\"VIN\"]:%s\n",jsonValue["VIN"].asString().c_str());
    //     LOG_Debug("g_root[\"IOT\"]:%s\n",jsonValue["IOT"].asString().c_str());
    //     iotDataLen = jsonValue["TY"].asString().length() / 2 + 1;
    //     if(0 == jsonValue["TY"].asString().compare("03"))
    //     {
    //         iotData = (guchar *)g_try_malloc0(iotDataLen);
    //         if(NULL == iotData)
    //         {
    //             LOG_Error("iotData malloc error\n");
    //             return RET_ERR;
    //         }
    //         StrToHex(iotData,(guchar *)(jsonValue["IOT"].asString().c_str()),jsonValue["TY"].asString().length());
    //         ParserIotData(iotData,iotDataLen);
    //         g_free(iotData);
    //     }
    // }
    // else
    // {
    //     return RET_ERR;
    // }
    iotData = (guchar *)g_try_malloc0(u32Len);
    if(NULL == iotData)
    {
        LOG_Error("iotData malloc error");
        return RET_ERR;
    }
    LOG_Debug("diagnosisData:%s,u32Len:%d",diagnosisData,u32Len);
    StrToHex(iotData,(guchar *)(diagnosisData),u32Len);
    // 解析函数
    ParserIotData(iotData,u32Len/2);
    g_free(iotData);
    return RET_OK;
}


gint DiagnosisLogin()
{
    IotDataFormat *logintData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    logintData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logintData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    //登入上报
    //TBOX 重新连接车辆控制平台、并汇报登入
    logintData->headerVersion = 0x05;
    logintData->flag = 0x00;
    logintData->pkgLength = 0x0F00;
    logintData->eventID = 0x00;
    logintData->randomData = 0x00;
    logintData->serviceID = 0x3EC1;
    logintData->subfunction = 0x0100;
    logintData->payloadLength = 0x0200;
    payloadLength = 2;
    logintDataLen = htons(logintData->pkgLength);
    craData = CalCrcCal((guint8 *)logintData,logintDataLen);
    logintData->payloadAndCheck[0] = craData / 256;
    logintData->payloadAndCheck[1] = craData % 256;

    return SendDiagnosisData((gpointer)logintData,(payloadLength + htons(logintData->pkgLength)),2);
}

gint DiagnosisLogout()
{
    IotDataFormat *logouttData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    //登出上报
    //向车辆控制平台汇报登出、并关闭系统
    logouttData->headerVersion = 0x05;
    logouttData->flag = 0x00;
    logouttData->pkgLength = 0x0F00;
    logouttData->eventID = 0x00;
    logouttData->randomData = 0x00;
    logouttData->serviceID = 0x3EC1;
    logouttData->subfunction = 0x0300;
    logouttData->payloadLength = 0x0200;
    payloadLength = 2;

    logintDataLen = htons(logouttData->pkgLength);
    craData = CalCrcCal((guint8 *)logouttData,logintDataLen);

    logouttData->payloadAndCheck[0] = craData / 256;
    logouttData->payloadAndCheck[1] = craData % 256;
    return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),2);
}

gint DiagnosisSleep()
{
    IotDataFormat *sleepData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 sleepDataLen = 0;
    gint errorReturn;
    sleepData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == sleepData)
    {
        LOG_Error("sleepData malloc error");
        return RET_ERR;
    }

    sleepData->headerVersion = 0x04;
    sleepData->flag = 0x00;
    sleepData->pkgLength = 0x0F00;
    sleepData->eventID = 0x00;
    sleepData->randomData = 0xDDCCBBAA;
    sleepData->serviceID = 0x3EC1;
    sleepData->subfunction = 0x0200;
    sleepData->payloadLength = 0x0500;
    payloadLength = 5;

    //设置payload为0x01,进入休眠,保持长连接,支持数据业务唤醒
    sleepData->payloadAndCheck[0] = 0x01;

    sleepDataLen = htons(sleepData->pkgLength);
    craData = CalCrcCal((guint8 *)sleepData,sleepDataLen + 3);

    sleepData->payloadAndCheck[payloadLength - 2] = craData / 256;
    sleepData->payloadAndCheck[payloadLength - 1] = craData % 256;
    //return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),2);
    errorReturn = SendDiagnosisData((gpointer)sleepData,(payloadLength + htons(sleepData->pkgLength)),2);
    g_free(sleepData);
    return errorReturn;    
}

gint DiagnosisWakeUp(uint8_t wakeUpSource)
{
    IotDataFormat *wakeUpData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 wakeUpDataLen = 0;
    gint source = 0;
    gint errorReturn;
    wakeUpData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == wakeUpData)
    {
        LOG_Error("wakeUpData malloc error");
        return RET_ERR;
    }

    wakeUpData->headerVersion = 0x04;
    wakeUpData->flag = 0x00;
    wakeUpData->pkgLength = 0x0F00;
    wakeUpData->eventID = 0x00;
    wakeUpData->randomData = 0xDDCCBBAA;
    wakeUpData->serviceID = 0x3EC1;
    wakeUpData->subfunction = 0x0400;
    wakeUpData->payloadLength = 0x0500;
    payloadLength = 5;

    //判断唤醒源
    switch (wakeUpSource)
    {
    case PM_WAKEUP_SOURCE_TIMER_HEARTBEAT:
        source = 0x10;
        break;
    case PM_WAKEUP_SOURCE_MCU:
    case PM_WAKEUP_SOURCE_BLE:
    case PM_WAKEUP_SOURCE_G_SENSOR:
        source = 0x08;
        break;
    case PM_WAKEUP_SOURCE_OTA:
    case PM_WAKEUP_SOURCE_DTC:
    case PM_WAKEUP_SOURCE_DATA:
        source = 0x04;
        break;
    case PM_WAKEUP_SOURCE_RING:
    case PM_WAKEUP_SOURCE_ECALL:
        source = 0x02;
        break;
    case PM_WAKEUP_SOURCE_SMS:
        // 与休眠发送包内容重叠,是否唤醒是依靠MCU唤醒源区分的?    
        source = 0x01;
        break;
    default:
        break;
    }
    wakeUpData->payloadAndCheck[0] = source;
    //MCU唤醒源默认0x00 0x03
    wakeUpData->payloadAndCheck[1] = 0x00;
    wakeUpData->payloadAndCheck[2] = 0x03;

    wakeUpDataLen = htons(wakeUpData->pkgLength);
    craData = CalCrcCal((guint8 *)wakeUpData,wakeUpDataLen + 3);

    wakeUpData->payloadAndCheck[payloadLength - 2] = craData / 256;
    wakeUpData->payloadAndCheck[payloadLength - 1] = craData % 256;
    //return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),2);
    errorReturn = SendDiagnosisData((gpointer)wakeUpData,(payloadLength + htons(wakeUpData->pkgLength)),2);
    g_free(wakeUpData);
    return errorReturn;
}


gint DiagnosisHeartbeat()
{
    IotDataFormat *logouttData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    gint errorReturn;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = 0x04;
    logouttData->flag = 0x00;
    logouttData->pkgLength = 0x0F00;
    logouttData->eventID = 0x00;
    logouttData->randomData = 0xDDCCBBAA;
    logouttData->serviceID = 0x3EC1;
    logouttData->subfunction = 0x0400;
    logouttData->payloadLength = 0x0500;
    payloadLength = 5;

    logintDataLen = htons(logouttData->pkgLength);
    craData = CalCrcCal((guint8 *)logouttData,logintDataLen + 3);

    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    //return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),2);
    errorReturn = SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),2);
    g_free(logouttData);
    return errorReturn;
}

static gint GetSwVersion(const char *filePathName, guchar *pSwVersion,  gint swVersionLen)
{
    char readBuff[128] = {0};
    gint retValue = 0;
    gint fileFd = -1;
    char projectNum[4] = {0};

    if(NULL == pSwVersion || swVersionLen < 7)
    {
        return RET_ERR;
    }

    fileFd = open(filePathName, O_RDONLY);
    if(fileFd < 0)
    {
        LOG_Error("open %s error", filePathName);
        fileFd = open(OEMAPP_VERSION_PATH_NAME, O_RDONLY);
    }

    if(fileFd < 0)
    {
        LOG_Error("open %s error", OEMAPP_VERSION_PATH_NAME);
        return RET_ERR;
    }

    retValue = read(fileFd, readBuff, 128);
    if(retValue < 11)
    {
        LOG_Error("read sw version error %s ", filePathName);
        close(fileFd);
        return RET_ERR;
    }

    pSwVersion[0] = readBuff[5];
    pSwVersion[1] = readBuff[6];
    pSwVersion[2] = readBuff[7];
    pSwVersion[3] = readBuff[8];
    pSwVersion[4] = readBuff[9];
    pSwVersion[5] = readBuff[10];

    projectNum[0] = readBuff[0];
    projectNum[1] = readBuff[1];
    projectNum[2] = readBuff[2];

    pSwVersion[6] = atoi(projectNum);
    
    close(fileFd);

    return retValue;
}

static gint SvaeBLeMac(guchar *bleMac, gint bleMacLen)
{
    char bleMacbuff[128] = {0};
    gint retValue = -1;

    if(NULL == bleMac)
    {
        return RET_ERR;
    }

    if(0 == bleMac[0]  && 0 == bleMac[1] && 0 == bleMac[2]\
    && 0 == bleMac[3]  && 0 == bleMac[4] && 0 == bleMac[5])
    {
        LOG_Debug("bleMac is 0");
        return RET_ERR;
    }

    if(0 == access(BLE_MAC_PATH_NAME, F_OK))
    {
        LOG_Debug("%s is exist", BLE_MAC_PATH_NAME);
        if(0 == access(BLE_MAC_FLASH_PATH_NAME, F_OK))
        {
            LOG_Debug("%s is exist", BLE_MAC_FLASH_PATH_NAME);
            return RET_OK;
        }
    }

    snprintf(bleMacbuff, 128, "echo '%02X%02X%02X%02X%02X%02X' > %s; cp %s %s",
                            bleMac[0], bleMac[1], bleMac[2],
                            bleMac[3], bleMac[4], bleMac[5],
                            BLE_MAC_PATH_NAME, BLE_MAC_PATH_NAME, BLE_MAC_FLASH_PATH_NAME);
    retValue = GetSystemRetValue(bleMacbuff);
    if(0 == retValue)
    {
        return RET_OK;
    }
    else
    {
        return RET_ERR;
    }
}

static gint GetBleMac(unsigned char *pResult, guint16 resultLen)
{
    unsigned char bleMacbuff[32] = {0};
    gint retValue = -1;

     if(NULL == pResult || resultLen < 6)
     {
         return RET_ERR;
     }
 
    if(0 == g_bleMac[0]  && 0 == g_bleMac[1] && 0 == g_bleMac[2]\
    && 0 == g_bleMac[3]  && 0 == g_bleMac[4] && 0 == g_bleMac[5])
    {
        retValue = GetFileContent(BLE_MAC_PATH_NAME, (char *)bleMacbuff, 32);
        if(retValue > 0)
        {
            StrToHex(pResult, bleMacbuff, retValue/2);
            return RET_OK;
        }
        retValue = GetFileContent(BLE_MAC_FLASH_PATH_NAME, (char *)bleMacbuff, 32);
        if(retValue > 0)
        {
            StrToHex(pResult, bleMacbuff, retValue/2);
            return RET_OK;
        }
        return RET_ERR;
    }


    pthread_mutex_lock(&g_controlLock);
    memcpy(pResult, g_bleMac, 6);
    LOG_Debug("g_bleMac %x:%x:%x:%x:%x:%x", 
                        pResult[0], pResult[1], pResult[2], pResult[3], pResult[4], pResult[5]);
    pthread_mutex_unlock(&g_controlLock);
    return RET_OK;
}

gint DiagnosisUploadeVersion()
{
    IotDataFormat *logouttData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    gint errorReturn;
    gint offset = 0;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 128);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = 0x04;
    logouttData->flag = 0x00;
    logouttData->pkgLength = 0x0F00;
    logouttData->eventID = 0x00;
    //logouttData->randomData = 0xDDCCBBAA;
    logouttData->serviceID = 0x6E91;
    logouttData->subfunction = 0x0700;
    logouttData->payloadLength = 0x5300;

    GetIMEI(&logouttData->payloadAndCheck[offset], 16);
    offset += 15;
    GetIccid(&logouttData->payloadAndCheck[offset], 21);
    offset += 20;
    GetVin(&logouttData->payloadAndCheck[offset], 18);
    offset += 17;
    /*reserved*/
    offset += 16;
    /*swversion*/
    GetSwVersion(OEMDATA_VERSION_PATH_NAME, &logouttData->payloadAndCheck[offset], 7);
    offset += 7;

    /*ble mac*/
    GetBleMac(&logouttData->payloadAndCheck[offset], 128 - offset);
    LOG_Debug("g_bleMac %x:%x:%x:%x:%x:%x", 
                        logouttData->payloadAndCheck[offset], 
                        logouttData->payloadAndCheck[offset + 1], 
                        logouttData->payloadAndCheck[offset + 2], 
                        logouttData->payloadAndCheck[offset + 3], 
                        logouttData->payloadAndCheck[offset + 4], 
                        logouttData->payloadAndCheck[offset + 5]);
    offset += 6;

    payloadLength = offset + 2;

    logintDataLen = htons(logouttData->pkgLength);
    craData = CalCrcCal((guint8 *)logouttData,logintDataLen + offset);

    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    //return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),2);
    errorReturn = SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),2);
    g_free(logouttData);
    return errorReturn;
}


gint SendRemoteControlAck(IotDataFormat remoteControlData)
{
    IotDataFormat *logouttData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = remoteControlData.headerVersion;
    logouttData->flag = remoteControlData.flag;
    logouttData->pkgLength = remoteControlData.pkgLength;
    logouttData->eventID = remoteControlData.eventID;
    logouttData->randomData = remoteControlData.randomData;
    logouttData->serviceID = 0x6D92;
    logouttData->subfunction = 0x0800;
    logouttData->payloadLength = 0x0200;
    payloadLength = 2;

    logintDataLen = htons(logouttData->pkgLength);
    craData = CalCrcCal((guint8 *)logouttData,logintDataLen);

    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),3);
}

gint SendRemoteControlResetAck(IotDataFormat remoteControlData,guchar status)
{
    IotDataFormat *logouttData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = remoteControlData.headerVersion;
    logouttData->flag = remoteControlData.flag;
    logouttData->pkgLength = remoteControlData.pkgLength;
    logouttData->eventID = remoteControlData.eventID;
    logouttData->randomData = remoteControlData.randomData;
    //logouttData->serviceID = 0x6D92;
    //logouttData->subfunction = 0x0400;
	if(status == 1)
	{
		logouttData->serviceID = 0x6D92;
		logouttData->subfunction = 0x0400;
    	logouttData->payloadLength = 0x0200;
    	payloadLength = 2;
	}else{
        //车速大于2km/h 错误码
        logouttData->payloadAndCheck[0] = 0x32;
		//logouttData->payloadAndCheck[2] |= (1<<7);
        logouttData->payloadAndCheck[1] = 0x55;

		logouttData->serviceID = 0xFFFF;
		logouttData->subfunction = 0xDD22;
    	logouttData->payloadLength = 0x0600;
    	payloadLength = 6;
	}

    logintDataLen = htons(logouttData->pkgLength);
    craData = CalCrcCal((guint8 *)logouttData,logintDataLen + payloadLength - 2);

    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;

    LOG_Debug("0x0240_28_logouttData->payloadAndCheck: %x:%x:%x:%x:%x:%x", 
                        logouttData->payloadAndCheck[0], 
                        logouttData->payloadAndCheck[1], 
                        logouttData->payloadAndCheck[2], 
                        logouttData->payloadAndCheck[3], 
                        logouttData->payloadAndCheck[4], 
                        logouttData->payloadAndCheck[5]);

    return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),3);
}
gint SendRemoteControlSeatResult(IotDataFormat remoteControlData,guchar status, char *result, int resultLen)
{
    IotDataFormat *logouttData = NULL;
    gint retValue = -1;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    char resultBuff[8] = {0};
    char type = 0x02;
    if(NULL == result || 4 > resultLen)
    {
        LOG_Error("logintData result error");
        return RET_ERR;        
    }

    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = remoteControlData.headerVersion;
    logouttData->flag = remoteControlData.flag;
    logouttData->pkgLength = remoteControlData.pkgLength;
    logouttData->eventID = remoteControlData.eventID;
    logouttData->randomData = remoteControlData.randomData;

    logouttData->payloadAndCheck[0] = 0x01;

    switch (status)
    {
    case 0:
        logouttData->payloadAndCheck[0] = 0x00;
        break;
    case 1:
        if(MSG_SUBCMD_CONTROL_OPEN_SEAT_HEAT == result[0] || MSG_SUBCMD_CONTROL_CLOSE_SEAT_HEAT == result[0])
        {
            //新增座椅加热错误码
            resultBuff[0] = 0x33;
            resultBuff[1] = result[2];
        }
        else
        {
            //新增座椅通风错误码
            resultBuff[0] = 0x34;
            resultBuff[1] = result[2];
        }

        break;
    case 2:
        // 通用错误码 0x32
        LOG_Error("IBC/BCM请求失败-通用错误码");
        //新增通用错误码
        resultBuff[0] = 0x32;
        // 第二个字节为远控-IBC/BCM请求失败的原因
        resultBuff[1] = result[2];
        resultBuff[2] = 0x00;
        resultBuff[3] = 0x00;
        break;
    case 3:
        // 超时
        LOG_Error("控制超时\n");
        resultBuff[0] = 0x16;

        resultBuff[1] = 0x00;
        resultBuff[2] = 0x00;
        resultBuff[3] = 0x00;
        resultBuff[4] = 0x00; 
        break;
    case 4:
        if(result[0] == 1)
        {
            //KL30电压低于设定电压
            resultBuff[0] = 0x32;
            resultBuff[1] = 0x08;
            resultBuff[2] = 0x04;
            resultBuff[3] = 0x00;
        }
        else
        {
            //0X02、0X03、0X04鉴权失败
            resultBuff[0] = 0x1C;
            resultBuff[1] = 0x01;
            resultBuff[2] = 0x00;
            resultBuff[3] = 0x00;
        }
        break;    
    default:
        break;
    }

    logouttData->serviceID = 0x6D92;
    logouttData->subfunction = 0x1E00;
    logouttData->payloadLength = 0x0900;
    payloadLength = 9;
    memset(logouttData->payloadAndCheck, 0 , 7);

        
    logouttData->payloadAndCheck[1] = resultBuff[0];
    logouttData->payloadAndCheck[2] = resultBuff[1];
    logouttData->payloadAndCheck[3] = resultBuff[2];
    logouttData->payloadAndCheck[4] = resultBuff[3];

    logintDataLen = htons(logouttData->pkgLength);
    craData = CalCrcCal((guint8 *)logouttData,logintDataLen + payloadLength - 2);
    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    retValue = SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)), type);

    g_free(logouttData);
    return retValue;
}


gint SendRemoteControlAcResult(IotDataFormat remoteControlData,guchar status, char *result, int resultLen)
{
    IotDataFormat *logouttData = NULL;
    gint retValue = -1;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    char resultBuff[8] = {0};
    char type = 0x03;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = remoteControlData.headerVersion;
    logouttData->flag = remoteControlData.flag;
    logouttData->pkgLength = remoteControlData.pkgLength;
    logouttData->eventID = remoteControlData.eventID;
    logouttData->randomData = remoteControlData.randomData;
    if(2 == status)
    {
        // 通用错误码 0x32
        LOG_Error("IBC/BCM请求失败-通用错误码");
        //新增通用错误码
        resultBuff[0] = 0x32;
        // 第二个字节为远控-IBC/BCM请求失败的原因
        resultBuff[1] = result[2];
        resultBuff[2] = 0x00;
        resultBuff[3] = 0x00;
    }
    else if(3 == status)
    {
        // 超时
        LOG_Error("控制超时");
        resultBuff[0] = 0x16;

        resultBuff[1] = 0x00;
        resultBuff[2] = 0x00;
        resultBuff[3] = 0x00;
        resultBuff[4] = 0x00;        
    }
    else if(4 == status)
    {
        if(result[0] == 1)
        {
            //KL30电压低于设定电压
            resultBuff[0] = 0x32;
            resultBuff[1] = 0x08;
            resultBuff[2] = 0x04;
            resultBuff[3] = 0x00;
        }
        else
        {
            //0X02、0X03、0X04鉴权失败
            resultBuff[0] = 0x1C;
            resultBuff[1] = 0x01;
            resultBuff[2] = 0x00;
            resultBuff[3] = 0x00;
        }
    }
    else if(0 != status && 0x0118 == g_controlCmd)
    {
        if(1 == result[2])
        {
            resultBuff[0] = 0x23;
            resultBuff[1] = 0x20;
        }
        else if(2 == result[2])
        {
            resultBuff[0] = 0x23;
            resultBuff[1] = 0x80;
        }
        else if(5 == result[2])
        {
            resultBuff[0] = 0x11;
            resultBuff[1] = 0x80;
        }
        else if(0 != result[3])
        {
            resultBuff[0] = 0x2F;
            resultBuff[1] = result[3];
        }
        else if(0 != result[4])
        {
            resultBuff[0] = 0x30;
            resultBuff[1] = result[4];
        }
        else if(0 != result[5])
        {
            resultBuff[0] = 0x30;
            resultBuff[2] = result[5];
        }
        else if(0 != result[6])
        {
            resultBuff[0] = 0x30;
            resultBuff[3] = result[6];
        }
    }
    else if(0 != status && 0x0119 == g_controlCmd)
    {
        if(0 != result[2])
        {
            resultBuff[0] = 0x23;
            resultBuff[1] = 0x40;
        }
        else if(0 != result[3])
        {
            resultBuff[0] = 0x2F;
            resultBuff[1] = result[3];
        }
        else if(0 != result[4])
        {
            resultBuff[0] = 0x30;
            resultBuff[1] = result[4];
        }
        else if(0 != result[5])
        {
            resultBuff[0] = 0x30;
            resultBuff[2] = result[5];
        }
        else if(0 != result[6])
        {
            resultBuff[0] = 0x30;
            resultBuff[3] = result[6];
        }
    }

    if(0x0119 == g_controlCmd && 0 != status)
    {
        logouttData->serviceID = 0xFFFF;
        logouttData->subfunction = 0xDD22;
        logouttData->payloadLength = 0x0600;
        payloadLength = 6;
        memcpy(logouttData->payloadAndCheck, resultBuff, 4);
    }
    else if(0x0119 == g_controlCmd && 0 == status)
    {
        logouttData->serviceID = 0x6D92;
        logouttData->subfunction = 0x0100;
        logouttData->payloadLength = 0x0800;
        payloadLength = 8;
        memset(logouttData->payloadAndCheck, 0 , 6);
        logouttData->payloadAndCheck[0] = 0x01;
        logouttData->payloadAndCheck[1] = 0x19;
        logouttData->payloadAndCheck[2] = 0x01;
    }
    else if(0x0118 == g_controlCmd)
    {
        logouttData->serviceID = 0x6D92;
        logouttData->subfunction = 0x9000;
        logouttData->payloadLength = 0x0700;
        payloadLength = 7;
        logouttData->payloadAndCheck[0] = status;
        memcpy(&logouttData->payloadAndCheck[1], resultBuff, 4);
        type = 0x02;
    }
    

    logintDataLen = htons(logouttData->pkgLength);
    //logouttData->payloadAndCheck[0] = status;
    craData = CalCrcCal((guint8 *)logouttData,logintDataLen + payloadLength - 2);
    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    retValue = SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)), type);

    g_free(logouttData);
    return retValue;
}

gint SendRemoteControlStatus(IotDataFormat remoteControlData,guchar status, guint16 subfunction)
{
    IotDataFormat *logouttData = NULL;
    gint retValue = -1;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = remoteControlData.headerVersion;
    logouttData->flag = remoteControlData.flag;
    logouttData->pkgLength = remoteControlData.pkgLength;
    logouttData->eventID = remoteControlData.eventID;
    logouttData->randomData = remoteControlData.randomData;
    logouttData->serviceID = 0xFFFF;
    logouttData->subfunction = subfunction;
    logouttData->payloadLength = 0x0300;
    payloadLength = 3;

    logintDataLen = htons(logouttData->pkgLength);
    logouttData->payloadAndCheck[0] = status;
    craData = CalCrcCal((guint8 *)logouttData,logintDataLen + payloadLength - 2);
    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    retValue = SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),2);

    g_free(logouttData);
    return retValue;
}

guint8 GetControlErrorCodeFlg(guint8 cmd)
{
    guint8 retValue = 0x00;
    switch(cmd)
    {
        case MSG_SUBCMD_CONTROL_REMOTE_SPEED_LIMIT:
            retValue = 0x25;
            break;
        case MSG_SUBCMD_CONTROL_PARKING_HEAT:
            retValue = 0x26;
            break;
        case MSG_SUBCMD_CONTROL_CHARGE_HEAT:
            retValue = 0x26;
            break;
        case MSG_SUBCMD_CONTROL_OPEN_DOOR:
        case MSG_SUBCMD_CONTROL_INTHE_CONTROL:
        case MSG_SUBCMD_CONTROL_OPEN_TRUNK:
        case MSG_SUBCMD_CONTROL_CLOSE_TRUNK:
        case MSG_SUBCMD_CONTROL_CLOSE_DOOR:
        case MSG_SUBCMD_CONTROL_FLASH_WHISTLE:
        case MSG_SUBCMD_CONTROL_CLOSE_WINDOW:
		case MSG_SUBCMD_CONTROL_OPEN_WINDOW:
		case MSG_SUBCMD_CONTROL_OPEN_SKYLIGHT:
		case MSG_SUBCMD_CONTROL_CLOSE_SKYLIGHT:
		case MSG_SUBCMD_CONTROL_OPEN_LIGHTS:
		case MSG_SUBCMD_CONTROL_CLOSE_LIGHTS:
            retValue = 0x11;
            break;
        case MSG_SUBCMD_CONTROL_CLOSE_AC:
			retValue = 0x20;
			break;
        case MSG_SUBCMD_CONTROL_START_ENGINE:
		case MSG_SUBCMD_CONTROL_TURNOFF_ENGINE:
		    retValue = 0x23;
            break;
        case MSG_SBUCMD_CONTROL_BLE_BUSY:
            retValue = 0x07;
            break;
        case MSG_SBUCMD_CONTROL_BLE_NOT_FIND:
            retValue = 0x1F;
            break;
        case MSG_SBUCMD_CONTROL_BLE_KEY_MAC:
            retValue = 0x24;
            break;
        case MSG_SUBCMD_CONTROL_OPEN_SENTINE_MODE:
        case MSG_SUBCMD_CONTROL_CLOSE_SENTINE_MODE:
            retValue = 0x22;
            break;
        case MSG_SUBCMD_CONTROL_REMOTE_AUTHORIZED_IGNITION:
            retValue = 0x1C;
            break;
        // case MSG_SUBCMD_CONTROL_UNLOCK_THE_CAR:
        //     retValue = 0x2D;
            break;
    }
    return retValue;
}

static gint SendDD22ControlResultData(guint8         cmd,
                                      IotDataFormat *controlData, 
                                      unsigned char *pResult, 
                                      guint16       resultLen, 
                                      gint          status)
{
    gint retValue = -1;
    guint16 craData = 0;
    IotDataFormat *pIotData = NULL;

    if(NULL == pResult || resultLen < 4 || NULL == controlData)
        return retValue;

    pIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + resultLen + 2);
    if(NULL == pIotData)
    {
        LOG_Error("SendControlResultData malloc error");
        return retValue;
    }
    
    pIotData->headerVersion = controlData->headerVersion;
    pIotData->flag          = controlData->flag;
    pIotData->pkgLength     = controlData->pkgLength;
    pIotData->eventID       = controlData->eventID;
    pIotData->randomData    = controlData->randomData;

    switch (status)
    {
    case 0:
        // 执行成功
        pIotData->serviceID   = 0x6D92;
        pIotData->subfunction = 0x0100;
        pIotData->payloadLength = 0x0800;
        memcpy(pIotData->payloadAndCheck, controlData->payloadAndCheck, 6);
        break;
    case 1:
        // 已经确定错误源的错误
        pIotData->serviceID   = 0xFFFF;
        pIotData->subfunction = 0xDD22;
        pIotData->payloadLength = 0x0600;
        pIotData->payloadAndCheck[0] = GetControlErrorCodeFlg(cmd);
        pIotData->payloadAndCheck[1] = pResult[0];
        pIotData->payloadAndCheck[2] = pResult[1];
        pIotData->payloadAndCheck[3] = pResult[2];
        pIotData->payloadAndCheck[4] = pResult[3];

        //开关尾门
        if(cmd == MSG_SUBCMD_CONTROL_OPEN_TRUNK || cmd == MSG_SUBCMD_CONTROL_CLOSE_TRUNK)
        {
            LOG_Error("尾门控制错误：");
            if(pResult[0] & 0x80)
            {
				pIotData->payloadAndCheck[0] = 0x11;
				pIotData->payloadAndCheck[1] = pResult[0];
				pIotData->payloadAndCheck[2] = 0x00;
				pIotData->payloadAndCheck[3] = pResult[2];
            }
            else if(pResult[0] & 0x40)
            {
                //新增通用错误码
                // 车速大于2km/h
				pIotData->payloadAndCheck[0] = 0x32;
				pIotData->payloadAndCheck[1] = 0x55;
				pIotData->payloadAndCheck[2] = 0x00;
				pIotData->payloadAndCheck[3] = 0x00;
            }
            else if(pResult[1] & 0x80 || pResult[1] & 0x40)
            {
                // 电动尾门故障、电动尾门相应超时
                pIotData->payloadAndCheck[0] = 0x31;
				pIotData->payloadAndCheck[1] = pResult[1];
				pIotData->payloadAndCheck[2] = 0x00;
				pIotData->payloadAndCheck[3] = 0x00;
            }
            else if(pResult[2] & 0x08)
            {
                //新增通用错误码
                // 变速箱当前档位为非 P
				pIotData->payloadAndCheck[0] = 0x32;
				pIotData->payloadAndCheck[1] = 0x21;
				pIotData->payloadAndCheck[2] = 0x00;
				pIotData->payloadAndCheck[3] = 0x00;                
            }
        }

        //此处与MCU对应的协议发生变动，需要重新适配上报协议,F510C适配，其他项目根据协议进行删减
        if((cmd == MSG_SUBCMD_CONTROL_CLOSE_DOOR || cmd == MSG_SUBCMD_CONTROL_OPEN_DOOR)
            && pResult[0] & 0x40)
        {
            // 解锁闭锁的第六位修改为了发生碰撞
            //新增通用错误码
            pIotData->payloadAndCheck[0] = 0x32;
            // 第二个字节为远控-IBC/BCM请求失败的原因
            // 0x1B=发生碰撞
            pIotData->payloadAndCheck[1] = 0x1B;
            pIotData->payloadAndCheck[2] = 0x00;
            pIotData->payloadAndCheck[3] = 0x00;
        }

        if( cmd == MSG_SUBCMD_CONTROL_OPEN_DOOR && pResult[0] & 0x20)
        {
            // 开门的第5位修改为了车速大于2km/h
            //新增通用错误码
            pIotData->payloadAndCheck[0] = 0x32;

            pIotData->payloadAndCheck[1] = 0x55;
            // 第二个字节为远控-IBC/BCM请求失败的原因
            // 0x55=车速大于2km/h 或 信号无效
            pIotData->payloadAndCheck[2] = 0x00;
            pIotData->payloadAndCheck[3] = 0x00;
        }      

        //此处与MCU对应的协议发生变动，需要重新适配上报协议,pResult[0]若为0x12，说明其为电源错误，其为错误源，其他项目根据协议进行删减
        //检测到 BCM 远程电源模式 BCMRemtCtrlMod≠远程空调模式，则 TBOX 反馈关闭空调失败，当前电源模式不支持（错误源：0x12，错误码：0x00 0x00 0x80）。
        if(cmd == MSG_SUBCMD_CONTROL_CLOSE_AC && pResult[0] & 0x12)
        {
            //修改错误源
            pIotData->payloadAndCheck[0] = 0x12;
            // 固定为00 00 80
            pIotData->payloadAndCheck[1] = 0x00;
            pIotData->payloadAndCheck[2] = 0x00;
            pIotData->payloadAndCheck[3] = 0x80;
        }

        break;
    case 3:
        // 超时
        LOG_Error("控制超时");
        pIotData->serviceID   = 0xFFFF;
        pIotData->subfunction = 0xDD22;
        pIotData->payloadLength = 0x0600;
        if(cmd == MSG_SUBCMD_CONTROL_PARKING_HEAT || cmd == MSG_SUBCMD_CONTROL_CHARGE_HEAT)
        {
            pIotData->payloadAndCheck[0] = 0x26;
        }
        else
        {
            pIotData->payloadAndCheck[0] = 0x16;
        }
        
        // pIotData->payloadAndCheck[1] = pResult[0];
        pIotData->payloadAndCheck[1] = 0x00;
        pIotData->payloadAndCheck[2] = 0x00;
        pIotData->payloadAndCheck[3] = 0x00;
        pIotData->payloadAndCheck[4] = 0x00;
        break;
    case 2:
        // 通用错误码 0x32
        LOG_Error("IBC/BCM请求失败-通用错误码");
        pIotData->serviceID   = 0xFFFF;
        pIotData->subfunction = 0xDD22;
        pIotData->payloadLength = 0x0600;

        //新增通用错误码
        pIotData->payloadAndCheck[0] = 0x32;

        pIotData->payloadAndCheck[1] = pResult[0];
        // 第二个字节为远控-IBC/BCM请求失败的原因
        pIotData->payloadAndCheck[2] = 0x00;
        pIotData->payloadAndCheck[3] = 0x00;
        pIotData->payloadAndCheck[4] = 0x00;
        break;
    case 4:
        // 前置条件不满足
        pIotData->serviceID   = 0xFFFF;
        pIotData->subfunction = 0xDD22;
        pIotData->payloadLength = 0x0600;
        if(pResult[0] == 1)
        {
            //KL30电压低于设定电压
            pIotData->payloadAndCheck[0] = 0x32;
            pIotData->payloadAndCheck[1] = 0x08;
            pIotData->payloadAndCheck[2] = 0x00;
            pIotData->payloadAndCheck[3] = 0x00;
        }
        else
        {
            //0X02、0X03、0X04鉴权失败
            pIotData->payloadAndCheck[0] = 0x1C;
            pIotData->payloadAndCheck[1] = 0x01;
            pIotData->payloadAndCheck[2] = 0x00;
            pIotData->payloadAndCheck[3] = 0x00;
        }
        break;
    default:
        break;
    }

    craData = CalCrcCal((guint8 *)pIotData, htons(pIotData->pkgLength) + resultLen);
    pIotData->payloadAndCheck[resultLen] = craData / 256;
    pIotData->payloadAndCheck[resultLen + 1] = craData % 256;
   
    retValue = SendDiagnosisData((gpointer)pIotData,(resultLen + 2 + htons(pIotData->pkgLength)), 3);

    g_free(pIotData);

    return retValue;
}


static gint SendBLEKeyControlResultData(  guint8         cmd,
                                          IotDataFormat *controlData, 
                                          unsigned char *pResult, 
                                          guint16       resultLen, 
                                          gint          status)
{
    gint retValue = -1;
    guint16 craData = 0;
    IotDataFormat *pIotData = NULL;

    if(NULL == pResult || resultLen < 4)
    {
        return RET_ERR;
    }

    pIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + resultLen + 2);
    if(NULL == pIotData)
    {
        LOG_Error("SendControlResultData malloc error");
        return retValue;
    }
    
    pIotData->headerVersion = controlData->headerVersion;
    pIotData->flag          = controlData->flag;
    pIotData->pkgLength     = controlData->pkgLength;
    pIotData->eventID       = controlData->eventID;
    pIotData->randomData    = controlData->randomData;

    if(0 == status)
    {
        pIotData->serviceID   = 0x5EA1;
        pIotData->subfunction = controlData->subfunction;
        pIotData->payloadLength = 0x0200;
    }
    else
    {
        pIotData->serviceID   = 0xFFFF;
        pIotData->subfunction = 0xCE31;
        pIotData->payloadLength = 0x0600;
        pIotData->payloadAndCheck[0] = GetControlErrorCodeFlg(cmd);
        pIotData->payloadAndCheck[1] = pResult[0];
        pIotData->payloadAndCheck[2] = pResult[1];
        pIotData->payloadAndCheck[3] = pResult[2];
        pIotData->payloadAndCheck[4] = pResult[3];
    }

    craData = CalCrcCal((guint8 *)pIotData, htons(pIotData->pkgLength) + resultLen);
    pIotData->payloadAndCheck[resultLen] = craData / 256;
    pIotData->payloadAndCheck[resultLen + 1] = craData % 256;
   
    retValue = SendDiagnosisData((gpointer)pIotData,(resultLen + 2 + htons(pIotData->pkgLength)), 3);

    g_free(pIotData);
    LOG_Debug("end control");
    return retValue;
}

static gint SendSelectBLEKeyResultData(IotDataFormat *controlData, 
                                          unsigned char *pResult, 
                                          guint16       resultLen)
{
    gint retValue = -1;
    guint16 craData = 0;
    guchar resultDataBuff2[512] = {0};
    gint dataOffset = 0;
    IotDataFormat *pIotData = NULL;
    gint count = 1;

    if(NULL == pResult || resultLen <= 0)
    {
        return RET_ERR;
    }

    if(resultLen < 17)
    {
        resultLen = 17;
    }

    pIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + resultLen + 2);
    if(NULL == pIotData)
    {
        LOG_Error("SendControlResultData malloc error");
        return retValue;
    }
    
    pIotData->headerVersion = controlData->headerVersion;
    pIotData->flag          = controlData->flag;
    pIotData->pkgLength     = controlData->pkgLength;
    pIotData->eventID       = controlData->eventID;
    pIotData->randomData    = controlData->randomData;
    pIotData->serviceID   = 0x5EA1;
    pIotData->subfunction = controlData->subfunction;

    if(0 == pResult[0] || resultLen < (25 * pResult[0] + 16))
    {
        pIotData->payloadLength = 0x1300;
        dataOffset  = 17;
    }
    else
    {
        if(pResult[0] > 5)
        {
            pResult[0] = 5;
        }
        resultDataBuff2[dataOffset++] = pResult[0];

        memcpy(&resultDataBuff2[dataOffset], &pResult[count], 25 * pResult[0] + 16);
        dataOffset += 25 * pResult[0] + 16;
        pIotData->payloadLength = htons(dataOffset + 2);
    }
    memcpy(pIotData->payloadAndCheck, resultDataBuff2, dataOffset);

    craData = CalCrcCal((guint8 *)pIotData, htons(pIotData->pkgLength) + dataOffset);
    pIotData->payloadAndCheck[dataOffset] = craData / 256;
    pIotData->payloadAndCheck[dataOffset + 1] = craData % 256;
   
    retValue = SendDiagnosisData((gpointer)pIotData,(dataOffset + 2 + htons(pIotData->pkgLength)), 3);

    g_free(pIotData);
    LOG_Debug("end control");
    return retValue;
}


static gint SendShardCarControlResultData(IotDataFormat controlData, 
                                          unsigned char *pResult, 
                                          guint16       resultLen, 
                                          gint          status)
{
    gint retValue = -1;
    guint16 craData = 0;
    IotDataFormat *pIotData = NULL;

    if(NULL == pResult || resultLen < 4)
    {
        return RET_ERR;
    }

    pIotData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + resultLen + 2);
    if(NULL == pIotData)
    {
        LOG_Error("SendControlResultData malloc error");
        return retValue;
    }
    
    pIotData->headerVersion = controlData.headerVersion;
    pIotData->flag          = controlData.flag;
    pIotData->pkgLength     = controlData.pkgLength;
    pIotData->eventID       = controlData.eventID;
    pIotData->randomData    = controlData.randomData;

    if(0 == status)
    {
        pIotData->serviceID   = 0x5EA1;
        pIotData->subfunction = controlData.subfunction;
        pIotData->payloadLength = 0x0200;
    }
    else
    {
        pIotData->serviceID   = 0xFFFF;
        pIotData->subfunction = 0xCE31;
        pIotData->payloadLength = 0x0600;
        pIotData->payloadAndCheck[0] = 0x12;
        pIotData->payloadAndCheck[1] = pResult[0];
        pIotData->payloadAndCheck[2] = pResult[1];
        pIotData->payloadAndCheck[3] = pResult[2];
        pIotData->payloadAndCheck[4] = pResult[3];
    }

    craData = CalCrcCal((guint8 *)pIotData, htons(pIotData->pkgLength) + resultLen);
    pIotData->payloadAndCheck[resultLen] = craData / 256;
    pIotData->payloadAndCheck[resultLen + 1] = craData % 256;
   
    retValue = SendDiagnosisData((gpointer)pIotData,(resultLen + 2 + htons(pIotData->pkgLength)), 3);

    g_free(pIotData);
    LOG_Debug("end control");
    return retValue;
}

static gint ParserControlAckData(unsigned char *pResult, guint16 resultLen)
{
    gint retValue = RET_ERR;
    guchar resultDataBuff[6] = {0};
    static guchar controlFlag = 0;
    static guchar otaWakeupFlag = 0;
    guchar controlData[4] = {0};
    gint resultDataLen = 4;

    if(NULL == pResult || resultLen < 4)
        return retValue;

    resultDataBuff[0] = pResult[2];
    resultDataBuff[1] = pResult[3];

    LOG_Debug("<********** Control RetValue: cmd :%x, status:%x, errcode:%x,%x **********>", 
                    pResult[0], pResult[1], pResult[2], pResult[3]);
    LOG_DebugHex(pResult, resultLen, "Control RetValue Buff");

    switch(pResult[0])
    {
        //车速限制
        case MSG_SUBCMD_CONTROL_REMOTE_SPEED_LIMIT:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[SpeedRemoteE] = 0;
            if(NULL != g_speedRemoteControlData)
            {
                if(0 == pResult[1])
                {
                    pResult[1] = 1;
                }
                if(0x0A != pResult[2])
                {
                    retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_REMOTE_SPEED_LIMIT,
                                                    g_speedRemoteControlData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                }
                g_free(g_speedRemoteControlData);
                g_speedRemoteControlData = NULL;
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        //驻车保温
        case MSG_SUBCMD_CONTROL_PARKING_HEAT:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[ParkingHeatE] = 0;
            if(NULL != g_parkingHeatingRemoteControlData)
            {
                if(0x01 == pResult[1])
                {
                    // 平台协议如果驻车保温失败则返回 0x00 0x00 0x00，而私有协议0x01为失败，不能够直接使用
                    resultDataBuff[0] = 0x00;
                    resultDataBuff[1] = 0x00;
                    resultDataBuff[2] = 0x00;

                }
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_PARKING_HEAT, 
                                                    g_parkingHeatingRemoteControlData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_parkingHeatingRemoteControlData);
                g_parkingHeatingRemoteControlData = NULL;
            }            
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_CHARGE_HEAT:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[RemoteHeatE] = 0;
            if(NULL != g_remoteHeatControlData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_CHARGE_HEAT, 
                                                    g_remoteHeatControlData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_remoteHeatControlData);
                g_remoteHeatControlData = NULL;
            }            
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_OPEN_DOOR:
        case MSG_SUBCMD_CONTROL_INTHE_CONTROL:
        case MSG_SUBCMD_CONTROL_OPEN_TRUNK:
            pthread_mutex_lock(&g_controlLock);
            if(NULL != g_openTrunk)
            {
                resultDataBuff[0] = pResult[2];
                resultDataBuff[1] = pResult[3];
                resultDataBuff[2] = pResult[4];
                g_timeOutArray[OpenTrunkE] = 0;
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_OPEN_TRUNK, 
                                                    g_openTrunk, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_openTrunk);
                g_openTrunk = NULL;
            }
            if(NULL != g_doorUnLockData)
            {
                g_timeOutArray[DoorUnLockE] = 0;
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_OPEN_DOOR, 
                                                    g_doorUnLockData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_doorUnLockData);
                g_doorUnLockData = NULL;
            }            
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_CLOSE_TRUNK:
            pthread_mutex_lock(&g_controlLock);
            if(NULL != g_closeTrunk)
            {
                resultDataBuff[0] = pResult[2];
                resultDataBuff[1] = pResult[3];
                resultDataBuff[2] = pResult[4];
                g_timeOutArray[CloseTrunkE] = 0;
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_CLOSE_TRUNK, 
                                                    g_closeTrunk, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_closeTrunk);
                g_closeTrunk = NULL;
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_CLOSE_DOOR:
            resultDataBuff[0] = pResult[2];
            resultDataBuff[1] = 0;
            resultDataBuff[2] = pResult[3];
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[DoorLockE] = 0;
            if(NULL != g_doorLockData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_CLOSE_DOOR, 
                                                    g_doorLockData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_doorLockData);
                g_doorLockData = NULL;
            }            
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_FLASH_WHISTLE:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[SearchCarE] = 0;
            if(NULL != g_searchCarControlData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_FLASH_WHISTLE, 
                                                    g_searchCarControlData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_searchCarControlData);
                g_searchCarControlData = NULL;
            }            
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_SHARED_CAR_PREBOOK:
            pthread_mutex_lock(&g_controlLock);
            if(0 != g_timeOutArray[CarPreBook])
            {
                g_timeOutArray[CarPreBook] = 0;
                retValue = SendShardCarControlResultData(g_shardCarPreBook, resultDataBuff, resultDataLen, pResult[1]);
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_SHARED_CAR_PREBOOK_CANCEL:
            pthread_mutex_lock(&g_controlLock);
            if(0 != g_timeOutArray[CarPreBookCancel])
            {
                g_timeOutArray[CarPreBookCancel] = 0;
                retValue = SendShardCarControlResultData(g_shardCarPreBookCancle, resultDataBuff, resultDataLen, pResult[1]);
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_SHARED_CAR_RETURN:
            pthread_mutex_lock(&g_controlLock);
            if(0 != g_timeOutArray[CarReturn])
            {
                g_timeOutArray[CarReturn] = 0;
                retValue = SendShardCarControlResultData(g_shardCarReturn, resultDataBuff, resultDataLen, pResult[1]);
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_ODER_CHARGING:
           pthread_mutex_lock(&g_controlLock);
            if(0 != g_timeOutArray[ReserverCharE] && pResult[1] != 0)
            {
                resultDataBuff[1] = 0x24;
            }
            else
            {
                resultDataBuff[1] = 0x25;
            }
            g_timeOutArray[ReserverCharE] = 0;
            pthread_mutex_unlock(&g_controlLock);
            if(pResult[1] != 0 && controlFlag < 3)
            {
                controlFlag++;
                ControlReserverCharging(controlData, 4);
            }
            else if(pResult[1] == 0)
            {
                controlFlag = 0;
                SendRemoteCmdResult(&g_reserverChargingData, (gchar *)resultDataBuff, 0, 0x6D92);
            }
            else
            {
                resultDataBuff[0] = 0x11;
                SendRemoteCmdResult(&g_reserverChargingData, (gchar *)resultDataBuff, 4, 0xFFFF);
            }
            pthread_mutex_lock(&g_controlLock);
            g_ReserverChargingTime.mcuSetState = 1;
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_CLOSE_WINDOW:
	   	    pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[WindowClose] = 0;
            if(NULL != g_windowCloseData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_CLOSE_WINDOW, 
                                                    g_windowCloseData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_windowCloseData);
                g_windowCloseData = NULL;
            }            
            pthread_mutex_unlock(&g_controlLock);
            break;
	    case MSG_SUBCMD_CONTROL_OPEN_WINDOW:
	   	    pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[WindowOpen] = 0;
            if(NULL != g_windowOpenData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_OPEN_WINDOW, 
                                                    g_windowOpenData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_windowOpenData);
                g_windowOpenData = NULL;
            }            
            pthread_mutex_unlock(&g_controlLock);
            break;
		case MSG_SUBCMD_CONTROL_OPEN_SKYLIGHT:
	   	    pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[LightOpen] = 0;
            if(NULL != g_lightOpenData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_OPEN_SKYLIGHT, 
                                                    g_lightOpenData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_lightOpenData);
                g_lightOpenData = NULL;
            }            
            pthread_mutex_unlock(&g_controlLock);
            break;
		case MSG_SUBCMD_CONTROL_CLOSE_SKYLIGHT:
	   	    pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[LightClose] = 0;
            if(NULL != g_lightCloseData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_CLOSE_SKYLIGHT, 
                                                    g_lightOpenData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_lightCloseData);
                g_lightCloseData = NULL;
            }            
            pthread_mutex_unlock(&g_controlLock);
            break;
		case MSG_SUBCMD_CONTROL_START_ENGINE:
	   	    pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[PowerUp] = 0;
		    g_timeOutArray[OtaPowerUp] = 0;
			//g_timeOutArray[RemoteBLEEnable] = 0;
            if(NULL != g_powerUpData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_START_ENGINE, 
                                                    g_powerUpData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_powerUpData);
                g_powerUpData = NULL;
            } 
			else if(NULL != g_otaPowerUpData)
            {
            	retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_START_ENGINE, 
                                                    g_otaPowerUpData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_otaPowerUpData);
                g_otaPowerUpData = NULL;
			}
            else if(1 == g_otaPowerOnFlag)
            {
                if(0 == pResult[1])
                {
                    g_otaPowerOnFlag = 0x00;
                }
                else
                {
                    g_otaPowerOnFlag = 0xC0;
                }
            }
			/*else if(NULL != g_remoteBleEnableData)
            {
            	retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_START_ENGINE, 
                                                    g_remoteBleEnableData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_remoteBleEnableData);
                g_remoteBleEnableData = NULL;
			}*/
            pthread_mutex_unlock(&g_controlLock);
            break;
		case MSG_SUBCMD_CONTROL_TURNOFF_ENGINE:
	   	    pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[PowerDown] = 0;
            if(NULL != g_powerDownData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_TURNOFF_ENGINE, 
                                                    g_powerDownData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_powerDownData);
                g_powerDownData = NULL;
            }
            else if(1 == g_otaPowerOffFlag)
            {
                if(0 == pResult[1])
                {
                    g_otaPowerOffFlag = 0x00;
                }
                else
                {
                    g_otaPowerOffFlag = 0xC0;
                }
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_OPEN_AC_OLD:
        case MSG_SUBCMD_CONTROL_OPEN_AC:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[AcOn] = 0;
            if(NULL == g_acOnData)
            {
                pthread_mutex_unlock(&g_controlLock);
                break;
            }

            memset(resultDataBuff, 0, 4);
            LOG_Debug("g_controlCmd :%04x", g_controlCmd);
            if(0x0000 != g_controlCmd && 0x0105 != g_controlCmd)
            {
                SendRemoteControlAcResult(*g_acOnData, pResult[1], (char*)pResult, 6);
            }
            else if(0 == pResult[1])
            {
                SendRemoteControlStatus(*g_acOnData, pResult[2], 0x0900);
            }
            else
            {
                resultDataBuff[0] = pResult[2];
                g_acOnData->serviceID = 0x0900;
                SendRemoteControlStatus(*g_acOnData, pResult[2], 0x0900);
            }
            g_controlCmd = 0x0000;
            g_free(g_acOnData);
            g_acOnData = NULL;
            pthread_mutex_unlock(&g_controlLock);
            break;
		case MSG_SUBCMD_CONTROL_CLOSE_AC:
	   	    pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[AcOff] = 0;
            resultDataBuff[0] = pResult[2];
            resultDataBuff[1] = pResult[3];
            resultDataBuff[2] = pResult[4];
            resultDataBuff[3] = pResult[5];
            resultDataBuff[4] = pResult[6];
            LOG_Debug("g_controlCmd :%04x", g_controlCmd);
            if(0x0000 != g_controlCmd && 0x0119 == g_controlCmd)
            {
                SendRemoteControlAcResult(*g_acOffData, pResult[1], (char*)pResult, 6);
            }
            else if(NULL != g_acOffData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_CLOSE_AC, 
                                                    g_acOffData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
            }    
            g_controlCmd = 0x0000;    
            g_free(g_acOffData);
            g_acOffData = NULL;       
            pthread_mutex_unlock(&g_controlLock);
            break;
		case MSG_SUBCMD_CONTROL_REMOTE_AUTHORIZED_IGNITION:
			pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[RemoteBLEEnable] = 0;
		    if(NULL != g_remoteBleEnableData)
            {
                if(0 != pResult[1] && 2 != pResult[1] && 4 != pResult[1])
                {
                    resultDataBuff[0] = 0x80;
                    resultDataBuff[1] = 0x00;
                    resultDataBuff[2] = 0x00;
                    resultDataBuff[3] = 0x00;
                }
            	retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_REMOTE_AUTHORIZED_IGNITION, 
                                                    g_remoteBleEnableData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_remoteBleEnableData);
                g_remoteBleEnableData = NULL;
			}
            pthread_mutex_unlock(&g_controlLock);
			break;
		case MSG_SUBCMD_CONTROL_OPEN_LIGHTS:
            resultDataBuff[0] = 0;
            resultDataBuff[1] = 0;
            resultDataBuff[2] = 0;
            if(0x80 == (pResult[2] & 0x80))
            {
                resultDataBuff[0] = 0x80;
            }
            else if(0x10 == (pResult[2] & 0x10))
            {
                if(1 == g_lightFlag)
                {
                    resultDataBuff[2] = 0x04;
                }
                else if(2 == g_lightFlag)
                {
                    resultDataBuff[2] = 0x02;
                }
            }
	   	    pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[LightOpen] = 0;
            if(NULL != g_lightOpenData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_OPEN_LIGHTS, 
                                                    g_lightOpenData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_lightOpenData);
                g_lightOpenData = NULL;
            }            
            pthread_mutex_unlock(&g_controlLock);
            break;
		case MSG_SUBCMD_CONTROL_CLOSE_LIGHTS:
			resultDataBuff[0] = 0;
			resultDataBuff[1] = 0;
			resultDataBuff[2] = 0;
            if(0x80 == (pResult[2] & 0x80))
            {
                resultDataBuff[0] = 0x80;
            }
            else
            {
                pResult[1] = 0;
            }

	   	    pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[LightClose] = 0;
            if(NULL != g_lightCloseData)
            {
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_CLOSE_LIGHTS, 
                                                    g_lightCloseData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    pResult[1]);
                g_free(g_lightCloseData);
                g_lightCloseData = NULL;
            }            
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_OPEN_SENTINE_MODE:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[SentineOpen] = 0;

            if(NULL != g_sintineOpenData)
            {
                if(0 == pResult[1])
                {
                    retValue = SendRemoteCmdResult(g_sintineOpenData,
                                                    (gchar *)g_sintineOpenData->payloadAndCheck,
                                                    6,
                                                    0x6D92);
                }
                else
                {
                    resultDataBuff[0] = 0x22;
                    resultDataBuff[1] = pResult[1] << 7;
                    retValue = SendRemoteCmdResult(g_sintineOpenData,
                                                    (gchar *)resultDataBuff,
                                                    4,
                                                    0xFFFF);
                }
                g_free(g_sintineOpenData);
                g_sintineOpenData = NULL;
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
		case MSG_SUBCMD_CONTROL_CLOSE_SENTINE_MODE:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[SentineClose] = 0;

            if(NULL != g_sintineCloseData)
            {
                if(0 == pResult[1])
                {
                    retValue = SendRemoteCmdResult(g_sintineCloseData,
                                                    (gchar *)g_sintineCloseData->payloadAndCheck,
                                                    6,
                                                    0x6D92);
                }
                else
                {
                    resultDataBuff[0] = 0x22;
                    resultDataBuff[1] = pResult[1] << 6;
                    retValue = SendRemoteCmdResult(g_sintineCloseData,
                                                    (gchar *)resultDataBuff,
                                                    4,
                                                    0xFFFF);
                }
                g_free(g_sintineCloseData);
                g_sintineCloseData = NULL;
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
            
        case MSG_SUBCMD_CONTROL_WINDOWS:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[ControlWindowsE] = 0;
            if(NULL != g_controlWindows)
            {
                if(0 == pResult[1])
                {
                    retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_WINDOWS,
                                                        g_controlWindows,
                                                        resultDataBuff,
                                                        6,
                                                        0);
                }
                else
                {
                    if(2 != pResult[1] && 4 != pResult[1])
                    {
                        resultDataBuff[0] = 0x00;
                        resultDataBuff[1] = 0x00;
                    }
                    retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_WINDOWS,
                                                        g_controlWindows,
                                                        resultDataBuff,
                                                        6,
                                                        3);
                }
                g_free(g_controlWindows);
                g_controlWindows = NULL;
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_UNLOCK_THE_CAR:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[UnlockTheCarE] = 0;
            if(NULL != g_unlockTheCar)
            {
                if(0 == pResult[1])
                {
                    retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_UNLOCK_THE_CAR,
                                                        g_unlockTheCar,
                                                        resultDataBuff,
                                                        6,
                                                        0);
                }
                else
                {
                    if(2 != pResult[1] && 4 != pResult[1])
                    {
                        resultDataBuff[0] = 0x00;
                        resultDataBuff[1] = 0x00;
                    }
                    retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_UNLOCK_THE_CAR,
                                                        g_unlockTheCar,
                                                        resultDataBuff,
                                                        6,
                                                        3);
                }
                g_free(g_controlWindows);
                g_controlWindows = NULL;
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_REMOTE_PARKING:
            pthread_mutex_lock(&g_controlLock);
            if(1 == g_remoteParkingFlag)
            {
                g_remoteParkingFlag = 0;
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_REMOTE_PARKING_STA:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[RemoteParkStaE] = 0;
            if(NULL != g_remoteParkingStatus)
            {
                memset(&resultDataBuff, 0, 6);
                if(0 == pResult[1])
                {
                    resultDataBuff[0] = 0x03;
                    resultDataBuff[1] = 0x19;
                    resultDataBuff[2] = (pResult[2] >> 4) & 0x0F;
                    resultDataBuff[3] = pResult[2]& 0x0F;
                    resultDataBuff[4] = pResult[3];
                    SendRemoteCmdResult(g_remoteParkingStatus, (gchar *)resultDataBuff, 6, 0x6D92);
                }
                else
                {
                    resultDataBuff[0] = 0x2D;
                    SendRemoteCmdResult(g_remoteParkingStatus, (gchar *)resultDataBuff, 4, 0xFFFF);
                }
                g_free(g_remoteParkingStatus);
                g_remoteParkingStatus = NULL;
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_OTA_WAKE_UP:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[OtaWakeupE] = 0;
            if(0 != pResult[1] && otaWakeupFlag < 3)
            {
                otaWakeupFlag++;
                ControlOTAWakeUp();
            }
            else
            {
                otaWakeupFlag = 0;
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_CHARGE_LIMIT:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[ChargeLimitE] = 0;
            if(NULL != g_chargeLimit)
            {
                memset(&resultDataBuff, 0, 6);
                retValue = SendDD22ControlResultData(MSG_SUBCMD_CONTROL_CHARGE_LIMIT,
                                                    g_chargeLimit,
                                                    resultDataBuff,
                                                    6,
                                                    pResult[1]);
                g_free(g_chargeLimit);
                g_chargeLimit = NULL;                
            }
            pthread_mutex_unlock(&g_controlLock);
            break;

        case MSG_SUBCMD_CONTROL_REMOTE_RESET:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[RemoteReset] = 0;
            if(NULL != g_remotereset)
            {
                if (pResult[1] == 0)
                {
                    //通过
                    memset(&resultDataBuff, 0, 6);
                    retValue = SendRemoteControlResetAck(*g_remotereset,1); //肯定回答
                    g_free(g_remotereset);
                    g_remotereset = NULL;
                    sleep(1);
                    LOG_Debug("reboot!\n");
                    system("reboot");               
                }
                else
                {
                    LOG_Debug("remote reboot fail !!!\n");
                    PmCleanSleepEvent(PM_SLEEP_EVENT_BUSYING);
                    //不通过
                    memset(&resultDataBuff, 0, 6);
                    retValue = SendRemoteControlResetAck(*g_remotereset,0); //否定回答
                    g_free(g_remotereset);
                    g_remotereset = NULL; 
                }
            }
           
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_OPEN_SEAT_HEAT:
        case MSG_SUBCMD_CONTROL_CLOSE_SEAT_HEAT:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[RemoteSeatHeat] = 0;
            if(NULL != g_remoteSeatHeat)
            {
                SendRemoteControlSeatResult(*g_remoteSeatHeat, pResult[1], (char*)pResult, resultLen);
            }
            pthread_mutex_unlock(&g_controlLock);
            break;
        case MSG_SUBCMD_CONTROL_OPEN_SEAT_VENTILATE:
        case MSG_SUBCMD_CONTROL_CLOSE_SEAT_VENTILATE:
            pthread_mutex_lock(&g_controlLock);
            g_timeOutArray[RemoteSeatVentilate] = 0;
            if(NULL != g_remoteSeatVentilate)
            {
                SendRemoteControlSeatResult(*g_remoteSeatVentilate, pResult[1], (char*)pResult, resultLen);
            }
            pthread_mutex_unlock(&g_controlLock);            
            break;
        default:
            break;
    }

    return retValue;
}

static gint ParserBLEKeyControlAckData(guint8 subcmd, unsigned char *pResult, guint16 resultLen)
{
    gint retValue = RET_ERR;
    guchar resultDataBuff[4] = {0};
    gint resultDataLen = 4;
    gint retCmd = 0;

    //LOG_Debug("%d", __LINE__);
    if(NULL == pResult || resultLen < 0)
    {
        return retValue;
    }
    
    if(subcmd == MSG_SUBCMD_CONTROL_BLE_MAC)
    {
        pthread_mutex_lock(&g_controlLock);
        if(resultLen >= 6)
        {
            memcpy(g_bleMac, pResult, 6);
            SvaeBLeMac(g_bleMac, 6);
            LOG_Debug("g_bleMac %x:%x:%x:%x:%x:%x", 
                    pResult[0], pResult[1], pResult[2], pResult[3], pResult[4], pResult[5]);
        }
        pthread_mutex_unlock(&g_controlLock);
        return RET_OK;
    }

    LOG_Debug("Control RetValue: cmd :%x, status:%x  (0x01 :succeed)", 
                    subcmd, pResult[0]);

    pthread_mutex_lock(&g_controlLock);
    g_timeOutArray[RemoteBLEKeyCrl] = 0;

    retCmd = MSG_SBUCMD_CONTROL_BLE_NOT_FIND;

    switch(subcmd)
    {
        case MSG_SUBCMD_CONTROL_CREATE_BLE_KEY:                 //  (0x01)    //新建蓝牙钥匙
            if(1 == pResult[1] && 2 == pResult[0])
            {
                retCmd = MSG_SBUCMD_CONTROL_BLE_KEY_MAC;
            }
        case MSG_SUBCMD_CONTROL_UPDATE_BLE_SECRET_KEY:          //  (0x02)    //更新蓝牙钥匙密钥  
        case MSG_SUBCMD_CONTROL_UPDATE_BLE_KEY_PERMISSION:      //  (0x03)    //更新蓝牙钥匙权限   
        case MSG_SUBCMD_CONTROL_DELETE_SPECIFIED_BLE_KEY:       //  (0x04)    //删除指定蓝牙钥匙
        case MSG_SUBCMD_CONTROL_DELECT_BLE_KEY:                 //  (0x06)    //删除所有蓝牙钥匙
            if(NULL == g_remoteBLEKeyData)
            {
                break;
            }
            if(1 == pResult[0])
            {
                retValue = SendBLEKeyControlResultData(MSG_SBUCMD_CONTROL_BLE_NOT_FIND,
                                                    g_remoteBLEKeyData, 
                                                    resultDataBuff, 
                                                    resultDataLen, 
                                                    0);
            }            
            else
            {
                SendBLEKeyControlResultData(retCmd, g_remoteBLEKeyData, pResult, 4, 1);
            }
            break;
        case MSG_SUBCMD_CONTROL_SELECT_BLE_KEY:             //  (0x05)    //查询（指定或者）全部蓝牙钥匙
            SendSelectBLEKeyResultData(g_remoteBLEKeyData, pResult, resultLen);
            break;
        default:
            break;
    }

    g_free(g_remoteBLEKeyData);
    g_remoteBLEKeyData = NULL;
    pthread_mutex_unlock(&g_controlLock);

    return retValue;
}

static char FindTimeOutCmd(gint cmd)
{
    char retValue = 0;
    switch(cmd)
    {
        case RemoteHeatE:
            retValue = MSG_SUBCMD_CONTROL_CHARGE_HEAT;
            break;
        case SpeedRemoteE:
            retValue = MSG_SUBCMD_CONTROL_REMOTE_SPEED_LIMIT;
            break;
        case ParkingHeatE:
            retValue = MSG_SUBCMD_CONTROL_PARKING_HEAT;
            break;
        case SearchCarE:
            retValue = MSG_SUBCMD_CONTROL_FLASH_WHISTLE;
            break;
        case ReserverCharE:
            retValue = MSG_SUBCMD_CONTROL_ODER_CHARGING;
            break;
        case DoorLockE:
            retValue = MSG_SUBCMD_CONTROL_CLOSE_DOOR;
            break;
        case DoorUnLockE:
            retValue = MSG_SUBCMD_CONTROL_OPEN_DOOR;
            break;
        case CarPreBook:
            retValue = MSG_SUBCMD_CONTROL_SHARED_CAR_PREBOOK;
            break;
        case CarPreBookCancel:
            retValue = MSG_SUBCMD_CONTROL_SHARED_CAR_PREBOOK_CANCEL;
            break;
        case CarReturn:
            retValue = MSG_SUBCMD_CONTROL_SHARED_CAR_RETURN;
            break;
        case WindowClose:
            retValue = MSG_SUBCMD_CONTROL_CLOSE_WINDOW;
            break;
        case WindowOpen:
            retValue = MSG_SUBCMD_CONTROL_OPEN_WINDOW;
            break;
        case LightOpen:
            retValue = MSG_SUBCMD_CONTROL_OPEN_LIGHTS;
            break;
        case LightClose:
            retValue = MSG_SUBCMD_CONTROL_CLOSE_LIGHTS;
            break;
        case PowerUp:
            retValue = MSG_SUBCMD_CONTROL_START_ENGINE;
            break;
        case PowerDown:
            retValue = MSG_SUBCMD_CONTROL_TURNOFF_ENGINE;
            break;
        case OtaPowerUp:
            retValue = MSG_SUBCMD_CONTROL_START_ENGINE;
            break;
        case AcOff:
            retValue = MSG_SUBCMD_CONTROL_CLOSE_AC;
            break;
        case AcOn:
            //retValue = MSG_SUBCMD_CONTROL_OPEN_AC;
            retValue = MSG_SUBCMD_CONTROL_OPEN_AC_OLD;
            break;
        case RemoteBLEEnable:
            // retValue = MSG_SUBCMD_CONTROL_SHARED_CAR_RETURN;
            retValue = MSG_SUBCMD_CONTROL_REMOTE_AUTHORIZED_IGNITION;
            break;
        case RemoteBLEKeyCrl:
            retValue = MSG_SBUCMD_CONTROL_BLE_NOT_FIND;
            break;
        case SentineOpen:
            retValue = MSG_SUBCMD_CONTROL_OPEN_SENTINE_MODE;
            break;
        case SentineClose:
            retValue = MSG_SUBCMD_CONTROL_CLOSE_SENTINE_MODE;
            break;
        case ControlWindowsE:
            retValue = MSG_SUBCMD_CONTROL_WINDOWS;
            break;
        case RemoteParkingE:
            retValue = MSG_SUBCMD_CONTROL_REMOTE_PARKING;
            break;
        case UnlockTheCarE:
            retValue = MSG_SUBCMD_CONTROL_UNLOCK_THE_CAR;
            break;
        case ChargeLimitE:
            retValue = MSG_SUBCMD_CONTROL_CHARGE_LIMIT;
            break;    
        default:
            retValue = 0;
            break;
    }

    return retValue;
}

gint ControlCmdTimeOut()
{
    guchar resultData[128] = {0};
    char retValue = 0;
    gint count = 0;
    guint8 subcmd = 0;
    gint resultDataLen = 0;
    

    resultData[1] = 0x01;
    pthread_mutex_lock(&g_controlLock);

    for(count = 0; count < MaxIndexE; count++)
    {
        if(1 == g_timeOutArray[count])
            break;
    }
    retValue = FindTimeOutCmd(count);
    if(retValue > 0)
    {
        resultData[0] = retValue;
        g_timeOutArray[count] = 0;
        LOG_Debug("timeOut cmd :%x", retValue);
        if(retValue == MSG_SBUCMD_CONTROL_BLE_NOT_FIND)   //蓝牙近场通信超时
        {   
            resultData[0] = 0x02;  //失败
            printf("%d", __LINE__);
            if(NULL != g_remoteBLEKeyData)
            {
                printf("%d", __LINE__);
                switch(g_remoteBLEKeyData->subfunction)
                {
                    case 0x1100:
                        subcmd = MSG_SUBCMD_CONTROL_CREATE_BLE_KEY;
                        break;
                    case 0x1200:
                        subcmd = MSG_SUBCMD_CONTROL_UPDATE_BLE_SECRET_KEY;
                        break;
                    case 0x1300:
                        subcmd = MSG_SUBCMD_CONTROL_UPDATE_BLE_KEY_PERMISSION;
                        break;
                    case 0x1400:
                        subcmd = MSG_SUBCMD_CONTROL_DELETE_SPECIFIED_BLE_KEY;
                        break;
                    case 0x1500:
                        subcmd = MSG_SUBCMD_CONTROL_DELECT_BLE_KEY;
                        break;
                    case 0x1600:
                        subcmd = MSG_SUBCMD_CONTROL_SELECT_BLE_KEY;
                        resultData[0] = 0x00;  //失败
                        break;
                    default:
                         break;
                }      
            }
            pthread_mutex_unlock(&g_controlLock);
            ParserBLEKeyControlAckData(subcmd, resultData, 4);
            return RET_OK;
        }
        else
        {
            pthread_mutex_unlock(&g_controlLock);
            ParserControlAckData(resultData, 4);
            return RET_OK;
        }
        g_timeOutArray[count] = 0;
        LOG_Debug("timeOut ble key cmd :%x", subcmd);
        pthread_mutex_unlock(&g_controlLock);
        // ParserBLEKeyControlAckData(subcmd, resultData, 4);
        return RET_OK;
    }
    
    
    for(count = 0; count < MaxIndexE; count++)
    {
        if(g_timeOutArray[count] > 0)
            g_timeOutArray[count]--;
    }
    pthread_mutex_unlock(&g_controlLock);
    usleep(990000);

    return RET_OK;
}



gint SendRemoteControlResult(IotDataFormat *remoteControlData,guchar result,guint8 status)
{
    IotDataFormat *logouttData = NULL;
    guint16 craData = 0;
    guint16 payloadLength = 0;
    guint32 logintDataLen = 0;
    gint errReturn;
    logouttData = (IotDataFormat *)g_try_malloc0(sizeof(IotDataFormat) + 64);
    LOG_Debug("get status : %d\n",status);
    if(NULL == logouttData)
    {
        LOG_Error("logintData malloc error");
        return RET_ERR;
    }

    logouttData->headerVersion = remoteControlData->headerVersion;
    logouttData->flag = remoteControlData->flag;
    logouttData->pkgLength = remoteControlData->pkgLength;
    logouttData->eventID = remoteControlData->eventID;
    logouttData->randomData = remoteControlData->randomData;
    if(result == 0)
    {
        logouttData->serviceID = 0x6D92;
        logouttData->subfunction = 0x0100;
        logouttData->payloadLength = 0x0800;
        payloadLength = 8;
        memcpy(logouttData->payloadAndCheck,remoteControlData->payloadAndCheck,0x08);
        logouttData->payloadAndCheck[5] = 0x00;

    }
    else if(result == 1)
    {
        logouttData->serviceID = 0x6D92;
        logouttData->subfunction = 0x0100;
        logouttData->payloadLength = 0x0800;
        payloadLength = 8;
        memcpy(logouttData->payloadAndCheck,remoteControlData->payloadAndCheck,0x08);
        logouttData->payloadAndCheck[5] = 0x01;
	//logouttData->payloadAndCheck[5] = status;
    }
    else 
    {
        logouttData->serviceID = 0xFFFF;
        logouttData->subfunction = 0xDD22;
        logouttData->payloadLength = 0x0600;
        payloadLength = 6;
        if(0x02 == remoteControlData->payloadAndCheck[0] && 0x03 == remoteControlData->payloadAndCheck[1])
        {
            logouttData->payloadAndCheck[0] = 0x25;
           // if(0x00 == remoteControlData->payloadAndCheck[2])
           // {
           //     logouttData->payloadAndCheck[1] = 0x05;
           // }
           // else
           // {
           //     logouttData->payloadAndCheck[1] = 0x06;
           // }
	        logouttData->payloadAndCheck[1] = status;
            logouttData->payloadAndCheck[2] = 0x00;
            logouttData->payloadAndCheck[3] = 0x00;
            
        }
        else if(0x01 == remoteControlData->payloadAndCheck[0] && 0x12 == remoteControlData->payloadAndCheck[1])
        {
            logouttData->payloadAndCheck[0] = 0x26;
            logouttData->payloadAndCheck[1] = 0x00;
            logouttData->payloadAndCheck[2] = 0x00;
            logouttData->payloadAndCheck[3] = 0x00;
        }
    }

    logintDataLen = htons(logouttData->pkgLength);
    craData = CalCrcCal((guint8 *)logouttData,logintDataLen + payloadLength - 2);
    logouttData->payloadAndCheck[payloadLength - 2] = craData / 256;
    logouttData->payloadAndCheck[payloadLength - 1] = craData % 256;
    //LOG_Debug("send data here : %d\n",logouttData->payloadAndCheck[1]);
    //return SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),3);
    errReturn = SendDiagnosisData((gpointer)logouttData,(payloadLength + htons(logouttData->pkgLength)),3);
    g_free(logouttData);
    return errReturn;
}


gint SendRemoteControlDataToMcu(guchar controlCmd ,guchar *controlDataBuff,gshort dataLen)
{
    gint count = 0;
    if(NULL == controlDataBuff)
    {
        return RET_ERR;
    }
    LOG_Debug("Send Control cmd: %x", controlCmd);
    for(count = 0; count < dataLen; count++)
    {
        LOG_Debug("Control Service Data[%d]: %x", count, controlDataBuff[count]);
    }
    
    LOG_Debug("MSG_CONTROL : %8x", MSG_CONTROL);
    McuMesg_t sendToMcu;
    sendToMcu.type = (SYSQueMsgType_e)(MSG_CONTROL | controlCmd);
    sendToMcu.payloadLen = dataLen;
    sendToMcu.payload = controlDataBuff;

    LOG_Debug("sendToMcu.type : %8x", sendToMcu.type);
    printf("sendToMcu.type : %8x\n", sendToMcu.type);
    return McuSrvSendMesg(&sendToMcu);
}

gint SendRemoteControlBLEKeyDataToMcu(guchar controlCmd ,guchar *controlDataBuff,gshort dataLen)
{
    gint count = 0;
    if(NULL == controlDataBuff)
    {
        return RET_ERR;
    }
    LOG_Debug("Send Control cmd: %x", controlCmd);
    for(count = 0; count < dataLen; count++)
    {
        LOG_Debug("Control Service Data[%d]: %x", count, controlDataBuff[count]);
    }
    
    McuMesg_t sendToMcu;
    sendToMcu.type = (SYSQueMsgType_e)(MSG_BLE_CONTROL_REQ | controlCmd);
    sendToMcu.payloadLen = dataLen;
    sendToMcu.payload = controlDataBuff;

    return McuSrvSendMesg(&sendToMcu);
}

void RemoteControlMcuHandleCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    if((NULL == recvData) && (0 == dataLen) && (NULL == userdata))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    LOG_Debug("%s, recv DataLen:%d, type:%08x", __FUNCTION__, dataLen, userdata);
    LOG_DebugHex(recvData, dataLen, "RemoteControl MCU Data");

    gint ret = 0;
    guchar count = 0;
    guchar oldStatus = 0;
    uint32_t aidMid = ((uint64_t)userdata & 0xffffffffffffff00);
    uint8_t subcmd = ((uint64_t)userdata & 0x00000000000000ff);
    LOG_Debug("enter get pther Success aidMid = %08x, subcmd = %02x\n", aidMid, subcmd);

    switch (aidMid)
    {
        case MSG_CONTROL:
            if(MSG_SUBCMD_CONTROL_RESULT == subcmd)
            {
            //LOG_Debug("get data and send Msg Success\n");
                if(MSG_SUBCMD_CONTROL_REMOTE_CHARGING == recvData[0])
                {
                    g_controlFlag &= 0xFC;
                    g_controlFlag |= 0x02;
                    LOG_Debug("智能补电指令成功");
                } 
                else 
                {
                    ParserControlAckData(const_cast<unsigned char*>(recvData), dataLen);
                }
            }
        break;
        case MSG_CONTROL_CYCLE_ACK:
            if(MSG_SBUCMD_CONTROL_REMOTE_MEMORY_PARKING == subcmd)
            {
                pthread_mutex_lock(&g_controlLock);
                LOG_Debug("subcmd : %02x", subcmd);
                if(MSG_SUBCMD_CONTROL_REMOTE_PARKING == recvData[0] && NULL != g_remoteParking)
                {
                    SendRemoteControlStatus(*g_remoteParking, recvData[2], 0x1700);
                    if(0x0B == recvData[1] || 0x0D == recvData[1] || \
                       0x07 == recvData[1] || 0x03 == recvData[1])
                    {
                        g_remoteParkingFlag = 0;
                    }
                }
                pthread_mutex_unlock(&g_controlLock);
                break;
            }
            if(MSG_SBUCMD_CONTROL_UNLOCK_CAR == subcmd)
            {
                pthread_mutex_lock(&g_controlLock);
                LOG_Debug("subcmd : %02x", subcmd);
                if(MSG_SUBCMD_CONTROL_UNLOCK_THE_CAR == recvData[0] && NULL != g_unlockTheCar)
                {
                    SendRemoteControlStatus(*g_unlockTheCar, recvData[1], 0x1800);
                    g_free(g_unlockTheCar);
                    g_unlockTheCar = NULL;
                }
                pthread_mutex_unlock(&g_controlLock);
                break;
            }
            if(0x02 == (g_controlFlag & 0x03))
            {
                count++;
                if((recvData[0] == 0x0E && oldStatus != recvData[1]))
                {
                    count = 0;
                    oldStatus = recvData[1];
                    SendRemoteControlStatus(remoteControlData,oldStatus, 0x0800);
                    LOG_Debug("智能补电状态更新");
                }
                if(oldStatus >= 5)
                {
                    g_controlFlag &= 0xFC;
                }
            }
            else if(0x01 == (g_controlFlag & 0x03))
            {
                count++;
                if(oldStatus != recvData[1] && recvData[0] == 0x0E)
                {
                    oldStatus = recvData[1];
                    SendRemoteControlStatus(remoteControlData,oldStatus, 0x0800);
                    LOG_Debug("智能补电状态更新");
                }
                if(15 >= count || oldStatus >= 5)
                {
                    g_controlFlag &= 0xFC;
                    count = 0;
                }
            }
            else
            {
                count = 0;
            }
            LOG_Debug("recvData:[%02x][%02x]", recvData[0], recvData[1]);
            break;
        case MSG_BLE_CONTROL_ACK:
            ParserBLEKeyControlAckData(subcmd, const_cast<unsigned char*>(recvData), dataLen);
            break;
        case MSG_MCU_SYNC_POWER:
            if(MSG_SUBCMD_RESERVATION_CHARGING_MCU_SYNC_TO_CPU == subcmd)
            {
                guchar response = 0x01;
                pthread_mutex_lock(&g_controlLock);
                g_ReserverChargingTime.mcuSetState = 0;
                LOG_DebugHex(recvData, dataLen, "local reservation charging");
                if(8 <= dataLen)
                {
                    g_ReserverChargingTime.version = 2;
                    g_ReserverChargingTime.flag = recvData[0];
                    g_ReserverChargingTime.appointmentType = recvData[1];
                    g_ReserverChargingTime.startTimeH = recvData[3];
                    g_ReserverChargingTime.startTimeM = recvData[4];
                    g_ReserverChargingTime.endTimeH = recvData[6];
                    g_ReserverChargingTime.endTimeM = recvData[7];
                    //车机预约充电中使能位，0代表无请求，1代表关闭预约充电，2代表开启预约充电
                    if(2 == g_ReserverChargingTime.flag)
                    {
                        g_ReserverChargingTime.flag = 1;
                    }
                    else
                    {
                        memset(&g_ReserverChargingTime,0,sizeof(g_ReserverChargingTime));
                    }
                    time_t now = time(NULL);
                    tm* local_time = localtime(&now);
                    int nowMinute =local_time->tm_hour * 60 + local_time->tm_min;
                    int reserverChargingTimeStartM = g_ReserverChargingTime.startTimeH * 60 + g_ReserverChargingTime.startTimeM;
                    int reserverChargingTimeEndM = g_ReserverChargingTime.endTimeH * 60 + g_ReserverChargingTime.endTimeM;
                    LOG_Debug("now:%d start:%d end:%d",nowMinute,reserverChargingTimeStartM,reserverChargingTimeEndM);


                    local_time->tm_hour = 0;
                    local_time->tm_min = 0;
                    local_time->tm_sec = 0;
                    g_ReserverChargingTime.singleStartTime = mktime(local_time) + reserverChargingTimeStartM * 60;
        
                    if(1 == g_ReserverChargingTime.appointmentType && reserverChargingTimeStartM < nowMinute)
                    {
                        // 如果预约开始时间小于现在时间，单次预约为次日，否则为当日；仅单次预约使用
                        g_ReserverChargingTime.singleStartTime += 86400;
                    }

                    g_ReserverChargingTime.sendFlag = 0;
                    g_ReserverChargingTime.sendFlagTmp = 0; 
                    response = 0;
                }
                pthread_mutex_unlock(&g_controlLock);
                SaveOderChargingTimeToFile(ODER_CHARGING_INFO, strlen(ODER_CHARGING_INFO));

                

                McuMesg_t sendToMcu;
                sendToMcu.type = (SYSQueMsgType_e)SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_CPU_RESP_TO_MCU;
                sendToMcu.payloadLen = sizeof(response);
                sendToMcu.payload = &response;
                McuSrvSendMesg(&sendToMcu);
                
                pthread_mutex_lock(&g_controlLock);
                g_ReserverChargingTime.mcuSetState = 1;
                pthread_mutex_unlock(&g_controlLock);
            }
            else if(MSG_SUBCMD_RESERVATION_CHARGING_CONFIRM == subcmd)
            {
                LOG_DebugHex(recvData, dataLen, "confirm reservation charging");
                guchar response = g_ReserverChargingTime.flag;
                McuMesg_t sendToMcu;
                sendToMcu.type = (SYSQueMsgType_e)SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_CONFIRM;
                sendToMcu.payloadLen = sizeof(response);
                sendToMcu.payload = &response;
                McuSrvSendMesg(&sendToMcu);
            }        
            break;
        case MSG_BLE_LOG:
            if(MSG_SUBCMD_BLE_LOG_Stream == subcmd)
            {
                LOG_DebugHex(recvData, dataLen, "ble_mcu_log");
            }
            else
            {
                LOG_Debug("ble_mcu_log:%s", recvData);
            }
            break;
        case MSG_MCU_LOG:
            if(MSG_SUBCMD_MCU_LOG_CAN == subcmd)
            {
                LOG_Debug("can_mcu_log:%s", recvData);
            }
            else if(MSG_SUBCMD_MCU_LOG_TSP == subcmd)
            {
                LOG_Debug("tsp_mcu_log:%s", recvData);
            }
            else if(MSG_SUBCMD_MCU_LOG_PM == subcmd)
            {
                LOG_Debug("mcu_log:%s", recvData);
            }
            else if(MSG_SUBCMD_MCU_LOG_RDM == subcmd)
            {
                LOG_Debug("rdm_mcu_log:%s", recvData);
            }
            break;
        case MSG_MCU_Debug_LOG:
            if(MSG_SUBCMD_MCU_Debug_LOG == subcmd)
            {
                LOG_Debug("debug_mcu_log:%s", recvData);
            }
            break;
        default:
            break;
        }
}

static void TspRpcCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    SgmwSrvRpcReq_t *req = (SgmwSrvRpcReq_t *)reqData;
    SgmwSrvRpcRes_t *res = (SgmwSrvRpcRes_t *)respData;
    LOG_DebugHex(reqData, reqLen, "TspRpcCallBack");
    gint count = 15;
    char powerOn = 1;
    char powerOff = 1;
    char retValue = 0x00;
    char flag = 0x00;
    switch (subcmd)
    {
    case RPC_SERVER_ABUP_SET_REMOTE_RESP:
        res->result = 0;
        memcpy(res->data, req->data, reqLen);
        *respLen = reqLen;
        return;
    case RPC_SERVER_ABUP_REMOTE_CTRL:
        if (0x01 == req->data[0]) //远程上电
        {
            flag = 1;
            ControlRemotePowerUp(NULL, 0);
        }
        else if (0x02 == req->data[0]) //远程下电
        {
            flag = 2;
            ControlRemotePowerDown(NULL, 0);
        }
        break;
    case RPC_SERVER_ABUP_CAROTA_HVUP_CTRL:
        ControlOTAPowerUp(NULL, 0);
        flag = 1;
        break;
    case RPC_SERVER_ABUP_CAROTA_HVDOWN_CTRL:
        ControlRemotePowerDown(NULL, 0);
        flag = 2;
        break;    
    default:
        return;
    }


    while(count > 0)
    {
        GetRdmRemoteCtrolRet(&powerOn, &powerOff);
        LOG_Debug("powerOn:  powerOff:  %02X,%02X", powerOn, powerOff);
        if((0 == powerOn || 0xC0 == powerOn) && 1 == flag)
        {
            res->result = RET_OK;
            res->data[0] = 0x01;
            res->data[1] = (uint8_t)powerOn;
            res->data[2] = 1;
            res->dataLen = sizeof(3 * sizeof(int));
            *respLen = (res->dataLen + 2); 
            break;
        }

        if((0 == powerOff || 0xC0 == powerOff) && 2== flag)
        {
            res->result = RET_OK;
            res->data[0] = 0x02;
            res->data[1] = (uint8_t)powerOff;
            res->data[2] = 1;
            res->dataLen = sizeof(3 * sizeof(int));
            *respLen = (res->dataLen + 2); 
            break;
        }
        count--;
        sleep(1);
    }

}

// static void CarotaHvCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
// {
//     gint count = 15;
//     char powerOn = 1;
//     char powerOff = 1;
//     char retValue = 0x00;
//     char flag = 0x00;

//     //掉远控接口实现科络达本地上下电功能
//     if(subcmd == RPC_SERVER_ABUP_CAROTA_HVUP_CTRL)
//     {
//         ControlOTAPowerUp(NULL,0); 
//         flag = 1;  
//     }
//     else if(subcmd == RPC_SERVER_ABUP_CAROTA_HVDOWN_CTRL)
//     {
        
//         ControlRemotePowerDown(NULL,0);
//         flag = 2;
//     }

//     while(count > 0)
//     {
//         GetRdmRemoteCtrolRet(&powerOn, &powerOff);
//         LOG_Debug("powerOn:  powerOff:  %02X,%02X", powerOn, powerOff);
//         if((0 == powerOn || 0xC0 == powerOn) && 1 == flag)
//         {
//             *respData = (uint8_t)powerOn;
//             *respLen = sizeof(uint8_t); 
//             break;
//         }

//         if((0 == powerOff || 0xC0 == powerOff) && 2== flag)
//         {
//             *respData = (uint8_t)powerOn;
//             *respLen = sizeof(uint8_t);
//             break;
//         }
//         count--;
//         sleep(1);
//     }

// }

int32_t InitSignalIpcCallback()
{

    RpcStartMsgServer((RPCServerType_e)RPC_QUE_TYPE_SGMW_APP_REMOTE_CTRL, TspRpcCallBack,1, NULL);
    //RpcStartMsgServer((RPCServerType_e)RPC_QUE_TYPE_SGMW_CAROTA_HV_CTRL, CarotaHvCallBack,1, NULL);
    LOG_Debug("111111111111111RpcStartMsgServer");

    LOG_Debug("%s SubscriptionQueueMsg from 0x0002036E to 0x000203F3", __FUNCTION__);
    LOG_Debug("%s SubscriptionQueueMsg 0x%08x", __FUNCTION__, MSG_CONTROL);
    LOG_Debug("%s SubscriptionQueueMsg 0x%08x", __FUNCTION__, MSG_SUBCMD_CONTROL_MCU_ACK);
    LOG_Debug("%s SubscriptionQueueMsg 0x%08x", __FUNCTION__, (SYSQueMsgType_e)(MSG_CONTROL | MSG_SUBCMD_CONTROL_MCU_ACK));
    //control订阅，from 0x0002036E to 0x000203F3
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_CONTROL | MSG_SUBCMD_CONTROL_MCU_ACK), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SGMW_ACK));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_CONTROL | MSG_SUBCMD_CONTROL_RESULT), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SGMW_RES));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_CONTROL_CYCLE_ACK | MSG_SBUCMD_CONTROL_REMOTE_POWER_SUPPLY), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_POWER_SUPPLY));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_CONTROL_CYCLE_ACK | MSG_SBUCMD_CONTROL_REMOTE_MEMORY_PARKING), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_MEMORY_PARKING));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_CONTROL_CYCLE_ACK | MSG_SBUCMD_CONTROL_UNLOCK_CAR), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_UNLOCK_CAR));

    LOG_Debug("%s SubscriptionQueueMsg from 0x00060201 to 0x00060207", __FUNCTION__);
    //ble订阅，from 0x00064201 to 0x00064207
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_BLE_CONTROL_ACK | MSG_SUBCMD_CONTROL_CREATE_BLE_KEY), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_CREATE_BLE_KEY_ACK));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_BLE_CONTROL_ACK | MSG_SUBCMD_CONTROL_UPDATE_BLE_SECRET_KEY), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_UPDATE_BLE_SECRET_KEY_ACK));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_BLE_CONTROL_ACK | MSG_SUBCMD_CONTROL_UPDATE_BLE_KEY_PERMISSION), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_UPDATE_BLE_KEY_PERMISSION_ACK));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_BLE_CONTROL_ACK | MSG_SUBCMD_CONTROL_DELETE_SPECIFIED_BLE_KEY), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_DELETE_SPECIFIED_BLE_KEY_ACK));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_BLE_CONTROL_ACK | MSG_SUBCMD_CONTROL_SELECT_BLE_KEY), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_SELECT_BLE_KEY_ACK));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_BLE_CONTROL_ACK | MSG_SUBCMD_CONTROL_DELECT_BLE_KEY), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_DELECT_BLE_KEY_ACK));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_BLE_CONTROL_ACK | MSG_SUBCMD_CONTROL_BLE_MAC), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_BLE_MAC_ACK));

    LOG_Debug("%s SubscriptionQueueMsg from from 0x00010409 to 0x0001040a", __FUNCTION__);
    //电源状态订阅，from 0x00010409 to 0x0001040a
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_MCU_SYNC_POWER | MSG_SUBCMD_RESERVATION_CHARGING_MCU_SYNC_TO_CPU), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_MCU_SYNC_TO_CPU));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_MCU_SYNC_POWER | MSG_SUBCMD_RESERVATION_CHARGING_CPU_RESP_TO_MCU), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_CPU_RESP_TO_MCU));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_MCU_SYNC_POWER | MSG_SUBCMD_RESERVATION_CHARGING_CONFIRM), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_CONFIRM));

    //蓝牙日志，from 0x00000200 to 0x00000201
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_BLE_LOG | MSG_SUBCMD_BLE_LOG_Stream), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_BLE_LOG_Stream));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_BLE_LOG | MSG_SUBCMD_BLE_LOG_String), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_BLE_LOG_String));

    //蓝牙日志，from 0x00000001 to 0x00000004
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_MCU_LOG | MSG_SUBCMD_MCU_LOG_CAN), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_LOG_CAN));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_MCU_LOG | MSG_SUBCMD_MCU_LOG_TSP), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_LOG_TSP));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_MCU_LOG | MSG_SUBCMD_MCU_LOG_PM), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_LOG_PM));
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_MCU_LOG | MSG_SUBCMD_MCU_LOG_RDM), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_LOG_RDM));
    //蓝牙日志，from 0x00000100 to 0x00000100
    SubscriptionQueueMsg((SYSQueMsgType_e)(MSG_MCU_Debug_LOG | MSG_SUBCMD_MCU_Debug_LOG), RemoteControlMcuHandleCallback, (void *)(SYS_QUE_TYPE_MCU_DEBUG_LOG));
 
    return RET_OK;
}


