cmake_minimum_required(VERSION 3.10)
project(e2sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajouter le répertoire d'inclusion de nlohmann_json
include_directories(${CMAKE_SOURCE_DIR}/external/json/include)

# Ajouter le sous-répertoire du simulateur KPM personnalisé
add_subdirectory(kpm_e2sm/src/kpm)

