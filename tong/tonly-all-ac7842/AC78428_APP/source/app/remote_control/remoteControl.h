/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: remoteControl.h
 Author: Simon
 Created Time: 2025-09-24
 Description: 
 Others: 
 *************************************************/
#ifndef _REMOTE_CONTROL_H_
#define _REMOTE_CONTROL_H_
 /****************************** include ***************************************/
#include "stdint.h"
#include "mpuHal.h"
#include "canHal.h"
/****************************** Macro Definitions ******************************/

/****************************** Type Definitions ******************************/
typedef enum
{
    RemoteControlStateIdle,
    RemoteControlStatePreCheck,
    RemoteControlStateCertification,
    RemoteControlStateProcessSignal,
    RemoteControlStateSendResult,  
}RemoteControlState_t;

/****************************** Command Macro Definitions ********************/
// Default command
#define CMD_DEFAULT_E               (0U)
// PEPS commands
#define CMD_ENGINE_SET_E            (1U)
#define CMD_ENGINE_ENABLE_E         (2U)
#define CMD_POWER_SET_E             (3U)
#define CMD_ENGINE_MODE_2_ON_E      (4U)
// HVAC commands
#define CMD_AC_SPEED_SET_E          (1U)  
#define CMD_AC_FRONT_DEF_SET_E      (2U)  
#define CMD_AC_REAR_DEF_SET_E       (3U)
#define CMD_AC_TEMP_SET_E           (4U)
#define CMD_AC_AIR_CAL_REQ_E        (5U)  
#define CMD_AC_WIND_EXIT_MODE_E     (6U)
#define CMD_AC_REQ_E                (7U)
#define CMD_AC_AUTOST_E             (8U)
#define CMD_AC_ION_REQ_E            (9U)
#define CMD_AC_TIME_SET_E           (10U)
#define CMD_AC_FRAG_ST_E            (11U)
#define CMD_AC_ONE_KEY_CLEAR_E      (12U)   
#define CMD_AC_TEMP_AUTO_SET_E      (13U)
// HVSM commands
#define CMD_M_SEAT_HEAT_SET_E       (1U)
#define CMD_M_SEAT_VENTILATE_E      (2U)
#define CMD_S_SEAT_HEAT_SET_E       (3U)
#define CMD_S_SEAT_VENTILATE_E      (4U)
// PLGM commands
#define CMD_DOORS_SET_E             (1U)
// BCM commands
#define CMD_MID_CTRL_LOCK_E         (1U)
#define CMD_LOW_BEAM_SET_E          (2U)
#define CMD_HORN_SET_E              (3U)
#define CMD_WIN_SET_E               (4U)
#define CMD_SUN_ROOF_SET_E          (5U)
#define CMD_PARK_LAMP_SET_E         (6U)
#define CMD_HAZARD_LAMP_SET_E       (7U)
#define CMD_LEFT_TURN_LAMP_E        (8U)
#define CMD_TRUNK_AJAR_E            (9U)
#define CMD_WIN_VENTILATE_SET_E     (10U)
#define CMD_MID_CTRL_FBD_LOCK_E     (11U)
#define CMD_HAZARD_LAMP_OFF_E       (12U)
//max command number
#define CMD_REMOTE_CTRL_MAX_NUM_E   (13U)

// Command ID type definition for backward compatibility
typedef uint8_t RemoteControlCmdId_t;

void TaskAppVehicleRemoteControl( void *pvParameters );
RemoteControlState_t RemoteControlGetTotalState(void);
uint8_t RemoteControlGetKeepWakeFlag(void);
uint8_t RemoteControlGetOtaFlag(void);
void ReceiveSeedMessage(CanHalMsg_t canMsg);
void ReceivePeriodMessage(CanHalMsg_t canMsg);
#endif
