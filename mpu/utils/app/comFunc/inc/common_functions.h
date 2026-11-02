/**
 * @file common_functions.h
 * @author liulei (lei.liu@sirun.net)
 * @brief common_functions sdk层接口头文件
 * @version 0.1
 * @date 2024-07-16
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _COMMON_FUNCTIONS_H_
#define _COMMON_FUNCTIONS_H_

#include <stdint.h>
#include "sr_param.h"
#include "pm_server_api.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif

//自定义参数 sr_param.h
typedef enum
{
    E_ParamId_TspTokenUrl = 101,         /* TSP获取Token的平台地址 */
    E_ParamId_Configuration,
}SGMW_DeviceParamEnum;

//唤醒源 pm_server_api.h
#define SGMW_PM_WAKEUP_SOURCE_TIMER_560        (44)  // 9.5min定时唤醒

// 电源管理状态
#define SGMW_PM_STATE_INIT             (0)               // 电源管理初始状态
#define SGMW_PM_STATE_NORMAL           PM_STATE_NORMAL   // 电源管理正常状态
#define SGMW_PM_STATE_SLEEP            PM_STATE_SLEEP    // 电源管理休眠状态

// 网卡名称
#define  APN1_INTERFACE		"rmnet_data0"
#define  APN2_INTERFACE		"ccinet2"
#define  APN3_INTERFACE		"ccinet3"

// 升级路径，主模组带EMMC，在挂在成功的情况下通过链接文件链接到EMMC中
#define  OTA_UPGRADE_PATH_NAME          "/oemdata/upgrade/"
#define  MQTT_ADDR_SQLITE_INDEX         (101)
#define  CAR_NUM_SQLITE_INDEX           (102)

