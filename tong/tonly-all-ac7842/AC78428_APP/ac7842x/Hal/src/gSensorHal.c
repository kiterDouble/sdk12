/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: gSensorHal.c
 Author:
 Created Time:
 Description:
 Others:
*************************************************/
/****************************** include ***************************************/
#include "gSensorHal.h"
#include "logHal.h"

#include "Gpio_Hal.h"
#include "I2c_Hal.h"
#include "Ckgen_Hal.h"
#include "Device_Types.h"

#include <stdarg.h>
#include <stdio.h>

/****************************** Macro Definitions ******************************/
#define IMU_I2C_INDEX       0
#define IMU_I2C_IRQN        I2C0_IRQn

/* IMU SDA Pin: PTA2 */
#define IMU_SDA_PORT        PORTID_A
#define IMU_SDA_PIN         2
#define IMU_SDA_PIN_MUX     PORT_MUX_ALT2

/* IMU SCL Pin: PTA3 */
#define IMU_SCL_PORT        PORTID_A
#define IMU_SCL_PIN         3
#define IMU_SCL_PIN_MUX     PORT_MUX_ALT2

/* IMU INT Pin: PTC5 */
#define IMU_IRQ_PORT        PORTID_C
#define IMU_IRQ_PIN         5
#define IMU_IRQ_PIN_MUX     PORT_MUX_AS_GPIO
#define IMU_IRQ_IRQN        PORTC_IRQn


// #define USE_SW_I2C

#ifdef USE_SW_I2C
    #define E_I2C_SLAVE_ADDR 0x6A
#else
    #define E_I2C_SLAVE_ADDR 0x6A
#endif

#define ACC_RATIO 16384

#ifdef USE_SW_I2C
    #define iic_scl_1() Gpio_Hal_WritePin(IMU_SCL_PORT, IMU_SCL_PIN, 1)
    #define iic_scl_0() Gpio_Hal_WritePin(IMU_SCL_PORT, IMU_SCL_PIN, 0)

    #define iic_sda_1() Gpio_Hal_WritePin(IMU_SDA_PORT, IMU_SDA_PIN, 1)
    #define iic_sda_0() Gpio_Hal_WritePin(IMU_SDA_PORT, IMU_SDA_PIN, 0)
    #define iic_sda_read() Gpio_Hal_ReadPin(IMU_SDA_PORT, IMU_SDA_PIN)
    #define iic_sda_in()  // {Gpio_Hal_SetPinDirection(IMU_SDA_PORT, IMU_SDA_PIN, GPIO_INPUT_DIRECTION);Gpio_Hal_SetPullMode(IMU_SDA_PORT, IMU_SDA_PIN, PORT_INTERNAL_PULL_NOT_ENABLED);}
    #define iic_sda_out() // {Gpio_Hal_SetPinDirection(IMU_SDA_PORT, IMU_SDA_PIN, GPIO_OUTPUT_DIRECTION);Gpio_Hal_SetPullMode(IMU_SDA_PORT, IMU_SDA_PIN, PORT_INTERNAL_PULL_UP_ENABLED);}
#endif

/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/
static uint8_t ReadBuffer[20]; // data received from EEPROM

volatile static uint8_t I2C_TxBuffer[11];  // 定义发送数据使用的数组
volatile static uint16_t I2C_RxBuffer[11]; // 定义接收数据使用的数组
volatile static uint32_t I2C_RxBufSN;      // 定义接收数组的当前序号
volatile static uint32_t I2C_CheckErrCnt;  // 防止I2C模块故障的计数变量

