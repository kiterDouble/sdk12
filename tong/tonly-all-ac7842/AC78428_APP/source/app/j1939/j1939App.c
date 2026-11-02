/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: j1939App.c
 Author: 
 Created Time: 
 Description: J1939 Application Layer Implementation
              Example implementation for common vehicle PGNs:
              - PGN 0xF004: Electronic Engine Controller 1 (Engine Speed)
              - PGN 0xFE6A: Vehicle Speed
              - PGN 0xFF02: Body Control Status (Custom TX PGN)
 Others: 
 *************************************************/

/****************************** include ***************************************/
#include "j1939App.h"
#include "j1939Tp.h"
#include "logHal.h"
#include "timerHal.h"
#include <string.h>

/****************************** Macro Definitions ******************************/
/* Periodic TX Intervals (ms) */
#define BODY_STATUS_TX_PERIOD           100     /* 10Hz */

/* Data Valid Timeout (ms) */
#define DATA_VALID_TIMEOUT              1000    /* 1 second */

/****************************** Type Definitions ******************************/

/****************************** Global Variables ******************************/
/* Received Data Buffers */
static J1939EngineData_t g_engineData;
static J1939VehicleSpeed_t g_vehicleSpeed;
static J1939BodyControlStatus_t g_bodyControlStatus;

/* Statistics */
static J1939AppStatistics_t g_appStats;

/* TX Timestamps */
static uint32_t g_lastBodyStatusTxTime = 0;

/* Initialization Flag */
static uint8_t g_appInitialized = 0;

/****************************** Function Declarations *************************/
static void J1939AppParseEngineSpeed(const J1939Message_t *msg);
static void J1939AppParseVehicleSpeed(const J1939Message_t *msg);
static int16_t J1939AppBuildBodyStatusMsg(uint8_t *data, uint8_t *length);

/****************************** Private Function Implementations **************/

/*************************************************
 Function:        J1939AppParseEngineSpeed
 Description:     Parse Engine Speed from PGN 0xF004
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          SPN 190: Engine Speed, 0.125 rpm/bit
 *************************************************/
static void J1939AppParseEngineSpeed(const J1939Message_t *msg)
{
    uint16_t rawSpeed;
    
    if (msg == NULL || msg->length < 8)
    {
        return;
    }
    
    /* Engine Speed: Byte 3-4, 0.125 rpm/bit, offset 0 */
    rawSpeed = msg->data[3] | ((uint16_t)msg->data[4] << 8);
    
    /* Convert to RPM (0.125 rpm/bit) */
    if (rawSpeed != 0xFFFF)  /* Check for not available */
    {
        g_engineData.engineSpeed = (uint16_t)((rawSpeed * 125) / 1000);
        g_engineData.valid = 1;
    }
    else
    {
        g_engineData.valid = 0;
    }
    
    /* Engine Percent Load: Byte 2, 1%/bit, offset 0 */
    if (msg->data[2] != 0xFF)
    {
        g_engineData.enginePercentLoad = msg->data[2];
    }
    
    g_appStats.engineSpeedRxCount++;
}

/*************************************************
 Function:        J1939AppParseVehicleSpeed
 Description:     Parse Vehicle Speed from PGN 0xFE6A
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          SPN 84: Vehicle Speed, 0.00390625 km/h/bit
 *************************************************/
static void J1939AppParseVehicleSpeed(const J1939Message_t *msg)
{
    uint16_t rawSpeed;
    
    if (msg == NULL || msg->length < 2)
    {
        return;
    }
    
    /* Vehicle Speed: Byte 1-2, 0.00390625 km/h/bit */
    rawSpeed = msg->data[1] | ((uint16_t)msg->data[2] << 8);
    
    if (rawSpeed != 0xFFFF)
    {
        /* Convert to 0.01 km/h for integer representation */
        g_vehicleSpeed.vehicleSpeed = (uint16_t)((rawSpeed * 100) / 256);
        g_vehicleSpeed.valid = 1;
    }
    else
    {
        g_vehicleSpeed.valid = 0;
    }
    
    g_appStats.vehicleSpeedRxCount++;
}

/*************************************************
 Function:        J1939AppBuildBodyStatusMsg
 Description:     Build Body Control Status message data
 Input:           data - Output data buffer
                  length - Output data length
 Output:          data, length
 Return:          J1939_OK on success
 Others:          Custom PGN 0xFF02 format
 *************************************************/
