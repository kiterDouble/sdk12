#ifndef _STRUCT_CANSTRUCT_H
#define _STRUCT_CANSTRUCT_H

#include <stdint.h>

#pragma pack(1)
typedef struct
{
    uint8_t vehicle_status;//车辆状态(CANID-0x4C5 StartBit-11 BitLen-2)
    uint8_t charge_discharge_status;//充放电状态(CANID-0x155 StartBit-5 BitLen-4)
    float vehicle_speed;//车速-单位km/h(CANID-0x36A StartBit-22 BitLen-15)
    float distance_travelled;//里程-单位km(CANID-0x120 StartBit-7 BitLen-32)
    float overall_voltage;//总电压-单位V(CANID-0x29a StartBit-4 BitLen-13)
    float overall_current;//总电流-单位A(CANID-0x287 StartBit-6 BitLen-15)
    uint8_t SOC;//SOC-单位%(CANID-0x36A StartBit-15 BitLen-8)
    uint8_t DCDC_status;//DC-DC状态(CANID-0x4C7 StartBit-6 BitLen-1)
    uint8_t gear;//档位(CANID-0x155 StartBit-35 BitLen-3)
    uint16_t insulation_ohms;//绝缘电阻-单位kΩ(CANID-0x287 StartBit-39 BitLen-16)
    uint8_t accelerator_pedal_travel;//加速踏板行程值-单位%(CANID-0x155 StartBit-23 BitLen-8)
    uint8_t brake_pedal_travel;//制动踏板行程值(CANID-0x155 StartBit-13 BitLen-1)
} NationalVehicleData_canVal_t;//国标整车数据结构体 根据Sitech_e50_mce.json与通信协议.xlsx来定义的结构体
#define NationalVehicleData_canVal_t_period (1000)
#define NationalVehicleData_canVal_t_msgid (0x00110001)

typedef struct
{
    uint8_t motor_0_state;//电机[0]状态
    int16_t drive_motor_0_controller_temperature;//驱动电机[0]控制器温度 ℃
    int16_t drive_motor_0_speed;//驱动电机[0]转速 RPM
    float drive_motor_0_torque;//驱动电机[0]转矩 Nm
    int16_t drive_motor_0_temperature;//驱动电机[0]温度 ℃
    uint16_t motor_controller_0_input_voltage;//电机控制器[0]输入电压 V
    int16_t motor_controller_0_input_current;//电机控制器[0]输入电流 A
} NationalMotorData_canVal_t;//国标电机数据结构体 根据Sitech_e50_mce.json与通信协议.xlsx来定义的结构体
#define NationalMotorData_canVal_t_period (1000)
#define NationalMotorData_canVal_t_msgid (0x00110002)

typedef struct
{
    uint8_t highest_voltage_battery_cell_id;//最高电压电池单体代号
    uint16_t highest_cell_voltage;//电池单体电压最高值 mV
    uint8_t lowest_voltage_battery_cell_id;//最低电压电池单体代号
    uint16_t lowest_cell_voltage;//电池单体电压最低值 mV
    uint8_t highest_probe_index;//最高温度探针序号
    int16_t highest_temperature_value;//最高温度值 ℃
    uint8_t lowest_probe_index;//最低温度探针序号
    int16_t lowest_temperature_value;//最低温度值 ℃
} NationalExtremeData_canVal_t;//国标极值数据结构体 根据Sitech_e50_mce.json与通信协议.xlsx来定义的结构体
#define NationalExtremeData_canVal_t_period (1000)
#define NationalExtremeData_canVal_t_msgid (0x00110003)

