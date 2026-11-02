#include "SecocSdk.h"
#include "SecocConfig.h"
#include "aes128.h"
#include "hsmHal.h"
#include "algorithm1.h"
#include "ParameterStoreManageApp.h"
#include <string.h>


static const SecocSdkMsg_t g_txSecocMsgConfigList[] = 
{
/*can id,auth start byte number***************************/
	{0x033,8},
	{0x3E9,8},
};


static const SecocSdkMsg_t g_rxSecocMsgConfigList[] = 
{
/*can id,auth start byte number***************************/
	{0x27F,8},
    {0x319,8},
    {0x345,8},
    {0x19C,8},
    {0x341,8},
    {0x395,8},
};

static uint8_t g_Vin[17];

static int IsSecocKeyValid(uint8_t *pKey)
{
	uint8_t i;
	uint8_t valid;
	//check all is 0xFF
	if(pKey[0]==0xFF)
	{
		valid = 0;
		for(i=0;i<16;i++)
		{
			if(pKey[i]!=0xFF)
			{
				valid = 1;
				break;
			}
		}		
		if(0==valid)
		{
			return -1;
		}		
	}	
	//check is all 0x00
	if(pKey[0]==0x00)
	{
		valid = 0;
		for(i=0;i<16;i++)
		{
			if(pKey[i]!=0x00)
			{
				valid = 1;
				break;
			}
		}	
		if(0==valid)
		{
			return -1;
		}			
	}
	return 0;
}

int16_t SecocGetKey(uint8_t *pKey,uint32_t *pKeyLength)
{
    uint8_t tem[30];
    //uint8_t vin[20];
	uint32_t len;

    len = 0;
    WorkFlashVehicleInforRead(E_PARAMETER_INFO_VIN,tem,&len);
    memcpy(g_Vin,tem,17);
    len = 0;
    WorkFlashVehicleInforRead(E_PARAMETER_INFO_SECOC_KEY_F1C7,tem,&len);       

   /* if(tem[0] == tem[1] == tem[2] == tem[3] == tem[4] == 
       tem[5] == tem[6] == tem[7] == tem[8] == tem[9] == 
       tem[10] == tem[11] == tem[12] == tem[13] == tem[14] == 
       tem[15])
    {
        SetDtcFaultState(DTC_ITEM_SECOC_KEY_MISSING);
    }
    else
    {
        ClearDtcFaultState(DTC_ITEM_SECOC_KEY_MISSING);
    }*/
    //-----------------------------------------------
	if(IsSecocKeyValid(tem)==0)
	{		
		SecOC_Algorithm1(tem,g_Vin,pKey);
        *pKeyLength = 16;
		return 0;
	}
	else
	{
		return -1;
	}
}

void SecocSaveTripCount(uint32_t value)
{
    uint8_t tem[8];
    memcpy(tem,&value,4);
    WorkFlashUserInfoStore(E_SECOC_TRIP_COUNT,tem,4);
}

void SecocReadTripCount(uint32_t* pValue)
{
    uint8_t tem[8];
    uint32_t len;
    
    // WorkFlashUserInfoRead(E_SECOC_TRIP_COUNT,tem,&len);
    WorkFlashVehicleInforRead(E_PARAMETER_INFO_SECOC_TRIP_COUNT, tem, &len);
    if(len>=4)
    {
        memcpy(pValue,tem,4);
    }
    else
    {
        *pValue = 0;
    }    
}

static uint8_t GetDefaultMacValueCode(void)
{
	uint8_t value;
	value = g_Vin[14]^g_Vin[15]^g_Vin[16];
	return value;//
}

void SetDefaultMacCodeFun(uint8_t *pDefaultMac,uint8_t macLength)
{
    memset(pDefaultMac,GetDefaultMacValueCode(),macLength);
}

void AesCalculateFun(uint8_t* pAuthData,uint32_t authLength,uint8_t *pKey,uint8_t *pResultOut)
{
#if(0)
    Aes128Cmac_t aes128Object;
    Aes128CmacLoadMacKey(&aes128Object,pKey);
    Aes128CmacGenerateMAC(&aes128Object,authLength,pAuthData,pResultOut);
#else    
    CryptoHalAesCmacEncrypt(pKey,16,pAuthData,authLength,pResultOut);
#endif    
}

void SecocErrorSyncMsgMacCheckCallBack(uint8_t flag)
{
    /*if(flag)
    {
        //SetDtcFaultState(DTC_ITEM_SECOC_GW_PDU_FAIL);
        SetDtcFaultState(DTC_ITEM_SECOC_FV_ERROR);
    }
    else
    {
        //ClearDtcFaultState(DTC_ITEM_SECOC_GW_PDU_FAIL);
        ClearDtcFaultState(DTC_ITEM_SECOC_FV_ERROR);
    }*/
}

static SecocMsgBuffer_t g_txSecocMsgBuffer[sizeof(g_txSecocMsgConfigList)/sizeof(g_txSecocMsgConfigList[0])];
static SecocMsgBuffer_t g_rxSecocMsgBuffer[sizeof(g_rxSecocMsgConfigList)/sizeof(g_rxSecocMsgConfigList[0])];
const SecocSdkConfig_t g_SecocSdkConfig = 
{
    .CAN_FV_TruncLength = 8,
    .CAN_MAC_TruncLength = 8,
    .CANFD_FV_TruncLength = 8,
    .CANFD_MAC_TruncLength = 8,
    .SecocFreshnessValueSyncAttempts = 3,
    .syncMsgRequestId = 0x463,
    .pTxSecocMsgConfig = g_txSecocMsgConfigList,
    .pRxSecocMsgConfig = g_rxSecocMsgConfigList,
    .pTxMsgBuffer = g_txSecocMsgBuffer,
    .pRxMsgBuffer = g_rxSecocMsgBuffer,
    .txSecocMsgNum = sizeof(g_txSecocMsgConfigList)/sizeof(g_txSecocMsgConfigList[0]),
    .rxSecocMsgNum = sizeof(g_rxSecocMsgConfigList)/sizeof(g_rxSecocMsgConfigList[0]),
    .pGetSecocKeyFun = SecocGetKey,
    .pAesCalculateFun = AesCalculateFun,
    .pSetDefaultMacCodeFun = SetDefaultMacCodeFun,
    .pSaveTripCountFun = SecocSaveTripCount,
    .pReadTripCountFun = SecocReadTripCount,    
    .pMsgMacCheckErrorFun = SecocErrorSyncMsgMacCheckCallBack,
};

void ProjectSecocConfig(void)
{
     SecocSdkInit(&g_SecocSdkConfig);
}