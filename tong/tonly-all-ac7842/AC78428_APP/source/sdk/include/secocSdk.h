#ifndef _SECOC_SDK_H
#define _SECOC_SDK_H

#include "stdint.h"
#include "canHal.h"

#define SECOC_ENABLE        0
#define SECOC_STATUS_OK     0
#define SECOC_STATUS_ERR    -1

typedef int16_t (* SecocGetKeyFun_t)(uint8_t *pKey,uint32_t *pKeyLength);
typedef void (* SecocSetDefaultMacCodeFun_t)(uint8_t *pMac,uint8_t macLength);
typedef void (* SecocAesCalculateFun_t)(uint8_t* pAuthData,uint32_t authLength,uint8_t *pKey,uint8_t *pResultOut);
typedef void (* SecocSaveTripCountFun_t)(uint32_t tripCount);
typedef void (* SecocReadTripCountFun_t)(uint32_t *pTripCount);
typedef void (* SecocSyncMsgMacCheckErrorFun_t)(uint8_t flag);

typedef uint32_t PduIdType;
/*
typedef struct 
{
	uint8_t data[64];
	uint16_t dlc;
}PduInfoType;
*/
typedef struct
{
    uint32_t canId;
    uint32_t authStartByte;
}SecocSdkMsg_t;

typedef struct
{
	uint32_t msgCount;//22bit
	uint32_t tripCnt;//24bit
	uint16_t resetCnt;//16bit
}SecocMsgBuffer_t;

typedef struct
{
	uint16_t CAN_FV_TruncLength;
	uint16_t CAN_MAC_TruncLength;
	uint16_t CANFD_FV_TruncLength;
	uint16_t CANFD_MAC_TruncLength;
	uint16_t SecocFreshnessValueSyncAttempts;
    uint32_t syncMsgRequestId;
    const SecocSdkMsg_t *pTxSecocMsgConfig;    
    const SecocSdkMsg_t *pRxSecocMsgConfig;
    SecocMsgBuffer_t*pTxMsgBuffer;
    SecocMsgBuffer_t*pRxMsgBuffer;
    uint16_t txSecocMsgNum;
    uint16_t rxSecocMsgNum;
    SecocGetKeyFun_t pGetSecocKeyFun;
    SecocAesCalculateFun_t pAesCalculateFun;
    SecocSaveTripCountFun_t pSaveTripCountFun;
    SecocReadTripCountFun_t pReadTripCountFun;
    SecocSetDefaultMacCodeFun_t pSetDefaultMacCodeFun;
    SecocSyncMsgMacCheckErrorFun_t pMsgMacCheckErrorFun;
}SecocSdkConfig_t;


int16_t SecocSdkInit(const SecocSdkConfig_t* pSecocConfig);
void SecocSdkReInit(void);
//Std_ReturnType SecocSdkTransmit(int16_t canHandle,uint32_t canId, const PduInfoType *PduInfoPtr);
int16_t SecocSdkTransmit(int16_t canHandle,uint32_t canId, const uint8_t *pTxData,uint16_t dlc,uint8_t fdFlag);
int16_t SecocSdkReceive(int16_t canHandle,CanHalMsg_t *pMsg,int32_t timeOut_ms);

void SecocSdkWakeUp(void);
void SecocSdkWakeupResetState(void);
void SecocSdkSyncMessageCycleProcess(int16_t canHandle,uint32_t cycleTime);
void SecocSdkSyncMessageReceiveProcess(uint32_t canId,uint8_t *pCanData);
void SecocSdkGetVersion(uint8_t *pVersion,uint8_t *pVersionLength);
int16_t SecocSdkCheckPdu(CanHalMsg_t *pMsg);
uint8_t SecocSdkKeyGetKeyIsValid(void);


void SecocSdkStartSyncMsgRequest(void);

void SecocSdkSetTripCount(uint32_t tripCountValue);
void SecocSdkSetRestCount(uint32_t resetCountValue);
uint32_t SecocSdkGetTripCount(void);
uint32_t SecocSdkGetRestCount(void);


#endif //_SECOC_H
