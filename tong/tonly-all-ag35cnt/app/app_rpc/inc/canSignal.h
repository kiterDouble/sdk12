#ifndef _STRUCT_CANSTRUCT_H
#define _STRUCT_CANSTRUCT_H

#include <stdint.h>

#pragma pack(1)
typedef struct
{
    uint8_t BCM_KeySt;//点火钥匙位置状态：0=Off, 1=Acc, 2=On, 3=Crank
    float BCS_VehSpd;//车速（km/h）：线性0.05625 km/h/bit，0~240
    uint16_t EMS_EngSpd;//发动机转速，分辨率1rpm/bit
    uint8_t GW_RemoteDiagCode;//需要远程诊断的故障等级：0=NO Fault，1=A DTC，2=B DTC，3=C DTC
    uint8_t BCS_VehSpdVD;//车速有效性：0=Not Valid，1=Valid
    uint8_t BCM_PGearSt;//P 挡状态：0=Not P gear, 1=P gear
    uint8_t FLDCM_RemoteDiagCode;//故障诊断代码（0=NO Fault，1=A DTC，2=B DTC，3=C DTC）
    uint8_t FRDCM_RemoteDiagCode;//故障诊断代码（0=NO Fault，1=A DTC，2=B DTC，3=C DTC）
    uint8_t BCM_RemoteDiagCode;//T-BOX 远程诊断代码：0=No Fault, 1=A DTC, 2=B DTC, 3=C DTC
} GacT08y1UdsSignal_canVal_t;//GAC_T08Y1
#define GacT08y1UdsSignal_canVal_t_period (1000)
#define GacT08y1UdsSignal_canVal_t_msgid (0x0001)

