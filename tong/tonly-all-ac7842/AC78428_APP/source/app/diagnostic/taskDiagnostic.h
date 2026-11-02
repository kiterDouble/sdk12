#ifndef _TASK_DIAGNOSTIC_H
#define _TASK_DIAGNOSTIC_H

#include <stdint.h>

typedef enum
{
	E_NO_SESSION = 0,
	E_DEFAULT_SESSION,
	E_PROGROM_SESSION,
	E_EXTEND_SESSION,
	E_NON_DIAGNOSTIC,
} SessionState_e;
#if 1 //SQ
typedef enum
{
	E_UDS_NONE_SECURITY_LEVEL = 0,
	E_UDS_SECURITY_LEVEL1,		  //  27 01/02	Level
	E_UDS_SECURITY_LEVEL_REPROG,  //  27 09/0A	Leve3
} UdsSecurityLevel_e;
#else
typedef enum
{
	E_UDS_NONE_SECURITY_LEVEL = 0,
	E_UDS_SECURITY_LEVEL1,		  //  27 01/02
	E_UDS_SECURITY_LEVEL2_CUSTOM, //  27 09/0A保留
	E_UDS_SECURITY_LEVEL2_STD,	  //  27 03/04
	E_UDS_SECURITY_LEVEL_REPROG,  //  27 11/12
	E_UDS_SECURITY_LEVEL3_STD,	  //  27 21/22
	E_UDS_SECURITY_LEVEL5,		  // 保留
	E_UDS_SECURITY_LEVEL9,		  // 保留
} UdsSecurityLevel_e;
#endif
/*
typedef union
{
	uint32_t u32Bit;
	uint16_t u16Bit[2];
	uint8_t  u8Bit[4];
} Unn1Word4Byte_t;
*/

int16_t TaskEcuDiagnosticInit(void);
void TaskEcuDiagnostic(void *pvParameters);
uint8_t GetTesterPresenceStatus(void); //获取当前诊断仪连接状态
SessionState_e GetCurrentSession(void);
UdsSecurityLevel_e GetCurrentSecurityLevel(void);
uint16_t GetVehicleSpeed(void); //获取车速
void SetEcuOnlineFlag(void);
#endif //_DIAGNOSTIC_H
