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

using json = nlohmann::json;
using namespace std;
using namespace std::chrono;

class E2Sim;
int gFuncId;

E2Sim e2sim;

// ------------------ Helper functions ------------------------------------
static string map_config_metric_to_cell_name(const string &conf_metric) {
  if(conf_metric == "dl_throughput_ue_mbps" || conf_metric == "ul_throughput_ue_mbps")
    return "Throughput";
  if(conf_metric == "dl_prb_available" || conf_metric == "ul_prb_available")
    return "Available PRBs";
  if(conf_metric == "radio_resource_utilization_percent")
    return "Radio Resource Utilization";
  if(conf_metric == "active_ue_count")
    return "Number of Active UEs";
  if(conf_metric == "user_outage_percent")
    return "User Outage";
  if(conf_metric == "ml_classification_accuracy_percent")
    return "ML Classification Accuracy";
  if(conf_metric == "ml_training_time_sec")
    return "ML Training Time";
  // For synthetic or traffic‑type metrics not present in report
  return "";
}

static double extract_first_number(const string &str) {
  const char *cstr = str.c_str();
  char *endptr = nullptr;
  double val = strtod(cstr, &endptr);
  return (cstr == endptr) ? 0.0 /* no numeric at start */ : val;
}

// ------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  LOG_I("Starting KPM simulator");

  uint8_t *nrcellid_buf = (uint8_t*)calloc(1, 5);
  nrcellid_buf[0] = 0x22;
  nrcellid_buf[1] = 0x5B;
  nrcellid_buf[2] = 0xD6;
  nrcellid_buf[3] = 0x00;
  nrcellid_buf[4] = 0x70;

  // ------------------------------------------------------------------
  // Parse config.json once to advertise metrics contained in "pm" list
  // ------------------------------------------------------------------
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
  std::uniform_int_distribution<> distrib(0, 4095);

  const char* func_id_str = std::getenv("RAN_FUNC_ID");
  ::gFuncId =123 func_id_str == nullptr ? distrib(gen) : std::stoi(func_id_str);

  e2sim.register_e2sm(gFuncId, ranfunc_ostr);
  e2sim.register_subscription_callback(gFuncId, &callback_kpm_subscription_request);
  e2sim.run_loop(argc, argv);
}

//--------------------------------------------------------------------------
//  Report loop adapted to config.json ("pm") & cellmeasreport.json structure
//--------------------------------------------------------------------------
void run_report_loop(long requestorId, long instanceId, long ranFunctionId, long actionId) {
  // ----------- Load config.json ---------------------------------------
  std::ifstream config_file("/opt/e2sim/kpm_e2sm/config.json");
  if (!config_file.is_open()) {
    LOG_E("Unable to open config.json");
    exit(1);
  }

  json config_json;
  config_file >> config_json;
  config_file.close();

  std::vector<std::string> metric_names = config_json["pm"].get<std::vector<std::string>>();

  // ------------ Load cellmeasreport.json ------------------------------
  std::ifstream reports_file("/opt/e2sim/kpm_e2sm/cellmeasreport.json");
  if (!reports_file.is_open()) {
    LOG_E("Can't open cellmeasreport.json, exiting ...");
    exit(1);
  }

  json cell_json;
  reports_file >> cell_json;
  reports_file.close();

  const json &metrics_array = cell_json["Metrics"];

  // Build a quick lookup from cell metric name to Example Value string
  std::vector<std::pair<std::string,std::string>> metric_lookup;
  for(const auto &m : metrics_array) {
    std::string name = m["Metric Name"].get<std::string>();
    std::string example = m["Example Value / Result"].get<std::string>();
    metric_lookup.emplace_back(name, example);
  }

  auto find_example = [&](const std::string &cell_name)->string {
    for(const auto &p : metric_lookup) if(p.first == cell_name) return p.second;
    return "";
  };

  long seqNum = 1;

  // Compose one report comprising all metrics requested in config.json
  {
    std::vector<double> metric_values;

    for (const auto &metric_id : metric_names) {
      std::string cell_name = map_config_metric_to_cell_name(metric_id);
      std::string example_str = cell_name.empty() ? "" : find_example(cell_name);
      double value = example_str.empty() ? 0.0 : extract_first_number(example_str);

      // Fallback to dummy generation when no value present
      if (value == 0.0) {
        if(metric_id.find("percent") != std::string::npos) {
          value = (double)(rand()%1000)/10.0; // 0.0 - 100.0
        } else if(metric_id.find("throughput") != std::string::npos) {
          value = (double)(rand()%10000)/10.0; // 0 - 1000 Mbps
        } else if(metric_id.find("prb") != std::string::npos) {
          value = rand()%52 + 48; // typical RB counts 48-100
        } else if(metric_id.find("active_ue") != std::string::npos) {
          value = rand()%128; // 0‑127 UEs
        } else {
          value = (double)(rand()%1000)/10.0;
        }
      }
      metric_values.push_back(value);
    }

    LOG_I("Sending single consolidated KPM report with %zu metrics", metric_values.size());

    E2SM_KPM_IndicationMessage_t *ind_message = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
    E2AP_PDU *pdu = (E2AP_PDU*)calloc(1, sizeof(E2AP_PDU));

    const char** cell_pms_labels = new const char*[metric_names.size()];
    for (size_t i = 0; i < metric_names.size(); ++i) {
      cell_pms_labels[i] = metric_names[i].c_str();
    }

    kpm_report_indication_message_initialized(ind_message, cell_pms_labels, metric_values.data(), metric_names.size());

    uint8_t e2sm_message_buf[8192] = {0};
    size_t e2sm_message_buf_size = 8192;
    asn_codec_ctx_t *opt_cod2;

    asn_enc_rval_t er_message = asn_encode_to_buffer(opt_cod2,
                                                     ATS_ALIGNED_BASIC_PER,
                                                     &asn_DEF_E2SM_KPM_IndicationMessage,
                                                     ind_message,
                                                     e2sm_message_buf, e2sm_message_buf_size);

    if (er_message.encoded <= 0) {
      LOG_E("Failed to encode E2SM KPM Indication Message.");
      exit(1);
    }

    E2SM_KPM_IndicationHeader_t* ind_header = (E2SM_KPM_IndicationHeader_t*)calloc(1, sizeof(E2SM_KPM_IndicationHeader_t));
    kpm_report_indication_header_initialized(ind_header);

    uint8_t e2sm_header_buf[8192] = {0};
    size_t e2sm_header_buf_size = 8192;

    asn_enc_rval_t er_header = asn_encode_to_buffer(opt_cod2,
                                                    ATS_ALIGNED_BASIC_PER,
                                                    &asn_DEF_E2SM_KPM_IndicationHeader,
                                                    ind_header,
                                                    e2sm_header_buf, e2sm_header_buf_size);

    if (er_header.encoded <= 0) {
      LOG_E("Failed to encode E2SM KPM Indication Header.");
      exit(1);
    }

    ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationHeader, ind_header);

    encoding::generate_e2apv1_indication_request_parameterized(pdu, requestorId,
                                                               instanceId, ranFunctionId,
                                                               actionId, seqNum, e2sm_header_buf,
                                                               er_header.encoded,
                                                               e2sm_message_buf, er_message.encoded);

    e2sim.encode_and_send_sctp_data(pdu);
    LOG_I("Sent indication #%ld", seqNum);
    seqNum++;

    delete[] cell_pms_labels;
  }
}

