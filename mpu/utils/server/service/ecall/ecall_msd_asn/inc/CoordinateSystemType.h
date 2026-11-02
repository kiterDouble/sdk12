#ifndef	_CoordinateSystemType_H_
#define	_CoordinateSystemType_H_


#include <asn_application.h>

#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CoordinateSystemType {
	CoordinateSystemType_wgs84	= 1,
	CoordinateSystemType_ps90	= 2
} e_CoordinateSystemType;

typedef long	 CoordinateSystemType_t;

extern asn_TYPE_descriptor_t asn_DEF_CoordinateSystemType;
asn_struct_free_f CoordinateSystemType_free;
asn_struct_print_f CoordinateSystemType_print;
asn_constr_check_f CoordinateSystemType_constraint;
ber_type_decoder_f CoordinateSystemType_decode_ber;
der_type_encoder_f CoordinateSystemType_encode_der;
xer_type_decoder_f CoordinateSystemType_decode_xer;
xer_type_encoder_f CoordinateSystemType_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _CoordinateSystemType_H_ */
#include <asn_internal.h>
