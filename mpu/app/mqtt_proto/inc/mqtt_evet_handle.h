/*************************************************************************
    > File Name: task_exc_handle.h
    > Author: fxl 
    > Created Time: 2021/4/25
    > Descripton: 
    > Version     time   
    > 01                   fxl    
 ************************************************************************/


#ifndef __MQTT_EVET_HNNDLE__H_
#define __MQTT_EVET_HNNDLE__H_
#include "mqtt_common.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct  
{
    char dipHeadLight; //远光灯灯是否开启，0关1开
    char lowBeamLight; //近光灯是否开启，0关1开
    char PsWdwOpenDgr; //副驾驶车窗是否开启，0关1开(开度小于2为关闭,大于等于2为开)
    char DrWdwOpenDgr; //驾驶员侧车窗开度 0关1开 (开度小于2为关闭,大于等于2为开)
    char sntrModSetSts; //哨兵模式状态，0关1开
    char vecChrgStsIndOn1; //快充充电状态，0未充电，1充电中
    char vecChrgStsIndOn2; //慢充充电状态，0未充电，1充电中
    char dDAjrSwAtv; //驾驶员侧门碰开关状态，0关，1开
    char tDAjrSwAtv; //尾门门碰开关状态，0关1开
    char keyPosition; //钥匙状态
    char psDoorOpenSwAct; //右前门锁开关状态，0关1开
    char pDAjrSwAtv; //右前门门碰开关状态，0关1开
    char drDoorOpenSwAct; //驾驶员侧门锁开关状态，0关1开
    char aCACButCntSt; //空调AC 按键控制状态
    char aCCntTmpOLfCntSt; //空调左温区控制温度控制状态,单位:℃
    char aCBLwLvlCntSt; //空调风量
    char chargeLoVolBatSts; //低压蓄电池补电状态，0正常工作，1停止补电
}VehicleStatus;

typedef struct
{
    char APPInfcSw;//应用程序界面切换
    char VehCntrlSt;//整车控制状态，0 本地模式 1 远程启动模式 2 泊车上电模式
    char IPAOperSts;//智能泊车辅助运行状态
    char VPAOperSts;//记忆泊车运行状态
    char IPAFailrReas;//智能泊车辅助 IPA失败原因
    char VPAgFailrReas;//记忆泊车失败原因
}VehicleParkingStstus;

typedef struct  
{
    char PsWdwOpenDgr; //副驾驶车窗是否开启，0关1开(开度小于2为关闭,大于等于2为开)
    char DrWdwOpenDgr; //驾驶员侧车窗开度 0关1开 (开度小于2为关闭,大于等于2为开)
    char tDAjrSwAtv; //尾门门碰开关状态，0关1开
    char pDAjrSwAtv; //右前门门碰开关状态，0关1开
    char dDAjrSwAtv; //驾驶员侧门碰开关状态，0关，1开
    char psDoorOpenSwAct; //右前门锁开关状态，0关1开
    char drDoorOpenSwAct; //驾驶员侧门锁开关状态，0关1开
    char remainOdo[2];
    char keyPosition; //钥匙状态
    char sntrModSetSts; //哨兵模式状态，0关1开
    char aCACButCntSt; //空调AC 按键控制状态
    char aCBLwLvlCntSt; //空调风量
    char aCCntTmpOLfCntSt; //空调左温区控制温度控制状态,单位:℃
    char batSoc;
    char chargeLoVolBatSts; //低压蓄电池补电状态，0正常工作，1停止补电
    char dipHeadLight; //远光灯灯是否开启，0关1开
    char lowBeamLight; //近光灯是否开启，0关1开
    char rRDoorOpenSwAct; //右后门锁开关状态 
    char rSDAjrSwAtv; //右后门门碰开关状态
    char rLDoorOpenSwAct; //左后门锁开关状态
    char lSDAjrSwAtv; //左后门门碰开关状态
    char VecChrgStsIndOn1; //快充充电状态
    char VecChrgStsIndOn2; //慢充充电状态
    char ACPwrModCntSt; //空调电源模式
    char SecRwLtWdwOpenDgr; //第二排左窗开关状态
    char SecRwRtWdwOpenDgr; //第二排右窗开关状态
    char drSeatVentRCC;  //驾驶员侧座椅远程通风设置状态
    char prSeatVentRCC;  //副驾驶侧座椅远程通风设置状态
    char drSeatHeatRCC;  //驾驶员侧座椅加热远程控制状态
    char prSeatHeatRCC;  //副驾驶员侧座椅加热远程控制状态
    
    char rlSeatVentRCC; //后排左侧座椅远程通风控制请求
    char rrSeatVentRCC; //后排右侧座椅远程通风控制请求
    char rlSeatHeatRCC;  //后排左侧座椅加热远程控制请求
    char rrSeatHeatRcc;  //后排右侧座椅加热远程控制请求
}VehicleStatusMCu;


