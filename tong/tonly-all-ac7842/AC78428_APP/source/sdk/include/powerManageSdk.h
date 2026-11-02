#ifndef _POWER_MANAGE_SDK_H
#define _POWER_MANAGE_SDK_H

#include <stdint.h>

#define PM_SDK_STATUS_OK        0
#define PM_SDK_STATUS_ERR        (-1)


typedef void (* PmWakeCallFun_t)(uint8_t mcuWakecpuSource,uint8_t cpuWakeupSource,uint32_t *pWakeDelayTime);

typedef enum
{
    E_PM_STATE_UNPOWED = 0,
    E_PM_STATE_POWER_ON,//1
    E_PM_STATE_NMPU_SLEEP,//2
    E_PM_STATE_NMPU_PRE_CHECK_CAN,//3
    E_PM_STATE_NMPU_WAKE,//4
    E_PM_STATE_NMPU_DELAY,//5
    E_PM_STATE_NMPU_CHECK_NM_STATUS,//6
    E_PM_STATE_WAKE,//7
    E_PM_STATE_WAKE_DELAY,//8
    E_PM_STATE_CHECK_NM_STATUS,//9
    E_PM_STATE_PRE_SLEEP_NOTICE,//10
    E_PM_STATE_PRE_SLEEP_WAIT,//11
    E_PM_STATE_MCU_SLEEP,//12
    E_PM_STATE_PRE_CHECK_CAN,//13
    E_PM_STATE_GET_MPU_WAKE_SOURCE,//14    
    E_PM_STATE_GET_MPU_SLEEP_FINISH,//15
}PmSdkState_e;

typedef struct 
{
    uint8_t mpuDeepSleep;
    uint8_t gSensorDeepSleep;
}DeepSleepConfig_t;

typedef struct
{
    uint8_t mpuDeepSleep;
    uint8_t gSensorDeepSleep;
}CustomSleepConfig_t;

typedef struct
{
    uint8_t degInfo;
    uint8_t canNmType;
    uint32_t wakeDelayTime;
    PmWakeCallFun_t wakeupFun;
    uint32_t kl30OffWakeDelay;
    DeepSleepConfig_t deepSleepConfig;
    CustomSleepConfig_t customSleepConfig;
    
}PmSdkConfig_t;


int16_t PowerManageSdkInit(const PmSdkConfig_t* pmConfig);

void PowerManageSdkCycleProcess(uint32_t cycleTime);
void PowerManageSdkPowerOn(void);
int16_t PowerManageSdkSetWakeDelay(uint32_t time);

int16_t PowerManageSdkResetWake(uint8_t wakeMode);

int16_t PowerManageSdkOpenHandle(const char* pName);

int16_t PowerManageSdkGetSleepState(int16_t pmHandle);

int16_t PowerManageSdkSetSleepAck(int16_t pmHandle);

int16_t PowerManageSdkSetTestMode(uint8_t mode);

void PowerManageSdkDeepSleep(uint8_t deepSleepFlag);

void PowerManageSdkForceSleep(void);

void PowerManageSdkGetPowerInfo(uint8_t* pPmState,uint8_t* pWakeupSource,uint32_t* pWakeCount);

void PowerManageSdkSetMpuWakeUpFlag(uint8_t flag);

void PowerManageSdkSetFastSleep(uint8_t fastSleepFlag);

uint8_t PowerManageSdkGetFastSleep(void);





#endif