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

add_executable(rc_sim
  rc_callbacks.cpp
  rc_handler.cpp
  rc_subscription_handler.cpp
  encode_rc.cpp
)

target_include_directories(rc_sim PUBLIC
  ${CMAKE_SOURCE_DIR}/asn1c
  ${CMAKE_SOURCE_DIR}/asn1_generated
  ${CMAKE_SOURCE_DIR}/src/DEF
  ${CMAKE_SOURCE_DIR}/src/SCTP
  ${CMAKE_SOURCE_DIR}/src/messagerouting
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/encoding
  ${CMAKE_SOURCE_DIR}/rc_e2sm/src/rc
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(rc_sim
  e2sim_shared
  pthread
)

install(
    TARGETS rc_sim
    DESTINATION ${install_bin}
)
