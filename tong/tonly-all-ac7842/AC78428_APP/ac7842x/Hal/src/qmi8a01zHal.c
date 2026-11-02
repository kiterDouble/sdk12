/*************************************************
 Copyright © 2025 SiRun (WuXi). All rights reserved.
 File Name: qmi8a01zHal.c
 Author: 
 Created Time: 2025-01-28
 Description: QMI8A01z 6-axis IMU sensor HAL driver implementation
 Others: QMI8A01z is part of QMI8658 family, supports I2C interface
 *************************************************/

/****************************** include ***************************************/
#include "qmi8a01zHal.h"
#include "logHal.h"

#include "Gpio_Hal.h"
#include "I2c_Hal.h"
#include "Ckgen_Hal.h"
#include "Device_Types.h"

#include <stdarg.h>
#include <stdio.h>

/****************************** Macro Definitions ******************************/
/* I2C Configuration - Reuse existing gSensorHal configuration */
#define IMU_I2C_INDEX                   0
#define IMU_I2C_IRQN                    I2C0_IRQn

/* GPIO Pin Configuration - Reuse existing pins */
#define IMU_SDA_PORT                    PORTID_A
#define IMU_SDA_PIN                     2
#define IMU_SDA_PIN_MUX                 PORT_MUX_ALT2

#define IMU_SCL_PORT                    PORTID_A
#define IMU_SCL_PIN                     3
#define IMU_SCL_PIN_MUX                 PORT_MUX_ALT2

#define IMU_INT_PORT                    PORTID_C
#define IMU_INT_PIN                     5
#define IMU_INT_PIN_MUX                 PORT_MUX_AS_GPIO

/* QMI8A01z 7-bit I2C slave address (0x6B >> 1 = 0x35) */
#define QMI8A01Z_I2C_SLAVE_ADDR_7BIT    ((QMI8A01Z_I2C_ADDR) >> 1)

/* Use Software I2C Implementation */
// #define USE_SW_I2C

/* I2C Timing Configuration */
#define I2C_DELAY_COUNT                23        // Software I2C delay for ~238kHz

/* Communication Timeout */
#define QMI8A01Z_TIMEOUT_COUNT         10000     // Initialization timeout
#define QMI8A01Z_DATA_READY_TIMEOUT    1000      // Data ready timeout

/* Default Configuration */
#define QMI8A01Z_DEFAULT_ACCEL_RANGE   QMI8A01Z_ACCEL_RANGE_2G
#define QMI8A01Z_DEFAULT_ACCEL_ODR     QMI8A01Z_ACCEL_ODR_105HZ

#ifdef USE_SW_I2C
/* Software I2C GPIO Macros */
#define iic_scl_1()                    Gpio_Hal_WritePin(IMU_SCL_PORT, IMU_SCL_PIN, 1)
#define iic_scl_0()                    Gpio_Hal_WritePin(IMU_SCL_PORT, IMU_SCL_PIN, 0)
#define iic_sda_1()                    Gpio_Hal_WritePin(IMU_SDA_PORT, IMU_SDA_PIN, 1)
#define iic_sda_0()                    Gpio_Hal_WritePin(IMU_SDA_PORT, IMU_SDA_PIN, 0)
#define iic_sda_read()                 Gpio_Hal_ReadPin(IMU_SDA_PORT, IMU_SDA_PIN)
#define iic_sda_in()                   // GPIO input configuration handled in init
#define iic_sda_out()                  // GPIO output configuration handled in init
#endif

#define EX_I2C_LEN               (4U)

/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/

static uint8_t Qmi8a01zWakeupFlag = 0;            // Wake-up detection flag
static uint16_t Qmi8a01zAccelScale = QMI8A01Z_ACCEL_SCALE_2G;  // Current scaling factor

