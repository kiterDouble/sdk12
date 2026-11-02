#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wait.h>
#include <errno.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <stdint.h>
 
#include <ECallMessage.h>
#include "MSDMessage.h"
#include "MSDStructure.h"
#include "AdditionalData.h"
#include "sr_log.h"
#include "ERAAdditionalData.h"

#if 1
#define MSD_LOG_INFO                ("/sdcard/log/msd.log")
#define MSD_ADDDATA_LOG_INFO        ("/sdcard/log/msd_adddata.log")
#define UPER_ENCODE_BUF_MAX_LENTH   (512)
BOOLEAN_t g_ASNTrue = 1;
BOOLEAN_t g_ASNFalse = 0;
 
struct AdditionalData *EncodeMsgAdditionalDataFill(msd_optionalData *orgData);
static int ECallASNSave2File(const char *path, asn_TYPE_descriptor_t *td, void *sptr);

static int ECallASNSave2File(const char *path, asn_TYPE_descriptor_t *td, void *sptr)
{
    FILE *file = fopen(path, "w");

    if (NULL != file)
    {
        LOG_Info("MSDMessage write to %s file", path);
        xer_fprint(file, td, sptr);
        fclose(file);
        return 0;
    }

    LOG_Info("MSDMessage write to %s file fail(%s)", path, strerror(errno));

    return -1; 
}

int EncodeMsgMSDStructureFill(MSDStructure_t *ms, ecallMsg *usrData)
{
    static VehicleLocationDelta_t N1Tmp;
    static VehicleLocationDelta_t N2Tmp;
    
    memset(&N1Tmp, 0, sizeof(VehicleLocationDelta_t));
    memset(&N2Tmp, 0, sizeof(VehicleLocationDelta_t));

    ms->messageIdentifier = usrData->messageIdentifier;
    
    // ControlType 
    ms->control.automaticActivation = usrData->control.automaticActivation;
    ms->control.testCall = usrData->control.testCall;
    ms->control.positionCanBeTrusted = usrData->control.positionCanBeTrusted;
    ms->control.vehicleType = usrData->control.vehType;
    
    // VIN 
    ms->vehicleIdentificationNumber.isowmi.size = 3;
    ms->vehicleIdentificationNumber.isowmi.buf = usrData->VIN.isowmi;

    ms->vehicleIdentificationNumber.isovds.size=6;
    ms->vehicleIdentificationNumber.isovds.buf = usrData->VIN.isovds;
    
    ms->vehicleIdentificationNumber.isovisModelyear.size=1;
    ms->vehicleIdentificationNumber.isovisModelyear.buf = & usrData->VIN.isovisModelyear[0];

    ms->vehicleIdentificationNumber.isovisSeqPlant.size=7;
    ms->vehicleIdentificationNumber.isovisSeqPlant.buf = usrData->VIN.isovisSeqPlant;
    
    ms->vehiclePropulsionStorageType.gasolineTankPresent = &g_ASNFalse;
    ms->vehiclePropulsionStorageType.dieselTankPresent = &g_ASNFalse;
    ms->vehiclePropulsionStorageType.compressedNaturalGas = &g_ASNFalse;
    ms->vehiclePropulsionStorageType.liquidPropaneGas = &g_ASNFalse;
    ms->vehiclePropulsionStorageType.electricEnergyStorage = &g_ASNFalse;
    ms->vehiclePropulsionStorageType.hydrogenStorage = &g_ASNFalse;
    ms->vehiclePropulsionStorageType.otherStorage = &g_ASNFalse;
    // VehiclePropulsionStorageType 
    if(usrData->vehPropulsionStorageType.gasolineTankPresent)
        ms->vehiclePropulsionStorageType.gasolineTankPresent = &g_ASNTrue;
    if(usrData->vehPropulsionStorageType.dieselTankPresent)
        ms->vehiclePropulsionStorageType.dieselTankPresent = &g_ASNTrue;
    if(usrData->vehPropulsionStorageType.compressedNaturalGas)
        ms->vehiclePropulsionStorageType.compressedNaturalGas = &g_ASNTrue;
    if(usrData->vehPropulsionStorageType.liquidPropaneGas)
        ms->vehiclePropulsionStorageType.liquidPropaneGas = &g_ASNTrue;
    if(usrData->vehPropulsionStorageType.electricEnergyStorage)
        ms->vehiclePropulsionStorageType.electricEnergyStorage = &g_ASNTrue;
    if(usrData->vehPropulsionStorageType.hydrogenStorage)
        ms->vehiclePropulsionStorageType.hydrogenStorage = &g_ASNTrue;
    if(usrData->vehPropulsionStorageType.otherStorage)
        ms->vehiclePropulsionStorageType.otherStorage = &g_ASNTrue;
    //TIME
    ms->timestamp = usrData->timestamp;
    // VehicleLocation 
    ms->vehicleLocation.positionLatitude = usrData->position.latitude;
    ms->vehicleLocation.positionLongitude = usrData->position.longitude;
    // VehicleLocationDelta 
    ms->vehicleDirection = usrData->vehicleDirection;
    
    ms->recentVehicleLocationN1 = (VehicleLocationDelta_t *)&N1Tmp;
    ms->recentVehicleLocationN2 = (VehicleLocationDelta_t *)&N2Tmp;
    ms->recentVehicleLocationN1->latitudeDelta = usrData->recentVehicleLocationN1.latitudeDelta;
    ms->recentVehicleLocationN1->longitudeDelta = usrData->recentVehicleLocationN1.longitudeDelta;
    ms->recentVehicleLocationN2->latitudeDelta = usrData->recentVehicleLocationN2.latitudeDelta;
    ms->recentVehicleLocationN2->longitudeDelta = usrData->recentVehicleLocationN2.longitudeDelta;
    ms->numberOfPassengers = (long *)&(usrData->numberOfPassengers);   /* 存在内存越界 */

    return 0;
}


