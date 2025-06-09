#ifndef RC_CALLBACKS_HPP
#define RC_CALLBACKS_HPP

#include <stdint.h>
#include "E2AP-PDU.h"

// Structures ASN.1 pour le décodage des messages RC
extern "C" {
    #include "E2SM-RC-ControlHeader.h"
    #include "E2SM-RC-ControlMessage.h"
    #include "E2AP-PDU.h"
}

// Fonction principale appelée lors de la réception d'un RICcontrolRequest
void handle_rc_control_request(E2AP_PDU_t *pdu);

// Fonctions utilitaires pour décoder les parties RC du message
void decode_and_print_rc_control_header(E2SM_RC_ControlHeader_t *header);
void decode_and_print_rc_control_message(E2SM_RC_ControlMessage_t *message);

#endif  // RC_CALLBACKS_HPP
