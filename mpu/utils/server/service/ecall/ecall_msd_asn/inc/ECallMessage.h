#ifndef	_ECallMessage_H_
#define	_ECallMessage_H_

#define MAX_ECALL_MSD_LENTH                               140 /**  Maximum size of the MSD sent to the network with an eCall */

#include "asn_application.h"

#include <NativeInteger.h>
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>
#include <BOOLEAN.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
    ECALL_TEST_MODE             = 0x01, 
    ECALL_EMERGENCY_MODE        = 0x02, 
    ECALL_RECONFIG_MODE         = 0x03, 
}ECALL_VARIANT_T;

typedef enum
{
    EU_MSD_VEHICLE_PASSENGER_M1=1,  ///< Passenger vehicle (Class M1)
    EU_MSD_VEHICLE_BUS_M2,          ///< Buses and coaches (Class M2)
    EU_MSD_VEHICLE_BUS_M3,          ///< Buses and coaches (Class M3)
    EU_MSD_VEHICLE_COMMERCIAL_N1,   ///< Light commercial vehicles (Class N1)
    EU_MSD_VEHICLE_HEAVY_N2,        ///< Heavy duty vehicles (Class N2)
    EU_MSD_VEHICLE_HEAVY_N3,        ///< Heavy duty vehicles (Class N3)
    EU_MSD_VEHICLE_MOTORCYCLE_L1E,  ///< Motorcycles (Class L1e)
    EU_MSD_VEHICLE_MOTORCYCLE_L2E,  ///< Motorcycles (Class L2e)
    EU_MSD_VEHICLE_MOTORCYCLE_L3E,  ///< Motorcycles (Class L3e)
    EU_MSD_VEHICLE_MOTORCYCLE_L4E,  ///< Motorcycles (Class L4e)
    EU_MSD_VEHICLE_MOTORCYCLE_L5E,  ///< Motorcycles (Class L5e)
    EU_MSD_VEHICLE_MOTORCYCLE_L6E,  ///< Motorcycles (Class L6e)
    EU_MSD_VEHICLE_MOTORCYCLE_L7E,  ///< Motorcycles (Class L7e)
}
msd_VehicleType;


typedef struct
{
    char                        ecall_msd[MAX_ECALL_MSD_LENTH];  //max msd length 140
    uint32_t                    msd_len;
    int                         manual;
    ECALL_VARIANT_T    eCallModeType; 
}ecall_info;


/* ECallMessage */
typedef struct ECallMessage {
	long	 msdVersion;
	OCTET_STRING_t	 msd;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ECallMessage_t;

typedef struct {
   uint8_t  oidlen;
   uint8_t  dataLen;
   uint8_t* oid;
   uint8_t* data;
} msd_optionalData;

typedef struct {
   BOOLEAN_t                 automaticActivation;
   BOOLEAN_t                 testCall;
   BOOLEAN_t                 positionCanBeTrusted;
   msd_VehicleType      vehType;
} msd_Control;

typedef struct {
   BOOLEAN_t gasolineTankPresent;
   BOOLEAN_t dieselTankPresent;
   BOOLEAN_t compressedNaturalGas;
   BOOLEAN_t liquidPropaneGas;
   BOOLEAN_t electricEnergyStorage;
   BOOLEAN_t hydrogenStorage;
   BOOLEAN_t otherStorage;
} msd_VehiclePropulsionStorageType;

typedef struct {
   int32_t    latitude;
   int32_t    longitude;
} msd_VehicleLocation;

typedef struct {
   int32_t    latitudeDelta;
   int32_t    longitudeDelta;
} msd_VehicleLocationDelta;

typedef struct __attribute__ ((__packed__)) {
   uint8_t    isowmi[3];
   uint8_t    isovds[6];
   uint8_t    isovisModelyear[1];
   uint8_t    isovisSeqPlant[7];
} msd_Vin;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ECallMessage;
typedef struct _ecallMsg {
    uint8_t                     messageIdentifier;
    msd_Control                 control;
    msd_Vin                     VIN;
    msd_VehiclePropulsionStorageType vehPropulsionStorageType;
    uint32_t                    timestamp;
    msd_VehicleLocation         position;
    uint8_t                     vehicleDirection;    
    BOOLEAN_t                        recentVehicleLocationN1Pres;      
    msd_VehicleLocationDelta    recentVehicleLocationN1;     
    BOOLEAN_t                        recentVehicleLocationN2Pres;      
    msd_VehicleLocationDelta    recentVehicleLocationN2;      
    BOOLEAN_t                        numberOfPassengersPres;
    uint8_t                     numberOfPassengers;    
    BOOLEAN_t                        optionalDataPres;
    msd_optionalData            optionalData;
} ecallMsg;

int Encode_Msg(ecallMsg *ecall_msd,char *uper_msd,char uper_len);
#ifdef __cplusplus
}
#endif

#endif	/* _ECallMessage_H_ */
#include <asn_internal.h>
