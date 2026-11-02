#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tbox_uds_tp.h"
#include "sr_log.h"
#include "err_code.h"
#include <glib.h>
#include "unistd.h"


#include "tbox_uds_service.h"


//#define _TBOX_UDS_DEBUG__ 
#ifdef _TBOX_UDS_DEBUG__
#define DEBUG(format,...) printf(format, ##__VA_ARGS__)
#else 
#define DEBUG(format,...) 
#endif 


//#define UDS_FLASH_31_STATUS_SUCCESS_CODE		0x00
#define UDS_FLASH_31_STATUS_SUCCESS_CODE m_udsFlash31StatusSuccessCode
#define TBOX_UDS_OBJECT_MAX			10
#define TBOX_UDS_CAN_CHANNEL_MAX		4



CTboxUdsService::CTboxUdsService()
{
	openFlag = 0x00;
	m_functionalAddressing = 0x00;
	m_udsFlash31StatusSuccessCode = 0x00;
	//printf("CTboxUdsService\r\n");
}

CTboxUdsService::~CTboxUdsService()
{
	//printf("~CTboxUdsService\r\n");
}

gint32 CTboxUdsService::Open(guint8 canChannel)
{
	gint32 udsHandle;
	gint32 result;
	
	DEBUG("----%s,%d", __FUNCTION__, __LINE__);
	if(openFlag)
	{
		return -1;
	}

	result = m_UdsTp.Open(canChannel);	
	if(result<0)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	openFlag = 0x01;
	return 0;
}

void  CTboxUdsService::Close()
{
	guint8 canChannel;
	gint32 udsIndex;
	DEBUG("----%s,%d", __FUNCTION__, __LINE__);

	m_UdsTp.Close();
	openFlag = 0x00;
}

