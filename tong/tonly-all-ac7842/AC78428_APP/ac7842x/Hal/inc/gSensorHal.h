/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: gSensorHal.h
 Author:
 Created Time:
 Description:
 Others:
*************************************************/
#ifndef _GSENSORHAL_H
#define _GSENSORHAL_H

/****************************** include ***************************************/
#include <stdint.h>

/****************************** Macro Definitions *****************************/
#define LSM6DSM_WHO_AM_I 0x0FU
#define LSM6DSM_CTRL1_XL 0x10U
#define LSM6DSM_STATUS_REG 0x1E

#define LSM6DSM_INT1_CTRL 0x0D
#define LSM6DSM_INT2_CTRL 0x0E

#define LSM6DSM_OUTX_L_XL 0x28
#define LSM6DSM_OUTX_H_XL 0x29
#define LSM6DSM_OUTY_L_XL 0x2A
#define LSM6DSM_OUTY_H_XL 0x2B
#define LSM6DSM_OUTZ_L_XL 0x2C
#define LSM6DSM_OUTZ_H_XL 0x2D

#define LSM6DSM_TAP_CFG 0x58
#define LSM6DSM_WAKE_UP_THS 0x5B
#define LSM6DSM_WAKE_UP_DUR 0x5C
#define LSM6DSM_MD1_CFG 0x5E
#define LSM6DSM_CTRL3_C 0x12

/****************************** Type Definitions ******************************/
typedef struct
{
    short accel_x;
    short accel_y;
    short accel_z;
} GSensorHalData_t;

/****************************** Function Declarations *************************/
/*************************************************
  Function:       GSensorHalInit
  Description:    Gsensor module init
  Input:          flag : Whether to set interrupt wake up
  Output:         none
  Return:         none
  Others:         none
*************************************************/
void GSensorHalInit(uint8_t flag);
/*************************************************
  Function:       GSensorHalGetValue
  Description:    Get Gsensor data
  Input:          none
  Output:         GSensordata : The obtained data address
  Return:         0 : get success
                  1 : get failed
  Others:         none
*************************************************/
int16_t GSensorHalGetValue(GSensorHalData_t *GSensordata);
/*************************************************
  Function:       GSensorHalGetWakeupFlag
  Description:    Get wake-up flag
  Input:          none
  Output:         none
  Return:         Lsm6dsmWakeupFlag : set the wake-up flag
                  0 : no wake-up
                  1 : set wake-up
  Others:         none
*************************************************/
int16_t GSensorHalGetWakeupFlag(void);
/*************************************************
  Function:       GSensorHalSetThreshold
  Description:    Set Gsensor interrupt threshold
  Input:          Threshold : interrupt threshold,The unit is determined by the set Accelerometer full-scale selection,
                              The default setting is 2g.
  Output:         none
  Return:         0 : set success
                  1 : set failed
  Others:         none
*************************************************/
int16_t GSensorHalSetThreshold(uint32_t Threshold);

void GSensorHalSetWakeup(void);

#endif //_GSENSORHAL_H