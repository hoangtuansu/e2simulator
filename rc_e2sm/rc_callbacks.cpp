// rc_callbacks.cpp

#include "rc_callbacks.hpp"
#include "E2AP-PDU.h"
#include "RICcontrolRequest.h"
#include "E2SM-RC-ControlHeader.h"
#include "E2SM-RC-ControlMessage.h"
#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "xer_decoder.h"
#include "xer_encoder.h"
}

void handle_rc_control_request(E2AP_PDU_t* pdu) {
    if (pdu->present != E2AP_PDU_PR_initiatingMessage) {
        printf("[RC] Not an initiatingMessage\n");
        return;
    }

    InitiatingMessage_t* initiatingMsg = pdu->choice.initiatingMessage;
    if (initiatingMsg->procedureCode != ProcedureCode_id_RICcontrol) {
        printf("[RC] Not a RICcontrol message\n");
        return;
    }

    RICcontrolRequest_t* controlReq = (RICcontrolRequest_t*)initiatingMsg->value.choice.RICcontrolRequest;
    for (int i = 0; i < controlReq->protocolIEs.list.count; i++) {
        RICcontrolRequest_IEs_t* ie = controlReq->protocolIEs.list.array[i];

        if (ie->id == ProtocolIE_ID_id_RICcontrolHeader) {
            E2SM_RC_ControlHeader_t* header = nullptr;
            xer_decode(0, &asn_DEF_E2SM_RC_ControlHeader, (void**)&header,
                       (char*)ie->value.choice.RICcontrolHeader.buf,
                       ie->value.choice.RICcontrolHeader.size);

            printf("\n[RC] ControlHeader received:\n");
            xer_fprint(stdout, &asn_DEF_E2SM_RC_ControlHeader, header);
        }

        if (ie->id == ProtocolIE_ID_id_RICcontrolMessage) {
            E2SM_RC_ControlMessage_t* message = nullptr;
            xer_decode(0, &asn_DEF_E2SM_RC_ControlMessage, (void**)&message,
                       (char*)ie->value.choice.RICcontrolMessage.buf,
                       ie->value.choice.RICcontrolMessage.size);

            printf("\n[RC] ControlMessage received:\n");
            xer_fprint(stdout, &asn_DEF_E2SM_RC_ControlMessage, message);
        }
    }
}
