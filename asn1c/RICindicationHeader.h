/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2AP-IEs"
 * 	found in "e2ap-ied-v03.01.asn"
 * 	`asn1c -pdu=auto -fincludes-quoted -fcompound-names -findirect-choice -fno-include-deps -no-gen-example -no-gen-OER -D /tmp/workspace/oransim-gerrit/e2sim/asn1c/`
 */

#ifndef	_RICindicationHeader_H_
#define	_RICindicationHeader_H_


#include "asn_application.h"

/* Including external dependencies */
#include "OCTET_STRING.h"

#ifdef __cplusplus
extern "C" {
#endif

/* RICindicationHeader */
typedef OCTET_STRING_t	 RICindicationHeader_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RICindicationHeader;
asn_struct_free_f RICindicationHeader_free;
asn_struct_print_f RICindicationHeader_print;
asn_constr_check_f RICindicationHeader_constraint;
ber_type_decoder_f RICindicationHeader_decode_ber;
der_type_encoder_f RICindicationHeader_encode_der;
xer_type_decoder_f RICindicationHeader_decode_xer;
xer_type_encoder_f RICindicationHeader_encode_xer;
jer_type_encoder_f RICindicationHeader_encode_jer;
per_type_decoder_f RICindicationHeader_decode_uper;
per_type_encoder_f RICindicationHeader_encode_uper;
per_type_decoder_f RICindicationHeader_decode_aper;
per_type_encoder_f RICindicationHeader_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _RICindicationHeader_H_ */
#include "asn_internal.h"