int Encode_Msg(ecallMsg *ecall_msd, char *uper_msd, size_t uper_len)
{
    ECallMessage_t msgECall = {0}; // 直接使用栈内存，避免二次释放问题
    MSDMessage_t msdStr = {0};
    asn_enc_rval_t ec;
    uint8_t sendbuf[UPER_ENCODE_BUF_MAX_LENTH] = {0};

    // 填充 MSD 数据
    EncodeMsgMSDStructureFill(&msdStr.msdStructure, ecall_msd);
    msdStr.optionalAdditionalData = EncodeMsgAdditionalDataFill(&ecall_msd->optionalData);

    // 编码 MSD 数据
    ec = uper_encode_to_buffer(&asn_DEF_MSDMessage, &msdStr, sendbuf, sizeof(sendbuf));
    if (ec.encoded == -1) {
        LOG_Error("Could not encode MSDMessage (at %s)", ec.failed_type ? ec.failed_type->name : "unknown");
        ASN_STRUCT_FREE(asn_DEF_AdditionalData, msdStr.optionalAdditionalData);
        return ec.encoded;
    }
    else
    {
        ECallASNSave2File(MSD_LOG_INFO, &asn_DEF_MSDMessage, &msdStr);
    }
    // 释放可选数据
    ASN_STRUCT_FREE(asn_DEF_AdditionalData, msdStr.optionalAdditionalData);

    // 填充 ECallMessage 结构
    msgECall.msdVersion = 2;
    OCTET_STRING_fromBuf(&msgECall.msd, (const char*)sendbuf, ec.encoded);

    // 编码 ECallMessage
    ec = uper_encode_to_buffer(&asn_DEF_ECallMessage, &msgECall, uper_msd, uper_len);
    if (ec.encoded == -1) {
        LOG_Error("Could not encode ECallMessage (at %s)", ec.failed_type ? ec.failed_type->name : "unknown");
    }

    // 手动释放 OCTET_STRING 中的内存，防止内存泄漏
    free(msgECall.msd.buf);

    return ec.encoded;
}

struct AdditionalData *EncodeMsgAdditionalDataFill(msd_optionalData *usrData)
{
    struct AdditionalData *msdData = NULL;
    struct ERAAdditionalData eraUsrData;
    char buff[UPER_ENCODE_BUF_MAX_LENTH];
    asn_enc_rval_t ec;
    int ret = 0;
    int oidTable[] = {1, 4, 2}; /* 1.4.2*/

