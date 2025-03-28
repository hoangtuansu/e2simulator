FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Installer dépendances
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libssl-dev \
    libboost-all-dev \
    libnlohmann-json-dev \
    && rm -rf /var/lib/apt/lists/*

# Créer répertoire de travail
WORKDIR /opt/e2sim

# Copier le simulateur et les fichiers
COPY kpm_e2sm/src/kpm/kpm_callbacks_custom.cpp ./kpm_callbacks_custom.cpp
COPY kpm_e2sm/kpi_traces.json ./kpi_traces.json

# Compiler le simulateur
RUN g++ -std=c++17 kpm_callbacks_custom.cpp -o kpm_trace_simulator -lnlohmann_json -pthread

# Commande de lancement
CMD ["./kpm_trace_simulator"]