static int16_t J1939AppBuildBodyStatusMsg(uint8_t *data, uint8_t *length)
{
    if (data == NULL || length == NULL)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    /* Byte 0: Door Status (Bit field) */
    data[0] = g_bodyControlStatus.doorStatus;
    
    /* Byte 1: Light Status (Bit field) */
    data[1] = g_bodyControlStatus.lightStatus;
    
    /* Byte 2: Window Status (Bit field) */
    data[2] = g_bodyControlStatus.windowStatus;
    
    /* Byte 3-4: Battery Voltage (mV, 10mV/bit) */
    data[3] = (uint8_t)(g_bodyControlStatus.batteryVoltage / 10);
    data[4] = (uint8_t)((g_bodyControlStatus.batteryVoltage / 10) >> 8);
    
    /* Byte 5-7: Reserved */
    data[5] = 0xFF;
    data[6] = 0xFF;
    data[7] = 0xFF;
    
    *length = 8;
    
    return J1939_OK;
}

/****************************** Public Function Implementations ***************/

/*************************************************
 Function:        J1939App_Init
 Description:     Initialize J1939 Application Layer
 Input:           None
 Output:          None
 Return:          J1939_OK on success
 Others:          Registers PGN callbacks with J1939 SDK
 *************************************************/
int16_t J1939App_Init(void)
{
    int16_t ret;
    
    if (g_appInitialized)
    {
        return J1939_OK;
    }
    
    /* Initialize data structures */
    memset(&g_engineData, 0, sizeof(J1939EngineData_t));
    memset(&g_vehicleSpeed, 0, sizeof(J1939VehicleSpeed_t));
    memset(&g_bodyControlStatus, 0, sizeof(J1939BodyControlStatus_t));
    memset(&g_appStats, 0, sizeof(J1939AppStatistics_t));
    
    /* Note: Transport Protocol is initialized by SDK layer */
    
    /* Register PGN Callbacks */
    ret = J1939Sdk_RegisterPgn(J1939_PGN_ELECTRONIC_ENGINE_CONTROLLER_1, 
                              J1939App_EngineSpeedCallback);
    if (ret != J1939_OK)
    {
        TBOX_PRINT("J1939App: Failed to register EEC1 PGN\r\n");
        return ret;
    }
    
    ret = J1939Sdk_RegisterPgn(J1939_PGN_VEHICLE_SPEED, 
                              J1939App_VehicleSpeedCallback);
    if (ret != J1939_OK)
    {
        TBOX_PRINT("J1939App: Failed to register Vehicle Speed PGN\r\n");
        return ret;
    }
    
    /* Configure TX PGN */
    ret = J1939Sdk_ConfigTxPgn(J1939_PGN_BODY_CONTROL_STATUS, 6, BODY_STATUS_TX_PERIOD);
    if (ret != J1939_OK)
    {
        TBOX_PRINT("J1939App: Failed to configure Body Status TX PGN\r\n");
        return ret;
    }
    
    g_appInitialized = 1;
    
    TBOX_PRINT("J1939App: Initialized successfully\r\n");
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939App_Deinit
 Description:     Deinitialize J1939 Application Layer
 Input:           None
 Output:          None
 Return:          None
 Others:          Unregisters PGN callbacks
 *************************************************/
void J1939App_Deinit(void)
{
    if (!g_appInitialized)
    {
        return;
    }
    
    /* Unregister PGNs */
    J1939Sdk_UnregisterPgn(J1939_PGN_ELECTRONIC_ENGINE_CONTROLLER_1);
    J1939Sdk_UnregisterPgn(J1939_PGN_VEHICLE_SPEED);
    
    /* Note: Transport Protocol is deinitialized by SDK layer */
    
    g_appInitialized = 0;
    
    TBOX_PRINT("J1939App: Deinitialized\r\n");
}

/*************************************************
 Function:        J1939App_CycleProcess
 Description:     Application cycle processing
 Input:           None
 Output:          None
 Return:          None
 Others:          Handles periodic TX transmissions
 *************************************************/
void J1939App_CycleProcess(void)
{
    uint32_t currentTime;
    
    if (!g_appInitialized)
    {
        return;
    }
    
    /* Get current time - use timer HAL */
    currentTime = TimerHalGetMs();
    
    /* Periodic Body Status TX */
    if ((currentTime - g_lastBodyStatusTxTime) >= BODY_STATUS_TX_PERIOD)
    {
        J1939App_TransmitBodyStatus();
        g_lastBodyStatusTxTime = currentTime;
    }
}

/*************************************************
 Function:        J1939App_GetEngineData
 Description:     Get latest Engine Data
 Input:           None
 Output:          engineData - Engine data structure
 Return:          J1939_OK if data is valid
 Others:          
 *************************************************/
int16_t J1939App_GetEngineData(J1939EngineData_t *engineData)
{
    if (engineData == NULL)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    memcpy(engineData, &g_engineData, sizeof(J1939EngineData_t));
    
    return g_engineData.valid ? J1939_OK : J1939_ERROR;
}

/*************************************************
 Function:        J1939App_GetVehicleSpeed
 Description:     Get latest Vehicle Speed
 Input:           None
 Output:          speedData - Speed data structure
 Return:          J1939_OK if data is valid
 Others:          
 *************************************************/
int16_t J1939App_GetVehicleSpeed(J1939VehicleSpeed_t *speedData)
{
    if (speedData == NULL)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    memcpy(speedData, &g_vehicleSpeed, sizeof(J1939VehicleSpeed_t));
    
    return g_vehicleSpeed.valid ? J1939_OK : J1939_ERROR;
}

/*************************************************
 Function:        J1939App_SetBodyControlStatus
 Description:     Set Body Control Status for TX
 Input:           status - Body control status
 Output:          None
 Return:          J1939_OK on success
 Others:          Data will be transmitted periodically
 *************************************************/
int16_t J1939App_SetBodyControlStatus(const J1939BodyControlStatus_t *status)
{
    if (status == NULL)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    memcpy(&g_bodyControlStatus, status, sizeof(J1939BodyControlStatus_t));
    
    return J1939_OK;
}

/*************************************************
 Function:        J1939App_EngineSpeedCallback
 Description:     Callback for Engine Speed PGN
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          Called by J1939 SDK when PGN 0xF004 received
 *************************************************/
void J1939App_EngineSpeedCallback(const J1939Message_t *msg)
{
    J1939AppParseEngineSpeed(msg);
}

/*************************************************
 Function:        J1939App_VehicleSpeedCallback
 Description:     Callback for Vehicle Speed PGN
 Input:           msg - J1939 message
 Output:          None
 Return:          None
 Others:          Called by J1939 SDK when PGN 0xFE6A received
 *************************************************/
void J1939App_VehicleSpeedCallback(const J1939Message_t *msg)
{
    J1939AppParseVehicleSpeed(msg);
}

/*************************************************
 Function:        J1939App_TransmitBodyStatus
 Description:     Transmit Body Control Status PGN
 Input:           None
 Output:          None
 Return:          J1939_OK on success
 Others:          PGN 0xFF02 broadcast
 *************************************************/
int16_t J1939App_TransmitBodyStatus(void)
{
    uint8_t data[8];
    uint8_t length;
    int16_t ret;
    
    if (!g_appInitialized)
    {
        return J1939_ERROR_NOT_INITIALIZED;
    }
    
    ret = J1939AppBuildBodyStatusMsg(data, &length);
    if (ret != J1939_OK)
    {
        return ret;
    }
    
    ret = J1939Sdk_Transmit(J1939_PGN_BODY_CONTROL_STATUS, 
                           J1939_GLOBAL_ADDRESS, data, length);
    if (ret == J1939_OK)
    {
        g_appStats.bodyStatusTxCount++;
    }
    else
    {
        g_appStats.errorCount++;
    }
    
    return ret;
}

/*************************************************
 Function:        J1939App_TransmitCustomPgn
 Description:     Transmit custom PGN
 Input:           pgn - PGN to transmit
                  data - Data payload
                  length - Data length
 Output:          None
 Return:          J1939_OK on success
 Others:          Broadcast transmission
 *************************************************/
int16_t J1939App_TransmitCustomPgn(uint32_t pgn, uint8_t *data, uint8_t length)
{
    if (!g_appInitialized)
    {
        return J1939_ERROR_NOT_INITIALIZED;
    }
    
    if (data == NULL || length == 0 || length > 8)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    return J1939Sdk_Transmit(pgn, J1939_GLOBAL_ADDRESS, data, length);
}

/*************************************************
 Function:        J1939App_GetStatistics
 Description:     Get application statistics
 Input:           None
 Output:          stats - Statistics structure
 Return:          None
 Others:          
 *************************************************/
void J1939App_GetStatistics(J1939AppStatistics_t *stats)
{
    if (stats != NULL)
    {
        memcpy(stats, &g_appStats, sizeof(J1939AppStatistics_t));
    }
}

/*************************************************
 Function:        J1939App_ResetStatistics
 Description:     Reset application statistics
 Input:           None
 Output:          None
 Return:          None
 Others:          
 *************************************************/
void J1939App_ResetStatistics(void)
{
    memset(&g_appStats, 0, sizeof(J1939AppStatistics_t));
}

/*************************************************
 Function:        J1939App_PrintEngineData
 Description:     Print Engine Data via debug output
 Input:           None
 Output:          None
 Return:          None
 Others:          For debugging purposes
 *************************************************/
void J1939App_PrintEngineData(void)
{
    if (g_engineData.valid)
    {
        TBOX_PRINT("J1939: Engine Speed = %d RPM, Load = %d%%\r\n",
                   g_engineData.engineSpeed,
                   g_engineData.enginePercentLoad);
    }
    else
    {
        TBOX_PRINT("J1939: Engine Data not available\r\n");
    }
}

/*************************************************
 Function:        J1939App_PrintVehicleSpeed
 Description:     Print Vehicle Speed via debug output
 Input:           None
 Output:          None
 Return:          None
 Others:          For debugging purposes
 *************************************************/
void J1939App_PrintVehicleSpeed(void)
{
    if (g_vehicleSpeed.valid)
    {
        TBOX_PRINT("J1939: Vehicle Speed = %d.%02d km/h\r\n",
                   g_vehicleSpeed.vehicleSpeed / 100,
                   g_vehicleSpeed.vehicleSpeed % 100);
    }
    else
    {
        TBOX_PRINT("J1939: Vehicle Speed not available\r\n");
    }
}

/*************************************************
 Function:        J1939AppTpRxCallback
 Description:     Transport Protocol RX complete callback
 Input:           pgn - PGN received
                  sourceAddr - Source address
                  data - Data buffer
                  length - Data length
 Output:          None
 Return:          None
 Others:          Called when multi-frame message received
 *************************************************/
static void J1939AppTpRxCallback(uint32_t pgn, uint8_t sourceAddr,
                                 const uint8_t *data, uint16_t length)
{
    (void)data;
    
    TBOX_PRINT("J1939 TP: Received PGN 0x%05X from SA 0x%02X, length %d\r\n",
               pgn, sourceAddr, length);
    
    /* Example: Process based on PGN */
    switch (pgn)
    {
        case J1939_PGN_VIN:
            /* Vehicle Identification Number received */
            TBOX_PRINT("J1939 TP: VIN received\r\n");
            break;
            
        case J1939_PGN_SOFTWARE_IDENTIFICATION:
            /* Software ID received */
            TBOX_PRINT("J1939 TP: Software ID received\r\n");
            break;
            
        default:
            /* Other multi-frame PGN */
            break;
    }
}

/*************************************************
 Function:        J1939AppTpTxCallback
 Description:     Transport Protocol TX complete callback
 Input:           pgn - PGN transmitted
                  destAddr - Destination address
                  status - 0=success, 1=error
 Output:          None
 Return:          None
 Others:          Called when multi-frame transmission complete
 *************************************************/
static void J1939AppTpTxCallback(uint32_t pgn, uint8_t destAddr, uint8_t status)
{
    if (status == 0)
    {
        TBOX_PRINT("J1939 TP: PGN 0x%05X sent to 0x%02X successfully\r\n", 
                   pgn, destAddr);
    }
    else
    {
        TBOX_PRINT("J1939 TP: PGN 0x%05X send to 0x%02X failed\r\n", 
                   pgn, destAddr);
    }
}

/*************************************************
 Function:        J1939App_TransmitLargeData
 Description:     Example: Transmit large data using TP
 Input:           destAddr - Destination address
                  data - Large data buffer
                  length - Data length (9-1785 bytes)
 Output:          None
 Return:          J1939_OK on success
 Others:          Uses RTS/CTS for specific destination, BAM for broadcast
 *************************************************/
int16_t J1939App_TransmitLargeData(uint8_t destAddr, uint8_t *data, uint16_t length)
{
    int16_t ret;
    
    if (!g_appInitialized)
    {
        return J1939_ERROR_NOT_INITIALIZED;
    }
    
    if (data == NULL || length <= 8 || length > J1939_MAX_DATA_LENGTH)
    {
        return J1939_ERROR_INVALID_PARAM;
    }
    
    /* Register TP callbacks */
    J1939Tp_RegisterRxCallback(J1939AppTpRxCallback);
    J1939Tp_RegisterTxCallback(J1939AppTpTxCallback);

    /* Send using appropriate method */
    if (destAddr == J1939_GLOBAL_ADDRESS)
    {
        /* Broadcast using BAM */
        ret = J1939Tp_TransmitBam(J1939_PGN_BODY_CONTROL_STATUS, data, length);
    }
    else
    {
        /* Point-to-point using RTS/CTS */
        ret = J1939Tp_Transmit(J1939_PGN_BODY_CONTROL_STATUS, destAddr, data, length);
    }
    
    return ret;
}