/* I2C0模块的主机配置数据 */
static const I2c_Hal_MasterConfigType g_I2cMasterConfig = {
    .TxDmaChannel = 0,           // 不使用DMA
    .RxDmaChannel = 0,           // 不使用DMA
    .SdaPin = 0,                 // HW I2C不需要EIO引脚号
    .SclPin = 0,                 // HW I2C不需要EIO引脚号
    .ResourceIndex = 0,          // HW I2C不需要
    .SlaveAddress = QMI8A01Z_I2C_SLAVE_ADDR_7BIT,  // 7-bit从机地址
    .PinLow = 0,                 // 不使用PLT
    .BaudRate = 100000,          // 标准模式 100Kbps
    .SendStop = TRUE,            // 自动发送STOP
    .SclSdaLowEn = FALSE,        // 不使用PLT
    .SyncEn = TRUE,              // 使能SCL同步
    .Arbitration = TRUE,         // 使能仲裁
    .Is10bitAddr = FALSE,        // 7-bit地址
    .TransferType = I2C_USING_INTERRUPTS,  // 使用中断模式
    .TimeCfgTypes = I2C_SCL,     // 仅SCL超时检测
};

/* I2C0通道配置 */
static const I2c_Hal_ChannelConfigType g_I2cConfig = {
    .I2cType = HW_I2C,           // 硬件I2C
    .I2cMode = I2C_MASTER,       // 主机模式
    .Callback = NULL,            // 不使用回调（当前所有中断禁用）
    .MasterConfigPtr = &g_I2cMasterConfig,
    .SlaveConfigPtr = NULL,
};

/****************************** Function Declarations *************************/
static Hal_StatusType I2C0_Init(void);
static int16_t Qmi8a01zWriteReg(uint8_t reg, uint8_t value);
static int16_t Qmi8a01zReadReg(uint8_t reg, uint8_t *value);
static int16_t Qmi8a01zReadMultiReg(uint8_t reg, uint8_t *buffer, uint8_t length);

#ifdef USE_SW_I2C
static void iic_delay(void);
static void iic_start(void);
static void iic_stop(void);
static uint8_t iic_wait_ack(void);
static void iic_ack(void);
static void iic_no_ack(void);
static void iic_send_byte(uint8_t byte);
static uint8_t iic_read_byte(uint8_t ack);
#endif

/****************************** Public Function Implementations ***************/

/**
 * @brief 初始化QMI8A01Z
 * 
 * @return int 
 */
int Qmi8a01z_Init(void)
{
    /* Initialize I2C communication */
    I2C0_Init();

    return 0;
}

/**
 * @brief 读取DeviceId
 * 
 * @return int 
 */
int Qmi8a01z_GetDeviceId(void)
{
    int ret = -1;
    uint8_t deviceId = 0;

    /* Read WHO_AM_I register using SyncTransceive */
    Hal_StatusType status = Qmi8a01zReadReg(QMI8A01Z_CMD_WHO_AM_I, &deviceId);
    if (status == QMI8A01Z_OK && deviceId == QMI8A01Z_DEVICE_ID) {
        TBOX_PRINT("QMI8A01z sensor initialized successfully\r\n");
        ret = 0;
    }

    return ret;
}