typedef struct
{
    uint8_t temperature_difference_alarm;//温度差异性报警 电池高温报警
    uint8_t overvoltage_energy_storage_alarm;//车载储能装置类型过压报警 车载储能装置类型欠压报警
    uint8_t low_SOC_alarm;//SOC低报警 SOC过高报警
    uint8_t overvoltage_single_cell_alarm;//单体电池过压报警 单体电池欠压报警 电池单体一致性差报警 车载储能装置类型过充
    uint8_t insulation_alarm;//绝缘报警
    uint8_t DCDC_temperature_alarm;//DCDC温度报警
    uint8_t Braking_status_alarm;//制动状态报警
    uint8_t DCDC_status_alarm;//DCDC状态报警
    uint8_t drive_motor_controller_temperature_alarm;//驱动电机控制器温度报警
    uint8_t high_voltage_interlock_status_alarm;//高压互锁状态报警
    uint8_t drive_motor_temperature_alarm;//驱动电机温度报警
} NationalAlarmData_canVal_t;//国标报警数据结构体 根据Sitech_e50_mce.json与通信协议.xlsx来定义的结构体
#define NationalAlarmData_canVal_t_period (1000)
#define NationalAlarmData_canVal_t_msgid (0x00110004)

typedef struct
{
    float rechargeable_battery_pack_voltage;//可充电储能装置电压
    float rechargeable_battery_pack_current;//可充电储能装置电流
    float cell_voltage_1;//单体电池电压[1]
    float cell_voltage_2;//单体电池电压[2]
    float cell_voltage_3;//单体电池电压[3]
    float cell_voltage_4;//单体电池电压[4]
    float cell_voltage_5;//单体电池电压[5]
    float cell_voltage_6;//单体电池电压[6]
    float cell_voltage_7;//单体电池电压[7]
    float cell_voltage_8;//单体电池电压[8]
    float cell_voltage_9;//单体电池电压[9]
    float cell_voltage_10;//单体电池电压[10]
    float cell_voltage_11;//单体电池电压[11]
    float cell_voltage_12;//单体电池电压[12]
    float cell_voltage_13;//单体电池电压[13]
    float cell_voltage_14;//单体电池电压[14]
    float cell_voltage_15;//单体电池电压[15]
    float cell_voltage_16;//单体电池电压[16]
    float cell_voltage_17;//单体电池电压[17]
    float cell_voltage_18;//单体电池电压[18]
    float cell_voltage_19;//单体电池电压[19]
    float cell_voltage_20;//单体电池电压[20]
    float cell_voltage_21;//单体电池电压[21]
    float cell_voltage_22;//单体电池电压[22]
    float cell_voltage_23;//单体电池电压[23]
    float cell_voltage_24;//单体电池电压[24]
    float cell_voltage_25;//单体电池电压[25]
    float cell_voltage_26;//单体电池电压[26]
    float cell_voltage_27;//单体电池电压[27]
    float cell_voltage_28;//单体电池电压[28]
    float cell_voltage_29;//单体电池电压[29]
    float cell_voltage_30;//单体电池电压[30]
    float cell_voltage_31;//单体电池电压[31]
    float cell_voltage_32;//单体电池电压[32]
    float cell_voltage_33;//单体电池电压[33]
    float cell_voltage_34;//单体电池电压[34]
    float cell_voltage_35;//单体电池电压[35]
    float cell_voltage_36;//单体电池电压[36]
    float cell_voltage_37;//单体电池电压[37]
    float cell_voltage_38;//单体电池电压[38]
    float cell_voltage_39;//单体电池电压[39]
    float cell_voltage_40;//单体电池电压[40]
    float cell_voltage_41;//单体电池电压[41]
    float cell_voltage_42;//单体电池电压[42]
    float cell_voltage_43;//单体电池电压[43]
    float cell_voltage_44;//单体电池电压[44]
    float cell_voltage_45;//单体电池电压[45]
    float cell_voltage_46;//单体电池电压[46]
    float cell_voltage_47;//单体电池电压[47]
    float cell_voltage_48;//单体电池电压[48]
    float cell_voltage_49;//单体电池电压[49]
    float cell_voltage_50;//单体电池电压[50]
    float cell_voltage_51;//单体电池电压[51]
    float cell_voltage_52;//单体电池电压[52]
    float cell_voltage_53;//单体电池电压[53]
    float cell_voltage_54;//单体电池电压[54]
    float cell_voltage_55;//单体电池电压[55]
    float cell_voltage_56;//单体电池电压[56]
    float cell_voltage_57;//单体电池电压[57]
    float cell_voltage_58;//单体电池电压[58]
    float cell_voltage_59;//单体电池电压[59]
    float cell_voltage_60;//单体电池电压[60]
    float cell_voltage_61;//单体电池电压[61]
    float cell_voltage_62;//单体电池电压[22]
    float cell_voltage_63;//单体电池电压[63]
    float cell_voltage_64;//单体电池电压[64]
    float cell_voltage_65;//单体电池电压[65]
    float cell_voltage_66;//单体电池电压[66]
    float cell_voltage_67;//单体电池电压[67]
    float cell_voltage_68;//单体电池电压[68]
    float cell_voltage_69;//单体电池电压[69]
    float cell_voltage_70;//单体电池电压[70]
    float cell_voltage_71;//单体电池电压[71]
    float cell_voltage_72;//单体电池电压[72]
    float cell_voltage_73;//单体电池电压[73]
    float cell_voltage_74;//单体电池电压[74]
    float cell_voltage_75;//单体电池电压[75]
    float cell_voltage_76;//单体电池电压[76]
    float cell_voltage_77;//单体电池电压[77]
    float cell_voltage_78;//单体电池电压[78]
    float cell_voltage_79;//单体电池电压[79]
    float cell_voltage_80;//单体电池电压[80]
    float cell_voltage_81;//单体电池电压[81]
    float cell_voltage_82;//单体电池电压[82]
    float cell_voltage_83;//单体电池电压[83]
    float cell_voltage_84;//单体电池电压[84]
    float cell_voltage_85;//单体电池电压[85]
    float cell_voltage_86;//单体电池电压[86]
    float cell_voltage_87;//单体电池电压[87]
    float cell_voltage_88;//单体电池电压[88]
    float cell_voltage_89;//单体电池电压[89]
    float cell_voltage_90;//单体电池电压[90]
    float cell_voltage_91;//单体电池电压[91]
    float cell_voltage_92;//单体电池电压[92]
    float cell_voltage_93;//单体电池电压[93]
    float cell_voltage_94;//单体电池电压[94]
    float cell_voltage_95;//单体电池电压[95]
    float cell_voltage_96;//单体电池电压[96]
    float cell_voltage_97;//单体电池电压[97]
    float cell_voltage_98;//单体电池电压[98]
    float cell_voltage_99;//单体电池电压[99]
    float cell_voltage_100;//单体电池电压[100]
    float cell_voltage_101;//单体电池电压[101]
    float cell_voltage_102;//单体电池电压[102]
    float cell_voltage_103;//单体电池电压[103]
    float cell_voltage_104;//单体电池电压[104]
    float cell_voltage_105;//单体电池电压[105]
    float cell_voltage_106;//单体电池电压[106]
    float cell_voltage_107;//单体电池电压[107]
    float cell_voltage_108;//单体电池电压[108]
    float cell_voltage_109;//单体电池电压[109]
    float cell_voltage_110;//单体电池电压[110]
    float cell_voltage_111;//单体电池电压[111]
    float cell_voltage_112;//单体电池电压[112]
    float cell_voltage_113;//单体电池电压[113]
    float cell_voltage_114;//单体电池电压[114]
    float cell_voltage_115;//单体电池电压[115]
    float cell_voltage_116;//单体电池电压[116]
    float cell_voltage_117;//单体电池电压[117]
    float cell_voltage_118;//单体电池电压[118]
    float cell_voltage_119;//单体电池电压[119]
    float cell_voltage_120;//单体电池电压[120]
} NationalBatteryVoltageData_canVal_t;//国标动力电池电压数据结构体 根据Sitech_e50_mce.json与通信协议.xlsx来定义的结构体
#define NationalBatteryVoltageData_canVal_t_period (1000)
#define NationalBatteryVoltageData_canVal_t_msgid (0x00110005)

