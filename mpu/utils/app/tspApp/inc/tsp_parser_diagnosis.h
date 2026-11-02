#ifndef _TSP_PARSER_DIAGNOSIS_H_
#define _TSP_PARSER_DIAGNOSIS_H_

#include <glib.h>
#include <fcntl.h>
#include <unistd.h>

#define TBOXCPUVERSIONNAME			"/oemapp/upgrade/version.txt"
#define ODER_CHARGING_INFO			"/oemdata/data/oderChargingV2.json"

typedef struct 
{
    // 是否打开预约充电
    guchar flag;
    // 使用的预约充电版本
    guchar version;
    // 预约充电模式 单次/每日（version为2时起效）
    guchar appointmentType;
    // MCU的设置状态：0未设置完成，1设置完成；默认为1，接收指令时清空，接收设置回复时置为1
    guchar mcuSetState;
    // v1版本使用
    unsigned long long startTime;
    unsigned long long endTime;

    // v2版本开始时间：小时 0-23
    guchar startTimeH;
    // v2版本开始时间：分钟 0-55
    guchar startTimeM;
    // v2版本结束时间：小时 0-23
    guchar endTimeH;
    // v2版本结束时间：分钟 0-55
    guchar endTimeM;
    // v2版本预约单次时开始时间,s
    unsigned long long singleStartTime;
    // v2版本发送标志位，预约每日时切换发送禁止/允许预约充电;启动时flag和flagTmp全部设置为0;
    guchar sendFlag;
    guchar sendFlagTmp; 
}ReserverCharging;

gint DiagnosisLogin();
gint DiagnosisLogout();
gint DiagnosisHeartbeat();
gint DiagnosisSleep();
gint DiagnosisWakeUp(uint8_t wakeUpSource);
gint DiagnosisUploadeVersion();
gint ControlCmdTimeOut();
gint GetReserverChargingTime(ReserverCharging *pReserverChargingData);
gint SetReserverChargingTime(ReserverCharging *pReserverChargingData);
gint ControlReserverCharging(guchar *pControlData, gint controlDataLen);
gint ControlReserverChargingOnlySend(guchar *pControlData, gint controlDataLen);
gint InitOderChargingTime(const char *pFilePath, gint filePathLen);
//gint32 checkIPonInterface(const char* ifname, struct in_addr ipaddr);
gint ParserDiagnosisData(gpointer pData, guint u32Len);
//gpointer RemoteControlMcuHandleTaskEntry(gpointer usrData);

gint ControlRemotePowerDown(guchar *pControlData, gint controlDataLen);
gint ControlOTAPowerUp(guchar *pControlData, gint controlDataLen);
gint GetRdmRemoteCtrolRet(char *pwerOn, char *powerOff);

gint ZelosControlRemotePowerup();
gint ZelosControlRemotePowerdown();

//C3
int32_t InitSignalIpcCallback();


#endif
