#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

#include "canHal.h"
#include "logHal.h"

#include "secocSdk.h"

#define TRIP_COUNT_INIT_VAL			0x00L
#define TRIP_COUNT_MAX_VAL			0xFFFFFFL
#define TRIP_COUNT_WINDOW_VAL		0x03L
#define RESET_COUNT_INIT_VAL		0x00L
#define RESET_COUNT_MAX_VAL			0xFFFFL
#define CAN_MGS_COUNT_INIT_VAL	0x00L

typedef struct
{
	//uint8_t authFailedCount;
	uint8_t authFailedMax;
	uint8_t syncMsgValid;
	uint8_t syncMsgRequestSendFlag;
	uint8_t syncMsgRequestSendStart;
	uint8_t syncMsgRequesCount;	
	uint8_t syncMsgCheckFailedCount;//MAC验证失败计数，要求大于9帧后记录DTC
	uint8_t secocWakeCheck;
    uint8_t syncMsgCheckFvFailedCount;//新鲜度值同步失败计数
	uint32_t syncMsgtimeoutCount;
    uint32_t canHandle;
}SecocManage_t;

typedef struct
{
	uint32_t tripCnt;//24bit
	uint32_t resetCnt;//16bit
}FreshVInfo_t;

typedef struct
{
	uint8_t key[16];
	uint8_t valid;
}SecocKey_t;


static SecocManage_t g_secocManage;
static FreshVInfo_t g_freshVInfo;
static SecocKey_t g_secocKey;
static SemaphoreHandle_t g_secocMutexHandle = NULL;
static const SecocSdkConfig_t* g_pSecocConfig = NULL;
static uint8_t g_enableSecocMsgSendFlag = 1;



static uint32_t ReadTripCountFromNVM(void)
{
	uint32_t value;
	
	value = 0;	
    if(g_pSecocConfig!=NULL)
    {
        g_pSecocConfig->pReadTripCountFun(&value);
        if(value>TRIP_COUNT_MAX_VAL)//invalid count,and return init value
        {
            return TRIP_COUNT_INIT_VAL;
        }        
    }
	return value;
}

static void SaveTripCountToNVM(uint32_t value)
{
    if(g_pSecocConfig!=NULL)
    {
        g_pSecocConfig->pSaveTripCountFun(value);
    }
}

//void SecOC_Init(const SecOC_ConfigType* config)
int16_t SecocSdkInit(const SecocSdkConfig_t* pSecocConfig)
{
	uint16_t i;
	uint16_t size;
	uint32_t len;
    
    if(pSecocConfig==NULL)
    {
        return SECOC_STATUS_ERR;
    }
    if(pSecocConfig->pTxMsgBuffer==NULL)
    {
        return SECOC_STATUS_ERR;
    }
    if(pSecocConfig->pRxMsgBuffer==NULL)
    {
        return SECOC_STATUS_ERR;
    }
    g_pSecocConfig = pSecocConfig;
	size = g_pSecocConfig->txSecocMsgNum;
	for(i=0;i<size;i++)
	{
		g_pSecocConfig->pTxMsgBuffer[i].msgCount = CAN_MGS_COUNT_INIT_VAL+1;
	}
	size = g_pSecocConfig->rxSecocMsgNum;
	for(i=0;i<size;i++)
	{
		g_pSecocConfig->pRxMsgBuffer[i].msgCount = CAN_MGS_COUNT_INIT_VAL+1;
	}    
	//
	//g_secocManage.authFailedCount = 0;
	g_secocManage.authFailedMax = 3;
	g_secocManage.syncMsgValid = 0;
	g_secocManage.syncMsgRequestSendFlag = 0;
	g_secocManage.syncMsgRequestSendStart = 0;
    g_secocManage.syncMsgCheckFvFailedCount = 0;
	g_secocManage.syncMsgCheckFailedCount = 0;
	g_secocManage.secocWakeCheck = 0;
	//
	g_freshVInfo.tripCnt = ReadTripCountFromNVM();
	g_freshVInfo.resetCnt = RESET_COUNT_INIT_VAL;
	//
	//g_secocInitState = SECOC_INIT;
    len = 0;
    if(g_pSecocConfig->pGetSecocKeyFun!=NULL)
    {
        if(g_pSecocConfig->pGetSecocKeyFun(g_secocKey.key,&len)==0)
        {
            g_secocKey.valid = 1;
        }
        else
        {
            g_secocKey.valid = 0;
        }        
    }
    else
    {
        g_secocKey.valid = 0;
    }
	g_secocMutexHandle = xSemaphoreCreateMutex();
    return SECOC_STATUS_OK;
}

