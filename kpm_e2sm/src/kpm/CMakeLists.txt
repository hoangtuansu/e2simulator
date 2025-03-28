#==================================================================================
#	Copyright (c) 2020 AT&T Intellectual Property.
#
#   Licensed under the Apache License, Version 2.0 (the "License"),
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#==================================================================================
cmake_minimum_required(VERSION 3.10)

# Ajouter un exécutable pour le simulateur de KPI
add_executable(e2sim_simulator
  kpm_callbacks.cpp
)

# Lier la bibliothèque JSON (sous-module)
target_link_libraries(e2sim_simulator
  nlohmann_json::nlohmann_json
  asn1c_library_name   # <- à adapter à ton projet ASN.1
)

