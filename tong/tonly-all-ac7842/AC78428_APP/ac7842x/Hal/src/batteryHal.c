/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: batteryHal.c
 Description: Battery module HAL adapted for new AutoChips SDK
*************************************************/
/****************************** include ***************************************/
#include "batteryHal.h"
#include "peripheralHal.h"
#include "Gpio_Hal.h"

/****************************** Macro Definitions ******************************/
/* CHAR_ON */
#define BATTERY_CHARGE_PORT           PORTID_C
#define BATTERY_CHARGE_PIN            8

/* 5V_OUT_ON */
#define BATTERY_OUT_PORT              PORTID_A
#define BATTERY_OUT_PIN               0

/* BATTERY_ON */
#define BATTERY_SWITCHON_PORT         PORTID_E
#define BATTERY_SWITCHON_PIN          9

/* CHECK_BATTERY */
#define BATTERY_CHECK_PORT            PORTID_D
#define BATTERY_CHECK_PIN             17

/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/
/****************************** Function Declarations *************************/
/****************************** Public Function Implementations ***************/

/*************************************************
Function:     BatteryHalInit
Description:  Battery module init
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalInit(void)
{
    /* CHAR_ON pin init */
    Gpio_Hal_SetMuxMode(BATTERY_CHARGE_PORT, BATTERY_CHARGE_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(BATTERY_CHARGE_PORT, BATTERY_CHARGE_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(BATTERY_CHARGE_PORT, BATTERY_CHARGE_PIN, GPIO_LOW_LEVEL);

    /* 5V_OUT_ON pin init */
    Gpio_Hal_SetMuxMode(BATTERY_OUT_PORT, BATTERY_OUT_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPullSel(BATTERY_OUT_PORT, BATTERY_OUT_PIN, PORT_INTERNAL_PULL_DOWN_ENABLED);
    Gpio_Hal_SetPinDirection(BATTERY_OUT_PORT, BATTERY_OUT_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(BATTERY_OUT_PORT, BATTERY_OUT_PIN, GPIO_LOW_LEVEL);

    /* BATTERY_ON pin init */
    Gpio_Hal_SetMuxMode(BATTERY_SWITCHON_PORT, BATTERY_SWITCHON_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(BATTERY_SWITCHON_PORT, BATTERY_SWITCHON_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(BATTERY_SWITCHON_PORT, BATTERY_SWITCHON_PIN, GPIO_LOW_LEVEL);

    /* CHECK_BATTERY pin init */
    Gpio_Hal_SetMuxMode(BATTERY_CHECK_PORT, BATTERY_CHECK_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(BATTERY_CHECK_PORT, BATTERY_CHECK_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(BATTERY_CHECK_PORT, BATTERY_CHECK_PIN, GPIO_LOW_LEVEL);
}

/*************************************************
Function:     BatteryHalEnableOut
Description:  Battery enable out
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalEnableOut(void)
{
  /* start battery switch */
  Gpio_Hal_WritePin(BATTERY_SWITCHON_PORT, BATTERY_SWITCHON_PIN, GPIO_HIGH_LEVEL);
  /* start battery boost */
  Gpio_Hal_WritePin(BATTERY_OUT_PORT, BATTERY_OUT_PIN, GPIO_HIGH_LEVEL);
}

/*************************************************
Function:     BatteryHalDisableOut
Description:  Battery disable out
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalDisableOut(void)
{
  /* close battery switch */
  Gpio_Hal_WritePin(BATTERY_SWITCHON_PORT, BATTERY_SWITCHON_PIN, GPIO_LOW_LEVEL);
  /* close battery boost */
  Gpio_Hal_WritePin(BATTERY_OUT_PORT, BATTERY_OUT_PIN, GPIO_LOW_LEVEL);
}

/*************************************************
Function:     BatteryHalEnableCharge
Description:  Battery enable charge
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalEnableCharge(void)
{
  /* start battery charge */
  Gpio_Hal_WritePin(BATTERY_CHARGE_PORT, BATTERY_CHARGE_PIN, GPIO_HIGH_LEVEL);
}

/*************************************************
Function:     BatteryHalDisableCharge
Description:  Battery disable charge
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalDisableCharge(void)
{
  /* stop battery charge */
  Gpio_Hal_WritePin(BATTERY_CHARGE_PORT, BATTERY_CHARGE_PIN, GPIO_LOW_LEVEL);
}

/*************************************************
Function:     BatteryHalGetVoltage
Description:  Get battery voltage
Input:        None
Output:       pVoltage : Address of the output voltage value
Return:       0  : get success
              -1 : get failed
Others:       None
*************************************************/
int16_t BatteryHalGetVoltage(uint32_t *pVoltage)
{
    int16_t ret = BATTERYHAL_ERR;
    uint32_t voltage = 0u;

    ret = PeripheralHalAdGet(AD_CHANNEL_BATTERY, &voltage);
    if (ret == BATTERYHAL_OK) {
        *pVoltage = voltage;
    }

    return ret;
}

/*************************************************
Function:     BatteryHalGetNtc
Description:  Get NTC AD value
Input:        None
Output:       pVoltage : Address of the output voltage value
Return:       0  : get success
              -1 : get failed
Others:       None
*************************************************/
int16_t BatteryHalGetNtc(uint32_t *pVoltage)
{
    int16_t ret = BATTERYHAL_ERR;
    uint32_t voltage = 0u;

    ret = PeripheralHalAdGet(AD_CHANNEL_NTC, &voltage);
    if (ret == BATTERYHAL_OK) {
        *pVoltage = voltage;
    }

    return ret;
}

/*************************************************
Function:     BatteryHalEnableCheck
Description:  Battery enable check 
Input:        None
Output:       None
Return:       None
Others:       None
*************************************************/
void BatteryHalEnableCheck(uint8_t flag)
{
  /* Currently empty implementation as original */
}

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
int16_t BatteryHalGetState(void)
{
    int16_t ret = BATTERYHAL_STATE_IDLE;
    Gpio_Hal_LevelType level = GPIO_LOW_LEVEL;
    
    do {
        level = Gpio_Hal_GetPinOutputLevel(BATTERY_CHARGE_PORT, BATTERY_CHARGE_PIN);
        if (level == GPIO_HIGH_LEVEL) {
            ret = BATTERYHAL_STATE_CHARGE;
            break;
        }
        
        level = Gpio_Hal_GetPinOutputLevel(BATTERY_SWITCHON_PORT, BATTERY_SWITCHON_PIN);
        if (level == GPIO_HIGH_LEVEL) {
            ret = BATTERYHAL_STATE_EN;
        } else {
            ret = BATTERYHAL_STATE_IDLE;
        }
    } while (0);
    
    return ret;
}