// IPC通信消息类型定义 queue_msg.h
typedef enum
{
    SYS_QUE_TYPE_INIT = 0x0000,
    /*内部通讯协议0x0001xxxx~0x003Fxxxx*/
    /*TBOX内部协议-mcu日志同步*/
    SYS_QUE_TYPE_MCU_LOG_CAN = 0x00004001,                      /*MCU的can日志-字符串,a00m40s01*/
    SYS_QUE_TYPE_MCU_LOG_TSP = 0x00004002,                      /*MCU的远控日志-字符串,a00m40s02*/
    SYS_QUE_TYPE_MCU_LOG_PM = 0x00004003,                      /*MCU的电源管理日志-字符串,a00m40s03*/
    SYS_QUE_TYPE_MCU_LOG_RDM = 0x00004004,                      /*MCU的远程诊断日志-字符串,a00m40s04*/
    SYS_QUE_TYPE_MCU_DEBUG_LOG = 0x00004100,                      /*MCU的DEBUG日志-字符串,a00m41s00*/
    SYS_QUE_TYPE_MCU_BLE_LOG_Stream = 0x00004200,                      /*蓝牙日志-字节流,a00m42s00*/
    SYS_QUE_TYPE_MCU_BLE_LOG_String = 0x00004201,                      /*蓝牙日志-字符串,a00m42s01*/
    SYS_QUE_TYPE_MPU_SYNC_MILEAGE_SYNC = 0x00012400,                /*MPU同步里程*/
    
    /*TBOX内部协议-A01状态同步*/
    SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_MCU_SYNC_TO_CPU = 0x00014000,          /*同步电源模式-MCU同步车机预约充电信息给CPU（event类型），a01m40s00*/
    SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_CPU_RESP_TO_MCU = 0x00014001,          /*同步电源模式-CPU回复同步车机预约充电信息成功，a01m40s01*/
    SYS_QUE_TYPE_MCU_SYNC_POWER_SYNC_RESERVATION_CHARGING_CONFIRM = 0x00014002,          /*同步电源模式-当MCU重启后，若发现自己处于预约充电状态，则向CPU请求确认，CPU回复是否有预约充电；，a01m40s02*/
    SYS_QUE_TYPE_MCU_SYNC_CAN_FRAME_SYNC = 0x00024200,                                  /*九识 同步CAN ID数据 a01m40s03(周期)*/
    
    /*TBOX内部协议-a02涉车服务*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_DOOR = 0x00024001,   /*车辆控制-开门,a02m40s01*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_DOOR = 0x00024002,   /*车辆控制-关门,a02m40s02*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_WINDOW = 0x00024003,   /*车辆控制-开窗,a02m40s03*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_WINDOW = 0x00024004,   /*车辆控制-关窗,a02m40s04*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_FLASH_WHISTLE = 0x00024005,   /*车辆控制-闪灯鸣笛,a02m40s05*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_AC = 0x00024006,   /*车辆控制-开启空调,a02m40s06*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_AC = 0x00024007,   /*车辆控制-关闭空调,a02m40s07*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_START_ENGINE = 0x00024008,   /*车辆控制-启动发动机,a02m40s08*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_TURNOFF_ENGINE = 0x00024009,   /*车辆控制-关闭发动机,a02m40s09*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_SKYLIGHT = 0x0002400A,   /*车辆控制-开启天窗,a02m40s0a*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_SKYLIGHT = 0x0002400B,   /*车辆控制-关闭天窗,a02m40s0b*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_TRUNK = 0x0002400C,   /*车辆控制-开启后备箱,a02m40s0c*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_TRUNK = 0x0002400D,   /*车辆控制-关闭后备箱,a02m40s0d*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_CHARGING = 0x0002400E,   /*车辆控制-智能补电,a02m40s0e*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_ODER_CHARGING = 0x0002400F,   /*车辆控制-预约充电,a02m40s0f*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_SPEED_LIMIT = 0x00024010,   /*车辆控制-远程限速,a02m40s10*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_PARKING_HEAT = 0x00024011,   /*车辆控制-驻车保温（电池加热）,a02m40s11*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CHARGE_HEAT = 0x00024012,   /*车辆控制-充电加热,a02m40s12*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_INTHE_CONTROL = 0x00024013,   /*车辆控制-中控解锁,a02m40s13*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SHARED_CAR_PREBOOK = 0x00024014,   /*车辆控制-车辆预约,a02m40s14*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SHARED_CAR_PREBOOK_CANCEL = 0x00024015,   /*车辆控制-取消车辆预约,a02m40s15*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SHARED_CAR_RETURN = 0x00024016,   /*车辆控制-还车,a02m40s16*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_LIGHTS  = 0x00024017,   /*车辆控制-开灯,a02m40s17*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_LIGHTS  = 0x00024018,   /*车辆控制-关灯,a02m40s18*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_AUTHORIZED_IGNITION = 0x00024019,   /*车辆控制-远程授权点火,a02m40s19*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_SENTINE_MODE = 0x00024020,   /*车辆控制-开启哨兵模式,a02m40s20*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_SENTINE_MODE = 0x00024021,   /*车辆控制-关闭哨兵模式,a02m40s21*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_WINDOWS = 0x00024022,   /*车辆控制-车窗控制(带开度),a02m40s22*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_PARKING = 0x00024023,   /*车辆控制-远程泊车,a02m40s23*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_UNLOCK_THE_CAR = 0x00024024,   /*车辆控制-下电锁车,a02m40s24*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_PARKING_STA = 0x00024025,   /*车辆控制-远程泊车状态查询,a02m40s25*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OTA_WAKE_UP = 0x00024026,   /*车辆控制-Ota唤醒(特例),a02m40s26*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CHARGE_LIMIT = 0x00024027,   /*车辆控制-充电电量限制(50-100),a02m40s27*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_RESET = 0x00024028,   /*车辆控制-远程重启,a02m40s28*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_SEAT_HEAT = 0x00024029,   /*车辆控制-远程打开座椅加热,a02m40s29*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_SEAT_HEAT = 0x00024030,   /*车辆控制-远程关闭座椅加热,a02m40s30*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_SEAT_VENTILATE = 0x00024031,   /*车辆控制-远程打开座椅通风,a02m40s31*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CLOSE_SEAT_VENTILATE = 0x00024032,   /*车辆控制-远程关闭座椅通风,a02m40s32*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_OPEN_AC_OLD = 0x00024033,   /*车辆控制-开启空调,a02m40s33*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SGMW_ACK = 0x0002406E,    /*车辆控制-MCU收到控制命令确认,a02m40s6E*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_SGMW_RES = 0x0002406F,    /*车辆控制-MCU控制命令结果,a02m40s6F*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_BLE_BUSY = 0x000240F1,   /*车辆控制-蓝牙钥匙繁忙,a02m40sf1*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_BLE_NOT_FIND = 0x000240F2,   /*车辆控制-找不到蓝牙钥匙,a02m40sf2*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_BLE_KEY_MAC = 0x000240F3,   /*车辆控制-蓝牙钥匙,a02m40sf3*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_GATEWAY_SIM1 = 0x00024044,   /*车辆控制-蓝牙钥匙,a02m40sf3*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_GATEWAY_SIM2 = 0x00024045,   /*车辆控制-蓝牙钥匙,a02m40sf3*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_FUNCTION_SWITCH = 0x00024046,   /*车辆控制-蓝牙钥匙,a02m40sf3*/
    SYS_QUE_TYPE_MCU_VEHICLE_EVENT_DATA = 0x00024100,   /*车身信息-事件上报,a02m41s00*/
    SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA = 0x00024102,   /*九识车身信息-事件上报,a02m41s02*/
    SYS_QUE_TYPE_MCU_RESET_RSON_DATA = 0x00012300,   /*九识重启原因信息上报*/
    SYS_QUE_TYPE_MCU_VEHICLE_CTROL_GATEWAY_REQ = 0x00310101,   /*九识控制网关*/
    SYS_QUE_TYPE_MCU_VEHICLE_CTROL_GATEWAY_RES = 0x00310102,   /*九识控制网关*/
    SYS_QUE_TYPE_MCU_VEHICLE_CTROL_SIMCARD_INFO_TO_MCU = 0x00310200,   /*九识SIM卡信息*/
    SYS_QUE_TYPE_MCU_VEHICLE_CTROL_SIMCARD_INFO_TO_MPU = 0x00310300,   /*九识SIM卡信息*/
    SYS_QUE_TYPE_MCU_VEHICLE_CTROL_SWITCH_POWERUP_TO_MPU = 0x00310401,   /*九识SWITCH上电通知*/
    SYS_QUE_TYPE_MCU_VEHICLE_CTROL_SWITCH_POWERDOWN_TO_MPU = 0x00310402,   /*九识SWITCH 下电通知*/
    SYS_QUE_TYPE_MCU_VEHICLE_CTROL_SWITCH_RESET_TO_MPU = 0x00310403,   /*九识SWITCH 复位通知*/
    SYS_QUE_TYPE_MCU_VEHICLE_CTROL_VERSION_SYNC_TO_MPU = 0x00310501,   /*九识MCU版本号及TBOX版本号同步*/
    SYS_QUE_TYPE_MCU_VEHICLE_CTROL_VERSION_SYNC_TO_MCU = 0x00310502,   /*九识CPU版本号同步*/
    SYS_QUE_TYPE_MCU_VEHICLE_DATABASE_CONTENT_SYNC_TO_MPU = 0x00310601,   /*九识数据库内容同步*/
    SYS_QUE_TYPE_MCU_VEHICLE_EMMC_RO_STATUS_SYNC_TO_MCU = 0x00310700,   /*九识EMMC ro 状态内容同步*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_POWER_SUPPLY = 0x00024201,   /*车辆控制结果周期上报-远程补电,a02m42s01*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_REMOTE_MEMORY_PARKING = 0x00024202,   /*车辆控制结果周期上报-远程记忆泊车,a02m42s02*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_UNLOCK_CAR = 0x00024203,   /*车辆控制结果周期上报-远程下电,a02m42s03*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_GBTEST = 0x00014201,  /*车辆31服务,a02m43s00*/
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_GBTEST_RESPONSE = 0x00014202,  /*车辆31服务结果反馈,a02m43s00*/
    SYS_QUE_TYPE_MCU_VEHICLE_RESDEVICECODE_DATA = 0x00024401,/*车辆可充电储能编码周期性上报,a32m09s01*/

    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_ESC = 0x00012000,/* 九识 ESC功能配置,a02m43s01 */
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_EXTERNAL_DISCHARGE = 0x00012100,/* 九识 对外放电功能配置,a02m43s02 */
    SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_CONTACT = 0x00012200,/* 九识 碰撞状态功能配置,a02m43s02 */

    /*TBOX内部协议-A06蓝牙服务*/ 
    SYS_QUE_TYPE_MCU_CREATE_BLE_KEY = 0x00064001,    //新建蓝牙钥匙
    SYS_QUE_TYPE_MCU_UPDATE_BLE_SECRET_KEY = 0x00064002,    //更新蓝牙钥匙密钥
    SYS_QUE_TYPE_MCU_UPDATE_BLE_KEY_PERMISSION = 0x00064003,    //更新蓝牙钥匙权限
    SYS_QUE_TYPE_MCU_DELETE_SPECIFIED_BLE_KEY = 0x00064004,    //删除指定蓝牙钥匙
    SYS_QUE_TYPE_MCU_SELECT_BLE_KEY = 0x00064005,    //查询（指定或者）全部蓝牙钥匙
    SYS_QUE_TYPE_MCU_DELECT_BLE_KEY = 0x00064006,    //删除所有蓝牙钥匙
    SYS_QUE_TYPE_MCU_BLE_MAC = 0x00064007,    //蓝牙MAC
    SYS_QUE_TYPE_MCU_CREATE_BLE_KEY_ACK = 0x00064101,    //新建蓝牙钥匙
    SYS_QUE_TYPE_MCU_UPDATE_BLE_SECRET_KEY_ACK = 0x00064102,    //更新蓝牙钥匙密钥
    SYS_QUE_TYPE_MCU_UPDATE_BLE_KEY_PERMISSION_ACK = 0x00064103,    //更新蓝牙钥匙权限
    SYS_QUE_TYPE_MCU_DELETE_SPECIFIED_BLE_KEY_ACK = 0x00064104,    //删除指定蓝牙钥匙
    SYS_QUE_TYPE_MCU_SELECT_BLE_KEY_ACK = 0x00064105,    //查询（指定或者）全部蓝牙钥匙
    SYS_QUE_TYPE_MCU_DELECT_BLE_KEY_ACK = 0x00064106,    //删除所有蓝牙钥匙
    SYS_QUE_TYPE_MCU_BLE_MAC_ACK = 0x00064107,    //蓝牙MAC地址，周期性

    /*TBOX内部协议-A15~2F 预留，用于定义MPU内部IPC消息*/
    /*TBOX内部协议-A15~2F 预留，用于定义MPU内部IPC消息 */
    SYS_QUE_TYPE_DOMAIN_FOTA_SEND = 0x002f0001,
    SYS_QUE_TYPE_DOMAIN_FOTA_RECEIVE = 0x002f0002,
    SYS_QUE_TYPE_DOMAIN_DOTA_SEND = 0x002f0003,
    SYS_QUE_TYPE_DOMAIN_DOTA_RECEIVE = 0x002f0004,
    /*0x002000为国标使用*/
    SYS_QUE_TYPE_GB32960_STATUS = 0x00200001,    //广播国标状态
    /*0x002001为远控使用*/
    SYS_QUE_TYPE_GB32960_Frequency = 0x00200101,    //广播国标上报频率
}SGMW_SYSQueMsgType_e;

