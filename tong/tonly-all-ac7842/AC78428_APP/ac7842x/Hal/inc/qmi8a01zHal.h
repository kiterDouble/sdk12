/*************************************************
 Copyright © 2025 SiRun (WuXi). All rights reserved.
 File Name: qmi8a01zHal.h
 Author: 
 Created Time: 2025-01-28
 Description: QMI8A01z 6-axis IMU sensor HAL driver
 Others: QMI8A01z is part of QMI8658 family, supports I2C interface
 *************************************************/

#ifndef _QMI8A01ZHAL_H
#define _QMI8A01ZHAL_H

/****************************** include ***************************************/
#include <stdint.h>

/****************************** Macro Definitions *****************************/
/* QMI8A01z I2C Slave Address */
#define QMI8A01Z_I2C_ADDR           0x6BU   /* SA0接地时为0x6B，接VCC时为0x6A */

/* Device ID(WHO_AM_I Value) */
#define QMI8A01Z_DEVICE_ID          0x05U

/* Device Revision ID(REVISION_ID Value) */
#define QMI8A01Z_REVISION_ID        0x7CU


/********************************************* Start QMI8A01z Register Map ********************************************/

/* General Purpose Registers */
#define QMI8A01Z_CMD_WHO_AM_I       0x00U   /* Device Identifier Register */
#define QMI8A01Z_CMD_REVISION_ID    0x01U   /* Device Revision ID Register */

/* Setup and Control Registers */
#define QMI8A01Z_CMD_CTRL1          0x02U   /* SPI Interface and Sensor Enable Register */
#define QMI8A01Z_CMD_CTRL2          0x03U   /* Accelerometer: Output Data Rate, Full Scale, Self-Test Register */
#define QMI8A01Z_CMD_CTRL3          0x04U   /* Gyroscope: Output Data Rate, Full Scale, Self-Test Register */
// #define QMI8A01Z_CTRL4           0x05U    // Control Register 4
#define QMI8A01Z_CMD_CTRL5          0x06U   /* Low pass filter setting Register */
// #define QMI8A01Z_CTRL6           0x07U    // Control Register 6
#define QMI8A01Z_CMD_CTRL7          0x08U   /* Enable Sensors Register */
#define QMI8A01Z_CMD_CTRL8          0x09U   /* Motion Detection Control Register */
#define QMI8A01Z_CMD_CTRL9          0x0AU   /* Host Commands Register */

/* Host Controlled Calibration Registers */
#define QMI8A01Z_CMD_CAL1_L         0x0BU
#define QMI8A01Z_CMD_CAL1_H         0x0CU
#define QMI8A01Z_CMD_CAL2_L         0x0DU
#define QMI8A01Z_CMD_CAL2_H         0x0EU
#define QMI8A01Z_CMD_CAL3_L         0x0FU
#define QMI8A01Z_CMD_CAL3_H         0x10U
#define QMI8A01Z_CMD_CAL4_L         0x11U
#define QMI8A01Z_CMD_CAL4_H         0x12U

/* FIFO Registers */
#define QMI8A01Z_CMD_FIFO_WTM_TH    0x13U
#define QMI8A01Z_CMD_FIFO_CTRL      0x14U
#define QMI8A01Z_CMD_FIFO_SMPL_CNT  0x15U
#define QMI8A01Z_CMD_FIFO_STATUS    0x16U
#define QMI8A01Z_CMD_FIFO_DATA      0x17U

/* Status Registers */
#define QMI8A01Z_CMD_STATUSINT      0x2DU   /* Sensor Data Availability with the Locking mechanism, CmdDone */
#define QMI8A01Z_CMD_STATUS0        0x2EU   /* Output Data Over Run and Data Availability */
#define QMI8A01Z_CMD_STATUS1        0x2FU   /* Miscellaneous Status: Any Motion, No Motion, Significant Motion */

/* Timestamp Registers */
#define QMI8A01Z_CMD_TIMESTAMP_LOW  0x30U
#define QMI8A01Z_CMD_TIMESTAMP_MID  0x31U
#define QMI8A01Z_CMD_TIMESTAMP_HIGH 0x32U

/* Data Output Registers */
#define QMI8A01Z_CMD_TEMP_L         0x33U   /* Temperature Output Data, lower 8 bits */
#define QMI8A01Z_CMD_TEMP_H         0x34U   /* Temperature Output Data, upper 8 bits */
#define QMI8A01Z_CMD_AX_L           0x35U    /* X-axis Acceleration LSB */
#define QMI8A01Z_CMD_AX_H           0x36U    /* X-axis Acceleration MSB */
#define QMI8A01Z_CMD_AY_L           0x37U    /* Y-axis Acceleration LSB */
#define QMI8A01Z_CMD_AY_H           0x38U    /* Y-axis Acceleration MSB */
#define QMI8A01Z_CMD_AZ_L           0x39U    /* Z-axis Acceleration LSB */
#define QMI8A01Z_CMD_AZ_H           0x3AU    /* Z-axis Acceleration MSB */
#define QMI8A01Z_CMD_GX_L           0x3BU    /* X-axis Angular Rate LSB */
#define QMI8A01Z_CMD_GX_H           0x3CU    /* X-axis Angular Rate MSB */
#define QMI8A01Z_CMD_GY_L           0x3DU    /* Y-axis Angular Rate LSB */
#define QMI8A01Z_CMD_GY_H           0x3EU    /* Y-axis Angular Rate MSB */
#define QMI8A01Z_CMD_GZ_L           0x3FU    /* Z-axis Angular Rate LSB */
#define QMI8A01Z_CMD_GZ_H           0x40U    /* Z-axis Angular Rate MSB */

