/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: vehicleSignalApp.c
 Author:
 Created Time:
 Description: vehicle signal application implementation
 Others:
*************************************************/
/****************************** include ***************************************/
#include "vehicleSignalApp.h"
#include "logHal.h"
#include <string.h>
/****************************** Macro Definitions ******************************/
#define CAN_ID_CONFIGURE_DEFINE_BEGIN(canChannel) \
    typedef struct                                \
    {
#define CAN_ID_CONFIGURE_DEFINE(canIdName) canId_map_configure_t s##canIdName;
#define CAN_ID_CONFIGURE_DEFINE_END(canChannel) \
    }                                           \
    struct_can##canChannel##Id_define_t;
/*****************************************************************************/
#define CAN_ID_CONFIGURE_BEGIN(CanChannel) const static struct_can##CanChannel##Id_define_t m_can##CanChannel##IdList = \
                                               {
#define CAN_V_ID_ELEMENT(canIdName, cycleTime, Id) .s##canIdName = {Id, cycleTime},
#define CAN_ID_CONFIGURE_END(CanChannel) \
    }                                    \
    ;                                    \
    static can_signal_msg_buffer_t m_can##CanChannel##RxMsgBuffer[sizeof(m_can##CanChannel##IdList) / sizeof(canId_map_configure_t)];
/*****************************************************************************/
#define CAN_MSG_BUFFER_ADDRESS(canChannel) (m_can##canChannel##RxMsgBuffer)
#define CAN_ID_TO_BUFFER_INDEX(canChannel, CanIdName) ((&m_can##canChannel##IdList.s##CanIdName - (canId_map_configure_t *)&m_can##canChannel##IdList))
#define CAN_MSG_BUFFER_SIZE(canChannel) (sizeof(m_can##canChannel##RxMsgBuffer) / sizeof(m_can##canChannel##RxMsgBuffer[0]))
#define CAN_MSG_BUFFER_GET_ELEMENT(canChannel, index) (m_can##canChannel##RxMsgBuffer[index])
#define CAN_ID_CONFIGURE_POINTER(canChannel) (&m_can##canChannel##IdList)
/****************************** Type Definitions ******************************/
#if 1  // tonly CAN矩阵
/*同力使用*/
CAN_ID_CONFIGURE_DEFINE_BEGIN(0)
CAN_ID_CONFIGURE_DEFINE(Q2V_SOC)
CAN_ID_CONFIGURE_DEFINE(BMS_SOC)
CAN_ID_CONFIGURE_DEFINE(VCU_LockVehicle)
CAN_ID_CONFIGURE_DEFINE(B2TM_Info)
CAN_ID_CONFIGURE_DEFINE(DB_EngSpd)
CAN_ID_CONFIGURE_DEFINE(TBOX_ActiveCmd)
CAN_ID_CONFIGURE_DEFINE(TBOX_TxKey)
CAN_ID_CONFIGURE_DEFINE(TBOX_LockCmd)
CAN_ID_CONFIGURE_DEFINE(TBOX_WakeCmd)
CAN_ID_CONFIGURE_DEFINE(VCU_KeySw)
CAN_ID_CONFIGURE_DEFINE(DB_LowBatVolt)
CAN_ID_CONFIGURE_DEFINE(EEC_ActSpd)
CAN_ID_CONFIGURE_DEFINE(MCU1_MotSpd)
CAN_ID_CONFIGURE_DEFINE(SteerPUmpCtrl2)
CAN_ID_CONFIGURE_DEFINE(SteerPUmpCtrl1)
CAN_ID_CONFIGURE_DEFINE(VCU_Seed)
CAN_ID_CONFIGURE_DEFINE(Tx_DB_VehicleParaG)
CAN_ID_CONFIGURE_DEFINE(Rx_DB_vehiclePara4)
CAN_ID_CONFIGURE_DEFINE_END(0)
#else
CAN_ID_CONFIGURE_DEFINE_BEGIN(0)
CAN_ID_CONFIGURE_DEFINE(GW_FLDCM_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_FRDCM_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_FLDCM_2_T)
CAN_ID_CONFIGURE_DEFINE(GW_BCM_TEL_T)
CAN_ID_CONFIGURE_DEFINE(GW_BCM_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_BCM_2_T)
CAN_ID_CONFIGURE_DEFINE(GW_PEPS_3_T)
CAN_ID_CONFIGURE_DEFINE(GW_PEPS_5_T)
CAN_ID_CONFIGURE_DEFINE(GW_PEPS_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_BCM_BCAN_11_T)
CAN_ID_CONFIGURE_DEFINE(GW_HVACF_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_SCM_HVSM_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_PLGM_1_T)
CAN_ID_CONFIGURE_DEFINE(GWM_TCAN_NM)
CAN_ID_CONFIGURE_DEFINE(GW_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_NMData)
CAN_ID_CONFIGURE_DEFINE(GW_BCS_EPB_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_BCS_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_SRS_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_SRS_2_T)
CAN_ID_CONFIGURE_DEFINE(GW_EMS_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_EBS_1_T)
CAN_ID_CONFIGURE_DEFINE(GW_ACU_1_T)
CAN_ID_CONFIGURE_DEFINE_END(0)
#endif

/****************************** Global Variables ******************************/
#if 1  // tonly CAN矩阵
CAN_ID_CONFIGURE_BEGIN(0)
/*同力使用*/
CAN_V_ID_ELEMENT(Q2V_SOC, 100, 0x1830B021)
CAN_V_ID_ELEMENT(BMS_SOC, 200, 0x1884EFF3)
CAN_V_ID_ELEMENT(DB_EngSpd, 100, 0x18F101D0)
CAN_V_ID_ELEMENT(TBOX_ActiveCmd, 100, 0x18FFD4FC)
CAN_V_ID_ELEMENT(TBOX_TxKey, 100, 0x18FFD5FC)
CAN_V_ID_ELEMENT(TBOX_LockCmd, 100, 0x18FFD6FC)
CAN_V_ID_ELEMENT(TBOX_WakeCmd, 100, 0x1820F03A)
CAN_V_ID_ELEMENT(VCU_KeySw, 100, 0x18F117D1)
CAN_V_ID_ELEMENT(DB_LowBatVolt, 500, 0x18F115D0)
CAN_V_ID_ELEMENT(EEC_ActSpd, 10, 0x0CF00400)
CAN_V_ID_ELEMENT(MCU1_MotSpd, 10, 0xCFF0008)
CAN_V_ID_ELEMENT(SteerPUmpCtrl2, 1000, 0x142FFE95)
CAN_V_ID_ELEMENT(SteerPUmpCtrl1, 1000, 0x142FFF95)
CAN_V_ID_ELEMENT(VCU_Seed, 0, 0x18FC0100)
CAN_V_ID_ELEMENT(VCU_LockVehicle, 100, 0x18FC0900)
CAN_V_ID_ELEMENT(B2TM_Info, 500, 0x18FF45F4)
CAN_V_ID_ELEMENT(Tx_DB_VehicleParaG, 200, 0x18F111D0)
CAN_V_ID_ELEMENT(Rx_DB_vehiclePara4, 100, 0x18FEE017)
CAN_ID_CONFIGURE_END(0)
#else
CAN_ID_CONFIGURE_BEGIN(0)
CAN_V_ID_ELEMENT(GW_FLDCM_1_T, 100, 0x3E5)
CAN_V_ID_ELEMENT(GW_FRDCM_1_T, 100, 0x3E7)
CAN_V_ID_ELEMENT(GW_FLDCM_2_T, 1000, 0x3E6)
CAN_V_ID_ELEMENT(GW_BCM_TEL_T, 1000, 0x046) // event message default
CAN_V_ID_ELEMENT(GW_BCM_1_T, 100, 0x1E3)
CAN_V_ID_ELEMENT(GW_BCM_2_T, 20, 0x25D)
CAN_V_ID_ELEMENT(GW_PEPS_3_T, 1000, 0x055)  // event message default
CAN_V_ID_ELEMENT(GW_PEPS_5_T, 1000, 0x04D)  // event message default
CAN_V_ID_ELEMENT(GW_PEPS_1_T, 100, 0x1E5)
CAN_V_ID_ELEMENT(GW_BCM_BCAN_11_T, 100, 0x3B9)
CAN_V_ID_ELEMENT(GW_HVACF_1_T, 200, 0x1E6)
CAN_V_ID_ELEMENT(GW_SCM_HVSM_1_T, 200, 0x34E)
CAN_V_ID_ELEMENT(GW_PLGM_1_T, 100, 0x305)
CAN_V_ID_ELEMENT(GWM_TCAN_NM, 200, 0x54F)
CAN_V_ID_ELEMENT(GW_1_T, 100, 0x3C8)
CAN_V_ID_ELEMENT(GW_NMData, 1000, 0x1FF) // event message default
CAN_V_ID_ELEMENT(GW_BCS_EPB_1_T, 100, 0x1ED)
CAN_V_ID_ELEMENT(GW_BCS_1_T, 100, 0x1EB)
CAN_V_ID_ELEMENT(GW_SRS_1_T, 20, 0x1AE)
CAN_V_ID_ELEMENT(GW_SRS_2_T, 100, 0x3AB)
CAN_V_ID_ELEMENT(GW_EMS_1_T, 100, 0x1EE)
CAN_V_ID_ELEMENT(GW_EBS_1_T, 500, 0x315)
CAN_V_ID_ELEMENT(GW_ACU_1_T, 500, 0x1E2)
CAN_ID_CONFIGURE_END(0)
#endif

