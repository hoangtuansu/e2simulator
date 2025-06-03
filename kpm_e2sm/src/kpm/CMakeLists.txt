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
#

add_executable(kpm_sim
  kpm_callbacks.cpp
  encode_kpm.cpp
)

# Inclure les headers du projet
target_include_directories(kpm_sim PUBLIC
  ${CMAKE_SOURCE_DIR}/asn1c
  ${CMAKE_SOURCE_DIR}/src/DEF
  ${CMAKE_SOURCE_DIR}/src/SCTP
  ${CMAKE_SOURCE_DIR}/src/messagerouting
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/encoding
)

# Lien avec la bibliothèque principale
target_link_libraries(kpm_sim
  e2sim_shared
  pthread
)

