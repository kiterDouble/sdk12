/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: canHal.h
 Author: 
 Created Time: 
 Description: 
 Others: 
*************************************************/
#ifndef _CANHAL_H
#define _CANHAL_H

/****************************** include ***************************************/
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
/****************************** Macro Definitions ******************************/
#define CAN_ERROR_INVALID_HANDLE        (-1)
#define CAN_ERROR_INVALID_PARAMETER     (-2)
#define CAN_ERROR_TIMEOUT               (-3)
#define CAN_ERROR_BUS_OFF_LINE          (-4)
#define CAN_ERROR_BUS_ERROR             (-5)
#define CAN_ERROR_TX_BUFFER_FULL        (-6)
#define CAN_ERROR_TEST_MODE             (-7)
#define CAN_ERROR_TX_DISABLE            (-8)
#define CAN_ERROR_MUTEX_NOT_INIT        (-9)
#define CAN_ERROR_MUTEX_TIMEOUT         (-10)

#define TBOX_CAN_CHANNEL_1              (0)
#define TBOX_CAN_CHANNEL_2              (1)
#define TBOX_CAN_CHANNEL_3              (2)
#define TBOX_CAN_CHANNEL_4              (3)
#define TBOX_CAN_CHANNEL_5              (4)
#define TBOX_CAN_CHANNEL_6              (5)

#define CAN_HAL_TIMESTAMP_TICK_HZ       (500000UL)

#define TBOX_CAN_CHANNEL_A              TBOX_CAN_CHANNEL_3
#define TBOX_CAN_CHANNEL_B              TBOX_CAN_CHANNEL_1
#define TBOX_CAN_CHANNEL_C              TBOX_CAN_CHANNEL_4
#define TBOX_CAN_CHANNEL_D              TBOX_CAN_CHANNEL_2
#define TBOX_CAN_CHANNEL_E              TBOX_CAN_CHANNEL_6
#define TBOX_CAN_CHANNEL_I              TBOX_CAN_CHANNEL_5

/* ==================== CANFD1 (对应原 CAN0) ==================== */
#define CANFD1_RX_PORT                  PORTID_B
#define CANFD1_RX_PIN                   0
#define CANFD1_TX_PORT                  PORTID_B
#define CANFD1_TX_PIN                   1
#define CANFD1_RX_MUX                   PORT_MUX_ALT5
#define CANFD1_TX_MUX                   PORT_MUX_ALT5
#define CANFD1_IRQ_MUX                  PORT_MUX_AS_GPIO
#define CANFD1_IRQN                     PORTB_IRQn

/* ==================== CANFD2 (对应原 CAN1) ==================== */
#define CANFD2_RX_PORT                  PORTID_A
#define CANFD2_RX_PIN                   12
#define CANFD2_TX_PORT                  PORTID_A
#define CANFD2_TX_PIN                   13
#define CANFD2_RX_MUX                   PORT_MUX_ALT3
#define CANFD2_TX_MUX                   PORT_MUX_ALT3
#define CANFD2_IRQ_MUX                  PORT_MUX_AS_GPIO 
#define CANFD2_IRQN                     PORTA_IRQn

/* ==================== CANFD3 (对应原 CAN2) ==================== */
#define CANFD3_RX_PORT                  PORTID_B
#define CANFD3_RX_PIN                   12
#define CANFD3_TX_PORT                  PORTID_B
#define CANFD3_TX_PIN                   13
#define CANFD3_RX_MUX                   PORT_MUX_ALT4
#define CANFD3_TX_MUX                   PORT_MUX_ALT4
#define CANFD3_IRQ_MUX                  PORT_MUX_AS_GPIO
#define CANFD3_IRQN                     PORTB_IRQn

/* ==================== CANFD4 (新增第4路CAN) ==================== */
#define CANFD4_RX_PORT                  PORTID_E
#define CANFD4_RX_PIN                   12
#define CANFD4_TX_PORT                  PORTID_E
#define CANFD4_TX_PIN                   3
#define CANFD4_RX_MUX                   PORT_MUX_ALT5
#define CANFD4_TX_MUX                   PORT_MUX_ALT5
#define CANFD4_IRQ_MUX                  PORT_MUX_AS_GPIO
#define CANFD4_IRQN                     PORTE_IRQn

/* ==================== STB 唤醒引脚控制 ==================== */
#define CAN1FD_STB_PORT                 PORTID_C
#define CAN1FD_STB_PIN                  10
#define CAN1FD_STB_PIN_MUX              PORT_MUX_AS_GPIO

#define CAN2FD_STB_PORT                 PORTID_C
#define CAN2FD_STB_PIN                  10
#define CAN2FD_STB_PIN_MUX              PORT_MUX_AS_GPIO

#define CAN3FD_STB_PORT                 PORTID_C
#define CAN3FD_STB_PIN                  10
#define CAN3FD_STB_PIN_MUX              PORT_MUX_AS_GPIO

#define CAN4FD_STB_PORT                 PORTID_C
#define CAN4FD_STB_PIN                  10
#define CAN4FD_STB_PIN_MUX              PORT_MUX_AS_GPIO

