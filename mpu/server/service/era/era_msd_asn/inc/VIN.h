

#ifndef	_VIN_H_
#define	_VIN_H_


#include <asn_application.h>

/* Including external dependencies */
#include <PrintableString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* VIN */
typedef struct VIN {
	PrintableString_t	 isowmi;
	PrintableString_t	 isovds;
	PrintableString_t	 isovisModelyear;
	PrintableString_t	 isovisSeqPlant;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VIN_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_VIN;

#ifdef __cplusplus
}
#endif

#endif	/* _VIN_H_ */
#include <asn_internal.h>