/*************************************************
  Function:       Qmi8a01zHalInit
  Description:    QMI8A01z sensor module initialization
  Input:          flag : Whether to set interrupt wake up
  Output:         none
  Return:         0 : init success
                  negative : init failed
  Others:         Initializes I2C communication and configures sensor
*************************************************/
int16_t Qmi8a01zHalInit(uint8_t flag)
{
    /* Initialize I2C communication */
    I2C0_Init();

#if 0
    /* Read WHO_AM_I register to verify device */
    if (Qmi8a01zReadReg(QMI8A01Z_WHO_AM_I, &deviceId) != QMI8A01Z_OK) {
        TBOX_PRINT("QMI8A01z: Failed to read WHO_AM_I register\r\n");
        return QMI8A01Z_ERR_COMM;
    }

    /* Verify device ID */
    if (deviceId != QMI8A01Z_DEVICE_ID) {
        TBOX_PRINT("QMI8A01z: Device ID mismatch, expected 0x%02X, got 0x%02X\r\n", 
                     QMI8A01Z_DEVICE_ID, deviceId);
        return QMI8A01Z_ERR_NO_DEVICE;
    }

    INFO_PRINT("QMI8A01z: Device detected, ID = 0x%02X\r\n", deviceId);
#endif

#if 0
    /* Perform soft reset */
    if (Qmi8a01zWriteReg(QMI8A01Z_RESET, 0x80) != QMI8A01Z_OK) {
        TBOX_PRINT("QMI8A01z: Failed to perform soft reset\r\n");
        return QMI8A01Z_ERR_COMM;
    }

    /* Wait for reset to complete */
    timerCounter = QMI8A01Z_TIMEOUT_COUNT;
    do {
        Qmi8a01zReadReg(QMI8A01Z_WHO_AM_I, &deviceId);
        timerCounter--;
        if (timerCounter < 5000) {
            /* Add small delay for reset completion */
            for (volatile int i = 0; i < 1000; i++);
        }
    } while ((deviceId != QMI8A01Z_DEVICE_ID) && (timerCounter > 0));

    if (timerCounter == 0) {
        TBOX_PRINT("QMI8A01z: Reset timeout\r\n");
        return QMI8A01Z_ERR_TIMEOUT;
    }

    /* Configure accelerometer range and ODR */
    ctrl1Value = QMI8A01Z_DEFAULT_ACCEL_RANGE | QMI8A01Z_CTRL1_ACCEL_ENABLE;
    ctrl2Value = QMI8A01Z_DEFAULT_ACCEL_ODR;

    if (Qmi8a01zWriteReg(QMI8A01Z_CTRL1, ctrl1Value) != QMI8A01Z_OK) {
        TBOX_PRINT("QMI8A01z: Failed to configure CTRL1 register\r\n");
        return QMI8A01Z_ERR_COMM;
    }

    if (Qmi8a01zWriteReg(QMI8A01Z_CTRL2, ctrl2Value) != QMI8A01Z_OK) {
        TBOX_PRINT("QMI8A01z: Failed to configure CTRL2 register\r\n");
        return QMI8A01Z_ERR_COMM;
    }

    /* Set scaling factor based on range */
    switch (QMI8A01Z_DEFAULT_ACCEL_RANGE) {
        case QMI8A01Z_ACCEL_RANGE_2G:
            Qmi8a01zAccelScale = QMI8A01Z_ACCEL_SCALE_2G;
            break;
        case QMI8A01Z_ACCEL_RANGE_4G:
            Qmi8a01zAccelScale = QMI8A01Z_ACCEL_SCALE_4G;
            break;
        case QMI8A01Z_ACCEL_RANGE_8G:
            Qmi8a01zAccelScale = QMI8A01Z_ACCEL_SCALE_8G;
            break;
        case QMI8A01Z_ACCEL_RANGE_16G:
            Qmi8a01zAccelScale = QMI8A01Z_ACCEL_SCALE_16G;
            break;
        default:
            Qmi8a01zAccelScale = QMI8A01Z_ACCEL_SCALE_2G;
            break;
    }

    /* Configure wake-up flag */
    Qmi8a01zWakeupFlag = (flag == 1) ? 1 : 0;

    INFO_PRINT("QMI8A01z: Initialization complete, range=±2g, ODR=105Hz\r\n");
#endif

    return QMI8A01Z_OK;
}