/* ==================== 中断复用引脚 (休眠唤醒用) ==================== */
#define CAN0_IRQ_MUX                    PORT_MUX_AS_GPIO
#define CAN1_IRQ_MUX                    PORT_MUX_AS_GPIO
#define CAN2_IRQ_MUX                    PORT_MUX_AS_GPIO
/****************************** Type Definitions ******************************/
typedef void(* typeCanTxRxHalCallBackPtr)(uint32_t canId,uint8_t *data,uint8_t dlc);
typedef void(* typeCanTxHalCallBackPtr)(void);
typedef void (*typeCanBusOffEventCallBack)(uint8_t flag);
typedef void(* typeCanNoAckStuffErrorRecoverEvent)(void);
typedef int16_t(* typeCanRxHalFilterCallBackPtr)(uint32_t canId,uint8_t *data,uint8_t dlc);
typedef enum 
{
    E_CAN_125K = 0,
    E_CAN_250K,
    E_CAN_500K,
    E_CAN_1000K,
    E_CAN_2000K,
    E_CAN_3000K,
    E_CAN_4000K,
    E_CAN_5000K,
}CanBaudType_e;

typedef struct
{
  uint8_t canChannel;
  uint8_t canFdMode;
  CanBaudType_e bandrate;
  CanBaudType_e dataBandrate;
  typeCanBusOffEventCallBack busOffEventFun;
}CanConfigure_t;

typedef struct
{
    uint32_t canId;
    uint8_t canData[64];
    uint8_t dlc;
    uint8_t txFlag;
    uint32_t hwTimeStamp;  /*!< Hardware RX timestamp captured in ISR.
                                AC7842x internal TSC increments at the nominal
                                CAN bit rate. At 500K, one tick is 2us. */
}CanHalMsg_t;

/**
 * @brief Convert hwTimeStamp (TSC ticks) to microseconds.
 * @param[in] hwTs       32-bit hardware timestamp from CanHalMsg_t.hwTimeStamp
 * @param[in] tickHz     TSC tick frequency in Hz (0 => CAN_HAL_TIMESTAMP_TICK_HZ)
 * @return Microseconds since TSC start (mod 32-bit)
 *
 * @details The timestamp tick rate is tied to the nominal CAN bit rate.
 *          The 32-bit return value wraps every ~71.6 minutes.
 */
static inline uint32_t CanHalHwTsToUs(uint32_t hwTs, uint32_t tickHz)
{
    if (tickHz == 0UL)
    {
        tickHz = CAN_HAL_TIMESTAMP_TICK_HZ;
    }
    return (uint32_t)(((uint64_t)hwTs * 1000000ULL) / (uint64_t)tickHz);
}

typedef struct
{
    uint32_t canIdExtdMin;
    uint32_t canIdExtdMax;  
    uint16_t canIdStdMin;
    uint16_t canIdStdMax;
    uint8_t useStdFilter;
    uint8_t useExdtFilter;
}CanHalMsgFilter_t;

typedef enum
{
    CAN_TX_PRIO_NORMAL = 0,
    CAN_TX_PRIO_HIGH   = 1
} CanTxPrio_e;
/****************************** Global Variables ******************************/
/****************************** Function Declarations *************************/
int16_t CanHalInit(const CanConfigure_t *pCanConfig,uint8_t CanNum);
int16_t CanHalOpen(uint8_t canChannel);
int16_t CanHalSetRxBuffer(int16_t canHandle,CanHalMsg_t *pMsgBuf,uint16_t bufMsgCount);
int16_t CanHalSetFilter(int16_t canHandle,CanHalMsgFilter_t *pMsgFilter);
int16_t CanHalSetFilterFun(int16_t canHandle,typeCanRxHalFilterCallBackPtr pFilterFun);
uint32_t CanHalGetTimestampTickHz(int16_t canHandle);
int16_t CanHalReceive(int16_t canHandle,CanHalMsg_t *pMsg,uint32_t timeOut_ms);
int16_t CanHalNmReceive(int16_t canHandle,CanHalMsg_t *pMsg,uint32_t timeOut_ms);
int16_t CanHalRecieveIsTimeOut(int16_t canHandle);
int16_t CanHalAppMsgEnable(int16_t canHandle);
int16_t CanHalAppMsgDisable(int16_t canHandle);
int16_t CanHalGetIsBusOffError(int16_t canHandle);
int16_t CanHalEnableTransmitLoopBack(int16_t canHandle);
int16_t CanHalDiagnosticReceive(int16_t canHandle,CanHalMsg_t *pMsg,uint32_t timeOut_ms);
int16_t CanHalTxMsgEnable(uint8_t canChannel);
int16_t CanHalTxMsgDisable(uint8_t canChannel);
int16_t CanHalSetMode(uint8_t mode);
int16_t CanHalRegisterTxFinishedCallBackFunction(int16_t canHandle,typeCanTxHalCallBackPtr pCallBackFun);
int16_t CanHalNmTransmit(int16_t canHandle,uint32_t canId,uint8_t *canData,uint8_t dlc,uint8_t fdFlag);
void CanHalCanBusOffProcessEnalbe(uint8_t canChannel);
void CanHalCanBusOffProcessDisalbe(uint8_t canChannel);
void CanHalSetCanTestMode(uint8_t canChannel,uint8_t modeFlag);
void CanHalEnableCanWakeup(uint8_t canChannel);
int16_t CanHalResetHardware(int16_t canHandle);
void CanHalReceiveTask( void *pvParameters );
void CanHalTestMain(void);
void CanHalClearReceiveCanNmFlag(void);
uint8_t CanHalReceiveCanNmFlagCheck(void);
uint8_t CanHalClearBusoffAppDisableFlag(int16_t canHandle);
void CanHalSendTask(void *pvParameters);
int16_t CanHalTransmitQueued(int16_t canHandle, uint32_t canId, const uint8_t *canData, uint8_t dlc, uint8_t txFlag, CanTxPrio_e prio);
uint8_t CanHalTxCanSendHook(uint8_t canChannel);
void CanHal_SetOtaMode(uint8_t enable);
#endif  // _CANHAL_H
