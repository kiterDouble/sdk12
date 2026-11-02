/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: remoteControlCanSignal.c
 Author: Simon
 Created Time: 2025-09-24
 Description: 
 Others: 
 *************************************************/
 /****************************** include ***************************************/
#include "stdio.h"
#include "canHal.h"
#include "secocSdk.h"
#include "vehicleSignalApp.h"
#include "remoteControlCanSignal.h"
#include "remoteControl.h"
#include "vehicleSignalApp.h"
#include "logHal.h"
/****************************** Macro Definitions ******************************/
#define REMOTE_CONTROL_SEND_BUF_SIZE    (8)
/****************************** Type Definitions ******************************/
typedef struct 
{
    RemoteControlReqSignalId_t reqId;
    CanParseSignal_t signalInfo;
  }SignalConfig_t;
typedef struct
{
    SignalConfig_t TEL_UnlockDoorReq;
    SignalConfig_t TEL_UnlockDoorReqVD;
    SignalConfig_t TEL_LockDoorReq;
    SignalConfig_t TEL_LockDoorReqVD;
    SignalConfig_t TEL_LowBeamLightOnReq;
    SignalConfig_t TEL_LowBeamLightOnReqVD;
    SignalConfig_t TEL_LowBeamLightOffReq;
    SignalConfig_t TEL_LowBeamLightOffReqVD;
    SignalConfig_t TEL_HornReq;
    SignalConfig_t TEL_HornReqVD;
    SignalConfig_t TEL_SeatHeatReq;
    SignalConfig_t TEL_SeatHeatReqVD;
    SignalConfig_t TEL_WindowsCloseReq;
    SignalConfig_t TEL_WindowsCloseReqVD;
    SignalConfig_t TEL_WindowsOpenReq;
    SignalConfig_t TEL_WindowsOpenReqVD;
    SignalConfig_t TEL_ParkingLampOnReq;
    SignalConfig_t TEL_ParkingLampOnReqVD;
    SignalConfig_t TEL_ParkingLampOffReq;
    SignalConfig_t TEL_ParkingLampOffReqVD;
    SignalConfig_t TEL_HazardLightOnReq;
    SignalConfig_t TEL_HazardLightOnReqVD;
    SignalConfig_t TEL_SunRoofOpenReq;
    SignalConfig_t TEL_SunRoofOpenReqVD;
    SignalConfig_t TEL_SunRoofCloseReq;
    SignalConfig_t TEL_SunRoofCloseReqVD;
    SignalConfig_t TEL_EngineStartReq;
    SignalConfig_t TEL_EngineStartReqVD;
    SignalConfig_t TEL_EngineStopReq;
    SignalConfig_t TEL_EngineStopReqVD;
    SignalConfig_t TEL_VoiceInput;
    SignalConfig_t TEL_VoiceInputVD;
    SignalConfig_t TEL_TrunkUnlockReq;
    SignalConfig_t TEL_TrunkUnlockReqVD;
    SignalConfig_t TEL_VehSearchReq;
    SignalConfig_t TEL_VehSearchReqVD;
    SignalConfig_t TEL_PowerOnReq;
    SignalConfig_t TEL_PowerOnReqVD;
    SignalConfig_t TEL_PowerOffReq;
    SignalConfig_t TEL_PowerOffReqVD;
    SignalConfig_t TEL_EngineForbidReq;
    SignalConfig_t TEL_EnginePermitReq;
    SignalConfig_t TEL_EngineForbidReqVD;
    SignalConfig_t TEL_EnginePermitReqVD;
    SignalConfig_t TEL_HornOffReq;
    SignalConfig_t TEL_HornOffReqVD;
    SignalConfig_t TEL_HazardLightOffReq;
    SignalConfig_t TEL_HazardLightOffReqVD;
    SignalConfig_t TEL_PLGMOpenReq;
    SignalConfig_t TEL_PLGMOpenReqVD;
    SignalConfig_t TEL_PLGMCloseReq;
    SignalConfig_t TEL_PLGMCloseReqVD;
    SignalConfig_t TEL_EngForceStartVD;
    SignalConfig_t TEL_EngForceStart;
    SignalConfig_t TEL_PowerOnTimerReq;
    SignalConfig_t TEL_AllHazardLightOffReq;
} RemoteControlReqSignal047_t;

