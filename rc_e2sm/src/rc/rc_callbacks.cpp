/*****************************************************************************
#                                                                            *
#   Adaptation “Radio Control” (E2SM-RC) du simulateur KPM.                  *
#   Inspiré de kpm_callbacks.cpp                                             *
#                                                                            *
#   TODO : implémenter les helpers d’encodage RC dans encode_rc.*            *
#                                                                            *
*****************************************************************************/

#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <random>
#include <cstdlib>

extern "C" {
  /* ---------- ASN.1 auto-généré RC ---------- */
  #include "OCTET_STRING.h"
  #include "asn_application.h"

  /* ===  E2SM-RC messages  === */
  #include "E2SM-RC-ControlHeader.h"
  #include "E2SM-RC-ControlHeader-Format1.h"
  #include "E2SM-RC-ControlMessage.h"
  #include "E2SM-RC-ControlMessage-Format1.h"
  #include "E2SM-RC-ActionDefinition.h"
  #include "E2SM-RC-RANfunction-Description.h"
  #include "E2SM-RC-EventTriggerDefinition.h"

  /* ===  E2AP  === */
  #include "E2AP-PDU.h"
  #include "RICsubscriptionRequest.h"
  #include "RICsubscriptionResponse.h"
  #include "RICactionType.h"
  #include "ProtocolIE-Field.h"
  #include "ProtocolIE-SingleContainer.h"
  #include "InitiatingMessage.h"
}

#include "rc_callbacks.hpp"
#include "encode_rc.hpp"          // <-- tes helpers d’encodage RC
#include "encode_e2apv1.hpp"
#include "e2sim_defs.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;
using namespace std::chrono;

/*****************************************************************
 * Globals
 *****************************************************************/
class E2Sim;
static E2Sim g_e2sim;
static int   gFuncId = 0;

/*****************************************************************
 * Helpers
 *****************************************************************/

static void build_and_send_rc_control(long reqId,
                                      long reqInstId,
                                      long ranFuncId,
                                      long actionId,
                                      uint8_t* controlHeaderBuf,
                                      size_t   controlHeaderLen,
                                      uint8_t* controlMessageBuf,
                                      size_t   controlMessageLen,
                                      long&    seqNum)
{
  /* Construit l’Indication / Control E2AP puis envoie via SCTP */
  E2AP_PDU_t* pdu = (E2AP_PDU_t*)calloc(1, sizeof(E2AP_PDU_t));

  // TODO : choisis la primitive E2AP adaptée :
  //        ici démonstration avec un ControlRequest
  encoding::generate_e2apv1_control_request_parameterized(
        pdu,
        reqId,
        reqInstId,
        ranFuncId,
        actionId,
        seqNum,
        controlHeaderBuf,
        controlHeaderLen,
        controlMessageBuf,
        controlMessageLen);

  g_e2sim.encode_and_send_sctp_data(pdu);
  seqNum++;
}

/*****************************************************************
 * Boucle d’envoi périodique des contrôles RC
 *****************************************************************/
