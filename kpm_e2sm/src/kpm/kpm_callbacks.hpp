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

#include <vector>
#include <string>
#include <cstdint>
#include "E2AP-PDU.h"

// Déclaration de la fonction principale de génération et d'envoi des rapports KPM
void generate_and_send_kpm_report();

// Encodage et envoi via SCTP (E2AP wrapper)
bool encode_and_send_e2ap_sctp(const std::vector<unsigned char>& kpm_encoded, int socket_fd);

#endif // KPM_CALLBACKS_HPP

