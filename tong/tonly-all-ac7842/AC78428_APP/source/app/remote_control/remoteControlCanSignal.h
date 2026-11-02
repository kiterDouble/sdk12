#ifndef _REMOTE_CONTROL_CAN_SIGNAL_H_
#define _REMOTE_CONTROL_CAN_SIGNAL_H_
#include "remoteControl.h"
#include "canParseSdk.h"
typedef struct
{
    uint8_t BMS_ST2_SOC;            /*电池包SOC*/ 
    uint8_t Tx_DB_MaxErrLevel;      /*整车故障等级*/
    uint8_t Rx_ActiveState;         /*锁车功能激活状态*/
    uint8_t Rx_LockState;           /*车辆实际锁车状态*/
    uint8_t Rx_KeyOkState;          /*Key 值正确状态*/ 
    uint8_t Rx_VcuResponse;         /*VCU应答帧*/ 
    uint8_t B2T_RQ_SetMode_A;       /*BMS请求BTMS模式*/

}RemoteControlStatusSignalInfo_t;

typedef enum
{
    REMOTE_CONTROL_TEL_IMMOCode1_E          = 0x04F,
    REMOTE_CONTROL_TEL_IMMOCode2_E          = 0x04E,
    REMOTE_CONTROL_TEL_RemoteControlReq_1_E = 0x047,
    REMOTE_CONTROL_TEL_HVSM_1_E             = 0x05B,
    REMOTE_CONTROL_TEL_16_B_T_E             = 0x077,
    REMOTE_CONTROL_TEL_HVACF_1_E            = 0x05C,
}RemoteControlReqCanId_t;

typedef enum
{
    TEL_IMMOCode1,
    TEL_IMMOCode2,
    TEL_UnlockDoorReq,
    TEL_UnlockDoorReqVD,
    TEL_LockDoorReq,
    TEL_LockDoorReqVD,
    TEL_LowBeamLightOnReq,
    TEL_LowBeamLightOnReqVD,
    TEL_LowBeamLightOffReq,
    TEL_LowBeamLightOffReqVD,
    TEL_HornReq,
    TEL_HornReqVD,
    TEL_SeatHeatReq,
    TEL_SeatHeatReqVD,
    TEL_WindowsCloseReq,
    TEL_WindowsCloseReqVD,
    TEL_WindowsOpenReq,
    TEL_WindowsOpenReqVD,
    TEL_ParkingLampOnReq,
    TEL_ParkingLampOnReqVD,
    TEL_ParkingLampOffReq,
    TEL_ParkingLampOffReqVD,
    TEL_HazardLightOnReq,
    TEL_HazardLightOnReqVD,
    TEL_SunRoofOpenReq,
    TEL_SunRoofOpenReqVD,
    TEL_SunRoofCloseReq,
    TEL_SunRoofCloseReqVD,
    TEL_EngineStartReq,
    TEL_EngineStartReqVD,
    TEL_EngineStopReq,
    TEL_EngineStopReqVD,
    TEL_VoiceInput,
    TEL_VoiceInputVD,
    TEL_TrunkUnlockReq,
    TEL_TrunkUnlockReqVD,
    TEL_VehSearchReq,
    TEL_VehSearchReqVD,
    TEL_PowerOnReq,
    TEL_PowerOnReqVD,
    TEL_PowerOffReq,
    TEL_PowerOffReqVD,
    TEL_EngineForbidReq,
    TEL_EnginePermitReq,
    TEL_EngineForbidReqVD,
    TEL_EnginePermitReqVD,
    TEL_HornOffReq,
    TEL_HornOffReqVD,
    TEL_HazardLightOffReq,
    TEL_HazardLightOffReqVD,
    TEL_PLGMOpenReq,
    TEL_PLGMOpenReqVD,
    TEL_PLGMCloseReq,
    TEL_PLGMCloseReqVD,
    TEL_EngForceStartVD,
    TEL_EngForceStart,
    TEL_PowerOnTimerReq,
    TEL_AllHazardLightOffReq,
    TEL_FLHeatingLevelReq,
    TEL_FLVentilatingLevelReq,
    TEL_FLHVSMAutoModeReq,
    TEL_FRHeatingLevelReq,
    TEL_FRVentilatingLevelReq,
    TEL_FRHVSMAutoModeReq,
    TEL_RLHeatingLevelReq,
    TEL_RLVentilatingLevelReq,
    TEL_RLHVSMAutoModeReq,
    TEL_RRHeatingLevelReq,
    TEL_RRVentilatingLevelReq,
    TEL_RRHVSMAutoModeReq,
    TEL_HVSMCtrlModeSt,
    TEL_HVACF_TempSelectManualReq,
    TEL_HVACF_DrTempSelectReq,
    TEL_HVACF_WindExitSpdReq,
    TEL_HVACF_WindExitModeReq,
    TEL_HVACF_RearDefrostReq,
    TEL_HVACF_ACReq,
    TEL_HVACF_AutoSt,
    TEL_HVACF_AirCirculationReq,
    TEL_HVACF_FrontDefReq,
    TEL_HVACF_ControlSt,
    TEL_HVACF_ACMaxReq,
    TEL_HVACF_IonReq,
    TEL_HVACF_DualReq,
    TEL_HVACF_TripleZoneReq,
    TEL_HVACF_PaTempSelectReq,
    TEL_HVACF_HVACCtrlModeSt,
    TEL_HVACF_RearTempSelectReq,
    TEL_HVACF_RearReq,
    TEL_WindowsVentilateModeReq,
    TEL_WindowsVentilateModeReqVD,
    TEL_WindowsVentilateModeCloseReq,
    TEL_WindowsVentilateModeCloseReqVD,

    TEL_RemoteControlReqSignalIdMax,
} RemoteControlReqSignalId_t;

void RemoteControlGetSignalValue(RemoteControlStatusSignalInfo_t *stRemoteCtrlSignalVal);
void RemoteCtrlSignalValToCanFrame(uint8_t* pMsgData, const CanParseSignal_t* signal, double signalValue);

/*************************************************
   Function:        RemoteControlGetReqCanId
   Description:     Get the current remote control request CAN ID
   Input:           None
   Output:          None
   Return:          Current remote control request CAN ID
   Others:          
 *************************************************/
uint32_t RemoteControlGetReqCanId(void);

#endif