/**
 * @name GetSystemRetValue
 * @brief 获取系统命令执行的返回值
 *        本函数通过调用system()函数执行指定的命令，并返回该命令的执行状态。
 * @param cmd 指向要执行的命令字符串的指针。
 * @return 如果执行成功，返回true；如果执行失败，返回false。
 */
gboolean GetSystemRetValue(const char *cmd);

/**
 * @brief   获取VIN 
 * 
 * @param   len为tboxVin指向空间大小，必须大于等于18；
 * 
 * @retval	ture -- VIN获取成功
 *          false -- VIN获取失败
 */
gboolean GetVin(uint8_t *tboxVin, size_t len);

/**
 * @brief   判断VIN是否为空
 * 
 * @retval	ture -- VIN不为空
 *          false -- VIN为空
 */
gboolean CheckVINIsEmpty(const guint8 *vin);

/**
 * @brief   判断VIN是否为空
 * 
 * @retval	ture -- VIN为空
 *          false -- VIN不为空
 */
gboolean CheckTboxVINIsEmpty();

/**
 * @brief   获取ICCID
 * 
 * @param   len为tboxIccid指向空间大小，必须大于等于21；
 * 
 * @retval	ture -- ICCID获取成功
 *          false -- ICCID获取失败
 */
gboolean GetIccid(uint8_t *tboxIccid, size_t len);

