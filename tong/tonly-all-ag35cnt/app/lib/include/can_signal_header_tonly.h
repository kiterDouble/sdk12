#ifndef _STRUCT_CANSTRUCT_H
#define _STRUCT_CANSTRUCT_H

#include <stdint.h>

#pragma pack(1)
typedef struct
{
    float Q2V_SOC;//Q2V-电池包SOC
    float BMS_SOC;//BMS-电池包SOC
    uint8_t gear;//档位信息
    float vehicle_speed;//车速
    float acc_pedal;//油门开度
    float brake_pedal;//制动踏板开度
    uint8_t PKB_status;//PKB状态
    uint8_t max_error_level;//最大故障等级
    uint8_t key_position;//钥匙开关位置
    float battery_voltage;//蓄电池电压
    float engine_speed;//发动机实际转速
    int16_t motor_speed;//电机转速
    float controller_bus_voltage_1;//控制器母线电压-1
    float controller_bus_voltage_2;//控制器母线电压-2
} TonlyAllCanSignal_canVal_t;//同力所有CAN信号集合
#define TonlyAllCanSignal_canVal_t_period (1000)
#define TonlyAllCanSignal_canVal_t_msgid (0x3FF0)

typedef struct
{
    uint8_t vehicle_status;//车辆状态: 0x01:车辆启动状态, 0x02:熄火, 0x03:其他状态, 0xFE:异常, 0xFF:无效
    uint8_t gear;//点火档位位置: 0x0:Initial, 0x1:OFF, 0x2:ACC, 0x3:ON, 0x4:Start, 0xF:Invalid
    float vehicle_speed;//车辆速度(km/h)
    int32_t drive_motor_speed;//驱动电机转速(r/min)
    uint8_t HV_ready_status;//整车Ready状态: 0x0:WAIT 0x1:READY
    uint8_t motor_status;//电机运行状态: 0x1:耗电, 0x2:发电, 0x3:关闭状态, 0x4:准备状态, 0xFE:异常, 0xFF:无效
    uint8_t BMS_fault_Lv;//BMS故障等级: 0x0:无故障, 0x1:一级级故障, 0x2:二级级故障, 0x3:三级级故障, 0x4:四级故障
    uint8_t VCU_sys_DTC;//VCU系统故障码: 0x0:无故障, 0x1~0x32:三级故障, 0x33~0x96:二级故障, 0x97~0xFA:一级故障, 0xFE:异常, 0xFF:无效
    uint8_t BMS_status;//BMS状态: 0x0:上电状态, 0x1:预留, 0x2:高压断开状态, 0x3:充电状态, 0x4:标定与诊断, 0x5:预留, 0x6:预留, 0x7:驻车加热状态
    uint8_t driver_departure_alarm;//驾驶员离座报警: 0x0:正常, 0x1:离开座位
    uint8_t remote_cmd_exec_status;//远程命令执行状态: 0x0:无, 0x1:握手校验, 0x2:车门解锁, 0x3:车门闭锁, 0x4:一键开窗, 0x5:一键关窗, 0x6:远程寻车, 0x7:一键启动, 0x8:一键熄火, 0x9:快速制冷, 0xA:快速制热, 0xB:关闭空调, 0x10:车辆上低压电, 0x11:电池预约加热
} MonitorCanSignalData_canVal_t;//终端管理平台CAN信号数据结构体
#define MonitorCanSignalData_canVal_t_period (1000)
#define MonitorCanSignalData_canVal_t_msgid (0x3FEF)

#pragma pack()

#endif
