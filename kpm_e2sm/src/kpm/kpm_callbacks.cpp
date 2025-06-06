/*****************************************************************************
#                                                                            *
# Copyright 2020 AT&T Intellectual Property                                  *
#                                                                            *
# Licensed under the Apache License, Version 2.0 (the "License");            *
# you may not use this file except in compliance with the License.           *
# You may obtain a copy of the License at                                    *
#                                                                            *
#      http://www.apache.org/licenses/LICENSE-2.0                            *
#                                                                            *
# Unless required by applicable law or agreed to in writing, software        *
# distributed under the License is distributed on an "AS IS" BASIS,          *
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
# See the License for the specific language governing permissions and        *
# limitations under the License.                                             *
#                                                                            *
******************************************************************************/

#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>

extern "C" {
  #include "OCTET_STRING.h"
  #include "asn_application.h"
  #include "E2SM-KPM-IndicationMessage.h"
  #include "E2SM-KPM-RANfunction-Description.h"
  #include "E2SM-KPM-IndicationHeader-Format1.h"
  #include "E2SM-KPM-ActionDefinition.h"
  #include "E2SM-KPM-IndicationHeader.h"
  #include "E2AP-PDU.h"
  #include "RICsubscriptionRequest.h"
  #include "RICsubscriptionResponse.h"
  #include "RICactionType.h"
  #include "ProtocolIE-Field.h"
  #include "ProtocolIE-SingleContainer.h"
  #include "InitiatingMessage.h"
}

#include "kpm_callbacks.hpp"
#include "encode_kpm.hpp"
#include "encode_e2apv1.hpp"

#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <time.h>
#include <random>

#include "viavi_connector.hpp"
#include "errno.h"
#include "e2sim_defs.h"
#include <cstdlib>
#include <strasser/csv.h>


using json = nlohmann::json;
using namespace std;
using namespace io;
using namespace std::chrono;

class E2Sim;
int gFuncId;

E2Sim e2sim;

#define NUMBER_OF_METRICS 8

int main(int argc, char* argv[]) {
  LOG_I("Starting KPM simulator");

  uint8_t *nrcellid_buf = (uint8_t*)calloc(1, 5);
  nrcellid_buf[0] = 0x22;
  nrcellid_buf[1] = 0x5B;
  nrcellid_buf[2] = 0xD6;
  nrcellid_buf[3] = 0x00;
  nrcellid_buf[4] = 0x70;

  std::ifstream ifs("/opt/e2sim/kpm_e2sm/config.json");
  json e2sim_config = json::parse(ifs);

  int numPMs = e2sim_config["pm"].size();
  LOG_D("There are %d metrics", numPMs);

  std::string pms[numPMs];
  std::string pm_str = "";

  for (int i = 0; i < numPMs; i++) {
    json::json_pointer pm(std::string("/pm/") + std::to_string(i));
    pms[i] = e2sim_config[pm].get<std::string>();
    pm_str = pm_str + pms[i] + (i == (numPMs - 1) ? "" : ", ");
  }

  LOG_I("List of metrics: %s", pm_str.c_str());

  asn_codec_ctx_t *opt_cod;

  E2SM_KPM_RANfunction_Description_t *ranfunc_desc =
    (E2SM_KPM_RANfunction_Description_t*)calloc(1, sizeof(E2SM_KPM_RANfunction_Description_t));
  encode_kpm_function_description(ranfunc_desc);

  uint8_t e2smbuffer[8192] = {0, };
  size_t e2smbuffer_size = 8192;

  asn_enc_rval_t er =
    asn_encode_to_buffer(opt_cod,
                         ATS_ALIGNED_BASIC_PER,
                         &asn_DEF_E2SM_KPM_RANfunction_Description,
                         ranfunc_desc, e2smbuffer, e2smbuffer_size);

  if (er.encoded == -1) {
    LOG_I("Failed to serialize function description data. Detail: %s.", asn_DEF_E2SM_KPM_RANfunction_Description.name);
  } else if (er.encoded > e2smbuffer_size) {
    LOG_I("Buffer of size %zu is too small for %s, need %zu", e2smbuffer_size, asn_DEF_E2SM_KPM_RANfunction_Description.name, er.encoded);
  }

  uint8_t *ranfuncdesc = (uint8_t*)calloc(1, er.encoded);
  memcpy(ranfuncdesc, e2smbuffer, er.encoded);

  OCTET_STRING_t *ranfunc_ostr = (OCTET_STRING_t*)calloc(1, sizeof(OCTET_STRING_t));
  ranfunc_ostr->buf = (uint8_t*)calloc(1, er.encoded);
  ranfunc_ostr->size = er.encoded;
  memcpy(ranfunc_ostr->buf, e2smbuffer, er.encoded);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, 4095); // range [0, 255]

  const char* func_id_str = std::getenv("RAN_FUNC_ID");
  ::gFuncId = func_id_str == nullptr ? distrib(gen) : std::stoi(func_id_str);

  e2sim.register_e2sm(gFuncId, ranfunc_ostr);
  e2sim.register_subscription_callback(gFuncId, &callback_kpm_subscription_request);
  e2sim.run_loop(argc, argv);
}