typedef struct
{
    SignalConfig_t TEL_FLHeatingLevelReq;
    SignalConfig_t TEL_FLVentilatingLevelReq;
    SignalConfig_t TEL_FLHVSMAutoModeReq;
    SignalConfig_t TEL_FRHeatingLevelReq;
    SignalConfig_t TEL_FRVentilatingLevelReq;
    SignalConfig_t TEL_FRHVSMAutoModeReq;
    SignalConfig_t TEL_RLHeatingLevelReq;
    SignalConfig_t TEL_RLVentilatingLevelReq;
    SignalConfig_t TEL_RLHVSMAutoModeReq;
    SignalConfig_t TEL_RRHeatingLevelReq;
    SignalConfig_t TEL_RRVentilatingLevelReq;
    SignalConfig_t TEL_RRHVSMAutoModeReq;
    SignalConfig_t TEL_HVSMCtrlModeSt;
} RemoteControlReqSignal05B_t;

typedef struct
{
    SignalConfig_t TEL_HVACF_TempSelectManualReq;
    SignalConfig_t TEL_HVACF_DrTempSelectReq;
    SignalConfig_t TEL_HVACF_WindExitSpdReq;
    SignalConfig_t TEL_HVACF_WindExitModeReq;
    SignalConfig_t TEL_HVACF_RearDefrostReq;
    SignalConfig_t TEL_HVACF_ACReq;
    SignalConfig_t TEL_HVACF_AutoSt;
    SignalConfig_t TEL_HVACF_AirCirculationReq;
    SignalConfig_t TEL_HVACF_FrontDefReq;
    SignalConfig_t TEL_HVACF_ControlSt;
    SignalConfig_t TEL_HVACF_ACMaxReq;
    SignalConfig_t TEL_HVACF_IonReq;
    SignalConfig_t TEL_HVACF_DualReq;
    SignalConfig_t TEL_HVACF_TripleZoneReq;
    SignalConfig_t TEL_HVACF_PaTempSelectReq;
    SignalConfig_t TEL_HVACF_HVACCtrlModeSt;
    SignalConfig_t TEL_HVACF_RearTempSelectReq;
    SignalConfig_t TEL_HVACF_RearReq;
} RemoteControlReqSignal05C_t;

typedef struct
{
    SignalConfig_t TEL_WindowsVentilateModeReq;
    SignalConfig_t TEL_WindowsVentilateModeReqVD;
    SignalConfig_t TEL_WindowsVentilateModeCloseReq;
    SignalConfig_t TEL_WindowsVentilateModeCloseReqVD;
} RemoteControlReqSignal077_t;
typedef struct
{
    SignalConfig_t TEL_IMMOCode1;
}RemoteControlReqSignal04F_t;

typedef struct
{
    SignalConfig_t TEL_IMMOCode2;
}RemoteControlReqSignal04E_t;
/****************************** Function Declarations *************************/

/****************************** Global Variables ******************************/
static const uint8_t g_CanSignalFormat = VEHICLE_CAN_UNPACK_FORMAT_INTEL;
static const uint8_t g_CanSignalFormatB = VEHICLE_CAN_UNPACK_FORMAT_MOTO_MSB;
static uint32_t g_remoteControlReqCanId = 0U;
static RemoteControlReqSignal04F_t g_04FCanConfigure = 
{
    .TEL_IMMOCode1 = 
    {
        .reqId = TEL_IMMOCode1,
        .signalInfo = {
            .msgBufferPointer = NULL,
            .canBufferIdIndex = 0,
            .dataType = (uint16_t)0,
            .startBit = (uint16_t)56,
            .bitLength = (uint16_t)64,
            .resulotion = (float)1.0,
            .offset = (float)0.0,
            .useInvalidFlag = 1,
        },
    }
};