typedef struct
{
    int16_t battery_cell_temp_1;//探针温度值1
    int16_t battery_cell_temp_2;//探针温度值2
    int16_t battery_cell_temp_3;//探针温度值3
    int16_t battery_cell_temp_4;//探针温度值4
    int16_t battery_cell_temp_5;//探针温度值5
    int16_t battery_cell_temp_6;//探针温度值6
    int16_t battery_cell_temp_7;//探针温度值7
    int16_t battery_cell_temp_8;//探针温度值8
    int16_t battery_cell_temp_9;//探针温度值9
    int16_t battery_cell_temp_10;//探针温度值10
    int16_t battery_cell_temp_11;//探针温度值11
    int16_t battery_cell_temp_12;//探针温度值12
    int16_t battery_cell_temp_13;//探针温度值13
    int16_t battery_cell_temp_14;//探针温度值14
    int16_t battery_cell_temp_15;//探针温度值15
    int16_t battery_cell_temp_16;//探针温度值16
    int16_t battery_cell_temp_17;//探针温度值17
    int16_t battery_cell_temp_18;//探针温度值18
    int16_t battery_cell_temp_19;//探针温度值19
    int16_t battery_cell_temp_20;//探针温度值20
    int16_t battery_cell_temp_21;//探针温度值21
    int16_t battery_cell_temp_22;//探针温度值22
    int16_t battery_cell_temp_23;//探针温度值23
    int16_t battery_cell_temp_24;//探针温度值24
    int16_t battery_cell_temp_25;//探针温度值25
    int16_t battery_cell_temp_26;//探针温度值26
    int16_t battery_cell_temp_27;//探针温度值27
    int16_t battery_cell_temp_28;//探针温度值28
    int16_t battery_cell_temp_29;//探针温度值29
    int16_t battery_cell_temp_30;//探针温度值30
    int16_t battery_cell_temp_31;//探针温度值31
    int16_t battery_cell_temp_32;//探针温度值32
    int16_t battery_cell_temp_33;//探针温度值33
    int16_t battery_cell_temp_34;//探针温度值34
    int16_t battery_cell_temp_35;//探针温度值35
    int16_t battery_cell_temp_36;//探针温度值36
    int16_t battery_cell_temp_37;//探针温度值37
    int16_t battery_cell_temp_38;//探针温度值38
    int16_t battery_cell_temp_39;//探针温度值39
    int16_t battery_cell_temp_40;//探针温度值40
    int16_t battery_cell_temp_41;//探针温度值17
    int16_t battery_cell_temp_42;//探针温度值42
    int16_t battery_cell_temp_43;//探针温度值43
    int16_t battery_cell_temp_44;//探针温度值44
    int16_t battery_cell_temp_45;//探针温度值45
    int16_t battery_cell_temp_46;//探针温度值46
    int16_t battery_cell_temp_47;//探针温度值47
    int16_t battery_cell_temp_48;//探针温度值48
} NationalBatCelTempData_canVal_t;//国标动力蓄电池包温度数据结构体 根据Sitech_e50_mce.json与通信协议.xlsx来定义的结构体
#define NationalBatCelTempData_canVal_t_period (1000)
#define NationalBatCelTempData_canVal_t_msgid (0x00110006)