/* Activity Detection Output Registers */
#define QMI8A01Z_CMD_TAP_STATUS     0x59U

/* Reset Register */
#define QMI8A01Z_CMD_RESET          0x60U    /* Soft Reset Register */

/********************************************* End QMI8A01z Register Map ********************************************/


/* Control Register 1 Bit Definitions */
#define QMI8A01Z_CTRL1_ACCEL_ENABLE    0x01U    // Enable Accelerometer
#define QMI8A01Z_CTRL1_GYRO_ENABLE     0x02U    // Enable Gyroscope

/* Accelerometer Range Settings (CTRL1) */
#define QMI8A01Z_ACCEL_RANGE_2G       0x00U    // ±2g range
#define QMI8A01Z_ACCEL_RANGE_4G       0x04U    // ±4g range
#define QMI8A01Z_ACCEL_RANGE_8G       0x08U    // ±8g range
#define QMI8A01Z_ACCEL_RANGE_16G      0x0CU    // ±16g range

/* Accelerometer ODR Settings (CTRL2) */
#define QMI8A01Z_ACCEL_ODR_21HZ       0x10U    // 21Hz output data rate
#define QMI8A01Z_ACCEL_ODR_105HZ      0x20U    // 105Hz output data rate
#define QMI8A01Z_ACCEL_ODR_209HZ      0x30U    // 209Hz output data rate
#define QMI8A01Z_ACCEL_ODR_417HZ      0x40U    // 417Hz output data rate
#define QMI8A01Z_ACCEL_ODR_833HZ      0x50U    // 833Hz output data rate
#define QMI8A01Z_ACCEL_ODR_1666HZ     0x60U    // 1666Hz output data rate
#define QMI8A01Z_ACCEL_ODR_3332HZ     0x70U    // 3332Hz output data rate

/* Status Register Bit Definitions */
#define QMI8A01Z_STATUS_ACCEL_RDY      0x01U    // Accelerometer data ready

/* Scaling Factors */
#define QMI8A01Z_ACCEL_SCALE_2G        16384U    // LSB/g for ±2g range
#define QMI8A01Z_ACCEL_SCALE_4G        8192U     // LSB/g for ±4g range
#define QMI8A01Z_ACCEL_SCALE_8G        4096U     // LSB/g for ±8g range
#define QMI8A01Z_ACCEL_SCALE_16G       2048U     // LSB/g for ±16g range

/* Error Codes */
#define QMI8A01Z_OK                    0          // Success
#define QMI8A01Z_ERR_INIT              -1         // Initialization failed
#define QMI8A01Z_ERR_COMM              -2         // Communication error
#define QMI8A01Z_ERR_NO_DEVICE         -3         // Device not found
#define QMI8A01Z_ERR_TIMEOUT           -4         // Timeout error

/****************************** Type Definitions ******************************/
typedef struct
{
    int16_t accel_x;    /* 加速度计X轴原始值 */
    int16_t accel_y;    /* 加速度计Y轴原始值 */
    int16_t accel_z;    /* 加速度计Z轴原始值 */
    int16_t gyro_x;     /* 陀螺仪X轴原始值 */
    int16_t gyro_y;     /* 陀螺仪Y轴原始值 */
    int16_t gyro_z;     /* 陀螺仪Z轴原始值 */
} Qmi8a01zHalData_t;

/****************************** Function Declarations *************************/

/**
 * @brief 初始化QMI8A01Z
 * 
 * @return int 
 */
int Qmi8a01z_Init(void);

/**
 * @brief 读取DeviceId
 * 
 * @return int 
 */
int Qmi8a01z_GetDeviceId(void);

/*************************************************
  Function:       Qmi8a01zHalGetValue
  Description:    Get QMI8A01z accelerometer data
  Input:          none
  Output:         sensorData : Pointer to data structure to store values
  Return:         0 : get success
                  1 : get failed
  Others:         Data is returned in mg units
*************************************************/
int16_t Qmi8a01zHalGetValue(Qmi8a01zHalData_t *sensorData);

/*************************************************
  Function:       Qmi8a01zHalGetWakeupFlag
  Description:    Get wake-up flag status
  Input:          none
  Output:         none
  Return:         0 : no wake-up
                  1 : wake-up detected
  Others:         Checks motion detection interrupt flag
*************************************************/
int16_t Qmi8a01zHalGetWakeupFlag(void);

/*************************************************
  Function:       Qmi8a01zHalSetThreshold
  Description:    Set motion detection threshold
  Input:          threshold : Motion detection threshold value
  Output:         none
  Return:         0 : set success
                  1 : set failed
  Others:         Configures interrupt threshold for wake-up
*************************************************/
int16_t Qmi8a01zHalSetThreshold(uint32_t threshold);

/*************************************************
  Function:       Qmi8a01zHalSetWakeup
  Description:    Enable/disable wake-up function
  Input:          none
  Output:         none
  Return:         none
  Others:         Configures sensor for motion wake-up detection
*************************************************/
void Qmi8a01zHalSetWakeup(void);

#endif //_QMI8A01ZHAL_H