void SecocSdkReInit(void)
{
	uint16_t i;
	uint16_t size;
	uint32_t len;
	size = g_pSecocConfig->txSecocMsgNum;
	for(i=0;i<size;i++)
	{
		g_pSecocConfig->pTxMsgBuffer[i].msgCount = CAN_MGS_COUNT_INIT_VAL+1;
	}
	size =g_pSecocConfig->rxSecocMsgNum;
	for(i=0;i<size;i++)
	{
		g_pSecocConfig->pRxMsgBuffer[i].msgCount = CAN_MGS_COUNT_INIT_VAL+1;
	}
	//
	//g_secocManage.authFailedCount = 0;
	g_secocManage.authFailedMax = 3;
	g_secocManage.syncMsgValid = 0;
	g_secocManage.syncMsgRequestSendFlag = 0;
	g_secocManage.syncMsgRequestSendStart = 0;
	g_secocManage.syncMsgCheckFailedCount = 0;
    g_secocManage.syncMsgCheckFvFailedCount = 0;
	g_secocManage.secocWakeCheck = 0;
	//WorkFlashReadSecocTripCnt()
	g_freshVInfo.tripCnt = ReadTripCountFromNVM();
	g_freshVInfo.resetCnt = RESET_COUNT_INIT_VAL;
	//
	//g_secocInitState = SECOC_INIT;
    len = 0;
    if(g_pSecocConfig->pGetSecocKeyFun!=NULL)
    {
        if(g_pSecocConfig->pGetSecocKeyFun(g_secocKey.key,&len)==0)
        {
            g_secocKey.valid = 1;
        }
        else
        {
            g_secocKey.valid = 0;
        }        
    }
    else
    {
        g_secocKey.valid = 0;
    }
}

/**************************
flag:
0: check passed
1: check failed

**************************/
void ErrorSyncMsgMacCheckCallBack(uint8_t flag)
{
    if(g_pSecocConfig!=NULL)
    {
        if(g_pSecocConfig->pMsgMacCheckErrorFun!=NULL)
        {
            g_pSecocConfig->pMsgMacCheckErrorFun(flag);
        }        
    }
}

void UpdateFreshVByRxMsgInfo(int16_t idIndex,uint8_t msgCountH,uint8_t data0,uint8_t data1)
{
	uint32_t tmp;
	tmp = (data0<<8)+data1;
	tmp = tmp>>2;
    if((g_pSecocConfig!=NULL) && (idIndex >= 0))
    {
        g_pSecocConfig->pRxMsgBuffer[idIndex].msgCount = tmp;
    }
}

int16_t CalcuteNewFreshValue(SecocMsgBuffer_t *pMsgBufer,uint16_t currentMsgCntL,int8_t currentMsgRstCntL,uint32_t *pMsgCntOut,uint32_t *pTripCntOut,uint16_t *pRstCntOut)
{
	int8_t rstCntL;
	int16_t result;
	uint64_t tripRstCnt;
	uint64_t tripRstcntLast;
	//uint16_t rstCnt;

	rstCntL = g_freshVInfo.resetCnt&0x03;
	result = 0;
	tripRstCnt = (g_freshVInfo.tripCnt<<16)+g_freshVInfo.resetCnt;
	tripRstcntLast = (pMsgBufer->tripCnt<<16)+pMsgBufer->resetCnt;
	if(rstCntL==currentMsgRstCntL)
	{				
		
	}
	else if((rstCntL-1)==currentMsgRstCntL)
	{
		tripRstCnt -= 1;
	}
	else if((rstCntL+1)==currentMsgRstCntL)
	{
		tripRstCnt += 1;
	}
	else if((rstCntL-2)==currentMsgRstCntL)
	{
		tripRstCnt -= 2;
	}
	else if((rstCntL+2)==currentMsgRstCntL)
	{
		tripRstCnt += 2;
	}
	else
	{
		//get fresh value failed
		return -1;
	}
	if(tripRstCnt==tripRstcntLast)
	{
		if(currentMsgCntL>(pMsgBufer->msgCount&0x3FFF))//no carry
		{
			//
			*pMsgCntOut = (pMsgBufer->msgCount&0x3FC000)|currentMsgCntL;
		}
		else //carrry
		{
			*pMsgCntOut = ((pMsgBufer->msgCount&0x3FC000)+0x4000)|currentMsgCntL;
		}
		*pTripCntOut = pMsgBufer->tripCnt;
		*pRstCntOut = pMsgBufer->resetCnt;
	}
	else if(tripRstCnt>tripRstcntLast)
	{
		//H = 0
		*pMsgCntOut = currentMsgCntL;
		*pTripCntOut = g_freshVInfo.tripCnt;
		//*pRstCntOut = tripRstCnt&0xFF;
        *pRstCntOut = tripRstCnt&0xFFFF;
	}
	else
	{
		return -1;
	}
	return result;
}

