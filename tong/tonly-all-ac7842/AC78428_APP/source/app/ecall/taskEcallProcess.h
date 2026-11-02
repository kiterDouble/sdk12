#ifndef _TASK_APP_ECALL_PROCESS_H
#define _TASK_APP_ECALL_PROCESS_H

#include <stdint.h>
#include "ecallHal.h"


/****************************************************************************************
* SOS 按键控制状态结构体
******************************************************************************************/
typedef enum
{
	E_SOS_BUTTON_STATE_IDLE	= 0x00,	    	/*按键未被操作*/
	E_SOS_BUTTON_STATE_CLICK,				/*按键被触发*/
	E_SOS_BUTTON_STATE_PRESS,				/*按键被按下未放开*/
	E_SOS_BUTTON_STATE_RELEASED,			/*按键被松开*/
	E_SOS_BUTTON_STATE_CANCELLED,			/*按键被取消*/
	E_SOS_BUTTON_STATE_TESTMODE,			/*按键测试模式*/
	E_SOS_BUTTON_STATE_HARDFAULT,			/*按键故障*/
}SosButtonState_e;

typedef struct
{
	uint32_t clickTime;		/*按键被按压后获取的系统时间*/	
	uint32_t pressTime;		/*按键被按住未放获取的时间状态*/
	uint32_t releasedTime;	/*按键被松开后获取的系统时间*/
	uint32_t cancelTime;	/*按键时间被取消*/
	uint8_t hardFault;
}SosButtonClickMsg_t;

typedef enum
{
    AIRBAG_PWM_UNKNOWN = 0,   /* 尚未稳定识别 */
    AIRBAG_PWM_NORMAL,        /* 正常 66.7% */
    AIRBAG_PWM_CRASH          /* 碰撞 33.3% */
} AirbagPwmState_e;

typedef enum
{
    TELEMATICS_MODE_NOT_ACTIVE = 0,   /* Not active */
    TELEMATICS_MODE_TBOX = 1,         /* TBOX mode */
    TELEMATICS_MODE_BLUETOOTH = 2,    /* Bluetooth mode */
    TELEMATICS_MODE_BCALL = 3,        /* Bcall Mode */
    TELEMATICS_MODE_ECALL = 4         /* Ecall Mode */
} TelematicsMode_e;

typedef enum
{
    E_ECALL_STATE_NOT_ACTIVE = 0,
    E_ECALL_STATE_INCOMING_TELEGRAM,
    E_ECALL_STATE_CONNECTING,
    E_ECALL_STATE_ANSWER_CALLS,
	E_ECALL_STATE_ON_THE_PHONE,
    E_ECALL_STATE_HANG_UP,
	E_ECALL_STATE_CALL_OVER,
} PhoneCallState_e;

/* 中断回调（每次上升沿/下降沿调用一次） */
void AirbagPwmIsrHandler(uint8_t level);
void TaskEcallProcess( void *pvParameters );
TelematicsMode_e XCallGetTelemataticsMode(void);
PhoneCallState_e XCallGetPhoneCallState(void);
void XCallSetTelemataticsMode(TelematicsMode_e mode);
void XCallSetPhoneCallState(PhoneCallState_e state);
#endif    //_TASK_APP_ECALL_PROCESS_H
