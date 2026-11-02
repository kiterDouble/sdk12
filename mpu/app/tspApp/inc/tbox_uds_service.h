#ifndef __TBOX_UDS_H__
#define __TBOX_UDS_H__

#include <glib.h>
//#include "lib_sirun.h"
#include "tbox_uds_tp.h"


#define TBOX_UDS_RESULT_OK                   0
#define TBOX_UDS_RESULT_ERROR_HANDLE         -1
#define TBOX_UDS_RESULT_ERROR_TP             -2
#define TBOX_UDS_RESULT_ERROR_CHANNEL        -3
#define TBOX_UDS_RESULT_ERROR_PARAMETER      -4
#define TBOX_UDS_RESULT_ERROR_RESPONE_FORMAT     -5
#define TBOX_UDS_RESULT_ERROR_RESPONE_NEGATIVE   -6
#define TBOX_UDS_RESULT_ERROR_RESPONE_TIMEOUT   -7

typedef struct
{
	guint32 physicalRequestId;
	guint32 functionalRequestId;
	guint32 responseId;
}STRUCT_UDS_CONFIGURE;


typedef enum
{
    UDS_SESSION_DEFAULT = 0x01,
    UDS_SESSION_PROGRAMMING = 0x02,   
    UDS_SESSION_EXTENDED = 0x03,
}ENUM_UDS_SESSION;

typedef struct 
{
    guint32 dtcCode;
    guint8 status;
}STRUCT_UDS_DTC;

typedef struct 
{
    guint8 ecuVoltage;
    guint16 motorSpeed;
    guint16 vehicleSpeed;
    guint32 vehicleOdlmeter;
    guint8 globalRealTime[7];
}STRUCT_UDS_DTC_SNAPSHOT;

typedef struct 
{
    guint8 faultOccurrenceCounter;
    guint8 faultPendingCounter;
    guint8 dtcAgingCounter;
    guint8 dtcAgedCounter;
}STRUCT_UDS_DTC_EXTENDED_DATA;

typedef struct 
{
    guint8 blockId;
    guint8 blockInfoData[30];
    guint8 blockInfoLength;
}STRUCT_UDS_FINGER_PRINT;

class CTboxUdsService
{
    public:
    static gint32 Initialize();
    static void Uninitialize();
    CTboxUdsService();
    ~CTboxUdsService();
    gint32 Open(guint8 canChannel);
    void  Close();

    gint32 ExcuteSessionControl(ENUM_UDS_SESSION destinationSession);
    gint32 ExcuteGetSeed(guint8 securityLevel,guint8 *pSeed,guint32 *pSeedLength);
    gint32 ExcuteSetKey(guint8 securityLevel,guint8 *pKey,guint32 keyLength);
    gint32 ExcuteControlDtcSetting(guint8 controlFlag);    
    gint32 ExcuteCommunicationControl(guint8 controlType,guint8 commnicationType);
    gint32 ExcuteControlTestPresent();
    gint32 ExcuteEcuReset(guint8 resetType);
    //
    gint32 ExcuteReadDtcListByStatusMask(guint8 statusMask,guint8 *pValidEcuMask,STRUCT_UDS_DTC *pDtc,guint32 *pDtcNum);
    gint32 ExcuteReadDtcNumberByStatusMask(guint8 statusMask,guint8 *pValidEcuMask,guint32 *pDtcNum);
    gint32 ExcuteReadDtcSnapshotRecord(guint32 dtcCode,STRUCT_UDS_DTC_SNAPSHOT *pSnapshot,guint8 *pDtcStatusOut);
    gint32 ExcuteReadDtcExtendedDataRecord(guint32 dtcCode,STRUCT_UDS_DTC_EXTENDED_DATA *pExtendedData,guint8 *pDtcStatusOut);
    gint32 ExcuteClearDtc(guint32 clearFlag);
    //
    gint32 ExcuteReadDataByIdentifier(guint16 did,guint8 *pDataOut,guint32 *pLengthOut);   
    gint32 ExcuteWriteDataByIdentifier(guint16 did,guint8 *pData,guint32 dataLength);
    gint32 ExcuteWriteFingerPrint(STRUCT_UDS_FINGER_PRINT *pFingerPrint,guint32 numOfFingerPrint);
    gint32 ExcuteReadFingerPrint(STRUCT_UDS_FINGER_PRINT *pFingerPrint,guint32 *pNumOfFingerPrint);
    // 
    gint32 ExcuteTransferData(guint8 BlockSequenceCounter,guint8 *pFlashData,guint32 flashDataLength);
    gint32 ExcuteTransferExit();
    gint32 ExcuteRequestDownLoad(guint32 startAddress,guint32 uncompressedSize,guint32 *pMaxNumberOfBlockLength);
    gint32 ExcuteRoutineControlEraseFlash(guint32 startAddress,guint32 length);
    gint32 ExcuteRoutineControlCheckDependence(guint8 *pDependenceResult);
    gint32 ExcuteRoutineControlCheckMemory(guint8 *pCrcData,guint32 crcLength);
    gint32 ExcuteRoutineControlCheckPreCondition();
    //
    void SetUdsFlash31StatusSuccessCode(guint8 successCode);

    private:
    CTboxUdsTp m_UdsTp;
    guint8 canChannel;
    guint8 openFlag;
    guint8 m_functionalAddressing;
    guint8 m_udsFlash31StatusSuccessCode;//UDS_FLASH_31_STATUS_SUCCESS_CODE
};

void UdsServiceTest(void);

 








#endif//__TBOX_UDS_H__
