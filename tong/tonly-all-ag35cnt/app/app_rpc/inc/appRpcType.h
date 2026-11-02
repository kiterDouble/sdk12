#ifndef APP_RPC_TYPE_H
#define APP_RPC_TYPE_H

#include "sr_param.h"

/*10-30为app层预留定时器ID，用户可以根据自己的需求进行定义，注意定时器不能重复创建，同一个ID删除后可再次创建*/
// #define CPU_APP_TIMER_START                             (10)
#define CPU_APP_TIMER_TBOX_RTC                             (11u) // tbox定时唤醒
#define CPU_APP_TIMER_OTA_RTC                              (12u) // OTA定时唤醒
// #define CPU_APP_TIMER_END                               (30)

typedef enum
{
    SYS_QUE_TYPE_MPU_SET_POWER_BLOCK_EVENT_TO_MCU = 0x00010407,   /*MPU主动抑制MCU休眠，a01m04s07 01：不允许休眠 00：允许休眠*/

    APP_RPC_QUE_TYPE_MCU_DTC_MCU_TO_MPU     = 0x00051022,         /*MCU请求MPU DTC 诊断调查表数据(读写),a05m10s22*/
    APP_RPC_QUE_TYPE_MCU_DTC_MPU_TO_MCU_ACK = 0x00051023,         /*MPU应答MCU DTC 结果,a05m10s23*/
    APP_RPC_QUE_TYPE_MCU_RESET_MPU_TO_MCU   = 0x00051024,         /*MPU请求MCU复位 参数 0x00复位MPU 0x01复位MCU*/
    APP_RPC_QUE_TYPE_TBOX_OTA_MODE_MPU_TO_MPU   = 0x00051025,     /*OTA Master同步TBOX OTA模式状态*/

    /*num(1byte) + did(2byte*num)*/
    APP_RPC_QUE_TYPE_SYNC_DTC_MPU_FROM_MCU     = 0x00010307,      /*MPU从MCU获取广汽下线写入的did信息*/
    /*num(1byte) + 
    did1(2byte) + len1(2byte) + data1(nbyte) + 
    did2(2byte) + len2(2byte) + data2(nbyte) + 
    ...
    didn(2byte) + lenn(2byte) + datan(nbyte)*/
    APP_RPC_QUE_TYPE_SYNC_DTC_MPU_FROM_MCU_ACK = 0x00010308,      /*MCU响应包*/
    APP_RPC_QUE_TYPE_SYNC_DTC_MCU_FROM_MPU     = 0x00010309,      /*MCU从MPU获取EOL数据*/
    APP_RPC_QUE_TYPE_SYNC_DTC_MCU_FROM_MPU_ACK = 0x00010310,      /*MPU应答包*/

    APP_RPC_QUE_TYPE_SYNC_FTY_PERIOD_STATUS_INFO_TO_MCU = 0x00010802,    /*CPU周期同步状态信息-CPU周期下发 GPS状态，经纬度、登录平台状态等，a01m08s02*/

    APP_RPC_QUE_TYPE_MCU_CANTP_MPU_TO_MCU = 0x00052001,     /*诊断刷写MPU到MCU消息*/
    APP_RPC_QUE_TYPE_MCU_CANTP_MPU_TO_MCU_ACK = 0x00052002, /*诊断刷写MPU到MCU消息确认*/
    APP_RPC_QUE_TYPE_MCU_CANTP_MCU_TO_MPU = 0x00052003,     /*诊断刷写MCU到MPU消息*/
    APP_RPC_QUE_TYPE_MCU_CANTP_MCU_TO_MPU_ACK = 0x00052004, /*诊断刷写MCU到MPU消息确认*/
} AppQueMsgType_e;

typedef enum
{

    RPC_APP_SERVER_TYPE_APP_BASE = 0x70001000,//APP使用的RPCServer Base id
    /* APP使用的RPCServer id 范围为0x70001001-0x7000FFFE */
    RPC_APP_SERVER_TYPE_TSP = 0x70001001,
    RPC_APP_SERVER_TYPE_DTC,
    RPC_APP_SERVER_TYPE_AVNT,
    RPC_APP_SERVER_TYPE_OTA,
    RPC_APP_SERVER_TYPE_POINTDATA,
} RPCAppType_e;

