

#ifndef	_VehiclePropulsionStorageType_H_
#define	_VehiclePropulsionStorageType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BOOLEAN.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

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

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_VehiclePropulsionStorageType;

#ifdef __cplusplus
}
#endif

#endif	/* _VehiclePropulsionStorageType_H_ */
#include <asn_internal.h>
