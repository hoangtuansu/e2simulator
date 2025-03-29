FROM debian:bullseye

# Installer les dépendances nécessaires
RUN apt-get update \
  && apt-get install -y build-essential git cmake libsctp-dev lksctp-tools autoconf automake \
  libtool bison flex libboost-all-dev iputils-ping \
  net-tools nano vim tcpdump nmap \
  && apt-get clean

# Debug context pour vérifier les fichiers visibles
RUN echo "🔍 CONTENU AVANT COPY" && ls -R /

# Cloner la bibliothèque JSON (version azadkuh) et l'intégrer
RUN git clone https://github.com/azadkuh/nlohmann_json_release.git
RUN mkdir -p /usr/local/include/nlohmann && \
    cp nlohmann_json_release/json.hpp /usr/local/include/nlohmann

# Créer les répertoires nécessaires
RUN mkdir -p /opt/e2sim/asn1c /opt/e2sim/kpm_e2sm /opt/e2sim/kpm_e2sm/asn1c /opt/e2sim/src

# Copier les sources de e2sim
COPY CMakeLists.txt /opt/e2sim/
COPY asn1c/ /opt/e2sim/asn1c
COPY src/ /opt/e2sim/src

# ✅ Vérifier le chemin kpm_e2sm/src/kpm
RUN echo "🔍 AVANT COPY DU KPM" && ls -R kpm_e2sm

# ✅ Copier le dossier kpm pour éviter erreur CMake
COPY kpm_e2sm/src/kpm /opt/e2sim/kpm_e2sm/src/kpm

# Construire e2sim principal
RUN mkdir /opt/e2sim/build && cd /opt/e2sim/build \
  && cmake .. && make package && cmake .. -DDEV_PKG=1 && make package

# Installer les paquets DEB générés
RUN dpkg -i /opt/e2sim/build/e2sim_1.0.0_amd64.deb /opt/e2sim/build/e2sim-dev_1.0.0_amd64.deb

# Copier les fichiers KPM
COPY ./kpm_e2sm/kpi_traces.json /opt/e2sim/kpm_e2sm/kpi_traces.json
COPY ./kpm_e2sm/src/kpm/kpm_callbacks.cpp /opt/e2sim/kpm_e2sm/src/kpm/kpm_callbacks.cpp
COPY ./kpm_e2sm/src/kpm/encode_kpm_indication.cpp /opt/e2sim/kpm_e2sm/src/kpm/encode_kpm_indication.cpp
COPY ./kpm_e2sm/src/kpm/build_e2ap_indication.cpp /opt/e2sim/kpm_e2sm/src/kpm/build_e2ap_indication.cpp
COPY ./kpm_e2sm/src/kpm/CMakeLists.txt /opt/e2sim/kpm_e2sm/src/kpm/CMakeLists.txt
COPY ./asn1c/ /opt/e2sim/kpm_e2sm/asn1c
COPY build_and_run.sh /opt/e2sim/build_and_run.sh

# Construire le simulateur complet
RUN mkdir /opt/e2sim/kpm_e2sm/.build && cd /opt/e2sim/kpm_e2sm/.build \
  && cmake .. && make install

# Lancement par défaut
CMD ["/opt/e2sim/kpm_e2sm/.build/e2sim_simulator"]