//--------------------------------------------------------------------------
void callback_kpm_subscription_request(E2AP_PDU_t *sub_req_pdu) {
  RICsubscriptionRequest_t orig_req =
    sub_req_pdu->choice.initiatingMessage->value.choice.RICsubscriptionRequest;

  int count = orig_req.protocolIEs.list.count;
  RICsubscriptionRequest_IEs_t **ies = (RICsubscriptionRequest_IEs_t**)orig_req.protocolIEs.list.array;

  long reqRequestorId = 0;
  long reqInstanceId = 0;
  long reqActionId = 0;

  std::vector<long> actionIdsAccept;
  std::vector<long> actionIdsReject;

  for (int i = 0; i < count; i++) {
    RICsubscriptionRequest_IEs_t *next_ie = ies[i];

    switch (next_ie->value.present) {
      case RICsubscriptionRequest_IEs__value_PR_RICrequestID: {
        RICrequestID_t reqId = next_ie->value.choice.RICrequestID;
        reqRequestorId = reqId.ricRequestorID;
        reqInstanceId = reqId.ricInstanceID;
        LOG_I("Received RIC Subscription Request: Requestor ID: %ld, Instance ID: %ld", reqRequestorId, reqInstanceId);
        break;
      }

      case RICsubscriptionRequest_IEs__value_PR_RICsubscriptionDetails: {
        RICsubscriptionDetails_t subDetails = next_ie->value.choice.RICsubscriptionDetails;
        RICactions_ToBeSetup_List_t actionList = subDetails.ricAction_ToBeSetup_List;

        int actionCount = actionList.list.count;
        auto **item_array = actionList.list.array;

        bool found = false;

        for (int j = 0; j < actionCount; j++) {
          auto *next_item = item_array[j];
          RICactionID_t actionId = ((RICaction_ToBeSetup_ItemIEs*)next_item)->value.choice.RICaction_ToBeSetup_Item.ricActionID;
          RICactionType_t actionType = ((RICaction_ToBeSetup_ItemIEs*)next_item)->value.choice.RICaction_ToBeSetup_Item.ricActionType;

          RICactionDefinition_t *ricActionDefinition = ((RICaction_ToBeSetup_ItemIEs*)next_item)->value.choice.RICaction_ToBeSetup_Item.ricActionDefinition;

          // Decode action definition if present
          if (ricActionDefinition) {
            E2SM_KPM_ActionDefinition *decoded = nullptr;
            asn_dec_rval_t rval = asn_decode(nullptr, ATS_ALIGNED_BASIC_PER,
                                             &asn_DEF_E2SM_KPM_ActionDefinition,
                                             (void**)&decoded,
                                             ricActionDefinition->buf,
                                             ricActionDefinition->size);

            if (rval.code != RC_OK) {
              LOG_E("Failed to decode ActionDefinition.");
              exit(1);
            }

            xer_fprint(stderr, &asn_DEF_E2SM_KPM_ActionDefinition, decoded);
            ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_ActionDefinition, decoded);
          }

          if (!found && actionType == RICactionType_report) {
            reqActionId = actionId;
            actionIdsAccept.push_back(actionId);
            found = true;
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

  // Build the E2AP Subscription Response
  E2AP_PDU *e2ap_pdu = (E2AP_PDU*)calloc(1, sizeof(E2AP_PDU));

  encoding::generate_e2apv1_subscription_response_success(
    e2ap_pdu,
    actionIdsAccept.data(),
    actionIdsReject.data(),
    actionIdsAccept.size(),
    actionIdsReject.size(),
    reqRequestorId,
    reqInstanceId
  );

  LOG_I("Sending E2AP subscription response...");
  e2sim.encode_and_send_sctp_data(e2ap_pdu);

  // Launch KPM report generation loop after successful subscription
  LOG_I("Starting KPM report loop after subscription...");
  run_report_loop(reqRequestorId, reqInstanceId, gFuncId, reqActionId);
}
