#ifndef _CANPARSESDK_H
#define _CANPARSESDK_H

#include "stdint.h"

#define VEHICLE_CAN_UNPACK_FORMAT_INTEL     	0
#define VEHICLE_CAN_UNPACK_FORMAT_MOTO_LSB		1		
#define VEHICLE_CAN_UNPACK_FORMAT_MOTO_MSB		2

#define CAN_V_FD_MAX_DATA_SIZE     64

typedef struct 
{
  void*     msgBufferPointer;
  uint32_t  canBufferIdIndex;
  uint16_t  dataType;
  uint16_t  startBit;
  uint16_t  bitLength;
  float     resulotion;
  float     offset;
  uint8_t   useInvalidFlag;
  uint32_t  InvalidData;
}CanParseSignal_t;

// typedef struct
// {
// 	const uint32_t canId;
// 	const uint32_t timeOutValue;
// }canId_map_configure_t;//canId_map_configure_t

typedef struct
{
	uint8_t        canData[CAN_V_FD_MAX_DATA_SIZE];	
	uint32_t       cyleTimeCount;
	uint8_t        cycleTimeOutFlag;
	uint8_t        rxFlag;
}CanSignalMsgBuffer_t;


int16_t CanParseSdkSignalIsValid(uint8_t CanDataFormat,const CanParseSignal_t* signal);
int16_t CanParseSdkReadSignal(uint8_t CanDataFormat,const CanParseSignal_t* signal,double* pValueOut);
int16_t PackSignalToFrame(uint8_t* pMsgData,uint8_t CanDataFormat,const CanParseSignal_t* signal,double signalValue);

#endif  //_CANPARSESDK_H