#if 1  // tonly
static const can0_signal_configure_t m_can0SignalConfigure =
{
    .Q2V_ST2_SOC =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, Q2V_SOC),
            .dataType = 0,
            .startBit = 0,
            .bitLength = 8,
            .resulotion = 0.4,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*Q2V电池包SOC*/
    .BMS_ST2_SOC =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, BMS_SOC),
            .dataType = 0,
            .startBit = 0,
            .bitLength = 8,
            .resulotion = 0.4,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*BMS电池包SOC*/
    .Tx_DB_GearNum =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, DB_EngSpd),
            .dataType = 0,
            .startBit = 0,
            .bitLength = 4,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*档位信息: 15-档位错误, 8-七档, 7-六档, 6-五档, 5-四档, 4-三档, 3-二档, 2-一档, 1-倒档, 0-空档*/
    .Tx_DB_VehicleSpeed =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, DB_EngSpd),
            .dataType = 0,
            .startBit = 8,
            .bitLength = 16,
            .resulotion = 0.00390625,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*车速 Km/h*/
    .Tx_DB_AccPed =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, DB_EngSpd),
            .dataType = 0,
            .startBit = 32,
            .bitLength = 8,
            .resulotion = 0.4,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*油门开度 %*/
    .Tx_DB_BrakePedal =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, DB_EngSpd),
            .dataType = 0,
            .startBit = 40,
            .bitLength = 8,
            .resulotion = 0.4,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*制动踏板开度 %*/
    .Tx_DB_PKB =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, DB_EngSpd),
            .dataType = 0,
            .startBit = 52,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*PKB状态: 0x0-无效, 0x1-有效*/
    .Tx_DB_MaxErrLevel =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, DB_EngSpd),
            .dataType = 0,
            .startBit = 61,
            .bitLength = 3,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*整车故障等级*/
    .VCU_KeyPosnSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, VCU_KeySw),
            .dataType = 0,
            .startBit = 0,
            .bitLength = 4,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*钥匙开关位置: 0x0-钥匙OFF, 0x1-钥匙ON, 0x2-钥匙START*/
    .Tx_DB_BattVolt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, DB_LowBatVolt),
            .dataType = 0,
            .startBit = 32,
            .bitLength = 16,
            .resulotion = 0.1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*蓄电池电压 V*/
    .Eng_EEC1_ActSpd =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, EEC_ActSpd),
            .dataType = 0,
            .startBit = 24,
            .bitLength = 16,
            .resulotion = 0.03125,
            .offset = -273,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*发动机实际转速 rpm*/
    .Rx_MCU1_MotSpd =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, MCU1_MotSpd),
            .dataType = 0,
            .startBit = 0,
            .bitLength = 16,
            .resulotion = 1,
            .offset = -15000,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*电机转速 rpm*/
    .Rx_SteerPumpCtrl2_BusVolt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, SteerPUmpCtrl2),
            .dataType = 0,
            .startBit = 16,
            .bitLength = 16,
            .resulotion = 0.1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*控制器母线电压2 V*/
    .Rx_SteerPumpCtrl1_BusVolt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, SteerPUmpCtrl1),
            .dataType = 0,
            .startBit = 16,
            .bitLength = 16,
            .resulotion = 0.1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*控制器母线电压1 V*/
    .Rx_VcuSeedValue =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, VCU_Seed),
            .dataType = 0,
            .startBit = 0,
            .bitLength = 24,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*vcu seed种子*/
    .Tx_DB_FaultCode =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, Tx_DB_VehicleParaG),
            .dataType = 0,
            .startBit = 0,
            .bitLength = 16,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*VCU故障码*/
    .Rx_DB_Trip_ODO =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, Rx_DB_vehiclePara4),
            .dataType = 0,
            .startBit = 32,
            .bitLength = 32,
            .resulotion = 0.125,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*仪表发送报文4 - 行驶里程 km*/
    .Rx_ActiveState =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, VCU_LockVehicle),
            .dataType = 0,
            .startBit = 25,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*锁车功能激活状态: 0-锁车功能未激活, 1-锁车功能激活*/
    .Rx_LockState =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, VCU_LockVehicle),
            .dataType = 0,
            .startBit = 26,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*车辆实际锁车状态: 0-未锁车, 1-锁车*/
    .Rx_KeyOkState =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, VCU_LockVehicle),
            .dataType = 0,
            .startBit = 27,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*Key 值正确状态: 0-Key值不正确, 1-Key值正确*/
    .Rx_VcuResponse =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, VCU_LockVehicle),
            .dataType = 0,
            .startBit = 48,
            .bitLength = 8,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*VCU应答帧: 0x01-锁车功能激活, 0x02-锁车功能关闭, 0x03-握手成功, 0x04-握手失败*/
    .B2T_RQ_SetMode_A  =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, B2TM_Info),
            .dataType = 0,
            .startBit = 1,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*BMS请求BTMS模式: 0x02-制热模式*/
     .T_Box_ActiveCmd  =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, TBOX_ActiveCmd),
            .dataType = 0,
            .startBit = 0,  
            .bitLength = 16,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*tbox激活指令*/ 
      .T_Box_TxKey  =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, TBOX_ActiveCmd),
            .dataType = 0,
            .startBit = 0,  
            .bitLength = 64,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*tbox key值*/ 
        .T_Box_LockCmd  =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, TBOX_LockCmd),
            .dataType = 0,
            .startBit = 0,  
            .bitLength = 16,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, /*tbox 锁车指令*/ 
        .T_Box_WakeCmd  =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, TBOX_WakeCmd),
            .dataType = 0,
            .startBit = 0,  
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0x0,
        }, /*tbox 唤醒指令*/    
};
#else
static const can0_signal_configure_t m_can0SignalConfigure =
    {
        .FLDCM_FLWinPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front left window position

        .FLDCM_RLWinPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear left window position

        .FLDCM_RLWinSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 22,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear left window status

        .FLDCM_FLWinOperationSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 19,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front left window operation status

        .FLDCM_RLWinOperationSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 16,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear left window operation status

        .FLDCM_FLWinSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 30,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front left window status

        .FLDCM_FrontDoorPwrWinSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 27,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front Left Door Power window switch status

        .FLDCM_RearDoorPwrWinSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear Left Door Power window switch status

        .FLDCM_FLWinOverheatSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 39,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front left window overheat status

        .FLDCM_RLWinOverheatSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 38,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear left window overheat status

        .FLDCM_FLWinVentilateModeSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 37,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front Left

        .FLDCM_RLWinVentilateModeSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 36,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear Left

        .FLDCM_DrvCfgSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 35,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Dirve Configuration Status

        .FLDCM_DrvCfgReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 34,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Dirve Configuration Request

        .FLDCM_FLDHPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 32,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front Left Door Handle Position

        .FLDCM_RLDHPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 46,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear Left Door Handle Position

        .FLDCM_FLDHOperationSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 45,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front Left Door Handle Operation Status

        .FLDCM_RLDHOperationSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 44,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear Left Door Handle Operation Status

        .FLDCM_NotSleepSource1 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 43,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Not Sleep Source1

        .FLDCM_MirrorOpSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 40,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Driver Door module Operation Status feedback to FLDCM,FRDCM

        .FLDCM_MirrorFoldUnfoldSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 54,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Left Rearview Mirror Fold/Unfold status

        .FLDCM_ExMirLedWarnFaultStL =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 53,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // BSD Light Failure_FL

        .FLDCM_NotSleepSource2 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 51,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Not Sleep Source2

        .FLDCM_Remotecontrolst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // FLDCM_Remote control status

        .FLDCM_RemoteDiagCode =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 62,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The fault identification of FLDCM

        .FLDCM_FLWinOperationRs =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 59,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front left window operation reasons

        .FLDCM_RLWinOperationRs =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_1_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear left window operation reasons

        .FRDCM_FRWinPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front right window position

        .FRDCM_RRWinPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear right window position

        .FRDCM_RRWinSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 22,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear right window status

        .FRDCM_FRWinOperationSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 19,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front right window operation status

        .FRDCM_RRWinOperationSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 16,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear right window operation status

        .FRDCM_FRWinSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 30,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front right window status

        .FRDCM_FrontDoorPwrWinSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 27,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front Right Door Power window switch status

        .FRDCM_RearDoorPwrWinSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear Right Door Power window switch status

        .FRDCM_FRWinOverheatSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 39,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front right window overheat status

        .FRDCM_RRWinOverheatSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 38,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear right window overheat status

        .FRDCM_FRWinVentilateModeSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 37,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front Right

        .FRDCM_RRWinVentilateModeSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 36,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear Right

        .FRDCM_DrvCfgSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 35,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Dirve Configuration Status

        .FRDCM_DrvCfgReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 34,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Dirve Configuration response

        .FRDCM_FRDHPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 32,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front Right Door Handle Position

        .FRDCM_RRDHPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 46,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear Right Door Handle Position

        .FRDCM_FRDHOperationSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 45,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front Right Door Handle  Operation Status

        .FRDCM_RRDHOperationSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 44,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear Right Door Handle  Operation Status

        .FRDCM_NotSleepSource1 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 43,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Not Sleep Source1

        .FRDCM_MirrorOpSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 40,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Passenger door module operation status feedback to FLDCM

        .FRDCM_MirrorFoldUnfoldSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 54,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Right Rearview Mirror Fold/Unfold status

        .FRDCM_ExMirLedWarnFaultStR =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 53,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // BSD light failure_FR

        .FRDCM_NotSleepSource2 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 51,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Not Sleep Source2

        .FRDCM_Remotecontrolst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // _Remote control status

        .FRDCM_RemoteDiagCode =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 62,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The fault identification of FRDCM

        .FRDCM_FRWinOperationRs =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 59,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front right window operation reasons

        .FRDCM_RRWinOperationRs =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FRDCM_1_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear right window operation reasons

        .DCM_OSRMSelectSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 6,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // OSRM Adjust Switch Status, Select

        .DCM_MemSysOperationReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 5,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Memory System Operation request for AVNT active pop-up window

        .DCM_PowerWinOpenWarn =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 4,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Window open warning

        .DCM_MirrorFoldUnfoldSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 2,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Manual rearview mirror fold/Unfold cmd

        .DCM_FLPwrWinSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 13,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front left  Power window switch status

        .DCM_RLPwrWinSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 10,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear left Power window switch status

        .DCM_CentralLockSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // DCM central lock switch status

        .DCM_FRPwrWinSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 21,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front Right Power window switch status

        .DCM_RRPwrWinSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 18,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear Right Power window switch status

        .DCM_MirrorFoldUnfoldCmd =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 16,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rearview Mirror Fold/Unfold Command

        .DCM_WinLockBtnSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 39,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // passenger and rear window lock status

        .DCM_PetWinlockBtnSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 38,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The lock status of window-control buttons related to pet mode

        .DCM_APWLEnableSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 31,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Anti-pinch Window Lifter operation

        .DCM_DoorHandleFuncCfgSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 30,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // DCM Door Handle function

        .DCM_OSRMAngleAdjFuncCfgSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 28,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Function Configuration Status，Reverse Mirror Dip

        .DCM_OSRMUpSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 27,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // OSRM Adjust Switch Status, up

        .DCM_OSRMDownSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 26,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // OSRM Adjust Switch Status, down

        .DCM_OSRMLeftSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 25,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // OSRM Adjust Switch Status, left

        .DCM_OSRMRightSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_FLDCM_2_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // OSRM Adjust Switch Status, right

        .BCM_TEL_IMMOCode0 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_TEL_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-BOX

        .BCM_TEL_IMMOCode1 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_TEL_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-BOX

        .BCM_TEL_IMMOCode2 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_TEL_T),
                .dataType = 0,
                .startBit = 16,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-BOX

        .BCM_TEL_IMMOCode3 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_TEL_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-BOX
            
        .BCM_TEL_IMMOCode4 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_TEL_T),
                .dataType = 0,
                .startBit = 32,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-BOX

        .BCM_TEL_IMMOCode5 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_TEL_T),
                .dataType = 0,
                .startBit = 40,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-BOX

        .BCM_TEL_IMMOCode6 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_TEL_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-BOX
        
        .BCM_TEL_IMMOCodeSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_TEL_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Request=Challenge code to T-BOX
         
        .BCM_IG1St =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 2,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Ignition 1 status

        .BCM_BatSOCLowWarning =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 3,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Battery Low SOC Warning

        .BCM_PGearSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 4,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // P gear status

        .BCM_LampOnWarningReq =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 5,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Lamp left on warning request

        .BCM_HornSwitchSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 6,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Horn switch status

        .BCM_FrontWiperServiceMode =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 7,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // BCM_FrontWiper Service Mode

        .BCM_TrunkReleaseCmd =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 14,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Trunk release command

        .BCM_SunRoofOpenWarning =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 27,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Sunroof open warning

        .BCM_RmtCtrlFobBattLow =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 29,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Remote control fob battery low

        .BCM_KeyInReminderWarnReq =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 30,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Key in reminder warning request

        .BCM_AlarmHistory =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 32,
            .bitLength = 3,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Alarm history

        .BCM_ETWS_St =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 40,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Engine theft warning system status

        .BCM_WinMoveSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 41,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Window move status

        .BCM_SunRoofPstSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 43,
            .bitLength = 4,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Sunroof position status

        .BCM_FOB_ID =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 51,
            .bitLength = 4,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // FOB ID

        .BCM_RemoteControlSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 56,
            .bitLength = 4,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Remote control status

        .RLS_LightSwitchReason =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 60,
            .bitLength = 4,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // RLS light switch reason

        .BCM_RemoteCtrlFDMRMCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 128,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Remote control FDMRM config status

        .BCM_FogLampTurnCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 129,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Fog lamp turn config status

        .BCM_RemoteUnlockCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 130,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Remote unlock config status

        .BCM_SpdAutoLockCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 131,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Speed auto lock config status

        .BCM_AutoUnlockCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 132,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Auto unlock config status 

        .BCM_FollowMeHomeCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 134,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Follow me home function configuration status

        .BCM_AutoLightSnsCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 136,
            .bitLength = 3,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Sensibility of auto light configuration status

        .BCM_FrontWiperSerPstFuncCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 139,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Front wiper service position function configuration status

        .BCM_RGearEnableRearWipeCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 140,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Reverse gear enable rear wiping configuration status

        .BCM_DTRLFuncCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 141,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // BCM day time running lamp function configuration status

        .BCM_HornCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 142,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // BCM horn active on lock and unlock configuration status

        .BCM_OSRMAutoFoldCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 143,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // BCM outside rear view mirror auto folding function configuration status

        .BCM_IALFuncCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 152,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // IAL(Interior Ambient Light) function status

        .BCM_AutoWiperCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 153,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Auto Wiper configuration status

        .BCM_AutoDomeLightCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 155,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // BCM auto dome light configuration status

        .BCM_DRLActive_cfgst =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 156,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Day time running lamp  configuration status，memory the ACU DRL SW Status

        .BCM_IGNOffTimeVD =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 157,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Ignition off time validity

        .BCM_IGNOffTime =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 158,
            .bitLength = 10,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Ignition off time

        .BCM_HeadlightDelayCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 165,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // BCM Head light delay function configuration status

        .BCM_AutoSRWinCloseCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 163,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Automactic Sunroof/Window Close configuration status of locking

        .BCM_RLSAutoSRWinCloseCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 164,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Automactic Sunroof/Window Close configuration status in rain

        .BCM_ACU_DRLSw_Enable =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 167,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // indicate if allow to control Day time running lamp  by ACU

        .BCM_LatestLockUnlockCmd =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 168,
            .bitLength = 5,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // The latest lock/unlock command source

        .BCM_ParkAutoUnlockCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 173,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Park auto unlock configuration status

        .BCM_ADSLightCfgSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 175,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // ADS light enable

        .BCM_HazardLampSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 192,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Hazard lamp status

        .BCM_RLSWarning_Rain =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 193,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // RLS auto wiper warning

        .BCM_RLSWarning_Light =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 194,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // RLS auto light warning

        .BCM_ReverseLightSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 196,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Reverse light status

        .BCM_ReverseLightFaultSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 197,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Reverse light fault status

        .BCM_BrakeLightSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 198,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Brake light status

        .BCM_SunshadePst =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 200,
            .bitLength = 7,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // The Sun Shade Position

        .BCM_RearWiperSwitchSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 207,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Rear wiper switch status

        .BCM_FRWinMoveSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 208,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Front rear window moved status

        .BCM_TEL_HazLampCtrlSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 214,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Hazard lamp in telematic control status

        .BCM_AlarmHistory1 =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 216,
            .bitLength = 3,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // BCM recent alarm trigger reason

        .BCM_TrunkReleaseCmd1 =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 219,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Trunk release command

        .BCM_FrontWiperSwitchSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 221,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Front wiper switch status

        .BCM_AmbientLightSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 223,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Ambient Light status

        .BCM_VCU_UnlockACChgPortReqP =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 224,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // BCM request VCU to unlock AC charge lock port(Periodic Message)

        .BCM_FuelCapAjarSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 225,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // High pressure FuelCap ajar status

        .BCM_RefuelSwSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 226,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Refule switch state

        .BCM_RLSAutoSRWinCloseResult =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 228,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Automactic Sunroof/Window Close Result

        .BCM_SearchingKeyReq =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 230,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // the request of searching key when relock , SBM lock and T-BOX lock

        .BCM_HornSwSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 231,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // horn switch status

        .BCM_SunRoofPst =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 232,
            .bitLength = 8,
            .resulotion = 0.5,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // (Front) SunRoof Position

        .BCM_HazardLampSwitchSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 241,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Hazard lamp switch status

        .BCM_MirrorAutoFoldUnfoldCmd =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 242,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Auto Rearview Mirror Auto Fold/Unfold comand

        .BCM_SunshadePstSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 244,
            .bitLength = 4,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // The Sunshade Position status.

        .BCM_RainstormModeReq =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 248,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Rain storm Mode request

        .RLS_AutoSRWinCloseReq =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 250,
            .bitLength = 3,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Sunroof/Window Close Request

        .BCM_ADSLightSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 253,
            .bitLength = 1,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Front and behind ADS light status

        .BCM_FWPIntLevelSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 378,
            .bitLength = 3,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Front Wiper interval level

        .BCM_FWPAutoLevelSt =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 381,
            .bitLength = 3,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Front Wiper RLS sensitivity level

        .BCM_HeadLightDelayTime =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 368,
            .bitLength = 3,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Head lightl delay time

        .BCM_RemoteDiagCode =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_1_T),
            .dataType = 0,
            .startBit = 376,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // IBCM Remote Diagnostic Code for T-BOx    

        .BCM_LowBeamSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Low beam status
        .BCM_HighBeamSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 1,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // High beam status
        .BCM_FrontFogLampSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 2,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front fog lamp status
        .BCM_RearFogLampSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 3,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear fog lamp status
        .BCM_FrontWiperSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 4,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front wiper status
        .BCM_KeyInsertSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 6,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Key insert status
        .BCM_ReverseSwitchStVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 7,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Flag indicating reverse switch status
        .BCM_KeySt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Ignition key position status
        .BCM_DTRLSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 10,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Day time running lamp status
        .BCM_BonnetAjarSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 11,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Bonnet ajar status
        .BCM_RearWiperSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 12,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear wiper status

        .BCM_DriverDoorAjarSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 14,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Driver door ajar status
        .BCM_BrakeLightSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 15,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Brake light switch status
        .BCM_TrunkAjarSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 16,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Trunk ajar status
        .BCM_PsngrDoorAjarSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 17,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Passenger door ajar status
        .BCM_RLDoorAjarSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 18,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear left door ajar status
        .BCM_RRDoorAjarSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 19,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear right door ajar status
        .BCM_DRLLFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 20,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Left side DRL(Daytime Running Lamp) fault status
        .BCM_CentralUnLockSWSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 21,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Central Unlock switch status
        .BCM_FrontWasherSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 22,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front washer status
        .BCM_RearWasherSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 23,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear washer status
        .BCM_RoomlampSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Room lamp status
        .BCM_DRLRFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 25,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Right side DRL(Daytime Running Lamp) fault status
        .BCM_TurnLightSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 26,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Turn light switch status

        .BCM_PositionLightswitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 29,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Position light switch status
        .BCM_PositionLightFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 30,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Position light fault status
        .BCM_LowBeamFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 31,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Lowbeam fault status
        .BCM_HornSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 32,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Horn status
        .BCM_LeftTurnLampSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 33,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Left turn lamp status
        .BCM_LeftTurnLampFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 34,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Left turn lamp fault status
        .BCM_RightTurnLampSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 35,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Right turn lamp status
        .BCM_RightTurnLampFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 36,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Right turn lamp fault status
        .BCM_ParkingLampSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 37,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Parking lamp status
        .BCM_TransportModeSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 38,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Vehicle transport mode request
        .BCM_VehReverseSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 39,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indication the BCM judgement of the vehicle reverse status
        .BCM_ATWS_St =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 40,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Anti-theft warning status
        .BCM_AutoHighBeamCtrlSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 43,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Auto light control by IFC(Intelligent Front Camera)status
        .BCM_BeamSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 44,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Beam switch status
        .BCM_MainLampSWSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 46,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Main Lamp switch status
        .BCM_BCAN_2_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The message counter is a 0-15 counter, increased of one unit...
        .BCM_DriverDoorLockSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 52,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Driver door lock status
        .BCM_PsngrDoorLockSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 53,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Passenger door lock status
        .BCM_InteriorTrunkReleaseSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 54,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Trunk release command
        .BCM_CentralLockSWSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 55,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Central lock switch status
        .BCM_BCAN_2_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_2_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Checksum = (byte0 +byte1 …+ byte6) XOR 0xFF

        .PEPS_DriverdoorReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Driver door sensor request

        .PEPS_PsngrdoorReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 2,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Passengner door sensor request

        .PEPS_DoorLockReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 4,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Door lock request

        .PEPS_DoorUnlockReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 5,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Door unlock request

        .PEPS_TrunkReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 6,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Trunk switch request

        .PEPS_keyComdInfor =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Key active command Information

        .PEPS_KeyReminderUnlock =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 16,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS remind unlock

        .PEPS_WELReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 17,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS welcome light request

        .PEPS_WALReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 19,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS walk away lock request

        .PEPS_APUReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 20,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS approach unlock request

        .PEPS_SATOReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 21,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS sensorless automatic trunk open request

        .PEPS_SATOReminder =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 22,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS SATO(sensorless automatic trunk open) reminder

        .PEPS_WALReadyToLockInd =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS WAL(walk away lock ) ready to lock indication

        .PEPS_FOB_ID =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 26,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Fob identification

        .PEPS_ESCLLockReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 30,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Request ESCL to perform locking

        .PEPS_FeedbackToTBOX =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_3_T),
                .dataType = 0,
                .startBit = 35,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS feedback to T-BOX instruction

        .PEPS_TEL_ChallengeCode0 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_5_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-Box instruction

        .PEPS_TEL_ChallengeCode1 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_5_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-Box instruction

        .PEPS_TEL_ChallengeCode2 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_5_T),
                .dataType = 0,
                .startBit = 16,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-Box instruction

        .PEPS_TEL_ChallengeCode3 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_5_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-Box instruction   

        .PEPS_TEL_ChallengeCode4 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_5_T),
                .dataType = 0,
                .startBit = 32,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-Box instruction   
        
        .PEPS_TEL_ChallengeCode5 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_5_T),
                .dataType = 0,
                .startBit = 40,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-Box instruction

        .PEPS_TEL_ChallengeCode6 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_5_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-Box instruction 
            
        .PEPS_TEL_ChallengeCode7 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_5_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Challenge code to T-Box instruction 

        .PEPS_SwitchOffSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS off switch status

        .PEPS_SysPowMode =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 12,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // System power mode

        .PEPS_StarterRelaySt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 19,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS starter relay status

        .PEPS_TeleAuthentSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS_Authentication status

        .PEPS_TeleFailReason =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 50,
                .bitLength = 6,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Remote control fail reason

        .PEPS_VoiceStartFailReason =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Voice start failure reason

        .PEPS_EngForbidSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 61,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Engine forbid start state

        .PEPS_TELMode =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 62,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Telematics remote controll mode

        .PEPS_OffWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 64,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS off warning

        .PEPS_AuthFailWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 66,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Authentication fail warnings

        .PEPS_OpIndicationWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 68,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Operation indication warning

        .PEPS_DeactivationWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 71,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Deactivation warning

        .PEPS_SSBWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 72,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Start and stop button warning

        .PEPS_ESCLWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 74,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // ESCL warning

        .PEPS_FobBattLowWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 77,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // FOB battery low warning

        .PEPS_DoorLockWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 78,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Door lock warning

        .PEPS_KeyReminderWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 80,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Key reminder warning

        .PEPS_KeyOutWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 82,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Key out warning

        .PEPS_VoiceInputWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 84,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Voice input failure warning

        .PEPS_WheelSpdInputWarn =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 86,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Wheel speed input failure warning

        .PEPS_Crankallow =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 87,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Allow cranking at mode 2

        .PEPS_IdInVehicleWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 111,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // When closed last door,if found id in vehicle ,polling condition not OK

        .PEPS_RemoteControlSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 114,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS remote control state

        .PEPS_WELQuitReason =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 160,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS WEL(welcome light)quit reason

        .PEPS_APUQuitReason =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 168,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS APU(approach unlock) quit reason

        .PEPS_WALQuitReason =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 176,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS WAL(walk away lock) quit reason

        .PEPS_SATOQuitReason =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PEPS_1_T),
                .dataType = 0,
                .startBit = 184,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEPS SATO(sensorless automatic trunk open)quit reason

        .BCM_A2PositionLightSwSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_BCAN_11_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // A2 Position Light switch status feedback to ACU

        .BCM_AlarmHazardLampSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_BCAN_11_T),
                .dataType = 0,
                .startBit = 1,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Alarm Hazard lamp status for Tbox

        .BCM_FrontWiperServiceEnable =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_BCAN_11_T),
                .dataType = 0,
                .startBit = 2,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // BCM_FrontWiperServiceEnable

        .BCM_CentralLampFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_BCAN_11_T),
                .dataType = 0,
                .startBit = 3,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Central Lamp Fault status

        .BCM_CentralLampSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_BCAN_11_T),
                .dataType = 0,
                .startBit = 4,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Central Lamp status

        .BCM_HeadLightLevelingSwSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_BCAN_11_T),
                .dataType = 0,
                .startBit = 5,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // BCM Head light level Virtual Switch Status

        .BCM_IHBColourSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_BCAN_11_T),
                .dataType = 0,
                .startBit = 14,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // BCM IHBC Colour State

        .IBCM_IgVoltage =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCM_BCAN_11_T),
                .dataType = 0,
                .startBit = 40,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // IBCM feedback the IG line voltage

        .HVACF_AirCompressorReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Air compressor request

        .HVACF_RearDefrostSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 2,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear window and  mirror heater activation status

        .HVACF_CorrectedExterTempVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 3,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // External temperature validity

        .HVACF_ACSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 6,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The HVACF A/C state display

        .HVACF_ACmaxSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 7,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The HVACF A/C max state display

        .HVACF_CorrectedExterTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // External temperature

        .HVACF_RawExterTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 16,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Raw external temperature

        .HVACF_TempSelectAuto =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 5,
                .resulotion = 0.5,
                .offset = 18,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The  temperature selected on auto HVACF

        .HVACF_AutoSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 30,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The HVACF auto state display

        .HVACF_Type =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 31,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The HVACF type

        .HVACF_WindExitMode =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 32,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The wind exit mode

        .HVAC_IonCleanFunction =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 35,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicates whether HVACF has Ionclean

        .HVACF_TelematicsSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 40,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // HVACF Telematics Status

        .HVACF_AirCirculationSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 45,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Airconditionor air circulation state

        .HVACF_DriverTempSelect =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 49,
                .bitLength = 5,
                .resulotion = 0.5,
                .offset = 18,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The selected temperature  of Driver side

        .HVACF_IonMode =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 54,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Ion mode

        .HVACF_WindExitSpd =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The wind exit speed

        .HVACF_PsnTempSelect =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 60,
                .bitLength = 5,
                .resulotion = 0.5,
                .offset = 18,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The selected temperature  of passenger side

        .HVACF_RawCabinTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 64,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Raw cabin temperature

        .HVACF_CorrectedCabinTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 72,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Corrected cabin temperature

        .HVACF_RawCabinTempVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 80,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Signal HVACF_RawCabinTemp validity

        .HVACF_CorrectedCabinTempVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 81,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Signal HVACF_CorrectedCabinTemp validity

        .HVACF_EngTorqReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 88,
                .bitLength = 8,
                .resulotion = 0.1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Engine torque request

        .HVACF_RearAutoSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 123,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The rear HVAC auto state display

        .HVACF_RearWindExitMode =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 124,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The rear wind exit mode

        .HVACF_PM25Value =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 150,
                .bitLength = 10,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PM2.5

        .HVACF_PM10Value =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 156,
                .bitLength = 10,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PM10

        .HVACF_PM25ValueVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 160,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // indicate signal “HVACF_PM2.5 Value”Validity

        .HVACF_PM10ValueVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 161,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // indicate signal “HVACF_PM10 Value”Validity

        .HVACF_PM25Sensormaintaininfo =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 163,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PM2.5 Sensor maintain information

        .HVACF_PM25SensormaintaininfoVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 168,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // indicate signal “HVACF_PM2.5 Sensor maintain info”Validity

        .HVACF_PM25Sns =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 169,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicates  whether HVACF has PM2.5 sensor or not

        .HVACF_RearCorrectedCabinTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 200,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .HVACF_RearEvaporatorTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 208,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear evaporator temprature

        .HVACF_FrontEvaporatorTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 216,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front evaporator temprature

        .HVACF_RearCorrectedCabinTempVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_HVACF_1_T),
                .dataType = 0,
                .startBit = 240,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear corrected cabin temperature validity

        .HVSM_FLHeatingActLevel =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SCM_HVSM_1_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front left seat heating actual level

        .HVSM_FLSeatTempVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SCM_HVSM_1_T),
                .dataType = 0,
                .startBit = 3,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front left seat ventilating actual level validity

        .HVSM_FRHeatingActLevel =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SCM_HVSM_1_T),
                .dataType = 0,
                .startBit = 4,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front right seat heating actual level

        .HVSM_FRSeatTempVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SCM_HVSM_1_T),
                .dataType = 0,
                .startBit = 7,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front right seat ventilating actual level validity

        .HVSM_FLVentilatingActLevel =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SCM_HVSM_1_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front left seat ventilating actual level

        .HVSM_FLHVSMAutoModeSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SCM_HVSM_1_T),
                .dataType = 0,
                .startBit = 11,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front left seat heating/ventilating auto mode status

        .HVSM_FRVentilatingActLevel =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SCM_HVSM_1_T),
                .dataType = 0,
                .startBit = 12,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front right seat ventilating actual level

        .HVSM_FRHVSMAutoModeSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SCM_HVSM_1_T),
                .dataType = 0,
                .startBit = 15,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front right seat heating/ventilating auto mode status

        .HVSM_FLSeatTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SCM_HVSM_1_T),
                .dataType = 0,
                .startBit = 16,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front left seat temperature

        .HVSM_FRSeatTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SCM_HVSM_1_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front right seat temperature

        .PLGM_Latchst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PLGM_1_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Tail door latch status

        .PLGM_Doorst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PLGM_1_T),
                .dataType = 0,
                .startBit = 5,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Tail door status

        .PLGM_LatchCinchedSW =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PLGM_1_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Cinching Latch Cinched switch status

        .PLGM_ModeSW =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PLGM_1_T),
                .dataType = 0,
                .startBit = 11,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PLG mode switch staus

        .PLGM_operationmode =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PLGM_1_T),
                .dataType = 0,
                .startBit = 14,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Present operation mode

        .PLGM_BuzzerReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PLGM_1_T),
                .dataType = 0,
                .startBit = 41,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Buzzer request mode

        .PLGM_Remotecontrolst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_PLGM_1_T),
                .dataType = 0,
                .startBit = 51,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Remote control mode and leave out of remote control mode reason

        .GWM_NodeAddress =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GWM_TCAN_NM),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // GWM node adress

        .GWM_NMDataField =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GWM_TCAN_NM),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 56,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // GWM Network management data

        .GW_LowBatPreWarning =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_1_T),
                .dataType = 0,
                .startBit = 5,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Low battery warning level

        .GW_RemoteDiagCode =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_1_T),
                .dataType = 0,
                .startBit = 18,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The flag that ECU need to be remote diagnosed

        .NMData =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_NMData),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 64,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .EPB_SwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB switch status

        .EPB_SwitchStVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 2,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB switch status validity

        .EPB_ExtAvailable =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 3,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB  status,to indicate wether

        .EPB_ActuatorSt_L =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 5,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB left actuator status

        .EPB_SysSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB system status

        .EPB_AchievedClampForce =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 11,
                .bitLength = 5,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB clamp force

        .EPB_DynamicApplySt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 27,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB dynamic apply status

        .EPB_MotorCurrent_L =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 30,
                .bitLength = 10,
                .resulotion = 0.1,
                .offset = -51.1,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB left motor current

        .EPB_CruiseCtrlCancelReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 37,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB cruise control cancel request

        .EPB_CruiseCtrlCancelReqVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 38,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB curise control cancel request validity

        .EPB_ActuatorSt_R =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 40,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB right  actuator status

        .EPB_MotorCurrent_R =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 43,
                .bitLength = 10,
                .resulotion = 0.1,
                .offset = -51.1,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB rignt motor current

        .EPB_1_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Message counter

        .EPB_TMMCtrlCmd =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 52,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // TMM control command

        .EPB_SwitchSt_R =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 53,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB switch status Right MCU detected

        .EPB_FailSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 55,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB fail status,to indicate wether EPB can response to external apply  request or not

        .EPB_1_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Message checksum

        .EPB_CmdDecelReqProtValue =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 76,
                .bitLength = 12,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB command deceleration request protect value

        .EPB_CmdDecelReqSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 80,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB command:deceleration request status

        .EPB_ReClampRequestToBCS =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 81,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB reclamp request to BCS

        .EPB_ReClampRequestToBCSVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 82,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB reclamp request to BCS validity

        .EPB_ActVehLongAccelVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 83,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Actual longitudinal acceleration validity

        .EPB_2_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 84,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The message counter is a 0-15 counter, increased of one unit on every message transmission

        .EPB_SwitchSt_L =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 98,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB switch status Left MCU detected

        .EPB_CmdDecelReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 100,
                .bitLength = 12,
                .resulotion = 0.01,
                .offset = -20.48,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB command:deceleration request

        .EPB_ActVehLongAccel =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 112,
                .bitLength = 8,
                .resulotion = 0.1,
                .offset = -12.7,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Actual longitudinal acceleration

        .EPB_2_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 120,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .EPB_SysWarnIndReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 128,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB system warning indication request

        .EPB_SysAudWarnReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 130,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB system audible warning request

        .EPB_SysBrkLightsReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 133,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB system brake lights request

        .EPB_SysStIndReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 134,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB system status indication request

        .EPB_DragMode =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 136,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Drag Mode status

        .EPB_StpOnBrkToRelParkBrkIndOn =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 138,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Step on brake to release park brake Indication on

        .EPB_SysDispMsgReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 140,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EPB system display message request

        .EPB_3_P_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 148,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The message counter is a 0-15 counter, increased of one unit on every message transmission

        .EPB_3_P_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_EPB_1_T),
                .dataType = 0,
                .startBit = 184,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .BCS_BrakeControllerTyp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Brake system type ABS or ESP

        .BCS_ABSActiveSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 1,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indication that ABS system currently performs a brake intervention

        .BCS_ABSFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 2,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // ABS system has detected a failure which does no allow a reliable ABS regulation and is therefore switched off

        .BCS_EBDFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 3,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // ABS system has detected a heavy fault, which does not even allow a reliable electronic brake distribution and is therefore completely shut down

        .BCS_EBDActiveSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 4,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indication that EBD system currently performs a brake/engine intervention

        .BCS_TCSFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 5,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // TCS is switched off due to detected failure

        .BCS_TCSActiveSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 6,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indication that TCS system currently performs a brake/engine intervention

        .BCS_VDCFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 7,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // VDC is switched off due to detected failure

        .BCS_EngTorqDecFast =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // This torque  is engine combustion(indicated) decreased target torque (fast)

        .BCS_EngTorqDecSlow =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 16,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // This torque  is engine combustion(indicated)decreased target torque (slow)

        .BCS_EngTorqInc =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // This torque  is  increased engine combustion(indicated)target torque

        .BCS_VehSpdVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 37,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Quality/fault information to vehicle speed

        .BCS_EngTorqDecActiveSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 38,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Decrease enginetorque  active status (fast and low)

        .BCS_ESPShiftInterference =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 39,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Request TCM prohibit the gear shift and hold the current gear position

        .BCS_VehSpd =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 40,
                .bitLength = 13,
                .resulotion = 0.05625,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Vehicle speed

        .BCS_2_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The message counter is a 0-15 counter, increased of one unit on every message transmission

        .BCS_EngTorqIncActiveSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 52,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Increased engine torque  active status

        .BCS_VDCActiveSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 53,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indication that VDC system currently performs a brake/engine intervention

        .BCS_BrkLightOn =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 54,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Request to ignite the brake light

        .BCS_ESPShiftInterferenceVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 55,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Singal BCS_ESPShiftInterference validity

        .BCS_2_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .BCS_HBBIntervention =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 116,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // This signal indicates if HBB is active or not

        .BCS_HBBStatus =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 117,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // This signal indicates the status of HBB

        .BCS_FLWheelSpdEdgesSum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 140,
                .bitLength = 12,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Accumulated sum of rising and falling edges of front left wheel speed sensor output·since IGN ON( the wheel plate have 43 teeth/cycle,the edge is 86)

        .BCS_FRWheelSpdEdgesSum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 144,
                .bitLength = 12,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Accumulated sum of rising and falling edges of front right wheel speed sensor output·since IGN ON( the wheel plate have 43 teeth/cycle,the edge is 86)

        .BCS_RLWheelSpdEdgesSum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 164,
                .bitLength = 12,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Accumulated sum of rising and falling edges of rear left wheel speed sensor output·since IGN ON( the wheel plate have 43teeth/cycle,the edge is 86)

        .BCS_RRWheelSpdEdgesSum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 168,
                .bitLength = 12,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Accumulated sum of rising and falling edges of  rear right wheel speed sensor output·since IGN ON( the wheel plate have 43 teeth/cycle,the edge is 86)

        .BCS_FLWheelSpdEdgesSumVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 176,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Quality/fault information to current wheel speed edges sum of front left wheel

        .BCS_FRWheelSpdEdgesSumVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 177,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Quality/fault information to current wheel speed edges sum of front right wheel

        .BCS_RLWheelSpdEdgesSumVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 178,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Quality/fault information to current wheel speed edges sum of rear left wheel

        .BCS_RRWheelSpdEdgesSumVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 179,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Quality/fault information to current wheel speed edges sum of rear right wheel

        .BCS_5_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 180,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The message counter is a 0-15 counter, increased of one unit on every message transmission

        .BCS_5_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 184,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .BCS_YawRateSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 196,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Yaw rate status

        .BCS_YawRate =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 200,
                .bitLength = 12,
                .resulotion = 0.0009765625,
                .offset = -2.0943,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Yaw rate

        .BCS_YawRateOffset =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 216,
                .bitLength = 12,
                .resulotion = 0.0009765625,
                .offset = -0.13,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Yaw rate offset

        .BCS_HHCCtrlSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 274,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // HHC(Hill Hold Control) control status

        .BCS_HDCCtrlSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 275,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // HDC(Hill Descent Control) control status

        .BCS_HHCErrSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 277,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // HHC error status

        .BCS_HDCErrSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 278,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // HDC error status

        .BCS_VLCActive =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 279,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicating whether VLC(Vehicle  Longitudinal Control) actively controls the target acceleration or not

        .BCS_CruiseCtrlCancelReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 280,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // BCS  request EMS to cancel cruise control

        .BCS_CDPIntervention =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 282,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // CDP(Controlled Deceleration for Parking brake) intervention

        .BCS_VLCAvailable =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 287,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicating whether VLC(Vehicle  Longitudinal Control) is basically ready for activation or not

        .BCS_VLCInternalTargetAccel =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 288,
                .bitLength = 8,
                .resulotion = 0.05,
                .offset = -7,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Internal VLC(Vehicle  Longitudinal Control) target acceleration

        .BCS_VLCEngTorqReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 296,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // VLC(Vehicle  Longitudinal Control) engine torque request

        .BCS_VLCFail =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 304,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // indicating whether VLC is error or not.

        .BCS_ParkingReqToEPB =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 305,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicates the release/lock brake request to EPB

        .BCS_AVHSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 307,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicates AVH(Auto Vehicle Hold)in active or not

        .BCS_AVHStandby =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 308,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicates AVH(Auto Vehicle Hold)in standby mode or not

        .BCS_AVHAvailable =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 309,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // HMI responsible, can get degrate AVH(Auto Vehicle Hold) statement from DSW and send out

        .BCS_VLCEngTorqReqAct =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 310,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // VLC(Vehicle  Longitudinal Control)engine torque request active

        .BCS_VLCTempOff =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 311,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // When brake disc is overtemperature,VLC(Vehicle  Longitudinal Control) turn off

        .BCS_NoBrakeForce =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 324,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // No brake pressure applied,generally it regards wheel cylinder presure

        .BCS_BrakeOverHeat =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 325,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicating brake temperature too high or not

        .BCS_MasterCylinderPrVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 326,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The valid value of BCS_MasterCylinderPr

        .BCS_MasterCylinderPrOffsetVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 327,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Master cylinder pressure offset valid value

        .BCS_MasterCylinderPr =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 328,
                .bitLength = 12,
                .resulotion = 0.1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Master cylinder pressure

        .BCS_ABAactive =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 338,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // ABA(Adaptive Brake Assistance) is active

        .BCS_ABAavailable =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 339,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The ABA(Adaptive Brake Assistance)  function is available

        .BCS_8_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 340,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .BCS_MasterCylinderPrOffset =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 344,
                .bitLength = 10,
                .resulotion = 0.1,
                .offset = -15,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Master cylinder pressure offset

        .BCS_ABPActive =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 352,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // ABP(Automatic brake Pre-fill) is active

        .BCS_ABPAvailable =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 353,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // ABP(Automatic brake Pre-fill) is available

        .BCS_AEBActive =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 354,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // AEB(Automatic Emergency

        .BCS_AEBAvailable =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 355,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // AEB(Automatic Emergency

        .BCS_AWBActive =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 356,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // AWB(Automatic Warning Brake) is active

        .BCS_AWBAvailable =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 357,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // AWB(Automatic Warning Brake) is available

        .BCS_CDDActive =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 358,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // CDD(Controlled Deceleration for Driver) is active

        .BCS_CDDAvailable =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 359,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // CDD(Controlled Deceleration for Driver) is available

        .BCS_CDDSActiveVehHold =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 360,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicates if CDD-S can hold the vehicle in standstill

        .BCS_VehicleStandStillSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 361,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicating whether the vehicle is standstill or not;

        .BCS_AEBIBActive =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 363,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicates AEBIB active or not, not really necessary since AEB Act and AEBBA act is available and could get the AEBIB active

        .BCS_AEBBAActive =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 364,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicates AEBBA active or not

        .BCS_CDDFail =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 367,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // CDD in failure

        .BCS_QDCACC =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 368,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // ACC error detected by ESP, like timeout or alivecounter error.

        .BCS_PEDPAvailable =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 370,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEDP is available

        .BCS_PEDPActive =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 371,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // PEDP is active

        .BCS_HazardActiveReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 372,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Request to active hazard

        .BCS_8_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 376,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .BCS_FLWheelRotatedDirection =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 390,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // FL wheel rotate direction

        .BCS_FLWheelRotatedDirectionVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 391,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // FL wheel rotate direction validity

        .BCS_FLWheelSpd =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 400,
                .bitLength = 13,
                .resulotion = 0.05625,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Current wheel speed information of front left wheel

        .BCS_FRWheelRotatedDirectionVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 413,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // FR wheel rotate direction validity

        .BCS_FRWheelRotatedDirection =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 414,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // FR wheel rotate direction

        .BCS_FRWheelSpd =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 416,
                .bitLength = 13,
                .resulotion = 0.05625,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Current wheel speed information of front left wheel

        .BCS_9_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 432,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The message counter is a 0-15 counter, increased of one unit on every message transmission

        .BCS_9_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 440,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .BCS_RLWheelRotatedDirection =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 454,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // RL wheel rotate direction

        .BCS_RLWheelRotatedDirectionVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 455,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // RL wheel rotate direction validity

        .BCS_RLWheelSpd =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 464,
                .bitLength = 13,
                .resulotion = 0.05625,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Current wheel speed information of front left wheel

        .BCS_RRWheelRotatedDirectionVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 477,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // RR wheel rotate direction validity

        .BCS_RRWheelRotatedDirection =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 478,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // RR wheel rotate direction

        .BCS_RRWheelSpd =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 480,
                .bitLength = 13,
                .resulotion = 0.05625,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Current wheel speed information of RRont left wheel

        .BCS_10_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 496,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The message counter is a 0-15 counter, increased of one unit on every message transmission

        .BCS_10_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_BCS_1_T),
                .dataType = 0,
                .startBit = 504,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .SRS_CrashOutputSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_1_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // To inform there's crash event happen in last 2sec.

        .SRS_Front_CrashOutputSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_1_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Front CrashOutput Status.To inform there's crash event happenin last 2 sec. After 2 sec, the signal willturn to 0.

        .SRS_LeftSide_CrashOutputSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_1_T),
                .dataType = 0,
                .startBit = 9,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Left Side CrashOutput Status.To inform there's crash event happenin last 2 sec. After 2 sec, the signal willturn to 0.

        .SRS_RightSide_CrashOutputSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_1_T),
                .dataType = 0,
                .startBit = 10,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Right Side CrashOutput Status.To inform there's crash event happenin last 2 sec. After 2 sec, the signal willturn to 0.

        .SRS_Rear_CrashOutputSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_1_T),
                .dataType = 0,
                .startBit = 11,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rear CrashOutput Status.To inform there's crash event happenin last 2 sec. After 2 sec, the signal willturn to 0.

        .SRS_RolloverCrashOutpuSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_1_T),
                .dataType = 0,
                .startBit = 12,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Rollover CrashOutput Status.To inform there's crash event happenin last 2 sec. After 2 sec, the signal willturn to 0.

        .SRS_NearCrashOutputSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_1_T),
                .dataType = 0,
                .startBit = 16,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // To inform there's near crash eventhappen in last 2sec.After 2 sec, the signal will turn to 0

        .SRS_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_1_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Message counter

        .SRS_1_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_1_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Checksum of the crash output status

        .SRS_PsngrSeatBeltSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_2_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // To identify whether Passenger Seat Belt is buckled

        .SRS_DriverSeatBeltSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_2_T),
                .dataType = 0,
                .startBit = 2,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // To identify whether Driver Seat Belt is buckled

        .SRS_RearLeftSeatBeltSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_2_T),
                .dataType = 0,
                .startBit = 4,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // To identify whether

        .SRS_AirbagFaultSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_2_T),
                .dataType = 0,
                .startBit = 6,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // To illuminate or turn off airbag warning indicator

        .SRS_RearRightSeatBeltSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_2_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // To identify  whether

        .SRS_RearMiddleSeatBeltSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_2_T),
                .dataType = 0,
                .startBit = 13,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // To identify whether

        .SRS_2_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_2_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The message counter is a 0-15 counter, increased of one unit on every message transmission

        .SRS_2_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_SRS_2_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Message checksum

        .EMS_MaxEngTorqNorm =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 0,
                .bitLength = 8,
                .resulotion = 10,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Max Engine combustion（indicated）Torque Norm

        .EMS_EngTorqTargetWOTCU =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Target Engine combustion（indicated）Torque Without TCU intervention

        .EMS_CANErr =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 20,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // CAN  message unplausible

        .EMS_EngIdleRefSpd =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 12,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Engine Idle reference speed

        .EMS_EngWaterTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 32,
                .bitLength = 8,
                .resulotion = 1,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Engine Water Temperature

        .EMS_OverRideSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 40,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicate driver's need for acceration is bigger than virtual acceration  while in cruise mode

        .EMS_EngTorqTargetWOTCUVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 41,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // "EMS_EngTorqTargetWOTCU" Valid Data

        .EMS_CruiseCtrlActive =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 42,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Cruise Control Active for display

        .EMS_BrakePedalStVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 44,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // "EMS_BrakePedalSt" valid data

        .EMS_BrakePedalSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 45,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Brake pedal status

        .EMS_EngWaterTempVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 46,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // "EMS_EngWaterTemp" valid data

        .EMS_BrkLightSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 47,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Brake Pedal Nomally Closed Switch Status

        .EMS_1_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Check the MessageCounter consistency at least on every message reception

        .EMS_TorqControlInhibit =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 52,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Set when a torque request can not be fulfilled. Fail safe operation

        .EMS_OBDDrivingCycleSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 53,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Diagnosis

        .EMS_OBDWarmUpCycleSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 54,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Diagnosis

        .EMS_ClutchPedalSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 55,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Clutch Pedal Status

        .EMS_1_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 56,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Message checksum

        .EMS_CompressorSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 64,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Compressor Status

        .EMS_EngTorqDriverReqVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 66,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // "EMS_EngTorqDriverReq" valid data

        .EMS_EngTorqVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 67,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // current engine combustion Torque status

        .EMS_CatalyticIgnitionSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 68,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Catalytic converter ignition state

        .EMS_EngFrictionTorqVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 69,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // "EMS_EngFrictionTorq" valid data

        .EMS_PTOpenRq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 70,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EMS request TCU open power train

        .EMS_IdleSpdInfo =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 71,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Idle Speed Information

        .EMS_EngTorq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 72,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Current engine combustion(indicated) torque

        .EMS_EngSpd =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 88,
                .bitLength = 16,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Engine Speed

        .EMS_EngTorqDriverReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 96,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Engine combustion(indicated) torque requested by driver or cruise control

        .EMS_EngFrictionTorq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 104,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Torque losses caused by engine mechanical losses，gas change and torque demands of auxiliary loads

        .EMS_2_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 112,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Check the MessageCounter consistency at least on every message reception

        .EMS_EngSpdVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 116,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // "EMS_EngSpd" valid data

        .EMS_CruiseFunInfo =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 117,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Cruise button information

        .EMS_2_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 120,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .EMS_MinEngTorq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 128,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Minimum Engine combustion Torque

        .EMS_MaxEngTorq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 136,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Maximum indicated engine combustion torque that would be available at current engine speed, current enviromental conditions.

        .EMS_3_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 144,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Check the MessageCounter consistency at least on every message reception

        .EMS_AtmosphericPressureVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 148,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Atmospheric Pressure Valid Data

        .EMS_BattVoltVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 149,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery voltage Valid Data

        .EMS_MaxEngTorqVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 150,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Maximum Engine Torque Valid Data

        .EMS_MinEngTorqVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 151,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // MinimumEngineTorque Valid Data

        .EMS_BattVolt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 152,
                .bitLength = 8,
                .resulotion = 0.1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery voltage

        .EMS_AtmosphericPressure =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 160,
                .bitLength = 8,
                .resulotion = 5,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Atmospheric Pressure

        .EMS_MotronicSignalRecovery =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 168,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Motronic Signal Recovery

        .EMS_BattIndSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 176,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // To ignite the charging indication light in instrument cluster

        .EMS_EcoLevel =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 180,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Eco mode level indication

        .EMS_3_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 184,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            },

        .EMS_MilSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 192,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Emission malfunction indication

        .EMS_EPCSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 193,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Engine usual malfunction

        .EMS_EngSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 194,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Engine running status

        .EMS_EMSReleased =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 195,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EMS released after decode

        .EMS_StartStopSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 197,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // It is for the Start&Stop management status display

        .EMS_FuelLevelVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 199,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Fuel level Valid Data

        .EMS_FuelLevel =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 200,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Fuel level

        .EMS_FuelPulse =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 216,
                .bitLength = 16,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Fuel consumption pulse

        .EMS_BattSOC =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 224,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // State of Charge of the Battery

        .EMS_EngCrankSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 232,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Engine crank status

        .EMS_BattSOCVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 233,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // State of Charge of the Battery validity

        .EMS_BattTempVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 234,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery temperature validity

        .EMS_IURPGenDen =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 236,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // indicates the update permission state of general denominator

        .EMS_IUPRGenInh =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 237,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // indicates the general denominator related pending fault state

        .EMS_StartStopSwitchSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 238,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Start and Stop switch status

        .EMS_IUPRIgCnt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 239,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // indicates the update permission state of IG counter

        .EMS_StartStopWarn =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 245,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Start and Stop state warning information on ICM

        .EMS_BattTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 248,
                .bitLength = 8,
                .resulotion = 1,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery temperature

        .EMS_GasPedalActPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 256,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Current accelerate pedal actual position

        .EMS_GasPedalActPstProtValue =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 264,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Current accelerate pedal actual position  protect value

        .EMS_6_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 272,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Message counter

        .EMS_GasPedalActPstVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 276,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Current accelerate pedal actual position validity

        .EMS_GearNeutralPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 277,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // (only for MT)Gear neutral positon

        .EMS_GearNeutralPstVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 278,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // (only for MT)Gear neutral positon validity

        .EMS_FuelCutOff =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 280,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Engine in Fuel Cut Off

        .EMS_AutoStopRq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 282,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EMS auto stop request

        .EMS_ClutchBottomPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 283,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Clutch pedal bottom sensor state

        .EMS_ClutchBottomPstVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 284,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EMS_ClutchBottomPst valid value

        .EMS_EngineStartStopSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 285,
                .bitLength = 3,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // It is set to Engine Stop when the Engine is turned off by the Start&Stop management

        .EMS_R2ControlInhibit =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 288,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Inhibit R2 control

        .EMS_TargetGear =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 289,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // (only for MT) Target gear

        .EMS_GearShiftIndication =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 293,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // (Only for MT) To indicate driver to shift gear

        .EMS_KickdownSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 295,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicate gas pedal kick-down state

        .EMS_DriverPowerReq =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 296,
                .bitLength = 8,
                .resulotion = 0.78125,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // It is the engine power set-point calculated in normal running conditions or during gearshift

        .EMS_DriverPowerReqProtValue =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 304,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Signal"EMS_DriverPowerReq" protect value

        .EMS_6_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 312,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Message checksum

        .EMS_EngIntakeAirTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 320,
                .bitLength = 8,
                .resulotion = 0.75,
                .offset = -48,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Intake air temperature

        .EMS_EngIntakeAirTempVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 328,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // "EMS_EngIntakeAirTemp" validity

        .EMS_R1CtrlSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 329,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // R1 relay control status

        .EMS_DrvMode =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 330,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Current drving mode

        .EMS_ATSReqRefuse =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 331,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Refuse to response to ATS request,  for EMS failure

        .EMS_ACCButtInfo =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 336,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // ACC Button Information

        .EMS_BrakeVacuumPressureVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 354,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Brake Vacuum Pressure Valid Data

        .EMS_GasPedalActPstforMRR =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 344,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EMS_GasPedalActPstforMRR

        .EMS_GasPedalActPstforMRRVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 359,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EMS_GasPedalActPstforMRR Valid Data

        .EMS_BrakeVacuumPressure =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 360,
                .bitLength = 10,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Brake Vacuum Pressure(BVP=AtmosphericPressure-BoosterInnerAbsolutePressure)

        .EMS_9_MsgCounter =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 368,
                .bitLength = 4,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Message counter

        .EMS_9_Checksum =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 376,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Message checksum

        .EMS_CruiseSpdSet =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 384,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // EMS cruise speed set value

        .EMS_GearNeutralPst1 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 392,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Gear neutral sensor1 position

        .EMS_GearNeutralPst2 =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 400,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Gear neutral sensor2 position

        .EMS_ClutchPst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 408,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Clutch position

        .EMS_FuelRailPr =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 416,
                .bitLength = 8,
                .resulotion = 2,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // GDI project Fuel Rail Pressure

        .EMS_OBDDrivingCycleStForPDTC =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 424,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Diagnosis,DrivingCycle status after OBD $04 clear the PDTC

        .EMS_EngOilPreLowLamp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 425,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Engine oil low pressure warnning Lamp State

        .EMS_GPFLightSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 426,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // GPF Indicator Light State

        .EMS_GearNeutralPst1VD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 428,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The valid value of EMS_GearNeutralPst1

        .EMS_GearNeutralPst2VD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 429,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The valid value of

        .EMS_ClutchSensorVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 430,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The valid value of Clutch sensor

        .EMS_FuelRailPrVD =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 431,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // The valid value of GDI project Fuel Rail Pressure

        .EMS_EngineCalculatedLoad =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 432,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Diagnosis,Engine load Calculated by EMS

        .EMS_AbsoluteThrottlepst =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EMS_1_T),
                .dataType = 0,
                .startBit = 440,
                .bitLength = 8,
                .resulotion = 0.392,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Absolute Throttle position

        .EBS_BattVolt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 10,
                .bitLength = 14,
                .resulotion = 0.0009765625,
                .offset = 3,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery voltage

        .EBS_CurrentRange =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 8,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery current range

        .EBS_BattCurrent =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 24,
                .bitLength = 16,
                .resulotion = 1,
                .offset = -32768,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery current

        .EBS_BattTemp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 32,
                .bitLength = 8,
                .resulotion = 1,
                .offset = -40,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery temperature

        .EBS_BattQuiescentCurrent =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 40,
                .bitLength = 8,
                .resulotion = 0.00390625,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery quiescent current

        .EBS_CurrentSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 48,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // current status

        .EBS_VoltSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 50,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // voltage Status

        .EBS_TempSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 52,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Temperature Status

        .EBS_CalibrationErr =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 54,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Error in calibration data

        .EBS_IdentificationErr =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 55,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // error in ECU indentification

        .EBS_Response_Err =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 63,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Response Error

        .EBS_BattInconsistencyFlag =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 62,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery inconsistency flag

        .EBS_BattDisconectionFlag =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 61,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery disconnection flag

        .EBS_SOC =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 64,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // State of Charge

        .EBS_SOH_SUL =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 72,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Max.possible SOC,reduced by sulphation

        .EBS_SOFV_StopEnable =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 80,
                .bitLength = 8,
                .resulotion = 0.0625,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery voltage at cranking peak with per-discharge

        .EBS_SOFV_Restart =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 88,
                .bitLength = 8,
                .resulotion = 0.0625,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Battery voltage at cranking peak without per-discharge

        .EBS_SOH_LAM =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 96,
                .bitLength = 8,
                .resulotion = 0.5,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // State of health(Loss of active mass)

        .EBS_SOH_COR =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 104,
                .bitLength = 8,
                .resulotion = 0.0625,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // State of health(corrosion)

        .EBS_SOC_STATE =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 114,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // SOC Status

        .EBS_SOH_SUL_St =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 116,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // SOH_SUL Status

        .EBS_SOFV_StopEnable_St =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 118,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // SOFV_StopEnable Status

        .EBS_SOFV_Restart_St =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 120,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // SOFV_Restart Status

        .EBS_SOH_LAM_St =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 122,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // SOH_LAM Status

        .EBS_SOH_COR_St =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 124,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // SOH_COR Status

        .EBS_Q_Charge =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 136,
                .bitLength = 16,
                .resulotion = 0.125,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // accumulated battery charge

        .EBS_Q_Discharge =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 152,
                .bitLength = 16,
                .resulotion = 0.125,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // accumulated battery discharge

        .EBS_DisChargeCurrWakeUpSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 192,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // DisCharge Current Wakeup function enable status

        .EBS_ChargeCurrWakeUpSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 193,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Charge Current Wakeup function enable status

        .EBS_SOCWakeUpSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 194,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Low  SOC_High Wakeup function enable status

        .EBS_DischargeCurrWakeUp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 195,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicate if the discharge current is bigger than the threshold

        .EBS_ChargeCurrWakeUp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 196,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicate if the charge current is bigger than the threshold

        .EBS_SOCWakeUp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 197,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicate if the SOC is  smaller than the threshold

        .EBS_SOC2WakeEnSt =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 198,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Low SOC Low Wakeup function enable status

        .EBS_SOC2WakeUp =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 199,
                .bitLength = 1,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicate if the SOC is  smaller than the threshold

        .EBS_DisChargeCurrWakeUpTreshold =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 208,
                .bitLength = 16,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // the DisCharge Current wakeup threshold value.

        .EBS_SOC2WakeUpTreshold =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 224,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // the LOwSOC-Low wakeup threshold value

        .EBS_SOCWakeUpTreshold =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 232,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // the SOC wakeup threshold value.

        .EBS_WakeupEnTime =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 240,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Wakeup enable time after Lin OFF

        .EBS_WakeupIntervalTime =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_EBS_1_T),
                .dataType = 0,
                .startBit = 248,
                .bitLength = 8,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // IBS  wake-up internal time  after Master without response with previous wake-up block.

        .ACU_Key2St =
            {
                .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
                .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_ACU_1_T),
                .dataType = 0,
                .startBit = 258,
                .bitLength = 2,
                .resulotion = 1,
                .offset = 0,
                .useInvalidFlag = 1,
                .InvalidData = 0xFFFFFFFF,
            }, // Indicate if the charge current is bigger than the threshold

        .ACU_TELBcallReq =
        {
            .msgBufferPointer = CAN_MSG_BUFFER_ADDRESS(0),
            .canBufferIdIndex = CAN_ID_TO_BUFFER_INDEX(0, GW_ACU_1_T),
            .dataType = 0,
            .startBit = 316,
            .bitLength = 2,
            .resulotion = 1,
            .offset = 0,
            .useInvalidFlag = 1,
            .InvalidData = 0xFFFFFFFF,
        }, // Indicate if the charge current is bigger than the threshold


};
#endif

