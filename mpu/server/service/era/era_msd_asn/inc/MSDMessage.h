

#ifndef	_MSDMessage_H_
#define	_MSDMessage_H_


#include <asn_application.h>

/* Including external dependencies */
#include "MSDStructure.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct AdditionalData;

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

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MSDMessage;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "AdditionalData.h"

#endif	/* _MSDMessage_H_ */
#include <asn_internal.h>
