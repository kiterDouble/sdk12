
#ifndef	_VehicleLocation_H_
#define	_VehicleLocation_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* VehicleLocation */
typedef struct VehicleLocation {
	long	 positionLatitude;
	long	 positionLongitude;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VehicleLocation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_VehicleLocation;

#ifdef __cplusplus
}
#endif

#endif	/* _VehicleLocation_H_ */
#include <asn_internal.h>
