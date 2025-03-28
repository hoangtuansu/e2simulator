cmake_minimum_required(VERSION 3.10)
project(e2sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajouter le sous-module nlohmann_json_release
add_subdirectory(external/json)

# Ajouter le sous-répertoire du simulateur KPM personnalisé
add_subdirectory(kpm_e2sm/src/kpm)
