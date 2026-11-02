#include "CrashInfo.h"

static asn_TYPE_member_t asn_MBR_CrashInfo_1[] = {
	{ ATF_POINTER, 8, offsetof(struct CrashInfo, crashFront),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashFront"
		},
	{ ATF_POINTER, 7, offsetof(struct CrashInfo, crashLeft),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashLeft"
		},
	{ ATF_POINTER, 6, offsetof(struct CrashInfo, crashRight),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashRight"
		},
	{ ATF_POINTER, 5, offsetof(struct CrashInfo, crashRear),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashRear"
		},
	{ ATF_POINTER, 4, offsetof(struct CrashInfo, crashRollover),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashRollover"
		},
	{ ATF_POINTER, 3, offsetof(struct CrashInfo, crashSide),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashSide"
		},
	{ ATF_POINTER, 2, offsetof(struct CrashInfo, crashFrontOrSide),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashFrontOrSide"
		},
	{ ATF_POINTER, 1, offsetof(struct CrashInfo, crashAnotherType),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"crashAnotherType"
		},
};
static const ber_tlv_tag_t asn_DEF_CrashInfo_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_CrashInfo_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* crashFront */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* crashLeft */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* crashRight */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* crashRear */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* crashRollover */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* crashSide */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* crashFrontOrSide */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 } /* crashAnotherType */
};
static asn_SEQUENCE_specifics_t asn_SPC_CrashInfo_specs_1 = {
	sizeof(struct CrashInfo),
	offsetof(struct CrashInfo, _asn_ctx),
	asn_MAP_CrashInfo_tag2el_1,
	8,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CrashInfo = {
	"CrashInfo",
	"CrashInfo",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_CrashInfo_tags_1,
	sizeof(asn_DEF_CrashInfo_tags_1)
		/sizeof(asn_DEF_CrashInfo_tags_1[0]), /* 1 */
	asn_DEF_CrashInfo_tags_1,	/* Same as above */
	sizeof(asn_DEF_CrashInfo_tags_1)
		/sizeof(asn_DEF_CrashInfo_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_CrashInfo_1,
	8,	/* Elements count */
	&asn_SPC_CrashInfo_specs_1	/* Additional specs */
};