    long crashSeverityASI15 = 0;
    struct DiagnosticResult diagnosticResult = {
        .micConnectionFailure = &usrData->diagMicConnection,
        .micFailure = &g_ASNFalse,
        .rightSpeakerFailure = &g_ASNFalse,
        .leftSpeakerFailure = &g_ASNFalse,
        .speakersFailure = &usrData->diagSpeakerConnection,
        .ignitionLineFailure = &usrData->diagIgnition,
        .uimFailure = &usrData->diagUIMTest,
        .statusIndicatorFailure = &g_ASNFalse,
        .batteryFailure = &usrData->diagBattery,
        .batteryVoltageLow = &g_ASNFalse,
        .crashSensorFailure = &usrData->diagRTADetector,
        .firmwareImageCorruption = &usrData->diagImageOk,
        .commModuleInterfaceFailure = &g_ASNFalse,
        .gnssReceiverFailure = &usrData->diagGNSSReceiver,
        .raimProblem = &g_ASNFalse,
        .gnssAntennaFailure = &g_ASNFalse,
        .commModuleFailure = &g_ASNFalse,
        .eventsMemoryOverflow = &g_ASNFalse,
        .crashProfileMemoryOverflow = &g_ASNFalse,
        .otherCriticalFailires = &g_ASNFalse,
        .otherNotCriticalFailures = &g_ASNFalse,
    };

    struct CrashInfo crashInfo =
    {
        .crashFront = &g_ASNFalse,
        .crashLeft = &g_ASNFalse,
        .crashRight = &g_ASNFalse,
        .crashRear = &g_ASNFalse,
        .crashRollover = &g_ASNFalse,
        .crashSide = &g_ASNFalse,
        .crashFrontOrSide = &g_ASNFalse,
        .crashAnotherType = &g_ASNFalse,
    };
    CoordinateSystemType_t coordinateSystemType = 1;

    memset(&eraUsrData, 0, sizeof(struct ERAAdditionalData));
    eraUsrData.crashSeverityASI15 =  &crashSeverityASI15;
    eraUsrData.diagnosticResult = &diagnosticResult;
    eraUsrData.crashInfo = &crashInfo;
    eraUsrData.coordinateSystemType = &coordinateSystemType;

    memset(buff, 0, sizeof(buff));
    ec = uper_encode_to_buffer(&asn_DEF_ERAAdditionalData, &eraUsrData, buff, sizeof(buff));
    if(ec.encoded  == -1)
    {
        LOG_Error("msd era_additional_data fail (at %s)",ec.failed_type ? ec.failed_type->name : "unknown");
        goto errEraAddData;
    } 
            
//    xer_fprint(stdout, &asn_DEF_ERAAdditionalData, &eraUsrData);
    ECallASNSave2File(MSD_ADDDATA_LOG_INFO, &asn_DEF_ERAAdditionalData, &eraUsrData);
    msdData = (struct AdditionalData *)calloc(1, sizeof(struct AdditionalData));
    if(NULL == msdData)
    {
        LOG_Error("msd era_additional_data calloc fail!");
        goto errEraAddDataCalloc;
    }

    ret = RELATIVE_OID_set_arcs(&msdData->oid, oidTable, sizeof(oidTable[0]), (sizeof(oidTable)/sizeof(oidTable[0])));
    if(-1 == ret)
    {
        LOG_Error("msd era_additional_data's relative_oid fail!");
        goto errOctetString;
    }

    ret = OCTET_STRING_fromBuf(&msdData->data, buff, ec.encoded);
    if(-1 == ret)
    {
        LOG_Error("msd era_additional_data's octet_string fail!");
        goto errOctetString;
    }
//    xer_fprint(stdout, &asn_DEF_AdditionalData, &msdData);

    
    return msdData;
errOctetString:
    free(msdData);
errEraAddDataCalloc:
errEraAddData:
    return NULL;
}



#else
#define MSD_LOG_INFO ("/log/msd.log")
#define UPER_ENCODE_BUF_MAX_LENTH (512)
static char sendbuf[UPER_ENCODE_BUF_MAX_LENTH] = {0};
 
