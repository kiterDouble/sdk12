

#ifndef	_VehicleLocationDelta_H_
#define	_VehicleLocationDelta_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* VehicleLocationDelta */
typedef struct VehicleLocationDelta {
	long	 latitudeDelta;
	long	 longitudeDelta;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VehicleLocationDelta_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_VehicleLocationDelta;

#ifdef __cplusplus
}
#endif

#endif	/* _VehicleLocationDelta_H_ */
#include <asn_internal.h>
