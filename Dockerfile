FROM debian:bullseye

# Installer les dépendances
RUN apt-get update && apt-get install -y \
  build-essential git cmake libsctp-dev lksctp-tools autoconf automake \
  libtool bison flex libboost-all-dev iputils-ping \
  net-tools nano vim tcpdump nmap && \
  apt-get clean

# Cloner la bibliothèque JSON
RUN git clone https://github.com/azadkuh/nlohmann_json_release.git
RUN mkdir -p /usr/local/include/nlohmann && \
    cp nlohmann_json_release/json.hpp /usr/local/include/nlohmann

# Créer les répertoires nécessaires
RUN mkdir -p /opt/e2sim/asn1c /opt/e2sim/kpm_e2sm/asn1c /opt/e2sim/src /opt/e2sim/kpm_e2sm/src/kpm

# Copier les fichiers sources
COPY CMakeLists.txt /opt/e2sim/
COPY asn1c/ /opt/e2sim/asn1c
COPY src/ /opt/e2sim/src
COPY kpm_e2sm/src/kpm/ /opt/e2sim/kpm_e2sm/src/kpm/
COPY kpm_e2sm/src/kpm/kpi_traces.json /opt/e2sim/kpm_e2sm/src/kpm/kpi_traces.json
COPY build_and_run.sh /opt/e2sim/build_and_run.sh

# Construire e2sim (core + simulateur personnalisé)
RUN mkdir -p /opt/e2sim/build && cd /opt/e2sim/build && cmake .. && make

# Commande par défaut : simulateur avec envoi SCTP
CMD ["/opt/e2sim/build/e2sim_simulator"]