typedef struct
{
    uint8_t data1000;//
    uint8_t data1001;//
    uint8_t data1002;//
    float data1003;//
    uint8_t data1004;//
    float LowBatVol;//低压蓄电池电压
    uint8_t BatFltLvl;//电池包告警级别
    uint8_t VehRdySts;//车辆启动状态g_vehicalStatusValueB
    uint8_t data1008;//
    uint8_t VecChrgRqst;//充电请求
    uint8_t CCSts;//慢充充电连接状态
    uint8_t data1011;//
    uint8_t data1012;//
    uint8_t data1013;//
    uint8_t BatSOC;//电池电量
    uint8_t VecFltIndOn;//整车故障指示
    uint8_t DCDCInternalTemp;//直流/直流变换器内部温度
    uint8_t DrivStOccSt;//主驾驶座占用状态
    uint8_t BatOthFltNum;//动力蓄电池其他故障总数
} Data_1000_1019_canVal_t;//自定义数据结构体 根据Sitech_e50_mce.json 数据项1000-1019定义的结构体
#define Data_1000_1019_canVal_t_period (1000)
#define Data_1000_1019_canVal_t_msgid (0x00110007)

typedef struct
{
    uint8_t can_0X284[12];
    uint8_t can_0X287[12];
    uint8_t can_0X297[12];
    uint8_t can_0X28C[12];
    uint8_t can_0X28B[12];
    uint8_t can_0X29A[12];
    uint8_t can_0X29B[12];
    uint8_t can_0X295[12];
    uint8_t can_0X289[12];
    uint8_t can_0X29E[12];
    uint8_t can_0X5D6[12];
    uint8_t can_0X5C6[12];
    uint8_t can_0X5C7[12];
    uint8_t can_0X5C8[12];
    uint8_t can_0X5C9[12];
    uint8_t can_0X5CA[12];
    uint8_t can_0X5D8[12];
    uint8_t can_0X59F[12];
    uint8_t can_0X5B0[12];
    uint8_t can_0X5B1[12];
    uint8_t can_0X5B2[12];
    uint8_t can_0X5B3[12];
    uint8_t can_0X5B4[12];
    uint8_t can_0X5B5[12];
    uint8_t can_0X5B6[12];
    uint8_t can_0X5B7[12];
    uint8_t can_0X5B8[12];
    uint8_t can_0X5A1[12];
    uint8_t can_0X5A2[12];
    uint8_t can_0X5A3[12];
    uint8_t can_0X5A4[12];
    uint8_t can_0X5A5[12];
    uint8_t can_0X5A6[12];
    uint8_t can_0X5A7[12];
    uint8_t can_0X5A8[12];
    uint8_t can_0X5A9[12];
    uint8_t can_0X5AA[12];
    uint8_t can_0X5AB[12];
    uint8_t can_0X5AC[12];
    uint8_t can_0X5AD[12];
    uint8_t can_0X5AE[12];
    uint8_t can_0X5AF[12];
    uint8_t can_0X5BB[12];
    uint8_t can_0X5BC[12];
    uint8_t can_0X5BD[12];
    uint8_t can_0X5BE[12];
    uint8_t can_0X5BF[12];
    uint8_t can_0X5C0[12];
    uint8_t can_0X5D9[12];
    uint8_t can_0X5DA[12];
    uint8_t can_0X5DB[12];
    uint8_t can_0X5DC[12];
    uint8_t can_0X58A[12];
    uint8_t can_0X58B[12];
    uint8_t can_0X58C[12];
    uint8_t can_0X58D[12];
    uint8_t can_0X58E[12];
    uint8_t can_0X59D[12];
    uint8_t can_0X4A1[12];
    uint8_t can_0X3C4[12];
    uint8_t can_0X652[12];
    uint8_t can_0X165[12];
    uint8_t can_0X3C1[12];
    uint8_t can_0X4C6[12];
    uint8_t can_0X271[12];
    uint8_t can_0X155[12];
    uint8_t can_0X4C5[12];
    uint8_t can_0X36A[12];
    uint8_t can_0X4C7[12];
    uint8_t can_0X36D[12];
    uint8_t can_0X36E[12];
    uint8_t can_0X651[12];
    uint8_t can_0X16C[12];
    uint8_t can_0X16E[12];
    uint8_t can_0X2AE[12];
    uint8_t can_0X4B2[12];
    uint8_t can_0X592[12];
    uint8_t can_0X593[12];
    uint8_t can_0X653[12];
    uint8_t can_0X4F6[12];
    uint8_t can_0X486[12];
    uint8_t can_0X4D5[12];
    uint8_t can_0X5C1[12];
    uint8_t can_0X361[12];
    uint8_t can_0X594[12];
    uint8_t can_0X654[12];
    uint8_t can_0X120[12];
    uint8_t can_0X565[12];
    uint8_t can_0X12A[12];
    uint8_t can_0X3C9[12];
    uint8_t can_0X57E[12];
    uint8_t can_0X279[12];
    uint8_t can_0X335[12];
    uint8_t can_0X1E9[12];
    uint8_t can_0X348[12];
    uint8_t can_0X34A[12];
    uint8_t can_0X17D[12];
    uint8_t can_0X1C6[12];
    uint8_t can_0X128[12];
    uint8_t can_0X230[12];
    uint8_t can_0X1E5[12];
    uint8_t can_0X0E1[12];
    uint8_t can_0X332[12];
    uint8_t can_0X363[12];
    uint8_t can_0X359[12];
    uint8_t can_0X140[12];
    uint8_t can_0X317[12];
    uint8_t can_0X125[12];
    uint8_t can_0X1F1[12];
    uint8_t can_0X319[12];
    uint8_t can_0X320[12];
    uint8_t can_0X53B[12];
    uint8_t can_0X52A[12];
    uint8_t can_0X43B[12];
    uint8_t can_0X122[12];
    uint8_t can_0X433[12];
    uint8_t can_0X107[12];
    uint8_t can_0X339[12];
    uint8_t can_0X527[12];
    uint8_t can_0X141[12];
    uint8_t can_0X350[12];
    uint8_t can_0X10F[12];
    uint8_t can_0x621[12];
    uint8_t can_0X425[12];
    uint8_t can_0X316[12];
    uint8_t can_0X406[12];
    uint8_t can_0X573[12];
    uint8_t can_0X53D[12];
    uint8_t can_0X131[12];
    uint8_t can_0X686[12];
} penetrateData_81_canVal_t;//自定义数据结构体 广播的消息结构中包含了原始的CAN数据
#define penetrateData_81_canVal_t_period (1000)
#define penetrateData_81_canVal_t_msgid (0x00110008)