/**
 * @brief   判断ICCID是否为空
 * 
 * @retval	ture -- ICCID不为空
 *          false -- ICCID为空
 */
gboolean CheckIccidIsEmpty(const guint8 *iccid);

/**
 * @brief   判断ICCID是否为空
 * 
 * @retval	ture -- ICCID不为空
 *          false -- ICCID为空
 */
gboolean CheckTboxIccidIsEmpty();

/**
 * @brief   获取软件版本号
 * 
 * @param   len为softVersion指向空间大小，必须大于等于12；
 * 
 * @retval	ture -- 版本获取成功
 *          false -- 版本获取失败
 */
gboolean GetSoftVersion(uint8_t *softVersion, size_t len);

/**
 * @brief   获取定位（从本地文件/oemdata/tmp/location中获取）
 * 
 * @param   len为location指向空间大小，必须大于等于22；
 * 
 * @retval	ture -- 定位获取成功
 *          false -- 定位获取失败
 */
gboolean GetLocation(uint8_t *location, size_t len);

/**
 * @brief   获取TspTokenUrl
 * 
 * @param   len为TspTokenUrl指向空间大小，必须大于等于77；
 * 
 * @retval	ture -- TspTokenUrl获取成功
 *          false -- TspTokenUrl获取失败
 */
