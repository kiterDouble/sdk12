/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: j1939App.h
 Author: 
 Created Time: 
 Description: J1939 Application Layer Header
              Example implementation for common vehicle PGNs
 Others: 
 *************************************************/

#ifndef _J1939_APP_H_
#define _J1939_APP_H_

/****************************** include ***************************************/
#include <stdint.h>
#include "j1939Sdk.h"

/****************************** Macro Definitions ******************************/
/* Common J1939 PGN Definitions */
#define J1939_PGN_ELECTRONIC_ENGINE_CONTROLLER_1   0x00F004  /* Engine Speed, Torque */
#define J1939_PGN_ELECTRONIC_ENGINE_CONTROLLER_2   0x00F003  /* Engine Load */
#define J1939_PGN_ELECTRONIC_TRANSMISSION_CONTROLLER 0x00F005 /* Transmission */
#define J1939_PGN_VEHICLE_SPEED                      0x00FE6A  /* Vehicle Speed */
#define J1939_PGN_FUEL_CONSUMPTION                   0x00FEF1  /* Fuel Economy */
#define J1939_PGN_ODOMETER                           0x00FEE8  /* High Resolution Distance */
#define J1939_PGN_TIME_DATE                          0x00FEE6  /* Time and Date */
#define J1939_PGN_VIN                                0x00FEEC  /* Vehicle Identification */
#define J1939_PGN_SOFTWARE_IDENTIFICATION            0x00FEDA  /* Component ID */

/* Custom Body Control PGNs */
#define J1939_PGN_BODY_CONTROL_STATUS                0x00FF02  /* Body Control Module Status */
#define J1939_PGN_DOOR_CONTROL                       0x00FF03  /* Door Status */
#define J1939_PGN_LIGHTING_CONTROL                   0x00FF04  /* Lighting Status */

/* SPN (Suspect Parameter Number) Definitions */
#define J1939_SPN_ENGINE_SPEED                       190
#define J1939_SPN_ENGINE_TORQUE                      513
#define J1939_SPN_ENGINE_PERCENT_LOAD                92
#define J1939_SPN_VEHICLE_SPEED                      84
#define J1939_SPN_ODOMETER                           917
#define J1939_SPN_FUEL_RATE                          183
#define J1939_SPN_INSTANT_FUEL_ECONOMY               3464

/****************************** Type Definitions ******************************/

/* Engine Data Structure */
typedef struct
{
    uint16_t engineSpeed;           /* RPM, 0.125 rpm/bit */
    uint16_t engineTorque;          /* Nm, 1 Nm/bit */
    uint8_t  enginePercentLoad;     /* %, 1%/bit */
    uint8_t  valid;                 /* Data valid flag */
} J1939EngineData_t;

/* Vehicle Speed Data Structure */
typedef struct
{
    uint16_t vehicleSpeed;          /* km/h, 0.00390625 km/h/bit */
    uint8_t  valid;                 /* Data valid flag */
} J1939VehicleSpeed_t;

/* Body Control Status Structure */
typedef struct
{
    uint8_t  doorStatus;            /* Bit field for 4 doors */
    uint8_t  lightStatus;           /* Bit field for lights */
    uint8_t  windowStatus;          /* Bit field for windows */
    uint16_t batteryVoltage;        /* mV */
    uint8_t  valid;
} J1939BodyControlStatus_t;

/* Application Statistics */
typedef struct
{
    uint32_t engineSpeedRxCount;
    uint32_t vehicleSpeedRxCount;
    uint32_t bodyStatusTxCount;
    uint32_t errorCount;
} J1939AppStatistics_t;

/****************************** Function Declarations *************************/

/* Initialization */
int16_t J1939App_Init(void);
void J1939App_Deinit(void);

/* Application Cycle Processing */
void J1939App_CycleProcess(void);

/* Data Access Functions */
int16_t J1939App_GetEngineData(J1939EngineData_t *engineData);
int16_t J1939App_GetVehicleSpeed(J1939VehicleSpeed_t *speedData);
int16_t J1939App_SetBodyControlStatus(const J1939BodyControlStatus_t *status);

/* PGN Callback Functions (called by J1939 SDK) */
void J1939App_EngineSpeedCallback(const J1939Message_t *msg);
void J1939App_VehicleSpeedCallback(const J1939Message_t *msg);

/* TX Functions */
int16_t J1939App_TransmitBodyStatus(void);
int16_t J1939App_TransmitCustomPgn(uint32_t pgn, uint8_t *data, uint8_t length);

/* Statistics */
void J1939App_GetStatistics(J1939AppStatistics_t *stats);
void J1939App_ResetStatistics(void);

/* Debug/Print Functions */
void J1939App_PrintEngineData(void);
void J1939App_PrintVehicleSpeed(void);

/* Transport Protocol Functions */
int16_t J1939App_TransmitLargeData(uint8_t destAddr, uint8_t *data, uint16_t length);

#endif /* _J1939_APP_H_ */