typedef struct
{
    uint8_t PLGM_Remotecontrolst;//遥控模式/退出原因：0=Default，1=Remote control mode，2=Vehicle speed/Gear error，3=Mode SW OFF，4=Spindle thermal protect，5=Hall failure，6=Voltage invalid，7=SpindleDriveFailure，8=BCM_RemoteControlSt!=1，9=LockFailure，10=Anti-Pinch，11~15=Reserved
    uint8_t HVACF_WindExitSpd;//风速级别：0=Off，1=Level 1，2=Level 2，3=Level 3，4=Level 4，5=Level 5，6=Level 6，7=Level 7，8~15=Not used
    uint8_t FRDCM_RRWinVentilateModeSt;//后右窗通风模式状态（0=Not actived，1=Actived）
    uint8_t FRDCM_FRWinVentilateModeSt;//前右窗通风模式状态（0=Not actived，1=Actived）
    uint8_t FLDCM_RLWinVentilateModeSt;//后左窗通风模式状态（0=Not actived，1=Actived）
    uint8_t FLDCM_FLWinVentilateModeSt;//前左窗通风模式状态（0=Not actived，1=Actived）
    uint8_t BCM_HazardLampSt;//双闪灯状态：0=Not active, 1=Active
    uint8_t BCM_TEL_HazLampCtrlSt;//双闪遥控（远程）控制状态：0=Normal, 1=Telematic
    float BCS_VehSpd;//车速（km/h）：线性0.05625 km/h/bit，0~240
    uint8_t BCS_VehSpdVD;//车速有效性：0=Not Valid，1=Valid
    uint8_t BCM_DriverDoorAjarSt;//主驾门开闭：0=Closed, 1=Open
    uint8_t BCM_PsngrDoorAjarSt;//副驾门开闭：0=Closed, 1=Open
    uint8_t BCM_RLDoorAjarSt;//左后门开闭：0=Closed, 1=Open
    uint8_t BCM_RRDoorAjarSt;//右后门开闭：0=Closed, 1=Open
    uint8_t BCM_KeySt;//点火钥匙位置状态：0=Off, 1=Acc, 2=On, 3=Crank
    uint8_t PLGM_ModeSW;//尾门模式按键：0=Not pressed，1=Pressed
    uint8_t EMS_EngSt;//发动机运行状态：0=Not run，1=Run
    uint8_t PEPS_RemoteControlSt;//远程控制模式状态：0=Not remote control mode，1=Remote control mode
    uint8_t PEPS_TELMode;//远程控制模式：0=Default，1=Mode 1，2=Mode 2，3=Reserved
    uint8_t BCM_ATWS_St;//防盗报警状态：0=Armed, 1=Prearmed, 2=Disarmed, 3=Not used, 4=Alarm, 5=Not used, 6=Not used, 7=Not used
    uint8_t EBS_SOC;//荷电状态SOC（%），分辨率1%/bit
    uint8_t BCM_RLSWarning_Rain;//雨量传感器自动雨刮告警：0=Not warning, 1=warning
    uint8_t BCM_BonnetAjarSt;//机舱盖开闭：0=Closed, 1=Open
    uint8_t BCM_SunRoofPstSt;//（前）天窗位置状态：0=Fully closed, 1=Slide open, 2=Tilt up, 3=Opening, 4=Closing, 5=Reversing, 6=Position not trusted, 7=Fully opened, 8=Comfort position, 9=Expanded slide open, 10~15=Not used
    uint8_t BCM_TrunkAjarSt;//后备箱开闭：0=Closed, 1=Open
    uint8_t FLDCM_FLWinSt;//前左窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FLDCM_RLWinSt;//后左窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FRDCM_FRWinSt;//前右窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FRDCM_RRWinSt;//后右窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint16_t ICM_AvaibleDriveDistance;//可续驶里程（线性：1 km/bit，偏移0）
    uint8_t PEPS_TeleFailReason;//远程控制失败原因：0=Default；1=not used；2=PEPS not learnt；3=not used；4=Tele option not enabled；5=Invalid tel mode；6=Speed invalid；7=Vehicle is moving；8=Brake pressed；9=AT不在P/MT离合或空挡失败；10=ATWS not ARM；11=任一车门开启；12=IG1输出故障；13=Starter输出故障；14=P挡故障(AT)；15=系统电源模式非OFF；16=远程电源模式非OFF；17=电压越界；18=SSB被按下；19=远程鉴权失败；20=EMS防盗失败；21=EMS无响应；22=ATWS报警；23=允许起动计时到期；24=最大鉴权计时到期；25=最大通电无起动计时到期；26=最大发动机运行计时到期；27=切换常规电源模式；28=远程请求关闭；29=当前模式不支持；30=复位导致退出；31=禁启功能不支持；32=禁启成功；33=允许启动车辆成功；34=无钥匙导致切换正常模式失败；35=ESCL Lock；53=收到多条命令；54=ATWS已布防；55=命令有效性CAN信号为0；56=SSB输入故障；57=机舱盖开启；58=后备箱打开；59=BCM超时；60=起动尝试超时；61=Not used；62=Engine stop；63=Not used
    uint8_t BCM_RemoteControlSt;//退出远程模式原因：0=Default, 1=Remote control mode, 2=Vehicle speed error, 3=Vehicle alarm error, 4=EMS alarm error, 5=Manual operation detected, 6=Authentication time out, 7=Authentication failure, 8=Voltage abnormal, 9=CrashEvent detected, 10=SBM message Received, 11=APA control event occurs, 12~15=Not used
    uint8_t PEPS_TeleAuthentSt;//远程鉴权状态：0=Default，1=Fail，2=Pass，3=Not used
} RemoteCtrlExtraInfoData_canVal_t;//远控附加信息数据结构体
#define RemoteCtrlExtraInfoData_canVal_t_period (1000)
#define RemoteCtrlExtraInfoData_canVal_t_msgid (0x0002)