static RemoteControlReqSignal04E_t g_04ECanConfigure = 
{
    .TEL_IMMOCode2 = 
    {
        .reqId = TEL_IMMOCode2,
        .signalInfo = {
            .msgBufferPointer = NULL,
            .canBufferIdIndex = 0,
            .dataType = (uint16_t)0,
            .startBit = (uint16_t)56,
            .bitLength = (uint16_t)64,
            .resulotion = (float)1.0,
            .offset = (float)0.0,
            .useInvalidFlag = 1,
        },
    }
};

static RemoteControlReqSignal047_t g_047Configure = 
{
    .TEL_UnlockDoorReq = 
    {
      .reqId = TEL_UnlockDoorReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)0,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_UnlockDoorReqVD =
    {
      .reqId = TEL_UnlockDoorReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)1,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_LockDoorReq =
    {
      .reqId = TEL_LockDoorReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)2,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_LockDoorReqVD =
    {
      .reqId = TEL_LockDoorReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)3,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_LowBeamLightOnReq =
    {
      .reqId = TEL_LowBeamLightOnReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)4,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_LowBeamLightOnReqVD =
    {
      .reqId = TEL_LowBeamLightOnReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)5,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_LowBeamLightOffReq =
    {
      .reqId = TEL_LowBeamLightOffReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)6,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_LowBeamLightOffReqVD =
    {
      .reqId = TEL_LowBeamLightOffReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)7,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_HornReq =
    {
      .reqId = TEL_HornReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)8,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_HornReqVD =
    {
      .reqId = TEL_HornReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)9,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_SeatHeatReq =
    {
      .reqId = TEL_SeatHeatReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)10,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_SeatHeatReqVD =
    {
      .reqId = TEL_SeatHeatReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)11,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_WindowsCloseReq =
    {
      .reqId = TEL_WindowsCloseReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)12,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_WindowsCloseReqVD =
    {
      .reqId = TEL_WindowsCloseReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)13,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_WindowsOpenReq =
    {
      .reqId = TEL_WindowsOpenReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)14,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_WindowsOpenReqVD =
    {
      .reqId = TEL_WindowsOpenReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)15,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_ParkingLampOnReq =
    {
      .reqId = TEL_ParkingLampOnReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)16,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_ParkingLampOnReqVD =
    {
      .reqId = TEL_ParkingLampOnReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)17,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_ParkingLampOffReq =
    {
      .reqId = TEL_ParkingLampOffReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)18,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_ParkingLampOffReqVD =
    {
      .reqId = TEL_ParkingLampOffReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)19,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_HazardLightOnReq =
    {
      .reqId = TEL_HazardLightOnReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)20,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_HazardLightOnReqVD =
    {
      .reqId = TEL_HazardLightOnReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)21,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_SunRoofOpenReq =
    {
      .reqId = TEL_SunRoofOpenReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)22,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_SunRoofOpenReqVD =
    {
      .reqId = TEL_SunRoofOpenReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)23,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_SunRoofCloseReq =
    {
      .reqId = TEL_SunRoofCloseReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)24,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_SunRoofCloseReqVD =
    {
      .reqId = TEL_SunRoofCloseReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)25,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_EngineStartReq =
    {
      .reqId = TEL_EngineStartReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)26,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_EngineStartReqVD =
    {
      .reqId = TEL_EngineStartReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)27,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_EngineStopReq =
    {
      .reqId = TEL_EngineStopReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)28,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_EngineStopReqVD =
    {
      .reqId = TEL_EngineStopReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)29,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_VoiceInput =
    {
      .reqId = TEL_VoiceInput,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)30,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_VoiceInputVD =
    {
      .reqId = TEL_VoiceInputVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)31,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_TrunkUnlockReq =
    {
      .reqId = TEL_TrunkUnlockReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)32,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_TrunkUnlockReqVD =
    {
      .reqId = TEL_TrunkUnlockReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)33,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_VehSearchReq =
    {
      .reqId = TEL_VehSearchReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)34,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_VehSearchReqVD =
    {
      .reqId = TEL_VehSearchReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)35,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_PowerOnReq =
    {
      .reqId = TEL_PowerOnReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)36,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_PowerOnReqVD =
    {
      .reqId = TEL_PowerOnReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)37,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_PowerOffReq =
    {
      .reqId = TEL_PowerOffReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)38,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_PowerOffReqVD =
    {
      .reqId = TEL_PowerOffReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)39,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_EngineForbidReq =
    {
      .reqId = TEL_EngineForbidReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)40,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_EnginePermitReq =
    {
      .reqId = TEL_EnginePermitReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)41,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_EngineForbidReqVD =
    {
      .reqId = TEL_EngineForbidReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)42,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_EnginePermitReqVD =
    {
      .reqId = TEL_EnginePermitReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)43,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_HornOffReq =
    {
      .reqId = TEL_HornOffReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)44,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_HornOffReqVD =
    {
      .reqId = TEL_HornOffReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)45,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_HazardLightOffReq =
    {
      .reqId = TEL_HazardLightOffReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)46,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_HazardLightOffReqVD =
    {
      .reqId = TEL_HazardLightOffReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)47,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_PLGMOpenReq =
    {
      .reqId = TEL_PLGMOpenReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)50,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_PLGMOpenReqVD =
    {
      .reqId = TEL_PLGMOpenReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)51,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_PLGMCloseReq =
    {
      .reqId = TEL_PLGMCloseReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)52,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_PLGMCloseReqVD =
    {
      .reqId = TEL_PLGMCloseReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)53,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_EngForceStartVD =
    {
      .reqId = TEL_EngForceStartVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)54,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_EngForceStart =
    {
      .reqId = TEL_EngForceStart,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)55,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_PowerOnTimerReq =
    {
      .reqId = TEL_PowerOnTimerReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)56,
        .bitLength = (uint16_t)6,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
    .TEL_AllHazardLightOffReq =
    {
      .reqId = TEL_AllHazardLightOffReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)62,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1.0,
        .offset = (float)0.0,
        .useInvalidFlag = 1,
        .InvalidData = 0,
      },
    },
};