/****************************** Function Declarations *************************/

/****************************** Public Function Implementations ***************/

/*************************************************
   Function:        GetCan0SignalConfigure
   Description:     Get the pointer to CAN0 signal configuration structure
   Input:           None
   Output:          None
   Return:          Pointer to m_can0SignalConfigure structure
   Others:          None
 *************************************************/
const can0_signal_configure_t *GetCan0SignalConfigure(void)
{
    return &m_can0SignalConfigure;
}

/*************************************************
   Function:        Can0RxCanMsgDispatch
   Description:     Dispatch received CAN0 message to corresponding buffer
   Input:           canId - CAN message ID
                    dlc - Data length code
                    pData - Pointer to received data
   Output:          None
   Return:          0 on success, 1 on failure
   Others:          Finds matching buffer based on CAN ID and stores data
 *************************************************/
int16_t Can0RxCanMsgDispatch(uint32_t canId, uint8_t dlc, uint8_t *pData)
{
    int16_t ret = 1;
    uint32_t i;
    canId_map_configure_t *pCanConfigure;
    can_signal_msg_buffer_t *pBufferElement;

    if ((pData != NULL) && (dlc != 0))
    {
        if (dlc > CAN_V_FD_MAX_DATA_SIZE)
        {
            dlc = CAN_V_FD_MAX_DATA_SIZE;
        }
        pCanConfigure = (canId_map_configure_t *)CAN_ID_CONFIGURE_POINTER(0);
        for (i = 0; i < CAN_MSG_BUFFER_SIZE(0); i++)
        {
            if (pCanConfigure[i].canId == (canId & 0x7FFFFFFF))  // 掩去 bit31 扩展帧标志位进行比较
            {
                pBufferElement = &CAN_MSG_BUFFER_GET_ELEMENT(0, i);
                memcpy(pBufferElement->canData, pData, dlc);
                pBufferElement->rxFlag = 1;
                ret = 0;
                break;
            }
        }
    }
    return ret;
}