typedef struct
{
    uint8_t GW_LowBatPreWarning;//低电量预警等级：0=off，1~7=level1~level7
    uint8_t TEL_ACU_GNSSclletAuthorMode;//the GNSS Collection authorization modeTBOX to ACU
    uint16_t TEL_ACU_GNSSclletAuthorStartYear;//the GNSS Collection authorization start year time TBOX to ACU
    uint8_t TEL_ACU_GNSSclletAuthorStartMon;//the GNSS Collection authorization start month time TBOX to ACU
    uint8_t TEL_ACU_GNSSclletAuthorStartDay;//the GNSS Collection authorization start day time TBOX to ACU
    uint16_t TEL_ACU_GNSSclletAuthorStopYear;//the GNSS Collection authorization stop year time TBOX to ACU
    uint16_t TEL_ACU_GNSSclletAuthorStopMon;//the GNSS Collection authorization stop month time TBOX to ACU
    uint16_t TEL_ACU_GNSSclletAuthorStopDay;//the GNSS Collection authorization stop day time TBOX to ACU
    uint8_t BCM_DriverDoorAjarSt;//主驾门开闭：0=Closed, 1=Open
    uint8_t BCM_PsngrDoorAjarSt;//副驾门开闭：0=Closed, 1=Open
    uint8_t BCM_RLDoorAjarSt;//左后门开闭：0=Closed, 1=Open
    uint8_t BCM_RRDoorAjarSt;//右后门开闭：0=Closed, 1=Open
    uint8_t BCM_DriverDoorLockSt;//主驾门锁：0=Unlock, 1=Lock
    uint8_t BCM_PsngrDoorLockSt;//副驾门锁：0=Unlock, 1=Lock
    uint8_t BCM_BonnetAjarSt;//机舱盖开闭：0=Closed, 1=Open
    uint8_t BCM_TrunkAjarSt;//后备箱开闭：0=Closed, 1=Open
    uint8_t BCM_SunRoofPstSt;//（前）天窗位置状态：0=Fully closed, 1=Slide open, 2=Tilt up, 3=Opening, 4=Closing, 5=Reversing, 6=Position not trusted, 7=Fully opened, 8=Comfort position, 9=Expanded slide open, 10~15=Not used
    uint8_t FLDCM_FLWinSt;//前左窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FRDCM_FRWinSt;//前右窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FLDCM_RLWinSt;//后左窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FRDCM_RRWinSt;//后右窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t HVACF_ACSt;//A/C状态显示：0=Not active，1=Active
    uint8_t HVSM_FLHeatingActLevel;//前左座椅加热实际档位：0=Off，1=Level 1，2=Level 2，3=Level 3，4~6=Not used，7=Error
    uint8_t HVSM_FRHeatingActLevel;//前右座椅加热实际档位：0=Off，1=Level 1，2=Level 2，3=Level 3，4~6=Not used，7=Error
    uint8_t HVSMR_RLHeatingActLevel;//后左座椅加热实际档位：0=Off，1=Level 1，2=Level 2，3=Level 3，4~6=Not used，7=Error
    uint8_t HVSMR_RRHeatingActLevel;//后右座椅加热实际档位：0=Off，1=Level 1，2=Level 2，3=Level 3，4~6=Not used，7=Error
    uint8_t HVSM_FLVentilatingActLevel;//前左座椅通风实际档位：0=Off，1=Level 1，2=Level 2，3=Level 3，4~6=Not used，7=Error
    uint8_t HVSM_FRVentilatingActLevel;//前右座椅通风实际档位：0=Off，1=Level 1，2=Level 2，3=Level 3，4~6=Not used，7=Error
    uint8_t HVSMR_RLVentilatingActLevel;//后左座椅通风实际档位：0=Off，1=Level 1，2=Level 2，3=Level 3，4~6=Not used，7=Error
    uint8_t HVSMR_RRVentilatingActLevel;//后右座椅通风实际档位：0=Off，1=Level 1，2=Level 2，3=Level 3，4~6=Not used，7=Error
    uint8_t EMS_EngSt;//发动机运行状态：0=Not run，1=Run
    uint8_t EBS_SOC;//荷电状态SOC（%），分辨率1%/bit
    float EBS_BattVolt;//蓄电池电压，分辨率(1/1024)V，偏移3V
    uint8_t EBS_SOC_STATE;//SOC状态：0=>15%偏差；1=<15%；2=<10%；3=无效
    uint8_t FRDCM_NotSleepSource1;//不休眠源1（bit0=CAN报文标志：0=Not active，1=Active）
    uint8_t FRDCM_NotSleepSource2;//不休眠源2（bit0=右窗电机热保护：0=Not active/1=Active；bit1=紧急模式：0=Not active/1=Active）
    uint8_t FLDCM_NotSleepSource1;//不休眠源1（bit0=CAN报文标志：0=Not active，1=Active）
    uint8_t FLDCM_NotSleepSource2;//不休眠源2（bit0=左窗电机热保护：0=Not active/1=Active；bit1=紧急模式：0=Not active/1=Active）
    uint8_t MFS_NotSleepWarning;//未休眠告警：0=默认，1=未休眠，2=频繁唤醒，3=未休眠且频繁唤醒
    uint8_t MFS_NotSleepSource;//未休眠告警来源（bit0）：0=未激活，1=激活
    uint8_t BCM_KeySt;//点火钥匙位置状态：0=Off, 1=Acc, 2=On, 3=Crank
    uint16_t ICM_VehRunningTimeTemp;//once Vehicle running time
    uint8_t ICM_FuelAvgConsumeTemp;//临时平均油耗（线性：0.1 L/100km/bit，偏移0）
    uint32_t ICM_TotalOdometer;//总里程（线性：1 km/bit，偏移0）
} EventNotifyData_canVal_t;//事件通知数据结构体
#define EventNotifyData_canVal_t_period (1000)
#define EventNotifyData_canVal_t_msgid (0x0003)

