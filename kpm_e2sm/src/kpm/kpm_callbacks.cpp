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
#include "e2sim_sctp.hpp"
#include "kpm_callbacks.hpp" 
#include "encode_kpm_indication.hpp"
#include "build_e2ap_indication.hpp"
#include "encode_e2apv1.hpp"
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
#include <cstring>  // pour memcpy

extern "C" {
    #include "asn_application.h"
    #include "asn_internal.h"
    #include "per_encoder.h"
}

using json = nlohmann::json;
extern int global_sock;
extern long global_ran_function_id;
extern uint16_t global_ran_node_id;

// ✅ Fonction encodage + envoi E2AP sur SCTP
bool encode_and_send_e2ap_sctp(E2AP_PDU_t* pdu, int socket_fd) {
    uint8_t buffer[4096];
    asn_enc_rval_t er = asn_encode_to_buffer(nullptr, ATS_ALIGNED_BASIC_PER,
                                             &asn_DEF_E2AP_PDU, pdu, buffer, sizeof(buffer));
    if (er.encoded <= 0) {
        std::cerr << "[E2AP ERROR] Échec d'encodage du message E2AP." << std::endl;
        return false;
    }

    sctp_buffer_t sctp_data;
    memcpy(sctp_data.buffer, buffer, er.encoded);
    sctp_data.len = er.encoded;

    int sent = sctp_send_data(socket_fd, sctp_data);
    return sent > 0;
}

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

                // Encode message
                std::vector<unsigned char> kpm_buf;
                bool success = false;

                if (value.is_number_float()) {
                    success = encode_kpm_indication(kpi_name, value.get<double>(), 0, kpm_buf);
                } else if (value.is_number_unsigned()) {
                    success = encode_kpm_indication(kpi_name, 0.0, value.get<unsigned>(), kpm_buf);
                } else if (value.is_number_integer()) {
                    int64_t signed_val = value.get<int64_t>();
                    success = encode_kpm_indication(kpi_name, 0.0, static_cast<unsigned>(signed_val), kpm_buf);
                }

                if (!success) {
                    std::cerr << "Erreur: Encodage KPM échoué pour " << kpi_name << std::endl;
                    continue;
                }

                // Construire et envoyer le message E2AP
                E2AP_PDU_t* pdu = build_e2ap_indication(global_ran_function_id, global_ran_node_id, kpm_buf);
                if (!encode_and_send_e2ap_sctp(pdu, global_sock)) {
                    std::cerr << "[E2AP ERROR] Échec d'envoi du message pour " << kpi_name << std::endl;
                } else {
                    std::cout << "Message KPM envoyé pour KPI: " << kpi_name << " (" << ts << ")" << std::endl;
                }

                ASN_STRUCT_FREE(asn_DEF_E2AP_PDU, pdu);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
}