void SecocSdkSetTripCount(uint32_t tripCountValue)
{	
	g_freshVInfo.tripCnt = tripCountValue;
	SaveTripCountToNVM(tripCountValue);
}
uint32_t SecocSdkGetTripCount(void)
{
    return g_freshVInfo.tripCnt;
}

void SecocSdkSetRestCount(uint32_t resetCountValue)
{
	int16_t i;
	uint16_t size;
	g_freshVInfo.resetCnt = resetCountValue;
	/*size = sizeof(g_pSecocConfig->pRxMsgBuffer)/sizeof(g_pSecocConfig->pRxMsgBuffer[0]);
	for(i=0;i<size;i++)
	{
		g_pSecocConfig->pRxMsgBuffer[i].msgCount = CAN_MGS_COUNT_INIT_VAL;
	}	*/
    if(g_pSecocConfig!=NULL)
    {
        size = g_pSecocConfig->txSecocMsgNum;
        for(i=0;i<size;i++)
        {
            g_pSecocConfig->pTxMsgBuffer[i].msgCount = CAN_MGS_COUNT_INIT_VAL+1;
        }	        
    }
}
uint32_t SecocSdkGetRestCount(void)
{
    return g_freshVInfo.resetCnt;
}
void IncreaseRestCount(void)
{
	int16_t i,size;	
    if(g_pSecocConfig!=NULL)
    {
        g_freshVInfo.resetCnt++;
        size = g_pSecocConfig->rxSecocMsgNum;
        for(i=0;i<size;i++)
        {
            g_pSecocConfig->pRxMsgBuffer[i].msgCount = 0;
        }	
        size = g_pSecocConfig->txSecocMsgNum;
        for(i=0;i<size;i++)
        {
            g_pSecocConfig->pTxMsgBuffer[i].msgCount = 0;
        }	        
    }
}

void ClearRxMsgCount(void)
{
	
}

int16_t SecocSdkTransmit(int16_t canHandle,uint32_t canId, const uint8_t *pTxData,uint16_t dlc,uint8_t fdFlag)
{
	int16_t i;
	int16_t idIndex;
	int16_t size;
	uint8_t tmp[100];
	uint16_t authLen;
	uint8_t macData[16];
	int16_t ret;
	const SecocSdkMsg_t *pTxMsgConfig;
    if(g_pSecocConfig==NULL)
    {
        return SECOC_STATUS_ERR;
    }
	size = g_pSecocConfig->txSecocMsgNum;
	idIndex = -1;
	for(i=0;i<size;i++)
	{
		if(g_pSecocConfig->pTxSecocMsgConfig[i].canId == canId)
		{
			idIndex = i;
			pTxMsgConfig = g_pSecocConfig->pTxSecocMsgConfig+i;
			break;
		}
	}
	//confirm can id is auth msg,else return error
	if(idIndex<0)
	{
		CanHalTransmitQueued(canHandle,canId,pTxData,dlc,fdFlag, CAN_TX_PRIO_NORMAL);//不带SecOC的CANFD报文
		return SECOC_STATUS_OK;
	}
	/*if(PduInfoPtr->dlc!=pTxMsgConfig->AuthStartByte)
	{
		return SECOC_STATUS_ERR;
	}*/
	xSemaphoreTake(g_secocMutexHandle, portMAX_DELAY);
	
	tmp[0] = (pTxMsgConfig->canId>>8)&0xFF;
	tmp[1] = (pTxMsgConfig->canId>>0)&0xFF;
	memcpy(tmp+2,pTxData,pTxMsgConfig->authStartByte);
	
	//calculate fresh value
	tmp[pTxMsgConfig->authStartByte+2] = (g_freshVInfo.tripCnt>>16)&0xFF;
	tmp[pTxMsgConfig->authStartByte+3] = (g_freshVInfo.tripCnt>>8)&0xFF;
	tmp[pTxMsgConfig->authStartByte+4] = (g_freshVInfo.tripCnt>>0)&0xFF;
	//
	tmp[pTxMsgConfig->authStartByte+5] = (g_freshVInfo.resetCnt>>8)&0xFF;
	tmp[pTxMsgConfig->authStartByte+6] = (g_freshVInfo.resetCnt>>0)&0xFF;
	//
	tmp[pTxMsgConfig->authStartByte+7] = (g_pSecocConfig->pTxMsgBuffer[idIndex].msgCount>>14)&0xFF;
	tmp[pTxMsgConfig->authStartByte+8] = (g_pSecocConfig->pTxMsgBuffer[idIndex].msgCount>>6)&0xFF;
	tmp[pTxMsgConfig->authStartByte+9] = (g_freshVInfo.resetCnt&0x03)|((g_pSecocConfig->pTxMsgBuffer[idIndex].msgCount<<2)&0xFC);
	//
    if(g_secocKey.valid)
	{
 		//fresh value trunn
		tmp[pTxMsgConfig->authStartByte+2] = tmp[pTxMsgConfig->authStartByte+8];
		tmp[pTxMsgConfig->authStartByte+3] = tmp[pTxMsgConfig->authStartByte+9];	 
        authLen = 2+ pTxMsgConfig->authStartByte + 8;
        if(g_pSecocConfig->pAesCalculateFun!=NULL)
        {
            g_pSecocConfig->pAesCalculateFun(tmp,authLen,g_secocKey.key,macData);
            //mac
            memcpy(tmp+pTxMsgConfig->authStartByte+4,macData,6);	
        }
        else
        {
            if(g_pSecocConfig->pSetDefaultMacCodeFun!=NULL)
            {
                g_pSecocConfig->pSetDefaultMacCodeFun(tmp+pTxMsgConfig->authStartByte+4,6);
            }
        }		
	}
	else		
	{
		//fresh value trunn
		tmp[pTxMsgConfig->authStartByte+2] = tmp[pTxMsgConfig->authStartByte+8];
		tmp[pTxMsgConfig->authStartByte+3] = tmp[pTxMsgConfig->authStartByte+9];	
		//mac
        if(g_pSecocConfig->pSetDefaultMacCodeFun!=NULL)
        {
            g_pSecocConfig->pSetDefaultMacCodeFun(tmp+pTxMsgConfig->authStartByte+4,6);
        }
	}
//	CanDriverHalTransmitFd(canHandle,canId,tmp+2,pTxMsgConfig->AuthStartByte+8,1);
//	m_txMsgBuffer[idIndex].msgCount++;
	ret = CanHalTransmitQueued(canHandle,canId,tmp+2,dlc,fdFlag, CAN_TX_PRIO_NORMAL);
	if(0 == ret)
    {
        g_pSecocConfig->pTxMsgBuffer[idIndex].msgCount++;
    }    
	xSemaphoreGive(g_secocMutexHandle);
	return SECOC_STATUS_OK;
}


