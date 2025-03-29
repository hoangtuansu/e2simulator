cmake_minimum_required(VERSION 3.10)
project(e2sim_simulator)

set(CMAKE_CXX_STANDARD 14)

# Inclure les chemins ASN.1 et JSON
include_directories(${CMAKE_SOURCE_DIR}/asn1c)
include_directories(/usr/local/include)
include_directories(${CMAKE_SOURCE_DIR}/kpm_e2sm/src/kpm)

# Ajouter les fichiers sources de base (e2sim)
file(GLOB_RECURSE CORE_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Ajouter les fichiers spécifiques du simulateur KPM
file(GLOB_RECURSE KPM_SOURCES ${CMAKE_SOURCE_DIR}/kpm_e2sm/src/kpm/*.cpp)

# Créer l’exécutable final
add_executable(e2sim_simulator ${CORE_SOURCES} ${KPM_SOURCES})

# Inclure la bibliothèque JSON header-only
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE /usr/local/include)

# Lier les bibliothèques nécessaires
target_link_libraries(e2sim_simulator
  nlohmann_json
  asn1c  # ou le nom réel de ta lib ASN.1 si différente
  sctp   # nécessaire pour l'envoi SCTP
)





