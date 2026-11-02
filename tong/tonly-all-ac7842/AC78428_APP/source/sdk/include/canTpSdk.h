#ifndef  _CANTPSDK_H
#define  _CANTPSDK_H

#include "canHal.h"


typedef struct
{
    uint8_t blockSize_APP;//application mode
    uint8_t STmin_APP;//application mode
    uint8_t blockSize_boot;//boot mode
    uint8_t STmin_boot;//boot mode
    uint8_t fillByte;    
    uint32_t N_As_enforced;//70ms
    uint32_t N_Ar_enforced;//70ms
    uint32_t N_Bs_enforced;//70ms
    uint32_t N_Br_enforced;//70ms
    uint32_t N_Cs_enforced;//70ms
    uint32_t N_Cr_enforced;//70ms
    uint32_t N_As_obd;//25ms
    uint32_t N_Ar_obd;//25ms
    uint32_t N_Bs_obd;//70m
    uint32_t N_Br_obd;//(N_Br+N_Ar)??25ms
    uint32_t N_Cs_obd;//(N_Cs+N_As) ??50ms
    uint32_t N_Cr_obd;//(N_Br+N_Ar)??25ms  

}CanTpSdkParameter_t;

typedef struct
{
  CanHalMsg_t physicalCanMsgBuf[30];
  CanHalMsg_t functionCanMsgBuf[10];  
}CanTpSdkBuffer_t;

//int16_t CanTpSdkInitialize(uint8_t canChannle);
int16_t CanTpSdkInitialize(uint8_t canChannle,const CanTpSdkParameter_t *pTpParameter,CanTpSdkBuffer_t *pTpBuffer);
int16_t CanTpSdkSetCanId(int16_t tpHandle,uint32_t pysicalRxId,uint32_t functionRxId,uint32_t txId);


int16_t CanTpSdkDataReceive(int16_t tpHandle,uint8_t *udsData,uint16_t *udsLen,uint8_t *pFunctionFlag);
int16_t CanTpSdkDataTransmit(int16_t tpHandle,uint32_t txCanId,uint8_t *txData,uint16_t txLength,uint8_t functionAddFlag);

void CanTpSdkSetBootMode(int16_t tpHandle,uint8_t bootModeFlag);

#endif /*_CANTPSDK_H*/