typedef struct
{
    uint8_t BCM_IG1St;//IG1 点火状态：0=Not active, 1=Active
    float EBS_BattVolt;//蓄电池电压，分辨率(1/1024)V，偏移3V
    uint8_t EBS_SOC;//荷电状态SOC（%），分辨率1%/bit
    float BCS_VehSpd;//车速（km/h）：线性0.05625 km/h/bit，0~240
    uint8_t BCS_VehSpdVD;//车速有效性：0=Not Valid，1=Valid
    uint8_t EMS_GearNeutralPst;//（MT）空挡位置：0=其他档位，1=空挡
    uint8_t EMS_GearNeutralPstVD;//（MT）空挡位置有效性：0=Not valid，1=Valid
    uint8_t TCU_GearForDisp;//档位显示：0=Invalid，1~9=1~9挡，10=No connection area，11=Sport，12=Drive，13=Park，14=Reverse，15=Neutral
    uint8_t BCM_ATWS_St;//防盗报警状态：0=Armed, 1=Prearmed, 2=Disarmed, 3=Not used, 4=Alarm, 5=Not used, 6=Not used, 7=Not used
} OtaMasterCanData_canVal_t;//ota master组件需要使用到的can数据
#define OtaMasterCanData_canVal_t_period (1000)
#define OtaMasterCanData_canVal_t_msgid (0x0004)

typedef struct
{
    uint8_t BCM_DriverDoorLockSt;//主驾门锁：0=Unlock, 1=Lock
    uint8_t BCM_PsngrDoorLockSt;//副驾门锁：0=Unlock, 1=Lock
    uint8_t BCM_DriverDoorAjarSt;//主驾门开闭：0=Closed, 1=Open
    uint8_t BCM_PsngrDoorAjarSt;//副驾门开闭：0=Closed, 1=Open
    uint8_t BCM_RLDoorAjarSt;//左后门开闭：0=Closed, 1=Open
    uint8_t BCM_RRDoorAjarSt;//右后门开闭：0=Closed, 1=Open
    uint8_t FRDCM_RRWinSt;//后右窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FRDCM_RRWinVentilateModeSt;//后右窗通风模式状态（0=Not actived，1=Actived）
    uint8_t FLDCM_RLWinSt;//后左窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FLDCM_RLWinVentilateModeSt;//后左窗通风模式状态（0=Not actived，1=Actived）
    uint8_t FRDCM_FRWinSt;//前右窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FRDCM_FRWinVentilateModeSt;//前右窗通风模式状态（0=Not actived，1=Actived）
    uint8_t FLDCM_FLWinSt;//前左窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FLDCM_FLWinVentilateModeSt;//前左窗通风模式状态（0=Not actived，1=Actived）
    uint8_t BCM_ATWS_St;//防盗报警状态：0=Armed, 1=Prearmed, 2=Disarmed, 3=Not used, 4=Alarm, 5=Not used, 6=Not used, 7=Not used
    uint8_t BCM_BonnetAjarSt;//机舱盖开闭：0=Closed, 1=Open
    uint8_t BCM_TrunkAjarSt;//后备箱开闭：0=Closed, 1=Open
    uint8_t BCM_SunRoofPstSt;//（前）天窗位置状态：0=Fully closed, 1=Slide open, 2=Tilt up, 3=Opening, 4=Closing, 5=Reversing, 6=Position not trusted, 7=Fully opened, 8=Comfort position, 9=Expanded slide open, 10~15=Not used
    uint8_t TPMS_ParkSupervisionSt;//TPMS驻车监管开关：0=ON，1=OFF
    uint8_t TPMS_WakeUpByLowPres;//驻车监管因“低压”唤醒标志：0=未被TPMS唤醒，1=被TPMS因低压唤醒
    uint8_t TPMS_WakeUpBySnsFail;//驻车监管因“传感器故障”唤醒标志：0=未被TPMS唤醒，1=被TPMS因传感器故障唤醒
    uint8_t PEPS_IdInVehicleWarning;//关最后一扇门后检测到车内有钥匙、轮询条件不满足：0=Not warning，1=Warning
    uint8_t BCM_RLSAutoSRWinCloseResult;//雨天自动关窗/天窗结果：0=No command, 1=Fail, 2=Success, 3=Not used
    uint8_t BCM_AlarmHazardLampSt;//报警双闪状态（T-Box）：0=normal，1=Active
} AbnormalAlarmData_canVal_t;//异常告警数据结构体
#define AbnormalAlarmData_canVal_t_period (1000)
#define AbnormalAlarmData_canVal_t_msgid (0x0005)