gint32 CTboxUdsService::ExcuteSessionControl(ENUM_UDS_SESSION destinationSession)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x10;
	requestData[1] = destinationSession;
	requestDataLength = 0x02;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	//printf("uds service ExcuteSessionControl result is %d\r\n",result);
	//printf("data length is %d\r\n",responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		printf("CTboxUdsService::ExcuteSessionControl Tp error code is %d\r\n",result);
		if(result==TBOX_UDS_TP_RESULT_ERROR_ECU_TIMEOUT)
		{
			return TBOX_UDS_RESULT_ERROR_RESPONE_TIMEOUT;
		}
		else
		{
			return TBOX_UDS_RESULT_ERROR_TP;
		}		
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<2)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x10+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=destinationSession)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteGetSeed(guint8 securityLevel,guint8 *pSeed,guint32 *pSeedLength)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x27;
	requestData[1] = securityLevel;
	requestDataLength = 0x02;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<2)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x27+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=securityLevel)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	seedLength = responseDataLength-2;
	for(i=0;i<seedLength;i++)
	{
		pSeed[i] = responseData[i+2];
	}
	*pSeedLength = seedLength;
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteSetKey(guint8 securityLevel,guint8 *pKey,guint32 keyLength)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x27;
	requestData[1] = securityLevel;
	for(i=0;i<keyLength;i++)
	{
		requestData[i+2] = pKey[i];
	}
	requestDataLength = 0x02+keyLength;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<2)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x27+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=securityLevel)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}

	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteControlDtcSetting(guint8 controlFlag)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x85;
	requestData[1] = controlFlag;
	requestDataLength = 0x02;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<2)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x85+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=controlFlag)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}

	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteClearDtc(guint32 clearFlag)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x14;
	requestData[1] = (clearFlag>>16)&0xFF;
	requestData[2] = (clearFlag>>8)&0xFF;
	requestData[3] = (clearFlag>>0)&0xFF;
	requestDataLength = 0x04;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<1)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x14+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	// if(responseData[1]!=requestData[1])
	// {
	// 	return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	// }
	// if(responseData[2]!=requestData[2])
	// {
	// 	return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	// }
	// if(responseData[3]!=requestData[3])
	// {
	// 	return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	// }
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteCommunicationControl(guint8 controlType,guint8 commnicationType)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x28;
	requestData[1] = controlType;
	requestData[2] = commnicationType;
	requestDataLength = 0x03;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<2)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x28+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteControlTestPresent()
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x3E;
	requestData[1] = 0x80;
	requestDataLength = 0x02;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<2)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x3E + 0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteReadDtcListByStatusMask(guint8 statusMask,guint8 *pValidEcuMask,STRUCT_UDS_DTC *pDtc,guint32 *pDtcNum)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	guint32 dtcNum;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x19;
	requestData[1] = 0x02;
	requestData[2] = statusMask;	
	requestDataLength = 0x03;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<3)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x19+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	*pValidEcuMask = responseData[2];
	dtcNum = (responseDataLength-3)/4;
	for(i=0;i<dtcNum;i++)
	{
		pDtc[i].dtcCode = ((guint32)responseData[i*4+3]<<16)+((guint32)responseData[i*4+4]<<8)+((guint32)responseData[i*4+5]<<0);
		pDtc[i].status = responseData[i*4+6];
	}
	*pDtcNum = dtcNum;
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteReadDtcNumberByStatusMask(guint8 statusMask,guint8 *pValidEcuMask,guint32 *pDtcNum)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	guint32 dtcNum;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x19;
	requestData[1] = 0x01;
	requestData[2] = statusMask;	
	requestDataLength = 0x03;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<3)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x19+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	*pValidEcuMask = responseData[2];

	*pDtcNum = (responseData[4]<<8)+responseData[5];
	return TBOX_UDS_RESULT_OK;
}

  gint32 CTboxUdsService::ExcuteReadDtcSnapshotRecord(guint32 dtcCode,STRUCT_UDS_DTC_SNAPSHOT *pSnapshot,guint8 *pDtcStatusOut)
  {
	guint16 did;
	gint32 byteOffset;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[4096];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	guint32 NumOfIdentifier;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x19;
	requestData[1] = 0x04;
	requestData[2] = (dtcCode>>16)&0xFF;	
	requestData[3] = (dtcCode>>8)&0xFF;	
	requestData[4] = dtcCode&0xFF;	
	requestData[5] = 0x01;	
	requestDataLength = 0x06;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<3)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x19+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[7]!=0x05)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	NumOfIdentifier = responseData[7];
	byteOffset = 0x08;
	for(i=0;i<NumOfIdentifier;i++)
	{
		//did = (responseData[8]<<8)+responseData[9];
		did = (responseData[byteOffset]<<8)+responseData[byteOffset+1];
		if(0xF010==did)//global real time data identifier
		{
			memcpy(pSnapshot->globalRealTime,responseData+byteOffset+2,sizeof(pSnapshot->globalRealTime));
			byteOffset += 9;
		}
		else if(0xF011==did)//ecu voltage
		{
			pSnapshot->ecuVoltage = responseData[byteOffset+2];
			byteOffset += 3;
		}
		else if(0xF012==did)//motor speed
		{
			pSnapshot->motorSpeed = (responseData[byteOffset+2]<<8)+responseData[byteOffset+3];
			byteOffset += 4;
		}
		else if(0xF013==did)//vehile speed
		{
			pSnapshot->vehicleSpeed = (responseData[byteOffset+2]<<8)+responseData[byteOffset+3];
			byteOffset += 4;
		}
		else if(0xF014==did)//vehicle odometer
		{
			pSnapshot->vehicleOdlmeter = (responseData[byteOffset+2]<<24)+(responseData[byteOffset+3]<<16)+(responseData[byteOffset+4]<<8)+responseData[byteOffset+5];
			byteOffset += 6;
		}
		else
		{
			//break;
			return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
		}
		
	}
	*pDtcStatusOut = responseData[5];
	return TBOX_UDS_RESULT_OK;
  }

 gint32 CTboxUdsService::ExcuteReadDtcExtendedDataRecord(guint32 dtcCode,STRUCT_UDS_DTC_EXTENDED_DATA *pExtendedData,guint8 *pDtcStatusOut)
 {
	guint8 recordNum;
	gint32 byteOffset;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[4096];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	guint32 NumOfRecord;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x19;
	requestData[1] = 0x06;
	requestData[2] = (dtcCode>>16)&0xFF;	
	requestData[3] = (dtcCode>>8)&0xFF;	
	requestData[4] = dtcCode&0xFF;	
	requestData[5] = 0xFF;	
	requestDataLength = 0x06;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<3)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x19+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}

	NumOfRecord = (responseDataLength-6)/2;
	byteOffset = 0x06;
	for(i=0;i<NumOfRecord;i++)
	{
		recordNum = responseData[byteOffset];
		if(0x01==recordNum)//fault occurrence counter
		{
			pExtendedData->faultOccurrenceCounter = responseData[byteOffset+1];
			byteOffset += 2;
		}
		else if(0x02==recordNum)//fault pending counter
		{
			pExtendedData->faultPendingCounter = responseData[byteOffset+1];
			byteOffset += 2;
		}
		else if(0x03==recordNum)//dtc aging counter
		{
			pExtendedData->dtcAgingCounter = responseData[byteOffset+1];
			byteOffset += 2;
		}
		else if(0x04==recordNum)//dtc aged counter
		{
			pExtendedData->dtcAgedCounter = responseData[byteOffset+1];
			byteOffset += 2;
		}
		else
		{
			//break;
			//return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
			byteOffset += 2;
		}
		
	}
	*pDtcStatusOut = responseData[5];
	return TBOX_UDS_RESULT_OK;
 }

