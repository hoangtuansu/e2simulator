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


#ifndef KPM_CALLBACKS_HPP
#define KPM_CALLBACKS_HPP

#include "e2sim.hpp"
#include <string>

/// Gère l’arrivée d’un message de type KPM Subscription Request
void callback_kpm_subscription_request(E2AP_PDU_t *pdu);

/// Lance la boucle d’envoi périodique des indications KPM (non utilisée si on injecte depuis un JSON externe)
void run_report_loop(long requestorId, long instanceId, long ranFunctionId, long actionId);

void generate_and_send_kpm_report();  // ✅ À ajouter si manquant
/// Charge le fichier JSON de KPI et commence l’injection automatique toutes les secondes.
/// Envoie un message E2SM-KPM par KPI, un par seconde.
/// @param json_path Chemin vers le fichier JSON contenant les traces KPI.
/// @param ricAddress Adresse IP du RIC.
/// @param ricPort Port SCTP du RIC.
void start_kpi_injection(const std::string& json_path, const std::string& ricAddress, int ricPort);

#endif  // KPM_CALLBACKS_HPP


