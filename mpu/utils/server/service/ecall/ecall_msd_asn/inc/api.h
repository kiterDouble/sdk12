
#ifndef	_ECallMessage_H_
#define	_ECallMessage_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Dependencies */
typedef enum VehicleType {
	VehicleType_passengerVehicleClassM1	= 1,
	VehicleType_busesAndCoachesClassM2	= 2,
	VehicleType_busesAndCoachesClassM3	= 3,
	VehicleType_lightCommercialVehiclesClassN1	= 4,
	VehicleType_heavyDutyVehiclesClassN2	= 5,
	VehicleType_heavyDutyVehiclesClassN3	= 6,
	VehicleType_motorcyclesClassL1e	= 7,
	VehicleType_motorcyclesClassL2e	= 8,
	VehicleType_motorcyclesClassL3e	= 9,
	VehicleType_motorcyclesClassL4e	= 10,
	VehicleType_motorcyclesClassL5e	= 11,
	VehicleType_motorcyclesClassL6e	= 12,
	VehicleType_motorcyclesClassL7e	= 13
	/*
	 * Enumeration is extensible
	 */
} e_VehicleType;


/* VehiclePropulsionStorageType */
typedef struct VehiclePropulsionStorageType {
	BOOLEAN_t	*gasolineTankPresent	/* DEFAULT FALSE */;
	BOOLEAN_t	*dieselTankPresent	/* DEFAULT FALSE */;
	BOOLEAN_t	*compressedNaturalGas	/* DEFAULT FALSE */;
	BOOLEAN_t	*liquidPropaneGas	/* DEFAULT FALSE */;
	BOOLEAN_t	*electricEnergyStorage	/* DEFAULT FALSE */;
	BOOLEAN_t	*hydrogenStorage	/* DEFAULT FALSE */;
	BOOLEAN_t	*otherStorage	/* DEFAULT FALSE */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VehiclePropulsionStorageType_t;
typedef struct VIN {
	PrintableString_t	 isowmi;
	PrintableString_t	 isovds;
	PrintableString_t	 isovisModelyear;
	PrintableString_t	 isovisSeqPlant;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VIN_t;

/* ControlType */
typedef struct ControlType {
	BOOLEAN_t	 automaticActivation;
	BOOLEAN_t	 testCall;
	BOOLEAN_t	 positionCanBeTrusted;
	VehicleType_t	 vehicleType;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ControlType_t;

/* VehicleLocation */
typedef struct VehicleLocation {
	long	 positionLatitude;
	long	 positionLongitude;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VehicleLocation_t;
/* VehicleLocationDelta */
typedef struct VehicleLocationDelta {
	long	 latitudeDelta;
	long	 longitudeDelta;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VehicleLocationDelta_t;

/* AdditionalData */
typedef struct AdditionalData {
	RELATIVE_OID_t	 oid;
	OCTET_STRING_t	 data;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AdditionalData_t;
/* MSDStructure */
typedef struct MSDStructure {
	long	 messageIdentifier;
	ControlType_t	 control;
	VIN_t	 vehicleIdentificationNumber;
	VehiclePropulsionStorageType_t	 vehiclePropulsionStorageType;
	unsigned long	 timestamp;
	VehicleLocation_t	 vehicleLocation;
	long	 vehicleDirection;
	struct VehicleLocationDelta	*recentVehicleLocationN1	/* OPTIONAL */;
	struct VehicleLocationDelta	*recentVehicleLocationN2	/* OPTIONAL */;
	long	*numberOfPassengers	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MSDStructure_t;
/* MSDMessage */
typedef struct MSDMessage {
	MSDStructure_t	 msdStructure;
	struct AdditionalData	*optionalAdditionalData	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MSDMessage_t;
/* ECallMessage */
typedef struct ECallMessage {
	long	 msdVersion;
	OCTET_STRING_t	 msd;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ECallMessage_t;

typedef struct OCTET_STRING {
	uint8_t *buf;	/* Buffer with consecutive OCTET_STRING bits */
	int size;	/* Size of the buffer */

	asn_struct_ctx_t _asn_ctx;	/* Parsing across buffer boundaries */
} OCTET_STRING_t;


/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ECallMessage;

#ifdef __cplusplus
}
#endif

#endif	/* _ECallMessage_H_ */
#include <asn_internal.h>
