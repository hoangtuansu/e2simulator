
FROM debian:bullseye

# Installer les dépendances nécessaires
RUN apt-get update \
  && apt-get install -y build-essential git cmake libsctp-dev lksctp-tools autoconf automake \
  libtool bison flex libboost-all-dev iputils-ping \
  net-tools nano vim tcpdump nmap \
  && apt-get clean

# Cloner la bibliothèque JSON (version azadkuh) et l'intégrer
RUN git clone https://github.com/azadkuh/nlohmann_json_release.git
RUN mkdir -p /usr/local/include/nlohmann && \
    cp nlohmann_json_release/json.hpp /usr/local/include/nlohmann

# Créer les répertoires nécessaires
RUN mkdir -p /opt/e2sim/asn1c /opt/e2sim/kpm_e2sm/asn1c /opt/e2sim/src /opt/e2sim/kpm_e2sm/src/kpm

# Copier les sources de e2sim (core)
COPY CMakeLists.txt /opt/e2sim/
COPY asn1c/ /opt/e2sim/asn1c
COPY src/ /opt/e2sim/src
COPY kpm_e2sm/src/kpm /opt/e2sim/kpm_e2sm/src/kpm 

# Construire e2sim principal (avec le dossier requis disponible)
RUN mkdir /opt/e2sim/build && cd /opt/e2sim/build \
  && cmake .. && make package && cmake .. -DDEV_PKG=1 && make package

# Installer les paquets DEB générés
RUN dpkg -i /opt/e2sim/build/e2sim_1.0.0_amd64.deb /opt/e2sim/build/e2sim-dev_1.0.0_amd64.deb

# Copier les fichiers spécifiques au simulateur KPM
COPY ./kpm_e2sm/kpi_traces.json /opt/e2sim/kpm_e2sm/kpi_traces.json
COPY ./kpm_e2sm/src/kpm/kpm_callbacks.cpp /opt/e2sim/kpm_e2sm/src/kpm/kpm_callbacks.cpp
COPY ./kpm_e2sm/src/kpm/encode_kpm_indication.cpp /opt/e2sim/kpm_e2sm/src/kpm/encode_kpm_indication.cpp
COPY ./kpm_e2sm/src/kpm/build_e2ap_indication.cpp /opt/e2sim/kpm_e2sm/src/kpm/build_e2ap_indication.cpp
COPY ./kpm_e2sm/src/kpm/CMakeLists.txt /opt/e2sim/kpm_e2sm/src/kpm/CMakeLists.txt
COPY ./asn1c/ /opt/e2sim/kpm_e2sm/asn1c
COPY build_and_run.sh /opt/e2sim/build_and_run.sh

# Construire le simulateur final
RUN mkdir /opt/e2sim/kpm_e2sm/.build && cd /opt/e2sim/kpm_e2sm/.build \
  && cmake .. && make install

# Lancement automatique
CMD ["/opt/e2sim/kpm_e2sm/.build/e2sim_simulator"]
