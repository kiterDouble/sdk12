/*************************************************
 Copyright © 2025 SiRun (AnHui) . All rights reserved.
 File Name: autosarNmSdk.h
 Author: 
 Created Time: 
 Description: 
 Others: 
*************************************************/
#ifndef _AUTOSARNMSDK_H
#define _AUTOSARNMSDK_H
/****************************** include ***************************************/
#include "canHal.h"

/****************************** Macro Definitions ******************************/

/****************************** Type Definitions ******************************/
typedef void (* NmAutosarEventCallBackFun_t)(uint8_t flag);

typedef struct
{
    uint8_t NodeId;
    uint32_t NodeIdMin;
    uint32_t NodeIdMax;
    uint8_t canChannel;  
    uint32_t BaseAddress;
    uint32_t startUpTime;                 //GAC rename  repeatMessageTime  to start up time
    uint32_t nmTimeOutTime;           
    uint32_t waitBusSleepTime;        
    uint32_t startxAppFrameTime;      
    uint32_t immediateCycleTime;      
    uint32_t msgCycleTime;            
    uint32_t immediateTimes;          
    uint32_t startTXTime;             
    uint32_t wakeupTime;              
    uint32_t busOffErrorEventLimitCount;
    uint32_t busOffQuickTime;
    uint32_t busOffSlowTime;
    uint32_t busOffQuickTimes;         
    NmAutosarEventCallBackFun_t canBusErrorCallBack;
}AutosarNmParameter_t;

/****************************** Global Variables ******************************/

/****************************** Function Declarations *************************/
int16_t AutosarNmSdkConfig(const AutosarNmParameter_t *pNmConfigre,uint8_t NmNumber);
int16_t AutosarNmSdkNetworkRequest(uint8_t LocalWakeup);
int16_t AutosarNmSdkNetworkRelease( const int16_t nmChannelHandle );
int16_t AutosarNmSdkEnableCommunication( uint8_t nmID );
int16_t AutosarNmSdkDisableCommunication( uint8_t nmID );
int16_t AutosarNmSdkGetSleepStatus(uint8_t nmID,uint8_t *sleepState);
int16_t AutosarNmSdkGetCanStatus(uint8_t nmID,uint8_t *ReceiveState);
int16_t AutosarNmSdkGetNetManageReceiveStatus(uint8_t nmID,uint8_t *ReceiveState);
void AutosarNmSdkGetNetManageReceiveStatusClear(uint8_t nmID);
void AutosarNmSdkGetNetManageCanChannel(uint8_t nmID,uint8_t *channelFlag);
void NetManageAutosarCycleProcess(void);
uint8_t AutosarNmSdkGetBusState(void);
int16_t AutosarNmSdkSetSubNetWakeupRequest(uint8_t wakeupRequestFlags);
int16_t AutosarNmSdkClearSubNetWakeupRequest(void);

#endif  //_AUTOSARNMSDK_H