int16_t SecocSdkReceive(int16_t canHandle,CanHalMsg_t *pMsg,int32_t timeOut_ms)
{
	int16_t ret;
	int16_t i;
	int16_t size;
	int16_t authLen;
	int16_t idIndex;
	CanHalMsg_t msg;
	uint8_t tmp[100];
	uint8_t macData[16];
	uint32_t tripCnt;
	uint16_t resetCnt;
	uint32_t msgCnt;
	uint8_t attemCnt;
	uint8_t checkMacValid;
	ret = CanHalReceive(canHandle,&msg,0);
	if(ret!=0)
	{
		return SECOC_STATUS_ERR;
	}
    if(g_pSecocConfig==NULL)
    {
        return SECOC_STATUS_OK;
    }
    if(g_pSecocConfig->pRxSecocMsgConfig==NULL)
    {
        return SECOC_STATUS_OK;
    }
	size = g_pSecocConfig->rxSecocMsgNum;
	idIndex = -1;
	for(i=0;i<size;i++)
	{
		if(g_pSecocConfig->pRxSecocMsgConfig[i].canId==msg.canId)
		{
			idIndex = i;
			break;
		}
	}
	if(idIndex<0)
	{
		memcpy(pMsg,&msg,sizeof(CanHalMsg_t));
		return SECOC_STATUS_OK;
	}
	/*if(msg.dlc!=g_pSecocConfig->pRxSecocMsgConfig[idIndex].AuthStartByte+8)
	{
		return SECOC_STATUS_ERR;
	}*/
	//wait freshvalue is valid
#if(0)
	if(g_secocManage.syncMsgValid==0)
	{
		for(i=0;i<200;i++)
		{
			RTOS_HalApiWait(10);
			if(g_secocManage.syncMsgValid)
			{
				break;
			}
		}
	}	
#else
	if(g_secocManage.syncMsgValid==0)
	{
		return SECOC_STATUS_ERR;
	}
#endif
	//msgCnt = ((msg.canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].AuthStartByte]<<8)|(msg.canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].AuthStartByte+1]))>>2;	
	//UpdateRxMsgCountByL(idIndex,msgCnt);
	//
	xSemaphoreTake(g_secocMutexHandle, portMAX_DELAY);
	
	tmp[0] = (msg.canId>>8)&0xFF;
	tmp[1] = (msg.canId>>0)&0xFF;
	memcpy(tmp+2,msg.canData,g_pSecocConfig->pRxSecocMsgConfig[i].authStartByte);
	//calculate fresh value
	CalcuteNewFreshValue(g_pSecocConfig->pRxMsgBuffer+idIndex,((msg.canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte]<<6)+(msg.canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+1]>>2)),msg.canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+1]&0x03,&msgCnt,&tripCnt,&resetCnt);
	
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+2] = (tripCnt>>16)&0xFF;
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+3] = (tripCnt>>8)&0xFF;
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+4] = (tripCnt>>0)&0xFF;
	//
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+5] = (resetCnt>>8)&0xFF;
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+6] = (resetCnt>>0)&0xFF;
	//
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+7] = (msgCnt>>16)&0xFF;
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+8] = msg.canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte];
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+9] = msg.canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+1];
	//
	if(g_secocKey.valid)
	{
		checkMacValid = 1;
		attemCnt = 0;
		do
		{
			authLen = 2+ g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte + 8;
            if(g_pSecocConfig->pAesCalculateFun!=NULL)
            {
                g_pSecocConfig->pAesCalculateFun(tmp,authLen,g_secocKey.key,macData);
            }
			for(i=0;i<6;i++)
			{
				if(macData[i]!=msg.canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+2+i])
				{					
					checkMacValid = 0;
					break;										
				}
			}
			if(checkMacValid)
			{
				if(attemCnt>0)
				{
					//UpdateFreshVByRxMsgInfo(idIndex,attemCnt,tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].AuthStartByte+8],tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].AuthStartByte+9]);
				}
				break;
			}
			else
			{
				attemCnt++;
				if(attemCnt<g_secocManage.authFailedMax)
				{
					tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+7]++;//high of msg count ++to try
					//continue;
				}
				else
				{
					xSemaphoreGive(g_secocMutexHandle);
					return SECOC_STATUS_ERR;
				}
				
			}
		}while(1);			
	}	
	memcpy(pMsg,&msg,sizeof(CanHalMsg_t));
	xSemaphoreGive(g_secocMutexHandle);
	return SECOC_STATUS_OK;
}


