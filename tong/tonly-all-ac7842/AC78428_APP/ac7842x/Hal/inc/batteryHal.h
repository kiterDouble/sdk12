/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: batteryHal.h
 Author: 
 Created Time: 
 Description: 
 Others: 
*************************************************/
#ifndef _BATTERYHAL_H
#define _BATTERYHAL_H

/****************************** include ***************************************/
#include <stdint.h>

/****************************** Macro Definitions *****************************/
/* error code */
#define BATTERYHAL_OK                    0
#define BATTERYHAL_ERR                  -1

/* battery state */
#define BATTERYHAL_STATE_IDLE            0
#define BATTERYHAL_STATE_CHARGE          1
#define BATTERYHAL_STATE_EN              2
/****************************** Type Definitions ******************************/
/****************************** Function Declarations *************************/
/*************************************************
Function:     BatteryHalInit
Description:  Battery module init
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalInit(void);

/*************************************************
Function:     BatteryHalEnableOut
Description:  Battery enable out
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalEnableOut(void);

/*************************************************
Function:     BatteryHalDisableOut
Description:  Battery disable out
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalDisableOut(void);

/*************************************************
Function:     BatteryHalEnableCharge
Description:  Battery enable charge
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalEnableCharge(void);

/*************************************************
Function:     BatteryHalDisableCharge
Description:  Battery disable charge
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalDisableCharge(void);

/*************************************************
Function:     BatteryHalGetVoltage
Description:  Get battery voltage
Input:        None
Output:       pVoltage : Address of the output voltage value
Return:       0  : get success
              -1 : get failed
Others:       None
*************************************************/
int16_t BatteryHalGetVoltage(uint32_t *pVoltage);

/*************************************************
Function:     BatteryHalGetNtc
Description:  Get NTC AD value
Input:        None
Output:       pVoltage : Address of the output voltage value
Return:       0  : get success
              -1 : get failed
Others:       None
*************************************************/
int16_t BatteryHalGetNtc(uint32_t *pVoltage);

/*************************************************
Function:     BatteryHalEnableCheck
Description:  Battery enable check 
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalEnableCheck(uint8_t flag);

/*************************************************
Function:     BatteryHalGetState
Description:  Get battery state
Input:        None
Output:       None
Return:       0 : idle state
              1 : charge state
              2 : enable state
Others:       None
*************************************************/
int16_t BatteryHalGetState(void);

#endif  //_BATTERYHAL_H