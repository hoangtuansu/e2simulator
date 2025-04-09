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
  std::uniform_int_distribution<> distrib(0, 4095);

  const char* func_id_str = std::getenv("RAN_FUNC_ID");
  ::gFuncId = func_id_str == nullptr ? distrib(gen) : std::stoi(func_id_str);

  e2sim.register_e2sm(gFuncId, ranfunc_ostr);
  e2sim.register_subscription_callback(gFuncId, &callback_kpm_subscription_request);
  e2sim.run_loop(argc, argv);
}

void run_report_loop(long requestorId, long instanceId, long ranFunctionId, long actionId) {
  std::ifstream config_file("/opt/e2sim/kpm_e2sm/config.json");
  if (!config_file.is_open()) {
    LOG_E("Unable to open config.json");
    exit(1);
  }

  json config_json;
  config_file >> config_json;
  std::vector<std::string> metric_names = config_json["columns"].get<std::vector<std::string>>();
  config_file.close();

  std::ifstream reports_file("/opt/e2sim/kpm_e2sm/cellmeasreport.json");
  if (!reports_file.is_open()) {
    LOG_E("Can't open cellmeasreport.json, exiting ...");
    exit(1);
  }

  json all_reports_json;
  reports_file >> all_reports_json;
  reports_file.close();

  long seqNum = 1;
  std::vector<std::string> traffic_types = {"eMBB", "URLLC", "mMTC"};

  for (const auto& traffic : traffic_types) {
    const auto& report_list = all_reports_json[traffic];

    for (const auto& report_entry : report_list) {
      std::vector<double> metric_values;

      for (const auto& metric : metric_names) {
        try {
          double value = report_entry.at(metric).get<double>();
          metric_values.push_back(value);
        } catch (...) {
          LOG_W("Metric %s not found in traffic type %s. Inserting 0.0 as default.", metric.c_str(), traffic.c_str());
          metric_values.push_back(0.0);
        }
      }

      LOG_I("Sending E2Node report for traffic type: %s", traffic.c_str());

      E2SM_KPM_IndicationMessage_t *ind_message_style1 = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
      E2AP_PDU *pdu_style1 = (E2AP_PDU*)calloc(1, sizeof(E2AP_PDU));

      const char** cell_pms_labels = new const char*[metric_names.size()];
      for (size_t i = 0; i < metric_names.size(); ++i) {
        cell_pms_labels[i] = metric_names[i].c_str();
      }

      kpm_report_indication_message_initialized(ind_message_style1, cell_pms_labels, metric_values.data(), metric_names.size());

      uint8_t e2sm_message_buf_style1[8192] = {0};
      size_t e2sm_message_buf_size_style1 = 8192;
      asn_codec_ctx_t *opt_cod2;

      asn_enc_rval_t er_message_style1 = asn_encode_to_buffer(opt_cod2,
                                                              ATS_ALIGNED_BASIC_PER,
                                                              &asn_DEF_E2SM_KPM_IndicationMessage,
                                                              ind_message_style1,
                                                              e2sm_message_buf_style1, e2sm_message_buf_size_style1);

      if (er_message_style1.encoded <= 0) {
        LOG_E("Failed to encode E2SM KPM Indication Message.");
        exit(1);
      }

      E2SM_KPM_IndicationHeader_t* ind_header_style1 = (E2SM_KPM_IndicationHeader_t*)calloc(1, sizeof(E2SM_KPM_IndicationHeader_t));
      kpm_report_indication_header_initialized(ind_header_style1);

      uint8_t e2sm_header_buf_style1[8192] = {0};
      size_t e2sm_header_buf_size_style1 = 8192;

      asn_enc_rval_t er_header_style1 = asn_encode_to_buffer(opt_cod2,
                                                             ATS_ALIGNED_BASIC_PER,
                                                             &asn_DEF_E2SM_KPM_IndicationHeader,
                                                             ind_header_style1,
                                                             e2sm_header_buf_style1, e2sm_header_buf_size_style1);

      if (er_header_style1.encoded <= 0) {
        LOG_E("Failed to encode E2SM KPM Indication Header.");
        exit(1);
      }

      ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationHeader, ind_header_style1);

      encoding::generate_e2apv1_indication_request_parameterized(pdu_style1, requestorId,
                                                                 instanceId, ranFunctionId,
                                                                 actionId, seqNum, e2sm_header_buf_style1,
                                                                 er_header_style1.encoded,
                                                                 e2sm_message_buf_style1, er_message_style1.encoded);

      e2sim.encode_and_send_sctp_data(pdu_style1);
      LOG_I("Sent indication #%ld for %s", seqNum, traffic.c_str());
      seqNum++;

      delete[] cell_pms_labels;
      std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
  }
}
