/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: ecallHal.c
 Author: 
 Created Time: 
 Description: 
 Others: 
*************************************************/
/****************************** include ***************************************/
#include "ecallHal.h"
#include "logHal.h"
#include "peripheralHal.h"
#include "Gpio_Hal.h"
#include "I2c_Hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"
#include "Core_Hal.h"

/****************************** Macro Definitions ******************************/
#define DEBUG_PRINT_ENABLE              (1U)
#define SOS_LED_QUEUE_SEND_WAIT_TIME    (1000U)

/* I2C Addresses */
#define SA51034_ADDRESS             (0xD8 >> 1)

/* Register Maps */
#define FAULT_REGISTER              (0x01)
#define DIAGNOSTIC_REGISTER         (0x02)
#define CONTROL_REGISTER            (0x03)

#define USER_I2C_SCB_INDEX          I2C_INSTANCE_ID 

#define I2C_SDA_PORT                PORTID_A
#define I2C_SDA_PORT_PIN            2
#define I2C_SDA_PORT_MUX            PORT_MUX_ALT3

#define I2C_SCL_PORT                PORTID_A
#define I2C_SCL_PORT_PIN            3
#define I2C_SCL_PORT_MUX            PORT_MUX_ALT3

/****************************** Type Definitions ******************************/
typedef struct
{
    EcallLedMode_e ledMode;         /*0:关闭，1:打开，2:闪烁*/
    uint16_t ledCounter;            /*0:关闭，1:打开，2:闪烁*/
    uint16_t ledOnTime;             /*LED打开的时间，单位ms*/
    uint16_t ledOffTime;            /*LED关闭的时间，单位ms*/
}EcallLedSwitchTime_t;

/****************************** Global Variables ******************************/
static EcallLedSwitchTime_t g_SosLedSwitchTime = {E_ECALL_LED_MODE_KEEP_OFF, 0, 0, 0};
static uint32_t g_ampInitTime = 0u;
static uint8_t g_ampInitFlag = 0u;
QueueHandle_t g_SosLedMsgQueue = NULL;
SemaphoreHandle_t  g_SosLedMutex = NULL;

static const I2c_Hal_MasterConfigType g_stcI2cMasterConfig =
{
    .TxDmaChannel = 0,
    .RxDmaChannel = 0,
    .SdaPin = I2C_SDA_PORT_PIN, 
    .SclPin = I2C_SCL_PORT_PIN, 
    .ResourceIndex = 0,
    .SlaveAddress = SA51034_ADDRESS,
    .PinLow = 0,
    .BaudRate = 100000,                  /* 100K 标准模式 */
    .SendStop = TRUE,
    .SclSdaLowEn = FALSE,
    .SyncEn = FALSE,
    .Arbitration = TRUE,
    .Is10bitAddr = FALSE,                /* 7位地址 */
    .TransferType = I2C_USING_INTERRUPTS,
    .TimeCfgTypes = I2C_SCL
};

static const I2c_Hal_ChannelConfigType g_stcI2cChannelConfig =
{
    .I2cType = HW_I2C,
    .I2cMode = I2C_MASTER,
    .Callback = NULL,
    .MasterConfigPtr = &g_stcI2cMasterConfig,
    .SlaveConfigPtr = NULL
};

/****************************** Function Declarations *************************/
void Sa51500Init(void);
void Sa51500Close(void);