void run_rc_report_loop(long requestorId,
                        long instanceId,
                        long ranFunctionId,
                        long actionId)
{
  long seqNum = 1;

  /* Exemple : lecture d’un fichier JSON décrivant les commandes RC
   * ----------------------------------------------------------------
   * Format attendu :
   * [
   *   { "pci" : 42, "txPower" : -3 },
   *   { "pci" : 43, "txPower" : -1 }
   * ]
   */
  std::ifstream inJson("/opt/e2sim/rc_e2sm/commands.json");
  if (!inJson.good()) {
    LOG_E("Unable to open commands.json — boucle RC arrêtée");
    return;
  }

  json commands = json::parse(inJson);

  for (const auto& cmd : commands) {
    /* ---------- 1. Construire le ControlHeader ---------- */
    E2SM_RC_ControlHeader_t* hdr = (E2SM_RC_ControlHeader_t*)calloc(1, sizeof(*hdr));
    // TODO : remplir hdr->choice...
    encode_rc_control_header_format1(hdr, cmd);

    uint8_t hdrBuf[1024] = {0};
    size_t  hdrLen      = encode_rc_to_buffer(hdr, hdrBuf, sizeof(hdrBuf));
    ASN_STRUCT_FREE(asn_DEF_E2SM_RC_ControlHeader, hdr);

    /* ---------- 2. Construire le ControlMessage ---------- */
    E2SM_RC_ControlMessage_t* msg = (E2SM_RC_ControlMessage_t*)calloc(1, sizeof(*msg));
    encode_rc_control_message_format1(msg, cmd);

    uint8_t msgBuf[1024] = {0};
    size_t  msgLen       = encode_rc_to_buffer(msg, msgBuf, sizeof(msgBuf));
    ASN_STRUCT_FREE(asn_DEF_E2SM_RC_ControlMessage, msg);

    /* ---------- 3. Packager et envoyer ---------- */
    build_and_send_rc_control(requestorId, instanceId, ranFunctionId,
                              actionId, hdrBuf, hdrLen, msgBuf, msgLen, seqNum);

    /* Pause — par exemple 1 s */
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

/*****************************************************************
 * Callback — réception de RIC Subscription Request
 *****************************************************************/
void callback_rc_subscription_request(E2AP_PDU_t* subReqPdu)
{
  /* Étapes :
   * 1. Extraire Requestor/Instance ID
   * 2. Scanner la RICaction_ToBeSetup_List
   * 3. Accepter la première action REPORT, rejeter les autres
   * 4. Répondre avec RICsubscriptionResponse
   * 5. Lancer run_rc_report_loop()
   */

  long reqRequestorId = 0;
  long reqInstanceId  = 0;
  long reqActionId    = 0;
  std::vector<long> actionIdsAccept;
  std::vector<long> actionIdsReject;

  RICsubscriptionRequest_t& req =
       subReqPdu->choice.initiatingMessage->value.choice.RICsubscriptionRequest;

  for (int i = 0; i < req.protocolIEs.list.count; ++i) {
    RICsubscriptionRequest_IEs_t* ie = req.protocolIEs.list.array[i];

    switch (ie->id) {
      case ProtocolIE_ID_id_RICrequestID: {
        reqRequestorId = ie->value.choice.RICrequestID.ricRequestorID;
        reqInstanceId  = ie->value.choice.RICrequestID.ricInstanceID;
        break;
      }
      case ProtocolIE_ID_id_RICsubscriptionDetails: {
        auto& list = ie->value.choice.RICsubscriptionDetails.ricAction_ToBeSetup_List;
        bool accepted = false;

        for (int j = 0; j < list.list.count; ++j) {
          auto* itemIE   = list.list.array[j];
          auto& item     = itemIE->value.choice.RICaction_ToBeSetup_Item;
          long  actionId = item.ricActionID;

          if (!accepted && item.ricActionType == RICactionType_report) {
            actionIdsAccept.push_back(actionId);
            reqActionId = actionId;
            accepted = true;
          } else {
            actionIdsReject.push_back(actionId);
          }
        }
        break;
      }
      default:
        break;
    }
  }

  /* ---------- Construire SubscriptionResponse (succès) ---------- */
  E2AP_PDU_t* respPdu = (E2AP_PDU_t*)calloc(1, sizeof(*respPdu));
  encoding::generate_e2apv1_subscription_response_success(
        respPdu,
        actionIdsAccept.data(),
        actionIdsReject.data(),
        actionIdsAccept.size(),
        actionIdsReject.size(),
        reqRequestorId,
        reqInstanceId);

  g_e2sim.encode_and_send_sctp_data(respPdu);

  /* ---------- Lancer la boucle d’envoi RC ---------- */
  std::thread(run_rc_report_loop,
              reqRequestorId,
              reqInstanceId,
              gFuncId,
              reqActionId).detach();
}

/*****************************************************************
 * main() — équivalent RC
 *****************************************************************/
int main(int argc, char* argv[])
{
  LOG_I("Starting RC simulator");

  /* -------- 1. Charger la RAN-function-description (E2SM-RC) -------- */
  asn_codec_ctx_t* opt_cod = nullptr;
  E2SM_RC_RANfunction_Description_t* ranFuncDesc =
      (E2SM_RC_RANfunction_Description_t*)calloc(1, sizeof(*ranFuncDesc));

  encode_rc_function_description(ranFuncDesc);   // TODO : à implémenter

  uint8_t buf[8192] = {0};
  size_t  bufSz     = sizeof(buf);

  auto er = asn_encode_to_buffer(opt_cod,
                                 ATS_ALIGNED_BASIC_PER,
                                 &asn_DEF_E2SM_RC_RANfunction_Description,
                                 ranFuncDesc,
                                 buf,
                                 bufSz);
  if (er.encoded <= 0) {
    LOG_E("Failed to encode RC RAN Func Description");
    exit(1);
  }

  /* -------- 2. Enregistrer la RAN Function auprès d’E2Sim -------- */
  std::random_device rd; std::mt19937 gen(rd());
  std::uniform_int_distribution<> d(0,4095);
  gFuncId = d(gen);

  OCTET_STRING_t* descOct = (OCTET_STRING_t*)calloc(1, sizeof(*descOct));
  descOct->buf  = (uint8_t*)calloc(1, er.encoded);
  descOct->size = er.encoded;
  memcpy(descOct->buf, buf, er.encoded);

  g_e2sim.register_e2sm(gFuncId, descOct);
  g_e2sim.register_subscription_callback(gFuncId, &callback_rc_subscription_request);

  /* -------- 3. Boucle principale -------- */
  g_e2sim.run_loop(argc, argv);
}
