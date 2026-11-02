#ifndef	_CrashInfo_H_
#define	_CrashInfo_H_


#include <asn_application.h>

#include <BOOLEAN.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrashInfo {
	BOOLEAN_t	*crashFront	/* OPTIONAL */;
	BOOLEAN_t	*crashLeft	/* OPTIONAL */;
	BOOLEAN_t	*crashRight	/* OPTIONAL */;
	BOOLEAN_t	*crashRear	/* OPTIONAL */;
	BOOLEAN_t	*crashRollover	/* OPTIONAL */;
	BOOLEAN_t	*crashSide	/* OPTIONAL */;
	BOOLEAN_t	*crashFrontOrSide	/* OPTIONAL */;
	BOOLEAN_t	*crashAnotherType	/* OPTIONAL */;
	
	asn_struct_ctx_t _asn_ctx;
} CrashInfo_t;

extern asn_TYPE_descriptor_t asn_DEF_CrashInfo;

#ifdef __cplusplus
}
#endif

#endif	/* _CrashInfo_H_ */
#include <asn_internal.h>
