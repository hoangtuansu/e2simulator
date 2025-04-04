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

#include "E2AP-PDU.h"
#include <vector>
#include "e2sim_sctp.hpp"  // nécessaire pour sctp_send_data()

// Fonction principale de génération/envoi des rapports KPM
void generate_and_send_kpm_report();

// Fonction interne : encode un PDU E2AP et l'envoie via SCTP
bool encode_and_send_e2ap_sctp(E2AP_PDU_t* pdu, int socket_fd);

#endif // KPM_CALLBACKS_HPP