// I2C0模块的主机配置数据
static const I2c_Hal_MasterConfigType g_I2cMasterConfig = {
    .TxDmaChannel = 0,           // 不使用DMA
    .RxDmaChannel = 0,           // 不使用DMA
    .SdaPin = 0,                 // HW I2C不需要EIO引脚号
    .SclPin = 0,                 // HW I2C不需要EIO引脚号
    .ResourceIndex = 0,          // HW I2C不需要
    .SlaveAddress = 0x35,        // LSM6DSM 7-bit地址: 0x6A >> 1 = 0x35
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

// I2C0通道配置
static const I2c_Hal_ChannelConfigType g_I2cConfig = {
    .I2cType = HW_I2C,           // 硬件I2C
    .I2cMode = I2C_MASTER,       // 主机模式
    .Callback = NULL,            // 不使用回调
    .MasterConfigPtr = &g_I2cMasterConfig,
    .SlaveConfigPtr = NULL,
};

/****************************** Function Declarations *************************/
static void I2cStopDetIsr(void);       // 声明I2C_INT_STOP_DET/停止位信号产生 对应的中断处理函数
static void I2cStartDetIsr(void);      // 声明I2C_INT_START_DET/起始位信号产生 对应的中断处理函数
static void I2xRxFullIsr(void);        // 声明I2C_INT_RX_FULL/接收FIFO满 对应的中断处理函数
static void I2cRxOverIsr(void);        // 声明I2C_INT_RX_OVERL/接收FIFO溢出 对应的中断处理函数
static void I2cSclStuckAtLowIsr(void); // 声明I2C SCL持续低电平 中断函数

/****************************** Public Function Implementations ***************/
#ifdef USE_SW_I2C
static void iic_sleep(void)
{
    Gpio_Hal_SetMuxMode(IMU_SDA_PORT, IMU_SDA_PIN, PORT_MUX_AS_GPIO); // 配置  端口为GPIO功能
    Gpio_Hal_SetPullMode(IMU_SDA_PORT, IMU_SDA_PIN, PORT_INTERNAL_PULL_UP_ENABLED);    // 使能内部上拉
    Gpio_Hal_SetOpenDrain(IMU_SDA_PORT, IMU_SDA_PIN, TRUE);
    Gpio_Hal_SetPinDirection(IMU_SDA_PORT, IMU_SDA_PIN, GPIO_INPUT_DIRECTION);
    // Gpio_Hal_WritePin(IMU_SDA_PORT, IMU_SDA_PIN, 0);

    Gpio_Hal_SetMuxMode(IMU_SCL_PORT, IMU_SCL_PIN, PORT_MUX_AS_GPIO); // 配置  端口为GPIO功能
    Gpio_Hal_SetPullMode(IMU_SCL_PORT, IMU_SCL_PIN, PORT_INTERNAL_PULL_UP_ENABLED);    // 使能内部上拉
    Gpio_Hal_SetOpenDrain(IMU_SCL_PORT, IMU_SCL_PIN, TRUE);
    Gpio_Hal_SetPinDirection(IMU_SCL_PORT, IMU_SCL_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(IMU_SCL_PORT, IMU_SCL_PIN, 0);
}

static void iic_delay(void)
{
#if 1
    // int n = 30;    // 高电平时间1.72us，低电平时间1.46us - 对应时钟频率 314.47K
    // int n = 39;    // 对应时钟频率 238.1K
    int n = 23;
    while (n--)
        ;
#else
    vTaskDelay(5);
#endif
}

static void iic_start(void)
{
    iic_sda_out(); // SDA线输出
    iic_sda_1();
    iic_scl_1();
    iic_delay();
    iic_sda_0(); // START:when CLK is high,DATA change form high to low
    iic_delay();
    iic_scl_0(); // 钳住I2C总线，准备发送或接收数据
}

static void iic_stop(void)
{
    iic_sda_out(); // sda线输出
    iic_scl_0();
    iic_sda_0(); // STOP:when CLK is high DATA change form low to high
    iic_delay();
    iic_scl_1();
    iic_sda_1(); // 发送I2C总线结束信号
    iic_delay();
}

static uint8_t iic_wait_ack(void)
{
    uint8_t ucErrTime = 0;

    iic_sda_in(); // SDA设置为输入
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
    iic_scl_0(); // 时钟输出0
    return 0;
}

static void iic_ack(void)
{
    iic_scl_0();
    iic_sda_out();
    iic_sda_0();
    iic_delay();
    iic_scl_1();
    iic_delay();
    iic_scl_0();
}

static void iic_no_ack(void)
{
    iic_scl_0();
    iic_sda_out();
    iic_sda_1();
    iic_delay();
    iic_scl_1();
    iic_delay();
    iic_scl_0();
}

static void iic_send_byte(uint8_t byte)
{
    uint8_t t;

    iic_sda_out();
    iic_scl_0(); // 拉低时钟开始数据传输
    for (t = 0; t < 8; t++) {
        if ((byte & 0x80) >> 7)
            iic_sda_1();
        else
            iic_sda_0();
        byte <<= 1;
        iic_delay();
        iic_scl_1();
        iic_delay();
        iic_scl_0();
        iic_delay();
    }
}

static uint8_t iic_read_byte(uint8_t ack)
{
    unsigned char i, receive = 0;

    iic_sda_in(); // SDA设置为输入
    for (i = 0; i < 8; i++) {
        iic_scl_0();
        iic_delay();
        iic_scl_1();
        receive <<= 1;
        if (iic_sda_read())
            receive++;
        iic_delay();
    }
    iic_no_ack();
    if (!ack)
        iic_ack(); // 发送nACK
    else
        iic_no_ack(); // 发送ACK
    return receive;
}
#endif

/*****************************************************************************
 * 函数:I2C0_Init
 * 功能:完成对I2C0模块的配置，把I2C0配置成主机模式。
 * 参数:
 * 返回:
 * 说明:1，I2C模块中包含4个TX_FIFO和4个RX_FIFO。
 *        2，当I2C在发送时掉线、时钟信号被干扰，会造成I2C通讯故障，I2C主机、从机无法接收、发送，可以使
 *            用复位MCU解除故障，也可以使用智芯半导体提供的I2C初始化函数解除故障
 *        3，为了解决I2C在发送时掉线产生的I2C模块无法继续工作的问题，需要在设置I2C模块之前，先把I2C使用的
 *            IO设置成GPIO功能，在完成设置I2C模块之后，把I2C使用的IO设置为I2C功能。
 *        4，波特率估算，在标准速度模式下，I2C模块内部的时钟分频系数约等于400。
 ****************************************************************************/
static void I2C0_Init(void)
{
#ifdef USE_SW_I2C
    Gpio_Hal_SetMuxMode(IMU_SDA_PORT, IMU_SDA_PIN, IMU_SDA_PIN_MUX); // 配置  端口为GPIO功能
    Gpio_Hal_SetPullMode(IMU_SDA_PORT, IMU_SDA_PIN, PORT_INTERNAL_PULL_UP_ENABLED);    // 使能内部上拉
    Gpio_Hal_SetPinDirection(IMU_SDA_PORT, IMU_SDA_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(IMU_SDA_PORT, IMU_SDA_PIN, 0);

    Gpio_Hal_SetMuxMode(IMU_SCL_PORT, IMU_SCL_PIN, IMU_SCL_PIN_MUX);  // 配置  端口为GPIO功能
    Gpio_Hal_SetPullMode(IMU_SCL_PORT, IMU_SCL_PIN, PORT_INTERNAL_PULL_UP_ENABLED);    // 使能内部上拉
    Gpio_Hal_SetPinDirection(IMU_SCL_PORT, IMU_SCL_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(IMU_SCL_PORT, IMU_SCL_PIN, 0);
#else
    // 需要在设置I2C模块之前，先把I2C使用的IO设置成GPIO功能，断开总线与模块之间的信号连接
    Gpio_Hal_SetMuxMode(IMU_SDA_PORT, IMU_SDA_PIN, PORT_MUX_AS_GPIO); // 配置  端口为GPIO功能
    Gpio_Hal_SetMuxMode(IMU_SCL_PORT, IMU_SCL_PIN, PORT_MUX_AS_GPIO);  // 配置  端口为GPIO功能

    // 初始化时钟源：I2C模块的时钟源选择外部晶振(HSE)
    Ckgen_Hal_SetPeriphClkMux(CKGEN_I2C0_CLK, CKGEN_HSE_CLK);
    // 设置I2C时钟的分频器，模块的时钟不能高于CPU的总线时钟
    Ckgen_Hal_SetPeriphClkDiv(CKGEN_I2C0_CLK, 2);
    // 使能I2C0总线时钟（包含复位和使能）
    Ckgen_Hal_EnablePeriphClk(CKGEN_I2C0_BUS_CLK, TRUE);

    // 初始化I2C模块寄存器
    I2c_Hal_Init(IMU_I2C_INDEX, &g_I2cConfig);

    // 设置中断信息
    NVIC_SetPriority(IMU_I2C_IRQN, 0x3); // 设置 I2C_IRQn 的中断优先级。(高)0--15(低)
    // NVIC_EnableIRQ(IMU_I2C_IRQN);                    //使能I2C_IRQn 中断
    NVIC_DisableIRQ(IMU_I2C_IRQN); // 禁止 I2C1_IRQn 中断

    // I2C_InstallCallBackFunc 映射说明：
    // AC7842x使用 I2c_Hal_ChannelConfigType.Callback 统一回调，
    // 在回调中通过 Event 参数判断事件类型 (I2C_MASTER_EVENT_END_TRANSFER 等)
    // 当前所有中断均被禁用，故回调保持为NULL

    // 初始化全局变量
    I2C_TxBuffer[0] = 0x00;
    I2C_TxBuffer[1] = 0x01;
    I2C_TxBuffer[2] = 0x02;
    I2C_TxBuffer[3] = 0x03;
    I2C_TxBuffer[4] = 0x04;
    I2C_TxBuffer[5] = 0x05;
    I2C_TxBuffer[6] = 0x06;
    I2C_TxBuffer[7] = 0x07;
    I2C_TxBuffer[8] = 0x08;
    I2C_TxBuffer[9] = 0x09;
    I2C_TxBuffer[10] = 0x0A;

    I2C_RxBuffer[0] = 0;
    I2C_RxBuffer[1] = 0;
    I2C_RxBuffer[2] = 0;
    I2C_RxBuffer[3] = 0;
    I2C_RxBuffer[4] = 0;
    I2C_RxBuffer[5] = 0;
    I2C_RxBuffer[6] = 0;
    I2C_RxBuffer[7] = 0;
    I2C_RxBuffer[8] = 0;
    I2C_RxBuffer[9] = 0;
    I2C_RxBuffer[10] = 0;
    I2C_RxBufSN = 0;
    I2C_CheckErrCnt = 0; // 防止I2C模块故障的计数变量

    // 需要在设置I2C模块后，把I2C使用的IO设置成I2C功能
    Gpio_Hal_SetMuxMode(IMU_SCL_PORT, IMU_SCL_PIN, IMU_SCL_PIN_MUX); // 设置为I2C_SCL功能
    Gpio_Hal_SetMuxMode(IMU_SDA_PORT, IMU_SDA_PIN, IMU_SDA_PIN_MUX); // 设置为I2C_SDA功能
    Gpio_Hal_SetPullMode(IMU_SDA_PORT, IMU_SDA_PIN, PORT_INTERNAL_PULL_UP_ENABLED);   // 使能内部上拉
    Gpio_Hal_SetPullMode(IMU_SCL_PORT, IMU_SCL_PIN, PORT_INTERNAL_PULL_UP_ENABLED);   // 使能内部上拉
#endif
}

/*****************************************************************************
 * 函数:I2C_MasterSendData
 * 功能:把需要发送的数据（包含寄存器地址+数据），通过I2C同步发送出去。
 * 参数:regAddr--目标寄存器地址；txBuffer--存放需要发送的数据；len--需要发送的数据的长度
 * 返回:STATUS_SUCCESS--发送成功；STATUS_ERROR--发送失败
 * 说明:AC7842x使用 I2c_Hal_SyncTransceive 替代 Z20K148MS 的 I2c_SetTargetAddr + I2c_MasterSendByte 轮询模式
 ****************************************************************************/
static Hal_StatusType I2C_MasterSendData(uint8_t regAddr, uint8_t *txBuffer, uint16_t len)
{
    Hal_StatusType status;
    uint8_t sendBuf[12];  // 最多: 1个寄存器地址 + 11个数据字节
    uint16_t i;

    if (len > 11) {
        return STATUS_ERROR;
    }

    sendBuf[0] = regAddr;
    for (i = 0; i < len; i++) {
        sendBuf[1 + i] = txBuffer[i];
    }

    DataTransmitType transmitCfg = {
        .SlaveAddress = (E_I2C_SLAVE_ADDR >> 1),  // 7-bit slave地址
        .DirType = I2C_WRITE,
        .SendStop = TRUE,
        .DataBufferPtr = sendBuf,
        .DataLength = (uint32)(len + 1),
        .Is10bitAddr = FALSE,
    };

    status = I2c_Hal_SyncTransceive(IMU_I2C_INDEX, &transmitCfg);
    return status;
}

/*****************************************************************************
 * 函数:I2C_MasterReceiveData
 * 功能:从指定寄存器地址读取数据。
 * 参数:regAddr--目标寄存器地址；rxBuffer--存放读取到的数据；len--需要读取的数据的长度
 * 返回:STATUS_SUCCESS--读取成功；STATUS_ERROR--读取失败
 * 说明:先发送寄存器地址（无STOP），再以重复START条件读取数据
 ****************************************************************************/
static Hal_StatusType I2C_MasterReceiveData(uint8_t regAddr, uint8_t *rxBuffer, uint16_t len)
{
    Hal_StatusType status;

    // Step 1: 发送寄存器地址（不发送STOP，以便后续重复START）
    DataTransmitType writeCfg = {
        .SlaveAddress = (E_I2C_SLAVE_ADDR >> 1),  // 7-bit slave地址
        .DirType = I2C_WRITE,
        .SendStop = FALSE,                         // 不发送STOP，准备重复START
        .DataBufferPtr = &regAddr,
        .DataLength = 1,
        .Is10bitAddr = FALSE,
    };

    status = I2c_Hal_SyncTransceive(IMU_I2C_INDEX, &writeCfg);
    if (status != STATUS_SUCCESS) {
        return STATUS_ERROR;
    }

    // Step 2: 读取数据
    DataTransmitType readCfg = {
        .SlaveAddress = (E_I2C_SLAVE_ADDR >> 1),  // 7-bit slave地址
        .DirType = I2C_READ,
        .SendStop = TRUE,
        .DataBufferPtr = rxBuffer,
        .DataLength = len,
        .Is10bitAddr = FALSE,
    };

    status = I2c_Hal_SyncTransceive(IMU_I2C_INDEX, &readCfg);
    return status;
}

/*****************************************************************************
 * 函数:I2cStopDetIsr
 * 功能: I2C_INT_STOP_DET/停止位信号产生 对应的中断处理函数
 * 参数:
 * 返回:
 * 说明:1，当前该中断已被禁用，保留ISR定义供后续使用
 ****************************************************************************/
void I2cStopDetIsr(void)
{
    // AC7842x: 清除中断标志由 HAL 内部处理，通过 I2c_Hal_ChannelConfigType.Callback 分发事件
    // Event = I2C_MASTER_EVENT_END_TRANSFER 表示传输完成
    I2C_CheckErrCnt = 0;  // 防止I2C模块故障的计数变量

    // 以下添加其他处理代码
}

/*****************************************************************************
 * 函数:I2cStartDetIsr
 * 功能: I2C_INT_START_DET/起始位信号产生 对应的中断处理函数
 * 参数:
 * 返回:
 * 说明:1，当前该中断已被禁用
 ****************************************************************************/
void I2cStartDetIsr(void)
{
    // AC7842x: 清除中断标志由 HAL 内部处理
    I2C_CheckErrCnt = 0;  // 防止I2C模块故障的计数变量

    // 以下添加其他处理代码
}

/*****************************************************************************
 * 函数:I2C1_Rx_Full_ISR
 * 功能: I2C_INT_RX_FULL 对应的中断处理函数
 * 参数:
 * 返回:
 * 说明:1，在使用接收中断时，建议把RX_FIFO门槛值设置为0，每次收到一个数据，进一次中断，可以有效避免
 *            接收数据的顺序产生混乱。
 *        2，当前该中断已被禁用
 ****************************************************************************/
void I2xRxFullIsr(void)
{
    // AC7842x: 接收由 I2c_Hal_SyncTransceive/I2c_Hal_AsyncTransceive 内部管理
    I2C_CheckErrCnt = 0;  // 防止I2C模块故障的计数变量

    // 以下添加其他处理代码
}

/*****************************************************************************
 * 函数:I2C1_Rx_OVER_ISR
 * 功能: I2C_INT_RX_OVER 对应的中断处理函数
 * 参数:
 * 返回:
 * 说明: 当前该中断已被禁用
 ****************************************************************************/
void I2cRxOverIsr(void)
{
    // AC7842x: 溢出由 HAL 内部处理
    I2C_CheckErrCnt = 0;  // 防止I2C模块故障的计数变量

    // 以下添加其他处理代码
}

/*****************************************************************************
 * 函数:I2C1_SCL_STUCK_AT_LOW_ISR
 * 功能: 声明I2C SCL持续低电平 中断处理函数
 * 参数:
 * 返回:
 * 说明: 当前该中断已被禁用
 ****************************************************************************/
void I2cSclStuckAtLowIsr(void)
{
    I2C0_Init(); // 重新初始化I2C模块
}

#if 0
static cy_en_scb_i2c_status_t I2cStart(uint8_t address)
{ 
    cy_en_scb_i2c_status_t  result_i2c;
    result_i2c = Cy_SCB_I2C_MasterSendStart(IMU_I2C_SYSCTRL, address, CY_SCB_I2C_WRITE_XFER, 50, &g_stc_i2c_context);
    return result_i2c;
}   

static cy_en_scb_i2c_status_t I2cStop(void)
{   
    cy_en_scb_i2c_status_t  result_i2c;
    result_i2c = Cy_SCB_I2C_MasterSendStop(IMU_I2C_SYSCTRL, TIMEOUT, &g_stc_i2c_context); 
    return result_i2c;
}  
  
//static cy_en_scb_i2c_status_t I2cWriteStop(void)
//{   
//    cy_en_scb_i2c_status_t  result_i2c;
//    result_i2c = Cy_SCB_I2C_MasterSendStop(IMU_I2C_SYSCTRL, TIMEOUT, &g_stc_i2c_context); 
//    return result_i2c;
//}   

static cy_en_scb_i2c_status_t I2CWriteByte(uint8_t cValue)
{
    // Transmite One Byte Data 
    cy_en_scb_i2c_status_t result_i2c;
    result_i2c = Cy_SCB_I2C_MasterWriteByte(IMU_I2C_SYSCTRL, cValue, TIMEOUT, &g_stc_i2c_context);
    return  result_i2c;
}

static cy_en_scb_i2c_status_t I2c_Continue(uint8_t address)
{
    cy_en_scb_i2c_status_t result_i2c;
    result_i2c = Cy_SCB_I2C_MasterSendReStart(IMU_I2C_SYSCTRL,address, CY_SCB_I2C_READ_XFER, TIMEOUT, &g_stc_i2c_context);
    return  result_i2c; 
}
#endif

static int32_t Lsm6dsmWriteoneByte(uint8_t address, uint8_t cValue)
{
#ifdef USE_SW_I2C
    iic_start();
    iic_send_byte(E_I2C_SLAVE_ADDR); // 发送IIC地址
    iic_wait_ack();
    iic_send_byte(address); // 发送地址
    iic_wait_ack();
    iic_send_byte(cValue); // 发送字节
    iic_wait_ack();
    iic_stop(); // 产生一个停止条件
    return 0;
#else
    return (I2C_MasterSendData(address, &cValue, 1) == STATUS_SUCCESS) ? 0 : 1;
#endif
}
#if 0
//static int32_t Lsm6dsmWriteByte(uint8_t address, uint8_t *buffer, uint8_t Length)
//{
//  int i;
//  if(I2cStart(E_I2C_SLAVE_ADDR) != CY_SCB_I2C_SUCCESS)    return 1;
//  if(I2CWriteByte(address) != CY_SCB_I2C_SUCCESS)    return 1;
//  for(i = 0; i < Length ; i++)
//  {
//      if(I2CWriteByte(*buffer++) != CY_SCB_I2C_SUCCESS) return 1;
//      DelayUs(2000); 
//  }
//  if(I2cStop() != CY_SCB_I2C_SUCCESS)    return 1;
//  return 0; 
//}

//static cy_en_scb_i2c_status_t I2c_Read( uint8_t *Rxbyte)
//{
//    cy_en_scb_i2c_status_t result_i2c;
//
//    result_i2c = Cy_SCB_I2C_MasterReadByte(IMU_I2C_SYSCTRL, CY_SCB_I2C_ACK, Rxbyte, TIMEOUT, &g_stc_i2c_context);
//    return result_i2c; 
//} 
static cy_en_scb_i2c_status_t I2c_ReadLast(uint8_t *Rxbyte)  
{
    cy_en_scb_i2c_status_t result_i2c;
    result_i2c = Cy_SCB_I2C_MasterReadByte(IMU_I2C_SYSCTRL, CY_SCB_I2C_NAK, Rxbyte, TIMEOUT, &g_stc_i2c_context);
    return result_i2c;                       
}
#endif

static int16_t Lsm6dsmReadOneByte(uint8_t address, uint8_t *buffer)
{
#ifdef USE_SW_I2C
    iic_start();
    iic_send_byte(E_I2C_SLAVE_ADDR); // 发送IIC地址
    iic_wait_ack();
    iic_send_byte(address); // 发送地址
    iic_wait_ack();
    iic_start();
    iic_send_byte(E_I2C_SLAVE_ADDR + 1); // 进入接收模式
    iic_wait_ack();
    *buffer = iic_read_byte(0);
    iic_stop(); // 产生一个停止条件
    return 0;
#else
    return (I2C_MasterReceiveData(address, buffer, 1) == STATUS_SUCCESS) ? 0 : 1;
#endif
}

static uint8_t Lsm6dsmWakeupFlag = 0;
static void Lsm6dsmInit(uint8_t flag)
{
    uint16_t timerCounter = 10000;
    Lsm6dsmReadOneByte(LSM6DSM_WHO_AM_I, ReadBuffer);
    while ((ReadBuffer[0] != 0x6B) && (timerCounter)) {
        timerCounter--;
        if (timerCounter < 5000) {
            Lsm6dsmReadOneByte(LSM6DSM_WHO_AM_I, ReadBuffer);
        }
        // RTOS_HalApiWait(50);
    }

    if (timerCounter == 0) {
        INFO_PRINT("Lsm6dsm init error,reg = %02x\r\n", Lsm6dsmReadOneByte(LSM6DSM_WHO_AM_I, ReadBuffer));
        return;
    }

    /*set ODR 416 Hz   full-scale ±2g*/
    Lsm6dsmWriteoneByte(LSM6DSM_CTRL1_XL, 0x60);
    // Lsm6dsmReadOneByte(LSM6DSM_CTRL1_XL,ReadBuffer);
    Lsm6dsmWriteoneByte(LSM6DSM_TAP_CFG, 0x00);
    if (flag == 1) {
        Lsm6dsmWakeupFlag = 1;
    } else {
        Lsm6dsmWakeupFlag = 0;
    }
}
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
int16_t GSensorHalGetWakeupFlag(void)
{
    return Lsm6dsmWakeupFlag;
}
/*************************************************
  Function:       GSensorHalInit
  Description:    Gsensor module init
  Input:          flag : Whether to set interrupt wake up
  Output:         none
  Return:         none
  Others:         none
*************************************************/
void GSensorHalInit(uint8_t flag)
{
    I2C0_Init();

    Lsm6dsmInit(flag);
}

/*************************************************
  Function:       GSensorHalGetValue
  Description:    Get Gsensor data
  Input:          none
  Output:         GSensordata : The obtained data address,uint g/1000
  Return:         0 : get success
                  1 : get failed
  Others:         none
*************************************************/
int16_t GSensorHalGetValue(GSensorHalData_t *GSensordata)
{
#if (1)
    uint8_t buf[6];
    short tem;
    double temp;
    Lsm6dsmReadOneByte(LSM6DSM_STATUS_REG, ReadBuffer);
    if ((ReadBuffer[0] & 0x01) != 0) {
        Lsm6dsmReadOneByte(LSM6DSM_OUTX_H_XL, ReadBuffer);
        buf[0] = ReadBuffer[0];
        Lsm6dsmReadOneByte(LSM6DSM_OUTX_L_XL, ReadBuffer);
        buf[1] = ReadBuffer[0];

        Lsm6dsmReadOneByte(LSM6DSM_OUTY_H_XL, ReadBuffer);
        buf[2] = ReadBuffer[0];
        Lsm6dsmReadOneByte(LSM6DSM_OUTY_L_XL, ReadBuffer);
        buf[3] = ReadBuffer[0];

        Lsm6dsmReadOneByte(LSM6DSM_OUTZ_H_XL, ReadBuffer);
        buf[4] = ReadBuffer[0];
        Lsm6dsmReadOneByte(LSM6DSM_OUTZ_L_XL, ReadBuffer);
        buf[5] = ReadBuffer[0];

        tem = (buf[0] << 8) | buf[1];
        temp = ((double)tem) / ACC_RATIO;
        GSensordata->accel_x = ((short)(temp * 1000));
        tem = (buf[2] << 8) | buf[3];
        temp = ((double)tem) / ACC_RATIO;
        GSensordata->accel_y = ((short)(temp * 1000));
        tem = (buf[4] << 8) | buf[5];
        temp = ((double)tem) / ACC_RATIO;
        GSensordata->accel_z = ((short)(temp * 1000));

        //        GSensordata->accel_x = (buf[0]<<8)|buf[1];
        //        GSensordata->accel_y = (buf[2]<<8)|buf[3];
        //        GSensordata->accel_z = (buf[4]<<8)|buf[5];
        return 0;
    }
#endif
    return 1;
}

/*************************************************
  Function:       GSensorHalSetThreshold
  Description:    Set Gsensor interrupt threshold
  Input:          Threshold : interrupt threshold,The unit is determined by the set Accelerometer full-scale selection,
                              The default setting is 2g.
  Output:         none
  Return:         0 : set success
                  1 : set failed
  Others:         The wake function is implemented using a high-pass digital filter
*************************************************/
int16_t GSensorHalSetThreshold(uint32_t Threshold)
{
    // enable basic interrupts
    if (Lsm6dsmWriteoneByte(LSM6DSM_TAP_CFG, 0x90))
        return 1;
    /*Set interrupt pin low active*/
    // if(Lsm6dsmWriteoneByte(LSM6DSM_CTRL3_C,0x60))         return 1;
    /*Set the minimum duration of the wake event to 0x00/ODR_XL*/
    if (Lsm6dsmWriteoneByte(LSM6DSM_WAKE_UP_DUR, 0x00))
        return 1;
    /*Set Interrupt wake up threshold is 2g/2^6*Threshold*/
    if (Lsm6dsmWriteoneByte(LSM6DSM_WAKE_UP_THS, Threshold))
        return 1;
    /*Wake interrupt drive to INT1 pin*/
    if (Lsm6dsmWriteoneByte(LSM6DSM_MD1_CFG, 0x20))
        return 1;
    return 0;
}

void GSensorHalSetWakeup(void)
{
    Lsm6dsmWriteoneByte(LSM6DSM_TAP_CFG, 0x00);
}