void get_cell_id(uint8_t *nrcellid_buf, char *cid_return_buf) {
  uint8_t nr0 = nrcellid_buf[0] >> 4;
  uint8_t nr1 = nrcellid_buf[0] << 4;
  nr1 = nr1 >> 4;

  uint8_t nr2 = nrcellid_buf[1] >> 4;
  uint8_t nr3 = nrcellid_buf[1] << 4;
  nr3 = nr3 >> 4;

  uint8_t nr4 = nrcellid_buf[2] >> 4;
  uint8_t nr5 = nrcellid_buf[2] << 4;
  nr5 = nr5 >> 4;

  uint8_t nr6 = nrcellid_buf[3] >> 4;
  uint8_t nr7 = nrcellid_buf[3] << 4;
  nr7 = nr7 >> 4;

  uint8_t nr8 = nrcellid_buf[4] >> 4;

  sprintf(cid_return_buf, "373437%d%d%d%d%d%d%d%d%d", nr0, nr1, nr2, nr3, nr4, nr5, nr6, nr7, nr8);
}

void run_report_loop(long requestorId, long instanceId, long ranFunctionId, long actionId) {
  std::string filename = "/opt/e2sim/kpm_e2sm/data.csv";

  try {
      // io::CSVReader because we expect 5 columns
      io::CSVReader<NUMBER_OF_METRICS> in(filename);

      

      // Read the header line, mapping to variables.
      // The library automatically handles type conversion based on the variable types you provide.
      in.read_header(io::ignore_extra_column,
                      "throughput",
                      "pdcpBytesDl",
                      "pdcpBytesUl",
                      "availPrbDl",
                      "availPrbUl",
                      "rsrp",
                      "rsrq",
                      "rssinr");

      double throughput;
      double pdcpBytesDl;
      double pdcpBytesUl;
      double availPrbDl;
      double availPrbUl;
      double rsrp;
      double rsrq;
      double rssinr;
      long seqNum = 1;

      std::cout << "Reading products.csv (ben-strasser/fast-cpp-csv-parser):\n";


      while (in.read_row(throughput, pdcpBytesDl, pdcpBytesUl, availPrbDl, 
                        availPrbUl, rsrp, rsrq, rssinr)) {
          std::cout << throughput << ", " 
                    << pdcpBytesDl << ", " 
                    << pdcpBytesUl << ", " 
                    << availPrbDl << ", " 
                    << availPrbUl << ", " 
                    << rsrp << ", " 
                    << rsrq << ", " 
                    << rssinr
                    << std::endl;

          LOG_I("Start sending E2Node measurement reports");

          asn_codec_ctx_t *opt_cod2;

          E2SM_KPM_IndicationMessage_t *ind_message_style1 = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
          E2AP_PDU *pdu_style1 = (E2AP_PDU*)calloc(1, sizeof(E2AP_PDU));
          
          const char* cell_pms_labels[NUMBER_OF_METRICS] = {"throughput", "pdcpBytesDl", "pdcpBytesUl", "availPrbDl", "availPrbUl", "rsrp", "rsrq", "rssinr"};
          double cell_pms_values[NUMBER_OF_METRICS] = {throughput, pdcpBytesDl, pdcpBytesUl, availPrbDl, availPrbUl, rsrp, rsrq, rssinr};

          kpm_report_indication_message_initialized(ind_message_style1, cell_pms_labels, cell_pms_values, NUMBER_OF_METRICS);

          LOG_I("E2SM KPM Indication message:");
          xer_fprint(stderr, &asn_DEF_E2SM_KPM_IndicationMessage, ind_message_style1);

          uint8_t e2sm_message_buf_style1[8192] = {0, };
          size_t e2sm_message_buf_size_style1 = 8192;

          asn_enc_rval_t er_message_style1 = asn_encode_to_buffer(opt_cod2,
                                                                  ATS_ALIGNED_BASIC_PER,
                                                                  &asn_DEF_E2SM_KPM_IndicationMessage,
                                                                  ind_message_style1,
                                                                  e2sm_message_buf_style1, e2sm_message_buf_size_style1);

          if (er_message_style1.encoded == -1) {
            LOG_I("Failed to serialize data. Detail: %s.", asn_DEF_E2SM_KPM_IndicationMessage.name);
            exit(1);
          } else if (er_message_style1.encoded > e2sm_message_buf_size_style1) {
            LOG_I("Buffer of size %zu is too small for %s, need %zu\n", e2sm_message_buf_size_style1, asn_DEF_E2SM_KPM_IndicationMessage.name, er_message_style1.encoded);
            exit(1);
          } else {
            LOG_I("Encoded Cell indication message succesfully, size in bytes: %ld", er_message_style1.encoded);
          }

          E2SM_KPM_IndicationHeader_t* ind_header_style1 = (E2SM_KPM_IndicationHeader_t*)calloc(1, sizeof(E2SM_KPM_IndicationHeader_t));

          kpm_report_indication_header_initialized(ind_header_style1);
          LOG_I("E2SM KPM Indication header:");
          xer_fprint(stderr, &asn_DEF_E2SM_KPM_IndicationHeader, ind_header_style1);

          uint8_t e2sm_header_buf_style1[8192] = {0, };
          size_t e2sm_header_buf_size_style1 = 8192;

          asn_enc_rval_t er_header_style1 = asn_encode_to_buffer(opt_cod2,
                                                                ATS_ALIGNED_BASIC_PER,
                                                                &asn_DEF_E2SM_KPM_IndicationHeader,
                                                                ind_header_style1,
                                                                e2sm_header_buf_style1, e2sm_header_buf_size_style1);

          if (er_header_style1.encoded == -1) {
            LOG_I("Failed to serialize data. Detail: %s.\n", asn_DEF_E2SM_KPM_IndicationHeader.name);
            exit(1);
          } else if (er_header_style1.encoded > e2sm_header_buf_size_style1) {
            LOG_I("Buffer of size %zu is too small for %s, need %zu", e2sm_header_buf_size_style1, asn_DEF_E2SM_KPM_IndicationHeader.name, er_header_style1.encoded);
            exit(1);
          } else {
            LOG_I("Encoded Cell indication header succesfully, size in bytes: %ld", er_header_style1.encoded);
          }

          ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationHeader, ind_header_style1);

          encoding::generate_e2apv1_indication_request_parameterized(pdu_style1, requestorId,
                                                                    instanceId, ranFunctionId,
                                                                    actionId, seqNum, e2sm_header_buf_style1,
                                                                    er_header_style1.encoded,
                                                                    e2sm_message_buf_style1, er_message_style1.encoded);

          e2sim.encode_and_send_sctp_data(pdu_style1);
          seqNum++;
          LOG_I("Succesfully sent subscribed PM values. Sleep 3s before sending again new values.");
          std::this_thread::sleep_for(std::chrono::milliseconds(3000));
      }

      std::cout << "Successfully read data from " << filename << std::endl;

  } catch (const io::error::can_not_open_file& e) {
      std::cerr << "Error: Could not open the file " << filename << ": " << e.what() << std::endl;
      return;
  } catch (const io::error::missing_column_in_header& e) {
      std::cerr << "Error: Missing column in CSV: " << e.what() << std::endl;
      return;
  } catch (const io::error::too_few_columns& e) {
      std::cerr << "Error: Too few columns in a row: " << e.what() << std::endl;
      return;
  } catch (const io::error::too_many_columns& e) {
      std::cerr << "Error: Too many columns in a row: " << e.what() << std::endl;
      return;
  } catch (const std::exception& e) {
      std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
      return;
  }
  
}

