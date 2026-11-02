/*************************************************
Copyright ? 2024 SiRun (Beijing) . All rights reserved.
  File Name:        batterySdk.h
  Author:          lei.wang
  Created Time:    2024
  Description:     备用电池管理SDK模块
   Others:      // 其它说明
*************************************************/

#ifndef _BATTERY_SDK_H_
#define _BATTERY_SDK_H_

#include "stdint.h"

typedef void (* ErrorRecallFun_t)(uint8_t flag);

typedef enum
{
    E_BATTERY_FDK,                                       //FDK电池
    E_BATTERY_XYSR,                                      //西叶-斯润
}BatteryType_E;

typedef enum
{
    E_BatteryState_Init,                                 //初始化状态
    E_BatteryState_BatteryTemperatureCheck,              //温度检查状态
    E_BatteryState_Wait5MinDelay,                        //5min延时状态
    E_BatteryState_BatteryVoltageCheck,                  //电压检查状态
    E_BatteryState_MiddleVoltageCharge,                  //充电
    E_BatteryState_MiddleVoltageChargeCheckTem,          //检查温度
    E_BatteryState_MiddleVoltageChargeFinish,            //充电完成
    E_BatteryState_LowVoltageCharge,                     //低压充电
    E_BatteryState_LowVoltageChargeCheckTem,             //低压充电检查温度
    E_BatteryState_LowVoltageChargeFinish,               //低压充电完成
    E_BatteryState_HealthCheck,                          //电池健康度检查
    E_BatteryState_TrickleCharge,                        //涓流充电
    E_BatteryState_ChargeFinishCheckTem,                 //充电结束检查温度
    E_BatteryState_ChargeFinishDelay10Min,               //充电结束延时10min
    E_BatteryState_StopCharge,                           //停止充电状态

}BatteryState_e;

typedef struct 
{
    int16_t tempHighErrorValue;                         //温度过高错误阈值 单位 ℃,
    int16_t tempHighAlarmValue;                         //温度过高报警阈值 单位 ℃
    uint16_t voltageHighErrorValue;                     //电压过高错误阈值 单位 mv
    uint16_t voltageLowErrorValue;                      //电压过低错误阈值 单位 mv
    uint8_t batterySocAlarmValue;                       //电压电量报警阈值 单位 %
    ErrorRecallFun_t batteryTemperatureHighErrorFun;    //温度过高错误处理
    ErrorRecallFun_t batteryNoPlugInErrorFun;           //无电池错误处理
    ErrorRecallFun_t batteryVoltageHighErrorFun;        //电压过高错误处理
    ErrorRecallFun_t batteryVoltageLowErrorFun;         //电压过低错误处理
    ErrorRecallFun_t batteryShortErrorFun;              //电池短路错误处理
}BatteryConfigure_t;


/*************************************************
  Function:       BatterySdkInit
  Description:    SDK备用电池管理模块初始化接口
  Input:          pBatteryConfigure：错误处理回调注册
                  cycleTime：设置process周期调用接口调用周期
  Output:         函数执行结果
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkInit(const BatteryConfigure_t *pBatteryConfigure,uint16_t cycleTime,BatteryType_E batteryType);

/*************************************************
  Function:       BatterySdkCycleProcess
  Description:    备用电池管理周期调用接口，该接口需要以一定周期进行循环调用
  Input:          无
  Output:         无
  Return:         
  Others:         
*************************************************/
void BatterySdkCycleProcess(void);

/*************************************************
  Function:       BatterySdkGetVoltage
  Description:    获取备用电池电压接口
  Input:          无
  Output:         pVoltage：电压值，单位mv
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkGetVoltage(uint32_t *pVoltage);

/*************************************************
  Function:       BatterySdkGetTemperature
  Description:    获取备用电池温度接口
  Input:          无
  Output:         pTemperature：备用电池温度值，单位：0.001摄氏度
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkGetTemperature(int32_t *pTemperature);

/*************************************************
  Function:       BatterySdkSetTestMode
  Description:    设置备用电池进入测试模式接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkSetTestMode(void);

/*************************************************
  Function:       BatterySdkSetNormalMode
  Description:    设置备用电池从测试模式进入正常模式接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkSetNormalMode(void);

/*************************************************
  Function:       BatterySdkTestModeChargeStart
  Description:    设置测试模式下备用电池充电接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkTestModeChargeStart(void);

/*************************************************
  Function:       BatterySdkTestModeChargeStop
  Description:    设置测试模式下备用电池停止充电接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkTestModeChargeStop(void);

/*************************************************
  Function:       BatterySdkTestModeDischargeStart
  Description:    设置测试模式下备用电池放电接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkTestModeDischargeStart(void);

/*************************************************
  Function:       BatterySdkTestModeDischargeStop
  Description:    设置测试模式下备用电池停止放电接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkTestModeDischargeStop(void);

/*************************************************
  Function:       BatterySdkGetBatteyStatus
  Description:    获取备用电池的连接状态
  Input:          无
  Output:         无
  Return:         0：连接
                  1：未连接
  Others:         
*************************************************/
uint8_t BatterySdkGetBatteyStatus(void);

/*************************************************
  Function:       BatterySdkGetBatteySoc
  Description:    获取备用电池的电池电量接口
  Input:          无
  Output:         无
  Return:         电池电量百分比
  Others:         
*************************************************/
uint32_t BatterySdkGetBatteySoc(void);

/*************************************************
  Function:       BatterySdkGetBatteyVoltageError
  Description:    获取备用电池的电压异常状态
  Input:          无
  Output:         0：nomal
                  1：低压警告
                  2：高压警告
  Others:         
*************************************************/
uint8_t BatterySdkGetBatteyVoltageError(void);
uint8_t BatterySdkGetBatteyVoltageAlarm(void);

/*************************************************
  Function:       BatterySdkGetBatteyTempError
  Description:    获取备用电池的温度异常状态
  Input:          无
  Output:         0：nomal
                  1：预留
                  2：高温警告
  Others:         
*************************************************/
uint8_t BatterySdkGetBatteyTempAlarm(void);

/*************************************************
  Function:       BatterySdkGetBatteyChargingDuration
  Description:    获取备用电池的充电时长,单位s
  Input:          无
  Output:         0：未充电
                  >0:充电时长
  Others:         
*************************************************/
uint32_t BatterySdkGetBatteyChargingDuration(void);                      

#endif