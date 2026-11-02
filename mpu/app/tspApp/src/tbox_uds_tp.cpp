#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include "sr_log.h"
#include "err_code.h"



#include "tbox_uds_tp.h"


//#define _TBOX_UDS_TP_TP_DEBUG__ 
#ifdef _TBOX_UDS_TP_TP_DEBUG__
#define DEBUG(format,...) printf(format, ##__VA_ARGS__)
#else 
#define DEBUG(format,...) 
#endif 



//gint32 CTboxUdsTp::m_mcuServerKeyid = -1;



gint32 CTboxUdsTp::TransmitUdsService(guint8 counterIndex,guint8 *pRequestData,guint16 requestLength,guint8 *pResponseData,guint32 *pResponseLength)
{
	gint32 ret;
	gint32 repeatCount;
	gint8 repeatFlag;
	ret = TBOX_UDS_TP_RESULT_OK;
	guint8 subCommand;
	guint8 txData[5000];

	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	memset(&txData, 0x0, sizeof(txData));		

	memcpy(&txData[3],pRequestData,requestLength);


	repeatCount = 0;
	repeatFlag = 0;
	do
	{
		txData[0] = 0x00;//reserved
		txData[1] = counterIndex;
		txData[2] = 0x00;//reserved
		subCommand = 0x81;
		//printf("counter index is %d \r\n",counterIndex);
		//printf("transmit request,repeat count is %d\r\n",repeatCount);		
		//liuleiif(SendMcuMsg(MSG_QUEUE_UDS_TBOX, requestLength+3, subCommand, txData)!=RET_OK)
		{
			if(repeatCount>=3)
			{
				ret = TBOX_UDS_TP_RESULT_ERROR_TX;
				printf("transmit error\n");
				break;
			}

		}	

		guint8 msgBuffer[5000];
		MsgQueueData *pRecv = (MsgQueueData *)msgBuffer;
		guint8  *pServiceData = pRecv->data.msg.serverData;
		guint32 rxLength;

		do
		{
			repeatFlag = 0;		
			//printf("uart receive start\r\n");			
			if(GetMcuMsg(m_mcuServerKeyid, pRecv, TBOX_UDS_TP_RESPONSE_TIMEOUT) < RET_OK)
			{
				repeatCount++;
				if(repeatCount>=3)
				{
					ret = TBOX_UDS_TP_RESULT_ERROR_TX_TIMEOUT;
				}
				else
				{
					repeatFlag = 1;
				}	
				printf("uds receive error\n");					
				break;
			}
			else
			{
				subCommand = pRecv->data.msg.subcmd;
				rxLength = pRecv->data.msg.len;
				gint32 i;
				LOG_Debug("receive data subcommdn is %d,length is %d, data is",subCommand,rxLength);
				for(i=0;i<10;i++)
				{
					LOG_Debug(" %d",pServiceData[i]);
				}
				LOG_Debug("\r\n");

				if(subCommand==0x03)
				{
					if(rxLength<3)
					{
						ret = TBOX_UDS_TP_RESULT_ERROR_TX_FORMAT;
						break;
					}
					if(pServiceData[2]!=counterIndex)
					{
						continue;
					}
					if(pServiceData[1]==0x00)
					{
						//
						memcpy(pResponseData,pServiceData+3,rxLength-3);
						*pResponseLength = rxLength-3;
						ret = TBOX_UDS_TP_RESULT_OK;
					}
					else
					{
						ret = TBOX_UDS_TP_RESULT_ERROR_PARAMETER;
					}
					break;
				}
				else
				{
					repeatCount++;
					if(repeatCount>=3)
					{
						repeatFlag = 0;
						ret = TBOX_UDS_TP_RESULT_ERROR_TX_TIMEOUT;
						break;
					}
				}
			}			
		} while (1);

		if(0==repeatFlag)//operation finished
		{
			break;			
		}		

	}while(1);
	//printf("TransmitUdsRequest end\r\n");
	return ret;
	
}

gint32 CTboxUdsTp::ExcuteUdsSerivceRawData(guint8 *pTxData,guint32 txLength,guint8 *pRxData,guint32 *pRxLength)
{
	gint32 ret;
	guint8 counterIndex;
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);
	//counterIndex = m_udsTpTxObject[canChannel].udsCounterIndex++;
	counterIndex = 0x01;
	ret = TransmitUdsService(counterIndex,pTxData,txLength,pRxData,pRxLength);	
	return ret;
}


void CTboxUdsTp::PrintByteArray(guint8 *pData,guint32 length)
{
	guint32 i;
	printf("length is %d data is",length);
	for(i=0;i<length;i++)
	{
		printf(" %x",pData[i]);
	}
	printf("\r\n");
}

gint32 CTboxUdsTp::Initialize()
{
	gint32 ret;

	ret = 0;	
	return ret;
}

void CTboxUdsTp::UnInitialize()
{

}
/******************************************************************************
 * 
 * return 
 * 		>=0,valid tp handle
 * 		<0,invalid handle,(error code)
 *****************************************************************************/
gint32 CTboxUdsTp::Open(guint8 canChannel)
{
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);
	m_mcuServerKeyid = InitMcuMsg();
	DEBUG("-----%d\n",m_mcuServerKeyid);
	if (m_mcuServerKeyid == RET_ERR)
    {
        LOG_Error("Init Mcu Error\n");
        return -1;
    }

	//int32_t SubscriptionQueueMsg(SYSQueMsgType_e type, QueSubCallback callbackFuc, void* userdata);
	/*sub msg*/
    //liuleiif (SubscriptionMcuMsg(m_mcuServerKeyid, MSG_QUEUE_UDS_TBOX) != RET_OK)
    {
        LOG_Error("Sub msg error\n");
        return -2;
    }
	return 0;
}

void CTboxUdsTp::Close()
{
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);
	UnSubscriptionMcuMsg(m_mcuServerKeyid, MSG_QUEUE_UDS_TBOX);	
}

gint32 CTboxUdsTp::ServiceExecute(guint8 addressingMode,guint8 *pTxData,guint32 txLength,guint8 *pRxData,guint32 *pRxLength)
{
	gint32 result;
	result = ExcuteUdsSerivceRawData(pTxData,txLength,pRxData,pRxLength);
	return result;
}

CTboxUdsTp::CTboxUdsTp()
{
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);
	m_mcuServerKeyid = -1;
}

CTboxUdsTp::~CTboxUdsTp()
{

}






























