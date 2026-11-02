#ifndef	_ERADataFormatId_H_
#define	_ERADataFormatId_H_


#include <asn_application.h>

#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef long	 ERADataFormatId_t;

extern asn_TYPE_descriptor_t asn_DEF_ERADataFormatId;
asn_struct_free_f ERADataFormatId_free;
asn_struct_print_f ERADataFormatId_print;
asn_constr_check_f ERADataFormatId_constraint;
ber_type_decoder_f ERADataFormatId_decode_ber;
der_type_encoder_f ERADataFormatId_encode_der;
xer_type_decoder_f ERADataFormatId_decode_xer;
xer_type_encoder_f ERADataFormatId_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _ERADataFormatId_H_ */
#include <asn_internal.h>