int16_t SecocSdkCheckPdu(CanHalMsg_t *pMsg)
{
	//int16_t ret;
	int16_t i;
	int16_t size;
	int16_t authLen;
	int16_t idIndex;
	//CanHalMsg_t msg;
	uint8_t tmp[80];
	uint8_t macData[16];
	uint32_t tripCnt;
	uint16_t resetCnt;
	uint32_t msgCnt;
	//uint8_t attemCnt;
	uint8_t checkMacValid;
    //判断是不是SecOC报文
    if(g_pSecocConfig==NULL)
    {
        return SECOC_STATUS_OK;
    }
	size = g_pSecocConfig->rxSecocMsgNum;
	idIndex = -1;
	for(i=0;i<size;i++)
	{
		if(g_pSecocConfig->pRxSecocMsgConfig[i].canId==pMsg->canId)
		{
			idIndex = i;
			break;
		}
	}
	if(idIndex<0)//不是SecOC报文直接退出
	{
		return SECOC_STATUS_OK;
	}
//	if(g_secocManage.syncMsgValid==0)//没有收到新鲜度值同步消息直接退出
//	{
//		return SECOC_STATUS_ERR;
//	}
	xSemaphoreTake(g_secocMutexHandle, portMAX_DELAY);
	
    //获取PDU的数据标识符
	tmp[0] = (pMsg->canId>>8)&0xFF;
	tmp[1] = (pMsg->canId>>0)&0xFF;
    //获取PDU原始数据
	memcpy(tmp+2,pMsg->canData,g_pSecocConfig->pRxSecocMsgConfig[i].authStartByte);
	//根据安全PDU中截取的新鲜度值获取完整的新鲜度值
	CalcuteNewFreshValue(g_pSecocConfig->pRxMsgBuffer+idIndex,((pMsg->canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte]<<6)+(pMsg->canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+1]>>2)),pMsg->canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+1]&0x03,&msgCnt,&tripCnt,&resetCnt);
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+2] = (tripCnt>>16)&0xFF;
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+3] = (tripCnt>>8)&0xFF;
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+4] = (tripCnt>>0)&0xFF;
	//
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+5] = (resetCnt>>8)&0xFF;
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+6] = (resetCnt>>0)&0xFF;
	//
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+7] = (msgCnt>>16)&0xFF;
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+8] = pMsg->canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte];
	tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+9] = pMsg->canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+1];
	//进行MAC校验
	if(g_secocKey.valid)
	{
		checkMacValid = 1;
        authLen = 2+ g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte + 8;
        if(g_pSecocConfig->pAesCalculateFun!=NULL)
        {
            g_pSecocConfig->pAesCalculateFun(tmp,authLen,g_secocKey.key,macData);
        }
        for(i=0;i<6;i++)
        {
            if(macData[i]!=pMsg->canData[g_pSecocConfig->pRxSecocMsgConfig[idIndex].authStartByte+2+i])
            {					
                checkMacValid = 0;
                break;										
            }
        }
        if(checkMacValid)//校验成功
        {
//            if(attemCnt>0)
//            {
//                //UpdateFreshVByRxMsgInfo(idIndex,attemCnt,tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].AuthStartByte+8],tmp[g_pSecocConfig->pRxSecocMsgConfig[idIndex].AuthStartByte+9]);
//            }
            
            /*if(msg.canId == 0x27F)
            {
                DtcCounter[0]=0;
                ClearDtcFaultState(DTC_ITEM_SECOC_HUT_PDU_FAIL);
            }
            else if(msg.canId == 0x319)
            {
                DtcCounter[1]=0;
                ClearDtcFaultState(DTC_ITEM_SECOC_GW_PDU_FAIL);
            }
            else if(msg.canId == 0x345)
            {
                DtcCounter[2]=0;
                ClearDtcFaultState(DTC_ITEM_SECOC_GW_PDU_FAIL);
            }
            else if(msg.canId == 0x19C)
            {
                DtcCounter[3]=0;
                ClearDtcFaultState(DTC_ITEM_SECOC_GW_PDU_FAIL);
            }
            else if(msg.canId == 0x341)
            {
                DtcCounter[4]=0;
                ClearDtcFaultState(DTC_ITEM_SECOC_GW_PDU_FAIL);
            }
            else if(msg.canId == 0x395)
            {
                DtcCounter[5]=0;
                ClearDtcFaultState(DTC_ITEM_SECOC_GW_PDU_FAIL);
            }*/
        }
        else//校验失败
        {
            //默认码判定
            checkMacValid = 1;
            if(g_pSecocConfig->pSetDefaultMacCodeFun!=NULL)
            {
                g_pSecocConfig->pSetDefaultMacCodeFun(macData,6);
            }            
            for(i=10;i<16;i++)
            {
                if(macData[i-10]!=pMsg->canData[i])
                {
                    checkMacValid = 0;
                    break;
                }
            }
            if(1 == checkMacValid)
            {                
                xSemaphoreGive(g_secocMutexHandle);
                return SECOC_STATUS_ERR;
            }            
            xSemaphoreGive(g_secocMutexHandle);
            return SECOC_STATUS_ERR;
        }			
	}
	xSemaphoreGive(g_secocMutexHandle);
	return SECOC_STATUS_OK;
}