typedef struct
{
    uint8_t VehicleSOC;//车辆剩余电量 0x30A  
    uint8_t ChargeStatus;//充电状态 0放电  1 充电正常 
    uint8_t VehicleStatus;//车辆状态 0x211 3下电  0 上电
    uint8_t VCU_EndurMile_km[4];// 续驶里程  0x3c0
    uint8_t VCU_VehODO_km[4];// 车辆行驶里程（精度0.1km） 0X3C0
    uint8_t VCU_EgyConspt100km[2];// 百公里能耗 0x3C2    
    uint8_t ESC_VehSpd_kph[2];// 车速 0X114
    uint8_t Low_voltage;//小电池电压  KL30电压
    uint8_t BCM_FLTirePres_kPa[2];// 左前轮胎压力  0x356
    uint8_t BCM_FRTirePres_kPa[2];// 右前轮胎压力  0x356
    uint8_t BCM_RLTirePres_kPa[2];// 左后轮胎压力  0x357
    uint8_t BCM_RRTirePres_kPa[2];// 右后轮胎压力  0x357
    uint8_t g_u32CanTxMsgCnt[4];// can发送计数 
    uint8_t g_u32CanRxMsgCnt[4];// can接收计数
    uint8_t VehicleMeterSOC;// 仪表校准
    uint8_t PowerOffCountdown;// 下电倒计时(0-63, Unit: 秒)
    uint8_t pwr[2];//TBOX供电电压
    uint8_t temp[2];//温度值
    uint8_t lanStatus;//内网状态
    uint8_t mcuRunTimes[4];			//MCU运行时间
    uint8_t masterModemResetNum[2];	//MCU复位主模组次数
    uint8_t slaveModemResetNum[2];	//MCU复位副模组次数
}VehicleCycleData;

typedef struct
{
    uint8_t Main_Modem;//0x0：未重启, 0x1:已重启
    uint8_t Main_Reset_Reason;//0x0:  未重启, 0x1:串口重启, 0x2:ping重启, 0x3:拨号异常重启
    uint8_t Main_Reset_Count;//MCU休眠唤醒周期内重启次数
    uint8_t Slave_Modem;//0x0：未重启, 0x1:已重启
    uint8_t Slave_Reset_Reason;//0x0:  未重启, 0x1:串口重启, 0x2:ping重启, 0x3:拨号异常重启
    uint8_t Slave_Reset_Count;// MCU休眠唤醒周期内重启次数
    uint8_t sleepDisableEvent;//MPU禁止休眠
    uint8_t AllNetSleepStatue;//Can维持唤醒
    uint8_t PmKeepWakeEvent;//MCU事件维持唤醒
    uint8_t LOW_POWER_MODE_FLAG;//低功耗模式
    uint8_t MainFaultFlag;//主模组异常
}ResetRsonCycleData;