gint32 CTboxUdsService::ExcuteReadDataByIdentifier(guint16 did,guint8 *pDataOut,guint32 *pLengthOut)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	guint32 dataLength;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x22;
	requestData[1] = did>>8;
	requestData[2] = did;	
	requestDataLength = 0x03;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	LOG_Debug("uds data length is %d,",responseDataLength);
	LOG_Debug("uds data is");
	for(i=0;i<responseDataLength;i++)
	{
		LOG_Debug(" %d",responseData[i]);
	}
	LOG_Debug("\r\n");
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<3)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x22+0x40))
	{
		printf("data 0 invlid\r\n");
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		printf("data 1 invlid\r\n");
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[2]!=requestData[2])
	{
		printf("data 2 invlid\r\n");
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	dataLength = responseDataLength-3;
	for(i=0;i<dataLength;i++)
	{
		pDataOut[i] = responseData[i+3];
	}
	*pLengthOut = dataLength;
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteEcuReset(guint8 resetType)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x11;
	requestData[1] = resetType;
	requestDataLength = 0x02;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<2)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x11+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteTransferData(guint8 BlockSequenceCounter,guint8 *pFlashData,guint32 flashDataLength)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[5000];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x36;
	requestData[1] = BlockSequenceCounter;
	for(i=0;i<flashDataLength;i++)
	{
		requestData[2+i] = pFlashData[i];
	}
	requestDataLength = 0x02+flashDataLength;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		printf("ExcuteTransferData failed tp code is %d\r\n",result);
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<2)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x36+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteTransferExit()
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x37;
	requestDataLength = 0x01;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<1)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x37+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteRequestDownLoad(guint32 startAddress,guint32 uncompressedSize,guint32 *pMaxNumberOfBlockLength)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 seedLength;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);;

	requestData[0] = 0x34;
	requestData[1] = 0x00;
	requestData[2] = 0x44;	

	requestData[3] = startAddress>>24;
	requestData[4] = startAddress>>16;
	requestData[5] = startAddress>>8;	
	requestData[6] = startAddress>>0;	

	requestData[7] = uncompressedSize>>24;
	requestData[8] = uncompressedSize>>16;
	requestData[9] = uncompressedSize>>8;	
	requestData[10] = uncompressedSize>>0;	

	requestDataLength = 11;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<3)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x34+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]==0x10)
	{
		*pMaxNumberOfBlockLength = (responseData[2]<<0);
	}
	else if(responseData[1]==0x20)
	{
		*pMaxNumberOfBlockLength = (responseData[2]<<8)+(responseData[3]<<0);
	}
	else if(responseData[1]==0x30)
	{
		*pMaxNumberOfBlockLength = (responseData[2]<<16)+(responseData[3]<<8)+(responseData[4]<<0);
	}
	else if(responseData[1]==0x40)
	{
		*pMaxNumberOfBlockLength = (responseData[2]<<24)+(responseData[3]<<16)+(responseData[4]<<8)+(responseData[5]<<0);
	}
	else 
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}		
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteRoutineControlEraseFlash(guint32 startAddress,guint32 length)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);
	printf("start address is %x,length is %x\r\n",startAddress,length);

	requestData[0] = 0x31;
	requestData[1] = 0x01;
	requestData[2] = 0xFF;//did H
	requestData[3] = 0x00;//did L

	requestData[4] = 0x44;
	requestData[5] = startAddress>>24;	
	requestData[6] = startAddress>>16;
	requestData[7] = startAddress>>8;
	requestData[8] = startAddress>>0;

	requestData[9] = length>>24;	
	requestData[10] = length>>16;
	requestData[11] = length>>8;
	requestData[12] = length>>0;

	requestDataLength = 13;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		printf("UdsTp error code is %d",result);
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<3)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	/*printf("response length is %d \r\n",responseDataLength);
	printf("data 0  is %x \r\n",responseData[0]);
	printf("data 1  is %x \r\n",responseData[1]);
	printf("data 2  is %x \r\n",responseData[2]);
	printf("data 3  is %x \r\n",responseData[3]);
	printf("data 4  is %x \r\n",responseData[4]);*/
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseDataLength<5)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}	
	if(responseData[0]!=(0x31+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[2]!=requestData[2])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[3]!=requestData[3])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}	
	if(responseData[4]!=UDS_FLASH_31_STATUS_SUCCESS_CODE)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}	
	return TBOX_UDS_RESULT_OK;
}