static uint8_t SecocSdkSyncMessageReceiveMacCheck(uint32_t canId,uint8_t *pCanData,uint8_t *macData)
{
    uint8_t macValid;
    uint8_t tmp[64];
    uint16_t authLen;
    //消息计数器的安全PDU校验
	tmp[0] = (canId>>8)&0xFF;
	tmp[1] = (canId>>0)&0xFF;
	tmp[2] = pCanData[0];
	tmp[3] = pCanData[1];
	tmp[4] = pCanData[2];
	tmp[5] = pCanData[3];
	tmp[6] = pCanData[4];
    authLen = 2+ 3 + 2;
    if(g_pSecocConfig!=NULL)
    {
        if(g_pSecocConfig->pAesCalculateFun!=NULL)
        {
            g_pSecocConfig->pAesCalculateFun(tmp,authLen,g_secocKey.key,macData);
        }        
    }
	macValid = 1;
	for(uint8_t i=0;i<11;i++)
	{
		if(macData[i]!=pCanData[i+5])
		{
			macValid = 0;
			break;
		}
	}
    return macValid;
}

void SecocSdkSyncMessageReceiveProcess(uint32_t canId,uint8_t *pCanData)
{
	//tripCnt:byte0~2;resetCnt:byte3~4;
	//mac:byte5~byte15
	uint8_t macData[16];
	uint16_t i;
	uint8_t macValid;
	uint32_t tripCnt;
	uint16_t restCnt;
//	if(0==g_secocManage.syncMsgValid)
//	{
//		g_secocManage.syncMsgValid = 1;
//		g_secocManage.syncMsgRequestSendFlag = 0;
//		g_secocManage.syncMsgtimeoutCount = 0;		
//	}
	xSemaphoreTake(g_secocMutexHandle, portMAX_DELAY);
	/*
    在文档《附录28：SecOC Cyber Security design Requirement Specification_V1.1.1》
    第53页要求：连续9帧MAC校验失败和新鲜度值同步消息失败，记录DTC
    */
    tripCnt = (pCanData[0]<<16)+(pCanData[1]<<8)+(pCanData[2]<<0);//同步计数器
	restCnt = (pCanData[3]<<8)+(pCanData[4]<<0);//重置计数器
	if(tripCnt<g_freshVInfo.tripCnt)
	{
		if(g_freshVInfo.tripCnt<(TRIP_COUNT_MAX_VAL-TRIP_COUNT_WINDOW_VAL)||(tripCnt>TRIP_COUNT_INIT_VAL+TRIP_COUNT_WINDOW_VAL))
		{
			//discard
            TBOX_PRINT("discard，FailedCount = %d\n",g_secocManage.syncMsgCheckFailedCount);
            //
            if(g_secocManage.syncMsgCheckFailedCount < 8)//当为8的时候，是第9次记录
            {
                g_secocManage.syncMsgCheckFailedCount++;
            }
            else
            {
                //TBOX_PRINT("set FV DTC\n");
                ErrorSyncMsgMacCheckCallBack(1);
            }
            //一次3帧同步失败，请求再次发送新鲜度值
            if(g_secocManage.syncMsgCheckFailedCount >= 3)
            {
                //SecocSdkStartSyncMsgRequest();
            }
            
            xSemaphoreGive(g_secocMutexHandle);
			return;
		}		
	}
	if((tripCnt==g_freshVInfo.tripCnt)&&(restCnt<=g_freshVInfo.resetCnt))
	{		
		//discard
        TBOX_PRINT("discard，FailedCount = %d\n",g_secocManage.syncMsgCheckFailedCount);
        if(g_secocManage.syncMsgCheckFailedCount < 8)
        {
            g_secocManage.syncMsgCheckFailedCount++;
        }
        else
        {
            //TBOX_PRINT("set FV DTC\n");
            ErrorSyncMsgMacCheckCallBack(1);
        }
        //一次3帧同步失败，请求再次发送新鲜度值
        if(g_secocManage.syncMsgCheckFailedCount >= 3)
        {
            //SecocSdkStartSyncMsgRequest();
        }
        xSemaphoreGive(g_secocMutexHandle);
		return;		
	}
    //g_secocManage.syncMsgCheckFvFailedCount = 0;
/*
    //消息计数器的安全PDU校验
	tmp[0] = (canId>>8)&0xFF;
	tmp[1] = (canId>>0)&0xFF;
	tmp[2] = pCanData[0];
	tmp[3] = pCanData[1];
	tmp[4] = pCanData[2];
	tmp[5] = pCanData[3];
	tmp[6] = pCanData[4];
    authLen = 2+ 3 + 2;
    if(g_pSecocConfig!=NULL)
    {
        if(g_pSecocConfig->pAesCalculateFun!=NULL)
        {
            g_pSecocConfig->pAesCalculateFun(tmp,authLen,g_secocKey.key,macData);
        }        
    }
	macValid = 1;
	for(i=0;i<11;i++)
	{
		if(macData[i]!=pCanData[i+5])
		{
			macValid = 0;
			break;
		}
	}
*/
    macValid = SecocSdkSyncMessageReceiveMacCheck( canId,pCanData,macData);
	if(macValid==0)//MAC校验失败
	{
		macValid = 1;
        if(g_pSecocConfig->pSetDefaultMacCodeFun!=NULL)
        {
            g_pSecocConfig->pSetDefaultMacCodeFun(macData,11);
        }
		for(i=0;i<11;i++)
		{
			if(macData[i]!=pCanData[i+5])
			{
				macValid = 0;
				break;
			}
		}
		if(macValid==0)//默认MAC验证失败
		{	
            if(g_secocManage.syncMsgCheckFailedCount<8)
            {
                TBOX_PRINT("FailedCount = %d\n",g_secocManage.syncMsgCheckFailedCount);
                g_secocManage.syncMsgCheckFailedCount++;
            }
            else
            {
                //TBOX_PRINT("set FV DTC\n");
                ErrorSyncMsgMacCheckCallBack(1);
            }
            //一次3帧同步失败，请求再次发送新鲜度值
            if(g_secocManage.syncMsgCheckFailedCount >= 3)
            {
                SecocSdkStartSyncMsgRequest();
            }
			xSemaphoreGive(g_secocMutexHandle);
			return;
		}
        else//验证成功
        {
            g_secocManage.syncMsgCheckFailedCount = 0;
            ErrorSyncMsgMacCheckCallBack(0);
            TBOX_PRINT("default MAC\n");
        }
	}
	else
	{
		g_secocManage.syncMsgCheckFailedCount = 0;
		ErrorSyncMsgMacCheckCallBack(0);
	}
    TBOX_PRINT("tripCnt = %d,restCnt = %d\n",tripCnt,restCnt);
	if(0==g_secocManage.syncMsgValid)
	{
		g_secocManage.syncMsgValid = 1;
		g_secocManage.syncMsgRequestSendFlag = 0;
		g_secocManage.syncMsgtimeoutCount = 0;		
	}
	SecocSdkSetTripCount(tripCnt);
	SecocSdkSetRestCount(restCnt);
	xSemaphoreGive(g_secocMutexHandle);
}

