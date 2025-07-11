/*******************************************************
*  Encoders “stub” pour E2SM-RC (analogue KPM)         *
*******************************************************/
#include "encode_rc.hpp"
#include <cstring>
#include "e2sim_defs.h"   // LOG_* macros

/* ------------------------------------------------------------------ */
/* 1) RAN-function description – très light :                         */
/*    juste de quoi passer la phase d’init (à enrichir plus tard).    */
void encode_rc_FunctionDefinition(E2SM_RC_RANFunctionDefinition_t* ranFunc_def)
{
  uint8_t* short_name = (uint8_t*)"ORAN-E2SM-RC";
  uint8_t* desc       = (uint8_t*)"RAN Control";
  uint8_t* oid        = (uint8_t*)"OID-RC";

  ASN_STRUCT_RESET(asn_DEF_E2SM_RC_RANfunction_Description, ranfunc_desc);

  /* ShortName ------------------------------------------------------ */
  ranfunc_desc->ranFunction_Name.ranFunction_ShortName.size = strlen((char*)short_name);
  ranfunc_desc->ranFunction_Name.ranFunction_ShortName.buf  =
        (uint8_t*)calloc(ranfunc_desc->ranFunction_Name.ranFunction_ShortName.size, 1);
  memcpy(ranfunc_desc->ranFunction_Name.ranFunction_ShortName.buf,
         short_name,
         ranfunc_desc->ranFunction_Name.ranFunction_ShortName.size);

  /* Description ---------------------------------------------------- */
  ranfunc_desc->ranFunction_Name.ranFunction_Description.buf =
        (uint8_t*)calloc(strlen((char*)desc), 1);
  memcpy(ranfunc_desc->ranFunction_Name.ranFunction_Description.buf,
         desc,
         strlen((char*)desc));
  ranfunc_desc->ranFunction_Name.ranFunction_Description.size = strlen((char*)desc);

  /* OID ------------------------------------------------------------ */
  ranfunc_desc->ranFunction_Name.ranFunction_E2SM_OID.buf =
        (uint8_t*)calloc(strlen((char*)oid), 1);
  memcpy(ranfunc_desc->ranFunction_Name.ranFunction_E2SM_OID.buf,
         oid,
         strlen((char*)oid));
  ranfunc_desc->ranFunction_Name.ranFunction_E2SM_OID.size = strlen((char*)oid);

  /* Instance ------------------------------------------------------- */
  ranfunc_desc->ranFunction_Name.ranFunction_Instance = (long*)calloc(1,sizeof(long));
  *ranfunc_desc->ranFunction_Name.ranFunction_Instance = 1;
}

/* ------------------------------------------------------------------ */
/* 2) Control-Header : structure vide “valide” pour l’instant.        */
void encode_rc_control_header(E2SM_RC_ControlHeader_t* ctrl_hdr /*, params */)
{
  ASN_STRUCT_RESET(asn_DEF_E2SM_RC_ControlHeader, ctrl_hdr);
  /* TODO : remplir les champs selon le scénario de test               */
}

/* ------------------------------------------------------------------ */
/* 3) Control-Message : structure vide “valide” pour l’instant.       */
void encode_rc_control_message(E2SM_RC_ControlMessage_t* ctrl_msg /*, params */)
{
  ASN_STRUCT_RESET(asn_DEF_E2SM_RC_ControlMessage, ctrl_msg);
  /* TODO : remplir les champs selon le scénario de test               */
}