static RemoteControlReqSignal05B_t g_05BConfigure = 
{
    .TEL_FLHeatingLevelReq = 
    {
      .reqId = TEL_FLHeatingLevelReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)0,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_FLVentilatingLevelReq = 
    {
      .reqId = TEL_FLVentilatingLevelReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)3,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_FLHVSMAutoModeReq = 
    {
      .reqId = TEL_FLHVSMAutoModeReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)6,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_FRHeatingLevelReq = 
    {
      .reqId = TEL_FRHeatingLevelReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)8,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_FRVentilatingLevelReq = 
    {
      .reqId = TEL_FRVentilatingLevelReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)11,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_FRHVSMAutoModeReq = 
    {
      .reqId = TEL_FRHVSMAutoModeReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)14,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_RLHeatingLevelReq = 
    {
      .reqId = TEL_RLHeatingLevelReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)16,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_RLVentilatingLevelReq = 
    {
      .reqId = TEL_RLVentilatingLevelReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)19,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_RLHVSMAutoModeReq = 
    {
      .reqId = TEL_RLHVSMAutoModeReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)22,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_RRHeatingLevelReq = 
    {
      .reqId = TEL_RRHeatingLevelReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)24,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_RRVentilatingLevelReq = 
    {
      .reqId = TEL_RRVentilatingLevelReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)27,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_RRHVSMAutoModeReq = 
    {
      .reqId = TEL_RRHVSMAutoModeReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)30,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVSMCtrlModeSt = 
    {
      .reqId = TEL_HVSMCtrlModeSt,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)32,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
};


