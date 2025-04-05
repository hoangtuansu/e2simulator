cmake_minimum_required(VERSION 3.10)
project(e2sim_simulator)

set(CMAKE_CXX_STANDARD 14)

# --- Répertoires d'inclusion ---
include_directories(
  ${CMAKE_SOURCE_DIR}/asn1c
  ${CMAKE_SOURCE_DIR}/asn1c/asn/v03
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/DEF
  ${CMAKE_SOURCE_DIR}/src/base
  ${CMAKE_SOURCE_DIR}/src/encoding
  ${CMAKE_SOURCE_DIR}/src/messagerouting
  ${CMAKE_SOURCE_DIR}/src/SCTP
  ${CMAKE_SOURCE_DIR}/kpm_e2sm/src/kpm
  /usr/local/include
)

# --- Fichiers sources ---
file(GLOB_RECURSE CORE_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE CORE_C_SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE KPM_SOURCES ${CMAKE_SOURCE_DIR}/kpm_e2sm/src/kpm/*.cpp)
file(GLOB_RECURSE ASN1C_SOURCES ${CMAKE_SOURCE_DIR}/asn1c/*.c)

# --- Compilation de la lib ASN.1 ---
add_library(asn1c STATIC ${ASN1C_SOURCES})

# --- Bibliothèque JSON header-only ---
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE /usr/local/include)

# --- Exécutable principal ---
add_executable(e2sim_simulator ${CORE_SOURCES} ${CORE_C_SOURCES} ${KPM_SOURCES})
target_link_libraries(e2sim_simulator asn1c nlohmann_json)

# Pour le SCTP (ajoute cette ligne si nécessaire)
find_library(SCTP_LIBRARY sctp)
if(SCTP_LIBRARY)
  target_link_libraries(e2sim_simulator ${SCTP_LIBRARY})
endif()