static void TranmsitSyncMesseRequest(int16_t canHandle)
{
	uint32_t canId;
	uint8_t canData[16];
	if(g_pSecocConfig==NULL)
    {
        return;
    }
	canId = g_pSecocConfig->syncMsgRequestId;
	memset(canData,0x00,sizeof(canData));
	canData[0] = 0x11;
    
    if(g_enableSecocMsgSendFlag)
    {
        CanHalTransmitQueued(canHandle,canId,canData,16,3, CAN_TX_PRIO_NORMAL);
    }
}

void SecocSdkSyncMessageCycleProcess(int16_t canHandle,uint32_t cycleTime)
{
	if(1==g_secocManage.syncMsgRequestSendFlag)//请求发送标志，在SecocSdkStartSyncMsgRequest置1
	{
		if(g_secocManage.syncMsgRequestSendStart)//在SecocSdkStartSyncMsgRequest置1
		{
			g_secocManage.syncMsgRequestSendStart = 0;
			TranmsitSyncMesseRequest(canHandle);	
			g_secocManage.syncMsgRequesCount++;
		}
		else
		{
			g_secocManage.syncMsgtimeoutCount+= 10;
			if(g_secocManage.syncMsgtimeoutCount>=50)
			{
				g_secocManage.syncMsgtimeoutCount = 0;
				if(g_secocManage.syncMsgRequesCount<3)
				{
					//transmit sync msg request
					TranmsitSyncMesseRequest(canHandle);	
					g_secocManage.syncMsgRequesCount++;
				}
				else
				{
					g_secocManage.syncMsgRequestSendFlag = 0;				
				}					
			}
			else
			{
                if(1==g_secocManage.syncMsgValid)//收到了新鲜度值同步请求消息
                {
                    g_secocManage.syncMsgRequestSendFlag = 0;
                }	
			}
		}			
	}
	if(g_secocManage.secocWakeCheck)	
	{
		if(0==g_secocManage.syncMsgValid)
		{
			if(g_secocManage.syncMsgtimeoutCount<150)
			{
				g_secocManage.syncMsgtimeoutCount += 10;
			}
			else
			{
				g_secocManage.syncMsgtimeoutCount = 0;
				g_secocManage.syncMsgRequesCount = 0;
				g_secocManage.syncMsgRequestSendStart = 1;
				g_secocManage.syncMsgRequestSendFlag = 1;
				g_secocManage.secocWakeCheck = 0;
			}												
		}
		else
		{
			g_secocManage.secocWakeCheck = 0;			
		}		
	}			
}