static RemoteControlReqSignal05C_t g_05CConfigure = 
{
    .TEL_HVACF_TempSelectManualReq = 
    {
      .reqId = TEL_HVACF_TempSelectManualReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)0,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_DrTempSelectReq = 
    {
      .reqId = TEL_HVACF_DrTempSelectReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)3,
        .bitLength = (uint16_t)5,
        .resulotion = (float)0.5,
        .offset = (float)18,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_WindExitSpdReq = 
    {
      .reqId = TEL_HVACF_WindExitSpdReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)8,
        .bitLength = (uint16_t)4,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_WindExitModeReq = 
    {
      .reqId = TEL_HVACF_WindExitModeReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)12,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_RearDefrostReq = 
    {
      .reqId = TEL_HVACF_RearDefrostReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)16,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_ACReq = 
    {
      .reqId = TEL_HVACF_ACReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)18,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_AutoSt = 
    {
      .reqId = TEL_HVACF_AutoSt,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)20,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_AirCirculationReq = 
    {
      .reqId = TEL_HVACF_AirCirculationReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)22,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_FrontDefReq = 
    {
      .reqId = TEL_HVACF_FrontDefReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)29,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 1,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_ControlSt = 
    {
      .reqId = TEL_HVACF_ControlSt,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)31,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 1,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_ACMaxReq = 
    {
      .reqId = TEL_HVACF_ACMaxReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)32,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_IonReq = 
    {
      .reqId = TEL_HVACF_IonReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)34,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_DualReq = 
    {
      .reqId = TEL_HVACF_DualReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)36,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_TripleZoneReq = 
    {
      .reqId = TEL_HVACF_TripleZoneReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)38,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_PaTempSelectReq = 
    {
      .reqId = TEL_HVACF_PaTempSelectReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)40,
        .bitLength = (uint16_t)5,
        .resulotion = (float)0.5,
        .offset = (float)18,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_HVACCtrlModeSt = 
    {
      .reqId = TEL_HVACF_HVACCtrlModeSt,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)45,
        .bitLength = (uint16_t)3,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_RearTempSelectReq = 
    {
      .reqId = TEL_HVACF_RearTempSelectReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)48,
        .bitLength = (uint16_t)5,
        .resulotion = (float)0.5,
        .offset = (float)18,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_HVACF_RearReq = 
    {
      .reqId = TEL_HVACF_RearReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)53,
        .bitLength = (uint16_t)2,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
};

static RemoteControlReqSignal077_t g_077Configure = 
{
    .TEL_WindowsVentilateModeReq = 
    {
      .reqId = TEL_WindowsVentilateModeReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)7,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_WindowsVentilateModeReqVD = 
    {
      .reqId = TEL_WindowsVentilateModeReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)8,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_WindowsVentilateModeCloseReq = 
    {
      .reqId = TEL_WindowsVentilateModeCloseReq,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)9,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
    .TEL_WindowsVentilateModeCloseReqVD = 
    {
      .reqId = TEL_WindowsVentilateModeCloseReqVD,
      .signalInfo = {
        .msgBufferPointer = NULL,
        .canBufferIdIndex = 0,
        .dataType = (uint16_t)0,
        .startBit = (uint16_t)10,
        .bitLength = (uint16_t)1,
        .resulotion = (float)1,
        .offset = (float)0,
        .useInvalidFlag = 0,
        .InvalidData = 0u,
      },
    },
};

/****************************** Public Function Implementations ***************/
/***************************************************************************
 * @Function    RemoteControlGetSignalValue
 * @Description Get the remote control status signal values from CAN bus
 * @Input       stRemoteCtrlSignalVal - Pointer to store the remote control status signal information
 * @Output      stRemoteCtrlSignalVal - Updated with remote control status signal values
 * @Return      None
 * @Others      This function reads various remote control related signals from CAN bus
 *              including BCM, EMS, EBS, BCS and PEPS signals
 ******************************************************************************/ 