/****************************** Public Function Implementations ***************/
void EcallGpioInit(void)
{
    /******** ECALL_LED_R (reserved) **************************/
    Gpio_Hal_SetMuxMode(ECALL_LED_R_PORT, ECALL_LED_R_PIN, ECALL_LED_R_PIN_MUX);
    Gpio_Hal_SetPinDirection(ECALL_LED_R_PORT, ECALL_LED_R_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(ECALL_LED_R_PORT, ECALL_LED_R_PIN, GPIO_LOW_LEVEL);

    /******** ECALL_LED_G ***************************/
    Gpio_Hal_SetMuxMode(ECALL_LED_G_PORT, ECALL_LED_G_PIN, ECALL_LED_G_PIN_MUX);
    Gpio_Hal_SetPinDirection(ECALL_LED_G_PORT, ECALL_LED_G_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(ECALL_LED_G_PORT, ECALL_LED_G_PIN, GPIO_LOW_LEVEL);

    /******** ECALL_PWR_EN ***************************/
    Gpio_Hal_SetMuxMode(ECALL_PWR_EN_PORT, ECALL_PWR_EN_PIN, ECALL_PWR_EN_PIN_MUX);
    Gpio_Hal_SetPinDirection(ECALL_PWR_EN_PORT, ECALL_PWR_EN_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(ECALL_PWR_EN_PORT, ECALL_PWR_EN_PIN, GPIO_HIGH_LEVEL);

    /******** ECALL_BUTTON ***************************/
    Gpio_Hal_SetMuxMode(ECALL_BUTTON_PORT, ECALL_BUTTON_PIN, ECALL_BUTTON_PIN_MUX);
    Gpio_Hal_SetPinDirection(ECALL_BUTTON_PORT, ECALL_BUTTON_PIN, GPIO_INPUT_DIRECTION);

    /******** vehicle mute (reserved) ***************************/
    Gpio_Hal_SetMuxMode(VEHICLE_MUTE_PORT, VEHICLE_MUTE_PIN, VEHICLE_MUTE_PIN_MUX);
    Gpio_Hal_SetPinDirection(VEHICLE_MUTE_PORT, VEHICLE_MUTE_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(VEHICLE_MUTE_PORT, VEHICLE_MUTE_PIN, GPIO_LOW_LEVEL);

    /******** CODEC_MIC_PWR_EN ***************************/
    Gpio_Hal_SetMuxMode(CODEC_MIC_PWR_EN_PORT, CODEC_MIC_PWR_EN_PIN, CODEC_MIC_PWR_EN_PIN_MUX);
    Gpio_Hal_SetPinDirection(CODEC_MIC_PWR_EN_PORT, CODEC_MIC_PWR_EN_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(CODEC_MIC_PWR_EN_PORT, CODEC_MIC_PWR_EN_PIN, GPIO_HIGH_LEVEL);

    /******** SRS_EN ***************************/
    Gpio_Hal_SetMuxMode(SRS_EN_PORT, SRS_EN_PIN, SRS_EN_PIN_MUX);
    Gpio_Hal_SetPinDirection(SRS_EN_PORT, SRS_EN_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(SRS_EN_PORT, SRS_EN_PIN, GPIO_HIGH_LEVEL);

    /******** SRS_STATE ***************************/
    Gpio_Hal_SetMuxMode(SRS_STATE_PORT, SRS_STATE_PIN, SRS_STATE_PIN_MUX);
    Gpio_Hal_SetPinDirection(SRS_STATE_PORT, SRS_STATE_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetPinIntSel(SRS_STATE_PORT, SRS_STATE_PIN, PORT_INT_EITHER_EDGE);
    
    Core_Hal_SetIrqPriority(SRS_STATE_PIN_IRQ, 0x3);
    Core_Hal_EnableIrq(SRS_STATE_PIN_IRQ);
}

void EcallHalSetSrsEn(uint8_t flag)
{
    if (0 == flag)
    {
        Gpio_Hal_SetPinIntSel(SRS_STATE_PORT, SRS_STATE_PIN, PORT_DMA_INT_DISABLED);
        Core_Hal_DisableIrq(SRS_STATE_PIN_IRQ);
        Gpio_Hal_WritePin(SRS_EN_PORT, SRS_EN_PIN, GPIO_LOW_LEVEL);
    }
    else
    {
        Gpio_Hal_SetPinIntSel(SRS_STATE_PORT, SRS_STATE_PIN, PORT_INT_EITHER_EDGE);
        Gpio_Hal_WritePin(SRS_EN_PORT, SRS_EN_PIN, GPIO_HIGH_LEVEL);
        Core_Hal_EnableIrq(SRS_STATE_PIN_IRQ);
    }
}

void EcallHalSetSosLedRedState(uint8_t state)
{
    Gpio_Hal_WritePin(ECALL_LED_R_PORT, ECALL_LED_R_PIN, state ? GPIO_HIGH_LEVEL : GPIO_LOW_LEVEL);
}

void EcallHalSetSosLedGreenState(uint8_t state)
{
    Gpio_Hal_WritePin(ECALL_LED_G_PORT, ECALL_LED_G_PIN, state ? GPIO_HIGH_LEVEL : GPIO_LOW_LEVEL);
}

void EcallHalSetAmpSdz(uint8_t flag)
{
    Gpio_Hal_WritePin(AMP_STB_PORT, AMP_STB_PIN, flag ? GPIO_HIGH_LEVEL : GPIO_LOW_LEVEL);
}

void EcallHalSetAmpMute(uint8_t flag)
{
    Gpio_Hal_WritePin(AMP_MUTE_PORT, AMP_MUTE_PIN, flag ? GPIO_HIGH_LEVEL : GPIO_LOW_LEVEL);
}

void EcallHalSetVehicleMute(uint8_t flag)
{
    if (0 == flag)
    {
        Gpio_Hal_WritePin(VEHICLE_MUTE_PORT, VEHICLE_MUTE_PIN, GPIO_LOW_LEVEL);
    }
    else
    {
        Gpio_Hal_WritePin(VEHICLE_MUTE_PORT, VEHICLE_MUTE_PIN, GPIO_HIGH_LEVEL);
    }
}

uint8_t EcallHalGetSosButtonStatus(void)
{
    uint32_t voltage;
    uint8_t buttonStatus;

    PeripheralHalAdGet(AD_CHANNEL_SOS_KEY, &voltage);  
    buttonStatus = (uint8_t)Gpio_Hal_ReadPin(ECALL_BUTTON_PORT, ECALL_BUTTON_PIN);
    
    if((buttonStatus == GPIO_HIGH_LEVEL) && (voltage > 1000) && (voltage < 2300))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t EcallHalGetSosButtonConnectStatus(void)
{
    if(Gpio_Hal_ReadPin(PORTID_C, 11) == GPIO_LOW_LEVEL)
    {
        return 1;
    }
    return 0;
}

static void I2C0_Init(void)
{
    /* Pinmux 配置 */
    Gpio_Hal_SetMuxMode(I2C_SDA_PORT, I2C_SDA_PORT_PIN, I2C_SDA_PORT_MUX);
    Gpio_Hal_SetMuxMode(I2C_SCL_PORT, I2C_SCL_PORT_PIN, I2C_SCL_PORT_MUX);
    
    /* 开启内部上拉 */
    Gpio_Hal_SetPullSel(I2C_SDA_PORT, I2C_SDA_PORT_PIN, PORT_INTERNAL_PULL_UP_ENABLED);
    Gpio_Hal_SetPullSel(I2C_SCL_PORT, I2C_SCL_PORT_PIN, PORT_INTERNAL_PULL_UP_ENABLED);

    I2c_Hal_Init(USER_I2C_SCB_INDEX, &g_stcI2cChannelConfig);
}

static int16_t Sa51034WriteOneByte(uint8_t regAddr, uint8_t cValue)
{
    uint8_t TxBuffer[2] = {regAddr, cValue};
    DataTransmitType txData;
    
    txData.SlaveAddress  = SA51034_ADDRESS;
    txData.DirType       = I2C_WRITE;
    txData.SendStop      = TRUE;
    txData.DataBufferPtr = TxBuffer;
    txData.DataLength    = 2;
    txData.Is10bitAddr   = FALSE;

    if (I2c_Hal_SyncTransceive(USER_I2C_SCB_INDEX, &txData) == 0x00U) // STATUS_SUCCESS
    {
        return 0;
    }
    return 1;
}

static int16_t Sa51034ReadOneByte(uint8_t regAddr, uint8_t *buffer)
{
    DataTransmitType txData;
    
    /* 第一步：写入寄存器地址（不发Stop信号，准备Restart读） */
    txData.SlaveAddress  = SA51034_ADDRESS;
    txData.DirType       = I2C_WRITE;
    txData.SendStop      = FALSE; 
    txData.DataBufferPtr = &regAddr;
    txData.DataLength    = 1;
    txData.Is10bitAddr   = FALSE;

    if (I2c_Hal_SyncTransceive(USER_I2C_SCB_INDEX, &txData) != 0x00U) // STATUS_SUCCESS
    {
        return 1;
    }

    /* 第二步：读取数据 */
    txData.DirType       = I2C_READ;
    txData.SendStop      = TRUE;
    txData.DataBufferPtr = buffer;
    txData.DataLength    = 1;

    if (I2c_Hal_SyncTransceive(USER_I2C_SCB_INDEX, &txData) == 0x00U)
    {
        return 0;
    }
    return 1;
}

void Sa51500Init(void)                                                    
{
    uint16_t timerCounter = 100;
    uint8_t controlRegValue = 0;

    if(g_ampInitFlag)
    {
        return;
    }

    EcallHalSetAmpSdz(1);
    EcallHalSetAmpMute(1);
    I2C0_Init();                                             

    do
    {
        Sa51034ReadOneByte(CONTROL_REGISTER, &controlRegValue);
        timerCounter--;
    } while ((controlRegValue != 0x78) && (timerCounter));

    if(timerCounter != 0)
    {
        g_ampInitFlag = 1;
        g_ampInitTime = xTaskGetTickCount();
    }
}

void Sa51500Close(void)
{
    EcallHalSetAmpSdz(0);
    EcallHalSetAmpMute(0);
    Gpio_Hal_SetMuxMode(I2C_SDA_PORT, I2C_SDA_PORT_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(I2C_SDA_PORT, I2C_SDA_PORT_PIN, GPIO_INPUT_DIRECTION);
    
    Gpio_Hal_SetMuxMode(I2C_SCL_PORT, I2C_SCL_PORT_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(I2C_SCL_PORT, I2C_SCL_PORT_PIN, GPIO_INPUT_DIRECTION);
    
    g_ampInitFlag = 0;
}

uint8_t EcallHalGetAmpFaultStatus(void)
{
    uint16_t timerCounter = 3;
    uint8_t faultRegValue = 0;
    uint8_t ret = 0;
    uint32_t readTime = 0;

    if(g_ampInitFlag == 0)
    {
        Sa51500Init();
    }

    readTime = xTaskGetTickCount();
    if( (osElapsedTimeGet( readTime, g_ampInitTime )) < 600 )
    {
        return 0xFF;
    }

    do
    {
        timerCounter--;
        ret = Sa51034ReadOneByte(FAULT_REGISTER, &faultRegValue);
        if(ret == 0)
        {
            return faultRegValue;
        }
    } while (timerCounter);

    return 0xFF;
}

uint8_t EcallHalGetAmpDiagnosticStatus(void)
{
    uint16_t timerCounter = 3;
    uint8_t diagnosticRegValue = 0;
    uint8_t ret = 0;
    uint32_t readTime = 0;

    if(g_ampInitFlag == 0)
    {
        Sa51500Init();
    }

    readTime = xTaskGetTickCount();
    if( (osElapsedTimeGet( readTime, g_ampInitTime )) < 600 )
    {
        return 0xFF;
    }

    do
    {
        timerCounter--;
        ret = Sa51034ReadOneByte(DIAGNOSTIC_REGISTER, &diagnosticRegValue);
        if(ret == 0)
        {
            return diagnosticRegValue;
        }
    } while (timerCounter);

    return 0xFF;
}

uint8_t EcallHalGetAmpControlStatus(void)
{
    uint16_t timerCounter = 3;
    uint8_t contralRegValue = 0;
    uint8_t ret = 0;
    uint32_t readTime = 0;

    if(g_ampInitFlag == 0)
    {
        Sa51500Init();
    }

    readTime = xTaskGetTickCount();
    if( (osElapsedTimeGet( readTime, g_ampInitTime )) < 600 )
    {
        return 0xFF;
    }

    do
    {
        timerCounter--;
        ret = Sa51034ReadOneByte(CONTROL_REGISTER, &contralRegValue);
        if(ret == 0)
        {
            return contralRegValue;
        }
    } while (timerCounter);

    return 0xFF;
}

uint8_t EcallHalSetAmpControlStatus(uint8_t value)
{
    uint16_t timerCounter = 3;
    uint8_t contralRegValue = 0;
    uint8_t ret = 0;

    if(value > 4)
    {
        return 0xFF;
    }

    if(g_ampInitFlag == 0)
    {
        Sa51500Init();
    }

    do
    {
        timerCounter--;

        if(value == 0)
        {
            contralRegValue = 0x78;
        }
        else
        {
            ret = Sa51034ReadOneByte(CONTROL_REGISTER, &contralRegValue);
            if(ret != 0)
            {
                continue;
            }
            contralRegValue &= 0x3F;
            contralRegValue |= value << 6;
        }
        
        ret = Sa51034WriteOneByte(CONTROL_REGISTER, contralRegValue);
        if(ret != 0)
        {
            continue;
        }
        ret = Sa51034ReadOneByte(CONTROL_REGISTER, &contralRegValue);
        if(ret == 0)
        {
           return 0;
        }
        
    } while (timerCounter);

    return 0xFF;
}

uint8_t EcallHalRestartAmpClose(void)
{
    if(g_ampInitFlag == 0)
    {
        Sa51500Init();
    }
    else
    {
        EcallHalSetAmpSdz(0);
    }
    
    return 0x00;
}

uint8_t EcallHalRestartAmpDiagnostic(void)
{
    if(g_ampInitFlag == 0)
    {
        Sa51500Init();
    }
    else
    {
        EcallHalSetAmpSdz(0);
        vTaskDelay(10);
        EcallHalSetAmpSdz(1);
        g_ampInitTime = xTaskGetTickCount();
    }
    
    return 0x00;
}

uint8_t EcallHalGetMicConnectStatus(void)
{
    #define MIC_CHECK_RANGE     ( 150 )
    #define MIC_NORMAL_VOL      ( 1580 )
    #define MIC_VCC_SHORT_VOL   ( 3299 )
    #define MIC_GND_SHORT_VOL   ( 30 )
    #define MIC_OPEN_VOL        ( 3000 )

    uint32_t voltage;

    PeripheralHalAdGet(AD_CHANNEL_MICP, &voltage);

    if((voltage > (MIC_NORMAL_VOL - MIC_CHECK_RANGE)) && (voltage < (MIC_NORMAL_VOL + MIC_CHECK_RANGE)))
    {
        return 0;
    }
    else if((voltage > (MIC_GND_SHORT_VOL + MIC_CHECK_RANGE)) && (voltage < (MIC_VCC_SHORT_VOL - MIC_CHECK_RANGE)))
    {
        return 1;
    }
    return 2;
}

void EcallHalInit(void)
{
    EcallGpioInit();
    g_SosLedMsgQueue = xQueueCreate(5, sizeof(SosLledState_e));
    g_SosLedMutex = xSemaphoreCreateMutex();  
}

uint32_t osElapsedTimeGet(uint32_t new_time, uint32_t old_time)
{
    uint32_t tempDa;

    if (new_time >= old_time)
    {
        tempDa = new_time - old_time;
    }
    else
    {
        tempDa = (0xFFFFFFFFu - old_time) + new_time + 1u;
    }

    return tempDa;
}

void EcallHalSosLedTimerCallback( void )
{
    static uint8_t st = 1;
    
    if( g_SosLedSwitchTime.ledMode == E_ECALL_LED_MODE_FLASH )
    {
        if( st == 0 )
        {
            EcallHalSetSosLedRedState( 0 );
            EcallHalSetSosLedGreenState( 0 );
            g_SosLedSwitchTime.ledCounter++;
            if( g_SosLedSwitchTime.ledCounter >= g_SosLedSwitchTime.ledOffTime )
            {
                st = 1;
                g_SosLedSwitchTime.ledCounter = 0;
            }
        }
        else if( st == 1 )
        {
            EcallHalSetSosLedRedState( 1 );
            EcallHalSetSosLedGreenState( 1 );
            g_SosLedSwitchTime.ledCounter++;
            if( g_SosLedSwitchTime.ledCounter >= g_SosLedSwitchTime.ledOnTime )
            {
                st = 0;
                g_SosLedSwitchTime.ledCounter = 0;
            }
        }
    }
}

void EcallHalSetSosLedMode(EcallLedMode_e mode , uint16_t on_time , uint16_t off_time )
{
    g_SosLedSwitchTime.ledMode =  mode;
    g_SosLedSwitchTime.ledCounter = 0;
    g_SosLedSwitchTime.ledOnTime = on_time;
    g_SosLedSwitchTime.ledOffTime = off_time;

    if (mode == E_ECALL_LED_MODE_KEEP_OFF)
    {
        EcallHalSetSosLedRedState( 0 );
        EcallHalSetSosLedGreenState( 0 );
    }
    else if (mode == E_ECALL_LED_MODE_KEEP_ON)
    {
        EcallHalSetSosLedRedState( 1 );
        EcallHalSetSosLedGreenState( 1 );
    }
}

void EcallHalSetMode(uint8_t wakeMode)
{   
    if(0 == wakeMode)
    {
        EcallHalSetSosLedRedState(0);
        EcallHalSetSosLedGreenState(0);
        EcallHalSetVehicleMute(0);
        Gpio_Hal_WritePin(ECALL_PWR_EN_PORT, ECALL_PWR_EN_PIN, GPIO_LOW_LEVEL);
        Gpio_Hal_WritePin(CODEC_MIC_PWR_EN_PORT, CODEC_MIC_PWR_EN_PIN, GPIO_LOW_LEVEL);
    }
    else if(1 == wakeMode)
    {
        EcallHalSetSosLedRedState(0);
        EcallHalSetSosLedGreenState(0); 
        Gpio_Hal_WritePin(ECALL_PWR_EN_PORT, ECALL_PWR_EN_PIN, GPIO_HIGH_LEVEL);
        Gpio_Hal_WritePin(CODEC_MIC_PWR_EN_PORT, CODEC_MIC_PWR_EN_PIN, GPIO_HIGH_LEVEL);
    }  
}

uint32_t EcallHalSosLedControlReceive( SosLledState_e *msg, uint32_t time_out )
{
    if(g_SosLedMsgQueue == NULL)
    {
        return pdFAIL;
    }
    if(xQueueReceive( g_SosLedMsgQueue, msg, time_out ) == pdPASS)
    {
        return pdPASS;
    }
    else
    {
        return pdFAIL;
    }
}

uint32_t EcallHalSosLedControlSend( SosLledState_e flash )
{
    SosLledState_e trig_msg;
    uint32_t  ret = pdFAIL;
    xSemaphoreTake(g_SosLedMutex,portMAX_DELAY);
    trig_msg = flash;
    ret = xQueueSend( g_SosLedMsgQueue , &trig_msg , SOS_LED_QUEUE_SEND_WAIT_TIME );
    xSemaphoreGive(g_SosLedMutex);
    return ret;
}