void SecocSdkWakeUp(void)
{
	uint32_t tripCnt;
	uint16_t i,size;
	tripCnt = ReadTripCountFromNVM();
	xSemaphoreTake(g_secocMutexHandle, portMAX_DELAY);
    if(g_pSecocConfig!=NULL)
    {
        g_freshVInfo.tripCnt = tripCnt;
        g_freshVInfo.resetCnt = RESET_COUNT_INIT_VAL;    
        size = g_pSecocConfig->txSecocMsgNum;
        for(i=0;i<size;i++)
        {
            g_pSecocConfig->pTxMsgBuffer[i].msgCount = CAN_MGS_COUNT_INIT_VAL+1;
        }
        size = g_pSecocConfig->rxSecocMsgNum;
        for(i=0;i<size;i++)
        {
            g_pSecocConfig->pRxMsgBuffer[i].msgCount = CAN_MGS_COUNT_INIT_VAL;
        }	
        g_secocManage.syncMsgValid = 0;
        g_secocManage.secocWakeCheck = 0;         
    }
	xSemaphoreGive(g_secocMutexHandle);
}

uint8_t SecocSdkKeyGetKeyIsValid(void)
{
    return g_secocKey.valid;
}

/*********************************
wakeup,and send the first nm msg
*********************************/
void SecocSdkWakeupResetState(void)
{	
	xSemaphoreTake(g_secocMutexHandle, portMAX_DELAY);
	g_secocManage.syncMsgtimeoutCount = 0;
	g_secocManage.syncMsgRequesCount = 0;
	//g_secocManage.syncMsgRequestSendStart = 1;
	//g_secocManage.syncMsgRequestSendFlag = 1;	
	g_secocManage.secocWakeCheck = 1;
	xSemaphoreGive(g_secocMutexHandle);
	
}

/******************************
busoff recover
***********************/
void SecocSdkStartSyncMsgRequest(void)
{
	g_secocManage.syncMsgValid = 0;
	g_secocManage.syncMsgtimeoutCount = 0;
	g_secocManage.syncMsgRequesCount = 0;
	g_secocManage.syncMsgRequestSendStart = 1;
	g_secocManage.syncMsgRequestSendFlag = 1;	
}
/*
const uint8_t m_SecocVersion[] = "00.01";

void SecOcGetVersion(uint8_t *pVersion,uint8_t *pVersionLength)
{
	memcpy(pVersion,m_SecocVersion,5);
	*pVersionLength = 5;
}
*/
