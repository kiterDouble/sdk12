#include "ERAAdditionalData.h"

static int memb_crashSeverityASI15_constraint_1(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 2047)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int asn_DFL_5_set_1(int set_value, void **sptr) {
	CoordinateSystemType_t *st = *sptr;
	
	if(!st) {
		if(!set_value) return -1;	/* Not a default value */
		st = (*sptr = CALLOC(1, sizeof(*st)));
		if(!st) return -1;
	}
	
	if(set_value) {
		/* Install default value 1 */
		*st = 1;
		return 0;
	} else {
		/* Test default value 1 */
		return (*st == 1);
	}
}
static asn_TYPE_member_t asn_MBR_ERAAdditionalData_1[] = {
	{ ATF_POINTER, 4, offsetof(struct ERAAdditionalData, crashSeverityASI15),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		memb_crashSeverityASI15_constraint_1,
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashSeverityASI15"
		},
	{ ATF_POINTER, 3, offsetof(struct ERAAdditionalData, diagnosticResult),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DiagnosticResult,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"diagnosticResult"
		},
	{ ATF_POINTER, 2, offsetof(struct ERAAdditionalData, crashInfo),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CrashInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashInfo"
		},
	{ ATF_POINTER, 1, offsetof(struct ERAAdditionalData, coordinateSystemType),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CoordinateSystemType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		asn_DFL_5_set_1,	/* DEFAULT 1 */
		"coordinateSystemType"
		},
};
static const ber_tlv_tag_t asn_DEF_ERAAdditionalData_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ERAAdditionalData_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* crashSeverityASI15 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* diagnosticResult */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* crashInfo */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* coordinateSystemType */
};
static asn_SEQUENCE_specifics_t asn_SPC_ERAAdditionalData_specs_1 = {
	sizeof(struct ERAAdditionalData),
	offsetof(struct ERAAdditionalData, _asn_ctx),
	asn_MAP_ERAAdditionalData_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	3,	/* Start extensions */
	5	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ERAAdditionalData = {
	"ERAAdditionalData",
	"ERAAdditionalData",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_ERAAdditionalData_tags_1,
	sizeof(asn_DEF_ERAAdditionalData_tags_1)
		/sizeof(asn_DEF_ERAAdditionalData_tags_1[0]), /* 1 */
	asn_DEF_ERAAdditionalData_tags_1,	/* Same as above */
	sizeof(asn_DEF_ERAAdditionalData_tags_1)
		/sizeof(asn_DEF_ERAAdditionalData_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_ERAAdditionalData_1,
	4,	/* Elements count */
	&asn_SPC_ERAAdditionalData_specs_1	/* Additional specs */
};

