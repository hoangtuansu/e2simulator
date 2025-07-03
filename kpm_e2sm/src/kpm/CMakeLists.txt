add_executable(kpm_sim
  kpm_callbacks.cpp
  encode_kpm.cpp
)

target_include_directories(kpm_sim PUBLIC
  ${CMAKE_SOURCE_DIR}/asn1c
  ${CMAKE_SOURCE_DIR}/asn1_generated
  ${CMAKE_SOURCE_DIR}/src/DEF
  ${CMAKE_SOURCE_DIR}/src/SCTP
  ${CMAKE_SOURCE_DIR}/src/messagerouting
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/encoding
  ${CMAKE_SOURCE_DIR}/kpm_e2sm/src/kpm
)

target_link_libraries(kpm_sim
  e2sim_shared
  pthread
)

install(
  TARGETS kpm_sim
  DESTINATION ${install_bin}
)
