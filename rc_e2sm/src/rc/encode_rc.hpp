/*******************************************************
*  Minimal E2SM-RC encoder header (analogue à KPM)     *
*******************************************************/
#ifndef ENCODE_RC_HPP
#define ENCODE_RC_HPP

extern "C" {
    #include "OCTET_STRING.h"
    #include "asn_application.h"
    #include "encode_rc.hpp"
    
    /* Structures génériques RC (générées dans asn1_generated/) */
    #include "E2SM-RC-ControlHeader.h"
    #include "E2SM-RC-ControlMessage.h"
    #include "E2SM-RC-ActionDefinition.h"
    #include "E2SM-RC-RANFunctionDefinition.h"
}

void encode_RC_RANFunctionDefinition(E2SM_RC_RANFunctionDefinition_t* ranFunc_desc);

void encode_rc_control_header(E2SM_RC_ControlHeader_t* ctrl_hdr);

void encode_rc_control_message(E2SM_RC_ControlMessage_t* ctrl_msg);

void encode_rc_action_definition(E2SM_RC_ActionDefinition_t* action_def);

#endif /* ENCODE_RC_HPP */