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

#define MSD_LOG_INFO ("/oemdata/log/MSD.log")
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
int Encode_Msg(ecallMsg *ecall_msd,char *uper_msd,char uper_len)
{
    
    ECallMessage_t *msg = NULL;
    MSDMessage_t *msdStr = NULL;
    uint8_t uperBuff[UPER_ENCODE_BUF_MAX_LENTH]={0,};
    char msgTmp[UPER_ENCODE_BUF_MAX_LENTH*2] = {0,};
    char msgStrTmp[UPER_ENCODE_BUF_MAX_LENTH*2] = {0,};
    char N1Tmp[UPER_ENCODE_BUF_MAX_LENTH*2] = {0,};
    char N2Tmp[UPER_ENCODE_BUF_MAX_LENTH*2] = {0,};
    BOOLEAN_t FLASE = 0;
    BOOLEAN_t TURE = 1;

    msg = (ECallMessage_t *)msgTmp;
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
    msdStr->msdStructure.numberOfPassengers = (long *)&ecall_msd->numberOfPassengers;
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
        FILE *file_stream = fopen("/oemdata/log/msd.log", "w");
        if (file_stream == NULL) {
            LOG_Info("Failed to open file for writing: %s", strerror(errno));
        }
        else
        {
            
            LOG_Info("MSDMessage write in /oemdata/log/msd.log");
            xer_fprint(file_stream, &asn_DEF_MSDMessage, msdStr);
            fclose(file_stream);
        }
        for( int i = 0 ;i < ec.encoded && i< UPER_ENCODE_BUF_MAX_LENTH;i++)
        {
                uperBuff[i] = *((unsigned char *)sendbuf + i);
        }
    }
	
    msg->msdVersion = 3;
    msg->msd.buf = uperBuff;
    msg->msd.size = ec.encoded;
    // 对从C结构编码成ASN1格式数据buf
    // UPER
    memset(sendbuf, 0, sizeof(sendbuf));
    ec = uper_encode_to_buffer(&asn_DEF_ECallMessage, msg, sendbuf, UPER_ENCODE_BUF_MAX_LENTH);
    if(ec.encoded  == -1) {
        LOG_Error("msdStr Could not encode MessageFrame ECallMessage (at %s)",ec.failed_type ? ec.failed_type->name : "unknown");
    } 
    else 
    {
        memcpy(uper_msd,sendbuf,ec.encoded);
    }
    return ec.encoded;
}
 
 
