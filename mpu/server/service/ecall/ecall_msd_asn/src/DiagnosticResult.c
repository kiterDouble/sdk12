#include "DiagnosticResult.h"

static asn_TYPE_member_t asn_MBR_DiagnosticResult_1[] = {
	{ ATF_POINTER, 21, offsetof(struct DiagnosticResult, micConnectionFailure),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"micConnectionFailure"
		},
	{ ATF_POINTER, 20, offsetof(struct DiagnosticResult, micFailure),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"micFailure"
		},
	{ ATF_POINTER, 19, offsetof(struct DiagnosticResult, rightSpeakerFailure),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"rightSpeakerFailure"
		},
	{ ATF_POINTER, 18, offsetof(struct DiagnosticResult, leftSpeakerFailure),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"leftSpeakerFailure"
		},
	{ ATF_POINTER, 17, offsetof(struct DiagnosticResult, speakersFailure),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"speakersFailure"
		},
	{ ATF_POINTER, 16, offsetof(struct DiagnosticResult, ignitionLineFailure),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ignitionLineFailure"
		},
	{ ATF_POINTER, 15, offsetof(struct DiagnosticResult, uimFailure),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"uimFailure"
		},
	{ ATF_POINTER, 14, offsetof(struct DiagnosticResult, statusIndicatorFailure),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"statusIndicatorFailure"
		},
	{ ATF_POINTER, 13, offsetof(struct DiagnosticResult, batteryFailure),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"batteryFailure"
		},
	{ ATF_POINTER, 12, offsetof(struct DiagnosticResult, batteryVoltageLow),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"batteryVoltageLow"
		},
	{ ATF_POINTER, 11, offsetof(struct DiagnosticResult, crashSensorFailure),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashSensorFailure"
		},
	{ ATF_POINTER, 10, offsetof(struct DiagnosticResult, firmwareImageCorruption),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"firmwareImageCorruption"
		},
	{ ATF_POINTER, 9, offsetof(struct DiagnosticResult, commModuleInterfaceFailure),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"commModuleInterfaceFailure"
		},
	{ ATF_POINTER, 8, offsetof(struct DiagnosticResult, gnssReceiverFailure),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gnssReceiverFailure"
		},
	{ ATF_POINTER, 7, offsetof(struct DiagnosticResult, raimProblem),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"raimProblem"
		},
	{ ATF_POINTER, 6, offsetof(struct DiagnosticResult, gnssAntennaFailure),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gnssAntennaFailure"
		},
	{ ATF_POINTER, 5, offsetof(struct DiagnosticResult, commModuleFailure),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"commModuleFailure"
		},
	{ ATF_POINTER, 4, offsetof(struct DiagnosticResult, eventsMemoryOverflow),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"eventsMemoryOverflow"
		},
	{ ATF_POINTER, 3, offsetof(struct DiagnosticResult, crashProfileMemoryOverflow),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashProfileMemoryOverflow"
		},
	{ ATF_POINTER, 2, offsetof(struct DiagnosticResult, otherCriticalFailires),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"otherCriticalFailires"
		},
	{ ATF_POINTER, 1, offsetof(struct DiagnosticResult, otherNotCriticalFailures),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"otherNotCriticalFailures"
		},
};
static const ber_tlv_tag_t asn_DEF_DiagnosticResult_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_DiagnosticResult_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* micConnectionFailure */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* micFailure */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* rightSpeakerFailure */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* leftSpeakerFailure */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* speakersFailure */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* ignitionLineFailure */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* uimFailure */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* statusIndicatorFailure */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* batteryFailure */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* batteryVoltageLow */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* crashSensorFailure */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* firmwareImageCorruption */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* commModuleInterfaceFailure */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* gnssReceiverFailure */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* raimProblem */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* gnssAntennaFailure */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* commModuleFailure */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* eventsMemoryOverflow */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* crashProfileMemoryOverflow */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* otherCriticalFailires */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 } /* otherNotCriticalFailures */
};
static asn_SEQUENCE_specifics_t asn_SPC_DiagnosticResult_specs_1 = {
	sizeof(struct DiagnosticResult),
	offsetof(struct DiagnosticResult, _asn_ctx),
	asn_MAP_DiagnosticResult_tag2el_1,
	21,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_DiagnosticResult = {
	"DiagnosticResult",
	"DiagnosticResult",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_DiagnosticResult_tags_1,
	sizeof(asn_DEF_DiagnosticResult_tags_1)
		/sizeof(asn_DEF_DiagnosticResult_tags_1[0]), /* 1 */
	asn_DEF_DiagnosticResult_tags_1,	/* Same as above */
	sizeof(asn_DEF_DiagnosticResult_tags_1)
		/sizeof(asn_DEF_DiagnosticResult_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_DiagnosticResult_1,
	21,	/* Elements count */
	&asn_SPC_DiagnosticResult_specs_1	/* Additional specs */
};

