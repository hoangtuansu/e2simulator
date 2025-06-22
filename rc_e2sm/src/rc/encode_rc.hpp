/*******************************************************
*  Minimal E2SM-RC encoder header (analogue à KPM)     *
*******************************************************/
#ifndef ENCODE_RC_HPP
#define ENCODE_RC_HPP

extern "C" {
  #include "OCTET_STRING.h"
  #include "asn_application.h"

  /* Structures génériques RC (tu les as déjà générées
     dans asn1_generated/) */
  #include "E2SM-RC-ControlHeader.h"
  #include "E2SM-RC-ControlMessage.h"
  #include "E2SM-RC-ActionDefinition.h"
  #include "E2SM-RC-RANfunction-Description.h"
}

void encode_rc_function_description(E2SM_RC_RANfunction_Description_t* ranfunc_desc);

void encode_rc_control_header(E2SM_RC_ControlHeader_t*  ctrl_hdr /*, params à préciser*/);

void encode_rc_control_message(E2SM_RC_ControlMessage_t* ctrl_msg /*, params à préciser*/);

#endif /* ENCODE_RC_HPP */
