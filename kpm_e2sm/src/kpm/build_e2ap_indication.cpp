
#include <vector>
#include <cstring>
#include <iostream>

extern "C" {
#include "E2AP-PDU.h"
#include "InitiatingMessage.h"
#include "RICindication.h"
#include "RICindicationHeader.h"
#include "RICindicationMessage.h"
#include "ProtocolIE-Field.h"
#include "asn_application.h"
#include "asn_internal.h"
}

// Construction simplifiée d'un message E2AP avec encapsulation d'un message KPM encodé
bool build_e2ap_indication(const std::vector<uint8_t>& kpm_encoded,
                           std::vector<uint8_t>& e2ap_output,
                           int request_id = 1,
                           int ran_func_id = 1) {
    E2AP_PDU_t* pdu = (E2AP_PDU_t*)calloc(1, sizeof(E2AP_PDU_t));
    if (!pdu) return false;

    // Type: InitiatingMessage -> RICindication
    pdu->present = E2AP_PDU_PR_initiatingMessage;
    pdu->choice.initiatingMessage = (InitiatingMessage_t*)calloc(1, sizeof(InitiatingMessage_t));
    InitiatingMessage_t* init_msg = pdu->choice.initiatingMessage;

    init_msg->procedureCode = ProcedureCode_id_RICindication;
    init_msg->criticality = Criticality_ignore;
    init_msg->value.present = InitiatingMessage__value_PR_RICindication;

    RICindication_t* indication = &init_msg->value.choice.RICindication;

    // RIC Request ID
    RICindication_IEs_t* ie_req = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));
    ie_req->id = ProtocolIE_ID_id_RICrequestID;
    ie_req->criticality = Criticality_ignore;
    ie_req->value.present = RICindication_IEs__value_PR_RICrequestID;
    ie_req->value.choice.RICrequestID.ricRequestorID = request_id;
    ie_req->value.choice.RICrequestID.ricInstanceID = 0;
    ASN_SEQUENCE_ADD(&indication->protocolIEs.list, ie_req);

    // RAN Function ID
    RICindication_IEs_t* ie_func = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));
    ie_func->id = ProtocolIE_ID_id_RANfunctionID;
    ie_func->criticality = Criticality_ignore;
    ie_func->value.present = RICindication_IEs__value_PR_RANfunctionID;
    ie_func->value.choice.RANfunctionID = ran_func_id;
    ASN_SEQUENCE_ADD(&indication->protocolIEs.list, ie_func);

    // Indication Header (vide ou fictif ici)
    RICindication_IEs_t* ie_hdr = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));
    ie_hdr->id = ProtocolIE_ID_id_RICindicationHeader;
    ie_hdr->criticality = Criticality_ignore;
    ie_hdr->value.present = RICindication_IEs__value_PR_RICindicationHeader;
    ie_hdr->value.choice.RICindicationHeader.buf = (uint8_t*)calloc(1, 1);
    ie_hdr->value.choice.RICindicationHeader.buf[0] = 0x00;
    ie_hdr->value.choice.RICindicationHeader.size = 1;
    ASN_SEQUENCE_ADD(&indication->protocolIEs.list, ie_hdr);

    // Indication Message
    RICindication_IEs_t* ie_msg = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));
    ie_msg->id = ProtocolIE_ID_id_RICindicationMessage;
    ie_msg->criticality = Criticality_ignore;
    ie_msg->value.present = RICindication_IEs__value_PR_RICindicationMessage;
    ie_msg->value.choice.RICindicationMessage.buf = (uint8_t*)calloc(1, kpm_encoded.size());
    memcpy(ie_msg->value.choice.RICindicationMessage.buf, kpm_encoded.data(), kpm_encoded.size());
    ie_msg->value.choice.RICindicationMessage.size = kpm_encoded.size();
    ASN_SEQUENCE_ADD(&indication->protocolIEs.list, ie_msg);

    // Encodage PER
    uint8_t buffer[4096];
    asn_enc_rval_t er = asn_encode_to_buffer(nullptr, ATS_ALIGNED_BASIC_PER,
        &asn_DEF_E2AP_PDU, pdu, buffer, sizeof(buffer));

    if (er.encoded > 0) {
        e2ap_output.assign(buffer, buffer + er.encoded);
        ASN_STRUCT_FREE(asn_DEF_E2AP_PDU, pdu);
        return true;
    } else {
        std::cerr << "[E2AP ERROR] Encoding failed: " << er.encoded << std::endl;
        ASN_STRUCT_FREE(asn_DEF_E2AP_PDU, pdu);
        return false;
    }
}