#if 1  /*TODO: 适配新的远控信号*/
void RemoteControlGetSignalValue(RemoteControlStatusSignalInfo_t *stRemoteCtrlSignalVal)
{
    double dataVaule = 0U;
    const can0_signal_configure_t *pCan0SignalConfigure = NULL;
    RemoteControlStatusSignalInfo_t *stRemoteCtrlSignalValTemp = NULL;

    pCan0SignalConfigure = GetCan0SignalConfigure();
    stRemoteCtrlSignalValTemp = stRemoteCtrlSignalVal;
    
    CanParseSdkReadSignal(g_CanSignalFormat, &pCan0SignalConfigure->BMS_ST2_SOC, &dataVaule);
    stRemoteCtrlSignalValTemp->BMS_ST2_SOC = (uint8_t)dataVaule;
//    TBOX_PRINT("BMS_ST2_SOC: %d\r\n", stRemoteCtrlSignalValTemp->BMS_ST2_SOC);
    CanParseSdkReadSignal(g_CanSignalFormat, &pCan0SignalConfigure->Tx_DB_MaxErrLevel, &dataVaule);
    stRemoteCtrlSignalValTemp->Tx_DB_MaxErrLevel = (uint8_t)dataVaule;
    
    CanParseSdkReadSignal(g_CanSignalFormat, &pCan0SignalConfigure->Rx_ActiveState, &dataVaule);
    stRemoteCtrlSignalValTemp->Rx_ActiveState = (uint8_t)dataVaule;
//    TBOX_PRINT("Rx_ActiveState: %x\r\n", stRemoteCtrlSignalValTemp->Rx_ActiveState);
    CanParseSdkReadSignal(g_CanSignalFormat, &pCan0SignalConfigure->Rx_LockState, &dataVaule);
    stRemoteCtrlSignalValTemp->Rx_LockState = (uint8_t)dataVaule;
    
    CanParseSdkReadSignal(g_CanSignalFormat, &pCan0SignalConfigure->Rx_KeyOkState, &dataVaule);
    stRemoteCtrlSignalValTemp->Rx_KeyOkState = (uint8_t)dataVaule;
    
    CanParseSdkReadSignal(g_CanSignalFormat, &pCan0SignalConfigure->Rx_VcuResponse, &dataVaule);
    stRemoteCtrlSignalValTemp->Rx_VcuResponse = (uint8_t)dataVaule;
    
    CanParseSdkReadSignal(g_CanSignalFormatB, &pCan0SignalConfigure->B2T_RQ_SetMode_A, &dataVaule);
    stRemoteCtrlSignalValTemp->B2T_RQ_SetMode_A = (uint8_t)dataVaule;
//    TBOX_PRINT("B2T_RQ_SetMode_A: %x\r\n", stRemoteCtrlSignalValTemp->B2T_RQ_SetMode_A);
  
}
#endif


/***************************************************************************//**
 * @Function    RemoteCtrlSignalValToCanFrame
 * @Description Pack remote control signal value into CAN frame based on signal ID
 * @Input       pMsgData - Pointer to CAN message data buffer
 *              signalId - Remote control request signal ID
 *              signalValue - Signal value to be packed
 * @Output      pMsgData - Updated CAN message data buffer with packed signal value
 * @Return      None
 * @Others      This function handles various remote control signals including door lock/unlock,
 *              lighting control, seat heating/ventilation, HVAC control, and other vehicle functions.
 *              It sets the appropriate CAN ID and uses PackSignalToFrame to encode the signal value.
 ******************************************************************************/
void RemoteCtrlSignalValToCanFrame(uint8_t* pMsgData, const CanParseSignal_t* signal, double signalValue)
{
    PackSignalToFrame(pMsgData, g_CanSignalFormat, signal, signalValue);         
}

/*************************************************
   Function:        RemoteControlGetReqCanId
   Description:     Get the current remote control request CAN ID
   Input:           None
   Output:          None
   Return:          Current remote control request CAN ID
   Others:          
 *************************************************/
uint32_t RemoteControlGetReqCanId(void)
{
    return g_remoteControlReqCanId;
};