typedef struct
{
    uint8_t totalCelNum;//单体蓄电池总数
    uint8_t batPakTemNum;//该包动力蓄电池温度探针总数
} TemperatureProbeArray_canVal_t;//温度探针个数和单体个数
#define TemperatureProbeArray_canVal_t_period (1000)
#define TemperatureProbeArray_canVal_t_msgid (0x00110009)

typedef struct
{
    uint8_t BMSCC2Sts;//电池管理系统检测快充连接状态
    uint8_t CCSts;//慢充充电连接状态
    uint8_t VecStatRdy;//整车高压状态
    uint8_t VecChrgingSts;//充电线连接状态
    uint8_t VecChrgStsIndOn;//充电状态
    float VehSpdAvgDrvn;//车速
    uint8_t VecSOC;//整车电池电量
    float BatExtVol;//电池包外侧总电压
    uint8_t VecActGearSts;//整车当前档位状态
    uint8_t KyPstn;//整车钥匙状态
    uint8_t AnThWaSt;//设防状态
    uint8_t HandBrkSts;//手刹状态
    uint8_t vcu_ota;//VCU 是否接收升级
} Data_Carota_canVal_t;//柯洛达所需信号项
#define Data_Carota_canVal_t_period (1000)
#define Data_Carota_canVal_t_msgid (0x00110010)

#pragma pack()

#endif