/*************************************************
   Function:        Can0RxCanMsgCycleCheck
   Description:     Check CAN0 message reception cycle and handle timeout
   Input:           cycleTime - Current cycle time
   Output:          None
   Return:          0 on success
   Others:          Updates message reception status, resets timeout flags,
                    and handles timeout conditions
 *************************************************/
int16_t Can0RxCanMsgCycleCheck(uint32_t cycleTime)
{
    uint32_t i;
    canId_map_configure_t *pCanConfigure;

    pCanConfigure = (canId_map_configure_t *)CAN_ID_CONFIGURE_POINTER(0);
    for (i = 0; i < CAN_MSG_BUFFER_SIZE(0); i++)
    {
        if (CAN_MSG_BUFFER_GET_ELEMENT(0, i).rxFlag)
        {
            CAN_MSG_BUFFER_GET_ELEMENT(0, i).rxFlag = 0;
            CAN_MSG_BUFFER_GET_ELEMENT(0, i).cyleTimeCount = 0;
            CAN_MSG_BUFFER_GET_ELEMENT(0, i).cycleTimeOutFlag = 0;
        }
        else
        {
            CAN_MSG_BUFFER_GET_ELEMENT(0, i).cyleTimeCount += cycleTime;
            if ((CAN_MSG_BUFFER_GET_ELEMENT(0, i).cyleTimeCount) > (10 * pCanConfigure[i].timeOutValue))
            {
                CAN_MSG_BUFFER_GET_ELEMENT(0, i).cyleTimeCount = 0;
                CAN_MSG_BUFFER_GET_ELEMENT(0, i).cycleTimeOutFlag = 1;
                CAN_MSG_BUFFER_GET_ELEMENT(0, i).rxFlag = 0;
            }
        }
    }
    return 0;
}