typedef struct
{
    uint64_t GWM_NMDataField;//GWM 网络管理数据字段（56 位原始值）
    uint8_t BCM_KeySt;//点火钥匙位置状态：0=Off, 1=Acc, 2=On, 3=Crank
    float EMS_EngTorq;//当前指示发动机扭矩（%），分辨率0.392%/bit
    uint16_t EMS_EngSpd;//发动机转速，分辨率1rpm/bit
    int16_t EMS_EngWaterTemp;//发动机冷却液温度，分辨率1℃/bit，偏移-40℃
    uint8_t TCU_GearForDisp;//档位显示：0=Invalid，1~9=1~9挡，10=No connection area，11=Sport，12=Drive，13=Park，14=Reverse，15=Neutral
    float BCS_VehSpd;//车速（km/h）：线性0.05625 km/h/bit，0~240
    uint8_t BCS_VehSpdVD;//车速有效性：0=Not Valid，1=Valid
    float EMS_GasPedalActPst;//当前加速踏板实际位置（%），分辨率0.392%/bit
    uint8_t BCM_DriverDoorAjarSt;//主驾门开闭：0=Closed, 1=Open
    uint8_t BCM_PsngrDoorAjarSt;//副驾门开闭：0=Closed, 1=Open
    uint8_t BCM_RLDoorAjarSt;//左后门开闭：0=Closed, 1=Open
    uint8_t BCM_RRDoorAjarSt;//右后门开闭：0=Closed, 1=Open
    uint8_t BCM_BonnetAjarSt;//机舱盖开闭：0=Closed, 1=Open
    uint8_t BCM_TrunkAjarSt;//后备箱开闭：0=Closed, 1=Open
    uint8_t BCM_SunRoofPstSt;//（前）天窗位置状态：0=Fully closed, 1=Slide open, 2=Tilt up, 3=Opening, 4=Closing, 5=Reversing, 6=Position not trusted, 7=Fully opened, 8=Comfort position, 9=Expanded slide open, 10~15=Not used
    uint8_t BCM_ParkingLampSt;//示宽灯：0=Not active, 1=Active
    float HVACF_CorrectedExterTemp;//外界温度（修正），线性：0.5℃/bit，偏移-40℃
    float ICM_FuelLevel;//燃油液位（线性：0.5%/bit，偏移0）
    uint32_t ICM_TotalOdometer;//总里程（线性：1 km/bit，偏移0）
    uint8_t ICM_OilPrSt;//机油压力信号（0=High level，1=Low level）
    uint8_t ICM_BFLWarning;//制动液位报警（0=No warning，1=warning）
    float ICM_FuelAverageConsumed;//平均油耗（线性：0.1 L/100km/bit，偏移0）
    uint16_t ICM_AvaibleDriveDistance;//可续驶里程（线性：1 km/bit，偏移0）
    uint8_t TPMS_FLTyreWarn;//前左胎告警位：bit0低压(0正常/1低压)，bit1高压(0正常/1高压)，bit2低温(0正常/1低温)，bit3高温(0正常/1高温)，bit4压力突变(0无/1有)，bit5传感器故障(0无/1有)，bit6标定状态(0已标定/1未标定)，bit7静音(0否/1是)
    uint8_t TPMS_FRTyreWarn;//前右胎告警位：bit0低压，bit1高压，bit2低温，bit3高温，bit4压力突变，bit5传感器故障，bit6标定状态，bit7静音（0为正常/否，1为异常/是）
    uint8_t TPMS_RLTyreWarn;//后左胎告警位：bit0低压，bit1高压，bit2低温，bit3高温，bit4压力突变，bit5传感器故障，bit6标定状态，bit7静音（0为正常/否，1为异常/是）
    uint8_t TPMS_RRTyreWarn;//后右胎告警位：bit0低压，bit1高压，bit2低温，bit3高温，bit4压力突变，bit5传感器故障，bit6标定状态，bit7静音（0为正常/否，1为异常/是）
    uint8_t SRS_AirbagFaultSt;//安全气囊故障指示：0=Not fault，1=Fault
    float TPMS_FLTyrePr;//前左胎压力（Linear：1.373 kPa/bit，偏移0；0xFF=等待：车速不足或此前未收到有效信号）
    float TPMS_FRTyrePr;//前右胎压力（Linear：1.373 kPa/bit，偏移0；0xFF=等待：车速不足或此前未收到有效信号）
    float TPMS_RLTyrePr;//后左胎压力（Linear：1.373 kPa/bit，偏移0；0xFF=等待：车速不足或此前未收到有效信号）
    float TPMS_RRTyrePr;//后右胎压力（Linear：1.373 kPa/bit，偏移0；0xFF=等待：车速不足或此前未收到有效信号）
    int8_t TPMS_FLTyreTemp;//前左胎温度（Linear：1℃/bit，偏移-50℃；0xFF=等待：车速不足或此前未收到有效信号）
    int8_t TPMS_FRTyreTemp;//前右胎温度（Linear：1℃/bit，偏移-50℃；0xFF=等待：车速不足或此前未收到有效信号）
    int8_t TPMS_RLTyreTemp;//后左胎温度（Linear：1℃/bit，偏移-50℃；0xFF=等待：车速不足或此前未收到有效信号）
    int8_t TPMS_RRTyreTemp;//后右胎温度（Linear：1℃/bit，偏移-50℃；0xFF=等待：车速不足或此前未收到有效信号）
    uint8_t BCM_DriverDoorLockSt;//主驾门锁：0=Unlock, 1=Lock
    uint8_t BCM_PsngrDoorLockSt;//副驾门锁：0=Unlock, 1=Lock
    uint8_t BCM_WinMoveSt;//车窗移动状态：0=Stopped, 1=Window closing, 2=Window opening, 3=Invalid
    uint8_t ICM_FuelLevelLowSt;//燃油低液位（0=Fuel level normal，1=Fuel level low）
    uint16_t HVACF_PM25Value;//PM2.5（μg/m³），线性：1/bit，偏移0
    uint16_t HVACF_PM10Value;//PM10（μg/m³），线性：1/bit，偏移0
    uint8_t HVACF_PM25ValueVD;//PM2.5数值有效性：0=Not valid，1=Valid
    uint8_t HVACF_PM10ValueVD;//PM10数值有效性：0=Not valid，1=Valid
    uint8_t HVACF_PM25Sensormaintaininfo;//PM2.5传感器维护信息：0=Not active，1=Active
    uint8_t HVACF_PM25SensormaintaininfoVD;//PM2.5传感器维护信息有效性：0=Not valid，1=Valid
    uint8_t HVACF_PM25Sns;//是否带PM2.5传感器：0=Not have，1=Have
    uint8_t GW_LowBatPreWarning;//低电量预警等级：0=off，1~7=level1~level7
    uint8_t FLDCM_FLWinSt;//前左窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FLDCM_RLWinSt;//后左窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FRDCM_FRWinSt;//前右窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t FRDCM_RRWinSt;//后右窗状态（0=Undefined，1=Fully closed，2=Between，3=Fully opened）
    uint8_t EBS_SOC;//荷电状态SOC（%），分辨率1%/bit
    uint8_t EBS_SOC_STATE;//SOC状态：0=>15%偏差；1=<15%；2=<10%；3=无效
    uint8_t PLGM_Doorst;//尾门状态：00=StatusUnknown，01=FullyOpened，02=FullyClosed，03=Opening，04=Closing，05=Hold，06=Brake，07=Reserved
    float HVACF_RawCabinTemp;//车内温度（原始），线性：0.5℃/bit，偏移-40℃
    uint8_t HVACF_CorrectedExterTempVD;//外界温度（修正）有效性：0=Not valid，1=Valid
} RemoteMonitorCanData_canVal_t;//远程监控can数据
#define RemoteMonitorCanData_canVal_t_period (1000)
#define RemoteMonitorCanData_canVal_t_msgid (0x0006)

