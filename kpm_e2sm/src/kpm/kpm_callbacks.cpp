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

#include "kpm_callbacks.hpp"
#include "encode_kpm_indication.hpp"
#include "build_e2ap_indication.hpp"
#include "e2sim_sctp.hpp"
#include "E2AP-PDU.h"
#include "InitiatingMessage.h"
#include "RICsubscriptionRequest.h"
#include "RICindication.h"
#include "e2sim_defs.h"
#include "e2sim.hpp"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>

using json = nlohmann::json;
extern int global_sock;
extern long global_ran_function_id;
extern uint16_t global_ran_node_id;

void generate_and_send_kpm_report() {
    std::ifstream file("kpi_traces.json");
    if (!file.is_open()) {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier KPI." << std::endl;
        return;
    }

    json traces;
    file >> traces;

    for (const auto& timestamp_entry : traces) {
        std::string ts = timestamp_entry["timestamp"];

        for (auto& category : {"eMBB", "mMTC", "URLLC"}) {
            if (!timestamp_entry.contains(category)) continue;
            for (auto& metric : timestamp_entry[category].items()) {
                std::string kpi_name = std::string(category) + "." + metric.key();
                auto value = metric.value();

                std::vector<unsigned char> kpm_buf;
                bool success = false;

                if (value.is_number_float()) {
                    success = encode_kpm_indication(kpi_name, value.get<double>(), 0, kpm_buf);
                } else if (value.is_number_unsigned()) {
                    success = encode_kpm_indication(kpi_name, 0.0, value.get<unsigned>(), kpm_buf);
                } else if (value.is_number_integer()) {
                    unsigned converted_val = static_cast<unsigned>(value.get<int64_t>());
                    success = encode_kpm_indication(kpi_name, 0.0, converted_val, kpm_buf);
                }

                if (!success) {
                    std::cerr << "Erreur: Encodage KPM échoué pour " << kpi_name << std::endl;
                    continue;
                }

                std::vector<uint8_t> kpm_buf_uint(kpm_buf.begin(), kpm_buf.end());
                std::vector<uint8_t> e2ap_buf;

                bool ok = build_e2ap_indication(kpm_buf_uint, e2ap_buf, global_ran_function_id, global_ran_node_id);
                if (!ok) {
                    std::cerr << "Erreur: build_e2ap_indication a échoué" << std::endl;
                    continue;
                }

                sctp_buffer_t sctp_data;
                sctp_data.len = e2ap_buf.size();
                memcpy(sctp_data.buffer, e2ap_buf.data(), sctp_data.len);

                int sent = sctp_send_data(global_sock, sctp_data);
                if (sent <= 0) {
                    std::cerr << "Erreur: Envoi SCTP échoué pour " << kpi_name << std::endl;
                    continue;
                }

                std::cout << "Message KPM envoyé pour KPI: " << kpi_name << " (" << ts << ")" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
}