/**
* @brief 将ECALL消息编码为上位机消息
*
* @param[in] ecall_msd ECALL消息结构体
* @param[out] uper_msd 上位机消息字符串
* @param[in] uper_len 上位机消息最大长度
* @return 编码成功返回1，否则返回0
*/
int Encode_Msg(ecallMsg *ecall_msd, char *uper_msd, size_t uper_len)
{
    ECallMessage_t *msgECall = NULL;
    MSDMessage_t *msdStr = NULL;
    uint8_t uperBuff[UPER_ENCODE_BUF_MAX_LENTH]={0,};
    char msgTmp[UPER_ENCODE_BUF_MAX_LENTH*2] = {0,};
    char msgStrTmp[UPER_ENCODE_BUF_MAX_LENTH*2] = {0,};
    char N1Tmp[UPER_ENCODE_BUF_MAX_LENTH*2] = {0,};
    char N2Tmp[UPER_ENCODE_BUF_MAX_LENTH*2] = {0,};
    BOOLEAN_t FLASE = 0;
    BOOLEAN_t TURE = 1;

    msgECall = (ECallMessage_t *)msgTmp;
    msdStr = (MSDMessage_t *)msgStrTmp;
    
    asn_enc_rval_t ec; /* Encoder return value */
    msdStr->msdStructure.messageIdentifier = ecall_msd->messageIdentifier;
    
    // ControlType 
    msdStr->msdStructure.control.automaticActivation = ecall_msd->control.automaticActivation;
    msdStr->msdStructure.control.testCall = ecall_msd->control.testCall;
    msdStr->msdStructure.control.positionCanBeTrusted = ecall_msd->control.positionCanBeTrusted;
    msdStr->msdStructure.control.vehicleType = ecall_msd->control.vehType;
    
    // VIN 
    msdStr->msdStructure.vehicleIdentificationNumber.isowmi.size = 3;
    msdStr->msdStructure.vehicleIdentificationNumber.isowmi.buf = ecall_msd->VIN.isowmi;

    msdStr->msdStructure.vehicleIdentificationNumber.isovds.size=6;
    msdStr->msdStructure.vehicleIdentificationNumber.isovds.buf = ecall_msd->VIN.isovds;
    
    msdStr->msdStructure.vehicleIdentificationNumber.isovisModelyear.size=1;
    msdStr->msdStructure.vehicleIdentificationNumber.isovisModelyear.buf = &ecall_msd->VIN.isovisModelyear[0];

    msdStr->msdStructure.vehicleIdentificationNumber.isovisSeqPlant.size=7;
    msdStr->msdStructure.vehicleIdentificationNumber.isovisSeqPlant.buf = ecall_msd->VIN.isovisSeqPlant;
    
    msdStr->msdStructure.vehiclePropulsionStorageType.gasolineTankPresent = &FLASE;
    msdStr->msdStructure.vehiclePropulsionStorageType.dieselTankPresent = &FLASE;
    msdStr->msdStructure.vehiclePropulsionStorageType.compressedNaturalGas = &FLASE;
    msdStr->msdStructure.vehiclePropulsionStorageType.liquidPropaneGas = &FLASE;
    msdStr->msdStructure.vehiclePropulsionStorageType.electricEnergyStorage = &FLASE;
    msdStr->msdStructure.vehiclePropulsionStorageType.hydrogenStorage = &FLASE;
    msdStr->msdStructure.vehiclePropulsionStorageType.otherStorage = &FLASE;
    // VehiclePropulsionStorageType 
    if(ecall_msd->vehPropulsionStorageType.gasolineTankPresent)
        msdStr->msdStructure.vehiclePropulsionStorageType.gasolineTankPresent = &TURE;
    if(ecall_msd->vehPropulsionStorageType.dieselTankPresent)
        msdStr->msdStructure.vehiclePropulsionStorageType.dieselTankPresent = &TURE;
    if(ecall_msd->vehPropulsionStorageType.compressedNaturalGas)
        msdStr->msdStructure.vehiclePropulsionStorageType.compressedNaturalGas = &TURE;
    if(ecall_msd->vehPropulsionStorageType.liquidPropaneGas)
        msdStr->msdStructure.vehiclePropulsionStorageType.liquidPropaneGas = &TURE;
    if(ecall_msd->vehPropulsionStorageType.electricEnergyStorage)
        msdStr->msdStructure.vehiclePropulsionStorageType.electricEnergyStorage = &TURE;
    if(ecall_msd->vehPropulsionStorageType.hydrogenStorage)
        msdStr->msdStructure.vehiclePropulsionStorageType.hydrogenStorage = &TURE;
    if(ecall_msd->vehPropulsionStorageType.otherStorage)
        msdStr->msdStructure.vehiclePropulsionStorageType.otherStorage = &TURE;
    //TIME
    msdStr->msdStructure.timestamp = ecall_msd->timestamp;
    // VehicleLocation 
    msdStr->msdStructure.vehicleLocation.positionLatitude = ecall_msd->position.latitude;
    msdStr->msdStructure.vehicleLocation.positionLongitude = ecall_msd->position.longitude;
    // VehicleLocationDelta 
    msdStr->msdStructure.vehicleDirection = ecall_msd->vehicleDirection;
    
    msdStr->msdStructure.recentVehicleLocationN1 = (VehicleLocationDelta_t *)N1Tmp;
    msdStr->msdStructure.recentVehicleLocationN2 = (VehicleLocationDelta_t *)N2Tmp;
    msdStr->msdStructure.recentVehicleLocationN1->latitudeDelta = ecall_msd->recentVehicleLocationN1.latitudeDelta;
    msdStr->msdStructure.recentVehicleLocationN1->longitudeDelta = ecall_msd->recentVehicleLocationN1.longitudeDelta;
    msdStr->msdStructure.recentVehicleLocationN2->latitudeDelta = ecall_msd->recentVehicleLocationN2.latitudeDelta;
    msdStr->msdStructure.recentVehicleLocationN2->longitudeDelta = ecall_msd->recentVehicleLocationN2.longitudeDelta;
    msdStr->msdStructure.numberOfPassengers = (long *)&ecall_msd->numberOfPassengers;   /* 存在内存越界 */
    // 对从C结构编码成ASN1格式数据buf
    // UPER
    memset(sendbuf, 0, sizeof(sendbuf));
    ec = uper_encode_to_buffer(&asn_DEF_MSDMessage, msdStr, sendbuf, UPER_ENCODE_BUF_MAX_LENTH);
    if(ec.encoded  == -1)
    {
        LOG_Error("msdStr Could not encode MessageFrame MSDMessage (at %s)",ec.failed_type ? ec.failed_type->name : "unknown");
        return ec.encoded;
    } 
    else 
    {
        FILE *file_stream = fopen(MSD_LOG_INFO, "w");
        if (file_stream == NULL) {
            LOG_Info("Failed to open file for writing: %s", strerror(errno));
        }
        else
        {
            
            LOG_Info("MSDMessage write in %s", MSD_LOG_INFO);
            xer_fprint(file_stream, &asn_DEF_MSDMessage, msdStr);
            fclose(file_stream);
        }
        for( int i = 0 ;i < ec.encoded && i< UPER_ENCODE_BUF_MAX_LENTH;i++)
        {
                uperBuff[i] = *((unsigned char *)sendbuf + i);
        }
    }
	
    msgECall->msdVersion = 2;            /* MSD version 2 */
    msgECall->msd.buf = uperBuff;
    msgECall->msd.size = ec.encoded;
    // 对从C结构编码成ASN1格式数据buf
    // UPER
    memset(sendbuf, 0, sizeof(sendbuf));
    ec = uper_encode_to_buffer(&asn_DEF_ECallMessage, msgECall, sendbuf, UPER_ENCODE_BUF_MAX_LENTH);
    if(ec.encoded  == -1) {
        LOG_Error("msdStr Could not encode MessageFrame ECallMessage (at %s)",ec.failed_type ? ec.failed_type->name : "unknown");
    } 
    else 
    {
        memcpy(uper_msd, sendbuf, ec.encoded);
    }
    return ec.encoded;
}
 
 
#endif 
