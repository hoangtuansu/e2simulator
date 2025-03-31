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
#ifndef ENCODE_KPM_HPP
#define ENCODE_KPM_HPP

#include <string>
#include <vector>

// La fonction encode les valeurs JSON dans un message E2SM-KPM IndicationMessage (Format1)
// et retourne le message encodé dans le buffer (en octets)
//
// Paramètres :
//   - kpi_name : le nom du KPI ("throughput", "latency", etc.)
//   - value : la valeur du KPI (ex: 34.5 Mbps ou 1.2 ms)
//   - id : l'identifiant de mesure
//   - buffer : où sera stocké le message ASN.1 encodé
//
// Retour : true si le message a été encodé avec succès

bool encode_kpm_indication(const std::string& kpi_name, double value, int id,
                           std::vector<unsigned char>& buffer);

#endif // ENCODE_KPM_HPP