gboolean GetTspTokenUrlUrl(uint8_t *TspTokenUrl, size_t len);

/**
 * @brief   获取在线配置参数
 * 
 * @param   len为ConfigurationParam指向空间大小，必须大于等于77；
 * 
 * @retval	ture -- ConfigurationParam获取成功
 *          false -- ConfigurationParam获取失败
 */
gboolean GetConfiguration(uint8_t *ConfigurationParam, size_t len);

/**
 * @brief   获取SN序列号
 * 
 * @param   len为指向空间大小，必须大于等于28；
 * 
 * @retval	ture -- tboxSN获取成功
 *          false -- tboxSN获取失败
 */
gboolean GetSN(uint8_t *tboxSN, size_t len);

/**
 * @brief   判断TboxSN是否为空
 * 
 * @retval	ture -- TboxSN不为空
 *          false -- TboxSN为空
 */
gboolean CheckSNIsEmpty(const guint8 *tboxSN);

/**
 * @brief   判断TboxSN是否为空
 * 
 * @retval	ture -- TboxSN不为空
 *          false -- TboxSN为空
 */
gboolean CheckTboxSNIsEmpty();

/**
 * @brief   获取IMEI
 * 
 * @param   len为指向空间大小，必须大于等于16；
 * 
 * @retval	ture -- IMEI获取成功
 *          false -- IMEI获取失败
 */
gboolean GetIMEI(uint8_t *IMEI, size_t len);

/**
 * @brief   判断IMEI是否为空
 * 
 * @retval	ture -- IMEI不为空
 *          false -- IMEI为空
 */
gboolean CheckIMEIIsEmpty(const guint8 *IMEI);

/**
 * @brief   判断IMEI是否为空
 * 
 * @retval	ture -- IMEI不为空
 *          false -- IMEI为空
 */
gboolean CheckTboxIMEIIsEmpty();

/**
 * @brief   获取GB addr
 * 
 * @param   len为GBAddr指向空间大小，必须大于等于65；
 * 
 * @retval	ture -- GB addr获取成功
 *          false -- GB addr获取失败
 */
gboolean GetGBAddr(uint8_t *GBAddr, size_t len);

/**
 * @brief   获取GB Frequency 不可用
 * 
 * @param   整形；
 * 
 * @retval	ture -- GBFrequency获取成功
 *          false -- GBFrequency获取失败
 */
//gboolean GetGBFrequency(uint8_t *GBFrequency);

/**
 * @brief   设置GB Frequency 不可用
 * 
 * @param   整形；
 * 
 * @retval	ture -- GBFrequency设置成功
 *          false -- GBFrequency设置失败
 */
//gboolean SetGBFrequency(uint8_t GBFrequency);


/**
 * @brief   获取编译时间
 * 
 * @param   
 * 
 * @retval	
 *          
 */

time_t GetCompareTime();

/**
 * @brief   检查IP地址是否被路由到某个网卡上
 * 
 * @param   ifname 网卡名称；
 *          ipaddr IP地址；
 * 
 * @retval	TRUE -- IP被路由到网卡中
 *          FALSE -- IP没有被路由到网卡中
 */
gboolean checkIPonInterface(const char* ifname, struct in_addr ipaddr);

gboolean GetCarNum(uint8_t *carNum, size_t len);

gboolean GetTboxSoftVersion(uint8_t *softVersion, int32_t softVersionLen);

gboolean SetTboxSoftVersion(uint8_t *softVersion, int32_t softVersionLen);

#ifdef __cplusplus
}
#endif

#endif
