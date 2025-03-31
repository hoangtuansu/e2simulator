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
#ifndef ENCODE_KPM_INDICATION_HPP
#define ENCODE_KPM_INDICATION_HPP

#include <string>
#include <vector>
#include <cstdint>

// Encode un message E2SM-KPM Indication
// - kpi_name : nom de la métrique (ex: "PRB_Utilization")
// - value1   : valeur de type réel
// - value2   : valeur de type entier
// - buffer   : contiendra les octets APER encodés
bool encode_kpm_indication(const std::string& kpi_name, double value1, int64_t value2, std::vector<unsigned char>& buffer);

#endif // ENCODE_KPM_INDICATION_HPP
