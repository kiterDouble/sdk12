#ifndef _QUEUEMSG_H
#define _QUEUEMSG_H

#include "glib.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // IPC通信消息类型定义
    typedef enum
    {
        SYS_QUE_TYPE_TEST1 = 0x1000,
        SYS_QUE_TYPE_TEST2 = 0x3FFE,
        /*内部通讯协议0x0001xxxx~0x003Fxxxx*/
    
        /*TBOX内部协议-A01状态同步，从APP中移入server，因为FTY中也需要使用*/
        SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW = 0x00010100,                      /*车辆硬线状态,a01m01s00*/
        /*TBOX内部协议-A01状态同步*/
        SYS_QUE_TYPE_MCU_SYNC_KLINE = 0x0001017f,                      /*车辆硬线状态,a01m01s00*/
        SYS_QUE_TYPE_MCU_SYNC_TIME = 0x00010201,                       /*同步时间，a01m02s01*/
        SYS_QUE_TYPE_MCU_SYNC_TBOX_READ_REQ = 0x00010301,              /*同步TBOX参数-读取CPU参数请求，a01m03s01*/
        SYS_QUE_TYPE_MCU_SYNC_TBOX_READ_RES = 0x00010302,              /*同步TBOX参数-读取CPU参数回复，a01m03s02*/
        SYS_QUE_TYPE_MCU_SYNC_TBOX_MCU_WRITE_REQ = 0x00010303,         /*同步TBOX参数-MCU同步参数到CPU请求，a01m03s03*/
        SYS_QUE_TYPE_MCU_SYNC_TBOX_MCU_WRITE_RES = 0x00010304,         /*同步TBOX参数-MCU同步参数到CPU回复，a01m03s04*/
        SYS_QUE_TYPE_MCU_SYNC_TBOX_CPU_WRITE_REQ = 0x00010305,         /*同步TBOX参数-CPU同步参数到MCU请求，a01m03s05*/
        SYS_QUE_TYPE_MCU_SYNC_TBOX_CPU_WRITE_RES = 0x00010306,         /*同步TBOX参数-CPU同步参数到MCU回复，a01m03s05*/
        SYS_QUE_TYPE_MCU_SYNC_POWER_SLEEP_REQ_RES = 0x00010401,        /*同步电源模式-MCU请求休眠/CPU休眠回复，a01m04s01*/
        SYS_QUE_TYPE_MCU_SYNC_POWER_WAKEUPSOURCE_REQ_RES = 0x00010404, /*同步电源模式-请求唤醒源/回复唤醒源，a01m04s04*/
        SYS_QUE_TYPE_MCU_SYNC_POWER_STATUS = 0x00010405,               /*同步电源模式-MCU同步电源状态到CPU，a01m04s05*/
        SYS_QUE_TYPE_MCU_SYNC_POWER_BLOCK_EVENT = 0x00010406,          /*同步电源模式-CPU同步阻塞事件到MCU，a01m04s06*/
        SYS_QUE_TYPE_MCU_SYNC_PIN_REQ = 0x00010501,                    /*CPU控制MCU引脚-请求控制引脚，a01m05s01*/
        SYS_QUE_TYPE_MCU_SYNC_PIN_RES = 0x00010502,                    /*CPU控制MCU引脚-控制结果回复，a01m05s02*/
        SYS_QUE_TYPE_MCU_SYNC_FTY_PERIOD_STATUS_INFO = 0x00010801,     /*CPU周期同步状态信息-CPU周期下发 GPS状态，经纬度、登录平台状态等，a01m08s01*/
        SYS_QUE_TYPE_MCU_SYNC_POSITON = 0x00011000,                    /*定位信息同步，a01m10s00*/
        SYS_QUE_TYPE_MCU_SYNC_NETWORK_STATUS = 0x00011100,             /*网络信息同步，a01m11s00*/
        SYS_QUE_TYPE_MCU_SYNC_MODEM_ALIVE = 0x00011101,                /*网络在线状态同步，a01m11s01*/
        SYS_QUE_TYPE_MCU_SYNC_MODEM_NET_REQ = 0x00011102,              /*模组网络切换请求，a01m11s02*/
        SYS_QUE_TYPE_MCU_SYNC_MODEM_NET_RES = 0x00011103,              /*模组网络切换回复，a01m11s03*/
        SYS_QUE_TYPE_MCU_SYNC_CAN = 0x00011200,                        /*CAN信息同步，a01m12s00*/
        SYS_QUE_TYPE_MCU_SYNC_BLE = 0x00011300,                        /*蓝牙信息同步，a01m13s00*/
        SYS_QUE_TYPE_MCU_SYNC_GSENSOR = 0x00011400,                    /*G-SENSOR信息同步，a01m14s00*/
        SYS_QUE_TYPE_MCU_SYNC_BATTERY = 0x00011500,                    /*电池信息同步，a01m15s00*/
        SYS_QUE_TYPE_MCU_SYNC_CPU_STATUS = 0x00011600,                 /*CPU状态信息同步，a01m16s00*/
        SYS_QUE_TYPE_MCU_SYNC_TSP_STATUS = 0x00011700,                 /*TSP平台连接状态信息同步，a01m17s00*/
        /*TBOX内部协议-A02涉车服务*/
        SYS_QUE_TYPE_MCU_VEHICLE_CAN_ID_REQ = 0x00020101,     /*MCU向CPU请求透传CAN ID配置信息,a02m01s01*/
        SYS_QUE_TYPE_MCU_VEHICLE_CAN_ID_RES = 0x00020102,     /*CPU回复MCU透传CAN ID配置信息请求,a02m01s02*/
        SYS_QUE_TYPE_MCU_VEHICLE_CAN_DATA = 0x00020200,       /*CAN数据透传,a02m02s00*/
        SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_ACK = 0x0002036E,    /*车辆控制-MCU收到控制命令确认,a02m03s6E*/
        SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_RES = 0x0002036F,    /*车辆控制-MCU控制命令结果,a02m03s6F*/
        SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_ENGINE = 0x00020301, /*车辆控制-开关引擎,a02m03s01*/
        SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_AC = 0x00020302,     /*车辆控制-开关空调,a02m03s02*/
        SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_LOCK = 0x00020303,   /*车辆控制-门锁控制,a02m03s03*/
        SYS_QUE_TYPE_MCU_VEHICLE_CONTROL_FIND = 0x00020304,   /*车辆控制-远程寻车,a02m03s04*/
        /*TBOX 内部协议-A03升级服务*/
        SYS_QUE_TYPE_MCU_UPGRADE_GET_VER = 0x00030100,        /*升级MCU获取版本号*/
        SYS_QUE_TYPE_MCU_UPGRADE_GET_SEED = 0x00030200,       /*升级MCU获取SEED*/
        SYS_QUE_TYPE_MCU_UPGRADE_UNLOCK_FLASH = 0x00030300,   /*升级MCU解锁下载功能*/
        SYS_QUE_TYPE_MCU_UPGRADE_PUT_FLASHING = 0x00030400,   /*升级MCU 进入下载模式*/
        SYS_QUE_TYPE_MCU_UPGRADE_ERASE_FLASH = 0x00030500,    /*升级MCU 擦除flash*/
        SYS_QUE_TYPE_MCU_UPGRADE_FLASH_MCU = 0x00030600,      /*升级MCU 下载升级数据*/
        SYS_QUE_TYPE_MCU_UPGRADE_FLASH_COMPLETE = 0x00030700, /*升级MCU下载完成*/
        SYS_QUE_TYPE_MCU_UPGRADE_RESET_MCU = 0x00030800,      /*升级MCU 重启MCU*/
        SYS_QUE_TYPE_MCU_UPGRADE_GET_RUN_APP = 0x00030900,    /*升级MCU 获取运行状态*/
        SYS_QUE_TYPE_BT_UPGRADE_READY = 0x00030a00,           /*升级bt 获取是否可以升级BT*/
        SYS_QUE_TYPE_BT_UPGRADE_ERASE_FLASH = 0x00030b00,     /*升级BT 擦除flash*/
        SYS_QUE_TYPE_BT_UPGRADE_FLASHING_BT = 0x00030c00,     /*升级BT 发送升级数据*/
        SYS_QUE_TYPE_BT_UPGRADE_FLASH_COMPLETE = 0x00030d00,  /*升级BT完成 发送crc*/
        SYS_QUE_TYPE_BT_UPGRADE_RESET_BT = 0x00030e00,        /*升级BT 重启BT*/ 
        /*TBOX内部协议-A04警情服务*/
        SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_REQ = 0x00041001, /*车辆控制-触发ECALL,a04m10s01*/
        SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_RES = 0x00041002, /*ECALL信息同步,a04m10s02*/
        SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_AMP_GAIN =  0x00041003, /* 设置AMP增益 */
        SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_CON_SERVICE = 0x00041004, /* 状态检测 */
        SYS_QUE_TYPE_MCU_ALARM_BCALL_EVENT_REQ = 0x00041401, /*车辆控制-触发ECALL,a04m10s01*/
        SYS_QUE_TYPE_MCU_ALARM_BCALL_EVENT_RES = 0x00041402, /*ECALL信息同步,a04m10s02*/

        /*TBOX内部协议-A05工装远程诊断协议*/
        SYS_QUE_TYPE_MCU_CANTP_MPU_TO_MCU = 0x00050101,     /*CANTP诊断MPU到MCU消息*/
        SYS_QUE_TYPE_MCU_CANTP_MPU_TO_MCU_ACK = 0x00050102, /*CANTP诊断MPU到MCU消息确认*/
        SYS_QUE_TYPE_MCU_CANTP_MCU_TO_MPU = 0x00050103,     /*CANTP诊断MCU到MPU消息*/
        SYS_QUE_TYPE_MCU_CANTP_MCU_TO_MPU_ACK = 0x00050104, /*CANTP诊断MCU到MPU消息确认*/     
        SYS_QUE_TYPE_MCU_SYNC_FTY_REQ = 0x00051001,                    /*工装管理-mcu控制cpu请求测试项，a05m10s01*/
        SYS_QUE_TYPE_MCU_SYNC_FTY_RES = 0x00051002,                    /*工装管理-mpu回复mcu请求测试项，a05m10s02*/
        /*TBOX内部协议-A10 电源管理对外广播*/
        SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY = 0x00100001, /*发布电源状态信息*/
        /*TBOX内部协议-A11 信号服务对外广播*/
        /*TBOX内部协议-A12 Modem服务,时间服务对外广播， E-CALL对外广播，B-CALL对外广播，网络管理*/
        SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY = 0x00120001, /*模组信息通知*/
        SYS_QUE_TYPE_MODEM_CALL_NOTIFY = 0x00120002,
        SYS_QUE_TYPE_MODEM_SMS_NOTIFY = 0x00120003,
        SYS_QUE_TYPE_TIME_STATUS_NOTIFY = 0x00120004,
        SYS_QUE_TYPE_ECALL_INCOMING_NOTIFY = 0x00120005,
        SYS_QUE_TYPE_BCALL_INCOMING_NOTIFY = 0x00120006,
        SYS_QUE_TYPE_NET_STATUS_NOTIFY = 0x00120007,
        SYS_QUE_TYPE_MODEM_HAL_ERROR_NOTIFY = 0x00120008,/*模组错误消息广播*/
        /*TBOX内部协议-A13 GNSS服务 RTK模块*/
        SYS_QUE_TYPE_GNSS_LOCATIONINFO = 0x00130001, /*位置信息广播         [gnss_ipc_msg.h---GnssLocation_t]*/
        SYS_QUE_TYPE_GNSS_NMEA = 0x00130002,         /*NMEA语句信息广播     [gnss_ipc_msg.h---GnssNmea_t]*/
        SYS_QUE_TYPE_RTK_INFO_STATUS = 0x00130004,      /*时间同步状态广播     [gnss_ipc_msg.h---GnssNmea_t]*/
        SYS_QUE_TYPE_PING_NET = 0x00130005,      /*PING网状态广播     [net_monitor_ipc_msg.h---NetState_t]*/
        /*TBOX内部协议-A14 用于v2x相关模块/服务广播数据*/
        /*TBOX内部协议-A15~2F 预留，用于定义MPU内部IPC消息*/
        SYS_QUE_TYPE_PARAM_UPDATE = 0x00150001,      /*参数数据库基础参数被更新时主动通知APP 消息结构:ParamInfo_t*/
        /*TBOX内部协议-A30~3F 预留，用于项目添加自定义应用。包括MCU与MPU之间，MPU内部*/
        SYS_QUE_TYPE_SGMW_UPGRADE_STATUS_SYNC = 0x00301001,//升级状态同步，仅限mpu内部使用
    } SYSQueMsgType_e;

    //RPC通信Server定义
    typedef enum
    {
        RPCSERVER_TYPE_MIN_LIMIT = 0x70000000, //请勿变更
        RPCSERVER_TYPE_RPCSERVERDEMO,
        RPCSERVER_TYPE_ATSERVER,
        RPCSERVER_TYPE_MCUSERVER,
        RPCSERVER_TYPE_GNSSSERVER,
        RPCSERVER_TYPE_ECALLSERVER,
        RPCSERVER_TYPE_BCALLSERVER,
        RPCSERVER_TYPE_PARAMSERVER,
        RPCSERVER_TYPE_MODEMSERVER,
        RPCSERVER_TYPE_PMSERVER,
        RPCSERVER_TYPE_NETSERVER,
        /* 有新增的server在上面往后依次罗列 */

        RPCSERVER_TYPE_APP_BASE = 0x70001000,//APP使用的RPCServer Base id
        /* APP使用的RPCServer id 范围为0x70001001-0x7000FFFE */

        RPC_QUE_TYPE_MAX_LIMIT = 0x7000FFFF//请勿变更
    } RPCServerType_e;

#pragma pack(1)
    typedef struct
    {
        long type; /**< type of SYSQueMsgType_e*/
        uint16_t datalen;
        uint8_t data[0];
    } SysQueueData_t;

    typedef struct
    {
        long type;          // RPC SERVER定义 每个进程启动一个SERVER 需要使用RPCServerType_e中的枚举与取值范围
        uint16_t subcmd;    // 每个SERVER中提供多条接口 subcmd代表接口编号
        uint32_t clienttid; // rpc client请求时携带的请求时所在的线程id
        uint16_t datalen;
        uint8_t data[0];
    } RpcQueueData_t;
#pragma pack()

#define SYSQUE_MSG_DATA_MAX_LEN (5120)

#ifdef __cplusplus
}
#endif

#endif
