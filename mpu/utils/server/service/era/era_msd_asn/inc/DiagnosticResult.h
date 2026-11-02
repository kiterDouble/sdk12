#ifndef	_DiagnosticResult_H_
#define	_DiagnosticResult_H_


#include <asn_application.h>

#include <BOOLEAN.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DiagnosticResult {
	BOOLEAN_t	*micConnectionFailure	/* OPTIONAL */;
	BOOLEAN_t	*micFailure	/* OPTIONAL */;
	BOOLEAN_t	*rightSpeakerFailure	/* OPTIONAL */;
	BOOLEAN_t	*leftSpeakerFailure	/* OPTIONAL */;
	BOOLEAN_t	*speakersFailure	/* OPTIONAL */;
	BOOLEAN_t	*ignitionLineFailure	/* OPTIONAL */;
	BOOLEAN_t	*uimFailure	/* OPTIONAL */;
	BOOLEAN_t	*statusIndicatorFailure	/* OPTIONAL */;
	BOOLEAN_t	*batteryFailure	/* OPTIONAL */;
	BOOLEAN_t	*batteryVoltageLow	/* OPTIONAL */;
	BOOLEAN_t	*crashSensorFailure	/* OPTIONAL */;
	BOOLEAN_t	*firmwareImageCorruption	/* OPTIONAL */;
	BOOLEAN_t	*commModuleInterfaceFailure	/* OPTIONAL */;
	BOOLEAN_t	*gnssReceiverFailure	/* OPTIONAL */;
	BOOLEAN_t	*raimProblem	/* OPTIONAL */;
	BOOLEAN_t	*gnssAntennaFailure	/* OPTIONAL */;
	BOOLEAN_t	*commModuleFailure	/* OPTIONAL */;
	BOOLEAN_t	*eventsMemoryOverflow	/* OPTIONAL */;
	BOOLEAN_t	*crashProfileMemoryOverflow	/* OPTIONAL */;
	BOOLEAN_t	*otherCriticalFailires	/* OPTIONAL */;
	BOOLEAN_t	*otherNotCriticalFailures	/* OPTIONAL */;
	
	asn_struct_ctx_t _asn_ctx;
} DiagnosticResult_t;

extern asn_TYPE_descriptor_t asn_DEF_DiagnosticResult;

#ifdef __cplusplus
}
#endif

#endif	/* _DiagnosticResult_H_ */
#include <asn_internal.h>