typedef enum
{
#if 1 //SQ 按需求添加
    E_ParamId_app_start = E_ParamId_UserStart,    
#else
    E_ParamId_app_start = E_ParamId_UserStart,
    E_ParamId_Transport_mode,           // 运输模式 0：disable 1:enable
    E_ParamId_Manufactory_mode,         // 工厂模式 0xFF-0x10:工厂模式（0xFF:初始值）0x0F-0x00：非工厂模式（0x00：出厂值）
    E_ParamId_Tbox_Running_Mode,        // TBOX运行模式
    E_ParamId_DTC_control_state,        // DTC控制状态
    E_ParamId_RTC_Wakeup_Interval,      // RTC唤醒间隔
    E_ParamId_TSP_Conn_Timeout,         // TSP平台连接超时
    E_ParamId_TSP_Timeout_For_Flowchart, // TSP流程超时时间
    E_ParamId_TSP_Ack_Time,              // TSP平台应答时间
    E_ParamId_Long_Connection_Cycle,     // 长连接周期
    E_ParamId_Ethernet_Config_Info = 110,      // Ethernet配置信息
    E_ParamId_TSP_Platform_Conn_Setting, // TSP平台连接设置
    E_ParamId_4G_HW_Reset_Time,          // 4G模块硬件复位时间
    E_ParamId_Sensitive_Data_Set,        // 敏感数据设置(16字节)
    E_ParamId_Sensitive_Data_Set_1,      // 敏感数据设置1(0-3字节)
    E_ParamId_Sensitive_Data_Set_2,      // 敏感数据设置2(4-7字节)
    E_ParamId_Sensitive_Data_Set_3,      // 敏感数据设置3(8-11字节)
    E_ParamId_Sensitive_Data_Set_4,      // 敏感数据设置4(12-15字节)
    E_ParamId_OTA_RTC_Wakeup_Param,      // OTA RTC唤醒参数(8 byte)
    E_ParamId_OTA_RTC_Wakeup_Param_1,    // OTA RTC唤醒参数1(0-3字节)
    E_ParamId_OTA_RTC_Wakeup_Param_2 = 120,    // OTA RTC唤醒参数2(4-7字节)
    E_ParamId_Burying_Point_Switch,      // 埋点开关(4字节)
    E_ParamId_Conn_Test_Addr,            // 连接测试地址

    E_ParamId_UserPrivateTspDomain,      // 专网TSP域名
    E_ParamId_UserPrivateTspPort,        // 专网TSP端口
    E_ParamId_UserPrivateTspPort_1,      // 专网TSP端口(0-3)
    E_ParamId_UserPrivateTspPort_2,      // 专网TSP端口(4-7)
    E_ParamId_UserPrivateTspConnStatus,  // 专网TSP连接状态
    E_ParamId_UserPublicTspDomain,       // 公网TSP域名
    E_ParamId_UserPublicTspPort,         // 公网TSP端口
    E_ParamId_UserPublicTspPort_1 = 130,       // 公网TSP端口(0-3)
    E_ParamId_UserPublicTspPort_2,       // 公网TSP端口(4-7)
    E_ParamId_UserPublicTspConnStatus,   // 公网TSP连接状态
    E_ParamId_UserPkiDomain,             // PKI域名
    E_ParamId_UserPkiPort,               // PKI端口
    E_ParamId_UserPkiPort_1,             // PKI端口(0-3)
    E_ParamId_UserPkiPort_2,             // PKI端口(4-7)
    E_ParamId_UserPkiCycle,              // PKI维护周期
    E_ParamId_UserLogUploadDomain,       // 日志上传域名
    E_ParamId_UserLogUploadPort,         // 日志上传端口
    E_ParamId_UserLogUploadPort_1 = 140,       // 日志上传端口(0-3)
    E_ParamId_UserLogUploadPort_2,       // 日志上传端口(0-3)
    E_ParamId_UserLogUploadAccount,      // 日志上传账号
    E_ParamId_UserLogUploadPassword,     // 日志上传密码
    E_ParamId_UserLogUploadInterface,    // 日志上传通道
    E_ParamId_UserLogUploadPath,         // 日志路径
    E_ParamId_UserOtaDomain,             // OTA域名
    E_ParamId_UserOtaPort,               // OTA端口
    E_ParamId_UserTelemetryDataDomain,   // 埋点数据域名
    E_ParamId_UserTelemetryDataPort,     // 埋点数据端口

    E_ParamId_4G_HW_Reset_Count = 150,
#endif
    E_ParamId_app_end = E_ParamId_UserEnd
} E_ParamId_app_e;

#endif