/*************************************************
   Function:        Can0GetRxFlagByCanId
   Description:     获取指定CAN ID的接收标志状态
   Input:           canId - 要查询的CAN ID
   Output:          None
   Return:          收到则返回1，未收到则返回0
   Others:
 *************************************************/
uint8_t Can0GetRxFlagByCanId(uint32_t canId)
{
    uint32_t i;
    uint8_t rxFlag = 0;
    canId_map_configure_t *pCanConfigure;

    pCanConfigure = (canId_map_configure_t *)CAN_ID_CONFIGURE_POINTER(0);
    for (i = 0; i < CAN_MSG_BUFFER_SIZE(0); i++)
    {
        if (pCanConfigure[i].canId == canId)
        {
            rxFlag = CAN_MSG_BUFFER_GET_ELEMENT(0, i).rxFlag;
            break;
        }
    }

    return rxFlag;
}

/*************************************************
   Function:        Can0ClearRxFlagByCanId
   Description:     清除指定CAN ID的接收标志
   Input:           canId - 要清除的CAN ID
   Output:          None
   Return:          清除成功返回0
   Others:
 *************************************************/
int16_t Can0ClearRxFlagByCanId(uint32_t canId)
{
    uint32_t i;
    canId_map_configure_t *pCanConfigure;

    pCanConfigure = (canId_map_configure_t *)CAN_ID_CONFIGURE_POINTER(0);
    for (i = 0; i < CAN_MSG_BUFFER_SIZE(0); i++)
    {
        if (pCanConfigure[i].canId == canId)
        {
            CAN_MSG_BUFFER_GET_ELEMENT(0, i).rxFlag = 0;
            break;
        }
    }

    return 0;
}