gint32 CTboxUdsService::ExcuteRoutineControlCheckPreCondition()
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint16 did;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	did = 0x0203;
	requestData[0] = 0x31;
	requestData[1] = 0x01;
	requestData[2] = did>>8;//did H
	requestData[3] = did;//did L

	requestDataLength = 4;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<3)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseDataLength<5)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}	
	if(responseData[0]!=(0x31+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[2]!=requestData[2])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[3]!=requestData[3])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}	
	if(responseData[4]!=UDS_FLASH_31_STATUS_SUCCESS_CODE)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}	
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteRoutineControlCheckDependence(guint8 *pDependenceResult)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint16 did;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	did = 0xFF01;
	requestData[0] = 0x31;
	requestData[1] = 0x01;
	requestData[2] = did>>8;//did H
	requestData[3] = did;//did L

	requestDataLength = 4;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<3)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseDataLength<5)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}	
	if(responseData[0]!=(0x31+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[2]!=requestData[2])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[3]!=requestData[3])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}	
	if(responseData[4]!=UDS_FLASH_31_STATUS_SUCCESS_CODE)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}	
	*pDependenceResult = responseData[4];
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteRoutineControlCheckMemory(guint8 *pCrcData,guint32 crcLength)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint16 did;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	did = 0x0202;
	requestData[0] = 0x31;
	requestData[1] = 0x01;
	requestData[2] = did>>8;//did H
	requestData[3] = did;//did L

	for(i=0;i<crcLength;i++)
	{
		requestData[i+4] = pCrcData[i];
	}
	requestDataLength = 4+crcLength;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<3)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseDataLength<5)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}	
	if(responseData[0]!=(0x31+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[2]!=requestData[2])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[3]!=requestData[3])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}	
	if(responseData[4]!=UDS_FLASH_31_STATUS_SUCCESS_CODE)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}	
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteWriteDataByIdentifier(guint16 did,guint8 *pData,guint32 dataLength)
{
	guint8 canChannel;
	gint32 udsIndex;
	gint32 result;
	guint8 requestData[1024];
	guint8 responseData[1024];
	guint32 requestDataLength,responseDataLength;
	guint32 i;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);

	requestData[0] = 0x2E;
	requestData[1] = (did>>8)&0xFF;//did H
	requestData[2] = did&0xFF;//did L

	for(i=0;i<dataLength;i++)
	{
		requestData[i+3] = pData[i];
	}
	requestDataLength = 3+dataLength;
	responseDataLength = 0x00;

	result = m_UdsTp.ServiceExecute(m_functionalAddressing,requestData,requestDataLength,responseData,&responseDataLength);
	if(result!=TBOX_UDS_TP_RESULT_OK)
	{
		return TBOX_UDS_RESULT_ERROR_TP;
	}
	if(m_functionalAddressing)
	{
		return TBOX_UDS_RESULT_OK;
	}
	if(responseDataLength<3)
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[0]==0x7F)//negative response
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE;
	}
	if(responseData[0]!=(0x31+0x40))
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[1]!=requestData[1])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	if(responseData[2]!=requestData[2])
	{
		return TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT;
	}
	return TBOX_UDS_RESULT_OK;
}

gint32 CTboxUdsService::ExcuteWriteFingerPrint(STRUCT_UDS_FINGER_PRINT *pFingerPrint,guint32 numOfFingerPrint)
{
	gint32 ret;
	gint32 i;
	guint8 fingerData[100];
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);
	
	fingerData[0] = pFingerPrint->blockId;
	for(i=0;i<pFingerPrint->blockInfoLength;i++)
	{
		fingerData[1+i] = pFingerPrint->blockInfoData[i];
	}
	ret = ExcuteWriteDataByIdentifier(0xF15A,fingerData,pFingerPrint->blockInfoLength+1);
	return ret;
}

#define FINGER_PIRNT_LENGTH		10
gint32 CTboxUdsService::ExcuteReadFingerPrint(STRUCT_UDS_FINGER_PRINT *pFingerPrint,guint32 *pNumOfFingerPrint)
{
	gint32 ret;
	guint32 i;
	guint8 fingerData[1024];
	guint32 length;
	guint32 fingerNum;
	
	DEBUG("----%s,%d\n", __FUNCTION__, __LINE__);
	
	ret = ExcuteReadDataByIdentifier(0xF15B,fingerData,&length);
	if(ret != TBOX_UDS_RESULT_OK)
	{
		return ret;
	}
	fingerNum = length/FINGER_PIRNT_LENGTH;
	for(i=0;i<fingerNum;i++)
	{
		memcpy(&pFingerPrint[i],&fingerData[FINGER_PIRNT_LENGTH*i],FINGER_PIRNT_LENGTH);
	}
	*pNumOfFingerPrint = fingerNum;
	return ret;
}


#if(1)
void UdsServiceTest(void)
{


}
#endif