typedef struct
{
    uint8_t modemMasterOrSlave;		//主副模组状态 1： 主模组，2：副模组
    uint8_t funcSwitch;				//功能开关状态  1:开 0:关 bit0: 自动切卡(切网关)， bit1:保留 ，bit2:低功耗  bit3:Q0S  bit4: 主模组锁定到LTE(0 AUTO)， bit5: 副模组锁定到LTE(0 AUTO)
    uint8_t ICCID[20];				//ICCID 
    uint8_t signalSsi;				//信号强度
    uint8_t signalSnr;				//信噪比
    uint8_t	flow[4];				//流量消耗单位MB
    uint8_t modemVer[7];			//模组版本号 6-5: 项目号 4: 主版本号， 3-2：MPU 版本号, 1-0: MCU 版本号
    uint8_t modemSta;				//模组状态： bit0-SIM卡状态(1:插入，0:未插入); bit1-注网状态(1:已注册，0:未注册); bit5-拨号状态(1:成功，0:失败); bit6-IP状态(1:已设置，0:未设置); bit7-升级状态(1:在升级，0:未升级)
    uint8_t modemResetNum[4];		//模组复位次数
    uint8_t modemDialNum[4];		//模组拨号次数
    uint8_t modemHQnum[4];			//模组信号强度持续时间S
    uint8_t modemDialType;			//模组拨号类型 1:NonCDMA(4G 非CDMA) 2:CDMA(4G CDMA) 3:NR(5G-SA) 4:LTE(5G-LTE) 5:ENDC(5G-NSA) 6: WCDMA(5G-CDMA)
    uint8_t dialStatus;				//模组拨号状态 1: 拨号成功，0:拨号不成功
    uint8_t pingStatus;				//模组上网状态(ping mqtt域名地址连通性1S超时) 1：网络通， 0：网络不通 
    uint8_t mcc[4];					//MCC 信息
    uint8_t mnc[4];					//MNC 信息
    uint8_t cid[4];					//4G cid
    uint8_t lac[2];					//4G LAC
    uint8_t pcid5g[2];				//5G pcid
    uint8_t cid5g[8];				//5G cid
}SimCardInfoToMpu_t;

typedef struct
{
    uint8_t ICCID[20];				//ICCID 
    uint8_t signalSsi;				//信号强度
    uint8_t signalSnr;				//信噪比
    uint8_t	flow[4];				//流量消耗单位MB
    uint8_t modemVer[7];			//模组版本号 6-5: 项目号 4: 主版本号， 3-2：MPU 版本号, 1-0: MCU 版本号
    uint8_t modemSta;				//模组状态： bit0-SIM卡状态(1:插入，0:未插入); bit1-注网状态(1:已注册，0:未注册); bit5-拨号状态(1:成功，0:失败); bit6-IP状态(1:已设置，0:未设置); bit7-升级状态(1:在升级，0:未升级)
    uint8_t modemResetNum[4];		//模组复位次数
    uint8_t modemDialNum[4];		//模组拨号次数
    uint8_t modemHQnum[4];			//模组信号强度持续时间S
    uint8_t modemDialType;			//模组拨号类型 1:NonCDMA(4G 非CDMA) 2:CDMA(4G CDMA) 3:NR(5G-SA) 4:LTE(5G-LTE) 5:ENDC(5G-NSA) 6: WCDMA(5G-CDMA)
    uint8_t dialStatus;				//模组拨号状态 1: 拨号成功，0:拨号不成功
    uint8_t pingStatus;				//模组上网状态(ping mqtt域名地址连通性1S超时) 1：网络通， 0：网络不通 
    uint8_t mcc[4];					//MCC 信息
    uint8_t mnc[4];					//MNC 信息
    uint8_t cid[4];					//4G cid
    uint8_t lac[2];					//4G LAC
    uint8_t pcid5g[2];				//5G pcid
    uint8_t cid5g[8];				//5G cid
}SimCardInfoToMcu_t;

typedef struct
{
    uint8_t gateway;    //192.168.1.1
    uint8_t xxka;       //192.168.1.2
    uint8_t xxkb;       //192.168.1.3
    uint8_t onlineLanguageModule;       //192.168.1.8
    uint8_t bluetoothToEthernetModule;  //192.168.1.20
    uint8_t module1;    //192.168.1.201
    uint8_t module2;    //192.168.1.202      
}LocalNetState_t;





int GetVehicleCycleData(VehicleCycleData *vehicleCycleData);
int GetRsonCycleData(ResetRsonCycleData *rsonCycleData);
int GetVehicleStatus(VehicleStatus *vehicleData);

int GetMcuVehicleStatus(VehicleStatusMCu *mcuVehicleData);

void RecvMcuEvetDataHandleCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

int32_t InitSignalIpcCallback();

#ifdef __cplusplus
}
#endif

#endif
/**********************************end*****************************************/