typedef struct
{
    float BCS_VehSpd;//车速（km/h）：线性0.05625 km/h/bit，0~240
    uint8_t SRS_CrashOutputSt;//近2秒是否发生碰撞事件：0=No crash，0xA0(维持2s)=Crash happen in last 2s
    uint8_t SRS_NearCrashOutputSt;//近似碰撞事件输出：0=No crash，1(维持2s)=Crash happen in last 2s
} XcallMsdCanData_canVal_t;//Xcall上传MSD数据需要用到的信号
#define XcallMsdCanData_canVal_t_period (100)
#define XcallMsdCanData_canVal_t_msgid (0x0007)

typedef struct
{
    uint8_t ACU_TEL_GNSSclletAuthorStartMonReq;//GNSS采集授权开始月份时间请求，从ACU到TBOX。值域：0到12，表示月份，0=无效，1到12为有效的月份。
    uint8_t ACU_TEL_GNSSclletAuthorStartDayReq;//GNSS采集授权开始日期时间请求，从ACU到TBOX。值域：0到31，表示日期，0=无效，1到31为有效的日期。
    uint8_t ACU_TEL_GNSSclletAuthorStopMonReq;//GNSS采集授权停止月份时间请求，从ACU到TBOX。值域：0到12，表示月份，0=无效，1到12为有效的月份。
    uint8_t ACU_TEL_GNSSclletAuthorStopDayReq;//GNSS采集授权停止日期时间请求，从ACU到TBOX。值域：0到31，表示日期，0=无效，1到31为有效的日期。
    uint16_t ACU_TEL_GNSSclletAuthorStartYearReq;//GNSS采集授权开始年份时间请求，从ACU到TBOX。值域：2000到2255，表示年份，2000到2255为有效的年份。
    uint16_t ACU_TEL_GNSSclletAuthorStopYearReq;//GNSS采集授权停止年份时间请求，从ACU到TBOX。值域：2000到2255，表示年份，2000到2255为有效的年份。
    uint8_t ACU_TEL_GNSSclletAuthorModeReq;//GNSS采集授权模式请求，从ACU到TBOX。值域：0到3，表示授权模式，0=保留，1=GNSS采集授权禁止请求，2=GNSS采集授权在有限时间内请求，3=GNSS采集授权在永久时间内请求。
    uint32_t ACU_TEL_GNSS_APPTypeStSetReq;//TBOX GNSS采集APP类型状态设置请求，从ACU到TBOX。值域：0到16777216，每个位表示一种APP类型，0=禁用，1=启用。仅在ACU_TBOX_GNSSCollectionAuthorizationMode为1时有效。
} SensitiveDataSettingCanData_canVal_t;//敏感数据设置
#define SensitiveDataSettingCanData_canVal_t_period (1000)
#define SensitiveDataSettingCanData_canVal_t_msgid (0x0008)

#pragma pack()

#endif