void callback_kpm_subscription_request(E2AP_PDU_t *sub_req_pdu) {
  // Record RIC Request ID
  // Go through RIC action to be Setup List
  // Find first entry with REPORT action Type
  // Record ricActionID
  // Encode subscription response

  RICsubscriptionRequest_t orig_req =
    sub_req_pdu->choice.initiatingMessage->value.choice.RICsubscriptionRequest;

  RICsubscriptionResponse_IEs_t *ricreqid =
    (RICsubscriptionResponse_IEs_t*)calloc(1, sizeof(RICsubscriptionResponse_IEs_t));

  int count = orig_req.protocolIEs.list.count;
  int size = orig_req.protocolIEs.list.size;

  RICsubscriptionRequest_IEs_t **ies = (RICsubscriptionRequest_IEs_t**)orig_req.protocolIEs.list.array;

  RICsubscriptionRequest_IEs__value_PR pres;

  long reqRequestorId;
  long reqInstanceId;
  long reqActionId;

  std::vector<long> actionIdsAccept;
  std::vector<long> actionIdsReject;

  LOG_I("Go through all the IEs in subscription request to construct neccessay parameters.");
  for (int i = 0; i < count; i++) {
    RICsubscriptionRequest_IEs_t *next_ie = ies[i];
    pres = next_ie->value.present;

    switch (pres) {
      case RICsubscriptionRequest_IEs__value_PR_RICrequestID: {
        RICrequestID_t reqId = next_ie->value.choice.RICrequestID;
        long requestorId = reqId.ricRequestorID;
        long instanceId = reqId.ricInstanceID;

        LOG_I("RequestorId: %ld, InstanceID: %ld", requestorId, instanceId);

        reqRequestorId = requestorId;
        reqInstanceId = instanceId;

        break;
      }
      case RICsubscriptionRequest_IEs__value_PR_RANfunctionID: {
        LOG_I("in case ran func id");
        break;
      }
      case RICsubscriptionRequest_IEs__value_PR_RICsubscriptionDetails: {
        RICsubscriptionDetails_t subDetails = next_ie->value.choice.RICsubscriptionDetails;
        RICeventTriggerDefinition_t triggerDef = subDetails.ricEventTriggerDefinition;
        RICactions_ToBeSetup_List_t actionList = subDetails.ricAction_ToBeSetup_List;
        // We are ignoring the trigger definition

        // We identify the first action whose type is REPORT
        // That is the only one accepted; all others are rejected

        int actionCount = actionList.list.count;
        LOG_I("Action count %d", actionCount);

        auto **item_array = actionList.list.array;

        bool foundAction = false;

        for (int i = 0; i < actionCount; i++) {
          auto *next_item = item_array[i];
          RICactionID_t actionId = ((RICaction_ToBeSetup_ItemIEs*)next_item)->value.choice.RICaction_ToBeSetup_Item.ricActionID;
          RICactionType_t actionType = ((RICaction_ToBeSetup_ItemIEs*)next_item)->value.choice.RICaction_ToBeSetup_Item.ricActionType;

          RICactionDefinition_t *ricActionDefinition = ((RICaction_ToBeSetup_ItemIEs*)next_item)->value.choice.RICaction_ToBeSetup_Item.ricActionDefinition;

          E2SM_KPM_ActionDefinition *action_definition_content = 0;
          asn_dec_rval_t rval;
          rval = asn_decode(0, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_KPM_ActionDefinition, (void**)&action_definition_content, ricActionDefinition->buf, ricActionDefinition->size);
          xer_fprint(stderr, &asn_DEF_RICactionDefinition, ricActionDefinition);
          xer_fprint(stderr, &asn_DEF_E2SM_KPM_ActionDefinition, action_definition_content);
          ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_ActionDefinition, action_definition_content);

          if (rval.code != RC_OK) {
            LOG_E("Failed to decode content of E2SM_KPM_ActionDefinition, size: %d, consumed byte: %ld.", ricActionDefinition->size, rval.consumed);
            LOG_I("Exiting e2sim");
            exit(1);
          }

          if (!foundAction && actionType == RICactionType_report) {
            reqActionId = actionId;
            actionIdsAccept.push_back(reqActionId);
            foundAction = true;
          } else {
            reqActionId = actionId;
            actionIdsReject.push_back(reqActionId);
          }
        }

        break;
      }
      default: {
        break;
      }
    }
  }

  for (int i = 0; i < actionIdsAccept.size(); i++)
    LOG_D("Accepted action ID:  %ld", actionIdsAccept.at(i));

  E2AP_PDU *e2ap_pdu = (E2AP_PDU*)calloc(1, sizeof(E2AP_PDU));

  long *accept_array = &actionIdsAccept[0];
  long *reject_array = &actionIdsReject[0];
  int accept_size = actionIdsAccept.size();
  int reject_size = actionIdsReject.size();

  encoding::generate_e2apv1_subscription_response_success(e2ap_pdu, accept_array, reject_array, accept_size, reject_size, reqRequestorId, reqInstanceId);

  LOG_I("Encode and sending E2AP subscription success response via SCTP");
  e2sim.encode_and_send_sctp_data(e2ap_pdu);

  LOG_I("Now generating data for subscription request");
  run_report_loop(reqRequestorId, reqInstanceId, gFuncId, reqActionId);
}