/*************************************************
  Function:       Qmi8a01zHalGetValue
  Description:    Get QMI8A01z accelerometer data
  Input:          none
  Output:         sensorData : Pointer to data structure to store values
  Return:         0 : get success
                  1 : get failed
  Others:         Data is returned in mg units
*************************************************/
int16_t Qmi8a01zHalGetValue(Qmi8a01zHalData_t *sensorData)
{
    uint8_t status;
    uint8_t buffer[6];
    short rawX, rawY, rawZ;
    double temp;

    if (sensorData == NULL) {
        TBOX_PRINT("QMI8A01z: NULL pointer parameter\r\n");
        return QMI8A01Z_ERR_COMM;
    }

    /* Check if data is ready */
    if (Qmi8a01zReadReg(QMI8A01Z_CMD_STATUS0, &status) != QMI8A01Z_OK) {
        TBOX_PRINT("QMI8A01z: Failed to read status register\r\n");
        return QMI8A01Z_ERR_COMM;
    }

    /* Wait for accelerometer data ready */
    if ((status & QMI8A01Z_STATUS_ACCEL_RDY) == 0) {
        return QMI8A01Z_ERR_COMM;  // Data not ready
    }

    /* Read all 6 acceleration data bytes (X, Y, Z) */
    if (Qmi8a01zReadMultiReg(QMI8A01Z_CMD_AX_L, buffer, 6) != QMI8A01Z_OK) {
        TBOX_PRINT("QMI8A01z: Failed to read acceleration data\r\n");
        return QMI8A01Z_ERR_COMM;
    }

    /* Combine LSB and MSB for each axis */
    rawX = (short)((buffer[1] << 8) | buffer[0]);
    rawY = (short)((buffer[3] << 8) | buffer[2]);
    rawZ = (short)((buffer[5] << 8) | buffer[4]);

    /* Convert raw data to mg units */
    temp = ((double)rawX) / Qmi8a01zAccelScale;
    sensorData->accel_x = (short)(temp * 1000);

    temp = ((double)rawY) / Qmi8a01zAccelScale;
    sensorData->accel_y = (short)(temp * 1000);

    temp = ((double)rawZ) / Qmi8a01zAccelScale;
    sensorData->accel_z = (short)(temp * 1000);

    return QMI8A01Z_OK;
}

/*************************************************
  Function:       Qmi8a01zHalGetWakeupFlag
  Description:    Get wake-up flag status
  Input:          none
  Output:         none
  Return:         0 : no wake-up
                   1 : wake-up detected
  Others:         Checks motion detection interrupt flag
*************************************************/
int16_t Qmi8a01zHalGetWakeupFlag(void)
{
    return Qmi8a01zWakeupFlag;
}

/*************************************************
  Function:       Qmi8a01zHalSetThreshold
  Description:    Set motion detection threshold
  Input:          threshold : Motion detection threshold value
  Output:         none
  Return:         0 : set success
                   1 : set failed
  Others:         Configures interrupt threshold for wake-up
*************************************************/
int16_t Qmi8a01zHalSetThreshold(uint32_t threshold)
{
    uint8_t thresholdValue;

    /* Validate threshold range */
    if (threshold > 255) {
        thresholdValue = 255;
    } else {
        thresholdValue = (uint8_t)threshold;
    }

    /* Configure motion detection threshold */
    /* Note: This is a simplified implementation */
    /* Full implementation would require configuring specific motion detection registers */
    if (Qmi8a01zWriteReg(QMI8A01Z_CMD_CTRL5, thresholdValue) != QMI8A01Z_OK) {
        TBOX_PRINT("QMI8A01z: Failed to set motion detection threshold\r\n");
        return QMI8A01Z_ERR_COMM;
    }

    INFO_PRINT("QMI8A01z: Motion detection threshold set to %d\r\n", thresholdValue);
    return QMI8A01Z_OK;
}

/*************************************************
  Function:       Qmi8a01zHalSetWakeup
  Description:    Enable/disable wake-up function
  Input:          none
  Output:         none
  Return:         none
  Others:         Configures sensor for motion wake-up detection
*************************************************/
void Qmi8a01zHalSetWakeup(void)
{
    /* Disable motion detection */
    /* Note: This is a simplified implementation */
    /* Full implementation would require disabling specific motion detection features */
    Qmi8a01zWriteReg(QMI8A01Z_CMD_CTRL5, 0x00);
    Qmi8a01zWakeupFlag = 0;
    
    INFO_PRINT("QMI8A01z: Wake-up function disabled\r\n");
}

