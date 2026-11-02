#ifndef _ALARM_SDK_H
#define _ALARM_SDK_H

#include <stdint.h>
#include "mpuHal.h"

typedef enum
{
    E_ECALL_TRIGGER_BTN_MANN = 0,
    E_ECALL_TRIGGER_SRS_AUTO,
    E_ECALL_TRIGGER_CAN_AUTO,
    E_ECALL_TRIGGER_GSENSOR_AOTU,
    E_ECALL_TRIGGER_TEST_MODE,		
}EcallTriggerType_e;

typedef enum
{
    ECALL_TRIGGER_RESULT_FAIL = 0,
    ECALL_TRIGGER_RESULT_SUCC,		
}EcallTriggerResult_e;

typedef enum
{
    E_ECALL_STATE_NO_ECALL = 0,
    E_ECALL_STATE_INCOMING_CALL,
    E_ECALL_STATE_IN_CALL,
    E_ECALL_STATE_DURING_CALL,
    E_ECALL_STATE_END_CALL,
    E_ECALL_STATE_END_CALL_ABNORM,
    E_ECALL_STATE_WAIT_PSPA_CALLBACK,
    E_ECALL_STATE_SEND_MSD,
    E_ECALL_STATE_PUSH_MSD,
    E_ECALL_STATE_UPDATE_MSD,
    E_ECALL_STATE_SEND_MSD_SUCC,
    E_ECALL_STATE_SEND_MSD_FAIL,
}EcallStatusType_e;

typedef enum
{
    E_NAD_FAULT = 0,
    E_BATTERY_FAULT,
    E_SPEAKER_FAULT,
    E_ABM_FAULT,
    E_CAN_FAULT,
    E_SOS_FAULT,
    E_MIC_FAULT,
    E_KL30_FAULT,
    E_SELF_TEST_STATE,
    E_MCU_FAULT_STATUS
}McuSelfcheckItem_e;

typedef enum
{
    E_GSM_MAIN_ANT_FAULT = 0,
    E_GSM_SEC_ANT_FAULT,
    E_GPS_ANT_FAULT,
    E_SIM_CARD_FAULT,
    E_MPU_FAULT_STATUS
}MpuSelfcheckItem_e;

/** ****************************************************************************
 * @brief 触发ECALL报警的状态
 *******************************************************************************/
#pragma pack(1)
typedef struct
{
	volatile uint8_t self_test;   	        /*设备自检 0:自检中 1:自检成功 2:自检异常 3:未知错误*/
	union   ///< 消息属性
	{
		volatile uint32_t fault_st; 		/*故障状态*/
		struct
		{
			uint32_t nad_fault : 4;		    /*NAD模组运行异常*/	  
			uint32_t battery_fault : 4; 	/*备用电池连接异常*/
			uint32_t speaker_fault : 4; 	    /*扬声器连接异常*/
			uint32_t abm_fault : 4; 	    /*ABM连接异常*/
			uint32_t can_fault : 4; 	    /*CAN硬线连接异常*/
			uint32_t sos_fault : 4;	        /*SOS(按键)连接异常*/
			uint32_t mic_fault : 4;	        /*MIC连接异常*/
            uint32_t kl30_fault : 4;        /*外部电源阀值检测异常*/
		};
	};
}McuSelfcheckMsg_t;

typedef struct
{
	union   ///< 消息属性
	{
		volatile uint32_t fault_st; 		/*故障状态*/
		struct
		{
			uint32_t gsmMainAnt_fault : 4;  /* 4G 主天线异常*/	  
			uint32_t gsmSecAnt_fault : 4; 	/* 4G 副天线异常*/
			uint32_t gpsAnt_fault : 4; 	    /* GPS 天线异常*/
			uint32_t simCard_fault : 4;     /* SIM 卡异常*/
			uint32_t reserve : 4; 	    /* */
			uint32_t reserve1 : 4;	    /* */
			uint32_t reserve2 : 4;	    /* */
            uint32_t reserve3 : 4;       /* */
		};
	};
}MpuSelfcheckMsg_t;

typedef enum
{
	E_SELFCHECK_RUN_INIT = 0,	
	E_SELFCHECK_RUN_START,
	E_SELFCHECK_RUN_STOP,		
	E_SELFCHECK_RUN_END,			
}SelfcheckRunState_e;

extern SelfcheckRunState_e g_selfcheckRunState;

int16_t AlarmSdkEcallTriger(uint8_t type);
int16_t AlarmSdkBcallTriger(uint8_t type);
int16_t AlarmSdkInit(void);
void AlarmSdkCycleProcess(void);
void AlarmSdkSetSelfcheckState(SelfcheckRunState_e state);
SelfcheckRunState_e AlarmSdkGetSelfcheckState(void);
void AlarmSdkSetMcuSelfcheckResult(uint8_t item, uint8_t result);
McuSelfcheckMsg_t* AlarmSdkGetMcuSelfcheckResult(void);
void AlarmSdkSetMpuSelfcheckResult(uint8_t item, uint8_t result);
MpuSelfcheckMsg_t* AlarmSdkGetMpuSelfcheckResult(void);
int16_t AlarmSdkSelfchackPeriSend(void);
uint8_t AlarmSdkGetEcallTriggerType(void);
uint8_t AlarmSdkGetBcallTriggerType(void);
int16_t AlarmSdkEcallClose(uint8_t type);
int16_t AlarmSdkBcallClose(uint8_t type);
uint8_t AlarmSdkGetEcallCallState(void);
uint8_t AlarmSdkGetBcallCallState(void);

#endif  // _ALARM_SDK_H