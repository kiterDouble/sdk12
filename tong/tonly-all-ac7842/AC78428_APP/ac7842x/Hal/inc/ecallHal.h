/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: ecalHal.h
 Author: 
 Created Time: 
 Description: 
 Others: 
*************************************************/
#ifndef _ECALLHAL_H
#define _ECALLHAL_H

/****************************** include ***************************************/
#include <stdint.h>

/****************************** Macro Definitions *****************************/
/* --- 适配后的 ECall 相关引脚定义 --- */

#define ECALL_LED_R_PORT                PORTID_D
#define ECALL_LED_R_PIN                 5U
#define ECALL_LED_R_PIN_MUX             PORT_MUX_AS_GPIO

#define ECALL_LED_G_PORT                PORTID_D
#define ECALL_LED_G_PIN                 12U
#define ECALL_LED_G_PIN_MUX                 PORT_MUX_AS_GPIO

#define ECALL_BK_LIGHT_PORT             PORTID_D
#define ECALL_BK_LIGHT_PIN              8U
#define ECALL_BK_LIGHT_PIN_MUX          PORT_MUX_AS_GPIO

#define ECALL_PWR_EN_PORT               PORTID_C
#define ECALL_PWR_EN_PIN                12U
#define ECALL_PWR_EN_PIN_MUX            PORT_MUX_AS_GPIO

#define ECALL_BUTTON_PORT               PORTID_C
#define ECALL_BUTTON_PIN                0U
#define ECALL_BUTTON_PIN_MUX            PORT_MUX_AS_GPIO

#define VEHICLE_MUTE_PORT               PORTID_D
#define VEHICLE_MUTE_PIN                9U
#define VEHICLE_MUTE_PIN_MUX            PORT_MUX_AS_GPIO

#define CODEC_MIC_PWR_EN_PORT           PORTID_A
#define CODEC_MIC_PWR_EN_PIN            17U
#define CODEC_MIC_PWR_EN_PIN_MUX        PORT_MUX_AS_GPIO

#define AMP_STB_PORT                    PORTID_E
#define AMP_STB_PIN                     14U
#define AMP_STB_PIN_MUX                 PORT_MUX_AS_GPIO

#define AMP_MUTE_PORT                   PORTID_B
#define AMP_MUTE_PIN                    11U
#define AMP_MUTE_PIN_MUX                PORT_MUX_AS_GPIO

#define FAULTZ_DET_PORT                 PORTID_E
#define FAULTZ_DET_PIN                  0U
#define FAULTZ_DET_PIN_MUX              PORT_MUX_AS_GPIO

#define SRS_EN_PORT                     PORTID_D
#define SRS_EN_PIN                      7U
#define SRS_EN_PIN_MUX                  PORT_MUX_AS_GPIO

#define SRS_STATE_PORT                  PORTID_D
#define SRS_STATE_PIN                   6U
#define SRS_STATE_PIN_MUX               PORT_MUX_AS_GPIO
#define SRS_STATE_PIN_IRQ               PORTD_IRQn

/****************************** Type Definitions ******************************/
typedef enum
{
    E_ECALL_LED_MODE_KEEP_OFF = 0x00,
    E_ECALL_LED_MODE_KEEP_ON,
    E_ECALL_LED_MODE_FLASH,
}EcallLedMode_e;

typedef enum
{
    E_SOS_LED_STATE_INIT = 0x00,   /*初始化状态*/
    E_SOS_LED_STATE_RING,          /*ECALL拨号中*/    
    E_SOS_LED_STATE_CALL,          /*ECALL通话中*/
    E_SOS_LED_STATE_WAIT_BACK,     /*ECALL等待PSAP应答*/
    E_SOS_LED_STATE_END,           /*ECALL结束、自检结束*/

    E_SOS_LED_STATE_SELFCHECK_ERR,      /*ECALL终端自检异常*/
    E_SOS_LED_STATE_WARNING,         /*LED灯状态重要提示*/
    E_SOS_LED_STATE_SELFCHECK_ON,            /*LED灯状态打开*/
}SosLledState_e;

/****************************** Function Declarations *************************/
void EcallHalSetSosLedRedState(uint8_t flag);
void EcallHalSetSosLedGreenState(uint8_t flag);
void EcallHalSetVehicleMute(uint8_t flag);
uint8_t EcallHalGetSosButtonStatus(void);
uint8_t EcallHalGetSosButtonConnectStatus(void);
uint8_t EcallHalGetMicConnectStatus(void);
void EcallHalSetAmpSdz(uint8_t flag);
void EcallHalInit(void);
uint8_t EcallHalGetAmpFaultStatus(void);
uint8_t EcallHalGetAmpDiagnosticStatus(void);
uint8_t EcallHalGetAmpControlStatus(void);
uint8_t EcallHalSetAmpControlStatus(uint8_t value);
uint8_t EcallHalRestartAmpClose(void);
uint8_t EcallHalRestartAmpDiagnostic(void);
uint32_t osElapsedTimeGet( uint32_t new_time, uint32_t old_time );
void EcallHalSosLedTimerCallback( void );
void EcallHalSetSosLedMode(EcallLedMode_e mode , uint16_t onTime , uint16_t offTime );
uint32_t EcallHalSosLedControlReceive( SosLledState_e *msg, uint32_t time_out );
uint32_t EcallHalSosLedControlSend( SosLledState_e flash );
void EcallHalSetMode(uint8_t wakeMode);
void EcallHalSetSrsEn(uint8_t flag);

#endif  // _ECALLHAL_H