/****************************** Private Function Implementations ***************/

/*************************************************
  Function:       I2C0_Init
  Description:    Initialize I2C0 for QMI8A01z communication
  Input:          none
  Output:         none
  Return:         STATUS_SUCCESS / STATUS_ERROR
  Others:         Reuses existing gSensorHal I2C configuration
*************************************************/
static Hal_StatusType I2C0_Init(void)
{
#ifdef USE_SW_I2C
    /* Configure GPIO pins for software I2C */
    Gpio_Hal_SetMuxMode(IMU_SDA_PORT, IMU_SDA_PIN, IMU_SDA_PIN_MUX);
    Gpio_Hal_SetPullMode(IMU_SDA_PORT, IMU_SDA_PIN, PORT_INTERNAL_PULL_UP_ENABLED);
    Gpio_Hal_SetPinDirection(IMU_SDA_PORT, IMU_SDA_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(IMU_SDA_PORT, IMU_SDA_PIN, 0);

    Gpio_Hal_SetMuxMode(IMU_SCL_PORT, IMU_SCL_PIN, IMU_SCL_PIN_MUX);
    Gpio_Hal_SetPullMode(IMU_SCL_PORT, IMU_SCL_PIN, PORT_INTERNAL_PULL_UP_ENABLED);
    Gpio_Hal_SetPinDirection(IMU_SCL_PORT, IMU_SCL_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(IMU_SCL_PORT, IMU_SCL_PIN, 0);

    return STATUS_SUCCESS;
#else
    Hal_StatusType status = STATUS_SUCCESS;

    /* Set the port pins multiplexer to GPIO first (to release I2C bus) */
    Gpio_Hal_SetMuxMode(IMU_SDA_PORT, IMU_SDA_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetMuxMode(IMU_SCL_PORT, IMU_SCL_PIN, PORT_MUX_AS_GPIO);

    /* Select function clock source: I2C0使用HSE外部晶振 */
    status = Ckgen_Hal_SetPeriphClkMux(CKGEN_I2C0_CLK, CKGEN_HSE_CLK);
    if (STATUS_ERROR == status) {
        TBOX_PRINT("Ckgen_Hal_SetPeriphClkMux fail\r\n");
    } else {
        /* Set I2C0 clock divider */
        Ckgen_Hal_SetPeriphClkDiv(CKGEN_I2C0_CLK, 8);

        /* Enable the I2C0 bus clock (includes reset and enable) */
        Ckgen_Hal_EnablePeriphClk(CKGEN_I2C0_BUS_CLK, TRUE);

        /* Init I2C via HAL */
        I2c_Hal_Init(IMU_I2C_INDEX, &g_I2cConfig);

        /* Enable I2C0 IRQ in NVIC */
        NVIC_EnableIRQ(IMU_I2C_IRQN);
    }

    /* Set the port pins multiplexer to the I2C0 mode */
    Gpio_Hal_SetMuxMode(IMU_SDA_PORT, IMU_SDA_PIN, IMU_SDA_PIN_MUX);
    Gpio_Hal_SetMuxMode(IMU_SCL_PORT, IMU_SCL_PIN, IMU_SCL_PIN_MUX);

    return status;
#endif
}

/*************************************************
  Function:       Qmi8a01zWriteReg
  Description:    Write a single byte to QMI8A01z register
  Input:          reg : Register address
                  value : Value to write
  Output:         none
  Return:         0 : success
                  negative : failure
  Others:         Uses I2C hardware with SyncTransceive
*************************************************/
static int16_t Qmi8a01zWriteReg(uint8_t reg, uint8_t value)
{
#ifdef USE_SW_I2C
    iic_start();
    iic_send_byte(QMI8A01Z_I2C_ADDR);  // Send device address (write)
    if (iic_wait_ack() != 0) {
        iic_stop();
        return QMI8A01Z_ERR_COMM;
    }
    
    iic_send_byte(reg);               // Send register address
    if (iic_wait_ack() != 0) {
        iic_stop();
        return QMI8A01Z_ERR_COMM;
    }
    
    iic_send_byte(value);             // Send data
    if (iic_wait_ack() != 0) {
        iic_stop();
        return QMI8A01Z_ERR_COMM;
    }
    
    iic_stop();
    return QMI8A01Z_OK;
#else
    /* Hardware I2C: send register address + data as a single write transaction */
    uint8_t dataBuf[2];
    dataBuf[0] = reg;
    dataBuf[1] = value;

    DataTransmitType transmitCfg = {
        .SlaveAddress = QMI8A01Z_I2C_SLAVE_ADDR_7BIT,
        .DirType = I2C_WRITE,
        .SendStop = TRUE,
        .DataBufferPtr = dataBuf,
        .DataLength = 2,
        .Is10bitAddr = FALSE,
    };

    Hal_StatusType status = I2c_Hal_SyncTransceive(IMU_I2C_INDEX, &transmitCfg);
    if (status != STATUS_SUCCESS) {
        return QMI8A01Z_ERR_COMM;
    }

    return QMI8A01Z_OK;
#endif
}


/*************************************************
  Function:       Qmi8a01zReadReg
  Description:    Read a single byte from QMI8A01z register
  Input:          reg : Register address
                  value : Pointer to store read value
  Output:         none
  Return:         0 : success
                  negative : failure
  Others:         Uses I2C hardware with SyncTransceive (write reg addr + repeated start + read)
*************************************************/
static int16_t Qmi8a01zReadReg(uint8_t reg, uint8_t *value)
{
#ifdef USE_SW_I2C
    if (value == NULL) {
        return QMI8A01Z_ERR_COMM;
    }

    iic_start();
    iic_send_byte(QMI8A01Z_I2C_ADDR);  // Send device address (write)
    if (iic_wait_ack() != 0) {
        iic_stop();
        return QMI8A01Z_ERR_COMM;
    }
    
    iic_send_byte(reg);               // Send register address
    if (iic_wait_ack() != 0) {
        iic_stop();
        return QMI8A01Z_ERR_COMM;
    }
    
    iic_start();                      // Repeated start
    iic_send_byte(QMI8A01Z_I2C_ADDR | 0x01);  // Send device address (read)
    if (iic_wait_ack() != 0) {
        iic_stop();
        return QMI8A01Z_ERR_COMM;
    }
    
    *value = iic_read_byte(0);        // Read byte with NACK
    iic_stop();
    
    return QMI8A01Z_OK;
#else
    /* Hardware I2C: write register address (no STOP), then read data */
    Hal_StatusType status;

    if (value == NULL) {
        return QMI8A01Z_ERR_COMM;
    }

    /* Step 1: Send register address (no STOP, to allow repeated START) */
    DataTransmitType writeCfg = {
        .SlaveAddress = QMI8A01Z_I2C_SLAVE_ADDR_7BIT,
        .DirType = I2C_WRITE,
        .SendStop = FALSE,              // 不发送STOP，准备重复START
        .DataBufferPtr = &reg,
        .DataLength = 1,
        .Is10bitAddr = FALSE,
    };

    status = I2c_Hal_SyncTransceive(IMU_I2C_INDEX, &writeCfg);
    if (status != STATUS_SUCCESS) {
        return QMI8A01Z_ERR_COMM;
    }

    /* Step 2: Read data */
    DataTransmitType readCfg = {
        .SlaveAddress = QMI8A01Z_I2C_SLAVE_ADDR_7BIT,
        .DirType = I2C_READ,
        .SendStop = TRUE,
        .DataBufferPtr = value,
        .DataLength = 1,
        .Is10bitAddr = FALSE,
    };

    status = I2c_Hal_SyncTransceive(IMU_I2C_INDEX, &readCfg);
    if (status != STATUS_SUCCESS) {
        return QMI8A01Z_ERR_COMM;
    }

    return QMI8A01Z_OK;
#endif
}

/*************************************************
  Function:       Qmi8a01zReadMultiReg
  Description:    Read multiple bytes from QMI8A01z registers
  Input:          reg : Starting register address
                  buffer : Buffer to store read data
                  length : Number of bytes to read
  Output:         none
  Return:         0 : success
                  negative : failure
  Others:         Uses I2C hardware with SyncTransceive (auto-increment read)
*************************************************/
static int16_t Qmi8a01zReadMultiReg(uint8_t reg, uint8_t *buffer, uint8_t length)
{
#ifdef USE_SW_I2C
    uint8_t i;

    if (buffer == NULL || length == 0) {
        return QMI8A01Z_ERR_COMM;
    }

    /* Send register address first */
    iic_start();
    iic_send_byte(QMI8A01Z_I2C_ADDR);  // Send device address (write)
    if (iic_wait_ack() != 0) {
        iic_stop();
        return QMI8A01Z_ERR_COMM;
    }
    
    iic_send_byte(reg);               // Send register address
    if (iic_wait_ack() != 0) {
        iic_stop();
        return QMI8A01Z_ERR_COMM;
    }
    
    /* Repeated start for read */
    iic_start();
    iic_send_byte(QMI8A01Z_I2C_ADDR | 0x01);  // Send device address (read)
    if (iic_wait_ack() != 0) {
        iic_stop();
        return QMI8A01Z_ERR_COMM;
    }
    
    /* Read multiple bytes */
    for (i = 0; i < length; i++) {
        if (i == (length - 1)) {
            buffer[i] = iic_read_byte(0);  // Last byte with NACK
        } else {
            buffer[i] = iic_read_byte(1);  // Other bytes with ACK
        }
    }
    
    iic_stop();
    return QMI8A01Z_OK;
#else
    /* Hardware I2C: write register address (no STOP), then read multiple bytes */
    Hal_StatusType status;

    if (buffer == NULL || length == 0) {
        return QMI8A01Z_ERR_COMM;
    }

    /* Step 1: Send register address (no STOP) */
    DataTransmitType writeCfg = {
        .SlaveAddress = QMI8A01Z_I2C_SLAVE_ADDR_7BIT,
        .DirType = I2C_WRITE,
        .SendStop = FALSE,
        .DataBufferPtr = &reg,
        .DataLength = 1,
        .Is10bitAddr = FALSE,
    };

    status = I2c_Hal_SyncTransceive(IMU_I2C_INDEX, &writeCfg);
    if (status != STATUS_SUCCESS) {
        return QMI8A01Z_ERR_COMM;
    }

    /* Step 2: Read multiple bytes (auto-increment) */
    DataTransmitType readCfg = {
        .SlaveAddress = QMI8A01Z_I2C_SLAVE_ADDR_7BIT,
        .DirType = I2C_READ,
        .SendStop = TRUE,
        .DataBufferPtr = buffer,
        .DataLength = length,
        .Is10bitAddr = FALSE,
    };

    status = I2c_Hal_SyncTransceive(IMU_I2C_INDEX, &readCfg);
    if (status != STATUS_SUCCESS) {
        return QMI8A01Z_ERR_COMM;
    }

    return QMI8A01Z_OK;
#endif
}

#ifdef USE_SW_I2C
/*************************************************
  Function:       iic_delay
  Description:    Software I2C timing delay
  Input:          none
  Output:         none
  Return:         none
  Others:         Provides timing for ~238kHz I2C clock
*************************************************/
static void iic_delay(void)
{
    int n = I2C_DELAY_COUNT;
    while (n--) {
        /* Simple delay loop */
    }
}

/*************************************************
  Function:       iic_start
  Description:    Generate I2C start condition
  Input:          none
  Output:         none
  Return:         none
  Others:         SDA transitions while SCL is high
*************************************************/
static void iic_start(void)
{
    iic_sda_out();
    iic_sda_1();
    iic_scl_1();
    iic_delay();
    iic_sda_0();  // START: when CLK is high, DATA changes from high to low
    iic_delay();
    iic_scl_0();  // Clamp I2C bus, prepare to send or receive data
    iic_delay();
}

/*************************************************
  Function:       iic_stop
  Description:    Generate I2C stop condition
  Input:          none
  Output:         none
  Return:         none
  Others:         SDA transitions while SCL is high
*************************************************/
static void iic_stop(void)
{
    iic_sda_out();
    iic_scl_0();
    iic_sda_0();  // STOP: when CLK is high, DATA changes from low to high
    iic_delay();
    iic_scl_1();
    iic_sda_1();  // Send I2C bus end signal
    iic_delay();
}

/*************************************************
  Function:       iic_wait_ack
  Description:    Wait for I2C acknowledge
  Input:          none
  Output:         none
  Return:         0 : ACK received
                   1 : NACK or timeout
  Others:         Checks SDA line for slave response
*************************************************/
static uint8_t iic_wait_ack(void)
{
    uint8_t ucErrTime = 0;

    iic_sda_in();
    iic_sda_1();
    iic_delay();
    iic_scl_1();
    iic_delay();
    
    while (iic_sda_read()) {
        ucErrTime++;
        if (ucErrTime > 250) {
            iic_stop();
            return 1;
        }
    }
    
    iic_scl_0();  // Clock output 0
    return 0;
}

/*************************************************
  Function:       iic_ack
  Description:    Generate I2C acknowledge
  Input:          none
  Output:         none
  Return:         none
  Others:         Pulls SDA low during clock pulse
*************************************************/
static void iic_ack(void)
{
    iic_scl_0();
    iic_sda_out();
    iic_sda_0();
    iic_delay();
    iic_scl_1();
    iic_delay();
    iic_scl_0();
    iic_delay();
}

/*************************************************
  Function:       iic_no_ack
  Description:    Generate I2C no acknowledge
  Input:          none
  Output:         none
  Return:         none
  Others:         Keeps SDA high during clock pulse
*************************************************/
static void iic_no_ack(void)
{
    iic_scl_0();
    iic_sda_out();
    iic_sda_1();
    iic_delay();
    iic_scl_1();
    iic_delay();
    iic_scl_0();
    iic_delay();
}

/*************************************************
  Function:       iic_send_byte
  Description:    Send a byte via I2C
  Input:          byte : Data byte to send
  Output:         none
  Return:         none
  Others:         Sends MSB first with proper timing
*************************************************/
static void iic_send_byte(uint8_t byte)
{
    uint8_t t;

    iic_sda_out();
    iic_scl_0();  // Pull clock low to start data transmission
    
    for (t = 0; t < 8; t++) {
        if ((byte & 0x80) >> 7) {
            iic_sda_1();
        } else {
            iic_sda_0();
        }
        byte <<= 1;
        iic_delay();
        iic_scl_1();
        iic_delay();
        iic_scl_0();
        iic_delay();
    }
}

/*************************************************
  Function:       iic_read_byte
  Description:    Read a byte via I2C
  Input:          ack : Whether to send ACK (1) or NACK (0)
  Output:         none
  Return:         Read byte value
  Others:         Reads MSB first with proper timing
*************************************************/
static uint8_t iic_read_byte(uint8_t ack)
{
    uint8_t i;
    uint8_t receive = 0;

    iic_sda_in();  // Set SDA as input
    
    for (i = 0; i < 8; i++) {
        iic_scl_0();
        iic_delay();
        iic_scl_1();
        receive <<= 1;
        if (iic_sda_read()) {
            receive++;
        }
        iic_delay();
    }
    
    if (ack == 1) {
        iic_ack();    // Send ACK
    } else {
        iic_no_ack();  // Send NACK
    }
    
    return receive;
}
#endif
