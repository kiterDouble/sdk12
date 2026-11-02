#ifndef	_ERAAdditionalData_H_
#define	_ERAAdditionalData_H_


#include <asn_application.h>

#include <NativeInteger.h>
#include "CoordinateSystemType.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

struct DiagnosticResult;
struct CrashInfo;

typedef struct ERAAdditionalData {
	long	*crashSeverityASI15	/* OPTIONAL */;
	struct DiagnosticResult	*diagnosticResult	/* OPTIONAL */;
	struct CrashInfo	*crashInfo	/* OPTIONAL */;
	CoordinateSystemType_t	*coordinateSystemType	/* DEFAULT 1 */;

	asn_struct_ctx_t _asn_ctx;
} ERAAdditionalData_t;

extern asn_TYPE_descriptor_t asn_DEF_ERAAdditionalData;

#ifdef __cplusplus
}
#endif

#include "DiagnosticResult.h"
#include "CrashInfo.h"

#endif	/* _ERAAdditionalData_H_ */
#include <asn_internal.h>
